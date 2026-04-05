#include <Engine/Types/CommonTypes.h>
#include "../Math.h"

#include <cmath>

using namespace WEngine;

float32 Math::Sin(float32 x)
{
    return sinf(x);
}

float32 Math::Cos(float32 x)
{
    return cosf(x);
}

float32 Math::Tan(float32 x)
{
    return tanf(x);
}

float32 Math::Asin(float32 x)
{
    return asinf(x);
}

float32 Math::Acos(float32 x)
{
    return acosf(x);
}

float32 Math::Atan(float32 x)
{
    return atanf(x);
}

float32 Math::Atan2(float32 x, float32 y)
{
    return atan2f(y, x);
}

