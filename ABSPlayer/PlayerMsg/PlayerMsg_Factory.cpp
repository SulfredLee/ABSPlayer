#include "PlayerMsg_Factory.h"

PlayerMsg_Factory::PlayerMsg_Factory()
{}

PlayerMsg_Factory::~PlayerMsg_Factory()
{}

SmartPointer<PlayerMsg_Base> PlayerMsg_Factory::CreateMsg(PlayerMsg_Type msgType)
{
    switch (msgType)
    {
        case PlayerMsg_Type_Dummy:
            return StaticCast<PlayerMsg_Base>(MakeSmartPointer<PlayerMsg_Dummy>());
        case PlayerMsg_Type_Open:
            return StaticCast<PlayerMsg_Base>(MakeSmartPointer<PlayerMsg_Open>());
        case PlayerMsg_Type_Play:
            return StaticCast<PlayerMsg_Base>(MakeSmartPointer<PlayerMsg_Play>());
        case PlayerMsg_Type_Pause:
            return StaticCast<PlayerMsg_Base>(MakeSmartPointer<PlayerMsg_Pause>());
        case PlayerMsg_Type_Stop:
            return StaticCast<PlayerMsg_Base>(MakeSmartPointer<PlayerMsg_Stop>());
        case PlayerMsg_Type_DownloadFile:
            return StaticCast<PlayerMsg_Base>(MakeSmartPointer<PlayerMsg_DownloadFile>());
        case PlayerMsg_Type_DownloadMPD:
            return StaticCast<PlayerMsg_Base>(MakeSmartPointer<PlayerMsg_DownloadMPD>());
        case PlayerMsg_Type_DownloadVideo:
            return StaticCast<PlayerMsg_Base>(MakeSmartPointer<PlayerMsg_DownloadVideo>());
        case PlayerMsg_Type_DownloadAudio:
            return StaticCast<PlayerMsg_Base>(MakeSmartPointer<PlayerMsg_DownloadAudio>());
        case PlayerMsg_Type_DownloadSubtitle:
            return StaticCast<PlayerMsg_Base>(MakeSmartPointer<PlayerMsg_DownloadSubtitle>());
        case PlayerMsg_Type_DownloadFinish:
            return StaticCast<PlayerMsg_Base>(MakeSmartPointer<PlayerMsg_DownloadFinish>());
        case PlayerMsg_Type_GetPlayerStage:
            return StaticCast<PlayerMsg_Base>(MakeSmartPointer<PlayerMsg_GetPlayerStage>());
        case PlayerMsg_Type_ProcessNextSegment:
            return StaticCast<PlayerMsg_Base>(MakeSmartPointer<PlayerMsg_ProcessNextSegment>());
        case PlayerMsg_Type_RefreshMPD:
            return StaticCast<PlayerMsg_Base>(MakeSmartPointer<PlayerMsg_RefreshMPD>());
        case PlayerMsg_Type_UpdateDownloadTime:
            return StaticCast<PlayerMsg_Base>(MakeSmartPointer<PlayerMsg_UpdateDownloadTime>());
        case PlayerMsg_Type_Base:
        default:
            return MakeSmartPointer<PlayerMsg_Base>();
    }
}
