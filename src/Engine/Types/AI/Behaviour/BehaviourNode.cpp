#include "BehaviourNode.h"

#include <algorithm>

using namespace WEngine::Behaviour;

void BehaviourNode::SpawnNode(ComponentArgs& ca)
{

}

void BehaviourNode::AttachChild(BehaviourNode *child)
{
    if (child == nullptr)
        return;

    m_children.push_back(child);
}

void BehaviourNode::AttachChildren(const wtl::vector<BehaviourNode*>& children)
{
    for (auto& child : children)
        AttachChild(child);
}

void BehaviourNode::DetachChild(uint64 ID)
{
    std::remove_if(m_children.begin(), m_children.end(), [ID](BehaviourNode* child)
    {
        return ID == child->m_ID;
    });
}

void BehaviourNode::DetachChild(const BehaviourNode *ptrRef)
{
    std::remove_if(m_children.begin(), m_children.end(), [ptrRef](BehaviourNode* child)
    {
        return ptrRef == child;
    });
}
