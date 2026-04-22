#include <Engine/EngineDefines.h>
#if GPU_BACKEND == GPU_VULKAN

#include <memory>

#include "GPU.h"

#include <Engine/EngineDefines.h>
#include <Game/GameDefines.h>

#include <vulkan/vulkan.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include "Engine/imgui/backends/imgui_impl_vulkan.h"
#include "Engine/imgui/backends/imgui_impl_sdl3.h"

#include "GPUSettings.h"
#include "Engine/Util/BitwiseMacros.h"
#include "Engine/Util/Log.h"

// --------------------------------------------------------------------------------------------------------------------
// ----------------------------------------------------- [ MEMORY ] ---------------------------------------------------
// --------------------------------------------------------------------------------------------------------------------

void* VulkanAllocate(void*, size_t size, size_t alignment, VkSystemAllocationScope scope)
{
    return WAllocator::AllocateAligned(size, alignment);
}
void* VulkanReallocate(void*, void* ptr, size_t newSize, size_t alignment, VkSystemAllocationScope scope)
{
    return WAllocator::ReallocateAligned(ptr, newSize, alignment);
}
void VulkanFree(void*, void* ptr)
{
    WAllocator::Free(ptr);
}
void VulkanInternalAllocation(void*, size_t size, VkInternalAllocationType allocationType, VkSystemAllocationScope scope)
{
    WAllocator::ReportExternalAllocation(size);
}
void VulkanInternalFree(void*, size_t size, VkInternalAllocationType allocationType, VkSystemAllocationScope scope)
{
    WAllocator::ReportExternalFree(size);
}

// --------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------- [GPU API TYPES] -------------------------------------------------
// --------------------------------------------------------------------------------------------------------------------

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


// --------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------ [GPU API VARIABLES] -----------------------------------------------
// --------------------------------------------------------------------------------------------------------------------

VkDebugUtilsMessengerEXT validationMessenger = VK_NULL_HANDLE;
VkAllocationCallbacks* allocator = nullptr;
VkAllocationCallbacks allocatorInternal;

VkInstance instance = VK_NULL_HANDLE;
VkPhysicalDevice gpuPhysicalDevice = VK_NULL_HANDLE;
VkDevice gpuDevice = VK_NULL_HANDLE;

VkSurfaceKHR screen = VK_NULL_HANDLE;
VkSwapchainKHR swapchain = VK_NULL_HANDLE;
wtl::vector<VkImage> swapchainImages;
uint32 swapchainCurrentImage = 0;
VkSemaphore imageAvailableSem = VK_NULL_HANDLE;
VkSemaphore renderFinishedSem = VK_NULL_HANDLE;
VkFence endOfFrameFence = VK_NULL_HANDLE;

uint32 queueFamilyCount = 0;
wtl::vector<QueueFamily> queueFamilies;
VkQueue primaryDrawQueue = VK_NULL_HANDLE;


// --------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------ [GPU API HELPERS] -------------------------------------------------
// --------------------------------------------------------------------------------------------------------------------

// This should be in an if statement. Returns true on success, false on any failure.
bool ParseVkResult(VkResult result)
{
    if (result == VK_SUCCESS)
        return true;

    WEngine::WLog::SetConsoleError();
    WEngine::WLog::ConsoleLog("[GPU ERROR] VkResult was not success!");
    return false;
}

void SetupAllocator()
{
    if constexpr (!GPUSettingsVulkan::useWAllocator)
        return;

    allocatorInternal.pfnAllocation = VulkanAllocate;
    allocatorInternal.pfnReallocation = VulkanReallocate;
    allocatorInternal.pfnFree = VulkanFree;
    allocatorInternal.pfnInternalAllocation = VulkanInternalAllocation;
    allocatorInternal.pfnInternalFree = VulkanInternalFree;

    allocator = &allocatorInternal;
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
        WEngine::WLog::ConsoleLog(std::format("{} Non-optimal use of Vulkan!\"{}\"\n{}", warnStart, pCallbackData->pMessageIdName, pCallbackData->pMessage));


    return VK_FALSE;
}

