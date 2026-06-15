#pragma once

#include <Engine/Types/CommonTypes.h>

namespace WEngine
{
    /**
     * Handle for a framebuffer.
     */
    struct Framebuffer
    {
        Framebuffer() : handle(0) {};
        Framebuffer(uint64 handle) : handle(handle) {};
        operator uint64() const { return handle; }
        uint64 handle;
    };
}