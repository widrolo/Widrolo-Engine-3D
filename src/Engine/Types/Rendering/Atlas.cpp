#include "Atlas.h"

#include <Engine/Util/Log.h>
#include <Engine/Util/Conversions.h>
#include <Engine/Core/Handlers/AssetRepo.h>
#include <Engine/Types/CoreSystems.h>

#include "Engine/Core/System/GPU.h"

using namespace WEngine;

void Atlas::LoadAtlas(Sprite& sprite, const std::string &atlasName)
{
	m_atlas = sprite.GetTexture();
	m_wholeSize = sprite.GetSize();
	m_animInfo = AnimationParser::ParseAnimation(atlasName);
	CalcAtlasProperties();
}

Texture Atlas::GetTexture()
{
	return m_atlas;
}

Vector2 Atlas::GetSizeWhole()
{
	return m_wholeSize;
}

Vector2 Atlas::GetSizeSingle()
{
	return m_frameSize;
}

std::array<Vector2, 2> Atlas::GetFrameUVCoords(uint16 frame)
{
	std::array<Vector2, 2> m_uv;

	Vector2 framePos{};
	framePos.y = (float32)floor((float32)frame / (float32)m_animInfo.GetHFramesCount());
	framePos.x = (float32)frame - framePos.y * m_animInfo.GetHFramesCount();

	m_uv[0] = Vector2(framePos.x * uvLenX, framePos.y * uvLenY);
	m_uv[1] = Vector2(uvLenX, uvLenY);


	return m_uv;
}

void Atlas::CalcAtlasProperties()
{
	uvLenX = 1.0f / (float32)m_animInfo.GetHFramesCount();
	uvLenY = 1.0f / (float32)m_animInfo.GetVFramesCount();

	m_frameSize.x = m_wholeSize.x / (float32)m_animInfo.GetHFramesCount();
	m_frameSize.y = m_wholeSize.y / (float32)m_animInfo.GetVFramesCount();
}
