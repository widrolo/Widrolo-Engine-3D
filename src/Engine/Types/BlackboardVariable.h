#pragma once
#include <string>
#include <Engine/Types/CommonTypes.h>
#include <Engine/Types/Rendering/Color.h>
#include <Engine/Math/Vector.h>

#include "Engine/Components/Component.h"
#include "Engine/Core/World/Sector.h"
#include "Engine/Util/Log.h"
#include <Engine/Core/World/Entity.h>

namespace WEngine
{
    enum class BlackboardVariableType
    {
        Boolean,
        Byte,
        Number,
        LargeNumber,
        Float,
        String,
        Vector2,
        Vector3,
        Color,
        Entity,

        BlackboardVariableType_Count
    };

    extern std::string BlackboardVariableType_Names[];

    using BlackboardValue = std::variant<
        bool,
        uint8,
        int32,
        int64,
        float32,
        std::string,
        Vector2,
        Vector3,
        Color,
        Entity*
    >;

    template<typename T>
    struct BlackboardVariableTypeMapper
    {
        static_assert(false, "Type is not an AllowedBlackboardVariableType");
    };

    template<> struct BlackboardVariableTypeMapper<bool>          { static constexpr BlackboardVariableType value = BlackboardVariableType::Boolean; };
    template<> struct BlackboardVariableTypeMapper<uint8>        { static constexpr BlackboardVariableType value = BlackboardVariableType::Byte; };
    template<> struct BlackboardVariableTypeMapper<int32>        { static constexpr BlackboardVariableType value = BlackboardVariableType::Number; };
    template<> struct BlackboardVariableTypeMapper<int64>        { static constexpr BlackboardVariableType value = BlackboardVariableType::LargeNumber; };
    template<> struct BlackboardVariableTypeMapper<float32>      { static constexpr BlackboardVariableType value = BlackboardVariableType::Float; };
    template<> struct BlackboardVariableTypeMapper<std::string>  { static constexpr BlackboardVariableType value = BlackboardVariableType::String; };
    template<> struct BlackboardVariableTypeMapper<Vector2>      { static constexpr BlackboardVariableType value = BlackboardVariableType::Vector2; };
    template<> struct BlackboardVariableTypeMapper<Vector3>      { static constexpr BlackboardVariableType value = BlackboardVariableType::Vector3; };
    template<> struct BlackboardVariableTypeMapper<Color>        { static constexpr BlackboardVariableType value = BlackboardVariableType::Color; };
    template<> struct BlackboardVariableTypeMapper<Entity*>      { static constexpr BlackboardVariableType value = BlackboardVariableType::Entity; };

    template<typename T>
    constexpr BlackboardVariableType ToBlackboardVariableType()
    {
        return BlackboardVariableTypeMapper<T>::value;
    }

    template<typename T>
    concept AllowedBlackboardVariableType = std::disjunction_v<
        std::is_same<T, bool>,
        std::is_same<T, uint8>,
        std::is_same<T, int32>,
        std::is_same<T, int64>,
        std::is_same<T, float32>,
        std::is_same<T, std::string>,
        std::is_same<T, Vector2>,
        std::is_same<T, Vector3>,
        std::is_same<T, Color>,
        std::is_same<T, Entity*>
    >;

    struct BlackboardVariable
    {
        BlackboardVariableType type;
        BlackboardValue value;

        template<typename T> requires AllowedBlackboardVariableType<T>
        BlackboardVariable(T val)
            : type(ToBlackboardVariableType<T>())
            , value(val)
        {}

        template<typename T> requires AllowedBlackboardVariableType<T>
        [[nodiscard]] T GetValue() const
        {
            if (type != ToBlackboardVariableType<T>())
            {
                WLog::SetConsoleWarning();
                WLog::ConsoleLog("Blackboard Error! Wrong variable type requested!");
                return T();
            }
            return std::get<T>(value);
        }

        template<typename T> requires AllowedBlackboardVariableType<T>
        void SetValue(T& value)
        {
            if (type != ToBlackboardVariableType<T>())
            {
                WLog::SetConsoleWarning();
                WLog::ConsoleLog("Blackboard Error! Type mismatch!");
                return;
            }
            this->value = value;
        }

        [[nodiscard]] BlackboardVariableType GetType() const { return type; }
    };


    struct BlackboardArray
    {
        BlackboardVariableType type;
        wtl::vector<BlackboardVariable> m_values;

        BlackboardArray() = default;

        template<typename T> requires AllowedBlackboardVariableType<T>
        static BlackboardArray Create(uint64 size, T defaultValue = T{})
        {
            BlackboardArray arr;
            arr.type = ToBlackboardVariableType<T>();
            arr.m_values.reserve(size);
            for (uint64 i = 0; i < size; ++i)
                arr.m_values.push_back(defaultValue);
            return arr;
        }

        template<typename T> requires AllowedBlackboardVariableType<T>
        [[nodiscard]] T GetValue(uint64 index) const
        {
            if (type != ToBlackboardVariableType<T>())
            {
                WLog::SetConsoleWarning();
                WLog::ConsoleLog("Blackboard Error! Wrong array type requested!");
                return T();
            }
            if (index >= m_values.size())
            {
                WLog::SetConsoleWarning();
                WLog::ConsoleLog("Blackboard Error! Array index out of bounds!");
                return T();
            }
            return m_values[index].GetValue<T>();
        }

        template<typename T> requires AllowedBlackboardVariableType<T>
        void SetValue(T& value, uint64 index)
        {
            if (type != ToBlackboardVariableType<T>())
            {
                WLog::SetConsoleWarning();
                WLog::ConsoleLog("Blackboard Error! Type mismatch!");
                return;
            }
            if (index >= m_values.size())
            {
                WLog::SetConsoleWarning();
                WLog::ConsoleLog("Blackboard Error! Array index out of bounds!");
                return;
            }
            m_values[index].SetValue(value);
        }

        [[nodiscard]] BlackboardVariableType GetType() const { return type; }
        [[nodiscard]] uint64 Size() const { return m_values.size(); }
    };
}
