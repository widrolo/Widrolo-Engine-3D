#pragma once
#include "Engine/Types/AI/Behaviour/BehaviourNode.h"

namespace WEngine::Behaviour
{

    class ParallelNode : public BehaviourNode
    {
    public:
        using BehaviourNode::BehaviourNode;

    public:
        BehaviourExecutionStatus Execute() override;
    };
}

