#include "AssetRepo.h"

#include <Engine/Util/Log.h>
#include <Engine/EngineDefines.h>
#include <Engine/Core/System/OS.h>
#include <fstream>
#include <sstream>
#include <algorithm>

#include <stb_image.h>
#include <chrono>
#include <filesystem>
#include <yaml-cpp/yaml.h>
#include <tinyxml2.h>

#include <shaderc/shaderc.hpp>

#include "RenderHandler.h"
#include "RNGHandler.h"
#include "Engine/Core/System/Iris.h"
#include "Engine/Types/CoreSystems.h"
#include "Engine/Types/Rendering/TextureInfo.h"
#include "Engine/Util/TextureSwizzler.h"
#include "Engine/Types/Rendering/DDS.h"

#include <Engine/Core/World/Sector.h>

#include "Engine/Util/TimeAnalysis.h"
#include "glm/gtc/quaternion.hpp"


using namespace WEngine;

AssetRepo::AssetRepo()
{
	m_dataPath = OS::GetProcessPath();
	WLog::ConsoleLog(std::format("Data path:\n\t{}", m_dataPath));

	// Remove the executable name
#ifdef WE_Windows
	sizeT found = m_dataPath.find_last_of("\\");
#endif
#ifdef WE_Linux
	sizeT found = m_dataPath.find_last_of('/');
#endif
	if (found != std::string::npos) { 
		m_dataPath = m_dataPath.substr(0, found + 1); 
	}

	m_dataPath += EngineSettings::dataPath;

	WLog::SetConsoleInfo();
	WLog::ConsoleLog(std::format("Data path:\n\t{}", m_dataPath));
}

template<>
void AssetRepo::GetAsset<YamlAssetMission>(YamlAssetMission& mission)
{
	TimeSample sample("AssetRepo::GetAsset<YamlAssetMission>");
	const std::string file = LoadTextFile(GetDataPath() + EngineSettings::sectorPath + mission.name + ".yaml");
	mission.root = YAML::Load(file);
}

template<>
void AssetRepo::GetAsset<MeshAssetMission>(MeshAssetMission& mission)
{
	// we dont do this because this gets called all the damn time and i dont want a high res clock call here
	//TimeSample sample("AssetRepo::GetAsset<MeshAssetMission>");
	if (mission.uid == 0 || mission.uid >= m_meshes.size())
	{
		WLog::SetConsoleWarning();
		WLog::ConsoleLog("Gave invalid uid.");
		return;
	}

	mission.model = m_meshes[mission.uid];
}

template<>
void AssetRepo::GetAsset<AudioClipAssetMission>(AudioClipAssetMission& mission)
{
	TimeSample sample("AssetRepo::GetAsset<AudioClipAssetMission>");
	AudioClip* clip = &m_audioRepo[mission.name];
	if (clip->audioBuf == nullptr)
		clip = LoadAudioWAV(mission.name);
	mission.clip = clip;
}

// deprecated???
template<>
void AssetRepo::GetAsset<UISheetAssetMission>(UISheetAssetMission& mission)
{
	TimeSample sample("AssetRepo::GetAsset<UISheetAssetMission>");
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
	TimeSample sample("AssetRepo::GetAsset<SpirVAssetMission>");
#ifdef PACKAGE
	// we will reenable it later. For now it will remain like this.
	//LoadSpirVFromSpv(mission);
	LoadSpirVFromGlsl(mission);
#else
	LoadSpirVFromGlsl(mission);
#endif

}

