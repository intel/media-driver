/*
* Copyright (c) 2015-2017, Intel Corporation
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
//! \file      mhw_memory_pool.cpp
//! \brief         This modules implements simple memory pool infrastructure for MHW 
//!
#include "mhw_memory_pool.h"
#include "mhw_state_heap.h"

typedef struct _MHW_MEMORY_POOL_ENTRY
{
    PMHW_MEMORY_POOL_ENTRY  pNext;              //!< Next pool entry (doubled linked list)
    PMHW_MEMORY_POOL_ENTRY  pPrev;              //!< Prev pool entry (doubled linked list)
    PMHW_MEMORY_POOL        pPool;              //!< Pool object
    void                    *pAllocation;        //!< Memory allocation from malloc (to be freed)
    uint32_t                dwSize;             //!< Memory size allocated from malloc
    void                    *pObjects;           //!< Pointer to first object in this pool entry
    uint32_t                dwCount;            //!< Number of objects in this pool allocation
} MHW_MEMORY_POOL_ENTRY, *PMHW_MEMORY_POOL_ENTRY;

MHW_MEMORY_POOL::MHW_MEMORY_POOL(uint32_t dwObjSize, uint32_t dwObjAlignment):
    m_pFirst(nullptr),
    m_pLast (nullptr),
    m_dwCount(0),
    m_dwSize(0),
    m_dwObjSize(dwObjSize),
    m_dwObjAlignment(dwObjAlignment),
    m_dwObjCount(0)
{

}

MHW_MEMORY_POOL::~MHW_MEMORY_POOL()
{
    PMHW_MEMORY_POOL_ENTRY pEntry;
    PMHW_MEMORY_POOL_ENTRY pNext;

    pEntry = m_pFirst;
    while (pEntry)
    {
        pNext = pEntry->pNext;
        if (pEntry->pAllocation)
        {
            MOS_FreeMemory(pEntry->pAllocation);
        }
        pEntry = pNext;
    }
}

// Allocate an array of objects to be added to the pool
void  *MHW_MEMORY_POOL::Allocate(uint32_t dwObjCount)
{
    uint8_t                *pObjects  = nullptr;
    uint32_t               dwSize;
    PMHW_MEMORY_POOL_ENTRY pEntry = nullptr;

    if (dwObjCount == 0)
    {
        return nullptr;
    }

    // Calculate total size - pool entry + objects + alignments
    dwSize = sizeof(MHW_MEMORY_POOL_ENTRY);
    dwSize += m_dwObjSize * dwObjCount + m_dwObjAlignment;

    pEntry = (PMHW_MEMORY_POOL_ENTRY) MOS_AllocMemory(dwSize);
    if (!pEntry)
    {
        return nullptr;
    }
    MOS_ZeroMemory(pEntry, dwSize);

    pObjects  = ((uint8_t*) pEntry) + sizeof(MHW_MEMORY_POOL_ENTRY);

    // adjust alignment if necessary
    if ((uintptr_t(pObjects) % m_dwObjAlignment))
    {
        // clear out lower bits
        pObjects = (uint8_t*)((uintptr_t)pObjects & (~(uintptr_t)(m_dwObjAlignment - 1)));
        // add alignment
        pObjects += m_dwObjAlignment;
    }

    // Insert pool entry into linked list
    pEntry->pNext = nullptr;
    pEntry->pPrev = m_pLast;
    m_pLast = pEntry;

    if (pEntry->pPrev) pEntry->pPrev->pNext = pEntry;

    if (!m_pFirst) m_pFirst = pEntry;

    // Setup pool entry
    pEntry->pPool       = this;
    pEntry->pAllocation = (void *)pEntry;
    pEntry->dwSize      = dwSize;
    pEntry->pObjects    = pObjects;
    pEntry->dwCount     = dwObjCount;

    // Update pool object
    m_dwCount++;
    m_dwSize     += dwSize;
    m_dwObjCount += dwObjCount;

    return pObjects;
}
