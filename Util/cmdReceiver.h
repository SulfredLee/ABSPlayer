#ifndef CMD_RECEIVER_H
#define CMD_RECEIVER_H
#include "PlayerMsg_Base.h"

#include <memory>

class cmdReceiver
{
 public:
    cmdReceiver();
    virtual ~cmdReceiver();

    virtual void UpdateCMDReceiver(std::shared_ptr<PlayerMsg_Base> msg) = 0;
};

#endif
