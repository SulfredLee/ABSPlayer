#include "PlayerMsg_DownloadSubtitle.h"

PlayerMsg_DownloadSubtitle::PlayerMsg_DownloadSubtitle()
{}

PlayerMsg_DownloadSubtitle::~PlayerMsg_DownloadSubtitle()
{}

// override
PlayerMsg_Type PlayerMsg_DownloadSubtitle::GetMsgType()
{
    return PlayerMsg_Type_DownloadMPD;
}

// override
std::string PlayerMsg_DownloadSubtitle::GetMsgTypeName()
{
    return "PlayerMsg_Type_DownloadMPD";
}
