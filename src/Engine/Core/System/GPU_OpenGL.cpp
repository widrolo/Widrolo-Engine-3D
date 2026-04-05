#include <Engine/EngineDefines.h>
#if GPU_BACKEND == GPU_OPENGL

#include "GPU.h"
#include <unordered_map>
#include <Engine/WTL/vector.h>
#include <Engine/Util/Log.h>
#include <Engine/Types/AssetMission.h>
#include <WGL.h>
#include <SDL3/SDL.h>

#include "Engine/Core/Handlers/AssetRepo.h"
#include "Engine/imgui/backends/imgui_impl_opengl3.h"
#include "Engine/imgui/backends/imgui_impl_sdl3.h"
#include "Engine/Types/CoreSystems.h"

// --------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------- [GPU API TYPES] -------------------------------------------------
// --------------------------------------------------------------------------------------------------------------------

struct OpenGL_Framebuffer
{
    GLuint framebuffer;
    WEngine::Texture framebufferTexture;
    WEngine::Vector2 size;
};

struct OpenGL_Model
{
    GLuint VBO;
    GLuint VAO;
    uint64 vertexCount;
    bool hasTexture;
};

struct OpenGL_Line
{
    GLuint VBO;
    GLuint VAO;
};

struct OpenGL_Texture
{
    GLuint texture;
    int32 width;
    int32 height;
};

// --------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------ [GPU API VARIABLES] -----------------------------------------------
// --------------------------------------------------------------------------------------------------------------------

SDL_GLContext openGLContext;

wtl::vector<GLuint> gpu_shaders;
std::unordered_map<std::string, WEngine::Shader> gpu_shadersHandles;

wtl::vector<OpenGL_Framebuffer> gpu_framebuffers;

wtl::vector<OpenGL_Model> gpu_models;
wtl::vector<OpenGL_Line> gpu_lines;

wtl::vector<OpenGL_Texture> gpu_textures;
std::unordered_map<std::string, WEngine::Shader> gpu_textureHandles;

static uint64 gpu_vramUsage = 0;
static uint32 gpu_drawCallsThisFrame = 0;
static uint32 gpu_drawCallsLastFrame = 0;

// --------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------ [GPU API HELPERS] -------------------------------------------------
// --------------------------------------------------------------------------------------------------------------------

uint64 CalcTextureSize(uint8 bytesPerPixel, uint32 width, uint32 height)
{
    return width * height * bytesPerPixel;
}

uint64 CalcModelSize(uint8 bytesPerVertex, uint32 vertexCount)
{
    return bytesPerVertex * vertexCount;
}

WEngine::Nullable<GLuint> CompileSingleShader(std::string source, GLenum type)
{
    GLuint shader = glCreateShader(type);

    const char* s = source.c_str();
    glShaderSource(shader, 1, &s, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        GLint logLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
        char* infoLog = (char*)wNew(logLength);
        glGetShaderInfoLog(shader, logLength, nullptr, infoLog);
        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog(std::format("Shader Compilation Failed:\n\t{}", std::string(infoLog)));
        wFree(infoLog);
        return WEngine::Nullable<GLuint>();
    }
    return shader;
}

GLuint LinkShaderProgramm(GLuint vert, GLuint frag)
{
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vert);
    glAttachShader(shaderProgram, frag);
    glLinkProgram(shaderProgram);

    GLint success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        GLint logLength;
        glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &logLength);
        char* infoLog = (char*)wNew(logLength);
        glGetProgramInfoLog(shaderProgram, logLength, nullptr, infoLog);

        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog(std::format("Shader Compilation Failed:\n\t{}", std::string(infoLog)));
        wFree(infoLog);
        glDeleteShader(vert);
        glDeleteShader(frag);
    }

    glDeleteShader(vert);
    glDeleteShader(frag);

    return shaderProgram;
}

