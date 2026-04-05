#include "SimulatableObject.h"

#include <Engine/Util/Log.h>

using namespace WEngine;

SimulatableObject::SimulatableObject()
    : m_id(GetNextID())
    , m_position(Vector2::Zero)
    , m_velocity(Vector2::Zero)
    , m_acceleration(Vector2::Zero)
    , m_gravity(0.0f)
    , m_damping(0.0f)
    , m_frozen(false)
    , m_areaType(PhysicsAreaType::None)
    , m_physicsArea(nullptr)
    , m_lockAreaType(false)
    , m_owner(nullptr)
{

}

SimulatableObject::~SimulatableObject()
{
    if (m_physicsArea != nullptr)
        WAllocator::Destruct(m_physicsArea);
}

void SimulatableObject::RemoveArea()
{
    if (m_lockAreaType)
    {
        WLog::SetConsoleWarning();
        WLog::ConsoleLog("Tried to remove area while area was locked!");
        return;
    }

    if (m_areaType == PhysicsAreaType::None)
    {
        WLog::SetConsoleInfo();
        WLog::ConsoleLog("Tried to remove area but there was no area to be removed.");
        return;
    }

    m_areaType = PhysicsAreaType::None;
    WAllocator::Destruct(m_physicsArea);
    m_physicsArea = nullptr;
}

void SimulatableObject::SetAreaCircle(const float32 radius)
{
    if (m_areaType != PhysicsAreaType::Circle)
    {
        if (m_lockAreaType)
        {
            WLog::SetConsoleWarning();
            WLog::ConsoleLog("Tried to set area to circle while area was locked!");
            return;
        }
        if (m_physicsArea != nullptr)
            WAllocator::Destruct(m_physicsArea);
        m_physicsArea = WAllocator::Construct<PhysicsAreaCircle>();
    }

    const auto circle = (PhysicsAreaCircle*)m_physicsArea;
    circle->SetRadius(radius);
    m_areaType = PhysicsAreaType::Circle;
}

void SimulatableObject::SetAreaRect(const Vector2 size)
{
    if (m_areaType != PhysicsAreaType::Rectangle)
    {
        if (m_lockAreaType)
        {
            WLog::SetConsoleWarning();
            WLog::ConsoleLog("Tried to set area to Rect while area was locked!");
            return;
        }
        if (m_physicsArea != nullptr)
            WAllocator::Destruct(m_physicsArea);
        m_physicsArea = WAllocator::Construct<PhysicsAreaRectangle>();
    }
    const auto rect = (PhysicsAreaRectangle*)m_physicsArea;
    rect->SetBox(size);
    m_areaType = PhysicsAreaType::Rectangle;
}

void SimulatableObject::SetAreaWorldBoundary(const WorldBoundaryOrientation orientation)
{
    if (m_areaType != PhysicsAreaType::WorldBoundary)
    {
        if (m_lockAreaType)
        {
            WLog::SetConsoleWarning();
            WLog::ConsoleLog("Tried to set area to WB while area was locked!");
            return;
        }
        if (m_physicsArea != nullptr)
            WAllocator::Destruct(m_physicsArea);
        m_physicsArea = WAllocator::Construct<PhysicsAreaWorldBoundary>();
    }
    const auto worldBoundary = (PhysicsAreaWorldBoundary*)m_physicsArea;
    worldBoundary->SetOrientation(orientation);
    m_areaType = PhysicsAreaType::WorldBoundary;
}

void SimulatableObject::SetAreaCapsule(const float32 radius, const float32 midHeight)
{
    if (m_areaType != PhysicsAreaType::Cylinder)
    {
        if (m_lockAreaType)
        {
            WLog::SetConsoleWarning();
            WLog::ConsoleLog("Tried to set area to Capsule while area was locked!");
            return;
        }
        if (m_physicsArea != nullptr)
            WAllocator::Destruct(m_physicsArea);
        m_physicsArea = WAllocator::Construct<PhysicsAreaCapsule>();
    }
    const auto capsule = (PhysicsAreaCapsule*)m_physicsArea;
    capsule->SetRadius(radius);
    capsule->SetMidHeight(midHeight);
    m_areaType = PhysicsAreaType::Cylinder;
}

void SimulatableObject::SetAreaPolygon(const wtl::vector<Vector2>& vertices)
{
    if (m_areaType != PhysicsAreaType::Polygon)
    {
        if (m_lockAreaType)
        {
            WLog::SetConsoleWarning();
            WLog::ConsoleLog("Tried to set area to Polygon while area was locked!");
            return;
        }
        if (m_physicsArea != nullptr)
            WAllocator::Destruct(m_physicsArea);
        m_physicsArea = WAllocator::Construct<PhysicsAreaPolygon>();
    }
    const auto polygon = (PhysicsAreaPolygon*)m_physicsArea;
    polygon->SetPolygons(vertices);
    m_areaType = PhysicsAreaType::Polygon;
}

Nullable<PhysicsAreaCircle*> SimulatableObject::GetAreaCircle() const
{
    if (m_areaType != PhysicsAreaType::Circle)
        return Nullable<PhysicsAreaCircle*>();
    return Nullable((PhysicsAreaCircle*)m_physicsArea);
}

Nullable<PhysicsAreaRectangle*> SimulatableObject::GetAreaRect() const
{
    if (m_areaType != PhysicsAreaType::Rectangle)
        return Nullable<PhysicsAreaRectangle*>();
    return Nullable((PhysicsAreaRectangle*)m_physicsArea);
}

Nullable<PhysicsAreaWorldBoundary*> SimulatableObject::GetAreaWorldBoundary() const
{
    if (m_areaType != PhysicsAreaType::WorldBoundary)
        return Nullable<PhysicsAreaWorldBoundary*>();
    return Nullable((PhysicsAreaWorldBoundary*)m_physicsArea);
}

Nullable<PhysicsAreaCapsule*> SimulatableObject::GetAreaCapsule() const
{
    if (m_areaType != PhysicsAreaType::Cylinder)
        return Nullable<PhysicsAreaCapsule*>();
    return Nullable((PhysicsAreaCapsule*)m_physicsArea);
}

Nullable<PhysicsAreaPolygon*> SimulatableObject::GetAreaPolygon() const
{
    if (m_areaType != PhysicsAreaType::Polygon)
        return Nullable<PhysicsAreaPolygon*>();
    return Nullable((PhysicsAreaPolygon*)m_physicsArea);
}

uint64 SimulatableObject::GetNextID()
{
    static uint64 nextID = 0;
    return nextID++;
}

