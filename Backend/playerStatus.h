#ifndef PLAYERSTATUS_H
#define PLAYERSTATUS_H
#include <string>
#include <memory>
#include "DefaultMutex.h"

enum StatusCMD
{
    StatusCMD_Set_MPD,
    StatusCMD_Set_Stage,
    StatusCMD_Get_MPD,
    StatusCMD_Get_Stage,
    StatusCMD_End
};

enum PlayerStage
{
    PlayerStage_Play,
    PlayerStage_Pause,
    PlayerStage_Stop,
    PlayerStage_End
};

class playerStatus
{
 public:
    playerStatus();
    ~playerStatus();

    void InitComponent();
    void ProcessStatusCMD(StatusCMD cmd, void* data);
 private:
    std::string m_mpdFile;
    PlayerStage m_stage;
    DefaultMutex m_mutex;
};

#endif
