#pragma once
#include "Color.h"
#include "Engine/Math/Vector.h"


namespace WEngine
{
    struct Sunlight
    {
        Vector3 direction;
        Color lightColor;
    };

    /**
     * The lighting struct sent to the GPU
     */
    struct LightingInfo
    {
        Sunlight sun;
        Vector3 cameraPos;
    };
}
