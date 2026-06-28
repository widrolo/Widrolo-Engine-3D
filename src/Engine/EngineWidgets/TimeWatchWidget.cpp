#include "TimeWatchWidget.h"

#include "Engine/Core/Handlers/TimeHandler.h"
#include "Engine/Types/CoreSystems.h"
#include "Engine/Types/Time/Date.h"

using namespace WEngine;

void TimeWatchWidget::Setup()
{
    m_widgetName = "Time Watch";
}

void TimeWatchWidget::RenderInternal()
{
    SetSize({300, 200});

    Date date = CoreSystems::GetTimeHandler()->GetDate();
    Time time = CoreSystems::GetTimeHandler()->GetTime();

    std::string dateStr = std::format("Date: {}/{}/{}", date.GetYear(), date.GetMonth(), date.GetDay());
    std::string timeStr = std::format("Time: {}:{}:{}", time.GetHours(), time.GetMinutes(), time.GetSeconds());

    ImGui::Text("%s", dateStr.c_str());
    ImGui::Text("%s", timeStr.c_str());
}
