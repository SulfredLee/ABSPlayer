#ifndef PLAYERTIMER_H
#define PLAYERTIMER_H
#include "linuxThread.h"
#include "playerMsgQ.h"
#include "PlayerMsg_Base.h"
#include "PlayerMsg_Common.h"
#include "PlayerMsg_Factory.h"
#include "DefaultMutex.h"

#include <map>

struct playerTimerEvent
{
    PlayerMsg_Type m_msgType;
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
 private:
    uint64_t GetCurrentMSec();
    // override
    void* Main();
 private:
    playerMsgQ* m_msgQ;
    std::map<uint64_t, playerTimerEvent> m_eventQ; // key: next event time, value: event content
    PlayerMsg_Factory m_msgFactory;
    DefaultMutex m_mutex;
};

#endif
