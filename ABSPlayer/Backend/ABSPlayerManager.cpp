#include "ABSPlayerManager.h"
#include "Logger.h"

#include <iostream>
#include <unistd.h>

#include "IMPD.h"
#include "libdash.h"

ABSPlayerManager::ABSPlayerManager()
{
}

ABSPlayerManager::~ABSPlayerManager()
{
    m_segmentSelector = nullptr;
    m_mpdDownloader.DeinitComponent();
    m_videoDownloader.DeinitComponent();
    m_audioDownloader.DeinitComponent();
    m_subtitleDownloader.DeinitComponent();
    m_eventTimer.DeinitComponent();
    stopThread();
    SmartPointer<PlayerMsg_Dummy> msgDummy = DynamicCast<PlayerMsg_Dummy>(m_msgFactory.CreateMsg(PlayerMsg_Type_Dummy));
    m_msgQ.AddMsg(StaticCast<PlayerMsg_Base>(msgDummy));
    joinThread();
    LOGMSG_INFO("OUT");
}

void ABSPlayerManager::InitComponent()
{
    m_msgQ.InitComponent(1024 * 1024 * 10); // 10 MByte buffer for message queue
    m_mpdDownloader.InitComponent(static_cast<CmdReceiver*>(this), "MPD_Download");
    m_videoDownloader.InitComponent(static_cast<CmdReceiver*>(this), "Video_Download");
    m_audioDownloader.InitComponent(static_cast<CmdReceiver*>(this), "Audio_Download");
    m_subtitleDownloader.InitComponent(static_cast<CmdReceiver*>(this), "Subtitle_Download");
    m_playerStatus.InitComponent();
    m_eventTimer.InitComponent(&m_msgQ);
    m_dirtyWriter.InitComponent();
    m_processMsgCounter.InitComponent("Manager_ProcessMsgCounter");
    m_cmdMsgCounter.InitComponent("Manager_cmdMsgCounter");
    startThread();
}

void ABSPlayerManager::UpdateCMD(SmartPointer<PlayerMsg_GetPlayerStage> msg)
{
    PlayerStage stage = PlayerStage_Stop;
    m_playerStatus.ProcessStatusCMD(StatusCMD_Get_Stage, static_cast<void*>(&stage));
    SmartPointer<PlayerMsg_GetPlayerStage> msgStage = DynamicCast<PlayerMsg_GetPlayerStage>(msg);
    msgStage->SetPlayerStage(stage);
}

void ABSPlayerManager::ProcessMsg(SmartPointer<PlayerMsg_Base> msg)
{
    m_processMsgCounter.AddCount(msg);

    switch(msg->GetMsgType())
    {
        case PlayerMsg_Type_Open:
            {
                ProcessMsg(DynamicCast<PlayerMsg_Open>(msg));
                break;
            }
        case PlayerMsg_Type_Play:
            {
                ProcessMsg(DynamicCast<PlayerMsg_Play>(msg));
                break;
            }
        case PlayerMsg_Type_ProcessNextSegment:
            {
                ProcessMsg(DynamicCast<PlayerMsg_ProcessNextSegment>(msg));
                break;
            }
        case PlayerMsg_Type_RefreshMPD:
            {
                ProcessMsg(DynamicCast<PlayerMsg_RefreshMPD>(msg));
                break;
            }
        case PlayerMsg_Type_DownloadMPD:
            {
                ProcessMsg(DynamicCast<PlayerMsg_DownloadMPD>(msg));
                break;
            }
        case PlayerMsg_Type_DownloadVideo:
            {
                ProcessMsg(DynamicCast<PlayerMsg_DownloadVideo>(msg));
                break;
            }
        case PlayerMsg_Type_DownloadAudio:
            {
                ProcessMsg(DynamicCast<PlayerMsg_DownloadAudio>(msg));
                break;
            }
        case PlayerMsg_Type_DownloadSubtitle:
            {
                ProcessMsg(DynamicCast<PlayerMsg_DownloadSubtitle>(msg));
                break;
            }
        case PlayerMsg_Type_DownloadFinish:
            {
                ProcessMsg(DynamicCast<PlayerMsg_DownloadFinish>(msg));
                break;
            }
        default:
            break;
    }
}

