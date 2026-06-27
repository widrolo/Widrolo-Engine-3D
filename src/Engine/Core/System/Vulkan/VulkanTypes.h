#pragma once

#include <Engine/EngineDefines.h>
#if GPU_BACKEND == GPU_VULKAN

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include <Engine/Types/CommonTypes.h>
#include <Engine/Util/BitwiseMacros.h>
#include <Engine/WTL/vector.h>
#include <Engine/Types/Rendering/Iris/InstThreadedList.h>

#include "Engine/Types/Rendering/LightingInfo.h"
#include "Engine/Types/Rendering/ShaderDefinition.h"
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
    uint32 swapchainImageCount = 0;
    uint32 swapchainCurrentImage = 0;

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

    // if no dedicated transfer queue was found, then this will be the same as the graphics queue
    uint32 primaryTransferQueueFamilyIndex = 0;
    VkQueue primaryTransferQueue = VK_NULL_HANDLE;
};

struct Vulkan_Shader;
struct Vulkan_Material
{
    // only albedo for now.
    VkImageView albedo;
    VkSampler albedoSampler;

    VkDescriptorSet materialDescriptorSet;
    WEngine::Shader materialShaderHandle;
    Vulkan_Shader* shader = nullptr;

    bool hasTextures = false;
};

struct Vulkan_Shader
{
    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;
    wtl::vector<Vulkan_Material> shaderMaterials;
    VkDescriptorPool descriptorPool;
    VkDescriptorSetLayout descriptorSetLayout;
    WEngine::ShaderDefinition shaderDefinition;
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

struct Vulkan_Texture
{
    VkImage image;
    VkImageView imageView;
    VkSampler sampler;
    VmaAllocation imageAllocation;
};

struct Vulkan_RenderTarget
{
    WEngine::Vector2 resolution;
    wtl::vector<VkImage> targetImages;
    wtl::vector<VkImageView> targetImageViews;
    wtl::vector<VmaAllocation> targetImageAlloc;
    wtl::vector<VkSemaphore> imageAvailableSems;
    wtl::vector<VkSemaphore> renderFinishedSems;
    wtl::vector<VkFence> endOfFrameFences;
    wtl::vector<VkCommandBuffer> cmdBuffs{};
};

struct RawLighting
{
    alignas(16) WEngine::Vector3 sunDir;
    alignas(16) WEngine::Vector3 sunColor;
    alignas(16) WEngine::Vector3 camPos;
};

struct Vulkan_Lighting
{
    WEngine::LightingInfo lightingInfo;
    VkBuffer lightBuffer;
    VmaAllocation lightAllocation;
    VmaAllocationInfo lightAllocInfo;
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool descriptorPool;
    VkDescriptorSet descriptorSet;
};

using BufferCollection = wtl::vector<std::pair<VkBuffer, VmaAllocation>>;

#endif