#pragma once

#include <Engine/Types/CommonTypes.h>
#include <Engine/Types/Jobs/Job.h>
#include <Engine/WTL/deque.h>
#include <atomic>
#include <mutex>

namespace WEngine
{
    // This one is not CamelCase to mirror real OS _start
    // entry points. It should be as obvious as possible
    // that this boots a thread.
    void _startThread(uint8 threadID);

    class ThreadScheduler
    {
    public:
        ThreadScheduler();
        ~ThreadScheduler();
    private:
        wtl::deque<Job> m_jobs;
        std::mutex m_jobMutex;

    public:
        void AddJob(const Job& job);
        void RunJobs();
    };
}

