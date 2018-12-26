#ifndef PALYERMSG_PLAY_H
#define PLAYERMSG_PLAY_H
#include "PlayerMsg_Base.h"

#include <string>

class PlayerMsg_Play : public PlayerMsg_Base
{
 public:
    PlayerMsg_Play();
    ~PlayerMsg_Play();

    // override
    PlayerMsg_Type GetMsgType();
    // override
    std::string GetMsgTypeName();
};

#endif
