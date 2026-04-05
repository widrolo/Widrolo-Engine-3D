#include "VecMath.h"

#include <Engine/Math/Vectors/Vec1f.h>
#include <Engine/Math/Vectors/Vec2f.h>
#include <Engine/Math/Vectors/Vec3f.h>
#include <Engine/Math/Vectors/Vec4f.h>

using namespace WEngine;

float32 VecMath::Dot(const Vector1 &v1, const Vector1 &v2)
{
    return v1.x * v2.x;
}
float32 VecMath::Dot(const Vector2 &v1, const Vector2 &v2)
{
    return v1.x * v2.x + v1.y * v2.y;
}
float32 VecMath::Dot(const Vector3 &v1, const Vector3 &v2)
{
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}
float32 VecMath::Dot(const Vector4 &v1, const Vector4 &v2)
{
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w;
}

Vector3 VecMath::Cross(const Vector3 &v1, const Vector3 &v2)
{
    return Vector3(v1.y * v2.z - v1.z * v2.y,
                v1.z * v2.x - v1.x * v2.z,
                v1.x * v2.y - v1.y * v2.x);
}
