#pragma once
#include "Engine/WTL/vector.h"

namespace WEngine
{
    struct VertexData;

    struct ModelInfo
    {
        std::string name;
        wtl::vector<VertexData> vertices;
        wtl::vector<uint32> indices;
    };
}
