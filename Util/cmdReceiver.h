#ifndef CMD_RECEIVER_H
#define CMD_RECEIVER_H
#include "PlayerMsg_Base.h"

class cmdReceiver
{
 public:
    cmdReceiver();
    virtual ~cmdReceiver();

    virtual void UpdateCMDReceiver(PlayerMsg_Base*& msg) = 0;
};

#endif
