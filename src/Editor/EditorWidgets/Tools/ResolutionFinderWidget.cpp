#include "ResolutionFinderWidget.h"

#include "Engine/Util/Log.h"

uint16 foundX = 0, foundY = 0;

using namespace WEditor;

void ResolutionFinderWidget::Setup()
{
    m_widgetName = "Resolution Finder";
    m_windowFlags |= ImGuiWindowFlags_NoDocking;
}

void ResolutionFinderWidget::RenderInternal()
{
    SetSize(WEngine::Vector2(200, 450));

    if (ImGui::BeginTable("worldSelectTable", 2, ImGuiTableFlags_Borders))
    {
        ImGui::TableSetupColumn("Support?");
        ImGui::TableSetupColumn("Resolution");
        ImGui::TableHeadersRow();

        for (auto& resolution : m_resolutions)
        {
            ImGui::TableNextRow();
            ImGui::PushID(&resolution);
            ImGui::TableNextColumn();
            ImGui::Checkbox("", &resolution.selected);

            ImGui::TableNextColumn();
            ImGui::Text("%ix%i", resolution.width, resolution.height);

            ImGui::PopID();
        }
        ImGui::EndTable();
    }

    if (ImGui::Button("Find Next"))
    {
        FindNextResolution();
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset"))
    {
        foundX = 0;
        foundY = 0;
    }
    ImGui::Text("%ix%i", foundX, foundY);
}

void ResolutionFinderWidget::FindNextResolution()
{
    bool found = false;
    while (found == false)
    {
        if (foundX > 2160)
            return;
        found = true;
        foundX += 16;
        foundY += 9;

        for (const auto& resolution : m_resolutions)
        {
            if (resolution.selected)
            {
                float32 xFit = resolution.width % foundX;
                float32 yFit = resolution.height % foundY;
                if (xFit != 0 || yFit != 0)
                {
                    found = false;
                }
            }
        }
    }
}
