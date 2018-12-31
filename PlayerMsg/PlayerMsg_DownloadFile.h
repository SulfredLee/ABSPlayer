#ifndef PLAYERMSG_DOWNLOADFILE_H
#define PLAYERMSG_DOWNLOADFILE_H
#include "PlayerMsg_Base.h"
#include <vector>

class PlayerMsg_DownloadFile : public PlayerMsg_Base
{
 public:
    PlayerMsg_DownloadFile();
    virtual ~PlayerMsg_DownloadFile();

    // override
    virtual PlayerMsg_Type GetMsgType();
    // override
    virtual std::string GetMsgTypeName();
    // override
    virtual size_t GetMsgSize(); // only report message size when message has media data

    void SetURL(std::string url);
    void SetFile(unsigned char* data, size_t dataLength);
    void ClearFile();
    void SetDownloadTime(uint64_t downloadTime);

    std::string GetURL();
    std::vector<unsigned char> GetFile();
    size_t GetFileLength();
    uint64_t GetDownloadTime();
 private:
    std::string m_url;
    std::vector<unsigned char> m_file;
    uint64_t m_downloadTime;
};

#endif
