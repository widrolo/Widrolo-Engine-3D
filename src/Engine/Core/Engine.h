#pragma once

#include <SDL3/SDL.h>
#include <WGL.h>
#include <string>
#include <Engine/Types/CommonTypes.h>
#include <chrono>

#include "Engine/Util/Time.h"

class Game;
namespace WEngine
{
	class Sector;
	struct CmdLineArgs
	{
		bool customResolution;
		int width;
		int height;
	};
	class Engine
	{
	public:
		Engine(int argc, char* argv[]);
	private:
		Sector* m_rootSector;
		Game* m_game; // you lost it.

		float64 m_physicsTickTimer = 0.0f;
		_GLOBAL_ uint16 m_physicsTickCounterLastFrame;

		_GLOBAL_ float64 m_deltaTime;
		_GLOBAL_ uint64 m_uptime; // in seconds

		// profiler
		_GLOBAL_ float m_frameBegin;
		_GLOBAL_ float m_widgetDraw;
		_GLOBAL_ float m_sectorLoad;
		_GLOBAL_ float m_entityTick;
		_GLOBAL_ float m_physicsTick;
		_GLOBAL_ float m_draw;

		CmdLineArgs m_cla = {};

	public:

		static float64 GetDeltaTime();

		static uint64 GetUptime()		{ return m_uptime; }
		static float32 GetFrameBegin()	{ return m_frameBegin; }
		static float32 GetWidgetDraw()	{ return m_widgetDraw; }
		static float32 GetSectorLoad()	{ return m_sectorLoad; }
		static float32 GetEntityTick()	{ return m_entityTick; }
		static float32 GetPhysicsTick()	{ return m_physicsTick; }
		static float32 GetDraw()		{ return m_draw; }

		static uint16 GetPhysicsTickCounter() { return m_physicsTickCounterLastFrame; }

	private:
		void ParseCommandLine(int argc, char* argv[]);

		void StartGame();
		void StartupMessage();

		void InitHandlers();
		void InitSteam();

		void UnRoundCorners();


		[[noreturn]]
		void Run();
		void Loop_Begin(std::chrono::steady_clock::time_point& last, StopWatch& uptime, std::chrono::time_point<std::chrono::steady_clock>& frameStart);
		void Loop_Tick();
		void Loop_Physics();
		void Loop_Audio();
		void Loop_Draw();
		void Loop_Finish();
		void Loop_Stall(std::chrono::time_point<std::chrono::steady_clock>& frameStart);
	};	
}

