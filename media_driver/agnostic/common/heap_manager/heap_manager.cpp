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
//! \file     heap_manager.cpp
//! \brief    Implements functionalities pertaining to the heap manager.
//!

#include "heap_manager.h"

HeapManager::~HeapManager()
{
    HEAP_FUNCTION_ENTER;
    m_currHeapId = 0;
    m_currHeapSize = 0;
    m_extendHeapSize = 0;
    m_osInterface = nullptr;
}

MOS_STATUS HeapManager::AcquireSpace(
    MemoryBlockManager::AcquireParams &params,
    std::vector<MemoryBlock> &blocks,
    uint32_t &spaceNeeded)
{
    HEAP_FUNCTION_ENTER;

    // first time space is being acquired, allocate the first heap
    if (m_heapIds.empty())
    {
        HEAP_CHK_STATUS(AllocateHeap(m_currHeapSize));
    }

    if (m_behavior != Behavior::clientControlled)
    {
        if (params.m_staticBlock)
        {
            HEAP_ASSERTMESSAGE("Static blocks are only allowed when the client controls behavior");
            return MOS_STATUS_INVALID_PARAMETER;
        }
        if (!m_blockManager.IsTrackerDataValid())
        {
            HEAP_ASSERTMESSAGE("A tracker must be registered before acquiring space if client does not control behavior");
            return MOS_STATUS_INVALID_PARAMETER;
        }
    }

    spaceNeeded = 0;
    MOS_STATUS acquireSpaceResult = m_blockManager.AcquireSpace(params, blocks, spaceNeeded);
    if (acquireSpaceResult == MOS_STATUS_CLIENT_AR_NO_SPACE)
    {
        bool blocksUpdated = false;
        // no need to refresh the block states for every space acquisition
        // attempt for first AcquireSpace failure
        HEAP_CHK_STATUS(m_blockManager.RefreshBlockStates(blocksUpdated));
        if (blocksUpdated)
        {
            if ((m_blockManager.AcquireSpace(params, blocks, spaceNeeded)) ==
                MOS_STATUS_CLIENT_AR_NO_SPACE)
            {
                // if space may not be acquired after refreshing block states, execute behavior
                HEAP_CHK_STATUS(BehaveWhenNoSpace());
                HEAP_CHK_STATUS(m_blockManager.AcquireSpace(params, blocks, spaceNeeded));
            }
        }
        else
        {
            // if no blocks updated after refresh, execute behavior
            HEAP_CHK_STATUS(BehaveWhenNoSpace());
            HEAP_CHK_STATUS(m_blockManager.AcquireSpace(params, blocks, spaceNeeded));
        }
    }
    else
    {
        HEAP_CHK_STATUS(acquireSpaceResult);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HeapManager::ClearSpace(MemoryBlock &block)
{
    HEAP_FUNCTION_ENTER;

    if (m_behavior != clientControlled)
    {
        HEAP_ASSERTMESSAGE("Only client controlled behavior allows for clients to clear space");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    HEAP_CHK_STATUS(m_blockManager.ClearSpace(block))

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HeapManager::RegisterTrackerResource(uint32_t *trackerData)
{
    HEAP_FUNCTION_ENTER;
    return m_blockManager.RegisterTrackerResource(trackerData);
}

MOS_STATUS HeapManager::RegisterTrackerProducer(FrameTrackerProducer *trackerProducer)
{
    HEAP_FUNCTION_ENTER;
    return m_blockManager.RegisterTrackerProducer(trackerProducer);
}

MOS_STATUS HeapManager::SetInitialHeapSize(uint32_t size)
{
    HEAP_FUNCTION_ENTER;

    if (size == 0)
    {
        HEAP_ASSERTMESSAGE("0 is an invalid size for heap extension");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    m_currHeapSize = MOS_ALIGN_CEIL(size, m_heapAlignment);

    return MOS_STATUS_SUCCESS;
}


MOS_STATUS HeapManager::SetExtendHeapSize(uint32_t size)
{
    HEAP_FUNCTION_ENTER;

    if (size == 0)
    {
        HEAP_ASSERTMESSAGE("0 is an invalid size for heap extension");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if (m_behavior == Behavior::wait)
    {
        HEAP_ASSERTMESSAGE("The heap may not be extended in the case of wait behavior");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    m_extendHeapSize = MOS_ALIGN_CEIL(size, m_heapAlignment);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HeapManager::RegisterOsInterface(PMOS_INTERFACE osInterface)
{
    HEAP_FUNCTION_ENTER;
    HEAP_CHK_NULL(osInterface);
    m_osInterface = osInterface;
    HEAP_CHK_STATUS(m_blockManager.RegisterOsInterface(m_osInterface));
    return MOS_STATUS_SUCCESS;
}

uint32_t HeapManager::GetTotalSize()
{
    HEAP_FUNCTION_ENTER;

    uint32_t totalSize = m_blockManager.GetSize();

    // Since allocation is delayed to when space is acquired, it is possible
    // for manager to be in a valid state if a size of 0 is returned. 
    if (totalSize == 0 && m_currHeapId == Heap::m_invalidId)
    {
        totalSize = m_currHeapSize;
    }

    return totalSize;
}

MOS_STATUS HeapManager::AllocateHeap(uint32_t size)
{
    HEAP_FUNCTION_ENTER;

    HEAP_CHK_NULL(m_osInterface);

    if (size == 0)
    {
        HEAP_ASSERTMESSAGE("0 is an invalid size for heap allocation");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    ++m_currHeapId;
    m_heapIds.push_back(m_currHeapId);

    HEAP_CHK_STATUS(m_blockManager.RegisterHeap(m_currHeapId, size, m_hwWriteOnlyHeap));

    return MOS_STATUS_SUCCESS;
}

void HeapManager::FreeHeap()
{
    // Free oldest heap
    auto heapId = m_heapIds.front();
    m_heapIds.pop_front();
    m_blockManager.UnregisterHeap(heapId);
}

MOS_STATUS HeapManager::Wait()
{
    HEAP_FUNCTION_ENTER_VERBOSE;

    bool blocksUpdated = false;

    for (auto waitMs = m_waitTimeout; waitMs > 0; waitMs -= m_waitIncrement)
    {
        MosUtilities::MosSleep(m_waitIncrement);
        HEAP_CHK_STATUS(m_blockManager.RefreshBlockStates(blocksUpdated));
        if (blocksUpdated)
        {
            break;
        }
    }

    return (blocksUpdated) ? MOS_STATUS_SUCCESS : MOS_STATUS_CLIENT_AR_NO_SPACE;
}

MOS_STATUS HeapManager::BehaveWhenNoSpace()
{
    HEAP_FUNCTION_ENTER_VERBOSE;

    switch (m_behavior)
    {
        case wait:
            HEAP_CHK_STATUS(Wait());
            break;
        case extend:
            m_currHeapSize += m_extendHeapSize;
            HEAP_CHK_STATUS(AllocateHeap(m_currHeapSize));
            break;
        case destructiveExtend:
            FreeHeap();
            m_currHeapSize += m_extendHeapSize;
            HEAP_CHK_STATUS(AllocateHeap(m_currHeapSize));
            break;
        case waitAndExtend:
            if (Wait() == MOS_STATUS_CLIENT_AR_NO_SPACE)
            {
                m_currHeapSize += m_extendHeapSize;
                HEAP_CHK_STATUS(AllocateHeap(m_currHeapSize));
            }
            break;
        case clientControlled:
            // heap manager gives control back to the client
            return MOS_STATUS_CLIENT_AR_NO_SPACE;
        default:
            HEAP_ASSERTMESSAGE("The requested behavior is not yet implemented");
            return MOS_STATUS_UNIMPLEMENTED;
    }

    return MOS_STATUS_SUCCESS;
}
