#include "SelectorNode.h"

using namespace WEngine::Behaviour;

BehaviourExecutionStatus SelectorNode::Execute()
{
    for (auto& child : m_children)
    {
        switch (child->Execute())
        {
            case BehaviourExecutionStatus::Failed:
                continue;
            case BehaviourExecutionStatus::Succeeded:
                return BehaviourExecutionStatus::Succeeded;
            case BehaviourExecutionStatus::Running:
                return BehaviourExecutionStatus::Running;
        }
    }
    return BehaviourExecutionStatus::Failed;
}
