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
//! \file      cm_hal_hashtable.cpp  
//! \brief         This modules implements a simple coalesced hash table used 
//!                for kernel search in dynamic state heap based CmHal. 
//!                It exposes hash table initialization, destruction,     
//!                registration, unregistration and search functions used to 
//!                speed up kernel search. The hash table size is currently 
//!                limited to 65535 (MAX uint16_t-1), and 2 keys may be used
//!                iKUID (Kernel Unique Identifier - int32_t) and 
//!                CacheID (Arbitrary Kernel Cache ID - int32_t).
//!                Given the dynamic nature of the ISH and kernel allocation,
//!                the hash table is allowed to grow dynamically as needed.  
//!

#include "cm_hal_hashtable.h"

MOS_STATUS CmHashTable::Init()
{
    MOS_STATUS               eStatus = MOS_STATUS_SUCCESS;
    PCM_HAL_HASH_TABLE_ENTRY pHashEntry = nullptr;

    pHashEntry = (PCM_HAL_HASH_TABLE_ENTRY)MOS_AllocMemory(CM_HAL_HASHTABLE_INITIAL * sizeof(CM_HAL_HASH_TABLE_ENTRY));
    if (!pHashEntry)
    {
        eStatus = MOS_STATUS_NO_SPACE;
        return eStatus;
    }


    m_hashTable.pHashEntries = pHashEntry;
    m_hashTable.wSize = CM_HAL_HASHTABLE_INITIAL;
    m_hashTable.wFree = 1; // First free element = 1 (0 is reserved for nullptr; 0xffff could be used instead, but 0 makes it cleaner/easier to read/understand)
    for (int i = 0; i < CM_HAL_HASHTABLE_INITIAL - 1; i++, pHashEntry++)
    {
        pHashEntry->UniqID = -1;
        pHashEntry->CacheID = -1;
        pHashEntry->wNext = i + 1;
        pHashEntry->pData = nullptr;
    }
    pHashEntry--;
    pHashEntry->wNext = 0;

    return eStatus;
}

void CmHashTable::Free()
{
    if (m_hashTable.pHashEntries) MOS_FreeMemory(m_hashTable.pHashEntries);
}

uint16_t CmHashTable::SimpleHash(int32_t value)
{
    uint16_t wHash = ((value >> 16) ^ value) & 0xFFFF;
    wHash = ((wHash >> 8) ^ wHash) & 0xFF;
    return wHash;
}

MOS_STATUS CmHashTable::Extend()
{
    uint16_t                    wEntry;
    PCM_HAL_HASH_TABLE_ENTRY    pEntry;
    int32_t                     iPrevSize, iNewSize;
    MOS_STATUS                  hr = MOS_STATUS_UNKNOWN;

    if (m_hashTable.wSize >= CM_HAL_HASHTABLE_MAX)
    {
        goto finish;
    }

    iPrevSize = m_hashTable.wSize * sizeof(CM_HAL_HASH_TABLE_ENTRY);
    iNewSize = iPrevSize + CM_HAL_HASHTABLE_INCREMENT * sizeof(CM_HAL_HASH_TABLE_ENTRY);
    pEntry = (PCM_HAL_HASH_TABLE_ENTRY)MOS_AllocMemory(iNewSize);
    if (!pEntry)
    {
        hr = MOS_STATUS_NO_SPACE;
        goto finish;
    }

    // Transfer the hash entries to larger table, free old (smaller) table
    MOS_SecureMemcpy(pEntry, iPrevSize, m_hashTable.pHashEntries, iPrevSize);
    MOS_FreeMemory(m_hashTable.pHashEntries);
    m_hashTable.pHashEntries = pEntry;

    // Initialize entries
    wEntry = m_hashTable.wSize + 1;
    pEntry += m_hashTable.wSize;
    for (int i = CM_HAL_HASHTABLE_INCREMENT; i > 0; i--, wEntry++, pEntry++)
    {
        pEntry->UniqID = -1;
        pEntry->CacheID = -1;
        pEntry->wNext = wEntry;
        pEntry->pData = nullptr;
    }
    pEntry--;

    // Update free list - new array is appended at the beginning of the free list, avoiding the need to traverse it.
    pEntry->wNext = m_hashTable.wFree;               // Last entry of newly created entries points to first pre-existing free entry
    m_hashTable.wFree = m_hashTable.wSize;               // Free list points to newly created array, which points to pre-existing array
    m_hashTable.wSize += CM_HAL_HASHTABLE_INCREMENT; // Update size of the hash table

    hr = MOS_STATUS_SUCCESS;

finish:
    return hr;
}
MOS_STATUS CmHashTable::Register(int32_t UniqID, int32_t CacheID, void  *pData)
{
    uint16_t                    wHash;
    uint16_t                    wEntry;
    PCM_HAL_HASH_TABLE_ENTRY    pEntry;
    MOS_STATUS                  hr = MOS_STATUS_UNKNOWN;

    wHash = SimpleHash(UniqID);

    // Get new entry
    wEntry = m_hashTable.wFree;

    // Extend hash table, get new free entry
    if (wEntry == 0)
    {
        hr = Extend();
        if (hr != MOS_STATUS_SUCCESS)
            goto finish;
        wEntry = m_hashTable.wFree;
    }

    // Remove entry from free list
    pEntry = m_hashTable.pHashEntries + wEntry;
    m_hashTable.wFree = pEntry->wNext;

    // Link hash entry
    pEntry->UniqID = UniqID;                   // save unique id
    pEntry->CacheID = CacheID;                  // save unique id
    pEntry->pData = pData;                    // save pointer to data
    pEntry->wNext = m_hashTable.wHead[wHash]; // points to next entry in same bucket
    m_hashTable.wHead[wHash] = wEntry;          // move entry to head of the bucket

    hr = MOS_STATUS_SUCCESS;

finish:
    return hr;
}

