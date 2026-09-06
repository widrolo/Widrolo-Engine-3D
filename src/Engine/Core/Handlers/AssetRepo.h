#pragma once
#include <Engine/Types/AssetMission.h>
#include <Engine/Types/Asset.h>

#include <unordered_map>
#include <string>
#include <Engine/Types/Refcounted.h>

#include "Editor/Core/World/EditorSector.h"
#include "Engine/Core/World/SectorEntry.h"
#include "Engine/Types/Rendering/Iris/Handles.h"
#include "Engine/Types/Rendering/Iris/IrisAssetComms.h"

#define ASSET_REPO_INEFFICIENT_LOADING_TEST 0
#define ASSET_REPO_STREAMING_VISUAL_TEST 0

namespace WEngine
{
	class Sector;
	struct TextureInfo;
	/**
	 * AssetRepo handles the loading and unloading of game assets such as sprites, shaders, YAML files, atlas info, audio clips, and UI sheets.
	 */
	class AssetRepo
	{
		struct ASMFHeader
		{
			char identifier[4];
			uint64 vertCount;
			uint64 indCount;
		};
		enum class StreamingProgress
		{
			Unloaded,
			Progress,
			Loaded,
		};
	public:
		/**
		 * Constructs an AssetRepo object and initializes the data path.
		 */
		AssetRepo();
		~AssetRepo() = default;
	public:
		/**
		 * As per engine requirements, all graphical assets are loaded up front.
		 * @note This can only be called once.
		 */
		void LoadAllGPUAssets();
		wtl::vector<Sector> LoadAllSectors();
		wtl::vector<WEditor::EditorSector> LoadAllEditorSectors();
		/**
		 * Since textures are a bit special when it comes to uploading to VRAM, we need to do it over time.
		 * @note This should be called at the beginning of every frame.
		 * @note One done, it turns itself off.
		 */
		void TickTextureUpload();
		void RegisterAllTextures();
		void PreloadSounds();
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

		/**
		 * Retrieves all assets within a directory and returns them.
		 * @param dirName Tree path as visible in Afterlife Browser.
		 * @return All assets in directory. Returns empty vector if directory is empty or doesnt exist.
		 */
		const wtl::vector<AssetRef>& GetAllAssetsInDir(const std::string& dirName);

		/**
		 * Retrieves all assets within a directory and returns them.
		 * @param dirName Tree path as visible in Afterlife Browser.
		 * @param type Type of the Assets.
		 * @return All assets of type in directory. Returns empty vector if directory is empty or doesnt exist.
		 */
		wtl::vector<AssetRef> GetAllAssetsInDirOfType(const std::string& dirName, AssetType type);

		/**
		 * Retrieves an asset of a given type within a directory and returns them.
		 * @param dirName Tree path as visible in Afterlife Browser.
		 * @param type Type of the Asset.
		 * @return UID of the first assets of type in directory. 0 if the asset cannot be found.
		 */
		uint64 GetFirstAssetInDirOfType(const std::string& dirName, AssetType type);

		/**
		 * Retrieves an asset of a given name within a directory and returns them.
		 * @param dirName Tree path as visible in Afterlife Browser.
		 * @param assetName Name of the Asset.
		 * @return UID of the first assets with this name in directory. 0 if the asset cannot be found.
		 * @note In contrast to the project file in Afterlife Browser, the name here does not refer to the name
		 * in the project file, but rather the sub name.
		 */
		uint64 GetAssetInDirByName(const std::string& dirName, const std::string& assetName);

		Iris::BufferHandle GetVertexBuffer() const { return m_vertexBuffer; }
		Iris::BufferHandle GetIndexBuffer() const { return m_indexBuffer; }

		bool IsTextureDoneLoading(uint64 uid) const;

	private:
		AudioClip* LoadAudioWAV(const std::string& name);
		std::string LoadTextFile(const std::string& path);

		void LoadSpirVFromGlsl(SpirVAssetMission& mission);
		void LoadSpirVFromSpv(SpirVAssetMission& mission);

		// ----- GPU Preloading -----
		void LoadAssetTable();
		void PrepareTransferBuffers();
		bool CheckForPackages();
		void ParsePackageTable(wtl::vector<std::pair<sizeT, sizeT>>& container, const std::string& tableName);
		void ExtractPackage(const wtl::vector<std::pair<sizeT, sizeT>>& locations, wtl::vector<byte*>& files,
			const std::string& package);
		ASMFHeader ReadASMFHeader(const byte* data);
		void ParseAndUploadMeshes(const wtl::vector<byte*>& meshFiles);
		void ParseTextures(const wtl::vector<byte*>& texFiles);
		void FillCopyBuffers(Iris::BufferHandle* handles, sizeT handleCount, sizeT textureWidth);
		void FinalizeTextureCopy();

		void LoadSingleSector(Sector& storage);
		void LoadSingleEditorSector(WEditor::EditorSector& storage);
		void SortSectorForRender(wtl::vector<SectorEntry>& entries);
		void UploadTransformsOfSector(Sector& storage, const wtl::vector<SectorEntry> &renderables);
		void CreateRenderPlanForSector(Sector& storage, const wtl::vector<SectorEntry> &renderables);

	private:
		std::string m_dataPath;
		std::unordered_map<std::string, AudioClip> m_audioRepo;
		std::unordered_map<std::string, wtl::vector<AssetRef>> m_assets;

		Iris::BufferHandle m_vertexBuffer;
		Iris::BufferHandle m_indexBuffer;

		Iris::CopyBufferHandle m_copyCmdBuffer;
		wtl::vector<std::pair<TextureInfoDDS, Iris::TextureHandle>> m_textures;
		wtl::vector<MeshInfo> m_meshes;

		// keep this as the bottom so it doesnt pollute the offsets of the rest
		// These should be fine-tuned in the final optimization pass of the game long after the content lock.
		wtl::vector<StreamingProgress> m_texturesDone;

#if ASSET_REPO_INEFFICIENT_LOADING_TEST
		_GLOBAL_CEX_ sizeT TexturesPerUpload_XS = 1;
		_GLOBAL_CEX_ sizeT TexturesPerUpload_S = 1;
		_GLOBAL_CEX_ sizeT TexturesPerUpload_M = 1;
		_GLOBAL_CEX_ sizeT TexturesPerUpload_L = 1;
		_GLOBAL_CEX_ sizeT TexturesPerUpload_X = 1;
#else
		_GLOBAL_CEX_ sizeT TexturesPerUpload_XS = 96;
		_GLOBAL_CEX_ sizeT TexturesPerUpload_S = 128;
		_GLOBAL_CEX_ sizeT TexturesPerUpload_M = 64;
		_GLOBAL_CEX_ sizeT TexturesPerUpload_L = 16;
		_GLOBAL_CEX_ sizeT TexturesPerUpload_X = 4;
#endif
		std::array<Iris::BufferHandle, TexturesPerUpload_XS>	m_copyBuffers_XS;	// for 128 or lower
		std::array<Iris::BufferHandle, TexturesPerUpload_S> 	m_copyBuffers_S;	// for 256 or lower
		std::array<Iris::BufferHandle, TexturesPerUpload_M> 	m_copyBuffers_M;	// for 512 or lower
		std::array<Iris::BufferHandle, TexturesPerUpload_L> 	m_copyBuffers_L;	// for 1024 or lower
		std::array<Iris::BufferHandle, TexturesPerUpload_X> 	m_copyBuffers_X;	// for 2048 or lower
	};
};

