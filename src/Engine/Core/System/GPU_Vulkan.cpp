#include <Engine/EngineDefines.h>
#if GPU_BACKEND == GPU_VULKAN

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

enum class BufferType
{
    Uniform,
    Storage,
    Vertex
};

enum class ImageBufferType
{
    Storage,
    Sampled,
    Input
};

struct Vulkan_Shader
{
    VkShaderModule vertexShader;
    VkShaderModule fragmentShader;
    VkShaderModule geometryShader;

    bool vertexPresent : 1;
    bool fragmentPresent : 1;
    bool geometryPresent : 1;
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

VkDebugUtilsMessengerEXT validationMessenger = VK_NULL_HANDLE;
VkAllocationCallbacks* allocator = nullptr;
VkAllocationCallbacks allocatorInternal;
VmaAllocator vmaAllocator;

VkInstance instance = VK_NULL_HANDLE;
VkPhysicalDevice gpuPhysicalDevice = VK_NULL_HANDLE;
VkDevice gpuDevice = VK_NULL_HANDLE;

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

uint32 queueFamilyCount = 0;
wtl::vector<QueueFamily> queueFamilies;
uint32 primaryDrawQueueFamilyIndex = 0;
VkQueue primaryDrawQueue = VK_NULL_HANDLE;

VkPipelineLayout pipelineLayout;

// These are just for prototyping

VkCommandPool testPool;
wtl::vector<VkCommandBuffer> testBufs;
VkRenderPass testPass;
VkPipeline testPipeline;

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
        WEngine::WLog::ConsoleLog(std::format("{} Non-optimal use of Vulkan!\"{}\"\n{}", warnStart, pCallbackData->pMessageIdName, pCallbackData->pMessage));


    return VK_FALSE;
}

// --------------------------------------------------- [API SETUP] ----------------------------------------------------

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

