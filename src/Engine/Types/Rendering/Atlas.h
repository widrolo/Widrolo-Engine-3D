#pragma once

#include <WGL.h>
#include <Engine/Math/Vector.h>
#include <string>
#include <array>
#include <vector>
#include <Engine/Types/Rendering/Animation.h>
#include <yaml-cpp/yaml.h>

#include "GPU/Texture.h"

namespace WEngine
{
	class Sprite;
	/**
	 * This class takes a sprite atlas and defines the amount of sprites it contains.
	 */
	class Atlas
	{
	public:
		Atlas() = default;
		~Atlas() = default;
	private:
		Texture m_atlas = 0;
		Vector2 m_wholeSize;
		Vector2 m_frameSize;

		float32 uvLenX, uvLenY;

		AnimationInformation m_animInfo;
	public:
		/**
		 * This loads the sprite atlas into memory.
		 * @param spriteFile Name of the sprite to be loaded.
		 */
		void LoadAtlas(Sprite& sprite, const std::string &atlasName);
		/**
		 * @return GL id of the sprite.
		 */
		Texture GetTexture();
		/**
		 * @return The size of the atlas image.
		 */
		Vector2 GetSizeWhole();
		/**
		 * @return The size of a single sprite on the atlas.
		 */
		Vector2 GetSizeSingle();

		/**
		 * Calculates and returns the UV Coordinates of the requested frame.
		 * @param frame Frame of the atlas.
		 * @return Array of two vertices, ret[0] being top left and ret[1] being bottom right.
		 */
		std::array<Vector2, 2> GetFrameUVCoords(uint16 frame);

		const AnimationInformation& GetAnimationInfo() { return m_animInfo; }

	private:
		void CalcAtlasProperties();
	};
}
