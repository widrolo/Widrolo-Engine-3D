#pragma once

// This file is called EngineDefines.h because it used to be just defines.

#include <string>
#include <Engine/Types/CommonTypes.h>
#include <Engine/Types/Version.h>
#include <Engine/Math/Vector.h>

#define STEAM 0 // setting this to 0 completely disables Steam code, good for changing store fronts

#define GPU_OPENGL 1 // this should ideally be deprecated soon in favour of Vulkan
#define GPU_VULKAN 2 // use this when compiling for targets "Linux", "Android", "BSD"
#define GPU_METAL 3 // use this when compiling for targets "MacOS", "iOS"
#define GPU_D3D12 4 // use this when compiling for targets "Windows", "XboxSeries"
#define GPU_GNM 5 // use this when compiling for targets "Playstation5", "Playstation6"
#define GPU_NVM 6 // use this when compiling for targets "NintendoS1", "NintendoS2", "NintendoS3"

#define GPU_BACKEND GPU_VULKAN

struct EngineSettings
{
	/** This sets the name of the game engine. */
	_GLOBAL_CONST_ std::string engineName = "Widrolo Engine 3D";
	/**
	 *	This is the primary engine version, and different to the game version in the game settings.
	 */
	_GLOBAL_CEX_ WEngine::Version engineVersion{ 0, 1, 0, WEngine::VersionKind::Dev };
	/**
	 *	This sets the resolution of only the output framebuffer, not the window.
	 *	The game engine will always scale it to the window size.
	 */
	//_GLOBAL_CONST_ WEngine::Vector2 resolution{ 960.0f, 540.0f }; // No Idea why constexpr is not working here
	_GLOBAL_CONST_ WEngine::Vector2 resolution{ 1920.0f, 1080.0f }; // No Idea why constexpr is not working here

	/**
	 *  This sets the dead zone for any controller joystick excluding steering accessories.
	 */
	_GLOBAL_CEX_ float32 inputStickDeadzone = 0.25f;
	/**
	 *  This sets threshold needed for the shoulder triggers to be read as pressed.
	 */
	_GLOBAL_CEX_ int16 triggerBoolPressThreshold = 25000; // If the trigger is read as ActionInput

	/**
	 *  This sets V-Sync, eliminating tearing. However, it also caps the frame rate the
	 *  refresh rate of the monitor.
	 */
	_GLOBAL_CEX_ bool enableVSync = true;
	/**
	 * Caps the frame rate.
	 * @note this should never be higher than 5000. Otherwise, semaphores bug out on Vulkan.
	 */
	_GLOBAL_CEX_ float64 maxFrameRate = 120.0f;
	/**
	 *  This dictates how many pixels make up one metre. It's a relic from the past, where
	 *  graphics were rendered with SDL instead of OpenGL, and pixels were the main unit of
	 *  measurement. This should not be touched, and must remain at 100!
	 */
	_GLOBAL_CEX_ uint16 pixelsPerMetre = 100;
	/**
	 *	If true, it will enable rounded corners on Windows 11. This does not work on Linux,
	 *	any Windows version older than Windows 11, or Windows 11 with the basic Microsoft
	 *	display driver.
	 */
	_GLOBAL_CEX_ bool enableRoundedCorners = false; // Only on Windows 11
	/**
	 *	If true, images will be filtered. This should be enabled for painted graphics, and
	 *	disabled for pixel art graphics.
	 */
	_GLOBAL_CEX_ bool useTextureFiltering = false;
	/**
	 *	GL shading language version used for shader compilation.
	 */
	_GLOBAL_CONST_ std::string glslVersion = "#version 430"; // temporary!!! should be 460!!!

#ifdef PACKAGE
	_GLOBAL_CONST_ std::string dataPath = "Data/";
#else
	/**
	 *	This sets the data path for the game data.
	 */
	_GLOBAL_CONST_ std::string dataPath = "../../GameData/Data/";
#endif
	/** This sets the sprite path. */
	_GLOBAL_CONST_ std::string spritePath = "Sprites/";
	/** This sets the sector path. */
	_GLOBAL_CONST_ std::string sectorPath = "Sectors/";
	/** This sets the shader path. */
	_GLOBAL_CONST_ std::string shaderPath = "Shaders/";
	/** This sets the audio path. */
	_GLOBAL_CONST_ std::string audioPath = "Audio/";
	/** This sets the UI sheet path. */
	_GLOBAL_CONST_ std::string uiSheetPath = "UISheets/";
	/** This sets the Behaviour Tree path. */
	_GLOBAL_CONST_ std::string behaviourTreePath = "Behaviours/";
	/** This sets the Behaviour Tree path. */
	_GLOBAL_CONST_ std::string meshPath = "Meshes/";

	/**
	 *	This sets the tick rate for the physics engine. The lower the tick rate (and
	 *	therefore higher update rate), the higher the accuracy of game physics, but also
	 *	lower performance.
	 */
	_GLOBAL_CEX_ float64 physicsTickRate = (1.0f/720.0f);
	/**
	 *	When enabled, the physics bounds will be drawn. However, this will also tank
	 *	performance, as all lines are a separate draw call for now.
	 */
	_GLOBAL_CEX_ bool physicsVisualize = true;
	/** Enables physics. */
	_GLOBAL_CEX_ bool physicsEnabled = true;

	/**
	 *	When deleting the parent of a UI element, the children will not get deleted.
	 *	I have no idea why this should ever be true though, but the option if there!
	 */
	_GLOBAL_CEX_ bool uiPromoteChildrenOnDelete = false;

	/**
	 *	When enabled, the Steam store will be booted alongside the handlers. For this,
	 *	a SteamworksSDK must be placed in the "libs" folder. Please do not manually set
	 *	this, but rather the STEAM macro.
	 */
	_GLOBAL_CEX_ bool steamStore = STEAM;
};

#ifndef GPU_BACKEND
#error "Define a GPU backend before compilation!"
#endif

#if GPU_BACKEND == GPU_OPENGL
#error "OpenGL is deprecated!"
#endif

#if GPU_BACKEND == GPU_METAL
#error "Metal backend is not yet supported!"
#endif

#if GPU_BACKEND == GPU_D3D12
#error "DirectX3D 12 backend is not yet supported!"
#endif

#if GPU_BACKEND == GPU_GNM
#error "Playstation GNM backend is not yet supported!"
#endif

#if GPU_BACKEND == GPU_NVM
#error "Nintendo NVM backend is not yet supported!"
#endif