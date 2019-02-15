#ifndef PLAYER_MSG_Q_H
#define PLAYER_MSG_Q_H
#include "DefaultMutex.h"
#include "PlayerMsg_Base.h"

#include <queue>
#include <memory>

class PlayerMsgQ
{
 public:
    PlayerMsgQ();
    ~PlayerMsgQ();

    void InitComponent(size_t totalMsgSizeLimit);
    bool AddMsg(std::shared_ptr<PlayerMsg_Base> msg);
    void GetMsg(std::shared_ptr<PlayerMsg_Base>& msg);
    int GetMsgNum();
    int GetTotalMsgSize();
 private:
    DefaultMutex m_mutex;
    pthread_cond_t m_cond;
    size_t m_totalMsgSizeLimit;
    int m_totalMsgSize;
    std::queue<std::shared_ptr<PlayerMsg_Base> > m_msgQ;
};

#endif
