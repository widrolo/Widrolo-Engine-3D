#include "StaticBody.h"

#include <Engine/Core/Handlers/PhysicsHandler.h>
#include <../../Core/World/Entity.h>
#include <Engine/Util/Log.h>
#include <Engine/Types/CoreSystems.h>

#include <Engine/EngineDefines.h>
#include <Engine/Types/SpawnArgs.h>

#include "Engine/Core/Physics/SimulatableObject.h"


using namespace WEngine;

REGISTER_COMPONENT(StaticBody)

StaticBody::StaticBody(Entity* e)
{
	COMP_SETUP("Static Body")
}

StaticBody::~StaticBody()
{
	UnloadComponent();
}


void StaticBody::UnloadComponent()
{
	m_physicsHandler->DeleteSimulatableObject(m_simulatableObject->GetID());
}

void StaticBody::Init()
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
	m_simulatableObject->SetFreezeState(true);
}

void StaticBody::Awake(ComponentArgs ca)
{
	m_physicsHandler = CoreSystems::GetPhysicsHandler();
	Init();
}

void StaticBody::PhysicsTick(float32 tr)
{
	auto overlap = CoreSystems::GetPhysicsHandler()->CheckOverlapping(m_simulatableObject->GetID());
	if (!overlap.empty())
	{
		//WLog::ConsoleLog("Yoooo");
	}
}

