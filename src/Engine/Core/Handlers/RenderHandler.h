#pragma once

#include <SDL3/SDL.h>

#include "Engine/Math/Vector.h"

namespace WEngine
{
	class CameraComponent;
	class RenderHandler
	{
	public:
		RenderHandler();

	private:
		Vector2 m_windowResolution;
		SDL_DisplayMode* m_displayMode = nullptr;
		SDL_Window* m_window = nullptr;

	public:
		void BeginFrame();
		void RenderFrame();

	private:

		void InitSDL();
		void InitImGui();
	};
}
