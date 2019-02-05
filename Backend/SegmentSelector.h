#ifndef SEGMENTSELECTOR_H
#define SEGMENTSELECTOR_H
#include "LinuxThread.h"
#include "CmdReceiver.h"
#include "PlayerMsgQ.h"
#include "PlayerMsg_Common.h"
#include "PlayerStatus.h"
#include "PlayerMsg_Factory.h"
#include "PlayerTimer.h"

#include <memory>

class SegmentSelector : public LinuxThread, public CmdReceiver
{
 public:
    SegmentSelector();
    virtual ~SegmentSelector();

    virtual void InitComponent(CmdReceiver* manager);
    // override
    bool UpdateCMD(std::shared_ptr<PlayerMsg_Base> msg);
 protected:
    virtual void ProcessMsg(std::shared_ptr<PlayerMsg_DownloadMPD> msg);
    virtual void ProcessMsg(std::shared_ptr<PlayerMsg_RefreshMPD> msg);
    virtual void ProcessMsg(std::shared_ptr<PlayerMsg_Play> msg) = 0;
    virtual void ProcessMsg(std::shared_ptr<PlayerMsg_Pause> msg) = 0;
    virtual void ProcessMsg(std::shared_ptr<PlayerMsg_Stop> msg) = 0;
    virtual void ProcessMsg(std::shared_ptr<PlayerMsg_DownloadFinish> msg) = 0;
    virtual void ProcessMsg(std::shared_ptr<PlayerMsg_ProcessNextSegment> msg) = 0;

    void SendToManager(std::shared_ptr<PlayerMsg_Base> msg);
 private:
    void ProcessMsg(std::shared_ptr<PlayerMsg_Base> msg);
    // override
    void* Main();
 protected:
    PlayerMsg_Factory m_msgFactory;
    PlayerTimer m_eventTimer;
 private:
    CmdReceiver* m_manager;
    PlayerMsgQ m_msgQ;
};

#endif
