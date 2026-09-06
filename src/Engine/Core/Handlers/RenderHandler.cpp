#include "RenderHandler.h"
#include <iostream>
#include <queue>

#include <Engine/Util/Log.h>
#include <Engine/Core/Handlers/AssetRepo.h>

#include <glm/gtc/matrix_transform.hpp>

#include <Engine/imgui/imgui.h>
#include <Engine/imgui/implot.h>

#include <Engine/Core/System/Iris.h>

#include "Engine/Types/DebugFlags.h"

#include "Input.h"
#include "Engine/EngineDefines.h"
#include "Engine/Core/Engine.h"
#include "Engine/Core/RenderPasses/Storage/Basics.h"
#include "Engine/Core/RenderPasses/Storage/Passes.h"
#include "Engine/Core/System/Haptic.h"
#include "Engine/imgui/ImGuizmo.h"
#include "Engine/Types/CoreSystems.h"
#include "Engine/Util/TimeAnalysis.h"
#include "glm/gtc/quaternion.hpp"


using namespace WEngine;

RenderHandler::RenderHandler()
{
	InitSDL();

	Iris::InitDesc desc{};
	desc.window = m_window;

	if (!Iris::Init(desc))
	{
		WLog::SetConsoleError();
		WLog::ConsoleLog("FATAL ERROR! GPU failed to initialize, aborting!");
		abort();
	}
	InitImGui();
	Iris::ConfigureImGui();
	CreateBasics();
	CreatePasses();
	m_textureTables.push_back({}); // dummy because UIDs are 1 ordered

	CoreSystems::GetAssetRepo()->LoadAllGPUAssets();

	m_projection = glm::perspective(
		glm::radians(60.0f),
		m_windowResolution.x / m_windowResolution.y,
		0.01f,
		1000.0f
		);
}

void RenderHandler::EnableEditorMode(const Vector2& viewportResolution)
{
	m_isEditor = true;
	m_viewportResolution = viewportResolution;

	m_projection = glm::perspective(
		glm::radians(90.0f),
		m_viewportResolution.x / m_viewportResolution.y,
		0.01f,
		1000.0f
		);
}

Iris::FramebufferHandle RenderHandler::EditorGetViewportFramebuffer()
{
	return Rendering::Passes::forward->GetFb();
}


void RenderHandler::BeginFrame()
{
	TimeSample sample("RenderHandler::BeginFrame");
	Iris::BeginFrame();
	Iris::AcquireSwapchainTexture();
	Iris::ImGuiNewFrame();
	ImGuizmo::BeginFrame();

	Vector3 camPos = m_camera.position;
	Quaternion camRot = m_camera.rotation;

	glm::quat q(camRot.w, camRot.x, camRot.y, camRot.z);

	m_viewMatrix = glm::mat4_cast(glm::conjugate(q));
	m_viewMatrix = glm::translate(m_viewMatrix, glm::vec3(-camPos.x, camPos.y, -camPos.z));
}

void RenderHandler::RenderFrame()
{
	TimeSample sample("RenderHandler::RenderFrame");

	Rendering::Passes::forward->Render();
	Rendering::Passes::normal->Render();
	Rendering::Passes::gtao->Render();
	Rendering::Passes::screen->Render();

	Iris::Present();
	m_renderedCamera = m_camera;
	m_renderQueue.clear();
	m_renderPlanQueue.clear();
	Iris::EndFrame();
	m_currentBoundTexture = 0;
}

void RenderHandler::RenderSingleMission(const RenderMission& mission, const glm::mat4& vp,
	Iris::CommandBufferHandle cmdBuff, Iris::GraphicsPipelineHandle singlePipe, bool noTex)
{
	TimeSample sample("RenderHandler::RenderSingleMission");
	if (!CoreSystems::GetAssetRepo()->IsTextureDoneLoading(mission.textureUID))
		return;
	MeshAssetMission meshMission{};
	meshMission.uid = mission.meshUID;
	CoreSystems::GetAssetRepo()->GetAsset(meshMission);

	sizeT indexSize = sizeof(uint32);
	sizeT vertexSize = sizeof(float32) * 3 + sizeof(float32) * 3 + sizeof(float32) * 2;

	sizeT indexCount = (meshMission.model.indexSize - meshMission.model.indexOffset) / indexSize;
	sizeT indexOffset = meshMission.model.indexOffset / indexSize;
	sizeT vertOffset = meshMission.model.vertexOffset / vertexSize;

	Mat4x4 mvp = Glm4x4ToMat4x4(vp * CalcModelMatrixGLM(mission.transform));
	Mat4x4 model = CalcModelMatrix(mission.transform);

	struct PushConstants
	{
		Mat4x4 mvp;
		Mat4x4 model;
	} pushConstants{};
	pushConstants.mvp = mvp;
	pushConstants.model = model;

	if (mission.textureUID != m_currentBoundTexture && !noTex)
		Iris::BindResourceTable(cmdBuff, singlePipe, 0, m_textureTables[mission.textureUID]);
	m_currentBoundTexture = mission.textureUID;
	Iris::SetPushConstants(cmdBuff, singlePipe, (byte*)&pushConstants, sizeof(pushConstants));
	Iris::DrawIndexed(cmdBuff, indexCount, 1, indexOffset, vertOffset, 0);
}

