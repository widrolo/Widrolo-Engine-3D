#include "ComponentFactory.h"
#include <Engine/Components/Component.h>
#include <Engine/Types/SpawnArgs.h>
#include <../../Core/World/Entity.h>
#include <Engine/Util/Log.h>
#include <unordered_map>

using namespace WEngine;

std::unordered_map<uint16_t, std::function<Component*(WEngine::Entity*)>>& GetRegistry()
{
	static std::unordered_map<uint16_t, std::function<Component*(WEngine::Entity*)>> registry(2048);
	return registry;
}

Component* ComponentFactory::CreateComponent(const ComponentArgs &args, Entity* e)
{
	auto& reg = GetRegistry();
	auto it = reg.find(args.componentTypeId);
	if (it == reg.end())
	{
		WLog::SetConsoleError();
		WLog::ConsoleLog(std::format("ComponentTypeID not registered: {}", args.componentTypeId));
		return nullptr;
	}

	// Very important to note, second is a function which creates the
	// component. The function is a lambda defined in Component.h.
	Component* component = it->second(e);

	if (component != nullptr)
		component->Awake(args);

	return component;
}

void ComponentFactory::Register(const uint16_t id, std::function<Component*(Entity*)> creator)
{
	GetRegistry()[id] = creator;
}