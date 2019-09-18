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
//! \file      renderhal_dsh.cpp 
//! \brief         This module implements render engine state heap management functions based     on dynamic state heap (DSH) infrastructure, rather than static state heap.     It allows dynamic allocation and expansion of general state heap (media states)     and instraction state heaps (media kernels) based on demand. It also allows     dynamic configuration of media states based on specific workload requirements,     as well as dynamic allocation of kernel scratch space (spill area). 
//!
#include "renderhal.h"
#include "renderhal_platform_interface.h"

// Defined in renderhal.c
extern const RENDERHAL_SURFACE_STATE_ENTRY g_cInitSurfaceStateEntry;

MOS_STATUS RenderHal_AllocateDebugSurface(
    PRENDERHAL_INTERFACE     pRenderHal);

//!
//! \brief    Extend the media state pool
//! \details  Extends the media state pool by adding 16 media state objects
//! \param    PRENDERHAL_STATE_HEAP pStateHeap
//!           [in] Pointer to the state heap
//! \return   MOS_STATUS
//!           SUCCESS    if media state pool was successfully extended
//!           NO_SPACE   if the pool of objects cannot be extended
//!
MOS_STATUS RenderHal_DSH_ExtendMediaStatePool(
    PRENDERHAL_STATE_HEAP pStateHeap)
{
    int32_t iSize;
    uint8_t *pPtr;
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    //---------------------------------------
    MHW_RENDERHAL_CHK_NULL_RETURN(pStateHeap);
    MHW_RENDERHAL_CHK_NULL_RETURN(pStateHeap->pMediaStatesMemPool);
    //---------------------------------------

    int32_t iCount = RENDERHAL_DSH_DYN_STATE_INC;

    // Extend pool and get pointer to first element of the new pool object
    PRENDERHAL_MEDIA_STATE      pFirst, pLast, pPrev;
    PRENDERHAL_MEDIA_STATE_LIST pList = &pStateHeap->FreeStates;

    uint32_t ID = pStateHeap->pMediaStatesMemPool->m_dwObjCount;
    pLast = pFirst = (PRENDERHAL_MEDIA_STATE)(pStateHeap->pMediaStatesMemPool->Allocate(iCount));
    if (!pLast)
    {
        eStatus = MOS_STATUS_NO_SPACE;
        goto finish;
    }

    // Increment size of the list
    pList->iCount += iCount;

    // Initialize new doubled linked list and internal structure
    iSize = pStateHeap->pMediaStatesMemPool->m_dwObjSize; // enough for media state and dynamic state objects
    pPtr  = (uint8_t*) pFirst;
    for (pPrev = nullptr; iCount > 0; iCount--, pPtr += iSize, pPrev = pLast)
    {
        // Create doubled linked list
        pLast = (PRENDERHAL_MEDIA_STATE) pPtr;
        pLast->Reserved = ID++;
        pLast->pPrev    = pPrev;
        if (pPrev) pPrev->pNext = pLast;

        // Point to dynamic states (co-located)
        pLast->pDynamicState = (PRENDERHAL_DYNAMIC_STATE) (pLast + 1);
    }
    pLast->pNext = nullptr;

    // Attach to end of existing pool
    pFirst->pPrev = pList->pTail;
    pList->pTail  = pLast;
    if (pFirst->pPrev)
        pFirst->pPrev->pNext = pFirst;
    else
        pList->pHead = pFirst;

finish:
    return eStatus;
}

//!
//! \brief    Get a media state from the pool
//! \details  Returns a pointer to a media state from the pool
//!           If pool is empty, the pool is extended (adds 16 media states)
//! \param    PRENDERHAL_STATE_HEAP pStateHeap
//!           [in] Pointer to the state heap
//! \return   PRENDERHAL_MEDIA_STATE pointer to a media state
//!
PRENDERHAL_MEDIA_STATE RenderHal_DSH_GetMediaStateFromPool(
    PRENDERHAL_STATE_HEAP pStateHeap)
{
    MOS_STATUS             eStatus;
    PRENDERHAL_MEDIA_STATE pMediaState = nullptr;

    //---------------------------------------
    if (pStateHeap == nullptr)
    {
        MHW_RENDERHAL_ASSERTMESSAGE("Null pointer detected!");
        return nullptr;
    }
    //---------------------------------------

    PRENDERHAL_MEDIA_STATE_LIST pList = &pStateHeap->FreeStates;

    // If pool is empty, extend pool of free media states
    if (pList->iCount == 0)
    {
        MHW_RENDERHAL_CHK_STATUS(RenderHal_DSH_ExtendMediaStatePool(pStateHeap));
    }

    // Get Element from the head of the list
    pMediaState = pList->pHead;

    // Detach element from the list
    if (pMediaState)
    {
        pList->iCount--;
        pList->pHead = pMediaState->pNext;
        if (pMediaState->pNext)
        {
            pMediaState->pNext->pPrev = nullptr;
        }
        else
        {   // List is now empty - last element was removed
            pList->pTail = nullptr;
            MHW_ASSERT(pList->iCount == 0);
        }

        pMediaState->pNext = pMediaState->pPrev = nullptr;
    }

finish:
    return pMediaState;
}

//!
//! \brief    Returns a media state to the pool
//! \details  Returns the media state to the pool list
//! \param    PRENDERHAL_STATE_HEAP pStateHeap
//!           [in] Pointer to the state heap
//! \param    PRENDERHAL_MEDIA_STATE  pMediaState
//!           [in] Pointer to the media state
//! \return   NONE
//!
void RenderHal_DSH_ReturnMediaStateToPool(
    PRENDERHAL_STATE_HEAP   pStateHeap,
    PRENDERHAL_MEDIA_STATE  pMediaState)
{
    //---------------------------------------
    MHW_RENDERHAL_CHK_NULL_NO_STATUS_RETURN(pStateHeap);
    MHW_RENDERHAL_CHK_NULL_NO_STATUS_RETURN(pMediaState);
    //---------------------------------------

    PRENDERHAL_MEDIA_STATE_LIST pList = &pStateHeap->FreeStates;

    // Attach element to the head of the list
    pMediaState->pPrev = pList->pTail;
    pList->pTail = pMediaState;
    if (pMediaState->pPrev)
    {
        pMediaState->pPrev->pNext = pMediaState;
    }
    else
    {   // List was empty - insert first element
        MHW_ASSERT(pList->iCount == 0);
        pList->pHead = pMediaState;
    }
    pList->iCount++;

    return;
}

//!
//! \brief    Extend the kernel allocation pool
//! \details  Extends the kernel allocation pool by adding 16 kernel allocations
//! \param    PRENDERHAL_STATE_HEAP pStateHeap
//!           [in] Pointer to the state heap
//! \return   MOS_STATUS
//!           SUCCESS    if kernel allocation pool was successfully extended
//!           NO_SPACE   if the pool of objects cannot be extended
//!
MOS_STATUS RenderHal_DSH_ExtendKernelAllocPool(
    PRENDERHAL_STATE_HEAP pStateHeap)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    //---------------------------------------
    MHW_RENDERHAL_CHK_NULL_RETURN(pStateHeap);
    MHW_RENDERHAL_CHK_NULL_RETURN(pStateHeap->pKernelAllocMemPool);
    //---------------------------------------

    int32_t iCount = RENDERHAL_DSH_KRN_ALLOC_INC;

    // Extend pool and get pointer to first element of the new pool object
    PRENDERHAL_KRN_ALLOCATION   pFirst, pLast;
    PRENDERHAL_KRN_ALLOC_LIST   pList = &pStateHeap->KernelAllocationPool;

    uint32_t ObjID = pStateHeap->pKernelAllocMemPool->m_dwObjCount;
    pLast = pFirst = (PRENDERHAL_KRN_ALLOCATION)(pStateHeap->pKernelAllocMemPool->Allocate(iCount));

    if (!pLast)
    {
        eStatus = MOS_STATUS_NO_SPACE;
        goto finish;
    }

    // Increment size of the list
    pList->iCount += iCount;

    // Initialize new doubled linked list
    for ( ; iCount > 0; iCount--, pLast++)
    {
        pLast->Reserved = ObjID++;
        pLast->pPrev = pLast - 1;
        pLast->pNext = pLast + 1;
        pLast->pList = pList;
        pLast->szKernelName = nullptr;
    }
    pLast--;
    pLast->pNext = nullptr;

    // Attach to end of existing pool
    pFirst->pPrev = pList->pTail;
    pList->pTail  = pLast;
    if (pFirst->pPrev)
    {
        pFirst->pPrev->pNext = pFirst;
    }
    else
    {
        pList->pHead = pFirst;
    }

finish:
    return eStatus;
}

//!
//! \brief    Detach kernel allocation
//! \details  Removes the kernel allocation from the current list
//! \param    PRENDERHAL_KRN_ALLOCATION pKernel
//!           [in] Pointer to the kernel allocation
//! \return   NONE
//!
void RenderHal_DSH_KernelDetach(PRENDERHAL_KRN_ALLOCATION pKernel)
{
    PRENDERHAL_KRN_ALLOC_LIST pList;

    if (!pKernel || !pKernel->pList) return;
    pList = pKernel->pList;

    // Kernel is last in its list
    if (pKernel->pNext == nullptr)
    {
        pList->pTail = pKernel->pPrev;
    }
    else
    {
        pKernel->pNext->pPrev = pKernel->pPrev;
    }

    // Kernel is first in its list
    if (pKernel->pPrev == nullptr)
    {
        pList->pHead = pKernel->pNext;
    }
    else
    {
        pKernel->pPrev->pNext = pKernel->pNext;
    }

    pKernel->pNext = nullptr;
    pKernel->pPrev = nullptr;
    pKernel->pList = nullptr;
    pList->iCount--;
}

//!
//! \brief    Removes kernel allocation from current list and attaches to designated list
//! \details  Removes the kernel allocation from the current list (pKernel->pList)
//|           and attaches kernel to pList (passed in as a parameter)
//! \param    PRENDERHAL_KRN_ALLOC_LIST pList
//!           [in] Pointer to the kernel allocation list to attach to
//! \param    PRENDERHAL_KRN_ALLOCATION pKernel
//!           [in] Pointer to the kernel allocation
//! \param    bool bHead
//!           [in] Indicates whether to attach to head of list or tail
//! \return   NONE
//!
void RenderHal_DSH_KernelAttach(PRENDERHAL_KRN_ALLOC_LIST pList, PRENDERHAL_KRN_ALLOCATION pKernel, bool bHead)
{
    if (!pList || !pKernel) return;

    // Detach from current list
    if (pKernel->pList)
    {
        RenderHal_DSH_KernelDetach(pKernel);
    }

    if (bHead)
    {
        pKernel->pPrev = nullptr;
        pKernel->pNext = pList->pHead;
        pList->pHead   = pKernel;
        if (pKernel->pNext)
        {
            pKernel->pNext->pPrev = pKernel;
        }
        else
        {
            pList->pTail = pKernel;
        }
    }
    else
    {
        pKernel->pPrev = pList->pTail;
        pKernel->pNext = nullptr;
        pList->pTail   = pKernel;
        if (pKernel->pPrev)
        {
            pKernel->pPrev->pNext = pKernel;
        }
        else
        {
            pList->pHead = pKernel;
        }
    }

    pKernel->pList = pList;
    pList->iCount++;
}

//!
//! \brief    Get a kernel allocation from the pool
//! \details  Returns a pointer to the kernel allocation from the pool
//!           If pool is empty, the pool is extended (adds 16 kernel allocations)
//! \param    PRENDERHAL_STATE_HEAP pStateHeap
//!           [in] Pointer to the state heap
//! \return   PRENDERHAL_KRN_ALLOCATION pointer to a kernel allocation
//!
PRENDERHAL_KRN_ALLOCATION RenderHal_DSH_GetKernelAllocationFromPool(PRENDERHAL_STATE_HEAP pStateHeap)
{
    MOS_STATUS                eStatus = MOS_STATUS_SUCCESS;
    PRENDERHAL_KRN_ALLOCATION pKernelAllocation = nullptr;

    MHW_RENDERHAL_CHK_NULL(pStateHeap);

    if (pStateHeap->KernelAllocationPool.iCount == 0)
    {
        MHW_RENDERHAL_CHK_STATUS(RenderHal_DSH_ExtendKernelAllocPool(pStateHeap));
    }

    pKernelAllocation = pStateHeap->KernelAllocationPool.pHead;
    RenderHal_DSH_KernelDetach(pKernelAllocation);

finish:
    return pKernelAllocation;
}

//!
//! \brief    Touch dynamic kernel
//! \details  Updates sync tag on kernel allocation
//!           Moves kernel to the tail of the pStateHeap->KernelsSubmitted list
//!           Submits the dynamic memory block
//!           Updates kernel usage
//! \param    PRENDERHAL_INTERFACE      pRenderHal
//!           [in] Pointer to the renderhal interface
//! \param    PRENDERHAL_KRN_ALLOCATION pKernelAllocation
//!           [in] Pointer to the kernel allocation
//! \return   NONE
//!
void RenderHal_DSH_TouchDynamicKernel(
    PRENDERHAL_INTERFACE      pRenderHal,
    PRENDERHAL_KRN_ALLOCATION pKernelAllocation)
{
    PRENDERHAL_STATE_HEAP      pStateHeap;
    PXMHW_STATE_HEAP_INTERFACE pMhwStateHeap;

    if (pRenderHal == nullptr || pRenderHal->pStateHeap == nullptr || pKernelAllocation == nullptr)
    {
        return;
    }

    pStateHeap    = pRenderHal->pStateHeap;
    pMhwStateHeap = pRenderHal->pMhwStateHeap;

    // Set sync tag, for deallocation control
    pKernelAllocation->dwFlags = RENDERHAL_KERNEL_ALLOCATION_LOCKED;
    FrameTrackerTokenFlat_SetProducer(&pKernelAllocation->trackerToken, &pRenderHal->trackerProducer);
    FrameTrackerTokenFlat_Merge(&pKernelAllocation->trackerToken,
                                pRenderHal->currentTrackerIndex,
                                pRenderHal->trackerProducer.GetNextTracker(pRenderHal->currentTrackerIndex));

    // Move kernel to the tail of the submitted list (list is sorted with the most recently used at the end)
    RenderHal_DSH_KernelAttach(&pStateHeap->KernelsSubmitted, pKernelAllocation, false);

    // Update memory block
    pMhwStateHeap->SubmitDynamicBlockDyn(MHW_ISH_TYPE, pKernelAllocation->pMemoryBlock, &pKernelAllocation->trackerToken);

    // Update kernel usage - this is where kernels age if they are not constantly used, and may be removed
    pKernelAllocation->dwCount++;
}

//!
//! \brief    Searches the hash table for the kernel
//! \details  Searches the hash table for the kernel using the UniqID and CacheID to determine if the
//!           kernel is already allocated in the ISH
//! \param    PRENDERHAL_INTERFACE      pRenderHal
//!           [in] Pointer to the renderhal interface
//! \param    int32_t iUniqID
//!           [in] unique kernel identifier
//! \param    int32_t iCacheID
//!           [in] cache identifier
//! \return   PRENDERHAL_KRN_ALLOCATION
//!           pKernelAllocation    if kernel is found in hash table
//!           nullptr              otherwise
//!
PRENDERHAL_KRN_ALLOCATION RenderHal_DSH_SearchDynamicKernel(PRENDERHAL_INTERFACE pRenderHal, int32_t iUniqID, int32_t iCacheID)
{
    PRENDERHAL_STATE_HEAP      pStateHeap;
    PRENDERHAL_KRN_ALLOCATION  pKernelAllocation = nullptr;

    pStateHeap = (pRenderHal) ? pRenderHal->pStateHeap : nullptr;
    if (pStateHeap)
    {
        uint16_t wSearchIndex = 0;
        pKernelAllocation = (PRENDERHAL_KRN_ALLOCATION) pStateHeap->kernelHashTable.Search(iUniqID, iCacheID, wSearchIndex);
    }

    return pKernelAllocation;
}

