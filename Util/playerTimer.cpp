#include "playerTimer.h"
#include "Logger.h"
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

playerTimer::playerTimer()
{
    m_msgQ = nullptr;
}

playerTimer::~playerTimer()
{
    stopThread();
    DefaultLock lock(&m_mutex);
    m_msgQ = nullptr;
    joinThread();
    LOGMSG_INFO("OUT");
}

void playerTimer::InitComponent(playerMsgQ* msgQ)
{
    m_msgQ = msgQ;
    startThread();
}

void playerTimer::DeinitComponent()
{
    DefaultLock lock(&m_mutex);
    m_msgQ = nullptr;
}

void playerTimer::AddEvent(PlayerMsg_Type msgType, uint64_t timeMSec, bool repeat)
{
    uint64_t targetTime = GetCurrentMSec() + timeMSec;
    playerTimerEvent tempEvent = {msgType, nullptr, targetTime, timeMSec, repeat};
    DefaultLock lock(&m_mutex);
    m_eventQ.insert(std::make_pair(targetTime, tempEvent));
}

void playerTimer::AddEvent(std::shared_ptr<PlayerMsg_Base> msg, uint64_t timeMSec, bool repeat)
{
    uint64_t targetTime = GetCurrentMSec() + timeMSec;
    playerTimerEvent tempEvent = {msg->GetMsgType(), msg, targetTime, timeMSec, repeat};
    DefaultLock lock(&m_mutex);
    m_eventQ.insert(std::make_pair(targetTime, tempEvent));
}

uint64_t playerTimer::GetCurrentMSec()
{
    struct timeval  tv;
    gettimeofday(&tv, NULL);

    uint64_t timeMSec = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000;
    return timeMSec;
}

// override
void* playerTimer::Main()
{
    LOGMSG_INFO("IN");

    while(isThreadRunning())
    {
        m_mutex.Lock();
        auto it = m_eventQ.begin();
        while(it != m_eventQ.end())
        {
            uint64_t currentMSec = GetCurrentMSec();
            if (it->first <= currentMSec)
            {
                // handle timer event
                if (m_msgQ)
                {
                    if (it->second.m_msg != nullptr)
                        m_msgQ->AddMsg(it->second.m_msg);
                    else
                        m_msgQ->AddMsg(m_msgFactory.CreateMsg(it->second.m_msgType));
                }
                // handle repeate event
                if (it->second.m_repeat)
                {
                    uint64_t nextTargetTime = currentMSec + it->second.m_duration;
                    playerTimerEvent tempEvent = {it->second.m_msgType, it->second.m_msg, nextTargetTime, it->second.m_duration, it->second.m_repeat};
                    m_eventQ.insert(std::make_pair(nextTargetTime, tempEvent));
                }
                // remove and reset iterator
                m_eventQ.erase(it);
                it = m_eventQ.begin();
            }
            else
            {
                break;
            }
        }
        m_mutex.Unlock();
        usleep(5 * 1000); // sleep 5 msec
    }

    LOGMSG_INFO("OUT");
    return NULL;
}
