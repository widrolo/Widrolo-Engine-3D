#pragma once

#include <Engine/EngineDefines.h>
#if GPU_BACKEND == GPU_VULKAN

#include "VulkanTypes.h"

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include "Engine/WTL/vector.h"
#include <Engine/Types/Rendering/VertextData.h>

#include "VulkanContext.h"
#include "VulkanStatistics.h"

bool SetupStationaryInstanceBuffer(VulkanContext& ctx, VulkanStatistics& stat);
bool SetupLightingBuffer(VulkanContext& ctx, VulkanStatistics& stat);
std::pair<VkBuffer, VmaAllocation> CreateVertexBuffer(VulkanContext& ctx, VulkanStatistics& stat,
    const wtl::vector<WEngine::VertexData>& vertData);
std::pair<VkBuffer, VmaAllocation> CreateIndexBuffer(VulkanContext& ctx, VulkanStatistics& stat,
    const wtl::vector<uint32>& indData);
std::pair<VkBuffer, VmaAllocation> InitInstanceBuffer(VulkanContext& ctx, VulkanStatistics& stat);
void ExpandInstanceBuffer(VulkanContext& ctx, VulkanStatistics& stat, Vulkan_Model& model, uint64 minSize);

#endif