//!
//! \brief    Allocates a kernel allocation
//! \details  Searches the hash table for the kernel and if not found
//!           gets a kernel allocation from the pool then
//!           registers it in the hash table and attaches it to the allocated list
//!           returns a pointer to the kernel allocation
//! \param    PRENDERHAL_INTERFACE      pRenderHal
//!           [in] Pointer to the renderhal interface
//! \param    int32_t iUniqID
//!           [in] unique kernel identifier
//! \param    int32_t iCacheID
//!           [in] cache identifier
//! \return   PRENDERHAL_KRN_ALLOCATION
//!
PRENDERHAL_KRN_ALLOCATION RenderHal_DSH_AllocateDynamicKernel(PRENDERHAL_INTERFACE pRenderHal, int32_t iUniqID, int32_t iCacheID)
{
    PRENDERHAL_STATE_HEAP      pStateHeap;
    PRENDERHAL_KRN_ALLOCATION  pKernelAllocation = nullptr;
    uint16_t                   wSearchIndex = 0;
    MOS_STATUS                 eStatus = MOS_STATUS_SUCCESS;

    pStateHeap = (pRenderHal) ? pRenderHal->pStateHeap : nullptr;
    if (pStateHeap == nullptr)
    {
        goto finish;
    }

    pKernelAllocation = (PRENDERHAL_KRN_ALLOCATION)pStateHeap->kernelHashTable.Search(iUniqID, iCacheID, wSearchIndex);

    if (!pKernelAllocation)
    {
        // Get new kernel allocation entry
        pKernelAllocation = RenderHal_DSH_GetKernelAllocationFromPool(pStateHeap);
        MHW_RENDERHAL_CHK_NULL(pKernelAllocation);

        // Clear kernel allocation entry, setup kernel uniq id
        uint32_t Reserved = pKernelAllocation->Reserved; // Preserve before
        MOS_ZeroMemory(pKernelAllocation, sizeof(RENDERHAL_KRN_ALLOCATION));
        pKernelAllocation->dwFlags  = RENDERHAL_KERNEL_ALLOCATION_FREE;    // Krn allocation does not point to a kernel
        pKernelAllocation->Reserved = Reserved;
        pKernelAllocation->iKUID    = iUniqID;
        pKernelAllocation->iKCID    = iCacheID;

        // Register kernel in hash table to speed up future search
        pStateHeap->kernelHashTable.Register(iUniqID, iCacheID, pKernelAllocation);

        // Insert kernel in allocated list
        RenderHal_DSH_KernelAttach(&pStateHeap->KernelsAllocated, pKernelAllocation, false);
    }

finish:
    return pKernelAllocation;
}

//!
//! \brief    Unregister a kernel
//! \details  Frees the memory block of the kernel in the ISH
//!           removes the kernel from the hash table and
//!           attaches the kernel to the allocation pool
//! \param    PRENDERHAL_INTERFACE      pRenderHal
//!           [in] Pointer to the renderhal interface
//! \param    PRENDERHAL_KRN_ALLOCATION pKernelAllocation
//!           [in] Pointer to the kernel allocation
//! \return   MOS_STATUS
//!
MOS_STATUS RenderHal_DSH_UnregisterKernel(PRENDERHAL_INTERFACE pRenderHal, PRENDERHAL_KRN_ALLOCATION pKernelAllocation)
{
    PRENDERHAL_STATE_HEAP      pStateHeap;
    MOS_STATUS                 eStatus = MOS_STATUS_SUCCESS;

    pStateHeap = (pRenderHal) ? pRenderHal->pStateHeap : nullptr;
    if (pStateHeap == nullptr)
    {
        goto finish;
    }

    // Free kernel before releasing
    if (pKernelAllocation->pMemoryBlock)
    {
        // Get new kernel allocation entry
        pRenderHal->pMhwStateHeap->FreeDynamicBlockDyn(MHW_ISH_TYPE, pKernelAllocation->pMemoryBlock);
        pKernelAllocation->pMemoryBlock = nullptr;
    }

    pStateHeap->kernelHashTable.Unregister(pKernelAllocation->iKUID, pKernelAllocation->iKCID);

    // Return the kernel allocation to pool
    RenderHal_DSH_KernelAttach(&pStateHeap->KernelAllocationPool, pKernelAllocation, false);

finish:
    return eStatus;
}

//!
//! \brief    Allocate GSH, SSH, ISH control structures and heaps
//! \details  Allocates State Heap control structure (system memory)
//!           and associated State Buffer in gfx/sys memory (GSH,ISH,SSH)
//!           initializes the State Heap control structure and state buffers
//!
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to Rendering Interface Structure
//! \param    PRENDERHAL_STATE_HEAP_SETTINGS pSettings
//!           [in] Pointer to state heap settings (GSH/SSH/ISH)
//! \return   MOS_STATUS
//!           SUCCESS if state heaps were successfully allocated and initialized
//!           OTHER   if bad parameters or allocation failed
//!
MOS_STATUS RenderHal_DSH_AllocateStateHeaps(
    PRENDERHAL_INTERFACE           pRenderHal,
    PRENDERHAL_STATE_HEAP_SETTINGS pSettings)
{
    MHW_STATE_HEAP_SETTINGS      MhwStateHeapSettings;
    PXMHW_STATE_HEAP_INTERFACE   pMhwStateHeap;
    MhwRenderInterface           *pMhwRender;
    PMHW_RENDER_STATE_SIZES      pHwSizes;
    PRENDERHAL_STATE_HEAP        pStateHeap = nullptr;
    MOS_STATUS                   eStatus;
    int32_t                      iSize;

    // Initialize locals
    eStatus          = MOS_STATUS_UNKNOWN;

    //---------------------------------------
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pOsInterface);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pMhwRenderInterface);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pStateHeap);
    MHW_RENDERHAL_CHK_NULL(pSettings);

    // verify GSH parameters and alignments
    MHW_RENDERHAL_ASSERT((pSettings->iSyncSize        % RENDERHAL_SYNC_BLOCK_ALIGN)   == 0);
    MHW_RENDERHAL_ASSERT((pSettings->iCurbeSize       % RENDERHAL_URB_BLOCK_ALIGN)    == 0);
    MHW_RENDERHAL_ASSERT((pSettings->iKernelHeapSize  % RENDERHAL_KERNEL_BLOCK_ALIGN) == 0);
    MHW_RENDERHAL_ASSERT((pSettings->iKernelBlockSize % RENDERHAL_KERNEL_BLOCK_ALIGN) == 0);
    // verify SSH parameters
    MHW_RENDERHAL_ASSERT(pSettings->iBindingTables >= RENDERHAL_SSH_BINDING_TABLES_MIN);
    MHW_RENDERHAL_ASSERT(pSettings->iBindingTables <= RENDERHAL_SSH_BINDING_TABLES_MAX);
    MHW_RENDERHAL_ASSERT(pSettings->iSurfaceStates >= RENDERHAL_SSH_SURFACE_STATES_MIN);
    MHW_RENDERHAL_ASSERT(pSettings->iSurfaceStates <= RENDERHAL_SSH_SURFACE_STATES_MAX);
    MHW_RENDERHAL_ASSERT(pSettings->iSurfacesPerBT >= RENDERHAL_SSH_SURFACES_PER_BT_MIN);
    MHW_RENDERHAL_ASSERT(pSettings->iSurfacesPerBT <= RENDERHAL_SSH_SURFACES_PER_BT_MAX);
    //---------------------------------------

    pMhwRender  = pRenderHal->pMhwRenderInterface;
    pStateHeap  = pRenderHal->pStateHeap;
    pHwSizes    = pRenderHal->pHwSizes;

    // Reset media state lists
    MOS_ZeroMemory(&pStateHeap->FreeStates           , sizeof(RENDERHAL_MEDIA_STATE_LIST));
    MOS_ZeroMemory(&pStateHeap->ReservedStates       , sizeof(RENDERHAL_MEDIA_STATE_LIST));
    MOS_ZeroMemory(&pStateHeap->SubmittedStates      , sizeof(RENDERHAL_MEDIA_STATE_LIST));
    MOS_ZeroMemory(&pStateHeap->KernelsAllocated     , sizeof(RENDERHAL_KRN_ALLOC_LIST));
    MOS_ZeroMemory(&pStateHeap->KernelsSubmitted     , sizeof(RENDERHAL_KRN_ALLOC_LIST));
    MOS_ZeroMemory(&pStateHeap->KernelAllocationPool , sizeof(RENDERHAL_KRN_ALLOC_LIST));

    // Create pool of media state objects
    iSize = sizeof(RENDERHAL_MEDIA_STATE) + sizeof(RENDERHAL_DYNAMIC_STATE) + 16; // Media state object + Dynamic states object (co-located)

    pStateHeap->pMediaStatesMemPool = MOS_New(MHW_MEMORY_POOL, iSize, sizeof(void *));
    MHW_RENDERHAL_CHK_NULL(pStateHeap->pMediaStatesMemPool);

    MHW_RENDERHAL_CHK_STATUS(RenderHal_DSH_ExtendMediaStatePool(pStateHeap));

    //-------------------------------------------------------------------------
    // Calculate offsets/sizes in GSH
    //-------------------------------------------------------------------------
    // Synchronization, debugging data
    pStateHeap->dwOffsetSync = 0;
    pStateHeap->dwSizeSync   = pSettings->iSyncSize;

    // Synchronization
    pStateHeap->dwNextTag    = 0;
    pStateHeap->dwSyncTag    = 0;
    pStateHeap->dwFrameId    = 0;

    //----------------------------------
    // Instruction State Heap
    //----------------------------------
    // Kernel Spill Area - allocated on demand
    pStateHeap->dwScratchSpaceSize = 0;

    //----------------------------------
    // Surface State Heap
    //----------------------------------
    // Reset initial SSH allocations
    pStateHeap->iCurSshBufferIndex    = 0;
    pStateHeap->iCurrentBindingTable  = 0;
    pStateHeap->iCurrentSurfaceState  = 0;

    // Calculate size of each Binding Table
    pStateHeap->iBindingTableSize =
        MOS_ALIGN_CEIL(pSettings->iSurfacesPerBT * pHwSizes->dwSizeBindingTableState, pSettings->iBTAlignment);

    // Calculate memory size to store all binding tables and surface states
    pStateHeap->dwSizeSSH = pSettings->iBindingTables * pStateHeap->iBindingTableSize +
                            pSettings->iSurfaceStates *
                            pRenderHal->pRenderHalPltInterface->GetSurfaceStateCmdSize();
    pRenderHal->dwIndirectHeapSize = MOS_ALIGN_CEIL(pStateHeap->dwSizeSSH, MHW_PAGE_SIZE);

    // Allocate local copy of SSH
    pStateHeap->pSshBuffer = (uint8_t*)MOS_AllocAndZeroMemory(pStateHeap->dwSizeSSH);
    if (!pStateHeap->pSshBuffer)
    {
        MHW_RENDERHAL_ASSERTMESSAGE("Fail to Allocate SSH buffer.");
        eStatus = MOS_STATUS_NO_SPACE;
        goto finish;
    }

    pStateHeap->iBindingTableOffset  = 0;
    pStateHeap->iSurfaceStateOffset  = pSettings->iBindingTables * pStateHeap->iBindingTableSize;
    pStateHeap->pSurfaceEntry        = (PRENDERHAL_SURFACE_STATE_ENTRY) MOS_AllocAndZeroMemory(pSettings->iSurfaceStates * sizeof(RENDERHAL_SURFACE_STATE_ENTRY));
    MHW_RENDERHAL_CHK_NULL(pStateHeap->pSurfaceEntry);

    //----------------------------------
    // Allocate Instruction State Heap in MHW
    //----------------------------------
    MhwStateHeapSettings.dwIshSize         = pRenderHal->DynamicHeapSettings.dwIshInitialSize;
    MhwStateHeapSettings.m_keepIshLocked   = true;
    MhwStateHeapSettings.dwNumSyncTags     = pSettings->iSyncSize;
    MhwStateHeapSettings.dwIshIncrement    = pRenderHal->DynamicHeapSettings.dwIshSizeIncrement;
    MhwStateHeapSettings.dwIshMaxSize      = pRenderHal->DynamicHeapSettings.dwIshMaximumSize;

    MHW_RENDERHAL_CHK_STATUS(pMhwRender->AllocateHeaps(MhwStateHeapSettings));
    MHW_RENDERHAL_CHK_NULL(pMhwRender->m_stateHeapInterface);
    pMhwStateHeap = pRenderHal->pMhwStateHeap = pMhwRender->m_stateHeapInterface->pStateHeapInterface;

    // Size of a Media ID entry
    pStateHeap->dwSizeMediaID = pHwSizes->dwSizeInterfaceDescriptor;

    // Not used in Dynamic mode - heaps may change
    Mos_ResetResource(&pStateHeap->GshOsResource);
    pStateHeap->pGshBuffer = nullptr;
    pStateHeap->bGshLocked = false;

    Mos_ResetResource(&pStateHeap->IshOsResource);
    pStateHeap->pIshBuffer = nullptr;
    pStateHeap->bIshLocked = false;

    // Setup pointer to sync tags
    pStateHeap->pSync = (uint32_t *)pRenderHal->pMhwStateHeap->GetCmdBufIdGlobalPointer();

    // Reset pool of Kernel allocations objects
    pStateHeap->pKernelAllocMemPool = MOS_New(MHW_MEMORY_POOL,
                                sizeof(RENDERHAL_KRN_ALLOCATION),
                                sizeof(void *));
    MHW_RENDERHAL_CHK_NULL(pStateHeap->pKernelAllocMemPool);

    MHW_RENDERHAL_CHK_STATUS(RenderHal_DSH_ExtendKernelAllocPool(pStateHeap));

    eStatus = MOS_STATUS_SUCCESS;

finish:
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        // Free SSH buffer
        if (pStateHeap)
        {
            if (pStateHeap->pSurfaceEntry)
            {
                MOS_FreeMemory(pStateHeap->pSurfaceEntry);
            }

            if (pStateHeap->pSshBuffer)
            {
                MOS_FreeMemory(pStateHeap->pSshBuffer);
            }
        }
    }

    return eStatus;
}

//!
//! \brief    Free State Heaps (including MHW interfaces)
//! \details  Free State Heap resources allocated by RenderHal
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to Render Hal Interface
//! \return   MOS_STATUS
//!
MOS_STATUS RenderHal_DSH_FreeStateHeaps(PRENDERHAL_INTERFACE pRenderHal)
{
    PMOS_INTERFACE            pOsInterface;
    PRENDERHAL_STATE_HEAP     pStateHeap;
    MOS_STATUS                eStatus;

    //------------------------------------------------
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pOsInterface);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pStateHeap);
    //------------------------------------------------

    eStatus      = MOS_STATUS_UNKNOWN;

    pOsInterface = pRenderHal->pOsInterface;
    pStateHeap   = pRenderHal->pStateHeap;

    // Free Surface State Entries
    if (pStateHeap->pSurfaceEntry)
    {
        // Free MOS surface in surface state entry
        for (int32_t index = 0; index < pRenderHal->StateHeapSettings.iSurfaceStates; ++index) {
            PRENDERHAL_SURFACE_STATE_ENTRY entry = pStateHeap->pSurfaceEntry + index;
            MOS_SafeFreeMemory(entry->pSurface);
            entry->pSurface = nullptr;
        }

        MOS_FreeMemory(pStateHeap->pSurfaceEntry);
        pStateHeap->pSurfaceEntry = nullptr;
    }

    // Free SSH Resource
    if (pStateHeap->pSshBuffer)
    {
        MOS_FreeMemory(pStateHeap->pSshBuffer);
        pStateHeap->pSshBuffer = nullptr;
    }

    if(pStateHeap->pMediaStatesMemPool)
    {
        MOS_Delete(pStateHeap->pMediaStatesMemPool);
        pStateHeap->pMediaStatesMemPool = nullptr;
    }

    if(pStateHeap->pKernelAllocMemPool)
    {
        MOS_Delete(pStateHeap->pKernelAllocMemPool);
        pStateHeap->pKernelAllocMemPool = nullptr;
    }

    // Free kernel hash table
    pRenderHal->pStateHeap->kernelHashTable.Free();

    // Free State Heap Control structure
    MOS_AlignedFreeMemory(pStateHeap);
    pRenderHal->pStateHeap = nullptr;

    pRenderHal->pRenderHalPltInterface->FreeScratchSpaceBuffer(pRenderHal);

    eStatus = MOS_STATUS_SUCCESS;

finish:
    return eStatus;
}

