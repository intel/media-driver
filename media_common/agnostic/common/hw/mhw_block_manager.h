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
//! \file      mhw_block_manager.h 
//! \brief         This modules implements state heap block manager used in MHW 
//!
#ifndef __MHW_BLOCK_MANAGER_H__
#define __MHW_BLOCK_MANAGER_H__

#include "mos_os.h"
#include "mhw_state_heap.h"
#include "mhw_memory_pool.h"

#define BLOCK_MANAGER_CHK_STATUS(_stmt)                                               \
    MOS_CHK_STATUS_RETURN(MOS_COMPONENT_HW, MOS_HW_SUBCOMP_ALL, _stmt)

#define BLOCK_MANAGER_CHK_STATUS_MESSAGE(_stmt, _message, ...)                        \
    MOS_CHK_STATUS_MESSAGE_RETURN(MOS_COMPONENT_HW, MOS_HW_SUBCOMP_ALL, _stmt, _message, ##__VA_ARGS__)

#define BLOCK_MANAGER_CHK_NULL(_ptr)                                                  \
    MOS_CHK_NULL_RETURN(MOS_COMPONENT_HW, MOS_HW_SUBCOMP_ALL, _ptr)

#define MHW_BLOCK_MANAGER_INVALID_TAG ((uint32_t)-1)

// Maximum allocation array size
#define MHW_BLOCK_MANAGER_MAX_BLOCK_ARRAY  64

typedef struct _MHW_BLOCK_MANAGER_PARAMS
{
    uint32_t                     dwPoolInitialCount;     //!< Initial number of memory blocks in pool
    uint32_t                     dwPoolMaxCount;         //!< Maximum number of memory blocks in pool
    uint32_t                     dwPoolIncrement;        //!< Memory block pool increment
    uint32_t                     dwHeapInitialSize;      //!< Initial heap size
    uint32_t                     dwHeapIncrement;        //!< Heap size increment
    uint32_t                     dwHeapMaxSize;          //!< Maximum overall heap size
    uint32_t                     dwHeapMaxCount;         //!< Maximum number of heaps
    uint32_t                     dwHeapGranularity;      //!< Block granularity
    uint32_t                     dwHeapBlockMinSize;     //!< Minimum block fragment size (never create a block smaller than this)
} MHW_BLOCK_MANAGER_PARAMS, *PMHW_BLOCK_MANAGER_PARAMS;

#define MHW_BLOCK_POSITION_TAIL (NULL)
#define MHW_BLOCK_POSITION_HEAD ((PMHW_STATE_HEAP_MEMORY_BLOCK) -1)

typedef struct _MHW_BLOCK_LIST
{
    PMHW_BLOCK_MANAGER              pBlockManager;      //!< Block Manager the list belongs to
    PMHW_STATE_HEAP_MEMORY_BLOCK    pHead;              //!< Head of the list
    PMHW_STATE_HEAP_MEMORY_BLOCK    pTail;              //!< Tail of the list
    MHW_BLOCK_STATE                 BlockState;         //!< Described the type of block the list contains
    int32_t                         iCount;             //!< Number of elements on the list
    uint32_t                        dwSize;             //!< Total memory in the list
    char                            szListName[16];     //!< List name for debugging purposes
} MHW_BLOCK_LIST, *PMHW_BLOCK_LIST;

struct MHW_BLOCK_MANAGER
{
private:
    MHW_BLOCK_MANAGER_PARAMS m_Params;                              //!< Block Manager configuration
    MHW_MEMORY_POOL          m_MemoryPool;                          //!< Memory pool of PMHW_STATE_HEAP_MEMORY_BLOCK objects
    MHW_BLOCK_LIST           m_BlockList[MHW_BLOCK_STATE_COUNT];    //!< Block lists associated with each block state
    PMHW_STATE_HEAP          m_pStateHeap;                          //!< Points to state heap

public:

    //!
    //! \brief    Initializes the MHW_BLOCK_MANAGER
    //! \details  Constructor of MHW_BLOCK_MANAGER which initializes all parameters and members
    //! \param    [in] pParams
    //!           Pointer to block manager params. If it is null, default param will be used.
    //!
    MHW_BLOCK_MANAGER(PMHW_BLOCK_MANAGER_PARAMS pParams);

    //!
    //! \brief    Destructor of MHW_BLOCK_MANAGER
    ~MHW_BLOCK_MANAGER();

    //!
    //! \brief    Adds newly created state heap to block manager
    //! \details  Adds a new state heap to memory block manager. A new memory free memory block is added to
    //!           the free list representing the new available heap.
    //! \param    [in] pStateHeap
    //!           Pointer to the newly created state heap
    //! \return   MOS_STATUS
    //!           Returns the status of the operation
    //!
    MOS_STATUS RegisterStateHeap(PMHW_STATE_HEAP    pStateHeap);

    //!
    //! \brief    Unregisters state heap from block manager prior to deallocation
    //! \details  Removes state heap from block manager. Memory blocks must be all deleted.
    //! \param    [in] pStateHeap
    //!           Pointer to the newly created state heap
    //! \return   MOS_STATUS
    //!           Returns the status of the operation
    //!
    MOS_STATUS UnregisterStateHeap(PMHW_STATE_HEAP    pStateHeap);

    //!
    //! \brief    Update block states based on last executed tag
    //! \details  Update block states based on last executed tag
    //!           submitted unlocked blocks are released;
    //!           move to allocated
    //! \param    [in] dwSyncTag
    //!           sync tag
    //! \return   MOS_STATUS
    //!           Returns the status of the operation
    //!
    MOS_STATUS Refresh();

    //!
    //! \brief    Allocate memory block with scratch space
    //! \details  Allocate memory block with scratch space
    //! \param    [in] dwSize
    //!           Size of memory block to allocate 
    //! \param    [in] dwAlignment
    //!           Alignment
    //! \param    [in] dwScratchSpace
    //!           Scratch space size
    //! \return   PMHW_STATE_HEAP_MEMORY_BLOCK
    //!           Pointer to the allocated memory block
    //!
    PMHW_STATE_HEAP_MEMORY_BLOCK AllocateWithScratchSpace(
        uint32_t            dwSize,
        uint32_t            dwAlignment,
        uint32_t            dwScratchSpace);

    //!
    //! \brief    Allocate free block of memory
    //! \details  Allocate free memory block of memory for use by the client
    //! \param    [in] dwSize
    //!           Size of memory to be allocated
    //! \param    [in] dwAlignment
    //!           Memory alignment
    //! \param    [in] pHeapAffinity
    //!           Pointer to heap affinity
    //! \return   PMHW_STATE_HEAP_MEMORY_BLOCK
    //!           Pointer to allocated memory block
    //!
    PMHW_STATE_HEAP_MEMORY_BLOCK AllocateBlock(
        uint32_t            dwSize,
        uint32_t            dwAlignment,
        PMHW_STATE_HEAP     pHeapAffinity);

    //!
    //! \brief    Free memory block
    //! \details  Free memory block according to the sync tag
    //! \param    [in] pBlock
    //!           Pointer to memory block to be freed
    //! \param    [in] dwSyncTag
    //!           Sync tag
    //! \return   MOS_STATUS
    //!           Returns the status of the operation
    //!
    MOS_STATUS FreeBlock(
        PMHW_STATE_HEAP_MEMORY_BLOCK pBlock);

    //!
    //! \brief    Calculate the memory of space required
    //! \details  Calculate the memory of space required
    //! \param    [in] pdwSizes
    //!           Pointer to memory block to be freed
    //! \param    [in] iCount
    //!           Sync tag
    //! \param    [in] dwAlignment
    //!           alignment
    //! \param    [in] bHeapAffinity 
    //!           heap affinity
    //! \param    [in] pHeapAffinity 
    //!           pointer to state heap affinity
    //! \return   uint32_t Size of space needed by client
    //!
    uint32_t CalculateSpaceNeeded(
        const uint32_t      *pdwSizes,
        int32_t             iCount,
        uint32_t            dwAlignment,
        bool                bHeapAffinity,
        PMHW_STATE_HEAP     pHeapAffinity);

    //!
    //! \brief    Submit memory block
    //! \details  Submit memory block according to the sync tag
    //! \param    [in] pBlock
    //!           Pointer to memory block to be submitted
    //! \param    [in] dwSyncTag
    //!           Sync tag
    //! \return   MOS_STATUS
    //!           Returns the status of the operation
    //!
    MOS_STATUS SubmitBlock(
        PMHW_STATE_HEAP_MEMORY_BLOCK pBlock,
        const FrameTrackerTokenFlat  *trackerToken);

    //!
    //! \brief    Set state heap to block manager
    //! \details  Set state heap to block manager
    //! \param    [in] pStateHeap
    //!           Pointer to state heap
    //!
    void SetStateHeap(PMHW_STATE_HEAP pStateHeap);

    // Multiple Block Allocation algorithm description (multiple kernel load):
    //
    // Multiple blocks must be efficiently allocated in multiple heaps or in a single heap.
    //
    // Each load may introduce block fragmentation. The more kernels are loaded, the greater
    // are the chances to fragment the heap, reducing the chances of finding contiguous for a
    // larger kernel. Therefore, the efficient way to use the heap is to load larger kernels
    // first ensuring they have enough contiguous space, and then load the smaller kernels,
    // which are more likely to fit in remaining blocks.
    //
    // So the algorithm is the following:
    //
    // 1) The total size of all blocks is calculated to check if there's even a fighting chance
    //    to load them all - if the amount available is insufficient, other measures will be taken.
    //
    // 2) In order to allocate blocks according to size, we first sort the array of sizes
    //    using an index array (never touching the source array!) - this is done by a merge sort
    //    implementation, which is O(n*log(n)) - may try using other algorithm such as QuickSort -
    //    although quicksort is not always the best (may be O(n^2) in same cases)
    //
    // 3) Select a specific heap (or nullptr if bHeapAffinity is false). Check if there is enough space
    //    to load all blocks - ignoring the fragmentation for now. Only by traversing the list of
    //    blocks we will be able to determine if the heap can be used or not. Trying to load larger
    //    kernels FIRST helps identifying heap fragmentation issues faster than leaving it for last.
    //
    // 4) Load the array of kernels one at a time according to size (larger first). Select a specific
    //    heap or nullptr (if any heap, don't care).
    //
    // 5) If all succeeded, then we are ready to return the list of blocks to the client.
    //
    // 6) If it fails, release the already allocated blocks. the algorithm does check for space in the
    //    current heap before even starting the loop. The failure will not happen for lack of space,
    //    but because of fragmentation of free space. In case of bHeapAffinity = false, we also check
    //    the aggregate free space.
    //
    // 7) If the heap affinity is selected and the intended heap is also provided, we're done - if we
    //    were not able to load the blocks, the overall operation has failed. If affinity is selected but
    //    the heap was not provided, we try the next heap until no more heaps are available. This particular
    //    situation is intended for loading all blocks in the same heap without specifying which one.
    //

    //!
    //! \brief    Allocate multiple blocks 
    //! \details  Allocate multiple blocks 
    //! \param    [in]  pdwSizes
    //!           Pointer to sizes
    //! \param    [in]  iCount
    //!           Count of blocks to be allocated
    //! \param    [in] dwAlignment
    //!           Alignment
    //! \param    [in] bHeapAffinity
    //!           true if all blocks must be allocated in the same heap 
    //! \param    [in] pHeapAffinity
    //!           Pointer to heap affinity 
    //! \return   PMHW_STATE_HEAP_MEMORY_BLOCK
    //!           Pointer to allocated memory block
    //!
    PMHW_STATE_HEAP_MEMORY_BLOCK AllocateMultiple(
        uint32_t            *pdwSizes,
        int32_t             iCount,
        uint32_t            dwAlignment,
        bool                bHeapAffinity,
        PMHW_STATE_HEAP     pHeapAffinity);

private:

    //!
    //! \brief    Gets memory block from pool
    //! \details  Gets memory block from pool, extending the pool if necessary
    //! \return   PMHW_STATE_HEAP_MEMORY_BLOCK
    //!           Returns a pointer to the memory block, nullptr if failed to allocate or
    //!           all blocks are in use and the pool reached its maximum size.
    //!
    PMHW_STATE_HEAP_MEMORY_BLOCK GetBlockFromPool();

    //!
    //! \brief    Extends pool of MHW_STATE_HEAP_MEMORY_BLOCK objects
    //! \details  Allocates an array of MHW_STATE_HEAP_MEMORY_BLOCK objects and appends to the pool of objects for reuse.
    //!           The allocation of memory block structures is limited to the maximum number defined when creating
    //!           the Memory Block Manager object. The increase in number of memory blocks may be due to increased
    //!           block fragmentation, system load or other factors (increase in queue depth, increas in memory
    //!           blocks maintained by the client - such as number of kernels loaded).
    //! \param    [in] dwCount
    //!           Number of additional blocks to add to the pool
    //! \return   N/A
    //!
    void ExtendPool(uint32_t dwCount);

    //!
    //! \brief    Attach block into appropriate memory block list
    //! \details  Sets block state and inserts it into the memory block list associated with the new state.
    //!           The block may be inserted at the "Head" or "Tail" of the list, or after another block from the same list.
    //! \param    [in]BlockState
    //!           New block state, defines which list the block is to be inserted.
    //!           NOTE: The block must not belong to any other list before calling this function (pPrev/pNext = nullptr).
    //! \param    [in] pBlock
    //!           Pointer to memory block
    //! \param    [in] pBlockPos
    //!           Pointer to memory block after which the block is to be inserted (inserted AFTER pBlockPos)
    //!           If set to MHW_BLOCK_POSITION_HEAD, inserts block at the head of the list.
    //!           If set to MHW_BLOCK_POSITION_TAIL, inserts block at the tail of the list.
    //! \return   MOS_STATUS
    //!           Returns error code
    //!
    MOS_STATUS AttachBlock(
        MHW_BLOCK_STATE              BlockState,
        PMHW_STATE_HEAP_MEMORY_BLOCK pBlock,
        PMHW_STATE_HEAP_MEMORY_BLOCK pBlockPos);

    //!
    //! \brief    INTERNAL function to insert a block into a block list.
    //! \details  Insert a block into a block list at a given position (head, tail, insert after block)
    //!           Ensures that the list and the block position are both valid.
    //!           Does not check if the block is still attached to another list.
    //!           IMPORTANT: Block must be detached, at the risk of corrupting other block lists.
    //!                      This function does not track state heap usage
    //! \param    [in] pList
    //!           Pointer to memory block list structure
    //! \param    [in] BlockState
    //!           New block state, defines the state of the new block,
    //!           must match the state associated with the list and reference block
    //! \param    [in] pBlock
    //!           Pointer to memory block to be inserted
    //! \param    [in] pBlockPos
    //!           Pointer to memory block after which the block is to be inserted (inserted AFTER pBlockPos)
    //!           If set to MHW_BLOCK_POSITION_HEAD, inserts block at the head of the list.
    //!           If set to MHW_BLOCK_POSITION_TAIL, inserts block at the tail of the list.
    //! \return   MOS_STATUS
    //!           Returns error code
    //!
    MOS_STATUS AttachBlockInternal(
        PMHW_BLOCK_LIST              pList,
        MHW_BLOCK_STATE              BlockState,
        PMHW_STATE_HEAP_MEMORY_BLOCK pBlock,
        PMHW_STATE_HEAP_MEMORY_BLOCK pBlockPos);

    //!
    //! \brief    Removes a block from a memory block list
    //! \details  Removes a block from a memory block list from a given 
    //! \param    [in] BlockState
    //!           Block state defines from which list the block is to be removed.
    //! \param    [in] pBlockPos
    //!           Defines the object in list to be detached.
    //!           If set to MHW_BLOCK_POSITION_HEAD, detaches the first block in the list.
    //!           If set to MHW_BLOCK_POSITION_TAIL, detaches the last block in the list.
    //! \return   PMHW_STATE_HEAP_MEMORY_BLOCK
    //!           Returns a pointer to the block detached, nullptr if the list is empty or invalid.
    //!
    PMHW_STATE_HEAP_MEMORY_BLOCK DetachBlock(
        MHW_BLOCK_STATE              BlockState,
        PMHW_STATE_HEAP_MEMORY_BLOCK pBlockPos);

    //!
    //! \brief    INTERNAL function to remove a block from a block list.
    //! \details  Removes a block from a list provided. The caller may specify the block
    //!           to remove, or request the head or tail of the list.
    //!           Ensures that the list and the block position are both valid.
    //!           Does not check if the block is still attached to another list.
    //!           IMPORTANT: Block must be detached, at the risk of corrupting other block lists.
    //!                      This function does not track state heap usage
    //! \param    [in] pList
    //!           Pointer to memory block list structure
    //! \param    [in] pBlock
    //!           Pointer to memory block to be inserted
    //! \return   PMHW_STATE_HEAP_MEMORY_BLOCK
    //!           Returns block, nullptr if failure or no block available
    //!
    PMHW_STATE_HEAP_MEMORY_BLOCK DetachBlockInternal(
        PMHW_BLOCK_LIST              pList,
        PMHW_STATE_HEAP_MEMORY_BLOCK pBlock);

    //!
    //! \brief    Interface to move block from one list to another
    //! \details  Moves a block from a list into another
    //!           Ensures that the list and the block position are both valid.
    //!           Does not check if the block is still attached to another list.
    //!           IMPORTANT: Block must be detached, at the risk of corrupting other block lists.
    //!                      This function does not track state heap usage
    //! \param    [in] pSrcList
    //!           Pointer to source list
    //! \param    [in] pDstList
    //!           Pointer to destination list
    //! \param    [in] pBlock
    //!           Pointer to memory block to be moved
    //! \param    [in] pBlockPos
    //!           Pointer to memory block after which the block is to be inserted (inserted AFTER pBlockPos)
    //!           If set to MHW_BLOCK_POSITION_HEAD, inserts block at the head of the list.
    //!           If set to MHW_BLOCK_POSITION_TAIL, inserts block at the tail of the list.
    //! \return   MOS_STATUS
    //!           Returns error code
    //!
    MOS_STATUS MoveBlock(
        PMHW_BLOCK_LIST              pSrcList,      // Source list
        PMHW_BLOCK_LIST              pDstList,      // Destination list
        PMHW_STATE_HEAP_MEMORY_BLOCK pBlock,        // Block to be moved (or HEAD/TAIL of source list)
        PMHW_STATE_HEAP_MEMORY_BLOCK pBlockPos);    // Position to insert (or HEAD/TAIL of target list)

    //!
    //! \brief    Returns memory block object to pool
    //! \details  Returns memory block object to pool
    //! \param    [in] pBlock
    //!           Pointer to memory block to be returned back to pool
    //! \return   N/A
    //!
    void ReturnBlockToPool(PMHW_STATE_HEAP_MEMORY_BLOCK pBlock);

    //!
    //! \brief    Consolidate free memory
    //! \details  Consolidate free memory blocks adjacent to a given free block (within the same state heap).
    //! \param    [in] pBlock
    //!           Pointer to free memory block
    //! \return    N/A
    //!
    void ConsolidateBlock(PMHW_STATE_HEAP_MEMORY_BLOCK pBlock);

    //!
    //! \brief    INTERNAL: Move block from free to allocated list
    //! \details  Move block from free to allocated list, setting up pointer/offset to data
    //! \param    [in] pBlock
    //!           Pointer to free memory block
    //! \param    uint32_t dwAlignment
    //!           [in] Data alignment, assured to be power of 2 by caller
    //! \return   MOS_STATUS
    //!           Returns the status of the operation
    //!
    MOS_STATUS AllocateBlockInternal(
        PMHW_STATE_HEAP_MEMORY_BLOCK pBlock,
        uint32_t                     dwAlignment);

    //!
    //! \brief    Split block internal
    //! \details  Split block internal
    //! \param    [in] pBlock
    //!           Pointer to block to be splitted
    //! \param    [in] dwSplitSize
    //!           Size of the block to be splitted 
    //! \param    [in] dwAlignment
    //!           Data alignment, assured to be power of 2 by caller
    //! \param    [in] bBackward
    //!           if true, use higher part of the block
    //! \return   MOS_STATUS
    //!           Returns the status of the operation
    //!
    MOS_STATUS SplitBlockInternal(
        PMHW_STATE_HEAP_MEMORY_BLOCK    pBlock,
        uint32_t                        dwSplitSize,
        uint32_t                        dwAlignment,
        bool                            bBackward);

    //!
    //! \brief    Merge blocks
    //! \details  Merge blocks
    //! \param    [in]  pBlockL
    //!           Pointer to block in lower memory
    //! \param    [in]  pBlockH
    //!           Pointer to block in higher memory
    //! \param    [in] dwAlignment
    //!           Data alignment, assured to be power of 2 by caller
    //! \param    [in] bBackward
    //!           if True, pBlcokL is merged into pBlockH
    //!           if False, pBlockH is merged into pBlockL
    //! \return   MOS_STATUS
    //!           Returns the status of the operation
    //!
    MOS_STATUS MergeBlocksInternal(
        PMHW_STATE_HEAP_MEMORY_BLOCK    pBlockL,
        PMHW_STATE_HEAP_MEMORY_BLOCK    pBlockH,
        uint32_t                        dwAlignment,
        bool                            bBackward);

    //!
    //! \brief    ResizeBlock
    //! \details  ResizeBlock
    //! \param    [in] pBlock
    //!           Pointer to block to be resized
    //! \param    [in]   dwNewSize
    //!           new size of block
    //! \param    [in] dwAlignment
    //!           Data alignment, assured to be power of 2 by caller
    //! \param    [in] bBackward
    //!           if True,  allow block to grow forward/backwards (moving its start offset)
    //!           if False, Always grow/shrink forward;
    //! \return   MOS_STATUS
    //!           Returns the status of the operation
    //!
    MOS_STATUS ResizeBlock(
        PMHW_STATE_HEAP_MEMORY_BLOCK    pBlock,
        uint32_t                        dwNewSize,
        uint32_t                        dwAlignment,
        bool                            bBackward);
};

#endif // __MHW_BLOCK_MANAGER_H__
