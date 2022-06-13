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
//! \file     memory_block.h
//! \brief    A memory block is a chunk of memory in a heap.
//! \details  All heap memory is described with memory blocks. Each block has a state
//!           which determines how it may be used and what other states it may be
//!           transferred to. The client has limited direct access to a memory block--
//!           data may be added to the memory location referred to nothing else may be
//!           modified.
//!

#ifndef __MEMORY_BLOCK_H__
#define __MEMORY_BLOCK_H__

#include <memory>
#include <string>
#include "heap.h"
#include "frame_tracker.h"

//! \brief   Describes a block of memory in a heap.
//! \details For internal use by the MemoryBlockManager only.
class MemoryBlockInternal
{
    friend class MemoryBlockManager;

public:
    MemoryBlockInternal() { HEAP_FUNCTION_ENTER_VERBOSE; }

    virtual ~MemoryBlockInternal() { HEAP_FUNCTION_ENTER; }

    //!
    //! \brief  Add data to the memory block.
    //! \param  [in] data
    //!         Pointer containint data to insert, must be valid
    //! \param  [in] dataOffset
    //!         Relative offset where the data should be inserted within the
    //!         memory block.
    //! \param  [in] dataSize
    //!         Size of the data to be copied from \a data.
    //! \param  [in] zeroBlock
    //!         Zeros the entire block, other parameters are not considered
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!       
    MOS_STATUS AddData(
        void* data,
        uint32_t dataOffset,
        uint32_t dataSize,
        bool zeroBlock = false);

    //!
    //! \brief  Read data from the memory block.
    //! \param  [out] data
    //!         Pointer to data returned from memory block, must be valid
    //! \param  [in] dataOffset
    //!         Relative offset where the data will be read from within the
    //!         memory block.
    //! \param  [in] dataSize
    //!         Size of the data to be copied to \a data.
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!       
    MOS_STATUS ReadData(
        void* data,
        uint32_t dataOffset,
        uint32_t dataSize);

    //!  
    //! \brief  Dumps the contents of this memory block
    //! \param  [in] filename
    //!         The file to be written, includes path
    //! \param  [in] offset
    //!         Additional offset to be added to this memory block's offest
    //! \param  [in] size
    //!         Size of the data to be dumped, cannot be greater than this memory block's size
    //! \param  [in] dumpInBinary
    //!         Dump the data as binary instead of 32-bit hex segments
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!  
    MOS_STATUS Dump(
        std::string filename,
        uint32_t offset = 0,
        uint32_t size = 0,
        bool dumpInBinary = false);

    //!
    //! \brief  Gets the relative offset of the memory block
    //! \return The relative offset of this block in the heap \see m_offset
    //!
    uint32_t GetOffset() { return m_offset; }

    //!
    //! \brief  Gets the size of the memory block
    //! \return The size of this block \see m_size
    //!
    uint32_t GetSize() { return m_size; }

    //!
    //! \brief  Gets the tracker ID
    //! \return This block's tracker ID \see m_trackerId
    //!
    uint32_t GetTrackerId() { return m_trackerId; }

    //!
    //! \brief  Gets the tracker Producer
    //! \return This block's tracker producer \see m_trackerProducer
    //!
    FrameTrackerToken *GetTrackerToken() { return &m_trackerToken; }

    //!
    //! \brief  Indicates whether or not the memory block is static
    //! \return If this block is static \see m_static
    //!
    bool IsStatic() { return m_static; }

    //! \brief Invalid value to be read from the tracker data \see MemoryBlockInternalManager::m_pTrackerData
    static const uint32_t m_invalidTrackerId = 0;

protected:
    //! \brief Possible states of a memory block
    //! \dot
    //! digraph states {
    //!     pool -> free [style = dotted];
    //!     free -> pool [style = dotted];
    //!     free -> allocated;
    //!     free -> deleted;
    //!     allocated -> submitted;
    //!     allocated -> deleted;
    //!     allocated -> allocated [label = "static" labeltooltip = "Static blocks are always in allocated state"];
    //!     submitted -> allocated [label = "!static" labeltooltip = "This state change requires a refresh."];
    //!     submitted -> free [label = "!static" labeltooltip = "This state change requires a refresh."];
    //!     submitted -> deleted;
    //!     deleted -> pool [style = dotted];
    //! }
    //! \enddot
    enum State
    {
        pool = 0,   //!< Memory block is blank and ready to be used by the memory block manager
        free,       //!< Memory block is available for use by the client
        allocated,  //!< Space has been acquired in the heap by the client and data may be added to the memory block
        submitted,  //!< Memory block is submitted, no further changes are permitted
        deleted,    //!< Memory block is in the process of being freed, may not be used
        stateCount  //!< Number of states possible for a memory block
    };

