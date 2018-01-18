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
//! \file      renderhal_hashtable.cpp 
//! \brief         This modules implements a simple coalesced hash table used for kernel search in     dynamic state heap based RenderHal. It exposes hash table initialization, destruction,     registration, unregistration and search functions used to speed up kernel search.     The hash table size is current limited to 65535 (MAX uint16_t-1), and 2 keys may be used     iKUID (Kernel Unique Identifier - int32_t) and CacheID (Arbitrary Kernel Cache ID - int32_t).     Given the dynamic nature of the ISH and kernel allocation, the hash table is allowed     to grow dynamically as needed. 
//!
#include "renderhal.h"
#include "renderhal_hashtable.h"

#define RENDERHAL_SIMPLE_HASH(wHash, iUID)  \
    wHash = ((iUID >> 16) ^ iUID) & 0xFFFF; \
    wHash = ((wHash >> 8) ^ wHash) & 0xFF;

MOS_STATUS RenderHal_HashTable_Init(PRENDERHAL_COALESCED_HASH_TABLE pHashTable)
{
    PRENDERHAL_HASH_TABLE_ENTRY pHashEntry = nullptr;
    MOS_STATUS eStatus = MOS_STATUS_UNKNOWN;

    if (!pHashTable) goto finish;

    MOS_ZeroMemory(pHashTable, sizeof(RENDERHAL_COALESCED_HASH_TABLE));
    pHashEntry = (PRENDERHAL_HASH_TABLE_ENTRY) MOS_AllocMemory(RENDERHAL_HASHTABLE_INITIAL * sizeof(RENDERHAL_HASH_TABLE_ENTRY));
    if (!pHashEntry)
    {
        eStatus = MOS_STATUS_NO_SPACE;
        goto finish;
    }

    pHashTable->pHashEntries = pHashEntry;
    pHashTable->wSize        = RENDERHAL_HASHTABLE_INITIAL;
    pHashTable->wFree        = 1; // First free element = 1 (0 is reserved for nullptr; 0xffff could be used instead, but 0 makes it cleaner/easier to read/understand)
    for (int i = 0; i < RENDERHAL_HASHTABLE_INITIAL - 1; i++, pHashEntry++)
    {
        pHashEntry->UniqID  = -1;
        pHashEntry->CacheID = -1;
        pHashEntry->wNext   = i + 1;
        pHashEntry->pData   = nullptr;
    }
    pHashEntry--;
    pHashEntry->wNext = 0;

    eStatus = MOS_STATUS_SUCCESS;
finish:
    return eStatus;
}

void RenderHal_HashTable_Free(PRENDERHAL_COALESCED_HASH_TABLE pHashTable)
{
    if (pHashTable)
    {
        if (pHashTable->pHashEntries) MOS_FreeMemory(pHashTable->pHashEntries);
        MOS_ZeroMemory(pHashTable, sizeof(RENDERHAL_COALESCED_HASH_TABLE));
    }
}

MOS_STATUS RenderHal_HashTable_Extend(PRENDERHAL_COALESCED_HASH_TABLE pHashTable)
{
    uint16_t                    wEntry;
    PRENDERHAL_HASH_TABLE_ENTRY pEntry;
    int32_t                     iPrevSize, iNewSize;
    MOS_STATUS                  eStatus = MOS_STATUS_UNKNOWN;

    if (!pHashTable || pHashTable->wSize >= RENDERHAL_HASHTABLE_MAX)
    {
        goto finish;
    }

    iPrevSize = pHashTable->wSize * sizeof(RENDERHAL_HASH_TABLE_ENTRY);
    iNewSize  = iPrevSize + RENDERHAL_HASHTABLE_INCREMENT * sizeof(RENDERHAL_HASH_TABLE_ENTRY);
    pEntry = (PRENDERHAL_HASH_TABLE_ENTRY) MOS_AllocMemory(iNewSize);
    if (!pEntry)
    {
        eStatus = MOS_STATUS_NO_SPACE;
        goto finish;
    }

    // Transfer the hash entries to larger table, free old (smaller) table
    MOS_SecureMemcpy(pEntry, iPrevSize, pHashTable->pHashEntries, iPrevSize);
    MOS_FreeMemory(pHashTable->pHashEntries);
    pHashTable->pHashEntries = pEntry;

    // Initialize entries
    wEntry  = pHashTable->wSize + 1;
    pEntry += pHashTable->wSize;
    for (int i = RENDERHAL_HASHTABLE_INCREMENT; i > 0; i--, wEntry++, pEntry++)
    {
        pEntry->UniqID  = -1;
        pEntry->CacheID = -1;
        pEntry->wNext   = wEntry;
        pEntry->pData   = nullptr;
    }
    pEntry--;

    // Update free list - new array is appended at the beginning of the free list, avoiding the need to traverse it.
    pEntry->wNext     = pHashTable->wFree;               // Last entry of newly created entries points to first pre-existing free entry
    pHashTable->wFree = pHashTable->wSize;               // Free list points to newly created array, which points to pre-existing array
    pHashTable->wSize  += RENDERHAL_HASHTABLE_INCREMENT; // Update size of the hash table

    eStatus = MOS_STATUS_SUCCESS;

finish:
    return eStatus;
}

