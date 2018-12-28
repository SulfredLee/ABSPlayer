#include "mplayerManager.h"
#include "Logger.h"

#include <iostream>
#include <unistd.h>

mplayerManager::mplayerManager()
{
}

mplayerManager::~mplayerManager()
{
    m_mpdDownloader.DeinitComponent();
    m_videoDownloader.DeinitComponent();
    m_audioDownloader.DeinitComponent();
    m_subtitleDownloader.DeinitComponent();
    stopThread();
    std::shared_ptr<PlayerMsg_Dummy> msgDummy = std::make_shared<PlayerMsg_Dummy>();
    m_msgQ.AddMsg(std::static_pointer_cast<PlayerMsg_Base>(msgDummy));
}

void mplayerManager::InitComponent()
{
    m_msgQ.InitComponent(1024 * 1024 * 10); // 10 MByte buffer for message queue
    m_mpdDownloader.InitComponent(static_cast<cmdReceiver*>(this));
    m_videoDownloader.InitComponent(static_cast<cmdReceiver*>(this));
    m_audioDownloader.InitComponent(static_cast<cmdReceiver*>(this));
    m_subtitleDownloader.InitComponent(static_cast<cmdReceiver*>(this));
    m_playerStatus.InitComponent();
    startThread();
}

void mplayerManager::ProcessMsg(std::shared_ptr<PlayerMsg_Base> msg)
{
    LOGMSG_INFO("Process message %s", msg->GetMsgTypeName().c_str());

    switch(msg->GetMsgType())
    {
        case PlayerMsg_Type_Open:
            {
                ProcessMsg(std::dynamic_pointer_cast<PlayerMsg_Open>(msg));
                break;
            }
        default:
            break;
    }
}

void mplayerManager::ProcessMsg(std::shared_ptr<PlayerMsg_Open> msg)
{
    std::shared_ptr<PlayerMsg_DownloadMPD> msgMPD = std::make_shared<PlayerMsg_DownloadMPD>();
    msgMPD->SetURL(msg->GetURL());
    std::shared_ptr<PlayerMsg_Base> msgTemp = std::static_pointer_cast<PlayerMsg_Base>(msgMPD);
    m_mpdDownloader.UpdateCMDReceiver(msgTemp);
}

// override
void mplayerManager::UpdateCMDReceiver(std::shared_ptr<PlayerMsg_Base> msg)
{
    LOGMSG_INFO("Received message %s", msg->GetMsgTypeName().c_str());

    switch(msg->GetMsgType())
    {
        case PlayerMsg_Type_DownloadMPD:
            {
                m_playerStatus.ProcessStatusCMD(StatusCMD_Set_MPD, static_cast<void*>(msg.get()));
                break;
            }
        case PlayerMsg_Type_Open:
        case PlayerMsg_Type_Play:
        case PlayerMsg_Type_Pause:
        case PlayerMsg_Type_Stop:
            {
                m_msgQ.AddMsg(msg);
                break;
            }
        default:
            break;
    }
}

// override
void* mplayerManager::Main()
{
    LOGMSG_INFO("IN");

    while(isThreadRunning())
    {
        std::shared_ptr<PlayerMsg_Base> msg;
        m_msgQ.GetMsg(msg);

        ProcessMsg(msg);
    }

    LOGMSG_INFO("OUT");
    return NULL;
}
