#pragma once

#include <Engine/WTL/map.h>
#include <Engine/WTL/vector.h>
#include <Engine/Types/Physics/OverlapResult.h>
#include <Engine/Types/Nullable.h>

namespace WEngine
{
	struct CoreSystems;
	class Engine;
	class SimulatableObject;
	class PhysicsWatchWidget;
	class PhysicsHandler
	{
		friend Engine;
		friend PhysicsWatchWidget;
	public:
		PhysicsHandler();
	
	private:
		wtl::map<uint64, SimulatableObject*> m_objects;

	public:
		void Tick();

		uint64 MakeSimulatableObject();
		[[nodiscard]] Nullable<SimulatableObject*> GetSimulatableObject(uint64 id);
		void DeleteSimulatableObject(uint64 id);

		[[nodiscard]] wtl::vector<OverlapResult> CheckOverlapping(uint64 id);

	private:
		void Setup();
		void Visualize();
	};
}



