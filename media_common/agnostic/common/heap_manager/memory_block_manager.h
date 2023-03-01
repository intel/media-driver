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
//! \file     memory_block_manager.h
//! \brief    Manages the memory blocks belonging to all heaps.
//! \details  Ensures that all memory is accounted for per heap for each block.
//!           Determines when blocks are no longer being processed and automatically
//!           reclaims free space in the state heap. Manages the state transitions of
//!           the memory blocks. The client has not direct access to the memory block
//!           manager.
//!

#ifndef __MEMORY_BLOCK_MANAGER_H__
#define __MEMORY_BLOCK_MANAGER_H__

#include <list>
#include <vector>
#include <memory>
#include "heap.h"
#include "memory_block.h"

class FrameTrackerProducer;

//! \brief Used by the heap manager to allow the client to access the heap memory.
class MemoryBlockManager
{
    friend class HeapManager;

public:
    //! \brief Used by the client to acquire space in heap memory.
    class AcquireParams
    {
    public:
        //! \brief initializes AcquireParams with a tracker ID
        AcquireParams(
            uint32_t trackerId,
            std::vector<uint32_t> &blockSizes) : m_blockSizes(blockSizes)
        {
            m_trackerId = trackerId;
        }
        virtual ~AcquireParams() {}

        //! \brief Block sizes to allocate
        std::vector<uint32_t>   &m_blockSizes;
        //! \brief Requested block alignment
        uint32_t                m_alignment = 0;
        //! \brief Index to the frame tracker
        uint32_t                m_trackerIndex = 0;
        //! \brief Must be valid, used for determining whether or not a the block is still in use. \see m_trackerData.
        uint32_t                m_trackerId = MemoryBlockInternal::m_invalidTrackerId;
        //! \brief Zero memory blocks after allocation
        bool                    m_zeroAssignedBlock = false;
        //! \brief Block allocations are flaged as static are expected to be controlled by the client. \see HeapManager::Behavior::clientControlled.
        bool                    m_staticBlock = false;
    };

    MemoryBlockManager() { HEAP_FUNCTION_ENTER; }
    virtual ~MemoryBlockManager();

protected:
    //!
    //! \brief  Acquires space within the heap(s)
    //! \param  [in] params
    //!         Parameters describing the requested space
    //! \param  [out] blocks
    //!         A vector containing the memory blocks allocated
    //! \param  [out] spaceNeeded
    //!         Amount of space that the heap(s) are short of to complete space acquisition
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AcquireSpace(
        AcquireParams &params,
        std::vector<MemoryBlock> &blocks,
        uint32_t &spaceNeeded);

    //!
    //! \brief  Indicates that the client is done editing the blocks
    //! \param  [in] blocks
    //!         Blocks to be submitted
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SubmitBlocks(std::vector<MemoryBlock> &blocks);

    //!
    //! \brief   Either directly adds a block to the free list, or prepares it to be added
    //! \param   [in] block
    //!          Block to be removed
    //! \return  MOS_STATUS
    //!          MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS ClearSpace(MemoryBlock &block);

    //! \brief   Reclaims free memory based on whether or not blocks are no longer in use \see m_trackerData
    //! \param   [out] blocksUpdated
    //!          If true blocks have been updated, if false, no blocks were updated
    //! \return  MOS_STATUS
    //!          MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS RefreshBlockStates(bool &blocksUpdated);

    //!
    //! \brief  Stores heap and initializes memory blocks for future use.
    //! \param  [in] heapId
    //!         ID for the heap to be created
    //! \param  [in] size
    //!         Size of the heap to be created.
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS RegisterHeap(uint32_t heapId, uint32_t size, bool hwWriteOnly = false);

    //!
    //! \brief  Removes the specified heap from the block manager, this occurs when the
    //!         heap is in the process of being deleted. Ensures heap is not in use.
    //! \param  [in] heapId
    //!         ID for heap to be removed.
    //!
    virtual MOS_STATUS UnregisterHeap(uint32_t heapId);

    //!
    //! \brief  Completes heap deletion for all heaps with unused space in the deleted heaps list.
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS CompleteHeapDeletion();

    //!
    //! \brief  Registers the tracker data to be used for determining whether a
    //!         memory block is available.
    //! \param  [in] trackerData
    //!         Must be valid. See description in \see MemoryBlockManager::m_trackerData.
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS RegisterTrackerResource(uint32_t *trackerData);

    //!
    //! \brief  Registers the tracker producer to be used for determining whether a
    //!         memory block is available. This function has a higher priority than
    //!         RegisterTrackerResource, so if it is called, the trackerResource will
    //!         be useless.
    //! \param  [in] trackerProducer
    //!         Must be valid; pointer to trackerProducer.
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS RegisterTrackerProducer(FrameTrackerProducer *trackerProducer);

    //!
    //! \brief  Gets the size of the all heaps
    //! \return The size of all heaps managed \see m_size
    //!
    uint32_t GetSize() { return m_totalSizeOfHeaps; }

