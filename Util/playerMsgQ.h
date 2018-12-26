#ifndef PLAYER_MSG_Q_H
#define PLAYER_MSG_Q_H
#include "DefaultMutex.h"
#include "PlayerMsg_Base.h"

#include <queue>

class playerMsgQ
{
 public:
    playerMsgQ();
    ~playerMsgQ();

    void InitComponent(int totalMsgSizeLimit);
    bool AddMsg(PlayerMsg_Base*& msg);
    void GetMsg(PlayerMsg_Base*& msg);
    int GetMsgNum();
    int GetTotalMsgSize();
 private:
    DefaultMutex m_mutex;
    pthread_cond_t m_cond;
    int m_totalMsgSizeLimit;
    int m_totalMsgSize;
    std::queue<PlayerMsg_Base*> m_msgQ;
};

#endif
