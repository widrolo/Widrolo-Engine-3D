#pragma once


#include <WGL.h>
#include <string>
#include <Engine/Math/Vector.h>

#include "GPU/Texture.h"

namespace WEngine
{
	/**
 	 * Represents a sprite in the game engine.
 	 */
	class Sprite
	{
	public:
		Sprite() = default;
		Sprite(Texture texture, Vector2 size) : m_spriteTexture(texture), m_size(size) {};
		~Sprite() = default;
	private:
		Texture m_spriteTexture = 0;
		Vector2 m_size;
	public:
		/**
	 	 * Returns the OpenGL texture handle of the loaded sprite.
	 	 * @return The OpenGL texture handle of the loaded sprite.
	 	 */
		Texture GetTexture();
		/**
		 * Returns the size of the loaded sprite in metres.
		 * @return The size of the loaded sprite as a Vector2 object.
		 */
		Vector2 GetSize();
	};
}
