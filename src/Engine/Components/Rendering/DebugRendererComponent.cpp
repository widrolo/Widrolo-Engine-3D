#include "DebugRendererComponent.h"

#include <Engine/Types/CoreSystems.h>
#include <Engine/Core/Handlers/RenderHandler.h>
#include <../../Core/World/Entity.h>

using namespace WEngine;

REGISTER_COMPONENT(DebugRendererComponent)

DebugRendererComponent::DebugRendererComponent(Entity* e)
{
	COMP_SETUP("DebugRendererComponent")
}

void DebugRendererComponent::RenderDebug()
{
	auto verts = GetRectangleVertices();

	// Wtf even was this component?!

	//RenderMissionLine* rm = WAllocator::Construct<RenderMissionLine>();
	//rm->color = { 255, 0, 0, 255 };
//
	//// Would get unrolled anyway.
	//rm->line = Line2D{ verts[0], verts[1] };
	//CoreSystems::GetRenderHandler()->AddToRenderQueue(rm);
	//rm->line = Line2D{ verts[0], verts[2] };
	//CoreSystems::GetRenderHandler()->AddToRenderQueue(rm);
	//rm->line = Line2D{ verts[0], verts[3] };
	//CoreSystems::GetRenderHandler()->AddToRenderQueue(rm);
	//rm->line = Line2D{ verts[1], verts[2] };
	//CoreSystems::GetRenderHandler()->AddToRenderQueue(rm);
	//rm->line = Line2D{ verts[1], verts[3] };
	//CoreSystems::GetRenderHandler()->AddToRenderQueue(rm);
	//rm->line = Line2D{ verts[2], verts[3] };
	//CoreSystems::GetRenderHandler()->AddToRenderQueue(rm);
}

void DebugRendererComponent::SetPosition(Vector2 pos, Vector2 size)
{
	position = pos;
	this->size = size;
}

void DebugRendererComponent::UnloadComponent()
{
}

std::array<Vector2, 4> DebugRendererComponent::GetRectangleVertices()
{
	std::array<Vector2, 4> verts;

	auto size = this->size / 2;

	verts[0] = { position.x - size.x, position.y + size.y };
	verts[1] = { position.x + size.x, position.y + size.y };
	verts[2] = { position.x - size.x, position.y - size.y };
	verts[3] = { position.x + size.x, position.y - size.y };
	return verts;
}