bool AssetRepo::IsTextureDoneLoading(uint64 uid) const
{
	// We clear the texture done cache in the finalization, so this is
	// necessary
	if (m_texturesDone.empty())
		return true;

	// we trust that the caller has already confirmed that the uid is correct.
	return m_texturesDone[uid] == StreamingProgress::Loaded;
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

void AssetRepo::LoadSpirVFromGlsl(SpirVAssetMission &mission)
{
	std::string path = GetDataPath() + EngineSettings::shaderPath + mission.name;
	shaderc_shader_kind kind = shaderc_glsl_infer_from_source; // just to shut up the compiler.
	switch (mission.shaderType)
	{
		case Iris::ShaderStage::Vertex:
			path += "Vertex";
			kind = shaderc_glsl_vertex_shader;
			break;
		case Iris::ShaderStage::Fragment:
			path += "Fragment";
			kind = shaderc_glsl_fragment_shader;
			break;
		case Iris::ShaderStage::Compute:
			path += "Compute";
			kind = shaderc_glsl_compute_shader;
			break;
		case Iris::ShaderStage::Geometry:
			path += "Geometry";
			kind = shaderc_glsl_geometry_shader;
			break;
		case Iris::ShaderStage::TessControl:
			path += "TessControl";
			kind = shaderc_glsl_tess_control_shader;
			break;
		case Iris::ShaderStage::TessEval:
			path += "TessEval";
			kind = shaderc_glsl_tess_evaluation_shader;
			break;
	}

	path += ".glsl";

	auto t0 = std::chrono::high_resolution_clock::now();
	auto shaderCode = LoadTextFile(path);

	shaderc::Compiler compiler;
	shaderc::CompileOptions options;

	options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);

#ifdef DEBUG
	options.SetOptimizationLevel(shaderc_optimization_level_zero);
	options.SetGenerateDebugInfo();
#else
	options.SetOptimizationLevel(shaderc_optimization_level_performance);
#endif
	auto res = compiler.CompileGlslToSpv(
		shaderCode,
		kind,
		"what.txt",
		options
	);

	if (res.GetCompilationStatus() != shaderc_compilation_status_success)
	{
		WLog::SetConsoleError();
		WLog::ConsoleLog(std::format("Failed to compile shader from GLSL to Spir-V:\n\t{}\n\t{}", path, res.GetErrorMessage()));
		return;
	}
	auto t1 = std::chrono::high_resolution_clock::now();
	WLog::ConsoleLog(std::format("Shader Compilation time: {}ms.",
		std::chrono::duration_cast<std::chrono::milliseconds>(t1-t0).count()));

	sizeT wordCount = res.cend() - res.cbegin();
	mission.shaderCode = wNewArr(uint32, wordCount);
	std::copy(res.cbegin(), res.cend(), mission.shaderCode);
	mission.shaderSize = wordCount * sizeof(uint32);
}

