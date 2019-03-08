#ifndef PLAYERMSG_DOWNLOADMPD_H
#define PLAYERMSG_DOWNLOADMPD_H
#include "PlayerMsg_DownloadFile.h"
#include "IMPD.h"
#include "libdash.h"
#include "SmartPointer.h"


class PlayerMsg_DownloadMPD : public PlayerMsg_DownloadFile
{
 public:
    PlayerMsg_DownloadMPD();
    ~PlayerMsg_DownloadMPD();

    // override
    PlayerMsg_Type GetMsgType();
    // override
    std::string GetMsgTypeName();
    void SetMPDFile(dash::mpd::IMPD* mpdFile);
    void SetMPDFile(SmartPointer<dash::mpd::IMPD> mpdFile);
    SmartPointer<dash::mpd::IMPD> GetAndMoveMPDFile();
    SmartPointer<dash::mpd::IMPD> GetMPDFile();
    bool IsMPDFileEmpty();
 private:
    SmartPointer<dash::mpd::IMPD> m_mpdFile;
};

#endif
