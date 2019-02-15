#include "PlayerStatus.h"
#include "PlayerMsg_Common.h"
#include "Logger.h"

PlayerStatus::PlayerStatus()
{}

PlayerStatus::~PlayerStatus()
{}

void PlayerStatus::InitComponent()
{
    m_stage = PlayerStage_Stop;
    m_ABSUrl.clear();
}

void PlayerStatus::ProcessStatusCMD(StatusCMD cmd, void* data)
{
    DefaultLock lock(&m_mutex);
    switch(cmd)
    {
        case StatusCMD_Get_ABSFileURL:
            {
                std::string* url = static_cast<std::string*>(data);
                *url = m_ABSUrl;
                break;
            }
        case StatusCMD_Get_Stage:
            {
                PlayerStage* stage = static_cast<PlayerStage*>(data);
                *stage = m_stage;
                break;
            }
        case StatusCMD_Set_ABSFileURL:
            {
                std::string* url = static_cast<std::string*>(data);
                m_ABSUrl = *url;
                break;
            }
        case StatusCMD_Set_Stage:
            {
                PlayerStage* stage = static_cast<PlayerStage*>(data);
                m_stage = *stage;
                LOGMSG_INFO("Current Player stage: %u", m_stage);
                break;
            }
        default:
            break;
    }
}
