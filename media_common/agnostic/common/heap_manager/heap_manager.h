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
//! \file     heap_manager.h
//! \brief    The client facing interface for handling heaps.
//!

#ifndef __HEAP_MANAGER_H__
#define __HEAP_MANAGER_H__

#include "memory_block_manager.h"

class FrameTrackerProducer;

//! \brief Client accessible manager for heaps.
class HeapManager
{

public:
    //! \brief Expected behavior when space is not available in the heap(s)
    enum Behavior
    {
        wait = 0,           //<! Waits for space to become available.
        extend,             //<! Allocates a new heap of the existing size + the default extend heap size.
        destructiveExtend,  //<! Performs the same action as extend and marks the old heap for delete.
        waitAndExtend,      //<! Waits for a fixed amount of time and then performs an extend if a timeout occurs.
        clientControlled    //<! If space is not available, heap manager returns so client can decide what to do
    };

    HeapManager()
    {
        HEAP_FUNCTION_ENTER;
    }
    virtual ~HeapManager();

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
        MemoryBlockManager::AcquireParams &params,
        std::vector<MemoryBlock> &blocks,
        uint32_t &spaceNeeded);

    //!
    //! \brief  Indicates that the client is done editing the blocks
    //! \param  [in] blocks
    //!         Blocks to be submitted
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SubmitBlocks(std::vector<MemoryBlock> &blocks)
    {
        return m_blockManager.SubmitBlocks(blocks);
    }

    //!
    //! \brief   Makes a block available in the heap.
    //! \details Expected to be used only when the behavior selected in HeapManager is client
    //!          controlled--the client wants direct control over what is removed from the heap
    //!          (static blocks would be used in this case).
    //! \param   [in] block
    //!          Block to be removed
    //! \return  MOS_STATUS
    //!          MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ClearSpace(MemoryBlock &block);

    //!
    //! \brief  Registers the tracker data to be used for determining whether a
    //!         memory block is available.
    //! \param  [in] trackerData
    //!         Must be valid; pointer to tracker data. \see MemoryBlockManager::m_trackerData.
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
    MOS_STATUS RegisterTrackerProducer(FrameTrackerProducer *trackerProducer);

    //!
    //! \brief  Updates the default behavior of the heap manager
    //! \param  [in] behavior
    //!         Expected behavior of the heap mamanger
    //!
    void SetDefaultBehavior(Behavior behavior)
    { 
        HEAP_FUNCTION_ENTER;
        if (behavior > clientControlled)
        {
            behavior = wait;
        }
        m_behavior = behavior;
    }

    //!
    //! \brief  Updates the initial heap size either for first allocation or for extension
    //! \param  [in] size
    //!         Updates the current heap size, must be non-zero. \see m_currHeapSize
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetInitialHeapSize(uint32_t size);

    //!
    //! \brief  Updates the extend heap size when the behavior is not to wait. \see m_behavior \see Behavior::wait
    //! \param  [in] size
    //!         Updates the extend heap size, must be non-zero. \see m_extendHeapSize
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetExtendHeapSize(uint32_t size);

    //!
    //! \brief  Stores the provided OS interface in the heap
    //! \param  [in] osInterface
    //!         Must be valid
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!  
    virtual MOS_STATUS RegisterOsInterface(PMOS_INTERFACE osInterface);

    //!
    //! \brief  Indicates the total size of all managed heaps
    //! \return The size of all heaps managed \see m_totalSizeOfHeaps
    //!  
    virtual uint32_t GetTotalSize();

    //!
    //! \brief  Indicates the size of heap extensions
    //! \return The size by which heaps are extended by \see m_extendHeapSize
    //!  
    uint32_t GetExtendSize()
    {
        HEAP_FUNCTION_ENTER;
        return m_extendHeapSize;
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
        return m_blockManager.LockHeapsOnAllocate();
    }

    //!
    //! \brief  Mark the heap as hardware write only heap or not
    void SetHwWriteOnlyHeap(bool isHwWriteOnlyHeap) { m_hwWriteOnlyHeap = isHwWriteOnlyHeap; }

private:
    //!
    //! \brief  Allocates a heap of requested size
    //! \param  [in] size
    //!         Size of the heap to be allocated
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateHeap(uint32_t size);

    //!
    //! \brief  Free or attempts to free a heap
    //!
    void FreeHeap();

    //!
    //! \brief  Wait for for space to be available in the heap
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Wait();

    //!
    //! \brief  If space may not be acquired, behave according to the current specified behavior.
    //!         \see m_behavior \see MemoryBlockManager::AcquireSpace
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS BehaveWhenNoSpace();

    //! \brief Alignment used for the heap size during allocation
    static const uint32_t m_heapAlignment = MOS_PAGE_SIZE;
    //! \brief Timeout in milliseconds for wait, currently fixed
    static const uint32_t m_waitTimeout = 100;
    //! \brief Wait increment in milliseconds, currently fixed
    static const uint32_t m_waitIncrement = 10;

    //! \brief Memory block manager for the heap(s)
    MemoryBlockManager m_blockManager;
    //! \brief The default behavior when space is not found in any of the heaps.
    Behavior m_behavior = Behavior::wait;
    //! \brief Current heap ID
    uint32_t m_currHeapId = Heap::m_invalidId;
    //! \brief The current heap size
    uint32_t m_currHeapSize = 0;
    //! \brief The size by which a heap is expected to be extended by when the behavior
    //!        is extend.
    uint32_t m_extendHeapSize = 0;
    //! \brief Stores the IDs for the heaps stored in the memory block manager
    std::list<uint32_t> m_heapIds;
    //! \brief OS interface used for managing graphics resources
    PMOS_INTERFACE m_osInterface = nullptr;
    //!< Indictaes that heap is used by hardware write only.
    bool m_hwWriteOnlyHeap = false;
};

#endif // __HEAP_MANAGER_H__
