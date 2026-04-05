#pragma once
#include <Engine/Components/Component.h>
#include <../../Math/Transform.h>
#include <../../Types/Rendering/Color.h>

namespace WEngine
{
	class CameraComponent : public Component
	{
	public:
		CameraComponent(Entity* e);
	private:
		Transform m_cameraTransform;
		Color m_backColor;
		Vector2 m_offset;
	public:
		const Transform GetCameraTransform() const;
		void SetCameraTransform(const Transform& newTranform);

		void Awake(ComponentArgs ca) override;
		void Start() override;
		void Tick(float32 dt) override;

		void UnloadComponent() override;

		Color GetBackColor();

		COMP_HASH(0x8a5b1295567acfa)
	};
}