    //!
    //! \brief  Sets up a memory block in the specified heap
    //! \param  [in] heap
    //!         Heap to which this memory block blongs.
    //! \param  [in] requestedState
    //!         Requested state for the block.
    //! \param  [in] prev
    //!         Memory block which is adjacent to this new block with a lower offset.
    //! \param  [in] offset
    //!         Relative offset within \a heap where this memory block is located
    //! \param  [in] size
    //!         Size of the memory block.
    //! \param  [in] trackerId
    //!         TrackerId associated with this memory block
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Create(
        Heap *heap,
        State requestedState,
        MemoryBlockInternal *prev,
        uint32_t offset,
        uint32_t size,
        uint32_t trackerId);

    //!
    //! \brief   Adjusts this block's information such that it may be combined with \a block.
    //! \details After combining, \a block will be returned to a pool state and this block
    //!          will have expanded forward or backward to encompass \a block.
    //! \param   [in] block
    //!          Must be adjacent to this block to be combined.
    //! \return  MOS_STATUS
    //!          MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Combine(MemoryBlockInternal *block);

    //!
    //! \brief  Downsizes this block to \a size, puts the remainder into a new block \a block.
    //! \param  [in,out] block
    //!         Must be in a pool state. Block which takes some of this blocks space.
    //! \param  [in] size
    //!         Size of the new block.
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Split(MemoryBlockInternal *block, uint32_t size);

    //!
    //! \brief  Indicates that the block is a placeholder until its state changes
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Pool();

    //!
    //! \brief  Indicates that the block is ready for re-use
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Free();

    //!
    //! \brief  Indicates that the block may be edited
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Allocate(uint32_t trackerId);

    //!
    //! \brief  Indicates that the block may be edited
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Allocate(uint32_t index, uint32_t trackerId, FrameTrackerProducer *producer = nullptr);

    //!
    //! \brief  Indicates that the client is done editing the memory block.
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Submit();

    //!
    //! \brief  Indicates that the heap is in the process of being freed.
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Delete();

    //!
    //! \brief  Indicates the state of the memory block
    //! \return This block's state \see m_state
    //!
    State GetState() { return m_state; }

    //!
    //! \brief  Forces \see m_static to false
    //!
    void ClearStatic() { m_static = false; }

    //!
    //! \brief  Forces \see m_static to true
    //!
    void SetStatic() { m_static = true; }

    //!
    //! \brief  Gets the previous memory block
    //! \return The previous adjacent block \see m_prev
    //!
    MemoryBlockInternal *GetPrev() { return m_prev; }

    //!
    //! \brief  Gets the next memory block
    //! \return The next adjacent block \see m_next
    //!
    MemoryBlockInternal *GetNext() { return m_next; }

    //!
    //! \brief  Gets the heap of the memory block
    //! \return The heap that this block belongs to \see m_heap
    //!
    Heap *GetHeap()
    {
        if (m_heap != nullptr)
        {
            if (!m_heap->IsValid())
            {
                HEAP_ASSERTMESSAGE("Memory block does not have a valid heap to return");
                return nullptr;
            }
        }
        return m_heap;
    }

private:
    //! \brief The heap referred to by the memory block
    Heap *m_heap = nullptr;
    //! \brief The relative offset of the memory block within the heap.
    uint32_t m_offset = 0;
    //! \brief Size of the memory block.
    uint32_t m_size = 0;
    //! \brief State of this memory block
    State m_state = State::pool;
    //! \brief   Static memory blocks are controlled by the client and may not be auto-freed
    //! \details Since the client controls static blocks, static blocks are allowed to have
    //!          invalid tracker IDs.
    bool m_static = false;
    //! \brief Software tag used to determine whether or not a memory block is still in use.
    uint32_t m_trackerId = m_invalidTrackerId;
    //! \brief Multiple software tags used to determine whether or not a memory block is still in use.
    FrameTrackerToken m_trackerToken;

