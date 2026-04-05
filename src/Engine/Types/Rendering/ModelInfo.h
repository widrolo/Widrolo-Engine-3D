#pragma once
#include <vector>

namespace WEngine
{
    struct ModelInfo
    {
        // these two are linked, mens that vertex n has the UV coords n.
        std::vector<Vector2> vertices;
        std::vector<Vector2> uvCoords;
    };
}
