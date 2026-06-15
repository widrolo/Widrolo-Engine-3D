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
    auto matN = ca.GetStringFromParams("materialName");

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

    if (matN.HasValue())
    {
        auto matNN = Iris::GetMaterial(matN.GetValue());
        if (!matNN.HasValue())
        {
            matNN = Iris::ALLOC_CompileMaterial(matN.GetValue());
            if (matNN.HasValue())
                m_material = matNN.GetValue();
        }
        else
        {
            m_material = matNN.GetValue();
        }
    }
}

void MeshRendererComponent::LateAwake()
{
    if (entity->IsStationary() && m_model != 0 && m_material != 0)
    {
        m_isStationary = true;
        CoreSystems::GetRenderHandler()->RecordStationaryAdd(m_model, m_material, entity->transform);
    }
}

void MeshRendererComponent::Draw()
{
    RenderMission mission;
    mission.transform = entity->transform;
    mission.model = m_model;
    mission.material = m_material;
    mission.isStationary = m_isStationary;

    CoreSystems::GetRenderHandler()->AddToRenderQueue(mission);

}
