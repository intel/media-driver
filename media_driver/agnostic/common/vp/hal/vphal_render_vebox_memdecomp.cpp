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
//! \file     vphal_render_vebox_memdecomp.cpp
//! \brief    Defines data structures and interfaces for media memory decompression.
//! \details
//!
#include "vphal_render_vebox_memdecomp.h"
#include "vphal_debug.h"

MediaVeboxDecompState::MediaVeboxDecompState():
    MediaMemDecompBaseState(),
    m_osInterface(nullptr),
    m_veboxInterface(nullptr),
    m_mhwMiInterface(nullptr),
    m_cpInterface(nullptr)
{
    m_veboxMMCResolveEnabled = false;
}

MediaVeboxDecompState::~MediaVeboxDecompState()
{
    MOS_STATUS              eStatus;

    if (m_cpInterface)
    {
        Delete_MhwCpInterface(m_cpInterface);
        m_cpInterface = nullptr;
    }

    if (m_veboxInterface)
    {
#if (_DEBUG || _RELEASE_INTERNAL)
        if (m_surfaceDumper)
        {
            MOS_Delete(m_surfaceDumper);
            m_surfaceDumper = nullptr;
        }
#endif
        eStatus = m_veboxInterface->DestroyHeap();
        MOS_Delete(m_veboxInterface);
        m_veboxInterface = nullptr;
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            VPHAL_MEMORY_DECOMP_ASSERTMESSAGE("Failed to destroy Vebox Interface, eStatus:%d.\n", eStatus);
        }
    }

    if (m_mhwMiInterface)
    {
        MOS_Delete(m_mhwMiInterface);
        m_mhwMiInterface = nullptr;
    }

    if (m_osInterface)
    {
        m_osInterface->pfnDestroy(m_osInterface, false);
        MOS_FreeMemory(m_osInterface);
        m_osInterface = nullptr;
    }
}

#if (_DEBUG || _RELEASE_INTERNAL)
MOS_STATUS CloneResourceInfo(PVPHAL_SURFACE pVphalSurface, PMOS_SURFACE pMosSurface)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(pVphalSurface);
    VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(pMosSurface);

    pVphalSurface->SurfType = SURF_NONE;

    pVphalSurface->OsResource        = pMosSurface->OsResource;
    pVphalSurface->dwWidth           = pMosSurface->dwWidth;
    pVphalSurface->dwHeight          = pMosSurface->dwHeight;
    pVphalSurface->dwPitch           = pMosSurface->dwPitch;
    pVphalSurface->Format            = pMosSurface->Format;
    pVphalSurface->TileType          = pMosSurface->TileType;
    pVphalSurface->dwDepth           = pMosSurface->dwDepth;
    pVphalSurface->dwSlicePitch      = pMosSurface->dwSlicePitch;
    pVphalSurface->dwOffset          = pMosSurface->dwOffset;
    pVphalSurface->bCompressible     = pMosSurface->bCompressible;
    pVphalSurface->bIsCompressed     = pMosSurface->bIsCompressed;
    pVphalSurface->CompressionMode   = pMosSurface->CompressionMode;
    pVphalSurface->CompressionFormat = pMosSurface->CompressionFormat;

    pVphalSurface->YPlaneOffset.iLockSurfaceOffset = pMosSurface->YPlaneOffset.iLockSurfaceOffset;
    pVphalSurface->YPlaneOffset.iSurfaceOffset     = pMosSurface->YPlaneOffset.iSurfaceOffset;
    pVphalSurface->YPlaneOffset.iXOffset           = pMosSurface->YPlaneOffset.iXOffset;
    pVphalSurface->YPlaneOffset.iYOffset           = pMosSurface->YPlaneOffset.iYOffset;

    pVphalSurface->UPlaneOffset.iLockSurfaceOffset = pMosSurface->UPlaneOffset.iLockSurfaceOffset;
    pVphalSurface->UPlaneOffset.iSurfaceOffset     = pMosSurface->UPlaneOffset.iSurfaceOffset;
    pVphalSurface->UPlaneOffset.iXOffset           = pMosSurface->UPlaneOffset.iXOffset;
    pVphalSurface->UPlaneOffset.iYOffset           = pMosSurface->UPlaneOffset.iYOffset;

    pVphalSurface->VPlaneOffset.iLockSurfaceOffset = pMosSurface->VPlaneOffset.iLockSurfaceOffset;
    pVphalSurface->VPlaneOffset.iSurfaceOffset     = pMosSurface->VPlaneOffset.iSurfaceOffset;
    pVphalSurface->VPlaneOffset.iXOffset           = pMosSurface->VPlaneOffset.iXOffset;
    pVphalSurface->VPlaneOffset.iYOffset           = pMosSurface->VPlaneOffset.iYOffset;

    return eStatus;
}

