#include "PlayerMsg_Stop.h"

PlayerMsg_Stop::PlayerMsg_Stop()
{}

PlayerMsg_Stop::~PlayerMsg_Stop()
{}

// override
PlayerMsg_Type PlayerMsg_Stop::GetMsgType()
{
    return PlayerMsg_Type_Stop;
}

// override
std::string PlayerMsg_Stop::GetMsgTypeName()
{
    return "PlayerMsg_Type_Stop";
}
