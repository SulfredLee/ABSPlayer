#include "PlayerMsg_Factory.h"

PlayerMsg_Factory::PlayerMsg_Factory()
{}

PlayerMsg_Factory::~PlayerMsg_Factory()
{}

std::shared_ptr<PlayerMsg_Base> PlayerMsg_Factory::CreateMsg(PlayerMsg_Type msgType)
{
    switch (msgType)
    {
        case PlayerMsg_Type_Dummy:
            return std::make_shared<PlayerMsg_Dummy>();
        case PlayerMsg_Type_Open:
            return std::make_shared<PlayerMsg_Open>();
        case PlayerMsg_Type_Play:
            return std::make_shared<PlayerMsg_Play>();
        case PlayerMsg_Type_Pause:
            return std::make_shared<PlayerMsg_Pause>();
        case PlayerMsg_Type_Stop:
            return std::make_shared<PlayerMsg_Stop>();
        case PlayerMsg_Type_DownloadFile:
            return std::make_shared<PlayerMsg_DownloadFile>();
        case PlayerMsg_Type_DownloadMPD:
            return std::make_shared<PlayerMsg_DownloadMPD>();
        case PlayerMsg_Type_DownloadVideo:
            return std::make_shared<PlayerMsg_DownloadVideo>();
        case PlayerMsg_Type_DownloadAudio:
            return std::make_shared<PlayerMsg_DownloadAudio>();
        case PlayerMsg_Type_DownloadSubtitle:
            return std::make_shared<PlayerMsg_DownloadSubtitle>();
        case PlayerMsg_Type_DownloadFinish:
            return std::make_shared<PlayerMsg_DownloadFinish>();
        case PlayerMsg_Type_GetPlayerStage:
            return std::make_shared<PlayerMsg_GetPlayerStage>();
        case PlayerMsg_Type_ProcessNextSegment:
            return std::make_shared<PlayerMsg_ProcessNextSegment>();
        case PlayerMsg_Type_RefreshMPD:
            return std::make_shared<PlayerMsg_RefreshMPD>();
        case PlayerMsg_Type_UpdateDownloadTime:
            return std::make_shared<PlayerMsg_UpdateDownloadTime>();
        case PlayerMsg_Type_Base:
        default:
            return std::make_shared<PlayerMsg_Base>();
    }
}