#define DumpSurfaceMemDecomp(MosSurface, SurfaceCounter, BufferCopy, Location) \
    {                                                                                                      \
        VPHAL_SURFACE VphalSurface = {};                                                                   \
        CloneResourceInfo(&VphalSurface, &MosSurface);                                     \
        if (m_surfaceDumper)                                                                               \
        {                                                                                                  \
            m_surfaceDumper->DumpSurface(&VphalSurface, SurfaceCounter, BufferCopy, Location);             \
        }                                                                                                  \
    }
#else
#define DumpSurfaceMemDecomp(MosSurface, surfaceCounter, DoubleCopy, Locationlocation) {}
#endif

MOS_STATUS MediaVeboxDecompState::MemoryDecompress(PMOS_RESOURCE targetResource)
{
    MOS_STATUS              eStatus = MOS_STATUS_SUCCESS;
    MOS_SURFACE             TargetSurface;

    MHW_FUNCTION_ENTER;

    VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(targetResource);
    
#if MOS_MEDIASOLO_SUPPORTED
    if (m_osInterface->bSoloInUse)
    {
        // Bypass
    }
    else
#endif
    {
        if (m_veboxMMCResolveEnabled)
        {
            MOS_ZeroMemory(&TargetSurface, sizeof(MOS_SURFACE));

            TargetSurface.Format = Format_Invalid;
            TargetSurface.OsResource = *targetResource;
            VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(GetResourceInfo(&TargetSurface));

            //Get context before proceeding
            auto gpuContext = m_osInterface->CurrentGpuContextOrdinal;

            if (TargetSurface.bCompressible)
            {
                DumpSurfaceMemDecomp(TargetSurface, m_surfaceDumpCounter, 0, VPHAL_DBG_DUMP_TYPE_PRE_MEMDECOMP);

                VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(RenderDecompCMD(&TargetSurface));

                DumpSurfaceMemDecomp(TargetSurface, m_surfaceDumpCounter++, 0, VPHAL_DBG_DUMP_TYPE_POST_MEMDECOMP);
            }
        }
    }

    return eStatus;
}

MOS_STATUS MediaVeboxDecompState::MediaMemoryCopy(
    PMOS_RESOURCE inputResource,
    PMOS_RESOURCE outputResource,
    bool          outputCompressed)
{
    MOS_STATUS                          eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(inputResource);
    VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(outputResource);

    MOS_SURFACE             sourceSurface;
    MOS_SURFACE             targetSurface;

    MOS_ZeroMemory(&targetSurface, sizeof(MOS_SURFACE));
    MOS_ZeroMemory(&sourceSurface, sizeof(MOS_SURFACE));

    targetSurface.Format     = Format_Invalid;
    targetSurface.OsResource = *outputResource;

#if !defined(LINUX) && !defined(ANDROID) && !EMUL
    // for Double Buffer copy, clear the allocationInfo temply
    MOS_ZeroMemory(&targetSurface.OsResource.AllocationInfo, sizeof(SResidencyInfo));
#endif

    sourceSurface.Format = Format_Invalid;
    sourceSurface.OsResource = *inputResource;
    VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(GetResourceInfo(&targetSurface));
    VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(GetResourceInfo(&sourceSurface));

    if (!outputCompressed && targetSurface.CompressionMode != MOS_MMC_DISABLED)
    {
        targetSurface.CompressionMode = MOS_MMC_RC;
    }

    //Get context before proceeding
    auto gpuContext = m_osInterface->CurrentGpuContextOrdinal;

    // Sync for Vebox write
    m_osInterface->pfnSyncOnResource(
        m_osInterface,
        &sourceSurface.OsResource,
        MOS_GPU_CONTEXT_VEBOX,
        false);

    DumpSurfaceMemDecomp(sourceSurface, m_surfaceDumpCounter, 1, VPHAL_DBG_DUMP_TYPE_PRE_MEMDECOMP);

    VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(RenderDoubleBufferDecompCMD(&sourceSurface, &targetSurface));

    DumpSurfaceMemDecomp(targetSurface, m_surfaceDumpCounter++, 1, VPHAL_DBG_DUMP_TYPE_POST_MEMDECOMP);

    return eStatus;
}

