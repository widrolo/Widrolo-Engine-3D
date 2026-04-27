#include "Editor.h"

#include <chrono>
#include <cstdlib>
#include <string>

#include <Engine/Core/System/Memory.h>
#include <Engine/Util/Log.h>
#include <Engine/EngineDefines.h>
#include <Engine/Core/Handlers/InputHandler.h>
#include <Engine/Core/Handlers/RenderHandler.h>
#include <Engine/Core/Handlers/AssetRepo.h>
#include <Engine/Types/CoreSystems.h>
#include <Engine/Core/World/Sector.h>

#include <Editor/EditorDefines.h>
#include <Editor/Types/EditorSystems.h>

#include <Editor/Core/Handlers/EditorUIHandler.h>
#include <Editor/Core/Handlers/MenubarHandler.h>
#include <Editor/Core/Handlers/CompSettingsRepo.h>

#include <Engine/imgui/imgui.h>
#include <Engine/imgui/imgui_internal.h>
#include <Engine/imgui/backends/imgui_impl_sdl3.h>
#include <Engine/imgui/backends/imgui_impl_opengl3.h>
#include <Engine/imgui/implot.h>

#include "Editor/Types/EditorState.h"

using namespace WEditor;


Editor::Editor(int argc, char *argv[])
{
	StartEditor();
}

void Editor::StartEditor()
{
	EditorState::EditorMode = true;
    StartupMessage();
    WEngine::WLog::ConsoleLog("--------------- Editor Init ----------------");

    InitHandlers();

    WEngine::WLog::SetConsoleSuccess();
    WEngine::WLog::ConsoleLog("--------------- Editor Done ----------------");

	EditorSystems::isEditorRunning = new bool;
	*EditorSystems::isEditorRunning = true;
	Run();
}

void Editor::StartupMessage()
{
#ifdef DEBUG
    WEngine::WLog::SetConsoleInfo();
    WEngine::WLog::ConsoleLog("Game executable has been compiled in debug mode, expect major performance issues!");
#endif // DEBUG

    WEngine::WLog::ConsoleLog(std::format("{} ver {}", EditorDefines::editorName, EngineSettings::engineVersion.ToString()));
}

template<class T>
void StartHandlerSingle(T** container, T** coreContainer, std::string name)
{
    *container = (T*)WAllocator::Construct<T>();
	*coreContainer = *container;
    if (*container == nullptr)
    {
        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog(std::format("{} creation failed", name));
        std::exit(-1);
    }
}

template<class T>
void StartHandlerSingleEditor(T** container, std::string name)
{
	*container = (T*)WAllocator::Construct<T>();
	if (*container == nullptr)
	{
		WEngine::WLog::SetConsoleError();
		WEngine::WLog::ConsoleLog(std::format("{} creation failed", name));
		std::exit(-1);
	}
}

void Editor::InitHandlers()
{
    StartHandlerSingle<WEngine::InputHandler>(&EditorSystems::inputHandler, &WEngine::CoreSystems::inputHandler, "Input Handler");
    StartHandlerSingle<WEngine::RenderHandler>(&EditorSystems::renderHandler, &WEngine::CoreSystems::renderHandler, "Render Handler");
    StartHandlerSingle<WEngine::AssetRepo>(&EditorSystems::assetRepo, &WEngine::CoreSystems::assetRepo, "Asset Repo");

	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	ImGui::GetStyle().WindowBorderSize = 1;
    StartHandlerSingleEditor<EditorUIHandler>(&EditorSystems::editorUIHandler, "Editor UI Handler");
    StartHandlerSingleEditor<MenubarHandler>(&EditorSystems::menubarHandler, "Menubar Handler");
    StartHandlerSingleEditor<CompSettingsRepo>(&EditorSystems::compSettingsRepo, "Component Settings Repo");

	WAllocator::Construct<WEngine::Sector, const std::string&>("root");
}

