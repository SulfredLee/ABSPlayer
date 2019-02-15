#include "PlayerMsg_Open.h"

PlayerMsg_Open::PlayerMsg_Open()
{}

PlayerMsg_Open::~PlayerMsg_Open()
{}

// override
PlayerMsg_Type PlayerMsg_Open::GetMsgType()
{
    return PlayerMsg_Type_Open;
}

// override
std::string PlayerMsg_Open::GetMsgTypeName()
{
    return "PlayerMsg_Type_Open";
}

void PlayerMsg_Open::SetURL(std::string url)
{
    m_url = url;
}

std::string PlayerMsg_Open::GetURL()
{
    return m_url;
}