MOS_STATUS MediaVeboxDecompState::Initialize(
    PMOS_INTERFACE               osInterface,
    MhwCpInterface              *cpInterface,
    PMHW_MI_INTERFACE            mhwMiInterface,
    PMHW_VEBOX_INTERFACE         veboxInterface)
{
    MOS_STATUS                  eStatus;
    MHW_VEBOX_GPUNODE_LIMIT     GpuNodeLimit;
    MOS_GPU_NODE                VeboxGpuNode;
    MOS_GPU_CONTEXT             VeboxGpuContext;
    RENDERHAL_SETTINGS          RenderHalSettings;

    eStatus = MOS_STATUS_SUCCESS;

    VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(osInterface);
    VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(cpInterface);
    VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(mhwMiInterface);
    VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(veboxInterface);

    m_osInterface     = osInterface;
    m_cpInterface     = cpInterface;
    m_mhwMiInterface  = mhwMiInterface;
    m_veboxInterface  = veboxInterface;

    // Set-Up Vebox decompression enable or not
    IsVeboxDecompressionEnabled();

    if (m_veboxInterface)
    {
        GpuNodeLimit.bCpEnabled = (m_osInterface->osCpInterface->IsCpEnabled())? true : false;

        // Check GPU Node decide logic together in this function
        VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(m_veboxInterface->FindVeboxGpuNodeToUse(&GpuNodeLimit));

        VeboxGpuNode = (MOS_GPU_NODE)(GpuNodeLimit.dwGpuNodeToUse);
        VeboxGpuContext = (VeboxGpuNode == MOS_GPU_NODE_VE) ? MOS_GPU_CONTEXT_VEBOX : MOS_GPU_CONTEXT_VEBOX2;

        // Create VEBOX/VEBOX2 Context
        VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(m_veboxInterface->CreateGpuContext(
            m_osInterface,
            VeboxGpuContext,
            VeboxGpuNode));

        // Register Vebox GPU context with the Batch Buffer completion event
        VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(m_osInterface->pfnRegisterBBCompleteNotifyEvent(
            m_osInterface,
            MOS_GPU_CONTEXT_VEBOX));

        if (m_veboxInterface->m_veboxHeap == nullptr)
        {
            m_veboxInterface->CreateHeap();
        }

#if (_DEBUG || _RELEASE_INTERNAL)
        CreateSurfaceDumper();
        if (m_surfaceDumper)
        {
            m_surfaceDumper->GetSurfaceDumpSpec();
        }
        else
        {
            VPHAL_MEMORY_DECOMP_ASSERTMESSAGE("surface dumpper creation failed.");
        }
#endif
    }

    return eStatus;
}
#if (_DEBUG || _RELEASE_INTERNAL)
void MediaVeboxDecompState::CreateSurfaceDumper()
{
    m_surfaceDumper = MOS_New(VphalSurfaceDumper, m_osInterface);
}
#endif

