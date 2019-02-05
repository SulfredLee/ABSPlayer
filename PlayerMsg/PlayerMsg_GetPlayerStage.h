#ifndef PLAYERMSG_GETPLAYERSTAGE_H
#define PLAYERMSG_GETPLAYERSTAGE_H
#include "PlayerMsg_Base.h"
#include "PlayerStatus.h"

class PlayerMsg_GetPlayerStage : public PlayerMsg_Base
{
 public:
    PlayerMsg_GetPlayerStage();
    ~PlayerMsg_GetPlayerStage();

    PlayerMsg_Type GetMsgType();
    std::string GetMsgTypeName();

    void SetPlayerStage(PlayerStage playerStage);
    PlayerStage GetPlayerStage();
 private:
    PlayerStage m_playerStage;
};

#endif
