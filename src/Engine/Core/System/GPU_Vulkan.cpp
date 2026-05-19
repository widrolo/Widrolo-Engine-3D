#include <Engine/EngineDefines.h>
#if GPU_BACKEND == GPU_VULKAN

#include "Iris.h"

#include <Engine/EngineDefines.h>
#include <Game/GameDefines.h>

#include <vulkan/vulkan.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include "Engine/imgui/imgui.h"
#include "Engine/imgui/backends/imgui_impl_sdl3.h"
#include "Engine/imgui/backends/imgui_impl_vulkan.h"

#include "GPUSettings.h"
#include "Engine/Util/BitwiseMacros.h"
#include "Engine/Util/Log.h"

#include <Engine/Types/Rendering/VertextData.h>

#include "Engine/Core/Handlers/AssetRepo.h"
#include "Engine/Types/CoreSystems.h"
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

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
};

struct Vulkan_Core
{
    VkDebugUtilsMessengerEXT validationMessenger = VK_NULL_HANDLE;
    VkAllocationCallbacks* allocator = nullptr;
    VkAllocationCallbacks allocatorInternal;
    VmaAllocator vmaAllocator;

    VkInstance instance = VK_NULL_HANDLE;
    VkPhysicalDevice gpuPhysicalDevice = VK_NULL_HANDLE;
    VkDevice gpuDevice = VK_NULL_HANDLE;
};

struct Vulkan_Shader
{

};

struct Vulkan_Model
{
    VkBuffer vertexBuffer;
    VmaAllocation vertexAllocation;
    uint64 vertexCount;
};


// --------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------ [GPU API VARIABLES] -----------------------------------------------
// --------------------------------------------------------------------------------------------------------------------

Vulkan_Core vcore;
Vulkan_Screen screen;

uint32 queueFamilyCount = 0;
wtl::vector<QueueFamily> queueFamilies;
uint32 primaryDrawQueueFamilyIndex = 0;
VkQueue primaryDrawQueue = VK_NULL_HANDLE;

VkPipelineLayout pipelineLayout;

// These are just for prototyping

VkCommandPool testPool;
wtl::vector<VkCommandBuffer> cmdBufs;
VkRenderPass testPass;
VkPipeline testPipeline;
VkDescriptorPool descriptorPool;

wtl::vector<Vulkan_Shader> loadedShaders;
std::unordered_map<std::string, WEngine::Shader> loadedShadersHandles;
wtl::vector<Vulkan_Model> loadedModels;

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


    return VK_FALSE;
}

// --------------------------------------------------- [API SETUP] ----------------------------------------------------

void SetupAllocator()
{
    if constexpr (!GPUSettingsVulkan::useWAllocator)
        return;

    vcore.allocatorInternal.pfnAllocation = VulkanAllocate;
    vcore.allocatorInternal.pfnReallocation = VulkanReallocate;
    vcore.allocatorInternal.pfnFree = VulkanFree;
    vcore.allocatorInternal.pfnInternalAllocation = VulkanInternalAllocation;
    vcore.allocatorInternal.pfnInternalFree = VulkanInternalFree;

    vcore.allocator = &vcore.allocatorInternal;
}

void SetupVmaAllocator()
{
    VmaVulkanFunctions vulkanFunctions = {};
    vulkanFunctions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
    vulkanFunctions.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;

    VmaAllocatorCreateInfo allocatorCreateInfo = {};
    allocatorCreateInfo.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
    allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_2;
    allocatorCreateInfo.physicalDevice = vcore.gpuPhysicalDevice;
    allocatorCreateInfo.device = vcore.gpuDevice;
    allocatorCreateInfo.instance = vcore.instance;
    allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;


    vmaCreateAllocator(&allocatorCreateInfo, &vcore.vmaAllocator);
}

