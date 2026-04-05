#include "SectorWatchWidget.h"

#include <Engine/Core/World/Sector.h>

using namespace WEngine;

void SectorWatchWidget::Setup()
{
    m_widgetName = "SectorWatch";
}

void SectorWatchWidget::RenderInternal()
{
    SetSize({300, 400});
    ImGui::Text("All loaded sectors (except for root)");

    const auto& sh = Sector::m_root;

    int i = 0;
    for (const auto& sector : sh->m_sectors)
    {
        ImGui::PushID(sector->m_name.c_str());
        ImGui::SeparatorText(sector->m_name.c_str());

        // holy shit string stuff is crazy here!
        if (ImGui::Button((std::string("Hide##") + std::to_string(i)).c_str())) ForceHideSector(*sector); ImGui::SameLine();
        if (ImGui::Button((std::string("Show##") + std::to_string(i)).c_str())) ForceShowSector(*sector); ImGui::SameLine();
        if (ImGui::Button((std::string("Unload##") + std::to_string(i)).c_str())) ForceUnloadSector(*sector);

        if (sector->m_ticking)
            ImGui::Text("Sector is ticking.");
        else
            ImGui::TextColored(ImVec4(194.0 / 255, 56.0 / 255, 41.0 / 255, 255.0 / 255), "Sector is hidden.");

        int entityN = sector->m_entities.size();
        if (entityN == 0)
            ImGui::Text("Sector has no entities.");
        else if (entityN == 1)
            ImGui::Text("Sector has 1 entity.");
        else
            ImGui::Text("Sector has %i entities.", entityN);
        ImGui::PopID();
        i++;
    }
}

void SectorWatchWidget::ForceHideSector(Sector& sector)
{
    sector.m_ticking = false;
}

void SectorWatchWidget::ForceShowSector(Sector& sector)
{
    sector.m_ticking = true;
}

void SectorWatchWidget::ForceUnloadSector(Sector& sector)
{
    sector.Unload();
}