    //!
    //! \brief  Determines whether a valid tracker data has been registered
    //! \return True if the pointer is valid, false otherwise
    //!
    bool IsTrackerDataValid()
    {
        return (!m_useProducer && m_trackerData != nullptr)
            || (m_useProducer && m_trackerProducer != nullptr);
    }

    //!
    //! \brief   All heaps allocated are locked and kept locked for their lifetimes
    //! \details May only be set before any heaps are allocated.
    //! \return  MOS_STATUS
    //!          MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS LockHeapsOnAllocate()
    {
        HEAP_FUNCTION_ENTER;
        if (m_totalSizeOfHeaps != 0)
        {
            HEAP_ASSERTMESSAGE("Locking heaps on allocate may only be set before heaps are allocated");
            return MOS_STATUS_UNKNOWN;
        }
        m_lockHeapsOnAllocate = true;
        return MOS_STATUS_SUCCESS;
    }

private:
    //! \brief Describes the information managed by this class for each heap registered
    struct HeapWithAdjacencyBlockList
    {
        HeapWithAdjacencyBlockList()
        {
            HEAP_FUNCTION_ENTER;
        }

        virtual ~HeapWithAdjacencyBlockList()
        {
            HEAP_FUNCTION_ENTER;
            MOS_Delete(m_heap);
            auto curr = m_adjacencyListBegin;
            MemoryBlockInternal *next = nullptr;

            while (curr != nullptr)
            {
                next = curr->GetNext();
                MOS_Delete(curr);
                curr = next;
            }
        }

        //! \brief Heap which all other members of this struct define.
        Heap *m_heap = nullptr;
        //! \brief The list of blocks in the heap arranged by offset in the heap. The head is a dummy block.
        MemoryBlockInternal *m_adjacencyListBegin = nullptr;
        //! \brief The size of the heap
        uint32_t m_size = 0;
    };

    //!
    //! \brief  Stores the provided OS interface in the heap
    //! \param  [in] osInterface
    //!         Must be valid
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!  
    virtual MOS_STATUS RegisterOsInterface(PMOS_INTERFACE osInterface);

    //!
    //! \brief  Determines whether or not space is available, if not enough space returns
    //!         the amount short in \a spaceNeeded
    //! \param  [in] sortedSizes
    //!         Requested block sizes sorted in decending order.
    //! \param  [in] params
    //!         Parameters describing the requested space
    //! \param  [out] spaceNeeded
    //!         Amount of space that the heap(s) are short of to complete space acquisition
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS IsSpaceAvailable(
        AcquireParams &params,
        uint32_t &spaceNeeded);

    //!
    //! \brief  Sets up memory blocks for the requested space
    //! \param  [in] sortedSizes
    //!         Requested block sizes sorted in decending order with original index saved from params.
    //! \param  [in] params
    //!         Parameters describing the requested space
    //! \param  [out] blocks
    //!         A vector containing the memory blocks allocated
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateSpace(
        AcquireParams &params,
        std::vector<MemoryBlock> &blocks);

    //!
    //! \brief  Sets up memory blocks for the requested space
    //! \param  [in] alignedSize
    //!         Aligned size of the memory requested
    //! \param  [in] trackerId
    //!         Tracker ID to be used for the allocated block
    //! \param  [in] staticBlock
    //!         Indicates that the newly created block is expected to be static
    //! \param  [out] freeBlock
    //!         Free block that will be used for the allocation of the new block
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateBlock(
        uint32_t alignedSize,
        uint32_t trackerId,
        bool staticBlock,
        MemoryBlockInternal *freeBlock);

    //!
    //! \brief  Sets up memory blocks for the requested space
    //! \param  [in] alignedSize
    //!         Aligned size of the memory requested
    //! \param  [in] trackerIndex
    //!         Tracker index to be used for the allocated block
    //! \param  [in] trackerId
    //!         Tracker ID to be used for the allocated block
    //! \param  [in] staticBlock
    //!         Indicates that the newly created block is expected to be static
    //! \param  [out] freeBlock
    //!         Free block that will be used for the allocation of the new block
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateBlock(
        uint32_t alignedSize,
        uint32_t trackerIndex,
        uint32_t trackerId,
        bool staticBlock,
        MemoryBlockInternal *freeBlock);

    //!
    //! \brief  Sets up memory blocks for the requested space
    //! \param  [in] block
    //!         Block to be added to the sorted pool of type \a state
    //! \param  [in] state
    //!         Type of pool to be added to
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AddBlockToSortedList(
        MemoryBlockInternal *block,
        MemoryBlockInternal::State state);

    //!
    //! \brief  Sets up memory blocks for the requested space
    //! \param  [in] block
    //!         Block to be added to the sorted pool of type \a state
    //! \param  [in] state
    //!         Type of pool to be added to, may not be of type MemoryBlockInternal::State::pool since a separate function is dedicated to that case. \see GetBlockFromPool
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS RemoveBlockFromSortedList(
        MemoryBlockInternal *block,
        MemoryBlockInternal::State state);

