#include <Engine/EngineDefines.h>
#if GPU_BACKEND == GPU_VULKAN

#include "VulkanCore.h"

#include "VulkanHelpers.h"
#include "VulkanQueues.h"
#include "Engine/Core/System/GPUSettings.h"
#include "Engine/Core/System/Memory.h"
#include "Engine/Util/Log.h"
#include "Game/GameDefines.h"
#include "SDL3/SDL_vulkan.h"

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

void SetupAllocator(VulkanContext& ctx)
{
    if constexpr (!GPUSettingsVulkan::useWAllocator)
        return;

    ctx.vcore.allocatorInternal.pfnAllocation = VulkanAllocate;
    ctx.vcore.allocatorInternal.pfnReallocation = VulkanReallocate;
    ctx.vcore.allocatorInternal.pfnFree = VulkanFree;
    ctx.vcore.allocatorInternal.pfnInternalAllocation = VulkanInternalAllocation;
    ctx.vcore.allocatorInternal.pfnInternalFree = VulkanInternalFree;

    ctx.vcore.allocator = &ctx.vcore.allocatorInternal;
}

void SetupVmaAllocator(VulkanContext& ctx)
{
    VmaVulkanFunctions vulkanFunctions = {};
    vulkanFunctions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
    vulkanFunctions.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;

    VmaAllocatorCreateInfo allocatorCreateInfo = {};
    allocatorCreateInfo.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
    allocatorCreateInfo.vulkanApiVersion = GetVulkanVersion();
    allocatorCreateInfo.physicalDevice = ctx.vcore.gpuPhysicalDevice;
    allocatorCreateInfo.device = ctx.vcore.gpuDevice;
    allocatorCreateInfo.instance = ctx.vcore.instance;
    allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;


    vmaCreateAllocator(&allocatorCreateInfo, &ctx.vcore.vmaAllocator);
}

bool SetupValidation(VulkanContext& ctx)
{
    if constexpr (!GPUSettingsVulkan::enableValidation)
        return true;

    auto vkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(ctx.vcore.instance, "vkCreateDebugUtilsMessengerEXT"));

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

    auto res = vkCreateDebugUtilsMessengerEXT(ctx.vcore.instance, &info, ctx.vcore.allocator,
        &ctx.vcore.validationMessenger);

    return ParseVkResult(res);
}

wtl::vector<std::string> GetExtensionsToLoad(VulkanContext& ctx)
{
    wtl::vector<std::string> extensions = {VK_EXT_DEBUG_UTILS_EXTENSION_NAME};
    uint32 count_instance_extensions;
    const char * const *instance_extensions = SDL_Vulkan_GetInstanceExtensions(&count_instance_extensions);

    if (instance_extensions != nullptr)
    {
        int count_extensions = count_instance_extensions + 1;
        for (int i = 1; i < count_extensions; i++)
            extensions.emplace_back(instance_extensions[i]);
    }

    return extensions;
}

bool SetupVkInstance(VulkanContext& ctx)
{
    const char *validationLayers[] = { "VK_LAYER_KHRONOS_validation" };

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = GameSettings::gameName.c_str();
    appInfo.pEngineName = EngineSettings::engineName.c_str();
    appInfo.apiVersion = GetVulkanVersion();

    wtl::vector<std::string> extensions = GetExtensionsToLoad(ctx);
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

    auto resIsnt = vkCreateInstance(&info, ctx.vcore.allocator, &ctx.vcore.instance);

    return ParseVkResult(resIsnt);
}

bool SetupGraphicsDevice(VulkanContext& ctx)
{
    uint32 gpusPresent;
    auto resPhy = vkEnumeratePhysicalDevices(ctx.vcore.instance, &gpusPresent, nullptr);

    if (!ParseVkResult(resPhy))
        return false;

    if (gpusPresent < 1)
    {
        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog("[GPU ERROR] No GPU found!");
        return false;
    }

    wtl::vector<VkPhysicalDevice> gpus(gpusPresent);
    auto resPhy2 = vkEnumeratePhysicalDevices(ctx.vcore.instance, &gpusPresent, gpus.data());

    if (!ParseVkResult(resPhy2))
        return false;

    for (uint32 i = 0; i < gpusPresent; i++)
    {
        VkPhysicalDeviceProperties2 properties{};
        properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        vkGetPhysicalDeviceProperties2(gpus[i], &properties);

        WEngine::WLog::ConsoleLog(std::format("Found GPU: {}", properties.properties.deviceName));
    }

    ctx.vcore.gpuPhysicalDevice = gpus[0];

    VkPhysicalDeviceProperties2 properties{};
    properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    vkGetPhysicalDeviceProperties2(ctx.vcore.gpuPhysicalDevice, &properties);

    WEngine::WLog::ConsoleLog(std::format("GPU selected for rendering: {}", properties.properties.deviceName));

    auto queues = FindDeviceQueues(ctx);

    const char* extensions[1] = { "VK_KHR_swapchain" };

    VkDeviceCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    info.queueCreateInfoCount = queues.size();
    info.pQueueCreateInfos = queues.data();
    info.enabledExtensionCount = 1;
    info.ppEnabledExtensionNames = extensions;

    // Do I also have to hire a babysitter for the damn gpu??
    auto resDev = vkCreateDevice(ctx.vcore.gpuPhysicalDevice, &info, ctx.vcore.allocator, &ctx.vcore.gpuDevice);

    SetupDeviceQueues(ctx);

    for (auto queue : queues)
        wFree((void*)queue.pQueuePriorities);


    if (!ParseVkResult(resDev))
        return false;

    return true;
}

#endif