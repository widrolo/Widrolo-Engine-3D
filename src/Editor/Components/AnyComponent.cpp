#include "AnyComponent.h"

#include <Engine/Core/World/Entity.h>
#include <Engine/Types/SpawnArgs.h>

#include <Editor/Types/ComponentSettingDefinition.h>

#include "Editor/Core/Handlers/CompSettingsRepo.h"
#include "Editor/Types/EditorSystems.h"
#include "Engine/Core/Handlers/AssetRepo.h"
#include "Engine/Core/Handlers/RenderHandler.h"
#include "Engine/Core/Physics/PhysicsVisualizer.h"
#include "Engine/Math/Shapes/Shapes2D.h"
#include "Engine/Types/AssetMission.h"
#include "Engine/Types/CoreSystems.h"
#include "Engine/Types/Rendering/Atlas.h"
#include "Engine/Types/Rendering/Sprite.h"
#include "Engine/Util/Log.h"

namespace WEngine {
    struct RenderMissionSprite;
}

using namespace WEditor;

// we dont need to register this component, as the editor is hardcoded to spawn this
// component only.

AnyComponent::AnyComponent(WEngine::Entity *e)
{
    COMP_SETUP("AnyComponent")
}

void AnyComponent::Init(uint16 ID, uint8 dataSize, WEngine::ComponentArgs args)
{
    Init(ID, dataSize);

    ApplySpawnData(args);
}

void AnyComponent::Init(uint16 ID, uint8 dataSize)
{
    m_ID = ID;
    m_data.resize(dataSize);

    if (dataSize == 0)
        return;

    for (int i = 0; i < dataSize; i++)
    {
        auto type = EditorSystems::GetCompSettingsRepo()->GetSettingType(ID, i);
        switch (type)
        {
            case ComponentOptionType::Number:
                m_data[i] = 0;
                break;
            case ComponentOptionType::Float:
                m_data[i] = 0.0f;
                break;
            case ComponentOptionType::Bool:
                m_data[i] = false;
                break;
            case ComponentOptionType::String:
                m_data[i] = std::string("");
                break;
            case ComponentOptionType::Vector2:
                m_data[i] = WEngine::Vector2(0.0f, 0.0f);
                break;
            case ComponentOptionType::Color:
                m_data[i] = WEngine::Color(255, 255, 255, 255);
                break;
            default:
                break;
        }
    }
}

void AnyComponent::SetData(uint8 location, OptionData data)
{
    m_data[location] = data;
}

void AnyComponent::ApplySpawnData(WEngine::ComponentArgs args)
{
    auto options = EditorSystems::GetCompSettingsRepo()->GetInternalOptions(m_ID);

    WEngine::Nullable<int64> resInt{};
    WEngine::Nullable<float32> resFloat{};
    WEngine::Nullable<bool> resBool{};
    WEngine::Nullable<std::string> resString{};
    WEngine::Nullable<WEngine::Vector2> resVector{};
    WEngine::Nullable<WEngine::Color> resColor{};


    for (int i = 0; i < options.size(); i++)
    {
        auto type = EditorSystems::GetCompSettingsRepo()->GetSettingType(m_ID, i);

        switch (type)
        {
            case ComponentOptionType::Number:
                resInt = args.GetIntFromParams(options[i].optionInternal);
                if (resInt.HasValue())
                    m_data[i] = (int32)resInt.GetValue();
                break;
            case ComponentOptionType::Float:
                resFloat = args.GetFloatFromParams(options[i].optionInternal);
                if (resFloat.HasValue())
                    m_data[i] = resFloat.GetValue();
                break;
            case ComponentOptionType::Bool:
                resBool = args.GetBoolFromParams(options[i].optionInternal);
                if (resBool.HasValue())
                    m_data[i] = resBool.GetValue();
                break;
            case ComponentOptionType::String:
                resString = args.GetStringFromParams(options[i].optionInternal);
                if (resString.HasValue())
                    m_data[i] = resString.GetValue();
                break;
            case ComponentOptionType::Vector2:
                resVector = args.GetVector2FromParams(options[i].optionInternal);
                if (resVector.HasValue())
                    m_data[i] = resVector.GetValue();
                break;
            case ComponentOptionType::Color:
                resColor = args.GetColorFromParams(options[i].optionInternal);
                if (resColor.HasValue())
                    m_data[i] = resColor.GetValue();
                break;
            default:
                break;
        }

    }
}

OptionData AnyComponent::GetData(uint8 location)
{
    return m_data[location];
}

