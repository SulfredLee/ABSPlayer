#ifndef LOGGER_H
#define LOGGER_H
#include <string>
#include <fstream>

#include <unistd.h>
#include <sys/syscall.h>

#include "DefaultMutex.h"

class Logger
{
 public:
    enum class LogLevel{DEBUG, WARN, INFO, ERROR};
    struct LoggerConfig
    {
        Logger::LogLevel logLevel;
        std::string logPath;
        std::string logFilePathName;
        int fileSize;
        int fileSizeLimit;
        bool isToConsole;
        bool isToFile;
    };
 public:
    ~Logger();

    static Logger& GetInstance();

    void InitComponent(const LoggerConfig& config);
    LoggerConfig GetConfig();
    void Log(LogLevel logLevel, const char* format, ...);
 private:
    LoggerConfig m_config;
    std::ofstream m_outFH;
    DefaultMutex m_DefaultMutex;
 private:
    Logger();
    Logger(const Logger&) = delete;
    Logger & operator=(const Logger&) = delete;

    std::string GetCurrentTime();
    std::string GetLogLevelName(LogLevel logLevel);
    bool OpenLogFile();
};

#define __LOGMSG(level, ...)                               \
    do{                                                     \
        char buffer[5120];                                   \
        snprintf(buffer, sizeof(buffer), __VA_ARGS__);      \
        Logger::GetInstance().Log(level, "%s,%s:%d,%4d,%s", __FILE__, __FUNCTION__, __LINE__, syscall(SYS_gettid), buffer); \
    }while(0)

#define LOGMSG_DEBUG(...) __LOGMSG(Logger::LogLevel::DEBUG, __VA_ARGS__)
#define LOGMSG_INFO(...)  __LOGMSG(Logger::LogLevel::INFO, __VA_ARGS__)
#define LOGMSG_WARN(...)  __LOGMSG(Logger::LogLevel::WARN, __VA_ARGS__)
#define LOGMSG_ERROR(...) __LOGMSG(Logger::LogLevel::ERROR, __VA_ARGS__)

#endif
