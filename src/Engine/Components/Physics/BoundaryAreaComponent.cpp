#include "BoundaryAreaComponent.h"
#include <Engine/Core/System/Memory.h>
#include <Engine/Types/SpawnArgs.h>
#include <Engine/Core/World/Entity.h>

using namespace WEngine;

REGISTER_COMPONENT(BoundaryAreaComponent);

BoundaryAreaComponent::BoundaryAreaComponent(Entity *e)
{
    COMP_SETUP("Boundary Area Component");
}

BoundaryAreaComponent::~BoundaryAreaComponent()
{

}

void BoundaryAreaComponent::Awake(ComponentArgs ca)
{

}

void BoundaryAreaComponent::Start()
{

}
