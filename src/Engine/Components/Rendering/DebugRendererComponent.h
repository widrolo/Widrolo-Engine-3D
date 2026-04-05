#pragma once
#include <Engine/Components/Component.h>
#include <Engine/Math/Vector.h>
#include <array>

namespace WEngine
{
	class RenderHandler;
	class DebugRendererComponent : public Component
	{
	public:
		DebugRendererComponent(Entity* e);

	public:
		Vector2 position = {};
		Vector2 size = {};

	public:
		void RenderDebug();
		void SetPosition(Vector2 pos, Vector2 size);

		void UnloadComponent() override;

		std::array<Vector2, 4> GetRectangleVertices();

		COMP_HASH(0xf137fb6a0c8db1db)
	};
}