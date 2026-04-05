#include "Quaternion.h"

#include <Engine/Math/Math.h>
using namespace WEngine;

const Quaternion Quaternion::Zero = { 0.0f, 0.0f, 0.0f, 0.0f };
const Quaternion Quaternion::One = { 1.0f, 1.0f, 1.0f, 1.0f };
const Quaternion Quaternion::Identity = { 0.0f, 0.0f, 0.0f, 1.0f };


Quaternion Quaternion::EulerToQuaternion(const Vector3& euler)
{
    // Save me Wikipedia, please!
    // https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles

    float32 cr = Math::Cos(euler.x * 0.5f);
    float32 sr = Math::Sin(euler.x * 0.5f);
    float32 cp = Math::Cos(euler.y * 0.5f);
    float32 sp = Math::Sin(euler.y * 0.5f);
    float32 cy = Math::Cos(euler.z * 0.5f);
    float32 sy = Math::Sin(euler.z * 0.5f);

    Quaternion q;
    q.w = cr * cp * cy + sr * sp * sy;
    q.x = sr * cp * cy - cr * sp * sy;
    q.y = cr * sp * cy + sr * cp * sy;
    q.z = cr * cp * sy - sr * sp * cy;

    return q;
}

Vector3 Quaternion::QuaternionToEuler(const Quaternion &q)
{
    // Save me Wikipedia, please!
    // https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles

    Vector3 euler;

    // x axis (roll)
    float32 sinr_cosp = 2 * (q.w * q.x + q.y * q.z);
    float32 cosr_cosp = 1 - 2 * (q.x * q.x + q.y * q.y);
    euler.x = Math::Atan2(sinr_cosp, cosr_cosp);

    // y axis (pitch)
    float32 sinp = Math::Sqrt(1 + 2 * (q.w * q.y - q.x * q.z));
    float32 cosp = Math::Sqrt(1 - 2 * (q.w * q.y - q.x * q.z));
    euler.y = 2.0 * Math::Atan2(sinp, cosp) - pi / 2.0;

    // y axis (yaw)
    float32 siny_cosp = 2 * (q.w * q.z + q.x * q.y);
    float32 cosy_cosp = 1 - 2 * (q.y * q.y + q.z * q.z);
    euler.z = Math::Atan2(siny_cosp, cosy_cosp);

    return euler;

}

void Quaternion::Normalize()
{
    float32 len = Magnitude();
    x = x / len;
    y = y / len;
    z = z / len;
    w = w / len;
}

float32 Quaternion::Magnitude() const
{
    return Math::Sqrt(x * x + y * y + z * z + w * w);
}
