#include "Component.h"

#include <Engine/Types/SpawnArgs.h>
#include <Engine/Components/InputComponent.h>
#include <Engine/Util/Log.h>
#include <Engine/Core/World/Entity.h>

using namespace WEngine;

Component::Component(Entity* e)
{
	COMP_SETUP("{unkn}");
}

void Component::Awake(ComponentArgs ca)
{
}

std::string Component::GetName() const
{
	return m_nameOfComponent;
}

Vector2 Component::GetPosition() const
{
	return entity->transform.position;
}

void Component::MakeErrorMsg(int line, std::string reason)
{
	WLog::SetConsoleError();
	WLog::ConsoleLog(std::format("Component {} of Entity {} on line {} : {}", m_nameOfComponent, m_ownerName, line, reason));
}

