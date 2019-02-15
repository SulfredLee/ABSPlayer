#include "PlayerMsg_GetPlayerStage.h"

PlayerMsg_GetPlayerStage::PlayerMsg_GetPlayerStage()
{
    m_playerStage = PlayerStage_Stop;
}

PlayerMsg_GetPlayerStage::~PlayerMsg_GetPlayerStage()
{}

PlayerMsg_Type PlayerMsg_GetPlayerStage::GetMsgType()
{
    return PlayerMsg_Type_GetPlayerStage;
}

std::string PlayerMsg_GetPlayerStage::GetMsgTypeName()
{
    return "PlayerMsg_Type_GetPlayerStage";
}

void PlayerMsg_GetPlayerStage::SetPlayerStage(PlayerStage playerStage)
{
    m_playerStage = playerStage;
}

PlayerStage PlayerMsg_GetPlayerStage::GetPlayerStage()
{
    return m_playerStage;
}
