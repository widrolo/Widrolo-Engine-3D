#include "Sector.h"

#include <Engine/Types/SpawnArgs.h>
#include <string>
#include <Engine/Types/CoreSystems.h>
#include <Engine/Core/World/Entity.h>
#include <Engine/Core/World/SectorLogic.h>
#include <Engine/Core/Handlers/AssetRepo.h>
#include <Engine/Util/Log.h>

#include <Engine/Core/System/Memory.h>

#include "Editor/Types/EditorState.h"

using namespace WEngine;

Sector::Sector(const std::string& sectorName)
	: m_name(sectorName), m_loaded(true)
{
	LoadArgsFromFile(sectorName);
}

void Sector::SectorInternalTick()
{
	if (m_unloadRequested)
		Unload();
}

void Sector::Start()
{
	for (const auto& entity : m_entities)
		entity->Start();
}

void Sector::Stop()
{
	for (const auto& entity : m_entities)
		entity->Stop();
}

void Sector::Tick(const float32 dt)
{
	if (!m_ticking) return;

	if (m_isRoot)
	{
		for (auto sector : m_sectors)
			sector->Tick(dt);
		return; // root has no entities
	}

	// Iterator was causing problems
	for (const auto& entity : m_entities)
		entity->Tick(dt);
}

void Sector::PhysicsTick(const float32 tr)
{
	if (!m_ticking) return;

	if (m_isRoot)
	{
		for (const auto& sector : m_sectors)
			sector->PhysicsTick(tr);
		return; // root has no entities
	}

	for (const auto& entity : m_entities)
		entity->PhysicsTick(tr);
}

void Sector::Draw()
{
	if (!m_ticking) return;

	if (m_isRoot)
	{
		for (const auto& sector : m_sectors)
			sector->Draw();
		return; // root has no entities
	}

	for (const auto& entity : m_entities)
		entity->Draw();
#if DEBUG
	for (size_t i = 0; i < m_entities.size(); i++)
		m_entities[i]->DrawDebug();
#endif
}

void Sector::AddEntityPost(const SpawnArgs& args)
{
	Entity* e = (Entity*)WAllocator::Construct<Entity>();
	e->Internal_ParentSector(this),
	e->Awake(args);
	m_entities.push_back(e);
}

void Sector::RemoveEntity(Entity* e)
{
	e->EntityDestroy();

	auto it = std::ranges::find(m_entities, e);
	if (it != m_entities.end())
	{
		m_entities.erase(it);
	}
}

void Sector::RequestUnload()
{
	WLog::ConsoleLog(std::format("{} was scheduled for unloading", m_name));
	m_unloadRequested = true;
}

void Sector::ShowSector()
{
	m_ticking = true;
	for (const auto& entity : m_entities)
		if (!entity->m_hasEverStarted)
			entity->Start();
}

Sector* Sector::LoadNewSector(const std::string &name)
{
	auto* s = (Sector*)WAllocator::Construct<Sector, const std::string&>(name);
	m_sectors.push_back(s);
	return s;
}

bool Sector::IsEntityPresent(Entity *e)
{
	for (const auto& entity : m_entities)
	{
		if (entity == e)
			return true;
	}
	return false;
}

bool Sector::IsEntityPresent(const std::string& name)
{
	for (const auto& entity : m_entities)
	{
		if (entity->entityName == name)
			return true;
	}
	return false;
}

void Sector::UnloadEntity(Entity *e)
{
	Entity* deletedEntity = nullptr;
	for (const auto& entity : m_entities)
	{
		if (entity == e)
		{
			entity->Stop();
			entity->EntityDestroy();
			deletedEntity = entity;
			break;
		}
	}
	if (deletedEntity != nullptr)
	{
		m_entities.erase(std::find(m_entities.begin(), m_entities.end(), deletedEntity));
	}
}

