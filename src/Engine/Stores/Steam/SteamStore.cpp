#include "SteamStore.h"

#include <cstdlib>
#include <filesystem>
#include <Engine/Util/Log.h>

#include <Engine/Util/Env.h>
#include <sstream>
#include <iostream>

#include "Engine/Core/Handlers/AssetRepo.h"
#include "Engine/Core/System/Memory.h"
#include "Engine/Types/CoreSystems.h"
#include "Engine/Util/TimeAnalysis.h"

using namespace WEngine;

const char* overlayWindowsStr[] = { "friends", "community", "players", "settings", "officialgamegroup", "stats", "achievements" };

// no idea what this is doing
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
	((std::string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}

SteamStore::SteamStore() 
{
#if STEAM
	// maybe hack, maybe not, who knows?
	if (SteamAPI_RestartAppIfNecessary(STEAMAPPID))
	{
		WLog::SetConsoleWarning();
		WLog::ConsoleLog("Steam is relaunching the game, killing this process...");
		std::exit(0);
	}

	if (!SteamAPI_Init())
	{
		WLog::SetConsoleError();
		WLog::ConsoleLog("Steam API Init failed!");
		return;
	}

	m_initSuccess = true;
	
	m_steamID = SteamUser()->GetSteamID();
	m_steamID64 = m_steamID.ConvertToUint64();
	//m_steamWebAPIKey = GetEnv("STEAM_WEB_API_KEY"); // Youll have to put your own in the env variable
#endif // STEAM
}

SteamStore::~SteamStore()
{
#if STEAM
	SteamAPI_Shutdown();
	//SteamInput()->Shutdown();
#endif // STEAM
}

std::string SteamStore::GetSteamAccountName()
{
	TimeSample sample("SteamStore::GetSteamAccountName");
#if STEAM
	return SteamFriends()->GetPersonaName();
#else
	return "STEAM DISABLED!";
#endif // STEAM
}

void SteamStore::DispatchCallbacks()
{
	TimeSample sample("SteamStore::DispatchCallbacks");
#if STEAM
	SteamAPI_RunCallbacks();
#endif // STEAM
}

void SteamStore::UnlockAchievement(std::string achievementApiName)
{
	TimeSample sample("SteamStore::UnlockAchievement");
#if STEAM
	
	if (!m_initSuccess)
		return;

	SteamUserStats()->SetAchievement(achievementApiName.c_str());

	if (!SteamUserStats()->StoreStats())
	{
		WLog::SetConsoleWarning();
		WLog::ConsoleLog(std::format("Achievement {} could not be unlocked!", achievementApiName));
		return;
	}

	WLog::SetConsoleSuccess();
	WLog::ConsoleLog(std::format("Achievement {} unlocked!", achievementApiName));

#endif // STEAM
}

void SteamStore::OpenOverlay(OverlayWindows overlay)
{
	TimeSample sample("SteamStore::OpenOverlay");
#if STEAM
	SteamFriends()->ActivateGameOverlay(overlayWindowsStr[overlay]);
#endif // STEAM
}

bool SteamStore::IsSteamDeck()
{
#if STEAM
	return m_initSuccess && SteamUtils() != nullptr && SteamUtils()->IsSteamRunningOnSteamDeck();
#else
	return false;
#endif // STEAM
}


// For copy and paste
#if STEAM
#endif // STEAM