void RenderHandler::RenderSinglePlan(const RenderPlan &plan, const Mat4x4 &vp,  Iris::CommandBufferHandle cmdBuff,
	Iris::GraphicsPipelineHandle statPipe, bool noTex)
{
	TimeSample sample("RenderHandler::RenderSinglePlan");
	Iris::BindVertexBuffers(cmdBuff, 1, {plan.statBuffer}, {0});
	for (const auto& part : plan.parts)
	{
		if (!CoreSystems::GetAssetRepo()->IsTextureDoneLoading(part.textureUID))
			continue;
		MeshAssetMission meshMission{};
		meshMission.uid = part.meshUID;
		CoreSystems::GetAssetRepo()->GetAsset(meshMission);

		sizeT indexSize = sizeof(uint32);
		sizeT vertexSize = sizeof(float32) * 3 + sizeof(float32) * 3 + sizeof(float32) * 2;

		sizeT indexCount = (meshMission.model.indexSize - meshMission.model.indexOffset) / indexSize;
		sizeT indexOffset = meshMission.model.indexOffset / indexSize;
		sizeT vertOffset = meshMission.model.vertexOffset / vertexSize;

		if (part.textureUID != m_currentBoundTexture && !noTex)
			Iris::BindResourceTable(cmdBuff, statPipe, 0, m_textureTables[part.textureUID]);
		m_currentBoundTexture = part.textureUID;
		Iris::SetPushConstants(cmdBuff, statPipe, (byte*)&vp, sizeof(vp));
		Iris::DrawIndexed(cmdBuff, indexCount, part.count, indexOffset, vertOffset, part.offset);
	}
}


void RenderHandler::UpdateCamera(const Transform &trans)
{
	m_camera = trans;
}

void RenderHandler::UpdateCamera(const Vector3& position, const Quaternion& rotation)
{
	m_camera.position = position;
	m_camera.rotation = rotation;
}

void RenderHandler::UpdateCameraColor(const Color& backColor)
{
	m_camColor = backColor;
}

const Transform & RenderHandler::GetCamera() const
{
	return m_camera;
}

void RenderHandler::AddToRenderQueue(RenderMission& mission)
{
	if (mission.meshUID == 0 || mission.textureUID == 0)
		return;
	m_renderQueue.push_back(mission);
}

void RenderHandler::AddPlanToRenderQueue(RenderPlan& plan)
{
	if (plan.statBuffer == 0)
		return;
	m_renderPlanQueue.push_back(plan);
}

void RenderHandler::RegisterTexture(Iris::TextureHandle handle)
{
	auto table = Iris::CreateResourceTable(Rendering::Basics::singleTexLayout);

	Iris::ResourceTableUpdateDesc desc{};
	Iris::ResourceTableWrite write{};

	write.binding = 0;
	write.type = Iris::ResourceTableEntryType::Texture;
	write.texture = handle;
	write.sampler = Rendering::Basics::sampler;

	desc.writes.push_back(write);
	Iris::UpdateResourceTable(table, desc);

	// this assumes that the asset repo sends them in order. If not, then we will know exactly when this
	// needs to be addressed.
	m_textureTables.push_back(table);
}

void RenderHandler::RenderScene(Iris::CommandBufferHandle cmdBuff, Iris::GraphicsPipelineHandle singlePipe,
	Iris::GraphicsPipelineHandle statPipe, bool noTex)
{
	auto vpGLM = m_projection * m_viewMatrix;

	wtl::vector<Iris::BufferHandle> vertBuffs{CoreSystems::GetAssetRepo()->GetVertexBuffer()};
	wtl::vector<sizeT> vertOffs{0};

	Mat4x4 vp = Glm4x4ToMat4x4(vpGLM);

	// plans are instanced batches, they need the instanced pipeline.
	Iris::BindGraphicsPipeline(cmdBuff, statPipe);
	Iris::BindVertexBuffers(cmdBuff, 0, vertBuffs, vertOffs);
	Iris::BindIndexBuffer(cmdBuff, CoreSystems::GetAssetRepo()->GetIndexBuffer(), 0);

	for (const auto& plan : m_renderPlanQueue)
		RenderSinglePlan(plan, vp, cmdBuff, statPipe, noTex);

	// missions are singular objects, they take the model matrix as a push constant.
	Iris::BindGraphicsPipeline(cmdBuff, singlePipe);
	Iris::BindVertexBuffers(cmdBuff, 0, vertBuffs, vertOffs);
	Iris::BindIndexBuffer(cmdBuff, CoreSystems::GetAssetRepo()->GetIndexBuffer(), 0);

	for (const auto& mission : m_renderQueue)
		RenderSingleMission(mission, vpGLM, cmdBuff, singlePipe, noTex);
}
Mat4x4 RenderHandler::CalcModelMatrix(const Transform &transform)
{
	return Glm4x4ToMat4x4(CalcModelMatrixGLM(transform));
}

