#include "BehaviourTree.h"

#include <Engine/Core/World/Entity.h>

using namespace WEngine::Behaviour;

BehaviourTree::BehaviourTree(Entity* owner)
{
    m_blackboard.AddVariable("self", owner);
}

void BehaviourTree::TickBrain()
{
    m_root->Execute();
}
