#include <Engine/EngineDefines.h>
#if GPU_BACKEND == GPU_VULKAN

#include "VulkanHelpers.h"
#include <Engine/Core/System/GPUSettings.h>
#include "Engine/Util/Log.h"
#include "Engine/WTL/vector.h"
#include <vulkan/vk_enum_string_helper.h>

bool ParseVkResult(VkResult result)
{
    if (result == VK_SUCCESS)
        return true;

    WEngine::WLog::SetConsoleError();
    WEngine::WLog::ConsoleLog(std::format("[GPU ERROR] VkResult was not success! {}", string_VkResult(result)));

    switch (GPUSettingsVulkan::invalidResultAction)
    {
        case GPUSettingsVulkan::InvalidResultAction::LetGo:
            return false;
        case GPUSettingsVulkan::InvalidResultAction::Stall:
            while (true);
        case GPUSettingsVulkan::InvalidResultAction::Abort:
            abort();
        default:
            return false;
    }
}

VkBool32 ValidationCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void*)
{
    if (pCallbackData->pMessage == nullptr)
        return VK_FALSE;

    std::string warnStart = "[GPU ";

    if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    {
        warnStart += "Warning]";
        WEngine::WLog::SetConsoleWarning();
    }
    else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
    {
        warnStart += "Error]";
        WEngine::WLog::SetConsoleError();
    }
    else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
    {
        warnStart += "Info]";
        WEngine::WLog::SetConsoleInfo();
    }

    if (messageTypes == VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
        WEngine::WLog::ConsoleLog(std::format("{} Validation layer tripped! \"{}\"\n{}", warnStart, pCallbackData->pMessageIdName, pCallbackData->pMessage));
    else if (messageTypes == VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
        WEngine::WLog::ConsoleLog(std::format("{} Non-optimal use of Vulkan! \"{}\"\n{}", warnStart, pCallbackData->pMessageIdName, pCallbackData->pMessage));

    if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
    {
        switch (GPUSettingsVulkan::validationErrorAction)
        {
            case GPUSettingsVulkan::InvalidResultAction::LetGo:
                return VK_FALSE;
            case GPUSettingsVulkan::InvalidResultAction::Stall:
                while (true);
            case GPUSettingsVulkan::InvalidResultAction::Abort:
                abort();
            default:
                return VK_FALSE;
        }
    }

    return VK_FALSE;
}

uint64 CalcTextureSize(uint8 bytesPerPixel, uint32 width, uint32 height)
{
    return width * height * bytesPerPixel;
}

uint64 CalcModelSize(uint8 bytesPerVertex, uint32 vertexCount)
{
    return bytesPerVertex * vertexCount;
}

VkFormat FindBestDepthFormat(const VulkanContext& ctx)
{
    wtl::vector<VkFormat> candidates =
    {
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT
    };

    for (VkFormat format : candidates)
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(ctx.vcore.gpuPhysicalDevice, format, &props);

        if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
            return format;
    }

    return VK_FORMAT_UNDEFINED;
}

VkFormat FindBestSwapchainFormat(const VulkanContext &ctx)
{
    wtl::vector<VkFormat> candidates =
    {
        VK_FORMAT_B8G8R8A8_UNORM,
        VK_FORMAT_R8G8B8A8_UNORM
    };

    for (VkFormat format : candidates)
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(ctx.vcore.gpuPhysicalDevice, format, &props);

        if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT)
            return format;
    }

    return VK_FORMAT_UNDEFINED;
}

uint64 GetSizeOfImageInBytes(WEngine::Vector2 imageSize, uint8 channelCount)
{
    return (uint64)imageSize.x * (uint64)imageSize.y * channelCount;
}

uint32 GetVulkanVersion()
{
    return VK_API_VERSION_1_4;
}

WEngine::Vector2 & GetFbResolution(const VulkanContext &ctx)
{
    return ctx.currentRenderTarget->resolution;
}

VkCommandBuffer& GetFbCmdBuff(const VulkanContext &ctx)
{
    return ctx.currentRenderTarget->cmdBuffs[ctx.screen.currentFrame];
}

VkImage& GetFbImage(const VulkanContext &ctx)
{
    return ctx.currentRenderTarget->targetImages[ctx.screen.currentFrame];
}

VkImageView& GetFbImageView(const VulkanContext &ctx)
{
    return ctx.currentRenderTarget->targetImageViews[ctx.screen.currentFrame];
}

VkSemaphore& GetFbImageAvailSem(const VulkanContext &ctx)
{
    return ctx.currentRenderTarget->imageAvailableSems[ctx.screen.currentFrame];
}

VkSemaphore& GetFbRenderFinishedSem(const VulkanContext &ctx)
{
    return ctx.currentRenderTarget->renderFinishedSems[ctx.screen.currentFrame];
}

VkFence& GetFbEndOfFrameFence(const VulkanContext &ctx)
{
    return ctx.currentRenderTarget->endOfFrameFences[ctx.screen.currentFrame];
}

void PopulatePushConstants(const VulkanContext &ctx, const Vulkan_Shader &shader, const WEngine::Mat4x4 &mvp)
{
    vkCmdPushConstants(GetFbCmdBuff(ctx), shader.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0,
        sizeof(WEngine::Mat4x4), &mvp);
}

#endif
