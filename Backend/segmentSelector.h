#ifndef SEGMENTSELECTOR_H
#define SEGMENTSELECTOR_H
#include "linuxThread.h"
#include "cmdReceiver.h"
#include "playerMsgQ.h"
#include "PlayerMsg_Common.h"
#include "playerStatus.h"
#include "PlayerMsg_Factory.h"

#include <memory>

class segmentSelector : public linuxThread, public cmdReceiver
{
 public:
    segmentSelector();
    virtual ~segmentSelector();

    virtual void InitComponent(cmdReceiver* manager);
    // override
    bool UpdateCMD(std::shared_ptr<PlayerMsg_Base> msg);
 protected:
    virtual void ProcessMsg(std::shared_ptr<PlayerMsg_DownloadMPD> msg);
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
 private:
    cmdReceiver* m_manager;
    playerMsgQ m_msgQ;
};

#endif
