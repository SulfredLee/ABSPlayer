#ifndef PLAYERTIMER_H
#define PLAYERTIMER_H
#include "LinuxThread.h"
#include "PlayerMsgQ.h"
#include "PlayerMsg_Base.h"
#include "PlayerMsg_Common.h"
#include "PlayerMsg_Factory.h"
#include "DefaultMutex.h"
#include "SmartPointer.h"

#include <map>

struct PlayerTimerEvent
{
    PlayerMsg_Type m_msgType;
    SmartPointer<PlayerMsg_Base> m_msg;
    uint64_t m_targetTime;
    uint64_t m_duration;
    bool m_repeat;
};

class PlayerTimer : public LinuxThread
{
 public:
    PlayerTimer();
    ~PlayerTimer();

    void InitComponent(PlayerMsgQ* msgQ);
    void DeinitComponent();
    void AddEvent(PlayerMsg_Type msgType, uint64_t timeMSec, bool repeat);
    void AddEvent(SmartPointer<PlayerMsg_Base> msg, uint64_t timeMSec);
    void RemoveEvent(PlayerMsg_Type msgType);
 private:
    uint64_t GetCurrentMSec();
    bool IsNonRepeatType(PlayerMsg_Type msgType);
    void AddEvent_priv(const uint64_t& targetTime, const PlayerTimerEvent& tempEvent);
    // override
    void* Main();
 private:
    PlayerMsgQ* m_msgQ;
    std::multimap<uint64_t, PlayerTimerEvent> m_eventQ; // key: next event time, value: event content
    PlayerMsg_Factory m_msgFactory;
    DefaultMutex m_mutex;
};

#endif
