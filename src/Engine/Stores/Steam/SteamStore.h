#pragma once

#include <Engine/EngineDefines.h>

#if STEAM
#include <steam_api.h>

#ifndef STEAM_API_H
#error "Please read the README to include the Steamworks SDK!"
#endif
#endif // STEAM

#include <string>
#include <Engine/Types/CommonTypes.h>

namespace WEngine
{
	enum OverlayWindows
	{
		Friends = 0,
		Community,
		Players,
		Settings,
		OfficialGameGroup,
		Stats,
		Achievements,
	};

	/**
	 * Class responsible for managing Steam store related operations.
	 */
	class SteamStore
	{
	public:
		SteamStore();
		~SteamStore();

	public:
		bool m_initSuccess;				///< Indicates whether the Steam API initialization was successful.
		bool m_usingSteam;				///< Indicates whether Steam integration is enabled or not.
		uint64 m_steamID64;				///< The 64-bit Steam ID of the user.
		std::string m_steamWebAPIKey;	///< The Web API key for Steam.
	private:
#if STEAM
		CSteamID m_steamID;
#endif

	public:
		/**
		 * Retrieves the account name of the current Steam user.
		 * @return A string containing the account name. If Steam is disabled, returns "STEAM DISABLED!".
		 */
		std::string GetSteamAccountName();
		/**
		 * Dispatches callbacks for the Steam API.
		 */
		void DispatchCallbacks();
		/**
		 * Unlocks a Steam achievement for the current user.
		 * @param achievementApiName The name of the achievement as it appears in the API.
		 */
		void UnlockAchievement(std::string achievementApiName);
		/**
		 * Opens a specific Steam overlay window.
		 * @param overlay The type of overlay window to open (see OverlayWindows enum).
		 */
		void OpenOverlay(OverlayWindows overlay);
	};
}
