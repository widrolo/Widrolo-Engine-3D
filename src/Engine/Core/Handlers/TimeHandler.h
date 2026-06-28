#pragma once
#include "Engine/Math/Vector.h"
#include "Engine/Types/CommonTypes.h"
#include "Engine/Types/Time/Date.h"
#include "Engine/Types/Time/Time.h"

namespace WEngine
{
    class TimeHandler
    {
    public:
        TimeHandler();

    public:
        void Update(float32 dt);

        void SetDate(const Date& date);
        void SetTime(const Time& time);

        [[nodiscard]] Date GetDate() const;
        [[nodiscard]] Time GetTime() const;

    private:
        void UpdateRenderTime();
        Vector3 CalcSunDir(float32 timeFactor);

    private:
        Date m_date;
        Time m_time;
        float32 m_accumulator;
    };
}
