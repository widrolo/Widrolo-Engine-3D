#pragma once
#include <array>
#include <string>

#include "ShaderSettings.h"
#include "Engine/WTL/vector.h"

namespace WEngine
{
    struct MaterialDefinitionSwizzle
    {
        // index to texturesPackaging declaring which texture is going to get generated
        uint8 packedTexTarget;

        // look at me trying to be efficient and all.
        struct SwizzleOrigin
        {
            // index to texturesDevel declaring which texture is going to get sampled
            uint8 develTexOrigin : 6;
            // which channel of the texture
            uint8 channel : 2;
        };

        // a swizzle per channel
        std::array<SwizzleOrigin, 4> swizzle;
    };
    struct MaterialDefinition
    {
        bool valid = false;
        std::string name;
        std::string shaderName;

        wtl::vector<std::string> texturesDevel;
        wtl::vector<std::string> texturesPackaging;

        wtl::vector<MaterialDefinitionSwizzle> swizzles;
        ShaderSettings params;
    };
}
