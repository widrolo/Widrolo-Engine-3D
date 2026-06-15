#pragma once
#include <Engine/EngineDefines.h>
#if GPU_BACKEND == GPU_VULKAN

#include "VulkanTypes.h"
#include "Engine/Types/Rendering/GPU/Material.h"

struct VulkanContext
{
    Vulkan_Core vcore{};
    Vulkan_Screen screen{};
    Vulkan_Queues queues{};
    Vulkan_StatBuf statBuf{};

    VkDescriptorPool imGuiDescriptorPool{};
    VkCommandPool commandPool{};

    wtl::vector<VkCommandBuffer> cmdBufs{};
    VkRenderPass renderPass{};

    wtl::vector<Vulkan_Shader> loadedShaders{};
    std::unordered_map<std::string, WEngine::Shader> loadedShadersHandles{};
    wtl::vector<Vulkan_Model> loadedModels{};
    std::unordered_map<std::string, WEngine::Model> loadedModelHandles{};
    wtl::vector<Vulkan_Material> loadedMaterials{};
    std::unordered_map<std::string, WEngine::Material> loadedMaterialHandles{};

    WEngine::Shader currentBoundShader = 999999999;
    std::vector<BufferCollection> bufferGraveyard{};
};

#endif
