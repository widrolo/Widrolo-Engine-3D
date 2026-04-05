#include "PhysicsSolver.h"

#include <cmath>
#include <Engine/Core/World/Entity.h>

#include <Engine/EngineDefines.h>
#include <pstl/utils.h>

#include "Engine/Types/CoreSystems.h"

using namespace WEngine;

// For floating point inaccuracies;
constexpr float32 epsilon = 1e-5f;

// jesus
static OverlapResult currentResult;

using OverlapFn = OverlapResult(*)(const SimulatableObject&, const SimulatableObject&);
using ResolveFn = void(*)(SimulatableObject&, const SimulatableObject&);

const OverlapFn PhysicsSolver::overlapTable[4][4] =
{
    /* Circle */
    {
        &TestOverlap_Circle_Circle,
        &TestOverlap_Circle_Rect,
        nullptr,
        nullptr
    },
    /* Rectangle */
    {
        &TestOverlap_Circle_Rect,
        &TestOverlap_Rect_Rect,
        nullptr,
        nullptr
    },
    /* Boundary */
    {
        nullptr,
        nullptr,
        nullptr,
        nullptr
    },
    /* Cylinder */
    {
        nullptr,
        nullptr,
        nullptr,
        nullptr
    }
};

const ResolveFn PhysicsSolver::resolveTable[4][4] =
{
    /* Circle */
    {
        &ResolveOverlap_Circle_Circle,
        nullptr,
        nullptr,
        nullptr
    },
    /* Rectangle */
    {
        nullptr,
        &ResolveOverlap_Rect_Rect,
        nullptr,
        nullptr
    },
    /* Boundary */
    {
        nullptr,
        nullptr,
        nullptr,
        nullptr
    },
    /* Cylinder */
    {
        nullptr,
        nullptr,
        nullptr,
        nullptr
    }
};

void PhysicsSolver::ApplyForces(SimulatableObject &object)
{
    if (object.m_frozen)
        return;

    object.m_velocity = object.m_velocity + object.m_acceleration * EngineSettings::physicsTickRate * CoreSystems::GetTimeScale();
    object.m_velocity = object.m_velocity + Vector2(0.0f, object.m_gravity) * EngineSettings::physicsTickRate * CoreSystems::GetTimeScale();
    object.m_velocity = object.m_velocity * (1.0f - object.m_damping * EngineSettings::physicsTickRate * CoreSystems::GetTimeScale());
    object.m_position = object.m_position + object.m_velocity * EngineSettings::physicsTickRate * CoreSystems::GetTimeScale();
}

OverlapResult PhysicsSolver::TestOverlap(const SimulatableObject &o1, const SimulatableObject &o2)
{
    if (o1.m_frozen)
        return OverlapResult{false};
    if (o1.m_areaType == PhysicsAreaType::None || o2.m_areaType == PhysicsAreaType::None)
        return OverlapResult{false};
    return overlapTable[(int)o1.m_areaType - 1][(int)o2.m_areaType - 1](o1, o2);
}

OverlapResult PhysicsSolver::TestOverlap(SimulatableObject &o1, SimulatableObject &o2, bool resolveToo)
{
    if (o1.m_frozen)
        return OverlapResult{false};
    if (o1.m_areaType == PhysicsAreaType::None || o2.m_areaType == PhysicsAreaType::None)
        return OverlapResult{false};
    currentResult = overlapTable[(int)o1.m_areaType - 1][(int)o2.m_areaType - 1](o1, o2);
    if (currentResult.isOverlapping && resolveToo && !o1.m_frozen)
    {
        currentResult.other = &o2;
        resolveTable[(int)o1.m_areaType - 1][(int)o2.m_areaType - 1](o1, o2);
    }
    return currentResult;
}

Vector2 PhysicsSolver::GetWorldPosition(const SimulatableObject &object)
{
    return object.m_owner->transform.position + object.m_physicsArea->GetOffset();
}

OverlapResult PhysicsSolver::TestOverlap_Circle_Circle(const SimulatableObject &o1, const SimulatableObject &o2)
{
    using AreaT = PhysicsAreaCircle*;
    float32 distWorldSqr = Vector2::SqrMagnitude(GetWorldPosition(o2) - GetWorldPosition(o1));
    float32 radiusSum = ((AreaT)o1.m_physicsArea)->GetRadius() + ((AreaT)o2.m_physicsArea)->GetRadius();
    OverlapResult result{};
    result.isOverlapping = radiusSum * radiusSum > distWorldSqr + epsilon;
    return result;
}

OverlapResult PhysicsSolver::TestOverlap_Circle_Rect(const SimulatableObject &o1, const SimulatableObject &o2)
{
    OverlapResult result{};

    const SimulatableObject* c;
    const SimulatableObject* r;
    // its not certain what shape is o1 and what is o2
    if (o1.GetAreaType() == PhysicsAreaType::Circle)
    {
        c = &o1;
        r = &o2;
    }
    else
    {
        c = &o2;
        r = &o1;
    }

    auto circle = (PhysicsAreaCircle*)c->m_physicsArea;
    auto rect = (PhysicsAreaRectangle*)r->m_physicsArea;

    const float32 circleX = c->GetPosition().x + c->GetOffset().x + 1.5f;
    const float32 circleY = c->GetPosition().y + c->GetOffset().y - 0.5f;
    const float32 radius = circle->GetRadius();
    const float32 rectX = r->GetPosition().x + r->GetOffset().x;
    const float32 rectY = r->GetPosition().y + r->GetOffset().y;
    const float32 rectW = std::abs(rect->GetBoxTopLeft().x) + std::abs(rect->GetBoxBottomRight().x);
    const float32  rectH = std::abs(rect->GetBoxTopLeft().y) + std::abs(rect->GetBoxBottomRight().y);

    auto deltaX = circleX - std::max(rectX, std::min(circleX, rectX + rectW));
    auto deltaY = circleY - std::max(rectY, std::min(circleY, rectY + rectH));

    result.isOverlapping = (deltaX * deltaX + deltaY * deltaY) < (radius * radius);
    return result;
}

