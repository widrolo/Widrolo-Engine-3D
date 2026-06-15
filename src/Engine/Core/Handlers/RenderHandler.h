#pragma once

#include <queue>
#include <SDL3/SDL.h>

#include "Engine/Math/Vector.h"
#include "Engine/Types/Rendering/InstanceData.h"
#include "Engine/Types/Rendering/RenderMission.h"
#include "Engine/Types/Rendering/GPU/Material.h"
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
			wtl::vector<RenderMission> missions;
		};

		struct MaterialGroup
		{
			Material groupID;
			wtl::vector<ModelGroup> models;
		};

		struct StationaryObjStaged
		{
			Model model;
			Material material;
			wtl::vector<InstanceData> instData;
		};

		struct StationaryRenderMission
		{
			Model model;
			Material material;
		};

		Vector2 m_windowResolution;
		SDL_DisplayMode* m_displayMode = nullptr;
		SDL_Window* m_window = nullptr;

		CameraComponent* m_camera = nullptr;

		wtl::deque<RenderMission> m_renderQueue;
		wtl::deque<StationaryObjStaged> m_stationaryAddQueue;

		// first vector is sorted by shader, second sorts missions by model.
		wtl::vector<MaterialGroup> m_sortedMissions;
		wtl::vector<StationaryRenderMission> m_stationaryMissions;

		glm::mat4 m_projection;
		glm::mat4 m_viewMatrix;

	public:
		void BeginFrame();
		void RenderFrame();

		void RegisterCamera(CameraComponent* camera);

		void AddToRenderQueue(RenderMission& mission);

		void RecordStationaryAdd(Model model, Material material, const Transform& transform);
		void PushStationaryData();
	private:
		Mat4x4 CalcModelMatrix(const Transform& transform);
		Mat4x4 CalcMVPMatrix(const Transform& transform);

		void RenderSingleMission(RenderMission& mission);
		void RenderModelGroup(const ModelGroup& group, Material material);

		void SortStationary(RenderMission& mission);

		void InsertModelIntoShaderGroup(RenderMission& mission, MaterialGroup& materialGroup);

		void SortMissions();
		void CleanSortedMissions();

		void InitSDL();
		void InitImGui();
	};
}