MOS_STATUS RenderHal_HashTable_Register(PRENDERHAL_COALESCED_HASH_TABLE pHashTable, int32_t UniqID, int32_t CacheID, void  *pData)
{
    uint16_t                    wHash;
    uint16_t                    wEntry;
    PRENDERHAL_HASH_TABLE_ENTRY pEntry;
    MOS_STATUS                  eStatus = MOS_STATUS_UNKNOWN;

    RENDERHAL_SIMPLE_HASH(wHash, UniqID);

    // Get new entry
    wEntry = pHashTable->wFree;

    // Extend hash table, get new free entry
    if (wEntry == 0)
    {
        MHW_RENDERHAL_CHK_STATUS(RenderHal_HashTable_Extend(pHashTable));
        wEntry = pHashTable->wFree;
    }

    // Remove entry from free list
    pEntry = pHashTable->pHashEntries + wEntry;
    pHashTable->wFree = pEntry->wNext;

    // Link hash entry
    pEntry->UniqID  = UniqID;                   // save unique id
    pEntry->CacheID = CacheID;                  // save unique id
    pEntry->pData   = pData;                    // save pointer to data
    pEntry->wNext   = pHashTable->wHead[wHash]; // points to next entry in same bucket
    pHashTable->wHead[wHash] = wEntry;          // move entry to head of the bucket

    eStatus = MOS_STATUS_SUCCESS;

finish:
    return eStatus;
}

void  *RenderHal_HashTable_Unregister(PRENDERHAL_COALESCED_HASH_TABLE pHashTable, int32_t UniqID, int32_t CacheID)
{
    uint16_t                    wHash;
    uint16_t                    wEntry = 0;
    uint16_t                    wSearchIndex;
    PRENDERHAL_HASH_TABLE_ENTRY pEntry, pPrevEntry;
    void                        *pData = nullptr;

    RENDERHAL_SIMPLE_HASH(wHash, UniqID);

    // Search for UniqID/CacheID (hashing should significantly speedup this search)
    pEntry = pPrevEntry = nullptr;
    wSearchIndex = pHashTable->wHead[wHash];
    bool bFound;

    if (CacheID >= 0)
    {
        // Search for UniqID/CacheID
        for (bFound = false; (wSearchIndex > 0) && (!bFound); wSearchIndex = pEntry->wNext)
        {
            wEntry = wSearchIndex;
            pEntry = pHashTable->pHashEntries + wSearchIndex;
            bFound = (pEntry->UniqID == UniqID) && (pEntry->CacheID == CacheID);
        }
    }
    else
    {
        // Search for UniqID (don't care about CacheID)
        for (bFound = false; (wSearchIndex > 0) && (!bFound); wSearchIndex = pEntry->wNext)
        {
            wEntry = wSearchIndex;
            pEntry = pHashTable->pHashEntries + wSearchIndex;
            bFound = (pEntry->UniqID == UniqID);
        }
    }

    // Entry found
    if (wEntry > 0)
    {
        // Detach from hash list
        if (pPrevEntry)
            pPrevEntry->wNext = pEntry->wNext;
        else
            pHashTable->wHead[wHash] = pEntry->wNext;

        // Move hash entry to free list
        pEntry->wNext = pHashTable->wFree;
        pHashTable->wFree = wEntry;
        pData = pEntry->pData;
    }

    return pData;
}

void  *RenderHal_HashTable_Search(PRENDERHAL_COALESCED_HASH_TABLE pHashTable, int32_t UniqID, int32_t CacheID, uint16_t &wSearchIndex)
{
    uint16_t                    wHash;
    uint16_t                    wEntry = 0;
    PRENDERHAL_HASH_TABLE_ENTRY pEntry = nullptr;
    void                        *pData  = nullptr;
    bool                        bFound;

    // Get first entry, or continue previous search
    if (wSearchIndex == 0 ||
        wSearchIndex >= pHashTable->wSize)
    {
        RENDERHAL_SIMPLE_HASH(wHash, UniqID);
        wSearchIndex = pHashTable->wHead[wHash];
    }

    if (CacheID >= 0)
    {
        // Search for UniqID/CacheID
        for (bFound = false; (wSearchIndex > 0) && (!bFound); wSearchIndex = pEntry->wNext)
        {
            wEntry = wSearchIndex;
            pEntry = pHashTable->pHashEntries + wSearchIndex;
            bFound = (pEntry->UniqID == UniqID) && (pEntry->CacheID == CacheID);
        }
    }
    else
    {
        // Search for UniqID (don't care about CacheID)
        for (bFound = false; (wSearchIndex > 0) && (!bFound); wSearchIndex = pEntry->wNext)
        {
            wEntry = wSearchIndex;
            pEntry = pHashTable->pHashEntries + wSearchIndex;
            bFound = (pEntry->UniqID == UniqID);
        }
    }

    // Retrieve user data
    if (bFound)
    {
        pData  = pEntry->pData;
        wEntry = pEntry->wNext;
    }

    return pData;
}
