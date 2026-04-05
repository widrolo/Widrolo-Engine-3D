#pragma once
#include <Engine/Core/System/Memory.h>

namespace wtl
{
    // This is based on the glibc allocator, but used the WAllocator
    // for memory allocation. This is so that its completely compatible with
    // the old one.
    template<class T>
    class allocator
    {
    public:
        using value_type = T;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;

        constexpr allocator() noexcept = default;
        constexpr allocator(const allocator&) noexcept = default;
        constexpr allocator(allocator&&) noexcept = default;
        template<class U>
        constexpr allocator(const allocator<U>&) noexcept {};
        constexpr ~allocator() noexcept = default;

        T* allocate(std::size_t size)
        {
            if (size == 0)
                return nullptr;
            void* p = WAllocator::Allocate(size * sizeof(T));
            if (!p)
                throw std::bad_alloc();

            return static_cast<T*>(p);
        }
        void deallocate(T* ptr, std::size_t size) noexcept
        {
            WAllocator::Free(ptr);
        }

        allocator& operator=(const allocator&) = default;
        bool operator==(const allocator&) const noexcept
        {
            return true;
        }
    };
}