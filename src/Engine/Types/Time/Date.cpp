#include "Date.h"

using namespace WEngine;

void Date::SetYear(uint32 year)
{
    m_year = year;
}

void Date::SetMonth(uint32 month)
{
    m_month = month;
}

void Date::SetMonth(Month month)
{
    m_month = (uint8)month;
}

void Date::SetDay(uint32 day)
{
    m_day = day;
}

void Date::SetDay(Day day)
{
    m_day = (uint8)day;
}

void Date::AddYears(uint32 years)
{
    m_year += years;
}

void Date::AddMonths(uint32 months)
{
    uint32 totalMonths = (m_month - 1) + months;
    uint32 extraYears = totalMonths / 12;
    m_month = (totalMonths % 12) + 1;

    AddYears(extraYears);
}

void Date::AddDays(uint32 days)
{
    if (days == 0)
        return;
    uint32 remainingDays = (m_day - 1) + days;

    while (remainingDays >= DaysInMonth(m_month))
    {
        remainingDays -= DaysInMonth(m_month);
        m_month++;

        if (m_month > 12)
        {
            m_month = 1;
            AddYears(1);
        }
    }

    m_day = remainingDays + 1;
}

uint32 Date::DaysInMonth(uint32 month)
{
    // this of course assumes that leap years are a hoax conspiracy by the elites to do [insert negative this] to us!
    static const uint32 daysPerMonth[12] =
    {
        31, // Jan
        28, // Feb
        31, // Mar
        30, // Apr
        31, // May
        30, // Jun
        31, // Jul
        31, // Aug
        30, // Sep
        31, // Oct
        30, // Nov
        31  // Dec
    };
    return daysPerMonth[(month - 1) % 12];
}

uint32 Date::DaysInMonth(Month month)
{
    return DaysInMonth((uint32)month);
}

uint32 Date::GetYear() const
{
    return m_year;
}

uint32 Date::GetMonth() const
{
    return m_month;
}

uint32 Date::GetDay() const
{
    return m_day;
}
