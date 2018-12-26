#include "Logger.h"

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

#include <stdarg.h>     /* va_list, va_start, va_arg, va_end */
#include <string.h>
#include <stdio.h>
#include <time.h>

#include <sys/time.h>

Logger::Logger()
{
    // handle default value
    m_config.logLevel = Logger::LogLevel::DEBUG;
    m_config.logPath = ".";
    m_config.fileSize = 0;
    m_config.fileSizeLimit = 4 * 1024 * 1024; // 4 MByte
    m_config.isToConsole = true;
    m_config.isToFile = true;
}

Logger::~Logger()
{}

Logger& Logger::GetInstance()
{
    static Logger instance;
    return instance;
}

void Logger::InitComponent(const Logger::LoggerConfig& config)
{
    DefaultLock lock(&m_DefaultMutex);
    m_config = config;
}

Logger::LoggerConfig Logger::GetConfig()
{
    DefaultLock lock(&m_DefaultMutex);
    return m_config;
}

void Logger::Log(LogLevel logLevel, const char* format, ...)
{
    DefaultLock lock(&m_DefaultMutex);
    if (logLevel < m_config.logLevel)
    {
        return;
    }
    char dest[5120]; // 5 KByte
    int destLen = sizeof(dest);
    memset(dest, 0, destLen);

    va_list	args;
    va_start(args, format);
    vsnprintf(dest, destLen, format, args);
    va_end(args);

    std::stringstream ss;
    ss << GetCurrentTime() << " [" << GetLogLevelName(logLevel) << "] " << dest;
    m_config.fileSize += ss.str().length();

    if (m_config.isToFile && OpenLogFile())
    {
        m_outFH << ss.str() << std::endl;
    }

    if (m_config.isToConsole)
    {
        std::cout << ss.str() << std::endl;
    }
}

std::string Logger::GetCurrentTime()
{
    timeval curTime;
    gettimeofday(&curTime, NULL);
    int milli = curTime.tv_usec / 1000;

    char buffer [80];
    strftime(buffer, 80, "%Y-%m-%d_%H:%M:%S", localtime(&curTime.tv_sec));

    char currentTime[84] = "";
    sprintf(currentTime, "%s:%d", buffer, milli);

    // time compare example
    // struct timeval tval_before, tval_after, tval_result;
    // gettimeofday(&tval_before, NULL);
    // // Some code you want to time, for example:
    // sleep(1);
    // gettimeofday(&tval_after, NULL);
    // timersub(&tval_after, &tval_before, &tval_result);
    // printf("Time elapsed: %ld.%06ld\n", (long int)tval_result.tv_sec, (long int)tval_result.tv_usec);
    return std::string(currentTime);
}

std::string Logger::GetLogLevelName(LogLevel logLevel)
{
    switch (logLevel)
    {
        case Logger::LogLevel::DEBUG:
            return "DEBUG";
        case Logger::LogLevel::WARN:
            return "WARN";
        case Logger::LogLevel::INFO:
            return "INFO";
        case Logger::LogLevel::ERROR:
            return "ERROR";
        default:
            return std::string();
    }
}

bool Logger::OpenLogFile()
{
    if (m_outFH.is_open() && m_config.fileSize < m_config.fileSizeLimit)
    {
        return true;
    }
    else if (m_outFH.is_open())
    {
        m_outFH.close();
        m_config.fileSize = 0;
    }
    else
    {
        m_config.fileSize = 0;
    }

    m_config.logFilePathName = m_config.logPath + "/" + GetCurrentTime() + ".log";
    m_outFH.open(m_config.logFilePathName, std::ios::out);
    if (m_outFH.is_open())
    {
        return true;
    }
    else
    {
        m_config.logFilePathName = "./" + GetCurrentTime() + ".log";
        m_outFH.open(m_config.logFilePathName, std::ios::out);
        if (m_outFH.is_open())
        {
            return true;
        }
        else
        {
            return false;
        }
    }
}
