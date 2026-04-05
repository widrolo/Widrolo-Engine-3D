#pragma once

#include <Engine/Types/CommonTypes.h>

namespace WEngine
{
    class Sector;
    class Entity;
    class Component;
}
namespace WEditor
{
    struct EditorState
    {
        _GLOBAL_ bool EditorMode = false;

        _GLOBAL_ WEngine::Sector* SelectedSector = nullptr;
        _GLOBAL_ WEngine::Entity* SelectedEntity = nullptr;
        _GLOBAL_ WEngine::Component* SelectedComponent = nullptr;
        _GLOBAL_ bool ViewportSelected = false;
    };
}