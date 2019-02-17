#ifndef PLAYERMSG_FACTORY_H
#define PLAYERMSG_FACTORY_H
#include "PlayerMsg_Base.h"
#include "PlayerMsg_Common.h"
#include "SmartPointer.h"


class PlayerMsg_Factory
{
 public:
    PlayerMsg_Factory();
    ~PlayerMsg_Factory();

    SmartPointer<PlayerMsg_Base> CreateMsg(PlayerMsg_Type msgType);
};

#endif
