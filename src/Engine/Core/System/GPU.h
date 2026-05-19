#pragma once

// This file does not contain LLM generated documentation

#include <../../Types/Rendering/GPU/Shader.h>
#include <Engine/Types/Nullable.h>
#include <SDL3/SDL.h>

#include "Engine/Types/Rendering/Color.h"
#include "../../Types/Rendering/GPU/Framebuffer.h"
#include "Engine/imgui/imgui.h"
#include "Engine/Types/Rendering/LineInfo.h"
#include "Engine/Types/Rendering/GPU/Line.h"
#include "Engine/Types/Rendering/ModelInfo.h"
#include "Engine/Types/Rendering/ShaderSettings.h"
#include "Engine/Types/Rendering/TextureInfo.h"
#include "Engine/Types/Rendering/GPU/Model.h"

/**
 * This class is the primary interface to talk to the GPU.
 * The different APIs implement it themselves, and are selected in the engine defines.
 * @note All types used are strictly API independent.
 * @see EngineDefines.h
 */
class GPU
{
public:
    static bool SETTING_InitGPUApi(SDL_Window* window);
    static void SETTING_ConfigureImGui(SDL_Window* window);
    static void SETTING_BeginNewFrame();
    static void SETTING_SetViewportSize(WEngine::Vector2 size);

    static WEngine::Nullable<WEngine::Shader> GetShader(const std::string& shaderName);

    // ----------------------- Shaders -----------------------

    static WEngine::Nullable<WEngine::Shader> ALLOC_CompileShader(const std::string& shaderName);

    // ----------------------- Models ------------------------

    static WEngine::Nullable<WEngine::Model> ALLOC_CreateModel(const WEngine::ModelInfo& model);

    // ----------------------- Drawing -----------------------

    static void DRAWCALL_ClearFrame(WEngine::Color clearColor);

    static void DRAWCALL_DrawModel(WEngine::Model model, WEngine::Shader shader, const WEngine::ShaderSettings& settings);

    static void DRAWCALL_ResetImGui();
    static void DRAWCALL_DrawImGui();

    static void DRAWCALL_SwapBuffers(SDL_Window* window);

    // ------------------------ Memory -----------------------

    static uint64 GetVramUsage();
    static uint32 GetDrawCallCountLastFrame();
    static WEngine::Nullable<ImTextureID> FramebufferToImGui(WEngine::Framebuffer framebuffer);
};
