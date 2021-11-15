/*
* Copyright (c) 2017-2021, Intel Corporation
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
//! \file     memory_block.cpp
//! \brief    Implements functionalities pertaining to memory blocks
//!

#include "memory_block.h"
#include "heap.h"

MOS_STATUS MemoryBlockInternal::AddData(
    void* data,
    uint32_t dataOffset,
    uint32_t dataSize,
    bool zeroBlock)
{
    HEAP_FUNCTION_ENTER_VERBOSE;

    if (m_state != allocated)
    {
        HEAP_ASSERTMESSAGE("Memory blocks may only have data added while in allocated state");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if (data == nullptr && !zeroBlock)
    {
        HEAP_ASSERTMESSAGE("No data was passed in to be added");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if (m_offset + dataOffset + dataSize > m_heap->GetSize() ||
        dataOffset + dataSize > m_size)
    {
        HEAP_ASSERTMESSAGE("Data will not fit within the this memory block or state heap");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    uint8_t *lockedResource = m_heap->Lock();
    HEAP_CHK_NULL(lockedResource);
    lockedResource += m_offset + dataOffset;

    if (zeroBlock)
    {
        memset(lockedResource, 0, m_size);
    }
    else
    {
        MOS_SecureMemcpy(
            lockedResource,
            m_size - dataOffset,
            data,
            dataSize);
    }

    m_heap->Unlock();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MemoryBlockInternal::ReadData(
    void* data,
    uint32_t dataOffset,
    uint32_t dataSize)
{
    HEAP_FUNCTION_ENTER_VERBOSE;

    if (data == nullptr)
    {
        HEAP_ASSERTMESSAGE("Pointer for data to be read back must be valid");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if (m_offset + dataOffset + dataSize > m_heap->GetSize() ||
        dataOffset + dataSize > m_size)
    {
        HEAP_ASSERTMESSAGE("Data attempting to read is outside this memory block or state heap");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    uint8_t *lockedResource = m_heap->Lock();
    HEAP_CHK_NULL(lockedResource);
    lockedResource += m_offset + dataOffset;

    MOS_SecureMemcpy(
        data,
        dataSize,
        lockedResource,
        dataSize);

    m_heap->Unlock();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MemoryBlockInternal::Dump(
    std::string filename,
    uint32_t offset,
    uint32_t size,
    bool dumpInBinary)
{
    HEAP_FUNCTION_ENTER_VERBOSE;

    if (m_state != allocated && m_state != submitted)
    {
        HEAP_ASSERTMESSAGE("Memory blocks not in allocated or submitted state do not containe valid data");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    size = size ? size : m_size;

    if (m_offset + offset + size > m_heap->GetSize() ||
        offset + size > m_size)
    {
        HEAP_ASSERTMESSAGE("Data will not fit within the this memory block or state heap");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    uint8_t *lockedResource = m_heap->Lock();
    HEAP_CHK_NULL(lockedResource);
    lockedResource += offset + m_offset;
    if (dumpInBinary)
    {
        HEAP_CHK_STATUS(MosUtilities::MosWriteFileFromPtr(
            filename.c_str(),
            lockedResource,
            size));
    }
    else
    {
        uint32_t *lockedResourceIn4Bytes = (uint32_t*)lockedResource;
        uint32_t sizeIn32BitHex = size / sizeof(uint32_t);
        uint32_t sizeOfLastHex = size % sizeof(uint32_t);
        std::string formattedData = "";
        char dataInHex[10] = {0};

        for (uint32_t i = 0; i < sizeIn32BitHex; i++)
        {
            MOS_SecureStringPrint(
                dataInHex,
                sizeof(dataInHex),
                sizeof(dataInHex),
                "%.8x ",
                lockedResourceIn4Bytes[i]);
            formattedData += dataInHex;
            if (i % 4 == 3)
            {
                formattedData += "\r\n";
            }
        }

        if (sizeOfLastHex > 0) // one last incomplete dword to print out
        {
            uint32_t lastHex = lockedResourceIn4Bytes[sizeIn32BitHex];
            lastHex = lastHex & ((4 - sizeOfLastHex) * 8);

            MOS_SecureStringPrint(dataInHex, sizeof(dataInHex), sizeof(dataInHex), "%.8x ", lastHex);
            formattedData += dataInHex;
        }

        HEAP_CHK_STATUS(MosUtilities::MosWriteFileFromPtr(
            filename.c_str(),
            (void*)formattedData.c_str(),
            formattedData.size()));
    }

    m_heap->Unlock();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MemoryBlockInternal::Create(
    Heap *heap,
    State requestedState,
    MemoryBlockInternal *prev,
    uint32_t offset,
    uint32_t size,
    uint32_t trackerId)
{
    HEAP_FUNCTION_ENTER_VERBOSE;

    HEAP_CHK_NULL(prev);

    if (m_state == State::deleted)
    {
        HEAP_ASSERTMESSAGE("Deleted blocks may not be re-used.");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if (heap == nullptr)
    {
        HEAP_ASSERTMESSAGE("Heap pointer is not valid");
        return MOS_STATUS_INVALID_PARAMETER;
    }
    else if (!heap->IsValid())
    {
        HEAP_ASSERTMESSAGE("Heap is not valid");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if (offset + size > heap->GetSize())
    {
        HEAP_ASSERTMESSAGE("Data will not fit within the state heap");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    m_heap = heap;
    m_offset = offset;
    m_size = size;

    if (requestedState == State::free)
    {
        HEAP_CHK_STATUS(Free());
    }
    else if (requestedState == State::allocated)
    {
        HEAP_CHK_STATUS(Allocate(trackerId));
    }
    else
    {
        HEAP_ASSERTMESSAGE("Only free and allocated blocks can be created");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    m_prev = prev;
    m_next = prev->m_next;
    prev->m_next = this;
    auto next = GetNext();
    if (next)
    {
        next->m_prev = this;
    }

    // expected to be inserted into a sorted list by MemoryBlockManager
    m_statePrev = m_stateNext = nullptr;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MemoryBlockInternal::Combine(MemoryBlockInternal *block)
{
    HEAP_FUNCTION_ENTER_VERBOSE;

    HEAP_CHK_NULL(block);

    if (block->m_state != State::free || m_state != State::free)
    {
        HEAP_ASSERTMESSAGE("Only free blocks may be combined");
        return MOS_STATUS_INVALID_PARAMETER;
    }
    if (m_static)
    {
        HEAP_ASSERTMESSAGE("Static blocks may not be modified");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    auto currPrev = GetPrev();
    auto currNext = GetNext();
    if (currPrev == block)
    {
        m_offset = block->m_offset;
        m_prev = block->m_prev;
        currPrev = GetPrev();
        if (currPrev)
        {
            currPrev->m_next = this;
        }
    }
    else if (currNext == block)
    {
        m_next = block->m_next;
        currNext = GetNext();
        if (currNext)
        {
            currNext->m_prev = this;
        }
    }
    else
    {
        HEAP_ASSERTMESSAGE("Only adjacent blocks may be combined");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    m_size += block->m_size;
    block->Pool();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MemoryBlockInternal::Split(MemoryBlockInternal *block, uint32_t size)
{
    HEAP_FUNCTION_ENTER_VERBOSE;

    HEAP_CHK_NULL(block);

    if (size == 0 || m_size == size)
    {
        HEAP_ASSERTMESSAGE("It is not possible to split a block if the new block has no size");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if (m_state != State::free)
    {
        HEAP_ASSERTMESSAGE("Only free blocks may be resized");
        return MOS_STATUS_INVALID_PARAMETER;
    }
    if (m_static)
    {
        HEAP_ASSERTMESSAGE("Static blocks may not be modified");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    uint32_t newBlockSize = m_size - size;
    HEAP_CHK_STATUS(block->Create(
        m_heap,
        State::free,
        this,
        m_offset + size,
        newBlockSize,
        m_invalidTrackerId));

    m_size = size;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MemoryBlockInternal::Pool()
{
    HEAP_FUNCTION_ENTER_VERBOSE;

    if (m_state == State::allocated || m_state == State::submitted)
    {
        HEAP_ASSERTMESSAGE("Allocated and submitted blocks may not be added to the pool");
        return MOS_STATUS_INVALID_PARAMETER;
    }
    if (m_stateListType != State::stateCount)
    {
        HEAP_ASSERTMESSAGE("Blocks must be removed from sorted list before changing state");
        return MOS_STATUS_INVALID_PARAMETER;
    }
    if (m_static)
    {
        HEAP_ASSERTMESSAGE("Static blocks may not be added to the pool");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    m_state = State::pool;
    m_heap = nullptr;
    m_offset = 0;
    m_size = 0;
    m_static = false;
    m_trackerId = m_invalidTrackerId;
    m_trackerToken.Clear();
    m_prev = m_next = nullptr;
    m_statePrev = m_stateNext = nullptr;
    m_stateListType = State::stateCount;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MemoryBlockInternal::Free()
{
    HEAP_FUNCTION_ENTER_VERBOSE;

    if (m_state == State::deleted)
    {
        HEAP_ASSERTMESSAGE("Deleted blocks may not be moved to any state but pool");
        return MOS_STATUS_INVALID_PARAMETER;
    }
    if (m_stateListType != State::stateCount)
    {
        HEAP_ASSERTMESSAGE("Blocks must be removed from sorted list before changing state");
        return MOS_STATUS_INVALID_PARAMETER;
    }
    if (m_static)
    {
        HEAP_ASSERTMESSAGE("Static blocks may not be freed");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if (m_state != State::pool &&
        m_state != State::free)
    {
        HEAP_CHK_STATUS(m_heap->AdjustFreeSpace(m_size));
    }

    m_state = State::free;
    m_trackerId = m_invalidTrackerId;
    m_trackerToken.Clear();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MemoryBlockInternal::Allocate(uint32_t trackerId)
{
    HEAP_FUNCTION_ENTER_VERBOSE;

    if (m_state != State::free)
    {
        HEAP_ASSERTMESSAGE("Only free blocks may be moved to allocated");
        return MOS_STATUS_INVALID_PARAMETER;
    }
    if (m_stateListType != State::stateCount)
    {
        HEAP_ASSERTMESSAGE("Blocks must be removed from sorted list before changing state");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if (trackerId == m_invalidTrackerId && !m_static)
    {
        HEAP_ASSERTMESSAGE("Only static blocks may have invalid tracker IDs");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    // Reflect the change in state from free space to used state.
    HEAP_CHK_STATUS(m_heap->AdjustUsedSpace(m_size));

    m_state = State::allocated;
    m_trackerId = trackerId;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MemoryBlockInternal::Allocate(uint32_t index, uint32_t trackerId, FrameTrackerProducer *producer)
{
    HEAP_FUNCTION_ENTER_VERBOSE;

    if (m_state != State::free)
    {
        HEAP_ASSERTMESSAGE("Only free blocks may be moved to allocated");
        return MOS_STATUS_INVALID_PARAMETER;
    }
    if (m_stateListType != State::stateCount)
    {
        HEAP_ASSERTMESSAGE("Blocks must be removed from sorted list before changing state");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if (trackerId == m_invalidTrackerId && !m_static)
    {
        HEAP_ASSERTMESSAGE("Only static blocks may have invalid tracker IDs");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    // Reflect the change in state from free space to used state.
    HEAP_CHK_STATUS(m_heap->AdjustUsedSpace(m_size));

    m_state = State::allocated;
    if (producer)
    {
        m_trackerToken.SetProducer(producer);
    }
    m_trackerToken.Merge(index, trackerId);

    return MOS_STATUS_SUCCESS;
}


MOS_STATUS MemoryBlockInternal::Submit()
{
    HEAP_FUNCTION_ENTER_VERBOSE;

    if (m_state != State::allocated)
    {
        HEAP_ASSERTMESSAGE("Only allocated blocks may be submitted");
        return MOS_STATUS_INVALID_PARAMETER;
    }
    if (m_static)
    {
        HEAP_ASSERTMESSAGE("Static blocks may not enter submitted state");
        return MOS_STATUS_INVALID_PARAMETER;
    }
    if (m_stateListType != State::stateCount)
    {
        HEAP_ASSERTMESSAGE("Blocks must be removed from sorted list before changing state");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    m_state = State::submitted;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MemoryBlockInternal::Delete()
{
    HEAP_FUNCTION_ENTER_VERBOSE;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    if (m_state == State::pool)
    {
        HEAP_ASSERTMESSAGE("Pool blocks should not be deleted");
        return MOS_STATUS_INVALID_PARAMETER;
    }
    if (m_stateListType != State::stateCount)
    {
        HEAP_ASSERTMESSAGE("Blocks must be removed from sorted list before changing state");
        return MOS_STATUS_INVALID_PARAMETER;
    }
    if (m_static)
    {
        HEAP_ASSERTMESSAGE("Static blocks may not be deleted");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if (!m_heap->IsFreeInProgress())
    {
        HEAP_ASSERTMESSAGE("Blocks should not be deleted unless the heap is being freed");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if (m_state != State::free && m_state != State::deleted)
    {
        HEAP_CHK_STATUS(m_heap->AdjustFreeSpace(m_size));
    }

    m_state = State::deleted;
    m_trackerId = m_invalidTrackerId;
    m_trackerToken.Clear();

    return MOS_STATUS_SUCCESS;
}
