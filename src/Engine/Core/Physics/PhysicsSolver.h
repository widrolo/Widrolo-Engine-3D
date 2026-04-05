#pragma once

#include <Engine/Core/Physics/SimulatableObject.h>
#include <Engine/Types/Physics/OverlapResult.h>

namespace WEngine
{
    class PhysicsSolver
    {
    public:
        static void ApplyForces(SimulatableObject& object);
        static void ResolveCollision(SimulatableObject& o1, SimulatableObject& o2);
        static OverlapResult TestOverlap(const SimulatableObject& o1, const SimulatableObject& o2);
        static OverlapResult TestOverlap(SimulatableObject& o1,SimulatableObject& o2, bool resolveToo = true);

        static Vector2 GetWorldPosition(const SimulatableObject& object);
    private:
        using OverlapFn = OverlapResult(*)(const SimulatableObject&, const SimulatableObject&);
        using ResolveFn = void(*)(SimulatableObject&, const SimulatableObject&);
        static const OverlapFn overlapTable[4][4];
        static const ResolveFn resolveTable[4][4];


        // --------------------------------------------- Tests ---------------------------------------------

        static OverlapResult TestOverlap_Circle_Circle(const SimulatableObject& o1, const  SimulatableObject& o2);
        static OverlapResult TestOverlap_Circle_Rect(const SimulatableObject& o1, const  SimulatableObject& o2);
        static OverlapResult TestOverlap_Rect_Rect(const SimulatableObject& o1, const  SimulatableObject& o2);

        // ------------------------------------------- Resolving -------------------------------------------

        static void ResolveOverlap_Circle_Circle(SimulatableObject& o1, const SimulatableObject& o2);
        static void ResolveOverlap_Rect_Rect(SimulatableObject& o1, const SimulatableObject& o2);

    };
}

