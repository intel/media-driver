/*
* Copyright (c) 2020-2022, Intel Corporation
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
//! \file     media_vebox_copy.cpp
//! \brief    Common Copy interface and structure used in Vebox Engine
//! \details  Common Copy interface and structure used in Vebox Engine

#include <stdint.h>
#include "media_vebox_copy.h"
#include "renderhal_legacy.h"
#include "mhw_vebox_itf.h"
#include "mhw_mi.h"
#include "mhw_utilities.h"
#include "mhw_utilities_next.h"
#include "mos_defs_specific.h"
#include "mos_os_cp_interface_specific.h"
#include "mos_resource_defs.h"
#include "mos_utilities.h"
#include "renderhal.h"
#include "hal_oca_interface.h"

#define SURFACE_DW_UY_OFFSET(pSurface) \
    ((pSurface) != nullptr ? ((pSurface)->UPlaneOffset.iSurfaceOffset - (pSurface)->dwOffset) / (pSurface)->dwPitch + (pSurface)->UPlaneOffset.iYOffset : 0)

#define SURFACE_DW_VY_OFFSET(pSurface) \
    ((pSurface) != nullptr ? ((pSurface)->VPlaneOffset.iSurfaceOffset - (pSurface)->dwOffset) / (pSurface)->dwPitch + (pSurface)->VPlaneOffset.iYOffset : 0)

VeboxCopyState::VeboxCopyState(PMOS_INTERFACE osInterface, MhwInterfaces* mhwInterfaces) :
    m_osInterface(osInterface),
    m_mhwInterfaces(nullptr),
    m_miInterface(nullptr),
    m_veboxInterface(nullptr),
    m_cpInterface(nullptr)
{
    m_veboxInterface = mhwInterfaces->m_veboxInterface;
    m_miInterface = mhwInterfaces->m_miInterface;
    m_cpInterface = mhwInterfaces->m_cpInterface;
}

VeboxCopyState::~VeboxCopyState()
{
    if (m_veboxInterface)
    {
            m_veboxInterface->DestroyHeap();
            m_veboxInterface = nullptr;
    }
}

MOS_STATUS VeboxCopyState::Initialize()
{
    MHW_VEBOX_GPUNODE_LIMIT     GpuNodeLimit;
    MOS_GPU_NODE                VeboxGpuNode;
    MOS_GPU_CONTEXT             VeboxGpuContext;

    if (m_veboxInterface)
    {
        if (m_veboxInterface->m_veboxHeap == nullptr)
        {
            VEBOX_COPY_CHK_STATUS_RETURN(m_veboxInterface->CreateHeap());
        }
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VeboxCopyState::CopyMainSurface(PMOS_SURFACE src, PMOS_SURFACE dst)
{
    VEBOX_COPY_CHK_NULL_RETURN(src);
    VEBOX_COPY_CHK_NULL_RETURN(dst);
    return CopyMainSurface(&src->OsResource, &dst->OsResource);
}

MOS_STATUS VeboxCopyState::CopyMainSurface(PMOS_RESOURCE src, PMOS_RESOURCE dst)
{
    MOS_STATUS                          eStatus = MOS_STATUS_SUCCESS;
    MHW_VEBOX_STATE_CMD_PARAMS          veboxStateCmdParams;
    MOS_COMMAND_BUFFER                  cmdBuffer;
    MhwVeboxInterface                   *veboxInterface;
    MHW_VEBOX_SURFACE_STATE_CMD_PARAMS  mhwVeboxSurfaceStateCmdParams;
    MHW_MI_FLUSH_DW_PARAMS              flushDwParams;
    uint32_t                            streamID = 0;
    const MHW_VEBOX_HEAP                *veboxHeap = nullptr;
    MOS_SURFACE inputSurface, outputSurface;

    VEBOX_COPY_CHK_NULL_RETURN(src);
    VEBOX_COPY_CHK_NULL_RETURN(dst);

    // Get input resource info
    MOS_ZeroMemory(&inputSurface, sizeof(MOS_SURFACE));
    inputSurface.OsResource = *src;
    VEBOX_COPY_CHK_STATUS_RETURN(GetResourceInfo(&inputSurface));

    // Get output resource info
    MOS_ZeroMemory(&outputSurface, sizeof(MOS_SURFACE));
    outputSurface.OsResource = *dst;
    VEBOX_COPY_CHK_STATUS_RETURN(GetResourceInfo(&outputSurface));

    // For RGB10/BGR10/Y210/Y410/A8, use other format instead. No need to check format again.
    AdjustSurfaceFormat(inputSurface);

    veboxInterface = m_veboxInterface;

    MOS_GPUCTX_CREATOPTIONS_ENHANCED      createOption = {};
    // no gpucontext will be created if the gpu context has been created before.
    VEBOX_COPY_CHK_STATUS_RETURN(m_osInterface->pfnCreateGpuContext(
        m_osInterface,
        MOS_GPU_CONTEXT_VEBOX,
        MOS_GPU_NODE_VE,
        &createOption));

    VEBOX_COPY_CHK_STATUS_RETURN(m_osInterface->pfnSetGpuContext(m_osInterface, MOS_GPU_CONTEXT_VEBOX));
 
    // Register Vebox GPU context with the Batch Buffer completion event
    VEBOX_COPY_CHK_STATUS_RETURN(m_osInterface->pfnRegisterBBCompleteNotifyEvent(
        m_osInterface,
        MOS_GPU_CONTEXT_VEBOX));

    // Reset allocation list and house keeping
    m_osInterface->pfnResetOsStates(m_osInterface);

    VEBOX_COPY_CHK_STATUS_RETURN(veboxInterface->GetVeboxHeapInfo(&veboxHeap));
    VEBOX_COPY_CHK_NULL_RETURN(m_osInterface->osCpInterface);

    //there is a new usage that input surface is clear and output surface is secure.
    //replace Huc Copy by DoubleBuffer resolve to update ccs data.
    //So need consolidate both input/output surface information to decide cp context.
     PMOS_RESOURCE surfaceArray[2];
     surfaceArray[0] = src;
     surfaceArray[1] = dst;

    // preprocess in cp first
    m_osInterface->osCpInterface->PrepareResources((void **)&surfaceArray, sizeof(surfaceArray) / sizeof(PMOS_RESOURCE), nullptr, 0);
    m_osInterface->pfnSetPerfTag(m_osInterface,VEBOX_COPY);
    // initialize the command buffer struct
    MOS_ZeroMemory(&cmdBuffer, sizeof(MOS_COMMAND_BUFFER));

    VEBOX_COPY_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));
    VEBOX_COPY_CHK_STATUS_RETURN(InitCommandBuffer(&cmdBuffer));

    HalOcaInterface::On1stLevelBBStart(cmdBuffer, *m_osInterface->pOsContext, m_osInterface->CurrentGpuContextHandle, *m_miInterface, 
        *m_miInterface->GetMmioRegisters());

    MediaPerfProfiler* perfProfiler = MediaPerfProfiler::Instance();
    VEBOX_COPY_CHK_NULL_RETURN(perfProfiler);
    VEBOX_COPY_CHK_STATUS_RETURN(perfProfiler->AddPerfCollectStartCmd((void*)this, m_osInterface, m_miInterface, &cmdBuffer));
    // Set Vebox Aux MMIO
    VEBOX_COPY_CHK_STATUS_RETURN(m_veboxInterface->setVeboxPrologCmd(m_miInterface, &cmdBuffer));

    // Prepare Vebox_Surface_State, surface input/and output are the same but the compressed status.
    VEBOX_COPY_CHK_STATUS_RETURN(SetupVeboxSurfaceState(&mhwVeboxSurfaceStateCmdParams, &inputSurface, &outputSurface));

    //---------------------------------
    // Send CMD: Vebox_Surface_State
    //---------------------------------
    VEBOX_COPY_CHK_STATUS_RETURN(veboxInterface->AddVeboxSurfaces(
        &cmdBuffer,
        &mhwVeboxSurfaceStateCmdParams));

    HalOcaInterface::OnDispatch(cmdBuffer, *m_osInterface, *m_miInterface, *m_miInterface->GetMmioRegisters());

    //---------------------------------
    // Send CMD: Vebox_Tiling_Convert
    //---------------------------------
    VEBOX_COPY_CHK_STATUS_RETURN(m_veboxInterface->AddVeboxTilingConvert(&cmdBuffer, &mhwVeboxSurfaceStateCmdParams.SurfInput, &mhwVeboxSurfaceStateCmdParams.SurfOutput));
    VEBOX_COPY_CHK_STATUS_RETURN(perfProfiler->AddPerfCollectEndCmd((void*)this, m_osInterface, m_miInterface, &cmdBuffer));

    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));

    VEBOX_COPY_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(
        &cmdBuffer,
        &flushDwParams));

    if (!m_osInterface->bEnableKmdMediaFrameTracking && veboxHeap)
    {
        MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
        flushDwParams.pOsResource = (PMOS_RESOURCE)&veboxHeap->DriverResource;
        flushDwParams.dwResourceOffset = veboxHeap->uiOffsetSync;
        flushDwParams.dwDataDW1 = veboxHeap->dwNextTag;
        VEBOX_COPY_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(
            &cmdBuffer,
            &flushDwParams));
    }

    VEBOX_COPY_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(
        &cmdBuffer,
        nullptr));

    HalOcaInterface::On1stLevelBBEnd(cmdBuffer, *m_osInterface);

    // Return unused command buffer space to OS
    m_osInterface->pfnReturnCommandBuffer(
        m_osInterface,
        &cmdBuffer,
        0);

    // Flush the command buffer
    VEBOX_COPY_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(
        m_osInterface,
        &cmdBuffer,
        false));

    veboxInterface->UpdateVeboxSync();

    return eStatus;
}

bool VeboxCopyState::IsSurfaceSupported(PMOS_RESOURCE surface)
{
    bool supported = false;
    MOS_SURFACE inputSurface;

    if (!surface)
    {
        return false;
    }

    // Get input resource info
    MOS_ZeroMemory(&inputSurface, sizeof(MOS_SURFACE));
    inputSurface.OsResource = *surface;
    GetResourceInfo(&inputSurface);

    supported = IsVeCopySupportedFormat(inputSurface.Format);

    if (inputSurface.TileType == MOS_TILE_LINEAR &&
        (inputSurface.dwPitch % 64))
    {
        supported = false;
    }

    return supported;
}

MOS_STATUS VeboxCopyState::GetResourceInfo(PMOS_SURFACE surface)
{
 MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;

    VEBOX_COPY_CHK_NULL_RETURN(m_osInterface);
    VEBOX_COPY_CHK_NULL_RETURN(surface);

    MOS_SURFACE resDetails;
    MOS_ZeroMemory(&resDetails, sizeof(resDetails));
    resDetails.Format = Format_Invalid;

    VEBOX_COPY_CHK_STATUS_RETURN(m_osInterface->pfnGetResourceInfo(
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
    surface->TileModeGMM                                        = resDetails.TileModeGMM;
    surface->bGMMTileEnabled                                    = resDetails.bGMMTileEnabled;
    surface->bCompressible                                      = resDetails.bCompressible;
    surface->bIsCompressed                                      = resDetails.bIsCompressed;
    surface->dwOffset                                           = resDetails.RenderOffset.YUV.Y.BaseOffset;
    surface->YPlaneOffset.iSurfaceOffset                        = resDetails.RenderOffset.YUV.Y.BaseOffset;
    surface->YPlaneOffset.iXOffset                              = resDetails.RenderOffset.YUV.Y.XOffset;
    surface->YPlaneOffset.iYOffset                              = resDetails.RenderOffset.YUV.Y.YOffset;
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

MOS_STATUS VeboxCopyState::SetupVeboxSurfaceState(
    PMHW_VEBOX_SURFACE_STATE_CMD_PARAMS mhwVeboxSurfaceStateCmdParams,
    PMOS_SURFACE                        inputSurface,
    PMOS_SURFACE                        outputSurface)
{
    MOS_STATUS              eStatus = MOS_STATUS_SUCCESS;
    bool                    inputIsLinearBuffer = false;
    bool                    outputIsLinearBuffer = false;
    uint32_t                bpp = 1;
    uint32_t                inputWidth = 0;
    uint32_t                outputWidth = 0;

    VEBOX_COPY_CHK_NULL_RETURN(inputSurface);
    VEBOX_COPY_CHK_NULL_RETURN(mhwVeboxSurfaceStateCmdParams);

    MOS_ZeroMemory(mhwVeboxSurfaceStateCmdParams, sizeof(*mhwVeboxSurfaceStateCmdParams));

    mhwVeboxSurfaceStateCmdParams->SurfInput.bActive    = mhwVeboxSurfaceStateCmdParams->SurfOutput.bActive    = true;
    mhwVeboxSurfaceStateCmdParams->SurfInput.dwBitDepth = mhwVeboxSurfaceStateCmdParams->SurfOutput.dwBitDepth = inputSurface->dwDepth;
    mhwVeboxSurfaceStateCmdParams->SurfInput.dwHeight   = mhwVeboxSurfaceStateCmdParams->SurfOutput.dwHeight   = inputSurface->dwHeight;
    mhwVeboxSurfaceStateCmdParams->SurfInput.dwWidth    = mhwVeboxSurfaceStateCmdParams->SurfOutput.dwWidth    = inputSurface->dwWidth;
    mhwVeboxSurfaceStateCmdParams->SurfInput.Format     = mhwVeboxSurfaceStateCmdParams->SurfOutput.Format     = inputSurface->Format;

    MOS_SURFACE inputDetails, outputDetails;
    MOS_ZeroMemory(&inputDetails, sizeof(inputDetails));
    MOS_ZeroMemory(&outputDetails, sizeof(outputDetails));
    inputDetails.Format = Format_Invalid;
    outputDetails.Format = Format_Invalid;

    if (inputSurface)
    {
        VEBOX_COPY_CHK_STATUS_RETURN(m_osInterface->pfnGetResourceInfo(
            m_osInterface,
            &inputSurface->OsResource,
            &inputDetails));
    }

    if (outputSurface)
    {
        VEBOX_COPY_CHK_STATUS_RETURN(m_osInterface->pfnGetResourceInfo(
            m_osInterface,
            &outputSurface->OsResource,
            &outputDetails));

        // Following settings are enabled only when outputSurface is availble
        inputIsLinearBuffer  = (inputDetails.dwHeight == 1) ? true : false;
        outputIsLinearBuffer = (outputDetails.dwHeight == 1) ? true : false;

        inputWidth = inputSurface->dwWidth;
        outputWidth = outputSurface->dwWidth;

        if (inputIsLinearBuffer)
        {
            bpp = outputDetails.dwPitch / outputDetails.dwWidth;
            if (outputDetails.dwPitch % outputDetails.dwWidth != 0)
            {
                inputWidth = outputDetails.dwPitch / bpp;
            }
        }
        else if (outputIsLinearBuffer)
        {
            bpp = inputDetails.dwPitch / inputDetails.dwWidth;
            if (inputDetails.dwPitch % inputDetails.dwWidth != 0)
            {
                outputWidth = inputDetails.dwPitch / bpp;
            }
        }
        else
        {
            VEBOX_COPY_NORMALMESSAGE("2D to 2D, no need for bpp setting.");
        }
    }


    if (inputSurface->dwPitch > 0            &&
       (inputSurface->Format == Format_P010  ||
        inputSurface->Format == Format_P016  ||
        inputSurface->Format == Format_NV12))
    {
        mhwVeboxSurfaceStateCmdParams->SurfInput.dwUYoffset = (!inputIsLinearBuffer) ? SURFACE_DW_UY_OFFSET(inputSurface) :
                                                              inputSurface->dwHeight;

        if (outputSurface)
        {
            mhwVeboxSurfaceStateCmdParams->SurfOutput.dwUYoffset = (!outputIsLinearBuffer) ? SURFACE_DW_UY_OFFSET(outputSurface) :
                                                                   outputSurface->dwHeight;
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
        mhwVeboxSurfaceStateCmdParams->SurfInput.TileModeGMM = mhwVeboxSurfaceStateCmdParams->SurfOutput.TileModeGMM = inputSurface->TileModeGMM;
        mhwVeboxSurfaceStateCmdParams->SurfInput.bGMMTileEnabled = mhwVeboxSurfaceStateCmdParams->SurfOutput.bGMMTileEnabled = inputSurface->bGMMTileEnabled;
        mhwVeboxSurfaceStateCmdParams->SurfOutput.dwPitch    = mhwVeboxSurfaceStateCmdParams->SurfInput.dwPitch      = inputSurface->dwPitch;
        mhwVeboxSurfaceStateCmdParams->SurfInput.pOsResource = mhwVeboxSurfaceStateCmdParams->SurfOutput.pOsResource = &(inputSurface->OsResource);
        mhwVeboxSurfaceStateCmdParams->SurfInput.dwYoffset   = mhwVeboxSurfaceStateCmdParams->SurfOutput.dwYoffset   = inputSurface->YPlaneOffset.iYOffset;
        mhwVeboxSurfaceStateCmdParams->SurfInput.dwOffset    = mhwVeboxSurfaceStateCmdParams->SurfOutput.dwOffset    = inputSurface->dwOffset;

        mhwVeboxSurfaceStateCmdParams->SurfInput.dwCompressionFormat = mhwVeboxSurfaceStateCmdParams->SurfOutput.dwCompressionFormat =
            inputSurface->CompressionFormat;
        mhwVeboxSurfaceStateCmdParams->SurfInput.CompressionMode  = inputSurface->CompressionMode;
        mhwVeboxSurfaceStateCmdParams->SurfOutput.CompressionMode = MOS_MMC_DISABLED;
    }
    else
    // double buffer resolve
    {
        mhwVeboxSurfaceStateCmdParams->SurfInput.TileType             = inputSurface->TileType;
        mhwVeboxSurfaceStateCmdParams->SurfInput.TileModeGMM          = inputSurface->TileModeGMM;
        mhwVeboxSurfaceStateCmdParams->SurfInput.bGMMTileEnabled      = inputSurface->bGMMTileEnabled;
        mhwVeboxSurfaceStateCmdParams->SurfOutput.TileType            = outputSurface->TileType;
        mhwVeboxSurfaceStateCmdParams->SurfOutput.TileModeGMM         = outputSurface->TileModeGMM;
        mhwVeboxSurfaceStateCmdParams->SurfOutput.bGMMTileEnabled     = outputSurface->bGMMTileEnabled;
        mhwVeboxSurfaceStateCmdParams->SurfInput.dwOffset             = inputSurface->dwOffset;
        mhwVeboxSurfaceStateCmdParams->SurfOutput.dwOffset            = outputSurface->dwOffset;

        // When surface is 1D but processed as 2D, fake a min(pitch, width) is needed as the pitch API passed may less surface width in 1D surface
        mhwVeboxSurfaceStateCmdParams->SurfInput.dwPitch              = (inputIsLinearBuffer) ?
                                                                         MOS_MIN(inputWidth * bpp, inputSurface->dwPitch) : inputSurface->dwPitch;
        mhwVeboxSurfaceStateCmdParams->SurfOutput.dwPitch             = (outputIsLinearBuffer) ?
                                                                         MOS_MIN(outputWidth * bpp, outputSurface->dwPitch) : outputSurface->dwPitch;
        mhwVeboxSurfaceStateCmdParams->SurfInput.pOsResource          = &(inputSurface->OsResource);
        mhwVeboxSurfaceStateCmdParams->SurfOutput.pOsResource         = &(outputSurface->OsResource);
        mhwVeboxSurfaceStateCmdParams->SurfInput.dwYoffset            = inputSurface->YPlaneOffset.iYOffset;
        mhwVeboxSurfaceStateCmdParams->SurfOutput.dwYoffset           = outputSurface->YPlaneOffset.iYOffset;
        mhwVeboxSurfaceStateCmdParams->SurfInput.dwCompressionFormat  = inputSurface->CompressionFormat;
        mhwVeboxSurfaceStateCmdParams->SurfOutput.dwCompressionFormat = outputSurface->CompressionFormat;
        mhwVeboxSurfaceStateCmdParams->SurfInput.CompressionMode      = inputSurface->CompressionMode;
        mhwVeboxSurfaceStateCmdParams->SurfOutput.CompressionMode     = outputSurface->CompressionMode;
    }

    return eStatus;
}

MOS_STATUS VeboxCopyState::InitCommandBuffer(PMOS_COMMAND_BUFFER cmdBuffer)
{
    PMOS_INTERFACE              pOsInterface;
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;
    uint32_t                    iRemaining;
    RENDERHAL_GENERIC_PROLOG_PARAMS         GenericPrologParams = {};
    PMOS_RESOURCE                           gpuStatusBuffer = nullptr;

    //---------------------------------------------
    VEBOX_COPY_CHK_NULL_RETURN(cmdBuffer);
    VEBOX_COPY_CHK_NULL_RETURN(m_osInterface);
    VEBOX_COPY_CHK_NULL_RETURN(m_miInterface);
    //---------------------------------------------

    eStatus = MOS_STATUS_SUCCESS;
    pOsInterface = m_osInterface;
    MOS_ZeroMemory(&GenericPrologParams, sizeof(RENDERHAL_GENERIC_PROLOG_PARAMS));

    MOS_GPU_CONTEXT gpuContext = m_osInterface->pfnGetGpuContext(m_osInterface);

#ifndef EMUL
    if (pOsInterface->bEnableKmdMediaFrameTracking)
    {
        // Get GPU Status buffer
        VEBOX_COPY_CHK_STATUS_RETURN(pOsInterface->pfnGetGpuStatusBufferResource(pOsInterface, gpuStatusBuffer));
        VEBOX_COPY_CHK_NULL_RETURN(gpuStatusBuffer);
        // Register the buffer
        VEBOX_COPY_CHK_STATUS_RETURN(pOsInterface->pfnRegisterResource(pOsInterface, gpuStatusBuffer, true, true));

        GenericPrologParams.bEnableMediaFrameTracking = true;
        GenericPrologParams.presMediaFrameTrackingSurface = gpuStatusBuffer;
        GenericPrologParams.dwMediaFrameTrackingTag = pOsInterface->pfnGetGpuStatusTag(pOsInterface, pOsInterface->CurrentGpuContextOrdinal);
        GenericPrologParams.dwMediaFrameTrackingAddrOffset = pOsInterface->pfnGetGpuStatusTagOffset(pOsInterface, pOsInterface->CurrentGpuContextOrdinal);

        // Increment GPU Status Tag
        pOsInterface->pfnIncrementGpuStatusTag(pOsInterface, pOsInterface->CurrentGpuContextOrdinal);
    }
#endif

    if (GenericPrologParams.bEnableMediaFrameTracking)
    {
        VEBOX_COPY_CHK_NULL_RETURN(GenericPrologParams.presMediaFrameTrackingSurface);
        cmdBuffer->Attributes.bEnableMediaFrameTracking = GenericPrologParams.bEnableMediaFrameTracking;
        cmdBuffer->Attributes.dwMediaFrameTrackingTag = GenericPrologParams.dwMediaFrameTrackingTag;
        cmdBuffer->Attributes.dwMediaFrameTrackingAddrOffset = GenericPrologParams.dwMediaFrameTrackingAddrOffset;
        cmdBuffer->Attributes.resMediaFrameTrackingSurface = GenericPrologParams.presMediaFrameTrackingSurface;
    }

    // initialize command buffer attributes
    cmdBuffer->Attributes.bTurboMode = false;
    cmdBuffer->Attributes.bMediaPreemptionEnabled = false;
    cmdBuffer->Attributes.dwMediaFrameTrackingAddrOffset = 0;

    MHW_GENERIC_PROLOG_PARAMS genericPrologParams;
    MOS_ZeroMemory(&genericPrologParams, sizeof(genericPrologParams));
    genericPrologParams.pOsInterface = m_osInterface;
    genericPrologParams.pvMiInterface = m_miInterface;
    genericPrologParams.bMmcEnabled = true;

    VEBOX_COPY_CHK_STATUS_RETURN(Mhw_SendGenericPrologCmd(
        cmdBuffer,
        &genericPrologParams));

    return eStatus;
}

bool VeboxCopyState::IsVeCopySupportedFormat(MOS_FORMAT format)
{
    if (format == Format_R10G10B10A2 ||
        format == Format_B10G10R10A2 ||
        format == Format_A8R8G8B8 ||
        format == Format_A8B8G8R8 ||
        format == Format_X8R8G8B8 ||
        format == Format_X8B8G8R8 ||
        IS_RGB64_FLOAT_FORMAT(format) ||

        format == Format_AYUV ||
        format == Format_Y410 ||
        format == Format_Y416 ||
        format == Format_Y210 ||
        format == Format_Y216 ||
        format == Format_YUY2 ||
        format == Format_NV12 ||
        format == Format_P010 ||
        format == Format_P016 ||

        format == Format_A8 ||
        format == Format_Y8 ||
        format == Format_L8 ||
        format == Format_P8 ||
        format == Format_Y16U)
    {
        return true;
    }
    else
    {
        VEBOX_COPY_NORMALMESSAGE("Unsupported format '0x%08x' for VEBOX copy.", format);
        return false;
    }
}

void VeboxCopyState::AdjustSurfaceFormat(MOS_SURFACE &surface)
{
    if (surface.Format == Format_R10G10B10A2 ||
        surface.Format == Format_B10G10R10A2 ||
        surface.Format == Format_Y410        ||
        surface.Format == Format_Y210)
    {
        // RGB10 not supported without IECP. Re-map RGB10/RGB10 as AYUV
        // Y410/Y210 has HW issue. Remap to AYUV.
        surface.Format = Format_AYUV;
    }
    else if (surface.Format == Format_A8)
    {
        surface.Format = Format_P8;
    }
}
