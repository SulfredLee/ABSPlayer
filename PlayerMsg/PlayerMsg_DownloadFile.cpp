#include "PlayerMsg_DownloadFile.h"

PlayerMsg_DownloadFile::PlayerMsg_DownloadFile()
{
    m_fileCount = 0;
}

PlayerMsg_DownloadFile::~PlayerMsg_DownloadFile()
{}

// override
PlayerMsg_Type PlayerMsg_DownloadFile::GetMsgType()
{
    return PlayerMsg_Type_DownloadFile;
}

// override
std::string PlayerMsg_DownloadFile::GetMsgTypeName()
{
    return "PlayerMsg_Type_DownloadFile";
}

// override
size_t PlayerMsg_DownloadFile::GetMsgSize() // only report message size when message has media data
{
    return m_file.size();
}

void PlayerMsg_DownloadFile::SetURL(std::string url)
{
    m_url = url;
}

void PlayerMsg_DownloadFile::SetFile(unsigned char* data, size_t dataLength)
{
    m_file.insert(m_file.end(), data, data + dataLength);
}

void PlayerMsg_DownloadFile::SetFileCount(size_t count)
{
    m_fileCount = count;
}

void PlayerMsg_DownloadFile::SetResponseCode(int32_t responseCode)
{
    m_responseCode = responseCode;
}

void PlayerMsg_DownloadFile::ClearFile()
{
    m_file.clear();
}

std::string PlayerMsg_DownloadFile::GetURL()
{
    return m_url;
}

std::vector<unsigned char> PlayerMsg_DownloadFile::GetFile()
{
    return m_file;
}

size_t PlayerMsg_DownloadFile::GetFileCount()
{
    return m_fileCount;
}

size_t PlayerMsg_DownloadFile::GetFileLength()
{
    return m_file.size();
}

void PlayerMsg_DownloadFile::SetDownloadTime(uint64_t downloadTime)
{
    m_downloadTime = downloadTime;
}

uint64_t PlayerMsg_DownloadFile::GetDownloadTime()
{
    return m_downloadTime;
}

int32_t PlayerMsg_DownloadFile::GetResponseCode()
{
    return m_responseCode;
}
