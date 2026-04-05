#pragma once
#include <Engine/WTL/vector.h>
#include <Engine/Types/CoreSystems.h>

#include <Engine/Types/Jobs/WorkerThreadState.h>
#include <Engine/Types/Jobs/JobLengthEstimation.h>
#include <Engine/Core/System/ThreadScheduler.h>

namespace WEngine
{
    class JobHandler
    {
    public:
        JobHandler();
        ~JobHandler();
    private:
        uint8 m_threadCount;
        wtl::vector<WorkerThreadState> m_workerThreadStates;
        wtl::vector<uint64> m_ownershipProof;
        wtl::vector<ThreadScheduler> m_threadSchedulers;

    public:
        ThreadScheduler* GetThreadScheduler(uint8 threadID);
        ThreadScheduler* GetThreadScheduler(uint8 threadID, uint64 ownershipProof);
        void AddScheduledJob(const Job& jobFunction, JobLengthEstimation estimation);


    private:
        void CreateSchedulers();
    };
}