//!
//! \brief    Assign memory block in state heap
//! \param    RENDERHAL_TR_RESOURCE trackerInfo
//!           [in] Pointer to tracker resource
//! \param    HeapManager heapManager
//!           [in] Pointer to heap manager
//! \param    MemoryBlock block
//!           [out] Pointer to memory block
//! \param    uint32_t size
//!           [in] size of requested memory block
//! \return   MOS_STATUS
//!
MOS_STATUS RenderHal_DSH_AssignSpaceInStateHeap(
    uint32_t trackerIndex,
    FrameTrackerProducer *trackerProducer,
    HeapManager *heapManager,
    MemoryBlock *block,
    uint32_t size)
{
    MOS_STATUS eStatus     = MOS_STATUS_SUCCESS;
    uint32_t   spaceNeeded = 0;
    std::vector<MemoryBlock> blocks;
    std::vector<uint32_t> blockSizes;

    MemoryBlockManager::AcquireParams acquireParams = 
        MemoryBlockManager::AcquireParams(trackerProducer->GetNextTracker(trackerIndex),
                                          blockSizes);
    acquireParams.m_trackerIndex = trackerIndex; // use the first tracker only

    MHW_RENDERHAL_CHK_NULL(heapManager);    
    MHW_RENDERHAL_CHK_NULL(block);
    MHW_RENDERHAL_CHK_NULL(trackerProducer);

    if (blockSizes.empty())
    {
        blockSizes.emplace_back(size);
    }
    else
    {
        blockSizes[0] = size;
    }

    MHW_RENDERHAL_CHK_STATUS(heapManager->AcquireSpace(acquireParams, blocks, spaceNeeded));

    if (blocks.empty())
    {
        MHW_RENDERHAL_ASSERTMESSAGE("No blocks were acquired");
        return MOS_STATUS_UNKNOWN;
    }
    if (!(blocks[0].IsValid()))
    {
        MHW_RENDERHAL_ASSERTMESSAGE("No blocks were acquired");
        return MOS_STATUS_UNKNOWN;
    }

    *block = blocks[0];

    // zero memory block
    block->AddData(nullptr, 0, 0, true);

finish:
    return eStatus;
}

//!
//! \brief    Refresh Sync
//! \details  Update Sync tags
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to Hardware Interface Structure
//! \return   MOS_STATUS
//!
MOS_STATUS RenderHal_DSH_RefreshSync(PRENDERHAL_INTERFACE pRenderHal)
{
    PRENDERHAL_STATE_HEAP       pStateHeap;
    PRENDERHAL_MEDIA_STATE      pCurMediaState;
    PRENDERHAL_MEDIA_STATE      pNextMediaState;
    PRENDERHAL_DYNAMIC_STATE    pDynamicState;
    PMHW_BATCH_BUFFER           pBatchBuffer;
    uint32_t                    dwCurrentTag;
    uint32_t                    dwCurrentFrameId;
    int32_t                     iStatesInUse;
    int32_t                     iBuffersInUse;
    int32_t                     iKernelsInUse;
    MOS_STATUS                  eStatus;
    MOS_NULL_RENDERING_FLAGS    NullRenderingFlags;
    PXMHW_STATE_HEAP_INTERFACE  pMhwStateHeap;
    PRENDERHAL_MEDIA_STATE_LIST pList;
    PRENDERHAL_KRN_ALLOCATION   pKrnAllocation, pNext;

    //----------------------------------
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pStateHeap);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pMhwStateHeap);
    //----------------------------------

    eStatus         = MOS_STATUS_UNKNOWN;

    // GSH must be locked
    pStateHeap    = pRenderHal->pStateHeap;
    pMhwStateHeap = pRenderHal->pMhwStateHeap;
    if (!pMhwStateHeap->GetCmdBufIdGlobalPointer())
    {
        goto finish;
    }

    // Most recent tags
    pStateHeap->dwSyncTag = dwCurrentTag     = pStateHeap->pSync[0];

    // Refresh batch buffers
    iBuffersInUse = 0;
    pBatchBuffer  = pRenderHal->pBatchBufferList;

    NullRenderingFlags  = pRenderHal->pOsInterface->pfnGetNullHWRenderFlags(pRenderHal->pOsInterface);

    for (; pBatchBuffer != nullptr; pBatchBuffer = pBatchBuffer->pNext)
    {
        if (!pBatchBuffer->bBusy) continue;

        // Clear BB busy flag when Sync Tag is reached
        if ((int32_t)(dwCurrentTag - pBatchBuffer->dwSyncTag) > 0 ||
            NullRenderingFlags.VPGobal)
        {
            pBatchBuffer->bBusy = false;
        }
        else
        {
            iBuffersInUse++;
        }
    }

    // Refresh media states
    pList = &(pStateHeap->SubmittedStates);

    pNextMediaState = nullptr;
    pCurMediaState  = pList->pHead;
    iStatesInUse    = 0;
    for (; pCurMediaState != nullptr; pCurMediaState = pNextMediaState)
    {
        // Save next media state before moving current state back to pool (if complete)
        pNextMediaState = pCurMediaState->pNext;

        if (!pCurMediaState->bBusy) continue;

        // The condition below is valid when sync tag wraps from 2^32-1 to 0
        if (FrameTrackerTokenFlat_IsExpired(&pCurMediaState->trackerToken))
        {
            pDynamicState = pCurMediaState->pDynamicState;
            pCurMediaState->bBusy = false;
            if (pRenderHal->bKerneltimeDump)
            {
                uint64_t uiNS;
                uint8_t *pCurrentPtr;
                uint64_t uiStartTime;
                uint64_t uiEndTime;
                uint64_t uiDiff;
                double  TimeMS;
                uint32_t uiComponent;
                uint32_t performanceSize = sizeof(uint64_t) * 2 + sizeof(RENDERHAL_COMPONENT);
                uint8_t *data = (uint8_t*)MOS_AllocAndZeroMemory(performanceSize);
                if (data == nullptr)
                {
                    eStatus = MOS_STATUS_NO_SPACE;
                    goto finish;
                }
                // Dump Kernel execution time when media state is being freed
                pDynamicState->memoryBlock.ReadData(data, pDynamicState->Performance.dwOffset, performanceSize);
                pCurrentPtr = data;
                uiStartTime = *((uint64_t *)pCurrentPtr);
                pCurrentPtr += sizeof(uint64_t);
                uiEndTime = *((uint64_t *)pCurrentPtr);
                pCurrentPtr += sizeof(uint64_t);
                uiComponent = *((RENDERHAL_COMPONENT *)pCurrentPtr);
                if (uiComponent < (uint32_t)RENDERHAL_COMPONENT_COUNT)
                {
                    // Convert ticks to ns
                    uiDiff = uiEndTime - uiStartTime;
                    uiNS = 0;
                    pRenderHal->pfnConvertToNanoSeconds(pRenderHal, uiDiff, &uiNS);

                    TimeMS = ((double)uiNS) / (1000 * 1000); // Convert to ms (double)

                    pRenderHal->kernelTime[uiComponent] += TimeMS;
                }

                MOS_SafeFreeMemory(data);
            }

            // Detach from submitted states, return to pool
            *((pCurMediaState->pNext) ? &(pCurMediaState->pNext->pPrev) : &(pList->pTail)) = pCurMediaState->pPrev;
            *((pCurMediaState->pPrev) ? &(pCurMediaState->pPrev->pNext) : &(pList->pHead)) = pCurMediaState->pNext;
            pCurMediaState->pPrev = pCurMediaState->pNext = nullptr;
            pList->iCount--;

            // Return media state object back to pool
            RenderHal_DSH_ReturnMediaStateToPool(pStateHeap, pCurMediaState);
        }
        else
        {
            iStatesInUse++;
        }
    }

    // Refresh kernels
    iKernelsInUse  = 0;
    pKrnAllocation = pStateHeap->KernelsSubmitted.pHead;
    for ( ; pKrnAllocation != nullptr; pKrnAllocation = pNext)
    {
        pNext = pKrnAllocation->pNext;

        // Move kernels back to allocated list (kernel remains cached)
        if (FrameTrackerTokenFlat_IsExpired(&pKrnAllocation->trackerToken))
        {
            RenderHal_DSH_KernelAttach(&pStateHeap->KernelsAllocated, pKrnAllocation, false);

            // Kernel block is flagged for removal (growing ISH) - delete block, mark kernel as stale
            if (pKrnAllocation->pMemoryBlock && pKrnAllocation->pMemoryBlock->bDelete)
            {
                pMhwStateHeap->FreeDynamicBlockDyn(MHW_ISH_TYPE, pKrnAllocation->pMemoryBlock);
                pKrnAllocation->dwFlags = RENDERHAL_KERNEL_ALLOCATION_USED;
            }
            else
            {
                // Mark kernel as in use, not locked
                pKrnAllocation->dwFlags = RENDERHAL_KERNEL_ALLOCATION_USED;
            }
        }
        else
        {
            iKernelsInUse++;
        }
    }

    // Refresh blocks
    pMhwStateHeap->RefreshDynamicHeapDyn(MHW_ISH_TYPE);

    // Save number of states/buffers in use
    pRenderHal->iBuffersInUse     = iBuffersInUse;
    pRenderHal->iKernelsInUse     = iKernelsInUse;
    pRenderHal->iMediaStatesInUse = iStatesInUse;

    eStatus = MOS_STATUS_SUCCESS;

finish:
    return eStatus;
}

//!
//! \brief    Extend the kernel state heap
//! \details  Increases the size of the ISH (up to the maximum size)
//!           Flags all submitted and allocated kernels as "stale" so can reload into new heap
//!           Allocates new ISH and deletes old heap
//!           Moves SIP binary to new heap
//! \param    PRENDERHAL_STATE_HEAP pStateHeap
//!           [in] Pointer to the state heap
//! \param    uint32_t dwAdditionalKernelSpaceNeeded
//!           [in] amount of additional space needed for kernel
//! \return   MOS_STATUS
//!
MOS_STATUS RenderHal_DSH_ExpandKernelStateHeap(
    PRENDERHAL_INTERFACE pRenderHal,
    uint32_t             dwAdditionalKernelSpaceNeeded)
{
    PXMHW_STATE_HEAP_INTERFACE pMhwStateHeap;
    PRENDERHAL_KRN_ALLOCATION pKrnAllocation;
    PRENDERHAL_STATE_HEAP     pStateHeap;
    uint32_t                  dwNewSize;
    MOS_STATUS                eStatus = MOS_STATUS_SUCCESS;
    PMHW_STATE_HEAP              pOldInstructionHeap;
    PMHW_STATE_HEAP_MEMORY_BLOCK pOldSipKernelBlock;

    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pStateHeap);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pMhwStateHeap);

    pMhwStateHeap = pRenderHal->pMhwStateHeap;
    pStateHeap    = pRenderHal->pStateHeap;

    // Increase size of the ISH (up to max size)
    dwNewSize = MOS_ALIGN_CEIL(pMhwStateHeap->GetISHPointer()->dwSize + dwAdditionalKernelSpaceNeeded,
                               pRenderHal->DynamicHeapSettings.dwIshSizeIncrement);
    if (dwNewSize > pRenderHal->DynamicHeapSettings.dwIshMaximumSize)
    {
        goto finish;
    }

    // Flag all submitted kernels as "stale", so they can be reloaded into the new heap
    for (pKrnAllocation = pStateHeap->KernelsSubmitted.pHead; pKrnAllocation != nullptr; pKrnAllocation = pKrnAllocation->pNext)
    {
        pKrnAllocation->dwFlags = RENDERHAL_KERNEL_ALLOCATION_STALE;
        if (pKrnAllocation->pMemoryBlock)
        {
            pKrnAllocation->pMemoryBlock->bStatic = false;
            MHW_RENDERHAL_CHK_STATUS(pMhwStateHeap->FreeDynamicBlockDyn(MHW_ISH_TYPE, pKrnAllocation->pMemoryBlock));
            pKrnAllocation->pMemoryBlock = nullptr;
        }
    }

    // Flag all kernels allocated as "stale", so they can be reloaded into the new heap
    for (pKrnAllocation = pStateHeap->KernelsAllocated.pHead; pKrnAllocation != nullptr; pKrnAllocation = pKrnAllocation->pNext)
    {
        pKrnAllocation->dwFlags = RENDERHAL_KERNEL_ALLOCATION_STALE;
        if (pKrnAllocation->pMemoryBlock)
        {
            MHW_RENDERHAL_CHK_STATUS(pMhwStateHeap->FreeDynamicBlockDyn(MHW_ISH_TYPE, pKrnAllocation->pMemoryBlock));
            pKrnAllocation->pMemoryBlock = nullptr;
        }
    }

    // Get SIP kernel block prior to expansion
    // NOTE: SIP kernel block is NOT associated with any KRN allocation and will be freed after copying to the new heap
    pOldInstructionHeap = pMhwStateHeap->GetISHPointer();
    pOldSipKernelBlock  = pOldInstructionHeap->pDebugKernel;

    // Allocate new instruction state heap
    MHW_RENDERHAL_CHK_STATUS(pMhwStateHeap->ExtendStateHeap(MHW_ISH_TYPE, dwNewSize));

    // Copy SIP kernel into the newly loaded ISH, mark old SIP block to be released when no longer in use
    if (pOldSipKernelBlock)
    {
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pfnLoadSipKernel(pRenderHal, pOldSipKernelBlock->pDataPtr, pOldSipKernelBlock->dwDataSize));
        pOldSipKernelBlock->bStatic = false;
        pOldInstructionHeap->pDebugKernel = nullptr;
    }

    // Free old instruction state heap
    MHW_RENDERHAL_CHK_STATUS(pMhwStateHeap->ReleaseStateHeapDyn(pOldInstructionHeap));

finish:
    return eStatus;
}

//!
//! \brief    Refresh available space in Dynamic ISH, create space by either unloading
//!           expired kernels or expanding the ISH (up to its max size)
//! \details  Refresh space in Dynamic ISH, creating contiguous space for kernel load
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to Hardware Interface Structure
//! \param    uint32_t dwSpaceNeeded
//!           [in] Space needed in dynamic ISH for new kernel to be loaded
//! \return   MOS_STATUS
//!
MOS_STATUS RenderHal_DSH_RefreshDynamicKernels(
    PRENDERHAL_INTERFACE        pRenderHal,
    uint32_t                    dwSpaceNeeded,
    uint32_t                    *pdwSizes,
    int32_t                     iCount)
{
    uint32_t                            TempArray[1] = { dwSpaceNeeded };
    MHW_STATE_HEAP_DYNAMIC_ALLOC_PARAMS Params;
    PRENDERHAL_STATE_HEAP               pStateHeap;
    PXMHW_STATE_HEAP_INTERFACE          pMhwStateHeap;
    PRENDERHAL_KRN_ALLOCATION           pKrnAllocation, pNext;
    MOS_STATUS                          eStatus = MOS_STATUS_SUCCESS;

    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pStateHeap);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pMhwStateHeap);

    pStateHeap    = pRenderHal->pStateHeap;
    pMhwStateHeap = pRenderHal->pMhwStateHeap;

    // Release move kernels from submitted back to allocated list according to Sync Tag
    pRenderHal->pfnRefreshSync(pRenderHal);

    // No block sizes provided, assume that the space requested must be contiguous
    if (!pdwSizes)
    {
        pdwSizes = TempArray;
        iCount   = 1;
    }

    // The function below will calculate how much space is still needed in state heap
    // considering the size of the blocks provided. This function accounts for fragmentation,
    // so even if the heap has enough space, the call may still return a non-zero value.

    Params.piSizes       = (int32_t*)pdwSizes;
    Params.iCount        = iCount;
    Params.dwAlignment   = 0;
    Params.bHeapAffinity = true;
    Params.pHeapAffinity = pMhwStateHeap->GetISHPointer(); // only search for space in active heap
    Params.dwScratchSpace= 0;

    dwSpaceNeeded = pMhwStateHeap->CalculateSpaceNeededDyn(MHW_ISH_TYPE, &Params);

    // Implementation note:
    // Kernels are moved from the KernelsAllocated list to the KernelsSubmitted
    // list whenever they are used via pfnTouchDynamicKernel function. When complete
    // they are moved back to end end of the KernelsAllocated list via pfnRefreshSync.
    // As a result, the allocation list is always sorted based on recent usage
    // (older kernels remain at the head of the list)
    //
    // The caching algorithm tries to find a balance between amount of times
    // the kernels was used, and how recently it was used - so kernels that are
    // heavily used but at the beginning of the list are less likely to be removed
    // compared with a kernel that was barely used (dwCount)
    //
    // IMPORTANT - Only one kernel allocation entry per Kernel UID (1:1 mapping)
    //

    pKrnAllocation = pStateHeap->KernelsAllocated.pHead;
    for ( ; (pKrnAllocation != nullptr) && (dwSpaceNeeded > 0); pKrnAllocation = pNext)
    {
        pNext = pKrnAllocation->pNext;

        if (pKrnAllocation->pMemoryBlock && pKrnAllocation->dwFlags != RENDERHAL_KERNEL_ALLOCATION_LOCKED)
        {
            // Only count this as available space if the block is in current heap!
            if (pKrnAllocation->pMemoryBlock->pStateHeap == pMhwStateHeap->GetISHPointer())
            {
                dwSpaceNeeded -= MOS_MIN(dwSpaceNeeded, pKrnAllocation->pMemoryBlock->dwBlockSize);
            }

            MHW_RENDERHAL_CHK_STATUS(pMhwStateHeap->FreeDynamicBlockDyn(MHW_ISH_TYPE, pKrnAllocation->pMemoryBlock));
            pKrnAllocation->pMemoryBlock = nullptr;
            pKrnAllocation->dwOffset     = 0;
            pKrnAllocation->dwFlags      = RENDERHAL_KERNEL_ALLOCATION_REMOVED;
        }
    }

    // Failed to remove enough kernels
    if (dwSpaceNeeded > 0)
    {
        eStatus = MOS_STATUS_UNKNOWN;
    }

