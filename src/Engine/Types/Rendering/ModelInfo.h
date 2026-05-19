#pragma once
#include "Engine/WTL/vector.h"

namespace WEngine
{
    struct VertexData;

    struct ModelInfo
    {
        wtl::vector<VertexData> vertices;
    };
}
