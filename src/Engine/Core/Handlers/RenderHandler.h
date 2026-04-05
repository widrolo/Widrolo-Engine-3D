#pragma once

#include <SDL3/SDL.h>

#include <../../Types/Rendering/RenderMission.h>
#include <Engine/Types/CommonTypes.h>
#include <../../Types/Rendering/GPU/Shader.h>
#include <glm.hpp>
#include <Engine/WTL/deque.h>

#include "Engine/Types/Nullable.h"
#include "Engine/Types/Rendering/PostProcessShader.h"
#include "Engine/Types/Rendering/GPU/Framebuffer.h"
#include "Engine/Types/Rendering/GPU/Model.h"

namespace WEngine
{
	class CameraComponent;
	class RenderHandler
	{
	public:
		/**
		 * Initializes the RenderHandler, setting up SDL, OpenGL, and ImGui.
		 */
		RenderHandler();

	private:
		uint16 m_screenWidth;
		uint16 m_screenHeight;

		uint16 m_internalWidth = 1920;
		uint16 m_internalHeight = 1080;

		SDL_DisplayMode* m_displayMode = new SDL_DisplayMode();
		SDL_Window* m_window = nullptr;

		Model m_screen;
		Model m_sprite;
		Model m_line;

		Shader m_screenShader;
		Shader m_spriteShader;
		Shader m_lineShader;
		// [1] = shader name; [2] = shader handle
		wtl::vector<std::pair<std::string, Shader>> m_shaderRepo;
		wtl::vector<PostProcessShader> m_ppShaders;

		Framebuffer m_framebuffer;
		std::array<Framebuffer, 2> m_ppFramebuffers;

		CameraComponent* m_currentCamera = nullptr;
		glm::mat4 m_projection;
		glm::mat4 m_view;

		std::array<wtl::deque<RenderMission>, max_uint8> m_renderQueue;
		wtl::deque<RenderVisualizationMission> m_visualizationQueue;

		bool m_hasOverrideScreenSize = false;
		Vector2 m_overrideScreenSize;
	public:
		/**
		 * Initializes fallback shaders for rendering sprites, lines, atlases, and screen elements.
		 */
		void InitFallbackShaders();
		/**
		 * Sets a new camera to be used for rendering.
		 * @param newCamera A pointer to the CameraComponent to use.
		 */
		void SetNewCamera(CameraComponent* newCamera);
		/**
		 * Retrieves the currently active camera.
		 * @return A pointer to the current CameraComponent, or nullptr if no camera is set.
		 */
		CameraComponent* GetCurrentCamera();
		/**
		 * Begins a frame for rendering, clearing the screen and setting up ImGui.
		 */
		void BeginFrame();
		/**
 		 * Adds a render mission to the render queue.
 		 * @param mission The mission to be added.
 		 */
		void AddToRenderQueue(RenderMission mission);
		void AddToVisualizationQueue(RenderVisualizationMission mission);
		/**
		 * Renders a frame, processing the render queue and drawing ImGui elements.
		 */
		void RenderFrame();
		/**
		 * Converts a world space coordinate to screen space.
		 * @param worldSpace The Vector2 containing the world coordinates that will be modified to screen coordinates.
		 */
		void WorldToScreenSpace(Vector2& worldSpace);
		/**
 		 * Retrieves the internal resolution of the rendering target.
 		 * @return A Vector2 representing the width and height of the internal rendering resolution.
 		 */
		Vector2 GetInternalResolution() const;
		/**
		 * Retrieves the window resolution.
		 * @return A Vector2 representing the width and height of the current window size.
		 */
		Vector2 GetWindowResolution() const;

		void ResizeViewport(Vector2 newSize);
		Framebuffer GetGameFramebuffer();

	private:
		void RenderQueue();
		void RenderVisualizationQueue();
		void RenderPostProcessedFrame();

		void InitSDL();
		void InitImGui();

		void RenderSingleMission(RenderMission& mission);
		void RenderSingleVisualization(RenderVisualizationMission& mission);

		Shader FindShader(const std::string& shaderName);

		void SetupVBOs();
		void SetupFramebuffer();
		void SetupPostProcessing();

		void SortSpriteZ(wtl::deque<RenderMission>& toBeSorted);

		void SetupPostProcessingShaders();
	};
}
