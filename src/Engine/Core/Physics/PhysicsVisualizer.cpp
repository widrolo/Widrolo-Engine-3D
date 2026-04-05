#include "PhysicsVisualizer.h"

#include <cmath>

#include "Engine/Types/Rendering/LineInfo.h"

using namespace WEngine;

wtl::vector<Line2D> PhysicsVisualizer::GetCircleVisual(const SimulatableObject& obj)
{
    auto areaNullable = obj.GetAreaCircle();
    if (!areaNullable.HasValue())
    {
        WLog::SetConsoleWarning();
        WLog::ConsoleLog("Tried to visualize a circle on a non circular object.");
        return wtl::vector<Line2D>();
    }

    const Color boundsColor = Color(0, 255, 0, 255);
    const PhysicsAreaCircle* area = areaNullable.GetValue();

    wtl::vector<Line2D> lines;

    uint8 sections = 32;

    for (int i = 0; i < sections; ++i)
    {
        Line2D line;
        float32 angle1 = (2.0f * 3.14159265f) / (float32)sections * ((float32)i);
        float32 angle2 = (2.0f * 3.14159265f) / (float32)sections * ((float32)i + 1);

        line.p1.x = cosf(angle1) * area->GetRadius() + 1 + obj.GetPosition().x + area->GetOffset().x;
        line.p1.y = sinf(angle1) * area->GetRadius() - 1 + obj.GetPosition().y + area->GetOffset().y;
        line.p2.x = cosf(angle2) * area->GetRadius() + 1 + obj.GetPosition().x + area->GetOffset().x;
        line.p2.y = sinf(angle2) * area->GetRadius() - 1 + obj.GetPosition().y + area->GetOffset().y;
        lines.push_back(line);
    }

    auto forces = VisualizeForces(obj);
    for (const auto& f : forces)
        lines.push_back(f);

    return lines;
}

wtl::vector<Line2D> PhysicsVisualizer::GetRectVisual(const SimulatableObject &obj)
{
    auto areaNullable = obj.GetAreaRect();
    if (!areaNullable.HasValue())
    {
        WLog::SetConsoleWarning();
        WLog::ConsoleLog("Tried to visualize a rect on a non rectangular object.");
        return wtl::vector<Line2D>();
    }

    const Color boundsColor = Color(0, 255, 0, 255);
    const PhysicsAreaRectangle* area = areaNullable.GetValue();
    const Vector2 a = area->GetBoxTopLeft();
    const Vector2 b = area->GetBoxBottomRight();

    wtl::vector<Line2D> lines;
    Line2D line;

    { // left line
        line.p1 = Vector2(a.x, a.y) + area->GetOffset() + obj.GetPosition();
        line.p2 = Vector2(b.x, a.y) + area->GetOffset() + obj.GetPosition();
        lines.push_back(line);
    }

    { // top line
        line.p1 = Vector2(a.x, a.y) + area->GetOffset() + obj.GetPosition();
        line.p2 = Vector2(a.x, b.y) + area->GetOffset() + obj.GetPosition();
        lines.push_back(line);
    }

    { // right line
        line.p1 = Vector2(b.x, a.y) + area->GetOffset() + obj.GetPosition();
        line.p2 = Vector2(b.x, b.y) + area->GetOffset() + obj.GetPosition();
        lines.push_back(line);
    }

    { // bottom line
        line.p1 = Vector2(a.x, b.y) + area->GetOffset() + obj.GetPosition();
        line.p2 = Vector2(b.x, b.y) + area->GetOffset() + obj.GetPosition();
        lines.push_back(line);
    }

    auto forces = VisualizeForces(obj);
    for (const auto& f : forces)
        lines.push_back(f);
    return lines;
}

wtl::vector<Line2D> PhysicsVisualizer::VisualizeForces(const SimulatableObject &obj)
{
    const Color velocityColor = Color(0, 255, 255, 255);
    const Color accColor = Color(255, 0, 0, 255);

    const float32 shortented = 5.0f;

    Line2D lvel;
    Line2D laccel;
    lvel.p1.x = obj.GetPosition().x + 1;
    lvel.p1.y = obj.GetPosition().y - 1;
    lvel.p2.x = obj.GetVelocity().x / shortented + obj.GetPosition().x + 1;
    lvel.p2.y = obj.GetVelocity().y / shortented + obj.GetPosition().y - 1;

    laccel.p1.x = obj.GetPosition().x + 1;
    laccel.p1.y = obj.GetPosition().y - 1;
    laccel.p2.x = obj.GetAcceleration().x / shortented + obj.GetPosition().x + 1;
    laccel.p2.y = obj.GetAcceleration().y / shortented + obj.GetPosition().y - 1;

    return wtl::vector<Line2D>({lvel, laccel});
}
