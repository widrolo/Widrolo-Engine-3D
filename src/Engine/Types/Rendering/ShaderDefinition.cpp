#include "ShaderDefinition.h"

#include "Engine/Util/Log.h"

using namespace WEngine;

void ShaderDefinition::Parse(const YAML::Node &root)
{
    // tessellation and geometry shading isnt supported by the renderer, therefore its skipped for now.
    // as said in the spec, no compute either

    if (!root["shader"])
    {
        WLog::SetConsoleError();
        WLog::ConsoleLog("Failed to load shader definition!");
        return;
    }

    const YAML::Node shader = root["shader"];

    // we need to at least have the name to give a better error is something is missing later.
    if (!shader["shaderName"])
    {
        WLog::SetConsoleError();
        WLog::ConsoleLog("Shader definition name not present!");
        return;
    }

    name = shader["shaderName"].as<std::string>();

    // some checks to see if the crucial stuff is present.
    if (!shader["vertexCode"] || !shader["fragmentCode"] || !shader["fragmentInfo"] || !shader["abundance"])
    {
        WLog::SetConsoleError();
        WLog::ConsoleLog(std::format("Shader \"{}\" is missing one of the following fields:\n"
            "\t vertexCode, fragmentCode, fragmentInfo, abundance", name));
        return;
    }

    vertexCodeName = shader["vertexCode"].as<std::string>();
    fragmentCodeName = shader["fragmentCode"].as<std::string>();
    abundance = shader["abundance"].as<uint8>();
    if (abundance > 3)
    {
        WLog::SetConsoleError();
        WLog::ConsoleLog(std::format("Shader sanity test tripped in \"{}\":\n"
            "\t abundance out of range, max allowed is 3!", name));
        return;
    }

    if (shader["depthTest"])
        depthTest = shader["depthTest"].as<bool>();

    const YAML::Node fragInfoNode = shader["fragmentInfo"];

    // some checks to see if the crucial stuff is present. We don't check for texture presence yet, as it may take zero.
    if (!fragInfoNode["expectTextureCount"] || !fragInfoNode["expectParams"])
    {
        WLog::SetConsoleError();
        WLog::ConsoleLog(std::format("Shader fragment info \"{}\" is missing one of the following fields:\n"
            "\t expectTextureCount, expectParams.", name));
        return;
    }

    fragInfo.expectTextureCount = fragInfoNode["expectTextureCount"].as<uint8>();

    if (fragInfo.expectTextureCount != 0)
    {
        // since we have textures now, we have to ensure proper sanity checks.
        if (!fragInfoNode["colorTextures"] || !fragInfoNode["pbrTextures"] || !fragInfoNode["expectChannelNames"])
        {
            WLog::SetConsoleError();
            WLog::ConsoleLog(std::format("Shader fragment info \"{}\" is missing one of the following fields:\n"
                "\t colorTextures, pbrTextures, expectChannelNames.", name));
            return;
        }

        uint8 sanityTextureCount = 0;
        for (const auto& texture : fragInfoNode["colorTextures"])
        {
            fragInfo.colorTextures.push_back(texture.as<std::string>());
            sanityTextureCount++;
        }

        for (const auto& texture : fragInfoNode["pbrTextures"])
        {
            fragInfo.pbrTextures.push_back(texture.as<std::string>());
            sanityTextureCount++;
        }

        for (const auto& chanName : fragInfoNode["expectChannelNames"])
        {
            fragInfo.expectChannelNames.push_back(chanName.as<std::string>());
        }

        if (fragInfo.expectTextureCount != sanityTextureCount)
        {
            WLog::SetConsoleError();
            WLog::ConsoleLog(std::format("Shader sanity test tripped in \"{}\":\n"
                "\t texture name count not equal to expected texture count!", name));
            return;
        }
    }

    for (const auto& param : fragInfoNode["expectParams"])
    {
        const YAML::Node name = param.first;
        const YAML::Node data = param.second;

        ShaderSettingType type;
        const std::string typeStr = data.as<std::string>();
        if (typeStr == "float") type = ShaderSettingType::Float;
        else if (typeStr == "int") type = ShaderSettingType::Int;
        else if (typeStr == "vec2") type = ShaderSettingType::Vec2;
        else if (typeStr == "vec3") type = ShaderSettingType::Vec3;
        else if (typeStr == "vec4") type = ShaderSettingType::Vec4;
        else if (typeStr == "mat4") type = ShaderSettingType::Matrix4;
        else
        {
            WLog::SetConsoleError();
            WLog::ConsoleLog(std::format("Shader \"{}\" definition parsing failed:\n"
                "\t Unexpected type \"{}\" in expectParams at \"{}\"!", this->name, typeStr, name.as<std::string>()));
            return ;
        }

        fragInfo.expectedParams.push_back({name.as<std::string>(), type});
    }

    valid = true;
}