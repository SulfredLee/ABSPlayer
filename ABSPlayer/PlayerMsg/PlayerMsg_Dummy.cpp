#include "PlayerMsg_Dummy.h"

PlayerMsg_Dummy::PlayerMsg_Dummy()
{}

PlayerMsg_Dummy::~PlayerMsg_Dummy()
{}

// override
PlayerMsg_Type PlayerMsg_Dummy::GetMsgType()
{
    return PlayerMsg_Type_Dummy;
}

// override
std::string PlayerMsg_Dummy::GetMsgTypeName()
{
    return "PlayerMsg_Type_Dummy";
}
