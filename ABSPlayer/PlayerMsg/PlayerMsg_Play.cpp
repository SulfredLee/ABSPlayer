#include "PlayerMsg_Play.h"

PlayerMsg_Play::PlayerMsg_Play()
{}

PlayerMsg_Play::~PlayerMsg_Play()
{}

// override
PlayerMsg_Type PlayerMsg_Play::GetMsgType()
{
    return PlayerMsg_Type_Play;
}

// override
std::string PlayerMsg_Play::GetMsgTypeName()
{
    return "PlayerMsg_Type_Play";
}
