#include "PlayerMsg_DownloadAudio.h"

PlayerMsg_DownloadAudio::PlayerMsg_DownloadAudio()
{}

PlayerMsg_DownloadAudio::~PlayerMsg_DownloadAudio()
{}

// override
PlayerMsg_Type PlayerMsg_DownloadAudio::GetMsgType()
{
    return PlayerMsg_Type_DownloadMPD;
}

// override
std::string PlayerMsg_DownloadAudio::GetMsgTypeName()
{
    return "PlayerMsg_Type_DownloadMPD";
}
