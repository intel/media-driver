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
//! \file     media_mem_decompression_next.cpp
//! \brief    Defines data structures and interfaces for media memory decompression.
//! \details
//!
#include "media_mem_decompression_next.h"
#include "vp_utils.h"
#include "renderhal.h"
#include "mos_os_cp_interface_specific.h"

MediaMemDeCompNext::MediaMemDeCompNext():
    m_osInterface(nullptr),
    m_veboxItf(nullptr),
    m_miItf(nullptr),
    m_cpInterface(nullptr)
{
    m_veboxMMCResolveEnabled = false;
    if (!m_renderMutex)
    {
        m_renderMutex = MosUtilities::MosCreateMutex();
    }
}

MediaMemDeCompNext::~MediaMemDeCompNext()
{
    MOS_STATUS              eStatus;

    if (m_veboxItf)
    {
        m_veboxItf->DestroyHeap();
    }

    if (m_cpInterface)
    {
        if (m_osInterface)
        {
            m_osInterface->pfnDeleteMhwCpInterface(m_cpInterface);
            m_cpInterface = nullptr;
        }
        else
        {
            VPHAL_MEMORY_DECOMP_ASSERTMESSAGE("Failed to destroy cpInterface.");
        }
    }

    if (m_osInterface)
    {
        m_osInterface->pfnDestroy(m_osInterface, false);
        MOS_FreeMemory(m_osInterface);
        m_osInterface = nullptr;
    }
    if (m_renderMutex)
    {
        MosUtilities::MosDestroyMutex(m_renderMutex);
        m_renderMutex = nullptr;
    }
}

MOS_STATUS MediaMemDeCompNext::MemoryDecompress(PMOS_RESOURCE targetResource)
{
    MOS_STATUS              eStatus = MOS_STATUS_SUCCESS;
    MOS_SURFACE             targetSurface = {};

    MHW_FUNCTION_ENTER;

    VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(targetResource);
    MOS_TraceEventExt(EVENT_MEDIA_COPY, EVENT_TYPE_START, nullptr, 0, nullptr, 0);
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
            MOS_ZeroMemory(&targetSurface, sizeof(MOS_SURFACE));

            targetSurface.Format = Format_Invalid;
            targetSurface.OsResource = *targetResource;
            VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(GetResourceInfo(&targetSurface));

            //Get context before proceeding
            auto gpuContext = m_osInterface->CurrentGpuContextOrdinal;

            if (targetSurface.bCompressible)
            {
                VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(m_renderMutex);
                MosUtilities::MosLockMutex(m_renderMutex);

#if (_DEBUG || _RELEASE_INTERNAL) && !defined(LINUX)
                TRACEDATA_MEDIA_MEM_DECOMP eventData = {0};
                TRACEDATA_MEDIA_MEM_DECOMP_INIT(
                    eventData,
                    targetResource->AllocationInfo.m_AllocationHandle,
                    targetSurface.dwWidth,
                    targetSurface.dwHeight,
                    targetSurface.Format,
                    *((int64_t *)&targetResource->pGmmResInfo->GetResFlags().Gpu),
                    *((int64_t *)&targetResource->pGmmResInfo->GetResFlags().Info)
                );
                MOS_TraceEventExt(EVENT_DDI_MEDIA_MEM_DECOMP_CALLBACK, EVENT_TYPE_INFO, &eventData, sizeof(eventData), nullptr, 0);
#endif
                eStatus = RenderDecompCMD(&targetSurface);
                if (eStatus != MOS_STATUS_SUCCESS)
                {
                    MosUtilities::MosUnlockMutex(m_renderMutex);
                    VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(eStatus);
                }
                else
                {
                    MosUtilities::MosUnlockMutex(m_renderMutex);
                }
            }
        }
    }
    MOS_TraceEventExt(EVENT_MEDIA_COPY, EVENT_TYPE_END, nullptr, 0, nullptr, 0);
    return eStatus;
}

