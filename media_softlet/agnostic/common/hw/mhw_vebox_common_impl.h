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

}  // namespace common
}  // namespace vebox
}  // namespace mhw

#endif  // __MHW_VEBOX_COMMON_IMPL_H__