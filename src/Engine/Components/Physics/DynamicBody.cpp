#include "DynamicBody.h"

#include <Engine/Core/Handlers/PhysicsHandler.h>
#include <../../Core/World/Entity.h>
#include <Engine/Util/Log.h>
#include <Engine/Types/CoreSystems.h>
#include <Engine/EngineDefines.h>

#include <Engine/Types/SpawnArgs.h>

#include "Engine/Core/Physics/SimulatableObject.h"

using namespace WEngine;

REGISTER_COMPONENT(DynamicBody)

DynamicBody::DynamicBody(Entity* e)
{
	COMP_SETUP("Dynamic Body");
}

DynamicBody::~DynamicBody()
{
	UnloadComponent();
}

void DynamicBody::UpdateEntity() const
{
	if constexpr (!EngineSettings::physicsEnabled)
		return;
	if (m_simulatableObject == nullptr)
		return;

	entity->transform.position = m_simulatableObject->GetPosition();
}

void DynamicBody::SetPosition(const Vector2 position) const
{
	if constexpr (!EngineSettings::physicsEnabled)
		return;
	if (m_simulatableObject == nullptr)
		return;

	m_simulatableObject->SetPosition(position);
}

void DynamicBody::SetAcceleration(const Vector2 acceleration) const
{
	if constexpr (!EngineSettings::physicsEnabled)
		return;
	if (m_simulatableObject == nullptr)
		return;
	m_simulatableObject->SetAcceleration(acceleration);
}

void DynamicBody::SetDamping(const float32 damping) const
{
	if constexpr (!EngineSettings::physicsEnabled)
		return;
	if (m_simulatableObject == nullptr)
		return;
	m_simulatableObject->SetDamping(damping);
}

void DynamicBody::SetStartAcceleration(const Vector2 sa) const
{
	if constexpr (!EngineSettings::physicsEnabled)
		return;
	if (m_simulatableObject == nullptr)
		return;
	m_simulatableObject->SetAcceleration(sa);
}


void DynamicBody::FreezeObject() const
{
	if constexpr (!EngineSettings::physicsEnabled)
		return;
	if (m_simulatableObject == nullptr)
		return;
	m_simulatableObject->SetFreezeState(true);
}

void DynamicBody::UnfreezeObject() const
{
	if constexpr (!EngineSettings::physicsEnabled)
		return;
	if (m_simulatableObject == nullptr)
		return;
	m_simulatableObject->SetFreezeState(false);
}


void DynamicBody::SetGravity(const float gravity) const
{
	if constexpr (!EngineSettings::physicsEnabled)
		return;
	if (m_simulatableObject == nullptr)
		return;
	m_simulatableObject->SetGravity(gravity);
}

void DynamicBody::SetVelocity(const Vector2 velocity) const
{
	if constexpr (!EngineSettings::physicsEnabled)
		return;
	if (m_simulatableObject == nullptr)
		return;
	m_simulatableObject->SetVelocity(velocity);
}

void DynamicBody::Init()
{
	if constexpr (!EngineSettings::physicsEnabled)
		return;

	uint64 id = m_physicsHandler->MakeSimulatableObject();
	auto obj = m_physicsHandler->GetSimulatableObject(id);

	if (obj.HasValue())
		m_simulatableObject = obj.GetValue();
	else
	{
		WLog::SetConsoleError();
		WLog::ConsoleLog("Physics Handler failed to make a simulatable object!");
		return;
	}
	m_simulatableObject->SetOwner(entity);
	m_simulatableObject->SetPosition(entity->transform.position);
}

void DynamicBody::Awake(ComponentArgs ca)
{
	m_physicsHandler = CoreSystems::GetPhysicsHandler();
	Init();

	auto gravity = ca.GetFloatFromParams("gravity");
	auto damping = ca.GetFloatFromParams("damping");
	auto freeze = ca.GetBoolFromParams("freeze");
	auto startAcc = ca.GetVector2FromParams("startaccel");

	if (gravity.HasValue())
		SetGravity(gravity.GetValue());
	else
		SetGravity(0.0f);

	if (damping.HasValue())
		SetDamping(damping.GetValue());
	else
		SetDamping(0.0f);

	if (freeze.HasValue())
		if (freeze.GetValue())
			FreezeObject();
		else
			UnfreezeObject();
	else
		UnfreezeObject();

	if (startAcc.HasValue())
		SetStartAcceleration(startAcc.GetValue());
	else
		SetStartAcceleration(Vector2::Zero);

}

void DynamicBody::PhysicsTick(float32 tr)
{
	auto overlap = CoreSystems::GetPhysicsHandler()->CheckOverlapping(m_simulatableObject->GetID());
	UpdateEntity();
}

void DynamicBody::UnloadComponent()
{
	m_physicsHandler->DeleteSimulatableObject(m_simulatableObject->GetID());
}
