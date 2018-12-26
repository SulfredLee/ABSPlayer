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

void mplayerManager::ProcessMsg(PlayerMsg_Base* msg)
{
    LOGMSG_INFO("Process message %s", msg->GetMsgTypeName().c_str());

    switch(msg->GetMsgType())
    {
        case PlayerMsg_Type_Open:
            {
                ProcessMsg(static_cast<PlayerMsg_Open*>(msg));
                break;
            }
        default:
            break;
    }
}

void mplayerManager::ProcessMsg(PlayerMsg_Open* msg)
{
    PlayerMsg_DownloadMPD* msgMPD = new PlayerMsg_DownloadMPD();
    msgMPD->SetURL(msg->GetURL());
    PlayerMsg_Base* msgTemp = static_cast<PlayerMsg_Base*>(msgMPD);
    m_mpdDownloader.UpdateCMDReceiver(msgTemp);
}

// override
void mplayerManager::UpdateCMDReceiver(PlayerMsg_Base*& msg)
{
    LOGMSG_INFO("Received message %s", msg->GetMsgTypeName().c_str());

    switch(msg->GetMsgType())
    {
        case PlayerMsg_Type_DownloadMPD:
            {
                m_playerStatus.ProcessStatusCMD(StatusCMD_Set_MPD, static_cast<void*>(msg));
                break;
            }
        case PlayerMsg_Type_Open:
        case PlayerMsg_Type_Play:
        case PlayerMsg_Type_Pause:
        case PlayerMsg_Type_Stop:
            {
                if (!m_msgQ.AddMsg(msg))
                {
                    LOGMSG_ERROR("AddMsg() fail, delete, %s", msg->GetMsgTypeName().c_str());
                    if (msg)
                        delete msg;
                    msg = NULL;
                }
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
        PlayerMsg_Base* msg = NULL;
        m_msgQ.GetMsg(msg);

        ProcessMsg(msg);

        if (msg)
        {
            delete msg;
            msg = NULL;
        }
    }

    LOGMSG_INFO("OUT");
    return NULL;
}
