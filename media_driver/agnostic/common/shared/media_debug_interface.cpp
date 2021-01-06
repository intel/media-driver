/*
* Copyright (c) 2020, Intel Corporation
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
#include "codechal_hw.h"
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
    if (m_configMgr->AttrIsEnabled(MediaDbgAttr::attrDumpToThreadFolder))
    {
        std::string ThreadSubFolder = "T" + std::to_string(MOS_GetCurrentThreadId()) + MOS_DIRECTORY_DELIMITER;
        m_outputFilePath            = m_outputFilePath + ThreadSubFolder;
        MOS_CreateDirectory(const_cast<char *>(m_outputFilePath.c_str()));
    }

    m_ddiFileName = m_outputFilePath + "ddi.par";
    std::ofstream ofs(m_ddiFileName, std::ios::out);
    ofs << "ParamFilePath"
        << " = \"" << m_fileName << "\"" << std::endl;
    ofs.close();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaDebugInterface::SetOutputFilePath()
{
    MOS_USER_FEATURE_VALUE_DATA       userFeatureData;
    MOS_USER_FEATURE_VALUE_WRITE_DATA userFeatureWriteData;
    MOS_USER_FEATURE_VALUE_ID         OutputPathKey = __MOS_USER_FEATURE_KEY_INVALID_ID;
    char                              stringData[MOS_MAX_PATH_LENGTH + 1];
    std::string                       DumpFilePath;

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
        OutputPathKey = SetOutputPathKey();
        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
        userFeatureData.StringData.pStringData = stringData;
        MOS_UserFeature_ReadValue_ID(
            NULL,
            OutputPathKey,
            &userFeatureData,
            m_osInterface->pOsContext);

        if (userFeatureData.StringData.uSize == MOS_MAX_PATH_LENGTH + 1)
        {
            userFeatureData.StringData.uSize = 0;
        }

        if (userFeatureData.StringData.uSize > 0)
        {
            if (userFeatureData.StringData.pStringData[userFeatureData.StringData.uSize - 2] != MOS_DIRECTORY_DELIMITER)
            {
                userFeatureData.StringData.pStringData[userFeatureData.StringData.uSize - 1] = MOS_DIRECTORY_DELIMITER;
                userFeatureData.StringData.pStringData[userFeatureData.StringData.uSize]     = '\0';
                userFeatureData.StringData.uSize++;
            }
            m_outputFilePath = userFeatureData.StringData.pStringData;
        }
        else
        {
#if defined(LINUX) || defined(ANDROID)
            m_outputFilePath = MOS_DEBUG_DEFAULT_OUTPUT_LOCATION;
#else
            // Use state separation APIs to obtain appropriate storage location
            if (SUCCEEDED(GetDriverPersistentStorageLocation(DumpFilePath)))
            {
                m_outputFilePath = DumpFilePath.c_str();
                OutputPathKey    = InitDefaultOutput();
                MOS_ZeroMemory(&userFeatureWriteData, sizeof(userFeatureWriteData));
                userFeatureWriteData.Value.StringData.pStringData = const_cast<char *>(m_outputFilePath.c_str());
                userFeatureWriteData.Value.StringData.uSize       = m_outputFilePath.size();
                userFeatureWriteData.ValueID                      = OutputPathKey;
                MOS_UserFeature_WriteValues_ID(NULL, &userFeatureWriteData, 1, m_osInterface->pOsContext);
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

MOS_STATUS MediaDebugInterface::DumpYUVSurface(
    PMOS_SURFACE           surface,
    const char *           attrName,
    const char *           surfName,
    MEDIA_DEBUG_STATE_TYPE mediaState,
    uint32_t               width_in,
    uint32_t               height_in)
{
    if (!DumpIsEnabled(attrName, mediaState))
    {
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

    const char *funcName = (m_mediafunction == MEDIA_FUNCTION_VP) ? "_VP" : ((m_mediafunction == MEDIA_FUNCTION_ENCODE) ? "_ENC" : "_DEC");
    std::string bufName  = std::string(surfName) + "_w[" + std::to_string(surface->dwWidth) + "]_h[" + std::to_string(surface->dwHeight) + "]_p[" + std::to_string(pitch) + "]";
    const char *filePath = CreateFileName(funcName, bufName.c_str(), MediaDbgExtType::yuv);

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
        data = surfBaseAddr + surface->UPlaneOffset.iLockSurfaceOffset;
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
}

MOS_STATUS MediaDebugInterface::DumpBuffer(
    PMOS_RESOURCE          resource,
    const char *           attrName,
    const char *           bufferName,
    uint32_t               size,
    uint32_t               offset,
    MEDIA_DEBUG_STATE_TYPE mediaState)
{
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

    MOS_LOCK_PARAMS lockFlags;
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.ReadOnly = 1;
    uint8_t *data      = (uint8_t *)m_osInterface->pfnLockResource(m_osInterface, resource, &lockFlags);
    MEDIA_DEBUG_CHK_NULL(data);
    data += offset;

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
    MOS_GFXRES_TYPE DefaultResType)
{
    MOS_ALLOC_GFXRES_PARAMS AllocParams;

    MEDIA_DEBUG_ASSERT(m_osInterface);
    MEDIA_DEBUG_ASSERT(&pSurface->OsResource);

    // bCompressible should be compared with bCompressible since it is inited by bCompressible in previous call
    // TileType of surface should be compared since we need to reallocate surface if TileType changes
    if (!Mos_ResourceIsNull(&pSurface->OsResource) &&
        (pSurface->dwWidth == pSrcSurf->dwWidth) &&
        (pSurface->dwHeight == pSrcSurf->dwHeight) &&
        (pSurface->Format == pSrcSurf->Format) &&
        (pSurface->bCompressible == pSrcSurf->bCompressible) &&
        (pSurface->CompressionMode == pSrcSurf->CompressionMode) &&
        (pSurface->TileType == pSrcSurf->TileType))
    {
        return MOS_STATUS_SUCCESS;
    }
    MOS_ZeroMemory(&AllocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));

#if !EMUL
    //  Need to reallocate surface according to expected tiletype instead of tiletype of the surface what we have
    if ((pSurface->OsResource.pGmmResInfo != nullptr) &&
        (pSurface->TileType == pSrcSurf->TileType))
    {
        // Reallocate but use same tile type and resource type as current
        AllocParams.TileType = pSurface->OsResource.TileType;
        AllocParams.Type     = DefaultResType;
    }
    else
#endif
    {
        // First time allocation. Caller must specify default params
        AllocParams.TileType = pSrcSurf->TileType;
        AllocParams.Type     = DefaultResType;
    }

    AllocParams.dwWidth         = pSrcSurf->dwWidth;
    AllocParams.dwHeight        = pSrcSurf->dwHeight;
    AllocParams.Format          = pSrcSurf->Format;
    AllocParams.bIsCompressible = pSrcSurf->bCompressible;
    AllocParams.CompressionMode = pSrcSurf->CompressionMode;
    AllocParams.pBufName        = pSurfaceName;
    AllocParams.dwArraySize     = 1;

    // Delete resource if already allocated
    m_osInterface->pfnFreeResource(m_osInterface, &(pSurface->OsResource));

    // Allocate surface
    CODECHAL_PUBLIC_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
        m_osInterface,
        &AllocParams,
        &pSurface->OsResource));

    pSurface->dwWidth         = pSrcSurf->dwWidth;
    pSurface->dwHeight        = pSrcSurf->dwHeight;
    pSurface->dwPitch         = pSrcSurf->dwPitch;
    pSurface->dwDepth         = pSrcSurf->dwDepth;
    pSurface->dwQPitch        = pSrcSurf->dwQPitch;
    pSurface->bArraySpacing   = pSrcSurf->bArraySpacing;
    pSurface->bCompressible   = pSrcSurf->bCompressible;
    pSurface->CompressionMode = pSrcSurf->CompressionMode;
    pSurface->bIsCompressed   = pSrcSurf->bIsCompressed;

    if (!m_osInterface->apoMosEnabled)
    {
        MOS_SURFACE details;
        MOS_ZeroMemory(&details, sizeof(details));
        details.Format = Format_Invalid;

        MEDIA_DEBUG_CHK_STATUS(m_osInterface->pfnGetResourceInfo(m_osInterface, &pSurface->OsResource, &details));

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

MOS_STATUS MediaDebugInterface::CopySurfaceData_Vdbox(
    uint32_t      dwDataSize,
    PMOS_RESOURCE presSourceSurface,
    PMOS_RESOURCE presCopiedSurface)
{
    MOS_COMMAND_BUFFER        CmdBuffer;
    MHW_MI_FLUSH_DW_PARAMS    FlushDwParams;
    MHW_GENERIC_PROLOG_PARAMS genericPrologParams;
    MHW_CP_COPY_PARAMS        cpCopyParams;
    MOS_NULL_RENDERING_FLAGS  NullRenderingFlags;
    MOS_GPU_CONTEXT           orgGpuContext;

    if (!m_vdboxContextCreated)
    {
        MOS_GPUCTX_CREATOPTIONS createOption;

        MEDIA_DEBUG_CHK_STATUS(m_osInterface->pfnCreateGpuContext(
            m_osInterface,
            MOS_GPU_CONTEXT_VIDEO,
            MOS_GPU_NODE_VIDEO,
            &createOption));

        // Register VDbox GPU context with the Batch Buffer completion event
        MEDIA_DEBUG_CHK_STATUS(m_osInterface->pfnRegisterBBCompleteNotifyEvent(
            m_osInterface,
            MOS_GPU_CONTEXT_VIDEO));

        m_vdboxContextCreated = true;
    }

    MEDIA_DEBUG_CHK_NULL(m_cpInterface);
    MEDIA_DEBUG_CHK_NULL(m_osInterface);
    MEDIA_DEBUG_CHK_NULL(m_miInterface);
    MEDIA_DEBUG_CHK_NULL(m_osInterface->pfnGetWaTable(m_osInterface));

    orgGpuContext = m_osInterface->CurrentGpuContextOrdinal;

    // Due to VDBOX cryto copy limitation, the size must be Cache line aligned
    if (!MOS_IS_ALIGNED(dwDataSize, MHW_CACHELINE_SIZE))
    {
        MEDIA_DEBUG_ASSERTMESSAGE("Size is not CACHE line aligned, cannot use VDBOX to copy.");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    MEDIA_DEBUG_CHK_STATUS(m_osInterface->pfnSetGpuContext(m_osInterface, MOS_GPU_CONTEXT_VIDEO));
    m_osInterface->pfnResetOsStates(m_osInterface);

    // Register the target resource
    MEDIA_DEBUG_CHK_STATUS(m_osInterface->pfnRegisterResource(
        m_osInterface,
        presCopiedSurface,
        true,
        true));

    // Register the source resource
    MEDIA_DEBUG_CHK_STATUS(m_osInterface->pfnRegisterResource(
        m_osInterface,
        presSourceSurface,
        false,
        true));

    MEDIA_DEBUG_CHK_STATUS(m_osInterface->pfnGetCommandBuffer(m_osInterface, &CmdBuffer, 0));

    MOS_ZeroMemory(&genericPrologParams, sizeof(genericPrologParams));
    genericPrologParams.pOsInterface  = m_osInterface;
    genericPrologParams.pvMiInterface = m_miInterface;
    genericPrologParams.bMmcEnabled   = false;
    MEDIA_DEBUG_CHK_STATUS(Mhw_SendGenericPrologCmd(&CmdBuffer, &genericPrologParams));

    MOS_ZeroMemory(&cpCopyParams, sizeof(cpCopyParams));
    cpCopyParams.size          = dwDataSize;
    cpCopyParams.presSrc       = presSourceSurface;
    cpCopyParams.presDst       = presCopiedSurface;
    cpCopyParams.isEncodeInUse = false;

    MEDIA_DEBUG_CHK_STATUS(m_cpInterface->SetCpCopy(m_osInterface, &CmdBuffer, &cpCopyParams));

    // MI_FLUSH
    MOS_ZeroMemory(&FlushDwParams, sizeof(FlushDwParams));
    MEDIA_DEBUG_CHK_STATUS(m_miInterface->AddMiFlushDwCmd(
        &CmdBuffer,
        &FlushDwParams));

    MEDIA_DEBUG_CHK_STATUS(m_miInterface->AddMiBatchBufferEnd(
        &CmdBuffer,
        nullptr));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &CmdBuffer, 0);

    NullRenderingFlags = m_osInterface->pfnGetNullHWRenderFlags(m_osInterface);

    MEDIA_DEBUG_CHK_STATUS(m_osInterface->pfnSubmitCommandBuffer(
        m_osInterface,
        &CmdBuffer,
        NullRenderingFlags.CtxVideo || NullRenderingFlags.CodecGlobal || NullRenderingFlags.CtxVideo || NullRenderingFlags.VPGobal));

    MEDIA_DEBUG_CHK_STATUS(m_osInterface->pfnSetGpuContext(m_osInterface, orgGpuContext));
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

#endif  // USE_MEDIA_DEBUG_TOOL
