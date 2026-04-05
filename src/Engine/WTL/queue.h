#pragma once
#include <queue>
#include "deque.h"
#include "allocator.h"

namespace wtl
{
    template<class T>
    using queue = std::queue<T, deque<T>>;
}