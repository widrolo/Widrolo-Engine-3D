#include "AssetRepo.h"

#include <Engine/Util/Log.h>
#include <Engine/EngineDefines.h>
#include <Engine/Core/System/OS.h>
#include <fstream>
#include <sstream>

#include <stb_image.h>
#include <yaml-cpp/yaml.h>
#include <tinyxml2/tinyxml2.h>

#include "Engine/Core/System/GPU.h"
#include "Engine/imgui/imgui.h"
#include "Engine/Util/Conversions.h"

using namespace WEngine;

AssetRepo::AssetRepo()
{
	m_dataPath = OS::GetProcessPath();
	WLog::ConsoleLog(std::format("Data path:\n\t{}", m_dataPath));

	// Remove the executable name
#ifdef WE_Windows
	size_t found = m_dataPath.find_last_of("\\"); 
#endif
#ifdef WE_Linux
	size_t found = m_dataPath.find_last_of('/');
#endif
	if (found != std::string::npos) { 
		m_dataPath = m_dataPath.substr(0, found + 1); 
	}

	m_dataPath += EngineSettings::dataPath;

	WLog::SetConsoleInfo();
	WLog::ConsoleLog(std::format("Data path:\n\t{}", m_dataPath));
}

template<>
void AssetRepo::GetAsset<ShaderAssetMission>(ShaderAssetMission& mission) 
{
	mission.vertexShaderSource = LoadTextFile(GetDataPath() + EngineSettings::shaderPath + mission.name + "Vertex.glsl");
	mission.fragmentShaderSource = LoadTextFile(GetDataPath() + EngineSettings::shaderPath + mission.name + "Fragment.glsl");
}

template<>
void AssetRepo::GetAsset<YamlAssetMission>(YamlAssetMission& mission)
{
	const std::string file = LoadTextFile(GetDataPath() + EngineSettings::sectorPath + mission.name + ".yaml");
	mission.root = YAML::Load(file);
}

template<>
void AssetRepo::GetAsset<AtlasInfoAssetMission>(AtlasInfoAssetMission& mission)
{
	const std::string file = LoadTextFile(GetDataPath() + EngineSettings::spritePath + mission.name + ".yaml");
	mission.root = YAML::Load(file);
}

template<>
void AssetRepo::GetAsset<AudioClipAssetMission>(AudioClipAssetMission& mission)
{
	AudioClip* clip = &m_audioRepo[mission.name];
	if (clip->audioBuf == nullptr)
		clip = LoadAudioWAV(mission.name);
	mission.clip = clip;
}

template<>
void AssetRepo::GetAsset<UISheetAssetMission>(UISheetAssetMission& mission)
{
	const std::string file = LoadTextFile(GetDataPath() + EngineSettings::uiSheetPath + mission.name + ".yaml");
	auto descriptor = YAML::Load(file);

	if (!descriptor["uisheet"])
	{
		WLog::SetConsoleError();
		WLog::ConsoleLog(std::format("{} descriptor does not contain Node \"uisheet\"", mission.name));
	}

	auto sheet = descriptor["uisheet"];

	if (!sheet["document"])
	{
		WLog::SetConsoleError();
		WLog::ConsoleLog(std::format("{} descriptor does not contain Node \"document\"", mission.name));
	}

	std::string docPath = GetDataPath() + EngineSettings::uiSheetPath + "Documents/" + sheet["document"].as<std::string>() + ".uidoc";
	mission.document.LoadFile(docPath.c_str());
	if (mission.document.ErrorID() != 0)
	{
		WLog::SetConsoleError();
		WLog::ConsoleLog(std::format("Error while reading UI Sheet document, Line {}:\n{}", mission.document.ErrorLineNum(), mission.document.ErrorStr()));
	}
}

template<>
void AssetRepo::GetAsset<SpirVAssetMission>(SpirVAssetMission& mission)
{
	std::string path = GetDataPath() + EngineSettings::shaderPath + mission.name;
	switch (mission.shaderType)
	{
		case SpirVAssetMission::VertexShader:
			path += "Vertex";
			break;
		case SpirVAssetMission::FragmentShader:
			path += "Fragment";
			break;
		case SpirVAssetMission::GeometryShader:
			path += "Geometry";
			break;
	}
	path += ".spv";
	std::ifstream file(path, std::ios::binary | std::ios::in | std::ios::ate);

	if (!file.is_open())
	{
		WLog::SetConsoleError();
		WLog::ConsoleLog(std::format("Failed to open Spir-V shader:\n\t{}", path));
		mission.shaderSize = 0;
		return;
	}

	mission.shaderSize = file.tellg();
	file.seekg(0, std::ios::beg);

	mission.shaderCode = new uint32[mission.shaderSize / sizeof(uint32)];
	file.read(reinterpret_cast<char*>(mission.shaderCode), mission.shaderSize);
	file.close();
}

AudioClip* AssetRepo::LoadAudioWAV(const std::string& name)
{
	AudioClip clip{};
	SDL_AudioSpec spec;

	const std::string path = GetDataPath() + EngineSettings::audioPath + name + ".wav";

	bool res = SDL_LoadWAV(path.c_str(), &spec, &clip.audioBuf, &clip.audioLen);

	if (!res || spec.freq == 0)
	{
		WLog::SetConsoleError();
		WLog::ConsoleLog(std::format("Failed to load audio clip:\n\t{}", path));
		return nullptr;
	}

	clip.format = spec.format;
	clip.freq = spec.freq;
	clip.channels = spec.channels;

	m_audioRepo[name] = clip;
	return &m_audioRepo[name];
}

std::string AssetRepo::LoadTextFile(const std::string& path)
{
	std::ifstream file(path);

	if (!file.is_open()) 
	{
		WLog::SetConsoleError();
		WLog::ConsoleLog(std::format("Failed to open file:\n\t{}", path));
		return "";
	}

	std::stringstream buffer;
	buffer << file.rdbuf();
	file.close();

	return buffer.str();
}
