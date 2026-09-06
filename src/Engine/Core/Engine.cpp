#include "Engine.h"

#include <chrono>
#include <thread>
#include <cstdlib>
#include <xmmintrin.h>

#include <Game/Core/Game.h>

#include <Engine/Core/Handlers/Input.h>
#include <Engine/Core/Handlers/RenderHandler.h>
#include <Engine/Core/Handlers/AssetRepo.h>
#include <Engine/Core/Handlers/PhysicsHandler.h>
#include <Engine/Core/Handlers/RNGHandler.h>
#include <Engine/Core/Handlers/AudioHandler.h>
#include <Engine/Core/Handlers/WidgetHandler.h>
#include <Engine/Core/Handlers/JobHandler.h>
#include <Engine/Core/Handlers/TimeHandler.h>
#include <Engine/Core/Handlers/SectorHandler.h>

#include <Engine/Stores/Steam/SteamStore.h>
#include <Engine/imgui/imgui.h>

#include <Engine/EngineDefines.h>
#include <Game/GameDefines.h>

#include <Engine/Core/World/Sector.h>
#include <Engine/Util/Timer.h>
#include <Engine/Util/Log.h>
#include <Engine/Types/CoreSystems.h>
#include <Engine/Core/System/Memory.h>

#include "Engine/Util/TimeAnalysis.h"
#include "System/Echo.h"
#include "System/Haptic.h"
#include "System/Iris.h"
using namespace WEngine;

#ifdef WE_Windows
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "Winmm.lib")
#endif

StopWatch bootTime;

Engine::Engine(int argc, char* argv[])
{
	ParseCommandLine(argc, argv);

	StartGame();
}

float64 Engine::GetDeltaTime()
{
	return m_deltaTime;
}

void Engine::ParseCommandLine(sizeT argc, char* argv[])
{
	for (sizeT i = 1; i < argc; i++)
	{
		std::string arg = argv[i];
		if (arg == "-resolution")
		{
			m_cla.customResolution = true;
			if (i + 2 < argc)
			{
				m_cla.width = std::stoi(argv[i + 1]);
				m_cla.height = std::stoi(argv[i + 2]);
				i += 2;
			}
			else
			{
				WLog::SetConsoleError();
				WLog::ConsoleLog("Invalid resolution arguments");
				return;
			}
		}
		if (arg == "-texless")
		{
			m_cla.testMode = true;
		}
	}
}

void Engine::StartGame()
{
	bootTime.Reset();
	StartupMessage();
	WLog::ConsoleLog("--------------- Engine Init ----------------");

	InitHandlers();
	InitSteam();

	if (CoreSystems::steamStore != nullptr && CoreSystems::steamStore->IsSteamDeck())
	{
		WLog::SetConsoleInfo();
		WLog::ConsoleLog("Running on a Steam Deck, enabling ImGui DPI scaling.");
		ImGui::GetIO().ConfigDpiScaleFonts = true;
	}

	float64 time = bootTime.GetTime<TimeUnit::Seconds>();
	WLog::SetConsoleInfo();
	WLog::ConsoleLog(std::format("Boot time: {} seconds.", time));

	WLog::SetConsoleSuccess();
	WLog::ConsoleLog("--------------- Engine Done ----------------");


	CoreSystems::isGameRunning = true;
	Run();
}

void Engine::StartupMessage()
{
#ifdef DEBUG
	WLog::SetConsoleInfo();
	WLog::ConsoleLog("Game executable has been compiled in debug mode, expect major performance issues!");
#endif // DEBUG

	WLog::ConsoleLog(std::format("{} ver {}", EngineSettings::engineName, EngineSettings::engineVersion.ToString()));
	WLog::ConsoleLog(std::format("{} ver {}", GameSettings::gameName, GameSettings::gameVersion.ToString()));
}

template<class T>
void StartHandlerSingle(T** container, const std::string& name)
{
	*container = (T*)WAllocator::Construct<T>();
	if (*container == nullptr)
	{
		WLog::SetConsoleError();
		WLog::ConsoleLog(std::format("{} creation failed", name));
		std::exit(-1);
	}
}

void Engine::InitHandlers()
{
	StartHandlerSingle<RNGHandler>(&CoreSystems::rngHandler, "RNG Handler");
	StartHandlerSingle<AssetRepo>(&CoreSystems::assetRepo, "Asset Repo");
	StartHandlerSingle<RenderHandler>(&CoreSystems::renderHandler, "Render Handler");
	StartHandlerSingle<PhysicsHandler>(&CoreSystems::physicsHandler, "Physics Handler");
	StartHandlerSingle<SectorHandler>(&CoreSystems::sectorHanlder, "Sector Handler");
	StartHandlerSingle<AudioHandler>(&CoreSystems::audioHandler, "Audio Handler");
	StartHandlerSingle<WidgetHandler>(&CoreSystems::widgetHandler, "Widget Handler");
	StartHandlerSingle<JobHandler>(&CoreSystems::jobHandler, "Job Handler");
	StartHandlerSingle<TimeHandler>(&CoreSystems::timeHandler, "Time Handler");

	Echo::InitDesc desc{};
	Echo::Init(desc);

	CoreSystems::assetRepo->RegisterAllTextures();
	Input::LoadInputMap();
	m_game = new Game();
}

void Engine::InitSteam()
{
	CoreSystems::steamStore = (SteamStore*)WAllocator::Construct<SteamStore>();
	if (CoreSystems::steamStore == nullptr)
	{
		WLog::SetConsoleError();
		WLog::ConsoleLog("Steam Store creation failed");
		return;
	}
	if (!CoreSystems::steamStore->m_initSuccess)
	{
		WLog::SetConsoleError();
		WLog::ConsoleLog("Steam Store access failed");
		return;
	}
	WLog::ConsoleLog(std::format("Steam User: {}", CoreSystems::steamStore->GetSteamAccountName()));
}

