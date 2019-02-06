#include "MPlayerManager.h"
#include "Logger.h"

#include <iostream>
#include <unistd.h>

#include "IMPD.h"
#include "libdash.h"

MPlayerManager::MPlayerManager()
{
}

MPlayerManager::~MPlayerManager()
{
    m_segmentSelector = nullptr;
    m_mpdDownloader.DeinitComponent();
    m_videoDownloader.DeinitComponent();
    m_audioDownloader.DeinitComponent();
    m_subtitleDownloader.DeinitComponent();
    m_eventTimer.DeinitComponent();
    stopThread();
    std::shared_ptr<PlayerMsg_Dummy> msgDummy = std::dynamic_pointer_cast<PlayerMsg_Dummy>(m_msgFactory.CreateMsg(PlayerMsg_Type_Dummy));
    m_msgQ.AddMsg(std::static_pointer_cast<PlayerMsg_Base>(msgDummy));
    joinThread();
    LOGMSG_INFO("OUT");
}

void MPlayerManager::InitComponent()
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

void MPlayerManager::UpdateCMD(std::shared_ptr<PlayerMsg_GetPlayerStage> msg)
{
    PlayerStage stage = PlayerStage_Stop;
    m_playerStatus.ProcessStatusCMD(StatusCMD_Get_Stage, static_cast<void*>(&stage));
    std::shared_ptr<PlayerMsg_GetPlayerStage> msgStage = std::dynamic_pointer_cast<PlayerMsg_GetPlayerStage>(msg);
    msgStage->SetPlayerStage(stage);
}

void MPlayerManager::ProcessMsg(std::shared_ptr<PlayerMsg_Base> msg)
{
    m_processMsgCounter.AddCount(msg);

    switch(msg->GetMsgType())
    {
        case PlayerMsg_Type_Open:
            {
                ProcessMsg(std::dynamic_pointer_cast<PlayerMsg_Open>(msg));
                break;
            }
        case PlayerMsg_Type_Play:
            {
                ProcessMsg(std::dynamic_pointer_cast<PlayerMsg_Play>(msg));
                break;
            }
        case PlayerMsg_Type_ProcessNextSegment:
            {
                ProcessMsg(std::dynamic_pointer_cast<PlayerMsg_ProcessNextSegment>(msg));
                break;
            }
        case PlayerMsg_Type_RefreshMPD:
            {
                ProcessMsg(std::dynamic_pointer_cast<PlayerMsg_RefreshMPD>(msg));
                break;
            }
        case PlayerMsg_Type_DownloadMPD:
            {
                ProcessMsg(std::dynamic_pointer_cast<PlayerMsg_DownloadMPD>(msg));
                break;
            }
        case PlayerMsg_Type_DownloadVideo:
            {
                ProcessMsg(std::dynamic_pointer_cast<PlayerMsg_DownloadVideo>(msg));
                break;
            }
        case PlayerMsg_Type_DownloadAudio:
            {
                ProcessMsg(std::dynamic_pointer_cast<PlayerMsg_DownloadAudio>(msg));
                break;
            }
        case PlayerMsg_Type_DownloadSubtitle:
            {
                ProcessMsg(std::dynamic_pointer_cast<PlayerMsg_DownloadSubtitle>(msg));
                break;
            }
        case PlayerMsg_Type_DownloadFinish:
            {
                ProcessMsg(std::dynamic_pointer_cast<PlayerMsg_DownloadFinish>(msg));
                break;
            }
        default:
            break;
    }
}

void MPlayerManager::ProcessMsg(std::shared_ptr<PlayerMsg_Open> msg)
{
    // update status
    PlayerStage stage = PlayerStage_Open;
    m_playerStatus.ProcessStatusCMD(StatusCMD_Set_Stage, static_cast<void*>(&stage));

    // save abs file url
    std::string ABSUrl = msg->GetURL();
    m_playerStatus.ProcessStatusCMD(StatusCMD_Set_ABSFileURL, static_cast<void*>(&ABSUrl));

    // create suitable SegmentSelector
    m_segmentSelector = std::make_shared<DashSegmentSelector>();
    if (m_segmentSelector) m_segmentSelector->InitComponent(static_cast<CmdReceiver*>(this));

    // download mpd file
    std::shared_ptr<PlayerMsg_DownloadMPD> msgMPD = std::dynamic_pointer_cast<PlayerMsg_DownloadMPD>(m_msgFactory.CreateMsg(PlayerMsg_Type_DownloadMPD));
    msgMPD->SetURL(msg->GetURL());
    std::shared_ptr<PlayerMsg_Base> msgTemp = std::static_pointer_cast<PlayerMsg_Base>(msgMPD);
    SendToMPDDownloader(msgTemp);
}

