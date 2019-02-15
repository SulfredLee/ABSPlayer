#ifndef PALYERMSG_OPEN_H
#define PLAYERMSG_OPEN_H
#include "PlayerMsg_Base.h"

#include <string>

class PlayerMsg_Open : public PlayerMsg_Base
{
 public:
    PlayerMsg_Open();
    ~PlayerMsg_Open();

    // override
    PlayerMsg_Type GetMsgType();
    // override
    std::string GetMsgTypeName();

    void SetURL(std::string url);
    std::string GetURL();
 private:
    std::string m_url;
};

#endif
