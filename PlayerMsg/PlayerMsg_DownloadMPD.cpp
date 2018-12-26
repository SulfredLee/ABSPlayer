#include "PlayerMsg_DownloadMPD.h"

PlayerMsg_DownloadMPD::PlayerMsg_DownloadMPD()
{}

PlayerMsg_DownloadMPD::~PlayerMsg_DownloadMPD()
{}

// override
PlayerMsg_Type PlayerMsg_DownloadMPD::GetMsgType()
{
    return PlayerMsg_Type_DownloadMPD;
}

// override
std::string PlayerMsg_DownloadMPD::GetMsgTypeName()
{
    return "PlayerMsg_Type_DownloadMPD";
}
