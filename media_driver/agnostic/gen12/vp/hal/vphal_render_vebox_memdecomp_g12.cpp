/*
* Copyright (c) 2019, Intel Corporation
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
//! \file     vphal_render_vebox_memdecomp_g12.cpp
//! \brief    Defines data structures and interfaces for media memory decompression.
//! \details  
//!

#include "vphal_render_vebox_memdecomp_g12.h"
#include "mhw_vebox_hwcmd_g12_X.h"
#include "mhw_vebox_g12_X.h"

MediaVeboxDecompStateG12::MediaVeboxDecompStateG12() :
    MediaVeboxDecompState()
{
}

MOS_STATUS MediaVeboxDecompStateG12::RenderDecompCMD(PMOS_SURFACE surface)
{
    MOS_STATUS                          eStatus = MOS_STATUS_SUCCESS;
    MHW_VEBOX_STATE_CMD_PARAMS          veboxStateCmdParams;
    MOS_COMMAND_BUFFER                  cmdBuffer;
    PMHW_VEBOX_INTERFACE                veboxInterface;
    MHW_VEBOX_SURFACE_STATE_CMD_PARAMS  mhwVeboxSurfaceStateCmdParams;
    MHW_MI_FLUSH_DW_PARAMS              flushDwParams;
    uint32_t                            streamID = 0;
    const MHW_VEBOX_HEAP                *veboxHeap = nullptr;

    VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(surface);

    if (surface->CompressionMode                &&
        surface->CompressionMode != MOS_MMC_MC  &&
        surface->CompressionMode != MOS_MMC_RC)
    {
        VPHAL_MEMORY_DECOMP_NORMALMESSAGE("Input surface is uncompressed, In_Place resolve is not needed");
        return eStatus;
    }

    if (!IsDecompressionFormatSupported(surface))
    {
        VPHAL_MEMORY_DECOMP_NORMALMESSAGE("Input surface is not supported by Vebox, In_Place resolve can't be done");
        return eStatus;
    }

    veboxInterface = m_veboxInterface;

    m_osInterface->pfnSetGpuContext(m_osInterface, MOS_GPU_CONTEXT_VEBOX);

    // Reset allocation list and house keeping
    m_osInterface->pfnResetOsStates(m_osInterface);

    VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(veboxInterface->GetVeboxHeapInfo(&veboxHeap));
    VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(&surface->OsResource);
    VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(m_osInterface->osCpInterface);

    // Check whether surface is ready for write
    m_osInterface->pfnSyncOnResource(
        m_osInterface,
        &surface->OsResource,
        MOS_GPU_CONTEXT_VEBOX,
        true);

    // preprocess in cp first
    m_osInterface->osCpInterface->PrepareResources((void **)&surface, 1, nullptr, 0);

    // initialize the command buffer struct
    MOS_ZeroMemory(&cmdBuffer, sizeof(MOS_COMMAND_BUFFER));

    VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));
    VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(InitCommandBuffer(&cmdBuffer));

    // Prepare Vebox_Surface_State, surface input/and output are the same but the compressed status.
    VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(SetupVeboxSurfaceState(&mhwVeboxSurfaceStateCmdParams, surface, nullptr));

    //---------------------------------
    // Send Pvt MMCD CMD
    //---------------------------------
    MhwVeboxInterfaceG12 *pVeboxInterfaceExt12;
    pVeboxInterfaceExt12 = (MhwVeboxInterfaceG12 *)veboxInterface;

    VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(pVeboxInterfaceExt12->setVeboxPrologCmd(
        m_mhwMiInterface,
        &cmdBuffer));

    //---------------------------------
    // Send CMD: Vebox_Surface_State
    //---------------------------------
    VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(veboxInterface->AddVeboxSurfaces(
        &cmdBuffer,
        &mhwVeboxSurfaceStateCmdParams));

    //---------------------------------
    // Send CMD: Vebox_Tiling_Convert
    //---------------------------------
    VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(VeboxSendVeboxTileConvertCMD(&cmdBuffer, surface, nullptr, streamID));

    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));

    VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(m_mhwMiInterface->AddMiFlushDwCmd(
        &cmdBuffer,
        &flushDwParams));

    if (!m_osInterface->bEnableKmdMediaFrameTracking && veboxHeap)
    {
        MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
        flushDwParams.pOsResource      = (PMOS_RESOURCE)&veboxHeap->DriverResource;
        flushDwParams.dwResourceOffset = veboxHeap->uiOffsetSync;
        flushDwParams.dwDataDW1        = veboxHeap->dwNextTag;
        VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(m_mhwMiInterface->AddMiFlushDwCmd(
            &cmdBuffer,
            &flushDwParams));
    }

    VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(m_mhwMiInterface->AddMiBatchBufferEnd(
        &cmdBuffer,
        nullptr));

    // Return unused command buffer space to OS
    m_osInterface->pfnReturnCommandBuffer(
        m_osInterface,
        &cmdBuffer,
        0);

    // Flush the command buffer
    VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(
        m_osInterface,
        &cmdBuffer,
        false));

    veboxInterface->UpdateVeboxSync();

    return eStatus;
}

MOS_STATUS MediaVeboxDecompStateG12::IsVeboxDecompressionEnabled()
{
    MOS_USER_FEATURE_VALUE_DATA         UserFeatureData;
    MOS_STATUS                          eStatus = MOS_STATUS_SUCCESS;

    MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
    UserFeatureData.i32DataFlag = MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE;

#ifdef LINUX
    UserFeatureData.bData = false; // disable VE Decompress on Linux
#else
    UserFeatureData.bData = true;
#endif

    MOS_USER_FEATURE_INVALID_KEY_ASSERT(MOS_UserFeature_ReadValue_ID(
        nullptr,
        __VPHAL_ENABLE_VEBOX_MMC_DECOMPRESS_ID,
        &UserFeatureData));

    m_veboxMMCResolveEnabled = UserFeatureData.bData ? true: false;

    return eStatus;
}

MOS_STATUS MediaVeboxDecompStateG12::RenderDoubleBufferDecompCMD(
    PMOS_SURFACE inputSurface, 
    PMOS_SURFACE outputSurface)
{
    MOS_STATUS                          eStatus = MOS_STATUS_SUCCESS;
    MHW_VEBOX_STATE_CMD_PARAMS          veboxStateCmdParams;
    MOS_COMMAND_BUFFER                  cmdBuffer;
    PMHW_VEBOX_INTERFACE                veboxInterface;
    MHW_VEBOX_SURFACE_STATE_CMD_PARAMS  mhwVeboxSurfaceStateCmdParams;
    MHW_MI_FLUSH_DW_PARAMS              flushDwParams;
    uint32_t                            streamID = 0;
    const MHW_VEBOX_HEAP                *veboxHeap = nullptr;

    VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(inputSurface);
    VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(outputSurface);

    veboxInterface = m_veboxInterface;

    m_osInterface->pfnSetGpuContext(m_osInterface, MOS_GPU_CONTEXT_VEBOX);

    // Reset allocation list and house keeping
    m_osInterface->pfnResetOsStates(m_osInterface);

    VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(veboxInterface->GetVeboxHeapInfo(&veboxHeap));
    VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(&inputSurface->OsResource);
    VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(&outputSurface->OsResource);
    VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(m_osInterface->osCpInterface);

    //there is a new usage that input surface is clear and output surface is secure.
    //replace Huc Copy by DoubleBuffer resolve to update ccs data.
    //So need consolidate both input/output surface information to decide cp context.
     PMOS_SURFACE surfaceArray[2];
     surfaceArray[0] = inputSurface;
     surfaceArray[1] = outputSurface;

    // preprocess in cp first
    m_osInterface->osCpInterface->PrepareResources((void **)&surfaceArray, sizeof(surfaceArray) / sizeof(PMOS_SURFACE), nullptr, 0);

    // initialize the command buffer struct
    MOS_ZeroMemory(&cmdBuffer, sizeof(MOS_COMMAND_BUFFER));

    VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));
    VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(InitCommandBuffer(&cmdBuffer));

    // Prepare Vebox_Surface_State, surface input/and output are the same but the compressed status.
    VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(SetupVeboxSurfaceState(&mhwVeboxSurfaceStateCmdParams, inputSurface, outputSurface));

    //---------------------------------
    // Send Pvt MMCD CMD
    //---------------------------------
    MediaVeboxDecompStateG12 *pVeboxInterfaceExt12;
    pVeboxInterfaceExt12 = (MediaVeboxDecompStateG12 *)veboxInterface;

    //---------------------------------
    // Send CMD: Vebox_Surface_State
    //---------------------------------
    VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(veboxInterface->AddVeboxSurfaces(
        &cmdBuffer,
        &mhwVeboxSurfaceStateCmdParams));

    //---------------------------------
    // Send CMD: Vebox_Tiling_Convert
    //---------------------------------
    VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(VeboxSendVeboxTileConvertCMD(&cmdBuffer, inputSurface, outputSurface, streamID));

    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));

    VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(m_mhwMiInterface->AddMiFlushDwCmd(
        &cmdBuffer,
        &flushDwParams));

    if (!m_osInterface->bEnableKmdMediaFrameTracking && veboxHeap)
    {
        MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
        flushDwParams.pOsResource = (PMOS_RESOURCE)&veboxHeap->DriverResource;
        flushDwParams.dwResourceOffset = veboxHeap->uiOffsetSync;
        flushDwParams.dwDataDW1 = veboxHeap->dwNextTag;
        VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(m_mhwMiInterface->AddMiFlushDwCmd(
            &cmdBuffer,
            &flushDwParams));
    }

    VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(m_mhwMiInterface->AddMiBatchBufferEnd(
        &cmdBuffer,
        nullptr));

    // Return unused command buffer space to OS
    m_osInterface->pfnReturnCommandBuffer(
        m_osInterface,
        &cmdBuffer,
        0);

    // Flush the command buffer
    VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(
        m_osInterface,
        &cmdBuffer,
        false));

    veboxInterface->UpdateVeboxSync();

    return eStatus;
}

MOS_STATUS MediaVeboxDecompStateG12::VeboxSendVeboxTileConvertCMD(
    PMOS_COMMAND_BUFFER cmdBuffer,
    PMOS_SURFACE        inputSurface,
    PMOS_SURFACE        outputSurface,
    uint32_t            streamID)
{
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;
    PMOS_SURFACE                    surface = nullptr;
    mhw_vebox_g12_X::VEB_DI_IECP_COMMAND_SURFACE_CONTROL_BITS_CMD veboxInputSurfCtrlBits, veboxOutputSurfCtrlBits;

    mhw_vebox_g12_X::VEBOX_TILING_CONVERT_CMD cmd;
    MHW_RESOURCE_PARAMS ResourceParams = {0};

    MOS_UNUSED(streamID);
    VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(cmdBuffer);
    VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(inputSurface);
    VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(m_osInterface);
    VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(m_veboxInterface);
    VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(m_veboxInterface->pfnAddResourceToCmd);

    // Set up VEB_DI_IECP_COMMAND_SURFACE_CONTROL_BITS
    MOS_ZeroMemory(&veboxInputSurfCtrlBits, sizeof(veboxInputSurfCtrlBits));
    MOS_ZeroMemory(&veboxOutputSurfCtrlBits, sizeof(veboxOutputSurfCtrlBits));

    // Set Input surface compression status
    if (inputSurface->CompressionMode != MOS_MMC_DISABLED)
    {
        veboxInputSurfCtrlBits.DW0.MemoryCompressionEnable = true;

        if (inputSurface->CompressionMode == MOS_MMC_RC)
        {
            veboxInputSurfCtrlBits.DW0.CompressionType = 1;
        }
        else
        {
            veboxInputSurfCtrlBits.DW0.CompressionType = 0;
        }
    }

    switch (inputSurface->TileType)
    {
    case MOS_TILE_YF:
        veboxInputSurfCtrlBits.DW0.TiledResourceModeForOutputFrameSurfaceBaseAddress = TRMODE_TILEYF;
        break;
    case MOS_TILE_YS:
        veboxInputSurfCtrlBits.DW0.TiledResourceModeForOutputFrameSurfaceBaseAddress = TRMODE_TILEYS;
        break;
    default:
        veboxInputSurfCtrlBits.DW0.TiledResourceModeForOutputFrameSurfaceBaseAddress = TRMODE_NONE;
        break;
    }

    // Set Output surface compression status
    if (outputSurface)
    {
        // Double Buffer copy
        surface = outputSurface;

        if (outputSurface->CompressionMode == MOS_MMC_MC)
        {
            veboxOutputSurfCtrlBits.DW0.MemoryCompressionEnable = true;
            veboxOutputSurfCtrlBits.DW0.CompressionType         = 0;
        }
        else if (outputSurface->CompressionMode == MOS_MMC_RC)
        {
            veboxOutputSurfCtrlBits.DW0.MemoryCompressionEnable = true;
            veboxOutputSurfCtrlBits.DW0.CompressionType         = 1;
        }
        else
        {
            veboxOutputSurfCtrlBits.DW0.MemoryCompressionEnable = false;
            veboxOutputSurfCtrlBits.DW0.CompressionType         = 0;
        }

    }
    else
    {
        // In-Place Resolve
        surface = inputSurface;
        veboxOutputSurfCtrlBits.DW0.MemoryCompressionEnable = true;
        veboxOutputSurfCtrlBits.DW0.CompressionType         = 1;
    }

    switch (surface->TileType)
    {
    case MOS_TILE_YF:
        veboxOutputSurfCtrlBits.DW0.TiledResourceModeForOutputFrameSurfaceBaseAddress = TRMODE_TILEYF;
        break;
    case MOS_TILE_YS:
        veboxOutputSurfCtrlBits.DW0.TiledResourceModeForOutputFrameSurfaceBaseAddress = TRMODE_TILEYS;
        break;
    default:
        veboxOutputSurfCtrlBits.DW0.TiledResourceModeForOutputFrameSurfaceBaseAddress = TRMODE_NONE;
        break;
    }

    ResourceParams.presResource = &inputSurface->OsResource;
    ResourceParams.HwCommandType = MOS_VEBOX_TILING_CONVERT;

    // set up DW[2:1], input graphics address
    ResourceParams.dwLocationInCmd = 1;
    ResourceParams.pdwCmd          = &(cmd.DW1_2.Value[0]);
    ResourceParams.bIsWritable     = false;
    ResourceParams.dwOffset        = inputSurface->dwOffset + veboxInputSurfCtrlBits.DW0.Value;
    m_veboxInterface->pfnAddResourceToCmd(m_osInterface, cmdBuffer, &ResourceParams);

    MOS_ZeroMemory(&ResourceParams, sizeof(MHW_RESOURCE_PARAMS));

    if (outputSurface)
    {
        ResourceParams.presResource = &outputSurface->OsResource;
    }
    else
    {
        ResourceParams.presResource = &inputSurface->OsResource;
    }

    ResourceParams.HwCommandType = MOS_VEBOX_TILING_CONVERT;

    // set up DW[4:3], output graphics address
    ResourceParams.dwLocationInCmd = 3;
    ResourceParams.pdwCmd          = &(cmd.DW3_4.Value[0]);
    ResourceParams.bIsWritable     = true;
    ResourceParams.dwOffset        = 
        (outputSurface != nullptr ? outputSurface->dwOffset : inputSurface->dwOffset) + veboxOutputSurfCtrlBits.DW0.Value;
    m_veboxInterface->pfnAddResourceToCmd(m_osInterface, cmdBuffer, &ResourceParams);

    Mos_AddCommand(cmdBuffer, &cmd, cmd.byteSize);

    return eStatus;
}
