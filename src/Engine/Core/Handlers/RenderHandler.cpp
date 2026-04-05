#include "RenderHandler.h"
#include <iostream>
#include <mmintrin.h>
#include <queue>

#include <Engine/Math/Vector.h>
#include <Engine/Util/Log.h>
#include <Engine/Util/Conversions.h>
#include <Engine/Components/Rendering/CameraComponent.h>
#include <Engine/Core/Handlers/AssetRepo.h>

#include <gtc/matrix_transform.hpp>

#include <Engine/imgui/imgui.h>
#include <Engine/imgui/implot.h>
#include <Engine/imgui/imgui_node_editor.h>

#include <Engine/Core/System/GPU.h>

#include "steamclientpublic.h"
#include "Engine/imgui/imgui_node_editor_internal.h"
#include "Engine/Types/DebugFlags.h"

using namespace WEngine;

RenderHandler::RenderHandler()
{
	InitSDL();
	if (!GPU::SETTING_InitGPUApi(m_window))
	{
		WLog::SetConsoleError();
		WLog::ConsoleLog("FATAL ERROR! GPU failed to initialize, aborting!");
		abort();
	}
	GPU::SETTING_ToggleBlending(true);
	GPU::SETTING_ToggleDepthTest(false);
	InitImGui();

	m_projection = glm::ortho(
		-(float)m_screenWidth / 2.0f, (float)m_screenWidth / 2.0f,
		-(float)m_screenHeight / 2.0f, (float)m_screenHeight / 2.0f,
		-1.0f, 1.0f
	);

	SetupVBOs();
	SetupFramebuffer();

}

void RenderHandler::InitFallbackShaders()
{
	auto screenNullable = GPU::ALLOC_CompileShader("screen");
	auto spriteNullable = GPU::ALLOC_CompileShader("sprite");
	auto lineNullable = GPU::ALLOC_CompileShader("line");

	if (screenNullable.HasValue()) m_screenShader = screenNullable.GetValue(); else goto fail;
	if (spriteNullable.HasValue()) m_spriteShader = spriteNullable.GetValue(); else goto fail;
	if (lineNullable.HasValue()) m_lineShader = lineNullable.GetValue(); else goto fail;

	SetupPostProcessing();

	return;

	fail:
	WLog::SetConsoleError();
	WLog::ConsoleLog("Shaders failed to load");
}

void RenderHandler::SetNewCamera(CameraComponent* newCamera)
{
	m_currentCamera = newCamera;
}

CameraComponent* RenderHandler::GetCurrentCamera()
{
	return m_currentCamera;
}

void RenderHandler::BeginFrame()
{
	GPU::DRAWCALL_ResetImGui();

	GPU::SETTING_SelectFramebuffer(m_framebuffer);
	if (m_hasOverrideScreenSize)
		GPU::SETTING_SetViewport(Vector2::Zero, m_overrideScreenSize);
	else
		GPU::SETTING_SetViewport(Vector2::Zero, Vector2(m_screenWidth, m_screenHeight));


	if (m_currentCamera == nullptr)
		GPU::DRAWCALL_ClearScreen(Color{25, 25, 25, 255});
	else
		GPU::DRAWCALL_ClearScreen(m_currentCamera->GetBackColor());
}

void RenderHandler::AddToRenderQueue(RenderMission mission)
{
	m_renderQueue[mission.layer].push_back(mission);
}

void RenderHandler::AddToVisualizationQueue(RenderVisualizationMission mission)
{
	m_visualizationQueue.push_back(mission);
}

