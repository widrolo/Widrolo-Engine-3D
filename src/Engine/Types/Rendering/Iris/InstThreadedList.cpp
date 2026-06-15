#include "InstThreadedList.h"

#include "Engine/Core/System/GPUSettings.h"
#include "Engine/Core/System/Memory.h"
#include "Engine/Util/Log.h"

InstThreadedList::InstThreadedList()
{
    MemListNode* node = wNewArr(MemListNode, 1);
    node->next = nullptr;
    node->prev = nullptr;
    node->nextOfKind = nullptr;
    node->prevOfKind = nullptr;
    node->model = 0;
    node->material = 0;
    node->offset = 0;
    node->size = GPUSettings::stationaryInstBufferSize;
    head = node;
    emptyHead = node;
    occupiedHead = nullptr;
}

InstThreadedList::~InstThreadedList()
{
    MemListNode* cursor = head;
    MemListNode* temp;
    while (cursor != nullptr)
    {
        temp = cursor->next;
        wFree(cursor);
        cursor = temp;
    }
}

std::pair<uint64, uint64> InstThreadedList::FindNode(WEngine::Model model, WEngine::Material material) const
{
    MemListNode* cursor = occupiedHead;

    while (cursor != nullptr)
    {
        if (cursor->model == model && cursor->material == material)
            return {cursor->offset, cursor->size};
        cursor = cursor->nextOfKind;
    }

    return {0,0};
}

std::pair<uint64, uint64> InstThreadedList::InsertData(WEngine::Model model, WEngine::Material material, uint64 size)
{
    if (occupiedHead == nullptr)
    {
        MemListNode* node = wNewArr(MemListNode, 1);
        node->next = emptyHead;
        node->prev = nullptr;
        node->nextOfKind = nullptr;
        node->prevOfKind = nullptr;
        node->model = model;
        node->material = material;
        node->size = size;
        node->offset = 0;

        if (size > emptyHead->size)
        {
            WEngine::WLog::SetConsoleError();
            WEngine::WLog::ConsoleLog("Could not fit new Stationary Instance Data, please increase it in GPUSettings.h");
            abort();
        }

        // if you hit this, you're dangerously close to a crash
        if (size == emptyHead->size)
        {
            MemListNode* freeNode = emptyHead;

            head = node;
            occupiedHead = node;
            emptyHead = nullptr;

            wFree(freeNode);
            return {0, size};
        }
        emptyHead->size -= size;
        emptyHead->offset = size;
        emptyHead->prev = node;

        occupiedHead = node;

        head = node;
        return {0, size};
    }

    MemListNode* cursor = occupiedHead;

    bool found = false;
    while (cursor != nullptr)
    {
        if (cursor->model == model && cursor->material == material)
        {
            found = true;
            break;
        }
        cursor = cursor->nextOfKind;
    }

    // case 1: we have the model
    if (found)
    {
        if (cursor->next != nullptr)
        {
            // case 1.1: we can expand
            if (cursor->next->model == 0 && cursor->next->size >= size)
            {
                cursor->size += size;
                cursor->next->offset += size;
                cursor->next->size -= size;

                MemListNode* freeNode = cursor->next;

                cursor->size += size;
                cursor->next = freeNode->next;
                if (freeNode->next != nullptr)
                    freeNode->next->prev = cursor;

                // unlink freeNode from free list
                if (freeNode->prevOfKind != nullptr)
                    freeNode->prevOfKind->nextOfKind = freeNode->nextOfKind;
                else
                    emptyHead = freeNode->nextOfKind;

                if (freeNode->nextOfKind != nullptr)
                    freeNode->nextOfKind->prevOfKind = freeNode->prevOfKind;

                wFree(freeNode);
            }
            // case 1.2: we cannot expand
            else
            {
                uint64 newSize = cursor->size + size;
                DecoupleOccupiedEntry(cursor);
                cursor->size = newSize;
                auto freeBlock = FindBestFit(cursor->size + size);
                SqueezeEntry(cursor, freeBlock);
            }
        }
        // case 1.3: same as 1.2, but now there is an out of memory risk.
        else
        {
            uint64 newSize = cursor->size + size;
            DecoupleOccupiedEntry(cursor);
            cursor->size = newSize;
            auto freeBlock = FindBestFit(cursor->size + size);
            SqueezeEntry(cursor, freeBlock);
        }
    }
    // case 2: we have to add a new entry.
    else
    {
        MemListNode* node = wNewArr(MemListNode, 1);
        node->next = nullptr;
        node->prev = nullptr;
        node->nextOfKind = nullptr;
        node->prevOfKind = nullptr;
        node->model = model;
        node->material = material;
        node->size = size;
        cursor = node;
        auto freeBlock = FindBestFit(size);
        SqueezeEntry(cursor, freeBlock);
    }

    return {cursor->offset, cursor->size};
}