finish:
    return eStatus;
}

void RenderHal_DSH_ResetDynamicKernels(
    PRENDERHAL_INTERFACE        pRenderHal)
{
    return;
}

//!
//! \brief    Load Kernel using dynamic state heap model
//! \details  Load a kernel from cache into GSH; do not unload kernels
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to Hardware Interface Structure
//! \param    PCRENDERHAL_KERNEL_PARAM pParameters
//!           [in] Pointer to Kernel Parameters
//! \param    PMHW_KERNEL_PARAMS pKernel
//!           [in] Pointer to Kernel entry
//! \param    Kdll_CacheEntry pKernelEntry
//!           [in] The cache entry pointer maintaining the load status.
//!                For cache entries from local variable,
//!                set it to nullptr to avoid memory corruption
//! \return   int32_t
//!           Index to a kernel allocation index
//!           -1 if invalid parameters, no available space and no
//!            deallocation possible
//!
PRENDERHAL_KRN_ALLOCATION RenderHal_DSH_LoadDynamicKernel(
    PRENDERHAL_INTERFACE        pRenderHal,
    PCRENDERHAL_KERNEL_PARAM    pParameters,
    PMHW_KERNEL_PARAM           pKernel,
    uint32_t                    *pdwLoaded)
{
    MHW_STATE_HEAP_DYNAMIC_ALLOC_PARAMS Params;              // Block allocation parameters
    PRENDERHAL_STATE_HEAP        pStateHeap;                 // Renderhal State Heap control structure
    PXMHW_STATE_HEAP_INTERFACE   pMhwStateHeap;              // Dynamic MHW state heap interface
    PRENDERHAL_KRN_ALLOCATION    pKernelAllocation = nullptr;   // Kernel Allocation
    PMHW_STATE_HEAP_MEMORY_BLOCK pKernelMemoryBlock;         // Memory block in ISH where kernel is loaded
    int32_t                      iKernelSize;                // Kernel size
    int32_t                      iKernelUniqueID;            // Kernel unique ID
    int32_t                      iKernelCacheID;             // Kernel cache ID
    uint16_t                     wSearchIndex = 0;
    MOS_STATUS                   eStatus = MOS_STATUS_SUCCESS;

    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pStateHeap);
    MHW_RENDERHAL_CHK_NULL(pParameters);
    MHW_RENDERHAL_CHK_NULL(pKernel);

    pStateHeap = pRenderHal->pStateHeap;
    pMhwStateHeap = pRenderHal->pMhwStateHeap;

    // Validate parameters
    if (pKernel->iSize == 0)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        MHW_RENDERHAL_NORMALMESSAGE("Failed to load kernel - invalid parameters.");
        goto finish;
    }

    // Kernel parameters
    iKernelSize     = pKernel->iSize;
    iKernelUniqueID = pKernel->iKUID;
    iKernelCacheID  = pKernel->iKCID;

    pKernelAllocation = (PRENDERHAL_KRN_ALLOCATION)pStateHeap->kernelHashTable.Search(iKernelUniqueID, iKernelCacheID, wSearchIndex);

    // Kernel already loaded
    if (pKernelAllocation)
    {
        // found match and Update kernel usage
        pRenderHal->pfnTouchDynamicKernel(pRenderHal, pKernelAllocation);

        // Increment reference counter
        pKernel->bLoaded = 1;

        goto finish;
    }

    // Get new kernel allocation entry
    pKernelAllocation = RenderHal_DSH_GetKernelAllocationFromPool(pRenderHal->pStateHeap);
    MHW_RENDERHAL_CHK_NULL(pKernelAllocation);

    // Simple allocation - get next kernel allocation entry available from pool

    // Prepare Media State Allocation in DSH
    Params.piSizes          = &iKernelSize;
    Params.iCount           = 1;
    Params.dwAlignment      = RENDERHAL_KERNEL_BLOCK_ALIGN;
    Params.bHeapAffinity    = 0;
    Params.pHeapAffinity    = 0;
    Params.dwScratchSpace   = 0;
    Params.pScratchSpace    = nullptr;
    Params.bZeroAssignedMem = 0;
    Params.bStatic          = true;
    Params.bGrow            = 0;

    pKernelMemoryBlock = pMhwStateHeap->AllocateDynamicBlockDyn(MHW_ISH_TYPE, &Params);

    // Here is where the dynamic state heap comes into play - by expanding ISH on demand to load a new kernel (ISH can literally start from size 0)
    if (!pKernelMemoryBlock)
    {
        // Extend ISH, making enough room to load the new kernel
        MHW_RENDERHAL_CHK_STATUS(RenderHal_DSH_ExpandKernelStateHeap(pRenderHal, iKernelSize));

        // Try again to load the kernel - this should now work, unless something went wrong with the allocation (max size reached)
        pKernelMemoryBlock = pMhwStateHeap->AllocateDynamicBlockDyn(MHW_ISH_TYPE, &Params);
    }

    // Failed to load kernel
    if (!pKernelMemoryBlock)
    {
        MHW_RENDERHAL_NORMALMESSAGE("Failed to load kernel - no space available in GSH.");
        // Return the kernel allocation to pool
        RenderHal_DSH_KernelAttach(&pStateHeap->KernelAllocationPool, pKernelAllocation, false);
        pKernelAllocation = nullptr;
        goto finish;
    }

    // Allocate kernel
    pKernelAllocation->iKID            = -1;
    pKernelAllocation->iKUID           = iKernelUniqueID;
    pKernelAllocation->iKCID           = iKernelCacheID;
    FrameTrackerTokenFlat_Clear(&pKernelAllocation->trackerToken);
    pKernelAllocation->dwOffset        = pKernelMemoryBlock->dwDataOffset;  // NOT USED IN DSH - KEPT FOR DEBUG SUPPORT
    pKernelAllocation->iSize           = pKernelMemoryBlock->dwDataSize;    // NOT USED IN DSH - KEPT FOR DEBUG SUPPORT
    pKernelAllocation->dwFlags         = RENDERHAL_KERNEL_ALLOCATION_USED;
    pKernelAllocation->dwCount         = 0;  // will be updated by "TouchKernel"
    pKernelAllocation->Params          = *pParameters;
    pKernelAllocation->pKernelEntry    = nullptr; // CURRENTLY NOT USED
    pKernelAllocation->iAllocIndex     = -1;   // DSH NOT USED
    pKernelAllocation->pMemoryBlock    = pKernelMemoryBlock;

    // Copy kernel data - securely erase unused part of the block
    MOS_SecureMemcpy(pKernelMemoryBlock->pDataPtr, iKernelSize, pKernel->pBinary, iKernelSize);
    if (iKernelSize < (int32_t)pKernelMemoryBlock->dwDataSize)
    {
        MOS_ZeroMemory((uint8_t*)pKernelMemoryBlock->pDataPtr + iKernelSize, pKernelMemoryBlock->dwDataSize - iKernelSize);
    }

finish:
    // Move kernel to the tail of the list of kernels (head of the list contains oldest kernels)
    if (pKernelAllocation)
    {
        // Move kernel allocation to end of list of kernels (sorted by usage)
        pRenderHal->pfnTouchDynamicKernel(pRenderHal, pKernelAllocation);
    }

    // Return kernel allocation
    return pKernelAllocation;
}

//!
//! \brief    Unload Kernel using dynamic model
//! \details  Unload a kernel from GSH, free kernel heap space
//!           Notify that the kernel has been unloaded (for tracking)
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to Hardware Interface Structure
//! \param    int32_t iKernelAllocationID
//!           [in] Kernel allocation index in GSH
//! \return   MOS_STATUS
//!           true if success
//!           false if invalid parameters or if kernel cannot be unloaded
//!
MOS_STATUS RenderHal_DSH_UnloadDynamicKernel(
    PRENDERHAL_INTERFACE        pRenderHal,
    PRENDERHAL_KRN_ALLOCATION   pKernelAllocation)
{
    PRENDERHAL_STATE_HEAP       pStateHeap;
    MOS_STATUS                  eStatus;

    //---------------------------------------
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pKernelAllocation);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pStateHeap);
    //---------------------------------------

    eStatus    = MOS_STATUS_UNKNOWN;
    pStateHeap = pRenderHal->pStateHeap;

    //---------------------------------------
    MHW_RENDERHAL_CHK_NULL(pStateHeap->pKernelAllocation);
    //---------------------------------------

    // Kernel already free - return it to pool
    if (pKernelAllocation->dwFlags == RENDERHAL_KERNEL_ALLOCATION_FREE)
    {
        goto finish;
    }

    // Update Sync tags
    MHW_RENDERHAL_CHK_STATUS(pRenderHal->pfnRefreshSync(pRenderHal));

    // Check if kernel may be unloaded
    if (!FrameTrackerTokenFlat_IsExpired(&pKernelAllocation->trackerToken))
    {
        goto finish;
    }

    // Release kernel entry (Offset/size may be used for reallocation)
    pKernelAllocation->iKID             = -1;
    pKernelAllocation->iKUID            = -1;
    pKernelAllocation->iKCID            = -1;
    FrameTrackerTokenFlat_Clear(&pKernelAllocation->trackerToken);
    pKernelAllocation->dwFlags          = RENDERHAL_KERNEL_ALLOCATION_FREE;
    pKernelAllocation->dwCount          = 0;
    pKernelAllocation->pKernelEntry     = nullptr;

    eStatus = MOS_STATUS_SUCCESS;

finish:
    return eStatus;
}

//!
//! \brief    Load Debug Kernel
//! \details  Load SIP Debug kernel from cache into GSH.
//!           Replace debug surface binding table index with the specified value.
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to Hardware Interface Structure
//! \param    void   *pSipKernel
//!           [in] Pointer to Debug (Sip) Kernel binary
//! \param    uint32_t dwSipSize
//!           [in] Debug (Sip) Kernel size
//! \return   MOS_STATUS
//!
MOS_STATUS RenderHal_DSH_LoadSipKernel(
    PRENDERHAL_INTERFACE    pRenderHal,
    void                    *pSipKernel,
    uint32_t                dwSipSize)
{
    PXMHW_STATE_HEAP_INTERFACE   pMhwStateHeap;
    PMHW_STATE_HEAP              pInstructionStateHeap = nullptr;
    PMHW_STATE_HEAP_MEMORY_BLOCK pIshMemoryBlock;
    MHW_STATE_HEAP_DYNAMIC_ALLOC_PARAMS Params;
    MOS_STATUS                   eStatus = MOS_STATUS_SUCCESS;

    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pMhwStateHeap);

    pMhwStateHeap = pRenderHal->pMhwStateHeap;

    // Mark previous SIP kernel for auto-release so it can be replaced
    pInstructionStateHeap = pMhwStateHeap->GetISHPointer();
    MHW_RENDERHAL_CHK_NULL(pInstructionStateHeap);
    if (pInstructionStateHeap->pDebugKernel)
    {
        pInstructionStateHeap->pDebugKernel->bStatic = false;
        pInstructionStateHeap->pDebugKernel = nullptr;
    }

    // If SIP size is zero or binary is nullptr, goto finish (SIP kernel unloaded)
    if (dwSipSize == 0 || pSipKernel == nullptr)
    {
        goto finish;
    }

    // Allocate new block for Debug (SIP) kernel
    MOS_ZeroMemory(&Params, sizeof(Params));
    Params.piSizes           = (int32_t*)&dwSipSize;
    Params.iCount            = 1;
    Params.dwAlignment       = RENDERHAL_KERNEL_BLOCK_ALIGN;
    Params.bHeapAffinity     = true;
    Params.pHeapAffinity     = pInstructionStateHeap;
    Params.dwScratchSpace    = 0;
    Params.pScratchSpace     = nullptr;
    Params.bZeroAssignedMem  = false;
    Params.bStatic           = true;
    Params.bGrow             = false;
#ifdef MHW_DYNAMIC_STATE_HEAP_DEBUGGING
    Params.dwBlockType       = MHW_BLOCK_DATA_SIP_KERNEL;
    Params.pClientObject     = nullptr;
#endif
    pIshMemoryBlock = pInstructionStateHeap->pDebugKernel = pMhwStateHeap->AllocateDynamicBlockDyn(MHW_ISH_TYPE, &Params);
    if (!pIshMemoryBlock)
    {
        eStatus = MOS_STATUS_NO_SPACE;
        goto finish;
    }

    // Copy SIP Debug kernel into SIP location in ISH (Instruction State Heap)
    MOS_SecureMemcpy(pIshMemoryBlock->pDataPtr, dwSipSize, pSipKernel, dwSipSize);

finish:
    return eStatus;
}

//!
//! \brief    Send SIP state command
//! \details  Send SIP state command pointing to the SIP kernel.
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to Hardware Interface Structure
//! \param    PMOS_COMMAND_BUFFER pCmdBuffer
//!           [in] Pointer to Command Buffer
//! \return   MOS_STATUS
//!
MOS_STATUS RenderHal_DSH_SendSipStateCmd(
    PRENDERHAL_INTERFACE  pRenderHal,
    PMOS_COMMAND_BUFFER   pCmdBuffer)
{
    PMHW_STATE_HEAP_MEMORY_BLOCK pSipMemoryBlock;
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pMhwStateHeap);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pMhwRenderInterface);

    pSipMemoryBlock = pRenderHal->pMhwStateHeap->GetISHPointer()->pDebugKernel;

    if ((pRenderHal->bSIPKernel || pRenderHal->bCSRKernel) && pSipMemoryBlock != nullptr)
    {
        MhwRenderInterface *pMhwRender        = pRenderHal->pMhwRenderInterface;
        pRenderHal->SipStateParams.bSipKernel = true;
        pRenderHal->SipStateParams.dwSipBase  = pSipMemoryBlock->dwDataOffset;
        eStatus = pMhwRender->AddSipStateCmd(pCmdBuffer, &pRenderHal->SipStateParams);
    }
    else
    {   // Fail - SIP kernel not loaded
        eStatus = MOS_STATUS_UNKNOWN;
    }

