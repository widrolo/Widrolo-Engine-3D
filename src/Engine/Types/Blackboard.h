#pragma once

#include "BlackboardVariable.h"

namespace WEngine
{
    using BlackboardEntryVar = std::pair<std::string, BlackboardVariable>;
    using BlackboardEntryArr = std::pair<std::string, BlackboardArray>;
    class Blackboard
    {
    public:
        Blackboard() = default;
        ~Blackboard() = default;

        template<typename T> requires AllowedBlackboardVariableType<T>
        void AddVariable(const std::string& variableName, T initialValue)
        {
            m_variables.push_back({variableName, initialValue});
        }
        template<typename T> requires AllowedBlackboardVariableType<T>
        void AddArray(const std::string& arrayName, const uint64 size, T defaultValue = T{})
        {
            m_arrays.push_back({arrayName, BlackboardArray::Create<T>(size, defaultValue)});
        }

        template<typename T> requires AllowedBlackboardVariableType<T>
        void SetVariable(const std::string& variableName, T value)
        {
            for (auto& var : m_variables)
            {
                if (var.first == variableName)
                {
                    var.second.SetValue(value);
                    return;
                }
            }
            WLog::SetConsoleWarning();
            WLog::ConsoleLog(std::format("Blackboard Error! Could not find variable \"{}\"", variableName));
        }

        template<typename T> requires AllowedBlackboardVariableType<T>
        void SetArrayVariable(const std::string& arrayName, uint32 index, T value)
        {
            for (auto& var : m_arrays)
            {
                if (var.first == arrayName)
                {
                    var.second.SetValue(value, index);
                    return;
                }
            }
            WLog::SetConsoleWarning();
            WLog::ConsoleLog(std::format("Blackboard Error! Could not find variable \"{}\"", arrayName));
        }

        template<typename T> requires AllowedBlackboardVariableType<T>
        T GetVariable(const std::string& variableName) const
        {
            for (const auto& var : m_variables)
            {
                if (var.first == variableName)
                    return var.second.GetValue<T>();
            }
            WLog::SetConsoleWarning();
            WLog::ConsoleLog(std::format("Blackboard Error! Could not find variable \"{}\"", variableName));
            return T{};
        }

        template<typename T> requires AllowedBlackboardVariableType<T>
        T GetArrayVariable(const std::string& arrayName, uint32 index) const
        {
            for (const auto& var : m_arrays)
            {
                if (var.first == arrayName)
                    return var.second.GetValue<T>(index);
            }
            WLog::SetConsoleWarning();
            WLog::ConsoleLog(std::format("Blackboard Error! Could not find variable \"{}\"", arrayName));
            return T{};
        }

    private:
        wtl::vector<BlackboardEntryVar> m_variables;
        wtl::vector<BlackboardEntryArr> m_arrays;
    };
}