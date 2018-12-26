#ifndef MPLAYER_MANAGER_H
#define MPLAYER_MANAGER_H
#include "linuxThread.h"
#include "cmdReceiver.h"
#include "playerMsgQ.h"
#include "fileDownloader.h"
#include "PlayerMsg_Common.h"
#include "playerStatus.h"

class mplayerManager : public linuxThread, public cmdReceiver
{
 public:
    mplayerManager();
    ~mplayerManager();

    void InitComponent();
    // override
    void UpdateCMDReceiver(PlayerMsg_Base*& msg);
 private:
    void ProcessMsg(PlayerMsg_Base* msg);
    void ProcessMsg(PlayerMsg_Open* msg);
    // override
    void* Main();
 private:
    playerMsgQ m_msgQ;
    fileDownloader m_mpdDownloader;
    fileDownloader m_videoDownloader;
    fileDownloader m_audioDownloader;
    fileDownloader m_subtitleDownloader;
    playerStatus m_playerStatus;
};

#endif