    //! \brief   The previous block in memory, this block is adjacent in heap memory.
    //! \details Due to the way that the memory manager handles heap memory block lists--by having
    //!          a dummy block at the start of a heaps block list, this pointer is always expected to
    //!          be valid for non-pool state blocks.
    MemoryBlockInternal *m_prev = nullptr;
    //! \brief   The next block in memory, this block is adjacent in heap memory.
    //! \details If this block is at the end of the heap, the next block is expected to be nullptr.
    MemoryBlockInternal *m_next = nullptr;

    //! \brief Previous sorted block, nullptr if this block is the first in the sorted list
    MemoryBlockInternal *m_statePrev = nullptr;
    //! \brief Next block in sorted list, nullptr if this block is last in the sorted list
    MemoryBlockInternal *m_stateNext = nullptr;
    //! \brief State type for the sorted list to which this block belongs, if between lists type is stateCount
    State m_stateListType = State::stateCount;
};

//! \brief Describes a block of memory in a heap.
class MemoryBlock
{
    friend class MemoryBlockManager;

public:
    MemoryBlock()
    {
        HEAP_FUNCTION_ENTER_VERBOSE;
        m_resource = nullptr;
    }

    virtual ~MemoryBlock() { HEAP_FUNCTION_ENTER_VERBOSE; }

    //!
    //! \brief  Add data to the memory block.
    //! \param  [in] data
    //!         Pointer containint data to insert, must be valid
    //! \param  [in] dataOffset
    //!         Relative offset where the data should be inserted within the
    //!         memory block.
    //! \param  [in] dataSize
    //!         Size of the data to be copied from \a data.
    //! \param  [in] zeroBlock
    //!         Zeros the entire block, other parameters are not considered
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!       
    MOS_STATUS AddData(
        void* data,
        uint32_t dataOffset,
        uint32_t dataSize,
        bool zeroBlock = false)
    {
        if (!m_valid || m_block == nullptr)
        {
            HEAP_ASSERTMESSAGE("The memory block is not valid!");
            return MOS_STATUS_INVALID_PARAMETER;
        }
        return m_block->AddData(data, dataOffset, dataSize, zeroBlock);
    }

