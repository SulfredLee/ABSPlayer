#include "playerStatus.h"
#include "PlayerMsg_Common.h"

playerStatus::playerStatus()
{}

playerStatus::~playerStatus()
{}

void playerStatus::InitComponent()
{
    m_stage = PlayerStage_Stop;
}

void playerStatus::ProcessStatusCMD(StatusCMD cmd, void* data)
{
    DefaultLock lock(&m_mutex);
    switch(cmd)
    {
        case StatusCMD_Get_MPD:
            {
                std::string* str = static_cast<std::string*>(data);
                *str = m_mpdFile;
                break;
            }
        case StatusCMD_Get_Stage:
            {
                PlayerStage* stage = static_cast<PlayerStage*>(data);
                *stage = m_stage;
                break;
            }
        case StatusCMD_Set_MPD:
            {
                PlayerMsg_DownloadMPD* msgMPD = static_cast<PlayerMsg_DownloadMPD*>(data);
                std::vector<unsigned char> mpdFile = msgMPD->GetFile();
                m_mpdFile.clear();
                m_mpdFile.insert(m_mpdFile.end(), mpdFile.begin(), mpdFile.end());
                break;
            }
        case StatusCMD_Set_Stage:
            {
                PlayerStage* stage = static_cast<PlayerStage*>(data);
                m_stage = *stage;
                break;
            }
        default:
            break;
    }
}
