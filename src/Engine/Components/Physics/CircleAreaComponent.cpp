#include "CircleAreaComponent.h"
#include <Engine/Core/System/Memory.h>
#include <Engine/Types/SpawnArgs.h>
#include <Engine/Core/World/Entity.h>
#include <Engine/Core/Physics/PhysicsVisualizer.h>
#include <Engine/Core/Handlers/RenderHandler.h>

#include "StaticBody.h"
#include "DynamicBody.h"
#include "Engine/Core/Physics/SimulatableObject.h"
#include "Engine/Types/CoreSystems.h"
#include "Engine/Util/Log.h"

using namespace WEngine;

REGISTER_COMPONENT(CircleAreaComponent);

CircleAreaComponent::CircleAreaComponent(Entity *e)
{
    COMP_SETUP("Circular Area Component");
}

CircleAreaComponent::~CircleAreaComponent()
{

}

void CircleAreaComponent::TryAttach()
{
    auto sbody = entity->GetComponent<StaticBody>();
    auto dbody = entity->GetComponent<DynamicBody>();

    if (sbody == nullptr && dbody == nullptr)
    {
        WLog::SetConsoleWarning();
        WLog::ConsoleLog(std::format("Found no suitable body in entity \"{}\", not attaching for now.", entity->entityName));
    }

    if (sbody != nullptr)
    {
        m_simulatableObject = sbody->GetSimulatableObject();
        return;
    }
    if (dbody != nullptr)
    {
        m_simulatableObject = dbody->GetSimulatableObject();
        return;
    }
}

void CircleAreaComponent::Awake(ComponentArgs ca)
{
    auto offset = ca.GetVector2FromParams("offset");
    auto radius = ca.GetFloatFromParams("areaRadius");

    if (offset.HasValue())
        m_offset = offset.GetValue();
    else
        m_offset = Vector2::Zero;

    if (radius.HasValue())
        m_radius = radius.GetValue();
    else
        m_radius = 0.0f;
}

void CircleAreaComponent::Start()
{
    TryAttach();
    if (m_simulatableObject != nullptr)
        Init();
}

void CircleAreaComponent::Draw()
{
    if (m_simulatableObject == nullptr)
        return;
    const auto lines = PhysicsVisualizer::GetCircleVisual(*m_simulatableObject);

    RenderVisualizationMission mission{};
    mission.lines = lines;
    mission.color = Color::Green;

    CoreSystems::GetRenderHandler()->AddToVisualizationQueue(mission);
}

void CircleAreaComponent::Init()
{
    m_simulatableObject->UnlockAreaType();
    m_simulatableObject->SetAreaCircle(m_radius);
    m_simulatableObject->LockAreaType();
    m_simulatableObject->SetAreaOffset(m_offset);
}
