#include "PlayerMsg_Base.h"

PlayerMsg_Base::PlayerMsg_Base()
{
    m_sender = "unknown";
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

size_t PlayerMsg_Base::GetMsgSize()
{
    return 0;
}

void PlayerMsg_Base::SetSender(std::string sender)
{
    m_sender = sender;
}

std::string PlayerMsg_Base::GetSender()
{
    return m_sender;
}
