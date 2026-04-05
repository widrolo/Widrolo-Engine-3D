#include "BranchNode.h"

using namespace WEngine::Behaviour;

BehaviourExecutionStatus BranchNode::Execute()
{
    if (m_children.size() != 2)
    {
        WLog::SetConsoleWarning();
        WLog::ConsoleLog("BranchNode Error! Child count is not 2!");
        return BehaviourExecutionStatus::Failed;
    }
}

void BranchNode::SpawnNode(ComponentArgs &ca)
{
    auto varName = ca.GetStringFromParams("varname");
    auto trueVar = ca.GetStringFromParams("truevar");

}
