#include "MeshRendererComponent.h"

#include <Engine/Types/Rendering/VertextData.h>
#include "Engine/Core/Handlers/AssetRepo.h"
#include "Engine/Core/Handlers/RenderHandler.h"
#include "Engine/Core/System/Iris.h"
#include "Engine/Types/SpawnArgs.h"
#include "Engine/Core/World/Entity.h"
#include "Engine/Types/AssetMission.h"
#include "Engine/Types/CoreSystems.h"
#include "Engine/Types/Rendering/RenderMission.h"
#include "Engine/Util/Log.h"

using namespace WEngine;

REGISTER_COMPONENT(MeshRendererComponent)

MeshRendererComponent::MeshRendererComponent(Entity *e)
{
    COMP_SETUP("MeshRendererComponent")
}

void MeshRendererComponent::Awake(ComponentArgs ca)
{
    auto modelN = ca.GetStringFromParams("meshName");
    auto shaderN = ca.GetStringFromParams("shaderName");
    auto instN = ca.GetBoolFromParams("instance");

    if (modelN.HasValue())
    {
        auto modelNN = Iris::GetModel(modelN.GetValue());
        if (!modelNN.HasValue())
        {
            MeshAssetMission mission;
            mission.name = modelN.GetValue();
            CoreSystems::GetAssetRepo()->GetAsset(mission);

            modelNN = Iris::ALLOC_CreateModel(mission.model);
            if (modelNN.HasValue())
                m_model = modelNN.GetValue();
        }
        else
        {
            m_model = modelNN.GetValue();
        }
    }

    if (shaderN.HasValue())
    {
        auto shaderNN = Iris::GetShader(shaderN.GetValue());
        if (!shaderNN.HasValue())
        {
            shaderNN = Iris::ALLOC_CompileShader(shaderN.GetValue());
            if (shaderNN.HasValue())
                m_shader = shaderNN.GetValue();
        }
        else
        {
            m_shader = shaderNN.GetValue();
        }
    }
}

void MeshRendererComponent::LateAwake()
{
    if (entity->IsStationary() && m_model != 0 && m_shader != 0)
    {
        m_isStationary = true;
        CoreSystems::GetRenderHandler()->RecordStationaryAdd(m_model, m_shader, entity->transform);
    }
}

void MeshRendererComponent::Draw()
{
    RenderMission mission;
    mission.transform = entity->transform;
    mission.model = m_model;
    mission.shader = m_shader;
    mission.isStationary = m_isStationary;

    CoreSystems::GetRenderHandler()->AddToRenderQueue(mission);

}
