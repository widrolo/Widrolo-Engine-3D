#include "Time.h"

using namespace WEngine;

void Time::SetHour(uint32 hour)
{
    m_hour = hour;
}

void Time::SetMinute(uint32 minute)
{
    m_minutes = minute;
}

void Time::SetSecond(uint32 second)
{
    m_seconds = second;
}

uint32 Time::AddHours(uint32 hours)
{
    uint32 totalHours = m_hour + hours;
    uint32 days = totalHours / 24;
    m_hour = totalHours % 24;
    return days;
}

uint32 Time::AddMinutes(uint32 minutes)
{
    uint32 totalMinutes = m_minutes + minutes;
    uint32 extraHours = totalMinutes / 60;
    m_minutes = totalMinutes % 60;

    return AddHours(extraHours);
}

uint32 Time::AddSeconds(uint32 seconds)
{
    uint32 totalSeconds = m_seconds + seconds;
    uint32 extraMinutes = totalSeconds / 60;
    m_seconds = totalSeconds % 60;

    return AddMinutes(extraMinutes);
}

uint32 Time::GetHours() const
{
    return m_hour;
}

uint32 Time::GetMinutes() const
{
    return m_minutes;
}

uint32 Time::GetSeconds() const
{
    return m_seconds;
}

