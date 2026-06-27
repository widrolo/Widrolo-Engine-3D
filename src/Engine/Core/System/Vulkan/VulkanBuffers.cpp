#include <Engine/EngineDefines.h>
#if GPU_BACKEND == GPU_VULKAN
#include "VulkanBuffers.h"

#include <cstring>

#include "VulkanHelpers.h"
#include "Engine/Core/System/GPUSettings.h"
#include "Engine/Types/Rendering/InstanceData.h"
#include "Engine/Util/Log.h"
#include <Engine/Types/Rendering/LightingInfo.h>

bool SetupStationaryInstanceBuffer(VulkanContext& ctx, VulkanStatistics& stat)
{
    VkBufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = GPUSettings::stationaryInstBufferSize;
    bufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

    VmaAllocationCreateInfo allocationCreateInfo{};
    allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
    allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocationCreateInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    VmaAllocationInfo bufferAllocInfo{};

    auto res = vmaCreateBuffer(ctx.vcore.vmaAllocator, &bufferCreateInfo, &allocationCreateInfo,
        &ctx.statBuf.statBuffer, &ctx.statBuf.statAllocation, &bufferAllocInfo);

    stat.vramUsage += bufferAllocInfo.size;

    if (!ParseVkResult(res))
    {
        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog("Failed to expand instance buffer");
        return false;
    }

    return true;
}

bool SetupLightingBuffer(VulkanContext &ctx, VulkanStatistics &stat)
{
    VkDeviceSize lightingBufferSize = sizeof(RawLighting);

    VkBufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = lightingBufferSize;
    bufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

    VmaAllocationCreateInfo allocationCreateInfo{};
    allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
    allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocationCreateInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;


    auto res = vmaCreateBuffer(ctx.vcore.vmaAllocator, &bufferCreateInfo, &allocationCreateInfo, &ctx.lighting.lightBuffer,
        &ctx.lighting.lightAllocation, &ctx.lighting.lightAllocInfo);

    stat.vramUsage += ctx.lighting.lightAllocInfo.size;

    if (!ParseVkResult(res))
    {
        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog("Failed to allocate lighting buffer");
        return false;
    }
    return true;
}

std::pair<VkBuffer, VmaAllocation> CreateVertexBuffer(VulkanContext& ctx, VulkanStatistics& stat,
                                                      const wtl::vector<WEngine::VertexData>& vertData)
{
    VkBuffer vertBuf;
    VmaAllocation vertAlloc;
    VkDeviceSize vertBufferSize = vertData.size() * sizeof(WEngine::VertexData);

    VkBufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = vertBufferSize;
    bufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

    VmaAllocationCreateInfo allocationCreateInfo{};
    allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
    allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocationCreateInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    VmaAllocationInfo bufferAllocInfo{};

    auto res = vmaCreateBuffer(ctx.vcore.vmaAllocator, &bufferCreateInfo, &allocationCreateInfo,
        &vertBuf, &vertAlloc, &bufferAllocInfo);

    stat.vramUsage += bufferAllocInfo.size;

    if (!ParseVkResult(res))
    {
        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog("Failed to allocate vertex buffer");
        return {VK_NULL_HANDLE, VK_NULL_HANDLE};
    }

    if (bufferAllocInfo.pMappedData)
    {
        memcpy(bufferAllocInfo.pMappedData, vertData.data(), vertBufferSize);
    }
    else
    {
        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog("Failed to allocate vertex buffer");
        return {VK_NULL_HANDLE, VK_NULL_HANDLE};
    }
    return {vertBuf, vertAlloc};
}

