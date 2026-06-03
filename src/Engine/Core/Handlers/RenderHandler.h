#pragma once

#include <queue>
#include <SDL3/SDL.h>

#include "Engine/Math/Vector.h"
#include "Engine/Types/Rendering/InstanceData.h"
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

		struct StationaryObjStaged
		{
			Model model;
			Shader shader;
			wtl::vector<InstanceData> instData;
		};

		struct StationaryRenderMission
		{
			Model model;
			Shader shader;
		};

		Vector2 m_windowResolution;
		SDL_DisplayMode* m_displayMode = nullptr;
		SDL_Window* m_window = nullptr;

		CameraComponent* m_camera = nullptr;

		wtl::deque<RenderMission> m_renderQueue;
		wtl::deque<StationaryObjStaged> m_stationaryAddQueue;

		// first vector is sorted by shader, second sorts missions by model.
		wtl::vector<ShaderGroup> m_sortedMissions;
		wtl::vector<StationaryRenderMission> m_stationaryMissions;

		glm::mat4 m_projection;
		glm::mat4 m_viewMatrix;

	public:
		void BeginFrame();
		void RenderFrame();

		void RegisterCamera(CameraComponent* camera);

		void AddToRenderQueue(RenderMission& mission);

		void RecordStationaryAdd(Model model, Shader shader, const Transform& transform);
		void PushStationaryData();
	private:
		Mat4x4 CalcModelMatrix(const Transform& transform);
		Mat4x4 CalcMVPMatrix(const Transform& transform);

		void RenderSingleMission(RenderMission& mission);
		void RenderModelGroup(const ModelGroup& group, Shader shader);

		void SortMissions();
		void SortStationary(const RenderMission& mission);
		void InsertModelIntoShaderGroup(RenderMission& mission, ShaderGroup& shaderGroup);
		void CleanSortedMissions();

		void InitSDL();
		void InitImGui();
	};
}