MOS_STATUS MediaMemDeCompNext::MediaMemoryCopy(PMOS_RESOURCE inputResource, PMOS_RESOURCE outputResource, bool outputCompressed)
{
    MOS_STATUS eStatus             = MOS_STATUS_SUCCESS;
    bool       bValidInputSurface  = false;
    bool       bValidOutputSurface = false;

    MHW_FUNCTION_ENTER;

    VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(inputResource);
    VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(outputResource);
    MOS_TraceEventExt(EVENT_MEDIA_COPY, EVENT_TYPE_START, nullptr, 0, nullptr, 0);

    MOS_SURFACE             sourceSurface = {};
    MOS_SURFACE             targetSurface = {};

    MOS_ZeroMemory(&targetSurface, sizeof(MOS_SURFACE));
    MOS_ZeroMemory(&sourceSurface, sizeof(MOS_SURFACE));

    targetSurface.Format     = Format_Invalid;
    targetSurface.OsResource = *outputResource;

#if !defined(LINUX) && !defined(ANDROID) && !EMUL && !_VULKAN
    // for Double Buffer copy, clear the allocationInfo temply
    MOS_ZeroMemory(&targetSurface.OsResource.AllocationInfo, sizeof(SResidencyInfo));
#endif

    sourceSurface.Format = Format_Invalid;
    sourceSurface.OsResource = *inputResource;
    VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(GetResourceInfo(&targetSurface));
    VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(GetResourceInfo(&sourceSurface));

#if (_DEBUG || _RELEASE_INTERNAL)
    {
        // Read user feature key to force outputCompressed
        ReadUserSettingForDebug(
            m_userSettingPtr,
            outputCompressed,
            __VPHAL_VEBOX_FORCE_VP_MEMCOPY_OUTPUTCOMPRESSED,
            MediaUserSetting::Group::Sequence);
    }
#endif

    if (!outputCompressed && targetSurface.CompressionMode != MOS_MMC_DISABLED)
    {
        targetSurface.CompressionMode = MOS_MMC_RC;
    }

    //if surface is linear buffer, use mos copy
    if (sourceSurface.TileType == MOS_TILE_LINEAR &&
        targetSurface.TileType == MOS_TILE_LINEAR &&
        sourceSurface.Type == MOS_GFXRES_BUFFER &&
        targetSurface.Type == MOS_GFXRES_BUFFER)
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

            uint8_t* lockedSrcAddr = (uint8_t*)m_osInterface->pfnLockResource(m_osInterface, &sourceSurface.OsResource, &lockSourceFlags);

            if (lockedSrcAddr == nullptr)
            {
                //non lockable resource enabled, we can't lock source surface
                eStatus = MOS_STATUS_NULL_POINTER;
                VPHAL_MEMORY_DECOMP_ASSERTMESSAGE("Failed to lock non-lockable input resource, buffer copy failed, eStatus:%d.\n", eStatus);
                MT_ERR2(MT_VE_DECOMP_COPY, MT_SURF_IS_INPUT, 1, MT_VE_DECOMP_COPY_SURF_LOCK_STATUS, eStatus);
                break;
            }

            uint8_t* lockedTarAddr = (uint8_t*)m_osInterface->pfnLockResource(m_osInterface, &targetSurface.OsResource, &lockTargetFlags);

            if (lockedTarAddr == nullptr)
            {
                eStatus = MOS_STATUS_NULL_POINTER;
                m_osInterface->pfnUnlockResource(m_osInterface, &sourceSurface.OsResource);
                VPHAL_MEMORY_DECOMP_ASSERTMESSAGE("Failed to lock non-lockable output resource, buffer copy failed, eStatus:%d.\n", eStatus);
                MT_ERR2(MT_VE_DECOMP_COPY, MT_SURF_IS_OUTPUT, 1, MT_VE_DECOMP_COPY_SURF_LOCK_STATUS, eStatus);
                break;
            }
            // This resource is a series of bytes. Is not 2 dimensional.
            uint32_t sizeSrcMain    = sourceSurface.dwSize;
            uint32_t sizeTargetMain = targetSurface.dwSize;
            eStatus = MOS_SecureMemcpy(lockedTarAddr, sizeTargetMain, lockedSrcAddr, sizeSrcMain);
            m_osInterface->pfnUnlockResource(m_osInterface, &sourceSurface.OsResource);
            m_osInterface->pfnUnlockResource(m_osInterface, &targetSurface.OsResource);

            if (eStatus != MOS_STATUS_SUCCESS)
            {
                VPHAL_MEMORY_DECOMP_ASSERTMESSAGE("Failed to copy linear buffer from source to target, eStatus:%d.\n", eStatus);
                MT_ERR1(MT_VE_DECOMP_COPY, MT_MOS_STATUS, eStatus);
                break;
            }
        } while (false);

        MOS_TraceEventExt(EVENT_MEDIA_COPY, EVENT_TYPE_END, nullptr, 0, nullptr, 0);
        return eStatus;
    }

    // if source surface or target surface is non-64align linear, return error here
    if ((sourceSurface.dwPitch % 64 != 0 && sourceSurface.TileType == MOS_TILE_LINEAR) ||
        (targetSurface.dwPitch % 64 != 0 && targetSurface.TileType == MOS_TILE_LINEAR))
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        VPHAL_MEMORY_DECOMP_ASSERTMESSAGE("VEBOX does not support non-64align pitch linear surface, eStatus:%d.\n", eStatus);
        MT_ERR2(MT_VE_DECOMP_COPY, MT_SURF_PITCH, sourceSurface.dwPitch, MT_SURF_PITCH, targetSurface.dwPitch);
        MOS_TraceEventExt(EVENT_MEDIA_COPY, EVENT_TYPE_END, nullptr, 0, nullptr, 0);
        return eStatus;
    }

    //Check whether surface is valid, or it will cause page fault
    m_osInterface->pfnVerifyMosSurface(&sourceSurface, bValidInputSurface);
    m_osInterface->pfnVerifyMosSurface(&targetSurface, bValidOutputSurface);
    if (!bValidInputSurface || !bValidOutputSurface)
    {
        VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    //Get context before proceeding
    auto gpuContext = m_osInterface->CurrentGpuContextOrdinal;

    // Sync for Vebox read
    m_osInterface->pfnSyncOnResource(
        m_osInterface,
        &sourceSurface.OsResource,
        MOS_GPU_CONTEXT_VEBOX,
        false);

    // Sync for Vebox write
    m_osInterface->pfnSyncOnResource(
        m_osInterface,
        &targetSurface.OsResource,
        MOS_GPU_CONTEXT_VEBOX,
        false);

#if (_DEBUG || _RELEASE_INTERNAL) && !defined(LINUX)
    TRACEDATA_MEDIACOPY eventData = {0};
    TRACEDATA_MEDIACOPY_INIT(
        eventData,
        inputResource->AllocationInfo.m_AllocationHandle,
        sourceSurface.dwWidth,
        sourceSurface.dwHeight,
        sourceSurface.Format,
        *((int64_t *)&inputResource->pGmmResInfo->GetResFlags().Gpu),
        *((int64_t *)&inputResource->pGmmResInfo->GetResFlags().Info),
        inputResource->pGmmResInfo->GetSetCpSurfTag(0, 0),
        outputResource->AllocationInfo.m_AllocationHandle,
        targetSurface.dwWidth,
        targetSurface.dwHeight,
        targetSurface.Format,
        *((int64_t *)&outputResource->pGmmResInfo->GetResFlags().Gpu),
        *((int64_t *)&outputResource->pGmmResInfo->GetResFlags().Info),
        outputResource->pGmmResInfo->GetSetCpSurfTag(0, 0)

    );
    MOS_TraceEventExt(EVENT_MEDIA_COPY, EVENT_TYPE_INFO, &eventData, sizeof(eventData), nullptr, 0);
#endif

    VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(RenderDoubleBufferDecompCMD(&sourceSurface, &targetSurface));

    MOS_TraceEventExt(EVENT_MEDIA_COPY, EVENT_TYPE_END, nullptr, 0, nullptr, 0);
    return eStatus;
}

