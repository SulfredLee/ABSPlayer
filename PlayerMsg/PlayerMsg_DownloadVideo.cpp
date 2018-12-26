#include "PlayerMsg_DownloadVideo.h"

PlayerMsg_DownloadVideo::PlayerMsg_DownloadVideo()
{}

PlayerMsg_DownloadVideo::~PlayerMsg_DownloadVideo()
{}

// override
PlayerMsg_Type PlayerMsg_DownloadVideo::GetMsgType()
{
    return PlayerMsg_Type_DownloadVideo;
}

// override
std::string PlayerMsg_DownloadVideo::GetMsgTypeName()
{
    return "PlayerMsg_Type_DownloadVideo";
}
