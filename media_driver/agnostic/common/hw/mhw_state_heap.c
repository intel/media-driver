/*
* Copyright (c) 2014-2017, Intel Corporation
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
//! \file      mhw_state_heap.c 
//! \brief         This modules implements HW interface layer to be used on all platforms on     all operating systems/DDIs, across MHW components. 
//!
#include "mhw_state_heap.h"
#include "mhw_utilities.h"
#include "mhw_block_manager.h"
#include "media_interfaces_mhw.h"
#include "mhw_mi.h"
#include "mhw_cp_interface.h"

extern const uint8_t g_cMhw_VDirection[MHW_NUM_FRAME_FIELD_TYPES] = {
    MEDIASTATE_VDIRECTION_FULL_FRAME,
    MEDIASTATE_VDIRECTION_TOP_FIELD,
    MEDIASTATE_VDIRECTION_BOTTOM_FIELD
};

extern const MHW_SAMPLER_STATE_UNORM_PARAM g_cInit_MhwSamplerStateUnormParam =
{
    MHW_SAMPLER_FILTER_CUSTOM,                      // SamplerFilterMode
    MHW_GFX3DSTATE_MAPFILTER_LINEAR,                // MagFilter
    MHW_GFX3DSTATE_MAPFILTER_LINEAR,                // MinFilter
    MHW_GFX3DSTATE_TEXCOORDMODE_CLAMP,              // AddressU
    MHW_GFX3DSTATE_TEXCOORDMODE_CLAMP,              // AddressV
    MHW_GFX3DSTATE_TEXCOORDMODE_CLAMP,              // AddressW
    MHW_SAMPLER_SURFACE_PIXEL_UINT,
    {0},
    {0},
    {0},
    {0},
    0,
    0,
    0
};

extern const MHW_SURFACE_STATE_PARAMS g_cInit_MhwSurfaceStateParams =
{
    nullptr,     // pSurfaceState

    0,        // dwCacheabilityControl
    0,        // dwFormat
    0,        // dwWidth
    0,        // dwHeight
    0,        // dwDepth
    0,        // dwPitch
    0,        // dwQPitch
    0,        // bUseAdvState
    0,        // AddressControl
    0,        // SurfaceType3D
    false,    // bTiledSurface
    false,    // bTileWalk
    false,    // bVerticalLineStride
    false,    // bVerticalLineStrideOffset
    false,    // bCompressionEnabled
    false,    // bCompressionMode
    0,        // MmcState
    false,    // bInterleaveChroma
    false,    // bHalfPitchChroma
    false,    // bSeperateUVPlane
    0,        // UVPixelOffsetUDirection
    0,        // UVPixelOffsetVDirection
    0,        // RotationMode
    0,        // bSurfaceArraySpacing
    false,    // bBoardColorOGL
    0,        // iXOffset
    0,        // iYOffset
    0,        // dwXOffsetForU
    0,        // dwYOffsetForU
    0,        // dwXOffsetForV
    0,        // dwYOffsetForV
    0,        // Compression Format
    0,        // L1CacheConfig

    nullptr,     // [out] pdwCmd
    0         // [out] dwLocationInCmd
};

extern const MHW_ID_ENTRY_PARAMS g_cInit_MhwIdEntryParams =
{
    0,       // dwMediaIdOffset
    0,       // iMediaId
    0,       // dwKernelOffset
    0,       // dwSamplerOffset
    0,       // dwSamplerCount
    0,       // dwBindingTableOffset
    0,       // iCurbeOffset
    0,       // iCurbeLength
    false,   // bBarrierEnable
    0,       // bGlobalBarrierEnable
    0,       // dwNumberofThreadsInGPGPUGroup
    0,       // dwSharedLocalMemorySize
    0        // iCrsThdConDataRdLn
};

//!
//! \brief    Assigns space in a state heap to a kernel state
//! \details  Client facing function to assign as space in a state heap a kernel state;
//!           if no space is available, a clean up is attempted 
//! \param    PMHW_STATE_HEAP_INTERFACE pCommonStateHeapInterface
//!           [in] State heap interface
//! \param    MHW_STATE_HEAP_TYPE StateHeapType
//!           [in] The state heap type requested (ISH/DSH)
//! \param    PMHW_KERNEL_STATE pKernelState
//!           [in] Kernel state to which a state heap space will be assigned
//! \param    uint32_t dwSpaceRequested
//!           [in] The amount of space requested from the state heap
//! \param    bool bStatic
//!           [in] Whether or not the space requested is static
//! \param    bool bZeroAssignedMem
//!           [in] Whether or not acquired memory should be zeroed
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, MOS_STATUS_CLIENT_AR_NO_SPACE if no space
//!           is available but it is possible for the client to wait, else fail reason
//!
MOS_STATUS Mhw_StateHeapInterface_AssignSpaceInStateHeap(
    PMHW_STATE_HEAP_INTERFACE   pCommonStateHeapInterface,
    MHW_STATE_HEAP_TYPE         StateHeapType,
    PMHW_KERNEL_STATE           pKernelState,
    uint32_t                    dwSpaceRequested,
    bool                        bStatic,
    bool                        bZeroAssignedMem)
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_CHK_NULL(pCommonStateHeapInterface);

    MHW_CHK_NULL(pCommonStateHeapInterface->pStateHeapInterface);

    MHW_CHK_STATUS(pCommonStateHeapInterface->pStateHeapInterface->AssignSpaceInStateHeap(
                  StateHeapType,
                  pKernelState,
                  dwSpaceRequested,
                  bStatic,
                  bZeroAssignedMem));

finish:
    return eStatus;
}

//!
//! \brief    Assigns space in a state heap to a kernel state
//! \details  Client facing function to assign as space in a state heap a kernel state;
//!           if no space is available, a clean up is attempted 
//! \param    [in] pCommonStateHeapInterface
//!           State heap interface
//! \param    [in] pKernelState
//!           Kernel state containing all memory blocks to submit
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success
//!
MOS_STATUS Mhw_StateHeapInterface_SubmitBlocks(
    PMHW_STATE_HEAP_INTERFACE   pCommonStateHeapInterface,
    PMHW_KERNEL_STATE           pKernelState)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_CHK_NULL(pCommonStateHeapInterface);

    MHW_CHK_NULL(pCommonStateHeapInterface->pStateHeapInterface);

    MHW_CHK_STATUS(pCommonStateHeapInterface->pStateHeapInterface->SubmitBlocks(pKernelState));

finish:
    return eStatus;
}

//!
//! \brief    Locks requested state heap
//! \details  Client facing function to lock a state heap
//! \param    PMHW_STATE_HEAP_INTERFACE pStateHeapInterface
//!           [in] State heap interface
//! \param    PMHW_STATE_HEAP pStateHeap
//!           [in] The state heap to be locked
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success
//!
MOS_STATUS Mhw_StateHeapInterface_LockStateHeap(
    PMHW_STATE_HEAP_INTERFACE   pCommonStateHeapInterface,
    PMHW_STATE_HEAP                    pStateHeap)
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_CHK_NULL(pCommonStateHeapInterface);

    MHW_CHK_NULL(pCommonStateHeapInterface->pStateHeapInterface);

    MHW_CHK_STATUS(pCommonStateHeapInterface->pStateHeapInterface->LockStateHeap(
                   pStateHeap));

finish:
    return eStatus;
}

//!
//! \brief    Unlocks requested state heap
//! \details  Client facing function to unlock a state heap
//! \param    PMHW_STATE_HEAP_INTERFACE pStateHeapInterface
//!           [in] State heap interface
//! \param    PMHW_STATE_HEAP pStateHeap
//!           [in] The state heap to be locked
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success
//!
MOS_STATUS Mhw_StateHeapInterface_UnlockStateHeap(
    PMHW_STATE_HEAP_INTERFACE   pCommonStateHeapInterface,
    PMHW_STATE_HEAP             pStateHeap)
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_CHK_NULL(pCommonStateHeapInterface);

    MHW_CHK_NULL(pCommonStateHeapInterface->pStateHeapInterface);

    MHW_CHK_STATUS(pCommonStateHeapInterface->pStateHeapInterface->UnLockStateHeap(
                   pStateHeap));

finish:
    return eStatus;
}

//!
//! \brief    Allocates a state heap of the requested type
//! \details  Client facing function to extend a state heap of the requested time, which
//!           involves allocating state heap and added it to the state heap liked list.
//! \param    PMHW_STATE_HEAP_INTERFACE pStateHeapInterface
//!           [in] State heap interface
//! \param    MHW_STATE_HEAP_TYPE StateHeapType
//!           [in] The state heap type requested (ISH/DSH)
//! \param    uint32_t dwSizeRequested
//!           [in] The size of the state heap
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS Mhw_StateHeapInterface_ExtendStateHeap(
    PMHW_STATE_HEAP_INTERFACE   pCommonStateHeapInterface,
    MHW_STATE_HEAP_TYPE         StateHeapType,
    uint32_t                    dwSizeRequested)
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_CHK_NULL(pCommonStateHeapInterface);

    MHW_CHK_NULL(pCommonStateHeapInterface->pStateHeapInterface);

    MHW_CHK_STATUS(pCommonStateHeapInterface->pStateHeapInterface->ExtendStateHeap(
                   StateHeapType, dwSizeRequested));

finish:
    return eStatus;
}

//!
//! \brief    Update CmdBufIdGlobal
//! \details  Client facing function to update CmdBufIdGlobal
//! \param    PMHW_STATE_HEAP_INTERFACE pStateHeapInterface
//!           [in] State heap interface
//! \param    PMOS_COMMAND_BUFFER pCmdBuffer
//!           [in] The command buffer containing the kernel workload
//! \param    void  *pvRenderEngineInterface
//!           [in] Render engine interface
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS Mhw_StateHeapInterface_UpdateGlobalCmdBufId(
    PMHW_STATE_HEAP_INTERFACE   pCommonStateHeapInterface)
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_CHK_NULL(pCommonStateHeapInterface);

    MHW_CHK_NULL(pCommonStateHeapInterface->pStateHeapInterface);

    MHW_CHK_STATUS(pCommonStateHeapInterface->pStateHeapInterface->UpdateGlobalCmdBufId());

finish:
    return eStatus;
}

//!
//! \brief    Set command buffer status pointer
//! \details  Client facing function to set command buffer status pointer
//! \param    PMHW_STATE_HEAP_INTERFACE pStateHeapInterface
//!           [in] State heap interface
//! \param    void  *pvCmdBufStatus
//!           [in] command buffer status pointer
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS Mhw_StateHeapInterface_SetCmdBufStatusPtr(
    PMHW_STATE_HEAP_INTERFACE   pCommonStateHeapInterface,
    void                        *pvCmdBufStatus)
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_CHK_NULL(pCommonStateHeapInterface);

    MHW_CHK_NULL(pCommonStateHeapInterface->pStateHeapInterface);

    MHW_CHK_STATUS(pCommonStateHeapInterface->pStateHeapInterface->SetCmdBufStatusPtr(
        pvCmdBufStatus));

finish:
    return eStatus;
}

//!
//! \brief    Calculate the space needed in the SSH
//! \details  Client facing function to calculate the space needed in the SSH
//!           given the number of binding table entries
//! \param    PMHW_STATE_HEAP_INTERFACE pStateHeapInterface
//!           [in] State heap interface
//! \param    uint32_t dwBtEntriesRequested
//!           [in] Binding table entries requested in the SSH
//! \param    uint32_t *pdwSshSize
//!           [out] The size needed in the SSH
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS Mhw_StateHeapInterface_CalculateSshAndBtSizesRequested(
    PMHW_STATE_HEAP_INTERFACE   pCommonStateHeapInterface,
    uint32_t                    dwBtEntriesRequested,
    uint32_t                    *pdwSshSize,
    uint32_t                    *pdwBtSize)
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_CHK_NULL(pCommonStateHeapInterface);

    MHW_CHK_NULL(pCommonStateHeapInterface->pStateHeapInterface);

    MHW_CHK_STATUS(pCommonStateHeapInterface->pStateHeapInterface->CalculateSshAndBtSizesRequested(
                   dwBtEntriesRequested, pdwSshSize, pdwBtSize));

finish:
    return eStatus;
}

//!
//! \brief    Request SSH space for a command buffer.
//! \details  Client facing function to request SSH space for a command buffer, if not enough
//!           space is available, more will be requested.
//! \param    PMHW_STATE_HEAP_INTERFACE pStateHeapInterface
//!           [in] State heap interface
//! \param    uint32_t dwBtEntriesRequested
//!           [in] Binding table entries requested in the SSH
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS Mhw_StateHeapInterface_RequestSshSpaceForCmdBuf(
    PMHW_STATE_HEAP_INTERFACE   pCommonStateHeapInterface,
    uint32_t                    dwBtEntriesRequested)
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_CHK_NULL(pCommonStateHeapInterface);

    MHW_CHK_NULL(pCommonStateHeapInterface->pStateHeapInterface);

    MHW_CHK_STATUS(pCommonStateHeapInterface->pStateHeapInterface->RequestSshSpaceForCmdBuf(
                   dwBtEntriesRequested));

finish:
    return eStatus;
}

MOS_STATUS Mhw_StateHeapInterface_SetInterfaceDescriptor (
        PMHW_STATE_HEAP_INTERFACE    pCommonStateHeapInterface,
        uint32_t                            dwNumIdsToSet,
        PMHW_INTERFACE_DESCRIPTOR_PARAMS    pParams)
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_CHK_NULL(pCommonStateHeapInterface);

    MHW_CHK_NULL(pCommonStateHeapInterface->pStateHeapInterface);

    MHW_CHK_STATUS(pCommonStateHeapInterface->pStateHeapInterface->SetInterfaceDescriptor(
                   dwNumIdsToSet, pParams));

finish:
    return eStatus;
}

MOS_STATUS Mhw_StateHeapInterface_SetInterfaceDescriptorEntry (
    PMHW_STATE_HEAP_INTERFACE    pCommonStateHeapInterface,
    PMHW_ID_ENTRY_PARAMS                pParams)
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_CHK_NULL(pCommonStateHeapInterface);

    MHW_CHK_NULL(pCommonStateHeapInterface->pStateHeapInterface);

    MHW_CHK_STATUS(pCommonStateHeapInterface->pStateHeapInterface->SetInterfaceDescriptorEntry(
                   pParams));

finish:
    return eStatus;
}

MOS_STATUS Mhw_StateHeapInterface_SetBindingTable (
    PMHW_STATE_HEAP_INTERFACE   pCommonStateHeapInterface,
    PMHW_KERNEL_STATE                  pKernelState)
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_CHK_NULL(pCommonStateHeapInterface);

    MHW_CHK_NULL(pCommonStateHeapInterface->pStateHeapInterface);

    MHW_CHK_STATUS(pCommonStateHeapInterface->pStateHeapInterface->SetBindingTable(
                   pKernelState));

finish:
    return eStatus;
}

MOS_STATUS Mhw_StateHeapInterface_SetBindingTableEntry (
    PMHW_STATE_HEAP_INTERFACE   pCommonStateHeapInterface,
    PMHW_BINDING_TABLE_PARAMS          pParams)
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_CHK_NULL(pCommonStateHeapInterface);

    MHW_CHK_NULL(pCommonStateHeapInterface->pStateHeapInterface);

    MHW_CHK_STATUS(pCommonStateHeapInterface->pStateHeapInterface->SetBindingTableEntry(
                   pParams));

finish:
    return eStatus;
}

MOS_STATUS Mhw_StateHeapInterface_SendBindingTableEntry (
    PMHW_STATE_HEAP_INTERFACE   pCommonStateHeapInterface,
    PMHW_BINDING_TABLE_SEND_PARAMS     pParams)
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_CHK_NULL(pCommonStateHeapInterface);

    MHW_CHK_NULL(pCommonStateHeapInterface->pStateHeapInterface);

    MHW_CHK_STATUS(pCommonStateHeapInterface->pStateHeapInterface->SendBindingTableEntry (
                   pParams));

finish:
    return eStatus;
}

MOS_STATUS Mhw_StateHeapInterface_SetSurfaceState(
    PMHW_STATE_HEAP_INTERFACE   pCommonStateHeapInterface,
    PMHW_KERNEL_STATE           pKernelState,
    PMOS_COMMAND_BUFFER         pCmdBuffer,
    uint32_t                    dwNumSurfaceStatesToSet,
    PMHW_RCS_SURFACE_PARAMS     pParams)
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_CHK_NULL(pCommonStateHeapInterface);

    MHW_CHK_NULL(pCommonStateHeapInterface->pStateHeapInterface);

    MHW_CHK_STATUS(pCommonStateHeapInterface->pStateHeapInterface->SetSurfaceState (
                   pKernelState,pCmdBuffer, dwNumSurfaceStatesToSet, pParams));

finish:
    return eStatus;
}

MOS_STATUS Mhw_StateHeapInterface_SetSurfaceStateEntry(
    PMHW_STATE_HEAP_INTERFACE    pCommonStateHeapInterface,
    PMHW_SURFACE_STATE_PARAMS           pParams)
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_CHK_NULL(pCommonStateHeapInterface);

    MHW_CHK_NULL(pCommonStateHeapInterface->pStateHeapInterface);

    MHW_CHK_STATUS(pCommonStateHeapInterface->pStateHeapInterface->SetSurfaceStateEntry(
                   pParams));

finish:
    return eStatus;
}

MOS_STATUS Mhw_StateHeapInterface_InitSamplerStates(
    PMHW_STATE_HEAP_INTERFACE   pCommonStateHeapInterface,
    void                        *pSampler,
    int32_t                     iSamplers)
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_CHK_NULL(pCommonStateHeapInterface);

    MHW_CHK_NULL(pCommonStateHeapInterface->pStateHeapInterface);

    MHW_CHK_STATUS(pCommonStateHeapInterface->pStateHeapInterface->InitSamplerStates(
                   pSampler,iSamplers));

finish:
    return eStatus;
}

MOS_STATUS Mhw_StateHeapInterface_SetSamplerState(
    PMHW_STATE_HEAP_INTERFACE   pCommonStateHeapInterface,
    void                               *pSampler,
    PMHW_SAMPLER_STATE_PARAM           pParams)
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_CHK_NULL(pCommonStateHeapInterface);

    MHW_CHK_NULL(pCommonStateHeapInterface->pStateHeapInterface);

    MHW_CHK_STATUS(pCommonStateHeapInterface->pStateHeapInterface->SetSamplerState(
                   pSampler,pParams));

finish:
    return eStatus;
}

uint32_t Mhw_StateHeapInterface_DSH_CalculateSpaceNeeded(
    PMHW_STATE_HEAP_INTERFACE            pStateHeapInterface,
    MHW_STATE_HEAP_TYPE                  StateHeapType,
    PMHW_STATE_HEAP_DYNAMIC_ALLOC_PARAMS pParams)
{
    if (pStateHeapInterface == nullptr)
    {
        MHW_ASSERTMESSAGE("Invalid Input Parameter");
        return 0;
    }
    else
    {
        if (pStateHeapInterface->pStateHeapInterface == nullptr)
        {
            MHW_ASSERTMESSAGE("Invalid Input Parameter");
            return 0;
        }
    }

    return  pStateHeapInterface->pStateHeapInterface->CalculateSpaceNeededDyn(
            StateHeapType,  pParams);
}

PMHW_STATE_HEAP_MEMORY_BLOCK Mhw_StateHeapInterface_DSH_AllocateDynamicBlock(
        PMHW_STATE_HEAP_INTERFACE            pStateHeapInterface,
        MHW_STATE_HEAP_TYPE                  StateHeapType,
        PMHW_STATE_HEAP_DYNAMIC_ALLOC_PARAMS pParams)
{
    if (pStateHeapInterface == nullptr)
    {
        MHW_ASSERTMESSAGE("Invalid Input Parameter");
        return (PMHW_STATE_HEAP_MEMORY_BLOCK)0;
    }
    else
    {
        if (pStateHeapInterface->pStateHeapInterface == nullptr)
        {
            MHW_ASSERTMESSAGE("Invalid Input Parameter");
            return (PMHW_STATE_HEAP_MEMORY_BLOCK)0;
        }
    }

    return pStateHeapInterface->pStateHeapInterface->AllocateDynamicBlockDyn(
        StateHeapType,  pParams);
}

MOS_STATUS Mhw_StateHeapInterface_DSH_SubmitDynamicBlock(
        PMHW_STATE_HEAP_INTERFACE            pStateHeapInterface,
        MHW_STATE_HEAP_TYPE                  StateHeapType,
        PMHW_STATE_HEAP_MEMORY_BLOCK         pBlock,
        FrameTrackerTokenFlat                *trackerToken)
{
    MHW_CHK_NULL_RETURN(pStateHeapInterface);
    MHW_CHK_NULL_RETURN(pStateHeapInterface->pStateHeapInterface);

    return pStateHeapInterface->pStateHeapInterface->SubmitDynamicBlockDyn(
        StateHeapType,  pBlock,  trackerToken);
}

MOS_STATUS Mhw_StateHeapInterface_DSH_FreeDynamicBlock(
        PMHW_STATE_HEAP_INTERFACE            pStateHeapInterface,
        MHW_STATE_HEAP_TYPE                  StateHeapType,
        PMHW_STATE_HEAP_MEMORY_BLOCK         pBlock)
{
    MHW_CHK_NULL_RETURN(pStateHeapInterface);
    MHW_CHK_NULL_RETURN(pStateHeapInterface->pStateHeapInterface);

    return pStateHeapInterface->pStateHeapInterface->FreeDynamicBlockDyn(
        StateHeapType,  pBlock);
}

MOS_STATUS Mhw_StateHeapInterface_DSH_RefreshDynamicHeap (
    PMHW_STATE_HEAP_INTERFACE   pStateHeapInterface,
    MHW_STATE_HEAP_TYPE         StateHeapType)
{
    MHW_CHK_NULL_RETURN(pStateHeapInterface);
    MHW_CHK_NULL_RETURN(pStateHeapInterface->pStateHeapInterface);

    return pStateHeapInterface->pStateHeapInterface->RefreshDynamicHeapDyn(
         StateHeapType);
}

MOS_STATUS Mhw_StateHeapInterface_DSH_ReleaseStateHeap(
    PMHW_STATE_HEAP_INTERFACE pStateHeapInterface,
    PMHW_STATE_HEAP pStateHeap)
{
    MHW_CHK_NULL_RETURN(pStateHeapInterface);
    MHW_CHK_NULL_RETURN(pStateHeapInterface->pStateHeapInterface);

    return pStateHeapInterface->pStateHeapInterface->ReleaseStateHeapDyn(pStateHeap);
}

//!
//! \brief    Allocates the state heap interface internal parameters
//! \details  Internal MHW function to allocate all parameters needed for the
//!           state heap interface
//! \param    PMHW_STATE_HEAP_INTERFACE pStateHeapInterface
//!           [in] State heap interface
//! \param    MHW_STATE_HEAP_SETTINGS StateHeapSettings
//!           [in] Setting used to initialize the state heap interface
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS Mhw_StateHeapInterface_Create(
    PMHW_STATE_HEAP_INTERFACE   *ppStateHeapInterface,
    MHW_STATE_HEAP_SETTINGS     StateHeapSettings)
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;
    MHW_CHK_NULL(ppStateHeapInterface);
    MHW_CHK_NULL(*ppStateHeapInterface);

    MHW_CHK_NULL((*ppStateHeapInterface)->pStateHeapInterface);

    MHW_CHK_STATUS((*ppStateHeapInterface)->pStateHeapInterface->InitializeInterface(
        StateHeapSettings));

finish:
    return eStatus;
}
//!
//! \brief    Destroys the state heap interface
//! \details  Internal MHW function to destroy the state heap interface
//! \param    PMHW_STATE_HEAP_INTERFACE pStateHeapInterface
//!           [in] State heap interface
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS Mhw_StateHeapInterface_Destroy(
    PMHW_STATE_HEAP_INTERFACE   pCommonStateHeapInterface)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    if (pCommonStateHeapInterface)
    {
        if (pCommonStateHeapInterface->pStateHeapInterface)
        {
            MOS_Delete(pCommonStateHeapInterface->pStateHeapInterface);
        }
        MOS_FreeMemory(pCommonStateHeapInterface);
    }

    return eStatus;
}

//!
//! \brief    Initializes the state heap interface
//! \details  Internal MHW function to initialize all function pointers and some parameters
//! \param    PMHW_STATE_HEAP_INTERFACE* ppStateHeapInterface
//!           [in/out] Poitner to state heap interface pointer to be allocated
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] OS interface
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS Mhw_StateHeapInterface_InitInterface(
    PMHW_STATE_HEAP_INTERFACE   *ppCommonStateHeapInterface,
    PMOS_INTERFACE              pOsInterface,
    uint8_t                     bDynamicMode)
{
    PMHW_STATE_HEAP_INTERFACE   pCommonStateHeapInterface = nullptr;
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;
    MhwInterfaces::CreateParams params;
    MhwInterfaces               *mhwInterfaces = nullptr;

    MHW_FUNCTION_ENTER;

    MHW_CHK_NULL(ppCommonStateHeapInterface);
    MHW_CHK_NULL(pOsInterface);

    pCommonStateHeapInterface =
        (PMHW_STATE_HEAP_INTERFACE)MOS_AllocAndZeroMemory(sizeof(MHW_STATE_HEAP_INTERFACE));
    MHW_CHK_NULL(pCommonStateHeapInterface);

    //Common Interfaces
    pCommonStateHeapInterface->pfnCreate                          = Mhw_StateHeapInterface_Create;
    pCommonStateHeapInterface->pfnDestroy                         = Mhw_StateHeapInterface_Destroy;
    pCommonStateHeapInterface->pfnLockStateHeap                   = Mhw_StateHeapInterface_LockStateHeap;
    pCommonStateHeapInterface->pfnUnlockStateHeap                 = Mhw_StateHeapInterface_UnlockStateHeap;
    pCommonStateHeapInterface->pfnAssignSpaceInStateHeap          = Mhw_StateHeapInterface_AssignSpaceInStateHeap;
    pCommonStateHeapInterface->pfnSubmitBlocks                    = Mhw_StateHeapInterface_SubmitBlocks;
    pCommonStateHeapInterface->pfnExtendStateHeap                 = Mhw_StateHeapInterface_ExtendStateHeap;
    pCommonStateHeapInterface->pfnUpdateGlobalCmdBufId            = Mhw_StateHeapInterface_UpdateGlobalCmdBufId;
    pCommonStateHeapInterface->pfnSetCmdBufStatusPtr              = Mhw_StateHeapInterface_SetCmdBufStatusPtr;
    pCommonStateHeapInterface->pfnRequestSshSpaceForCmdBuf        = Mhw_StateHeapInterface_RequestSshSpaceForCmdBuf;
    pCommonStateHeapInterface->pfnCalculateSshAndBtSizesRequested = Mhw_StateHeapInterface_CalculateSshAndBtSizesRequested;

    //Interfaces in dynamic mode
    pCommonStateHeapInterface->pfnCalculateDynamicSpaceNeeded     = Mhw_StateHeapInterface_DSH_CalculateSpaceNeeded;
    pCommonStateHeapInterface->pfnAllocateDynamicBlock            = Mhw_StateHeapInterface_DSH_AllocateDynamicBlock;
    pCommonStateHeapInterface->pfnSubmitDynamicBlock              = Mhw_StateHeapInterface_DSH_SubmitDynamicBlock;
    pCommonStateHeapInterface->pfnFreeDynamicBlock                = Mhw_StateHeapInterface_DSH_FreeDynamicBlock;
    pCommonStateHeapInterface->pfnRefreshDynamicHeap              = Mhw_StateHeapInterface_DSH_RefreshDynamicHeap;
    pCommonStateHeapInterface->pfnReleaseStateHeap                = Mhw_StateHeapInterface_DSH_ReleaseStateHeap;

    //Generic Interfaces
    pCommonStateHeapInterface->pfnSetInterfaceDescriptor      = Mhw_StateHeapInterface_SetInterfaceDescriptor;
    pCommonStateHeapInterface->pfnSetInterfaceDescriptorEntry = Mhw_StateHeapInterface_SetInterfaceDescriptorEntry;
    pCommonStateHeapInterface->pfnSetBindingTable             = Mhw_StateHeapInterface_SetBindingTable;
    pCommonStateHeapInterface->pfnSetSurfaceState             = Mhw_StateHeapInterface_SetSurfaceState;
    pCommonStateHeapInterface->pfnSetBindingTableEntry        = Mhw_StateHeapInterface_SetBindingTableEntry;
    pCommonStateHeapInterface->pfnSendBindingTableEntry       = Mhw_StateHeapInterface_SendBindingTableEntry;
    pCommonStateHeapInterface->pfnSetSurfaceStateEntry        = Mhw_StateHeapInterface_SetSurfaceStateEntry;
    pCommonStateHeapInterface->pfnInitSamplerStates           = Mhw_StateHeapInterface_InitSamplerStates;
    pCommonStateHeapInterface->pfnSetSamplerState             = Mhw_StateHeapInterface_SetSamplerState;

    MOS_ZeroMemory(&params, sizeof(params));
    params.Flags.m_stateHeap = true;
    params.m_heapMode = bDynamicMode;
    mhwInterfaces = MhwInterfaces::CreateFactory(params, pOsInterface);
    if (mhwInterfaces)
    {
        MHW_CHK_NULL(mhwInterfaces->m_stateHeapInterface);
        pCommonStateHeapInterface->pStateHeapInterface = mhwInterfaces->m_stateHeapInterface;
        // MhwInterfaces always create CP and MI interfaces, so we have to delete those we don't need.
        MOS_Delete(mhwInterfaces->m_miInterface);
        Delete_MhwCpInterface(mhwInterfaces->m_cpInterface);
        mhwInterfaces->m_cpInterface = NULL;
        MOS_Delete(mhwInterfaces);
    }
    else
    {
        MHW_ASSERTMESSAGE("Allocate MhwInterfaces failed");
        eStatus = MOS_STATUS_NO_SPACE;
        goto finish;
    }

    *ppCommonStateHeapInterface = pCommonStateHeapInterface;

finish:
    if (eStatus != MOS_STATUS_SUCCESS && pCommonStateHeapInterface)
    {
        pCommonStateHeapInterface->pfnDestroy(pCommonStateHeapInterface);
        *ppCommonStateHeapInterface = nullptr;
    }
    return eStatus;
}

XMHW_STATE_HEAP_INTERFACE::XMHW_STATE_HEAP_INTERFACE(
    PMOS_INTERFACE pInputOSInterface,
    int8_t         bDynamicMode):
    m_pWaTable(nullptr),
    m_pdwCmdBufIdGlobal(nullptr),
    m_dwCurrCmdBufId(0),
    m_pSyncTags(nullptr),
    m_dwCurrSyncTag(0),
    m_dwInvalidSyncTagId(0),
    m_bRegisteredBBCompleteNotifyEvent(false),
    m_pInstructionStateHeaps(nullptr),
    m_dwNumIsh(0),
    m_dwNumDsh(0),
    m_pDynamicStateHeaps(nullptr),
    m_bDynamicMode(bDynamicMode),
    m_pIshBlockManager(nullptr),
    m_pDshBlockManager(nullptr),
    m_pOsInterface(pInputOSInterface),
    m_wIdAlignment(0),
    m_wBtIdxAlignment(0),
    m_wCurbeAlignment(0),
    m_wSizeOfCmdSamplerState(0),
    m_dwMaxSurfaceStateSize(0),
    m_pfnAddResourceToCmd(nullptr),
    m_wSizeOfCmdInterfaceDescriptorData(0)
{
    MHW_FUNCTION_ENTER;

    MOS_ZeroMemory(&m_resCmdBufIdGlobal, sizeof(m_resCmdBufIdGlobal));
    MOS_ZeroMemory(&m_SurfaceStateHeap, sizeof(m_SurfaceStateHeap));
    MOS_ZeroMemory(&m_HwSizes, sizeof(m_HwSizes));
    MOS_ZeroMemory(&m_StateHeapSettings, sizeof(m_StateHeapSettings));
}

XMHW_STATE_HEAP_INTERFACE::~XMHW_STATE_HEAP_INTERFACE()
{
    MHW_FUNCTION_ENTER;

    if (m_bDynamicMode == MHW_DGSH_MODE)
    {
        // heap manager destructors called automatically
        return;
    }

    PMHW_STATE_HEAP              pStateHeapPtr, pStateHeapNext;
    PMHW_STATE_HEAP_MEMORY_BLOCK pMemBlkPtr, pMemBlkNext;

    //Release m_SyncTags
    MOS_FreeMemory(m_pSyncTags);

    // Destroy all memory block objects and block manager objects for ISH, DSH
    if(m_bDynamicMode == MHW_DSH_MODE)
    {
        MOS_Delete(m_pIshBlockManager);
        MOS_Delete(m_pDshBlockManager);
    }

    //Release m_resCmdBufIdGlobal
    if (m_pOsInterface != nullptr)
    {
        m_pOsInterface->pfnUnlockResource(m_pOsInterface, &m_resCmdBufIdGlobal);
        m_pOsInterface->pfnFreeResource(m_pOsInterface, &m_resCmdBufIdGlobal);
    }

    //Free ISH
    pStateHeapPtr = m_pInstructionStateHeaps;
    for (uint32_t i = 0; i < m_dwNumIsh; i++)
    {
        pStateHeapNext = pStateHeapPtr->pNext;
        if (m_pOsInterface != nullptr)
        {
            if (pStateHeapPtr->bKeepLocked)
            {
                pStateHeapPtr->bKeepLocked = false;
                UnLockStateHeap(pStateHeapPtr);
            }
            m_pOsInterface->pfnFreeResource(m_pOsInterface, &pStateHeapPtr->resHeap);
        }

        if(m_bDynamicMode == MHW_RENDER_HAL_MODE)
        {
            pMemBlkPtr = pStateHeapPtr->pMemoryHead;
            while (pMemBlkPtr)
            {
                pMemBlkNext = pMemBlkPtr->pNext;
                MOS_FreeMemory(pMemBlkPtr);
                pMemBlkPtr = pMemBlkNext;
            }
        }
        MOS_FreeMemory(pStateHeapPtr);
        pStateHeapPtr = pStateHeapNext;
    }

    // Free DSH
    pStateHeapPtr = m_pDynamicStateHeaps;
    for (uint32_t i = 0; i < m_dwNumDsh; i++)
    {
        if (pStateHeapPtr == nullptr)
        {
             break;
        }

        pStateHeapNext = pStateHeapPtr->pNext;
        if (m_pOsInterface != nullptr)
        {
            if (pStateHeapPtr->bKeepLocked)
            {
                pStateHeapPtr->bKeepLocked = false;
                UnLockStateHeap(pStateHeapPtr);
            }
            m_pOsInterface->pfnFreeResource(m_pOsInterface, &pStateHeapPtr->resHeap);
        }

        if(m_bDynamicMode == MHW_RENDER_HAL_MODE)
        {
            pMemBlkPtr = pStateHeapPtr->pMemoryHead;
            while (pMemBlkPtr)
            {
                pMemBlkNext = pMemBlkPtr->pNext;
                MOS_FreeMemory(pMemBlkPtr);
                pMemBlkPtr = pMemBlkNext;
            }
        }

        MOS_FreeMemory(pStateHeapPtr);
        pStateHeapPtr = pStateHeapNext;
    }

    return ;
}

MOS_STATUS XMHW_STATE_HEAP_INTERFACE::InitializeInterface(
    MHW_STATE_HEAP_SETTINGS StateHeapSettings)
{
    MOS_ALLOC_GFXRES_PARAMS     AllocParams;
    MOS_LOCK_PARAMS             LockParams;
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    //state heap settings
    m_StateHeapSettings = StateHeapSettings;

    MHW_CHK_NULL(m_pOsInterface);

    m_pWaTable  = m_pOsInterface->pfnGetWaTable(m_pOsInterface);

    if (m_pOsInterface->bUsesGfxAddress)
    {
        m_pfnAddResourceToCmd = Mhw_AddResourceToCmd_GfxAddress;
    }
    else if (m_pOsInterface->bUsesPatchList)
    {
        m_pfnAddResourceToCmd = Mhw_AddResourceToCmd_PatchList;
    }
    else
    {
        // No addressing method selected
        eStatus = MOS_STATUS_UNKNOWN;
        goto finish;
    }

    if (m_bDynamicMode == MHW_DGSH_MODE)
    {
        m_ishManager.RegisterOsInterface(m_pOsInterface);
        m_ishManager.SetDefaultBehavior(StateHeapSettings.m_ishBehavior);
        m_ishManager.SetInitialHeapSize(StateHeapSettings.dwIshSize);
        if (StateHeapSettings.m_ishBehavior == HeapManager::Behavior::extend ||
            StateHeapSettings.m_ishBehavior == HeapManager::Behavior::destructiveExtend ||
            StateHeapSettings.m_ishBehavior == HeapManager::Behavior::waitAndExtend)
        {
            m_ishManager.SetExtendHeapSize(StateHeapSettings.dwIshIncrement);
        }
        if (StateHeapSettings.m_keepIshLocked)
        {
            MHW_MI_CHK_STATUS(m_ishManager.LockHeapsOnAllocate());
        }

        m_dshManager.RegisterOsInterface(m_pOsInterface);
        m_dshManager.SetDefaultBehavior(StateHeapSettings.m_dshBehavior);
        m_dshManager.SetInitialHeapSize(StateHeapSettings.dwDshSize);
        if (StateHeapSettings.m_dshBehavior == HeapManager::Behavior::extend ||
            StateHeapSettings.m_dshBehavior == HeapManager::Behavior::destructiveExtend ||
            StateHeapSettings.m_dshBehavior == HeapManager::Behavior::waitAndExtend)
        {
            m_dshManager.SetExtendHeapSize(StateHeapSettings.dwDshIncrement);
        }
        if (StateHeapSettings.m_keepDshLocked)
        {
            MHW_MI_CHK_STATUS(m_dshManager.LockHeapsOnAllocate());
        }

        return MOS_STATUS_SUCCESS;
    }

    //Sync tags and sync tag id
    m_pSyncTags = (PMHW_SYNC_TAG)MOS_AllocAndZeroMemory(sizeof(MHW_SYNC_TAG) *
                        StateHeapSettings.dwNumSyncTags);
    MHW_CHK_NULL(m_pSyncTags);

    if(m_bDynamicMode == MHW_DSH_MODE)
    {
        m_dwInvalidSyncTagId = 0;

        //Allocate block manager for ISH
        m_pIshBlockManager = MOS_New(MHW_BLOCK_MANAGER, nullptr);
        MHW_CHK_NULL(m_pIshBlockManager);


    }
    else // m_bDynamicMode == MHW_RENDER_HAL_MODE
    {
        m_dwInvalidSyncTagId = StateHeapSettings.dwNumSyncTags;

        //Extend state heap for DSH
        MHW_CHK_STATUS(ExtendStateHeap(
            MHW_DSH_TYPE,
            StateHeapSettings.dwDshSize));
        if (StateHeapSettings.m_keepDshLocked)
        {
            MHW_CHK_STATUS(LockStateHeap(m_pDynamicStateHeaps));
            m_pDynamicStateHeaps->bKeepLocked = true;
        }
    }

    //Allocate resCmdBufIdGlobal
    MOS_ZeroMemory(&AllocParams, sizeof(AllocParams));
    AllocParams.Type = MOS_GFXRES_BUFFER;
    AllocParams.TileType = MOS_TILE_LINEAR;
    AllocParams.Format = Format_Buffer;
    AllocParams.dwBytes = MHW_CACHELINE_SIZE;
    AllocParams.pBufName = "CmdBufIdGlobal";
    MHW_CHK_STATUS(m_pOsInterface->pfnAllocateResource(
        m_pOsInterface,
        &AllocParams,
        &m_resCmdBufIdGlobal));
    m_dwCurrCmdBufId = 1;

    MOS_ZeroMemory(&LockParams, sizeof(LockParams));
    LockParams.WriteOnly = 1;
    m_pdwCmdBufIdGlobal = (uint32_t*)m_pOsInterface->pfnLockResource(
        m_pOsInterface,
        &m_resCmdBufIdGlobal,
        &LockParams);
    MHW_CHK_NULL(m_pdwCmdBufIdGlobal);
    MOS_ZeroMemory(m_pdwCmdBufIdGlobal, AllocParams.dwBytes);
    m_dwCurrCmdBufId = 1;

    //Extend state heap for ISH
    MHW_CHK_STATUS(ExtendStateHeap(
        MHW_ISH_TYPE,
        StateHeapSettings.dwIshSize));
    if (StateHeapSettings.m_keepIshLocked)
    {
        MHW_CHK_NULL(m_pInstructionStateHeaps);
        MHW_CHK_STATUS(LockStateHeap(m_pInstructionStateHeaps));
        m_pInstructionStateHeaps->bKeepLocked = true;
    }

finish:
    return eStatus;
}

MOS_STATUS XMHW_STATE_HEAP_INTERFACE::InitMemoryBlock(
    PMHW_STATE_HEAP                 pStateHeap,
    PMHW_STATE_HEAP_MEMORY_BLOCK    *ppMemoryBlock,
    uint32_t                        dwRequestedSize,
    bool                            bStatic)
{
    PMHW_STATE_HEAP_MEMORY_BLOCK    pMemoryBlock;
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;

    MHW_CHK_NULL(ppMemoryBlock);

    if (dwRequestedSize > 0)
    {
        pMemoryBlock = (PMHW_STATE_HEAP_MEMORY_BLOCK)MOS_AllocAndZeroMemory(
                        sizeof(MHW_STATE_HEAP_MEMORY_BLOCK));
        MHW_CHK_NULL(pMemoryBlock);

        FrameTrackerTokenFlat_Invalidate(&pMemoryBlock->trackerToken);
        pMemoryBlock->dwBlockSize = dwRequestedSize;
        pMemoryBlock->pStateHeap = pStateHeap;
        pMemoryBlock->bStatic = bStatic;

        *ppMemoryBlock = pMemoryBlock;
    }

finish:
    return eStatus;
}

MOS_STATUS XMHW_STATE_HEAP_INTERFACE::InsertMemoryBlock(
        PMHW_STATE_HEAP_MEMORY_BLOCK    pMemoryBlockFree,
        PMHW_STATE_HEAP_MEMORY_BLOCK    pMemoryBlockToAdd)
{
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;

    MHW_CHK_NULL(pMemoryBlockFree);
    MHW_CHK_NULL(pMemoryBlockToAdd);

    if (pMemoryBlockFree->dwBlockSize < pMemoryBlockToAdd->dwBlockSize)
    {
        MHW_ASSERTMESSAGE("Free block does not have enough space to contain new block.");
        eStatus = MOS_STATUS_NO_SPACE;
        goto finish;
    }

    pMemoryBlockFree->dwBlockSize -= pMemoryBlockToAdd->dwBlockSize;
    pMemoryBlockToAdd->dwOffsetInStateHeap =
        pMemoryBlockFree->dwOffsetInStateHeap + pMemoryBlockFree->dwBlockSize;

    MHW_CHK_STATUS(InsertLinkedList(
        pMemoryBlockFree,
        pMemoryBlockToAdd));

finish:
    return eStatus;
}

MOS_STATUS XMHW_STATE_HEAP_INTERFACE::InsertLinkedList(
        PMHW_STATE_HEAP_MEMORY_BLOCK    pStartNode,
        PMHW_STATE_HEAP_MEMORY_BLOCK    pNodeToAdd)
{
    PMHW_STATE_HEAP_MEMORY_BLOCK    pStartNext;
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;

    MHW_CHK_NULL(pStartNode);
    MHW_CHK_NULL(pNodeToAdd);

    pStartNext = pStartNode->pNext;

    pStartNode->pNext = pNodeToAdd;
    pNodeToAdd->pPrev = pStartNode;
    pNodeToAdd->pNext = pStartNext;
    if (pStartNext)
    {
        pStartNext->pPrev = pNodeToAdd;
    }

finish:
    return eStatus;
}

MOS_STATUS XMHW_STATE_HEAP_INTERFACE::ReturnSpaceMemoryBlock(
    PMHW_STATE_HEAP_MEMORY_BLOCK    pMemoryBlock)
{
    PMHW_STATE_HEAP_MEMORY_BLOCK    pPrevBlock, pNextBlock;
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;

    MHW_CHK_NULL(pMemoryBlock);

    if (pMemoryBlock->bStatic)
    {
        MHW_VERBOSEMESSAGE("Static blocks do not need to be cleaned up.");
        goto finish;
    }

    pPrevBlock = pMemoryBlock->pPrev;
    pNextBlock = pMemoryBlock->pNext;

    FrameTrackerTokenFlat_Invalidate(&pMemoryBlock->trackerToken);

    if (pPrevBlock && !pPrevBlock->bStatic)
    {
        if (!FrameTrackerTokenFlat_IsValid(&pPrevBlock->trackerToken))
        {
            pPrevBlock->dwBlockSize += pMemoryBlock->dwBlockSize;
            pPrevBlock->pNext = pNextBlock;
            if (pNextBlock)
            {
                pNextBlock->pPrev = pPrevBlock;
            }
            MOS_FreeMemory(pMemoryBlock);
            pMemoryBlock = pPrevBlock;
        }
    }

    if (pNextBlock && !pNextBlock->bStatic)
    {
        if (!FrameTrackerTokenFlat_IsValid(&pNextBlock->trackerToken))
        {
            pMemoryBlock->dwBlockSize += pNextBlock->dwBlockSize;
            pMemoryBlock->pNext = pNextBlock->pNext;
            if (pNextBlock->pNext)
            {
                pNextBlock->pNext->pPrev = pMemoryBlock;
            }
            MOS_FreeMemory(pNextBlock);
        }
    }

finish:
    return eStatus;
}

MOS_STATUS XMHW_STATE_HEAP_INTERFACE::AssignSpaceInStateHeap(
    MHW_STATE_HEAP_TYPE         StateHeapType,
    PMHW_KERNEL_STATE           pKernelState,
    uint32_t                    dwSpaceRequested,
    bool                        bStatic,
    bool                        bZeroAssignedMem)
{
    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(pKernelState);

    HeapManager *heapManager = nullptr;
    MemoryBlock *requestedBlock = nullptr;
    if (StateHeapType == MHW_ISH_TYPE)
    {
        heapManager = &m_ishManager;
        requestedBlock = &pKernelState->m_ishRegion;
    }
    else if (StateHeapType == MHW_DSH_TYPE)
    {
        heapManager = &m_dshManager;
        requestedBlock = &pKernelState->m_dshRegion;
    }
    else if (StateHeapType == MHW_SSH_TYPE)
    {
        pKernelState->dwSshOffset = m_SurfaceStateHeap.dwCurrOffset;
        m_SurfaceStateHeap.dwCurrOffset += pKernelState->dwSshSize;
        if (m_SurfaceStateHeap.dwCurrOffset > m_SurfaceStateHeap.dwSize)
        {
            MHW_ASSERTMESSAGE("Insufficient space requested for SSH");
            return MOS_STATUS_NO_SPACE;
        }
        return MOS_STATUS_SUCCESS;
    }
    else
    {
        MHW_ASSERTMESSAGE("Unsupported state heap type.");
        return MOS_STATUS_INVALID_PARAMETER;
    }
    MHW_MI_CHK_NULL(heapManager);

    uint32_t spaceNeeded = 0;
    MemoryBlockManager::AcquireParams acquireParams =
        MemoryBlockManager::AcquireParams(pKernelState->m_currTrackerId, m_blockSizes);
    acquireParams.m_staticBlock = bStatic ? true : false;
    if (m_blockSizes.empty())
    {
        m_blockSizes.emplace_back(dwSpaceRequested);
    }
    else
    {
        m_blockSizes[0] = dwSpaceRequested;
    }

    MHW_MI_CHK_STATUS(heapManager->AcquireSpace(
        acquireParams,
        m_blocks,
        spaceNeeded));

    if (m_blocks.empty())
    {
        MHW_ASSERTMESSAGE("No blocks were acquired");
        return MOS_STATUS_UNKNOWN;
    }
    if (!m_blocks[0].IsValid())
    {
        MHW_ASSERTMESSAGE("No blocks were acquired");
        return MOS_STATUS_UNKNOWN;
    }

    *requestedBlock = m_blocks[0];

    if (bZeroAssignedMem)
    {
        requestedBlock->AddData(nullptr, 0, 0, true);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS XMHW_STATE_HEAP_INTERFACE::SubmitBlocks(PMHW_KERNEL_STATE pKernelState)
{
    MHW_MI_CHK_NULL(pKernelState);
    if (!pKernelState->m_ishRegion.IsStatic())
    {
        std::vector<MemoryBlock> block;
        block.push_back(pKernelState->m_ishRegion);
        MHW_MI_CHK_STATUS(m_ishManager.SubmitBlocks(block));
    }
    if (!pKernelState->m_dshRegion.IsStatic())
    {
        std::vector<MemoryBlock> block;
        block.push_back(pKernelState->m_dshRegion);
        MHW_MI_CHK_STATUS(m_dshManager.SubmitBlocks(block));
    }

    pKernelState->m_currTrackerId = MemoryBlock::m_invalidTrackerId;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS XMHW_STATE_HEAP_INTERFACE::LockStateHeap(
    PMHW_STATE_HEAP             pStateHeap)
{
    PMOS_INTERFACE  pOsInterface = nullptr;
    MOS_LOCK_PARAMS LockParams;
    MOS_STATUS      eStatus = MOS_STATUS_SUCCESS;

    MHW_CHK_NULL(pStateHeap);

    if (pStateHeap->bKeepLocked)
    {
        MHW_CHK_NULL(pStateHeap->pvLockedHeap);
        goto finish;
    }

    pOsInterface = m_pOsInterface;

    MOS_ZeroMemory(&LockParams, sizeof(LockParams));
    LockParams.WriteOnly = 1;
    LockParams.NoOverWrite = 1;
    LockParams.Uncached = 1;
    pStateHeap->pvLockedHeap =
        pOsInterface->pfnLockResource(pOsInterface, &pStateHeap->resHeap, &LockParams);
    MHW_CHK_NULL(pStateHeap->pvLockedHeap);

finish:
    return eStatus;
}

MOS_STATUS  XMHW_STATE_HEAP_INTERFACE::UnLockStateHeap(
    PMHW_STATE_HEAP             pStateHeap)
{
    MOS_STATUS      eStatus = MOS_STATUS_SUCCESS;

    MHW_CHK_NULL(pStateHeap);

    if (pStateHeap->bKeepLocked)
    {
        MHW_CHK_NULL(pStateHeap->pvLockedHeap);
        goto finish;
    }

    MHW_CHK_STATUS(m_pOsInterface->pfnUnlockResource(m_pOsInterface, &pStateHeap->resHeap));

    pStateHeap->pvLockedHeap = nullptr;

finish:
    return eStatus;
}

MOS_STATUS XMHW_STATE_HEAP_INTERFACE::ExtendStateHeap(
    MHW_STATE_HEAP_TYPE         StateHeapType,
    uint32_t                    dwSizeRequested)
{
    if (m_bDynamicMode == MHW_DSH_MODE)
    {
        return ExtendStateHeapDyn(StateHeapType,dwSizeRequested);
    }
    else if (m_bDynamicMode == MHW_RENDER_HAL_MODE)
    {
        return ExtendStateHeapSta(StateHeapType,dwSizeRequested);
    }
    else
    {
        return MOS_STATUS_UNKNOWN;
    }
}

MOS_STATUS XMHW_STATE_HEAP_INTERFACE::UpdateGlobalCmdBufId()
{
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    m_SurfaceStateHeap.dwCurrOffset = 0;

    return eStatus;
}

MOS_STATUS XMHW_STATE_HEAP_INTERFACE::SetCmdBufStatusPtr(void *pvCmdBufStatus)
{
    MHW_FUNCTION_ENTER;
    MHW_MI_CHK_NULL(pvCmdBufStatus);
    m_pdwCmdBufIdGlobal = (uint32_t*)pvCmdBufStatus;
    MHW_MI_CHK_STATUS(m_ishManager.RegisterTrackerResource((uint32_t*)m_pdwCmdBufIdGlobal));
    MHW_MI_CHK_STATUS(m_dshManager.RegisterTrackerResource((uint32_t*)m_pdwCmdBufIdGlobal));
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS XMHW_STATE_HEAP_INTERFACE::RequestSshSpaceForCmdBuf(
    uint32_t                    dwBtEntriesRequested)
{
    PMOS_INTERFACE      pOsInterface;
    PMHW_STATE_HEAP     pStateHeap;
    MOS_COMMAND_BUFFER  CmdBuffer;
    uint32_t            dwRequestedSshSize = 0, dwBtSize = 0;
    uint32_t            uiExistingSshSize = 0, uiExistingSshOffset = 0;
    MOS_STATUS          eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    pOsInterface    = m_pOsInterface;
    pStateHeap      = &m_SurfaceStateHeap;
    MHW_CHK_NULL(pOsInterface);
    MHW_CHK_NULL(pStateHeap);

    MHW_CHK_STATUS(pOsInterface->pfnGetIndirectState(
        pOsInterface,
        &uiExistingSshOffset,
        &uiExistingSshSize));
    pStateHeap->dwSize = (uint32_t)uiExistingSshSize;

    dwBtSize =
        MOS_ALIGN_CEIL(dwBtEntriesRequested, m_wBtIdxAlignment);
    MHW_CHK_STATUS(CalculateSshAndBtSizesRequested(
        dwBtEntriesRequested,
        &dwRequestedSshSize,
        &dwBtSize));
    dwRequestedSshSize = MOS_ALIGN_CEIL(dwRequestedSshSize, (1 << MHW_SSH_BASE_SHIFT));

    if (dwRequestedSshSize > pStateHeap->dwSize)
    {
        MHW_CHK_STATUS(pOsInterface->pfnSetIndirectStateSize(
            pOsInterface,
            (uint32_t)dwRequestedSshSize));
        MOS_ZeroMemory(&CmdBuffer, sizeof(CmdBuffer));
        MHW_CHK_STATUS(pOsInterface->pfnGetCommandBuffer(pOsInterface, &CmdBuffer, 0));
        MHW_CHK_STATUS(pOsInterface->pfnResetCommandBuffer(pOsInterface, &CmdBuffer));
        pOsInterface->pfnReturnCommandBuffer(pOsInterface, &CmdBuffer, 0);
        pOsInterface->pfnResetOsStates(pOsInterface);

        m_SurfaceStateHeap.dwSize = dwRequestedSshSize;
    }

finish:
    return eStatus;
}

MOS_STATUS XMHW_STATE_HEAP_INTERFACE::ExtendStateHeapDyn(
        MHW_STATE_HEAP_TYPE         StateHeapType,
        uint32_t                    dwSizeRequested)
{
    PMHW_STATE_HEAP             pNewStateHeap = nullptr;
    PMHW_STATE_HEAP            *ppStateHeapPtr;
    MOS_ALLOC_GFXRES_PARAMS     AllocParams;
    uint32_t                    dwNumHeaps;
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;
    PMHW_BLOCK_MANAGER          pBlockManager = nullptr;

    MHW_FUNCTION_ENTER;

    pNewStateHeap = (PMHW_STATE_HEAP)MOS_AllocAndZeroMemory(sizeof(MHW_STATE_HEAP));
    MHW_CHK_NULL(pNewStateHeap);

    pNewStateHeap->dwSize = MOS_ALIGN_CEIL(dwSizeRequested, MHW_CACHELINE_SIZE);
    pNewStateHeap->dwUsed = 0;
    pNewStateHeap->dwFree = pNewStateHeap->dwSize;

    pNewStateHeap->pMhwStateHeapInterface = this;

    MOS_ZeroMemory(&AllocParams, sizeof(AllocParams));
    AllocParams.Type      = MOS_GFXRES_BUFFER;
    AllocParams.TileType  = MOS_TILE_LINEAR;
    AllocParams.Format    = Format_Buffer;
    AllocParams.dwBytes   = pNewStateHeap->dwSize;
    AllocParams.pBufName  = "DynamicStateHeap";
    MHW_CHK_STATUS(m_pOsInterface->pfnAllocateResource(
        m_pOsInterface,
        &AllocParams,
        &pNewStateHeap->resHeap));
    MHW_CHK_STATUS(m_pOsInterface->pfnRegisterResource(m_pOsInterface, &pNewStateHeap->resHeap,
                                                       true, true));
    if (StateHeapType == MHW_ISH_TYPE)
    {
        if (m_StateHeapSettings.m_keepIshLocked)
        {
            MHW_CHK_STATUS(LockStateHeap(pNewStateHeap));
            pNewStateHeap->bKeepLocked = true;
        }

        ppStateHeapPtr = &m_pInstructionStateHeaps;
        dwNumHeaps     = m_dwNumIsh++;
        pBlockManager  = m_pIshBlockManager;
    }
    else
    {
        if (m_StateHeapSettings.m_keepDshLocked)
        {
            MHW_CHK_STATUS(LockStateHeap(pNewStateHeap));
            pNewStateHeap->bKeepLocked = true;
        }

        ppStateHeapPtr = &m_pDynamicStateHeaps;
        dwNumHeaps     = m_dwNumDsh++;
        pBlockManager  = m_pDshBlockManager;
    }

    pNewStateHeap->pNext = *ppStateHeapPtr;
    *ppStateHeapPtr = pNewStateHeap;
    if (pNewStateHeap->pNext) pNewStateHeap->pNext->pPrev = pNewStateHeap;

    pBlockManager->SetStateHeap(pNewStateHeap);

    // Register new state heap with block manager
    pBlockManager->RegisterStateHeap(pNewStateHeap);

finish:
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        if (pNewStateHeap)
        {
            if (m_pOsInterface)
            {
                m_pOsInterface->pfnFreeResource(m_pOsInterface, &pNewStateHeap->resHeap);
            }

            MOS_FreeMemory(pNewStateHeap);
        }
    }

    return eStatus;
}

MOS_STATUS XMHW_STATE_HEAP_INTERFACE::ExtendStateHeapSta(
        MHW_STATE_HEAP_TYPE         StateHeapType,
        uint32_t                    dwSizeRequested)
{
    PMOS_INTERFACE              pOsInterface = nullptr;
    PMHW_STATE_HEAP             pNewStateHeap = nullptr, pPrevStateHeap, *ppStateHeapPtr;
    MOS_ALLOC_GFXRES_PARAMS     AllocParams;
    uint32_t                    i, dwNumHeaps;
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    pOsInterface = m_pOsInterface;
    MHW_CHK_NULL(pOsInterface);

    pNewStateHeap = (PMHW_STATE_HEAP)MOS_AllocAndZeroMemory(sizeof(MHW_STATE_HEAP));
    MHW_CHK_NULL(pNewStateHeap);

    pNewStateHeap->dwSize = MOS_ALIGN_CEIL(dwSizeRequested, MHW_CACHELINE_SIZE);
    MOS_ZeroMemory(&AllocParams, sizeof(AllocParams));
    AllocParams.Type = MOS_GFXRES_BUFFER;
    AllocParams.TileType = MOS_TILE_LINEAR;
    AllocParams.Format = Format_Buffer;
    AllocParams.dwBytes = pNewStateHeap->dwSize;
    AllocParams.pBufName = "StateHeap";
    MHW_CHK_STATUS(pOsInterface->pfnAllocateResource(
        pOsInterface,
        &AllocParams,
        &pNewStateHeap->resHeap));

    MHW_CHK_STATUS(InitMemoryBlock(
        pNewStateHeap,
        &pNewStateHeap->pMemoryHead,
        pNewStateHeap->dwSize,
        false));

    if (StateHeapType == MHW_ISH_TYPE)
    {
        ppStateHeapPtr = &m_pInstructionStateHeaps;
        dwNumHeaps = m_dwNumIsh++;
    }
    else
    {
        ppStateHeapPtr = &m_pDynamicStateHeaps;
        dwNumHeaps = m_dwNumDsh++;
    }
    MHW_CHK_NULL(ppStateHeapPtr);
    pPrevStateHeap = nullptr;
    for (i = 0; i < dwNumHeaps; i++)
    {
        pPrevStateHeap = *ppStateHeapPtr;
        ppStateHeapPtr = &(*ppStateHeapPtr)->pNext;
    }

    *ppStateHeapPtr = pNewStateHeap;
    pNewStateHeap->pPrev = pPrevStateHeap;

finish:
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        if (pNewStateHeap)
        {
            MOS_FreeMemAndSetNull(pNewStateHeap->pMemoryHead);
            if (pOsInterface)
            {
                pOsInterface->pfnFreeResource(pOsInterface, &pNewStateHeap->resHeap);
            }
            MOS_FreeMemory(pNewStateHeap);
        }
    }

    return eStatus;
}

uint32_t XMHW_STATE_HEAP_INTERFACE::CalculateSpaceNeededDyn(
    MHW_STATE_HEAP_TYPE                  StateHeapType,
    PMHW_STATE_HEAP_DYNAMIC_ALLOC_PARAMS pParams)
{

    PMHW_STATE_HEAP                 pStateHeap;
    PMHW_BLOCK_MANAGER              pBlockManager = nullptr;
    uint32_t                        dwNeeded = 0;
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_CHK_NULL(pParams);
    MHW_CHK_NULL(pParams->piSizes);

    if (pParams->iCount <= 0)
    {
        goto finish;
    }

    if (StateHeapType == MHW_ISH_TYPE)
    {
        MHW_CHK_NULL(m_pInstructionStateHeaps);
        pStateHeap = m_pInstructionStateHeaps;
        pBlockManager = m_pIshBlockManager;
    }
    else if (StateHeapType == MHW_DSH_TYPE)
    {
        MHW_CHK_NULL(m_pDynamicStateHeaps);
        pStateHeap = m_pDynamicStateHeaps;
        pBlockManager = m_pDshBlockManager;
    }
    else
    {
        MHW_ASSERTMESSAGE("Unsupported state heap type.");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        goto finish;
    }

    // Allocate simple block
    MHW_CHK_NULL(pBlockManager);
    dwNeeded = pBlockManager->CalculateSpaceNeeded((uint32_t*)pParams->piSizes, pParams->iCount,
                                                   pParams->dwAlignment, pParams->bHeapAffinity, pParams->pHeapAffinity);

finish:
    return dwNeeded;

}

MOS_STATUS XMHW_STATE_HEAP_INTERFACE::CalculateSshAndBtSizesRequested(
        uint32_t                    dwBtEntriesRequested,
        uint32_t                    *pdwSshSize,
        uint32_t                    *pdwBtSize)
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_CHK_NULL(pdwSshSize);
    MHW_CHK_NULL(pdwBtSize);

    dwBtEntriesRequested =
        MOS_ALIGN_CEIL(dwBtEntriesRequested, m_wBtIdxAlignment);
    *pdwBtSize  = dwBtEntriesRequested * m_HwSizes.dwSizeBindingTableState;
    *pdwSshSize =
        (*pdwBtSize) + (dwBtEntriesRequested * m_dwMaxSurfaceStateSize);

finish:
    return eStatus;
}

MOS_STATUS XMHW_STATE_HEAP_INTERFACE::ReleaseStateHeapDyn(
    PMHW_STATE_HEAP pStateHeap)
{
    PMHW_STATE_HEAP pFirstHeap;
    MOS_STATUS      eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_CHK_NULL(pStateHeap);
    MHW_CHK_NULL(pStateHeap->pBlockManager);

    // Mark state heap for deletion (so size is not accounted for)
    pStateHeap->bDeleted = true;

    // Mark blocks for deletion, release heap when all blocks are released
    eStatus = pStateHeap->pBlockManager->UnregisterStateHeap(pStateHeap);
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        // The only reason for this condition is in case some blocks are still in submitted state
        // The block manager will call this function from within when the last block is released
        // In the meantime, this heap will not contain any free blocks for allocation
        eStatus = MOS_STATUS_SUCCESS;
        goto finish;
    }

    // Find first heap of the chain (so we can figure out if its DSH or ISH!)
    for (pFirstHeap = pStateHeap; pFirstHeap->pPrev != nullptr; pFirstHeap = pFirstHeap->pPrev) ;

    // Detach heap from chain
    if (pStateHeap->pPrev)
    {
        pStateHeap->pPrev->pNext = pStateHeap->pNext;
    }

    if (pStateHeap->pNext)
    {
        pStateHeap->pNext->pPrev = pStateHeap->pPrev;
    }

    // Update heaps
    if (pFirstHeap == m_pDynamicStateHeaps)
    {
        m_dwNumDsh--;
        if (pStateHeap == m_pDynamicStateHeaps)
        {
            m_pDynamicStateHeaps           = pStateHeap->pNext;
            m_pDshBlockManager->SetStateHeap(pStateHeap->pNext);
        }
    }
    else if (pFirstHeap == m_pInstructionStateHeaps)
    {
        m_dwNumIsh--;
        if (pStateHeap == m_pInstructionStateHeaps)
        {
            m_pInstructionStateHeaps       = pStateHeap->pNext;
            m_pIshBlockManager->SetStateHeap(pStateHeap->pNext);
        }
    }

    // Unlock heap
    if (pStateHeap->bKeepLocked)
    {
        pStateHeap->bKeepLocked = false;
        UnLockStateHeap(pStateHeap);
    }

    // Free OS resource
    MHW_CHK_NULL(m_pOsInterface);
    m_pOsInterface->pfnFreeResource(m_pOsInterface, &pStateHeap->resHeap);

    // Free MHW State Heap structure
    MOS_FreeMemory(pStateHeap);

finish:
    return eStatus;
}

PMHW_STATE_HEAP_MEMORY_BLOCK  XMHW_STATE_HEAP_INTERFACE::AllocateDynamicBlockDyn(
        MHW_STATE_HEAP_TYPE                  StateHeapType,
        PMHW_STATE_HEAP_DYNAMIC_ALLOC_PARAMS pParams)
{
    PMHW_STATE_HEAP                *ppStateHeap;
    PMHW_BLOCK_MANAGER              pBlockManager = nullptr;
    PMHW_STATE_HEAP_MEMORY_BLOCK    pMemoryBlock = nullptr;
    PMHW_STATE_HEAP_MEMORY_BLOCK    pAuxBlock;
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;
    uint32_t                        dwMinSize, dwIncrement, dwMaxSize;

    MHW_FUNCTION_ENTER;

    MHW_CHK_NULL(pParams);
    MHW_CHK_NULL(pParams->piSizes);
    MHW_ASSERT(pParams->iCount > 0);

    if (StateHeapType == MHW_ISH_TYPE)
    {
        ppStateHeap   = &m_pInstructionStateHeaps;
        pBlockManager = m_pIshBlockManager;
        dwMinSize     = m_StateHeapSettings.dwIshSize;
        dwIncrement   = m_StateHeapSettings.dwIshIncrement;
        dwMaxSize     = m_StateHeapSettings.dwIshMaxSize;
    }
    else if (StateHeapType == MHW_DSH_TYPE)
    {
        ppStateHeap   = &m_pDynamicStateHeaps;
        pBlockManager = m_pDshBlockManager;
        dwMinSize     = m_StateHeapSettings.dwDshSize;
        dwIncrement   = m_StateHeapSettings.dwDshIncrement;
        dwMaxSize     = m_StateHeapSettings.dwDshMaxSize;
    }
    else
    {
        MHW_ASSERTMESSAGE("Unsupported state heap type.");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        goto finish;
    }

    do
    {
        // Allocate simple block
        MHW_CHK_NULL(pBlockManager);
        if (pParams->iCount == 1)
        {
            if (pParams->dwScratchSpace == 0 )
            {
                pMemoryBlock = pBlockManager->AllocateBlock((uint32_t)pParams->piSizes[0],
                                                         pParams->dwAlignment,
                                                         pParams->pHeapAffinity);
            }
            else
            {
                pMemoryBlock = pBlockManager->AllocateWithScratchSpace(
                                                         (uint32_t)pParams->piSizes[0],
                                                         pParams->dwAlignment,
                                                         pParams->dwScratchSpace);
            }

            if (pMemoryBlock)
            {
                pParams->pScratchSpace  = pMemoryBlock->pStateHeap->pScratchSpace;
                pParams->dwScratchSpace = pMemoryBlock->pStateHeap->dwScratchSpace;
            }
        }
        else
        {
            pMemoryBlock = pBlockManager->AllocateMultiple( (uint32_t*)pParams->piSizes,
                                                             pParams->iCount,
                                                             pParams->dwAlignment,
                                                             pParams->bHeapAffinity,
                                                             pParams->pHeapAffinity);
        }

        // Allocation failed
        if (!pMemoryBlock)
        {
            uint32_t dwTotalSize = 0;

            // Do not allow heap to grow automatically
            // Note: Caller may try to clean up the heap, wait or implement a different heap expansion logic
            if (!pParams->bGrow)
            {
                break;
            }

            // Calculate size of all heaps (do not account heaps already being removed)
            for (PMHW_STATE_HEAP pStateHeap = *ppStateHeap; pStateHeap; pStateHeap = pStateHeap->pNext)
            {
                if (!pStateHeap->bDeleted)
                {
                    dwTotalSize += pStateHeap->dwSize;
                }
            }

            // Did not reach heap size limit - calculate increment to fit all allocations + scratch space (if GSH)
            uint32_t dwExtendSize = 0;
            if (dwTotalSize < dwMaxSize)
            {
                for (int32_t i = 0; i < pParams->iCount; i++)
                {
                    dwExtendSize += MOS_ALIGN_CEIL(pParams->piSizes[i], pParams->dwAlignment);
                }
                dwExtendSize = MOS_ALIGN_CEIL(dwExtendSize, dwIncrement);
                dwExtendSize = MOS_ALIGN_CEIL(dwExtendSize + pParams->dwScratchSpace, dwIncrement);
                dwExtendSize = MOS_MAX(dwExtendSize, dwMinSize);

                ExtendStateHeap(StateHeapType, dwExtendSize);
            }
            else
            {
                break;
            }
        }
    } while (pMemoryBlock == nullptr);

    // Zero memory blocks
    pAuxBlock = pMemoryBlock;
    for (int32_t i = pParams->iCount; (pAuxBlock != nullptr) && (i > 0);  i--, pAuxBlock = pAuxBlock->pNext)
    {
        // Set block static flag
        pAuxBlock->bStatic = pParams->bStatic;

        // Erase block contents
        if (pParams->bZeroAssignedMem)
        {
            MHW_CHK_STATUS(LockStateHeap(pAuxBlock->pStateHeap));
            MOS_ZeroMemory(pAuxBlock->pDataPtr - pAuxBlock->dwAlignment, pAuxBlock->dwBlockSize);
            MHW_CHK_STATUS(UnLockStateHeap(pAuxBlock->pStateHeap));
        }
    }

finish:
    // Failed - release memory blocks back to "Free" queue
    if (eStatus != MOS_STATUS_SUCCESS && pBlockManager != nullptr)
    {
        // Something went wrong - release blocks if already allocated
        for (; pMemoryBlock != nullptr; pMemoryBlock = pAuxBlock)
        {
            pAuxBlock = pMemoryBlock->pNext;            // Get next block (must be done before Mhw_BlockManager_Free)
            pBlockManager->FreeBlock(pMemoryBlock);  // Release block back to "Free" queue
        }
    }

    return pMemoryBlock;
}

MOS_STATUS XMHW_STATE_HEAP_INTERFACE::SubmitDynamicBlockDyn(
        MHW_STATE_HEAP_TYPE                  StateHeapType,
        PMHW_STATE_HEAP_MEMORY_BLOCK         pBlock,
        const FrameTrackerTokenFlat          *trakcerToken)
{
    PMHW_BLOCK_MANAGER  pBlockManager = nullptr;
    MOS_STATUS          eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_CHK_NULL(pBlock);

    if (StateHeapType == MHW_ISH_TYPE)
    {
        pBlockManager = m_pIshBlockManager;
    }
    else if (StateHeapType == MHW_DSH_TYPE)
    {
        pBlockManager = m_pDshBlockManager;
    }
    else
    {
        MHW_ASSERTMESSAGE("Unsupported state heap type.");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        goto finish;
    }

    // Submit block
    MHW_CHK_NULL(pBlockManager);
    MHW_CHK_STATUS(pBlockManager->SubmitBlock(pBlock, trakcerToken));

finish:
    return eStatus;
}

MOS_STATUS XMHW_STATE_HEAP_INTERFACE::FreeDynamicBlockDyn(
        MHW_STATE_HEAP_TYPE                  StateHeapType,
        PMHW_STATE_HEAP_MEMORY_BLOCK         pBlock)
{
    PMHW_BLOCK_MANAGER  pBlockManager = nullptr;
    MOS_STATUS          eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_CHK_NULL(pBlock);

    if (StateHeapType == MHW_ISH_TYPE)
    {
        pBlockManager = m_pIshBlockManager;
    }
    else if (StateHeapType == MHW_DSH_TYPE)
    {
        pBlockManager = m_pDshBlockManager;
    }
    else
    {
        MHW_ASSERTMESSAGE("Unsupported state heap type.");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        goto finish;
    }

    // Free block
    MHW_CHK_STATUS(pBlockManager->FreeBlock(pBlock));

finish:
    return eStatus;
}

MOS_STATUS XMHW_STATE_HEAP_INTERFACE::RefreshDynamicHeapDyn (
    MHW_STATE_HEAP_TYPE         StateHeapType)
{
    PMHW_BLOCK_MANAGER  pBlockManager = nullptr;
    MOS_STATUS          eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    if (StateHeapType == MHW_ISH_TYPE)
    {
        pBlockManager = m_pIshBlockManager;
    }
    else if (StateHeapType == MHW_DSH_TYPE)
    {
        pBlockManager = m_pDshBlockManager;
    }
    else
    {
        MHW_ASSERTMESSAGE("Unsupported state heap type.");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        goto finish;
    }

    // Free block
    MHW_CHK_NULL(pBlockManager)
    MHW_CHK_STATUS(pBlockManager->Refresh());

finish:
    return eStatus;
}