WEngine::Nullable<OptionData> AnyComponent::FindDataByName(const std::string &name)
{
    auto options = EditorSystems::GetCompSettingsRepo()->GetInternalOptions(m_ID);
    for (int i = 0; i < options.size(); i++)
    {
        if (options[i].optionInternal == name)
            return WEngine::Nullable<OptionData>(m_data[i]);
    }
    return WEngine::Nullable<OptionData>();
}

void AnyComponent::Draw()
{
    TryDrawGameGraphics();
    TryDrawDebugGraphics();

    DrawOnSelected();
}

void AnyComponent::TryDrawGameGraphics()
{
    switch (m_ID)
    {
        case 2: // SpriteRendererComponent
            GFX_Game_DrawSprite();
            break;
        case 8: // AnimationRendererComponent
            GFX_Game_DrawAnim();
            break;
        default: break;
    }
}

void AnyComponent::TryDrawDebugGraphics()
{
    switch (m_ID)
    {
        case 10: // CircleAreaComponent
            GFX_Dbg_CircleArea();
            break;
        case 11: // RectAreaComponent
            GFX_Dbg_RectArea();
            break;
        default: break;
    }
}

void AnyComponent::DrawOnSelected()
{

}

void AnyComponent::GFX_Game_DrawSprite()
{
    std::string spriteName;
    float32 scale;
    bool isAtlas;
    int32 atlasFrame;

    auto spriteNameNullable = FindDataByName("spriteName");
    auto scaleNullable = FindDataByName("scale");
    auto isAtlasNullable = FindDataByName("isAtlas");
    auto atlasFrameNullable = FindDataByName("atlasFrame");

    if (spriteNameNullable.HasValue())
        spriteName = std::get<std::string>(spriteNameNullable.GetValue());
    else
        return; // No sprite thats why

    if (scaleNullable.HasValue())
        scale = std::get<float>(scaleNullable.GetValue());
    else
        scale = 1.0f;

    if (isAtlasNullable.HasValue())
        isAtlas = std::get<bool>(isAtlasNullable.GetValue());
    else
        isAtlas = false;

    if (atlasFrameNullable.HasValue())
        atlasFrame = std::get<int>(atlasFrameNullable.GetValue());
    else
        atlasFrame = 0;

    WEngine::SpriteAssetMission mission{};
    mission.name = spriteName;

    if (!isAtlas)
    {
        if (!m_isInitializedDraw)
        {
            WEngine::CoreSystems::GetAssetRepo()->GetAsset(mission);
            sprite = mission.sprite;
            m_isInitializedDraw = true;
        }
        WEngine::Rectangle rect{};
        rect.p1 = entity->transform.position;
        rect.p2 = sprite.GetSize() * scale;

        WEngine::RenderMission mission{};
        mission.layer = (uint8) WEngine::RenderLayer::Default;
        mission.quadBounds = rect;
        mission.shaderSettings.push_back({WEngine::ShaderSettingType::Color, WEngine::Color::White, "u_Color"});
        mission.shaderSettings.push_back({WEngine::ShaderSettingType::Vec2, WEngine::Vector2(-1, -1), "u_flip"});
        mission.shaderSettings.push_back({WEngine::ShaderSettingType::Texture, sprite.GetTexture(), "u_texture"});

        WEngine::CoreSystems::GetRenderHandler()->AddToRenderQueue(mission);
    }
    else
    {
        if (!m_isInitializedDraw)
        {
            WEngine::CoreSystems::GetAssetRepo()->GetAsset(mission);
            atlas.LoadAtlas(mission.sprite, spriteName);
            m_isInitializedDraw = true;
        }
        WEngine::Rectangle rect{};
        rect.p1 = entity->transform.position;
        rect.p2 = atlas.GetSizeSingle() * scale;

        auto uvs = atlas.GetFrameUVCoords(atlasFrame);
        WEngine::Vector4 uvCoords{uvs[0].x, uvs[0].y, uvs[1].x, uvs[1].y};

        WEngine::RenderMission mission{};
        mission.layer = (uint8)WEngine::RenderLayer::Default;
        mission.shader = "atlas";
        mission.quadBounds = rect;
        mission.shaderSettings.push_back({WEngine::ShaderSettingType::Color, WEngine::Color::White, "u_Color"});
        mission.shaderSettings.push_back({WEngine::ShaderSettingType::Vec2, WEngine::Vector2(-1, -1), "u_flip"});
        mission.shaderSettings.push_back({WEngine::ShaderSettingType::Vec4, uvCoords, "u_AtlasUV"});
        mission.shaderSettings.push_back({WEngine::ShaderSettingType::Texture, atlas.GetTexture(), "u_texture"});

        WEngine::CoreSystems::GetRenderHandler()->AddToRenderQueue(mission);
    }
}

