#pragma once

#include <cstdint>
#include <functional>

namespace WEngine {
    struct ComponentArgs;
    class Component;
    class Entity;
}

class EditorCompFactory
{
    friend WEngine::Entity;
public:
    static WEngine::Component* CreateComponent(const WEngine::ComponentArgs &args, WEngine::Entity* e);
};
