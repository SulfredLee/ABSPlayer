#ifndef PLAYERSTATUS_H
#define PLAYERSTATUS_H
#include <string>
#include <memory>
#include "DefaultMutex.h"

enum StatusCMD
{
    StatusCMD_Set_Stage,
    StatusCMD_Set_ABSFileURL,
    StatusCMD_Get_Stage,
    StatusCMD_Get_ABSFileURL,
    StatusCMD_End
};

enum PlayerStage
{
    PlayerStage_Open,
    PlayerStage_Open_Finish,
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
    PlayerStage m_stage;
    std::string m_ABSUrl;
    DefaultMutex m_mutex;
};

#endif
