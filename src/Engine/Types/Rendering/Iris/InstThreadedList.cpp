#include "InstThreadedList.h"

#include "Engine/Core/System/GPUSettings.h"
#include "Engine/Core/System/Memory.h"
#include "Engine/Util/Log.h"

InstThreadedList::InstThreadedList()
{
    MemListNode* node = wNewArr(MemListNode, 1);
    node->next = nullptr;
    node->nextOfKind = nullptr;
    node->prev = nullptr;
    node->prevOfKind = nullptr;
    node->model = 0;
    node->shader = 0;
    node->offset = 0;
    node->size = GPUSettings::stationaryInstBufferSize;
    head = node;
    emptyHead = node;
}

InstThreadedList::~InstThreadedList()
{

}

std::pair<uint64, uint64> InstThreadedList::FindNode(WEngine::Model model, WEngine::Shader shader) const
{
    MemListNode* cursor = occupiedHead;

    while (cursor != nullptr)
    {
        if (cursor->model == model && cursor->shader == shader)
            return {cursor->offset, cursor->size};
        cursor = cursor->next;
    }

    return {0,0};
}

std::pair<uint64, uint64> InstThreadedList::InsertData(WEngine::Model model, WEngine::Shader shader, uint64 size)
{
    if (occupiedHead == nullptr)
    {
        MemListNode* node = wNewArr(MemListNode, 1);
        node->next = emptyHead;
        node->nextOfKind = nullptr;
        node->model = model;
        node->shader = shader;
        node->size = size;
        node->offset = 0;

        emptyHead->size -= size;
        emptyHead->offset = size;

        head = node;
        return {0, size};
    }

    MemListNode* cursor = occupiedHead;

    bool found = false;
    while (cursor != nullptr)
    {
        if (cursor->model == model && cursor->shader == shader)
        {
            found = true;
            break;
        }
        cursor = cursor->next;
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
            }
            // case 1.2: we cannot expand
            else
            {
                DecoupleOccupiedEntry(cursor);
                auto freeBlock = FindBestFit(size);
                SqueezeEntry(cursor, freeBlock);
            }

        }
    }
    // case 2: we have to add a new entry.
    else
    {

    }
}

void InstThreadedList::ClearNode(WEngine::Model model, WEngine::Shader shader)
{

}

void InstThreadedList::Defragment()
{
    WEngine::WLog::SetConsoleError();
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
        debugInfo.shader = cursor->shader;
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
        if (cursor->size >= size)
            return cursor;
        cursor = cursor->next;
    }

    WEngine::WLog::SetConsoleWarning();
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

    }
    // case 2: surrounded by free blocks
    if (freeBehind && freeInFront)
    {

    }
    // case 3: free block behind
    if (freeBehind && !freeInFront)
    {

    }
    // case 4: free block in front
    if (!freeBehind && freeInFront)
    {

    }

    // decouple cursor from list and kind list
    // account for freeing block, handle offset
}

void InstThreadedList::SqueezeEntry(MemListNode *entry, MemListNode *freeBlock)
{
    // move there
    // inform the free block.
    // couple into list, connect kind pointer.
    // change cursor offset.
}
