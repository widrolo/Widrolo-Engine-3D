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

    public:
        void SetYear(uint32 year);
        void SetMonth(uint32 month);
        void SetMonth(Month month);
        void SetDay(uint32 day);
        void SetDay(Day day);

    private:
        uint32 m_year;
        uint32 m_month;
        uint32 m_day;
    };
}