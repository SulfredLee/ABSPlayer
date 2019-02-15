#ifndef PALYERMSG_DUMMY_H
#define PLAYERMSG_DUMMY_H
#include "PlayerMsg_Base.h"

#include <string>

class PlayerMsg_Dummy : public PlayerMsg_Base
{
 public:
    PlayerMsg_Dummy();
    ~PlayerMsg_Dummy();

    // override
    PlayerMsg_Type GetMsgType();
    // override
    std::string GetMsgTypeName();
};

#endif
