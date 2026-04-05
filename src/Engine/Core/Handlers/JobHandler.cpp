#include "JobHandler.h"

#include <Engine/Util/Log.h>

using namespace WEngine;

JobHandler::JobHandler()
{

}
JobHandler::~JobHandler()
{

}

ThreadScheduler* JobHandler::GetThreadScheduler(const uint8 threadID)
{
    auto state = m_workerThreadStates[threadID];
    if (state == WorkerThreadState::Free)
    {
        WLog::SetConsoleError();
        WLog::ConsoleLog("Tried to access unscheduled thread");
        return nullptr;
    }
    if (state == WorkerThreadState::Owned)
    {
        WLog::SetConsoleError();
        WLog::ConsoleLog("Tried to access owned thread without a proof of ownership");
        return nullptr;
    }

    return &m_threadSchedulers[threadID];
}

ThreadScheduler* JobHandler::GetThreadScheduler(uint8 threadID, uint64 ownershipProof)
{
    auto state = m_workerThreadStates[threadID];
    if (state == WorkerThreadState::Free)
    {
        WLog::SetConsoleError();
        WLog::ConsoleLog("Tried to access unscheduled thread");
        return nullptr;
    }
    if (state == WorkerThreadState::Owned || m_ownershipProof[threadID] != ownershipProof)
    {
        WLog::SetConsoleError();
        WLog::ConsoleLog("Tried to access owned thread with a false proof of ownership");
        return nullptr;
    }

    return &m_threadSchedulers[threadID];
}

void JobHandler::AddScheduledJob(const Job& jobFunction, JobLengthEstimation estimation)
{

}
