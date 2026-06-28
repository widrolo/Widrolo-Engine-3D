#pragma once
#include "Day.h"
#include "Month.h"
#include "Engine/Types/CommonTypes.h"

namespace WEngine
{
    // Well, in this case we want to have a baseline. Setting to year 0 would be dumb, wouldnt it?
    constexpr uint32 realisticBaseYear = 1900;
    class Date
    {
    public:
        Date() : m_year(realisticBaseYear), m_month(0), m_day(0) {}
        Date(uint32 year, uint32 month, uint32 day) : m_year(year), m_month(month), m_day(day) {}
        Date(uint32 year, Month month, Day day) : m_year(year), m_month((uint8)month), m_day((uint8)day) {}

    public:
        void SetYear(uint32 year);
        void SetMonth(uint32 month);
        void SetMonth(Month month);
        void SetDay(uint32 day);
        void SetDay(Day day);

        void AddYears(uint32 years);
        void AddMonths(uint32 months);
        void AddDays(uint32 days);

        uint32 DaysInMonth(uint32 month);
        uint32 DaysInMonth(Month month);

        [[nodiscard]] uint32 GetYear() const;
        [[nodiscard]] uint32 GetMonth() const;
        [[nodiscard]] uint32 GetDay() const;

    private:
        uint32 m_year;
        uint32 m_month;
        uint32 m_day;
    };
}