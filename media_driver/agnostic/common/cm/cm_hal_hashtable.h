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
//! \file      cm_hal_hashtable.h  
//! \brief         This modules defines hash table implementation used for kernel search in CMHal.  
//!
#ifndef __CM_HAL_HASHTABLE_H__
#define __CM_HAL_HASHTABLE_H__

#include "mos_os.h"
#include "stdint.h"

#define CM_HAL_HASHTABLE_INITIAL   128
#define CM_HAL_HASHTABLE_INCREMENT 64
#define CM_HAL_HASHTABLE_MAX       2048

typedef struct _CM_HAL_HASH_TABLE_ENTRY
{
    int32_t UniqID;
    int32_t CacheID;
    uint16_t wNext;
    void    *pData;
} CM_HAL_HASH_TABLE_ENTRY, *PCM_HAL_HASH_TABLE_ENTRY;

typedef struct _CM_HAL_COALESCED_HASH_TABLE
{
    uint16_t                    wHead[256];         // Head of bucket list, 0 if empty
    uint16_t                    wFree;              // Head of the free hash table list, 0 if not present
    uint16_t                    wSize;              // Size of the hash table currently allocated
    CM_HAL_HASH_TABLE_ENTRY *pHashEntries;       // Dynamically expanding coalescing hash table
} CM_HAL_COALESCED_HASH_TABLE, *PCM_HAL_COALESCED_HASH_TABLE;

typedef struct _CM_HAL_COALESCED_HASH_TABLE *PCM_HAL_COALESCED_HASH_TABLE;

class CmHashTable
{
public:
    MOS_STATUS Init();
    void Free();
    MOS_STATUS Register(int32_t UniqID, int32_t CacheID, void  *pData);
    void*      Search(int32_t UniqID, int32_t CacheID, uint16_t &wSearchIndex);
    void*      Unregister(int32_t UniqID, int32_t CacheID);

private:
    uint16_t   SimpleHash(int32_t value);
    MOS_STATUS Extend();
    CM_HAL_COALESCED_HASH_TABLE m_hashTable;
};

#endif // __CM_HAL_HASHTABLE_H__
