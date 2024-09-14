/*
* Copyright (c) 2022, Intel Corporation
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
//! \file     media_vebox_copy_next.cpp
//! \brief    Common Copy interface and structure used in Vebox Engine
//! \details  Common Copy interface and structure used in Vebox Engine
#include "media_vebox_copy_next.h"
#include "renderhal.h"
#include "mhw_vebox_itf.h"
#include "mos_os_cp_interface_specific.h"
#include "media_copy_common.h"
#include "hal_oca_interface_next.h"

#define SURFACE_DW_UY_OFFSET(pSurface) \
    ((pSurface) != nullptr ? ((pSurface)->UPlaneOffset.iSurfaceOffset - (pSurface)->dwOffset) / (pSurface)->dwPitch + (pSurface)->UPlaneOffset.iYOffset : 0)

#define SURFACE_DW_VY_OFFSET(pSurface) \
    ((pSurface) != nullptr ? ((pSurface)->VPlaneOffset.iSurfaceOffset - (pSurface)->dwOffset) / (pSurface)->dwPitch + (pSurface)->VPlaneOffset.iYOffset : 0)

VeboxCopyStateNext::VeboxCopyStateNext(PMOS_INTERFACE osInterface) :
    m_osInterface(osInterface),
    m_mhwInterfaces(nullptr),
    m_cpInterface(nullptr)
{
    MOS_ZeroMemory(&params, sizeof(params));
    params.Flags.m_vebox = 1;
    m_mhwInterfaces = MhwInterfacesNext::CreateFactory(params, osInterface);
    if (m_mhwInterfaces != nullptr)
    {
        m_miItf    = m_mhwInterfaces->m_miItf;
        m_veboxItf = m_mhwInterfaces->m_veboxItf;
    }
}

VeboxCopyStateNext::VeboxCopyStateNext(PMOS_INTERFACE osInterface, MhwInterfacesNext* mhwInterfaces) :
    m_osInterface(osInterface),
    m_mhwInterfaces(nullptr),
    m_cpInterface(nullptr)
{   
    m_cpInterface = mhwInterfaces->m_cpInterface;
    m_miItf       = mhwInterfaces->m_miItf;
    m_veboxItf    = mhwInterfaces->m_veboxItf;
}

VeboxCopyStateNext::~VeboxCopyStateNext()
{
    if (m_veboxItf != nullptr)
    {
        m_veboxItf->DestroyHeap();
    }

    if(m_mhwInterfaces != nullptr)
    {
        m_mhwInterfaces->Destroy();
        MOS_Delete(m_mhwInterfaces);
    }  
}

MOS_STATUS VeboxCopyStateNext::Initialize()
{
    VEBOX_COPY_CHK_NULL_RETURN(m_veboxItf);
    const MHW_VEBOX_HEAP* veboxHeap = nullptr;
    m_veboxItf->GetVeboxHeapInfo(&veboxHeap);

    if (veboxHeap == nullptr)
    {
        m_veboxItf->CreateHeap();
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VeboxCopyStateNext::CopyMainSurface(PMOS_SURFACE src, PMOS_SURFACE dst)
{
    VEBOX_COPY_CHK_NULL_RETURN(src);
    VEBOX_COPY_CHK_NULL_RETURN(dst);
    return CopyMainSurface(&src->OsResource, &dst->OsResource);
}

MOS_STATUS VeboxCopyStateNext::CopyMainSurface(PMOS_RESOURCE src, PMOS_RESOURCE dst)
{
    MOS_STATUS                          eStatus = MOS_STATUS_SUCCESS;
    MHW_VEBOX_STATE_CMD_PARAMS          veboxStateCmdParams;
    MOS_COMMAND_BUFFER                  cmdBuffer;
    MHW_VEBOX_SURFACE_STATE_CMD_PARAMS  mhwVeboxSurfaceStateCmdParams;
    uint32_t                            streamID = 0;
    const MHW_VEBOX_HEAP                *veboxHeap = nullptr;
    MOS_SURFACE inputSurface, outputSurface;

    VEBOX_COPY_CHK_NULL_RETURN(src);
    VEBOX_COPY_CHK_NULL_RETURN(dst);
    VEBOX_COPY_CHK_NULL_RETURN(m_miItf);
    VEBOX_COPY_CHK_NULL_RETURN(m_veboxItf);

    // Get input resource info
    MOS_ZeroMemory(&inputSurface, sizeof(MOS_SURFACE));
    inputSurface.OsResource = *src;
    GetResourceInfo(&inputSurface);

    // Get output resource info
    MOS_ZeroMemory(&outputSurface, sizeof(MOS_SURFACE));
    outputSurface.OsResource = *dst;
    GetResourceInfo(&outputSurface);

    // For RGB10/BGR10/Y210/Y410/A8, use other format instead. No need to check format again.
    AdjustSurfaceFormat(inputSurface);

    MHW_VEBOX_GPUNODE_LIMIT     GpuNodeLimit;
    MOS_GPU_NODE                VeboxGpuNode;
    MOS_GPU_CONTEXT             VeboxGpuContext;
    GpuNodeLimit.bCpEnabled = (m_osInterface->osCpInterface->IsCpEnabled()) ? true : false;
    VEBOX_COPY_CHK_STATUS_RETURN(m_veboxItf->FindVeboxGpuNodeToUse(&GpuNodeLimit));
    VeboxGpuNode = (MOS_GPU_NODE)(GpuNodeLimit.dwGpuNodeToUse);
    VeboxGpuContext = (VeboxGpuNode == MOS_GPU_NODE_VE) ? MOS_GPU_CONTEXT_VEBOX : MOS_GPU_CONTEXT_VEBOX2;
    // Create VEBOX/VEBOX2 Context
    VEBOX_COPY_CHK_STATUS_RETURN(CreateGpuContext(
        m_osInterface,
        VeboxGpuContext,
        VeboxGpuNode));

    // Register Vebox GPU context with the Batch Buffer completion event
    VEBOX_COPY_CHK_STATUS_RETURN(m_osInterface->pfnRegisterBBCompleteNotifyEvent(
        m_osInterface,
        VeboxGpuContext));

    VEBOX_COPY_CHK_STATUS_RETURN(m_osInterface->pfnSetGpuContext(m_osInterface, VeboxGpuContext));

    m_osInterface->pfnSetPerfTag(m_osInterface, VEBOX_COPY);

    // Reset allocation list and house keeping
    m_osInterface->pfnResetOsStates(m_osInterface);

    VEBOX_COPY_CHK_STATUS_RETURN(m_veboxItf->GetVeboxHeapInfo(&veboxHeap));
    VEBOX_COPY_CHK_NULL_RETURN(m_osInterface->osCpInterface);

    //there is a new usage that input surface is clear and output surface is secure.
    //replace Huc Copy by DoubleBuffer resolve to update ccs data.
    //So need consolidate both input/output surface information to decide cp context.
     PMOS_RESOURCE surfaceArray[2];
     surfaceArray[0] = src;
     surfaceArray[1] = dst;

    // preprocess in cp first
     VEBOX_COPY_CHK_STATUS_RETURN(
         m_osInterface->osCpInterface->PrepareResources((void **)&surfaceArray, sizeof(surfaceArray) / sizeof(PMOS_RESOURCE), nullptr, 0));

    // initialize the command buffer struct
    MOS_ZeroMemory(&cmdBuffer, sizeof(MOS_COMMAND_BUFFER));

    VEBOX_COPY_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));
    VEBOX_COPY_CHK_STATUS_RETURN(InitCommandBuffer(&cmdBuffer));

    HalOcaInterfaceNext::On1stLevelBBStart(cmdBuffer, m_osInterface->pOsContext, m_osInterface->CurrentGpuContextHandle, m_miItf, *m_miItf->GetMmioRegisters());

    MediaPerfProfiler* perfProfiler = MediaPerfProfiler::Instance();
    VEBOX_COPY_CHK_NULL_RETURN(perfProfiler);
    VEBOX_COPY_CHK_STATUS_RETURN(perfProfiler->AddPerfCollectStartCmd((void*)this, m_osInterface, m_miItf, &cmdBuffer));
    VEBOX_COPY_CHK_STATUS_RETURN(NullHW::StartPredicateNext(m_osInterface, m_miItf, &cmdBuffer));
    // Set Vebox MMIO
    VEBOX_COPY_CHK_STATUS_RETURN(m_miItf->AddVeboxMMIOPrologCmd(&cmdBuffer));

    // Prepare Vebox_Surface_State, surface input/and output are the same but the compressed status.
    VEBOX_COPY_CHK_STATUS_RETURN(SetupVeboxSurfaceState(&mhwVeboxSurfaceStateCmdParams, &inputSurface, &outputSurface));

    //---------------------------------
    // Send CMD: Vebox_Surface_State
    //---------------------------------
    VEBOX_COPY_CHK_STATUS_RETURN(m_veboxItf->AddVeboxSurfaces(
        &cmdBuffer,
        &mhwVeboxSurfaceStateCmdParams));

    HalOcaInterfaceNext::OnDispatch(cmdBuffer, *m_osInterface, m_miItf, *m_miItf->GetMmioRegisters());

    //---------------------------------
    // Send CMD: Vebox_Tiling_Convert
    //---------------------------------
    VEBOX_COPY_CHK_STATUS_RETURN(m_veboxItf->AddVeboxTilingConvert(&cmdBuffer, &mhwVeboxSurfaceStateCmdParams.SurfInput, &mhwVeboxSurfaceStateCmdParams.SurfOutput));

    auto& flushDwParams = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
    flushDwParams = {};
    VEBOX_COPY_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(&cmdBuffer));

    if (!m_osInterface->bEnableKmdMediaFrameTracking && veboxHeap)
    {
        flushDwParams = {};
        flushDwParams.pOsResource = (PMOS_RESOURCE)&veboxHeap->DriverResource;
        flushDwParams.dwResourceOffset = veboxHeap->uiOffsetSync;
        flushDwParams.dwDataDW1 = veboxHeap->dwNextTag;

        auto skuTable = m_osInterface->pfnGetSkuTable(m_osInterface);
        if (skuTable && MEDIA_IS_SKU(skuTable, FtrEnablePPCFlush))
        {
            flushDwParams.bEnablePPCFlush = true;
        }
        VEBOX_COPY_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(&cmdBuffer));
    }
    VEBOX_COPY_CHK_STATUS_RETURN(NullHW::StopPredicateNext(m_osInterface, m_miItf, &cmdBuffer));
    VEBOX_COPY_CHK_STATUS_RETURN(perfProfiler->AddPerfCollectEndCmd((void*)this, m_osInterface, m_miItf, &cmdBuffer));

    HalOcaInterfaceNext::On1stLevelBBEnd(cmdBuffer, *m_osInterface);

    VEBOX_COPY_CHK_STATUS_RETURN(m_miItf->AddMiBatchBufferEnd(&cmdBuffer, nullptr));

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

    m_veboxItf->UpdateVeboxSync();

    return eStatus;
}

bool VeboxCopyStateNext::IsSurfaceSupported(PMOS_RESOURCE surface)
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

MOS_STATUS VeboxCopyStateNext::GetResourceInfo(PMOS_SURFACE surface)
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
    surface->dwOffset                                           = resDetails.RenderOffset.YUV.Y.BaseOffset + surface->OsResource.dwOffsetForMono;
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

MOS_STATUS VeboxCopyStateNext::SetupVeboxSurfaceState(
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
    mhwVeboxSurfaceStateCmdParams->SurfInput.dwHeight   = mhwVeboxSurfaceStateCmdParams->SurfOutput.dwHeight   = 
        MOS_MIN(inputSurface->dwHeight, ((outputSurface!= nullptr) ? outputSurface->dwHeight : inputSurface->dwHeight));
    mhwVeboxSurfaceStateCmdParams->SurfInput.dwWidth    = mhwVeboxSurfaceStateCmdParams->SurfOutput.dwWidth    = 
        MOS_MIN(inputSurface->dwWidth, ((outputSurface != nullptr) ? outputSurface->dwWidth : inputSurface->dwWidth));
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

MOS_STATUS VeboxCopyStateNext::InitCommandBuffer(PMOS_COMMAND_BUFFER cmdBuffer)
{
    PMOS_INTERFACE              pOsInterface;
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;
    uint32_t                    iRemaining;
    RENDERHAL_GENERIC_PROLOG_PARAMS         GenericPrologParams = {};
    PMOS_RESOURCE                           gpuStatusBuffer = nullptr;

    //---------------------------------------------
    VEBOX_COPY_CHK_NULL_RETURN(cmdBuffer);
    VEBOX_COPY_CHK_NULL_RETURN(m_osInterface);
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
    genericPrologParams.bMmcEnabled = true;

    VEBOX_COPY_CHK_STATUS_RETURN(Mhw_SendGenericPrologCmdNext(
        cmdBuffer,
        &genericPrologParams,
        m_miItf));

    return eStatus;
}

bool VeboxCopyStateNext::IsVeCopySupportedFormat(MOS_FORMAT format)
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

void VeboxCopyStateNext::AdjustSurfaceFormat(MOS_SURFACE &surface)
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

MOS_STATUS VeboxCopyStateNext::CreateGpuContext(
    PMOS_INTERFACE  pOsInterface,
    MOS_GPU_CONTEXT VeboxGpuContext,
    MOS_GPU_NODE    VeboxGpuNode)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_CHK_NULL_RETURN(pOsInterface);

    if (!MOS_VE_CTXBASEDSCHEDULING_SUPPORTED(pOsInterface))
    {
        MOS_GPUCTX_CREATOPTIONS createOption;
        // Create VEBOX/VEBOX2 Context
        MHW_CHK_STATUS_RETURN(pOsInterface->pfnCreateGpuContext(
            pOsInterface,
            VeboxGpuContext,
            VeboxGpuNode,
            &createOption));
    }
    else
    {
        MOS_GPUCTX_CREATOPTIONS_ENHANCED createOptionenhanced;
        // vebox copy always uses 1 vebox to copy each frame.
        createOptionenhanced.LRCACount = 1;

        // Create virtual engine context for vebox
        MHW_CHK_STATUS_RETURN(pOsInterface->pfnCreateGpuContext(
            pOsInterface,
            VeboxGpuContext,
            VeboxGpuNode,
            &createOptionenhanced));
    }

    return eStatus;
}