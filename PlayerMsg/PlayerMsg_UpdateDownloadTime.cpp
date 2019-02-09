#include "PlayerMsg_UpdateDownloadTime.h"

PlayerMsg_UpdateDownloadTime::PlayerMsg_UpdateDownloadTime()
{
}

PlayerMsg_UpdateDownloadTime::~PlayerMsg_UpdateDownloadTime()
{
}

// override
PlayerMsg_Type PlayerMsg_UpdateDownloadTime::GetMsgType()
{
}

// override
std::string PlayerMsg_UpdateDownloadTime::GetMsgTypeName()
{
    return "PlayerMsg_Type_UpdateDownloadTime";
}

void PlayerMsg_UpdateDownloadTime::SetFileType(PlayerMsg_Type fileType)
{
    m_downloadedFileType = fileType;
}

PlayerMsg_Type PlayerMsg_UpdateDownloadTime::GetFileType()
{
    return m_downloadedFileType;
}
