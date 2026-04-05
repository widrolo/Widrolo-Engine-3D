#pragma once

#include <Engine/Math/Vectors/Vec3f.h>

namespace WEngine
{
    struct Sphere
    {
        Vector3 pos;
        float32 radius;
    };
    struct Cuboid
    {
        Vector3 p1;
        Vector3 p2;
    };
    struct Plane
    {
        Vector3 pos;
        Vector3 normal;
    };
}