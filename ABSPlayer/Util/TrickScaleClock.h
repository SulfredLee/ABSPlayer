#ifndef TRICKSCALECLOCK_H
#define TRICKSCALECLOCK_H
#include "LinuxThread.h"
#include "DefaultMutex.h"

#include <stdint.h>
#include <string>

class TrickScaleClock : public LinuxThread
{
 public:
    TrickScaleClock();
    ~TrickScaleClock();
    void InitComponent(uint64_t timeMSec, int32_t trickScale, std::string thisName);

    void SetTimeMSec(uint64_t timeMSec);
    void SetTrickScale(int32_t trickScale);

    uint64_t GetTimeMSec();
    int32_t GetTrickScale();

    // override
    void* Main();
 private:
    uint64_t m_timeMSec;
    int32_t m_trickScale;
    DefaultMutex m_mutex;
};

#endif
