#include "Animation.h"

#include "Engine/Core/Handlers/AssetRepo.h"
#include "Engine/Types/AssetMission.h"
#include "Engine/Types/CoreSystems.h"
#include "Engine/Util/Log.h"

using namespace WEngine;

Nullable<Animation> AnimationInformation::GetAnimation(const std::string &archetype, const std::string &subtype,
    const std::string &name) const
{
    for (const auto& arch : m_archetypes)
    {
        if (arch.name != archetype)
            continue;
        for (const auto& sub : arch.subtypes)
        {
            if (sub.name != subtype)
                continue;
            for (const auto& anim : sub.animations)
            {
                if (anim.name == name)
                    return Nullable<Animation>(anim);
            }
            return Nullable<Animation>();
        }
        return Nullable<Animation>();
    }
    return Nullable<Animation>();
}

AnimationInformation AnimationParser::ParseAnimation(const std::string &animName)
{
    AtlasInfoAssetMission mission{};
    mission.name = animName;
    CoreSystems::GetAssetRepo()->GetAsset(mission);


    AnimationInformation animOut{};
    animOut.m_animationName = animName;

    if (mission.root.IsNull())
    {
        animOut.m_animationName = "";
        return animOut;
    }

    uint32 expectedAnimationCount = 0;
    wtl::vector<Animation> animationCache;

    ParseCuts(mission.root, animOut);
    ParseAnimInfo(mission.root, animOut, expectedAnimationCount);

    return animOut;
}

void AnimationParser::ParseCuts(const YAML::Node& root, AnimationInformation& info)
{
    auto atlasCuts = root["atlas"];
    info.m_hFramesCount = atlasCuts["h_frames"].as<uint16>();
    info.m_vFramesCount = atlasCuts["v_frames"].as<uint16>();
}

void AnimationParser::ParseAnimBase(const YAML::Node &root, wtl::vector<Animation> &animCache, uint32 expected)
{
    animCache.resize(expected);
    auto animations = root["anims"];

    for (const auto& animDef : animations)
    {
        const YAML::Node& node = animDef.second;
        Animation animation;
        animation.name = node["name"].as<std::string>();
        animation.startFrame = node["range"][0].as<uint16>();
        animation.endFrame = node["range"][1].as<uint16>();

        animCache.push_back(animation);
    }
}

void AnimationParser::ParseAnimInfo(const YAML::Node &root, AnimationInformation &info, uint32 &expected)
{
    auto animInfo = root["animation-info"];
    info.m_framerate = animInfo["framerate"].as<uint8>();

    auto archetypes = animInfo["archetypes"];

    for (const auto& arch_kv : archetypes)
    {
        const YAML::Node& arch = arch_kv.second;
        AnimationInformation::AnimationArchetype archetype{};
        archetype.name = arch["name"].as<std::string>();

        for (const auto& subt_kv : arch)
        {
            const YAML::Node& subt = subt_kv.second;

            if (!subt.IsMap()) continue;

            AnimationInformation::AnimationSubtype subtype{};
            subtype.name = subt["name"].as<std::string>();

            for (const auto& anim_kv : subt)
            {
                const YAML::Node& anim = anim_kv.second;
                if (!anim.IsMap()) continue;

                Animation animation{};
                animation.name = anim["name"].as<std::string>();
                animation.startFrame = anim["range"][0].as<uint16>();
                animation.endFrame = anim["range"][1].as<uint16>();
                if (animation.endFrame == 0 && expected != 0)
                    animation.frameCount = 0;
                else
                    animation.frameCount = animation.endFrame - animation.startFrame + 1;
                subtype.animations.push_back(animation);
                expected++;
            }

            archetype.subtypes.push_back(subtype);
        }
        info.m_archetypes.push_back(archetype);
    }
}
