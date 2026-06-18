#include <Engine/EngineDefines.h>
#if GPU_BACKEND == GPU_VULKAN

#include "VulkanImages.h"

#include <cstring>
#include <vulkan/vulkan.h>

#include "VulkanHelpers.h"
#include "Engine/Util/Log.h"

bool SetupDepthImage(VulkanContext& ctx, VulkanStatistics& stat)
{
    VkImageCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    info.imageType = VK_IMAGE_TYPE_2D;
    info.extent = { (uint32)EngineSettings::resolution.x, (uint32)EngineSettings::resolution.y, 1 };
    info.mipLevels = 1;
    info.arrayLayers = 1;
    info.format = FindBestDepthFormat(ctx);
    info.tiling = VK_IMAGE_TILING_OPTIMAL;
    info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    info.samples = VK_SAMPLE_COUNT_1_BIT;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    allocInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    VmaAllocationInfo allocationInfo{};

    auto res = vmaCreateImage(ctx.vcore.vmaAllocator, &info, &allocInfo, &ctx.screen.depthImage, &ctx.screen.depthAllocation,
        &allocationInfo);

    stat.vramUsage += allocationInfo.size;

    if (!ParseVkResult(res))
    {
        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog("Unable to create depth image.");
        return false;
    }

    VkImageViewCreateInfo depthViewInfo{};
    depthViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    depthViewInfo.image = ctx.screen.depthImage;
    depthViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    depthViewInfo.format = FindBestDepthFormat(ctx);
    depthViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    depthViewInfo.subresourceRange.baseMipLevel = 0;
    depthViewInfo.subresourceRange.levelCount = 1;
    depthViewInfo.subresourceRange.baseArrayLayer = 0;
    depthViewInfo.subresourceRange.layerCount = 1;

    vkCreateImageView(ctx.vcore.gpuDevice, &depthViewInfo, ctx.vcore.allocator, &ctx.screen.depthImageView);

    return true;
}

bool SetupTransferCommandBuffer(VulkanContext &ctx)
{
    VkCommandPoolCreateInfo commandPoolInfo{};
    commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolInfo.queueFamilyIndex = ctx.queues.primaryTransferQueueFamilyIndex;
    commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    VkCommandPool cmdPool{};
    auto res = vkCreateCommandPool(ctx.vcore.gpuDevice, &commandPoolInfo, ctx.vcore.allocator, &cmdPool);

    if (!ParseVkResult(res))
    {
        WEngine::WLog::SetConsoleWarning();
        WEngine::WLog::ConsoleLog("Unable to create transfer command buffer!");
        return false;
    }

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = cmdPool;
    allocInfo.commandBufferCount = 1;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    res = vkAllocateCommandBuffers(ctx.vcore.gpuDevice, &allocInfo, &ctx.transferCommandBuffer);

    if (!ParseVkResult(res))
    {
        WEngine::WLog::SetConsoleWarning();
        WEngine::WLog::ConsoleLog("Unable to create transfer command buffer!");
        return false;
    }


    VkSemaphoreCreateInfo semInfo{};
    semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vkCreateSemaphore(ctx.vcore.gpuDevice, &semInfo, ctx.vcore.allocator, &ctx.transferSemaphore);

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    vkCreateFence(ctx.vcore.gpuDevice, &fenceInfo, ctx.vcore.allocator, &ctx.transferFence);

    return true;
}