void ApplyShaderSettings(GLuint glShader, const WEngine::ShaderSettings& settings)
{
    WEngine::Vector2 v2;
    WEngine::Vector4 v4;
    WEngine::Color c;
    WEngine::Mat4x4 m4;
    WEngine::Texture tx;
    for (const auto& setting : settings)
    {
        switch (setting.type)
        {
            case WEngine::ShaderSettingType::Float:
                glUniform1f(glGetUniformLocation(glShader, setting.settingName.c_str()), std::get<float32>(setting.option));
                break;
            case WEngine::ShaderSettingType::Bool:
                glUniform1i(glGetUniformLocation(glShader, setting.settingName.c_str()), std::get<bool>(setting.option));
                break;
            case WEngine::ShaderSettingType::UInt:
                glUniform1ui(glGetUniformLocation(glShader, setting.settingName.c_str()), std::get<uint32>(setting.option));
                break;
            case WEngine::ShaderSettingType::Int:
                glUniform1i(glGetUniformLocation(glShader, setting.settingName.c_str()), std::get<int32>(setting.option));
                break;
            case WEngine::ShaderSettingType::Vec2:
                v2 = std::get<WEngine::Vector2>(setting.option);
                glUniform2f(glGetUniformLocation(glShader, setting.settingName.c_str()), v2.x, v2.y);
                break;
            case WEngine::ShaderSettingType::Vec4:
                v4 = std::get<WEngine::Vector4>(setting.option);
                glUniform4f(glGetUniformLocation(glShader, setting.settingName.c_str()), v4.x, v4.y, v4.z, v4.w);
                break;
            case WEngine::ShaderSettingType::Color:
                c = std::get<WEngine::Color>(setting.option);
                glUniform4f(glGetUniformLocation(glShader, setting.settingName.c_str()),
                    c.red / 255.f, c.green / 255.f, c.blue / 255.f, c.alpha / 255.f);
                break;
            case WEngine::ShaderSettingType::Matrix4:
                m4 = std::get<WEngine::Mat4x4>(setting.option);
                glUniformMatrix4fv(glGetUniformLocation(glShader, setting.settingName.c_str()), 1, GL_FALSE, m4.GetRawData().data());
                break;
            case WEngine::ShaderSettingType::Texture:
                tx = std::get<WEngine::Texture>(setting.option);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, gpu_textures[tx - 1].texture);
                glUniform1i(glGetUniformLocation(glShader, setting.settingName.c_str()), 0);
                break;
            default: break;
        }
    }
}

// --------------------------------------------------------------------------------------------------------------------
// ------------------------------------------ [GPU INTERFACE IMPLEMENTATION] ------------------------------------------
// --------------------------------------------------------------------------------------------------------------------

void GPU::SETTING_ConfigureSDL()
{
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3); // temporary, should be 4.6
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
}

bool GPU::SETTING_InitGPUApi(SDL_Window* window)
{
    openGLContext = SDL_GL_CreateContext(window);

    if (openGLContext == nullptr)
    {
        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog(std::format("Failed to create a GL context, {}", SDL_GetError()));
        return false;
    }
    SDL_GL_MakeCurrent(window, openGLContext);

    if (!SDL_GL_SetSwapInterval(EngineSettings::enableVSync))
    {
        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog(std::format("Unable to set VSync, {}", SDL_GetError()));
        return false;
    }

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
    {
        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog("Failed to initialize GLAD");
        return false;
    }

    return true;
}

void GPU::SETTING_ConfigureImGui(SDL_Window *window)
{
    ImGui_ImplSDL3_InitForOpenGL(window, openGLContext);
    ImGui_ImplOpenGL3_Init(EngineSettings::glslVersion.c_str());
}

void GPU::SETTING_SetViewport(WEngine::Vector2 topLeft, WEngine::Vector2 bottomRight)
{
    glViewport((uint16)topLeft.x, (uint16)topLeft.y, (uint16)bottomRight.x, (uint16)bottomRight.y);
}

void GPU::SETTING_ToggleBlending(bool enabled)
{
    if (enabled)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    else
    {
        glDisable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ZERO);
    }
}

void GPU::SETTING_ToggleDepthTest(bool enabled)
{
    if (enabled)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);
}

void GPU::SETTING_ToggleWireFrame(bool enabled)
{
    glPolygonMode(GL_FRONT_AND_BACK, enabled ? GL_LINE : GL_FILL);
}

WEngine::Nullable<WEngine::Shader> GPU::ALLOC_CompileShader(const std::string& name)
{
    auto shaderTest = GetShader(name);

    if (shaderTest.HasValue())
    {
        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog(std::format("Shader already {} compiled", name));
    }

    WEngine::ShaderAssetMission mission{};
    mission.name = name;
    WEngine::CoreSystems::GetAssetRepo()->GetAsset(mission);

    auto vertexShader = CompileSingleShader(mission.vertexShaderSource, GL_VERTEX_SHADER);
    auto fragmentShader = CompileSingleShader(mission.fragmentShaderSource, GL_FRAGMENT_SHADER);

    if (!vertexShader.HasValue() || !fragmentShader.HasValue())
        return WEngine::Nullable<WEngine::Shader>();

    GLuint m_program = LinkShaderProgramm(vertexShader.GetValue(), fragmentShader.GetValue());

    gpu_shaders.push_back(m_program);
    // This might seem jank, but its actually fine since we dont unload shaders.
    WEngine::Shader shader = gpu_shaders.size();
    gpu_shadersHandles[name] = shader;

    return WEngine::Nullable<WEngine::Shader>(shader);
}

