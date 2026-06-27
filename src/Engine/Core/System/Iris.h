#pragma once

// This file does not contain LLM generated documentation

#include <../../Types/Rendering/GPU/Shader.h>
#include <Engine/Types/Nullable.h>
#include <SDL3/SDL.h>

#include "Engine/Types/Rendering/Color.h"
#include "../../Types/Rendering/GPU/Framebuffer.h"
#include "Engine/imgui/imgui.h"
#include "Engine/Types/Rendering/InstanceData.h"
#include "Engine/Types/Rendering/LightingInfo.h"
#include "Engine/Types/Rendering/LineInfo.h"
#include "Engine/Types/Rendering/GPU/Line.h"
#include "Engine/Types/Rendering/ModelInfo.h"
#include "Engine/Types/Rendering/ShaderDefinition.h"
#include "Engine/Types/Rendering/ShaderSettings.h"
#include "Engine/Types/Rendering/TextureInfo.h"
#include "Engine/Types/Rendering/GPU/Material.h"
#include "Engine/Types/Rendering/GPU/Model.h"
#include "Engine/Types/Rendering/Iris/InstThreadedList.h"
#include "Engine/Types/Rendering/Iris/IrisAssetComms.h"

/**
 * This class is the primary interface to talk to the GPU.
 * The different APIs implement it themselves, and are selected in the engine defines.
 * @note All types used are strictly API independent.
 * @see EngineDefines.h
 */
class Iris
{
public:
    static bool SETTING_InitGPUApi(SDL_Window* window);
    static void SETTING_BeginNewPreFrame();
    static void SETTING_BeginNewFrame();
    static void SETTING_SetViewportSize(WEngine::Vector2 size);
    static void SETTING_SetLighting(const WEngine::LightingInfo& light);


    // ----------------------- Shaders -----------------------

    static WEngine::Nullable<WEngine::ShaderDefinition> GetShaderDef(const std::string& shaderName);
    static WEngine::Nullable<WEngine::Material> GetMaterial(const std::string &matName);
    static WEngine::Nullable<WEngine::Shader> GetShader(const std::string &shaderName);
    static WEngine::Nullable<WEngine::Shader> GetShader(WEngine::Material matQuery);
    static WEngine::Nullable<WEngine::Material> ALLOC_CompileMaterial(const std::string& matName);

    // ----------------------- Models ------------------------

    static WEngine::Nullable<WEngine::Model> GetModel(const std::string& modelName);
    static WEngine::Nullable<WEngine::Model> ALLOC_CreateModel(const WEngine::ModelInfo& model);

    // ----------------------- Drawing -----------------------

    static void DRAWCALL_ClearFrame(WEngine::Color clearColor);

    static void DRAWCALL_DrawModel(WEngine::Model model, WEngine::Material material, const WEngine::Mat4x4& mvp);
    static void DRAWCALL_DrawModelInstanced(WEngine::Model model, WEngine::Material material,
        const WEngine::Mat4x4& vp, const wtl::vector<WEngine::InstanceData>& instanceMats);
    static void DRAWCALL_DrawModelInstancedStationary(WEngine::Model model, WEngine::Material material,
        const WEngine::Mat4x4& vp);


    static void DRAWCALL_DrawToDisplay(SDL_Window* window);

    // --------------------- Framebuffers --------------------

    static WEngine::Nullable<WEngine::Framebuffer> ALLOC_CreateFramebuffer(const WEngine::Vector2& resolution);

    static void SETTING_SelectFramebufferForRender(WEngine::Framebuffer framebuffer);
    static void SETTING_SelectFramebufferScreenForRender();
    static void SETTING_FinishFramebufferRender();

    // ------------------------ Memory -----------------------

    static uint64 GetVramUsage();
    static uint32 GetDrawCallCountLastFrame();
    static bool IsFirstFrame();
    static wtl::vector<MemListDebugInfo> GetStatInstBufAllocInfo();

    static void AddStationaryObjects(WEngine::Model model, WEngine::Material material,
        wtl::vector<WEngine::InstanceData> instanceMats);

    // ------------------------- ImGui -----------------------

    static void SETTING_ConfigureImGui(SDL_Window* window);
    static void DRAWCALL_ResetImGui();
    static void DRAWCALL_DrawImGui();
    static WEngine::Nullable<ImTextureID> FramebufferToImGui(WEngine::Framebuffer framebuffer);

    // --------------------- Asset Repo ----------------------

    static void AssetIrisCommunication(WEngine::AssetIrisCommunication& mission);
};
