#include "CylinderAreaComponent.h"
#include <Engine/Core/System/Memory.h>
#include <Engine/Types/SpawnArgs.h>
#include <Engine/Core/World/Entity.h>

using namespace WEngine;

REGISTER_COMPONENT(CylinderAreaComponent);

CylinderAreaComponent::CylinderAreaComponent(Entity *e)
{
    COMP_SETUP("Cylindrical Area Component");
}

CylinderAreaComponent::~CylinderAreaComponent()
{

}

void CylinderAreaComponent::Awake(ComponentArgs ca)
{

}

void CylinderAreaComponent::Start()
{

}
