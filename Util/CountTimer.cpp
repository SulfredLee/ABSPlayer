#include "CountTimer.h"

CountTimer::CountTimer()
{}

CountTimer::~CountTimer()
{}

void CountTimer::Start()
{
    m_start = std::chrono::system_clock::now();
}

void CountTimer::Stop()
{
    m_end = std::chrono::system_clock::now();
    m_elapsed_seconds = m_end - m_start;
}

double CountTimer::GetSecondDouble() const
{
    return m_elapsed_seconds.count();
}

uint64_t CountTimer::GetSecond() const
{
    return static_cast<uint64_t>(m_elapsed_seconds.count() + 0.5);
}

uint64_t CountTimer::GetMSecond() const
{
    return static_cast<uint64_t>(m_elapsed_seconds.count() * 1000 + 0.5);
}

uint64_t CountTimer::GetNSecond() const
{
    return static_cast<uint64_t>(m_elapsed_seconds.count() * 1000 * 1000 + 0.5);
}

std::string CountTimer::GetTimeString() const
{
    std::time_t end_time = std::chrono::system_clock::to_time_t(m_end);
    return std::ctime(&end_time);
}