    //!
    //! \brief  Gets a pool type block from the sorted block pool, if pool is empty allocates a new one
    //!         \see m_sortedBlockList[MemoryBlockInternal::State::pool]
    //! \return MemoryBlockInternal*
    //!         Valid pointer if success, nullptr if fail
    //!
    MemoryBlockInternal* GetBlockFromPool();

    //!
    //! \brief  Removes all blocks with heaps matching \a heapId from the sorted block pool for \a state. \see m_sortedBlockList
    //! \param  [in] heapId
    //!         Heap whose blocks should be removed
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS RemoveHeapFromSortedBlockList(
        uint32_t heapId);

    //!
    //! \brief  Merges two contiguous blocks into one.
    //! \param  [in,out] blockCombined
    //!         Block into which \a blockRelease will be merged in to, this block is the output of the merge
    //! \param  [in] blockRelease
    //!         Block to be merged into \a blockCombined, this block will be added back to the pool after the merge
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS MergeBlocks(
        MemoryBlockInternal *blockCombined,
        MemoryBlockInternal *blockRelease);

    //!
    //! \brief  Temporary function while MOS utilities function is added for smart pointers
    //! \return std::shared_ptr<T>
    //!         Valid pointer if success, nullptr if fail
    //!
    template <class T, class... Args>
    std::shared_ptr<T> MakeShared(Args&&... args)
    {
        std::shared_ptr<T> block = nullptr;
        try
        {
            block = std::make_shared<T>(args...);
        }
        catch (const std::bad_alloc& e)
        {
            HEAP_ASSERTMESSAGE("Allocation of memory block failed!");
            return nullptr;
        }
        return block;
    }


    //! \brief  Used to retain information about the requested block sizes after those
    //!         sizes are sorted for the return of blocks.\see AcquireParams::m_blockSizes \see AcquireSpace
    struct SortedSizePair
    {
        //! \brief Constructs SortedSizePair
        SortedSizePair() {}
        SortedSizePair(uint32_t originalIdx, uint32_t blockSize)
            : m_originalIdx(originalIdx), m_blockSize(blockSize) {}
        uint32_t m_originalIdx = 0; //!< Original index of the requested block size \see AcquireParams::m_blockSizes
        uint32_t m_blockSize = 0;   //!< Aligned block size
    };

    //! \brief Alignment for blocks in heap, currently fixed at a cacheline
    static const uint16_t m_blockAlignment = 64;
    //! \brief Alignment for heap, currently fixed at a page
    static const uint16_t m_heapAlignment = MOS_PAGE_SIZE;
    //! \brief Number of submissions before a refresh, currently fixed
    static const uint16_t m_numSubmissionsForRefresh = 128;

    //! \brief Total size of all managed heaps.
    uint32_t m_totalSizeOfHeaps = 0;
    //! \brief The list of block pools per heap
    std::list<std::shared_ptr<HeapWithAdjacencyBlockList>> m_heaps;
    //! \brief List of block pools per heap for heaps in deletion process
    std::list<std::shared_ptr<HeapWithAdjacencyBlockList>> m_deletedHeaps;
    //! \brief Pools of memory blocks sorted by their states based on the state indicated
    //!        by the latest TrackerId. The free pool is sorted in ascending order.
    MemoryBlockInternal *m_sortedBlockList[MemoryBlockInternal::State::stateCount] = {nullptr};
    //! \brief Number of entries in each sorted block list.
    uint32_t m_sortedBlockListNumEntries[MemoryBlockInternal::State::stateCount] = {0};
    //! \brief Sizes of each block pool.
    //! \brief MemoryBlockInternal::State::pool type blocks have no size, and thus that pool also is expected to be size 0.
    uint32_t m_sortedBlockListSizes[MemoryBlockInternal::State::stateCount] = {0};
    //! \brief   Used to compare to a memory block's tracker ID. If the value is greater
    //!          than the tracker ID, then the block is no longer in use and may be reclaimed
    //!          as free space, or if the block is static transitioned to allocated state so
    //!          it may be re-used. \see MemoryBlockInternal::m_trackerId
    //! \details The client is expected to manage this tracker; it should be valid for the
    //!          life of the heap manager. The tracker may be updated at whatever granularity is
    //!          appropriate--per workload, per frame, etc. It is expected that when the
    //!          tracker data indicates that all blocks with tracker IDs less than or equal
    //!          to the udpated value may be re-used.
    uint32_t *m_trackerData = nullptr;
    PMOS_INTERFACE m_osInterface = nullptr; //!< OS interface used for managing graphics resources
    bool m_lockHeapsOnAllocate = false;             //!< All heaps allocated with the keep locked flag.
    
    //! \brief Persistent storage for the sorted sizes used during AcquireSpace()
    std::list<SortedSizePair> m_sortedSizes;
    //! \brief TrackerProducer
    FrameTrackerProducer *m_trackerProducer = nullptr;
    //! \bried Whether trackerProducer is set
    bool m_useProducer = false;
};
#endif // __MEMORY_BLOCK_MANAGER_H__
