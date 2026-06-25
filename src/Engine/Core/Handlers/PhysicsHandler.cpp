#include "PhysicsHandler.h"

#include <Engine/EngineDefines.h>

#include <Engine/Util/Log.h>
#include <Engine/Types/CoreSystems.h>
#include <Engine/Core/Handlers/RenderHandler.h>

#include <Jolt/Jolt.h>

#include <Jolt/Core/Factory.h>
#include <Jolt/RegisterTypes.h>
#include "Jolt/Core/JobSystemThreadPool.h"
#include "Jolt/Core/TempAllocator.h"
#include "Jolt/Physics/PhysicsSettings.h"

#include "Editor/Types/EditorState.h"

using namespace WEngine;

static void TraceImpl(const char *inFMT, ...)
{
	// Format the message
	va_list list;
	va_start(list, inFMT);
	char buffer[1024];
	vsnprintf(buffer, sizeof(buffer), inFMT, list);
	va_end(list);

	// Print to the TTY
	WLog::ConsoleLog(buffer);
}

static bool AssertFailedImpl(const char *inExpression, const char *inMessage, const char *inFile, uint inLine)
{
	// since we get jolt specific files, we canot use out own logging system for now.
	std::cout << inFile << ":" << inLine << ": (" << inExpression << ") " << (inMessage != nullptr? inMessage : "") << std::endl;

	// Breakpoint
	return true;
};

PhysicsHandler::PhysicsHandler()
{
	if constexpr (!PhysicsSettings::physicsEnabled)
		return;
	Setup();

	JPH::RegisterDefaultAllocator();
	JPH::Trace = TraceImpl;
	JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = AssertFailedImpl;)

	JPH::Factory::sInstance = WAllocator::Construct<JPH::Factory>();
	JPH::RegisterTypes();
	JPH::TempAllocatorImpl tempAlloc(10 * MB);

	JPH::JobSystemThreadPool jobSys(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, std::thread::hardware_concurrency() - 1);

	//m_physicsSystem.Init(PhysicsSettings::maxBodies, PhysicsSettings::numBodyMutexes, PhysicsSettings::maxBodyPairs,
	//	PhysicsSettings::maxContactConstraints)
}

void PhysicsHandler::Tick()
{
	if constexpr (!PhysicsSettings::physicsEnabled)
		return;

}

uint64 PhysicsHandler::MakeSimulatableObject()
{
	if (WEditor::EditorState::EditorMode)
		return 0;
	if constexpr (!PhysicsSettings::physicsEnabled)
		return 0;

}

Nullable<SimulatableObject*> PhysicsHandler::GetSimulatableObject(uint64 id)
{
	if (WEditor::EditorState::EditorMode)
		return Nullable<SimulatableObject*>();
	if constexpr (!PhysicsSettings::physicsEnabled)
		return Nullable<SimulatableObject*>();

}

void PhysicsHandler::DeleteSimulatableObject(uint64 id)
{
	if constexpr (!PhysicsSettings::physicsEnabled)
		return;

}

wtl::vector<OverlapResult> PhysicsHandler::CheckOverlapping(uint64 id)
{
	if constexpr (!PhysicsSettings::physicsEnabled)
		return wtl::vector<OverlapResult>();

}


void PhysicsHandler::Setup()
{
	if constexpr (!PhysicsSettings::physicsEnabled)
		return;
}


void PhysicsHandler::Visualize()
{
	if constexpr (!PhysicsSettings::physicsVisualize)
		return;

}
