#pragma once

#include <Engine/Types/CommonTypes.h>
#include <array>

namespace WEngine
{
    struct Vector1;
    struct Vector2;
    struct Vector3;
    struct Vector4;

    class VecMath
    {
    public:
        static float32 Dot(const Vector1& v1, const Vector1& v2);
        static float32 Dot(const Vector2& v1, const Vector2& v2);
        static float32 Dot(const Vector3& v1, const Vector3& v2);
        static float32 Dot(const Vector4& v1, const Vector4& v2);
        template<uint8 n>
        static float32 Dot(const std::array<float32, n>& row, const std::array<float32, n>& col)
        {
            float32 sum = 0.0f;
            for (uint8 i = 0; i < n; i++)
            {
                sum += row[i] * col[i];
            }
            return sum;
        }
        static Vector3 Cross(const Vector3& v1, const Vector3& v2);
    };



}
