#pragma once

#include <Engine/Types/CommonTypes.h>

namespace WEngine
{
    struct Line
    {
        Line() : handle(0) {};
        Line(uint64 handle) : handle(handle) {};
        operator uint64() const { return handle; }
        uint64 handle;
    };
}