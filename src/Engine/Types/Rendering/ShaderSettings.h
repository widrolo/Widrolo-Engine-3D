#pragma once
#include <variant>
#include <vector>

#include "Color.h"
#include "Engine/Math/Matrix.h"
#include "Engine/Math/Vector.h"
#include "Engine/WTL/vector.h"
#include "GPU/Texture.h"

namespace WEngine
{
    using ShaderSettingData = std::variant<float32, bool, uint32, int32, Vector2, Vector4, Color, Mat4x4, Texture>;
    enum class ShaderSettingType : uint8
    {
        Float,
        Bool,
        UInt,
        Int,
        Vec2,
        Vec4,
        Color,
        Matrix4,
        Texture
    };
    struct ShaderSetting
    {
        ShaderSettingType type;
        ShaderSettingData option;
        std::string settingName;
    };

    using ShaderSettings = wtl::vector<ShaderSetting>;

}
