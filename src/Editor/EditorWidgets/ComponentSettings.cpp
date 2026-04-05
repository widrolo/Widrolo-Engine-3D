#include "ComponentSettings.h"

#include "Editor/Components/AnyComponent.h"
#include "Editor/Core/Handlers/CompSettingsRepo.h"
#include "Editor/Types/EditorState.h"
#include "Editor/Types/EditorSystems.h"
#include "Engine/Core/World/Sector.h"

using namespace WEditor;

void ComponentSettings::Setup()
{
    m_widgetName = "Component Settings";
}

void ComponentSettings::RenderInternal()
{
    auto comp = EditorState::SelectedComponent;

    if (comp == nullptr)
    {
        ImGui::Text("No component selected");
        return;
    }
    ShowComponent();
}

void ComponentSettings::ShowComponent()
{
    auto comp = (AnyComponent*)EditorState::SelectedComponent;

    auto compDef = EditorSystems::GetCompSettingsRepo()->GetInternalOptions(comp->m_ID);

    ImGui::SeparatorText(EditorSystems::GetCompSettingsRepo()->GetSettingName(comp->m_ID).c_str());
    ImGui::Text("%s", EditorSystems::GetCompSettingsRepo()->GetSettingDesc(comp->m_ID).c_str());

    int iter = 0;
    for (auto option : EditorSystems::GetCompSettingsRepo()->GetInternalOptions(comp->m_ID))
    {
        ShowOption(option, iter);
        iter++;
    }
}

void ComponentSettings::ShowOption(const ComponentOption& option, uint8 optionNumber)
{
    AnyComponent* comp = (AnyComponent*)EditorState::SelectedComponent;

	// MY EYESSS!!!!!
	// Do yourself a favour, and dont look at this,
	// this seems to work quite well, so look somewhere
	// else for the bug

	static int32 numberIn = 0;
	static float floatIn = 0;
	static bool boolIn = false;
	static const char* textIn;
	static float f2In[2] = {0, 0};
	static float f4In[4] = {0, 0, 0, 0};

	std::string stringIn;
    WEngine::Vector2 vecIn;
    WEngine::Color colorIn;

	bool changed;

	switch (option.type)
	{
	case ComponentOptionType::Number:
		numberIn = std::get<int32>(comp->GetData(optionNumber));
		changed = ImGui::DragInt(option.optionName.c_str(), &numberIn);
		comp->SetData(optionNumber, numberIn);
		break;
	case ComponentOptionType::Float:
		floatIn = std::get<float>(comp->GetData(optionNumber));
		changed = ImGui::DragFloat(option.optionName.c_str(), &floatIn, 0.01f);
		comp->SetData(optionNumber, floatIn);
		break;
	case ComponentOptionType::Bool:
		boolIn = std::get<bool>(comp->GetData(optionNumber));
		changed = ImGui::Checkbox(option.optionName.c_str(), &boolIn);
		comp->SetData(optionNumber, boolIn);
		break;
	case ComponentOptionType::String:
		stringIn = std::get<std::string>(comp->GetData(optionNumber));
		textIn = stringIn.c_str();
		changed = ImGui::InputText(option.optionName.c_str(), const_cast<char*>(textIn), 128);
		stringIn = textIn;
		comp->SetData(optionNumber, stringIn);
		break;
	case ComponentOptionType::Vector2:
		vecIn = std::get<WEngine::Vector2>(comp->GetData(optionNumber));
		f2In[0] = vecIn.x;
		f2In[1] = vecIn.y;
		changed = ImGui::DragFloat2(option.optionName.c_str(), f2In, 0.01f);
		vecIn.x = f2In[0];
		vecIn.y = f2In[1];
		comp->SetData(optionNumber, vecIn);
		break;
	case ComponentOptionType::Color:
		colorIn = std::get<WEngine::Color>(comp->GetData(optionNumber));
		f4In[0] = colorIn.red / 255.0f;
		f4In[1] = colorIn.green / 255.0f;
		f4In[2] = colorIn.blue / 255.0f;
		f4In[3] = colorIn.alpha / 255.0f;
		changed = ImGui::ColorEdit4(option.optionName.c_str(), f4In);
		colorIn.red		= (uint8)(f4In[0] * 255);
		colorIn.green	= (uint8)(f4In[1] * 255);
		colorIn.blue	= (uint8)(f4In[2] * 255);
		colorIn.alpha	= (uint8)(f4In[3] * 255);
		comp->SetData(optionNumber, colorIn);
		break;

	default:
		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(255, 0, 0, 255));
		ImGui::Text("ERR Type!");
		ImGui::PopStyleColor(1);
		ImGui::SameLine();
		ImGui::Text("%s", option.optionName.c_str());
		break;
	}

	ImGui::SetItemTooltip("%s", option.optionDesctription.c_str());

	if (changed)
		EditorState::SelectedSector->m_changedInEditor = true;
}
