#ifndef PLAYERMSG_DOWNLOADMPD_H
#define PLAYERMSG_DOWNLOADMPD_H
#include "PlayerMsg_DownloadFile.h"

class PlayerMsg_DownloadMPD : public PlayerMsg_DownloadFile
{
 public:
    PlayerMsg_DownloadMPD();
    ~PlayerMsg_DownloadMPD();

    // override
    PlayerMsg_Type GetMsgType();
    // override
    std::string GetMsgTypeName();
};

#endif
