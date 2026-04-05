#pragma once

#include <Engine/Types/CommonTypes.h>

enum ComponentTypeID : uint16
{
#define X(name) name##_ID,
#include "EngineComponentList.xmacro"
    INTERNAL_ENGINE_COMPONENT_END = 1023,
#include <Game/GameComponentList.xmacro>
#undef X
    COMPONENT_TYPE_COUNT
};