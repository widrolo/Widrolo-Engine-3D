#pragma once
#include <string>

#include "Engine/Types/CommonTypes.h"
#include "Engine/WTL/vector.h"

#include "ShaderSettings.h"
#include "yaml-cpp/yaml.h"

namespace WEngine
{
    struct ShaderDefinition
    {
        bool valid = false;
        std::string name;

        std::string vertexCodeName;
        std::string fragmentCodeName;
        uint8 abundance = 0;
        bool depthTest = true;

        struct FragmentInfo
        {
            uint8 expectTextureCount;
            wtl::vector<std::string> colorTextures;
            wtl::vector<std::string> pbrTextures;
            wtl::vector<std::string> expectChannelNames;
            wtl::vector<std::pair<std::string, ShaderSettingType>> expectedParams;
        } fragInfo;

        void Parse(const YAML::Node& root);
    };
}