void ABSPlayerManager::ProcessMsg(SmartPointer<PlayerMsg_Open> msg)
{
    // update status
    PlayerStage stage = PlayerStage_Open;
    m_playerStatus.ProcessStatusCMD(StatusCMD_Set_Stage, static_cast<void*>(&stage));

    // save abs file url
    std::string ABSUrl = msg->GetURL();
    m_playerStatus.ProcessStatusCMD(StatusCMD_Set_ABSFileURL, static_cast<void*>(&ABSUrl));

    // create suitable SegmentSelector
    m_segmentSelector = StaticCast<SegmentSelector>(MakeSmartPointer<DashSegmentSelector>());
    if (m_segmentSelector.Get()) m_segmentSelector->InitComponent(static_cast<CmdReceiver*>(this));

    // download mpd file
    SmartPointer<PlayerMsg_DownloadMPD> msgMPD = DynamicCast<PlayerMsg_DownloadMPD>(m_msgFactory.CreateMsg(PlayerMsg_Type_DownloadMPD));
    msgMPD->SetURL(msg->GetURL());
    SmartPointer<PlayerMsg_Base> msgTemp = StaticCast<PlayerMsg_Base>(msgMPD);
    SendToMPDDownloader(msgTemp);
}

void ABSPlayerManager::ProcessMsg(SmartPointer<PlayerMsg_Play> msg)
{
    // get status
    PlayerStage stage;
    m_playerStatus.ProcessStatusCMD(StatusCMD_Get_Stage, static_cast<void*>(&stage));
    if (stage == PlayerStage_Open_Finish || stage == PlayerStage_Pause)
    {
        // update status
        stage = PlayerStage_Play;
        m_playerStatus.ProcessStatusCMD(StatusCMD_Set_Stage, static_cast<void*>(&stage));

        // signal SegmentSelector
        SendToSegmentSelector(StaticCast<PlayerMsg_Base>(msg));
    }
    else if (stage == PlayerStage_Stop)
    {
        // get abs file url
        std::string ABSUrl;
        m_playerStatus.ProcessStatusCMD(StatusCMD_Get_ABSFileURL, static_cast<void*>(&ABSUrl));
        if (ABSUrl.length()) // if we have abs url, we will download it and play
        {
            SmartPointer<PlayerMsg_Open> msgOpen= DynamicCast<PlayerMsg_Open>(m_msgFactory.CreateMsg(PlayerMsg_Type_Open));
            msgOpen->SetURL(ABSUrl);
            ProcessMsg(msgOpen);
            ProcessMsg(msg);
        }
    }
    else if (stage == PlayerStage_Open)
    {
        // wait a short time to see if the abs file is downloaded successfully
        m_eventTimer.AddEvent(PlayerMsg_Type_Play, 100, false);
    }
}

void ABSPlayerManager::ProcessMsg(SmartPointer<PlayerMsg_ProcessNextSegment> msg)
{
    SendToSegmentSelector(StaticCast<PlayerMsg_Base>(msg));
}

void ABSPlayerManager::ProcessMsg(SmartPointer<PlayerMsg_RefreshMPD> msg)
{
    if (msg->GetSender() == "SegmentSelector")
    {
        msg->SetSender("ABSPlayerManager");
        m_eventTimer.AddEvent(StaticCast<PlayerMsg_Base>(msg), msg->GetMinimumUpdatePeriod());
    }
    else if (msg->GetSender() == "ABSPlayerManager")
    {
        SendToMPDDownloader(StaticCast<PlayerMsg_Base>(msg));
    }
    else if (msg->GetSender() == "FileDownloader")
    {
        if (msg->IsMPDFileEmpty())
        {
            if (msg->GetURL().length())
            {
                msg->SetSender("ABSPlayerManager");
                m_eventTimer.AddEvent(StaticCast<PlayerMsg_Base>(msg), 500);
            }
        }
        else
        {
            SendToSegmentSelector(StaticCast<PlayerMsg_Base>(msg));
        }
    }
}