void RenderHandler::RenderFrame()
{
	GPU::SETTING_ToggleWireFrame(DebugFlags::GetFlag(0, 0));
	GPU::SETTING_ToggleBlending(!DebugFlags::GetFlag(0, 0));

	if (m_currentCamera != nullptr)
	{
		auto camPos = m_currentCamera->GetCameraTransform().position;
		auto camSize = m_currentCamera->GetCameraTransform().size;
		WorldToScreenSpace(camPos);
		m_view = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f / camSize.x, 1.0f / camSize.y, 1.0f)) *
			glm::translate(glm::mat4(1.0f), glm::vec3(-camPos.x, -camPos.y, 0.0f));
	}

	RenderQueue();
	RenderVisualizationQueue();

	GPU::SETTING_SelectFramebufferScreen();
	if (m_hasOverrideScreenSize)
		GPU::SETTING_SetViewport(Vector2::Zero, m_overrideScreenSize);
	else
		GPU::SETTING_SetViewport(Vector2::Zero, Vector2(m_screenWidth, m_screenHeight));
	GPU::DRAWCALL_ClearScreen(Color{0, 0, 0, 255});

	RenderPostProcessedFrame();

	GPU::DRAWCALL_DrawImGui();
	GPU::DRAWCALL_SwapBuffers(m_window);
}

void RenderHandler::WorldToScreenSpace(Vector2& worldSpace)
{
	worldSpace.x = (float)MetreToPixel(worldSpace.x);
	worldSpace.y = (float)MetreToPixel(worldSpace.y);
}

Vector2 RenderHandler::GetInternalResolution() const
{
	return Vector2(m_internalWidth, m_internalHeight);
}

Vector2 RenderHandler::GetWindowResolution() const
{
	return Vector2(m_screenWidth, m_screenHeight);
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

	GPU::SETTING_ConfigureSDL();

	SDL_DisplayID display = SDL_GetPrimaryDisplay();
	m_displayMode = const_cast<SDL_DisplayMode*>(SDL_GetCurrentDisplayMode(display));

	Uint32 windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_BORDERLESS;

	if (m_displayMode != nullptr)
	{
		this->m_screenWidth = m_displayMode->w;
		this->m_screenHeight = m_displayMode->h;
	}
	else
	{
		WLog::SetConsoleWarning();
		WLog::ConsoleLog(std::format("Couldnt grab display mode, defaulting back to 800x600: {}", SDL_GetError()));
		this->m_screenWidth = 800;
		this->m_screenHeight = 600;
	}

	SDL_PropertiesID props = SDL_CreateProperties();
	SDL_SetStringProperty(props, SDL_PROP_WINDOW_CREATE_TITLE_STRING, EngineSettings::engineName.c_str());
	SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_X_NUMBER, 0);
	SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_Y_NUMBER, 0);
	SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, m_screenWidth);
	SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, m_screenHeight);
	// As in the migration guide, this isnt optimal, but its ok
	SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_FLAGS_NUMBER, windowFlags);

	m_window = SDL_CreateWindowWithProperties(props);

	if (m_window == nullptr)
	{
		WLog::SetConsoleError();
		WLog::ConsoleLog("Window couldnt be opened");
		return;
	}
	WLog::ConsoleLog(std::format("Window opened at resolution {}x{}", m_screenWidth, m_screenHeight));
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

	GPU::SETTING_ConfigureImGui(m_window);

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

void RenderHandler::ResizeViewport(Vector2 newSize)
{
	auto fbNullable = GPU::ALLOC_CreateFramebuffer(newSize);

	if (!fbNullable.HasValue())
	{
		WLog::SetConsoleError();
		WLog::ConsoleLog("Could not create Framebuffer for resizing.");
		return;
	}

	m_framebuffer = fbNullable.GetValue();

	m_projection = glm::ortho(
		-newSize.x / 2.0f, newSize.x / 2.0f,
		-newSize.y / 2.0f, newSize.y / 2.0f,
		-1.0f, 1.0f
	);

	m_hasOverrideScreenSize = true;
	m_overrideScreenSize = newSize;
}

Framebuffer RenderHandler::GetGameFramebuffer()
{
	return m_framebuffer;
}

void RenderHandler::RenderQueue()
{
	uint8 i = 0;
	for (auto& layer : m_renderQueue)
	{
		if (i == (uint8)RenderLayer::Default)
			SortSpriteZ(layer);

		for (auto& mission : layer)
			RenderSingleMission(mission);
		layer.clear();
		i++;
	}
}

