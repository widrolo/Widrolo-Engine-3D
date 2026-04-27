#include "RectAreaComponent.h"
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

REGISTER_COMPONENT(RectAreaComponent);

RectAreaComponent::RectAreaComponent(Entity *e)
{
    COMP_SETUP("Rectangular Area Component");
}

RectAreaComponent::~RectAreaComponent()
{

}

void RectAreaComponent::TryAttach()
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

void RectAreaComponent::SetSize(Vector2 size)
{
    m_areaSize = size;

    auto areaNullable = m_simulatableObject->GetAreaRect();

    if (!areaNullable.HasValue())
    {
        WLog::SetConsoleWarning();
        WLog::ConsoleLog("Tried to set rect size but area is not rect!");
    }

    auto area = areaNullable.GetValue();
    area->SetBox(size);
}

void RectAreaComponent::Awake(ComponentArgs ca)
{
    auto offset = ca.GetVector2FromParams("offset");
    auto size = ca.GetVector2FromParams("areaSize");

    if (offset.HasValue())
        m_offset = offset.GetValue();
    else
        m_offset = Vector2::Zero;

    if (size.HasValue())
        m_areaSize = size.GetValue();
    else
        m_areaSize = Vector2::Zero;
}

void RectAreaComponent::Start()
{
    TryAttach();
    if (m_simulatableObject != nullptr)
        Init();
}

void RectAreaComponent::Draw()
{
    if (m_simulatableObject == nullptr)
        return;
    auto lines = PhysicsVisualizer::GetRectVisual(*m_simulatableObject);

    RenderVisualizationMission mission{};
    mission.lines = lines;
    mission.color = Color::Green;

    //CoreSystems::GetRenderHandler()->AddToVisualizationQueue(mission);
}

void RectAreaComponent::Init()
{
    m_simulatableObject->UnlockAreaType();
    m_simulatableObject->SetAreaRect(m_areaSize);
    m_simulatableObject->LockAreaType();
    m_simulatableObject->SetAreaOffset(m_offset);
}
