#include "PhysicsWatchWidget.h"

#include <cmath>
#include <Engine/Core/Engine.h>

#include "Engine/Core/Handlers/PhysicsHandler.h"
#include "Engine/Core/Physics/SimulatableObject.h"
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
    std::string objCount = "Sim Objects Count: " + std::to_string(ph->m_objects.size());

    ImGui::Text("%s", tickCount.c_str());
    ImGui::Text("%s", objCount.c_str());
    ImGui::Separator();
    for (const auto& o : ph->m_objects)
    {
        const auto& obj = o.second;
        if (obj == nullptr)
            continue;
        ImGui::PushID(obj->GetID());
        if (ImGui::TreeNode(obj->GetOwner()->entityName.c_str()))
        {
            float32 p[2] = {obj->GetPosition().x, obj->GetPosition().y};
            float32 v[2] = {obj->GetVelocity().x, obj->GetVelocity().y};
            float32 a[2] = {obj->GetAcceleration().x, obj->GetAcceleration().y};
            float32 g = obj->GetGravity();
            float32 d = obj->GetDamping();
            ImGui::DragFloat2("Position", p, 0.1f);
            ImGui::DragFloat2("Velocity", v, 0.1f);
            ImGui::DragFloat2("Acceleration", a, 0.1f);
            ImGui::DragFloat("Gravity", &g, 0.1f);
            ImGui::DragFloat("Damping", &d, 0.1f);

            switch (obj->GetAreaType())
            {
                case PhysicsAreaType::Circle:
                    PrintCircleArea(obj);
                    break;
                case PhysicsAreaType::Rectangle:
                    PrintRectArea(obj);
                    break;
                default:
                    break;
            }

            obj->SetPosition({p[0], p[1]});
            obj->SetVelocity({v[0], v[1]});
            obj->SetAcceleration({a[0], a[1]});
            obj->SetGravity(g);
            obj->SetDamping(d);

            ImGui::TreePop();
        }
        ImGui::PopID();
    }
}

void PhysicsWatchWidget::PrintCircleArea(const SimulatableObject* obj)
{
    ImGui::Text("Circular Area Info:");

    float32 p[2] = {obj->GetOffset().x, obj->GetOffset().y};
    const auto nullableR = obj->GetAreaCircle();
    float32 r;
    if (nullableR.HasValue())
        r = nullableR.GetValue()->GetRadius();
    else
    {
        ImGui::Text("Circular Area info is unavailable!");
        return;
    }

    ImGui::DragFloat2("Offset", p, 0.1f);
    ImGui::DragFloat("Radius", &r, 0.1f);

    obj->SetAreaOffset({p[0], p[1]});
    nullableR.GetValue()->SetRadius(r);
}

void PhysicsWatchWidget::PrintRectArea(const SimulatableObject *obj)
{
    ImGui::Text("Rectangular Area Info:");

    float32 p[2] = {obj->GetOffset().x, obj->GetOffset().y};
    const auto nullableRect = obj->GetAreaRect();
    Vector2 boxSize;
    if (nullableRect.HasValue())
    {
        boxSize = nullableRect.GetValue()->GetBoxSize();
    }
    else
    {
        ImGui::Text("Rectangular Area info is unavailable!");
        return;
    }

    ImGui::DragFloat2("Offset", p, 0.1f);

    float32 size[2];
    size[0] = boxSize.x;
    size[1] = boxSize.y;

    ImGui::DragFloat2("Size", size, 0.1f);

    obj->SetAreaOffset({p[0], p[1]});
    nullableRect.GetValue()->SetBox(boxSize);
}
