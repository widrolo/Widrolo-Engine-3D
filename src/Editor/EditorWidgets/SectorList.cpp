#include "SectorList.h"

#include "Editor/Types/EditorState.h"
#include <Engine/Core/World/Sector.h>
#include <Engine/Core/World/Entity.h>
#include <Engine/Components/Component.h>

#include <Editor/Types/EditorSystems.h>
#include <Engine/EngineDefines.h>
#include <Engine/Core/Handlers/AssetRepo.h>

#include "Editor/Types/ComponentSettingDefinition.h"
#include "Engine/Types/Rendering/Color.h"
#include "Engine/Util/Log.h"
#include <Editor/Core/Handlers/CompSettingsRepo.h>

#include "Editor/Components/AnyComponent.h"

using namespace WEditor;

void SectorList::Setup()
{
    m_widgetName = "Loaded Sectors";
}

void SectorList::RenderInternal()
{
    ImGui::SetWindowSize({ 250, 200 }, 0);

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(20.f, 10.f));
    ImGui::InputText("##", m_sectorName, 128, ImGuiInputTextFlags_ElideLeft);
    ImGui::SameLine();
    if (ImGui::Button("Load"))
        LoadSector();
    ImGui::PopStyleVar();

    ImGui::Separator();

    if (ImGui::TreeNodeEx("Sector List"))
    {
        ShowSectorList();
        ImGui::TreePop();
    }
}

void SectorList::ShowSectorList()
{
    auto sectorList = WEngine::Sector::m_root->m_sectors;

    int iter = 0;

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(20.f, 10.f));
    for (auto sector : sectorList)
    {
        if (sector->m_name == "viewport")
            continue;

        ImGui::SeparatorText(sector->m_name.c_str());
        std::string selText = "Select##" + std::to_string(iter);
        std::string logicText = "Logic##" + std::to_string(iter);
        std::string unloadText = "Unload##" + std::to_string(iter);
        std::string saveText = "Save";
        if (sector->m_changedInEditor)
            saveText += "*";
        saveText += "##" + std::to_string(iter);

        if (ImGui::Button(selText.c_str()))
        {
            EditorState::SelectedSector = sector;
            EditorState::SelectedEntity = nullptr;
            EditorState::SelectedComponent = nullptr;
        }

        ImGui::SameLine();
        ImGui::Button(logicText.c_str());
        ImGui::SameLine();
        if (ImGui::Button(unloadText.c_str()))
        {
            if (sector == EditorState::SelectedSector)
            {
                EditorState::SelectedSector = nullptr;
                EditorState::SelectedEntity = nullptr;
                EditorState::SelectedComponent = nullptr;
            }
            sector->RequestUnload();
            sector->SectorInternalTick();
        }
        ImGui::SameLine();
        if (ImGui::Button(saveText.c_str()))
        {
            sector->m_changedInEditor = false;
            SaveSectorToYaml(sector);
        }

        iter++;

    }
    ImGui::PopStyleVar();
}

void SectorList::LoadSector()
{
    WEngine::Sector* sector = new WEngine::Sector(m_sectorName);
    WEngine::Sector::m_root->m_sectors.push_back(sector);
    sector->m_ticking = true;
    m_sectorName[0] = '\0';
}

void SectorList::SaveSectorToYaml(WEngine::Sector *sector)
{
    YAML::Node root;
    YAML::Node entities;
    root["entities"] = entities;

    for (int i = 0; i < sector->m_entities.size(); i++)
    {
        std::string entryName = "e" + std::to_string(i);
        auto entry = GetEntityInfoYaml(sector->m_entities[i]);
        entities[entryName] = entry;
    }

    YAML::Emitter out;
    out << root;

    // GetDataPath() + EngineSettings::sectorPath + mission.name + ".yaml"

    std::string secName = sector->m_name;
    std::transform(secName.begin(), secName.end(), secName.begin(), [](unsigned char c) { return std::tolower(c); });

    std::string path = EditorSystems::GetAssetRepo()->GetDataPath();
    path += EngineSettings::sectorPath;
    path += secName;
    path += ".yaml";

    std::ofstream fout(path);
    fout << out.c_str();
    fout.close();

    WEngine::WLog::ConsoleLog(std::format("Saved changes to:\n\t\t{}", path));
}

YAML::Node SectorList::GetEntityInfoYaml(WEngine::Entity *e)
{
    YAML::Node root;

    root["name"] = e->entityName;

    YAML::Node posNode;
    posNode.push_back(e->transform.position.x);
    posNode.push_back(e->transform.position.y);
    posNode.SetStyle(YAML::EmitterStyle::Flow);

    root["position"] = posNode;

    YAML::Node sizeNode;
    sizeNode.push_back(e->transform.size.x);
    sizeNode.push_back(e->transform.size.y);
    sizeNode.SetStyle(YAML::EmitterStyle::Flow);
    root["size"] = sizeNode;

    YAML::Node comps;
    root["components"] = comps;

    for (int i = 0; i < e->m_components.size(); i++)
    {
        std::string entryName = "c" + std::to_string(i);
        auto entry = GetComponentInfoYaml(e->m_components[i]);
        comps[entryName] = entry;
    }

    return YAML::Clone(root);
}

YAML::Node SectorList::GetComponentInfoYaml(WEngine::Component *comp)
{
    YAML::Node root;

    AnyComponent* any = (AnyComponent*)comp;

    root["component-type"] = any->m_ID;

    auto options = EditorSystems::GetCompSettingsRepo()->GetInternalOptions(any->m_ID);

    WEngine::Vector2 vec;
    WEngine::Color col;

    for (int i = 0; i < options.size(); i++)
    {
        auto option = options[i];

        switch (option.type)
        {
            case ComponentOptionType::Number:
                root[option.optionInternal] = std::get<int32>(any->GetData(i));
                break;
            case ComponentOptionType::Float:
                root[option.optionInternal] = std::get<float32>(any->GetData(i));
                break;
            case ComponentOptionType::Bool:
                root[option.optionInternal] = std::get<bool>(any->GetData(i));
                break;
            case ComponentOptionType::String:
                root[option.optionInternal] = std::get<std::string>(any->GetData(i)).c_str();
                break;
            case ComponentOptionType::Vector2:
            {
                vec = std::get<WEngine::Vector2>(any->GetData(i));

                YAML::Node v;
                v.push_back(vec.x);
                v.push_back(vec.y);
                v.SetStyle(YAML::EmitterStyle::Flow);

                root[option.optionInternal] = YAML::Clone(v);
                break;
            }

            case ComponentOptionType::Color:
            {
                col = std::get<WEngine::Color>(any->GetData(i));

                YAML::Node c;
                c.push_back((uint32)col.red);
                c.push_back((uint32)col.green);
                c.push_back((uint32)col.blue);
                c.push_back((uint32)col.alpha);
                c.SetStyle(YAML::EmitterStyle::Flow);

                root[option.optionInternal] = YAML::Clone(c);
                break;
            }
            default:
                break;
        }
    }

    return YAML::Clone(root);
}
