#include "Engine.h"

#include <chrono>
#include <thread>
#include <cstdlib>

#include <Game/Core/Game.h>

#include <Engine/Core/Handlers/InputHandler.h>
#include <Engine/Core/Handlers/RenderHandler.h>
#include <Engine/Core/Handlers/AssetRepo.h>
#include <Engine/Core/Handlers/PhysicsHandler.h>
#include <Engine/Core/Handlers/RNGHandler.h>
#include <Engine/Core/Handlers/AudioHandler.h>
#include <Engine/Core/Handlers/WidgetHandler.h>
#include <Engine/Core/Handlers/JobHandler.h>

#include <Engine/Stores/Steam/SteamStore.h>

#include <Engine/EngineDefines.h>
#include <Game/GameDefines.h>

#include <Engine/Core/World/Sector.h>
#include <Engine/Util/Time.h>
#include <Engine/Util/Log.h>
#include <Engine/Types/CoreSystems.h>
#include <Engine/Core/System/Memory.h>
using namespace WEngine;

#ifdef WE_Windows
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "Winmm.lib")
#endif

Engine::Engine(int argc, char* argv[])
{
	ParseCommandLine(argc, argv);

	StartGame();
}

float64 Engine::GetDeltaTime()
{
	return m_deltaTime;
}

void Engine::ParseCommandLine(int argc, char* argv[])
{
	std::string arg;
	for (int i = 1; i < argc; i++)
	{
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
	}
}

void Engine::StartGame()
{
	StartupMessage();
	WLog::ConsoleLog("--------------- Engine Init ----------------");

	InitHandlers();
	InitSteam();

	WLog::SetConsoleSuccess();
	WLog::ConsoleLog("--------------- Engine Done ----------------");

	CoreSystems::isGameRunning = new bool; // i bet youve never seen this before
	*CoreSystems::isGameRunning = true;
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
void StartHandlerSingle(T** container, std::string name)
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
	StartHandlerSingle<InputHandler>(&CoreSystems::inputHandler, "Input Handler");
	StartHandlerSingle<RenderHandler>(&CoreSystems::renderHandler, "Render Handler");
	StartHandlerSingle<AssetRepo>(&CoreSystems::assetRepo, "Asset Repo");
	StartHandlerSingle<PhysicsHandler>(&CoreSystems::physicsHandler, "Physics Handler");
	StartHandlerSingle<RNGHandler>(&CoreSystems::rngHandler, "RNG Handler");
	StartHandlerSingle<AudioHandler>(&CoreSystems::audioHandler, "Audio Handler");
	StartHandlerSingle<WidgetHandler>(&CoreSystems::widgetHandler, "Widget Handler");
	StartHandlerSingle<JobHandler>(&CoreSystems::jobHandler, "Job Handler");

	m_rootSector = new Sector("root");
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
	std::chrono::steady_clock::time_point lastUpdate;
	std::chrono::time_point<std::chrono::steady_clock> frameStart;
	StopWatch uptime;

	m_rootSector->Start();
	CoreSystems::GetRenderHandler()->InitFallbackShaders();
	uptime.Reset();
	m_physicsTickTimer = 0.0f;
	m_game->PreGameLoop();


	while (*CoreSystems::isGameRunning)
	{
		Loop_Begin(lastUpdate, uptime, frameStart);
		Loop_Tick();
		Loop_Physics();
		Loop_Audio();
		Loop_Draw();
		Loop_Finish();
		Loop_Stall(frameStart);
	}

	std::exit(EXIT_SUCCESS);
}

void Engine::Loop_Begin(std::chrono::steady_clock::time_point& last, StopWatch& uptime, std::chrono::time_point<std::chrono::steady_clock>& frameStart)
{
	StopWatch timings;
	m_uptime = (uint64)uptime.GetTime<TimeUnit::Seconds>();
	timings.Reset();
	// delta time
	auto now = std::chrono::steady_clock::now();
	m_deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(now - last).count() / 1000000.0f;
	last = now;

	frameStart = std::chrono::steady_clock::now();


	m_physicsTickTimer += m_deltaTime;
	CoreSystems::renderHandler->BeginFrame();

	m_game->GameLoopBegin();
	m_frameBegin = timings.GetTime<TimeUnit::Microseconds>();
	timings.Reset();

	m_sectorLoad = timings.GetTime<TimeUnit::Microseconds>();
	timings.Reset();
}

void Engine::Loop_Tick()
{

	StopWatch timings;

	CoreSystems::inputHandler->FetchInput();
	m_game->GameLoopTickEarly();
	m_rootSector->Tick(m_deltaTime * CoreSystems::GetTimeScale());
	m_game->GameLoopTickLate();
	m_entityTick = timings.GetTime<TimeUnit::Microseconds>();
	timings.Reset();
}

void Engine::Loop_Physics()
{
	StopWatch timings;
	m_physicsTickCounterLastFrame = 0;

	m_game->GameLoopPhysicsEarly();
	// this can only happen on boot, and is a bug
	if (m_physicsTickTimer / EngineSettings::physicsTickRate >= 100)
	{
		m_physicsTickTimer = 0.0f;
		goto skipPhysics;
	}
	while (m_physicsTickTimer > EngineSettings::physicsTickRate)
	{
		m_physicsTickTimer -= EngineSettings::physicsTickRate;
		m_rootSector->PhysicsTick(EngineSettings::physicsTickRate * CoreSystems::GetTimeScale());
		CoreSystems::physicsHandler->Tick();
		m_physicsTickCounterLastFrame++;
	}
	skipPhysics:
	m_game->GameLoopPhysicsLate();
	m_physicsTick = timings.GetTime<TimeUnit::Microseconds>();
	timings.Reset();
}

void Engine::Loop_Audio()
{
	m_game->GameLoopAudioEarly();
	CoreSystems::audioHandler->AudioTick();
	m_game->GameLoopAudioLate();
}

void Engine::Loop_Draw()
{
	StopWatch timings;
	m_game->GameLoopWidgetEarly();
	CoreSystems::widgetHandler->DrawWidgets();

	m_widgetDraw = timings.GetTime<TimeUnit::Microseconds>();
	timings.Reset();

	m_game->GameLoopDrawEarly();
	m_rootSector->Draw();
	if constexpr (EngineSettings::physicsEnabled)
	{
		CoreSystems::physicsHandler->Visualize();
	}

	m_game->GameLoopDrawJustBefore();
	CoreSystems::renderHandler->RenderFrame();
	m_game->GameLoopDrawLate();
	m_draw = timings.GetTime<TimeUnit::Microseconds>();
	timings.Reset();
}

void Engine::Loop_Finish()
{
	StopWatch timings;
	CoreSystems::steamStore->DispatchCallbacks();

	m_game->GameLoopFinish();


}

void Engine::Loop_Stall(std::chrono::time_point<std::chrono::steady_clock>& frameStart)
{
	auto frameEndReal = std::chrono::steady_clock::now();
	auto frameEnd = frameStart - frameEndReal + std::chrono::microseconds(cap);

	if (frameEnd.count() < 0)
		frameEnd = std::chrono::microseconds(0);

	uint32 sleepMs = (uint32)frameEnd.count() / (1000 * 1000);
	if (sleepMs > 1) SDL_Delay(sleepMs - 1);

	// Spin for the remainder, this makes sure that the frame rate is rock solid on the cap.
	while (std::chrono::steady_clock::now() < frameStart + std::chrono::microseconds(cap));
}