MOS_STATUS MediaMemDeCompNext::MediaMemoryCopy2D(PMOS_RESOURCE inputResource, PMOS_RESOURCE outputResource, uint32_t copyPitch, uint32_t copyHeight, uint32_t copyInputOffset, uint32_t copyOutputOffset, uint32_t bpp, bool outputCompressed)
{
    MOS_STATUS eStatus             = MOS_STATUS_SUCCESS;
    bool       bValidInputSurface  = false;
    bool       bValidOutputSurface = false;

    MHW_FUNCTION_ENTER;

    VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(inputResource);
    VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(outputResource);
    MOS_TraceEventExt(EVENT_MEDIA_COPY, EVENT_TYPE_START, nullptr, 0, nullptr, 0);

    MOS_SURFACE             sourceSurface;
    MOS_SURFACE             targetSurface;

    MOS_ZeroMemory(&targetSurface, sizeof(MOS_SURFACE));
    MOS_ZeroMemory(&sourceSurface, sizeof(MOS_SURFACE));

    targetSurface.Format = Format_Invalid;
    targetSurface.OsResource = *outputResource;

#if !defined(LINUX) && !defined(ANDROID) && !EMUL && !_VULKAN
    // for Double Buffer copy, clear the allocationInfo temply
    MOS_ZeroMemory(&targetSurface.OsResource.AllocationInfo, sizeof(SResidencyInfo));
#endif

    sourceSurface.Format = Format_Invalid;
    sourceSurface.OsResource = *inputResource;
    VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(GetResourceInfo(&targetSurface));
    VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(GetResourceInfo(&sourceSurface));

#if (_DEBUG || _RELEASE_INTERNAL)
    {
        // Read user feature key to Force outputCompressed
        ReadUserSettingForDebug(
            m_userSettingPtr,
            outputCompressed,
            __VPHAL_VEBOX_FORCE_VP_MEMCOPY_OUTPUTCOMPRESSED,
            MediaUserSetting::Group::Sequence);
    }
#endif

    if (!outputCompressed && targetSurface.CompressionMode != MOS_MMC_DISABLED)
    {
        targetSurface.CompressionMode = MOS_MMC_RC;
    }

    //Get context before proceeding
    auto gpuContext = m_osInterface->CurrentGpuContextOrdinal;
    uint32_t pixelInByte = 1;

    switch (bpp)
    {
    case 8:
        targetSurface.Format = Format_Y8;
        sourceSurface.Format = Format_Y8;
        pixelInByte = 1;
        break;
    case 16:
        targetSurface.Format = Format_Y16U;
        sourceSurface.Format = Format_Y16U;
        pixelInByte = 2;
        break;
    case 32:
        targetSurface.Format = Format_AYUV;
        sourceSurface.Format = Format_AYUV;
        pixelInByte = 4;
        break;
    case 64:
        targetSurface.Format = Format_Y416;
        sourceSurface.Format = Format_Y416;
        pixelInByte = 8;
        break;
    default:
        targetSurface.Format = Format_Y8;
        sourceSurface.Format = Format_Y8;
        pixelInByte = 1;
        break;
    }


    sourceSurface.dwOffset = copyInputOffset;
    targetSurface.dwOffset = copyOutputOffset;

    sourceSurface.dwWidth = copyPitch / pixelInByte;
    sourceSurface.dwPitch  = copyPitch;
    sourceSurface.dwHeight = copyHeight;
    targetSurface.dwWidth = copyPitch / pixelInByte;
    targetSurface.dwPitch  = copyPitch;
    targetSurface.dwHeight = copyHeight;

    //Check whether surface is valid, or it will cause page fault
    m_osInterface->pfnVerifyMosSurface(&sourceSurface, bValidInputSurface);
    m_osInterface->pfnVerifyMosSurface(&targetSurface, bValidOutputSurface);
    if (!bValidInputSurface || !bValidOutputSurface)
    {
        VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    // Sync for Vebox write
    m_osInterface->pfnSyncOnResource(
        m_osInterface,
        &sourceSurface.OsResource,
        MOS_GPU_CONTEXT_VEBOX,
        false);

    VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(RenderDoubleBufferDecompCMD(&sourceSurface, &targetSurface));

    MOS_TraceEventExt(EVENT_MEDIA_COPY, EVENT_TYPE_END, nullptr, 0, nullptr, 0);
    return eStatus;
}

MOS_STATUS MediaMemDeCompNext::MediaMemoryTileConvert(PMOS_RESOURCE inputResource, PMOS_RESOURCE outputResource, uint32_t copyWidth, uint32_t copyHeight, uint32_t copyInputOffset, uint32_t copyOutputOffset, bool isTileToLinear, bool outputCompressed)
{
    MOS_STATUS                          eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(inputResource);
    VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(outputResource);
    MOS_TraceEventExt(EVENT_MEDIA_COPY, EVENT_TYPE_START, nullptr, 0, nullptr, 0);

    MOS_SURFACE             sourceSurface;
    MOS_SURFACE             targetSurface;

    MOS_ZeroMemory(&targetSurface, sizeof(MOS_SURFACE));
    MOS_ZeroMemory(&sourceSurface, sizeof(MOS_SURFACE));

    targetSurface.Format = Format_Invalid;
    targetSurface.OsResource = *outputResource;

#if !defined(LINUX) && !defined(ANDROID) && !EMUL && !_VULKAN
    // for Double Buffer copy, clear the allocationInfo temply
    MOS_ZeroMemory(&targetSurface.OsResource.AllocationInfo, sizeof(SResidencyInfo));
#endif

    sourceSurface.Format = Format_Invalid;
    sourceSurface.OsResource = *inputResource;
    VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(GetResourceInfo(&targetSurface));
    VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(GetResourceInfo(&sourceSurface));

    if (targetSurface.TileType == MOS_TILE_LINEAR &&
        sourceSurface.TileType == MOS_TILE_LINEAR)
    {
        VPHAL_MEMORY_DECOMP_NORMALMESSAGE("unsupport linear to linear convert, return unsupport feature");
        MT_ERR2(MT_VE_DECOMP_COPY, MT_SURF_TILE_TYPE, MOS_TILE_LINEAR, MT_SURF_TILE_TYPE, MOS_TILE_LINEAR);
        return MOS_STATUS_PLATFORM_NOT_SUPPORTED;
    }

    MOS_FORMAT format = Format_Any;
    if (isTileToLinear)
    {
        if (!IsFormatSupported(&sourceSurface))
        {
            VPHAL_MEMORY_DECOMP_NORMALMESSAGE("unsupport processing format, return unsupport feature");
            MT_ERR2(MT_VE_DECOMP_COPY, MT_SURF_MOS_FORMAT, sourceSurface.Format, MT_SURF_IS_INPUT, 1);
            return MOS_STATUS_PLATFORM_NOT_SUPPORTED;
        }

        format = sourceSurface.Format;
        copyHeight = sourceSurface.dwHeight;
    }
    else
    {
        if (!IsFormatSupported(&targetSurface))
        {
            VPHAL_MEMORY_DECOMP_NORMALMESSAGE("unsupport processing format, return unsupport feature");
            MT_ERR2(MT_VE_DECOMP_COPY, MT_SURF_MOS_FORMAT, targetSurface.Format, MT_SURF_IS_OUTPUT, 1);
            return MOS_STATUS_PLATFORM_NOT_SUPPORTED;
        }

        format = targetSurface.Format;
        copyHeight = targetSurface.dwHeight;
    }

    if (!outputCompressed && targetSurface.CompressionMode != MOS_MMC_DISABLED)
    {
        targetSurface.CompressionMode = MOS_MMC_RC;
    }

    //Get context before proceeding
    auto gpuContext = m_osInterface->CurrentGpuContextOrdinal;

    targetSurface.Format = format;
    sourceSurface.Format = format;

    sourceSurface.dwOffset = copyInputOffset;
    targetSurface.dwOffset = copyOutputOffset;

    sourceSurface.dwWidth = copyWidth;
    sourceSurface.dwHeight = copyHeight;
    targetSurface.dwWidth = copyWidth;
    targetSurface.dwHeight = copyHeight;

    // Sync for Vebox write
    m_osInterface->pfnSyncOnResource(
        m_osInterface,
        &targetSurface.OsResource,
        MOS_GPU_CONTEXT_VEBOX,
        false);

    VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(RenderDoubleBufferDecompCMD(&sourceSurface, &targetSurface));

    MOS_TraceEventExt(EVENT_MEDIA_COPY, EVENT_TYPE_END, nullptr, 0, nullptr, 0);
    return eStatus;
}

MOS_STATUS MediaMemDeCompNext::Initialize(PMOS_INTERFACE osInterface, MhwInterfacesNext* mhwInterfaces)
{
    MOS_STATUS                  eStatus         = MOS_STATUS_SUCCESS;
    MHW_VEBOX_GPUNODE_LIMIT     gpuNodeLimit    = {};
    MOS_GPU_NODE                veboxGpuNode    = MOS_GPU_NODE_VE;
    MOS_GPU_CONTEXT             veboxGpuContext = MOS_GPU_CONTEXT_VEBOX;

    VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(osInterface);
    VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(mhwInterfaces);
    VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(mhwInterfaces->m_cpInterface);
    VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(mhwInterfaces->m_miItf);
    VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(mhwInterfaces->m_veboxItf);

    m_osInterface = osInterface;
    m_cpInterface = mhwInterfaces->m_cpInterface;
    m_miItf       = mhwInterfaces->m_miItf;
    m_veboxItf    = mhwInterfaces->m_veboxItf;

    m_userSettingPtr = m_osInterface->pfnGetUserSettingInstance(m_osInterface);

    // Set-Up Vebox decompression enable or not
    IsVeboxDecompressionEnabled();

    if (m_veboxItf)
    {
        gpuNodeLimit.bCpEnabled = (m_osInterface->osCpInterface->IsCpEnabled()) ? true : false;

        // Check GPU Node decide logic together in this function
        VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(m_veboxItf->FindVeboxGpuNodeToUse(&gpuNodeLimit));

        veboxGpuNode = (MOS_GPU_NODE)(gpuNodeLimit.dwGpuNodeToUse);
        veboxGpuContext = (veboxGpuNode == MOS_GPU_NODE_VE) ? MOS_GPU_CONTEXT_VEBOX : MOS_GPU_CONTEXT_VEBOX2;

        // Create VEBOX/VEBOX2 Context
        VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(m_veboxItf->CreateGpuContext(
            m_osInterface,
            veboxGpuContext,
            veboxGpuNode));

        // Register Vebox GPU context with the Batch Buffer completion event
        VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(m_osInterface->pfnRegisterBBCompleteNotifyEvent(
            m_osInterface,
            MOS_GPU_CONTEXT_VEBOX));

        const MHW_VEBOX_HEAP* veboxHeap = nullptr;
        VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(m_veboxItf->GetVeboxHeapInfo(&veboxHeap));
        if (veboxHeap == nullptr)
        {
            m_veboxItf->CreateHeap();
        }
    }

    return eStatus;
}

MOS_STATUS MediaMemDeCompNext::GetResourceInfo(PMOS_SURFACE surface)
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

    surface->Format                         = resDetails.Format;
    surface->dwWidth                        = resDetails.dwWidth;
    surface->dwHeight                       = resDetails.dwHeight;
    surface->dwPitch                        = resDetails.dwPitch;
    surface->dwDepth                        = resDetails.dwDepth;
    surface->bArraySpacing                  = resDetails.bArraySpacing;
    surface->TileType                       = resDetails.TileType;
    surface->TileModeGMM                    = resDetails.TileModeGMM;
    surface->bGMMTileEnabled                = resDetails.bGMMTileEnabled;
    surface->bCompressible                  = resDetails.bCompressible;
    surface->bIsCompressed                  = resDetails.bIsCompressed;
    surface->dwOffset                       = resDetails.RenderOffset.YUV.Y.BaseOffset;
    surface->YPlaneOffset.iSurfaceOffset    = resDetails.RenderOffset.YUV.Y.BaseOffset;
    surface->YPlaneOffset.iXOffset          = resDetails.RenderOffset.YUV.Y.XOffset;
    surface->YPlaneOffset.iYOffset          = resDetails.RenderOffset.YUV.Y.YOffset;
    surface->UPlaneOffset.iSurfaceOffset    = resDetails.RenderOffset.YUV.U.BaseOffset;
    surface->UPlaneOffset.iXOffset          = resDetails.RenderOffset.YUV.U.XOffset;
    surface->UPlaneOffset.iYOffset          = resDetails.RenderOffset.YUV.U.YOffset;
    surface->VPlaneOffset.iSurfaceOffset    = resDetails.RenderOffset.YUV.V.BaseOffset;
    surface->VPlaneOffset.iXOffset          = resDetails.RenderOffset.YUV.V.XOffset;
    surface->VPlaneOffset.iYOffset          = resDetails.RenderOffset.YUV.V.YOffset;
    surface->dwSize = (uint32_t)surface->OsResource.pGmmResInfo->GetSizeMainSurface();

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


MOS_STATUS MediaMemDeCompNext::SetupVeboxSurfaceState(PMHW_VEBOX_SURFACE_STATE_CMD_PARAMS mhwVeboxSurfaceStateCmdParams, PMOS_SURFACE inputSurface, PMOS_SURFACE outputSurface)
{
 MOS_STATUS              eStatus = MOS_STATUS_SUCCESS;
    bool  inputIsLinearBuffer = false;
    bool  outputIsLinearBuffer = false;
    uint32_t bpp = 1;
    uint32_t inputWidth = 0;
    uint32_t outputWidth = 0;

    VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(inputSurface);
    VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(mhwVeboxSurfaceStateCmdParams);

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
        VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(m_osInterface->pfnGetResourceInfo(
            m_osInterface,
            &inputSurface->OsResource,
            &inputDetails));
    }

    if (outputSurface)
    {
        VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(m_osInterface->pfnGetResourceInfo(
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
            VPHAL_MEMORY_DECOMP_NORMALMESSAGE("2D to 2D, no need for bpp setting.");
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
    mhwVeboxSurfaceStateCmdParams->SurfInput.rcMaxSrc.right  = mhwVeboxSurfaceStateCmdParams->SurfOutput.rcMaxSrc.right  = (long)(mhwVeboxSurfaceStateCmdParams->SurfInput.dwWidth);
    mhwVeboxSurfaceStateCmdParams->SurfInput.rcMaxSrc.top    = mhwVeboxSurfaceStateCmdParams->SurfOutput.rcMaxSrc.top    = 0;
    mhwVeboxSurfaceStateCmdParams->SurfInput.rcMaxSrc.bottom = mhwVeboxSurfaceStateCmdParams->SurfOutput.rcMaxSrc.bottom = (long)(mhwVeboxSurfaceStateCmdParams->SurfInput.dwHeight);
    mhwVeboxSurfaceStateCmdParams->bOutputValid = true;

    // if output surface is null, then Inplace resolve happens
    if (!outputSurface)
    {
        mhwVeboxSurfaceStateCmdParams->SurfInput.TileType        = mhwVeboxSurfaceStateCmdParams->SurfOutput.TileType            = inputSurface->TileType;
        mhwVeboxSurfaceStateCmdParams->SurfInput.TileModeGMM     = mhwVeboxSurfaceStateCmdParams->SurfOutput.TileModeGMM         = inputSurface->TileModeGMM;
        mhwVeboxSurfaceStateCmdParams->SurfInput.bGMMTileEnabled = mhwVeboxSurfaceStateCmdParams->SurfOutput.bGMMTileEnabled     = inputSurface->bGMMTileEnabled;
        mhwVeboxSurfaceStateCmdParams->SurfOutput.dwPitch        = mhwVeboxSurfaceStateCmdParams->SurfInput.dwPitch              = inputSurface->dwPitch;
        mhwVeboxSurfaceStateCmdParams->SurfInput.pOsResource     = mhwVeboxSurfaceStateCmdParams->SurfOutput.pOsResource         = &(inputSurface->OsResource);
        mhwVeboxSurfaceStateCmdParams->SurfInput.dwYoffset       = mhwVeboxSurfaceStateCmdParams->SurfOutput.dwYoffset           = inputSurface->YPlaneOffset.iYOffset;

        mhwVeboxSurfaceStateCmdParams->SurfInput.dwCompressionFormat = mhwVeboxSurfaceStateCmdParams->SurfOutput.dwCompressionFormat =
            inputSurface->CompressionFormat;
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
    }

    return eStatus;
}

MOS_STATUS MediaMemDeCompNext::InitCommandBuffer(PMOS_COMMAND_BUFFER cmdBuffer)
{
    PMOS_INTERFACE                   osInterface         = nullptr;
    MOS_STATUS                       eStatus             = MOS_STATUS_SUCCESS;
    uint32_t                         iRemaining          = 0;
    PMOS_RESOURCE                    gpuStatusBuffer     = nullptr;
    RENDERHAL_GENERIC_PROLOG_PARAMS  genericPrologParams = {};

    //---------------------------------------------
    VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(cmdBuffer);
    VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(m_osInterface);
    VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(m_miItf);
    //---------------------------------------------
    osInterface = m_osInterface;

    MOS_GPU_CONTEXT gpuContext = m_osInterface->pfnGetGpuContext(m_osInterface);

#ifndef EMUL
    if (osInterface->bEnableKmdMediaFrameTracking)
    {
        // Get GPU Status buffer
        VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(osInterface->pfnGetGpuStatusBufferResource(osInterface, gpuStatusBuffer));
        VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(gpuStatusBuffer);
        // Register the buffer
        VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(osInterface->pfnRegisterResource(osInterface, gpuStatusBuffer, true, true));

        genericPrologParams.bEnableMediaFrameTracking = true;
        genericPrologParams.presMediaFrameTrackingSurface = gpuStatusBuffer;
        genericPrologParams.dwMediaFrameTrackingTag = osInterface->pfnGetGpuStatusTag(osInterface, osInterface->CurrentGpuContextOrdinal);
        genericPrologParams.dwMediaFrameTrackingAddrOffset = osInterface->pfnGetGpuStatusTagOffset(osInterface, osInterface->CurrentGpuContextOrdinal);

        // Increment GPU Status Tag
        osInterface->pfnIncrementGpuStatusTag(osInterface, osInterface->CurrentGpuContextOrdinal);
    }
#endif

    if (genericPrologParams.bEnableMediaFrameTracking)
    {
        VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(genericPrologParams.presMediaFrameTrackingSurface);
        cmdBuffer->Attributes.bEnableMediaFrameTracking      = genericPrologParams.bEnableMediaFrameTracking;
        cmdBuffer->Attributes.dwMediaFrameTrackingTag        = genericPrologParams.dwMediaFrameTrackingTag;
        cmdBuffer->Attributes.dwMediaFrameTrackingAddrOffset = genericPrologParams.dwMediaFrameTrackingAddrOffset;
        cmdBuffer->Attributes.resMediaFrameTrackingSurface   = genericPrologParams.presMediaFrameTrackingSurface;
    }

    // initialize command buffer attributes
    cmdBuffer->Attributes.bTurboMode = false;
    cmdBuffer->Attributes.bMediaPreemptionEnabled = false;
    cmdBuffer->Attributes.dwMediaFrameTrackingAddrOffset = 0;

    MHW_GENERIC_PROLOG_PARAMS genericPrologMhwParams;
    MOS_ZeroMemory(&genericPrologMhwParams, sizeof(genericPrologMhwParams));
    genericPrologMhwParams.pOsInterface  = m_osInterface;
    genericPrologMhwParams.pvMiInterface = nullptr;
    genericPrologMhwParams.bMmcEnabled   = true;
    VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(Mhw_SendGenericPrologCmdNext(
        cmdBuffer,
        &genericPrologMhwParams,
        m_miItf));

    return eStatus;
}

bool MediaMemDeCompNext::IsFormatSupported(PMOS_SURFACE surface)
{
    bool    bRet = false;

    if (surface->Format == Format_R10G10B10A2 ||
        surface->Format == Format_B10G10R10A2 ||
        surface->Format == Format_Y410 ||
        surface->Format == Format_Y210)
    {
        // Re-map RGB10/RGB10/Y410/Y210 as AYUV
        surface->Format = Format_AYUV;
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
        surface->Format == Format_420O)
    {
        surface->Format = Format_P8;
        surface->dwHeight = surface->dwSize / surface->dwPitch;
    }

    if (IS_RGB64_FLOAT_FORMAT(surface->Format) ||
        IS_RGB64_FORMAT(surface->Format)       ||
       (surface->Format == Format_G32R32F))
    {
        surface->Format = Format_Y416;
    }

    // Check if Sample Format is supported for decompression
    if (surface->Format != Format_NV12 &&
        surface->Format != Format_AYUV &&
        surface->Format != Format_Y416 &&
        surface->Format != Format_P010 &&
        surface->Format != Format_P016 &&
        !IS_PA_FORMAT(surface->Format) &&
        surface->Format != Format_A8R8G8B8 &&
        surface->Format != Format_A8B8G8R8 &&
        surface->Format != Format_X8R8G8B8 &&
        surface->Format != Format_X8B8G8R8 &&
        surface->Format != Format_P8 &&
        surface->Format != Format_Y16U)
    {
        VPHAL_MEMORY_DECOMP_NORMALMESSAGE("Unsupported Source Format '0x%08x' for VEBOX Decompression.", surface->Format);
        return false;
    }

    return true;;
}
