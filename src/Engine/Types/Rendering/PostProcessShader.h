#pragma once
#include "ShaderSettings.h"
#include "GPU/Shader.h"

namespace WEngine
{
    struct PostProcessShader
    {
        std::string shaderName;
        Shader shader;
        ShaderSettings settings;
    };
}
