#pragma once
#include "Engine/Math/Vectors/Vec4f.h"
#include "Engine/Math/Vectors/Vec3f.h"

namespace WEngine
{
    struct Quaternion
    {
        float x, y, z, w;

        Quaternion() : x(0.0f), y(0.0f), z(0.0f), w(1.0f) {}
        Quaternion(const Quaternion& other) : x(other.x), y(other.y), z(other.z), w(other.w) {}
        Quaternion(float x, float y, float z, float w) : y(y), z(z), w(w) {}
        Quaternion(const Vector4& other) : x(other.x), y(other.y), z(other.z), w(other.w) {}

        const static Quaternion Zero;
        const static Quaternion One;
        const static Quaternion Identity;

        [[nodiscard]] static Quaternion EulerToQuaternion(const Vector3& euler);
        [[nodiscard]] static Vector3 QuaternionToEuler(const Quaternion& q);

        void Normalize();
        [[nodiscard]] float32 Magnitude() const;
    };
}