bool SetupValidation()
{
    if constexpr (!GPUSettingsVulkan::enableValidation)
        return true;

    auto vkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));

    if (vkCreateDebugUtilsMessengerEXT == nullptr)
    {
        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog("Failed to load vkCreateDebugUtilsMessengerEXT");
        return false;
    }

    VkDebugUtilsMessengerCreateInfoEXT info{};
    info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    info.flags = 0;
    info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    info.pfnUserCallback = ValidationCallback;
    info.pUserData = nullptr;

    auto res = vkCreateDebugUtilsMessengerEXT(instance, &info, allocator, &validationMessenger);

    return ParseVkResult(res);
}

wtl::vector<std::string> GetExtensionsToLoad()
{
    wtl::vector<std::string> extensions = {VK_EXT_DEBUG_UTILS_EXTENSION_NAME};
    Uint32 count_instance_extensions;
    const char * const *instance_extensions = SDL_Vulkan_GetInstanceExtensions(&count_instance_extensions);

    if (instance_extensions != nullptr)
    {
        int count_extensions = count_instance_extensions + 1;
        for (int i = 1; i < count_extensions; i++)
            extensions.emplace_back(instance_extensions[i]);
    }

    return extensions;
}

bool SetupVkInstance()
{
    const char *validationLayers[] = { "VK_LAYER_KHRONOS_validation" };

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = GameSettings::gameName.c_str();
    appInfo.pEngineName = EngineSettings::engineName.c_str();
    appInfo.apiVersion = VK_API_VERSION_1_4;

    wtl::vector<std::string> extensions = GetExtensionsToLoad();
    std::vector<const char*> extensionsData;
    extensionsData.reserve(extensions.size());
    for (const auto& ext : extensions) {
        extensionsData.push_back(ext.c_str());
    }

    VkInstanceCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    info.pApplicationInfo = &appInfo;
    info.enabledExtensionCount = extensions.size();
    info.ppEnabledExtensionNames = extensionsData.data();
    info.enabledLayerCount = 1;
    info.ppEnabledLayerNames = validationLayers;

    auto resIsnt = vkCreateInstance(&info, allocator, &instance);

    return ParseVkResult(resIsnt);
}

wtl::vector<VkDeviceQueueCreateInfo> FindDeviceQueues()
{
    vkGetPhysicalDeviceQueueFamilyProperties2(gpuPhysicalDevice, &queueFamilyCount, nullptr);
    wtl::vector<VkQueueFamilyProperties2> families(queueFamilyCount);
    for (auto& family : families)
        family.sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2;
    vkGetPhysicalDeviceQueueFamilyProperties2(gpuPhysicalDevice, &queueFamilyCount, families.data());

    queueFamilies.resize(queueFamilyCount);
    wtl::vector<VkDeviceQueueCreateInfo> infos(queueFamilyCount);

    for (int i = 0; i < queueFamilyCount; i++)
    {
        const auto& properties = families[i].queueFamilyProperties;
        queueFamilies[i].familyIndex = i;
        queueFamilies[i].queues.resize(properties.queueCount);
        queueFamilies[i].purpose = properties.queueFlags;

        // yes its leaking, but its so little that it wouldnt even matter on a snes.
        float32* queuePriorities = (float32*)WAllocator::Allocate(properties.queueCount * sizeof(float32));
        // ooga booga first has high priority
        for (int i = 0; i < properties.queueCount; i++)
            queuePriorities[i] = 0.0f;
        queuePriorities[0] = 1.0f;



        VkDeviceQueueCreateInfo queueInfo{};
        queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfo.queueFamilyIndex = i;
        queueInfo.queueCount = properties.queueCount;
        queueInfo.pQueuePriorities = queuePriorities;
        infos[i] = queueInfo;
    }

    for (auto& familiy : queueFamilies)
    {
        if (familiy.purpose & (uint8)QueuePurpose::Drawing)
        {
            primaryDrawQueue = familiy.queues[0];
            break;
        }
    }

    return infos;
}

void SetupDeviceQueues()
{
    for (int i = 0; i < queueFamilyCount; i++)
    {
        if (queueFamilies[i].purpose & (uint8)QueuePurpose::Drawing)
        {
            vkGetDeviceQueue(gpuDevice, i, 0, &queueFamilies[i].queues[0]);
            primaryDrawQueue = queueFamilies[i].queues[0];
            break;
        }
    }
}

