#pragma once
#include <vector>
#include "allocator.h"

namespace wtl
{
    template<class T>
    using vector = std::vector<T, allocator<T>>;
}