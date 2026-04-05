#include "EngineControlWidget.h"

#include <Engine/Types/CoreSystems.h>

using namespace WEngine;

void EngineControlWidget::Setup()
{
    m_widgetName = "Engine Control";
    m_windowFlags |= ImGuiWindowFlags_NoTitleBar;
    m_windowFlags |= ImGuiWindowFlags_NoMove;
    m_windowFlags |= ImGuiWindowFlags_NoResize;
    m_windowFlags |= ImGuiWindowFlags_NoCollapse;
    m_windowFlags |= ImGuiWindowFlags_NoNav;
}

void EngineControlWidget::RenderInternal()
{
    SetPosition({15, 100});
    SetSize({300, 150});

    ImGui::Text("Engine Control");
    ImGui::Text("Press F12 to Enable/Disable Engine Widgets");
    ImGui::Separator();

    if (ImGui::Button("Shut Down", {138, 45}))
        CoreSystems::ShutdownGame();

    ImGui::SameLine();

    // This will be important for bug testing later on
    if (ImGui::Button("Dump Game State", {138, 45}))
        ;

    float32 timeScale = CoreSystems::GetTimeScale();
    ImGui::DragFloat("Time Scale", &timeScale, 0.1f, 0.0f, 3.0f, "%.1f");
    CoreSystems::SetTimeScale(timeScale);

}
