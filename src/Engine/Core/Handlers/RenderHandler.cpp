#include "RenderHandler.h"
#include <iostream>
#include <queue>

#include <Engine/Util/Log.h>
#include <Engine/Components/Rendering/CameraComponent.h>
#include <Engine/Core/Handlers/AssetRepo.h>

#include <gtc/matrix_transform.hpp>

#include <Engine/imgui/imgui.h>
#include <Engine/imgui/implot.h>

#include <Engine/Core/System/Iris.h>

#include "Engine/Types/DebugFlags.h"

#include "InputHandler.h"
#include "Engine/EngineDefines.h"
#include "Engine/Types/CoreSystems.h"


using namespace WEngine;

RenderHandler::RenderHandler()
{
	InitSDL();
	if (!Iris::SETTING_InitGPUApi(m_window))
	{
		WLog::SetConsoleError();
		WLog::ConsoleLog("FATAL ERROR! GPU failed to initialize, aborting!");
		abort();
	}
	InitImGui();

	m_projection = glm::perspective(
		glm::radians(90.0f),
		m_windowResolution.x / m_windowResolution.y,
		0.01f,
		1000.0f
		);
}

void RenderHandler::EnableEditorMode(const Vector2& viewportResolution)
{
	m_isEditor = true;
	m_viewportResolution = viewportResolution;

	auto fbN = Iris::ALLOC_CreateFramebuffer(viewportResolution);

	if (fbN.HasValue())
		m_viewportFb = fbN.GetValue();
	else
	{
		WLog::SetConsoleError();
		WLog::ConsoleLog("Could not create a new Framebuffer for the viewport, falling back!");
		m_isEditor = false;
	}

	m_projection = glm::perspective(
		glm::radians(90.0f),
		m_viewportResolution.x / m_viewportResolution.y,
		0.01f,
		1000.0f
		);
}

Framebuffer RenderHandler::EditorGetViewportFramebuffer()
{
	return m_viewportFb;
}

void RenderHandler::BeginFrame()
{
	Iris::SETTING_BeginNewFrame();

	if (m_isEditor)
		Iris::SETTING_SelectFramebufferForRender(m_viewportFb);
	else
		Iris::SETTING_SelectFramebufferScreenForRender();

	Iris::DRAWCALL_ResetImGui();
	Iris::DRAWCALL_ClearFrame(m_camera->GetBackColor());
	if (m_isEditor)
		Iris::SETTING_SetViewportSize(m_viewportResolution);
	else
		Iris::SETTING_SetViewportSize(EngineSettings::resolution);


	if (m_camera == nullptr)
		m_viewMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	else
	{
		Vector3 camPos = m_camera->GetPosition();
		Vector3 camRot = m_camera->GetRotation();

		m_viewMatrix = glm::mat4(1.0f);

		m_viewMatrix = glm::rotate(m_viewMatrix, glm::radians(camRot.x), glm::vec3(1, 0, 0));
		m_viewMatrix = glm::rotate(m_viewMatrix, glm::radians(camRot.y), glm::vec3(0, 1, 0));
		m_viewMatrix = glm::rotate(m_viewMatrix, glm::radians(camRot.z), glm::vec3(0, 0, 1));

		m_viewMatrix = glm::translate(m_viewMatrix, -glm::vec3(camPos.x, camPos.y, camPos.z));
	}
}

void RenderHandler::RenderFrame()
{
	SortMissions();

	for (auto& materialGroup : m_sortedMissions)
	{
		for (auto& modelGroup : materialGroup.models)
		{
			RenderModelGroup(modelGroup, materialGroup.groupID);
		}
	}

	Mat4x4 vp = Glm4x4ToMat4x4(m_projection * m_viewMatrix);

	for (auto& stat : m_stationaryMissions)
		Iris::DRAWCALL_DrawModelInstancedStationary(stat.model, stat.material, vp);

	if (m_isEditor)
	{
		Iris::SETTING_FinishFramebufferRender();
		Iris::SETTING_SelectFramebufferScreenForRender();
		Iris::DRAWCALL_ClearFrame(m_camera->GetBackColor());
		Iris::DRAWCALL_DrawImGui();
		Iris::SETTING_FinishFramebufferRender();
	}
	else
	{
		Iris::DRAWCALL_DrawImGui();
		Iris::SETTING_FinishFramebufferRender();
	}

	Iris::DRAWCALL_DrawToDisplay(m_window);

	m_renderQueue.clear();
	CleanSortedMissions();
}

void RenderHandler::RegisterCamera(CameraComponent *camera)
{
	m_camera = camera;
}

void RenderHandler::AddToRenderQueue(RenderMission& mission)
{
	if (mission.model == 0 || mission.material == 0)
		return;
	m_renderQueue.push_back(mission);
}

