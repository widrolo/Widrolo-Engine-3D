#pragma once

#include <Engine/Components/Component.h>
#include <Engine/Types/CommonTypes.h>

#include "Engine/Core/World/Sector.h"

namespace WEngine
{
	struct SpawnArgs;
	class Entity;
	class SectorComponent : public Component
	{
	public:
		SectorComponent(Entity* e);

	public:
		void Start() override;

		void LoadSector(std::string sectorName);
		void UnloadSector(std::string sectorName);
		void ShowSector(std::string sectorName);
		void HideSector(std::string sectorName);

		void AddEntity(SpawnArgs args);
		void RemoveEntitySelf();
		void RemoveEntityThisSector(Entity* e);
		void RemoveEntityThisSector(std::string entityName);
		void RemoveEntityAnywhere(Entity* e);
		void RemoveEntityAnywhere(std::string entityName);

	private:
		Sector* m_thisSector;

		COMP_HASH(0x6d70fd3c072de078)
	};

}

