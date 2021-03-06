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

void PlayerMsg_DownloadMPD::SetMPDFile(dash::mpd::IMPD* mpdFile)
{
    m_mpdFile = SmartPointer<dash::mpd::IMPD>(mpdFile);
}

void PlayerMsg_DownloadMPD::SetMPDFile(SmartPointer<dash::mpd::IMPD> mpdFile)
{
    m_mpdFile = mpdFile;
}

SmartPointer<dash::mpd::IMPD> PlayerMsg_DownloadMPD::GetAndMoveMPDFile()
{
    return std::move(m_mpdFile);
}

SmartPointer<dash::mpd::IMPD> PlayerMsg_DownloadMPD::GetMPDFile()
{
    return m_mpdFile;
}

bool PlayerMsg_DownloadMPD::IsMPDFileEmpty()
{
    return m_mpdFile.Get() == nullptr ? true : false;
}
