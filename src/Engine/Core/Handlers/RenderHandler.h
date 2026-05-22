#pragma once

#include <queue>
#include <SDL3/SDL.h>

#include "Engine/Math/Vector.h"
#include "Engine/Types/Rendering/RenderMission.h"
#include "Engine/WTL/deque.h"

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

		CameraComponent* m_camera = nullptr;

		wtl::deque<RenderMission> m_renderQueue;

		glm::mat4 m_projection;
		glm::mat4 m_viewMatrix;

	public:
		void BeginFrame();
		void RenderFrame();

		void RegisterCamera(CameraComponent* camera);

		void AddToRenderQueue(RenderMission& mission);

	private:
		void RenderSingleMission(RenderMission& mission);

		void InitSDL();
		void InitImGui();
	};
}
