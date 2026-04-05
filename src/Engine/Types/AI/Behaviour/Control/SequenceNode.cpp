#include "SequenceNode.h"

using namespace WEngine::Behaviour;

BehaviourExecutionStatus SequenceNode::Execute()
{
    bool anyRunning = false;
    for (auto& child : m_children)
    {
        switch (child->Execute())
        {
            case BehaviourExecutionStatus::Failed:
                return BehaviourExecutionStatus::Failed;
            case BehaviourExecutionStatus::Succeeded:
                continue;
            case BehaviourExecutionStatus::Running:
                anyRunning = true;
                continue;
        }
    }

    if (!anyRunning)
        return BehaviourExecutionStatus::Running;
    return BehaviourExecutionStatus::Succeeded;
}