void Sector::UnloadEntity(const std::string& name)
{
	Entity* deletedEntity = nullptr;
	for (const auto& entity : m_entities)
	{
		if (entity->entityName == name)
		{
			entity->Stop();
			entity->EntityDestroy();
			deletedEntity = entity;
			break;
		}
	}
	if (deletedEntity != nullptr)
	{
		m_entities.erase(std::find(m_entities.begin(), m_entities.end(), deletedEntity));
	}
}


void Sector::AddEntity(Entity* e)
{
	if (m_isRoot)
	{
		WLog::SetConsoleError();
		WLog::ConsoleLog("Tried to add entity to root sector");
		return;
	}

	m_entities.push_back(e);
}

void Sector::LoadArgsFromFile(const std::string& sectorName)
{
	YamlAssetMission mission;
	mission.name = sectorName;

	CoreSystems::GetAssetRepo()->GetAsset(mission);

	if (mission.root["preload_hints_sprites"])
	{
		const YAML::Node preloadNode = mission.root["preload_hints_sprites"];

		SpriteAssetMission spriteMission;

		for (const auto item : preloadNode) {
			spriteMission.name = item.as<std::string>();
			CoreSystems::GetAssetRepo()->GetAsset(spriteMission);
		}
	}

	wtl::vector<SpawnArgs> args;
	if (mission.root["entities"])
	{
		const YAML::Node entities = mission.root["entities"];

		for (const auto entityNode : entities)
		{
			const YAML::Node data = entityNode.second;

			SpawnArgs arg;

			arg.name = data["name"].as<std::string>();

			const YAML::Node& pos = data["position"];
			arg.transform.position = { pos[0].as<float32>(), pos[1].as<float32>() };
			const YAML::Node& size = data["size"];
			arg.transform.size = { size[0].as<float32>(), size[1].as<float32>() };

			if (data["components"])
			{
				const YAML::Node components = data["components"];
				for (const auto componentNode : components)
				{
					const YAML::Node compData = componentNode.second;
					ComponentArgs ca;
					ca.componentTypeId = compData["component-type"].as<uint16>();
					ca.componentRoot = compData;
					arg.ca.push_back(ca);
				}
			}
			args.push_back(arg);
		}
	}

	for (const auto& arg : args)
	{
		Entity* e = (Entity*)WAllocator::Construct<Entity>();
		e->Internal_ParentSector(this);
		e->Awake(arg);
		m_entities.push_back(e);
	}


	// This is here because it would be a waste to 
	// have it in the main vector.
	// Until we have tests or widgets that need this
	// will stay here. i <3 tech debt
	wtl::vector<SectorLogic*> initLogic;

	if (mission.root["logic"])
	{
		const YAML::Node logic = mission.root["logic"];

		for (const auto entityNode : logic)
		{
			const YAML::Node data = entityNode.second;

			SectorLogic* l = (SectorLogic*)WAllocator::Construct<SectorLogic, Sector*>(this);

			std::string boundType = data["bounds-type"].as<std::string>();

			if (boundType == "init")
				initLogic.push_back(l);

			if (data["logic-def"])
			{
				const YAML::Node def = data["logic-def"];
				l->SelectType((SectorLogicType)def["logic-type"].as<uint16>(), def);
			}
		}
	}

	for (auto il : initLogic)
	{
		il->Execute();
		WAllocator::Destruct(il);
	}

	WLog::SetConsoleInfo();
	WLog::ConsoleLog(std::format("Loaded Sector: {}", sectorName));

}

// This will later also inform the AssetRepo about 
// it being unloaded. For now the AssetRepo is too
// dumb for this, since it has no concept of what
// sector has what data.
void Sector::Unload()
{
	for (const auto entity : m_entities)
	{
		entity->Stop();
		entity->EntityDestroy();
	}
	m_entities.clear();

	std::erase(m_sectors, this);

	WAllocator::Destruct(this);
}

void Sector::Root_StartSector(Sector* sector)
{
	if (!m_isRoot) return;

	sector->Start();
}

void Sector::Root_StopSector(Sector* sector)
{
	if (!m_isRoot) return;

	sector->Stop();
}
