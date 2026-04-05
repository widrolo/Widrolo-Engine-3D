#pragma once

#include <string>
#include <Engine/Types/CommonTypes.h>
#include <Engine/Types/Version.h>

struct GameSettings
{
	_GLOBAL_CONST_ std::string gameName = "Test Game";
	_GLOBAL_CEX_ WEngine::Version gameVersion{ 0, 0, 0, WEngine::VersionKind::Dev };
};