bool SetupValidation()
{
    if constexpr (!GPUSettingsVulkan::enableValidation)
        return true;

    auto vkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(vcore.instance, "vkCreateDebugUtilsMessengerEXT"));

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

    auto res = vkCreateDebugUtilsMessengerEXT(vcore.instance, &info, vcore.allocator, &vcore.validationMessenger);

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

    auto resIsnt = vkCreateInstance(&info, vcore.allocator, &vcore.instance);

    return ParseVkResult(resIsnt);
}

wtl::vector<VkDeviceQueueCreateInfo> FindDeviceQueues()
{
    vkGetPhysicalDeviceQueueFamilyProperties2(vcore.gpuPhysicalDevice, &queueFamilyCount, nullptr);
    wtl::vector<VkQueueFamilyProperties2> families(queueFamilyCount);
    for (auto& family : families)
        family.sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2;
    vkGetPhysicalDeviceQueueFamilyProperties2(vcore.gpuPhysicalDevice, &queueFamilyCount, families.data());

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

    uint32 count = 0; // piss and shit code
    for (auto& familiy : queueFamilies)
    {
        if (familiy.purpose & (uint8)QueuePurpose::Drawing)
        {
            primaryDrawQueue = familiy.queues[0];
            primaryDrawQueueFamilyIndex = count;
            break;
        }
        count++;
    }

    return infos;
}

void SetupDeviceQueues()
{
    for (int i = 0; i < queueFamilyCount; i++)
    {
        if (queueFamilies[i].purpose & (uint8)QueuePurpose::Drawing)
        {
            vkGetDeviceQueue(vcore.gpuDevice, i, 0, &queueFamilies[i].queues[0]);
            primaryDrawQueue = queueFamilies[i].queues[0];
            break;
        }
    }
}

bool SetupGraphicsDevice()
{
    uint32 gpusPresent;
    auto resPhy = vkEnumeratePhysicalDevices(vcore.instance, &gpusPresent, nullptr);

    if (!ParseVkResult(resPhy))
        return false;

    if (gpusPresent < 1)
    {
        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog("[GPU ERROR] No GPU found!");
        return false;
    }

    wtl::vector<VkPhysicalDevice> gpus(gpusPresent);
    auto resPhy2 = vkEnumeratePhysicalDevices(vcore.instance, &gpusPresent, gpus.data());

    if (!ParseVkResult(resPhy2))
        return false;

    for (uint32 i = 0; i < gpusPresent; i++)
    {
        VkPhysicalDeviceProperties2 properties{};
        properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        vkGetPhysicalDeviceProperties2(gpus[i], &properties);

        WEngine::WLog::ConsoleLog(std::format("Found GPU: {}", properties.properties.deviceName));
    }

    vcore.gpuPhysicalDevice = gpus[0];

    VkPhysicalDeviceProperties2 properties{};
    properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    vkGetPhysicalDeviceProperties2(vcore.gpuPhysicalDevice, &properties);

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
    auto resDev = vkCreateDevice(vcore.gpuPhysicalDevice, &info, vcore.allocator, &vcore.gpuDevice);

    SetupDeviceQueues();

    if (!ParseVkResult(resDev))
        return false;

    return true;
}

