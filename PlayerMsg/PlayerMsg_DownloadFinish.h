#ifndef PLAYERMSG_DOWNLOADFINISH_H
#define PLAYERMSG_DOWNLOADFINISH_H
#include "PlayerMsg_Base.h"
#include <string>

class PlayerMsg_DownloadFinish : public PlayerMsg_Base
{
 public:
    PlayerMsg_DownloadFinish();
    ~PlayerMsg_DownloadFinish();

    // override
    PlayerMsg_Type GetMsgType();
    // override
    std::string GetMsgTypeName();

    void SetFileType(PlayerMsg_Type fileType);
    void SetFileName(std::string fileName);
    void SetSize(uint32_t size);
    void SetSpeed(uint32_t speed);
    void SetTimeSpent(uint32_t timeSpent);
    void SetDownloadTime(uint64_t downloadTime);

    PlayerMsg_Type GetFileType();
    std::string GetFileName();
    uint32_t GetSize();
    uint32_t GetSpeed();
    uint32_t GetTimeSpent();
    uint64_t GetDownloadTime();
 private:
    PlayerMsg_Type m_downloadedFileType;
    std::string m_downloadedFileName;
    uint32_t m_downloadedSize; // Byte
    uint32_t m_downloadedSpeed; // Byte per sec
    uint32_t m_downloadedTimeSpent; // MSec
    uint64_t m_downloadTime;
};

#endif
