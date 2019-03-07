#include "ServerTimeSynHelper.h"
#include "Logger.h"
#include <sys/time.h>
#include <ctime>
#include <stdio.h>

ServerTimeSynHelper::ServerTimeSynHelper()
{
    m_serverTime = 0;
    m_localTime = 0;
}

ServerTimeSynHelper::~ServerTimeSynHelper()
{
}

void ServerTimeSynHelper::UpdateServerUTCTime(const std::string& timeStr)
{
    m_localTime = GetCurrentUTCTime();
    GetUTCDateTimeString2MSec(timeStr, m_serverTime);
}

void ServerTimeSynHelper::UpdateServerUTCTime(uint64_t timeUTC)
{
    m_localTime = GetCurrentUTCTime();
    m_serverTime = timeUTC;
}

uint64_t ServerTimeSynHelper::Get_UTC_Time()
{
    if (m_serverTime)
        return m_serverTime + (GetCurrentUTCTime() - m_localTime);
    else
    {
        LOGMSG_WARN("We are NOT using server time");
        return GetCurrentUTCTime();
    }
}

bool ServerTimeSynHelper::GetUTCDateTimeString2MSec(std::string timeStr, uint64_t& timeMSec)
{
    if (timeStr.length() == 20)
    {
        int year, month, day, hour, minute, second;
        sscanf(timeStr.c_str(), "%d-%d-%dT%d:%d:%dZ", &year, &month, &day, &hour, &minute, &second);

        if (year < 1970)
        {
            timeMSec = 0;
            return true;
        }

        struct tm timeInfo;
        timeInfo.tm_year = year - 1900;
        timeInfo.tm_mon = month - 1;
        timeInfo.tm_mday = day;
        timeInfo.tm_hour = hour;
        timeInfo.tm_min = minute;
        timeInfo.tm_sec = second;

        timeMSec = static_cast<uint64_t>(timegm(&timeInfo)) * 1000;

        return true;
    }
    else
    {
        LOGMSG_ERROR("timeStr format is not correct: %s", timeStr.c_str());
        return false;
    }
}

uint64_t ServerTimeSynHelper::GetCurrentUTCTime()
{
    struct timeval curTV;
    struct timezone curTZ;
    gettimeofday(&curTV, &curTZ);
    return (static_cast<uint64_t>(curTV.tv_sec) * 1000 + curTV.tv_usec / 1000.0) + 0.5;
}