finish:
    return eStatus;
}
//!
//! \brief    Allocate Media ID
//! \details  Allocates an setup Interface Descriptor for Media Pipeline
//!           Kernel        must be preloaded using pfnLoadKernel
//!           Curbe         must be allocated using pfnAllocateCurbe
//!           Binding Table must be allocated using pfnAllocateBindingTable
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to Hardware Interface Structure
//! \param    int32_t iKernelAllocationID
//!           [in] Kernel Allocation ID
//! \param    int32_t iBindingTableID
//!           [in] Binding Table ID
//! \param    int32_t iCurbeOffset
//!           [in] Curbe Offset (from Curbe base)
//! \param    int32_t iCurbeLength
//!           [in] Curbe Length
//! \param    int32_t iCrsThrdConstDataLn
//!           [in] Cross Thread constant data length
//! \param    PMHW_GPGPU_WALKER_PARAMS pGpGpuWalkerParams
//!           [in] Pointer to GpGpu Walker Params
//! \return   int32_t
//!           Media Interface descriptor ID
//!           -1 if invalid parameters, no ID entry available in GSH
//!
int32_t RenderHal_DSH_AllocateDynamicMediaID(
    PRENDERHAL_INTERFACE        pRenderHal,
    PRENDERHAL_KRN_ALLOCATION   pKernelAllocation,
    int32_t                     iBindingTableID,
    int32_t                     iCurbeOffset,
    int32_t                     iCurbeLength,
    int32_t                     iCrsThrdConstDataLn,
    PMHW_GPGPU_WALKER_PARAMS    pGpGpuWalkerParams)
{
    PRENDERHAL_MEDIA_STATE      pMediaState;
    PRENDERHAL_DYNAMIC_STATE    pDynamicState;
    PRENDERHAL_KRN_ALLOCATION  *pKrnAllocationTable;
    int32_t                     iCurbeSize;
    int32_t                     iInterfaceDescriptor;
    RENDERHAL_INTERFACE_DESCRIPTOR_PARAMS InterfaceDescriptorParams;

    iInterfaceDescriptor = -1;

    // Validate Renderhal and state heap
    if (pRenderHal == nullptr || pRenderHal->pStateHeap == nullptr)
    {
        MHW_RENDERHAL_ASSERTMESSAGE("Invalid State.");
        goto finish;
    }

    // Obtain pointer to current media state
    pMediaState = pRenderHal->pStateHeap->pCurMediaState;
    if (pMediaState == nullptr || pMediaState->pDynamicState == nullptr)
    {
        MHW_RENDERHAL_ASSERTMESSAGE("Invalid Media State.");
        goto finish;
    }

    // Obtain pointer to dynamic media state
    pDynamicState = pMediaState->pDynamicState;

    // Validate kernel allocation ID (kernel must be pre-loaded into GSH)
    if (pKernelAllocation == nullptr            ||                      // Allocation must be valid
        pKernelAllocation->pMemoryBlock == nullptr ||                      // Kernel must be loaded
        //pKernelAllocation->dwFlags == RENDERHAL_KERNEL_ALLOCATION_FREE ||  kernel may be reused
        pKernelAllocation->iSize == 0)                                  // Size must be valid
    {
        MHW_RENDERHAL_ASSERTMESSAGE("Invalid Kernel Allocation");
        goto finish;
    }

    // Check Curbe allocation (CURBE_Lenght is in 256-bit count -> convert to bytes)
    iCurbeSize = iCurbeLength;
    if (iCurbeSize <= 0)
    {
        // Curbe is not used by the kernel
        iCurbeSize = iCurbeOffset = 0;
    }
    // Validate Curbe Offset (curbe must be pre-allocated)
    else if ( iCurbeOffset < 0 ||                                           // Not allocated
             (iCurbeOffset & 0x1F) != 0 ||                                  // Invalid alignment
             (iCurbeOffset + iCurbeSize) > pDynamicState->Curbe.iCurrent)   // Invalid size
    {
        MHW_RENDERHAL_ASSERTMESSAGE("Invalid Curbe Allocation.");
        goto finish;
    }

    // Try to reuse interface descriptor (for 2nd level buffer optimizations)
    // Check if ID already in use by another kernel - must use a different ID
    pKrnAllocationTable  = pDynamicState->pKrnAllocations;
    iInterfaceDescriptor = pKernelAllocation->iKID;
    if (iInterfaceDescriptor >= 0 &&                                            // If kernel has a preferred interface descriptor
        pKrnAllocationTable[iInterfaceDescriptor] != pKernelAllocation)         // AND the interface descriptor is used by another kernel ....
    {
        iInterfaceDescriptor = -1;                                              // Choose another media interface descriptor
    }

    // Search available ID in current media state heap
    if (iInterfaceDescriptor < 0)
    {
        int32_t iMax = pDynamicState->MediaID.iCount;
        for (iInterfaceDescriptor = 0;
             iInterfaceDescriptor < iMax;
             iInterfaceDescriptor++)
        {
            if (pKrnAllocationTable[iInterfaceDescriptor] == nullptr)
            {
                break;
            }
        }

        // All IDs are in use - fail
        if (iInterfaceDescriptor >= iMax)
        {
            MHW_RENDERHAL_ASSERTMESSAGE("No Interface Descriptor available.");
            iInterfaceDescriptor = -1;
            goto finish;
        }
    }

    if (iInterfaceDescriptor >= pDynamicState->MediaID.iCount)
    {
        MHW_RENDERHAL_ASSERTMESSAGE("Invalid Interface Descriptor.");
        iInterfaceDescriptor = -1;
        goto finish;
    }

    MOS_ZeroMemory((void *)&InterfaceDescriptorParams, sizeof(InterfaceDescriptorParams));

    InterfaceDescriptorParams.iMediaID            = iInterfaceDescriptor;
    InterfaceDescriptorParams.iBindingTableID     = iBindingTableID;
    InterfaceDescriptorParams.iCurbeOffset        = iCurbeOffset;
    InterfaceDescriptorParams.iCurbeLength        = iCurbeLength;
    InterfaceDescriptorParams.iCrsThrdConstDataLn = iCrsThrdConstDataLn;

    // barrier and slm
    if (pGpGpuWalkerParams && pGpGpuWalkerParams->GpGpuEnable)
    {
        InterfaceDescriptorParams.blBarrierEnable = true;
        InterfaceDescriptorParams.iNumberThreadsInGroup = pGpGpuWalkerParams->ThreadWidth * pGpGpuWalkerParams->ThreadHeight;
        InterfaceDescriptorParams.iSLMSize = pGpGpuWalkerParams->SLMSize; // The shift will be handled by pfnEncodeSLMSize()
        InterfaceDescriptorParams.blGlobalBarrierEnable = false;   // BDW+, default is 0
    }
    else //Reset barrier and slm setting since they may be set before
    {
        InterfaceDescriptorParams.blBarrierEnable = false;
        InterfaceDescriptorParams.iNumberThreadsInGroup = pRenderHal->dwMinNumberThreadsInGroup;
        InterfaceDescriptorParams.iSLMSize = 0;
        InterfaceDescriptorParams.iCrsThrdConstDataLn &= pRenderHal->dwMaskCrsThdConDataRdLn;
        InterfaceDescriptorParams.blGlobalBarrierEnable = false;   // BDW+, default is 0
    }

    // Setup Media ID entry - this call could be HW dependent
    if (MOS_STATUS_SUCCESS != pRenderHal->pfnSetupInterfaceDescriptor(
                                             pRenderHal,
                                             pMediaState,
                                             pKernelAllocation,
                                             &InterfaceDescriptorParams))
    {
        MHW_RENDERHAL_ASSERTMESSAGE("Failed to setup Interface Descriptor.");
        iInterfaceDescriptor = -1;
        goto finish;
    }

    // Set kernel allocation for the current Media ID
    pKrnAllocationTable[iInterfaceDescriptor] = pKernelAllocation;

    // Set preferred Media ID for the current kernel
    // This is necessary for 2nd level BB optimization.
    if (pKernelAllocation->iKID < 0)
    {
        pKernelAllocation->iKID = iInterfaceDescriptor;
    }

    // Update kernel usage
    pRenderHal->pfnTouchDynamicKernel(pRenderHal, pKernelAllocation);

finish:
    return iInterfaceDescriptor;
}

//!
//! \brief    Get a media ID
//! \details  Returns an interface descriptor for this media state
//!           sets kernel allocation for the media ID
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to the renderhal interface
//! \param    PRENDERHAL_MEDIA_STATE pMediaState
//!           [in] Pointer to the media state
//! \param    PRENDERHAL_KRN_ALLOCATION   pKernelAllocation
//!           [in] Pointer to the kernel allocation
//! \return   int32_t mediaID
//!
int32_t RenderHal_DSH_GetMediaID(
    PRENDERHAL_INTERFACE        pRenderHal,
    PRENDERHAL_MEDIA_STATE      pMediaState,
    PRENDERHAL_KRN_ALLOCATION   pKernelAllocation)
{
    PRENDERHAL_KRN_ALLOCATION *pKrnAllocations;
    int32_t                    iInterfaceDescriptor = -1;
    MOS_STATUS                 eStatus = MOS_STATUS_UNKNOWN;

    MHW_RENDERHAL_CHK_NULL(pMediaState);
    MHW_RENDERHAL_CHK_NULL(pMediaState->pDynamicState);
    MHW_RENDERHAL_CHK_NULL(pMediaState->pDynamicState->pKrnAllocations);
    MHW_RENDERHAL_CHK_NULL(pKernelAllocation);

    iInterfaceDescriptor = pKernelAllocation->iKID;
    pKrnAllocations      = pMediaState->pDynamicState->pKrnAllocations;

    // Try to reuse interface descriptor (for 2nd level buffer optimizations)
    // Check if ID already in use by another kernel - must use a different ID
    if (iInterfaceDescriptor >= 0 &&
        pKrnAllocations[iInterfaceDescriptor] != nullptr &&
        pKrnAllocations[iInterfaceDescriptor] != pKernelAllocation)
    {
        iInterfaceDescriptor = -1;
    }

    // Search available ID in current media state heap
    if (iInterfaceDescriptor < 0)
    {
        int32_t iMax = pMediaState->pDynamicState->MediaID.iCount;
        for (iInterfaceDescriptor = 0;
             iInterfaceDescriptor < iMax;
             iInterfaceDescriptor++)
        {
            if (pKrnAllocations[iInterfaceDescriptor] == nullptr)
            {
                break;
            }
        }

        // All IDs are in use - fail
        if (iInterfaceDescriptor >= iMax)
        {
            MHW_RENDERHAL_ASSERT("No Interface Descriptor available.");
            iInterfaceDescriptor = -1;
            goto finish;
        }
    }

    // Set kernel allocation for the current Media ID
    pKrnAllocations[iInterfaceDescriptor] = pKernelAllocation;

    // Set preferred Media ID for the current kernel
    // This is necessary for 2nd level BB optimization.
    if (pKernelAllocation->iKID < 0)
    {
        pKernelAllocation->iKID = iInterfaceDescriptor;
    }

finish:
    return iInterfaceDescriptor;
}