void Editor::Run()
{
	std::chrono::steady_clock::time_point lastUpdate;

	bool imguiDemo = true;
	const uint64 cap = static_cast<uint64>((1.0f / EngineSettings::maxFrameRate) * 1000000); // shut up damn compiler!!

	while (*EditorSystems::isEditorRunning)
	{
		// delta time
		auto now = std::chrono::steady_clock::now();
		lastUpdate = now;

		auto frameStart = std::chrono::steady_clock::now();

		EditorSystems::renderHandler->BeginFrame();
		EditorSystems::inputHandler->FetchInput();

		EditorSystems::menubarHandler->Render();
		Dock();
		EditorSystems::editorUIHandler->DrawWidgets();

		WEngine::Sector::m_root->Draw();

		EditorSystems::renderHandler->RenderFrame();

		auto frameEndReal = std::chrono::steady_clock::now();
		auto frameEnd = frameStart + std::chrono::microseconds(cap) - frameEndReal;

		if (frameEnd.count() < 0)
			frameEnd = std::chrono::microseconds(0);

		// 120 fps frame rate limit
		uint32 duration = (uint32)frameEnd.count() / (1000 * 1000);
		SDL_Delay(duration);
	}

    std::exit(EXIT_SUCCESS);
}

void Editor::Dock()
{
	ImGuiWindowFlags dockWindowFlags{};
	dockWindowFlags |= ImGuiWindowFlags_NoResize;
	dockWindowFlags |= ImGuiWindowFlags_NoMove;
	dockWindowFlags |= ImGuiWindowFlags_NoTitleBar;
	dockWindowFlags |= ImGuiWindowFlags_NoCollapse;
	dockWindowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
	dockWindowFlags |= ImGuiWindowFlags_NoFocusOnAppearing;

	ImGui::Begin("MainDockSpace", nullptr, dockWindowFlags);


	auto view = ImGui::GetMainViewport()->Size;
	ImGui::SetWindowPos(ImVec2(0, 20));
	view.y = view.y - 20;
	ImGui::SetWindowSize(view);

	ImGuiID dockspaceId = ImGui::GetID("Dock");
	ImGui::DockSpace(dockspaceId);


	static bool firstTime = true;
	if (firstTime)
	{
		firstTime = false;

		ImGui::DockBuilderRemoveNode(dockspaceId);
		ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_DockSpace);
		ImGui::DockBuilderSetNodeSize(dockspaceId, ImGui::GetMainViewport()->Size);

		// Split dock into left, center, right
		ImGuiID dockMainId = dockspaceId;
		ImGuiID dockIdLeft, dockIdRight, dockIdCenter;
		dockIdLeft = ImGui::DockBuilderSplitNode(dockMainId, ImGuiDir_Left, 0.20f, nullptr, &dockMainId);
		dockIdRight = ImGui::DockBuilderSplitNode(dockMainId, ImGuiDir_Right, 0.25f, nullptr, &dockMainId);
		dockIdCenter = dockMainId;

		ImGuiID dockIdLeftTop, dockIdLeftBottom;
		dockIdLeftTop = ImGui::DockBuilderSplitNode(dockIdLeft, ImGuiDir_Up, 0.33f, nullptr, &dockIdLeftBottom);

		ImGuiID dockIdRightTop, dockIdRightBottom;
		dockIdRightTop = ImGui::DockBuilderSplitNode(dockIdRight, ImGuiDir_Up, 0.5f, nullptr, &dockIdRightBottom);

		ImGui::DockBuilderDockWindow("Loaded Sectors", dockIdLeftTop);
		ImGui::DockBuilderDockWindow("Entity List", dockIdLeftBottom);
		ImGui::DockBuilderDockWindow("Viewport", dockIdCenter);
		ImGui::DockBuilderDockWindow("Entity Properties", dockIdRightTop);
		ImGui::DockBuilderDockWindow("Component Settings", dockIdRightBottom);

		ImGui::DockBuilderFinish(dockspaceId);
	}

	ImGui::End();
}
