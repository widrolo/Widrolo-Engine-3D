#include "ComponentList.h"

#include "Editor/Components/AnyComponent.h"
#include "Editor/Core/Handlers/CompSettingsRepo.h"
#include "Editor/Types/EditorState.h"
#include "Editor/Types/EditorSystems.h"
#include "Engine/Core/World/Entity.h"
#include "Engine/Core/World/Sector.h"

namespace WEditor {
	class AnyComponent;
}

using namespace WEditor;

void ComponentList::Setup()
{
    m_widgetName = "Entity Properties";
}

void ComponentList::RenderInternal()
{
    if (EditorState::SelectedEntity == nullptr)
	{
		ImGui::Text("No Component Selected");
		return;
	}

	auto entity = EditorState::SelectedEntity;

    WEngine::Vector2 vec;

	m_entityName = entity->entityName.c_str();
	m_entityPos[0] = entity->transform.position.x;
	m_entityPos[1] = entity->transform.position.y;
	m_entitySize[0] = entity->transform.size.x;
	m_entitySize[1] = entity->transform.size.y;

	ImGui::SeparatorText("Entity Settings");
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(20.f, 10.f));
	ImGui::InputText("##", const_cast<char*>(m_entityName), 128, ImGuiInputTextFlags_ElideLeft);
	ImGui::SameLine();
	if (ImGui::Button("Rename"))
	{
		entity->entityName = m_entityName;
		EditorState::SelectedSector->m_changedInEditor = true;
	}
	ImGui::PopStyleVar();

	if (ImGui::DragFloat2("Position", m_entityPos, 0.1f))
	{
		vec = WEngine::Vector2(m_entityPos[0], m_entityPos[1]);
		entity->transform.position = vec;
		EditorState::SelectedSector->m_changedInEditor = true;
	}

	if (ImGui::DragFloat2("Size", m_entitySize, 0.1f))
	{
		vec = WEngine::Vector2(m_entitySize[0], m_entitySize[1]);
		entity->transform.size = vec;
		EditorState::SelectedSector->m_changedInEditor = true;
	}

	ImGui::SeparatorText("Component Settings");

	static int newCompID = 0;
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(20.f, 10.f));
	ImGui::InputInt("##1", &newCompID);
	ImGui::SameLine();
	if (ImGui::Button("Create"))
	{
		AnyComponent* any = new AnyComponent(entity);
		int capacity = EditorSystems::GetCompSettingsRepo()->GetSettingCapacity(newCompID);
		any->Init(newCompID, capacity);
		entity->m_components.push_back(any);
		EditorState::SelectedSector->m_changedInEditor = true;
	}

	if (ImGui::Button("Remove Selected"))
	{
		if (EditorState::SelectedComponent == nullptr)
			goto noremove;
		if (entity->m_components.empty())
			goto noremove;
		entity->m_components.erase(
			std::remove(entity->m_components.begin(), entity->m_components.end(), EditorState::SelectedComponent));

		delete EditorState::SelectedComponent;
		EditorState::SelectedComponent = nullptr;

		EditorState::SelectedSector->m_changedInEditor = true;
	}
	noremove:
	ImGui::PopStyleVar();

	if (ImGui::TreeNodeEx("Components", ImGuiTreeNodeFlags_DrawLinesFull))
	{
		ShowComponentsInEntity();
		ImGui::TreePop();
	}
}

void ComponentList::ShowComponentsInEntity()
{
	auto entity = EditorState::SelectedEntity;

	int iter = 0;

	ImGuiTreeNodeFlags treeFlags = 0;
	treeFlags |= ImGuiTreeNodeFlags_Leaf;

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.f, 10.f));
	for (auto comp : entity->m_components)
	{
		std::string compRealName = EditorSystems::GetCompSettingsRepo()->GetSettingName(((AnyComponent*)comp)->m_ID);
		std::string compName = compRealName + "##" + std::to_string(iter);
		ImGuiTreeNodeFlags leaf = treeFlags;

		if (EditorState::SelectedComponent == comp)
			leaf |= ImGuiTreeNodeFlags_Selected;

		ImGui::TreeNodeEx(compName.c_str(), leaf);

		//ImGui::Text(entity->entityName.c_str());
		if (ImGui::IsItemClicked())
			EditorState::SelectedComponent = comp;

		ImGui::TreePop();
		iter++;
	}

	ImGui::PopStyleVar();
}