void RenderHandler::RecordStationaryAdd(Model model, Material material, const Transform& transform)
{
	for (auto& objects : m_stationaryAddQueue)
	{
		if (objects.model == model && objects.material == material)
		{
			objects.instData.push_back({CalcModelMatrix(transform)});
			return;
		}
	}

	StationaryObjStaged obj;
	obj.model = model;
	obj.material = material;
	obj.instData.push_back({CalcModelMatrix(transform)});
	m_stationaryAddQueue.push_back(obj);
}

void RenderHandler::PushStationaryData()
{
	for (auto& object : m_stationaryAddQueue)
		Iris::AddStationaryObjects(object.model, object.material, object.instData);

	m_stationaryAddQueue.clear();
}

Mat4x4 RenderHandler::CalcModelMatrix(const Transform &transform)
{
	Vector3 modPos = transform.position;
	Vector3 modRot = transform.rotation;
	Vector3 modSca = transform.size;

	glm::mat4 modelMatrix = glm::mat4(1.0f);

	modelMatrix = glm::translate(modelMatrix, glm::vec3(modPos.x, modPos.y, modPos.z));

	modelMatrix = glm::rotate(modelMatrix, glm::radians(modRot.x - 180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(modRot.y), glm::vec3(0.0f, 1.0f, 0.0f));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(modRot.z), glm::vec3(0.0f, 0.0f, 1.0f));

	modelMatrix = glm::scale(modelMatrix, glm::vec3(modSca.x, modSca.y, modSca.z));

	return Glm4x4ToMat4x4(modelMatrix);
}

Mat4x4 RenderHandler::CalcMVPMatrix(const Transform &transform)
{
	Vector3 modPos = transform.position;
	Vector3 modRot = transform.rotation;
	Vector3 modSca = transform.size;

	glm::mat4 modelMatrix = glm::mat4(1.0f);

	modelMatrix = glm::translate(modelMatrix, glm::vec3(modPos.x, modPos.y, modPos.z));

	modelMatrix = glm::rotate(modelMatrix, glm::radians(modRot.x - 180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(modRot.y), glm::vec3(0.0f, 1.0f, 0.0f));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(modRot.z), glm::vec3(0.0f, 0.0f, 1.0f));

	modelMatrix = glm::scale(modelMatrix, glm::vec3(modSca.x, modSca.y, modSca.z));

	glm::mat4 mvpG = m_projection * m_viewMatrix * modelMatrix;

	return Glm4x4ToMat4x4(mvpG);
}

void RenderHandler::RenderSingleMission(RenderMission &mission)
{
	Mat4x4 mvp = CalcMVPMatrix(mission.transform);
	Iris::DRAWCALL_DrawModel(mission.model, mission.material, mvp);
}

void RenderHandler::RenderModelGroup(const ModelGroup &group, Material material)
{
	wtl::vector<InstanceData> instances(group.missions.size());

	for (int i = 0; i < group.missions.size(); i++)
	{
		Mat4x4 model = CalcModelMatrix(group.missions[i].transform);
		instances[i] = {model};
	}

	Mat4x4 vp = Glm4x4ToMat4x4(m_projection * m_viewMatrix);

	Iris::DRAWCALL_DrawModelInstanced(group.groupID, material, vp, instances);
}

void RenderHandler::SortStationary(RenderMission &mission)
{
	for (auto& objects : m_stationaryMissions)
	{
		if (objects.model == mission.model && objects.material == mission.material)
			return;
	}
	m_stationaryMissions.push_back({mission.model, mission.material});
}

void RenderHandler::InsertModelIntoShaderGroup(RenderMission &mission, MaterialGroup &materialGroup)
{
	bool foundModel = false;
	for (uint64 i = 0; i < materialGroup.models.size(); ++i)
	{
		if (materialGroup.models[i].groupID == mission.model)
		{
			foundModel = true;
			materialGroup.models[i].missions.push_back(mission);
		}
	}
	if (!foundModel)
	{
		ModelGroup group;
		group.groupID = mission.model;
		group.missions.push_back(mission);
		materialGroup.models.push_back(group);
	}
}

void RenderHandler::SortMissions()
{
	for (auto& mission : m_renderQueue)
	{
		if (mission.isStationary)
		{
			SortStationary(mission);
			continue;
		}
		bool foundShader = false;
		for (uint64 i = 0; i < m_sortedMissions.size(); ++i)
		{
			if (m_sortedMissions[i].groupID == mission.material)
			{
				foundShader = true;
				InsertModelIntoShaderGroup(mission, m_sortedMissions[i]);
			}
		}
		if (!foundShader)
		{
			MaterialGroup group;
			group.groupID = mission.material;
			InsertModelIntoShaderGroup(mission, group);
			m_sortedMissions.push_back(group);
		}
	}
}

void RenderHandler::CleanSortedMissions()
{
	for (auto& shaderGroup : m_sortedMissions)
	{
		for (auto& modelGroup : shaderGroup.models)
		{
			modelGroup.missions.clear();
		}
		shaderGroup.models.clear();
	}
	m_sortedMissions.clear();
	m_stationaryMissions.clear();
}

