#include "SystemWidget.h"

#include <Engine/Types/CoreSystems.h>
#include <Engine/Core/Handlers/WidgetHandler.h>

void WEngine::SystemWidget::Setup()
{
    m_widgetName = "System";
    m_windowFlags |= ImGuiWindowFlags_NoTitleBar;
    m_windowFlags |= ImGuiWindowFlags_NoMove;
    m_windowFlags |= ImGuiWindowFlags_NoResize;
    m_windowFlags |= ImGuiWindowFlags_NoCollapse;
    m_windowFlags |= ImGuiWindowFlags_NoNav;
}

void WEngine::SystemWidget::RenderInternal()
{
    SetPosition({15, 275});
    SetSize({300, 350});
    ImGui::Text("Press F1 to Enable/Disable System Widget");
    ImGui::Separator();

    const auto wh = CoreSystems::GetWidgetHandler();

    // it starts at 4 because some widgets like statistics or engine control should not be here
    for (int i = 4; i < (uint16)SysWidgetTypes::SysWidget_Count; i++)
    {
        ImGui::PushID(i);
        ImGui::Checkbox("", &wh->m_systemWidgets[i]->m_open);
        ImGui::SameLine();
        ImGui::Text("%s", wh->m_systemWidgets[i]->m_widgetName.c_str());
        ImGui::PopID();
    }
}
