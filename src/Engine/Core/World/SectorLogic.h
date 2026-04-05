#pragma once

#include <Engine/Types/CommonTypes.h>
#include <yaml-cpp/yaml.h>

namespace WEngine
{
	enum class SectorLogicType : uint16
	{
		BecomeRoot = 0,
		LoadSector,
		UnloadSector,
		ShowSector,
		HideSector,
	};

	class Sector;
	class SectorLogic
	{
	public:
		SectorLogic(Sector* sector);
	private:
		SectorLogicType m_logicType;
		Sector* m_sector;
		YAML::Node m_node; // dont worry, i dislike this too

	public:
		void Execute();
		void SelectType(SectorLogicType type, YAML::Node node);
	private:
		void Logic_BecomeRoot();
		void Logic_LoadSector();
		void Logic_UnloadSector();
		void Logic_ShowSector();
		void Logic_HideSector();
	};
}



