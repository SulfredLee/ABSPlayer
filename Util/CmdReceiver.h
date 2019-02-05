#ifndef CMD_RECEIVER_H
#define CMD_RECEIVER_H
#include "PlayerMsg_Base.h"

#include <memory>

class CmdReceiver
{
 public:
    CmdReceiver();
    virtual ~CmdReceiver();

    virtual bool UpdateCMD(std::shared_ptr<PlayerMsg_Base> msg) = 0;
};

#endif
