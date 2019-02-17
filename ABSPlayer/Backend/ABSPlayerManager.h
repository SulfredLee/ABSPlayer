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
#include "SmartPointer.h"


class ABSPlayerManager : public LinuxThread, public CmdReceiver
{
 public:
    ABSPlayerManager();
    ~ABSPlayerManager();

    void InitComponent();
    // override
    bool UpdateCMD(SmartPointer<PlayerMsg_Base> msg);
 private:
    void UpdateCMD(SmartPointer<PlayerMsg_GetPlayerStage> msg);

    void ProcessMsg(SmartPointer<PlayerMsg_Base> msg);
    void ProcessMsg(SmartPointer<PlayerMsg_Open> msg);
    void ProcessMsg(SmartPointer<PlayerMsg_Play> msg);
    void ProcessMsg(SmartPointer<PlayerMsg_ProcessNextSegment> msg);
    void ProcessMsg(SmartPointer<PlayerMsg_RefreshMPD> msg);
    void ProcessMsg(SmartPointer<PlayerMsg_DownloadMPD> msg);
    void ProcessMsg(SmartPointer<PlayerMsg_DownloadVideo> msg);
    void ProcessMsg(SmartPointer<PlayerMsg_DownloadAudio> msg);
    void ProcessMsg(SmartPointer<PlayerMsg_DownloadSubtitle> msg);
    void ProcessMsg(SmartPointer<PlayerMsg_DownloadFinish> msg);

    bool SendToDirtyWriter(SmartPointer<PlayerMsg_Base> msg);
    bool SendToSegmentSelector(SmartPointer<PlayerMsg_Base> msg);
    bool SendToMPDDownloader(SmartPointer<PlayerMsg_Base> msg);
    bool SendToVideoDownloader(SmartPointer<PlayerMsg_Base> msg);
    bool SendToAudioDownloader(SmartPointer<PlayerMsg_Base> msg);
    bool SendToSubtitleDownloader(SmartPointer<PlayerMsg_Base> msg);
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
    SmartPointer<SegmentSelector> m_segmentSelector;
    MsgCounter m_processMsgCounter;
    MsgCounter m_cmdMsgCounter;
    ErrorHandler m_errorHandler;
};

#endif
