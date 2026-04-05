#include "Memory.h"
#include <Engine/Util/Log.h>
#include <cstdlib>
#include <cstring>



void WAllocator::BootAllocator()
{
    MemoryUsed() = 0;
    MemorySizes().clear();
}

void WAllocator::ReportExternalAllocation(uint64 size) noexcept
{
    MemoryUsed() += size;
}

void WAllocator::ReportExternalFree(uint64 size) noexcept
{
    MemoryUsed() -= size;
}

void* WAllocator::Allocate(uint64 size)
{
    void* p = std::malloc(size);
    if (!p) throw std::bad_alloc();

    MemoryUsed() += size;
    MemorySizes()[p] = size;
    return p;
}

void* WAllocator::Reallocate(void* ptr, uint64 size)
{
    auto& sizes = MemorySizes();
    uint64 oldSize = sizes[ptr];

    if (size < oldSize)
    {
        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog("Bad Reallocation: new size is smaller than old size!");
        throw std::bad_alloc();
    }

    void* newPtr = std::realloc(ptr, size);
    if (!newPtr) throw std::bad_alloc();

    sizes.erase(ptr);
    sizes[newPtr] = size;

    MemoryUsed() += (size - oldSize);
    return newPtr;
}

void WAllocator::Free(void* ptr) noexcept
{
    auto& sizes = MemorySizes();
    auto it = sizes.find(ptr);
    if (it != sizes.end())
    {
        MemoryUsed() -= it->second;
        sizes.erase(it);
    }
    std::free(ptr);
}

uint64 WAllocator::GetMemoryUsage()
{
    return MemoryUsed();
}

