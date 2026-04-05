#include "PhysicsHandler.h"

#include <ranges>
#include <Engine/EngineDefines.h>

#include <Engine/Util/Log.h>
#include <Engine/Types/CoreSystems.h>
#include <Engine/Core/Handlers/RenderHandler.h>
#include <Engine/Core/Physics/SimulatableObject.h>
#include <Engine/Core/Physics/PhysicsSolver.h>

#include "Editor/Types/EditorState.h"

using namespace WEngine;

PhysicsHandler::PhysicsHandler()
{
	if constexpr (!EngineSettings::physicsEnabled)
		return;
	Setup();
}

void PhysicsHandler::Tick()
{
	if constexpr (!EngineSettings::physicsEnabled)
		return;

	for (int i = 0; i < m_objects.size(); ++i)
	{
		auto& sim = m_objects[i];
		if (sim == nullptr) // FIXME: fix erasing the element
			continue;
		PhysicsSolver::ApplyForces(*sim);
		for (int j = i; j < m_objects.size(); ++j)
		{
			auto& otherSim = m_objects[j];
			if (otherSim == nullptr)
				continue;
			if (otherSim == sim)
				continue;
			PhysicsSolver::TestOverlap(*sim, *otherSim, true);
		}
	}

}

uint64 PhysicsHandler::MakeSimulatableObject()
{
	if (WEditor::EditorState::EditorMode)
		return 0;
	if constexpr (!EngineSettings::physicsEnabled)
		return 0;

	auto obj = WAllocator::Construct<SimulatableObject>();
	m_objects[obj->GetID()] = obj;
	return obj->GetID();
}

Nullable<SimulatableObject*> PhysicsHandler::GetSimulatableObject(uint64 id)
{
	if (WEditor::EditorState::EditorMode)
		return Nullable<SimulatableObject*>();
	if constexpr (!EngineSettings::physicsEnabled)
		return Nullable<SimulatableObject*>();

	const auto obj = m_objects[id];

	if (obj == nullptr)
		return Nullable<SimulatableObject*>();
	return Nullable<SimulatableObject*>(obj);
}

void PhysicsHandler::DeleteSimulatableObject(uint64 id)
{
	if constexpr (!EngineSettings::physicsEnabled)
		return;

	auto it = m_objects.find(id);
	if (it == m_objects.end())
		return;

	if (it->second == nullptr)
	{
		m_objects.erase(it);
		return;
	}

	WAllocator::Destruct<SimulatableObject>(it->second);
	m_objects.erase(it);
	return;
}

wtl::vector<OverlapResult> PhysicsHandler::CheckOverlapping(uint64 id)
{
	if constexpr (!EngineSettings::physicsEnabled)
		return wtl::vector<OverlapResult>();

	const auto& obj = m_objects[id];

	if (obj == nullptr)
		return wtl::vector<OverlapResult>();

	wtl::vector<OverlapResult> results;

	for (const auto& obj2 : m_objects)
	{
		if (obj2.second == nullptr)
			continue;
		if (obj2.second == obj)
			continue;

		auto res = PhysicsSolver::TestOverlap(*obj, *obj2.second);
		if (res.isOverlapping)
			results.push_back(res);
	}
	return results;
}


void PhysicsHandler::Setup()
{
	if constexpr (!EngineSettings::physicsEnabled)
		return;
}


void PhysicsHandler::Visualize()
{
	if constexpr (!EngineSettings::physicsVisualize)
		return;


}
