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
    WEngine::Nullable<WEngine::Vector3> resVector3{};
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
            case ComponentOptionType::Vector3:
                resVector3 = args.GetVector3FromParams(options[i].optionInternal);
                if (resVector3.HasValue())
                    m_data[i] = resVector3.GetValue();
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
    //simulatableObject.SetPosition(entity->transform.position);
    simulatableObject.SetFreezeState(true);
    auto lines = WEngine::PhysicsVisualizer::GetCircleVisual(simulatableObject);

    WEngine::RenderVisualizationMission mission{};
    mission.lines = lines;
    mission.color = WEngine::Color::Green;

    //WEngine::CoreSystems::GetRenderHandler()->AddToVisualizationQueue(mission);
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
    //simulatableObject.SetPosition(entity->transform.position);
    simulatableObject.SetFreezeState(true);
    auto lines = WEngine::PhysicsVisualizer::GetRectVisual(simulatableObject);

    WEngine::RenderVisualizationMission mission{};
    mission.lines = lines;
    mission.color = WEngine::Color::Green;

    //WEngine::CoreSystems::GetRenderHandler()->AddToVisualizationQueue(mission);
}
