#include "Timer.h"

#include <ctime>

using namespace WEngine;

TimePoint Timer::GetLocalTime()
{
    auto now = std::chrono::system_clock::now();
    auto chronoTime = std::chrono::system_clock::to_time_t(now);
    auto time = std::localtime(&chronoTime);

    TimePoint timePoint{};
    timePoint.Second = time->tm_sec;
    timePoint.Minute = time->tm_min;
    timePoint.Hour = time->tm_hour;
    timePoint.Day = time->tm_mday;
    timePoint.Month = time->tm_mon;
    timePoint.Year = time->tm_year + 1900;

    return timePoint;
}

void StopWatch::Reset()
{
    m_time = std::chrono::high_resolution_clock::now();
}

template<>
float64 StopWatch::GetTime<TimeUnit::Millennia>() const
{
    return GetTime<TimeUnit::Years>() * (1.0 / 1000.0);
}
template<>
float64 StopWatch::GetTime<TimeUnit::Centuries>() const
{
    return GetTime<TimeUnit::Years>() * (1.0 / 100.0);
}
template<>
float64 StopWatch::GetTime<TimeUnit::Decades>() const
{
    return GetTime<TimeUnit::Years>() * (1.0 / 10.0);
}
template<>
float64 StopWatch::GetTime<TimeUnit::Years>() const
{
    return GetTime<TimeUnit::Days>() * (1.0 / 365.25);
}
template<>
float64 StopWatch::GetTime<TimeUnit::Months>() const
{
    // Its not really a gregorian calendar type of month, rather
    // a 12th of the year thing.
    // Chrono actually does have a months option, but we dont
    // want the gregorian calendar here.
    return GetTime<TimeUnit::Days>() * (1.0 / (365.25 / 12.0));
}
template<>
float64 StopWatch::GetTime<TimeUnit::Weeks>() const
{
    return GetTime<TimeUnit::Days>() * (1.0 / 7.0);
}
template<>
float64 StopWatch::GetTime<TimeUnit::Days>() const
{
    return GetTime<TimeUnit::Hours>() * (1.0 / 24.0);
}
template<>
float64 StopWatch::GetTime<TimeUnit::Hours>() const
{
    return GetTime<TimeUnit::Minutes>() * (1.0 / 60.0);
}
template<>
float64 StopWatch::GetTime<TimeUnit::Minutes>() const
{
    // We will be using seconds as a base unit from now on, we wont
    // need more accuracy beyond this.
    return GetTime<TimeUnit::Seconds>() * (1.0 / 60.0);
}
template<>
float64 StopWatch::GetTime<TimeUnit::Seconds>() const
{
    return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - m_time).count();
}
template<>
float64 StopWatch::GetTime<TimeUnit::Milliseconds>() const
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - m_time).count();
}

template<>
float64 StopWatch::GetTime<TimeUnit::Microseconds>() const
{
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - m_time).count();
}