#include "MsgCounter.h"
#include "Logger.h"

MsgCounter::MsgCounter()
{
}

MsgCounter::~MsgCounter()
{
    LOGMSG_INFO("Start OUT");
    stopThread();
    joinThread();
    LOGMSG_INFO("OUT");
}

void MsgCounter::InitComponent(std::string thisName)
{
    LOGMSG_CLASS_NAME(thisName);
    startThread();
}

void MsgCounter::AddCount(SmartPointer<PlayerMsg_Base> msg)
{
    DefaultLock lock(&m_mutex);

    std::map<std::string, uint32_t>::iterator it = m_counterDB.find(msg->GetMsgTypeName());
    if (it != m_counterDB.end())
        it->second++;
    else
        m_counterDB.insert(std::make_pair<std::string, uint32_t>(msg->GetMsgTypeName(), 1));
}

// override
void* MsgCounter::Main()
{
    LOGMSG_INFO("IN");

    int loopCount = 0;
    while(isThreadRunning())
    {
        while (loopCount < 500) // wait for 5 second
        {
            if (!isThreadRunning()) break;
            usleep(1000);
            loopCount++;
        }
        loopCount = 0;
        if (!isThreadRunning()) break;

        DefaultLock lock(&m_mutex);
        LOGMSG_INFO("================================================================== start print");
        for (std::map<std::string, uint32_t>::iterator it = m_counterDB.begin(); it != m_counterDB.end(); it++)
        {
            LOGMSG_INFO("%s %u", it->first.c_str(), it->second);
        }
        LOGMSG_INFO("================================================================== end print");
        m_counterDB.clear();
    }

    LOGMSG_INFO("OUT");
    return NULL;
}
