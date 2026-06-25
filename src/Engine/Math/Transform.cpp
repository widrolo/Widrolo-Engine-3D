#include "Transform.h"

#include <cmath>

using namespace WEngine;

const Transform Transform::Zero = { Vector3::Zero, Vector3::Zero, Vector3::One };

Vector3 Transform::Forward()
{
    float32 pitch = rotation.x * (M_PI / 180.0f);
    float32 yaw   = rotation.y * (M_PI / 180.0f);

    Vector3 forward{
        std::sin(yaw) * std::cos(pitch),
        std::sin(pitch),
        -std::cos(yaw) * std::cos(pitch)
    };

    forward.NormaliseThis();

    return forward;
}

Vector3 Transform::Right()
{
    float32 yaw = rotation.y * (M_PI / 180.0f);

    Vector3 right{
        std::cos(yaw),
        0.0f,
        std::sin(yaw)  // was -std::sin(yaw)
    };

    right.NormaliseThis();
    return right;
}

Vector3 Transform::Up()
{
    Vector3 up = VecMath::Cross(Forward(), Right());
    up.NormaliseThis();
    return up;
}
