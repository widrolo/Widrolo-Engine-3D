#include "AssetRepo.h"

#include <Engine/Util/Log.h>
#include <Engine/EngineDefines.h>
#include <Engine/Core/System/OS.h>
#include <fstream>
#include <sstream>

#include <stb_image.h>
#define TINYGLTF_IMPLEMENTATION
#include <Engine/gl/gltf/tiny_gltf_v3.h>
#include <yaml-cpp/yaml.h>
#include <tinyxml2/tinyxml2.h>

#include "Engine/imgui/imgui.h"
#include <shaderc/shaderc.hpp>

#include "RNGHandler.h"
#include "Engine/Types/CoreSystems.h"
#include "Engine/Types/Rendering/TextureInfo.h"
#include "Engine/Types/Rendering/VertextData.h"
#include "Engine/Util/TextureSwizzler.h"


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
void AssetRepo::GetAsset<MeshAssetMission>(MeshAssetMission& mission)
{
	const std::string file = GetDataPath() + EngineSettings::meshPath + mission.name + ".glb";

	tg3_parse_options opts;
	tg3_error_stack errors;
	tg3_model model;

	tg3_parse_options_init(&opts);
	tg3_error_stack_init(&errors);

	tg3_error_code err = tg3_parse_file(&model, &errors, file.c_str(), 10, &opts);
	if (err != TG3_OK) {
		for (uint32_t i = 0; i < errors.count; i++) {
			WLog::SetConsoleError();
			WLog::ConsoleLog("Could not load glb model:");
			fprintf(stderr, "[%d] %s\n", (int)errors.entries[i].severity,
					errors.entries[i].message ? errors.entries[i].message : "(null)");
		}
		return;
	}

	mission.model.name = mission.name;

	// TODO: fix up this vibe coded mess so it fits within the project nicely.

	auto GetAccessorData = [&](int accessorIdx, size_t componentSize) -> std::pair<const uint8_t*, size_t>
	{
		auto& acc     = model.accessors[accessorIdx];
		auto& bufView = model.buffer_views[acc.buffer_view];
		auto& buf     = model.buffers[bufView.buffer];

		const uint8_t* data = buf.data.data + bufView.byte_offset + acc.byte_offset;
		size_t stride = (bufView.byte_stride != 0) ? bufView.byte_stride : componentSize;
		return { data, stride };
	};

	auto FindAttribute = [](const tg3_primitive& prim, const char* name) -> int
	{
		for (size_t i = 0; i < prim.attributes_count; i++)
			if (strcmp(prim.attributes[i].key.data, name) == 0)
				return prim.attributes[i].value;
		return -1;
	};

	ModelInfo& out = mission.model;

    for (size_t mi = 0; mi < model.meshes_count; mi++)
    {
        auto& mesh = model.meshes[mi];
        for (size_t pi = 0; pi < mesh.primitives_count; pi++)
        {
            auto& prim = mesh.primitives[pi];

            // ── Indices ──────────────────────────────────────────────────────
            {
                auto& acc            = model.accessors[prim.indices];
                auto [data, stride]  = GetAccessorData(prim.indices, 0 /*unused*/);
                size_t indexStart    = out.indices.size();
                out.indices.resize(indexStart + acc.count);

                for (size_t i = 0; i < acc.count; i++)
                {
                    uint32 idx = 0;
                    if      (acc.component_type == TG3_COMPONENT_TYPE_UNSIGNED_BYTE)
                        idx = *reinterpret_cast<const uint8_t*> (data + i * sizeof(uint8_t));
                    else if (acc.component_type == TG3_COMPONENT_TYPE_UNSIGNED_SHORT)
                        idx = *reinterpret_cast<const uint16_t*>(data + i * sizeof(uint16_t));
                    else if (acc.component_type == TG3_COMPONENT_TYPE_UNSIGNED_INT)
                        idx = *reinterpret_cast<const uint32_t*>(data + i * sizeof(uint32_t));

                    out.indices[indexStart + i] = idx;
                }
            }

            // ── Vertex count (driven by POSITION) ────────────────────────────
            int posIdx   = FindAttribute(prim, "POSITION");
            int colIdx   = FindAttribute(prim, "COLOR_0");
            int uv0Idx   = FindAttribute(prim, "TEXCOORD_0");
            int uv1Idx   = FindAttribute(prim, "TEXCOORD_1");

            if (posIdx == -1) continue; // no positions = skip

            size_t vertCount  = model.accessors[posIdx].count;
            size_t vertStart  = out.vertices.size();
            out.vertices.resize(vertStart + vertCount);

            // ── POSITION (vec3 float) ─────────────────────────────────────────
            {
                auto [data, stride] = GetAccessorData(posIdx, sizeof(float) * 3);
                for (size_t i = 0; i < vertCount; i++)
                {
                    const float* v = reinterpret_cast<const float*>(data + i * stride);
                    out.vertices[vertStart + i].position = { v[0], v[1], v[2] };
                }
            }

            // ── COLOR_0 (vec3 or vec4 float, default white if missing) ────────
            if (colIdx != -1)
            {
                auto& acc           = model.accessors[colIdx];
                bool isVec4         = (acc.type == TG3_TYPE_VEC4);
                size_t compSize     = isVec4 ? sizeof(float) * 4 : sizeof(float) * 3;
                auto [data, stride] = GetAccessorData(colIdx, compSize);

                for (size_t i = 0; i < vertCount; i++)
                {
                    const float* v = reinterpret_cast<const float*>(data + i * stride);
                    out.vertices[vertStart + i].vertColor = { v[0], v[1], v[2] };
                }
            }
            else
            {


                for (size_t i = 0; i < vertCount; i++)
                {
                	// just for testing around now.
                	float32 r = CoreSystems::GetRNGHandler()->GetRandomFloat(0.0f, 1.0f);
                	float32 g = CoreSystems::GetRNGHandler()->GetRandomFloat(0.0f, 1.0f);
                	float32 b = CoreSystems::GetRNGHandler()->GetRandomFloat(0.0f, 1.0f);
                    out.vertices[vertStart + i].vertColor = { r, g, b };
                }
            }

            // ── TEXCOORD_0 (vec2 float) ───────────────────────────────────────
            if (uv0Idx != -1)
            {
                auto [data, stride] = GetAccessorData(uv0Idx, sizeof(float) * 2);
                for (size_t i = 0; i < vertCount; i++)
                {
                    const float* v = reinterpret_cast<const float*>(data + i * stride);
                    out.vertices[vertStart + i].uv0Coord = { v[0], v[1] };
                }
            }

            // ── TEXCOORD_1 (vec2 float) ───────────────────────────────────────
            if (uv1Idx != -1)
            {
                auto [data, stride] = GetAccessorData(uv1Idx, sizeof(float) * 2);
                for (size_t i = 0; i < vertCount; i++)
                {
                    const float* v = reinterpret_cast<const float*>(data + i * stride);
                    out.vertices[vertStart + i].uv1Coord = { v[0], v[1] };
                }
            }
        }
    }

	tg3_model_free(&model);
	tg3_error_stack_free(&errors);

	WLog::ConsoleLog(std::format("Loaded model \"{}\"", mission.name));
}

