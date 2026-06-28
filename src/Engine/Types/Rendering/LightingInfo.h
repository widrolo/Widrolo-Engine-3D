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

    struct AmbientLight
    {
        float32 intensity;
        Color ambientColor;
    };

    /**
     * The lighting struct sent to the GPU
     */
    struct LightingInfo
    {
        Sunlight sun;
        AmbientLight ambient;
        Vector3 cameraPos;
        float32 timeOfDay;
    };
}