std::pair<VkBuffer, VmaAllocation> CreateIndexBuffer(VulkanContext& ctx, VulkanStatistics& stat,
    const wtl::vector<uint32>& indData)
{
    VkBuffer indBuf;
    VmaAllocation indAlloc;
    VkDeviceSize indBufferSize = indData.size() * sizeof(uint32);

    VkBufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = indBufferSize;
    bufferCreateInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

    VmaAllocationCreateInfo allocationCreateInfo{};
    allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
    allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocationCreateInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    VmaAllocationInfo bufferAllocInfo{};

    auto res = vmaCreateBuffer(ctx.vcore.vmaAllocator, &bufferCreateInfo, &allocationCreateInfo,
        &indBuf, &indAlloc, &bufferAllocInfo);

    stat.vramUsage += bufferAllocInfo.size;

    if (!ParseVkResult(res))
    {
        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog("Failed to allocate index buffer");
        return {VK_NULL_HANDLE, VK_NULL_HANDLE};
    }

    if (bufferAllocInfo.pMappedData)
    {
        memcpy(bufferAllocInfo.pMappedData, indData.data(), indBufferSize);
    }
    else
    {
        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog("Failed to allocate index buffer");
        return {VK_NULL_HANDLE, VK_NULL_HANDLE};
    }
    return {indBuf, indAlloc};
}

std::pair<VkBuffer, VmaAllocation> InitInstanceBuffer(VulkanContext& ctx, VulkanStatistics& stat)
{
    VkBuffer instBuf;
    VmaAllocation instAlloc;
    VkDeviceSize isntBufferSize = GPUSettingsVulkan::maxInstanceBufferSize * sizeof(WEngine::InstanceData);

    VkBufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = isntBufferSize;
    bufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

    VmaAllocationCreateInfo allocationCreateInfo{};
    allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
    allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocationCreateInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    VmaAllocationInfo bufferAllocInfo{};

    auto res = vmaCreateBuffer(ctx.vcore.vmaAllocator, &bufferCreateInfo, &allocationCreateInfo,
        &instBuf, &instAlloc, &bufferAllocInfo);

    stat.vramUsage += bufferAllocInfo.size;

    if (!ParseVkResult(res))
    {
        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog("Failed to allocate instance buffer");
        return {VK_NULL_HANDLE, VK_NULL_HANDLE};
    }

    return {instBuf, instAlloc};
}

void ExpandInstanceBuffer(VulkanContext& ctx, VulkanStatistics& stat, Vulkan_Model& model, uint64 minSize)
{
    WEngine::WLog::SetConsoleInfo();
    WEngine::WLog::ConsoleLog(std::format("Invoked an instance buffer expansion: {}", minSize));

    VmaAllocationInfo bufferAllocInfo{};
    vmaGetAllocationInfo(ctx.vcore.vmaAllocator, model.instanceAllocation, &bufferAllocInfo);
    uint64 oldSize = bufferAllocInfo.size / sizeof(WEngine::InstanceData);

    stat.vramUsage -= bufferAllocInfo.size;

    VkBuffer instBuf;
    VmaAllocation instAlloc;
    VkDeviceSize newSize = minSize * 2 * sizeof(WEngine::InstanceData);

    VkBufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = newSize;
    bufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

    VmaAllocationCreateInfo allocationCreateInfo{};
    allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
    allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocationCreateInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    auto res = vmaCreateBuffer(ctx.vcore.vmaAllocator, &bufferCreateInfo, &allocationCreateInfo,
        &instBuf, &instAlloc, &bufferAllocInfo);

    stat.vramUsage += bufferAllocInfo.size;

    if (!ParseVkResult(res))
    {
        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog("Failed to expand instance buffer");
        return;
    }

    WEngine::InstanceData* instOld;
    vmaMapMemory(ctx.vcore.vmaAllocator, model.instanceAllocation, (void**)&instOld);
    memcpy(bufferAllocInfo.pMappedData, instOld, oldSize * sizeof(WEngine::InstanceData));
    vmaUnmapMemory(ctx.vcore.vmaAllocator, model.instanceAllocation);
    ctx.bufferGraveyard[ctx.screen.currentFrame].push_back({model.instanceBuffer, model.instanceAllocation});

    model.instanceBuffer = instBuf;
    model.instanceAllocation = instAlloc;
    model.instanceBufferSize = minSize * 2;
}

#endif
