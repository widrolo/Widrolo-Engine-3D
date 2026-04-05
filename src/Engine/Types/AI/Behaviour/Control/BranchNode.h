#pragma once

#include "Engine/Types/BlackboardVariable.h"
#include "Engine/Types/AI/Behaviour/BehaviourNode.h"

namespace WEngine::Behaviour
{
    class BranchNode : public BehaviourNode
    {
    public:
        using BehaviourNode::BehaviourNode;
    public:
        BehaviourExecutionStatus Execute() override;
        void SpawnNode(ComponentArgs &ca) override;

    private:
        BehaviourExecutionStatus CheckBool();
        BehaviourExecutionStatus CheckByte();
        BehaviourExecutionStatus CheckNumber();
        BehaviourExecutionStatus CheckLargeNumber();
        BehaviourExecutionStatus CheckFloat();
        BehaviourExecutionStatus CheckString();
        BehaviourExecutionStatus CheckVector2();
        BehaviourExecutionStatus CheckVector3();
        BehaviourExecutionStatus CheckColor();
        BehaviourExecutionStatus CheckEntity();


    private:
        std::string m_variableName;
        std::string m_trueVar;
    };
}