void ABSPlayerManager::ProcessMsg(SmartPointer<PlayerMsg_DownloadMPD> msg)
{
    SmartPointer<PlayerMsg_DownloadMPD> msgMPD = DynamicCast<PlayerMsg_DownloadMPD>(msg);
    if (msgMPD->IsMPDFileEmpty())
    {
        LOGMSG_ERROR("Cannot download mpd file");
        // update status
        PlayerStage stage = PlayerStage_Stop;
        m_playerStatus.ProcessStatusCMD(StatusCMD_Set_Stage, static_cast<void*>(&stage));

        // save abs file url
        std::string ABSUrl = "";
        m_playerStatus.ProcessStatusCMD(StatusCMD_Set_ABSFileURL, static_cast<void*>(&ABSUrl));
    }
    else
    {
        // update status
        PlayerStage stage = PlayerStage_Open_Finish;
        m_playerStatus.ProcessStatusCMD(StatusCMD_Set_Stage, static_cast<void*>(&stage));

        SendToSegmentSelector(StaticCast<PlayerMsg_Base>(msg));
    }
}

void ABSPlayerManager::ProcessMsg(SmartPointer<PlayerMsg_DownloadVideo> msg)
{
    if (msg->GetSender() == "SegmentSelector")
    {
        if (msg->GetErrorMsg() == "")
            SendToVideoDownloader(StaticCast<PlayerMsg_Base>(msg));
        else if (msg->GetErrorMsg() == "Live_Media_EOS")
        {
            SmartPointer<PlayerMsg_ProcessNextSegment> msgNext = DynamicCast<PlayerMsg_ProcessNextSegment>(m_msgFactory.CreateMsg(PlayerMsg_Type_ProcessNextSegment));
            msgNext->SetSegmentType(PlayerMsg_Type_DownloadVideo);
            m_eventTimer.AddEvent(StaticCast<PlayerMsg_Base>(msgNext), 500);
            LOGMSG_INFO("Wait and then process message with download time: %lu", msg->GetDownloadTime());
        }
    }
    else if (msg->GetSender() == "FileDownloader")
    {
        if (!SendToDirtyWriter(StaticCast<PlayerMsg_Base>(msg)))
        {
            m_eventTimer.AddEvent(StaticCast<PlayerMsg_Base>(msg), 100); // try to process this message later
        }
    }
}

void ABSPlayerManager::ProcessMsg(SmartPointer<PlayerMsg_DownloadAudio> msg)
{
    if (msg->GetSender() == "SegmentSelector")
    {
        if (msg->GetErrorMsg() == "")
            SendToAudioDownloader(StaticCast<PlayerMsg_Base>(msg));
        else if (msg->GetErrorMsg() == "Live_Media_EOS")
        {
            SmartPointer<PlayerMsg_ProcessNextSegment> msgNext = DynamicCast<PlayerMsg_ProcessNextSegment>(m_msgFactory.CreateMsg(PlayerMsg_Type_ProcessNextSegment));
            msgNext->SetSegmentType(PlayerMsg_Type_DownloadAudio);
            m_eventTimer.AddEvent(StaticCast<PlayerMsg_Base>(msgNext), 500);
            LOGMSG_INFO("Wait and then process message with download time: %lu", msg->GetDownloadTime());
        }
    }
    else if (msg->GetSender() == "FileDownloader")
    {
        if (!SendToDirtyWriter(StaticCast<PlayerMsg_Base>(msg)))
        {
            m_eventTimer.AddEvent(StaticCast<PlayerMsg_Base>(msg), 100); // try to process this message later
        }
    }
}

void ABSPlayerManager::ProcessMsg(SmartPointer<PlayerMsg_DownloadSubtitle> msg)
{
}

