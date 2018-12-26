#ifndef PLAYERMSG_DOWNLOADVIDEO_H
#define PLAYERMSG_DOWNLOADVIDEO_H
#include "PlayerMsg_DownloadFile.h"

class PlayerMsg_DownloadVideo : public PlayerMsg_DownloadFile
{
 public:
    PlayerMsg_DownloadVideo();
    ~PlayerMsg_DownloadVideo();

    // override
    PlayerMsg_Type GetMsgType();
    // override
    std::string GetMsgTypeName();
};

#endif
