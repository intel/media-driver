/*
* Copyright (c) 2014-2018, Intel Corporation
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
//! \file     mhw_vebox.cpp
//! \brief    MHW interface for constructing commands for the VEBOX
//! \details  Impelements the functionalities common across all platforms for MHW_VEBOX
//!

#include "mhw_utilities.h"
#include "mhw_vebox.h"

void MhwVeboxInterface::RefreshVeboxSync()
{
    PMHW_VEBOX_HEAP             pVeboxHeap;
    PMHW_VEBOX_HEAP_STATE       pCurInstance;
    PMOS_INTERFACE              pOsInterface;
    uint32_t                    dwCurrentTag;
    int32_t                     i;
    int32_t                     iInstanceInUse;
    MOS_NULL_RENDERING_FLAGS    NullRenderingFlags;

    MHW_FUNCTION_ENTER;
    if (m_veboxHeap == nullptr ||
        m_osInterface == nullptr )
    {
        MHW_ASSERTMESSAGE("RefreshVeboxSync failed due to m_veboxHeap or m_osInterface is invalid ");
        return;
    }
    iInstanceInUse = 0;

    // Vebox Heap will always be locked by driver
    pVeboxHeap   = m_veboxHeap;
    pOsInterface = m_osInterface;

    // Most recent tag
    if (pOsInterface->bEnableKmdMediaFrameTracking)
    {
        dwCurrentTag = pOsInterface->pfnGetGpuStatusSyncTag(pOsInterface, MOS_GPU_CONTEXT_VEBOX);
    }
    else
    {
        dwCurrentTag = pVeboxHeap->pSync[0];
    }
    pVeboxHeap->dwSyncTag = dwCurrentTag - 1;

    NullRenderingFlags  = m_osInterface->pfnGetNullHWRenderFlags(
        m_osInterface);

    // Refresh VeboxHeap states
    pCurInstance = pVeboxHeap->pStates;
    for (i = m_veboxSettings.uiNumInstances; i > 0; i--, pCurInstance++)
    {
        if (!pCurInstance->bBusy) continue;

        // The condition below is valid when sync tag wraps from 2^32-1 to 0
        if (((int32_t)(dwCurrentTag - pCurInstance->dwSyncTag) >= 0) ||
            NullRenderingFlags.VPGobal ||
            NullRenderingFlags.VPDnDi)
        {
            pCurInstance->bBusy = false;
        }
        else
        {
            iInstanceInUse++;
        }
    }

    // Save number of instance in use
    m_veboxHeapInUse = iInstanceInUse;
}

MOS_STATUS MhwVeboxInterface::AssignVeboxState()
{
    uint32_t                dwWaitMs, dwWaitTag;
    MOS_STATUS              eStatus = MOS_STATUS_SUCCESS;
    PMOS_INTERFACE          pOsInterface;
    PMHW_VEBOX_HEAP_STATE   pVeboxCurState;
    PMHW_VEBOX_HEAP         pVeboxHeap;
    uint32_t                uiOffset;

    MHW_FUNCTION_ENTER;
    MHW_CHK_NULL(m_veboxHeap);
    MHW_CHK_NULL(m_osInterface);

    pVeboxHeap     = m_veboxHeap;
    pVeboxCurState = &m_veboxHeap->pStates[pVeboxHeap->uiNextState];
    pOsInterface   = m_osInterface;

    // Refresh sync tag for all vebox heap instance
    RefreshVeboxSync();

    // Check validity of  current vebox heap instance
    // The code below is unlikely to be executed - unless all Vebox states are in use
    // If this ever happens, please consider increasing the number of media states
    MHW_CHK_NULL(pVeboxCurState);
    if (pVeboxCurState->bBusy)
    {
        // Get current vebox instance sync tag
        dwWaitTag = pVeboxCurState->dwSyncTag;

        // Wait for Batch Buffer complete event OR timeout
        for (dwWaitMs = MHW_TIMEOUT_MS_DEFAULT; dwWaitMs > 0; dwWaitMs--)
        {
            uint32_t dwCurrentTag;

            MHW_CHK_STATUS(pOsInterface->pfnWaitForBBCompleteNotifyEvent(
                pOsInterface,
                MOS_GPU_CONTEXT_VEBOX,
                MHW_EVENT_TIMEOUT_MS));

            if (pOsInterface->bEnableKmdMediaFrameTracking)
            {
                dwCurrentTag = pOsInterface->pfnGetGpuStatusSyncTag(pOsInterface, MOS_GPU_CONTEXT_VEBOX);
            }
            else
            {
                dwCurrentTag = pVeboxHeap->pSync[0];
            }
            // Mark current instance status as availabe. Wait if this sync tag came back from GPU
            if ((int32_t)(dwCurrentTag - dwWaitTag) >= 0)
            {
                pVeboxCurState->bBusy = false;
                break;
            }
        }

        // Timeout
        if (dwWaitMs == 0)
        {
            MHW_ASSERTMESSAGE("Timeout on waiting for free Vebox Heap.");
            eStatus = MOS_STATUS_UNKNOWN;
            goto finish;
        }
    }

    // Prepare syncTag for GPU write back
    if (pOsInterface->bEnableKmdMediaFrameTracking)
    {
        pVeboxCurState->dwSyncTag = pOsInterface->pfnGetGpuStatusTag(pOsInterface, MOS_GPU_CONTEXT_VEBOX);
    }
    else
    {
        pVeboxCurState->dwSyncTag = pVeboxHeap->dwNextTag;
    }

    // Assign current state and increase next state
    pVeboxHeap->uiCurState  = pVeboxHeap->uiNextState;
    pVeboxHeap->uiNextState = (pVeboxHeap->uiNextState + 1) %
                              (m_veboxSettings.uiNumInstances);

    //Clean the memory of current veboxheap to avoid the history states
    uiOffset = pVeboxHeap->uiCurState * pVeboxHeap->uiInstanceSize;
    MOS_ZeroMemory(pVeboxHeap->pLockedDriverResourceMem + uiOffset, pVeboxHeap->uiInstanceSize);

finish:
    return eStatus;
}

MOS_STATUS MhwVeboxInterface::UpdateVeboxSync()
{
    PMHW_VEBOX_HEAP          pVeboxHeap;
    MOS_STATUS               eStatus = MOS_STATUS_SUCCESS;
    PMOS_INTERFACE           pOsInterface;

    MHW_FUNCTION_ENTER;

    MHW_CHK_NULL(m_veboxHeap);
    MHW_CHK_NULL(m_osInterface);

    pVeboxHeap      = m_veboxHeap;
    pOsInterface    = m_osInterface;

    // If KMD frame tracking is on, the dwSyncTag has been set to gpu status tag
    // in Mhw_VeboxInterface_AssignVeboxState(). dwNextTag is not used anymore.
    if (!pOsInterface->bEnableKmdMediaFrameTracking)
    {
        pVeboxHeap->pStates[pVeboxHeap->uiCurState].dwSyncTag =
            pVeboxHeap->dwNextTag++;
    }
    pVeboxHeap->pStates[pVeboxHeap->uiCurState].bBusy = true;

finish:
    return eStatus;
}

MOS_STATUS MhwVeboxInterface::GetVeboxHeapInfo(
    const MHW_VEBOX_HEAP     **ppVeboxHeap)
{
    MOS_STATUS               eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;
    MHW_CHK_NULL(ppVeboxHeap);

    *ppVeboxHeap = (const MHW_VEBOX_HEAP *)m_veboxHeap;

finish:
    return eStatus;
}

MOS_STATUS MhwVeboxInterface::CreateHeap( )
{
    MOS_STATUS              eStatus;
    uint8_t                 *pMem;
    uint32_t                uiSize;
    uint32_t                uiOffset;
    MOS_ALLOC_GFXRES_PARAMS AllocParams;
    MOS_LOCK_PARAMS         LockFlags;

    eStatus         = MOS_STATUS_SUCCESS;

    uiSize =  sizeof(MHW_VEBOX_HEAP);
    uiSize += m_veboxSettings.uiNumInstances *
              sizeof(MHW_VEBOX_HEAP_STATE);

    // Allocate memory for VEBOX
    pMem = (uint8_t*)MOS_AllocAndZeroMemory(uiSize);
    MHW_CHK_NULL(pMem);

    m_veboxHeap = (PMHW_VEBOX_HEAP)pMem;

    m_veboxHeap->pStates =
    (PMHW_VEBOX_HEAP_STATE)(pMem + sizeof(MHW_VEBOX_HEAP));

    // Assign offsets and sizes
    uiOffset = 0;
    m_veboxHeap->uiDndiStateOffset = uiOffset;
    uiOffset += m_veboxSettings.uiDndiStateSize;

    m_veboxHeap->uiIecpStateOffset = uiOffset;
    uiOffset += m_veboxSettings.uiIecpStateSize;

    m_veboxHeap->uiGamutStateOffset = uiOffset;
    uiOffset += m_veboxSettings.uiGamutStateSize;

    m_veboxHeap->uiVertexTableOffset = uiOffset;
    uiOffset += m_veboxSettings.uiVertexTableSize;

    m_veboxHeap->uiCapturePipeStateOffset = uiOffset;
    uiOffset += m_veboxSettings.uiCapturePipeStateSize;

    m_veboxHeap->uiGammaCorrectionStateOffset = uiOffset;
    uiOffset += m_veboxSettings.uiGammaCorrectionStateSize;

    m_veboxHeap->ui3DLUTStateOffset = uiOffset;
    uiOffset += m_veboxSettings.ui3DLUTStateSize;

    m_veboxHeap->ui1DLUTStateOffset = uiOffset;
    uiOffset += m_veboxSettings.ui1DLUTStateSize;

    m_veboxHeap->uiInstanceSize = uiOffset;

    // Appending VeboxHeap sync data after all vebox heap instances
    m_veboxHeap->uiOffsetSync   =
        m_veboxHeap->uiInstanceSize *
        m_veboxSettings.uiNumInstances;

    // Allocate GPU memory
    uiSize = m_veboxHeap->uiInstanceSize *
             m_veboxSettings.uiNumInstances +
             m_veboxSettings.uiSyncSize;

    // for using vdbox copy, the size have to be cache line aligned
    MOS_ALIGN_CEIL(uiSize, MHW_CACHELINE_SIZE);

    m_veboxHeap->uiStateHeapSize = uiSize;

    MOS_ZeroMemory(&AllocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));

    AllocParams.Type     = MOS_GFXRES_BUFFER;
    AllocParams.TileType = MOS_TILE_LINEAR;
    AllocParams.Format   = Format_Buffer;
    AllocParams.dwBytes  = uiSize;
    AllocParams.pBufName = "VphalVeboxHeap";

    MHW_CHK_STATUS(m_osInterface->pfnAllocateResource(
        m_osInterface,
        &AllocParams,
        &m_veboxHeap->DriverResource));

    MHW_CHK_STATUS(m_osInterface->pfnAllocateResource(
        m_osInterface,
        &AllocParams,
        &m_veboxHeap->KernelResource));

    // Lock the driver resource
    MOS_ZeroMemory(&LockFlags, sizeof(MOS_LOCK_PARAMS));

    LockFlags.NoOverWrite = 1;

    m_veboxHeap->pLockedDriverResourceMem =
        (uint8_t*)m_osInterface->pfnLockResource(
                            m_osInterface,
                            &m_veboxHeap->DriverResource,
                            &LockFlags);
    MHW_CHK_NULL(m_veboxHeap->pLockedDriverResourceMem);

    // Initialize VeboxHeap controls that depend on mapping
    m_veboxHeap->pSync =
        (uint32_t*) (m_veboxHeap->pLockedDriverResourceMem +
                  m_veboxHeap->uiOffsetSync);

finish:
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        DestroyHeap();
    }
    return eStatus;
}

MOS_STATUS MhwVeboxInterface::DestroyHeap()
{
    PMOS_INTERFACE       pOsInterface;
    MOS_STATUS           eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;
    MHW_CHK_NULL(m_osInterface);

    pOsInterface = m_osInterface;

    if (m_veboxHeap)
    {
        if (!Mos_ResourceIsNull(&m_veboxHeap->DriverResource))
        {
            if (m_veboxHeap->pLockedDriverResourceMem)
            {
                pOsInterface->pfnUnlockResource(
                    pOsInterface,
                    &m_veboxHeap->DriverResource);
            }

            pOsInterface->pfnFreeResource(
                pOsInterface,
                &m_veboxHeap->DriverResource);
        }

        if (!Mos_ResourceIsNull(&m_veboxHeap->KernelResource))
        {
            pOsInterface->pfnFreeResource(
                pOsInterface,
                &m_veboxHeap->KernelResource);
        }

        MOS_FreeMemory(m_veboxHeap);
        m_veboxHeap = nullptr;
    }

finish:
    return eStatus;
}

MhwVeboxInterface::MhwVeboxInterface(PMOS_INTERFACE pOsInterface)
{
    MHW_FUNCTION_ENTER;

    MOS_ZeroMemory(&m_veboxSettings, sizeof(m_veboxSettings));
    pfnAddResourceToCmd = nullptr;

    if (pOsInterface == nullptr)
    {
        MHW_ASSERTMESSAGE("Invalid input pointers provided");
        return;
    }
    m_osInterface   = pOsInterface;

    if (m_osInterface->bUsesGfxAddress)
    {
        pfnAddResourceToCmd = Mhw_AddResourceToCmd_GfxAddress;
    }
    else  //PatchList
    {
        pfnAddResourceToCmd = Mhw_AddResourceToCmd_PatchList;
    }
}
