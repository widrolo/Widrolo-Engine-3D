#pragma once
#include <deque>
#include "allocator.h"

namespace wtl
{
    template<class T>
    using deque = std::deque<T, allocator<T>>;
}