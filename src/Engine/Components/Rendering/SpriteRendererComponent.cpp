#include "SpriteRendererComponent.h"

#include <Engine/Types/CoreSystems.h>
#include <Engine/Core/Handlers/RenderHandler.h>
#include <Engine/Core/Handlers/AssetRepo.h>
#include <Engine/Core/Handlers/RenderHandler.h>
#include <../../Types/Rendering/Sprite.h>
#include <Engine/Types/SpawnArgs.h>
#include <Engine/Util/Log.h>
#include <../../Core/World/Entity.h>

using namespace WEngine;

REGISTER_COMPONENT(SpriteRendererComponent)

SpriteRendererComponent::SpriteRendererComponent(Entity* e)
{
	COMP_SETUP("SpriteRendererComponent")
}

void SpriteRendererComponent::CreateSprite(std::string name)
{
	SpriteAssetMission mission{};
	mission.name = name;
	CoreSystems::GetAssetRepo()->GetAsset(mission);
	m_sprite = mission.sprite;
}

void SpriteRendererComponent::CreateAtlas(std::string name)
{
	SpriteAssetMission mission{};
	mission.name = name;
	CoreSystems::GetAssetRepo()->GetAsset(mission);
	m_atlas.LoadAtlas(mission.sprite, name);
}


void SpriteRendererComponent::RenderSprite()
{
	Rectangle rect{};
	rect.p1 = position;
	rect.p2 = m_sprite.GetSize() * scale;

	Vector2 flip = { m_flipX ? 1.0f : -1.0f, m_flipY ? 1.0f : -1.0f };

	RenderMission mission{};
	mission.layer = (uint8)RenderLayer::Default;
	mission.quadBounds = rect;
	mission.shaderSettings.push_back({ShaderSettingType::Color, Color::White, "u_Color"});
	mission.shaderSettings.push_back({ShaderSettingType::Vec2, flip, "u_flip"});
	mission.shaderSettings.push_back({ShaderSettingType::Texture, m_sprite.GetTexture(), "u_texture"});

	CoreSystems::GetRenderHandler()->AddToRenderQueue(mission);
}

void SpriteRendererComponent::RenderAtlas()
{
	Rectangle rect{};
	rect.p1 = position;
	rect.p2 = m_atlas.GetSizeSingle() * scale;

	Vector2 flip = { m_flipX ? 1.0f : -1.0f, m_flipY ? 1.0f : -1.0f };
	auto uvs = m_atlas.GetFrameUVCoords(m_atlasFrame);
	Vector4 uvCoords{uvs[0].x, uvs[0].y, uvs[1].x, uvs[1].y};

	RenderMission mission{};
	mission.layer = (uint8)RenderLayer::Default;
	mission.shader = "atlas";
	mission.quadBounds = rect;
	mission.shaderSettings.push_back({ShaderSettingType::Color, Color::White, "u_Color"});
	mission.shaderSettings.push_back({ShaderSettingType::Vec2, flip, "u_flip"});
	mission.shaderSettings.push_back({ShaderSettingType::Vec4, uvCoords, "u_AtlasUV"});
	mission.shaderSettings.push_back({ShaderSettingType::Texture, m_atlas.GetTexture(), "u_texture"});

	CoreSystems::GetRenderHandler()->AddToRenderQueue(mission);
}

void SpriteRendererComponent::SetPosition(Vector2 pos)
{
	position = pos;
}

void SpriteRendererComponent::SetFlip(bool flipX, bool flipY)
{
	m_flipX = flipX;
	m_flipY = flipY;
}

void SpriteRendererComponent::SetAtlasFrame(uint16 atlasFrame)
{
	m_atlasFrame = atlasFrame;
}

const Sprite SpriteRendererComponent::GetSprite() const
{
	return m_sprite;
}

void SpriteRendererComponent::Awake(ComponentArgs ca)
{
	auto spriteName = ca.GetStringFromParams("spriteName");

	if (!spriteName.HasValue())
	{
		WLog::SetConsoleError();
		WLog::ConsoleLog("No sprite in Yaml");
		return;
	}

	auto isAtlas = ca.GetBoolFromParams("isAtlas");
	auto atlasFrame = ca.GetIntFromParams("atlasFrame");
	auto size = ca.GetFloatFromParams("scale");

	if (size.HasValue())
		scale = size.GetValue();
	else
		scale = 1; // Assume scale 1

	// This is a fucking parkour...
	if (isAtlas.HasValue())
	{
		if (isAtlas.GetValue())
		{
			m_renderingAtlas = true;
			CreateAtlas(spriteName.GetValue());

			if (atlasFrame.HasValue())
				m_atlasFrame = (uint16)atlasFrame.GetValue();
			else
				m_atlasFrame = 0;
		}
		else
		{
			CreateSprite(spriteName.GetValue());
		}
	}
	else
	{
		CreateSprite(spriteName.GetValue());
	}
	SetFlip(false, false);
}

void SpriteRendererComponent::Tick(float32 dt)
{
	SetPosition(entity->transform.position);
}

void SpriteRendererComponent::Draw()
{
	if (m_renderingAtlas)
		RenderAtlas();
	else
		RenderSprite();
}
