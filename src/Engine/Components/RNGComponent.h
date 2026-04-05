#pragma once
#include "Component.h"

#include <Engine/Types/CommonTypes.h>

namespace WEngine
{
	class RNGHandler;
	struct Vector2;
	class RNGComponent : public Component
	{
	public:
		RNGComponent(Entity* e);

	public:
		int64 GetRandomInt(int64 min, int64 max);
		float32 GetRandomFloat(float32 min, float32 max);
		Vector2 GetRandomVector2(float32 maxLength);

		int64 GetRandomInt();
		float32 GetRandomFloat();

		COMP_HASH(0xc00a84e10fc641b7)
	};
}

