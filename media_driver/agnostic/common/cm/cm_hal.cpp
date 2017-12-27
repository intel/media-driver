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
//! \file      cm_hal.cpp
//! \brief     HAL Layer for CM Component
//!
#include "mos_os.h"
#include "cm_hal.h"
#include "media_interfaces_cmhal.h"
#include "media_interfaces_mhw.h"
#include "cm_common.h"
#include "cm_hal_vebox.h"
#include "cm_mem.h"
#include "renderhal_platform_interface.h"

#define INDEX_ALIGN(index, elemperIndex, base) ((index * elemperIndex)/base + ( (index *elemperIndex % base))? 1:0)

//----------------------------------
//| CM scoreboard XY
//----------------------------------
struct CM_HAL_SCOREBOARD_XY
{
    int32_t x;
    int32_t y;
};
typedef CM_HAL_SCOREBOARD_XY *PCM_HAL_SCOREBOARD_XY;

//---------------------------------------
//| CM scoreboard XY with mask
//---------------------------------------
struct CM_HAL_SCOREBOARD_XY_MASK
{
    int32_t x;
    int32_t y;
    uint8_t mask;
    uint8_t resetMask;
};
typedef CM_HAL_SCOREBOARD_XY_MASK *PCM_HAL_SCOREBOARD_XY_MASK;

//------------------------------------------------------------------------------
//| CM kernel slice and subslice being assigned to (for EnqueueWithHints)
//------------------------------------------------------------------------------
struct CM_HAL_KERNEL_SLICE_SUBSLICE
{
    uint32_t slice;
    uint32_t subSlice;
};
typedef CM_HAL_KERNEL_SLICE_SUBSLICE *PCM_HAL_KERNEL_SLICE_SUBSLICE;

//------------------------------------------------------------------------------
//| CM kernel information for EnqueueWithHints to assign subslice
//------------------------------------------------------------------------------
struct CM_HAL_KERNEL_SUBSLICE_INFO
{
    uint32_t numSubSlices;
    uint32_t counter;
    PCM_HAL_KERNEL_SLICE_SUBSLICE  pDestination;
};
typedef CM_HAL_KERNEL_SUBSLICE_INFO *PCM_HAL_KERNEL_SUBSLICE_INFO;





// forward declaration
int32_t HalCm_InsertCloneKernel(
    PCM_HAL_STATE              pState,
    PCM_HAL_KERNEL_PARAM       pKernelParam,
    PRENDERHAL_KRN_ALLOCATION  &pKernelAllocation);

extern MOS_STATUS HalCm_GetSipBinary(
    PCM_HAL_STATE   pState);

#if MDF_COMMAND_BUFFER_DUMP
extern int32_t HalCm_InitDumpCommandBuffer(PCM_HAL_STATE pState);

extern int32_t HalCm_DumpCommadBuffer(PCM_HAL_STATE pState, PMOS_COMMAND_BUFFER pCmdBuffer,
               int offsetSurfaceState, size_t sizeOfSurfaceState);
#endif

#if MDF_CURBE_DATA_DUMP
extern int32_t HalCm_InitDumpCurbeData(PCM_HAL_STATE pState);

extern int32_t HalCm_DumpCurbeData(PCM_HAL_STATE pState);
#endif

#if MDF_SURFACE_CONTENT_DUMP
extern int32_t HalCm_InitSurfaceDump(PCM_HAL_STATE pState);
#endif

//===============<Private Functions>============================================
//*-----------------------------------------------------------------------------
//| Purpose:    Align to the next power of 2
//| Returns:    Aligned data
//| Reference:  http://graphics.stanford.edu/~seander/bithacks.html#DetermineIfPowerOf2
//*-----------------------------------------------------------------------------
__inline uint32_t HalCm_GetPow2Aligned(uint32_t d)
{
    CM_ASSERT(d > 0);

    // subtract the number first
    --d;

    d |= d >> 1;
    d |= d >> 2;
    d |= d >> 4;
    d |= d >> 8;
    d |= d >> 16;

    return ++d;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Checks if Task has any thread arguments
//| Returns:    True if task has any thread arguments, false otherwise
//*-----------------------------------------------------------------------------
bool HalCm_GetTaskHasThreadArg(PCM_HAL_KERNEL_PARAM *pKernels, uint32_t numKernels)
{
    PCM_HAL_KERNEL_PARAM            pKernelParam;
    PCM_HAL_KERNEL_ARG_PARAM        pArgParam;
    bool                            threadArgExists = false;
    
    for( uint32_t iKrn = 0; iKrn < numKernels; iKrn++)
    {
        pKernelParam    = pKernels[iKrn];
        for(uint32_t argIndex = 0; argIndex < pKernelParam->iNumArgs; argIndex++)
        {
            pArgParam = &pKernelParam->CmArgParams[argIndex];
            if( pArgParam->bPerThread )
            {
                threadArgExists = true;
                break;
            }
        }

        if( threadArgExists )
            break;
    }

    return threadArgExists;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Allocate Timestamp Resource
//| Returns:    Result of the operation
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_AllocateTsResource(
    PCM_HAL_STATE pState)                                                       // [in] Pointer to CM HAL State
{
    MOS_STATUS              hr;
    uint32_t                iSize;
    PMOS_INTERFACE          pOsInterface;
    MOS_ALLOC_GFXRES_PARAMS AllocParams;
    MOS_LOCK_PARAMS         LockFlags;

    hr              = MOS_STATUS_SUCCESS;
    pOsInterface    = pState->pOsInterface;

    iSize = ((sizeof(uint64_t)* CM_SYNC_QWORD_PER_TASK) + (sizeof(uint64_t)* CM_FRAME_TRACKING_QWORD_PER_TASK)) * pState->CmDeviceParam.iMaxTasks;    // 2 QWORDs for each kernel in the task

    MOS_ZeroMemory(&AllocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    AllocParams.Type    = MOS_GFXRES_BUFFER;
    AllocParams.dwBytes = iSize;
    AllocParams.Format  = Format_Buffer;  //used in RenderHal_OsAllocateResource_Linux
    AllocParams.TileType= MOS_TILE_LINEAR;
    AllocParams.pBufName = "TsResource";

    CM_HRESULT2MOSSTATUS_AND_CHECK(pOsInterface->pfnAllocateResource(
        pOsInterface,
        &AllocParams,
        &pState->TsResource.OsResource));

    // Lock the Resource
    MOS_ZeroMemory(&LockFlags, sizeof(MOS_LOCK_PARAMS));

    LockFlags.ReadOnly = 1;
    LockFlags.ForceCached = true;   

    pState->TsResource.pData = (uint8_t*)pOsInterface->pfnLockResource(
                                        pOsInterface, 
                                        &pState->TsResource.OsResource, 
                                        &LockFlags);

    CM_CHK_NULL_RETURN_MOSSTATUS(pState->TsResource.pData);

    pState->TsResource.bLocked  = true;

finish:
    return hr;
}


//*-----------------------------------------------------------------------------
//| Purpose:    Free Timestamp Resource
//| Returns:    Result of the operation
//*-----------------------------------------------------------------------------
__inline void HalCm_FreeTsResource(
    PCM_HAL_STATE pState)                                                       // [in] Pointer to CM HAL State
{
    PMOS_INTERFACE      pOsInterface;
    MOS_STATUS          hr;

    pOsInterface    = pState->pOsInterface;

    if (!Mos_ResourceIsNull(&pState->TsResource.OsResource))
    {
        if (pState->TsResource.bLocked)
        {
            hr = (MOS_STATUS)pOsInterface->pfnUnlockResource(
                    pOsInterface, 
                    &pState->TsResource.OsResource);

            CM_ASSERT(hr == MOS_STATUS_SUCCESS);
        }

        pOsInterface->pfnFreeResourceWithFlag(
            pOsInterface,
            &pState->TsResource.OsResource,
            SURFACE_FLAG_ASSUME_NOT_IN_USE);
    }
}

//*-----------------------------------------------------------------------------
//| Purpose:    Allocate CSR Resource
//| Returns:    Result of the operation
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_AllocateCSRResource(
    PCM_HAL_STATE pState)       // [in] Pointer to CM HAL State
{
    PMOS_INTERFACE          pOsInterface = pState->pOsInterface;
    MOS_STATUS              hr = MOS_STATUS_SUCCESS;
    uint32_t                iSize;
    MOS_ALLOC_GFXRES_PARAMS AllocParams;

    //Enable Mid-thread
    pState->pRenderHal->pfnEnableGpgpuMiddleThreadPreemption(pState->pRenderHal);

    iSize = CM_CSR_SURFACE_SIZE;

    MOS_ZeroMemory(&AllocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    AllocParams.Type = MOS_GFXRES_BUFFER;
    AllocParams.dwBytes = iSize;
    AllocParams.Format = Format_RAW;  //used in VpHal_OsAllocateResource_Linux
    AllocParams.TileType = MOS_TILE_LINEAR;
    AllocParams.pBufName = "CSRResource";

    CM_HRESULT2MOSSTATUS_AND_CHECK(pOsInterface->pfnAllocateResource(
        pOsInterface,
        &AllocParams,
        &pState->CSRResource));

finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Allocate Sip Resource
//| Returns:    Result of the operation
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_AllocateSipResource(
    PCM_HAL_STATE pState)       // [in] Pointer to CM HAL State
{
    PMOS_INTERFACE          pOsInterface = pState->pOsInterface;
    MOS_STATUS              hr = MOS_STATUS_SUCCESS;
    uint32_t                iSize;
    MOS_ALLOC_GFXRES_PARAMS AllocParams;
    MOS_LOCK_PARAMS         LockFlags;

    iSize = CM_DEBUG_SURFACE_SIZE;

    MOS_ZeroMemory(&AllocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    AllocParams.Type = MOS_GFXRES_BUFFER;
    AllocParams.dwBytes = iSize;
    AllocParams.Format = Format_Buffer;  //used in RenderHal_OsAllocateResource_Linux
    AllocParams.TileType = MOS_TILE_LINEAR;
    AllocParams.pBufName = "SipResource";

    CM_HRESULT2MOSSTATUS_AND_CHECK(pOsInterface->pfnAllocateResource(
        pOsInterface,
        &AllocParams,
        &pState->SipResource.OsResource));

    // Lock the Resource
    MOS_ZeroMemory(&LockFlags, sizeof(MOS_LOCK_PARAMS));

    LockFlags.ReadOnly = 1;
    LockFlags.ForceCached = true;

    pState->SipResource.pData = (uint8_t*)pOsInterface->pfnLockResource(
        pOsInterface,
        &pState->SipResource.OsResource,
        &LockFlags);
    CM_CHK_NULL_RETURN_MOSSTATUS(pState->SipResource.pData);

    pState->SipResource.bLocked = true;

finish:
    return hr;
}


//*-----------------------------------------------------------------------------
//| Purpose:    Free CSR Resource
//| Returns:    Result of the operation
//*-----------------------------------------------------------------------------
__inline void HalCm_FreeCsrResource(
    PCM_HAL_STATE pState)   // [in] Pointer to CM HAL State
{
    PMOS_INTERFACE      pOsInterface = pState->pOsInterface;

    if (!Mos_ResourceIsNull(&pState->CSRResource))
    {
        pOsInterface->pfnFreeResourceWithFlag(
            pOsInterface,
            &pState->CSRResource,
            SURFACE_FLAG_ASSUME_NOT_IN_USE);
    }
}

//*-----------------------------------------------------------------------------
//| Purpose:    Free Sip Resource
//| Returns:    Result of the operation
//*-----------------------------------------------------------------------------
__inline void HalCm_FreeSipResource(
    PCM_HAL_STATE pState)   // [in] Pointer to CM HAL State
{
    PMOS_INTERFACE      pOsInterface = pState->pOsInterface;
    MOS_STATUS          hr = MOS_STATUS_SUCCESS;

    if (!Mos_ResourceIsNull(&pState->SipResource.OsResource))
    {
        if (pState->SipResource.bLocked)
        {
            hr = (MOS_STATUS)pOsInterface->pfnUnlockResource(
                    pOsInterface, 
                    &pState->SipResource.OsResource);

            CM_ASSERT(hr == MOS_STATUS_SUCCESS);
        }

        pOsInterface->pfnFreeResourceWithFlag(
            pOsInterface,
            &pState->SipResource.OsResource,
            SURFACE_FLAG_ASSUME_NOT_IN_USE);
    }
}

//*-----------------------------------------------------------------------------
//| Purpose: Sets Arg data in the buffer
//| Returns: Result of the operation
//*-----------------------------------------------------------------------------
__inline void HalCm_SetArgData(
    PCM_HAL_KERNEL_ARG_PARAM    pArgParam,
    uint32_t                    iThreadIndex,
    uint8_t                     *pBuffer)
{
    uint8_t *pDst;
    uint8_t *pSrc;

    pDst = pBuffer + pArgParam->iPayloadOffset;
    pSrc = pArgParam->pFirstValue + (iThreadIndex * pArgParam->iUnitSize);

    MOS_SecureMemcpy(pDst, pArgParam->iUnitSize, pSrc, pArgParam->iUnitSize);
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get the Buffer Entry 
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
__inline MOS_STATUS HalCm_GetResourceUPEntry( 
    PCM_HAL_STATE           pState,                                             // [in]  Pointer to CM State
    uint32_t                dwHandle,                                           // [in]  Handle
    PCM_HAL_SURFACE2D_UP_ENTRY    *pEntryOut)                                         // [out] Buffer Entry
{
    MOS_STATUS                    hr;
    PCM_HAL_SURFACE2D_UP_ENTRY    pEntry;

    hr = MOS_STATUS_SUCCESS;

    if (dwHandle >= pState->CmDeviceParam.iMax2DSurfaceUPTableSize)
    {
        CM_ERROR_ASSERT("Invalid handle '%d'", dwHandle);
        goto finish;
    }

    pEntry = &pState->pSurf2DUPTable[dwHandle];
    if (pEntry->iWidth == 0)
    {
        CM_ERROR_ASSERT("handle '%d' is not set", dwHandle);
        goto finish;
    }

    *pEntryOut = pEntry;

finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get the Buffer Entry 
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
__inline MOS_STATUS HalCm_GetBufferEntry(
    PCM_HAL_STATE           pState,                                             // [in]  Pointer to CM State
    uint32_t                dwHandle,                                           // [in]  Handle
    PCM_HAL_BUFFER_ENTRY    *pEntryOut)                                         // [out] Buffer Entry
{
    MOS_STATUS              hr;
    PCM_HAL_BUFFER_ENTRY    pEntry;

    hr = MOS_STATUS_SUCCESS;

    if (dwHandle >= pState->CmDeviceParam.iMaxBufferTableSize)
    {
        CM_ERROR_ASSERT("Invalid handle '%d'", dwHandle);
        goto finish;
    }

    pEntry = &pState->pBufferTable[dwHandle];
    if (pEntry->iSize == 0)
    {
        CM_ERROR_ASSERT("handle '%d' is not set", dwHandle);
        goto finish;
    }

    *pEntryOut = pEntry;

finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get the Surface2D Entry 
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
__inline MOS_STATUS HalCm_GetSurface2DEntry(
    PCM_HAL_STATE           pState,                                             // [in]  Pointer to CM State
    uint32_t                dwHandle,                                           // [in]  Handle
    PCM_HAL_SURFACE2D_ENTRY    *pEntryOut)                                         // [out] Buffer Entry
{
    MOS_STATUS                 hr;
    PCM_HAL_SURFACE2D_ENTRY    pEntry;

    hr = MOS_STATUS_SUCCESS;

    if (dwHandle >= pState->CmDeviceParam.iMax2DSurfaceTableSize)
    {
        CM_ERROR_ASSERT("Invalid handle '%d'", dwHandle);
        goto finish;
    }

    pEntry = &pState->pUmdSurf2DTable[dwHandle];
    if ((pEntry->iWidth == 0)||(pEntry->iHeight == 0))
    {
        CM_ERROR_ASSERT("handle '%d' is not set", dwHandle);
        goto finish;
    }

    *pEntryOut = pEntry;

finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get the 3D Entry 
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
__inline MOS_STATUS HalCm_Get3DResourceEntry(
    PCM_HAL_STATE               pState,                                         // [in]  Pointer to CM State
    uint32_t                    dwHandle,                                       // [in]  Handle
    PCM_HAL_3DRESOURCE_ENTRY    *pEntryOut)                                     // [out] Buffer Entry
{
    MOS_STATUS                  hr;
    PCM_HAL_3DRESOURCE_ENTRY    pEntry;

    hr = MOS_STATUS_SUCCESS;

    if (dwHandle >= pState->CmDeviceParam.iMax3DSurfaceTableSize)
    {
        CM_ERROR_ASSERT("Invalid handle '%d'", dwHandle);
        goto finish;
    }

    pEntry = &pState->pSurf3DTable[dwHandle];
    if (Mos_ResourceIsNull(&pEntry->OsResource))
    {
        CM_ERROR_ASSERT("3D handle '%d' is not set", dwHandle);
        goto finish;
    }

    *pEntryOut = pEntry;

finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Allocates and sets up Task Param memory structure
//| Return:     true if enabled
//| Note:       A single layer of memory is allocated to avoid fragmentation
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_AllocateTables(
    PCM_HAL_STATE   pState)         // [in] Pointer to HAL CM state
{
    MOS_STATUS              hr;
    PCM_HAL_DEVICE_PARAM    pDeviceParam;
    uint8_t                 *pb;
    uint32_t                iLookUpTableSize;
    uint32_t                iSamplerTableSize;
    uint32_t                iVmeTableSize;
    uint32_t                iSampler8x8TableSize;
    uint32_t                iTaskStatusTableSize;
    uint32_t                iBT2DIndexTableSize;
    uint32_t                iBT2DUPIndexTableSize;
    uint32_t                iBT3DIndexTableSize;
    uint32_t                iBTBufferIndexTableSize;
    uint32_t                iSamplerIndexTableSize;
    uint32_t                iVmeIndexTableSize;
    uint32_t                iSampler8x8IndexTableSize;
    uint32_t                iBufferTableSize;
    uint32_t                i2DSURFUPTableSize;
    uint32_t                i3DSurfTableSize;
    uint32_t                iSize;
    uint32_t                i2DSURFTableSize;

    hr      = MOS_STATUS_SUCCESS;
    pDeviceParam  = &pState->CmDeviceParam;

    iLookUpTableSize        = pDeviceParam->iMax2DSurfaceTableSize    * 
                              sizeof(CMLOOKUP_ENTRY);
    i2DSURFTableSize        = pDeviceParam->iMax2DSurfaceTableSize    * 
                            sizeof(CM_HAL_SURFACE2D_ENTRY);
    iBufferTableSize        = pDeviceParam->iMaxBufferTableSize       * 
                              sizeof(CM_HAL_BUFFER_ENTRY);
    i2DSURFUPTableSize      = pDeviceParam->iMax2DSurfaceUPTableSize  * 
                              sizeof(CM_HAL_SURFACE2D_UP_ENTRY);
    i3DSurfTableSize        = pDeviceParam->iMax3DSurfaceTableSize    *
                              sizeof(CM_HAL_3DRESOURCE_ENTRY);
    iSamplerTableSize       = pDeviceParam->iMaxSamplerTableSize      * 
                              sizeof(MHW_SAMPLER_STATE_PARAM);
    iSampler8x8TableSize    = pDeviceParam->iMaxSampler8x8TableSize      * 
                              sizeof(CM_HAL_SAMPLER_8X8_ENTRY);
    iTaskStatusTableSize    = pDeviceParam->iMaxTasks                 * sizeof(char);
    iBT2DIndexTableSize     = pDeviceParam->iMax2DSurfaceTableSize    * sizeof(CM_HAL_MULTI_USE_BTI_ENTRY);
    iBT2DUPIndexTableSize   = pDeviceParam->iMax2DSurfaceUPTableSize  * sizeof(CM_HAL_MULTI_USE_BTI_ENTRY);
    iBT3DIndexTableSize     = pDeviceParam->iMax3DSurfaceTableSize    * sizeof(CM_HAL_MULTI_USE_BTI_ENTRY);
    iBTBufferIndexTableSize = pDeviceParam->iMaxBufferTableSize       * sizeof(CM_HAL_MULTI_USE_BTI_ENTRY);
    iSamplerIndexTableSize  = pDeviceParam->iMaxSamplerTableSize      * sizeof(char);
    iSampler8x8IndexTableSize = pDeviceParam->iMaxSampler8x8TableSize      * sizeof(char);

    iSize                   = iLookUpTableSize          +
                              i2DSURFTableSize          +
                              iBufferTableSize          +
                              i2DSURFUPTableSize        +
                              i3DSurfTableSize          +
                              iSamplerTableSize         +
                              iSampler8x8TableSize      +
                              iTaskStatusTableSize      +
                              iBT2DIndexTableSize       +
                              iBT2DUPIndexTableSize     +
                              iBT3DIndexTableSize       +
                              iBTBufferIndexTableSize   +
                              iSamplerIndexTableSize    +
                              iSampler8x8IndexTableSize;

    pState->pTableMem = MOS_AllocAndZeroMemory(iSize);
    CM_CHK_NULL_RETURN_MOSSTATUS(pState->pTableMem);
    pb                          = (uint8_t*)pState->pTableMem;

    pState->pSurf2DTable        = (PCMLOOKUP_ENTRY)pb;
    pb                          += iLookUpTableSize;

    pState->pUmdSurf2DTable     = (PCM_HAL_SURFACE2D_ENTRY)pb;
    pb                          += i2DSURFTableSize;

    pState->pBufferTable        = (PCM_HAL_BUFFER_ENTRY)pb;
    pb                          += iBufferTableSize;

    pState->pSurf2DUPTable      = (PCM_HAL_SURFACE2D_UP_ENTRY)pb;
    pb                          += i2DSURFUPTableSize;

    pState->pSurf3DTable        = (PCM_HAL_3DRESOURCE_ENTRY)pb;
    pb                          += i3DSurfTableSize;

    pState->pSamplerTable       = (PMHW_SAMPLER_STATE_PARAM)pb;
    pb                          += iSamplerTableSize;

    pState->pSampler8x8Table    = (PCM_HAL_SAMPLER_8X8_ENTRY)pb;
    pb                          += iSampler8x8TableSize;

    pState->pTaskStatusTable    = (char *)pb;
    pb                          += iTaskStatusTableSize;

    pState->pBT2DIndexTable     = (PCM_HAL_MULTI_USE_BTI_ENTRY)pb;
    pb                          += iBT2DIndexTableSize;

    pState->pBT2DUPIndexTable   = (PCM_HAL_MULTI_USE_BTI_ENTRY)pb;
    pb                          += iBT2DUPIndexTableSize;

    pState->pBT3DIndexTable     = (PCM_HAL_MULTI_USE_BTI_ENTRY)pb;
    pb                          += iBT3DIndexTableSize;

    pState->pBTBufferIndexTable = (PCM_HAL_MULTI_USE_BTI_ENTRY)pb;
    pb                          += iBTBufferIndexTableSize;

    pState->pSamplerIndexTable  = (char *)pb;
    pb                          += iSamplerIndexTableSize;

    pState->pSampler8x8IndexTable  = (char *)pb;
    pb                          += iSampler8x8IndexTableSize;

finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Adds a tag to distinguish between same kernel ID
//|             Used for batch buffer re-use when splitting large task into 
//|             smaller pieces for EnqueueWithHints
//|             Using bits [48:42] from kernel ID for extra tag
//| Returns:    Result of the operation
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_AddKernelIDTag(
    PCM_HAL_KERNEL_PARAM     *pKernels,
    uint32_t                 iNumKernels,
    uint32_t                 iNumTasks,
    uint32_t                 iNumCurrentTask)
{
    uint32_t i;
    uint64_t numTasks;
    uint64_t numCurrentTask;
    uint64_t numTasksMask;
    uint64_t numCurrentTaskMask;

    numTasks = iNumTasks;
    numCurrentTask = iNumCurrentTask;
    numTasksMask = numTasks << 45;
    numCurrentTaskMask = numCurrentTask << 42;

    for( i = 0; i < iNumKernels; ++i )
    {
        pKernels[i]->uiKernelId |= numTasksMask;
        pKernels[i]->uiKernelId |= numCurrentTaskMask;
    }
    
    return MOS_STATUS_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Gets the Batch Buffer for rendering. If needed, de-allocate /
//|             allocate the memory for BB
//| Returns:    Result of the operation
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_GetBatchBuffer(
    PCM_HAL_STATE           pState,                                             // [in]  Pointer to CM State
    uint32_t                iNumKernels,                                        // [in]  Number of Kernels
    PCM_HAL_KERNEL_PARAM    *pKernels,                                          // [in]  Array for kernel data 
    PMHW_BATCH_BUFFER       *ppBb)                                              // [out] Batch Buffer Out
{
    MOS_STATUS              hr;
    PMHW_BATCH_BUFFER pBb = nullptr;
    PRENDERHAL_INTERFACE    pRenderHal;
    int32_t                 iSize;
    uint32_t                i;
    uint32_t                j;
    uint32_t                k;
    int32_t                 iFreeIdx;
    uint64_t                uiKernelIds[CM_MAX_KERNELS_PER_TASK];
    uint64_t                uiKernelParamsIds[CM_MAX_KERNELS_PER_TASK];
    CM_HAL_BB_DIRTY_STATUS  bbDirtyStatus;
    PCM_HAL_BB_ARGS       pBBCmArgs;

    hr              = MOS_STATUS_SUCCESS;
    pRenderHal      = pState->pRenderHal;
    iFreeIdx        = CM_INVALID_INDEX;
    bbDirtyStatus   = CM_HAL_BB_CLEAN;

    // Align the Batch Buffer size to power of 2
    iSize = HalCm_GetPow2Aligned(pState->pTaskParam->iBatchBufferSize);

    MOS_ZeroMemory(&uiKernelIds, CM_MAX_KERNELS_PER_TASK * sizeof(uint64_t));
    MOS_ZeroMemory(&uiKernelParamsIds, CM_MAX_KERNELS_PER_TASK * sizeof(uint64_t));

    //Sanity check for batch buffer
    if (iSize > CM_MAX_BB_SIZE)
    {
        CM_ERROR_ASSERT_RETURN(MOS_STATUS_EXCEED_MAX_BB_SIZE, "Batch Buffer Size exeeceds Max '%d'", iSize);
        goto finish;
    }

    for( i = 0; i < iNumKernels; ++i )
    {
        // remove upper 16 bits used for kernel binary re-use in GSH
        uiKernelParamsIds[i] = ((pKernels[i])->uiKernelId << 16 ) >> 16;
    }

#if CM_BATCH_BUFFER_REUSE_ENABLE

    bbDirtyStatus = CM_HAL_BB_CLEAN;
    for (k = 0; k < iNumKernels; ++k)
    {
        if (pKernels[k]->KernelThreadSpaceParam.BBdirtyStatus == CM_HAL_BB_DIRTY)
        {
            bbDirtyStatus = CM_HAL_BB_DIRTY;
            break;
        }
    }

    for (i = 0; i < (uint32_t)pState->iNumBatchBuffers; i++)
    {
        pBb = &pState->pBatchBuffers[i];
        CM_CHK_NULL_RETURN_MOSSTATUS(pBb);
        CM_CHK_NULL_RETURN_MOSSTATUS(pBb->pPrivateData);

        //if (!Mos_ResourceIsNull(&pBb->OsResource) && (!pBb->bBusy))
        if (!Mos_ResourceIsNull(&pBb->OsResource))
        {
            MOS_FillMemory(uiKernelIds, sizeof(uint64_t)*CM_MAX_KERNELS_PER_TASK, 0);
            for (j = 0; j < iNumKernels; j ++)
            {
                uiKernelIds[j] = uiKernelParamsIds[j];
            }

            pBBCmArgs = (PCM_HAL_BB_ARGS)pBb->pPrivateData;
            if (RtlEqualMemory(uiKernelIds, pBBCmArgs->uiKernelIds, sizeof(uint64_t)*CM_MAX_KERNELS_PER_TASK))
            {
                if( pBb->bBusy && bbDirtyStatus == CM_HAL_BB_DIRTY )
                {
                    pBBCmArgs->bLatest = false;
                }
                else if( pBBCmArgs->bLatest == true )
                {
                    break;
                }
            }
        }
    }
    if (i < (uint32_t)pState->iNumBatchBuffers)
    {
        CM_CHK_NULL_RETURN_MOSSTATUS(pBb);
        CM_CHK_NULL_RETURN_MOSSTATUS(pBb->pPrivateData);
        pBBCmArgs = (PCM_HAL_BB_ARGS)pBb->pPrivateData;

        pBBCmArgs->uiRefCount ++;
        pBb->iCurrent   = 0;
        pBb->dwSyncTag  = 0;
        pBb->iRemaining = pBb->iSize;
        *ppBb   = pBb;
        hr      = MOS_STATUS_SUCCESS;
        goto finish;
    }
#endif

    for (i = 0; i < (uint32_t)pState->iNumBatchBuffers; i++)
    {
        pBb = &pState->pBatchBuffers[i];
        CM_CHK_NULL_RETURN_MOSSTATUS(pBb);
        // No holes in the array of batch buffers
        if (Mos_ResourceIsNull(&pBb->OsResource))
        {
            iFreeIdx = i;
            break;
        }
    }
    if (iFreeIdx == CM_INVALID_INDEX)
    {
        for (i = 0; i < (uint32_t)pState->iNumBatchBuffers; i++)
        {
            pBb = &pState->pBatchBuffers[i];
            CM_CHK_NULL_RETURN_MOSSTATUS(pBb);
            CM_CHK_NULL_RETURN_MOSSTATUS(pBb->pPrivateData);
            pBBCmArgs = (PCM_HAL_BB_ARGS)pBb->pPrivateData;
            if (!pBb->bBusy)
            {
                if (pBb->iSize >= iSize)
                {
                    pBb->iCurrent   = 0;
                    pBb->iRemaining = pBb->iSize;
                    pBb->dwSyncTag  = 0;

                    pBBCmArgs->uiRefCount = 1;
                    for (i = 0; i <iNumKernels; i ++)
                    {
                        pBBCmArgs->uiKernelIds[i] = uiKernelParamsIds[i];
                    }

                    pBBCmArgs->bLatest = true;

                    *ppBb   = pBb;
                    hr      = MOS_STATUS_SUCCESS;
                    goto finish;
                }

                if (iFreeIdx == CM_INVALID_INDEX)
                {
                    iFreeIdx = i;
                }
            }
        }
    }
    if (iFreeIdx == CM_INVALID_INDEX)
    {
        CM_ERROR_ASSERT("No batch buffer available");
        goto finish;
    }

    pBb = &pState->pBatchBuffers[iFreeIdx];
    CM_CHK_NULL_RETURN_MOSSTATUS(pBb);
    CM_CHK_NULL_RETURN_MOSSTATUS(pBb->pPrivateData);
    pBBCmArgs = (PCM_HAL_BB_ARGS)pBb->pPrivateData;
    pBBCmArgs->uiRefCount = 1;
    for (i = 0; i <iNumKernels; i ++)
    {
        pBBCmArgs->uiKernelIds[i] =  uiKernelParamsIds[i];
    }

    pBBCmArgs->bLatest = true;

    if (!Mos_ResourceIsNull(&pBb->OsResource))
    {
        // Deallocate Batch Buffer
        CM_CHK_MOSSTATUS(pRenderHal->pfnFreeBB(pRenderHal, pBb));
    }

    // Allocate Batch Buffer
    CM_CHK_MOSSTATUS(pRenderHal->pfnAllocateBB(pRenderHal, pBb, iSize));
    *ppBb = pBb;

finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Parse the Kernel and populate the Task Param structure
//| Return:     Result of the operation
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_ParseTask(
    PCM_HAL_STATE               pState,                                         // [in] Pointer to HAL CM state
    PCM_HAL_EXEC_TASK_PARAM     pExecParam)                                     // [in] Pointer to Exec Task Param
{
    MOS_STATUS                  hr;
    PCM_HAL_TASK_PARAM          pTaskParam;
    PCM_HAL_KERNEL_PARAM        pKernelParam;
    uint32_t                    iHdrSize;
    uint32_t                    iTotalThreads;
    uint32_t                    iKrn;
    uint32_t                    dwCurbeOffset;
    PMHW_VFE_SCOREBOARD         pScoreboardParams;
    uint32_t                    hasThreadArg;
    bool                        bNonstallingScoreboardEnable;
    CM_HAL_DEPENDENCY           vfeDependencyInfo;
    PCM_HAL_KERNEL_THREADSPACE_PARAM pKernelTSParam;
    uint32_t                    i, j, k;
    uint8_t                     reuseBBUpdateMask;
    bool                        bitIsSet;
    PCM_HAL_MASK_AND_RESET      pDependencyMask;
    uint32_t                    uSurfaceNumber;
    uint32_t                    uSurfaceIndex;
    bool                        threadArgExists;

    hr                                 = MOS_STATUS_SUCCESS;
    dwCurbeOffset                      = 0;
    iTotalThreads                      = 0;
    pTaskParam                         = pState->pTaskParam;
    pTaskParam->iBatchBufferSize       = 0;
    hasThreadArg                       = 0;
    bNonstallingScoreboardEnable       = true;
    reuseBBUpdateMask                  = 0;
    bitIsSet                           = false;
    threadArgExists                    = false;
    iHdrSize = pState->pRenderHal->pHwSizes->dwSizeMediaObjectHeaderCmd;
    pTaskParam->DependencyPattern      = pExecParam->DependencyPattern;
    pTaskParam->threadSpaceWidth       = pExecParam->threadSpaceWidth;
    pTaskParam->threadSpaceHeight      = pExecParam->threadSpaceHeight;
    pTaskParam->WalkingPattern         = pExecParam->WalkingPattern;
    pTaskParam->walkingParamsValid     = pExecParam->walkingParamsValid;
    pTaskParam->dependencyVectorsValid = pExecParam->dependencyVectorsValid;
    if( pTaskParam->walkingParamsValid )
    {
        pTaskParam->walkingParams = pExecParam->walkingParams;
    }
    if( pTaskParam->dependencyVectorsValid )
    {
        pTaskParam->dependencyVectors = pExecParam->dependencyVectors;
    }
    pTaskParam->KernelDebugEnabled  = (uint32_t)pExecParam->bKernelDebugEnabled;
    //GT-PIN
    pTaskParam->SurEntryInfoArrays  = pExecParam->SurEntryInfoArrays;

    pTaskParam->surfacePerBT = 0;

    pTaskParam->ColorCountMinusOne  = pExecParam->ColorCountMinusOne;
    pTaskParam->MediaWalkerGroupSelect = pExecParam->MediaWalkerGroupSelect;

    if (pExecParam->ppThreadCoordinates)
    {
        pTaskParam->ppThreadCoordinates = pExecParam->ppThreadCoordinates;
    }

    pTaskParam->ppDependencyMasks = pExecParam->ppDependencyMasks;
    pTaskParam->uiSyncBitmap = pExecParam->uiSyncBitmap;
    pTaskParam->uiConditionalEndBitmap = pExecParam->uiConditionalEndBitmap;
    MOS_SecureMemcpy(pTaskParam->conditionalEndInfo, sizeof(pTaskParam->conditionalEndInfo), pExecParam->ConditionalEndInfo, sizeof(pExecParam->ConditionalEndInfo));

    pTaskParam->uiNumKernels = pExecParam->iNumKernels;
    pTaskParam->taskConfig   = pExecParam->taskConfig;
    pState->WalkerParams.CmWalkerEnable = true;
    pState->pRenderHal->IsMDFLoad = (pTaskParam->taskConfig.turboBoostFlag == CM_TURBO_BOOST_ENABLE);

    for (iKrn = 0; iKrn < pExecParam->iNumKernels; iKrn++)
    {
        if ((pExecParam->pKernels[iKrn] == nullptr) || 
            (pExecParam->piKernelSizes[iKrn] == 0))
        {
            CM_ERROR_ASSERT("Invalid Kernel data");
            goto finish;
        }

        pKernelParam    = (PCM_HAL_KERNEL_PARAM)pExecParam->pKernels[iKrn];
        PCM_INDIRECT_SURFACE_INFO       pIndirectSurfaceInfo = pKernelParam->IndirectDataParam.pSurfaceInfo;
        uSurfaceNumber = 0;
        if (pKernelParam->IndirectDataParam.iSurfaceCount)
        {
            uSurfaceIndex = 0;
            for (i = 0; i < pKernelParam->IndirectDataParam.iSurfaceCount; i++)
            {
                uSurfaceIndex = (pIndirectSurfaceInfo + i)->iBindingTableIndex > uSurfaceIndex ? ((pIndirectSurfaceInfo + i)->iBindingTableIndex + (pIndirectSurfaceInfo + i)->iNumBTIPerSurf - 1) : uSurfaceIndex;
                uSurfaceNumber = uSurfaceNumber + (pIndirectSurfaceInfo + i)->iNumBTIPerSurf;
            }
            pTaskParam->surfacePerBT = pTaskParam->surfacePerBT > uSurfaceIndex ? pTaskParam->surfacePerBT : uSurfaceIndex;
        }

        uSurfaceNumber += pKernelParam->iNumSurfaces;
        pTaskParam->surfacePerBT = pTaskParam->surfacePerBT < uSurfaceNumber ? 
                                            uSurfaceNumber : pTaskParam->surfacePerBT;

        // 26Z must be media object because by default it uses thread dependency mask
        // if there is no thread payload and dependency is not WAVEFRONT26Z, check if walker can be used
        if ( pKernelParam->iPayloadSize == 0)
        {
            //per-kernel thread space is avaiable, and check it at first
            if((pKernelParam->KernelThreadSpaceParam.iThreadSpaceWidth != 0) && 
                (pKernelParam->KernelThreadSpaceParam.patternType != CM_WAVEFRONT26Z) && 
                (pKernelParam->KernelThreadSpaceParam.patternType != CM_WAVEFRONT26ZI) && 
                (pKernelParam->KernelThreadSpaceParam.pThreadCoordinates == nullptr))
            {
                pKernelParam->WalkerParams.CmWalkerEnable = true;
                pKernelParam->WalkerParams.GroupIdLoopSelect = pExecParam->MediaWalkerGroupSelect;
            }
            else if (pKernelParam->KernelThreadSpaceParam.iThreadSpaceWidth == 0)
            {
                //Check per-task thread space setting
                if (pState->pTaskParam->ppThreadCoordinates)
                {
                     if (pState->pTaskParam->ppThreadCoordinates[iKrn] == nullptr)
                     {
                        pKernelParam->WalkerParams.CmWalkerEnable = true;
                        pKernelParam->WalkerParams.GroupIdLoopSelect = pExecParam->MediaWalkerGroupSelect;
                     }
                }
                else
                {
                    pKernelParam->WalkerParams.CmWalkerEnable = true;
                    pKernelParam->WalkerParams.GroupIdLoopSelect = pExecParam->MediaWalkerGroupSelect;
                }
            }
        }

        //Media walker mode will be disabled if any kernel need use media object, we don't support mixed working modes
        pState->WalkerParams.CmWalkerEnable &= pKernelParam->WalkerParams.CmWalkerEnable;

        if (!pState->WalkerParams.CmWalkerEnable)
        {
            pTaskParam->iBatchBufferSize += 
                pKernelParam->iNumThreads * (iHdrSize +  MOS_MAX(pKernelParam->iPayloadSize, 4));
        }

        iTotalThreads += pKernelParam->iNumThreads;

    }

    pTaskParam->iBatchBufferSize += CM_EXTRA_BB_SPACE;

    if (pState->pCmHalInterface->IsScoreboardParamNeeded())
    {
        pScoreboardParams = &pState->ScoreboardParams;
        pScoreboardParams->ScoreboardMask = 0;
        pScoreboardParams->ScoreboardType = bNonstallingScoreboardEnable;

        // set VFE scoreboarding information from union of kernel dependency vectors
        MOS_ZeroMemory(&vfeDependencyInfo, sizeof(CM_HAL_DEPENDENCY));
        for (iKrn = 0; iKrn < pExecParam->iNumKernels; iKrn++)
        {
            pKernelParam = pExecParam->pKernels[iKrn];
            pKernelTSParam = &pKernelParam->KernelThreadSpaceParam;

            // calculate union dependency vector of all kernels with dependency
            if (pKernelTSParam->dependencyInfo.count || pKernelTSParam->dependencyVectorsValid)
            {
                if (vfeDependencyInfo.count == 0)
                {
                    if (pKernelTSParam->dependencyInfo.count)
                    {
                        MOS_SecureMemcpy(&vfeDependencyInfo, sizeof(CM_HAL_DEPENDENCY), &pKernelTSParam->dependencyInfo, sizeof(CM_HAL_DEPENDENCY));
                    }
                    else if (pKernelTSParam->dependencyVectorsValid)
                    {
                        MOS_SecureMemcpy(&vfeDependencyInfo, sizeof(CM_HAL_DEPENDENCY), &pKernelTSParam->dependencyVectors, sizeof(CM_HAL_DEPENDENCY));
                    }
                    pKernelTSParam->globalDependencyMask = (1 << vfeDependencyInfo.count) - 1;
                }
                else
                {
                    uint32_t count = 0;
                    CM_HAL_DEPENDENCY dependencyInfo;
                    if (pKernelTSParam->dependencyVectorsValid)
                    {
                        count = pKernelTSParam->dependencyVectors.count;
                        MOS_SecureMemcpy(&dependencyInfo.deltaX, sizeof(int32_t) * count, &pKernelTSParam->dependencyVectors.deltaX, sizeof(int32_t) * count);
                        MOS_SecureMemcpy(&dependencyInfo.deltaY, sizeof(int32_t) * count, &pKernelTSParam->dependencyVectors.deltaY, sizeof(int32_t) * count);
                    }
                    else
                    {
                        count = pKernelTSParam->dependencyInfo.count;
                        MOS_SecureMemcpy(&dependencyInfo.deltaX, sizeof(int32_t) * count, &pKernelTSParam->dependencyInfo.deltaX, sizeof(int32_t) * count);
                        MOS_SecureMemcpy(&dependencyInfo.deltaY, sizeof(int32_t) * count, &pKernelTSParam->dependencyInfo.deltaY, sizeof(int32_t) * count);
                    }

                    for (j = 0; j < count; ++j)
                    {
                        for (k = 0; k < vfeDependencyInfo.count; ++k)
                        {
                            if ((dependencyInfo.deltaX[j] == vfeDependencyInfo.deltaX[k]) &&
                                (dependencyInfo.deltaY[j] == vfeDependencyInfo.deltaY[k]))
                            {
                                CM_HAL_SETBIT(pKernelTSParam->globalDependencyMask, k);
                                break;
                            }
                        }
                        if (k == vfeDependencyInfo.count)
                        {
                            vfeDependencyInfo.deltaX[vfeDependencyInfo.count] = dependencyInfo.deltaX[j];
                            vfeDependencyInfo.deltaY[vfeDependencyInfo.count] = dependencyInfo.deltaY[j];
                            CM_HAL_SETBIT(pKernelTSParam->globalDependencyMask, vfeDependencyInfo.count);
                            vfeDependencyInfo.count++;
                        }
                    }
                }
            }

            reuseBBUpdateMask |= pKernelTSParam->reuseBBUpdateMask;
        }

        if (vfeDependencyInfo.count > CM_HAL_MAX_DEPENDENCY_COUNT)
        {
            CM_ERROR_ASSERT("Union of kernel dependencies exceeds max dependency count (8)");
            goto finish;
        }

        pScoreboardParams->ScoreboardMask = (uint8_t)vfeDependencyInfo.count;
        for (i = 0; i < pScoreboardParams->ScoreboardMask; ++i)
        {
            pScoreboardParams->ScoreboardDelta[i].x = vfeDependencyInfo.deltaX[i];
            pScoreboardParams->ScoreboardDelta[i].y = vfeDependencyInfo.deltaY[i];
        }

        //If no dependency defined in kernel data, then check per-task thread space setting
        if (pScoreboardParams->ScoreboardMask == 0)
        {
            if (pTaskParam->dependencyVectorsValid)
            {
                pScoreboardParams->ScoreboardMask = (uint8_t)pTaskParam->dependencyVectors.count;
                for (uint32_t i = 0; i < pScoreboardParams->ScoreboardMask; ++i)
                {
                    pScoreboardParams->ScoreboardDelta[i].x = pTaskParam->dependencyVectors.deltaX[i];
                    pScoreboardParams->ScoreboardDelta[i].y = pTaskParam->dependencyVectors.deltaY[i];
                }
            }
            else
            {
                switch (pTaskParam->DependencyPattern)
                {
                case CM_NONE_DEPENDENCY:
                    break;

                case CM_VERTICAL_WAVE:
                    pScoreboardParams->ScoreboardMask = 1;
                    pScoreboardParams->ScoreboardDelta[0].x = 0xF; // -1 in uint8_t:4
                    pScoreboardParams->ScoreboardDelta[0].y = 0;
                    break;

                case CM_HORIZONTAL_WAVE:
                    pScoreboardParams->ScoreboardMask = 1;
                    pScoreboardParams->ScoreboardDelta[0].x = 0;
                    pScoreboardParams->ScoreboardDelta[0].y = 0xF; // -1 in uint8_t:4
                    break;

                case CM_WAVEFRONT:
                    pScoreboardParams->ScoreboardMask = 3;
                    pScoreboardParams->ScoreboardDelta[0].x = 0xF; // -1 in uint8_t:4
                    pScoreboardParams->ScoreboardDelta[0].y = 0;
                    pScoreboardParams->ScoreboardDelta[1].x = 0xF; // -1 in uint8_t:4
                    pScoreboardParams->ScoreboardDelta[1].y = 0xF; // -1 in uint8_t:4
                    pScoreboardParams->ScoreboardDelta[2].x = 0;
                    pScoreboardParams->ScoreboardDelta[2].y = 0xF; // -1 in uint8_t:4
                    break;

                case CM_WAVEFRONT26:
                    pScoreboardParams->ScoreboardMask = 4;
                    pScoreboardParams->ScoreboardDelta[0].x = 0xF; // -1 in uint8_t:4
                    pScoreboardParams->ScoreboardDelta[0].y = 0;
                    pScoreboardParams->ScoreboardDelta[1].x = 0xF; // -1 in uint8_t:4
                    pScoreboardParams->ScoreboardDelta[1].y = 0xF; // -1 in uint8_t:4
                    pScoreboardParams->ScoreboardDelta[2].x = 0;
                    pScoreboardParams->ScoreboardDelta[2].y = 0xF; // -1 in uint8_t:4
                    pScoreboardParams->ScoreboardDelta[3].x = 1;
                    pScoreboardParams->ScoreboardDelta[3].y = 0xF; // -1 in uint8_t:4
                    break;

                case CM_WAVEFRONT26Z:
                case CM_WAVEFRONT26ZIG:
                    pScoreboardParams->ScoreboardMask = 5;
                    pScoreboardParams->ScoreboardDelta[0].x = 0xF; // -1 in uint8_t:4
                    pScoreboardParams->ScoreboardDelta[0].y = 1;
                    pScoreboardParams->ScoreboardDelta[1].x = 0xF; // -1 in uint8_t:4
                    pScoreboardParams->ScoreboardDelta[1].y = 0;
                    pScoreboardParams->ScoreboardDelta[2].x = 0xF; // -1 in uint8_t:4
                    pScoreboardParams->ScoreboardDelta[2].y = 0xF; // -1 in uint8_t:4
                    pScoreboardParams->ScoreboardDelta[3].x = 0;
                    pScoreboardParams->ScoreboardDelta[3].y = 0xF; // -1 in uint8_t:4
                    pScoreboardParams->ScoreboardDelta[4].x = 1;
                    pScoreboardParams->ScoreboardDelta[4].y = 0xF; // -1 in uint8_t:4
                    break;

                case CM_WAVEFRONT26ZI:
                    pScoreboardParams->ScoreboardMask = 7;
                    pScoreboardParams->ScoreboardDelta[0].x = 0xF; // -1 in uint8_t:4
                    pScoreboardParams->ScoreboardDelta[0].y = 1;
                    pScoreboardParams->ScoreboardDelta[1].x = 0xE;  // -2
                    pScoreboardParams->ScoreboardDelta[1].y = 0;
                    pScoreboardParams->ScoreboardDelta[2].x = 0xF; // -1 in uint8_t:4
                    pScoreboardParams->ScoreboardDelta[2].y = 0;
                    pScoreboardParams->ScoreboardDelta[3].x = 0xF; // -1 in uint8_t:4
                    pScoreboardParams->ScoreboardDelta[3].y = 0xF; // -1 in uint8_t:4
                    pScoreboardParams->ScoreboardDelta[4].x = 0;
                    pScoreboardParams->ScoreboardDelta[4].y = 0xF; // -1 in uint8_t:4
                    pScoreboardParams->ScoreboardDelta[5].x = 1;
                    pScoreboardParams->ScoreboardDelta[5].y = 0xF; // -1 in uint8_t:4
                    pScoreboardParams->ScoreboardDelta[6].x = 1;
                    pScoreboardParams->ScoreboardDelta[6].y = 0;
                    break;

                case CM_WAVEFRONT26X:
                    pScoreboardParams->ScoreboardMask = 7;
                    pScoreboardParams->ScoreboardDelta[0].x = 0xF;
                    pScoreboardParams->ScoreboardDelta[0].y = 3;
                    pScoreboardParams->ScoreboardDelta[1].x = 0xF;
                    pScoreboardParams->ScoreboardDelta[1].y = 1;
                    pScoreboardParams->ScoreboardDelta[2].x = 0xF;
                    pScoreboardParams->ScoreboardDelta[2].y = 0xF;
                    pScoreboardParams->ScoreboardDelta[3].x = 0;
                    pScoreboardParams->ScoreboardDelta[3].y = 0xF;
                    pScoreboardParams->ScoreboardDelta[4].x = 0;
                    pScoreboardParams->ScoreboardDelta[4].y = 0xE;
                    pScoreboardParams->ScoreboardDelta[5].x = 0;
                    pScoreboardParams->ScoreboardDelta[5].y = 0xD;
                    pScoreboardParams->ScoreboardDelta[6].x = 1;
                    pScoreboardParams->ScoreboardDelta[6].y = 0xD;
                    break;

                default:
                    pTaskParam->DependencyPattern = CM_NONE_DEPENDENCY;
                    break;

                }
            }
        }
    }
    //Set size of surface binding table size
    CM_SURFACE_BTI_INFO SurfBTIInfo;
    pState->pCmHalInterface->GetHwSurfaceBTIInfo(&SurfBTIInfo);
    
    pTaskParam->surfacePerBT += SurfBTIInfo.dwNormalSurfaceStart ;

    // add one if kernel debugger is enabled
    if (pExecParam->bKernelDebugEnabled)
    {
        pTaskParam->surfacePerBT += CM_RESERVED_SURFACE_NUMBER_FOR_KERNEL_DEBUG;
    }

    //If global surface is used and current surface bt size less than the max index of reserved surfaces
    //use set it as max bti size
    if ((pExecParam->bGlobalSurfaceUsed) && (pTaskParam->surfacePerBT < SurfBTIInfo.dwReservedSurfaceEnd))
    {
        pTaskParam->surfacePerBT = CM_MAX_STATIC_SURFACE_STATES_PER_BT;
    }

    //Make sure surfacePerBT do not exceed CM_MAX_STATIC_SURFACE_STATES_PER_BT
    pTaskParam->surfacePerBT = MOS_MIN(CM_MAX_STATIC_SURFACE_STATES_PER_BT, pTaskParam->surfacePerBT);

    if( pTaskParam->ppDependencyMasks )
    {
        for (iKrn = 0; iKrn < pExecParam->iNumKernels; iKrn++)
        {
            pKernelParam    = pExecParam->pKernels[iKrn];
            pDependencyMask = pTaskParam->ppDependencyMasks[iKrn];
            if( pDependencyMask )
            {
                for( i = 0; i < pKernelParam->iNumThreads; ++i )
                {
                    reuseBBUpdateMask |= pDependencyMask[i].resetMask;
                }
            }
        }
    }

    CM_HAL_CHECKBIT_IS_SET(bitIsSet, reuseBBUpdateMask, CM_NO_BATCH_BUFFER_REUSE_BIT_POS);
    if( bitIsSet || reuseBBUpdateMask == 0 )
    {
        pTaskParam->reuseBBUpdateMask = 0;
    }
    else
    {
        pTaskParam->reuseBBUpdateMask = 1;
    }

    threadArgExists = HalCm_GetTaskHasThreadArg(pExecParam->pKernels, pExecParam->iNumKernels);

    // For media object with thread arg, only support up to CM_MAX_USER_THREADS (512*512) threads
    // otherwise can support up to 262144 media object commands in batch buffer
    if (!pState->WalkerParams.CmWalkerEnable) {
        if (!threadArgExists)
        {
            if(iTotalThreads > CM_MAX_USER_THREADS_NO_THREADARG)
            {
                CM_ERROR_ASSERT(
                    "Total task threads '%d' exceeds max allowed threads '%d'",
                    iTotalThreads, 
                    CM_MAX_USER_THREADS_NO_THREADARG);
                goto finish;
            }
        }
        else
        {
            if (iTotalThreads > CM_MAX_USER_THREADS)
            {
                CM_ERROR_ASSERT(
                    "Total task threads '%d' exceeds max allowed threads '%d'",
                    iTotalThreads, 
                    CM_MAX_USER_THREADS);
                goto finish;
            }
        }
    }

finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Parse the Kernel and populate the Task Param structure
//| Return:     Result of the operation
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_ParseGroupTask(
    PCM_HAL_STATE                       pState,           // [in] Pointer to HAL CM state
    PCM_HAL_EXEC_GROUP_TASK_PARAM       pExecGroupParam)  // [in] Pointer to Exec Task Param
{
    PCM_HAL_TASK_PARAM      pTaskParam      = pState->pTaskParam;
    MOS_STATUS              hr              = MOS_STATUS_SUCCESS;
    PCM_HAL_KERNEL_PARAM    pKernelParam    = nullptr;
    uint32_t                uSurfaceNumber;
    uint32_t                uSurfaceIndex;

    pTaskParam->SurEntryInfoArrays  = pExecGroupParam->SurEntryInfoArrays;  //GT-PIN
    pTaskParam->iBatchBufferSize = 0;
    pTaskParam->KernelDebugEnabled  = (uint32_t)pExecGroupParam->bKernelDebugEnabled;

    pTaskParam->uiNumKernels = pExecGroupParam->iNumKernels;
    pTaskParam->uiSyncBitmap = pExecGroupParam->uiSyncBitmap;
    pTaskParam->taskConfig = pExecGroupParam->taskConfig;
    for (uint32_t iKrn = 0; iKrn < pExecGroupParam->iNumKernels; iKrn ++)
    {
        pKernelParam = pExecGroupParam->pKernels[iKrn];
        PCM_INDIRECT_SURFACE_INFO       pIndirectSurfaceInfo = pKernelParam->IndirectDataParam.pSurfaceInfo;
        uSurfaceNumber = 0;
        if (pKernelParam->IndirectDataParam.iSurfaceCount)
        {
            uSurfaceIndex = 0;
            for (uint32_t i = 0; i < pKernelParam->IndirectDataParam.iSurfaceCount; i++)
            {
                uSurfaceIndex = (pIndirectSurfaceInfo + i)->iBindingTableIndex > uSurfaceIndex ? (pIndirectSurfaceInfo + i)->iBindingTableIndex : uSurfaceIndex;
                uSurfaceNumber++;
            }
            pTaskParam->surfacePerBT = pTaskParam->surfacePerBT > uSurfaceIndex ? pTaskParam->surfacePerBT : uSurfaceIndex;
        }

        uSurfaceNumber += pKernelParam->iNumSurfaces;
        
        pTaskParam->surfacePerBT = pTaskParam->surfacePerBT < uSurfaceNumber ? 
                                            uSurfaceNumber : pTaskParam->surfacePerBT;
    }

    CM_SURFACE_BTI_INFO SurfBTIInfo;
    pState->pCmHalInterface->GetHwSurfaceBTIInfo(&SurfBTIInfo);
    
    pTaskParam->surfacePerBT += SurfBTIInfo.dwNormalSurfaceStart ;

    // add one if kernel debugger is enabled
    if (pExecGroupParam->bKernelDebugEnabled)
    {
        pTaskParam->surfacePerBT += CM_RESERVED_SURFACE_NUMBER_FOR_KERNEL_DEBUG;
    }

    //If global surface is used and current surface bt size less than the max index of reserved surfaces
    //use set it as max bti size
    if ((pExecGroupParam->bGlobalSurfaceUsed) && 
        (pTaskParam->surfacePerBT < SurfBTIInfo.dwReservedSurfaceEnd))
    {
        pTaskParam->surfacePerBT = CM_MAX_STATIC_SURFACE_STATES_PER_BT;
    }

    //Make sure surfacePerBT do not exceed CM_MAX_STATIC_SURFACE_STATES_PER_BT
    pTaskParam->surfacePerBT = MOS_MIN(CM_MAX_STATIC_SURFACE_STATES_PER_BT, pTaskParam->surfacePerBT);

    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Parse the Kernel and populate the Hints Task Param structure
//| Return:     Result of the operation
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_ParseHintsTask(
    PCM_HAL_STATE                     pState,                                         // [in] Pointer to HAL CM state
    PCM_HAL_EXEC_HINTS_TASK_PARAM     pExecHintsParam)
{
    MOS_STATUS                        hr;
    PCM_HAL_TASK_PARAM                pTaskParam;
    PCM_HAL_KERNEL_PARAM              pKernelParam;
    uint32_t                          iHdrSize;
    uint32_t                          iTotalThreads;
    uint32_t                          iKrn;
    uint32_t                          dwCurbeOffset;
    PMHW_VFE_SCOREBOARD               pScoreboardParams;
    uint32_t                          hasThreadArg;
    bool                              bNonstallingScoreboardEnable;
    bool                              bitIsSet;
    uint8_t                           reuseBBUpdateMask;
    bool                              threadArgExists;

    hr                                = MOS_STATUS_SUCCESS;
    iKrn                              = 0;
    pTaskParam                        = pState->pTaskParam;
    bNonstallingScoreboardEnable      = true;
    bitIsSet                          = false;
    dwCurbeOffset                     = 0;
    hasThreadArg                      = 0;
    iTotalThreads                     = 0;
    reuseBBUpdateMask                 = 0;
    threadArgExists                   = false;

    iHdrSize = pState->pRenderHal->pHwSizes->dwSizeMediaObjectHeaderCmd;
    pScoreboardParams = &pState->ScoreboardParams;


    for( iKrn = 0; iKrn < pExecHintsParam->iNumKernels; ++iKrn )
    {
        if ((pExecHintsParam->pKernels[iKrn] == nullptr) ||
            (pExecHintsParam->piKernelSizes[iKrn] == 0))
        {
            CM_ERROR_ASSERT("Invalid Kernel data");
            goto finish;
        }

        // Parse the kernel Param
        pKernelParam =  pExecHintsParam->pKernels[iKrn];

        // if any kernel disables non-stalling, the non-stalling will be disabled
        bNonstallingScoreboardEnable &= (pKernelParam->dwCmFlags & CM_KERNEL_FLAGS_NONSTALLING_SCOREBOARD) ? true : false;

        if (!pState->WalkerParams.CmWalkerEnable)
        {
            pTaskParam->iBatchBufferSize += 
                pKernelParam->iNumThreads * (iHdrSize +  MOS_MAX(pKernelParam->iPayloadSize, 4));
        }

        iTotalThreads += pKernelParam->iNumThreads;

        reuseBBUpdateMask |= pKernelParam->KernelThreadSpaceParam.reuseBBUpdateMask;
    }

    CM_HAL_CHECKBIT_IS_SET(bitIsSet, reuseBBUpdateMask, CM_NO_BATCH_BUFFER_REUSE_BIT_POS);
    if( bitIsSet || reuseBBUpdateMask == 0 )
    {
        pTaskParam->reuseBBUpdateMask = 0;
    }
    else
    {
        pTaskParam->reuseBBUpdateMask = 1;
    }

    pTaskParam->iBatchBufferSize += CM_EXTRA_BB_SPACE;

    pScoreboardParams->ScoreboardType = bNonstallingScoreboardEnable;

    threadArgExists = HalCm_GetTaskHasThreadArg(pExecHintsParam->pKernels, pExecHintsParam->iNumKernels);

    if (!pState->WalkerParams.CmWalkerEnable) {
        if (!threadArgExists)
        {
            if(iTotalThreads > CM_MAX_USER_THREADS_NO_THREADARG)
            {
                CM_ERROR_ASSERT(
                    "Total task threads '%d' exceeds max allowed threads '%d'",
                    iTotalThreads, 
                    CM_MAX_USER_THREADS_NO_THREADARG);
                goto finish;
            }
        }
        else
        {
            if (iTotalThreads > CM_MAX_USER_THREADS)
            {
                CM_ERROR_ASSERT(
                    "Total task threads '%d' exceeds max allowed threads '%d'",
                    iTotalThreads, 
                    CM_MAX_USER_THREADS);
                goto finish;
            }
        }
    }

finish:
    return hr;
}


/*
** check to see if kernel entry is flaged as free or it is null
** used for combining
*/
bool bIsFree( PRENDERHAL_KRN_ALLOCATION pKAlloc ) 
{
    if (pKAlloc== nullptr) 
    {
        return false;
    }
    else
    {
        if (pKAlloc->dwFlags != RENDERHAL_KERNEL_ALLOCATION_FREE)
        {
            return false;
        }
    }

    return true;
}

/*
** local used supporting function
** setup correct values according to input and copy kernelBinary as needed
*/
void CmLoadKernel(PCM_HAL_STATE             pState,
                  PRENDERHAL_STATE_HEAP     pStateHeap,
                  PRENDERHAL_KRN_ALLOCATION pKernelAllocation,
                  uint32_t dwSync,
                  uint32_t dwCount,
                  PRENDERHAL_KERNEL_PARAM   pParameters,
                  PCM_HAL_KERNEL_PARAM      pKernelParam,
                  MHW_KERNEL_PARAM         *pMhwKernelParam,
                  bool                      isCloneEntry)
{
    UNUSED(pState);
    if (pMhwKernelParam)
    {
        pKernelAllocation->iKID        = -1;
        pKernelAllocation->iKUID       = pMhwKernelParam->iKUID;
        pKernelAllocation->iKCID       = pMhwKernelParam->iKCID;
        pKernelAllocation->dwSync      = dwSync;
        pKernelAllocation->dwCount     = dwCount & 0xFFFFFFFF; // 28 bits
        pKernelAllocation->dwFlags     = RENDERHAL_KERNEL_ALLOCATION_USED;
        pKernelAllocation->Params      = *pParameters;
        pKernelAllocation->pMhwKernelParam = pMhwKernelParam;

        if (!isCloneEntry)
        {
            // Copy kernel data
            // Copy MovInstruction First
            MOS_SecureMemcpy(pStateHeap->pIshBuffer + pKernelAllocation->dwOffset,
                pKernelParam->iMovInsDataSize,
                pKernelParam->pMovInsData,
                pKernelParam->iMovInsDataSize);

            // Copy Cm Kernel Binary
            MOS_SecureMemcpy(pStateHeap->pIshBuffer + pKernelAllocation->dwOffset + pKernelParam->iMovInsDataSize,
                pKernelParam->iKernelBinarySize - pKernelParam->iMovInsDataSize,
                pKernelParam->pKernelBinary,
                pKernelParam->iKernelBinarySize - pKernelParam->iMovInsDataSize);

            // Padding bytes dummy instructions after kernel binary to resolve page fault issue
            MOS_ZeroMemory(pStateHeap->pIshBuffer + pKernelAllocation->dwOffset + pKernelParam->iKernelBinarySize, CM_KERNEL_BINARY_PADDING_SIZE);
        }
    }
    else
    {
        pKernelAllocation->iKID        = -1;
        pKernelAllocation->iKUID       = -1;
        pKernelAllocation->iKCID       = -1;
        pKernelAllocation->dwSync      = 0;
        pKernelAllocation->dwCount     = 0;
        pKernelAllocation->dwFlags     = RENDERHAL_KERNEL_ALLOCATION_FREE;
        pKernelAllocation->pMhwKernelParam = nullptr;
        pKernelAllocation->cloneKernelParams.cloneKernelID       = -1;
        pKernelAllocation->cloneKernelParams.isClone             = false;
        pKernelAllocation->cloneKernelParams.isHeadKernel        = false;
        pKernelAllocation->cloneKernelParams.kernelBinaryAllocID = -1;
        pKernelAllocation->cloneKernelParams.referenceCount      = 0;
    }
}

/*
** local used supporting function
** Try to find free entry which is big enough to load kernel binary
** If we cannot find one, then return fail, so we will delete more entries
*/
int32_t CmSearchFreeSlotSize(PCM_HAL_STATE pState, MHW_KERNEL_PARAM *pMhwKernelParam, bool isCloneEntry)
{
    PRENDERHAL_STATE_HEAP     pStateHeap;
    PRENDERHAL_KRN_ALLOCATION pKernelAllocation;
    int32_t                 iKernelAllocationID;
    int32_t                 iReturnVal = -1;
    int32_t                 iNeededSize;

    pStateHeap          = pState->pRenderHal->pStateHeap;
    pKernelAllocation   = pStateHeap->pKernelAllocation;

    if (isCloneEntry)
    {
        iNeededSize = CM_64BYTE;
    }
    else
    {
        iNeededSize = pMhwKernelParam->iSize;
    }

    for (iKernelAllocationID = 0;
         iKernelAllocationID < pState->nNumKernelsInGSH;
         iKernelAllocationID++, pKernelAllocation++)
    {
        if(pKernelAllocation->dwFlags == RENDERHAL_KERNEL_ALLOCATION_FREE)
        {
            if(pState->pTotalKernelSize[iKernelAllocationID] >= iNeededSize)
            {
                // found free slot which is big enough
                return iKernelAllocationID;
            }
        }
    }

    // not found
    return iReturnVal;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Updates the clone entries' head kernel binary allocation IDs
//|             Function is called after kernel allocations are shifted due to combining neighboring free entries
//| Return:     Result of the operation
//*-----------------------------------------------------------------------------
void HalCm_UpdateCloneKernel(PCM_HAL_STATE pState,
    uint32_t shiftPoint,
    CM_SHIFT_DIRECTION shiftDirection,
    uint32_t shiftFactor)
{
    PRENDERHAL_STATE_HEAP       pStateHeap;
    PRENDERHAL_KRN_ALLOCATION   pKernelAllocation;
    int32_t                     allocationID;

    pStateHeap = pState->pRenderHal->pStateHeap;
    pKernelAllocation = pStateHeap->pKernelAllocation;

    for (allocationID = 0; allocationID < pState->nNumKernelsInGSH; allocationID++, pKernelAllocation++)
    {
        pKernelAllocation = &(pStateHeap->pKernelAllocation[allocationID]);
        if (pKernelAllocation->cloneKernelParams.isClone && ((pKernelAllocation->cloneKernelParams.kernelBinaryAllocID) > (int32_t)shiftPoint))
        {
            if (shiftDirection == CM_SHIFT_LEFT)
            {
                pKernelAllocation->cloneKernelParams.kernelBinaryAllocID = pKernelAllocation->cloneKernelParams.kernelBinaryAllocID + shiftFactor;
            }
            else
            {
                pKernelAllocation->cloneKernelParams.kernelBinaryAllocID = pKernelAllocation->cloneKernelParams.kernelBinaryAllocID - shiftFactor;
            }
        }
    }
}

/*
** local used supporting function
** We found free slot and load kernel to this slot. There are 3 cases (see code)
*/
int32_t CmAddCurrentKernelToFreeSlot(PCM_HAL_STATE pState,
                                  int32_t slot,
                                  PRENDERHAL_KERNEL_PARAM pParameters,
                                  PCM_HAL_KERNEL_PARAM    pKernelParam,
                                  MHW_KERNEL_PARAM       *pMhwKernelParam,
                                  CM_CLONE_TYPE           cloneType,
                                  int32_t                 headKernelAllocationID)
{
    PRENDERHAL_STATE_HEAP       pStateHeap;
    PRENDERHAL_KRN_ALLOCATION   pKernelAllocation, pKernelAllocationN;

    int32_t hr = CM_SUCCESS;
    int32_t i;
    int32_t totalSize, tmpSize, dwOffset, neededSize;
    bool    bAdjust, isCloneEntry, isHeadKernel, isCloneAsHead, bAdjustHeadKernelID;
    uint32_t tag;

    pStateHeap          = pState->pRenderHal->pStateHeap;
    pKernelAllocation   = pStateHeap->pKernelAllocation;
    bAdjustHeadKernelID = false;

    switch (cloneType)
    {
        case CM_CLONE_ENTRY:
        {
            neededSize    = CM_64BYTE;
            isCloneEntry  = true;
            isHeadKernel  = false;
            isCloneAsHead = false;
        }
        break;
        case CM_HEAD_KERNEL:
        {
            neededSize    = pMhwKernelParam->iSize;
            isHeadKernel  = true;
            isCloneEntry  = false;
            isCloneAsHead = false;
        }
        break;
        case CM_CLONE_AS_HEAD_KERNEL:
        {
            neededSize    = pMhwKernelParam->iSize;
            isHeadKernel  = true;
            isCloneEntry  = false;
            isCloneAsHead = true;
        }
        break;
        case CM_NO_CLONE:
        {
            neededSize    = pMhwKernelParam->iSize;
            isCloneEntry  = false;
            isHeadKernel  = false;
            isCloneAsHead = false;
        }
        break;
        default:
        {
            hr = CM_FAILURE;
            goto finish;
        }
    }

    // to check if we have perfect size match
    if(pStateHeap->pKernelAllocation[slot].iSize == neededSize)
    {
        bAdjust = false;
    }
    else
    {
        bAdjust = true;
    }

    if ((pState->nNumKernelsInGSH < pState->CmDeviceParam.iMaxGSHKernelEntries) && bAdjust)
    {
        // we have extra entry to add
        // add new entry and pump index down below
        int32_t lastKernel = pState->nNumKernelsInGSH - 1;
        for(i = lastKernel; i>slot; i--)
        {
            pKernelAllocation = &pStateHeap->pKernelAllocation[i];
            pKernelAllocationN = &pStateHeap->pKernelAllocation[i+1];
            *pKernelAllocationN = *pKernelAllocation;
            pState->pTotalKernelSize[i+1] = pState->pTotalKernelSize[i];
        }

        if (lastKernel > slot)
        {
            // update the headKernelAllocationID if it was shifted
            if (headKernelAllocationID > slot)
            {
                headKernelAllocationID++;
                bAdjustHeadKernelID = true;
            }
        }

        totalSize = pState->pTotalKernelSize[slot];
        tmpSize = neededSize;

        dwOffset = pStateHeap->pKernelAllocation[slot].dwOffset;

        // now add new one
        pKernelAllocation = &pStateHeap->pKernelAllocation[slot];
        if(pState->bCBBEnabled)
        {
            tag = pState->pOsInterface->pfnGetGpuStatusTag(pState->pOsInterface,
                pState->pOsInterface->CurrentGpuContextOrdinal);
        }
        else
        {
            tag = pStateHeap->dwNextTag;
        }

        CmLoadKernel(pState, pStateHeap, pKernelAllocation, tag, pStateHeap->dwAccessCounter, pParameters, pKernelParam, pMhwKernelParam, isCloneEntry);
        pStateHeap->dwAccessCounter++;

        pKernelAllocation->iSize = tmpSize;
        pState->pTotalKernelSize[slot] = MOS_ALIGN_CEIL(tmpSize, 64);

        // insert a new slot which is free with rest 
        tmpSize = MOS_ALIGN_CEIL(tmpSize, 64);  // HW required 64 byte align
        pKernelAllocation = &pStateHeap->pKernelAllocation[slot+1];
        CmLoadKernel(pState, pStateHeap, pKernelAllocation, 0, 0, pParameters, pKernelParam, nullptr, isCloneEntry);
        pKernelAllocation->dwOffset = dwOffset+tmpSize;
        pKernelAllocation->iSize = 0;
        pState->pTotalKernelSize[slot+1] = totalSize - tmpSize;

        // added one more entry
        pState->nNumKernelsInGSH++;

        pKernelAllocation = &pStateHeap->pKernelAllocation[slot];
        if (isCloneEntry)
        {
            if (!pStateHeap->pKernelAllocation[headKernelAllocationID].cloneKernelParams.isHeadKernel)
            {
                // ERROR thought kernel with allocation ID, headKernelAllocationID, was a head kernel, but it's not
                hr = CM_FAILURE;
                goto finish;
            }

            pKernelAllocation->cloneKernelParams.dwOffsetForAllocID  = dwOffset;
            pKernelAllocation->dwOffset                              = pStateHeap->pKernelAllocation[headKernelAllocationID].dwOffset;
            pKernelAllocation->cloneKernelParams.isClone             = true;
            pKernelAllocation->cloneKernelParams.kernelBinaryAllocID = headKernelAllocationID;
            pKernelAllocation->cloneKernelParams.cloneKernelID       = pStateHeap->pKernelAllocation[headKernelAllocationID].iKUID;

            pStateHeap->pKernelAllocation[headKernelAllocationID].cloneKernelParams.referenceCount = pStateHeap->pKernelAllocation[headKernelAllocationID].cloneKernelParams.referenceCount + 1;

            // update head kernel's dwCount after updating the clone entry's dwCount so that clone will be selected for deletion first
            pStateHeap->pKernelAllocation[headKernelAllocationID].dwCount = pStateHeap->dwAccessCounter++;

        }
        else
        {
            pKernelAllocation->dwOffset = dwOffset;

            if (isHeadKernel)
            {
                pKernelAllocation->cloneKernelParams.isHeadKernel = true;
                if (isCloneAsHead)
                {
                    pKernelAllocation->cloneKernelParams.cloneKernelID = pKernelParam->ClonedKernelParam.kernelID;
                }
            }
        }

        if (lastKernel > slot)
        {
            HalCm_UpdateCloneKernel(pState, slot, CM_SHIFT_LEFT, 1);
            if (isCloneEntry && bAdjustHeadKernelID)
            {
                // if clone entry and already adjusted head kernel ID, then adjusted again in HalCm_UpdateCloneKernel, need to do only once
                pKernelAllocation->cloneKernelParams.kernelBinaryAllocID = pKernelAllocation->cloneKernelParams.kernelBinaryAllocID - 1;
            }
        }
    }
    else if (pState->nNumKernelsInGSH < pState->CmDeviceParam.iMaxGSHKernelEntries)
    {
        // no need to create a new entry since we have the same size
        pKernelAllocation = &pStateHeap->pKernelAllocation[slot];

        if(pState->bCBBEnabled)
        {
            tag = pState->pOsInterface->pfnGetGpuStatusTag(pState->pOsInterface, 
                pState->pOsInterface->CurrentGpuContextOrdinal);
        }
        else
        {
            tag = pStateHeap->dwNextTag;
        }

        CmLoadKernel(pState, pStateHeap, pKernelAllocation, tag, pStateHeap->dwAccessCounter, pParameters, pKernelParam, pMhwKernelParam, isCloneEntry);
        pStateHeap->dwAccessCounter++;
        // no change for pKernelAllocation->dwOffset
        pKernelAllocation->iSize = neededSize;
        pState->pTotalKernelSize[slot] = MOS_ALIGN_CEIL(pMhwKernelParam->iSize, 64);
        if (isCloneEntry)
        {
            if (!pStateHeap->pKernelAllocation[headKernelAllocationID].cloneKernelParams.isHeadKernel)
            {
                // ERROR thought kernel with allocation ID, headKernelAllocationID, was a head kernel, but it's not
                hr = CM_FAILURE;
                goto finish;
            }

            pKernelAllocation->cloneKernelParams.dwOffsetForAllocID  = pKernelAllocation->dwOffset;
            pKernelAllocation->dwOffset                              = pStateHeap->pKernelAllocation[headKernelAllocationID].dwOffset;
            pKernelAllocation->cloneKernelParams.isClone             = true;
            pKernelAllocation->cloneKernelParams.kernelBinaryAllocID = headKernelAllocationID;
            pKernelAllocation->cloneKernelParams.cloneKernelID       = pStateHeap->pKernelAllocation[headKernelAllocationID].iKUID;

            pStateHeap->pKernelAllocation[headKernelAllocationID].cloneKernelParams.referenceCount = pStateHeap->pKernelAllocation[headKernelAllocationID].cloneKernelParams.referenceCount + 1;

            // update head kernel's dwCount after updating the clone entry's dwCount so that clone will be selected for deletion first
            pStateHeap->pKernelAllocation[headKernelAllocationID].dwCount = pStateHeap->dwAccessCounter++;
        }
        else if (isHeadKernel)
        {
            pKernelAllocation->cloneKernelParams.isHeadKernel = true;
            if (isCloneAsHead)
            {
                pKernelAllocation->cloneKernelParams.cloneKernelID = pKernelParam->ClonedKernelParam.kernelID;
            }
        }
    }
    else
    {
        // all slots are used, but we have one free which is big enough
        // we may have fragmentation, but code is the same as above case
        pKernelAllocation = &pStateHeap->pKernelAllocation[slot];

        if(pState->bCBBEnabled)
        {
            tag = pState->pOsInterface->pfnGetGpuStatusTag(pState->pOsInterface, pState->pOsInterface->CurrentGpuContextOrdinal);
        }
        else
        {
            tag = pStateHeap->dwNextTag;
        }
        
        CmLoadKernel(pState, pStateHeap, pKernelAllocation, tag, pStateHeap->dwAccessCounter, pParameters, pKernelParam, pMhwKernelParam, isCloneEntry);
        pStateHeap->dwAccessCounter++;
        // pKernelAllocation->iTotalSize is not changed, but we have smaller actual size
        // no change for pKernelAllocation->dwOffset
        pKernelAllocation->iSize = neededSize;

        if (isCloneEntry)
        {
            if (!pStateHeap->pKernelAllocation[headKernelAllocationID].cloneKernelParams.isHeadKernel)
            {
                // ERROR thought kernel with allocation ID, headKernelAllocationID, was a head kernel, but it's not
                hr = CM_FAILURE;
                goto finish;
            }

            pKernelAllocation->cloneKernelParams.dwOffsetForAllocID  = pKernelAllocation->dwOffset;
            pKernelAllocation->dwOffset                              = pStateHeap->pKernelAllocation[headKernelAllocationID].dwOffset;
            pKernelAllocation->cloneKernelParams.isClone             = true;
            pKernelAllocation->cloneKernelParams.kernelBinaryAllocID = headKernelAllocationID;
            pKernelAllocation->cloneKernelParams.cloneKernelID       = pStateHeap->pKernelAllocation[headKernelAllocationID].iKUID;

            pStateHeap->pKernelAllocation[headKernelAllocationID].cloneKernelParams.referenceCount = pStateHeap->pKernelAllocation[headKernelAllocationID].cloneKernelParams.referenceCount + 1;

            // update head kernel's dwCount after updating the clone entry's dwCount so that clone will be selected for deletion first
            pStateHeap->pKernelAllocation[headKernelAllocationID].dwCount = pStateHeap->dwAccessCounter++;
        }
        else if (isHeadKernel)
        {
            pKernelAllocation->cloneKernelParams.isHeadKernel = true;
            if (isCloneAsHead)
            {
                pKernelAllocation->cloneKernelParams.cloneKernelID = pKernelParam->ClonedKernelParam.kernelID;
            }
        }
    }

finish:
    return hr;
}

/*----------------------------------------------------------------------------
| Name      : HalCm_UnLoadKernel ( Replace RenderHal_UnloadKernel)
\---------------------------------------------------------------------------*/
int32_t HalCm_UnloadKernel(
    PCM_HAL_STATE              pState,
    PRENDERHAL_KRN_ALLOCATION  pKernelAllocation)
{
    PRENDERHAL_INTERFACE       pRenderHal = pState->pRenderHal;
    PRENDERHAL_STATE_HEAP      pStateHeap;
    int32_t                    hr;


    //---------------------------------------
    CMCHK_NULL(pRenderHal);
    CMCHK_NULL(pRenderHal->pStateHeap);
    CMCHK_NULL(pKernelAllocation);
    //---------------------------------------

    hr      = CM_FAILURE;
    pStateHeap = pRenderHal->pStateHeap;

    if (pKernelAllocation->dwFlags == RENDERHAL_KERNEL_ALLOCATION_FREE)
    {
        goto finish;
    }

    CMCHK_HR(HalCm_SyncKernel(pState, pKernelAllocation->dwSync));
    
    // Unload kernel
    if (pKernelAllocation->pMhwKernelParam)
    {
        pKernelAllocation->pMhwKernelParam->bLoaded = 0;
    }

    if (pKernelAllocation->cloneKernelParams.isClone)
    {
        if (pStateHeap->pKernelAllocation[pKernelAllocation->cloneKernelParams.kernelBinaryAllocID].cloneKernelParams.isHeadKernel)
        {
            if ((pStateHeap->pKernelAllocation[pKernelAllocation->cloneKernelParams.kernelBinaryAllocID].cloneKernelParams.referenceCount) <= 0)
            {
                // ERROR
                hr = CM_FAILURE;
                goto finish;
            }
        }
        else
        {
            // ERROR
            hr = CM_FAILURE;
            goto finish;
        }

        pStateHeap->pKernelAllocation[pKernelAllocation->cloneKernelParams.kernelBinaryAllocID].cloneKernelParams.referenceCount = 
            pStateHeap->pKernelAllocation[pKernelAllocation->cloneKernelParams.kernelBinaryAllocID].cloneKernelParams.referenceCount - 1;

        // restore the dwOffset for this allocationID
        pKernelAllocation->dwOffset = pKernelAllocation->cloneKernelParams.dwOffsetForAllocID;
    }
    else if (pKernelAllocation->cloneKernelParams.isHeadKernel && pKernelAllocation->cloneKernelParams.referenceCount != 0)
    {
        // ERROR, cloned kernel entries should have been selected for deletion before head kernel entry
        hr = CM_FAILURE;
        goto finish;
    }

    // Release kernel entry (Offset/size may be used for reallocation)
    pKernelAllocation->iKID     = -1;
    pKernelAllocation->iKUID    = -1;
    pKernelAllocation->iKCID    = -1;
    pKernelAllocation->dwSync   = 0;
    pKernelAllocation->dwFlags          = RENDERHAL_KERNEL_ALLOCATION_FREE;
    pKernelAllocation->dwCount  = 0;
    pKernelAllocation->pMhwKernelParam  = nullptr;
    pKernelAllocation->cloneKernelParams.cloneKernelID       = -1;
    pKernelAllocation->cloneKernelParams.isClone             = false;
    pKernelAllocation->cloneKernelParams.isHeadKernel        = false;
    pKernelAllocation->cloneKernelParams.kernelBinaryAllocID = -1;
    pKernelAllocation->cloneKernelParams.referenceCount      = 0;

    hr = CM_SUCCESS;

finish:
    return hr;
}

/*----------------------------------------------------------------------------
| Name      : HalCmw_TouchKernel ( Replace RenderHal_TouchKernel)
\---------------------------------------------------------------------------*/
int32_t HalCm_TouchKernel(
    PCM_HAL_STATE       pState,
    int32_t             iKernelAllocationID)
{
    int32_t                     hr = CM_SUCCESS;
    PRENDERHAL_STATE_HEAP       pStateHeap;
    PRENDERHAL_KRN_ALLOCATION   pKernelAllocation;
    PRENDERHAL_KRN_ALLOCATION   pHeadKernelAllocation;
    uint32_t                    tag;

    PRENDERHAL_INTERFACE pRenderHal = pState->pRenderHal;
    PMOS_INTERFACE pOsInterface     = pState->pOsInterface;

    pStateHeap = (pRenderHal) ? pRenderHal->pStateHeap : nullptr;
    if (pStateHeap == nullptr ||
        pStateHeap->pKernelAllocation == nullptr ||
        iKernelAllocationID < 0 ||
        iKernelAllocationID >= pRenderHal->StateHeapSettings.iKernelCount)
    {
        hr = CM_FAILURE;
        goto finish;
    }

    // Update usage
    pKernelAllocation = &(pStateHeap->pKernelAllocation[iKernelAllocationID]);
    if (pKernelAllocation->dwFlags != RENDERHAL_KERNEL_ALLOCATION_FREE &&
        pKernelAllocation->dwFlags != RENDERHAL_KERNEL_ALLOCATION_LOCKED)
    {
        pKernelAllocation->dwCount = pStateHeap->dwAccessCounter++;
    }

    // Set sync tag, for deallocation control
    if(pState->bCBBEnabled)
    {
        tag = pOsInterface->pfnGetGpuStatusTag(pOsInterface, pOsInterface->CurrentGpuContextOrdinal);
    }
    else
    {
        tag = pStateHeap->dwNextTag;
    }

    pKernelAllocation->dwSync = tag;

    // if this kernel allocation is a cloned kernel, update the orig kernel sync tag and access counter
    if (pKernelAllocation->cloneKernelParams.isClone)
    {
        pHeadKernelAllocation = &(pStateHeap->pKernelAllocation[pKernelAllocation->cloneKernelParams.kernelBinaryAllocID]);

        if (pHeadKernelAllocation->cloneKernelParams.referenceCount <= 0)
        {
            // ERROR
            hr = CM_FAILURE;
            goto finish;
        }

        pHeadKernelAllocation->dwSync = tag;
        pHeadKernelAllocation->dwCount = pStateHeap->dwAccessCounter++;

    }

finish:
    return hr;
}

/*
**  Supporting function
**  Delete oldest entry from table to free more space
**  According to different cases, we will combine space with previous or next slot to get max space
*/
int32_t CmDeleteOldestKernel(PCM_HAL_STATE pState, MHW_KERNEL_PARAM *pMhwKernelParam)
{
    PRENDERHAL_KRN_ALLOCATION  pKernelAllocation;
    PRENDERHAL_INTERFACE       pRenderHal = pState->pRenderHal;;
    PRENDERHAL_STATE_HEAP      pStateHeap = pRenderHal->pStateHeap;
    UNUSED(pState);
    UNUSED(pMhwKernelParam);

    uint32_t dwOldest = 0;
    uint32_t dwLastUsed;
    int32_t iKernelAllocationID, iSearchIndex = -1, index = -1;
    int32_t alignedSize, shiftOffset;
    int32_t hr = CM_SUCCESS;

    pKernelAllocation   = pStateHeap->pKernelAllocation;

    // Search and deallocate oldest kernel (most likely this is optimal scheduling algorithm)
    pKernelAllocation = pStateHeap->pKernelAllocation;
    for (iKernelAllocationID = 0;
        iKernelAllocationID < pState->nNumKernelsInGSH;
        iKernelAllocationID++, pKernelAllocation++)
    {
        // Skip unused entries
        // Skip kernels flagged as locked (cannot be automatically deallocated)
        if (pKernelAllocation->dwFlags == RENDERHAL_KERNEL_ALLOCATION_FREE ||
            pKernelAllocation->dwFlags == RENDERHAL_KERNEL_ALLOCATION_LOCKED)
        {
            continue;
        }

        // Find kernel not used for the greater amount of time (measured in number of operations)
        // Must not unload recently allocated kernels
        dwLastUsed = (uint32_t)(pStateHeap->dwAccessCounter - pKernelAllocation->dwCount);
        if (dwLastUsed > dwOldest)
        {
            iSearchIndex = iKernelAllocationID;
            dwOldest     = dwLastUsed;
        }
    }

    // Did not found any entry for deallocation, we get into a strange case!
    if (iSearchIndex < 0)
    {
        CM_ERROR_ASSERT("Failed to delete any slot from GSH. It is impossible.");
        iKernelAllocationID = RENDERHAL_KERNEL_LOAD_FAIL;
        return CM_FAILURE;
    }

    if (pStateHeap->pKernelAllocation[iSearchIndex].cloneKernelParams.isHeadKernel &&
        (pStateHeap->pKernelAllocation[iSearchIndex].cloneKernelParams.referenceCount != 0))
    {
        // ERROR, chose a head kernel for deletion but it still has clones pointing to it
        return CM_FAILURE;
    }

    // Free kernel entry and states associated with the kernel (if any)
    pKernelAllocation = &pStateHeap->pKernelAllocation[iSearchIndex];
    if (HalCm_UnloadKernel(pState, pKernelAllocation) != CM_SUCCESS)
    {
        CM_ERROR_ASSERT("Failed to load kernel - no space available in GSH.");
        iKernelAllocationID = RENDERHAL_KERNEL_LOAD_FAIL;
        return CM_FAILURE;
    }

    // Let's check if we can merge iSearchIndex-1, iSearchIndex, iSearchIndex+1
    index = iSearchIndex;
    PRENDERHAL_KRN_ALLOCATION pKAlloc0, pKAlloc1, pKAlloc2;
    pKAlloc0 = (index == 0)? nullptr : &pStateHeap->pKernelAllocation[index-1];
    pKAlloc1 = &pStateHeap->pKernelAllocation[index];  // free one
    pKAlloc2 = (index == pState->CmDeviceParam.iMaxGSHKernelEntries - 1) ? nullptr : &pStateHeap->pKernelAllocation[index + 1];

    if (bIsFree(pKAlloc0) && bIsFree(pKAlloc2))
    {
        // merge 3 into 1 slot and bump index after
        pStateHeap->pKernelAllocation[index-1].dwFlags = RENDERHAL_KERNEL_ALLOCATION_FREE;
        pState->pTotalKernelSize[index-1] += pState->pTotalKernelSize[index] + pState->pTotalKernelSize[index+1];
        pStateHeap->pKernelAllocation[index-1].iSize = 0;
        // no change for pStateHeap->pKernelAllocation[index-1].dwOffset

        // copy the rest
        for (int32_t i = index + 2; i<pState->nNumKernelsInGSH; i++)
        {
            pStateHeap->pKernelAllocation[i-2] = pStateHeap->pKernelAllocation[i];
            pState->pTotalKernelSize[i-2] = pState->pTotalKernelSize[i];
        }

        pState->nNumKernelsInGSH -= 2;

        if ( index == 0 )
            HalCm_UpdateCloneKernel(pState, 0, CM_SHIFT_RIGHT, 2);
        else
            HalCm_UpdateCloneKernel(pState, index - 1, CM_SHIFT_RIGHT, 2);
    }
    else if (bIsFree(pKAlloc0))
    {
        // merge before and current into 1 slot
        pStateHeap->pKernelAllocation[index-1].dwFlags = RENDERHAL_KERNEL_ALLOCATION_FREE;
        pState->pTotalKernelSize[index-1] += pState->pTotalKernelSize[index];
        pStateHeap->pKernelAllocation[index-1].iSize = 0;
        // no change for pStateHeap->pKernelAllocation[index-1].dwOffset

        for (int32_t i = index + 1; i<pState->nNumKernelsInGSH; i++)
        {
            pStateHeap->pKernelAllocation[i-1] = pStateHeap->pKernelAllocation[i];
            pState->pTotalKernelSize[i-1] = pState->pTotalKernelSize[i];
        }

        pState->nNumKernelsInGSH -= 1;

        if ( index == 0 )
            HalCm_UpdateCloneKernel(pState, 0, CM_SHIFT_RIGHT, 1);
        else
            HalCm_UpdateCloneKernel(pState, index - 1, CM_SHIFT_RIGHT, 1);

    } 
    else if (bIsFree(pKAlloc2))
    {
        // pKAlloc0 is not free, but it can be nullptr
        // merge after and current into 1 slot
        pStateHeap->pKernelAllocation[index].dwFlags = RENDERHAL_KERNEL_ALLOCATION_FREE;
        pState->pTotalKernelSize[index] += pState->pTotalKernelSize[index+1];
        pStateHeap->pKernelAllocation[index].iSize = 0;
        if (pKAlloc0)
        {
            // get free space starting point
            alignedSize = MOS_ALIGN_CEIL(pKAlloc0->iSize, 64);
            shiftOffset = pState->pTotalKernelSize[index-1] - alignedSize;

            pState->pTotalKernelSize[index-1] -= shiftOffset;
            // no change for pStateHeap->pKernelAllocation[index-1].iSize -= 0;
            pState->pTotalKernelSize[index] += shiftOffset;
            pStateHeap->pKernelAllocation[index].dwOffset -= shiftOffset;
        }

        for (int32_t i = index + 1; i<pState->nNumKernelsInGSH; i++)
        {
            pStateHeap->pKernelAllocation[i] = pStateHeap->pKernelAllocation[i+1];
            pState->pTotalKernelSize[i] = pState->pTotalKernelSize[i+1];
        }

        pState->nNumKernelsInGSH -= 1;

        if ( index == 0 )
            HalCm_UpdateCloneKernel(pState, 0, CM_SHIFT_RIGHT, 1);
        else
            HalCm_UpdateCloneKernel(pState, index - 1, CM_SHIFT_RIGHT, 1);
    }
    else
    {
        // no merge
        pStateHeap->pKernelAllocation[index].dwFlags = RENDERHAL_KERNEL_ALLOCATION_FREE;
        // no change for pStateHeap->pKernelAllocation[index].iTotalSize;
        pStateHeap->pKernelAllocation[index].iSize = 0;
        if(pKAlloc0)
        {
            // get free space starting point
            alignedSize = MOS_ALIGN_CEIL(pKAlloc0->iSize, 64);
            shiftOffset = pState->pTotalKernelSize[index-1] - alignedSize;
            pState->pTotalKernelSize[index-1] -= shiftOffset;
            // no change for pStateHeap->pKernelAllocation[index-1].iSize -= 0;
            pState->pTotalKernelSize[index] += shiftOffset;
            pStateHeap->pKernelAllocation[index].dwOffset -= shiftOffset;
        }
        // no change for pStateHeap->iNumKernels;
    }

    return hr;
}

/*----------------------------------------------------------------------------
| Name      : HalCm_LoadKernel ( Replace RenderHal_LoadKernel)
\---------------------------------------------------------------------------*/
int32_t HalCm_LoadKernel(
    PCM_HAL_STATE             pState,
    PCM_HAL_KERNEL_PARAM      pKernelParam,
    int32_t                   iSamplerCount,
    PRENDERHAL_KRN_ALLOCATION &pKernelAllocation)
{
    PRENDERHAL_STATE_HEAP     pStateHeap;
    PRENDERHAL_INTERFACE      pRenderHal;
    int32_t                 hr;
    PRENDERHAL_KERNEL_PARAM   pParameters;
    PMHW_KERNEL_PARAM         pMhwKernelParam;

    int32_t iKernelAllocationID;    // Kernel allocation ID in GSH
    int32_t iKernelCacheID;         // Kernel cache ID
    int32_t iKernelUniqueID;        // Kernel unique ID
    void    *pKernelPtr;
    int32_t iKernelSize;
    int32_t iSearchIndex;
    int32_t iFreeSlot;
    bool    isClonedKernel;
    bool    hasClones;

    hr                  = CM_SUCCESS;
    pRenderHal          = pState->pRenderHal;
    pStateHeap          = (pRenderHal) ? pRenderHal->pStateHeap : nullptr;
    iKernelAllocationID = RENDERHAL_KERNEL_LOAD_FAIL;
    pMhwKernelParam     = &(pState->KernelParams_Mhw);
    pParameters         = &(pState->KernelParams_RenderHal.Params);

    // Validate parameters
    if (pStateHeap == nullptr ||
        pStateHeap->bIshLocked == false ||
        pStateHeap->pKernelAllocation == nullptr ||
        pKernelParam->iKernelBinarySize == 0 ||
        pState->nNumKernelsInGSH > pState->CmDeviceParam.iMaxGSHKernelEntries)
    {
        CM_ERROR_ASSERT("Failed to load kernel - invalid parameters.");
        return CM_FAILURE;
    }

    isClonedKernel = pKernelParam->ClonedKernelParam.isClonedKernel;
    hasClones      = pKernelParam->ClonedKernelParam.hasClones;

    pParameters->Sampler_Count = iSamplerCount;
    pMhwKernelParam->iKUID     = static_cast<int>( (pKernelParam->uiKernelId >> 32) );
    pMhwKernelParam->iKCID     = -1;
    pMhwKernelParam->pBinary   = pKernelParam->pKernelBinary;
    pMhwKernelParam->iSize     = pKernelParam->iKernelBinarySize + CM_KERNEL_BINARY_PADDING_SIZE;

    // Kernel parameters
    pKernelPtr      = pMhwKernelParam->pBinary;
    iKernelSize     = pMhwKernelParam->iSize;
    iKernelUniqueID = pMhwKernelParam->iKUID;
    iKernelCacheID  = pMhwKernelParam->iKCID;

    // Check if kernel is already loaded; Search free allocation index
    iSearchIndex = -1;
    pKernelAllocation = pStateHeap->pKernelAllocation;
    for (iKernelAllocationID = 0;
         iKernelAllocationID <  pState->nNumKernelsInGSH;
         iKernelAllocationID++, pKernelAllocation++)
    {
        if (pKernelAllocation->iKUID == iKernelUniqueID &&
            pKernelAllocation->iKCID == iKernelCacheID)
        {
            // found match and Update kernel usage
            hr = HalCm_TouchKernel(pState, iKernelAllocationID);
            if (hr == CM_FAILURE)
            {
                goto finish;
            }
            // Increment reference counter
            pMhwKernelParam->bLoaded = 1;
            // Record kernel allocation
            pKernelAllocation = &pStateHeap->pKernelAllocation[iKernelAllocationID];

            goto finish;
        }
    }

    // JG need to integrate cloneKernel to DSH
    if (isClonedKernel || hasClones)
    {
        hr = HalCm_InsertCloneKernel(pState, pKernelParam, pKernelAllocation);
        goto finish;
    }
    // JG

    // here is the algorithm
    // 1) search for free slot which is big enough to load current kerenel
    // 2) if found slot, then add current kerenel
    // 3) if we cannot find slot, we need to delete some entry (delete oldest first), after delete oldest entry 
    //    we will loop over to step 1 until we get enough space. 
    // The algorithm won't fail except we load 1 kernel which is larger than 2MB
    do 
    {
        iFreeSlot = CmSearchFreeSlotSize(pState, pMhwKernelParam, false);
        if (iFreeSlot >= 0)
        {
            // found free slot which is big enough to hold kernel
            hr = CmAddCurrentKernelToFreeSlot(pState, iFreeSlot, pParameters, pKernelParam, pMhwKernelParam, CM_NO_CLONE, -1);
            // update GSH states pStateHeap->numKernels inside add function
            break;
        }
        else
        {
            if (CmDeleteOldestKernel(pState, pMhwKernelParam) != CM_SUCCESS)
            {
                return CM_FAILURE;
            }
        }
    } while(1);

    pMhwKernelParam->bLoaded = 1;  // Increment reference counter
    pKernelAllocation = &pStateHeap->pKernelAllocation[iFreeSlot];  // Record kernel allocation

finish:

    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Loads cloned kernel entries and kernels with clones into free slot 
//| Return:     Result of the operation
//*-----------------------------------------------------------------------------
int32_t HalCm_InsertCloneKernel(
    PCM_HAL_STATE              pState,
    PCM_HAL_KERNEL_PARAM       pKernelParam,
    PRENDERHAL_KRN_ALLOCATION  &pKernelAllocation)
{
    int32_t                   hr              = CM_SUCCESS;
    int32_t                   iKernelAllocationID;    // Kernel allocation ID in GSH
    uint32_t                  tag;
    PMOS_INTERFACE            pOsInterface    = pState->pOsInterface;
    PMHW_KERNEL_PARAM         pMhwKernelParam = &(pState->KernelParams_Mhw);
    int32_t                   iFreeSlot       = -1;
    PRENDERHAL_STATE_HEAP     pStateHeap = pState->pRenderHal->pStateHeap;

    pKernelAllocation = pState->pRenderHal->pStateHeap->pKernelAllocation;

    for (iKernelAllocationID = 0; iKernelAllocationID < pState->nNumKernelsInGSH;
        iKernelAllocationID++, pKernelAllocation++)
    {
        if (pKernelAllocation->cloneKernelParams.isHeadKernel)
        {
            if ((pKernelAllocation->iKUID                           == pKernelParam->ClonedKernelParam.kernelID) ||       // original kernel that cloned from is already loaded as head
                (pKernelAllocation->cloneKernelParams.cloneKernelID == pKernelParam->ClonedKernelParam.kernelID) ||       // another clone from same original kernel is serving as the head
                (pKernelAllocation->cloneKernelParams.cloneKernelID == static_cast<int>(pKernelParam->uiKernelId >> 32))) // clone is serving as the head and this is the original kernel
            {
                // found match, insert 64B dummy entry and set piKAID
                do
                {
                    // Before getting a free slot, update head kernel sync tag and dwCount so head will not be selected for deletion
                    // then update head kernel dwCount after inserting clone 
                    // so that clone will be selected first for deletion (this is done in CmAddCurrentKernelToFreeSlot)

                    // update head kernel sync tag
                    if(pState->bCBBEnabled)
                    {
                        tag = pOsInterface->pfnGetGpuStatusTag(pOsInterface, pOsInterface->CurrentGpuContextOrdinal);
                    }
                    else
                    {
                        tag = pState->pRenderHal->pStateHeap->dwNextTag;
                    }
                    pKernelAllocation->dwSync = tag;

                    // update the head kernel dwCount so it will not be selected for deletion
                    pKernelAllocation->dwCount = pState->pRenderHal->pStateHeap->dwAccessCounter++;

                    iFreeSlot = CmSearchFreeSlotSize(pState, pMhwKernelParam, true);
                    if (iFreeSlot >= 0)
                    {
                        // found free slot
                        hr = CmAddCurrentKernelToFreeSlot(pState, iFreeSlot, &(pState->KernelParams_RenderHal.Params),
                            pKernelParam, &(pState->KernelParams_Mhw), CM_CLONE_ENTRY, iKernelAllocationID);

                        goto finish;

                    }
                    else
                    {
                        if (CmDeleteOldestKernel(pState, pMhwKernelParam) != CM_SUCCESS)
                        {
                            hr = CM_FAILURE;
                            goto finish;
                        }
                    }
                } while (1);
            }
        }
    }

    // didn't find a match, insert this kernel as the head kernel
    do
    {
        iFreeSlot = CmSearchFreeSlotSize(pState, pMhwKernelParam, false);
        if (iFreeSlot >= 0)
        {
            if (pKernelParam->ClonedKernelParam.isClonedKernel)
            {
                hr = CmAddCurrentKernelToFreeSlot(pState, iFreeSlot, &(pState->KernelParams_RenderHal.Params),
                    pKernelParam, &(pState->KernelParams_Mhw), CM_CLONE_AS_HEAD_KERNEL, -1);
            }
            else
            {
                hr = CmAddCurrentKernelToFreeSlot(pState, iFreeSlot, &(pState->KernelParams_RenderHal.Params),
                    pKernelParam, &(pState->KernelParams_Mhw), CM_HEAD_KERNEL, -1);
            }
            break;
        }
        else
        {
            if (CmDeleteOldestKernel(pState, pMhwKernelParam) != CM_SUCCESS)
            {
                hr = CM_FAILURE;
                goto finish;
            }
        }
    } while (1);

finish:

    if (hr == CM_SUCCESS)
    {
        pMhwKernelParam->bLoaded = 1;
        pKernelAllocation = &pStateHeap->pKernelAllocation[iFreeSlot];
    }

    return hr;
}

//!
//! \brief    Get offset and/or pointer to sampler state
//! \details  Get offset and/or pointer to sampler state in General State Heap,
//!           (Cm customized version of the RenderHal function which calculates 
//!           the sampler offset by MDF owned parameters).
//! \param    PCM_HAL_STATE pState
//!           [in] Pointer to CM_HAL_STATE structure
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to RenderHal Interface
//! \param    int iMediaID
//!           [in] Media ID associated with sampler
//! \param    int iSamplerOffset
//!           [in] sampler offset from the base of current kernel's sampler heap
//! \param    int iSamplerBTI
//!           [in] sampler BTI
//! \param    unsigned long *pdwSamplerOffset
//!           [out] optional; offset of sampler state from GSH base
//! \param    void **ppSampler
//!           [out] optional; pointer to sampler state in GSH
//! \return   MOS_STATUS
//!
MOS_STATUS HalCm_GetSamplerOffsetAndPtr(
    PCM_HAL_STATE            pState,
    PRENDERHAL_INTERFACE     pRenderHal,
    int                      iMediaID,
    unsigned int             iSamplerOffset,
    unsigned int             iSamplerBTI,
    PMHW_SAMPLER_STATE_PARAM pSamplerParam,
    unsigned long           *pdwSamplerOffset,
    void                   **ppSampler)
{
    unsigned int sampler_offset = pRenderHal->pStateHeap->pCurMediaState->pDynamicState->pMemoryBlock->dwDataOffset +
                                  pRenderHal->pStateHeap->pCurMediaState->pDynamicState->Sampler3D.dwOffset +
                                  pState->pTaskParam->sampler_offsets_by_kernel[iMediaID] +
                                  iSamplerOffset;
    if (pdwSamplerOffset != nullptr)
    {
        *pdwSamplerOffset = sampler_offset;
    }

    *ppSampler = (void *)((unsigned char *)pRenderHal->pStateHeap->pCurMediaState->pDynamicState->pMemoryBlock->pStateHeap->pvLockedHeap +
                                sampler_offset);

    if (pSamplerParam->SamplerType == MHW_SAMPLER_TYPE_3D)
    {
        pSamplerParam->Unorm.IndirectStateOffset = MOS_ALIGN_CEIL(pRenderHal->pStateHeap->pCurMediaState->pDynamicState->pMemoryBlock->dwDataOffset +
                                                                  pRenderHal->pStateHeap->pCurMediaState->pDynamicState->Sampler3D.dwOffset +
                                                                  pState->pTaskParam->sampler_indirect_offsets_by_kernel[iMediaID] +
                                                                  iSamplerBTI * pRenderHal->pHwSizes->dwSizeSamplerIndirectState,
                                                                  1 << MHW_SAMPLER_INDIRECT_SHIFT);
        pSamplerParam->Unorm.pIndirectState = (void *)((unsigned char *)pRenderHal->pStateHeap->pCurMediaState->pDynamicState->pMemoryBlock->pStateHeap->pvLockedHeap +
                                                             pSamplerParam->Unorm.IndirectStateOffset);
    }
    return MOS_STATUS_SUCCESS;
}

//!
//! \brief      Setup Interface Descriptor
//! \details    Set interface descriptor, (overriding RenderHal function),
//!             (Cm customized version of the RenderHal function which set
//!             dwSamplerOffset and dwSamplerCount by MDF owned parameters).
//! \param      PCM_HAL_STATE                           pState
//!             [in]    Pointer to CM_HAL_STATE structure
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
MOS_STATUS HalCm_SetupInterfaceDescriptor(
    PCM_HAL_STATE                          pState,
    PRENDERHAL_INTERFACE                   pRenderHal,
    PRENDERHAL_MEDIA_STATE                 pMediaState,
    PRENDERHAL_KRN_ALLOCATION              pKernelAllocation,
    PRENDERHAL_INTERFACE_DESCRIPTOR_PARAMS pInterfaceDescriptorParams)
{
    MOS_STATUS               eStatus = MOS_STATUS_SUCCESS;
    MHW_ID_ENTRY_PARAMS      Params;
    PRENDERHAL_STATE_HEAP    pStateHeap;
    PRENDERHAL_DYNAMIC_STATE pDynamicState;
    unsigned long            dwMediaStateOffset;

    //-----------------------------------------
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pMhwStateHeap);
    MHW_RENDERHAL_CHK_NULL(pMediaState);
    MHW_RENDERHAL_CHK_NULL(pMediaState->pDynamicState);
    MHW_RENDERHAL_CHK_NULL(pMediaState->pDynamicState->pMemoryBlock);
    MHW_RENDERHAL_CHK_NULL(pKernelAllocation);
    MHW_RENDERHAL_CHK_NULL(pKernelAllocation->pMemoryBlock);
    MHW_RENDERHAL_CHK_NULL(pInterfaceDescriptorParams);
    //-----------------------------------------

    // Get states, params
    pStateHeap = pRenderHal->pStateHeap;
    pDynamicState = pMediaState->pDynamicState;
    dwMediaStateOffset = pDynamicState->pMemoryBlock->dwDataOffset;

    Params.dwMediaIdOffset = dwMediaStateOffset + pDynamicState->MediaID.dwOffset;
    Params.iMediaId = pInterfaceDescriptorParams->iMediaID;
    Params.dwKernelOffset = pKernelAllocation->dwOffset;
    Params.dwSamplerOffset = dwMediaStateOffset + pDynamicState->Sampler3D.dwOffset + pState->pTaskParam->sampler_offsets_by_kernel[Params.iMediaId];
    Params.dwSamplerCount = ( pState->pTaskParam->sampler_counts_by_kernel[Params.iMediaId] + 3 ) / 4;
    Params.dwSamplerCount = (Params.dwSamplerCount > 4) ? 4 : Params.dwSamplerCount;
    Params.dwBindingTableOffset = pInterfaceDescriptorParams->iBindingTableID * pStateHeap->iBindingTableSize;
    Params.iCurbeOffset = pInterfaceDescriptorParams->iCurbeOffset;
    Params.iCurbeLength = pInterfaceDescriptorParams->iCurbeLength;

    Params.bBarrierEnable = pInterfaceDescriptorParams->blBarrierEnable;
    Params.bGlobalBarrierEnable = pInterfaceDescriptorParams->blGlobalBarrierEnable;    //It's only applied for BDW+
    Params.dwNumberofThreadsInGPGPUGroup = pInterfaceDescriptorParams->iNumberThreadsInGroup;
    Params.dwSharedLocalMemorySize = pRenderHal->pfnEncodeSLMSize(pRenderHal, pInterfaceDescriptorParams->iSLMSize);
    Params.iCrsThdConDataRdLn = pInterfaceDescriptorParams->iCrsThrdConstDataLn;
    Params.pGeneralStateHeap = pDynamicState->pMemoryBlock->pStateHeap;

    MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwStateHeap->SetInterfaceDescriptorEntry(&Params));
    pDynamicState->MediaID.iCurrent++;

finish:
    return eStatus;
}

/*----------------------------------------------------------------------------
| Name      : HalCm_AllocateMediaID  replace old RenderHal_AllocateMediaID
|             Don't need touch kernel since we handle this a loadKernel time
|
| Purpose   : Allocates an setup Interface Descriptor for Media Pipeline
|
| Arguments : [in] pRenderHal          - Pointer to RenderHal interface structure
|             [in] pKernelParam        - Pointer to Kernel parameters
|             [in] pKernelAllocationID - Pointer to Kernel allocation
|             [in] iBindingTableID     - Binding table ID
|             [in] iCurbeOffset        - Curbe offset (from CURBE base)
|
| Returns   : Media Interface descriptor ID
|             -1 if invalid parameters
|                   no Interface Descriptor entry available in GSH
|
| Comments  : Kernel        must be preloaded
|             Curbe         must be allocated using pfnAllocateCurbe
|             Binding Table must be allocated using pfnAllocateBindingTable
\---------------------------------------------------------------------------*/
//!
//! \brief
//! \details
//! \param    PRENDERHAL_INTERFACE       pRenderHal
//| \param    PCM_HAL_KERNEL_PARAM       pKernelParam
//| \param    PRENDERHAL_KRN_ALLOCATION  pKernelAllocation
//| \param    int32_t                    iBindingTableID
//| \param    int32_t                    iCurbeOffset
//! \return   int32_t
//!
int32_t HalCm_AllocateMediaID(
    PCM_HAL_STATE               pState,
    PCM_HAL_KERNEL_PARAM        pKernelParam,
    PRENDERHAL_KRN_ALLOCATION   pKernelAllocation,
    int32_t                    iBindingTableID,
    int32_t                    iCurbeOffset)
{
    PRENDERHAL_INTERFACE    pRenderHal = pState->pRenderHal;
    PRENDERHAL_MEDIA_STATE  pCurMediaState;
    int32_t                 iCurbeSize, iCurbeCurrent;
    int32_t                 iInterfaceDescriptor;
    RENDERHAL_INTERFACE_DESCRIPTOR_PARAMS InterfaceDescriptorParams;

    iInterfaceDescriptor = -1;

    // Obtain pointer and validate current media state
    pCurMediaState = pRenderHal->pStateHeap->pCurMediaState;

    if (pState->bDynamicStateHeap)
    {
        if (pCurMediaState == nullptr || (pState->bDynamicStateHeap && (pCurMediaState->pDynamicState == nullptr)))
        {
            CM_ASSERTMESSAGE("Invalid Media State.");
            goto finish;
        }
    }
    else
    {
        if (pCurMediaState == nullptr)
        {
            CM_ASSERTMESSAGE("Invalid Media State.");
            goto finish;
        }
    }

    // Validate kernel allocation (kernel must be pre-loaded into GSH)
    if (!pKernelAllocation ||
        pKernelAllocation->dwFlags == RENDERHAL_KERNEL_ALLOCATION_FREE ||
        pKernelAllocation->iSize == 0)
    {
        CM_ASSERTMESSAGE("Error: Invalid Kernel Allocation.");
        goto finish;
    }

    // Check Curbe allocation (CURBE_Lenght is in 256-bit count -> convert to bytes)
    iCurbeSize = pKernelParam->iCurbeSizePerThread;

    if (pState->bDynamicStateHeap)
    {
        iCurbeCurrent = pCurMediaState->pDynamicState->Curbe.iCurrent;
    }
    else
    {
        iCurbeCurrent = pCurMediaState->iCurbeOffset;
    }

    if (iCurbeSize <= 0)
    {
        // Curbe is not used by the kernel
        iCurbeSize = iCurbeOffset = 0;
    }
    // Validate Curbe Offset (curbe must be pre-allocated)
    else if ( iCurbeOffset < 0 ||                                       // Not allocated
             (iCurbeOffset & 0x1F) != 0 ||                              // Invalid alignment
             (iCurbeOffset + iCurbeSize) > iCurbeCurrent)               // Invalid size
    {
        CM_ASSERTMESSAGE("Error: Invalid Curbe Allocation.");
        goto finish;
    }

    // Try to reuse interface descriptor (for 2nd level buffer optimizations)
    // Check if ID already in use by another kernel - must use a different ID
    iInterfaceDescriptor = pRenderHal->pfnGetMediaID(pRenderHal, pCurMediaState, pKernelAllocation);
    if (iInterfaceDescriptor < 0)
    {
        CM_ASSERTMESSAGE("Error: No Interface Descriptor available.");
        goto finish;
    }

    InterfaceDescriptorParams.iMediaID            = iInterfaceDescriptor;
    InterfaceDescriptorParams.iBindingTableID     = iBindingTableID;  

    //CURBE size and offset setting
    //Media w/o group: only per-thread CURBE is used, CrossThread CURBE is not used.
    //Media w/ group: should follow GPGPU walker setting, there is per-thread CURBE and cross-thread CURBE. But per-thread CURBE should be ZERO, and all should be cross-thread CURBE
    //GPGPU: both per-thread CURBE and cross-thread CURBE need be set.
    InterfaceDescriptorParams.iCurbeOffset = iCurbeOffset;
    if ((!pKernelParam->GpGpuWalkerParams.CmGpGpuEnable) && (pKernelParam->KernelThreadSpaceParam.groupSelect == CM_MW_GROUP_NONE) && (pState->pTaskParam->MediaWalkerGroupSelect == CM_MW_GROUP_NONE))
    {   //Media pipe without group
        InterfaceDescriptorParams.iCurbeLength          = pKernelParam->iCurbeSizePerThread;
        InterfaceDescriptorParams.iCrsThrdConstDataLn   = pKernelParam->iCrsThrdConstDataLn;    //should always be 0 in this case
        InterfaceDescriptorParams.iNumberThreadsInGroup = (pKernelParam->iNumberThreadsInGroup > 0) ? pKernelParam->iNumberThreadsInGroup : 1;  // This field should not be set to 0 even if the barrier is disabled, since an accurate value is needed for proper pre-emption.
        InterfaceDescriptorParams.blGlobalBarrierEnable = false;
        InterfaceDescriptorParams.blBarrierEnable       = false;
        InterfaceDescriptorParams.iSLMSize              = 0;
    }
    else if ((!pKernelParam->GpGpuWalkerParams.CmGpGpuEnable) && ((pKernelParam->KernelThreadSpaceParam.groupSelect != CM_MW_GROUP_NONE) || (pState->pTaskParam->MediaWalkerGroupSelect != CM_MW_GROUP_NONE)))
    {   //Media w/ group
        InterfaceDescriptorParams.iCurbeLength          = 0;                                    //No using per-thread CURBE
        InterfaceDescriptorParams.iCrsThrdConstDataLn   = pKernelParam->iCurbeSizePerThread;    //treat all CURBE as cross-thread CURBE
        InterfaceDescriptorParams.iNumberThreadsInGroup = (pKernelParam->iNumberThreadsInGroup > 0) ? pKernelParam->iNumberThreadsInGroup : 1;  // This field should not be set to 0 even if the barrier is disabled, since an accurate value is needed for proper pre-emption.
        InterfaceDescriptorParams.blBarrierEnable       = (pKernelParam->iBarrierMode != CM_NO_BARRIER) ? true : false;
        InterfaceDescriptorParams.blGlobalBarrierEnable = (pKernelParam->iBarrierMode == CM_GLOBAL_BARRIER) ? true : false;
        InterfaceDescriptorParams.iSLMSize              = pKernelParam->iSLMSize;
    }
    else
    {   //GPGPU pipe
        InterfaceDescriptorParams.iCurbeLength          = pKernelParam->iCurbeSizePerThread;
        InterfaceDescriptorParams.iCrsThrdConstDataLn   = pKernelParam->iCrsThrdConstDataLn;
        InterfaceDescriptorParams.iNumberThreadsInGroup = (pKernelParam->iNumberThreadsInGroup > 0) ? pKernelParam->iNumberThreadsInGroup : 1;
        InterfaceDescriptorParams.blBarrierEnable       = (pKernelParam->iBarrierMode != CM_NO_BARRIER) ? true : false;
        InterfaceDescriptorParams.blGlobalBarrierEnable = (pKernelParam->iBarrierMode == CM_GLOBAL_BARRIER) ? true : false;
        InterfaceDescriptorParams.iSLMSize              = pKernelParam->iSLMSize;
    }
    if (pState->use_new_sampler_heap == true)
    {
        HalCm_SetupInterfaceDescriptor(pState, pRenderHal, pCurMediaState, pKernelAllocation, &InterfaceDescriptorParams);
    }
    else
    {
        // Setup Media ID entry - this call could be HW dependent
        pRenderHal->pfnSetupInterfaceDescriptor(
            pRenderHal,
            pCurMediaState,
            pKernelAllocation,
            &InterfaceDescriptorParams);
    }

finish:
    return iInterfaceDescriptor;
}


bool isRenderTarget(PCM_HAL_STATE pState, uint32_t iIndex)
{
    bool bReadSync = false;
    if ( ( pState->GpuContext == MOS_GPU_CONTEXT_RENDER3 ) || ( pState->GpuContext == MOS_GPU_CONTEXT_RENDER4 ) ) 
    {
        bReadSync = pState->pUmdSurf2DTable[iIndex].bReadSync[pState->GpuContext - MOS_GPU_CONTEXT_RENDER3];
    }
    if (bReadSync)
        return false;
    else
        return true;
}

int32_t HalCm_DSH_LoadKernelArray(
    PCM_HAL_STATE               pState,
    PCM_HAL_KERNEL_PARAM       *pKernelArray,
    int32_t                     iKernelCount,
    PRENDERHAL_KRN_ALLOCATION  *pKrnAllocation)
{
    PRENDERHAL_INTERFACE         pRenderHal;
    PCM_HAL_KERNEL_PARAM         pKernel;
    PMHW_STATE_HEAP_MEMORY_BLOCK pMemoryBlock;                             // Kernel memory block
    int32_t                      iTotalSize;                               // Total size
    uint32_t                     iBlockSize[CM_MAX_KERNELS_PER_TASK];      // Size of kernels to load
    int32_t                      iBlockCount;                              // Number of kernels to load
    MOS_STATUS                   eStatus = MOS_STATUS_SUCCESS;
    int32_t                      hr = CM_FAILURE;
    uint32_t                     dwCurrId, dwNextId;

    pRenderHal = pState->pRenderHal;
    dwNextId = pRenderHal->pfnGetNextFrameId(pRenderHal, MOS_GPU_CONTEXT_INVALID_HANDLE);
    dwCurrId = pRenderHal->pfnGetCurrentFrameId(pRenderHal, MOS_GPU_CONTEXT_INVALID_HANDLE);

    do
    {
        iBlockCount = 0;
        iTotalSize = 0;

        // Obtain list of kernels already loaded, discard kernels loaded in older heaps.
        // Calculate total size of kernels to be loaded, and get size of largest kernel.
        for (int i = 0; i < iKernelCount; i++)
        {
            // Find out if kernel is already allocated and loaded in ISH
            pKernel = pKernelArray[i];
            pKrnAllocation[i] = (PRENDERHAL_KRN_ALLOCATION)pRenderHal->pfnSearchDynamicKernel(pRenderHal, static_cast<int>((pKernel->uiKernelId >> 32)), -1);

            // Kernel is allocated - check if kernel is in current ISH
            if (pKrnAllocation[i])
            {
                // Check if kernel is loaded
                pMemoryBlock = pKrnAllocation[i]->pMemoryBlock;

                if (pMemoryBlock)
                {
                    // Kernel needs to be reloaded in current heap
                    if (pMemoryBlock->pStateHeap != pRenderHal->pMhwStateHeap->GetISHPointer()) //pInstructionStateHeaps
                    {
                        pRenderHal->pMhwStateHeap->FreeDynamicBlockDyn(MHW_ISH_TYPE, pMemoryBlock, dwCurrId);
                        pKrnAllocation[i]->pMemoryBlock = nullptr;
                    }
                    else
                    {
                        // Increment kernel usage count, used in kernel caching architecture
                        pState->dwDSHKernelCacheHit++;
                        pKrnAllocation[i]->dwCount++;

                        // Lock kernel to avoid removal while loading other kernels
                        pKrnAllocation[i]->dwFlags = RENDERHAL_KERNEL_ALLOCATION_LOCKED;
                    }
                }
                else if (pKrnAllocation[i]->dwFlags == RENDERHAL_KERNEL_ALLOCATION_REMOVED)
                {
                    // This is a kernel that was unloaded and now needs to be reloaded
                    // Track how many times this "cache miss" happens to determine if the
                    // ISH is under pressure and needs to be expanded
                    pState->dwDSHKernelCacheMiss++;
                }
            }
            else
            {
                // Assign kernel allocation for this kernel
                pKrnAllocation[i] = pRenderHal->pfnAllocateDynamicKernel(pRenderHal, static_cast<int>((pKernel->uiKernelId >> 32)), -1);
                CM_CHK_NULL(pKrnAllocation[i]);
            }

            // Kernel is not loaded -> add to list of kernels to be loaded
            if (pKrnAllocation[i]->pMemoryBlock == nullptr &&
                pKrnAllocation[i]->dwFlags != RENDERHAL_KERNEL_ALLOCATION_LOADING)
            {
                // Increment amount of data that needs to be loaded in ISH (kernel already registered but unloaded)
                iBlockSize[iBlockCount++] = pKernel->iKernelBinarySize + CM_KERNEL_BINARY_PADDING_SIZE;
                iTotalSize += pKernel->iKernelBinarySize + CM_KERNEL_BINARY_PADDING_SIZE;

                // Flag this kernel as loading - one single kernel instance is needed, not multiple!
                // If the same kernel is used multiple times, avoid multiple reservations/loads
                pKrnAllocation[i]->dwFlags = RENDERHAL_KERNEL_ALLOCATION_LOADING;
            }
        }

        // Use Hit/Miss ratio to ignore eventual cache misses
        // This code prevents ISH reallocation in case of eventual cache misses
        while (pState->dwDSHKernelCacheHit >= HAL_CM_KERNEL_CACHE_HIT_TO_MISS_RATIO)
        {
            if (pState->dwDSHKernelCacheMiss > 0) pState->dwDSHKernelCacheMiss--;
            pState->dwDSHKernelCacheHit -= HAL_CM_KERNEL_CACHE_HIT_TO_MISS_RATIO;
        }

        // Grow the kernel heap if too many kernels are being reloaded or there isn't enough room to load all kernels
        if (pState->dwDSHKernelCacheMiss > HAL_CM_KERNEL_CACHE_MISS_THRESHOLD ||
            pRenderHal->pfnRefreshDynamicKernels(pRenderHal, iTotalSize, iBlockSize, iBlockCount) != MOS_STATUS_SUCCESS)
        {
            pRenderHal->pfnExpandKernelStateHeap(pRenderHal, (uint32_t)iTotalSize);
            pState->dwDSHKernelCacheHit = 0;
            pState->dwDSHKernelCacheMiss = 0;
            continue;
        }

        // iBlockSize/iBlockCount define a list of blocks that must be loaded in current ISH for the
        // kernels not yet present. Pre-existing kernels are marked as bStatic to avoid being unloaded here
        if (iBlockCount > 0)
        {
            // Allocate array of kernels
            MHW_STATE_HEAP_DYNAMIC_ALLOC_PARAMS Params;
            Params.piSizes = (int32_t*)iBlockSize;
            Params.iCount = iBlockCount;
            Params.dwAlignment = RENDERHAL_KERNEL_BLOCK_ALIGN;
            Params.bHeapAffinity = true;                                     // heap affinity - load all kernels in the same heap
            Params.pHeapAffinity = pRenderHal->pMhwStateHeap->GetISHPointer();    // Select the active instruction heap
            Params.dwScratchSpace = 0;
            Params.bZeroAssignedMem = true;
            Params.bStatic = true;
            Params.bGrow = false;

            // Try to allocate array of blocks; if it fails, we may need to clear some space or grow the heap!
            pMemoryBlock = pRenderHal->pMhwStateHeap->AllocateDynamicBlockDyn(MHW_ISH_TYPE, &Params);
            if (!pMemoryBlock)
            {
                // Reset flags
                for (int i = 0; i < iKernelCount; i++)
                {
                    if (pKrnAllocation[i] && pKrnAllocation[i]->dwFlags == RENDERHAL_KERNEL_ALLOCATION_LOADING)
                    {
                        pKrnAllocation[i]->dwFlags = RENDERHAL_KERNEL_ALLOCATION_STALE;
                    }
                }

                if (pRenderHal->pfnRefreshDynamicKernels(pRenderHal, iTotalSize, iBlockSize, iBlockCount) != MOS_STATUS_SUCCESS)
                {
                    pRenderHal->pfnExpandKernelStateHeap(pRenderHal, (uint32_t)iTotalSize);
                }
                continue;
            }

            // All blocks are allocated in ISH
            // Setup kernel allocations, load kernel binaries
            for (int32_t i = 0; i < iKernelCount; i++)
            {
                // Load kernels in ISH
                if (!pKrnAllocation[i]->pMemoryBlock)
                {
                    PCM_HAL_KERNEL_PARAM      pKernelParam = pKernelArray[i];
                    PRENDERHAL_KRN_ALLOCATION pAllocation = pKrnAllocation[i];
                    if (pMemoryBlock)
                    {
                        pAllocation->iKID = -1;
                        pAllocation->iKUID = static_cast<int>((pKernelArray[i]->uiKernelId >> 32));
                        pAllocation->iKCID = -1;
                        pAllocation->dwSync = dwNextId;
                        pAllocation->dwOffset = pMemoryBlock->dwDataOffset;
                        pAllocation->iSize = pKernelArray[i]->iKernelBinarySize + CM_KERNEL_BINARY_PADDING_SIZE;
                        pAllocation->dwCount = 0;
                        pAllocation->dwFlags = RENDERHAL_KERNEL_ALLOCATION_USED;
                        pAllocation->Params = pState->KernelParams_RenderHal.Params;
                        pAllocation->pMhwKernelParam = &pState->KernelParams_Mhw;
                        pAllocation->pMemoryBlock = pMemoryBlock;

                        // Copy kernel data
                        // Copy MovInstruction First
                        if (pAllocation->pMemoryBlock &&
                            pAllocation->pMemoryBlock->dwDataSize >= pKernelParam->iKernelBinarySize)
                        {
                            MOS_SecureMemcpy(pAllocation->pMemoryBlock->pDataPtr,
                                pKernelParam->iMovInsDataSize,
                                pKernelParam->pMovInsData,
                                pKernelParam->iMovInsDataSize);

                            // Copy Cm Kernel Binary
                            MOS_SecureMemcpy(pAllocation->pMemoryBlock->pDataPtr + pKernelParam->iMovInsDataSize,
                                pKernelParam->iKernelBinarySize - pKernelParam->iMovInsDataSize,
                                pKernelParam->pKernelBinary,
                                pKernelParam->iKernelBinarySize - pKernelParam->iMovInsDataSize);

                            // Padding bytes dummy instructions after kernel binary to resolve page fault issue
                            MOS_ZeroMemory(pAllocation->pMemoryBlock->pDataPtr + pKernelParam->iKernelBinarySize, CM_KERNEL_BINARY_PADDING_SIZE);
                        }

                        // Get next memory block returned as part of the array
                        pMemoryBlock = pMemoryBlock->pNext;
                    }
                }
            }
        }

        // Kernel load was successfull, or nothing else to load -
        // Quit the kernel load loop
        hr = CM_SUCCESS;
        eStatus = MOS_STATUS_SUCCESS;
        break;

    } while (1);

finish:
    if (eStatus == MOS_STATUS_SUCCESS)
    {
        for (int32_t i = 0; i < iKernelCount; i++)
        {
            pRenderHal->pfnTouchDynamicKernel(pRenderHal, pKrnAllocation[i]);
        }
    }

    return hr;
}


MOS_STATUS HalCm_DSH_GetDynamicStateConfiguration(
    PCM_HAL_STATE                         pState,
    PRENDERHAL_DYNAMIC_MEDIA_STATE_PARAMS pParams,
    uint32_t                              iNumKernels,
    PCM_HAL_KERNEL_PARAM                 *pKernels,
    uint32_t                              *piCurbeOffsets)
{
    PCM_HAL_KERNEL_PARAM      pCmKernel;

    PRENDERHAL_INTERFACE pRenderHal = pState->pRenderHal;
    PRENDERHAL_KRN_ALLOCATION pKrnAllocation;

    MOS_ZeroMemory(pParams, sizeof(RENDERHAL_DYNAMIC_MEDIA_STATE_PARAMS));

    pParams->iMaxMediaIDs = iNumKernels;

    for (uint32_t i = 0; i < iNumKernels; i++)
    {
        pCmKernel = pKernels[i];

        // get max curbe size
        int32_t iCurbeSize = MOS_ALIGN_CEIL(pCmKernel->iKrnCurbeSize, pState->pRenderHal->dwCurbeBlockAlign);
        int32_t iCurbeOffset = piCurbeOffsets[i] + iCurbeSize;
        pParams->iMaxCurbeOffset = MOS_MAX(pParams->iMaxCurbeOffset, iCurbeOffset);
        pParams->iMaxCurbeSize += iCurbeSize;

        // get max spill size
        pParams->iMaxSpillSize = MOS_MAX(pParams->iMaxSpillSize, (int32_t)pCmKernel->iSpillSize);

        // check if kernel already used - increase Max Media ID to allow BB reuse logic
        pKrnAllocation = pRenderHal->pfnSearchDynamicKernel(pRenderHal, static_cast<int>((pCmKernel->uiKernelId >> 32)), -1);
        if (pKrnAllocation)
        {
            pParams->iMaxMediaIDs = MOS_MAX(pParams->iMaxMediaIDs, pKrnAllocation->iKID + 1);
        }
    }

    if (pState->use_new_sampler_heap == true)
    {
        // Update offset to the base of first kernel and update count
        // for 3D sampler, update indirect state information
        unsigned int heap_offset = 0;
        unsigned int sampler_3D_count = 0;
        MHW_SAMPLER_STATE_PARAM sampler_param_mhw = {};
        SamplerParam sampler_param = {};
        sampler_param_mhw.SamplerType = MHW_SAMPLER_TYPE_3D;
        pState->pCmHalInterface->GetSamplerParamInfoForSamplerType(&sampler_param_mhw, sampler_param);
        for (unsigned int i = 0; i < iNumKernels; i++)
        {
            pCmKernel = pKernels[i];
            std::list<SamplerParam> *sampler_heap = pCmKernel->sampler_heap;
            std::list<SamplerParam>::iterator iter;

            heap_offset = MOS_ALIGN_CEIL(heap_offset, MHW_SAMPLER_STATE_ALIGN);
            pState->pTaskParam->sampler_offsets_by_kernel[i] = heap_offset;
            pState->pTaskParam->sampler_counts_by_kernel[i] = sampler_heap->size();

            if (sampler_heap->size() > 0)
            {
                heap_offset = heap_offset + sampler_heap->back().heap_offset + sampler_heap->back().size;

                // 3D sampler needs indirect sampler heap, so calculates the required size 
                // and offset for indirect sampler heap.
                unsigned int max_3D_count = 0;
                for (iter = sampler_heap->begin(); iter != sampler_heap->end(); iter++)
                {
                    if (iter->element_type == sampler_param.element_type)
                    {
                        if (iter->user_defined_bti == true)
                        {
                            max_3D_count = iter->bti + 1;
                        }
                        else
                        {
                            max_3D_count += 1;
                        }
                    }
                }
                heap_offset = MOS_ALIGN_CEIL(heap_offset, MHW_SAMPLER_STATE_ALIGN);
                pState->pTaskParam->sampler_indirect_offsets_by_kernel[i] = heap_offset;
                heap_offset += max_3D_count * pState->pRenderHal->pHwSizes->dwSizeSamplerIndirectState;
                sampler_3D_count += max_3D_count;
            }
        }

        // Temporary solution for DSH sampler heap assginment:
        // Adjust sampler space for DSH, because the DSH use sampler count to 
        // allocate the space. However the mechanism is not correct. The sampler 
        // heap size is actually calculated by the maximum offset of the largest 
        // sampler type.
        // So the offset of largest element plus the size of all of the largest 
        // element samplers should be equal to the maximum size. However we cannot 
        // do this because of the DSH's mechanism. 
        // To resolve this, we first let DSH allocate enough 3D samplers
        // (because 3D samplers has indirect state), then just convert the rest of 
        // the heap to AVS. Here we only care about the size, not the correct 
        // number because we are going to calculate the offset by ourself. 
        // Since DSH allocation has some alignments inside, the actually size of the 
        // heap should be slightly larger, which should be OK.

        sampler_param_mhw.SamplerType = MHW_SAMPLER_TYPE_AVS;
        pState->pCmHalInterface->GetSamplerParamInfoForSamplerType(&sampler_param_mhw, sampler_param);
        pParams->iMaxSamplerIndex3D = (sampler_3D_count + iNumKernels - 1) / iNumKernels;
        pParams->iMaxSamplerIndexAVS = ((heap_offset - sampler_3D_count * (pState->pRenderHal->pHwSizes->dwSizeSamplerState + pState->pRenderHal->pHwSizes->dwSizeSamplerIndirectState)) + sampler_param.bti_multiplier * iNumKernels - 1) / (sampler_param.bti_multiplier * iNumKernels);
    }
    else
    {
        // Get total sampler count

        // Initialize pointers to samplers and reset sampler index table
        MOS_FillMemory(pState->pSamplerIndexTable, pState->CmDeviceParam.iMaxSamplerTableSize, CM_INVALID_INDEX);

        pParams->iMaxSamplerIndex3D = CM_MAX_3D_SAMPLER_SIZE;
        pParams->iMaxSamplerIndexAVS = CM_MAX_AVS_SAMPLER_SIZE;
        pParams->iMaxSamplerIndexConv = 0;
        pParams->iMaxSamplerIndexMisc = 0;
        pParams->iMax8x8Tables = CM_MAX_AVS_SAMPLER_SIZE;
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HalCm_DSH_UnregisterKernel(
    PCM_HAL_STATE               pState,
    uint64_t                    uiKernelId)
{
    PRENDERHAL_INTERFACE pRenderHal = pState->pRenderHal;
    PRENDERHAL_KRN_ALLOCATION pKrnAllocation = pRenderHal->pfnSearchDynamicKernel(pRenderHal, static_cast<int>((uiKernelId >> 32)), -1);
    if (pKrnAllocation)
    {
        pRenderHal->pfnUnregisterKernel(pRenderHal, pKrnAllocation);
    }
    return MOS_STATUS_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose: Setup Sampler State
//| Returns: Result of the operation
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_SetupSamplerState(
    PCM_HAL_STATE                   pState,
    PCM_HAL_KERNEL_PARAM            pKernelParam,
    PCM_HAL_KERNEL_ARG_PARAM        pArgParam,
    PCM_HAL_INDEX_PARAM             pIndexParam,
    int32_t                         iMediaID,
    uint32_t                        iThreadIndex,
    uint8_t                         *pBuffer)
{
    MOS_STATUS                  hr;
    PRENDERHAL_INTERFACE        pRenderHal;
    PMHW_SAMPLER_STATE_PARAM    pSamplerParam;
    uint8_t                     *pSrc;
    uint8_t                     *pDst;
    uint32_t                    iIndex;
    uint32_t                    iSamplerIndex = 0;
    void                        *pSampler = nullptr;

    hr = MOS_STATUS_SUCCESS;

    CM_CHK_NULL_RETURN_MOSSTATUS(pState);

    pRenderHal    = pState->pRenderHal;

    if (pIndexParam->dwSamplerIndexCount >= (uint32_t)pRenderHal->StateHeapSettings.iSamplers)
    {
        CM_ERROR_ASSERT(
            "Exceeded Max samplers '%d'", 
            pIndexParam->dwSamplerIndexCount);
        goto finish;
    }

    // Get the Index to sampler array from the kernel data
    //----------------------------------
    CM_ASSERT(pArgParam->iUnitSize == sizeof(iIndex));
    //----------------------------------

    pSrc    = pArgParam->pFirstValue + (iThreadIndex * pArgParam->iUnitSize);
    iIndex  = *((uint32_t*)pSrc);

    // check to see if the data present for the sampler in the array
    if (iIndex >= pState->CmDeviceParam.iMaxSamplerTableSize || 
        !pState->pSamplerTable[iIndex].bInUse)
    {
        CM_ERROR_ASSERT(
            "Invalid Sampler array index '%d'", iIndex);
        goto finish;
    }
    // Setup samplers
    pSamplerParam = &pState->pSamplerTable[iIndex];

    if (pState->use_new_sampler_heap == true)
    {
        std::list<SamplerParam>::iterator iter;
        for (iter = pKernelParam->sampler_heap->begin(); iter != pKernelParam->sampler_heap->end(); iter++)
        {
            if ((iter->sampler_table_index == iIndex)&&(iter->regular_bti == true))
            {
                break;
            }
        }
        if (iter != pKernelParam->sampler_heap->end())
        {
            iSamplerIndex = iter->bti;
        }
        else
        {
            // There must be incorrect internal logic
            CM_ERROR_ASSERT( "BTI calculation error in cm_hal\n");
            return MOS_STATUS_UNKNOWN;
        }
        HalCm_GetSamplerOffsetAndPtr(pState, pRenderHal, iMediaID, iter->heap_offset, iter->bti, pSamplerParam, nullptr, &pSampler);
    }
    else
    {
        // Check to see if sampler is already assigned
        iSamplerIndex = pState->pSamplerIndexTable[iIndex];
        if ((int)iSamplerIndex == CM_INVALID_INDEX)
        {

            switch (pState->pSamplerTable[iIndex].ElementType)
            {

                case MHW_Sampler2Elements:
                {
                    unsigned int index = 0;
                    index = pState->SamplerStatistics.Sampler_indexBase[MHW_Sampler2Elements];
                    while (pState->pSamplerIndexTable[index] != CM_INVALID_INDEX)
                    {
                        index++;
                    }
                    iSamplerIndex = index;
                    pState->SamplerStatistics.Sampler_indexBase[MHW_Sampler2Elements] = (index + 1);
                    break;
                }
                case MHW_Sampler4Elements:
                {
                    unsigned int index = 0;
                    index = pState->SamplerStatistics.Sampler_indexBase[MHW_Sampler4Elements];
                    while (pState->pSamplerIndexTable[index] != CM_INVALID_INDEX)
                    {
                        index++;
                    }
                    iSamplerIndex = index;
                    pState->SamplerStatistics.Sampler_indexBase[MHW_Sampler4Elements] = (index + 1);
                    break;
                }
                case MHW_Sampler8Elements:
                {
                    unsigned int index = 0;
                    index = pState->SamplerStatistics.Sampler_indexBase[MHW_Sampler8Elements];
                    while (pState->pSamplerIndexTable[index] != CM_INVALID_INDEX)
                    {
                        index++;
                    }
                    iSamplerIndex = index;
                    pState->SamplerStatistics.Sampler_indexBase[MHW_Sampler8Elements] = (index + 1);
                    break;
                }
                case MHW_Sampler64Elements:
                {
                    unsigned int index = 0;
                    index = pState->SamplerStatistics.Sampler_indexBase[MHW_Sampler64Elements];
                    while (pState->pSamplerIndexTable[index] != CM_INVALID_INDEX)
                    {
                        index += index + 2;
                    }
                    iSamplerIndex = index;
                    pState->SamplerStatistics.Sampler_indexBase[MHW_Sampler64Elements] = (index + 2);

                    break;
                }
                case MHW_Sampler128Elements:
                {
                    unsigned int index = 0;
                    index = pState->SamplerStatistics.Sampler_indexBase[MHW_Sampler128Elements];
                    while (pState->pSamplerIndexTable[index] != CM_INVALID_INDEX)
                    {
                        index++;
                    }
                    iSamplerIndex = index;
                    pState->SamplerStatistics.Sampler_indexBase[MHW_Sampler128Elements] = (index + 1);

                    break;
                }
                default:
                    CM_ASSERTMESSAGE("Invalid sampler type '%d'.", pState->pSamplerTable[iIndex].SamplerType);
                    break;
            }
        }

        CM_CHK_MOSSTATUS(pRenderHal->pfnGetSamplerOffsetAndPtr(pRenderHal, iMediaID, iSamplerIndex, pSamplerParam, nullptr, &pSampler));
    }
    CM_CHK_MOSSTATUS(pRenderHal->pMhwStateHeap->SetSamplerState(pSampler, pSamplerParam));

    pState->pSamplerIndexTable[iIndex] = (unsigned char)iSamplerIndex;

    // Update the Batch Buffer
    if (pBuffer)
    {
        pDst = pBuffer + pArgParam->iPayloadOffset;
        *((uint32_t*)pDst) = iSamplerIndex;
    }
    
finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose: Setup Sampler State
//| Returns: Result of the operation
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_SetupSamplerStateWithBTIndex(
    PCM_HAL_STATE                   pState,
    PCM_HAL_KERNEL_PARAM            pKernelParam,
    PCM_HAL_SAMPLER_BTI_ENTRY       pSamplerBTIEntry,
    uint32_t                        iSamplerCount,
    int32_t                         iMediaID )
{
    MOS_STATUS                      hr = MOS_STATUS_SUCCESS;
    PRENDERHAL_INTERFACE            pRenderHal;
    PMHW_SAMPLER_STATE_PARAM        pSamplerParam;
    uint32_t                        iIndex;
    uint32_t                        iSamplerIndex;
    void                            *pSampler = nullptr;

    pRenderHal = pState->pRenderHal;

    if (pState->use_new_sampler_heap != true)
    {
        if (iSamplerCount >= (uint32_t)pRenderHal->StateHeapSettings.iSamplers)
        {
            CM_ERROR_ASSERT(
                "Exceeded Max samplers '%d'",
                iSamplerCount);
            goto finish;
        }
    }

    iIndex = pSamplerBTIEntry[ iSamplerCount ].iSamplerIndex;

    // check to see if the data present for the sampler in the array
    if ( iIndex >= pState->CmDeviceParam.iMaxSamplerTableSize ||
         !pState->pSamplerTable[ iIndex ].bInUse )
    {
        CM_ERROR_ASSERT(
            "Invalid Sampler array index '%d'", iIndex );
        goto finish;
    }

    iSamplerIndex = pSamplerBTIEntry[ iSamplerCount ].iSamplerBTI;
    // Setup samplers
    pSamplerParam = &pState->pSamplerTable[ iIndex ];

    if (pState->use_new_sampler_heap == true)
    {
        std::list<SamplerParam>::iterator iter;
        for (iter = pKernelParam->sampler_heap->begin(); iter != pKernelParam->sampler_heap->end(); iter++)
        {
            if ((iter->sampler_table_index == iIndex) && (iter->bti == iSamplerIndex) && (iter->user_defined_bti == true))
            {
                break;
            }
        }
        if (iter == pKernelParam->sampler_heap->end())
        {
            // There must be incorrect internal logic
            CM_ERROR_ASSERT("BTI calculation error in cm_hal\n");
            return MOS_STATUS_UNKNOWN;
        }
        HalCm_GetSamplerOffsetAndPtr(pState, pRenderHal, iMediaID, iter->heap_offset, iter->bti, pSamplerParam, nullptr, &pSampler);
    }
    else
    {
        CM_CHK_MOSSTATUS(pRenderHal->pfnGetSamplerOffsetAndPtr(pRenderHal, iMediaID, iSamplerIndex, pSamplerParam, nullptr, &pSampler));
    }

    CM_CHK_MOSSTATUS( pRenderHal->pMhwStateHeap->SetSamplerState(pSampler, pSamplerParam ) );

finish:
    return hr;
}


//*-----------------------------------------------------------------------------
//| Purpose: Setup Buffer Surface State
//| Returns: Result of the operation
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_SetupBufferSurfaceState(
    PCM_HAL_STATE               pState,
    PCM_HAL_KERNEL_ARG_PARAM    pArgParam,
    PCM_HAL_INDEX_PARAM         pIndexParam,
    int32_t                     iBindingTable,
    int16_t                     globalSurface,
    uint32_t                    iThreadIndex,
    uint8_t                     *pBuffer)
{
    MOS_STATUS                  hr;
    RENDERHAL_SURFACE               Surface;
    PMOS_SURFACE                    pMosSurface;
    RENDERHAL_SURFACE_STATE_PARAMS  SurfaceParam;
    PRENDERHAL_INTERFACE            pRenderHal;
    PRENDERHAL_SURFACE_STATE_ENTRY  pSurfaceEntry;
    uint8_t                     *pSrc;
    uint8_t                     *pDst;
    uint32_t                    iIndex;
    uint32_t                    iBTIndex;
    uint16_t                    memObjCtl;
    uint32_t                    dwOffsetSrc;
    PRENDERHAL_STATE_HEAP       pStateHeap;
    CM_SURFACE_BTI_INFO         SurfBTIInfo;

    hr              = MOS_STATUS_UNKNOWN;
    pRenderHal      = pState->pRenderHal;
    //GT-PIN
    PCM_HAL_TASK_PARAM     pTaskParam = pState->pTaskParam;

    // Get the Index to Buffer array from the kernel data
    CM_ASSERT(pArgParam->iUnitSize == sizeof(iIndex));

    //Init SurfBTIInfo
    pState->pCmHalInterface->GetHwSurfaceBTIInfo(&SurfBTIInfo);

    pSrc      = pArgParam->pFirstValue + (iThreadIndex * pArgParam->iUnitSize);
    iIndex    = *((uint32_t*)pSrc) & CM_SURFACE_MASK;
    if (iIndex == CM_NULL_SURFACE)
    {
        if (pBuffer)
        {
            pDst = pBuffer + pArgParam->iPayloadOffset;
            *((uint32_t*)pDst) = CM_NULL_SURFACE_BINDING_INDEX;
        }

        hr = MOS_STATUS_SUCCESS;
        goto finish;
    }

    memObjCtl = pState->pBufferTable[iIndex].memObjCtl;
    if (!memObjCtl)
    {
        memObjCtl = CM_DEFAULT_CACHE_TYPE;
    }

    // check to see if index is valid
    if (iIndex >= pState->CmDeviceParam.iMaxBufferTableSize || 
        (pState->pBufferTable[iIndex].iSize == 0))
    {
        CM_ERROR_ASSERT(
            "Invalid Buffer Surface array index '%d'", iIndex);
        goto finish;
    }

    // Check to see if buffer is already assigned
    iBTIndex = pState->pBTBufferIndexTable[iIndex].BTI.RegularSurfIndex;
    if (iBTIndex == ( unsigned char )CM_INVALID_INDEX || pArgParam->bAliasCreated == true) 
    {
        if (globalSurface < 0)
        {
            iBTIndex = HalCm_GetFreeBindingIndex(pState, pIndexParam, 1);
        }
        else 
        {
            iBTIndex = globalSurface + SurfBTIInfo.dwReservedSurfaceStart; //CM_BINDING_START_INDEX_OF_GLOBAL_SURFACE(pState);
            if ((int32_t)iBTIndex >=  (SurfBTIInfo.dwReservedSurfaceStart + CM_MAX_GLOBAL_SURFACE_NUMBER) ) {
                CM_ERROR_ASSERT("Exceeded Max Global Surfaces '%d'", iBTIndex);
                goto finish;
            }
        }
        // Get Details of Buffer surface and fill the Surface 
        CM_CHK_MOSSTATUS(HalCm_GetSurfaceAndRegister(pState, &Surface, CM_ARGUMENT_SURFACEBUFFER, iIndex, 0));

        MOS_ZeroMemory(&SurfaceParam, sizeof(SurfaceParam));

        // override the buffer offset and size if alias is used
        pMosSurface = &(Surface.OsSurface);
        if (pState->pBufferTable[iIndex].surfaceStateEntry[pArgParam->iAliasIndex / pState->nSurfaceArraySize].iSurfaceStateSize)
        {
            pMosSurface->dwWidth = pState->pBufferTable[iIndex].surfaceStateEntry[pArgParam->iAliasIndex / pState->nSurfaceArraySize].iSurfaceStateSize;
            pMosSurface->dwOffset = pState->pBufferTable[iIndex].surfaceStateEntry[pArgParam->iAliasIndex / pState->nSurfaceArraySize].iSurfaceStateOffset;
            Surface.rcSrc.right = pMosSurface->dwWidth;
            Surface.rcDst.right = pMosSurface->dwWidth;
        }
        // override the mocs value if it is set
        if (pState->pBufferTable[iIndex].surfaceStateEntry[pArgParam->iAliasIndex / pState->nSurfaceArraySize].wSurfaceStateMOCS)
        {
            memObjCtl = pState->pBufferTable[iIndex].surfaceStateEntry[pArgParam->iAliasIndex / pState->nSurfaceArraySize].wSurfaceStateMOCS;
        }

        //Cache configurations
        pState->pCmHalInterface->HwSetSurfaceMemoryObjectControl(memObjCtl, &SurfaceParam);

        // Set the bRenderTarget by default
        SurfaceParam.bRenderTarget = true;
        
        // Setup Buffer surface
        CM_CHK_MOSSTATUS(pRenderHal->pfnSetupBufferSurfaceState(
                pRenderHal,
                &Surface, 
                &SurfaceParam,
                &pSurfaceEntry));

        // Bind the Surface State
        pSurfaceEntry->pSurface = &Surface.OsSurface;
        CM_ASSERT(((int32_t)iBTIndex) < pRenderHal->StateHeapSettings.iSurfacesPerBT + SurfBTIInfo.dwNormalSurfaceStart);
        CM_CHK_MOSSTATUS(pRenderHal->pfnBindSurfaceState(
               pRenderHal, 
               iBindingTable,
               iBTIndex,
               pSurfaceEntry));

        if ((pTaskParam->SurEntryInfoArrays.dwKrnNum != 0) &&
            (pTaskParam->SurEntryInfoArrays.pSurfEntryInfosArray != nullptr))
        {
            //GT-Pin
           uint32_t dummy = 0;
           CM_CHK_MOSSTATUS(HalCm_GetSurfaceDetails(
                   pState, 
                   pIndexParam,
                   iBTIndex,
                   Surface.OsSurface,
                   globalSurface, 
                   nullptr, 
                   dummy,
                   SurfaceParam,
                   CM_ARGUMENT_SURFACEBUFFER));
        }

        // Update index to table
        pState->pBTBufferIndexTable[ iIndex ].BTI.RegularSurfIndex = iBTIndex;
        pState->pBTBufferIndexTable[ iIndex ].nPlaneNumber = 1;

        pStateHeap = pRenderHal->pStateHeap;
        dwOffsetSrc = ( pStateHeap->iCurSshBufferIndex * pStateHeap->dwSshIntanceSize ) +   // Points to the Base of Current SSH Buffer Instance
                            ( pStateHeap->iBindingTableOffset ) +                       // Moves the pointer to Base of Array of Binding Tables
                            ( iBindingTable * pStateHeap->iBindingTableSize ) +         // Moves the pointer to a Particular Binding Table
                            ( iBTIndex * sizeof( uint32_t ) );                             // Move the pointer to correct entry

        pState->pBTBufferIndexTable[ iIndex ].BTITableEntry.RegularBTIEntryPos = pStateHeap->pSshBuffer + dwOffsetSrc;
    }
    else
    {
        pStateHeap = pRenderHal->pStateHeap;

        // Get Offset to Current Binding Table 
        uint32_t dwOffsetCurrentBTStart = ( pStateHeap->iCurSshBufferIndex * pStateHeap->dwSshIntanceSize ) +     // Points to the Base of Current SSH Buffer Instance
                                       ( pStateHeap->iBindingTableOffset ) +                             // Moves the pointer to Base of Array of Binding Tables
                                       ( iBindingTable * pStateHeap->iBindingTableSize );                // Moves the pointer to a Particular Binding Table

        uint32_t *pCurrentBTStart = ( uint32_t *)( pStateHeap->pSshBuffer + dwOffsetCurrentBTStart );

        int nEntryIndex = (int) ((uint32_t*)( pState->pBTBufferIndexTable[ iIndex ].BTITableEntry.RegularBTIEntryPos ) - pCurrentBTStart);

        if ( ( nEntryIndex < 0 ) || ( nEntryIndex >= pRenderHal->StateHeapSettings.iSurfacesPerBT ) )
        {
            uint32_t iSurfaceEntries = pState->pBTBufferIndexTable[ iIndex ].nPlaneNumber;
            if ( globalSurface < 0 )
            {
                iBTIndex = HalCm_GetFreeBindingIndex( pState, pIndexParam, iSurfaceEntries );
            }
            else
            {
                iBTIndex = globalSurface + SurfBTIInfo.dwReservedSurfaceStart;
                if ( ( int32_t )iBTIndex >= (SurfBTIInfo.dwReservedSurfaceStart + CM_MAX_GLOBAL_SURFACE_NUMBER ) )
                {
                    CM_ERROR_ASSERT( "Exceeded Max Global Surfaces '%d'", iBTIndex );
                    goto finish;
                }
            }

            // Bind the Surface State
            CM_ASSERT( ( ( int32_t )iBTIndex ) < pRenderHal->StateHeapSettings.iSurfacesPerBT + SurfBTIInfo.dwNormalSurfaceStart);

            // Get Offset to Current Binding Table 
            uint32_t dwOffsetDst = dwOffsetCurrentBTStart + ( iBTIndex * sizeof( uint32_t ) );                             // Move the pointer to correct entry

            uint32_t *pBindingTableEntry = ( uint32_t *)( pStateHeap->pSshBuffer + dwOffsetDst );
            MOS_SecureMemcpy( pBindingTableEntry, sizeof( uint32_t ) * iSurfaceEntries, pState->pBTBufferIndexTable[ iIndex ].BTITableEntry.RegularBTIEntryPos, sizeof( uint32_t ) * iSurfaceEntries );

            // Update index to table
            pState->pBTBufferIndexTable[ iIndex ].BTI.RegularSurfIndex = iBTIndex;
            pState->pBTBufferIndexTable[ iIndex ].BTITableEntry.RegularBTIEntryPos = pBindingTableEntry;
        }
    }

    // Update the Batch Buffer
    if (pBuffer)
    {
        pDst = pBuffer + pArgParam->iPayloadOffset;
        *((uint32_t*)pDst) = iBTIndex;
    }
    hr = MOS_STATUS_SUCCESS;

finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose: Setup 3D Surface State
//| Returns: Result of the operation
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_Setup3DSurfaceState(
    PCM_HAL_STATE               pState,
    PCM_HAL_KERNEL_ARG_PARAM    pArgParam,
    PCM_HAL_INDEX_PARAM         pIndexParam,
    int32_t                     iBindingTable,
    uint32_t                    iThreadIndex,
    uint8_t                     *pBuffer)
{
    MOS_STATUS                  hr;
    PRENDERHAL_INTERFACE            pRenderHal;
    RENDERHAL_SURFACE               Surface;
    RENDERHAL_SURFACE_STATE_PARAMS  SurfaceParam;
    PRENDERHAL_SURFACE_STATE_ENTRY  pSurfaceEntries[MHW_MAX_SURFACE_PLANES];
    RENDERHAL_GET_SURFACE_INFO      Info;
    uint8_t                     *pSrc;
    uint8_t                     *pDst;
    int32_t                     iSurfaceEntries;
    uint32_t                    iIndex;
    uint32_t                    iBTIndex;
    uint16_t                    memObjCtl;
    uint32_t                    i;
    uint32_t                    dwOffsetSrc;
    PRENDERHAL_STATE_HEAP       pStateHeap;
    CM_SURFACE_BTI_INFO         SurfBTIInfo;

    hr              = MOS_STATUS_UNKNOWN;
    pRenderHal  = pState->pRenderHal;
    //GT-PIN
    PCM_HAL_TASK_PARAM     pTaskParam = pState->pTaskParam;

    pState->pCmHalInterface->GetHwSurfaceBTIInfo(&SurfBTIInfo);

    // Get the Index to 3dsurface array from the kernel data
    CM_ASSERT(pArgParam->iUnitSize == sizeof(iIndex));
    pSrc      = pArgParam->pFirstValue + (iThreadIndex * pArgParam->iUnitSize);
    iIndex    = *((uint32_t*)pSrc) & CM_SURFACE_MASK;
    if (iIndex == CM_NULL_SURFACE)
    {
        if (pBuffer)
        {
            pDst = pBuffer + pArgParam->iPayloadOffset;
            *((uint32_t*)pDst) = CM_NULL_SURFACE_BINDING_INDEX;
        }

        hr = MOS_STATUS_SUCCESS;
        goto finish;
    }

    memObjCtl = pState->pSurf3DTable[iIndex].memObjCtl;
    if (!memObjCtl)
    {
        memObjCtl = CM_DEFAULT_CACHE_TYPE;
    }

    // check to see if the data present for the 3d surface in the array
    if ((iIndex >= pState->CmDeviceParam.iMax3DSurfaceTableSize)            || 
        Mos_ResourceIsNull(&pState->pSurf3DTable[iIndex].OsResource))
    {
        CM_ERROR_ASSERT(
            "Invalid 2D Surface array index '%d'", iIndex);
        goto finish;
    }

    // Check to see if surface is already assigned
    iBTIndex = pState->pBT3DIndexTable[iIndex].BTI.RegularSurfIndex;
    if ( iBTIndex == ( unsigned char )CM_INVALID_INDEX )
    {
        uint32_t dwTempPlaneIndex = 0;
        iSurfaceEntries = 0;

        // Get Details of 3D surface and fill the Surface 
        CM_CHK_MOSSTATUS(HalCm_GetSurfaceAndRegister(pState, &Surface, CM_ARGUMENT_SURFACE3D, iIndex, 0));

        // Setup 3D surface
        MOS_ZeroMemory(&SurfaceParam, sizeof(SurfaceParam));
        SurfaceParam.Type       = pRenderHal->SurfaceTypeDefault;
        SurfaceParam.Boundary   = RENDERHAL_SS_BOUNDARY_ORIGINAL;
        SurfaceParam.bRenderTarget = true;

        //Cache configurations
        pState->pCmHalInterface->HwSetSurfaceMemoryObjectControl(memObjCtl, &SurfaceParam);

        CM_CHK_MOSSTATUS(pRenderHal->pfnSetupSurfaceState(
                    pRenderHal, 
                    &Surface, 
                    &SurfaceParam, 
                    &iSurfaceEntries, 
                    pSurfaceEntries,
                    nullptr));

        MOS_ZeroMemory(&Info, sizeof(RENDERHAL_GET_SURFACE_INFO));

        CM_CHK_MOSSTATUS(RenderHal_GetSurfaceInfo(
            pState->pOsInterface,
            &Info,
            &Surface.OsSurface));

        iBTIndex = HalCm_GetFreeBindingIndex(pState, pIndexParam, iSurfaceEntries);
        for (i = 0; i < (uint32_t)iSurfaceEntries; i++)
        {
            // Bind the Surface State
            CM_CHK_MOSSTATUS(pRenderHal->pfnBindSurfaceState(
                        pRenderHal, 
                        iBindingTable,
                        iBTIndex + i,
                        pSurfaceEntries[i]));

            if ((pTaskParam->SurEntryInfoArrays.dwKrnNum != 0) &&
                (pTaskParam->SurEntryInfoArrays.pSurfEntryInfosArray != nullptr)) 
            {
                CM_CHK_MOSSTATUS(HalCm_GetSurfaceDetails(
                        pState,
                        pIndexParam,
                        iBTIndex + i,
                        Surface.OsSurface,
                        0, 
                        pSurfaceEntries[i],
                        dwTempPlaneIndex,
                        SurfaceParam,
                        CM_ARGUMENT_SURFACE3D));
            }
        }
        // Update index to table
        pState->pBT3DIndexTable[ iIndex ].BTI.RegularSurfIndex = iBTIndex;
        pState->pBT3DIndexTable[ iIndex ].nPlaneNumber = iSurfaceEntries;

        pStateHeap = pRenderHal->pStateHeap;
        dwOffsetSrc = ( pStateHeap->iCurSshBufferIndex * pStateHeap->dwSshIntanceSize ) +  // Points to the Base of Current SSH Buffer Instance
                            ( pStateHeap->iBindingTableOffset ) +                       // Moves the pointer to Base of Array of Binding Tables
                            ( iBindingTable * pStateHeap->iBindingTableSize ) +         // Moves the pointer to a Particular Binding Table
                            ( iBTIndex * sizeof( uint32_t ) );                             // Move the pointer to correct entry

        pState->pBT3DIndexTable[ iIndex ].BTITableEntry.RegularBTIEntryPos = pStateHeap->pSshBuffer + dwOffsetSrc;
    }
    else
    {
        pStateHeap = pRenderHal->pStateHeap;

        // Get Offset to Current Binding Table 
        uint32_t dwOffsetCurrentBTStart = ( pStateHeap->iCurSshBufferIndex * pStateHeap->dwSshIntanceSize ) +     // Points to the Base of Current SSH Buffer Instance
                                       ( pStateHeap->iBindingTableOffset ) +                             // Moves the pointer to Base of Array of Binding Tables
                                       ( iBindingTable * pStateHeap->iBindingTableSize );                // Moves the pointer to a Particular Binding Table

        uint32_t *pCurrentBTStart = ( uint32_t *)( pStateHeap->pSshBuffer + dwOffsetCurrentBTStart );

        int nEntryIndex = (int)((uint32_t*)( pState->pBT3DIndexTable[ iIndex ].BTITableEntry.RegularBTIEntryPos ) - pCurrentBTStart);

        if ( ( nEntryIndex < 0 ) || ( nEntryIndex >= pRenderHal->StateHeapSettings.iSurfacesPerBT ) )
        {
            iSurfaceEntries = pState->pBT3DIndexTable[ iIndex ].nPlaneNumber;
            iBTIndex = HalCm_GetFreeBindingIndex( pState, pIndexParam, iSurfaceEntries );

            // Bind the Surface State
            CM_ASSERT( ( ( int32_t )iBTIndex ) < pRenderHal->StateHeapSettings.iSurfacesPerBT + SurfBTIInfo.dwNormalSurfaceStart);

            // Get Offset to Current Binding Table 
            uint32_t dwOffsetDst = dwOffsetCurrentBTStart + ( iBTIndex * sizeof( uint32_t ) );                             // Move the pointer to correct entry

            uint32_t *pBindingTableEntry = ( uint32_t *)( pStateHeap->pSshBuffer + dwOffsetDst );
            MOS_SecureMemcpy( pBindingTableEntry, sizeof( uint32_t ) * iSurfaceEntries, pState->pBT3DIndexTable[ iIndex ].BTITableEntry.RegularBTIEntryPos, sizeof( uint32_t ) * iSurfaceEntries );

            // Update index to table
            pState->pBT3DIndexTable[ iIndex ].BTI.RegularSurfIndex = iBTIndex;
            pState->pBT3DIndexTable[ iIndex ].BTITableEntry.RegularBTIEntryPos = pBindingTableEntry;
        }
    }

    // Update the Batch Buffer
    if (pBuffer)
    {
        pDst = pBuffer + pArgParam->iPayloadOffset;
        *((uint32_t*)pDst) = iBTIndex;
    }

    hr = MOS_STATUS_SUCCESS;

finish:
    return hr;
}

/*----------------------------------------------------------------------------
| Purpose   : Set's surface state interlaced settings
| Returns   : dword value
\---------------------------------------------------------------------------*/
MOS_STATUS HalCm_HwSetSurfaceProperty(
    PCM_HAL_STATE                   pState,
    CM_FRAME_TYPE                   frameType,
    PRENDERHAL_SURFACE_STATE_PARAMS pParams)
{
    MOS_STATUS hr = MOS_STATUS_SUCCESS;

    switch (frameType)
    {
    case CM_FRAME:
        pParams->bVertStride = 0;
        pParams->bVertStrideOffs = 0;
        break;
    case CM_TOP_FIELD:
        pParams->bVertStride = 1;
        pParams->bVertStrideOffs = 0;
        break;
    case CM_BOTTOM_FIELD:
        pParams->bVertStride = 1;
        pParams->bVertStrideOffs = 1;
        break;
    default:
        hr = MOS_STATUS_UNKNOWN;
    }

    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose: Setup 2D Surface State
//| Returns: Result of the operation
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_Setup2DSurfaceStateBasic(
    PCM_HAL_STATE                      pState,
    PCM_HAL_KERNEL_ARG_PARAM           pArgParam,
    PCM_HAL_INDEX_PARAM                pIndexParam,
    int32_t                            iBindingTable,
    uint32_t                           iThreadIndex,
    bool                               pixelPitch,
    uint8_t                            *pBuffer,
    bool                               multipleBinding )
{
    MOS_STATUS                  hr;
    RENDERHAL_SURFACE               RenderHalSurface;
    PMOS_SURFACE                    pSurface;
    RENDERHAL_SURFACE_STATE_PARAMS  SurfaceParam;
    PRENDERHAL_INTERFACE            pRenderHal;
    PRENDERHAL_SURFACE_STATE_ENTRY  pSurfaceEntries[ MHW_MAX_SURFACE_PLANES ];
    uint8_t                     *pSrc;
    uint8_t                     *pDst;
    int32_t                     iSurfaceEntries = 0;
    uint32_t                    iIndex;
    uint32_t                    iBTIndex;
    uint16_t                    memObjCtl;
    uint32_t                    i;
    uint32_t                    dwTempPlaneIndex = 0;
    uint32_t                    dwOffsetSrc;
    PRENDERHAL_STATE_HEAP       pStateHeap;
    PCM_HAL_SURFACE2D_SURFACE_STATE_PARAM pSurfStateParam = nullptr;
    UNUSED(multipleBinding);

    hr = MOS_STATUS_UNKNOWN;
    pRenderHal = pState->pRenderHal;
    MOS_ZeroMemory(&RenderHalSurface, sizeof(RenderHalSurface));
    pSurface   = &RenderHalSurface.OsSurface;
    iSurfaceEntries = 0;

    //GT-PIN
    PCM_HAL_TASK_PARAM     pTaskParam = pState->pTaskParam;

    // Get the Index to 2dsurface array from the kernel data
    CM_ASSERT( pArgParam->iUnitSize == sizeof( iIndex ) );
    pSrc = pArgParam->pFirstValue + ( iThreadIndex * pArgParam->iUnitSize );
    iIndex = *( ( uint32_t *)pSrc ) & CM_SURFACE_MASK;
    if ( iIndex == CM_NULL_SURFACE )
    {
        if ( pBuffer )
        {
            pDst = pBuffer + pArgParam->iPayloadOffset;
            *( ( uint32_t *)pDst ) = CM_NULL_SURFACE_BINDING_INDEX;
        }

        hr = MOS_STATUS_SUCCESS;
        goto finish;
    }

    memObjCtl = pState->pUmdSurf2DTable[iIndex].memObjCtl;
    if ( !memObjCtl )
    {
        memObjCtl = CM_DEFAULT_CACHE_TYPE;
    }

    // check to see if the data present for the 2d surface in the array
    if ( iIndex >= pState->CmDeviceParam.iMax2DSurfaceTableSize ||
         Mos_ResourceIsNull( &pState->pUmdSurf2DTable[ iIndex ].OsResource ) )
    {
        CM_ERROR_ASSERT(
            "Invalid 2D Surface array index '%d'", iIndex );
        goto finish;
    }

    // Check to see if surface is already assigned
    unsigned char nBTIRegularSurf, nBTISamplerSurf;
    nBTIRegularSurf = pState->pBT2DIndexTable[ iIndex ].BTI.RegularSurfIndex;
    nBTISamplerSurf = pState->pBT2DIndexTable[ iIndex ].BTI.SamplerSurfIndex;

    if (((!pixelPitch && (nBTIRegularSurf != (unsigned char)CM_INVALID_INDEX)) || (pixelPitch && (nBTISamplerSurf != (unsigned char)CM_INVALID_INDEX))) && pArgParam->bAliasCreated == false )
    {
        if ( pixelPitch )
        {
            iBTIndex = nBTISamplerSurf;
        }
        else
        {
            iBTIndex = nBTIRegularSurf;
        }

        pStateHeap = pRenderHal->pStateHeap;

        // Get Offset to Current Binding Table 
        uint32_t dwOffsetCurrentBTStart = ( pStateHeap->iCurSshBufferIndex * pStateHeap->dwSshIntanceSize ) +     // Points to the Base of Current SSH Buffer Instance
                                       ( pStateHeap->iBindingTableOffset ) +                             // Moves the pointer to Base of Array of Binding Tables
                                       ( iBindingTable * pStateHeap->iBindingTableSize );                // Moves the pointer to a Particular Binding Table

        uint32_t *pCurrentBTStart = ( uint32_t *)( pStateHeap->pSshBuffer + dwOffsetCurrentBTStart );

        int nEntryIndex = 0;
        
        if ( pixelPitch )
        {
            nEntryIndex = (int)((uint32_t*)( pState->pBT2DIndexTable[ iIndex ].BTITableEntry.SamplerBTIEntryPos ) - pCurrentBTStart);
        }
        else
        {
            nEntryIndex = (int)((uint32_t*)( pState->pBT2DIndexTable[ iIndex ].BTITableEntry.RegularBTIEntryPos ) - pCurrentBTStart);
        }

        if ( ( nEntryIndex < 0 ) || ( nEntryIndex >= pRenderHal->StateHeapSettings.iSurfacesPerBT ) )
        {
            iSurfaceEntries = pState->pBT2DIndexTable[ iIndex ].nPlaneNumber;

            iBTIndex = HalCm_GetFreeBindingIndex( pState, pIndexParam, iSurfaceEntries );

            // Get Offset to Current Binding Table 
            uint32_t dwOffsetDst = dwOffsetCurrentBTStart + ( iBTIndex * sizeof( uint32_t ) );            // Move the pointer to correct entry

            uint32_t *pBindingTableEntry = ( uint32_t *)( pStateHeap->pSshBuffer + dwOffsetDst );

            if ( pixelPitch )
            {
                MOS_SecureMemcpy( pBindingTableEntry, sizeof( uint32_t ) * iSurfaceEntries, pState->pBT2DIndexTable[ iIndex ].BTITableEntry.SamplerBTIEntryPos, sizeof( uint32_t ) * iSurfaceEntries );
            }
            else
            {
                MOS_SecureMemcpy( pBindingTableEntry, sizeof( uint32_t ) * iSurfaceEntries, pState->pBT2DIndexTable[ iIndex ].BTITableEntry.RegularBTIEntryPos, sizeof( uint32_t ) * iSurfaceEntries );
            }

            // update index to table
            if ( pixelPitch )
            {
                pState->pBT2DIndexTable[ iIndex ].BTI.SamplerSurfIndex = iBTIndex;
                pState->pBT2DIndexTable[ iIndex ].BTITableEntry.SamplerBTIEntryPos = pBindingTableEntry;
            }
            else
            {
                pState->pBT2DIndexTable[ iIndex ].BTI.RegularSurfIndex = iBTIndex;
                pState->pBT2DIndexTable[ iIndex ].BTITableEntry.RegularBTIEntryPos = pBindingTableEntry;
            }
        }

        // Update the Batch Buffer
        if ( pBuffer )
        {
            pDst = pBuffer + pArgParam->iPayloadOffset;
            *( ( uint32_t *)pDst ) = iBTIndex;
        }

        hr = MOS_STATUS_SUCCESS;
        goto finish;
    }

    CM_CHK_MOSSTATUS( HalCm_GetSurfaceAndRegister( pState, &RenderHalSurface, CM_ARGUMENT_SURFACE2D, iIndex, pixelPitch ) );

    // Setup 2D surface
    MOS_ZeroMemory(&SurfaceParam, sizeof(SurfaceParam));
    SurfaceParam.Type       = pRenderHal->SurfaceTypeDefault;
    SurfaceParam.Boundary   = RENDERHAL_SS_BOUNDARY_ORIGINAL;
    SurfaceParam.bVertStride = 0; 
    SurfaceParam.bVertStrideOffs = 0; 
    if (!pixelPitch) {
        SurfaceParam.bWidthInDword_UV = true;
        SurfaceParam.bWidthInDword_Y = true;
    }

    SurfaceParam.bRenderTarget = isRenderTarget(pState, iIndex);
    pSurfStateParam = &(pState->pUmdSurf2DTable[iIndex].surfaceStateParam[pArgParam->iAliasIndex / pState->nSurfaceArraySize]);
    if (pSurfStateParam->width)
    {
        pSurface->dwWidth = pSurfStateParam->width;
    }
    if (pSurfStateParam->height)
    {
        pSurface->dwHeight = pSurfStateParam->height;
    }
    if (pSurfStateParam->depth)
    {
        pSurface->dwDepth = pSurfStateParam->depth;
    }
    if (pSurfStateParam->pitch)
    {
        pSurface->dwPitch= pSurfStateParam->pitch;
    }
    if (pSurfStateParam->format)
    {
        pSurface->Format = (MOS_FORMAT)pSurfStateParam->format;
    }
    if (pSurfStateParam->surface_x_offset)
    {
        pSurface->YPlaneOffset.iXOffset = pSurfStateParam->surface_x_offset;
        if (pSurface->Format == Format_NV12)
        {
            pSurface->UPlaneOffset.iXOffset += pSurfStateParam->surface_x_offset;
        }
    }
    if (pSurfStateParam->surface_y_offset)
    {
        pSurface->YPlaneOffset.iYOffset = pSurfStateParam->surface_y_offset;
        if (pSurface->Format == Format_NV12)
        {
            pSurface->UPlaneOffset.iYOffset += pSurfStateParam->surface_y_offset/2;
        }
    }
    if (pSurfStateParam->memory_object_control)
    {
        memObjCtl = pSurfStateParam->memory_object_control;
    }
        
    if(pixelPitch)
        RenderHalSurface.Rotation = pState->pUmdSurf2DTable[iIndex].rotationFlag;

    //Cache configurations
    pState->pCmHalInterface->HwSetSurfaceMemoryObjectControl(memObjCtl, &SurfaceParam);

    // interlace setting
    HalCm_HwSetSurfaceProperty(pState,
        pState->pUmdSurf2DTable[iIndex].frameType,
        &SurfaceParam);

    CM_CHK_MOSSTATUS(pRenderHal->pfnSetupSurfaceState(
                  pRenderHal, 
                  &RenderHalSurface, 
                  &SurfaceParam, 
                  &iSurfaceEntries, 
                  pSurfaceEntries,
                  nullptr));

    //add this to avoid klocwork warning
    iSurfaceEntries = MOS_MIN( iSurfaceEntries, MHW_MAX_SURFACE_PLANES );

    iBTIndex = HalCm_GetFreeBindingIndex(pState, pIndexParam, iSurfaceEntries);
    for (i = 0; i < (uint32_t)iSurfaceEntries; i++)
    {
        // Bind the Surface State
        CM_CHK_MOSSTATUS(pRenderHal->pfnBindSurfaceState(
                        pRenderHal, 
                        iBindingTable,
                        iBTIndex + i,
                        pSurfaceEntries[i]));
        if ((pTaskParam->SurEntryInfoArrays.dwKrnNum !=0) &&
            (pTaskParam->SurEntryInfoArrays.pSurfEntryInfosArray != nullptr))
        { 
            //GT-Pin
            CM_CHK_MOSSTATUS(HalCm_GetSurfaceDetails(
                    pState,
                    pIndexParam,
                    iBTIndex + i,
                    *pSurface, 
                    0,
                    pSurfaceEntries[i],
                    dwTempPlaneIndex,
                    SurfaceParam,
                    CM_ARGUMENT_SURFACE2D));
        }
    }

    // only update the reuse table for non-aliased surface
    if ( pArgParam->bAliasCreated == false )
    {
        pState->pBT2DIndexTable[ iIndex ].nPlaneNumber = iSurfaceEntries;
        // Get Offset to Current Binding Table 
        pStateHeap = pRenderHal->pStateHeap;
        dwOffsetSrc = ( pStateHeap->iCurSshBufferIndex * pStateHeap->dwSshIntanceSize ) +     // Points to the Base of Current SSH Buffer Instance
            ( pStateHeap->iBindingTableOffset ) +                       // Moves the pointer to Base of Array of Binding Tables
            ( iBindingTable * pStateHeap->iBindingTableSize ) +         // Moves the pointer to a Particular Binding Table
            ( iBTIndex * sizeof( uint32_t ) );                             // Move the pointer to correct entry

        if ( pixelPitch )
        {
            pState->pBT2DIndexTable[ iIndex ].BTI.SamplerSurfIndex = iBTIndex;
            pState->pBT2DIndexTable[ iIndex ].BTITableEntry.SamplerBTIEntryPos = pStateHeap->pSshBuffer + dwOffsetSrc;
        }
        else
        {
            pState->pBT2DIndexTable[ iIndex ].BTI.RegularSurfIndex = iBTIndex;
            pState->pBT2DIndexTable[ iIndex ].BTITableEntry.RegularBTIEntryPos = pStateHeap->pSshBuffer + dwOffsetSrc;
        }
    }

    // Update the Batch Buffer
    if (pBuffer)
    {
        pDst = pBuffer + pArgParam->iPayloadOffset;
        *((uint32_t*)pDst) = iBTIndex;
    }

    // reset surface height and width
    pSurface->dwWidth = pState->pUmdSurf2DTable[iIndex].iWidth;
    pSurface->dwHeight = pState->pUmdSurf2DTable[iIndex].iHeight;

    hr = MOS_STATUS_SUCCESS;

finish:
    return hr;
}

MOS_STATUS HalCm_Setup2DSurfaceState(
    PCM_HAL_STATE              pState,
    PCM_HAL_KERNEL_ARG_PARAM   pArgParam,
    PCM_HAL_INDEX_PARAM        pIndexParam,
    int32_t                    iBindingTable,
    uint32_t                   iThreadIndex,
    uint8_t                    *pBuffer)
{
    MOS_STATUS                 hr;

    //Binding surface based at the unit of dword
    CM_CHK_MOSSTATUS(HalCm_Setup2DSurfaceStateBasic(
                    pState, pArgParam, pIndexParam, iBindingTable, iThreadIndex, false, pBuffer, false));
    hr = MOS_STATUS_SUCCESS;

finish:
    return hr;
}

MOS_STATUS HalCm_Setup2DSurfaceSamplerState(
    PCM_HAL_STATE              pState,
    PCM_HAL_KERNEL_ARG_PARAM   pArgParam,
    PCM_HAL_INDEX_PARAM        pIndexParam,
    int32_t                    iBindingTable,
    uint32_t                   iThreadIndex,
    uint8_t                    *pBuffer)
{
    MOS_STATUS                 hr;

    //Binding surface based at the unit of dword
    CM_CHK_MOSSTATUS(HalCm_Setup2DSurfaceStateBasic(
        pState, pArgParam, pIndexParam, iBindingTable, iThreadIndex, true, pBuffer, false));
    hr = MOS_STATUS_SUCCESS;

finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose: Setup 2D Surface State
//| Returns: Result of the operation
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_Setup2DSurfaceUPStateBasic(
    PCM_HAL_STATE               pState,
    PCM_HAL_KERNEL_ARG_PARAM    pArgParam,
    PCM_HAL_INDEX_PARAM         pIndexParam,
    int32_t                     iBindingTable,
    uint32_t                    iThreadIndex,
    uint8_t                     *pBuffer,
    bool                        pixelPitch)
{
    MOS_STATUS                  hr;
    RENDERHAL_SURFACE               Surface;
    RENDERHAL_SURFACE_STATE_PARAMS  SurfaceParam;
    PRENDERHAL_INTERFACE            pRenderHal;
    PRENDERHAL_SURFACE_STATE_ENTRY  pSurfaceEntries[MHW_MAX_SURFACE_PLANES];
    uint8_t                     *pSrc;
    uint8_t                     *pDst;
    int32_t                     iSurfaceEntries;
    uint32_t                    iIndex;
    uint32_t                    iBTIndex;
    uint16_t                    memObjCtl;
    uint32_t                    i;
    uint32_t                    dwOffsetSrc;
    PRENDERHAL_STATE_HEAP       pStateHeap;

    hr              = MOS_STATUS_UNKNOWN;
    pRenderHal    = pState->pRenderHal;
    //GT-PIN
    PCM_HAL_TASK_PARAM     pTaskParam = pState->pTaskParam;

    // Get the Index to sampler array from the kernel data
    CM_ASSERT(pArgParam->iUnitSize == sizeof(iIndex));
    pSrc      = pArgParam->pFirstValue + (iThreadIndex * pArgParam->iUnitSize);
    iIndex    = *((uint32_t*)pSrc) & CM_SURFACE_MASK;
    if (iIndex == CM_NULL_SURFACE)
    {
        if (pBuffer)
        {
            pDst = pBuffer + pArgParam->iPayloadOffset;
            *((uint32_t*)pDst) = CM_NULL_SURFACE_BINDING_INDEX;
        }

        hr = MOS_STATUS_SUCCESS;
        goto finish;
    }

    memObjCtl = pState->pSurf2DUPTable[iIndex].memObjCtl;
    if (!memObjCtl)
    {
        memObjCtl = CM_DEFAULT_CACHE_TYPE;
    }

    // check to see if the data present for the sampler in the array
    if (iIndex >= pState->CmDeviceParam.iMax2DSurfaceUPTableSize || 
        (pState->pSurf2DUPTable[iIndex].iWidth == 0))
    {
        CM_ERROR_ASSERT(
            "Invalid 2D SurfaceUP array index '%d'", iIndex);
        goto finish;
    }

    // Check to see if surface is already assigned
    if ( pixelPitch )
    {
        iBTIndex = pState->pBT2DUPIndexTable[ iIndex ].BTI.SamplerSurfIndex;
    }
    else
    {
        iBTIndex = pState->pBT2DUPIndexTable[ iIndex ].BTI.RegularSurfIndex;
    }

    if ( iBTIndex == ( unsigned char )CM_INVALID_INDEX )
    {
        uint32_t dwTempPlaneIndex = 0;

        // Get Details of 2DUP surface and fill the Surface 
        CM_CHK_MOSSTATUS(HalCm_GetSurfaceAndRegister(pState, &Surface, CM_ARGUMENT_SURFACE2D_UP, iIndex, pixelPitch));

        // Setup 2D surface
        MOS_ZeroMemory(&SurfaceParam, sizeof(SurfaceParam));
        SurfaceParam.Type       = pRenderHal->SurfaceTypeDefault;
        SurfaceParam.Boundary   = RENDERHAL_SS_BOUNDARY_ORIGINAL;

        if (!pixelPitch) {
            SurfaceParam.bWidthInDword_UV = true;
            SurfaceParam.bWidthInDword_Y = true;
        }
        
        SurfaceParam.bRenderTarget = true;

        //Cache configurations
        pState->pCmHalInterface->HwSetSurfaceMemoryObjectControl(memObjCtl, &SurfaceParam);

        // interlace setting
        HalCm_HwSetSurfaceProperty(pState,
            pState->pUmdSurf2DTable[iIndex].frameType,
            &SurfaceParam);

        CM_CHK_MOSSTATUS(pRenderHal->pfnSetupSurfaceState(
                    pRenderHal, 
                    &Surface, 
                    &SurfaceParam, 
                    &iSurfaceEntries, 
                    pSurfaceEntries,
                    nullptr));

        //GT-PIN
        iBTIndex = HalCm_GetFreeBindingIndex(pState, pIndexParam, iSurfaceEntries);
        for (i = 0; i < (uint32_t)iSurfaceEntries; i++)
        {
            // Bind the Surface State
            CM_CHK_MOSSTATUS(pRenderHal->pfnBindSurfaceState(
                        pRenderHal, 
                        iBindingTable,
                        iBTIndex + i,
                        pSurfaceEntries[i]));
            //GT-Pin
            if ((pTaskParam->SurEntryInfoArrays.dwKrnNum != 0) &&
                (pTaskParam->SurEntryInfoArrays.pSurfEntryInfosArray != nullptr))
            { 
                 CM_CHK_MOSSTATUS(HalCm_GetSurfaceDetails(
                         pState,
                         pIndexParam,
                         iBTIndex + i,
                         Surface.OsSurface,
                         0,
                         pSurfaceEntries[i],
                         dwTempPlaneIndex,
                         SurfaceParam,
                         CM_ARGUMENT_SURFACE2D_UP));
            }
        }
        pState->pBT2DUPIndexTable[ iIndex ].nPlaneNumber = iSurfaceEntries;

        pStateHeap = pRenderHal->pStateHeap;
        dwOffsetSrc = ( pStateHeap->iCurSshBufferIndex * pStateHeap->dwSshIntanceSize ) +     // Points to the Base of Current SSH Buffer Instance
                            ( pStateHeap->iBindingTableOffset ) +                       // Moves the pointer to Base of Array of Binding Tables
                            ( iBindingTable * pStateHeap->iBindingTableSize ) +         // Moves the pointer to a Particular Binding Table
                            ( iBTIndex * sizeof( uint32_t ) );                             // Move the pointer to correct entry

        if ( pixelPitch )
        {
            pState->pBT2DUPIndexTable[ iIndex ].BTI.SamplerSurfIndex = iBTIndex;
            pState->pBT2DUPIndexTable[ iIndex ].BTITableEntry.SamplerBTIEntryPos = pStateHeap->pSshBuffer + dwOffsetSrc;
        }
        else
        {
            pState->pBT2DUPIndexTable[ iIndex ].BTI.RegularSurfIndex = iBTIndex;
            pState->pBT2DUPIndexTable[ iIndex ].BTITableEntry.RegularBTIEntryPos = pStateHeap->pSshBuffer + dwOffsetSrc;
        }
    }
    else
    {
        pStateHeap = pRenderHal->pStateHeap;

        // Get Offset to Current Binding Table 
        uint32_t dwOffsetCurrentBTStart = ( pStateHeap->iCurSshBufferIndex * pStateHeap->dwSshIntanceSize ) +     // Points to the Base of Current SSH Buffer Instance
                                       ( pStateHeap->iBindingTableOffset ) +                             // Moves the pointer to Base of Array of Binding Tables
                                       ( iBindingTable * pStateHeap->iBindingTableSize );                // Moves the pointer to a Particular Binding Table
        
        uint32_t *pCurrentBTStart = ( uint32_t *)( pStateHeap->pSshBuffer + dwOffsetCurrentBTStart );

        int nEntryIndex = 0;
        
        if ( pixelPitch )
        {
            nEntryIndex = (int) ((uint32_t*)( pState->pBT2DUPIndexTable[ iIndex ].BTITableEntry.SamplerBTIEntryPos ) - pCurrentBTStart);
        }
        else
        {
            nEntryIndex = (int) ((uint32_t*)( pState->pBT2DUPIndexTable[ iIndex ].BTITableEntry.RegularBTIEntryPos ) - pCurrentBTStart);
        }

        if ( ( nEntryIndex < 0 ) || ( nEntryIndex >= pRenderHal->StateHeapSettings.iSurfacesPerBT ) )
        {
            uint32_t iSurfaceEntries = pState->pBT2DUPIndexTable[ iIndex ].nPlaneNumber;

            iBTIndex = HalCm_GetFreeBindingIndex( pState, pIndexParam, iSurfaceEntries );

            // Get Offset to Current Binding Table 
            uint32_t dwOffsetDst = dwOffsetCurrentBTStart + ( iBTIndex * sizeof( uint32_t ) );                             // Move the pointer to correct entry

            uint32_t *pBindingTableEntry = ( uint32_t *)( pStateHeap->pSshBuffer + dwOffsetDst );
            if ( pixelPitch )
            {
                MOS_SecureMemcpy( pBindingTableEntry, sizeof( uint32_t ) * iSurfaceEntries, pState->pBT2DUPIndexTable[ iIndex ].BTITableEntry.SamplerBTIEntryPos, sizeof( uint32_t ) * iSurfaceEntries );
            }
            else
            {
                MOS_SecureMemcpy( pBindingTableEntry, sizeof( uint32_t ) * iSurfaceEntries, pState->pBT2DUPIndexTable[ iIndex ].BTITableEntry.RegularBTIEntryPos, sizeof( uint32_t ) * iSurfaceEntries );
            }

            // update index to table
            if ( pixelPitch )
            {
                pState->pBT2DUPIndexTable[ iIndex ].BTI.SamplerSurfIndex = iBTIndex;
                pState->pBT2DUPIndexTable[ iIndex ].BTITableEntry.SamplerBTIEntryPos = pBindingTableEntry;
            }
            else
            {
                pState->pBT2DUPIndexTable[ iIndex ].BTI.RegularSurfIndex = iBTIndex;
                pState->pBT2DUPIndexTable[ iIndex ].BTITableEntry.RegularBTIEntryPos = pBindingTableEntry;
            }
        }
    }

    // Update the Batch Buffer
    if (pBuffer)
    {
        pDst = pBuffer + pArgParam->iPayloadOffset;
        *((uint32_t*)pDst) = iBTIndex;
    }

    hr = MOS_STATUS_SUCCESS;

finish:
    return hr;
}

MOS_STATUS HalCm_Setup2DSurfaceUPState(
    PCM_HAL_STATE               pState,
    PCM_HAL_KERNEL_ARG_PARAM    pArgParam,
    PCM_HAL_INDEX_PARAM         pIndexParam,
    int32_t                     iBindingTable,
    uint32_t                    iThreadIndex,
    uint8_t                     *pBuffer)
{
    MOS_STATUS                 hr;

    //Binding surface based at the unit of dword
    CM_CHK_MOSSTATUS(HalCm_Setup2DSurfaceUPStateBasic(
                    pState, pArgParam, pIndexParam, iBindingTable, iThreadIndex, pBuffer, false));
    hr = MOS_STATUS_SUCCESS;

finish:
    return hr;
}

MOS_STATUS HalCm_Setup2DSurfaceUPSamplerState(
    PCM_HAL_STATE               pState,
    PCM_HAL_KERNEL_ARG_PARAM    pArgParam,
    PCM_HAL_INDEX_PARAM         pIndexParam,
    int32_t                     iBindingTable,
    uint32_t                    iThreadIndex,
    uint8_t                     *pBuffer)
{
    MOS_STATUS                 hr;

    //Binding surface based at the unit of pixel
    CM_CHK_MOSSTATUS(HalCm_Setup2DSurfaceUPStateBasic(
                    pState, pArgParam, pIndexParam, iBindingTable, iThreadIndex, pBuffer, true));
    hr = MOS_STATUS_SUCCESS;

finish:
    return hr;
}

MOS_STATUS HalCm_SetupSpecificVmeSurfaceState(
    PCM_HAL_STATE                     pState,
    PCM_HAL_INDEX_PARAM               pIndexParam,
    int32_t                           iBindingTable,
    uint32_t                          SurfIndex,
    uint32_t                          iBTIndex,
    uint16_t                          MemObjCtl,
    uint32_t                          surfaceStateWidth,
    uint32_t                          surfaceStateHeight)
{
    MOS_STATUS                      hr;
    RENDERHAL_SURFACE               Surface;
    int32_t                         iSurfaceEntries = 0;
    RENDERHAL_SURFACE_STATE_PARAMS  SurfaceParam;
    PRENDERHAL_INTERFACE            pRenderHal;
    PRENDERHAL_SURFACE_STATE_ENTRY  pSurfaceEntries[MHW_MAX_SURFACE_PLANES];
    uint32_t                        dwTempPlaneIndex = 0;
    PMOS_SURFACE                    pMosSurface = nullptr;

    hr               = MOS_STATUS_UNKNOWN;
    pRenderHal     = pState->pRenderHal;
    iSurfaceEntries  = 0;

    PCM_HAL_TASK_PARAM pTaskParam = pState->pTaskParam;

    // Get Details of VME surface and fill the Surface 
    CM_CHK_MOSSTATUS(HalCm_GetSurfaceAndRegister(pState, &Surface, CM_ARGUMENT_VME_STATE, SurfIndex, 0));

    // Setup 2D surface
    MOS_ZeroMemory(&SurfaceParam, sizeof(SurfaceParam));
    SurfaceParam.Type              = pRenderHal->SurfaceTypeAdvanced;
    SurfaceParam.bRenderTarget     = true;
    SurfaceParam.bWidthInDword_Y   = false;
    SurfaceParam.bWidthInDword_UV  = false;
    SurfaceParam.Boundary          = RENDERHAL_SS_BOUNDARY_ORIGINAL;
    SurfaceParam.bVmeUse           = true;

    // Overwrite the width and height if specified
    if (surfaceStateWidth && surfaceStateHeight)
    {
        pMosSurface = &Surface.OsSurface;
        if (surfaceStateWidth > pMosSurface->dwWidth || surfaceStateHeight > pMosSurface->dwHeight)
        {
            CM_ASSERTMESSAGE("Error: VME surface state's resolution is larger than the original surface.");
            hr = MOS_STATUS_INVALID_PARAMETER;
            goto finish;
        }
        pMosSurface->dwWidth = surfaceStateWidth;
        pMosSurface->dwHeight = surfaceStateHeight;
    }

    //Cache configurations
    pState->pCmHalInterface->HwSetSurfaceMemoryObjectControl(MemObjCtl, &SurfaceParam);
    CM_CHK_MOSSTATUS(pRenderHal->pfnSetupSurfaceState(
                        pRenderHal, 
                        &Surface, 
                        &SurfaceParam, 
                        &iSurfaceEntries, 
                        pSurfaceEntries,
                        nullptr));

    CM_ASSERT(iSurfaceEntries == 1);

    {
        // Bind the Surface State
        CM_CHK_MOSSTATUS(pRenderHal->pfnBindSurfaceState(
                            pRenderHal, 
                            iBindingTable,
                            iBTIndex,
                            pSurfaceEntries[0]));

        if ((pTaskParam->SurEntryInfoArrays.dwKrnNum != 0) &&
            (pTaskParam->SurEntryInfoArrays.pSurfEntryInfosArray != nullptr))
        {
            CM_CHK_MOSSTATUS(HalCm_GetSurfaceDetails(
                    pState,
                    pIndexParam,
                    iBTIndex,
                    Surface.OsSurface,
                    0,
                    pSurfaceEntries[0],
                    dwTempPlaneIndex,
                    SurfaceParam,
                    CM_ARGUMENT_SURFACE2D));
        }
    }
    pState->pBT2DIndexTable[ SurfIndex ].BTI.VmeSurfIndex = iBTIndex;

    hr = MOS_STATUS_SUCCESS;

finish:
    return hr;

}

//*-----------------------------------------------------------------------------
//| Purpose: Setup VME Surface State
//| Returns: Result of the operation
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_SetupVmeSurfaceState(
    PCM_HAL_STATE               pState,
    PCM_HAL_KERNEL_ARG_PARAM    pArgParam,
    PCM_HAL_INDEX_PARAM         pIndexParam,
    int32_t                     iBindingTable,
    uint32_t                    iThreadIndex,
    uint8_t                     *pBuffer)
{
    MOS_STATUS                  hr;
    PRENDERHAL_INTERFACE        pRenderHal;
    PCM_HAL_VME_ARG_VALUE       pVmeSrc;
    uint8_t                     *pDst;
    uint32_t                    index[CM_MAX_VME_BINDING_INDEX_1];
    uint16_t                    memObjCtl[CM_MAX_VME_BINDING_INDEX_1];
    uint32_t                    FwSurfCount = 0;
    uint32_t                    BwSurfCount = 0;
    bool                        alreadyBind = true;
    uint32_t                    surfPairNum;
    uint32_t                    bIndex;
    uint32_t                    curBTIndex;
    uint32_t                    iBTIndex;
    uint32_t                    surfaceStateWidth = 0;
    uint32_t                    surfaceStateHeight = 0;
    uint32_t                    *f_ptr = nullptr;
    uint32_t                    *b_ptr = nullptr;
    uint32_t                    *refSurfaces = nullptr;

    hr              = MOS_STATUS_UNKNOWN;
    pRenderHal    = pState->pRenderHal;
    iBTIndex        = 0;

    MOS_ZeroMemory(memObjCtl, CM_MAX_VME_BINDING_INDEX_1*sizeof(uint16_t));
    MOS_ZeroMemory(index, CM_MAX_VME_BINDING_INDEX_1*sizeof(uint32_t));

    CM_ASSERT(pArgParam->iUnitSize <= sizeof(uint32_t)*(CM_MAX_VME_BINDING_INDEX_1 + 2));
    CM_ASSERT(iThreadIndex == 0); // VME surface is not allowed in thread arg
   
    pVmeSrc = (PCM_HAL_VME_ARG_VALUE)pArgParam->pFirstValue;
    FwSurfCount = pVmeSrc->fwRefNum;
    BwSurfCount = pVmeSrc->bwRefNum;
    refSurfaces = findRefInVmeArg(pVmeSrc);

    index[0] = pVmeSrc->curSurface & CM_SURFACE_MASK;
    // check to see if index[0] is valid
    if (index[0] == CM_NULL_SURFACE)
    {
        if (pBuffer)
        {
            pDst = pBuffer + pArgParam->iPayloadOffset;
            *((uint32_t*)pDst) = CM_NULL_SURFACE_BINDING_INDEX;
        }

        hr = MOS_STATUS_SUCCESS;
        goto finish;
    }

    if (index[0] >= pState->CmDeviceParam.iMax2DSurfaceTableSize || 
        Mos_ResourceIsNull(&pState->pUmdSurf2DTable[index[0]].OsResource))
    {
        CM_ERROR_ASSERT(
            "Invalid 2D Surface array index '%d'", index[0]);
        goto finish;
    }
    
    memObjCtl[0] = pState->pUmdSurf2DTable[index[0]].memObjCtl;
    if (!memObjCtl[0])
    {
        memObjCtl[0] = CM_DEFAULT_CACHE_TYPE;
    }
    for (bIndex = 0; bIndex < (pVmeSrc->fwRefNum + pVmeSrc->bwRefNum); bIndex++)
    {
        index[bIndex + 1] = refSurfaces[bIndex] & CM_SURFACE_MASK;
        memObjCtl[bIndex + 1] = pState->pUmdSurf2DTable[index[bIndex + 1]].memObjCtl;
        if (!memObjCtl[bIndex + 1])
        {
            memObjCtl[bIndex + 1] = CM_DEFAULT_CACHE_TYPE;
        }
    }

    surfaceStateWidth = pVmeSrc->surfStateParam.iSurfaceStateWidth;
    surfaceStateHeight = pVmeSrc->surfStateParam.iSurfaceStateHeight;

    f_ptr = index + 1;
    b_ptr = index + 1 + FwSurfCount;

    //Max surface pair number
    surfPairNum = FwSurfCount > BwSurfCount ? FwSurfCount : BwSurfCount;

    iBTIndex = curBTIndex = HalCm_GetFreeBindingIndex(pState, pIndexParam, surfPairNum*2 + 1);

    HalCm_SetupSpecificVmeSurfaceState(pState, pIndexParam, iBindingTable, index[0], curBTIndex, memObjCtl[0], surfaceStateWidth, surfaceStateHeight);
    curBTIndex++;

    //Setup surface states interleavely for backward and forward surfaces pairs.
    for (bIndex = 0; bIndex < surfPairNum; bIndex++)
    {
        if (bIndex < FwSurfCount)
        {
            HalCm_SetupSpecificVmeSurfaceState(pState, pIndexParam, iBindingTable, f_ptr[bIndex], curBTIndex, memObjCtl[bIndex + 1], surfaceStateWidth, surfaceStateHeight);
        }
        curBTIndex++;

        if (bIndex < BwSurfCount)
        {
            HalCm_SetupSpecificVmeSurfaceState(pState, pIndexParam, iBindingTable, b_ptr[bIndex], curBTIndex, memObjCtl[bIndex+ 1 + FwSurfCount], surfaceStateWidth, surfaceStateHeight);
        }
        curBTIndex++;
    }

    // Update the Batch Buffer
    if (pBuffer)
    {
        pDst = pBuffer + pArgParam->iPayloadOffset;
        *((uint32_t*)pDst) = iBTIndex;
    }

    hr = MOS_STATUS_SUCCESS;

finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose: Setup VME Surface State
//| Returns: Result of the operation
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_SetupSampler8x8SurfaceState(
    PCM_HAL_STATE               pState,
    PCM_HAL_KERNEL_ARG_PARAM    pArgParam,
    PCM_HAL_INDEX_PARAM         pIndexParam,
    int32_t                     iBindingTable,
    uint32_t                    iThreadIndex,
    uint8_t                     *pBuffer)
{
    MOS_STATUS                  hr;
    RENDERHAL_SURFACE               Surface;
    RENDERHAL_SURFACE_STATE_PARAMS  SurfaceParam;
    PRENDERHAL_INTERFACE            pRenderHal;
    PRENDERHAL_SURFACE_STATE_ENTRY  pSurfaceEntries[MHW_MAX_SURFACE_PLANES];
    uint8_t                     *pSrc;
    uint8_t                     *pDst;
    int32_t                     iSurfaceEntries;
    uint32_t                    index;
    uint16_t                    memObjCtl;
    int32_t                     i;
    uint32_t                    iBTIndex;
    uint32_t                    dwTempPlaneIndex = 0;
    uint32_t                    dwOffsetSrc;
    PRENDERHAL_STATE_HEAP       pStateHeap;

    hr               = MOS_STATUS_UNKNOWN;
    pRenderHal     = pState->pRenderHal;

    PCM_HAL_TASK_PARAM          pTaskParam    = pState->pTaskParam;

    iSurfaceEntries = 0;

    CM_ASSERT(pArgParam->iUnitSize == sizeof(uint32_t));

    pSrc      = pArgParam->pFirstValue + (iThreadIndex * pArgParam->iUnitSize);
    index     = *((uint32_t*)pSrc) & CM_SURFACE_MASK;
    if (index == CM_NULL_SURFACE)
    {
        if (pBuffer)
        {
            pDst = pBuffer + pArgParam->iPayloadOffset;
            *((uint32_t*)pDst) = CM_NULL_SURFACE_BINDING_INDEX;
        }

        hr = MOS_STATUS_SUCCESS;
        goto finish;
    }

    memObjCtl = pState->pUmdSurf2DTable[index].memObjCtl;
    if (!memObjCtl)
    {
        memObjCtl = CM_DEFAULT_CACHE_TYPE;
    }

    // check to see if index is valid
    if (index >= pState->CmDeviceParam.iMax2DSurfaceTableSize || 
       Mos_ResourceIsNull(&pState->pUmdSurf2DTable[index].OsResource))
    {
        CM_ERROR_ASSERT(
            "Invalid 2D Surface array index '%d'", index);
        goto finish;
    }

    pRenderHal->bEnableP010SinglePass = pState->pCmHalInterface->IsP010SinglePassSupported();

    iBTIndex = pState->pBT2DIndexTable[ index ].BTI.Sampler8x8SurfIndex;
    if ( iBTIndex == ( unsigned char )CM_INVALID_INDEX )
    {
        // Get Details of Sampler8x8 surface and fill the Surface
        CM_CHK_MOSSTATUS( HalCm_GetSurfaceAndRegister( pState, &Surface, pArgParam->Kind, index, 0 ) );

        // Setup surface
        MOS_ZeroMemory( &SurfaceParam, sizeof( SurfaceParam ) );
        SurfaceParam.Type = pRenderHal->SurfaceTypeAdvanced;
        SurfaceParam.bRenderTarget = true;
        SurfaceParam.bWidthInDword_Y = false;
        SurfaceParam.bWidthInDword_UV = false;
        SurfaceParam.Boundary = RENDERHAL_SS_BOUNDARY_ORIGINAL;
        SurfaceParam.bVASurface = ( pArgParam->Kind == CM_ARGUMENT_SURFACE_SAMPLER8X8_VA ) ? 1 : 0;
        SurfaceParam.AddressControl = pArgParam->nCustomValue;

        //Set memory object control
        pState->pCmHalInterface->HwSetSurfaceMemoryObjectControl(memObjCtl, &SurfaceParam);
        
        Surface.Rotation = pState->pUmdSurf2DTable[index].rotationFlag;
        Surface.ChromaSiting = pState->pUmdSurf2DTable[index].chromaSiting;
        iSurfaceEntries = 0;
        
        // interlace setting
        HalCm_HwSetSurfaceProperty(pState,
            pState->pUmdSurf2DTable[index].frameType,
            &SurfaceParam);

        CM_CHK_MOSSTATUS( pRenderHal->pfnSetupSurfaceState(
            pRenderHal,
            &Surface,
            &SurfaceParam,
            &iSurfaceEntries,
            pSurfaceEntries,
            nullptr ) );

        CM_ASSERT( iSurfaceEntries == 1 );

        iBTIndex = HalCm_GetFreeBindingIndex( pState, pIndexParam, iSurfaceEntries );

        for ( i = 0; i < iSurfaceEntries; i++ )
        {
            // Bind the Surface State
            CM_CHK_MOSSTATUS( pRenderHal->pfnBindSurfaceState(
                pRenderHal,
                iBindingTable,
                iBTIndex + i,
                pSurfaceEntries[ i ] ) );

            if ( ( pTaskParam->SurEntryInfoArrays.dwKrnNum != 0 ) &&
                 ( pTaskParam->SurEntryInfoArrays.pSurfEntryInfosArray != nullptr ) )
            {
                CM_CHK_MOSSTATUS( HalCm_GetSurfaceDetails(
                    pState,
                    pIndexParam,
                    iBTIndex + i,
                    Surface.OsSurface,
                    0,
                    pSurfaceEntries[ i ],
                    dwTempPlaneIndex,
                    SurfaceParam,
                    CM_ARGUMENT_SURFACE2D ) );
            }
        }


        pStateHeap = pRenderHal->pStateHeap;
        dwOffsetSrc = ( pStateHeap->iCurSshBufferIndex * pStateHeap->dwSshIntanceSize ) +     // Points to the Base of Current SSH Buffer Instance
                      ( pStateHeap->iBindingTableOffset ) +                       // Moves the pointer to Base of Array of Binding Tables
                      ( iBindingTable * pStateHeap->iBindingTableSize ) +         // Moves the pointer to a Particular Binding Table
                      ( iBTIndex * sizeof( uint32_t ) );                             // Move the pointer to correct entry

        pState->pBT2DIndexTable[ index ].nPlaneNumber = iSurfaceEntries;
        pState->pBT2DIndexTable[ index ].BTITableEntry.Sampler8x8BTIEntryPos = pStateHeap->pSshBuffer + dwOffsetSrc;
        pState->pBT2DIndexTable[ index ].BTI.Sampler8x8SurfIndex = iBTIndex;
    }
    else
    {
        pStateHeap = pRenderHal->pStateHeap;

        // Get Offset to Current Binding Table 
        uint32_t dwOffsetCurrentBTStart = ( pStateHeap->iCurSshBufferIndex * pStateHeap->dwSshIntanceSize ) +     // Points to the Base of Current SSH Buffer Instance
                                       ( pStateHeap->iBindingTableOffset ) +                             // Moves the pointer to Base of Array of Binding Tables
                                       ( iBindingTable * pStateHeap->iBindingTableSize );                // Moves the pointer to a Particular Binding Table

        uint32_t *pCurrentBTStart = ( uint32_t *)( pStateHeap->pSshBuffer + dwOffsetCurrentBTStart );

        int nEntryIndex = 0;

        nEntryIndex = ( int )( ( uint32_t *)( pState->pBT2DIndexTable[ index ].BTITableEntry.Sampler8x8BTIEntryPos ) - pCurrentBTStart );

        if ( ( nEntryIndex < 0 ) || ( nEntryIndex >= pRenderHal->StateHeapSettings.iSurfacesPerBT ) )
        {
            uint32_t iSurfaceEntries = pState->pBT2DIndexTable[ index ].nPlaneNumber;

            iBTIndex = HalCm_GetFreeBindingIndex( pState, pIndexParam, iSurfaceEntries );

            // Get Offset to Current Binding Table 
            uint32_t dwOffsetDst = dwOffsetCurrentBTStart + ( iBTIndex * sizeof( uint32_t ) );                             // Move the pointer to correct entry

            uint32_t *pBindingTableEntry = ( uint32_t *)( pStateHeap->pSshBuffer + dwOffsetDst );
            MOS_SecureMemcpy( pBindingTableEntry, sizeof( uint32_t ) * iSurfaceEntries, pState->pBT2DIndexTable[ index ].BTITableEntry.Sampler8x8BTIEntryPos, sizeof( uint32_t ) * iSurfaceEntries );

            // update index to table
            pState->pBT2DIndexTable[ index ].BTI.Sampler8x8SurfIndex = iBTIndex;
            pState->pBT2DIndexTable[ index ].BTITableEntry.Sampler8x8BTIEntryPos = pBindingTableEntry;
        }
    }
    // Update the Batch Buffer
    if ( pBuffer )
    {
        pDst = pBuffer + pArgParam->iPayloadOffset;
        *( ( uint32_t *)pDst ) = pState->pBT2DIndexTable[ index ].BTI.Sampler8x8SurfIndex;
    }

    hr = MOS_STATUS_SUCCESS;

finish:
    pRenderHal->bEnableP010SinglePass = false;
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose: Setup State Buffer Surface State
//| Returns: Result of the operation
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_SetupStateBufferSurfaceState(
    PCM_HAL_STATE               pState,
    PCM_HAL_KERNEL_ARG_PARAM    pArgParam,
    PCM_HAL_INDEX_PARAM         pIndexParam,
    int32_t                     iBindingTable,
    uint32_t                    iThreadIndex,
    uint8_t                     *pBuffer )
{
    MOS_STATUS hr = MOS_STATUS_SUCCESS;
    PRENDERHAL_INTERFACE            pRenderHal;
    RENDERHAL_SURFACE_STATE_PARAMS  SurfaceParam;
    RENDERHAL_SURFACE               renderhal_surface;
    PRENDERHAL_SURFACE_STATE_ENTRY  pSurfaceEntry;
    uint32_t                        iBTIndex;
    CM_SURFACE_BTI_INFO             SurfBTIInfo;
    uint16_t                        memObjCtl;

    pState->pCmHalInterface->GetHwSurfaceBTIInfo( &SurfBTIInfo );
    uint32_t surf_index = reinterpret_cast< uint32_t *>( pArgParam->pFirstValue )[ 0 ];

    surf_index = surf_index & CM_SURFACE_MASK;
    memObjCtl = pState->pBufferTable[ surf_index ].memObjCtl;

    iBTIndex = HalCm_GetFreeBindingIndex( pState, pIndexParam, 1 );

    pRenderHal = pState->pRenderHal;
    MOS_ZeroMemory( &renderhal_surface, sizeof( renderhal_surface ) );

    // Get Details of Sampler8x8 surface and fill the Surface
    CM_CHK_MOSSTATUS( HalCm_GetSurfaceAndRegister( pState, &renderhal_surface, pArgParam->Kind, surf_index, 0 ) );

    MOS_ZeroMemory( &SurfaceParam, sizeof( SurfaceParam ) );

    // Set the bRenderTarget by default
    SurfaceParam.bRenderTarget = true;

    //Cache configurations default
    pState->pCmHalInterface->HwSetSurfaceMemoryObjectControl( memObjCtl, &SurfaceParam );

    // Setup Buffer surface
    CM_CHK_MOSSTATUS( pRenderHal->pfnSetupBufferSurfaceState(
        pRenderHal,
        &renderhal_surface,
        &SurfaceParam,
        &pSurfaceEntry ) );

    // Bind the Surface State
    pSurfaceEntry->pSurface = &renderhal_surface.OsSurface;
    CM_ASSERT( ( ( int32_t )iBTIndex ) < pRenderHal->StateHeapSettings.iSurfacesPerBT + SurfBTIInfo.dwNormalSurfaceStart );
    CM_CHK_MOSSTATUS( pRenderHal->pfnBindSurfaceState(
        pRenderHal,
        iBindingTable,
        iBTIndex,
        pSurfaceEntry ) );

    if ( pBuffer )
    {
        *( ( uint32_t *)( pBuffer + pArgParam->iPayloadOffset ) ) = iBTIndex;
    }

finish:
    return hr;
}

//------------------------------------------------------------------------------
//| Purpose: Get usr defined threadcount / threadgroup
//| Returns:    Result of the operation
//------------------------------------------------------------------------------
MOS_STATUS HalCm_GetMaxThreadCountPerThreadGroup(
    PCM_HAL_STATE                   pState,                     // [in] Pointer to CM State
    uint32_t                        *pThreadsPerThreadGroup)     // [out] Pointer to pThreadsPerThreadGroup
{
    MOS_STATUS                  hr = MOS_STATUS_SUCCESS;

    CM_PLATFORM_INFO      platformInfo;
    MOS_ZeroMemory(&platformInfo, sizeof(CM_PLATFORM_INFO));
    CM_CHK_MOSSTATUS( pState->pfnGetPlatformInfo( pState, &platformInfo, false) );

    if (platformInfo.numMaxEUsPerPool)
    {
        *pThreadsPerThreadGroup = (platformInfo.numHWThreadsPerEU) * (platformInfo.numMaxEUsPerPool);
    }
    else
    {
        *pThreadsPerThreadGroup = (platformInfo.numHWThreadsPerEU) * (platformInfo.numEUsPerSubSlice);
    }

finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:  Decodes hints to get number and size of kernel groups
//| Returns:  Result of the operation
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_GetNumKernelsPerGroup(
    uint8_t     hintsBits,
    uint32_t    numKernels,
    uint32_t    *pNumKernelsPerGroup,
    uint32_t    *pNumKernelGroups,
    uint32_t    *pRemapKrnToGrp,
    uint32_t    *pRemapGrpToKrn
    )
{
    MOS_STATUS  hr   = MOS_STATUS_SUCCESS;
    uint32_t currGrp = 0;
    uint32_t i       = 0;

    // first group at least has one kernel
    pNumKernelsPerGroup[currGrp]++;
    pRemapGrpToKrn[currGrp] = 0;

    for( i = 0; i < numKernels - 1; ++i )
    {
        if( (hintsBits & CM_HINTS_LEASTBIT_MASK) == CM_HINTS_LEASTBIT_MASK )
        {
            currGrp++;
            *pNumKernelGroups = *pNumKernelGroups + 1;

            pRemapGrpToKrn[currGrp] = i + 1;
        }
        pNumKernelsPerGroup[currGrp]++;
        hintsBits >>= 1;
        pRemapKrnToGrp[i+1] = currGrp;
    }

    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:  Gets information about max parallelism graphs
//|           numThreadsOnSides based on formula to sum 1 to n: (n(n+1))/2
//| Returns:  Result of the operation
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_GetParallelGraphInfo(
    uint32_t                       maximum,
    uint32_t                       numThreads,
    uint32_t                       width,
    uint32_t                       height,
    PCM_HAL_PARALLELISM_GRAPH_INFO graphInfo,
    CM_DEPENDENCY_PATTERN          pattern,
    bool                           bNoDependencyCase)
{
    MOS_STATUS hr             = MOS_STATUS_SUCCESS;
    uint32_t numThreadsOnSides = 0;
    uint32_t numMaxRepeat      = 0;
    uint32_t numSteps          = 0;

    switch( pattern )
    {
        case CM_NONE_DEPENDENCY:
            if (bNoDependencyCase)
            {
                maximum = 1;
                numMaxRepeat = width * height;
                numSteps = width * height;
            }
            // do nothing will depend on other kernels
            break;

        case CM_VERTICAL_WAVE:
            numMaxRepeat = width;
            numSteps = width;
            break;

        case CM_HORIZONTAL_WAVE:
            numMaxRepeat = height;
            numSteps = height;
            break;

        case CM_WAVEFRONT:
            numThreadsOnSides = ( maximum - 1 ) * maximum;
            numMaxRepeat = (numThreads - numThreadsOnSides ) / maximum;
            numSteps = ( maximum - 1) * 2 + numMaxRepeat;
            break;

        case CM_WAVEFRONT26:
            numThreadsOnSides = ( maximum - 1 ) * maximum * 2;
            numMaxRepeat = (numThreads - numThreadsOnSides ) / maximum;
            numSteps = ( (maximum - 1) * 2 ) * 2 + numMaxRepeat;
            break;

        case CM_WAVEFRONT26Z:
            // do nothing already set outside of this function
            break;

        default:
            CM_ERROR_ASSERT("Unsupported dependency pattern for EnqueueWithHints");
            goto finish;
    }

    graphInfo->maxParallelism = maximum;
    graphInfo->numMaxRepeat = numMaxRepeat;
    graphInfo->numSteps = numSteps;

finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:  Sets dispatch pattern based on max parallelism for media objects
//| Returns:  Result of the operation
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_SetDispatchPattern(
    CM_HAL_PARALLELISM_GRAPH_INFO  graphInfo,
    CM_DEPENDENCY_PATTERN          pattern,
    uint32_t                       *pDispatchFreq
    )
{
    MOS_STATUS hr = MOS_STATUS_SUCCESS;
    uint32_t i  = 0;
    uint32_t j  = 0;
    uint32_t k  = 0;

    switch( pattern )
    {
    case CM_NONE_DEPENDENCY:
        break;
    case CM_HORIZONTAL_WAVE:
    case CM_VERTICAL_WAVE:
        for( i = 0; i < graphInfo.numSteps; ++i )
        {
            pDispatchFreq[i] = graphInfo.maxParallelism;
        }
        break;
    case CM_WAVEFRONT:
        for( i = 1; i < graphInfo.maxParallelism; ++i )
        {
            pDispatchFreq[i-1] = i;
        }
        for( j = 0; j < graphInfo.numMaxRepeat; ++i, ++j )
        {
            pDispatchFreq[i-1] = graphInfo.maxParallelism;
        }
        for( j = graphInfo.maxParallelism - 1; i <= graphInfo.numSteps; ++i, --j )
        {
            pDispatchFreq[i-1] = j;
        }
        break;
    case CM_WAVEFRONT26:
        for( i = 1, j = 0; i < graphInfo.maxParallelism; ++i, j +=2 )
        {
            pDispatchFreq[j] = i;
            pDispatchFreq[j+1] = i;
        }
        for( k = 0; k < graphInfo.numMaxRepeat; ++k, ++j)
        {
            pDispatchFreq[j] = graphInfo.maxParallelism;
        }
        for( i = graphInfo.maxParallelism - 1; j < graphInfo.numSteps; j +=2, --i )
        {
            pDispatchFreq[j] = i;
            pDispatchFreq[j+1] = i;
        }
        break;
    case CM_WAVEFRONT26Z:
        break;
    default:
        CM_ERROR_ASSERT("Unsupported dependency pattern for EnqueueWithHints");
        goto finish;
    }

finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:  Sets dispatch frequency for kernel group based on number of steps
//| Returns:  Result of the operation
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_SetKernelGrpFreqDispatch(
    PCM_HAL_PARALLELISM_GRAPH_INFO  graphInfo,
    PCM_HAL_KERNEL_GROUP_INFO       groupInfo,
    uint32_t                        numKernelGroups,
    uint32_t                        *pMinSteps)
{
    MOS_STATUS hr = MOS_STATUS_SUCCESS;
    uint32_t i  = 0;
    uint32_t j  = 0;
    uint32_t tmpSteps = 0;
    uint32_t kerIndex = 0;

    for( i = 0; i < numKernelGroups; ++i)
    {
        for( j = 0; j < groupInfo[i].numKernelsInGroup; ++j )
        {
            tmpSteps += graphInfo[kerIndex].numSteps;
            kerIndex++;
        }

        if ( tmpSteps ) 
        {
            *pMinSteps = MOS_MIN(*pMinSteps, tmpSteps);
            groupInfo[i].numStepsInGrp = tmpSteps;
        }

        tmpSteps = 0;
    }

    for( i = 0; i < numKernelGroups; ++i )
    {
        groupInfo[i].freqDispatch = (uint32_t)ceil( (groupInfo[i].numStepsInGrp / (double)*pMinSteps) );
    }

    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:  Sets dispatch pattern for kernel with no dependency based on 
//|           the minimum number of steps calculated from kernels with dependency 
//| Returns:  Result of the operation
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_SetNoDependKernelDispatchPattern(
    uint32_t                        numThreads,
    uint32_t                        minSteps,
    uint32_t                        *pDispatchFreq)
{
    MOS_STATUS hr = MOS_STATUS_SUCCESS;
    uint32_t i = 0;
    uint32_t numEachStep = 0;
    uint32_t total = 0;

    numEachStep = numThreads / minSteps;
    for( i = 0; i < minSteps; ++i )
    {
        pDispatchFreq[i] = numEachStep;
        total += numEachStep;
    }

    while( total != numThreads )
    {
        // dispatch more at beginning
        i = 0;
        pDispatchFreq[i]++;
        total++;
        i++;
    }

    return hr;
}

MOS_STATUS HalCm_FinishStatesForKernel(
    PCM_HAL_STATE                   pState,                                     // [in] Pointer to CM State
    PRENDERHAL_MEDIA_STATE          pMediaState,
    PMHW_BATCH_BUFFER               pBatchBuffer,                               // [in] Pointer to Batch Buffer
    int32_t                         iTaskId,                                    // [in] Task ID
    PCM_HAL_KERNEL_PARAM            pKernelParam,
    int32_t                         iKernelIndex,
    PCM_HAL_INDEX_PARAM             pIndexParam,
    int32_t                         iBindingTable,
    int32_t                         iMediaID,
    PRENDERHAL_KRN_ALLOCATION       pKrnAllocation
    )
{
    MOS_STATUS                      hr = MOS_STATUS_SUCCESS;
    PCM_HAL_TASK_PARAM              pTaskParam = pState->pTaskParam;
    PRENDERHAL_INTERFACE            pRenderHal = pState->pRenderHal;
    PCM_HAL_WALKER_PARAMS           pMediaWalkerParams = &pKernelParam->WalkerParams;
    PCM_GPGPU_WALKER_PARAMS         pPerKernelGpGpuWalkerParams = &pKernelParam->GpGpuWalkerParams;
    PCM_HAL_SCOREBOARD              pThreadCoordinates = nullptr;
    PCM_HAL_MASK_AND_RESET          pDependencyMask = nullptr;
    bool                            enableThreadSpace = false;
    bool                            enableKernelThreadSpace = false;
    PCM_HAL_SCOREBOARD              pKernelThreadCoordinates = nullptr;
    UNUSED(iTaskId);

    MHW_MEDIA_OBJECT_PARAMS         MediaObjectParam;
    PCM_HAL_KERNEL_ARG_PARAM        pArgParam;
    MHW_PIPE_CONTROL_PARAMS         PipeControlParam;
    uint32_t                        i;
    uint32_t                        iHdrSize;
    uint32_t                        aIndex;
    uint32_t                        tIndex;
    uint32_t                        index;

    //GT-PIN
    pTaskParam->iCurKrnIndex =  iKernelIndex;

    CmSafeMemSet(&MediaObjectParam, 0, sizeof(MHW_MEDIA_OBJECT_PARAMS));

    if (pPerKernelGpGpuWalkerParams->CmGpGpuEnable)
    {
        // GPGPU_WALKER, just update ID here. other fields are already filled.
        pPerKernelGpGpuWalkerParams->InterfaceDescriptorOffset = iMediaID;// MediaObjectParam.dwInterfaceDescriptorOffset;
    }
    else if (pMediaWalkerParams->CmWalkerEnable)
    {
        // Media walker, just update ID here. other fields are already filled.
        pMediaWalkerParams->InterfaceDescriptorOffset = iMediaID;
    }
    else
    {
        // MEDIA_OBJECT
        MediaObjectParam.dwInterfaceDescriptorOffset = iMediaID;
        iHdrSize = pRenderHal->pHwSizes->dwSizeMediaObjectHeaderCmd;

        if (pKernelParam->IndirectDataParam.iIndirectDataSize)
        {
            MediaObjectParam.dwInlineDataSize = 0;
        }
        else
        {
            MediaObjectParam.dwInlineDataSize = MOS_MAX(pKernelParam->iPayloadSize, 4);
        }

        if (pTaskParam->ppThreadCoordinates)
        {
            pThreadCoordinates = pTaskParam->ppThreadCoordinates[iKernelIndex];
            if (pThreadCoordinates)
            {
                enableThreadSpace = true;
            }
        }
        else if (pKernelParam->KernelThreadSpaceParam.pThreadCoordinates)
        {
            pKernelThreadCoordinates = pKernelParam->KernelThreadSpaceParam.pThreadCoordinates;
            if (pKernelThreadCoordinates)
            {
                enableKernelThreadSpace = true;
            }
        }

        if (pTaskParam->ppDependencyMasks)
        {
            pDependencyMask = pTaskParam->ppDependencyMasks[iKernelIndex];
        }

        CM_CHK_NULL_RETURN_MOSSTATUS( pBatchBuffer );

        uint8_t inlineData[CM_MAX_THREAD_PAYLOAD_SIZE];
        uint8_t *pCmd_inline = inlineData;
        uint32_t cmd_size = MediaObjectParam.dwInlineDataSize + iHdrSize;

        // Setup states for arguments and threads
        if (((PCM_HAL_BB_ARGS)pBatchBuffer->pPrivateData)->uiRefCount > 1)
        {
            uint8_t *pBBuffer = pBatchBuffer->pData + pBatchBuffer->iCurrent;
            for (aIndex = 0; aIndex < pKernelParam->iNumArgs; aIndex++)
            {
                pArgParam = &pKernelParam->CmArgParams[aIndex];

                if ((pKernelParam->dwCmFlags & CM_KERNEL_FLAGS_CURBE) && !pArgParam->bPerThread)
                {
                    continue;
                }

                for (tIndex = 0; tIndex < pKernelParam->iNumThreads; tIndex++)
                {
                    index = tIndex * pArgParam->bPerThread;

                    //-----------------------------------------------------
                    CM_ASSERT(pArgParam->iPayloadOffset < pKernelParam->iPayloadSize);
                    //-----------------------------------------------------

                    switch(pArgParam->Kind)
                    {
                    case CM_ARGUMENT_GENERAL:
                        break;

                    case CM_ARGUMENT_SAMPLER:
                        CM_CHK_MOSSTATUS(HalCm_SetupSamplerState(
                            pState, pKernelParam, pArgParam, pIndexParam,  iMediaID, index, nullptr));
                        break;

                    case CM_ARGUMENT_SURFACEBUFFER:
                        CM_CHK_MOSSTATUS(HalCm_SetupBufferSurfaceState(
                            pState, pArgParam, pIndexParam, iBindingTable, -1, index, nullptr));
                        break;

                    case CM_ARGUMENT_SURFACE2D_UP:
                        CM_CHK_MOSSTATUS(HalCm_Setup2DSurfaceUPState(
                            pState, pArgParam, pIndexParam, iBindingTable, index, nullptr));
                        break;

                    case CM_ARGUMENT_SURFACE2DUP_SAMPLER:
                        CM_CHK_MOSSTATUS(HalCm_Setup2DSurfaceUPSamplerState(
                            pState, pArgParam, pIndexParam, iBindingTable, index, nullptr));
                        break;

                    case CM_ARGUMENT_SURFACE2D_SAMPLER:
                        CM_CHK_MOSSTATUS(HalCm_Setup2DSurfaceSamplerState(
                            pState, pArgParam, pIndexParam, iBindingTable, 0, nullptr));
                        break;

                    case CM_ARGUMENT_SURFACE2D:
                        CM_CHK_MOSSTATUS(HalCm_Setup2DSurfaceState(
                            pState, pArgParam, pIndexParam, iBindingTable, index, nullptr));
                        break;

                    case CM_ARGUMENT_SURFACE3D:
                        CM_CHK_MOSSTATUS(HalCm_Setup3DSurfaceState(
                            pState, pArgParam, pIndexParam, iBindingTable, index, nullptr));
                        break;

                    case CM_ARGUMENT_SURFACE_VME:
                        CM_CHK_MOSSTATUS(HalCm_SetupVmeSurfaceState(
                            pState, pArgParam, pIndexParam, iBindingTable, 0, nullptr));
                        break;

                    case CM_ARGUMENT_SURFACE_SAMPLER8X8_AVS:
                    case CM_ARGUMENT_SURFACE_SAMPLER8X8_VA:
                        CM_CHK_MOSSTATUS(HalCm_SetupSampler8x8SurfaceState(
                            pState, pArgParam, pIndexParam, iBindingTable, 0, nullptr));
                        break;

                    default:
                        CM_ERROR_ASSERT(
                            "Argument kind '%d' is not supported", pArgParam->Kind);
                        goto finish;
                    }
                }

                if( pDependencyMask )
                {
                    if( pDependencyMask[tIndex].resetMask == CM_RESET_DEPENDENCY_MASK )
                    {
                        MOS_SecureMemcpy(pBBuffer + (CM_SCOREBOARD_MASK_POS_IN_MEDIA_OBJECT_CMD*sizeof(uint32_t)), 
                            sizeof(uint8_t), &pDependencyMask[tIndex].mask, sizeof(uint8_t));
                    }
                }
                pBatchBuffer->iCurrent += cmd_size;
                pBBuffer += cmd_size;
            }
        }
        else
        {
            //Insert synchronization if needed (PIPE_CONTROL)
            // 1. synchronization is set
            // 2. the next kernel has dependency pattern
            if((iKernelIndex > 0) && ((pTaskParam->uiSyncBitmap & ((uint64_t)1 << (iKernelIndex-1))) || (pKernelParam->KernelThreadSpaceParam.patternType != CM_NONE_DEPENDENCY)))
            {
                PipeControlParam = g_cRenderHal_InitPipeControlParams;
                PipeControlParam.presDest                = nullptr;
                PipeControlParam.dwFlushMode             = MHW_FLUSH_CUSTOM; // Use custom flags
                PipeControlParam.dwPostSyncOp            = MHW_FLUSH_NOWRITE;
                PipeControlParam.bDisableCSStall         = false;
                PipeControlParam.bTlbInvalidate          = false;
                PipeControlParam.bFlushRenderTargetCache = true;
                PipeControlParam.bInvalidateTextureCache = true;
                pRenderHal->pMhwMiInterface->AddPipeControl(nullptr, pBatchBuffer, &PipeControlParam);
            }

            uint8_t *pBBuffer = pBatchBuffer->pData + pBatchBuffer->iCurrent;
            for (tIndex = 0; tIndex < pKernelParam->iNumThreads; tIndex++)
            {
                if (enableThreadSpace)
                {
                    MediaObjectParam.VfeScoreboard.ScoreboardEnable = (pState->ScoreboardParams.ScoreboardMask==0) ? 0:1;
                    MediaObjectParam.VfeScoreboard.Value[0] = pThreadCoordinates[tIndex].x;
                    MediaObjectParam.VfeScoreboard.Value[1] = pThreadCoordinates[tIndex].y;
                    MediaObjectParam.VfeScoreboard.ScoreboardColor = pThreadCoordinates[tIndex].color;
                    MediaObjectParam.dwSliceDestinationSelect = pThreadCoordinates[tIndex].sliceSelect;
                    MediaObjectParam.dwHalfSliceDestinationSelect = pThreadCoordinates[tIndex].subSliceSelect;
                    if( !pDependencyMask )
                    {
                        MediaObjectParam.VfeScoreboard.ScoreboardMask = (1 << pState->ScoreboardParams.ScoreboardMask)-1;
                    }
                    else
                    {
                        MediaObjectParam.VfeScoreboard.ScoreboardMask = pDependencyMask[tIndex].mask;
                    }
                }
                else if (enableKernelThreadSpace)
                {
                    MediaObjectParam.VfeScoreboard.ScoreboardEnable = (pState->ScoreboardParams.ScoreboardMask == 0) ? 0 : 1;
                    MediaObjectParam.VfeScoreboard.Value[0] = pKernelThreadCoordinates[tIndex].x;
                    MediaObjectParam.VfeScoreboard.Value[1] = pKernelThreadCoordinates[tIndex].y;
                    MediaObjectParam.VfeScoreboard.ScoreboardColor = pKernelThreadCoordinates[tIndex].color;
                    MediaObjectParam.dwSliceDestinationSelect = pKernelThreadCoordinates[tIndex].sliceSelect;
                    MediaObjectParam.dwHalfSliceDestinationSelect = pKernelThreadCoordinates[tIndex].subSliceSelect;
                    if (!pDependencyMask)
                    {
                        MediaObjectParam.VfeScoreboard.ScoreboardMask = (1 << pState->ScoreboardParams.ScoreboardMask) - 1;
                    }
                    else
                    {
                        MediaObjectParam.VfeScoreboard.ScoreboardMask = pDependencyMask[tIndex].mask;
                    }
                }
                else
                {
                    MediaObjectParam.VfeScoreboard.Value[0] = tIndex % pTaskParam->threadSpaceWidth;
                    MediaObjectParam.VfeScoreboard.Value[1] = tIndex / pTaskParam->threadSpaceWidth;
                }

                for (aIndex = 0; aIndex < pKernelParam->iNumArgs; aIndex++)
                {
                    pArgParam = &pKernelParam->CmArgParams[aIndex];
                    index = tIndex * pArgParam->bPerThread;

                    if ((pKernelParam->dwCmFlags & CM_KERNEL_FLAGS_CURBE) && !pArgParam->bPerThread)
                    {
                        continue;
                    }

                    //-----------------------------------------------------
                    CM_ASSERT(pArgParam->iPayloadOffset < pKernelParam->iPayloadSize);
                    //-----------------------------------------------------

                    switch(pArgParam->Kind)
                    {
                    case CM_ARGUMENT_GENERAL:
                        MOS_SecureMemcpy( 
                            pCmd_inline + pArgParam->iPayloadOffset, 
                            pArgParam->iUnitSize,
                            pArgParam->pFirstValue + index * pArgParam->iUnitSize,
                            pArgParam->iUnitSize);
                        break;

                    case CM_ARGUMENT_SAMPLER:
                        CM_CHK_MOSSTATUS(HalCm_SetupSamplerState(
                            pState, pKernelParam, pArgParam, pIndexParam,  iMediaID, index, pCmd_inline));
                        break;

                    case CM_ARGUMENT_SURFACEBUFFER:
                        CM_CHK_MOSSTATUS(HalCm_SetupBufferSurfaceState(
                            pState, pArgParam, pIndexParam, iBindingTable, -1, index, pCmd_inline));
                        break;

                    case CM_ARGUMENT_SURFACE2D_UP:
                        CM_CHK_MOSSTATUS(HalCm_Setup2DSurfaceUPState(
                            pState, pArgParam, pIndexParam, iBindingTable, index, pCmd_inline));
                        break;

                    case CM_ARGUMENT_SURFACE2DUP_SAMPLER:
                        CM_CHK_MOSSTATUS(HalCm_Setup2DSurfaceUPSamplerState(
                            pState, pArgParam, pIndexParam, iBindingTable, index, pCmd_inline));
                        break;

                    case CM_ARGUMENT_SURFACE2D_SAMPLER:
                        CM_CHK_MOSSTATUS(HalCm_Setup2DSurfaceSamplerState(
                            pState, pArgParam, pIndexParam, iBindingTable, index, pCmd_inline));
                        break;

                    case CM_ARGUMENT_SURFACE2D:
                        CM_CHK_MOSSTATUS(HalCm_Setup2DSurfaceState(
                            pState, pArgParam, pIndexParam, iBindingTable, index, pCmd_inline));
                        break;

                    case CM_ARGUMENT_SURFACE3D:
                        CM_CHK_MOSSTATUS(HalCm_Setup3DSurfaceState(
                            pState, pArgParam, pIndexParam, iBindingTable, index, pCmd_inline));
                        break;

                    case CM_ARGUMENT_SURFACE_VME:
                        CM_CHK_MOSSTATUS(HalCm_SetupVmeSurfaceState(
                            pState, pArgParam, pIndexParam, iBindingTable, 0, pCmd_inline));
                        break;

                    case CM_ARGUMENT_SURFACE_SAMPLER8X8_AVS:
                    case CM_ARGUMENT_SURFACE_SAMPLER8X8_VA:
                        CM_CHK_MOSSTATUS(HalCm_SetupSampler8x8SurfaceState(
                            pState, pArgParam, pIndexParam, iBindingTable, 0, pCmd_inline));
                        break;

                    default:
                        CM_ERROR_ASSERT(
                            "Argument kind '%d' is not supported", pArgParam->Kind);
                        goto finish;
                    }
                }

                MediaObjectParam.pInlineData = inlineData;
                pState->pRenderHal->pMhwRenderInterface->AddMediaObject(nullptr, pBatchBuffer, &MediaObjectParam);
            }
        }
    }

    for (i = 0; i < CM_MAX_GLOBAL_SURFACE_NUMBER; i++) {
        if ((pKernelParam->globalSurface[i] & CM_SURFACE_MASK) != CM_NULL_SURFACE)
        {
             CM_HAL_KERNEL_ARG_PARAM   ArgParam;
             pArgParam = &ArgParam;

             ArgParam.Kind = CM_ARGUMENT_SURFACEBUFFER;
             ArgParam.iPayloadOffset = 0;
             ArgParam.iUnitCount = 1;
             ArgParam.iUnitSize = sizeof(uint32_t);
             ArgParam.bPerThread = false;
             ArgParam.pFirstValue = (uint8_t*)&pKernelParam->globalSurface[i];
             ArgParam.iAliasIndex = 0;

             CM_CHK_MOSSTATUS(HalCm_SetupBufferSurfaceState(
                       pState, pArgParam, pIndexParam, iBindingTable, (int16_t)i, 0, nullptr));
        }
    }

    // set number of samplers
    pKrnAllocation->Params.Sampler_Count = pIndexParam->dwSamplerIndexCount;

    // add SIP surface
    if (pKernelParam->bKernelDebugEnabled)
    {
        CM_CHK_MOSSTATUS(HalCm_SetupSipSurfaceState(pState, pIndexParam, iBindingTable));
    }

finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:  Finishes setting up HW states for the kernel
//|           Used by EnqueueWithHints
//| Returns:  Result of the operation
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_FinishStatesForKernelMix( 
    PCM_HAL_STATE                      pState, 
    PMHW_BATCH_BUFFER                  pBatchBuffer,
    int32_t                            iTaskId,
    PCM_HAL_KERNEL_PARAM*              pCmExecKernels,
    PCM_HAL_INDEX_PARAM                pIndexParams,
    int32_t                            *pBindingTableEntries,
    int32_t                            *pMediaIds,
    PRENDERHAL_KRN_ALLOCATION         *pKrnAllocations,
    uint32_t                           iNumKernels,
    uint32_t                           iHints,
    bool                               lastTask)
{
    MOS_STATUS                         hr                      = MOS_STATUS_SUCCESS;
    PRENDERHAL_INTERFACE               pRenderHal              = pState->pRenderHal;
    PMHW_MEDIA_OBJECT_PARAMS           pMediaObjectParams      = nullptr;
    PCM_HAL_KERNEL_PARAM*              pKernelParams           = nullptr;
    PCM_HAL_KERNEL_ARG_PARAM*          pArgParams              = nullptr;
    PCM_HAL_BB_ARGS                    pBbCmArgs               = nullptr;
    PMHW_VFE_SCOREBOARD                pScoreboardParams       = nullptr;
    PCM_HAL_PARALLELISM_GRAPH_INFO     pParallelGraphInfo      = nullptr;
    PCM_HAL_KERNEL_ARG_PARAM           pArgParam               = nullptr;
    PCM_HAL_KERNEL_SUBSLICE_INFO       pKernelsSliceInfo       = nullptr;
    PCM_HAL_KERNEL_THREADSPACE_PARAM   pKernelTSParam          = nullptr;
    PCM_HAL_KERNEL_GROUP_INFO          pGroupInfo              = nullptr;
    CM_HAL_DEPENDENCY                  vfeDependencyInfo             ;
    CM_PLATFORM_INFO                   platformInfo                  ;
    CM_GT_SYSTEM_INFO                  systemInfo                    ;
    CM_HAL_SCOREBOARD_XY_MASK          threadCoordinates             ;
    uint32_t                           **pDependRemap            = nullptr;
    uint32_t                           **pDispatchFreq           = nullptr;
    uint8_t                            **pCmd_inline             = nullptr;
    uint32_t                           *pCmd_sizes              = nullptr;
    uint32_t                           *pRemapKrnToGrp          = nullptr;
    uint32_t                           *pRemapGrpToKrn          = nullptr;
    uint32_t                           *pNumKernelsPerGrp       = nullptr;
    uint8_t                            *pKernelScoreboardMask   = nullptr;
    uint8_t                            hintsBits               = 0;
    uint8_t                            tmpThreadScoreboardMask = 0;
    uint8_t                            scoreboardMask          = 0;
    bool                               bSingleSubSlice         = false;
    bool                               bEnableThreadSpace      = false;
    bool                               bKernelFound            = false;
    bool                               bUpdateCurrKernel       = false;
    bool                               bNoDependencyCase       = false;
    uint32_t                           iAdjustedYCoord         = 0;
    uint32_t                           numKernelGroups         = CM_HINTS_DEFAULT_NUM_KERNEL_GRP;
    uint32_t                           totalNumThreads         = 0;
    uint32_t                           iHdrSize                = 0;
    uint32_t                           i                       = 0;
    uint32_t                           j                       = 0;
    uint32_t                           k                       = 0;
    uint32_t                           tmp                     = 0;
    uint32_t                           tmp1                    = 0;
    uint32_t                           loopCount               = 0;
    uint32_t                           aIndex                  = 0;
    uint32_t                           index                   = 0;
    uint32_t                           totalReqSubSlices       = 0;
    uint32_t                           difference              = 0;
    uint32_t                           curKernel               = 0;
    uint32_t                           numSet                  = 0;
    uint32_t                           sliceIndex              = 0;
    uint32_t                           tmpNumSubSlice          = 0;
    uint32_t                           tmpNumKernelsPerGrp     = 0;
    uint32_t                           maximum                 = 0;
    uint32_t                           count                   = 0;
    uint32_t                           numDispatched           = 0;
    uint32_t                           tmpIndex                = 0;
    uint32_t                           numStepsDispatched      = 0;
    uint32_t                           minSteps                = UINT_MAX;
    uint32_t                           grpId                   = 0;
    uint32_t                           allocSize               = 0;
    uint32_t                           iCurrentKernel          = 0;
    uint32_t                           iRoundRobinCount        = 0;
    uint32_t                           numTasks                = 0;
    uint32_t                           extraSWThreads          = 0;
    UNUSED(iTaskId);

    CM_CHK_NULL_RETURN_MOSSTATUS(pBatchBuffer);

    MOS_ZeroMemory(&threadCoordinates, sizeof(CM_HAL_SCOREBOARD_XY_MASK));
    MOS_ZeroMemory(&vfeDependencyInfo, sizeof(CM_HAL_DEPENDENCY));
    MOS_ZeroMemory(&platformInfo, sizeof(CM_PLATFORM_INFO));
    MOS_ZeroMemory(&systemInfo, sizeof(CM_GT_SYSTEM_INFO));

    pMediaObjectParams = (PMHW_MEDIA_OBJECT_PARAMS)MOS_AllocAndZeroMemory(sizeof(MHW_MEDIA_OBJECT_PARAMS)*iNumKernels);
    pKernelParams = (PCM_HAL_KERNEL_PARAM*)MOS_AllocAndZeroMemory(sizeof(PCM_HAL_KERNEL_PARAM)*iNumKernels);
    pArgParams    = (PCM_HAL_KERNEL_ARG_PARAM*)MOS_AllocAndZeroMemory(sizeof(PCM_HAL_KERNEL_ARG_PARAM)*iNumKernels);
    pCmd_inline = (uint8_t**)MOS_AllocAndZeroMemory(sizeof(uint8_t*)*iNumKernels);
    pCmd_sizes = (uint32_t*)MOS_AllocAndZeroMemory(sizeof(uint32_t)*iNumKernels);
    pRemapKrnToGrp = (uint32_t*)MOS_AllocAndZeroMemory(sizeof(uint32_t)*iNumKernels);
    pRemapGrpToKrn = (uint32_t*)MOS_AllocAndZeroMemory(sizeof(uint32_t)*iNumKernels);
    pKernelScoreboardMask = (uint8_t*)MOS_AllocAndZeroMemory(sizeof(uint8_t)*iNumKernels);
    pDependRemap = (uint32_t**)MOS_AllocAndZeroMemory(sizeof(uint32_t*)*iNumKernels);
    pParallelGraphInfo = (PCM_HAL_PARALLELISM_GRAPH_INFO)MOS_AllocAndZeroMemory(sizeof(CM_HAL_PARALLELISM_GRAPH_INFO)*iNumKernels);
    pDispatchFreq = (uint32_t**)MOS_AllocAndZeroMemory(sizeof(uint32_t*)*iNumKernels);
    pNumKernelsPerGrp = (uint32_t*)MOS_AllocAndZeroMemory(sizeof(uint32_t)*iNumKernels);

    if( !pMediaObjectParams || !pKernelParams || !pArgParams ||
        !pCmd_inline || !pCmd_sizes ||
        !pRemapKrnToGrp || !pRemapGrpToKrn || !pKernelScoreboardMask || !pDependRemap || 
        !pParallelGraphInfo || !pDispatchFreq || !pNumKernelsPerGrp )
    {
        CM_ERROR_ASSERT("Memory allocation failed in EnqueueWithHints");
        goto finish;
    }

    pState->bEUSaturationEnabled = true;

    hintsBits = (iHints & CM_HINTS_MASK_KERNEL_GROUPS) >> CM_HINTS_NUM_BITS_WALK_OBJ;
    CM_CHK_MOSSTATUS(HalCm_GetNumKernelsPerGroup(hintsBits, iNumKernels, pNumKernelsPerGrp, 
        &numKernelGroups, pRemapKrnToGrp, pRemapGrpToKrn));

    pKernelsSliceInfo = (PCM_HAL_KERNEL_SUBSLICE_INFO)MOS_AllocAndZeroMemory(sizeof(CM_HAL_KERNEL_SUBSLICE_INFO)*numKernelGroups);
    pGroupInfo = (PCM_HAL_KERNEL_GROUP_INFO)MOS_AllocAndZeroMemory(sizeof(CM_HAL_KERNEL_GROUP_INFO)*numKernelGroups);
    if( !pKernelsSliceInfo || !pGroupInfo )
    {
        CM_ERROR_ASSERT("Memory allocation failed in EnqueueWithHints");
        goto finish;
    }

    for( i = 0; i < numKernelGroups; ++i) 
    {
        pGroupInfo[i].numKernelsInGroup = pNumKernelsPerGrp[i];
    }

    iHdrSize = pRenderHal->pHwSizes->dwSizeMediaObjectHeaderCmd;

    for ( i = 0; i < iNumKernels; ++i )
    {
        pKernelParams[i] = pCmExecKernels[i];

        pMediaObjectParams[i].dwInterfaceDescriptorOffset = pMediaIds[i];
        pMediaObjectParams[i].dwInlineDataSize = MOS_MAX(pKernelParams[i]->iPayloadSize, 4);

        pCmd_inline[i] = (uint8_t*)MOS_AllocAndZeroMemory(sizeof(uint8_t) * 1024);
        pCmd_sizes[i] = pMediaObjectParams[i].dwInlineDataSize + iHdrSize;

        totalNumThreads += pKernelParams[i]->iNumThreads;
    }

    numTasks = ( iHints & CM_HINTS_MASK_NUM_TASKS ) >> CM_HINTS_NUM_BITS_TASK_POS;
    if( numTasks > 1 )
    {
        if( lastTask )
        {
            extraSWThreads = totalNumThreads % numTasks;
        }

        totalNumThreads = (totalNumThreads / numTasks) + extraSWThreads;   
    }

    for( i = 0; i < iNumKernels; ++i )
    {
        pDependRemap[i] = (uint32_t*)MOS_AllocAndZeroMemory(sizeof(uint32_t) * CM_HAL_MAX_DEPENDENCY_COUNT);
        for( k = 0; k < CM_HAL_MAX_DEPENDENCY_COUNT; ++k )
        {
            // initialize each index to map to itself
            pDependRemap[i][k] = k;
        }
    }

    for( i = 0; i < iNumKernels; ++i )
    {
        pKernelTSParam = &pKernelParams[i]->KernelThreadSpaceParam;

        // calculate union dependency vector of all kernels with dependency
        if( pKernelTSParam->dependencyInfo.count )
        {
            if( vfeDependencyInfo.count == 0 )
            {
                MOS_SecureMemcpy(&vfeDependencyInfo, sizeof(CM_HAL_DEPENDENCY), &pKernelTSParam->dependencyInfo, sizeof(CM_HAL_DEPENDENCY));
                pKernelScoreboardMask[i] = ( 1 << vfeDependencyInfo.count ) - 1;
            }
            else
            {
                for( j = 0; j < pKernelTSParam->dependencyInfo.count; ++j )
                {
                    for( k = 0; k < vfeDependencyInfo.count; ++k )
                    {
                        if( (pKernelTSParam->dependencyInfo.deltaX[j] == vfeDependencyInfo.deltaX[k]) &&
                            (pKernelTSParam->dependencyInfo.deltaY[j] == vfeDependencyInfo.deltaY[k]) )
                        {
                            CM_HAL_SETBIT(pKernelScoreboardMask[i], k);
                            pDependRemap[i][j] = k;
                            break;
                        }
                    }
                    if ( k == vfeDependencyInfo.count )
                    {
                        vfeDependencyInfo.deltaX[vfeDependencyInfo.count] = pKernelTSParam->dependencyInfo.deltaX[j];
                        vfeDependencyInfo.deltaY[vfeDependencyInfo.count] = pKernelTSParam->dependencyInfo.deltaY[j];
                        CM_HAL_SETBIT(pKernelScoreboardMask[i], vfeDependencyInfo.count);
                        vfeDependencyInfo.count++;
                        pDependRemap[i][j] = k;
                    }
                }
            }
        }
    } // for num kernels

    if( vfeDependencyInfo.count > CM_HAL_MAX_DEPENDENCY_COUNT )
    {
        CM_ERROR_ASSERT("Union of kernel dependencies exceeds max dependency count (8)");
        goto finish;
    }

    // set VFE scoreboarding information from union of kernel dependency vectors
    pScoreboardParams = &pState->ScoreboardParams;
    pScoreboardParams->ScoreboardMask = (uint8_t)vfeDependencyInfo.count;
    for( i = 0; i < pScoreboardParams->ScoreboardMask; ++i )
    {
        pScoreboardParams->ScoreboardDelta[i].x = vfeDependencyInfo.deltaX[i];
        pScoreboardParams->ScoreboardDelta[i].y = vfeDependencyInfo.deltaY[i];
    }

    if (vfeDependencyInfo.count == 0)
    {
        bNoDependencyCase = true;
    }

    CM_CHK_MOSSTATUS(pState->pfnGetPlatformInfo(pState, &platformInfo, true));
    bSingleSubSlice = (platformInfo.numSubSlices == 1) ? true : false;

    CM_CHK_MOSSTATUS(pState->pfnGetGTSystemInfo(pState, &systemInfo));

    if( !bSingleSubSlice )
    {
        for( i = 0; i < numKernelGroups; ++i )
        {
            tmpNumKernelsPerGrp = pNumKernelsPerGrp[i];

            for( j = 0; j < tmpNumKernelsPerGrp; ++j ) 
            {
                pKernelTSParam = &pKernelParams[count]->KernelThreadSpaceParam;

                switch( pKernelTSParam->patternType )
                {
                case CM_NONE_DEPENDENCY:
                    maximum = pKernelParams[count]->iNumThreads;
                    break;
                case CM_WAVEFRONT:
                    maximum = MOS_MIN(pKernelTSParam->iThreadSpaceWidth, pKernelTSParam->iThreadSpaceHeight);
                    break;
                case CM_WAVEFRONT26:
                    maximum = MOS_MIN( ((pKernelTSParam->iThreadSpaceWidth + 1) >> 1), pKernelTSParam->iThreadSpaceHeight);
                    break;
                case CM_VERTICAL_WAVE:
                    maximum = pKernelTSParam->iThreadSpaceHeight;
                    break;
                case CM_HORIZONTAL_WAVE:
                    maximum = pKernelTSParam->iThreadSpaceWidth;
                    break;
                case CM_WAVEFRONT26Z:
                    maximum = MOS_MIN( ((pKernelTSParam->iThreadSpaceWidth - 1) >> 1), pKernelTSParam->iThreadSpaceHeight);
                    break;
                default:
                    CM_ERROR_ASSERT("Unsupported dependency pattern for EnqueueWithHints");
                    goto finish;
                }

                if( pKernelTSParam->patternType != CM_WAVEFRONT26Z )
                {
                    CM_CHK_MOSSTATUS(HalCm_GetParallelGraphInfo(maximum, pKernelParams[count]->iNumThreads, 
                        pKernelTSParam->iThreadSpaceWidth, pKernelTSParam->iThreadSpaceHeight,
                        &pParallelGraphInfo[count], pKernelTSParam->patternType, bNoDependencyCase));
                }
                else
                {
                    pParallelGraphInfo[count].numSteps = pKernelTSParam->dispatchInfo.numWaves;
                }

                if( pKernelTSParam->patternType != CM_NONE_DEPENDENCY )
                {
                    pDispatchFreq[count] = (uint32_t*)MOS_AllocAndZeroMemory(sizeof(uint32_t)*pParallelGraphInfo[count].numSteps);
                    if( !pDispatchFreq[count] )
                    {
                        CM_ERROR_ASSERT("Memory allocation failed for EnqueueWithHints");
                        goto finish;
                    }

                    if( pKernelTSParam->patternType != CM_WAVEFRONT26Z )
                    {
                        CM_CHK_MOSSTATUS(HalCm_SetDispatchPattern(pParallelGraphInfo[count], pKernelTSParam->patternType, pDispatchFreq[count]));
                    }
                    else
                    {
                        MOS_SecureMemcpy(pDispatchFreq[count], sizeof(uint32_t)*pParallelGraphInfo[count].numSteps,
                                         pKernelTSParam->dispatchInfo.pNumThreadsInWave, sizeof(uint32_t)*pParallelGraphInfo[count].numSteps);
                    }
                }

                if (!bNoDependencyCase)
                {
                    tmpNumSubSlice =
                        (maximum / (platformInfo.numEUsPerSubSlice * platformInfo.numHWThreadsPerEU)) + 1;

                    if (tmpNumSubSlice > platformInfo.numSubSlices)
                    {
                        tmpNumSubSlice = platformInfo.numSubSlices - 1;
                    }


                    if (tmpNumSubSlice > pKernelsSliceInfo[i].numSubSlices)
                    {
                        pKernelsSliceInfo[i].numSubSlices = tmpNumSubSlice;
                    }
                }
                else
                {
                    pKernelsSliceInfo[i].numSubSlices = platformInfo.numSubSlices;
                }

                count++;
            }
        }

        if (!bNoDependencyCase)
        {
            for (i = 0; i < numKernelGroups; ++i)
            {
                totalReqSubSlices += pKernelsSliceInfo[i].numSubSlices;
            }

            // adjust if requested less or more subslices than architecture has
            if (totalReqSubSlices < platformInfo.numSubSlices)
            {
                // want to add subslices starting from K0
                difference = platformInfo.numSubSlices - totalReqSubSlices;
                tmp = tmp1 = 0;
                for (i = 0; i < difference; ++i)
                {
                    tmp = tmp1 % numKernelGroups;
                    pKernelsSliceInfo[tmp].numSubSlices++;
                    totalReqSubSlices++;
                    tmp1++;
                }
            }
            else if (totalReqSubSlices > platformInfo.numSubSlices)
            {
                // want to subtract subslices starting from last kernel
                difference = totalReqSubSlices - platformInfo.numSubSlices;
                tmp = 0;
                tmp1 = numKernelGroups - 1;
                for (i = numKernelGroups - 1, j = 0; j < difference; --i, ++j)
                {
                    tmp = tmp1 % numKernelGroups;
                    pKernelsSliceInfo[tmp].numSubSlices--;
                    totalReqSubSlices--;
                    tmp1 += numKernelGroups - 1;
                }
            }

            if (totalReqSubSlices != platformInfo.numSubSlices)
            {
                CM_ERROR_ASSERT("Total requested sub-slices does not match platform's number of sub-slices");
                goto finish;
            }
        }

        for(i = 0; i < numKernelGroups; ++i)
        {
            pKernelsSliceInfo[i].pDestination = (PCM_HAL_KERNEL_SLICE_SUBSLICE)MOS_AllocAndZeroMemory(sizeof(CM_HAL_KERNEL_SLICE_SUBSLICE)*pKernelsSliceInfo[i].numSubSlices);
            if( !pKernelsSliceInfo[i].pDestination )
            {
                CM_ERROR_ASSERT("Memory allocation failed in EnqueueWithHints");
                goto finish;
            }
        }

        // set slice, subslice for each kernel group
        if (systemInfo.isSliceInfoValid)
        {
            for (i = 0; i < systemInfo.numMaxSlicesSupported; ++i)
            {
                for (j = 0; j < (systemInfo.numMaxSubSlicesSupported / systemInfo.numMaxSlicesSupported); ++j)
                {
                    if (systemInfo.sliceInfo[i].SubSliceInfo[j].Enabled && systemInfo.sliceInfo[i].Enabled)
                    {
                        if (pKernelsSliceInfo[curKernel].numSubSlices == numSet)
                        {
                            curKernel++;
                            numSet = 0;
                        }

                        pKernelsSliceInfo[curKernel].pDestination[numSet].slice = i;
                        pKernelsSliceInfo[curKernel].pDestination[numSet].subSlice = j;

                        numSet++;
                    }
                }
            }
        }

        // set freq dispatch ratio for each group
        CM_CHK_MOSSTATUS(HalCm_SetKernelGrpFreqDispatch(pParallelGraphInfo, pGroupInfo, numKernelGroups, &minSteps));

        // set dispatch pattern for kernel with no dependency
        for( i = 0; i < iNumKernels; ++i )
        {
            if( pKernelParams[i]->KernelThreadSpaceParam.patternType == CM_NONE_DEPENDENCY )
            {
                grpId = pRemapKrnToGrp[i];
                allocSize = 0;

                if( pGroupInfo[grpId].freqDispatch == 0 )
                {
                    allocSize = minSteps;
                    pGroupInfo[grpId].freqDispatch = 1;
                }
                else
                {
                    allocSize = minSteps * pGroupInfo[grpId].freqDispatch;
                    pGroupInfo[grpId].freqDispatch = pGroupInfo[grpId].freqDispatch * 2;
                }

                pDispatchFreq[i] = (uint32_t*)MOS_AllocAndZeroMemory(sizeof(uint32_t)*allocSize);
                if( !pDispatchFreq[i] )
                {
                    CM_ERROR_ASSERT("Memory allocation failed in EnqueueWithHints");
                    goto finish;
                }

                CM_CHK_MOSSTATUS(HalCm_SetNoDependKernelDispatchPattern(pKernelParams[i]->iNumThreads, 
                    allocSize, pDispatchFreq[i]));
            }
        }
    }

    CM_CHK_NULL_RETURN_MOSSTATUS(pBatchBuffer->pPrivateData);
    pBbCmArgs = (PCM_HAL_BB_ARGS) pBatchBuffer->pPrivateData;
    if( pBbCmArgs->uiRefCount > 1 )
    {

        uint8_t *pBBuffer = pBatchBuffer->pData + pBatchBuffer->iCurrent;
        bUpdateCurrKernel = false;
        for( i = 0; i < totalNumThreads; ++i )
        {
            if( !bSingleSubSlice )
            {
                if( (pDispatchFreq[iCurrentKernel][pState->HintIndexes.iDispatchIndexes[iCurrentKernel]] == numDispatched) ||
                    (pState->HintIndexes.iKernelIndexes[iCurrentKernel] >= pKernelParams[iCurrentKernel]->iNumThreads) )
                {
                    numDispatched = 0;
                    numStepsDispatched++;
                    pState->HintIndexes.iDispatchIndexes[iCurrentKernel]++;

                    if( pState->HintIndexes.iKernelIndexes[iCurrentKernel] >= pKernelParams[iCurrentKernel]->iNumThreads )
                    {
                        bUpdateCurrKernel = true;
                        pGroupInfo[pRemapKrnToGrp[iCurrentKernel]].numKernelsFinished++;
                        if( pGroupInfo[pRemapKrnToGrp[iCurrentKernel]].numKernelsFinished == 
                            pGroupInfo[pRemapKrnToGrp[iCurrentKernel]].numKernelsInGroup )
                        {
                            pGroupInfo[pRemapKrnToGrp[iCurrentKernel]].groupFinished = 1;
                        }
                        else
                        {
                            pRemapGrpToKrn[tmpIndex]++;
                        }
                    }

                    if( (pGroupInfo[pRemapKrnToGrp[iCurrentKernel]].freqDispatch == numStepsDispatched) ||
                        bUpdateCurrKernel )
                    {
                        numStepsDispatched = 0;
                        iRoundRobinCount++;

                        tmpIndex = iRoundRobinCount % numKernelGroups;

                        if( pGroupInfo[tmpIndex].groupFinished )
                        {
                            loopCount = 0;
                            while( (loopCount < numKernelGroups) && (!bKernelFound) )
                            {
                                iRoundRobinCount++;
                                tmpIndex = iRoundRobinCount % numKernelGroups;
                                if( pState->HintIndexes.iKernelIndexes[pRemapGrpToKrn[tmpIndex]] < pKernelParams[pRemapGrpToKrn[tmpIndex]]->iNumThreads )
                                {
                                    bKernelFound = true;
                                }
                                loopCount++;
                            }
                            if( !bKernelFound )
                            {
                                // Error shouldn't be here
                                // if still in for loop totalNumThreads, needs to be a kernel with threads left
                                CM_ERROR_ASSERT("Couldn't find kernel with threads left for EnqueueWithHints");
                                goto finish;
                            }
                        }
                        iCurrentKernel = pRemapGrpToKrn[tmpIndex];
                    }
                }
            }
            else
            {
                 if( pState->HintIndexes.iKernelIndexes[iCurrentKernel] >= pKernelParams[iCurrentKernel]->iNumThreads )
                 {
                     iCurrentKernel++;
                 }
            }

            if( pKernelParams[iCurrentKernel]->KernelThreadSpaceParam.pThreadCoordinates )
            {
                threadCoordinates.y = pKernelParams[iCurrentKernel]->KernelThreadSpaceParam.pThreadCoordinates[pState->HintIndexes.iKernelIndexes[iCurrentKernel]].y;
                threadCoordinates.mask = pKernelParams[iCurrentKernel]->KernelThreadSpaceParam.pThreadCoordinates[pState->HintIndexes.iKernelIndexes[iCurrentKernel]].mask;
                bEnableThreadSpace = true;
                threadCoordinates.resetMask = pKernelParams[iCurrentKernel]->KernelThreadSpaceParam.pThreadCoordinates[pState->HintIndexes.iKernelIndexes[iCurrentKernel]].resetMask;
            }

             if( bEnableThreadSpace )
             {
                 if( threadCoordinates.mask != CM_DEFAULT_THREAD_DEPENDENCY_MASK )
                 {
                     tmpThreadScoreboardMask = pKernelScoreboardMask[iCurrentKernel];
                     // do the remapping
                     for( k = 0; k < pKernelParams[iCurrentKernel]->KernelThreadSpaceParam.dependencyInfo.count; ++k )
                     {
                         if( (threadCoordinates.mask & CM_HINTS_LEASTBIT_MASK) == 0 )
                         {
                             CM_HAL_UNSETBIT(tmpThreadScoreboardMask, pDependRemap[iCurrentKernel][k]);
                         }

                         threadCoordinates.mask = threadCoordinates.mask >> 1;
                     }
                         scoreboardMask = tmpThreadScoreboardMask;
                 }
                 else
                 {
                     scoreboardMask = pKernelScoreboardMask[iCurrentKernel];
                 }
             }
             else
             {
                 threadCoordinates.y = pState->HintIndexes.iKernelIndexes[iCurrentKernel] / pKernelParams[iCurrentKernel]->KernelThreadSpaceParam.iThreadSpaceWidth;
                 scoreboardMask = pKernelScoreboardMask[iCurrentKernel];
             }

             iAdjustedYCoord = 0;
             if( iCurrentKernel > 0 )
             {
                 // if not first kernel, and has dependency, 
                 // and along scoreboard border top need to mask out dependencies with y < 0
                 if( pKernelScoreboardMask[iCurrentKernel] )
                 {
                     if( threadCoordinates.y == 0 )
                     {
                         for( k = 0; k < vfeDependencyInfo.count; ++k )
                         {
                             if( vfeDependencyInfo.deltaY[k] < 0 )
                             {
                                 CM_HAL_UNSETBIT(scoreboardMask, k);
                             }
                         }
                     }
                 }
             }

             if( iCurrentKernel < iNumKernels - 1 )
             {
                 // if not last kernel, and has dependency,
                 // along scoreboard border bottom need to mask out dependencies with y > 0
                 if( pKernelScoreboardMask[iCurrentKernel] )
                 {
                     if( threadCoordinates.y == (pKernelParams[iCurrentKernel]->KernelThreadSpaceParam.iThreadSpaceHeight - 1))
                     {
                         for( k = 0; k < vfeDependencyInfo.count; ++k)
                         {
                             if( vfeDependencyInfo.deltaY[k] > 0 )
                             {
                                 CM_HAL_UNSETBIT(scoreboardMask, k);
                             }
                         }
                     }
                 }
             }

            for( aIndex = 0; aIndex < pKernelParams[iCurrentKernel]->iNumArgs; aIndex++ )
            {
                pArgParams[iCurrentKernel] = &pKernelParams[iCurrentKernel]->CmArgParams[aIndex];
                index = pState->HintIndexes.iKernelIndexes[iCurrentKernel] * pArgParams[iCurrentKernel]->bPerThread;

                if( (pKernelParams[iCurrentKernel]->dwCmFlags & CM_KERNEL_FLAGS_CURBE) && !pArgParams[iCurrentKernel]->bPerThread )
                {
                    continue;
                }

                CM_ASSERT(pArgParams[iCurrentKernel]->iPayloadOffset < pKernelParams[iCurrentKernel]->iPayloadSize);

                switch(pArgParams[iCurrentKernel]->Kind)
                {
                case CM_ARGUMENT_GENERAL:
                    break;

                case CM_ARGUMENT_SAMPLER:
                    CM_CHK_MOSSTATUS(HalCm_SetupSamplerState(
                        pState, pKernelParams[iCurrentKernel], pArgParams[iCurrentKernel], &pIndexParams[iCurrentKernel],
                        pMediaIds[iCurrentKernel], index, nullptr));
                    break;

                case CM_ARGUMENT_SURFACEBUFFER:
                    CM_CHK_MOSSTATUS(HalCm_SetupBufferSurfaceState(
                        pState, pArgParams[iCurrentKernel], &pIndexParams[iCurrentKernel],
                        pBindingTableEntries[iCurrentKernel], -1, index, nullptr));
                    break;

                case CM_ARGUMENT_SURFACE2D_UP:
                    CM_CHK_MOSSTATUS(HalCm_Setup2DSurfaceUPState(
                        pState, pArgParams[iCurrentKernel], &pIndexParams[iCurrentKernel],
                        pBindingTableEntries[iCurrentKernel], index, nullptr));
                    break;

                case CM_ARGUMENT_SURFACE2DUP_SAMPLER:
                    CM_CHK_MOSSTATUS(HalCm_Setup2DSurfaceUPSamplerState(
                        pState, pArgParams[iCurrentKernel], &pIndexParams[iCurrentKernel],
                        pBindingTableEntries[iCurrentKernel], index, nullptr));
                    break;

                case CM_ARGUMENT_SURFACE2D_SAMPLER:
                    CM_CHK_MOSSTATUS(HalCm_Setup2DSurfaceSamplerState(
                        pState, pArgParams[iCurrentKernel], &pIndexParams[iCurrentKernel], 
                        pBindingTableEntries[iCurrentKernel], 0, nullptr));
                    break;

                case CM_ARGUMENT_SURFACE2D:
                    CM_CHK_MOSSTATUS(HalCm_Setup2DSurfaceState(
                        pState, pArgParams[iCurrentKernel], &pIndexParams[iCurrentKernel],
                        pBindingTableEntries[iCurrentKernel], index, nullptr));
                    break;

                case CM_ARGUMENT_SURFACE3D:
                    CM_CHK_MOSSTATUS(HalCm_Setup3DSurfaceState(
                        pState, pArgParams[iCurrentKernel], &pIndexParams[iCurrentKernel],
                        pBindingTableEntries[iCurrentKernel], index, nullptr));
                    break;

                case CM_ARGUMENT_SURFACE_VME:
                    CM_CHK_MOSSTATUS(HalCm_SetupVmeSurfaceState(
                        pState, pArgParams[iCurrentKernel], &pIndexParams[iCurrentKernel], 
                        pBindingTableEntries[iCurrentKernel], 0, nullptr));
                    break;

                case CM_ARGUMENT_SURFACE_SAMPLER8X8_VA:
                case CM_ARGUMENT_SURFACE_SAMPLER8X8_AVS:
                    CM_CHK_MOSSTATUS(HalCm_SetupSampler8x8SurfaceState(
                        pState, pArgParams[iCurrentKernel], &pIndexParams[iCurrentKernel],
                        pBindingTableEntries[iCurrentKernel], 0, nullptr));
                    break;

                default:
                    hr = MOS_STATUS_UNKNOWN;
                    CM_ERROR_ASSERT(
                        "Argument kind '%d' is not supported", pArgParams[iCurrentKernel]->Kind);
                    goto finish;

                 } // switch argKind
             } // for numArgs

            if( threadCoordinates.resetMask == CM_RESET_DEPENDENCY_MASK )
            {
                MOS_SecureMemcpy(pBBuffer + (CM_SCOREBOARD_MASK_POS_IN_MEDIA_OBJECT_CMD*sizeof(uint32_t)), 
                    sizeof(uint8_t), &scoreboardMask, sizeof(uint8_t));
            }

            pBatchBuffer->iCurrent += pCmd_sizes[iCurrentKernel];
            pBBuffer += pCmd_sizes[iCurrentKernel];

            pState->HintIndexes.iKernelIndexes[iCurrentKernel]++;
            bEnableThreadSpace = false;
            bKernelFound = false;
            bUpdateCurrKernel = false;
            numDispatched++;
        } // for totalNumThreads
    } // if uiRefCount > 1
    else
    {
        uint8_t *pBBuffer = pBatchBuffer->pData + pBatchBuffer->iCurrent;
        bUpdateCurrKernel = false;

        for( i = 0; i < totalNumThreads; ++i)
        {
            if( !bSingleSubSlice )
            {
                if( (pDispatchFreq[iCurrentKernel][pState->HintIndexes.iDispatchIndexes[iCurrentKernel]] == numDispatched) ||
                    (pState->HintIndexes.iKernelIndexes[iCurrentKernel] >= pKernelParams[iCurrentKernel]->iNumThreads) )
                {
                    numDispatched = 0;
                    numStepsDispatched++;
                    pState->HintIndexes.iDispatchIndexes[iCurrentKernel]++;

                    if( pState->HintIndexes.iKernelIndexes[iCurrentKernel] >= pKernelParams[iCurrentKernel]->iNumThreads )
                    {
                        bUpdateCurrKernel = true;
                        pGroupInfo[pRemapKrnToGrp[iCurrentKernel]].numKernelsFinished++;
                        if( pGroupInfo[pRemapKrnToGrp[iCurrentKernel]].numKernelsFinished == 
                            pGroupInfo[pRemapKrnToGrp[iCurrentKernel]].numKernelsInGroup )
                        {
                            pGroupInfo[pRemapKrnToGrp[iCurrentKernel]].groupFinished = 1;
                        }
                        else
                        {
                            pRemapGrpToKrn[tmpIndex]++;
                        }
                    }

                    if( (pGroupInfo[pRemapKrnToGrp[iCurrentKernel]].freqDispatch == numStepsDispatched) ||
                        bUpdateCurrKernel )
                    {
                        numStepsDispatched = 0;
                        iRoundRobinCount++;

                        tmpIndex = iRoundRobinCount % numKernelGroups;

                        if( pGroupInfo[tmpIndex].groupFinished )
                        {
                            loopCount = 0;
                            while( (loopCount < numKernelGroups) && (!bKernelFound) )
                            {
                                iRoundRobinCount++;
                                tmpIndex = iRoundRobinCount % numKernelGroups;
                                if( pState->HintIndexes.iKernelIndexes[pRemapGrpToKrn[tmpIndex]] < pKernelParams[pRemapGrpToKrn[tmpIndex]]->iNumThreads )
                                {
                                    bKernelFound = true;
                                }
                                loopCount++;
                            }
                            if( !bKernelFound )
                            {
                                // Error shouldn't be here
                                // if still in for loop totalNumThreads, needs to be a kernel with threads left
                                CM_ERROR_ASSERT("Couldn't find kernel with threads left for EnqueueWithHints");
                                goto finish;
                            }
                        }

                        iCurrentKernel = pRemapGrpToKrn[tmpIndex];
                    }
                }
            }
            else
            {
                if( pState->HintIndexes.iKernelIndexes[iCurrentKernel] >= pKernelParams[iCurrentKernel]->iNumThreads )
                {
                    iCurrentKernel++;
                }
            }

            if( pKernelParams[iCurrentKernel]->KernelThreadSpaceParam.pThreadCoordinates )
            {
                threadCoordinates.x = pKernelParams[iCurrentKernel]->KernelThreadSpaceParam.pThreadCoordinates[pState->HintIndexes.iKernelIndexes[iCurrentKernel]].x;
                threadCoordinates.y = pKernelParams[iCurrentKernel]->KernelThreadSpaceParam.pThreadCoordinates[pState->HintIndexes.iKernelIndexes[iCurrentKernel]].y;
                threadCoordinates.mask = pKernelParams[iCurrentKernel]->KernelThreadSpaceParam.pThreadCoordinates[pState->HintIndexes.iKernelIndexes[iCurrentKernel]].mask;
                bEnableThreadSpace = true;
            }

            pMediaObjectParams[iCurrentKernel].VfeScoreboard.ScoreboardEnable =
                    (pKernelParams[iCurrentKernel]->KernelThreadSpaceParam.dependencyInfo.count == 0) ? 0:1;

            if( !bSingleSubSlice && systemInfo.isSliceInfoValid )
            {
                sliceIndex = pKernelsSliceInfo[pRemapKrnToGrp[iCurrentKernel]].counter % pKernelsSliceInfo[pRemapKrnToGrp[iCurrentKernel]].numSubSlices;
                pMediaObjectParams[iCurrentKernel].dwSliceDestinationSelect = pKernelsSliceInfo[pRemapKrnToGrp[iCurrentKernel]].pDestination[sliceIndex].slice;
                pMediaObjectParams[iCurrentKernel].dwHalfSliceDestinationSelect = pKernelsSliceInfo[pRemapKrnToGrp[iCurrentKernel]].pDestination[sliceIndex].subSlice;
                pMediaObjectParams[iCurrentKernel].bForceDestination = true;

                pKernelsSliceInfo[pRemapKrnToGrp[iCurrentKernel]].counter++;
            }

            if( bEnableThreadSpace )
            {
                pMediaObjectParams[iCurrentKernel].VfeScoreboard.Value[0] = threadCoordinates.x;
                pMediaObjectParams[iCurrentKernel].VfeScoreboard.Value[1] = threadCoordinates.y;
                if( threadCoordinates.mask != CM_DEFAULT_THREAD_DEPENDENCY_MASK )
                {
                    tmpThreadScoreboardMask = pKernelScoreboardMask[iCurrentKernel];
                    // do the remapping
                    for( k = 0; k < pKernelParams[iCurrentKernel]->KernelThreadSpaceParam.dependencyInfo.count; ++k )
                    {
                        if( (threadCoordinates.mask & CM_HINTS_LEASTBIT_MASK) == 0 )
                        {
                            CM_HAL_UNSETBIT(tmpThreadScoreboardMask, pDependRemap[iCurrentKernel][k]);
                        }

                        threadCoordinates.mask = threadCoordinates.mask >> 1;
                    }

                    pMediaObjectParams[iCurrentKernel].VfeScoreboard.ScoreboardMask = tmpThreadScoreboardMask;
                }
                else
                {
                    pMediaObjectParams[iCurrentKernel].VfeScoreboard.ScoreboardMask = pKernelScoreboardMask[iCurrentKernel];
                }
            }
            else
            {
                pMediaObjectParams[iCurrentKernel].VfeScoreboard.Value[0] = pState->HintIndexes.iKernelIndexes[iCurrentKernel] % 
                        pKernelParams[iCurrentKernel]->KernelThreadSpaceParam.iThreadSpaceWidth;
                pMediaObjectParams[iCurrentKernel].VfeScoreboard.Value[1] = pState->HintIndexes.iKernelIndexes[iCurrentKernel] /
                        pKernelParams[iCurrentKernel]->KernelThreadSpaceParam.iThreadSpaceWidth;
                pMediaObjectParams[iCurrentKernel].VfeScoreboard.ScoreboardMask = pKernelScoreboardMask[iCurrentKernel];
            }

             iAdjustedYCoord = 0;
             // adjust y coordinate for kernels after the first one
             if( iCurrentKernel > 0 )
             {
                 // if not first kernel, and has dependency, 
                 // and along scoreboard border need to mask out dependencies with y < 0
                 if( pKernelScoreboardMask[iCurrentKernel] )
                 {
                     if (pMediaObjectParams[iCurrentKernel].VfeScoreboard.Value[1] == 0)
                     {
                         for( k = 0; k < vfeDependencyInfo.count; ++k )
                         {
                             if( vfeDependencyInfo.deltaY[k] < 0 )
                             {
                                 CM_HAL_UNSETBIT(pMediaObjectParams[iCurrentKernel].VfeScoreboard.ScoreboardMask, k);
                             }
                         }
                     }
                 }

                 for( j = iCurrentKernel; j > 0; --j )
                 {
                     iAdjustedYCoord += pKernelParams[j-1]->KernelThreadSpaceParam.iThreadSpaceHeight;
                 }
             }

             if( iCurrentKernel < iNumKernels - 1 )
             {
                 // if not last kernel, and has dependency,
                 // along scoreboard border bottom need to mask out dependencies with y > 0
                 if( pKernelScoreboardMask[iCurrentKernel] )
                 {
                     if (pMediaObjectParams[iCurrentKernel].VfeScoreboard.Value[1] == 
                           (pKernelParams[iCurrentKernel]->KernelThreadSpaceParam.iThreadSpaceHeight - 1))
                     {
                         for( k = 0; k < vfeDependencyInfo.count; ++k )
                         {
                             if( vfeDependencyInfo.deltaY[k] > 0 )
                             {
                                 CM_HAL_UNSETBIT(pMediaObjectParams[iCurrentKernel].VfeScoreboard.ScoreboardMask, k);
                             }
                         }
                     }
                 }
             }

            pMediaObjectParams[iCurrentKernel].VfeScoreboard.Value[1] =
                pMediaObjectParams[iCurrentKernel].VfeScoreboard.Value[1] + iAdjustedYCoord;

            for( aIndex = 0; aIndex < pKernelParams[iCurrentKernel]->iNumArgs; aIndex++ )
            {
                pArgParams[iCurrentKernel] = &pKernelParams[iCurrentKernel]->CmArgParams[aIndex];
                index = pState->HintIndexes.iKernelIndexes[iCurrentKernel] * pArgParams[iCurrentKernel]->bPerThread;

                if( (pKernelParams[iCurrentKernel]->dwCmFlags & CM_KERNEL_FLAGS_CURBE) && !pArgParams[iCurrentKernel]->bPerThread )
                {
                    continue;
                }

                CM_ASSERT(pArgParams[iCurrentKernel]->iPayloadOffset < pKernelParams[iCurrentKernel]->iPayloadSize);

                switch(pArgParams[iCurrentKernel]->Kind)
                {
                case CM_ARGUMENT_GENERAL:
                    MOS_SecureMemcpy(
                        pCmd_inline[iCurrentKernel] + pArgParams[iCurrentKernel]->iPayloadOffset,
                        pArgParams[iCurrentKernel]->iUnitSize,
                        pArgParams[iCurrentKernel]->pFirstValue + index * pArgParams[iCurrentKernel]->iUnitSize,
                        pArgParams[iCurrentKernel]->iUnitSize);
                    break;

                case CM_ARGUMENT_SAMPLER:
                    CM_CHK_MOSSTATUS(HalCm_SetupSamplerState(
                        pState, pKernelParams[iCurrentKernel], pArgParams[iCurrentKernel], &pIndexParams[iCurrentKernel],
                        pMediaIds[iCurrentKernel], index, pCmd_inline[iCurrentKernel]));
                    break;

                case CM_ARGUMENT_SURFACEBUFFER:
                    CM_CHK_MOSSTATUS(HalCm_SetupBufferSurfaceState(
                        pState, pArgParams[iCurrentKernel], &pIndexParams[iCurrentKernel],
                        pBindingTableEntries[iCurrentKernel], -1, index, pCmd_inline[iCurrentKernel]));
                    break;

                case CM_ARGUMENT_SURFACE2D_UP:
                    CM_CHK_MOSSTATUS(HalCm_Setup2DSurfaceUPState(
                        pState, pArgParams[iCurrentKernel], &pIndexParams[iCurrentKernel],
                        pBindingTableEntries[iCurrentKernel], index, pCmd_inline[iCurrentKernel]));
                    break;

                case CM_ARGUMENT_SURFACE2DUP_SAMPLER:
                    CM_CHK_MOSSTATUS(HalCm_Setup2DSurfaceUPSamplerState(
                        pState, pArgParams[iCurrentKernel], &pIndexParams[iCurrentKernel],
                        pBindingTableEntries[iCurrentKernel], index, pCmd_inline[iCurrentKernel]));
                    break;

                case CM_ARGUMENT_SURFACE2D_SAMPLER:
                    CM_CHK_MOSSTATUS(HalCm_Setup2DSurfaceSamplerState(
                        pState, pArgParams[iCurrentKernel], &pIndexParams[iCurrentKernel], 
                        pBindingTableEntries[iCurrentKernel], index, pCmd_inline[iCurrentKernel]));
                    break;

                case CM_ARGUMENT_SURFACE2D:
                    CM_CHK_MOSSTATUS(HalCm_Setup2DSurfaceState(
                        pState, pArgParams[iCurrentKernel], &pIndexParams[iCurrentKernel],
                        pBindingTableEntries[iCurrentKernel], index, pCmd_inline[iCurrentKernel]));
                    break;

                case CM_ARGUMENT_SURFACE3D:
                    CM_CHK_MOSSTATUS(HalCm_Setup3DSurfaceState(
                        pState, pArgParams[iCurrentKernel], &pIndexParams[iCurrentKernel],
                        pBindingTableEntries[iCurrentKernel], index, pCmd_inline[iCurrentKernel]));
                    break;

                case CM_ARGUMENT_SURFACE_VME:
                    CM_CHK_MOSSTATUS(HalCm_SetupVmeSurfaceState(
                        pState, pArgParams[iCurrentKernel], &pIndexParams[iCurrentKernel], 
                        pBindingTableEntries[iCurrentKernel], 0, pCmd_inline[iCurrentKernel]));
                    break;

                case CM_ARGUMENT_SURFACE_SAMPLER8X8_VA:
                case CM_ARGUMENT_SURFACE_SAMPLER8X8_AVS:
                    CM_CHK_MOSSTATUS(HalCm_SetupSampler8x8SurfaceState(
                        pState, pArgParams[iCurrentKernel], &pIndexParams[iCurrentKernel],
                        pBindingTableEntries[iCurrentKernel], 0, pCmd_inline[iCurrentKernel]));
                    break;

                default:
                    hr = MOS_STATUS_UNKNOWN;
                    CM_ERROR_ASSERT(
                        "Argument kind '%d' is not supported", pArgParams[iCurrentKernel]->Kind);
                    goto finish;
                }
            }

            pMediaObjectParams[iCurrentKernel].pInlineData = pCmd_inline[iCurrentKernel];
            pState->pRenderHal->pMhwRenderInterface->AddMediaObject(nullptr, pBatchBuffer, &pMediaObjectParams[iCurrentKernel]);

            pState->HintIndexes.iKernelIndexes[iCurrentKernel]++;
            bEnableThreadSpace = false;
            bKernelFound = false;
            bUpdateCurrKernel = false;
            numDispatched++;
        } // for totalNumThreads
    } // else refCount <= 1


    // setup global surfaces
    for( j = 0; j < iNumKernels; ++j )
    {
        for( i = 0; i < CM_MAX_GLOBAL_SURFACE_NUMBER; ++i ) 
        {
            if(( pKernelParams[j]->globalSurface[i] & CM_SURFACE_MASK) != CM_NULL_SURFACE)
            {
                CM_HAL_KERNEL_ARG_PARAM ArgParam;
                pArgParam = &ArgParam;

                ArgParam.Kind = CM_ARGUMENT_SURFACEBUFFER;
                ArgParam.iPayloadOffset = 0;
                ArgParam.iUnitCount = 1;
                ArgParam.iUnitSize = sizeof(uint32_t);
                ArgParam.bPerThread = false;
                ArgParam.pFirstValue = (uint8_t*)&pKernelParams[j]->globalSurface[i];

                CM_CHK_MOSSTATUS(HalCm_SetupBufferSurfaceState(
                    pState, pArgParam, &pIndexParams[j], pBindingTableEntries[j],
                    (int16_t)i, 0, nullptr));
            }
        }

        // set number of samplers
        pKrnAllocations[j]->Params.Sampler_Count = pIndexParams[j].dwSamplerIndexCount;
    }

    // check to make sure we did all threads for all kernels
    if (numTasks <= 1 || lastTask )
    {
        for( i = 0; i < iNumKernels; ++i )
        {
            if( pState->HintIndexes.iKernelIndexes[i] < pKernelParams[i]->iNumThreads )
            {
                CM_ERROR_ASSERT("Not all threads for all kernels were put into batch buffer");
                goto finish;
            }
        }
    }

    if ( lastTask )
    {
        MOS_ZeroMemory(&pState->HintIndexes.iKernelIndexes, sizeof(uint32_t) * CM_MAX_TASKS_EU_SATURATION);
        MOS_ZeroMemory(&pState->HintIndexes.iDispatchIndexes, sizeof(uint32_t) * CM_MAX_TASKS_EU_SATURATION); 
    }

finish:
    // free memory
    if( pMediaObjectParams )            MOS_FreeMemory(pMediaObjectParams);
    if( pKernelParams )                 MOS_FreeMemory(pKernelParams);
    if( pArgParams )                    MOS_FreeMemory(pArgParams);
    if( pCmd_sizes )                    MOS_FreeMemory(pCmd_sizes);
    if( pRemapKrnToGrp )                MOS_FreeMemory(pRemapKrnToGrp);
    if( pRemapGrpToKrn )                MOS_FreeMemory(pRemapGrpToKrn);
    if( pKernelScoreboardMask )         MOS_FreeMemory(pKernelScoreboardMask);
    if( pParallelGraphInfo )            MOS_FreeMemory(pParallelGraphInfo);
    if( pNumKernelsPerGrp )             MOS_FreeMemory(pNumKernelsPerGrp);
    if( pGroupInfo )                    MOS_FreeMemory(pGroupInfo);

    if( pCmd_inline )
    {
        for( i = 0; i < iNumKernels; ++i )
        {
            if( pCmd_inline[i] )
                MOS_FreeMemory(pCmd_inline[i]);
        }
       MOS_FreeMemory(pCmd_inline);
    }

    if( pKernelsSliceInfo )
    {
        for( i = 0; i < numKernelGroups; ++i )
        {
            if( pKernelsSliceInfo[i].pDestination )
                MOS_FreeMemory(pKernelsSliceInfo[i].pDestination);
        }
        MOS_FreeMemory(pKernelsSliceInfo);
    }

    if( pDependRemap )
    {
        for( i = 0; i < iNumKernels; ++i )
        {
            if( pDependRemap[i] )
                MOS_FreeMemory(pDependRemap[i]);
        }
        MOS_FreeMemory(pDependRemap);
    }

    if( pDispatchFreq )
    {
        for( i = 0; i < iNumKernels; ++i )
        {
            if( pDispatchFreq[i] )
                MOS_FreeMemory(pDispatchFreq[i]);
        }
        MOS_FreeMemory(pDispatchFreq);
    }

    return hr;
}

uint32_t HalCm_ThreadsNumberPerGroup_MW(PCM_HAL_WALKER_PARAMS pWalkerParams)
{
    int local_inner_count = 0, local_mid_count = 0, local_outer_count = 0, global_inner_count = 0, global_outer_count = 0;
    int local_inner_count_max = 0, local_mid_count_max = 0, local_outer_count_max = 0, global_inner_count_max = 0;
    int global_inner_x = 0, global_inner_y = 0;
    int global_inner_x_copy = 0, global_inner_y_copy = 0;
    int mid_x = 0, mid_y = 0, mid_step = 0;
    int outer_x = 0, outer_y = 0;
    int local_inner_x = 0, local_inner_y = 0;
    int block_size_x = 0, block_size_y = 0;
    //int x, y;

    int local_loop_exec_count = pWalkerParams->dwLocalLoopExecCount; 
    int global_loop_exec_count = pWalkerParams->dwGlobalLoopExecCount;
    int globalresX = pWalkerParams->GlobalResolution.x, globalresY = pWalkerParams->GlobalResolution.y;
    int global_outer_x = pWalkerParams->GlobalStart.x, global_outer_y = pWalkerParams->GlobalStart.y;
    int global_outer_stepx = pWalkerParams->GlobalOutlerLoopStride.x, global_outer_stepy = pWalkerParams->GlobalOutlerLoopStride.y;
    int global_inner_stepx = pWalkerParams->GlobalInnerLoopUnit.x, global_inner_stepy = pWalkerParams->GlobalInnerLoopUnit.y;
    int middle_stepx = pWalkerParams->MidLoopUnitX, middle_stepy = pWalkerParams->MidLoopUnitY, extrasteps = pWalkerParams->MiddleLoopExtraSteps;
    int localblockresX = pWalkerParams->BlockResolution.x, localblockresY = pWalkerParams->BlockResolution.y;
    int localStartX = pWalkerParams->LocalStart.x, localStartY = pWalkerParams->LocalStart.y;
    int local_outer_stepx = pWalkerParams->LocalOutLoopStride.x, local_outer_stepy = pWalkerParams->LocalOutLoopStride.y;
    int local_inner_stepx = pWalkerParams->LocalInnerLoopUnit.x, local_inner_stepy = pWalkerParams->LocalInnerLoopUnit.y;

    uint32_t threads_number_pergroup = 0;

    //do global_outer_looper initialization
    while (((global_outer_x >= globalresX) && (global_inner_stepx < 0)) ||
        ((global_outer_x + localblockresX) < 0) && (global_inner_stepx > 0) ||
        ((global_outer_y >= globalresY) && (global_inner_stepy < 0)) ||
        ((global_outer_x + localblockresY) < 0) && (global_inner_stepy > 0))
    {
        global_outer_x += global_inner_stepx;
        global_outer_y += global_inner_stepy;
    }

    //global_ouer_loop_in_bounds()
    while ((global_outer_x < globalresX) &&
        (global_outer_y < globalresY) &&
        (global_outer_x + localblockresX > 0) &&
        (global_outer_y + localblockresY > 0) &&
        (global_outer_count <= global_loop_exec_count))
    {
        global_inner_x = global_outer_x;
        global_inner_y = global_outer_y;

        if (global_inner_count_max < global_inner_count)
        {
            global_inner_count_max = global_inner_count;
        }
        global_inner_count = 0;

        //global_inner_loop_in_bounds()
        while ((global_inner_x < globalresX) &&
            (global_inner_y < globalresY) &&
            (global_inner_x + localblockresX > 0) &&
            (global_inner_y + localblockresY > 0))
        {
            global_inner_x_copy = global_inner_x;
            global_inner_y_copy = global_inner_y;
            if (global_inner_x < 0)
                global_inner_x_copy = 0;
            if (global_inner_y < 0)
                global_inner_y_copy = 0;

            if (global_inner_x < 0)
                block_size_x = localblockresX + global_inner_x;
            else if ((globalresX - global_inner_x) < localblockresX)
                block_size_x = globalresX - global_inner_x;
            else
                block_size_x = localblockresX;
            if (global_inner_y < 0)
                block_size_y = localblockresY + global_inner_y;
            else if ((globalresY - global_inner_y) < localblockresY)
                block_size_y = globalresY - global_inner_y;
            else
                block_size_y = localblockresY;

            outer_x = localStartX;
            outer_y = localStartY;
            
            if (local_outer_count_max < local_outer_count)
            {
                local_outer_count_max = local_outer_count;
            }
            local_outer_count = 0;

            while ((outer_x >= block_size_x && local_inner_stepx < 0) ||
                (outer_x < 0 && local_inner_stepx > 0) ||
                (outer_y >= block_size_y && local_inner_stepy < 0) ||
                (outer_y < 0 && local_inner_stepy > 0))
            {
                outer_x += local_inner_stepx;
                outer_y += local_inner_stepy;
            }

            //local_outer_loop_in_bounds()
            while ((outer_x < block_size_x) &&
                (outer_y < block_size_y) &&
                (outer_x >= 0) &&
                (outer_y >= 0) &&
                (local_outer_count <= local_loop_exec_count))
            {
                mid_x = outer_x;
                mid_y = outer_y;
                mid_step = 0;

                if (local_mid_count_max < local_mid_count)
                {
                    local_mid_count_max = local_mid_count;
                }
                local_mid_count = 0;

                //local_middle_steps_remaining()
                while ((mid_step <= extrasteps) &&
                    (mid_x < block_size_x) &&
                    (mid_y < block_size_y) &&
                    (mid_x >= 0) &&
                    (mid_y >= 0))
                {
                    local_inner_x = mid_x;
                    local_inner_y = mid_y;

                    if (local_inner_count_max < local_inner_count)
                    {
                        local_inner_count_max = local_inner_count;
                    }
                    local_inner_count = 0;

                    //local_inner_loop_shrinking()
                    while ((local_inner_x < block_size_x) &&
                        (local_inner_y < block_size_y) &&
                        (local_inner_x >= 0) &&
                        (local_inner_y >= 0))
                    {
                        //x = local_inner_x + global_inner_x_copy;
                        //y = local_inner_y + global_inner_y_copy;
                        local_inner_count ++;

                        local_inner_x += local_inner_stepx;
                        local_inner_y += local_inner_stepy;
                    }
                    local_mid_count++;
                    mid_step++;
                    mid_x += middle_stepx;
                    mid_y += middle_stepy;
                }
                local_outer_count += 1;
                outer_x += local_outer_stepx;
                outer_y += local_outer_stepy;
                while ((outer_x >= block_size_x && local_inner_stepx < 0) ||
                    (outer_x <0 && local_inner_stepx > 0) ||
                    (outer_y >= block_size_y && local_inner_stepy < 0) ||
                    (outer_y <0 && local_inner_stepy > 0))
                {
                    outer_x += local_inner_stepx;
                    outer_y += local_inner_stepy;
                }
            }
            global_inner_count++;
            global_inner_x += global_inner_stepx;
            global_inner_y += global_inner_stepy;
        }
        global_outer_count += 1;
        global_outer_x += global_outer_stepx;
        global_outer_y += global_outer_stepy;
        while (((global_outer_x >= globalresX) && (global_inner_stepx < 0)) ||
            ((global_outer_x + localblockresX) < 0) && (global_inner_stepx > 0) ||
            ((global_outer_y >= globalresY) && (global_inner_stepy < 0)) ||
            ((global_outer_x + localblockresY) < 0) && (global_inner_stepy > 0))
        {
            global_outer_x += global_inner_stepx;
            global_outer_y += global_inner_stepy;
        }
    }

    switch (pWalkerParams->GroupIdLoopSelect)
    {
        case CM_MW_GROUP_COLORLOOP:
            threads_number_pergroup = pWalkerParams->ColorCountMinusOne + 1;
            break;
        case CM_MW_GROUP_INNERLOCAL:
            threads_number_pergroup = local_inner_count * (pWalkerParams->ColorCountMinusOne + 1);
            break;
        case CM_MW_GROUP_MIDLOCAL:
            threads_number_pergroup = local_mid_count * local_inner_count * (pWalkerParams->ColorCountMinusOne + 1);
            break;
        case CM_MW_GROUP_OUTERLOCAL:
            threads_number_pergroup = local_outer_count * local_mid_count * local_inner_count * (pWalkerParams->ColorCountMinusOne + 1);
            break;
        case CM_MW_GROUP_INNERGLOBAL:
            threads_number_pergroup = global_inner_count * local_outer_count * local_mid_count * local_inner_count * (pWalkerParams->ColorCountMinusOne + 1);
            break;
        default:
            threads_number_pergroup = global_outer_count * global_inner_count * local_outer_count * local_mid_count * local_inner_count * (pWalkerParams->ColorCountMinusOne + 1);
            break;
    }

    return threads_number_pergroup;
}

MOS_STATUS HalCm_SetupMediaWalkerParams(
    PCM_HAL_STATE                 pState,
    PCM_HAL_KERNEL_PARAM          pKernelParam)
{
    MOS_STATUS                      hr = MOS_STATUS_SUCCESS;
    PCM_HAL_TASK_PARAM              pTaskParam = pState->pTaskParam;
    PCM_HAL_WALKER_PARAMS           pWalkerParams = &pKernelParam->WalkerParams;

    //Using global walker enable flag
    pWalkerParams->CmWalkerEnable = pState->WalkerParams.CmWalkerEnable;
    if (pWalkerParams->CmWalkerEnable)
    {
        // MEDIA_WALKER
        CM_HAL_KERNEL_THREADSPACE_PARAM kernelThreadSpace;
        if (pKernelParam->KernelThreadSpaceParam.iThreadSpaceWidth)
        {
            kernelThreadSpace.iThreadSpaceWidth = pKernelParam->KernelThreadSpaceParam.iThreadSpaceWidth;
            kernelThreadSpace.iThreadSpaceHeight = pKernelParam->KernelThreadSpaceParam.iThreadSpaceHeight;
            kernelThreadSpace.patternType = pKernelParam->KernelThreadSpaceParam.patternType;
            kernelThreadSpace.walkingPattern = pKernelParam->KernelThreadSpaceParam.walkingPattern;
            kernelThreadSpace.groupSelect = pKernelParam->KernelThreadSpaceParam.groupSelect;
            kernelThreadSpace.colorCountMinusOne = pKernelParam->KernelThreadSpaceParam.colorCountMinusOne;
        }
        else
        {
            kernelThreadSpace.iThreadSpaceWidth = (uint16_t)pTaskParam->threadSpaceWidth;
            kernelThreadSpace.iThreadSpaceHeight = (uint16_t)pTaskParam->threadSpaceHeight;
            kernelThreadSpace.patternType = pTaskParam->DependencyPattern;
            kernelThreadSpace.walkingPattern = pTaskParam->WalkingPattern;
            kernelThreadSpace.groupSelect = pTaskParam->MediaWalkerGroupSelect;
            kernelThreadSpace.colorCountMinusOne = pTaskParam->ColorCountMinusOne;
        }

        // check for valid thread space width and height here since different from media object
        if (kernelThreadSpace.iThreadSpaceWidth > pState->pCmHalInterface->GetMediaWalkerMaxThreadWidth())
        {
            CM_ASSERTMESSAGE("Error: Exceeds the maximum thread space width.");
            hr = MOS_STATUS_INVALID_PARAMETER;
            goto finish;
        }
        if (kernelThreadSpace.iThreadSpaceHeight > pState->pCmHalInterface->GetMediaWalkerMaxThreadHeight())
        {
            CM_ASSERTMESSAGE("Error: Exceeds the maximum thread space height.");
            hr = MOS_STATUS_INVALID_PARAMETER;
            goto finish;
        }

        //pWalkerParams->InterfaceDescriptorOffset = iMediaID;// MediaObjectParam.dwInterfaceDescriptorOffset;
        pWalkerParams->InlineDataLength = MOS_ALIGN_CEIL(pKernelParam->IndirectDataParam.iIndirectDataSize, 4);
        pWalkerParams->pInlineData = pKernelParam->IndirectDataParam.pIndirectData;

        pWalkerParams->ColorCountMinusOne = kernelThreadSpace.colorCountMinusOne;// pTaskParam->ColorCountMinusOne;
        pWalkerParams->GroupIdLoopSelect = (uint32_t)kernelThreadSpace.groupSelect;

        CM_WALKING_PATTERN walkPattern = kernelThreadSpace.walkingPattern;
        switch (kernelThreadSpace.patternType)
        {
            case CM_NONE_DEPENDENCY:
                break;
            case CM_HORIZONTAL_WAVE:
                walkPattern = CM_WALK_HORIZONTAL;
                break;
            case CM_VERTICAL_WAVE:
                walkPattern = CM_WALK_VERTICAL;
                break;
            case CM_WAVEFRONT:
                walkPattern = CM_WALK_WAVEFRONT;
                break;
            case CM_WAVEFRONT26:
                walkPattern = CM_WALK_WAVEFRONT26;
                break;
            case CM_WAVEFRONT26X:
                if (kernelThreadSpace.iThreadSpaceWidth > 1)
                {
                    walkPattern = CM_WALK_WAVEFRONT26X;
                }
                else
                {
                    walkPattern = CM_WALK_DEFAULT;
                }
                break;
            case CM_WAVEFRONT26ZIG:
                if (kernelThreadSpace.iThreadSpaceWidth > 2)
                {
                    walkPattern = CM_WALK_WAVEFRONT26ZIG;
                }
                else
                {
                    walkPattern = CM_WALK_DEFAULT;
                }
                break;
            default:
                CM_ASSERTMESSAGE("Error: Invalid walking pattern.");
                walkPattern = CM_WALK_DEFAULT;
                break;
        }
        if (pTaskParam->walkingParamsValid)
        {
            CM_CHK_MOSSTATUS(pState->pCmHalInterface->SetMediaWalkerParams
                (pTaskParam->walkingParams, pWalkerParams));

            if (walkPattern == CM_WALK_HORIZONTAL || walkPattern == CM_WALK_DEFAULT)
            {
                pWalkerParams->LocalEnd.x = pWalkerParams->BlockResolution.x - 1;
            }
            else if (walkPattern == CM_WALK_VERTICAL)
            {
                pWalkerParams->LocalEnd.y = pWalkerParams->BlockResolution.y - 1;
            }
        }
        else if (pKernelParam->KernelThreadSpaceParam.walkingParamsValid)
        {
            CM_CHK_MOSSTATUS(pState->pCmHalInterface->SetMediaWalkerParams(
                pKernelParam->KernelThreadSpaceParam.walkingParams, pWalkerParams));

            if (walkPattern == CM_WALK_HORIZONTAL || walkPattern == CM_WALK_DEFAULT)
            {
                pWalkerParams->LocalEnd.x = pWalkerParams->BlockResolution.x - 1;
            }
            else if (walkPattern == CM_WALK_VERTICAL)
            {
                pWalkerParams->LocalEnd.y = pWalkerParams->BlockResolution.y - 1;
            }

        }
        else
        {
            //Local loop parameters
            pWalkerParams->BlockResolution.x = kernelThreadSpace.iThreadSpaceWidth;
            pWalkerParams->BlockResolution.y = kernelThreadSpace.iThreadSpaceHeight;

            pWalkerParams->LocalStart.x = 0;
            pWalkerParams->LocalStart.y = 0;
            pWalkerParams->LocalEnd.x = 0;
            pWalkerParams->LocalEnd.y = 0;

            pWalkerParams->dwGlobalLoopExecCount = 1;
            pWalkerParams->MidLoopUnitX = 0;
            pWalkerParams->MidLoopUnitY = 0;
            pWalkerParams->MiddleLoopExtraSteps = 0;

            // account for odd Height/Width for 26x and 26Zig
            uint16_t adjHeight = ((kernelThreadSpace.iThreadSpaceHeight + 1) >> 1) << 1;
            uint16_t adjWidth = ((kernelThreadSpace.iThreadSpaceWidth + 1) >> 1) << 1;

            uint32_t maxThreadWidth = pState->pCmHalInterface->GetMediaWalkerMaxThreadWidth();
            switch (walkPattern)
            {
                case CM_WALK_DEFAULT:
                case CM_WALK_HORIZONTAL:
                    if (kernelThreadSpace.iThreadSpaceWidth == pKernelParam->iNumThreads &&
                        kernelThreadSpace.iThreadSpaceHeight == 1)
                    {
                        pWalkerParams->BlockResolution.x = MOS_MIN(pKernelParam->iNumThreads, maxThreadWidth);
                        pWalkerParams->BlockResolution.y = 1 + pKernelParam->iNumThreads / maxThreadWidth;
                    }
                    pWalkerParams->dwLocalLoopExecCount = pWalkerParams->BlockResolution.y - 1;

                    pWalkerParams->LocalOutLoopStride.x = 0;
                    pWalkerParams->LocalOutLoopStride.y = 1;
                    pWalkerParams->LocalInnerLoopUnit.x = 1;
                    pWalkerParams->LocalInnerLoopUnit.y = 0;

                    pWalkerParams->LocalEnd.x = pWalkerParams->BlockResolution.x - 1;

                    break;

                case CM_WALK_WAVEFRONT:
                    pWalkerParams->dwLocalLoopExecCount = kernelThreadSpace.iThreadSpaceWidth + (kernelThreadSpace.iThreadSpaceHeight - 1) * 1 - 1;

                    pWalkerParams->LocalOutLoopStride.x = 1;
                    pWalkerParams->LocalOutLoopStride.y = 0;
                    pWalkerParams->LocalInnerLoopUnit.x = 0xFFFF;  // -1 in uint32_t:16
                    pWalkerParams->LocalInnerLoopUnit.y = 1;
                    break;

                case CM_WALK_WAVEFRONT26:
                    pWalkerParams->dwLocalLoopExecCount = kernelThreadSpace.iThreadSpaceWidth + (kernelThreadSpace.iThreadSpaceHeight - 1) * 2 - 1;

                    pWalkerParams->LocalOutLoopStride.x = 1;
                    pWalkerParams->LocalOutLoopStride.y = 0;
                    pWalkerParams->LocalInnerLoopUnit.x = 0xFFFE;  // -2 in uint32_t:16
                    pWalkerParams->LocalInnerLoopUnit.y = 1;
                    break;

                case CM_WALK_WAVEFRONT26X:
                    pWalkerParams->dwLocalLoopExecCount = adjWidth - 1 + (adjHeight - 4) / 2;
                    pWalkerParams->dwGlobalLoopExecCount = 0;

                    pWalkerParams->LocalOutLoopStride.x = 1;
                    pWalkerParams->LocalOutLoopStride.y = 0;
                    pWalkerParams->LocalInnerLoopUnit.x = 0xFFFE;
                    pWalkerParams->LocalInnerLoopUnit.y = 4;

                    pWalkerParams->MiddleLoopExtraSteps = 3;
                    pWalkerParams->MidLoopUnitX = 0;
                    pWalkerParams->MidLoopUnitY = 1;
                    break;

                case CM_WALK_WAVEFRONT26ZIG:
                    pWalkerParams->dwLocalLoopExecCount = 1;
                    pWalkerParams->dwGlobalLoopExecCount = (adjHeight / 2 - 1) * 2 + (adjWidth / 2) - 1;

                    pWalkerParams->LocalOutLoopStride.x = 0;
                    pWalkerParams->LocalOutLoopStride.y = 1;
                    pWalkerParams->LocalInnerLoopUnit.x = 1;
                    pWalkerParams->LocalInnerLoopUnit.y = 0;

                    pWalkerParams->BlockResolution.x = 2;
                    pWalkerParams->BlockResolution.y = 2;

                    pWalkerParams->LocalEnd.x = pWalkerParams->BlockResolution.x - 1;
                    break;

                case CM_WALK_VERTICAL:
                    pWalkerParams->dwLocalLoopExecCount = pWalkerParams->BlockResolution.x - 1;

                    pWalkerParams->LocalOutLoopStride.x = 1;
                    pWalkerParams->LocalOutLoopStride.y = 0;
                    pWalkerParams->LocalInnerLoopUnit.x = 0;
                    pWalkerParams->LocalInnerLoopUnit.y = 1;

                    pWalkerParams->LocalEnd.y = pWalkerParams->BlockResolution.y - 1;

                    break;

                case CM_WALK_WAVEFRONT45D:
                    pWalkerParams->dwLocalLoopExecCount = 0x7ff;
                    pWalkerParams->dwGlobalLoopExecCount = 0x7ff;

                    pWalkerParams->LocalStart.x = kernelThreadSpace.iThreadSpaceWidth;
                    pWalkerParams->LocalOutLoopStride.x = 1;
                    pWalkerParams->LocalOutLoopStride.y = 0;
                    pWalkerParams->LocalInnerLoopUnit.x = 0xFFFF;  // -1 in uint32_t:16
                    pWalkerParams->LocalInnerLoopUnit.y = 1;
                    break;
  
                case CM_WALK_WAVEFRONT45XD_2:
                    pWalkerParams->dwLocalLoopExecCount = 0x7ff;
                    pWalkerParams->dwGlobalLoopExecCount = 0x7ff;

                    // Local
                    pWalkerParams->LocalStart.x = kernelThreadSpace.iThreadSpaceWidth;
                    pWalkerParams->LocalOutLoopStride.x = 1;
                    pWalkerParams->LocalOutLoopStride.y = 0;
                    pWalkerParams->LocalInnerLoopUnit.x = 0xFFFF;  // -1 in uint32_t:16
                    pWalkerParams->LocalInnerLoopUnit.y = 2;

                    // Mid
                    pWalkerParams->MiddleLoopExtraSteps = 1;
                    pWalkerParams->MidLoopUnitX = 0;
                    pWalkerParams->MidLoopUnitY = 1;

                    break;

                default:
                    pWalkerParams->dwLocalLoopExecCount = MOS_MIN(pKernelParam->iNumThreads, 0x3FF);

                    pWalkerParams->LocalOutLoopStride.x = 0;
                    pWalkerParams->LocalOutLoopStride.y = 1;
                    pWalkerParams->LocalInnerLoopUnit.x = 1;
                    pWalkerParams->LocalInnerLoopUnit.y = 0;
                    break;
            }

            //Global loop parameters: execution count, resolution and strides
            //Since no global loop, global resolution equals block resolution.
            pWalkerParams->GlobalStart.x = 0;
            pWalkerParams->GlobalStart.y = 0;
            pWalkerParams->GlobalOutlerLoopStride.y = 0;

            if (walkPattern == CM_WALK_WAVEFRONT26ZIG)
            {
                pWalkerParams->GlobalResolution.x = kernelThreadSpace.iThreadSpaceWidth;
                pWalkerParams->GlobalResolution.y = kernelThreadSpace.iThreadSpaceHeight;
                pWalkerParams->GlobalOutlerLoopStride.x = 2;
                pWalkerParams->GlobalInnerLoopUnit.x = 0xFFFC;
                pWalkerParams->GlobalInnerLoopUnit.y = 2;
            }
            else
            {
                pWalkerParams->GlobalResolution.x = pWalkerParams->BlockResolution.x;
                pWalkerParams->GlobalResolution.y = pWalkerParams->BlockResolution.y;
                pWalkerParams->GlobalOutlerLoopStride.x = pWalkerParams->GlobalResolution.x;
                pWalkerParams->GlobalInnerLoopUnit.x = 0;
                pWalkerParams->GlobalInnerLoopUnit.y = pWalkerParams->GlobalResolution.y;
            }
        }

        //Need calculate number threads per group for media walker, the minimum value is 1
        if (kernelThreadSpace.groupSelect > CM_MW_GROUP_NONE)   
        {
            pKernelParam->iNumberThreadsInGroup = HalCm_ThreadsNumberPerGroup_MW(pWalkerParams);
        }
        else
        {
            pKernelParam->iNumberThreadsInGroup = 1;
        }
    }

finish:
    return hr;
}

MOS_STATUS HalCm_AcquireSamplerStatistics(PCM_HAL_STATE pState)
{
    MOS_STATUS       hr = MOS_STATUS_SUCCESS;
    uint32_t i = 0;

    unsigned int max_BTIindex[MAX_ELEMENT_TYPE_COUNT] = {0}; //tempoary variable, it will hold the max BTI index in each element type

    /* enumerate through the samplerTable for the one in use, then count and analyze */
    for (i = 0; i < pState->CmDeviceParam.iMaxSamplerTableSize; i++) {  //pState->CmDeviceParam.iMaxSamplerTableSize;

        if (pState->pSamplerTable[i].bInUse) {
            uint32_t iSamplerIndex = pState->pSamplerIndexTable[i];
            if (iSamplerIndex != CM_INVALID_INDEX) {
                MHW_SAMPLER_ELEMENT_TYPE ElementType = pState->pSamplerTable[i].ElementType;
                max_BTIindex[ElementType] = (max_BTIindex[ElementType] > iSamplerIndex) ? max_BTIindex[ElementType] : iSamplerIndex;
            }
            else
                pState->SamplerStatistics.SamplerCount[pState->pSamplerTable[i].ElementType]++;
        }
    
    }

    int tempbase=0;
    pState->SamplerStatistics.Sampler_indexBase[MHW_Sampler2Elements] = (pState->SamplerStatistics.SamplerCount[MHW_Sampler2Elements]) ? 0 : -1;
    tempbase = pState->SamplerStatistics.Sampler_indexBase[MHW_Sampler2Elements];
    pState->SamplerStatistics.Sampler_indexBase[MHW_Sampler4Elements] = ((pState->SamplerStatistics.SamplerCount[MHW_Sampler4Elements]) ? ((tempbase == -1) ? 0 : INDEX_ALIGN(pState->SamplerStatistics.SamplerCount[MHW_Sampler2Elements], 2, 4)) : tempbase);
    tempbase = pState->SamplerStatistics.Sampler_indexBase[MHW_Sampler4Elements];
    pState->SamplerStatistics.Sampler_indexBase[MHW_Sampler8Elements] = ((pState->SamplerStatistics.SamplerCount[MHW_Sampler8Elements]) ? ((tempbase == -1) ? 0 : INDEX_ALIGN(pState->SamplerStatistics.SamplerCount[MHW_Sampler4Elements], 4, 8)) : tempbase);
    tempbase = pState->SamplerStatistics.Sampler_indexBase[MHW_Sampler8Elements];
    pState->SamplerStatistics.Sampler_indexBase[MHW_Sampler64Elements] = ((pState->SamplerStatistics.SamplerCount[MHW_Sampler64Elements]) ? ((tempbase == -1) ? 0 : INDEX_ALIGN(pState->SamplerStatistics.SamplerCount[MHW_Sampler8Elements], 8, 64)) : tempbase);
    tempbase = pState->SamplerStatistics.Sampler_indexBase[MHW_Sampler64Elements];
    pState->SamplerStatistics.Sampler_indexBase[MHW_Sampler128Elements] = ((pState->SamplerStatistics.SamplerCount[MHW_Sampler128Elements]) ? ((tempbase == -1) ? 0 : INDEX_ALIGN(pState->SamplerStatistics.SamplerCount[MHW_Sampler64Elements], 64, 128)) : tempbase);

        
    /* There are Sampler BTI, next step needs to consider it during calculate the base */
    for (int k = MHW_Sampler2Elements; k < MHW_Sampler128Elements; k++) {
        if (pState->SamplerStatistics.Sampler_indexBase[k + 1] < max_BTIindex[k])
            pState->SamplerStatistics.Sampler_indexBase[k + 1] = max_BTIindex[k];
    }
    return hr;
}


//*-----------------------------------------------------------------------------
//| Purpose:  Initial setup of HW states for the kernel
//| Returns:  Result of the operation
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_SetupStatesForKernelInitial(
    PCM_HAL_STATE                 pState,
    PRENDERHAL_MEDIA_STATE        pMediaState,
    PMHW_BATCH_BUFFER             pBatchBuffer,
    int32_t                       iTaskId,
    PCM_HAL_KERNEL_PARAM          pKernelParam,
    PCM_HAL_INDEX_PARAM           pIndexParam,
    uint32_t                      iKernelCurbeOffset,
    int32_t&                          iBindingTable,
    int32_t&                          iMediaID,
    PRENDERHAL_KRN_ALLOCATION    &pKrnAllocation)
{
    MOS_STATUS                      hr = MOS_STATUS_SUCCESS;
    PRENDERHAL_INTERFACE            pRenderHal = pState->pRenderHal;
    PRENDERHAL_STATE_HEAP           pStateHeap = pRenderHal->pStateHeap;
    PCM_INDIRECT_SURFACE_INFO       pIndirectSurfaceInfo = pKernelParam->IndirectDataParam.pSurfaceInfo;
    PCM_GPGPU_WALKER_PARAMS         pPerKernelGpGpuWalkerParames = &pKernelParam->GpGpuWalkerParams;
    UNUSED(pBatchBuffer);
    UNUSED(iTaskId);

    MHW_MEDIA_OBJECT_PARAMS         MediaObjectParam;
    PCM_HAL_KERNEL_ARG_PARAM        pArgParam;
    uint32_t                        iHdrSize;
    uint32_t                        index;
    uint32_t                        value;
    uint32_t                        btIndex;
    uint32_t                        surfIndex;
    uint32_t                        aIndex;
    uint32_t                        id_z;
    uint32_t                        id_y;
    uint32_t                        id_x;
    uint32_t                        localId_index;
    CM_SURFACE_BTI_INFO             SurfBTIInfo;

    bool                            vme_used = false;
    CM_PLATFORM_INFO                platformInfo;

    localId_index = pKernelParam->localId_index;

    pState->pCmHalInterface->GetHwSurfaceBTIInfo(&SurfBTIInfo);

    HalCm_PreSetBindingIndex(pIndexParam, CM_NULL_SURFACE_BINDING_INDEX, CM_NULL_SURFACE_BINDING_INDEX);

    HalCm_PreSetBindingIndex(pIndexParam, SurfBTIInfo.dwReservedSurfaceStart,
        SurfBTIInfo.dwReservedSurfaceStart + CM_MAX_GLOBAL_SURFACE_NUMBER - 1);

    if (pKernelParam->IndirectDataParam.iSurfaceCount)
    {
        for (index = 0; index < pKernelParam->IndirectDataParam.iSurfaceCount; index++)
        {
            value = (pIndirectSurfaceInfo + index)->iBindingTableIndex;
            HalCm_PreSetBindingIndex(pIndexParam, value, value);
        }
    }

    // Get the binding table for this kernel
    CM_CHK_MOSSTATUS(pRenderHal->pfnAssignBindingTable(pRenderHal, &iBindingTable));

    if (pState->bDynamicStateHeap)
    {
        // Kernels are already pre-loaded in GSH
        // pKrnAllocation is the head of a linked list
        if (!pKrnAllocation)
        {
            CM_ASSERTMESSAGE("Error: Invalid kernel allocation.");
            goto finish;
        }
    }
    else
    {
        // Load the Kernel to GSH
        CM_CHK_MOSSTATUS(HalCm_LoadKernel(
            pState,
            pKernelParam,
            0,
            pKrnAllocation));
    }

    // initialize curbe buffer
    if (pKernelParam->iKrnCurbeSize > 0)
    {
        // Update Curbe offset after curbe load command
        if (pState->bDynamicStateHeap)
        {
            pMediaState->pDynamicState->Curbe.iCurrent += MOS_ALIGN_CEIL(pKernelParam->iKrnCurbeSize, pState->pRenderHal->dwCurbeBlockAlign);
        }
        else
        {
            pMediaState->iCurbeOffset += MOS_ALIGN_CEIL(pKernelParam->iKrnCurbeSize, pState->pRenderHal->dwCurbeBlockAlign);
        }
    }

    //Setup  media walker parameters if it is
    CM_CHK_MOSSTATUS(HalCm_SetupMediaWalkerParams(pState, pKernelParam));

    // Allocate Interface Descriptor
    iMediaID = HalCm_AllocateMediaID(
        pState,
        pKernelParam,
        pKrnAllocation,
        iBindingTable,
        iKernelCurbeOffset);

    if (iMediaID < 0)
    {
        CM_ERROR_ASSERT("Unable to get Media ID");
        goto finish;
    }

    // Setup the Media object
    iHdrSize = pRenderHal->pHwSizes->dwSizeMediaObjectHeaderCmd;
    MediaObjectParam.dwInterfaceDescriptorOffset = iMediaID;
    if (pKernelParam->IndirectDataParam.iIndirectDataSize)
    {
        MediaObjectParam.dwInlineDataSize = 0;
    }
    else
    {
        MediaObjectParam.dwInlineDataSize = MOS_MAX(pKernelParam->iPayloadSize, 4);
    }

    // set surface state and binding table
    if (pKernelParam->IndirectDataParam.iSurfaceCount)
    {
        for (index = 0; index < pKernelParam->IndirectDataParam.iSurfaceCount; index++)
        {
            btIndex = (pIndirectSurfaceInfo + index)->iBindingTableIndex;
            surfIndex = (pIndirectSurfaceInfo + index)->iSurfaceIndex;
            switch ((pIndirectSurfaceInfo + index)->iKind)
            {
            case CM_ARGUMENT_SURFACEBUFFER:
                CM_CHK_MOSSTATUS(HalCm_SetupBufferSurfaceStateWithBTIndex(
                    pState, iBindingTable, surfIndex, btIndex, 0));
                break;

            case CM_ARGUMENT_SURFACE2D:
                CM_CHK_MOSSTATUS(HalCm_Setup2DSurfaceStateWithBTIndex(
                    pState, iBindingTable, surfIndex, btIndex, 0));
                break;

            case CM_ARGUMENT_SURFACE2D_UP:
                CM_CHK_MOSSTATUS(HalCm_Setup2DSurfaceUPStateWithBTIndex(
                    pState, iBindingTable, surfIndex, btIndex, 0));
                break;

            case CM_ARGUMENT_SURFACE2D_SAMPLER:
                CM_CHK_MOSSTATUS(HalCm_Setup2DSurfaceStateWithBTIndex(
                    pState, iBindingTable, surfIndex, btIndex, 1));
                break;
            case CM_ARGUMENT_SURFACE2DUP_SAMPLER:
                CM_CHK_MOSSTATUS(HalCm_Setup2DSurfaceUPStateWithBTIndex(
                    pState, iBindingTable, surfIndex, btIndex, 1));
                break;
            case CM_ARGUMENT_SURFACE_SAMPLER8X8_AVS:
            case CM_ARGUMENT_SURFACE_SAMPLER8X8_VA:
                CM_CHK_MOSSTATUS(HalCm_SetupSampler8x8SurfaceStateWithBTIndex(
                    pState, iBindingTable, surfIndex, btIndex, 0, (CM_HAL_KERNEL_ARG_KIND)(pIndirectSurfaceInfo + index)->iKind, 0));
                break;
            case CM_ARGUMENT_SURFACE3D:
                CM_CHK_MOSSTATUS(HalCm_Setup3DSurfaceStateWithBTIndex(
                    pState, iBindingTable, surfIndex, btIndex));
                break;
            default:
                CM_ERROR_ASSERT(
                    "Indirect Data Surface kind is not supported");
                goto finish;
            }
        }
    }

    // set sampler bti
    if (pKernelParam->SamplerBTIParam.iSamplerCount > 0)
    {
        for (uint32_t i = 0; i < pKernelParam->SamplerBTIParam.iSamplerCount; i++)
        {
            HalCm_SetupSamplerStateWithBTIndex(pState, pKernelParam, &pKernelParam->SamplerBTIParam.SamplerInfo[0], i, iMediaID);
        }
    }

    if ( ( pKernelParam->iCurbeSizePerThread > 0 ) && ( pKernelParam->state_buffer_type == CM_STATE_BUFFER_NONE ) )
    {
        uint8_t data[CM_MAX_THREAD_PAYLOAD_SIZE + 32];
        uint8_t curbe[CM_MAX_CURBE_SIZE_PER_TASK + 32];

        MOS_ZeroMemory(data, sizeof(data));
        MOS_ZeroMemory(curbe, sizeof(curbe));
        for (aIndex = 0; aIndex < pKernelParam->iNumArgs; aIndex++)
        {
            pArgParam = &pKernelParam->CmArgParams[aIndex];

            if (pArgParam->bPerThread || pArgParam->bIsNull)
            {
                continue;
            }

            switch (pArgParam->Kind)
            {
            case CM_ARGUMENT_GENERAL:
            case CM_ARGUMENT_IMPLICT_GROUPSIZE:
            case CM_ARGUMENT_IMPLICT_LOCALSIZE:
            case CM_ARGUMENT_IMPLICIT_LOCALID:
            case CM_ARGUMENT_GENERAL_DEPVEC:
                HalCm_SetArgData(pArgParam, 0, data);
                break;

            case CM_ARGUMENT_SAMPLER:
                CM_CHK_MOSSTATUS(HalCm_SetupSamplerState(
                    pState, pKernelParam, pArgParam, pIndexParam, iMediaID, 0, data));
                break;

            case CM_ARGUMENT_SURFACEBUFFER:
                CM_CHK_MOSSTATUS(HalCm_SetupBufferSurfaceState(
                    pState, pArgParam, pIndexParam, iBindingTable, -1, 0, data));
                break;

            case CM_ARGUMENT_SURFACE2D_UP:
                CM_CHK_MOSSTATUS(HalCm_Setup2DSurfaceUPState(
                    pState, pArgParam, pIndexParam, iBindingTable, 0, data));
                break;

            case CM_ARGUMENT_SURFACE2DUP_SAMPLER:
                CM_CHK_MOSSTATUS(HalCm_Setup2DSurfaceUPSamplerState(
                    pState, pArgParam, pIndexParam, iBindingTable, 0, data));
                break;

            case CM_ARGUMENT_SURFACE2D_SAMPLER:
                CM_CHK_MOSSTATUS(HalCm_Setup2DSurfaceSamplerState(
                    pState, pArgParam, pIndexParam, iBindingTable, 0, data));
                break;

            case CM_ARGUMENT_SURFACE2D:
                CM_CHK_MOSSTATUS(HalCm_Setup2DSurfaceState(
                    pState, pArgParam, pIndexParam, iBindingTable, 0, data));
                break;

            case CM_ARGUMENT_SURFACE3D:
                CM_CHK_MOSSTATUS(HalCm_Setup3DSurfaceState(
                    pState, pArgParam, pIndexParam, iBindingTable, 0, data));
                break;

            case CM_ARGUMENT_SURFACE_VME:   // 3 surface indices
                CM_CHK_MOSSTATUS(HalCm_SetupVmeSurfaceState(
                    pState, pArgParam, pIndexParam, iBindingTable, 0, data));
                vme_used = true;
                break;

            case CM_ARGUMENT_SURFACE_SAMPLER8X8_AVS:   // sampler 8x8  surface
            case CM_ARGUMENT_SURFACE_SAMPLER8X8_VA:    // sampler 8x8  surface
                CM_CHK_MOSSTATUS(HalCm_SetupSampler8x8SurfaceState(
                    pState, pArgParam, pIndexParam, iBindingTable, 0, data));
                break;

            case CM_ARGUMENT_STATE_BUFFER:
                CM_CHK_MOSSTATUS( HalCm_SetupStateBufferSurfaceState(
                    pState, pArgParam, pIndexParam, iBindingTable, 0, data ) );
                break;

            case CM_ARGUMENT_SURFACE:
                // Allow null surface
                break;
            case CM_ARGUMENT_SURFACE2D_SCOREBOARD:
                CM_CHK_MOSSTATUS(HalCm_Setup2DSurfaceState(
                    pState, pArgParam, pIndexParam, iBindingTable, 0, data));
                break;

            default:
                CM_ERROR_ASSERT(
                    "Argument kind '%d' is not supported", pArgParam->Kind);
                goto finish;
            }
        }

        if (pPerKernelGpGpuWalkerParames->CmGpGpuEnable)
        {
            uint32_t offset = 0;

            uint32_t local_id_x_offset = pKernelParam->CmArgParams[localId_index].iPayloadOffset;
            uint32_t local_id_y_offset = local_id_x_offset + 4;
            uint32_t local_id_z_offset = local_id_x_offset + 8;

            //iKrnCurbeSize aligned when parsing task
            int32_t crossThreadSize = pKernelParam->iCrsThrdConstDataLn;

            //Cross thread constant data
            MOS_SecureMemcpy(curbe + offset, crossThreadSize, data, crossThreadSize);
            offset += crossThreadSize;

            //Per-thread data
            for (id_z = 0; id_z < pPerKernelGpGpuWalkerParames->ThreadDepth; id_z++)
            {
                for (id_y = 0; id_y < pPerKernelGpGpuWalkerParames->ThreadHeight; id_y++)
                {
                    for (id_x = 0; id_x < pPerKernelGpGpuWalkerParames->ThreadWidth; id_x++)
                    {
                        *((uint32_t *)(data + local_id_x_offset)) = id_x;
                        *((uint32_t *)(data + local_id_y_offset)) = id_y;
                        *((uint32_t *)(data + local_id_z_offset)) = id_z;
                        MOS_SecureMemcpy(curbe + offset, pKernelParam->iCurbeSizePerThread, data + crossThreadSize, pKernelParam->iCurbeSizePerThread);
                        offset += pKernelParam->iCurbeSizePerThread;
                    }
                }
            }

            // tell pfnLoadCurbeData the current curbe offset
            if (pState->bDynamicStateHeap)
            {
                PRENDERHAL_DYNAMIC_STATE pDynamicState = pStateHeap->pCurMediaState->pDynamicState;
                pDynamicState->Curbe.iCurrent -= MOS_ALIGN_CEIL(pKernelParam->iKrnCurbeSize, pState->pRenderHal->dwCurbeBlockAlign);
            }
            else
            {
                pStateHeap->pCurMediaState->iCurbeOffset -= MOS_ALIGN_CEIL(pKernelParam->iKrnCurbeSize, pState->pRenderHal->dwCurbeBlockAlign);
            }
            // update curbe with data.
            pRenderHal->pfnLoadCurbeData(pRenderHal,
                pStateHeap->pCurMediaState,
                curbe,
                pKernelParam->iKrnCurbeSize);
        }
        else
        {
            CM_ASSERT(pKernelParam->iKrnCurbeSize == pKernelParam->iCurbeSizePerThread);

            // tell pfnLoadCurbeData the current curbe offset
            if (pState->bDynamicStateHeap)
            {
                PRENDERHAL_DYNAMIC_STATE pDynamicState = pStateHeap->pCurMediaState->pDynamicState;
                pDynamicState->Curbe.iCurrent -= MOS_ALIGN_CEIL(pKernelParam->iKrnCurbeSize, pState->pRenderHal->dwCurbeBlockAlign);
            }
            else
            {
                pStateHeap->pCurMediaState->iCurbeOffset -= MOS_ALIGN_CEIL(pKernelParam->iKrnCurbeSize, pState->pRenderHal->dwCurbeBlockAlign);
            }
            // update curbe with data.
            pRenderHal->pfnLoadCurbeData(pRenderHal,
                pStateHeap->pCurMediaState,
                data,
                pKernelParam->iKrnCurbeSize);
        }

        if ((vme_used == true) && pState->pCmHalInterface->IsSliceShutdownEnabled())
        {
            CM_CHK_MOSSTATUS(pState->pfnGetPlatformInfo(pState, &platformInfo, true));
            CM_POWER_OPTION  CMPower;
            CMPower.nSlice = 1;
            CMPower.nSubSlice = platformInfo.numSubSlices / 2;
            CMPower.nEU = 0;
            pState->pfnSetPowerOption(pState, &CMPower);
        }
    }

#if MDF_CURBE_DATA_DUMP
    if (pState->bDumpCurbeData)
    {
        HalCm_DumpCurbeData(pState);
    }
#endif

finish:
    return hr;
}

MOS_STATUS HalCm_SetConditionalEndInfo(
    PCM_HAL_STATE pState,
    PCM_HAL_CONDITIONAL_BB_END_INFO pConditionalEndInfo,
    PMHW_MI_CONDITIONAL_BATCH_BUFFER_END_PARAMS pConditionalBBEndParams,
    uint32_t index
    )
{
    if (index >= CM_MAX_CONDITIONAL_END_CMDS)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    MOS_ZeroMemory(&pConditionalBBEndParams[index], sizeof(MHW_MI_CONDITIONAL_BATCH_BUFFER_END_PARAMS));

    pConditionalBBEndParams[index].presSemaphoreBuffer = &(pState->pBufferTable[pConditionalEndInfo[index].bufferTableIndex].OsResource);
    pConditionalBBEndParams[index].dwValue             = pConditionalEndInfo[index].compareValue;
    pConditionalBBEndParams[index].bDisableCompareMask = pConditionalEndInfo[index].bDisableCompareMask;
    pConditionalBBEndParams[index].dwOffset            = pConditionalEndInfo[index].offset;

    return MOS_STATUS_SUCCESS;
}

//===============<Interface Functions>==========================================

//*-----------------------------------------------------------------------------
//| Purpose: Allocate Structures required for HW Rendering
//| Returns: Result of the operation
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_Allocate(
    PCM_HAL_STATE pState)                                                       // [in] Pointer to CM State
{
    MOS_STATUS              hr;
    PCM_HAL_DEVICE_PARAM    pDeviceParam;
    PRENDERHAL_INTERFACE     pRenderHal;
    PRENDERHAL_STATE_HEAP_SETTINGS pStateHeapSettings;
    uint32_t                i;
    MOS_NULL_RENDERING_FLAGS NullHWAccelerationEnable;
    RENDERHAL_SETTINGS       RenderHalSettings;
    uint32_t                maxTasks;


    PMHW_BATCH_BUFFER        pBb = nullptr;

    //------------------------------------
    CM_ASSERT(pState);
    //------------------------------------

    hr              = MOS_STATUS_UNKNOWN;
    pDeviceParam    = &pState->CmDeviceParam;
    pRenderHal         = pState->pRenderHal;
    pStateHeapSettings = &pRenderHal->StateHeapSettings;

    pStateHeapSettings->iCurbeSize        = CM_MAX_CURBE_SIZE_PER_TASK;
    pStateHeapSettings->iMediaStateHeaps  = pDeviceParam->iMaxTasks + 1;              // + 1 to handle sync issues with current RenderHal impl (we can remove this once we insert sync value in 2nd level BB)
    pStateHeapSettings->iMediaIDs         = pDeviceParam->iMaxKernelsPerTask;         // Number of Media IDs = Number of Kernels/Task

    pStateHeapSettings->iKernelCount      = pDeviceParam->iMaxGSHKernelEntries;
    pStateHeapSettings->iKernelBlockSize  = pDeviceParam->iMaxKernelBinarySize;       // The kernel occupied memory need be this block size aligned 256K for IVB/HSW
    pStateHeapSettings->iKernelHeapSize   = pDeviceParam->iMaxGSHKernelEntries * CM_32K;                       // CM_MAX_GSH_KERNEL_ENTRIES * 32*1024;      
    pState->pTotalKernelSize              = (int32_t*)MOS_AllocAndZeroMemory(sizeof(int32_t) * pDeviceParam->iMaxGSHKernelEntries);
    if(!pState->pTotalKernelSize)
    {
        CM_ERROR_ASSERT("Could not allocate enough memory for pState->pTotalKernelSize\n");
        hr = MOS_STATUS_NO_SPACE;
        goto finish;
    }

    pStateHeapSettings->iPerThreadScratchSize = pDeviceParam->iMaxPerThreadScratchSpaceSize;
    pStateHeapSettings->iSipSize          = CM_MAX_SIP_SIZE;
    pStateHeapSettings->iBindingTables    = pDeviceParam->iMaxKernelsPerTask;         // Number of Binding tables = Number of Kernels/Task
    pStateHeapSettings->iSurfacesPerBT    = CM_MAX_SURFACE_STATES_PER_BT;             // Allocate Max Binding Table indices per binding table
    pStateHeapSettings->iSurfaceStates    = CM_MAX_SURFACE_STATES;                    // Allocate Max Surfaces that can be indexed
    pStateHeapSettings->iSamplersAVS      = pDeviceParam->iMaxAVSSamplers;            // Allocate Max AVS samplers

    // Initialize RenderHal Interface
    CM_CHK_MOSSTATUS(pRenderHal->pfnInitialize(pRenderHal, nullptr));

    // Initialize Vebox Interface
    CM_CHK_MOSSTATUS(pState->pVeboxInterface->CreateHeap());

    // Initialize the table only in Static Mode (DSH doesn't use this table at all)
    if (!pState->bDynamicStateHeap)
    {
        // Init the data in kernel entries for Dynamic GSH
        for (int32_t iKernelID = 0; iKernelID < pStateHeapSettings->iKernelCount; ++iKernelID)
        {
            if (iKernelID > 0)
            {
                pState->pTotalKernelSize[iKernelID] = 0;
            }
            else
            {
                pState->pTotalKernelSize[iKernelID] = pStateHeapSettings->iKernelHeapSize;
            }
        }
        pState->nNumKernelsInGSH = 1;
    }

    // Allocate BB (one for each media-state heap)
    pState->iNumBatchBuffers = pStateHeapSettings->iMediaStateHeaps;
    pState->pBatchBuffers = (PMHW_BATCH_BUFFER)MOS_AllocAndZeroMemory(
                                    pState->iNumBatchBuffers *
                                    sizeof(MHW_BATCH_BUFFER));

    CM_CHK_NULL_RETURN_MOSSTATUS(pState->pBatchBuffers);

    pBb = pState->pBatchBuffers;
    for (i = 0; i < (uint32_t)pState->iNumBatchBuffers; i ++, pBb ++)
    {
        pBb->dwSyncTag    = 0;
        pBb->bMatch       = false;
        pBb->iPrivateType = RENDERHAL_BB_TYPE_CM;
        pBb->iPrivateSize = sizeof(CM_HAL_BB_ARGS);
        pBb->pPrivateData = (PCM_HAL_BB_ARGS)MOS_AllocAndZeroMemory(sizeof(CM_HAL_BB_ARGS));
        CM_CHK_NULL_RETURN_MOSSTATUS(pBb->pPrivateData);
        ((PCM_HAL_BB_ARGS)pBb->pPrivateData)->uiRefCount = 1;
    }

    // Allocate TimeStamp Buffer
    CM_CHK_MOSSTATUS(HalCm_AllocateTsResource(pState));

    CM_CHK_MOSSTATUS(HalCm_AllocateTables(pState));

    // Allocate Task Param to hold max tasks
    pState->pTaskParam = (PCM_HAL_TASK_PARAM)MOS_AllocAndZeroMemory(sizeof(CM_HAL_TASK_PARAM));
    CM_CHK_NULL_RETURN_MOSSTATUS(pState->pTaskParam);
    pState->iCurrentTaskEntry = 0;

    // Allocate Task TimeStamp to hold time stamps
    pState->pTaskTimeStamp = (PCM_HAL_TASK_TIMESTAMP)MOS_AllocAndZeroMemory(sizeof(CM_HAL_TASK_TIMESTAMP));
    CM_CHK_NULL_RETURN_MOSSTATUS(pState->pTaskTimeStamp);

    // Setup Registration table entries
    pState->SurfaceRegTable.Count      = pState->CmDeviceParam.iMax2DSurfaceTableSize;
    pState->SurfaceRegTable.pEntries   = pState->pSurf2DTable;

    maxTasks = pState->CmDeviceParam.iMaxTasks;
    // Initialize the task status table
    MOS_FillMemory(pState->pTaskStatusTable, (size_t)maxTasks, CM_INVALID_INDEX);

    // Init the null render flag
    NullHWAccelerationEnable  = pState->pOsInterface->pfnGetNullHWRenderFlags(pState->pOsInterface);
    pState->bNullHwRenderCm          = NullHWAccelerationEnable.Cm || NullHWAccelerationEnable.VPGobal;

    //during initialization stage to allocate sip resource and Get sip binary.
    if (pState->Platform.eRenderCoreFamily < IGFX_GEN10_CORE)
    {
        if ((pState->bDisabledMidThreadPreemption == false)
            || (pState->bEnabledKernelDebug == true))
        {
            CM_CHK_MOSSTATUS(pState->pCmHalInterface->AllocateSIPCSRResource());
            pState->pfnGetSipBinary(pState);
        }
    }
    //Init flag for conditional batch buffer
    pState->bCBBEnabled = HalCm_IsCbbEnabled(pState);
    
    //Turn Turbo boost on
    CM_CHK_MOSSTATUS(pState->pfnEnableTurboBoost(pState));

    hr = MOS_STATUS_SUCCESS;

finish:
    return hr;
}

uint16_t HalCm_GetKernelPerfTag(
    PCM_HAL_STATE           pCmState, 
    PCM_HAL_KERNEL_PARAM    *pKernelParams,
    uint32_t                iNumKernels)
{
    using namespace std;
    
    CM_ASSERT(pCmState);
    CM_ASSERT(pKernelParams);

    int perfTagKernelNum = iNumKernels - 1;
    if (iNumKernels > MAX_COMBINE_NUM_IN_PERFTAG)
    {
        perfTagKernelNum = MAX_COMBINE_NUM_IN_PERFTAG - 1;
    }

    // get a combined kernel name
    uint32_t len = iNumKernels * CM_MAX_KERNEL_NAME_SIZE_IN_BYTE;
    char *combinedName = MOS_NewArray(char, len);
    if (combinedName == nullptr)
    { // Not need to abort the process as this is only for pnp profiling
        CM_ASSERTMESSAGE("Error: Memory allocation error in getPertTag.");
        return 0; // return the default perftag
    }
    CmSafeMemSet(combinedName, 0, len);

    MOS_SecureStrcat(combinedName, len, pKernelParams[0]->pKernelName);
    for (uint32_t i = 1; i < iNumKernels; i++)
    {
        MOS_SecureStrcat(combinedName, len, ";");
        MOS_SecureStrcat(combinedName, len, pKernelParams[i]->pKernelName);
    }

    // get perftag index
    int perfTagIndex = 0;
    map<string, int>::iterator ite = pCmState->pPerfTagIndexMap[perfTagKernelNum]->find(combinedName);
    if (ite == pCmState->pPerfTagIndexMap[perfTagKernelNum]->end())
    {
        if (pCmState->currentPerfTagIndex[perfTagKernelNum] <= MAX_CUSTOMIZED_PERFTAG_INDEX)
        {
            pCmState->pPerfTagIndexMap[perfTagKernelNum]->insert(pair<string, int>(combinedName, pCmState->currentPerfTagIndex[perfTagKernelNum]));
            perfTagIndex = pCmState->currentPerfTagIndex[perfTagKernelNum] ++;
        }
    }
    else
    {
        perfTagIndex = ite->second;
    }

    perfTagIndex = (perfTagIndex &0xFF) | (perfTagKernelNum << 8);
    MosSafeDeleteArray(combinedName);
    return (uint16_t)perfTagIndex;
}


//*-----------------------------------------------------------------------------
//| Purpose: Executes the CM Task
//| Returns: Result of the operation
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_ExecuteTask(
    PCM_HAL_STATE           pState,                                             // [in] Pointer to CM State
    PCM_HAL_EXEC_TASK_PARAM pExecParam)                                         // [in] Pointer to Task Param
{
    MOS_STATUS              hr;
    PRENDERHAL_INTERFACE    pRenderHal;
    PRENDERHAL_MEDIA_STATE  pMediaState;
    PMHW_BATCH_BUFFER       pBatchBuffer;
    PCM_HAL_BB_ARGS         pBbCmArgs;
    PCM_HAL_KERNEL_PARAM    pKernelParam;
    int32_t                 iTaskId;
    int32_t                 iRemBindingTables;
    int32_t                 iBindingTable;
    int32_t                 iBTI;
    int32_t                 iMediaID;
    PRENDERHAL_KRN_ALLOCATION pKrnAllocations[CM_MAX_KERNELS_PER_TASK];
    uint32_t                dwVfeCurbeSize;
    uint32_t                dwMaxInlineDataSize, dwMaxIndirectDataSize;
    uint32_t                i;
    void                    *pCmdBuffer = nullptr;
    PCM_HAL_TASK_PARAM      pTaskParam = pState->pTaskParam;
    uint32_t                uiBTSizePower_2;
    PMOS_INTERFACE          pOsInterface = nullptr;

    //-----------------------------------
    CM_ASSERT(pState);
    CM_ASSERT(pExecParam);
    //-----------------------------------

    hr              = MOS_STATUS_SUCCESS;
    pRenderHal      = pState->pRenderHal;
    pMediaState     = nullptr;
    pBatchBuffer    = nullptr;

    if (pExecParam->iNumKernels > pState->CmDeviceParam.iMaxKernelsPerTask)
    {
        CM_ERROR_ASSERT("Number of Kernels per task exceeds maximum");
        goto finish;
    }

    pState->pOsInterface->pfnSetGpuContext(pState->pOsInterface, (MOS_GPU_CONTEXT)pExecParam->queueOption.GPUContext);

    // Reset states before execute 
    // (clear allocations, get GSH allocation index + any additional housekeeping)
    pState->pOsInterface->pfnResetOsStates(pState->pOsInterface);
    CM_CHK_MOSSTATUS(pRenderHal->pfnReset(pRenderHal));

    MOS_ZeroMemory(pState->pTaskParam, sizeof(CM_HAL_TASK_PARAM));

    MOS_FillMemory(
        pState->pBT2DIndexTable,
        pState->CmDeviceParam.iMax2DSurfaceTableSize * sizeof( CM_HAL_MULTI_USE_BTI_ENTRY ),
        CM_INVALID_INDEX );

    MOS_FillMemory(
        pState->pBT2DUPIndexTable,
        pState->CmDeviceParam.iMax2DSurfaceUPTableSize * sizeof( CM_HAL_MULTI_USE_BTI_ENTRY ),
        CM_INVALID_INDEX );

    MOS_FillMemory(
        pState->pBT3DIndexTable,
        pState->CmDeviceParam.iMax3DSurfaceTableSize * sizeof( CM_HAL_MULTI_USE_BTI_ENTRY ),
        CM_INVALID_INDEX );

    MOS_FillMemory(
        pState->pBTBufferIndexTable,
        pState->CmDeviceParam.iMaxBufferTableSize * sizeof( CM_HAL_MULTI_USE_BTI_ENTRY ),
        CM_INVALID_INDEX );

    MOS_FillMemory(
        pState->pSamplerIndexTable,
        pState->CmDeviceParam.iMaxSamplerTableSize,
        CM_INVALID_INDEX);

    MOS_FillMemory(
        pState->pSampler8x8IndexTable,
        pState->CmDeviceParam.iMaxSampler8x8TableSize,
        CM_INVALID_INDEX);

    pState->WalkerParams.CmWalkerEnable = 0;

    dwVfeCurbeSize = 0;
    dwMaxInlineDataSize = 0;
    dwMaxIndirectDataSize = 0;

    // Get the Task Id
    CM_CHK_MOSSTATUS(HalCm_GetNewTaskId(pState, &iTaskId));

    // Parse the task
    CM_CHK_MOSSTATUS(HalCm_ParseTask(pState, pExecParam));

    // Reset the SSH configuration according to the property of the task
    uiBTSizePower_2 = (uint32_t)pRenderHal->StateHeapSettings.iBTAlignment/pRenderHal->pRenderHalPltInterface->GetBTStateCmdSize();
    while (uiBTSizePower_2 < pTaskParam->surfacePerBT) 
    { 
        uiBTSizePower_2 = uiBTSizePower_2 * 2;
    } 
    pTaskParam->surfacePerBT = uiBTSizePower_2;
    pRenderHal->pStateHeap->iBindingTableSize = MOS_ALIGN_CEIL(pTaskParam->surfacePerBT *  // Reconfigure the binding table size
                                                 pRenderHal->pRenderHalPltInterface->GetBTStateCmdSize(), pRenderHal->StateHeapSettings.iBTAlignment);

    pRenderHal->StateHeapSettings.iBindingTables = pRenderHal->StateHeapSettings.iBindingTables *             // Reconfigure the binding table number
                                                         pRenderHal->StateHeapSettings.iSurfacesPerBT / pTaskParam->surfacePerBT;
    pRenderHal->StateHeapSettings.iSurfacesPerBT = pTaskParam->surfacePerBT;                            // Reconfigure the surface per BT

    if (pExecParam->iNumKernels > (uint32_t)pRenderHal->StateHeapSettings.iBindingTables)
    {
        CM_ERROR_ASSERT("Number of Kernels per task exceeds the number can be hold by binding table");
        goto finish;
    }

    if (pExecParam->bKernelDebugEnabled && Mos_ResourceIsNull(&pState->SipResource.OsResource))
    {
       HalCm_AllocateSipResource( pState); // create  sip resource if it does not exist
    }

    // Assign a MediaState from the MediaStateHeap
    // !!!! THIS MUST BE BEFORE Getting the BATCH_BUFFER !!!
    // since this method syncs the batch buffer and media state.
    if (pState->bDynamicStateHeap)
    {
        if ( pExecParam->user_defined_media_state != nullptr )
        {
            // use exsiting media state as current state 
            pMediaState = static_cast< PRENDERHAL_MEDIA_STATE >( pExecParam->user_defined_media_state );

            // update current state to dsh
            pRenderHal->pStateHeap->pCurMediaState = pMediaState;
            // Refresh sync tag for all media states in submitted queue
            pRenderHal->pfnRefreshSync( pRenderHal );
        }
        else
        {
            // Obtain media state configuration - Curbe, Samplers (3d/AVS/VA), 8x8 sampler table, Media IDs, Kernel Spill area
            RENDERHAL_DYNAMIC_MEDIA_STATE_PARAMS Params;
            HalCm_DSH_GetDynamicStateConfiguration( pState, &Params, pExecParam->iNumKernels, pExecParam->pKernels, pExecParam->piKernelCurbeOffset );

            // Prepare Media States to accommodate all parameters - Curbe, Samplers (3d/AVS/VA), 8x8 sampler table, Media IDs
            pMediaState = pRenderHal->pfnAssignDynamicState( pRenderHal, &Params, RENDERHAL_COMPONENT_CM );
        }
    }
    else
    {
        pMediaState = pRenderHal->pfnAssignMediaState(pRenderHal, RENDERHAL_COMPONENT_CM);
    }
    CM_CHK_NULL_RETURN_MOSSTATUS(pMediaState);

    // Assign/Reset SSH instance
    CM_CHK_MOSSTATUS(pRenderHal->pfnAssignSshInstance(pRenderHal));

    // Dynamic Batch Buffer allocation

    if (!pState->WalkerParams.CmWalkerEnable)
    {
        // Get the Batch buffer
        CM_CHK_MOSSTATUS(HalCm_GetBatchBuffer(pState, pExecParam->iNumKernels, pExecParam->pKernels, &pBatchBuffer));
        CM_CHK_NULL_RETURN_MOSSTATUS(pBatchBuffer);
        pBbCmArgs = (PCM_HAL_BB_ARGS)pBatchBuffer->pPrivateData;

        // Lock the batch buffer
        if ( (pBbCmArgs->uiRefCount == 1) ||
             (pState->pTaskParam->reuseBBUpdateMask == 1) )
        {
            CM_CHK_MOSSTATUS(pRenderHal->pfnLockBB(pRenderHal, pBatchBuffer));
        }
    }

    if (pState->use_new_sampler_heap == false)
    {
        HalCm_AcquireSamplerStatistics(pState);
    }

    // Load all kernels in the same state heap - expand ISH if necessary BEFORE programming media states.
    // This is better than having to expand ISH in the middle of loading, when part of MediaIDs are
    // already programmed - not a problem in the old implementation where it would simply remove old
    // kernels out of the way.
    if (pState->bDynamicStateHeap)
    {
        CM_CHK_MOSSTATUS(HalCm_DSH_LoadKernelArray(pState, pExecParam->pKernels, pExecParam->iNumKernels, pKrnAllocations));
    }

    for (i = 0; i < pExecParam->iNumKernels; i++)
    {
        CM_HAL_INDEX_PARAM indexParam;
        MOS_ZeroMemory(&indexParam, sizeof(CM_HAL_INDEX_PARAM));
        pKernelParam = pExecParam->pKernels[i];

        CM_CHK_MOSSTATUS(HalCm_SetupStatesForKernelInitial(pState, pMediaState, pBatchBuffer, iTaskId, pKernelParam, &indexParam,
            pExecParam->piKernelCurbeOffset[i], iBTI, iMediaID, pKrnAllocations[i]));

        CM_CHK_MOSSTATUS(HalCm_FinishStatesForKernel(pState, pMediaState, pBatchBuffer, iTaskId, pKernelParam, i, &indexParam,
            iBTI, iMediaID, pKrnAllocations[i]));

        dwVfeCurbeSize += MOS_ALIGN_CEIL(pKernelParam->iKrnCurbeSize, pState->pRenderHal->dwCurbeBlockAlign);
        if (pKernelParam->iPayloadSize > dwMaxInlineDataSize)
        {
            dwMaxInlineDataSize = pKernelParam->iPayloadSize;
        }
        if (pKernelParam->IndirectDataParam.iIndirectDataSize > dwMaxIndirectDataSize)
        {
            dwMaxIndirectDataSize = pKernelParam->IndirectDataParam.iIndirectDataSize;
        }

        if (pExecParam->uiConditionalEndBitmap & (uint64_t)1 << i)
        {
            CM_CHK_MOSSTATUS(HalCm_SetConditionalEndInfo(pState, pTaskParam->conditionalEndInfo, pTaskParam->conditionalBBEndParams, i));
        }
    }

    // Store the Max Payload Sizes in the Task Params
    pState->pTaskParam->dwVfeCurbeSize = dwVfeCurbeSize;
    if (dwMaxIndirectDataSize)
    {
        pState->pTaskParam->dwUrbEntrySize = dwMaxIndirectDataSize;
    }
    else
    {
        pState->pTaskParam->dwUrbEntrySize = dwMaxInlineDataSize;
    }

    // We may have to send additional Binding table commands in command buffer.
    // This is needed because the surface offset (from the base on SSH) 
    // calculation takes into account the max binding tables allocated in the 
    // SSH.
    iRemBindingTables = pRenderHal->StateHeapSettings.iBindingTables - pExecParam->iNumKernels;

    if (iRemBindingTables > 0)
    {
        for (i = 0; i < (uint32_t)iRemBindingTables; i++)
        {
            CM_CHK_MOSSTATUS(pRenderHal->pfnAssignBindingTable(
                    pRenderHal, 
                    &iBindingTable));
        }
    }

    // until now, we know binding table index for debug surface
    // let's get system thread
    pOsInterface = pState->pOsInterface;
    pOsInterface->pfnResetPerfBufferID(pOsInterface);
    if (pOsInterface->pfnIsPerfTagSet(pOsInterface) == false)
    {
        pOsInterface->pfnIncPerfFrameID(pOsInterface);
        uint16_t perfTag = HalCm_GetKernelPerfTag(pState, pExecParam->pKernels, pExecParam->iNumKernels);
        pOsInterface->pfnSetPerfTag(pOsInterface, perfTag);
    }

    // Submit HW commands and states
    CM_CHK_MOSSTATUS(pState->pCmHalInterface->SubmitCommands(
                    pBatchBuffer, iTaskId, pExecParam->pKernels, &pCmdBuffer));

    // Set the Task ID
    pExecParam->iTaskIdOut = iTaskId;

    // Set OS data
    if(pCmdBuffer)
    {
        pExecParam->OsData = pCmdBuffer;
    }

    // Update the task ID table
    pState->pTaskStatusTable[iTaskId] = (char)iTaskId;

finish:

    if (pState->bDynamicStateHeap)
    {
        if (pMediaState && hr != MOS_STATUS_SUCCESS)
        {
            // Failed, release media state and heap resources
            pRenderHal->pfnReleaseDynamicState(pRenderHal, pMediaState);
        }
        else
        {
            pRenderHal->pfnSubmitDynamicState(pRenderHal, pMediaState);
        }
    }


    if (pBatchBuffer)  // for Media Walker, pBatchBuffer is empty
    {
        if (pBatchBuffer->bLocked)
        {
            // Only happens in Error cases
            CM_CHK_NULL_RETURN_MOSSTATUS(pBatchBuffer->pPrivateData);
            if (((PCM_HAL_BB_ARGS)pBatchBuffer->pPrivateData)->uiRefCount == 1)
            {
                pRenderHal->pfnUnlockBB(pRenderHal, pBatchBuffer);
            }
        }
    }

    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose: Executes the CM Group Task
//| Returns: Result of the operation
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_ExecuteGroupTask(
    PCM_HAL_STATE                   pState,           // [in] Pointer to CM State
    PCM_HAL_EXEC_GROUP_TASK_PARAM   pExecGroupParam)  // [in] Pointer to Task Param
{
    MOS_STATUS              hr = MOS_STATUS_SUCCESS;
    PRENDERHAL_INTERFACE     pRenderHal = pState->pRenderHal;
    CM_HAL_INDEX_PARAM      indexParam;
    int32_t                 iTaskId;
    uint32_t                iRemBindingTables;
    int32_t                 iBindingTable;
    int32_t                 iBTI;
    int32_t                 iMediaID;
    PRENDERHAL_MEDIA_STATE  pMediaState = nullptr;
    uint32_t                i;
    void                    *pCmdBuffer   = nullptr;
    PCM_HAL_KERNEL_PARAM    pKernelParam = nullptr;
    PCM_HAL_TASK_PARAM      pTaskParam = pState->pTaskParam;
    uint32_t                uiBTSizePower_2;
    uint32_t                dwVfeCurbeSize = 0;
    PRENDERHAL_KRN_ALLOCATION pKrnAllocations[CM_MAX_KERNELS_PER_TASK];
    PMOS_INTERFACE          pOsInterface = nullptr;

    //-----------------------------------
    CM_ASSERT(pState);
    CM_ASSERT(pExecGroupParam);
    //-----------------------------------

    pState->pOsInterface->pfnSetGpuContext(pState->pOsInterface, (MOS_GPU_CONTEXT)pExecGroupParam->queueOption.GPUContext);

    MOS_ZeroMemory(pState->pTaskParam, sizeof(CM_HAL_TASK_PARAM));
    MOS_ZeroMemory(&indexParam, sizeof(CM_HAL_INDEX_PARAM));

    MOS_FillMemory(
        pState->pBT2DIndexTable,
        pState->CmDeviceParam.iMax2DSurfaceTableSize * sizeof( CM_HAL_MULTI_USE_BTI_ENTRY ),
        CM_INVALID_INDEX );

    MOS_FillMemory(
        pState->pBT2DUPIndexTable,
        pState->CmDeviceParam.iMax2DSurfaceUPTableSize * sizeof( CM_HAL_MULTI_USE_BTI_ENTRY ),
        CM_INVALID_INDEX );

    MOS_FillMemory(
        pState->pBT3DIndexTable,
        pState->CmDeviceParam.iMax3DSurfaceTableSize * sizeof( CM_HAL_MULTI_USE_BTI_ENTRY ),
        CM_INVALID_INDEX );

    MOS_FillMemory(
        pState->pBTBufferIndexTable,
        pState->CmDeviceParam.iMaxBufferTableSize * sizeof( CM_HAL_MULTI_USE_BTI_ENTRY ),
        CM_INVALID_INDEX );
    MOS_FillMemory(
        pState->pSamplerIndexTable,
        pState->CmDeviceParam.iMaxSamplerTableSize,
        CM_INVALID_INDEX);
    MOS_FillMemory(
        pState->pSampler8x8IndexTable,
        pState->CmDeviceParam.iMaxSampler8x8TableSize,
        CM_INVALID_INDEX);

    // Reset states before execute 
    // (clear allocations, get GSH allocation index + any additional housekeeping)
    pState->pOsInterface->pfnResetOsStates(pState->pOsInterface);
    CM_CHK_MOSSTATUS(pRenderHal->pfnReset(pRenderHal));

    pState->WalkerParams.CmWalkerEnable = 0;
    pState->pTaskParam->blGpGpuWalkerEnabled = true;

    // Get the Task Id
    CM_CHK_MOSSTATUS(HalCm_GetNewTaskId(pState, &iTaskId));

    // Parse the task
    CM_CHK_MOSSTATUS(HalCm_ParseGroupTask(pState, pExecGroupParam));

    // Reset the SSH configuration according to the property of the task
    uiBTSizePower_2 = (uint32_t)pRenderHal->StateHeapSettings.iBTAlignment/pRenderHal->pRenderHalPltInterface->GetBTStateCmdSize();
    while (uiBTSizePower_2 < pTaskParam->surfacePerBT) 
    { 
        uiBTSizePower_2 = uiBTSizePower_2 * 2;
    } 
    pTaskParam->surfacePerBT = uiBTSizePower_2;
    pRenderHal->pStateHeap->iBindingTableSize = MOS_ALIGN_CEIL(pTaskParam->surfacePerBT *               // Reconfigure the binding table size
                                                         pRenderHal->pRenderHalPltInterface->GetBTStateCmdSize(), 
                                                         pRenderHal->StateHeapSettings.iBTAlignment);

    pRenderHal->StateHeapSettings.iBindingTables           = pRenderHal->StateHeapSettings.iBindingTables *          // Reconfigure the binding table number
                                                         pRenderHal->StateHeapSettings.iSurfacesPerBT / pTaskParam->surfacePerBT;
    pRenderHal->StateHeapSettings.iSurfacesPerBT           = pTaskParam->surfacePerBT;                           // Reconfigure the surface per BT

    if (pExecGroupParam->iNumKernels > (uint32_t)pRenderHal->StateHeapSettings.iBindingTables)
    {
        CM_ERROR_ASSERT("Number of Kernels per task exceeds the number can be hold by binding table");
        goto finish;
    }

    if (pExecGroupParam->bKernelDebugEnabled && Mos_ResourceIsNull(&pState->SipResource.OsResource))
    {
       HalCm_AllocateSipResource( pState); // create  sip resource if it does not exist
    }

    // Assign a MediaState from the MediaStateHeap
    // !!!! THIS MUST BE BEFORE Getting the BATCH_BUFFER !!!
    // since this method syncs the batch buffer and media state.
    if (pState->bDynamicStateHeap)
    {
        if ( pExecGroupParam->user_defined_media_state != nullptr )
        {
            // Preload all kernels
            CM_CHK_MOSSTATUS( HalCm_DSH_LoadKernelArray( pState, pExecGroupParam->pKernels, pExecGroupParam->iNumKernels, pKrnAllocations ) );

            // use exsiting media state as current state 
            pMediaState = static_cast< PRENDERHAL_MEDIA_STATE >( pExecGroupParam->user_defined_media_state );

            // update current state to dsh
            pRenderHal->pStateHeap->pCurMediaState = pMediaState;
            // Refresh sync tag for all media states in submitted queue
            pRenderHal->pfnRefreshSync( pRenderHal );
        }
        else
        {
            // Preload all kernels
            CM_CHK_MOSSTATUS(HalCm_DSH_LoadKernelArray(pState, pExecGroupParam->pKernels, pExecGroupParam->iNumKernels, pKrnAllocations));

            // Obtain media state configuration - Curbe, Samplers (3d/AVS/VA), 8x8 sampler table, Media IDs, Kernel Spill area
            RENDERHAL_DYNAMIC_MEDIA_STATE_PARAMS Params;
            HalCm_DSH_GetDynamicStateConfiguration(pState, &Params, pExecGroupParam->iNumKernels, pExecGroupParam->pKernels, pExecGroupParam->piKernelCurbeOffset);

            // Prepare Media States to accommodate all parameters
            pMediaState = pRenderHal->pfnAssignDynamicState(pRenderHal, &Params, RENDERHAL_COMPONENT_CM);
        }
    }
    else
    {
        // Assign a MediaState from the MediaStateHeap
        // !!!! THIS MUST BE BEFORE Getting the BATCH_BUFFER !!!
        // since this method syncs the batch buffer and media state.
        pMediaState = pRenderHal->pfnAssignMediaState(pRenderHal, RENDERHAL_COMPONENT_CM);
    }
    CM_CHK_NULL_RETURN_MOSSTATUS(pMediaState);

    // Assign/Reset SSH instance
    CM_CHK_MOSSTATUS(pRenderHal->pfnAssignSshInstance(pRenderHal));

    if (pState->use_new_sampler_heap == false)
    {
        HalCm_AcquireSamplerStatistics(pState);
    }

    for (i = 0; i < pExecGroupParam->iNumKernels; i++)
    {
        CM_HAL_INDEX_PARAM indexParam;
        MOS_ZeroMemory(&indexParam, sizeof(CM_HAL_INDEX_PARAM));
        pKernelParam = pExecGroupParam->pKernels[i];

        CM_CHK_MOSSTATUS(HalCm_SetupStatesForKernelInitial(pState, pMediaState, nullptr, iTaskId, pKernelParam, &indexParam,
            pExecGroupParam->piKernelCurbeOffset[i], iBTI, iMediaID, pKrnAllocations[i]));

        CM_CHK_MOSSTATUS(HalCm_FinishStatesForKernel(pState, pMediaState, nullptr, iTaskId, pKernelParam, i, &indexParam,
            iBTI, iMediaID, pKrnAllocations[i]));

        dwVfeCurbeSize += MOS_ALIGN_CEIL(pKernelParam->iKrnCurbeSize, pState->pRenderHal->dwCurbeBlockAlign);
    }

    // Store the Max Payload Sizes in the Task Params
    pState->pTaskParam->dwVfeCurbeSize = dwVfeCurbeSize;
    pState->pTaskParam->dwUrbEntrySize = 0;

    // We may have to send additional Binding table commands in command buffer.
    // This is needed because the surface offset (from the base on SSH) 
    // calculation takes into account the max binding tables allocated in the 
    // SSH.
    iRemBindingTables = pRenderHal->StateHeapSettings.iBindingTables - pExecGroupParam->iNumKernels;

    if (iRemBindingTables > 0)
    {
        for (i = 0; i < iRemBindingTables; i++)
        {
            CM_CHK_MOSSTATUS(pRenderHal->pfnAssignBindingTable(
                    pRenderHal, 
                    &iBindingTable));
        }
    }

    // until now, we know binding table index for debug surface
    // let's get system thread
    if (pExecGroupParam->bKernelDebugEnabled)
    {
        CM_CHK_MOSSTATUS(pState->pfnGetSipBinary(pState));
    }

    pOsInterface = pState->pOsInterface;
    pOsInterface->pfnResetPerfBufferID(pOsInterface);
    if (pOsInterface->pfnIsPerfTagSet(pOsInterface) == false)
    {
        pOsInterface->pfnIncPerfFrameID(pOsInterface);
        int perfTag = HalCm_GetKernelPerfTag(pState, pExecGroupParam->pKernels, pExecGroupParam->iNumKernels);
        pOsInterface->pfnSetPerfTag(pOsInterface, (uint16_t)perfTag);
    }

    // Submit HW commands and states
    CM_CHK_MOSSTATUS(pState->pCmHalInterface->SubmitCommands(
                     nullptr, iTaskId, pExecGroupParam->pKernels, &pCmdBuffer));

    // Set the Task ID
    pExecGroupParam->iTaskIdOut = iTaskId;

    // Set OS data
    if(pCmdBuffer)
    {
        pExecGroupParam->OsData = pCmdBuffer;
    }

    // Update the task ID table
    pState->pTaskStatusTable[iTaskId] = (char)iTaskId;

finish:

    if (pState->bDynamicStateHeap)
    {
        if (pMediaState && hr != MOS_STATUS_SUCCESS)
        {
            // Failed, release media state and heap resources
            pRenderHal->pfnReleaseDynamicState(pRenderHal, pMediaState);
        }
        else
        {
            pRenderHal->pfnSubmitDynamicState(pRenderHal, pMediaState);
        }
    }

    return hr;
}

MOS_STATUS HalCm_ExecuteHintsTask(
    PCM_HAL_STATE                 pState,                     // [in] Pointer to CM State
    PCM_HAL_EXEC_HINTS_TASK_PARAM pExecHintsParam)            // [in] Pointer to Task Param
{
    MOS_STATUS              hr;
    PRENDERHAL_INTERFACE    pRenderHal;
    PRENDERHAL_MEDIA_STATE  pMediaState;
    PMHW_BATCH_BUFFER       pBatchBuffer;
    PCM_HAL_BB_ARGS         pBbCmArgs;
    PCM_HAL_KERNEL_PARAM    pKernelParam;
    uint32_t                i;
    uint32_t                numTasks;
    uint64_t                uiOrigKernelIds[CM_MAX_KERNELS_PER_TASK];
    int32_t                 iTaskId;
    int32_t                 iRemBindingTables;
    int32_t                 iBindingTable;
    uint32_t                dwVfeCurbeSize;
    uint32_t                dwMaxInlineDataSize;
    uint32_t                dwMaxIndirectDataSize;
    int32_t                 *pBindingTableEntries;
    int32_t                 *pMediaIds;
    PRENDERHAL_KRN_ALLOCATION *pKrnAllocations;
    PCM_HAL_INDEX_PARAM     pIndexParams;
    bool                    useMediaObjects;
    void                    *pCmdBuffer;
    bool                    splitTask;
    bool                    lastTask;
    PMOS_INTERFACE          pOsInterface = nullptr;

    //------------------------------------
    CM_ASSERT(pState);
    CM_ASSERT(pExecHintsParam);
    //------------------------------------

    hr                   = MOS_STATUS_SUCCESS;
    pRenderHal           = pState->pRenderHal;
    pMediaState          = nullptr;
    pBatchBuffer         = nullptr;
    pBindingTableEntries = nullptr;
    pMediaIds            = nullptr;
    pKrnAllocations      = nullptr;
    pIndexParams         = nullptr;
    useMediaObjects      = false;
    pCmdBuffer           = nullptr;
    splitTask            = false;
    lastTask             = false;

    if (pExecHintsParam->iNumKernels > pState->CmDeviceParam.iMaxKernelsPerTask)
    {
        CM_ERROR_ASSERT("Number of Kernels per task exceeds maximum");
        goto finish;
    }

    pState->pOsInterface->pfnSetGpuContext(pState->pOsInterface, (MOS_GPU_CONTEXT)pExecHintsParam->queueOption.GPUContext);

    pBindingTableEntries = (int*)MOS_AllocAndZeroMemory(sizeof(int)*pExecHintsParam->iNumKernels);
    pMediaIds = (int*)MOS_AllocAndZeroMemory(sizeof(int)* pExecHintsParam->iNumKernels);
    pKrnAllocations = (PRENDERHAL_KRN_ALLOCATION *)MOS_AllocAndZeroMemory(sizeof(void *)* pExecHintsParam->iNumKernels);
    pIndexParams = (PCM_HAL_INDEX_PARAM)MOS_AllocAndZeroMemory(sizeof(CM_HAL_INDEX_PARAM)* pExecHintsParam->iNumKernels);
    if (!pBindingTableEntries || !pMediaIds || !pKrnAllocations || !pIndexParams)
    {
        CM_ERROR_ASSERT("Memory allocation failed in ExecuteHints Task");
        goto finish;
    }

    // check hints to see if need to split into multiple tasks
    numTasks = ( pExecHintsParam->iHints & CM_HINTS_MASK_NUM_TASKS ) >> CM_HINTS_NUM_BITS_TASK_POS;
    if( numTasks > 1 )
    {
        splitTask = true;
    }

    MOS_FillMemory(pBindingTableEntries, sizeof(int) * pExecHintsParam->iNumKernels, CM_INVALID_INDEX);
    MOS_FillMemory(pMediaIds, sizeof(int) * pExecHintsParam->iNumKernels, CM_INVALID_INDEX);
    MOS_FillMemory(pKrnAllocations, sizeof(void *)* pExecHintsParam->iNumKernels, 0);

    // Reset states before execute
    // (clear allocations, get GSH allocation index + any additional housekeeping)
    pState->pOsInterface->pfnResetOsStates(pState->pOsInterface);
    CM_CHK_MOSSTATUS(pRenderHal->pfnReset(pRenderHal));

    MOS_ZeroMemory(pState->pTaskParam, sizeof(CM_HAL_TASK_PARAM));

    MOS_FillMemory(
        pState->pBT2DIndexTable,
        pState->CmDeviceParam.iMax2DSurfaceTableSize * sizeof( CM_HAL_MULTI_USE_BTI_ENTRY ),
        CM_INVALID_INDEX );

    MOS_FillMemory(
        pState->pBT2DUPIndexTable,
        pState->CmDeviceParam.iMax2DSurfaceUPTableSize * sizeof( CM_HAL_MULTI_USE_BTI_ENTRY ),
        CM_INVALID_INDEX );

    MOS_FillMemory(
        pState->pBT3DIndexTable,
        pState->CmDeviceParam.iMax3DSurfaceTableSize * sizeof( CM_HAL_MULTI_USE_BTI_ENTRY ),
        CM_INVALID_INDEX );

    MOS_FillMemory(
        pState->pBTBufferIndexTable,
        pState->CmDeviceParam.iMaxBufferTableSize * sizeof( CM_HAL_MULTI_USE_BTI_ENTRY ),
        CM_INVALID_INDEX );

    MOS_FillMemory(
        pState->pSamplerIndexTable,
        pState->CmDeviceParam.iMaxSamplerTableSize,
        CM_INVALID_INDEX);

    MOS_FillMemory(
        pState->pSampler8x8IndexTable,
        pState->CmDeviceParam.iMaxSampler8x8TableSize,
        CM_INVALID_INDEX);

    pState->WalkerParams.CmWalkerEnable = 0;

    dwVfeCurbeSize = 0;
    dwMaxInlineDataSize = 0;
    dwMaxIndirectDataSize = 0;

    MOS_ZeroMemory(&uiOrigKernelIds, CM_MAX_KERNELS_PER_TASK * sizeof(uint64_t));

    // Get the Task Id
    CM_CHK_MOSSTATUS(HalCm_GetNewTaskId(pState, &iTaskId));

    // Parse the task
    CM_CHK_MOSSTATUS(HalCm_ParseHintsTask(pState, pExecHintsParam));

    // Assign a MediaState from the MediaStateHeap
    // !!!! THIS MUST BE BEFORE Getting the BATCH_BUFFER !!!
    // since this method syncs the batch buffer and media state.
    if (pState->bDynamicStateHeap)
    {
        if ( pExecHintsParam->user_defined_media_state != nullptr )
        {
            // use exsiting media state as current state 
            pMediaState = static_cast< PRENDERHAL_MEDIA_STATE >( pExecHintsParam->user_defined_media_state );

            // update current state to dsh
            pRenderHal->pStateHeap->pCurMediaState = pMediaState;
            // Refresh sync tag for all media states in submitted queue
            pRenderHal->pfnRefreshSync( pRenderHal );
        }
        else
        {
            // Obtain media state configuration - Curbe, Samplers (3d/AVS/VA), 8x8 sampler table, Media IDs, Kernel Spill area
            RENDERHAL_DYNAMIC_MEDIA_STATE_PARAMS Params;
            HalCm_DSH_GetDynamicStateConfiguration(pState, &Params, pExecHintsParam->iNumKernels, pExecHintsParam->pKernels, pExecHintsParam->piKernelCurbeOffset);

            // Prepare Media States to accommodate all parameters - Curbe, Samplers (3d/AVS/VA), 8x8 sampler table, Media IDs
            pMediaState = pRenderHal->pfnAssignDynamicState(pRenderHal, &Params, RENDERHAL_COMPONENT_CM);
        }
    }
    else
    {
        pMediaState = pRenderHal->pfnAssignMediaState(pRenderHal, RENDERHAL_COMPONENT_CM);
    }
    CM_CHK_NULL_RETURN_MOSSTATUS(pMediaState);

    if (pState->use_new_sampler_heap == false)
    {
        HalCm_AcquireSamplerStatistics(pState);
    }

    // Assign/Reset SSH instance
    CM_CHK_MOSSTATUS(pRenderHal->pfnAssignSshInstance(pRenderHal));

    if (!pState->WalkerParams.CmWalkerEnable)
    {
        if( splitTask )
        {
            // save original kernel IDs for kernel binary re-use in GSH
            for( i = 0; i < pExecHintsParam->iNumKernels; ++i )
            {
                uiOrigKernelIds[i] = pExecHintsParam->pKernels[i]->uiKernelId;
            }

            // need to add tag to kernel IDs to distinguish batch buffer
            CM_CHK_MOSSTATUS(HalCm_AddKernelIDTag(pExecHintsParam->pKernels, pExecHintsParam->iNumKernels, numTasks, pExecHintsParam->iNumTasksGenerated));
        }

        // Get the Batch buffer
        CM_CHK_MOSSTATUS(HalCm_GetBatchBuffer(pState, pExecHintsParam->iNumKernels, pExecHintsParam->pKernels, &pBatchBuffer));

        if( splitTask )
        {
            // restore kernel IDs for kernel binary re-use in GSH
            for( i = 0; i < pExecHintsParam->iNumKernels; ++i )
            {
                pExecHintsParam->pKernels[i]->uiKernelId = uiOrigKernelIds[i];
            } 
        }

        // Lock the batch buffer
        CM_CHK_NULL_RETURN_MOSSTATUS(pBatchBuffer->pPrivateData);
        pBbCmArgs = (PCM_HAL_BB_ARGS)pBatchBuffer->pPrivateData;
        if ( (pBbCmArgs->uiRefCount == 1) ||
             ( pState->pTaskParam->reuseBBUpdateMask == 1) ) 
        {
            CM_CHK_MOSSTATUS(pRenderHal->pfnLockBB(pRenderHal, pBatchBuffer));
        }
    }

    // Load all kernels in the same state heap - expand ISH if necessary BEFORE programming media states.
    // This is better than having to expand ISH in the middle of loading, when part of MediaIDs are
    // already programmed - not a problem in the old implementation where it would simply remove old
    // kernels out of the way.
    if (pState->bDynamicStateHeap)
    {
        CM_CHK_MOSSTATUS(HalCm_DSH_LoadKernelArray(pState, pExecHintsParam->pKernels, pExecHintsParam->iNumKernels, pKrnAllocations));
    }

    // 0: media walker
    // 1: media object
    if( (pExecHintsParam->iHints & CM_HINTS_MASK_MEDIAOBJECT) == CM_HINTS_MASK_MEDIAOBJECT )
    {
        for (i = 0; i < pExecHintsParam->iNumKernels; ++i)
        {
            CM_CHK_MOSSTATUS(HalCm_SetupStatesForKernelInitial(pState, pMediaState, pBatchBuffer, iTaskId, pExecHintsParam->pKernels[i], &pIndexParams[i],
                pExecHintsParam->piKernelCurbeOffset[i], pBindingTableEntries[i], pMediaIds[i], pKrnAllocations[i]));
        }

        CM_CHK_NULL_RETURN_MOSSTATUS(pBatchBuffer);

        CM_CHK_MOSSTATUS(HalCm_FinishStatesForKernelMix(pState, pBatchBuffer, iTaskId, pExecHintsParam->pKernels,
            pIndexParams, pBindingTableEntries, pMediaIds, pKrnAllocations, pExecHintsParam->iNumKernels, pExecHintsParam->iHints, pExecHintsParam->isLastTask));

        for( i = 0; i < pExecHintsParam->iNumKernels; ++i)
        {
            pKernelParam = pExecHintsParam->pKernels[i];
            dwVfeCurbeSize += MOS_ALIGN_CEIL(pKernelParam->iKrnCurbeSize, pState->pRenderHal->dwCurbeBlockAlign);
            if( pKernelParam->iPayloadSize > dwMaxInlineDataSize)
            {
                dwMaxInlineDataSize = pKernelParam->iPayloadSize;
            }
            if( pKernelParam->IndirectDataParam.iIndirectDataSize > dwMaxIndirectDataSize )
            {
                dwMaxIndirectDataSize = pKernelParam->IndirectDataParam.iIndirectDataSize;
            }
        }

        // Store the Max Payload Sizes in the Task Param
        pState->pTaskParam->dwVfeCurbeSize = dwVfeCurbeSize;
        if( dwMaxIndirectDataSize)
        {
            pState->pTaskParam->dwVfeCurbeSize = dwMaxIndirectDataSize;
        }
        else
        {
            pState->pTaskParam->dwUrbEntrySize = dwMaxInlineDataSize;
        }

        // We may have to send additional Binding table commands in command buffer.
        // This is needed because the surface offset (from the base on SSH)
        // calculation takes into account the max binding tables allocated in the 
        // SSH.
        iRemBindingTables = pState->CmDeviceParam.iMaxKernelsPerTask -
            pExecHintsParam->iNumKernels;

        if( iRemBindingTables > 0)
        {
            for( i = 0; i < (uint32_t)iRemBindingTables; ++i)
            {
                CM_CHK_MOSSTATUS(pRenderHal->pfnAssignBindingTable(
                    pRenderHal,
                    &iBindingTable));
            }
        }

        pOsInterface = pState->pOsInterface;
        pOsInterface->pfnResetPerfBufferID(pOsInterface);
        if (pOsInterface->pfnIsPerfTagSet(pOsInterface) == false)
        {
            pOsInterface->pfnIncPerfFrameID(pOsInterface);
            int perfTag = HalCm_GetKernelPerfTag(pState, pExecHintsParam->pKernels, pExecHintsParam->iNumKernels);
            pOsInterface->pfnSetPerfTag(pOsInterface, (uint16_t)perfTag);
        }

        // Submit HW commands and states
        CM_CHK_MOSSTATUS(pState->pCmHalInterface->SubmitCommands(
                        pBatchBuffer, iTaskId, pExecHintsParam->pKernels, &pCmdBuffer));

        // Set the Task ID
        pExecHintsParam->iTaskIdOut = iTaskId;

        // Set OS data
        if( pCmdBuffer )
        {
            pExecHintsParam->OsData = pCmdBuffer;
        }

        // Update the task ID table
        pState->pTaskStatusTable[iTaskId] = (char)iTaskId;
    }
    else
    {
        // use media walker
        // unimplemented for now
        CM_ASSERTMESSAGE("Error: Media walker is not supported.");
        hr = MOS_STATUS_UNKNOWN;
    }

finish:

    if (pState->bDynamicStateHeap)
    {
        if (pMediaState && hr != MOS_STATUS_SUCCESS)
        {
            // Failed, release media state and heap resources
            pRenderHal->pfnReleaseDynamicState(pRenderHal, pMediaState);
        }
        else
        {
            pRenderHal->pfnSubmitDynamicState(pRenderHal, pMediaState);
        }
    }

    if (pBatchBuffer) // for MediaWalker, pBatchBuffer is empty
    {
        if (pBatchBuffer->bLocked)
        {
            // Only happens in Error cases
            CM_CHK_NULL_RETURN_MOSSTATUS(pBatchBuffer->pPrivateData);

            if (((PCM_HAL_BB_ARGS)pBatchBuffer->pPrivateData)->uiRefCount == 1)
            {
                pRenderHal->pfnUnlockBB(pRenderHal, pBatchBuffer);
            }
        }
    }

    // free memory
    if( pBindingTableEntries )          MOS_FreeMemory(pBindingTableEntries);
    if( pMediaIds )                     MOS_FreeMemory(pMediaIds);
    if( pKrnAllocations )               MOS_FreeMemory(pKrnAllocations);
    if( pIndexParams )                  MOS_FreeMemory( pIndexParams );

    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Send Commands to HW
//| Returns:    Get the HAL Max values
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_GetMaxValues(
    PCM_HAL_STATE           pState,                                             // [in]  Pointer to CM State
    PCM_HAL_MAX_VALUES      pMaxValues)                                         // [out] Pointer to Max values
{
    PRENDERHAL_INTERFACE  pRenderHal;

    pRenderHal = pState->pRenderHal;

    pMaxValues->iMaxTasks                         = pState->CmDeviceParam.iMaxTasks;
    pMaxValues->iMaxKernelsPerTask                = CM_MAX_KERNELS_PER_TASK;
    pMaxValues->iMaxKernelBinarySize              = pState->CmDeviceParam.iMaxKernelBinarySize;
    pMaxValues->iMaxSpillSizePerHwThread          = pState->CmDeviceParam.iMaxPerThreadScratchSpaceSize;
    pMaxValues->iMaxSamplerTableSize              = CM_MAX_SAMPLER_TABLE_SIZE;
    pMaxValues->iMaxBufferTableSize               = CM_MAX_BUFFER_SURFACE_TABLE_SIZE;
    pMaxValues->iMax2DSurfaceTableSize            = CM_MAX_2D_SURFACE_TABLE_SIZE;
    pMaxValues->iMax3DSurfaceTableSize            = CM_MAX_3D_SURFACE_TABLE_SIZE;
    pMaxValues->iMaxArgsPerKernel                 = CM_MAX_ARGS_PER_KERNEL;
    pMaxValues->iMaxUserThreadsPerTask            = CM_MAX_USER_THREADS;
    pMaxValues->iMaxUserThreadsPerTaskNoThreadArg = CM_MAX_USER_THREADS_NO_THREADARG;
    pMaxValues->iMaxArgByteSizePerKernel          = CM_MAX_ARG_BYTE_PER_KERNEL;
    pMaxValues->iMaxSurfacesPerKernel             = pRenderHal->pHwCaps->dwMaxBTIndex;
    pMaxValues->iMaxSamplersPerKernel             = pRenderHal->pHwCaps->dwMaxUnormSamplers;
    pMaxValues->iMaxHwThreads                     = pRenderHal->pHwCaps->dwMaxThreads;

    return MOS_STATUS_SUCCESS;
}


//*-----------------------------------------------------------------------------
//| Purpose:    Get the HAL Max extended values
//| Returns:    Get the HAL Max extended values
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_GetMaxValuesEx(
    PCM_HAL_STATE           pState,                                             // [in]  Pointer to CM State
    PCM_HAL_MAX_VALUES_EX   pMaxValuesEx)                                       // [out] Pointer to extended Max values
{
    MOS_STATUS  hr = MOS_STATUS_SUCCESS;
    pMaxValuesEx->iMax2DUPSurfaceTableSize = CM_MAX_2D_SURFACE_UP_TABLE_SIZE;
    pMaxValuesEx->iMaxSampler8x8TableSize = CM_MAX_SAMPLER_8X8_TABLE_SIZE;
    pMaxValuesEx->iMaxCURBESizePerKernel = CM_MAX_CURBE_SIZE_PER_KERNEL;
    pMaxValuesEx->iMaxCURBESizePerTask = CM_MAX_CURBE_SIZE_PER_TASK;
    pMaxValuesEx->iMaxIndirectDataSizePerKernel = CM_MAX_INDIRECT_DATA_SIZE_PER_KERNEL;

    //MaxThreadWidth x MaxThreadHeight x ColorCount
    pMaxValuesEx->iMaxUserThreadsPerMediaWalker = \
                            pState->pCmHalInterface->GetMediaWalkerMaxThreadWidth()* \
                            pState->pCmHalInterface->GetMediaWalkerMaxThreadHeight() * \
                            CM_THREADSPACE_MAX_COLOR_COUNT;

    CM_CHK_MOSSTATUS(HalCm_GetMaxThreadCountPerThreadGroup( pState, &pMaxValuesEx->iMaxUserThreadsPerThreadGroup ) );

finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Register Sampler
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_RegisterSampler(
    PCM_HAL_STATE           pState,                                             // [in]  Pointer to CM State
    PCM_HAL_SAMPLER_PARAM   pParam)                                             // [in]  Pointer to Sampler Param
{
    MOS_STATUS              hr;
    PMHW_SAMPLER_STATE_PARAM pEntry;
    uint32_t                i;

    hr      = MOS_STATUS_SUCCESS;
    pEntry  = nullptr;

    // Find a free slot
    for (i = 0; i < pState->CmDeviceParam.iMaxSamplerTableSize; i++)
    {
        if (!pState->pSamplerTable[i].bInUse)
        {
            pEntry              = &pState->pSamplerTable[i];
            pParam->dwHandle    = (uint32_t)i;
            break;
        }
    }

    if (!pEntry)
    {
        CM_ERROR_ASSERT("Sampler table is full");
        goto finish;
    }

    pEntry->SamplerType  = MHW_SAMPLER_TYPE_3D;
    if (pState->use_new_sampler_heap == true)
    {
        pEntry->ElementType = MHW_Sampler1Element;
    }
    else
    {
        pEntry->ElementType = MHW_Sampler4Elements;
    }
    CM_CHK_MOSSTATUS(HalCm_GetGfxMapFilter(pParam->MinFilter,  &pEntry->Unorm.MinFilter));
    CM_CHK_MOSSTATUS(HalCm_GetGfxMapFilter(pParam->MagFilter,  &pEntry->Unorm.MagFilter));
    CM_CHK_MOSSTATUS(HalCm_GetGfxTextAddress(pParam->AddressU, &pEntry->Unorm.AddressU));
    CM_CHK_MOSSTATUS(HalCm_GetGfxTextAddress(pParam->AddressV, &pEntry->Unorm.AddressV));
    CM_CHK_MOSSTATUS(HalCm_GetGfxTextAddress(pParam->AddressW, &pEntry->Unorm.AddressW));

    pEntry->Unorm.SurfaceFormat = (MHW_SAMPLER_SURFACE_PIXEL_TYPE)pParam->SurfaceFormat;
    switch (pEntry->Unorm.SurfaceFormat)
    {
        case MHW_SAMPLER_SURFACE_PIXEL_UINT:
            pEntry->Unorm.BorderColorRedU = pParam->BorderColorRedU;
            pEntry->Unorm.BorderColorGreenU = pParam->BorderColorGreenU;
            pEntry->Unorm.BorderColorBlueU = pParam->BorderColorBlueU;
            pEntry->Unorm.BorderColorAlphaU = pParam->BorderColorAlphaU;
            break;
        case MHW_SAMPLER_SURFACE_PIXEL_SINT:
            pEntry->Unorm.BorderColorRedS = pParam->BorderColorRedS;
            pEntry->Unorm.BorderColorGreenS = pParam->BorderColorGreenS;
            pEntry->Unorm.BorderColorBlueS = pParam->BorderColorBlueS;
            pEntry->Unorm.BorderColorAlphaS = pParam->BorderColorAlphaS;
            break;
        default:
            pEntry->Unorm.BorderColorRedF = pParam->BorderColorRedF;
            pEntry->Unorm.BorderColorGreenF = pParam->BorderColorGreenF;
            pEntry->Unorm.BorderColorBlueF = pParam->BorderColorBlueF;
            pEntry->Unorm.BorderColorAlphaF = pParam->BorderColorAlphaF;
    }
    pEntry->Unorm.bBorderColorIsValid = true;

    pEntry->bInUse = true;

finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    UnRegister Sampler
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_UnRegisterSampler(
    PCM_HAL_STATE               pState,                                         // [in]  Pointer to CM State
    uint32_t                    dwHandle)                                       // [in]  Pointer to Sampler Param
{
    MOS_STATUS              hr;
    PMHW_SAMPLER_STATE_PARAM pEntry;

    hr = MOS_STATUS_SUCCESS;

    if (dwHandle >= pState->CmDeviceParam.iMaxSamplerTableSize)
    {
        CM_ERROR_ASSERT("Invalid handle '%d'", dwHandle);
        goto finish;
    }

    pEntry = &pState->pSamplerTable[dwHandle];

    // need to clear the state entirely instead of just setting bInUse to false
    MOS_ZeroMemory(pEntry, sizeof(MHW_SAMPLER_STATE_PARAM));

finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Register Sampler8x8
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_RegisterSampler8x8(
    PCM_HAL_STATE                pState, 
    PCM_HAL_SAMPLER_8X8_PARAM    pParam) 
{
    return pState->pCmHalInterface->RegisterSampler8x8(pParam);
}

//*-----------------------------------------------------------------------------
//| Purpose:    UnRegister Sampler
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_UnRegisterSampler8x8(
    PCM_HAL_STATE               pState,                                         // [in]  Pointer to CM State
    uint32_t                    dwHandle)                                       // [in]  Pointer to Sampler8x8 Param
{
    MOS_STATUS                  hr;
    uint32_t                    index_8x8;
    PMHW_SAMPLER_STATE_PARAM    pEntry;
    PCM_HAL_SAMPLER_8X8_ENTRY   pSampler8x8Entry;

    hr = MOS_STATUS_SUCCESS;

    if (dwHandle >= pState->CmDeviceParam.iMaxSamplerTableSize) {
        CM_ERROR_ASSERT("Invalid handle '%d'", dwHandle);
        goto finish;
    }

    pEntry = &pState->pSamplerTable[dwHandle];
    pEntry->bInUse = false;

    if ( pEntry->SamplerType == MHW_SAMPLER_TYPE_AVS )
    {
        index_8x8 = pEntry->Avs.stateID;
        if ( index_8x8 >= pState->CmDeviceParam.iMaxSampler8x8TableSize )
        {
            CM_ERROR_ASSERT( "Invalid 8x8 handle '%d'", dwHandle );
            goto finish;
        }

        pSampler8x8Entry = &pState->pSampler8x8Table[ index_8x8 ];
    pSampler8x8Entry->bInUse = false;
    }

    // need to clear the state entirely instead of just setting bInUse to false
    MOS_ZeroMemory(pEntry, sizeof(MHW_SAMPLER_STATE_PARAM));
finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Frees the buffer and removes from the table
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_FreeBuffer(
    PCM_HAL_STATE           pState,                                             // [in]  Pointer to CM State
    uint32_t                dwHandle)                                           // [in]  Pointer to Buffer Param
{
    MOS_STATUS              hr;
    PCM_HAL_BUFFER_ENTRY    pEntry;
    PMOS_INTERFACE          pOsInterface;

    hr              = MOS_STATUS_SUCCESS;
    pOsInterface    = pState->pOsInterface;

    // Get the Buffer Entry
    CM_CHK_MOSSTATUS(HalCm_GetBufferEntry(pState, dwHandle, &pEntry));
    if (pEntry->isAllocatedbyCmrtUmd)
    {
        pOsInterface->pfnFreeResourceWithFlag(pOsInterface, &pEntry->OsResource, SURFACE_FLAG_ASSUME_NOT_IN_USE);
    }
    else
    {
        HalCm_OsResource_Unreference(&pEntry->OsResource);
    }
    pOsInterface->pfnResetResourceAllocationIndex(pOsInterface, &pEntry->OsResource);
    pEntry->iSize = 0;
    pEntry->pAddress = nullptr;

finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Set surface read flag used in on demand sync 
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_SetSurfaceReadFlag(
    PCM_HAL_STATE           pState,                                             // [in]  Pointer to CM State
    uint32_t                dwHandle,                                           // [in]  index of surface 2d
    bool                    bReadSync)                                          
{
    MOS_STATUS                 hr  = MOS_STATUS_SUCCESS;
    PCM_HAL_SURFACE2D_ENTRY    pEntry;

    // Get the Buffer Entry
    CM_CHK_MOSSTATUS(HalCm_GetSurface2DEntry(pState, dwHandle, &pEntry));

    // Two slots, RENDER3 and RENDER4
    if ( ( pState->GpuContext == MOS_GPU_CONTEXT_RENDER3 ) || ( pState->GpuContext == MOS_GPU_CONTEXT_RENDER4 ) ) 
    {
        pEntry->bReadSync[pState->GpuContext - MOS_GPU_CONTEXT_RENDER3] = bReadSync;
    }
    else
    {
        return MOS_STATUS_UNKNOWN;
    }

finish:
    return hr;
}


//*-----------------------------------------------------------------------------
//| Purpose:    Read the data from buffer and return
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_LockBuffer(
    PCM_HAL_STATE           pState,                                             // [in]  Pointer to CM State
    PCM_HAL_BUFFER_PARAM    pParam)                                             // [in]  Pointer to Buffer Param
{
    MOS_STATUS              hr;
    PCM_HAL_BUFFER_ENTRY    pEntry;
    PMOS_INTERFACE          pOsInterface;
    MOS_LOCK_PARAMS         LockFlags;
    hr              = MOS_STATUS_SUCCESS;
    pOsInterface    = pState->pOsInterface;

    CM_CHK_MOSSTATUS(HalCm_GetBufferEntry(pState, pParam->dwHandle, &pEntry));

    if ((pParam->iLockFlag != CM_HAL_LOCKFLAG_READONLY) && (pParam->iLockFlag != CM_HAL_LOCKFLAG_WRITEONLY) )
    {
        CM_ERROR_ASSERT("Invalid lock flag!");
        hr = MOS_STATUS_UNKNOWN;
        goto finish;
    }

    // Lock the resource
    MOS_ZeroMemory(&LockFlags, sizeof(MOS_LOCK_PARAMS));

    if (pParam->iLockFlag == CM_HAL_LOCKFLAG_READONLY)
    {
        LockFlags.ReadOnly = true;
    }
    else
    {
        LockFlags.WriteOnly = true;
    }

    LockFlags.ForceCached = true;
    pParam->pData = pOsInterface->pfnLockResource(
                    pOsInterface, 
                    &pEntry->OsResource, 
                    &LockFlags);
    CM_CHK_NULL_RETURN_MOSSTATUS(pParam->pData);

finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Writes the data to buffer
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_UnlockBuffer(
    PCM_HAL_STATE           pState,                                             // [in]  Pointer to CM State
    PCM_HAL_BUFFER_PARAM    pParam)                                             // [in]  Pointer to Buffer Param
{
    MOS_STATUS              hr;
    PCM_HAL_BUFFER_ENTRY    pEntry;
    PMOS_INTERFACE          pOsInterface;

    hr              = MOS_STATUS_SUCCESS;
    pOsInterface    = pState->pOsInterface;

    CM_CHK_MOSSTATUS(HalCm_GetBufferEntry(pState, pParam->dwHandle, &pEntry));

    CM_HRESULT2MOSSTATUS_AND_CHECK(pOsInterface->pfnUnlockResource(pOsInterface, &pEntry->OsResource));

finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Frees the buffer and removes from the table
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_FreeSurface2DUP(
    PCM_HAL_STATE           pState,                                             // [in]  Pointer to CM State
    uint32_t                dwHandle)                                           // [in]  Pointer to Buffer Param
{
    MOS_STATUS                    hr;
    PCM_HAL_SURFACE2D_UP_ENTRY    pEntry;
    PMOS_INTERFACE          pOsInterface;

    hr              = MOS_STATUS_SUCCESS;
    pOsInterface    = pState->pOsInterface;

    // Get the Buffer Entry
    CM_CHK_MOSSTATUS(HalCm_GetResourceUPEntry(pState, dwHandle, &pEntry));

    pOsInterface->pfnFreeResourceWithFlag(pOsInterface, &pEntry->OsResource, SURFACE_FLAG_ASSUME_NOT_IN_USE);

    pOsInterface->pfnResetResourceAllocationIndex(pOsInterface, &pEntry->OsResource);
    pEntry->iWidth = 0;

finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get 2D surface pitch and physical size
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_GetSurface2DTileYPitch(
    PCM_HAL_STATE                pState,                                             // [in]  Pointer to CM State
    PCM_HAL_SURFACE2D_PARAM      pParam)                                        // [in]  Pointer to Buffer Param
{
    MOS_STATUS                  hr;
    MOS_SURFACE                 Surface;
    PRENDERHAL_INTERFACE        pRenderHal;
    uint32_t                    iIndex;
    RENDERHAL_GET_SURFACE_INFO  Info;

    //-----------------------------------------------
    CM_ASSERT(pState);
    //-----------------------------------------------

    hr              = MOS_STATUS_UNKNOWN;
    pRenderHal    = pState->pRenderHal;
    iIndex          = pParam->dwHandle;

    // Get Details of 2D surface and fill the Surface 
    MOS_ZeroMemory(&Surface, sizeof(Surface));

    Surface.OsResource  = pState->pUmdSurf2DTable[iIndex].OsResource;
    Surface.dwWidth     = pState->pUmdSurf2DTable[iIndex].iWidth;
    Surface.dwHeight    = pState->pUmdSurf2DTable[iIndex].iHeight;
    Surface.Format      = pState->pUmdSurf2DTable[iIndex].format;
    Surface.dwDepth     = 1;

    MOS_ZeroMemory(&Info, sizeof(RENDERHAL_GET_SURFACE_INFO));

    CM_CHK_MOSSTATUS(RenderHal_GetSurfaceInfo(
        pState->pOsInterface,
        &Info,
        &Surface));

    pParam->iPitch      = Surface.dwPitch;

finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Sets width and height values for 2D surface state
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_Set2DSurfaceStateParam(
     PCM_HAL_STATE                            pState,
     PCM_HAL_SURFACE2D_SURFACE_STATE_PARAM    pParam,
     uint32_t                                 iAliasIndex,
     uint32_t                                 dwHandle)
{
    MOS_STATUS hr;
    uint32_t width;
    uint32_t height;

    CM_CHK_NULL_RETURN_MOSSTATUS(pState);
    CM_CHK_NULL_RETURN_MOSSTATUS(pParam);

    hr     = MOS_STATUS_SUCCESS;
    pState->pUmdSurf2DTable[dwHandle].surfaceStateParam[iAliasIndex / pState->nSurfaceArraySize] = *pParam;

finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Sets width and height values for 2D surface state
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_SetBufferSurfaceStateParameters(
     PCM_HAL_STATE                            pState,
     PCM_HAL_BUFFER_SURFACE_STATE_PARAM       pParam)
{
    MOS_STATUS hr;
    uint32_t size;
    uint32_t offset;
    uint32_t iIndex;
    uint32_t iAliasIndex;

    CM_CHK_NULL_RETURN_MOSSTATUS(pState);
    CM_CHK_NULL_RETURN_MOSSTATUS(pParam);
    
    hr     = MOS_STATUS_SUCCESS;
    iIndex = pParam->dwHandle;
    iAliasIndex = pParam->iAliasIndex;
    
    pState->pBufferTable[iIndex].surfaceStateEntry[iAliasIndex / pState->nSurfaceArraySize].iSurfaceStateSize = pParam->iSize;
    pState->pBufferTable[iIndex].surfaceStateEntry[iAliasIndex / pState->nSurfaceArraySize].iSurfaceStateOffset = pParam->iOffset;
    pState->pBufferTable[iIndex].surfaceStateEntry[iAliasIndex / pState->nSurfaceArraySize].wSurfaceStateMOCS = pParam->wMOCS;
    
finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Sets mocs value for surface
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_SetSurfaceMOCS(
     PCM_HAL_STATE                  pState,
     uint32_t                       dwHandle,
     uint16_t                       mocs,
     uint32_t                       argKind)
{
    MOS_STATUS hr = MOS_STATUS_SUCCESS;

    switch (argKind)
    {
        case CM_ARGUMENT_SURFACEBUFFER:
            pState->pBufferTable[dwHandle].memObjCtl = mocs;
            break;
        case CM_ARGUMENT_SURFACE2D:
        case CM_ARGUMENT_SURFACE2D_SAMPLER:
        case CM_ARGUMENT_SURFACE_SAMPLER8X8_AVS:
        case CM_ARGUMENT_SURFACE_SAMPLER8X8_VA:
            pState->pUmdSurf2DTable[dwHandle].memObjCtl = mocs;
            break;
        case CM_ARGUMENT_SURFACE2D_UP:
        case CM_ARGUMENT_SURFACE2DUP_SAMPLER:
            pState->pSurf2DUPTable[dwHandle].memObjCtl = mocs;
            break;
        case CM_ARGUMENT_SURFACE3D:
            pState->pSurf3DTable[dwHandle].memObjCtl = mocs;
            break;
        default:
            CM_ERROR_ASSERT("Invalid argument type in MOCS settings");
            goto finish;
    }
    
finish:
    return hr;
}


//*-----------------------------------------------------------------------------
//| Purpose:    Allocate Surface 2D
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_AllocateSurface2D(
    PCM_HAL_STATE                pState,                                             // [in]  Pointer to CM State
    PCM_HAL_SURFACE2D_PARAM      pParam)                                             // [in]  Pointer to surface 2D Param
{
    MOS_STATUS              hr;
    PMOS_INTERFACE          pOsInterface;
    PCM_HAL_SURFACE2D_ENTRY pEntry = nullptr;
    MOS_ALLOC_GFXRES_PARAMS AllocParams;
    uint32_t                i;

    //-----------------------------------------------
    CM_ASSERT(pParam->iWidth > 0);
    //-----------------------------------------------

    hr              = MOS_STATUS_SUCCESS;
    pOsInterface    = pState->pOsInterface;

    // Find a free slot
    for (i = 0; i < pState->CmDeviceParam.iMax2DSurfaceTableSize; i++)
    {
        if(Mos_ResourceIsNull(&pState->pUmdSurf2DTable[i].OsResource))
        {
            pEntry              = &pState->pUmdSurf2DTable[i];
            pParam->dwHandle    = (uint32_t)i;
            break;
        }
    }

    if (!pEntry)
    {
        CM_ERROR_ASSERT("Surface2D table is full");
        goto finish;
    }

    if(pParam->isAllocatedbyCmrtUmd)
    {
        MOS_ZeroMemory(&AllocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        AllocParams.Type          = MOS_GFXRES_2D;
        AllocParams.dwWidth       = pParam->iWidth;
        AllocParams.dwHeight      = pParam->iHeight;
        AllocParams.pSystemMemory = pParam->pData;
        AllocParams.Format        = pParam->format;
        AllocParams.TileType      = MOS_TILE_Y;
        AllocParams.pBufName      = "CmSurface2D";

        CM_HRESULT2MOSSTATUS_AND_CHECK(pOsInterface->pfnAllocateResource(
            pOsInterface, 
            &AllocParams,
            &pEntry->OsResource));

        pEntry->iWidth                  = pParam->iWidth;
        pEntry->iHeight                 = pParam->iHeight;
        pEntry->format                  = pParam->format;
        pEntry->isAllocatedbyCmrtUmd    = pParam->isAllocatedbyCmrtUmd;
    }
    else
    {
        pEntry->iWidth  = pParam->iWidth;
        pEntry->iHeight = pParam->iHeight;
        pEntry->format  = pParam->format;
        pEntry->isAllocatedbyCmrtUmd = false;
        pEntry->OsResource = *pParam->pMosResource;

        HalCm_OsResource_Reference(&pEntry->OsResource);
    }

    for (int i = 0; i < CM_HAL_GPU_CONTEXT_COUNT; i++)
    {
        pEntry->bReadSync[i] = false;
    }

finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Frees the Surface 2D and removes from the table
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_FreeSurface2D(
    PCM_HAL_STATE           pState,                                             // [in]  Pointer to CM State
    uint32_t                dwHandle)                                           // [in]  Pointer to Buffer Param
{
    MOS_STATUS                 hr;
    PCM_HAL_SURFACE2D_ENTRY    pEntry;
    PMOS_INTERFACE             pOsInterface;

    hr              = MOS_STATUS_SUCCESS;
    pOsInterface    = pState->pOsInterface;

    // Get the Buffer Entry
    CM_CHK_MOSSTATUS(HalCm_GetSurface2DEntry(pState, dwHandle, &pEntry));
    if(pEntry->isAllocatedbyCmrtUmd)
    {
        pOsInterface->pfnFreeResourceWithFlag(pOsInterface, &pEntry->OsResource, SURFACE_FLAG_ASSUME_NOT_IN_USE);
    }
    else
    {
        HalCm_OsResource_Unreference(&pEntry->OsResource);
    }

    MOS_ZeroMemory(&pEntry->OsResource, sizeof(pEntry->OsResource));

    pEntry->iWidth = 0;
    pEntry->iHeight = 0;
    pEntry->frameType = CM_FRAME;

    for (int i = 0; i < CM_HAL_GPU_CONTEXT_COUNT; i++)
    {
        pEntry->bReadSync[i] = false;
    }

finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Frees the resource and removes from the table
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_Free3DResource(
    PCM_HAL_STATE           pState,                                             // [in]  Pointer to CM State
    uint32_t                dwHandle)                                           // [in]  Pointer to Buffer Param
{
    MOS_STATUS               hr;
    PCM_HAL_3DRESOURCE_ENTRY pEntry;
    PMOS_INTERFACE           pOsInterface;

    hr              = MOS_STATUS_SUCCESS;
    pOsInterface    = pState->pOsInterface;

    // Get the Buffer Entry
    CM_CHK_MOSSTATUS(HalCm_Get3DResourceEntry(pState, dwHandle, &pEntry));

    pOsInterface->pfnFreeResourceWithFlag(pOsInterface, &pEntry->OsResource, SURFACE_FLAG_ASSUME_NOT_IN_USE);

    pOsInterface->pfnResetResourceAllocationIndex(pOsInterface, &pEntry->OsResource);

finish:
    return hr;
}


//*-----------------------------------------------------------------------------
//| Purpose:    Lock the resource and return
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_Lock3DResource(
    PCM_HAL_STATE               pState,                                         // [in]  Pointer to CM State
    PCM_HAL_3DRESOURCE_PARAM    pParam)                                         // [in]  Pointer to 3D Param
{
    MOS_STATUS                  hr = MOS_STATUS_SUCCESS;
    PCM_HAL_3DRESOURCE_ENTRY    pEntry;
    MOS_LOCK_PARAMS             LockFlags;
    RENDERHAL_GET_SURFACE_INFO  Info;
    PMOS_INTERFACE              pOsInterface = nullptr;
    MOS_SURFACE                 Surface;

    // Get the 3D Resource Entry
    CM_CHK_MOSSTATUS(HalCm_Get3DResourceEntry(pState, pParam->dwHandle, &pEntry));
    if ((pParam->iLockFlag != CM_HAL_LOCKFLAG_READONLY) && (pParam->iLockFlag != CM_HAL_LOCKFLAG_WRITEONLY) )
    {
        CM_ERROR_ASSERT("Invalid lock flag!");
        hr = MOS_STATUS_UNKNOWN;
        goto finish;
    }

    // Get resource information
    MOS_ZeroMemory(&Surface, sizeof(Surface));
    Surface.OsResource = pEntry->OsResource;
    Surface.Format     = Format_Invalid;
    pOsInterface       = pState->pOsInterface;

    MOS_ZeroMemory(&Info, sizeof(RENDERHAL_GET_SURFACE_INFO));

    CM_CHK_MOSSTATUS(RenderHal_GetSurfaceInfo(
              pOsInterface,
              &Info,
              &Surface));

    pParam->pPitch   = Surface.dwPitch;
    pParam->dwQPitch = Surface.dwQPitch;
    pParam->bQPitchEnable = pState->pCmHalInterface->IsSurf3DQpitchSupportedbyHw();

    // Lock the resource
    MOS_ZeroMemory(&LockFlags, sizeof(MOS_LOCK_PARAMS));

    if (pParam->iLockFlag == CM_HAL_LOCKFLAG_READONLY)
    {
        LockFlags.ReadOnly = true;
    }
    else
    {
        LockFlags.WriteOnly = true;
    }

    LockFlags.ForceCached = true;
    pParam->pData = pOsInterface->pfnLockResource(
                    pOsInterface, 
                    &pEntry->OsResource,
                    &LockFlags);
    CM_CHK_NULL_RETURN_MOSSTATUS(pParam->pData);

finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Unlock the resource and return
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_Unlock3DResource(
    PCM_HAL_STATE               pState,                                         // [in]  Pointer to CM State
    PCM_HAL_3DRESOURCE_PARAM    pParam)                                         // [in]  Pointer to 3D Param
{
    MOS_STATUS                  hr;
    PCM_HAL_3DRESOURCE_ENTRY    pEntry;
    PMOS_INTERFACE              pOsInterface;

    hr              = MOS_STATUS_SUCCESS;
    pOsInterface    = pState->pOsInterface;

    // Get the 3D Resource Entry
    CM_CHK_MOSSTATUS(HalCm_Get3DResourceEntry(pState, pParam->dwHandle, &pEntry));

    // Lock the resource
    CM_HRESULT2MOSSTATUS_AND_CHECK(pOsInterface->pfnUnlockResource(pOsInterface, &pEntry->OsResource));

finish:
    return hr;
}


MOS_STATUS HalCm_SetCompressionMode(
    PCM_HAL_STATE               pState,
    CM_HAL_SURFACE2D_COMPRESSIOM_PARAM  MmcParam)                                       
{
    MOS_STATUS              hr = MOS_STATUS_SUCCESS;
    PMOS_INTERFACE          pOsInterface = pState->pOsInterface;

    CM_ASSERT(MmcParam.dwHandle);

    PCM_HAL_SURFACE2D_ENTRY     pEntry;

    // Get the 2D Resource Entry
    CM_CHK_MOSSTATUS(HalCm_GetSurface2DEntry(pState, MmcParam.dwHandle, &pEntry));

    //set compression bit passed down  
    CM_CHK_MOSSTATUS(pOsInterface->pfnSetMemoryCompressionMode(pOsInterface, &(pEntry->OsResource), (MOS_MEMCOMP_STATE)MmcParam.MmcMode));
    
    

finish:
    return hr;
}

MOS_STATUS HalCm_SetL3Cache(
    const L3ConfigRegisterValues            *pL3Values,
    PCmHalL3Settings                      pCmHalL3Cache )
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    // in legacy platforms, we map:
    // ConfigRegister0->SqcReg1
    // ConfigRegister1->CntlReg2
    // ConfigRegister2->CntlReg3
    // ConfigRegister3->CntlReg
    CM_CHK_NULL( pCmHalL3Cache );
    CM_CHK_NULL(pL3Values);
    
    pCmHalL3Cache->override_settings    =
                (pL3Values->config_register0  || pL3Values->config_register1 ||
                 pL3Values->config_register2 || pL3Values->config_register3 );
    pCmHalL3Cache->cntl_reg_override    = (pL3Values->config_register3 != 0);
    pCmHalL3Cache->cntl_reg2_override   = (pL3Values->config_register1 != 0);
    pCmHalL3Cache->cntl_reg3_override   = (pL3Values->config_register2 != 0);
    pCmHalL3Cache->sqc_reg1_override    = (pL3Values->config_register0 != 0);
    pCmHalL3Cache->cntl_reg             = pL3Values->config_register3;
    pCmHalL3Cache->cntl_reg2            = pL3Values->config_register1;
    pCmHalL3Cache->cntl_reg3            = pL3Values->config_register2;
    pCmHalL3Cache->sqc_reg1             = pL3Values->config_register0;

finish:
    return MOS_STATUS_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Set Cap values
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_SetCaps(
    PCM_HAL_STATE              pState,
    PCM_HAL_MAX_SET_CAPS_PARAM pSetCapsParam)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CM_CHK_NULL(pState);
    CM_CHK_NULL(pSetCapsParam);
    CM_CHK_NULL(pState->pRenderHal);
    CM_CHK_NULL(pState->pRenderHal->pHwCaps)

    switch (pSetCapsParam->Type)
    {
    case CM_SET_MAX_HW_THREADS:
        if( pSetCapsParam->MaxValue <= 0 ||
            pSetCapsParam->MaxValue > pState->pRenderHal->pHwCaps->dwMaxThreads )
        {
            eStatus = MOS_STATUS_UNKNOWN;
            goto finish;
        }
        else
        {
            pState->MaxHWThreadValues.APIValue = pSetCapsParam->MaxValue;
        }
        break;

    case CM_SET_HW_L3_CONFIG:
        eStatus = pState->pCmHalInterface->SetL3CacheConfig( &pSetCapsParam->L3CacheValues,
                    &pState->l3_settings );
        break;

    default:
        eStatus = MOS_STATUS_UNKNOWN;
        goto finish;
    }

finish:
    return eStatus;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Task sets the power option which will be used by this task
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_SetPowerOption(
    PCM_HAL_STATE               pState,
    PCM_POWER_OPTION            pPowerOption )
{
    MOS_SecureMemcpy( &pState->PowerOption, sizeof( pState->PowerOption ), pPowerOption, sizeof( pState->PowerOption ) );
    return MOS_STATUS_SUCCESS;
}

//*-----------------------------------------------------------------------------
// Purpose: Get the time in ns from QueryPerformanceCounter
// Returns: Result of the operation
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_GetGlobalTime(LARGE_INTEGER *pGlobalTime)
{
    if(pGlobalTime == nullptr)
    {
        return MOS_STATUS_NULL_POINTER;
    }

    if (MOS_QueryPerformanceCounter((uint64_t*)&(pGlobalTime->QuadPart)) == false)
    {
        return MOS_STATUS_UNKNOWN;
    }

    return MOS_STATUS_SUCCESS;
}

//*-----------------------------------------------------------------------------
// Purpose: Convert time from nanosecond to QPC time
// Returns: Result of the operation
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_ConvertToQPCTime(uint64_t nanoseconds, LARGE_INTEGER *QPCTime)
{
    LARGE_INTEGER     perfFreq;

    if(QPCTime == nullptr)
    {
        return MOS_STATUS_NULL_POINTER;
    }
        
    if (MOS_QueryPerformanceFrequency((uint64_t*)&perfFreq.QuadPart) == false)
    {
        return MOS_STATUS_UNKNOWN;
    }

    QPCTime->QuadPart = (uint64_t)(nanoseconds * perfFreq.QuadPart / 1000000000.0);

    return MOS_STATUS_SUCCESS;
}

//------------------------------------------------------------------------------
//| Purpose: Halcm updates power state to hw state
//| Returns: 
//------------------------------------------------------------------------------
MOS_STATUS HalCm_UpdatePowerOption(
    PCM_HAL_STATE               pState,
    PCM_POWER_OPTION            pPowerOption )
{
    MOS_STATUS hr = MOS_STATUS_SUCCESS;
    PRENDERHAL_INTERFACE pRenderHal = pState->pRenderHal;

    RENDERHAL_POWEROPTION PowerOption;
    PowerOption.nSlice     = (uint8_t)pPowerOption->nSlice;
    PowerOption.nSubSlice  = (uint8_t)pPowerOption->nSubSlice;
    PowerOption.nEU        = (uint8_t)pPowerOption->nEU;

    // option set in CM create device to use slice shutdown for life of CM device ( override previous value if necessary )
    if ( pState->bRequestSingleSlice == true )
    {
        PowerOption.nSlice = 1;
    }

    pRenderHal->pfnSetPowerOptionMode( pRenderHal, &PowerOption );

    return hr;
}

MOS_STATUS HalCm_InitPerfTagIndexMap(PCM_HAL_STATE pCmState)
{
    using namespace std;
    CM_ASSERT(pCmState);
    for (int i = 0; i < MAX_COMBINE_NUM_IN_PERFTAG; i++)
    {
        pCmState->currentPerfTagIndex[i] = 1;
#if MOS_MESSAGES_ENABLED
        pCmState->pPerfTagIndexMap[i] = MOS_NewUtil<map<string, int> >(__FUNCTION__, __FILE__, __LINE__);
#else
        pCmState->pPerfTagIndexMap[i] = MOS_NewUtil<map<string, int> >();
#endif
       
        CM_CHK_NULL_RETURN(pCmState->pPerfTagIndexMap[i]);
    }
    
    pCmState->pPerfTagIndexMap[0]->insert(pair<string, int>("surfaceCopy_read_NV12_32x32", GPUCOPY_READ_PERFTAG_INDEX));
    pCmState->pPerfTagIndexMap[0]->insert(pair<string, int>("surfaceCopy_read_NV12_aligned_32x32", GPUCOPY_READ_PERFTAG_INDEX));
    pCmState->pPerfTagIndexMap[0]->insert(pair<string, int>("surfaceCopy_read_32x32", GPUCOPY_READ_PERFTAG_INDEX));
    pCmState->pPerfTagIndexMap[0]->insert(pair<string, int>("surfaceCopy_read_aligned_32x32", GPUCOPY_READ_PERFTAG_INDEX));
    
    pCmState->pPerfTagIndexMap[0]->insert(pair<string, int>("surfaceCopy_write_NV12_32x32", GPUCOPY_WRITE_PERFTAG_INDEX));
    pCmState->pPerfTagIndexMap[0]->insert(pair<string, int>("surfaceCopy_write_32x32", GPUCOPY_WRITE_PERFTAG_INDEX));
    
    pCmState->pPerfTagIndexMap[0]->insert(pair<string, int>("SurfaceCopy_2DTo2D_NV12_32x32", GPUCOPY_G2G_PERFTAG_INDEX));
    pCmState->pPerfTagIndexMap[0]->insert(pair<string, int>("SurfaceCopy_2DTo2D_32x32", GPUCOPY_G2G_PERFTAG_INDEX));
    
    pCmState->pPerfTagIndexMap[0]->insert(pair<string, int>("SurfaceCopy_BufferToBuffer_4k", GPUCOPY_C2C_PERFTAG_INDEX));
    pCmState->pPerfTagIndexMap[0]->insert(pair<string, int>("SurfaceCopy_BufferToBuffer_4k", GPUCOPY_C2C_PERFTAG_INDEX));

    pCmState->pPerfTagIndexMap[0]->insert(pair<string, int>("surfaceCopy_set_NV12", GPUINIT_PERFTAG_INDEX));
    pCmState->pPerfTagIndexMap[0]->insert(pair<string, int>("surfaceCopy_set", GPUINIT_PERFTAG_INDEX));
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HalCm_InsertToStateBufferList(
    PCM_HAL_STATE               pState,
    void                        *kernel_ptr,
    uint32_t                    state_buffer_index,
    CM_STATE_BUFFER_TYPE        state_buffer_type,
    uint32_t                    state_buffer_size,
    uint64_t                    state_buffer_va_ptr,
    PRENDERHAL_MEDIA_STATE      media_state_ptr )
{
    MOS_STATUS result = MOS_STATUS_SUCCESS;

    CM_HAL_STATE_BUFFER_ENTRY entry;
    entry.kernel_ptr = kernel_ptr;
    entry.state_buffer_index = state_buffer_index;
    entry.state_buffer_type = state_buffer_type;
    entry.state_buffer_size = state_buffer_size;
    entry.state_buffer_va_ptr = state_buffer_va_ptr;
    entry.media_state_ptr = media_state_ptr;

    ( *pState->state_buffer_list_ptr )[ kernel_ptr ] = entry;
    return result;
}

MOS_STATUS HalCm_DeleteFromStateBufferList(
    PCM_HAL_STATE               pState,
    void                        *kernel_ptr )
{
    MOS_STATUS result = MOS_STATUS_SUCCESS;

    pState->state_buffer_list_ptr->erase( kernel_ptr );

    return result;
}

PRENDERHAL_MEDIA_STATE HalCm_GetMediaStatePtrForKernel(
    PCM_HAL_STATE               pState,
    void                        *kernel_ptr )
{
    if ( pState->state_buffer_list_ptr->find( kernel_ptr ) != pState->state_buffer_list_ptr->end() )
    {
        return ( *pState->state_buffer_list_ptr )[ kernel_ptr ].media_state_ptr;
    }
    else
    {
        return nullptr;
    }
}

uint64_t HalCm_GetStateBufferVAPtrForSurfaceIndex(
    PCM_HAL_STATE               pState,
    uint32_t                    surf_index )
{
    for ( auto list_item = pState->state_buffer_list_ptr->begin(); list_item != pState->state_buffer_list_ptr->end(); list_item++ )
    {
        if ( list_item->second.state_buffer_index == surf_index )
        {
            return list_item->second.state_buffer_va_ptr;
        }
    }
    return 0;
}

PRENDERHAL_MEDIA_STATE HalCm_GetMediaStatePtrForSurfaceIndex(
    PCM_HAL_STATE               pState,
    uint32_t                    surf_index )
{
    for ( auto list_item = pState->state_buffer_list_ptr->begin(); list_item != pState->state_buffer_list_ptr->end(); list_item++ )
    {
        if ( list_item->second.state_buffer_index == surf_index )
        {
            return list_item->second.media_state_ptr;
        }
    }
    return nullptr;
}

uint64_t HalCm_GetStateBufferVAPtrForMediaStatePtr(
    PCM_HAL_STATE               pState,
    PRENDERHAL_MEDIA_STATE      media_state_ptr )
{
    for ( auto list_item = pState->state_buffer_list_ptr->begin(); list_item != pState->state_buffer_list_ptr->end(); list_item++ )
    {
        if ( list_item->second.media_state_ptr == media_state_ptr )
        {
            return list_item->second.state_buffer_va_ptr;
        }
    }
    return 0;
}

uint32_t HalCm_GetStateBufferSizeForKernel(
    PCM_HAL_STATE               pState,
    void                        *kernel_ptr )
{
    if ( pState->state_buffer_list_ptr->find( kernel_ptr ) != pState->state_buffer_list_ptr->end() )
    {
        return ( *pState->state_buffer_list_ptr )[ kernel_ptr ].state_buffer_size;
    }
    else
    {
        return 0;
    }
}

CM_STATE_BUFFER_TYPE HalCm_GetStateBufferTypeForKernel(
    PCM_HAL_STATE               pState,
    void                        *kernel_ptr )
{
    if ( pState->state_buffer_list_ptr->find( kernel_ptr ) != pState->state_buffer_list_ptr->end() )
    {
        return ( *pState->state_buffer_list_ptr )[ kernel_ptr ].state_buffer_type;
    }
    else
    {
        return CM_STATE_BUFFER_NONE;
    }
}

MOS_STATUS HalCm_CreateGPUContext(
    PCM_HAL_STATE   pState,
    MOS_GPU_CONTEXT gpu_context,
    MOS_GPU_NODE    gpu_node)
{
    MOS_STATUS hr = MOS_STATUS_SUCCESS;
    unsigned int  numCmdBuffers = pState->CmDeviceParam.iMaxTasks;

    // Create Compute Context on Compute Node
    CM_HRESULT2MOSSTATUS_AND_CHECK(pState->pOsInterface->pfnCreateGpuContext(
        pState->pOsInterface,
        gpu_context,
        gpu_node,
        numCmdBuffers));

    // Register Compute Context with the Batch Buffer completion event
    CM_HRESULT2MOSSTATUS_AND_CHECK(pState->pOsInterface->pfnRegisterBBCompleteNotifyEvent(
        pState->pOsInterface,
        gpu_context));

finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Creates instance of HAL CM State
//| Returns:    Result of the operation
//| Note:       Caller must call pfnAllocate to allocate all HalCm/Mhw states and objects.
//|             Caller MUST call HalCm_Destroy to destroy the instance
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_Create(
    PMOS_CONTEXT            pOsDriverContext,   // [in] OS Driver Context
    PCM_HAL_CREATE_PARAM     pParam,             // [in] Create Param
    PCM_HAL_STATE           *pCmState)          // [out] double pointer to CM State
{
    MOS_STATUS          hr;
    PCM_HAL_STATE       pState = nullptr;
    uint32_t            numCmdBuffers = 0;
    MhwInterfaces       *mhwInterfaces = nullptr;
    MhwInterfaces::CreateParams params;

    //-----------------------------------------
    CM_ASSERT(pOsDriverContext);
    CM_ASSERT(pParam);
    CM_ASSERT(pCmState);
    //-----------------------------------------

    hr  = MOS_STATUS_SUCCESS;

    // Allocate State structure
    pState = (PCM_HAL_STATE)MOS_AllocAndZeroMemory(sizeof(CM_HAL_STATE));
    CM_CHK_NULL_RETURN_MOSSTATUS(pState);

    // Allocate/Initialize OS Interface
    pState->pOsInterface = (PMOS_INTERFACE)
                                MOS_AllocAndZeroMemory(sizeof(MOS_INTERFACE));
    CM_CHK_NULL_RETURN_MOSSTATUS(pState->pOsInterface);
    pState->pOsInterface->bDeallocateOnExit = true;
    CM_HRESULT2MOSSTATUS_AND_CHECK(Mos_InitInterface(pState->pOsInterface, pOsDriverContext, COMPONENT_CM));

    pState->pOsInterface->pfnGetPlatform(pState->pOsInterface, &pState->Platform);
    pState->pSkuTable = pState->pOsInterface->pfnGetSkuTable(pState->pOsInterface);
    pState->pWaTable  = pState->pOsInterface->pfnGetWaTable (pState->pOsInterface);

    //GPU context
    pState->GpuContext =   pParam->bRequestCustomGpuContext? MOS_GPU_CONTEXT_RENDER4 : MOS_GPU_CONTEXT_RENDER3;

    numCmdBuffers = HalCm_GetNumCmdBuffers(pState->pOsInterface, pParam->MaxTaskNumber);

    // Create Render GPU Context
    CM_HRESULT2MOSSTATUS_AND_CHECK(pState->pOsInterface->pfnCreateGpuContext(
        pState->pOsInterface,
        pState->GpuContext,
        MOS_GPU_NODE_3D,
        numCmdBuffers));

    // Set current GPU context
    CM_HRESULT2MOSSTATUS_AND_CHECK(pState->pOsInterface->pfnSetGpuContext(
        pState->pOsInterface,
        pState->GpuContext));

    // Register Render GPU context with the event
    CM_HRESULT2MOSSTATUS_AND_CHECK(pState->pOsInterface->pfnRegisterBBCompleteNotifyEvent(
        pState->pOsInterface,
        pState->GpuContext));

    // Create VEBOX Context
    CM_HRESULT2MOSSTATUS_AND_CHECK(pState->pOsInterface->pfnCreateGpuContext(
        pState->pOsInterface,
        MOS_GPU_CONTEXT_VEBOX,
        MOS_GPU_NODE_VE,
        MOS_GPU_CONTEXT_CREATE_DEFAULT));

    // Register Vebox GPU context with the Batch Buffer completion event
    CM_HRESULT2MOSSTATUS_AND_CHECK(pState->pOsInterface->pfnRegisterBBCompleteNotifyEvent(
        pState->pOsInterface,
        MOS_GPU_CONTEXT_VEBOX));

    // Allocate/Initialize CM Rendering Interface
    pState->pRenderHal = (PRENDERHAL_INTERFACE)
                                MOS_AllocAndZeroMemory(sizeof(RENDERHAL_INTERFACE));
    CM_CHK_NULL_RETURN_MOSSTATUS(pState->pRenderHal);

    pState->bDynamicStateHeap             = pParam->bDynamicStateHeap;
    pState->pRenderHal->bDynamicStateHeap = pState->bDynamicStateHeap;

    if (pState->bDynamicStateHeap)
    {
        CM_CHK_MOSSTATUS(RenderHal_InitInterface_Dynamic(pState->pRenderHal, &pState->pCpInterface, pState->pOsInterface));
    }
    else
    {
        CM_CHK_MOSSTATUS(RenderHal_InitInterface(pState->pRenderHal, &pState->pCpInterface, pState->pOsInterface));
    }

    // Allocate/Initialize VEBOX Interface
    CmSafeMemSet(&params, 0, sizeof(params));
    params.Flags.m_vebox = 1;
    mhwInterfaces = MhwInterfaces::CreateFactory(params, pState->pOsInterface);
    if (mhwInterfaces)
    {
        pState->pVeboxInterface = mhwInterfaces->m_veboxInterface;

        // MhwInterfaces always create CP and MI interfaces, so we have to delete those we don't need. 
        MOS_Delete(mhwInterfaces->m_miInterface);
        MOS_Delete(mhwInterfaces->m_cpInterface);
        MOS_Delete(mhwInterfaces);
    }
    else
    {
        CM_ASSERTMESSAGE("Allocate MhwInterfaces failed");
        return MOS_STATUS_NO_SPACE;
    }

    // set IsMDFLoad to distinguish MDF context from other Media Contexts
    pState->pRenderHal->IsMDFLoad = true;

    // disable YV12SinglePass as CMRT & compiler don't support it
    pState->pRenderHal->bEnableYV12SinglePass = false;

    pState->CmDeviceParam.iMaxKernelBinarySize      = CM_KERNEL_BINARY_BLOCK_SIZE;

    // set if the new sampler heap management is used or not
    // currently new sampler heap management depends on DSH
    if (pState->bDynamicStateHeap) 
    {
        pState->use_new_sampler_heap = true;
    } 
    else 
    {
        pState->use_new_sampler_heap = false;
    }

    //Get Max Scratch Space Size
    if( pParam->DisableScratchSpace)
    {
        pState->CmDeviceParam.iMaxPerThreadScratchSpaceSize = 0; 
    }
    else
    {
         //Gen7_5 + : (MaxScratchSpaceSize + 1) *16k
          if(pParam->ScratchSpaceSize == CM_DEVICE_CONFIG_SCRATCH_SPACE_SIZE_DEFAULT)
          { //By default, 128K for HSW
               pState->CmDeviceParam.iMaxPerThreadScratchSpaceSize = 8 * CM_DEVICE_CONFIG_SCRATCH_SPACE_SIZE_16K_STEP;
          }
          else
          {
               pState->CmDeviceParam.iMaxPerThreadScratchSpaceSize = (pParam->ScratchSpaceSize)*
                                CM_DEVICE_CONFIG_SCRATCH_SPACE_SIZE_16K_STEP;
          }
    }

    // Initialize kernel parameters
    pState->KernelParams_RenderHal.pMhwKernelParam = &pState->KernelParams_Mhw;

    // Enable SLM in L3 Cache
    pState->l3_settings.enable_slm = true;

    // Slice shutdown
    pState->bRequestSingleSlice = pParam->bRequestSliceShutdown;

    //mid thread preemption on/off and SIP debug control
    pState->bDisabledMidThreadPreemption = pParam->bDisabledMidThreadPreemption;
    pState->bEnabledKernelDebug = pParam->bEnabledKernelDebug;

    // init mapping for the state buffer
#if MOS_MESSAGES_ENABLED
    pState->state_buffer_list_ptr = MOS_NewUtil<std::map< void *, CM_HAL_STATE_BUFFER_ENTRY> >(__FUNCTION__, __FILE__, __LINE__);
#else 
    pState->state_buffer_list_ptr = MOS_NewUtil<std::map< void *, CM_HAL_STATE_BUFFER_ENTRY> >();
#endif

    CM_CHK_NULL_RETURN_MOSSTATUS( pState->state_buffer_list_ptr );

    MOS_ZeroMemory(&pState->HintIndexes.iKernelIndexes, sizeof(uint32_t) * CM_MAX_TASKS_EU_SATURATION);
    MOS_ZeroMemory(&pState->HintIndexes.iDispatchIndexes, sizeof(uint32_t) * CM_MAX_TASKS_EU_SATURATION);

#if USE_EXTENSION_CODE
    pState->bMockRuntimeEnabled = pParam->bMockRuntimeEnabled;
#endif

    pState->CmDeviceParam.iMaxKernelsPerTask        = CM_MAX_KERNELS_PER_TASK;
    pState->CmDeviceParam.iMaxSamplerTableSize      = CM_MAX_SAMPLER_TABLE_SIZE;
    pState->CmDeviceParam.iMaxSampler8x8TableSize   = pState->pRenderHal->pHwSizes->dwSizeSampler8x8Table;
    pState->CmDeviceParam.iMaxBufferTableSize       = CM_MAX_BUFFER_SURFACE_TABLE_SIZE;
    pState->CmDeviceParam.iMax2DSurfaceUPTableSize  = CM_MAX_2D_SURFACE_UP_TABLE_SIZE;
    pState->CmDeviceParam.iMax2DSurfaceTableSize    = CM_MAX_2D_SURFACE_TABLE_SIZE;
    pState->CmDeviceParam.iMax3DSurfaceTableSize    = CM_MAX_3D_SURFACE_TABLE_SIZE;
    pState->CmDeviceParam.iMaxTasks                 = pParam->MaxTaskNumber;
    pState->CmDeviceParam.iMaxAVSSamplers           = CM_MAX_AVS_SAMPLER_SIZE;
    pState->CmDeviceParam.iMaxGSHKernelEntries      = pParam->KernelBinarySizeinGSH / (CM_32K);

    if (pState->bDynamicStateHeap)
    {
        // Initialize Kernel Cache Hit/Miss counters
        pState->dwDSHKernelCacheMiss = 0;
        pState->dwDSHKernelCacheHit  = 0;
    }

    // Setup Function pointers
    pState->pfnCmAllocate                  = HalCm_Allocate;
    pState->pfnGetMaxValues                = HalCm_GetMaxValues;
    pState->pfnGetMaxValuesEx              = HalCm_GetMaxValuesEx;
    pState->pfnExecuteTask                 = HalCm_ExecuteTask;
    pState->pfnExecuteGroupTask            = HalCm_ExecuteGroupTask;
    pState->pfnExecuteHintsTask            = HalCm_ExecuteHintsTask;
    pState->pfnRegisterSampler             = HalCm_RegisterSampler;
    pState->pfnUnRegisterSampler           = HalCm_UnRegisterSampler;
    pState->pfnRegisterSampler8x8          = HalCm_RegisterSampler8x8; 
    pState->pfnUnRegisterSampler8x8        = HalCm_UnRegisterSampler8x8;
    pState->pfnFreeBuffer                  = HalCm_FreeBuffer;
    pState->pfnLockBuffer                  = HalCm_LockBuffer;
    pState->pfnUnlockBuffer                = HalCm_UnlockBuffer;
    pState->pfnFreeSurface2DUP             = HalCm_FreeSurface2DUP;
    pState->pfnGetSurface2DTileYPitch      = HalCm_GetSurface2DTileYPitch;
    pState->pfnSet2DSurfaceStateParam      = HalCm_Set2DSurfaceStateParam;
    pState->pfnSetBufferSurfaceStatePara   = HalCm_SetBufferSurfaceStateParameters;
    pState->pfnSetSurfaceMOCS              = HalCm_SetSurfaceMOCS;
    /************************************************************/
    pState->pfnAllocateSurface2D           = HalCm_AllocateSurface2D;
    pState->pfnFreeSurface2D               = HalCm_FreeSurface2D;
    pState->pfnLock2DResource              = HalCm_Lock2DResource;
    pState->pfnUnlock2DResource            = HalCm_Unlock2DResource;
    pState->pfnSetCompressionMode          = HalCm_SetCompressionMode;
    /************************************************************/
    pState->pfnFree3DResource              = HalCm_Free3DResource;
    pState->pfnLock3DResource              = HalCm_Lock3DResource;
    pState->pfnUnlock3DResource            = HalCm_Unlock3DResource;
    pState->pfnSetCaps                     = HalCm_SetCaps;
    pState->pfnSetPowerOption              = HalCm_SetPowerOption;
    pState->pfnUpdatePowerOption           = HalCm_UpdatePowerOption;

    pState->pfnSendMediaWalkerState        = HalCm_SendMediaWalkerState;
    pState->pfnSendGpGpuWalkerState        = HalCm_SendGpGpuWalkerState;
    pState->pfnSetSurfaceReadFlag          = HalCm_SetSurfaceReadFlag;
    pState->pfnSetVtuneProfilingFlag       = HalCm_SetVtuneProfilingFlag;
    pState->pfnExecuteVeboxTask            = HalCm_ExecuteVeboxTask;
    pState->pfnGetSipBinary                = HalCm_GetSipBinary;
    pState->pfnGetTaskSyncLocation         = HalCm_GetTaskSyncLocation;

    pState->pfnGetGlobalTime               = HalCm_GetGlobalTime;
    pState->pfnConvertToQPCTime            = HalCm_ConvertToQPCTime;  

    pState->pfnInsertToStateBufferList = HalCm_InsertToStateBufferList;
    pState->pfnDeleteFromStateBufferList = HalCm_DeleteFromStateBufferList;
    pState->pfnGetMediaStatePtrForKernel = HalCm_GetMediaStatePtrForKernel;
    pState->pfnGetStateBufferVAPtrForSurfaceIndex = HalCm_GetStateBufferVAPtrForSurfaceIndex;
    pState->pfnGetMediaStatePtrForSurfaceIndex = HalCm_GetMediaStatePtrForSurfaceIndex;
    pState->pfnGetStateBufferVAPtrForMediaStatePtr = HalCm_GetStateBufferVAPtrForMediaStatePtr;
    pState->pfnGetStateBufferSizeForKernel = HalCm_GetStateBufferSizeForKernel;
    pState->pfnGetStateBufferTypeForKernel = HalCm_GetStateBufferTypeForKernel;
    pState->pfnCreateGPUContext            = HalCm_CreateGPUContext;
    pState->pfnDSHUnregisterKernel         = HalCm_DSH_UnregisterKernel;

    //==========<Initialize 5 OS-dependent DDI functions: pfnAllocate3DResource, pfnAllocateSurface2DUP====
    //                 pfnAllocateBuffer,pfnRegisterKMDNotifyEventHandle, pfnGetSurface2DPitchAndSize >==== 
    HalCm_OsInitInterface(pState);

    HalCm_InitPerfTagIndexMap(pState);

    pState->MaxHWThreadValues.userFeatureValue = 0;
    pState->MaxHWThreadValues.APIValue = 0;

    HalCm_GetUserFeatureSettings(pState);

#if MDF_COMMAND_BUFFER_DUMP
    HalCm_InitDumpCommandBuffer(pState);
    pState->pfnInitDumpCommandBuffer = HalCm_InitDumpCommandBuffer;
    pState->pfnDumpCommadBuffer      = HalCm_DumpCommadBuffer;
#endif //MDF_COMMAND_BUFFER_DUMP

#if MDF_CURBE_DATA_DUMP
    HalCm_InitDumpCurbeData(pState);
#endif

#if MDF_SURFACE_CONTENT_DUMP
    HalCm_InitSurfaceDump(pState);
#endif

    pState->pCmHalInterface = CMHalDevice::CreateFactory(pState);

finish:
    if (hr != MOS_STATUS_SUCCESS)
    {
        HalCm_Destroy(pState);
        *pCmState = nullptr;
    }
    else 
    {
        *pCmState = pState;
    }

    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose: Destroys instance of HAL CM State
//| Returns: N/A
//*-----------------------------------------------------------------------------
void HalCm_Destroy(
    PCM_HAL_STATE pState)                                                       // [in] Pointer to CM State
{
    MOS_STATUS hr;
    int32_t    i;

    if (pState)
    {
        //Delete CmHal Interface
        MosSafeDelete(pState->pCmHalInterface);
        MosSafeDelete(pState->pCpInterface);
        MosSafeDelete(pState->state_buffer_list_ptr);

        // Delete Batch Buffers
        if (pState->pBatchBuffers)
        {
            for (i=0; i < pState->iNumBatchBuffers; i++)
            {
                if (!Mos_ResourceIsNull(&pState->pBatchBuffers[i].OsResource))
                {
                    hr = (MOS_STATUS)pState->pRenderHal->pfnFreeBB(
                                pState->pRenderHal,
                                &pState->pBatchBuffers[i]);

                    CM_ASSERT(hr == MOS_STATUS_SUCCESS);
                }

                MOS_FreeMemory(pState->pBatchBuffers[i].pPrivateData);
            }

            MOS_FreeMemory(pState->pBatchBuffers);
            pState->pBatchBuffers = nullptr;
        }

        // Delete TimeStamp Buffer
        HalCm_FreeTsResource(pState);
        if ((pState->bDisabledMidThreadPreemption == false) || (pState->bEnabledKernelDebug == true)) {
            // Delete CSR surface
            HalCm_FreeCsrResource(pState);

            // Delete sip surface
            HalCm_FreeSipResource(pState);

        }
        if (pState->hLibModule)
        {
            MOS_FreeLibrary(pState->hLibModule);
            pState->hLibModule = nullptr;
        }

        // Delete RenderHal Interface
        if (pState->pRenderHal)
        {
            pState->pRenderHal->pfnDestroy(pState->pRenderHal);
            MOS_FreeMemory(pState->pRenderHal);
            pState->pRenderHal = nullptr;
        }

        // Delete VEBOX Interface
        if (pState->pVeboxInterface
            && pState->pVeboxInterface->m_veboxHeap)
        {
            pState->pVeboxInterface->DestroyHeap( );
            MOS_Delete(pState->pVeboxInterface);
            pState->pVeboxInterface = nullptr;
        }

        // Delete OS Interface
        if (pState->pOsInterface)
        {
            pState->pOsInterface->pfnDestroy(pState->pOsInterface, true);
            if (pState->pOsInterface->bDeallocateOnExit)
            {
                MOS_FreeMemory(pState->pOsInterface);
                pState->pOsInterface = nullptr;
            }
        }

        // Delete the TaskParam
        MOS_FreeMemory(pState->pTaskParam);

        // Delete the TaskTimeStamp
        MOS_FreeMemory(pState->pTaskTimeStamp);

        // Delete Tables
        MOS_FreeMemory(pState->pTableMem);

        // Delete the pTotalKernelSize table for GSH
        MOS_FreeMemory(pState->pTotalKernelSize);

        // Delete the perfTag Map
        for (int i = 0; i < MAX_COMBINE_NUM_IN_PERFTAG; i++)
        {
            MosSafeDelete(pState->pPerfTagIndexMap[i]);
        }

        // Delete the state
        MOS_FreeMemory(pState);
    }
}

void HalCm_GetUserFeatureSettings(
    PCM_HAL_STATE                  pCmState
)
{
#if (_DEBUG || _RELEASE_INTERNAL)
    PMOS_INTERFACE                  pOsInterface;
    PMOS_USER_FEATURE_INTERFACE     pUserFeatureInterface;
    MOS_USER_FEATURE                UserFeature;
    MOS_USER_FEATURE_VALUE          UserFeatureValue;

    MOS_ZeroMemory(&UserFeatureValue, sizeof(UserFeatureValue));
    pOsInterface            = pCmState->pOsInterface;
    pUserFeatureInterface   = &pOsInterface->UserFeatureInterface;
    UserFeature             = *pUserFeatureInterface->pUserFeatureInit;
    UserFeature.Type        = MOS_USER_FEATURE_TYPE_USER;
    UserFeature.pPath       = (char *)__MEDIA_USER_FEATURE_SUBKEY_INTERNAL;
    UserFeature.pValues     = &UserFeatureValue;
    UserFeature.uiNumValues = 1;

    if (pUserFeatureInterface->pfnReadValue(
          pUserFeatureInterface,
          &UserFeature,
          (char *)VPHAL_CM_MAX_THREADS,
          MOS_USER_FEATURE_VALUE_TYPE_UINT32) == MOS_STATUS_SUCCESS)
    {
        uint32_t uiData = UserFeature.pValues[0].u32Data;
        if ((uiData > 0) && (uiData <= pCmState->pRenderHal->pHwCaps->dwMaxThreads))
        {
            pCmState->MaxHWThreadValues.userFeatureValue = uiData;
        }
    }
#else
    UNUSED(pCmState);
#endif // _DEBUG || _RELEASE_INTERNAL
}

//*-----------------------------------------------------------------------------
//| Purpose: Gathers information about the surface - used by GT-Pin
//| Returns: MOS_STATUS_SUCCESS if surface type recognized, S_FAIL otherwise
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_GetSurfaceDetails(
    PCM_HAL_STATE                  pCmState,
    PCM_HAL_INDEX_PARAM            pIndexParam,
    uint32_t                       iBTIndex,
    MOS_SURFACE&                   Surface,
    int16_t                        globalSurface,
    PRENDERHAL_SURFACE_STATE_ENTRY pSurfaceEntry,
    uint32_t                       dwTempPlaneIndex,
    RENDERHAL_SURFACE_STATE_PARAMS SurfaceParam,
    CM_HAL_KERNEL_ARG_KIND         argKind
    )
{
    MOS_STATUS                 hr             = MOS_STATUS_UNKNOWN;
    PCM_SURFACE_DETAILS        pSurfaceInfos  = nullptr;
    PCM_SURFACE_DETAILS        pgSurfaceInfos = nullptr;
    PCM_HAL_TASK_PARAM         pTaskParam     = pCmState->pTaskParam;
    uint32_t                   iCurKrnIndex   = pTaskParam->iCurKrnIndex;
    PMOS_PLANE_OFFSET          pPlaneOffset   = 0;
    uint32_t                   maxEntryNum    = 0;
    MOS_OS_FORMAT              tempOsFormat   ;

    CM_SURFACE_BTI_INFO SurfBTIInfo;
    pCmState->pCmHalInterface->GetHwSurfaceBTIInfo(&SurfBTIInfo);
    
    UNUSED(pIndexParam);

    if(iCurKrnIndex+1>pTaskParam->SurEntryInfoArrays.dwKrnNum)
    {
        CM_ERROR_ASSERT(
            "Mismatched kernel index: iCurKrnIndex '%d' vs dwKrnNum '%d'", 
            iCurKrnIndex,pTaskParam->SurEntryInfoArrays.dwKrnNum);
        goto finish;
    }

    pSurfaceInfos  = pTaskParam->SurEntryInfoArrays.pSurfEntryInfosArray[iCurKrnIndex].pSurfEntryInfos;
    pgSurfaceInfos = pTaskParam->SurEntryInfoArrays.pSurfEntryInfosArray[iCurKrnIndex].pGlobalSurfInfos;

    tempOsFormat   = pCmState->pOsInterface->pfnFmt_MosToOs(Surface.Format);

    switch (argKind)
    {
    case CM_ARGUMENT_SURFACEBUFFER:

        if((iBTIndex >= SurfBTIInfo.dwReservedSurfaceStart) &&
            (iBTIndex < SurfBTIInfo.dwReservedSurfaceStart + CM_MAX_GLOBAL_SURFACE_NUMBER))
        {
            iBTIndex = iBTIndex - SurfBTIInfo.dwReservedSurfaceStart;

            maxEntryNum = pTaskParam->SurEntryInfoArrays.pSurfEntryInfosArray->dwGlobalSurfNum;
            if ( iBTIndex >= maxEntryNum )
            {
                CM_ERROR_ASSERT(
                "Array for surface details is full: Max number of entries '%d' and trying to add index '%d'",
                maxEntryNum, iBTIndex);
                goto finish;
            }

            MOS_ZeroMemory(&pgSurfaceInfos[iBTIndex], sizeof(CM_SURFACE_DETAILS));
            pgSurfaceInfos[iBTIndex].dwWidth  = Surface.dwWidth;
            pgSurfaceInfos[iBTIndex].dwFormat = DDI_FORMAT_UNKNOWN;
        }
        else
        {
            iBTIndex = iBTIndex - SurfBTIInfo.dwReservedSurfaceStart;
            maxEntryNum = pTaskParam->SurEntryInfoArrays.pSurfEntryInfosArray->dwMaxEntryNum;
            if ( iBTIndex >= maxEntryNum )
            {
                CM_ERROR_ASSERT(
                "Array for surface details is full: Max number of entries '%d' and trying to add index '%d'",
                maxEntryNum, iBTIndex);
                goto finish;
            }

            MOS_ZeroMemory(&pSurfaceInfos[iBTIndex], sizeof(CM_SURFACE_DETAILS));
            pSurfaceInfos[iBTIndex].dwWidth  = Surface.dwWidth;
            pSurfaceInfos[iBTIndex].dwFormat = DDI_FORMAT_UNKNOWN;
        }

        if (globalSurface < 0) 
        {
            ++pTaskParam->SurEntryInfoArrays.pSurfEntryInfosArray[iCurKrnIndex].dwUsedIndex;
        }

        hr = MOS_STATUS_SUCCESS;
        break;

    case CM_ARGUMENT_SURFACE2D_UP:
    case CM_ARGUMENT_SURFACE2D:
    // VME surface and sampler8x8 called with CM_ARGUMENT_SURFACE2D

         iBTIndex = iBTIndex - SurfBTIInfo.dwReservedSurfaceStart;
         maxEntryNum = pTaskParam->SurEntryInfoArrays.pSurfEntryInfosArray->dwMaxEntryNum;

         if ( iBTIndex >= maxEntryNum )
         {
             CM_ERROR_ASSERT(
             "Array for surface details is full: Max number of entries '%d' and trying to add index '%d'",
             maxEntryNum, iBTIndex);
             goto finish;
         }

         pSurfaceInfos[iBTIndex].dwWidth              = pSurfaceEntry->dwWidth;
         pSurfaceInfos[iBTIndex].dwHeight             = pSurfaceEntry->dwHeight;
         pSurfaceInfos[iBTIndex].dwDepth              = 0;
         pSurfaceInfos[iBTIndex].dwFormat             = (DdiSurfaceFormat)tempOsFormat;
         pSurfaceInfos[iBTIndex].dwPlaneIndex         = dwTempPlaneIndex;
         pSurfaceInfos[iBTIndex].dwPitch              = pSurfaceEntry->dwPitch;
         pSurfaceInfos[iBTIndex].dwSlicePitch         = 0;
         pSurfaceInfos[iBTIndex].dwSurfaceBaseAddress = 0;
         pSurfaceInfos[iBTIndex].u8TileWalk           = pSurfaceEntry->bTileWalk;
         pSurfaceInfos[iBTIndex].u8TiledSurface       = pSurfaceEntry->bTiledSurface;

         if (pSurfaceEntry->YUVPlane == MHW_U_PLANE || 
             pSurfaceEntry->YUVPlane == MHW_V_PLANE)
         {
             pPlaneOffset = (pSurfaceEntry->YUVPlane == MHW_U_PLANE) 
             ? &Surface.UPlaneOffset 
             : &Surface.VPlaneOffset;

             pSurfaceInfos[iBTIndex].dwYOffset = pPlaneOffset->iYOffset >> 1;

             if ( argKind == CM_ARGUMENT_SURFACE2D_UP )
             {
                 pSurfaceInfos[iBTIndex].dwXOffset = (pPlaneOffset->iXOffset/(uint32_t)sizeof(uint32_t)) >> 2;
             }
             else
             {
                 uint32_t dwPixelsPerSampleUV = 0;
                 //Get Pixels Per Sample if we use dataport read
                 if(SurfaceParam.bWidthInDword_UV)
                 {
                     RenderHal_GetPixelsPerSample(Surface.Format, &dwPixelsPerSampleUV);
                 }
                 else
                 {
                     // If the kernel uses sampler - do not change width (it affects coordinates)
                     dwPixelsPerSampleUV = 1;
                 }

                 if(dwPixelsPerSampleUV == 1)
                 {
                     pSurfaceInfos[iBTIndex].dwXOffset = pPlaneOffset->iXOffset >> 2;
                 }
                 else
                 {
                     pSurfaceInfos[iBTIndex].dwXOffset = (pPlaneOffset->iXOffset/(uint32_t)sizeof(uint32_t)) >> 2;
                 }
             }
         }
         else
         {
             pSurfaceInfos[iBTIndex].dwXOffset = (Surface.YPlaneOffset.iXOffset/(uint32_t)sizeof(uint32_t)) >> 2;
             pSurfaceInfos[iBTIndex].dwYOffset = Surface.YPlaneOffset.iYOffset >> 1;
         } 

         ++pTaskParam->SurEntryInfoArrays.pSurfEntryInfosArray[iCurKrnIndex].dwUsedIndex;
         ++dwTempPlaneIndex;

         hr = MOS_STATUS_SUCCESS;
         break;

    case CM_ARGUMENT_SURFACE3D:

        iBTIndex = iBTIndex - SurfBTIInfo.dwNormalSurfaceStart;
        maxEntryNum = pTaskParam->SurEntryInfoArrays.pSurfEntryInfosArray->dwMaxEntryNum;

        if ( iBTIndex >= maxEntryNum )
        {
            CM_ERROR_ASSERT(
            "Array for surface details is full: Max number of entries '%d' and trying to add index '%d'",
            maxEntryNum, iBTIndex);
            goto finish;
        }

        pSurfaceInfos[iBTIndex].dwWidth              = pSurfaceEntry->dwWidth;
        pSurfaceInfos[iBTIndex].dwHeight             = pSurfaceEntry->dwHeight;
        pSurfaceInfos[iBTIndex].dwDepth              = Surface.dwDepth;
        pSurfaceInfos[iBTIndex].dwFormat             = (DdiSurfaceFormat)tempOsFormat;
        pSurfaceInfos[iBTIndex].dwPitch              = pSurfaceEntry->dwPitch;
        pSurfaceInfos[iBTIndex].dwPlaneIndex         = dwTempPlaneIndex;
        pSurfaceInfos[iBTIndex].dwSlicePitch         = Surface.dwSlicePitch;
        pSurfaceInfos[iBTIndex].dwSurfaceBaseAddress = 0;
        pSurfaceInfos[iBTIndex].u8TileWalk           = pSurfaceEntry->bTileWalk;
        pSurfaceInfos[iBTIndex].u8TiledSurface       = pSurfaceEntry->bTiledSurface;

        if (pSurfaceEntry->YUVPlane == MHW_U_PLANE || 
            pSurfaceEntry->YUVPlane == MHW_V_PLANE)
        {
            pPlaneOffset = (pSurfaceEntry->YUVPlane == MHW_U_PLANE) 
            ? &Surface.UPlaneOffset 
            : &Surface.VPlaneOffset;

            pSurfaceInfos[iBTIndex].dwYOffset = pPlaneOffset->iYOffset >> 1;
            pSurfaceInfos[iBTIndex].dwXOffset = (pPlaneOffset->iXOffset/(uint32_t)sizeof(uint32_t)) >> 2;
        }
        else
        { 
            pSurfaceInfos[iBTIndex].dwXOffset = (Surface.YPlaneOffset.iXOffset/(uint32_t)sizeof(uint32_t)) >> 2;
            pSurfaceInfos[iBTIndex].dwYOffset = Surface.YPlaneOffset.iYOffset >> 1;
        }

        ++dwTempPlaneIndex;
        ++pTaskParam->SurEntryInfoArrays.pSurfEntryInfosArray[iCurKrnIndex].dwUsedIndex;

        hr = MOS_STATUS_SUCCESS;
        break;

    default:
        break;
    }

 finish:
        return hr;
}

uint32_t HalCm_GetFreeBindingIndex(
    PCM_HAL_STATE             pState,
    PCM_HAL_INDEX_PARAM       pIndexParam,
    uint32_t                  total)
{
    CM_SURFACE_BTI_INFO SurfBTIInfo;
    pState->pCmHalInterface->GetHwSurfaceBTIInfo(&SurfBTIInfo);
    
    uint32_t bt_index = SurfBTIInfo.dwNormalSurfaceStart;
    uint32_t un_allocated = total;
    
    while (bt_index < 256 && un_allocated > 0)
    {
        uint32_t array_index = bt_index >> 5;
        uint32_t bit_mask = 1 << (bt_index % 32);
        if (pIndexParam->dwBTArray[array_index] & bit_mask)
        {
            // oops, occupied
            if (un_allocated != total)
            {
                // clear previous allocation
                uint32_t allocated = total - un_allocated;
                uint32_t tmp_index = bt_index - 1;
                while (allocated > 0)
                {
                    uint32_t array_index = tmp_index >> 5;
                    uint32_t bit_mask = 1 << (tmp_index % 32);
                    pIndexParam->dwBTArray[array_index] &= ~bit_mask;
                    allocated--;
                    tmp_index--;
                }
                // reset
                un_allocated = total;
            }
        }
        else
        {
            pIndexParam->dwBTArray[array_index] |= bit_mask;
            un_allocated--;
        }
        bt_index++;
    }

    if (un_allocated == 0)
    {
        // found slot
        return bt_index - total;
    }

    // no slot
    return 0;
}

void HalCm_PreSetBindingIndex(
    PCM_HAL_INDEX_PARAM     pIndexParam,
    uint32_t                start,
    uint32_t                end)
{
    uint32_t bt_index;
    for ( bt_index = start; bt_index <= end ; bt_index++)
    {
        uint32_t array_index = bt_index >> 5;
        uint32_t bit_mask = 1 << (bt_index % 32);
        pIndexParam->dwBTArray[array_index] |= bit_mask;
    }
}

//*-----------------------------------------------------------------------------
//| Purpose: Setup Surface State with BTIndex
//| Returns: Result of the operation
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_Setup2DSurfaceStateWithBTIndex(
    PCM_HAL_STATE                      pState,
    int32_t                            iBindingTable,
    uint32_t                           surfIndex,
    uint32_t                           btIndex,
    bool                               pixelPitch)
{
    PRENDERHAL_INTERFACE            pRenderHal = pState->pRenderHal;
    MOS_STATUS                  hr;
    RENDERHAL_SURFACE               Surface;
    RENDERHAL_SURFACE_STATE_PARAMS  SurfaceParam;
    PRENDERHAL_SURFACE_STATE_ENTRY  pSurfaceEntries[MHW_MAX_SURFACE_PLANES];
    int32_t                     iSurfaceEntries, i;
    uint16_t                    memObjCtl;
    uint32_t                    dwOffsetSrc;
    PRENDERHAL_STATE_HEAP       pStateHeap;

    hr              = MOS_STATUS_UNKNOWN;
    iSurfaceEntries = 0;

    if (surfIndex == CM_NULL_SURFACE)
    {
        return MOS_STATUS_SUCCESS;
    }

    memObjCtl = CM_DEFAULT_CACHE_TYPE;


    // check the surfIndex
    if (surfIndex >= pState->CmDeviceParam.iMax2DSurfaceTableSize ||
        Mos_ResourceIsNull(&pState->pUmdSurf2DTable[surfIndex].OsResource) )
    {
        CM_ERROR_ASSERT(
            "Invalid 2D Surface array index '%d'", surfIndex);
        return MOS_STATUS_UNKNOWN;
    }

    // Check to see if surface is already assigned
    uint32_t nBTInTable = ( unsigned char )CM_INVALID_INDEX;
    if ( pixelPitch )
    {
        nBTInTable = pState->pBT2DIndexTable[ surfIndex ].BTI.SamplerSurfIndex;
    }
    else
    {
        nBTInTable = pState->pBT2DIndexTable[ surfIndex ].BTI.RegularSurfIndex;
    }

    if ( btIndex == nBTInTable )
    {
        iSurfaceEntries = pState->pBT2DIndexTable[ surfIndex ].nPlaneNumber;

        pStateHeap = pRenderHal->pStateHeap;

        // Get Offset to Current Binding Table 
        uint32_t dwOffsetDst = ( pStateHeap->iCurSshBufferIndex * pStateHeap->dwSshIntanceSize ) +     // Points to the Base of Current SSH Buffer Instance
                            ( pStateHeap->iBindingTableOffset ) +                       // Moves the pointer to Base of Array of Binding Tables
                            ( iBindingTable * pStateHeap->iBindingTableSize ) +         // Moves the pointer to a Particular Binding Table
                            ( btIndex * sizeof( uint32_t ) );                              // Move the pointer to correct entry

        uint32_t *pBindingTableEntry = ( uint32_t *)( pStateHeap->pSshBuffer + dwOffsetDst );

        if ( pixelPitch )
        {
            MOS_SecureMemcpy( pBindingTableEntry, sizeof( uint32_t ) * iSurfaceEntries, pState->pBT2DIndexTable[ surfIndex ].BTITableEntry.SamplerBTIEntryPos, sizeof( uint32_t ) * iSurfaceEntries );
        }
        else
        {
            MOS_SecureMemcpy( pBindingTableEntry, sizeof( uint32_t ) * iSurfaceEntries, pState->pBT2DIndexTable[ surfIndex ].BTITableEntry.RegularBTIEntryPos, sizeof( uint32_t ) * iSurfaceEntries );
        }

        return MOS_STATUS_SUCCESS;
    }

    // Get Details of 2D surface and fill the Surface 
    CM_CHK_MOSSTATUS(HalCm_GetSurfaceAndRegister(pState, &Surface, CM_ARGUMENT_SURFACE2D, surfIndex, pixelPitch));

    // Setup 2D surface
    MOS_ZeroMemory(&SurfaceParam, sizeof(SurfaceParam));
    SurfaceParam.Type       = pRenderHal->SurfaceTypeDefault;
    SurfaceParam.Boundary   = RENDERHAL_SS_BOUNDARY_ORIGINAL;
    if (!pixelPitch) {
        SurfaceParam.bWidthInDword_UV = true;
        SurfaceParam.bWidthInDword_Y = true;
    }
    
    SurfaceParam.bRenderTarget = isRenderTarget(pState, surfIndex);

    //Cache configurations
    pState->pCmHalInterface->HwSetSurfaceMemoryObjectControl(memObjCtl, &SurfaceParam);

    CM_CHK_MOSSTATUS(pRenderHal->pfnSetupSurfaceState(
                  pRenderHal, 
                  &Surface, 
                  &SurfaceParam, 
                  &iSurfaceEntries, 
                  pSurfaceEntries,
                  nullptr));

    for (i = 0; i < iSurfaceEntries; i++)
    {
        // Bind the Surface State
        CM_CHK_MOSSTATUS(pRenderHal->pfnBindSurfaceState(
                        pRenderHal, 
                        iBindingTable,
                        btIndex + i,
                        pSurfaceEntries[i]));
    }

    pState->pBT2DIndexTable[ surfIndex ].nPlaneNumber = iSurfaceEntries;
    // Get Offset to Current Binding Table 
    pStateHeap = pRenderHal->pStateHeap;
    dwOffsetSrc = ( pStateHeap->iCurSshBufferIndex * pStateHeap->dwSshIntanceSize ) +     // Points to the Base of Current SSH Buffer Instance
                        ( pStateHeap->iBindingTableOffset ) +                       // Moves the pointer to Base of Array of Binding Tables
                        ( iBindingTable * pStateHeap->iBindingTableSize ) +         // Moves the pointer to a Particular Binding Table
                        ( btIndex * sizeof( uint32_t ) );                              // Move the pointer to correct entry

    if ( pixelPitch )
    {
        pState->pBT2DIndexTable[ surfIndex ].BTI.SamplerSurfIndex = btIndex;
        pState->pBT2DIndexTable[ surfIndex ].BTITableEntry.SamplerBTIEntryPos = pStateHeap->pSshBuffer + dwOffsetSrc;
    }
    else
    {
        pState->pBT2DIndexTable[ surfIndex ].BTI.RegularSurfIndex = btIndex;
        pState->pBT2DIndexTable[ surfIndex ].BTITableEntry.RegularBTIEntryPos = pStateHeap->pSshBuffer + dwOffsetSrc;
    }

    hr = MOS_STATUS_SUCCESS;

finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose: Setup Buffer Surface State with BTIndex
//| Returns: Result of the operation
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_SetupBufferSurfaceStateWithBTIndex(
    PCM_HAL_STATE                      pState,
    int32_t                            iBindingTable,
    uint32_t                           surfIndex,
    uint32_t                           btIndex,
    bool                               pixelPitch)
{
    PRENDERHAL_INTERFACE            pRenderHal = pState->pRenderHal;
    MOS_STATUS                      hr;
    RENDERHAL_SURFACE               Surface;
    RENDERHAL_SURFACE_STATE_PARAMS  SurfaceParam;
    PRENDERHAL_SURFACE_STATE_ENTRY  pSurfaceEntry;
    uint16_t                        memObjCtl;
    uint32_t                        dwOffsetSrc;
    PRENDERHAL_STATE_HEAP           pStateHeap;
    UNUSED(pixelPitch);

    hr              = MOS_STATUS_UNKNOWN;

    if (surfIndex == CM_NULL_SURFACE)
    {
        return MOS_STATUS_SUCCESS;
    }

    memObjCtl = CM_DEFAULT_CACHE_TYPE;

    // Check to see if surface is already assigned
    if ( btIndex == ( uint32_t )pState->pBTBufferIndexTable[ surfIndex ].BTI.RegularSurfIndex )
    {
        uint32_t iSurfaceEntries = pState->pBTBufferIndexTable[ surfIndex ].nPlaneNumber;

        pStateHeap = pRenderHal->pStateHeap;

        // Get Offset to Current Binding Table 
        uint32_t dwOffsetDst = ( pStateHeap->iCurSshBufferIndex * pStateHeap->dwSshIntanceSize ) +     // Points to the Base of Current SSH Buffer Instance
                            ( pStateHeap->iBindingTableOffset ) +                       // Moves the pointer to Base of Array of Binding Tables
                            ( iBindingTable * pStateHeap->iBindingTableSize ) +               // Moves the pointer to a Particular Binding Table
                            ( btIndex * sizeof( uint32_t ) );                              // Move the pointer to correct entry

        uint32_t *pBindingTableEntry = ( uint32_t *)( pStateHeap->pSshBuffer + dwOffsetDst );
        MOS_SecureMemcpy( pBindingTableEntry, sizeof( uint32_t ) * iSurfaceEntries, pState->pBTBufferIndexTable[ surfIndex ].BTITableEntry.RegularBTIEntryPos, sizeof( uint32_t ) * iSurfaceEntries );

        return MOS_STATUS_SUCCESS;
    }

    // Get Details of Buffer surface and fill the Surface 
    CM_CHK_MOSSTATUS(HalCm_GetSurfaceAndRegister(pState, &Surface, CM_ARGUMENT_SURFACEBUFFER, surfIndex, 0));

    // set up buffer surface
    MOS_ZeroMemory(&SurfaceParam, sizeof(SurfaceParam));

    // Set bRenderTarget by default
    SurfaceParam.bRenderTarget = true;

    // Setup Buffer surface
    CM_CHK_MOSSTATUS(pRenderHal->pfnSetupBufferSurfaceState(
            pRenderHal, 
            &Surface, 
            &SurfaceParam,
            &pSurfaceEntry));

    //Cache configurations
    pState->pCmHalInterface->HwSetSurfaceMemoryObjectControl(memObjCtl, &SurfaceParam);

    // Bind the Surface State
    CM_CHK_MOSSTATUS(pRenderHal->pfnBindSurfaceState(
               pRenderHal, 
               iBindingTable,
               btIndex,
               pSurfaceEntry));

    pState->pBTBufferIndexTable[ surfIndex ].BTI.RegularSurfIndex = btIndex;
    pState->pBTBufferIndexTable[ surfIndex ].nPlaneNumber = 1;

    pStateHeap = pRenderHal->pStateHeap;
    dwOffsetSrc = ( pStateHeap->iCurSshBufferIndex * pStateHeap->dwSshIntanceSize ) +     // Points to the Base of Current SSH Buffer Instance
                        ( pStateHeap->iBindingTableOffset ) +                       // Moves the pointer to Base of Array of Binding Tables
                        ( iBindingTable * pStateHeap->iBindingTableSize ) +         // Moves the pointer to a Particular Binding Table
                        ( btIndex * sizeof( uint32_t ) );                              // Move the pointer to correct entry

    pState->pBTBufferIndexTable[ surfIndex ].BTITableEntry.RegularBTIEntryPos = pStateHeap->pSshBuffer + dwOffsetSrc;

    hr = MOS_STATUS_SUCCESS;

finish:
    return hr;
}

MOS_STATUS HalCm_Setup2DSurfaceUPStateWithBTIndex(
    PCM_HAL_STATE                      pState,
    int32_t                            iBindingTable,
    uint32_t                           surfIndex,
    uint32_t                           btIndex,
    bool                               pixelPitch)
{
    MOS_STATUS                     hr;
    RENDERHAL_SURFACE              Surface;
    RENDERHAL_SURFACE_STATE_PARAMS SurfaceParam;
    PRENDERHAL_INTERFACE           pRenderHal;
    PRENDERHAL_SURFACE_STATE_ENTRY pSurfaceEntries[MHW_MAX_SURFACE_PLANES];
    int32_t                        iSurfaceEntries, i;
    uint16_t                       memObjCtl;
    uint32_t                       dwOffsetSrc;
    PRENDERHAL_STATE_HEAP          pStateHeap;

    hr              = MOS_STATUS_UNKNOWN;
    pRenderHal    = pState->pRenderHal;

    if (surfIndex == CM_NULL_SURFACE)
    {
        return MOS_STATUS_SUCCESS;
    }

    memObjCtl = CM_DEFAULT_CACHE_TYPE;

    // Check to see if surface is already assigned
    uint32_t nBTInTable = ( unsigned char )CM_INVALID_INDEX;
    if ( pixelPitch )
    {
        nBTInTable = pState->pBT2DUPIndexTable[ surfIndex ].BTI.SamplerSurfIndex;
    }
    else
    {
        nBTInTable = pState->pBT2DUPIndexTable[ surfIndex ].BTI.RegularSurfIndex;
    }

    if ( btIndex == nBTInTable )
    {
        uint32_t iSurfaceEntries = pState->pBT2DUPIndexTable[ surfIndex ].nPlaneNumber;

        pStateHeap = pRenderHal->pStateHeap;

        // Get Offset to Current Binding Table 
        uint32_t dwOffsetDst = ( pStateHeap->iCurSshBufferIndex * pStateHeap->dwSshIntanceSize ) +   // Points to the Base of Current SSH Buffer Instance
                            ( pStateHeap->iBindingTableOffset ) +                       // Moves the pointer to Base of Array of Binding Tables
                            ( iBindingTable * pStateHeap->iBindingTableSize ) +         // Moves the pointer to a Particular Binding Table
                            ( btIndex * sizeof( uint32_t ) );                              // Move the pointer to correct entry

        uint32_t *pBindingTableEntry = ( uint32_t *)( pStateHeap->pSshBuffer + dwOffsetDst );
        if ( pixelPitch )
        {
            MOS_SecureMemcpy( pBindingTableEntry, sizeof( uint32_t ) * iSurfaceEntries, pState->pBT2DUPIndexTable[ surfIndex ].BTITableEntry.SamplerBTIEntryPos, sizeof( uint32_t ) * iSurfaceEntries );
        }
        else
        {
            MOS_SecureMemcpy( pBindingTableEntry, sizeof( uint32_t ) * iSurfaceEntries, pState->pBT2DUPIndexTable[ surfIndex ].BTITableEntry.RegularBTIEntryPos, sizeof( uint32_t ) * iSurfaceEntries );
        }

        return MOS_STATUS_SUCCESS;
    }

    // Get Details of 2DUP surface and fill the Surface 
    CM_CHK_MOSSTATUS( HalCm_GetSurfaceAndRegister( pState, &Surface, CM_ARGUMENT_SURFACE2D_UP, surfIndex, pixelPitch ) );

    // Setup 2D surface
    MOS_ZeroMemory( &SurfaceParam, sizeof( SurfaceParam ) );
    SurfaceParam.Type = pRenderHal->SurfaceTypeDefault;
    SurfaceParam.Boundary = RENDERHAL_SS_BOUNDARY_ORIGINAL;

    if ( !pixelPitch )
    {
        SurfaceParam.bWidthInDword_UV = true;
        SurfaceParam.bWidthInDword_Y = true;
    }
    
    SurfaceParam.bRenderTarget = true;

    //Cache configurations
    pState->pCmHalInterface->HwSetSurfaceMemoryObjectControl(memObjCtl, &SurfaceParam);

    CM_CHK_MOSSTATUS(pRenderHal->pfnSetupSurfaceState(
                pRenderHal, 
                &Surface, 
                &SurfaceParam, 
                &iSurfaceEntries, 
                pSurfaceEntries,
                nullptr));

    for (i = 0; i < iSurfaceEntries; i++)
    {
        // Bind the Surface State
        CM_CHK_MOSSTATUS(pRenderHal->pfnBindSurfaceState(
                        pRenderHal, 
                        iBindingTable,
                        btIndex + i,
                        pSurfaceEntries[i]));
    }

    pState->pBT2DUPIndexTable[ surfIndex ].nPlaneNumber = iSurfaceEntries;

    pStateHeap = pRenderHal->pStateHeap;
    dwOffsetSrc = ( pStateHeap->iCurSshBufferIndex * pStateHeap->dwSshIntanceSize ) +     // Points to the Base of Current SSH Buffer Instance
                        ( pStateHeap->iBindingTableOffset ) +                       // Moves the pointer to Base of Array of Binding Tables
                        ( iBindingTable * pStateHeap->iBindingTableSize ) +         // Moves the pointer to a Particular Binding Table
                        ( btIndex * sizeof( uint32_t ) );                              // Move the pointer to correct entry

    if ( pixelPitch )
    {
        pState->pBT2DUPIndexTable[ surfIndex ].BTI.SamplerSurfIndex = btIndex;
        pState->pBT2DUPIndexTable[ surfIndex ].BTITableEntry.SamplerBTIEntryPos = pStateHeap->pSshBuffer + dwOffsetSrc;
    }
    else
    {
        pState->pBT2DUPIndexTable[ surfIndex ].BTI.RegularSurfIndex = btIndex;
        pState->pBT2DUPIndexTable[ surfIndex ].BTITableEntry.RegularBTIEntryPos = pStateHeap->pSshBuffer + dwOffsetSrc;
    }

    hr = MOS_STATUS_SUCCESS;

finish:
    return hr;
}

MOS_STATUS HalCm_SetupSampler8x8SurfaceStateWithBTIndex(
    PCM_HAL_STATE           pState,
    int32_t                 iBindingTable,
    uint32_t                surfIndex,
    uint32_t                btIndex,
    bool                    pixelPitch,
    CM_HAL_KERNEL_ARG_KIND  iKind,
    uint32_t                AddressControl )
{
    MOS_STATUS                  hr;
    RENDERHAL_SURFACE               Surface;
    RENDERHAL_SURFACE_STATE_PARAMS  SurfaceParam;
    PRENDERHAL_INTERFACE            pRenderHal;
    PRENDERHAL_SURFACE_STATE_ENTRY  pSurfaceEntries[ MHW_MAX_SURFACE_PLANES ];
    int32_t                         iSurfaceEntries;
    uint16_t                        memObjCtl;
    int32_t                         i;
    uint32_t                        dwOffsetSrc;
    PRENDERHAL_STATE_HEAP           pStateHeap;
    UNUSED(pixelPitch);

    hr = MOS_STATUS_UNKNOWN;
    pRenderHal = pState->pRenderHal;

    if ( surfIndex == CM_NULL_SURFACE )
    {
        hr = MOS_STATUS_SUCCESS;
        goto finish;
    }

    memObjCtl = CM_DEFAULT_CACHE_TYPE;

    // check to see if index is valid
    if ( surfIndex >= pState->CmDeviceParam.iMax2DSurfaceTableSize ||
         Mos_ResourceIsNull( &pState->pUmdSurf2DTable[ surfIndex ].OsResource ) )
    {
        CM_ERROR_ASSERT(
            "Invalid 2D Surface array index '%d'", surfIndex );
        goto finish;
    }

    // Get Details of Sampler8x8 surface and fill the Surface
    CM_CHK_MOSSTATUS( HalCm_GetSurfaceAndRegister( pState, &Surface, iKind, surfIndex, 0 ) );

    // Setup surface
    MOS_ZeroMemory( &SurfaceParam, sizeof( SurfaceParam ) );
    SurfaceParam.Type = pRenderHal->SurfaceTypeAdvanced;
    SurfaceParam.bRenderTarget = true;
    SurfaceParam.bWidthInDword_Y = false;
    SurfaceParam.bWidthInDword_UV = false;
    SurfaceParam.Boundary = RENDERHAL_SS_BOUNDARY_ORIGINAL;
    SurfaceParam.bVASurface = ( iKind == CM_ARGUMENT_SURFACE_SAMPLER8X8_VA ) ? 1 : 0;
    SurfaceParam.AddressControl = AddressControl;
    pState->pCmHalInterface->HwSetSurfaceMemoryObjectControl(memObjCtl, &SurfaceParam );
    pRenderHal->bEnableP010SinglePass = pState->pCmHalInterface->IsP010SinglePassSupported();
    iSurfaceEntries = 0;
    CM_CHK_MOSSTATUS( pRenderHal->pfnSetupSurfaceState(
        pRenderHal,
        &Surface,
        &SurfaceParam,
        &iSurfaceEntries,
        pSurfaceEntries,
        nullptr ) );

    CM_ASSERT( iSurfaceEntries == 1 );

    for ( i = 0; i < iSurfaceEntries; i++ )
    {
        // Bind the Surface State
        CM_CHK_MOSSTATUS( pRenderHal->pfnBindSurfaceState(
            pRenderHal,
            iBindingTable,
            btIndex + i,
            pSurfaceEntries[ i ] ) );
    }

    pStateHeap = pRenderHal->pStateHeap;
    dwOffsetSrc = ( pStateHeap->iCurSshBufferIndex * pStateHeap->dwSshIntanceSize ) +     // Points to the Base of Current SSH Buffer Instance
        ( pStateHeap->iBindingTableOffset ) +                       // Moves the pointer to Base of Array of Binding Tables
        ( iBindingTable * pStateHeap->iBindingTableSize ) +         // Moves the pointer to a Particular Binding Table
        ( btIndex * sizeof( uint32_t ) );                              // Move the pointer to correct entry

    pState->pBT2DIndexTable[ surfIndex ].nPlaneNumber = iSurfaceEntries;
    pState->pBT2DIndexTable[ surfIndex ].BTITableEntry.Sampler8x8BTIEntryPos = pStateHeap->pSshBuffer + dwOffsetSrc;
    pState->pBT2DIndexTable[ surfIndex ].BTI.Sampler8x8SurfIndex = btIndex;

    hr = MOS_STATUS_SUCCESS;

finish:
    pRenderHal->bEnableP010SinglePass = false;
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose: Setup 3D Surface State with BTIndex
//| Returns: Result of the operation 
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_Setup3DSurfaceStateWithBTIndex(
    PCM_HAL_STATE                      pState,
    int32_t                            iBindingTable,
    uint32_t                           surfIndex,
    uint32_t                           btIndex)
{
    PRENDERHAL_INTERFACE            pRenderHal = pState->pRenderHal;
    MOS_STATUS                      hr;
    RENDERHAL_SURFACE               Surface;
    RENDERHAL_SURFACE_STATE_PARAMS  SurfaceParam;
    PRENDERHAL_SURFACE_STATE_ENTRY  pSurfaceEntries[MHW_MAX_SURFACE_PLANES];
    int32_t                     iSurfaceEntries, i;
    uint16_t                    memObjCtl;
    uint32_t                    dwOffsetSrc;
    PRENDERHAL_STATE_HEAP       pStateHeap;

    hr = MOS_STATUS_UNKNOWN;
    iSurfaceEntries = 0;

    if (surfIndex == CM_NULL_SURFACE)
    {
        return MOS_STATUS_SUCCESS;
    }

    memObjCtl = CM_DEFAULT_CACHE_TYPE;


    // check the surfIndex
    if (surfIndex >= pState->CmDeviceParam.iMax3DSurfaceTableSize ||
        Mos_ResourceIsNull(&pState->pSurf3DTable[surfIndex].OsResource))
    {
        CM_ERROR_ASSERT(
            "Invalid 3D Surface array index '%d'", surfIndex);
        return MOS_STATUS_UNKNOWN;
    }

    // Check to see if surface is already assigned
    uint32_t nBTInTable = (unsigned char)CM_INVALID_INDEX;
    nBTInTable = pState->pBT3DIndexTable[surfIndex].BTI.RegularSurfIndex;

    if (btIndex == nBTInTable)
    {
        iSurfaceEntries = pState->pBT3DIndexTable[surfIndex].nPlaneNumber;

        pStateHeap = pRenderHal->pStateHeap;

        // Get Offset to Current Binding Table 
        uint32_t dwOffsetDst = (pStateHeap->iCurSshBufferIndex * pStateHeap->dwSshIntanceSize) +     // Points to the Base of Current SSH Buffer Instance
            (pStateHeap->iBindingTableOffset) +                       // Moves the pointer to Base of Array of Binding Tables
            (iBindingTable * pStateHeap->iBindingTableSize) +         // Moves the pointer to a Particular Binding Table
            (btIndex * sizeof(uint32_t));                              // Move the pointer to correct entry

        uint32_t *pBindingTableEntry = (uint32_t*)(pStateHeap->pSshBuffer + dwOffsetDst);

        MOS_SecureMemcpy(pBindingTableEntry, sizeof(uint32_t)* iSurfaceEntries, pState->pBT3DIndexTable[surfIndex].BTITableEntry.RegularBTIEntryPos, sizeof(uint32_t)* iSurfaceEntries);

        return MOS_STATUS_SUCCESS;
    }

    // Get Details of 3D surface and fill the Surface 
    CM_CHK_MOSSTATUS(HalCm_GetSurfaceAndRegister(pState, &Surface, CM_ARGUMENT_SURFACE3D, surfIndex, false));

    // Setup 3D surface
    MOS_ZeroMemory(&SurfaceParam, sizeof(SurfaceParam));
    SurfaceParam.Type = pRenderHal->SurfaceTypeDefault;
    SurfaceParam.Boundary = RENDERHAL_SS_BOUNDARY_ORIGINAL;

    //Cache configurations
    pState->pCmHalInterface->HwSetSurfaceMemoryObjectControl(memObjCtl, &SurfaceParam);

    //Set bRenderTarget by default
    SurfaceParam.bRenderTarget = true;

    CM_CHK_MOSSTATUS(pRenderHal->pfnSetupSurfaceState(
        pRenderHal,
        &Surface,
        &SurfaceParam,
        &iSurfaceEntries,
        pSurfaceEntries,
        nullptr));


    for (i = 0; i < iSurfaceEntries; i++)
    {
        // Bind the Surface State
        CM_CHK_MOSSTATUS(pRenderHal->pfnBindSurfaceState(
            pRenderHal,
            iBindingTable,
            btIndex + i,
            pSurfaceEntries[i]));
    }
    pState->pBT3DIndexTable[surfIndex].BTI.RegularSurfIndex = btIndex;
    pState->pBT3DIndexTable[surfIndex].nPlaneNumber = iSurfaceEntries;
    // Get Offset to Current Binding Table 
    pStateHeap = pRenderHal->pStateHeap;
    dwOffsetSrc = (pStateHeap->iCurSshBufferIndex * pStateHeap->dwSshIntanceSize) +     // Points to the Base of Current SSH Buffer Instance
        (pStateHeap->iBindingTableOffset) +                       // Moves the pointer to Base of Array of Binding Tables
        (iBindingTable * pStateHeap->iBindingTableSize) +         // Moves the pointer to a Particular Binding Table
        (btIndex * sizeof(uint32_t));                              // Move the pointer to correct entry

    pState->pBT3DIndexTable[surfIndex].BTI.RegularSurfIndex = btIndex;
    pState->pBT3DIndexTable[surfIndex].BTITableEntry.RegularBTIEntryPos = pStateHeap->pSshBuffer + dwOffsetSrc;

    hr = MOS_STATUS_SUCCESS;

finish:
    return hr;
}


//|-----------------------------------------------------------------------------
//| Purpose   : Tag-based Synchronization on Resource
//| Input     : pState   - Hal CM State
//|             pSurface    Surface
//|             isWrite  - Write or Read 
//| Returns   : Result of the operation
//|-----------------------------------------------------------------------------
MOS_STATUS HalCm_SyncOnResource(
    PCM_HAL_STATE           pState,
    PMOS_SURFACE            pSurface,
    bool                    isWrite)
{
    MOS_STATUS              hr;
    PMOS_INTERFACE          pOsInterface;

    hr           = MOS_STATUS_SUCCESS;
    pOsInterface = pState->pOsInterface;

    if (pSurface == nullptr || Mos_ResourceIsNull(&pSurface->OsResource))
    {
        CM_ASSERTMESSAGE("Input resource is not valid.");
        hr = MOS_STATUS_UNKNOWN;
        return hr;
    }

    pOsInterface->pfnSyncOnResource(
            pOsInterface, 
            &(pSurface->OsResource),
            pState->pOsInterface->CurrentGpuContextOrdinal, //pState->GpuContext,
            isWrite);

    // Sync Render Target with Overlay Context
    if (pSurface->bOverlay)
    {
        pOsInterface->pfnSyncOnOverlayResource(
            pOsInterface,
            &(pSurface->OsResource),
            pState->GpuContext);
    }

    return hr;
}

//!
//! \brief    Send Media Walker State
//! \details  Send MEDIA_OBJECT_WALKER command
//! \param    PCM_HAL_STATE pState
//!           [in] Pointer to CM_HAL_STATE Structure
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to Hardware Interface Structure
//! \param    PMOS_COMMAND_BUFFER pCmdBuffer
//!           [in] Pointer to Command Buffer
//! \return   MOS_STATUS
//!
MOS_STATUS HalCm_SendMediaWalkerState(
    PCM_HAL_STATE               pState,
    PCM_HAL_KERNEL_PARAM        pKernelParam,
    PMOS_COMMAND_BUFFER         pCmdBuffer)
{
    PRENDERHAL_INTERFACE      pRenderHal;
    MHW_WALKER_PARAMS         MediaWalkerParams;
    MOS_STATUS                hr;

    hr         = MOS_STATUS_SUCCESS;
    pRenderHal = pState->pRenderHal;

    MOS_SecureMemcpy(&MediaWalkerParams, sizeof(MHW_WALKER_PARAMS), &pKernelParam->WalkerParams, sizeof(CM_HAL_WALKER_PARAMS));

    if (pKernelParam->KernelThreadSpaceParam.iThreadSpaceWidth)
    {
        //per-kernel thread space is set, need use its own dependency mask
        MediaWalkerParams.UseScoreboard  = pRenderHal->VfeScoreboard.ScoreboardEnable;
        MediaWalkerParams.ScoreboardMask = pKernelParam->KernelThreadSpaceParam.globalDependencyMask;
    }
    else
    {
        //No per-kernel thread space setting, need use per-task depedency mask
        MediaWalkerParams.UseScoreboard  = pRenderHal->VfeScoreboard.ScoreboardEnable;
        MediaWalkerParams.ScoreboardMask = pRenderHal->VfeScoreboard.ScoreboardMask;
    }

    hr = pRenderHal->pMhwRenderInterface->AddMediaObjectWalkerCmd(
                                  pCmdBuffer, &MediaWalkerParams);


    return hr;
}

//!
//! \brief    Send GpGpu Walker State
//! \details  Send GPGPU_WALKER state
//! \param    PCM_HAL_STATE pState
//!           [in] Pointer to CM_HAL_STATE Structure
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to Hardware Interface Structure
//! \param    PMOS_COMMAND_BUFFER pCmdBuffer
//!           [in] Pointer to Command Buffer
//! \return   MOS_STATUS
//!
MOS_STATUS HalCm_SendGpGpuWalkerState(
    PCM_HAL_STATE               pState,
    PCM_HAL_KERNEL_PARAM        pKernelParam,
    PMOS_COMMAND_BUFFER         pCmdBuffer)
{
    MhwRenderInterface           *pMhwRender;
    MHW_GPGPU_WALKER_PARAMS      GpGpuWalkerParams;
    MOS_STATUS                   hr;

    hr           = MOS_STATUS_SUCCESS;
    pMhwRender = pState->pRenderHal->pMhwRenderInterface;

    GpGpuWalkerParams.InterfaceDescriptorOffset = pKernelParam->GpGpuWalkerParams.InterfaceDescriptorOffset;
    GpGpuWalkerParams.GpGpuEnable               = pKernelParam->GpGpuWalkerParams.CmGpGpuEnable;
    GpGpuWalkerParams.GroupWidth                = pKernelParam->GpGpuWalkerParams.GroupWidth;
    GpGpuWalkerParams.GroupHeight               = pKernelParam->GpGpuWalkerParams.GroupHeight;
    GpGpuWalkerParams.GroupDepth               = pKernelParam->GpGpuWalkerParams.GroupDepth;   
    GpGpuWalkerParams.ThreadWidth               = pKernelParam->GpGpuWalkerParams.ThreadWidth;
    GpGpuWalkerParams.ThreadHeight              = pKernelParam->GpGpuWalkerParams.ThreadHeight;
    GpGpuWalkerParams.ThreadDepth               = pKernelParam->GpGpuWalkerParams.ThreadDepth;
    GpGpuWalkerParams.SLMSize                   = pKernelParam->iSLMSize;

    hr = pMhwRender->AddGpGpuWalkerStateCmd(pCmdBuffer, &GpGpuWalkerParams);

    return hr;
}



//!
//! \brief    Surface Format Convert
//! \details  Convert RENDERHAL_SURFACE to MHW_VEBOX_SURFACE
//! \param    PRENDERHAL_SURFACE            pRenderHalSurface
//!           [in] Pointer to RENDERHAL_SURFACE Structure
//! \param    PMHW_VEBOX_SURFACE_PARAMS    pMhwVeboxSurface
//!           [in] Pointer to PMHW_VEBOX_SURFACE_PARAMS
//! \return   MOS_STATUS
//!
MOS_STATUS HalCm_Convert_RENDERHAL_SURFACE_To_MHW_VEBOX_SURFACE(
    PRENDERHAL_SURFACE              pRenderHalSurface,
    PMHW_VEBOX_SURFACE_PARAMS    pMhwVeboxSurface)
{
    PMOS_SURFACE                    pSurface;
    MOS_STATUS                      hr = MOS_STATUS_SUCCESS;

    CM_CHK_NULL_RETURN_MOSSTATUS(pRenderHalSurface);
    CM_CHK_NULL_RETURN_MOSSTATUS(pMhwVeboxSurface);

    pSurface = &pRenderHalSurface->OsSurface;
    pMhwVeboxSurface->Format        = pSurface->Format;
    pMhwVeboxSurface->dwWidth       = pSurface->dwWidth;
    pMhwVeboxSurface->dwHeight      = pSurface->dwHeight;
    pMhwVeboxSurface->dwPitch       = pSurface->dwPitch;
    pMhwVeboxSurface->TileType      = pSurface->TileType;
    pMhwVeboxSurface->rcMaxSrc      = pRenderHalSurface->rcMaxSrc;
    pMhwVeboxSurface->pOsResource   = &pSurface->OsResource;

finish:
    return hr;
}

//!
//! \brief    Set Vtune Profiling Flag
//! \details  Trun Vtune Profiling Flag On or off
//! \param    PCM_HAL_STATE pState
//!           [in] Pointer to CM_HAL_STATE Structure
//! \return   MOS_STATUS_SUCCESS
//!
MOS_STATUS HalCm_SetVtuneProfilingFlag(
    PCM_HAL_STATE               pState,
    bool                        bVtuneOn)
{

    pState->bVtuneProfilerOn   = bVtuneOn;

    return MOS_STATUS_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get the offset for the Task Sync Location given the task ID
//| Returns:    Sync Location
//*-----------------------------------------------------------------------------
int32_t HalCm_GetTaskSyncLocation(
    int32_t             iTaskId)        // [in] Task ID
{
    return (iTaskId * (CM_SYNC_QWORD_PER_TASK * sizeof(uint64_t)
            +(CM_FRAME_TRACKING_QWORD_PER_TASK * sizeof(uint64_t))));
}

void HalCm_GetLegacyRenderHalL3Setting( CmHalL3Settings *l3_settings_ptr, RENDERHAL_L3_CACHE_SETTINGS *l3_settings_legacy_ptr )
{
    *l3_settings_legacy_ptr = {};
    l3_settings_legacy_ptr->bOverride = l3_settings_ptr->override_settings;
    l3_settings_legacy_ptr->bEnableSLM = l3_settings_ptr->enable_slm;
    l3_settings_legacy_ptr->bL3CachingEnabled = l3_settings_ptr->l3_caching_enabled;
    l3_settings_legacy_ptr->bCntlRegOverride = l3_settings_ptr->cntl_reg_override;
    l3_settings_legacy_ptr->bCntlReg2Override = l3_settings_ptr->cntl_reg2_override;
    l3_settings_legacy_ptr->bCntlReg3Override = l3_settings_ptr->cntl_reg3_override;
    l3_settings_legacy_ptr->bSqcReg1Override = l3_settings_ptr->sqc_reg1_override;
    l3_settings_legacy_ptr->bSqcReg4Override = l3_settings_ptr->sqc_reg4_override;
    l3_settings_legacy_ptr->bLra1RegOverride = l3_settings_ptr->lra1_reg_override;
    l3_settings_legacy_ptr->dwCntlReg = l3_settings_ptr->cntl_reg;
    l3_settings_legacy_ptr->dwCntlReg2 = l3_settings_ptr->cntl_reg2;
    l3_settings_legacy_ptr->dwCntlReg3 = l3_settings_ptr->cntl_reg3;
    l3_settings_legacy_ptr->dwSqcReg1 = l3_settings_ptr->sqc_reg1;
    l3_settings_legacy_ptr->dwSqcReg4 = l3_settings_ptr->sqc_reg4;
    l3_settings_legacy_ptr->dwLra1Reg = l3_settings_ptr->lra1_reg;

    return;
}
