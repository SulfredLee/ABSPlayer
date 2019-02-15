#ifndef PALYERMSG_PAUSE_H
#define PLAYERMSG_PAUSE_H
#include "PlayerMsg_Base.h"

#include <string>

class PlayerMsg_Pause : public PlayerMsg_Base
{
 public:
    PlayerMsg_Pause();
    ~PlayerMsg_Pause();

    // override
    PlayerMsg_Type GetMsgType();
    // override
    std::string GetMsgTypeName();
};

#endif
