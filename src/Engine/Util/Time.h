#pragma once

#include <Engine/Types/CommonTypes.h>
#include <chrono>

namespace WEngine
{
    /**
     * Represents a time unit used for measuring elapsed time.
     */
    enum class TimeUnit
    {
        // gimmick
        Millennia,      //!< A period of 1000 years
        Centuries,      //!< A period of 100 years
        Decades,        //!< A period of 10 years
        Years,          //!< A calendar year (365.25 days)
        Months,         //!< A month as a fraction of a year (365.25 / 12 days)
        Weeks,          //!< A week (7 days)
        Days,           //!< A day (24 hours)
        Hours,          //!< An hour (60 minutes)
        Minutes,        //!< A minute (60 seconds)
        Seconds,        //!< A second
        Milliseconds,   //!< A millisecond (1/1000 of a second)
        Microseconds    //!< A microsecond (1/1000000 of a second)
    };


    /**
     * Represents a specific moment in time.
     */
    struct TimePoint {
        uint16 Year;            //!< The year (e.g., 2022)
        uint16 Month;           //!< The month (1-12)
        uint16 DayOfWeek;       //!< The day of the week (0-6, where 0 is Sunday)
        uint16 Day;             //!< The day of the month (1-31)
        uint16 Hour;            //!< The hour of the day (0-23)
        uint16 Minute;          //!< The minute of the hour (0-59)
        uint16 Second;          //!< The second of the minute (0-59)
        uint16 Milliseconds;    //!< The millisecond of the second (0-999)
    };

    // The stopwatch was inspired by The Cherno
    // https://gist.github.com/TheCherno/b2c71c9291a4a1a29c889e76173c8d14
    /**
     * A stopwatch used for measuring elapsed time.
     */
    class StopWatch
    {
    public:
        /**
         * Resets the stopwatch, setting the start time to the current time point.
         */
        void Reset();
        /**
         * Retrieves the elapsed time in the specified time unit since the last reset of the stopwatch.
         * @tparam TimeUnit The time unit for measuring the elapsed time.
         * @return The elapsed time in the specified time unit.
         */
        template<TimeUnit TimeUnit> [[nodiscard]] float64 GetTime() const;
    private:
        std::chrono::time_point<std::chrono::high_resolution_clock> m_time;
    };

    /**
     * Retrieves the elapsed time in millennia since the last reset of the stopwatch.
     * @return The elapsed time in millennia.
     */
    template<> float64 StopWatch::GetTime<TimeUnit::Millennia>() const;

    /**
     * Retrieves the elapsed time in centuries since the last reset of the stopwatch.
     * @return The elapsed time in centuries.
     */
    template<> float64 StopWatch::GetTime<TimeUnit::Centuries>() const;

    /**
     * Retrieves the elapsed time in decades since the last reset of the stopwatch.
     * @return The elapsed time in decades.
     */
    template<> float64 StopWatch::GetTime<TimeUnit::Decades>() const;

    /**
     * Retrieves the elapsed time in years since the last reset of the stopwatch.
     * @return The elapsed time in years.
     */
    template<> float64 StopWatch::GetTime<TimeUnit::Years>() const;

    /**
     * Retrieves the elapsed time in months (as a fraction of a year) since the last reset of the stopwatch.
     * @return The elapsed time in months.
     */
    template<> float64 StopWatch::GetTime<TimeUnit::Months>() const;

    /**
     * Retrieves the elapsed time in weeks since the last reset of the stopwatch.
     * @return The elapsed time in weeks.
     */
    template<> float64 StopWatch::GetTime<TimeUnit::Weeks>() const;

    /**
     * Retrieves the elapsed time in days since the last reset of the stopwatch.
     * @return The elapsed time in days.
     */
    template<> float64 StopWatch::GetTime<TimeUnit::Days>() const;

    /**
     * Retrieves the elapsed time in hours since the last reset of the stopwatch.
     * @return The elapsed time in hours.
     */
    template<> float64 StopWatch::GetTime<TimeUnit::Hours>() const;

    /**
     * Retrieves the elapsed time in minutes since the last reset of the stopwatch.
     * @return The elapsed time in minutes.
     */
    template<> float64 StopWatch::GetTime<TimeUnit::Minutes>() const;

    /**
     * Retrieves the elapsed time in seconds since the last reset of the stopwatch.
     * @return The elapsed time in seconds.
     */
    template<> float64 StopWatch::GetTime<TimeUnit::Seconds>() const;

    /**
     * Retrieves the elapsed time in milliseconds since the last reset of the stopwatch.
     * @return The elapsed time in milliseconds.
     */
    template<> float64 StopWatch::GetTime<TimeUnit::Milliseconds>() const;

    /**
     * Retrieves the elapsed time in microseconds since the last reset of the stopwatch.
     * @return The elapsed time in microseconds.
     */
    template<> float64 StopWatch::GetTime<TimeUnit::Microseconds>() const;

    /**
     * A utility class for working with time-related functions.
     */
    class Time
    {
    public:
        /**
         * Retrieves the current local time.
         * @return The current local time as a TimePoint struct.
         */
        static TimePoint GetLocalTime();
    };
}

