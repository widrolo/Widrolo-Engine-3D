#pragma once

#include <Engine/Types/CommonTypes.h>

namespace WEngine
{
    /**
     * Handle for a texture.
     */
    struct Texture
    {
        Texture() : handle(0) {};
        Texture(uint64 handle) : handle(handle) {};
        operator uint64() const { return handle; }
        uint64 handle;
    };
}