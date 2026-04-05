#pragma once

#include <cstdint>
#include <functional>

namespace WEngine {
	struct ComponentArgs;
	class Component;
	class Entity;
}

class ComponentFactory
{
	friend WEngine::Entity;
public:
	static WEngine::Component* CreateComponent(const WEngine::ComponentArgs &args, WEngine::Entity* e);
	static void Register(uint16_t id, std::function<WEngine::Component*(WEngine::Entity*)> creator);
};