    //!
    //! \brief  Read data from the memory block.
    //! \param  [out] data
    //!         Pointer to data returned from memory block, must be valid
    //! \param  [in] dataOffset
    //!         Relative offset where the data will be read from within the
    //!         memory block.
    //! \param  [in] dataSize
    //!         Size of the data to be copied to \a data.
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!       
    MOS_STATUS ReadData(
        void* data,
        uint32_t dataOffset,
        uint32_t dataSize)
    {
        if (!m_valid || m_block == nullptr)
        {
            HEAP_ASSERTMESSAGE("The memory block is not valid!");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        return m_block->ReadData(data, dataOffset, dataSize);
    }

    //!  
    //! \brief  Dumps the contents of this memory block
    //! \param  [in] filename
    //!         The file to be written, includes path
    //! \param  [in] offset
    //!         Additional offset to be added to this memory block's offest
    //! \param  [in] size
    //!         Size of the data to be dumped, cannot be greater than this memory block's size
    //! \param  [in] dumpInBinary
    //!         Dump the data as binary instead of 32-bit hex segments
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!  
    MOS_STATUS Dump(
        std::string filename,
        uint32_t offset = 0,
        uint32_t size = 0,
        bool dumpInBinary = false)
    {
        if (!m_valid || m_block == nullptr)
        {
            HEAP_ASSERTMESSAGE("The memory block is not valid!");
            return MOS_STATUS_INVALID_PARAMETER;
        }
        return m_block->Dump(filename, offset, size, dumpInBinary);
    }

    //!
    //! \brief  Indicates whether or not the memory block is static
    //! \return If this block is static \see m_static
    //!
    bool IsValid() { return m_valid; }

    //!
    //! \brief  Gets the relative offset of the memory block
    //! \return The relative offset of this block in the heap \see m_offset
    //!
    uint32_t GetOffset() { return m_offset; }

    //!
    //! \brief  Gets the size of the memory block
    //! \return The size of this block \see m_size
    //!
    uint32_t GetSize() { return m_size; }

    //!
    //! \brief  Gets the tracker ID
    //! \return This block's tracker ID \see m_trackerId
    //!
    uint32_t GetTrackerId() { return m_trackerId; }

    //!
    //! \brief  Indicates whether or not the memory block is static
    //! \return If this block is static \see m_static
    //!
    bool IsStatic() { return m_static; }

    //!
    //! \brief  Gets the heap of the memory block
    //! \return The heap that this block belongs to \see m_heap
    //!
    MOS_RESOURCE* GetResource()
    {
        HEAP_FUNCTION_ENTER_VERBOSE;
        if (!m_valid)
        {
            HEAP_ASSERTMESSAGE("The memory block is not valid!");
            return nullptr;
        }
        if (Mos_ResourceIsNull(m_resource))
        {
            HEAP_ASSERTMESSAGE("The heap resource is invalid");
            return nullptr;
        }
        return m_resource;
    }

    //!
    //! \brief  Gets the size of the heap to which the memory block belongs
    //! \return The size of the associated heap \see m_heapSize
    //!
    uint32_t GetHeapSize() { return m_heapSize; }

#if (_DEBUG || _RELEASE_INTERNAL)
    //!
    //! \brief   Gets the locked heap
    //! \details To be used when the memory block dump is insufficient.
    //! \return  Pointer to the locked heap data \see m_lockedHeap
    //!
    uint8_t* GetHeapLockedData() { return m_lockedHeap; }
#endif

    //! \brief Invalid value to be read from the tracker data \see MemoryBlockInternalManager::m_pTrackerData
    static const uint32_t m_invalidTrackerId = MemoryBlockInternal::m_invalidTrackerId;

protected:
    //!
    //! \brief  Initializes memory block based on provided inputs
    //! \param  [in] internalBlock
    //!         Internal block used to initialize this memory block
    //! \param  [in] heap
    //!         Heap used to initialize this memory block
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS CreateFromInternalBlock(
        MemoryBlockInternal *internalBlock,
        Heap *heap,
        uint8_t *lockedHeap)
    {
        HEAP_CHK_NULL(internalBlock);
        HEAP_CHK_NULL(heap);
        if (!heap->IsValid())
        {
            HEAP_ASSERTMESSAGE("Memory block does not have a valid heap to return");
            return MOS_STATUS_INVALID_PARAMETER;
        }
        HEAP_CHK_NULL(m_resource = heap->GetResource());
        m_heapSize = heap->GetSize();
#if (_DEBUG || _RELEASE_INTERNAL)
        m_lockedHeap = lockedHeap;
#endif
        m_offset = internalBlock->GetOffset();
        m_size = internalBlock->GetSize();
        m_static = internalBlock->IsStatic();
        m_trackerId = internalBlock->GetTrackerId();
        m_block = internalBlock;
        m_valid = true;
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief  Gets the internal memory block
    //! \return The internal memory block backing up this data
    //!
    MemoryBlockInternal *GetInternalBlock() { return m_block; }

private:
    bool m_valid = false;       //!< Memory block is valid.
    MOS_RESOURCE *m_resource;    //!< Graphics resource for the heap
#if (_DEBUG || _RELEASE_INTERNAL)
    uint8_t *m_lockedHeap = nullptr;    //!< Pointer to the locked heap data, only valid if kept locked
#endif
    uint32_t m_heapSize = 0;    //!< Size of the heap
    uint32_t m_offset = 0;      //!< relative offset of the memory block within the heap.
    uint32_t m_size = 0;        //!< Size of the memory block.
    //! \brief   Static memory blocks are controlled by the client and may not be auto-freed
    //! \details Since the client controls static blocks, static blocks are allowed to have
    //!          invalid tracker IDs.
    bool m_static = false;
    //! \brief Software tag used to determine whether or not a memory block is still in use.
    uint32_t m_trackerId = m_invalidTrackerId;
    MemoryBlockInternal *m_block = nullptr; //!< Reference to the internal memory block
};

#endif // __MEMORY_BLOCK_H__