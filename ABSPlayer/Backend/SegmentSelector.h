#ifndef SEGMENTSELECTOR_H
#define SEGMENTSELECTOR_H
#include "LinuxThread.h"
#include "CmdReceiver.h"
#include "PlayerMsgQ.h"
#include "PlayerMsg_Common.h"
#include "PlayerStatus.h"
#include "PlayerMsg_Factory.h"
#include "PlayerTimer.h"
#include "SmartPointer.h"


class SegmentSelector : public LinuxThread, public CmdReceiver
{
 public:
    SegmentSelector();
    virtual ~SegmentSelector();

    virtual void InitComponent(CmdReceiver* manager);
    // override
    bool UpdateCMD(SmartPointer<PlayerMsg_Base> msg);
 protected:
    virtual void ProcessMsg(SmartPointer<PlayerMsg_DownloadMPD> msg);
    virtual void ProcessMsg(SmartPointer<PlayerMsg_RefreshMPD> msg);
    virtual void ProcessMsg(SmartPointer<PlayerMsg_Play> msg) = 0;
    virtual void ProcessMsg(SmartPointer<PlayerMsg_Pause> msg) = 0;
    virtual void ProcessMsg(SmartPointer<PlayerMsg_Stop> msg) = 0;
    virtual void ProcessMsg(SmartPointer<PlayerMsg_DownloadFinish> msg) = 0;
    virtual void ProcessMsg(SmartPointer<PlayerMsg_ProcessNextSegment> msg) = 0;
    virtual void ProcessMsg(SmartPointer<PlayerMsg_UpdateDownloadTime> msg) = 0;

    void SendToManager(SmartPointer<PlayerMsg_Base> msg);
 private:
    void ProcessMsg(SmartPointer<PlayerMsg_Base> msg);
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
