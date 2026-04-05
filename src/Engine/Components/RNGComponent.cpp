#include "RNGComponent.h"
#include <Engine/Core/Handlers/RNGHandler.h>
#include <Engine/Types/CoreSystems.h>
#include <Engine/Math/Vector.h>
#include <Engine/Core/World/Entity.h>

using namespace WEngine;

REGISTER_COMPONENT(RNGComponent)

RNGComponent::RNGComponent(Entity* e)
{
	COMP_SETUP("RNGComponent")
}

int64 RNGComponent::GetRandomInt(int64 min, int64 max)
{
	return CoreSystems::GetRNGHandler()->GetRandomInt(min, max);
}

float32 RNGComponent::GetRandomFloat(float32 min, float32 max)
{
	return CoreSystems::GetRNGHandler()->GetRandomFloat(min, max);
}

Vector2 RNGComponent::GetRandomVector2(float32 maxLength)
{
	return CoreSystems::GetRNGHandler()->GetRandomVector2(maxLength);
}

int64 RNGComponent::GetRandomInt()
{
	return CoreSystems::GetRNGHandler()->GetRandomInt(-100, 100);
}

float32 RNGComponent::GetRandomFloat()
{
	return CoreSystems::GetRNGHandler()->GetRandomFloat(-1.0f, 1.0f);
}

