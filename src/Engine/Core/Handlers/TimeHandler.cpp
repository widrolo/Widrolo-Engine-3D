#include "TimeHandler.h"

#include <cmath>

#include "RenderHandler.h"
#include "Engine/EngineDefines.h"
#include "Engine/Types/CoreSystems.h"
#include "Engine/Util/Log.h"

using namespace WEngine;

TimeHandler::TimeHandler()
{
    m_date = Date(TimeSettings::startYear, TimeSettings::startMonth, TimeSettings::startDay);
    m_time = Time(TimeSettings::startHour, TimeSettings::startMinute, TimeSettings::startSecond);
    m_accumulator = 0.0f;
}

void TimeHandler::Update(float32 dt)
{
    m_accumulator += dt * TimeSettings::gameSecondPerRealSecond;
    uint32 secs = std::floor(m_accumulator);
    m_accumulator -= (float32)secs;
    uint32 days = m_time.AddSeconds(secs);
    m_date.AddDays(days);
    UpdateRenderTime();
}

void TimeHandler::SetDate(const Date &date)
{
    m_date = date;
}

void TimeHandler::SetTime(const Time &time)
{
    m_time = time;
}

Date TimeHandler::GetDate() const
{
    return m_date;
}

Time TimeHandler::GetTime() const
{
    return m_time;
}

void TimeHandler::UpdateRenderTime()
{
    constexpr uint32 secondsInDay = 60 * 60 * 24;

    uint32 secs = m_time.GetSeconds();
    secs += m_time.GetMinutes() * 60;
    secs += m_time.GetHours() * 60 * 60;

    float32 timeFactor = (float32)secs / (float32)secondsInDay;
    CoreSystems::GetRenderHandler()->SetLightTime(timeFactor);

    auto sun = CoreSystems::GetRenderHandler()->GetSunlight();
    sun.direction = CalcSunDir(timeFactor);
    sun.direction.y = -sun.direction.y;

    CoreSystems::GetRenderHandler()->SetSunlight(sun);

    float32 colorFactor = std::max(-std::pow(timeFactor - 0.533, 4) * 110 + 1, 0.0);

    CoreSystems::GetRenderHandler()->SetSunlightColorFactor(colorFactor);

    WLog::ConsoleLog(std::format("{}", colorFactor));

}

Vector3 TimeHandler::CalcSunDir(float32 timeFactor)
{
    constexpr float32 axialTilt = 23.5f * (std::numbers::pi / 180.0f);
    const Vector3 axis = Vector3(cosf(axialTilt), sinf(axialTilt), 0.0f);

    const Vector3 start = Vector3(0.0f, 0.0f, 1.0f);

    const float32 phaseOffset = std::numbers::pi * 0.5f;
    const float32 theta = timeFactor * 2.0f * std::numbers::pi + phaseOffset;
    const float32 cosT  = cosf(theta);
    const float32 sinT  = sinf(theta);

    const float32 dot = VecMath::Dot(axis, start);
    const Vector3 cross = VecMath::Cross(axis, start);

    return start * cosT + cross * sinT + axis * dot * (1.0f - cosT);
}
