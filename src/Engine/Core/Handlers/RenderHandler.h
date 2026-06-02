#pragma once

#include <queue>
#include <SDL3/SDL.h>

#include "Engine/Math/Vector.h"
#include "Engine/Types/Rendering/RenderMission.h"
#include "Engine/WTL/deque.h"
#include "Engine/WTL/list.h"

namespace WEngine
{
	class CameraComponent;
	class RenderHandler
	{
	public:
		RenderHandler();

	private:

		struct ModelGroup
		{
			Model groupID;
			wtl::vector<RenderMission> models;
		};

		struct ShaderGroup
		{
			Shader groupID;
			wtl::vector<ModelGroup> models;
		};

		struct StatinaryObjStaged
		{
			Model model;
			Shader shader;
			Transform transform;
		};

		Vector2 m_windowResolution;
		SDL_DisplayMode* m_displayMode = nullptr;
		SDL_Window* m_window = nullptr;

		CameraComponent* m_camera = nullptr;

		wtl::deque<RenderMission> m_renderQueue;
		wtl::deque<StatinaryObjStaged> m_stationaryAddQueue;

		// first vector is sorted by shader, second sorts missions by model.
		wtl::vector<ShaderGroup> m_sortedMissions;

		glm::mat4 m_projection;
		glm::mat4 m_viewMatrix;

	public:
		void BeginFrame();
		void RenderFrame();

		void RegisterCamera(CameraComponent* camera);

		void AddToRenderQueue(RenderMission& mission);

		void RecordStationaryAdd(Model model, Shader shader, const Transform& transform);

	private:
		Mat4x4 CalcModelMatrix(const Transform& transform);
		Mat4x4 CalcMVPMatrix(const Transform& transform);

		void RenderSingleMission(RenderMission& mission);
		void RenderModelGroup(const ModelGroup& group, Shader shader);

		void SortMissions();
		void InsertModelIntoShaderGroup(RenderMission& mission, ShaderGroup& shaderGroup);
		void CleanSortedMissions();

		void InitSDL();
		void InitImGui();
	};
}