bool SetupGraphicsDevice()
{
    uint32 gpusPresent;
    auto resPhy = vkEnumeratePhysicalDevices(instance, &gpusPresent, nullptr);

    if (!ParseVkResult(resPhy))
        return false;

    if (gpusPresent < 1)
    {
        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog("[GPU ERROR] No GPU found!");
        return false;
    }

    wtl::vector<VkPhysicalDevice> gpus(gpusPresent);
    auto resPhy2 = vkEnumeratePhysicalDevices(instance, &gpusPresent, gpus.data());

    if (!ParseVkResult(resPhy2))
        return false;

    for (uint32 i = 0; i < gpusPresent; i++)
    {
        VkPhysicalDeviceProperties2 properties{};
        properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        vkGetPhysicalDeviceProperties2(gpus[i], &properties);

        WEngine::WLog::ConsoleLog(std::format("Found GPU: {}", properties.properties.deviceName));
    }

    gpuPhysicalDevice = gpus[0];

    VkPhysicalDeviceProperties2 properties{};
    properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    vkGetPhysicalDeviceProperties2(gpuPhysicalDevice, &properties);

    WEngine::WLog::ConsoleLog(std::format("GPU selected for rendering: {}", properties.properties.deviceName));

    auto queues = FindDeviceQueues();

    const char* extensions[1] = { "VK_KHR_swapchain" };

    VkDeviceCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    info.queueCreateInfoCount = queues.size();
    info.pQueueCreateInfos = queues.data();
    info.enabledExtensionCount = 1;
    info.ppEnabledExtensionNames = extensions;

    // Do I also have to hire a babysitter for the damn gpu??
    auto resDev = vkCreateDevice(gpuPhysicalDevice, &info, allocator, &gpuDevice);

    SetupDeviceQueues();

    if (!ParseVkResult(resDev))
        return false;

    return true;
}

