/*
* Copyright (c) 2017, Intel Corporation
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
* OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*/
//!
//! \file     memory_block_manager.cpp
//! \brief    Implements functionalities pertaining to the memory block manager
//!

#include "memory_block_manager.h"

MemoryBlockManager::~MemoryBlockManager()
{
    HEAP_FUNCTION_ENTER;
    m_heaps.clear();
    m_deletedHeaps.clear();
    HEAP_VERBOSEMESSAGE("Total heap size is %d", m_totalSizeOfHeaps);
    for (uint8_t i = 0; i < MemoryBlockInternal::State::stateCount; i++)
    {
        HEAP_VERBOSEMESSAGE("%d blocks in pool %d with size %d", m_sortedBlockListNumEntries[i], i, m_sortedBlockListSizes[i]);

        // pool blocks are not part of the adjacency list and must be freed separately
        if (i == MemoryBlockInternal::State::pool)
        {
            auto curr = m_sortedBlockList[i];
            MemoryBlockInternal *next = nullptr;
            while (curr != nullptr)
            {
                next = curr->m_stateNext;
                MOS_Delete(curr);
                curr = next;
            }
        }
    }
}

MOS_STATUS MemoryBlockManager::AcquireSpace(
    AcquireParams &params,
    std::vector<MemoryBlock> &blocks,
    uint32_t &spaceNeeded)
{
    HEAP_FUNCTION_ENTER;

    if (params.m_blockSizes.empty())
    {
        HEAP_ASSERTMESSAGE("No space is being requested");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if (m_sortedSizes.size() != params.m_blockSizes.size())
    {
        m_sortedSizes.resize(params.m_blockSizes.size());
    }
    uint32_t alignment = MOS_MAX(m_blockAlignment, MOS_ALIGN_CEIL(params.m_alignment, m_blockAlignment));
    uint32_t idx = 0;
    auto sortedIterator = m_sortedSizes.begin();
    for (auto requestIterator = params.m_blockSizes.begin();
        requestIterator != params.m_blockSizes.end();
        ++requestIterator)
    {
        if (sortedIterator == m_sortedSizes.end())
        {
            HEAP_ASSERTMESSAGE("sortedSizes is expected to be the same size as the number of blocks requested");
            return MOS_STATUS_UNKNOWN;
        }
        (*sortedIterator).m_originalIdx = idx++;
        (*sortedIterator).m_blockSize = MOS_ALIGN_CEIL(*requestIterator, alignment);
        ++sortedIterator;
    }
    if (m_sortedSizes.size() > 1)
    {
        m_sortedSizes.sort([](SortedSizePair &a, SortedSizePair &b) { return a.m_blockSize > b.m_blockSize; });
    }

    if (m_sortedBlockListNumEntries[MemoryBlockInternal::submitted] > m_numSubmissionsForRefresh)
    {
        bool blocksUpdated = false;
        HEAP_CHK_STATUS(RefreshBlockStates(blocksUpdated));
    }

    spaceNeeded = 0;
    HEAP_CHK_STATUS(IsSpaceAvailable(params, spaceNeeded));
    if (spaceNeeded == 0)
    {
        HEAP_CHK_STATUS(AllocateSpace(params, blocks));
        return MOS_STATUS_SUCCESS;
    }

    blocks.clear();
    return MOS_STATUS_CLIENT_AR_NO_SPACE;
}

MOS_STATUS MemoryBlockManager::SubmitBlocks(std::vector<MemoryBlock> &blocks)
{
    HEAP_FUNCTION_ENTER;

    if (blocks.empty())
    {
        HEAP_ASSERTMESSAGE("No blocks to submit");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    for (uint32_t i = 0; i < blocks.size(); ++i)
    {
        if (!blocks[i].IsValid())
        {
            HEAP_ASSERTMESSAGE("Block %d is not valid and may not be submitted", i);
            return MOS_STATUS_INVALID_PARAMETER;
        }

        auto internalBlock = blocks[i].GetInternalBlock();
        HEAP_CHK_NULL(internalBlock);
        HEAP_CHK_STATUS(RemoveBlockFromSortedList(internalBlock, internalBlock->GetState()));
        HEAP_CHK_STATUS(internalBlock->Submit());
        HEAP_CHK_STATUS(AddBlockToSortedList(internalBlock, internalBlock->GetState()));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MemoryBlockManager::ClearSpace(MemoryBlock &block)
{
    HEAP_FUNCTION_ENTER;

    if (!block.IsValid())
    {
        HEAP_ASSERTMESSAGE("Block is not valid and may not be cleared");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    auto internalBlock = block.GetInternalBlock();
    HEAP_CHK_NULL(internalBlock);
    internalBlock->ClearStatic();
    bool blocksUpdated = false;
    HEAP_CHK_STATUS(RefreshBlockStates(blocksUpdated));
    // Reset memory block to blank state to return to client
    block = MemoryBlock();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MemoryBlockManager::RefreshBlockStates(bool &blocksUpdated)
{
    HEAP_FUNCTION_ENTER_VERBOSE;

    if ((!m_useProducer && m_trackerData == nullptr) 
        || (m_useProducer && m_trackerProducer == nullptr))
    {
        HEAP_ASSERTMESSAGE("It is not possible to check current tracker ID");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    blocksUpdated = false;
    uint32_t currTrackerId = 0;
    if (!m_useProducer)
    {
        currTrackerId = *m_trackerData;
    }

    auto block = m_sortedBlockList[MemoryBlockInternal::State::submitted];
    MemoryBlockInternal *nextSubmitted = nullptr;
    while (block != nullptr)
    {
        nextSubmitted = block->m_stateNext;
        FrameTrackerToken *trackerToken = block->GetTrackerToken();
        if ( (!m_useProducer && block->GetTrackerId() <= currTrackerId)
            ||(m_useProducer && trackerToken->IsExpired()))
        {
            auto heap = block->GetHeap();
            HEAP_CHK_NULL(heap);

            if (heap->IsFreeInProgress())
            {
                // Add the block to deleted list instead of freed to prevent it from being reused
                HEAP_CHK_STATUS(RemoveBlockFromSortedList(block, block->GetState()));
                HEAP_CHK_STATUS(block->Delete());
                HEAP_CHK_STATUS(AddBlockToSortedList(block, block->GetState()));
                block = nextSubmitted;
                continue;
            }

            HEAP_CHK_STATUS(RemoveBlockFromSortedList(block, block->GetState()));
            HEAP_CHK_STATUS(block->Free());
            HEAP_CHK_STATUS(AddBlockToSortedList(block, block->GetState()));

            // Consolidate free blocks
            auto prev = block->GetPrev(), next = block->GetNext();
            if (prev && prev->GetState() == MemoryBlockInternal::State::free)
            {
                HEAP_CHK_STATUS(MergeBlocks(prev, block));
                // re-assign block to pPrev for use in MergeBlocks with pNext
                block = prev;
            }
            else if (prev == nullptr)
            {
                HEAP_ASSERTMESSAGE("The previous block should always be valid");
                return MOS_STATUS_UNKNOWN;
            }

            if (next && next->GetState() == MemoryBlockInternal::State::free)
            {
                HEAP_CHK_STATUS(MergeBlocks(block, next));
            }

            blocksUpdated = true;
        }
        block = nextSubmitted;
    }

    if (blocksUpdated && !m_deletedHeaps.empty())
    {
        HEAP_CHK_STATUS(CompleteHeapDeletion());
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MemoryBlockManager::RegisterHeap(uint32_t heapId, uint32_t size , bool hwWriteOnly)
{
    HEAP_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    auto heap = MOS_New(Heap, heapId);
    HEAP_CHK_NULL(heap);
    eStatus = heap->RegisterOsInterface(m_osInterface);
    if (MOS_FAILED(eStatus))
    {
        MOS_Delete(heap);
        HEAP_CHK_STATUS(eStatus);
    }
    size = MOS_ALIGN_CEIL(size, m_heapAlignment);
    
    if (hwWriteOnly)
    {
        heap->SetHeapHwWriteOnly(hwWriteOnly);
    }

    eStatus = heap->Allocate(size, m_lockHeapsOnAllocate);
    if (MOS_FAILED(eStatus))
    {
        MOS_Delete(heap);
        HEAP_CHK_STATUS(eStatus);
    }

    if (heap->IsValid())
    {
        MemoryBlockInternal *adjacencyListBegin = nullptr;
        adjacencyListBegin = MOS_New(MemoryBlockInternal);
        if (adjacencyListBegin == nullptr)
        {
            MOS_Delete(heap);
            HEAP_CHK_STATUS(MOS_STATUS_NULL_POINTER);
        }
        auto block = GetBlockFromPool();
        if (block == nullptr)
        {
            MOS_Delete(adjacencyListBegin);
            MOS_Delete(heap);
            HEAP_ASSERTMESSAGE("block be null");
            return MOS_STATUS_NULL_POINTER;
        }

        std::shared_ptr<HeapWithAdjacencyBlockList> managedHeap = nullptr;
        managedHeap = MakeShared<HeapWithAdjacencyBlockList>();
        if (managedHeap == nullptr)
        {
            MOS_Delete(heap);
            MOS_Delete(adjacencyListBegin);
            HEAP_CHK_STATUS(MOS_STATUS_NULL_POINTER);
        }
        managedHeap->m_heap = heap;
        managedHeap->m_size = managedHeap->m_heap->GetSize();
        managedHeap->m_adjacencyListBegin = adjacencyListBegin;
        m_totalSizeOfHeaps += managedHeap->m_size;
        m_heaps.push_back(std::move(managedHeap));

        HEAP_CHK_STATUS(block->Create(
            heap,
            MemoryBlockInternal::State::free,
            adjacencyListBegin,
            0,
            size,
            MemoryBlockInternal::m_invalidTrackerId));
        HEAP_CHK_STATUS(AddBlockToSortedList(block, block->GetState()));
    }
    else
    {
        HEAP_ASSERTMESSAGE("Cannot register invalid heap");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    return eStatus;
}

MOS_STATUS MemoryBlockManager::UnregisterHeap(uint32_t heapId)
{
    HEAP_FUNCTION_ENTER;

    for (auto iterator = m_heaps.begin(); iterator != m_heaps.end(); ++iterator)
    {
        if (heapId == (*iterator)->m_heap->GetId())
        {
            bool blocksUpdated = false;
            HEAP_CHK_STATUS(RefreshBlockStates(blocksUpdated));
            (*iterator)->m_heap->PrepareForFree();
            m_totalSizeOfHeaps -= (*iterator)->m_heap->GetSize();

            // free blocks may be removed right away
            auto block = m_sortedBlockList[MemoryBlockInternal::State::free];
            MemoryBlockInternal *next = nullptr;
            while (block != nullptr)
            {
                next = block->m_stateNext;
                auto heap = block->GetHeap();
                if (heap != nullptr)
                {
                    if (heap->GetId() == heapId)
                    {
                        HEAP_CHK_STATUS(RemoveBlockFromSortedList(block, block->GetState()));
                        HEAP_CHK_STATUS(block->Delete());
                        HEAP_CHK_STATUS(AddBlockToSortedList(block, block->GetState()));
                    }
                }
                else
                {
                    HEAP_ASSERTMESSAGE("A block with an invalid heap is in the free list!");
                    return MOS_STATUS_UNKNOWN;
                }
                block = next;
            }

            m_deletedHeaps.push_back((*iterator));
            m_heaps.erase(iterator);
            HEAP_CHK_STATUS(CompleteHeapDeletion());
            return MOS_STATUS_SUCCESS;
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MemoryBlockManager::CompleteHeapDeletion()
{
    HEAP_FUNCTION_ENTER_VERBOSE;

    auto iterator = m_deletedHeaps.begin();
    while (iterator != m_deletedHeaps.end())
    {
        // if the heap is still not empty, continue with the loop
        if ((*iterator)->m_heap->GetUsedSize() == 0)
        {
            uint32_t heapId = (*iterator)->m_heap->GetId();
            HEAP_CHK_STATUS(RemoveHeapFromSortedBlockList(heapId));
            iterator = m_deletedHeaps.erase(iterator);
        }
        else
        {
            ++iterator;
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MemoryBlockManager::RegisterTrackerResource(uint32_t *trackerData)
{
    HEAP_FUNCTION_ENTER;
    HEAP_CHK_NULL(trackerData);
    m_trackerData = trackerData;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MemoryBlockManager::RegisterTrackerProducer(FrameTrackerProducer *trackerProducer)
{
    HEAP_FUNCTION_ENTER;
    HEAP_CHK_NULL(trackerProducer);
    m_trackerProducer = trackerProducer;
    m_useProducer = true;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MemoryBlockManager::RegisterOsInterface(PMOS_INTERFACE osInterface)
{
    HEAP_FUNCTION_ENTER;
    HEAP_CHK_NULL(osInterface);
    m_osInterface = osInterface;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MemoryBlockManager::IsSpaceAvailable(
    AcquireParams &params,
    uint32_t &spaceNeeded)
{
    HEAP_FUNCTION_ENTER_VERBOSE;

    if (m_sortedSizes.empty())
    {
        HEAP_ASSERTMESSAGE("No space is being requested");
        return MOS_STATUS_INVALID_PARAMETER;
    }
    if (m_sortedBlockList[MemoryBlockInternal::State::free] == nullptr)
    {
        bool blocksUpdated = false;
        HEAP_CHK_STATUS(RefreshBlockStates(blocksUpdated));
        if (!blocksUpdated)
        {
            HEAP_NORMALMESSAGE("All heap space is in use by active workloads.");
        }
    }

    auto block = m_sortedBlockList[MemoryBlockInternal::State::free];
 
    for (auto requestIterator = m_sortedSizes.begin();
        requestIterator != m_sortedSizes.end();
        ++requestIterator)
    {
        if (block == nullptr)
        {
            // There is no free space available, add up all requested sizes
            spaceNeeded += (*requestIterator).m_blockSize;
        }
        else
        {
            // Determine whether or not there is space available
            if ((*requestIterator).m_blockSize > block->GetSize())
            {
                // The requested size is larger than the largest free block size
                spaceNeeded += (*requestIterator).m_blockSize;
                continue;
            }
            else
            {
                // determine if there is enough space in the heaps
                auto freeBlockSize = block->GetSize();
                while (requestIterator != m_sortedSizes.end())
                {
                    if ((*requestIterator).m_blockSize < freeBlockSize)
                    {
                        // if the aligned size fits within the current free block, consider it used
                        freeBlockSize -= (*requestIterator).m_blockSize;
                        ++requestIterator;
                    }
                    else
                    {
                        // if the aligned size does not fit within the curret free block, continue for loop
                        break;
                    }
                }
                if (requestIterator == m_sortedSizes.end())
                {
                    break;
                }
            }
        }
        if (block != nullptr)
        {
            block = block->m_stateNext;
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MemoryBlockManager::AllocateSpace(
    AcquireParams &params,
    std::vector<MemoryBlock> &blocks)
{
    HEAP_FUNCTION_ENTER_VERBOSE;

    if (m_sortedSizes.empty())
    {
        HEAP_ASSERTMESSAGE("No space is being requested");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if (m_sortedBlockList[MemoryBlockInternal::State::free] == nullptr)
    {
        HEAP_ASSERTMESSAGE("No free blocks available");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if (blocks.size() != m_sortedSizes.size())
    {
        blocks.resize(m_sortedSizes.size());
    }

    for (auto requestIterator = m_sortedSizes.begin();
        requestIterator != m_sortedSizes.end();
        ++requestIterator)
    {
        bool allocated = false;
        auto block = m_sortedBlockList[MemoryBlockInternal::State::free];
        while (block != nullptr)
        {
            if (block->GetSize() >= (*requestIterator).m_blockSize)
            {
                auto heap = block->GetHeap();
                HEAP_CHK_NULL(heap);
                if (!m_useProducer)
                {
                    HEAP_CHK_STATUS(AllocateBlock(
                        (*requestIterator).m_blockSize,
                        params.m_trackerId,
                        params.m_staticBlock,
                        block));
                }
                else
                {
                    HEAP_CHK_STATUS(AllocateBlock(
                        (*requestIterator).m_blockSize,
                        params.m_trackerIndex,
                        params.m_trackerId,
                        params.m_staticBlock,
                        block));
                }
                if ((*requestIterator).m_originalIdx >= m_sortedSizes.size())
                {
                    HEAP_ASSERTMESSAGE("Index is out of bounds");
                    return MOS_STATUS_INVALID_PARAMETER;
                }
                HEAP_CHK_STATUS(blocks[(*requestIterator).m_originalIdx].CreateFromInternalBlock(
                    block,
                    heap,
                    heap->m_keepLocked ? heap->m_lockedHeap : nullptr));
                allocated = true;
                break;
            }
            block = block->m_stateNext;
        }

        if (!allocated)
        {
            HEAP_ASSERTMESSAGE("No free block was found for the data! This should not occur.");
            return MOS_STATUS_UNKNOWN;
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MemoryBlockManager::AllocateBlock(
    uint32_t alignedSize,
    uint32_t trackerId,
    bool staticBlock,
    MemoryBlockInternal *freeBlock)
{
    HEAP_FUNCTION_ENTER_VERBOSE;

    HEAP_CHK_NULL(freeBlock);

    if (alignedSize == 0 || alignedSize > freeBlock->GetSize())
    {
        HEAP_ASSERTMESSAGE("Size requested is invalid");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if (freeBlock->GetState() != MemoryBlockInternal::State::free)
    {
        HEAP_ASSERTMESSAGE("Free block is not free");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    HEAP_CHK_STATUS(RemoveBlockFromSortedList(freeBlock, freeBlock->GetState()));

    if (alignedSize < freeBlock->GetSize())
    {
        auto remainderBlock = GetBlockFromPool();
        HEAP_CHK_NULL(remainderBlock);
        freeBlock->Split(remainderBlock, alignedSize);
        HEAP_CHK_STATUS(AddBlockToSortedList(remainderBlock, remainderBlock->GetState()));
    }
    if (staticBlock)
    {
        freeBlock->SetStatic();
    }
    HEAP_CHK_STATUS(freeBlock->Allocate(trackerId));
    HEAP_CHK_STATUS(AddBlockToSortedList(freeBlock, freeBlock->GetState()));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MemoryBlockManager::AllocateBlock(
    uint32_t alignedSize,
    uint32_t index,
    uint32_t trackerId,
    bool staticBlock,
    MemoryBlockInternal *freeBlock)
{
    HEAP_FUNCTION_ENTER_VERBOSE;

    HEAP_CHK_NULL(freeBlock);

    if (!m_useProducer)
    {
        HEAP_ASSERTMESSAGE("FrameTrackerProducer need to be registered");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if (alignedSize == 0 || alignedSize > freeBlock->GetSize())
    {
        HEAP_ASSERTMESSAGE("Size requested is invalid");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if (freeBlock->GetState() != MemoryBlockInternal::State::free)
    {
        HEAP_ASSERTMESSAGE("Free block is not free");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    HEAP_CHK_STATUS(RemoveBlockFromSortedList(freeBlock, freeBlock->GetState()));

    if (alignedSize < freeBlock->GetSize())
    {
        auto remainderBlock = GetBlockFromPool();
        HEAP_CHK_NULL(remainderBlock);
        freeBlock->Split(remainderBlock, alignedSize);
        HEAP_CHK_STATUS(AddBlockToSortedList(remainderBlock, remainderBlock->GetState()));
    }
    if (staticBlock)
    {
        freeBlock->SetStatic();
    }
    HEAP_CHK_STATUS(freeBlock->Allocate(index, trackerId, m_trackerProducer));
    HEAP_CHK_STATUS(AddBlockToSortedList(freeBlock, freeBlock->GetState()));

    return MOS_STATUS_SUCCESS;
}


MOS_STATUS MemoryBlockManager::AddBlockToSortedList(
    MemoryBlockInternal *block,
    MemoryBlockInternal::State state)
{
    HEAP_FUNCTION_ENTER_VERBOSE;

    HEAP_CHK_NULL(block);

    if (block->m_statePrev || block->m_stateNext)
    {
        HEAP_ASSERTMESSAGE("Inserted blocks should not be in a state list already");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if (state != block->GetState() ||
        block->m_stateListType != MemoryBlockInternal::State::stateCount)
    {
        HEAP_ASSERTMESSAGE("State does not match list to be added to");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    auto curr = m_sortedBlockList[state];

    switch (state)
    {
        case MemoryBlockInternal::State::free:
        {
            bool inserted = false;
            MemoryBlockInternal *prev = nullptr;
            while (curr != nullptr)
            {
                if (curr->GetSize() <= block->GetSize())
                {
                    if (prev)
                    {
                        prev->m_stateNext = block;
                    }
                    else
                    {
                        m_sortedBlockList[state] = block;
                    }
                    curr->m_statePrev = block;
                    block->m_statePrev = prev;
                    block->m_stateNext = curr;
                    inserted = true;
                    break;
                }
                prev = curr;
                curr = curr->m_stateNext;
            }
            if (!inserted)
            {
                if (prev == nullptr)
                {
                    block->m_stateNext = m_sortedBlockList[state];
                    m_sortedBlockList[state] = block;
                }
                else
                {
                    block->m_statePrev = prev;
                    prev->m_stateNext = block;
                }
            }
            block->m_stateListType = state;
            m_sortedBlockListNumEntries[state]++;
            m_sortedBlockListSizes[state] += block->GetSize();
            break;
        }
        case MemoryBlockInternal::State::allocated:
        case MemoryBlockInternal::State::submitted:
        case MemoryBlockInternal::State::deleted:
            block->m_stateNext = curr;
            if (curr)
            {
                curr->m_statePrev = block;
            }
            m_sortedBlockList[state] = block;
            block->m_stateListType = state;
            m_sortedBlockListNumEntries[state]++;
            m_sortedBlockListSizes[state] += block->GetSize();
            break;
        case MemoryBlockInternal::State::pool: 
            block->m_stateNext = curr;
            if (curr)
            {
                curr->m_statePrev = block;
            }
            block->m_stateListType = state;
            m_sortedBlockList[state] = block;
            m_sortedBlockListNumEntries[state]++;
            break;
        default:
            HEAP_ASSERTMESSAGE("This state type is unsupported");
            return MOS_STATUS_INVALID_PARAMETER;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MemoryBlockManager::RemoveBlockFromSortedList(
    MemoryBlockInternal *block,
    MemoryBlockInternal::State state)
{
    HEAP_FUNCTION_ENTER_VERBOSE;

    HEAP_CHK_NULL(block);

    switch (state)
    {
        case MemoryBlockInternal::State::free:
        case MemoryBlockInternal::State::allocated:
        case MemoryBlockInternal::State::submitted:
        case MemoryBlockInternal::State::deleted:
        {
            if (block->m_statePrev)
            {
                block->m_statePrev->m_stateNext = block->m_stateNext;
            }
            else
            {
                // special case for beginning of list
                m_sortedBlockList[state] = block->m_stateNext;
            }
            if (block->m_stateNext)
            {
                block->m_stateNext->m_statePrev = block->m_statePrev;
            }
            block->m_statePrev = block->m_stateNext = nullptr;
            block->m_stateListType = MemoryBlockInternal::State::stateCount;
            m_sortedBlockListNumEntries[state]--;
            m_sortedBlockListSizes[state] -= block->GetSize();
            break;
        }
        default:
            HEAP_ASSERTMESSAGE("This state type is unsupported");
            return MOS_STATUS_INVALID_PARAMETER;
            break;
    }

    return MOS_STATUS_SUCCESS;
}

MemoryBlockInternal *MemoryBlockManager::GetBlockFromPool()
{
    HEAP_FUNCTION_ENTER_VERBOSE;

    MemoryBlockInternal *block = nullptr;

    if (m_sortedBlockList[MemoryBlockInternal::State::pool] == nullptr)
    {
        block = MOS_New(MemoryBlockInternal);
    }
    else
    {
        block = m_sortedBlockList[MemoryBlockInternal::State::pool];
        if (block->m_stateNext)
        {
            block->m_stateNext->m_statePrev = nullptr;
        }
        // refresh beginning of list
        m_sortedBlockList[MemoryBlockInternal::State::pool] = block->m_stateNext;
        block->m_statePrev = block->m_stateNext = nullptr;
        block->m_stateListType = MemoryBlockInternal::State::stateCount;
        m_sortedBlockListNumEntries[MemoryBlockInternal::State::pool]--;
    }

    return block;
}

MOS_STATUS MemoryBlockManager::RemoveHeapFromSortedBlockList(uint32_t heapId)
{
    for (auto state = 0; state < MemoryBlockInternal::State::stateCount; ++state)
    {
        if (state == MemoryBlockInternal::State::pool)
        {
            continue;
        }

        auto curr = m_sortedBlockList[state];
        Heap *heap = nullptr;
        MemoryBlockInternal *nextBlock = nullptr;
        while (curr != nullptr)
        {
            nextBlock = curr->m_stateNext;
            heap = curr->GetHeap();
            HEAP_CHK_NULL(heap);
            if (heap->GetId() == heapId)
            {
                HEAP_CHK_STATUS(RemoveBlockFromSortedList(curr, curr->GetState()));
            }
            curr = nextBlock;
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MemoryBlockManager::MergeBlocks(
    MemoryBlockInternal *blockCombined,
    MemoryBlockInternal *blockRelease)
{
    HEAP_FUNCTION_ENTER_VERBOSE;

    HEAP_CHK_NULL(blockCombined);
    HEAP_CHK_NULL(blockRelease);

    if (blockCombined->GetState() != MemoryBlockInternal::State::free ||
        blockRelease->GetState() != MemoryBlockInternal::State::free)
    {
        HEAP_ASSERTMESSAGE("Only free blocks may be merged");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    HEAP_CHK_STATUS(RemoveBlockFromSortedList(blockCombined, blockCombined->GetState()));
    HEAP_CHK_STATUS(RemoveBlockFromSortedList(blockRelease, blockRelease->GetState()));
    HEAP_CHK_STATUS(blockCombined->Combine(blockRelease));
    HEAP_CHK_STATUS(AddBlockToSortedList(blockRelease, blockRelease->GetState()));
    HEAP_CHK_STATUS(AddBlockToSortedList(blockCombined, blockCombined->GetState()));

    return MOS_STATUS_SUCCESS;
}

