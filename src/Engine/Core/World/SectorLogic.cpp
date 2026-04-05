#include <Engine/Core/World/SectorLogic.h>
#include <Engine/Util/Log.h>
#include <Engine/Core/World/Sector.h>

#include <Engine/Core/System/Memory.h>


using namespace WEngine;

SectorLogic::SectorLogic(Sector* sector)
{
	m_sector = sector;
}

void SectorLogic::Execute()
{
	switch (m_logicType)
	{
	case SectorLogicType::BecomeRoot: Logic_BecomeRoot(); break;
	case SectorLogicType::LoadSector: Logic_LoadSector(); break;
	case SectorLogicType::UnloadSector: Logic_UnloadSector(); break;
	case SectorLogicType::ShowSector: Logic_ShowSector(); break;
	case SectorLogicType::HideSector: Logic_HideSector(); break;
	default:
		break;
	}
}

void SectorLogic::SelectType(SectorLogicType type, YAML::Node node)
{
	m_logicType = type;
	m_node = node;
}

void SectorLogic::Logic_BecomeRoot()
{
	//WConsoleLog("Logic_BecomeRoot");
	if (Sector::m_anyRootExists)
		return;

	m_sector->m_root = m_sector;
	m_sector->m_anyRootExists = true;
	m_sector->m_isRoot = true;
	m_sector->m_ticking = true;
}

void SectorLogic::Logic_LoadSector()
{
	//WConsoleLog("Logic_LoadSector");
	if (m_node["sector"])
	{
		//WConsoleLog("%S", m_node["sector"].as<std::string>());
		Sector* s = (Sector*)WAllocator::Construct<Sector, const std::string&>(m_node["sector"].as<std::string>());
		//Sector* s = new Sector(m_node["sector"].as<std::string>());
		Sector::m_root->m_sectors.push_back(s);
		
	}
	else
	{
		WLog::SetConsoleError();
		WLog::ConsoleLog("Tried load a sector without a sector name");
	}
}

void SectorLogic::Logic_UnloadSector()
{
	//WConsoleLog("Logic_UnloadSector");
	if (m_node["sector"])
	{
		//WConsoleLog("%S", m_node["sector"].as<std::string>());
		std::string sname = m_node["sector"].as<std::string>();
		for (auto sector : Sector::m_root->m_sectors)
		{
			if (sector->m_name == sname)
			{
				Sector::m_root->RequestUnload();
				return;
			}
		}
	}
	else
	{
		WLog::SetConsoleError();
		WLog::ConsoleLog("Tried load a sector without a sector name");
	}
}

void SectorLogic::Logic_ShowSector()
{
	//WConsoleLog("Logic_ShowSector");
	if (m_node["sector"])
	{
		//WConsoleLog("%S", m_node["sector"].as<std::string>());
		std::string sname = m_node["sector"].as<std::string>();
		for (auto sector : Sector::m_root->m_sectors)
		{
			if (sector->m_name == sname)
			{
				Sector::m_root->Root_StartSector(sector);
				sector->m_ticking = true;
				return;
			}
		}
	}
	else
	{
		WLog::SetConsoleError();
		WLog::ConsoleLog("Tried load a sector without a sector name");
	}
}

void SectorLogic::Logic_HideSector()
{
	//WConsoleLog("Logic_HideSector");
	if (m_node["sector"])
	{
		//WConsoleLog("%S", m_node["sector"].as<std::string>());
		std::string sname = m_node["sector"].as<std::string>();
		for (auto sector : Sector::m_root->m_sectors)
		{
			if (sector->m_name == sname)
			{
				Sector::m_root->Root_StopSector(sector);
				sector->m_ticking = false;
				return;
			}
		}
	}
	else
	{
		WLog::SetConsoleError();
		WLog::ConsoleLog("Tried load a sector without a sector name");
	}
}
