#ifndef PLAYERMSG_DOWNLOADSUBTITLE_H
#define PLAYERMSG_DOWNLOADSUBTITLE_H
#include "PlayerMsg_DownloadFile.h"

class PlayerMsg_DownloadSubtitle : public PlayerMsg_DownloadFile
{
 public:
    PlayerMsg_DownloadSubtitle();
    ~PlayerMsg_DownloadSubtitle();

    // override
    PlayerMsg_Type GetMsgType();
    // override
    std::string GetMsgTypeName();
};

#endif
