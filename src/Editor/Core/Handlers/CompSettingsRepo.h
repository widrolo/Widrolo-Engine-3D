#pragma once

#include <array>
#include <Engine/Types/CommonTypes.h>
#include <Editor/Types/ComponentSettingDefinition.h>

namespace WEditor
{
    class CompSettingsRepo
    {
    public:
        CompSettingsRepo();
        ~CompSettingsRepo();

    private:
        static constexpr uint16 MAX_COMPONENTS = 4096;
        _GLOBAL_ std::array<ComponentSettingDefinition, MAX_COMPONENTS> m_settingDefs;

    public:
        void Init();
        static uint8 GetSettingCapacity(uint64 ID);
        static ComponentOptionType GetSettingType(uint64 ID, uint8 settingNumber);
        static std::string GetSettingName(uint64 ID);
        static std::string GetSettingDesc(uint64 ID);
        static const wtl::vector<ComponentOption> GetInternalOptions(uint64 ID);
        static std::array<ComponentSettingDefinition, MAX_COMPONENTS>& GetAllComponentSettings();

    private:
        ComponentOptionType StringToCompOptType(std::string type);
    };

}