void AnyComponent::GFX_Game_DrawAnim()
{
    std::string spriteName;
    float32 scale;
    WEngine::Vector2 offset;
    int32 atlasFrame = 1; // Sometimes, the zero sprite may be the missing one.

    auto spriteNameNullable = FindDataByName("atlasName");
    auto scaleNullable = FindDataByName("scale");
    auto offsetNullable = FindDataByName("offset");

    if (spriteNameNullable.HasValue())
        spriteName = std::get<std::string>(spriteNameNullable.GetValue());
    else
        return; // No sprite thats why

    if (scaleNullable.HasValue())
        scale = std::get<float32>(scaleNullable.GetValue());
    else
        scale = 1.0f;

    if (offsetNullable.HasValue())
        offset = std::get<WEngine::Vector2>(offsetNullable.GetValue());
    else
        offset = WEngine::Vector2::Zero;

    WEngine::SpriteAssetMission mission{};
    mission.name = spriteName;
    if (!m_isInitializedDraw)
    {
        WEngine::CoreSystems::GetAssetRepo()->GetAsset(mission);
        atlas.LoadAtlas(mission.sprite, spriteName);
        m_isInitializedDraw = true;
    }
    WEngine::Rectangle rect{};
    rect.p1 = entity->transform.position + offset;
    rect.p2 = atlas.GetSizeSingle() * scale;

    auto uvs = atlas.GetFrameUVCoords(atlasFrame);
    WEngine::Vector4 uvCoords{uvs[0].x, uvs[0].y, uvs[1].x, uvs[1].y};

    WEngine::RenderMission animMission{};
    animMission.layer = (uint8)WEngine::RenderLayer::Default;
    animMission.shader = "atlas";
    animMission.quadBounds = rect;
    animMission.shaderSettings.push_back({WEngine::ShaderSettingType::Color, WEngine::Color::White, "u_Color"});
    animMission.shaderSettings.push_back({WEngine::ShaderSettingType::Vec2, WEngine::Vector2(-1, -1), "u_flip"});
    animMission.shaderSettings.push_back({WEngine::ShaderSettingType::Vec4, uvCoords, "u_AtlasUV"});
    animMission.shaderSettings.push_back({WEngine::ShaderSettingType::Texture, atlas.GetTexture(), "u_texture"});

    WEngine::CoreSystems::GetRenderHandler()->AddToRenderQueue(animMission);
}

void AnyComponent::GFX_Dbg_CircleArea()
{
    float32 radius;
    WEngine::Vector2 offset;

    auto radiusNullable = FindDataByName("areaRadius");
    auto offsetNullable = FindDataByName("offset");

    if (radiusNullable.HasValue())
        radius = std::get<float32>(radiusNullable.GetValue());
    else
        radius = 0.0f;

    if (offsetNullable.HasValue())
        offset = std::get<WEngine::Vector2>(offsetNullable.GetValue());
    else
        offset = WEngine::Vector2::Zero;

    simulatableObject.SetAreaCircle(radius);
    simulatableObject.SetAreaOffset(offset);
    simulatableObject.SetOwner(entity);
    simulatableObject.SetPosition(entity->transform.position);
    simulatableObject.SetFreezeState(true);
    auto lines = WEngine::PhysicsVisualizer::GetCircleVisual(simulatableObject);

    WEngine::RenderVisualizationMission mission{};
    mission.lines = lines;
    mission.color = WEngine::Color::Green;

    WEngine::CoreSystems::GetRenderHandler()->AddToVisualizationQueue(mission);
}

void AnyComponent::GFX_Dbg_RectArea()
{
    WEngine::Vector2 size;
    WEngine::Vector2 offset;

    auto sizeNullable = FindDataByName("areaSize");
    auto offsetNullable = FindDataByName("offset");

    if (sizeNullable.HasValue())
        size = std::get<WEngine::Vector2>(sizeNullable.GetValue());
    else
        size = WEngine::Vector2::Zero;

    if (offsetNullable.HasValue())
        offset = std::get<WEngine::Vector2>(offsetNullable.GetValue());
    else
        offset = WEngine::Vector2::Zero;

    simulatableObject.SetAreaRect(size);
    simulatableObject.SetAreaOffset(offset);
    simulatableObject.SetOwner(entity);
    simulatableObject.SetPosition(entity->transform.position);
    simulatableObject.SetFreezeState(true);
    auto lines = WEngine::PhysicsVisualizer::GetRectVisual(simulatableObject);

    WEngine::RenderVisualizationMission mission{};
    mission.lines = lines;
    mission.color = WEngine::Color::Green;

    WEngine::CoreSystems::GetRenderHandler()->AddToVisualizationQueue(mission);
}
