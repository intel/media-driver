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
//! \file      mhw_block_manager.cpp
//! \brief         This modules implements memory block management functions as part of MHW     dynamic state heap implementation 
//!
#include "mhw_block_manager.h"
#include "mhw_utilities.h"
#include "mos_os_specific.h"

MHW_BLOCK_MANAGER_PARAMS MhwBlockManagerParams_default =
{
    64,                                // Initial number of block objects in pool
    1024,                              // Maximum number of block objects in pool
    64,                                // Block pool increment size
    0x00080000,                        // Initial heap size         (512k)
    0x00080000,                        // Heap size increment       (512k)
    0x01000000,                        // Maximum overall heap size (16M)
    32,                                // Maximum number of heaps (32) (512k x 32 = 16M)
    0x0800,                            // Block granularity   = 2k (also represents min block alignment)
    0x0800                             // Min free block size = 2k (to reduce fragmentation)
};

const char *szListName[MHW_BLOCK_STATE_COUNT] = {
    "POOL",
    "FREE",
    "ALLOCATED",
    "SUBMITTED",
    "DELETED"
};

void Mhw_BlockManager_ReverseMergeSort_With_Index(const uint32_t *pdwSizes, int32_t iCount, uint8_t *pSortedIndex)
{
    uint8_t i, n;
    uint8_t *pSrc, *pDst;                               // Source and Destination groups (alternate)
    uint8_t Index1[MHW_BLOCK_MANAGER_MAX_BLOCK_ARRAY];  // Temporary sorted indexes 1
    uint8_t Index2[MHW_BLOCK_MANAGER_MAX_BLOCK_ARRAY];  // Temporary sorted indexes 2 (alternating with Index1)
    uint8_t *s1, *s2;                                   // Merge source groups 1 and 2
    uint8_t n1, n2;                                     // Merge sizes  groups 1 and 2
    uint8_t *d;                                         // Merge destination

    // Very simple cases
    if (iCount <= 1)
    {
        pSortedIndex[0] = 0;
        return;
    }
    else if (iCount == 2)
    {
        pSortedIndex[0] = (pdwSizes[0] < pdwSizes[1]) ? 1 : 0;
        pSortedIndex[1] = 1 - pSortedIndex[0];
        return;
    }

    // Initialize sorted index table
    for (i = 0; i < iCount; i++)
    {
        Index1[i] = i;
    }

    // Start alternating buffers (last will be the actual output)
    pSrc = Index1;
    pDst = Index2;

    // Merge sort algorithm:
    //      Sort will perform O(log n) passes; first pass sorts (iCount/2) groups of 2, then (iCount/4) groups of 4, and so on.
    //      Each pass requires traversal of the entire list - O(n)
    //      Algorithm is expected to be O(n * log n)
    for (n = 1; n < iCount; n *= 2)
    {
        // Setup sorted target output
        if (n*2 < iCount)
        {
            d = pDst;
        }
        else
        {   // Last pass, output goes to caller
            d = pSortedIndex;
        }

        // Group selection and merge - O(n)
        s1 = pSrc - n;    // First  group
        s2 = pSrc;        // Second group
        for (i = n; i < iCount; i += 2*n)   // i is the offset of the 2nd group
        {
            s1 += n;
            n1 = n;
            s2 += n;
            n2 = n;

            // Limit size of last group
            if (i + n > iCount)
            {
                n2 = iCount - i;
            }

            // Merge groups
            while (n1 > 0 && n2 > 0)
            {
                if (pdwSizes[*s1] >= pdwSizes[*s2])
                {
                    *(d++) = *(s1++);
                    n1--;
                }
                else
                {
                    *(d++) = *(s2++);
                    n2--;
                }
            }

            // Merge remaining items
            while (n1 > 0)
            {
                *(d++) = *s1++;
                n1--;
            }
            while (n2 > 0)
            {
                *(d++) = *s2++;
                n2--;
            }
        }

        // Copy the last group (not merge-sorted)
        for (i = i - n; i < iCount; i++)
        {
            *(d++) = *(s2++);
        }

        // New pass, switch Src/Dst sorted index buffers
        d    = pDst;
        pDst = pSrc;
        pSrc = d;
    }
}

void Mhw_BlockManager_ReverseMergeSort(uint32_t *pdwSizes, int32_t iCount)
{
    uint8_t i, n;
    uint32_t *pSrc, *pDst;                                     // Source and Destination groups (alternate)
    uint32_t Buffer1[2 * MHW_BLOCK_MANAGER_MAX_BLOCK_ARRAY];   // Temporary sorted buffer 1
    uint32_t Buffer2[2 * MHW_BLOCK_MANAGER_MAX_BLOCK_ARRAY];   // Temporary sorted buffer 1
    uint32_t *s1, *s2;                                         // Merge source groups 1 and 2
    uint8_t n1, n2;                                            // Merge sizes  groups 1 and 2
    uint32_t *d;                                               // Merge destination

    // Very simple cases
    if (iCount <= 1)
    {
        return;
    }
    else if (iCount == 2)
    {
        if (pdwSizes[0] < pdwSizes[1])
        {
            uint32_t tmp   = pdwSizes[1];
            pdwSizes[1] = pdwSizes[0];
            pdwSizes[0] = tmp;
        }
        return;
    }
    else if (iCount > 2 * MHW_BLOCK_MANAGER_MAX_BLOCK_ARRAY)
    {
        return;
    }

    // Merge sort algorithm:
    //      Sort will perform O(log n) passes; first pass sorts (iCount/2) groups of 2, then (iCount/4) groups of 4, and so on.
    //      Each pass requires traversal of the entire list - O(n)
    //      Algorithm is expected to be O(n * log n)
    pSrc = Buffer1;
    pDst = Buffer2;
    for (n = 1; n < iCount; n *= 2)
    {
        if (n == 1)
        {
            s1 = pdwSizes - 1;
            s2 = pdwSizes;
        }
        else
        {
            s1 = pSrc - n;
            s2 = pSrc;
        }

        // Setup sorted target output
        if (n*2 < iCount)
        {
            d = pDst;
        }
        else
        {   // Last pass, output goes to caller
            d = pdwSizes;
        }

        // Group selection and merge - O(n)
        for (i = n; i < iCount; i += 2*n)   // i is the offset of the 2nd group
        {
            s1 += n;
            n1 =  n;
            s2 += n;
            n2 =  n;

            // Limit size of last group
            if (i + n > iCount)
            {
                n2 = iCount - i;
            }

            // Merge groups
            while (n1 > 0 && n2 > 0)
            {
                if (*s1 >= *s2)
                {
                    *(d++) = *(s1++);
                    n1--;
                }
                else
                {
                    *(d++) = *(s2++);
                    n2--;
                }
            }

            // Merge remaining items
            while (n1 > 0)
            {
                *(d++) = *s1++;
                n1--;
            }
            while (n2 > 0)
            {
                *(d++) = *s2++;
                n2--;
            }
        }

        // Copy the last group (not merge-sorted)
        for (i = i - n; i < iCount; i++)
        {
            *(d++) = *(s2++);
        }

        // New pass, switch Src/Dst
        d    = pDst;
        pDst = pSrc;
        pSrc = d;
    }
}

MHW_BLOCK_MANAGER::MHW_BLOCK_MANAGER(PMHW_BLOCK_MANAGER_PARAMS pParams):
    m_MemoryPool(sizeof(MHW_STATE_HEAP_MEMORY_BLOCK), sizeof(void *)),
    m_pStateHeap(nullptr)
{
    //Init Parameters
    if(pParams != nullptr)
    {
        m_Params = *pParams;
    }
    else
    {
        m_Params = MhwBlockManagerParams_default;
    }

    //Init Memory block list
    for (int32_t i = (int32_t)MHW_BLOCK_STATE_POOL; i < MHW_BLOCK_STATE_COUNT; i++)
    {
        MOS_ZeroMemory(&m_BlockList[i], sizeof(MHW_BLOCK_LIST));
        m_BlockList[i].BlockState    = (MHW_BLOCK_STATE) i;
        m_BlockList[i].pBlockManager = this;
        MOS_SecureStrcpy(m_BlockList[i].szListName, 16, szListName[i]);
    }

    //Extend Pool
    ExtendPool(m_Params.dwPoolInitialCount);
}

