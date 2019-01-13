#include "PlayerMsg_DownloadFinish.h"

PlayerMsg_DownloadFinish::PlayerMsg_DownloadFinish()
{
    m_downloadedSize = 0;
    m_downloadedSpeed = 0;
    m_downloadedTimeSpent = 0;
}

PlayerMsg_DownloadFinish::~PlayerMsg_DownloadFinish()
{
}

// override
PlayerMsg_Type PlayerMsg_DownloadFinish::GetMsgType()
{
    return PlayerMsg_Type_DownloadFinish;
}

// override
std::string PlayerMsg_DownloadFinish::GetMsgTypeName()
{
    return "PlayerMsg_Type_DownloadFinish";
}

void PlayerMsg_DownloadFinish::SetFileType(PlayerMsg_Type fileType)
{
    m_downloadedFileType = fileType;
}

void PlayerMsg_DownloadFinish::SetFileName(std::string fileName)
{
    m_downloadedFileName = fileName;
}

void PlayerMsg_DownloadFinish::SetSize(uint32_t size)
{
    m_downloadedSize = size;
}

void PlayerMsg_DownloadFinish::SetSpeed(uint32_t speed)
{
    m_downloadedSpeed = speed;
}

void PlayerMsg_DownloadFinish::SetTimeSpent(uint32_t timeSpent)
{
    m_downloadedTimeSpent = timeSpent;
}

PlayerMsg_Type PlayerMsg_DownloadFinish::GetFileType()
{
    return m_downloadedFileType;
}

std::string PlayerMsg_DownloadFinish::GetFileName()
{
    return m_downloadedFileName;
}

uint32_t PlayerMsg_DownloadFinish::GetSize()
{
    return m_downloadedSize;
}

uint32_t PlayerMsg_DownloadFinish::GetSpeed()
{
    return m_downloadedSpeed;
}

uint32_t PlayerMsg_DownloadFinish::GetTimeSpent()
{
    return m_downloadedTimeSpent;
}

void PlayerMsg_DownloadFinish::SetDownloadTime(uint64_t downloadTime)
{
    m_downloadTime = downloadTime;
}

void PlayerMsg_DownloadFinish::SetResponseCode(int32_t responseCode)
{
    m_responseCode = responseCode;
}

uint64_t PlayerMsg_DownloadFinish::GetDownloadTime()
{
    return m_downloadTime;
}

int32_t PlayerMsg_DownloadFinish::GetResponseCode()
{
    return m_responseCode;
}