void RenderHandler::RenderVisualizationQueue()
{
	for (auto& visualization : m_visualizationQueue)
		RenderSingleVisualization(visualization);

	m_visualizationQueue.clear();
}

void RenderHandler::RenderPostProcessedFrame()
{
	Texture fbTexture = GPU::GetFramebufferTexture(m_framebuffer).GetValue();
	Texture pingTexture = GPU::GetFramebufferTexture(m_ppFramebuffers[0]).GetValue();
	Texture pongTexture = GPU::GetFramebufferTexture(m_ppFramebuffers[1]).GetValue();
	GPU::SETTING_ToggleBlending(true);
	GPU::SETTING_ToggleWireFrame(false);

	GPU::SETTING_SelectFramebuffer(m_ppFramebuffers[0]);
	ShaderSettings settings1;
	settings1.push_back({ShaderSettingType::Texture, fbTexture, "u_Texture"});
	GPU::DRAWCALL_DrawModel(m_screen, m_screenShader, settings1);

	bool finalPong = false;

	for (int i = 0; i < m_ppShaders.size(); i++)
	{
		if (i % 2 == 0)
		{
			GPU::SETTING_SelectFramebuffer(m_ppFramebuffers[0]);
			m_ppShaders[i].settings[0].option = pongTexture;
			GPU::DRAWCALL_DrawModel(m_screen, m_ppShaders[i].shader, m_ppShaders[i].settings);
			finalPong = false;
		}
		else
		{
			GPU::SETTING_SelectFramebuffer(m_ppFramebuffers[1]);
			m_ppShaders[i].settings[0].option = pingTexture;
			GPU::DRAWCALL_DrawModel(m_screen, m_ppShaders[i].shader, m_ppShaders[i].settings);
			finalPong = true;
		}
	}

	ShaderSettings settingsFinal;
	if (finalPong)
	{
		GPU::SETTING_SelectFramebufferScreen();
		settingsFinal.push_back({ShaderSettingType::Texture, pongTexture, "u_Texture"});
		GPU::DRAWCALL_DrawModel(m_screen, m_screenShader, settingsFinal);
	}
	else
	{
		GPU::SETTING_SelectFramebufferScreen();
		settingsFinal.push_back({ShaderSettingType::Texture, pingTexture, "u_Texture"});
		GPU::DRAWCALL_DrawModel(m_screen, m_screenShader, settingsFinal);
	}
}

void RenderHandler::RenderSingleMission(RenderMission& mission)
{
	if (m_currentCamera == nullptr)
		return;

	Rectangle r = mission.quadBounds;

	WorldToScreenSpace(r.p1);
	WorldToScreenSpace(r.p2);

	const glm::mat4 model =
		glm::translate(glm::mat4(1.0f), glm::vec3(r.p1.x, r.p1.y, 0.0f)) *
		glm::scale(glm::mat4(1.0f), glm::vec3(r.p2.x, r.p2.y, 1.0f)) *
		glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, 0.5f, 0.0f));
	Mat4x4 mvp = Glm4x4ToMat4x4(m_projection * m_view * model);

	mission.shaderSettings.push_back({ShaderSettingType::Matrix4, mvp, "u_MVP"});
	mission.shaderSettings.push_back({ShaderSettingType::Bool, EngineSettings::useTextureFiltering, "u_Filtering"});


	GPU::DRAWCALL_DrawModel(m_sprite, FindShader(mission.shader), mission.shaderSettings);
}