void Engine::UnRoundCorners()
{
#ifdef ewbofdbgoewvbfiuwbv
//#ifdef WE_Windows

	if constexpr (EngineSettings::enableRoundedCorners)
	{
		HWND hwnd = (HWND)SDL_GetPointerProperty(SDL_GetWindowProperties(m_window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
		if (hwnd) 
		{
			DWM_WINDOW_CORNER_PREFERENCE preference = DWMWCP_DONOTROUND;
			DwmSetWindowAttribute(hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, &preference, sizeof(preference));
		}
		else 
		{
			WLog::SetConsoleError();
			WLog::ConsoleLog("Failed to get HWND");
			SDL_Log("Failed to get HWND: %s", SDL_GetError());
			return;
		}
		
		
	}
#endif
}

constexpr uint64 cap = static_cast<uint64>((1.0f / EngineSettings::maxFrameRate) * 1000000);

[[noreturn]]
void Engine::Run()
{
	std::chrono::steady_clock::time_point lastUpdate = std::chrono::steady_clock::now();
	std::chrono::time_point<std::chrono::steady_clock> frameStart;
	StopWatch uptime;

	uptime.Reset();
	m_physicsTickTimer = 0.0f;
	m_game->PreGameLoop();

	TimeSample rootSample("Root", true);
	while (CoreSystems::isGameRunning)
	{
		Loop_Begin(lastUpdate, uptime, frameStart);
		Loop_Tick();
		Loop_Physics();
		Loop_Audio();
		Loop_Draw();
		Loop_Finish();
		rootSample.SaveAndResetRoot();

		Loop_Stall(frameStart);
	}
	Echo::Shutdown();
	std::exit(EXIT_SUCCESS);
}

void Engine::Loop_Begin(std::chrono::steady_clock::time_point& last, StopWatch& uptime, std::chrono::time_point<std::chrono::steady_clock>& frameStart)
{
	TimeSample sample("Engine::Loop_Begin");
	m_uptime = (uint64)uptime.GetTime<TimeUnit::Seconds>();
	// delta time
	auto now = std::chrono::steady_clock::now();
	m_deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(now - last).count() / 1000000.0f;
	last = now;

	frameStart = std::chrono::steady_clock::now();
	m_physicsTickTimer += m_deltaTime;

	CoreSystems::GetAssetRepo()->TickTextureUpload();
	CoreSystems::timeHandler->Update(m_deltaTime * CoreSystems::GetTimeScale());

	m_game->GameLoopBegin(m_deltaTime * CoreSystems::GetTimeScale());

	// This should be in Loop_Tick, but I like seeing input and logic being separate in timings.
	Haptic::FetchInput();
}

void Engine::Loop_Tick()
{
	TimeSample sample("Engine::Loop_Tick");

	m_game->GameLoopTickEarly();
	m_game->GameLoopTick();
	m_game->GameLoopTickLate();
}

void Engine::Loop_Physics()
{
	TimeSample sample("Engine::Loop_Physics");
	m_physicsTickCounterLastFrame = 0;

	m_game->GameLoopPhysicsEarly();
	// this can only happen on boot, and is a bug
	if (!(m_physicsTickTimer > PhysicsSettings::physicsTickRate))
	{
		// This is here because otherwise, it flickers in the engine timing widget.
		{TimeSample sample("Game::GameLoopPhysics");}
		{TimeSample sample("PhysicsHandler::Tick");}
	}
	while (m_physicsTickTimer > PhysicsSettings::physicsTickRate)
	{
		m_physicsTickTimer -= PhysicsSettings::physicsTickRate;
		m_game->GameLoopPhysics();
		CoreSystems::physicsHandler->Tick();
		m_physicsTickCounterLastFrame++;
	}
	skipPhysics:
	m_game->GameLoopPhysicsLate();
}

void Engine::Loop_Audio()
{
	TimeSample sample("Engine::Loop_Audio");
	m_game->GameLoopAudioEarly();
	CoreSystems::audioHandler->AudioTick();
	m_game->GameLoopAudioLate();
}

void Engine::Loop_Draw()
{
	TimeSample sample("Engine::Loop_Draw");
	CoreSystems::renderHandler->BeginFrame();

	m_game->GameLoopWidgetEarly();
	CoreSystems::widgetHandler->DrawWidgets();

	m_game->GameLoopDrawEarly();
	m_game->GameLoopDraw();
	CoreSystems::sectorHanlder->DrawSectors();
	if constexpr (PhysicsSettings::physicsEnabled)
	{
		CoreSystems::physicsHandler->Visualize();
	}

	CoreSystems::renderHandler->RenderFrame();
	m_game->GameLoopDrawLate();
}

void Engine::Loop_Finish()
{
	TimeSample sample("Engine::Loop_Finish");
	CoreSystems::steamStore->DispatchCallbacks();

	m_game->GameLoopFinish();


}

void Engine::Loop_Stall(std::chrono::time_point<std::chrono::steady_clock>& frameStart)
{
	//TimeSample sample("Engine::Loop_Stall");
	const auto target = frameStart + std::chrono::microseconds(cap);
	const auto now = std::chrono::steady_clock::now();
	if (now < target)
	{
		const auto spinWindow = std::chrono::microseconds(1000);
		if (target - now > spinWindow)
			std::this_thread::sleep_until(target - spinWindow);

		while (std::chrono::steady_clock::now() < target)
			_mm_pause();
	}
}

