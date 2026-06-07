#pragma once

#include <Engine/EngineDefines.h>
#if GPU_BACKEND == GPU_VULKAN

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include <Engine/Types/CommonTypes.h>
#include <Engine/Util/BitwiseMacros.h>
#include <Engine/WTL/vector.h>
#include <Engine/Types/Rendering/Iris/InstThreadedList.h>

#include "Engine/Types/Rendering/GPU/Material.h"


enum class QueuePurpose : uint8
{
    Drawing     = BIT(0),
    Compute     = BIT(1),
    Transfer    = BIT(2),
    Sparse      = BIT(3),
};

struct QueueFamily
{
    uint8 familyIndex;
    uint8 purpose;
    wtl::vector<VkQueue> queues;
};

struct Vulkan_Screen
{
    VkSurfaceKHR screen = VK_NULL_HANDLE;
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    wtl::vector<VkImage> swapchainImages;
    wtl::vector<VkImageView> swapchainImageViews;
    wtl::vector<VkFramebuffer> swapchainFramebuffers;
    uint32 swapchainCurrentImage = 0;

    wtl::vector<VkSemaphore> imageAvailableSems;
    wtl::vector<VkSemaphore> renderFinishedSems;
    wtl::vector<VkFence> endOfFrameFences;
    uint32 currentFrame = 0;

    VkImage depthImage;
    VkImageView depthImageView;
    VmaAllocation depthAllocation;
};

struct Vulkan_Core
{
    VkDebugUtilsMessengerEXT validationMessenger = VK_NULL_HANDLE;
    VkAllocationCallbacks* allocator = nullptr;
    VkAllocationCallbacks allocatorInternal{};
    VmaAllocator vmaAllocator = VK_NULL_HANDLE;

    VkInstance instance = VK_NULL_HANDLE;
    VkPhysicalDevice gpuPhysicalDevice = VK_NULL_HANDLE;
    VkDevice gpuDevice = VK_NULL_HANDLE;
};

struct Vulkan_Queues
{
    uint32 queueFamilyCount = 0;
    wtl::vector<QueueFamily> queueFamilies;
    uint32 primaryDrawQueueFamilyIndex = 0;
    VkQueue primaryDrawQueue = VK_NULL_HANDLE;
};

struct Vulkan_Material
{
    // only albedo for now.
    VkImageView albedo;
    VkSampler albedoSampler;

    VkDescriptorSet materialDescriptorSet;
    WEngine::Shader materialShader;
};

struct Vulkan_Shader
{
    VkPipeline pipeline;
    wtl::vector<Vulkan_Material> shaderMaterials;
};

struct Vulkan_Model
{
    VkBuffer vertexBuffer;
    VmaAllocation vertexAllocation;
    VkBuffer indexBuffer;
    VmaAllocation indexAllocation;
    uint32 vertexCount;
    uint32 indexCount;

    VkBuffer instanceBuffer;
    VmaAllocation instanceAllocation;
    uint16 activeInstances;
    uint32 instanceBufferSize;
};


struct Vulkan_StatBuf
{
    InstThreadedList statBookkeep;
    VkBuffer statBuffer;
    VmaAllocation statAllocation;
};

using BufferCollection = wtl::vector<std::pair<VkBuffer, VmaAllocation>>;

#endif