MOS_STATUS MediaVeboxDecompState::GetResourceInfo(PMOS_SURFACE surface)
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;

    VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(m_osInterface);
    VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(surface);

    MOS_SURFACE resDetails;
    MOS_ZeroMemory(&resDetails, sizeof(resDetails));
    resDetails.Format = Format_Invalid;

    VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(m_osInterface->pfnGetResourceInfo(
        m_osInterface,
        &surface->OsResource,
        &resDetails));

    surface->Format                                             = resDetails.Format;
    surface->dwWidth                                            = resDetails.dwWidth;
    surface->dwHeight                                           = resDetails.dwHeight;
    surface->dwPitch                                            = resDetails.dwPitch;
    surface->dwDepth                                            = resDetails.dwDepth;
    surface->bArraySpacing                                      = resDetails.bArraySpacing;
    surface->TileType                                           = resDetails.TileType;
    surface->bCompressible                                      = resDetails.bCompressible;
    surface->bIsCompressed                                      = resDetails.bIsCompressed;
    surface->dwOffset                                           = resDetails.RenderOffset.YUV.Y.BaseOffset;
    surface->UPlaneOffset.iSurfaceOffset                        = resDetails.RenderOffset.YUV.U.BaseOffset;
    surface->UPlaneOffset.iXOffset                              = resDetails.RenderOffset.YUV.U.XOffset;
    surface->UPlaneOffset.iYOffset                              = resDetails.RenderOffset.YUV.U.YOffset;
    surface->VPlaneOffset.iSurfaceOffset                        = resDetails.RenderOffset.YUV.V.BaseOffset;
    surface->VPlaneOffset.iXOffset                              = resDetails.RenderOffset.YUV.V.XOffset;
    surface->VPlaneOffset.iYOffset                              = resDetails.RenderOffset.YUV.V.YOffset;
    surface->dwSize                                             = (uint32_t)surface->OsResource.pGmmResInfo->GetSizeMainSurface();

    MOS_MEMCOMP_STATE mmcMode;

    MOS_ZeroMemory(&mmcMode, sizeof(mmcMode));
    m_osInterface->pfnGetMemoryCompressionMode(m_osInterface, &surface->OsResource, &mmcMode);
    surface->CompressionMode = (MOS_RESOURCE_MMC_MODE)mmcMode;

    if (mmcMode)
    {
        m_osInterface->pfnGetMemoryCompressionFormat(m_osInterface, &surface->OsResource, &surface->CompressionFormat);
        if ((surface->TileType == MOS_TILE_Y ||
             surface->TileType == MOS_TILE_YS))
        {
            surface->bCompressible   = true;
            surface->bIsCompressed   = true;
            surface->CompressionMode = (MOS_RESOURCE_MMC_MODE)mmcMode;
        }
    }

    return eStatus;
}

#define SURFACE_DW_UY_OFFSET(pSurface) \
    ((pSurface) != nullptr ? ((pSurface)->UPlaneOffset.iSurfaceOffset - (pSurface)->dwOffset) / (pSurface)->dwPitch + (pSurface)->UPlaneOffset.iYOffset : 0)

#define SURFACE_DW_VY_OFFSET(pSurface) \
    ((pSurface) != nullptr ? ((pSurface)->VPlaneOffset.iSurfaceOffset - (pSurface)->dwOffset) / (pSurface)->dwPitch + (pSurface)->VPlaneOffset.iYOffset : 0)

