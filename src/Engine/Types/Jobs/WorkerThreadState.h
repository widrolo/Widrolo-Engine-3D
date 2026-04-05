#pragma once

namespace WEngine
{
    enum class WorkerThreadState
    {
        Free,
        Scheduled,
        Owned
    };
}