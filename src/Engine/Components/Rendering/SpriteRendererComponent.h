#pragma once
#include <Engine/Components/Component.h>
#include <../../Math/Math.h>

#include <Engine/Types/CommonTypes.h>
#include <../../Types/Rendering/Sprite.h>
#include <../../Types/Rendering/Atlas.h>

namespace WEngine
{
	class SpriteRendererComponent : public Component
	{
	public:
		SpriteRendererComponent(Entity* e);
	
	public:
		Vector2 position = {};
		float32 scale = 1.0f;
	private:
		Sprite m_sprite;
		Atlas m_atlas;

		uint16 m_atlasFrame;

		bool m_renderingAtlas = false;

		bool m_flipX, m_flipY;

	public:
		void CreateSprite(std::string name);
		void CreateAtlas(std::string name);
		void RenderSprite();
		void RenderAtlas();
		void SetPosition(Vector2 pos);
		void SetFlip(bool flipX, bool flipY);
		void SetAtlasFrame(uint16 atlasFrame);

		const Sprite GetSprite() const;

		void Awake(ComponentArgs ca) override;
		void Tick(float32 dt) override;
		void Draw() override;

		COMP_HASH(0x6654b8ac3a19d36f)
	};
}