void ABSPlayerManager::ProcessMsg(SmartPointer<PlayerMsg_DownloadFinish> msg)
{
    if (!SendToDirtyWriter(StaticCast<PlayerMsg_Base>(msg)))
    {
        m_eventTimer.AddEvent(StaticCast<PlayerMsg_Base>(msg), 100); // try to process this message later
    }
    else
    {
        if (m_segmentSelector.Get())
        {
            SendToSegmentSelector(StaticCast<PlayerMsg_Base>(msg));
            // process next segment
            SmartPointer<PlayerMsg_ProcessNextSegment> msgNext = DynamicCast<PlayerMsg_ProcessNextSegment>(m_msgFactory.CreateMsg(PlayerMsg_Type_ProcessNextSegment));
            msgNext->SetSegmentType(msg->GetFileType());
            if (msg->GetResponseCode() == 200)
                SendToSegmentSelector(StaticCast<PlayerMsg_Base>(msgNext));
            else
            {
                if (m_errorHandler.IsTryAgain(msg))
                {
                    LOGMSG_INFO("Process %s later", msg->GetMsgTypeName().c_str());
                    m_eventTimer.AddEvent(StaticCast<PlayerMsg_Base>(msgNext), 500);
                }
                else
                {
                    // skip this segment and process next one
                    SmartPointer<PlayerMsg_UpdateDownloadTime> msgUpdateTime = DynamicCast<PlayerMsg_UpdateDownloadTime>(m_msgFactory.CreateMsg(PlayerMsg_Type_UpdateDownloadTime));
                    msgUpdateTime->SetFileType(msg->GetFileType());
                    SendToSegmentSelector(StaticCast<PlayerMsg_Base>(msgUpdateTime));
                    SendToSegmentSelector(StaticCast<PlayerMsg_Base>(msgNext));
                }
            }
        }
    }
}

bool ABSPlayerManager::SendToDirtyWriter(SmartPointer<PlayerMsg_Base> msg)
{
    msg->SetSender("ABSPlayerManager");
    return m_dirtyWriter.UpdateCMD(msg);
}

bool ABSPlayerManager::SendToSegmentSelector(SmartPointer<PlayerMsg_Base> msg)
{
    msg->SetSender("ABSPlayerManager");
    if (m_segmentSelector.Get())
        return m_segmentSelector->UpdateCMD(msg);
    else
        return false;
    return false; // should not go to this point
}

bool ABSPlayerManager::SendToMPDDownloader(SmartPointer<PlayerMsg_Base> msg)
{
    msg->SetSender("ABSPlayerManager");
    return m_mpdDownloader.UpdateCMD(msg);
}

bool ABSPlayerManager::SendToVideoDownloader(SmartPointer<PlayerMsg_Base> msg)
{
    msg->SetSender("ABSPlayerManager");
    return m_videoDownloader.UpdateCMD(msg);
}

bool ABSPlayerManager::SendToAudioDownloader(SmartPointer<PlayerMsg_Base> msg)
{
    msg->SetSender("ABSPlayerManager");
    return m_audioDownloader.UpdateCMD(msg);
}

bool ABSPlayerManager::SendToSubtitleDownloader(SmartPointer<PlayerMsg_Base> msg)
{
    msg->SetSender("ABSPlayerManager");
    return m_subtitleDownloader.UpdateCMD(msg);
}

// override
bool ABSPlayerManager::UpdateCMD(SmartPointer<PlayerMsg_Base> msg)
{
    m_cmdMsgCounter.AddCount(msg);

    bool ret = true;
    switch(msg->GetMsgType())
    {
        case PlayerMsg_Type_GetPlayerStage:
            {
                UpdateCMD(DynamicCast<PlayerMsg_GetPlayerStage>(msg));
                break;
            }
        case PlayerMsg_Type_Open:
        case PlayerMsg_Type_Play:
        case PlayerMsg_Type_Pause:
        case PlayerMsg_Type_Stop:
        case PlayerMsg_Type_DownloadMPD:
        case PlayerMsg_Type_DownloadVideo:
        case PlayerMsg_Type_DownloadAudio:
        case PlayerMsg_Type_DownloadFinish:
        case PlayerMsg_Type_RefreshMPD:
            {
                if (!m_msgQ.AddMsg(msg))
                {
                    ret = false;
                    LOGMSG_ERROR("AddMsg fail");
                }
                break;
            }
        default:
            break;
    }
    return ret;
}

// override
void* ABSPlayerManager::Main()
{
    LOGMSG_INFO("IN");

    while(isThreadRunning())
    {
        SmartPointer<PlayerMsg_Base> msg;
        m_msgQ.GetMsg(msg);

        if (!isThreadRunning()) break;

        ProcessMsg(msg);
    }

    LOGMSG_INFO("OUT");
    return NULL;
}