MOS_STATUS MediaVeboxDecompState::SetupVeboxSurfaceState(
    PMHW_VEBOX_SURFACE_STATE_CMD_PARAMS mhwVeboxSurfaceStateCmdParams, 
    PMOS_SURFACE                        inputSurface,
    PMOS_SURFACE                        outputSurface)
{
    MOS_STATUS              eStatus = MOS_STATUS_SUCCESS;

    VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(inputSurface);
    VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(mhwVeboxSurfaceStateCmdParams);

    MOS_ZeroMemory(mhwVeboxSurfaceStateCmdParams, sizeof(*mhwVeboxSurfaceStateCmdParams));

    mhwVeboxSurfaceStateCmdParams->SurfInput.bActive    = mhwVeboxSurfaceStateCmdParams->SurfOutput.bActive    = true;
    mhwVeboxSurfaceStateCmdParams->SurfInput.dwBitDepth = mhwVeboxSurfaceStateCmdParams->SurfOutput.dwBitDepth = inputSurface->dwDepth;
    mhwVeboxSurfaceStateCmdParams->SurfInput.dwHeight   = mhwVeboxSurfaceStateCmdParams->SurfOutput.dwHeight   = inputSurface->dwHeight;
    mhwVeboxSurfaceStateCmdParams->SurfInput.dwWidth    = mhwVeboxSurfaceStateCmdParams->SurfOutput.dwWidth    = inputSurface->dwWidth;
    mhwVeboxSurfaceStateCmdParams->SurfInput.Format     = mhwVeboxSurfaceStateCmdParams->SurfOutput.Format     = inputSurface->Format;


    if (inputSurface->dwPitch > 0            &&
       (inputSurface->Format == Format_P010  ||
        inputSurface->Format == Format_P016  ||
        inputSurface->Format == Format_NV12))
    {
        mhwVeboxSurfaceStateCmdParams->SurfInput.dwUYoffset = SURFACE_DW_UY_OFFSET(inputSurface);

        if (outputSurface)
        {
            mhwVeboxSurfaceStateCmdParams->SurfOutput.dwUYoffset = SURFACE_DW_UY_OFFSET(outputSurface);
        }
        else
        {
            mhwVeboxSurfaceStateCmdParams->SurfOutput.dwUYoffset = mhwVeboxSurfaceStateCmdParams->SurfInput.dwUYoffset;
        }
    }

    mhwVeboxSurfaceStateCmdParams->SurfInput.rcMaxSrc.left   = mhwVeboxSurfaceStateCmdParams->SurfOutput.rcMaxSrc.left   = 0;
    mhwVeboxSurfaceStateCmdParams->SurfInput.rcMaxSrc.right  = mhwVeboxSurfaceStateCmdParams->SurfOutput.rcMaxSrc.right  = (long)inputSurface->dwWidth;
    mhwVeboxSurfaceStateCmdParams->SurfInput.rcMaxSrc.top    = mhwVeboxSurfaceStateCmdParams->SurfOutput.rcMaxSrc.top    = 0;
    mhwVeboxSurfaceStateCmdParams->SurfInput.rcMaxSrc.bottom = mhwVeboxSurfaceStateCmdParams->SurfOutput.rcMaxSrc.bottom = (long)inputSurface->dwHeight;
    mhwVeboxSurfaceStateCmdParams->bOutputValid = true;

    // if output surface is null, then Inplace resolve happens
    if (!outputSurface)
    {
        mhwVeboxSurfaceStateCmdParams->SurfInput.TileType    = mhwVeboxSurfaceStateCmdParams->SurfOutput.TileType    = inputSurface->TileType;
        mhwVeboxSurfaceStateCmdParams->SurfOutput.dwPitch    = mhwVeboxSurfaceStateCmdParams->SurfInput.dwPitch      = inputSurface->dwPitch;
        mhwVeboxSurfaceStateCmdParams->SurfInput.pOsResource = mhwVeboxSurfaceStateCmdParams->SurfOutput.pOsResource = &(inputSurface->OsResource);
        mhwVeboxSurfaceStateCmdParams->SurfInput.dwYoffset   = mhwVeboxSurfaceStateCmdParams->SurfOutput.dwYoffset   = inputSurface->YPlaneOffset.iYOffset;
        
        mhwVeboxSurfaceStateCmdParams->SurfInput.dwCompressionFormat = mhwVeboxSurfaceStateCmdParams->SurfOutput.dwCompressionFormat =
            inputSurface->CompressionFormat;
    }
    else
    // double buffer resolve
    {
        mhwVeboxSurfaceStateCmdParams->SurfInput.TileType     = inputSurface->TileType;
        mhwVeboxSurfaceStateCmdParams->SurfOutput.TileType    = outputSurface->TileType;
        mhwVeboxSurfaceStateCmdParams->SurfInput.dwPitch      = inputSurface->dwPitch;
        mhwVeboxSurfaceStateCmdParams->SurfOutput.dwPitch     = outputSurface->dwPitch;
        mhwVeboxSurfaceStateCmdParams->SurfInput.pOsResource  = &(inputSurface->OsResource);
        mhwVeboxSurfaceStateCmdParams->SurfOutput.pOsResource = &(outputSurface->OsResource);
        mhwVeboxSurfaceStateCmdParams->SurfInput.dwYoffset    = inputSurface->YPlaneOffset.iYOffset;
        mhwVeboxSurfaceStateCmdParams->SurfInput.dwCompressionFormat = inputSurface->CompressionFormat;
        mhwVeboxSurfaceStateCmdParams->SurfOutput.dwCompressionFormat = outputSurface->CompressionFormat;
    }

    return eStatus;
}