void RenderHandler::RenderSingleVisualization(RenderVisualizationMission &mission)
{
	if (m_currentCamera == nullptr)
		return;

	Mat4x4 mvp = Glm4x4ToMat4x4(m_projection * m_view);
	ShaderSettings settings;

	for (const auto& line : mission.lines)
	{
		Line2D l = line;
		WorldToScreenSpace(l.p1);
		WorldToScreenSpace(l.p2);
		settings.push_back({ShaderSettingType::Matrix4, mvp, "u_MVP"});
		settings.push_back({ShaderSettingType::Color, mission.color, "u_Color"});
		GPU::DRAWCALL_DrawLine(m_line, m_lineShader, settings, {l});
	}

	mission.lines.clear();
}

Shader RenderHandler::FindShader(const std::string& shaderName)
{
	if (shaderName.empty() || shaderName == "sprite")
		return m_spriteShader;

	for (const auto& shader : m_shaderRepo)
	{
		if (shader.first == shaderName)
			return shader.second;
	}

	const auto shaderNullable = GPU::ALLOC_CompileShader(shaderName);
	if (!shaderNullable.HasValue())
		return m_spriteShader;

	const std::pair newShader = {shaderName, shaderNullable.GetValue()};
	m_shaderRepo.push_back(newShader);

	return m_spriteShader;
}

void RenderHandler::SetupVBOs()
{
	const ModelInfo infoScreen = {
		{ // verts
			{-1.0f, -1.0f}, {1.0f, -1.0f}, {-1.0f,  1.0f}, { 1.0f,  1.0f}
		},
		{ // uvs
			{0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f}
		},
	};

	const ModelInfo infoSprite = {
		{ // verts
			{0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f}
		},
		{ // uvs
			{0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f}
		},
	};

	const auto screenModel = GPU::ALLOC_CreateModel(infoScreen);
	const auto spriteModel = GPU::ALLOC_CreateModel(infoSprite);
	const auto lineModel = GPU::ALLOC_CreateLine();

	if (screenModel.HasValue()) m_screen = screenModel.GetValue();
	if (spriteModel.HasValue()) m_sprite = spriteModel.GetValue();
	if (lineModel.HasValue()) m_line = lineModel.GetValue();
}

void RenderHandler::SetupFramebuffer()
{
	auto fbNullable = GPU::ALLOC_CreateFramebuffer(Vector2(m_screenWidth, m_screenHeight));

	if (fbNullable.HasValue())
		m_framebuffer = fbNullable.GetValue();
}

void RenderHandler::SetupPostProcessing()
{
	auto fbNullable1 = GPU::ALLOC_CreateFramebuffer(Vector2(m_screenWidth, m_screenHeight));
	auto fbNullable2 = GPU::ALLOC_CreateFramebuffer(Vector2(m_screenWidth, m_screenHeight));

	if (fbNullable1.HasValue())
		m_ppFramebuffers[0] = fbNullable1.GetValue();

	if (fbNullable2.HasValue())
		m_ppFramebuffers[1] = fbNullable2.GetValue();

	SetupPostProcessingShaders();
}

void RenderHandler::SortSpriteZ(wtl::deque<RenderMission>& toBeSorted)
{
	if (DebugFlags::GetFlag(0, 1))
		return;

	std::sort(toBeSorted.begin(), toBeSorted.end(),
		[](const RenderMission& a, const RenderMission& b)
		{
			float aSize = a.quadBounds.p2.y - a.quadBounds.p1.y;
			float bSize = b.quadBounds.p2.y - b.quadBounds.p1.y;
			return a.quadBounds.p1.y - aSize / 2 > b.quadBounds.p1.y - bSize / 2;
		});
}

void RenderHandler::SetupPostProcessingShaders()
{
	{
		PostProcessShader shader;
		shader.shaderName = "ColorCorrect";
		auto sn = GPU::ALLOC_CompileShader(shader.shaderName);
		shader.shader = sn.GetValue();
		shader.settings.push_back({ShaderSettingType::Texture, 0, "u_Texture"});
		shader.settings.push_back({ShaderSettingType::Float, 0.0f, "saturation"});
		m_ppShaders.push_back(shader);

		WLog::ConsoleLog(std::format("Loaded Post Processing shader {}", shader.shaderName));
	}
}