Vulkan_Texture CreateTexture(VulkanContext &ctx, VulkanStatistics &stat, const WEngine::TextureInfo &texInfo)
{
    Vulkan_Texture tex{};

    VkImageCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    info.imageType = VK_IMAGE_TYPE_2D;
    info.extent = { (uint32)texInfo.width, (uint32)texInfo.height, 1 };
    info.mipLevels = 1;
    info.arrayLayers = 1;
    info.format = VK_FORMAT_R8G8B8A8_UNORM;
    info.tiling = VK_IMAGE_TILING_OPTIMAL;
    info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    info.samples = VK_SAMPLE_COUNT_1_BIT;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    allocInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;


    VmaAllocationInfo allocationInfo{};

    auto res = vmaCreateImage(ctx.vcore.vmaAllocator, &info, &allocInfo, &tex.image, &tex.imageAllocation, &allocationInfo);

    stat.vramUsage += allocationInfo.size;

    if (!ParseVkResult(res))
    {
        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog("Unable to create texture.");
        return {};
    }

    VkImageViewCreateInfo imageViewInfo{};
    imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewInfo.image = tex.image;
    imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    // no mips for now
    imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageViewInfo.subresourceRange.baseMipLevel = 0;
    imageViewInfo.subresourceRange.levelCount = 1;
    imageViewInfo.subresourceRange.baseArrayLayer = 0;
    imageViewInfo.subresourceRange.layerCount = 1;

    res = vkCreateImageView(ctx.vcore.gpuDevice, &imageViewInfo, ctx.vcore.allocator, &tex.imageView);

    if (!ParseVkResult(res))
    {
        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog("Unable to create image view for texture.");
        return {};
    }

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 1.0f;

    res = vkCreateSampler(ctx.vcore.gpuDevice, &samplerInfo, ctx.vcore.allocator, &tex.sampler);

    if (!ParseVkResult(res))
    {
        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog("Unable to create sampler for texture.");
        return {};
    }

    return tex;
}

void BeginTextureUpload(VulkanContext &ctx)
{
    vkResetCommandBuffer(ctx.transferCommandBuffer, 0);
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(ctx.transferCommandBuffer, &beginInfo);
}

void QueueTexture(VulkanContext &ctx, const Vulkan_Texture& tex, const WEngine::TextureInfo &texInfo)
{
    VkImageMemoryBarrier pipeBarrier{};
    pipeBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    pipeBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    pipeBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    pipeBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    pipeBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    pipeBarrier.image = tex.image;
    pipeBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    pipeBarrier.subresourceRange.baseMipLevel = 0;
    pipeBarrier.subresourceRange.levelCount = 1;
    pipeBarrier.subresourceRange.baseArrayLayer = 0;
    pipeBarrier.subresourceRange.layerCount = 1;
    pipeBarrier.srcAccessMask = 0;
    pipeBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    vkCmdPipelineBarrier(ctx.transferCommandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
        0, 0, nullptr, 0, nullptr, 1, &pipeBarrier);

    VkBuffer staging;
    VmaAllocation stagingAlloc;

    const uint64 size = texInfo.width * texInfo.height * texInfo.channels;
    VkBufferCreateInfo bufInfo{};
    bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufInfo.size = size;
    bufInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
    allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

    VmaAllocationInfo allocationInfo{};
    vmaCreateBuffer(ctx.vcore.vmaAllocator, &bufInfo, &allocInfo, &staging, &stagingAlloc, &allocationInfo);

    void *mapped;
    vmaMapMemory(ctx.vcore.vmaAllocator, stagingAlloc, &mapped);
    memcpy(mapped, texInfo.data, size);
    vmaUnmapMemory(ctx.vcore.vmaAllocator, stagingAlloc);

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = { (uint32)texInfo.width, (uint32)texInfo.height, 1 };

    vkCmdCopyBufferToImage(ctx.transferCommandBuffer, staging, tex.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    VkImageMemoryBarrier postBarrier = pipeBarrier;
    postBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    postBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    postBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    postBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(ctx.transferCommandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        0, 0, nullptr, 0, nullptr, 1, &postBarrier);

    ctx.stagingBuffers.push_back({staging, stagingAlloc});
}

void UploadTextures(VulkanContext &ctx)
{
    vkEndCommandBuffer(ctx.transferCommandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &ctx.transferCommandBuffer;

    vkQueueSubmit(ctx.queues.primaryTransferQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(ctx.queues.primaryTransferQueue);

    for (auto& staging : ctx.stagingBuffers)
    {
        vmaDestroyBuffer(ctx.vcore.vmaAllocator, staging.first, staging.second);
    }
    ctx.stagingBuffers.clear();
}

#endif