void RenderHandler::InitSDL()
{
	SDL_InitFlags initFlags = SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_GAMEPAD;
	SDL_SetHint(SDL_HINT_IME_IMPLEMENTED_UI, "1");

	if (!SDL_Init(initFlags))
	{
		WLog::SetConsoleError();
		WLog::ConsoleLog(std::format("SDL Initialisation failed: {}", SDL_GetError()));
		return;
	}

	SDL_DisplayID display = SDL_GetPrimaryDisplay();
	m_displayMode = const_cast<SDL_DisplayMode*>(SDL_GetCurrentDisplayMode(display));

	Uint32 windowFlags = SDL_WINDOW_BORDERLESS;
#if GPU_BACKEND == GPU_VULKAN
	windowFlags |= SDL_WINDOW_VULKAN;
#endif


	if (m_displayMode != nullptr)
	{
		m_windowResolution.x = m_displayMode->w;
		m_windowResolution.y = m_displayMode->h;
	}
	else
	{
		WLog::SetConsoleWarning();
		WLog::ConsoleLog(std::format("Couldnt grab display mode, defaulting back to 800x600: {}", SDL_GetError()));
		m_windowResolution.x = 800;
		m_windowResolution.y = 600;
	}

	SDL_PropertiesID props = SDL_CreateProperties();
	SDL_SetStringProperty(props, SDL_PROP_WINDOW_CREATE_TITLE_STRING, EngineSettings::engineName.c_str());
	SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_X_NUMBER, 0);
	SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_Y_NUMBER, 0);
	SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, m_windowResolution.x);
	SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, m_windowResolution.y);
	// As in the migration guide, this isnt optimal, but its ok
	SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_FLAGS_NUMBER, windowFlags);

	m_window = SDL_CreateWindowWithProperties(props);

	if (m_window == nullptr)
	{
		WLog::SetConsoleError();
		WLog::ConsoleLog("Window couldnt be opened");
		return;
	}
	WLog::ConsoleLog(std::format("Window opened at resolution {}x{}", m_windowResolution.x, m_windowResolution.y));

	CoreSystems::GetInputHandler()->SetWindow(m_window);
}
void RenderHandler::InitImGui()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImPlot::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.IniFilename = nullptr;

	ImGui::StyleColorsDark();

	Iris::SETTING_ConfigureImGui(m_window);

	ImGuiStyle& style = ImGui::GetStyle();

	style.FrameRounding = 4;
	style.WindowBorderSize = 0;
	style.GrabMinSize = 8;
	style.ScrollbarSize = 8;
	style.WindowRounding = 0;
	style.WindowTitleAlign = { 0.5f, 0.5f };

	ImPlotStyle& ipStyle = ImPlot::GetStyle();

	ipStyle.PlotDefaultSize = {200, 200};

	ImPlot::StyleColorsDark();
	auto colors = style.Colors;

	colors[ImGuiCol_Border] = ImVec4(0.50f, 0.43f, 0.43f, 0.50f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.48f, 0.16f, 0.16f, 0.54f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.98f, 0.26f, 0.26f, 0.40f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.98f, 0.26f, 0.26f, 0.67f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.48f, 0.16f, 0.16f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.98f, 0.26f, 0.26f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.88f, 0.24f, 0.24f, 1.00f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.98f, 0.26f, 0.26f, 1.00f);
	colors[ImGuiCol_Button] = ImVec4(0.98f, 0.26f, 0.26f, 0.40f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.98f, 0.26f, 0.26f, 1.00f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.98f, 0.06f, 0.06f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(0.98f, 0.26f, 0.26f, 0.31f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.98f, 0.26f, 0.26f, 0.80f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.98f, 0.26f, 0.26f, 1.00f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.75f, 0.10f, 0.10f, 0.78f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.75f, 0.10f, 0.10f, 1.00f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.98f, 0.26f, 0.26f, 0.20f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.98f, 0.26f, 0.26f, 0.67f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.98f, 0.26f, 0.26f, 0.95f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.98f, 0.26f, 0.26f, 0.80f);
	colors[ImGuiCol_Tab] = ImVec4(0.58f, 0.18f, 0.18f, 0.86f);
	colors[ImGuiCol_TabSelected] = ImVec4(0.68f, 0.20f, 0.20f, 1.00f);
	colors[ImGuiCol_TabSelectedOverline] = ImVec4(0.98f, 0.26f, 0.26f, 1.00f);
	colors[ImGuiCol_TabDimmed] = ImVec4(0.15f, 0.07f, 0.07f, 0.97f);
	colors[ImGuiCol_TabDimmedSelected] = ImVec4(0.42f, 0.14f, 0.14f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.35f, 0.58f, 1.00f, 1.00f);


}