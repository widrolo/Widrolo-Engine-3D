#pragma once

#include <map>
#include "allocator.h"

namespace wtl
{
    template<class Key, class T>
    using map = std::map<Key, T, std::less<Key>, allocator<std::pair<const Key, T>>>;
}