//!
//! \brief    Assign Dynamic Media State
//! \details  Gets a pointer to the next available media state in GSH;
//!           fails if not available
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to Hadrware Interface Structure
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to Hadrware Interface Structure
//! \param    RENDERHAL_COMPONENT componentID
//!           [in] Identifier of the requesting component
//! \return   PRENDERHAL_MEDIA_STATE
//!           gets a new Media State, returns pointer to Media State structure
//!           nullptr - invalid, no states available + timeout
//!
PRENDERHAL_MEDIA_STATE RenderHal_DSH_AssignDynamicState(
    PRENDERHAL_INTERFACE                  pRenderHal,
    PRENDERHAL_DYNAMIC_MEDIA_STATE_PARAMS pParams,
    RENDERHAL_COMPONENT                   componentID)
{
    MHW_STATE_HEAP_DYNAMIC_ALLOC_PARAMS Params;              // Block allocation parameters
    PRENDERHAL_MEDIA_STATE              pMediaState = nullptr;  // Media state structure
    PRENDERHAL_DYNAMIC_STATE            pDynamicState;       // Dynamic media state structure
    PXMHW_STATE_HEAP_INTERFACE          pMhwStateHeap;
    PMHW_RENDER_STATE_SIZES             pHwSizes;
    MOS_STATUS                          eStatus = MOS_STATUS_SUCCESS;
    uint32_t                            dwSizeMediaState = 0;
    uint32_t                            dwMediaStateAlign;
    uint32_t                            dwSamplerStateAlign;
    uint8_t                             *pCurrentPtr;
    uint8_t                             *performanceMemory = nullptr;
    uint32_t                            performanceSize;
    uint32_t                            currentExtendSize = 0;

    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pStateHeap);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pMhwStateHeap);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pOsInterface);

    // Assign new media state (get from pool)
    pMediaState = RenderHal_DSH_GetMediaStateFromPool(pRenderHal->pStateHeap);
    MHW_RENDERHAL_CHK_NULL(pMediaState);
    MHW_RENDERHAL_CHK_NULL(pMediaState->pDynamicState);

    // Get pointer to state heap interface
    pMhwStateHeap = pRenderHal->pMhwStateHeap;

    // Get hw structure sizes
    pHwSizes = pRenderHal->pHwSizes;

    // Point to dynamic state
    pDynamicState = pMediaState->pDynamicState;
    MOS_ZeroMemory(pDynamicState, sizeof(RENDERHAL_DYNAMIC_STATE));

    // Reset some states
    pMediaState->bBusy = false; // will be busy after submission

    // Setup parameters
    pDynamicState->Curbe.dwOffset = dwSizeMediaState;
    pDynamicState->Curbe.dwSize   = (uint32_t)MOS_MAX(pParams->iMaxCurbeSize, pParams->iMaxCurbeOffset);

    // Align media state for CURBE
    dwMediaStateAlign = pRenderHal->dwCurbeBlockAlign;
    dwSizeMediaState  = MOS_ALIGN_CEIL(pDynamicState->Curbe.dwSize, dwMediaStateAlign);

    // Align media states for Samplers
    dwSamplerStateAlign = MHW_SAMPLER_STATE_ALIGN;
    dwSizeMediaState    = MOS_ALIGN_CEIL(dwSizeMediaState, dwSamplerStateAlign);

    // Allocate Samplers
    if (pRenderHal->bHasCombinedAVSSamplerState == false)
    {
        uint32_t dwSamplerSize = 0;

        // This is needed regardless of 3D samplers
        /// Used to calculate offset to sampler area during interface descriptor setup
        pDynamicState->Sampler3D.dwOffset = dwSizeMediaState;

        // 3D Sampler states
        if (pParams->iMaxSamplerIndex3D > 0)
        {
            pDynamicState->Sampler3D.iCount   = pParams->iMaxSamplerIndex3D;
            pDynamicState->Sampler3D.dwSize   = MOS_ALIGN_CEIL(pParams->iMaxSamplerIndex3D * pHwSizes->dwSizeSamplerState,  MHW_SAMPLER_STATE_ALIGN);
            dwSamplerSize                     = pDynamicState->Sampler3D.dwSize;
        }

        // AVS sampler states (colocated with 3D => make sure the area is large enough for both)
        if (pParams->iMaxSamplerIndexAVS > 0)
        {
            pDynamicState->SamplerAVS.dwOffset = dwSizeMediaState;
            pDynamicState->SamplerAVS.iCount   = pParams->iMaxSamplerIndexAVS;
            pDynamicState->SamplerAVS.dwSize   = MOS_ALIGN_CEIL(pParams->iMaxSamplerIndexAVS * pHwSizes->dwSizeSamplerStateAvs,  MHW_SAMPLER_STATE_ALIGN);
            dwSamplerSize                      = MOS_MAX(dwSamplerSize, pDynamicState->SamplerAVS.dwSize);
        }

        // Sampler Indirect State (border color associated with each 3D sampler)
        if (pParams->iMaxSamplerIndex3D > 0)
        {
            pDynamicState->SamplerInd.dwOffset = MOS_ALIGN_CEIL(dwSizeMediaState + dwSamplerSize, MHW_SAMPLER_STATE_ALIGN);
            pDynamicState->SamplerInd.iCount   = pParams->iMaxSamplerIndex3D;
            pDynamicState->SamplerInd.dwSize   = MOS_ALIGN_CEIL(pParams->iMaxSamplerIndex3D * pHwSizes->dwSizeSamplerIndirectState, MHW_SAMPLER_STATE_ALIGN);
            dwSamplerSize                     += pDynamicState->SamplerInd.dwSize;
        }

        pDynamicState->dwSizeSamplers = dwSamplerSize = MOS_ALIGN_CEIL(dwSamplerSize, dwSamplerStateAlign);
        dwSamplerSize *= pParams->iMaxMediaIDs;     // Move tables to end of all sampler states

        // 8x8 tables
        if (pParams->iMax8x8Tables > 0)
        {
            pDynamicState->Table8x8.dwOffset   = MOS_ALIGN_CEIL(dwSizeMediaState + dwSamplerSize, MHW_SAMPLER_STATE_ALIGN);
            pDynamicState->Table8x8.iCount     = pParams->iMax8x8Tables;
            pDynamicState->Table8x8.dwSize     = MOS_ALIGN_CEIL(pHwSizes->dwSizeSamplerStateTable8x8,  MHW_SAMPLER_STATE_ALIGN);
            dwSamplerSize                     += pParams->iMax8x8Tables * pDynamicState->Table8x8.dwSize;
       }

        dwSamplerSize     = MOS_ALIGN_CEIL(dwSamplerSize, dwSamplerStateAlign);
        dwSizeMediaState += dwSamplerSize;
    }
    else
    {
        uint32_t dwSamplerSize = 0;

        // common base for all sampler types
        pDynamicState->Sampler3D  .dwOffset = dwSizeMediaState;
        pDynamicState->SamplerAVS .dwOffset = dwSizeMediaState;
        pDynamicState->SamplerConv.dwOffset = dwSizeMediaState;
        pDynamicState->SamplerMisc.dwOffset = dwSizeMediaState;

        // 3D sampler
        pDynamicState->Sampler3D.iCount = pParams->iMaxSamplerIndex3D;
        pDynamicState->Sampler3D.dwSize = pParams->iMaxSamplerIndex3D * pHwSizes->dwSizeSamplerState;

        // Misc (VA) sampler
        if (pParams->iMaxSamplerIndexMisc > 0)
        {
            dwSamplerStateAlign = MOS_MAX(dwMediaStateAlign, MHW_SAMPLER_STATE_VA_ALIGN);
            pDynamicState->SamplerMisc.iCount = pParams->iMaxSamplerIndexMisc;
            pDynamicState->SamplerMisc.dwSize = pParams->iMaxSamplerIndexMisc * pHwSizes->dwSizeSamplerStateVA;
        }

        // AVS sampler
        if (pParams->iMaxSamplerIndexAVS > 0)
        {
            uint32_t dwAlign = GFX_IS_GEN_9_OR_LATER(pRenderHal->Platform) ? MHW_SAMPLER_STATE_AVS_ALIGN_G9 : MHW_SAMPLER_STATE_AVS_ALIGN;
            uint32_t dwInc   = GFX_IS_GEN_9_OR_LATER(pRenderHal->Platform) ? MHW_SAMPLER_STATE_AVS_INC_G9   : MHW_SAMPLER_STATE_AVS_INC_G8;

            dwSamplerStateAlign = MOS_MAX(dwMediaStateAlign, dwAlign);
            pDynamicState->SamplerAVS.iCount = pParams->iMaxSamplerIndexAVS;
            pDynamicState->SamplerAVS.dwSize = (pParams->iMaxSamplerIndexAVS - 1) * dwInc + pHwSizes->dwSizeSamplerStateAvs;
        }

        // Conv sampler
        if (pParams->iMaxSamplerIndexConv > 0)
        {
            uint32_t dwAlign = GFX_IS_GEN_9_OR_LATER(pRenderHal->Platform) ? MHW_SAMPLER_STATE_AVS_ALIGN_G9 : MHW_SAMPLER_STATE_AVS_ALIGN;
            uint32_t dwInc   = GFX_IS_GEN_9_OR_LATER(pRenderHal->Platform) ? MHW_SAMPLER_STATE_CONV_INC_G9  : MHW_SAMPLER_STATE_CONV_INC_G8;

            dwSamplerStateAlign = MOS_MAX(dwMediaStateAlign, dwAlign);
            pDynamicState->SamplerConv.iCount = pParams->iMaxSamplerIndexConv;
            pDynamicState->SamplerConv.dwSize = (pParams->iMaxSamplerIndexConv - 1) * dwInc + pHwSizes->dwSizeSamplerStateVAConvolve;
        }

        // Get largest of all
        dwSamplerSize = MOS_MAX(pDynamicState->Sampler3D.dwSize, pDynamicState->SamplerMisc.dwSize);
        dwSamplerSize = MOS_MAX(pDynamicState->SamplerAVS.dwSize, dwSamplerSize);
        dwSamplerSize = MOS_MAX(pDynamicState->SamplerConv.dwSize, dwSamplerSize);

        // Sampler Indirect State (border color associated with each 3D sampler)
        if (pParams->iMaxSamplerIndex3D > 0)
        {
            dwSamplerSize                      = MOS_ALIGN_CEIL(dwSamplerSize, MHW_SAMPLER_STATE_ALIGN);
            pDynamicState->SamplerInd.dwOffset = dwSizeMediaState + dwSamplerSize;
            pDynamicState->SamplerInd.iCount   = pParams->iMaxSamplerIndex3D;
            pDynamicState->SamplerInd.dwSize   = pParams->iMaxSamplerIndex3D * pHwSizes->dwSizeSamplerIndirectState;
            dwSamplerSize                     += pDynamicState->SamplerInd.dwSize;
        }

        dwSamplerSize = MOS_ALIGN_CEIL(dwSamplerSize, dwSamplerStateAlign);

        pDynamicState->dwSizeSamplers = dwSamplerSize;
        dwSizeMediaState += pParams->iMaxMediaIDs * pDynamicState->dwSizeSamplers;
    }

    // Interface Descriptors
    pDynamicState->MediaID.iCount   = pParams->iMaxMediaIDs;
    pDynamicState->MediaID.dwOffset = dwSizeMediaState;
    pDynamicState->MediaID.dwSize   = pParams->iMaxMediaIDs * pHwSizes->dwSizeInterfaceDescriptor;
    dwSizeMediaState               += pDynamicState->MediaID.dwSize;

    // Area for Performance collection
    pDynamicState->Performance.iCount   = 1;
    pDynamicState->Performance.dwOffset = dwSizeMediaState;
    pDynamicState->Performance.dwSize   = 64;
    dwSizeMediaState                   += pDynamicState->Performance.dwSize;

    // Kernel Spill Area
    if (pParams->iMaxSpillSize > 0)
    {
        // per thread scratch space must be 1K*(2^n), (2K*(2^n) for BDW A0), alignment is 1kB
        int iPerThreadScratchSpace = 0;
        if (pRenderHal->pfnPerThreadScratchSpaceStart2K(pRenderHal))
        {
            iPerThreadScratchSpace = 2048;
        }
        else if (pRenderHal->pRenderHalPltInterface
                 ->PerThreadScratchSpaceStart64Byte(pRenderHal))
        {
            iPerThreadScratchSpace = 64;
        }
        else
        {
            iPerThreadScratchSpace = 1024;
        }

        for (iPerThreadScratchSpace; iPerThreadScratchSpace < pParams->iMaxSpillSize; iPerThreadScratchSpace <<= 1);
        pDynamicState->iMaxScratchSpacePerThread = pParams->iMaxSpillSize
                = iPerThreadScratchSpace;

        MOS_STATUS result = pRenderHal->pRenderHalPltInterface
            ->AllocateScratchSpaceBuffer(iPerThreadScratchSpace, pRenderHal);
        if (MOS_STATUS_UNIMPLEMENTED == result)  // Scratch space buffer is not supported
        {
            pDynamicState->dwScratchSpace
                    = pRenderHal->pfnGetScratchSpaceSize(pRenderHal,
                                                         iPerThreadScratchSpace);
            pDynamicState->scratchSpaceOffset = dwSizeMediaState;

            // Allocate more 1k space in state heap, which is used to make scratch space offset 1k-aligned.
            dwSizeMediaState += pDynamicState->dwScratchSpace + MHW_SCRATCH_SPACE_ALIGN;
            currentExtendSize = pRenderHal->dgsheapManager->GetExtendSize();
            if (currentExtendSize < pDynamicState->dwScratchSpace)
            {
                // update extend size for scratch space
                MHW_RENDERHAL_CHK_STATUS(
                    pRenderHal->dgsheapManager->SetExtendHeapSize(
                        pDynamicState->dwScratchSpace));
            }
        }
        else
        {
            MHW_RENDERHAL_CHK_STATUS(result);
        }
    }

    // Use generic heap manager to allocate memory block for dynamic general state heap
    MHW_RENDERHAL_CHK_STATUS(pRenderHal->pfnAssignSpaceInStateHeap(
        pRenderHal->currentTrackerIndex,
        &pRenderHal->trackerProducer,
        pRenderHal->dgsheapManager,
        &pDynamicState->memoryBlock,
        dwSizeMediaState)); 

    if (pParams->iMaxSpillSize > 0 && currentExtendSize > 0)
    {
        // Restore original extend heap size
        MHW_RENDERHAL_CHK_STATUS(
            pRenderHal->dgsheapManager->SetExtendHeapSize(
                currentExtendSize));

        // Specifies the 1k-byte aligned address offset to scratch space for
        // use by the kernel.  This pointer is relative to the
        // General State Base Address (1k aligned)
        // Format = GeneralStateOffset[31:10]
        pDynamicState->scratchSpaceOffset += pDynamicState->memoryBlock.GetOffset();
        pDynamicState->scratchSpaceOffset = MOS_ALIGN_CEIL(pDynamicState->scratchSpaceOffset, MHW_SCRATCH_SPACE_ALIGN);
    }
 
    // set the sync tag for the media state
    FrameTrackerTokenFlat_SetProducer(&pMediaState->trackerToken, &pRenderHal->trackerProducer);
    FrameTrackerTokenFlat_Merge(&pMediaState->trackerToken, 
                                pRenderHal->currentTrackerIndex,
                                pRenderHal->trackerProducer.GetNextTracker(pRenderHal->currentTrackerIndex));

    // Reset HW allocations
    pRenderHal->iChromaKeyCount = 0;
    for (int32_t i = 0; i < pRenderHal->iMaxPalettes; i++)
    {
        pRenderHal->Palette[i].iNumEntries = 0;
    }

    // Zero Memory start time and end time
    performanceSize = (sizeof(uint64_t) * 2) + sizeof(RENDERHAL_COMPONENT);
    performanceMemory = (uint8_t*)MOS_AllocAndZeroMemory(performanceSize);
    pCurrentPtr = performanceMemory;
    pCurrentPtr += (sizeof(uint64_t) * 2);
    *((RENDERHAL_COMPONENT *)pCurrentPtr) = componentID;
    pDynamicState->memoryBlock.AddData(
        performanceMemory, 
        pDynamicState->Performance.dwOffset, 
        performanceSize);

finish:

    MOS_SafeFreeMemory(performanceMemory);

    if (eStatus == MOS_STATUS_SUCCESS)
    {
        if (pRenderHal && pRenderHal->pStateHeap)
        {
            pRenderHal->pStateHeap->pCurMediaState = pMediaState;
        }

        if (pRenderHal)
        {
            // Refresh sync tag for all media states in submitted queue
            pRenderHal->pfnRefreshSync(pRenderHal);
        }
    }
    else
    {
        RenderHal_DSH_ReturnMediaStateToPool(pRenderHal->pStateHeap, pMediaState);
        pMediaState = nullptr;
    }

    return pMediaState;
}

//!
//! \brief    Releases the dynamic state
//! \details  Returns the media state to the pool
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to renderhal interface
//! \param    PRENDERHAL_MEDIA_STATE pMediaState
//!           [in] Pointer to media state
//! \return   MOS_STATUS
//!
MOS_STATUS RenderHal_DSH_ReleaseDynamicState(
    PRENDERHAL_INTERFACE   pRenderHal,
    PRENDERHAL_MEDIA_STATE pMediaState)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pStateHeap);
    MHW_RENDERHAL_CHK_NULL(pMediaState);

    // Media state cannot be busy or attached to any list (free/submitted)
    if (pMediaState->bBusy || pMediaState->pNext || pMediaState->pPrev)
    {
        MHW_RENDERHAL_ASSERTMESSAGE("Invalid Media State object");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        goto finish;
    }

    // Return media state to pool for reuse
    RenderHal_DSH_ReturnMediaStateToPool(pRenderHal->pStateHeap, pMediaState);

finish:
    return eStatus;
}

//!
//! \brief    Submit the dynamic state
//! \details  Move memory block to the submitted queue
//!           Updates the sync tag for the scratch space
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to renderhal interface
//! \param    PRENDERHAL_MEDIA_STATE pMediaState
//!           [in] Pointer to media state
//! \return   MOS_STATUS
//!
MOS_STATUS RenderHal_DSH_SubmitDynamicState(
    PRENDERHAL_INTERFACE   pRenderHal,
    PRENDERHAL_MEDIA_STATE pMediaState)
{
    PRENDERHAL_MEDIA_STATE_LIST pList;
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    std::vector<MemoryBlock> blocks;

    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pStateHeap);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pMhwStateHeap);
    MHW_RENDERHAL_CHK_NULL(pMediaState);
    MHW_RENDERHAL_CHK_NULL(pMediaState->pDynamicState);

    // Media state cannot be attached to any list (free/submitted)
    if (pMediaState->pNext || pMediaState->pPrev)
    {
        MHW_RENDERHAL_ASSERTMESSAGE("Invalid Media State object");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        goto finish;
    }

    // Flag as busy (should be already)
    pMediaState->bBusy = true;

    blocks.push_back(pMediaState->pDynamicState->memoryBlock);
    pRenderHal->dgsheapManager->SubmitBlocks(blocks);

    // Attached to end of submitted queue (media state in execution queue)
    pList = &pRenderHal->pStateHeap->SubmittedStates;

    pMediaState->pPrev = pList->pTail;
    pList->pTail = pMediaState;
    if (pMediaState->pPrev)
    {
        pMediaState->pPrev->pNext = pMediaState;
    }
    else
    {   // List was empty - insert first element
        MHW_ASSERT(pList->iCount == 0);
        pList->pHead = pMediaState;
    }
    pList->iCount++;

finish:
    return eStatus;
}

//!
//! \brief    Load Curbe Data
//! \details  Allocates and load CURBE data for Media
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in]  Pointer to RenderHal Interface structure
//! \param    PRENDERHAL_MEDIA_STATE pCurMediaState
//!           [out] Pointer to Current Media State structure
//! \param    void  *pData
//!           [in]  Pointer to Data
//! \param    int32_t iSize
//!           [in]  Number of bytes to allocate
//! \return   int32_t
//!           Offset of the CURBE block from CURBE base (in bytes)
//!           -1 if no CURBE space available in GSH
//!
int32_t RenderHal_DSH_LoadCurbeData(
    PRENDERHAL_INTERFACE    pRenderHal,
    PRENDERHAL_MEDIA_STATE  pMediaState,
    void                    *pData,
    int32_t                 iSize)
{
    MOS_STATUS               eStatus = MOS_STATUS_SUCCESS;
    int32_t                  iOffset;
    int32_t                  iCurbeSize;
    PRENDERHAL_DYNAMIC_STATE pDynamicState;
    uint8_t*                 pRemainingCurbe;

    iOffset         = -1;
    pRemainingCurbe =  nullptr;

    if (pRenderHal == nullptr || pMediaState == nullptr || pData == nullptr)
    {
        MHW_RENDERHAL_ASSERTMESSAGE("Null pointer found.");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        goto finish;
    }

    if (pMediaState && pMediaState->pDynamicState)
    {
        // Check if dynamic state is valid
        pDynamicState = pMediaState->pDynamicState;
        if (!pDynamicState->memoryBlock.IsValid())
        {
            goto finish;
        }

        iCurbeSize = MOS_ALIGN_CEIL(iSize, pRenderHal->dwCurbeBlockAlign);
        if (pDynamicState->Curbe.iCurrent + iCurbeSize <= (int)pDynamicState->Curbe.dwSize)
        {
            iOffset = pDynamicState->Curbe.iCurrent;
            pDynamicState->Curbe.iCurrent += iCurbeSize;

            if (pData)
            {
                MHW_RENDERHAL_CHK_STATUS(pDynamicState->memoryBlock.AddData(
                    pData,
                    pDynamicState->Curbe.dwOffset + iOffset,
                    iSize));

                // Zero remaining CURBE (for buffer alignment)
                iCurbeSize -= iSize;
                if (iCurbeSize > 0)
                {
                    pRemainingCurbe = (uint8_t*)MOS_AllocAndZeroMemory(sizeof(uint8_t)*iCurbeSize);
                    MHW_RENDERHAL_CHK_STATUS(pDynamicState->memoryBlock.AddData(
                        pRemainingCurbe,
                        pDynamicState->Curbe.dwOffset + iOffset + iSize,
                        iCurbeSize));
                }
            }
        }
    }

finish:
    if(pRemainingCurbe)
    {
        MOS_SafeFreeMemory(pRemainingCurbe);
    }

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        iOffset = -1;
    }
    return iOffset;
}

