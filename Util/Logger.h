#ifndef LOGGER_H
#define LOGGER_H
#include <string>
#include <fstream>
#include <map>

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
    static std::string GetFileName(const std::string& fillPath);

    void InitComponent(const LoggerConfig& config);
    LoggerConfig GetConfig();
    void Log(LogLevel logLevel, const char* format, ...);
    void AddClassName(std::string className, void* object);
    std::string GetClassName(void* object, std::string prettyFunction);
 private:
    LoggerConfig m_config;
    std::ofstream m_outFH;
    DefaultMutex m_DefaultMutex;
    std::map<void*, std::string> m_classNameMap; // key: object address, object class name
 private:
    Logger();
    Logger(const Logger&) = delete;
    Logger & operator=(const Logger&) = delete;

    std::string GetCurrentTime();
    std::string GetLogLevelName(LogLevel logLevel);
    std::string _GetClassName(const std::string& prettyFunction);
    bool OpenLogFile();
};

#define __LOGMSG(level, ...)                               \
    do{                                                     \
        char buffer[5120];                                   \
        snprintf(buffer, sizeof(buffer), __VA_ARGS__);      \
        Logger::GetInstance().Log(level, "%25s,%25s:%6d,%6d,%25s,%s", Logger::GetFileName(__FILE__).c_str(), __FUNCTION__, __LINE__, syscall(SYS_gettid), Logger::GetInstance().GetClassName(this, __PRETTY_FUNCTION__).c_str(), buffer); \
    }while(0)

#define LOGMSG_DEBUG(...) __LOGMSG(Logger::LogLevel::DEBUG, __VA_ARGS__)
#define LOGMSG_INFO(...)  __LOGMSG(Logger::LogLevel::INFO, __VA_ARGS__)
#define LOGMSG_WARN(...)  __LOGMSG(Logger::LogLevel::WARN, __VA_ARGS__)
#define LOGMSG_ERROR(...) __LOGMSG(Logger::LogLevel::ERROR, __VA_ARGS__)
#define LOGMSG_CLASS_NAME(className) Logger::GetInstance().AddClassName(className, this)

#endif