bool SetupSwapchain()
{
    VkSurfaceCapabilitiesKHR capabilities{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpuPhysicalDevice, screen, &capabilities);

    uint32 fmtCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(gpuPhysicalDevice, screen, &fmtCount, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(fmtCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(gpuPhysicalDevice, screen, &fmtCount, formats.data());


    VkSwapchainCreateInfoKHR info{};
    info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    info.surface = screen;
    info.minImageCount = capabilities.minImageCount;

    info.imageFormat = VK_FORMAT_B8G8R8A8_SRGB;
    info.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    info.imageExtent = capabilities.currentExtent;
    info.imageArrayLayers = 1;
    info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    info.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
    info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    info.oldSwapchain = VK_NULL_HANDLE;

    auto res = vkCreateSwapchainKHR(gpuDevice, &info, allocator, &swapchain);

    VkSemaphoreCreateInfo semInfo{};
    semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vkCreateSemaphore(gpuDevice, &semInfo, allocator, &imageAvailableSem);
    vkCreateSemaphore(gpuDevice, &semInfo, allocator, &renderFinishedSem);

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    vkCreateFence(gpuDevice, &fenceInfo, allocator, &endOfFrameFence);

    uint32 swapchainImageCount;
    vkGetSwapchainImagesKHR(gpuDevice, swapchain, &swapchainImageCount, nullptr);
    swapchainImages.resize(swapchainImageCount);
    vkGetSwapchainImagesKHR(gpuDevice, swapchain, &swapchainImageCount, swapchainImages.data());

    return ParseVkResult(res);
}

// --------------------------------------------------------------------------------------------------------------------
// ------------------------------------------ [GPU INTERFACE IMPLEMENTATION] ------------------------------------------
// --------------------------------------------------------------------------------------------------------------------

void GPU::SETTING_ConfigureSDL()
{

}

bool GPU::SETTING_InitGPUApi(SDL_Window *window)
{
    if (!SetupVkInstance())
        return false;

#ifndef PACKAGE
    SetupValidation();
#endif

    if (!SetupGraphicsDevice())
        return false;

    if (!SDL_Vulkan_CreateSurface(window, instance, allocator, &screen))
    {
        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog(std::format("Failed to create a Vulkan surface for the screen, {}", SDL_GetError()));
        return false;
    }

    if (!SetupSwapchain())
        return false;

    return true;
}

void GPU::SETTING_ConfigureImGui(SDL_Window *window)
{
    ImGui_ImplSDL3_InitForVulkan(window);
    ImGui_ImplVulkan_InitInfo initInfo{};
    initInfo.ApiVersion = VK_API_VERSION_1_4;
    initInfo.Instance = instance;
    initInfo.PhysicalDevice = gpuPhysicalDevice;
    initInfo.Device = gpuDevice;
    initInfo.Allocator = allocator;

    //ImGui_ImplVulkan_Init(&initInfo);
}

void GPU::SETTING_SetViewport(WEngine::Vector2 topLeft, WEngine::Vector2 bottomRight)
{

}

void GPU::SETTING_ToggleBlending(bool enabled)
{

}

void GPU::SETTING_ToggleDepthTest(bool enabled)
{

}

void GPU::SETTING_ToggleWireFrame(bool enabled)
{

}

WEngine::Nullable<WEngine::Shader> GPU::ALLOC_CompileShader(const std::string &name)
{
    return WEngine::Nullable<WEngine::Shader>();
}

WEngine::Nullable<WEngine::Shader> GPU::GetShader(const std::string &name)
{
    return WEngine::Nullable<WEngine::Shader>();
}

WEngine::Nullable<WEngine::Framebuffer> GPU::ALLOC_CreateFramebuffer(const WEngine::Vector2 &resolution)
{
    return WEngine::Nullable<WEngine::Framebuffer>();
}

void GPU::SETTING_SelectFramebuffer(WEngine::Framebuffer framebuffer)
{

}

void GPU::SETTING_SelectFramebufferScreen()
{

}

WEngine::Nullable<WEngine::Texture> GPU::GetFramebufferTexture(WEngine::Framebuffer framebuffer)
{
    return WEngine::Nullable<WEngine::Texture>();
}

WEngine::Nullable<WEngine::Model> GPU::ALLOC_CreateModel(const WEngine::ModelInfo &model)
{
    return WEngine::Nullable<WEngine::Model>();
}

WEngine::Nullable<WEngine::Line> GPU::ALLOC_CreateLine()
{
    return WEngine::Nullable<WEngine::Line>();
}

WEngine::Nullable<WEngine::Texture> GPU::ALLOC_CreateTexture(const WEngine::TextureInfo &textureData)
{
    return WEngine::Nullable<WEngine::Texture>();
}

WEngine::Nullable<WEngine::Texture> GPU::GetTexture(const std::string &name)
{
    return WEngine::Nullable<WEngine::Texture>();
}

WEngine::Nullable<WEngine::Vector2> GPU::GetTextureSize(WEngine::Texture texture)
{
    return WEngine::Nullable<WEngine::Vector2>();
}

void GPU::DRAWCALL_ClearScreen(WEngine::Color clearColor)
{
    WEngine::WLog::ConsoleLog("About to get some new images!");
    vkAcquireNextImageKHR(gpuDevice, swapchain, max_uint64, imageAvailableSem, VK_NULL_HANDLE, &swapchainCurrentImage);
    WEngine::WLog::ConsoleLog(std::format("Got some: {}!", swapchainCurrentImage));
}

void GPU::DRAWCALL_DrawModel(WEngine::Model model, WEngine::Shader shader, const WEngine::ShaderSettings &settings)
{

}

void GPU::DRAWCALL_DrawLine(WEngine::Line line, WEngine::Shader shader, const WEngine::ShaderSettings &settings,
    const WEngine::LineInfo &newModel)
{

}

void GPU::DRAWCALL_ResetImGui()
{

}

void GPU::DRAWCALL_DrawImGui()
{

}

void GPU::DRAWCALL_SwapBuffers(SDL_Window *window)
{
    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &imageAvailableSem;
    submitInfo.pWaitDstStageMask = &waitStage;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &renderFinishedSem;
    submitInfo.commandBufferCount = 0;

    vkQueueSubmit(primaryDrawQueue, 1, &submitInfo, endOfFrameFence);

    vkWaitForFences(gpuDevice, 1, &endOfFrameFence, VK_TRUE, UINT64_MAX);
    vkResetFences(gpuDevice, 1, &endOfFrameFence);

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &renderFinishedSem;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain;
    presentInfo.pImageIndices = &swapchainCurrentImage;
    presentInfo.pResults = nullptr;

    vkQueuePresentKHR(primaryDrawQueue, &presentInfo);
}

uint64 GPU::GetVramUsage()
{
    return 0;
}

uint32 GPU::GetDrawCallCountLastFrame()
{
    return 0;
}

WEngine::Nullable<ImTextureID> GPU::FramebufferToImGui(WEngine::Framebuffer framebuffer)
{
    return WEngine::Nullable<ImTextureID>();
}

#endif
