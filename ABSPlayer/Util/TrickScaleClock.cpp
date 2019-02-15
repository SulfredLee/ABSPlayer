#include "TrickScaleClock.h"
#include "Logger.h"

TrickScaleClock::TrickScaleClock()
{
    m_timeMSec = 0;
    m_trickScale = 1000;
}

TrickScaleClock::~TrickScaleClock()
{
    stopThread();
    joinThread();
    LOGMSG_INFO("OUT");
}

void TrickScaleClock::InitComponent(uint64_t timeMSec, int32_t trickScale, std::string thisName)
{
    LOGMSG_CLASS_NAME(thisName);
    SetTimeMSec(timeMSec);
    SetTrickScale(trickScale);
    startThread();
}

void TrickScaleClock::SetTimeMSec(uint64_t timeMSec)
{
    DefaultLock lock(&m_mutex);
    m_timeMSec = timeMSec;
}

void TrickScaleClock::SetTrickScale(int32_t trickScale)
{
    DefaultLock lock(&m_mutex);
    m_trickScale = trickScale;
}

uint64_t TrickScaleClock::GetTimeMSec()
{
    DefaultLock lock(&m_mutex);
    return m_timeMSec;
}

int32_t TrickScaleClock::GetTrickScale()
{
    DefaultLock lock(&m_mutex);
    return m_trickScale;
}

// override
void* TrickScaleClock::Main()
{
    LOGMSG_INFO("IN");

    while(isThreadRunning())
    {
        if (!isThreadRunning()) break;
        usleep(10000); // 10 msec
        DefaultLock lock(&m_mutex);
        m_timeMSec += m_trickScale / 100;
    }

    LOGMSG_INFO("OUT");
    return NULL;
}
