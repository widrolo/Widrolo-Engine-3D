#include "PhysicsWatchWidget.h"

#include <cmath>
#include <Engine/Core/Engine.h>

#include "Engine/Core/Handlers/PhysicsHandler.h"
#include "Engine/Core/World/Entity.h"
#include "Engine/Types/CoreSystems.h"

using namespace WEngine;

void PhysicsWatchWidget::Setup()
{
    m_widgetName = "Physics Watch";
}

void PhysicsWatchWidget::RenderInternal()
{
    SetSize({300, 400});

    PhysicsHandler* ph = CoreSystems::GetPhysicsHandler();

    ImGui::Text("Physics Engine State");
    ImGui::Separator();

    std::string tickCount = "Ticks Last Frame: " + std::to_string(Engine::GetPhysicsTickCounter());

    ImGui::Text("%s", tickCount.c_str());
    ImGui::Separator();
}

