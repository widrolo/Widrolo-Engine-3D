#include "SectorComponent.h"
#include <Engine/Types/CoreSystems.h>
#include <Engine/Core/World/Entity.h>

#include "Engine/Types/SpawnArgs.h"

using namespace WEngine;

REGISTER_COMPONENT(SectorComponent)

SectorComponent::SectorComponent(Entity* e)
{
	COMP_SETUP("Sector Component")
}

void SectorComponent::Start()
{
	m_thisSector = entity->parentSector;
}

void SectorComponent::LoadSector(std::string sectorName)
{

}

void SectorComponent::UnloadSector(std::string sectorName)
{

}

void SectorComponent::ShowSector(std::string sectorName)
{

}

void SectorComponent::HideSector(std::string sectorName)
{

}

void SectorComponent::AddEntity(SpawnArgs args)
{

}

void SectorComponent::RemoveEntitySelf()
{
	m_thisSector->RemoveEntity(entity);
}

void SectorComponent::RemoveEntityThisSector(Entity *e)
{
	if (m_thisSector->IsEntityPresent(e))
	{
		m_thisSector->UnloadEntity(e);
	}
}

void SectorComponent::RemoveEntityThisSector(std::string entityName)
{
	if (m_thisSector->IsEntityPresent(entityName))
	{
		m_thisSector->UnloadEntity(entityName);
	}
}

void SectorComponent::RemoveEntityAnywhere(Entity *e)
{

}

void SectorComponent::RemoveEntityAnywhere(std::string entityName)
{

}

