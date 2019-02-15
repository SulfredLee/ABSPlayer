#ifndef MPLAYER_MANAGER_H
#define MPLAYER_MANAGER_H
#include "LinuxThread.h"
#include "CmdReceiver.h"
#include "PlayerMsgQ.h"
#include "FileDownloader.h"
#include "PlayerMsg_Common.h"
#include "PlayerStatus.h"
#include "SegmentSelector.h"
#include "DashSegmentSelector.h"
#include "PlayerTimer.h"
#include "DirtyWriter.h"
#include "PlayerMsg_Factory.h"
#include "MsgCounter.h"
#include "ErrorHandler.h"

#include <memory>

class ABSPlayerManager : public LinuxThread, public CmdReceiver
{
 public:
    ABSPlayerManager();
    ~ABSPlayerManager();

    void InitComponent();
    // override
    bool UpdateCMD(std::shared_ptr<PlayerMsg_Base> msg);
 private:
    void UpdateCMD(std::shared_ptr<PlayerMsg_GetPlayerStage> msg);

    void ProcessMsg(std::shared_ptr<PlayerMsg_Base> msg);
    void ProcessMsg(std::shared_ptr<PlayerMsg_Open> msg);
    void ProcessMsg(std::shared_ptr<PlayerMsg_Play> msg);
    void ProcessMsg(std::shared_ptr<PlayerMsg_ProcessNextSegment> msg);
    void ProcessMsg(std::shared_ptr<PlayerMsg_RefreshMPD> msg);
    void ProcessMsg(std::shared_ptr<PlayerMsg_DownloadMPD> msg);
    void ProcessMsg(std::shared_ptr<PlayerMsg_DownloadVideo> msg);
    void ProcessMsg(std::shared_ptr<PlayerMsg_DownloadAudio> msg);
    void ProcessMsg(std::shared_ptr<PlayerMsg_DownloadSubtitle> msg);
    void ProcessMsg(std::shared_ptr<PlayerMsg_DownloadFinish> msg);

    bool SendToDirtyWriter(std::shared_ptr<PlayerMsg_Base> msg);
    bool SendToSegmentSelector(std::shared_ptr<PlayerMsg_Base> msg);
    bool SendToMPDDownloader(std::shared_ptr<PlayerMsg_Base> msg);
    bool SendToVideoDownloader(std::shared_ptr<PlayerMsg_Base> msg);
    bool SendToAudioDownloader(std::shared_ptr<PlayerMsg_Base> msg);
    bool SendToSubtitleDownloader(std::shared_ptr<PlayerMsg_Base> msg);
    // override
    void* Main();
 private:
    PlayerMsg_Factory m_msgFactory;
    PlayerMsgQ m_msgQ;
    DirtyWriter m_dirtyWriter;
    PlayerTimer m_eventTimer;
    FileDownloader m_mpdDownloader;
    FileDownloader m_videoDownloader;
    FileDownloader m_audioDownloader;
    FileDownloader m_subtitleDownloader;
    PlayerStatus m_playerStatus;
    std::shared_ptr<SegmentSelector> m_segmentSelector;
    MsgCounter m_processMsgCounter;
    MsgCounter m_cmdMsgCounter;
    ErrorHandler m_errorHandler;
};

#endif
