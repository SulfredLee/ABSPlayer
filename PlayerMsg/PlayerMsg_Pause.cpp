#include "PlayerMsg_Pause.h"

PlayerMsg_Pause::PlayerMsg_Pause()
{}

PlayerMsg_Pause::~PlayerMsg_Pause()
{}

// override
PlayerMsg_Type PlayerMsg_Pause::GetMsgType()
{
    return PlayerMsg_Type_Pause;
}

// override
std::string PlayerMsg_Pause::GetMsgTypeName()
{
    return "PlayerMsg_Type_Pause";
}
