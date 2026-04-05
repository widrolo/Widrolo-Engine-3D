#pragma once

#include <Engine/Core/Physics/SimulatableObject.h>
#include <Engine/Math/Shapes/Shapes2D.h>
#include <Engine/Types/Rendering/RenderMission.h>
#include <Engine/WTL/vector.h>

namespace WEngine
{
    class PhysicsVisualizer
    {
    public:
        static wtl::vector<Line2D> GetCircleVisual(const SimulatableObject& obj);
        static wtl::vector<Line2D> GetRectVisual(const SimulatableObject& obj);

    private:

        static wtl::vector<Line2D> VisualizeForces(const SimulatableObject& obj);
    };
}

