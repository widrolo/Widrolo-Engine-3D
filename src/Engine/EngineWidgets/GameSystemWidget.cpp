#include "GameSystemWidget.h"

#include <Engine/Types/CoreSystems.h>
#include <Engine/Core/Handlers/WidgetHandler.h>

void WEngine::GameSystemWidget::Setup()
{
    m_widgetName = "Game System";
    m_windowFlags |= ImGuiWindowFlags_NoTitleBar;
    m_windowFlags |= ImGuiWindowFlags_NoMove;
    m_windowFlags |= ImGuiWindowFlags_NoResize;
    m_windowFlags |= ImGuiWindowFlags_NoCollapse;
    m_windowFlags |= ImGuiWindowFlags_NoNav;
}

void WEngine::GameSystemWidget::RenderInternal()
{
    SetPosition({15, 650});
    SetSize({300, 350});
    ImGui::Text("Press F2 to Enable/Disable Game Widget");
    ImGui::Separator();

    const auto wh = CoreSystems::GetWidgetHandler();

    int i = 0;

    for (auto& weak : wh->m_gameWidgets)
    {
        if (auto widget = weak.lock())
        {
            ImGui::PushID(i);
            ImGui::Checkbox("", &widget->m_open);
            ImGui::SameLine();
            ImGui::Text("%s", widget->m_widgetName.c_str());
            ImGui::PopID();
            i++;
        }
    }
}
