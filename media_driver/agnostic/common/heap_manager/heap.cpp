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
//! \file     heap.cpp
//! \brief    Implements functionalities pertaining to heaps.
//!

#include "heap.h"

Heap::Heap()
{
    HEAP_FUNCTION_ENTER;
    m_id = m_invalidId;
}

Heap::Heap(uint32_t id)
{
    HEAP_FUNCTION_ENTER;
    m_id = id;
}

Heap::~Heap()
{
    HEAP_FUNCTION_ENTER;
    if (m_osInterface != nullptr)
    {
        if (m_lockedHeap != nullptr)
        {
            m_osInterface->pfnUnlockResource(m_osInterface, m_resource);
        }
        if (!Mos_ResourceIsNull(m_resource))
        {
            m_osInterface->pfnFreeResource(m_osInterface, m_resource);
            MOS_FreeMemory(m_resource);
        }
    }
}

MOS_STATUS Heap::RegisterOsInterface(PMOS_INTERFACE osInterface)
{
    HEAP_FUNCTION_ENTER;
    HEAP_CHK_NULL(osInterface);
    m_osInterface = osInterface;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Heap::Allocate(uint32_t heapSize, bool keepLocked)
{
    HEAP_FUNCTION_ENTER;

    if (heapSize == 0)
    {
        HEAP_ASSERTMESSAGE("No size requested for heap allocation!");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if (m_resource != nullptr)
    {
        HEAP_ASSERTMESSAGE("A heap has already been allocated!");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    m_resource = (PMOS_RESOURCE)MOS_AllocAndZeroMemory(sizeof(MOS_RESOURCE));
    HEAP_CHK_NULL(m_resource);
    HEAP_CHK_NULL(m_osInterface);

    MOS_ALLOC_GFXRES_PARAMS allocParams;
    memset(&allocParams, 0, sizeof(allocParams));
    allocParams.Type = MOS_GFXRES_BUFFER;
    allocParams.TileType = MOS_TILE_LINEAR;
    allocParams.Format = Format_Buffer;
    allocParams.dwBytes = heapSize;
    allocParams.pBufName = "Heap";
    HEAP_CHK_STATUS(m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParams,
        m_resource));

    // explicit set to skip resource sync, heap resource should be used block by block
    // driver should ensure non concurrent access the same block
    HEAP_CHK_STATUS(m_osInterface->pfnSkipResourceSync(m_resource));

    if (keepLocked)
    {
        m_lockedHeap = Lock();
        HEAP_CHK_NULL(m_lockedHeap);
        m_keepLocked = keepLocked;
    }

    m_size = heapSize;
    m_freeSpace = m_size;

    return MOS_STATUS_SUCCESS;
}

uint8_t* Heap::Lock()
{
    HEAP_FUNCTION_ENTER_VERBOSE;
    if (m_keepLocked)
    {
        return m_lockedHeap;
    }

    if (m_osInterface == nullptr)
    {
        HEAP_ASSERTMESSAGE("Invalid m_osInterface(nullptr)");
        return nullptr;
    }

    MOS_LOCK_PARAMS lockParams;
    memset(&lockParams, 0, sizeof(lockParams));
    lockParams.WriteOnly = 1;
    lockParams.NoOverWrite = 1;
    lockParams.Uncached = 1;
    uint8_t* pLockedResource =
        (uint8_t*)m_osInterface->pfnLockResource(m_osInterface, m_resource, &lockParams);
    return pLockedResource;
}

MOS_STATUS Heap::Dump()
{
    return MOS_STATUS_UNIMPLEMENTED;
}

MOS_STATUS Heap::AdjustFreeSpace(uint32_t addedSpace)
{
    HEAP_FUNCTION_ENTER_VERBOSE;

    if (addedSpace + m_freeSpace > m_size ||
        m_usedSpace < addedSpace)
    {
        HEAP_ASSERTMESSAGE("Provided space will not fit in the heap");
        return MOS_STATUS_INVALID_PARAMETER;
    }
    m_freeSpace += addedSpace;
    m_usedSpace -= addedSpace;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Heap::AdjustUsedSpace(uint32_t addedSpace)
{
    HEAP_FUNCTION_ENTER_VERBOSE;

    if (addedSpace + m_usedSpace > m_size ||
        m_freeSpace < addedSpace)
    {
        HEAP_ASSERTMESSAGE("Provided space will not fit in the heap");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if (m_freeInProgress)
    {
        HEAP_ASSERTMESSAGE("Heap is in the process of being freed, cannot add space as used");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    m_freeSpace -= addedSpace;
    m_usedSpace += addedSpace;

    return MOS_STATUS_SUCCESS;
}