void AssetRepo::LoadSpirVFromSpv(SpirVAssetMission &mission)
{
	std::string path = GetDataPath() + EngineSettings::shaderPath + mission.name;
	switch (mission.shaderType)
	{
		case Iris::ShaderStage::Vertex:
			path += "Vertex";
			break;
		case Iris::ShaderStage::Fragment:
			path += "Fragment";
			break;
		case Iris::ShaderStage::Compute:
			path += "Compute";
			break;
		case Iris::ShaderStage::Geometry:
			path += "Geometry";
			break;
		case Iris::ShaderStage::TessControl:
			path += "TessControl";
			break;
		case Iris::ShaderStage::TessEval:
			path += "TessEval";
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

void AssetRepo::LoadAssetTable()
{
	std::string tablePath = GetDataPath() + "/Packages/Assets.yaml";
	std::string table = LoadTextFile(tablePath);

	YAML::Node root = YAML::Load(table);

	if (!root["Assets"])
	{
		WLog::SetConsoleWarning();
		WLog::ConsoleLog("Asset Table is missing Assets definition");
		return;
	}

	for (const auto& entry : root["Assets"])
	{
		const YAML::Node& asset = entry.second;

		if (!asset["Dir"])
		{
			WLog::SetConsoleWarning();
			WLog::ConsoleLog("Asset is missing directory path, skipping this asset!");
			continue;
		}
		std::string dir = asset["Dir"].as<std::string>();
		wtl::vector<AssetRef> assets;

		for (const auto& dataEntry : asset["Data"])
		{
			const YAML::Node& data = dataEntry.second;
			if (!data["Name"] || !data["Type"] || !data["UID"])
			{
				WLog::SetConsoleWarning();
				WLog::ConsoleLog(std::format("Data within {} is not properly defined", dir));
				continue;
			}

			AssetRef ref;
			ref.name = data["Name"].as<std::string>();
			// -1 Because Afterlife Browser defines a "None" type which shifts it all over by one.
			ref.type = (AssetType)(data["Type"].as<sizeT>() - 1);
			ref.uid = data["UID"].as<sizeT>();
			assets.push_back(ref);
		}

		m_assets[dir] = std::move(assets);
	}
}

void AssetRepo::LoadAllGPUAssets()
{
	static bool everRan = false;
	if (everRan)
		return;
	everRan = true;

	if (!CheckForPackages())
		return;

	PrepareTransferBuffers();
	LoadAssetTable();

	wtl::vector<std::pair<sizeT, sizeT>> meshTable;
	wtl::vector<std::pair<sizeT, sizeT>> textureTable;

	ParsePackageTable(meshTable, "Meshes.yaml");
	ParsePackageTable(textureTable, "Textures.yaml");

	wtl::vector<byte*> meshFiles;
	wtl::vector<byte*> texFiles;

	ExtractPackage(meshTable, meshFiles, "Meshes.pkg");
	ExtractPackage(textureTable, texFiles, "Textures.pkg");
	ParseAndUploadMeshes(meshFiles);
	ParseTextures(texFiles);
}

wtl::vector<Sector> AssetRepo::LoadAllSectors()
{
	auto sectorDefs = OS::GetAllFileNamesInDir(GetDataPath() + EngineSettings::sectorPath);

	wtl::vector<Sector> sectors;

	for (auto& def : sectorDefs)
	{
		std::string sectorName = std::filesystem::path(def).stem().string();

		if (sectorName == "$Sample")
			continue;
		Sector newSec = Sector(sectorName);
		LoadSingleSector(newSec);
		wtl::vector<SectorEntry> renderable = newSec.m_entries;
		SortSectorForRender(renderable);
		UploadTransformsOfSector(newSec, renderable);
		CreateRenderPlanForSector(newSec, renderable);
		sectors.push_back(newSec);
	}

	return sectors;
}

wtl::vector<WEditor::EditorSector> AssetRepo::LoadAllEditorSectors()
{
	auto sectorDefs = OS::GetAllFileNamesInDir(GetDataPath() + EngineSettings::sectorPath);

	wtl::vector<WEditor::EditorSector> sectors;

	for (auto& def : sectorDefs)
	{
		std::string sectorName = std::filesystem::path(def).stem().string();

		if (sectorName == "$Sample")
			continue;
		WEditor::EditorSector newSec{};
		newSec.name = sectorName;
		LoadSingleEditorSector(newSec);
		sectors.push_back(newSec);
	}

	return sectors;
}

void AssetRepo::TickTextureUpload()
{
	TimeSample sample("AssetRepo::TickTextureUpload");
	static bool finishedUpload = false;
	if (finishedUpload)
		return;

	if (!Iris::IsCopyPassDone(m_copyCmdBuffer))
		return;

#if ASSET_REPO_STREAMING_VISUAL_TEST
	constexpr uint32 StreamingVisualFrameDelay = 70;
	static uint8 streamingVisualCounter = 0;
	if (streamingVisualCounter != StreamingVisualFrameDelay)
	{
		streamingVisualCounter++;
		return;
	}
	else
		streamingVisualCounter = 0;
#endif

	bool allDone = true;

	for (sizeT i = 0; i < m_texturesDone.size(); i++)
	{
		if (m_texturesDone[i] == StreamingProgress::Progress)
			m_texturesDone[i] = StreamingProgress::Loaded;

		if (m_texturesDone[i] == StreamingProgress::Loaded)
			continue;
		allDone = false;
		break;
	}

	if (allDone)
	{
		finishedUpload = true;
		FinalizeTextureCopy();
		WLog::SetConsoleInfo();
		WLog::ConsoleLog("Texture copy has finished!");
		return;
	}

	Iris::BeginCopyPass(m_copyCmdBuffer);
	WLog::ConsoleLog(" ---- Begin Texture Stream Pass -----");

	FillCopyBuffers(m_copyBuffers_XS.data(), m_copyBuffers_XS.size(), 128);
	FillCopyBuffers(m_copyBuffers_S.data(), m_copyBuffers_S.size(), 256);
	FillCopyBuffers(m_copyBuffers_M.data(), m_copyBuffers_M.size(), 512);
	FillCopyBuffers(m_copyBuffers_L.data(), m_copyBuffers_L.size(), 1024);
	FillCopyBuffers(m_copyBuffers_X.data(), m_copyBuffers_X.size(), 2048);

	Iris::EndCopyPass(m_copyCmdBuffer);
}

void AssetRepo::RegisterAllTextures()
{
	static bool everRan = false;
	if (everRan)
		return;
	everRan = true;

	for (const auto& tex : m_textures)
		CoreSystems::GetRenderHandler()->RegisterTexture(tex.second);
}

void AssetRepo::PreloadSounds()
{
	
}

const wtl::vector<AssetRef>& AssetRepo::GetAllAssetsInDir(const std::string &dirName)
{
	static wtl::vector<AssetRef> dummy{};
	if (!m_assets.contains(dirName))
		return dummy;

	return m_assets[dirName];
}

wtl::vector<AssetRef> AssetRepo::GetAllAssetsInDirOfType(const std::string &dirName, AssetType type)
{
	if (!m_assets.contains(dirName))
		return {};

	auto& assetRefs = m_assets[dirName];

	wtl::vector<AssetRef> assets;
	for (auto& ref : assetRefs)
	{
		if (ref.type == type)
			assets.push_back(ref);
	}

	return assets;
}

uint64 AssetRepo::GetFirstAssetInDirOfType(const std::string &dirName, AssetType type)
{
	if (!m_assets.contains(dirName))
		return 0;

	auto& assetRefs = m_assets[dirName];

	for (auto& ref : assetRefs)
	{
		if (ref.type == type)
			return ref.uid;
	}

	return 0;
}

uint64 AssetRepo::GetAssetInDirByName(const std::string &dirName, const std::string &assetName)
{
	if (!m_assets.contains(dirName))
		return 0;

	auto& assetRefs = m_assets[dirName];

	for (auto& ref : assetRefs)
	{
		if (ref.name == assetName)
			return ref.uid;
	}

	return 0;
}

void AssetRepo::PrepareTransferBuffers()
{
	m_texturesDone.resize(m_textures.size());
	for (sizeT i = 0; i < m_texturesDone.size(); i++)
		m_texturesDone[i] = StreamingProgress::Unloaded;

	m_copyCmdBuffer = Iris::CreateCopyBuffer();

	// BC1 DDS Sizes (according to nvcompress)
	constexpr sizeT Tex128  =    11*KB + 1*KB;
	constexpr sizeT Tex256  =    43*KB + 1*KB;
	constexpr sizeT Tex512  =   171*KB + 1*KB;
	constexpr sizeT Tex1024 =   683*KB + 1*KB;
	constexpr sizeT Tex2048 =  2731*KB + 1*KB;
	// I'm unsure if nvcompress uses 1000 or 1024 for their kilobytes, but we have 1024 so it should
	// be fine either way. We will also add 1 kilobyte on top just to be on the safe size.
	Iris::BufferDesc desc{};
	desc.usage = Iris::BufferUsage::TransferSrc;

	desc.debugName = "Texture Transfer Buffer Extra Small";
	for (auto& buff : m_copyBuffers_XS)
	{
		desc.size = Tex128;
		buff = Iris::CreateBuffer(desc);
	}

	desc.debugName = "Texture Transfer Buffer Small";
	for (auto& buff : m_copyBuffers_S)
	{
		desc.size = Tex256;
		buff = Iris::CreateBuffer(desc);
	}

	desc.debugName = "Texture Transfer Buffer Medium";
	for (auto& buff : m_copyBuffers_M)
	{
		desc.size = Tex512;
		buff = Iris::CreateBuffer(desc);
	}

	desc.debugName = "Texture Transfer Buffer Large";
	for (auto& buff : m_copyBuffers_L)
	{
		desc.size = Tex1024;
		buff = Iris::CreateBuffer(desc);
	}

	desc.debugName = "Texture Transfer Buffer Extra Large";
	for (auto& buff : m_copyBuffers_X)
	{
		desc.size = Tex2048;
		buff = Iris::CreateBuffer(desc);
	}

}

bool AssetRepo::CheckForPackages()
{
	std::string packPath = GetDataPath() + "/Packages/";
	std::string texPack = packPath + "Textures.pkg";
	std::string texPackTable = packPath + "Textures.yaml";
	std::string meshPack = packPath + "Meshes.pkg";
	std::string meshPackTable = packPath + "Meshes.yaml";
	std::string assetTable = packPath + "Assets.yaml";

	if (!std::filesystem::exists(texPack))
	{
		WLog::SetConsoleWarning();
		WLog::ConsoleLog("Texture package not found");
		return false;
	}
	if (!std::filesystem::exists(texPackTable))
	{
		WLog::SetConsoleWarning();
		WLog::ConsoleLog("Texture table not found");
		return false;
	}
	if (!std::filesystem::exists(meshPack))
	{
		WLog::SetConsoleWarning();
		WLog::ConsoleLog("Mesh package not found");
		return false;
	}
	if (!std::filesystem::exists(meshPackTable))
	{
		WLog::SetConsoleWarning();
		WLog::ConsoleLog("Mesh table not found");
		return false;
	}
	if (!std::filesystem::exists(meshPackTable))
	{
		WLog::SetConsoleWarning();
		WLog::ConsoleLog("Mesh table not found");
		return false;
	}
	return true;
}

void AssetRepo::ParsePackageTable(wtl::vector<std::pair<sizeT, sizeT>>& container, const std::string& tableName)
{
	std::string tablePath = GetDataPath() + "/Packages/" + tableName;
	std::string table = LoadTextFile(tablePath);

	YAML::Node root = YAML::Load(table);

	if (!root["Table"])
	{
		WLog::SetConsoleWarning();
		WLog::ConsoleLog(std::format("Package table {} is missing Table definition", tableName));
		return;
	}

	for (const auto& entry : root["Table"])
	{
		const YAML::Node& location = entry.second;

		std::pair<sizeT, sizeT> res;
		res.first = location["Offset"].as<sizeT>();
		res.second = location["Size"].as<sizeT>();
		container.push_back(res);
	}
}

void AssetRepo::ExtractPackage(const wtl::vector<std::pair<sizeT, sizeT>>& locations, wtl::vector<byte*>& files,
	const std::string& package)
{
	files.reserve(locations.size());

	std::string packPath = GetDataPath() + "/Packages/" + package;
	std::ifstream pack(packPath, std::ios::binary);
	if (!pack)
	{
		WLog::SetConsoleWarning();
		WLog::ConsoleLog(std::format("Cant load package, unable to open {}!", package));
		return;
	}

	// The package table has been validated a bajillion times on export. if we have an unexpected EOF,
	// then its the fault of the user.
	for (const auto& loc : locations) // first is offset, second is size. both in bytes
	{
		char* file = wNewArr(char, loc.second);
		files.push_back((byte*)file);

		pack.clear();
		pack.seekg(loc.first, std::ios::beg);
		pack.read(file, loc.second);
		std::streamsize got = pack.gcount();
		if (got < loc.second)
		{
			WLog::SetConsoleWarning();
			WLog::ConsoleLog("Something went wrong while unpacking. Verify game files on steam.");
		}
	}
}

AssetRepo::ASMFHeader AssetRepo::ReadASMFHeader(const byte *data)
{
	ASMFHeader h{};
	std::memcpy(h.identifier, data + 0, 4);
	std::memcpy(&h.vertCount, data + 4, 8);
	std::memcpy(&h.indCount,  data + 12, 8);
	return h;
}

void AssetRepo::ParseAndUploadMeshes(const wtl::vector<byte*>& meshFiles)
{
	wtl::vector<ASMFHeader> meshFileHeaders;
	meshFileHeaders.reserve(meshFiles.size());

	const sizeT headerSize = 20;
	const sizeT vertSize = 32;
	const sizeT indexSize = 4;
	sizeT totalVertCount = 0;
	sizeT totalIndCount = 0;
	for (const auto* mesh : meshFiles)
	{
		ASMFHeader header = ReadASMFHeader(mesh);
		meshFileHeaders.push_back(header);
		totalVertCount += header.vertCount;
		totalIndCount += header.indCount;
	}

	// the idea is: we combine all meshes into one payload that we send to the GPU.
	byte* vertexPayload = wNewArr(byte, totalVertCount * vertSize);
	byte* indexPayload = wNewArr(byte, totalIndCount * indexSize);
	sizeT vertHead = 0;
	sizeT indHead = 0;

	MeshInfo dummy{};
	m_meshes.push_back(dummy);

	for (sizeT i = 0; i < meshFiles.size(); i++)
	{
		std::memcpy(vertexPayload + vertHead, meshFiles[i] + headerSize, meshFileHeaders[i].vertCount * vertSize);
		sizeT indexOffset = meshFileHeaders[i].vertCount * vertSize + headerSize;
		std::memcpy(indexPayload + indHead, meshFiles[i] + indexOffset, meshFileHeaders[i].indCount * indexSize);

		MeshInfo location;
		location.vertexOffset = vertHead;
		location.indexOffset = indHead;
		vertHead += meshFileHeaders[i].vertCount * vertSize;
		indHead += meshFileHeaders[i].indCount * indexSize;
		location.vertexSize = vertHead;
		location.indexSize = indHead;

		m_meshes.push_back(location);
	}

	for (auto* mesh : meshFiles)
		wFree(mesh);

	Iris::BufferDesc desc{};
	desc.debugName = "Mesh Vertex Payload";
	desc.size = vertHead;
	desc.usage = Iris::BufferUsage::Vertex;
	m_vertexBuffer = Iris::CreateBuffer(desc, vertexPayload, vertHead);
	desc.debugName = "Mesh Index Payload";
	desc.size = indHead;
	desc.usage = Iris::BufferUsage::Index;
	m_indexBuffer = Iris::CreateBuffer(desc, indexPayload, indHead);

	wFree(vertexPayload);
	wFree(indexPayload);
}

void AssetRepo::ParseTextures(const wtl::vector<byte*>& texFiles)
{
	m_textures.reserve(texFiles.size());
	sizeT uidCounter = 1;
	for (auto* tex : texFiles)
	{
		auto dds = ParseDDS(tex);
		TextureInfoDDS info{};
		info.data     = wNewArr(byte, dds.data.size());
		info.dataSize = dds.data.size();
		info.width    = dds.width;
		info.height   = dds.height;
		info.mipCount = dds.mips;
		info.format   = dds.format;

		memcpy(info.data, dds.data.data(), dds.data.size());

		Iris::TextureDesc desc;
		desc.debugName = std::format("Texture UID:{}", uidCounter);
		desc.format = Iris::ImgFormat::BC1;
		desc.width  = dds.width;
		desc.height = dds.height;
		desc.mipLevels = dds.mips;
		auto handle = Iris::CreateTexture(desc);

		m_textures.push_back({info, handle});
		wFree(tex);
		uidCounter++;
	}

	m_texturesDone.resize(m_textures.size());
	for (sizeT i = 0; i < m_texturesDone.size(); i++)
		m_texturesDone[i] = StreamingProgress::Unloaded;
}

void AssetRepo::FillCopyBuffers(Iris::BufferHandle* handles, sizeT handleCount, sizeT textureWidth)
{
	TimeSample sample("AssetRepo::FillCopyBuffers");
	uint32 handleCursor = 0;
	for (sizeT i = 0; i < m_textures.size(); i++)
	{
		if (m_textures[i].first.width == textureWidth && m_texturesDone[i] == StreamingProgress::Unloaded)
		{
			Iris::UpdateBuffer(handles[handleCursor], 0, m_textures[i].first.data, m_textures[i].first.dataSize);

			Iris::CopyBufferToTexture(m_copyCmdBuffer, handles[handleCursor], 0, m_textures[i].second);

			WLog::SetConsoleInfo();
			WLog::ConsoleLog(std::format("Submitted a {} texture for copy.", textureWidth));

			m_texturesDone[i] = StreamingProgress::Progress;
			handleCursor++;
			if (handleCursor == handleCount)
				return;
		}
	}
}

void AssetRepo::FinalizeTextureCopy()
{
	for (auto& tex : m_textures)
		wFree(tex.first.data);
	m_texturesDone.clear();

	for (auto& buff : m_copyBuffers_XS)
		Iris::DestroyBuffer(buff);

	for (auto& buff : m_copyBuffers_S)
		Iris::DestroyBuffer(buff);

	for (auto& buff : m_copyBuffers_M)
		Iris::DestroyBuffer(buff);

	for (auto& buff : m_copyBuffers_L)
		Iris::DestroyBuffer(buff);

	for (auto& buff : m_copyBuffers_X)
		Iris::DestroyBuffer(buff);

}

void AssetRepo::LoadSingleSector(Sector& storage)
{
	std::string defPath = GetDataPath() + EngineSettings::sectorPath + storage.GetName() + ".yaml";
	std::string sectorDef = LoadTextFile(defPath);

	YAML::Node root = YAML::Load(sectorDef);

	if (!root["sector"])
	{
		WLog::SetConsoleWarning();
		WLog::ConsoleLog(std::format("Unable to load sector \"{}\", missing sector identifier", storage.GetName()));
		return;
	}

	for (const auto& entry : root["sector"])
	{
		const auto& entryDef = entry.second;

		std::string assetPath = entryDef["asset"].as<std::string>();
		uint32 mesh = GetFirstAssetInDirOfType(assetPath, AssetType::StaticMesh);
		uint32 tex = GetFirstAssetInDirOfType(assetPath, AssetType::Texture);
		//uint32 col = GetFirstAssetInDirOfType(assetPath, );

		const YAML::Node& pos = entryDef["position"];
		const YAML::Node& rot = entryDef["rotation"];
		const YAML::Node& size = entryDef["size"];

		Transform t;
		t.position = { pos[0].as<float32>(), pos[1].as<float32>(), pos[2].as<float32>() };
		t.rotation = { rot[0].as<float32>(), rot[1].as<float32>(), rot[2].as<float32>(), rot[3].as<float32>() };
		t.size = { size[0].as<float32>(), size[1].as<float32>(), size[2].as<float32>() };

		SectorEntry newEntry = SectorEntry(mesh, tex, 0, t);
		storage.m_entries.push_back(newEntry);
	}

}

void AssetRepo::LoadSingleEditorSector(WEditor::EditorSector &storage)
{
	std::string defPath = GetDataPath() + EngineSettings::sectorPath + storage.name + ".yaml";
	std::string sectorDef = LoadTextFile(defPath);

	YAML::Node root = YAML::Load(sectorDef);

	if (!root["sector"])
	{
		WLog::SetConsoleWarning();
		WLog::ConsoleLog(std::format("Unable to load sector \"{}\", missing sector identifier", storage.name));
		return;
	}

	for (const auto& entry : root["sector"])
	{
		const auto& entryDef = entry.second;

		const YAML::Node& pos = entryDef["position"];
		const YAML::Node& rot = entryDef["rotation"];
		const YAML::Node& size = entryDef["size"];

		Transform t;
		t.position = { pos[0].as<float32>(), pos[1].as<float32>(), pos[2].as<float32>() };
		t.rotation = { rot[0].as<float32>(), rot[1].as<float32>(), rot[2].as<float32>(), rot[3].as<float32>() };
		t.size = { size[0].as<float32>(), size[1].as<float32>(), size[2].as<float32>() };

		WEditor::EditorSectorEntry newEntry;
		newEntry.asset = entryDef["asset"].as<std::string>();
		newEntry.name = entryDef["name"].as<std::string>();
		newEntry.transform = t;
		storage.entries.push_back(newEntry);
	}
}

void AssetRepo::SortSectorForRender(wtl::vector<SectorEntry>& entries)
{
	std::erase_if(entries, [](const SectorEntry& entry) { return !entry.HasVisuals(); });

	std::sort(entries.begin(), entries.end(),
		[](const SectorEntry& a, const SectorEntry& b)
		{
			if (a.GetTexture() != b.GetTexture())
				return a.GetTexture() < b.GetTexture();
			return a.GetMesh() < b.GetMesh();
		});
}

void AssetRepo::UploadTransformsOfSector(Sector &storage, const wtl::vector<SectorEntry> &renderables)
{
	wtl::vector<Mat4x4> stationaryPayload;

	for (const auto& entry : renderables)
	{
		glm::quat q(entry.GetTransform().rotation.w, entry.GetTransform().rotation.x,
			entry.GetTransform().rotation.y, entry.GetTransform().rotation.z);

		glm::mat4 modelMatrix = glm::mat4_cast(q);

		modelMatrix[0] *= entry.GetTransform().size.x;
		modelMatrix[1] *= entry.GetTransform().size.y;
		modelMatrix[2] *= entry.GetTransform().size.z;

		modelMatrix[3] = glm::vec4(entry.GetTransform().position.x, -entry.GetTransform().position.y,
			entry.GetTransform().position.z, 1.0f);

		stationaryPayload.push_back(Glm4x4ToMat4x4(modelMatrix));
	}

	Iris::BufferDesc desc;
	desc.debugName = storage.GetName() + " stat buff";
	desc.size = sizeof(Mat4x4) * stationaryPayload.size();
	desc.usage = Iris::BufferUsage::Vertex;

	storage.m_renderPlan.statBuffer = Iris::CreateBuffer(desc, (byte*)stationaryPayload.data(),
		sizeof(Mat4x4) * stationaryPayload.size());
}

void AssetRepo::CreateRenderPlanForSector(Sector& storage, const wtl::vector<SectorEntry> &renderables)
{
	if (renderables.empty())
		return;
	RenderPlanPart part{};
	// we need to load it with stuff so it can start going.
	part.meshUID = renderables[0].GetMesh();
	part.textureUID = renderables[0].GetTexture();
	part.count = 0;

	sizeT offsetCounter = 0; // technically not supposed to be sizeT, as RenderPlan doesnt use 64 bits for offset.
	for (const auto& entry : renderables)
	{
		if (entry.GetTexture() == part.textureUID && entry.GetMesh() == part.meshUID)
		{
			part.count++;
			offsetCounter++;
			continue;
		}

		storage.m_renderPlan.parts.push_back(part);

		part.offset = offsetCounter;
		part.count = 1;
		part.meshUID = entry.GetMesh();
		part.textureUID = entry.GetTexture();

		offsetCounter++;
	}
	// the last one seems to go forgotten for some reason. idk why.
	storage.m_renderPlan.parts.push_back(part);
}
