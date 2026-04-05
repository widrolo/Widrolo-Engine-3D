#include "SpawnArgs.h"
#include <sstream>

using namespace WEngine;

Nullable<int64> ComponentArgs::GetIntFromParams(std::string varName)
{
    if (!componentRoot[varName])
        return Nullable<int64>();

    return componentRoot[varName].as<int64>();
}

Nullable<float32> ComponentArgs::GetFloatFromParams(std::string varName)
{
    if (!componentRoot[varName])
        return Nullable<float32>();
    return componentRoot[varName].as<float32>();
}

Nullable<bool> ComponentArgs::GetBoolFromParams(std::string varName)
{
    if (!componentRoot[varName])
        return Nullable<bool>();
    return componentRoot[varName].as<bool>();
}

Nullable<std::string> ComponentArgs::GetStringFromParams(std::string varName)
{
    if (!componentRoot[varName])
       return Nullable<std::string>();
    return componentRoot[varName].as<std::string>();
}

Nullable<Vector2> ComponentArgs::GetVector2FromParams(std::string varName)
{
    if (!componentRoot[varName])
        return Nullable<Vector2>();
    const YAML::Node pos = componentRoot[varName];
    Vector2 v = { pos[0].as<float32>(), pos[1].as<float32>() };

    return v;
}

Nullable<Color> ComponentArgs::GetColorFromParams(std::string varName)
{
    if (!componentRoot[varName])
        return Nullable<Color>();
    const YAML::Node pos = componentRoot[varName];
    Color c = { pos[0].as<uint8>(), pos[1].as<uint8>(), pos[2].as<uint8>(), pos[3].as<uint8>() };

    return c;
}