template<>
void AssetRepo::GetAsset<SpirVAssetMission>(SpirVAssetMission& mission)
{
#ifdef PACKAGE
	//LoadSpirVFromSpv(mission);
	LoadSpirVFromGlsl(mission);
#else
	LoadSpirVFromGlsl(mission);
#endif

}

void AssetRepo::IrisAssetComms(IrisAssetCommunication &mission)
{
	switch (mission.commType)
	{
		case IrisAssetCommunicationType::GetMaterial:
			IrisCommsGetMat(mission);
			break;
		case IrisAssetCommunicationType::RetireMaterial:
			IrisCommsRetMat(mission);
			break;
	}
}

TextureInfo AssetRepo::LoadTexturePNG(const std::string& path)
{
	TextureInfo info{};
	info.data = stbi_load(path.c_str(), &info.width, &info.height, &info.channels, 4);
	if (info.data == nullptr)
	{
		WLog::SetConsoleError();
		WLog::ConsoleLog(std::format("Failed to load texture {}", path));
		return TextureInfo{};
	}
	uint64 size = info.width * info.height * 4;
	WAllocator::ReportExternalAllocation(size);
	return info;
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
		case SpirVAssetMission::VertexShader:
			path += "Vertex";
			kind = shaderc_glsl_vertex_shader;
			break;
		case SpirVAssetMission::FragmentShader:
			path += "Fragment";
			kind = shaderc_glsl_fragment_shader;
			break;
		case SpirVAssetMission::GeometryShader:
			path += "Geometry";
			kind = shaderc_glsl_geometry_shader;
			break;
	}

	path += ".glsl";

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

	size_t wordCount = res.cend() - res.cbegin();
	mission.shaderCode = wNewArr(uint32, wordCount);
	std::copy(res.cbegin(), res.cend(), mission.shaderCode);
	mission.shaderSize = wordCount * sizeof(uint32);
}

