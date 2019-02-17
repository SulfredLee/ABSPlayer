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
    m_mpdFile = SmartPointer<dash::mpd::IMPD>(mpdFile);
}

void PlayerMsg_RefreshMPD::SetMPDFile(SmartPointer<dash::mpd::IMPD> mpdFile)
{
    m_mpdFile = mpdFile;
}

SmartPointer<dash::mpd::IMPD> PlayerMsg_RefreshMPD::GetAndMoveMPDFile()
{
    return std::move(m_mpdFile);
}

bool PlayerMsg_RefreshMPD::IsMPDFileEmpty()
{
    return m_mpdFile.Get() == nullptr ? true : false;
}

void PlayerMsg_RefreshMPD::SetMinimumUpdatePeriod(uint64_t minimumUpdatePeriod)
{
    m_minimumUpdatePeriod = minimumUpdatePeriod;
}

uint64_t PlayerMsg_RefreshMPD::GetMinimumUpdatePeriod()
{
    return m_minimumUpdatePeriod;
}
