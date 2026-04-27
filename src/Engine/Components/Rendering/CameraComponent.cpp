#include "CameraComponent.h"

#include <../../Core/World/Entity.h>
#include <Engine/Types/CoreSystems.h>
#include <Engine/Core/Handlers/RenderHandler.h>
#include <../../Core/World/Entity.h>
#include <Engine/Types/SpawnArgs.h>
#include <../../Types/Rendering/Color.h>

using namespace WEngine;

REGISTER_COMPONENT(CameraComponent)

CameraComponent::CameraComponent(Entity* e)
{
	COMP_SETUP("Camera Component")
	m_cameraTransform = Transform::Zero;
}

const Transform CameraComponent::GetCameraTransform() const
{
	Transform t = m_cameraTransform;
	t.position = t.position + m_offset; // += not yet implemented for vec2
	return t;
}

void CameraComponent::SetCameraTransform(const Transform& newTranform)
{
	m_cameraTransform = newTranform;
}

void CameraComponent::Awake(ComponentArgs ca)
{
	auto color = ca.GetColorFromParams("backColor");
	auto offset = ca.GetVector2FromParams("offset");

	if (color.HasValue())
		m_backColor = color.GetValue();
	else
		m_backColor = Color(0, 0, 0, 255); // fallback to black

	if (offset.HasValue())
		m_offset = offset.GetValue();
	else
		m_offset = Vector2::Zero; // Fallback to no offset
}

void CameraComponent::Start()
{
	//CoreSystems::GetRenderHandler()->SetNewCamera(this);
}

void CameraComponent::Tick(float32 dt)
{
	SetCameraTransform(entity->transform);
}

void CameraComponent::UnloadComponent()
{
	//if (CoreSystems::GetRenderHandler()->GetCurrentCamera() == this)
	//	CoreSystems::GetRenderHandler()->SetNewCamera(nullptr);
}

Color CameraComponent::GetBackColor()
{
	return m_backColor;
}
