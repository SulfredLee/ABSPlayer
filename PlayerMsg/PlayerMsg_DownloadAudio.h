#ifndef PLAYERMSG_DOWNLOADAUDIO_H
#define PLAYERMSG_DOWNLOADAUDIO_H
#include "PlayerMsg_DownloadFile.h"

class PlayerMsg_DownloadAudio : public PlayerMsg_DownloadFile
{
 public:
    PlayerMsg_DownloadAudio();
    ~PlayerMsg_DownloadAudio();

    // override
    PlayerMsg_Type GetMsgType();
    // override
    std::string GetMsgTypeName();
};

#endif
