#pragma once

#include <Engine/Math/Vector.h>

namespace WEngine
{
	/*
	* Transform
	* 
	* Holds information for the position, the size and
	* the rotation in deg (0 - 360)
	* 
	*/

	struct Transform
	{
		Transform(): position(Vector2::Zero), size(Vector2::Zero) {}
		Transform(Vector2 pos, Vector2 size) : position(pos), size(size) {}

		Vector2 position;
		Vector2 size;

		const static Transform Zero;
	};
}