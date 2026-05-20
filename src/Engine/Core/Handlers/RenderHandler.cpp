#include "RenderHandler.h"
#include <iostream>
#include <queue>

#include <Engine/Math/Vector.h>
#include <Engine/Util/Log.h>
#include <Engine/Components/Rendering/CameraComponent.h>
#include <Engine/Core/Handlers/AssetRepo.h>

#include <gtc/matrix_transform.hpp>

#include <Engine/imgui/imgui.h>
#include <Engine/imgui/implot.h>

#include <Engine/Core/System/Iris.h>

#include "Engine/imgui/imgui_node_editor_internal.h"
#include "Engine/Types/DebugFlags.h"

#include <Engine/Types/Rendering/VertextData.h>

#include "InputHandler.h"
#include "Engine/Types/CoreSystems.h"

using namespace WEngine;

Model testModel;
Shader triShader;

glm::mat4 projection;
glm::mat4 viewMatrix;

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

	ModelInfo info;
	info.vertices.resize(8);
	info.vertices[0] = {{ 0.1f, -0.1f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f}}; // t r f
	info.vertices[1] = {{ 0.1f,	 0.1f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f}}; // b r f
	info.vertices[2] = {{-0.1f,  0.1f, -0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {0.0f, 0.0f}}; // b l f
	info.vertices[3] = {{-0.1f, -0.1f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f}}; // t l f

	info.vertices[4] = {{ 0.1f, -0.1f, -0.7f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f}}; // t r b
	info.vertices[5] = {{ 0.1f,	 0.1f, -0.7f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f}}; // b r b
	info.vertices[6] = {{-0.1f,  0.1f, -0.7f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {0.0f, 0.0f}}; // b l b
	info.vertices[7] = {{-0.1f, -0.1f, -0.7f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f}}; // t l b

	info.indices =
		{0, 1, 2,
		 0, 2, 3,
		 0, 3, 4,
		 3, 7, 4,
		 0, 4, 5,
		 0, 5, 1,
		 3, 2, 7,
		 2, 6, 7,
		 2, 1, 5,
		 2, 5, 6,
		 4, 6, 5,
		 4, 7, 6};

	auto modelN = Iris::ALLOC_CreateModel(info);

	if (modelN.HasValue())
		testModel = modelN.GetValue();
	else
	{
		WLog::SetConsoleError();
		WLog::ConsoleLog("DAMMIT!!!");
	}

	auto shaderN = Iris::ALLOC_CompileShader("triangle");

	if (shaderN.HasValue())
		triShader = shaderN.GetValue();
	else
	{
		WLog::SetConsoleError();
		WLog::ConsoleLog("DAMMIT AGAIN!!!");
	}

	projection = glm::perspective(
		glm::radians(90.0f),
		m_windowResolution.x / m_windowResolution.y,
		0.01f,
		100.0f
		);
}

void RenderHandler::BeginFrame()
{
	Iris::SETTING_BeginNewFrame();
	Iris::DRAWCALL_ResetImGui();
	Iris::DRAWCALL_ClearFrame(Color{30, 30, 30, 255});
	Iris::SETTING_SetViewportSize(EngineSettings::resolution);



	if (m_camera == nullptr)
		viewMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	else
	{
		Vector3 camPos = m_camera->GetPosition();
		Vector3 camRot = m_camera->GetRotation();

		viewMatrix = glm::mat4(1.0f);

		viewMatrix = glm::rotate(viewMatrix, glm::radians(camRot.x), glm::vec3(1, 0, 0));
		viewMatrix = glm::rotate(viewMatrix, glm::radians(camRot.y), glm::vec3(0, 1, 0));
		viewMatrix = glm::rotate(viewMatrix, glm::radians(camRot.z), glm::vec3(0, 0, 1));

		viewMatrix = glm::translate(viewMatrix, -glm::vec3(camPos.x, camPos.y, camPos.z));
	}
}

void RenderHandler::RenderFrame()
{
	for (auto& mission : m_renderQueue)
	{
		RenderSingleMission(mission);
	}

	Iris::DRAWCALL_DrawImGui();
	Iris::DRAWCALL_SwapBuffers(m_window);
	m_renderQueue.clear();
}

void RenderHandler::RegisterCamera(CameraComponent *camera)
{
	m_camera = camera;
}

void RenderHandler::AddToRenderQueue(RenderMission& mission)
{
	m_renderQueue.push_back(mission);
}

void RenderHandler::RenderSingleMission(RenderMission &mission)
{
	Vector3 modPos = mission.transform.position;
	Vector3 modRot = mission.transform.rotation;
	Vector3 modSca = mission.transform.size;

	glm::mat4 modelMatrix = glm::mat4(1.0f);

	modelMatrix = glm::translate(modelMatrix, glm::vec3(modPos.x, modPos.y, modPos.z));

	modelMatrix = glm::rotate(modelMatrix, glm::radians(modRot.x), glm::vec3(1.0f, 0.0f, 0.0f));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(modRot.y), glm::vec3(0.0f, 1.0f, 0.0f));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(modRot.z), glm::vec3(0.0f, 0.0f, 1.0f));

	modelMatrix = glm::scale(modelMatrix, glm::vec3(modSca.x, modSca.y, modSca.z));

	glm::mat4 mvpG = projection * viewMatrix * modelMatrix;

	Mat4x4 mvp = Glm4x4ToMat4x4(mvpG);
	ShaderSettings shaderSettings{};
	shaderSettings.push_back({ShaderSettingType::Matrix4, mvp, "mvp"});
	Iris::DRAWCALL_DrawModel(testModel, triShader, shaderSettings);
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