void RenderHandler::CreateBasics()
{
	float32 screen[] = {
		//  pos          uv
		-1.0f, -1.0f,  0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
		-1.0f,  1.0f,  0.0f, 1.0f,
		 1.0f,  1.0f,  1.0f, 1.0f
	};

	Iris::BufferDesc buffDesc{};
	buffDesc.debugName = "Screen Mesh";
	buffDesc.usage = Iris::BufferUsage::Vertex;
	buffDesc.size = sizeof(screen);
	Rendering::Basics::screenMesh = Iris::CreateBuffer(buffDesc, (byte*)screen, sizeof(screen));

	Iris::ResourceTableLayoutDesc desc{};
	desc.debugName = "Main Table Layout";

	Iris::ResourceTableLayoutEntry texEntry{};
	texEntry.binding = 0;
	texEntry.stages = Iris::ShaderStage::Fragment;
	texEntry.type = Iris::ResourceTableEntryType::Texture;
	texEntry.count = 1; // one texture per table
	desc.entries.push_back(texEntry);

	Rendering::Basics::singleTexLayout = Iris::CreateResourceTableLayout(desc);

	Iris::SamplerDesc samplerDesc{};
	samplerDesc.debugName = "Main Sampler";
	Rendering::Basics::sampler = Iris::CreateSampler(samplerDesc);
}

void RenderHandler::CreatePasses()
{
	Rendering::Passes::forward = WAllocator::Construct<Rendering::ForwardPass>();
	Rendering::Passes::normal = WAllocator::Construct<Rendering::NormalPass>();
	Rendering::Passes::gtao = WAllocator::Construct<Rendering::GTAOPass>();
	Rendering::Passes::screen = WAllocator::Construct<Rendering::ScreenPass>();

	Rendering::Passes::forward->SetupPass();
	Rendering::Passes::normal->SetupPass();
	Rendering::Passes::gtao->SetupPass();
	Rendering::Passes::screen->SetupPass();
}

const glm::mat4& RenderHandler::GetProjectionMatrix() const
{
	return m_projection;
}

const glm::mat4 & RenderHandler::GetViewMatrix() const
{
	return m_viewMatrix;
}

const Transform& RenderHandler::GetRenderedCameraTransform() const
{
	return m_renderedCamera;
}

glm::mat4 RenderHandler::CalcModelMatrixGLM(const Transform &transform)
{
	glm::quat q(transform.rotation.w, transform.rotation.x,
				transform.rotation.y, transform.rotation.z);

	glm::mat4 modelMatrix = glm::mat4_cast(q);

	modelMatrix[0] *= transform.size.x;
	modelMatrix[1] *= transform.size.y;
	modelMatrix[2] *= transform.size.z;

	modelMatrix[3] = glm::vec4(transform.position.x, -transform.position.y, transform.position.z, 1.0f);
	return modelMatrix;
}

// just so i can close this part of code.
namespace WEngine
{
	void RenderHandler::InitSDL()
	{
		SDL_InitFlags initFlags = SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_GAMEPAD;
		SDL_SetHint(SDL_HINT_IME_IMPLEMENTED_UI, "1");

		if (!SDL_Init(initFlags))
		{
			WLog::SetConsoleError();
			WLog::ConsoleLog(std::format("SDL Initialisation failed: {}", SDL_GetError()));
			abort();
		}

		SDL_DisplayID display = SDL_GetPrimaryDisplay();
		m_displayMode = const_cast<SDL_DisplayMode*>(SDL_GetCurrentDisplayMode(display));

		Uint32 windowFlags = SDL_WINDOW_BORDERLESS;
	#if GPU_BACKEND == GPU_VULKAN
		windowFlags |= SDL_WINDOW_VULKAN;
	#endif


		if (m_displayMode != nullptr)
		{
			if (Engine::GetCla().customResolution)
			{
				m_windowResolution.x = (float32)Engine::GetCla().width;
				m_windowResolution.y = (float32)Engine::GetCla().height;
			}
			else
			{
				// Fullscreen game: adopt the display's native resolution. Keeps ImGui, input and
				// rendering 1:1 with the panel, otherwise UI ends up tiny (e.g. Steam Deck).
				m_windowResolution.x = (float32)m_displayMode->w;
				m_windowResolution.y = (float32)m_displayMode->h;
			}
		}
		else
		{
			WLog::SetConsoleWarning();
			WLog::ConsoleLog(std::format("Couldnt grab display mode, defaulting back to 800x600: {}", SDL_GetError()));
			m_windowResolution.x = 800;
			m_windowResolution.y = 600;
		}

		// The screen-space passes, the depth target and the swapchain accounting all read
		// EngineSettings::resolution, so keep it in sync with the actual window resolution.
		EngineSettings::resolution = m_windowResolution;

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
			abort();
		}
		WLog::ConsoleLog(std::format("Window opened at resolution {}x{}", m_windowResolution.x, m_windowResolution.y));

		Haptic::Init(m_window);
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

		//Iris::SETTING_ConfigureImGui(m_window);

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
}

