#pragma once

#include <Engine/Types/CommonTypes.h>

namespace WEditor
{
	class Editor;
}

namespace WEngine
{

	/*
		Core Systems

		The core systems provide an easy way for any
		part of the code to access the core engine
		features.

		This is specifically made for components, as
		they usually require access to the engine
		itself, and only rarely govern game logic.
	*/

	class AssetRepo;
	class PhysicsHandler;
	class RenderHandler;
	class InputHandler;
	class RNGHandler;
	class AudioHandler;
	class WidgetHandler;
	class UIHandler;
	class JobHandler;

	class SteamStore;
	class GameMode;

	class Engine;
	class Sector;
	/**
 	* Core systems of the game engine. Holds pointers to various core engine components
 	* and provides methods to access them.
 	*/
	struct CoreSystems
	{
		friend Engine;
		friend WEditor::Editor;
		friend Sector;
	private:
		// Pointers to core engine systems
		_GLOBAL_ RenderHandler* renderHandler;
		_GLOBAL_ InputHandler* inputHandler;
		_GLOBAL_ AssetRepo* assetRepo;
		_GLOBAL_ PhysicsHandler* physicsHandler;
		_GLOBAL_ RNGHandler* rngHandler;
		_GLOBAL_ AudioHandler* audioHandler;
		_GLOBAL_ WidgetHandler* widgetHandler;
		_GLOBAL_ UIHandler* uiHandler;
		_GLOBAL_ JobHandler* jobHandler;

		_GLOBAL_ bool* isGameRunning; // Setting this to false will stop the game

		_GLOBAL_ SteamStore* steamStore;
		_GLOBAL_ float32 timeScale = 1.0f;
	public:
		static RenderHandler* GetRenderHandler() { return renderHandler; }
		static InputHandler* GetInputHandler() { return inputHandler; }
		static AssetRepo* GetAssetRepo() { return assetRepo; }
		static PhysicsHandler* GetPhysicsHandler() { return physicsHandler; }
		static RNGHandler* GetRNGHandler() { return rngHandler; }
		static AudioHandler* GetAudioHandler() { return audioHandler; }
		static WidgetHandler* GetWidgetHandler() { return widgetHandler; }
		static UIHandler* GetUiHandler() { return uiHandler; }
		static JobHandler* GetJobHandler() { return jobHandler; }

		static SteamStore* GetSteamStore() { return steamStore; }

		/**
	 	 * Check if the game is running.
	 	 * @return Pointer to a boolean indicating whether the game is running.
	 	 */
		static bool* IsGameRunning() { return isGameRunning; }
		/**
		 * Shutdown the game by setting the game running flag to false.
		 */
		static void ShutdownGame() { *isGameRunning = false; }

		static float32 GetTimeScale() { return timeScale; }
		static void SetTimeScale(float32 scale)
		{
			if (scale < 0.0f)
				scale = 0.0f;
			timeScale = scale;
		}
	};
}