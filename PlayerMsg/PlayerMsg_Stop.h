#ifndef PALYERMSG_STOP_H
#define PLAYERMSG_STOP_H
#include "PlayerMsg_Base.h"

#include <string>

class PlayerMsg_Stop : public PlayerMsg_Base
{
 public:
    PlayerMsg_Stop();
    ~PlayerMsg_Stop();

    // override
    PlayerMsg_Type GetMsgType();
    // override
    std::string GetMsgTypeName();
};

#endif