WEngine::Nullable<WEngine::Shader> GPU::GetShader(const std::string& name)
{
    if (gpu_shadersHandles.contains(name))
        return WEngine::Nullable<WEngine::Shader>(gpu_shadersHandles[name]);
    return WEngine::Nullable<WEngine::Shader>();
}

WEngine::Nullable<WEngine::Framebuffer> GPU::ALLOC_CreateFramebuffer(const WEngine::Vector2 &resolution)
{
    OpenGL_Framebuffer framebuffer;
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
             (uint16)resolution.x, (uint16)resolution.y, 0,
             GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glBindTexture(GL_TEXTURE_2D, 0);

    glGenFramebuffers(1, &framebuffer.framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog(std::format("Framebuffer is not complete: {}", glGetError()));
        return WEngine::Nullable<WEngine::Framebuffer>();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    gpu_vramUsage += CalcTextureSize(4, (int32)resolution.x, (int32)resolution.y);

    OpenGL_Texture openGLTexture = {texture, (int32)resolution.x, (int32)resolution.y};
    gpu_textures.push_back(openGLTexture);
    WEngine::Texture texID = gpu_textures.size();
    framebuffer.framebufferTexture = texID;
    gpu_framebuffers.push_back(framebuffer);
    return WEngine::Nullable<WEngine::Framebuffer>(gpu_framebuffers.size());

}

void GPU::SETTING_SelectFramebuffer(WEngine::Framebuffer framebuffer)
{
    if (framebuffer > gpu_framebuffers.size())
    {
        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog(std::format("Framebuffer does not exist: {}", framebuffer));
        return;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, gpu_framebuffers[framebuffer - 1].framebuffer);
}

void GPU::SETTING_SelectFramebufferScreen()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

WEngine::Nullable<WEngine::Texture> GPU::GetFramebufferTexture(WEngine::Framebuffer framebuffer)
{
    if (framebuffer > gpu_framebuffers.size())
    {
        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog(std::format("Framebuffer does not exist: {}", framebuffer));
        return WEngine::Nullable<WEngine::Texture>();
    }

    auto texture = gpu_framebuffers[framebuffer - 1].framebufferTexture;
    return WEngine::Nullable<WEngine::Texture>(texture);
}

WEngine::Nullable<WEngine::Model> GPU::ALLOC_CreateModel(const WEngine::ModelInfo& model)
{
    OpenGL_Model newModel{};
    newModel.vertexCount = model.vertices.size();

    // times two because vec2, and times two again because of UVs
    uint64 modelSize = newModel.vertexCount * sizeof(float32) * 2 * 2;

    float32 modelBuffer[newModel.vertexCount * 4];
    for (uint32 i = 0; i < model.vertices.size(); i++)
    {
        modelBuffer[0 + i * 4] = model.vertices[i].x;
        modelBuffer[1 + i * 4] = model.vertices[i].y;
        modelBuffer[2 + i * 4] = model.uvCoords[i].x;
        modelBuffer[3 + i * 4] = model.uvCoords[i].y;
    }

    glGenVertexArrays(1, &newModel.VAO);
    glGenBuffers(1, &newModel.VBO);

    glBindVertexArray(newModel.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, newModel.VBO);
    glBufferData(GL_ARRAY_BUFFER, modelSize, modelBuffer, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, newModel.vertexCount * sizeof(float), nullptr);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, newModel.vertexCount * sizeof(float), (void*)(2 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    gpu_vramUsage += modelSize;

    gpu_models.push_back(newModel);
    return WEngine::Nullable<WEngine::Model>(gpu_models.size());
}

WEngine::Nullable<WEngine::Line> GPU::ALLOC_CreateLine()
{
    OpenGL_Line newLine{};

    glGenVertexArrays(1, &newLine.VAO);
    glGenBuffers(1, &newLine.VBO);

    glBindVertexArray(newLine.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, newLine.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4, nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    gpu_vramUsage += sizeof(float) * 4;

    gpu_lines.push_back(newLine);
    return WEngine::Nullable<WEngine::Line>(gpu_lines.size());
}

WEngine::Nullable<WEngine::Texture> GPU::ALLOC_CreateTexture(const WEngine::TextureInfo &textureData)
{
    OpenGL_Texture newTexture{};
    if (textureData.name.empty())
    {
        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog("Texture must have a name to be created");
        return WEngine::Nullable<WEngine::Texture>();
    }
    if (textureData.data == nullptr)
    {
        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog("Texture data was nullptr, couldn't create.");
        return WEngine::Nullable<WEngine::Texture>();
    }
    if (textureData.width <= 0 || textureData.height <= 0)
    {
        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog("Texture size is invalid");
        return WEngine::Nullable<WEngine::Texture>();
    }

    glGenTextures(1, &newTexture.texture);
    glBindTexture(GL_TEXTURE_2D, newTexture.texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureData.width, textureData.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData.data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    if constexpr (!EngineSettings::useTextureFiltering)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
    else
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    newTexture.width = textureData.width;
    newTexture.height = textureData.height;

    gpu_vramUsage += CalcTextureSize(4, (int32)newTexture.width, (int32)newTexture.height);

    gpu_textures.push_back(newTexture);
    WEngine::Texture texture = gpu_textures.size();
    gpu_textureHandles[textureData.name] = texture;

    return WEngine::Nullable<WEngine::Texture>(texture);
}

WEngine::Nullable<WEngine::Texture> GPU::GetTexture(const std::string &name)
{
    if (gpu_textureHandles.contains(name))
        return WEngine::Nullable<WEngine::Texture>(gpu_textureHandles[name]);
    return WEngine::Nullable<WEngine::Texture>();
}

WEngine::Nullable<WEngine::Vector2> GPU::GetTextureSize(WEngine::Texture texture)
{
    if (texture > gpu_textures.size())
        return WEngine::Nullable<WEngine::Vector2>();

    WEngine::Vector2 size = { (float32)gpu_textures[texture - 1].width, (float32)gpu_textures[texture - 1].height };
    return WEngine::Nullable<WEngine::Vector2>(size);
}

void GPU::DRAWCALL_ClearScreen(WEngine::Color clearColor)
{
    glClearColor((float32)clearColor.red / 255.f, (float32)clearColor.green / 255.f,
        (float32)clearColor.blue / 255.f, (float32)clearColor.alpha / 255.f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void GPU::DRAWCALL_DrawModel(WEngine::Model model, WEngine::Shader shader, const WEngine::ShaderSettings& settings)
{
    GLuint glShader = gpu_shaders[shader - 1];

    glUseProgram(glShader);

    ApplyShaderSettings(glShader, settings);

    glBindVertexArray(gpu_models[model - 1].VAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, gpu_models[model - 1].vertexCount);

    glBindVertexArray(0);
    glUseProgram(0);
    gpu_drawCallsThisFrame++;
}

void GPU::DRAWCALL_DrawLine(WEngine::Line line, WEngine::Shader shader,
    const WEngine::ShaderSettings &settings,  const WEngine::LineInfo& newModel)
{
    GLuint glShader = gpu_shaders[shader - 1];
    OpenGL_Line glModel = gpu_lines[line - 1];

    glUseProgram(glShader);

    ApplyShaderSettings(glShader, settings);

    uint64 modelSize = sizeof(float32) * 2 * 2;

    std::array<float32, 4> vertData;
    vertData[0] = newModel.line.p1.x;
    vertData[1] = newModel.line.p1.y;
    vertData[2] = newModel.line.p2.x;
    vertData[3] = newModel.line.p2.y;

    glBindBuffer(GL_ARRAY_BUFFER, glModel.VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, modelSize, vertData.data());

    glBindVertexArray(glModel.VAO);
    glDrawArrays(GL_LINES, 0, 2);

    glBindVertexArray(0);
    glUseProgram(0);
    gpu_drawCallsThisFrame++;
}

void GPU::DRAWCALL_ResetImGui()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
}

void GPU::DRAWCALL_DrawImGui()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // This is actually inaccurate, ImGui does way more than just one draw call.
    gpu_drawCallsThisFrame++;
}

void GPU::DRAWCALL_SwapBuffers(SDL_Window *window)
{
    SDL_GL_SwapWindow(window);
    gpu_drawCallsLastFrame = gpu_drawCallsThisFrame;
    gpu_drawCallsThisFrame = 0;
}

uint64 GPU::GetVramUsage()
{
    return gpu_vramUsage;
}

uint32 GPU::GetDrawCallCountLastFrame()
{
    return gpu_drawCallsLastFrame;
}

WEngine::Nullable<ImTextureID> GPU::FramebufferToImGui(WEngine::Framebuffer framebuffer)
{
    if (framebuffer > gpu_framebuffers.size())
    {
        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog(std::format("Framebuffer does not exist: {}", framebuffer));
        return WEngine::Nullable<ImTextureID>();
    }

    auto fb = gpu_framebuffers[framebuffer - 1];
    GLuint texture = gpu_textures[fb.framebufferTexture - 1].texture;
    return WEngine::Nullable<ImTextureID>(texture);
}


#endif
