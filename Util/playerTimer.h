#ifndef PLAYERTIMER_H
#define PLAYERTIMER_H
#include "linuxThread.h"
#include "playerMsgQ.h"
#include "PlayerMsg_Base.h"
#include "PlayerMsg_Common.h"
#include "PlayerMsg_Factory.h"
#include "DefaultMutex.h"

#include <map>
#include <memory>

struct playerTimerEvent
{
    PlayerMsg_Type m_msgType;
    std::shared_ptr<PlayerMsg_Base> m_msg;
    uint64_t m_targetTime;
    uint64_t m_duration;
    bool m_repeat;
};

class playerTimer : public linuxThread
{
 public:
    playerTimer();
    ~playerTimer();

    void InitComponent(playerMsgQ* msgQ);
    void DeinitComponent();
    void AddEvent(PlayerMsg_Type msgType, uint64_t timeMSec, bool repeat);
    void AddEvent(std::shared_ptr<PlayerMsg_Base> msg, uint64_t timeMSec);
    void RemoveEvent(PlayerMsg_Type msgType);
 private:
    uint64_t GetCurrentMSec();
    void AddEvent_priv(const uint64_t& targetTime, const playerTimerEvent& tempEvent);
    // override
    void* Main();
 private:
    playerMsgQ* m_msgQ;
    std::map<uint64_t, playerTimerEvent> m_eventQ; // key: next event time, value: event content
    PlayerMsg_Factory m_msgFactory;
    DefaultMutex m_mutex;
};

#endif
