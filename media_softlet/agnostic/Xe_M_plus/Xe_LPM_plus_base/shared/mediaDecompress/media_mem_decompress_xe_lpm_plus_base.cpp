/*
* Copyright (c) 2022-2023, Intel Corporation
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
//! \file     media_mem_decompress_xe_lpm_plus_base.cpp
//! \brief    Common interface used in media decompression
//! \details  Common interface used in media decompression which are platform independent
//!

#include "media_mem_decompress_xe_lpm_plus_base.h"
#include "hal_oca_interface_next.h"
#include "vp_common.h"
#include "mhw_vebox_hwcmd_xe_lpm_plus_next.h"
#include "mos_interface.h"
#include "mos_os_cp_interface_specific.h"

MediaMemDeCompNext_Xe_Lpm_Plus_Base::MediaMemDeCompNext_Xe_Lpm_Plus_Base() :
    MediaMemDeCompNext()
{
}

MOS_STATUS MediaMemDeCompNext_Xe_Lpm_Plus_Base::RenderDecompCMD(PMOS_SURFACE surface)
{
    MOS_STATUS                          eStatus = MOS_STATUS_SUCCESS;
    MHW_VEBOX_STATE_CMD_PARAMS          veboxStateCmdParams;
    MOS_COMMAND_BUFFER                  cmdBuffer;
    MHW_VEBOX_SURFACE_STATE_CMD_PARAMS  mhwVeboxSurfaceStateCmdParams;
    MHW_MI_FLUSH_DW_PARAMS              flushDwParams;
    uint32_t                            streamID = 0;
    const MHW_VEBOX_HEAP*               veboxHeap = nullptr;
    MOS_CONTEXT*                        pOsContext = nullptr;
    PMHW_MI_MMIOREGISTERS               pMmioRegisters = nullptr;
    bool                                isPerfCollected = false;
    MediaPerfProfiler*                  perfProfiler = nullptr;
    uint32_t                            perfTag = 0;

    VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(surface);
    VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(m_osInterface);
    VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(pOsContext = m_osInterface->pOsContext);
    VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(m_miItf);
    VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(m_veboxItf);
    VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(pMmioRegisters = m_miItf->GetMmioRegisters());

    if (surface->CompressionMode &&
        surface->CompressionMode != MOS_MMC_MC &&
        surface->CompressionMode != MOS_MMC_RC)
    {
        VPHAL_MEMORY_DECOMP_NORMALMESSAGE("Input surface is uncompressed, In_Place resolve is not needed");
        return eStatus;
    }

    if (!IsFormatSupported(surface))
    {
        VPHAL_MEMORY_DECOMP_NORMALMESSAGE("Input surface is not supported by Vebox, In_Place resolve can't be done");
        return eStatus;
    }

    m_osInterface->pfnSetGpuContext(m_osInterface, MOS_GPU_CONTEXT_VEBOX);
    if (m_syncResource)
    {
        VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(m_osInterface->pfnRegisterResource(m_osInterface, m_syncResource, true, true));
    }

    // Reset allocation list and house keeping
    m_osInterface->pfnResetOsStates(m_osInterface);

    VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(m_veboxItf->GetVeboxHeapInfo(&veboxHeap));
    VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(&surface->OsResource);
    VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(m_osInterface->osCpInterface);

    // Check whether surface is ready for write
    m_osInterface->pfnSyncOnResource(
        m_osInterface,
        &surface->OsResource,
        MOS_GPU_CONTEXT_VEBOX,
        true);

    // preprocess in cp first
    m_osInterface->osCpInterface->PrepareResources((void**)&surface, 1, nullptr, 0);

    // initialize the command buffer struct
    MOS_ZeroMemory(&cmdBuffer, sizeof(MOS_COMMAND_BUFFER));

    VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    HalOcaInterfaceNext::On1stLevelBBStart(
        cmdBuffer, 
        (MOS_CONTEXT_HANDLE)pOsContext,
        m_osInterface->CurrentGpuContextHandle,
        m_miItf, 
        *pMmioRegisters);

    VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(InitCommandBuffer(&cmdBuffer));

    if (m_multiprocesssinglebin)
    {
        perfTag = m_osInterface->pfnGetPerfTag(m_osInterface);
        // check the decompress was called from whether media driver or apps
        if (perfTag != VPHAL_NONE)
        {
            isPerfCollected = true;

            perfProfiler = MediaPerfProfiler::Instance();
            VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(perfProfiler);
            // clear the decompress tag to separate the vpblt perf tag
            m_osInterface->pfnSetPerfTag(m_osInterface, VPHAL_NONE);
            VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(perfProfiler->Initialize((void *)this, m_osInterface));
            VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(perfProfiler->AddPerfCollectStartCmd((void *)this, m_osInterface, m_miItf, &cmdBuffer));
        }
    }

    // Prepare Vebox_Surface_State, surface input/and output are the same but the compressed status.
    VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(SetupVeboxSurfaceState(&mhwVeboxSurfaceStateCmdParams, surface, nullptr));

    //---------------------------------
    // Send Pvt MMCD CMD
    //---------------------------------
    VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(m_miItf->AddVeboxMMIOPrologCmd(&cmdBuffer));


    //---------------------------------
    // Send CMD: Vebox_Surface_State
    //---------------------------------
    VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(m_veboxItf->AddVeboxSurfaces(
        &cmdBuffer,
        &mhwVeboxSurfaceStateCmdParams));

    HalOcaInterfaceNext::OnDispatch(cmdBuffer, *m_osInterface, m_miItf, *pMmioRegisters);

    //---------------------------------
    // Send CMD: Vebox_Tiling_Convert
    //---------------------------------
    VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(VeboxSendVeboxTileConvertCMD(&cmdBuffer, surface, nullptr, streamID));

    auto& par = m_miItf->GETPAR_MI_FLUSH_DW();
    par = {};
    VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(m_miItf->ADDCMD_MI_FLUSH_DW(&cmdBuffer));

    if (!m_osInterface->bEnableKmdMediaFrameTracking && veboxHeap)
    {
        auto& par = m_miItf->GETPAR_MI_FLUSH_DW();
        par = {};
        par.pOsResource = (PMOS_RESOURCE)&veboxHeap->DriverResource;
        par.dwResourceOffset = veboxHeap->uiOffsetSync;
        par.dwDataDW1 = veboxHeap->dwNextTag;
        VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(m_miItf->ADDCMD_MI_FLUSH_DW(&cmdBuffer));
    }

    HalOcaInterfaceNext::On1stLevelBBEnd(cmdBuffer, *m_osInterface);

    if (isPerfCollected)
    {
        VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(perfProfiler);
        VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(perfProfiler->AddPerfCollectEndCmd((void *)this, m_osInterface, m_miItf, &cmdBuffer));
    }

    VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(m_miItf->AddMiBatchBufferEnd(
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

    m_veboxItf->UpdateVeboxSync();

    // Restore the perf tag
    if (isPerfCollected)
    {
        m_osInterface->pfnSetPerfTag(m_osInterface, perfTag);
    }

    return eStatus;
}

MOS_STATUS MediaMemDeCompNext_Xe_Lpm_Plus_Base::IsVeboxDecompressionEnabled()
{
    bool                                customValue = false;
    MOS_STATUS                          eStatus = MOS_STATUS_SUCCESS;

#ifdef LINUX
    customValue = true;  // enable VE Decompress on Linux
#else
    customValue = true;
#endif

    ReadUserSetting(
        m_userSettingPtr,
        m_veboxMMCResolveEnabled,
        __VPHAL_ENABLE_VEBOX_MMC_DECOMPRESS,
        MediaUserSetting::Group::Device,
        customValue,
        true);

#if (LINUX || _DEBUG || _RELEASE_INTERNAL)
    // Read multi processes single binary flag
    ReadUserSetting(
        m_userSettingPtr,
        m_multiprocesssinglebin,
        __MEDIA_USER_FEATURE_VALUE_PERF_PROFILER_MUL_PROC_SINGLE_BIN,
        MediaUserSetting::Group::Device);
#endif

    return eStatus;
}

MOS_STATUS MediaMemDeCompNext_Xe_Lpm_Plus_Base::RenderDoubleBufferDecompCMD(PMOS_SURFACE inputSurface, PMOS_SURFACE outputSurface)
{
    MOS_STATUS                          eStatus = MOS_STATUS_SUCCESS;
    MHW_VEBOX_STATE_CMD_PARAMS          veboxStateCmdParams;
    MOS_COMMAND_BUFFER                  cmdBuffer;
    MHW_VEBOX_SURFACE_STATE_CMD_PARAMS  mhwVeboxSurfaceStateCmdParams;
    MHW_MI_FLUSH_DW_PARAMS              flushDwParams;
    uint32_t                            streamID = 0;
    const MHW_VEBOX_HEAP* veboxHeap = nullptr;
    bool                                allocated = false;
    bool                                outputLinearNon64Align = false;
    bool                                inputLinearNon64Align = false;
    PMOS_SURFACE                        copy_input = nullptr;
    PMOS_SURFACE                        copy_output = nullptr;

    VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(inputSurface);
    VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(outputSurface);

    if (!IsFormatSupported(inputSurface) || !IsFormatSupported(outputSurface))
    {
        VPHAL_MEMORY_DECOMP_NORMALMESSAGE("Input surface is not supported by Vebox, In_Place resolve can't be done");
        return eStatus;
    }

    if (inputSurface->TileType == MOS_TILE_LINEAR &&
        outputSurface->TileType == MOS_TILE_LINEAR)
    {
        VPHAL_MEMORY_DECOMP_ASSERTMESSAGE("Input/Output pitch is all linear. Return unsupport!");
        return MOS_STATUS_PLATFORM_NOT_SUPPORTED;
    }
    else if ((outputSurface->TileType == MOS_TILE_LINEAR && (!MOS_IS_ALIGNED(outputSurface->dwPitch, MHW_VEBOX_LINEAR_PITCH))))
    {
        VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(ReAllocateLinearSurface(
            &m_tempLinearSurface,
            "OutputLinearBuffer",
            outputSurface->Format,
            MOS_GFXRES_2D,
            MOS_ALIGN_CEIL(outputSurface->dwWidth, MHW_VEBOX_LINEAR_PITCH),
            outputSurface->dwHeight,
            &allocated));

        if (allocated)
        {
            m_tempLinearSurface.dwWidth = outputSurface->dwWidth;
            outputLinearNon64Align = true;
            copy_input = inputSurface;
            copy_output = &m_tempLinearSurface;
        }
        else
        {
            VPHAL_MEMORY_DECOMP_ASSERTMESSAGE("Input/Output pitch is not 64 aligned. Return Invalid Status!");
            return MOS_STATUS_INVALID_PARAMETER;
        }
    }
    else if ((inputSurface->TileType == MOS_TILE_LINEAR && (!MOS_IS_ALIGNED(inputSurface->dwPitch, MHW_VEBOX_LINEAR_PITCH))))
    {
        VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(ReAllocateLinearSurface(
            &m_tempLinearSurface,
            "OutputLinearBuffer",
            inputSurface->Format,
            MOS_GFXRES_2D,
            MOS_ALIGN_CEIL(inputSurface->dwWidth, MHW_VEBOX_LINEAR_PITCH),
            inputSurface->dwHeight,
            &allocated));

        if (allocated)
        {
            m_tempLinearSurface.dwWidth = inputSurface->dwWidth;
            inputLinearNon64Align = true;
            copy_input = &m_tempLinearSurface;
            copy_output = outputSurface;
        }
        else
        {
            VPHAL_MEMORY_DECOMP_ASSERTMESSAGE("Input/Output pitch is not 64 aligned. Return Invalid Status!");
            return MOS_STATUS_INVALID_PARAMETER;
        }
    }
    else
    {
        copy_input = inputSurface;
        copy_output = outputSurface;
        VPHAL_MEMORY_DECOMP_NORMALMESSAGE("Vebox Can handled copy conditions");
    }

    m_osInterface->pfnSetGpuContext(m_osInterface, MOS_GPU_CONTEXT_VEBOX);

    // Reset allocation list and house keeping
    m_osInterface->pfnResetOsStates(m_osInterface);

    VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(m_veboxItf->GetVeboxHeapInfo(&veboxHeap));
    VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(&copy_input->OsResource);
    VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(&copy_output->OsResource);
    VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(m_osInterface->osCpInterface);

    //there is a new usage that input surface is clear and output surface is secure.
    //replace Huc Copy by DoubleBuffer resolve to update ccs data.
    //So need consolidate both input/output surface information to decide cp context.
    PMOS_SURFACE surfaceArray[2];
    surfaceArray[0] = copy_input;
    surfaceArray[1] = copy_output;

    // preprocess in cp first
    m_osInterface->osCpInterface->PrepareResources((void**)&surfaceArray, sizeof(surfaceArray) / sizeof(PMOS_SURFACE), nullptr, 0);

    if (inputLinearNon64Align)
    {
        VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(LinearCopyWith64Aligned(inputSurface, &m_tempLinearSurface));
    }

    // initialize the command buffer struct
    MOS_ZeroMemory(&cmdBuffer, sizeof(MOS_COMMAND_BUFFER));

    VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));
    VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(InitCommandBuffer(&cmdBuffer));

    // Prepare Vebox_Surface_State, surface input/and output are the same but the compressed status.
    VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(SetupVeboxSurfaceState(&mhwVeboxSurfaceStateCmdParams, copy_input, copy_output));

    //---------------------------------
    // Send Pvt MMCD CMD
    //---------------------------------
    VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(m_miItf->AddVeboxMMIOPrologCmd(&cmdBuffer));

    //---------------------------------
    // Send CMD: Vebox_Surface_State
    //---------------------------------
    VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(m_veboxItf->AddVeboxSurfaces(
        &cmdBuffer,
        &mhwVeboxSurfaceStateCmdParams));

    //---------------------------------
    // Send CMD: Vebox_Tiling_Convert
    //---------------------------------
    VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(VeboxSendVeboxTileConvertCMD(&cmdBuffer, copy_input, copy_output, streamID));

    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));

    auto& par = m_miItf->GETPAR_MI_FLUSH_DW();
    par = {};
    VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(m_miItf->ADDCMD_MI_FLUSH_DW(&cmdBuffer));

    if (!m_osInterface->bEnableKmdMediaFrameTracking && veboxHeap)
    {
        auto& par = m_miItf->GETPAR_MI_FLUSH_DW();
        par = {};
        par.pOsResource = (PMOS_RESOURCE)&veboxHeap->DriverResource;
        par.dwResourceOffset = veboxHeap->uiOffsetSync;
        par.dwDataDW1 = veboxHeap->dwNextTag;
        VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(m_miItf->ADDCMD_MI_FLUSH_DW(&cmdBuffer));
    }

    VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(m_miItf->AddMiBatchBufferEnd(
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

    m_veboxItf->UpdateVeboxSync();

    if (outputLinearNon64Align)
    {
        VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(LinearCopyWith64Aligned(&m_tempLinearSurface, outputSurface));
    }

    return eStatus;
}

MOS_STATUS MediaMemDeCompNext_Xe_Lpm_Plus_Base::LinearCopyWith64Aligned(PMOS_SURFACE inputSurface, PMOS_SURFACE outputSurface)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    //if surface is linear buffer, use mos copy
    if (inputSurface->TileType == MOS_TILE_LINEAR &&
        outputSurface->TileType == MOS_TILE_LINEAR &&
        inputSurface->Type == MOS_GFXRES_BUFFER &&
        outputSurface->Type == MOS_GFXRES_BUFFER)
    {
        do
        {
            MOS_LOCK_PARAMS lockSourceFlags;
            MOS_ZeroMemory(&lockSourceFlags, sizeof(MOS_LOCK_PARAMS));
            lockSourceFlags.ReadOnly = 1;
            lockSourceFlags.WriteOnly = 0;

            MOS_LOCK_PARAMS lockTargetFlags;
            MOS_ZeroMemory(&lockTargetFlags, sizeof(MOS_LOCK_PARAMS));
            lockTargetFlags.ReadOnly = 0;
            lockTargetFlags.WriteOnly = 1;

            uint8_t* lockedSrcAddr = (uint8_t*)m_osInterface->pfnLockResource(m_osInterface, &inputSurface->OsResource, &lockSourceFlags);

            if (lockedSrcAddr == nullptr)
            {
                //non lockable resource enabled, we can't lock source surface
                eStatus = MOS_STATUS_NULL_POINTER;
                VPHAL_MEMORY_DECOMP_ASSERTMESSAGE("Failed to lock non-lockable input resource, buffer copy failed, eStatus:%d.\n", eStatus);
                break;
            }

            uint8_t* lockedTarAddr = (uint8_t*)m_osInterface->pfnLockResource(m_osInterface, &outputSurface->OsResource, &lockTargetFlags);

            if (lockedTarAddr == nullptr)
            {
                eStatus = MOS_STATUS_NULL_POINTER;
                m_osInterface->pfnUnlockResource(m_osInterface, &inputSurface->OsResource);
                VPHAL_MEMORY_DECOMP_ASSERTMESSAGE("Failed to lock non-lockable output resource, buffer copy failed, eStatus:%d.\n", eStatus);
                break;
            }
            // This resource is a series of bytes. Is not 2 dimensional.

            for (uint32_t copy_height = 0; copy_height < inputSurface->dwHeight; copy_height++)
            {
                uint32_t copy_width = MOS_MIN(inputSurface->dwPitch, outputSurface->dwPitch);
                eStatus = MOS_SecureMemcpy(lockedTarAddr, copy_width, lockedSrcAddr, copy_width);
                lockedSrcAddr += inputSurface->dwPitch;
                lockedTarAddr += outputSurface->dwPitch;
            }
            m_osInterface->pfnUnlockResource(m_osInterface, &inputSurface->OsResource);
            m_osInterface->pfnUnlockResource(m_osInterface, &outputSurface->OsResource);

            if (eStatus != MOS_STATUS_SUCCESS)
            {
                VPHAL_MEMORY_DECOMP_ASSERTMESSAGE("Failed to copy linear buffer from source to target, eStatus:%d.\n", eStatus);
                break;
            }
        } while (false);

        MOS_TraceEventExt(EVENT_MEDIA_COPY, EVENT_TYPE_END, nullptr, 0, nullptr, 0);
        return eStatus;
    }

    return eStatus;
}

MOS_STATUS MediaMemDeCompNext_Xe_Lpm_Plus_Base::ReAllocateLinearSurface(PMOS_SURFACE pSurface, PCCHAR pSurfaceName, MOS_FORMAT format, MOS_GFXRES_TYPE DefaultResType, uint32_t dwWidth, uint32_t dwHeight, bool* pbAllocated)
{
    MOS_STATUS              eStatus;
    VPHAL_GET_SURFACE_INFO  Info;
    MOS_ALLOC_GFXRES_PARAMS allocParams;

    //---------------------------------
    VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(m_osInterface);
    //---------------------------------

    eStatus = MOS_STATUS_SUCCESS;
    *pbAllocated = false;

    if (!Mos_ResourceIsNull(&pSurface->OsResource) &&
        (pSurface->dwWidth == dwWidth) &&
        (pSurface->dwHeight == dwHeight) &&
        (pSurface->Format == format))
    {
        *pbAllocated = true;
        return eStatus;
    }

    MOS_ZeroMemory(&allocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));

    allocParams.Type = DefaultResType;
    allocParams.TileType = MOS_TILE_LINEAR;

    allocParams.dwWidth = dwWidth;
    allocParams.dwHeight = dwHeight;
    allocParams.Format = format;
    allocParams.bIsCompressible = false;
    allocParams.pBufName = pSurfaceName;
    allocParams.dwArraySize = 1;
    allocParams.ResUsageType = MOS_HW_RESOURCE_USAGE_VP_INTERNAL_READ_WRITE_FF;

    // Delete resource if already allocated
    m_osInterface->pfnFreeResource(m_osInterface, &(pSurface->OsResource));

    // Allocate surface
    VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParams,
        &pSurface->OsResource));

    // Get surface information
    MOS_ZeroMemory(&Info, sizeof(VPHAL_GET_SURFACE_INFO));

    // Pre-set to get surface info
    pSurface->Format = format;
    VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(GetResourceInfo(pSurface));
    *pbAllocated = true;

    return eStatus;
}

MOS_STATUS MediaMemDeCompNext_Xe_Lpm_Plus_Base::VeboxSendVeboxTileConvertCMD(PMOS_COMMAND_BUFFER cmdBuffer, PMOS_SURFACE inputSurface, PMOS_SURFACE outputSurface, uint32_t streamID)
{
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;
    MHW_VEBOX_SURFACE_PARAMS        inputSurfaceParam = {};
    MHW_VEBOX_SURFACE_PARAMS        outputSurfaceParam = {};
    PMOS_SURFACE                    surface = nullptr;

    MOS_UNUSED(streamID);
    VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(cmdBuffer);
    VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(inputSurface);
    VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(m_osInterface);
    VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(m_veboxItf);

    inputSurfaceParam.pOsResource       = &inputSurface->OsResource;
    inputSurfaceParam.CompressionMode   = inputSurface->CompressionMode;
    inputSurfaceParam.TileType          = inputSurface->TileType;
    inputSurfaceParam.bGMMTileEnabled   = inputSurface->bGMMTileEnabled;
    inputSurfaceParam.TileModeGMM       = inputSurface->TileModeGMM;
    inputSurfaceParam.dwOffset          = inputSurface->dwOffset;

    // Set Output surface compression status
    if (outputSurface)
    {
        // Double Buffer copy
        outputSurfaceParam.pOsResource       = &outputSurface->OsResource;
        outputSurfaceParam.CompressionMode   = outputSurface->CompressionMode;
        outputSurfaceParam.TileType          = outputSurface->TileType;
        outputSurfaceParam.bGMMTileEnabled   = outputSurface->bGMMTileEnabled;
        outputSurfaceParam.TileModeGMM       = outputSurface->TileModeGMM;
        outputSurfaceParam.dwOffset          = outputSurface->dwOffset;

    }
    else
    {
        // In-Place Resolve
        outputSurfaceParam = inputSurfaceParam;
        if (outputSurfaceParam.CompressionMode != MOS_MMC_DISABLED)
        {
            outputSurfaceParam.CompressionMode = MOS_MMC_RC;
        }
    }

    VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(m_veboxItf->AddVeboxTilingConvert(cmdBuffer, &inputSurfaceParam, &outputSurfaceParam));

    return eStatus;
}
