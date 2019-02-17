#ifndef CMD_RECEIVER_H
#define CMD_RECEIVER_H
#include "PlayerMsg_Base.h"
#include "SmartPointer.h"


class CmdReceiver
{
 public:
    CmdReceiver();
    virtual ~CmdReceiver();

    virtual bool UpdateCMD(SmartPointer<PlayerMsg_Base> msg) = 0;
};

#endif
