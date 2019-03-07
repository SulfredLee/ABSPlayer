#ifndef SERVER_TIME_SYN_HELPER_H
#define SERVER_TIME_SYN_HELPER_H
#include <string>
#include <stdint.h>

class ServerTimeSynHelper
{
 public:
    ServerTimeSynHelper();
    ~ServerTimeSynHelper();

    void UpdateServerUTCTime(const std::string& timeStr);
    void UpdateServerUTCTime(uint64_t timeUTC);

    uint64_t Get_UTC_Time();
 private:
    bool GetUTCDateTimeString2MSec(std::string timeStr, uint64_t& timeMSec); // input a UTC date time string
    uint64_t GetCurrentUTCTime();
 private:
    uint64_t m_serverTime;
    uint64_t m_localTime;
};

#endif
