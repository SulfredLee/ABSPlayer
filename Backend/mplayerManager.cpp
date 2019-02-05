#include "mplayerManager.h"
#include "Logger.h"

#include <iostream>
#include <unistd.h>

#include "IMPD.h"
#include "libdash.h"

mplayerManager::mplayerManager()
{
}

mplayerManager::~mplayerManager()
{
    m_segmentSelector = nullptr;
    m_mpdDownloader.DeinitComponent();
    m_videoDownloader.DeinitComponent();
    m_audioDownloader.DeinitComponent();
    m_subtitleDownloader.DeinitComponent();
    m_eventTimer.DeinitComponent();
    stopThread();
    std::shared_ptr<PlayerMsg_Dummy> msgDummy = std::make_shared<PlayerMsg_Dummy>();
    m_msgQ.AddMsg(std::static_pointer_cast<PlayerMsg_Base>(msgDummy));
    joinThread();
    LOGMSG_INFO("OUT");
}

void mplayerManager::InitComponent()
{
    m_msgQ.InitComponent(1024 * 1024 * 10); // 10 MByte buffer for message queue
    m_mpdDownloader.InitComponent(static_cast<cmdReceiver*>(this), "MPD_Download");
    m_videoDownloader.InitComponent(static_cast<cmdReceiver*>(this), "Video_Download");
    m_audioDownloader.InitComponent(static_cast<cmdReceiver*>(this), "Audio_Download");
    m_subtitleDownloader.InitComponent(static_cast<cmdReceiver*>(this), "Subtitle_Download");
    m_playerStatus.InitComponent();
    m_eventTimer.InitComponent(&m_msgQ);
    m_dirtyWriter.InitComponent();
    startThread();
}

void mplayerManager::UpdateCMD(std::shared_ptr<PlayerMsg_GetPlayerStage> msg)
{
    PlayerStage stage = PlayerStage_Stop;
    m_playerStatus.ProcessStatusCMD(StatusCMD_Get_Stage, static_cast<void*>(&stage));
    std::shared_ptr<PlayerMsg_GetPlayerStage> msgStage = std::dynamic_pointer_cast<PlayerMsg_GetPlayerStage>(msg);
    msgStage->SetPlayerStage(stage);
}

