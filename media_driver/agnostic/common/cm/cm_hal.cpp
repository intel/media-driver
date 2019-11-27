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
#include "cm_execution_adv.h"
#include "cm_extension_creator.h"

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
    PCM_HAL_KERNEL_SLICE_SUBSLICE  destination;
};
typedef CM_HAL_KERNEL_SUBSLICE_INFO *PCM_HAL_KERNEL_SUBSLICE_INFO;

// forward declaration
int32_t HalCm_InsertCloneKernel(
    PCM_HAL_STATE              state,
    PCM_HAL_KERNEL_PARAM       kernelParam,
    PRENDERHAL_KRN_ALLOCATION  &kernelAllocation);

extern MOS_STATUS HalCm_GetSipBinary(
    PCM_HAL_STATE   state);

#if MDF_COMMAND_BUFFER_DUMP
extern int32_t HalCm_InitDumpCommandBuffer(PCM_HAL_STATE state);

extern int32_t HalCm_DumpCommadBuffer(PCM_HAL_STATE state, PMOS_COMMAND_BUFFER cmdBuffer,
               int offsetSurfaceState, size_t sizeOfSurfaceState);
#endif

#if MDF_CURBE_DATA_DUMP
extern int32_t HalCm_InitDumpCurbeData(PCM_HAL_STATE state);

extern int32_t HalCm_DumpCurbeData(PCM_HAL_STATE state);
#endif

#if MDF_SURFACE_CONTENT_DUMP
extern int32_t HalCm_InitSurfaceDump(PCM_HAL_STATE state);

#endif

#if MDF_SURFACE_STATE_DUMP
extern int32_t HalCm_InitDumpSurfaceState(PCM_HAL_STATE state);
extern int32_t HalCm_DumpSurfaceState(PCM_HAL_STATE state, int offsetSurfaceState, size_t sizeOfSurfaceState);
#endif

#if MDF_INTERFACE_DESCRIPTOR_DATA_DUMP
extern int32_t HalCm_InitDumpInterfaceDescriporData(PCM_HAL_STATE state);
extern int32_t HalCm_DumpInterfaceDescriptorData(PCM_HAL_STATE state);
#endif

extern uint64_t HalCm_GetTsFrequency(PMOS_INTERFACE pOsInterface);

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
bool HalCm_GetTaskHasThreadArg(PCM_HAL_KERNEL_PARAM *kernels, uint32_t numKernels)
{
    PCM_HAL_KERNEL_PARAM            kernelParam;
    PCM_HAL_KERNEL_ARG_PARAM        argParam;
    bool                            threadArgExists = false;

    for( uint32_t krn = 0; krn < numKernels; krn++)
    {
        kernelParam    = kernels[krn];
        for(uint32_t argIndex = 0; argIndex < kernelParam->numArgs; argIndex++)
        {
            argParam = &kernelParam->argParams[argIndex];
            if( argParam->perThread )
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
    PCM_HAL_STATE state)                                                       // [in] Pointer to CM HAL State
{
    MOS_STATUS              eStatus = MOS_STATUS_SUCCESS;
    uint32_t                size;
    PMOS_INTERFACE          osInterface;
    MOS_ALLOC_GFXRES_PARAMS allocParams;
    MOS_LOCK_PARAMS         lockFlags;

    osInterface    = state->osInterface;

    size = state->cmHalInterface->GetTimeStampResourceSize() * state->cmDeviceParam.maxTasks;    
    // allocate render engine Ts Resource
    MOS_ZeroMemory(&allocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParams.Type    = MOS_GFXRES_BUFFER;
    allocParams.dwBytes = size;
    allocParams.Format  = Format_Buffer;  //used in RenderHal_OsAllocateResource_Linux
    allocParams.TileType= MOS_TILE_LINEAR;
    allocParams.pBufName = "TsResource";

    CM_CHK_HRESULT_GOTOFINISH_MOSERROR(
        osInterface->pfnAllocateResource(osInterface,
                                         &allocParams,
                                         &state->renderTimeStampResource.osResource));
    CM_CHK_MOSSTATUS_GOTOFINISH(
        osInterface->pfnRegisterResource(osInterface,
                                         &state->renderTimeStampResource.osResource,
                                         true, true));

    osInterface->pfnSkipResourceSync(&state->renderTimeStampResource.osResource);

    // Lock the Resource
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));

    lockFlags.ReadOnly = 1;
    lockFlags.ForceCached = true;

    state->renderTimeStampResource.data = (uint8_t*)osInterface->pfnLockResource(
                                        osInterface,
                                        &state->renderTimeStampResource.osResource,
                                        &lockFlags);

    CM_CHK_NULL_GOTOFINISH_MOSERROR(state->renderTimeStampResource.data);

    state->renderTimeStampResource.locked  = true;

    //allocated for vebox TS resource

    MOS_ZeroMemory(&allocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParams.Type = MOS_GFXRES_BUFFER;
    allocParams.dwBytes = size;
    allocParams.Format = Format_Buffer;  //used in RenderHal_OsAllocateResource_Linux
    allocParams.TileType = MOS_TILE_LINEAR;
    allocParams.pBufName = "TsResource";

    CM_CHK_HRESULT_GOTOFINISH_MOSERROR(osInterface->pfnAllocateResource(
            osInterface,
            &allocParams,
            &state->veboxTimeStampResource.osResource));

    // Lock the Resource
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));

    lockFlags.ReadOnly = 1;
    lockFlags.ForceCached = true;

    state->veboxTimeStampResource.data = (uint8_t*)osInterface->pfnLockResource(
            osInterface,
            &state->veboxTimeStampResource.osResource,
            &lockFlags);

    CM_CHK_NULL_GOTOFINISH_MOSERROR(state->veboxTimeStampResource.data);

    state->veboxTimeStampResource.locked  = true;

finish:
    return eStatus;
}

//! \brief    Allocate tracker resource
//! \param    [in] state
//!           Pointer to CM_HAL_STATE structure
//! \return   MOS_STATUS
MOS_STATUS HalCm_AllocateTrackerResource(
    PCM_HAL_STATE           state)
{
    MOS_STATUS                          eStatus = MOS_STATUS_SUCCESS;
    MOS_ALLOC_GFXRES_PARAMS             allocParamsLinearBuffer;
    MOS_LOCK_PARAMS                     lockFlags;
    PMOS_INTERFACE                      osInterface;
    PRENDERHAL_INTERFACE                renderHal;

    osInterface = state->osInterface;
    renderHal   = state->renderHal;

    // Tracker producer for RENDER engine
    renderHal->trackerProducer.Initialize(osInterface);

    // Tracker resource for VeBox engine
    Mos_ResetResource(&renderHal->veBoxTrackerRes.osResource);

    MOS_ZeroMemory(&allocParamsLinearBuffer, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsLinearBuffer.Type = MOS_GFXRES_BUFFER;
    allocParamsLinearBuffer.TileType = MOS_TILE_LINEAR;
    allocParamsLinearBuffer.Format = Format_Buffer;
    allocParamsLinearBuffer.dwBytes = MHW_CACHELINE_SIZE;
    allocParamsLinearBuffer.pBufName = "VeboxTrackerRes";

    CM_CHK_HRESULT_GOTOFINISH_MOSERROR(osInterface->pfnAllocateResource(
        osInterface,
        &allocParamsLinearBuffer,
        &renderHal->veBoxTrackerRes.osResource));

    // Lock the Resource
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));

    lockFlags.ReadOnly = 1;
    lockFlags.ForceCached = true;

    renderHal->veBoxTrackerRes.data = (uint32_t*)osInterface->pfnLockResource(
        osInterface,
        &renderHal->veBoxTrackerRes.osResource,
        &lockFlags);

    CM_CHK_NULL_GOTOFINISH_MOSERROR(renderHal->veBoxTrackerRes.data);

    *(renderHal->veBoxTrackerRes.data) = MemoryBlock::m_invalidTrackerId;

    renderHal->veBoxTrackerRes.currentTrackerId = 1;

    renderHal->veBoxTrackerRes.locked = true;

finish:
    return eStatus;
}

//! \brief    Initialize dynamic state heap
//! \param    [in] state
//!           Pointer to CM_HAL_STATE structure
//! \param    [in] heapParam
//!           Pointer to CM_HAL_HEAP_PARAM structure
//! \return   MOS_STATUS
MOS_STATUS HalCm_InitializeDynamicStateHeaps(
    PCM_HAL_STATE           state,
    CM_HAL_HEAP_PARAM       *heapParam)
{
    MOS_STATUS   eStatus        = MOS_STATUS_SUCCESS;
    HeapManager* dgsHeap   = state->renderHal->dgsheapManager;

    CM_CHK_NULL_GOTOFINISH_MOSERROR(heapParam);

    dgsHeap = MOS_New(HeapManager);
    CM_CHK_NULL_GOTOFINISH_MOSERROR(dgsHeap);
    CM_CHK_MOSSTATUS_GOTOFINISH(dgsHeap->RegisterOsInterface(state->osInterface));

    dgsHeap->SetDefaultBehavior(heapParam->behaviorGSH);
    CM_CHK_MOSSTATUS_GOTOFINISH(dgsHeap->SetInitialHeapSize(heapParam->initialSizeGSH));
    CM_CHK_MOSSTATUS_GOTOFINISH(dgsHeap->SetExtendHeapSize(heapParam->extendSizeGSH));
    CM_CHK_MOSSTATUS_GOTOFINISH(dgsHeap->RegisterTrackerProducer(heapParam->trackerProducer));
    // lock the heap in the beginning, so cpu doesn't need to wait gpu finishing occupying it to lock it again
    CM_CHK_MOSSTATUS_GOTOFINISH(dgsHeap->LockHeapsOnAllocate());

    state->renderHal->dgsheapManager = dgsHeap;

finish:
    return eStatus;
}


//*-----------------------------------------------------------------------------
//| Purpose:    Free Timestamp Resource
//| Returns:    Result of the operation
//*-----------------------------------------------------------------------------
__inline void HalCm_FreeTsResource(
    PCM_HAL_STATE state)                                                       // [in] Pointer to CM HAL State
{
    PMOS_INTERFACE      osInterface;
    MOS_STATUS          hr;

    osInterface    = state->osInterface;

    if (!Mos_ResourceIsNull(&state->renderTimeStampResource.osResource))
    {
        if (state->renderTimeStampResource.locked)
        {
            hr = (MOS_STATUS)osInterface->pfnUnlockResource(
                    osInterface,
                    &state->renderTimeStampResource.osResource);

            CM_ASSERT(hr == MOS_STATUS_SUCCESS);
        }

        osInterface->pfnFreeResourceWithFlag(
            osInterface,
            &state->renderTimeStampResource.osResource,
            SURFACE_FLAG_ASSUME_NOT_IN_USE);
    }

    //free vebox TS resource

    if (!Mos_ResourceIsNull(&state->veboxTimeStampResource.osResource))
    {
        if (state->veboxTimeStampResource.locked)
        {
            hr = (MOS_STATUS)osInterface->pfnUnlockResource(
                osInterface,
                &state->veboxTimeStampResource.osResource);

            CM_ASSERT(hr == MOS_STATUS_SUCCESS);
        }

        osInterface->pfnFreeResourceWithFlag(
            osInterface,
            &state->veboxTimeStampResource.osResource,
            SURFACE_FLAG_ASSUME_NOT_IN_USE);
    }
}

//! \brief    Free tracker resource
//! \param    PCM_HAL_STATE state
//!           [in] Pointer to CM_HAL_STATE structure
//! \return   void
__inline void HalCm_FreeTrackerResources(
    PCM_HAL_STATE state)                                                       // [in] Pointer to CM HAL State
{
    PMOS_INTERFACE      osInterface;
    MOS_STATUS          hr;

    osInterface = state->osInterface;

    if (!Mos_ResourceIsNull(&state->renderHal->veBoxTrackerRes.osResource))
    {
        if (state->renderHal->veBoxTrackerRes.locked)
        {
            hr = (MOS_STATUS)osInterface->pfnUnlockResource(
                osInterface,
                &state->renderHal->veBoxTrackerRes.osResource);

            CM_ASSERT(hr == MOS_STATUS_SUCCESS);
        }

        osInterface->pfnFreeResourceWithFlag(
            osInterface,
            &state->renderHal->veBoxTrackerRes.osResource,
            SURFACE_FLAG_ASSUME_NOT_IN_USE);
    }
}

//*-----------------------------------------------------------------------------
//| Purpose:    Allocate CSR Resource
//| Returns:    Result of the operation
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_AllocateCSRResource(
    PCM_HAL_STATE state)       // [in] Pointer to CM HAL State
{
    PMOS_INTERFACE          osInterface = state->osInterface;
    MOS_STATUS              eStatus = MOS_STATUS_SUCCESS;
    uint32_t                size;
    MOS_ALLOC_GFXRES_PARAMS allocParams;

    //Enable Mid-thread
    state->renderHal->pfnEnableGpgpuMiddleThreadPreemption(state->renderHal);

    size = CM_CSR_SURFACE_SIZE;

    MOS_ZeroMemory(&allocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParams.Type = MOS_GFXRES_BUFFER;
    allocParams.dwBytes = size;
    allocParams.Format = Format_RAW;  //used in VpHal_OsAllocateResource_Linux
    allocParams.TileType = MOS_TILE_LINEAR;
    allocParams.pBufName = "CSRResource";

    CM_CHK_HRESULT_GOTOFINISH_MOSERROR(osInterface->pfnAllocateResource(
        osInterface,
        &allocParams,
        &state->csrResource));

    osInterface->pfnSkipResourceSync(&state->csrResource);

finish:
    return eStatus;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Allocate Sip Resource
//| Returns:    Result of the operation
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_AllocateSipResource(
    PCM_HAL_STATE state)       // [in] Pointer to CM HAL State
{
    PMOS_INTERFACE          osInterface = state->osInterface;
    MOS_STATUS              eStatus = MOS_STATUS_SUCCESS;
    uint32_t                size;
    MOS_ALLOC_GFXRES_PARAMS allocParams;
    MOS_LOCK_PARAMS         lockFlags;

    size = CM_DEBUG_SURFACE_SIZE;

    MOS_ZeroMemory(&allocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParams.Type = MOS_GFXRES_BUFFER;
    allocParams.dwBytes = size;
    allocParams.Format = Format_Buffer;  //used in RenderHal_OsAllocateResource_Linux
    allocParams.TileType = MOS_TILE_LINEAR;
    allocParams.pBufName = "SipResource";

    CM_CHK_HRESULT_GOTOFINISH_MOSERROR(osInterface->pfnAllocateResource(
        osInterface,
        &allocParams,
        &state->sipResource.osResource));

    // Lock the Resource
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));

    lockFlags.ReadOnly = 1;
    lockFlags.ForceCached = true;

    state->sipResource.data = (uint8_t*)osInterface->pfnLockResource(
        osInterface,
        &state->sipResource.osResource,
        &lockFlags);
    CM_CHK_NULL_GOTOFINISH_MOSERROR(state->sipResource.data);

    state->sipResource.locked = true;

finish:
    return eStatus;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Free CSR Resource
//| Returns:    Result of the operation
//*-----------------------------------------------------------------------------
__inline void HalCm_FreeCsrResource(
    PCM_HAL_STATE state)   // [in] Pointer to CM HAL State
{
    PMOS_INTERFACE      osInterface = state->osInterface;

    if (!Mos_ResourceIsNull(&state->csrResource))
    {
        osInterface->pfnFreeResourceWithFlag(
            osInterface,
            &state->csrResource,
            SURFACE_FLAG_ASSUME_NOT_IN_USE);
    }
}

//*-----------------------------------------------------------------------------
//| Purpose:    Free Sip Resource
//| Returns:    Result of the operation
//*-----------------------------------------------------------------------------
__inline void HalCm_FreeSipResource(
    PCM_HAL_STATE state)   // [in] Pointer to CM HAL State
{
    PMOS_INTERFACE      osInterface = state->osInterface;
    MOS_STATUS          hr = MOS_STATUS_SUCCESS;

    if (!Mos_ResourceIsNull(&state->sipResource.osResource))
    {
        if (state->sipResource.locked)
        {
            hr = (MOS_STATUS)osInterface->pfnUnlockResource(
                    osInterface,
                    &state->sipResource.osResource);

            CM_ASSERT(hr == MOS_STATUS_SUCCESS);
        }

        osInterface->pfnFreeResourceWithFlag(
            osInterface,
            &state->sipResource.osResource,
            SURFACE_FLAG_ASSUME_NOT_IN_USE);
    }
}

//*-----------------------------------------------------------------------------
//| Purpose: Sets Arg data in the buffer
//| Returns: Result of the operation
//*-----------------------------------------------------------------------------
__inline void HalCm_SetArgData(
    PCM_HAL_KERNEL_ARG_PARAM    argParam,
    uint32_t                    threadIndex,
    uint8_t                     *buffer)
{
    uint8_t *dst;
    uint8_t *src;

    dst = buffer + argParam->payloadOffset;
    src = argParam->firstValue + (threadIndex * argParam->unitSize);

    MOS_SecureMemcpy(dst, argParam->unitSize, src, argParam->unitSize);
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get the Buffer Entry
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
__inline MOS_STATUS HalCm_GetResourceUPEntry(
    PCM_HAL_STATE           state,                                             // [in]  Pointer to CM State
    uint32_t                handle,                                           // [in]  Handle
    PCM_HAL_SURFACE2D_UP_ENTRY    *entryOut)                                         // [out] Buffer Entry
{
    MOS_STATUS                    eStatus;
    PCM_HAL_SURFACE2D_UP_ENTRY    entry;

    eStatus = MOS_STATUS_SUCCESS;

    if (handle >= state->cmDeviceParam.max2DSurfaceUPTableSize)
    {
        eStatus = MOS_STATUS_INVALID_HANDLE;
        CM_ASSERTMESSAGE("Invalid handle '%d'", handle);
        goto finish;
    }

    entry = &state->surf2DUPTable[handle];
    if (entry->width == 0)
    {
        eStatus = MOS_STATUS_INVALID_HANDLE;
        CM_ASSERTMESSAGE("handle '%d' is not set", handle);
        goto finish;
    }

    *entryOut = entry;

finish:
    return eStatus;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get the Buffer Entry
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
__inline MOS_STATUS HalCm_GetBufferEntry(
    PCM_HAL_STATE           state,                                             // [in]  Pointer to CM State
    uint32_t                handle,                                           // [in]  Handle
    PCM_HAL_BUFFER_ENTRY    *entryOut)                                         // [out] Buffer Entry
{
    MOS_STATUS              eStatus;
    PCM_HAL_BUFFER_ENTRY    entry;

    eStatus = MOS_STATUS_SUCCESS;

    if (handle >= state->cmDeviceParam.maxBufferTableSize)
    {
        eStatus = MOS_STATUS_INVALID_HANDLE;
        CM_ASSERTMESSAGE("Invalid handle '%d'", handle);
        goto finish;
    }

    entry = &state->bufferTable[handle];
    if (entry->size == 0)
    {
        eStatus = MOS_STATUS_INVALID_HANDLE;
        CM_ASSERTMESSAGE("handle '%d' is not set", handle);
        goto finish;
    }

    *entryOut = entry;

finish:
    return eStatus;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get the Surface2D Entry
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
__inline MOS_STATUS HalCm_GetSurface2DEntry(
    PCM_HAL_STATE           state,                                             // [in]  Pointer to CM State
    uint32_t                handle,                                           // [in]  Handle
    PCM_HAL_SURFACE2D_ENTRY    *entryOut)                                         // [out] Buffer Entry
{
    MOS_STATUS                 eStatus;
    PCM_HAL_SURFACE2D_ENTRY    entry;

    eStatus = MOS_STATUS_SUCCESS;

    if (handle >= state->cmDeviceParam.max2DSurfaceTableSize)
    {
        eStatus = MOS_STATUS_INVALID_HANDLE;
        CM_ASSERTMESSAGE("Invalid handle '%d'", handle);
        goto finish;
    }

    entry = &state->umdSurf2DTable[handle];
    if ((entry->width == 0)||(entry->height == 0))
    {
        eStatus = MOS_STATUS_INVALID_HANDLE;
        CM_ASSERTMESSAGE("handle '%d' is not set", handle);
        goto finish;
    }

    *entryOut = entry;

finish:
    return eStatus;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get the 3D Entry
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
__inline MOS_STATUS HalCm_Get3DResourceEntry(
    PCM_HAL_STATE               state,                                         // [in]  Pointer to CM State
    uint32_t                    handle,                                       // [in]  Handle
    PCM_HAL_3DRESOURCE_ENTRY    *entryOut)                                     // [out] Buffer Entry
{
    MOS_STATUS                  eStatus;
    PCM_HAL_3DRESOURCE_ENTRY    entry;

    eStatus = MOS_STATUS_SUCCESS;

    if (handle >= state->cmDeviceParam.max3DSurfaceTableSize)
    {
        eStatus = MOS_STATUS_INVALID_HANDLE;
        CM_ASSERTMESSAGE("Invalid handle '%d'", handle);
        goto finish;
    }

    entry = &state->surf3DTable[handle];
    if (Mos_ResourceIsNull(&entry->osResource))
    {
        eStatus = MOS_STATUS_INVALID_HANDLE;
        CM_ASSERTMESSAGE("3D handle '%d' is not set", handle);
        goto finish;
    }

    *entryOut = entry;

finish:
    return eStatus;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Allocates and sets up Task Param memory structure
//| Return:     true if enabled
//| Note:       A single layer of memory is allocated to avoid fragmentation
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_AllocateTables(
    PCM_HAL_STATE   state)         // [in] Pointer to HAL CM state
{
    MOS_STATUS              eStatus = MOS_STATUS_SUCCESS;
    PCM_HAL_DEVICE_PARAM    deviceParam;
    uint8_t                 *pb;
    uint32_t                lookUpTableSize;
    uint32_t                samplerTableSize;
    uint32_t                vmeTableSize;
    uint32_t                sampler8x8TableSize;
    uint32_t                taskStatusTableSize;
    uint32_t                bt2DIndexTableSize;
    uint32_t                bt2DUPIndexTableSize;
    uint32_t                bt3DIndexTableSize;
    uint32_t                btbufferIndexTableSize;
    uint32_t                samplerIndexTableSize;
    uint32_t                vmeIndexTableSize;
    uint32_t                sampler8x8IndexTableSize;
    uint32_t                bufferTableSize;
    uint32_t                i2DSURFUPTableSize;
    uint32_t                i3DSurfTableSize;
    uint32_t                size;
    uint32_t                i2DSURFTableSize;

    deviceParam  = &state->cmDeviceParam;

    lookUpTableSize        = deviceParam->max2DSurfaceTableSize    *
                              sizeof(CMLOOKUP_ENTRY);
    i2DSURFTableSize        = deviceParam->max2DSurfaceTableSize    *
                            sizeof(CM_HAL_SURFACE2D_ENTRY);
    bufferTableSize        = deviceParam->maxBufferTableSize       *
                              sizeof(CM_HAL_BUFFER_ENTRY);
    i2DSURFUPTableSize      = deviceParam->max2DSurfaceUPTableSize  *
                              sizeof(CM_HAL_SURFACE2D_UP_ENTRY);
    i3DSurfTableSize        = deviceParam->max3DSurfaceTableSize    *
                              sizeof(CM_HAL_3DRESOURCE_ENTRY);
    samplerTableSize       = deviceParam->maxSamplerTableSize      *
                              sizeof(MHW_SAMPLER_STATE_PARAM);
    sampler8x8TableSize    = deviceParam->maxSampler8x8TableSize      *
                              sizeof(CM_HAL_SAMPLER_8X8_ENTRY);
    taskStatusTableSize    = deviceParam->maxTasks                 * sizeof(char);
    bt2DIndexTableSize     = deviceParam->max2DSurfaceTableSize    * sizeof(CM_HAL_MULTI_USE_BTI_ENTRY);
    bt2DUPIndexTableSize   = deviceParam->max2DSurfaceUPTableSize  * sizeof(CM_HAL_MULTI_USE_BTI_ENTRY);
    bt3DIndexTableSize     = deviceParam->max3DSurfaceTableSize    * sizeof(CM_HAL_MULTI_USE_BTI_ENTRY);
    btbufferIndexTableSize = deviceParam->maxBufferTableSize       * sizeof(CM_HAL_MULTI_USE_BTI_ENTRY);
    samplerIndexTableSize  = deviceParam->maxSamplerTableSize      * sizeof(char);
    sampler8x8IndexTableSize = deviceParam->maxSampler8x8TableSize      * sizeof(char);

    size                   = lookUpTableSize          +
                              i2DSURFTableSize          +
                              bufferTableSize          +
                              i2DSURFUPTableSize        +
                              i3DSurfTableSize          +
                              samplerTableSize         +
                              sampler8x8TableSize      +
                              taskStatusTableSize      +
                              bt2DIndexTableSize       +
                              bt2DUPIndexTableSize     +
                              bt3DIndexTableSize       +
                              btbufferIndexTableSize   +
                              samplerIndexTableSize    +
                              sampler8x8IndexTableSize;

    state->tableMemories = MOS_AllocAndZeroMemory(size);
    CM_CHK_NULL_GOTOFINISH_MOSERROR(state->tableMemories);
    pb                          = (uint8_t*)state->tableMemories;

    state->surf2DTable        = (PCMLOOKUP_ENTRY)pb;
    pb                          += lookUpTableSize;

    state->umdSurf2DTable     = (PCM_HAL_SURFACE2D_ENTRY)pb;
    pb                          += i2DSURFTableSize;

    state->bufferTable        = (PCM_HAL_BUFFER_ENTRY)pb;
    pb                          += bufferTableSize;

    state->surf2DUPTable      = (PCM_HAL_SURFACE2D_UP_ENTRY)pb;
    pb                          += i2DSURFUPTableSize;

    state->surf3DTable        = (PCM_HAL_3DRESOURCE_ENTRY)pb;
    pb                          += i3DSurfTableSize;

    state->samplerTable       = (PMHW_SAMPLER_STATE_PARAM)pb;
    pb                          += samplerTableSize;

    state->sampler8x8Table    = (PCM_HAL_SAMPLER_8X8_ENTRY)pb;
    pb                          += sampler8x8TableSize;

    state->taskStatusTable    = (char *)pb;
    pb                          += taskStatusTableSize;

    state->bti2DIndexTable     = (PCM_HAL_MULTI_USE_BTI_ENTRY)pb;
    pb                          += bt2DIndexTableSize;

    state->bti2DUPIndexTable   = (PCM_HAL_MULTI_USE_BTI_ENTRY)pb;
    pb                          += bt2DUPIndexTableSize;

    state->bti3DIndexTable     = (PCM_HAL_MULTI_USE_BTI_ENTRY)pb;
    pb                          += bt3DIndexTableSize;

    state->btiBufferIndexTable = (PCM_HAL_MULTI_USE_BTI_ENTRY)pb;
    pb                          += btbufferIndexTableSize;

    state->samplerIndexTable  = (char *)pb;
    pb                          += samplerIndexTableSize;

    state->sampler8x8IndexTable  = (char *)pb;
    pb                          += sampler8x8IndexTableSize;

finish:
    return MOS_STATUS_SUCCESS;
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
    uint32_t                 numKernels,
    uint32_t                 numTasks,
    uint32_t                 numCurrentTask)
{
    uint32_t i;
    uint64_t tmpNumTasks;
    uint64_t tmpNumCurrentTask;
    uint64_t tmpNumTasksMask;
    uint64_t tmpNumCurrentTaskMask;

    tmpNumTasks = numTasks;
    tmpNumCurrentTask = numCurrentTask;
    tmpNumTasksMask = tmpNumTasks << 45;
    tmpNumCurrentTaskMask = tmpNumCurrentTask << 42;

    for( i = 0; i < numKernels; ++i )
    {
        pKernels[i]->kernelId |= tmpNumTasksMask;
        pKernels[i]->kernelId |= tmpNumCurrentTaskMask;
    }

    return MOS_STATUS_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Gets the Batch Buffer for rendering. If needed, de-allocate /
//|             allocate the memory for BB
//| Returns:    Result of the operation
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_GetBatchBuffer(
    PCM_HAL_STATE           state,                                             // [in]  Pointer to CM State
    uint32_t                numKernels,                                        // [in]  Number of Kernels
    PCM_HAL_KERNEL_PARAM    *kernels,                                          // [in]  Array for kernel data
    PMHW_BATCH_BUFFER       *batchBufferOut)                                   // [out] Batch Buffer Out
{
    MOS_STATUS              eStatus;
    PMHW_BATCH_BUFFER batchBuffer = nullptr;
    PRENDERHAL_INTERFACE    renderHal;
    int32_t                 size;
    uint32_t                i;
    uint32_t                j;
    uint32_t                k;
    int32_t                 freeIdx;
    uint64_t                kernelIds[CM_MAX_KERNELS_PER_TASK];
    uint64_t                kernelParamsIds[CM_MAX_KERNELS_PER_TASK];
    CM_HAL_BB_DIRTY_STATUS  bbDirtyStatus;
    PCM_HAL_BB_ARGS       bbcmArgs;

    eStatus        = MOS_STATUS_SUCCESS;
    renderHal      = state->renderHal;
    freeIdx        = CM_INVALID_INDEX;
    bbDirtyStatus   = CM_HAL_BB_CLEAN;

    // Align the Batch Buffer size to power of 2
    size = HalCm_GetPow2Aligned(state->taskParam->batchBufferSize);

    MOS_ZeroMemory(&kernelIds, CM_MAX_KERNELS_PER_TASK * sizeof(uint64_t));
    MOS_ZeroMemory(&kernelParamsIds, CM_MAX_KERNELS_PER_TASK * sizeof(uint64_t));

    //Sanity check for batch buffer
    if (size > CM_MAX_BB_SIZE)
    {
        eStatus = MOS_STATUS_EXCEED_MAX_BB_SIZE;
        CM_ASSERTMESSAGE("Batch Buffer Size exeeceds Max '%d'", size);
        goto finish;
    }

    for( i = 0; i < numKernels; ++i )
    {
        // remove upper 16 bits used for kernel binary re-use in GSH
        kernelParamsIds[i] = ((kernels[i])->kernelId << 16 ) >> 16;
    }

#if CM_BATCH_BUFFER_REUSE_ENABLE

    bbDirtyStatus = CM_HAL_BB_CLEAN;
    for (k = 0; k < numKernels; ++k)
    {
        if (kernels[k]->kernelThreadSpaceParam.bbDirtyStatus == CM_HAL_BB_DIRTY)
        {
            bbDirtyStatus = CM_HAL_BB_DIRTY;
            break;
        }
    }

    for (i = 0; i < (uint32_t)state->numBatchBuffers; i++)
    {
        batchBuffer = &state->batchBuffers[i];
        CM_CHK_NULL_GOTOFINISH_MOSERROR(batchBuffer);
        CM_CHK_NULL_GOTOFINISH_MOSERROR(batchBuffer->pPrivateData);

        //if (!Mos_ResourceIsNull(&batchBuffer->OsResource) && (!batchBuffer->bBusy))
        if (!Mos_ResourceIsNull(&batchBuffer->OsResource))
        {
            MOS_FillMemory(kernelIds, sizeof(uint64_t)*CM_MAX_KERNELS_PER_TASK, 0);
            for (j = 0; j < numKernels; j ++)
            {
                kernelIds[j] = kernelParamsIds[j];
            }

            bbcmArgs = (PCM_HAL_BB_ARGS)batchBuffer->pPrivateData;
            if (RtlEqualMemory(kernelIds, bbcmArgs->kernelIds, sizeof(uint64_t)*CM_MAX_KERNELS_PER_TASK))
            {
                if( batchBuffer->bBusy && bbDirtyStatus == CM_HAL_BB_DIRTY )
                {
                    bbcmArgs->latest = false;
                }
                else if( bbcmArgs->latest == true )
                {
                    break;
                }
            }
        }
    }
    if (i < (uint32_t)state->numBatchBuffers)
    {
        CM_CHK_NULL_GOTOFINISH_MOSERROR(batchBuffer);
        CM_CHK_NULL_GOTOFINISH_MOSERROR(batchBuffer->pPrivateData);
        bbcmArgs = (PCM_HAL_BB_ARGS)batchBuffer->pPrivateData;

        bbcmArgs->refCount ++;
        batchBuffer->iCurrent   = 0;
        batchBuffer->dwSyncTag  = 0;
        batchBuffer->iRemaining = batchBuffer->iSize;
        *batchBufferOut   = batchBuffer;
        eStatus      = MOS_STATUS_SUCCESS;
        goto finish;
    }
#endif

    for (i = 0; i < (uint32_t)state->numBatchBuffers; i++)
    {
        batchBuffer = &state->batchBuffers[i];
        CM_CHK_NULL_GOTOFINISH_MOSERROR(batchBuffer);
        // No holes in the array of batch buffers
        if (Mos_ResourceIsNull(&batchBuffer->OsResource))
        {
            freeIdx = i;
            break;
        }
    }
    if (freeIdx == CM_INVALID_INDEX)
    {
        for (i = 0; i < (uint32_t)state->numBatchBuffers; i++)
        {
            batchBuffer = &state->batchBuffers[i];
            CM_CHK_NULL_GOTOFINISH_MOSERROR(batchBuffer);
            CM_CHK_NULL_GOTOFINISH_MOSERROR(batchBuffer->pPrivateData);
            bbcmArgs = (PCM_HAL_BB_ARGS)batchBuffer->pPrivateData;
            if (!batchBuffer->bBusy)
            {
                if (batchBuffer->iSize >= size)
                {
                    batchBuffer->iCurrent   = 0;
                    batchBuffer->iRemaining = batchBuffer->iSize;
                    batchBuffer->dwSyncTag  = 0;

                    bbcmArgs->refCount = 1;
                    for (i = 0; i <numKernels; i ++)
                    {
                        bbcmArgs->kernelIds[i] = kernelParamsIds[i];
                    }

                    bbcmArgs->latest = true;

                    *batchBufferOut   = batchBuffer;
                    eStatus = MOS_STATUS_SUCCESS;
                    goto finish;
                }

                if (freeIdx == CM_INVALID_INDEX)
                {
                    freeIdx = i;
                }
            }
        }
    }
    if (freeIdx == CM_INVALID_INDEX)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        CM_ASSERTMESSAGE("No batch buffer available");
        goto finish;
    }

    batchBuffer = &state->batchBuffers[freeIdx];
    CM_CHK_NULL_GOTOFINISH_MOSERROR(batchBuffer);
    CM_CHK_NULL_GOTOFINISH_MOSERROR(batchBuffer->pPrivateData);
    bbcmArgs = (PCM_HAL_BB_ARGS)batchBuffer->pPrivateData;
    bbcmArgs->refCount = 1;
    for (i = 0; i <numKernels; i ++)
    {
        bbcmArgs->kernelIds[i] =  kernelParamsIds[i];
    }

    bbcmArgs->latest = true;

    if (!Mos_ResourceIsNull(&batchBuffer->OsResource))
    {
        // Deallocate Batch Buffer
        CM_CHK_MOSSTATUS_GOTOFINISH(renderHal->pfnFreeBB(renderHal, batchBuffer));
    }

    // Allocate Batch Buffer
    CM_CHK_MOSSTATUS_GOTOFINISH(renderHal->pfnAllocateBB(renderHal, batchBuffer, size));
    *batchBufferOut = batchBuffer;

finish:
    return eStatus;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Parse the Kernel and populate the Task Param structure
//| Return:     Result of the operation
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_ParseTask(
    PCM_HAL_STATE               state,                                         // [in] Pointer to HAL CM state
    PCM_HAL_EXEC_TASK_PARAM     execParam)                                     // [in] Pointer to Exec Task Param
{
    MOS_STATUS                  eStatus;
    PCM_HAL_TASK_PARAM          taskParam;
    PCM_HAL_KERNEL_PARAM        kernelParam;
    uint32_t                    hdrSize;
    uint32_t                    totalThreads;
    uint32_t                    krn;
    uint32_t                    curbeOffset;
    PMHW_VFE_SCOREBOARD         scoreboardParams;
    uint32_t                    hasThreadArg;
    bool                        nonstallingScoreboardEnable;
    CM_HAL_DEPENDENCY           vfeDependencyInfo;
    PCM_HAL_KERNEL_THREADSPACE_PARAM kernelTSParam;
    uint32_t                    i, j, k;
    uint8_t                     reuseBBUpdateMask;
    bool                        bitIsSet;
    PCM_HAL_MASK_AND_RESET      dependencyMask;
    uint32_t                    uSurfaceNumber;
    uint32_t                    uSurfaceIndex;
    bool                        threadArgExists;

    eStatus                           = MOS_STATUS_SUCCESS;
    curbeOffset                      = 0;
    totalThreads                      = 0;
    taskParam                         = state->taskParam;
    taskParam->batchBufferSize        = 0;
    hasThreadArg                       = 0;
    nonstallingScoreboardEnable       = true;
    reuseBBUpdateMask                  = 0;
    bitIsSet                           = false;
    threadArgExists                    = false;
    hdrSize = state->renderHal->pHwSizes->dwSizeMediaObjectHeaderCmd;
    taskParam->dependencyPattern      = execParam->dependencyPattern;
    taskParam->threadSpaceWidth       = execParam->threadSpaceWidth;
    taskParam->threadSpaceHeight      = execParam->threadSpaceHeight;
    taskParam->walkingPattern         = execParam->walkingPattern;
    taskParam->walkingParamsValid     = execParam->walkingParamsValid;
    taskParam->dependencyVectorsValid = execParam->dependencyVectorsValid;
    if( taskParam->walkingParamsValid )
    {
        taskParam->walkingParams = execParam->walkingParams;
    }
    if( taskParam->dependencyVectorsValid )
    {
        taskParam->dependencyVectors = execParam->dependencyVectors;
    }
    taskParam->kernelDebugEnabled  = (uint32_t)execParam->kernelDebugEnabled;
    //GT-PIN
    taskParam->surfEntryInfoArrays  = execParam->surfEntryInfoArrays;

    taskParam->surfacePerBT = 0;

    taskParam->colorCountMinusOne  = execParam->colorCountMinusOne;
    taskParam->mediaWalkerGroupSelect = execParam->mediaWalkerGroupSelect;

    if (execParam->threadCoordinates)
    {
        taskParam->threadCoordinates = execParam->threadCoordinates;
    }

    taskParam->dependencyMasks = execParam->dependencyMasks;
    taskParam->syncBitmap = execParam->syncBitmap;
    taskParam->conditionalEndBitmap = execParam->conditionalEndBitmap;
    MOS_SecureMemcpy(taskParam->conditionalEndInfo, sizeof(taskParam->conditionalEndInfo), execParam->conditionalEndInfo, sizeof(execParam->conditionalEndInfo));

    taskParam->numKernels = execParam->numKernels;
    taskParam->taskConfig   = execParam->taskConfig;
    state->walkerParams.CmWalkerEnable = true;
    state->renderHal->IsMDFLoad = (taskParam->taskConfig.turboBoostFlag == CM_TURBO_BOOST_ENABLE);

    for (krn = 0; krn < execParam->numKernels; krn++)
    {
        if ((execParam->kernels[krn] == nullptr) ||
            (execParam->kernelSizes[krn] == 0))
        {
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            CM_ASSERTMESSAGE("Invalid Kernel data");
            goto finish;
        }

        kernelParam    = (PCM_HAL_KERNEL_PARAM)execParam->kernels[krn];
        PCM_INDIRECT_SURFACE_INFO       indirectSurfaceInfo = kernelParam->indirectDataParam.surfaceInfo;
        uSurfaceNumber = 0;
        if (kernelParam->indirectDataParam.surfaceCount)
        {
            uSurfaceIndex = 0;
            for (i = 0; i < kernelParam->indirectDataParam.surfaceCount; i++)
            {
                uSurfaceIndex = (indirectSurfaceInfo + i)->bindingTableIndex > uSurfaceIndex ? ((indirectSurfaceInfo + i)->bindingTableIndex + (indirectSurfaceInfo + i)->numBTIPerSurf - 1) : uSurfaceIndex;
                uSurfaceNumber = uSurfaceNumber + (indirectSurfaceInfo + i)->numBTIPerSurf;
            }
            taskParam->surfacePerBT = taskParam->surfacePerBT > uSurfaceIndex ? taskParam->surfacePerBT : uSurfaceIndex;
        }

        uSurfaceNumber += kernelParam->numSurfaces;
        taskParam->surfacePerBT = taskParam->surfacePerBT < uSurfaceNumber ?
                                            uSurfaceNumber : taskParam->surfacePerBT;

        // 26Z must be media object because by default it uses thread dependency mask
        // if there is no thread payload and dependency is not WAVEFRONT26Z, check if walker can be used
        if ( kernelParam->payloadSize == 0)
        {
            //per-kernel thread space is avaiable, and check it at first
            if((kernelParam->kernelThreadSpaceParam.threadSpaceWidth != 0) &&
                (kernelParam->kernelThreadSpaceParam.patternType != CM_WAVEFRONT26Z) &&
                (kernelParam->kernelThreadSpaceParam.patternType != CM_WAVEFRONT26ZI) &&
                (kernelParam->kernelThreadSpaceParam.threadCoordinates == nullptr))
            {
                kernelParam->walkerParams.cmWalkerEnable = true;
                kernelParam->walkerParams.groupIdLoopSelect = execParam->mediaWalkerGroupSelect;
            }
            else if (kernelParam->kernelThreadSpaceParam.threadSpaceWidth == 0)
            {
                //Check per-task thread space setting
                if (state->taskParam->threadCoordinates)
                {
                     if (state->taskParam->threadCoordinates[krn] == nullptr)
                     {
                        kernelParam->walkerParams.cmWalkerEnable = true;
                        kernelParam->walkerParams.groupIdLoopSelect = execParam->mediaWalkerGroupSelect;
                     }
                }
                else
                {
                    kernelParam->walkerParams.cmWalkerEnable = true;
                    kernelParam->walkerParams.groupIdLoopSelect = execParam->mediaWalkerGroupSelect;
                }
            }
        }

        //Media walker mode will be disabled if any kernel need use media object, we don't support mixed working modes
        state->walkerParams.CmWalkerEnable &= kernelParam->walkerParams.cmWalkerEnable;

        if (!state->walkerParams.CmWalkerEnable)
        {
            taskParam->batchBufferSize +=
                kernelParam->numThreads * (hdrSize +  MOS_MAX(kernelParam->payloadSize, 4));
        }

        totalThreads += kernelParam->numThreads;

    }

    taskParam->batchBufferSize += CM_EXTRA_BB_SPACE;

    if (state->cmHalInterface->IsScoreboardParamNeeded())
    {
        scoreboardParams = &state->scoreboardParams;
        scoreboardParams->ScoreboardMask = 0;
        scoreboardParams->ScoreboardType = nonstallingScoreboardEnable;

        // set VFE scoreboarding information from union of kernel dependency vectors
        MOS_ZeroMemory(&vfeDependencyInfo, sizeof(CM_HAL_DEPENDENCY));
        for (krn = 0; krn < execParam->numKernels; krn++)
        {
            kernelParam = execParam->kernels[krn];
            kernelTSParam = &kernelParam->kernelThreadSpaceParam;

            // calculate union dependency vector of all kernels with dependency
            if (kernelTSParam->dependencyInfo.count || kernelTSParam->dependencyVectorsValid)
            {
                if (vfeDependencyInfo.count == 0)
                {
                    if (kernelTSParam->dependencyInfo.count)
                    {
                        MOS_SecureMemcpy(&vfeDependencyInfo, sizeof(CM_HAL_DEPENDENCY), &kernelTSParam->dependencyInfo, sizeof(CM_HAL_DEPENDENCY));
                    }
                    else if (kernelTSParam->dependencyVectorsValid)
                    {
                        MOS_SecureMemcpy(&vfeDependencyInfo, sizeof(CM_HAL_DEPENDENCY), &kernelTSParam->dependencyVectors, sizeof(CM_HAL_DEPENDENCY));
                    }
                    kernelTSParam->globalDependencyMask = (1 << vfeDependencyInfo.count) - 1;
                }
                else
                {
                    uint32_t count = 0;
                    CM_HAL_DEPENDENCY dependencyInfo;
                    if (kernelTSParam->dependencyVectorsValid)
                    {
                        count = kernelTSParam->dependencyVectors.count;
                        MOS_SecureMemcpy(&dependencyInfo.deltaX, sizeof(int32_t) * count, &kernelTSParam->dependencyVectors.deltaX, sizeof(int32_t) * count);
                        MOS_SecureMemcpy(&dependencyInfo.deltaY, sizeof(int32_t) * count, &kernelTSParam->dependencyVectors.deltaY, sizeof(int32_t) * count);
                    }
                    else
                    {
                        count = kernelTSParam->dependencyInfo.count;
                        MOS_SecureMemcpy(&dependencyInfo.deltaX, sizeof(int32_t) * count, &kernelTSParam->dependencyInfo.deltaX, sizeof(int32_t) * count);
                        MOS_SecureMemcpy(&dependencyInfo.deltaY, sizeof(int32_t) * count, &kernelTSParam->dependencyInfo.deltaY, sizeof(int32_t) * count);
                    }

                    for (j = 0; j < count; ++j)
                    {
                        for (k = 0; k < vfeDependencyInfo.count; ++k)
                        {
                            if ((dependencyInfo.deltaX[j] == vfeDependencyInfo.deltaX[k]) &&
                                (dependencyInfo.deltaY[j] == vfeDependencyInfo.deltaY[k]))
                            {
                                CM_HAL_SETBIT(kernelTSParam->globalDependencyMask, k);
                                break;
                            }
                        }
                        if (k == vfeDependencyInfo.count)
                        {
                            vfeDependencyInfo.deltaX[vfeDependencyInfo.count] = dependencyInfo.deltaX[j];
                            vfeDependencyInfo.deltaY[vfeDependencyInfo.count] = dependencyInfo.deltaY[j];
                            CM_HAL_SETBIT(kernelTSParam->globalDependencyMask, vfeDependencyInfo.count);
                            vfeDependencyInfo.count++;
                        }
                    }
                }
            }

            reuseBBUpdateMask |= kernelTSParam->reuseBBUpdateMask;
        }

        if (vfeDependencyInfo.count > CM_HAL_MAX_DEPENDENCY_COUNT)
        {
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            CM_ASSERTMESSAGE("Union of kernel dependencies exceeds max dependency count (8)");
            goto finish;
        }

        scoreboardParams->ScoreboardMask = (uint8_t)vfeDependencyInfo.count;
        for (i = 0; i < scoreboardParams->ScoreboardMask; ++i)
        {
            scoreboardParams->ScoreboardDelta[i].x = vfeDependencyInfo.deltaX[i];
            scoreboardParams->ScoreboardDelta[i].y = vfeDependencyInfo.deltaY[i];
        }

        //If no dependency defined in kernel data, then check per-task thread space setting
        if (scoreboardParams->ScoreboardMask == 0)
        {
            if (taskParam->dependencyVectorsValid)
            {
                scoreboardParams->ScoreboardMask = (uint8_t)taskParam->dependencyVectors.count;
                for (uint32_t i = 0; i < scoreboardParams->ScoreboardMask; ++i)
                {
                    scoreboardParams->ScoreboardDelta[i].x = taskParam->dependencyVectors.deltaX[i];
                    scoreboardParams->ScoreboardDelta[i].y = taskParam->dependencyVectors.deltaY[i];
                }
            }
            else
            {
                switch (taskParam->dependencyPattern)
                {
                case CM_NONE_DEPENDENCY:
                    break;

                case CM_VERTICAL_WAVE:
                    scoreboardParams->ScoreboardMask = 1;
                    scoreboardParams->ScoreboardDelta[0].x = 0xF; // -1 in uint8_t:4
                    scoreboardParams->ScoreboardDelta[0].y = 0;
                    break;

                case CM_HORIZONTAL_WAVE:
                    scoreboardParams->ScoreboardMask = 1;
                    scoreboardParams->ScoreboardDelta[0].x = 0;
                    scoreboardParams->ScoreboardDelta[0].y = 0xF; // -1 in uint8_t:4
                    break;

                case CM_WAVEFRONT:
                    scoreboardParams->ScoreboardMask = 3;
                    scoreboardParams->ScoreboardDelta[0].x = 0xF; // -1 in uint8_t:4
                    scoreboardParams->ScoreboardDelta[0].y = 0;
                    scoreboardParams->ScoreboardDelta[1].x = 0xF; // -1 in uint8_t:4
                    scoreboardParams->ScoreboardDelta[1].y = 0xF; // -1 in uint8_t:4
                    scoreboardParams->ScoreboardDelta[2].x = 0;
                    scoreboardParams->ScoreboardDelta[2].y = 0xF; // -1 in uint8_t:4
                    break;

                case CM_WAVEFRONT26:
                    scoreboardParams->ScoreboardMask = 4;
                    scoreboardParams->ScoreboardDelta[0].x = 0xF; // -1 in uint8_t:4
                    scoreboardParams->ScoreboardDelta[0].y = 0;
                    scoreboardParams->ScoreboardDelta[1].x = 0xF; // -1 in uint8_t:4
                    scoreboardParams->ScoreboardDelta[1].y = 0xF; // -1 in uint8_t:4
                    scoreboardParams->ScoreboardDelta[2].x = 0;
                    scoreboardParams->ScoreboardDelta[2].y = 0xF; // -1 in uint8_t:4
                    scoreboardParams->ScoreboardDelta[3].x = 1;
                    scoreboardParams->ScoreboardDelta[3].y = 0xF; // -1 in uint8_t:4
                    break;

                case CM_WAVEFRONT26Z:
                case CM_WAVEFRONT26ZIG:
                    scoreboardParams->ScoreboardMask = 5;
                    scoreboardParams->ScoreboardDelta[0].x = 0xF; // -1 in uint8_t:4
                    scoreboardParams->ScoreboardDelta[0].y = 1;
                    scoreboardParams->ScoreboardDelta[1].x = 0xF; // -1 in uint8_t:4
                    scoreboardParams->ScoreboardDelta[1].y = 0;
                    scoreboardParams->ScoreboardDelta[2].x = 0xF; // -1 in uint8_t:4
                    scoreboardParams->ScoreboardDelta[2].y = 0xF; // -1 in uint8_t:4
                    scoreboardParams->ScoreboardDelta[3].x = 0;
                    scoreboardParams->ScoreboardDelta[3].y = 0xF; // -1 in uint8_t:4
                    scoreboardParams->ScoreboardDelta[4].x = 1;
                    scoreboardParams->ScoreboardDelta[4].y = 0xF; // -1 in uint8_t:4
                    break;

                case CM_WAVEFRONT26ZI:
                    scoreboardParams->ScoreboardMask = 7;
                    scoreboardParams->ScoreboardDelta[0].x = 0xF; // -1 in uint8_t:4
                    scoreboardParams->ScoreboardDelta[0].y = 1;
                    scoreboardParams->ScoreboardDelta[1].x = 0xE;  // -2
                    scoreboardParams->ScoreboardDelta[1].y = 0;
                    scoreboardParams->ScoreboardDelta[2].x = 0xF; // -1 in uint8_t:4
                    scoreboardParams->ScoreboardDelta[2].y = 0;
                    scoreboardParams->ScoreboardDelta[3].x = 0xF; // -1 in uint8_t:4
                    scoreboardParams->ScoreboardDelta[3].y = 0xF; // -1 in uint8_t:4
                    scoreboardParams->ScoreboardDelta[4].x = 0;
                    scoreboardParams->ScoreboardDelta[4].y = 0xF; // -1 in uint8_t:4
                    scoreboardParams->ScoreboardDelta[5].x = 1;
                    scoreboardParams->ScoreboardDelta[5].y = 0xF; // -1 in uint8_t:4
                    scoreboardParams->ScoreboardDelta[6].x = 1;
                    scoreboardParams->ScoreboardDelta[6].y = 0;
                    break;

                case CM_WAVEFRONT26X:
                    scoreboardParams->ScoreboardMask = 7;
                    scoreboardParams->ScoreboardDelta[0].x = 0xF;
                    scoreboardParams->ScoreboardDelta[0].y = 3;
                    scoreboardParams->ScoreboardDelta[1].x = 0xF;
                    scoreboardParams->ScoreboardDelta[1].y = 1;
                    scoreboardParams->ScoreboardDelta[2].x = 0xF;
                    scoreboardParams->ScoreboardDelta[2].y = 0xF;
                    scoreboardParams->ScoreboardDelta[3].x = 0;
                    scoreboardParams->ScoreboardDelta[3].y = 0xF;
                    scoreboardParams->ScoreboardDelta[4].x = 0;
                    scoreboardParams->ScoreboardDelta[4].y = 0xE;
                    scoreboardParams->ScoreboardDelta[5].x = 0;
                    scoreboardParams->ScoreboardDelta[5].y = 0xD;
                    scoreboardParams->ScoreboardDelta[6].x = 1;
                    scoreboardParams->ScoreboardDelta[6].y = 0xD;
                    break;

                default:
                    taskParam->dependencyPattern = CM_NONE_DEPENDENCY;
                    break;

                }
            }
        }
    }
    //Set size of surface binding table size
    CM_SURFACE_BTI_INFO surfBTIInfo;
    state->cmHalInterface->GetHwSurfaceBTIInfo(&surfBTIInfo);

    taskParam->surfacePerBT += surfBTIInfo.normalSurfaceStart ;

    // add one if kernel debugger is enabled
    if (execParam->kernelDebugEnabled)
    {
        taskParam->surfacePerBT += CM_RESERVED_SURFACE_NUMBER_FOR_KERNEL_DEBUG;
    }

    //If global surface is used and current surface bt size less than the max index of reserved surfaces
    //use set it as max bti size
    if ((execParam->globalSurfaceUsed) && (taskParam->surfacePerBT < surfBTIInfo.reservedSurfaceEnd))
    {
        taskParam->surfacePerBT = CM_MAX_STATIC_SURFACE_STATES_PER_BT;
    }

    //Make sure surfacePerBT do not exceed CM_MAX_STATIC_SURFACE_STATES_PER_BT
    taskParam->surfacePerBT = MOS_MIN(CM_MAX_STATIC_SURFACE_STATES_PER_BT, taskParam->surfacePerBT);

    if( taskParam->dependencyMasks )
    {
        for (krn = 0; krn < execParam->numKernels; krn++)
        {
            kernelParam    = execParam->kernels[krn];
            dependencyMask = taskParam->dependencyMasks[krn];
            if( dependencyMask )
            {
                for( i = 0; i < kernelParam->numThreads; ++i )
                {
                    reuseBBUpdateMask |= dependencyMask[i].resetMask;
                }
            }
        }
    }

    CM_HAL_CHECKBIT_IS_SET(bitIsSet, reuseBBUpdateMask, CM_NO_BATCH_BUFFER_REUSE_BIT_POS);
    if( bitIsSet || reuseBBUpdateMask == 0 )
    {
        taskParam->reuseBBUpdateMask = 0;
    }
    else
    {
        taskParam->reuseBBUpdateMask = 1;
    }

    threadArgExists = HalCm_GetTaskHasThreadArg(execParam->kernels, execParam->numKernels);

    // For media object with thread arg, only support up to CM_MAX_USER_THREADS (512*512) threads
    // otherwise can support up to 262144 media object commands in batch buffer
    if (!state->walkerParams.CmWalkerEnable) {
        if (!threadArgExists)
        {
            if(totalThreads > CM_MAX_USER_THREADS_NO_THREADARG)
            {
                eStatus = MOS_STATUS_INVALID_PARAMETER;
                CM_ASSERTMESSAGE(
                    "Total task threads '%d' exceeds max allowed threads '%d'",
                    totalThreads,
                    CM_MAX_USER_THREADS_NO_THREADARG);
                goto finish;
            }
        }
        else
        {
            if (totalThreads > CM_MAX_USER_THREADS)
            {
                eStatus = MOS_STATUS_INVALID_PARAMETER;
                CM_ASSERTMESSAGE(
                    "Total task threads '%d' exceeds max allowed threads '%d'",
                    totalThreads,
                    CM_MAX_USER_THREADS);
                goto finish;
            }
        }
    }

    taskParam->queueOption = execParam->queueOption;

finish:
    return eStatus;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Parse the Kernel and populate the Task Param structure
//| Return:     Result of the operation
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_ParseGroupTask(
    PCM_HAL_STATE                       state,           // [in] Pointer to HAL CM state
    PCM_HAL_EXEC_GROUP_TASK_PARAM       execGroupParam)  // [in] Pointer to Exec Task Param
{
    PCM_HAL_TASK_PARAM      taskParam      = state->taskParam;
    MOS_STATUS              eStatus        = MOS_STATUS_SUCCESS;
    PCM_HAL_KERNEL_PARAM    kernelParam    = nullptr;
    uint32_t                uSurfaceIndex;

    taskParam->surfEntryInfoArrays  = execGroupParam->surEntryInfoArrays;  //GT-PIN
    taskParam->batchBufferSize = 0;
    taskParam->kernelDebugEnabled  = (uint32_t)execGroupParam->kernelDebugEnabled;

    taskParam->numKernels = execGroupParam->numKernels;
    taskParam->syncBitmap = execGroupParam->syncBitmap;
    taskParam->conditionalEndBitmap = execGroupParam->conditionalEndBitmap;
    MOS_SecureMemcpy(taskParam->conditionalEndInfo, sizeof(taskParam->conditionalEndInfo),
                     execGroupParam->conditionalEndInfo, sizeof(execGroupParam->conditionalEndInfo));

    taskParam->taskConfig = execGroupParam->taskConfig;

    MOS_SecureMemcpy(taskParam->krnExecCfg, sizeof(taskParam->krnExecCfg),
                     execGroupParam->krnExecCfg, sizeof(execGroupParam->krnExecCfg));

    for (uint32_t krn = 0; krn < execGroupParam->numKernels; krn ++)
    {
        kernelParam = execGroupParam->kernels[krn];
        PCM_INDIRECT_SURFACE_INFO       indirectSurfaceInfo = kernelParam->indirectDataParam.surfaceInfo;
        uint32_t uSurfaceNumber = 0;
        if (kernelParam->indirectDataParam.surfaceCount)
        {
            uSurfaceIndex = 0;
            for (uint32_t i = 0; i < kernelParam->indirectDataParam.surfaceCount; i++)
            {
                uSurfaceIndex = (indirectSurfaceInfo + i)->bindingTableIndex > uSurfaceIndex ? (indirectSurfaceInfo + i)->bindingTableIndex : uSurfaceIndex;
                uSurfaceNumber++;
            }
            taskParam->surfacePerBT = taskParam->surfacePerBT > uSurfaceIndex ? taskParam->surfacePerBT : uSurfaceIndex;
        }

        uSurfaceNumber += kernelParam->numSurfaces;

        taskParam->surfacePerBT = taskParam->surfacePerBT < uSurfaceNumber ?
                                            uSurfaceNumber : taskParam->surfacePerBT;
    }

    CM_SURFACE_BTI_INFO surfBTIInfo;
    state->cmHalInterface->GetHwSurfaceBTIInfo(&surfBTIInfo);

    taskParam->surfacePerBT += surfBTIInfo.normalSurfaceStart ;

    // add one if kernel debugger is enabled
    if (execGroupParam->kernelDebugEnabled)
    {
        taskParam->surfacePerBT += CM_RESERVED_SURFACE_NUMBER_FOR_KERNEL_DEBUG;
    }

    //If global surface is used and current surface bt size less than the max index of reserved surfaces
    //use set it as max bti size
    if ((execGroupParam->globalSurfaceUsed) &&
        (taskParam->surfacePerBT < surfBTIInfo.reservedSurfaceEnd))
    {
        taskParam->surfacePerBT = CM_MAX_STATIC_SURFACE_STATES_PER_BT;
    }

    //Make sure surfacePerBT do not exceed CM_MAX_STATIC_SURFACE_STATES_PER_BT
    taskParam->surfacePerBT = MOS_MIN(CM_MAX_STATIC_SURFACE_STATES_PER_BT, taskParam->surfacePerBT);

    taskParam->queueOption = execGroupParam->queueOption;
    taskParam->mosVeHintParams = execGroupParam->mosVeHintParams;

    return eStatus;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Parse the Kernel and populate the Hints Task Param structure
//| Return:     Result of the operation
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_ParseHintsTask(
    PCM_HAL_STATE                     state,                                         // [in] Pointer to HAL CM state
    PCM_HAL_EXEC_HINTS_TASK_PARAM     execHintsParam)
{
    MOS_STATUS                        eStatus;
    PCM_HAL_TASK_PARAM                taskParam;
    PCM_HAL_KERNEL_PARAM              kernelParam;
    uint32_t                          hdrSize;
    uint32_t                          totalThreads;
    uint32_t                          krn;
    uint32_t                          curbeOffset;
    PMHW_VFE_SCOREBOARD               scoreboardParams;
    uint32_t                          hasThreadArg;
    bool                              nonstallingScoreboardEnable;
    bool                              bitIsSet;
    uint8_t                           reuseBBUpdateMask;
    bool                              threadArgExists;

    eStatus                          = MOS_STATUS_SUCCESS;
    krn                              = 0;
    taskParam                        = state->taskParam;
    nonstallingScoreboardEnable      = true;
    bitIsSet                          = false;
    curbeOffset                     = 0;
    hasThreadArg                      = 0;
    totalThreads                     = 0;
    reuseBBUpdateMask                 = 0;
    threadArgExists                   = false;

    hdrSize = state->renderHal->pHwSizes->dwSizeMediaObjectHeaderCmd;
    scoreboardParams = &state->scoreboardParams;

    for( krn = 0; krn < execHintsParam->numKernels; ++krn )
    {
        if ((execHintsParam->kernels[krn] == nullptr) ||
            (execHintsParam->kernelSizes[krn] == 0))
        {
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            CM_ASSERTMESSAGE("Invalid Kernel data");
            goto finish;
        }

        // Parse the kernel Param
        kernelParam =  execHintsParam->kernels[krn];

        // if any kernel disables non-stalling, the non-stalling will be disabled
        nonstallingScoreboardEnable &= (kernelParam->cmFlags & CM_KERNEL_FLAGS_NONSTALLING_SCOREBOARD) ? true : false;

        if (!state->walkerParams.CmWalkerEnable)
        {
            taskParam->batchBufferSize +=
                kernelParam->numThreads * (hdrSize +  MOS_MAX(kernelParam->payloadSize, 4));
        }

        totalThreads += kernelParam->numThreads;

        reuseBBUpdateMask |= kernelParam->kernelThreadSpaceParam.reuseBBUpdateMask;
    }

    CM_HAL_CHECKBIT_IS_SET(bitIsSet, reuseBBUpdateMask, CM_NO_BATCH_BUFFER_REUSE_BIT_POS);
    if( bitIsSet || reuseBBUpdateMask == 0 )
    {
        taskParam->reuseBBUpdateMask = 0;
    }
    else
    {
        taskParam->reuseBBUpdateMask = 1;
    }

    taskParam->batchBufferSize += CM_EXTRA_BB_SPACE;

    scoreboardParams->ScoreboardType = nonstallingScoreboardEnable;

    threadArgExists = HalCm_GetTaskHasThreadArg(execHintsParam->kernels, execHintsParam->numKernels);

    if (!state->walkerParams.CmWalkerEnable) {
        if (!threadArgExists)
        {
            if(totalThreads > CM_MAX_USER_THREADS_NO_THREADARG)
            {
                eStatus = MOS_STATUS_INVALID_PARAMETER;
                CM_ASSERTMESSAGE(
                    "Total task threads '%d' exceeds max allowed threads '%d'",
                    totalThreads,
                    CM_MAX_USER_THREADS_NO_THREADARG);
                goto finish;
            }
        }
        else
        {
            if (totalThreads > CM_MAX_USER_THREADS)
            {
                eStatus = MOS_STATUS_INVALID_PARAMETER;
                CM_ASSERTMESSAGE(
                    "Total task threads '%d' exceeds max allowed threads '%d'",
                    totalThreads,
                    CM_MAX_USER_THREADS);
                goto finish;
            }
        }
    }

    taskParam->queueOption = execHintsParam->queueOption;

finish:
    return eStatus;
}

/*
** check to see if kernel entry is flaged as free or it is null
** used for combining
*/
bool bIsFree( PRENDERHAL_KRN_ALLOCATION kAlloc )
{
    if (kAlloc== nullptr)
    {
        return false;
    }
    else
    {
        if (kAlloc->dwFlags != RENDERHAL_KERNEL_ALLOCATION_FREE)
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
void CmLoadKernel(PCM_HAL_STATE             state,
                  PRENDERHAL_STATE_HEAP     stateHeap,
                  PRENDERHAL_KRN_ALLOCATION kernelAllocation,
                  uint32_t sync,
                  uint32_t count,
                  PRENDERHAL_KERNEL_PARAM   parameters,
                  PCM_HAL_KERNEL_PARAM      kernelParam,
                  MHW_KERNEL_PARAM         *mhwKernelParam,
                  bool                      isCloneEntry)
{
    UNUSED(state);
    if (mhwKernelParam)
    {
        kernelAllocation->iKID        = -1;
        kernelAllocation->iKUID       = mhwKernelParam->iKUID;
        kernelAllocation->iKCID       = mhwKernelParam->iKCID;
        kernelAllocation->dwSync      = sync;
        kernelAllocation->dwCount     = count & 0xFFFFFFFF; // 28 bits
        kernelAllocation->dwFlags     = RENDERHAL_KERNEL_ALLOCATION_USED;
        kernelAllocation->Params      = *parameters;
        kernelAllocation->pMhwKernelParam = mhwKernelParam;

        if (!isCloneEntry)
        {
            // Copy kernel data
            // Copy MovInstruction First
            MOS_SecureMemcpy(stateHeap->pIshBuffer + kernelAllocation->dwOffset,
                kernelParam->movInsDataSize,
                kernelParam->movInsData,
                kernelParam->movInsDataSize);

            // Copy Cm Kernel Binary
            MOS_SecureMemcpy(stateHeap->pIshBuffer + kernelAllocation->dwOffset + kernelParam->movInsDataSize,
                kernelParam->kernelBinarySize - kernelParam->movInsDataSize,
                kernelParam->kernelBinary,
                kernelParam->kernelBinarySize - kernelParam->movInsDataSize);

            // Padding bytes dummy instructions after kernel binary to resolve page fault issue
            MOS_ZeroMemory(stateHeap->pIshBuffer + kernelAllocation->dwOffset + kernelParam->kernelBinarySize, CM_KERNEL_BINARY_PADDING_SIZE);
        }
    }
    else
    {
        kernelAllocation->iKID        = -1;
        kernelAllocation->iKUID       = -1;
        kernelAllocation->iKCID       = -1;
        kernelAllocation->dwSync      = 0;
        FrameTrackerTokenFlat_Clear(&kernelAllocation->trackerToken);
        kernelAllocation->dwCount     = 0;
        kernelAllocation->dwFlags     = RENDERHAL_KERNEL_ALLOCATION_FREE;
        kernelAllocation->pMhwKernelParam = nullptr;
        kernelAllocation->cloneKernelParams.cloneKernelID       = -1;
        kernelAllocation->cloneKernelParams.isClone             = false;
        kernelAllocation->cloneKernelParams.isHeadKernel        = false;
        kernelAllocation->cloneKernelParams.kernelBinaryAllocID = -1;
        kernelAllocation->cloneKernelParams.referenceCount      = 0;
    }
}

/*
** local used supporting function
** Try to find free entry which is big enough to load kernel binary
** If we cannot find one, then return fail, so we will delete more entries
*/
int32_t CmSearchFreeSlotSize(PCM_HAL_STATE state, MHW_KERNEL_PARAM *mhwKernelParam, bool isCloneEntry)
{
    PRENDERHAL_STATE_HEAP     stateHeap;
    PRENDERHAL_KRN_ALLOCATION kernelAllocation;
    int32_t                 kernelAllocationID;
    int32_t                 returnVal = -1;
    int32_t                 neededSize;

    stateHeap          = state->renderHal->pStateHeap;
    kernelAllocation   = stateHeap->pKernelAllocation;

    if (isCloneEntry)
    {
        neededSize = CM_64BYTE;
    }
    else
    {
        neededSize = mhwKernelParam->iSize;
    }

    for (kernelAllocationID = 0;
         kernelAllocationID < state->kernelNumInGsh;
         kernelAllocationID++, kernelAllocation++)
    {
        if(kernelAllocation->dwFlags == RENDERHAL_KERNEL_ALLOCATION_FREE)
        {
            if(state->totalKernelSize[kernelAllocationID] >= neededSize)
            {
                // found free slot which is big enough
                return kernelAllocationID;
            }
        }
    }

    // not found
    return returnVal;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Updates the clone entries' head kernel binary allocation IDs
//|             Function is called after kernel allocations are shifted due to combining neighboring free entries
//| Return:     Result of the operation
//*-----------------------------------------------------------------------------
void HalCm_UpdateCloneKernel(PCM_HAL_STATE state,
    uint32_t shiftPoint,
    CM_SHIFT_DIRECTION shiftDirection,
    uint32_t shiftFactor)
{
    PRENDERHAL_STATE_HEAP       stateHeap;
    PRENDERHAL_KRN_ALLOCATION   kernelAllocation;
    int32_t                     allocationID;

    stateHeap = state->renderHal->pStateHeap;
    kernelAllocation = stateHeap->pKernelAllocation;

    for (allocationID = 0; allocationID < state->kernelNumInGsh; allocationID++, kernelAllocation++)
    {
        kernelAllocation = &(stateHeap->pKernelAllocation[allocationID]);
        if (kernelAllocation->cloneKernelParams.isClone && ((kernelAllocation->cloneKernelParams.kernelBinaryAllocID) > (int32_t)shiftPoint))
        {
            if (shiftDirection == CM_SHIFT_LEFT)
            {
                kernelAllocation->cloneKernelParams.kernelBinaryAllocID = kernelAllocation->cloneKernelParams.kernelBinaryAllocID + shiftFactor;
            }
            else
            {
                kernelAllocation->cloneKernelParams.kernelBinaryAllocID = kernelAllocation->cloneKernelParams.kernelBinaryAllocID - shiftFactor;
            }
        }
    }
}

/*
** local used supporting function
** We found free slot and load kernel to this slot. There are 3 cases (see code)
*/
int32_t CmAddCurrentKernelToFreeSlot(PCM_HAL_STATE state,
                                  int32_t slot,
                                  PRENDERHAL_KERNEL_PARAM parameters,
                                  PCM_HAL_KERNEL_PARAM    kernelParam,
                                  MHW_KERNEL_PARAM       *mhwKernelParam,
                                  CM_CLONE_TYPE           cloneType,
                                  int32_t                 headKernelAllocationID)
{
    PRENDERHAL_STATE_HEAP       stateHeap;
    PRENDERHAL_KRN_ALLOCATION   kernelAllocation, pKernelAllocationN;

    int32_t hr = CM_SUCCESS;
    int32_t i;
    int32_t totalSize, tmpSize, dwOffset, neededSize;
    bool    adjust, isCloneEntry, isHeadKernel, isCloneAsHead, adjustHeadKernelID;
    uint32_t tag;

    stateHeap          = state->renderHal->pStateHeap;
    kernelAllocation   = stateHeap->pKernelAllocation;
    adjustHeadKernelID = false;

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
            neededSize    = mhwKernelParam->iSize;
            isHeadKernel  = true;
            isCloneEntry  = false;
            isCloneAsHead = false;
        }
        break;
        case CM_CLONE_AS_HEAD_KERNEL:
        {
            neededSize    = mhwKernelParam->iSize;
            isHeadKernel  = true;
            isCloneEntry  = false;
            isCloneAsHead = true;
        }
        break;
        case CM_NO_CLONE:
        {
            neededSize    = mhwKernelParam->iSize;
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
    if(stateHeap->pKernelAllocation[slot].iSize == neededSize)
    {
        adjust = false;
    }
    else
    {
        adjust = true;
    }

    if ((state->kernelNumInGsh < state->cmDeviceParam.maxGshKernelEntries) && adjust)
    {
        // we have extra entry to add
        // add new entry and pump index down below
        int32_t lastKernel = state->kernelNumInGsh - 1;
        for(i = lastKernel; i>slot; i--)
        {
            kernelAllocation = &stateHeap->pKernelAllocation[i];
            pKernelAllocationN = &stateHeap->pKernelAllocation[i+1];
            *pKernelAllocationN = *kernelAllocation;
            state->totalKernelSize[i+1] = state->totalKernelSize[i];
        }

        if (lastKernel > slot)
        {
            // update the headKernelAllocationID if it was shifted
            if (headKernelAllocationID > slot)
            {
                headKernelAllocationID++;
                adjustHeadKernelID = true;
            }
        }

        totalSize = state->totalKernelSize[slot];
        tmpSize = neededSize;

        dwOffset = stateHeap->pKernelAllocation[slot].dwOffset;

        // now add new one
        kernelAllocation = &stateHeap->pKernelAllocation[slot];
        if(state->cbbEnabled)
        {
            tag = state->osInterface->pfnGetGpuStatusTag(state->osInterface,
                state->osInterface->CurrentGpuContextOrdinal);
        }
        else
        {
            tag = stateHeap->dwNextTag;
        }

        CmLoadKernel(state, stateHeap, kernelAllocation, tag, stateHeap->dwAccessCounter, parameters, kernelParam, mhwKernelParam, isCloneEntry);
        stateHeap->dwAccessCounter++;

        kernelAllocation->iSize = tmpSize;
        state->totalKernelSize[slot] = MOS_ALIGN_CEIL(tmpSize, 64);

        // insert a new slot which is free with rest
        tmpSize = MOS_ALIGN_CEIL(tmpSize, 64);  // HW required 64 byte align
        kernelAllocation = &stateHeap->pKernelAllocation[slot+1];
        CmLoadKernel(state, stateHeap, kernelAllocation, 0, 0, parameters, kernelParam, nullptr, isCloneEntry);
        kernelAllocation->dwOffset = dwOffset+tmpSize;
        kernelAllocation->iSize = 0;
        state->totalKernelSize[slot+1] = totalSize - tmpSize;

        // added one more entry
        state->kernelNumInGsh++;

        kernelAllocation = &stateHeap->pKernelAllocation[slot];
        if (isCloneEntry)
        {
            if (!stateHeap->pKernelAllocation[headKernelAllocationID].cloneKernelParams.isHeadKernel)
            {
                // ERROR thought kernel with allocation ID, headKernelAllocationID, was a head kernel, but it's not
                hr = CM_FAILURE;
                goto finish;
            }

            kernelAllocation->cloneKernelParams.dwOffsetForAllocID  = dwOffset;
            kernelAllocation->dwOffset                              = stateHeap->pKernelAllocation[headKernelAllocationID].dwOffset;
            kernelAllocation->cloneKernelParams.isClone             = true;
            kernelAllocation->cloneKernelParams.kernelBinaryAllocID = headKernelAllocationID;
            kernelAllocation->cloneKernelParams.cloneKernelID       = stateHeap->pKernelAllocation[headKernelAllocationID].iKUID;

            stateHeap->pKernelAllocation[headKernelAllocationID].cloneKernelParams.referenceCount = stateHeap->pKernelAllocation[headKernelAllocationID].cloneKernelParams.referenceCount + 1;

            // update head kernel's count after updating the clone entry's count so that clone will be selected for deletion first
            stateHeap->pKernelAllocation[headKernelAllocationID].dwCount = stateHeap->dwAccessCounter++;

        }
        else
        {
            kernelAllocation->dwOffset = dwOffset;

            if (isHeadKernel)
            {
                kernelAllocation->cloneKernelParams.isHeadKernel = true;
                if (isCloneAsHead)
                {
                    kernelAllocation->cloneKernelParams.cloneKernelID = kernelParam->clonedKernelParam.kernelID;
                }
            }
        }

        if (lastKernel > slot)
        {
            HalCm_UpdateCloneKernel(state, slot, CM_SHIFT_LEFT, 1);
            if (isCloneEntry && adjustHeadKernelID)
            {
                // if clone entry and already adjusted head kernel ID, then adjusted again in HalCm_UpdateCloneKernel, need to do only once
                kernelAllocation->cloneKernelParams.kernelBinaryAllocID = kernelAllocation->cloneKernelParams.kernelBinaryAllocID - 1;
            }
        }
    }
    else if (state->kernelNumInGsh < state->cmDeviceParam.maxGshKernelEntries)
    {
        // no need to create a new entry since we have the same size
        kernelAllocation = &stateHeap->pKernelAllocation[slot];

        if(state->cbbEnabled)
        {
            tag = state->osInterface->pfnGetGpuStatusTag(state->osInterface,
                state->osInterface->CurrentGpuContextOrdinal);
        }
        else
        {
            tag = stateHeap->dwNextTag;
        }

        CmLoadKernel(state, stateHeap, kernelAllocation, tag, stateHeap->dwAccessCounter, parameters, kernelParam, mhwKernelParam, isCloneEntry);
        stateHeap->dwAccessCounter++;
        // no change for kernelAllocation->dwOffset
        kernelAllocation->iSize = neededSize;
        state->totalKernelSize[slot] = MOS_ALIGN_CEIL(mhwKernelParam->iSize, 64);
        if (isCloneEntry)
        {
            if (!stateHeap->pKernelAllocation[headKernelAllocationID].cloneKernelParams.isHeadKernel)
            {
                // ERROR thought kernel with allocation ID, headKernelAllocationID, was a head kernel, but it's not
                hr = CM_FAILURE;
                goto finish;
            }

            kernelAllocation->cloneKernelParams.dwOffsetForAllocID  = kernelAllocation->dwOffset;
            kernelAllocation->dwOffset                              = stateHeap->pKernelAllocation[headKernelAllocationID].dwOffset;
            kernelAllocation->cloneKernelParams.isClone             = true;
            kernelAllocation->cloneKernelParams.kernelBinaryAllocID = headKernelAllocationID;
            kernelAllocation->cloneKernelParams.cloneKernelID       = stateHeap->pKernelAllocation[headKernelAllocationID].iKUID;

            stateHeap->pKernelAllocation[headKernelAllocationID].cloneKernelParams.referenceCount = stateHeap->pKernelAllocation[headKernelAllocationID].cloneKernelParams.referenceCount + 1;

            // update head kernel's count after updating the clone entry's count so that clone will be selected for deletion first
            stateHeap->pKernelAllocation[headKernelAllocationID].dwCount = stateHeap->dwAccessCounter++;
        }
        else if (isHeadKernel)
        {
            kernelAllocation->cloneKernelParams.isHeadKernel = true;
            if (isCloneAsHead)
            {
                kernelAllocation->cloneKernelParams.cloneKernelID = kernelParam->clonedKernelParam.kernelID;
            }
        }
    }
    else
    {
        // all slots are used, but we have one free which is big enough
        // we may have fragmentation, but code is the same as above case
        kernelAllocation = &stateHeap->pKernelAllocation[slot];

        if(state->cbbEnabled)
        {
            tag = state->osInterface->pfnGetGpuStatusTag(state->osInterface, state->osInterface->CurrentGpuContextOrdinal);
        }
        else
        {
            tag = stateHeap->dwNextTag;
        }

        CmLoadKernel(state, stateHeap, kernelAllocation, tag, stateHeap->dwAccessCounter, parameters, kernelParam, mhwKernelParam, isCloneEntry);
        stateHeap->dwAccessCounter++;
        // kernelAllocation->iTotalSize is not changed, but we have smaller actual size
        // no change for kernelAllocation->dwOffset
        kernelAllocation->iSize = neededSize;

        if (isCloneEntry)
        {
            if (!stateHeap->pKernelAllocation[headKernelAllocationID].cloneKernelParams.isHeadKernel)
            {
                // ERROR thought kernel with allocation ID, headKernelAllocationID, was a head kernel, but it's not
                hr = CM_FAILURE;
                goto finish;
            }

            kernelAllocation->cloneKernelParams.dwOffsetForAllocID  = kernelAllocation->dwOffset;
            kernelAllocation->dwOffset                              = stateHeap->pKernelAllocation[headKernelAllocationID].dwOffset;
            kernelAllocation->cloneKernelParams.isClone             = true;
            kernelAllocation->cloneKernelParams.kernelBinaryAllocID = headKernelAllocationID;
            kernelAllocation->cloneKernelParams.cloneKernelID       = stateHeap->pKernelAllocation[headKernelAllocationID].iKUID;

            stateHeap->pKernelAllocation[headKernelAllocationID].cloneKernelParams.referenceCount = stateHeap->pKernelAllocation[headKernelAllocationID].cloneKernelParams.referenceCount + 1;

            // update head kernel's count after updating the clone entry's count so that clone will be selected for deletion first
            stateHeap->pKernelAllocation[headKernelAllocationID].dwCount = stateHeap->dwAccessCounter++;
        }
        else if (isHeadKernel)
        {
            kernelAllocation->cloneKernelParams.isHeadKernel = true;
            if (isCloneAsHead)
            {
                kernelAllocation->cloneKernelParams.cloneKernelID = kernelParam->clonedKernelParam.kernelID;
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
    PCM_HAL_STATE              state,
    PRENDERHAL_KRN_ALLOCATION  kernelAllocation)
{
    PRENDERHAL_INTERFACE       renderHal = state->renderHal;
    PRENDERHAL_STATE_HEAP      stateHeap;
    int32_t                    hr;

    //---------------------------------------
    CM_CHK_NULL_GOTOFINISH_CMERROR(renderHal);
    CM_CHK_NULL_GOTOFINISH_CMERROR(renderHal->pStateHeap);
    CM_CHK_NULL_GOTOFINISH_CMERROR(kernelAllocation);
    //---------------------------------------

    hr      = CM_FAILURE;
    stateHeap = renderHal->pStateHeap;

    if (kernelAllocation->dwFlags == RENDERHAL_KERNEL_ALLOCATION_FREE)
    {
        goto finish;
    }

    CM_CHK_CMSTATUS_GOTOFINISH(HalCm_SyncKernel(state, kernelAllocation->dwSync));

    // Unload kernel
    if (kernelAllocation->pMhwKernelParam)
    {
        kernelAllocation->pMhwKernelParam->bLoaded = 0;
    }

    if (kernelAllocation->cloneKernelParams.isClone)
    {
        if (stateHeap->pKernelAllocation[kernelAllocation->cloneKernelParams.kernelBinaryAllocID].cloneKernelParams.isHeadKernel)
        {
            if ((stateHeap->pKernelAllocation[kernelAllocation->cloneKernelParams.kernelBinaryAllocID].cloneKernelParams.referenceCount) <= 0)
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

        stateHeap->pKernelAllocation[kernelAllocation->cloneKernelParams.kernelBinaryAllocID].cloneKernelParams.referenceCount =
            stateHeap->pKernelAllocation[kernelAllocation->cloneKernelParams.kernelBinaryAllocID].cloneKernelParams.referenceCount - 1;

        // restore the dwOffset for this allocationID
        kernelAllocation->dwOffset = kernelAllocation->cloneKernelParams.dwOffsetForAllocID;
    }
    else if (kernelAllocation->cloneKernelParams.isHeadKernel && kernelAllocation->cloneKernelParams.referenceCount != 0)
    {
        // ERROR, cloned kernel entries should have been selected for deletion before head kernel entry
        hr = CM_FAILURE;
        goto finish;
    }

    // Release kernel entry (Offset/size may be used for reallocation)
    kernelAllocation->iKID     = -1;
    kernelAllocation->iKUID    = -1;
    kernelAllocation->iKCID    = -1;
    kernelAllocation->dwSync   = 0;
    FrameTrackerTokenFlat_Clear(&kernelAllocation->trackerToken);
    kernelAllocation->dwFlags          = RENDERHAL_KERNEL_ALLOCATION_FREE;
    kernelAllocation->dwCount  = 0;
    kernelAllocation->pMhwKernelParam  = nullptr;
    kernelAllocation->cloneKernelParams.cloneKernelID       = -1;
    kernelAllocation->cloneKernelParams.isClone             = false;
    kernelAllocation->cloneKernelParams.isHeadKernel        = false;
    kernelAllocation->cloneKernelParams.kernelBinaryAllocID = -1;
    kernelAllocation->cloneKernelParams.referenceCount      = 0;

    hr = CM_SUCCESS;

finish:
    return hr;
}

/*----------------------------------------------------------------------------
| Name      : HalCmw_TouchKernel ( Replace RenderHal_TouchKernel)
\---------------------------------------------------------------------------*/
int32_t HalCm_TouchKernel(
    PCM_HAL_STATE       state,
    int32_t             kernelAllocationID)
{
    int32_t                     hr = CM_SUCCESS;
    PRENDERHAL_STATE_HEAP       stateHeap;
    PRENDERHAL_KRN_ALLOCATION   kernelAllocation;
    PRENDERHAL_KRN_ALLOCATION   headKernelAllocation;
    uint32_t                    tag;

    PRENDERHAL_INTERFACE renderHal = state->renderHal;
    PMOS_INTERFACE osInterface     = state->osInterface;

    stateHeap = (renderHal) ? renderHal->pStateHeap : nullptr;
    if (stateHeap == nullptr ||
        stateHeap->pKernelAllocation == nullptr ||
        kernelAllocationID < 0 ||
        kernelAllocationID >= renderHal->StateHeapSettings.iKernelCount)
    {
        hr = CM_FAILURE;
        goto finish;
    }

    // Update usage
    kernelAllocation = &(stateHeap->pKernelAllocation[kernelAllocationID]);
    if (kernelAllocation->dwFlags != RENDERHAL_KERNEL_ALLOCATION_FREE &&
        kernelAllocation->dwFlags != RENDERHAL_KERNEL_ALLOCATION_LOCKED)
    {
        kernelAllocation->dwCount = stateHeap->dwAccessCounter++;
    }

    // Set sync tag, for deallocation control
    if(state->cbbEnabled)
    {
        tag = osInterface->pfnGetGpuStatusTag(osInterface, osInterface->CurrentGpuContextOrdinal);
    }
    else
    {
        tag = stateHeap->dwNextTag;
    }

    kernelAllocation->dwSync = tag;

    // if this kernel allocation is a cloned kernel, update the orig kernel sync tag and access counter
    if (kernelAllocation->cloneKernelParams.isClone)
    {
        headKernelAllocation = &(stateHeap->pKernelAllocation[kernelAllocation->cloneKernelParams.kernelBinaryAllocID]);

        if (headKernelAllocation->cloneKernelParams.referenceCount <= 0)
        {
            // ERROR
            hr = CM_FAILURE;
            goto finish;
        }

        headKernelAllocation->dwSync = tag;
        headKernelAllocation->dwCount = stateHeap->dwAccessCounter++;

    }

finish:
    return hr;
}

/*
**  Supporting function
**  Delete oldest entry from table to free more space
**  According to different cases, we will combine space with previous or next slot to get max space
*/
int32_t CmDeleteOldestKernel(PCM_HAL_STATE state, MHW_KERNEL_PARAM *mhwKernelParam)
{
    PRENDERHAL_KRN_ALLOCATION  kernelAllocation;
    PRENDERHAL_INTERFACE       renderHal = state->renderHal;;
    PRENDERHAL_STATE_HEAP      stateHeap = renderHal->pStateHeap;
    UNUSED(state);
    UNUSED(mhwKernelParam);

    uint32_t oldest = 0;
    uint32_t lastUsed;
    int32_t kernelAllocationID, searchIndex = -1, index = -1;
    int32_t alignedSize, shiftOffset;
    int32_t hr = CM_SUCCESS;

    kernelAllocation   = stateHeap->pKernelAllocation;

    // Search and deallocate oldest kernel (most likely this is optimal scheduling algorithm)
    kernelAllocation = stateHeap->pKernelAllocation;
    for (kernelAllocationID = 0;
        kernelAllocationID < state->kernelNumInGsh;
        kernelAllocationID++, kernelAllocation++)
    {
        // Skip unused entries
        // Skip kernels flagged as locked (cannot be automatically deallocated)
        if (kernelAllocation->dwFlags == RENDERHAL_KERNEL_ALLOCATION_FREE ||
            kernelAllocation->dwFlags == RENDERHAL_KERNEL_ALLOCATION_LOCKED)
        {
            continue;
        }

        // Find kernel not used for the greater amount of time (measured in number of operations)
        // Must not unload recently allocated kernels
        lastUsed = (uint32_t)(stateHeap->dwAccessCounter - kernelAllocation->dwCount);
        if (lastUsed > oldest)
        {
            searchIndex = kernelAllocationID;
            oldest     = lastUsed;
        }
    }

    // Did not found any entry for deallocation, we get into a strange case!
    if (searchIndex < 0)
    {
        CM_ASSERTMESSAGE("Failed to delete any slot from GSH. It is impossible.");
        return CM_FAILURE;
    }

    if (stateHeap->pKernelAllocation[searchIndex].cloneKernelParams.isHeadKernel &&
        (stateHeap->pKernelAllocation[searchIndex].cloneKernelParams.referenceCount != 0))
    {
        // ERROR, chose a head kernel for deletion but it still has clones pointing to it
        return CM_FAILURE;
    }

    // Free kernel entry and states associated with the kernel (if any)
    kernelAllocation = &stateHeap->pKernelAllocation[searchIndex];
    if (HalCm_UnloadKernel(state, kernelAllocation) != CM_SUCCESS)
    {
        CM_ASSERTMESSAGE("Failed to load kernel - no space available in GSH.");
        return CM_FAILURE;
    }

    // Let's check if we can merge searchIndex-1, searchIndex, searchIndex+1
    index = searchIndex;
    PRENDERHAL_KRN_ALLOCATION kAlloc0, kAlloc1, kAlloc2;
    kAlloc0 = (index == 0)? nullptr : &stateHeap->pKernelAllocation[index-1];
    kAlloc1 = &stateHeap->pKernelAllocation[index];  // free one
    kAlloc2 = (index == state->cmDeviceParam.maxGshKernelEntries - 1) ? nullptr : &stateHeap->pKernelAllocation[index + 1];

    if (bIsFree(kAlloc0) && bIsFree(kAlloc2))
    {
        // merge 3 into 1 slot and bump index after
        stateHeap->pKernelAllocation[index-1].dwFlags = RENDERHAL_KERNEL_ALLOCATION_FREE;
        state->totalKernelSize[index-1] += state->totalKernelSize[index] + state->totalKernelSize[index+1];
        stateHeap->pKernelAllocation[index-1].iSize = 0;
        // no change for stateHeap->pKernelAllocation[index-1].dwOffset

        // copy the rest
        for (int32_t i = index + 2; i<state->kernelNumInGsh; i++)
        {
            stateHeap->pKernelAllocation[i-2] = stateHeap->pKernelAllocation[i];
            state->totalKernelSize[i-2] = state->totalKernelSize[i];
        }

        state->kernelNumInGsh -= 2;

        if ( index == 0 )
            HalCm_UpdateCloneKernel(state, 0, CM_SHIFT_RIGHT, 2);
        else
            HalCm_UpdateCloneKernel(state, index - 1, CM_SHIFT_RIGHT, 2);
    }
    else if (bIsFree(kAlloc0))
    {
        // merge before and current into 1 slot
        stateHeap->pKernelAllocation[index-1].dwFlags = RENDERHAL_KERNEL_ALLOCATION_FREE;
        state->totalKernelSize[index-1] += state->totalKernelSize[index];
        stateHeap->pKernelAllocation[index-1].iSize = 0;
        // no change for stateHeap->pKernelAllocation[index-1].dwOffset

        for (int32_t i = index + 1; i<state->kernelNumInGsh; i++)
        {
            stateHeap->pKernelAllocation[i-1] = stateHeap->pKernelAllocation[i];
            state->totalKernelSize[i-1] = state->totalKernelSize[i];
        }

        state->kernelNumInGsh -= 1;

        if ( index == 0 )
            HalCm_UpdateCloneKernel(state, 0, CM_SHIFT_RIGHT, 1);
        else
            HalCm_UpdateCloneKernel(state, index - 1, CM_SHIFT_RIGHT, 1);

    }
    else if (bIsFree(kAlloc2))
    {
        // kAlloc0 is not free, but it can be nullptr
        // merge after and current into 1 slot
        stateHeap->pKernelAllocation[index].dwFlags = RENDERHAL_KERNEL_ALLOCATION_FREE;
        state->totalKernelSize[index] += state->totalKernelSize[index+1];
        stateHeap->pKernelAllocation[index].iSize = 0;
        if (kAlloc0)
        {
            // get free space starting point
            alignedSize = MOS_ALIGN_CEIL(kAlloc0->iSize, 64);
            shiftOffset = state->totalKernelSize[index-1] - alignedSize;

            state->totalKernelSize[index-1] -= shiftOffset;
            // no change for stateHeap->pKernelAllocation[index-1].iSize -= 0;
            state->totalKernelSize[index] += shiftOffset;
            stateHeap->pKernelAllocation[index].dwOffset -= shiftOffset;
        }

        for (int32_t i = index + 1; i<state->kernelNumInGsh; i++)
        {
            stateHeap->pKernelAllocation[i] = stateHeap->pKernelAllocation[i+1];
            state->totalKernelSize[i] = state->totalKernelSize[i+1];
        }

        state->kernelNumInGsh -= 1;

        if ( index == 0 )
            HalCm_UpdateCloneKernel(state, 0, CM_SHIFT_RIGHT, 1);
        else
            HalCm_UpdateCloneKernel(state, index - 1, CM_SHIFT_RIGHT, 1);
    }
    else
    {
        // no merge
        stateHeap->pKernelAllocation[index].dwFlags = RENDERHAL_KERNEL_ALLOCATION_FREE;
        // no change for stateHeap->pKernelAllocation[index].iTotalSize;
        stateHeap->pKernelAllocation[index].iSize = 0;
        if(kAlloc0)
        {
            // get free space starting point
            alignedSize = MOS_ALIGN_CEIL(kAlloc0->iSize, 64);
            shiftOffset = state->totalKernelSize[index-1] - alignedSize;
            state->totalKernelSize[index-1] -= shiftOffset;
            // no change for stateHeap->pKernelAllocation[index-1].iSize -= 0;
            state->totalKernelSize[index] += shiftOffset;
            stateHeap->pKernelAllocation[index].dwOffset -= shiftOffset;
        }
        // no change for stateHeap->iNumKernels;
    }

    return hr;
}

/*----------------------------------------------------------------------------
| Name      : HalCm_LoadKernel ( Replace RenderHal_LoadKernel)
\---------------------------------------------------------------------------*/
int32_t HalCm_LoadKernel(
    PCM_HAL_STATE             state,
    PCM_HAL_KERNEL_PARAM      kernelParam,
    int32_t                   samplerCount,
    PRENDERHAL_KRN_ALLOCATION &kernelAllocation)
{
    PRENDERHAL_STATE_HEAP     stateHeap;
    PRENDERHAL_INTERFACE      renderHal;
    int32_t                 hr;
    PRENDERHAL_KERNEL_PARAM   parameters;
    PMHW_KERNEL_PARAM         mhwKernelParam;

    int32_t kernelAllocationID;    // Kernel allocation ID in GSH
    int32_t kernelCacheID;         // Kernel cache ID
    int32_t kernelUniqueID;        // Kernel unique ID
    void    *kernelPtr;
    int32_t kernelSize;
    int32_t searchIndex;
    int32_t freeSlot;
    bool    isClonedKernel;
    bool    hasClones;

    hr                  = CM_SUCCESS;
    renderHal          = state->renderHal;
    stateHeap          = (renderHal) ? renderHal->pStateHeap : nullptr;
    kernelAllocationID = RENDERHAL_KERNEL_LOAD_FAIL;
    mhwKernelParam     = &(state->kernelParamsMhw);
    parameters         = &(state->kernelParamsRenderHal.Params);

    // Validate parameters
    if (stateHeap == nullptr ||
        stateHeap->bIshLocked == false ||
        stateHeap->pKernelAllocation == nullptr ||
        kernelParam->kernelBinarySize == 0 ||
        state->kernelNumInGsh > state->cmDeviceParam.maxGshKernelEntries)
    {
        CM_ASSERTMESSAGE("Failed to load kernel - invalid parameters.");
        return CM_FAILURE;
    }

    isClonedKernel = kernelParam->clonedKernelParam.isClonedKernel;
    hasClones      = kernelParam->clonedKernelParam.hasClones;

    parameters->Sampler_Count = samplerCount;
    mhwKernelParam->iKUID     = static_cast<int>( (kernelParam->kernelId >> 32) );
    mhwKernelParam->iKCID     = -1;
    mhwKernelParam->pBinary   = kernelParam->kernelBinary;
    mhwKernelParam->iSize     = kernelParam->kernelBinarySize + CM_KERNEL_BINARY_PADDING_SIZE;

    // Kernel parameters
    kernelPtr      = mhwKernelParam->pBinary;
    kernelSize     = mhwKernelParam->iSize;
    kernelUniqueID = mhwKernelParam->iKUID;
    kernelCacheID  = mhwKernelParam->iKCID;

    // Check if kernel is already loaded; Search free allocation index
    searchIndex = -1;
    kernelAllocation = stateHeap->pKernelAllocation;
    for (kernelAllocationID = 0;
         kernelAllocationID <  state->kernelNumInGsh;
         kernelAllocationID++, kernelAllocation++)
    {
        if (kernelAllocation->iKUID == kernelUniqueID &&
            kernelAllocation->iKCID == kernelCacheID)
        {
            // found match and Update kernel usage
            hr = HalCm_TouchKernel(state, kernelAllocationID);
            if (hr == CM_FAILURE)
            {
                goto finish;
            }
            // Increment reference counter
            mhwKernelParam->bLoaded = 1;
            // Record kernel allocation
            kernelAllocation = &stateHeap->pKernelAllocation[kernelAllocationID];

            goto finish;
        }
    }

    if (isClonedKernel || hasClones)
    {
        hr = HalCm_InsertCloneKernel(state, kernelParam, kernelAllocation);
        goto finish;
    }

    // here is the algorithm
    // 1) search for free slot which is big enough to load current kerenel
    // 2) if found slot, then add current kerenel
    // 3) if we cannot find slot, we need to delete some entry (delete oldest first), after delete oldest entry
    //    we will loop over to step 1 until we get enough space.
    // The algorithm won't fail except we load 1 kernel which is larger than 2MB
    do
    {
        freeSlot = CmSearchFreeSlotSize(state, mhwKernelParam, false);
        if (freeSlot >= 0)
        {
            // found free slot which is big enough to hold kernel
            hr = CmAddCurrentKernelToFreeSlot(state, freeSlot, parameters, kernelParam, mhwKernelParam, CM_NO_CLONE, -1);
            // update GSH states stateHeap->numKernels inside add function
            break;
        }
        else
        {
            if (CmDeleteOldestKernel(state, mhwKernelParam) != CM_SUCCESS)
            {
                return CM_FAILURE;
            }
        }
    } while(1);

    mhwKernelParam->bLoaded = 1;  // Increment reference counter
    kernelAllocation = &stateHeap->pKernelAllocation[freeSlot];  // Record kernel allocation

finish:

    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Loads cloned kernel entries and kernels with clones into free slot
//| Return:     Result of the operation
//*-----------------------------------------------------------------------------
int32_t HalCm_InsertCloneKernel(
    PCM_HAL_STATE              state,
    PCM_HAL_KERNEL_PARAM       kernelParam,
    PRENDERHAL_KRN_ALLOCATION  &kernelAllocation)
{
    int32_t                   hr              = CM_SUCCESS;
    int32_t                   kernelAllocationID;    // Kernel allocation ID in GSH
    uint32_t                  tag;
    PMOS_INTERFACE            osInterface    = state->osInterface;
    PMHW_KERNEL_PARAM         mhwKernelParam = &(state->kernelParamsMhw);
    int32_t                   freeSlot       = -1;
    PRENDERHAL_STATE_HEAP     stateHeap = state->renderHal->pStateHeap;

    kernelAllocation = state->renderHal->pStateHeap->pKernelAllocation;

    for (kernelAllocationID = 0; kernelAllocationID < state->kernelNumInGsh;
        kernelAllocationID++, kernelAllocation++)
    {
        if (kernelAllocation->cloneKernelParams.isHeadKernel)
        {
            if ((kernelAllocation->iKUID                           == kernelParam->clonedKernelParam.kernelID) ||       // original kernel that cloned from is already loaded as head
                (kernelAllocation->cloneKernelParams.cloneKernelID == kernelParam->clonedKernelParam.kernelID) ||       // another clone from same original kernel is serving as the head
                (kernelAllocation->cloneKernelParams.cloneKernelID == static_cast<int>(kernelParam->kernelId >> 32))) // clone is serving as the head and this is the original kernel
            {
                // found match, insert 64B dummy entry and set piKAID
                do
                {
                    // Before getting a free slot, update head kernel sync tag and count so head will not be selected for deletion
                    // then update head kernel count after inserting clone
                    // so that clone will be selected first for deletion (this is done in CmAddCurrentKernelToFreeSlot)

                    // update head kernel sync tag
                    if(state->cbbEnabled)
                    {
                        tag = osInterface->pfnGetGpuStatusTag(osInterface, osInterface->CurrentGpuContextOrdinal);
                    }
                    else
                    {
                        tag = state->renderHal->pStateHeap->dwNextTag;
                    }
                    kernelAllocation->dwSync = tag;

                    // update the head kernel count so it will not be selected for deletion
                    kernelAllocation->dwCount = state->renderHal->pStateHeap->dwAccessCounter++;

                    freeSlot = CmSearchFreeSlotSize(state, mhwKernelParam, true);
                    if (freeSlot >= 0)
                    {
                        // found free slot
                        hr = CmAddCurrentKernelToFreeSlot(state, freeSlot, &(state->kernelParamsRenderHal.Params),
                            kernelParam, &(state->kernelParamsMhw), CM_CLONE_ENTRY, kernelAllocationID);

                        goto finish;

                    }
                    else
                    {
                        if (CmDeleteOldestKernel(state, mhwKernelParam) != CM_SUCCESS)
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
        freeSlot = CmSearchFreeSlotSize(state, mhwKernelParam, false);
        if (freeSlot >= 0)
        {
            if (kernelParam->clonedKernelParam.isClonedKernel)
            {
                hr = CmAddCurrentKernelToFreeSlot(state, freeSlot, &(state->kernelParamsRenderHal.Params),
                    kernelParam, &(state->kernelParamsMhw), CM_CLONE_AS_HEAD_KERNEL, -1);
            }
            else
            {
                hr = CmAddCurrentKernelToFreeSlot(state, freeSlot, &(state->kernelParamsRenderHal.Params),
                    kernelParam, &(state->kernelParamsMhw), CM_HEAD_KERNEL, -1);
            }
            break;
        }
        else
        {
            if (CmDeleteOldestKernel(state, mhwKernelParam) != CM_SUCCESS)
            {
                hr = CM_FAILURE;
                goto finish;
            }
        }
    } while (1);

finish:

    if (hr == CM_SUCCESS)
    {
        mhwKernelParam->bLoaded = 1;
        kernelAllocation = &stateHeap->pKernelAllocation[freeSlot];
    }

    return hr;
}

//!
//! \brief    Get offset to sampler state
//! \details  Get offset to sampler state in General State Heap,
//!           (Cm customized version of the RenderHal function which calculates 
//!           the sampler offset by MDF owned parameters).
//! \param    PCM_HAL_STATE state
//!           [in] Pointer to CM_HAL_STATE structure
//! \param    PRENDERHAL_INTERFACE renderHal
//!           [in] Pointer to RenderHal Interface
//! \param    int mediaID
//!           [in] Media ID associated with sampler
//! \param    int samplerOffset
//!           [in] sampler offset from the base of current kernel's sampler heap
//! \param    int samplerBTI
//!           [in] sampler BTI
//! \param    unsigned long *pdwSamplerOffset
//!           [out] optional; offset of sampler state from GSH base
//! \return   MOS_STATUS
//!
MOS_STATUS HalCm_GetSamplerOffset(
    PCM_HAL_STATE            state,
    PRENDERHAL_INTERFACE     renderHal,
    int                      mediaID,
    unsigned int             samplerOffset,
    unsigned int             samplerBTI,
    PMHW_SAMPLER_STATE_PARAM samplerParam,
    uint32_t                *pdwSamplerOffset)
{
    unsigned int tmpSamplerOffset = renderHal->pStateHeap->pCurMediaState->pDynamicState->Sampler3D.dwOffset +
                                  state->taskParam->samplerOffsetsByKernel[mediaID] +
                                  samplerOffset;

    if (pdwSamplerOffset != nullptr)
    {
        *pdwSamplerOffset = tmpSamplerOffset;
    }

    if (samplerParam->SamplerType == MHW_SAMPLER_TYPE_3D)
    {
        samplerParam->Unorm.IndirectStateOffset = MOS_ALIGN_CEIL( renderHal->pStateHeap->pCurMediaState->pDynamicState->Sampler3D.dwOffset +
                                                                  state->taskParam->samplerIndirectOffsetsByKernel[mediaID] +
                                                                  samplerBTI * renderHal->pHwSizes->dwSizeSamplerIndirectState,
                                                                  1 << MHW_SAMPLER_INDIRECT_SHIFT);
    }
    return MOS_STATUS_SUCCESS;
}

//!
//! \brief      Setup Interface Descriptor
//! \details    Set interface descriptor, (overriding RenderHal function),
//!             (Cm customized version of the RenderHal function which set
//!             dwSamplerOffset and dwSamplerCount by MDF owned parameters).
//! \param      PCM_HAL_STATE                           state
//!             [in]    Pointer to CM_HAL_STATE structure
//! \param      PRENDERHAL_INTERFACE                    renderHal
//!             [in]    Pointer to HW interface
//! \param      PRENDERHAL_MEDIA_STATE                  mediaState
//!             [in]    Pointer to media state
//! \param      PRENDERHAL_KRN_ALLOCATION               kernelAllocation
//!             [in]    Pointer to kernel allocation
//! \param      PRENDERHAL_INTERFACE_DESCRIPTOR_PARAMS  interfaceDescriptorParams
//!             [in]    Pointer to interface descriptor parameters
//! \param      PMHW_GPGPU_WALKER_PARAMS          pGpGpuWalkerParams
//!             [in]    Pointer to gpgpu walker parameters
//! \return     MOS_STATUS
//!
MOS_STATUS HalCm_SetupInterfaceDescriptor(
    PCM_HAL_STATE                          state,
    PRENDERHAL_INTERFACE                   renderHal,
    PRENDERHAL_MEDIA_STATE                 mediaState,
    PRENDERHAL_KRN_ALLOCATION              kernelAllocation,
    PRENDERHAL_INTERFACE_DESCRIPTOR_PARAMS interfaceDescriptorParams)
{
    MOS_STATUS               eStatus = MOS_STATUS_SUCCESS;
    MHW_ID_ENTRY_PARAMS      params;
    PRENDERHAL_STATE_HEAP    stateHeap;
    PRENDERHAL_DYNAMIC_STATE dynamicState;
    unsigned long            mediaStateOffset;

    //-----------------------------------------
    MHW_RENDERHAL_CHK_NULL(renderHal);
    MHW_RENDERHAL_CHK_NULL(renderHal->pMhwStateHeap);
    MHW_RENDERHAL_CHK_NULL(mediaState);
    MHW_RENDERHAL_CHK_NULL(mediaState->pDynamicState);
    MHW_RENDERHAL_CHK_NULL(interfaceDescriptorParams);
    //-----------------------------------------

    // Get states, params
    stateHeap = renderHal->pStateHeap;
    dynamicState = mediaState->pDynamicState;
    mediaStateOffset = dynamicState->memoryBlock.GetOffset();

    params.dwMediaIdOffset = mediaStateOffset + dynamicState->MediaID.dwOffset;
    params.iMediaId = interfaceDescriptorParams->iMediaID;
    params.dwKernelOffset = kernelAllocation->dwOffset;
    params.dwSamplerOffset = mediaStateOffset + dynamicState->Sampler3D.dwOffset + state->taskParam->samplerOffsetsByKernel[params.iMediaId];
    params.dwSamplerCount = ( state->taskParam->samplerCountsByKernel[params.iMediaId] + 3 ) / 4;
    params.dwSamplerCount = (params.dwSamplerCount > 4) ? 4 : params.dwSamplerCount;
    params.dwBindingTableOffset = interfaceDescriptorParams->iBindingTableID * stateHeap->iBindingTableSize;
    params.iCurbeOffset = interfaceDescriptorParams->iCurbeOffset;
    params.iCurbeLength = interfaceDescriptorParams->iCurbeLength;

    params.bBarrierEnable = interfaceDescriptorParams->blBarrierEnable;
    params.bGlobalBarrierEnable = interfaceDescriptorParams->blGlobalBarrierEnable;    //It's only applied for BDW+
    params.dwNumberofThreadsInGPGPUGroup = interfaceDescriptorParams->iNumberThreadsInGroup;
    params.dwSharedLocalMemorySize = renderHal->pfnEncodeSLMSize(renderHal, interfaceDescriptorParams->iSLMSize);
    params.iCrsThdConDataRdLn = interfaceDescriptorParams->iCrsThrdConstDataLn;
    params.memoryBlock = &dynamicState->memoryBlock;

    MHW_RENDERHAL_CHK_STATUS(renderHal->pMhwStateHeap->AddInterfaceDescriptorData(&params));
    dynamicState->MediaID.iCurrent++;

finish:
    return eStatus;
}

/*----------------------------------------------------------------------------
| Name      : HalCm_AllocateMediaID  replace old RenderHal_AllocateMediaID
|             Don't need touch kernel since we handle this a loadKernel time
|
| Purpose   : Allocates an setup Interface Descriptor for Media Pipeline
|
| Arguments : [in] renderHal          - Pointer to RenderHal interface structure
|             [in] kernelParam        - Pointer to Kernel parameters
|             [in] pKernelAllocationID - Pointer to Kernel allocation
|             [in] bindingTableID     - Binding table ID
|             [in] curbeOffset        - Curbe offset (from CURBE base)
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
//! \param    PRENDERHAL_INTERFACE       renderHal
//| \param    PCM_HAL_KERNEL_PARAM       kernelParam
//| \param    PRENDERHAL_KRN_ALLOCATION  kernelAllocation
//| \param    int32_t                    bindingTableID
//| \param    int32_t                    curbeOffset
//! \return   int32_t
//!
int32_t HalCm_AllocateMediaID(
    PCM_HAL_STATE               state,
    PCM_HAL_KERNEL_PARAM        kernelParam,
    PRENDERHAL_KRN_ALLOCATION   kernelAllocation,
    int32_t                    bindingTableID,
    int32_t                    curbeOffset)
{
    PRENDERHAL_INTERFACE    renderHal = state->renderHal;
    PRENDERHAL_MEDIA_STATE  curMediaState;
    int32_t                 curbeSize, iCurbeCurrent;
    int32_t                 interfaceDescriptor;
    RENDERHAL_INTERFACE_DESCRIPTOR_PARAMS interfaceDescriptorParams;

    interfaceDescriptor = -1;

    // Obtain pointer and validate current media state
    curMediaState = renderHal->pStateHeap->pCurMediaState;

    if (state->dshEnabled)
    {
        if (curMediaState == nullptr || (state->dshEnabled && (curMediaState->pDynamicState == nullptr)))
        {
            CM_ASSERTMESSAGE("Invalid Media State.");
            goto finish;
        }
    }
    else
    {
        if (curMediaState == nullptr)
        {
            CM_ASSERTMESSAGE("Invalid Media State.");
            goto finish;
        }
    }

    // Validate kernel allocation (kernel must be pre-loaded into GSH)
    if (!kernelAllocation ||
        kernelAllocation->dwFlags == RENDERHAL_KERNEL_ALLOCATION_FREE ||
        kernelAllocation->iSize == 0)
    {
        CM_ASSERTMESSAGE("Error: Invalid Kernel Allocation.");
        goto finish;
    }

    // Check Curbe allocation (CURBE_Lenght is in 256-bit count -> convert to bytes)
    curbeSize = kernelParam->curbeSizePerThread;

    if (state->dshEnabled)
    {
        iCurbeCurrent = curMediaState->pDynamicState->Curbe.iCurrent;
    }
    else
    {
        iCurbeCurrent = curMediaState->iCurbeOffset;
    }

    if (curbeSize <= 0)
    {
        // Curbe is not used by the kernel
        curbeSize = curbeOffset = 0;
    }
    // Validate Curbe Offset (curbe must be pre-allocated)
    else if ( curbeOffset < 0 ||                                       // Not allocated
             (curbeOffset & 0x1F) != 0 ||                              // Invalid alignment
             (curbeOffset + curbeSize) > iCurbeCurrent)               // Invalid size
    {
        CM_ASSERTMESSAGE("Error: Invalid Curbe Allocation.");
        goto finish;
    }

    // Try to reuse interface descriptor (for 2nd level buffer optimizations)
    // Check if ID already in use by another kernel - must use a different ID
    interfaceDescriptor = renderHal->pfnGetMediaID(renderHal, curMediaState, kernelAllocation);
    if (interfaceDescriptor < 0)
    {
        CM_ASSERTMESSAGE("Error: No Interface Descriptor available.");
        goto finish;
    }

    interfaceDescriptorParams.iMediaID            = interfaceDescriptor;
    interfaceDescriptorParams.iBindingTableID     = bindingTableID;

    //CURBE size and offset setting
    //Media w/o group: only per-thread CURBE is used, CrossThread CURBE is not used.
    //Media w/ group: should follow GPGPU walker setting, there is per-thread CURBE and cross-thread CURBE. But per-thread CURBE should be ZERO, and all should be cross-thread CURBE
    //GPGPU: both per-thread CURBE and cross-thread CURBE need be set.
    interfaceDescriptorParams.iCurbeOffset = curbeOffset;
    if ((!kernelParam->gpgpuWalkerParams.gpgpuEnabled) && (kernelParam->kernelThreadSpaceParam.groupSelect == CM_MW_GROUP_NONE) && (state->taskParam->mediaWalkerGroupSelect == CM_MW_GROUP_NONE))
    {   //Media pipe without group
        interfaceDescriptorParams.iCurbeLength          = kernelParam->curbeSizePerThread;
        interfaceDescriptorParams.iCrsThrdConstDataLn   = kernelParam->crossThreadConstDataLen;    //should always be 0 in this case
        interfaceDescriptorParams.iNumberThreadsInGroup = (kernelParam->numberThreadsInGroup > 0) ? kernelParam->numberThreadsInGroup : 1;  // This field should not be set to 0 even if the barrier is disabled, since an accurate value is needed for proper pre-emption.
        interfaceDescriptorParams.blGlobalBarrierEnable = false;
        interfaceDescriptorParams.blBarrierEnable       = false;
        interfaceDescriptorParams.iSLMSize              = 0;
    }
    else if ((!kernelParam->gpgpuWalkerParams.gpgpuEnabled) && ((kernelParam->kernelThreadSpaceParam.groupSelect != CM_MW_GROUP_NONE) || (state->taskParam->mediaWalkerGroupSelect != CM_MW_GROUP_NONE)))
    {   //Media w/ group
        interfaceDescriptorParams.iCurbeLength          = 0;                                    //No using per-thread CURBE
        interfaceDescriptorParams.iCrsThrdConstDataLn   = kernelParam->curbeSizePerThread;    //treat all CURBE as cross-thread CURBE
        interfaceDescriptorParams.iNumberThreadsInGroup = (kernelParam->numberThreadsInGroup > 0) ? kernelParam->numberThreadsInGroup : 1;  // This field should not be set to 0 even if the barrier is disabled, since an accurate value is needed for proper pre-emption.
        interfaceDescriptorParams.blBarrierEnable       = (kernelParam->barrierMode != CM_NO_BARRIER) ? true : false;
        interfaceDescriptorParams.blGlobalBarrierEnable = (kernelParam->barrierMode == CM_GLOBAL_BARRIER) ? true : false;
        interfaceDescriptorParams.iSLMSize              = kernelParam->slmSize;
    }
    else
    {   //GPGPU pipe
        interfaceDescriptorParams.iCurbeLength          = kernelParam->curbeSizePerThread;
        interfaceDescriptorParams.iCrsThrdConstDataLn   = kernelParam->crossThreadConstDataLen;
        interfaceDescriptorParams.iNumberThreadsInGroup = (kernelParam->numberThreadsInGroup > 0) ? kernelParam->numberThreadsInGroup : 1;
        interfaceDescriptorParams.blBarrierEnable       = (kernelParam->barrierMode != CM_NO_BARRIER) ? true : false;
        interfaceDescriptorParams.blGlobalBarrierEnable = (kernelParam->barrierMode == CM_GLOBAL_BARRIER) ? true : false;
        interfaceDescriptorParams.iSLMSize              = kernelParam->slmSize;
    }
    if (state->useNewSamplerHeap == true)
    {
        HalCm_SetupInterfaceDescriptor(state, renderHal, curMediaState, kernelAllocation, &interfaceDescriptorParams);
    }
    else
    {
        // Setup Media ID entry - this call could be HW dependent
        renderHal->pfnSetupInterfaceDescriptor(
            renderHal,
            curMediaState,
            kernelAllocation,
            &interfaceDescriptorParams);
    }

finish:
    return interfaceDescriptor;
}

bool isRenderTarget(PCM_HAL_STATE state, uint32_t index)
{
    bool readSync = false;

    readSync = state->umdSurf2DTable[index].readSyncs[state->osInterface->CurrentGpuContextOrdinal];

    if (readSync)
        return false;
    else
        return true;
}

int32_t HalCm_DSH_LoadKernelArray(
    PCM_HAL_STATE               state,
    PCM_HAL_KERNEL_PARAM       *kernelArray,
    int32_t                     kernelCount,
    PRENDERHAL_KRN_ALLOCATION  *krnAllocation)
{
    PRENDERHAL_INTERFACE         renderHal;
    PCM_HAL_KERNEL_PARAM         kernel;
    PMHW_STATE_HEAP_MEMORY_BLOCK memoryBlock;                             // Kernel memory block
    int32_t                      totalSize;                               // Total size
    uint32_t                     blockSize[CM_MAX_KERNELS_PER_TASK];      // Size of kernels to load
    int32_t                      blockCount;                              // Number of kernels to load
    MOS_STATUS                   eStatus = MOS_STATUS_SUCCESS;
    int32_t                      hr = CM_FAILURE;

    renderHal = state->renderHal;
    state->criticalSectionDSH->Acquire();
    do
    {
        blockCount = 0;
        totalSize = 0;

        // Obtain list of kernels already loaded, discard kernels loaded in older heaps.
        // Calculate total size of kernels to be loaded, and get size of largest kernel.
        for (int i = 0; i < kernelCount; i++)
        {
            // Find out if kernel is already allocated and loaded in ISH
            kernel = kernelArray[i];
            krnAllocation[i] = (PRENDERHAL_KRN_ALLOCATION)renderHal->pfnSearchDynamicKernel(renderHal, static_cast<int>((kernel->kernelId >> 32)), -1);

            // Kernel is allocated - check if kernel is in current ISH
            if (krnAllocation[i])
            {
                // Check if kernel is loaded
                memoryBlock = krnAllocation[i]->pMemoryBlock;

                if (memoryBlock)
                {
                    // Kernel needs to be reloaded in current heap
                    if (memoryBlock->pStateHeap != renderHal->pMhwStateHeap->GetISHPointer() || state->forceKernelReload) //pInstructionStateHeaps
                    {
                        renderHal->pMhwStateHeap->FreeDynamicBlockDyn(MHW_ISH_TYPE, memoryBlock);
                        krnAllocation[i]->pMemoryBlock = nullptr;
                    }
                    else
                    {
                        // Increment kernel usage count, used in kernel caching architecture
                        state->dshKernelCacheHit++;
                        krnAllocation[i]->dwCount++;

                        // Lock kernel to avoid removal while loading other kernels
                        krnAllocation[i]->dwFlags = RENDERHAL_KERNEL_ALLOCATION_LOCKED;
                    }
                }
                else if (krnAllocation[i]->dwFlags == RENDERHAL_KERNEL_ALLOCATION_REMOVED)
                {
                    // This is a kernel that was unloaded and now needs to be reloaded
                    // Track how many times this "cache miss" happens to determine if the
                    // ISH is under pressure and needs to be expanded
                    state->dshKernelCacheMiss++;
                }
            }
            else
            {
                // Assign kernel allocation for this kernel
                krnAllocation[i] = renderHal->pfnAllocateDynamicKernel(renderHal, static_cast<int>((kernel->kernelId >> 32)), -1);
                CM_CHK_NULL_GOTOFINISH_MOSERROR(krnAllocation[i]);
            }

            // Kernel is not loaded -> add to list of kernels to be loaded
            if (krnAllocation[i]->pMemoryBlock == nullptr &&
                krnAllocation[i]->dwFlags != RENDERHAL_KERNEL_ALLOCATION_LOADING)
            {
                // Increment amount of data that needs to be loaded in ISH (kernel already registered but unloaded)
                blockSize[blockCount++] = kernel->kernelBinarySize + CM_KERNEL_BINARY_PADDING_SIZE;
                totalSize += kernel->kernelBinarySize + CM_KERNEL_BINARY_PADDING_SIZE;

                // Flag this kernel as loading - one single kernel instance is needed, not multiple!
                // If the same kernel is used multiple times, avoid multiple reservations/loads
                krnAllocation[i]->dwFlags = RENDERHAL_KERNEL_ALLOCATION_LOADING;
            }
        }

        // Use Hit/Miss ratio to ignore eventual cache misses
        // This code prevents ISH reallocation in case of eventual cache misses
        while (state->dshKernelCacheHit >= HAL_CM_KERNEL_CACHE_HIT_TO_MISS_RATIO)
        {
            if (state->dshKernelCacheMiss > 0) state->dshKernelCacheMiss--;
            state->dshKernelCacheHit -= HAL_CM_KERNEL_CACHE_HIT_TO_MISS_RATIO;
        }

        // Grow the kernel heap if too many kernels are being reloaded or there isn't enough room to load all kernels
        if (state->dshKernelCacheMiss > HAL_CM_KERNEL_CACHE_MISS_THRESHOLD ||
            renderHal->pfnRefreshDynamicKernels(renderHal, totalSize, blockSize, blockCount) != MOS_STATUS_SUCCESS)
        {
            renderHal->pfnExpandKernelStateHeap(renderHal, (uint32_t)totalSize);
            state->dshKernelCacheHit = 0;
            state->dshKernelCacheMiss = 0;
            continue;
        }

        // blockSize/blockCount define a list of blocks that must be loaded in current ISH for the
        // kernels not yet present. Pre-existing kernels are marked as bStatic to avoid being unloaded here
        if (blockCount > 0)
        {
            // Allocate array of kernels
            MHW_STATE_HEAP_DYNAMIC_ALLOC_PARAMS params;
            params.piSizes = (int32_t*)blockSize;
            params.iCount = blockCount;
            params.dwAlignment = RENDERHAL_KERNEL_BLOCK_ALIGN;
            params.bHeapAffinity = true;                                     // heap affinity - load all kernels in the same heap
            params.pHeapAffinity = renderHal->pMhwStateHeap->GetISHPointer();    // Select the active instruction heap
            params.dwScratchSpace = 0;
            params.bZeroAssignedMem = true;
            params.bStatic = true;
            params.bGrow = false;

            // Try to allocate array of blocks; if it fails, we may need to clear some space or grow the heap!
            memoryBlock = renderHal->pMhwStateHeap->AllocateDynamicBlockDyn(MHW_ISH_TYPE, &params);
            if (!memoryBlock)
            {
                // Reset flags
                for (int i = 0; i < kernelCount; i++)
                {
                    if (krnAllocation[i] && krnAllocation[i]->dwFlags == RENDERHAL_KERNEL_ALLOCATION_LOADING)
                    {
                        krnAllocation[i]->dwFlags = RENDERHAL_KERNEL_ALLOCATION_STALE;
                    }
                }

                if (renderHal->pfnRefreshDynamicKernels(renderHal, totalSize, blockSize, blockCount) != MOS_STATUS_SUCCESS)
                {
                    renderHal->pfnExpandKernelStateHeap(renderHal, (uint32_t)totalSize);
                }
                continue;
            }

            // All blocks are allocated in ISH
            // Setup kernel allocations, load kernel binaries
            for (int32_t i = 0; i < kernelCount; i++)
            {
                // Load kernels in ISH
                if (!krnAllocation[i]->pMemoryBlock)
                {
                    PCM_HAL_KERNEL_PARAM      kernelParam = kernelArray[i];
                    PRENDERHAL_KRN_ALLOCATION allocation = krnAllocation[i];
                    if (memoryBlock)
                    {
                        allocation->iKID = -1;
                        allocation->iKUID = static_cast<int>((kernelArray[i]->kernelId >> 32));
                        allocation->iKCID = -1;
                        FrameTrackerTokenFlat_SetProducer(&allocation->trackerToken, &renderHal->trackerProducer);
                        FrameTrackerTokenFlat_Merge(&allocation->trackerToken,
                                                    renderHal->currentTrackerIndex,
                                                    renderHal->trackerProducer.GetNextTracker(renderHal->currentTrackerIndex));
                        allocation->dwOffset = memoryBlock->dwDataOffset;
                        allocation->iSize = kernelArray[i]->kernelBinarySize + CM_KERNEL_BINARY_PADDING_SIZE;
                        allocation->dwCount = 0;
                        allocation->dwFlags = RENDERHAL_KERNEL_ALLOCATION_USED;
                        allocation->Params = state->kernelParamsRenderHal.Params;
                        allocation->pMhwKernelParam = &state->kernelParamsMhw;
                        allocation->pMemoryBlock = memoryBlock;

                        // Copy kernel data
                        // Copy MovInstruction First
                        if (allocation->pMemoryBlock &&
                            allocation->pMemoryBlock->dwDataSize >= kernelParam->kernelBinarySize)
                        {
                            MOS_SecureMemcpy(allocation->pMemoryBlock->pDataPtr,
                                kernelParam->movInsDataSize,
                                kernelParam->movInsData,
                                kernelParam->movInsDataSize);

                            // Copy Cm Kernel Binary
                            MOS_SecureMemcpy(allocation->pMemoryBlock->pDataPtr + kernelParam->movInsDataSize,
                                kernelParam->kernelBinarySize - kernelParam->movInsDataSize,
                                kernelParam->kernelBinary,
                                kernelParam->kernelBinarySize - kernelParam->movInsDataSize);

                            // Padding bytes dummy instructions after kernel binary to resolve page fault issue
                            MOS_ZeroMemory(allocation->pMemoryBlock->pDataPtr + kernelParam->kernelBinarySize, CM_KERNEL_BINARY_PADDING_SIZE);
                        }

                        // Get next memory block returned as part of the array
                        memoryBlock = memoryBlock->pNext;
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
        for (int32_t i = 0; i < kernelCount; i++)
        {
            renderHal->pfnTouchDynamicKernel(renderHal, krnAllocation[i]);
        }
    }
    state->criticalSectionDSH->Release();
    return hr;
}

MOS_STATUS HalCm_DSH_GetDynamicStateConfiguration(
    PCM_HAL_STATE                         state,
    PRENDERHAL_DYNAMIC_MEDIA_STATE_PARAMS params,
    uint32_t                              numKernels,
    PCM_HAL_KERNEL_PARAM                 *kernels,
    uint32_t                              *piCurbeOffsets)
{
    PCM_HAL_KERNEL_PARAM      cmKernel;

    PRENDERHAL_INTERFACE renderHal = state->renderHal;
    PRENDERHAL_KRN_ALLOCATION krnAllocation;

    MOS_ZeroMemory(params, sizeof(RENDERHAL_DYNAMIC_MEDIA_STATE_PARAMS));

    params->iMaxMediaIDs = numKernels;

    for (uint32_t i = 0; i < numKernels; i++)
    {
        cmKernel = kernels[i];

        // get max curbe size
        int32_t curbeSize = MOS_ALIGN_CEIL(cmKernel->totalCurbeSize, state->renderHal->dwCurbeBlockAlign);
        int32_t curbeOffset = piCurbeOffsets[i] + curbeSize;
        params->iMaxCurbeOffset = MOS_MAX(params->iMaxCurbeOffset, curbeOffset);
        params->iMaxCurbeSize += curbeSize;

        // get max spill size
        params->iMaxSpillSize = MOS_MAX(params->iMaxSpillSize, (int32_t)cmKernel->spillSize);

        // check if kernel already used - increase Max Media ID to allow BB reuse logic
        krnAllocation = renderHal->pfnSearchDynamicKernel(renderHal, static_cast<int>((cmKernel->kernelId >> 32)), -1);
        if (krnAllocation)
        {
            params->iMaxMediaIDs = MOS_MAX(params->iMaxMediaIDs, krnAllocation->iKID + 1);
        }
    }

    if (state->useNewSamplerHeap == true)
    {
        // Update offset to the base of first kernel and update count
        // for 3D sampler, update indirect state information
        unsigned int heapOffset = 0;
        unsigned int sampler3DCount = 0;
        MHW_SAMPLER_STATE_PARAM samplerParamMhw = {};
        SamplerParam samplerParam = {};
        samplerParamMhw.SamplerType = MHW_SAMPLER_TYPE_3D;
        state->cmHalInterface->GetSamplerParamInfoForSamplerType(&samplerParamMhw, samplerParam);
        for (unsigned int i = 0; i < numKernels; i++)
        {
            cmKernel = kernels[i];
            std::list<SamplerParam> *sampler_heap = cmKernel->samplerHeap;
            std::list<SamplerParam>::iterator iter;

            heapOffset = MOS_ALIGN_CEIL(heapOffset, MHW_SAMPLER_STATE_ALIGN);
            state->taskParam->samplerOffsetsByKernel[i] = heapOffset;
            state->taskParam->samplerCountsByKernel[i] = sampler_heap->size();

            if (sampler_heap->size() > 0)
            {
                heapOffset = heapOffset + sampler_heap->back().heapOffset + sampler_heap->back().size;

                // 3D sampler needs indirect sampler heap, so calculates the required size
                // and offset for indirect sampler heap.
                unsigned int max3DCount = 0;
                for (iter = sampler_heap->begin(); iter != sampler_heap->end(); ++iter)
                {
                    if (iter->elementType == samplerParam.elementType)
                    {
                        if (iter->userDefinedBti == true)
                        {
                            max3DCount = iter->bti + 1;
                        }
                        else
                        {
                            max3DCount += 1;
                        }
                    }
                }
                heapOffset = MOS_ALIGN_CEIL(heapOffset, MHW_SAMPLER_STATE_ALIGN);
                state->taskParam->samplerIndirectOffsetsByKernel[i] = heapOffset;
                heapOffset += max3DCount * state->renderHal->pHwSizes->dwSizeSamplerIndirectState;
                sampler3DCount += max3DCount;
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

        samplerParamMhw.SamplerType = MHW_SAMPLER_TYPE_AVS;
        state->cmHalInterface->GetSamplerParamInfoForSamplerType(&samplerParamMhw, samplerParam);
        params->iMaxSamplerIndex3D = (sampler3DCount + numKernels - 1) / numKernels;
        params->iMaxSamplerIndexAVS = ((heapOffset - sampler3DCount * (state->renderHal->pHwSizes->dwSizeSamplerState + state->renderHal->pHwSizes->dwSizeSamplerIndirectState)) + samplerParam.btiMultiplier * numKernels - 1) / (samplerParam.btiMultiplier * numKernels);
    }
    else
    {
        // Get total sampler count

        // Initialize pointers to samplers and reset sampler index table
        MOS_FillMemory(state->samplerIndexTable, state->cmDeviceParam.maxSamplerTableSize, CM_INVALID_INDEX);

        params->iMaxSamplerIndex3D = CM_MAX_3D_SAMPLER_SIZE;
        params->iMaxSamplerIndexAVS = CM_MAX_AVS_SAMPLER_SIZE;
        params->iMaxSamplerIndexConv = 0;
        params->iMaxSamplerIndexMisc = 0;
        params->iMax8x8Tables = CM_MAX_AVS_SAMPLER_SIZE;
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HalCm_DSH_UnregisterKernel(
    PCM_HAL_STATE               state,
    uint64_t                    kernelId)
{
    PRENDERHAL_INTERFACE renderHal = state->renderHal;
    PRENDERHAL_KRN_ALLOCATION krnAllocation = renderHal->pfnSearchDynamicKernel(renderHal, static_cast<int>((kernelId >> 32)), -1);
    if (krnAllocation)
    {
        state->criticalSectionDSH->Acquire();
        renderHal->pfnUnregisterKernel(renderHal, krnAllocation);
        state->criticalSectionDSH->Release();
    }
    return MOS_STATUS_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose: Setup Sampler State
//| Returns: Result of the operation
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_SetupSamplerState(
    PCM_HAL_STATE                   state,
    PCM_HAL_KERNEL_PARAM            kernelParam,
    PCM_HAL_KERNEL_ARG_PARAM        argParam,
    PCM_HAL_INDEX_PARAM             indexParam,
    int32_t                         mediaID,
    uint32_t                        threadIndex,
    uint8_t                         *buffer)
{
    MOS_STATUS                  eStatus;
    PRENDERHAL_INTERFACE        renderHal;
    PMHW_SAMPLER_STATE_PARAM    samplerParam;
    uint8_t                     *src;
    uint8_t                     *dst;
    uint32_t                    index;
    uint32_t                    samplerIndex = 0;
    void                        *sampler = nullptr;
    uint32_t                    samplerOffset = 0;

    eStatus = MOS_STATUS_SUCCESS;

    CM_CHK_NULL_GOTOFINISH_MOSERROR(state);

    renderHal    = state->renderHal;

    if (indexParam->samplerIndexCount >= (uint32_t)renderHal->StateHeapSettings.iSamplers)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        CM_ASSERTMESSAGE(
            "Exceeded Max samplers '%d'",
            indexParam->samplerIndexCount);
        goto finish;
    }

    // Get the Index to sampler array from the kernel data
    //----------------------------------
    CM_ASSERT(argParam->unitSize == sizeof(index));
    //----------------------------------

    src    = argParam->firstValue + (threadIndex * argParam->unitSize);
    index  = *((uint32_t*)src);

    // check to see if the data present for the sampler in the array
    if (index >= state->cmDeviceParam.maxSamplerTableSize ||
        !state->samplerTable[index].bInUse)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        CM_ASSERTMESSAGE(
            "Invalid Sampler array index '%d'", index);
        goto finish;
    }
    // Setup samplers
    samplerParam = &state->samplerTable[index];

    if (state->useNewSamplerHeap == true)
    {
        std::list<SamplerParam>::iterator iter;
        for (iter = kernelParam->samplerHeap->begin(); iter != kernelParam->samplerHeap->end(); ++iter)
        {
            if ((iter->samplerTableIndex == index)&&(iter->regularBti == true))
            {
                break;
            }
        }
        if (iter != kernelParam->samplerHeap->end())
        {
            samplerIndex = iter->bti;
        }
        else
        {
            // There must be incorrect internal logic
            CM_ASSERTMESSAGE( "BTI calculation error in cm_hal\n");
            return MOS_STATUS_UNKNOWN;
        }
        HalCm_GetSamplerOffset(state, renderHal, mediaID, iter->heapOffset, iter->bti, samplerParam, &samplerOffset);
    }
    else
    {
        // Check to see if sampler is already assigned
        samplerIndex = state->samplerIndexTable[index];
        if ((int)samplerIndex == CM_INVALID_INDEX)
        {

            switch (state->samplerTable[index].ElementType)
            {

                case MHW_Sampler2Elements:
                {
                    unsigned int index = 0;
                    index = state->samplerStatistics.samplerIndexBase[MHW_Sampler2Elements];
                    while (state->samplerIndexTable[index] != CM_INVALID_INDEX)
                    {
                        index++;
                    }
                    samplerIndex = index;
                    state->samplerStatistics.samplerIndexBase[MHW_Sampler2Elements] = (index + 1);
                    break;
                }
                case MHW_Sampler4Elements:
                {
                    unsigned int index = 0;
                    index = state->samplerStatistics.samplerIndexBase[MHW_Sampler4Elements];
                    while (state->samplerIndexTable[index] != CM_INVALID_INDEX)
                    {
                        index++;
                    }
                    samplerIndex = index;
                    state->samplerStatistics.samplerIndexBase[MHW_Sampler4Elements] = (index + 1);
                    break;
                }
                case MHW_Sampler8Elements:
                {
                    unsigned int index = 0;
                    index = state->samplerStatistics.samplerIndexBase[MHW_Sampler8Elements];
                    while (state->samplerIndexTable[index] != CM_INVALID_INDEX)
                    {
                        index++;
                    }
                    samplerIndex = index;
                    state->samplerStatistics.samplerIndexBase[MHW_Sampler8Elements] = (index + 1);
                    break;
                }
                case MHW_Sampler64Elements:
                {
                    unsigned int index = 0;
                    index = state->samplerStatistics.samplerIndexBase[MHW_Sampler64Elements];
                    while (state->samplerIndexTable[index] != CM_INVALID_INDEX)
                    {
                        index += index + 2;
                    }
                    samplerIndex = index;
                    state->samplerStatistics.samplerIndexBase[MHW_Sampler64Elements] = (index + 2);

                    break;
                }
                case MHW_Sampler128Elements:
                {
                    unsigned int index = 0;
                    index = state->samplerStatistics.samplerIndexBase[MHW_Sampler128Elements];
                    while (state->samplerIndexTable[index] != CM_INVALID_INDEX)
                    {
                        index++;
                    }
                    samplerIndex = index;
                    state->samplerStatistics.samplerIndexBase[MHW_Sampler128Elements] = (index + 1);

                    break;
                }
                default:
                    CM_ASSERTMESSAGE("Invalid sampler type '%d'.", state->samplerTable[index].SamplerType);
                    break;
            }
        }

        CM_CHK_MOSSTATUS_GOTOFINISH(renderHal->pfnGetSamplerOffsetAndPtr(
            renderHal, 
            mediaID, 
            samplerIndex, 
            samplerParam, 
            &samplerOffset, 
            &sampler));
    }
    CM_CHK_MOSSTATUS_GOTOFINISH(renderHal->pMhwStateHeap->AddSamplerStateData(
        samplerOffset, 
        &(renderHal->pStateHeap->pCurMediaState->pDynamicState->memoryBlock), 
        samplerParam));

    state->samplerIndexTable[index] = (unsigned char)samplerIndex;

    // Update the Batch Buffer
    if (buffer)
    {
        dst = buffer + argParam->payloadOffset;
        *((uint32_t*)dst) = samplerIndex;
    }

finish:
    return eStatus;
}

//*-----------------------------------------------------------------------------
//| Purpose: Setup Sampler State
//| Returns: Result of the operation
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_SetupSamplerStateWithBTIndex(
    PCM_HAL_STATE                   state,
    PCM_HAL_KERNEL_PARAM            kernelParam,
    PCM_HAL_SAMPLER_BTI_ENTRY       samplerBTIEntry,
    uint32_t                        samplerCount,
    int32_t                         mediaID )
{
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;
    PRENDERHAL_INTERFACE            renderHal;
    PMHW_SAMPLER_STATE_PARAM        samplerParam;
    uint32_t                        index;
    uint32_t                        samplerIndex;
    void                            *sampler = nullptr;
    uint32_t                        samplerOffset = 0;

    renderHal = state->renderHal;

    if (state->useNewSamplerHeap != true)
    {
        if (samplerCount >= (uint32_t)renderHal->StateHeapSettings.iSamplers)
        {
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            CM_ASSERTMESSAGE(
                "Exceeded Max samplers '%d'",
                samplerCount);
            goto finish;
        }
    }

    index = samplerBTIEntry[ samplerCount ].samplerIndex;

    // check to see if the data present for the sampler in the array
    if ( index >= state->cmDeviceParam.maxSamplerTableSize ||
         !state->samplerTable[ index ].bInUse )
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        CM_ASSERTMESSAGE(
            "Invalid Sampler array index '%d'", index );
        goto finish;
    }

    samplerIndex = samplerBTIEntry[ samplerCount ].samplerBTI;
    // Setup samplers
    samplerParam = &state->samplerTable[ index ];

    if (state->useNewSamplerHeap == true)
    {
        std::list<SamplerParam>::iterator iter;
        for (iter = kernelParam->samplerHeap->begin(); iter != kernelParam->samplerHeap->end(); ++iter)
        {
            if ((iter->samplerTableIndex == index) && (iter->bti == samplerIndex) && (iter->userDefinedBti == true))
            {
                break;
            }
        }
        if (iter == kernelParam->samplerHeap->end())
        {
            // There must be incorrect internal logic
            CM_ASSERTMESSAGE("BTI calculation error in cm_hal\n");
            return MOS_STATUS_UNKNOWN;
        }
        HalCm_GetSamplerOffset(state, renderHal, mediaID, iter->heapOffset, iter->bti, samplerParam, &samplerOffset);
    }
    else
    {
        CM_CHK_MOSSTATUS_GOTOFINISH(renderHal->pfnGetSamplerOffsetAndPtr(renderHal, mediaID, samplerIndex, samplerParam, &samplerOffset, &sampler));
    }

    CM_CHK_MOSSTATUS_GOTOFINISH(renderHal->pMhwStateHeap->AddSamplerStateData(
        samplerOffset,
        &(renderHal->pStateHeap->pCurMediaState->pDynamicState->memoryBlock),
        samplerParam));

finish:
    return eStatus;
}

//*-----------------------------------------------------------------------------
//| Purpose: Setup Buffer surface State
//| Returns: Result of the operation
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_SetupBufferSurfaceState(
    PCM_HAL_STATE               state,
    PCM_HAL_KERNEL_ARG_PARAM    argParam,
    PCM_HAL_INDEX_PARAM         indexParam,
    int32_t                     bindingTable,
    int16_t                     globalSurface,
    uint32_t                    threadIndex,
    uint8_t                     *buffer)
{
    MOS_STATUS                  eStatus;
    RENDERHAL_SURFACE               surface;
    PMOS_SURFACE                    mosSurface;
    RENDERHAL_SURFACE_STATE_PARAMS  surfaceParam;
    PRENDERHAL_INTERFACE            renderHal;
    PRENDERHAL_SURFACE_STATE_ENTRY  surfaceEntry;
    uint8_t                     *src;
    uint8_t                     *dst;
    uint32_t                    index;
    uint32_t                    btIndex;
    uint16_t                    memObjCtl;
    uint32_t                    offsetSrc;
    PRENDERHAL_STATE_HEAP       stateHeap;
    CM_SURFACE_BTI_INFO         surfBTIInfo;

    eStatus              = MOS_STATUS_UNKNOWN;
    renderHal      = state->renderHal;
    //GT-PIN
    PCM_HAL_TASK_PARAM     taskParam = state->taskParam;

    // Get the Index to Buffer array from the kernel data
    CM_ASSERT(argParam->unitSize == sizeof(index));

    //Init surfBTIInfo
    state->cmHalInterface->GetHwSurfaceBTIInfo(&surfBTIInfo);

    src      = argParam->firstValue + (threadIndex * argParam->unitSize);
    index    = *((uint32_t*)src) & CM_SURFACE_MASK;
    if (index == CM_NULL_SURFACE)
    {
        if (buffer)
        {
            dst = buffer + argParam->payloadOffset;
            *((uint32_t*)dst) = CM_NULL_SURFACE_BINDING_INDEX;
        }

        eStatus = MOS_STATUS_SUCCESS;
        goto finish;
    }

    memObjCtl = state->bufferTable[index].memObjCtl;
    if (!memObjCtl)
    {
        memObjCtl = CM_DEFAULT_CACHE_TYPE;
    }

    // check to see if index is valid
    if (index >= state->cmDeviceParam.maxBufferTableSize ||
        (state->bufferTable[index].size == 0))
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        CM_ASSERTMESSAGE(
            "Invalid Buffer surface array index '%d'", index);
        goto finish;
    }

    // Check to see if buffer is already assigned
    btIndex = state->btiBufferIndexTable[index].BTI.regularSurfIndex;
    if (btIndex == ( unsigned char )CM_INVALID_INDEX || argParam->aliasCreated == true)
    {
        if (globalSurface < 0)
        {
            btIndex = HalCm_GetFreeBindingIndex(state, indexParam, 1);
        }
        else
        {
            btIndex = globalSurface + surfBTIInfo.reservedSurfaceStart; //CM_BINDING_START_INDEX_OF_GLOBAL_SURFACE(state);
            if ( btIndex >=  (surfBTIInfo.reservedSurfaceStart + CM_MAX_GLOBAL_SURFACE_NUMBER) ) {
                eStatus = MOS_STATUS_INVALID_PARAMETER;
                CM_ASSERTMESSAGE("Exceeded Max Global Surfaces '%d'", btIndex);
                goto finish;
            }
        }
        // Get Details of Buffer surface and fill the surface
        CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_GetSurfaceAndRegister(state, &surface, CM_ARGUMENT_SURFACEBUFFER, index, 0));

        MOS_ZeroMemory(&surfaceParam, sizeof(surfaceParam));

        // override the buffer offset and size if alias is used
        mosSurface = &(surface.OsSurface);
        if (state->bufferTable[index].surfaceStateEntry[argParam->aliasIndex / state->surfaceArraySize].surfaceStateSize)
        {
            mosSurface->dwWidth = state->bufferTable[index].surfaceStateEntry[argParam->aliasIndex / state->surfaceArraySize].surfaceStateSize;
            mosSurface->dwOffset = state->bufferTable[index].surfaceStateEntry[argParam->aliasIndex / state->surfaceArraySize].surfaceStateOffset;
            surface.rcSrc.right = mosSurface->dwWidth;
            surface.rcDst.right = mosSurface->dwWidth;
        }
        // override the mocs value if it is set
        if (state->bufferTable[index].surfaceStateEntry[argParam->aliasIndex / state->surfaceArraySize].surfaceStateMOCS)
        {
            memObjCtl = state->bufferTable[index].surfaceStateEntry[argParam->aliasIndex / state->surfaceArraySize].surfaceStateMOCS;
        }

        //Cache configurations
        state->cmHalInterface->HwSetSurfaceMemoryObjectControl(memObjCtl, &surfaceParam);

        // Set the bRenderTarget by default
        surfaceParam.bRenderTarget = true;

        // Setup Buffer surface
        CM_CHK_MOSSTATUS_GOTOFINISH(renderHal->pfnSetupBufferSurfaceState(
                renderHal,
                &surface,
                &surfaceParam,
                &surfaceEntry));

        // Bind the surface State
        CM_ASSERT(((int32_t)btIndex) < renderHal->StateHeapSettings.iSurfacesPerBT + surfBTIInfo.normalSurfaceStart);
        CM_CHK_MOSSTATUS_GOTOFINISH(renderHal->pfnBindSurfaceState(
               renderHal,
               bindingTable,
               btIndex,
               surfaceEntry));

        if ((taskParam->surfEntryInfoArrays.kernelNum != 0) &&
            (taskParam->surfEntryInfoArrays.surfEntryInfosArray != nullptr))
        {
            //GT-Pin
           uint32_t dummy = 0;
           CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_GetSurfaceDetails(
                   state,
                   indexParam,
                   btIndex,
                   surface.OsSurface,
                   globalSurface,
                   nullptr,
                   dummy,
                   surfaceParam,
                   CM_ARGUMENT_SURFACEBUFFER));
        }

        // Update index to table
        state->btiBufferIndexTable[ index ].BTI.regularSurfIndex = btIndex;
        state->btiBufferIndexTable[ index ].nPlaneNumber = 1;

        stateHeap = renderHal->pStateHeap;
        offsetSrc = ( stateHeap->iCurSshBufferIndex * stateHeap->dwSshIntanceSize ) +   // Points to the Base of Current SSH Buffer Instance
                            ( stateHeap->iBindingTableOffset ) +                       // Moves the pointer to Base of Array of Binding Tables
                            ( bindingTable * stateHeap->iBindingTableSize ) +         // Moves the pointer to a Particular Binding Table
                            ( btIndex * sizeof( uint32_t ) );                             // Move the pointer to correct entry

        state->btiBufferIndexTable[ index ].BTITableEntry.regularBtiEntryPosition = stateHeap->pSshBuffer + offsetSrc;
    }
    else
    {
        stateHeap = renderHal->pStateHeap;

        // Get Offset to Current Binding Table
        uint32_t offsetCurrentBTStart = ( stateHeap->iCurSshBufferIndex * stateHeap->dwSshIntanceSize ) +     // Points to the Base of Current SSH Buffer Instance
                                       ( stateHeap->iBindingTableOffset ) +                             // Moves the pointer to Base of Array of Binding Tables
                                       ( bindingTable * stateHeap->iBindingTableSize );                // Moves the pointer to a Particular Binding Table

        uint32_t *currentBTStart = ( uint32_t *)( stateHeap->pSshBuffer + offsetCurrentBTStart );

        int nEntryIndex = (int) ((uint32_t*)( state->btiBufferIndexTable[ index ].BTITableEntry.regularBtiEntryPosition ) - currentBTStart);

        if ( ( nEntryIndex < 0 ) || ( nEntryIndex >= renderHal->StateHeapSettings.iSurfacesPerBT ) )
        {
            uint32_t surfaceEntries = state->btiBufferIndexTable[ index ].nPlaneNumber;
            if ( globalSurface < 0 )
            {
                btIndex = HalCm_GetFreeBindingIndex( state, indexParam, surfaceEntries );
            }
            else
            {
                btIndex = globalSurface + surfBTIInfo.reservedSurfaceStart;
                if ( btIndex >= (surfBTIInfo.reservedSurfaceStart + CM_MAX_GLOBAL_SURFACE_NUMBER ) )
                {
                    eStatus = MOS_STATUS_INVALID_PARAMETER;
                    CM_ASSERTMESSAGE( "Exceeded Max Global Surfaces '%d'", btIndex );
                    goto finish;
                }
            }

            // Bind the surface State
            CM_ASSERT( ( ( int32_t )btIndex ) < renderHal->StateHeapSettings.iSurfacesPerBT + surfBTIInfo.normalSurfaceStart);

            // Get Offset to Current Binding Table
            uint32_t offsetDst = offsetCurrentBTStart + ( btIndex * sizeof( uint32_t ) );                             // Move the pointer to correct entry

            uint32_t *bindingTableEntry = ( uint32_t *)( stateHeap->pSshBuffer + offsetDst );
            MOS_SecureMemcpy( bindingTableEntry, sizeof( uint32_t ) * surfaceEntries, state->btiBufferIndexTable[ index ].BTITableEntry.regularBtiEntryPosition, sizeof( uint32_t ) * surfaceEntries );

            // Update index to table
            state->btiBufferIndexTable[ index ].BTI.regularSurfIndex = btIndex;
            state->btiBufferIndexTable[ index ].BTITableEntry.regularBtiEntryPosition = bindingTableEntry;
        }
    }

    // Update the Batch Buffer
    if (buffer)
    {
        dst = buffer + argParam->payloadOffset;
        *((uint32_t*)dst) = btIndex;
    }
    eStatus = MOS_STATUS_SUCCESS;

finish:
    return eStatus;
}

//*-----------------------------------------------------------------------------
//| Purpose: Setup 3D surface State
//| Returns: Result of the operation
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_Setup3DSurfaceState(
    PCM_HAL_STATE               state,
    PCM_HAL_KERNEL_ARG_PARAM    argParam,
    PCM_HAL_INDEX_PARAM         indexParam,
    int32_t                     bindingTable,
    uint32_t                    threadIndex,
    uint8_t                     *buffer)
{
    MOS_STATUS                  eStatus;
    PRENDERHAL_INTERFACE            renderHal;
    RENDERHAL_SURFACE               surface;
    RENDERHAL_SURFACE_STATE_PARAMS  surfaceParam;
    PRENDERHAL_SURFACE_STATE_ENTRY  surfaceEntries[MHW_MAX_SURFACE_PLANES];
    RENDERHAL_GET_SURFACE_INFO      info;
    uint8_t                     *src;
    uint8_t                     *dst;
    int32_t                     nSurfaceEntries;
    uint32_t                    index;
    uint32_t                    btIndex;
    uint16_t                    memObjCtl;
    uint32_t                    i;
    uint32_t                    offsetSrc;
    PRENDERHAL_STATE_HEAP       stateHeap;
    CM_SURFACE_BTI_INFO         surfBTIInfo;

    eStatus              = MOS_STATUS_UNKNOWN;
    renderHal  = state->renderHal;
    //GT-PIN
    PCM_HAL_TASK_PARAM     taskParam = state->taskParam;

    state->cmHalInterface->GetHwSurfaceBTIInfo(&surfBTIInfo);

    // Get the Index to 3dsurface array from the kernel data
    CM_ASSERT(argParam->unitSize == sizeof(index));
    src      = argParam->firstValue + (threadIndex * argParam->unitSize);
    index    = *((uint32_t*)src) & CM_SURFACE_MASK;
    if (index == CM_NULL_SURFACE)
    {
        if (buffer)
        {
            dst = buffer + argParam->payloadOffset;
            *((uint32_t*)dst) = CM_NULL_SURFACE_BINDING_INDEX;
        }

        eStatus = MOS_STATUS_SUCCESS;
        goto finish;
    }

    memObjCtl = state->surf3DTable[index].memObjCtl;
    if (!memObjCtl)
    {
        memObjCtl = CM_DEFAULT_CACHE_TYPE;
    }

    // check to see if the data present for the 3d surface in the array
    if ((index >= state->cmDeviceParam.max3DSurfaceTableSize)            ||
        Mos_ResourceIsNull(&state->surf3DTable[index].osResource))
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        CM_ASSERTMESSAGE(
            "Invalid 2D surface array index '%d'", index);
        goto finish;
    }

    // Check to see if surface is already assigned
    btIndex = state->bti3DIndexTable[index].BTI.regularSurfIndex;
    if ( btIndex == ( unsigned char )CM_INVALID_INDEX )
    {
        uint32_t tempPlaneIndex = 0;
        nSurfaceEntries = 0;

        // Get Details of 3D surface and fill the surface
        CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_GetSurfaceAndRegister(state, &surface, CM_ARGUMENT_SURFACE3D, index, 0));

        // Setup 3D surface
        MOS_ZeroMemory(&surfaceParam, sizeof(surfaceParam));
        surfaceParam.Type       = renderHal->SurfaceTypeDefault;
        surfaceParam.Boundary   = RENDERHAL_SS_BOUNDARY_ORIGINAL;
        surfaceParam.bRenderTarget = true;

        //Cache configurations
        state->cmHalInterface->HwSetSurfaceMemoryObjectControl(memObjCtl, &surfaceParam);

        CM_CHK_MOSSTATUS_GOTOFINISH(renderHal->pfnSetupSurfaceState(
                    renderHal,
                    &surface,
                    &surfaceParam,
                    &nSurfaceEntries,
                    surfaceEntries,
                    nullptr));

        MOS_ZeroMemory(&info, sizeof(RENDERHAL_GET_SURFACE_INFO));

        CM_CHK_MOSSTATUS_GOTOFINISH(RenderHal_GetSurfaceInfo(
            state->osInterface,
            &info,
            &surface.OsSurface));

        btIndex = HalCm_GetFreeBindingIndex(state, indexParam, nSurfaceEntries);
        for (i = 0; i < (uint32_t)nSurfaceEntries; i++)
        {
            *(surfaceEntries[i]->pSurface) = surface.OsSurface;

            // Bind the surface State
            CM_CHK_MOSSTATUS_GOTOFINISH(renderHal->pfnBindSurfaceState(
                        renderHal,
                        bindingTable,
                        btIndex + i,
                        surfaceEntries[i]));

            if ((taskParam->surfEntryInfoArrays.kernelNum != 0) &&
                (taskParam->surfEntryInfoArrays.surfEntryInfosArray != nullptr))
            {
                CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_GetSurfaceDetails(
                        state,
                        indexParam,
                        btIndex + i,
                        surface.OsSurface,
                        0,
                        surfaceEntries[i],
                        tempPlaneIndex,
                        surfaceParam,
                        CM_ARGUMENT_SURFACE3D));
            }
        }
        // Update index to table
        state->bti3DIndexTable[ index ].BTI.regularSurfIndex = btIndex;
        state->bti3DIndexTable[ index ].nPlaneNumber = nSurfaceEntries;

        stateHeap = renderHal->pStateHeap;
        offsetSrc = ( stateHeap->iCurSshBufferIndex * stateHeap->dwSshIntanceSize ) +  // Points to the Base of Current SSH Buffer Instance
                            ( stateHeap->iBindingTableOffset ) +                       // Moves the pointer to Base of Array of Binding Tables
                            ( bindingTable * stateHeap->iBindingTableSize ) +         // Moves the pointer to a Particular Binding Table
                            ( btIndex * sizeof( uint32_t ) );                             // Move the pointer to correct entry

        state->bti3DIndexTable[ index ].BTITableEntry.regularBtiEntryPosition = stateHeap->pSshBuffer + offsetSrc;
    }
    else
    {
        stateHeap = renderHal->pStateHeap;

        // Get Offset to Current Binding Table
        uint32_t offsetCurrentBTStart = ( stateHeap->iCurSshBufferIndex * stateHeap->dwSshIntanceSize ) +     // Points to the Base of Current SSH Buffer Instance
                                       ( stateHeap->iBindingTableOffset ) +                             // Moves the pointer to Base of Array of Binding Tables
                                       ( bindingTable * stateHeap->iBindingTableSize );                // Moves the pointer to a Particular Binding Table

        uint32_t *currentBTStart = ( uint32_t *)( stateHeap->pSshBuffer + offsetCurrentBTStart );

        int nEntryIndex = (int)((uint32_t*)( state->bti3DIndexTable[ index ].BTITableEntry.regularBtiEntryPosition ) - currentBTStart);

        if ( ( nEntryIndex < 0 ) || ( nEntryIndex >= renderHal->StateHeapSettings.iSurfacesPerBT ) )
        {
            nSurfaceEntries = state->bti3DIndexTable[ index ].nPlaneNumber;
            btIndex = HalCm_GetFreeBindingIndex( state, indexParam, nSurfaceEntries );

            // Bind the surface State
            CM_ASSERT( ( ( int32_t )btIndex ) < renderHal->StateHeapSettings.iSurfacesPerBT + surfBTIInfo.normalSurfaceStart);

            // Get Offset to Current Binding Table
            uint32_t offsetDst = offsetCurrentBTStart + ( btIndex * sizeof( uint32_t ) );                             // Move the pointer to correct entry

            uint32_t *bindingTableEntry = ( uint32_t *)( stateHeap->pSshBuffer + offsetDst );
            MOS_SecureMemcpy( bindingTableEntry, sizeof( uint32_t ) * nSurfaceEntries, state->bti3DIndexTable[ index ].BTITableEntry.regularBtiEntryPosition, sizeof( uint32_t ) * nSurfaceEntries );

            // Update index to table
            state->bti3DIndexTable[ index ].BTI.regularSurfIndex = btIndex;
            state->bti3DIndexTable[ index ].BTITableEntry.regularBtiEntryPosition = bindingTableEntry;
        }
    }

    // Update the Batch Buffer
    if (buffer)
    {
        dst = buffer + argParam->payloadOffset;
        *((uint32_t*)dst) = btIndex;
    }

    eStatus = MOS_STATUS_SUCCESS;

finish:
    return eStatus;
}

/*----------------------------------------------------------------------------
| Purpose   : Set's surface state interlaced settings
| Returns   : dword value
\---------------------------------------------------------------------------*/
MOS_STATUS HalCm_HwSetSurfaceProperty(
    PCM_HAL_STATE                   state,
    CM_FRAME_TYPE                   frameType,
    PRENDERHAL_SURFACE_STATE_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    switch (frameType)
    {
    case CM_FRAME:
        params->bVertStride = 0;
        params->bVertStrideOffs = 0;
        break;
    case CM_TOP_FIELD:
        params->bVertStride = 1;
        params->bVertStrideOffs = 0;
        break;
    case CM_BOTTOM_FIELD:
        params->bVertStride = 1;
        params->bVertStrideOffs = 1;
        break;
    default:
        eStatus = MOS_STATUS_UNKNOWN;
    }

    return eStatus;
}

// A special treatment of NV12 format. Offset of the UV plane in an NV12 surface is adjusted, so
// this plane can be accessed as a separate R8G8 surface in kernels.
static bool UpdateSurfaceAliasPlaneOffset(
    CM_HAL_SURFACE2D_SURFACE_STATE_PARAM *surfaceStateParam,
    MOS_SURFACE *mosSurface)
{
    if (Format_R8G8UN != surfaceStateParam->format
        || Format_NV12 != mosSurface->Format)
    {
        mosSurface->Format
                = static_cast<MOS_FORMAT>(surfaceStateParam->format);
        return false;  // No need to update offset.
    }
    mosSurface->dwOffset = mosSurface->UPlaneOffset.iSurfaceOffset;
    mosSurface->Format = Format_R8G8UN;
    return false;
}

//*-----------------------------------------------------------------------------
//| Purpose: Setup 2D surface State
//| Returns: Result of the operation
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_Setup2DSurfaceStateBasic(
    PCM_HAL_STATE                      state,
    PCM_HAL_KERNEL_ARG_PARAM           argParam,
    PCM_HAL_INDEX_PARAM                indexParam,
    int32_t                            bindingTable,
    uint32_t                           threadIndex,
    bool                               pixelPitch,
    uint8_t                            *buffer,
    bool                               multipleBinding )
{
    MOS_STATUS                  eStatus;
    RENDERHAL_SURFACE               renderHalSurface;
    PMOS_SURFACE                    surface;
    RENDERHAL_SURFACE_STATE_PARAMS  surfaceParam;
    PRENDERHAL_INTERFACE            renderHal;
    PRENDERHAL_SURFACE_STATE_ENTRY  surfaceEntries[ MHW_MAX_SURFACE_PLANES ];
    uint8_t                     *src;
    uint8_t                     *dst;
    int32_t                     nSurfaceEntries = 0;
    uint32_t                    index;
    uint32_t                    btIndex;
    uint16_t                    memObjCtl;
    uint32_t                    i;
    uint32_t                    tempPlaneIndex = 0;
    uint32_t                    offsetSrc;
    PRENDERHAL_STATE_HEAP       stateHeap;
    PCM_HAL_SURFACE2D_SURFACE_STATE_PARAM surfStateParam = nullptr;
    UNUSED(multipleBinding);

    eStatus = MOS_STATUS_UNKNOWN;
    renderHal = state->renderHal;
    MOS_ZeroMemory(&renderHalSurface, sizeof(renderHalSurface));
    surface   = &renderHalSurface.OsSurface;
    nSurfaceEntries = 0;

    //GT-PIN
    PCM_HAL_TASK_PARAM     taskParam = state->taskParam;

    // Get the Index to 2dsurface array from the kernel data
    CM_ASSERT( argParam->unitSize == sizeof( index ) );
    src = argParam->firstValue + ( threadIndex * argParam->unitSize );
    index = *( ( uint32_t *)src ) & CM_SURFACE_MASK;
    if ( index == CM_NULL_SURFACE )
    {
        if ( buffer )
        {
            dst = buffer + argParam->payloadOffset;
            *( ( uint32_t *)dst ) = CM_NULL_SURFACE_BINDING_INDEX;
        }

        eStatus = MOS_STATUS_SUCCESS;
        goto finish;
    }

    memObjCtl = state->umdSurf2DTable[index].memObjCtl;
    if ( !memObjCtl )
    {
        memObjCtl = CM_DEFAULT_CACHE_TYPE;
    }

    // check to see if the data present for the 2d surface in the array
    if ( index >= state->cmDeviceParam.max2DSurfaceTableSize ||
         Mos_ResourceIsNull( &state->umdSurf2DTable[ index ].osResource ) )
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        CM_ASSERTMESSAGE(
            "Invalid 2D surface array index '%d'", index );
        goto finish;
    }

    // Check to see if surface is already assigned
    unsigned char nBTIRegularSurf, nBTISamplerSurf;
    nBTIRegularSurf = state->bti2DIndexTable[ index ].BTI.regularSurfIndex;
    nBTISamplerSurf = state->bti2DIndexTable[ index ].BTI.samplerSurfIndex;

    if (((!pixelPitch && (nBTIRegularSurf != (unsigned char)CM_INVALID_INDEX)) || (pixelPitch && (nBTISamplerSurf != (unsigned char)CM_INVALID_INDEX))) && argParam->aliasCreated == false )
    {
        if ( pixelPitch )
        {
            btIndex = nBTISamplerSurf;
        }
        else
        {
            btIndex = nBTIRegularSurf;
        }

        stateHeap = renderHal->pStateHeap;

        // Get Offset to Current Binding Table
        uint32_t offsetCurrentBTStart = ( stateHeap->iCurSshBufferIndex * stateHeap->dwSshIntanceSize ) +     // Points to the Base of Current SSH Buffer Instance
                                       ( stateHeap->iBindingTableOffset ) +                             // Moves the pointer to Base of Array of Binding Tables
                                       ( bindingTable * stateHeap->iBindingTableSize );                // Moves the pointer to a Particular Binding Table

        uint32_t *currentBTStart = ( uint32_t *)( stateHeap->pSshBuffer + offsetCurrentBTStart );

        int nEntryIndex = 0;

        if ( pixelPitch )
        {
            nEntryIndex = (int)((uint32_t*)( state->bti2DIndexTable[ index ].BTITableEntry.samplerBtiEntryPosition ) - currentBTStart);
        }
        else
        {
            nEntryIndex = (int)((uint32_t*)( state->bti2DIndexTable[ index ].BTITableEntry.regularBtiEntryPosition ) - currentBTStart);
        }

        if ( ( nEntryIndex < 0 ) || ( nEntryIndex >= renderHal->StateHeapSettings.iSurfacesPerBT ) )
        {
            nSurfaceEntries = state->bti2DIndexTable[ index ].nPlaneNumber;

            btIndex = HalCm_GetFreeBindingIndex( state, indexParam, nSurfaceEntries );

            // Get Offset to Current Binding Table
            uint32_t offsetDst = offsetCurrentBTStart + ( btIndex * sizeof( uint32_t ) );            // Move the pointer to correct entry

            uint32_t *bindingTableEntry = ( uint32_t *)( stateHeap->pSshBuffer + offsetDst );

            if ( pixelPitch )
            {
                MOS_SecureMemcpy( bindingTableEntry, sizeof( uint32_t ) * nSurfaceEntries, state->bti2DIndexTable[ index ].BTITableEntry.samplerBtiEntryPosition, sizeof( uint32_t ) * nSurfaceEntries );
            }
            else
            {
                MOS_SecureMemcpy( bindingTableEntry, sizeof( uint32_t ) * nSurfaceEntries, state->bti2DIndexTable[ index ].BTITableEntry.regularBtiEntryPosition, sizeof( uint32_t ) * nSurfaceEntries );
            }

            // update index to table
            if ( pixelPitch )
            {
                state->bti2DIndexTable[ index ].BTI.samplerSurfIndex = btIndex;
                state->bti2DIndexTable[ index ].BTITableEntry.samplerBtiEntryPosition = bindingTableEntry;
            }
            else
            {
                state->bti2DIndexTable[ index ].BTI.regularSurfIndex = btIndex;
                state->bti2DIndexTable[ index ].BTITableEntry.regularBtiEntryPosition = bindingTableEntry;
            }
        }

        // Update the Batch Buffer
        if ( buffer )
        {
            dst = buffer + argParam->payloadOffset;
            *( ( uint32_t *)dst ) = btIndex;
        }

        eStatus = MOS_STATUS_SUCCESS;
        goto finish;
    }

    CM_CHK_MOSSTATUS_GOTOFINISH( HalCm_GetSurfaceAndRegister( state, &renderHalSurface, CM_ARGUMENT_SURFACE2D, index, pixelPitch ) );

    // Setup 2D surface
    MOS_ZeroMemory(&surfaceParam, sizeof(surfaceParam));
    surfaceParam.Type       = renderHal->SurfaceTypeDefault;
    surfaceParam.Boundary   = RENDERHAL_SS_BOUNDARY_ORIGINAL;
    surfaceParam.bVertStride = 0;
    surfaceParam.bVertStrideOffs = 0;
    if (!pixelPitch) {
        surfaceParam.bWidthInDword_UV = true;
        surfaceParam.bWidthInDword_Y = true;
    }

    surfaceParam.bRenderTarget = isRenderTarget(state, index);
    surfStateParam = &(state->umdSurf2DTable[index].surfaceStateParam[argParam->aliasIndex / state->surfaceArraySize]);
    if (surfStateParam->width)
    {
        surface->dwWidth = surfStateParam->width;
    }
    if (surfStateParam->height)
    {
        surface->dwHeight = surfStateParam->height;
    }
    if (surfStateParam->depth)
    {
        surface->dwDepth = surfStateParam->depth;
    }
    if (surfStateParam->pitch)
    {
        surface->dwPitch= surfStateParam->pitch;
    }
    if (surfStateParam->format)
    {
        UpdateSurfaceAliasPlaneOffset(surfStateParam, surface);
    }
    if (surfStateParam->surfaceXOffset)
    {
        surface->YPlaneOffset.iXOffset = surfStateParam->surfaceXOffset;
        if (surface->Format == Format_NV12)
        {
            surface->UPlaneOffset.iXOffset += surfStateParam->surfaceXOffset;
        }
    }
    if (surfStateParam->surfaceYOffset)
    {
        surface->YPlaneOffset.iYOffset = surfStateParam->surfaceYOffset;
        if (surface->Format == Format_NV12)
        {
            surface->UPlaneOffset.iYOffset += surfStateParam->surfaceYOffset/2;
        }
    }
    if (surfStateParam->memoryObjectControl)
    {
        memObjCtl = surfStateParam->memoryObjectControl;
    }

    if(pixelPitch)
        renderHalSurface.Rotation = state->umdSurf2DTable[index].rotationFlag;

    //Cache configurations
    state->cmHalInterface->HwSetSurfaceMemoryObjectControl(memObjCtl, &surfaceParam);

    // interlace setting
    HalCm_HwSetSurfaceProperty(state,
        state->umdSurf2DTable[index].frameType,
        &surfaceParam);

    CM_CHK_MOSSTATUS_GOTOFINISH(renderHal->pfnSetupSurfaceState(
                  renderHal,
                  &renderHalSurface,
                  &surfaceParam,
                  &nSurfaceEntries,
                  surfaceEntries,
                  nullptr));

    nSurfaceEntries = MOS_MIN( nSurfaceEntries, MHW_MAX_SURFACE_PLANES );

    btIndex = HalCm_GetFreeBindingIndex(state, indexParam, nSurfaceEntries);
    for (i = 0; i < (uint32_t)nSurfaceEntries; i++)
    {
        // Bind the surface State
        CM_CHK_MOSSTATUS_GOTOFINISH(renderHal->pfnBindSurfaceState(
                        renderHal,
                        bindingTable,
                        btIndex + i,
                        surfaceEntries[i]));
        if ((taskParam->surfEntryInfoArrays.kernelNum !=0) &&
            (taskParam->surfEntryInfoArrays.surfEntryInfosArray != nullptr))
        {
            //GT-Pin
            CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_GetSurfaceDetails(
                    state,
                    indexParam,
                    btIndex + i,
                    *surface,
                    0,
                    surfaceEntries[i],
                    tempPlaneIndex,
                    surfaceParam,
                    CM_ARGUMENT_SURFACE2D));
        }
        surfaceEntries[i]->pSurface->dwWidth = state->umdSurf2DTable[index].width;
        surfaceEntries[i]->pSurface->dwHeight = state->umdSurf2DTable[index].height;
    }

    // only update the reuse table for non-aliased surface
    if ( argParam->aliasCreated == false )
    {
        state->bti2DIndexTable[ index ].nPlaneNumber = nSurfaceEntries;
        // Get Offset to Current Binding Table
        stateHeap = renderHal->pStateHeap;
        offsetSrc = ( stateHeap->iCurSshBufferIndex * stateHeap->dwSshIntanceSize ) +     // Points to the Base of Current SSH Buffer Instance
            ( stateHeap->iBindingTableOffset ) +                       // Moves the pointer to Base of Array of Binding Tables
            ( bindingTable * stateHeap->iBindingTableSize ) +         // Moves the pointer to a Particular Binding Table
            ( btIndex * sizeof( uint32_t ) );                             // Move the pointer to correct entry

        if ( pixelPitch )
        {
            state->bti2DIndexTable[ index ].BTI.samplerSurfIndex = btIndex;
            state->bti2DIndexTable[ index ].BTITableEntry.samplerBtiEntryPosition = stateHeap->pSshBuffer + offsetSrc;
        }
        else
        {
            state->bti2DIndexTable[ index ].BTI.regularSurfIndex = btIndex;
            state->bti2DIndexTable[ index ].BTITableEntry.regularBtiEntryPosition = stateHeap->pSshBuffer + offsetSrc;
        }
    }

    // Update the Batch Buffer
    if (buffer)
    {
        dst = buffer + argParam->payloadOffset;
        *((uint32_t*)dst) = btIndex;
    }

    // reset surface height and width
    surface->dwWidth = state->umdSurf2DTable[index].width;
    surface->dwHeight = state->umdSurf2DTable[index].height;

    eStatus = MOS_STATUS_SUCCESS;

finish:
    return eStatus;
}


MOS_STATUS HalCm_Setup2DSurfaceState(
    PCM_HAL_STATE              state,
    PCM_HAL_KERNEL_ARG_PARAM   argParam,
    PCM_HAL_INDEX_PARAM        indexParam,
    int32_t                    bindingTable,
    uint32_t                   threadIndex,
    uint8_t                    *buffer)
{
    MOS_STATUS                 eStatus;

    if (state->cmHalInterface->GetDecompressFlag())
    {
        HalCm_DecompressSurface(state, argParam, threadIndex);
    }

    //Binding surface based at the unit of dword
    CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_Setup2DSurfaceStateBasic(
                    state, argParam, indexParam, bindingTable, threadIndex, false, buffer, false));
    eStatus = MOS_STATUS_SUCCESS;

finish:
    return eStatus;
}

MOS_STATUS HalCm_Setup2DSurfaceSamplerState(
    PCM_HAL_STATE              state,
    PCM_HAL_KERNEL_ARG_PARAM   argParam,
    PCM_HAL_INDEX_PARAM        indexParam,
    int32_t                    bindingTable,
    uint32_t                   threadIndex,
    uint8_t                    *buffer)
{
    MOS_STATUS                 eStatus;

    //Binding surface based at the unit of dword
    CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_Setup2DSurfaceStateBasic(
        state, argParam, indexParam, bindingTable, threadIndex, true, buffer, false));
    eStatus = MOS_STATUS_SUCCESS;

finish:
    return eStatus;
}

//*-----------------------------------------------------------------------------
//| Purpose: Setup 2D surface State
//| Returns: Result of the operation
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_Setup2DSurfaceUPStateBasic(
    PCM_HAL_STATE               state,
    PCM_HAL_KERNEL_ARG_PARAM    argParam,
    PCM_HAL_INDEX_PARAM         indexParam,
    int32_t                     bindingTable,
    uint32_t                    threadIndex,
    uint8_t                     *buffer,
    bool                        pixelPitch)
{
    MOS_STATUS                  eStatus;
    RENDERHAL_SURFACE               surface;
    RENDERHAL_SURFACE_STATE_PARAMS  surfaceParam;
    PRENDERHAL_INTERFACE            renderHal;
    PRENDERHAL_SURFACE_STATE_ENTRY  surfaceEntries[MHW_MAX_SURFACE_PLANES];
    uint8_t                     *src;
    uint8_t                     *dst;
    int32_t                     nSurfaceEntries;
    uint32_t                    index;
    uint32_t                    btIndex;
    uint16_t                    memObjCtl;
    uint32_t                    i;
    uint32_t                    offsetSrc;
    PRENDERHAL_STATE_HEAP       stateHeap;

    eStatus              = MOS_STATUS_UNKNOWN;
    renderHal    = state->renderHal;
    //GT-PIN
    PCM_HAL_TASK_PARAM     taskParam = state->taskParam;

    // Get the Index to sampler array from the kernel data
    CM_ASSERT(argParam->unitSize == sizeof(index));
    src      = argParam->firstValue + (threadIndex * argParam->unitSize);
    index    = *((uint32_t*)src) & CM_SURFACE_MASK;
    if (index == CM_NULL_SURFACE)
    {
        if (buffer)
        {
            dst = buffer + argParam->payloadOffset;
            *((uint32_t*)dst) = CM_NULL_SURFACE_BINDING_INDEX;
        }

        eStatus = MOS_STATUS_SUCCESS;
        goto finish;
    }

    memObjCtl = state->surf2DUPTable[index].memObjCtl;
    if (!memObjCtl)
    {
        memObjCtl = CM_DEFAULT_CACHE_TYPE;
    }

    // check to see if the data present for the sampler in the array
    if (index >= state->cmDeviceParam.max2DSurfaceUPTableSize ||
        (state->surf2DUPTable[index].width == 0))
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        CM_ASSERTMESSAGE(
            "Invalid 2D SurfaceUP array index '%d'", index);
        goto finish;
    }

    // Check to see if surface is already assigned
    if ( pixelPitch )
    {
        btIndex = state->bti2DUPIndexTable[ index ].BTI.samplerSurfIndex;
    }
    else
    {
        btIndex = state->bti2DUPIndexTable[ index ].BTI.regularSurfIndex;
    }

    if ( btIndex == ( unsigned char )CM_INVALID_INDEX )
    {
        uint32_t tempPlaneIndex = 0;

        // Get Details of 2DUP surface and fill the surface
        CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_GetSurfaceAndRegister(state, &surface, CM_ARGUMENT_SURFACE2D_UP, index, pixelPitch));

        // Setup 2D surface
        MOS_ZeroMemory(&surfaceParam, sizeof(surfaceParam));
        surfaceParam.Type       = renderHal->SurfaceTypeDefault;
        surfaceParam.Boundary   = RENDERHAL_SS_BOUNDARY_ORIGINAL;

        if (!pixelPitch) {
            surfaceParam.bWidthInDword_UV = true;
            surfaceParam.bWidthInDword_Y = true;
        }

        surfaceParam.bRenderTarget = true;

        //Cache configurations
        state->cmHalInterface->HwSetSurfaceMemoryObjectControl(memObjCtl, &surfaceParam);

        // interlace setting
        HalCm_HwSetSurfaceProperty(state,
            state->umdSurf2DTable[index].frameType,
            &surfaceParam);

        CM_CHK_MOSSTATUS_GOTOFINISH(renderHal->pfnSetupSurfaceState(
                    renderHal,
                    &surface,
                    &surfaceParam,
                    &nSurfaceEntries,
                    surfaceEntries,
                    nullptr));

        //GT-PIN
        btIndex = HalCm_GetFreeBindingIndex(state, indexParam, nSurfaceEntries);
        for (i = 0; i < (uint32_t)nSurfaceEntries; i++)
        {
            // Bind the surface State
            CM_CHK_MOSSTATUS_GOTOFINISH(renderHal->pfnBindSurfaceState(
                        renderHal,
                        bindingTable,
                        btIndex + i,
                        surfaceEntries[i]));
            //GT-Pin
            if ((taskParam->surfEntryInfoArrays.kernelNum != 0) &&
                (taskParam->surfEntryInfoArrays.surfEntryInfosArray != nullptr))
            {
                CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_GetSurfaceDetails(
                         state,
                         indexParam,
                         btIndex + i,
                         surface.OsSurface,
                         0,
                         surfaceEntries[i],
                         tempPlaneIndex,
                         surfaceParam,
                         CM_ARGUMENT_SURFACE2D_UP));
            }
        }
        state->bti2DUPIndexTable[ index ].nPlaneNumber = nSurfaceEntries;

        stateHeap = renderHal->pStateHeap;
        offsetSrc = ( stateHeap->iCurSshBufferIndex * stateHeap->dwSshIntanceSize ) +     // Points to the Base of Current SSH Buffer Instance
                            ( stateHeap->iBindingTableOffset ) +                       // Moves the pointer to Base of Array of Binding Tables
                            ( bindingTable * stateHeap->iBindingTableSize ) +         // Moves the pointer to a Particular Binding Table
                            ( btIndex * sizeof( uint32_t ) );                             // Move the pointer to correct entry

        if ( pixelPitch )
        {
            state->bti2DUPIndexTable[ index ].BTI.samplerSurfIndex = btIndex;
            state->bti2DUPIndexTable[ index ].BTITableEntry.samplerBtiEntryPosition = stateHeap->pSshBuffer + offsetSrc;
        }
        else
        {
            state->bti2DUPIndexTable[ index ].BTI.regularSurfIndex = btIndex;
            state->bti2DUPIndexTable[ index ].BTITableEntry.regularBtiEntryPosition = stateHeap->pSshBuffer + offsetSrc;
        }
    }
    else
    {
        stateHeap = renderHal->pStateHeap;

        // Get Offset to Current Binding Table
        uint32_t offsetCurrentBTStart = ( stateHeap->iCurSshBufferIndex * stateHeap->dwSshIntanceSize ) +     // Points to the Base of Current SSH Buffer Instance
                                       ( stateHeap->iBindingTableOffset ) +                             // Moves the pointer to Base of Array of Binding Tables
                                       ( bindingTable * stateHeap->iBindingTableSize );                // Moves the pointer to a Particular Binding Table

        uint32_t *currentBTStart = ( uint32_t *)( stateHeap->pSshBuffer + offsetCurrentBTStart );

        int nEntryIndex = 0;

        if ( pixelPitch )
        {
            nEntryIndex = (int) ((uint32_t*)( state->bti2DUPIndexTable[ index ].BTITableEntry.samplerBtiEntryPosition ) - currentBTStart);
        }
        else
        {
            nEntryIndex = (int) ((uint32_t*)( state->bti2DUPIndexTable[ index ].BTITableEntry.regularBtiEntryPosition ) - currentBTStart);
        }

        if ( ( nEntryIndex < 0 ) || ( nEntryIndex >= renderHal->StateHeapSettings.iSurfacesPerBT ) )
        {
            uint32_t tmpSurfaceEntries = state->bti2DUPIndexTable[ index ].nPlaneNumber;

            btIndex = HalCm_GetFreeBindingIndex( state, indexParam, tmpSurfaceEntries );

            // Get Offset to Current Binding Table
            uint32_t offsetDst = offsetCurrentBTStart + ( btIndex * sizeof( uint32_t ) );                             // Move the pointer to correct entry

            uint32_t *bindingTableEntry = ( uint32_t *)( stateHeap->pSshBuffer + offsetDst );
            if ( pixelPitch )
            {
                MOS_SecureMemcpy( bindingTableEntry, sizeof( uint32_t ) * tmpSurfaceEntries, state->bti2DUPIndexTable[ index ].BTITableEntry.samplerBtiEntryPosition, sizeof( uint32_t ) * tmpSurfaceEntries );
            }
            else
            {
                MOS_SecureMemcpy( bindingTableEntry, sizeof( uint32_t ) * tmpSurfaceEntries, state->bti2DUPIndexTable[ index ].BTITableEntry.regularBtiEntryPosition, sizeof( uint32_t ) * tmpSurfaceEntries );
            }

            // update index to table
            if ( pixelPitch )
            {
                state->bti2DUPIndexTable[ index ].BTI.samplerSurfIndex = btIndex;
                state->bti2DUPIndexTable[ index ].BTITableEntry.samplerBtiEntryPosition = bindingTableEntry;
            }
            else
            {
                state->bti2DUPIndexTable[ index ].BTI.regularSurfIndex = btIndex;
                state->bti2DUPIndexTable[ index ].BTITableEntry.regularBtiEntryPosition = bindingTableEntry;
            }
        }
    }

    // Update the Batch Buffer
    if (buffer)
    {
        dst = buffer + argParam->payloadOffset;
        *((uint32_t*)dst) = btIndex;
    }

    eStatus = MOS_STATUS_SUCCESS;

finish:
    return eStatus;
}

MOS_STATUS HalCm_Setup2DSurfaceUPState(
    PCM_HAL_STATE               state,
    PCM_HAL_KERNEL_ARG_PARAM    argParam,
    PCM_HAL_INDEX_PARAM         indexParam,
    int32_t                     bindingTable,
    uint32_t                    threadIndex,
    uint8_t                     *buffer)
{
    MOS_STATUS                 eStatus;

    //Binding surface based at the unit of dword
    CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_Setup2DSurfaceUPStateBasic(
                    state, argParam, indexParam, bindingTable, threadIndex, buffer, false));
    eStatus = MOS_STATUS_SUCCESS;

finish:
    return eStatus;
}

MOS_STATUS HalCm_Setup2DSurfaceUPSamplerState(
    PCM_HAL_STATE               state,
    PCM_HAL_KERNEL_ARG_PARAM    argParam,
    PCM_HAL_INDEX_PARAM         indexParam,
    int32_t                     bindingTable,
    uint32_t                    threadIndex,
    uint8_t                     *buffer)
{
    MOS_STATUS                 eStatus;

    //Binding surface based at the unit of pixel
    CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_Setup2DSurfaceUPStateBasic(
                    state, argParam, indexParam, bindingTable, threadIndex, buffer, true));
    eStatus = MOS_STATUS_SUCCESS;

finish:
    return eStatus;
}

MOS_STATUS HalCm_SetupSpecificVmeSurfaceState(
    PCM_HAL_STATE                     state,
    PCM_HAL_INDEX_PARAM               indexParam,
    int32_t                           bindingTable,
    uint32_t                          surfIndex,
    uint32_t                          btIndex,
    uint16_t                          memObjCtl,
    uint32_t                          surfaceStateWidth,
    uint32_t                          surfaceStateHeight)
{
    MOS_STATUS                      eStatus;
    RENDERHAL_SURFACE               surface;
    int32_t                         nSurfaceEntries = 0;
    RENDERHAL_SURFACE_STATE_PARAMS  surfaceParam;
    PRENDERHAL_INTERFACE            renderHal;
    PRENDERHAL_SURFACE_STATE_ENTRY  surfaceEntries[MHW_MAX_SURFACE_PLANES];
    uint32_t                        tempPlaneIndex = 0;
    PMOS_SURFACE                    mosSurface = nullptr;

    eStatus               = MOS_STATUS_UNKNOWN;
    renderHal     = state->renderHal;
    nSurfaceEntries  = 0;

    PCM_HAL_TASK_PARAM taskParam = state->taskParam;

    // Get Details of VME surface and fill the surface
    CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_GetSurfaceAndRegister(state, &surface, CM_ARGUMENT_VME_STATE, surfIndex, 0));

    // Setup 2D surface
    MOS_ZeroMemory(&surfaceParam, sizeof(surfaceParam));
    surfaceParam.Type              = renderHal->SurfaceTypeAdvanced;
    surfaceParam.bRenderTarget     = true;
    surfaceParam.bWidthInDword_Y   = false;
    surfaceParam.bWidthInDword_UV  = false;
    surfaceParam.Boundary          = RENDERHAL_SS_BOUNDARY_ORIGINAL;
    surfaceParam.bVmeUse           = true;

    // Overwrite the width and height if specified
    if (surfaceStateWidth && surfaceStateHeight)
    {
        mosSurface = &surface.OsSurface;
        if (surfaceStateWidth > mosSurface->dwWidth || surfaceStateHeight > mosSurface->dwHeight)
        {
            CM_ASSERTMESSAGE("Error: VME surface state's resolution is larger than the original surface.");
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            goto finish;
        }
        mosSurface->dwWidth = surfaceStateWidth;
        mosSurface->dwHeight = surfaceStateHeight;
    }

    //Cache configurations
    state->cmHalInterface->HwSetSurfaceMemoryObjectControl(memObjCtl, &surfaceParam);
    CM_CHK_MOSSTATUS_GOTOFINISH(renderHal->pfnSetupSurfaceState(
                        renderHal,
                        &surface,
                        &surfaceParam,
                        &nSurfaceEntries,
                        surfaceEntries,
                        nullptr));

    CM_ASSERT(nSurfaceEntries == 1);

    {
        // Bind the surface State
        CM_CHK_MOSSTATUS_GOTOFINISH(renderHal->pfnBindSurfaceState(
                            renderHal,
                            bindingTable,
                            btIndex,
                            surfaceEntries[0]));

        if ((taskParam->surfEntryInfoArrays.kernelNum != 0) &&
            (taskParam->surfEntryInfoArrays.surfEntryInfosArray != nullptr))
        {
            CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_GetSurfaceDetails(
                    state,
                    indexParam,
                    btIndex,
                    surface.OsSurface,
                    0,
                    surfaceEntries[0],
                    tempPlaneIndex,
                    surfaceParam,
                    CM_ARGUMENT_SURFACE2D));
        }
    }
    state->bti2DIndexTable[ surfIndex ].BTI.vmeSurfIndex = btIndex;

    eStatus = MOS_STATUS_SUCCESS;

finish:
    return eStatus;

}

//*-----------------------------------------------------------------------------
//| Purpose: Setup VME surface State
//| Returns: Result of the operation
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_SetupVmeSurfaceState(
    PCM_HAL_STATE               state,
    PCM_HAL_KERNEL_ARG_PARAM    argParam,
    PCM_HAL_INDEX_PARAM         indexParam,
    int32_t                     bindingTable,
    uint32_t                    threadIndex,
    uint8_t                     *buffer)
{
    MOS_STATUS                  eStatus;
    PRENDERHAL_INTERFACE        renderHal;
    PCM_HAL_VME_ARG_VALUE       vmeSrc;
    uint8_t                     *dst;
    uint32_t                    index[CM_MAX_VME_BINDING_INDEX_1];
    uint16_t                    memObjCtl[CM_MAX_VME_BINDING_INDEX_1];
    uint32_t                    fwSurfCount = 0;
    uint32_t                    bwSurfCount = 0;
    bool                        alreadyBind = true;
    uint32_t                    surfPairNum;
    uint32_t                    idx;
    uint32_t                    curBTIndex;
    uint32_t                    btIndex;
    uint32_t                    surfaceStateWidth = 0;
    uint32_t                    surfaceStateHeight = 0;
    uint32_t                    *fPtr = nullptr;
    uint32_t                    *bPtr = nullptr;
    uint32_t                    *refSurfaces = nullptr;

    eStatus              = MOS_STATUS_UNKNOWN;
    renderHal    = state->renderHal;
    btIndex        = 0;

    MOS_ZeroMemory(memObjCtl, CM_MAX_VME_BINDING_INDEX_1*sizeof(uint16_t));
    MOS_ZeroMemory(index, CM_MAX_VME_BINDING_INDEX_1*sizeof(uint32_t));

    CM_ASSERT(argParam->unitSize <= sizeof(uint32_t)*(CM_MAX_VME_BINDING_INDEX_1 + 2));
    CM_ASSERT(threadIndex == 0); // VME surface is not allowed in thread arg

    vmeSrc = (PCM_HAL_VME_ARG_VALUE)argParam->firstValue;
    fwSurfCount = vmeSrc->fwRefNum;
    bwSurfCount = vmeSrc->bwRefNum;
    refSurfaces = findRefInVmeArg(vmeSrc);

    index[0] = vmeSrc->curSurface & CM_SURFACE_MASK;
    // check to see if index[0] is valid
    if (index[0] == CM_NULL_SURFACE)
    {
        if (buffer)
        {
            dst = buffer + argParam->payloadOffset;
            *((uint32_t*)dst) = CM_NULL_SURFACE_BINDING_INDEX;
        }

        eStatus = MOS_STATUS_SUCCESS;
        goto finish;
    }

    if (index[0] >= state->cmDeviceParam.max2DSurfaceTableSize ||
        Mos_ResourceIsNull(&state->umdSurf2DTable[index[0]].osResource))
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        CM_ASSERTMESSAGE(
            "Invalid 2D surface array index '%d'", index[0]);
        goto finish;
    }

    memObjCtl[0] = state->umdSurf2DTable[index[0]].memObjCtl;
    if (!memObjCtl[0])
    {
        memObjCtl[0] = CM_DEFAULT_CACHE_TYPE;
    }
    for (idx = 0; idx < (vmeSrc->fwRefNum + vmeSrc->bwRefNum); idx++)
    {
        index[idx + 1] = refSurfaces[idx] & CM_SURFACE_MASK;
        memObjCtl[idx + 1] = state->umdSurf2DTable[index[idx + 1]].memObjCtl;
        if (!memObjCtl[idx + 1])
        {
            memObjCtl[idx + 1] = CM_DEFAULT_CACHE_TYPE;
        }
    }

    surfaceStateWidth = vmeSrc->surfStateParam.surfaceStateWidth;
    surfaceStateHeight = vmeSrc->surfStateParam.surfaceStateHeight;

    fPtr = index + 1;
    bPtr = index + 1 + fwSurfCount;

    //Max surface pair number
    surfPairNum = fwSurfCount > bwSurfCount ? fwSurfCount : bwSurfCount;

    btIndex = curBTIndex = HalCm_GetFreeBindingIndex(state, indexParam, surfPairNum*2 + 1);

    HalCm_SetupSpecificVmeSurfaceState(state, indexParam, bindingTable, index[0], curBTIndex, memObjCtl[0], surfaceStateWidth, surfaceStateHeight);
    curBTIndex++;

    //Setup surface states interleavely for backward and forward surfaces pairs.
    for (idx = 0; idx < surfPairNum; idx++)
    {
        if (idx < fwSurfCount)
        {
            HalCm_SetupSpecificVmeSurfaceState(state, indexParam, bindingTable, fPtr[idx], curBTIndex, memObjCtl[idx + 1], surfaceStateWidth, surfaceStateHeight);
        }
        curBTIndex++;

        if (idx < bwSurfCount)
        {
            HalCm_SetupSpecificVmeSurfaceState(state, indexParam, bindingTable, bPtr[idx], curBTIndex, memObjCtl[idx+ 1 + fwSurfCount], surfaceStateWidth, surfaceStateHeight);
        }
        curBTIndex++;
    }

    // Update the Batch Buffer
    if (buffer)
    {
        dst = buffer + argParam->payloadOffset;
        *((uint32_t*)dst) = btIndex;
    }

    eStatus = MOS_STATUS_SUCCESS;

finish:
    return eStatus;
}

static bool
UpdateMosSurfaceFromAliasState(CM_HAL_STATE *state,
                               CM_HAL_KERNEL_ARG_PARAM *argParam,
                               uint32_t surface_index,
                               MOS_SURFACE *surface)
{
    uint32_t surface_state_index = argParam->aliasIndex/state->surfaceArraySize;
    const CM_HAL_SURFACE2D_SURFACE_STATE_PARAM &surface_state_param
            = state->umdSurf2DTable[surface_index].surfaceStateParam[
                surface_state_index];
    if (surface_state_param.width)
    {
        surface->dwWidth = surface_state_param.width;
    }
    if (surface_state_param.height)
    {
        surface->dwHeight = surface_state_param.height;
    }
    if (surface_state_param.depth)
    {
        surface->dwDepth = surface_state_param.depth;
    }
    if (surface_state_param.pitch)
    {
        surface->dwPitch= surface_state_param.pitch;
    }
    if (surface_state_param.format)
    {
        surface->Format
                = static_cast<MOS_FORMAT>(surface_state_param.format);
    }
    if (surface_state_param.surfaceXOffset)
    {
        surface->YPlaneOffset.iXOffset = surface_state_param.surfaceXOffset;
    }
    if (surface_state_param.surfaceYOffset)
    {
        surface->YPlaneOffset.iYOffset = surface_state_param.surfaceYOffset;
    }
    if (surface_state_param.surfaceOffset)
    {
        surface->dwOffset = surface_state_param.surfaceOffset;
    }

    return true;
}

//*-----------------------------------------------------------------------------
//| Purpose: Setup VME surface State
//| Returns: Result of the operation
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_SetupSampler8x8SurfaceState(
    PCM_HAL_STATE               state,
    PCM_HAL_KERNEL_ARG_PARAM    argParam,
    PCM_HAL_INDEX_PARAM         indexParam,
    int32_t                     bindingTable,
    uint32_t                    threadIndex,
    uint8_t                     *buffer)
{
    MOS_STATUS                  eStatus;
    RENDERHAL_SURFACE               surface;
    RENDERHAL_SURFACE_STATE_PARAMS  surfaceParam;
    PRENDERHAL_INTERFACE            renderHal;
    PRENDERHAL_SURFACE_STATE_ENTRY  surfaceEntries[MHW_MAX_SURFACE_PLANES];
    uint8_t                     *src;
    uint8_t                     *dst;
    int32_t                     nSurfaceEntries;
    uint32_t                    index;
    uint16_t                    memObjCtl;
    int32_t                     i;
    uint32_t                    btIndex;
    uint32_t                    tempPlaneIndex = 0;
    uint32_t                    offsetSrc;
    PRENDERHAL_STATE_HEAP       stateHeap;

    eStatus               = MOS_STATUS_UNKNOWN;
    renderHal     = state->renderHal;

    PCM_HAL_TASK_PARAM          taskParam    = state->taskParam;

    nSurfaceEntries = 0;

    CM_ASSERT(argParam->unitSize == sizeof(uint32_t));

    src      = argParam->firstValue + (threadIndex * argParam->unitSize);
    index     = *((uint32_t*)src) & CM_SURFACE_MASK;
    if (index == CM_NULL_SURFACE)
    {
        if (buffer)
        {
            dst = buffer + argParam->payloadOffset;
            *((uint32_t*)dst) = CM_NULL_SURFACE_BINDING_INDEX;
        }

        eStatus = MOS_STATUS_SUCCESS;
        goto finish;
    }

    memObjCtl = state->umdSurf2DTable[index].memObjCtl;
    if (!memObjCtl)
    {
        memObjCtl = CM_DEFAULT_CACHE_TYPE;
    }

    // check to see if index is valid
    if (index >= state->cmDeviceParam.max2DSurfaceTableSize ||
       Mos_ResourceIsNull(&state->umdSurf2DTable[index].osResource))
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        CM_ASSERTMESSAGE(
            "Invalid 2D surface array index '%d'", index);
        goto finish;
    }

    renderHal->bEnableP010SinglePass = state->cmHalInterface->IsP010SinglePassSupported();

    btIndex = state->bti2DIndexTable[ index ].BTI.sampler8x8SurfIndex;
    if (btIndex == ( unsigned char )CM_INVALID_INDEX || argParam->aliasCreated)
    {
        // Get Details of Sampler8x8 surface and fill the surface
        CM_CHK_MOSSTATUS_GOTOFINISH( HalCm_GetSurfaceAndRegister( state, &surface, argParam->kind, index, 0 ) );

        // Setup surface
        MOS_ZeroMemory( &surfaceParam, sizeof( surfaceParam ) );
        surfaceParam.Type = renderHal->SurfaceTypeAdvanced;
        surfaceParam.bRenderTarget = true;
        surfaceParam.bWidthInDword_Y = false;
        surfaceParam.bWidthInDword_UV = false;
        surfaceParam.Boundary = RENDERHAL_SS_BOUNDARY_ORIGINAL;
        surfaceParam.bVASurface = ( argParam->kind == CM_ARGUMENT_SURFACE_SAMPLER8X8_VA ) ? 1 : 0;
        surfaceParam.AddressControl = argParam->nCustomValue;

        UpdateMosSurfaceFromAliasState(state, argParam, index,
                                       &surface.OsSurface);

        //Set memory object control
        state->cmHalInterface->HwSetSurfaceMemoryObjectControl(memObjCtl, &surfaceParam);

        surface.Rotation = state->umdSurf2DTable[index].rotationFlag;
        surface.ChromaSiting = state->umdSurf2DTable[index].chromaSiting;
        surface.ScalingMode = RENDERHAL_SCALING_AVS;
        nSurfaceEntries = 0;

        // interlace setting
        HalCm_HwSetSurfaceProperty(state,
            state->umdSurf2DTable[index].frameType,
            &surfaceParam);

        CM_CHK_MOSSTATUS_GOTOFINISH( renderHal->pfnSetupSurfaceState(
            renderHal,
            &surface,
            &surfaceParam,
            &nSurfaceEntries,
            surfaceEntries,
            nullptr ) );

        CM_ASSERT( nSurfaceEntries == 1 );

        btIndex = HalCm_GetFreeBindingIndex( state, indexParam, nSurfaceEntries );

        for ( i = 0; i < nSurfaceEntries; i++ )
        {
            // Bind the surface State
            CM_CHK_MOSSTATUS_GOTOFINISH( renderHal->pfnBindSurfaceState(
                renderHal,
                bindingTable,
                btIndex + i,
                surfaceEntries[ i ] ) );

            if ( ( taskParam->surfEntryInfoArrays.kernelNum != 0 ) &&
                 ( taskParam->surfEntryInfoArrays.surfEntryInfosArray != nullptr ) )
            {
                CM_CHK_MOSSTATUS_GOTOFINISH( HalCm_GetSurfaceDetails(
                    state,
                    indexParam,
                    btIndex + i,
                    surface.OsSurface,
                    0,
                    surfaceEntries[ i ],
                    tempPlaneIndex,
                    surfaceParam,
                    CM_ARGUMENT_SURFACE2D ) );
            }
        }

        stateHeap = renderHal->pStateHeap;
        offsetSrc = ( stateHeap->iCurSshBufferIndex * stateHeap->dwSshIntanceSize ) +     // Points to the Base of Current SSH Buffer Instance
                      ( stateHeap->iBindingTableOffset ) +                       // Moves the pointer to Base of Array of Binding Tables
                      ( bindingTable * stateHeap->iBindingTableSize ) +         // Moves the pointer to a Particular Binding Table
                      ( btIndex * sizeof( uint32_t ) );                             // Move the pointer to correct entry

        state->bti2DIndexTable[ index ].nPlaneNumber = nSurfaceEntries;
        state->bti2DIndexTable[ index ].BTITableEntry.sampler8x8BtiEntryPosition = stateHeap->pSshBuffer + offsetSrc;
        state->bti2DIndexTable[ index ].BTI.sampler8x8SurfIndex = btIndex;
    }
    else
    {
        stateHeap = renderHal->pStateHeap;

        // Get Offset to Current Binding Table
        uint32_t offsetCurrentBTStart = ( stateHeap->iCurSshBufferIndex * stateHeap->dwSshIntanceSize ) +     // Points to the Base of Current SSH Buffer Instance
                                       ( stateHeap->iBindingTableOffset ) +                             // Moves the pointer to Base of Array of Binding Tables
                                       ( bindingTable * stateHeap->iBindingTableSize );                // Moves the pointer to a Particular Binding Table

        uint32_t *currentBTStart = ( uint32_t *)( stateHeap->pSshBuffer + offsetCurrentBTStart );

        int nEntryIndex = 0;

        nEntryIndex = ( int )( ( uint32_t *)( state->bti2DIndexTable[ index ].BTITableEntry.sampler8x8BtiEntryPosition ) - currentBTStart );

        if ( ( nEntryIndex < 0 ) || ( nEntryIndex >= renderHal->StateHeapSettings.iSurfacesPerBT ) )
        {
            uint32_t tmpSurfaceEntries = state->bti2DIndexTable[ index ].nPlaneNumber;

            btIndex = HalCm_GetFreeBindingIndex( state, indexParam, tmpSurfaceEntries );

            // Get Offset to Current Binding Table
            uint32_t offsetDst = offsetCurrentBTStart + ( btIndex * sizeof( uint32_t ) );                             // Move the pointer to correct entry

            uint32_t *bindingTableEntry = ( uint32_t *)( stateHeap->pSshBuffer + offsetDst );
            MOS_SecureMemcpy( bindingTableEntry, sizeof( uint32_t ) * tmpSurfaceEntries, state->bti2DIndexTable[ index ].BTITableEntry.sampler8x8BtiEntryPosition, sizeof( uint32_t ) * tmpSurfaceEntries );

            // update index to table
            state->bti2DIndexTable[ index ].BTI.sampler8x8SurfIndex = btIndex;
            state->bti2DIndexTable[ index ].BTITableEntry.sampler8x8BtiEntryPosition = bindingTableEntry;
        }
    }
    // Update the Batch Buffer
    if ( buffer )
    {
        dst = buffer + argParam->payloadOffset;
        *( ( uint32_t *)dst ) = state->bti2DIndexTable[ index ].BTI.sampler8x8SurfIndex;
    }

    eStatus = MOS_STATUS_SUCCESS;

finish:
    renderHal->bEnableP010SinglePass = false;
    return eStatus;
}

//*-----------------------------------------------------------------------------
//| Purpose: Setup State Buffer surface State
//| Returns: Result of the operation
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_SetupStateBufferSurfaceState(
    PCM_HAL_STATE               state,
    PCM_HAL_KERNEL_ARG_PARAM    argParam,
    PCM_HAL_INDEX_PARAM         indexParam,
    int32_t                     bindingTable,
    uint32_t                    threadIndex,
    uint8_t                     *buffer )
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    PRENDERHAL_INTERFACE            renderHal;
    RENDERHAL_SURFACE_STATE_PARAMS  surfaceParam;
    RENDERHAL_SURFACE               renderhalSurface;
    PRENDERHAL_SURFACE_STATE_ENTRY  surfaceEntry;
    uint32_t                        btIndex;
    CM_SURFACE_BTI_INFO             surfBTIInfo;
    uint16_t                        memObjCtl;

    state->cmHalInterface->GetHwSurfaceBTIInfo( &surfBTIInfo );
    uint32_t surfIndex = reinterpret_cast< uint32_t *>( argParam->firstValue )[ 0 ];

    surfIndex = surfIndex & CM_SURFACE_MASK;
    memObjCtl = state->bufferTable[ surfIndex ].memObjCtl;

    btIndex = HalCm_GetFreeBindingIndex( state, indexParam, 1 );

    renderHal = state->renderHal;
    MOS_ZeroMemory( &renderhalSurface, sizeof( renderhalSurface ) );

    // Get Details of Sampler8x8 surface and fill the surface
    CM_CHK_MOSSTATUS_GOTOFINISH( HalCm_GetSurfaceAndRegister( state, &renderhalSurface, argParam->kind, surfIndex, 0 ) );

    MOS_ZeroMemory( &surfaceParam, sizeof( surfaceParam ) );

    // Set the bRenderTarget by default
    surfaceParam.bRenderTarget = true;

    //Cache configurations default
    state->cmHalInterface->HwSetSurfaceMemoryObjectControl( memObjCtl, &surfaceParam );

    // Setup Buffer surface
    CM_CHK_MOSSTATUS_GOTOFINISH( renderHal->pfnSetupBufferSurfaceState(
        renderHal,
        &renderhalSurface,
        &surfaceParam,
        &surfaceEntry ) );

    // Bind the surface State
    CM_ASSERT( ( ( int32_t )btIndex ) < renderHal->StateHeapSettings.iSurfacesPerBT + surfBTIInfo.normalSurfaceStart );
    CM_CHK_MOSSTATUS_GOTOFINISH( renderHal->pfnBindSurfaceState(
        renderHal,
        bindingTable,
        btIndex,
        surfaceEntry ) );

    if ( buffer )
    {
        *( ( uint32_t *)( buffer + argParam->payloadOffset ) ) = btIndex;
    }

finish:
    return eStatus;
}

//------------------------------------------------------------------------------
//| Purpose: Get usr defined threadcount / threadgroup
//| Returns:    Result of the operation
//------------------------------------------------------------------------------
MOS_STATUS HalCm_GetMaxThreadCountPerThreadGroup(
    PCM_HAL_STATE                   state,                     // [in] Pointer to CM State
    uint32_t                        *threadsPerThreadGroup)     // [out] Pointer to threadsPerThreadGroup
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    CM_PLATFORM_INFO      platformInfo;
    MOS_ZeroMemory(&platformInfo, sizeof(CM_PLATFORM_INFO));
    CM_CHK_MOSSTATUS_GOTOFINISH( state->pfnGetPlatformInfo( state, &platformInfo, false) );

    if (platformInfo.numMaxEUsPerPool)
    {
        *threadsPerThreadGroup = (platformInfo.numHWThreadsPerEU) * (platformInfo.numMaxEUsPerPool);
    }
    else
    {
        *threadsPerThreadGroup = (platformInfo.numHWThreadsPerEU) * (platformInfo.numEUsPerSubSlice);
    }

finish:
    return eStatus;
}

//*-----------------------------------------------------------------------------
//| Purpose:  Decodes hints to get number and size of kernel groups
//| Returns:  Result of the operation
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_GetNumKernelsPerGroup(
    uint8_t     hintsBits,
    uint32_t    numKernels,
    uint32_t    *numKernelsPerGroup,
    uint32_t    *numKernelGroups,
    uint32_t    *remapKernelToGroup,
    uint32_t    *remapGroupToKernel
    )
{
    MOS_STATUS  eStatus   = MOS_STATUS_SUCCESS;
    uint32_t currGrp = 0;
    uint32_t i       = 0;

    // first group at least has one kernel
    numKernelsPerGroup[currGrp]++;
    remapGroupToKernel[currGrp] = 0;

    for( i = 0; i < numKernels - 1; ++i )
    {
        if( (hintsBits & CM_HINTS_LEASTBIT_MASK) == CM_HINTS_LEASTBIT_MASK )
        {
            currGrp++;
            *numKernelGroups = *numKernelGroups + 1;

            remapGroupToKernel[currGrp] = i + 1;
        }
        numKernelsPerGroup[currGrp]++;
        hintsBits >>= 1;
        remapKernelToGroup[i+1] = currGrp;
    }

    return eStatus;
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
    bool                           noDependencyCase)
{
    MOS_STATUS eStatus             = MOS_STATUS_SUCCESS;
    uint32_t numThreadsOnSides = 0;
    uint32_t numMaxRepeat      = 0;
    uint32_t numSteps          = 0;

    switch( pattern )
    {
        case CM_NONE_DEPENDENCY:
            if (noDependencyCase)
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
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            CM_ASSERTMESSAGE("Unsupported dependency pattern for EnqueueWithHints");
            goto finish;
    }

    graphInfo->maxParallelism = maximum;
    graphInfo->numMaxRepeat = numMaxRepeat;
    graphInfo->numSteps = numSteps;

finish:
    return eStatus;
}

//*-----------------------------------------------------------------------------
//| Purpose:  Sets dispatch pattern based on max parallelism for media objects
//| Returns:  Result of the operation
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_SetDispatchPattern(
    CM_HAL_PARALLELISM_GRAPH_INFO  graphInfo,
    CM_DEPENDENCY_PATTERN          pattern,
    uint32_t                       *dispatchFreq
    )
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
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
            dispatchFreq[i] = graphInfo.maxParallelism;
        }
        break;
    case CM_WAVEFRONT:
        for( i = 1; i < graphInfo.maxParallelism; ++i )
        {
            dispatchFreq[i-1] = i;
        }
        for( j = 0; j < graphInfo.numMaxRepeat; ++i, ++j )
        {
            dispatchFreq[i-1] = graphInfo.maxParallelism;
        }
        for( j = graphInfo.maxParallelism - 1; i <= graphInfo.numSteps; ++i, --j )
        {
            dispatchFreq[i-1] = j;
        }
        break;
    case CM_WAVEFRONT26:
        for( i = 1, j = 0; i < graphInfo.maxParallelism; ++i, j +=2 )
        {
            dispatchFreq[j] = i;
            dispatchFreq[j+1] = i;
        }
        for( k = 0; k < graphInfo.numMaxRepeat; ++k, ++j)
        {
            dispatchFreq[j] = graphInfo.maxParallelism;
        }
        for( i = graphInfo.maxParallelism - 1; j < graphInfo.numSteps; j +=2, --i )
        {
            dispatchFreq[j] = i;
            dispatchFreq[j+1] = i;
        }
        break;
    case CM_WAVEFRONT26Z:
        break;
    default:
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        CM_ASSERTMESSAGE("Unsupported dependency pattern for EnqueueWithHints");
        goto finish;
    }

finish:
    return eStatus;
}

//*-----------------------------------------------------------------------------
//| Purpose:  Sets dispatch frequency for kernel group based on number of steps
//| Returns:  Result of the operation
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_SetKernelGrpFreqDispatch(
    PCM_HAL_PARALLELISM_GRAPH_INFO  graphInfo,
    PCM_HAL_KERNEL_GROUP_INFO       groupInfo,
    uint32_t                        numKernelGroups,
    uint32_t                        *minSteps)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
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
            *minSteps = MOS_MIN(*minSteps, tmpSteps);
            groupInfo[i].numStepsInGrp = tmpSteps;
        }

        tmpSteps = 0;
    }

    for( i = 0; i < numKernelGroups; ++i )
    {
        groupInfo[i].freqDispatch = (uint32_t)ceil( (groupInfo[i].numStepsInGrp / (double)*minSteps) );
    }

    return eStatus;
}

//*-----------------------------------------------------------------------------
//| Purpose:  Sets dispatch pattern for kernel with no dependency based on
//|           the minimum number of steps calculated from kernels with dependency
//| Returns:  Result of the operation
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_SetNoDependKernelDispatchPattern(
    uint32_t                        numThreads,
    uint32_t                        minSteps,
    uint32_t                        *dispatchFreq)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    uint32_t i = 0;
    uint32_t numEachStep = 0;
    uint32_t total = 0;

    numEachStep = numThreads / minSteps;
    for( i = 0; i < minSteps; ++i )
    {
        dispatchFreq[i] = numEachStep;
        total += numEachStep;
    }

    while( total != numThreads )
    {
        // dispatch more at beginning
        i = 0;
        dispatchFreq[i]++;
        total++;
        i++;
    }

    return eStatus;
}

MOS_STATUS HalCm_FinishStatesForKernel(
    PCM_HAL_STATE                   state,                                     // [in] Pointer to CM State
    PRENDERHAL_MEDIA_STATE          mediaState,
    PMHW_BATCH_BUFFER               batchBuffer,                               // [in] Pointer to Batch Buffer
    int32_t                         taskId,                                    // [in] Task ID
    PCM_HAL_KERNEL_PARAM            kernelParam,
    int32_t                         kernelIndex,
    PCM_HAL_INDEX_PARAM             indexParam,
    int32_t                         bindingTable,
    int32_t                         mediaID,
    PRENDERHAL_KRN_ALLOCATION       krnAllocation
    )
{
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;
    PCM_HAL_TASK_PARAM              taskParam = state->taskParam;
    PRENDERHAL_INTERFACE            renderHal = state->renderHal;
    PCM_HAL_WALKER_PARAMS           mediaWalkerParams = &kernelParam->walkerParams;
    PCM_GPGPU_WALKER_PARAMS         perKernelGpGpuWalkerParams = &kernelParam->gpgpuWalkerParams;
    PCM_HAL_SCOREBOARD              threadCoordinates = nullptr;
    PCM_HAL_MASK_AND_RESET          dependencyMask = nullptr;
    bool                            enableThreadSpace = false;
    bool                            enableKernelThreadSpace = false;
    PCM_HAL_SCOREBOARD              kernelThreadCoordinates = nullptr;
    UNUSED(taskId);

    MHW_MEDIA_OBJECT_PARAMS         mediaObjectParam;
    PCM_HAL_KERNEL_ARG_PARAM        argParam;
    MHW_PIPE_CONTROL_PARAMS         pipeControlParam;
    uint32_t                        i;
    uint32_t                        hdrSize;
    uint32_t                        aIndex;
    uint32_t                        tIndex;
    uint32_t                        index;

    //GT-PIN
    taskParam->curKernelIndex =  kernelIndex;

    CmSafeMemSet(&mediaObjectParam, 0, sizeof(MHW_MEDIA_OBJECT_PARAMS));

    if (perKernelGpGpuWalkerParams->gpgpuEnabled)
    {
        // GPGPU_WALKER, just update ID here. other fields are already filled.
        perKernelGpGpuWalkerParams->interfaceDescriptorOffset = mediaID;// mediaObjectParam.dwInterfaceDescriptorOffset;
    }
    else if (mediaWalkerParams->cmWalkerEnable)
    {
        // Media walker, just update ID here. other fields are already filled.
        mediaWalkerParams->interfaceDescriptorOffset = mediaID;
    }
    else
    {
        // MEDIA_OBJECT
        mediaObjectParam.dwInterfaceDescriptorOffset = mediaID;
        hdrSize = renderHal->pHwSizes->dwSizeMediaObjectHeaderCmd;

        if (kernelParam->indirectDataParam.indirectDataSize)
        {
            mediaObjectParam.dwInlineDataSize = 0;
        }
        else
        {
            mediaObjectParam.dwInlineDataSize = MOS_MAX(kernelParam->payloadSize, 4);
        }

        if (taskParam->threadCoordinates)
        {
            threadCoordinates = taskParam->threadCoordinates[kernelIndex];
            if (threadCoordinates)
            {
                enableThreadSpace = true;
            }
        }
        else if (kernelParam->kernelThreadSpaceParam.threadCoordinates)
        {
            kernelThreadCoordinates = kernelParam->kernelThreadSpaceParam.threadCoordinates;
            if (kernelThreadCoordinates)
            {
                enableKernelThreadSpace = true;
            }
        }

        if (taskParam->dependencyMasks)
        {
            dependencyMask = taskParam->dependencyMasks[kernelIndex];
        }

        CM_CHK_NULL_GOTOFINISH_MOSERROR( batchBuffer );

        uint8_t inlineData[CM_MAX_THREAD_PAYLOAD_SIZE];
        uint8_t *cmdInline = inlineData;
        uint32_t cmdSize = mediaObjectParam.dwInlineDataSize + hdrSize;

        // Setup states for arguments and threads
        if (((PCM_HAL_BB_ARGS)batchBuffer->pPrivateData)->refCount > 1)
        {
            uint8_t *bBuffer = batchBuffer->pData + batchBuffer->iCurrent;
            for (aIndex = 0; aIndex < kernelParam->numArgs; aIndex++)
            {
                argParam = &kernelParam->argParams[aIndex];

                if ((kernelParam->cmFlags & CM_KERNEL_FLAGS_CURBE) && !argParam->perThread)
                {
                    continue;
                }

                for (tIndex = 0; tIndex < kernelParam->numThreads; tIndex++)
                {
                    index = tIndex * argParam->perThread;

                    //-----------------------------------------------------
                    CM_ASSERT(argParam->payloadOffset < kernelParam->payloadSize);
                    //-----------------------------------------------------

                    switch(argParam->kind)
                    {
                    case CM_ARGUMENT_GENERAL:
                        break;

                    case CM_ARGUMENT_SAMPLER:
                        CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_SetupSamplerState(
                            state, kernelParam, argParam, indexParam,  mediaID, index, nullptr));
                        break;

                    case CM_ARGUMENT_SURFACEBUFFER:
                        CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_SetupBufferSurfaceState(
                            state, argParam, indexParam, bindingTable, -1, index, nullptr));
                        break;

                    case CM_ARGUMENT_SURFACE2D_UP:
                        CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_Setup2DSurfaceUPState(
                            state, argParam, indexParam, bindingTable, index, nullptr));
                        break;

                    case CM_ARGUMENT_SURFACE2DUP_SAMPLER:
                        CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_Setup2DSurfaceUPSamplerState(
                            state, argParam, indexParam, bindingTable, index, nullptr));
                        break;

                    case CM_ARGUMENT_SURFACE2D_SAMPLER:
                        CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_Setup2DSurfaceSamplerState(
                            state, argParam, indexParam, bindingTable, 0, nullptr));
                        break;

                    case CM_ARGUMENT_SURFACE2D:
                        CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_Setup2DSurfaceState(
                            state, argParam, indexParam, bindingTable, index, nullptr));
                        break;

                    case CM_ARGUMENT_SURFACE3D:
                        CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_Setup3DSurfaceState(
                            state, argParam, indexParam, bindingTable, index, nullptr));
                        break;

                    case CM_ARGUMENT_SURFACE_VME:
                        CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_SetupVmeSurfaceState(
                            state, argParam, indexParam, bindingTable, 0, nullptr));
                        break;

                    case CM_ARGUMENT_SURFACE_SAMPLER8X8_AVS:
                    case CM_ARGUMENT_SURFACE_SAMPLER8X8_VA:
                        CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_SetupSampler8x8SurfaceState(
                            state, argParam, indexParam, bindingTable, 0, nullptr));
                        break;

                    default:
                        eStatus = MOS_STATUS_INVALID_PARAMETER;
                        CM_ASSERTMESSAGE(
                            "Argument kind '%d' is not supported", argParam->kind);
                        goto finish;
                    }
                }

                if( dependencyMask )
                {
                    if( dependencyMask[tIndex].resetMask == CM_RESET_DEPENDENCY_MASK )
                    {
                        MOS_SecureMemcpy(bBuffer + (CM_SCOREBOARD_MASK_POS_IN_MEDIA_OBJECT_CMD*sizeof(uint32_t)),
                            sizeof(uint8_t), &dependencyMask[tIndex].mask, sizeof(uint8_t));
                    }
                }
                batchBuffer->iCurrent += cmdSize;
                bBuffer += cmdSize;
            }
        }
        else
        {
            //Insert synchronization if needed (PIPE_CONTROL)
            // 1. synchronization is set
            // 2. the next kernel has dependency pattern
            if((kernelIndex > 0) && ((taskParam->syncBitmap & ((uint64_t)1 << (kernelIndex-1))) || (kernelParam->kernelThreadSpaceParam.patternType != CM_NONE_DEPENDENCY)))
            {
                pipeControlParam = g_cRenderHal_InitPipeControlParams;
                pipeControlParam.presDest                = nullptr;
                pipeControlParam.dwFlushMode             = MHW_FLUSH_CUSTOM; // Use custom flags
                pipeControlParam.dwPostSyncOp            = MHW_FLUSH_NOWRITE;
                pipeControlParam.bDisableCSStall         = false;
                pipeControlParam.bTlbInvalidate          = false;
                pipeControlParam.bFlushRenderTargetCache = true;
                pipeControlParam.bInvalidateTextureCache = true;
                renderHal->pMhwMiInterface->AddPipeControl(nullptr, batchBuffer, &pipeControlParam);
            }

            uint8_t *bBuffer = batchBuffer->pData + batchBuffer->iCurrent;
            for (tIndex = 0; tIndex < kernelParam->numThreads; tIndex++)
            {
                if (enableThreadSpace)
                {
                    mediaObjectParam.VfeScoreboard.ScoreboardEnable = (state->scoreboardParams.ScoreboardMask==0) ? 0:1;
                    mediaObjectParam.VfeScoreboard.Value[0] = threadCoordinates[tIndex].x;
                    mediaObjectParam.VfeScoreboard.Value[1] = threadCoordinates[tIndex].y;
                    mediaObjectParam.VfeScoreboard.ScoreboardColor = threadCoordinates[tIndex].color;
                    mediaObjectParam.dwSliceDestinationSelect = threadCoordinates[tIndex].sliceSelect;
                    mediaObjectParam.dwHalfSliceDestinationSelect = threadCoordinates[tIndex].subSliceSelect;
                    if( !dependencyMask )
                    {
                        mediaObjectParam.VfeScoreboard.ScoreboardMask = (1 << state->scoreboardParams.ScoreboardMask)-1;
                    }
                    else
                    {
                        mediaObjectParam.VfeScoreboard.ScoreboardMask = dependencyMask[tIndex].mask;
                    }
                }
                else if (enableKernelThreadSpace)
                {
                    mediaObjectParam.VfeScoreboard.ScoreboardEnable = (state->scoreboardParams.ScoreboardMask == 0) ? 0 : 1;
                    mediaObjectParam.VfeScoreboard.Value[0] = kernelThreadCoordinates[tIndex].x;
                    mediaObjectParam.VfeScoreboard.Value[1] = kernelThreadCoordinates[tIndex].y;
                    mediaObjectParam.VfeScoreboard.ScoreboardColor = kernelThreadCoordinates[tIndex].color;
                    mediaObjectParam.dwSliceDestinationSelect = kernelThreadCoordinates[tIndex].sliceSelect;
                    mediaObjectParam.dwHalfSliceDestinationSelect = kernelThreadCoordinates[tIndex].subSliceSelect;
                    if (!dependencyMask)
                    {
                        mediaObjectParam.VfeScoreboard.ScoreboardMask = (1 << state->scoreboardParams.ScoreboardMask) - 1;
                    }
                    else
                    {
                        mediaObjectParam.VfeScoreboard.ScoreboardMask = dependencyMask[tIndex].mask;
                    }
                }
                else
                {
                    mediaObjectParam.VfeScoreboard.Value[0] = tIndex % taskParam->threadSpaceWidth;
                    mediaObjectParam.VfeScoreboard.Value[1] = tIndex / taskParam->threadSpaceWidth;
                }

                for (aIndex = 0; aIndex < kernelParam->numArgs; aIndex++)
                {
                    argParam = &kernelParam->argParams[aIndex];
                    index = tIndex * argParam->perThread;

                    if ((kernelParam->cmFlags & CM_KERNEL_FLAGS_CURBE) && !argParam->perThread)
                    {
                        continue;
                    }

                    //-----------------------------------------------------
                    CM_ASSERT(argParam->payloadOffset < kernelParam->payloadSize);
                    //-----------------------------------------------------

                    switch(argParam->kind)
                    {
                    case CM_ARGUMENT_GENERAL:
                        MOS_SecureMemcpy(
                            cmdInline + argParam->payloadOffset,
                            argParam->unitSize,
                            argParam->firstValue + index * argParam->unitSize,
                            argParam->unitSize);
                        break;

                    case CM_ARGUMENT_SAMPLER:
                        CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_SetupSamplerState(
                            state, kernelParam, argParam, indexParam,  mediaID, index, cmdInline));
                        break;

                    case CM_ARGUMENT_SURFACEBUFFER:
                        CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_SetupBufferSurfaceState(
                            state, argParam, indexParam, bindingTable, -1, index, cmdInline));
                        break;

                    case CM_ARGUMENT_SURFACE2D_UP:
                        CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_Setup2DSurfaceUPState(
                            state, argParam, indexParam, bindingTable, index, cmdInline));
                        break;

                    case CM_ARGUMENT_SURFACE2DUP_SAMPLER:
                        CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_Setup2DSurfaceUPSamplerState(
                            state, argParam, indexParam, bindingTable, index, cmdInline));
                        break;

                    case CM_ARGUMENT_SURFACE2D_SAMPLER:
                        CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_Setup2DSurfaceSamplerState(
                            state, argParam, indexParam, bindingTable, index, cmdInline));
                        break;

                    case CM_ARGUMENT_SURFACE2D:
                        CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_Setup2DSurfaceState(
                            state, argParam, indexParam, bindingTable, index, cmdInline));
                        break;

                    case CM_ARGUMENT_SURFACE3D:
                        CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_Setup3DSurfaceState(
                            state, argParam, indexParam, bindingTable, index, cmdInline));
                        break;

                    case CM_ARGUMENT_SURFACE_VME:
                        CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_SetupVmeSurfaceState(
                            state, argParam, indexParam, bindingTable, 0, cmdInline));
                        break;

                    case CM_ARGUMENT_SURFACE_SAMPLER8X8_AVS:
                    case CM_ARGUMENT_SURFACE_SAMPLER8X8_VA:
                        CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_SetupSampler8x8SurfaceState(
                            state, argParam, indexParam, bindingTable, 0, cmdInline));
                        break;

                    default:
                        eStatus = MOS_STATUS_INVALID_PARAMETER;
                        CM_ASSERTMESSAGE(
                            "Argument kind '%d' is not supported", argParam->kind);
                        goto finish;
                    }
                }

                mediaObjectParam.pInlineData = inlineData;
                state->renderHal->pMhwRenderInterface->AddMediaObject(nullptr, batchBuffer, &mediaObjectParam);
            }
        }
    }

    for (i = 0; i < CM_MAX_GLOBAL_SURFACE_NUMBER; i++) {
        if ((kernelParam->globalSurface[i] & CM_SURFACE_MASK) != CM_NULL_SURFACE)
        {
             CM_HAL_KERNEL_ARG_PARAM   tempArgParam;
             argParam = &tempArgParam;

             tempArgParam.kind = CM_ARGUMENT_SURFACEBUFFER;
             tempArgParam.payloadOffset = 0;
             tempArgParam.unitCount = 1;
             tempArgParam.unitSize = sizeof(uint32_t);
             tempArgParam.perThread = false;
             tempArgParam.firstValue = (uint8_t*)&kernelParam->globalSurface[i];
             tempArgParam.aliasIndex = 0;
             tempArgParam.aliasCreated = false;

             CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_SetupBufferSurfaceState(
                       state, argParam, indexParam, bindingTable, (int16_t)i, 0, nullptr));
        }
    }

    // set number of samplers
    krnAllocation->Params.Sampler_Count = indexParam->samplerIndexCount;

    // add SIP surface
    if (kernelParam->kernelDebugEnabled)
    {
        CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_SetupSipSurfaceState(state, indexParam, bindingTable));
    }

finish:
    return eStatus;
}

//*-----------------------------------------------------------------------------
//| Purpose:  Finishes setting up HW states for the kernel
//|           Used by EnqueueWithHints
//| Returns:  Result of the operation
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_FinishStatesForKernelMix(
    PCM_HAL_STATE                      state,
    PMHW_BATCH_BUFFER                  batchBuffer,
    int32_t                            taskId,
    PCM_HAL_KERNEL_PARAM*              cmExecKernels,
    PCM_HAL_INDEX_PARAM                indexParams,
    int32_t                            *bindingTableEntries,
    int32_t                            *mediaIds,
    PRENDERHAL_KRN_ALLOCATION         *krnAllocations,
    uint32_t                           numKernels,
    uint32_t                           hints,
    bool                               lastTask)
{
    MOS_STATUS                         eStatus                = MOS_STATUS_SUCCESS;
    PRENDERHAL_INTERFACE               renderHal              = state->renderHal;
    PMHW_MEDIA_OBJECT_PARAMS           mediaObjectParams      = nullptr;
    PCM_HAL_KERNEL_PARAM*              kernelParams           = nullptr;
    PCM_HAL_KERNEL_ARG_PARAM*          argParams              = nullptr;
    PCM_HAL_BB_ARGS                    bbCmArgs               = nullptr;
    PMHW_VFE_SCOREBOARD                scoreboardParams       = nullptr;
    PCM_HAL_PARALLELISM_GRAPH_INFO     parallelGraphInfo      = nullptr;
    PCM_HAL_KERNEL_ARG_PARAM           argParam               = nullptr;
    PCM_HAL_KERNEL_SUBSLICE_INFO       kernelsSliceInfo       = nullptr;
    PCM_HAL_KERNEL_THREADSPACE_PARAM   kernelTSParam          = nullptr;
    PCM_HAL_KERNEL_GROUP_INFO          groupInfo              = nullptr;
    CM_HAL_DEPENDENCY                  vfeDependencyInfo             ;
    CM_PLATFORM_INFO                   platformInfo                  ;
    CM_GT_SYSTEM_INFO                  systemInfo                    ;
    CM_HAL_SCOREBOARD_XY_MASK          threadCoordinates             ;
    uint32_t                           **dependRemap            = nullptr;
    uint32_t                           **dispatchFreq           = nullptr;
    uint8_t                            **cmdInline             = nullptr;
    uint32_t                           *cmdSizes              = nullptr;
    uint32_t                           *remapKrnToGrp          = nullptr;
    uint32_t                           *remapGrpToKrn          = nullptr;
    uint32_t                           *numKernelsPerGrp       = nullptr;
    uint8_t                            *kernelScoreboardMask   = nullptr;
    uint8_t                            hintsBits               = 0;
    uint8_t                            tmpThreadScoreboardMask = 0;
    uint8_t                            scoreboardMask          = 0;
    bool                               singleSubSlice         = false;
    bool                               enableThreadSpace      = false;
    bool                               kernelFound            = false;
    bool                               updateCurrKernel       = false;
    bool                               noDependencyCase       = false;
    bool                               sufficientSliceInfo    = true;
    uint32_t                           adjustedYCoord         = 0;
    uint32_t                           numKernelGroups         = CM_HINTS_DEFAULT_NUM_KERNEL_GRP;
    uint32_t                           totalNumThreads         = 0;
    uint32_t                           hdrSize                = 0;
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
    uint32_t                           numSubSlicesEnabled     = 0;
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
    uint32_t                           currentKernel          = 0;
    uint32_t                           roundRobinCount        = 0;
    uint32_t                           numTasks                = 0;
    uint32_t                           extraSWThreads          = 0;
    UNUSED(taskId);

    CM_CHK_NULL_GOTOFINISH_MOSERROR(batchBuffer);

    MOS_ZeroMemory(&threadCoordinates, sizeof(CM_HAL_SCOREBOARD_XY_MASK));
    MOS_ZeroMemory(&vfeDependencyInfo, sizeof(CM_HAL_DEPENDENCY));
    MOS_ZeroMemory(&platformInfo, sizeof(CM_PLATFORM_INFO));
    MOS_ZeroMemory(&systemInfo, sizeof(CM_GT_SYSTEM_INFO));

    mediaObjectParams = (PMHW_MEDIA_OBJECT_PARAMS)MOS_AllocAndZeroMemory(sizeof(MHW_MEDIA_OBJECT_PARAMS)*numKernels);
    kernelParams = (PCM_HAL_KERNEL_PARAM*)MOS_AllocAndZeroMemory(sizeof(PCM_HAL_KERNEL_PARAM)*numKernels);
    argParams    = (PCM_HAL_KERNEL_ARG_PARAM*)MOS_AllocAndZeroMemory(sizeof(PCM_HAL_KERNEL_ARG_PARAM)*numKernels);
    cmdInline = (uint8_t**)MOS_AllocAndZeroMemory(sizeof(uint8_t*)*numKernels);
    cmdSizes = (uint32_t*)MOS_AllocAndZeroMemory(sizeof(uint32_t)*numKernels);
    remapKrnToGrp = (uint32_t*)MOS_AllocAndZeroMemory(sizeof(uint32_t)*numKernels);
    remapGrpToKrn = (uint32_t*)MOS_AllocAndZeroMemory(sizeof(uint32_t)*numKernels);
    kernelScoreboardMask = (uint8_t*)MOS_AllocAndZeroMemory(sizeof(uint8_t)*numKernels);
    dependRemap = (uint32_t**)MOS_AllocAndZeroMemory(sizeof(uint32_t*)*numKernels);
    parallelGraphInfo = (PCM_HAL_PARALLELISM_GRAPH_INFO)MOS_AllocAndZeroMemory(sizeof(CM_HAL_PARALLELISM_GRAPH_INFO)*numKernels);
    dispatchFreq = (uint32_t**)MOS_AllocAndZeroMemory(sizeof(uint32_t*)*numKernels);
    numKernelsPerGrp = (uint32_t*)MOS_AllocAndZeroMemory(sizeof(uint32_t)*numKernels);

    if( !mediaObjectParams || !kernelParams || !argParams ||
        !cmdInline || !cmdSizes ||
        !remapKrnToGrp || !remapGrpToKrn || !kernelScoreboardMask || !dependRemap ||
        !parallelGraphInfo || !dispatchFreq || !numKernelsPerGrp )
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        CM_ASSERTMESSAGE("Memory allocation failed in EnqueueWithHints");
        goto finish;
    }

    state->euSaturationEnabled = true;

    hintsBits = (hints & CM_HINTS_MASK_KERNEL_GROUPS) >> CM_HINTS_NUM_BITS_WALK_OBJ;
    CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_GetNumKernelsPerGroup(hintsBits, numKernels, numKernelsPerGrp,
        &numKernelGroups, remapKrnToGrp, remapGrpToKrn));

    kernelsSliceInfo = (PCM_HAL_KERNEL_SUBSLICE_INFO)MOS_AllocAndZeroMemory(sizeof(CM_HAL_KERNEL_SUBSLICE_INFO)*numKernelGroups);
    groupInfo = (PCM_HAL_KERNEL_GROUP_INFO)MOS_AllocAndZeroMemory(sizeof(CM_HAL_KERNEL_GROUP_INFO)*numKernelGroups);
    if( !kernelsSliceInfo || !groupInfo )
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        CM_ASSERTMESSAGE("Memory allocation failed in EnqueueWithHints");
        goto finish;
    }

    for( i = 0; i < numKernelGroups; ++i)
    {
        groupInfo[i].numKernelsInGroup = numKernelsPerGrp[i];
    }

    hdrSize = renderHal->pHwSizes->dwSizeMediaObjectHeaderCmd;

    for ( i = 0; i < numKernels; ++i )
    {
        kernelParams[i] = cmExecKernels[i];

        mediaObjectParams[i].dwInterfaceDescriptorOffset = mediaIds[i];
        mediaObjectParams[i].dwInlineDataSize = MOS_MAX(kernelParams[i]->payloadSize, 4);

        cmdInline[i] = (uint8_t*)MOS_AllocAndZeroMemory(sizeof(uint8_t) * 1024);
        cmdSizes[i] = mediaObjectParams[i].dwInlineDataSize + hdrSize;

        totalNumThreads += kernelParams[i]->numThreads;
    }

    numTasks = ( hints & CM_HINTS_MASK_NUM_TASKS ) >> CM_HINTS_NUM_BITS_TASK_POS;
    if( numTasks > 1 )
    {
        if( lastTask )
        {
            extraSWThreads = totalNumThreads % numTasks;
        }

        totalNumThreads = (totalNumThreads / numTasks) + extraSWThreads;
    }

    for( i = 0; i < numKernels; ++i )
    {
        dependRemap[i] = (uint32_t*)MOS_AllocAndZeroMemory(sizeof(uint32_t) * CM_HAL_MAX_DEPENDENCY_COUNT);
        for( k = 0; k < CM_HAL_MAX_DEPENDENCY_COUNT; ++k )
        {
            // initialize each index to map to itself
            dependRemap[i][k] = k;
        }
    }

    for( i = 0; i < numKernels; ++i )
    {
        kernelTSParam = &kernelParams[i]->kernelThreadSpaceParam;

        // calculate union dependency vector of all kernels with dependency
        if( kernelTSParam->dependencyInfo.count )
        {
            if( vfeDependencyInfo.count == 0 )
            {
                MOS_SecureMemcpy(&vfeDependencyInfo, sizeof(CM_HAL_DEPENDENCY), &kernelTSParam->dependencyInfo, sizeof(CM_HAL_DEPENDENCY));
                kernelScoreboardMask[i] = ( 1 << vfeDependencyInfo.count ) - 1;
            }
            else
            {
                for( j = 0; j < kernelTSParam->dependencyInfo.count; ++j )
                {
                    for( k = 0; k < vfeDependencyInfo.count; ++k )
                    {
                        if( (kernelTSParam->dependencyInfo.deltaX[j] == vfeDependencyInfo.deltaX[k]) &&
                            (kernelTSParam->dependencyInfo.deltaY[j] == vfeDependencyInfo.deltaY[k]) )
                        {
                            CM_HAL_SETBIT(kernelScoreboardMask[i], k);
                            dependRemap[i][j] = k;
                            break;
                        }
                    }
                    if ( k == vfeDependencyInfo.count )
                    {
                        vfeDependencyInfo.deltaX[vfeDependencyInfo.count] = kernelTSParam->dependencyInfo.deltaX[j];
                        vfeDependencyInfo.deltaY[vfeDependencyInfo.count] = kernelTSParam->dependencyInfo.deltaY[j];
                        CM_HAL_SETBIT(kernelScoreboardMask[i], vfeDependencyInfo.count);
                        vfeDependencyInfo.count++;
                        dependRemap[i][j] = k;
                    }
                }
            }
        }
    } // for num kernels

    if( vfeDependencyInfo.count > CM_HAL_MAX_DEPENDENCY_COUNT )
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        CM_ASSERTMESSAGE("Union of kernel dependencies exceeds max dependency count (8)");
        goto finish;
    }

    // set VFE scoreboarding information from union of kernel dependency vectors
    scoreboardParams = &state->scoreboardParams;
    scoreboardParams->ScoreboardMask = (uint8_t)vfeDependencyInfo.count;
    for( i = 0; i < scoreboardParams->ScoreboardMask; ++i )
    {
        scoreboardParams->ScoreboardDelta[i].x = vfeDependencyInfo.deltaX[i];
        scoreboardParams->ScoreboardDelta[i].y = vfeDependencyInfo.deltaY[i];
    }

    if (vfeDependencyInfo.count == 0)
    {
        noDependencyCase = true;
    }

    CM_CHK_MOSSTATUS_GOTOFINISH(state->pfnGetPlatformInfo(state, &platformInfo, true));
    singleSubSlice = (platformInfo.numSubSlices == 1) ? true : false;

    CM_CHK_MOSSTATUS_GOTOFINISH(state->pfnGetGTSystemInfo(state, &systemInfo));

    if( !singleSubSlice )
    {
        for( i = 0; i < numKernelGroups; ++i )
        {
            tmpNumKernelsPerGrp = numKernelsPerGrp[i];

            for( j = 0; j < tmpNumKernelsPerGrp; ++j )
            {
                kernelTSParam = &kernelParams[count]->kernelThreadSpaceParam;

                switch( kernelTSParam->patternType )
                {
                case CM_NONE_DEPENDENCY:
                    maximum = kernelParams[count]->numThreads;
                    break;
                case CM_WAVEFRONT:
                    maximum = MOS_MIN(kernelTSParam->threadSpaceWidth, kernelTSParam->threadSpaceHeight);
                    break;
                case CM_WAVEFRONT26:
                    maximum = MOS_MIN( ((kernelTSParam->threadSpaceWidth + 1) >> 1), kernelTSParam->threadSpaceHeight);
                    break;
                case CM_VERTICAL_WAVE:
                    maximum = kernelTSParam->threadSpaceHeight;
                    break;
                case CM_HORIZONTAL_WAVE:
                    maximum = kernelTSParam->threadSpaceWidth;
                    break;
                case CM_WAVEFRONT26Z:
                    maximum = MOS_MIN( ((kernelTSParam->threadSpaceWidth - 1) >> 1), kernelTSParam->threadSpaceHeight);
                    break;
                default:
                    eStatus = MOS_STATUS_INVALID_PARAMETER;
                    CM_ASSERTMESSAGE("Unsupported dependency pattern for EnqueueWithHints");
                    goto finish;
                }

                if( kernelTSParam->patternType != CM_WAVEFRONT26Z )
                {
                    CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_GetParallelGraphInfo(maximum, kernelParams[count]->numThreads,
                        kernelTSParam->threadSpaceWidth, kernelTSParam->threadSpaceHeight,
                        &parallelGraphInfo[count], kernelTSParam->patternType, noDependencyCase));
                }
                else
                {
                    parallelGraphInfo[count].numSteps = kernelTSParam->dispatchInfo.numWaves;
                }

                if( kernelTSParam->patternType != CM_NONE_DEPENDENCY )
                {
                    dispatchFreq[count] = (uint32_t*)MOS_AllocAndZeroMemory(sizeof(uint32_t)*parallelGraphInfo[count].numSteps);
                    if( !dispatchFreq[count] )
                    {
                        eStatus = MOS_STATUS_INVALID_PARAMETER;
                        CM_ASSERTMESSAGE("Memory allocation failed for EnqueueWithHints");
                        goto finish;
                    }

                    if( kernelTSParam->patternType != CM_WAVEFRONT26Z )
                    {
                        CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_SetDispatchPattern(parallelGraphInfo[count], kernelTSParam->patternType, dispatchFreq[count]));
                    }
                    else
                    {
                        MOS_SecureMemcpy(dispatchFreq[count], sizeof(uint32_t)*parallelGraphInfo[count].numSteps,
                                         kernelTSParam->dispatchInfo.numThreadsInWave, sizeof(uint32_t)*parallelGraphInfo[count].numSteps);
                    }
                }

                if (!noDependencyCase)
                {
                    tmpNumSubSlice =
                        (maximum / (platformInfo.numEUsPerSubSlice * platformInfo.numHWThreadsPerEU)) + 1;

                    if (tmpNumSubSlice > platformInfo.numSubSlices)
                    {
                        tmpNumSubSlice = platformInfo.numSubSlices - 1;
                    }

                    if (tmpNumSubSlice > kernelsSliceInfo[i].numSubSlices)
                    {
                        kernelsSliceInfo[i].numSubSlices = tmpNumSubSlice;
                    }
                }
                else
                {
                    kernelsSliceInfo[i].numSubSlices = platformInfo.numSubSlices;
                }

                count++;
            }
        }

        if (!noDependencyCase)
        {
            for (i = 0; i < numKernelGroups; ++i)
            {
                totalReqSubSlices += kernelsSliceInfo[i].numSubSlices;
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
                    kernelsSliceInfo[tmp].numSubSlices++;
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
                    kernelsSliceInfo[tmp].numSubSlices--;
                    totalReqSubSlices--;
                    tmp1 += numKernelGroups - 1;
                }
            }

            if (totalReqSubSlices != platformInfo.numSubSlices)
            {
                eStatus = MOS_STATUS_INVALID_PARAMETER;
                CM_ASSERTMESSAGE("Total requested sub-slices does not match platform's number of sub-slices");
                goto finish;
            }
        }

        for(i = 0; i < numKernelGroups; ++i)
        {
            kernelsSliceInfo[i].destination = (PCM_HAL_KERNEL_SLICE_SUBSLICE)MOS_AllocAndZeroMemory(sizeof(CM_HAL_KERNEL_SLICE_SUBSLICE)*kernelsSliceInfo[i].numSubSlices);
            if( !kernelsSliceInfo[i].destination )
            {
                eStatus = MOS_STATUS_INVALID_PARAMETER;
                CM_ASSERTMESSAGE("Memory allocation failed in EnqueueWithHints");
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
                        if (curKernel < numKernelGroups)
                        {
                            if (kernelsSliceInfo[curKernel].numSubSlices == numSet)
                            {
                                curKernel++;
                                numSet = 0;
                            }
                        }

                        if (curKernel < numKernelGroups)
                        {
                            kernelsSliceInfo[curKernel].destination[numSet].slice = i;
                            kernelsSliceInfo[curKernel].destination[numSet].subSlice = j;

                            numSet++;
                        }

                        numSubSlicesEnabled++;
                    }
                }
            }


            if (numSubSlicesEnabled != platformInfo.numSubSlices)
            {
                // not enough slice information, do not assign sub-slice destination
                sufficientSliceInfo = false;
            }
        }

        // set freq dispatch ratio for each group
        CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_SetKernelGrpFreqDispatch(parallelGraphInfo, groupInfo, numKernelGroups, &minSteps));

        // set dispatch pattern for kernel with no dependency
        for( i = 0; i < numKernels; ++i )
        {
            if( kernelParams[i]->kernelThreadSpaceParam.patternType == CM_NONE_DEPENDENCY )
            {
                grpId = remapKrnToGrp[i];
                allocSize = 0;

                if( groupInfo[grpId].freqDispatch == 0 )
                {
                    allocSize = minSteps;
                    groupInfo[grpId].freqDispatch = 1;
                }
                else
                {
                    allocSize = minSteps * groupInfo[grpId].freqDispatch;
                    groupInfo[grpId].freqDispatch = groupInfo[grpId].freqDispatch * 2;
                }

                dispatchFreq[i] = (uint32_t*)MOS_AllocAndZeroMemory(sizeof(uint32_t)*allocSize);
                if( !dispatchFreq[i] )
                {
                    eStatus = MOS_STATUS_INVALID_PARAMETER;
                    CM_ASSERTMESSAGE("Memory allocation failed in EnqueueWithHints");
                    goto finish;
                }

                CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_SetNoDependKernelDispatchPattern(kernelParams[i]->numThreads,
                    allocSize, dispatchFreq[i]));
            }
        }
    }

    CM_CHK_NULL_GOTOFINISH_MOSERROR(batchBuffer->pPrivateData);
    bbCmArgs = (PCM_HAL_BB_ARGS) batchBuffer->pPrivateData;
    if( bbCmArgs->refCount > 1 )
    {

        uint8_t *bBuffer = batchBuffer->pData + batchBuffer->iCurrent;
        updateCurrKernel = false;
        for( i = 0; i < totalNumThreads; ++i )
        {
            if( !singleSubSlice )
            {
                if( (dispatchFreq[currentKernel][state->hintIndexes.dispatchIndexes[currentKernel]] == numDispatched) ||
                    (state->hintIndexes.kernelIndexes[currentKernel] >= kernelParams[currentKernel]->numThreads) )
                {
                    numDispatched = 0;
                    numStepsDispatched++;
                    state->hintIndexes.dispatchIndexes[currentKernel]++;

                    if( state->hintIndexes.kernelIndexes[currentKernel] >= kernelParams[currentKernel]->numThreads )
                    {
                        updateCurrKernel = true;
                        groupInfo[remapKrnToGrp[currentKernel]].numKernelsFinished++;
                        if( groupInfo[remapKrnToGrp[currentKernel]].numKernelsFinished ==
                            groupInfo[remapKrnToGrp[currentKernel]].numKernelsInGroup )
                        {
                            groupInfo[remapKrnToGrp[currentKernel]].groupFinished = 1;
                        }
                        else
                        {
                            remapGrpToKrn[tmpIndex]++;
                        }
                    }

                    if( (groupInfo[remapKrnToGrp[currentKernel]].freqDispatch == numStepsDispatched) ||
                        updateCurrKernel )
                    {
                        numStepsDispatched = 0;
                        roundRobinCount++;

                        tmpIndex = roundRobinCount % numKernelGroups;

                        if( groupInfo[tmpIndex].groupFinished )
                        {
                            loopCount = 0;
                            while( (loopCount < numKernelGroups) && (!kernelFound) )
                            {
                                roundRobinCount++;
                                tmpIndex = roundRobinCount % numKernelGroups;
                                if( state->hintIndexes.kernelIndexes[remapGrpToKrn[tmpIndex]] < kernelParams[remapGrpToKrn[tmpIndex]]->numThreads )
                                {
                                    kernelFound = true;
                                }
                                loopCount++;
                            }
                            if( !kernelFound )
                            {
                                // Error shouldn't be here
                                // if still in for loop totalNumThreads, needs to be a kernel with threads left
                                eStatus = MOS_STATUS_UNKNOWN;
                                CM_ASSERTMESSAGE("Couldn't find kernel with threads left for EnqueueWithHints");
                                goto finish;
                            }
                        }
                        currentKernel = remapGrpToKrn[tmpIndex];
                    }
                }
            }
            else
            {
                 if( state->hintIndexes.kernelIndexes[currentKernel] >= kernelParams[currentKernel]->numThreads )
                 {
                     currentKernel++;
                 }
            }

            if( kernelParams[currentKernel]->kernelThreadSpaceParam.threadCoordinates )
            {
                threadCoordinates.y = kernelParams[currentKernel]->kernelThreadSpaceParam.threadCoordinates[state->hintIndexes.kernelIndexes[currentKernel]].y;
                threadCoordinates.mask = kernelParams[currentKernel]->kernelThreadSpaceParam.threadCoordinates[state->hintIndexes.kernelIndexes[currentKernel]].mask;
                enableThreadSpace = true;
                threadCoordinates.resetMask = kernelParams[currentKernel]->kernelThreadSpaceParam.threadCoordinates[state->hintIndexes.kernelIndexes[currentKernel]].resetMask;
            }

             if( enableThreadSpace )
             {
                 if( threadCoordinates.mask != CM_DEFAULT_THREAD_DEPENDENCY_MASK )
                 {
                     tmpThreadScoreboardMask = kernelScoreboardMask[currentKernel];
                     // do the remapping
                     for( k = 0; k < kernelParams[currentKernel]->kernelThreadSpaceParam.dependencyInfo.count; ++k )
                     {
                         if( (threadCoordinates.mask & CM_HINTS_LEASTBIT_MASK) == 0 )
                         {
                             CM_HAL_UNSETBIT(tmpThreadScoreboardMask, dependRemap[currentKernel][k]);
                         }

                         threadCoordinates.mask = threadCoordinates.mask >> 1;
                     }
                         scoreboardMask = tmpThreadScoreboardMask;
                 }
                 else
                 {
                     scoreboardMask = kernelScoreboardMask[currentKernel];
                 }
             }
             else
             {
                 threadCoordinates.y = state->hintIndexes.kernelIndexes[currentKernel] / kernelParams[currentKernel]->kernelThreadSpaceParam.threadSpaceWidth;
                 scoreboardMask = kernelScoreboardMask[currentKernel];
             }

             adjustedYCoord = 0;
             if( currentKernel > 0 )
             {
                 // if not first kernel, and has dependency,
                 // and along scoreboard border top need to mask out dependencies with y < 0
                 if( kernelScoreboardMask[currentKernel] )
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

             if( currentKernel < numKernels - 1 )
             {
                 // if not last kernel, and has dependency,
                 // along scoreboard border bottom need to mask out dependencies with y > 0
                 if( kernelScoreboardMask[currentKernel] )
                 {
                     if( threadCoordinates.y == (kernelParams[currentKernel]->kernelThreadSpaceParam.threadSpaceHeight - 1))
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

            for( aIndex = 0; aIndex < kernelParams[currentKernel]->numArgs; aIndex++ )
            {
                argParams[currentKernel] = &kernelParams[currentKernel]->argParams[aIndex];
                index = state->hintIndexes.kernelIndexes[currentKernel] * argParams[currentKernel]->perThread;

                if( (kernelParams[currentKernel]->cmFlags & CM_KERNEL_FLAGS_CURBE) && !argParams[currentKernel]->perThread )
                {
                    continue;
                }

                CM_ASSERT(argParams[currentKernel]->payloadOffset < kernelParams[currentKernel]->payloadSize);

                switch(argParams[currentKernel]->kind)
                {
                case CM_ARGUMENT_GENERAL:
                    break;

                case CM_ARGUMENT_SAMPLER:
                    CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_SetupSamplerState(
                        state, kernelParams[currentKernel], argParams[currentKernel], &indexParams[currentKernel],
                        mediaIds[currentKernel], index, nullptr));
                    break;

                case CM_ARGUMENT_SURFACEBUFFER:
                    CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_SetupBufferSurfaceState(
                        state, argParams[currentKernel], &indexParams[currentKernel],
                        bindingTableEntries[currentKernel], -1, index, nullptr));
                    break;

                case CM_ARGUMENT_SURFACE2D_UP:
                    CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_Setup2DSurfaceUPState(
                        state, argParams[currentKernel], &indexParams[currentKernel],
                        bindingTableEntries[currentKernel], index, nullptr));
                    break;

                case CM_ARGUMENT_SURFACE2DUP_SAMPLER:
                    CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_Setup2DSurfaceUPSamplerState(
                        state, argParams[currentKernel], &indexParams[currentKernel],
                        bindingTableEntries[currentKernel], index, nullptr));
                    break;

                case CM_ARGUMENT_SURFACE2D_SAMPLER:
                    CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_Setup2DSurfaceSamplerState(
                        state, argParams[currentKernel], &indexParams[currentKernel],
                        bindingTableEntries[currentKernel], 0, nullptr));
                    break;

                case CM_ARGUMENT_SURFACE2D:
                    CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_Setup2DSurfaceState(
                        state, argParams[currentKernel], &indexParams[currentKernel],
                        bindingTableEntries[currentKernel], index, nullptr));
                    break;

                case CM_ARGUMENT_SURFACE3D:
                    CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_Setup3DSurfaceState(
                        state, argParams[currentKernel], &indexParams[currentKernel],
                        bindingTableEntries[currentKernel], index, nullptr));
                    break;

                case CM_ARGUMENT_SURFACE_VME:
                    CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_SetupVmeSurfaceState(
                        state, argParams[currentKernel], &indexParams[currentKernel],
                        bindingTableEntries[currentKernel], 0, nullptr));
                    break;

                case CM_ARGUMENT_SURFACE_SAMPLER8X8_VA:
                case CM_ARGUMENT_SURFACE_SAMPLER8X8_AVS:
                    CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_SetupSampler8x8SurfaceState(
                        state, argParams[currentKernel], &indexParams[currentKernel],
                        bindingTableEntries[currentKernel], 0, nullptr));
                    break;

                default:
                    eStatus = MOS_STATUS_INVALID_PARAMETER;
                    CM_ASSERTMESSAGE(
                        "Argument kind '%d' is not supported", argParams[currentKernel]->kind);
                    goto finish;

                 } // switch argKind
             } // for numArgs

            if( threadCoordinates.resetMask == CM_RESET_DEPENDENCY_MASK )
            {
                MOS_SecureMemcpy(bBuffer + (CM_SCOREBOARD_MASK_POS_IN_MEDIA_OBJECT_CMD*sizeof(uint32_t)),
                    sizeof(uint8_t), &scoreboardMask, sizeof(uint8_t));
            }

            batchBuffer->iCurrent += cmdSizes[currentKernel];
            bBuffer += cmdSizes[currentKernel];

            state->hintIndexes.kernelIndexes[currentKernel]++;
            enableThreadSpace = false;
            kernelFound = false;
            updateCurrKernel = false;
            numDispatched++;
        } // for totalNumThreads
    } // if uiRefCount > 1
    else
    {
        uint8_t *bBuffer = batchBuffer->pData + batchBuffer->iCurrent;
        updateCurrKernel = false;

        for( i = 0; i < totalNumThreads; ++i)
        {
            if( !singleSubSlice )
            {
                if( (dispatchFreq[currentKernel][state->hintIndexes.dispatchIndexes[currentKernel]] == numDispatched) ||
                    (state->hintIndexes.kernelIndexes[currentKernel] >= kernelParams[currentKernel]->numThreads) )
                {
                    numDispatched = 0;
                    numStepsDispatched++;
                    state->hintIndexes.dispatchIndexes[currentKernel]++;

                    if( state->hintIndexes.kernelIndexes[currentKernel] >= kernelParams[currentKernel]->numThreads )
                    {
                        updateCurrKernel = true;
                        groupInfo[remapKrnToGrp[currentKernel]].numKernelsFinished++;
                        if( groupInfo[remapKrnToGrp[currentKernel]].numKernelsFinished ==
                            groupInfo[remapKrnToGrp[currentKernel]].numKernelsInGroup )
                        {
                            groupInfo[remapKrnToGrp[currentKernel]].groupFinished = 1;
                        }
                        else
                        {
                            remapGrpToKrn[tmpIndex]++;
                        }
                    }

                    if( (groupInfo[remapKrnToGrp[currentKernel]].freqDispatch == numStepsDispatched) ||
                        updateCurrKernel )
                    {
                        numStepsDispatched = 0;
                        roundRobinCount++;

                        tmpIndex = roundRobinCount % numKernelGroups;

                        if( groupInfo[tmpIndex].groupFinished )
                        {
                            loopCount = 0;
                            while( (loopCount < numKernelGroups) && (!kernelFound) )
                            {
                                roundRobinCount++;
                                tmpIndex = roundRobinCount % numKernelGroups;
                                if( state->hintIndexes.kernelIndexes[remapGrpToKrn[tmpIndex]] < kernelParams[remapGrpToKrn[tmpIndex]]->numThreads )
                                {
                                    kernelFound = true;
                                }
                                loopCount++;
                            }
                            if( !kernelFound )
                            {
                                // Error shouldn't be here
                                // if still in for loop totalNumThreads, needs to be a kernel with threads left
                                eStatus = MOS_STATUS_UNKNOWN;
                                CM_ASSERTMESSAGE("Couldn't find kernel with threads left for EnqueueWithHints");
                                goto finish;
                            }
                        }

                        currentKernel = remapGrpToKrn[tmpIndex];
                    }
                }
            }
            else
            {
                if( state->hintIndexes.kernelIndexes[currentKernel] >= kernelParams[currentKernel]->numThreads )
                {
                    currentKernel++;
                }
            }

            if( kernelParams[currentKernel]->kernelThreadSpaceParam.threadCoordinates )
            {
                threadCoordinates.x = kernelParams[currentKernel]->kernelThreadSpaceParam.threadCoordinates[state->hintIndexes.kernelIndexes[currentKernel]].x;
                threadCoordinates.y = kernelParams[currentKernel]->kernelThreadSpaceParam.threadCoordinates[state->hintIndexes.kernelIndexes[currentKernel]].y;
                threadCoordinates.mask = kernelParams[currentKernel]->kernelThreadSpaceParam.threadCoordinates[state->hintIndexes.kernelIndexes[currentKernel]].mask;
                enableThreadSpace = true;
            }

            mediaObjectParams[currentKernel].VfeScoreboard.ScoreboardEnable =
                    (kernelParams[currentKernel]->kernelThreadSpaceParam.dependencyInfo.count == 0) ? 0:1;

            if( !singleSubSlice && systemInfo.isSliceInfoValid && sufficientSliceInfo )
            {
                sliceIndex = kernelsSliceInfo[remapKrnToGrp[currentKernel]].counter % kernelsSliceInfo[remapKrnToGrp[currentKernel]].numSubSlices;
                mediaObjectParams[currentKernel].dwSliceDestinationSelect = kernelsSliceInfo[remapKrnToGrp[currentKernel]].destination[sliceIndex].slice;
                mediaObjectParams[currentKernel].dwHalfSliceDestinationSelect = kernelsSliceInfo[remapKrnToGrp[currentKernel]].destination[sliceIndex].subSlice;
                mediaObjectParams[currentKernel].bForceDestination = true;

                kernelsSliceInfo[remapKrnToGrp[currentKernel]].counter++;
            }

            if( enableThreadSpace )
            {
                mediaObjectParams[currentKernel].VfeScoreboard.Value[0] = threadCoordinates.x;
                mediaObjectParams[currentKernel].VfeScoreboard.Value[1] = threadCoordinates.y;
                if( threadCoordinates.mask != CM_DEFAULT_THREAD_DEPENDENCY_MASK )
                {
                    tmpThreadScoreboardMask = kernelScoreboardMask[currentKernel];
                    // do the remapping
                    for( k = 0; k < kernelParams[currentKernel]->kernelThreadSpaceParam.dependencyInfo.count; ++k )
                    {
                        if( (threadCoordinates.mask & CM_HINTS_LEASTBIT_MASK) == 0 )
                        {
                            CM_HAL_UNSETBIT(tmpThreadScoreboardMask, dependRemap[currentKernel][k]);
                        }

                        threadCoordinates.mask = threadCoordinates.mask >> 1;
                    }

                    mediaObjectParams[currentKernel].VfeScoreboard.ScoreboardMask = tmpThreadScoreboardMask;
                }
                else
                {
                    mediaObjectParams[currentKernel].VfeScoreboard.ScoreboardMask = kernelScoreboardMask[currentKernel];
                }
            }
            else
            {
                mediaObjectParams[currentKernel].VfeScoreboard.Value[0] = state->hintIndexes.kernelIndexes[currentKernel] %
                        kernelParams[currentKernel]->kernelThreadSpaceParam.threadSpaceWidth;
                mediaObjectParams[currentKernel].VfeScoreboard.Value[1] = state->hintIndexes.kernelIndexes[currentKernel] /
                        kernelParams[currentKernel]->kernelThreadSpaceParam.threadSpaceWidth;
                mediaObjectParams[currentKernel].VfeScoreboard.ScoreboardMask = kernelScoreboardMask[currentKernel];
            }

             adjustedYCoord = 0;
             // adjust y coordinate for kernels after the first one
             if( currentKernel > 0 )
             {
                 // if not first kernel, and has dependency,
                 // and along scoreboard border need to mask out dependencies with y < 0
                 if( kernelScoreboardMask[currentKernel] )
                 {
                     if (mediaObjectParams[currentKernel].VfeScoreboard.Value[1] == 0)
                     {
                         for( k = 0; k < vfeDependencyInfo.count; ++k )
                         {
                             if( vfeDependencyInfo.deltaY[k] < 0 )
                             {
                                 CM_HAL_UNSETBIT(mediaObjectParams[currentKernel].VfeScoreboard.ScoreboardMask, k);
                             }
                         }
                     }
                 }

                 for( j = currentKernel; j > 0; --j )
                 {
                     adjustedYCoord += kernelParams[j-1]->kernelThreadSpaceParam.threadSpaceHeight;
                 }
             }

             if( currentKernel < numKernels - 1 )
             {
                 // if not last kernel, and has dependency,
                 // along scoreboard border bottom need to mask out dependencies with y > 0
                 if( kernelScoreboardMask[currentKernel] )
                 {
                     if (mediaObjectParams[currentKernel].VfeScoreboard.Value[1] ==
                           (kernelParams[currentKernel]->kernelThreadSpaceParam.threadSpaceHeight - 1))
                     {
                         for( k = 0; k < vfeDependencyInfo.count; ++k )
                         {
                             if( vfeDependencyInfo.deltaY[k] > 0 )
                             {
                                 CM_HAL_UNSETBIT(mediaObjectParams[currentKernel].VfeScoreboard.ScoreboardMask, k);
                             }
                         }
                     }
                 }
             }

            mediaObjectParams[currentKernel].VfeScoreboard.Value[1] =
                mediaObjectParams[currentKernel].VfeScoreboard.Value[1] + adjustedYCoord;

            for( aIndex = 0; aIndex < kernelParams[currentKernel]->numArgs; aIndex++ )
            {
                argParams[currentKernel] = &kernelParams[currentKernel]->argParams[aIndex];
                index = state->hintIndexes.kernelIndexes[currentKernel] * argParams[currentKernel]->perThread;

                if( (kernelParams[currentKernel]->cmFlags & CM_KERNEL_FLAGS_CURBE) && !argParams[currentKernel]->perThread )
                {
                    continue;
                }

                CM_ASSERT(argParams[currentKernel]->payloadOffset < kernelParams[currentKernel]->payloadSize);

                switch(argParams[currentKernel]->kind)
                {
                case CM_ARGUMENT_GENERAL:
                    MOS_SecureMemcpy(
                        cmdInline[currentKernel] + argParams[currentKernel]->payloadOffset,
                        argParams[currentKernel]->unitSize,
                        argParams[currentKernel]->firstValue + index * argParams[currentKernel]->unitSize,
                        argParams[currentKernel]->unitSize);
                    break;

                case CM_ARGUMENT_SAMPLER:
                    CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_SetupSamplerState(
                        state, kernelParams[currentKernel], argParams[currentKernel], &indexParams[currentKernel],
                        mediaIds[currentKernel], index, cmdInline[currentKernel]));
                    break;

                case CM_ARGUMENT_SURFACEBUFFER:
                    CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_SetupBufferSurfaceState(
                        state, argParams[currentKernel], &indexParams[currentKernel],
                        bindingTableEntries[currentKernel], -1, index, cmdInline[currentKernel]));
                    break;

                case CM_ARGUMENT_SURFACE2D_UP:
                    CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_Setup2DSurfaceUPState(
                        state, argParams[currentKernel], &indexParams[currentKernel],
                        bindingTableEntries[currentKernel], index, cmdInline[currentKernel]));
                    break;

                case CM_ARGUMENT_SURFACE2DUP_SAMPLER:
                    CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_Setup2DSurfaceUPSamplerState(
                        state, argParams[currentKernel], &indexParams[currentKernel],
                        bindingTableEntries[currentKernel], index, cmdInline[currentKernel]));
                    break;

                case CM_ARGUMENT_SURFACE2D_SAMPLER:
                    CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_Setup2DSurfaceSamplerState(
                        state, argParams[currentKernel], &indexParams[currentKernel],
                        bindingTableEntries[currentKernel], index, cmdInline[currentKernel]));
                    break;

                case CM_ARGUMENT_SURFACE2D:
                    CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_Setup2DSurfaceState(
                        state, argParams[currentKernel], &indexParams[currentKernel],
                        bindingTableEntries[currentKernel], index, cmdInline[currentKernel]));
                    break;

                case CM_ARGUMENT_SURFACE3D:
                    CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_Setup3DSurfaceState(
                        state, argParams[currentKernel], &indexParams[currentKernel],
                        bindingTableEntries[currentKernel], index, cmdInline[currentKernel]));
                    break;

                case CM_ARGUMENT_SURFACE_VME:
                    CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_SetupVmeSurfaceState(
                        state, argParams[currentKernel], &indexParams[currentKernel],
                        bindingTableEntries[currentKernel], 0, cmdInline[currentKernel]));
                    break;

                case CM_ARGUMENT_SURFACE_SAMPLER8X8_VA:
                case CM_ARGUMENT_SURFACE_SAMPLER8X8_AVS:
                    CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_SetupSampler8x8SurfaceState(
                        state, argParams[currentKernel], &indexParams[currentKernel],
                        bindingTableEntries[currentKernel], 0, cmdInline[currentKernel]));
                    break;

                default:
                    eStatus = MOS_STATUS_INVALID_PARAMETER;
                    CM_ASSERTMESSAGE(
                        "Argument kind '%d' is not supported", argParams[currentKernel]->kind);
                    goto finish;
                }
            }

            mediaObjectParams[currentKernel].pInlineData = cmdInline[currentKernel];
            state->renderHal->pMhwRenderInterface->AddMediaObject(nullptr, batchBuffer, &mediaObjectParams[currentKernel]);

            state->hintIndexes.kernelIndexes[currentKernel]++;
            enableThreadSpace = false;
            kernelFound = false;
            updateCurrKernel = false;
            numDispatched++;
        } // for totalNumThreads
    } // else refCount <= 1

    // setup global surfaces
    for( j = 0; j < numKernels; ++j )
    {
        for( i = 0; i < CM_MAX_GLOBAL_SURFACE_NUMBER; ++i )
        {
            if(( kernelParams[j]->globalSurface[i] & CM_SURFACE_MASK) != CM_NULL_SURFACE)
            {
                CM_HAL_KERNEL_ARG_PARAM tmpArgParam;
                argParam = &tmpArgParam;

                tmpArgParam.kind = CM_ARGUMENT_SURFACEBUFFER;
                tmpArgParam.payloadOffset = 0;
                tmpArgParam.unitCount = 1;
                tmpArgParam.unitSize = sizeof(uint32_t);
                tmpArgParam.perThread = false;
                tmpArgParam.firstValue = (uint8_t*)&kernelParams[j]->globalSurface[i];

                CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_SetupBufferSurfaceState(
                    state, argParam, &indexParams[j], bindingTableEntries[j],
                    (int16_t)i, 0, nullptr));
            }
        }

        // set number of samplers
        krnAllocations[j]->Params.Sampler_Count = indexParams[j].samplerIndexCount;
    }

    // check to make sure we did all threads for all kernels
    if (numTasks <= 1 || lastTask )
    {
        for( i = 0; i < numKernels; ++i )
        {
            if( state->hintIndexes.kernelIndexes[i] < kernelParams[i]->numThreads )
            {
                eStatus = MOS_STATUS_INVALID_PARAMETER;
                CM_ASSERTMESSAGE("Not all threads for all kernels were put into batch buffer");
                goto finish;
            }
        }
    }

    if ( lastTask )
    {
        MOS_ZeroMemory(&state->hintIndexes.kernelIndexes, sizeof(uint32_t) * CM_MAX_TASKS_EU_SATURATION);
        MOS_ZeroMemory(&state->hintIndexes.dispatchIndexes, sizeof(uint32_t) * CM_MAX_TASKS_EU_SATURATION);
    }

finish:
    // free memory
    if( mediaObjectParams )            MOS_FreeMemory(mediaObjectParams);
    if( kernelParams )                 MOS_FreeMemory(kernelParams);
    if( argParams )                    MOS_FreeMemory(argParams);
    if( cmdSizes )                    MOS_FreeMemory(cmdSizes);
    if( remapKrnToGrp )                MOS_FreeMemory(remapKrnToGrp);
    if( remapGrpToKrn )                MOS_FreeMemory(remapGrpToKrn);
    if( kernelScoreboardMask )         MOS_FreeMemory(kernelScoreboardMask);
    if( parallelGraphInfo )            MOS_FreeMemory(parallelGraphInfo);
    if( numKernelsPerGrp )             MOS_FreeMemory(numKernelsPerGrp);
    if( groupInfo )                    MOS_FreeMemory(groupInfo);

    if( cmdInline )
    {
        for( i = 0; i < numKernels; ++i )
        {
            if( cmdInline[i] )
                MOS_FreeMemory(cmdInline[i]);
        }
       MOS_FreeMemory(cmdInline);
    }

    if( kernelsSliceInfo )
    {
        for( i = 0; i < numKernelGroups; ++i )
        {
            if( kernelsSliceInfo[i].destination )
                MOS_FreeMemory(kernelsSliceInfo[i].destination);
        }
        MOS_FreeMemory(kernelsSliceInfo);
    }

    if( dependRemap )
    {
        for( i = 0; i < numKernels; ++i )
        {
            if( dependRemap[i] )
                MOS_FreeMemory(dependRemap[i]);
        }
        MOS_FreeMemory(dependRemap);
    }

    if( dispatchFreq )
    {
        for( i = 0; i < numKernels; ++i )
        {
            if( dispatchFreq[i] )
                MOS_FreeMemory(dispatchFreq[i]);
        }
        MOS_FreeMemory(dispatchFreq);
    }

    return eStatus;
}

uint32_t HalCm_ThreadsNumberPerGroup_MW(PCM_HAL_WALKER_PARAMS walkerParams)
{
    int localInnerCount = 0, localMidCount = 0, localOuterCount = 0, globalInnerCount = 0, globalOuterCount = 0;
    int localInnerCountMax = 0, localMidCountMax = 0, localOuterCountMax = 0, globalInnerCountMax = 0;
    int midX = 0, midY = 0, midStep = 0;
    int outerX = 0, outerY = 0;
    int localInnerX = 0, localInnerY = 0;
    int blockSizeX = 0, blockSizeY = 0;
    //int x, y;

    int localLoopExecCount = walkerParams->localLoopExecCount;
    int globalLoopExecCount = walkerParams->globalLoopExecCount;
    int globalresX = walkerParams->globalResolution.x, globalresY = walkerParams->globalResolution.y;
    int globalOuterX = walkerParams->globalStart.x, globalOuterY = walkerParams->globalStart.y;
    int globalOuterStepX = walkerParams->globalOutlerLoopStride.x, globalOuterStepY = walkerParams->globalOutlerLoopStride.y;
    int globalInnerStepX = walkerParams->globalInnerLoopUnit.x, globalInnerStepY = walkerParams->globalInnerLoopUnit.y;
    int middleStepX = walkerParams->midLoopUnitX, middleStepY = walkerParams->midLoopUnitY, extraSteps = walkerParams->middleLoopExtraSteps;
    int localblockresX = walkerParams->blockResolution.x, localblockresY = walkerParams->blockResolution.y;
    int localStartX = walkerParams->localStart.x, localStartY = walkerParams->localStart.y;
    int localOuterStepX = walkerParams->localOutLoopStride.x, localOuterStepY = walkerParams->localOutLoopStride.y;
    int localInnerStepX = walkerParams->localInnerLoopUnit.x, localInnerStepY = walkerParams->localInnerLoopUnit.y;

    uint32_t threadsNumberPergroup = 0;

    //do global_outer_looper initialization
    while (((globalOuterX >= globalresX) && (globalInnerStepX < 0)) ||
        (((globalOuterX + localblockresX) < 0) && (globalInnerStepX > 0)) ||
        ((globalOuterY >= globalresY) && (globalInnerStepY < 0)) ||
        (((globalOuterX + localblockresY) < 0) && (globalInnerStepY > 0)))
    {
        globalOuterX += globalInnerStepX;
        globalOuterY += globalInnerStepY;
    }

    //global_ouer_loop_in_bounds()
    while ((globalOuterX < globalresX) &&
        (globalOuterY < globalresY) &&
        (globalOuterX + localblockresX > 0) &&
        (globalOuterY + localblockresY > 0) &&
        (globalOuterCount <= globalLoopExecCount))
    {
        int globalInnerX = globalOuterX;
        int globalInnerY = globalOuterY;

        if (globalInnerCountMax < globalInnerCount)
        {
            globalInnerCountMax = globalInnerCount;
        }
        globalInnerCount = 0;

        //global_inner_loop_in_bounds()
        while ((globalInnerX < globalresX) &&
            (globalInnerY < globalresY) &&
            (globalInnerX + localblockresX > 0) &&
            (globalInnerY + localblockresY > 0))
        {
            int globalInnerXCopy = globalInnerX;
            int globalInnerYCopy = globalInnerY;
            if (globalInnerX < 0)
                globalInnerXCopy = 0;
            if (globalInnerY < 0)
                globalInnerYCopy = 0;

            if (globalInnerX < 0)
                blockSizeX = localblockresX + globalInnerX;
            else if ((globalresX - globalInnerX) < localblockresX)
                blockSizeX = globalresX - globalInnerX;
            else
                blockSizeX = localblockresX;
            if (globalInnerY < 0)
                blockSizeY = localblockresY + globalInnerY;
            else if ((globalresY - globalInnerY) < localblockresY)
                blockSizeY = globalresY - globalInnerY;
            else
                blockSizeY = localblockresY;

            outerX = localStartX;
            outerY = localStartY;

            if (localOuterCountMax < localOuterCount)
            {
                localOuterCountMax = localOuterCount;
            }
            localOuterCount = 0;

            while ((outerX >= blockSizeX && localInnerStepX < 0) ||
                (outerX < 0 && localInnerStepX > 0) ||
                (outerY >= blockSizeY && localInnerStepY < 0) ||
                (outerY < 0 && localInnerStepY > 0))
            {
                outerX += localInnerStepX;
                outerY += localInnerStepY;
            }

            //local_outer_loop_in_bounds()
            while ((outerX < blockSizeX) &&
                (outerY < blockSizeY) &&
                (outerX >= 0) &&
                (outerY >= 0) &&
                (localOuterCount <= localLoopExecCount))
            {
                midX = outerX;
                midY = outerY;
                midStep = 0;

                if (localMidCountMax < localMidCount)
                {
                    localMidCountMax = localMidCount;
                }
                localMidCount = 0;

                //local_middle_steps_remaining()
                while ((midStep <= extraSteps) &&
                    (midX < blockSizeX) &&
                    (midY < blockSizeY) &&
                    (midX >= 0) &&
                    (midY >= 0))
                {
                    localInnerX = midX;
                    localInnerY = midY;

                    if (localInnerCountMax < localInnerCount)
                    {
                        localInnerCountMax = localInnerCount;
                    }
                    localInnerCount = 0;

                    //local_inner_loop_shrinking()
                    while ((localInnerX < blockSizeX) &&
                        (localInnerY < blockSizeY) &&
                        (localInnerX >= 0) &&
                        (localInnerY >= 0))
                    {
                        //x = localInnerX + globalInnerXCopy;
                        //y = localInnerY + globalInnerYCopy;
                        localInnerCount ++;

                        localInnerX += localInnerStepX;
                        localInnerY += localInnerStepY;
                    }
                    localMidCount++;
                    midStep++;
                    midX += middleStepX;
                    midY += middleStepY;
                }
                localOuterCount += 1;
                outerX += localOuterStepX;
                outerY += localOuterStepY;
                while ((outerX >= blockSizeX && localInnerStepX < 0) ||
                    (outerX <0 && localInnerStepX > 0) ||
                    (outerY >= blockSizeY && localInnerStepY < 0) ||
                    (outerY <0 && localInnerStepY > 0))
                {
                    outerX += localInnerStepX;
                    outerY += localInnerStepY;
                }
            }
            globalInnerCount++;
            globalInnerX += globalInnerStepX;
            globalInnerY += globalInnerStepY;
        }
        globalOuterCount += 1;
        globalOuterX += globalOuterStepX;
        globalOuterY += globalOuterStepY;
        while (((globalOuterX >= globalresX) && (globalInnerStepX < 0)) ||
            (((globalOuterX + localblockresX) < 0) && (globalInnerStepX > 0)) ||
            ((globalOuterY >= globalresY) && (globalInnerStepY < 0)) ||
            (((globalOuterX + localblockresY) < 0) && (globalInnerStepY > 0)))
        {
            globalOuterX += globalInnerStepX;
            globalOuterY += globalInnerStepY;
        }
    }

    switch (walkerParams->groupIdLoopSelect)
    {
        case CM_MW_GROUP_COLORLOOP:
            threadsNumberPergroup = walkerParams->colorCountMinusOne + 1;
            break;
        case CM_MW_GROUP_INNERLOCAL:
            threadsNumberPergroup = localInnerCount * (walkerParams->colorCountMinusOne + 1);
            break;
        case CM_MW_GROUP_MIDLOCAL:
            threadsNumberPergroup = localMidCount * localInnerCount * (walkerParams->colorCountMinusOne + 1);
            break;
        case CM_MW_GROUP_OUTERLOCAL:
            threadsNumberPergroup = localOuterCount * localMidCount * localInnerCount * (walkerParams->colorCountMinusOne + 1);
            break;
        case CM_MW_GROUP_INNERGLOBAL:
            threadsNumberPergroup = globalInnerCount * localOuterCount * localMidCount * localInnerCount * (walkerParams->colorCountMinusOne + 1);
            break;
        default:
            threadsNumberPergroup = globalOuterCount * globalInnerCount * localOuterCount * localMidCount * localInnerCount * (walkerParams->colorCountMinusOne + 1);
            break;
    }

    return threadsNumberPergroup;
}

MOS_STATUS HalCm_SetupMediaWalkerParams(
    PCM_HAL_STATE                 state,
    PCM_HAL_KERNEL_PARAM          kernelParam)
{
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;
    PCM_HAL_TASK_PARAM              taskParam = state->taskParam;
    PCM_HAL_WALKER_PARAMS           walkerParams = &kernelParam->walkerParams;

    //Using global walker enable flag
    walkerParams->cmWalkerEnable = state->walkerParams.CmWalkerEnable;
    if (walkerParams->cmWalkerEnable)
    {
        // MEDIA_WALKER
        CM_HAL_KERNEL_THREADSPACE_PARAM kernelThreadSpace;
        if (kernelParam->kernelThreadSpaceParam.threadSpaceWidth)
        {
            kernelThreadSpace.threadSpaceWidth = kernelParam->kernelThreadSpaceParam.threadSpaceWidth;
            kernelThreadSpace.threadSpaceHeight = kernelParam->kernelThreadSpaceParam.threadSpaceHeight;
            kernelThreadSpace.patternType = kernelParam->kernelThreadSpaceParam.patternType;
            kernelThreadSpace.walkingPattern = kernelParam->kernelThreadSpaceParam.walkingPattern;
            kernelThreadSpace.groupSelect = kernelParam->kernelThreadSpaceParam.groupSelect;
            kernelThreadSpace.colorCountMinusOne = kernelParam->kernelThreadSpaceParam.colorCountMinusOne;
        }
        else
        {
            kernelThreadSpace.threadSpaceWidth = (uint16_t)taskParam->threadSpaceWidth;
            kernelThreadSpace.threadSpaceHeight = (uint16_t)taskParam->threadSpaceHeight;
            kernelThreadSpace.patternType = taskParam->dependencyPattern;
            kernelThreadSpace.walkingPattern = taskParam->walkingPattern;
            kernelThreadSpace.groupSelect = taskParam->mediaWalkerGroupSelect;
            kernelThreadSpace.colorCountMinusOne = taskParam->colorCountMinusOne;
        }

        // check for valid thread space width and height here since different from media object
        if (kernelThreadSpace.threadSpaceWidth > state->cmHalInterface->GetMediaWalkerMaxThreadWidth())
        {
            CM_ASSERTMESSAGE("Error: Exceeds the maximum thread space width.");
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            goto finish;
        }
        if (kernelThreadSpace.threadSpaceHeight > state->cmHalInterface->GetMediaWalkerMaxThreadHeight())
        {
            CM_ASSERTMESSAGE("Error: Exceeds the maximum thread space height.");
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            goto finish;
        }

        //walkerParams->InterfaceDescriptorOffset = mediaID;// mediaObjectParam.dwInterfaceDescriptorOffset;
        walkerParams->inlineDataLength = MOS_ALIGN_CEIL(kernelParam->indirectDataParam.indirectDataSize, 4);
        walkerParams->inlineData = kernelParam->indirectDataParam.indirectData;

        walkerParams->colorCountMinusOne = kernelThreadSpace.colorCountMinusOne;// taskParam->ColorCountMinusOne;
        walkerParams->groupIdLoopSelect = (uint32_t)kernelThreadSpace.groupSelect;

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
                if (kernelThreadSpace.threadSpaceWidth > 1)
                {
                    walkPattern = CM_WALK_WAVEFRONT26X;
                }
                else
                {
                    walkPattern = CM_WALK_DEFAULT;
                }
                break;
            case CM_WAVEFRONT26ZIG:
                if (kernelThreadSpace.threadSpaceWidth > 2)
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
        if (taskParam->walkingParamsValid)
        {
            CM_CHK_MOSSTATUS_GOTOFINISH(state->cmHalInterface->SetMediaWalkerParams
                (taskParam->walkingParams, walkerParams));

            if (walkPattern == CM_WALK_HORIZONTAL || walkPattern == CM_WALK_DEFAULT)
            {
                walkerParams->localEnd.x = walkerParams->blockResolution.x - 1;
            }
            else if (walkPattern == CM_WALK_VERTICAL)
            {
                walkerParams->localEnd.y = walkerParams->blockResolution.y - 1;
            }
        }
        else if (kernelParam->kernelThreadSpaceParam.walkingParamsValid)
        {
            CM_CHK_MOSSTATUS_GOTOFINISH(state->cmHalInterface->SetMediaWalkerParams(
                kernelParam->kernelThreadSpaceParam.walkingParams, walkerParams));

            if (walkPattern == CM_WALK_HORIZONTAL || walkPattern == CM_WALK_DEFAULT)
            {
                walkerParams->localEnd.x = walkerParams->blockResolution.x - 1;
            }
            else if (walkPattern == CM_WALK_VERTICAL)
            {
                walkerParams->localEnd.y = walkerParams->blockResolution.y - 1;
            }

        }
        else
        {
            //Local loop parameters
            walkerParams->blockResolution.x = kernelThreadSpace.threadSpaceWidth;
            walkerParams->blockResolution.y = kernelThreadSpace.threadSpaceHeight;

            walkerParams->localStart.x = 0;
            walkerParams->localStart.y = 0;
            walkerParams->localEnd.x = 0;
            walkerParams->localEnd.y = 0;

            walkerParams->globalLoopExecCount = 1;
            walkerParams->midLoopUnitX = 0;
            walkerParams->midLoopUnitY = 0;
            walkerParams->middleLoopExtraSteps = 0;

            // account for odd Height/Width for 26x and 26Zig
            uint16_t adjHeight = ((kernelThreadSpace.threadSpaceHeight + 1) >> 1) << 1;
            uint16_t adjWidth = ((kernelThreadSpace.threadSpaceWidth + 1) >> 1) << 1;

            uint32_t maxThreadWidth = state->cmHalInterface->GetMediaWalkerMaxThreadWidth();
            switch (walkPattern)
            {
                case CM_WALK_DEFAULT:
                case CM_WALK_HORIZONTAL:
                    if (kernelThreadSpace.threadSpaceWidth == kernelParam->numThreads &&
                        kernelThreadSpace.threadSpaceHeight == 1)
                    {
                        walkerParams->blockResolution.x = MOS_MIN(kernelParam->numThreads, maxThreadWidth);
                        walkerParams->blockResolution.y = 1 + kernelParam->numThreads / maxThreadWidth;
                    }
                    walkerParams->localLoopExecCount = walkerParams->blockResolution.y - 1;

                    walkerParams->localOutLoopStride.x = 0;
                    walkerParams->localOutLoopStride.y = 1;
                    walkerParams->localInnerLoopUnit.x = 1;
                    walkerParams->localInnerLoopUnit.y = 0;

                    walkerParams->localEnd.x = walkerParams->blockResolution.x - 1;

                    break;

                case CM_WALK_WAVEFRONT:
                    walkerParams->localLoopExecCount = kernelThreadSpace.threadSpaceWidth + (kernelThreadSpace.threadSpaceHeight - 1) * 1 - 1;

                    walkerParams->localOutLoopStride.x = 1;
                    walkerParams->localOutLoopStride.y = 0;
                    walkerParams->localInnerLoopUnit.x = 0xFFFF;  // -1 in uint32_t:16
                    walkerParams->localInnerLoopUnit.y = 1;
                    break;

                case CM_WALK_WAVEFRONT26:
                    walkerParams->globalResolution.x = kernelThreadSpace.threadSpaceWidth;
                    walkerParams->globalResolution.y = kernelThreadSpace.threadSpaceHeight;
                    walkerParams->localOutLoopStride.x = 1;
                    walkerParams->localOutLoopStride.y = 0;
                    walkerParams->localInnerLoopUnit.x = 0xFFFE;  // -2 in uint32_t:16
                    walkerParams->localInnerLoopUnit.y = 1;
                    walkerParams->localLoopExecCount = kernelThreadSpace.threadSpaceWidth +
                                                       (kernelThreadSpace.threadSpaceHeight - 1) * 2 - 1;

                    //localLoopExecCount has limitation, it should be less than 2^12
                    while (walkerParams->localLoopExecCount >= 0xFFF)
                    {
                        //separate to multiple global levels
                        if (walkerParams->blockResolution.x > (walkerParams->blockResolution.y * 2))
                        {
                            walkerParams->blockResolution.x = (walkerParams->blockResolution.x+1) >> 1;
                            walkerParams->globalLoopExecCount = (walkerParams->globalResolution.x +
                            walkerParams->blockResolution.x - 1) / walkerParams->blockResolution.x;
                        }
                        else
                        {
                            walkerParams->blockResolution.y = (walkerParams->blockResolution.y + 1) >> 1;
                        }
                        walkerParams->localLoopExecCount = walkerParams->blockResolution.x +
                                                           (walkerParams->blockResolution.y - 1) * 2 - 1;
                    }
                    walkerParams->globalOutlerLoopStride.x = walkerParams->blockResolution.x;
                    walkerParams->globalOutlerLoopStride.y = 0;
                    walkerParams->globalInnerLoopUnit.x = 0;
                    walkerParams->globalInnerLoopUnit.y = walkerParams->blockResolution.y;
                    break;

                case CM_WALK_WAVEFRONT26X:
                case CM_WALK_WAVEFRONT26XALT:
                    walkerParams->localLoopExecCount = 0x7ff;
                    walkerParams->globalLoopExecCount = 0;

                    walkerParams->localOutLoopStride.x = 1;
                    walkerParams->localOutLoopStride.y = 0;
                    walkerParams->localInnerLoopUnit.x = 0xFFFE;  // -2 in uint32_t:16
                    walkerParams->localInnerLoopUnit.y = 2;

                    walkerParams->middleLoopExtraSteps = 1;
                    walkerParams->midLoopUnitX = 0;
                    walkerParams->midLoopUnitY = 1;
                    break;

                case CM_WALK_WAVEFRONT26ZIG:
                    walkerParams->localLoopExecCount = 1;
                    walkerParams->globalLoopExecCount = (adjHeight / 2 - 1) * 2 + (adjWidth / 2) - 1;

                    walkerParams->localOutLoopStride.x = 0;
                    walkerParams->localOutLoopStride.y = 1;
                    walkerParams->localInnerLoopUnit.x = 1;
                    walkerParams->localInnerLoopUnit.y = 0;

                    walkerParams->blockResolution.x = 2;
                    walkerParams->blockResolution.y = 2;

                    walkerParams->localEnd.x = walkerParams->blockResolution.x - 1;
                    break;

                case CM_WALK_VERTICAL:
                    walkerParams->localLoopExecCount = walkerParams->blockResolution.x - 1;

                    walkerParams->localOutLoopStride.x = 1;
                    walkerParams->localOutLoopStride.y = 0;
                    walkerParams->localInnerLoopUnit.x = 0;
                    walkerParams->localInnerLoopUnit.y = 1;

                    walkerParams->localEnd.y = walkerParams->blockResolution.y - 1;

                    break;

                case CM_WALK_WAVEFRONT45D:
                    walkerParams->localLoopExecCount = 0x7ff;
                    walkerParams->globalLoopExecCount = 0x7ff;

                    walkerParams->localStart.x = kernelThreadSpace.threadSpaceWidth;
                    walkerParams->localOutLoopStride.x = 1;
                    walkerParams->localOutLoopStride.y = 0;
                    walkerParams->localInnerLoopUnit.x = 0xFFFF;  // -1 in uint32_t:16
                    walkerParams->localInnerLoopUnit.y = 1;
                    break;

                case CM_WALK_WAVEFRONT45XD_2:
                    walkerParams->localLoopExecCount = 0x7ff;
                    walkerParams->globalLoopExecCount = 0x7ff;

                    // Local
                    walkerParams->localStart.x = kernelThreadSpace.threadSpaceWidth;
                    walkerParams->localOutLoopStride.x = 1;
                    walkerParams->localOutLoopStride.y = 0;
                    walkerParams->localInnerLoopUnit.x = 0xFFFF;  // -1 in uint32_t:16
                    walkerParams->localInnerLoopUnit.y = 2;

                    // Mid
                    walkerParams->middleLoopExtraSteps = 1;
                    walkerParams->midLoopUnitX = 0;
                    walkerParams->midLoopUnitY = 1;

                    break;

                case CM_WALK_WAVEFRONT26D:
                    walkerParams->localLoopExecCount = 0x7ff;
                    walkerParams->globalLoopExecCount = 0x7ff;

                    walkerParams->localStart.x = kernelThreadSpace.threadSpaceWidth;
                    walkerParams->localOutLoopStride.x = 1;
                    walkerParams->localOutLoopStride.y = 0;
                    walkerParams->localInnerLoopUnit.x = 0xFFFE;  // -2 in uint32_t:16
                    walkerParams->localInnerLoopUnit.y = 1;
                    break;

                case CM_WALK_WAVEFRONT26XD:
                    walkerParams->localLoopExecCount = 0x7ff;
                    walkerParams->globalLoopExecCount = 0x7ff;

                    // Local
                    walkerParams->localStart.x = kernelThreadSpace.threadSpaceWidth;
                    walkerParams->localOutLoopStride.x = 1;
                    walkerParams->localOutLoopStride.y = 0;
                    walkerParams->localInnerLoopUnit.x = 0xFFFE;  // -2 in uint32_t:16
                    walkerParams->localInnerLoopUnit.y = 2;

                    // Mid
                    walkerParams->middleLoopExtraSteps = 1;
                    walkerParams->midLoopUnitX = 0;
                    walkerParams->midLoopUnitY = 1;
                    break;

                default:
                    walkerParams->localLoopExecCount = MOS_MIN(kernelParam->numThreads, 0x3FF);

                    walkerParams->localOutLoopStride.x = 0;
                    walkerParams->localOutLoopStride.y = 1;
                    walkerParams->localInnerLoopUnit.x = 1;
                    walkerParams->localInnerLoopUnit.y = 0;
                    break;
            }

            //Global loop parameters: execution count, resolution and strides
            //Since no global loop, global resolution equals block resolution.
            walkerParams->globalStart.x = 0;
            walkerParams->globalStart.y = 0;
            walkerParams->globalOutlerLoopStride.y = 0;

            if (walkPattern == CM_WALK_WAVEFRONT26ZIG)
            {
                walkerParams->globalResolution.x = kernelThreadSpace.threadSpaceWidth;
                walkerParams->globalResolution.y = kernelThreadSpace.threadSpaceHeight;
                walkerParams->globalOutlerLoopStride.x = 2;
                walkerParams->globalInnerLoopUnit.x = 0xFFFC;
                walkerParams->globalInnerLoopUnit.y = 2;
            }
            else if(walkPattern != CM_WALK_WAVEFRONT26)
            {
                walkerParams->globalResolution.x = walkerParams->blockResolution.x;
                walkerParams->globalResolution.y = walkerParams->blockResolution.y;
                walkerParams->globalOutlerLoopStride.x = walkerParams->globalResolution.x;
                walkerParams->globalInnerLoopUnit.x = 0;
                walkerParams->globalInnerLoopUnit.y = walkerParams->globalResolution.y;
            }
        }

        //Need calculate number threads per group for media walker, the minimum value is 1
        if (kernelThreadSpace.groupSelect > CM_MW_GROUP_NONE)
        {
            kernelParam->numberThreadsInGroup = HalCm_ThreadsNumberPerGroup_MW(walkerParams);
        }
        else
        {
            kernelParam->numberThreadsInGroup = 1;
        }
    }

finish:
    return eStatus;
}

MOS_STATUS HalCm_AcquireSamplerStatistics(PCM_HAL_STATE state)
{
    MOS_STATUS       eStatus = MOS_STATUS_SUCCESS;
    uint32_t i = 0;

    unsigned int maxBTIindex[MAX_ELEMENT_TYPE_COUNT] = {0}; //tempoary variable, it will hold the max BTI index in each element type

    /* enumerate through the samplerTable for the one in use, then count and analyze */
    for (i = 0; i < state->cmDeviceParam.maxSamplerTableSize; i++) {  //state->CmDeviceParam.iMaxSamplerTableSize;

        if (state->samplerTable[i].bInUse) {
            uint32_t samplerIndex = state->samplerIndexTable[i];
            if (samplerIndex != CM_INVALID_INDEX) {
                MHW_SAMPLER_ELEMENT_TYPE elementType = state->samplerTable[i].ElementType;
                maxBTIindex[elementType] = (maxBTIindex[elementType] > samplerIndex) ? maxBTIindex[elementType] : samplerIndex;
            }
            else
                state->samplerStatistics.samplerCount[state->samplerTable[i].ElementType]++;
        }

    }

    int tempbase=0;
    state->samplerStatistics.samplerIndexBase[MHW_Sampler2Elements]
        = (state->samplerStatistics.samplerCount[MHW_Sampler2Elements]) ? 0 : -1;
    tempbase
        = state->samplerStatistics.samplerIndexBase[MHW_Sampler2Elements];
    state->samplerStatistics.samplerIndexBase[MHW_Sampler4Elements]
        = (state->samplerStatistics.samplerCount[MHW_Sampler4Elements]) ?
        ((tempbase == -1) ? 0 : INDEX_ALIGN(state->samplerStatistics.samplerCount[MHW_Sampler2Elements], 2, 4))
        : tempbase;
    tempbase
        = state->samplerStatistics.samplerIndexBase[MHW_Sampler4Elements];
    state->samplerStatistics.samplerIndexBase[MHW_Sampler8Elements]
        = (state->samplerStatistics.samplerCount[MHW_Sampler8Elements]) ?
        ((tempbase == -1) ? 0 : INDEX_ALIGN(state->samplerStatistics.samplerCount[MHW_Sampler4Elements], 4, 8))
        : tempbase;
    tempbase
        = state->samplerStatistics.samplerIndexBase[MHW_Sampler8Elements];
    state->samplerStatistics.samplerIndexBase[MHW_Sampler64Elements]
        = (state->samplerStatistics.samplerCount[MHW_Sampler64Elements]) ?
        ((tempbase == -1) ? 0 : INDEX_ALIGN(state->samplerStatistics.samplerCount[MHW_Sampler8Elements], 8, 64))
        : tempbase;
    tempbase
        = state->samplerStatistics.samplerIndexBase[MHW_Sampler64Elements];
    state->samplerStatistics.samplerIndexBase[MHW_Sampler128Elements]
        = (state->samplerStatistics.samplerCount[MHW_Sampler128Elements]) ?
        ((tempbase == -1) ? 0 : INDEX_ALIGN(state->samplerStatistics.samplerCount[MHW_Sampler64Elements], 64, 128))
        : tempbase;

    /* There are Sampler BTI, next step needs to consider it during calculate the base */
    for (int k = MHW_Sampler2Elements; k < MHW_Sampler128Elements; k++) {
        if (state->samplerStatistics.samplerIndexBase[k + 1] < maxBTIindex[k])
            state->samplerStatistics.samplerIndexBase[k + 1] = maxBTIindex[k];
    }
    return eStatus;
}

//*-----------------------------------------------------------------------------
//| Purpose:  Initial setup of HW states for the kernel
//| Returns:  Result of the operation
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_SetupStatesForKernelInitial(
    PCM_HAL_STATE                 state,
    PRENDERHAL_MEDIA_STATE        mediaState,
    PMHW_BATCH_BUFFER             batchBuffer,
    int32_t                       taskId,
    PCM_HAL_KERNEL_PARAM          kernelParam,
    PCM_HAL_INDEX_PARAM           indexParam,
    uint32_t                      kernelCurbeOffset,
    int32_t&                          bindingTable,
    int32_t&                          mediaID,
    PRENDERHAL_KRN_ALLOCATION    &krnAllocation)
{
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;
    PRENDERHAL_INTERFACE            renderHal = state->renderHal;
    PRENDERHAL_STATE_HEAP           stateHeap = renderHal->pStateHeap;
    PCM_INDIRECT_SURFACE_INFO       indirectSurfaceInfo = kernelParam->indirectDataParam.surfaceInfo;
    PCM_GPGPU_WALKER_PARAMS         perKernelGpGpuWalkerParames = &kernelParam->gpgpuWalkerParams;
    UNUSED(batchBuffer);
    UNUSED(taskId);

    MHW_MEDIA_OBJECT_PARAMS         mediaObjectParam;
    PCM_HAL_KERNEL_ARG_PARAM        argParam;
    uint32_t                        hdrSize;
    uint32_t                        index;
    uint32_t                        value;
    uint32_t                        btIndex;
    uint32_t                        surfIndex;
    uint32_t                        aIndex;
    uint32_t                        idZ;
    uint32_t                        idY;
    uint32_t                        idX;
    uint32_t                        localIdIndex;
    CM_SURFACE_BTI_INFO             surfBTIInfo;

    bool                            vmeUsed = false;
    CM_PLATFORM_INFO                platformInfo;

    localIdIndex = kernelParam->localIdIndex;

    state->cmHalInterface->GetHwSurfaceBTIInfo(&surfBTIInfo);

    HalCm_PreSetBindingIndex(indexParam, CM_NULL_SURFACE_BINDING_INDEX, CM_NULL_SURFACE_BINDING_INDEX);

    HalCm_PreSetBindingIndex(indexParam, surfBTIInfo.reservedSurfaceStart,
        surfBTIInfo.reservedSurfaceStart + CM_MAX_GLOBAL_SURFACE_NUMBER - 1);

    if (kernelParam->indirectDataParam.surfaceCount)
    {
        for (index = 0; index < kernelParam->indirectDataParam.surfaceCount; index++)
        {
            value = (indirectSurfaceInfo + index)->bindingTableIndex;
            HalCm_PreSetBindingIndex(indexParam, value, value);
        }
    }

    // Get the binding table for this kernel
    CM_CHK_MOSSTATUS_GOTOFINISH(renderHal->pfnAssignBindingTable(renderHal, &bindingTable));

    if (state->dshEnabled)
    {
        // Kernels are already pre-loaded in GSH
        // krnAllocation is the head of a linked list
        if (!krnAllocation)
        {
            CM_ASSERTMESSAGE("Error: Invalid kernel allocation.");
            goto finish;
        }
    }
    else
    {
        // Load the Kernel to GSH
        CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_LoadKernel(
            state,
            kernelParam,
            0,
            krnAllocation));
    }

    // initialize curbe buffer
    if (kernelParam->totalCurbeSize > 0)
    {
        // Update Curbe offset after curbe load command
        if (state->dshEnabled)
        {
            mediaState->pDynamicState->Curbe.iCurrent += MOS_ALIGN_CEIL(kernelParam->totalCurbeSize, state->renderHal->dwCurbeBlockAlign);
        }
        else
        {
            mediaState->iCurbeOffset += MOS_ALIGN_CEIL(kernelParam->totalCurbeSize, state->renderHal->dwCurbeBlockAlign);
        }
    }

    //Setup  media walker parameters if it is
    CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_SetupMediaWalkerParams(state, kernelParam));

    // Allocate Interface Descriptor
    mediaID = HalCm_AllocateMediaID(
        state,
        kernelParam,
        krnAllocation,
        bindingTable,
        kernelCurbeOffset);

    if (mediaID < 0)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        CM_ASSERTMESSAGE("Unable to get Media ID");
        goto finish;
    }

    // Setup the Media object
    hdrSize = renderHal->pHwSizes->dwSizeMediaObjectHeaderCmd;
    mediaObjectParam.dwInterfaceDescriptorOffset = mediaID;
    if (kernelParam->indirectDataParam.indirectDataSize)
    {
        mediaObjectParam.dwInlineDataSize = 0;
    }
    else
    {
        mediaObjectParam.dwInlineDataSize = MOS_MAX(kernelParam->payloadSize, 4);
    }

    // set surface state and binding table
    if (kernelParam->indirectDataParam.surfaceCount)
    {
        for (index = 0; index < kernelParam->indirectDataParam.surfaceCount; index++)
        {
            btIndex = (indirectSurfaceInfo + index)->bindingTableIndex;
            surfIndex = (indirectSurfaceInfo + index)->surfaceIndex;
            switch ((indirectSurfaceInfo + index)->kind)
            {
            case CM_ARGUMENT_SURFACEBUFFER:
                CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_SetupBufferSurfaceStateWithBTIndex(
                    state, bindingTable, surfIndex, btIndex, 0));
                break;

            case CM_ARGUMENT_SURFACE2D:
                CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_Setup2DSurfaceStateWithBTIndex(
                    state, bindingTable, surfIndex, btIndex, 0));
                break;

            case CM_ARGUMENT_SURFACE2D_UP:
                CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_Setup2DSurfaceUPStateWithBTIndex(
                    state, bindingTable, surfIndex, btIndex, 0));
                break;

            case CM_ARGUMENT_SURFACE2D_SAMPLER:
                CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_Setup2DSurfaceStateWithBTIndex(
                    state, bindingTable, surfIndex, btIndex, 1));
                break;
            case CM_ARGUMENT_SURFACE2DUP_SAMPLER:
                CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_Setup2DSurfaceUPStateWithBTIndex(
                    state, bindingTable, surfIndex, btIndex, 1));
                break;
            case CM_ARGUMENT_SURFACE_SAMPLER8X8_AVS:
            case CM_ARGUMENT_SURFACE_SAMPLER8X8_VA:
                CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_SetupSampler8x8SurfaceStateWithBTIndex(
                    state, bindingTable, surfIndex, btIndex, 0, (CM_HAL_KERNEL_ARG_KIND)(indirectSurfaceInfo + index)->kind, 0));
                break;
            case CM_ARGUMENT_SURFACE3D:
                CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_Setup3DSurfaceStateWithBTIndex(
                    state, bindingTable, surfIndex, btIndex));
                break;
            default:
                eStatus = MOS_STATUS_INVALID_PARAMETER;
                CM_ASSERTMESSAGE("Indirect Data surface kind is not supported");
                goto finish;
            }
        }
    }

    // set sampler bti
    if (kernelParam->samplerBTIParam.samplerCount > 0)
    {
        for (uint32_t i = 0; i < kernelParam->samplerBTIParam.samplerCount; i++)
        {
            HalCm_SetupSamplerStateWithBTIndex(state, kernelParam, &kernelParam->samplerBTIParam.samplerInfo[0], i, mediaID);
        }
    }

    if ( ( kernelParam->curbeSizePerThread > 0 ) && ( kernelParam->stateBufferType == CM_STATE_BUFFER_NONE ) )
    {
        uint8_t data[CM_MAX_THREAD_PAYLOAD_SIZE + 32];
        uint8_t curbe[CM_MAX_CURBE_SIZE_PER_TASK + 32];

        MOS_ZeroMemory(data, sizeof(data));
        MOS_ZeroMemory(curbe, sizeof(curbe));
        for (aIndex = 0; aIndex < kernelParam->numArgs; aIndex++)
        {
            argParam = &kernelParam->argParams[aIndex];

            if (argParam->perThread || argParam->isNull)
            {
                continue;
            }

            switch (argParam->kind)
            {
            case CM_ARGUMENT_GENERAL:
            case CM_ARGUMENT_IMPLICT_GROUPSIZE:
            case CM_ARGUMENT_IMPLICT_LOCALSIZE:
            case CM_ARGUMENT_IMPLICIT_LOCALID:
            case CM_ARGUMENT_GENERAL_DEPVEC:
                HalCm_SetArgData(argParam, 0, data);
                break;

            case CM_ARGUMENT_SAMPLER:
                CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_SetupSamplerState(
                    state, kernelParam, argParam, indexParam, mediaID, 0, data));
                break;

            case CM_ARGUMENT_SURFACEBUFFER:
                CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_SetupBufferSurfaceState(
                    state, argParam, indexParam, bindingTable, -1, 0, data));
                break;

            case CM_ARGUMENT_SURFACE2D_UP:
                CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_Setup2DSurfaceUPState(
                    state, argParam, indexParam, bindingTable, 0, data));
                break;

            case CM_ARGUMENT_SURFACE2DUP_SAMPLER:
                CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_Setup2DSurfaceUPSamplerState(
                    state, argParam, indexParam, bindingTable, 0, data));
                break;

            case CM_ARGUMENT_SURFACE2D_SAMPLER:
                CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_Setup2DSurfaceSamplerState(
                    state, argParam, indexParam, bindingTable, 0, data));
                break;

            case CM_ARGUMENT_SURFACE2D:
                CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_Setup2DSurfaceState(
                    state, argParam, indexParam, bindingTable, 0, data));
                break;

            case CM_ARGUMENT_SURFACE3D:
                CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_Setup3DSurfaceState(
                    state, argParam, indexParam, bindingTable, 0, data));
                break;

            case CM_ARGUMENT_SURFACE_VME:   // 3 surface indices
                CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_SetupVmeSurfaceState(
                    state, argParam, indexParam, bindingTable, 0, data));
                vmeUsed = true;
                break;

            case CM_ARGUMENT_SURFACE_SAMPLER8X8_AVS:   // sampler 8x8  surface
            case CM_ARGUMENT_SURFACE_SAMPLER8X8_VA:    // sampler 8x8  surface
                CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_SetupSampler8x8SurfaceState(
                    state, argParam, indexParam, bindingTable, 0, data));
                break;

            case CM_ARGUMENT_STATE_BUFFER:
                CM_CHK_MOSSTATUS_GOTOFINISH( HalCm_SetupStateBufferSurfaceState(
                    state, argParam, indexParam, bindingTable, 0, data ) );
                break;

            case CM_ARGUMENT_SURFACE:
                // Allow null surface
                break;
            case CM_ARGUMENT_SURFACE2D_SCOREBOARD:
                CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_Setup2DSurfaceState(
                    state, argParam, indexParam, bindingTable, 0, data));
                break;

            default:
                eStatus = MOS_STATUS_INVALID_PARAMETER;
                CM_ASSERTMESSAGE("Argument kind '%d' is not supported", argParam->kind);
                goto finish;
            }
        }

        if (perKernelGpGpuWalkerParames->gpgpuEnabled)
        {
            uint32_t offset = 0;

            uint32_t localIdXOffset = kernelParam->argParams[localIdIndex].payloadOffset;
            uint32_t localIdYOffset = localIdXOffset + 4;
            uint32_t localIdZOffset = localIdXOffset + 8;

            //totalCurbeSize aligned when parsing task
            int32_t crossThreadSize = kernelParam->crossThreadConstDataLen;

            //Cross thread constant data
            MOS_SecureMemcpy(curbe + offset, crossThreadSize, data, crossThreadSize);
            offset += crossThreadSize;

            //Per-thread data
            for (idZ = 0; idZ < perKernelGpGpuWalkerParames->threadDepth; idZ++)
            {
                for (idY = 0; idY < perKernelGpGpuWalkerParames->threadHeight; idY++)
                {
                    for (idX = 0; idX < perKernelGpGpuWalkerParames->threadWidth; idX++)
                    {
                        *((uint32_t *)(data + localIdXOffset)) = idX;
                        *((uint32_t *)(data + localIdYOffset)) = idY;
                        *((uint32_t *)(data + localIdZOffset)) = idZ;
                        MOS_SecureMemcpy(curbe + offset, kernelParam->curbeSizePerThread, data + crossThreadSize, kernelParam->curbeSizePerThread);
                        offset += kernelParam->curbeSizePerThread;
                    }
                }
            }

            // tell pfnLoadCurbeData the current curbe offset
            if (state->dshEnabled)
            {
                PRENDERHAL_DYNAMIC_STATE dynamicState = stateHeap->pCurMediaState->pDynamicState;
                dynamicState->Curbe.iCurrent -= MOS_ALIGN_CEIL(kernelParam->totalCurbeSize, state->renderHal->dwCurbeBlockAlign);
                kernelParam->curbeOffset = dynamicState->Curbe.iCurrent;
            }
            else
            {
                stateHeap->pCurMediaState->iCurbeOffset -= MOS_ALIGN_CEIL(kernelParam->totalCurbeSize, state->renderHal->dwCurbeBlockAlign);
                kernelParam->curbeOffset = stateHeap->pCurMediaState->iCurbeOffset;
            }
            // update curbe with data.
            renderHal->pfnLoadCurbeData(renderHal,
                stateHeap->pCurMediaState,
                curbe,
                kernelParam->totalCurbeSize);
        }
        else
        {
            CM_ASSERT(kernelParam->totalCurbeSize == kernelParam->curbeSizePerThread);

            // tell pfnLoadCurbeData the current curbe offset
            if (state->dshEnabled)
            {
                PRENDERHAL_DYNAMIC_STATE dynamicState = stateHeap->pCurMediaState->pDynamicState;
                dynamicState->Curbe.iCurrent -= MOS_ALIGN_CEIL(kernelParam->totalCurbeSize, state->renderHal->dwCurbeBlockAlign);
                kernelParam->curbeOffset = dynamicState->Curbe.iCurrent;
            }
            else
            {
                stateHeap->pCurMediaState->iCurbeOffset -= MOS_ALIGN_CEIL(kernelParam->totalCurbeSize, state->renderHal->dwCurbeBlockAlign);
                kernelParam->curbeOffset = stateHeap->pCurMediaState->iCurbeOffset;
            }
            // update curbe with data.
            renderHal->pfnLoadCurbeData(renderHal,
                stateHeap->pCurMediaState,
                data,
                kernelParam->totalCurbeSize);
        }

        if (state->cmHalInterface->IsOverridePowerOptionPerGpuContext() == false) // false means override per Batch.
        {
            if ((vmeUsed == true) && state->cmHalInterface->IsRequestShutdownSubslicesForVmeUsage())
            {
                CM_CHK_MOSSTATUS_GOTOFINISH(state->pfnGetPlatformInfo(state, &platformInfo, true));
                CM_POWER_OPTION  cmPower;
                cmPower.nSlice = 1;
                cmPower.nSubSlice = platformInfo.numSubSlices / 2;
                cmPower.nEU = (uint16_t)platformInfo.numEUsPerSubSlice;
                state->pfnSetPowerOption(state, &cmPower);
            }
        }
    }

#if MDF_CURBE_DATA_DUMP
    if (state->dumpCurbeData)
    {
        HalCm_DumpCurbeData(state);
    }

#endif

#if MDF_INTERFACE_DESCRIPTOR_DATA_DUMP
    if (state->dumpIDData)
    {
        HalCm_DumpInterfaceDescriptorData(state);
    }   
#endif

finish:
    return eStatus;
}

MOS_STATUS HalCm_SetConditionalEndInfo(
    PCM_HAL_STATE state,
    PCM_HAL_CONDITIONAL_BB_END_INFO conditionalEndInfo,
    PMHW_MI_CONDITIONAL_BATCH_BUFFER_END_PARAMS conditionalBBEndParams,
    uint32_t index
    )
{
    if (index >= CM_MAX_CONDITIONAL_END_CMDS)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    MOS_ZeroMemory(&conditionalBBEndParams[index], sizeof(MHW_MI_CONDITIONAL_BATCH_BUFFER_END_PARAMS));

    conditionalBBEndParams[index].presSemaphoreBuffer = &(state->bufferTable[conditionalEndInfo[index].bufferTableIndex].osResource);
    conditionalBBEndParams[index].dwValue             = conditionalEndInfo[index].compareValue;
    conditionalBBEndParams[index].bDisableCompareMask = conditionalEndInfo[index].disableCompareMask;
    conditionalBBEndParams[index].dwOffset            = conditionalEndInfo[index].offset;

    return MOS_STATUS_SUCCESS;
}

//===============<Interface Functions>==========================================

//*-----------------------------------------------------------------------------
//| Purpose: Allocate Structures required for HW Rendering
//| Returns: Result of the operation
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_Allocate(
    PCM_HAL_STATE state)                                                       // [in] Pointer to CM State
{
    MOS_STATUS              eStatus;
    PCM_HAL_DEVICE_PARAM    deviceParam;
    PRENDERHAL_INTERFACE     renderHal;
    PRENDERHAL_STATE_HEAP_SETTINGS stateHeapSettings;
    uint32_t                i;
    MOS_NULL_RENDERING_FLAGS nullHWAccelerationEnable;
    RENDERHAL_SETTINGS       renderHalSettings;
    uint32_t                maxTasks;

    PMHW_BATCH_BUFFER        batchBuffer = nullptr;

    //------------------------------------
    CM_ASSERT(state);
    //------------------------------------

    eStatus           = MOS_STATUS_UNKNOWN;
    deviceParam    = &state->cmDeviceParam;
    renderHal         = state->renderHal;
    stateHeapSettings = &renderHal->StateHeapSettings;

    stateHeapSettings->iCurbeSize        = CM_MAX_CURBE_SIZE_PER_TASK;
    stateHeapSettings->iMediaStateHeaps  = deviceParam->maxTasks + 1;              // + 1 to handle sync issues with current RenderHal impl (we can remove this once we insert sync value in 2nd level BB)
    stateHeapSettings->iMediaIDs         = deviceParam->maxKernelsPerTask;         // Number of Media IDs = Number of Kernels/Task

    stateHeapSettings->iKernelCount      = deviceParam->maxGshKernelEntries;
    stateHeapSettings->iKernelBlockSize  = deviceParam->maxKernelBinarySize;       // The kernel occupied memory need be this block size aligned 256K for IVB/HSW
    stateHeapSettings->iKernelHeapSize   = deviceParam->maxGshKernelEntries * CM_32K;                       // CM_MAX_GSH_KERNEL_ENTRIES * 32*1024;
    state->totalKernelSize              = (int32_t*)MOS_AllocAndZeroMemory(sizeof(int32_t) * deviceParam->maxGshKernelEntries);
    if(!state->totalKernelSize)
    {
        CM_ASSERTMESSAGE("Could not allocate enough memory for state->totalKernelSize\n");
        eStatus = MOS_STATUS_NO_SPACE;
        goto finish;
    }

    stateHeapSettings->iPerThreadScratchSize = deviceParam->maxPerThreadScratchSpaceSize;
    stateHeapSettings->iSipSize          = CM_MAX_SIP_SIZE;
    stateHeapSettings->iBindingTables    = deviceParam->maxKernelsPerTask;         // Number of Binding tables = Number of Kernels/Task
    stateHeapSettings->iSurfacesPerBT    = CM_MAX_SURFACE_STATES_PER_BT;             // Allocate Max Binding Table indices per binding table
    stateHeapSettings->iSurfaceStates    = CM_MAX_SURFACE_STATES;                    // Allocate Max Surfaces that can be indexed
    stateHeapSettings->iSamplersAVS      = deviceParam->maxAvsSamplers;            // Allocate Max AVS samplers

    // Initialize RenderHal Interface
    CM_CHK_MOSSTATUS_GOTOFINISH(renderHal->pfnInitialize(renderHal, nullptr));

    // Initialize Vebox Interface
    if (state->veboxInterface)
    {
        CM_CHK_MOSSTATUS_GOTOFINISH(state->veboxInterface->CreateHeap());
    }

    // Initialize the table only in Static Mode (DSH doesn't use this table at all)
    if (!state->dshEnabled)
    {
        // Init the data in kernel entries for Dynamic GSH
        for (int32_t kernelID = 0; kernelID < stateHeapSettings->iKernelCount; ++kernelID)
        {
            if (kernelID > 0)
            {
                state->totalKernelSize[kernelID] = 0;
            }
            else
            {
                state->totalKernelSize[kernelID] = stateHeapSettings->iKernelHeapSize;
            }
        }
        state->kernelNumInGsh = 1;
    }

    // Allocate BB (one for each media-state heap)
    state->numBatchBuffers = stateHeapSettings->iMediaStateHeaps;
    state->batchBuffers = (PMHW_BATCH_BUFFER)MOS_AllocAndZeroMemory(
                                    state->numBatchBuffers *
                                    sizeof(MHW_BATCH_BUFFER));

    CM_CHK_NULL_GOTOFINISH_MOSERROR(state->batchBuffers);

    batchBuffer = state->batchBuffers;
    for (i = 0; i < (uint32_t)state->numBatchBuffers; i ++, batchBuffer ++)
    {
        batchBuffer->dwSyncTag    = 0;
        batchBuffer->bMatch       = false;
        batchBuffer->iPrivateType = RENDERHAL_BB_TYPE_CM;
        batchBuffer->iPrivateSize = sizeof(CM_HAL_BB_ARGS);
        batchBuffer->pPrivateData = (PCM_HAL_BB_ARGS)MOS_AllocAndZeroMemory(sizeof(CM_HAL_BB_ARGS));
        CM_CHK_NULL_GOTOFINISH_MOSERROR(batchBuffer->pPrivateData);
        ((PCM_HAL_BB_ARGS)batchBuffer->pPrivateData)->refCount = 1;
    }

    // Allocate TimeStamp Buffer
    CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_AllocateTsResource(state));

    // Allocate tracker resources
    CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_AllocateTrackerResource(state));

    // Initialize dynamic general state heap
    CM_HAL_HEAP_PARAM heapParams;
    heapParams.behaviorGSH        = HeapManager::Behavior::destructiveExtend;
    heapParams.initialSizeGSH     = 0x0080000;
    heapParams.extendSizeGSH      = 0x0080000;
    heapParams.trackerProducer    = &state->renderHal->trackerProducer;
    CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_InitializeDynamicStateHeaps(state, &heapParams));

    CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_AllocateTables(state));

    // Allocate Task Param to hold max tasks
    state->taskParam = (PCM_HAL_TASK_PARAM)MOS_AllocAndZeroMemory(sizeof(CM_HAL_TASK_PARAM));
    CM_CHK_NULL_GOTOFINISH_MOSERROR(state->taskParam);
    state->currentTaskEntry = 0;

    // Allocate Task TimeStamp to hold time stamps
    state->taskTimeStamp = (PCM_HAL_TASK_TIMESTAMP)MOS_AllocAndZeroMemory(sizeof(CM_HAL_TASK_TIMESTAMP));
    CM_CHK_NULL_GOTOFINISH_MOSERROR(state->taskTimeStamp);

    // Setup Registration table entries
    state->surfaceRegTable.count      = state->cmDeviceParam.max2DSurfaceTableSize;
    state->surfaceRegTable.entries    = state->surf2DTable;

    maxTasks = state->cmDeviceParam.maxTasks;
    // Initialize the task status table
    MOS_FillMemory(state->taskStatusTable, (size_t)maxTasks, CM_INVALID_INDEX);

    // Init the null render flag
    nullHWAccelerationEnable  = state->osInterface->pfnGetNullHWRenderFlags(state->osInterface);
    state->nullHwRenderCm          = nullHWAccelerationEnable.Cm || nullHWAccelerationEnable.VPGobal;

    //during initialization stage to allocate sip resource and Get sip binary.
    if ((state->midThreadPreemptionDisabled == false)
     || (state->kernelDebugEnabled == true))
    {
        CM_CHK_MOSSTATUS_GOTOFINISH(state->cmHalInterface->AllocateSIPCSRResource());
        state->pfnGetSipBinary(state);
    }

    //Init flag for conditional batch buffer
    state->cbbEnabled = HalCm_IsCbbEnabled(state);

    //Turn Turbo boost on
    CM_CHK_MOSSTATUS_GOTOFINISH(state->pfnEnableTurboBoost(state));

    state->tsFrequency = Mos_Specific_GetTsFrequency(state->osInterface);

    if (state->refactor)
    {
        state->advExecutor = CmExtensionCreator<CmExecutionAdv>::CreateClass();
        if (state->advExecutor == nullptr)
        {
            CM_ASSERTMESSAGE("Could not allocate enough memory for state->advExecutor\n");
            eStatus = MOS_STATUS_NO_SPACE;
            goto finish;
        }
        state->advExecutor->Initialize(state);
    }
    else
    {
        state->advExecutor = nullptr;
    }

    eStatus = MOS_STATUS_SUCCESS;

finish:
    return eStatus;
}

uint16_t HalCm_GetKernelPerfTag(
    PCM_HAL_STATE           cmState,
    PCM_HAL_KERNEL_PARAM    *kernelParams,
    uint32_t                numKernels)
{
    using namespace std;

    CM_ASSERT(cmState);
    CM_ASSERT(kernelParams);

    int perfTagKernelNum = numKernels - 1;
    if (numKernels > MAX_COMBINE_NUM_IN_PERFTAG)
    {
        perfTagKernelNum = MAX_COMBINE_NUM_IN_PERFTAG - 1;
    }

    // get a combined kernel name
    uint32_t len = numKernels * CM_MAX_KERNEL_NAME_SIZE_IN_BYTE;
    char *combinedName = MOS_NewArray(char, len);
    if (combinedName == nullptr)
    { // Not need to abort the process as this is only for pnp profiling
        CM_ASSERTMESSAGE("Error: Memory allocation error in getPertTag.");
        return 0; // return the default perftag
    }
    CmSafeMemSet(combinedName, 0, len);

    MOS_SecureStrcat(combinedName, len, kernelParams[0]->kernelName);
    for (uint32_t i = 1; i < numKernels; i++)
    {
        MOS_SecureStrcat(combinedName, len, ";");
        MOS_SecureStrcat(combinedName, len, kernelParams[i]->kernelName);
    }

    // get perftag index
    int perfTagIndex = 0;
    map<string, int>::iterator ite = cmState->perfTagIndexMap[perfTagKernelNum]->find(combinedName);
    if (ite == cmState->perfTagIndexMap[perfTagKernelNum]->end())
    {
        if (cmState->currentPerfTagIndex[perfTagKernelNum] <= MAX_CUSTOMIZED_PERFTAG_INDEX)
        {
            cmState->perfTagIndexMap[perfTagKernelNum]->insert(pair<string, int>(combinedName, cmState->currentPerfTagIndex[perfTagKernelNum]));
            perfTagIndex = cmState->currentPerfTagIndex[perfTagKernelNum] ++;
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
    PCM_HAL_STATE           state,                                             // [in] Pointer to CM State
    PCM_HAL_EXEC_TASK_PARAM execParam)                                         // [in] Pointer to Task Param
{
    MOS_STATUS              eStatus;
    PRENDERHAL_INTERFACE    renderHal;
    PRENDERHAL_MEDIA_STATE  mediaState;
    PMHW_BATCH_BUFFER       batchBuffer;
    PCM_HAL_BB_ARGS         bbCmArgs;
    PCM_HAL_KERNEL_PARAM    kernelParam;
    int32_t                 taskId;
    int32_t                 remBindingTables;
    int32_t                 bindingTable;
    int32_t                 bti;
    int32_t                 mediaID;
    PRENDERHAL_KRN_ALLOCATION krnAllocations[CM_MAX_KERNELS_PER_TASK];
    uint32_t                vfeCurbeSize;
    uint32_t                maxInlineDataSize, maxIndirectDataSize;
    uint32_t                i;
    void                    *cmdBuffer = nullptr;
    PCM_HAL_TASK_PARAM      taskParam = state->taskParam;
    uint32_t                btsizePower2;
    PMOS_INTERFACE          osInterface = nullptr;

    //-----------------------------------
    CM_ASSERT(state);
    CM_ASSERT(execParam);
    //-----------------------------------

    eStatus        = MOS_STATUS_SUCCESS;
    renderHal      = state->renderHal;
    mediaState     = nullptr;
    batchBuffer    = nullptr;

    if (execParam->numKernels > state->cmDeviceParam.maxKernelsPerTask)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        CM_ASSERTMESSAGE("Number of Kernels per task exceeds maximum");
        goto finish;
    }

    // Reset states before execute
    // (clear allocations, get GSH allocation index + any additional housekeeping)
    state->osInterface->pfnResetOsStates(state->osInterface);
    CM_CHK_MOSSTATUS_GOTOFINISH(renderHal->pfnReset(renderHal));

    MOS_ZeroMemory(state->taskParam, sizeof(CM_HAL_TASK_PARAM));

    MOS_FillMemory(
        state->bti2DIndexTable,
        state->cmDeviceParam.max2DSurfaceTableSize * sizeof( CM_HAL_MULTI_USE_BTI_ENTRY ),
        CM_INVALID_INDEX );

    MOS_FillMemory(
        state->bti2DUPIndexTable,
        state->cmDeviceParam.max2DSurfaceUPTableSize * sizeof( CM_HAL_MULTI_USE_BTI_ENTRY ),
        CM_INVALID_INDEX );

    MOS_FillMemory(
        state->bti3DIndexTable,
        state->cmDeviceParam.max3DSurfaceTableSize * sizeof( CM_HAL_MULTI_USE_BTI_ENTRY ),
        CM_INVALID_INDEX );

    MOS_FillMemory(
        state->btiBufferIndexTable,
        state->cmDeviceParam.maxBufferTableSize * sizeof( CM_HAL_MULTI_USE_BTI_ENTRY ),
        CM_INVALID_INDEX );

    MOS_FillMemory(
        state->samplerIndexTable,
        state->cmDeviceParam.maxSamplerTableSize,
        CM_INVALID_INDEX);

    MOS_FillMemory(
        state->sampler8x8IndexTable,
        state->cmDeviceParam.maxSampler8x8TableSize,
        CM_INVALID_INDEX);

    state->walkerParams.CmWalkerEnable = 0;

    vfeCurbeSize = 0;
    maxInlineDataSize = 0;
    maxIndirectDataSize = 0;

    // Get the Task Id
    CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_GetNewTaskId(state, &taskId));

    // Parse the task
    CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_ParseTask(state, execParam));

    // Reset the SSH configuration according to the property of the task
    renderHal->pStateHeap->iBindingTableSize = MOS_ALIGN_CEIL(taskParam->surfacePerBT *  // Reconfigure the binding table size
                                                 renderHal->pRenderHalPltInterface->GetBTStateCmdSize(), renderHal->StateHeapSettings.iBTAlignment);
    
    taskParam->surfacePerBT = renderHal->pStateHeap->iBindingTableSize/renderHal->pRenderHalPltInterface->GetBTStateCmdSize();
    
    renderHal->StateHeapSettings.iBindingTables = renderHal->StateHeapSettings.iBindingTables *             // Reconfigure the binding table number
                                                         renderHal->StateHeapSettings.iSurfacesPerBT / taskParam->surfacePerBT;
    
    renderHal->StateHeapSettings.iSurfacesPerBT = taskParam->surfacePerBT;                            // Reconfigure the surface per BT

    if (execParam->numKernels > (uint32_t)renderHal->StateHeapSettings.iBindingTables)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        CM_ASSERTMESSAGE("Number of Kernels per task exceeds the number can be hold by binding table");
        goto finish;
    }

    if (execParam->kernelDebugEnabled && Mos_ResourceIsNull(&state->sipResource.osResource))
    {
       HalCm_AllocateSipResource( state); // create  sip resource if it does not exist
    }

    // Assign a MediaState from the MediaStateHeap
    // !!!! THIS MUST BE BEFORE Getting the BATCH_BUFFER !!!
    // since this method syncs the batch buffer and media state.
    if (state->dshEnabled)
    {
        if ( execParam->userDefinedMediaState != nullptr )
        {
            // use exsiting media state as current state
            mediaState = static_cast< PRENDERHAL_MEDIA_STATE >( execParam->userDefinedMediaState );

            // update current state to dsh
            renderHal->pStateHeap->pCurMediaState = mediaState;
            // Refresh sync tag for all media states in submitted queue
            state->criticalSectionDSH->Acquire();
            renderHal->pfnRefreshSync( renderHal );
            state->criticalSectionDSH->Release();
        }
        else
        {
            // Obtain media state configuration - Curbe, Samplers (3d/AVS/VA), 8x8 sampler table, Media IDs, Kernel Spill area
            RENDERHAL_DYNAMIC_MEDIA_STATE_PARAMS params;
            state->criticalSectionDSH->Acquire();
            HalCm_DSH_GetDynamicStateConfiguration( state, &params, execParam->numKernels, execParam->kernels, execParam->kernelCurbeOffset );

            // Prepare Media States to accommodate all parameters - Curbe, Samplers (3d/AVS/VA), 8x8 sampler table, Media IDs
            mediaState = renderHal->pfnAssignDynamicState( renderHal, &params, RENDERHAL_COMPONENT_CM );
            state->criticalSectionDSH->Release();
        }
    }
    else
    {
        mediaState = renderHal->pfnAssignMediaState(renderHal, RENDERHAL_COMPONENT_CM);
    }
    CM_CHK_NULL_GOTOFINISH_MOSERROR(mediaState);

    // Assign/Reset SSH instance
    CM_CHK_MOSSTATUS_GOTOFINISH(renderHal->pfnAssignSshInstance(renderHal));

    // Dynamic Batch Buffer allocation

    if (!state->walkerParams.CmWalkerEnable)
    {
        // Get the Batch buffer
        CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_GetBatchBuffer(state, execParam->numKernels, execParam->kernels, &batchBuffer));
        CM_CHK_NULL_GOTOFINISH_MOSERROR(batchBuffer);
        bbCmArgs = (PCM_HAL_BB_ARGS)batchBuffer->pPrivateData;

        // Lock the batch buffer
        if ( (bbCmArgs->refCount == 1) ||
             (state->taskParam->reuseBBUpdateMask == 1) )
        {
            CM_CHK_MOSSTATUS_GOTOFINISH(renderHal->pfnLockBB(renderHal, batchBuffer));
        }
    }

    if (state->useNewSamplerHeap == false)
    {
        HalCm_AcquireSamplerStatistics(state);
    }

    // Load all kernels in the same state heap - expand ISH if necessary BEFORE programming media states.
    // This is better than having to expand ISH in the middle of loading, when part of MediaIDs are
    // already programmed - not a problem in the old implementation where it would simply remove old
    // kernels out of the way.
    if (state->dshEnabled)
    {
        CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_DSH_LoadKernelArray(state, execParam->kernels, execParam->numKernels, krnAllocations));
    }

    for (i = 0; i < execParam->numKernels; i++)
    {
        CM_HAL_INDEX_PARAM indexParam;
        MOS_ZeroMemory(&indexParam, sizeof(CM_HAL_INDEX_PARAM));
        kernelParam = execParam->kernels[i];

        CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_SetupStatesForKernelInitial(state, mediaState, batchBuffer, taskId, kernelParam, &indexParam,
            execParam->kernelCurbeOffset[i], bti, mediaID, krnAllocations[i]));

        CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_FinishStatesForKernel(state, mediaState, batchBuffer, taskId, kernelParam, i, &indexParam,
            bti, mediaID, krnAllocations[i]));

        vfeCurbeSize += MOS_ALIGN_CEIL(kernelParam->totalCurbeSize, state->renderHal->dwCurbeBlockAlign);
        if (kernelParam->payloadSize > maxInlineDataSize)
        {
            maxInlineDataSize = kernelParam->payloadSize;
        }
        if (kernelParam->indirectDataParam.indirectDataSize > maxIndirectDataSize)
        {
            maxIndirectDataSize = kernelParam->indirectDataParam.indirectDataSize;
        }

        if (execParam->conditionalEndBitmap & (uint64_t)1 << i)
        {
            CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_SetConditionalEndInfo(state, taskParam->conditionalEndInfo, taskParam->conditionalBBEndParams, i));
        }
    }

    // Store the Max Payload Sizes in the Task params
    state->taskParam->vfeCurbeSize = vfeCurbeSize;
    if (maxIndirectDataSize)
    {
        state->taskParam->urbEntrySize = maxIndirectDataSize;
    }
    else
    {
        state->taskParam->urbEntrySize = maxInlineDataSize;
    }

    // We may have to send additional Binding table commands in command buffer.
    // This is needed because the surface offset (from the base on SSH)
    // calculation takes into account the max binding tables allocated in the
    // SSH.
    remBindingTables = renderHal->StateHeapSettings.iBindingTables - execParam->numKernels;

    if (remBindingTables > 0)
    {
        for (i = 0; i < (uint32_t)remBindingTables; i++)
        {
            CM_CHK_MOSSTATUS_GOTOFINISH(renderHal->pfnAssignBindingTable(
                    renderHal,
                    &bindingTable));
        }
    }

    // until now, we know binding table index for debug surface
    // let's get system thread
    osInterface = state->osInterface;
    osInterface->pfnResetPerfBufferID(osInterface);
    if (osInterface->pfnIsPerfTagSet(osInterface) == false)
    {
        osInterface->pfnIncPerfFrameID(osInterface);
        uint16_t perfTag = HalCm_GetKernelPerfTag(state, execParam->kernels, execParam->numKernels);
        osInterface->pfnSetPerfTag(osInterface, perfTag);
    }
#if (_RELEASE_INTERNAL || _DEBUG)
#if defined(CM_DIRECT_GUC_SUPPORT)
    // Update the task ID table
    state->taskStatusTable[taskId] = (char)taskId;

    //for GuC direct submission, need to send out dummy command buffer to make sure PDP table got binded
    CM_CHK_MOSSTATUS_GOTOFINISH(state->cmHalInterface->SubmitDummyCommands(
        batchBuffer, taskId, execParam->kernels, &cmdBuffer));

    /* make sure Dummy submission is done */

    CM_HAL_QUERY_TASK_PARAM queryParam;

    queryParam.taskId = taskId;
    queryParam.status = CM_TASK_IN_PROGRESS;

    do {
        state->pfnQueryTask(state, &queryParam);
    } while (queryParam.status != CM_TASK_FINISHED);

#endif
#endif

    // Submit HW commands and states
    CM_CHK_MOSSTATUS_GOTOFINISH(state->cmHalInterface->SubmitCommands(
                    batchBuffer, taskId, execParam->kernels, &cmdBuffer));

    // Set the Task ID
    execParam->taskIdOut = taskId;

    // Set OS data
    if(cmdBuffer)
    {
        execParam->osData = cmdBuffer;
    }

    // Update the task ID table
    state->taskStatusTable[taskId] = (char)taskId;

finish:

    if (state->dshEnabled)
    {
        state->criticalSectionDSH->Acquire();
        if (mediaState && eStatus != MOS_STATUS_SUCCESS)
        {
            // Failed, release media state and heap resources
            renderHal->pfnReleaseDynamicState(renderHal, mediaState);
        }
        else
        {
            renderHal->pfnSubmitDynamicState(renderHal, mediaState);
        }
        state->criticalSectionDSH->Release();
    }

    if (batchBuffer)  // for Media Walker, batchBuffer is empty
    {
        if (batchBuffer->bLocked)
        {
            // Only happens in Error cases
            CM_CHK_NULL_RETURN_MOSERROR(batchBuffer->pPrivateData);
            if (((PCM_HAL_BB_ARGS)batchBuffer->pPrivateData)->refCount == 1)
            {
                renderHal->pfnUnlockBB(renderHal, batchBuffer);
            }
        }
    }

    return eStatus;
}

//*-----------------------------------------------------------------------------
//| Purpose: Executes the CM Group Task
//| Returns: Result of the operation
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_ExecuteGroupTask(
    PCM_HAL_STATE                   state,           // [in] Pointer to CM State
    PCM_HAL_EXEC_GROUP_TASK_PARAM   execGroupParam)  // [in] Pointer to Task Param
{
    MOS_STATUS              eStatus = MOS_STATUS_SUCCESS;
    PRENDERHAL_INTERFACE     renderHal = state->renderHal;
    CM_HAL_INDEX_PARAM      indexParam;
    int32_t                 taskId;
    uint32_t                remBindingTables;
    int32_t                 bindingTable;
    int32_t                 bti;
    int32_t                 mediaID;
    PRENDERHAL_MEDIA_STATE  mediaState = nullptr;
    uint32_t                i;
    void                    *cmdBuffer   = nullptr;
    PCM_HAL_KERNEL_PARAM    kernelParam = nullptr;
    PCM_HAL_TASK_PARAM      taskParam = state->taskParam;
    uint32_t                btsizePower2;
    uint32_t                vfeCurbeSize = 0;
    PRENDERHAL_KRN_ALLOCATION krnAllocations[CM_MAX_KERNELS_PER_TASK];
    PMOS_INTERFACE          osInterface = nullptr;

    //-----------------------------------
    CM_ASSERT(state);
    CM_ASSERT(execGroupParam);
    //-----------------------------------

    MOS_ZeroMemory(state->taskParam, sizeof(CM_HAL_TASK_PARAM));
    MOS_ZeroMemory(&indexParam, sizeof(CM_HAL_INDEX_PARAM));

    MOS_FillMemory(
        state->bti2DIndexTable,
        state->cmDeviceParam.max2DSurfaceTableSize * sizeof( CM_HAL_MULTI_USE_BTI_ENTRY ),
        CM_INVALID_INDEX );

    MOS_FillMemory(
        state->bti2DUPIndexTable,
        state->cmDeviceParam.max2DSurfaceUPTableSize * sizeof( CM_HAL_MULTI_USE_BTI_ENTRY ),
        CM_INVALID_INDEX );

    MOS_FillMemory(
        state->bti3DIndexTable,
        state->cmDeviceParam.max3DSurfaceTableSize * sizeof( CM_HAL_MULTI_USE_BTI_ENTRY ),
        CM_INVALID_INDEX );

    MOS_FillMemory(
        state->btiBufferIndexTable,
        state->cmDeviceParam.maxBufferTableSize * sizeof( CM_HAL_MULTI_USE_BTI_ENTRY ),
        CM_INVALID_INDEX );
    MOS_FillMemory(
        state->samplerIndexTable,
        state->cmDeviceParam.maxSamplerTableSize,
        CM_INVALID_INDEX);
    MOS_FillMemory(
        state->sampler8x8IndexTable,
        state->cmDeviceParam.maxSampler8x8TableSize,
        CM_INVALID_INDEX);

    // Reset states before execute
    // (clear allocations, get GSH allocation index + any additional housekeeping)
    state->osInterface->pfnResetOsStates(state->osInterface);
    CM_CHK_MOSSTATUS_GOTOFINISH(renderHal->pfnReset(renderHal));

    state->walkerParams.CmWalkerEnable = 0;
    state->taskParam->blGpGpuWalkerEnabled = true;

    // Get the Task Id
    CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_GetNewTaskId(state, &taskId));

    // Parse the task
    CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_ParseGroupTask(state, execGroupParam));

    // Reset the SSH configuration according to the property of the task
    renderHal->pStateHeap->iBindingTableSize = MOS_ALIGN_CEIL(taskParam->surfacePerBT *               // Reconfigure the binding table size
                                                         renderHal->pRenderHalPltInterface->GetBTStateCmdSize(),
                                                         renderHal->StateHeapSettings.iBTAlignment);

    taskParam->surfacePerBT = renderHal->pStateHeap->iBindingTableSize / renderHal->pRenderHalPltInterface->GetBTStateCmdSize();

    renderHal->StateHeapSettings.iBindingTables           = renderHal->StateHeapSettings.iBindingTables *          // Reconfigure the binding table number
                                                         renderHal->StateHeapSettings.iSurfacesPerBT / taskParam->surfacePerBT;
    renderHal->StateHeapSettings.iSurfacesPerBT           = taskParam->surfacePerBT;                           // Reconfigure the surface per BT

    if (execGroupParam->numKernels > (uint32_t)renderHal->StateHeapSettings.iBindingTables)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        CM_ASSERTMESSAGE("Number of Kernels per task exceeds the number can be hold by binding table");
        goto finish;
    }

    if (execGroupParam->kernelDebugEnabled && Mos_ResourceIsNull(&state->sipResource.osResource))
    {
       HalCm_AllocateSipResource( state); // create  sip resource if it does not exist
    }

    // Assign a MediaState from the MediaStateHeap
    // !!!! THIS MUST BE BEFORE Getting the BATCH_BUFFER !!!
    // since this method syncs the batch buffer and media state.
    if (state->dshEnabled)
    {
        if ( execGroupParam->userDefinedMediaState != nullptr )
        {
            // Preload all kernels
            CM_CHK_MOSSTATUS_GOTOFINISH( HalCm_DSH_LoadKernelArray( state, execGroupParam->kernels, execGroupParam->numKernels, krnAllocations ) );

            // use exsiting media state as current state
            mediaState = static_cast< PRENDERHAL_MEDIA_STATE >( execGroupParam->userDefinedMediaState );

            // update current state to dsh
            renderHal->pStateHeap->pCurMediaState = mediaState;
            state->criticalSectionDSH->Acquire();
            // Refresh sync tag for all media states in submitted queue
            renderHal->pfnRefreshSync( renderHal );
            state->criticalSectionDSH->Release();
        }
        else
        {
            // Preload all kernels
            CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_DSH_LoadKernelArray(state, execGroupParam->kernels, execGroupParam->numKernels, krnAllocations));

            // Obtain media state configuration - Curbe, Samplers (3d/AVS/VA), 8x8 sampler table, Media IDs, Kernel Spill area
            RENDERHAL_DYNAMIC_MEDIA_STATE_PARAMS params;

            state->criticalSectionDSH->Acquire();
            HalCm_DSH_GetDynamicStateConfiguration(state, &params, execGroupParam->numKernels, execGroupParam->kernels, execGroupParam->kernelCurbeOffset);
            // Prepare Media States to accommodate all parameters
            mediaState = renderHal->pfnAssignDynamicState(renderHal, &params, RENDERHAL_COMPONENT_CM);
            state->criticalSectionDSH->Release();
        }
    }
    else
    {
        // Assign a MediaState from the MediaStateHeap
        // !!!! THIS MUST BE BEFORE Getting the BATCH_BUFFER !!!
        // since this method syncs the batch buffer and media state.
        mediaState = renderHal->pfnAssignMediaState(renderHal, RENDERHAL_COMPONENT_CM);
    }
    CM_CHK_NULL_GOTOFINISH_MOSERROR(mediaState);

    // Assign/Reset SSH instance
    CM_CHK_MOSSTATUS_GOTOFINISH(renderHal->pfnAssignSshInstance(renderHal));

    if (state->useNewSamplerHeap == false)
    {
        HalCm_AcquireSamplerStatistics(state);
    }

    for (i = 0; i < execGroupParam->numKernels; i++)
    {
        CM_HAL_INDEX_PARAM indexParam;
        MOS_ZeroMemory(&indexParam, sizeof(CM_HAL_INDEX_PARAM));
        kernelParam = execGroupParam->kernels[i];

        CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_SetupStatesForKernelInitial(state, mediaState, nullptr, taskId, kernelParam, &indexParam,
            execGroupParam->kernelCurbeOffset[i], bti, mediaID, krnAllocations[i]));

        CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_FinishStatesForKernel(state, mediaState, nullptr, taskId, kernelParam, i, &indexParam,
            bti, mediaID, krnAllocations[i]));

        vfeCurbeSize += MOS_ALIGN_CEIL(kernelParam->totalCurbeSize, state->renderHal->dwCurbeBlockAlign);

        if (execGroupParam->conditionalEndBitmap & (uint64_t)1 << i)
        {
            CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_SetConditionalEndInfo(state, taskParam->conditionalEndInfo, taskParam->conditionalBBEndParams, i));
        }
    }

    // Store the Max Payload Sizes in the Task params
    state->taskParam->vfeCurbeSize = vfeCurbeSize;
    state->taskParam->urbEntrySize = 0;

    // We may have to send additional Binding table commands in command buffer.
    // This is needed because the surface offset (from the base on SSH)
    // calculation takes into account the max binding tables allocated in the
    // SSH.
    remBindingTables = renderHal->StateHeapSettings.iBindingTables - execGroupParam->numKernels;

    if (remBindingTables > 0)
    {
        for (i = 0; i < remBindingTables; i++)
        {
            CM_CHK_MOSSTATUS_GOTOFINISH(renderHal->pfnAssignBindingTable(
                    renderHal,
                    &bindingTable));
        }
    }

    // until now, we know binding table index for debug surface
    // let's get system thread
    if (execGroupParam->kernelDebugEnabled)
    {
        CM_CHK_MOSSTATUS_GOTOFINISH(state->pfnGetSipBinary(state));
    }

    osInterface = state->osInterface;
    osInterface->pfnResetPerfBufferID(osInterface);
    if (osInterface->pfnIsPerfTagSet(osInterface) == false)
    {
        osInterface->pfnIncPerfFrameID(osInterface);
        int perfTag = HalCm_GetKernelPerfTag(state, execGroupParam->kernels, execGroupParam->numKernels);
        osInterface->pfnSetPerfTag(osInterface, (uint16_t)perfTag);
    }

    // Submit HW commands and states
    CM_CHK_MOSSTATUS_GOTOFINISH(state->cmHalInterface->SubmitCommands(
                     nullptr, taskId, execGroupParam->kernels, &cmdBuffer));

    // Set the Task ID
    execGroupParam->taskIdOut = taskId;

    // Set OS data
    if(cmdBuffer)
    {
        execGroupParam->osData = cmdBuffer;
    }

    // Update the task ID table
    state->taskStatusTable[taskId] = (char)taskId;

finish:

    if (state->dshEnabled)
    {
        state->criticalSectionDSH->Acquire();
        if (mediaState && eStatus != MOS_STATUS_SUCCESS)
        {
            // Failed, release media state and heap resources
            renderHal->pfnReleaseDynamicState(renderHal, mediaState);
        }
        else
        {
            renderHal->pfnSubmitDynamicState(renderHal, mediaState);
        }
        state->criticalSectionDSH->Release();
    }

    return eStatus;
}

MOS_STATUS HalCm_ExecuteHintsTask(
    PCM_HAL_STATE                 state,                     // [in] Pointer to CM State
    PCM_HAL_EXEC_HINTS_TASK_PARAM execHintsParam)            // [in] Pointer to Task Param
{
    MOS_STATUS              eStatus;
    PRENDERHAL_INTERFACE    renderHal;
    PRENDERHAL_MEDIA_STATE  mediaState;
    PMHW_BATCH_BUFFER       batchBuffer;
    PCM_HAL_BB_ARGS         bbCmArgs;
    PCM_HAL_KERNEL_PARAM    kernelParam;
    uint32_t                i;
    uint32_t                numTasks;
    uint64_t                origKernelIds[CM_MAX_KERNELS_PER_TASK];
    int32_t                 taskId;
    int32_t                 remBindingTables;
    int32_t                 bindingTable;
    uint32_t                vfeCurbeSize;
    uint32_t                maxInlineDataSize;
    uint32_t                maxIndirectDataSize;
    int32_t                 *bindingTableEntries;
    int32_t                 *mediaIds;
    PRENDERHAL_KRN_ALLOCATION *krnAllocations;
    PCM_HAL_INDEX_PARAM     indexParams;
    bool                    useMediaObjects;
    void                    *cmdBuffer;
    bool                    splitTask;
    bool                    lastTask;
    PMOS_INTERFACE          osInterface = nullptr;

    //------------------------------------
    CM_ASSERT(state);
    CM_ASSERT(execHintsParam);
    //------------------------------------

    eStatus             = MOS_STATUS_SUCCESS;
    renderHal           = state->renderHal;
    mediaState          = nullptr;
    batchBuffer         = nullptr;
    bindingTableEntries = nullptr;
    mediaIds            = nullptr;
    krnAllocations      = nullptr;
    indexParams         = nullptr;
    useMediaObjects      = false;
    cmdBuffer           = nullptr;
    splitTask            = false;
    lastTask             = false;

    if (execHintsParam->numKernels > state->cmDeviceParam.maxKernelsPerTask)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        CM_ASSERTMESSAGE("Number of Kernels per task exceeds maximum");
        goto finish;
    }

    bindingTableEntries = (int*)MOS_AllocAndZeroMemory(sizeof(int)*execHintsParam->numKernels);
    mediaIds = (int*)MOS_AllocAndZeroMemory(sizeof(int)* execHintsParam->numKernels);
    krnAllocations = (PRENDERHAL_KRN_ALLOCATION *)MOS_AllocAndZeroMemory(sizeof(void *)* execHintsParam->numKernels);
    indexParams = (PCM_HAL_INDEX_PARAM)MOS_AllocAndZeroMemory(sizeof(CM_HAL_INDEX_PARAM)* execHintsParam->numKernels);
    if (!bindingTableEntries || !mediaIds || !krnAllocations || !indexParams)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        CM_ASSERTMESSAGE("Memory allocation failed in ExecuteHints Task");
        goto finish;
    }

    // check hints to see if need to split into multiple tasks
    numTasks = ( execHintsParam->hints & CM_HINTS_MASK_NUM_TASKS ) >> CM_HINTS_NUM_BITS_TASK_POS;
    if( numTasks > 1 )
    {
        splitTask = true;
    }

    MOS_FillMemory(bindingTableEntries, sizeof(int) * execHintsParam->numKernels, CM_INVALID_INDEX);
    MOS_FillMemory(mediaIds, sizeof(int) * execHintsParam->numKernels, CM_INVALID_INDEX);
    MOS_FillMemory(krnAllocations, sizeof(void *)* execHintsParam->numKernels, 0);

    // Reset states before execute
    // (clear allocations, get GSH allocation index + any additional housekeeping)
    state->osInterface->pfnResetOsStates(state->osInterface);
    CM_CHK_MOSSTATUS_GOTOFINISH(renderHal->pfnReset(renderHal));

    MOS_ZeroMemory(state->taskParam, sizeof(CM_HAL_TASK_PARAM));

    MOS_FillMemory(
        state->bti2DIndexTable,
        state->cmDeviceParam.max2DSurfaceTableSize * sizeof( CM_HAL_MULTI_USE_BTI_ENTRY ),
        CM_INVALID_INDEX );

    MOS_FillMemory(
        state->bti2DUPIndexTable,
        state->cmDeviceParam.max2DSurfaceUPTableSize * sizeof( CM_HAL_MULTI_USE_BTI_ENTRY ),
        CM_INVALID_INDEX );

    MOS_FillMemory(
        state->bti3DIndexTable,
        state->cmDeviceParam.max3DSurfaceTableSize * sizeof( CM_HAL_MULTI_USE_BTI_ENTRY ),
        CM_INVALID_INDEX );

    MOS_FillMemory(
        state->btiBufferIndexTable,
        state->cmDeviceParam.maxBufferTableSize * sizeof( CM_HAL_MULTI_USE_BTI_ENTRY ),
        CM_INVALID_INDEX );

    MOS_FillMemory(
        state->samplerIndexTable,
        state->cmDeviceParam.maxSamplerTableSize,
        CM_INVALID_INDEX);

    MOS_FillMemory(
        state->sampler8x8IndexTable,
        state->cmDeviceParam.maxSampler8x8TableSize,
        CM_INVALID_INDEX);

    state->walkerParams.CmWalkerEnable = 0;

    vfeCurbeSize = 0;
    maxInlineDataSize = 0;
    maxIndirectDataSize = 0;

    MOS_ZeroMemory(&origKernelIds, CM_MAX_KERNELS_PER_TASK * sizeof(uint64_t));

    // Get the Task Id
    CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_GetNewTaskId(state, &taskId));

    // Parse the task
    CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_ParseHintsTask(state, execHintsParam));

    // Assign a MediaState from the MediaStateHeap
    // !!!! THIS MUST BE BEFORE Getting the BATCH_BUFFER !!!
    // since this method syncs the batch buffer and media state.
    if (state->dshEnabled)
    {
        if ( execHintsParam->userDefinedMediaState != nullptr )
        {
            // use exsiting media state as current state
            mediaState = static_cast< PRENDERHAL_MEDIA_STATE >( execHintsParam->userDefinedMediaState );

            // update current state to dsh
            renderHal->pStateHeap->pCurMediaState = mediaState;
            // Refresh sync tag for all media states in submitted queue
            state->criticalSectionDSH->Acquire();
            renderHal->pfnRefreshSync( renderHal );
            state->criticalSectionDSH->Release();
        }
        else
        {
            // Obtain media state configuration - Curbe, Samplers (3d/AVS/VA), 8x8 sampler table, Media IDs, Kernel Spill area
            RENDERHAL_DYNAMIC_MEDIA_STATE_PARAMS params;
            state->criticalSectionDSH->Acquire();
            HalCm_DSH_GetDynamicStateConfiguration(state, &params, execHintsParam->numKernels, execHintsParam->kernels, execHintsParam->kernelCurbeOffset);

            // Prepare Media States to accommodate all parameters - Curbe, Samplers (3d/AVS/VA), 8x8 sampler table, Media IDs
            mediaState = renderHal->pfnAssignDynamicState(renderHal, &params, RENDERHAL_COMPONENT_CM);
            state->criticalSectionDSH->Release();
        }
    }
    else
    {
        mediaState = renderHal->pfnAssignMediaState(renderHal, RENDERHAL_COMPONENT_CM);
    }
    CM_CHK_NULL_GOTOFINISH_MOSERROR(mediaState);

    if (state->useNewSamplerHeap == false)
    {
        HalCm_AcquireSamplerStatistics(state);
    }

    // Assign/Reset SSH instance
    CM_CHK_MOSSTATUS_GOTOFINISH(renderHal->pfnAssignSshInstance(renderHal));

    if (!state->walkerParams.CmWalkerEnable)
    {
        if( splitTask )
        {
            // save original kernel IDs for kernel binary re-use in GSH
            for( i = 0; i < execHintsParam->numKernels; ++i )
            {
                origKernelIds[i] = execHintsParam->kernels[i]->kernelId;
            }

            // need to add tag to kernel IDs to distinguish batch buffer
            CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_AddKernelIDTag(execHintsParam->kernels, execHintsParam->numKernels, numTasks, execHintsParam->numTasksGenerated));
        }

        // Get the Batch buffer
        CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_GetBatchBuffer(state, execHintsParam->numKernels, execHintsParam->kernels, &batchBuffer));

        if( splitTask )
        {
            // restore kernel IDs for kernel binary re-use in GSH
            for( i = 0; i < execHintsParam->numKernels; ++i )
            {
                execHintsParam->kernels[i]->kernelId = origKernelIds[i];
            }
        }

        // Lock the batch buffer
        CM_CHK_NULL_GOTOFINISH_MOSERROR(batchBuffer->pPrivateData);
        bbCmArgs = (PCM_HAL_BB_ARGS)batchBuffer->pPrivateData;
        if ( (bbCmArgs->refCount == 1) ||
             ( state->taskParam->reuseBBUpdateMask == 1) )
        {
            CM_CHK_MOSSTATUS_GOTOFINISH(renderHal->pfnLockBB(renderHal, batchBuffer));
        }
    }

    // Load all kernels in the same state heap - expand ISH if necessary BEFORE programming media states.
    // This is better than having to expand ISH in the middle of loading, when part of MediaIDs are
    // already programmed - not a problem in the old implementation where it would simply remove old
    // kernels out of the way.
    if (state->dshEnabled)
    {
        CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_DSH_LoadKernelArray(state, execHintsParam->kernels, execHintsParam->numKernels, krnAllocations));
    }

    // 0: media walker
    // 1: media object
    if( (execHintsParam->hints & CM_HINTS_MASK_MEDIAOBJECT) == CM_HINTS_MASK_MEDIAOBJECT )
    {
        for (i = 0; i < execHintsParam->numKernels; ++i)
        {
            CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_SetupStatesForKernelInitial(state, mediaState, batchBuffer, taskId, execHintsParam->kernels[i], &indexParams[i],
                execHintsParam->kernelCurbeOffset[i], bindingTableEntries[i], mediaIds[i], krnAllocations[i]));
        }

        CM_CHK_NULL_GOTOFINISH_MOSERROR(batchBuffer);

        CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_FinishStatesForKernelMix(state, batchBuffer, taskId, execHintsParam->kernels,
            indexParams, bindingTableEntries, mediaIds, krnAllocations, execHintsParam->numKernels, execHintsParam->hints, execHintsParam->isLastTask));

        for( i = 0; i < execHintsParam->numKernels; ++i)
        {
            kernelParam = execHintsParam->kernels[i];
            vfeCurbeSize += MOS_ALIGN_CEIL(kernelParam->totalCurbeSize, state->renderHal->dwCurbeBlockAlign);
            if( kernelParam->payloadSize > maxInlineDataSize)
            {
                maxInlineDataSize = kernelParam->payloadSize;
            }
            if( kernelParam->indirectDataParam.indirectDataSize > maxIndirectDataSize )
            {
                maxIndirectDataSize = kernelParam->indirectDataParam.indirectDataSize;
            }
        }

        // Store the Max Payload Sizes in the Task Param
        state->taskParam->vfeCurbeSize = vfeCurbeSize;
        if( maxIndirectDataSize)
        {
            state->taskParam->vfeCurbeSize = maxIndirectDataSize;
        }
        else
        {
            state->taskParam->urbEntrySize = maxInlineDataSize;
        }

        // We may have to send additional Binding table commands in command buffer.
        // This is needed because the surface offset (from the base on SSH)
        // calculation takes into account the max binding tables allocated in the
        // SSH.
        remBindingTables = state->cmDeviceParam.maxKernelsPerTask -
            execHintsParam->numKernels;

        if( remBindingTables > 0)
        {
            for( i = 0; i < (uint32_t)remBindingTables; ++i)
            {
                CM_CHK_MOSSTATUS_GOTOFINISH(renderHal->pfnAssignBindingTable(
                    renderHal,
                    &bindingTable));
            }
        }

        osInterface = state->osInterface;
        osInterface->pfnResetPerfBufferID(osInterface);
        if (osInterface->pfnIsPerfTagSet(osInterface) == false)
        {
            osInterface->pfnIncPerfFrameID(osInterface);
            int perfTag = HalCm_GetKernelPerfTag(state, execHintsParam->kernels, execHintsParam->numKernels);
            osInterface->pfnSetPerfTag(osInterface, (uint16_t)perfTag);
        }

        // Submit HW commands and states
        CM_CHK_MOSSTATUS_GOTOFINISH(state->cmHalInterface->SubmitCommands(
                        batchBuffer, taskId, execHintsParam->kernels, &cmdBuffer));

        // Set the Task ID
        execHintsParam->taskIdOut = taskId;

        // Set OS data
        if( cmdBuffer )
        {
            execHintsParam->osData = cmdBuffer;
        }

        // Update the task ID table
        state->taskStatusTable[taskId] = (char)taskId;
    }
    else
    {
        // use media walker
        // unimplemented for now
        CM_ASSERTMESSAGE("Error: Media walker is not supported.");
        eStatus = MOS_STATUS_UNKNOWN;
    }

finish:

    if (state->dshEnabled)
    {
        state->criticalSectionDSH->Acquire();
        if (mediaState && eStatus != MOS_STATUS_SUCCESS)
        {
            // Failed, release media state and heap resources
            renderHal->pfnReleaseDynamicState(renderHal, mediaState);
        }
        else
        {
            renderHal->pfnSubmitDynamicState(renderHal, mediaState);
        }
        state->criticalSectionDSH->Release();
    }

    if (batchBuffer) // for MediaWalker, batchBuffer is empty
    {
        if (batchBuffer->bLocked)
        {
            // Only happens in Error cases
            if (batchBuffer->pPrivateData && ((PCM_HAL_BB_ARGS)batchBuffer->pPrivateData)->refCount == 1)
            {
                renderHal->pfnUnlockBB(renderHal, batchBuffer);
            }
            else if (batchBuffer->pPrivateData == nullptr)
            {
                eStatus = MOS_STATUS_NULL_POINTER;
            }
        }
    }

    // free memory
    if( bindingTableEntries )          MOS_FreeMemory(bindingTableEntries);
    if( mediaIds )                     MOS_FreeMemory(mediaIds);
    if( krnAllocations )               MOS_FreeMemory(krnAllocations);
    if( indexParams )                  MOS_FreeMemory( indexParams );

    return eStatus;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Send Commands to HW
//| Returns:    Get the HAL Max values
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_GetMaxValues(
    PCM_HAL_STATE           state,                                             // [in]  Pointer to CM State
    PCM_HAL_MAX_VALUES      maxValues)                                         // [out] Pointer to Max values
{
    PRENDERHAL_INTERFACE  renderHal;

    renderHal = state->renderHal;

    maxValues->maxTasks                         = state->cmDeviceParam.maxTasks;
    maxValues->maxKernelsPerTask                = CM_MAX_KERNELS_PER_TASK;
    maxValues->maxKernelBinarySize              = state->cmDeviceParam.maxKernelBinarySize;
    maxValues->maxSpillSizePerHwThread          = state->cmDeviceParam.maxPerThreadScratchSpaceSize;
    maxValues->maxSamplerTableSize              = CM_MAX_SAMPLER_TABLE_SIZE;
    maxValues->maxBufferTableSize               = CM_MAX_BUFFER_SURFACE_TABLE_SIZE;
    maxValues->max2DSurfaceTableSize            = CM_MAX_2D_SURFACE_TABLE_SIZE;
    maxValues->max3DSurfaceTableSize            = CM_MAX_3D_SURFACE_TABLE_SIZE;
    maxValues->maxArgsPerKernel                 = CM_MAX_ARGS_PER_KERNEL;
    maxValues->maxUserThreadsPerTask            = CM_MAX_USER_THREADS;
    maxValues->maxUserThreadsPerTaskNoThreadArg = CM_MAX_USER_THREADS_NO_THREADARG;
    maxValues->maxArgByteSizePerKernel          = CM_MAX_ARG_BYTE_PER_KERNEL;
    maxValues->maxSurfacesPerKernel             = renderHal->pHwCaps->dwMaxBTIndex;
    maxValues->maxSamplersPerKernel             = renderHal->pHwCaps->dwMaxUnormSamplers;
    maxValues->maxHwThreads                     = renderHal->pHwCaps->dwMaxThreads;

    return MOS_STATUS_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get the HAL Max extended values
//| Returns:    Get the HAL Max extended values
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_GetMaxValuesEx(
    PCM_HAL_STATE           state,                                             // [in]  Pointer to CM State
    PCM_HAL_MAX_VALUES_EX   maxValuesEx)                                       // [out] Pointer to extended Max values
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;
    maxValuesEx->max2DUPSurfaceTableSize = CM_MAX_2D_SURFACE_UP_TABLE_SIZE;
    maxValuesEx->maxSampler8x8TableSize = CM_MAX_SAMPLER_8X8_TABLE_SIZE;
    maxValuesEx->maxCURBESizePerKernel = CM_MAX_CURBE_SIZE_PER_KERNEL;
    maxValuesEx->maxCURBESizePerTask = CM_MAX_CURBE_SIZE_PER_TASK;
    maxValuesEx->maxIndirectDataSizePerKernel = CM_MAX_INDIRECT_DATA_SIZE_PER_KERNEL;

    //MaxThreadWidth x MaxThreadHeight x ColorCount
    maxValuesEx->maxUserThreadsPerMediaWalker = \
                            state->cmHalInterface->GetMediaWalkerMaxThreadWidth()* \
                            state->cmHalInterface->GetMediaWalkerMaxThreadHeight() * \
                            CM_THREADSPACE_MAX_COLOR_COUNT;

    CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_GetMaxThreadCountPerThreadGroup( state, &maxValuesEx->maxUserThreadsPerThreadGroup ) );

finish:
    return eStatus;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Register Sampler
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_RegisterSampler(
    PCM_HAL_STATE           state,                                             // [in]  Pointer to CM State
    PCM_HAL_SAMPLER_PARAM   param)                                             // [in]  Pointer to Sampler Param
{
    MOS_STATUS              eStatus;
    PMHW_SAMPLER_STATE_PARAM entry;
    uint32_t                i;

    eStatus      = MOS_STATUS_SUCCESS;
    entry  = nullptr;

    // Find a free slot
    for (i = 0; i < state->cmDeviceParam.maxSamplerTableSize; i++)
    {
        if (!state->samplerTable[i].bInUse)
        {
            entry              = &state->samplerTable[i];
            param->handle      = (uint32_t)i;
            break;
        }
    }

    if (!entry)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        CM_ASSERTMESSAGE("Sampler table is full");
        goto finish;
    }

    entry->SamplerType  = MHW_SAMPLER_TYPE_3D;
    if (state->useNewSamplerHeap == true)
    {
        entry->ElementType = MHW_Sampler1Element;
    }
    else
    {
        entry->ElementType = MHW_Sampler4Elements;
    }
    CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_GetGfxMapFilter(param->minFilter,  &entry->Unorm.MinFilter));
    CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_GetGfxMapFilter(param->magFilter,  &entry->Unorm.MagFilter));
    CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_GetGfxTextAddress(param->addressU, &entry->Unorm.AddressU));
    CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_GetGfxTextAddress(param->addressV, &entry->Unorm.AddressV));
    CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_GetGfxTextAddress(param->addressW, &entry->Unorm.AddressW));

    entry->Unorm.SurfaceFormat = (MHW_SAMPLER_SURFACE_PIXEL_TYPE)param->surfaceFormat;
    switch (entry->Unorm.SurfaceFormat)
    {
        case MHW_SAMPLER_SURFACE_PIXEL_UINT:
            entry->Unorm.BorderColorRedU = param->borderColorRedU;
            entry->Unorm.BorderColorGreenU = param->borderColorGreenU;
            entry->Unorm.BorderColorBlueU = param->borderColorBlueU;
            entry->Unorm.BorderColorAlphaU = param->borderColorAlphaU;
            break;
        case MHW_SAMPLER_SURFACE_PIXEL_SINT:
            entry->Unorm.BorderColorRedS = param->borderColorRedS;
            entry->Unorm.BorderColorGreenS = param->borderColorGreenS;
            entry->Unorm.BorderColorBlueS = param->borderColorBlueS;
            entry->Unorm.BorderColorAlphaS = param->borderColorAlphaS;
            break;
        default:
            entry->Unorm.BorderColorRedF = param->borderColorRedF;
            entry->Unorm.BorderColorGreenF = param->borderColorGreenF;
            entry->Unorm.BorderColorBlueF = param->borderColorBlueF;
            entry->Unorm.BorderColorAlphaF = param->borderColorAlphaF;
    }
    entry->Unorm.bBorderColorIsValid = true;

    entry->bInUse = true;

finish:
    return eStatus;
}

//*-----------------------------------------------------------------------------
//| Purpose:    UnRegister Sampler
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_UnRegisterSampler(
    PCM_HAL_STATE               state,                                         // [in]  Pointer to CM State
    uint32_t                    handle)                                       // [in]  Pointer to Sampler Param
{
    MOS_STATUS              eStatus;
    PMHW_SAMPLER_STATE_PARAM entry;

    eStatus = MOS_STATUS_SUCCESS;

    if (handle >= state->cmDeviceParam.maxSamplerTableSize)
    {
        eStatus = MOS_STATUS_INVALID_HANDLE;
        CM_ASSERTMESSAGE("Invalid handle '%d'", handle);
        goto finish;
    }

    entry = &state->samplerTable[handle];

    // need to clear the state entirely instead of just setting bInUse to false
    MOS_ZeroMemory(entry, sizeof(MHW_SAMPLER_STATE_PARAM));

finish:
    return eStatus;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Register Sampler8x8
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_RegisterSampler8x8(
    PCM_HAL_STATE                state,
    PCM_HAL_SAMPLER_8X8_PARAM    param)
{
    return state->cmHalInterface->RegisterSampler8x8(param);
}

//*-----------------------------------------------------------------------------
//| Purpose:    UnRegister Sampler
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_UnRegisterSampler8x8(
    PCM_HAL_STATE               state,                                         // [in]  Pointer to CM State
    uint32_t                    handle)                                       // [in]  Pointer to Sampler8x8 Param
{
    MOS_STATUS                  eStatus;
    uint32_t                    index8x8;
    PMHW_SAMPLER_STATE_PARAM    entry;
    PCM_HAL_SAMPLER_8X8_ENTRY   sampler8x8Entry;

    eStatus = MOS_STATUS_SUCCESS;

    if (handle >= state->cmDeviceParam.maxSamplerTableSize) {
        eStatus = MOS_STATUS_INVALID_HANDLE;
        CM_ASSERTMESSAGE("Invalid handle '%d'", handle);
        goto finish;
    }

    entry = &state->samplerTable[handle];
    entry->bInUse = false;

    if ( entry->SamplerType == MHW_SAMPLER_TYPE_AVS )
    {
        index8x8 = entry->Avs.stateID;
        if ( index8x8 >= state->cmDeviceParam.maxSampler8x8TableSize )
        {
            eStatus = MOS_STATUS_INVALID_HANDLE;
            CM_ASSERTMESSAGE( "Invalid 8x8 handle '%d'", handle );
            goto finish;
        }

        sampler8x8Entry = &state->sampler8x8Table[ index8x8 ];
    sampler8x8Entry->inUse = false;
    }

    // need to clear the state entirely instead of just setting bInUse to false
    MOS_ZeroMemory(entry, sizeof(MHW_SAMPLER_STATE_PARAM));
finish:
    return eStatus;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Frees the buffer and removes from the table
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_FreeBuffer(
    PCM_HAL_STATE           state,                                             // [in]  Pointer to CM State
    uint32_t                handle)                                           // [in]  Pointer to Buffer Param
{
    MOS_STATUS              eStatus;
    PCM_HAL_BUFFER_ENTRY    entry;
    PMOS_INTERFACE          osInterface;

    eStatus        = MOS_STATUS_SUCCESS;
    osInterface    = state->osInterface;

    // Get the Buffer Entry
    CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_GetBufferEntry(state, handle, &entry));

    if (state->advExecutor)
    {
        state->advExecutor->DeleteBufferStateMgr(entry->surfStateMgr);
    }
    if (entry->isAllocatedbyCmrtUmd)
    {
        osInterface->pfnFreeResourceWithFlag(osInterface, &entry->osResource, SURFACE_FLAG_ASSUME_NOT_IN_USE);
    }
    else
    {
        HalCm_OsResource_Unreference(&entry->osResource);
    }
    osInterface->pfnResetResourceAllocationIndex(osInterface, &entry->osResource);
    entry->size = 0;
    entry->address = nullptr;

finish:
    return eStatus;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Set surface read flag used in on demand sync
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_SetSurfaceReadFlag(
    PCM_HAL_STATE           state,                                             // [in]  Pointer to CM State
    uint32_t                handle,                                           // [in]  index of surface 2d
    bool                    readSync,
    MOS_GPU_CONTEXT         gpuContext)
{
    MOS_STATUS                 eStatus  = MOS_STATUS_SUCCESS;
    PCM_HAL_SURFACE2D_ENTRY    entry;

    // Get the Buffer Entry
    CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_GetSurface2DEntry(state, handle, &entry));

    if (HalCm_IsValidGpuContext(gpuContext))
    {
        entry->readSyncs[gpuContext] = readSync;
        if (state->advExecutor)
        {
            state->advExecutor->Set2DRenderTarget(entry->surfStateMgr, !readSync);
        }
    }
    else
    {
        return MOS_STATUS_UNKNOWN;
    }

finish:
    return eStatus;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Read the data from buffer and return
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_LockBuffer(
    PCM_HAL_STATE           state,                                             // [in]  Pointer to CM State
    PCM_HAL_BUFFER_PARAM    param)                                             // [in]  Pointer to Buffer Param
{
    MOS_STATUS              eStatus;
    PCM_HAL_BUFFER_ENTRY    entry;
    PMOS_INTERFACE          osInterface;
    MOS_LOCK_PARAMS         lockFlags;
    eStatus        = MOS_STATUS_SUCCESS;
    osInterface    = state->osInterface;

    CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_GetBufferEntry(state, param->handle, &entry));
    if ((param->lockFlag != CM_HAL_LOCKFLAG_READONLY) && (param->lockFlag != CM_HAL_LOCKFLAG_WRITEONLY) )
    {
        eStatus = MOS_STATUS_INVALID_HANDLE;
        CM_ASSERTMESSAGE("Invalid lock flag!");
        eStatus = MOS_STATUS_UNKNOWN;
        goto finish;
    }

    CM_CHK_HRESULT_GOTOFINISH_MOSERROR(
        osInterface->pfnRegisterResource(osInterface, &entry->osResource, true,
                                         true));

    // Lock the resource
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));

    if (param->lockFlag == CM_HAL_LOCKFLAG_READONLY)
    {
        lockFlags.ReadOnly = true;
    }
    else
    {
        lockFlags.WriteOnly = true;
    }

    lockFlags.ForceCached = true;
    param->data = osInterface->pfnLockResource(
                    osInterface,
                    &entry->osResource,
                    &lockFlags);
    CM_CHK_NULL_GOTOFINISH_MOSERROR(param->data);

finish:
    return eStatus;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Writes the data to buffer
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_UnlockBuffer(
    PCM_HAL_STATE           state,                                             // [in]  Pointer to CM State
    PCM_HAL_BUFFER_PARAM    param)                                             // [in]  Pointer to Buffer Param
{
    MOS_STATUS              eStatus;
    PCM_HAL_BUFFER_ENTRY    entry;
    PMOS_INTERFACE          osInterface;

    eStatus        = MOS_STATUS_SUCCESS;
    osInterface    = state->osInterface;

    CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_GetBufferEntry(state, param->handle, &entry));

    CM_CHK_HRESULT_GOTOFINISH_MOSERROR(osInterface->pfnUnlockResource(osInterface, &entry->osResource));

finish:
    return eStatus;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Frees the buffer and removes from the table
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_FreeSurface2DUP(
    PCM_HAL_STATE           state,                                             // [in]  Pointer to CM State
    uint32_t                handle)                                           // [in]  Pointer to Buffer Param
{
    MOS_STATUS                    eStatus;
    PCM_HAL_SURFACE2D_UP_ENTRY    entry;
    PMOS_INTERFACE          osInterface;

    eStatus        = MOS_STATUS_SUCCESS;
    osInterface    = state->osInterface;

    // Get the Buffer Entry
    CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_GetResourceUPEntry(state, handle, &entry));

    if (state->advExecutor)
    {
        state->advExecutor->Delete2Dor3DStateMgr(entry->surfStateMgr);
    }

    osInterface->pfnFreeResourceWithFlag(osInterface, &entry->osResource, SURFACE_FLAG_ASSUME_NOT_IN_USE);

    osInterface->pfnResetResourceAllocationIndex(osInterface, &entry->osResource);
    entry->width = 0;

finish:
    return eStatus;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get 2D surface pitch and physical size
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_GetSurface2DTileYPitch(
    PCM_HAL_STATE                state,                                             // [in]  Pointer to CM State
    PCM_HAL_SURFACE2D_PARAM      param)                                        // [in]  Pointer to Buffer Param
{
    MOS_STATUS                  eStatus;
    MOS_SURFACE                 surface;
    PRENDERHAL_INTERFACE        renderHal;
    uint32_t                    index;
    RENDERHAL_GET_SURFACE_INFO  info;

    //-----------------------------------------------
    CM_ASSERT(state);
    //-----------------------------------------------

    eStatus       = MOS_STATUS_UNKNOWN;
    renderHal     = state->renderHal;
    index         = param->handle;

    // Get Details of 2D surface and fill the surface
    MOS_ZeroMemory(&surface, sizeof(surface));

    surface.OsResource  = state->umdSurf2DTable[index].osResource;
    surface.dwWidth     = state->umdSurf2DTable[index].width;
    surface.dwHeight    = state->umdSurf2DTable[index].height;
    surface.Format      = state->umdSurf2DTable[index].format;
    surface.dwDepth     = 1;

    MOS_ZeroMemory(&info, sizeof(RENDERHAL_GET_SURFACE_INFO));

    CM_CHK_MOSSTATUS_GOTOFINISH(RenderHal_GetSurfaceInfo(
        state->osInterface,
        &info,
        &surface));

    param->pitch      = surface.dwPitch;

finish:
    return eStatus;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Sets width and height values for 2D surface state
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_Set2DSurfaceStateParam(
     PCM_HAL_STATE                            state,
     PCM_HAL_SURFACE2D_SURFACE_STATE_PARAM    param,
     uint32_t                                 aliasIndex,
     uint32_t                                 handle)
{
    MOS_STATUS eStatus;
    uint32_t width;
    uint32_t height;

    CM_CHK_NULL_GOTOFINISH_MOSERROR(state);
    CM_CHK_NULL_GOTOFINISH_MOSERROR(param);

    eStatus     = MOS_STATUS_SUCCESS;
    if (aliasIndex < state->surfaceArraySize)
    {
        state->umdSurf2DTable[handle].surfStateSet = true;
    }
    state->umdSurf2DTable[handle].surfaceStateParam[aliasIndex / state->surfaceArraySize] = *param;

finish:
    return eStatus;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Sets width and height values for 2D surface state
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_SetBufferSurfaceStateParameters(
     PCM_HAL_STATE                            state,
     PCM_HAL_BUFFER_SURFACE_STATE_PARAM       param)
{
    MOS_STATUS eStatus;
    uint32_t size;
    uint32_t offset;
    uint32_t index;
    uint32_t aliasIndex;

    CM_CHK_NULL_GOTOFINISH_MOSERROR(state);
    CM_CHK_NULL_GOTOFINISH_MOSERROR(param);

    eStatus     = MOS_STATUS_SUCCESS;
    index = param->handle;
    aliasIndex = param->aliasIndex;

    if (aliasIndex < state->surfaceArraySize)
        state->bufferTable[index].surfStateSet = true;

    state->bufferTable[index].surfaceStateEntry[aliasIndex / state->surfaceArraySize].surfaceStateSize = param->size;
    state->bufferTable[index].surfaceStateEntry[aliasIndex / state->surfaceArraySize].surfaceStateOffset = param->offset;
    state->bufferTable[index].surfaceStateEntry[aliasIndex / state->surfaceArraySize].surfaceStateMOCS = param->mocs;

finish:
    return eStatus;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Sets mocs value for surface
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_SetSurfaceMOCS(
     PCM_HAL_STATE                  state,
     uint32_t                       handle,
     uint16_t                       mocs,
     uint32_t                       argKind)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    switch (argKind)
    {
        case CM_ARGUMENT_SURFACEBUFFER:
            state->bufferTable[handle].memObjCtl = mocs;
            if (state->advExecutor)
            {
                state->advExecutor->SetBufferMemoryObjectControl(state->bufferTable[handle].surfStateMgr, mocs);
            }
            break;
        case CM_ARGUMENT_SURFACE2D:
        case CM_ARGUMENT_SURFACE2D_SAMPLER:
        case CM_ARGUMENT_SURFACE_SAMPLER8X8_AVS:
        case CM_ARGUMENT_SURFACE_SAMPLER8X8_VA:
            state->umdSurf2DTable[handle].memObjCtl = mocs;
            if (state->advExecutor)
            {
                state->advExecutor->Set2Dor3DMemoryObjectControl(state->umdSurf2DTable[handle].surfStateMgr, mocs);
            }
            break;
        case CM_ARGUMENT_SURFACE2D_UP:
        case CM_ARGUMENT_SURFACE2DUP_SAMPLER:
            state->surf2DUPTable[handle].memObjCtl = mocs;
            if (state->advExecutor)
            {
                state->advExecutor->Set2Dor3DMemoryObjectControl(state->surf2DUPTable[handle].surfStateMgr, mocs);
            }
            break;
        case CM_ARGUMENT_SURFACE3D:
            state->surf3DTable[handle].memObjCtl = mocs;
            if (state->advExecutor)
            {
                state->advExecutor->Set2Dor3DMemoryObjectControl(state->surf3DTable[handle].surfStateMgr, mocs);
            }
            break;
        default:
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            CM_ASSERTMESSAGE("Invalid argument type in MOCS settings");
            goto finish;
    }

finish:
    return eStatus;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Allocate surface 2D
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_AllocateSurface2D(
    PCM_HAL_STATE                state,                                             // [in]  Pointer to CM State
    PCM_HAL_SURFACE2D_PARAM      param)                                             // [in]  Pointer to surface 2D Param
{
    MOS_STATUS              eStatus;
    PMOS_INTERFACE          osInterface;
    PCM_HAL_SURFACE2D_ENTRY entry = nullptr;
    MOS_ALLOC_GFXRES_PARAMS allocParams;
    uint32_t                i;

    //-----------------------------------------------
    CM_ASSERT(param->width > 0);
    //-----------------------------------------------

    eStatus              = MOS_STATUS_SUCCESS;
    osInterface    = state->osInterface;

    // Find a free slot
    for (i = 0; i < state->cmDeviceParam.max2DSurfaceTableSize; i++)
    {
        if(Mos_ResourceIsNull(&state->umdSurf2DTable[i].osResource))
        {
            entry              = &state->umdSurf2DTable[i];
            param->handle      = (uint32_t)i;
            break;
        }
    }

    if (!entry)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        CM_ASSERTMESSAGE("Surface2D table is full");
        goto finish;
    }

    if(param->isAllocatedbyCmrtUmd)
    {
        MOS_ZeroMemory(&allocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParams.Type          = MOS_GFXRES_2D;
        allocParams.dwWidth       = param->width;
        allocParams.dwHeight      = param->height;
        allocParams.pSystemMemory = param->data;
        allocParams.Format        = param->format;
        allocParams.TileType      = MOS_TILE_Y;
        allocParams.pBufName      = "CmSurface2D";

        CM_CHK_HRESULT_GOTOFINISH_MOSERROR(osInterface->pfnAllocateResource(
            osInterface,
            &allocParams,
            &entry->osResource));

        entry->width                  = param->width;
        entry->height                 = param->height;
        entry->format                  = param->format;
        entry->isAllocatedbyCmrtUmd    = param->isAllocatedbyCmrtUmd;
    }
    else
    {
        entry->width  = param->width;
        entry->height = param->height;
        entry->format  = param->format;
        entry->isAllocatedbyCmrtUmd = false;
        entry->osResource = *param->mosResource;
        HalCm_OsResource_Reference(&entry->osResource);
    }
    // set default CM MOS usage
    entry->memObjCtl = (state->cmHalInterface->GetDefaultMOCS()) << 8;

    if (state->advExecutor)
    {
        entry->surfStateMgr = state->advExecutor->Create2DStateMgr(&entry->osResource);
        state->advExecutor->Set2Dor3DOrigFormat(entry->surfStateMgr, entry->format);
        state->advExecutor->Set2Dor3DOrigDimension(entry->surfStateMgr,
                                                 entry->width,
                                                 entry->height,
                                                 0); // no need to change depth in 2D surface
    }
   
    for (int i = 0; i < CM_HAL_GPU_CONTEXT_COUNT; i++)
    {
        entry->readSyncs[i] = false;
    }

finish:
    return eStatus;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Allocate surface 2D
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_UpdateSurface2D(
    PCM_HAL_STATE                state,                                             // [in]  Pointer to CM State
    PCM_HAL_SURFACE2D_PARAM      param)                                             // [in]  Pointer to surface 2D Param
{
    MOS_STATUS              hr;
    PMOS_INTERFACE          osInterface;
    PCM_HAL_SURFACE2D_ENTRY entry = nullptr;
    MOS_ALLOC_GFXRES_PARAMS allocParams;
    uint32_t                i = param->handle;

    //-----------------------------------------------
    CM_ASSERT(param->width > 0);
    //-----------------------------------------------

    hr              = MOS_STATUS_SUCCESS;
    osInterface    = state->osInterface;

    entry = &state->umdSurf2DTable[i];

    HalCm_OsResource_Unreference(&entry->osResource);

    entry->width  = param->width;
    entry->height = param->height;
    entry->format  = param->format;
    entry->isAllocatedbyCmrtUmd = false;
    entry->osResource = *param->mosResource;

    HalCm_OsResource_Reference(&entry->osResource);

    if (state->advExecutor)
    {
        state->advExecutor->Delete2Dor3DStateMgr(entry->surfStateMgr);
        entry->surfStateMgr = state->advExecutor->Create2DStateMgr(&entry->osResource);
        state->advExecutor->Set2Dor3DOrigFormat(entry->surfStateMgr, entry->format);
        state->advExecutor->Set2Dor3DOrigDimension(entry->surfStateMgr,
                                                 entry->width,
                                                 entry->height,
                                                 0); // no need to change depth in 2D surface
    }
    
    for (int i = 0; i < CM_HAL_GPU_CONTEXT_COUNT; i++)
    {
        entry->readSyncs[i] = false;
    }

    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Allocate Linear Buffer or BufferUP
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_UpdateBuffer(
    PCM_HAL_STATE           state,                                             // [in]  Pointer to CM State
    PCM_HAL_BUFFER_PARAM    param)                                             // [in]  Pointer to Buffer Param
{
    MOS_STATUS              hr;
    PMOS_INTERFACE          osInterface;
    PCM_HAL_BUFFER_ENTRY    entry = nullptr;
    MOS_ALLOC_GFXRES_PARAMS allocParams;
    uint32_t                i = param->handle;
    PMOS_RESOURCE           osResource;

    //-----------------------------------------------
    CM_ASSERT(param->size > 0);
    //-----------------------------------------------

    hr              = MOS_STATUS_SUCCESS;
    osInterface    = state->renderHal->pOsInterface;

    entry              = &state->bufferTable[i];

    HalCm_OsResource_Unreference(&entry->osResource);
    entry->osResource = *param->mosResource;
    HalCm_OsResource_Reference(&entry->osResource);

    entry->size = param->size;
    entry->isAllocatedbyCmrtUmd = false;
    entry->surfaceStateEntry[0].surfaceStateSize = entry->size;
    entry->surfaceStateEntry[0].surfaceStateOffset = 0;
    entry->surfaceStateEntry[0].surfaceStateMOCS = 0;

    if (state->advExecutor)
    {
        state->advExecutor->DeleteBufferStateMgr(entry->surfStateMgr);
        entry->surfStateMgr = state->advExecutor->CreateBufferStateMgr(&entry->osResource);
        state->advExecutor->SetBufferOrigSize(entry->surfStateMgr, entry->size);
    }
    
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Frees the surface 2D and removes from the table
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_FreeSurface2D(
    PCM_HAL_STATE           state,                                             // [in]  Pointer to CM State
    uint32_t                handle)                                           // [in]  Pointer to Buffer Param
{
    MOS_STATUS                 eStatus;
    PCM_HAL_SURFACE2D_ENTRY    entry;
    PMOS_INTERFACE             osInterface;

    eStatus              = MOS_STATUS_SUCCESS;
    osInterface    = state->osInterface;

    // Get the Buffer Entry
    CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_GetSurface2DEntry(state, handle, &entry));
    if (state->advExecutor)
    {
        state->advExecutor->Delete2Dor3DStateMgr(entry->surfStateMgr);
    }
    
    if(entry->isAllocatedbyCmrtUmd)
    {
        osInterface->pfnFreeResourceWithFlag(osInterface, &entry->osResource, SURFACE_FLAG_ASSUME_NOT_IN_USE);
    }
    else
    {
        HalCm_OsResource_Unreference(&entry->osResource);
    }

    MOS_ZeroMemory(&entry->osResource, sizeof(entry->osResource));

    entry->width = 0;
    entry->height = 0;
    entry->frameType = CM_FRAME;

    for (int i = 0; i < CM_HAL_GPU_CONTEXT_COUNT; i++)
    {
        entry->readSyncs[i] = false;
    }

finish:
    return eStatus;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Allocate 3D resource
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_AllocateSurface3D(CM_HAL_STATE *state, // [in]  Pointer to CM State
                                   CM_HAL_3DRESOURCE_PARAM *param) // [in]  Pointer to Buffer Param)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    //-----------------------------------------------
    CM_ASSERT(state);
    CM_ASSERT(param->depth  > 1);
    CM_ASSERT(param->width  > 0);
    CM_ASSERT(param->height > 0);
    //-----------------------------------------------

    // Finds a free slot.
    CM_HAL_3DRESOURCE_ENTRY *entry = nullptr;
    for (uint32_t i = 0; i < state->cmDeviceParam.max3DSurfaceTableSize; i++)
    {
        if (Mos_ResourceIsNull(&state->surf3DTable[i].osResource))
        {
            entry = &state->surf3DTable[i];
            param->handle = (uint32_t)i;
            break;
        }
    }
    if (!entry)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        CM_ASSERTMESSAGE("3D surface table is full");
        return eStatus;
    }
    Mos_ResetResource(&entry->osResource);  // Resets the Resource

    MOS_ALLOC_GFXRES_PARAMS alloc_params;
    MOS_ZeroMemory(&alloc_params, sizeof(alloc_params));
    alloc_params.Type          = MOS_GFXRES_VOLUME;
    alloc_params.TileType      = MOS_TILE_Y;
    alloc_params.dwWidth       = param->width;
    alloc_params.dwHeight      = param->height;
    alloc_params.dwDepth       = param->depth;
    alloc_params.pSystemMemory = param->data;
    alloc_params.Format        = param->format;
    alloc_params.pBufName      = "CmSurface3D";

    MOS_INTERFACE *osInterface = state->renderHal->pOsInterface;
    CM_CHK_HRESULT_GOTOFINISH_MOSERROR(osInterface->pfnAllocateResource(
        osInterface,
        &alloc_params,
        &entry->osResource));
    entry->width = param->width;
    entry->height = param->height;
    entry->depth = param->depth;
    entry->format = param->format;

    if (state->advExecutor)
    {
        entry->surfStateMgr = state->advExecutor->Create3DStateMgr(&entry->osResource);
        state->advExecutor->Set2Dor3DOrigDimension(entry->surfStateMgr,
                                                 entry->width,
                                                 entry->height,
                                                 entry->depth);
    }
finish:
    return eStatus;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Frees the resource and removes from the table
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_Free3DResource(
    PCM_HAL_STATE           state,                                             // [in]  Pointer to CM State
    uint32_t                handle)                                           // [in]  Pointer to Buffer Param
{
    MOS_STATUS               eStatus;
    PCM_HAL_3DRESOURCE_ENTRY entry;
    PMOS_INTERFACE           osInterface;

    eStatus              = MOS_STATUS_SUCCESS;
    osInterface    = state->osInterface;

    // Get the Buffer Entry
    CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_Get3DResourceEntry(state, handle, &entry));
    if (state->advExecutor)
    {
        state->advExecutor->Delete2Dor3DStateMgr(entry->surfStateMgr);
    }

    osInterface->pfnFreeResourceWithFlag(osInterface, &entry->osResource, SURFACE_FLAG_ASSUME_NOT_IN_USE);

    osInterface->pfnResetResourceAllocationIndex(osInterface, &entry->osResource);

finish:
    return eStatus;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Lock the resource and return
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_Lock3DResource(
    PCM_HAL_STATE               state,                                         // [in]  Pointer to CM State
    PCM_HAL_3DRESOURCE_PARAM    param)                                         // [in]  Pointer to 3D Param
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;
    PCM_HAL_3DRESOURCE_ENTRY    entry;
    MOS_LOCK_PARAMS             lockFlags;
    RENDERHAL_GET_SURFACE_INFO  info;
    PMOS_INTERFACE              osInterface = nullptr;
    MOS_SURFACE                 surface;

    // Get the 3D Resource Entry
    CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_Get3DResourceEntry(state, param->handle, &entry));
    if ((param->lockFlag != CM_HAL_LOCKFLAG_READONLY) && (param->lockFlag != CM_HAL_LOCKFLAG_WRITEONLY) )
    {
        CM_ASSERTMESSAGE("Invalid lock flag!");
        eStatus = MOS_STATUS_UNKNOWN;
        goto finish;
    }

    // Get resource information
    MOS_ZeroMemory(&surface, sizeof(surface));
    surface.OsResource = entry->osResource;
    surface.Format     = Format_Invalid;
    osInterface       = state->osInterface;

    MOS_ZeroMemory(&info, sizeof(RENDERHAL_GET_SURFACE_INFO));

    CM_CHK_MOSSTATUS_GOTOFINISH(RenderHal_GetSurfaceInfo(
              osInterface,
              &info,
              &surface));

    param->pitch  = surface.dwPitch;
    param->qpitch = surface.dwQPitch;
    param->qpitchEnabled = state->cmHalInterface->IsSurf3DQpitchSupportedbyHw();

    // Lock the resource
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));

    if (param->lockFlag == CM_HAL_LOCKFLAG_READONLY)
    {
        lockFlags.ReadOnly = true;
    }
    else
    {
        lockFlags.WriteOnly = true;
    }

    lockFlags.ForceCached = true;
    param->data = osInterface->pfnLockResource(
                    osInterface,
                    &entry->osResource,
                    &lockFlags);
    CM_CHK_NULL_GOTOFINISH_MOSERROR(param->data);

finish:
    return eStatus;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Unlock the resource and return
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_Unlock3DResource(
    PCM_HAL_STATE               state,                                         // [in]  Pointer to CM State
    PCM_HAL_3DRESOURCE_PARAM    param)                                         // [in]  Pointer to 3D Param
{
    MOS_STATUS                  eStatus;
    PCM_HAL_3DRESOURCE_ENTRY    entry;
    PMOS_INTERFACE              osInterface;

    eStatus        = MOS_STATUS_SUCCESS;
    osInterface    = state->osInterface;

    // Get the 3D Resource Entry
    CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_Get3DResourceEntry(state, param->handle, &entry));

    // Lock the resource
    CM_CHK_HRESULT_GOTOFINISH_MOSERROR(osInterface->pfnUnlockResource(osInterface, &entry->osResource));

finish:
    return eStatus;
}

MOS_STATUS HalCm_SetCompressionMode(
    PCM_HAL_STATE               state,
    CM_HAL_SURFACE2D_COMPRESSIOM_PARAM  mmcParam)
{
    MOS_STATUS              eStatus = MOS_STATUS_SUCCESS;
    PMOS_INTERFACE          osInterface = state->osInterface;
    PCM_HAL_SURFACE2D_ENTRY     entry;

    // Get the 2D Resource Entry
    CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_GetSurface2DEntry(state, mmcParam.handle, &entry));

    //set compression bit passed down
    CM_CHK_MOSSTATUS_GOTOFINISH(osInterface->pfnSetMemoryCompressionMode(osInterface, &(entry->osResource), (MOS_MEMCOMP_STATE)mmcParam.mmcMode));

finish:
    return eStatus;
}

MOS_STATUS HalCm_SetL3Cache(
    const L3ConfigRegisterValues            *l3Values,
    PCmHalL3Settings                      cmHalL3Cache )
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    // in legacy platforms, we map:
    // ConfigRegister0->SqcReg1
    // ConfigRegister1->CntlReg2
    // ConfigRegister2->CntlReg3
    // ConfigRegister3->CntlReg
    CM_CHK_NULL_GOTOFINISH_MOSERROR( cmHalL3Cache );
    CM_CHK_NULL_GOTOFINISH_MOSERROR(l3Values);

    cmHalL3Cache->overrideSettings    =
                (l3Values->config_register0  || l3Values->config_register1 ||
                 l3Values->config_register2 || l3Values->config_register3 );
    cmHalL3Cache->cntlRegOverride    = (l3Values->config_register3 != 0);
    cmHalL3Cache->cntlReg2Override   = (l3Values->config_register1 != 0);
    cmHalL3Cache->cntlReg3Override   = (l3Values->config_register2 != 0);
    cmHalL3Cache->sqcReg1Override    = (l3Values->config_register0 != 0);
    cmHalL3Cache->cntlReg             = l3Values->config_register3;
    cmHalL3Cache->cntlReg2            = l3Values->config_register1;
    cmHalL3Cache->cntlReg3            = l3Values->config_register2;
    cmHalL3Cache->sqcReg1             = l3Values->config_register0;

finish:
    return MOS_STATUS_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Set Cap values
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_SetCaps(
    PCM_HAL_STATE              state,
    PCM_HAL_MAX_SET_CAPS_PARAM setCapsParam)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CM_CHK_NULL_GOTOFINISH_MOSERROR(state);
    CM_CHK_NULL_GOTOFINISH_MOSERROR(setCapsParam);
    CM_CHK_NULL_GOTOFINISH_MOSERROR(state->renderHal);
    CM_CHK_NULL_GOTOFINISH_MOSERROR(state->renderHal->pHwCaps)

    switch (setCapsParam->type)
    {
    case CM_SET_MAX_HW_THREADS:
        if( setCapsParam->maxValue <= 0 ||
            setCapsParam->maxValue > state->renderHal->pHwCaps->dwMaxThreads )
        {
            eStatus = MOS_STATUS_UNKNOWN;
            goto finish;
        }
        else
        {
            state->maxHWThreadValues.apiValue = (setCapsParam->maxValue == 0) ? 0:
                    MOS_MAX(setCapsParam->maxValue, state->cmHalInterface->GetSmallestMaxThreadNum());
        }
        break;

    case CM_SET_HW_L3_CONFIG:
        eStatus = state->cmHalInterface->SetL3CacheConfig( &setCapsParam->l3CacheValues,
                    &state->l3Settings );
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
    PCM_HAL_STATE               state,
    PCM_POWER_OPTION            powerOption )
{
    if (state->cmHalInterface->IsOverridePowerOptionPerGpuContext())
    {
        CM_NORMALMESSAGE("WARNING: Deprecated function due to per context SSEU overriding is enabled.\n");
        return MOS_STATUS_SUCCESS;
    }
    
    MOS_SecureMemcpy( &state->powerOption, sizeof( state->powerOption ), powerOption, sizeof( state->powerOption ) );
    return MOS_STATUS_SUCCESS;
}

//*-----------------------------------------------------------------------------
// Purpose: Get the time in ns from QueryPerformanceCounter
// Returns: Result of the operation
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_GetGlobalTime(LARGE_INTEGER *globalTime)
{
    if(globalTime == nullptr)
    {
        return MOS_STATUS_NULL_POINTER;
    }

    if (MOS_QueryPerformanceCounter((uint64_t*)&(globalTime->QuadPart)) == false)
    {
        return MOS_STATUS_UNKNOWN;
    }

    return MOS_STATUS_SUCCESS;
}

//*-----------------------------------------------------------------------------
// Purpose: Convert time from nanosecond to QPC time
// Returns: Result of the operation
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_ConvertToQPCTime(uint64_t nanoseconds, LARGE_INTEGER *qpcTime)
{
    LARGE_INTEGER     perfFreq;

    if(qpcTime == nullptr)
    {
        return MOS_STATUS_NULL_POINTER;
    }

    if (MOS_QueryPerformanceFrequency((uint64_t*)&perfFreq.QuadPart) == false)
    {
        return MOS_STATUS_UNKNOWN;
    }

    qpcTime->QuadPart = (uint64_t)(nanoseconds * perfFreq.QuadPart / 1000000000.0);

    return MOS_STATUS_SUCCESS;
}

//------------------------------------------------------------------------------
//| Purpose: Halcm updates power state to hw state
//| Returns:
//------------------------------------------------------------------------------
MOS_STATUS HalCm_UpdatePowerOption(
    PCM_HAL_STATE               state,
    PCM_POWER_OPTION            powerOption )
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    
    if (state->cmHalInterface->IsOverridePowerOptionPerGpuContext())
    {
        CM_NORMALMESSAGE("WARNING: Deprecated function due to per context SSEU overriding is enabled.\n");
        return MOS_STATUS_SUCCESS;
    }
    
    PRENDERHAL_INTERFACE renderHal = state->renderHal;

    RENDERHAL_POWEROPTION renderPowerOption;
    renderPowerOption.nSlice     = (uint8_t)powerOption->nSlice;
    renderPowerOption.nSubSlice  = (uint8_t)powerOption->nSubSlice;
    renderPowerOption.nEU        = (uint8_t)powerOption->nEU;

    // option set in CM create device to use slice shutdown for life of CM device ( override previous value if necessary )
    if ( state->requestSingleSlice == true )
    {
        renderPowerOption.nSlice = 1;
    }

    renderHal->pfnSetPowerOptionMode( renderHal, &renderPowerOption );

    return eStatus;
}

MOS_STATUS HalCm_InitPerfTagIndexMap(PCM_HAL_STATE cmState)
{
    using namespace std;
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    CM_ASSERT(cmState);
    for (int i = 0; i < MAX_COMBINE_NUM_IN_PERFTAG; i++)
    {
        cmState->currentPerfTagIndex[i] = 1;
#if MOS_MESSAGES_ENABLED
        cmState->perfTagIndexMap[i] = MOS_NewUtil<map<string, int> >(__FUNCTION__, __FILE__, __LINE__);
#else
        cmState->perfTagIndexMap[i] = MOS_NewUtil<map<string, int> >();
#endif

        CM_CHK_NULL_GOTOFINISH_MOSERROR(cmState->perfTagIndexMap[i]);
    }

    cmState->perfTagIndexMap[0]->insert(pair<string, int>("surfaceCopy_read_NV12_32x32", GPUCOPY_READ_PERFTAG_INDEX));
    cmState->perfTagIndexMap[0]->insert(pair<string, int>("surfaceCopy_read_NV12_aligned_32x32", GPUCOPY_READ_PERFTAG_INDEX));
    cmState->perfTagIndexMap[0]->insert(pair<string, int>("surfaceCopy_read_32x32", GPUCOPY_READ_PERFTAG_INDEX));
    cmState->perfTagIndexMap[0]->insert(pair<string, int>("surfaceCopy_read_aligned_32x32", GPUCOPY_READ_PERFTAG_INDEX));

    cmState->perfTagIndexMap[0]->insert(pair<string, int>("surfaceCopy_write_NV12_32x32", GPUCOPY_WRITE_PERFTAG_INDEX));
    cmState->perfTagIndexMap[0]->insert(pair<string, int>("surfaceCopy_write_32x32", GPUCOPY_WRITE_PERFTAG_INDEX));

    cmState->perfTagIndexMap[0]->insert(pair<string, int>("SurfaceCopy_2DTo2D_NV12_32x32", GPUCOPY_G2G_PERFTAG_INDEX));
    cmState->perfTagIndexMap[0]->insert(pair<string, int>("SurfaceCopy_2DTo2D_32x32", GPUCOPY_G2G_PERFTAG_INDEX));

    cmState->perfTagIndexMap[0]->insert(pair<string, int>("SurfaceCopy_BufferToBuffer_4k", GPUCOPY_C2C_PERFTAG_INDEX));
    cmState->perfTagIndexMap[0]->insert(pair<string, int>("SurfaceCopy_BufferToBuffer_4k", GPUCOPY_C2C_PERFTAG_INDEX));

    cmState->perfTagIndexMap[0]->insert(pair<string, int>("surfaceCopy_set_NV12", GPUINIT_PERFTAG_INDEX));
    cmState->perfTagIndexMap[0]->insert(pair<string, int>("surfaceCopy_set", GPUINIT_PERFTAG_INDEX));

finish:
    return eStatus;
}

MOS_STATUS HalCm_DeleteFromStateBufferList(
    PCM_HAL_STATE               state,
    void                        *kernelPtr )
{
    MOS_STATUS result = MOS_STATUS_SUCCESS;

    state->state_buffer_list_ptr->erase( kernelPtr );

    return result;
}

PRENDERHAL_MEDIA_STATE HalCm_GetMediaStatePtrForKernel(
    PCM_HAL_STATE               state,
    void                        *kernelPtr )
{
    if ( state->state_buffer_list_ptr->find( kernelPtr ) != state->state_buffer_list_ptr->end() )
    {
        return ( *state->state_buffer_list_ptr )[ kernelPtr ].mediaStatePtr;
    }
    else
    {
        return nullptr;
    }
}

uint64_t HalCm_GetStateBufferVAPtrForSurfaceIndex(
    PCM_HAL_STATE               state,
    uint32_t                    surfIndex )
{
    for ( auto listItem = state->state_buffer_list_ptr->begin(); listItem != state->state_buffer_list_ptr->end(); listItem++ )
    {
        if ( listItem->second.stateBufferIndex == surfIndex )
        {
            return listItem->second.stateBufferVaPtr;
        }
    }
    return 0;
}

PRENDERHAL_MEDIA_STATE HalCm_GetMediaStatePtrForSurfaceIndex(
    PCM_HAL_STATE               state,
    uint32_t                    surfIndex )
{
    for ( auto listItem = state->state_buffer_list_ptr->begin(); listItem != state->state_buffer_list_ptr->end(); listItem++ )
    {
        if ( listItem->second.stateBufferIndex == surfIndex )
        {
            return listItem->second.mediaStatePtr;
        }
    }
    return nullptr;
}

uint64_t HalCm_GetStateBufferVAPtrForMediaStatePtr(
    PCM_HAL_STATE               state,
    PRENDERHAL_MEDIA_STATE      mediaStatePtr )
{
    for ( auto listItem = state->state_buffer_list_ptr->begin(); listItem != state->state_buffer_list_ptr->end(); listItem++ )
    {
        if ( listItem->second.mediaStatePtr == mediaStatePtr )
        {
            return listItem->second.stateBufferVaPtr;
        }
    }
    return 0;
}

uint32_t HalCm_GetStateBufferSizeForKernel(
    PCM_HAL_STATE               state,
    void                        *kernelPtr )
{
    if ( state->state_buffer_list_ptr->find( kernelPtr ) != state->state_buffer_list_ptr->end() )
    {
        return ( *state->state_buffer_list_ptr )[ kernelPtr ].stateBufferSize;
    }
    else
    {
        return 0;
    }
}

CM_STATE_BUFFER_TYPE HalCm_GetStateBufferTypeForKernel(
    PCM_HAL_STATE               state,
    void                        *kernelPtr )
{
    if ( state->state_buffer_list_ptr->find( kernelPtr ) != state->state_buffer_list_ptr->end() )
    {
        return ( *state->state_buffer_list_ptr )[ kernelPtr ].stateBufferType;
    }
    else
    {
        return CM_STATE_BUFFER_NONE;
    }
}

void LoadUserFeatures(MOS_GPUCTX_CREATOPTIONS *createOptions)
{
#if (_DEBUG || _RELEASE_INTERNAL)
    MOS_USER_FEATURE_VALUE_DATA  user_feature_data;
    MOS_ZeroMemory(&user_feature_data, sizeof(user_feature_data));
    MOS_STATUS result
            = MOS_UserFeature_ReadValue_ID(
                nullptr, __MEDIA_USER_FEATURE_VALUE_MDF_FORCE_RAMODE,
                &user_feature_data);
    if (MOS_STATUS_SUCCESS == result && user_feature_data.i32Data == 1)
    {
        createOptions->RAMode = 1;
    }
#endif
    return;
}

MOS_STATUS HalCm_CreateGPUContext(
    PCM_HAL_STATE            state,
    MOS_GPU_CONTEXT          gpuContext,
    MOS_GPU_NODE             gpuNode,
    PMOS_GPUCTX_CREATOPTIONS pMosGpuContextCreateOption)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    LoadUserFeatures(pMosGpuContextCreateOption);

    // Create Compute Context on Compute Node
    CM_CHK_HRESULT_GOTOFINISH_MOSERROR(state->osInterface->pfnCreateGpuContext(
        state->osInterface,
        gpuContext,
        gpuNode,
        pMosGpuContextCreateOption));

    // Register Compute Context with the Batch Buffer completion event
    CM_CHK_HRESULT_GOTOFINISH_MOSERROR(state->osInterface->pfnRegisterBBCompleteNotifyEvent(
        state->osInterface,
        gpuContext));

finish:
    return eStatus;
}

GPU_CONTEXT_HANDLE
HalCm_CreateGpuComputeContext(CM_HAL_STATE *state,
                              MOS_GPUCTX_CREATOPTIONS *createOptions)
{
    LoadUserFeatures(createOptions);

    GPU_CONTEXT_HANDLE context_handle
            = state->osInterface->pfnCreateGpuComputeContext(
                state->osInterface, MOS_GPU_CONTEXT_CM_COMPUTE, createOptions);
    if (MOS_GPU_CONTEXT_INVALID_HANDLE != context_handle)
    {
        state->osInterface->pfnRegisterBBCompleteNotifyEvent(
            state->osInterface, MOS_GPU_CONTEXT_CM_COMPUTE);
    }
    return context_handle;
}

uint32_t HalCm_SetGpuContext(CM_HAL_STATE *halState,
                             MOS_GPU_CONTEXT contextName,
                             uint32_t streamIndex,
                             GPU_CONTEXT_HANDLE contextHandle)
{
    uint32_t old_stream_idx = halState->osInterface->streamIndex;
    halState->osInterface->streamIndex = streamIndex;
    MOS_STATUS result = MOS_STATUS_SUCCESS;

    if (MOS_GPU_CONTEXT_INVALID_HANDLE == contextHandle)
    {
        result = halState->osInterface->pfnSetGpuContext(halState->osInterface,
                                                         contextName);
    }
    else
    {
        result = halState->osInterface->pfnSetGpuContextFromHandle(
            halState->osInterface, contextName, contextHandle);
    }

    if (MOS_STATUS_SUCCESS != result)
    {
        halState->osInterface->streamIndex = old_stream_idx;
        return INVALID_STREAM_INDEX;
    }
    return old_stream_idx;
}

MOS_STATUS HalCm_SelectSyncBuffer(CM_HAL_STATE *halState, uint32_t bufferIdx)
{
    if (bufferIdx >= halState->cmDeviceParam.maxBufferTableSize)
    {
        halState->syncBuffer = nullptr;
        return MOS_STATUS_SUCCESS;
    }
    CM_HAL_BUFFER_ENTRY *entry = halState->bufferTable + bufferIdx;
    halState->syncBuffer = &entry->osResource;
    MOS_INTERFACE *os_interface = halState->osInterface;
    return os_interface->pfnRegisterResource(os_interface, halState->syncBuffer,
                                             true, true);
}

//*-----------------------------------------------------------------------------
//| Purpose:    Creates instance of HAL CM State
//| Returns:    Result of the operation
//| Note:       Caller must call pfnAllocate to allocate all HalCm/Mhw states and objects.
//|             Caller MUST call HalCm_Destroy to destroy the instance
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_Create(
    PMOS_CONTEXT            osDriverContext,   // [in] OS Driver Context
    PCM_HAL_CREATE_PARAM     param,             // [in] Create Param
    PCM_HAL_STATE           *cmState)          // [out] double pointer to CM State
{
    MOS_STATUS          eStatus;
    PCM_HAL_STATE       state = nullptr;
    uint32_t            numCmdBuffers = 0;
    MhwInterfaces       *mhwInterfaces = nullptr;
    MhwInterfaces::CreateParams params;
    MOS_GPUCTX_CREATOPTIONS createOption;

    //-----------------------------------------
    CM_ASSERT(osDriverContext);
    CM_ASSERT(param);
    CM_ASSERT(cmState);
    //-----------------------------------------

    eStatus  = MOS_STATUS_SUCCESS;

    // Allocate State structure
    state = (PCM_HAL_STATE)MOS_AllocAndZeroMemory(sizeof(CM_HAL_STATE));
    CM_CHK_NULL_GOTOFINISH_MOSERROR(state);

    // Allocate/Initialize OS Interface
    state->osInterface = (PMOS_INTERFACE)
                                MOS_AllocAndZeroMemory(sizeof(MOS_INTERFACE));
    CM_CHK_NULL_GOTOFINISH_MOSERROR(state->osInterface);
    state->osInterface->bDeallocateOnExit = true;
    CM_CHK_HRESULT_GOTOFINISH_MOSERROR(Mos_InitInterface(state->osInterface, osDriverContext, COMPONENT_CM));
#if (_RELEASE_INTERNAL || _DEBUG)
#if defined(CM_DIRECT_GUC_SUPPORT)
    state->osInterface->m_pWorkQueueMngr = new CMRTWorkQueueMngr();
#endif
#endif

    state->osInterface->pfnGetPlatform(state->osInterface, &state->platform);
    state->skuTable = state->osInterface->pfnGetSkuTable(state->osInterface);
    state->waTable  = state->osInterface->pfnGetWaTable (state->osInterface);

    // Create VEBOX Context
    createOption.CmdBufferNumScale = MOS_GPU_CONTEXT_CREATE_DEFAULT;
    CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_CreateGPUContext(
        state,
        MOS_GPU_CONTEXT_VEBOX,
        MOS_GPU_NODE_VE,
        &createOption));

    // Allocate/Initialize CM Rendering Interface
    state->renderHal = (PRENDERHAL_INTERFACE)
                                MOS_AllocAndZeroMemory(sizeof(RENDERHAL_INTERFACE));
    CM_CHK_NULL_GOTOFINISH_MOSERROR(state->renderHal);

    state->dshEnabled                   = param->dynamicStateHeap;
    state->renderHal->bDynamicStateHeap = state->dshEnabled;

    if (state->dshEnabled)
    {
        CM_CHK_MOSSTATUS_GOTOFINISH(RenderHal_InitInterface_Dynamic(state->renderHal, &state->cpInterface, state->osInterface));
    }
    else
    {
        CM_CHK_MOSSTATUS_GOTOFINISH(RenderHal_InitInterface(state->renderHal, &state->cpInterface, state->osInterface));
    }

    // Allocate/Initialize VEBOX Interface
    if (!param->disableVebox)
    {
        CmSafeMemSet(&params, 0, sizeof(params));
        params.Flags.m_vebox = 1;
        mhwInterfaces = MhwInterfaces::CreateFactory(params, state->osInterface);
        if (mhwInterfaces)
        {
            CM_CHK_NULL_GOTOFINISH_MOSERROR(mhwInterfaces->m_veboxInterface);
            state->veboxInterface = mhwInterfaces->m_veboxInterface;

            // MhwInterfaces always create CP and MI interfaces, so we have to delete those we don't need.
            MOS_Delete(mhwInterfaces->m_miInterface);
            Delete_MhwCpInterface(mhwInterfaces->m_cpInterface);
            mhwInterfaces->m_cpInterface = nullptr;
            MOS_Delete(mhwInterfaces);
        }
        else
        {
            CM_ASSERTMESSAGE("Allocate MhwInterfaces failed");
            return MOS_STATUS_NO_SPACE;
        }
    }
    else
    {
        state->veboxInterface = nullptr;
    }

    // set IsMDFLoad to distinguish MDF context from other Media Contexts
    state->renderHal->IsMDFLoad = true;

    // disable YV12SinglePass as CMRT & compiler don't support it
    state->renderHal->bEnableYV12SinglePass = false;

    state->cmDeviceParam.maxKernelBinarySize      = CM_KERNEL_BINARY_BLOCK_SIZE;

    // set if the new sampler heap management is used or not
    // currently new sampler heap management depends on DSH
    if (state->dshEnabled)
    {
        state->useNewSamplerHeap = true;
    }
    else
    {
        state->useNewSamplerHeap = false;
    }

    //Get Max Scratch Space Size
    if( param->disableScratchSpace)
    {
        state->cmDeviceParam.maxPerThreadScratchSpaceSize = 0;
    }
    else
    {
         //Gen7_5 + : (MaxScratchSpaceSize + 1) *16k
          if(param->scratchSpaceSize == CM_DEVICE_CONFIG_SCRATCH_SPACE_SIZE_DEFAULT)
          { //By default, 128K for HSW
               state->cmDeviceParam.maxPerThreadScratchSpaceSize = 8 * CM_DEVICE_CONFIG_SCRATCH_SPACE_SIZE_16K_STEP;
          }
          else
          {
               state->cmDeviceParam.maxPerThreadScratchSpaceSize = (param->scratchSpaceSize)*
                                CM_DEVICE_CONFIG_SCRATCH_SPACE_SIZE_16K_STEP;
          }
    }

    // Initialize kernel parameters
    state->kernelParamsRenderHal.pMhwKernelParam = &state->kernelParamsMhw;

    // Enable SLM in L3 Cache
    state->l3Settings.enableSlm = true;

    // Slice shutdown
    state->requestSingleSlice = param->requestSliceShutdown;

    //mid thread preemption on/off and SIP debug control
    state->midThreadPreemptionDisabled = param->disabledMidThreadPreemption;
    state->kernelDebugEnabled = param->enabledKernelDebug;

    // init mapping for the state buffer
#if MOS_MESSAGES_ENABLED
    state->state_buffer_list_ptr = MOS_NewUtil<std::map< void *, CM_HAL_STATE_BUFFER_ENTRY> >(__FUNCTION__, __FILE__, __LINE__);
#else
    state->state_buffer_list_ptr = MOS_NewUtil<std::map< void *, CM_HAL_STATE_BUFFER_ENTRY> >();
#endif

    CM_CHK_NULL_GOTOFINISH_MOSERROR( state->state_buffer_list_ptr );

    MOS_ZeroMemory(&state->hintIndexes.kernelIndexes, sizeof(uint32_t) * CM_MAX_TASKS_EU_SATURATION);
    MOS_ZeroMemory(&state->hintIndexes.dispatchIndexes, sizeof(uint32_t) * CM_MAX_TASKS_EU_SATURATION);

    // get the global media profiler
    state->perfProfiler = MediaPerfProfiler::Instance();
    CM_CHK_NULL_GOTOFINISH_MOSERROR(state->perfProfiler);
    CM_CHK_MOSSTATUS_GOTOFINISH(state->perfProfiler->Initialize((void*)state, state->osInterface));

    state->criticalSectionDSH = MOS_New(CMRT_UMD::CSync);
    CM_CHK_NULL_GOTOFINISH_MOSERROR(state->criticalSectionDSH);

    state->cmDeviceParam.maxKernelsPerTask        = CM_MAX_KERNELS_PER_TASK;
    state->cmDeviceParam.maxSamplerTableSize      = CM_MAX_SAMPLER_TABLE_SIZE;
    state->cmDeviceParam.maxSampler8x8TableSize   = state->renderHal->pHwSizes->dwSizeSampler8x8Table;
    state->cmDeviceParam.maxBufferTableSize       = CM_MAX_BUFFER_SURFACE_TABLE_SIZE;
    state->cmDeviceParam.max2DSurfaceUPTableSize  = CM_MAX_2D_SURFACE_UP_TABLE_SIZE;
    state->cmDeviceParam.max2DSurfaceTableSize    = CM_MAX_2D_SURFACE_TABLE_SIZE;
    state->cmDeviceParam.max3DSurfaceTableSize    = CM_MAX_3D_SURFACE_TABLE_SIZE;
    state->cmDeviceParam.maxTasks                 = param->maxTaskNumber;
    state->cmDeviceParam.maxAvsSamplers           = CM_MAX_AVS_SAMPLER_SIZE;
    state->cmDeviceParam.maxGshKernelEntries      = param->kernelBinarySizeinGSH / (CM_32K);

    if (state->dshEnabled)
    {
        // Initialize Kernel Cache Hit/Miss counters
        state->dshKernelCacheMiss = 0;
        state->dshKernelCacheHit  = 0;
    }

    // Setup Function pointers
    state->pfnCmAllocate                  = HalCm_Allocate;
    state->pfnGetMaxValues                = HalCm_GetMaxValues;
    state->pfnGetMaxValuesEx              = HalCm_GetMaxValuesEx;
    state->pfnExecuteTask                 = HalCm_ExecuteTask;
    state->pfnExecuteGroupTask            = HalCm_ExecuteGroupTask;
    state->pfnExecuteHintsTask            = HalCm_ExecuteHintsTask;
    state->pfnRegisterSampler             = HalCm_RegisterSampler;
    state->pfnUnRegisterSampler           = HalCm_UnRegisterSampler;
    state->pfnRegisterSampler8x8          = HalCm_RegisterSampler8x8;
    state->pfnUnRegisterSampler8x8        = HalCm_UnRegisterSampler8x8;
    state->pfnFreeBuffer                  = HalCm_FreeBuffer;
    state->pfnLockBuffer                  = HalCm_LockBuffer;
    state->pfnUnlockBuffer                = HalCm_UnlockBuffer;
    state->pfnFreeSurface2DUP             = HalCm_FreeSurface2DUP;
    state->pfnGetSurface2DTileYPitch      = HalCm_GetSurface2DTileYPitch;
    state->pfnSet2DSurfaceStateParam      = HalCm_Set2DSurfaceStateParam;
    state->pfnSetBufferSurfaceStatePara   = HalCm_SetBufferSurfaceStateParameters;
    state->pfnSetSurfaceMOCS              = HalCm_SetSurfaceMOCS;
    /************************************************************/
    state->pfnAllocateSurface2D           = HalCm_AllocateSurface2D;
    state->pfnAllocate3DResource          = HalCm_AllocateSurface3D;
    state->pfnFreeSurface2D               = HalCm_FreeSurface2D;
    state->pfnLock2DResource              = HalCm_Lock2DResource;
    state->pfnUnlock2DResource            = HalCm_Unlock2DResource;
    state->pfnSetCompressionMode          = HalCm_SetCompressionMode;
    /************************************************************/
    state->pfnFree3DResource              = HalCm_Free3DResource;
    state->pfnLock3DResource              = HalCm_Lock3DResource;
    state->pfnUnlock3DResource            = HalCm_Unlock3DResource;
    state->pfnSetCaps                     = HalCm_SetCaps;
    state->pfnSetPowerOption              = HalCm_SetPowerOption;
    state->pfnUpdatePowerOption           = HalCm_UpdatePowerOption;

    state->pfnSendMediaWalkerState        = HalCm_SendMediaWalkerState;
    state->pfnSendGpGpuWalkerState        = HalCm_SendGpGpuWalkerState;
    state->pfnSetSurfaceReadFlag          = HalCm_SetSurfaceReadFlag;
    state->pfnSetVtuneProfilingFlag       = HalCm_SetVtuneProfilingFlag;
    state->pfnExecuteVeboxTask            = HalCm_ExecuteVeboxTask;
    state->pfnGetSipBinary                = HalCm_GetSipBinary;
    state->pfnGetTaskSyncLocation         = HalCm_GetTaskSyncLocation;

    state->pfnGetGlobalTime               = HalCm_GetGlobalTime;
    state->pfnConvertToQPCTime            = HalCm_ConvertToQPCTime;

    state->pfnDeleteFromStateBufferList = HalCm_DeleteFromStateBufferList;
    state->pfnGetMediaStatePtrForKernel = HalCm_GetMediaStatePtrForKernel;
    state->pfnGetStateBufferVAPtrForSurfaceIndex = HalCm_GetStateBufferVAPtrForSurfaceIndex;
    state->pfnGetMediaStatePtrForSurfaceIndex = HalCm_GetMediaStatePtrForSurfaceIndex;
    state->pfnGetStateBufferVAPtrForMediaStatePtr = HalCm_GetStateBufferVAPtrForMediaStatePtr;
    state->pfnGetStateBufferSizeForKernel = HalCm_GetStateBufferSizeForKernel;
    state->pfnGetStateBufferTypeForKernel = HalCm_GetStateBufferTypeForKernel;
    state->pfnCreateGPUContext            = HalCm_CreateGPUContext;
    state->pfnCreateGpuComputeContext     = HalCm_CreateGpuComputeContext;
    state->pfnSetGpuContext               = HalCm_SetGpuContext;
    state->pfnSelectSyncBuffer            = HalCm_SelectSyncBuffer;
    state->pfnDSHUnregisterKernel         = HalCm_DSH_UnregisterKernel;

    state->pfnUpdateBuffer                = HalCm_UpdateBuffer;
    state->pfnUpdateSurface2D             = HalCm_UpdateSurface2D;

    //==========<Initialize 5 OS-dependent DDI functions: pfnAllocate3DResource, pfnAllocateSurface2DUP====
    //                 pfnAllocateBuffer,pfnRegisterKMDNotifyEventHandle, pfnGetSurface2DPitchAndSize >====
    HalCm_OsInitInterface(state);

    HalCm_InitPerfTagIndexMap(state);

    state->maxHWThreadValues.userFeatureValue = 0;
    state->maxHWThreadValues.apiValue = 0;

    HalCm_GetUserFeatureSettings(state);

#if MDF_COMMAND_BUFFER_DUMP
    HalCm_InitDumpCommandBuffer(state);
    state->pfnInitDumpCommandBuffer = HalCm_InitDumpCommandBuffer;
    state->pfnDumpCommadBuffer      = HalCm_DumpCommadBuffer;
#endif //MDF_COMMAND_BUFFER_DUMP

#if MDF_CURBE_DATA_DUMP
    HalCm_InitDumpCurbeData(state);
#endif

#if MDF_SURFACE_CONTENT_DUMP
    HalCm_InitSurfaceDump(state);
#endif

#if MDF_SURFACE_STATE_DUMP
    HalCm_InitDumpSurfaceState(state);
    state->pfnInitDumpSurfaceState = HalCm_InitDumpSurfaceState;
    state->pfnDumpSurfaceState = HalCm_DumpSurfaceState;
#endif

#if MDF_INTERFACE_DESCRIPTOR_DATA_DUMP
  HalCm_InitDumpInterfaceDescriporData(state);
#endif

    state->cmHalInterface = CMHalDevice::CreateFactory(state);
    CM_CHK_NULL_GOTOFINISH_MOSERROR(state->cmHalInterface);

    if (param->refactor)
    {
        state->refactor = true;
    }
    else
    {
        state->refactor = false;
    }

    state->requestCustomGpuContext = param->requestCustomGpuContext;

#if (_DEBUG || _RELEASE_INTERNAL)
    {
        MOS_USER_FEATURE_VALUE_DATA userFeatureData;

        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_MDF_FORCE_EXECUTION_PATH_ID,
            &userFeatureData);

        if (userFeatureData.i32Data == 1)
        {
            state->refactor = false;
        }
        else if (userFeatureData.i32Data == 2)
        {
            state->refactor = true;
            state->cmHalInterface->SetFastPathByDefault(true);
        }

        FILE *fp1 = nullptr;
        MOS_SecureFileOpen(&fp1, "refactor.key", "r");
        if (fp1 != nullptr)
        {
            state->refactor = true;
            state->cmHalInterface->SetFastPathByDefault(true);
            fclose(fp1);
        }

        FILE *fp2 = nullptr;
        MOS_SecureFileOpen(&fp2, "origin.key", "r");
        if (fp2 != nullptr)
        {
            state->refactor = false;
            fclose(fp2);
        }
    }
#endif

    if (state->refactor)
    {
        CM_NORMALMESSAGE("Info: Fast path is enabled!\n");
    }
    else
    {
        CM_NORMALMESSAGE("Info: Fast path is disabled!\n");
    }

finish:
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        HalCm_Destroy(state);
        *cmState = nullptr;
    }
    else
    {
        *cmState = state;
    }

    return eStatus;
}

//*-----------------------------------------------------------------------------
//| Purpose: Destroys instance of HAL CM State
//| Returns: N/A
//*-----------------------------------------------------------------------------
void HalCm_Destroy(
    PCM_HAL_STATE state)                                                       // [in] Pointer to CM State
{
    MOS_STATUS eStatus;
    int32_t    i;

    if (state)
    {
        //Delete CmHal Interface
        MosSafeDelete(state->cmHalInterface);
        Delete_MhwCpInterface(state->cpInterface);
        state->cpInterface = nullptr;
        MosSafeDelete(state->state_buffer_list_ptr);
        MosSafeDelete(state->criticalSectionDSH);

        // Delete the unified media profiler
        if (state->perfProfiler)
        {
            MediaPerfProfiler::Destroy(state->perfProfiler, (void*)state, state->osInterface);
            state->perfProfiler = nullptr;
        }
        
        // Delete Batch Buffers
        if (state->batchBuffers)
        {
            for (i=0; i < state->numBatchBuffers; i++)
            {
                if (!Mos_ResourceIsNull(&state->batchBuffers[i].OsResource))
                {
                    eStatus = (MOS_STATUS)state->renderHal->pfnFreeBB(
                                state->renderHal,
                                &state->batchBuffers[i]);

                    CM_ASSERT(eStatus == MOS_STATUS_SUCCESS);
                }

                MOS_FreeMemory(state->batchBuffers[i].pPrivateData);
            }

            MOS_FreeMemory(state->batchBuffers);
            state->batchBuffers = nullptr;
        }

        // Delete TimeStamp Buffer
        HalCm_FreeTsResource(state);
        if ((state->midThreadPreemptionDisabled == false) || (state->kernelDebugEnabled == true)) {
            // Delete CSR surface
            HalCm_FreeCsrResource(state);

            // Delete sip surface
            HalCm_FreeSipResource(state);
        }

        // Delete tracker resource
        HalCm_FreeTrackerResources(state);

        // Delete advance executor
        MOS_Delete(state->advExecutor);

        // Delete heap manager
        if (state->renderHal)
        {
            MOS_Delete(state->renderHal->dgsheapManager);
        }

        if (state->hLibModule)
        {
            MOS_FreeLibrary(state->hLibModule);
            state->hLibModule = nullptr;
        }

        // Delete RenderHal Interface
        if (state->renderHal)
        {
            if (state->renderHal->pfnDestroy)
            {
                state->renderHal->pfnDestroy(state->renderHal);
            }
            MOS_FreeMemory(state->renderHal);
            state->renderHal = nullptr;
        }

        // Delete VEBOX Interface
        if (state->veboxInterface
            && state->veboxInterface->m_veboxHeap)
        {
            state->veboxInterface->DestroyHeap( );
            MOS_Delete(state->veboxInterface);
            state->veboxInterface = nullptr;
        }

        // Delete OS Interface
        if (state->osInterface)
        {
            if (state->osInterface->pfnDestroy)
            {
                state->osInterface->pfnDestroy(state->osInterface, true);
            }
            if (state->osInterface->bDeallocateOnExit)
            {
                MOS_FreeMemory(state->osInterface);
                state->osInterface = nullptr;
            }
        }

        // Delete the TaskParam
        MOS_FreeMemory(state->taskParam);

        // Delete the TaskTimeStamp
        MOS_FreeMemory(state->taskTimeStamp);

        // Delete Tables
        MOS_FreeMemory(state->tableMemories);

        // Delete the pTotalKernelSize table for GSH
        MOS_FreeMemory(state->totalKernelSize);

        // Delete the perfTag Map
        for (int i = 0; i < MAX_COMBINE_NUM_IN_PERFTAG; i++)
        {
            MosSafeDelete(state->perfTagIndexMap[i]);
        }

        // Delete the state
        MOS_FreeMemory(state);
    }
}

void HalCm_GetUserFeatureSettings(
    PCM_HAL_STATE                  cmState
)
{
#if (_DEBUG || _RELEASE_INTERNAL)
    PMOS_INTERFACE osInterface = cmState->osInterface;

    MOS_USER_FEATURE_VALUE_DATA userFeatureData;

    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_MDF_MAX_THREAD_NUM_ID,
        &userFeatureData);

    if (userFeatureData.i32Data != 0)
    {
        uint32_t data = userFeatureData.i32Data;
        if ((data > 0) && (data <= cmState->renderHal->pHwCaps->dwMaxThreads))
        {
            cmState->maxHWThreadValues.userFeatureValue = data;
        }
    }

#else
    UNUSED(cmState);
#endif // _DEBUG || _RELEASE_INTERNAL
}

//*-----------------------------------------------------------------------------
//| Purpose: Gathers information about the surface - used by GT-Pin
//| Returns: MOS_STATUS_SUCCESS if surface type recognized, S_FAIL otherwise
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_GetSurfaceDetails(
    PCM_HAL_STATE                  cmState,
    PCM_HAL_INDEX_PARAM            indexParam,
    uint32_t                       btIndex,
    MOS_SURFACE&                   surface,
    int16_t                        globalSurface,
    PRENDERHAL_SURFACE_STATE_ENTRY surfaceEntry,
    uint32_t                       tempPlaneIndex,
    RENDERHAL_SURFACE_STATE_PARAMS surfaceParam,
    CM_HAL_KERNEL_ARG_KIND         argKind
    )
{
    MOS_STATUS                 eStatus = MOS_STATUS_UNKNOWN;
    PCM_SURFACE_DETAILS        surfaceInfos  = nullptr;
    PCM_SURFACE_DETAILS        pgSurfaceInfos = nullptr;
    PCM_HAL_TASK_PARAM         taskParam     = cmState->taskParam;
    uint32_t                   curKernelIndex = taskParam->curKernelIndex;
    PMOS_PLANE_OFFSET          planeOffset   = 0;
    uint32_t                   maxEntryNum    = 0;
    MOS_OS_FORMAT              tempOsFormat   ;

    CM_SURFACE_BTI_INFO surfBTIInfo;
    cmState->cmHalInterface->GetHwSurfaceBTIInfo(&surfBTIInfo);

    UNUSED(indexParam);

    if(curKernelIndex+1>taskParam->surfEntryInfoArrays.kernelNum)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        CM_ASSERTMESSAGE(
            "Mismatched kernel index: curKernelIndex '%d' vs krnNum '%d'",
            curKernelIndex,taskParam->surfEntryInfoArrays.kernelNum);
        goto finish;
    }

    surfaceInfos  = taskParam->surfEntryInfoArrays.surfEntryInfosArray[curKernelIndex].surfEntryInfos;
    pgSurfaceInfos = taskParam->surfEntryInfoArrays.surfEntryInfosArray[curKernelIndex].globalSurfInfos;

    tempOsFormat   = cmState->osInterface->pfnFmt_MosToOs(surface.Format);

    switch (argKind)
    {
    case CM_ARGUMENT_SURFACEBUFFER:

        if((btIndex >= surfBTIInfo.reservedSurfaceStart) &&
            (btIndex < surfBTIInfo.reservedSurfaceStart + CM_MAX_GLOBAL_SURFACE_NUMBER))
        {
            btIndex = btIndex - surfBTIInfo.reservedSurfaceStart;

            maxEntryNum = taskParam->surfEntryInfoArrays.surfEntryInfosArray->globalSurfNum;
            if ( btIndex >= maxEntryNum )
            {
                eStatus = MOS_STATUS_INVALID_PARAMETER;
                CM_ASSERTMESSAGE(
                "Array for surface details is full: Max number of entries '%d' and trying to add index '%d'",
                maxEntryNum, btIndex);
                goto finish;
            }

            MOS_ZeroMemory(&pgSurfaceInfos[btIndex], sizeof(CM_SURFACE_DETAILS));
            pgSurfaceInfos[btIndex].width  = surface.dwWidth;
            pgSurfaceInfos[btIndex].format = DDI_FORMAT_UNKNOWN;
        }
        else
        {
            btIndex = btIndex - surfBTIInfo.reservedSurfaceStart - CM_MAX_GLOBAL_SURFACE_NUMBER;
            maxEntryNum = taskParam->surfEntryInfoArrays.surfEntryInfosArray->maxEntryNum;
            if ( btIndex >= maxEntryNum )
            {
                eStatus = MOS_STATUS_INVALID_PARAMETER;
                CM_ASSERTMESSAGE(
                "Array for surface details is full: Max number of entries '%d' and trying to add index '%d'",
                maxEntryNum, btIndex);
                goto finish;
            }

            MOS_ZeroMemory(&surfaceInfos[btIndex], sizeof(CM_SURFACE_DETAILS));
            surfaceInfos[btIndex].width  = surface.dwWidth;
            surfaceInfos[btIndex].format = DDI_FORMAT_UNKNOWN;
        }

        if (globalSurface < 0)
        {
            ++taskParam->surfEntryInfoArrays.surfEntryInfosArray[curKernelIndex].usedIndex;
        }

        eStatus = MOS_STATUS_SUCCESS;
        break;

    case CM_ARGUMENT_SURFACE2D_UP:
    case CM_ARGUMENT_SURFACE2D:
    // VME surface and sampler8x8 called with CM_ARGUMENT_SURFACE2D
         btIndex = btIndex - surfBTIInfo.reservedSurfaceStart - CM_MAX_GLOBAL_SURFACE_NUMBER;
         maxEntryNum = taskParam->surfEntryInfoArrays.surfEntryInfosArray->maxEntryNum;

         if ( btIndex >= maxEntryNum )
         {
             eStatus = MOS_STATUS_INVALID_PARAMETER;
             CM_ASSERTMESSAGE(
             "Array for surface details is full: Max number of entries '%d' and trying to add index '%d'",
             maxEntryNum, btIndex);
             goto finish;
         }

         surfaceInfos[btIndex].width              = surfaceEntry->dwWidth;
         surfaceInfos[btIndex].height             = surfaceEntry->dwHeight;
         surfaceInfos[btIndex].depth              = 0;
         surfaceInfos[btIndex].format             = (DdiSurfaceFormat)tempOsFormat;
         surfaceInfos[btIndex].planeIndex         = tempPlaneIndex;
         surfaceInfos[btIndex].pitch              = surfaceEntry->dwPitch;
         surfaceInfos[btIndex].slicePitch         = 0;
         surfaceInfos[btIndex].surfaceBaseAddress = 0;
         surfaceInfos[btIndex].tileWalk           = surfaceEntry->bTileWalk;
         surfaceInfos[btIndex].tiledSurface       = surfaceEntry->bTiledSurface;

         if (surfaceEntry->YUVPlane == MHW_U_PLANE ||
             surfaceEntry->YUVPlane == MHW_V_PLANE)
         {
             planeOffset = (surfaceEntry->YUVPlane == MHW_U_PLANE)
             ? &surface.UPlaneOffset
             : &surface.VPlaneOffset;

             surfaceInfos[btIndex].yOffset = planeOffset->iYOffset >> 1;

             if ( argKind == CM_ARGUMENT_SURFACE2D_UP )
             {
                 surfaceInfos[btIndex].xOffset = (planeOffset->iXOffset/(uint32_t)sizeof(uint32_t)) >> 2;
             }
             else
             {
                 uint32_t pixelsPerSampleUV = 0;
                 //Get Pixels Per Sample if we use dataport read
                 if(surfaceParam.bWidthInDword_UV)
                 {
                     RenderHal_GetPixelsPerSample(surface.Format, &pixelsPerSampleUV);
                 }
                 else
                 {
                     // If the kernel uses sampler - do not change width (it affects coordinates)
                     pixelsPerSampleUV = 1;
                 }

                 if(pixelsPerSampleUV == 1)
                 {
                     surfaceInfos[btIndex].xOffset = planeOffset->iXOffset >> 2;
                 }
                 else
                 {
                     surfaceInfos[btIndex].xOffset = (planeOffset->iXOffset/(uint32_t)sizeof(uint32_t)) >> 2;
                 }
             }
         }
         else
         {
             surfaceInfos[btIndex].xOffset = (surface.YPlaneOffset.iXOffset/(uint32_t)sizeof(uint32_t)) >> 2;
             surfaceInfos[btIndex].yOffset = surface.YPlaneOffset.iYOffset >> 1;
         }

         ++taskParam->surfEntryInfoArrays.surfEntryInfosArray[curKernelIndex].usedIndex;
         ++tempPlaneIndex;

         eStatus = MOS_STATUS_SUCCESS;
         break;

    case CM_ARGUMENT_SURFACE3D:

        btIndex = btIndex - surfBTIInfo.normalSurfaceStart - CM_MAX_GLOBAL_SURFACE_NUMBER;
        maxEntryNum = taskParam->surfEntryInfoArrays.surfEntryInfosArray->maxEntryNum;

        if ( btIndex >= maxEntryNum )
        {
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            CM_ASSERTMESSAGE(
            "Array for surface details is full: Max number of entries '%d' and trying to add index '%d'",
            maxEntryNum, btIndex);
            goto finish;
        }

        surfaceInfos[btIndex].width              = surfaceEntry->dwWidth;
        surfaceInfos[btIndex].height             = surfaceEntry->dwHeight;
        surfaceInfos[btIndex].depth              = surface.dwDepth;
        surfaceInfos[btIndex].format             = (DdiSurfaceFormat)tempOsFormat;
        surfaceInfos[btIndex].pitch              = surfaceEntry->dwPitch;
        surfaceInfos[btIndex].planeIndex         = tempPlaneIndex;
        surfaceInfos[btIndex].slicePitch         = surface.dwSlicePitch;
        surfaceInfos[btIndex].surfaceBaseAddress = 0;
        surfaceInfos[btIndex].tileWalk           = surfaceEntry->bTileWalk;
        surfaceInfos[btIndex].tiledSurface       = surfaceEntry->bTiledSurface;

        if (surfaceEntry->YUVPlane == MHW_U_PLANE ||
            surfaceEntry->YUVPlane == MHW_V_PLANE)
        {
            planeOffset = (surfaceEntry->YUVPlane == MHW_U_PLANE)
            ? &surface.UPlaneOffset
            : &surface.VPlaneOffset;

            surfaceInfos[btIndex].yOffset = planeOffset->iYOffset >> 1;
            surfaceInfos[btIndex].xOffset = (planeOffset->iXOffset/(uint32_t)sizeof(uint32_t)) >> 2;
        }
        else
        {
            surfaceInfos[btIndex].xOffset = (surface.YPlaneOffset.iXOffset/(uint32_t)sizeof(uint32_t)) >> 2;
            surfaceInfos[btIndex].yOffset = surface.YPlaneOffset.iYOffset >> 1;
        }

        ++tempPlaneIndex;
        ++taskParam->surfEntryInfoArrays.surfEntryInfosArray[curKernelIndex].usedIndex;

        eStatus = MOS_STATUS_SUCCESS;
        break;

    default:
        break;
    }

 finish:
        return eStatus;
}

uint32_t HalCm_GetFreeBindingIndex(
    PCM_HAL_STATE             state,
    PCM_HAL_INDEX_PARAM       indexParam,
    uint32_t                  total)
{
    CM_SURFACE_BTI_INFO surfBTIInfo;
    state->cmHalInterface->GetHwSurfaceBTIInfo(&surfBTIInfo);

    uint32_t btIndex = surfBTIInfo.normalSurfaceStart;
    uint32_t unAllocated = total;

    while (btIndex < 256 && unAllocated > 0)
    {
        uint32_t arrayIndex = btIndex >> 5;
        uint32_t bitMask = (uint32_t)0x1 << (btIndex % 32);
        if (indexParam->btArray[arrayIndex] & bitMask)
        {
            // oops, occupied
            if (unAllocated != total)
            {
                // clear previous allocation
                uint32_t allocated = total - unAllocated;
                uint32_t tmpIndex = btIndex - 1;
                while (allocated > 0)
                {
                    uint32_t arrayIndex = tmpIndex >> 5;
                    uint32_t bitMask = 1 << (tmpIndex % 32);
                    indexParam->btArray[arrayIndex] &= ~bitMask;
                    allocated--;
                    tmpIndex--;
                }
                // reset
                unAllocated = total;
            }
        }
        else
        {
            indexParam->btArray[arrayIndex] |= bitMask;
            unAllocated--;
        }
        btIndex++;
    }

    if (unAllocated == 0)
    {
        // found slot
        return btIndex - total;
    }

    // no slot
    return 0;
}

void HalCm_PreSetBindingIndex(
    PCM_HAL_INDEX_PARAM     indexParam,
    uint32_t                start,
    uint32_t                end)
{
    uint32_t btIndex;
    for ( btIndex = start; btIndex <= end ; btIndex++)
    {
        uint32_t arrayIndex = btIndex >> 5;
        uint32_t bitMask = 1 << (btIndex % 32);
        indexParam->btArray[arrayIndex] |= bitMask;
    }
}

//*-----------------------------------------------------------------------------
//| Purpose: Setup surface State with BTIndex
//| Returns: Result of the operation
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_Setup2DSurfaceStateWithBTIndex(
    PCM_HAL_STATE                      state,
    int32_t                            bindingTable,
    uint32_t                           surfIndex,
    uint32_t                           btIndex,
    bool                               pixelPitch)
{
    PRENDERHAL_INTERFACE            renderHal = state->renderHal;
    MOS_STATUS                  eStatus;
    RENDERHAL_SURFACE               surface;
    RENDERHAL_SURFACE_STATE_PARAMS  surfaceParam;
    PRENDERHAL_SURFACE_STATE_ENTRY  surfaceEntries[MHW_MAX_SURFACE_PLANES];
    int32_t                     nSurfaceEntries, i;
    uint16_t                    memObjCtl;
    uint32_t                    offsetSrc;
    PRENDERHAL_STATE_HEAP       stateHeap;

    eStatus         = MOS_STATUS_UNKNOWN;
    nSurfaceEntries = 0;

    if (surfIndex == CM_NULL_SURFACE)
    {
        return MOS_STATUS_SUCCESS;
    }

    memObjCtl = CM_DEFAULT_CACHE_TYPE;

    // check the surfIndex
    if (surfIndex >= state->cmDeviceParam.max2DSurfaceTableSize ||
        Mos_ResourceIsNull(&state->umdSurf2DTable[surfIndex].osResource) )
    {
        CM_ASSERTMESSAGE(
            "Invalid 2D surface array index '%d'", surfIndex);
        return MOS_STATUS_UNKNOWN;
    }

    // Check to see if surface is already assigned
    uint32_t nBTInTable = ( unsigned char )CM_INVALID_INDEX;
    if ( pixelPitch )
    {
        nBTInTable = state->bti2DIndexTable[ surfIndex ].BTI.samplerSurfIndex;
    }
    else
    {
        nBTInTable = state->bti2DIndexTable[ surfIndex ].BTI.regularSurfIndex;
    }

    if ( btIndex == nBTInTable )
    {
        nSurfaceEntries = state->bti2DIndexTable[ surfIndex ].nPlaneNumber;

        stateHeap = renderHal->pStateHeap;

        // Get Offset to Current Binding Table
        uint32_t offsetDst = ( stateHeap->iCurSshBufferIndex * stateHeap->dwSshIntanceSize ) +     // Points to the Base of Current SSH Buffer Instance
                            ( stateHeap->iBindingTableOffset ) +                       // Moves the pointer to Base of Array of Binding Tables
                            ( bindingTable * stateHeap->iBindingTableSize ) +         // Moves the pointer to a Particular Binding Table
                            ( btIndex * sizeof( uint32_t ) );                              // Move the pointer to correct entry

        uint32_t *bindingTableEntry = ( uint32_t *)( stateHeap->pSshBuffer + offsetDst );

        if ( pixelPitch )
        {
            MOS_SecureMemcpy( bindingTableEntry, sizeof( uint32_t ) * nSurfaceEntries, state->bti2DIndexTable[ surfIndex ].BTITableEntry.samplerBtiEntryPosition, sizeof( uint32_t ) * nSurfaceEntries );
        }
        else
        {
            MOS_SecureMemcpy( bindingTableEntry, sizeof( uint32_t ) * nSurfaceEntries, state->bti2DIndexTable[ surfIndex ].BTITableEntry.regularBtiEntryPosition, sizeof( uint32_t ) * nSurfaceEntries );
        }

        return MOS_STATUS_SUCCESS;
    }

    // Get Details of 2D surface and fill the surface
    CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_GetSurfaceAndRegister(state, &surface, CM_ARGUMENT_SURFACE2D, surfIndex, pixelPitch));

    // Setup 2D surface
    MOS_ZeroMemory(&surfaceParam, sizeof(surfaceParam));
    surfaceParam.Type       = renderHal->SurfaceTypeDefault;
    surfaceParam.Boundary   = RENDERHAL_SS_BOUNDARY_ORIGINAL;
    if (!pixelPitch) {
        surfaceParam.bWidthInDword_UV = true;
        surfaceParam.bWidthInDword_Y = true;
    }

    surfaceParam.bRenderTarget = isRenderTarget(state, surfIndex);

    //Cache configurations
    state->cmHalInterface->HwSetSurfaceMemoryObjectControl(memObjCtl, &surfaceParam);

    CM_CHK_MOSSTATUS_GOTOFINISH(renderHal->pfnSetupSurfaceState(
                  renderHal,
                  &surface,
                  &surfaceParam,
                  &nSurfaceEntries,
                  surfaceEntries,
                  nullptr));

    for (i = 0; i < nSurfaceEntries; i++)
    {
        // Bind the surface State
        CM_CHK_MOSSTATUS_GOTOFINISH(renderHal->pfnBindSurfaceState(
                        renderHal,
                        bindingTable,
                        btIndex + i,
                        surfaceEntries[i]));
    }

    state->bti2DIndexTable[ surfIndex ].nPlaneNumber = nSurfaceEntries;
    // Get Offset to Current Binding Table
    stateHeap = renderHal->pStateHeap;
    offsetSrc = ( stateHeap->iCurSshBufferIndex * stateHeap->dwSshIntanceSize ) +     // Points to the Base of Current SSH Buffer Instance
                        ( stateHeap->iBindingTableOffset ) +                       // Moves the pointer to Base of Array of Binding Tables
                        ( bindingTable * stateHeap->iBindingTableSize ) +         // Moves the pointer to a Particular Binding Table
                        ( btIndex * sizeof( uint32_t ) );                              // Move the pointer to correct entry

    if ( pixelPitch )
    {
        state->bti2DIndexTable[ surfIndex ].BTI.samplerSurfIndex = btIndex;
        state->bti2DIndexTable[ surfIndex ].BTITableEntry.samplerBtiEntryPosition = stateHeap->pSshBuffer + offsetSrc;
    }
    else
    {
        state->bti2DIndexTable[ surfIndex ].BTI.regularSurfIndex = btIndex;
        state->bti2DIndexTable[ surfIndex ].BTITableEntry.regularBtiEntryPosition = stateHeap->pSshBuffer + offsetSrc;
    }

    eStatus = MOS_STATUS_SUCCESS;

finish:
    return eStatus;
}

//*-----------------------------------------------------------------------------
//| Purpose: Setup Buffer surface State with BTIndex
//| Returns: Result of the operation
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_SetupBufferSurfaceStateWithBTIndex(
    PCM_HAL_STATE                      state,
    int32_t                            bindingTable,
    uint32_t                           surfIndex,
    uint32_t                           btIndex,
    bool                               pixelPitch)
{
    PRENDERHAL_INTERFACE            renderHal = state->renderHal;
    MOS_STATUS                      eStatus;
    RENDERHAL_SURFACE               surface;
    RENDERHAL_SURFACE_STATE_PARAMS  surfaceParam;
    PRENDERHAL_SURFACE_STATE_ENTRY  surfaceEntry;
    uint16_t                        memObjCtl;
    uint32_t                        offsetSrc;
    PRENDERHAL_STATE_HEAP           stateHeap;
    UNUSED(pixelPitch);

    eStatus              = MOS_STATUS_UNKNOWN;

    if (surfIndex == CM_NULL_SURFACE)
    {
        return MOS_STATUS_SUCCESS;
    }

    memObjCtl = CM_DEFAULT_CACHE_TYPE;

    // Check to see if surface is already assigned
    if ( btIndex == ( uint32_t )state->btiBufferIndexTable[ surfIndex ].BTI.regularSurfIndex )
    {
        uint32_t nSurfaceEntries = state->btiBufferIndexTable[ surfIndex ].nPlaneNumber;

        stateHeap = renderHal->pStateHeap;

        // Get Offset to Current Binding Table
        uint32_t offsetDst = ( stateHeap->iCurSshBufferIndex * stateHeap->dwSshIntanceSize ) +     // Points to the Base of Current SSH Buffer Instance
                            ( stateHeap->iBindingTableOffset ) +                       // Moves the pointer to Base of Array of Binding Tables
                            ( bindingTable * stateHeap->iBindingTableSize ) +               // Moves the pointer to a Particular Binding Table
                            ( btIndex * sizeof( uint32_t ) );                              // Move the pointer to correct entry

        uint32_t *bindingTableEntry = ( uint32_t *)( stateHeap->pSshBuffer + offsetDst );
        MOS_SecureMemcpy( bindingTableEntry, sizeof( uint32_t ) * nSurfaceEntries, state->btiBufferIndexTable[ surfIndex ].BTITableEntry.regularBtiEntryPosition, sizeof( uint32_t ) * nSurfaceEntries );

        return MOS_STATUS_SUCCESS;
    }

    // Get Details of Buffer surface and fill the surface
    CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_GetSurfaceAndRegister(state, &surface, CM_ARGUMENT_SURFACEBUFFER, surfIndex, 0));

    // set up buffer surface
    MOS_ZeroMemory(&surfaceParam, sizeof(surfaceParam));

    // Set bRenderTarget by default
    surfaceParam.bRenderTarget = true;

    // Setup Buffer surface
    CM_CHK_MOSSTATUS_GOTOFINISH(renderHal->pfnSetupBufferSurfaceState(
            renderHal,
            &surface,
            &surfaceParam,
            &surfaceEntry));

    //Cache configurations
    state->cmHalInterface->HwSetSurfaceMemoryObjectControl(memObjCtl, &surfaceParam);

    // Bind the surface State
    CM_CHK_MOSSTATUS_GOTOFINISH(renderHal->pfnBindSurfaceState(
               renderHal,
               bindingTable,
               btIndex,
               surfaceEntry));

    state->btiBufferIndexTable[ surfIndex ].BTI.regularSurfIndex = btIndex;
    state->btiBufferIndexTable[ surfIndex ].nPlaneNumber = 1;

    stateHeap = renderHal->pStateHeap;
    offsetSrc = ( stateHeap->iCurSshBufferIndex * stateHeap->dwSshIntanceSize ) +     // Points to the Base of Current SSH Buffer Instance
                        ( stateHeap->iBindingTableOffset ) +                       // Moves the pointer to Base of Array of Binding Tables
                        ( bindingTable * stateHeap->iBindingTableSize ) +         // Moves the pointer to a Particular Binding Table
                        ( btIndex * sizeof( uint32_t ) );                              // Move the pointer to correct entry

    state->btiBufferIndexTable[ surfIndex ].BTITableEntry.regularBtiEntryPosition = stateHeap->pSshBuffer + offsetSrc;

    eStatus = MOS_STATUS_SUCCESS;

finish:
    return eStatus;
}

MOS_STATUS HalCm_Setup2DSurfaceUPStateWithBTIndex(
    PCM_HAL_STATE                      state,
    int32_t                            bindingTable,
    uint32_t                           surfIndex,
    uint32_t                           btIndex,
    bool                               pixelPitch)
{
    MOS_STATUS                     eStatus;
    RENDERHAL_SURFACE              surface;
    RENDERHAL_SURFACE_STATE_PARAMS surfaceParam;
    PRENDERHAL_INTERFACE           renderHal;
    PRENDERHAL_SURFACE_STATE_ENTRY surfaceEntries[MHW_MAX_SURFACE_PLANES];
    int32_t                        nSurfaceEntries, i;
    uint16_t                       memObjCtl;
    uint32_t                       offsetSrc;
    PRENDERHAL_STATE_HEAP          stateHeap;

    eStatus              = MOS_STATUS_UNKNOWN;
    renderHal    = state->renderHal;

    if (surfIndex == CM_NULL_SURFACE)
    {
        return MOS_STATUS_SUCCESS;
    }

    memObjCtl = CM_DEFAULT_CACHE_TYPE;

    // Check to see if surface is already assigned
    uint32_t nBTInTable = ( unsigned char )CM_INVALID_INDEX;
    if ( pixelPitch )
    {
        nBTInTable = state->bti2DUPIndexTable[ surfIndex ].BTI.samplerSurfIndex;
    }
    else
    {
        nBTInTable = state->bti2DUPIndexTable[ surfIndex ].BTI.regularSurfIndex;
    }

    if ( btIndex == nBTInTable )
    {
        uint32_t nSurfaceEntries = state->bti2DUPIndexTable[ surfIndex ].nPlaneNumber;

        stateHeap = renderHal->pStateHeap;

        // Get Offset to Current Binding Table
        uint32_t offsetDst = ( stateHeap->iCurSshBufferIndex * stateHeap->dwSshIntanceSize ) +   // Points to the Base of Current SSH Buffer Instance
                            ( stateHeap->iBindingTableOffset ) +                       // Moves the pointer to Base of Array of Binding Tables
                            ( bindingTable * stateHeap->iBindingTableSize ) +         // Moves the pointer to a Particular Binding Table
                            ( btIndex * sizeof( uint32_t ) );                              // Move the pointer to correct entry

        uint32_t *bindingTableEntry = ( uint32_t *)( stateHeap->pSshBuffer + offsetDst );
        if ( pixelPitch )
        {
            MOS_SecureMemcpy( bindingTableEntry, sizeof( uint32_t ) * nSurfaceEntries, state->bti2DUPIndexTable[ surfIndex ].BTITableEntry.samplerBtiEntryPosition, sizeof( uint32_t ) * nSurfaceEntries );
        }
        else
        {
            MOS_SecureMemcpy( bindingTableEntry, sizeof( uint32_t ) * nSurfaceEntries, state->bti2DUPIndexTable[ surfIndex ].BTITableEntry.regularBtiEntryPosition, sizeof( uint32_t ) * nSurfaceEntries );
        }

        return MOS_STATUS_SUCCESS;
    }

    // Get Details of 2DUP surface and fill the surface
    CM_CHK_MOSSTATUS_GOTOFINISH( HalCm_GetSurfaceAndRegister( state, &surface, CM_ARGUMENT_SURFACE2D_UP, surfIndex, pixelPitch ) );

    // Setup 2D surface
    MOS_ZeroMemory( &surfaceParam, sizeof( surfaceParam ) );
    surfaceParam.Type = renderHal->SurfaceTypeDefault;
    surfaceParam.Boundary = RENDERHAL_SS_BOUNDARY_ORIGINAL;

    if ( !pixelPitch )
    {
        surfaceParam.bWidthInDword_UV = true;
        surfaceParam.bWidthInDword_Y = true;
    }

    surfaceParam.bRenderTarget = true;

    //Cache configurations
    state->cmHalInterface->HwSetSurfaceMemoryObjectControl(memObjCtl, &surfaceParam);

    CM_CHK_MOSSTATUS_GOTOFINISH(renderHal->pfnSetupSurfaceState(
                renderHal,
                &surface,
                &surfaceParam,
                &nSurfaceEntries,
                surfaceEntries,
                nullptr));

    for (i = 0; i < nSurfaceEntries; i++)
    {
        // Bind the surface State
        CM_CHK_MOSSTATUS_GOTOFINISH(renderHal->pfnBindSurfaceState(
                        renderHal,
                        bindingTable,
                        btIndex + i,
                        surfaceEntries[i]));
    }

    state->bti2DUPIndexTable[ surfIndex ].nPlaneNumber = nSurfaceEntries;

    stateHeap = renderHal->pStateHeap;
    offsetSrc = ( stateHeap->iCurSshBufferIndex * stateHeap->dwSshIntanceSize ) +     // Points to the Base of Current SSH Buffer Instance
                        ( stateHeap->iBindingTableOffset ) +                       // Moves the pointer to Base of Array of Binding Tables
                        ( bindingTable * stateHeap->iBindingTableSize ) +         // Moves the pointer to a Particular Binding Table
                        ( btIndex * sizeof( uint32_t ) );                              // Move the pointer to correct entry

    if ( pixelPitch )
    {
        state->bti2DUPIndexTable[ surfIndex ].BTI.samplerSurfIndex = btIndex;
        state->bti2DUPIndexTable[ surfIndex ].BTITableEntry.samplerBtiEntryPosition = stateHeap->pSshBuffer + offsetSrc;
    }
    else
    {
        state->bti2DUPIndexTable[ surfIndex ].BTI.regularSurfIndex = btIndex;
        state->bti2DUPIndexTable[ surfIndex ].BTITableEntry.regularBtiEntryPosition = stateHeap->pSshBuffer + offsetSrc;
    }

    eStatus = MOS_STATUS_SUCCESS;

finish:
    return eStatus;
}

MOS_STATUS HalCm_SetupSampler8x8SurfaceStateWithBTIndex(
    PCM_HAL_STATE           state,
    int32_t                 bindingTable,
    uint32_t                surfIndex,
    uint32_t                btIndex,
    bool                    pixelPitch,
    CM_HAL_KERNEL_ARG_KIND  kind,
    uint32_t                addressControl )
{
    MOS_STATUS                  eStatus;
    RENDERHAL_SURFACE               surface;
    RENDERHAL_SURFACE_STATE_PARAMS  surfaceParam;
    PRENDERHAL_INTERFACE            renderHal;
    PRENDERHAL_SURFACE_STATE_ENTRY  surfaceEntries[ MHW_MAX_SURFACE_PLANES ];
    int32_t                         nSurfaceEntries;
    uint16_t                        memObjCtl;
    int32_t                         i;
    uint32_t                        offsetSrc;
    PRENDERHAL_STATE_HEAP           stateHeap;
    UNUSED(pixelPitch);

    eStatus = MOS_STATUS_UNKNOWN;
    renderHal = state->renderHal;

    if ( surfIndex == CM_NULL_SURFACE )
    {
        eStatus = MOS_STATUS_SUCCESS;
        goto finish;
    }

    memObjCtl = CM_DEFAULT_CACHE_TYPE;

    // check to see if index is valid
    if ( surfIndex >= state->cmDeviceParam.max2DSurfaceTableSize ||
         Mos_ResourceIsNull( &state->umdSurf2DTable[ surfIndex ].osResource ) )
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        CM_ASSERTMESSAGE(
            "Invalid 2D surface array index '%d'", surfIndex );
        goto finish;
    }

    // Get Details of Sampler8x8 surface and fill the surface
    CM_CHK_MOSSTATUS_GOTOFINISH( HalCm_GetSurfaceAndRegister( state, &surface, kind, surfIndex, 0 ) );

    // Setup surface
    MOS_ZeroMemory( &surfaceParam, sizeof( surfaceParam ) );
    surfaceParam.Type = renderHal->SurfaceTypeAdvanced;
    surfaceParam.bRenderTarget = true;
    surfaceParam.bWidthInDword_Y = false;
    surfaceParam.bWidthInDword_UV = false;
    surfaceParam.Boundary = RENDERHAL_SS_BOUNDARY_ORIGINAL;
    surfaceParam.bVASurface = ( kind == CM_ARGUMENT_SURFACE_SAMPLER8X8_VA ) ? 1 : 0;
    surfaceParam.AddressControl = addressControl;
    state->cmHalInterface->HwSetSurfaceMemoryObjectControl(memObjCtl, &surfaceParam );
    renderHal->bEnableP010SinglePass = state->cmHalInterface->IsP010SinglePassSupported();
    nSurfaceEntries = 0;
    CM_CHK_MOSSTATUS_GOTOFINISH( renderHal->pfnSetupSurfaceState(
        renderHal,
        &surface,
        &surfaceParam,
        &nSurfaceEntries,
        surfaceEntries,
        nullptr ) );

    CM_ASSERT( nSurfaceEntries == 1 );

    for ( i = 0; i < nSurfaceEntries; i++ )
    {
        // Bind the surface State
        CM_CHK_MOSSTATUS_GOTOFINISH( renderHal->pfnBindSurfaceState(
            renderHal,
            bindingTable,
            btIndex + i,
            surfaceEntries[ i ] ) );
    }

    stateHeap = renderHal->pStateHeap;
    offsetSrc = ( stateHeap->iCurSshBufferIndex * stateHeap->dwSshIntanceSize ) +     // Points to the Base of Current SSH Buffer Instance
        ( stateHeap->iBindingTableOffset ) +                       // Moves the pointer to Base of Array of Binding Tables
        ( bindingTable * stateHeap->iBindingTableSize ) +         // Moves the pointer to a Particular Binding Table
        ( btIndex * sizeof( uint32_t ) );                              // Move the pointer to correct entry

    state->bti2DIndexTable[ surfIndex ].nPlaneNumber = nSurfaceEntries;
    state->bti2DIndexTable[ surfIndex ].BTITableEntry.sampler8x8BtiEntryPosition = stateHeap->pSshBuffer + offsetSrc;
    state->bti2DIndexTable[ surfIndex ].BTI.sampler8x8SurfIndex = btIndex;

    eStatus = MOS_STATUS_SUCCESS;

finish:
    renderHal->bEnableP010SinglePass = false;
    return eStatus;
}

//*-----------------------------------------------------------------------------
//| Purpose: Setup 3D surface State with BTIndex
//| Returns: Result of the operation
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_Setup3DSurfaceStateWithBTIndex(
    PCM_HAL_STATE                      state,
    int32_t                            bindingTable,
    uint32_t                           surfIndex,
    uint32_t                           btIndex)
{
    PRENDERHAL_INTERFACE            renderHal = state->renderHal;
    MOS_STATUS                      eStatus;
    RENDERHAL_SURFACE               surface;
    RENDERHAL_SURFACE_STATE_PARAMS  surfaceParam;
    PRENDERHAL_SURFACE_STATE_ENTRY  surfaceEntries[MHW_MAX_SURFACE_PLANES];
    int32_t                     nSurfaceEntries, i;
    uint16_t                    memObjCtl;
    uint32_t                    offsetSrc;
    PRENDERHAL_STATE_HEAP       stateHeap;

    eStatus = MOS_STATUS_UNKNOWN;
    nSurfaceEntries = 0;

    if (surfIndex == CM_NULL_SURFACE)
    {
        return MOS_STATUS_SUCCESS;
    }

    memObjCtl = CM_DEFAULT_CACHE_TYPE;

    // check the surfIndex
    if (surfIndex >= state->cmDeviceParam.max3DSurfaceTableSize ||
        Mos_ResourceIsNull(&state->surf3DTable[surfIndex].osResource))
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        CM_ASSERTMESSAGE(
            "Invalid 3D surface array index '%d'", surfIndex);
        return MOS_STATUS_UNKNOWN;
    }

    // Check to see if surface is already assigned
    uint32_t nBTInTable = (unsigned char)CM_INVALID_INDEX;
    nBTInTable = state->bti3DIndexTable[surfIndex].BTI.regularSurfIndex;

    if (btIndex == nBTInTable)
    {
        nSurfaceEntries = state->bti3DIndexTable[surfIndex].nPlaneNumber;

        stateHeap = renderHal->pStateHeap;

        // Get Offset to Current Binding Table
        uint32_t offsetDst = (stateHeap->iCurSshBufferIndex * stateHeap->dwSshIntanceSize) +     // Points to the Base of Current SSH Buffer Instance
            (stateHeap->iBindingTableOffset) +                       // Moves the pointer to Base of Array of Binding Tables
            (bindingTable * stateHeap->iBindingTableSize) +         // Moves the pointer to a Particular Binding Table
            (btIndex * sizeof(uint32_t));                              // Move the pointer to correct entry

        uint32_t *bindingTableEntry = (uint32_t*)(stateHeap->pSshBuffer + offsetDst);

        MOS_SecureMemcpy(bindingTableEntry, sizeof(uint32_t)* nSurfaceEntries, state->bti3DIndexTable[surfIndex].BTITableEntry.regularBtiEntryPosition, sizeof(uint32_t)* nSurfaceEntries);

        return MOS_STATUS_SUCCESS;
    }

    // Get Details of 3D surface and fill the surface
    CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_GetSurfaceAndRegister(state, &surface, CM_ARGUMENT_SURFACE3D, surfIndex, false));

    // Setup 3D surface
    MOS_ZeroMemory(&surfaceParam, sizeof(surfaceParam));
    surfaceParam.Type = renderHal->SurfaceTypeDefault;
    surfaceParam.Boundary = RENDERHAL_SS_BOUNDARY_ORIGINAL;

    //Cache configurations
    state->cmHalInterface->HwSetSurfaceMemoryObjectControl(memObjCtl, &surfaceParam);

    //Set bRenderTarget by default
    surfaceParam.bRenderTarget = true;

    CM_CHK_MOSSTATUS_GOTOFINISH(renderHal->pfnSetupSurfaceState(
        renderHal,
        &surface,
        &surfaceParam,
        &nSurfaceEntries,
        surfaceEntries,
        nullptr));

    for (i = 0; i < nSurfaceEntries; i++)
    {
        // Bind the surface State
        CM_CHK_MOSSTATUS_GOTOFINISH(renderHal->pfnBindSurfaceState(
            renderHal,
            bindingTable,
            btIndex + i,
            surfaceEntries[i]));
    }
    state->bti3DIndexTable[surfIndex].BTI.regularSurfIndex = btIndex;
    state->bti3DIndexTable[surfIndex].nPlaneNumber = nSurfaceEntries;
    // Get Offset to Current Binding Table
    stateHeap = renderHal->pStateHeap;
    offsetSrc = (stateHeap->iCurSshBufferIndex * stateHeap->dwSshIntanceSize) +     // Points to the Base of Current SSH Buffer Instance
        (stateHeap->iBindingTableOffset) +                       // Moves the pointer to Base of Array of Binding Tables
        (bindingTable * stateHeap->iBindingTableSize) +         // Moves the pointer to a Particular Binding Table
        (btIndex * sizeof(uint32_t));                              // Move the pointer to correct entry

    state->bti3DIndexTable[surfIndex].BTI.regularSurfIndex = btIndex;
    state->bti3DIndexTable[surfIndex].BTITableEntry.regularBtiEntryPosition = stateHeap->pSshBuffer + offsetSrc;

    eStatus = MOS_STATUS_SUCCESS;

finish:
    return eStatus;
}

//|-----------------------------------------------------------------------------
//| Purpose   : Tag-based Synchronization on Resource
//| Input     : state   - Hal CM State
//|             surface    surface
//|             isWrite  - Write or Read
//| Returns   : Result of the operation
//|-----------------------------------------------------------------------------
MOS_STATUS HalCm_SyncOnResource(
    PCM_HAL_STATE           state,
    PMOS_SURFACE            surface,
    bool                    isWrite)
{
    MOS_STATUS              eStatus;
    PMOS_INTERFACE          osInterface;

    eStatus           = MOS_STATUS_SUCCESS;
    osInterface = state->osInterface;

    if (surface == nullptr || Mos_ResourceIsNull(&surface->OsResource))
    {
        CM_ASSERTMESSAGE("Input resource is not valid.");
        eStatus = MOS_STATUS_UNKNOWN;
        return eStatus;
    }

    osInterface->pfnSyncOnResource(
            osInterface,
            &(surface->OsResource),
            state->osInterface->CurrentGpuContextOrdinal, //state->GpuContext,
            isWrite);

    // Sync Render Target with Overlay Context
    if (surface->bOverlay)
    {
        osInterface->pfnSyncOnOverlayResource(
            osInterface,
            &(surface->OsResource),
            state->osInterface->CurrentGpuContextOrdinal);
    }

    return eStatus;
}

//!
//! \brief    Send Media Walker State
//! \details  Send MEDIA_OBJECT_WALKER command
//! \param    PCM_HAL_STATE state
//!           [in] Pointer to CM_HAL_STATE Structure
//! \param    PRENDERHAL_INTERFACE renderHal
//!           [in] Pointer to Hardware Interface Structure
//! \param    PMOS_COMMAND_BUFFER cmdBuffer
//!           [in] Pointer to Command Buffer
//! \return   MOS_STATUS
//!
MOS_STATUS HalCm_SendMediaWalkerState(
    PCM_HAL_STATE               state,
    PCM_HAL_KERNEL_PARAM        kernelParam,
    PMOS_COMMAND_BUFFER         cmdBuffer)
{
    PRENDERHAL_INTERFACE      renderHal;
    MHW_WALKER_PARAMS         mediaWalkerParams;
    MOS_STATUS                eStatus;

    eStatus         = MOS_STATUS_SUCCESS;
    renderHal = state->renderHal;

    MOS_SecureMemcpy(&mediaWalkerParams, sizeof(MHW_WALKER_PARAMS), &kernelParam->walkerParams, sizeof(CM_HAL_WALKER_PARAMS));

    if (kernelParam->kernelThreadSpaceParam.threadSpaceWidth)
    {
        //per-kernel thread space is set, need use its own dependency mask
        mediaWalkerParams.UseScoreboard  = renderHal->VfeScoreboard.ScoreboardEnable;
        mediaWalkerParams.ScoreboardMask = kernelParam->kernelThreadSpaceParam.globalDependencyMask;
    }
    else
    {
        //No per-kernel thread space setting, need use per-task depedency mask
        mediaWalkerParams.UseScoreboard  = renderHal->VfeScoreboard.ScoreboardEnable;
        mediaWalkerParams.ScoreboardMask = renderHal->VfeScoreboard.ScoreboardMask;
    }

    eStatus = renderHal->pMhwRenderInterface->AddMediaObjectWalkerCmd(
                                  cmdBuffer, &mediaWalkerParams);

    return eStatus;
}

//!
//! \brief    Send GpGpu Walker State
//! \details  Send GPGPU_WALKER state
//! \param    PCM_HAL_STATE state
//!           [in] Pointer to CM_HAL_STATE Structure
//! \param    PRENDERHAL_INTERFACE renderHal
//!           [in] Pointer to Hardware Interface Structure
//! \param    PMOS_COMMAND_BUFFER cmdBuffer
//!           [in] Pointer to Command Buffer
//! \return   MOS_STATUS
//!
MOS_STATUS HalCm_SendGpGpuWalkerState(
    PCM_HAL_STATE               state,
    PCM_HAL_KERNEL_PARAM        kernelParam,
    PMOS_COMMAND_BUFFER         cmdBuffer)
{
    MhwRenderInterface           *mhwRender;
    MHW_GPGPU_WALKER_PARAMS      gpGpuWalkerParams;
    MOS_STATUS                   eStatus;

    eStatus           = MOS_STATUS_SUCCESS;
    mhwRender = state->renderHal->pMhwRenderInterface;

    gpGpuWalkerParams.InterfaceDescriptorOffset = kernelParam->gpgpuWalkerParams.interfaceDescriptorOffset;
    gpGpuWalkerParams.GpGpuEnable               = kernelParam->gpgpuWalkerParams.gpgpuEnabled;
    gpGpuWalkerParams.GroupWidth                = kernelParam->gpgpuWalkerParams.groupWidth;
    gpGpuWalkerParams.GroupHeight               = kernelParam->gpgpuWalkerParams.groupHeight;
    gpGpuWalkerParams.GroupDepth               = kernelParam->gpgpuWalkerParams.groupDepth;
    gpGpuWalkerParams.ThreadWidth               = kernelParam->gpgpuWalkerParams.threadWidth;
    gpGpuWalkerParams.ThreadHeight              = kernelParam->gpgpuWalkerParams.threadHeight;
    gpGpuWalkerParams.ThreadDepth               = kernelParam->gpgpuWalkerParams.threadDepth;
    gpGpuWalkerParams.SLMSize                   = kernelParam->slmSize;

    eStatus = mhwRender->AddGpGpuWalkerStateCmd(cmdBuffer, &gpGpuWalkerParams);

    return eStatus;
}

//!
//! \brief    surface Format Convert
//! \details  Convert RENDERHAL_SURFACE to MHW_VEBOX_SURFACE
//! \param    PRENDERHAL_SURFACE            renderHalSurface
//!           [in] Pointer to RENDERHAL_SURFACE Structure
//! \param    PMHW_VEBOX_SURFACE_PARAMS    mhwVeboxSurface
//!           [in] Pointer to PMHW_VEBOX_SURFACE_PARAMS
//! \return   MOS_STATUS
//!
MOS_STATUS HalCm_Convert_RENDERHAL_SURFACE_To_MHW_VEBOX_SURFACE(
    PRENDERHAL_SURFACE              renderHalSurface,
    PMHW_VEBOX_SURFACE_PARAMS    mhwVeboxSurface)
{
    PMOS_SURFACE                    surface;
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;

    CM_CHK_NULL_GOTOFINISH_MOSERROR(renderHalSurface);
    CM_CHK_NULL_GOTOFINISH_MOSERROR(mhwVeboxSurface);

    surface = &renderHalSurface->OsSurface;
    mhwVeboxSurface->Format        = surface->Format;
    mhwVeboxSurface->dwWidth       = surface->dwWidth;
    mhwVeboxSurface->dwHeight      = surface->dwHeight;
    mhwVeboxSurface->dwPitch       = surface->dwPitch;
    if (surface->dwPitch > 0)
    {
        mhwVeboxSurface->dwUYoffset = ((surface->UPlaneOffset.iSurfaceOffset - surface->YPlaneOffset.iSurfaceOffset) / surface->dwPitch)
                                      + surface->UPlaneOffset.iYOffset;
    }
    mhwVeboxSurface->TileType      = surface->TileType;
    mhwVeboxSurface->rcMaxSrc      = renderHalSurface->rcMaxSrc;
    mhwVeboxSurface->pOsResource   = &surface->OsResource;

finish:
    return eStatus;
}

//!
//! \brief    Set Vtune Profiling Flag
//! \details  Trun Vtune Profiling Flag On or off
//! \param    PCM_HAL_STATE state
//!           [in] Pointer to CM_HAL_STATE Structure
//! \return   MOS_STATUS_SUCCESS
//!
MOS_STATUS HalCm_SetVtuneProfilingFlag(
    PCM_HAL_STATE               state,
    bool                        vtuneOn)
{

    state->vtuneProfilerOn   = vtuneOn;

    return MOS_STATUS_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get the offset for the Task Sync Location given the task ID
//| Returns:    Sync Location
//*-----------------------------------------------------------------------------
int32_t HalCm_GetTaskSyncLocation(
    PCM_HAL_STATE       state,
    int32_t             taskId)        // [in] Task ID
{
    return (taskId * state->cmHalInterface->GetTimeStampResourceSize());
}

void HalCm_GetLegacyRenderHalL3Setting( CmHalL3Settings *l3SettingsPtr, RENDERHAL_L3_CACHE_SETTINGS *l3SettingsLegacyPtr )
{
    *l3SettingsLegacyPtr = {};
    l3SettingsLegacyPtr->bOverride = l3SettingsPtr->overrideSettings;
    l3SettingsLegacyPtr->bEnableSLM = l3SettingsPtr->enableSlm;
    l3SettingsLegacyPtr->bL3CachingEnabled = l3SettingsPtr->l3CachingEnabled;
    l3SettingsLegacyPtr->bCntlRegOverride = l3SettingsPtr->cntlRegOverride;
    l3SettingsLegacyPtr->bCntlReg2Override = l3SettingsPtr->cntlReg2Override;
    l3SettingsLegacyPtr->bCntlReg3Override = l3SettingsPtr->cntlReg3Override;
    l3SettingsLegacyPtr->bSqcReg1Override = l3SettingsPtr->sqcReg1Override;
    l3SettingsLegacyPtr->bSqcReg4Override = l3SettingsPtr->sqcReg4Override;
    l3SettingsLegacyPtr->bLra1RegOverride = l3SettingsPtr->lra1RegOverride;
    l3SettingsLegacyPtr->dwCntlReg = l3SettingsPtr->cntlReg;
    l3SettingsLegacyPtr->dwCntlReg2 = l3SettingsPtr->cntlReg2;
    l3SettingsLegacyPtr->dwCntlReg3 = l3SettingsPtr->cntlReg3;
    l3SettingsLegacyPtr->dwSqcReg1 = l3SettingsPtr->sqcReg1;
    l3SettingsLegacyPtr->dwSqcReg4 = l3SettingsPtr->sqcReg4;
    l3SettingsLegacyPtr->dwLra1Reg = l3SettingsPtr->lra1Reg;

    return;
}

uint64_t HalCm_ConvertTicksToNanoSeconds(
    PCM_HAL_STATE               state,
    uint64_t                    ticks)
{
    if (state->tsFrequency == 0)
    {
        // if KMD doesn't report an valid value, fall back to default configs
        return state->cmHalInterface->ConverTicksToNanoSecondsDefault(ticks);
    }
    return (ticks * 1000000000) / (state->tsFrequency);
}

//!
//! \brief    Check GPU context
//! \details  Check if the GPU context is valid for CM layer
//! \param    MOS_GPU_CONTEXT gpuContext
//!           [in] GPU Context ordinal
//! \return   true/false
//!
bool HalCm_IsValidGpuContext(
    MOS_GPU_CONTEXT             gpuContext)
{
    if( gpuContext == MOS_GPU_CONTEXT_RENDER3
     || gpuContext == MOS_GPU_CONTEXT_RENDER4
     || gpuContext == MOS_GPU_CONTEXT_CM_COMPUTE
     || gpuContext == MOS_GPU_CONTEXT_VEBOX)
    {
        return true;
    }
    else
    {
        CM_ASSERTMESSAGE("Invalid GPU context for CM.");
        return false;
    }
}
