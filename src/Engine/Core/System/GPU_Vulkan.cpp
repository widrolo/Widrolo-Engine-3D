#include <Engine/EngineDefines.h>
#if GPU_BACKEND == GPU_VULKAN

#include "GPU.h"

#include <Engine/EngineDefines.h>
#include <Game/GameDefines.h>

#include <vulkan/vulkan.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include "Engine/Util/Log.h"


// --------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------- [GPU API TYPES] -------------------------------------------------
// --------------------------------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------ [GPU API VARIABLES] -----------------------------------------------
// --------------------------------------------------------------------------------------------------------------------

VkInstance instance = VK_NULL_HANDLE;
VkSurfaceKHR screen = VK_NULL_HANDLE;
VkDebugUtilsMessengerEXT validationMessenger;

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

VkBool32 ValidationCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT             messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void*                                       pUserData)
{
    if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        WEngine::WLog::SetConsoleWarning();
    else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        WEngine::WLog::SetConsoleError();

    WEngine::WLog::ConsoleLog(std::format("[GPU ERROR] Validation layer tripped!\n{}", pCallbackData->pMessage));

    return VK_FALSE;
}

bool SetupValidation()
{
    PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT =
        reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
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
    info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    info.pfnUserCallback = ValidationCallback;
    info.pUserData = nullptr;

    auto res = vkCreateDebugUtilsMessengerEXT(instance, &info, nullptr, &validationMessenger);

    return ParseVkResult(res);
}

void GPU::SETTING_ConfigureSDL()
{

}

bool GPU::SETTING_InitGPUApi(SDL_Window *window)
{
    Uint32 count_instance_extensions;
    const char * const *instance_extensions = SDL_Vulkan_GetInstanceExtensions(&count_instance_extensions);

    if (instance_extensions == nullptr)
        return false;

    int count_extensions = count_instance_extensions + 1;
    const char **extensions = (const char **)SDL_malloc(count_extensions * sizeof(const char *));
    extensions[0] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
    SDL_memcpy(&extensions[1], instance_extensions, count_instance_extensions * sizeof(const char*));

    const char *validationLayers[] = { "VK_LAYER_KHRONOS_validation" };

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = GameSettings::gameName.c_str();
    appInfo.pEngineName = EngineSettings::engineName.c_str();
    appInfo.apiVersion = VK_API_VERSION_1_4;

    VkInstanceCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    info.pApplicationInfo = &appInfo;
    info.enabledExtensionCount = count_extensions;
    info.ppEnabledExtensionNames = extensions;
    info.enabledLayerCount = 1;
    info.ppEnabledLayerNames = validationLayers;

    auto resIsnt = vkCreateInstance(&info, nullptr, &instance);

    SDL_free(extensions);

    if (!ParseVkResult(resIsnt))
        return false;

#ifndef PACKAGE
    SetupValidation();
#endif

    if (!SDL_Vulkan_CreateSurface(window, instance, nullptr, &screen))
    {
        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog(std::format("Failed to create a Vulkan surface for the screen, {}", SDL_GetError()));
        return false;
    }

    return true;
}

void GPU::SETTING_ConfigureImGui(SDL_Window *window)
{

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

}

uint64 GPU::GetVramUsage()
{

}

uint32 GPU::GetDrawCallCountLastFrame()
{

}

WEngine::Nullable<ImTextureID> GPU::FramebufferToImGui(WEngine::Framebuffer framebuffer)
{

}

#endif