void InstThreadedList::ClearNode(WEngine::Model model, WEngine::Material material)
{
    MemListNode* cursor = occupiedHead;
    while (cursor != nullptr)
    {
        if (cursor->model == model && cursor->material == material)
        {
            DecoupleOccupiedEntry(cursor);

            wFree(cursor);
            return;
        }
        cursor = cursor->nextOfKind;
    }
}

void InstThreadedList::Defragment()
{
    WEngine::WLog::SetConsoleWarning();
    WEngine::WLog::ConsoleLog("Call to defragment Instance Buffer while its not yet implemented!");
}

wtl::vector<MemListDebugInfo> InstThreadedList::GetDebugInfo() const
{
    wtl::vector<MemListDebugInfo> info;
    MemListNode* cursor = head;

    while (cursor != nullptr)
    {
        MemListDebugInfo debugInfo;
        debugInfo.model = cursor->model;
        debugInfo.material = cursor->material;
        debugInfo.offset = cursor->offset;
        debugInfo.size = cursor->size;
        info.push_back(debugInfo);
        cursor = cursor->next;
    }

    return info;
}

MemListNode* InstThreadedList::FindBestFit(uint64 size) const
{
    MemListNode* cursor = emptyHead;

    while (cursor != nullptr)
    {
        // not best fit yet, but its good enough for now.
        if (cursor->model == 0 && cursor->size >= size)
            return cursor;
        cursor = cursor->nextOfKind;
    }

    WEngine::WLog::SetConsoleError();
    WEngine::WLog::ConsoleLog("Could not fit new Stationary Instance Data, please increase it in GPUSettings.h");
    abort();
}

