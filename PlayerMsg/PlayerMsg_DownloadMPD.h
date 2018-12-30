#ifndef PLAYERMSG_DOWNLOADMPD_H
#define PLAYERMSG_DOWNLOADMPD_H
#include "PlayerMsg_DownloadFile.h"
#include "IMPD.h"
#include "libdash.h"

#include <memory>

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
    void SetMPDFile(std::shared_ptr<dash::mpd::IMPD> mpdFile);
    std::shared_ptr<dash::mpd::IMPD> GetAndMoveMPDFile();
    bool IsMPDFileEmpty();
 private:
    std::shared_ptr<dash::mpd::IMPD> m_mpdFile;
};

#endif
