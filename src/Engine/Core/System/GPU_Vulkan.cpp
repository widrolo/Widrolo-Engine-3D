#include <Engine/EngineDefines.h>
#if GPU_BACKEND == GPU_VULKAN

#include "GPU.h"

void GPU::SETTING_ConfigureSDL()
{

}

bool GPU::SETTING_InitGPUApi(SDL_Window *window)
{
    return false;
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

#endif
