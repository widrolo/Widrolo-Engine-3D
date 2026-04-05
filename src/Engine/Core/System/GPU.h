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
    /**
     * This configures SDL to use the selected API.
     */
    static void SETTING_ConfigureSDL();

    /**
     * Initializes the selected API.
     * @param window The primary SDL window.
     * @return True is the initialization was successful, false if the initialization failed.
     */
    static bool SETTING_InitGPUApi(SDL_Window* window);

    /**
     * Configures ImGui to use the selected API.
     * @param window The primary SDL window.
     */
    static void SETTING_ConfigureImGui(SDL_Window* window);

    /**
     * Sets the size of the Viewport.
     * @param topLeft Top left corner of the Viewport.
     * @param bottomRight Bottom right corner of the Viewport.
     */
    static void SETTING_SetViewport(WEngine::Vector2 topLeft, WEngine::Vector2 bottomRight);

    /**
     * Toggles blending for sprites based on the alpha channel.
     * @param enabled The state to be set.
     */
    static void SETTING_ToggleBlending(bool enabled);

    /**
     * Toggles depth testing.
     * @param enabled The state to be set.
     * @warning Don't set this to true when using 2D rendering.
     */
    static void SETTING_ToggleDepthTest(bool enabled);

    /**
     * Enables wireframe rendering.
     * @param enabled The state to be set.
     * @note When turning this on, don't forget to also turn off blending. Some lines are invisible otherwise.
     */
    static void SETTING_ToggleWireFrame(bool enabled);

    // ----------------------- Shaders -----------------------
    /**
     * This retrieves and compiles a shader.
     * @param name Name of the shader to be compiled.
     * @return A nullable handle to the newly created shader.
     * @note This function may allocate video memory. In this case, the allocated memory is not tracked.
     * @note Shaders are meant to be located in Data/Shaders. The strictly enforced naming convention is xxxVertex.glsl and
     * xxxFragment.glsl. If either are not found, then the compilation fails.
     */
    static WEngine::Nullable<WEngine::Shader> ALLOC_CompileShader(const std::string& name);

    /**
     * Finds the shader in the repository and returns it.
     * @param name Name of the shader to be retrieved.
     * @return A nullable handle to the shader.
     * @note This function does not try to compile a shader that was not found in the repository.
     */
    static WEngine::Nullable<WEngine::Shader> GetShader(const std::string& name);


    // -------------------- Framebuffers ---------------------
    /**
     * Creates a new Framebuffer.
     * @param resolution The resolution of the framebuffer in pixels.
     * @return A nullable handle to the newly created framebuffer.
     * @note This function may allocate video memory.
     */
    static WEngine::Nullable<WEngine::Framebuffer> ALLOC_CreateFramebuffer(const WEngine::Vector2& resolution);

    /**
     * Sets a framebuffer to be a render target.
     * @param framebuffer The framebuffer to become the rendering target.
     */
    static void SETTING_SelectFramebuffer(WEngine::Framebuffer framebuffer);

    /**
     * Sets the screen to be a render target.
     */
    static void SETTING_SelectFramebufferScreen();

    /**
     * Gets the associated texture of a framebuffer.
     * @param framebuffer Framebuffer of the texture.
     * @return A nullable handle to the texture.
     */
    static WEngine::Nullable<WEngine::Texture> GetFramebufferTexture(WEngine::Framebuffer framebuffer);


    // ----------------------- Models ------------------------
    /**
     * Copies a model from memory to video memory.
     * @param model Model to be copied into video memory.
     * @return A nullable handle to the newly created model.
     * @note This function may allocate video memory.
     */
    static WEngine::Nullable<WEngine::Model> ALLOC_CreateModel(const WEngine::ModelInfo& model);

    /**
     * Creates a line object.
     * @return A nullable handle to the newly created line.
     * @note This function may allocate video memory.
     * @note Due to the nature of lines, further information is passed in during the draw call. Therefore, consider only
     * creating one line.
     */
    static WEngine::Nullable<WEngine::Line> ALLOC_CreateLine();


    // ----------------------- Textures ----------------------
    /**
     * Copies a texture from memory to video memory.
     * @param textureData Texture to be copied into video memory.
     * @return A nullable handle to the newly created texture.
     * @note This function may allocate video memory.
     */
    static WEngine::Nullable<WEngine::Texture> ALLOC_CreateTexture(const WEngine::TextureInfo& textureData);

    /**
     * Finds a texture in the repository and returns it.
     * @param name Name of the texture to be retrieved.
     * @return A nullable handle to the texture.
     * @note This function does not try to load a new texture that was not found in the repository.
     */
    static WEngine::Nullable<WEngine::Texture> GetTexture(const std::string& name);

    /**
     * Queries the texture for its size.
     * @param texture Texture to queried.
     * @return A nullable size of the texture.
     */
    static WEngine::Nullable<WEngine::Vector2> GetTextureSize(WEngine::Texture texture);

    // ----------------------- Drawing -----------------------
    /**
     * Clears the screen by flooding it with a color.
     * @param clearColor The color which will be used to clear the screen.
     * @note This function invokes a drawcall.
     */
    static void DRAWCALL_ClearScreen(WEngine::Color clearColor);

    /**
     * Draws a model.
     * @param model Madel to be rendered.
     * @param shader Shader to be used for rendering the model.
     * @param settings Settings in the shader.
     * @note This function invokes a drawcall.
     */
    static void DRAWCALL_DrawModel(WEngine::Model model, WEngine::Shader shader, const WEngine::ShaderSettings& settings);

    /**
     * Draws a line.
     * @param line Line to be rendered.
     * @param shader Shader to be used for rendering the model.
     * @param settings Settings in the shader.
     * @param newModel The positions of the line.
     * @note This function invokes a drawcall.
     */
    static void DRAWCALL_DrawLine(WEngine::Line line, WEngine::Shader shader,
                                  const WEngine::ShaderSettings& settings, const WEngine::LineInfo& newModel);

    /**
     * Starts a new frame in ImGui.
     */
    static void DRAWCALL_ResetImGui();

    /**
     * Renders all ImGui widgets.
     */
    static void DRAWCALL_DrawImGui();

    /**
     * Swaps the buffer to present the rendered frame.
     * @param window Window to present the rendered frame to.
     */
    static void DRAWCALL_SwapBuffers(SDL_Window* window);

    // ------------------------ Memory -----------------------
    /**
     * Gets the VRAM usage.
     * @return Video memory usage in bytes.
     * @note This may not be 100% accurate.
     */
    static uint64 GetVramUsage();

    /**
     * Gets the draw call count during the last frame.
     * @return The amount of draw calls in the last frame.
     * @note This may not be accurate, especially when using ImGui Widgets.
     */
    static uint32 GetDrawCallCountLastFrame();

    /**
     * Converts a framebuffer to an ImGui compatible texture.
     * @param framebuffer The framebuffer to converted to an ImGui compatible texture.
     * @return A nullable ImGui compatible texture.
     */
    static WEngine::Nullable<ImTextureID> FramebufferToImGui(WEngine::Framebuffer framebuffer);
};
