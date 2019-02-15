#ifndef PLAYERMSG_UPDATE_DOWNLOADTIME_H
#define PLAYERMSG_UPDATE_DOWNLOADTIME_H
#include "PlayerMsg_Base.h"

class PlayerMsg_UpdateDownloadTime : public PlayerMsg_Base
{
 public:
    PlayerMsg_UpdateDownloadTime();
    ~PlayerMsg_UpdateDownloadTime();

    // override
    PlayerMsg_Type GetMsgType();
    // override
    std::string GetMsgTypeName();

    void SetFileType(PlayerMsg_Type fileType);

    PlayerMsg_Type GetFileType();
 private:
    PlayerMsg_Type m_downloadedFileType;
};

#endif
