#include "AnimationRendererComponent.h"
#include <../../Core/World/Entity.h>
#include <Engine/Types/CoreSystems.h>
#include <Engine/Core/Handlers/RenderHandler.h>

#include "Engine/Core/Handlers/AssetRepo.h"
#include "Engine/Core/Handlers/RNGHandler.h"
#include "Engine/Util/Log.h"

using namespace WEngine;

REGISTER_COMPONENT(AnimationRendererComponent)

AnimationRendererComponent::AnimationRendererComponent(Entity* e)
{
	COMP_SETUP("AnimationRendererComponent");
}

void AnimationRendererComponent::Awake(ComponentArgs ca)
{
	auto spriteName = ca.GetStringFromParams("atlasName");
	auto offset = ca.GetVector2FromParams("offset");
	auto size = ca.GetFloatFromParams("scale");

	if (!spriteName.HasValue())
	{
		WLog::SetConsoleError();
		WLog::ConsoleLog("No sprite in Yaml");
		return;
	}

	if (offset.HasValue())
		m_offset = offset.GetValue();

	if (size.HasValue())
		m_scale = size.GetValue();

	auto startAnimArch = ca.GetStringFromParams("startAnimArch");
	auto startAnimSubt = ca.GetStringFromParams("startAnimSubt");
	auto startAnimName = ca.GetStringFromParams("startAnimName");


	CreateAtlas(spriteName.GetValue());

	if (startAnimName.HasValue() && startAnimSubt.HasValue() && startAnimArch.HasValue())
		SetAnimDirect(startAnimArch.GetValue(), startAnimSubt.GetValue(), startAnimName.GetValue());
	else
		m_currentAnim = {};

	m_nextFrameCounter = 0.0f;
	SetupAfterAnimSwitch();

	SetFlip(false, false);
}

void AnimationRendererComponent::Tick(float32 dt)
{
	m_position = entity->transform.position;

	m_nextFrameCounter += dt;

	if (m_nextFrameCounter >= m_frameTimeBetween)
	{
		m_nextFrameCounter = 0.0f;

		if (m_atlas.GetAnimationInfo().GetName().empty())
		{
			m_currentFrame = 0;
			m_isPlaying = false;
			return;
		}

		if (m_currentFrame != m_currentAnim.endFrame)
		{
			m_currentFrame++;
			m_isPlaying = true;
		}
		else
		{
			m_isPlaying = false;
		}
	}
}

void AnimationRendererComponent::Draw()
{
	RenderAnim();
}

void AnimationRendererComponent::CreateAtlas(const std::string& name)
{
	SpriteAssetMission mission{};
	mission.name = name;
	CoreSystems::GetAssetRepo()->GetAsset(mission);
	m_atlas.LoadAtlas(mission.sprite, name);
}

void AnimationRendererComponent::RenderAnim()
{
	Rectangle rect{};
	rect.p1 = m_position + m_offset;
	rect.p2 = m_atlas.GetSizeSingle() * m_scale;

	Vector2 flip = { m_flipX ? 1.0f : -1.0f, m_flipY ? 1.0f : -1.0f };
	auto uvs = m_atlas.GetFrameUVCoords(m_currentFrame);
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

void AnimationRendererComponent::SetFlip(bool flipX, bool flipY)
{
	m_flipX = flipX;
	m_flipY = flipY;
}

void AnimationRendererComponent::SetOffset(const Vector2 offset)
{
	m_offset = offset;
}

void AnimationRendererComponent::SetAnimDirect(const std::string &archetype, const std::string &subtype, const std::string &name)
{
	auto animN = m_atlas.GetAnimationInfo().GetAnimation(archetype, subtype, name);

	if (animN.HasValue())
		m_currentAnim = animN.GetValue();
	else
		m_currentAnim = {};

	SetupAfterAnimSwitch();
}

uint8 AnimationRendererComponent::SetAnimRandom(const std::string &archetype, const std::string &subtype, const std::string &name, const uint8 variations)
{
	if (variations == 0)
	{
		SetAnimDirect(archetype, subtype, name + std::to_string(0));
		return 0;
	}
	const uint8 decision = CoreSystems::GetRNGHandler()->GetRandomInt(0, variations - 1);
	const std::string variantName = name + std::to_string(decision);
	SetAnimDirect(archetype, subtype, variantName);
	return decision;
}

void AnimationRendererComponent::SetAtlas(const std::string &atlasName)
{
	CreateAtlas(atlasName);
}

bool AnimationRendererComponent::IsAnimationPlaying() const
{
	return m_isPlaying;
}

void AnimationRendererComponent::SetupAfterAnimSwitch()
{
	m_nextFrameCounter = 0.0f;

	m_frameTimeBetween = 1.0f / (float32)m_atlas.GetAnimationInfo().GetFramerate();
	m_currentFrame = m_currentAnim.startFrame;
	m_isPlaying = true;
}