void MPlayerManager::ProcessMsg(std::shared_ptr<PlayerMsg_Play> msg)
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
        SendToSegmentSelector(msg);
    }
    else if (stage == PlayerStage_Stop)
    {
        // get abs file url
        std::string ABSUrl;
        m_playerStatus.ProcessStatusCMD(StatusCMD_Get_ABSFileURL, static_cast<void*>(&ABSUrl));
        if (ABSUrl.length()) // if we have abs url, we will download it and play
        {
            std::shared_ptr<PlayerMsg_Open> msgOpen= std::dynamic_pointer_cast<PlayerMsg_Open>(m_msgFactory.CreateMsg(PlayerMsg_Type_Open));
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

void MPlayerManager::ProcessMsg(std::shared_ptr<PlayerMsg_ProcessNextSegment> msg)
{
    SendToSegmentSelector(msg);
}

void MPlayerManager::ProcessMsg(std::shared_ptr<PlayerMsg_RefreshMPD> msg)
{
    if (msg->GetSender() == "SegmentSelector")
    {
        msg->SetSender("MPlayerManager");
        m_eventTimer.AddEvent(msg, msg->GetMinimumUpdatePeriod());
    }
    else if (msg->GetSender() == "MPlayerManager")
    {
        SendToMPDDownloader(msg);
    }
    else if (msg->GetSender() == "FileDownloader")
    {
        if (msg->IsMPDFileEmpty())
        {
            if (msg->GetURL().length())
            {
                msg->SetSender("MPlayerManager");
                m_eventTimer.AddEvent(msg, 500);
            }
        }
        else
        {
            SendToSegmentSelector(msg);
        }
    }
}

void MPlayerManager::ProcessMsg(std::shared_ptr<PlayerMsg_DownloadMPD> msg)
{
    std::shared_ptr<PlayerMsg_DownloadMPD> msgMPD = std::dynamic_pointer_cast<PlayerMsg_DownloadMPD>(msg);
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

        SendToSegmentSelector(msg);
    }
}

void MPlayerManager::ProcessMsg(std::shared_ptr<PlayerMsg_DownloadVideo> msg)
{
    if (msg->GetSender() == "SegmentSelector")
    {
        if (m_videoSegmentURL != msg->GetURL())
        {
            SendToVideoDownloader(msg);
            m_videoSegmentURL = msg->GetURL();
        }
    }
    else if (msg->GetSender() == "FileDownloader")
    {
        if (!SendToDirtyWriter(msg))
        {
            m_eventTimer.AddEvent(msg, 100); // try to process this message later
        }
    }
}

void MPlayerManager::ProcessMsg(std::shared_ptr<PlayerMsg_DownloadAudio> msg)
{
    if (msg->GetSender() == "SegmentSelector")
    {
        if (m_audioSegmentURL != msg->GetURL())
        {
            SendToAudioDownloader(msg);
            m_audioSegmentURL = msg->GetURL();
        }
    }
    else if (msg->GetSender() == "FileDownloader")
    {
        if (!SendToDirtyWriter(msg))
        {
            m_eventTimer.AddEvent(msg, 100); // try to process this message later
        }
    }
}

void MPlayerManager::ProcessMsg(std::shared_ptr<PlayerMsg_DownloadSubtitle> msg)
{
}

void MPlayerManager::ProcessMsg(std::shared_ptr<PlayerMsg_DownloadFinish> msg)
{
    if (!SendToDirtyWriter(msg))
    {
        m_eventTimer.AddEvent(msg, 100); // try to process this message later
    }
    else
    {
        if (m_segmentSelector)
        {
            SendToSegmentSelector(msg);
            // check response code
            std::shared_ptr<PlayerMsg_DownloadFinish> msgFinish = std::dynamic_pointer_cast<PlayerMsg_DownloadFinish>(msg);
            // process next segment
            std::shared_ptr<PlayerMsg_ProcessNextSegment> msgNext = std::dynamic_pointer_cast<PlayerMsg_ProcessNextSegment>(m_msgFactory.CreateMsg(PlayerMsg_Type_ProcessNextSegment));
            msgNext->SetSegmentType(msgFinish->GetFileType());
            if (msgFinish->GetResponseCode() == 200)
                SendToSegmentSelector(msgNext);
            else
            {
                LOGMSG_INFO("Process %s later", msg->GetMsgTypeName().c_str());
                m_eventTimer.AddEvent(msgNext, 500);
            }
        }
    }
}

bool MPlayerManager::SendToDirtyWriter(std::shared_ptr<PlayerMsg_Base> msg)
{
    msg->SetSender("MPlayerManager");
    return m_dirtyWriter.UpdateCMD(msg);
}

bool MPlayerManager::SendToSegmentSelector(std::shared_ptr<PlayerMsg_Base> msg)
{
    msg->SetSender("MPlayerManager");
    if (m_segmentSelector)
        return m_segmentSelector->UpdateCMD(msg);
    else
        return false;
}

bool MPlayerManager::SendToMPDDownloader(std::shared_ptr<PlayerMsg_Base> msg)
{
    msg->SetSender("MPlayerManager");
    return m_mpdDownloader.UpdateCMD(msg);
}

bool MPlayerManager::SendToVideoDownloader(std::shared_ptr<PlayerMsg_Base> msg)
{
    msg->SetSender("MPlayerManager");
    return m_videoDownloader.UpdateCMD(msg);
}

bool MPlayerManager::SendToAudioDownloader(std::shared_ptr<PlayerMsg_Base> msg)
{
    msg->SetSender("MPlayerManager");
    return m_audioDownloader.UpdateCMD(msg);
}

bool MPlayerManager::SendToSubtitleDownloader(std::shared_ptr<PlayerMsg_Base> msg)
{
    msg->SetSender("MPlayerManager");
    return m_subtitleDownloader.UpdateCMD(msg);
}

// override
bool MPlayerManager::UpdateCMD(std::shared_ptr<PlayerMsg_Base> msg)
{
    m_cmdMsgCounter.AddCount(msg);

    bool ret = true;
    switch(msg->GetMsgType())
    {
        case PlayerMsg_Type_GetPlayerStage:
            {
                UpdateCMD(std::dynamic_pointer_cast<PlayerMsg_GetPlayerStage>(msg));
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
void* MPlayerManager::Main()
{
    LOGMSG_INFO("IN");

    while(isThreadRunning())
    {
        std::shared_ptr<PlayerMsg_Base> msg;
        m_msgQ.GetMsg(msg);

        if (!isThreadRunning()) break;

        ProcessMsg(msg);
    }

    LOGMSG_INFO("OUT");
    return NULL;
}