void InstThreadedList::DecoupleOccupiedEntry(MemListNode *entry)
{
    bool freeBehind = false;
    bool freeInFront = false;

    if (entry->next != nullptr)
    {
        if (entry->next->model == 0)
            freeInFront = true;
    }
    if (entry->prev != nullptr)
    {
        if (entry->prev->model == 0)
            freeBehind = true;
    }

    // case 1: no free blocks around
    if (!freeBehind && !freeInFront)
    {
        MemListNode* node = wNewArr(MemListNode, 1);
        node->next = entry->next;
        node->prev = entry->prev;
        node->nextOfKind = nullptr;
        node->prevOfKind = nullptr;
        node->model = 0;
        node->material = 0;
        node->offset = entry->offset;
        node->size = entry->size;

        if (entry->next != nullptr)
            entry->next->prev = node;
        if (entry->prev != nullptr)
            entry->prev->next = node;

        MemListNode* cursor = emptyHead;
        while (cursor != nullptr)
        {
            if (cursor->offset > node->offset)
                break;
            cursor = cursor->nextOfKind;
        }
        // FIXME: What if cursor ends up being last?
        MemListNode* back = cursor->prevOfKind;
        if (back == nullptr)
            emptyHead = node;
        else
            back->nextOfKind = node;
        cursor->prevOfKind = node;
        node->nextOfKind = cursor;
        node->prevOfKind = back;

        if (entry == head)
            head = node;
    }
    // case 2: surrounded by free blocks
    if (freeBehind && freeInFront)
    {
        MemListNode* back = entry->prev;
        MemListNode* front = entry->next;

        // back one will consume the front one.
        back->size += entry->size + front->size;
        back->next = front->next;
        if (back->next != nullptr)
            back->next->prev = back;
        back->nextOfKind = front->nextOfKind;
        if (back->nextOfKind != nullptr)
            back->nextOfKind->prevOfKind = back;


        wFree(front);
    }
    // case 3: free block behind
    if (freeBehind && !freeInFront)
    {
        MemListNode* back = entry->prev;
        back->size += entry->size;
        back->next = entry->next;
        if (entry->next != nullptr)
            entry->next->prev = back;
    }
    // case 4: free block in front
    if (!freeBehind && freeInFront)
    {
        MemListNode* front = entry->next;
        front->size += entry->size;
        front->offset = entry->offset;
        front->prev = entry->prev;
        if (entry->prev != nullptr)
            entry->prev->next = front;

        if (entry == head)
            head = front;
    }

    if (entry->prevOfKind != nullptr)
        entry->prevOfKind->nextOfKind = entry->nextOfKind;
    if (entry->nextOfKind != nullptr)
        entry->nextOfKind->prevOfKind = entry->prevOfKind;

    if (entry == occupiedHead)
        occupiedHead = entry->nextOfKind;

    entry->next = nullptr;
    entry->prev = nullptr;
    entry->nextOfKind = nullptr;
    entry->prevOfKind = nullptr;
}

void InstThreadedList::SqueezeEntry(MemListNode* entry, MemListNode* freeBlock)
{
    if (entry == nullptr || freeBlock == nullptr)
    {
        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog("InstThreadedList::SqueezeEntry() received nullptr!");
        abort();
    }
    if (freeBlock->model != 0)
    {
        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog("InstThreadedList::SqueezeEntry() received non-free freeBlock!");
        abort();
    }
    if (freeBlock->size < entry->size)
    {
        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog("Unable to squeeze memory entry, please fix InstThreadedList::FindBestFit()!");
        abort();
    }
    const uint64 allocatedOffset = freeBlock->offset;
    const uint64 allocatedSize = entry->size;

    // just for good measure
    entry->next = nullptr;
    entry->prev = nullptr;
    entry->nextOfKind = nullptr;
    entry->prevOfKind = nullptr;
    entry->offset = allocatedOffset;

    // case 1: replace the free block withe the entry
    if (freeBlock->size == allocatedSize)
    {
        MemListNode* oldPrev = freeBlock->prev;
        MemListNode* oldNext = freeBlock->next;
        // Remove freeBlock from free-kind list.
        if (freeBlock->prevOfKind != nullptr)
            freeBlock->prevOfKind->nextOfKind = freeBlock->nextOfKind;
        else
            emptyHead = freeBlock->nextOfKind;
        if (freeBlock->nextOfKind != nullptr)
            freeBlock->nextOfKind->prevOfKind = freeBlock->prevOfKind;
        // Replace freeBlock with entry in main list.
        entry->prev = oldPrev;
        entry->next = oldNext;
        if (oldPrev != nullptr)
            oldPrev->next = entry;
        else
            head = entry;
        if (oldNext != nullptr)
            oldNext->prev = entry;
        wFree(freeBlock);
    }
    // case 2: change the free block to fit it.
    else
    {
        MemListNode* oldPrev = freeBlock->prev;
        entry->prev = oldPrev;
        entry->next = freeBlock;
        if (oldPrev != nullptr)
            oldPrev->next = entry;
        else
            head = entry;
        freeBlock->prev = entry;
        freeBlock->offset += allocatedSize;
        freeBlock->size -= allocatedSize;
    }

    entry->prevOfKind = nullptr;
    entry->nextOfKind = occupiedHead;
    if (occupiedHead != nullptr)
        occupiedHead->prevOfKind = entry;
    occupiedHead = entry;
}
