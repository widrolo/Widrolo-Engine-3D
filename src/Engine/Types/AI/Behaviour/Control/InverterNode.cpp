#include "InverterNode.h"

using namespace WEngine::Behaviour;

BehaviourExecutionStatus InverterNode::Execute()
{
    for (auto& child : m_children)
    {
        switch (child->Execute())
        {
            case BehaviourExecutionStatus::Failed:
                return BehaviourExecutionStatus::Succeeded;
            case BehaviourExecutionStatus::Succeeded:
                return BehaviourExecutionStatus::Failed;
            case BehaviourExecutionStatus::Running:
                return BehaviourExecutionStatus::Running;
        }
    }
    return BehaviourExecutionStatus::Succeeded;
}