MOS_STATUS MediaVeboxDecompState::InitCommandBuffer(
    PMOS_COMMAND_BUFFER               cmdBuffer)
{
    PMOS_INTERFACE              pOsInterface;
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;
    MEDIA_SYSTEM_INFO           *pGtSystemInfo;
    uint32_t                    iRemaining;
    RENDERHAL_GENERIC_PROLOG_PARAMS         GenericPrologParams = {};
    MOS_RESOURCE                            GpuStatusBuffer;

    //---------------------------------------------
    VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(cmdBuffer);
    VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(m_osInterface);
    VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(m_mhwMiInterface);
    //---------------------------------------------

    eStatus = MOS_STATUS_SUCCESS;
    pOsInterface = m_osInterface;

    MOS_GPU_CONTEXT gpuContext = m_osInterface->pfnGetGpuContext(m_osInterface);

#ifndef EMUL
    if (pOsInterface->bEnableKmdMediaFrameTracking)
    {
        // Get GPU Status buffer
        VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(pOsInterface->pfnGetGpuStatusBufferResource(pOsInterface, &GpuStatusBuffer));

        // Register the buffer
        VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(pOsInterface->pfnRegisterResource(pOsInterface, &GpuStatusBuffer, true, true));

        GenericPrologParams.bEnableMediaFrameTracking      = true;
        GenericPrologParams.presMediaFrameTrackingSurface  = &GpuStatusBuffer;
        GenericPrologParams.dwMediaFrameTrackingTag        = pOsInterface->pfnGetGpuStatusTag(pOsInterface, pOsInterface->CurrentGpuContextOrdinal);
        GenericPrologParams.dwMediaFrameTrackingAddrOffset = pOsInterface->pfnGetGpuStatusTagOffset(pOsInterface, pOsInterface->CurrentGpuContextOrdinal);

        // Increment GPU Status Tag
        pOsInterface->pfnIncrementGpuStatusTag(pOsInterface, pOsInterface->CurrentGpuContextOrdinal);
    }
#endif

    if (GenericPrologParams.bEnableMediaFrameTracking)
    {
        VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(GenericPrologParams.presMediaFrameTrackingSurface);
        cmdBuffer->Attributes.bEnableMediaFrameTracking       = GenericPrologParams.bEnableMediaFrameTracking;
        cmdBuffer->Attributes.dwMediaFrameTrackingTag         = GenericPrologParams.dwMediaFrameTrackingTag;
        cmdBuffer->Attributes.dwMediaFrameTrackingAddrOffset  = GenericPrologParams.dwMediaFrameTrackingAddrOffset;
        cmdBuffer->Attributes.resMediaFrameTrackingSurface    = *(GenericPrologParams.presMediaFrameTrackingSurface);
    }

    // initialize command buffer attributes
    cmdBuffer->Attributes.bTurboMode = false;
    cmdBuffer->Attributes.bMediaPreemptionEnabled = false;
    cmdBuffer->Attributes.dwMediaFrameTrackingAddrOffset = 0;

    MHW_GENERIC_PROLOG_PARAMS genericPrologParams;
    MOS_ZeroMemory(&genericPrologParams, sizeof(genericPrologParams));
    genericPrologParams.pOsInterface = m_osInterface;
    genericPrologParams.pvMiInterface = m_mhwMiInterface;
    genericPrologParams.bMmcEnabled = true;

    VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(Mhw_SendGenericPrologCmd(
        cmdBuffer,
        &genericPrologParams));

    return eStatus;
}

bool MediaVeboxDecompState::IsDecompressionFormatSupported(PMOS_SURFACE surface)
{
    bool    bRet = false;

    if (surface->Format == Format_R10G10B10A2 ||
        surface->Format == Format_B10G10R10A2)
    {
        //For Vebox comsume RGB10, Re-map RGB10 as Y410
        surface->Format = Format_Y410;
    }

    if (surface->Format == Format_A8 ||
        surface->Format == Format_Y8 ||
        surface->Format == Format_L8 ||
        surface->Format == Format_P8 ||
        surface->Format == Format_STMM)
    {
        surface->Format = Format_P8;
    }

    if (surface->Format == Format_IMC3 ||
        surface->Format == Format_444P ||
        surface->Format == Format_422H ||
        surface->Format == Format_422V ||
        surface->Format == Format_411P ||
        surface->Format == Format_411R ||
        surface->Format == Format_444P ||
        surface->Format == Format_RGBP ||
        surface->Format == Format_BGRP ||
        surface->Format == Format_400P ||
        surface->Format == Format_420O )
    {
        surface->Format   = Format_P8;
        surface->dwHeight = surface->dwSize / surface->dwPitch;
    }

    // Check if Sample Format is supported for decompression
    if (surface->Format != Format_NV12        &&
        surface->Format != Format_AYUV        &&
        surface->Format != Format_Y416        &&
        surface->Format != Format_P010        &&
        surface->Format != Format_P016        &&
        !IS_PA_FORMAT(surface->Format)        &&
        surface->Format != Format_A8R8G8B8    &&
        surface->Format != Format_A8B8G8R8    &&
        surface->Format != Format_X8R8G8B8    &&
        surface->Format != Format_X8B8G8R8    &&
        surface->Format != Format_P8)
    {
        VPHAL_MEMORY_DECOMP_NORMALMESSAGE("Unsupported Source Format '0x%08x' for VEBOX Decompression.", surface->Format);
        goto finish;
    }

    bRet = true;

finish:
    return bRet;
}
