#pragma once

#include "BehaviourExecutionStatus.h"
#include "Engine/Types/SpawnArgs.h"
#include "Engine/WTL/vector.h"

namespace WEngine
{
    class Blackboard;
}

namespace WEngine::Behaviour
{
    class BehaviourNode
    {
    public:
        BehaviourNode(Blackboard* blackboard) : m_ID(GetNextID()), m_blackboard(blackboard) {}
        virtual ~BehaviourNode() = default;

    public:
        virtual void SpawnNode(ComponentArgs& ca);
        virtual BehaviourExecutionStatus Execute() = 0;

        void AttachChild(BehaviourNode* child);
        void AttachChildren(const wtl::vector<BehaviourNode*>& children);

        void DetachChild(uint64 ID);
        void DetachChild(const BehaviourNode* ptrRef);

    private:
        static uint64 GetNextID()
        {
            static uint64 id = 0;
            return id++;
        }

    protected:
        uint64 m_ID;
        BehaviourNode* m_parent = nullptr;
        Blackboard* m_blackboard = nullptr;
        wtl::vector<BehaviourNode*> m_children;
    };
}