//!
//! \brief    Send Curbe Load
//! \details  Send Curbe Load command
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to Hardware Interface Structure
//! \param    PMOS_COMMAND_BUFFER pCmdBuffer
//!           [in] Pointer to Command Buffer
//! \return   MOS_STATUS
//!
MOS_STATUS RenderHal_DSH_SendCurbeLoad(
    PRENDERHAL_INTERFACE    pRenderHal,
    PMOS_COMMAND_BUFFER     pCmdBuffer)
{
    MHW_CURBE_LOAD_PARAMS    CurbeLoadParams;
    PRENDERHAL_STATE_HEAP    pStateHeap;
    PRENDERHAL_MEDIA_STATE   pMediaState;
    PRENDERHAL_DYNAMIC_STATE pDynamicState;
    MOS_STATUS               eStatus = MOS_STATUS_SUCCESS;

    //-----------------------------------------
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pCmdBuffer);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pStateHeap);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pMhwRenderInterface);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pStateHeap->pCurMediaState);
    //-----------------------------------------

    pMediaState = pRenderHal->pStateHeap->pCurMediaState;
    MHW_RENDERHAL_CHK_NULL(pMediaState->pDynamicState);

    eStatus       = MOS_STATUS_SUCCESS;
    pStateHeap    = pRenderHal->pStateHeap;
    pDynamicState = pMediaState->pDynamicState;

    // CURBE size is in bytes
    if (pDynamicState->Curbe.iCurrent != 0)
    {
        CurbeLoadParams.pKernelState            = nullptr;
        CurbeLoadParams.bOldInterface           = false;
        CurbeLoadParams.dwCURBETotalDataLength  = pDynamicState->Curbe.iCurrent;
        CurbeLoadParams.dwCURBEDataStartAddress = pDynamicState->memoryBlock.GetOffset() +   // media state offset from GSH base
                                                  pDynamicState->Curbe.dwOffset;                // curbe data offset in media state

        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwRenderInterface->AddMediaCurbeLoadCmd(pCmdBuffer, &CurbeLoadParams));
    }

finish:
    return eStatus;
}

//!
//! \brief    Send state base address
//! \details  Send state base address command
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to Hardware Interface Structure
//! \param    PMOS_COMMAND_BUFFER pCmdBuffer
//!           [in] Pointer to Command Buffer
//! \return   MOS_STATUS
//!
MOS_STATUS RenderHal_DSH_SendStateBaseAddress(PRENDERHAL_INTERFACE pRenderHal, PMOS_COMMAND_BUFFER pCmdBuffer)
{
    PRENDERHAL_STATE_HEAP       pStateHeap;
    PRENDERHAL_DYNAMIC_STATE    pDynamicState;
    PMOS_RESOURCE               pGshResource;
    uint32_t                    dwGshSize;
    PMOS_RESOURCE               pIshResource;
    uint32_t                    dwIshSize;
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;
    //----------------------------------------
    MHW_RENDERHAL_CHK_NULL(pCmdBuffer);
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pStateHeap);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pStateHeap->pCurMediaState);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pMhwStateHeap);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pMhwRenderInterface);
    //----------------------------------------

    pStateHeap    = pRenderHal->pStateHeap;
    pDynamicState = pStateHeap->pCurMediaState->pDynamicState;

    MHW_RENDERHAL_CHK_NULL(pDynamicState);

    pGshResource  = pDynamicState->memoryBlock.GetResource();
    dwGshSize     = pDynamicState->memoryBlock.GetHeapSize();
    pIshResource  = &(pRenderHal->pMhwStateHeap->GetISHPointer()->resHeap);
    dwIshSize     = pRenderHal->pMhwStateHeap->GetISHPointer()->dwSize;

    pRenderHal->StateBaseAddressParams.presGeneralState              = pGshResource;
    pRenderHal->StateBaseAddressParams.dwGeneralStateSize            = dwGshSize;
    pRenderHal->StateBaseAddressParams.presDynamicState              = pGshResource;
    pRenderHal->StateBaseAddressParams.dwDynamicStateSize            = dwGshSize;
    pRenderHal->StateBaseAddressParams.bDynamicStateRenderTarget     = false;
    pRenderHal->StateBaseAddressParams.presIndirectObjectBuffer      = pGshResource;
    pRenderHal->StateBaseAddressParams.dwIndirectObjectBufferSize    = dwGshSize;
    pRenderHal->StateBaseAddressParams.presInstructionBuffer         = pIshResource;
    pRenderHal->StateBaseAddressParams.dwInstructionBufferSize       = dwIshSize;

    eStatus = pRenderHal->pMhwRenderInterface->AddStateBaseAddrCmd(pCmdBuffer,
                                                                   &pRenderHal->StateBaseAddressParams);
finish:
    return eStatus;
}

//!
//! \brief    Reset RenderHal States
//! \details  Reset RenderHal States in preparation for a new command buffer
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to RenderHal Interface Structure
//! \return   MOS_STATUS
//!
MOS_STATUS RenderHal_DSH_Reset(
    PRENDERHAL_INTERFACE pRenderHal)
{
    MOS_STATUS eStatus;

    //----------------------------------
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    //----------------------------------

    eStatus         = MOS_STATUS_SUCCESS;

    // Do not register GSH resource until we allocate the Media State
    // Media State can be in any of the current DSH instances

    // Do not register ISH resource until we load all kernels
    // ISH could be reallocated to accomodate kernels being loaded in heap

    //MHW_RENDERHAL_CHK_STATUS(pOsInterface->pfnRegisterResource(pOsInterface,
    //                                        &pStateHeap->IshOsResource,
    //                                        true,
    //                                        true));

    // Reset Slice Shutdown Mode
    pRenderHal->bRequestSingleSlice   = false;
    pRenderHal->PowerOption.nSlice    = 0;
    pRenderHal->PowerOption.nEU       = 0;
    pRenderHal->PowerOption.nSubSlice = 0;

finish:
    return eStatus;
}

//!
//! \brief    Send Sync Tag
//! \details  Sends Synchronization Tags for G75
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to RenderHal Interface Structure
//! \param    PMOS_COMMAND_BUFFER pCmdBuffer
//!           [in] Pointer to Command Buffer
//! \return   MOS_STATUS
//!
MOS_STATUS RenderHal_DSH_SendSyncTag(
    PRENDERHAL_INTERFACE    pRenderHal,
    PMOS_COMMAND_BUFFER     pCmdBuffer)
{
    PRENDERHAL_STATE_HEAP           pStateHeap;
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;
    PXMHW_STATE_HEAP_INTERFACE      pMhwStateHeap;
    PMHW_MI_INTERFACE               pMhwMiInterface;
    MHW_PIPE_CONTROL_PARAMS         PipeCtl;

    //-------------------------------------
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pMhwStateHeap);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pStateHeap);
    //-------------------------------------

    pStateHeap      = pRenderHal->pStateHeap;
    pMhwStateHeap   = pRenderHal->pMhwStateHeap;
    pMhwMiInterface = pRenderHal->pMhwMiInterface;

    // Send PIPE_CONTROL Token
    // CMD_MI_FLUSH is disabled by default on GT, use PIPE_CONTROL
    // Requires a token and the actual pipe control command
    // Flush write caches
    PipeCtl = g_cRenderHal_InitPipeControlParams;
    PipeCtl.presDest          = pMhwStateHeap->GetResCmdBufIdGlobal();
    PipeCtl.dwPostSyncOp      = MHW_FLUSH_NOWRITE;
    PipeCtl.dwFlushMode       = MHW_FLUSH_WRITE_CACHE;
    MHW_RENDERHAL_CHK_STATUS(pMhwMiInterface->AddPipeControl(pCmdBuffer, nullptr, &PipeCtl));

    // Invalidate read-only caches and perform a post sync write
    PipeCtl = g_cRenderHal_InitPipeControlParams;
    PipeCtl.presDest          = pMhwStateHeap->GetResCmdBufIdGlobal();
    PipeCtl.dwResourceOffset  = pStateHeap->dwOffsetSync;
    PipeCtl.dwPostSyncOp      = MHW_FLUSH_WRITE_IMMEDIATE_DATA;
    PipeCtl.dwFlushMode       = MHW_FLUSH_READ_CACHE;
    PipeCtl.dwDataDW1         = pStateHeap->dwNextTag;
    MHW_RENDERHAL_CHK_STATUS(pMhwMiInterface->AddPipeControl(pCmdBuffer, nullptr, &PipeCtl));

finish:
    return eStatus;
}

//!
//! \brief      Sets Sampler States
//! \details    Initialize and set sampler states
//! \param      PRENDERHAL_INTERFACE pRenderHal
//!             [in]    Pointer to HW interface
//! \param      int32_t iMediaID
//!             [in]    Media ID
//! \param      PRENDERHAL_SAMPLER_STATE_PARAMS pSamplerParams
//!             [in]    Pointer to Sampler parameters
//! \param      int32_t iSamplers
//!             [in]    Number of samplers
//! \return     MOS_STATUS MOS_STATUS_SUCCESS if success, otherwise MOS_STATUS_UNKNOWN
//!
MOS_STATUS RenderHal_DSH_SetSamplerStates(
    PRENDERHAL_INTERFACE        pRenderHal,
    int32_t                     iMediaID,
    PMHW_SAMPLER_STATE_PARAM    pSamplerParams,
    int32_t                     iSamplers)
{
    MHW_RENDERHAL_ASSERT(true);
    return MOS_STATUS_UNIMPLEMENTED;
}

//!
//! \brief      Setup Interface Descriptor
//! \details    Set interface descriptor
//! \param      PRENDERHAL_INTERFACE                    pRenderHal
//!             [in]    Pointer to HW interface
//! \param      PRENDERHAL_MEDIA_STATE                  pMediaState
//!             [in]    Pointer to media state
//! \param      PRENDERHAL_KRN_ALLOCATION               pKernelAllocation
//!             [in]    Pointer to kernel allocation
//! \param      PRENDERHAL_INTERFACE_DESCRIPTOR_PARAMS  pInterfaceDescriptorParams
//!             [in]    Pointer to interface descriptor parameters
//! \param      PMHW_GPGPU_WALKER_PARAMS          pGpGpuWalkerParams
//!             [in]    Pointer to gpgpu walker parameters
//! \return     MOS_STATUS
//!
MOS_STATUS RenderHal_DSH_SetupInterfaceDescriptor(
    PRENDERHAL_INTERFACE                   pRenderHal,
    PRENDERHAL_MEDIA_STATE                 pMediaState,
    PRENDERHAL_KRN_ALLOCATION              pKernelAllocation,
    PRENDERHAL_INTERFACE_DESCRIPTOR_PARAMS pInterfaceDescriptorParams)
{
    MOS_STATUS               eStatus = MOS_STATUS_SUCCESS;
    MHW_ID_ENTRY_PARAMS      Params;
    PRENDERHAL_STATE_HEAP    pStateHeap;
    PRENDERHAL_DYNAMIC_STATE pDynamicState;
    uint32_t                 dwMediaStateOffset;

    //-----------------------------------------
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pMhwStateHeap);
    MHW_RENDERHAL_CHK_NULL(pMediaState);
    MHW_RENDERHAL_CHK_NULL(pMediaState->pDynamicState);
    MHW_RENDERHAL_CHK_NULL(pKernelAllocation);
    MHW_RENDERHAL_CHK_NULL(pKernelAllocation->pMemoryBlock);
    MHW_RENDERHAL_CHK_NULL(pInterfaceDescriptorParams);
    //-----------------------------------------

    // Get states, params
    pStateHeap = pRenderHal->pStateHeap;
    pDynamicState = pMediaState->pDynamicState;
    dwMediaStateOffset = pDynamicState->memoryBlock.GetOffset();

    Params.dwMediaIdOffset = dwMediaStateOffset + pDynamicState->MediaID.dwOffset;
    Params.iMediaId = pInterfaceDescriptorParams->iMediaID;
    Params.dwKernelOffset = pKernelAllocation->dwOffset;
    Params.dwSamplerOffset = dwMediaStateOffset + pDynamicState->Sampler3D.dwOffset +
        pInterfaceDescriptorParams->iMediaID * pDynamicState->dwSizeSamplers;
    Params.dwSamplerCount = pKernelAllocation->Params.Sampler_Count;
    Params.dwBindingTableOffset = pInterfaceDescriptorParams->iBindingTableID * pStateHeap->iBindingTableSize;
    Params.iCurbeOffset = pInterfaceDescriptorParams->iCurbeOffset;
    Params.iCurbeLength = pInterfaceDescriptorParams->iCurbeLength;

    Params.bBarrierEnable = pInterfaceDescriptorParams->blBarrierEnable;
    Params.bGlobalBarrierEnable = pInterfaceDescriptorParams->blGlobalBarrierEnable;    //It's only applied for BDW+
    Params.dwNumberofThreadsInGPGPUGroup = pInterfaceDescriptorParams->iNumberThreadsInGroup;
    Params.dwSharedLocalMemorySize = pRenderHal->pfnEncodeSLMSize(pRenderHal, pInterfaceDescriptorParams->iSLMSize);
    Params.iCrsThdConDataRdLn = pInterfaceDescriptorParams->iCrsThrdConstDataLn;
    Params.memoryBlock = &pDynamicState->memoryBlock;

    MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwStateHeap->AddInterfaceDescriptorData(&Params));
    pDynamicState->MediaID.iCurrent++;

finish:
    return eStatus;
}

