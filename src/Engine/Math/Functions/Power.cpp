#include <Engine/Types/CommonTypes.h>
#include "../Math.h"

#include <cmath>

using namespace WEngine;

float32 Math::Pow(const float32 x, const int32 exp)
{
    return powf(x, (float)exp);
}

float32 Math::Sqrt(const float32 x)
{
    return sqrtf(x);
}
