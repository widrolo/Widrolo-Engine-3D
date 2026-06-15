#pragma once

#include <Engine/Types/Rendering/Color.h>
#include <Engine/Math/Shapes.h>
#include <Engine/Types/CommonTypes.h>

#include "ShaderSettings.h"
#include "Engine/Math/Transform.h"
#include "GPU/Model.h"
#include "GPU/Material.h"

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
		Material material;
		Model model;
		Transform transform;
		bool isStationary;
	};

	struct RenderVisualizationMission
	{
		Color color;
		wtl::vector<Line2D> lines;
	};
}
