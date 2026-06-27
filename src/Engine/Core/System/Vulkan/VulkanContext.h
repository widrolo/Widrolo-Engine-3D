#pragma once
#include <Engine/EngineDefines.h>
#if GPU_BACKEND == GPU_VULKAN

#include "VulkanTypes.h"
#include "Engine/Types/Rendering/LightingInfo.h"
#include "Engine/Types/Rendering/GPU/Material.h"

struct VulkanContext
{
    Vulkan_Core vcore{};
    Vulkan_Screen screen{};
    Vulkan_Queues queues{};
    Vulkan_StatBuf statBuf{};

    VkDescriptorPool imGuiDescriptorPool{};
    VkCommandPool commandPool{};

    VkCommandBuffer transferCommandBuffer{};
    VkSemaphore transferSemaphore{};
    VkFence transferFence{};
    wtl::vector<std::pair<VkBuffer, VmaAllocation>> stagingBuffers{};

    wtl::vector<Vulkan_Shader> loadedShaders{};
    std::unordered_map<std::string, WEngine::Shader> loadedShadersHandles{};
    wtl::vector<Vulkan_Model> loadedModels{};
    std::unordered_map<std::string, WEngine::Model> loadedModelHandles{};
    wtl::vector<Vulkan_Material> loadedMaterials{};
    std::unordered_map<std::string, WEngine::Material> loadedMaterialHandles{};
    wtl::vector<Vulkan_Texture> loadedTextures{};

    Vulkan_RenderTarget displayTarget{};
    wtl::vector<Vulkan_RenderTarget> renderTargets{};
    Vulkan_RenderTarget* currentRenderTarget{};

    WEngine::Shader currentBoundShader = 999999999;
    std::vector<BufferCollection> bufferGraveyard{};
    bool firstFrame = true;
    bool isCommandRecording = false;

    Vulkan_Lighting lighting{};
};

#endif
