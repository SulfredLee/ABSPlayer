#ifndef COUNTTIMER_H
#define COUNTTIMER_H
#include <chrono>
#include <ctime>
#include <string>

class CountTimer
{
 public:
    CountTimer();
    ~CountTimer();

    void Start();
    void Stop();

    double GetSecondDouble() const;
    uint64_t GetSecond() const;
    uint64_t GetMSecond() const;
    uint64_t GetNSecond() const;
    std::string GetTimeString() const;
 private:
    std::chrono::time_point<std::chrono::system_clock> m_start;
    std::chrono::time_point<std::chrono::system_clock> m_end;
    std::chrono::duration<double> m_elapsed_seconds;
};

#endif
