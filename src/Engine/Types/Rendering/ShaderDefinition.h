#pragma once
#include <string>

#include "Engine/Types/CommonTypes.h"
#include "Engine/WTL/vector.h"

#include "ShaderSettings.h"

namespace WEngine
{
    struct ShaderDefinition
    {
        bool valid = false;
        std::string name;

        std::string vertexCodeName;
        std::string fragmentCodeName;
        uint8 abundance = 0;

        struct FragmentInfo
        {
            uint8 expectTextureCount;
            wtl::vector<std::string> colorTextures;
            wtl::vector<std::string> pbrTextures;
            wtl::vector<std::string> expectChannelNames;
            wtl::vector<std::pair<std::string, ShaderSettingType>> expectedParams;
        } fragInfo;
    };
}
