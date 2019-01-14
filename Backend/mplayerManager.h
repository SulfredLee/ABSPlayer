#ifndef MPLAYER_MANAGER_H
#define MPLAYER_MANAGER_H
#include "linuxThread.h"
#include "cmdReceiver.h"
#include "playerMsgQ.h"
#include "fileDownloader.h"
#include "PlayerMsg_Common.h"
#include "playerStatus.h"
#include "segmentSelector.h"
#include "dashSegmentSelector.h"
#include "playerTimer.h"
#include "dirtyWriter.h"
#include "PlayerMsg_Factory.h"

#include <memory>

class mplayerManager : public linuxThread, public cmdReceiver
{
 public:
    mplayerManager();
    ~mplayerManager();

    void InitComponent();
    // override
    bool UpdateCMD(std::shared_ptr<PlayerMsg_Base> msg);
 private:
    void ProcessMsg(std::shared_ptr<PlayerMsg_Base> msg);
    void ProcessMsg(std::shared_ptr<PlayerMsg_Open> msg);
    void ProcessMsg(std::shared_ptr<PlayerMsg_Play> msg);
    void ProcessMsg(std::shared_ptr<PlayerMsg_ProcessNextSegment> msg);
    // override
    void* Main();
 private:
    PlayerMsg_Factory m_msgFactory;
    playerMsgQ m_msgQ;
    dirtyWriter m_dirtyWriter;
    playerTimer m_eventTimer;
    fileDownloader m_mpdDownloader;
    fileDownloader m_videoDownloader;
    fileDownloader m_audioDownloader;
    fileDownloader m_subtitleDownloader;
    playerStatus m_playerStatus;
    std::shared_ptr<segmentSelector> m_segmentSelector;
    PlayerMsg_Type m_lastMsgType;
};

#endif
