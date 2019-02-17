#ifndef PLAYERMSG_REFRESHMPD_H
#define PLAYERMSG_REFRESHMPD_H
#include "PlayerMsg_DownloadFile.h"
#include "IMPD.h"
#include "libdash.h"
#include "SmartPointer.h"


class PlayerMsg_RefreshMPD : public PlayerMsg_DownloadFile
{
 public:
    PlayerMsg_RefreshMPD();
    ~PlayerMsg_RefreshMPD();

    // override
    PlayerMsg_Type GetMsgType();
    // override
    std::string GetMsgTypeName();
    uint64_t GetMinimumUpdatePeriod();

    void SetMPDFile(dash::mpd::IMPD* mpdFile);
    void SetMPDFile(SmartPointer<dash::mpd::IMPD> mpdFile);
    SmartPointer<dash::mpd::IMPD> GetAndMoveMPDFile();
    bool IsMPDFileEmpty();
    void SetMinimumUpdatePeriod(uint64_t minimumUpdatePeriod);
 private:
    SmartPointer<dash::mpd::IMPD> m_mpdFile;
    uint64_t m_minimumUpdatePeriod;
};

#endif
