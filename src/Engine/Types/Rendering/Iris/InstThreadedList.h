#pragma once

#include <utility>
#include <Engine/Types/CommonTypes.h>
#include <Engine/Types/Rendering/GPU/Model.h>
#include <Engine/Types/Rendering/GPU/Shader.h>

#include "Engine/WTL/vector.h"

// by the way, terrible cache efficiency! This is one whole ass cache line long.
struct MemListNode
{
    MemListNode* next;
    MemListNode* nextOfKind;
    MemListNode* prev;
    MemListNode* prevOfKind;
    WEngine::Model model;
    WEngine::Shader shader;
    uint64 offset;
    uint64 size;
};

struct MemListDebugInfo
{
    WEngine::Model model;
    WEngine::Shader shader;
    uint64 offset;
    uint64 size;
};

/**
 * This class manages the bookkeeping for the Iris stationary instance buffer.
 */
class InstThreadedList
{
public:
    InstThreadedList();
    ~InstThreadedList();

    /**
     * Finds a given node by the model and shader and returns the place in the buffer.
     * @param model model to be found.
     * @param shader shader to be found.
     * @return [0] offset in the buffer in bytes; [1] reach in the buffer in bytes.
     * @note if the node cannot be found, then it returns 0,0.
     */
    std::pair<uint64, uint64> FindNode(WEngine::Model model, WEngine::Shader shader) const;

    /**
     * Inserts instance data into the buffer.
     * @param model model to be inserted.
     * @param shader shader to be inserted.
     * @param size size to allocate in bytes.
     * @return [0] offset in the buffer in bytes; [1] reach in the buffer in bytes.
     * @note if a place cannot be found, then it returns 0,0.
     * @note this may decide to reallocate the block, please make sure to check up with FindNode first!
     */
    std::pair<uint64, uint64> InsertData(WEngine::Model model, WEngine::Shader shader, uint64 size);

    /**
     * Clears an occupied region and makes it empty.
     * @param model model to be cleared.
     * @param shader shader to be cleared.
     */
    void ClearNode(WEngine::Model model, WEngine::Shader shader);

    /**
     * @warning due to the complexity between this and Iris, this is not yet implemented.
     */
    void Defragment();

    [[nodiscard]] wtl::vector<MemListDebugInfo> GetDebugInfo() const;

private:
    [[nodiscard]] MemListNode* FindBestFit(uint64 size) const;
    void DecoupleOccupiedEntry(MemListNode* entry);
    void SqueezeEntry(MemListNode* entry, MemListNode* freeBlock);

private:
    MemListNode* head;
    MemListNode* emptyHead;
    MemListNode* occupiedHead;
};
