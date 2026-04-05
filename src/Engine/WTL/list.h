#pragma once
#include <list>
#include "allocator.h"

namespace wtl
{
    template<class T>
    using list = std::list<T, allocator<T>>;
}