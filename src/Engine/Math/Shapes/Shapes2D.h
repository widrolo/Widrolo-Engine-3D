#pragma once

#include <Engine/Math/Vectors/Vec2f.h>

namespace WEngine
{
    struct Line2D
    {
        Vector2 p1;
        Vector2 p2;
    };
    struct Triangle
    {
        Vector2 p1;
        Vector2 p2;
        Vector2 p3;
    };
    struct Rectangle
    {
        Vector2 p1;
        Vector2 p2;
    };
    struct Circle
    {
        Vector2 pos;
        float32 radius;
    };
}