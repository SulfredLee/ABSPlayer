#ifndef ERRORHANDLER_H
#define ERRORHANDLER_H
#include "DefaultMutex.h"
#include "PlayerMsg_Common.h"

#include <memory>
#include <string>

class ErrorHandler
{
 public:
    ErrorHandler();
    ~ErrorHandler();

    bool IsTryAgain(std::shared_ptr<PlayerMsg_DownloadFinish> msg);
    bool IsTryAgain_Video(std::shared_ptr<PlayerMsg_DownloadFinish> msg);
    bool IsTryAgain_Audio(std::shared_ptr<PlayerMsg_DownloadFinish> msg);
    bool IsTryAgain_Subtitle(std::shared_ptr<PlayerMsg_DownloadFinish> msg);
 private:
    DefaultMutex m_mutex;

    std::string m_videoFileName;
    std::string m_audioFileName;
    std::string m_subtitleFileName;

    int m_videoErrorCount;
    int m_audioErrorCount;
    int m_subtitleErrorCount;

    int m_maxErrorCount;
};

#endif