bool Rect_Rect_Overlapping(const SimulatableObject &o1, const SimulatableObject &o2)
{
    const auto a = o1.GetAreaRect().GetValue();
    const auto b = o2.GetAreaRect().GetValue();

    // compiler will clean this mess up pinkie promise.
    const float32 aLeft       = (PhysicsSolver::GetWorldPosition(o1) +  a->GetBoxTopLeft()).x;
    const float32 aRight      = (PhysicsSolver::GetWorldPosition(o1) +  a->GetBoxBottomRight()).x;
    const float32 aTop        = (PhysicsSolver::GetWorldPosition(o1) +  a->GetBoxTopLeft()).y;
    const float32 aBottom     = (PhysicsSolver::GetWorldPosition(o1) +  a->GetBoxBottomRight()).y;
    const float32 bLeft       = (PhysicsSolver::GetWorldPosition(o2) +  b->GetBoxTopLeft()).x;
    const float32 bRight      = (PhysicsSolver::GetWorldPosition(o2) +  b->GetBoxBottomRight()).x;
    const float32 bTop        = (PhysicsSolver::GetWorldPosition(o2) +  b->GetBoxTopLeft()).y;
    const float32 bBottom     = (PhysicsSolver::GetWorldPosition(o2) +  b->GetBoxBottomRight()).y;


    // widrolo flavored aabb. I kinda reinvented aabb checks when i was
    // doing that side project engine for a casio calculator:
    // https://github.com/widrolo/Prizm-Widrolo-Engine/blob/master/src/Engine/include/core/Collision.h line 130
    // this is still the same algorithm i sketched in a notebook 3 years ago.
    const bool xAxisAlligned = (aRight > bLeft && bRight > aLeft);
    const bool yAxisAlligned = (aBottom < bTop && bBottom < aTop);

    return xAxisAlligned && yAxisAlligned;
}

OverlapResult PhysicsSolver::TestOverlap_Rect_Rect(const SimulatableObject &o1, const SimulatableObject &o2)
{
    OverlapResult result{};

    const auto area1Nullable = o1.GetAreaRect();
    const auto area2Nullable = o2.GetAreaRect();

    if (!area1Nullable.HasValue() || !area2Nullable.HasValue())
    {
        WLog::SetConsoleWarning();
        WLog::ConsoleLog("Tried to test 2 Rect collision but one is not Rect.");
        return result;
    }

    const auto a = area1Nullable.GetValue();
    const auto b = area2Nullable.GetValue();

    result.isOverlapping = Rect_Rect_Overlapping(o1, o2);

    if (result.isOverlapping)
    {
        float32 dx =  GetWorldPosition(o1).x - GetWorldPosition(o2).x;
        float32 dy =  GetWorldPosition(o1).y - GetWorldPosition(o2).y;

        float32 px = (a->GetBoxSize().x / 2.0f + b->GetBoxSize().x / 2.0f) + std::abs(dx);
        float32 py = (a->GetBoxSize().y / 2.0f + b->GetBoxSize().y / 2.0f) + std::abs(dy);

        if (px > py)
        {
            if (dx < 0)
                result.overlapDir = OverlapDir::East;
            else
                result.overlapDir = OverlapDir::West;
        }
        else
        {
            if (dy < 0)
                result.overlapDir = OverlapDir::North;
            else
                result.overlapDir = OverlapDir::South;
        }
    }
    return result;
}

// PhysicsHandler passes it like this: o1 = main; o2 = other;
// We must then only move o1, not to disturb o2.
// Yes, this should in theory affect both, but in this case its ok since were running
// the physics at 720 tps, so its not going to be noticeable anyway.
// Also, we can always assume were overlapping, that test is always running beforehand.

void PhysicsSolver::ResolveOverlap_Circle_Circle(SimulatableObject& o1, const SimulatableObject& o2)
{
    if (o1.m_frozen)
        return;

    PhysicsAreaCircle* circleArea1 = o1.GetAreaCircle().GetValue();
    PhysicsAreaCircle* circleArea2 = o2.GetAreaCircle().GetValue();

    Vector2 dir = Vector2::Normalised(GetWorldPosition(o1) - GetWorldPosition(o2));
    float32 radiusSum = circleArea1->GetRadius() + circleArea2->GetRadius();
    // high school ahh math
    Vector2 newPos = GetWorldPosition(o2) + dir * radiusSum;
    o1.SetPosition(newPos);
}

void PhysicsSolver::ResolveOverlap_Rect_Rect(SimulatableObject& o1, const SimulatableObject& o2)
{
    if (o1.m_frozen || currentResult.overlapDir == OverlapDir::None || !o2.m_frozen )
        return;

    // What im about to do heavily relies on that 720 tps of the physics engine
    o1.m_position = o1.m_position - o1.m_velocity * EngineSettings::physicsTickRate * CoreSystems::GetTimeScale();
}