//!
//! \brief    Set Vfe State Params
//! \details  Sets VFE State parameters
//!           this functions must be called to setup
//!           parameters for pMhwRender->AddMediaVfeCmd()
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to Hardware Interface Structure
//! \param    uint32_t dwDebugCounterControl
//!           [in] Debug Counter Control
//! \param    uint32_t dwMaximumNumberofThreads
//!           [in] Maximum Number of Threads
//! \param    uint32_t dwCURBEAllocationSize
//!           [in] CURBE Allocation Size
//! \param    uint32_t dwURBEntryAllocationSize
//!           [in] URB Entry Allocation Size
//! \param    PRENDERHAL_SCOREBOARD_PARAMS pScoreboardParams
//!           [in] Pointer to Scoreboard Params
//! \return   MOS_STATUS
//!
MOS_STATUS RenderHal_DSH_SetVfeStateParams(
    PRENDERHAL_INTERFACE    pRenderHal,
    uint32_t                dwDebugCounterControl,
    uint32_t                dwMaximumNumberofThreads,
    uint32_t                dwCURBEAllocationSize,
    uint32_t                dwURBEntryAllocationSize,
    PMHW_VFE_SCOREBOARD     pScoreboardParams)
{
    PMHW_VFE_PARAMS                 pVfeParams;
    PRENDERHAL_STATE_HEAP           pStateHeap;
    PRENDERHAL_DYNAMIC_STATE        pDynamicState;
    PMHW_RENDER_ENGINE_CAPS         pHwCaps;
    PRENDERHAL_STATE_HEAP_SETTINGS  pSettings;
    uint32_t                        dwMaxURBSize;
    uint32_t                        dwMaxCURBEAllocationSize;
    uint32_t                        dwMaxURBEntryAllocationSize;
    uint32_t                        dwNumberofURBEntries;
    uint32_t                        dwMaxURBEntries;
    uint32_t                        dwMaxInterfaceDescriptorEntries;
    MOS_STATUS                      eStatus;
    uint32_t i;

    //---------------------------------------------
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pWaTable);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pStateHeap);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pHwCaps);
    //---------------------------------------------

    eStatus = MOS_STATUS_SUCCESS;
    pStateHeap = pRenderHal->pStateHeap;
    pHwCaps = pRenderHal->pHwCaps;
    pVfeParams = pRenderHal->pRenderHalPltInterface->GetVfeStateParameters();
    pSettings = &(pRenderHal->StateHeapSettings);

    pVfeParams->pKernelState = nullptr;
    pVfeParams->eVfeSliceDisable = MHW_VFE_SLICE_ALL;

    // Get pointer to current dynamic state
    MHW_RENDERHAL_CHK_NULL(pStateHeap->pCurMediaState);
    MHW_RENDERHAL_CHK_NULL(pStateHeap->pCurMediaState->pDynamicState);
    pDynamicState = pStateHeap->pCurMediaState->pDynamicState;

    //-------------------------------------------------------------------------
    // In this calculation, URBEntryAllocationSize and CURBEAllocationSize are
    // in 256-bit units:
    // (URBEntryAllocationSize * NumberofURBEntries + CURBEAllocationSize +
    //  MaxInterfaceDescriptorEntries) <= 2048
    //-------------------------------------------------------------------------

    // get the Max for all the fields
    dwMaxURBSize = pHwCaps->dwMaxURBSize;
    dwMaxURBEntries = pHwCaps->dwMaxURBEntries;
    dwMaxURBEntryAllocationSize = pHwCaps->dwMaxURBEntryAllocationSize;
    dwMaxCURBEAllocationSize = pHwCaps->dwMaxCURBEAllocationSize;
    dwMaxInterfaceDescriptorEntries = pHwCaps->dwMaxInterfaceDescriptorEntries;

    dwCURBEAllocationSize = MOS_MAX(dwCURBEAllocationSize, (uint32_t)pDynamicState->Curbe.iCurrent);
    dwCURBEAllocationSize = MOS_ROUNDUP_SHIFT(dwCURBEAllocationSize, 5);
    dwURBEntryAllocationSize = MOS_ROUNDUP_SHIFT(dwURBEntryAllocationSize, 5);
    dwURBEntryAllocationSize = MOS_MAX(1, dwURBEntryAllocationSize);
    dwNumberofURBEntries = (dwMaxURBSize - dwCURBEAllocationSize - dwMaxInterfaceDescriptorEntries) / dwURBEntryAllocationSize;
    dwNumberofURBEntries = MOS_CLAMP_MIN_MAX(dwNumberofURBEntries, 1, 32);

    pVfeParams->dwDebugCounterControl = dwDebugCounterControl;
    pVfeParams->dwNumberofURBEntries = dwNumberofURBEntries;
    pVfeParams->dwMaximumNumberofThreads = (dwMaximumNumberofThreads == RENDERHAL_USE_MEDIA_THREADS_MAX) ?
        pHwCaps->dwMaxThreads : MOS_MIN(dwMaximumNumberofThreads, pHwCaps->dwMaxThreads);
    pVfeParams->dwCURBEAllocationSize = dwCURBEAllocationSize << 5;
    pVfeParams->dwURBEntryAllocationSize = dwURBEntryAllocationSize;

    MHW_RENDERHAL_ASSERT(dwNumberofURBEntries <= dwMaxURBEntries);
    MHW_RENDERHAL_ASSERT(dwCURBEAllocationSize <= dwMaxCURBEAllocationSize);
    MHW_RENDERHAL_ASSERT(dwURBEntryAllocationSize <= dwMaxURBEntryAllocationSize);
    MHW_RENDERHAL_ASSERT(dwNumberofURBEntries * dwURBEntryAllocationSize + dwCURBEAllocationSize + dwMaxInterfaceDescriptorEntries <= dwMaxURBSize);

    // Setup Scoreboard Parameters
    if (pScoreboardParams)
    {
        MHW_RENDERHAL_ASSERT(pScoreboardParams->ScoreboardMask < 8);

        pRenderHal->VfeScoreboard.ScoreboardEnable = true;
        pRenderHal->VfeScoreboard.ScoreboardMask = (1 << pScoreboardParams->ScoreboardMask) - 1;
        pRenderHal->VfeScoreboard.ScoreboardType = pScoreboardParams->ScoreboardType;
        for (i = 0; i < pScoreboardParams->ScoreboardMask; i++)
        {
            pRenderHal->VfeScoreboard.ScoreboardDelta[i].x = pScoreboardParams->ScoreboardDelta[i].x;
            pRenderHal->VfeScoreboard.ScoreboardDelta[i].y = pScoreboardParams->ScoreboardDelta[i].y;
        }
    }
    else
    {
        pRenderHal->VfeScoreboard.ScoreboardEnable = true;
        pRenderHal->VfeScoreboard.ScoreboardMask = 0x0;
    }

    // Setup VFE Scoreboard parameters
    pVfeParams->Scoreboard = pRenderHal->VfeScoreboard;

    // Setup Kernel Scratch Space
    if (pDynamicState->dwScratchSpace > 0)
    {
        int32_t iSize;
        int32_t iRemain;
        int32_t iPerThreadScratchSize;

        MHW_RENDERHAL_ASSERT(pDynamicState->iMaxScratchSpacePerThread ==
            MOS_ALIGN_CEIL(pDynamicState->iMaxScratchSpacePerThread, 1024));

        if (pRenderHal->pfnPerThreadScratchSpaceStart2K(pRenderHal))
            iPerThreadScratchSize = pDynamicState->iMaxScratchSpacePerThread >> 10;
        else
            iPerThreadScratchSize = pDynamicState->iMaxScratchSpacePerThread >> 9;

        iRemain = iPerThreadScratchSize % 2;
        iPerThreadScratchSize = iPerThreadScratchSize / 2;
        iSize = 0;
        while (!iRemain && (iPerThreadScratchSize / 2))
        {
            iSize++;
            iRemain = iPerThreadScratchSize % 2;
            iPerThreadScratchSize = iPerThreadScratchSize / 2;
        }

        MHW_RENDERHAL_ASSERT(!iRemain && iPerThreadScratchSize);
        MHW_RENDERHAL_ASSERT(iSize < 12);

        pVfeParams->dwPerThreadScratchSpace   = (uint32_t)iSize;
        pVfeParams->dwScratchSpaceBasePointer = pDynamicState->scratchSpaceOffset;
    }
    else
    {
        pVfeParams->dwPerThreadScratchSpace = 0;
        pVfeParams->dwScratchSpaceBasePointer = 0;
    }

finish:
    return eStatus;
}

//!
//! \brief    Get offset and/or pointer to sampler state
//! \details  Get offset and/or pointer to sampler state in General State Heap
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to RenderHal Interface
//! \param    int32_t iMediaID
//!           [in] Media ID associated with sampler
//! \param    int32_t iSamplerID
//!           [in] Sampler ID
//! \param    uint32_t *pdwSamplerOffset
//!           [out] optional; offset of sampler state from GSH base
//! \param    void  **ppSampler
//!           [out] optional; pointer to sampler state in GSH
//! \return   MOS_STATUS
//!
MOS_STATUS RenderHal_DSH_GetSamplerOffsetAndPtr(
    PRENDERHAL_INTERFACE     pRenderHal,
    int32_t                  iMediaID,
    int32_t                  iSamplerID,
    PMHW_SAMPLER_STATE_PARAM pSamplerParams,
    uint32_t                 *pdwSamplerOffset,
    void                    **ppSampler)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pRenderHalPltInterface);

    MHW_RENDERHAL_CHK_STATUS( pRenderHal->pRenderHalPltInterface->GetSamplerOffsetAndPtr_DSH(
                                                    pRenderHal,
                                                    iMediaID,
                                                    iSamplerID,
                                                    pSamplerParams,
                                                    pdwSamplerOffset,
                                                    ppSampler));
finish:
    return eStatus;
}

//!
//! \brief    Initialize
//! \details  Initialize HW states
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to RenderHal Interface Structure
//! \param    PCRENDERHAL_SETTINGS pSettings
//!           [in] Pointer to Settings
//! \return   MOS_STATUS
//!           true  if succeeded
//!           false if failed to allocate/initialize HW commands
//!
MOS_STATUS RenderHal_DSH_Initialize(
    PRENDERHAL_INTERFACE   pRenderHal,
    PRENDERHAL_SETTINGS    pSettings)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    PRENDERHAL_STATE_HEAP pStateHeap;

    //------------------------------------------------
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    //------------------------------------------------

    // Allocate State Heap control structure (aligned)
    pRenderHal->pStateHeap = pStateHeap = (PRENDERHAL_STATE_HEAP)MOS_AlignedAllocMemory(sizeof(RENDERHAL_STATE_HEAP), 16);
    pRenderHal->dwStateHeapSize = sizeof(RENDERHAL_STATE_HEAP);
    MHW_RENDERHAL_CHK_NULL(pStateHeap);

    pRenderHal->pStateHeap->kernelHashTable = CmHashTable();
    pRenderHal->pStateHeap->kernelHashTable.Init();

    // Apply state heap settings (iMediaStates not actually used in DSH)
    if (pSettings)
    {
        pRenderHal->StateHeapSettings.iMediaStateHeaps = pSettings->iMediaStates;
    }

    // Apply Dynamic state heap settings
    if (pSettings && pSettings->pDynSettings)
    {
        pRenderHal->DynamicHeapSettings = *(pSettings->pDynSettings);
    }

    // Apply SSH settings for the current platform
    pRenderHal->StateHeapSettings.iSurfaceStateHeaps =
                                pRenderHal->StateHeapSettings.iMediaStateHeaps;

    // Allocate and initialize state heaps (GSH, SSH, ISH)
    MHW_RENDERHAL_CHK_STATUS(pRenderHal->pfnAllocateStateHeaps(pRenderHal, &pRenderHal->StateHeapSettings));

    // If ASM debug is enabled, allocate debug resource
    MHW_RENDERHAL_CHK_STATUS(RenderHal_AllocateDebugSurface(pRenderHal));

finish:
    return eStatus;
}

//!
//! \brief    Issue command to write timestamp
//! \param    [in] pRenderHal
//! \param    [in] pCmdBuffer
//! \param    [in] bStartTime
//! \return   MOS_STATUS
//!
MOS_STATUS RenderHal_DSH_SendTimingData(
    PRENDERHAL_INTERFACE         pRenderHal,
    PMOS_COMMAND_BUFFER          pCmdBuffer,
    bool                         bStartTime);

//! Following functions are defined in RenderHal and are not used by RenderHal_DSH
//! ------------------------------------------------------------------------------
PRENDERHAL_MEDIA_STATE RenderHal_DSH_AssignMediaState(
    PRENDERHAL_INTERFACE     pRenderHal,
    RENDERHAL_COMPONENT      componentID)
{
    MHW_RENDERHAL_ASSERT( true );
    return nullptr;
}

int32_t RenderHal_DSH_AllocateMediaID(
    PRENDERHAL_INTERFACE        pRenderHal,
    int32_t                     iKernelAllocationID,
    int32_t                     iBindingTableID,
    int32_t                     iCurbeOffset,
    int32_t                     iCurbeLength,
    int32_t                     iCrsThrdConstDataLn,
    PMHW_GPGPU_WALKER_PARAMS    pGpGpuWalkerParams)
{
    MHW_RENDERHAL_ASSERT( true );
    return -1;
}

int32_t RenderHal_DSH_LoadKernel(
    PRENDERHAL_INTERFACE        pRenderHal,
    PCRENDERHAL_KERNEL_PARAM    pParameters,
    PMHW_KERNEL_PARAM           pKernel,
    Kdll_CacheEntry             *pKernelEntry)
{
    MHW_RENDERHAL_ASSERT( true );
    return -1;
}

MOS_STATUS RenderHal_DSH_UnloadKernel(
    PRENDERHAL_INTERFACE        pRenderHal,
    int32_t                     iKernelAllocationID)
{
    MHW_RENDERHAL_ASSERT( true );
    return MOS_STATUS_UNIMPLEMENTED;
}

void RenderHal_DSH_ResetKernels(
    PRENDERHAL_INTERFACE        pRenderHal)
{
    MHW_RENDERHAL_ASSERT( true );
    return;
}

void RenderHal_DSH_TouchKernel(
    PRENDERHAL_INTERFACE        pRenderHal,
    int32_t                     iKernelAllocationID)
{
    MHW_RENDERHAL_ASSERT( true );
    return;
}

int32_t RenderHal_DSH_GetKernelOffset(
    PRENDERHAL_INTERFACE        pRenderHal,
    int32_t                     iKernelAllocationIndex)
{
    MHW_RENDERHAL_ASSERT( true );
    return 0;
}
//! ------------------------------------------------------------------------------

void RenderHal_InitInterfaceEx(PRENDERHAL_INTERFACE pRenderHal);

//!
//! \brief    Init Interface using Dynamic State Heap
//! \details  Initializes RenderHal Interface structure, responsible for HW
//!           abstraction of HW Rendering Engine for CM(MDF) and VP.
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to RenderHal Interface Structure
//! \param    MhwCpInterface** ppCpInterface
//!           [in/out] Pointer of pointer to MHW CP Interface Structure, which 
//!           is created during renderhal initialization
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS Interface Structure
//! \return   MOS_STATUS
//!           MOS_STATUS_UNKNOWN : Invalid parameters
//!
MOS_STATUS RenderHal_InitInterface_Dynamic(
    PRENDERHAL_INTERFACE pRenderHal,
    MhwCpInterface       **ppCpInterface,
    PMOS_INTERFACE       pOsInterface)
{
    MOS_STATUS eStatus;

    //---------------------------------------
    MHW_RENDERHAL_ASSERT(pRenderHal);
    //---------------------------------------
    // Init interfaces - some of which will be later replaced for DSH
    MHW_RENDERHAL_CHK_STATUS(RenderHal_InitInterface(pRenderHal, ppCpInterface, pOsInterface));

    // Initialization/Cleanup function
    pRenderHal->pfnInitialize                 = RenderHal_DSH_Initialize;

    // Allocate/Destroy state heaps
    pRenderHal->pfnAllocateStateHeaps         = RenderHal_DSH_AllocateStateHeaps;
    pRenderHal->pfnFreeStateHeaps             = RenderHal_DSH_FreeStateHeaps;

    pRenderHal->pfnAssignSpaceInStateHeap     = RenderHal_DSH_AssignSpaceInStateHeap;

    // Media states management functions
    pRenderHal->pfnAssignMediaState           = RenderHal_DSH_AssignMediaState;
    pRenderHal->pfnAllocateMediaID            = RenderHal_DSH_AllocateMediaID;
    pRenderHal->pfnGetMediaID                 = RenderHal_DSH_GetMediaID;
    pRenderHal->pfnAssignDynamicState         = RenderHal_DSH_AssignDynamicState;
    pRenderHal->pfnReleaseDynamicState        = RenderHal_DSH_ReleaseDynamicState;
    pRenderHal->pfnSubmitDynamicState         = RenderHal_DSH_SubmitDynamicState;
    pRenderHal->pfnAllocateDynamicMediaID     = RenderHal_DSH_AllocateDynamicMediaID;

    // Kernel management functions
    pRenderHal->pfnRefreshSync                = RenderHal_DSH_RefreshSync;
    pRenderHal->pfnLoadKernel                 = RenderHal_DSH_LoadKernel;
    pRenderHal->pfnUnloadKernel               = RenderHal_DSH_UnloadKernel;
    pRenderHal->pfnResetKernels               = RenderHal_DSH_ResetKernels;
    pRenderHal->pfnTouchKernel                = RenderHal_DSH_TouchKernel;
    pRenderHal->pfnGetKernelOffset            = RenderHal_DSH_GetKernelOffset;
    pRenderHal->pfnUnregisterKernel           = RenderHal_DSH_UnregisterKernel;

    // Dynamic Kernel management functions (not implemented here)
    pRenderHal->pfnLoadDynamicKernel          = RenderHal_DSH_LoadDynamicKernel;
    pRenderHal->pfnAllocateDynamicKernel      = RenderHal_DSH_AllocateDynamicKernel;
    pRenderHal->pfnSearchDynamicKernel        = RenderHal_DSH_SearchDynamicKernel;
    pRenderHal->pfnUnloadDynamicKernel        = RenderHal_DSH_UnloadDynamicKernel;
    pRenderHal->pfnRefreshDynamicKernels      = RenderHal_DSH_RefreshDynamicKernels;
    pRenderHal->pfnResetDynamicKernels        = RenderHal_DSH_ResetDynamicKernels;
    pRenderHal->pfnTouchDynamicKernel         = RenderHal_DSH_TouchDynamicKernel;
    pRenderHal->pfnExpandKernelStateHeap      = RenderHal_DSH_ExpandKernelStateHeap;

    // ISA ASM Debug support functions
    pRenderHal->pfnLoadSipKernel              = RenderHal_DSH_LoadSipKernel;
    pRenderHal->pfnSendSipStateCmd            = RenderHal_DSH_SendSipStateCmd;

    // Command buffer programming functions
    pRenderHal->pfnLoadCurbeData              = RenderHal_DSH_LoadCurbeData;
    pRenderHal->pfnSendCurbeLoad              = RenderHal_DSH_SendCurbeLoad;
    pRenderHal->pfnSendStateBaseAddress       = RenderHal_DSH_SendStateBaseAddress;

    // Initialize OS dependent RenderHal Interfaces common to all platforms
    pRenderHal->pfnReset                      = RenderHal_DSH_Reset;
    pRenderHal->pfnSendTimingData             = RenderHal_DSH_SendTimingData;
    pRenderHal->pfnSendSyncTag                = RenderHal_DSH_SendSyncTag;

    // Sampler state, interface descriptor, VFE params
    pRenderHal->pfnSetSamplerStates           = RenderHal_DSH_SetSamplerStates;
    pRenderHal->pfnSetupInterfaceDescriptor   = RenderHal_DSH_SetupInterfaceDescriptor;
    pRenderHal->pfnSetVfeStateParams          = RenderHal_DSH_SetVfeStateParams;

    // Hardware dependent calls
    pRenderHal->pfnGetSamplerOffsetAndPtr     = RenderHal_DSH_GetSamplerOffsetAndPtr;

    // Special functions
    RenderHal_InitInterfaceEx(pRenderHal);

    // Initialize the DSH settings
    pRenderHal->pRenderHalPltInterface->InitDynamicHeapSettings(pRenderHal);

finish:
    return eStatus;
}