void mplayerManager::ProcessMsg(std::shared_ptr<PlayerMsg_Base> msg)
{
    LOGMSG_DEBUG("Process message %s from: %s", msg->GetMsgTypeName().c_str(), msg->GetSender().c_str());

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

void mplayerManager::ProcessMsg(std::shared_ptr<PlayerMsg_Open> msg)
{
    // update status
    PlayerStage stage = PlayerStage_Open;
    m_playerStatus.ProcessStatusCMD(StatusCMD_Set_Stage, static_cast<void*>(&stage));

    // save abs file url
    std::string ABSUrl = msg->GetURL();
    m_playerStatus.ProcessStatusCMD(StatusCMD_Set_ABSFileURL, static_cast<void*>(&ABSUrl));

    // create suitable segmentSelector
    m_segmentSelector = std::make_shared<dashSegmentSelector>();
    if (m_segmentSelector) m_segmentSelector->InitComponent(static_cast<cmdReceiver*>(this));

    // download mpd file
    std::shared_ptr<PlayerMsg_DownloadMPD> msgMPD = std::make_shared<PlayerMsg_DownloadMPD>();
    msgMPD->SetURL(msg->GetURL());
    std::shared_ptr<PlayerMsg_Base> msgTemp = std::static_pointer_cast<PlayerMsg_Base>(msgMPD);
    SendToMPDDownloader(msgTemp);
}

void mplayerManager::ProcessMsg(std::shared_ptr<PlayerMsg_Play> msg)
{
    // get status
    PlayerStage stage;
    m_playerStatus.ProcessStatusCMD(StatusCMD_Get_Stage, static_cast<void*>(&stage));
    if (stage == PlayerStage_Open_Finish || stage == PlayerStage_Pause)
    {
        // update status
        stage = PlayerStage_Play;
        m_playerStatus.ProcessStatusCMD(StatusCMD_Set_Stage, static_cast<void*>(&stage));

        // signal segmentSelector
        SendToSegmentSelector(msg);
    }
    else if (stage == PlayerStage_Stop)
    {
        // get abs file url
        std::string ABSUrl;
        m_playerStatus.ProcessStatusCMD(StatusCMD_Get_ABSFileURL, static_cast<void*>(&ABSUrl));
        if (ABSUrl.length()) // if we have abs url, we will download it and play
        {
            std::shared_ptr<PlayerMsg_Open> msgOpen = std::make_shared<PlayerMsg_Open>();
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

void mplayerManager::ProcessMsg(std::shared_ptr<PlayerMsg_ProcessNextSegment> msg)
{
    SendToSegmentSelector(msg);
}

void mplayerManager::ProcessMsg(std::shared_ptr<PlayerMsg_RefreshMPD> msg)
{
    if (msg->GetSender() == "segmentSelector")
    {
        msg->SetSender("mplayerManager");
        m_eventTimer.AddEvent(msg, msg->GetMinimumUpdatePeriod());
    }
    else if (msg->GetSender() == "mplayerManager")
    {
        SendToMPDDownloader(msg);
    }
    else if (msg->GetSender() == "fileDownloader")
    {
        if (msg->IsMPDFileEmpty())
        {
            if (msg->GetURL().length())
            {
                msg->SetSender("mplayerManager");
                m_eventTimer.AddEvent(msg, 500);
            }
        }
        else
        {
            SendToSegmentSelector(msg);
        }
    }
}

void mplayerManager::ProcessMsg(std::shared_ptr<PlayerMsg_DownloadMPD> msg)
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

void mplayerManager::ProcessMsg(std::shared_ptr<PlayerMsg_DownloadVideo> msg)
{
    if (msg->GetSender() == "segmentSelector")
    {
        if (m_videoSegmentURL != msg->GetURL())
        {
            SendToVideoDownloader(msg);
            m_videoSegmentURL = msg->GetURL();
        }
    }
    else if (msg->GetSender() == "fileDownloader")
    {
        if (!SendToDirtyWriter(msg))
        {
            m_eventTimer.AddEvent(msg, 100); // try to process this message later
        }
    }
}

void mplayerManager::ProcessMsg(std::shared_ptr<PlayerMsg_DownloadAudio> msg)
{
    if (msg->GetSender() == "segmentSelector")
    {
        SendToAudioDownloader(msg);
    }
    else if (msg->GetSender() == "fileDownloader")
    {
        if (!SendToDirtyWriter(msg))
        {
            m_eventTimer.AddEvent(msg, 100); // try to process this message later
        }
    }
}

void mplayerManager::ProcessMsg(std::shared_ptr<PlayerMsg_DownloadSubtitle> msg)
{
}

void mplayerManager::ProcessMsg(std::shared_ptr<PlayerMsg_DownloadFinish> msg)
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

bool mplayerManager::SendToDirtyWriter(std::shared_ptr<PlayerMsg_Base> msg)
{
    msg->SetSender("mplayerManager");
    return m_dirtyWriter.UpdateCMD(msg);
}

bool mplayerManager::SendToSegmentSelector(std::shared_ptr<PlayerMsg_Base> msg)
{
    msg->SetSender("mplayerManager");
    if (m_segmentSelector)
        return m_segmentSelector->UpdateCMD(msg);
    else
        return false;
}

bool mplayerManager::SendToMPDDownloader(std::shared_ptr<PlayerMsg_Base> msg)
{
    msg->SetSender("mplayerManager");
    return m_mpdDownloader.UpdateCMD(msg);
}

bool mplayerManager::SendToVideoDownloader(std::shared_ptr<PlayerMsg_Base> msg)
{
    msg->SetSender("mplayerManager");
    return m_videoDownloader.UpdateCMD(msg);
}

bool mplayerManager::SendToAudioDownloader(std::shared_ptr<PlayerMsg_Base> msg)
{
    msg->SetSender("mplayerManager");
    return m_audioDownloader.UpdateCMD(msg);
}

bool mplayerManager::SendToSubtitleDownloader(std::shared_ptr<PlayerMsg_Base> msg)
{
    msg->SetSender("mplayerManager");
    return m_subtitleDownloader.UpdateCMD(msg);
}

// override
bool mplayerManager::UpdateCMD(std::shared_ptr<PlayerMsg_Base> msg)
{
    LOGMSG_DEBUG("Received message %s from: %s", msg->GetMsgTypeName().c_str(), msg->GetSender().c_str());

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
void* mplayerManager::Main()
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
