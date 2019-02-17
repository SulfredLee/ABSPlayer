#ifndef MSGCOUNTER_H
#define MSGCOUNTER_H
#include "LinuxThread.h"
#include "DefaultMutex.h"
#include "PlayerMsg_Common.h"
#include "SmartPointer.h"

#include <stdint.h>
#include <map>
#include <string>

class MsgCounter : public LinuxThread
{
 public:
    MsgCounter();
    ~MsgCounter();

    void InitComponent(std::string thisName);
    void AddCount(SmartPointer<PlayerMsg_Base> msg);
    // override
    void* Main();
 private:
    DefaultMutex m_mutex;
    std::map<std::string, uint32_t> m_counterDB;
};

#endif
