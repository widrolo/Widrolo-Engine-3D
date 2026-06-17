#pragma once
#include <Engine/Types/AssetMission.h>

#include <unordered_map>
#include <string>
#include <Engine/Types/Refcounted.h>

namespace WEngine
{
	struct TextureInfo;
	/**
	 * AssetRepo handles the loading and unloading of game assets such as sprites, shaders, YAML files, atlas info, audio clips, and UI sheets.
	 */
	class AssetRepo
	{
	public:
		/**
		 * Constructs an AssetRepo object and initializes the data path.
		 */
		AssetRepo();
		~AssetRepo() = default;
	private:
		std::string m_dataPath;
		std::unordered_map<std::string, AudioClip> m_audioRepo;
		std::unordered_map<std::string, Ref<TextureInfo>> m_textureRepo;

	public:
		/**
		 * Gets the asset specified by the mission parameter.
		 * @tparam T The type of asset mission to handle (e.g., SpriteAssetMission, ShaderAssetMission, etc.).
		 * @param mission A reference to the asset mission object containing information about the requested asset.
		 */
		template<class T = AssetMissionBase>
		void GetAsset(T& mission);
		/**
		 * Gets the data path where assets are stored.
		 * @return A string containing the data path.
		 */
		std::string GetDataPath() const { return m_dataPath; }

		void IrisAssetComms(IrisAssetCommunication& mission);

	private:
		TextureInfo LoadTexturePNG(const std::string& path);
		AudioClip* LoadAudioWAV(const std::string& name);
		std::string LoadTextFile(const std::string& path);

		void LoadSpirVFromGlsl(SpirVAssetMission& mission);
		void LoadSpirVFromSpv(SpirVAssetMission& mission);

		// ---------------------------------- [IRIS COMMS] ----------------------------------

		void IrisCommsGetMat(IrisAssetCommunication& mission);
		void IrisCommsRetMat(IrisAssetCommunication& mission);

		void IrisCommsGetMatDevel(IrisAssetCommunication& mission);
		void IrisCommsGetMatPackage(IrisAssetCommunication& mission);

	};
};

