#pragma once

#include <Engine/Types/Rendering/Sprite.h>
#include <Engine/Types/Rendering/Atlas.h>
#include <Engine/Types/Rendering/Color.h>
#include <Engine/Math/Shapes.h>
#include <Engine/Types/CommonTypes.h>

#include "ShaderSettings.h"

namespace WEngine
{
	/**
	 * Determines the order in which the layers will be rendered.
	 */
	enum class RenderLayer : uint8
	{
		None = 0, ///< Not rendered.
		Default = 1, ///< Default.
		UI = 255, ///< UI, should be the top most in gameplay.
	};

	struct RenderMission
	{
		uint32 layer;
		std::string shader;
		ShaderSettings shaderSettings;
		Rectangle quadBounds;
	};

	struct RenderVisualizationMission
	{
		Color color;
		wtl::vector<Line2D> lines;
	};
}
