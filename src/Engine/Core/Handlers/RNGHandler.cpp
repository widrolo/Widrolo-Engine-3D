#include "RNGHandler.h"
#include <Engine/Math/Vector.h>

using namespace WEngine;

RNGHandler::RNGHandler()
{
	m_rng = std::mt19937(m_rd());
}

int64 RNGHandler::GetRandomInt(const int64 min, const int64 max)
{
	std::uniform_int_distribution<int64> dist(min, max);
	return dist(m_rng);
}

float32 RNGHandler::GetRandomFloat(const float32 min, const float32 max)
{
	std::uniform_real_distribution<float32> dist(min, max);
	return dist(m_rng);
}

Vector2 RNGHandler::GetRandomVector2(const float32 maxLength)
{
	float32 angle = GetRandomFloat(0.0f, 2.0f * 3.14159265f);
	float32 length = GetRandomFloat(0.0f, maxLength);

	Vector2 vec;
	vec.x = cosf(angle) * length;
	vec.y = sinf(angle) * length;
	return vec;
}
