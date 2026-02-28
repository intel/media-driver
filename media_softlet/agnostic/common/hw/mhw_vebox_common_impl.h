/*
* Copyright (c) 2026, Intel Corporation
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
//! \file     mhw_vebox_common_impl.h
//! \brief    MHW VEBOX common implementation helper functions
//! \details  Common helper utilities for VEBOX_STATE command initialization
//!           across multiple platforms. These non-member helper functions
//!           extract duplicated logic to improve maintainability.
//!

#ifndef __MHW_VEBOX_COMMON_IMPL_H__
#define __MHW_VEBOX_COMMON_IMPL_H__

#include "mos_os.h"
#include "mhw_vebox_itf.h"
#include "mhw_utilities_next.h"
#include "hal_oca_interface_next.h"

namespace mhw {
namespace vebox {
namespace common {

//!
//! \brief    Helper function for adding VEBOX resource to command
//! \details  Initializes MHW_RESOURCE_PARAMS and calls AddResourceToCmd,
//!           handles command buffer dump and OCA interface tracing
//! \param    [in] pOsInterface
//!           Pointer to OS interface
//! \param    [in] pCmdBuffer
//!           Pointer to command buffer
//! \param    [in] pResource
//!           Pointer to resource to add
//! \param    [in] dwOffset
//!           Offset in resource
//! \param    [in,out] pdwCmd
//!           Pointer to command DW to update
//! \param    [in] dwLocationInCmd
//!           Location in command
//! \param    [in] bUseSharedMocs
//!           Whether to use shared MOCS
//! \param    [in] dwSharedMocsOffset
//!           Shared MOCS offset (used when bUseSharedMocs is true)
//! \param    [in] pOsContext
//!           Pointer to OS context
//! \param    [in] pVeboxHeap
//!           Pointer to VEBOX heap
//! \param    [in] resourceName
//!           Resource name for command buffer dump
//! \param    [in] stateSize
//!           State size for OCA interface
//! \param    [in] AddResourceToCmd
//!           Function pointer to AddResourceToCmd
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
template <typename cmd_t>
inline MOS_STATUS AddVeboxResource(
    PMOS_INTERFACE pOsInterface,
    PMOS_COMMAND_BUFFER pCmdBuffer,
    PMOS_RESOURCE pResource,
    uint32_t dwOffset,
    uint32_t* pdwCmd,
    uint32_t dwLocationInCmd,
    bool bUseSharedMocs,
    int32_t dwSharedMocsOffset,
    PMOS_CONTEXT pOsContext,
    MHW_VEBOX_HEAP* pVeboxHeap,
    const char* resourceName,
    uint32_t stateSize,
    MOS_STATUS(*AddResourceToCmd)(PMOS_INTERFACE, PMOS_COMMAND_BUFFER, PMHW_RESOURCE_PARAMS))
{
    MHW_RESOURCE_PARAMS ResourceParams = {};
    uint32_t* pIndirectState = nullptr;

    MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
    ResourceParams.presResource = pResource;
    ResourceParams.dwOffset = dwOffset;
    ResourceParams.pdwCmd = pdwCmd;
    ResourceParams.dwLocationInCmd = dwLocationInCmd;
    ResourceParams.HwCommandType = MOS_VEBOX_STATE;
    
    if (bUseSharedMocs)
    {
        ResourceParams.dwSharedMocsOffset = dwSharedMocsOffset;
    }

    MHW_CHK_STATUS_RETURN(AddResourceToCmd(
        pOsInterface,
        pCmdBuffer,
        &ResourceParams));

#if MOS_COMMAND_BUFFER_DUMP_SUPPORTED
    if (*ResourceParams.pdwCmd != 0 || *(ResourceParams.pdwCmd + 1) != 0)
    {
        pIndirectState = (uint32_t*)(pVeboxHeap->pLockedDriverResourceMem + ResourceParams.dwOffset);
        pOsInterface->pfnAddIndirectState(pOsInterface,
            stateSize,
            pIndirectState,
            ResourceParams.pdwCmd,
            ResourceParams.pdwCmd + 1,
            resourceName);
    }
#endif

    HalOcaInterfaceNext::OnIndirectState(*pCmdBuffer, (MOS_CONTEXT_HANDLE)pOsContext, ResourceParams.presResource, ResourceParams.dwOffset, false, stateSize);

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Helper function for DNDI state setup
//! \param    [in,out] cmd
//!           Reference to VEBOX_STATE command
//! \param    [in] pOsInterface
//!           Pointer to OS interface
//! \param    [in] pCmdBuffer
//!           Pointer to command buffer
//! \param    [in] pVeboxHeap
//!           Pointer to VEBOX heap
//! \param    [in] uiInstanceBaseAddr
//!           Instance base address
//! \param    [in] bCmBuffer
//!           Whether using CM buffer
//! \param    [in] pVeboxParamResource
//!           Pointer to VEBOX parameter resource
//! \param    [in] pOsContext
//!           Pointer to OS context
//! \param    [in] uiDndiStateSize
//!           DNDI state size
//! \param    [in] AddResourceToCmd
//!           Function pointer to AddResourceToCmd
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
template <typename cmd_t>
inline MOS_STATUS SetupVeboxDndiState(
    typename cmd_t::VEBOX_STATE_CMD& cmd,
    PMOS_INTERFACE pOsInterface,
    PMOS_COMMAND_BUFFER pCmdBuffer,
    MHW_VEBOX_HEAP* pVeboxHeap,
    uint32_t uiInstanceBaseAddr,
    bool bCmBuffer,
    PMOS_RESOURCE pVeboxParamResource,
    PMOS_CONTEXT pOsContext,
    uint32_t uiDndiStateSize,
    MOS_STATUS(*AddResourceToCmd)(PMOS_INTERFACE, PMOS_COMMAND_BUFFER, PMHW_RESOURCE_PARAMS))
{
    PMOS_RESOURCE pVeboxHeapResource = nullptr;
    uint32_t dwOffset = 0;

    if (bCmBuffer)
    {
        pVeboxHeapResource = pVeboxParamResource;
        dwOffset = pVeboxHeap->uiDndiStateOffset;
    }
    else
    {
        pVeboxHeapResource = &pVeboxHeap->DriverResource;
        dwOffset = pVeboxHeap->uiDndiStateOffset + uiInstanceBaseAddr;
    }

    return AddVeboxResource<cmd_t>(
        pOsInterface,
        pCmdBuffer,
        pVeboxHeapResource,
        dwOffset,
        &(cmd.DW2.Value),
        2,
        false,
        0,
        pOsContext,
        pVeboxHeap,
        "VEBOX_DNDI_STATE_CMD",
        uiDndiStateSize,
        AddResourceToCmd);
}

//!
//! \brief    Helper function for IECP state setup
//! \param    [in,out] cmd
//!           Reference to VEBOX_STATE command
//! \param    [in] pOsInterface
//!           Pointer to OS interface
//! \param    [in] pCmdBuffer
//!           Pointer to command buffer
//! \param    [in] pVeboxHeap
//!           Pointer to VEBOX heap
//! \param    [in] uiInstanceBaseAddr
//!           Instance base address
//! \param    [in] bCmBuffer
//!           Whether using CM buffer
//! \param    [in] pVeboxParamResource
//!           Pointer to VEBOX parameter resource
//! \param    [in] pOsContext
//!           Pointer to OS context
//! \param    [in] uiIecpStateSize
//!           IECP state size
//! \param    [in] AddResourceToCmd
//!           Function pointer to AddResourceToCmd
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
template <typename cmd_t>
inline MOS_STATUS SetupVeboxIecpState(
    typename cmd_t::VEBOX_STATE_CMD& cmd,
    PMOS_INTERFACE pOsInterface,
    PMOS_COMMAND_BUFFER pCmdBuffer,
    MHW_VEBOX_HEAP* pVeboxHeap,
    uint32_t uiInstanceBaseAddr,
    bool bCmBuffer,
    PMOS_RESOURCE pVeboxParamResource,
    PMOS_CONTEXT pOsContext,
    uint32_t uiIecpStateSize,
    MOS_STATUS(*AddResourceToCmd)(PMOS_INTERFACE, PMOS_COMMAND_BUFFER, PMHW_RESOURCE_PARAMS))
{
    PMOS_RESOURCE pVeboxHeapResource = nullptr;
    uint32_t dwOffset = 0;

    if (bCmBuffer)
    {
        pVeboxHeapResource = pVeboxParamResource;
        dwOffset = pVeboxHeap->uiIecpStateOffset;
    }
    else
    {
        pVeboxHeapResource = &pVeboxHeap->DriverResource;
        dwOffset = pVeboxHeap->uiIecpStateOffset + uiInstanceBaseAddr;
    }

    return AddVeboxResource<cmd_t>(
        pOsInterface,
        pCmdBuffer,
        pVeboxHeapResource,
        dwOffset,
        &(cmd.DW4.Value),
        4,
        true,
        1 - 4,
        pOsContext,
        pVeboxHeap,
        "VEBOX_IECP_STATE_CMD",
        uiIecpStateSize,
        AddResourceToCmd);
}

//!
//! \brief    Helper function for HDR/Gamut state setup
//! \param    [in,out] cmd
//!           Reference to VEBOX_STATE command
//! \param    [in] pOsInterface
//!           Pointer to OS interface
//! \param    [in] pCmdBuffer
//!           Pointer to command buffer
//! \param    [in] pVeboxHeap
//!           Pointer to VEBOX heap
//! \param    [in] uiInstanceBaseAddr
//!           Instance base address
//! \param    [in] bCmBuffer
//!           Whether using CM buffer
//! \param    [in] pVeboxParamResource
//!           Pointer to VEBOX parameter resource
//! \param    [in] bHdrEnabled
//!           Whether HDR is enabled
//! \param    [in] pOsContext
//!           Pointer to OS context
//! \param    [in] uiStateSize
//!           State size (HDR or Gamut)
//! \param    [in] AddResourceToCmd
//!           Function pointer to AddResourceToCmd
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
template <typename cmd_t>
inline MOS_STATUS SetupVeboxHdrGamutState(
    typename cmd_t::VEBOX_STATE_CMD& cmd,
    PMOS_INTERFACE pOsInterface,
    PMOS_COMMAND_BUFFER pCmdBuffer,
    MHW_VEBOX_HEAP* pVeboxHeap,
    uint32_t uiInstanceBaseAddr,
    bool bCmBuffer,
    PMOS_RESOURCE pVeboxParamResource,
    bool bHdrEnabled,
    PMOS_CONTEXT pOsContext,
    uint32_t uiStateSize,
    MOS_STATUS(*AddResourceToCmd)(PMOS_INTERFACE, PMOS_COMMAND_BUFFER, PMHW_RESOURCE_PARAMS))
{
    PMOS_RESOURCE pVeboxHeapResource = nullptr;
    uint32_t dwOffset = 0;
    const char* resourceName = nullptr;

    if (bCmBuffer)
    {
        pVeboxHeapResource = pVeboxParamResource;
        dwOffset = bHdrEnabled ? pVeboxHeap->uiHdrStateOffset : pVeboxHeap->uiGamutStateOffset;
    }
    else
    {
        pVeboxHeapResource = &pVeboxHeap->DriverResource;
        dwOffset = (bHdrEnabled ? pVeboxHeap->uiHdrStateOffset : pVeboxHeap->uiGamutStateOffset) + uiInstanceBaseAddr;
    }

    resourceName = bHdrEnabled ? "VEBOX_HDR_STATE_CMD" : "Gamut_Expansion_Gamma_Correction_CMD";

    return AddVeboxResource<cmd_t>(
        pOsInterface,
        pCmdBuffer,
        pVeboxHeapResource,
        dwOffset,
        &(cmd.DW6.Value),
        6,
        true,
        1 - 6,
        pOsContext,
        pVeboxHeap,
        resourceName,
        uiStateSize,
        AddResourceToCmd);
}

//!
//! \brief    Helper function for Vertex Table setup
//! \param    [in,out] cmd
//!           Reference to VEBOX_STATE command
//! \param    [in] pOsInterface
//!           Pointer to OS interface
//! \param    [in] pCmdBuffer
//!           Pointer to command buffer
//! \param    [in] pVeboxHeap
//!           Pointer to VEBOX heap
//! \param    [in] uiInstanceBaseAddr
//!           Instance base address
//! \param    [in] bCmBuffer
//!           Whether using CM buffer
//! \param    [in] pVeboxParamResource
//!           Pointer to VEBOX parameter resource
//! \param    [in] pOsContext
//!           Pointer to OS context
//! \param    [in] uiVertexTableSize
//!           Vertex table size
//! \param    [in] AddResourceToCmd
//!           Function pointer to AddResourceToCmd
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
template <typename cmd_t>
inline MOS_STATUS SetupVeboxVertexTable(
    typename cmd_t::VEBOX_STATE_CMD& cmd,
    PMOS_INTERFACE pOsInterface,
    PMOS_COMMAND_BUFFER pCmdBuffer,
    MHW_VEBOX_HEAP* pVeboxHeap,
    uint32_t uiInstanceBaseAddr,
    bool bCmBuffer,
    PMOS_RESOURCE pVeboxParamResource,
    PMOS_CONTEXT pOsContext,
    uint32_t uiVertexTableSize,
    MOS_STATUS(*AddResourceToCmd)(PMOS_INTERFACE, PMOS_COMMAND_BUFFER, PMHW_RESOURCE_PARAMS))
{
    PMOS_RESOURCE pVeboxHeapResource = nullptr;
    uint32_t dwOffset = 0;

    if (bCmBuffer)
    {
        pVeboxHeapResource = pVeboxParamResource;
        dwOffset = pVeboxHeap->uiVertexTableOffset;
    }
    else
    {
        pVeboxHeapResource = &pVeboxHeap->DriverResource;
        dwOffset = pVeboxHeap->uiVertexTableOffset + uiInstanceBaseAddr;
    }

    return AddVeboxResource<cmd_t>(
        pOsInterface,
        pCmdBuffer,
        pVeboxHeapResource,
        dwOffset,
        &(cmd.DW8.Value),
        8,
        true,
        1 - 8,
        pOsContext,
        pVeboxHeap,
        "VEBOX_VERTEX_TABLE_CMD",
        uiVertexTableSize,
        AddResourceToCmd);
}

//!
//! \brief    Helper function for Capture Pipe setup
//! \param    [in,out] cmd
//!           Reference to VEBOX_STATE command
//! \param    [in] pOsInterface
//!           Pointer to OS interface
//! \param    [in] pCmdBuffer
//!           Pointer to command buffer
//! \param    [in] pVeboxHeap
//!           Pointer to VEBOX heap
//! \param    [in] uiInstanceBaseAddr
//!           Instance base address
//! \param    [in] bCmBuffer
//!           Whether using CM buffer
//! \param    [in] pVeboxParamResource
//!           Pointer to VEBOX parameter resource
//! \param    [in] pOsContext
//!           Pointer to OS context
//! \param    [in] uiCapturePipeStateSize
//!           Capture pipe state size
//! \param    [in] AddResourceToCmd
//!           Function pointer to AddResourceToCmd
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
template <typename cmd_t>
inline MOS_STATUS SetupVeboxCapturePipe(
    typename cmd_t::VEBOX_STATE_CMD& cmd,
    PMOS_INTERFACE pOsInterface,
    PMOS_COMMAND_BUFFER pCmdBuffer,
    MHW_VEBOX_HEAP* pVeboxHeap,
    uint32_t uiInstanceBaseAddr,
    bool bCmBuffer,
    PMOS_RESOURCE pVeboxParamResource,
    PMOS_CONTEXT pOsContext,
    uint32_t uiCapturePipeStateSize,
    MOS_STATUS(*AddResourceToCmd)(PMOS_INTERFACE, PMOS_COMMAND_BUFFER, PMHW_RESOURCE_PARAMS))
{
    PMOS_RESOURCE pVeboxHeapResource = nullptr;
    uint32_t dwOffset = 0;

    if (bCmBuffer)
    {
        pVeboxHeapResource = pVeboxParamResource;
        dwOffset = pVeboxHeap->uiCapturePipeStateOffset;
    }
    else
    {
        pVeboxHeapResource = &pVeboxHeap->DriverResource;
        dwOffset = pVeboxHeap->uiCapturePipeStateOffset + uiInstanceBaseAddr;
    }

    return AddVeboxResource<cmd_t>(
        pOsInterface,
        pCmdBuffer,
        pVeboxHeapResource,
        dwOffset,
        &(cmd.DW10.Value),
        10,
        true,
        1 - 10,
        pOsContext,
        pVeboxHeap,
        "VEBOX_CAPTURE_PIPE_STATE_CMD",
        uiCapturePipeStateSize,
        AddResourceToCmd);
}

//!
//! \brief    Helper function for LACE LUT setup
//! \param    [in,out] cmd
//!           Reference to VEBOX_STATE command
//! \param    [in] pOsInterface
//!           Pointer to OS interface
//! \param    [in] pCmdBuffer
//!           Pointer to command buffer
//! \param    [in] pLaceLookUpTables
//!           Pointer to LACE lookup tables resource
//! \param    [in] AddResourceToCmd
//!           Function pointer to AddResourceToCmd
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
template <typename cmd_t>
inline MOS_STATUS SetupVeboxLaceLut(
    typename cmd_t::VEBOX_STATE_CMD& cmd,
    PMOS_INTERFACE pOsInterface,
    PMOS_COMMAND_BUFFER pCmdBuffer,
    PMOS_RESOURCE pLaceLookUpTables,
    MOS_STATUS(*AddResourceToCmd)(PMOS_INTERFACE, PMOS_COMMAND_BUFFER, PMHW_RESOURCE_PARAMS))
{
    if (pLaceLookUpTables == nullptr)
    {
        return MOS_STATUS_SUCCESS;
    }

    MHW_RESOURCE_PARAMS ResourceParams = {};
    MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
    ResourceParams.presResource = pLaceLookUpTables;
    ResourceParams.dwOffset = 0;
    ResourceParams.pdwCmd = &(cmd.DW12.Value);
    ResourceParams.dwLocationInCmd = 12;
    ResourceParams.HwCommandType = MOS_VEBOX_STATE;
    ResourceParams.dwSharedMocsOffset = 1 - 12;

    return AddResourceToCmd(
        pOsInterface,
        pCmdBuffer,
        &ResourceParams);
}

//!
//! \brief    Helper function for Gamma Correction setup
//! \param    [in,out] cmd
//!           Reference to VEBOX_STATE command
//! \param    [in] pOsInterface
//!           Pointer to OS interface
//! \param    [in] pCmdBuffer
//!           Pointer to command buffer
//! \param    [in] pVeboxHeap
//!           Pointer to VEBOX heap
//! \param    [in] uiInstanceBaseAddr
//!           Instance base address
//! \param    [in] bCmBuffer
//!           Whether using CM buffer
//! \param    [in] pVeboxParamResource
//!           Pointer to VEBOX parameter resource
//! \param    [in] pOsContext
//!           Pointer to OS context
//! \param    [in] uiGammaCorrectionStateSize
//!           Gamma correction state size
//! \param    [in] AddResourceToCmd
//!           Function pointer to AddResourceToCmd
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
template <typename cmd_t>
inline MOS_STATUS SetupVeboxGammaCorrection(
    typename cmd_t::VEBOX_STATE_CMD& cmd,
    PMOS_INTERFACE pOsInterface,
    PMOS_COMMAND_BUFFER pCmdBuffer,
    MHW_VEBOX_HEAP* pVeboxHeap,
    uint32_t uiInstanceBaseAddr,
    bool bCmBuffer,
    PMOS_RESOURCE pVeboxParamResource,
    PMOS_CONTEXT pOsContext,
    uint32_t uiGammaCorrectionStateSize,
    MOS_STATUS(*AddResourceToCmd)(PMOS_INTERFACE, PMOS_COMMAND_BUFFER, PMHW_RESOURCE_PARAMS))
{
    PMOS_RESOURCE pVeboxHeapResource = nullptr;
    uint32_t dwOffset = 0;

    if (bCmBuffer)
    {
        pVeboxHeapResource = pVeboxParamResource;
        dwOffset = pVeboxHeap->uiGammaCorrectionStateOffset;
    }
    else
    {
        pVeboxHeapResource = &pVeboxHeap->DriverResource;
        dwOffset = pVeboxHeap->uiGammaCorrectionStateOffset + uiInstanceBaseAddr;
    }

    return AddVeboxResource<cmd_t>(
        pOsInterface,
        pCmdBuffer,
        pVeboxHeapResource,
        dwOffset,
        &(cmd.DW14_15.Value[0]),
        14,
        true,
        1 - 14,
        pOsContext,
        pVeboxHeap,
        "PMHW_FORWARD_GAMMA_SEG",
        uiGammaCorrectionStateSize,
        AddResourceToCmd);
}

//!
//! \brief    Helper function for 3D LUT setup
//! \param    [in,out] cmd
//!           Reference to VEBOX_STATE command
//! \param    [in] pOsInterface
//!           Pointer to OS interface
//! \param    [in] pCmdBuffer
//!           Pointer to command buffer
//! \param    [in] pVebox3DLookUpTables
//!           Pointer to 3D LUT resource
//! \param    [in] AddResourceToCmd
//!           Function pointer to AddResourceToCmd
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
template <typename cmd_t>
inline MOS_STATUS SetupVebox3DLut(
    typename cmd_t::VEBOX_STATE_CMD& cmd,
    PMOS_INTERFACE pOsInterface,
    PMOS_COMMAND_BUFFER pCmdBuffer,
    PMOS_RESOURCE pVebox3DLookUpTables,
    MOS_STATUS(*AddResourceToCmd)(PMOS_INTERFACE, PMOS_COMMAND_BUFFER, PMHW_RESOURCE_PARAMS))
{
    if (pVebox3DLookUpTables == nullptr)
    {
        return MOS_STATUS_SUCCESS;
    }

    MHW_RESOURCE_PARAMS ResourceParams = {};
    MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
    ResourceParams.presResource = pVebox3DLookUpTables;
    ResourceParams.dwOffset = 0;
    ResourceParams.pdwCmd = &(cmd.DW16.Value);
    ResourceParams.dwLocationInCmd = 16;
    ResourceParams.HwCommandType = MOS_VEBOX_STATE;
    ResourceParams.dwSharedMocsOffset = 1 - 16;

    return AddResourceToCmd(
        pOsInterface,
        pCmdBuffer,
        &ResourceParams);
}

//!
//! \brief    Helper function for 1D LUT setup
//! \param    [in,out] cmd
//!           Reference to VEBOX_STATE command
//! \param    [in] pOsInterface
//!           Pointer to OS interface
//! \param    [in] pCmdBuffer
//!           Pointer to command buffer
//! \param    [in] pVebox1DLookUpTables
//!           Pointer to 1D LUT resource
//! \param    [in] AddResourceToCmd
//!           Function pointer to AddResourceToCmd
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
template <typename cmd_t>
inline MOS_STATUS SetupVebox1DLut(
    typename cmd_t::VEBOX_STATE_CMD& cmd,
    PMOS_INTERFACE pOsInterface,
    PMOS_COMMAND_BUFFER pCmdBuffer,
    PMOS_RESOURCE pVebox1DLookUpTables,
    MOS_STATUS(*AddResourceToCmd)(PMOS_INTERFACE, PMOS_COMMAND_BUFFER, PMHW_RESOURCE_PARAMS))
{
    if (pVebox1DLookUpTables == nullptr)
    {
        return MOS_STATUS_SUCCESS;
    }

    MHW_RESOURCE_PARAMS ResourceParams = {};
    MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
    ResourceParams.presResource = pVebox1DLookUpTables;
    ResourceParams.dwOffset = 0;
    ResourceParams.pdwCmd = &(cmd.DW21.Value);
    ResourceParams.dwLocationInCmd = 21;
    ResourceParams.HwCommandType = MOS_VEBOX_STATE;
    ResourceParams.dwSharedMocsOffset = 1 - 21;

    return AddResourceToCmd(
        pOsInterface,
        pCmdBuffer,
        &ResourceParams);
}

//!
//! \brief    Helper function for dummy IECP resource setup
//! \param    [in,out] cmd
//!           Reference to VEBOX_STATE command
//! \param    [in] pOsInterface
//!           Pointer to OS interface
//! \param    [in] pCmdBuffer
//!           Pointer to command buffer
//! \param    [in,out] pDummyIecpResource
//!           Pointer to dummy IECP resource
//! \param    [in] uiIecpStateSize
//!           IECP state size
//! \param    [in] pOsContext
//!           Pointer to OS context
//! \param    [in] AddResourceToCmd
//!           Function pointer to AddResourceToCmd
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
template <typename cmd_t>
inline MOS_STATUS SetupDummyIecpResource(
    typename cmd_t::VEBOX_STATE_CMD& cmd,
    PMOS_INTERFACE pOsInterface,
    PMOS_COMMAND_BUFFER pCmdBuffer,
    PMOS_RESOURCE pDummyIecpResource,
    uint32_t uiIecpStateSize,
    PMOS_CONTEXT pOsContext,
    MOS_STATUS (*AddResourceToCmd)(PMOS_INTERFACE, PMOS_COMMAND_BUFFER, PMHW_RESOURCE_PARAMS))
{
    MOS_ALLOC_GFXRES_PARAMS AllocParamsForBufferLinear = {};
    MHW_RESOURCE_PARAMS ResourceParams = {};

    // Allocate Resource to avoid Page Fault issue since HW will access it
    if (Mos_ResourceIsNull(pDummyIecpResource))
    {
        AllocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
        AllocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
        AllocParamsForBufferLinear.Format = Format_Buffer;
        AllocParamsForBufferLinear.dwBytes = uiIecpStateSize;
        AllocParamsForBufferLinear.pBufName = "DummyIecpResource";
        AllocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_VP_INTERNAL_READ_WRITE_FF;

        MHW_CHK_STATUS_RETURN(pOsInterface->pfnAllocateResource(
            pOsInterface,
            &AllocParamsForBufferLinear,
            pDummyIecpResource));
    }

    MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
    ResourceParams.presResource = pDummyIecpResource;
    ResourceParams.dwOffset = 0;
    ResourceParams.pdwCmd = &(cmd.DW4.Value);
    ResourceParams.dwLocationInCmd = 4;
    ResourceParams.HwCommandType = MOS_VEBOX_STATE;
    ResourceParams.dwSharedMocsOffset = 1 - 4;

    MHW_CHK_STATUS_RETURN(AddResourceToCmd(
        pOsInterface,
        pCmdBuffer,
        &ResourceParams));

    HalOcaInterfaceNext::OnIndirectState(*pCmdBuffer, (MOS_CONTEXT_HANDLE)pOsContext, ResourceParams.presResource, 0, true, 0);

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Helper function for mode controls (DW1) setup
//! \param    [in,out] cmd
//!           Reference to VEBOX_STATE command
//! \param    [in] params
//!           Reference to VEBOX_STATE parameters
//! \param    [in] pOsInterface
//!           Pointer to OS interface
//! \param    [in] cacheUsage
//!           Cache usage policy (default: MOS_HW_RESOURCE_USAGE_VP_INTERNAL_READ_WRITE_FF)
//! \return   void
//!
template <typename cmd_t, typename ParamType>
inline void SetupVeboxModeControls(
    typename cmd_t::VEBOX_STATE_CMD& cmd,
    const ParamType& params,
    PMOS_INTERFACE pOsInterface,
    MOS_HW_RESOURCE_DEF cacheUsage = MOS_HW_RESOURCE_USAGE_VP_INTERNAL_READ_WRITE_FF)
{
    cmd.DW1.ColorGamutExpansionEnable = params.VeboxMode.ColorGamutExpansionEnable;
    cmd.DW1.ColorGamutCompressionEnable = params.VeboxMode.ColorGamutCompressionEnable;
    cmd.DW1.GlobalIecpEnable = params.VeboxMode.GlobalIECPEnable;
    cmd.DW1.DnEnable = params.VeboxMode.DNEnable;
    cmd.DW1.DiEnable = params.VeboxMode.DIEnable;
    cmd.DW1.DnDiFirstFrame = params.VeboxMode.DNDIFirstFrame;
    cmd.DW1.DiOutputFrames = params.VeboxMode.DIOutputFrames;
    cmd.DW1.DemosaicEnable = params.VeboxMode.DemosaicEnable;
    cmd.DW1.VignetteEnable = params.VeboxMode.VignetteEnable;
    cmd.DW1.AlphaPlaneEnable = params.VeboxMode.AlphaPlaneEnable;
    cmd.DW1.HotPixelFilteringEnable = params.VeboxMode.HotPixelFilteringEnable;
    cmd.DW1.LaceCorrectionEnable = params.VeboxMode.LACECorrectionEnable;
    cmd.DW1.DisableEncoderStatistics = params.VeboxMode.DisableEncoderStatistics;
    cmd.DW1.DisableTemporalDenoiseFilter = params.VeboxMode.DisableTemporalDenoiseFilter;
    cmd.DW1.SinglePipeEnable = params.VeboxMode.SinglePipeIECPEnable;
    cmd.DW1.ScalarMode = params.VeboxMode.ScalarMode;
    cmd.DW1.ForwardGammaCorrectionEnable = params.VeboxMode.ForwardGammaCorrectionEnable;
    cmd.DW1.HdrEnable = params.VeboxMode.Hdr1DLutEnable;
    cmd.DW1.Fp16ModeEnable = params.VeboxMode.Fp16ModeEnable;
    cmd.DW1.StateSurfaceControlBits = (pOsInterface->pfnCachePolicyGetMemoryObject(
        cacheUsage,
        pOsInterface->pfnGetGmmClientContext(pOsInterface))).DwordValue;
}

//!
//! \brief    Helper function for chroma sampling (DW18) setup
//! \param    [in,out] cmd
//!           Reference to VEBOX_STATE command
//! \param    [in] params
//!           Reference to VEBOX_STATE parameters
//! \return   void
//!
template <typename cmd_t, typename ParamType>
inline void SetupVeboxChromaSampling(
    typename cmd_t::VEBOX_STATE_CMD& cmd,
    const ParamType& params)
{
    cmd.DW18.ChromaUpsamplingCoSitedHorizontalOffset = params.ChromaSampling.ChromaUpsamplingCoSitedHorizontalOffset;
    cmd.DW18.ChromaUpsamplingCoSitedVerticalOffset = params.ChromaSampling.ChromaUpsamplingCoSitedVerticalOffset;
    cmd.DW18.ChromaDownsamplingCoSitedHorizontalOffset = params.ChromaSampling.ChromaDownsamplingCoSitedHorizontalOffset;
    cmd.DW18.ChromaDownsamplingCoSitedVerticalOffset = params.ChromaSampling.ChromaDownsamplingCoSitedVerticalOffset;
    cmd.DW18.BypassChromaUpsampling = params.ChromaSampling.BypassChromaUpsampling;
    cmd.DW18.BypassChromaDownsampling = params.ChromaSampling.BypassChromaDownsampling;
}

//!
//! \brief    Helper function for 3D LUT controls (DW17 for older platforms)
//! \param    [in,out] cmd
//!           Reference to VEBOX_STATE command
//! \param    [in] params
//!           Reference to VEBOX_STATE parameters
//! \return   void
//!
template <typename cmd_t, typename ParamType>
inline void SetupVebox3DLutControlsLegacy(
    typename cmd_t::VEBOX_STATE_CMD& cmd,
    const ParamType& params)
{
    cmd.DW17.EncDataControlFor3DLUT = 0;
    cmd.DW17.Lut3DMemoryLayoutControl = 0;  // Legacy layout
    cmd.DW17.ChannelMappingSwapForLut3D = params.LUT3D.ChannelMappingSwapForLut3D;
    cmd.DW17.ArbitrationPriorityControlForLut3D = params.LUT3D.ArbitrationPriorityControl;
    cmd.DW17.Lut3DMocsTable = params.Vebox3DLookUpTablesSurfCtrl.Gen9.Index;
}

//!
//! \brief    Helper function for 3D LUT controls (DW23 for newer platforms)
//! \param    [in,out] cmd
//!           Reference to VEBOX_STATE command
//! \param    [in] params
//!           Reference to VEBOX_STATE parameters
//! \return   void
//!
template <typename cmd_t, typename ParamType>
inline void SetupVebox3DLutControlsNew(
    typename cmd_t::VEBOX_STATE_CMD& cmd,
    const ParamType& params)
{
    cmd.DW23.ChannelMappingSwapForLut3D = params.LUT3D.ChannelMappingSwapForLut3D;
    cmd.DW23.ArbitrationPriorityControlForLut3D = params.LUT3D.ArbitrationPriorityControl;
    cmd.DW23.Lut3DMocsTable = params.Vebox3DLookUpTablesSurfCtrl.Gen9.Index;
}

//!
//! \brief    Helper function for tracing indirect state information
//! \param    [in] cmdBuffer
//!           Reference to command buffer
//! \param    [in] osContext
//!           Reference to OS context
//! \param    [in] bCmBuffer
//!           Whether using CM buffer
//! \param    [in] bUseKernelResource
//!           Whether using kernel resource
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
inline MOS_STATUS TraceIndirectStateInfo(
    MOS_COMMAND_BUFFER& cmdBuffer,
    MOS_CONTEXT& osContext,
    bool bCmBuffer,
    bool bUseKernelResource)
{
    if (bCmBuffer)
    {
        char ocaLog[] = "Vebox indirect state use CmBuffer";
        HalOcaInterfaceNext::TraceMessage(cmdBuffer, (MOS_CONTEXT_HANDLE)&osContext, ocaLog, sizeof(ocaLog));
    }
    else
    {
        if (bUseKernelResource)
        {
            char ocaLog[] = "Vebox indirect state use KernelResource";
            HalOcaInterfaceNext::TraceMessage(cmdBuffer, (MOS_CONTEXT_HANDLE)&osContext, ocaLog, sizeof(ocaLog));
        }
        else
        {
            char ocaLog[] = "Vebox indirect state use DriverResource";
            HalOcaInterfaceNext::TraceMessage(cmdBuffer, (MOS_CONTEXT_HANDLE)&osContext, ocaLog, sizeof(ocaLog));
        }
    }
    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Helper function for VEBOX surface format determination
//! \details  Extracts surface format determination logic shared across all platforms.
//!           This helper function encapsulates both the primary format switch (handling
//!           25+ surface formats including YUV, RGB, Bayer patterns) and the secondary
//!           Bayer input alignment switch. Each platform's SetVeboxSurfaces can call
//!           this helper function and only handle platform-specific differences.
//! \param    [in] VeboxSurfaceState
//!           Reference to VEBOX_SURFACE_STATE command structure (used for format constants)
//! \param    [in] pSurfaceParam
//!           Pointer to surface parameters containing format and bit depth information
//! \param    [in] bIsOutputSurface
//!           Flag indicating whether this is an output surface
//! \param    [out] dwFormat
//!           Reference to format value to be set
//! \param    [out] bInterleaveChroma
//!           Reference to interleave chroma flag to be set
//! \param    [out] wUYOffset
//!           Reference to U/Y offset value to be set
//! \param    [out] bBayerOffset
//!           Reference to Bayer pattern offset to be set
//! \param    [out] bBayerStride
//!           Reference to Bayer pattern stride to be set
//! \param    [out] bBayerInputAlignment
//!           Reference to Bayer input alignment to be set
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
template <typename cmd_t>
inline MOS_STATUS SetVeboxSurfaceFormat(
    typename cmd_t::VEBOX_SURFACE_STATE_CMD& VeboxSurfaceState,
    PMHW_VEBOX_SURFACE_PARAMS pSurfaceParam,
    bool bIsOutputSurface,
    uint32_t& dwFormat,
    bool& bInterleaveChroma,
    uint16_t& wUYOffset,
    uint8_t& bBayerOffset,
    uint8_t& bBayerStride,
    uint8_t& bBayerInputAlignment)
{
    // Primary format switch - determine surface format based on input format
    switch (pSurfaceParam->Format)
    {
    case Format_NV12:
        dwFormat = VeboxSurfaceState.SURFACE_FORMAT_PLANAR4208;
        bInterleaveChroma = true;
        wUYOffset = (uint16_t)pSurfaceParam->dwUYoffset;
        break;

    case Format_YUYV:
    case Format_YUY2:
        dwFormat = VeboxSurfaceState.SURFACE_FORMAT_YCRCBNORMAL;
        break;

    case Format_UYVY:
        dwFormat = VeboxSurfaceState.SURFACE_FORMAT_YCRCBSWAPY;
        break;

    case Format_AYUV:
        dwFormat = VeboxSurfaceState.SURFACE_FORMAT_PACKED444A8;
        break;

    case Format_Y416:
        dwFormat = VeboxSurfaceState.SURFACE_FORMAT_PACKED44416;
        break;

    case Format_Y410:
        dwFormat = VeboxSurfaceState.SURFACE_FORMAT_PACKED44410;
        break;

    case Format_YVYU:
        dwFormat = VeboxSurfaceState.SURFACE_FORMAT_YCRCBSWAPUV;
        break;

    case Format_VYUY:
        dwFormat = VeboxSurfaceState.SURFACE_FORMAT_YCRCBSWAPUVY;
        break;

    case Format_A8B8G8R8:
    case Format_X8B8G8R8:
        dwFormat = VeboxSurfaceState.SURFACE_FORMAT_R8G8B8A8UNORMR8G8B8A8UNORMSRGB;
        break;

    case Format_A16B16G16R16:
    case Format_A16R16G16B16:
    case Format_A16B16G16R16F:
    case Format_A16R16G16B16F:
        dwFormat = VeboxSurfaceState.SURFACE_FORMAT_R16G16B16A16;
        break;

    case Format_L8:
    case Format_P8:
    case Format_Y8:
        dwFormat = VeboxSurfaceState.SURFACE_FORMAT_Y8UNORM;
        break;

    case Format_IRW0:
        dwFormat = VeboxSurfaceState.SURFACE_FORMAT_BAYERPATTERN;
        bBayerOffset = VeboxSurfaceState.BAYER_PATTERN_OFFSET_PIXELATX0_Y0ISBLUE;
        bBayerStride = VeboxSurfaceState.BAYER_PATTERN_FORMAT_16_BITINPUTATA16_BITSTRIDE;
        break;

    case Format_IRW1:
        dwFormat = VeboxSurfaceState.SURFACE_FORMAT_BAYERPATTERN;
        bBayerOffset = VeboxSurfaceState.BAYER_PATTERN_OFFSET_PIXELATX0_Y0ISRED;
        bBayerStride = VeboxSurfaceState.BAYER_PATTERN_FORMAT_16_BITINPUTATA16_BITSTRIDE;
        break;

    case Format_IRW2:
        dwFormat = VeboxSurfaceState.SURFACE_FORMAT_BAYERPATTERN;
        bBayerOffset = VeboxSurfaceState.BAYER_PATTERN_OFFSET_PIXELATX0_Y0ISGREEN_PIXELATX1_Y0ISRED;
        bBayerStride = VeboxSurfaceState.BAYER_PATTERN_FORMAT_16_BITINPUTATA16_BITSTRIDE;
        break;

    case Format_IRW3:
        dwFormat = VeboxSurfaceState.SURFACE_FORMAT_BAYERPATTERN;
        bBayerOffset = VeboxSurfaceState.BAYER_PATTERN_OFFSET_PIXELATX0_Y0ISGREEN_PIXELATX1_Y0ISBLUE;
        bBayerStride = VeboxSurfaceState.BAYER_PATTERN_FORMAT_16_BITINPUTATA16_BITSTRIDE;
        break;

    case Format_IRW4:
        dwFormat = VeboxSurfaceState.SURFACE_FORMAT_BAYERPATTERN;
        bBayerOffset = VeboxSurfaceState.BAYER_PATTERN_OFFSET_PIXELATX0_Y0ISBLUE;
        bBayerStride = VeboxSurfaceState.BAYER_PATTERN_FORMAT_8_BITINPUTATA8_BITSTRIDE;
        break;

    case Format_IRW5:
        dwFormat = VeboxSurfaceState.SURFACE_FORMAT_BAYERPATTERN;
        bBayerOffset = VeboxSurfaceState.BAYER_PATTERN_OFFSET_PIXELATX0_Y0ISRED;
        bBayerStride = VeboxSurfaceState.BAYER_PATTERN_FORMAT_8_BITINPUTATA8_BITSTRIDE;
        break;

    case Format_IRW6:
        dwFormat = VeboxSurfaceState.SURFACE_FORMAT_BAYERPATTERN;
        bBayerOffset = VeboxSurfaceState.BAYER_PATTERN_OFFSET_PIXELATX0_Y0ISGREEN_PIXELATX1_Y0ISRED;
        bBayerStride = VeboxSurfaceState.BAYER_PATTERN_FORMAT_8_BITINPUTATA8_BITSTRIDE;
        break;

    case Format_IRW7:
        dwFormat = VeboxSurfaceState.SURFACE_FORMAT_BAYERPATTERN;
        bBayerOffset = VeboxSurfaceState.BAYER_PATTERN_OFFSET_PIXELATX0_Y0ISGREEN_PIXELATX1_Y0ISBLUE;
        bBayerStride = VeboxSurfaceState.BAYER_PATTERN_FORMAT_8_BITINPUTATA8_BITSTRIDE;
        break;

    case Format_P010:
    case Format_P016:
        dwFormat = VeboxSurfaceState.SURFACE_FORMAT_PLANAR42016;
        bInterleaveChroma = true;
        wUYOffset = (uint16_t)pSurfaceParam->dwUYoffset;
        break;

    case Format_A8R8G8B8:
    case Format_X8R8G8B8:
        // Platform-specific logic: output surface uses different format
        if (bIsOutputSurface)
        {
            dwFormat = VeboxSurfaceState.SURFACE_FORMAT_B8G8R8A8UNORM;
        }
        else
        {
            dwFormat = VeboxSurfaceState.SURFACE_FORMAT_R8G8B8A8UNORMR8G8B8A8UNORMSRGB;
        }
        break;

    case Format_R10G10B10A2:
    case Format_B10G10R10A2:
        dwFormat = VeboxSurfaceState.SURFACE_FORMAT_R10G10B10A2UNORMR10G10B10A2UNORMSRGB;
        break;

    case Format_Y216:
    case Format_Y210:
        dwFormat = VeboxSurfaceState.SURFACE_FORMAT_PACKED42216;
        break;

    case Format_P216:
    case Format_P210:
        dwFormat = VeboxSurfaceState.SURFACE_FORMAT_PLANAR42216;
        wUYOffset = (uint16_t)pSurfaceParam->dwUYoffset;
        break;

    case Format_Y16S:
    case Format_Y16U:
        dwFormat = VeboxSurfaceState.SURFACE_FORMAT_Y16UNORM;
        break;

    default:
        MHW_ASSERTMESSAGE("Unsupported format.");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    // Secondary Bayer input alignment switch - only for input surfaces
    if (!bIsOutputSurface)
    {
        // Camera pipe will use 10/12/14 for LSB, 0 for MSB. For other pipelines,
        // dwBitDepth is inherited from pSrc->dwDepth which may not be among (0,10,12,14).
        // For such cases should use MSB as default value.
        switch (pSurfaceParam->dwBitDepth)
        {
        case 10:
            bBayerInputAlignment = VeboxSurfaceState.BAYER_INPUT_ALIGNMENT_10BITLSBALIGNEDDATA;
            break;

        case 12:
            bBayerInputAlignment = VeboxSurfaceState.BAYER_INPUT_ALIGNMENT_12BITLSBALIGNEDDATA;
            break;

        case 14:
            bBayerInputAlignment = VeboxSurfaceState.BAYER_INPUT_ALIGNMENT_14BITLSBALIGNEDDATA;
            break;

        case 0:
        default:
            bBayerInputAlignment = VeboxSurfaceState.BAYER_INPUT_ALIGNMENT_MSBALIGNEDDATA;
            break;
        }
    }
    else
    {
        bBayerInputAlignment = VeboxSurfaceState.BAYER_INPUT_ALIGNMENT_MSBALIGNEDDATA;
    }

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Helper function for setting up CSC coefficients for BT601 color space
//! \details  Configures CSC transformation coefficients for BT601 color space.
//!           Supports two modes: Color Balance (YUV to RGB with 16-bit precision)
//!           and Gamut Expansion (YUV to RGB with 10-bit precision).
//! \param    [in,out] pCscState
//!           Pointer to CSC state structure to be configured
//! \param    [in] isColorBalance
//!           Mode selector: true for Color Balance mode, false for Gamut Expansion mode
//! \return   void
//!
template <typename CSCStateType>
inline void SetupCSCCoefficients_BT601(CSCStateType* pCscState, bool isColorBalance)
{
    pCscState->DW0.TransformEnable = true;
    
    if (isColorBalance)
    {
        // Color Balance mode: 16-bit precision coefficients
        pCscState->DW0.C0          = 76309;
        pCscState->DW1.C1          = 0;
        pCscState->DW2.C2          = 104597;
        pCscState->DW3.C3          = 76309;
        pCscState->DW4.C4          = MOS_BITFIELD_VALUE((uint32_t)-25675, 19);
        pCscState->DW5.C5          = MOS_BITFIELD_VALUE((uint32_t)-53279, 19);
        pCscState->DW6.C6          = 76309;
        pCscState->DW7.C7          = 132201;
        pCscState->DW8.C8          = 0;
        pCscState->DW9.OffsetIn1   = MOS_BITFIELD_VALUE((uint32_t)-2048, 16);
        pCscState->DW9.OffsetOut1  = 0;
        pCscState->DW10.OffsetIn2  = MOS_BITFIELD_VALUE((uint32_t)-16384, 16);
        pCscState->DW10.OffsetOut2 = 0;
        pCscState->DW11.OffsetIn3  = MOS_BITFIELD_VALUE((uint32_t)-16384, 16);
        pCscState->DW11.OffsetOut3 = 0;
    }
    else
    {
        // Gamut Expansion mode: 10-bit precision coefficients
        pCscState->DW0.C0          = 1192;
        pCscState->DW1.C1          = MOS_BITFIELD_VALUE((uint32_t)-2, 19);
        pCscState->DW2.C2          = 1634;
        pCscState->DW3.C3          = 1192;
        pCscState->DW4.C4          = MOS_BITFIELD_VALUE((uint32_t)-401, 19);
        pCscState->DW5.C5          = MOS_BITFIELD_VALUE((uint32_t)-833, 19);
        pCscState->DW6.C6          = 1192;
        pCscState->DW7.C7          = 2066;
        pCscState->DW8.C8          = MOS_BITFIELD_VALUE((uint32_t)-1, 19);
        pCscState->DW9.OffsetIn1   = MOS_BITFIELD_VALUE((uint32_t)-64, 16);
        pCscState->DW9.OffsetOut1  = 0;
        pCscState->DW10.OffsetIn2  = MOS_BITFIELD_VALUE((uint32_t)-512, 16);
        pCscState->DW10.OffsetOut2 = 0;
        pCscState->DW11.OffsetIn3  = MOS_BITFIELD_VALUE((uint32_t)-512, 16);
        pCscState->DW11.OffsetOut3 = 0;
    }
}

//!
//! \brief    Helper function for setting up CSC coefficients for BT709 color space
//! \details  Configures CSC transformation coefficients for BT709 color space.
//!           Supports two modes: Color Balance (YUV to RGB with 16-bit precision)
//!           and Gamut Expansion (YUV to RGB with 10-bit precision).
//! \param    [in,out] pCscState
//!           Pointer to CSC state structure to be configured
//! \param    [in] isColorBalance
//!           Mode selector: true for Color Balance mode, false for Gamut Expansion mode
//! \return   void
//!
template <typename CSCStateType>
inline void SetupCSCCoefficients_BT709(CSCStateType* pCscState, bool isColorBalance)
{
    pCscState->DW0.TransformEnable = true;
    
    if (isColorBalance)
    {
        // Color Balance mode: 16-bit precision coefficients
        pCscState->DW0.C0          = 76309;
        pCscState->DW1.C1          = 0;
        pCscState->DW2.C2          = 117489;
        pCscState->DW3.C3          = 76309;
        pCscState->DW4.C4          = MOS_BITFIELD_VALUE((uint32_t)-13975, 19);
        pCscState->DW5.C5          = MOS_BITFIELD_VALUE((uint32_t)-34925, 19);
        pCscState->DW6.C6          = 76309;
        pCscState->DW7.C7          = 138438;
        pCscState->DW8.C8          = 0;
        pCscState->DW9.OffsetIn1   = MOS_BITFIELD_VALUE((uint32_t)-2048, 16);
        pCscState->DW9.OffsetOut1  = 0;
        pCscState->DW10.OffsetIn2  = MOS_BITFIELD_VALUE((uint32_t)-16384, 16);
        pCscState->DW10.OffsetOut2 = 0;
        pCscState->DW11.OffsetIn3  = MOS_BITFIELD_VALUE((uint32_t)-16384, 16);
        pCscState->DW11.OffsetOut3 = 0;
    }
    else
    {
        // Gamut Expansion mode: 10-bit precision coefficients
        pCscState->DW0.C0          = 1192;
        pCscState->DW1.C1          = MOS_BITFIELD_VALUE((uint32_t)-1, 19);
        pCscState->DW2.C2          = 1835;
        pCscState->DW3.C3          = 1192;
        pCscState->DW4.C4          = MOS_BITFIELD_VALUE((uint32_t)-218, 19);
        pCscState->DW5.C5          = MOS_BITFIELD_VALUE((uint32_t)-537, 19);
        pCscState->DW6.C6          = 1192;
        pCscState->DW7.C7          = 2164;
        pCscState->DW8.C8          = 1;
        pCscState->DW9.OffsetIn1   = MOS_BITFIELD_VALUE((uint32_t)-64, 16);
        pCscState->DW9.OffsetOut1  = 0;
        pCscState->DW10.OffsetIn2  = MOS_BITFIELD_VALUE((uint32_t)-512, 16);
        pCscState->DW10.OffsetOut2 = 0;
        pCscState->DW11.OffsetIn3  = MOS_BITFIELD_VALUE((uint32_t)-512, 16);
        pCscState->DW11.OffsetOut3 = 0;
    }
}

//!
//! \brief    Helper function for populating GE_Values array with identity mapping
//! \details  Fills the GE_Values array with identity transformation (257*i for all channels).
//!           This is used when no gamma correction is needed (both input and output gamma are 1.0).
//! \param    [out] usGE_Values
//!           2D array [256][8] to be populated with identity values
//! \return   void
//!
inline void PopulateGEValues_Identity(uint16_t usGE_Values[256][8])
{
    for (uint32_t i = 0; i < 256; i++)
    {
        usGE_Values[i][0] = 257 * i;
        usGE_Values[i][1] = 257 * i;
        usGE_Values[i][2] = 257 * i;
        usGE_Values[i][3] = 257 * i;
        usGE_Values[i][4] = 257 * i;
        usGE_Values[i][5] = 257 * i;
        usGE_Values[i][6] = 257 * i;
        usGE_Values[i][7] = 257 * i;
    }
}

//!
//! \brief    Helper function for populating GE_Values array with gamma correction
//! \details  Fills the GE_Values array using power function-based gamma correction.
//!           Applies inverse gamma to channels 1-3 and forward gamma to channels 5-7.
//!           If both gamma values are 1.0 (identity), calls PopulateGEValues_Identity instead.
//! \param    [out] usGE_Values
//!           2D array [256][8] to be populated with gamma-corrected values
//! \param    [in] dInverseGamma
//!           Inverse gamma value (typically 1.0, 2.2, or 2.6)
//! \param    [in] dForwardGamma
//!           Forward gamma value (typically 1.0, 2.2, or 2.6)
//! \param    [in] isIdentity
//!           Flag indicating if both gamma values are 1.0 (identity transformation)
//! \return   void
//!
inline void PopulateGEValues_Gamma(uint16_t usGE_Values[256][8], double dInverseGamma, double dForwardGamma, bool isIdentity)
{
    if (isIdentity)
    {
        PopulateGEValues_Identity(usGE_Values);
        return;
    }
    
    // Note: Loop only goes to 254, not 255
    for (uint32_t i = 0; i < 255; i++)
    {
        usGE_Values[i][0] = 256 * i;
        usGE_Values[i][1] = (uint16_t)MOS_F_ROUND(pow((double)((double)i / 256), dInverseGamma) * 65536);
        usGE_Values[i][2] = usGE_Values[i][1];
        usGE_Values[i][3] = usGE_Values[i][1];
        
        usGE_Values[i][4] = 256 * i;
        usGE_Values[i][5] = (uint16_t)MOS_F_ROUND(pow((double)((double)i / 256), 1 / dForwardGamma) * 65536);
        usGE_Values[i][6] = usGE_Values[i][5];
        usGE_Values[i][7] = usGE_Values[i][5];
    }
}

//!
//! \brief    Helper function for populating GE_Values array from BT2020 global LUTs
//! \details  Fills the GE_Values array using pre-defined BT2020 inverse and forward
//!           pixel value and gamma LUT arrays. Used for BT2020 color space conversion.
//! \param    [out] usGE_Values
//!           2D array [256][8] to be populated with BT2020 LUT values
//! \return   void
//!
inline void PopulateGEValues_BT2020(uint16_t usGE_Values[256][8])
{
    for (uint32_t i = 0; i < 256; i++)
    {
        usGE_Values[i][0] = (uint16_t)g_Vebox_BT2020_Inverse_Pixel_Value[i];
        usGE_Values[i][1] = (uint16_t)g_Vebox_BT2020_Inverse_Gamma_LUT[i];
        usGE_Values[i][2] = (uint16_t)g_Vebox_BT2020_Inverse_Gamma_LUT[i];
        usGE_Values[i][3] = (uint16_t)g_Vebox_BT2020_Inverse_Gamma_LUT[i];
        
        usGE_Values[i][4] = (uint16_t)g_Vebox_BT2020_Forward_Pixel_Value[i];
        usGE_Values[i][5] = (uint16_t)g_Vebox_BT2020_Forward_Gamma_LUT[i];
        usGE_Values[i][6] = (uint16_t)g_Vebox_BT2020_Forward_Gamma_LUT[i];
        usGE_Values[i][7] = (uint16_t)g_Vebox_BT2020_Forward_Gamma_LUT[i];
    }
}

//!
//! \brief    Helper function for populating GE_Values array from 1DLUT with AYUV processing
//! \details  Fills the GE_Values array using 1DLUT data for AYUV channel processing.
//!           Channels 0-3 use identity mapping, channels 4-7 use 1DLUT values.
//!           Special handling for boundary values (i==0 and i==255).
//! \param    [out] usGE_Values
//!           2D array [256][8] to be populated with 1DLUT AYUV values
//! \param    [in] pForwardGamma
//!           Pointer to forward gamma LUT data (4 values per entry: in_val, Y, U, V)
//! \param    [in] LUTSize
//!           Size of the LUT (typically 256)
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
inline MOS_STATUS PopulateGEValues_1DLUT_AYUV(uint16_t usGE_Values[256][8], const uint16_t* pForwardGamma, uint32_t LUTSize)
{
    MHW_CHK_NULL_RETURN(pForwardGamma);
    
    for (uint32_t i = 0; i < LUTSize; i++)
    {
        // Channels 0-3: Identity mapping
        usGE_Values[i][0] = 257 * i;
        usGE_Values[i][1] = 257 * i;
        usGE_Values[i][2] = 257 * i;
        usGE_Values[i][3] = 257 * i;
        
        // Channels 4-7: 1DLUT AYUV values
        uint32_t nIndex = 4 * i;
        uint16_t in_val = pForwardGamma[nIndex];
        uint16_t vchan1_y = pForwardGamma[nIndex + 1];
        uint16_t vchan2_u = pForwardGamma[nIndex + 2];
        uint16_t vchan3_v = pForwardGamma[nIndex + 3];
        
        // Special handling for boundary values
        usGE_Values[i][4] = (i == 0) ? 0 : ((i == 255) ? 0xffff : in_val);
        usGE_Values[i][5] = vchan1_y;
        usGE_Values[i][6] = vchan2_u;
        usGE_Values[i][7] = vchan3_v;
    }
    
    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Helper function for setting up Gamut Compression
//! \details  Configures gamut compression settings based on compression mode (Basic or Advanced).
//!           For Basic mode, sets up scaling factor if specified. For Advanced mode, sets up
//!           full range mapping parameters (D1Out, DOutDefault, DInDefault, D1In).
//!           Calls SetVertexTableCallback to set up vertex table for the specified color space.
//! \param    [in,out] pGamutState
//!           Pointer to VEBOX_GAMUT_CONTROL_STATE_CMD structure
//! \param    [in] gamutCmd
//!           Reference to VEBOX_GAMUT_CONTROL_STATE_CMD for constant values
//! \param    [in] pVeboxGamutParams
//!           Pointer to gamut parameters
//! \param    [in] SetVertexTableCallback
//!           Callback function to set vertex table for the color space
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
template <typename cmd_t>
inline MOS_STATUS SetGamutCompression(
    typename cmd_t::VEBOX_GAMUT_CONTROL_STATE_CMD* pGamutState,
    const typename cmd_t::VEBOX_GAMUT_CONTROL_STATE_CMD& gamutCmd,
    PMHW_VEBOX_GAMUT_PARAMS pVeboxGamutParams)
{
    MHW_CHK_NULL_RETURN(pGamutState);
    MHW_CHK_NULL_RETURN(pVeboxGamutParams);
    
    if (pVeboxGamutParams->GCompMode == MHW_GAMUT_MODE_BASIC)
    {
        pGamutState->DW15.Fullrangemappingenable = false;
        
        if (pVeboxGamutParams->GCompBasicMode == gamutCmd.GCC_BASICMODESELECTION_SCALINGFACTOR)
        {
            pGamutState->DW17.GccBasicmodeselection = gamutCmd.GCC_BASICMODESELECTION_SCALINGFACTOR;
            pGamutState->DW17.Basicmodescalingfactor = pVeboxGamutParams->iBasicModeScalingFactor;
        }
    }
    else if (pVeboxGamutParams->GCompMode == MHW_GAMUT_MODE_ADVANCED)
    {
        pGamutState->DW15.Fullrangemappingenable = true;
        pGamutState->DW15.D1Out = pVeboxGamutParams->iDout;
        pGamutState->DW15.DOutDefault = pVeboxGamutParams->iDoutDefault;
        pGamutState->DW15.DInDefault = pVeboxGamutParams->iDinDefault;
        pGamutState->DW16.D1In = pVeboxGamutParams->iDin;
    }
    else
    {
        MHW_ASSERTMESSAGE("Invalid GAMUT MODE");
    }
    
    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Helper function for setting up Color Balance
//! \details  Configures color balance by setting up CSC coefficients for YUV to RGB conversion
//!           based on the color space (BT601 or BT709). Sets up gamut state with global mode
//!           enabled and copies the color correction matrix. Populates GE_Values array with
//!           identity mapping and copies to gamma correction structure.
//! \param    [in,out] pIecpState
//!           Pointer to VEBOX_IECP_STATE_CMD structure
//! \param    [in] pVeboxGamutParams
//!           Pointer to gamut parameters
//! \param    [out] usGE_Values
//!           2D array [256][8] for GE values
//! \param    [out] pVeboxGEGammaCorrection
//!           Pointer to gamma correction structure
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
template <typename cmd_t>
inline MOS_STATUS SetColorBalance(
    typename cmd_t::VEBOX_IECP_STATE_CMD* pIecpState,
    PMHW_VEBOX_GAMUT_PARAMS pVeboxGamutParams,
    uint16_t usGE_Values[256][8],
    void* pVeboxGEGammaCorrection)
{
    MHW_CHK_NULL_RETURN(pIecpState);
    MHW_CHK_NULL_RETURN(pVeboxGamutParams);
    MHW_CHK_NULL_RETURN(pVeboxGEGammaCorrection);
    
    // Setup CSC coefficients based on color space (Color Balance mode)
    if (pVeboxGamutParams->ColorSpace == MHW_CSpace_BT601 ||
        pVeboxGamutParams->ColorSpace == MHW_CSpace_xvYCC601 ||
        pVeboxGamutParams->ColorSpace == MHW_CSpace_BT601_FullRange)
    {
        SetupCSCCoefficients_BT601(&pIecpState->CscState, true);
    }
    else if (pVeboxGamutParams->ColorSpace == MHW_CSpace_BT709 ||
             pVeboxGamutParams->ColorSpace == MHW_CSpace_xvYCC709 ||
             pVeboxGamutParams->ColorSpace == MHW_CSpace_BT709_FullRange)
    {
        SetupCSCCoefficients_BT709(&pIecpState->CscState, true);
    }
    else
    {
        MHW_ASSERTMESSAGE("Unknown primary");
    }
    
    // Setup gamut state - properly access from pIecpState
    auto pGamutState = &pIecpState->GamutState;
    MHW_CHK_NULL_RETURN(pGamutState);
    pGamutState->DW0.GlobalModeEnable = true;
    pGamutState->DW1.CmW = 1023;
    
    // Copy color correction matrix
    pGamutState->DW1.C0 = pVeboxGamutParams->Matrix[0][0];
    pGamutState->DW0.C1 = pVeboxGamutParams->Matrix[0][1];
    pGamutState->DW3.C2 = pVeboxGamutParams->Matrix[0][2];
    pGamutState->DW2.C3 = pVeboxGamutParams->Matrix[1][0];
    pGamutState->DW5.C4 = pVeboxGamutParams->Matrix[1][1];
    pGamutState->DW4.C5 = pVeboxGamutParams->Matrix[1][2];
    pGamutState->DW7.C6 = pVeboxGamutParams->Matrix[2][0];
    pGamutState->DW6.C7 = pVeboxGamutParams->Matrix[2][1];
    pGamutState->DW8.C8 = pVeboxGamutParams->Matrix[2][2];
    
    // Set all offsets to 0
    pGamutState->DW9.OffsetInR = 0;
    pGamutState->DW10.OffsetInG = 0;
    pGamutState->DW11.OffsetInB = 0;
    pGamutState->DW12.OffsetOutR = 0;
    pGamutState->DW13.OffsetOutG = 0;
    pGamutState->DW14.OffsetOutB = 0;
    
    // Populate GE_Values with identity mapping
    PopulateGEValues_Identity(usGE_Values);
    
    // Copy to gamma correction structure (1024 DWords = 256 entries * 8 uint16_t / 2)
    MOS_SecureMemcpy(pVeboxGEGammaCorrection, sizeof(uint32_t) * 1024, usGE_Values, sizeof(uint16_t) * 8 * 256);
    
    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Helper function for setting up Gamut Expansion
//! \details  Configures gamut expansion by setting up CSC coefficients for YUV to RGB conversion
//!           based on the color space (BT601 or BT709). Handles both Basic and Advanced gamut
//!           expansion modes. Copies the color correction matrix to gamut state.
//! \param    [in,out] pIecpState
//!           Pointer to VEBOX_IECP_STATE_CMD structure
//! \param    [in] pVeboxGamutParams
//!           Pointer to gamut parameters
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
template <typename cmd_t>
inline MOS_STATUS SetGamutExpansion(
    typename cmd_t::VEBOX_IECP_STATE_CMD* pIecpState,
    PMHW_VEBOX_GAMUT_PARAMS pVeboxGamutParams)
{
    MHW_CHK_NULL_RETURN(pIecpState);
    MHW_CHK_NULL_RETURN(pVeboxGamutParams);
    
    // Setup CSC coefficients based on color space (Gamut Expansion mode)
    if (pVeboxGamutParams->ColorSpace == MHW_CSpace_BT601 ||
        pVeboxGamutParams->ColorSpace == MHW_CSpace_xvYCC601 ||
        pVeboxGamutParams->ColorSpace == MHW_CSpace_BT601_FullRange)
    {
        SetupCSCCoefficients_BT601(&pIecpState->CscState, false);
    }
    else if (pVeboxGamutParams->ColorSpace == MHW_CSpace_BT709 ||
             pVeboxGamutParams->ColorSpace == MHW_CSpace_xvYCC709 ||
             pVeboxGamutParams->ColorSpace == MHW_CSpace_BT709_FullRange)
    {
        SetupCSCCoefficients_BT709(&pIecpState->CscState, false);
    }
    else
    {
        MHW_ASSERTMESSAGE("Unknown primary");
    }
    
    // Setup gamut state based on expansion mode - properly access from pIecpState
    auto pGamutState = &pIecpState->GamutState;
    MHW_CHK_NULL_RETURN(pGamutState);
    if (pVeboxGamutParams->GExpMode == MHW_GAMUT_MODE_BASIC)
    {
        pGamutState->DW0.GlobalModeEnable = true;
        pGamutState->DW1.CmW = 1023;
    }
    else if (pVeboxGamutParams->GExpMode == MHW_GAMUT_MODE_ADVANCED)
    {
        pGamutState->DW0.GlobalModeEnable = false;
    }
    else
    {
        MHW_ASSERTMESSAGE("Invalid GAMUT MODE");
    }
    
    // Copy color correction matrix
    pGamutState->DW1.C0 = pVeboxGamutParams->Matrix[0][0];
    pGamutState->DW0.C1 = pVeboxGamutParams->Matrix[0][1];
    pGamutState->DW3.C2 = pVeboxGamutParams->Matrix[0][2];
    pGamutState->DW2.C3 = pVeboxGamutParams->Matrix[1][0];
    pGamutState->DW5.C4 = pVeboxGamutParams->Matrix[1][1];
    pGamutState->DW4.C5 = pVeboxGamutParams->Matrix[1][2];
    pGamutState->DW7.C6 = pVeboxGamutParams->Matrix[2][0];
    pGamutState->DW6.C7 = pVeboxGamutParams->Matrix[2][1];
    pGamutState->DW8.C8 = pVeboxGamutParams->Matrix[2][2];
    
    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Helper function for setting up Gamma Correction
//! \details  Configures gamma correction by setting up CSC coefficients for YUV to RGB conversion
//!           based on the color space. For BT2020, calls BT2020YUVToRGBCallback. Sets up color
//!           correction matrix (CCM) for BT2020->BT709/BT601 conversion if needed. Populates
//!           GE_Values array using power function-based gamma correction or identity mapping.
//! \param    [in,out] pIecpState
//!           Pointer to VEBOX_IECP_STATE_CMD structure
//! \param    [in] pVeboxGamutParams
//!           Pointer to gamut parameters
//! \param    [out] usGE_Values
//!           2D array [256][8] for GE values
//! \param    [out] pVeboxGEGammaCorrection
//!           Pointer to gamma correction structure
//! \param    [in] pVeboxInterface
//!           Pointer to VEBOX interface
//! \param    [in] pVeboxHeap
//!           Pointer to VEBOX heap
//! \param    [in] pVeboxIecpParams
//!           Pointer to IECP parameters
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
template <typename cmd_t, typename T>
inline MOS_STATUS SetGammaCorrection(
    typename cmd_t::VEBOX_IECP_STATE_CMD* pIecpState,
    PMHW_VEBOX_GAMUT_PARAMS pVeboxGamutParams,
    uint16_t usGE_Values[256][8],
    void* pVeboxGEGammaCorrection,
    T* pVeboxInterface,
    MHW_VEBOX_HEAP* pVeboxHeap,
    PMHW_VEBOX_IECP_PARAMS pVeboxIecpParams)
{
    MHW_CHK_NULL_RETURN(pIecpState);
    MHW_CHK_NULL_RETURN(pVeboxGamutParams);
    MHW_CHK_NULL_RETURN(pVeboxGEGammaCorrection);
    MHW_CHK_NULL_RETURN(pVeboxInterface);
    
    // Setup CSC coefficients based on color space (Color Balance mode for gamma correction)
    if (pVeboxGamutParams->ColorSpace == MHW_CSpace_BT601 ||
        pVeboxGamutParams->ColorSpace == MHW_CSpace_xvYCC601 ||
        pVeboxGamutParams->ColorSpace == MHW_CSpace_BT601_FullRange)
    {
        SetupCSCCoefficients_BT601(&pIecpState->CscState, true);
    }
    else if (pVeboxGamutParams->ColorSpace == MHW_CSpace_BT709 ||
             pVeboxGamutParams->ColorSpace == MHW_CSpace_xvYCC709 ||
             pVeboxGamutParams->ColorSpace == MHW_CSpace_BT709_FullRange)
    {
        SetupCSCCoefficients_BT709(&pIecpState->CscState, true);
    }
    else if (pVeboxGamutParams->ColorSpace == MHW_CSpace_BT2020 ||
             pVeboxGamutParams->ColorSpace == MHW_CSpace_BT2020_FullRange)
    {
        pVeboxInterface->VeboxInterface_BT2020YUVToRGB(pVeboxHeap, pVeboxIecpParams, pVeboxGamutParams);
    }
    else
    {
        MHW_ASSERTMESSAGE("Unknown primary");
    }
    
    // CCM is needed for CSC (BT2020->BT709/BT601 with different gamma)
    bool bEnableCCM = (pVeboxGamutParams->InputGammaValue != pVeboxGamutParams->OutputGammaValue);
    
    // Setup gamut state - properly access from pIecpState
    auto pGamutState = &pIecpState->GamutState;
    MHW_CHK_NULL_RETURN(pGamutState);
    pGamutState->DW0.GlobalModeEnable = true;
    pGamutState->DW1.CmW = 1023;
    
    if ((pVeboxGamutParams->ColorSpace == MHW_CSpace_BT2020) && bEnableCCM)
    {
        if (pVeboxGamutParams->dstColorSpace == MHW_CSpace_BT709)
        {
            // BT2020->BT709 CCM matrix
            pGamutState->DW1.C0 = 108190;
            pGamutState->DW0.C1 = MOS_BITFIELD_VALUE((uint32_t)-38288, 21);
            pGamutState->DW3.C2 = MOS_BITFIELD_VALUE((uint32_t)-4747, 21);
            pGamutState->DW2.C3 = MOS_BITFIELD_VALUE((uint32_t)-7967, 21);
            pGamutState->DW5.C4 = 74174;
            pGamutState->DW4.C5 = MOS_BITFIELD_VALUE((uint32_t)-557, 21);
            pGamutState->DW7.C6 = MOS_BITFIELD_VALUE((uint32_t)-1198, 21);
            pGamutState->DW6.C7 = MOS_BITFIELD_VALUE((uint32_t)-6587, 21);
            pGamutState->DW8.C8 = 73321;
        }
        else
        {
            // BT2020->BT601 CCM matrix
            pGamutState->DW1.C0 = 116420;
            pGamutState->DW0.C1 = MOS_BITFIELD_VALUE((uint32_t)-45094, 21);
            pGamutState->DW3.C2 = MOS_BITFIELD_VALUE((uint32_t)-5785, 21);
            pGamutState->DW2.C3 = MOS_BITFIELD_VALUE((uint32_t)-10586, 21);
            pGamutState->DW5.C4 = 77814;
            pGamutState->DW4.C5 = MOS_BITFIELD_VALUE((uint32_t)-1705, 21);
            pGamutState->DW7.C6 = MOS_BITFIELD_VALUE((uint32_t)-1036, 21);
            pGamutState->DW6.C7 = MOS_BITFIELD_VALUE((uint32_t)-6284, 21);
            pGamutState->DW8.C8 = 72864;
        }
    }
    else
    {
        // Identity matrix
        pGamutState->DW1.C0 = 65536;
        pGamutState->DW0.C1 = 0;
        pGamutState->DW3.C2 = 0;
        pGamutState->DW2.C3 = 0;
        pGamutState->DW5.C4 = 65536;
        pGamutState->DW4.C5 = 0;
        pGamutState->DW7.C6 = 0;
        pGamutState->DW6.C7 = 0;
        pGamutState->DW8.C8 = 65536;
        pGamutState->DW9.OffsetInR = 0;
        pGamutState->DW10.OffsetInG = 0;
        pGamutState->DW11.OffsetInB = 0;
        pGamutState->DW12.OffsetOutR = 0;
        pGamutState->DW13.OffsetOutG = 0;
        pGamutState->DW14.OffsetOutB = 0;
    }
    
    // Map gamma values
    double dInverseGamma = 1.0;
    if (pVeboxGamutParams->InputGammaValue == MHW_GAMMA_1P0)
    {
        dInverseGamma = 1.0;
    }
    else if (pVeboxGamutParams->InputGammaValue == MHW_GAMMA_2P2)
    {
        dInverseGamma = 2.2;
    }
    else if (pVeboxGamutParams->InputGammaValue == MHW_GAMMA_2P6)
    {
        dInverseGamma = 2.6;
    }
    else
    {
        MHW_ASSERTMESSAGE("Invalid InputGammaValue");
    }
    
    double dForwardGamma = 1.0;
    if (pVeboxGamutParams->OutputGammaValue == MHW_GAMMA_1P0)
    {
        dForwardGamma = 1.0;
    }
    else if (pVeboxGamutParams->OutputGammaValue == MHW_GAMMA_2P2)
    {
        dForwardGamma = 2.2;
    }
    else if (pVeboxGamutParams->OutputGammaValue == MHW_GAMMA_2P6)
    {
        dForwardGamma = 2.6;
    }
    else
    {
        MHW_ASSERTMESSAGE("Invalid OutputGammaValue");
    }
    
    // Populate GE_Values with gamma correction
    bool isIdentity = (pVeboxGamutParams->InputGammaValue == MHW_GAMMA_1P0) && 
                      (pVeboxGamutParams->OutputGammaValue == MHW_GAMMA_1P0);
    PopulateGEValues_Gamma(usGE_Values, dInverseGamma, dForwardGamma, isIdentity);
    
    // Copy to gamma correction structure
    if (isIdentity)
    {
        MOS_SecureMemcpy(pVeboxGEGammaCorrection, sizeof(uint32_t) * 1024, usGE_Values, sizeof(uint16_t) * 8 * 256);
    }
    else
    {
        MOS_SecureMemcpy(pVeboxGEGammaCorrection, sizeof(uint32_t) * 1020, usGE_Values, sizeof(uint16_t) * 8 * 255);
    }
    
    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Helper function for setting up BT2020 CSC
//! \details  Configures BT2020 color space conversion with global tone mapping LUTs.
//!           If 1DLUT is active, sets up CCM from 1DLUT parameters and returns early.
//!           Otherwise, sets up BT2020->BT709/BT601 CCM matrix, populates GE_Values
//!           from BT2020 global LUTs, and calls BT2020YUVToRGBCallback.
//! \param    [in,out] pIecpState
//!           Pointer to VEBOX_IECP_STATE_CMD structure
//! \param    [in] pVeboxIecpParams
//!           Pointer to IECP parameters
//! \param    [in] pVeboxGamutParams
//!           Pointer to gamut parameters
//! \param    [out] usGE_Values
//!           2D array [256][8] for GE values
//! \param    [out] pVeboxGEGammaCorrection
//!           Pointer to gamma correction structure
//! \param    [in] pVeboxInterface
//!           Pointer to VEBOX interface
//! \param    [in] pVeboxHeap
//!           Pointer to VEBOX heap
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
template <typename cmd_t, typename T>
inline MOS_STATUS SetBT2020CSC(
    typename cmd_t::VEBOX_IECP_STATE_CMD* pIecpState,
    PMHW_VEBOX_IECP_PARAMS pVeboxIecpParams,
    PMHW_VEBOX_GAMUT_PARAMS pVeboxGamutParams,
    uint16_t usGE_Values[256][8],
    void* pVeboxGEGammaCorrection,
    T* pVeboxInterface,
    MHW_VEBOX_HEAP* pVeboxHeap)
{
    MHW_CHK_NULL_RETURN(pIecpState);
    MHW_CHK_NULL_RETURN(pVeboxIecpParams);
    MHW_CHK_NULL_RETURN(pVeboxGamutParams);
    MHW_CHK_NULL_RETURN(pVeboxGEGammaCorrection);
    MHW_CHK_NULL_RETURN(pVeboxInterface);
    
    // Check if 1DLUT is active
    if (pVeboxIecpParams->s1DLutParams.bActive)
    {
        // CCM setting if 1DLUT VEBOX HDR enabled
        auto p1DLutParams = &pVeboxIecpParams->s1DLutParams;
        
        pIecpState->CcmState.DW1.C0 = p1DLutParams->pCCM[0];
        pIecpState->CcmState.DW0.C1 = MOS_BITFIELD_VALUE((uint32_t)p1DLutParams->pCCM[1], 27);
        pIecpState->CcmState.DW3.C2 = MOS_BITFIELD_VALUE((uint32_t)p1DLutParams->pCCM[2], 27);
        pIecpState->CcmState.DW2.C3 = MOS_BITFIELD_VALUE((uint32_t)p1DLutParams->pCCM[3], 27);
        pIecpState->CcmState.DW5.C4 = p1DLutParams->pCCM[4];
        pIecpState->CcmState.DW4.C5 = MOS_BITFIELD_VALUE((uint32_t)p1DLutParams->pCCM[5], 27);
        pIecpState->CcmState.DW7.C6 = MOS_BITFIELD_VALUE((uint32_t)p1DLutParams->pCCM[6], 27);
        pIecpState->CcmState.DW6.C7 = MOS_BITFIELD_VALUE((uint32_t)p1DLutParams->pCCM[7], 27);
        pIecpState->CcmState.DW8.C8 = p1DLutParams->pCCM[8];
        pIecpState->CcmState.DW9.OffsetInR = p1DLutParams->pCCM[9];
        pIecpState->CcmState.DW10.OffsetInG = p1DLutParams->pCCM[10];
        pIecpState->CcmState.DW11.OffsetInB = p1DLutParams->pCCM[11];
        pIecpState->CcmState.DW12.OffsetOutR = p1DLutParams->pCCM[12];
        pIecpState->CcmState.DW13.OffsetOutG = p1DLutParams->pCCM[13];
        pIecpState->CcmState.DW14.OffsetOutB = p1DLutParams->pCCM[14];
        
        auto pGamutState = &pIecpState->GamutState;
        pGamutState->DW0.GlobalModeEnable = false;
        
        // Still need to set CSC params here
        pVeboxInterface->VeboxInterface_BT2020YUVToRGB(pVeboxHeap, pVeboxIecpParams, pVeboxGamutParams);
        
        return MOS_STATUS_SUCCESS;
    }
    
    // Setup gamut state for BT2020 CSC
    auto pGamutState = &pIecpState->GamutState;
    pGamutState->DW0.GlobalModeEnable = true;
    pGamutState->DW1.CmW = 1023;  // Colorimetric accurate image
    
    if (pVeboxGamutParams->dstColorSpace == MHW_CSpace_BT601)
    {
        // BT2020->BT601 CCM matrix
        pGamutState->DW1.C0 = 116420;
        pGamutState->DW0.C1 = MOS_BITFIELD_VALUE((uint32_t)-45094, 21);
        pGamutState->DW3.C2 = MOS_BITFIELD_VALUE((uint32_t)-5785, 21);
        pGamutState->DW2.C3 = MOS_BITFIELD_VALUE((uint32_t)-10586, 21);
        pGamutState->DW5.C4 = 77814;
        pGamutState->DW4.C5 = MOS_BITFIELD_VALUE((uint32_t)-1705, 21);
        pGamutState->DW7.C6 = MOS_BITFIELD_VALUE((uint32_t)-1036, 21);
        pGamutState->DW6.C7 = MOS_BITFIELD_VALUE((uint32_t)-6284, 21);
        pGamutState->DW8.C8 = 72864;
    }
    else  // BT709, sRGB has same chromaticity CIE 1931
    {
        // BT2020->BT709 CCM matrix
        pGamutState->DW1.C0 = 108190;
        pGamutState->DW0.C1 = MOS_BITFIELD_VALUE((uint32_t)-38288, 21);
        pGamutState->DW3.C2 = MOS_BITFIELD_VALUE((uint32_t)-4747, 21);
        pGamutState->DW2.C3 = MOS_BITFIELD_VALUE((uint32_t)-7967, 21);
        pGamutState->DW5.C4 = 74174;
        pGamutState->DW4.C5 = MOS_BITFIELD_VALUE((uint32_t)-557, 21);
        pGamutState->DW7.C6 = MOS_BITFIELD_VALUE((uint32_t)-1198, 21);
        pGamutState->DW6.C7 = MOS_BITFIELD_VALUE((uint32_t)-6587, 21);
        pGamutState->DW8.C8 = 73321;
    }
    
    // Populate GE_Values from BT2020 global LUTs
    PopulateGEValues_BT2020(usGE_Values);
    
    // Copy to gamma correction structure (1024 DWords)
    MOS_SecureMemcpy(pVeboxGEGammaCorrection, sizeof(uint32_t) * 1024, usGE_Values, sizeof(uint16_t) * 8 * 256);
    
    // Back end CSC setting, need to convert BT2020 YUV input to RGB before GE
    pVeboxInterface->VeboxInterface_BT2020YUVToRGB(pVeboxHeap, pVeboxIecpParams, pVeboxGamutParams);
    
    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Helper function for setting up 1DLUT Processing
//! \details  Configures 1DLUT processing with AYUV channel processing for platforms that support it.
//!           For platforms that don't support full processing, only
//!           logs a message. For other platforms, sets up gamut state and populates GE_Values
//!           from 1DLUT data with identity matrix.
//! \param    [in,out] pGamutState
//!           Pointer to VEBOX_GAMUT_CONTROL_STATE_CMD structure
//! \param    [in] pVeboxIecpParams
//!           Pointer to IECP parameters
//! \param    [out] usGE_Values
//!           2D array [256][8] for GE values
//! \param    [out] pVeboxGEGammaCorrection
//!           Pointer to gamma correction structure
//! \param    [in] enableFullProcessing
//!           Flag to enable full AYUV processing
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
template <typename cmd_t>
inline MOS_STATUS Set1DLUTProcessing(
    typename cmd_t::VEBOX_GAMUT_CONTROL_STATE_CMD* pGamutState,
    PMHW_VEBOX_IECP_PARAMS pVeboxIecpParams,
    uint16_t usGE_Values[256][8],
    void* pVeboxGEGammaCorrection,
    bool enableFullProcessing)
{
    MHW_CHK_NULL_RETURN(pVeboxIecpParams);
    
    if (!enableFullProcessing)
    {
        // Platforms that don't support full processing only log for 1DLUT
        MHW_NORMALMESSAGE("Use VEBOX_SHAPER_1K_LOOKUP_STATE for 1DLUT (4x 1DLUT but Gamut State only has 1DLUT)!");
        return MOS_STATUS_SUCCESS;
    }
    
    // Full AYUV processing for other platforms
    MHW_CHK_NULL_RETURN(pGamutState);
    MHW_CHK_NULL_RETURN(pVeboxIecpParams);
    MHW_CHK_NULL_RETURN(pVeboxGEGammaCorrection);
    
    // Gamut Expansion setting
    pGamutState->DW0.GlobalModeEnable = true;
    pGamutState->DW1.CmW = 1023;
    
    // Populate GE_Values from 1DLUT AYUV data
    MHW_CHK_STATUS_RETURN(PopulateGEValues_1DLUT_AYUV(
        usGE_Values,
        (uint16_t*)pVeboxIecpParams->s1DLutParams.p1DLUT,
        pVeboxIecpParams->s1DLutParams.LUTSize));
    
    // Set identity matrix
    pGamutState->DW1.C0 = 65536;
    pGamutState->DW0.C1 = 0;
    pGamutState->DW3.C2 = 0;
    pGamutState->DW2.C3 = 0;
    pGamutState->DW5.C4 = 65536;
    pGamutState->DW4.C5 = 0;
    pGamutState->DW7.C6 = 0;
    pGamutState->DW6.C7 = 0;
    pGamutState->DW8.C8 = 65536;
    pGamutState->DW9.OffsetInR = 0;
    pGamutState->DW10.OffsetInG = 0;
    pGamutState->DW11.OffsetInB = 0;
    pGamutState->DW12.OffsetOutR = 0;
    pGamutState->DW13.OffsetOutG = 0;
    pGamutState->DW14.OffsetOutB = 0;
    
    // Copy to gamma correction structure (1024 DWords)
    MOS_SecureMemcpy(pVeboxGEGammaCorrection, sizeof(uint32_t) * 1024, usGE_Values, sizeof(uint16_t) * 8 * 256);
    
    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Helper function for programming CCM as identity matrix
//! \details  Sets the Color Correction Matrix to identity (diagonal 0x400000, off-diagonal 0)
//!           with all offsets zeroed. Used when no color correction is needed.
//! \param    [in,out] pIecpState
//!           Pointer to IECP state structure containing CcmState
//! \param    [in] ccmEnable
//!           Value to set for ColorCorrectionMatrixEnable flag
//! \return   void
//!
template <typename IecpState_t>
inline void SetCcmIdentityMatrix(IecpState_t* pIecpState, bool ccmEnable)
{
    pIecpState->CcmState.DW0.ColorCorrectionMatrixEnable = ccmEnable;
    pIecpState->CcmState.DW1.C0                          = 0x400000;
    pIecpState->CcmState.DW0.C1                          = 0;
    pIecpState->CcmState.DW3.C2                          = 0;
    pIecpState->CcmState.DW2.C3                          = 0;
    pIecpState->CcmState.DW5.C4                          = 0x400000;
    pIecpState->CcmState.DW4.C5                          = 0;
    pIecpState->CcmState.DW7.C6                          = 0;
    pIecpState->CcmState.DW6.C7                          = 0;
    pIecpState->CcmState.DW8.C8                          = 0x400000;
    pIecpState->CcmState.DW9.OffsetInR                   = 0;
    pIecpState->CcmState.DW10.OffsetInG                  = 0;
    pIecpState->CcmState.DW11.OffsetInB                  = 0;
    pIecpState->CcmState.DW12.OffsetOutR                 = 0;
    pIecpState->CcmState.DW13.OffsetOutG                 = 0;
    pIecpState->CcmState.DW14.OffsetOutB                 = 0;
}

//!
//! \brief    Helper function for programming CCM based on color space
//! \details  Programs the Color Correction Matrix coefficients for BT709 or BT2020
//!           color spaces. For other color spaces, optionally programs identity matrix
//!           and always asserts with an error message.
//! \param    [in,out] pIecpState
//!           Pointer to IECP state structure containing CcmState
//! \param    [in] colorSpace
//!           Input color space (MHW_CSpace_BT709, MHW_CSpace_BT709_FullRange,
//!           MHW_CSpace_BT2020, MHW_CSpace_BT2020_FullRange, or other)
//! \param    [in] programFallbackIdentity
//!           If true, programs identity matrix for unsupported color spaces before asserting.
//!           If false, only asserts without programming
//! \return   void
//!
template <typename IecpState_t>
inline void SetCcmColorSpace(IecpState_t* pIecpState, MHW_CSPACE colorSpace, bool programFallbackIdentity = true)
{
    // Always disable CCM enable for color space programming
    pIecpState->CcmState.DW0.ColorCorrectionMatrixEnable = false;

    if ((colorSpace == MHW_CSpace_BT709) || (colorSpace == MHW_CSpace_BT709_FullRange))
    {
        // BT709 CCM coefficients
        pIecpState->CcmState.DW1.C0          = 0x00009937;
        pIecpState->CcmState.DW0.C1          = 0x000115f6;
        pIecpState->CcmState.DW3.C2          = 0;
        pIecpState->CcmState.DW2.C3          = 0x00009937;
        pIecpState->CcmState.DW5.C4          = 0x07ffe3f1;
        pIecpState->CcmState.DW4.C5          = 0x07ffb9e0;
        pIecpState->CcmState.DW7.C6          = 0x00009937;
        pIecpState->CcmState.DW6.C7          = 0;
        pIecpState->CcmState.DW8.C8          = 0x0000ebe6;
        // Limited range BT709 has non-zero input offsets; full range has zero offsets
        pIecpState->CcmState.DW9.OffsetInR   = (colorSpace == MHW_CSpace_BT709) ? 0xf8000000 : 0;
        pIecpState->CcmState.DW10.OffsetInG  = (colorSpace == MHW_CSpace_BT709) ? 0xc0000000 : 0;
        pIecpState->CcmState.DW11.OffsetInB  = (colorSpace == MHW_CSpace_BT709) ? 0xc0000000 : 0;
        pIecpState->CcmState.DW12.OffsetOutR = 0;
        pIecpState->CcmState.DW13.OffsetOutG = 0;
        pIecpState->CcmState.DW14.OffsetOutB = 0;
    }
    else if ((colorSpace == MHW_CSpace_BT2020) || (colorSpace == MHW_CSpace_BT2020_FullRange))
    {
        // BT2020 CCM coefficients
        pIecpState->CcmState.DW1.C0          = 0x00009937;
        pIecpState->CcmState.DW0.C1          = 0x000119d4;
        pIecpState->CcmState.DW3.C2          = 0;
        pIecpState->CcmState.DW2.C3          = 0x00009937;
        pIecpState->CcmState.DW5.C4          = 0x07ffe75a;
        pIecpState->CcmState.DW4.C5          = 0x07ffaa6a;
        pIecpState->CcmState.DW7.C6          = 0x00009937;
        pIecpState->CcmState.DW6.C7          = 0;
        pIecpState->CcmState.DW8.C8          = 0x0000dce4;
        // Limited range BT2020 has non-zero input offsets; full range has zero offsets
        pIecpState->CcmState.DW9.OffsetInR   = (colorSpace == MHW_CSpace_BT2020) ? 0xf8000000 : 0;
        pIecpState->CcmState.DW10.OffsetInG  = (colorSpace == MHW_CSpace_BT2020) ? 0xc0000000 : 0;
        pIecpState->CcmState.DW11.OffsetInB  = (colorSpace == MHW_CSpace_BT2020) ? 0xc0000000 : 0;
        pIecpState->CcmState.DW12.OffsetOutR = 0;
        pIecpState->CcmState.DW13.OffsetOutG = 0;
        pIecpState->CcmState.DW14.OffsetOutB = 0;
    }
    else
    {
        // Unsupported color space: optionally program identity matrix, then assert
        if (programFallbackIdentity)
        {
            SetCcmIdentityMatrix(pIecpState, false);
        }
        MHW_ASSERTMESSAGE("Unsupported Input Color Space!");
    }
}

//!
//! \brief    Helper function for programming identity forward gamma LUT
//! \details  Programs the forward gamma LUT with identity mapping (256*i for each entry).
//!           Handles two special-entry modes: dual (entries 254 and 255) or single (entry 255 only).
//! \param    [in,out] pForwardGamma
//!           Pointer to array of forward gamma correction state entries
//! \param    [in] loopBound
//!           Number of entries to fill with identity (254 for dual-special-entry mode,
//!           255 for single-special-entry mode)
//! \param    [in] dualSpecialEntries
//!           If true, entries 254 and 255 are both special.
//!           If false, only entry 255 is special.
//! \return   void
//!
template <typename FwdGamma_t>
inline void SetForwardGammaIdentity(FwdGamma_t* pForwardGamma, uint32_t loopBound, bool dualSpecialEntries)
{
    // Fill identity entries from 0 to loopBound (exclusive)
    for (uint32_t i = 0; i < loopBound; i++)
    {
        pForwardGamma[i].DW0.PointValueForForwardGammaLut        = 256 * i;
        pForwardGamma[i].DW1.ForwardRChannelGammaCorrectionValue = 256 * i;
        pForwardGamma[i].DW2.ForwardGChannelGammaCorrectionValue = 256 * i;
        pForwardGamma[i].DW3.ForwardBChannelGammaCorrectionValue = 256 * i;
    }

    if (dualSpecialEntries)
    {
        // Dual special entries: entry 254 is all 0xffff, entry 255 point is 0xffffffff
        pForwardGamma[254].DW0.PointValueForForwardGammaLut        = 0xffff;
        pForwardGamma[254].DW1.ForwardRChannelGammaCorrectionValue = 0xffff;
        pForwardGamma[254].DW2.ForwardGChannelGammaCorrectionValue = 0xffff;
        pForwardGamma[254].DW3.ForwardBChannelGammaCorrectionValue = 0xffff;

        pForwardGamma[255].DW0.PointValueForForwardGammaLut        = 0xffffffff;
    }
    else
    {
        // Single special entry: entry 255 point is 0xffff
        pForwardGamma[255].DW0.PointValueForForwardGammaLut        = 0xffff;
    }

    // Entry 255 channel values are always 0xffff
    pForwardGamma[255].DW1.ForwardRChannelGammaCorrectionValue = 0xffff;
    pForwardGamma[255].DW2.ForwardGChannelGammaCorrectionValue = 0xffff;
    pForwardGamma[255].DW3.ForwardBChannelGammaCorrectionValue = 0xffff;
}

//!
//! \brief    Helper function for programming inverse gamma from API-provided 1D LUT
//! \details  Programs the inverse gamma LUT entries from the API-provided 1D LUT data
//!           for the active range, and zeros out the remaining entries up to fullSize.
//! \param    [in,out] pInverseGamma
//!           Pointer to array of inverse gamma correction state entries
//! \param    [in] p1DLut
//!           Pointer to 1D LUT data (4 uint16_t values per entry: index, R, G, B)
//! \param    [in] lutSize
//!           Number of active LUT entries to program from p1DLut
//! \param    [in] fullSize
//!           Total number of inverse gamma entries in hardware (4096 for older platforms,
//!           1024 for newer platforms)
//! \return   void
//!
template <typename InvGamma_t>
inline void SetInverseGammaFromLut(InvGamma_t* pInverseGamma, uint16_t* p1DLut, uint32_t lutSize, uint32_t fullSize)
{
    // Program active LUT entries from API-provided data
    for (uint32_t i = 0; i < lutSize; i++)
    {
        pInverseGamma[i].DW0.Value                               = 0;
        pInverseGamma[i].DW1.InverseRChannelGammaCorrectionValue = (uint32_t)(p1DLut[4 * i + 1]);  // 32-bit precision
        pInverseGamma[i].DW2.InverseGChannelGammaCorrectionValue = (uint32_t)(p1DLut[4 * i + 2]);  // 32-bit precision
        pInverseGamma[i].DW3.InverseBChannelGammaCorrectionValue = (uint32_t)(p1DLut[4 * i + 3]);  // 32-bit precision
    }

    // Zero out remaining entries beyond the active LUT range
    for (uint32_t i = lutSize; i < fullSize; i++)
    {
        pInverseGamma[i].DW0.Value                               = 0;
        pInverseGamma[i].DW1.InverseRChannelGammaCorrectionValue = 0;
        pInverseGamma[i].DW2.InverseGChannelGammaCorrectionValue = 0;
        pInverseGamma[i].DW3.InverseBChannelGammaCorrectionValue = 0;
    }
}

//!
//! \brief    Helper function for programming identity inverse gamma LUT
//! \details  Programs the inverse gamma LUT with identity mapping using linear interpolation
//!           from 0 to maxValLutOut. The last entry is always set to maxValLutOut exactly.
//! \param    [in,out] pInverseGamma
//!           Pointer to array of inverse gamma correction state entries
//! \param    [in] nLutInBitDepth
//!           Bit depth of the LUT input (e.g., 12 for 4096-entry LUT, 10 for 1024-entry LUT)
//! \param    [in] totalEntries
//!           Total number of entries to program (e.g., 4096 or 1024)
//! \return   void
//!
template <typename InvGamma_t>
inline void SetInverseGammaIdentity(InvGamma_t* pInverseGamma, uint32_t nLutInBitDepth, uint32_t totalEntries)
{
    uint64_t maxValLutIn  = (((uint64_t)1) << nLutInBitDepth) - 1;
    uint64_t maxValLutOut = (((uint64_t)1) << 32) - 1;
    uint32_t lastEntry    = totalEntries - 1;

    for (uint32_t i = 0; i < totalEntries; i++)
    {
        float    x               = (float)i / (float)maxValLutIn;
        uint32_t nCorrectedValue = (i < lastEntry) ? (uint32_t)(x * (float)maxValLutOut + 0.5f) : (uint32_t)maxValLutOut;

        pInverseGamma[i].DW0.Value                               = 0;
        pInverseGamma[i].DW1.InverseRChannelGammaCorrectionValue = nCorrectedValue;
        pInverseGamma[i].DW2.InverseGChannelGammaCorrectionValue = nCorrectedValue;
        pInverseGamma[i].DW3.InverseBChannelGammaCorrectionValue = nCorrectedValue;
    }
}

}  // namespace common
}  // namespace vebox
}  // namespace mhw

#endif  // __MHW_VEBOX_COMMON_IMPL_H__