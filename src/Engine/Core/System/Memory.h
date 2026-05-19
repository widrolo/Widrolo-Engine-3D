#pragma once
#include <Engine/Types/CommonTypes.h>
#include <unordered_map>

class WAllocator
{
public:
    static void BootAllocator();

    // These ones are no joke, they can directly change the usage statistic
    // without a tracked allocation. These should only be used when allocating
    // a new object in a library. Since the Allocator will be able to detect leaks, its
    // crucial to report any frees, or else it will report a leak when there isnt.
    static void ReportExternalAllocation(uint64 size) noexcept;
    static void ReportExternalFree(uint64 size) noexcept;

    static void* Allocate(uint64 size);
    static void* AllocateAligned(uint64 size, uint64 alignment);
    static void* Reallocate(void* ptr, uint64 size);
    static void* ReallocateAligned(void* ptr, uint64 size, uint64 alignment);
    static void Free(void* ptr) noexcept;

    template<class T, typename... Args>
    static T* Construct(Args&&... args)
    {
        uint64 size = sizeof(T);
        MemoryUsed() += size;
        T* obj = new T(std::forward<Args>(args)...);

        MemorySizes()[obj] = size;
        return obj;
    }
    template<class T>
    static void Destruct(T* p)
    {
        uint64 size = MemorySizes()[p];
        MemoryUsed() -= size;
        delete p;
    }

    static uint64 GetMemoryUsage();

private:
    static std::unordered_map<void*, uint64>& MemorySizes()
    {
        static std::unordered_map<void*, uint64> map;
        return map;
    }

    static uint64& MemoryUsed()
    {
        static uint64 used = 0;
        return used;
    }
};

#define wNew(size) WAllocator::Allocate(size);
#define wNewArr(type, count) (type*)WAllocator::Allocate(count * sizeof(type));
#define wFree(ptr) WAllocator::Free(ptr);