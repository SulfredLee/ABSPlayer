#include "PlayerMsg_RefreshMPD.h"

PlayerMsg_RefreshMPD::PlayerMsg_RefreshMPD()
{}

PlayerMsg_RefreshMPD::~PlayerMsg_RefreshMPD()
{}

// override
PlayerMsg_Type PlayerMsg_RefreshMPD::GetMsgType()
{
    return PlayerMsg_Type_RefreshMPD;
}

// override
std::string PlayerMsg_RefreshMPD::GetMsgTypeName()
{
    return "PlayerMsg_Type_RefreshMPD";
}

void PlayerMsg_RefreshMPD::SetMPDFile(dash::mpd::IMPD* mpdFile)
{
    m_mpdFile = std::shared_ptr<dash::mpd::IMPD>(mpdFile);
}

void PlayerMsg_RefreshMPD::SetMPDFile(std::shared_ptr<dash::mpd::IMPD> mpdFile)
{
    m_mpdFile = mpdFile;
}

std::shared_ptr<dash::mpd::IMPD> PlayerMsg_RefreshMPD::GetAndMoveMPDFile()
{
    return std::move(m_mpdFile);
}

bool PlayerMsg_RefreshMPD::IsMPDFileEmpty()
{
    return m_mpdFile == nullptr ? true : false;
}
