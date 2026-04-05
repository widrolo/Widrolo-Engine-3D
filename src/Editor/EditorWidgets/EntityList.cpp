#include "EntityList.h"

#include "Editor/Types/EditorState.h"
#include "Engine/Core/World/Entity.h"
#include "Engine/Core/World/Sector.h"
#include "Engine/Math/Transform.h"

using namespace WEditor;

void EntityList::Setup()
{
    m_widgetName = "Entity List";
}

void EntityList::RenderInternal()
{
    if (EditorState::SelectedSector == nullptr)
    {
        ImGui::Text("No Sector Selected");
        return;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(20.f, 10.f));
    if (ImGui::Button("Create"))
    {
        WEngine::Sector* sec = EditorState::SelectedSector;
        WEngine::Entity* e = new WEngine::Entity();
        e->entityName = "New Entity";
        e->transform = WEngine::Transform::Zero;
        e->InitBaseComponents();

        sec->AddEntity(e);

        EditorState::SelectedSector->m_changedInEditor = true;
    }

    ImGui::SameLine();

    if (ImGui::Button("Remove Selected"))
    {
        if (EditorState::SelectedEntity == nullptr)
            goto noremove;
        WEngine::Sector* sec = EditorState::SelectedSector;

        std::erase(sec->m_entities, EditorState::SelectedEntity);

        EditorState::SelectedEntity->EntityDestroy();

        delete EditorState::SelectedEntity;
        EditorState::SelectedEntity = nullptr;

        EditorState::SelectedSector->m_changedInEditor = true;
    }
    noremove:
    ImGui::PopStyleVar();

    auto sector = EditorState::SelectedSector;

    if (ImGui::TreeNodeEx(sector->m_name.c_str(), ImGuiTreeNodeFlags_DrawLinesFull))
    {
        ShowEntitiesInSector();
        ImGui::TreePop();
    }
}

void EntityList::ShowEntitiesInSector()
{
    auto sector = EditorState::SelectedSector;

    int iter = 0;

    ImGuiTreeNodeFlags treeFlags = 0;
    treeFlags |= ImGuiTreeNodeFlags_Leaf;

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.f, 10.f));
    for (auto entity : sector->m_entities)
    {
        std::string entName = entity->entityName + "##" + std::to_string(iter);
        ImGuiTreeNodeFlags leaf = treeFlags;

        if (EditorState::SelectedEntity == entity)
            leaf |= ImGuiTreeNodeFlags_Selected;

        ImGui::TreeNodeEx(entName.c_str(), leaf);

        if (ImGui::IsItemClicked())
        {
            EditorState::SelectedEntity = entity;
            EditorState::SelectedComponent = nullptr;
        }

        ImGui::TreePop();
        iter++;
    }

    ImGui::PopStyleVar();
}
