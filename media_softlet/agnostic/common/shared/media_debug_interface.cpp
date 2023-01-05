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
//! \file     media_debug_interface.cpp
//! \brief    Defines the debug interface shared by all of Media.
//! \details  The debug interface dumps output from Media based on input config file.
//!
#include "media_debug_interface.h"
#if USE_MEDIA_DEBUG_TOOL
#include "media_debug_config_manager.h"
#include "media_debug_fast_dump.h"
#include "codec_hw_next.h"
#include <fstream>
#include <sstream>
#include <iomanip>

#if !defined(LINUX) && !defined(ANDROID)
#include "UmdStateSeparation.h"
#endif

MediaDebugInterface::MediaDebugInterface()
{
    memset(&m_currPic, 0, sizeof(CODEC_PICTURE));
    memset(m_fileName, 0, sizeof(m_fileName));
    memset(m_path, 0, sizeof(m_path));
    InitCRCTable(m_crcTable);

    m_dumpYUVSurface = [this](
                           PMOS_SURFACE           surface,
                           const char            *attrName,
                           const char            *surfName,
                           MEDIA_DEBUG_STATE_TYPE mediaState,
                           uint32_t               width_in,
                           uint32_t               height_in) {
        if (!DumpIsEnabled(attrName, mediaState))
        {
            return MOS_STATUS_SUCCESS;
        }

        const char *funcName = (m_mediafunction == MEDIA_FUNCTION_VP) ? "_VP" : ((m_mediafunction == MEDIA_FUNCTION_ENCODE) ? "_ENC" : "_DEC");
        std::string bufName  = std::string(surfName) + "_w[" + std::to_string(surface->dwWidth) + "]_h[" + std::to_string(surface->dwHeight) + "]_p[" + std::to_string(surface->dwPitch) + "]";
        const char *filePath = CreateFileName(funcName, bufName.c_str(), MediaDbgExtType::yuv);

        if (DumpIsEnabled(MediaDbgAttr::attrEnableFastDump))
        {
            MediaDebugFastDump::Dump(surface->OsResource, filePath);
            return MOS_STATUS_SUCCESS;
        }

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

            if (DumpIsEnabled(MediaDbgAttr::attrDisableSwizzleForDumps))
            {
                if (CodecHal_PictureIsField(m_currPic))
                {
                    return MOS_STATUS_INVALID_PARAMETER;
                }
                else
                {
                    return DumpNotSwizzled(surfName, m_temp2DSurfForCopy, lockedAddr, sizeToBeCopied);
                }
            }
        }

        uint32_t sizeMain = (uint32_t)(surface->OsResource.pGmmResInfo->GetSizeMainSurface());
        if (DumpIsEnabled(MediaDbgAttr::attrDisableSwizzleForDumps))
        {
            if (CodecHal_PictureIsField(m_currPic))
            {
                return MOS_STATUS_INVALID_PARAMETER;
            }
            else
            {
                return DumpNotSwizzled(surfName, *surface, lockedAddr, sizeMain);
            }
        }

        uint8_t *surfBaseAddr = (uint8_t *)MOS_AllocMemory(sizeMain);
        MEDIA_DEBUG_CHK_NULL(surfBaseAddr);

        if (DumpIsEnabled(MediaDbgAttr::attrForceYUVDumpWithMemcpy))
        {
            MOS_SecureMemcpy(surfBaseAddr, sizeMain, lockedAddr, sizeMain);  // Firstly, copy to surfBaseAddr to faster unlock resource
            m_osInterface->pfnUnlockResource(m_osInterface, &surface->OsResource);
            lockedAddr   = surfBaseAddr;
            surfBaseAddr = (uint8_t *)MOS_AllocMemory(sizeMain);
            MEDIA_DEBUG_CHK_NULL(surfBaseAddr);
        }

        // Always use MOS swizzle instead of GMM Cpu blit
        MEDIA_DEBUG_CHK_NULL(surfBaseAddr);
        Mos_SwizzleData(lockedAddr, surfBaseAddr, surface->TileType, MOS_TILE_LINEAR, sizeMain / surface->dwPitch, surface->dwPitch, 0);

        uint8_t *data = surfBaseAddr;
        data += surface->dwOffset + surface->YPlaneOffset.iYOffset * surface->dwPitch;

        uint32_t width  = width_in ? width_in : surface->dwWidth;
        uint32_t height = height_in ? height_in : surface->dwHeight;

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

        std::ofstream ofs(filePath, std::ios_base::out | std::ios_base::binary);
        if (ofs.fail())
        {
            return MOS_STATUS_UNKNOWN;
        }

        // write luma data to file
        for (uint32_t h = 0; h < height; h++)
        {
            ofs.write((char *)data, width);
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
            data    = surfBaseAddr + surface->UPlaneOffset.iLockSurfaceOffset;
            if (surface->Format == Format_422V || surface->Format == Format_IMC3)
            {
                vPlaneData = surfBaseAddr + surface->VPlaneOffset.iLockSurfaceOffset;
            }

#endif

            // write chroma data to file
            for (uint32_t h = 0; h < height; h++)
            {
                ofs.write((char *)data, width);
                data += pitch;
            }

            // write v planar data to file
            if (surface->Format == Format_422V || surface->Format == Format_IMC3)
            {
                for (uint32_t h = 0; h < height; h++)
                {
                    ofs.write((char *)vPlaneData, width);
                    vPlaneData += pitch;
                }
            }
        }
        ofs.close();

        if (DumpIsEnabled(MediaDbgAttr::attrForceYUVDumpWithMemcpy))
        {
            MOS_FreeMemory(lockedAddr);
        }
        else
        {
            m_osInterface->pfnUnlockResource(m_osInterface, &surface->OsResource);
        }
        MOS_FreeMemory(surfBaseAddr);

        return MOS_STATUS_SUCCESS;
    };

    m_dumpBuffer = [this](
                       PMOS_RESOURCE          resource,
                       const char            *attrName,
                       const char            *bufferName,
                       uint32_t               size,
                       uint32_t               offset,
                       MEDIA_DEBUG_STATE_TYPE mediaState) {
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
                attrEnabled = m_configMgr->AttrIsEnabled(mediaState, attrName);
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
            std::string kernelName = m_configMgr->GetMediaStateStr(mediaState);
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

MediaDebugInterface::~MediaDebugInterface()
{
    if (nullptr != m_configMgr)
    {
        MOS_Delete(m_configMgr);
    }

    if (!Mos_ResourceIsNull(&m_temp2DSurfForCopy.OsResource))
    {
        m_osInterface->pfnFreeResource(m_osInterface, &m_temp2DSurfForCopy.OsResource);
    }
}

MOS_STATUS MediaDebugInterface::InitDumpLocation()
{
    m_crcGoldenRefFileName = m_outputFilePath + std::string("GoldenReference.txt");
    if (m_configMgr->AttrIsEnabled(MediaDbgAttr::attrDumpToThreadFolder))
    {
        std::string ThreadSubFolder = "T" + std::to_string(MosUtilities::MosGetCurrentThreadId()) + MOS_DIRECTORY_DELIMITER;
        m_outputFilePath            = m_outputFilePath + ThreadSubFolder;
        MosUtilities::MosCreateDirectory(const_cast<char *>(m_outputFilePath.c_str()));
    }

    m_ddiFileName = m_outputFilePath + "ddi.par";
    std::ofstream ofs(m_ddiFileName, std::ios::out);
    ofs << "ParamFilePath"
        << " = \"" << m_fileName << "\"" << std::endl;
    ofs.close();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaDebugInterface::SetFastDumpConfig(MediaCopyBaseState *mediaCopy)
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
                {MediaDbgAttr::attrEncodeRawInputSurface, TR_KEY_ENCODE_DATA_INPUT_SURFACE},
                {MediaDbgAttr::attrReferenceSurfaces, TR_KEY_ENCODE_DATA_REF_SURFACE},
                {MediaDbgAttr::attrReconstructedSurface, TR_KEY_ENCODE_DATA_RECON_SURFACE},
                {MediaDbgAttr::attrBitstream, TR_KEY_ENCODE_DATA_BITSTREAM},
            };
        };

        auto dumpEnabled = std::make_shared<DumpEnabled>();

        m_dumpYUVSurface = [this, dumpEnabled, traceSetting, suffix](
                               PMOS_SURFACE           surface,
                               const char            *attrName,
                               const char            *surfName,
                               MEDIA_DEBUG_STATE_TYPE mediaState,
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
                           PMOS_RESOURCE          resource,
                           const char            *attrName,
                           const char            *bufferName,
                           uint32_t               size,
                           uint32_t               offset,
                           MEDIA_DEBUG_STATE_TYPE mediaState) {
            if ((*dumpEnabled)(attrName))
            {
                MediaDebugFastDump::Dump(
                    *resource,
                    std::string(traceSetting->fastDump.filePath) +
                        std::to_string(m_bufferDumpFrameNum) +
                        '-' +
                        bufferName +
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

MOS_STATUS MediaDebugInterface::SetOutputFilePath()
{
    std::string          outputPathKey;
    std::string          dumpFilePath;

    MEDIA_DEBUG_FUNCTION_ENTER;

#ifdef LINUX
    char *customizedOutputPath = getenv("MOS_DEBUG_OUTPUT_LOCATION");
    if (customizedOutputPath != nullptr && strlen(customizedOutputPath) != 0)
    {
        m_outputFilePath = customizedOutputPath;
        m_outputFilePath.erase(m_outputFilePath.find_last_not_of(" \n\r\t") + 1);
        if (m_outputFilePath[m_outputFilePath.length() - 1] != MOS_DIRECTORY_DELIMITER)
            m_outputFilePath += MOS_DIRECTORY_DELIMITER;
    }
    else
#endif
    {
        outputPathKey = SetOutputPathKey();
        ReadUserSettingForDebug(
            m_userSettingPtr,
            m_outputFilePath,
            outputPathKey,
            MediaUserSetting::Group::Device);

        if (!m_outputFilePath.empty())
        {
            if (m_outputFilePath.back() != MOS_DIRECTORY_DELIMITER)
            {
                m_outputFilePath += MOS_DIRECTORY_DELIMITER;
            }
        }
        else
        {
#if defined(LINUX) || defined(ANDROID)
            m_outputFilePath = MOS_DEBUG_DEFAULT_OUTPUT_LOCATION;
#else
            // Use state separation APIs to obtain appropriate storage location
            if (SUCCEEDED(GetDriverPersistentStorageLocation(dumpFilePath)))
            {
                m_outputFilePath = dumpFilePath.c_str();
                outputPathKey    = InitDefaultOutput();
                ReportUserSettingForDebug(
                    m_userSettingPtr,
                    outputPathKey,
                    m_outputFilePath,
                    MediaUserSetting::Group::Device);
            }
            else
            {
                return MOS_STATUS_UNKNOWN;
            }
#endif
        }
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaDebugInterface::DumpStringStream(std::stringstream& ss, const char* bufferName, const char* attrName)
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

bool MediaDebugInterface::DumpIsEnabled(
    const char *              attr,
    MEDIA_DEBUG_STATE_TYPE mediaState)
{
    if (nullptr == m_configMgr)
    {
        return false;
    }

    if (mediaState != CODECHAL_NUM_MEDIA_STATES)
    {
        return m_configMgr->AttrIsEnabled(mediaState, attr);
    }
    else
    {
        return m_configMgr->AttrIsEnabled(attr);
    }
}

const char *MediaDebugInterface::CreateFileName(
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

MOS_STATUS MediaDebugInterface::DumpCmdBuffer(
    PMOS_COMMAND_BUFFER    cmdBuffer,
    MEDIA_DEBUG_STATE_TYPE mediaState,
    const char *           cmdName)
{
    MEDIA_DEBUG_FUNCTION_ENTER;

    bool attrEnabled = m_configMgr->AttrIsEnabled(MediaDbgAttr::attrCmdBufferMfx);

    if (!attrEnabled && mediaState != CODECHAL_NUM_MEDIA_STATES)
    {
        attrEnabled = m_configMgr->AttrIsEnabled(mediaState, MediaDbgAttr::attrCmdBuffer);
    }

    if (!attrEnabled)
    {
        return MOS_STATUS_SUCCESS;
    }

    bool binaryDumpEnabled = m_configMgr->AttrIsEnabled(MediaDbgAttr::attrDumpCmdBufInBinary);

    std::string funcName = cmdName ? cmdName : m_configMgr->GetMediaStateStr(mediaState);
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

void MediaDebugInterface::PackGoldenReferences(std::initializer_list<std::vector<uint32_t>> goldenReference)
{
    for (auto beg = goldenReference.begin(); beg != goldenReference.end(); beg++)
    {
        m_goldenReferences.push_back(*beg);
    }
}

MOS_STATUS MediaDebugInterface::CaptureGoldenReference(uint8_t *buf, uint32_t size, uint32_t hwCrcValue)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    if (m_enableHwDebugHooks &&
        !m_goldenReferenceExist)
    {
        if (m_swCRC)
        {
            if (buf == nullptr)
            {
                return MOS_STATUS_NULL_POINTER;
            }
            auto crcVal = CalculateCRC(buf, size, 0);
            m_crcGoldenReference.push_back(crcVal);
        }
        else
        {
            m_crcGoldenReference.push_back(hwCrcValue);
        }
    }
    return eStatus;
}

MOS_STATUS MediaDebugInterface::FillSemaResource(std::vector<uint32_t*> &vSemaData, std::vector<uint32_t> &data)
{
    if (vSemaData.size() != data.size())
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }
    for (uint32_t i = 0; i < vSemaData.size(); i++)
    {
        MEDIA_DEBUG_CHK_NULL(vSemaData[i]);
        *vSemaData[i] = data[i];
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaDebugInterface::LockResource(uint32_t *semaData, PMOS_RESOURCE reSemaphore)
{
    CodechalResLock semaLock(m_osInterface, reSemaphore);
    semaData = (uint32_t *)semaLock.Lock(CodechalResLock::writeOnly);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaDebugInterface::LockSemaResource(std::vector<uint32_t *> &vSemaData, std::vector<MOS_RESOURCE> &vResource)
{
    for (uint32_t i = 0; i < vResource.size(); i++)
    {
        CodechalResLock semaLock(m_osInterface, &vResource[i]);
        uint32_t *      smData = (uint32_t *)semaLock.Lock(CodechalResLock::writeOnly);
        LockResource(smData, &vResource[i]);
        vSemaData.push_back(smData);
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaDebugInterface::DumpToFile(const GoldenReferences &goldenReferences)
{
    std::ofstream ofs(m_crcGoldenRefFileName, std::ios_base::out);
    if (goldenReferences.size() <= 0)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }
    for (uint32_t i = 0; i < goldenReferences[0].size(); i++)
    {
        for (auto golden : goldenReferences)
        {
            ofs << golden[i] << '\t';
        }
        ofs << '\n';
    }
    ofs.close();
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaDebugInterface::DumpGoldenReference()
{
    if (m_enableHwDebugHooks && !m_goldenReferenceExist && m_goldenReferences.size() > 0)
    {
        return DumpToFile(m_goldenReferences);
    }
    else
    {
        return MOS_STATUS_UNKNOWN;
    }
}

MOS_STATUS MediaDebugInterface::DumpYUVSurfaceToBuffer(PMOS_SURFACE surface,
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

MOS_STATUS MediaDebugInterface::LoadGoldenReference()
{
    std::ifstream ifs(m_crcGoldenRefFileName, std::ios_base::in);
    std::vector<uint32_t> lines;
    std::string           str;
    uint32_t              num;
    if (!ifs)
    {
        return MOS_STATUS_FILE_OPEN_FAILED;
    }
    uint32_t crc;
    while (!ifs.eof())
    {
        std::getline(ifs, str);
        std::stringstream stringin(str); 
        lines.clear();
        while (stringin >> num)
        {
            lines.push_back(num);
        }
        m_goldenReferences.push_back(lines);
    }
    ifs.close();
    m_goldenReferences.pop_back();
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaDebugInterface::Dump2ndLvlBatch(
    PMHW_BATCH_BUFFER      batchBuffer,
    MEDIA_DEBUG_STATE_TYPE mediaState,
    const char *           batchName)
{
    MEDIA_DEBUG_FUNCTION_ENTER;

    bool attrEnabled = m_configMgr->AttrIsEnabled(MediaDbgAttr::attr2ndLvlBatchMfx);

    if (!attrEnabled && mediaState != CODECHAL_NUM_MEDIA_STATES)
    {
        attrEnabled = m_configMgr->AttrIsEnabled(mediaState, MediaDbgAttr::attr2ndLvlBatch);
    }

    if (!attrEnabled)
    {
        return MOS_STATUS_SUCCESS;
    }

    bool        batchLockedForDebug = !batchBuffer->bLocked;
    std::string funcName            = batchName ? batchName : m_configMgr->GetMediaStateStr(mediaState);

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

MOS_STATUS MediaDebugInterface::DumpCurbe(
    MEDIA_DEBUG_STATE_TYPE mediaState,
    PMHW_KERNEL_STATE      kernelState)
{
    MEDIA_DEBUG_FUNCTION_ENTER;

    if (mediaState >= CODECHAL_NUM_MEDIA_STATES ||
        !m_configMgr->AttrIsEnabled(mediaState, MediaDbgAttr::attrCurbe))
    {
        return MOS_STATUS_SUCCESS;
    }

    std::string funcName   = m_configMgr->GetMediaStateStr(mediaState);
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

MOS_STATUS MediaDebugInterface::DumpMDFCurbe(
    MEDIA_DEBUG_STATE_TYPE mediaState,
    uint8_t *              curbeBuffer,
    uint32_t               curbeSize)
{
    MEDIA_DEBUG_FUNCTION_ENTER;

    uint8_t *  curbeAlignedData = nullptr;
    uint32_t   curbeAlignedSize = 0;
    MOS_STATUS eStatus          = MOS_STATUS_SUCCESS;

    if (mediaState >= CODECHAL_NUM_MEDIA_STATES ||
        !m_configMgr->AttrIsEnabled(mediaState, MediaDbgAttr::attrCurbe))
    {
        return eStatus;
    }

    std::string funcName   = m_configMgr->GetMediaStateStr(mediaState);
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

MOS_STATUS MediaDebugInterface::DumpKernelRegion(
    MEDIA_DEBUG_STATE_TYPE mediaState,
    MHW_STATE_HEAP_TYPE    stateHeap,
    PMHW_KERNEL_STATE      kernelState)
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
        attrEnabled = m_configMgr->AttrIsEnabled(mediaState, MediaDbgAttr::attrIsh);
        bufferType  = MediaDbgBufferType::bufISH;
    }
    else if (stateHeap == MHW_DSH_TYPE)
    {
        regionBlock = &kernelState->m_dshRegion;
        attrEnabled = m_configMgr->AttrIsEnabled(mediaState, MediaDbgAttr::attrDsh);
        bufferType  = MediaDbgBufferType::bufDSH;
    }
    else
    {
        attrEnabled = m_configMgr->AttrIsEnabled(mediaState, MediaDbgAttr::attrSsh);
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

    std::string funcName = m_configMgr->GetMediaStateStr(mediaState);

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

MOS_STATUS MediaDebugInterface::DumpRgbDataOnYUVSurface(
    PMOS_SURFACE           surface,
    const char *           attrName,
    const char *           surfName,
    MEDIA_DEBUG_STATE_TYPE mediaState,
    uint32_t               width_in,
    uint32_t               height_in)
{
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaDebugInterface::DumpYUVSurface(
    PMOS_SURFACE           surface,
    const char *           attrName,
    const char *           surfName,
    MEDIA_DEBUG_STATE_TYPE mediaState,
    uint32_t               width_in,
    uint32_t               height_in)
{
    return m_dumpYUVSurface(
        surface,
        attrName,
        surfName,
        mediaState,
        width_in,
        height_in);
}

MOS_STATUS MediaDebugInterface::DumpUncompressedYUVSurface(PMOS_SURFACE surface)
{
    if (surface == nullptr)
    {
        MEDIA_DEBUG_ASSERTMESSAGE("Surface for Dump is nullptr");
        return MOS_STATUS_NULL_POINTER;
    }

    MEDIA_DEBUG_CHK_STATUS(ReAllocateSurface(
        &m_temp2DSurfForCopy,
        surface,
        "Temp2DSurfForSurfDumper",
        MOS_GFXRES_2D,
        true));

    MEDIA_DEBUG_VERBOSEMESSAGE("Temp2DSurfaceForCopy width %d, height %d, pitch %d, TileType %d, bIsCompressed %d, CompressionMode %d",
        m_temp2DSurfForCopy.dwWidth,
        m_temp2DSurfForCopy.dwHeight,
        m_temp2DSurfForCopy.dwPitch,
        m_temp2DSurfForCopy.TileType,
        m_temp2DSurfForCopy.bIsCompressed,
        m_temp2DSurfForCopy.CompressionMode);

    // Copy Original Surface to a Linear/Uncompressed surface, later lock can use m_temp2DSurfForCopy for uncompresed surfaces
    // if the return is not supported, then the lock should only happen on origianl resource
    MEDIA_DEBUG_CHK_STATUS(m_osInterface->pfnDoubleBufferCopyResource(
        m_osInterface,
        &surface->OsResource,
        &m_temp2DSurfForCopy.OsResource,
        false));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaDebugInterface::DumpBuffer(
    PMOS_RESOURCE          resource,
    const char *           attrName,
    const char *           bufferName,
    uint32_t               size,
    uint32_t               offset,
    MEDIA_DEBUG_STATE_TYPE mediaState)
{
    return m_dumpBuffer(
        resource,
        attrName,
        bufferName,
        size,
        offset,
        mediaState);
}

MOS_STATUS MediaDebugInterface::DumpSurface(
    PMOS_SURFACE              surface,
    const char *              attrName,
    const char *              surfaceName,
    MEDIA_DEBUG_STATE_TYPE    mediaState)
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
        attrEnabled = m_configMgr->AttrIsEnabled(mediaState, attrName);
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
        std::string kernelName = m_configMgr->GetMediaStateStr(mediaState);
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

MOS_STATUS MediaDebugInterface::DumpData(
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
MOS_STATUS MediaDebugInterface::DumpSurfaceInfo(
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

#define FIELD_TO_OFS_8SHIFT(name) FIELD_TO_OFS(name, "        ")
MOS_STATUS MediaDebugInterface::DumpMosSpecificResourceInfoToOfs(
    PMOS_RESOURCE pOsResource,
    std::ofstream& ofs)
{
    MEDIA_DEBUG_FUNCTION_ENTER;
    if (Mos_ResourceIsNull(pOsResource))
    {
        MEDIA_DEBUG_ASSERTMESSAGE("pOsResource is null");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    PMOS_RESOURCE ptr = pOsResource;
    ofs << "MOS_RESOURCE:" << std::endl;
#if !defined(LINUX) || defined(WDDM_LINUX)
    FIELD_TO_OFS(RunTimeHandle, );
#else
    FIELD_TO_OFS(iWidth, );
    FIELD_TO_OFS(iHeight, );
    FIELD_TO_OFS(iSize, );
    FIELD_TO_OFS(iPitch, );
    FIELD_TO_OFS(iDepth, );
    FIELD_TO_OFS(Format, );
    FIELD_TO_OFS(iCount, );
    FIELD_TO_OFS(dwGfxAddress, );
    FIELD_TO_OFS(isTiled, );
    FIELD_TO_OFS(TileType, );
    FIELD_TO_OFS(bMapped, );

    EMPTY_TO_OFS();
    FIELD_TO_OFS(bo->size, );
    FIELD_TO_OFS(bo->align, );
    FIELD_TO_OFS(bo->offset, );
    FIELD_TO_OFS(bo->handle, );
    FIELD_TO_OFS(bo->offset64, );
    FIELD_TO_OFS(bo->aux_mapped, );
#endif
    ofs << "iAllocationIndex[MOS_GPU_CONTEXT_MAX == " << (uint32_t)MOS_GPU_CONTEXT_MAX << "]: {";
    for (int i = 0; i < MOS_GPU_CONTEXT_MAX; ++i)
        ofs << ptr->iAllocationIndex[i] << ", ";
    ofs << "}" << std::endl;

    {
        PGMM_RESOURCE_INFO ptr = pOsResource->pGmmResInfo;
        EMPTY_TO_OFS();
        ofs << "GMM_RESOURCE_INFO:" << std::endl;
        FIELD_TO_OFS(GetResourceType(), );
        FIELD_TO_OFS(GetResourceFormat(), );
        FIELD_TO_OFS(GetBitsPerPixel(), );

        {
            GMM_RESOURCE_FLAG flags = pOsResource->pGmmResInfo->GetResFlags();
            GMM_RESOURCE_FLAG* ptr = &flags;

            EMPTY_TO_OFS();
            ofs << "    GMM_RESOURCE_FLAG:" << std::endl;
            FIELD_TO_OFS_8SHIFT(Gpu.CameraCapture);
            FIELD_TO_OFS_8SHIFT(Gpu.CCS);
            FIELD_TO_OFS_8SHIFT(Gpu.ColorDiscard);
            FIELD_TO_OFS_8SHIFT(Gpu.ColorSeparation);
            FIELD_TO_OFS_8SHIFT(Gpu.ColorSeparationRGBX);
            FIELD_TO_OFS_8SHIFT(Gpu.Constant);
            FIELD_TO_OFS_8SHIFT(Gpu.Depth);
            FIELD_TO_OFS_8SHIFT(Gpu.FlipChain);
            FIELD_TO_OFS_8SHIFT(Gpu.FlipChainPreferred);
            FIELD_TO_OFS_8SHIFT(Gpu.HistoryBuffer);
            FIELD_TO_OFS_8SHIFT(Gpu.HiZ);
            FIELD_TO_OFS_8SHIFT(Gpu.Index);
            FIELD_TO_OFS_8SHIFT(Gpu.IndirectClearColor);
            FIELD_TO_OFS_8SHIFT(Gpu.InstructionFlat);
            FIELD_TO_OFS_8SHIFT(Gpu.InterlacedScan);
            FIELD_TO_OFS_8SHIFT(Gpu.MCS);
            FIELD_TO_OFS_8SHIFT(Gpu.MMC);
            FIELD_TO_OFS_8SHIFT(Gpu.MotionComp);
            FIELD_TO_OFS_8SHIFT(Gpu.NoRestriction);
            FIELD_TO_OFS_8SHIFT(Gpu.Overlay);
            FIELD_TO_OFS_8SHIFT(Gpu.Presentable);
            FIELD_TO_OFS_8SHIFT(Gpu.ProceduralTexture);
            FIELD_TO_OFS_8SHIFT(Gpu.Query);
            FIELD_TO_OFS_8SHIFT(Gpu.RenderTarget);
            FIELD_TO_OFS_8SHIFT(Gpu.S3d);
            FIELD_TO_OFS_8SHIFT(Gpu.S3dDx);
            FIELD_TO_OFS_8SHIFT(Gpu.__S3dNonPacked);
            FIELD_TO_OFS_8SHIFT(Gpu.ScratchFlat);
            FIELD_TO_OFS_8SHIFT(Gpu.SeparateStencil);
            FIELD_TO_OFS_8SHIFT(Gpu.State);
            FIELD_TO_OFS_8SHIFT(Gpu.Stream);
            FIELD_TO_OFS_8SHIFT(Gpu.TextApi);
            FIELD_TO_OFS_8SHIFT(Gpu.Texture);
            FIELD_TO_OFS_8SHIFT(Gpu.TiledResource);
            FIELD_TO_OFS_8SHIFT(Gpu.TilePool);
            FIELD_TO_OFS_8SHIFT(Gpu.UnifiedAuxSurface);
            FIELD_TO_OFS_8SHIFT(Gpu.Vertex);
            FIELD_TO_OFS_8SHIFT(Gpu.Video);
            FIELD_TO_OFS_8SHIFT(Gpu.__NonMsaaTileXCcs);
            FIELD_TO_OFS_8SHIFT(Gpu.__NonMsaaTileYCcs);
            FIELD_TO_OFS_8SHIFT(Gpu.__MsaaTileMcs);
            FIELD_TO_OFS_8SHIFT(Gpu.__NonMsaaLinearCCS);
            FIELD_TO_OFS_8SHIFT(Gpu.__Remaining);

            EMPTY_TO_OFS();
            FIELD_TO_OFS_8SHIFT(Info.AllowVirtualPadding);
            FIELD_TO_OFS_8SHIFT(Info.BigPage);
            FIELD_TO_OFS_8SHIFT(Info.Cacheable);
            FIELD_TO_OFS_8SHIFT(Info.ContigPhysMemoryForiDART);
            FIELD_TO_OFS_8SHIFT(Info.CornerTexelMode);
            FIELD_TO_OFS_8SHIFT(Info.ExistingSysMem);
            FIELD_TO_OFS_8SHIFT(Info.ForceResidency);
            FIELD_TO_OFS_8SHIFT(Info.Gfdt);
            FIELD_TO_OFS_8SHIFT(Info.GttMapType);
            FIELD_TO_OFS_8SHIFT(Info.HardwareProtected);
            FIELD_TO_OFS_8SHIFT(Info.KernelModeMapped);
            FIELD_TO_OFS_8SHIFT(Info.LayoutBelow);
            FIELD_TO_OFS_8SHIFT(Info.LayoutMono);
            FIELD_TO_OFS_8SHIFT(Info.LayoutRight);
            FIELD_TO_OFS_8SHIFT(Info.LocalOnly);
            FIELD_TO_OFS_8SHIFT(Info.Linear);
            FIELD_TO_OFS_8SHIFT(Info.MediaCompressed);
            FIELD_TO_OFS_8SHIFT(Info.NoOptimizationPadding);
            FIELD_TO_OFS_8SHIFT(Info.NoPhysMemory);
            FIELD_TO_OFS_8SHIFT(Info.NotLockable);
            FIELD_TO_OFS_8SHIFT(Info.NonLocalOnly);
            FIELD_TO_OFS_8SHIFT(Info.StdSwizzle);
            FIELD_TO_OFS_8SHIFT(Info.PseudoStdSwizzle);
            FIELD_TO_OFS_8SHIFT(Info.Undefined64KBSwizzle);
            FIELD_TO_OFS_8SHIFT(Info.RedecribedPlanes);
            FIELD_TO_OFS_8SHIFT(Info.RenderCompressed);
            FIELD_TO_OFS_8SHIFT(Info.Rotated);
            FIELD_TO_OFS_8SHIFT(Info.Shared);
            FIELD_TO_OFS_8SHIFT(Info.SoftwareProtected);
            FIELD_TO_OFS_8SHIFT(Info.SVM);
#if !defined(LINUX)
            FIELD_TO_OFS_8SHIFT(Info.Tile4);
            FIELD_TO_OFS_8SHIFT(Info.Tile64);
#endif
            FIELD_TO_OFS_8SHIFT(Info.TiledW);
            FIELD_TO_OFS_8SHIFT(Info.TiledX);
            FIELD_TO_OFS_8SHIFT(Info.TiledY);
            FIELD_TO_OFS_8SHIFT(Info.TiledYf);
            FIELD_TO_OFS_8SHIFT(Info.TiledYs);
            FIELD_TO_OFS_8SHIFT(Info.XAdapter);
            FIELD_TO_OFS_8SHIFT(Info.__PreallocatedResInfo);
#if !defined(LINUX)
            FIELD_TO_OFS_8SHIFT(Info.LMemBarPreferred);
            FIELD_TO_OFS_8SHIFT(Info.LMemBarOrNonlocalOnly);
            FIELD_TO_OFS_8SHIFT(Info.LMemBarIndifferent);
            FIELD_TO_OFS_8SHIFT(Info.CpuVisibleOnDemand);
            FIELD_TO_OFS_8SHIFT(Info.DwmFbrResource);
#endif

            EMPTY_TO_OFS();
            FIELD_TO_OFS_8SHIFT(Wa.GTMfx2ndLevelBatchRingSizeAlign);
            FIELD_TO_OFS_8SHIFT(Wa.ILKNeedAvcMprRowStore32KAlign);
            FIELD_TO_OFS_8SHIFT(Wa.ILKNeedAvcDmvBuffer32KAlign);
            FIELD_TO_OFS_8SHIFT(Wa.NoBufferSamplerPadding);
            FIELD_TO_OFS_8SHIFT(Wa.NoLegacyPlanarLinearVideoRestrictions);
            FIELD_TO_OFS_8SHIFT(Wa.CHVAstcSkipVirtualMips);
            FIELD_TO_OFS_8SHIFT(Wa.DisablePackedMipTail);
            FIELD_TO_OFS_8SHIFT(Wa.__ForceOtherHVALIGN4);
            FIELD_TO_OFS_8SHIFT(Wa.DisableDisplayCcsClearColor);
            FIELD_TO_OFS_8SHIFT(Wa.DisableDisplayCcsCompression);
            FIELD_TO_OFS_8SHIFT(Wa.PreGen12FastClearOnly);
#if !defined(LINUX)
            FIELD_TO_OFS_8SHIFT(Wa.ForceStdAllocAlign);
#endif
        }

        FIELD_TO_OFS(GetBaseWidth(), );
        FIELD_TO_OFS(GetBaseHeight(), );
        FIELD_TO_OFS(GetBaseDepth(), );
        FIELD_TO_OFS(GetMaxLod(), );
        FIELD_TO_OFS(GetArraySize(), );
        FIELD_TO_OFS(GetSetCpSurfTag(0, 0), );
        FIELD_TO_OFS(GetCachePolicyUsage(), );
        FIELD_TO_OFS(GetNumSamples(), );
        FIELD_TO_OFS(GetSamplePattern(), );

        EMPTY_TO_OFS();
        FIELD_TO_OFS(IsArraySpacingSingleLod(), );
        FIELD_TO_OFS(GetBaseAlignment(), );
        FIELD_TO_OFS(GetHAlign(), );
        FIELD_TO_OFS(GetVAlign(), );
        FIELD_TO_OFS(GetMipTailStartLodSurfaceState(), );
        FIELD_TO_OFS(GetQPitch(), );

        EMPTY_TO_OFS();
        ofs << "MmcMode[GMM_MAX_MMC_INDEX == " << GMM_MAX_MMC_INDEX << "]: {";
        for (uint32_t i = 0; i < GMM_MAX_MMC_INDEX; ++i)
            ofs << (uint32_t)ptr->GetMmcMode(i) << ", ";
        ofs << "}" << std::endl;

        ofs << "MmcHint[GMM_MAX_MMC_INDEX == " << GMM_MAX_MMC_INDEX << "]: {";
        for (uint32_t i = 0; i < GMM_MAX_MMC_INDEX; ++i)
            ofs << (uint32_t)ptr->GetMmcHint(i) << ", ";
        ofs << "}" << std::endl;

        FIELD_TO_OFS(GetRenderPitch(), );
        FIELD_TO_OFS(GetSizeMainSurface(), );
        FIELD_TO_OFS(GmmGetTileMode(), );

#if !defined(LINUX)
        EMPTY_TO_OFS();
        FIELD_TO_OFS(GetMultiTileArch().Enable, );
#endif
    }

    return MOS_STATUS_SUCCESS;
}
#undef FIELD_TO_OFS_8SHIFT
#undef RESOURCE_OFFSET_TO_OFS
#undef PLANE_OFFSET_TO_OFS
#undef OFFSET_FIELD_TO_OFS
#undef UNION_END_TO_OFS
#undef UNION_STRUCT_FIELD_TO_OFS
#undef UNION_STRUCT_START_TO_OFS
#undef EMPTY_TO_OFS
#undef FIELD_TO_OFS

MOS_STATUS MediaDebugInterface::DumpBltOutput(
    PMOS_SURFACE surface,
    const char * attrName)
{
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaDebugInterface::DeleteCfgLinkNode(uint32_t frameIdx)
{
    return m_configMgr->DeleteCfgNode(frameIdx);
}

MOS_STATUS MediaDebugInterface::ReAllocateSurface(
    PMOS_SURFACE    pSurface,
    PMOS_SURFACE    pSrcSurf,
    PCCHAR          pSurfaceName,
    MOS_GFXRES_TYPE defaultResType,
    bool            useLinearResource)
{
    MOS_ALLOC_GFXRES_PARAMS allocParams;

    MEDIA_DEBUG_ASSERT(m_osInterface);
    MEDIA_DEBUG_ASSERT(&pSurface->OsResource);

    // bCompressible should be compared with bCompressible since it is inited by bCompressible in previous call
    // TileType of surface should be compared since we need to reallocate surface if TileType changes
    if (!Mos_ResourceIsNull(&pSurface->OsResource) &&
        (pSurface->dwWidth == pSrcSurf->dwWidth) &&
        (pSurface->dwHeight == pSrcSurf->dwHeight) &&
        (pSurface->Format == pSrcSurf->Format) &&
        ((pSurface->bCompressible == pSrcSurf->bCompressible) || useLinearResource) && // omit this check as linear surface is uncompressible
        (pSurface->CompressionMode == pSrcSurf->CompressionMode) &&
        ((pSurface->TileType == pSrcSurf->TileType) ||
         (pSurface->TileType == MOS_TILE_LINEAR && useLinearResource))) // when useLinearResource no reallocation needed
    {
        MEDIA_DEBUG_VERBOSEMESSAGE("Skip to reallocate temp surface.");
        return MOS_STATUS_SUCCESS;
    }
    MOS_ZeroMemory(&allocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));

#if !EMUL
    //  Need to reallocate surface according to expected tiletype instead of tiletype of the surface what we have
    if ((pSurface->OsResource.pGmmResInfo != nullptr) &&
        (pSurface->TileType == pSrcSurf->TileType))
    {
        // Reallocate but use same tile type and resource type as current
        allocParams.TileType = pSurface->OsResource.TileType;
        allocParams.Type     = defaultResType;
    }
    else
#endif
    {
        // First time allocation. Caller must specify default params
        allocParams.TileType = pSrcSurf->TileType;
        allocParams.Type     = defaultResType;
    }

    // Force to use tile linear for reallocated resource
    if (useLinearResource)
    {
        allocParams.TileType = MOS_TILE_LINEAR;
        allocParams.Type = MOS_GFXRES_2D;
    }

    allocParams.dwWidth         = pSrcSurf->dwWidth;
    allocParams.dwHeight        = pSrcSurf->dwHeight;
    allocParams.Format          = pSrcSurf->Format;
    allocParams.bIsCompressible = pSrcSurf->bCompressible;
    allocParams.CompressionMode = pSrcSurf->CompressionMode;
    allocParams.pBufName        = pSurfaceName;
    allocParams.dwArraySize     = 1;

    // Delete resource if already allocated
    m_osInterface->pfnFreeResource(m_osInterface, &(pSurface->OsResource));

    // Allocate surface
    CODECHAL_PUBLIC_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParams,
        &pSurface->OsResource));

    if (!m_osInterface->apoMosEnabled)
    {
        MOS_SURFACE details;
        MOS_ZeroMemory(&details, sizeof(details));
        details.Format = Format_Invalid;

        MEDIA_DEBUG_CHK_STATUS(m_osInterface->pfnGetResourceInfo(m_osInterface, &pSurface->OsResource, &details));

        pSurface->dwWidth                     = details.dwWidth;
        pSurface->dwHeight                    = details.dwHeight;
        pSurface->dwPitch                     = details.dwPitch;
        pSurface->dwDepth                     = details.dwDepth;
        pSurface->dwQPitch                    = details.dwQPitch;
        pSurface->bArraySpacing               = details.bArraySpacing;
        pSurface->bCompressible               = details.bCompressible;
        pSurface->CompressionMode             = details.CompressionMode;
        pSurface->bIsCompressed               = details.bIsCompressed;
        pSurface->Format                      = details.Format;
        pSurface->TileType                    = details.TileType;
        pSurface->dwOffset                    = details.RenderOffset.YUV.Y.BaseOffset;
        pSurface->YPlaneOffset.iSurfaceOffset = details.RenderOffset.YUV.Y.BaseOffset;
        pSurface->YPlaneOffset.iXOffset       = details.RenderOffset.YUV.Y.XOffset;
        pSurface->YPlaneOffset.iYOffset =
            (pSurface->YPlaneOffset.iSurfaceOffset - pSurface->dwOffset) / pSurface->dwPitch +
            details.RenderOffset.YUV.Y.YOffset;
        pSurface->UPlaneOffset.iSurfaceOffset = details.RenderOffset.YUV.U.BaseOffset;
        pSurface->UPlaneOffset.iXOffset       = details.RenderOffset.YUV.U.XOffset;
        pSurface->UPlaneOffset.iYOffset =
            (pSurface->UPlaneOffset.iSurfaceOffset - pSurface->dwOffset) / pSurface->dwPitch +
            details.RenderOffset.YUV.U.YOffset;
        pSurface->UPlaneOffset.iLockSurfaceOffset = details.LockOffset.YUV.U;
        pSurface->VPlaneOffset.iSurfaceOffset     = details.RenderOffset.YUV.V.BaseOffset;
        pSurface->VPlaneOffset.iXOffset           = details.RenderOffset.YUV.V.XOffset;
        pSurface->VPlaneOffset.iYOffset =
            (pSurface->VPlaneOffset.iSurfaceOffset - pSurface->dwOffset) / pSurface->dwPitch +
            details.RenderOffset.YUV.V.YOffset;
        pSurface->VPlaneOffset.iLockSurfaceOffset = details.LockOffset.YUV.V;
    }
    else
    {
        pSurface->Format = Format_Invalid;
        MEDIA_DEBUG_CHK_STATUS(m_osInterface->pfnGetResourceInfo(m_osInterface, &pSurface->OsResource, pSurface));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaDebugInterface::DumpNotSwizzled(
    std::string  surfName,
    MOS_SURFACE &surf,
    uint8_t *    lockedAddr,
    int32_t      size)
{
    const char *funcName = (m_mediafunction == MEDIA_FUNCTION_VP) ? "_VP" : ((m_mediafunction == MEDIA_FUNCTION_ENCODE) ? "_ENC" : "_DEC");
    int         YOffset  = surf.dwOffset + surf.YPlaneOffset.iYOffset * surf.dwPitch;

#ifdef LINUX
    int UOffset = surf.UPlaneOffset.iSurfaceOffset;
    int VOffset = surf.VPlaneOffset.iSurfaceOffset;
#else
    int UOffset = surf.UPlaneOffset.iLockSurfaceOffset;
    int VOffset = surf.VPlaneOffset.iLockSurfaceOffset;
#endif

    std::string bufName = std::string(surfName) + "NotSwizzled_format[" + std::to_string((int)surf.Format) + "]_w[" + std::to_string(surf.dwWidth) + "]_h[" + std::to_string(surf.dwHeight) + "]_p[" + std::to_string(surf.dwPitch) + "]_srcTiling[" + std::to_string((int)surf.TileType) + "]_sizeMain[" + std::to_string(size) + "]_YOffset[" + std::to_string(YOffset) + "]_UOffset[" + std::to_string(UOffset) + "]_VOffset[" + std::to_string(VOffset) + "]";

    const char *  filePath = CreateFileName(funcName, bufName.c_str(), MediaDbgExtType::yuv);
    std::ofstream ofs(filePath, std::ios_base::out | std::ios_base::binary);
    if (ofs.fail())
    {
        return MOS_STATUS_UNKNOWN;
    }

    uint8_t *data = lockedAddr;
    ofs.write((char *)data, size);
    ofs.close();

    m_osInterface->pfnUnlockResource(m_osInterface, &surf.OsResource);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaDebugInterface::Dump2DBufferInBinary(
    uint8_t *data,
    uint32_t width,
    uint32_t height,
    uint32_t pitch)
{
    MEDIA_DEBUG_CHK_NULL(data);

    const char *filePath = m_outputFileName.c_str();

    if (width == 0 || height == 0 || pitch == 0)
    {
        return MOS_STATUS_UNKNOWN;
    }

    std::ofstream ofs(filePath, std::ios_base::out | std::ios_base::binary);
    if (ofs.fail())
    {
        return MOS_STATUS_UNKNOWN;
    }

    for (uint32_t h = 0; h < height; h++)
    {
        ofs.write((char *)data, width);
        data += pitch;
    }

    ofs.close();
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaDebugInterface::DumpBufferInBinary(uint8_t *data, uint32_t size)
{
    MEDIA_DEBUG_CHK_NULL(data);

    const char *filePath = m_outputFileName.c_str();

    if (size == 0)
    {
        return MOS_STATUS_UNKNOWN;
    }

    std::ofstream ofs(filePath, std::ios_base::out | std::ios_base::binary);
    if (ofs.fail())
    {
        return MOS_STATUS_UNKNOWN;
    }

    ofs.write((char *)data, size);
    ofs.close();
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaDebugInterface::DumpBufferInHexDwords(uint8_t *data, uint32_t size)
{
    MEDIA_DEBUG_CHK_NULL(data);

    const char *filePath = m_outputFileName.c_str();

    if (size == 0)
    {
        return MOS_STATUS_UNKNOWN;
    }

    std::ofstream ofs(filePath);

    if (ofs.fail())
    {
        return MOS_STATUS_UNKNOWN;
    }

    uint32_t dwordSize  = size / sizeof(uint32_t);
    uint32_t remainSize = size % sizeof(uint32_t);

    uint32_t *dwordData = (uint32_t *)data;
    uint32_t  i;
    for (i = 0; i < dwordSize; i++)
    {
        ofs << std::hex << std::setw(8) << std::setfill('0') << +dwordData[i] << " ";
        if (i % 4 == 3)
        {
            ofs << std::endl;
        }
    }

    if (remainSize > 0)
    {
        uint32_t lastWord = dwordData[i] & (0xFFFFFFFF << ((8 - remainSize * 2) * 4));
        ofs << std::hex << std::setw(8) << std::setfill('0') << +lastWord << std::endl;
    }

    ofs.close();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaDebugInterface::InitCRCTable(uint32_t crcTable[256])
{
    uint32_t polynomial = 0xEDB88320;
    for (uint32_t i = 0; i < 256; i++)
    {
        uint32_t c = i;
        for (size_t j = 0; j < 8; j++)
        {
            if (c & 1)
            {
                c = polynomial ^ (c >> 1);
            }
            else
            {
                c >>= 1;
            }
        }
        crcTable[i] = c;
    }
    return MOS_STATUS_SUCCESS;
}

uint32_t MediaDebugInterface::CalculateCRC(const void *buf, size_t len, uint32_t initial)
{
    uint32_t       c = initial ^ 0xFFFFFFFF;
    const uint8_t *u = static_cast<const uint8_t *>(buf);
    for (size_t i = 0; i < len; ++i)
    {
        c = m_crcTable[(c ^ u[i]) & 0xFF] ^ (c >> 8);
    }
    return c ^ 0xFFFFFFFF;
}

#endif  // USE_MEDIA_DEBUG_TOOL
