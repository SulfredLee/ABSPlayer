#ifndef MSGCOUNTER_H
#define MSGCOUNTER_H
#include "LinuxThread.h"
#include "DefaultMutex.h"
#include "PlayerMsg_Common.h"

#include <stdint.h>
#include <memory>
#include <map>
#include <string>

class MsgCounter : public LinuxThread
{
 public:
    MsgCounter();
    ~MsgCounter();

    void InitComponent(std::string thisName);
    void AddCount(std::shared_ptr<PlayerMsg_Base> msg);
    // override
    void* Main();
 private:
    DefaultMutex m_mutex;
    std::map<std::string, uint32_t> m_counterDB;
};

#endif