void AssetRepo::LoadSpirVFromSpv(SpirVAssetMission &mission)
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

void AssetRepo::IrisCommsGetMat(IrisAssetCommunication &mission)
{
	// This is only here because I don't trust myself.
	mission.textureData.resize(mission.matDef.texturesPackaging.size());

#ifdef PACKAGE
	IrisCommsGetMatPackage(mission);
#else
	IrisCommsGetMatDevel(mission);
#endif
}

void AssetRepo::IrisCommsRetMat(IrisAssetCommunication &mission)
{
	for (const auto& request : mission.matDef.texturesPackaging)
	{
		// We can kinda guarantee that it exists. Proof is because i said so; q.e.d. :)
		bool isUnused = m_textureRepo.at(request).Remove();

		if (isUnused)
		{
			TextureInfo info = m_textureRepo.at(request).Get();

			free(info.data);

			uint64 size = info.width * info.height * 4;
			WAllocator::ReportExternalFree(size);

			m_textureRepo.erase(request);
		}
	}
}

void AssetRepo::IrisCommsGetMatDevel(IrisAssetCommunication &mission)
{
	int32 i = 0;

	wtl::vector<uint8> missing;

	for (const auto& request : mission.matDef.texturesPackaging)
	{
		if (m_textureRepo.contains(request))
		{
			mission.textureData[i] = m_textureRepo.at(request).Get();
			m_textureRepo.at(request).Add();
		}
		else
		{
			missing.push_back(i);
		}
		i++;
	}

	i = 0;
	for (const auto& index : missing)
	{
		TextureSwizzler swizzler;
		for (const auto& swizzle : mission.matDef.swizzles)
		{
			if (swizzle.packedTexTarget == index)
			{
				std::string develTex = mission.matDef.texturesDevel[swizzle.swizzle[i].develTexOrigin];

				TextureInfo info = LoadTexturePNG(m_dataPath + EngineSettings::texturePath + develTex);
				swizzler.AddSource(&info, swizzle.swizzle[i].channel, i);
				i++;
			}
		}
		swizzler.Swizzle();
		TextureInfo info = swizzler.RetrieveResult();
		m_textureRepo.try_emplace(mission.matDef.texturesPackaging[index], info);
		mission.textureData[index] = info;
		m_textureRepo.at(mission.matDef.texturesPackaging[index]).Add();
	}
}

void AssetRepo::IrisCommsGetMatPackage(IrisAssetCommunication &mission)
{
	int32 i = 0;
	for (const auto& request : mission.matDef.texturesPackaging)
	{
		if (!m_textureRepo.contains(request))
		{
			TextureInfo info = LoadTexturePNG(m_dataPath + EngineSettings::texturePath + request);
			m_textureRepo.try_emplace(request, info);
		}

		mission.textureData[i] = m_textureRepo.at(request).Get();
		m_textureRepo.at(request).Add();
		i++;
	}
}
