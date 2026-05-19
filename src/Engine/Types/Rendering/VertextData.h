#pragma once
#include "Engine/Math/Vector.h"

namespace WEngine
{
    struct VertexData
    {
        Vector3 position;   // position of the vertex in model space
        Vector3 vertColor;  // vertex color, for blending materials with vertex painting
        Vector2 uv0Coord;   // for color uv, which can overlap
        Vector2 uv1Coord;   // for shadow uv, which cannot overlap
    };
}