MHW_BLOCK_MANAGER::~MHW_BLOCK_MANAGER()
{

}

void MHW_BLOCK_MANAGER::SetStateHeap(PMHW_STATE_HEAP pStateHeap)
{
    if (pStateHeap)
    {
        m_pStateHeap = pStateHeap;
    }
    return;
}

MOS_STATUS MHW_BLOCK_MANAGER::RegisterStateHeap(
    PMHW_STATE_HEAP    pStateHeap)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    // Setup state heap associated with the memory block manager
    if (!m_pStateHeap)
    {
        m_pStateHeap = pStateHeap;
    }

    // Indicates that state heap is now being managed by this block manager object
    pStateHeap->pBlockManager = this;

    // Get memory block object to represent the state heap memory (free)
    PMHW_STATE_HEAP_MEMORY_BLOCK pBlock = GetBlockFromPool();
    if (!pBlock)
    {
        return MOS_STATUS_NO_SPACE;
    }

    // Setup block parameters
    pBlock->pStateHeap          = pStateHeap;           // Points to state heap
    pBlock->pHeapPrev           = nullptr;                 // First Block in current heap
    pBlock->pHeapNext           = nullptr;                 // Last Block in current heap
    pBlock->dwOffsetInStateHeap = 0;                    // Offset is 0 = base of state heap
    pBlock->dwBlockSize         = pStateHeap->dwSize;   // Size of the entire state heap
    FrameTrackerTokenFlat_Validate(&pBlock->trackerToken);
    pBlock->bStatic             = 0;

    // Set first/last memory block in state heap
    pStateHeap->pMemoryHead     = pBlock;
    pStateHeap->pMemoryTail     = pBlock;

    // Reset special state heap controls
    pStateHeap->pDebugKernel    = nullptr;                 // Debug kernel loaded in this heap (if ISH)
    pStateHeap->pScratchSpace   = nullptr;                 // Active scratch space in this heap (if GSH)
    pStateHeap->dwScratchSpace  = 0;                    // Current scratch space size in this heap

    // Insert block at the end of the free memory pool
    AttachBlock(MHW_BLOCK_STATE_FREE, pBlock, MHW_BLOCK_POSITION_TAIL);

    return eStatus;
}

MOS_STATUS MHW_BLOCK_MANAGER::UnregisterStateHeap(
    PMHW_STATE_HEAP    pStateHeap)
{
    MHW_STATE_HEAP_MEMORY_BLOCK *pBlock;

    bool bReleaseHeap = true;

    // Verification loop - check if heap can be freed
    for (pBlock = pStateHeap->pMemoryHead; pBlock != nullptr; pBlock = pBlock->pHeapNext)
    {
        // Move blocks not in use to deleted queue - so they cannot be reused
        // NOTE: Blocks in SUBMITTED state need to wait for completion before releasing
        if (pBlock->BlockState == MHW_BLOCK_STATE_FREE ||
            pBlock->BlockState == MHW_BLOCK_STATE_ALLOCATED)
        {
            // Update heap usage
            if (pBlock->BlockState == MHW_BLOCK_STATE_FREE)
            {
                pStateHeap->dwFree -= pBlock->dwBlockSize;
            }
            else
            {
                pStateHeap->dwUsed -= pBlock->dwBlockSize;
            }

            DetachBlock(pBlock->BlockState, pBlock);
            AttachBlock(MHW_BLOCK_STATE_DELETED, pBlock, MHW_BLOCK_POSITION_TAIL);
        }
        // Block is allocated or submitted - mark block for deletion when released
        else if (pBlock->BlockState != MHW_BLOCK_STATE_DELETED)
        {
            pBlock->bStatic = false;    // Unlock block
            pBlock->bDelete = true;     // Mark block for deletion when done
            bReleaseHeap = false;       // Heap cannot be released at this moment
        }
    }

    // All blocks have been sucessfully deleted -> move all blocks to pool, unregister state heap
    if (bReleaseHeap)
    {
        // Return deleted blocks back to pool
        for (pBlock = pStateHeap->pMemoryHead; pBlock != nullptr; pBlock = pBlock->pHeapNext)
        {
            // Sanity check - all blocks must be in this state!
            if (pBlock->BlockState == MHW_BLOCK_STATE_DELETED)
            {
                DetachBlock(MHW_BLOCK_STATE_DELETED, pBlock);
                ReturnBlockToPool(pBlock);
            }
            else
            {
                // NEVER SUPPOSED TO HAPPEN DUE TO PREVIOUS LOOP
                MHW_ASSERTMESSAGE("ERROR: Mhw_BlockManager_UnregisterStateHeap: Invalid state, heap blocks are supposed to be all deleted by now");
            }
        }
        return MOS_STATUS_SUCCESS;
    }
    else
    {   // State heap cannot be unregistered because some blocks are still in use
        return MOS_STATUS_UNKNOWN;
    }
}

PMHW_STATE_HEAP_MEMORY_BLOCK MHW_BLOCK_MANAGER::GetBlockFromPool()
{
    PMHW_STATE_HEAP_MEMORY_BLOCK pBlock = nullptr;

    // Ran out of memory blocks... extend pool of memory block objects
    if (m_BlockList[MHW_BLOCK_STATE_POOL].iCount == 0)
    {
        ExtendPool(m_Params.dwPoolIncrement);
    }

    // Retrieve block object from head of the pool
    pBlock = DetachBlock(MHW_BLOCK_STATE_POOL, MHW_BLOCK_POSITION_HEAD);

    return pBlock;
}

void MHW_BLOCK_MANAGER::ExtendPool(uint32_t  dwCount)
{
    uint32_t dwBlockID = m_MemoryPool.m_dwObjCount; // Block ID starts from the current block count

    // Limits the number of memory blocks
    if (m_MemoryPool.m_dwCount + dwCount > m_Params.dwPoolMaxCount)
    {
        dwCount = m_Params.dwPoolMaxCount - m_MemoryPool.m_dwCount;
    }

    // Extend pool of block objects
    if (dwCount > 0)
    {
        // Allocate array of block objects into the pool
        MHW_STATE_HEAP_MEMORY_BLOCK *pBlockArray = (MHW_STATE_HEAP_MEMORY_BLOCK *) (m_MemoryPool.Allocate(dwCount));
        if (pBlockArray)
        {
            // Insert newly created block objects into the linked list of pool objects available to the memory block manager
            for (; dwCount > 0; dwCount--, pBlockArray++)
            {
                pBlockArray->dwBlockSize = 0;
                pBlockArray->pPrev       = pBlockArray->pNext = nullptr;
                pBlockArray->Reserved    = dwBlockID++;
                AttachBlock(MHW_BLOCK_STATE_POOL, pBlockArray, MHW_BLOCK_POSITION_TAIL);
            }
        }
    }
}

