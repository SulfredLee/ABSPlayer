#ifndef PLAYERMSG_FACTORY_H
#define PLAYERMSG_FACTORY_H
#include "PlayerMsg_Base.h"
#include "PlayerMsg_Common.h"

#include <memory>

class PlayerMsg_Factory
{
 public:
    PlayerMsg_Factory();
    ~PlayerMsg_Factory();

    std::shared_ptr<PlayerMsg_Base> CreateMsg(PlayerMsg_Type msgType);
};

#endif
