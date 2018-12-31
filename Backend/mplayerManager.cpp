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
    m_mpdDownloader.InitComponent(static_cast<cmdReceiver*>(this));
    m_videoDownloader.InitComponent(static_cast<cmdReceiver*>(this));
    m_audioDownloader.InitComponent(static_cast<cmdReceiver*>(this));
    m_subtitleDownloader.InitComponent(static_cast<cmdReceiver*>(this));
    m_playerStatus.InitComponent();
    m_eventTimer.InitComponent(&m_msgQ);
    m_dirtyWriter.InitComponent();
    startThread();
}

void mplayerManager::ProcessMsg(std::shared_ptr<PlayerMsg_Base> msg)
{
    LOGMSG_INFO("Process message %s from: %s", msg->GetMsgTypeName().c_str(), msg->GetSender().c_str());

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
    m_mpdDownloader.UpdateCMD(msgTemp);
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
        if (m_segmentSelector) m_segmentSelector->UpdateCMD(msg);
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
            UpdateCMD(std::static_pointer_cast<PlayerMsg_Base>(msgOpen));
            UpdateCMD(msg);
        }
    }
    else if (stage == PlayerStage_Open)
    {
        // wait a short time to see if the abs file is downloaded successfully
        m_eventTimer.AddEvent(PlayerMsg_Type_Play, 100, false);
    }
}

// override
bool mplayerManager::UpdateCMD(std::shared_ptr<PlayerMsg_Base> msg)
{
    LOGMSG_INFO("Received message %s from: %s", msg->GetMsgTypeName().c_str(), msg->GetSender().c_str());

    bool ret = true;
    switch(msg->GetMsgType())
    {
        case PlayerMsg_Type_DownloadMPD:
            {
                std::shared_ptr<PlayerMsg_DownloadMPD> msgMPD = std::static_pointer_cast<PlayerMsg_DownloadMPD>(msg);
                if (msgMPD->IsMPDFileEmpty())
                {
                    LOGMSG_ERROR("Cannot download abs file");
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

                    if (m_segmentSelector) m_segmentSelector->UpdateCMD(msg);
                }
                break;
            }
        case PlayerMsg_Type_GetPlayerStage:
            {
                PlayerStage stage = PlayerStage_Stop;
                m_playerStatus.ProcessStatusCMD(StatusCMD_Get_Stage, static_cast<void*>(&stage));
                std::shared_ptr<PlayerMsg_GetPlayerStage> msgStage = std::static_pointer_cast<PlayerMsg_GetPlayerStage>(msg);
                msgStage->SetPlayerStage(stage);
                break;
            }
        case PlayerMsg_Type_DownloadVideo:
            {
                if (msg->GetSender() == "segmentSelector")
                {
                    m_videoDownloader.UpdateCMD(msg);
                }
                else if (msg->GetSender() == "fileDownloader")
                {
                    if (!m_dirtyWriter.UpdateCMD(msg))
                    {
                        m_eventTimer.AddEvent(msg, 100, false); // try to process this message later
                    }
                }
                break;
            }
        case PlayerMsg_Type_DownloadAudio:
            {
                if (msg->GetSender() == "segmentSelector")
                {
                    m_audioDownloader.UpdateCMD(msg);
                }
                else if (msg->GetSender() == "fileDownloader")
                {
                    if (!m_dirtyWriter.UpdateCMD(msg))
                    {
                        m_eventTimer.AddEvent(msg, 100, false); // try to process this message later
                    }
                }
                break;
            }
        case PlayerMsg_Type_DownloadFinish:
            {
                if (m_segmentSelector)
                {
                    m_segmentSelector->UpdateCMD(msg);
                    // process next segment
                    std::shared_ptr<PlayerMsg_ProcessNextSegment> msgNext = std::dynamic_pointer_cast<PlayerMsg_ProcessNextSegment>(m_msgFactory.CreateMsg(PlayerMsg_Type_ProcessNextSegment));
                    std::shared_ptr<PlayerMsg_DownloadFinish> msgFinish = std::dynamic_pointer_cast<PlayerMsg_DownloadFinish>(msg);
                    msgNext->SetSegmentType(msgFinish->GetFileType());
                    m_segmentSelector->UpdateCMD(msgNext);
                }
                break;
            }
        case PlayerMsg_Type_Open:
        case PlayerMsg_Type_Play:
        case PlayerMsg_Type_Pause:
        case PlayerMsg_Type_Stop:
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
