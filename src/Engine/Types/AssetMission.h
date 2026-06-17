#pragma once

#include <string>
#include <Engine/Types/CommonTypes.h>
#include <yaml-cpp/yaml.h>
#include <tinyxml2/tinyxml2.h>
#include <Engine/Types/Audio.h>

#include "Rendering/MaterialDefinition.h"
#include "Rendering/ModelInfo.h"
#include "Rendering/TextureInfo.h"

namespace WEngine
{
	/**
	 * Represents a base structure for asset missions in the game engine.
	 */
	struct AssetMissionBase
	{
		std::string name;
	};

	/**
 	 * Represents a mission to load a shader asset.
 	 */
	struct ShaderAssetMission : public AssetMissionBase
	{
		std::string vertexShaderSource;
		std::string fragmentShaderSource;
	};

	/**
	  * Represents a mission to load a shader asset.
	  */
	struct SpirVAssetMission : public AssetMissionBase
	{
		enum SpirVMissionShaderType
		{
			VertexShader,
			FragmentShader,
			GeometryShader,
		} shaderType;
		uint64 shaderSize;
		uint32* shaderCode;
	};

	/**
	 * Represents a mission to load a YAML asset.
	 */
	struct YamlAssetMission : public AssetMissionBase
	{
		YAML::Node root;
	};

	/**
	 * Represents a mission to retrieve or load an audio clip asset.
	 */
	struct AudioClipAssetMission : public AssetMissionBase
	{
		AudioClip* clip;
	};

	/**
	 * Represents a mission to load a UI sheet asset.
	 */
	struct UISheetAssetMission : public AssetMissionBase
	{
		tinyxml2::XMLDocument document;
	};

	struct MeshAssetMission : public AssetMissionBase
	{
		ModelInfo model;
	};

	// ---------------------------------- [IRIS SPECIFIC] ----------------------------------

	enum class IrisAssetCommunicationType : uint8
	{
		GetMaterial,
		RetireMaterial
	};
	struct IrisAssetCommunication
	{
		IrisAssetCommunicationType commType;
		MaterialDefinition matDef;
		wtl::vector<TextureInfo> textureData;
	};
}