MOS_STATUS MHW_BLOCK_MANAGER::AttachBlock(
    MHW_BLOCK_STATE              BlockState,
    PMHW_STATE_HEAP_MEMORY_BLOCK pBlock,
    PMHW_STATE_HEAP_MEMORY_BLOCK pBlockPos)
{
    PMHW_BLOCK_LIST pList;
    MOS_STATUS      eStatus = MOS_STATUS_SUCCESS;

    // Verify parameters - objects cannot be null, block state must be valid
    if (pBlock == nullptr ||
        BlockState <  MHW_BLOCK_STATE_POOL ||
        BlockState >= MHW_BLOCK_STATE_COUNT)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    // Fails if block is still attached to a list
    if (pBlock->pPrev || pBlock->pNext)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    // Get list associated with block state; move block to the list
    pList = &m_BlockList[BlockState];
    BLOCK_MANAGER_CHK_STATUS(AttachBlockInternal(pList, BlockState, pBlock, pBlockPos));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MHW_BLOCK_MANAGER::AttachBlockInternal(
    PMHW_BLOCK_LIST              pList,
    MHW_BLOCK_STATE              BlockState,
    PMHW_STATE_HEAP_MEMORY_BLOCK pBlock,
    PMHW_STATE_HEAP_MEMORY_BLOCK pBlockPos)
{

    // Check if this is the correct list!
    if (pList->BlockState != BlockState)
    {
        MHW_ASSERTMESSAGE("ERROR: Mhw_BlockManager_AttachBlock_Internal: Block state doesn't match the list provided");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    // Setup the block state
    pBlock->BlockState = BlockState;

    // Attaches to the head of the list
    if (pBlockPos == MHW_BLOCK_POSITION_TAIL)
    {
        pBlock->pPrev = pList->pTail;
        pBlock->pNext = nullptr;
    }
    // Attaches to the tail of the list
    else if (pBlockPos == MHW_BLOCK_POSITION_HEAD)
    {
        pBlock->pPrev = nullptr;
        pBlock->pNext = pList->pHead;
    }
    // Insert after block provided - ensures that it belongs to the same list
    else if (pBlockPos->BlockState == BlockState)
    {
        pBlock->pPrev = pBlockPos;
        pBlock->pNext = pBlockPos->pNext;
    }
    // Insertion point does not belong to the same list
    else
    {
        MHW_ASSERTMESSAGE("ERROR: Mhw_BlockManager_AttachBlock_Internal: Reference block does not belong to the list provided");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    // Modify previous block or head of the list
    if (pBlock->pPrev)
    {
        pBlock->pPrev->pNext = pBlock;
    }
    else
    {
        pList->pHead = pBlock;
    }

    // Modify next block or tail of the list
    if (pBlock->pNext)
    {
        pBlock->pNext->pPrev = pBlock;
    }
    else
    {
        pList->pTail = pBlock;
    }

    // Track size and number of blocks in the list
    pList->dwSize += pBlock->dwBlockSize;
    pList->iCount++;

    return MOS_STATUS_SUCCESS;
}

PMHW_STATE_HEAP_MEMORY_BLOCK MHW_BLOCK_MANAGER::DetachBlock(
    MHW_BLOCK_STATE              BlockState,
    PMHW_STATE_HEAP_MEMORY_BLOCK pBlockPos)
{
    PMHW_BLOCK_LIST              pList;
    PMHW_STATE_HEAP_MEMORY_BLOCK pBlock =  nullptr;

    // Verify parameters - object cannot be null, block state must be valid
    if (BlockState <  MHW_BLOCK_STATE_POOL ||
        BlockState >= MHW_BLOCK_STATE_COUNT)
    {
        return nullptr;
    }

    // Get list associated with block state
    pList = &m_BlockList[BlockState];

    // Remove block from list, performing sanity check (check if block is in correct list)
    pBlock = DetachBlockInternal(pList, pBlockPos);

    return pBlock;
}

PMHW_STATE_HEAP_MEMORY_BLOCK MHW_BLOCK_MANAGER::DetachBlockInternal(
    PMHW_BLOCK_LIST              pList,
    PMHW_STATE_HEAP_MEMORY_BLOCK pBlock)
{

    if(pList == nullptr)
    {
        return nullptr;
    }

#ifdef MHW_DYNAMIC_STATE_HEAP_LOGGING
    const char *szPos = "REF ";
    if (pBlock == MHW_BLOCK_POSITION_HEAD)
        szPos = "HEAD";
    else if (pBlock == MHW_BLOCK_POSITION_TAIL)
        szPos = "TAIL";
#endif

    // Get block from the head of the list
    if (pBlock == MHW_BLOCK_POSITION_HEAD)
    {
        pBlock = pList->pHead;
    }
    // Get block from the tail of the list
    else if (pBlock == MHW_BLOCK_POSITION_TAIL)
    {
        pBlock = pList->pTail;
    }
    // Block does not belong to correct list - ASSERT and get block from head of the list
    else if (pBlock->BlockState != pList->BlockState)
    {
        MHW_ASSERTMESSAGE("ERROR: Mhw_BlockManager_DetachBlock_Internal: Block provided does not belong to the list provided");
        pBlock = nullptr;
    }

    if (!pBlock)
    {
        return nullptr;
    }

    // Detach block from previous; if first, update head of the list
    if (pBlock->pPrev)
    {
        pBlock->pPrev->pNext = pBlock->pNext;
    }
    else
    {
        pList->pHead = pBlock->pNext;
    }

    // Detach block from next; if last, update tail of the list
    if (pBlock->pNext)
    {
        pBlock->pNext->pPrev = pBlock->pPrev;
    }
    else
    {
        pList->pTail = pBlock->pPrev;
    }

    // reset pointers - block is detached
    pBlock->pNext = pBlock->pPrev = nullptr;

    // track size and number of block in the list
    pList->dwSize -= pBlock->dwBlockSize;
    pList->iCount--;

    return pBlock;
}

MOS_STATUS MHW_BLOCK_MANAGER::MoveBlock(
    PMHW_BLOCK_LIST              pSrcList,      // Source list
    PMHW_BLOCK_LIST              pDstList,      // Destination list
    PMHW_STATE_HEAP_MEMORY_BLOCK pBlock,        // Block to be moved (or HEAD/TAIL of source list)
    PMHW_STATE_HEAP_MEMORY_BLOCK pBlockPos)     // Position to insert (or HEAD/TAIL of target list)
{
    MOS_STATUS      eStatus = MOS_STATUS_SUCCESS;

    BLOCK_MANAGER_CHK_NULL(pSrcList);
    BLOCK_MANAGER_CHK_NULL(pDstList);

    pBlock = DetachBlockInternal(pSrcList, pBlock);
    BLOCK_MANAGER_CHK_NULL(pBlock);

    eStatus = AttachBlockInternal(pDstList, pDstList->BlockState, pBlock, pBlockPos);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        MHW_ASSERTMESSAGE("ERROR: Mhw_BlockManager_MoveBlock_Internal: Failed to move block");
        AttachBlockInternal(pDstList, pDstList->BlockState, pBlock, MHW_BLOCK_POSITION_TAIL);
    }

    return eStatus;
}

void MHW_BLOCK_MANAGER::ReturnBlockToPool(PMHW_STATE_HEAP_MEMORY_BLOCK pBlock)
{
    pBlock->dwBlockSize = 0;
    pBlock->pPrev       = pBlock->pNext = nullptr;

    AttachBlock(MHW_BLOCK_STATE_POOL, pBlock, MHW_BLOCK_POSITION_TAIL);

    return;
}

MOS_STATUS MHW_BLOCK_MANAGER::Refresh()
{
    PMHW_STATE_HEAP_MEMORY_BLOCK pBlock, pNext;
    PMHW_BLOCK_LIST              pList;
    MOS_STATUS                   eStatus = MOS_STATUS_SUCCESS;

    // Refresh status of SUBMITTED blocks
    pList  = &m_BlockList[MHW_BLOCK_STATE_SUBMITTED];
    pNext  = nullptr;
    for (pBlock = pList->pHead; pBlock != nullptr; pBlock = pNext)
    {
        // check of block may be released - if not, continue loop
        // NOTE - blocks are to be inserted in the sequence of execution,
        //        so we may finish the search as soon as we find the first
        //        block still "in use" (block tag > current sync tag)
        //        For now we're doing an exhaustive search, but it may not be needed - once
        //        we find the first block still in execution, we can probably stop the search

        // Save next pointer before moving the block to another queue
        pNext = pBlock->pNext;

        // Check if block is still in use, if so, continue search
        // NOTE - the following expression avoids sync tag wrapping around MAX_INT -> 0
        if (!FrameTrackerTokenFlat_IsExpired(&pBlock->trackerToken))
        {
            continue;
        }

        // Block is flagged for deletion
        if (pBlock->bDelete)
        {
            BLOCK_MANAGER_CHK_STATUS(FreeBlock(pBlock));
        }
        // Block is static -> move back to allocated list
        else if (pBlock->bStatic)
        {
            BLOCK_MANAGER_CHK_STATUS(MoveBlock(pList, &m_BlockList[MHW_BLOCK_STATE_ALLOCATED],
                           pBlock, MHW_BLOCK_POSITION_TAIL));
        }
        else
        // Block is non-static  -> free block
        {
            FreeBlock(pBlock);
        }
    }

    return eStatus;
}

void MHW_BLOCK_MANAGER::ConsolidateBlock(
    PMHW_STATE_HEAP_MEMORY_BLOCK pBlock)
{
    PMHW_STATE_HEAP_MEMORY_BLOCK pAux;

    // Check input parameters
    if (!pBlock || pBlock->BlockState != MHW_BLOCK_STATE_FREE)
    {
        return;
    }

    // Consolidate pBlock with previous blocks
    PMHW_BLOCK_LIST pFree = &m_BlockList[MHW_BLOCK_STATE_FREE];
    for (pAux = pBlock->pHeapPrev; (pAux != nullptr) && (pAux->BlockState == MHW_BLOCK_STATE_FREE); pAux = pBlock->pHeapPrev)
    {
        // Remove block from free block list
        DetachBlock(MHW_BLOCK_STATE_FREE, pAux);

        // Absorb free space into original block by adjusting offset/size
        pBlock->dwOffsetInStateHeap -= pAux->dwBlockSize;
        pBlock->dwBlockSize         += pAux->dwBlockSize;
        pFree->dwSize               += pAux->dwBlockSize;

        // Detach memory block from sequential memory list
        pBlock->pHeapPrev = pAux->pHeapPrev;
        if (pBlock->pHeapPrev)
        {
            pBlock->pHeapPrev->pHeapNext = pBlock;
        }
        else
        {
            pBlock->pStateHeap->pMemoryHead = pBlock;
        }

        // Memory block object no longer needed - return to pool after consolidation
        ReturnBlockToPool(pAux);
    }

    // Consolidate pBlock with following blocks
    for (pAux = pBlock->pHeapNext; (pAux != nullptr) && (pAux->BlockState == MHW_BLOCK_STATE_FREE); pAux = pBlock->pHeapNext)
    {
        // Remove block from free block list
        DetachBlock(MHW_BLOCK_STATE_FREE, pAux);

        // Absorb free space into original block by adjusting block size (offset remains the same)
        pBlock->dwBlockSize += pAux->dwBlockSize;
        pFree->dwSize       += pAux->dwBlockSize;

        // Detach memory block from sequential memory list
        pBlock->pHeapNext = pAux->pHeapNext;
        if (pBlock->pHeapNext)
        {
            pBlock->pHeapNext->pHeapPrev = pBlock;
        }
        else
        {
            pBlock->pStateHeap->pMemoryTail = pBlock;
        }

        // Memory block object no longer needed - return to pool after consolidation
        ReturnBlockToPool(pAux);
    }
}

MOS_STATUS MHW_BLOCK_MANAGER::AllocateBlockInternal(
    PMHW_STATE_HEAP_MEMORY_BLOCK pBlock,
    uint32_t                     dwAlignment)
{
    PMHW_STATE_HEAP pHeap;
    MOS_STATUS      eStatus = MOS_STATUS_SUCCESS;
    BLOCK_MANAGER_CHK_NULL(pBlock);

    // Remove block from free list - if block is not in free list, this operation will fail and return nullptr
    pBlock = DetachBlock( MHW_BLOCK_STATE_FREE, pBlock);
    BLOCK_MANAGER_CHK_NULL(pBlock);

    // Initialize block data structures
    pBlock->bDelete      = false;

    pBlock->dwDataOffset = MOS_ALIGN_CEIL(pBlock->dwOffsetInStateHeap, dwAlignment);
    pBlock->dwAlignment  = pBlock->dwDataOffset - pBlock->dwOffsetInStateHeap;
    pBlock->dwDataSize   = pBlock->dwBlockSize  - pBlock->dwAlignment;
    pBlock->pDataPtr     = (uint8_t*)pBlock->pStateHeap->pvLockedHeap + pBlock->dwDataOffset;

    // Move block to allocated list
    AttachBlock(MHW_BLOCK_STATE_ALLOCATED, pBlock, MHW_BLOCK_POSITION_TAIL);

    // Update available space in heap
    pHeap = pBlock->pStateHeap;
    pHeap->dwFree -= pBlock->dwBlockSize;
    pHeap->dwUsed += pBlock->dwBlockSize;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MHW_BLOCK_MANAGER::SplitBlockInternal(
    PMHW_STATE_HEAP_MEMORY_BLOCK    pBlock,
    uint32_t                        dwSplitSize,
    uint32_t                        dwAlignment,
    bool                            bBackward)
{
    uint32_t                     dwSplitOffset = 0;
    MOS_STATUS                   eStatus = MOS_STATUS_SUCCESS;
    PMHW_STATE_HEAP_MEMORY_BLOCK pBlockL, pBlockH;

    BLOCK_MANAGER_CHK_NULL(pBlock);

    // Split cannot be less than min block size allowed
    dwSplitSize = MOS_MAX(dwSplitSize, m_Params.dwHeapBlockMinSize);
    if (pBlock->dwBlockSize < dwSplitSize)
    {
        return MOS_STATUS_UNKNOWN;
    }

    // Verify block state
    if (pBlock->BlockState <= MHW_BLOCK_STATE_POOL  ||          // Cannot split a block object from pool (contains invalid data)
        pBlock->BlockState >= MHW_BLOCK_STATE_DELETED)          // Cannot split a block being deleted
    {
        return MOS_STATUS_INVALID_PARAMETER;

    }

    // Select list
    PMHW_BLOCK_LIST pList = &(m_BlockList[pBlock->BlockState]);

    if (bBackward)
    {
        // Split FROM THE END of the block (higher offset)
        dwSplitOffset = MOS_ALIGN_FLOOR(pBlock->dwOffsetInStateHeap + pBlock->dwBlockSize - dwSplitSize, dwAlignment);
        dwSplitOffset = MOS_ALIGN_FLOOR(dwSplitOffset, m_Params.dwHeapGranularity);
    }
    else
    {
        // Split FROM THE BEGINNING of the block (lower offset)
        dwSplitOffset = MOS_ALIGN_CEIL(pBlock->dwOffsetInStateHeap, dwAlignment);
        dwSplitOffset = MOS_ALIGN_CEIL(dwSplitOffset + dwSplitSize, m_Params.dwHeapGranularity);
    }

    // Fail if block cannot be split
    if (dwSplitOffset < pBlock->dwOffsetInStateHeap + m_Params.dwHeapBlockMinSize ||                       // First fragment is too small
        pBlock->dwOffsetInStateHeap + pBlock->dwBlockSize < dwSplitOffset + m_Params.dwHeapBlockMinSize)   // Second fragment is too small
    {
        return MOS_STATUS_UNKNOWN;
    }

    if (bBackward)
    {
        pBlockH = pBlock;       // We'll keep the high end of the block
        pBlockL = GetBlockFromPool();
        BLOCK_MANAGER_CHK_NULL(pBlockL);

        uint32_t reserved = pBlockL->Reserved;
        *pBlockL = *pBlock;
        pBlockL->Reserved = reserved;

        if (pBlock->pPrev)
        {
            pBlock->pPrev->pNext = pBlockL;
        }
        else
        {
            pList->pHead = pBlockL;
        }

        if (pBlock->pHeapPrev)
        {
            pBlock->pHeapPrev->pHeapNext = pBlockL;
        }
        else
        {
            pBlock->pStateHeap->pMemoryHead = pBlockL;
        }
    }
    else
    {
        pBlockL = pBlock;       // We'll keep the low end of the block
        pBlockH = GetBlockFromPool();
        BLOCK_MANAGER_CHK_NULL(pBlockH);

        uint32_t reserved = pBlockH->Reserved;
        *pBlockH = *pBlock;
        pBlockH->Reserved = reserved;

        if (pBlock->pNext)
        {
            pBlock->pNext->pPrev = pBlockH;
        }
        else
        {
            pList->pTail = pBlockH;
        }

        if (pBlock->pHeapNext)
        {
            pBlock->pHeapNext->pHeapPrev = pBlockH;
        }
        else
        {
            pBlock->pStateHeap->pMemoryTail = pBlockH;
        }
    }

    // Update block adjacency list
    pBlockL->pHeapNext    = pBlockH;
    pBlockH->pHeapPrev    = pBlockL;

    // Ensures that the new block is tracked in the same list as the parent block, update block count
    pList->iCount++;
    pBlockL->pNext = pBlockH;
    pBlockH->pPrev = pBlockL;

    // Adjust Block sizes
    pBlockL->dwBlockSize         = dwSplitOffset - pBlockL->dwOffsetInStateHeap;   // Updates L block size based on split offset
    pBlockH->dwOffsetInStateHeap = dwSplitOffset;                                  // Sets 2nd block offset
    pBlockH->dwBlockSize        -= pBlockL->dwBlockSize;                           // Updates H block size by subtracting L block size

    // Adjust Block data related pointers/sizes only if block is not free
    if (pBlockL->BlockState != MHW_BLOCK_STATE_FREE)
    {
        pBlockL->dwDataSize  -= pBlockH->dwBlockSize;                           // Removes size of new block from amount of data available
        pBlockH->dwDataOffset = MOS_ALIGN_CEIL(dwSplitOffset, dwAlignment);     // Adjust offset to data (accounting for alignment)
        pBlockH->dwAlignment  = pBlockH->dwDataOffset - dwSplitOffset;          // Calculate alignment shift
        pBlockH->dwDataSize   = pBlockH->dwBlockSize - dwAlignment;             // Adjust amount of data available
        pBlockH->pDataPtr     = (uint8_t*)pBlockH->pStateHeap->pvLockedHeap + pBlockH->dwDataOffset; // Setup pointer to data (the heap is locked)
    }

    return eStatus;
}

MOS_STATUS MHW_BLOCK_MANAGER::MergeBlocksInternal(
    PMHW_STATE_HEAP_MEMORY_BLOCK    pBlockL,        // block in lower memory
    PMHW_STATE_HEAP_MEMORY_BLOCK    pBlockH,        // block in higher memory
    uint32_t                        dwAlignment,    // final block alignment
    bool                            bBackward)      // true if pBlockL (free) is merged into pBlockH; false if pBlockH (free) is merged into pBlockL
{
    PMHW_BLOCK_LIST pList;
    MOS_STATUS      eStatus = MOS_STATUS_SUCCESS;

    BLOCK_MANAGER_CHK_NULL(pBlockL);
    BLOCK_MANAGER_CHK_NULL(pBlockH);

    // Blocks must be contiguous in memory
    if (pBlockL->pHeapNext != pBlockH ||
        pBlockH->pHeapPrev != pBlockL)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    // 1st block (L) is merged into 2nd (H)
    if (bBackward)
    {
        if (pBlockL->BlockState != MHW_BLOCK_STATE_FREE ||      // 1st block must be free
            pBlockH->BlockState <  MHW_BLOCK_STATE_FREE ||      // 2nd block cannot be in pool
            pBlockH->BlockState >  MHW_BLOCK_STATE_SUBMITTED)   //           or deleted
        {
            return MOS_STATUS_INVALID_PARAMETER;
        }

        // Merge blocks
        pBlockL = DetachBlock(MHW_BLOCK_STATE_FREE, pBlockL);
        BLOCK_MANAGER_CHK_NULL(pBlockL);

        pBlockH->dwOffsetInStateHeap  = pBlockL->dwOffsetInStateHeap;
        pBlockH->dwBlockSize         += pBlockL->dwBlockSize;

        // Add size to the target block list
        pList = &(m_BlockList[pBlockH->BlockState]);
        pList->dwSize += pBlockL->dwBlockSize;

        // If block allocated or submitted, adjust data references (don't care if block is free)
        if (pBlockH->BlockState != MHW_BLOCK_STATE_FREE)
        {
            pBlockH->dwDataOffset   = MOS_ALIGN_CEIL(pBlockH->dwOffsetInStateHeap, dwAlignment);
            pBlockH->dwAlignment    = pBlockH->dwDataOffset - pBlockH->dwOffsetInStateHeap;
            pBlockH->dwDataSize     = pBlockH->dwBlockSize  - pBlockH->dwAlignment;
            pBlockH->pDataPtr       = (uint8_t*)pBlockH->pStateHeap->pvLockedHeap + pBlockH->dwDataOffset;

            // Free block is now in use - track heap usage
            pBlockH->pStateHeap->dwFree -= pBlockL->dwBlockSize;
            pBlockH->pStateHeap->dwUsed += pBlockL->dwBlockSize;
        }

        // Return block object to the pool
        ReturnBlockToPool(pBlockL);
    }
    else
    // 2nd block (H) is merged into 1st (L)
    {
        if (pBlockH->BlockState != MHW_BLOCK_STATE_FREE ||       // 2nd block must be free
            pBlockL->BlockState <  MHW_BLOCK_STATE_FREE ||       // 1nd block must not be in pool
            pBlockL->BlockState >  MHW_BLOCK_STATE_SUBMITTED)    //           or deleted
        {
            return MOS_STATUS_INVALID_PARAMETER;
        }

        // Merge blocks
        pBlockH = DetachBlock(MHW_BLOCK_STATE_FREE, pBlockH);
        BLOCK_MANAGER_CHK_NULL(pBlockH);

        pBlockL->dwBlockSize += pBlockH->dwBlockSize;
        if (pBlockL->BlockState != MHW_BLOCK_STATE_FREE)
        {
            pBlockL->dwDataSize         += pBlockH->dwBlockSize;
            pBlockL->pStateHeap->dwFree -= pBlockL->dwBlockSize;
            pBlockL->pStateHeap->dwUsed += pBlockL->dwBlockSize;
        }

        // Add size to the target block list
        pList = &(m_BlockList[pBlockL->BlockState]);
        pList->dwSize += pBlockH->dwBlockSize;

        // Return block object to the pool
        ReturnBlockToPool(pBlockH);
    }

    return eStatus;
}

MOS_STATUS MHW_BLOCK_MANAGER::ResizeBlock(
    PMHW_STATE_HEAP_MEMORY_BLOCK    pBlock,
    uint32_t                        dwNewSize,
    uint32_t                        dwAlignment,
    bool                            bBackward)      // false => Always grow/shrink forward; true => allow block to grow forward/backwards (moving its start offset)
{
    MOS_STATUS                   eStatus = MOS_STATUS_SUCCESS;
    PMHW_STATE_HEAP_MEMORY_BLOCK pNewBlock;

    BLOCK_MANAGER_CHK_NULL(pBlock);
    MHW_ASSERT(dwNewSize > 0);

    // Verify block state
    if (pBlock->BlockState <= MHW_BLOCK_STATE_POOL  ||          // Cannot touch a block object from pool - invalid data
        pBlock->BlockState >= MHW_BLOCK_STATE_DELETED)          // Cannot touch a block being deleted
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    // Shrinking current block
    if (dwNewSize < pBlock->dwBlockSize)
    {
        // Split block into 2 - (bBackwards -> shrink the block by keeping the 2nd half (anchor end of the block)
        eStatus = SplitBlockInternal(pBlock, dwNewSize, dwAlignment, bBackward);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            // This error just means that the block cannot shrink - not an actual issue here
            if (eStatus == MOS_STATUS_UNKNOWN)
            {
                eStatus = MOS_STATUS_SUCCESS;
                return eStatus;
            }
        }

        // Select block to be freed
        pBlock = (bBackward) ? pBlock->pPrev : pBlock->pNext;
        BLOCK_MANAGER_CHK_NULL(pBlock);

        if (pBlock->BlockState == MHW_BLOCK_STATE_SUBMITTED)
        {
            // mark block for release as soon it is no longer in use by GPU
            pBlock->bStatic  = false;
        }
        else
        {
            // Free block - block is in allocated state
            FreeBlock(pBlock);
        }

        // Success!
        return MOS_STATUS_SUCCESS;
    }

    // Check for contiguous available space in forward direction first
    uint32_t dwAvailable = pBlock->dwDataSize;
    for (pNewBlock = pBlock->pHeapNext;
         (dwAvailable < dwNewSize) && (pNewBlock) && (pNewBlock->BlockState == MHW_BLOCK_STATE_FREE);
         pNewBlock = pNewBlock->pHeapNext)
    {
        dwAvailable += pNewBlock->dwBlockSize;
    }

    // Check for contiguous available space in backward direction
    if (bBackward)
    {
        // Update available space to account for block alignment
        dwAvailable += pBlock->dwAlignment - dwAlignment;
        for (pNewBlock = pBlock->pHeapPrev;
             (dwAvailable < dwNewSize) && (pNewBlock) && (pNewBlock->BlockState == MHW_BLOCK_STATE_FREE);
             pNewBlock = pNewBlock->pHeapPrev)
        {
            dwAvailable += pNewBlock->dwBlockSize;
        }
    }

    // Check if block can be resized
    if (dwAvailable < dwNewSize)
    {
        return MOS_STATUS_UNKNOWN;
    }

    // Start block expansion forward
    for (pNewBlock = pBlock->pHeapNext;
         (pBlock->dwDataSize < dwNewSize) && (pNewBlock) && (pNewBlock->BlockState == MHW_BLOCK_STATE_FREE);
         pNewBlock = pBlock->pHeapNext)
    {
        // Next block is too large - split the block before merging
        if (pBlock->dwDataSize + pNewBlock->dwBlockSize > dwNewSize)
        {
            SplitBlockInternal(pNewBlock, dwNewSize - pBlock->dwDataSize, dwAlignment, false);
        }

        // Merge block with next
        MergeBlocksInternal(pBlock, pNewBlock, dwAlignment, false);
    }

    // Continue expanding backward
    if (bBackward)
    {
        for (pNewBlock = pBlock->pHeapPrev;
             (dwAvailable < dwNewSize) && (pNewBlock) && (pNewBlock->BlockState == MHW_BLOCK_STATE_FREE);
             pNewBlock = pBlock->pHeapPrev)
        {
            // Prev block is too large - split the block before merging
            uint32_t dwAdjust = MOS_ALIGN_CEIL(pNewBlock->dwOffsetInStateHeap, dwAlignment) - pNewBlock->dwOffsetInStateHeap;
            if (pBlock->dwBlockSize + pNewBlock->dwBlockSize - dwAdjust > dwNewSize)
            {
                SplitBlockInternal(pNewBlock, dwNewSize - pBlock->dwBlockSize, dwAlignment, true);
            }

            // Merge block with previous
            MergeBlocksInternal(pNewBlock, pBlock, dwAlignment, true);
        }
    }

    return eStatus;
}

PMHW_STATE_HEAP_MEMORY_BLOCK MHW_BLOCK_MANAGER::AllocateWithScratchSpace(
    uint32_t            dwSize,
    uint32_t            dwAlignment,
    uint32_t            dwScratchSpace)
{
    MOS_STATUS                   eStatus    = MOS_STATUS_SUCCESS;
    PMHW_STATE_HEAP_MEMORY_BLOCK pBlock     = nullptr;
    PMHW_STATE_HEAP_MEMORY_BLOCK pScratch   = nullptr;

    // Fix alignment - must be a power of 2
    if (dwAlignment < m_Params.dwHeapGranularity)
    {
        // Blocks are already aligned
        dwAlignment = 1;
    }
    else
    {
        dwAlignment--;
        dwAlignment |= dwAlignment >>  1;
        dwAlignment |= dwAlignment >>  2;
        dwAlignment |= dwAlignment >>  4;
        dwAlignment |= dwAlignment >>  8;
        dwAlignment |= dwAlignment >> 16;
        dwAlignment++;
    }

    // Try to search search state heap with large enough scratch space
    // and with enough free space
    PMHW_STATE_HEAP pNextStateHeap;
    for (PMHW_STATE_HEAP pStateHeap = m_pStateHeap; (pStateHeap); pStateHeap = pNextStateHeap)
    {
        // Save next state heap
        pNextStateHeap = pStateHeap->pNext;

        // Space needed for block (accounting for alignment and for scratch space)
        uint32_t dwBlockNeeded = MOS_ALIGN_CEIL(dwSize + dwAlignment - 1, m_Params.dwHeapGranularity);

        // Scratch space needed = space requested + room for alignment - space already allocated
        uint32_t dwScratchNeeded = 0;
        if (dwScratchSpace > 0 && pStateHeap->dwScratchSpace < dwScratchSpace)
        {
            dwScratchNeeded = dwScratchSpace;
            if (m_Params.dwHeapGranularity < MHW_SCRATCH_SPACE_ALIGN)
            {
                dwScratchNeeded += MHW_SCRATCH_SPACE_ALIGN - m_Params.dwHeapGranularity;
            }
            if (pStateHeap->pScratchSpace)
            {
                dwScratchNeeded -= pStateHeap->pScratchSpace->dwBlockSize;
            }
        }

        if (pStateHeap->dwSize < dwScratchNeeded)
        {
            // Heap is too small for current scratch space size - mark for deletion
            pStateHeap->pMhwStateHeapInterface->ReleaseStateHeapDyn(pStateHeap);
            continue;
        }
        else if (pStateHeap->dwFree < (dwScratchNeeded + dwBlockNeeded))
        {
            // Heap can still be used, but currently full, try next heap
            continue;
        }

        // Allocate scratch space first
        if (dwScratchNeeded)
        {
            pScratch = nullptr;

            // Already present - EXPAND SCRATCH SPACE
            if (pStateHeap->pScratchSpace)
            {
                // Resize existing scratch space trying to use all free space towards the end of the heap and then growing towards the center.
                eStatus = ResizeBlock(pStateHeap->pScratchSpace, dwScratchSpace, MHW_SCRATCH_SPACE_ALIGN, true);
                if (eStatus == MOS_STATUS_SUCCESS)
                {
                    pScratch = pStateHeap->pScratchSpace;                // Indicates success
                    pStateHeap->dwScratchSpace = pScratch->dwDataSize;   // Available scratch space size (aligned)
                }
            }

            // Scratch space not present or failed to expand - find a new scratch space
            if (!pScratch)
            {
                // Search for scratch space at the end of the heap towards the beginning
                // This model allows for better growth without collision with media state heaps
                for (pScratch = pStateHeap->pMemoryTail; pScratch != pBlock; pScratch = pScratch->pHeapPrev)
                {
                    if (pScratch->BlockState == MHW_BLOCK_STATE_FREE &&
                        pScratch->dwBlockSize >= dwScratchNeeded)
                    {
                        // Found scratch space large enough
                        break;
                    }
                }

                // Not enough contiguous space for scratch in heap - try next heap
                if (!pScratch)
                {
                    continue;
                }

                // CREATE SCRATCH SPACE

                // Split block to the necessary size, using the higher portion (closer to end of the heap)
                // NOTE: Block select could be much larger than needed, even the size of the entire state heap - that's why it needs to be split
                eStatus = SplitBlockInternal(pScratch, dwScratchNeeded, MHW_SCRATCH_SPACE_ALIGN, true);
                if (eStatus == MOS_STATUS_UNKNOWN) eStatus = MOS_STATUS_SUCCESS;   // Don't care if block could not be split
                if (eStatus != MOS_STATUS_SUCCESS)
                {
                    continue;
                }

                // Move block to allocated list, mark it as static (do not release upon completion)
                AllocateBlockInternal(pScratch, MHW_SCRATCH_SPACE_ALIGN);
                pScratch->bStatic = true;

                // Free the old scratch space
                // NOTE: scratch spaces are also tracked by Sync Tags and maintained in submitted/allocated lists,
                //       If in use, it just clears the bStatic flag, so it will be freed when no longer in use.
                if (pStateHeap->pScratchSpace)
                {
                    FreeBlock(pStateHeap->pScratchSpace);
                }

                // Setup new heap scratch space and size
                pStateHeap->pScratchSpace  = pScratch;
                pStateHeap->dwScratchSpace = pScratch->dwDataSize;
            }
        }

        // Try to allocate block in the same heap as the scratch space
        pBlock = AllocateBlock(dwSize, dwAlignment, pStateHeap);
        if (pBlock)
        {
            break;
        }
    }

    return pBlock;
}

PMHW_STATE_HEAP_MEMORY_BLOCK MHW_BLOCK_MANAGER::AllocateBlock(
    uint32_t            dwSize,
    uint32_t            dwAlignment,
    PMHW_STATE_HEAP     pHeapAffinity)
{
    PMHW_STATE_HEAP_MEMORY_BLOCK pBlock = nullptr;
    PMHW_BLOCK_LIST              pFree  = &m_BlockList[MHW_BLOCK_STATE_FREE];
    uint32_t                     dwAdjust;     // Offset adjustment for alignment purposes
    uint32_t                     dwAllocSize;  // Actual allocation size accounting for alignment and other restrictions
    MOS_STATUS                   eStatus = MOS_STATUS_SUCCESS;

    // Fix alignment - must be a power of 2 (minimum 1)
    if (dwAlignment) dwAlignment--;
    dwAlignment |= dwAlignment >>  1;
    dwAlignment |= dwAlignment >>  2;
    dwAlignment |= dwAlignment >>  4;
    dwAlignment |= dwAlignment >>  8;
    dwAlignment |= dwAlignment >> 16;
    dwAlignment++;

    // Search must include space for block granularity
    if (dwAlignment <= m_Params.dwHeapGranularity)
    {
        // Alignment should be fulfilled by the heap granularity
        dwAllocSize = dwSize;
    }
    else
    {
        // Worst case scenario - original implementation was checking alignment of
        // each free block, but it was overkill - so now we just consider the worst case
        dwAllocSize = dwSize + dwAlignment - m_Params.dwHeapGranularity;
    }

    // Enforce min block size
    dwAllocSize = MOS_MAX(m_Params.dwHeapBlockMinSize, dwAllocSize);

    // Search list of free blocks for the first match
    for (pBlock = pFree->pHead; pBlock != nullptr; pBlock = pBlock->pNext)
    {
        // Skip this heap if we are looking for allocation in a specific heap
        if (pHeapAffinity  && pBlock->pStateHeap != pHeapAffinity)
        {
            continue;
        }

        // Check if aligned block fits the request -> break with a successful block
        if (pBlock->dwBlockSize >= dwAllocSize)
        {
            break;
        }
    }

    // No block was found - fail search
    if (!pBlock)
    {
        return nullptr;
    }

    // Block was found, adjust the allocation size to account for
    // heap granularity and block alignment
    dwAdjust    = MOS_ALIGN_OFFSET(pBlock->dwOffsetInStateHeap, dwAlignment);                 // Increase in size to align data
    dwAllocSize = MOS_ALIGN_CEIL(dwSize + dwAdjust, m_Params.dwHeapGranularity); // Account for heap granularity (avoid odd addresses in heap)
    dwAllocSize = MOS_MAX(dwAllocSize, m_Params.dwHeapBlockMinSize);

    // Just a precaution - sanity check - in case of last block in heap, and total heap size is not a multiple of granularity
    if (pBlock->dwBlockSize < dwAllocSize)
    {
        // This should never happend because it is part of the search condition!
        MHW_ASSERT(pBlock->dwBlockSize >= (dwAdjust + dwSize));

        dwAllocSize = pBlock->dwBlockSize;
    }

    // Split block, move to allocated list
    if (pBlock->dwBlockSize > dwAllocSize)
    {
        // Split free block in 2, keep the first part (lower offset)
        eStatus = SplitBlockInternal(pBlock, dwAllocSize, dwAlignment, false);
        if (eStatus != MOS_STATUS_SUCCESS &&
            eStatus != MOS_STATUS_UNKNOWN)
        {
            MHW_ASSERTMESSAGE("ERROR: AllocateBlock: Failed to allocate block");
            return nullptr;
        }
    }

    // Move block from free to allocated queue
    DetachBlock(MHW_BLOCK_STATE_FREE,      pBlock);
    AttachBlock(MHW_BLOCK_STATE_ALLOCATED, pBlock, MHW_BLOCK_POSITION_TAIL);
    pBlock->pStateHeap->dwUsed += pBlock->dwBlockSize;
    pBlock->pStateHeap->dwFree -= pBlock->dwBlockSize;

    // Reset some fields
    pBlock->bDelete       = false;
    FrameTrackerTokenFlat_Validate(&pBlock->trackerToken);

    // Setup aligned offset, size and data
    pBlock->dwDataOffset = MOS_ALIGN_CEIL(pBlock->dwOffsetInStateHeap, dwAlignment);
    pBlock->dwAlignment  = pBlock->dwDataOffset - pBlock->dwOffsetInStateHeap;
    pBlock->dwDataSize   = pBlock->dwBlockSize  - pBlock->dwAlignment;
    pBlock->pDataPtr     = (uint8_t*)pBlock->pStateHeap->pvLockedHeap + pBlock->dwDataOffset;

    // return block to client
    return pBlock;
}

MOS_STATUS MHW_BLOCK_MANAGER::FreeBlock(
    PMHW_STATE_HEAP_MEMORY_BLOCK pBlock)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    BLOCK_MANAGER_CHK_NULL(pBlock);

    // Block still in use - mark for auto-release when complete
    if (pBlock->BlockState == MHW_BLOCK_STATE_SUBMITTED)
    {
        // sync tag not provided or block still in use - flag it for automatic release when complete
        if (!FrameTrackerTokenFlat_IsExpired(&pBlock->trackerToken))
        {
            pBlock->bStatic  = false;
            return eStatus;
        }
    }
    else if (pBlock->BlockState != MHW_BLOCK_STATE_ALLOCATED)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    // Remove block from its current list
    DetachBlock(pBlock->BlockState, pBlock);

    // If block is marked for deletion - move to deleted list so it cannot be reallocated
    if (pBlock->bDelete)
    {
        MHW_STATE_HEAP *pStateHeap = pBlock->pStateHeap;
        pStateHeap->dwUsed -= pBlock->dwBlockSize;

        AttachBlock(MHW_BLOCK_STATE_DELETED, pBlock, MHW_BLOCK_POSITION_TAIL);

        // Last block was removed from StateHeap -> state heap may be unregistered and deleted
        if (pStateHeap->dwUsed == 0)
        {
            pStateHeap->pMhwStateHeapInterface->ReleaseStateHeapDyn(pStateHeap);
        }
    }
    else
    {
        // Update state heap usage
        pBlock->pStateHeap->dwUsed -= pBlock->dwBlockSize;
        pBlock->pStateHeap->dwFree += pBlock->dwBlockSize;

        // Blocks are freed and placed at the beginning of the free block list
        AttachBlock(MHW_BLOCK_STATE_FREE, pBlock, MHW_BLOCK_POSITION_TAIL);

        // Consolidate memory immediately after release - so free block are ALWAYS merged
        ConsolidateBlock(pBlock);
    }

    return eStatus;
}

uint32_t MHW_BLOCK_MANAGER::CalculateSpaceNeeded(
    const uint32_t      *pdwSizes,
    int32_t             iCount,
    uint32_t            dwAlignment,
    bool                bHeapAffinity,
    PMHW_STATE_HEAP     pHeapAffinity)
{
    uint32_t                     dwNeeded = 0;
    uint8_t                      SortedIndex[MHW_BLOCK_MANAGER_MAX_BLOCK_ARRAY];
    uint32_t                     FreeBlockSizes[MHW_BLOCK_MANAGER_MAX_BLOCK_ARRAY * 2];
    uint32_t                     dwBlockOverhead = 0;   // Block size overhead to ensure alignment
    uint32_t                     dwAllocSize;

    // Check parameters
    if (iCount <= 0 || iCount > MHW_BLOCK_MANAGER_MAX_BLOCK_ARRAY)
    {
        return dwNeeded;
    }

    // This is the minimum block
    uint32_t dwBlockGranularity = m_Params.dwHeapGranularity;
    uint32_t dwBlockMinSize     = m_Params.dwHeapBlockMinSize;
    if (dwAlignment > dwBlockGranularity)
    {
        dwBlockOverhead = dwAlignment - dwBlockGranularity;
    }

    // Very simple case - single block search
    if (iCount == 1)
    {
        PMHW_STATE_HEAP_MEMORY_BLOCK pBlock = m_BlockList[MHW_BLOCK_STATE_FREE].pHead;
        dwNeeded = pdwSizes[0];
        for ( ; pBlock != nullptr && dwNeeded > 0; pBlock = pBlock->pNext)
        {
            if (bHeapAffinity && pHeapAffinity != pBlock->pStateHeap)
            {
                continue;
            }

            if (dwNeeded < pBlock->dwBlockSize)
            {
                dwNeeded = 0;
            }
        }

        return dwNeeded;
    }

    // Sort input block sizes (with index, to avoid modifying the input array)
    Mhw_BlockManager_ReverseMergeSort_With_Index(pdwSizes, iCount, SortedIndex);

    // Read all available blocks, keep the largest blocks
    int iIndex = 0;
    PMHW_STATE_HEAP_MEMORY_BLOCK pBlock = m_BlockList[MHW_BLOCK_STATE_FREE].pHead;
    for ( ; pBlock != nullptr; pBlock = pBlock->pNext)
    {
        // Select free blocks that fit the request
        if (bHeapAffinity && pHeapAffinity != pBlock->pStateHeap) continue;
        FreeBlockSizes[iIndex++] = pBlock->dwBlockSize;

        // If buffer is full, sort it, only take the largest blocks (overwrite remaining blocks)
        if (iIndex == MHW_BLOCK_MANAGER_MAX_BLOCK_ARRAY * 2)
        {
            Mhw_BlockManager_ReverseMergeSort(FreeBlockSizes, iIndex);
            iIndex = iCount;
        }
    }

    // Final sort after all block are in
    Mhw_BlockManager_ReverseMergeSort(FreeBlockSizes, iIndex);
    FreeBlockSizes[iIndex] = 0; // Null termination simplifies the algorithm that follows

    // Start process of fitting requested blocks with available blocks
    uint32_t *pFreeBlock = &FreeBlockSizes[0];
    for (iIndex = 0; iIndex < iCount; iIndex++)
    {
        dwAllocSize = MOS_ALIGN_CEIL(pdwSizes[SortedIndex[iIndex]] + dwBlockOverhead, dwBlockGranularity);
        dwAllocSize = MOS_MAX(dwAllocSize, dwBlockMinSize);

        // Block doesn't fit - add to space needed
        if (dwAllocSize > *pFreeBlock)
        {
            dwNeeded += dwAllocSize;
        }
        else
        {
            // Allocate block
            uint32_t dwRemaining = *pFreeBlock = *pFreeBlock - dwAllocSize;
            uint32_t          *pAux;

            // Remaining is out of order, keep list sorted using insertion sort
            if (dwRemaining < pFreeBlock[1])
            {
                for (pAux = pFreeBlock; dwRemaining < pAux[1]; pAux++) pAux[0] = pAux[1];
                pAux[0] = dwRemaining;
            }
        }
    }

    return dwNeeded;
}

MOS_STATUS MHW_BLOCK_MANAGER::SubmitBlock(
    PMHW_STATE_HEAP_MEMORY_BLOCK pBlock,
    const FrameTrackerTokenFlat  *trackerToken)
{
    MOS_STATUS                   eStatus = MOS_STATUS_SUCCESS;

    BLOCK_MANAGER_CHK_NULL(pBlock);

    if (pBlock->BlockState == MHW_BLOCK_STATE_POOL    ||        // Block cannot be submitted directly from pool
        pBlock->BlockState == MHW_BLOCK_STATE_FREE    ||        // or from free (must be allocated first)
        pBlock->BlockState == MHW_BLOCK_STATE_DELETED)          // or from deleted (the block is being DELETED for crying out loud!)
    {
        MHW_ASSERTMESSAGE("ERROR: SubmitBlock: Block in invalid state, cannot be enqueued");
        return MOS_STATUS_UNKNOWN;
    }

    // Detach block from whatever list it's in - could be in SUBMITTED state (block is being reused - i.e., kernel or scratch space)
    pBlock = DetachBlock(pBlock->BlockState, pBlock);
    BLOCK_MANAGER_CHK_NULL(pBlock);

    // Set block sync tag - used for refreshing the block status
    FrameTrackerTokenFlat_Merge(&pBlock->trackerToken, trackerToken);

    BLOCK_MANAGER_CHK_STATUS(AttachBlock(MHW_BLOCK_STATE_SUBMITTED, pBlock, MHW_BLOCK_POSITION_TAIL));

    return eStatus;
}

PMHW_STATE_HEAP_MEMORY_BLOCK MHW_BLOCK_MANAGER::AllocateMultiple(
    uint32_t            *pdwSizes,
    int32_t             iCount,
    uint32_t            dwAlignment,
    bool                bHeapAffinity,
    PMHW_STATE_HEAP     pHeapAffinity)
{
    PMHW_STATE_HEAP                pStateHeap;
    uint32_t                       dwTotalSize = 0;
    uint8_t                        SortedIndex[MHW_BLOCK_MANAGER_MAX_BLOCK_ARRAY];  // uint8_t is only used because the array size is <256
    PMHW_STATE_HEAP_MEMORY_BLOCK   pBlockArray[MHW_BLOCK_MANAGER_MAX_BLOCK_ARRAY];  // This could be allocated dynamically, same for the above
    PMHW_STATE_HEAP_MEMORY_BLOCK   pBlock;

    // Clear the result
    pBlockArray[0] = nullptr;

    if (iCount <= 0 || iCount > MHW_BLOCK_MANAGER_MAX_BLOCK_ARRAY)
    {
        return nullptr;
    }

    // Get total allocation size
    for (int i = 0; i < iCount; i++)
    {
        dwTotalSize += pdwSizes[i];
    }

    Mhw_BlockManager_ReverseMergeSort_With_Index(pdwSizes, iCount, SortedIndex);

    if (bHeapAffinity)
    {
        if (pHeapAffinity)
        {
            // Set heap affinity - all blocks will be allocated from this one particular heap
            pStateHeap = pHeapAffinity;
        }
        else
        {
            // Heap affinity is not set - start from the current active heap (most recently allocated heap)
            pStateHeap = m_pStateHeap;
        }
    }
    else
    {   // Don't care about heap affinity, blocks can be spread across all available state heaps
        // Just calculate total space available to make sure we have it
        uint32_t dwTotalHeapSpace = 0;
        for (pStateHeap = m_pStateHeap; pStateHeap != nullptr; pStateHeap = pStateHeap->pNext)
        {
            // Calculate total free space in all available heaps
            // NOTE: Heap being deleted as free space set to 0
            //       and all Free blocks are now Deleted (in deleted queue)
            dwTotalHeapSpace += pStateHeap->dwFree;
        }

        // Not enough space
        if (dwTotalHeapSpace < dwTotalSize)
        {
            return pBlockArray[0];
        }
    }

    // Loop to try loading all kernels into the same heap
    do {
        if ( (!bHeapAffinity) ||                   // no affinity set -> blocks can be spread across multiple heaps
              pStateHeap->dwFree >= dwTotalSize)   // this heap has enough free space
        {
            int32_t i, j;

            // Allocate array according to size
            for (i = 0, pBlock = nullptr; i < iCount; i++)
            {   // NOTE - don't care about scratch space, already checked
                pBlock = pBlockArray[SortedIndex[i]] = AllocateBlock(pdwSizes[SortedIndex[i]], dwAlignment, pStateHeap);
                if (!pBlock)
                {
                    break;
                }
            }

            // Allocations all sucessfull, quit search
            if (pBlock)
            {
                break;
            }

            // One of the block allocations failed - free all blocks already allocated (try another heap)
            for (j = 0; j < i; j++)
            {
                FreeBlock(pBlockArray[SortedIndex[j]]);
            }

            // Clear the output
            pBlockArray[0] = nullptr;
        }

        // Try another heap if bHeapAffinity is set
        if (bHeapAffinity)
        {
            pStateHeap = (pHeapAffinity) ? nullptr : pStateHeap->pNext;
        }
    } while (pStateHeap);

    // Allocation successful, reorder blocks according to original request
    if (pBlockArray[0])
    {
        for (int32_t i = 0; i < iCount; i++)
        {
            pBlock = DetachBlock(MHW_BLOCK_STATE_ALLOCATED, pBlockArray[i]);
            AttachBlock(MHW_BLOCK_STATE_ALLOCATED, pBlock, MHW_BLOCK_POSITION_TAIL);
        }
    }

    return pBlockArray[0];
}