void* CmHashTable::Search(int32_t UniqID, int32_t CacheID, uint16_t &wSearchIndex)
{
    PCM_HAL_HASH_TABLE_ENTRY    pEntry = nullptr;
    void                        *pData = nullptr;
    bool                        bFound;

    // Get first entry, or continue previous search
    if (wSearchIndex == 0 ||
        wSearchIndex >= m_hashTable.wSize)
    {
        uint16_t wHash = SimpleHash(UniqID);
        wSearchIndex = m_hashTable.wHead[wHash];
    }

    if (CacheID >= 0)
    {
        // Search for UniqID/CacheID
        for (bFound = false; (wSearchIndex > 0) && (!bFound); wSearchIndex = pEntry->wNext)
        {
            pEntry = m_hashTable.pHashEntries + wSearchIndex;
            bFound = (pEntry->UniqID == UniqID) && (pEntry->CacheID == CacheID);
        }
    }
    else
    {
        // Search for UniqID (don't care about CacheID)
        for (bFound = false; (wSearchIndex > 0) && (!bFound); wSearchIndex = pEntry->wNext)
        {
            pEntry = m_hashTable.pHashEntries + wSearchIndex;
            bFound = (pEntry->UniqID == UniqID);
        }
    }

    // Retrieve user data
    if (bFound)
    {
        pData = pEntry->pData;
    }

    return pData;
}

void* CmHashTable::Unregister(int32_t UniqID, int32_t CacheID)
{
    uint16_t                    wHash;
    uint16_t                    wEntry = 0;
    uint16_t                    wSearchIndex;
    PCM_HAL_HASH_TABLE_ENTRY    pEntry, pPrevEntry;
    void                        *pData = nullptr;


    wHash = SimpleHash(UniqID);

    // Search for UniqID/CacheID (hashing should significantly speedup this search)
    pEntry = pPrevEntry = nullptr;
    wSearchIndex = m_hashTable.wHead[wHash];
    bool bFound;

    if (CacheID >= 0)
    {
        // Search for UniqID/CacheID
        for (bFound = false; (wSearchIndex > 0) && (!bFound); wSearchIndex = pEntry->wNext)
        {
            wEntry = wSearchIndex;
            pEntry = m_hashTable.pHashEntries + wSearchIndex;
            bFound = (pEntry->UniqID == UniqID) && (pEntry->CacheID == CacheID);
        }
    }
    else
    {
        // Search for UniqID (don't care about CacheID)
        for (bFound = false; (wSearchIndex > 0) && (!bFound); wSearchIndex = pEntry->wNext)
        {
            wEntry = wSearchIndex;
            pEntry = m_hashTable.pHashEntries + wSearchIndex;
            bFound = (pEntry->UniqID == UniqID);
        }
    }

    // Entry found
    if (wEntry > 0)
    {
        // Detach from hash list
        m_hashTable.wHead[wHash] = pEntry->wNext;

        // Move hash entry to free list
        pEntry->wNext = m_hashTable.wFree;
        m_hashTable.wFree = wEntry;
        pData = pEntry->pData;
    }

    return pData;
}
