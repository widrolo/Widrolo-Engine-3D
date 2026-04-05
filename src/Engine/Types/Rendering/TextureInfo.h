#pragma once
#include <string>
#include <Engine/Types/CommonTypes.h>

namespace WEngine
{
    struct TextureInfo
    {
        std::string name;
        uint8* data;
        uint16 width;
        uint16 height;
    };
}
