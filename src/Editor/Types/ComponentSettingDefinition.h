#pragma once

#include <Engine/WTL/vector.h>
#include <string>

namespace WEditor
{
    enum class ComponentOptionType
    {
        None,
        Number,
        Float,
        Bool,
        String,
        Vector2,
        Vector3,
        Color,
    };

    struct ComponentOption
    {
        std::string optionName;
        std::string optionDesctription;
        std::string optionInternal;
        ComponentOptionType type;
    };

    struct ComponentSettingDefinition
    {
        std::string componentName;
        std::string componentDesctription;
        wtl::vector<ComponentOption> options;
    };
}