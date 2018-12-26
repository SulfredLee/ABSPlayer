#include "PlayerMsg_Base.h"

PlayerMsg_Base::PlayerMsg_Base()
{
}

PlayerMsg_Base::~PlayerMsg_Base()
{
}

PlayerMsg_Type PlayerMsg_Base::GetMsgType()
{
    return PlayerMsg_Type_Base;
}

std::string PlayerMsg_Base::GetMsgTypeName()
{
    return "PlayerMsg_Type_Base";
}

int PlayerMsg_Base::GetMsgSize()
{
    return 0;
}
