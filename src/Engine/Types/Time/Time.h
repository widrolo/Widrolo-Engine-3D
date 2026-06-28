#pragma once
#include "Engine/Types/CommonTypes.h"

namespace WEngine
{
    class Time
    {
    public:
        Time() : m_hour(0), m_minutes(0), m_seconds(0) {}
        Time(uint32 hour, uint32 minutes, uint32 seconds)
            : m_hour(hour),
              m_minutes(minutes),
              m_seconds(seconds) {}

        void SetHour(uint32 hour);
        void SetMinute(uint32 minute);
        void SetSecond(uint32 second);

        /**
         *
         * @param hours Number of hours to advance.
         * @return Number of days to advance as a result.
         */
        uint32 AddHours(uint32 hours);

        /**
         *
         * @param minutes Number of minutes to advance.
         * @return Number of days to advance as a result.
         */
        uint32 AddMinutes(uint32 minutes);

        /**
         *
         * @param seconds Number of seconds to advance.
         * @return Number of days to advance as a result.
         */
        uint32 AddSeconds(uint32 seconds);

        [[nodiscard]] uint32 GetHours();
        [[nodiscard]] uint32 GetMinutes();
        [[nodiscard]] uint32 GetSeconds();

    private:
        uint32 m_hour;
        uint32 m_minutes;
        uint32 m_seconds;
    };
}
