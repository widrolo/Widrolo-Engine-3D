#pragma once

#include <Engine/Types/CommonTypes.h>

namespace WEngine
{
    enum class OverlapDir : uint8
    {
        None = 0,
        North,
        South,
        East,
        West,
    };
    class SimulatableObject;
    struct OverlapResult
    {
        bool isOverlapping;
        bool deepPenetration;
        OverlapDir overlapDir;
        SimulatableObject* other;
    };
}