void SetupSwapchainFramebuffers()
{
    screen.swapchainImageViews.resize(screen.swapchainImages.size());
    screen.swapchainFramebuffers.resize(screen.swapchainImages.size());


    for (int i = 0; i < screen.swapchainImages.size(); i++)
    {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = screen.swapchainImages[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = VK_FORMAT_B8G8R8A8_UNORM;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        vkCreateImageView(vcore.gpuDevice, &viewInfo, vcore.allocator, &screen.swapchainImageViews[i]);

        VkFramebufferCreateInfo fbInfo{};
        fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fbInfo.renderPass = testPass;
        fbInfo.attachmentCount = 1;
        fbInfo.pAttachments = &screen.swapchainImageViews[i];
        fbInfo.width = EngineSettings::resolution.x;
        fbInfo.height = EngineSettings::resolution.y;
        fbInfo.layers = 1;

        vkCreateFramebuffer(vcore.gpuDevice, &fbInfo, vcore.allocator, &screen.swapchainFramebuffers[i]);
    }
}

bool SetupSwapchain()
{
    VkSurfaceCapabilitiesKHR capabilities{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vcore.gpuPhysicalDevice, screen.screen, &capabilities);

    uint32 fmtCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(vcore.gpuPhysicalDevice, screen.screen, &fmtCount, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(fmtCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(vcore.gpuPhysicalDevice, screen.screen, &fmtCount, formats.data());


    VkSwapchainCreateInfoKHR info{};
    info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    info.surface = screen.screen;
    info.minImageCount = capabilities.minImageCount;

    info.imageFormat = VK_FORMAT_B8G8R8A8_UNORM;
    info.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    info.imageExtent = capabilities.currentExtent;
    info.imageArrayLayers = 1;
    info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    info.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
    info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    info.oldSwapchain = VK_NULL_HANDLE;

    auto res = vkCreateSwapchainKHR(vcore.gpuDevice, &info, vcore.allocator, &screen.swapchain);

    uint32 swapchainImageCount;
    vkGetSwapchainImagesKHR(vcore.gpuDevice, screen.swapchain, &swapchainImageCount, nullptr);
    screen.swapchainImages.resize(swapchainImageCount);
    vkGetSwapchainImagesKHR(vcore.gpuDevice, screen.swapchain, &swapchainImageCount, screen.swapchainImages.data());

    screen.imageAvailableSems.resize(swapchainImageCount);
    screen.renderFinishedSems.resize(swapchainImageCount);
    screen.endOfFrameFences.resize(swapchainImageCount);


    VkSemaphoreCreateInfo semInfo{};
    semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    for (uint32 i = 0; i < swapchainImageCount; i++)
        vkCreateSemaphore(vcore.gpuDevice, &semInfo, vcore.allocator, &screen.imageAvailableSems[i]);
    for (uint32 i = 0; i < swapchainImageCount; i++)
        vkCreateSemaphore(vcore.gpuDevice, &semInfo, vcore.allocator, &screen.renderFinishedSems[i]);

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    for (uint32 i = 0; i < swapchainImageCount; i++)
        vkCreateFence(vcore.gpuDevice, &fenceInfo, vcore.allocator, &screen.endOfFrameFences[i]);


    return ParseVkResult(res);
}

bool SetupPipelineLayout()
{
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    auto resLayout = vkCreatePipelineLayout(vcore.gpuDevice, &pipelineLayoutInfo, vcore.allocator, &pipelineLayout);
    if (!ParseVkResult(resLayout))
    {
        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog("Unable to create pipeline layout!");
        return false;
    }
    return true;
}

bool SetupDescriptorPool()
{
    // So far im only using this for imgui, so please adjust when needed.

    wtl::vector<VkDescriptorPoolSize> poolSizes =
    {
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE},
        { VK_DESCRIPTOR_TYPE_SAMPLER, IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE },
    };
    VkDescriptorPoolCreateInfo descInfo{};
    descInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    descInfo.maxSets = 0;
    for (VkDescriptorPoolSize& pool_size : poolSizes)
        descInfo.maxSets += pool_size.descriptorCount;
    descInfo.poolSizeCount = poolSizes.size();
    descInfo.pPoolSizes = poolSizes.data();

    auto res = vkCreateDescriptorPool(vcore.gpuDevice, &descInfo, vcore.allocator, &descriptorPool);

    if (!ParseVkResult(res))
    {
        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog("Unable to create descriptor pool!");
        return false;
    }
    return true;
}

// ------------------------------------------------- [MEM FUNCTIONS] --------------------------------------------------

VkCommandPool CreateCommandPool()
{
    VkCommandPool commandPool;

    VkCommandPoolCreateInfo commandPoolInfo{};
    commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolInfo.queueFamilyIndex = primaryDrawQueueFamilyIndex;
    commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    auto res = vkCreateCommandPool(vcore.gpuDevice, &commandPoolInfo, vcore.allocator, &commandPool);

    if (!ParseVkResult(res))
    {
        WEngine::WLog::SetConsoleWarning();
        WEngine::WLog::ConsoleLog("Unable to create command buffer!");
        return VK_NULL_HANDLE;
    }
    return commandPool;
}

// -------------------------------------------------- [RENDER SETUP] --------------------------------------------------

VkCommandBuffer CreateCommandBuffer(VkCommandPool cmdPool)
{
    VkCommandBuffer cmdBuff;

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = cmdPool;
    allocInfo.commandBufferCount = 1;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    auto res = vkAllocateCommandBuffers(vcore.gpuDevice, &allocInfo, &cmdBuff);

    if (!ParseVkResult(res))
    {
        WEngine::WLog::SetConsoleWarning();
        WEngine::WLog::ConsoleLog("Unable to create command buffer!");
        return VK_NULL_HANDLE;
    }

    return cmdBuff;
}

VkRenderPass CreateBasicRenderPass()
{
    VkRenderPass renderPass;

    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = VK_FORMAT_B8G8R8A8_UNORM;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;
    auto res = vkCreateRenderPass(vcore.gpuDevice, &renderPassInfo, vcore.allocator, &renderPass);
    if (!ParseVkResult(res))
    {
        WEngine::WLog::SetConsoleWarning();
        WEngine::WLog::ConsoleLog("Unable to create command buffer!");
        return VK_NULL_HANDLE;
    }
    return renderPass;
}

VkShaderModule CompileShader(const WEngine::SpirVAssetMission& spirvAssetMission)
{
    VkShaderModule module;
    VkShaderModuleCreateInfo shaderModuleInfo{};
    shaderModuleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleInfo.codeSize = spirvAssetMission.shaderSize;
    shaderModuleInfo.pCode = spirvAssetMission.shaderCode;

    auto res = vkCreateShaderModule(vcore.gpuDevice, &shaderModuleInfo, vcore.allocator, &module);
    if (!ParseVkResult(res))
    {
        WEngine::WLog::SetConsoleWarning();
        WEngine::WLog::ConsoleLog("Unable to create shader module!");
        return VK_NULL_HANDLE;
    }
    return module;
}

VkPipeline CreateBasicPipeline(VkRenderPass renderPass)
{
    VkPipeline pipeline;

    // ----------------------------------------------- [INPUT ASSEMBLY] -----------------------------------------------

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
    inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    // --------------------------------------------- [VERTEX DEFINITION] ----------------------------------------------

    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(WEngine::VertexData);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    // [0] = Position (Vector3)   |   [1] = Color (Vector3)   |   [2] = Color UV (Vector2)   |   [3] = Shadow UV (Vector2)
    std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT; // Khronos had a meth party while making this one
    attributeDescriptions[0].offset = offsetof(WEngine::VertexData, position);
    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT; // Khronos had a meth party while making this one
    attributeDescriptions[1].offset = offsetof(WEngine::VertexData, vertColor);
    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT; // Khronos had a meth party while making this one
    attributeDescriptions[2].offset = offsetof(WEngine::VertexData, uv0Coord);
    attributeDescriptions[3].binding = 0;
    attributeDescriptions[3].location = 3;
    attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT; // Khronos had a meth party while making this one
    attributeDescriptions[3].offset = offsetof(WEngine::VertexData, uv1Coord);

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    // ------------------------------------------ [RASTERIZER AND RENDERING] ------------------------------------------

    VkPipelineRasterizationStateCreateInfo rasterInfo{};
    rasterInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterInfo.lineWidth = 1.0f;

    VkPipelineColorBlendAttachmentState blendState{};
    blendState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo blendInfo{};
    blendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    blendInfo.attachmentCount = 1;
    blendInfo.pAttachments = &blendState;

    VkPipelineViewportStateCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewInfo.viewportCount = 1;
    viewInfo.scissorCount = 1;

    VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};
    depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

    VkPipelineMultisampleStateCreateInfo multisampleInfo{};
    multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    std::array<VkDynamicState, 2> dynamics{};
    dynamics[0] = VK_DYNAMIC_STATE_VIEWPORT;
    dynamics[1] = VK_DYNAMIC_STATE_SCISSOR;

    VkPipelineDynamicStateCreateInfo dynamicInfo{};
    dynamicInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicInfo.dynamicStateCount = dynamics.size();
    dynamicInfo.pDynamicStates = dynamics.data();

    // -------------------------------------------------- [SHADERS] ---------------------------------------------------

    // Sperm Vee
    WEngine::SpirVAssetMission vertexShaderCode{};
    vertexShaderCode.shaderType = WEngine::SpirVAssetMission::VertexShader;
    vertexShaderCode.name = "triangle";
    WEngine::SpirVAssetMission fragmentShaderCode{};
    fragmentShaderCode.shaderType = WEngine::SpirVAssetMission::FragmentShader;
    fragmentShaderCode.name = "triangle";
    WEngine::CoreSystems::GetAssetRepo()->GetAsset(vertexShaderCode);
    WEngine::CoreSystems::GetAssetRepo()->GetAsset(fragmentShaderCode);

    std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages{};

    shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages[0].pName = "main";
    shaderStages[0].module = CompileShader(vertexShaderCode);
    shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[1].pName = "main";
    shaderStages[1].module = CompileShader(fragmentShaderCode);

    VkGraphicsPipelineCreateInfo pipeInfo{};
    pipeInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeInfo.pInputAssemblyState = &inputAssemblyInfo;
    pipeInfo.pVertexInputState = &vertexInputInfo;
    pipeInfo.pRasterizationState = &rasterInfo;
    pipeInfo.pColorBlendState = &blendInfo;
    pipeInfo.pViewportState = &viewInfo;
    pipeInfo.pDepthStencilState = &depthStencilInfo;
    pipeInfo.pMultisampleState = &multisampleInfo;
    pipeInfo.pDynamicState = &dynamicInfo;
    pipeInfo.stageCount = shaderStages.size();
    pipeInfo.pStages = shaderStages.data();
    pipeInfo.renderPass = renderPass;
    pipeInfo.layout = pipelineLayout;

    auto res = vkCreateGraphicsPipelines(vcore.gpuDevice, VK_NULL_HANDLE, 1, &pipeInfo, vcore.allocator, &pipeline);

    if (!ParseVkResult(res))
    {
        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog("Failed to create graphics pipeline.");
    }

    vkDestroyShaderModule(vcore.gpuDevice, shaderStages[0].module, vcore.allocator);
    vkDestroyShaderModule(vcore.gpuDevice, shaderStages[1].module, vcore.allocator);

    wFree(vertexShaderCode.shaderCode);
    wFree(fragmentShaderCode.shaderCode);

    return pipeline;
}


// ------------------------------------------------ [HELPER FUNCTIONS] ------------------------------------------------

uint64 GetSizeOfImageInBytes(WEngine::Vector2 imageSize, uint8 channelCount)
{
    return (uint64)imageSize.x * (uint64)imageSize.y * channelCount;
}

// --------------------------------------------------------------------------------------------------------------------
// ------------------------------------------ [GPU INTERFACE IMPLEMENTATION] ------------------------------------------
// --------------------------------------------------------------------------------------------------------------------

bool Iris::SETTING_InitGPUApi(SDL_Window *window)
{
    if (!SetupVkInstance())
        return false;

#ifndef PACKAGE
    SetupValidation();
#endif

    if (!SetupGraphicsDevice())
        return false;

    SetupVmaAllocator();

    if (!SDL_Vulkan_CreateSurface(window, vcore.instance, vcore.allocator, &screen.screen))
    {
        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog(std::format("Failed to create a Vulkan surface for the screen, {}", SDL_GetError()));
        return false;
    }

    if (!SetupSwapchain())
        return false;

    if (!SetupPipelineLayout())
        return false;

    if (!SetupDescriptorPool())
        return false;

    cmdBufs.resize(screen.swapchainImages.size());
    testPool = CreateCommandPool();
    for (uint32 i = 0; i < screen.swapchainImages.size(); i++)
        cmdBufs[i] = CreateCommandBuffer(testPool);
    testPass = CreateBasicRenderPass();
    testPipeline = CreateBasicPipeline(testPass);

    SetupSwapchainFramebuffers();

    return true;
}

void Iris::SETTING_ConfigureImGui(SDL_Window *window)
{
    ImGui_ImplSDL3_InitForVulkan(window);
    ImGui_ImplVulkan_InitInfo initInfo{};
    initInfo.ApiVersion = VK_API_VERSION_1_4;
    initInfo.Instance = vcore.instance;
    initInfo.PhysicalDevice = vcore.gpuPhysicalDevice;
    initInfo.Device = vcore.gpuDevice;
    initInfo.Allocator = vcore.allocator;
    initInfo.QueueFamily = primaryDrawQueueFamilyIndex;
    initInfo.Queue = primaryDrawQueue;
    initInfo.ImageCount = screen.swapchainImages.size();
    initInfo.MinImageCount = screen.swapchainImages.size();
    initInfo.DescriptorPoolSize = 8;
    initInfo.PipelineInfoMain.RenderPass = testPass;
    initInfo.PipelineInfoMain.Subpass = 0;
    initInfo.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    ImGui_ImplVulkan_Init(&initInfo);
}


void Iris::SETTING_BeginNewFrame()
{
    vkWaitForFences(vcore.gpuDevice, 1, &screen.endOfFrameFences[screen.currentFrame], VK_TRUE, UINT64_MAX);
    vkResetFences(vcore.gpuDevice, 1, &screen.endOfFrameFences[screen.currentFrame]);
    vkAcquireNextImageKHR(vcore.gpuDevice, screen.swapchain, max_uint64, screen.imageAvailableSems[screen.currentFrame],
        VK_NULL_HANDLE, &screen.swapchainCurrentImage);

    // Testing cmd buffer

    vkResetCommandBuffer(cmdBufs[screen.currentFrame], 0);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(cmdBufs[screen.currentFrame], &beginInfo);
}

void Iris::SETTING_SetViewportSize(WEngine::Vector2 size)
{
    VkViewport viewport{};
    viewport.width = size.x;
    viewport.height = size.y;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(cmdBufs[screen.currentFrame], 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.extent.width = size.x;
    scissor.extent.height = size.y;
    vkCmdSetScissor(cmdBufs[screen.currentFrame], 0, 1, &scissor);
}

WEngine::Nullable<WEngine::Shader> Iris::GetShader(const std::string &shaderName)
{
    if (loadedShadersHandles.contains(shaderName))
        return WEngine::Nullable<WEngine::Shader>(loadedShadersHandles[shaderName]);
    return WEngine::Nullable<WEngine::Shader>();
}

WEngine::Nullable<WEngine::Shader> Iris::ALLOC_CompileShader(const std::string& shaderName)
{
    auto check = GetShader(shaderName);

    if (check.HasValue())
    {
        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog(std::format("Shader already {} compiled", shaderName));
        return WEngine::Nullable<WEngine::Shader>();
    }

    return WEngine::Nullable<WEngine::Shader>();
}

WEngine::Nullable<WEngine::Model> Iris::ALLOC_CreateModel(const WEngine::ModelInfo &model)
{
    Vulkan_Model vkModel{};
    vkModel.vertexCount = model.vertices.size();
    VkDeviceSize vertBufferSize = model.vertices.size() * sizeof(WEngine::VertexData);

    VkBufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = vertBufferSize;
    bufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

    VmaAllocationCreateInfo allocationCreateInfo{};
    allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
    allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocationCreateInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    VmaAllocationInfo bufferAllocInfo{};

    auto res = vmaCreateBuffer(vcore.vmaAllocator, &bufferCreateInfo, &allocationCreateInfo,
        &vkModel.vertexBuffer, &vkModel.vertexAllocation, &bufferAllocInfo);

    if (!ParseVkResult(res))
    {
        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog("Failed to allocate vertex buffer");
        return WEngine::Nullable<WEngine::Model>();
    }

    if (bufferAllocInfo.pMappedData)
    {
        memcpy(bufferAllocInfo.pMappedData, model.vertices.data(), vertBufferSize);
    }
    else
    {
        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog("Failed to allocate vertex buffer");
        return WEngine::Nullable<WEngine::Model>();
    }

    loadedModels.push_back(vkModel);
    return WEngine::Nullable<WEngine::Model>(loadedModels.size());
}


void Iris::DRAWCALL_ClearFrame(WEngine::Color clearColor)
{
    VkClearValue clearCol{};
    WEngine::Colorf col(clearColor);
    clearCol = { col.red, col.green, col.blue, col.alpha };


    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = testPass;
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearCol;
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = {(uint32)EngineSettings::resolution.x, (uint32)EngineSettings::resolution.y};
    renderPassInfo.framebuffer = screen.swapchainFramebuffers[screen.swapchainCurrentImage];

    vkCmdBeginRenderPass(cmdBufs[screen.currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void Iris::DRAWCALL_DrawModel(WEngine::Model model, WEngine::Shader shader, const WEngine::ShaderSettings &settings)
{
    vkCmdBindPipeline(cmdBufs[screen.currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, testPipeline);

    Vulkan_Model vkModel = loadedModels[model - 1];
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(cmdBufs[screen.currentFrame], 0, 1, &vkModel.vertexBuffer, &offset);

    vkCmdDraw(cmdBufs[screen.currentFrame], 3, 1, 0, 0);

}

void Iris::DRAWCALL_ResetImGui()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
}

void Iris::DRAWCALL_DrawImGui()
{
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmdBufs[screen.currentFrame]);
}

void Iris::DRAWCALL_SwapBuffers(SDL_Window *window)
{
    vkCmdEndRenderPass(cmdBufs[screen.currentFrame]);
    auto res = vkEndCommandBuffer(cmdBufs[screen.currentFrame]);
    if (!ParseVkResult(res))
        WEngine::WLog::ConsoleLog("Something went wrong after ending the command buffer!");

    // just for now
    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &screen.imageAvailableSems[screen.currentFrame];
    submitInfo.pWaitDstStageMask = &waitStage;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &screen.renderFinishedSems[screen.currentFrame];
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmdBufs[screen.currentFrame];

    vkQueueSubmit(primaryDrawQueue, 1, &submitInfo, screen.endOfFrameFences[screen.currentFrame]);

    VkResult renderRes;
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &screen.renderFinishedSems[screen.currentFrame];
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &screen.swapchain;
    presentInfo.pImageIndices = &screen.swapchainCurrentImage;
    presentInfo.pResults = &renderRes;

    vkQueuePresentKHR(primaryDrawQueue, &presentInfo);

    screen.currentFrame = (screen.currentFrame + 1) % screen.swapchainImages.size();

    if (!ParseVkResult(renderRes))
        WEngine::WLog::ConsoleLog("Something went wrong during rendering!");
}

uint64 Iris::GetVramUsage()
{
    return 0;
}

uint32 Iris::GetDrawCallCountLastFrame()
{
    return 0;
}

WEngine::Nullable<ImTextureID> Iris::FramebufferToImGui(WEngine::Framebuffer framebuffer)
{
    return WEngine::Nullable<ImTextureID>();
}

#endif
