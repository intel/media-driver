/*
* Copyright (c) 2024, Intel Corporation
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
//! \file     surface_state_heap_mgr.cpp
//! \brief    Render Engine state heap manager for VP and CM
//! \details  Platform/OS Independent Render Engine state heap management interfaces
//!
#include "surface_state_heap_mgr.h"
#include "vp_utils.h"

SurfaceStateHeapManager::SurfaceStateHeapManager(PMOS_INTERFACE pOsInterface) : m_osInterface(pOsInterface)
{
}

SurfaceStateHeapManager::~SurfaceStateHeapManager()
{
    DestroyHeap();
}

MOS_STATUS SurfaceStateHeapManager::CreateHeap(size_t surfStateSize)
{
    uint8_t                *pMem;
    uint32_t                uiSize;
    uint32_t                uiOffset;
    MOS_ALLOC_GFXRES_PARAMS AllocParams;
    MOS_LOCK_PARAMS         LockFlags;
    MOS_STATUS              eStatus = MOS_STATUS_SUCCESS;

    uiSize = sizeof(SURFACE_STATES_HEAP_OBJ);
    uiSize += MAX_SURFACE_STATES *
              sizeof(SURFACE_STATES_OBJ);

    // Allocate memory for render surface state heap
    pMem = (uint8_t *)MOS_AllocAndZeroMemory(uiSize);
    MHW_CHK_NULL_RETURN(pMem);

    m_surfStateHeap = (SURFACE_STATES_HEAP_OBJ *)pMem;

    m_surfStateHeap->pSurfStateObj =
        (SURFACE_STATES_OBJ *)(pMem + sizeof(SURFACE_STATES_HEAP_OBJ));

    // Appending sync data after all heap instances
    m_surfStateHeap->uiOffsetSync = surfStateSize * MAX_SURFACE_STATES;

    // Allocate GPU memory
    uiSize = surfStateSize * MAX_SURFACE_STATES + SYNC_SIZE;

    m_surfStateHeap->uiStateHeapSize = uiSize;
    m_surfStateHeap->uiInstanceSize = surfStateSize;

    MOS_ZeroMemory(&AllocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));

    AllocParams.Type         = MOS_GFXRES_BUFFER;
    AllocParams.TileType     = MOS_TILE_LINEAR;
    AllocParams.Format       = Format_Buffer;
    AllocParams.dwBytes      = uiSize;
    AllocParams.pBufName     = "VphalSurfaceState";
    AllocParams.ResUsageType = MOS_HW_RESOURCE_USAGE_VP_INTERNAL_READ_WRITE_RENDER;
    AllocParams.dwMemType    = MOS_MEMPOOL_VIDEOMEMORY;

    MHW_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
        m_osInterface,
        &AllocParams,
        &m_surfStateHeap->osResource));

    // Lock the driver resource
    MOS_ZeroMemory(&LockFlags, sizeof(MOS_LOCK_PARAMS));

    LockFlags.NoOverWrite = 1;

    m_surfStateHeap->pLockedOsResourceMem =
        (uint8_t *)m_osInterface->pfnLockResource(
            m_osInterface,
            &m_surfStateHeap->osResource,
            &LockFlags);
    MHW_CHK_NULL_RETURN(m_surfStateHeap->pLockedOsResourceMem);

    // Initialize VeboxHeap controls that depend on mapping
    m_surfStateHeap->pSync =
        (uint32_t *)(m_surfStateHeap->pLockedOsResourceMem +
                     m_surfStateHeap->uiOffsetSync);

    return eStatus;
}

MOS_STATUS SurfaceStateHeapManager::DestroyHeap()
{
    if (m_surfStateHeap)
    {
        if (!Mos_ResourceIsNull(&m_surfStateHeap->osResource))
        {
            if (m_surfStateHeap->pLockedOsResourceMem)
            {
                m_osInterface->pfnUnlockResource(
                    m_osInterface,
                    &m_surfStateHeap->osResource);
            }

            m_osInterface->pfnFreeResource(
                m_osInterface,
                &m_surfStateHeap->osResource);
        }

        MOS_FreeMemory(m_surfStateHeap);
        m_surfStateHeap = nullptr;
    }
    return MOS_STATUS_SUCCESS;
}

void SurfaceStateHeapManager::RefreshSync()
{
    SURFACE_STATES_OBJ      *pCurInstance;
    SURFACE_STATES_HEAP_OBJ *pSurfStateHeap;
    uint32_t                 dwCurrentTag;
    int32_t                  i;
    int32_t                  iInstanceInUse;
    MOS_NULL_RENDERING_FLAGS NullRenderingFlags;

    MHW_FUNCTION_ENTER;
    if (m_surfStateHeap == nullptr ||
        m_osInterface == nullptr)
    {
        MHW_ASSERTMESSAGE("RefreshSync failed due to m_veboxHeap or m_osInterface is invalid ");
        return;
    }
    iInstanceInUse = 0;

    // Vebox Heap will always be locked by driver
    pSurfStateHeap = m_surfStateHeap;

    // Most recent tag
    if (m_osInterface->bEnableKmdMediaFrameTracking)
    {
        dwCurrentTag = m_osInterface->pfnGetGpuStatusSyncTag(m_osInterface, MOS_GPU_CONTEXT_COMPUTE);
    }
    else
    {
        dwCurrentTag = pSurfStateHeap->pSync[0];
    }
    pSurfStateHeap->dwSyncTag = dwCurrentTag - 1;

    NullRenderingFlags = m_osInterface->pfnGetNullHWRenderFlags(
        m_osInterface);

    // Refresh VeboxHeap states
    pCurInstance = pSurfStateHeap->pSurfStateObj;
    for (i = MAX_SURFACE_STATES; i > 0; i--, pCurInstance++)
    {
        if (!pCurInstance->bBusy)
            continue;

        // The condition below is valid when sync tag wraps from 2^32-1 to 0
        if (((int32_t)(dwCurrentTag - pCurInstance->dwSyncTag) >= 0) ||
            NullRenderingFlags.VPGobal)
        {
            pCurInstance->bBusy = false;
        }
        else
        {
            iInstanceInUse++;
        }
    }

    // Save number of instance in use
    m_surfHeapInUse = iInstanceInUse;
}

MOS_STATUS SurfaceStateHeapManager::AssignSurfaceState()
{
    VP_FUNC_CALL();

    uint32_t   dwWaitMs, dwWaitTag;
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    SURFACE_STATES_OBJ      *pSurfStateCurObj;
    SURFACE_STATES_HEAP_OBJ *pSurfStateHeap;
    uint32_t                 uiOffset;

    MHW_FUNCTION_ENTER;
    MHW_CHK_NULL_RETURN(m_surfStateHeap);
    MHW_CHK_NULL_RETURN(m_osInterface);

    pSurfStateHeap = m_surfStateHeap;
    pSurfStateCurObj = &m_surfStateHeap->pSurfStateObj[m_surfStateHeap->uiNextState];

    // Refresh sync tag for all heap instance
    RefreshSync();

    // Check validity of  current vebox heap instance
    // The code below is unlikely to be executed - unless all Vebox states are in use
    // If this ever happens, please consider increasing the number of media states
    MHW_CHK_NULL_RETURN(pSurfStateCurObj);
    if (pSurfStateCurObj->bBusy)
    {
        // Get current vebox instance sync tag
        dwWaitTag = pSurfStateCurObj->dwSyncTag;

        // Wait for Batch Buffer complete event OR timeout
        for (dwWaitMs = MHW_TIMEOUT_MS_DEFAULT; dwWaitMs > 0; dwWaitMs--)
        {
            uint32_t dwCurrentTag;

            MHW_CHK_STATUS_RETURN(m_osInterface->pfnWaitForBBCompleteNotifyEvent(
                m_osInterface,
                MOS_GPU_CONTEXT_COMPUTE,
                MHW_EVENT_TIMEOUT_MS));

            if (m_osInterface->bEnableKmdMediaFrameTracking)
            {
                dwCurrentTag = m_osInterface->pfnGetGpuStatusSyncTag(m_osInterface, MOS_GPU_CONTEXT_COMPUTE);
            }
            else
            {
                dwCurrentTag = pSurfStateHeap->pSync[0];
            }
            // Mark current instance status as availabe. Wait if this sync tag came back from GPU
            if ((int32_t)(dwCurrentTag - dwWaitTag) >= 0)
            {
                pSurfStateCurObj->bBusy = false;
                break;
            }
        }

        // Timeout
        if (dwWaitMs == 0)
        {
            MHW_ASSERTMESSAGE("Timeout on waiting for free Vebox Heap.");
            eStatus = MOS_STATUS_UNKNOWN;
            return eStatus;
        }
    }

    // Prepare syncTag for GPU write back
    if (m_osInterface->bEnableKmdMediaFrameTracking)
    {
        pSurfStateCurObj->dwSyncTag = m_osInterface->pfnGetGpuStatusTag(m_osInterface, MOS_GPU_CONTEXT_COMPUTE);
    }
    else
    {
        pSurfStateCurObj->dwSyncTag = pSurfStateHeap->dwNextTag;
    }

    // Assign current state and increase next state
    pSurfStateHeap->uiCurState  = pSurfStateHeap->uiNextState;
    pSurfStateHeap->uiNextState = (pSurfStateHeap->uiNextState + 1) % MAX_SURFACE_STATES;

    //Clean the memory of current veboxheap to avoid the history states
    uiOffset = pSurfStateHeap->uiCurState * pSurfStateHeap->uiInstanceSize;
    MOS_ZeroMemory(pSurfStateHeap->pLockedOsResourceMem + uiOffset, pSurfStateHeap->uiInstanceSize);

    return eStatus;
}