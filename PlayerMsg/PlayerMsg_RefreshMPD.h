#ifndef PLAYERMSG_REFRESHMPD_H
#define PLAYERMSG_REFRESHMPD_H
#include "PlayerMsg_DownloadFile.h"
#include "IMPD.h"
#include "libdash.h"

#include <memory>

class PlayerMsg_RefreshMPD : public PlayerMsg_DownloadFile
{
 public:
    PlayerMsg_RefreshMPD();
    ~PlayerMsg_RefreshMPD();

    // override
    PlayerMsg_Type GetMsgType();
    // override
    std::string GetMsgTypeName();
    void SetMPDFile(dash::mpd::IMPD* mpdFile);
    void SetMPDFile(std::shared_ptr<dash::mpd::IMPD> mpdFile);
    std::shared_ptr<dash::mpd::IMPD> GetAndMoveMPDFile();
    bool IsMPDFileEmpty();
 private:
    std::shared_ptr<dash::mpd::IMPD> m_mpdFile;
};

#endif