#pragma once

#include <Engine/Types/CommonTypes.h>

namespace WEngine
{
    struct Model
    {
        Model() : handle(0) {};
        Model(uint64 handle) : handle(handle) {};
        operator uint64() const { return handle; }
        uint64 handle;
    };
}