#pragma once

#include <Engine/Types/CommonTypes.h>

namespace WEngine
{
    struct Material
    {
        Material() : handle(0) {};
        Material(uint64 handle) : handle(handle) {};
        operator uint64() const { return handle; }
        uint64 handle;
    };
}