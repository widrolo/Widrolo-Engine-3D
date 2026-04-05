#include "Entity.h"
#include <Engine/Types/SpawnArgs.h>
#include <Engine/Math/Transform.h>
#include <Engine/Components/Component.h>
#include <Engine/Components/InputComponent.h>
#include <Engine/Components/Creation/ComponentFactory.h>
#include <Engine/Core/System/Memory.h>

#include <Engine/Util/Log.h>

#include "Editor/Types/EditorState.h"
#include <Editor/Components/EditorCompFactory.h>

using namespace WEngine;

void Entity::Awake(const SpawnArgs& args)
{
	EntityStart(args);
}

void Entity::Start()
{
	m_hasEverStarted = true;
	auto comps = m_components;
	for (const auto& comp : comps)
	{
		comp->Start();
	}
}

void Entity::Stop()
{
	for (const auto& comp : m_components)
	{
		comp->Stop();
	}
}

void Entity::CreateComponent(const ComponentArgs& ca)
{
	Component* comp;
	if (!WEditor::EditorState::EditorMode)
		comp = ComponentFactory::CreateComponent(ca, this);
	else
		comp = EditorCompFactory::CreateComponent(ca, this);
	m_components.push_back(comp);
	comp->Start();
}

void Entity::Tick(const float32 dt)
{
	for (const auto& comp : m_components)
	{
		comp->Tick(dt);
	}
}

void Entity::PhysicsTick(const float32 tr)
{
	for (const auto& comp : m_components)
		comp->PhysicsTick(tr);
}

void Entity::Draw()
{
	for (const auto& comp : m_components)
	{
		comp->Draw();
	}
}

void Entity::DrawDebug()
{
	for (const auto& comp : m_components)
	{
		comp->DrawDebug();
	}
}

void Entity::EntityStart(const SpawnArgs& args)
{
	entityName = args.name;
	transform = args.transform;
	InitBaseComponents();

	for (const auto& ca : args.ca)
	{
		Component* comp;
		if (!WEditor::EditorState::EditorMode)
			comp = ComponentFactory::CreateComponent(ca, this);
		else
			comp = EditorCompFactory::CreateComponent(ca, this);
		m_components.push_back(comp);
	}
}

void Entity::Internal_ParentSector(Sector* parentSector)
{
	this->parentSector = parentSector;
}

void Entity::EntityDestroy()
{
	for (const auto& comp : m_components)
	{
		comp->UnloadComponent();
		WAllocator::Destruct(comp);
	}
}

void Entity::InitBaseComponents()
{
	ComponentArgs args{ 7, YAML::Node() };

	input = dynamic_cast<InputComponent*>(ComponentFactory::CreateComponent(args, this));
}