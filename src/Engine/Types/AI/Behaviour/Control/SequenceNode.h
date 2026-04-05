#pragma once

#include "Engine/Types/AI/Behaviour/BehaviourNode.h"

namespace WEngine::Behaviour
{
    class SequenceNode : public BehaviourNode
    {
    public:
        using BehaviourNode::BehaviourNode;
    public:
        BehaviourExecutionStatus Execute() override;
    };
}