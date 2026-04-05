#include "CompSettingsRepo.h"

#include <Engine/Types/CoreSystems.h>
#include <Engine/Types/AssetMission.h>
#include <Engine/Core/Handlers/AssetRepo.h>

using namespace WEditor;

CompSettingsRepo::CompSettingsRepo()
{
    Init();
}

CompSettingsRepo::~CompSettingsRepo()
{

}

void CompSettingsRepo::Init()
{
    auto assetRepo = WEngine::CoreSystems::GetAssetRepo();
    WEngine::YamlAssetMission mission{};

    mission.name = "../CompDef/BaseComponents";
    assetRepo->GetAsset(mission);

    const YAML::Node comps = mission.root["components"];

    for (const auto compNode : comps)
    {
        const YAML::Node comp = compNode.second;
        uint32 ID = comp["ID"].as<uint32>();
        std::string name = comp["Name"].as<std::string>();
        std::string desc = comp["Description"].as<std::string>();

        m_settingDefs[ID].componentName = name;
        m_settingDefs[ID].componentDesctription = desc;

        const YAML::Node settings = comp["Settings"];

        if (!settings)
            continue;

        for (const auto settNode : settings)
        {
            const YAML::Node sett = settNode.second;
            std::string settName = sett["Name"].as<std::string>();
            std::string settDesc = sett["Description"].as<std::string>();
            std::string settInter = sett["Internal"].as<std::string>();
            auto type = StringToCompOptType(sett["Type"].as<std::string>());

            ComponentOption option{};
            option.optionName = settName;
            option.optionDesctription = settDesc;
            option.optionInternal = settInter;
            option.type = type;
            m_settingDefs[ID].options.push_back(option);
        }
    }

    return;
}

uint8 CompSettingsRepo::GetSettingCapacity(uint64 ID)
{
    return (uint8)m_settingDefs[ID].options.size();
}

ComponentOptionType CompSettingsRepo::GetSettingType(uint64 ID, uint8 settingNumber)
{
    return m_settingDefs[ID].options[settingNumber].type;
}

std::string CompSettingsRepo::GetSettingName(uint64 ID)
{
    return m_settingDefs[ID].componentName;
}

std::string CompSettingsRepo::GetSettingDesc(uint64 ID)
{
    return m_settingDefs[ID].componentDesctription;
}

const wtl::vector<ComponentOption> CompSettingsRepo::GetInternalOptions(uint64 ID)
{
    return m_settingDefs[ID].options;
}

ComponentOptionType CompSettingsRepo::StringToCompOptType(std::string type)
{
    const std::unordered_map<std::string, ComponentOptionType> lookup = {
        {"None",    ComponentOptionType::None},
        {"Number",  ComponentOptionType::Number},
        {"Float",   ComponentOptionType::Float},
        {"Bool",    ComponentOptionType::Bool},
        {"String",  ComponentOptionType::String},
        {"Vector2", ComponentOptionType::Vector2},
        {"Color",   ComponentOptionType::Color}
    };

    auto it = lookup.find(type);
    if (it != lookup.end())
        return it->second;

    return ComponentOptionType::None;
}

