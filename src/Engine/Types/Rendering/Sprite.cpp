#include "Sprite.h"

#include "Engine/Core/System/GPU.h"

using namespace WEngine;

Texture Sprite::GetTexture()
{
	return m_spriteTexture;
}

Vector2 Sprite::GetSize()
{
	return m_size;
}
