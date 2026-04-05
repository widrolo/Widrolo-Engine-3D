#include "ParallelNode.h"

using namespace WEngine::Behaviour;

BehaviourExecutionStatus ParallelNode::Execute()
{
    uint16 successCount = 0;
    uint16 runningCount = 0;
    for (auto& child : m_children)
    {
        switch (child->Execute())
        {
            case BehaviourExecutionStatus::Failed:
                continue;
            case BehaviourExecutionStatus::Succeeded:
                successCount++;
                continue;
            case BehaviourExecutionStatus::Running:
                runningCount++;
                continue;
        }
    }

    if (successCount > 0)
        return BehaviourExecutionStatus::Succeeded;
    if (runningCount > 0)
        return BehaviourExecutionStatus::Running;
    return BehaviourExecutionStatus::Failed;

}