void SetupVmaAllocator()
{
    VmaVulkanFunctions vulkanFunctions = {};
    vulkanFunctions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
    vulkanFunctions.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;

    VmaAllocatorCreateInfo allocatorCreateInfo = {};
    allocatorCreateInfo.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
    allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_2;
    allocatorCreateInfo.physicalDevice = gpuPhysicalDevice;
    allocatorCreateInfo.device = gpuDevice;
    allocatorCreateInfo.instance = instance;
    allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;


    vmaCreateAllocator(&allocatorCreateInfo, &vmaAllocator);
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

void SetupSwapchainFramebuffers()
{
    swapchainImageViews.resize(swapchainImages.size());
    swapchainFramebuffers.resize(swapchainImages.size());


    for (int i = 0; i < swapchainImages.size(); i++)
    {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = swapchainImages[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = VK_FORMAT_B8G8R8A8_SRGB;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        vkCreateImageView(gpuDevice, &viewInfo, allocator, &swapchainImageViews[i]);

        VkFramebufferCreateInfo fbInfo{};
        fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fbInfo.renderPass = testPass;
        fbInfo.attachmentCount = 1;
        fbInfo.pAttachments = &swapchainImageViews[i];
        fbInfo.width = EngineSettings::resolution.x;
        fbInfo.height = EngineSettings::resolution.y;
        fbInfo.layers = 1;

        vkCreateFramebuffer(gpuDevice, &fbInfo, allocator, &swapchainFramebuffers[i]);
    }
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

    uint32 swapchainImageCount;
    vkGetSwapchainImagesKHR(gpuDevice, swapchain, &swapchainImageCount, nullptr);
    swapchainImages.resize(swapchainImageCount);
    vkGetSwapchainImagesKHR(gpuDevice, swapchain, &swapchainImageCount, swapchainImages.data());

    imageAvailableSems.resize(swapchainImageCount);
    renderFinishedSems.resize(swapchainImageCount);
    endOfFrameFences.resize(swapchainImageCount);


    VkSemaphoreCreateInfo semInfo{};
    semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    for (uint32 i = 0; i < swapchainImageCount; i++)
        vkCreateSemaphore(gpuDevice, &semInfo, allocator, &imageAvailableSems[i]);
    for (uint32 i = 0; i < swapchainImageCount; i++)
        vkCreateSemaphore(gpuDevice, &semInfo, allocator, &renderFinishedSems[i]);

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    for (uint32 i = 0; i < swapchainImageCount; i++)
        vkCreateFence(gpuDevice, &fenceInfo, allocator, &endOfFrameFences[i]);


    return ParseVkResult(res);
}

bool SetupPipelineLayout()
{
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    auto resLayout = vkCreatePipelineLayout(gpuDevice, &pipelineLayoutInfo, allocator, &pipelineLayout);
    if (!ParseVkResult(resLayout))
    {
        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog("Unable to create pipeline layout!");
        return false;
    }
    return true;
}

// ------------------------------------------------- [MEM FUNCTIONS] --------------------------------------------------

// This function does not always allocate the exact amount of video memory as passed in, as Vulkan
// imposes memory size requirements. The memory size may at any time be bigger than the value passed,
// but never smaller.
VkDeviceMemory AllocateVideoMemory(const uint64 minSize, VkBuffer buffer)
{
    VkMemoryRequirements req;
    vkGetBufferMemoryRequirements(gpuDevice, buffer, &req);

    uint64 size = minSize;
    if (minSize < req.size)
        size = req.size;

    VkDeviceMemory memory;
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = size;
    allocInfo.memoryTypeIndex = 0;
    auto memRes = vkAllocateMemory(gpuDevice, &allocInfo, allocator, &memory);

    if (!ParseVkResult(memRes))
    {
        WEngine::WLog::SetConsoleWarning();
        WEngine::WLog::ConsoleLog("Unable to allocate memory buffer on GPU!");
        return VK_NULL_HANDLE;
    }
    return memory;
}
VkDeviceMemory AllocateVideoMemory(const uint64 minSize, VkImage image)
{
    VkMemoryRequirements req;
    vkGetImageMemoryRequirements(gpuDevice, image, &req);

    uint64 size = minSize;
    if (minSize < req.size)
        size = req.size;

    VkDeviceMemory memory;
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = size;
    allocInfo.memoryTypeIndex = 0;
    auto memRes = vkAllocateMemory(gpuDevice, &allocInfo, allocator, &memory);

    if (!ParseVkResult(memRes))
    {
        WEngine::WLog::SetConsoleWarning();
        WEngine::WLog::ConsoleLog("Unable to allocate memory buffer on GPU!");
        return VK_NULL_HANDLE;
    }
    return memory;
}

VkBuffer CreateBuffer(const uint64 size, BufferType bufferType)
{
    VkBuffer buffer;

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    switch (bufferType)
    {
        case BufferType::Uniform:
            bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            break;
        case BufferType::Storage:
            bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
            break;
        case BufferType::Vertex:
            bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            break;
    }
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    auto bufRes = vkCreateBuffer(gpuDevice, &bufferInfo, allocator, &buffer);

    if (!ParseVkResult(bufRes))
    {
        WEngine::WLog::SetConsoleWarning();
        WEngine::WLog::ConsoleLog("Unable to create buffer on GPU!");
        return VK_NULL_HANDLE;
    }

    return buffer;
}

VkImage CreateImage(ImageBufferType imageType, WEngine::Vector2 imageSize)
{
    VkImage image;

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D; // what kinda question is that???
    imageInfo.extent = { (uint32)imageSize.x, (uint32)imageSize.y, 1 };
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    switch (imageType)
    {
        case ImageBufferType::Storage:
            imageInfo.usage = VK_IMAGE_USAGE_STORAGE_BIT;
            imageInfo.format = VK_FORMAT_R8G8B8A8_UINT; // and so god decided it all to be uint from now on!
            break;
        case ImageBufferType::Sampled:
            imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
            imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
            break;
        case ImageBufferType::Input:
            imageInfo.usage = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
            imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
            break;
    }

    auto res = vkCreateImage(gpuDevice, &imageInfo, allocator, &image);

    if (!ParseVkResult(res))
    {
        WEngine::WLog::SetConsoleWarning();
        WEngine::WLog::ConsoleLog("Unable to create image buffer!");
        return VK_NULL_HANDLE;
    }

    return image;
}

VkImageView CreateImageView(VkImage image, ImageBufferType imageType)
{
    VkImageView imageView;

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    if (imageType == ImageBufferType::Storage)
        viewInfo.format = VK_FORMAT_R8G8B8A8_UINT;
    else
        viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;


    viewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
    viewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
    viewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
    viewInfo.components.a = VK_COMPONENT_SWIZZLE_A;

    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    auto res = vkCreateImageView(gpuDevice, &viewInfo, allocator, &imageView);

    if (!ParseVkResult(res))
    {
        WEngine::WLog::SetConsoleWarning();
        WEngine::WLog::ConsoleLog("Unable to create image view!");
        return VK_NULL_HANDLE;
    }

    return imageView;
}

VkCommandPool CreateCommandPool()
{
    VkCommandPool commandPool;

    VkCommandPoolCreateInfo commandPoolInfo{};
    commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolInfo.queueFamilyIndex = primaryDrawQueueFamilyIndex;
    commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    auto res = vkCreateCommandPool(gpuDevice, &commandPoolInfo, allocator, &commandPool);

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

    auto res = vkAllocateCommandBuffers(gpuDevice, &allocInfo, &cmdBuff);

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
    colorAttachment.format = VK_FORMAT_B8G8R8A8_SRGB;
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
    auto res = vkCreateRenderPass(gpuDevice, &renderPassInfo, allocator, &renderPass);
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

    auto res = vkCreateShaderModule(gpuDevice, &shaderModuleInfo, allocator, &module);
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

    // [0] = Position (Vector3)     |     [1] = UV (Vector2)
    std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT; // Khronos had a meth party while making this one
    attributeDescriptions[0].offset = offsetof(WEngine::VertexData, position);
    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT; // Khronos had a meth party while making this one
    attributeDescriptions[1].offset = offsetof(WEngine::VertexData, uvCoord);

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

    auto res = vkCreateGraphicsPipelines(gpuDevice, VK_NULL_HANDLE, 1, &pipeInfo, allocator, &pipeline);

    if (!ParseVkResult(res))
    {
        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog("Failed to create graphics pipeline.");
    }

    vkDestroyShaderModule(gpuDevice, shaderStages[0].module, allocator);
    vkDestroyShaderModule(gpuDevice, shaderStages[1].module, allocator);

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

bool GPU::SETTING_InitGPUApi(SDL_Window *window)
{
    if (!SetupVkInstance())
        return false;

#ifndef PACKAGE
    SetupValidation();
#endif

    if (!SetupGraphicsDevice())
        return false;

    SetupVmaAllocator();

    if (!SDL_Vulkan_CreateSurface(window, instance, allocator, &screen))
    {
        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog(std::format("Failed to create a Vulkan surface for the screen, {}", SDL_GetError()));
        return false;
    }

    if (!SetupSwapchain())
        return false;

    if (!SetupPipelineLayout())
        return false;

    testBufs.resize(swapchainImages.size());
    testPool = CreateCommandPool();
    for (uint32 i = 0; i < swapchainImages.size(); i++)
        testBufs[i] = CreateCommandBuffer(testPool);
    testPass = CreateBasicRenderPass();
    testPipeline = CreateBasicPipeline(testPass);

    SetupSwapchainFramebuffers();

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
    initInfo.QueueFamily = primaryDrawQueueFamilyIndex;
    initInfo.Queue = primaryDrawQueue;

    //ImGui_ImplVulkan_Init(&initInfo);
}


void GPU::SETTING_BeginNewFrame()
{
    vkWaitForFences(gpuDevice, 1, &endOfFrameFences[currentFrame], VK_TRUE, UINT64_MAX);
    vkResetFences(gpuDevice, 1, &endOfFrameFences[currentFrame]);
    vkAcquireNextImageKHR(gpuDevice, swapchain, max_uint64, imageAvailableSems[currentFrame], VK_NULL_HANDLE, &swapchainCurrentImage);

    // Testing cmd buffer

    vkResetCommandBuffer(testBufs[currentFrame], 0);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(testBufs[currentFrame], &beginInfo);
}

void GPU::SETTING_SetViewportSize(WEngine::Vector2 size)
{
    VkViewport viewport{};
    viewport.width = size.x;
    viewport.height = size.y;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(testBufs[currentFrame], 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.extent.width = size.x;
    scissor.extent.height = size.y;
    vkCmdSetScissor(testBufs[currentFrame], 0, 1, &scissor);
}

WEngine::Nullable<WEngine::Shader> GPU::GetShader(const std::string &shaderName)
{
    if (loadedShadersHandles.contains(shaderName))
        return WEngine::Nullable<WEngine::Shader>(loadedShadersHandles[shaderName]);
    return WEngine::Nullable<WEngine::Shader>();
}

WEngine::Nullable<WEngine::Shader> GPU::ALLOC_CompileShader(const std::string& shaderName)
{
    auto check = GetShader(shaderName);

    if (check.HasValue())
    {
        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog(std::format("Shader already {} compiled", shaderName));
        return check;
    }
}

WEngine::Nullable<WEngine::Model> GPU::ALLOC_CreateModel(const WEngine::ModelInfo &model)
{
    Vulkan_Model vkModel{};
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

    auto res = vmaCreateBuffer(vmaAllocator, &bufferCreateInfo, &allocationCreateInfo,
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


void GPU::DRAWCALL_ClearFrame(WEngine::Color clearColor)
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
    renderPassInfo.framebuffer = swapchainFramebuffers[swapchainCurrentImage];

    vkCmdBeginRenderPass(testBufs[currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void GPU::DRAWCALL_DrawModel(WEngine::Model model, WEngine::Shader shader, const WEngine::ShaderSettings &settings)
{
    vkCmdBindPipeline(testBufs[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, testPipeline);

    Vulkan_Model vkModel = loadedModels[model - 1];
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(testBufs[currentFrame], 0, 1, &vkModel.vertexBuffer, &offset);

    vkCmdDraw(testBufs[currentFrame], 3, 1, 0, 0);

}

void GPU::DRAWCALL_ResetImGui()
{
    ImGui_ImplVulkan_NewFrame();
}

void GPU::DRAWCALL_DrawImGui()
{

}

void GPU::DRAWCALL_SwapBuffers(SDL_Window *window)
{
    vkCmdEndRenderPass(testBufs[currentFrame]);
    auto res = vkEndCommandBuffer(testBufs[currentFrame]);
    if (!ParseVkResult(res))
        WEngine::WLog::ConsoleLog("Something went wrong after ending the command buffer!");

    // just for now
    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &imageAvailableSems[currentFrame];
    submitInfo.pWaitDstStageMask = &waitStage;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &renderFinishedSems[currentFrame];
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &testBufs[currentFrame];

    vkQueueSubmit(primaryDrawQueue, 1, &submitInfo, endOfFrameFences[currentFrame]);

    VkResult renderRes;
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &renderFinishedSems[currentFrame];
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain;
    presentInfo.pImageIndices = &swapchainCurrentImage;
    presentInfo.pResults = &renderRes;

    vkQueuePresentKHR(primaryDrawQueue, &presentInfo);

    currentFrame = (currentFrame + 1) % swapchainImages.size();

    if (!ParseVkResult(renderRes))
        WEngine::WLog::ConsoleLog("Something went wrong during rendering!");
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
