#pragma once
#include <Engine/WTL/vector.h>
#include <fstream>

#include <Engine/Types/CommonTypes.h>

namespace WEditor
{
	class Editor;
	class SectorList;
	class EntityList;
	class ComponentList;
	class ComponentSettings;
}

namespace WEngine
{
	class Entity;
	struct SpawnArgs;
	class SectorLogic;
	class SectorWatchWidget;

	struct SectorLogicBound
	{
		SectorLogic* logic;
	};
	class Sector
	{
		// i mean, at this point i might just make everything public.
		friend SectorLogic;
		friend SectorWatchWidget;
		friend WEditor::Editor;
		friend WEditor::SectorList;
		friend WEditor::EntityList;
		friend WEditor::ComponentList;
		friend WEditor::ComponentSettings;
	public:
		Sector(const std::string& sectorName);
	private:
		std::string m_name;
		wtl::vector<Entity*> m_entities;

		// only available for root sector
		_GLOBAL_ wtl::vector<Sector*> m_sectors;
		_GLOBAL_ bool m_anyRootExists;
		_GLOBAL_ Sector* m_root;
		bool m_isRoot = false;

		bool m_loaded = false;
		bool m_ticking = false;

		bool m_unloadRequested = false;
		bool m_changedInEditor = false;
	public:
		void SectorInternalTick();

		void Start();
		void Stop();
		void Tick(float32 dt);
		void PhysicsTick(float32 tr);
		void Draw();
		void AddEntityPost(const SpawnArgs& args);
		void RemoveEntity(Entity* e);

		void RequestUnload();

		void ShowSector();

		static Sector* LoadNewSector(const std::string& name);

		bool IsEntityPresent(Entity* e);
		bool IsEntityPresent(const std::string& name);
		void UnloadEntity(Entity* e);
		void UnloadEntity(const std::string& name);
	private:
		void AddEntity(Entity* e);

		void LoadArgsFromFile(const std::string& sectorName); // this should be in Asset Repo

		void Unload();

		void Root_StartSector(Sector* sector);
		void Root_StopSector(Sector* sector);
	};
}