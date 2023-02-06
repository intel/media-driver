/*
* Copyright (c) 2011-2021, Intel Corporation
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
//! \file     codechal_debug.cpp
//! \brief    Defines the debug interface shared by codec only.
//! \details  The debug interface dumps output from Media based on in input config file.
//!
#include "codechal_debug.h"
#if USE_CODECHAL_DEBUG_TOOL
#include "codechal_debug_config_manager.h"
#include "media_debug_fast_dump.h"
#include <iomanip>

CodechalDebugInterface::CodechalDebugInterface()
{
    MOS_ZeroMemory(m_fileName, sizeof(m_fileName));
    MOS_ZeroMemory(m_path, sizeof(m_path));

    m_dumpYUVSurface = [this](
                           PMOS_SURFACE              surface,
                           const char               *attrName,
                           const char               *surfName,
                           CODECHAL_MEDIA_STATE_TYPE mediaState,
                           uint32_t                  width_in,
                           uint32_t                  height_in) {
        bool     hasAuxSurf   = false;
        bool     isPlanar     = true;
        bool     hasRefSurf   = false;
        uint8_t *surfBaseAddr = nullptr;
        uint8_t *lockedAddr   = nullptr;
        if (!DumpIsEnabled(attrName, mediaState))
        {
            return MOS_STATUS_SUCCESS;
        }

        const char *funcName = (m_codecFunction == CODECHAL_FUNCTION_DECODE) ? "_DEC" : (m_codecFunction == CODECHAL_FUNCTION_CENC_DECODE ? "_DEC" : "_ENC");
        std::string bufName  = std::string(surfName) + "_w[" + std::to_string(surface->dwWidth) + "]_h[" + std::to_string(surface->dwHeight) + "]_p[" + std::to_string(surface->dwPitch) + "]";

        const char *filePath = CreateFileName(funcName, bufName.c_str(), hasAuxSurf ? ".Y" : ".yuv");

        if (DumpIsEnabled(MediaDbgAttr::attrEnableFastDump))
        {
            MediaDebugFastDump::Dump(surface->OsResource, filePath);
            return MOS_STATUS_SUCCESS;
        }

        MOS_LOCK_PARAMS   lockFlags;
        GMM_RESOURCE_FLAG gmmFlags;

        MOS_ZeroMemory(&gmmFlags, sizeof(gmmFlags));
        CODECHAL_DEBUG_CHK_NULL(surface);
        gmmFlags   = surface->OsResource.pGmmResInfo->GetResFlags();
        hasAuxSurf = (gmmFlags.Gpu.MMC || gmmFlags.Info.MediaCompressed) && gmmFlags.Gpu.UnifiedAuxSurface;

        if (!m_configMgr->AttrIsEnabled(CodechalDbgAttr::attrDecodeAuxSurface))
        {
            hasAuxSurf = false;
        }

        isPlanar = !!(m_osInterface->pfnGetGmmClientContext(m_osInterface)->IsPlanar(surface->OsResource.pGmmResInfo->GetResourceFormat()));

        MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
        lockFlags.ReadOnly     = 1;
        lockFlags.TiledAsTiled = 1;  // Bypass GMM CPU blit due to some issues in GMM CpuBlt function
        if (hasAuxSurf)
        {
            // Dump MMC surface as raw layout
            lockFlags.NoDecompress = 1;
        }

        // when not setting dump compressed surface cfg, use ve copy for dump
        if (!m_configMgr->AttrIsEnabled(CodechalDbgAttr::attrDecodeCompSurface))
        {
            DumpUncompressedYUVSurface(surface);
            lockedAddr = (uint8_t *)m_osInterface->pfnLockResource(m_osInterface, &m_temp2DSurfForCopy.OsResource, &lockFlags);
            CODECHAL_DEBUG_CHK_NULL(lockedAddr);
            surface = &m_temp2DSurfForCopy;
        }
        else
        {
            // when setting dump compressed surface cfg, try directly lock surface
            lockedAddr = (uint8_t *)m_osInterface->pfnLockResource(m_osInterface, &surface->OsResource, &lockFlags);
            // if failed to lock, try to submit copy task and reallocate temp surface for dump
            if (lockedAddr == nullptr)
            {
                uint32_t        sizeToBeCopied = 0;
                MOS_GFXRES_TYPE ResType;

#if !defined(LINUX)
                ResType = surface->OsResource.ResType;

                CODECHAL_DEBUG_CHK_STATUS(ReAllocateSurface(
                    &m_temp2DSurfForCopy,
                    surface,
                    "Temp2DSurfForSurfDumper",
                    ResType));
#endif

                uint32_t arraySize = surface->OsResource.pGmmResInfo->GetArraySize();

                if (arraySize == 0)
                {
                    return MOS_STATUS_UNKNOWN;
                }

                uint32_t sizeSrcSurface = (uint32_t)(surface->OsResource.pGmmResInfo->GetSizeMainSurface()) / arraySize;
                // Ensure allocated buffer size contains the source surface size
                if (m_temp2DSurfForCopy.OsResource.pGmmResInfo->GetSizeMainSurface() >= sizeSrcSurface)
                {
                    sizeToBeCopied = sizeSrcSurface;
                }

                if (sizeToBeCopied == 0)
                {
                    CODECHAL_DEBUG_ASSERTMESSAGE("Cannot allocate correct size, failed to copy nonlockable resource");
                    return MOS_STATUS_INVALID_PARAMETER;
                }

                CODECHAL_DEBUG_VERBOSEMESSAGE("Temp2DSurfaceForCopy width %d, height %d, pitch %d, TileType %d, bIsCompressed %d, CompressionMode %d",
                    m_temp2DSurfForCopy.dwWidth,
                    m_temp2DSurfForCopy.dwHeight,
                    m_temp2DSurfForCopy.dwPitch,
                    m_temp2DSurfForCopy.TileType,
                    m_temp2DSurfForCopy.bIsCompressed,
                    m_temp2DSurfForCopy.CompressionMode);

                uint32_t bpp = surface->OsResource.pGmmResInfo->GetBitsPerPixel();

                CODECHAL_DEBUG_CHK_STATUS(m_osInterface->pfnMediaCopyResource2D(m_osInterface, &surface->OsResource, &m_temp2DSurfForCopy.OsResource, surface->dwPitch, sizeSrcSurface / (surface->dwPitch), 0, 0, bpp, false));

                lockedAddr = (uint8_t *)m_osInterface->pfnLockResource(m_osInterface, &m_temp2DSurfForCopy.OsResource, &lockFlags);
                CODECHAL_DEBUG_CHK_NULL(lockedAddr);
                surface = &m_temp2DSurfForCopy;
            }
        }
        gmmFlags     = surface->OsResource.pGmmResInfo->GetResFlags();
        surfBaseAddr = lockedAddr;

        // Use MOS swizzle for non-vecopy dump
        if (m_configMgr->AttrIsEnabled(CodechalDbgAttr::attrDecodeCompSurface))
        {
            uint32_t sizeMain = (uint32_t)(surface->OsResource.pGmmResInfo->GetSizeMainSurface());
            surfBaseAddr      = (uint8_t *)MOS_AllocMemory(sizeMain);
            CODECHAL_DEBUG_CHK_NULL(surfBaseAddr);
            Mos_SwizzleData(lockedAddr, surfBaseAddr, surface->TileType, MOS_TILE_LINEAR, sizeMain / surface->dwPitch, surface->dwPitch, !MEDIA_IS_SKU(m_osInterface->pfnGetSkuTable(m_osInterface), FtrTileY) || gmmFlags.Info.Tile4);
        }

        uint8_t *data = surfBaseAddr;
        data += surface->dwOffset + surface->YPlaneOffset.iYOffset * surface->dwPitch;

        uint32_t width      = width_in ? width_in : surface->dwWidth;
        uint32_t height     = height_in ? height_in : surface->dwHeight;
        uint32_t lumaheight = 0;

        switch (surface->Format)
        {
        case Format_YUY2:
        case Format_P010:
        case Format_P016:
            width = width << 1;
            break;
        case Format_Y216:
        case Format_Y210:  //422 10bit -- Y0[15:0]:U[15:0]:Y1[15:0]:V[15:0] = 32bits per pixel = 4Bytes per pixel
        case Format_Y410:  //444 10bit -- A[31:30]:V[29:20]:Y[19:10]:U[9:0] = 32bits per pixel = 4Bytes per pixel
        case Format_R10G10B10A2:
        case Format_AYUV:  //444 8bit  -- A[31:24]:Y[23:16]:U[15:8]:V[7:0] = 32bits per pixel = 4Bytes per pixel
        case Format_A8R8G8B8:
            width = width << 2;
            break;
        case Format_Y416:
            width = width << 3;
            break;
        case Format_R8G8B8:
            width = width * 3;
            break;
        default:
            break;
        }

        uint32_t pitch = surface->dwPitch;
        if (surface->Format == Format_UYVY)
            pitch = width;

        if (CodecHal_PictureIsBottomField(m_currPic))
        {
            data += pitch;
        }

        if (CodecHal_PictureIsField(m_currPic))
        {
            pitch *= 2;
            height /= 2;
        }

        lumaheight = hasAuxSurf ? GFX_ALIGN(height, 32) : height;

        std::ofstream ofs(filePath, std::ios_base::out | std::ios_base::binary);
        if (ofs.fail())
        {
            return MOS_STATUS_UNKNOWN;
        }

        // write luma data to file
        for (uint32_t h = 0; h < lumaheight; h++)
        {
            ofs.write((char *)data, hasAuxSurf ? pitch : width);
            data += pitch;
        }

        switch (surface->Format)
        {
        case Format_NV12:
        case Format_P010:
        case Format_P016:
            height = height >> 1;
            break;
        case Format_Y416:  //444 16bit
        case Format_AYUV:  //444 8bit
        case Format_AUYV:
        case Format_Y410:  //444 10bit
        case Format_R10G10B10A2:
            height = height << 1;
            break;
        case Format_YUY2:
        case Format_YUYV:
        case Format_YVYU:
        case Format_UYVY:
        case Format_VYUY:
        case Format_Y216:  //422 16bit
        case Format_Y210:  //422 10bit
        case Format_P208:  //422 8bit
        case Format_RGBP:
        case Format_BGRP:
            break;
        case Format_422V:
        case Format_IMC3:
            height = height / 2;
            break;
        default:
            height = 0;
            break;
        }

        uint8_t *vPlaneData = surfBaseAddr;
        if (isPlanar)
        {
            if (hasAuxSurf)
            {
                data = surfBaseAddr + surface->UPlaneOffset.iSurfaceOffset;
            }
            else
            {
// LibVA uses iSurfaceOffset instead of iLockSurfaceOffset.To use this dump in Linux,
// changing the variable to iSurfaceOffset may be necessary for the chroma dump to work properly.
#if defined(LINUX)
                data = surfBaseAddr + surface->UPlaneOffset.iSurfaceOffset;
                if (surface->Format == Format_422V || surface->Format == Format_IMC3)
                {
                    vPlaneData = surfBaseAddr + surface->VPlaneOffset.iSurfaceOffset;
                }
#else
                data = surfBaseAddr + surface->UPlaneOffset.iLockSurfaceOffset;
                if (surface->Format == Format_422V ||
                    surface->Format == Format_IMC3 ||
                    surface->Format == Format_RGBP ||
                    surface->Format == Format_BGRP)
                {
                    vPlaneData = surfBaseAddr + surface->VPlaneOffset.iLockSurfaceOffset;
                }
#endif
            }

            //No seperate chroma for linear surfaces
            // Seperate Y/UV if MMC is enabled
            if (hasAuxSurf)
            {
                const char   *uvfilePath = CreateFileName(funcName, bufName.c_str(), ".UV");
                std::ofstream ofs1(uvfilePath, std::ios_base::out | std::ios_base::binary);
                if (ofs1.fail())
                {
                    return MOS_STATUS_UNKNOWN;
                }
                // write chroma data to file
                for (uint32_t h = 0; h < GFX_ALIGN(height, 32); h++)
                {
                    ofs1.write((char *)data, pitch);
                    data += pitch;
                }
                ofs1.close();
            }
            else
            {
                // write chroma data to file
                for (uint32_t h = 0; h < height; h++)
                {
                    ofs.write((char *)data, hasAuxSurf ? pitch : width);
                    data += pitch;
                }

                // write v planar data to file
                if (surface->Format == Format_422V ||
                    surface->Format == Format_IMC3 ||
                    surface->Format == Format_RGBP ||
                    surface->Format == Format_BGRP)
                {
                    for (uint32_t h = 0; h < height; h++)
                    {
                        ofs.write((char *)vPlaneData, hasAuxSurf ? pitch : width);
                        vPlaneData += pitch;
                    }
                }
            }
            ofs.close();
        }

        if (hasAuxSurf)
        {
            uint32_t resourceIndex = m_osInterface->pfnGetResourceIndex(&surface->OsResource);
            uint8_t *yAuxData      = (uint8_t *)lockedAddr + surface->OsResource.pGmmResInfo->GetPlanarAuxOffset(resourceIndex, GMM_AUX_Y_CCS);
            uint32_t yAuxSize      = isPlanar ? ((uint32_t)(surface->OsResource.pGmmResInfo->GetPlanarAuxOffset(resourceIndex, GMM_AUX_UV_CCS) -
                                                       surface->OsResource.pGmmResInfo->GetPlanarAuxOffset(resourceIndex, GMM_AUX_Y_CCS)))
                                              : (uint32_t)surface->OsResource.pGmmResInfo->GetAuxQPitch();

            // Y Aux data
            const char   *yAuxfilePath = CreateFileName(funcName, bufName.c_str(), ".Yaux");
            std::ofstream ofs2(yAuxfilePath, std::ios_base::out | std::ios_base::binary);
            if (ofs2.fail())
            {
                return MOS_STATUS_UNKNOWN;
            }
            ofs2.write((char *)yAuxData, yAuxSize);
            ofs2.close();

            if (isPlanar)
            {
                uint8_t *uvAuxData = (uint8_t *)lockedAddr + surface->OsResource.pGmmResInfo->GetPlanarAuxOffset(resourceIndex, GMM_AUX_UV_CCS);
                uint32_t uvAuxSize = (uint32_t)surface->OsResource.pGmmResInfo->GetAuxQPitch() - yAuxSize;

                // UV Aux data
                const char   *uvAuxfilePath = CreateFileName(funcName, bufName.c_str(), ".UVaux");
                std::ofstream ofs3(uvAuxfilePath, std::ios_base::out | std::ios_base::binary);
                if (ofs3.fail())
                {
                    return MOS_STATUS_UNKNOWN;
                }
                ofs3.write((char *)uvAuxData, uvAuxSize);

                ofs3.close();
            }
        }

        if (surfBaseAddr && m_configMgr->AttrIsEnabled(CodechalDbgAttr::attrDecodeCompSurface))
        {
            MOS_FreeMemory(surfBaseAddr);
        }
        if (lockedAddr)
        {
            m_osInterface->pfnUnlockResource(m_osInterface, &surface->OsResource);
        }

        return MOS_STATUS_SUCCESS;
    };

    m_dumpBuffer = [this](
                       PMOS_RESOURCE             resource,
                       const char               *attrName,
                       const char               *bufferName,
                       uint32_t                  size,
                       uint32_t                  offset,
                       CODECHAL_MEDIA_STATE_TYPE mediaState) {
        MEDIA_DEBUG_FUNCTION_ENTER;

        MEDIA_DEBUG_CHK_NULL(resource);
        MEDIA_DEBUG_CHK_NULL(bufferName);

        if (size == 0)
        {
            return MOS_STATUS_SUCCESS;
        }

        if (attrName)
        {
            bool attrEnabled = false;

            if (mediaState == CODECHAL_NUM_MEDIA_STATES)
            {
                attrEnabled = m_configMgr->AttrIsEnabled(attrName);
            }
            else
            {
                attrEnabled = static_cast<CodecDebugConfigMgr*>(m_configMgr)->AttrIsEnabled(mediaState, attrName);
            }

            if (!attrEnabled)
            {
                return MOS_STATUS_SUCCESS;
            }
        }

        const char *fileName;
        bool        binaryDump = m_configMgr->AttrIsEnabled(MediaDbgAttr::attrDumpBufferInBinary);
        const char *extType    = binaryDump ? MediaDbgExtType::dat : MediaDbgExtType::txt;

        if (mediaState == CODECHAL_NUM_MEDIA_STATES)
        {
            fileName = CreateFileName(bufferName, attrName, extType);
        }
        else
        {
            std::string kernelName = static_cast<CodecDebugConfigMgr*>(m_configMgr)->GetMediaStateStr(mediaState);
            fileName               = CreateFileName(kernelName.c_str(), bufferName, extType);
        }

        if (DumpIsEnabled(MediaDbgAttr::attrEnableFastDump))
        {
            MediaDebugFastDump::Dump(*resource, fileName, size, offset);
            return MOS_STATUS_SUCCESS;
        }

        MOS_LOCK_PARAMS lockFlags;
        MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
        lockFlags.ReadOnly = 1;
        uint8_t *data      = (uint8_t *)m_osInterface->pfnLockResource(m_osInterface, resource, &lockFlags);
        MEDIA_DEBUG_CHK_NULL(data);
        data += offset;

        MOS_STATUS status;
        if (binaryDump)
        {
            status = DumpBufferInBinary(data, size);
        }
        else
        {
            status = DumpBufferInHexDwords(data, size);
        }

        if (data)
        {
            m_osInterface->pfnUnlockResource(m_osInterface, resource);
        }

        return status;
    };
}

CodechalDebugInterface::~CodechalDebugInterface()
{
    if (nullptr != m_configMgr)
    {
        MOS_Delete(m_configMgr);
    }

    MediaDebugFastDump::DestroyInstance();
}

void CodechalDebugInterface::CheckGoldenReferenceExist()
{
    std::ifstream crcGoldenRefStream(m_crcGoldenRefFileName);
    m_goldenReferenceExist = crcGoldenRefStream.good() ? true : false;
}

MOS_STATUS CodechalDebugInterface::DumpRgbDataOnYUVSurface(
    PMOS_SURFACE              surface,
    const char               *attrName,
    const char               *surfName,
    CODECHAL_MEDIA_STATE_TYPE mediaState,
    uint32_t                  width_in,
    uint32_t                  height_in)
{
    bool     hasAuxSurf   = false;
    bool     isPlanar     = true;
    bool     hasRefSurf   = false;
    uint8_t *surfBaseAddr = nullptr;
    uint8_t *lockedAddr   = nullptr;

    if (!DumpIsEnabled(attrName, mediaState))
    {
        return MOS_STATUS_SUCCESS;
    }

    //To write RGB data on AYUV surface for validation purpose due to similiar layout
    int32_t sfcOutputRgbFormatFlag = 0;
    int32_t bIsSfcOutputLinearFlag = 0;
    //tile type read from reg key
    {
        MediaUserSetting::Value outValue;
        ReadUserSettingForDebug(
            m_userSettingPtr,
            outValue,
            __MEDIA_USER_FEATURE_VALUE_DECODE_SFC_RGBFORMAT_OUTPUT_DEBUG,
            MediaUserSetting::Group::Sequence);
        sfcOutputRgbFormatFlag = outValue.Get<int32_t>();
    }

    //rgb format output read from reg key
    {
        MediaUserSetting::Value outValue;
        ReadUserSettingForDebug(
            m_userSettingPtr,
            outValue,
            __MEDIA_USER_FEATURE_VALUE_DECODE_SFC_LINEAR_OUTPUT_DEBUG,
            MediaUserSetting::Group::Sequence);
        bIsSfcOutputLinearFlag = outValue.Get<int32_t>();
    }

    if (!sfcOutputRgbFormatFlag)
    {
        return MOS_STATUS_SUCCESS;
    }

    MOS_FORMAT  outputFormat = surface->Format;
    const char *filePostfix  = "unsupport";

    switch (sfcOutputRgbFormatFlag)
    {
    case 1:  //RGBP
        outputFormat = Format_RGBP;
        filePostfix  = "rgbp";
        break;
    case 2:  //BGRP
        outputFormat = Format_BGRP;
        filePostfix  = "bgrp";
        break;
    case 3:  //RGB24 only support Tile Linear
        if (bIsSfcOutputLinearFlag)
        {
            outputFormat = Format_R8G8B8;
            filePostfix  = "rgb24";
            break;
        }
    default:
        CODECHAL_DEBUG_ASSERTMESSAGE("unsupport rgb format");
        return MOS_STATUS_SUCCESS;
    }

    MOS_TILE_TYPE outputTileType = MOS_TILE_Y;
    if (bIsSfcOutputLinearFlag == 1)
    {
        outputTileType = MOS_TILE_LINEAR;
    }

    MOS_LOCK_PARAMS   lockFlags;
    GMM_RESOURCE_FLAG gmmFlags;

    MOS_ZeroMemory(&gmmFlags, sizeof(gmmFlags));
    CODECHAL_DEBUG_CHK_NULL(surface);
    gmmFlags   = surface->OsResource.pGmmResInfo->GetResFlags();
    hasAuxSurf = (gmmFlags.Gpu.MMC || gmmFlags.Info.MediaCompressed) && gmmFlags.Gpu.UnifiedAuxSurface;

    if (!m_configMgr->AttrIsEnabled(CodechalDbgAttr::attrDecodeAuxSurface))
    {
        hasAuxSurf = false;
    }

    if (strcmp(attrName, CodechalDbgAttr::attrDecodeReferenceSurfaces) == 0)
    {
        hasRefSurf = true;
    }

    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.ReadOnly     = 1;
    lockFlags.TiledAsTiled = 1;  // Bypass GMM CPU blit due to some issues in GMM CpuBlt function
    if (hasAuxSurf)
    {
        // Dump MMC surface as raw layout
        lockFlags.NoDecompress = 1;
    }

    lockedAddr = (uint8_t *)m_osInterface->pfnLockResource(m_osInterface, &surface->OsResource, &lockFlags);

    if (lockedAddr == nullptr)  // Failed to lock. Try to submit copy task and dump another surface
    {
        uint32_t        sizeToBeCopied = 0;
        MOS_GFXRES_TYPE ResType;

#if LINUX
        // Linux does not have OsResource->ResType
        ResType = surface->Type;
#else
        ResType = surface->OsResource.ResType;
#endif

        CODECHAL_DEBUG_CHK_STATUS(ReAllocateSurface(
            &m_temp2DSurfForCopy,
            surface,
            "Temp2DSurfForSurfDumper",
            ResType));

        if (!hasRefSurf)
        {
            uint32_t arraySize = surface->OsResource.pGmmResInfo->GetArraySize();

            if (arraySize == 0)
            {
                return MOS_STATUS_UNKNOWN;
            }

            uint32_t sizeSrcSurface = (uint32_t)(surface->OsResource.pGmmResInfo->GetSizeMainSurface()) / arraySize;
            // Ensure allocated buffer size contains the source surface size
            if (m_temp2DSurfForCopy.OsResource.pGmmResInfo->GetSizeMainSurface() >= sizeSrcSurface)
            {
                sizeToBeCopied = sizeSrcSurface;
            }

            if (sizeToBeCopied == 0)
            {
                // Currently, MOS's pfnAllocateResource does not support allocate a surface reference to another surface.
                // When the source surface is not created from Media, it is possible that we cannot allocate the same size as source.
                // For example, on Gen9, Render target might have GMM set CCS=1 MMC=0, but MOS cannot allocate surface with such combination.
                // When Gmm allocation parameter is different, the resulting surface size/padding/pitch will be differnt.
                // Once if MOS can support allocate a surface by reference another surface, we can do a bit to bit copy without problem.
                CODECHAL_DEBUG_ASSERTMESSAGE("Cannot allocate correct size, failed to copy nonlockable resource");
                return MOS_STATUS_NULL_POINTER;
            }

            CODECHAL_DEBUG_VERBOSEMESSAGE("Temp2DSurfaceForCopy width %d, height %d, pitch %d, TileType %d, bIsCompressed %d, CompressionMode %d",
                m_temp2DSurfForCopy.dwWidth,
                m_temp2DSurfForCopy.dwHeight,
                m_temp2DSurfForCopy.dwPitch,
                m_temp2DSurfForCopy.TileType,
                m_temp2DSurfForCopy.bIsCompressed,
                m_temp2DSurfForCopy.CompressionMode);

            uint32_t bpp = surface->OsResource.pGmmResInfo->GetBitsPerPixel();

            CODECHAL_DEBUG_CHK_STATUS(m_osInterface->pfnMediaCopyResource2D(m_osInterface, &surface->OsResource, &m_temp2DSurfForCopy.OsResource, surface->dwPitch, sizeSrcSurface / (surface->dwPitch), 0, 0, bpp, false));
        }
        else
        {
            CODECHAL_DEBUG_CHK_STATUS(m_osInterface->pfnDoubleBufferCopyResource(m_osInterface, &surface->OsResource, &m_temp2DSurfForCopy.OsResource, false));
        }

        lockedAddr = (uint8_t *)m_osInterface->pfnLockResource(m_osInterface, &m_temp2DSurfForCopy.OsResource, &lockFlags);
        CODECHAL_DEBUG_CHK_NULL(lockedAddr);

        surface  = &m_temp2DSurfForCopy;
        gmmFlags = surface->OsResource.pGmmResInfo->GetResFlags();
    }

    if (hasAuxSurf)
    {
        uint32_t sizeMain = (uint32_t)(surface->OsResource.pGmmResInfo->GetSizeMainSurface());
        surfBaseAddr      = (uint8_t *)MOS_AllocMemory(sizeMain);
        CODECHAL_DEBUG_CHK_NULL(surfBaseAddr);

        Mos_SwizzleData(lockedAddr, surfBaseAddr, surface->TileType, MOS_TILE_LINEAR, sizeMain / surface->dwPitch, surface->dwPitch, true);
    }
    else
    {
        surfBaseAddr = lockedAddr;
    }

    // Allocate RGB surface internally to get surface info
    MOS_SURFACE *tmpRgbSurface = MOS_New(MOS_SURFACE);
    CODECHAL_DEBUG_CHK_NULL(tmpRgbSurface);
    MOS_ALLOC_GFXRES_PARAMS allocParams;
    MOS_ZeroMemory(&allocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParams.Type              = MOS_GFXRES_2D;
    allocParams.TileType          = outputTileType;
    allocParams.Format            = outputFormat;
    allocParams.dwWidth           = surface->dwWidth;
    allocParams.dwHeight          = surface->dwHeight;
    allocParams.dwArraySize       = 1;
    allocParams.pBufName          = "dump rgb surface";
    allocParams.bIsCompressible   = false;
    allocParams.ResUsageType      = MOS_HW_RESOURCE_DEF_MAX;
    allocParams.m_tileModeByForce = MOS_TILE_UNSET_GMM;

    CODECHAL_DEBUG_CHK_STATUS(m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParams,
        &tmpRgbSurface->OsResource));

    tmpRgbSurface->Format = Format_Invalid;
    CODECHAL_DEBUG_CHK_STATUS(m_osInterface->pfnGetResourceInfo(m_osInterface, &tmpRgbSurface->OsResource, tmpRgbSurface));

    uint32_t rgbPitch      = tmpRgbSurface->dwPitch;
    uint32_t alignedHeight = surface->dwHeight;
    uint32_t alignedWidth  = surface->dwWidth;

    if (outputFormat == Format_RGBP || outputFormat == Format_BGRP && rgbPitch)
    {
        alignedHeight = tmpRgbSurface->RenderOffset.YUV.U.BaseOffset / rgbPitch;
        alignedWidth  = rgbPitch;
    }

    // Always use MOS swizzle instead of GMM Cpu blit
    uint32_t sizeMain = (uint32_t)(surface->OsResource.pGmmResInfo->GetSizeMainSurface());
    surfBaseAddr      = (uint8_t *)MOS_AllocMemory(sizeMain);
    CODECHAL_DEBUG_CHK_NULL(surfBaseAddr);

    int32_t  swizzleHeight = sizeMain / surface->dwPitch;
    int32_t  swizzlePitch  = surface->dwPitch;
    uint8_t *data          = lockedAddr;

    CODECHAL_DEBUG_VERBOSEMESSAGE("RGB Surface info: format %d, tiletype %d, pitch %d,UOffset %d, dwSize %d, sizemain %d",
        outputFormat,
        tmpRgbSurface->TileType,
        tmpRgbSurface->dwPitch,
        tmpRgbSurface->RenderOffset.YUV.U.BaseOffset,
        tmpRgbSurface->dwSize,
        sizeMain);

    if (outputTileType == MOS_TILE_Y)
    {
        swizzlePitch = rgbPitch;
        Mos_SwizzleData(lockedAddr, surfBaseAddr, outputTileType, MOS_TILE_LINEAR, sizeMain / swizzlePitch, swizzlePitch, !MEDIA_IS_SKU(m_osInterface->pfnGetSkuTable(m_osInterface), FtrTileY) || gmmFlags.Info.Tile4);
        data = surfBaseAddr;
    }

    data += surface->dwOffset + surface->YPlaneOffset.iYOffset * surface->dwPitch;

    uint32_t width      = (width_in ? width_in : surface->dwWidth) << 2;
    uint32_t height     = height_in ? height_in : surface->dwHeight;
    uint32_t lumaheight = 0;
    uint32_t pitch      = surface->dwPitch;

    lumaheight = hasAuxSurf ? GFX_ALIGN(height, 32) : height;

    const char *funcName = (m_codecFunction == CODECHAL_FUNCTION_DECODE) ? "_DEC" : (m_codecFunction == CODECHAL_FUNCTION_CENC_DECODE ? "_DEC" : "_ENC");
    std::string bufName  = std::string(surfName) + "_w[" + std::to_string(alignedWidth) + "]_h[" + std::to_string(alignedHeight) + "]_p[" + std::to_string(rgbPitch) + "]";

    const char *filePath = CreateFileName(funcName, bufName.c_str(), filePostfix);

    std::ofstream ofs(filePath, std::ios_base::out | std::ios_base::binary);
    if (ofs.fail())
    {
        return MOS_STATUS_UNKNOWN;
    }

    // RGB data has one zero byte in every 64 bytes, need to remove this byte
    if (IS_RGB24_FORMAT(outputFormat))
    {
        for (uint32_t h = 0; h < lumaheight; h++)
        {
            width                 = surface->dwWidth * 3;
            int dummyBytesPerLine = width / 63;
            int resPixel          = width - 63 * dummyBytesPerLine;
            int resBytesPerLine   = rgbPitch;
            for (int p = 0; p < dummyBytesPerLine; p++)
            {
                ofs.write((char *)data, 63);
                data += 64;
                resBytesPerLine -= 64;
            }

            if (resPixel > 0)
            {
                ofs.write((char *)data, resPixel);
                data += resPixel;
                resBytesPerLine -= resPixel;
            }

            if (resBytesPerLine)
            {
                data += resBytesPerLine;
            }
        }
    }
    else
    {
        for (uint32_t h = 0; h < lumaheight; h++)
        {
            ofs.write((char *)data, hasAuxSurf ? pitch : width);
            data += pitch;
        }
    }

    if (surfBaseAddr)
    {
        MOS_FreeMemory(surfBaseAddr);
    }
    if (lockedAddr)
    {
        m_osInterface->pfnUnlockResource(m_osInterface, &surface->OsResource);
    }

    if (&(tmpRgbSurface->OsResource))
    {
        m_osInterface->pfnFreeResource(m_osInterface, &(tmpRgbSurface->OsResource));
    }
    if (tmpRgbSurface)
    {
        MOS_Delete(tmpRgbSurface);
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalDebugInterface::DumpBltOutput(
    PMOS_SURFACE surface,
    const char  *attrName)
{
    if (!DumpIsEnabled(attrName))
    {
        return MOS_STATUS_SUCCESS;
    }
    else
    {
        CODECHAL_DEBUG_NORMALMESSAGE("Don't support BltOutput dump, pls not use BltState directly!");
        return MOS_STATUS_SUCCESS;
    }

}

MOS_STATUS CodechalDebugInterface::InitializeUserSetting()
{
    CODECHAL_DEBUG_FUNCTION_ENTER;
    CODECHAL_DEBUG_CHK_STATUS(MediaDebugInterface::InitializeUserSetting());

    DeclareUserSettingKeyForDebug(
        m_userSettingPtr,
        __MEDIA_USER_FEATURE_ENABLE_HW_DEBUG_HOOKS_DEBUG,
        MediaUserSetting::Group::Device,
        int32_t(0),
        false);

    DeclareUserSettingKeyForDebug(
        m_userSettingPtr,
        __MEDIA_USER_FEATURE_VALUE_CODECHAL_FRAME_NUMBER_TO_STOP_DEBUG,
        MediaUserSetting::Group::Device,
        int32_t(-1),
        false);

    DeclareUserSettingKeyForDebug(
        m_userSettingPtr,
        __MEDIA_USER_FEATURE_VALUE_CODECHAL_ENABLE_SW_CRC_DEBUG,
        MediaUserSetting::Group::Device,
        uint32_t(0),
        false);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalDebugInterface::Initialize(
    CodechalHwInterfaceNext *hwInterface,
    CODECHAL_FUNCTION        codecFunction,
    MediaCopyBaseState      *mediaCopy)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    CODECHAL_DEBUG_CHK_NULL(hwInterface);
    m_hwInterfaceNext  = hwInterface;
    m_codecFunction    = codecFunction;
    m_osInterface      = m_hwInterfaceNext->GetOsInterface();
    m_cpInterface      = m_hwInterfaceNext->GetCpInterface();

    CODECHAL_DEBUG_CHK_NULL(m_osInterface);
    m_userSettingPtr = m_osInterface->pfnGetUserSettingInstance(m_osInterface);
    CODECHAL_DEBUG_CHK_STATUS(InitializeUserSetting());

    //dump loctaion is codechaldump
    MediaDebugInterface::SetOutputFilePath();

    m_configMgr = MOS_New(CodecDebugConfigMgr, this, m_codecFunction, m_outputFilePath);
    CODECHAL_DEBUG_CHK_NULL(m_configMgr);
    CODECHAL_DEBUG_CHK_STATUS(m_configMgr->ParseConfig(m_osInterface->pOsContext));

    MediaDebugInterface::InitDumpLocation();

#if (_DEBUG || _RELEASE_INTERNAL)
    {
        MediaUserSetting::Value outValue;
        ReadUserSettingForDebug(
            m_userSettingPtr,
            outValue,
            __MEDIA_USER_FEATURE_ENABLE_HW_DEBUG_HOOKS_DEBUG,
            MediaUserSetting::Group::Device,
            0,
            true);
        m_enableHwDebugHooks = outValue.Get<bool>();
    }
    CheckGoldenReferenceExist();
    if (m_enableHwDebugHooks && m_goldenReferenceExist)
    {
        LoadGoldenReference();
    }

    {
        MediaUserSetting::Value outValue;
        ReadUserSettingForDebug(
            m_userSettingPtr,
            outValue,
            __MEDIA_USER_FEATURE_VALUE_CODECHAL_FRAME_NUMBER_TO_STOP_DEBUG,
            MediaUserSetting::Group::Device,
            -1,
            true);
        m_stopFrameNumber = outValue.Get<int32_t>();
    }

    {
        MediaUserSetting::Value outValue;
        ReadUserSettingForDebug(
            m_userSettingPtr,
            outValue,
            __MEDIA_USER_FEATURE_VALUE_CODECHAL_ENABLE_SW_CRC_DEBUG,
            MediaUserSetting::Group::Device,
            0,
            true);
        m_swCRC = outValue.Get<bool>();
    }
#endif

    SetFastDumpConfig(mediaCopy);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalDebugInterface::DumpHucDmem(
    PMOS_RESOURCE             dmemResource,
    uint32_t                  dmemSize,
    uint32_t                  hucPassNum,
    CodechalHucRegionDumpType dumpType)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (!m_configMgr->AttrIsEnabled(MediaDbgAttr::attrHuCDmem) && !MosUtilities::GetTraceSetting())
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DEBUG_CHK_NULL(dmemResource);
    if (Mos_ResourceIsNull(dmemResource))
    {
        return MOS_STATUS_NULL_POINTER;
    }

    std::string funcName = "";
    if (m_codecFunction == CODECHAL_FUNCTION_DECODE)
    {
        funcName = "DEC_";
    }
    else if (m_codecFunction == CODECHAL_FUNCTION_CENC_DECODE)
    {
        funcName = "DEC_Cenc_";
    }
    else
    {
        funcName = "ENC_";
    }

    std::string dmemName = MediaDbgBufferType::bufHucDmem;
    std::string passName = std::to_string(hucPassNum);
    switch (dumpType)
    {
    case hucRegionDumpInit:
        funcName = funcName + dmemName + "_InitPass" + passName;
        break;
    case hucRegionDumpUpdate:
        funcName = funcName + dmemName + "_UpdatePass" + passName;
        break;
    case hucRegionDumpLAUpdate:
        funcName = funcName + dmemName + "_LookaheadUpdatePass" + passName;
        break;
    case hucRegionDumpRegionLocked:
        funcName = funcName + dmemName + "_RegionLocked" + passName;
        break;
    case hucRegionDumpCmdInitializer:
        funcName = funcName + dmemName + "_CmdInitializerPass" + passName;
        break;
    case hucRegionDumpPakIntegrate:
        funcName = funcName + dmemName + "_PakIntPass" + passName;
        break;
    case hucRegionDumpHpu:
        funcName = funcName + dmemName + "_HpuPass" + passName;
        break;
    case hucRegionDumpHpuSuperFrame:
        funcName = funcName + dmemName + "_HpuPass" + passName + "_SuperFramePass";
        break;
    case hucRegionDumpBackAnnotation:
        funcName = funcName + dmemName + "_BackAnnotationPass" + passName;
        break;
    default:
        funcName = funcName + dmemName + "_Pass" + passName;
        break;
    }

    return DumpBuffer(dmemResource, MediaDbgAttr::attrHuCDmem, funcName.c_str(), dmemSize);
}

std::string CodechalDebugInterface::SetOutputPathKey()
{
    return __MEDIA_USER_FEATURE_VALUE_CODECHAL_DEBUG_OUTPUT_DIRECTORY;
}

std::string CodechalDebugInterface::InitDefaultOutput()
{
    m_outputFilePath.append(MEDIA_DEBUG_CODECHAL_DUMP_OUTPUT_FOLDER);
    return SetOutputPathKey();
}

MOS_STATUS CodechalDebugInterface::DumpHucRegion(
    PMOS_RESOURCE             region,
    uint32_t                  regionOffset,
    uint32_t                  regionSize,
    uint32_t                  regionNum,
    const char *              regionName,
    bool                      inputBuffer,
    uint32_t                  hucPassNum,
    CodechalHucRegionDumpType dumpType)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (!m_configMgr->AttrIsEnabled(MediaDbgAttr::attrHucRegions) && !MosUtilities::GetTraceSetting())
    {
        return MOS_STATUS_SUCCESS;
    }
    CODECHAL_DEBUG_ASSERT(regionNum < 16);
    CODECHAL_DEBUG_CHK_NULL(region);

    if (Mos_ResourceIsNull(region))
    {
        return MOS_STATUS_NULL_POINTER;
    }

    std::string funcName = "";
    if (m_codecFunction == CODECHAL_FUNCTION_DECODE)
    {
        funcName = "DEC_";
    }
    else if (m_codecFunction == CODECHAL_FUNCTION_CENC_DECODE)
    {
        funcName = "DEC_CENC_";
    }
    else
    {
        funcName = "ENC_";
    }

    std::string bufName       = MediaDbgBufferType::bufHucRegion;
    std::string inputName     = (inputBuffer) ? "Input_" : "Output_";
    std::string regionNumName = std::to_string(regionNum);
    std::string passName      = std::to_string(hucPassNum);
    switch (dumpType)
    {
    case hucRegionDumpInit:
        funcName = funcName + inputName + bufName + regionNumName + regionName + "_InitPass" + passName;
        break;
    case hucRegionDumpUpdate:
        funcName = funcName + inputName + bufName + regionNumName + regionName + "_UpdatePass" + passName;
        break;
    case hucRegionDumpLAUpdate:
        funcName = funcName + inputName + bufName + regionNumName + regionName + "_LookaheadUpdatePass" + passName;
        break;
    case hucRegionDumpRegionLocked:
        funcName = funcName + inputName + bufName + regionNumName + regionName + "_RegionLockedPass" + passName;
        break;
    case hucRegionDumpCmdInitializer:
        funcName = funcName + inputName + bufName + regionNumName + regionName + "_CmdInitializerPass" + passName;
        break;
    case hucRegionDumpPakIntegrate:
        funcName = funcName + inputName + bufName + regionNumName + regionName + "_PakIntPass" + passName;
        break;
    case hucRegionDumpHpu:
        funcName = funcName + inputName + bufName + regionNumName + regionName + "_HpuPass" + passName;
        break;
    case hucRegionDumpBackAnnotation:
        funcName = funcName + inputName + bufName + regionNumName + regionName + "_BackAnnotationPass" + passName;
        break;
    default:
        funcName = funcName + inputName + bufName + regionNumName + regionName + "_Pass" + passName;
        break;
    }

    return DumpBuffer(region, MediaDbgAttr::attrHucRegions, funcName.c_str(), regionSize, regionOffset);
}

#define FIELD_TO_OFS(field_name) ofs << print_shift << std::setfill(' ') << std::setw(25) << std::left << std::string(#field_name) + ": " << (int64_t)report->field_name << std::endl;
#define PTR_TO_OFS(ptr_name) ofs << print_shift << std::setfill(' ') << std::setw(25) << std::left << std::string(#ptr_name) + ": " << report->ptr_name << std::endl;
MOS_STATUS CodechalDebugInterface::DumpEncodeStatusReport(const encode::EncodeStatusReportData *report)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    CODECHAL_DEBUG_CHK_NULL(report);

    const char *bufferName = "EncodeStatusReport_Parsed";
    const char *attrName   = MediaDbgAttr::attrStatusReport;
    if (!m_configMgr->AttrIsEnabled(attrName))
    {
        return MOS_STATUS_SUCCESS;
    }

    const char *  filePath = CreateFileName(bufferName, attrName, MediaDbgExtType::txt);
    std::ofstream ofs(filePath);

    if (ofs.fail())
    {
        return MOS_STATUS_UNKNOWN;
    }
    std::string print_shift = "";
    sizeof(report->codecStatus);
    FIELD_TO_OFS(codecStatus);
    FIELD_TO_OFS(statusReportNumber);
    FIELD_TO_OFS(currOriginalPic.FrameIdx);
    FIELD_TO_OFS(currOriginalPic.PicFlags);
    FIELD_TO_OFS(currOriginalPic.PicEntry);
    FIELD_TO_OFS(func);
    PTR_TO_OFS(  currRefList);
    ofs << std::endl;

    FIELD_TO_OFS(sequential);
    FIELD_TO_OFS(bitstreamSize);
    FIELD_TO_OFS(qpY);
    FIELD_TO_OFS(suggestedQPYDelta);
    FIELD_TO_OFS(numberPasses);
    FIELD_TO_OFS(averageQP);
    FIELD_TO_OFS(hwCounterValue.IV);
    FIELD_TO_OFS(hwCounterValue.Count);
    PTR_TO_OFS(hwCtr);
    FIELD_TO_OFS(queryStatusFlags);

    print_shift = "    ";
    FIELD_TO_OFS(panicMode);
    FIELD_TO_OFS(sliceSizeOverflow);
    FIELD_TO_OFS(numSlicesNonCompliant);
    FIELD_TO_OFS(longTermReference);
    FIELD_TO_OFS(frameSkipped);
    FIELD_TO_OFS(sceneChangeDetected);
    print_shift = "";
    ofs << std::endl;

    FIELD_TO_OFS(mad);
    FIELD_TO_OFS(loopFilterLevel);
    FIELD_TO_OFS(longTermIndication);
    FIELD_TO_OFS(nextFrameWidthMinus1);
    FIELD_TO_OFS(nextFrameHeightMinus1);
    FIELD_TO_OFS(numberSlices);

    FIELD_TO_OFS(psnrX100[0]);
    FIELD_TO_OFS(psnrX100[1]);
    FIELD_TO_OFS(psnrX100[2]);

    FIELD_TO_OFS(numberTilesInFrame);
    FIELD_TO_OFS(usedVdBoxNumber);
    FIELD_TO_OFS(sizeOfSliceSizesBuffer);
    PTR_TO_OFS(  sliceSizes);
    FIELD_TO_OFS(sizeOfTileInfoBuffer);
    PTR_TO_OFS(  hevcTileinfo);
    FIELD_TO_OFS(numTileReported);
    ofs << std::endl;

    FIELD_TO_OFS(streamId);
    PTR_TO_OFS(  pLookaheadStatus);
    ofs.close();

    return MOS_STATUS_SUCCESS;
}

#undef FIELD_TO_OFS
#undef PTR_TO_OFS

bool CodechalDebugInterface::DumpIsEnabled(
    const char *              attr,
    CODECHAL_MEDIA_STATE_TYPE mediaState)
{
    if (nullptr == m_configMgr)
    {
        return false;
    }

    if (mediaState != CODECHAL_NUM_MEDIA_STATES)
    {
        return static_cast<CodecDebugConfigMgr*>(m_configMgr)->AttrIsEnabled(mediaState, attr);
    }
    else
    {
        return m_configMgr->AttrIsEnabled(attr);
    }
}

MOS_STATUS CodechalDebugInterface::SetFastDumpConfig(MediaCopyBaseState *mediaCopy)
{
    auto traceSetting = MosUtilities::GetTraceSetting();
    if (!mediaCopy || !(DumpIsEnabled(MediaDbgAttr::attrEnableFastDump) || traceSetting))
    {
        return MOS_STATUS_SUCCESS;
    }

    MediaDebugFastDump::Config cfg{};
    if (traceSetting)
    {
        const auto &c           = traceSetting->fastDump;
        cfg.allowDataLoss       = c.allowDataLoss;
        cfg.frameIdx            = c.frameIdxBasedSampling ? &m_bufferDumpFrameNum : nullptr;
        cfg.samplingTime        = static_cast<size_t>(c.samplingTime);
        cfg.samplingInterval    = static_cast<size_t>(c.samplingInterval);
        cfg.memUsagePolicy      = c.memUsagePolicy;
        cfg.maxPrioritizedMem   = c.maxPrioritizedMem;
        cfg.maxDeprioritizedMem = c.maxDeprioritizedMem;
        cfg.weightRenderCopy    = c.weightRenderCopy;
        cfg.weightVECopy        = c.weightVECopy;
        cfg.weightBLTCopy       = c.weightBLTCopy;
        cfg.writeMode           = c.writeMode;
        cfg.bufferSize          = static_cast<size_t>(c.bufferSize);
        cfg.informOnError       = c.informOnError;

        auto suffix = cfg.writeMode < 2 ? ".bin" : cfg.writeMode == 2 ? ".txt"
                                                                      : "";

        class DumpEnabled
        {
        public:
            bool operator()(const char *attrName)
            {
                decltype(m_filter)::const_iterator it;
                return (it = m_filter.find(attrName)) != m_filter.end() &&
                       MOS_TraceKeyEnabled(it->second);
            }

        private:
            const std::map<std::string, MEDIA_EVENT_FILTER_KEYID> m_filter = {
                {MediaDbgAttr::attrDecodeOutputSurface, TR_KEY_DECODE_DSTYUV},
                {MediaDbgAttr::attrDecodeReferenceSurfaces, TR_KEY_DECODE_REFYUV},
                {MediaDbgAttr::attrDecodeBitstream, TR_KEY_DECODE_BITSTREAM},
                {MediaDbgAttr::attrEncodeRawInputSurface, TR_KEY_ENCODE_DATA_INPUT_SURFACE},
                {MediaDbgAttr::attrReferenceSurfaces, TR_KEY_ENCODE_DATA_REF_SURFACE},
                {MediaDbgAttr::attrReconstructedSurface, TR_KEY_ENCODE_DATA_RECON_SURFACE},
                {MediaDbgAttr::attrBitstream, TR_KEY_ENCODE_DATA_BITSTREAM},
                {MediaDbgAttr::attrHuCDmem, TR_KEY_ENCODE_DATA_HUC_DMEM},
                {MediaDbgAttr::attrHucRegions, TR_KEY_ENCODE_DATA_HUC_REGION},
            };
        };

        auto dumpEnabled = std::make_shared<DumpEnabled>();

        m_dumpYUVSurface = [this, dumpEnabled, traceSetting, suffix](
                               PMOS_SURFACE              surface,
                               const char               *attrName,
                               const char               *surfName,
                               CODECHAL_MEDIA_STATE_TYPE mediaState,
                               uint32_t,
                               uint32_t) {
            if ((*dumpEnabled)(attrName))
            {
                MediaDebugFastDump::Dump(
                    surface->OsResource,
                    std::string(traceSetting->fastDump.filePath) +
                        std::to_string(m_bufferDumpFrameNum) +
                        '-' +
                        surfName +
                        "w[0]_h[0]_p[0]" +
                        suffix);
            }
            return MOS_STATUS_SUCCESS;
        };

        m_dumpBuffer = [this, dumpEnabled, traceSetting, suffix](
                           PMOS_RESOURCE             resource,
                           const char               *attrName,
                           const char               *compName,
                           uint32_t                  size,
                           uint32_t                  offset,
                           CODECHAL_MEDIA_STATE_TYPE mediaState) {
            if ((*dumpEnabled)(attrName))
            {
                std::string bufferName = "";

                if (!strcmp(attrName, "DecodeBitstream") || !strcmp(attrName, "Bitstream"))
                {
                    bufferName = "_Bitstream";
                }

                MediaDebugFastDump::Dump(
                    *resource,
                    std::string(traceSetting->fastDump.filePath) +
                        std::to_string(m_bufferDumpFrameNum) +
                        '-' +
                        compName + bufferName +
                        suffix,
                    size,
                    offset);
            }
            return MOS_STATUS_SUCCESS;
        };
    }
    else
    {
        cfg.allowDataLoss = DumpIsEnabled(MediaDbgAttr::attrFastDumpAllowDataLoss);
        cfg.informOnError = DumpIsEnabled(MediaDbgAttr::attrFastDumpInformOnError);
    }

    MediaDebugFastDump::CreateInstance(*m_osInterface, *mediaCopy, &cfg);

    return MOS_STATUS_SUCCESS;
}

const char *CodechalDebugInterface::CreateFileName(
    const char *funcName,
    const char *bufType,
    const char *extType)
{
    if (nullptr == funcName || nullptr == extType)
    {
        return nullptr;
    }

    char frameType = 'X';
    // Sets the frameType label
    if (m_frameType == I_TYPE)
    {
        frameType = 'I';
    }
    else if (m_frameType == P_TYPE)
    {
        frameType = 'P';
    }
    else if (m_frameType == B_TYPE)
    {
        frameType = 'B';
    }
    else if (m_frameType == MIXED_TYPE)
    {
        frameType = 'M';
    }

    const char *fieldOrder;
    // Sets the Field Order label
    if (CodecHal_PictureIsTopField(m_currPic))
    {
        fieldOrder = MediaDbgFieldType::topField;
    }
    else if (CodecHal_PictureIsBottomField(m_currPic))
    {
        fieldOrder = MediaDbgFieldType::botField;
    }
    else
    {
        fieldOrder = MediaDbgFieldType::frame;
    }

    // Sets the Postfix label
    if (m_configMgr->AttrIsEnabled(MediaDbgAttr::attrDumpBufferInBinary) &&
        strcmp(extType, MediaDbgExtType::txt) == 0)
    {
        extType = MediaDbgExtType::dat;
    }

    if (bufType != nullptr &&
        !strncmp(bufType, MediaDbgBufferType::bufSlcParams, sizeof(MediaDbgBufferType::bufSlcParams) - 1) && !strncmp(funcName, "_DDIEnc", sizeof("_DDIEnc") - 1))
    {
        m_outputFileName = m_outputFilePath +
                           std::to_string(m_bufferDumpFrameNum) + '-' +
                           std::to_string(m_streamId) + '_' +
                           std::to_string(m_sliceId + 1) +
                           funcName + '_' + bufType + '_' + frameType + fieldOrder + extType;
    }
    else if (bufType != nullptr &&
             !strncmp(bufType, MediaDbgBufferType::bufEncodePar, sizeof(MediaDbgBufferType::bufEncodePar) - 1))
    {
        if (!strncmp(funcName, "EncodeSequence", sizeof("EncodeSequence") - 1))
        {
            m_outputFileName = m_outputFilePath +
                               std::to_string(m_streamId) + '_' +
                               funcName + extType;
        }
        else
        {
            m_outputFileName = m_outputFilePath +
                               std::to_string(m_bufferDumpFrameNum) + '-' +
                               std::to_string(m_streamId) + '_' +
                               funcName + frameType + fieldOrder + extType;
        }
    }
    else
    {
        if (funcName[0] == '_')
            funcName += 1;

        if (bufType != nullptr)
        {
            m_outputFileName = m_outputFilePath +
                               std::to_string(m_bufferDumpFrameNum) + '-' +
                               std::to_string(m_streamId) + '_' +
                               funcName + '_' + bufType + '_' + frameType + fieldOrder + extType;
        }
        else
        {
            m_outputFileName = m_outputFilePath +
                               std::to_string(m_bufferDumpFrameNum) + '-' +
                               std::to_string(m_streamId) + '_' +
                               funcName + '_' + frameType + fieldOrder + extType;
        }
    }

    return m_outputFileName.c_str();
}

MOS_STATUS CodechalDebugInterface::DumpStringStream(std::stringstream& ss, const char* bufferName, const char* attrName)
{
    MEDIA_DEBUG_FUNCTION_ENTER;

    MEDIA_DEBUG_CHK_NULL(bufferName);
    MEDIA_DEBUG_CHK_NULL(attrName);

    if (!m_configMgr->AttrIsEnabled(attrName))
    {
        return MOS_STATUS_SUCCESS;
    }

    const char* filePath = CreateFileName(bufferName, nullptr, MediaDbgExtType::txt);
    std::ofstream ofs(filePath);
    ofs << ss.str();
    ofs.close();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalDebugInterface::DumpCmdBuffer(
    PMOS_COMMAND_BUFFER       cmdBuffer,
    CODECHAL_MEDIA_STATE_TYPE mediaState,
    const char *              cmdName)
{
    MEDIA_DEBUG_FUNCTION_ENTER;

    bool attrEnabled = m_configMgr->AttrIsEnabled(MediaDbgAttr::attrCmdBufferMfx);

    if (!attrEnabled && mediaState != CODECHAL_NUM_MEDIA_STATES)
    {
        attrEnabled = static_cast<CodecDebugConfigMgr*>(m_configMgr)->AttrIsEnabled(mediaState, MediaDbgAttr::attrCmdBuffer);
    }

    if (!attrEnabled)
    {
        return MOS_STATUS_SUCCESS;
    }

    bool binaryDumpEnabled = m_configMgr->AttrIsEnabled(MediaDbgAttr::attrDumpCmdBufInBinary);

    std::string funcName = cmdName ? cmdName : static_cast<CodecDebugConfigMgr*>(m_configMgr)->GetMediaStateStr(mediaState);
    const char *fileName = CreateFileName(
        funcName.c_str(),
        MediaDbgBufferType::bufCmd,
        binaryDumpEnabled ? MediaDbgExtType::dat : MediaDbgExtType::txt);

    if (binaryDumpEnabled)
    {
        DumpBufferInBinary((uint8_t *)cmdBuffer->pCmdBase, (uint32_t)cmdBuffer->iOffset);
    }
    else
    {
        DumpBufferInHexDwords((uint8_t *)cmdBuffer->pCmdBase, (uint32_t)cmdBuffer->iOffset);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalDebugInterface::Dump2ndLvlBatch(
    PMHW_BATCH_BUFFER         batchBuffer,
    CODECHAL_MEDIA_STATE_TYPE mediaState,
    const char *              batchName)
{
    MEDIA_DEBUG_FUNCTION_ENTER;

    bool attrEnabled = m_configMgr->AttrIsEnabled(MediaDbgAttr::attr2ndLvlBatchMfx);

    if (!attrEnabled && mediaState != CODECHAL_NUM_MEDIA_STATES)
    {
        attrEnabled = static_cast<CodecDebugConfigMgr*>(m_configMgr)->AttrIsEnabled(mediaState, MediaDbgAttr::attr2ndLvlBatch);
    }

    if (!attrEnabled)
    {
        return MOS_STATUS_SUCCESS;
    }

    bool        batchLockedForDebug = !batchBuffer->bLocked;
    std::string funcName            = batchName ? batchName : static_cast<CodecDebugConfigMgr*>(m_configMgr)->GetMediaStateStr(mediaState);

    if (batchLockedForDebug)
    {
        (Mhw_LockBb(m_osInterface, batchBuffer));
    }

    const char *fileName = CreateFileName(
        funcName.c_str(),
        MediaDbgBufferType::buf2ndLvl,
        MediaDbgExtType::txt);

    batchBuffer->pData += batchBuffer->dwOffset;

    DumpBufferInHexDwords(batchBuffer->pData,
        (uint32_t)batchBuffer->iLastCurrent);

    if (batchLockedForDebug)
    {
        (Mhw_UnlockBb(m_osInterface, batchBuffer, false));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalDebugInterface::DumpCurbe(
    CODECHAL_MEDIA_STATE_TYPE mediaState,
    PMHW_KERNEL_STATE         kernelState)
{
    MEDIA_DEBUG_FUNCTION_ENTER;

    if (mediaState >= CODECHAL_NUM_MEDIA_STATES ||
        !static_cast<CodecDebugConfigMgr*>(m_configMgr)->AttrIsEnabled(mediaState, MediaDbgAttr::attrCurbe))
    {
        return MOS_STATUS_SUCCESS;
    }

    std::string funcName   = static_cast<CodecDebugConfigMgr*>(m_configMgr)->GetMediaStateStr(mediaState);
    bool        binaryDump = m_configMgr->AttrIsEnabled(MediaDbgAttr::attrDumpBufferInBinary);

    const char *fileName = CreateFileName(
        funcName.c_str(),
        MediaDbgBufferType::bufCurbe,
        MediaDbgExtType::txt);

    return kernelState->m_dshRegion.Dump(
        fileName,
        kernelState->dwCurbeOffset,
        kernelState->KernelParams.iCurbeLength,
        binaryDump);
}

MOS_STATUS CodechalDebugInterface::DumpMDFCurbe(
    CODECHAL_MEDIA_STATE_TYPE mediaState,
    uint8_t *                 curbeBuffer,
    uint32_t                  curbeSize)
{
    MEDIA_DEBUG_FUNCTION_ENTER;

    uint8_t *  curbeAlignedData = nullptr;
    uint32_t   curbeAlignedSize = 0;
    MOS_STATUS eStatus          = MOS_STATUS_SUCCESS;

    if (mediaState >= CODECHAL_NUM_MEDIA_STATES ||
        !static_cast<CodecDebugConfigMgr*>(m_configMgr)->AttrIsEnabled(mediaState, MediaDbgAttr::attrCurbe))
    {
        return eStatus;
    }

    std::string funcName   = static_cast<CodecDebugConfigMgr*>(m_configMgr)->GetMediaStateStr(mediaState);
    bool        binaryDump = m_configMgr->AttrIsEnabled(MediaDbgAttr::attrDumpBufferInBinary);
    const char *extType    = binaryDump ? MediaDbgExtType::dat : MediaDbgExtType::txt;

    const char *fileName = CreateFileName(
        funcName.c_str(),
        MediaDbgBufferType::bufCurbe,
        extType);

    curbeAlignedSize = MOS_ALIGN_CEIL(curbeSize, 64);
    curbeAlignedData = (uint8_t *)malloc(curbeAlignedSize * sizeof(uint8_t));
    if (curbeAlignedData == nullptr)
    {
        eStatus = MOS_STATUS_NULL_POINTER;
        return eStatus;
    }

    MOS_ZeroMemory(curbeAlignedData, curbeAlignedSize);
    MOS_SecureMemcpy(curbeAlignedData, curbeSize, curbeBuffer, curbeSize);

    if (binaryDump)
    {
        eStatus = DumpBufferInBinary(curbeAlignedData, curbeAlignedSize);
    }
    else
    {
        eStatus = DumpBufferInHexDwords(curbeAlignedData, curbeAlignedSize);
    }

    free(curbeAlignedData);

    return eStatus;
}

MOS_STATUS CodechalDebugInterface::DumpKernelRegion(
    CODECHAL_MEDIA_STATE_TYPE mediaState,
    MHW_STATE_HEAP_TYPE       stateHeap,
    PMHW_KERNEL_STATE         kernelState)
{
    MEDIA_DEBUG_FUNCTION_ENTER;

    uint8_t *sshData = nullptr;
    uint32_t sshSize = 0;

    MemoryBlock *regionBlock = nullptr;
    bool         attrEnabled = false;
    const char * bufferType;
    if (stateHeap == MHW_ISH_TYPE)
    {
        regionBlock = &kernelState->m_ishRegion;
        attrEnabled = static_cast<CodecDebugConfigMgr*>(m_configMgr)->AttrIsEnabled(mediaState, MediaDbgAttr::attrIsh);
        bufferType  = MediaDbgBufferType::bufISH;
    }
    else if (stateHeap == MHW_DSH_TYPE)
    {
        regionBlock = &kernelState->m_dshRegion;
        attrEnabled = static_cast<CodecDebugConfigMgr*>(m_configMgr)->AttrIsEnabled(mediaState, MediaDbgAttr::attrDsh);
        bufferType  = MediaDbgBufferType::bufDSH;
    }
    else
    {
        attrEnabled = static_cast<CodecDebugConfigMgr*>(m_configMgr)->AttrIsEnabled(mediaState, MediaDbgAttr::attrSsh);
        bufferType  = MediaDbgBufferType::bufSSH;

        MEDIA_DEBUG_CHK_NULL(m_osInterface);
        MEDIA_DEBUG_CHK_STATUS(m_osInterface->pfnGetIndirectStatePointer(
            m_osInterface,
            &sshData));
        sshData += kernelState->dwSshOffset;
        sshSize = kernelState->dwSshSize;
    }

    if (!attrEnabled)
    {
        return MOS_STATUS_SUCCESS;
    }

    std::string funcName = static_cast<CodecDebugConfigMgr*>(m_configMgr)->GetMediaStateStr(mediaState);

    const char *fileName = CreateFileName(
        funcName.c_str(),
        bufferType,
        MediaDbgExtType::txt);

    bool binaryDump = m_configMgr->AttrIsEnabled(MediaDbgAttr::attrDumpBufferInBinary);

    if (regionBlock)
    {
        return regionBlock->Dump(fileName, 0, 0, binaryDump);
    }
    else
    {
        return DumpBufferInHexDwords(sshData, sshSize);
    }
}

MOS_STATUS CodechalDebugInterface::DumpYUVSurfaceToBuffer(PMOS_SURFACE surface,
    uint8_t *                                                          buffer,
    uint32_t &                                                         size)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MOS_LOCK_PARAMS lockFlags;
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.ReadOnly     = 1;
    lockFlags.TiledAsTiled = 1;  // Bypass GMM CPU blit due to some issues in GMM CpuBlt function

    uint8_t *lockedAddr = (uint8_t *)m_osInterface->pfnLockResource(m_osInterface, &surface->OsResource, &lockFlags);
    if (lockedAddr == nullptr)  // Failed to lock. Try to submit copy task and dump another surface
    {
        uint32_t        sizeToBeCopied = 0;
        MOS_GFXRES_TYPE ResType;

#if LINUX
        // Linux does not have OsResource->ResType
        ResType = surface->Type;
#else
        ResType = surface->OsResource.ResType;
#endif

        GMM_RESOURCE_FLAG gmmFlags  = surface->OsResource.pGmmResInfo->GetResFlags();
        bool              allocated = false;

        MEDIA_DEBUG_CHK_STATUS(ReAllocateSurface(
            &m_temp2DSurfForCopy,
            surface,
            "Temp2DSurfForSurfDumper",
            ResType));

        // Ensure allocated buffer size contains the source surface size
        if (m_temp2DSurfForCopy.OsResource.pGmmResInfo->GetSizeMainSurface() >= surface->OsResource.pGmmResInfo->GetSizeMainSurface())
        {
            sizeToBeCopied = (uint32_t)surface->OsResource.pGmmResInfo->GetSizeMainSurface();
        }

        if (sizeToBeCopied == 0)
        {
            // Currently, MOS's pfnAllocateResource does not support allocate a surface reference to another surface.
            // When the source surface is not created from Media, it is possible that we cannot allocate the same size as source.
            // For example, on Gen9, Render target might have GMM set CCS=1 MMC=0, but MOS cannot allocate surface with such combination.
            // When Gmm allocation parameter is different, the resulting surface size/padding/pitch will be differnt.
            // Once if MOS can support allocate a surface by reference another surface, we can do a bit to bit copy without problem.
            MEDIA_DEBUG_ASSERTMESSAGE("Cannot allocate correct size, failed to copy nonlockable resource");
            return MOS_STATUS_NULL_POINTER;
        }

        MEDIA_DEBUG_VERBOSEMESSAGE("Temp2DSurfaceForCopy width %d, height %d, pitch %d, TileType %d, bIsCompressed %d, CompressionMode %d",
            m_temp2DSurfForCopy.dwWidth,
            m_temp2DSurfForCopy.dwHeight,
            m_temp2DSurfForCopy.dwPitch,
            m_temp2DSurfForCopy.TileType,
            m_temp2DSurfForCopy.bIsCompressed,
            m_temp2DSurfForCopy.CompressionMode);

        if (CopySurfaceData_Vdbox(sizeToBeCopied, &surface->OsResource, &m_temp2DSurfForCopy.OsResource) != MOS_STATUS_SUCCESS)
        {
            MEDIA_DEBUG_ASSERTMESSAGE("CopyDataSurface_Vdbox failed");
            m_osInterface->pfnFreeResource(m_osInterface, &m_temp2DSurfForCopy.OsResource);
            return MOS_STATUS_NULL_POINTER;
        }
        lockedAddr = (uint8_t *)m_osInterface->pfnLockResource(m_osInterface, &m_temp2DSurfForCopy.OsResource, &lockFlags);
        MEDIA_DEBUG_CHK_NULL(lockedAddr);
    }

    uint32_t sizeMain     = (uint32_t)(surface->OsResource.pGmmResInfo->GetSizeMainSurface());
    uint8_t *surfBaseAddr = (uint8_t *)MOS_AllocMemory(sizeMain);
    MEDIA_DEBUG_CHK_NULL(surfBaseAddr);

    Mos_SwizzleData(lockedAddr, surfBaseAddr, surface->TileType, MOS_TILE_LINEAR, sizeMain / surface->dwPitch, surface->dwPitch, 0);

    uint8_t *data = surfBaseAddr;
    data += surface->dwOffset + surface->YPlaneOffset.iYOffset * surface->dwPitch;

    uint32_t width  = surface->dwWidth;
    uint32_t height = surface->dwHeight;

    switch (surface->Format)
    {
    case Format_YUY2:
    case Format_Y216V:
    case Format_P010:
    case Format_P016:
        width = width << 1;
        break;
    case Format_Y216:
    case Format_Y210:  //422 10bit -- Y0[15:0]:U[15:0]:Y1[15:0]:V[15:0] = 32bits per pixel = 4Bytes per pixel
    case Format_Y410:  //444 10bit -- A[31:30]:V[29:20]:Y[19:10]:U[9:0] = 32bits per pixel = 4Bytes per pixel
    case Format_R10G10B10A2:
    case Format_AYUV:  //444 8bit  -- A[31:24]:Y[23:16]:U[15:8]:V[7:0] = 32bits per pixel = 4Bytes per pixel
    case Format_A8R8G8B8:
        width = width << 2;
        break;
    default:
        break;
    }

    uint32_t pitch = surface->dwPitch;
    if (surface->Format == Format_UYVY)
        pitch = width;

    if (CodecHal_PictureIsBottomField(m_currPic))
    {
        data += pitch;
    }

    if (CodecHal_PictureIsField(m_currPic))
    {
        pitch *= 2;
        height /= 2;
    }

    // write luma data to file
    for (uint32_t h = 0; h < height; h++)
    {
        MOS_SecureMemcpy(buffer, width, data, width);
        buffer += width;
        size += width;
        data += pitch;
    }

    if (surface->Format != Format_A8B8G8R8)
    {
        switch (surface->Format)
        {
        case Format_NV12:
        case Format_P010:
        case Format_P016:
            height >>= 1;
            break;
        case Format_Y416:
        case Format_AUYV:
        case Format_R10G10B10A2:
            height *= 2;
            break;
        case Format_YUY2:
        case Format_YUYV:
        case Format_YUY2V:
        case Format_Y216V:
        case Format_YVYU:
        case Format_UYVY:
        case Format_VYUY:
        case Format_Y216:  //422 16bit
        case Format_Y210:  //422 10bit
        case Format_P208:  //422 8bit
            break;
        case Format_422V:
        case Format_IMC3:
            height = height / 2;
            break;
        case Format_AYUV:
        default:
            height = 0;
            break;
        }

        uint8_t *vPlaneData = surfBaseAddr;
#ifdef LINUX
        data = surfBaseAddr + surface->UPlaneOffset.iSurfaceOffset;
        if (surface->Format == Format_422V || surface->Format == Format_IMC3)
        {
            vPlaneData = surfBaseAddr + surface->VPlaneOffset.iSurfaceOffset;
        }
#else
        data = surfBaseAddr + surface->UPlaneOffset.iLockSurfaceOffset;
        if (surface->Format == Format_422V || surface->Format == Format_IMC3)
        {
            vPlaneData = surfBaseAddr + surface->VPlaneOffset.iLockSurfaceOffset;
        }

#endif

        // write chroma data to file
        for (uint32_t h = 0; h < height; h++)
        {
            MOS_SecureMemcpy(buffer, width, data, width);
            buffer += width;
            size += width;
            data += pitch;
        }

        // write v planar data to file
        if (surface->Format == Format_422V || surface->Format == Format_IMC3)
        {
            for (uint32_t h = 0; h < height; h++)
            {
                MOS_SecureMemcpy(buffer, width, vPlaneData, width);
                buffer += width;
                size += width;
                vPlaneData += pitch;
            }
        }
    }

    m_osInterface->pfnUnlockResource(m_osInterface, &surface->OsResource);

    MOS_FreeMemory(surfBaseAddr);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalDebugInterface::DumpYUVSurface(
    PMOS_SURFACE              surface,
    const char *              attrName,
    const char *              surfName,
    CODECHAL_MEDIA_STATE_TYPE mediaState,
    uint32_t                  width_in,
    uint32_t                  height_in)
{
    return m_dumpYUVSurface(
        surface,
        attrName,
        surfName,
        mediaState,
        width_in,
        height_in);
}

MOS_STATUS CodechalDebugInterface::DumpBuffer(
    PMOS_RESOURCE             resource,
    const char *              attrName,
    const char *              bufferName,
    uint32_t                  size,
    uint32_t                  offset,
    CODECHAL_MEDIA_STATE_TYPE mediaState)
{
    return m_dumpBuffer(
        resource,
        attrName,
        bufferName,
        size,
        offset,
        mediaState);
}

MOS_STATUS CodechalDebugInterface::DumpSurface(
    PMOS_SURFACE              surface,
    const char *              attrName,
    const char *              surfaceName,
    CODECHAL_MEDIA_STATE_TYPE mediaState)
{
    MEDIA_DEBUG_FUNCTION_ENTER;

    MEDIA_DEBUG_CHK_NULL(surface);
    MEDIA_DEBUG_CHK_NULL(attrName);
    MEDIA_DEBUG_CHK_NULL(surfaceName);

    bool attrEnabled = false;

    if (mediaState == CODECHAL_NUM_MEDIA_STATES)
    {
        attrEnabled = m_configMgr->AttrIsEnabled(attrName);
    }
    else
    {
        attrEnabled = static_cast<CodecDebugConfigMgr*>(m_configMgr)->AttrIsEnabled(mediaState, attrName);
    }

    if (!attrEnabled)
    {
        return MOS_STATUS_SUCCESS;
    }

    bool        binaryDump = m_configMgr->AttrIsEnabled(MediaDbgAttr::attrDumpBufferInBinary);
    const char *extType    = binaryDump ? MediaDbgExtType::dat : MediaDbgExtType::txt;

    MOS_LOCK_PARAMS lockFlags;
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.ReadOnly = 1;
    uint8_t *data      = (uint8_t *)m_osInterface->pfnLockResource(m_osInterface, &surface->OsResource, &lockFlags);
    MEDIA_DEBUG_CHK_NULL(data);

    std::string bufName = std::string(surfaceName) + "_w[" + std::to_string(surface->dwWidth) + "]_h[" + std::to_string(surface->dwHeight) + "]_p[" + std::to_string(surface->dwPitch) + "]";
    const char *fileName;
    if (mediaState == CODECHAL_NUM_MEDIA_STATES)
    {
        fileName = CreateFileName(bufName.c_str(), nullptr, extType);
    }
    else
    {
        std::string kernelName = static_cast<CodecDebugConfigMgr*>(m_configMgr)->GetMediaStateStr(mediaState);
        fileName               = CreateFileName(kernelName.c_str(), bufName.c_str(), extType);
    }

    MOS_STATUS status;
    if (binaryDump)
    {
        status = Dump2DBufferInBinary(data, surface->dwWidth, surface->dwHeight, surface->dwPitch);
    }
    else
    {
        status = DumpBufferInHexDwords(data, surface->dwHeight * surface->dwPitch);
    }

    if (data)
    {
        m_osInterface->pfnUnlockResource(m_osInterface, &surface->OsResource);
    }

    return status;
}

MOS_STATUS CodechalDebugInterface::DumpData(
    void *      data,
    uint32_t    size,
    const char *attrName,
    const char *bufferName)
{
    MEDIA_DEBUG_FUNCTION_ENTER;

    MEDIA_DEBUG_CHK_NULL(data);
    MEDIA_DEBUG_CHK_NULL(attrName);
    MEDIA_DEBUG_CHK_NULL(bufferName);

    if (!m_configMgr->AttrIsEnabled(attrName))
    {
        return MOS_STATUS_SUCCESS;
    }

    bool        binaryDump = m_configMgr->AttrIsEnabled(MediaDbgAttr::attrDumpBufferInBinary);
    const char *fileName   = CreateFileName(bufferName, nullptr, binaryDump ? MediaDbgExtType::dat : MediaDbgExtType::txt);

    if (binaryDump)
    {
        DumpBufferInBinary((uint8_t *)data, size);
    }
    else
    {
        DumpBufferInHexDwords((uint8_t *)data, size);
    }

    return MOS_STATUS_SUCCESS;
}

#define FIELD_TO_OFS(name, shift) ofs << shift #name << ": " << (int64_t)ptr->name << std::endl;
#define EMPTY_TO_OFS()
#define UNION_STRUCT_START_TO_OFS()     ofs << "union"      << std::endl \
                                            << "{"          << std::endl \
                                            << "    struct" << std::endl \
                                            << "    {"      << std::endl;

#define UNION_STRUCT_FIELD_TO_OFS(name) ofs << "        "#name << ": " << ptr->name << std::endl;
#define UNION_END_TO_OFS(name)          ofs << "    }"      << std::endl \
                                            << "    "#name << ": " << ptr->name << std::endl \
                                            << "}" << std::endl;
#define OFFSET_FIELD_TO_OFS(class_name, f_name, shift) << shift "                 "#f_name": " << ptr->class_name.f_name << std::endl
#define PLANE_OFFSET_TO_OFS(name) ofs << "MOS_PLANE_OFFSET "#name << std::endl \
                                                            OFFSET_FIELD_TO_OFS(name, iSurfaceOffset,)    \
                                                            OFFSET_FIELD_TO_OFS(name, iXOffset,)          \
                                                            OFFSET_FIELD_TO_OFS(name, iYOffset,)          \
                                                            OFFSET_FIELD_TO_OFS(name, iLockSurfaceOffset,);
#define RESOURCE_OFFSET_TO_OFS(name, shift) ofs << shift "MOS_RESOURCE_OFFSETS "#name << std::endl \
                                                                                OFFSET_FIELD_TO_OFS(name, BaseOffset, shift) \
                                                                                OFFSET_FIELD_TO_OFS(name, XOffset, shift)    \
                                                                                OFFSET_FIELD_TO_OFS(name, YOffset, shift);

#define FIELD_TO_OFS_8SHIFT(name) FIELD_TO_OFS(name, "        ")

MOS_STATUS CodechalDebugInterface::DumpSurfaceInfo(
    PMOS_SURFACE surface,
    const char* surfaceName)
{
    MEDIA_DEBUG_FUNCTION_ENTER;

    MEDIA_DEBUG_CHK_NULL(surface);
    MEDIA_DEBUG_CHK_NULL(surfaceName);

    if (!m_configMgr->AttrIsEnabled(MediaDbgAttr::attrSurfaceInfo))
    {
        return MOS_STATUS_SUCCESS;
    }

    const char* funcName = (m_mediafunction == MEDIA_FUNCTION_VP) ? "_VP" : ((m_mediafunction == MEDIA_FUNCTION_ENCODE) ? "_ENC" : "_DEC");
    const char* filePath = CreateFileName(funcName, surfaceName, MediaDbgExtType::txt);
    std::ofstream ofs(filePath);
    PMOS_SURFACE ptr = surface;

    if (ofs.fail())
    {
        return MOS_STATUS_UNKNOWN;
    }

    ofs << "Surface name: " << surfaceName << std::endl;

    EMPTY_TO_OFS();
    ofs << "MOS_SURFACE:" << std::endl;
    FIELD_TO_OFS(dwArraySlice, );
    FIELD_TO_OFS(dwMipSlice, );
    FIELD_TO_OFS(S3dChannel, );

    EMPTY_TO_OFS();
    FIELD_TO_OFS(Type, );
    FIELD_TO_OFS(bOverlay, );
    FIELD_TO_OFS(bFlipChain, );

#if !defined(LINUX)
    EMPTY_TO_OFS();
    UNION_STRUCT_START_TO_OFS();
    UNION_STRUCT_FIELD_TO_OFS(dwFirstArraySlice);
    UNION_STRUCT_FIELD_TO_OFS(dwFirstMipSlice);
    UNION_END_TO_OFS(dwSubResourceIndex);
#endif

    EMPTY_TO_OFS();
    FIELD_TO_OFS(dwWidth, );
    FIELD_TO_OFS(dwHeight, );
    FIELD_TO_OFS(dwSize, );
    FIELD_TO_OFS(dwDepth, );
    FIELD_TO_OFS(dwArraySize, );
    FIELD_TO_OFS(dwLockPitch, );
    FIELD_TO_OFS(dwPitch, );
    FIELD_TO_OFS(dwSlicePitch, );
    FIELD_TO_OFS(dwQPitch, );
    FIELD_TO_OFS(TileType, );
    FIELD_TO_OFS(TileModeGMM, );
    FIELD_TO_OFS(bGMMTileEnabled, );
    FIELD_TO_OFS(Format, );
    FIELD_TO_OFS(bArraySpacing, );
    FIELD_TO_OFS(bCompressible, );

    EMPTY_TO_OFS();
    FIELD_TO_OFS(dwOffset, );
    PLANE_OFFSET_TO_OFS(YPlaneOffset);
    PLANE_OFFSET_TO_OFS(UPlaneOffset);
    PLANE_OFFSET_TO_OFS(VPlaneOffset);

    EMPTY_TO_OFS();
    UNION_STRUCT_START_TO_OFS();
    RESOURCE_OFFSET_TO_OFS(RenderOffset.YUV.Y, "    ");
    RESOURCE_OFFSET_TO_OFS(RenderOffset.YUV.U, "    ");
    RESOURCE_OFFSET_TO_OFS(RenderOffset.YUV.V, "    ");
    ofs << "    } YUV;" << std::endl;
    RESOURCE_OFFSET_TO_OFS(RenderOffset.RGB, );
    ofs << "}" << std::endl;

    EMPTY_TO_OFS();
    UNION_STRUCT_START_TO_OFS();
    UNION_STRUCT_FIELD_TO_OFS(LockOffset.YUV.Y);
    UNION_STRUCT_FIELD_TO_OFS(LockOffset.YUV.U);
    UNION_STRUCT_FIELD_TO_OFS(LockOffset.YUV.V);
    UNION_END_TO_OFS(LockOffset.RGB);

    EMPTY_TO_OFS();
    FIELD_TO_OFS(bIsCompressed, );
    FIELD_TO_OFS(CompressionMode, );
    FIELD_TO_OFS(CompressionFormat, );
    FIELD_TO_OFS(YoffsetForUplane, );
    FIELD_TO_OFS(YoffsetForVplane, );

    EMPTY_TO_OFS();
    EMPTY_TO_OFS();
    MOS_STATUS sts = DumpMosSpecificResourceInfoToOfs(&surface->OsResource, ofs);
    ofs.close();

    return sts;
}

#endif // USE_CODECHAL_DEBUG_TOOL
