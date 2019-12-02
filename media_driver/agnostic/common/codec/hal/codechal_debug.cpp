/*
* Copyright (c) 2017-2019, Intel Corporation
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
//! \brief    Defines the debug interface shared by all of CodecHal.
//! \details  The debug interface dumps output from CodecHal based on in input config file.
//!
#include "codechal_debug.h"
#if USE_CODECHAL_DEBUG_TOOL
#include "codechal_debug_config_manager.h"
#include "codechal_hw.h"
#include <fstream>
#include <sstream>
#include <iomanip>

#if !defined(LINUX) && !defined(ANDROID)
#include "UmdStateSeparation.h"
#endif

CodechalDebugInterface::CodechalDebugInterface()
{
    memset(&m_currPic, 0, sizeof(CODEC_PICTURE));
    memset(m_fileName, 0, sizeof(m_fileName));
    memset(m_path, 0, sizeof(m_path));
}
CodechalDebugInterface::~CodechalDebugInterface()
{
    if (nullptr != m_configMgr)
    {
        MOS_Delete(m_configMgr);
    }
}

MOS_STATUS CodechalDebugInterface::Initialize(
    CodechalHwInterface *hwInterface,
    CODECHAL_FUNCTION      codecFunction)
{
    MOS_USER_FEATURE_VALUE_DATA       userFeatureData;
    MOS_USER_FEATURE_VALUE_WRITE_DATA userFeatureWriteData;
    char                              stringData[MOS_MAX_PATH_LENGTH + 1];
    std::string                       codechalDumpFilePath;

    CODECHAL_DEBUG_FUNCTION_ENTER;

    CODECHAL_DEBUG_CHK_NULL(hwInterface);

    m_hwInterface = hwInterface;
    m_osInterface = m_hwInterface->GetOsInterface();

#ifdef LINUX
    char* customizedOutputPath = getenv("MOS_DEBUG_OUTPUT_LOCATION");
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
        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
        userFeatureData.StringData.pStringData = stringData;
        MOS_UserFeature_ReadValue_ID(
            NULL,
            __MEDIA_USER_FEATURE_VALUE_CODECHAL_DEBUG_OUTPUT_DIRECTORY_ID,
            &userFeatureData);

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
            if (SUCCEEDED(GetDriverPersistentStorageLocation(codechalDumpFilePath)))
            {
                m_outputFilePath = codechalDumpFilePath.c_str();
                m_outputFilePath.append(MOS_CODECHAL_DUMP_OUTPUT_FOLDER);

                MOS_ZeroMemory(&userFeatureWriteData, sizeof(userFeatureWriteData));
                userFeatureWriteData.Value.StringData.pStringData = const_cast<char *>(m_outputFilePath.c_str());
                userFeatureWriteData.Value.StringData.uSize       = m_outputFilePath.size();
                userFeatureWriteData.ValueID                      = __MEDIA_USER_FEATURE_VALUE_CODECHAL_DUMP_OUTPUT_DIRECTORY_ID;
                MOS_UserFeature_WriteValues_ID(NULL, &userFeatureWriteData, 1);
            }
            else
            {
                return MOS_STATUS_UNKNOWN;
            }
#endif
        }
    }

    m_codecFunction = codecFunction;
    m_configMgr = MOS_New(CodechalDebugConfigMgr, this, codecFunction, m_outputFilePath);
    CODECHAL_DEBUG_CHK_NULL(m_configMgr);
    CODECHAL_DEBUG_CHK_STATUS(m_configMgr->ParseConfig());

    // Create thread specified sub folder as dump folder.
    if (m_configMgr->AttrIsEnabled(CodechalDbgAttr::attrDumpToThreadFolder))
    {
        std::string  ThreadSubFolder = "T" + std::to_string(MOS_GetCurrentThreadId()) + MOS_DIRECTORY_DELIMITER;
        m_outputFilePath = m_outputFilePath + ThreadSubFolder;
        MOS_CreateDirectory(const_cast<char*>(m_outputFilePath.c_str()));
    }

    m_ddiFileName = m_outputFilePath + "ddi.par";
    std::ofstream ofs(m_ddiFileName, std::ios::out);
    ofs << "ParamFilePath"
        << " = \"" << m_fileName << "\"" << std::endl;
    ofs.close();

    return MOS_STATUS_SUCCESS;
}

bool CodechalDebugInterface::DumpIsEnabled(
    const char *attr,
    CODECHAL_MEDIA_STATE_TYPE mediaState)
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
        fieldOrder = CodechalDbgFieldType::topField;
    }
    else if (CodecHal_PictureIsBottomField(m_currPic))
    {
        fieldOrder = CodechalDbgFieldType::botField;
    }
    else
    {
        fieldOrder = CodechalDbgFieldType::frame;
    }

    // Sets the Postfix label
    if (m_configMgr->AttrIsEnabled(CodechalDbgAttr::attrDumpBufferInBinary) &&
        strcmp(extType, CodechalDbgExtType::txt) == 0)
    {
        extType = CodechalDbgExtType::dat;
    }

    if (bufType != nullptr &&
        !strncmp(bufType, CodechalDbgBufferType::bufSlcParams, sizeof(CodechalDbgBufferType::bufSlcParams) - 1)
        && !strncmp(funcName, "_DDIEnc", sizeof("_DDIEnc") - 1))
    {
        m_outputFileName = m_outputFilePath +
                           std::to_string(m_bufferDumpFrameNum) + '-' +
                           std::to_string(m_streamId) + '_' +
                           std::to_string(m_sliceId + 1) +
                           funcName + '_' + bufType + '_' + frameType + fieldOrder + extType;
    }
    else if (bufType != nullptr &&
        !strncmp(bufType, CodechalDbgBufferType::bufEncodePar, sizeof(CodechalDbgBufferType::bufEncodePar) - 1))
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
        if (funcName[0]=='_')
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

MOS_STATUS CodechalDebugInterface::DumpCmdBuffer(
    PMOS_COMMAND_BUFFER       cmdBuffer,
    CODECHAL_MEDIA_STATE_TYPE mediaState,
    const char*    cmdName)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    bool attrEnabled = m_configMgr->AttrIsEnabled(CodechalDbgAttr::attrCmdBufferMfx);

    if (!attrEnabled && mediaState != CODECHAL_NUM_MEDIA_STATES)
    {
        attrEnabled = m_configMgr->AttrIsEnabled(mediaState, CodechalDbgAttr::attrCmdBuffer);
    }

    if (!attrEnabled)
    {
        return MOS_STATUS_SUCCESS;
    }

    bool binaryDumpEnabled = m_configMgr->AttrIsEnabled(CodechalDbgAttr::attrDumpCmdBufInBinary);

    std::string funcName = cmdName ? cmdName : m_configMgr->GetMediaStateStr(mediaState);
    const char *fileName = CreateFileName(
        funcName.c_str(),
        CodechalDbgBufferType::bufCmd,
        binaryDumpEnabled ? CodechalDbgExtType::dat : CodechalDbgExtType::txt);

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
    const char*     batchName)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    bool attrEnabled = m_configMgr->AttrIsEnabled(CodechalDbgAttr::attr2ndLvlBatchMfx);

    if (!attrEnabled && mediaState != CODECHAL_NUM_MEDIA_STATES)
    {
        attrEnabled = m_configMgr->AttrIsEnabled(mediaState, CodechalDbgAttr::attr2ndLvlBatch);
    }

    if (!attrEnabled)
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DEBUG_CHK_NULL(m_hwInterface);

    bool        batchLockedForDebug = !batchBuffer->bLocked;
    std::string funcName            = batchName ? batchName : m_configMgr->GetMediaStateStr(mediaState);

    if (batchLockedForDebug)
    {
        (Mhw_LockBb(m_osInterface, batchBuffer));
    }

    const char *fileName = CreateFileName(
        funcName.c_str(),
        CodechalDbgBufferType::buf2ndLvl,
        CodechalDbgExtType::txt);

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
    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (mediaState >= CODECHAL_NUM_MEDIA_STATES ||
        !m_configMgr->AttrIsEnabled(mediaState, CodechalDbgAttr::attrCurbe))
    {
        return MOS_STATUS_SUCCESS;
    }

    std::string funcName = m_configMgr->GetMediaStateStr(mediaState);
    bool binaryDump = m_configMgr->AttrIsEnabled(CodechalDbgAttr::attrDumpBufferInBinary);

    const char *fileName = CreateFileName(
        funcName.c_str(),
        CodechalDbgBufferType::bufCurbe,
        CodechalDbgExtType::txt);

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
    CODECHAL_DEBUG_FUNCTION_ENTER;

    uint8_t     *curbeAlignedData = nullptr;
    uint32_t     curbeAlignedSize = 0;
    MOS_STATUS   eStatus = MOS_STATUS_SUCCESS;

    if (mediaState >= CODECHAL_NUM_MEDIA_STATES ||
        !m_configMgr->AttrIsEnabled(mediaState, CodechalDbgAttr::attrCurbe))
    {
        return eStatus;
    }

    std::string funcName = m_configMgr->GetMediaStateStr(mediaState);
    bool binaryDump = m_configMgr->AttrIsEnabled(CodechalDbgAttr::attrDumpBufferInBinary);
    const char* extType = binaryDump ? CodechalDbgExtType::dat : CodechalDbgExtType::txt;

    const char *fileName = CreateFileName(
        funcName.c_str(),
        CodechalDbgBufferType::bufCurbe,
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
    CODECHAL_DEBUG_FUNCTION_ENTER;

    uint8_t *sshData = nullptr;
    uint32_t sshSize = 0;

    MemoryBlock *regionBlock = nullptr;
    bool         attrEnabled = false;
    const char * bufferType;
    if (stateHeap == MHW_ISH_TYPE)
    {
        regionBlock = &kernelState->m_ishRegion;
        attrEnabled = m_configMgr->AttrIsEnabled(mediaState, CodechalDbgAttr::attrIsh);
        bufferType  = CodechalDbgBufferType::bufISH;
    }
    else if (stateHeap == MHW_DSH_TYPE)
    {
        regionBlock = &kernelState->m_dshRegion;
        attrEnabled = m_configMgr->AttrIsEnabled(mediaState, CodechalDbgAttr::attrDsh);
        bufferType  = CodechalDbgBufferType::bufDSH;
    }
    else
    {
        attrEnabled = m_configMgr->AttrIsEnabled(mediaState, CodechalDbgAttr::attrSsh);
        bufferType  = CodechalDbgBufferType::bufSSH;

        CODECHAL_DEBUG_CHK_NULL(m_osInterface);
        CODECHAL_DEBUG_CHK_STATUS(m_osInterface->pfnGetIndirectStatePointer(
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
        CodechalDbgExtType::txt);

    bool binaryDump = m_configMgr->AttrIsEnabled(CodechalDbgAttr::attrDumpBufferInBinary);

    if (regionBlock)
    {
        return regionBlock->Dump(fileName, 0, 0, binaryDump);
    }
    else
    {
        return DumpBufferInHexDwords(sshData, sshSize);
    }
}

MOS_STATUS CodechalDebugInterface::DumpYUVSurface(
    PMOS_SURFACE              surface,
    const char *              attrName,
    const char *              surfName,
    CODECHAL_MEDIA_STATE_TYPE mediaState,
    uint32_t                  width_in,
    uint32_t                  height_in)
{
    if (!DumpIsEnabled(attrName, mediaState))
    {
        return MOS_STATUS_SUCCESS;
    }

    MOS_LOCK_PARAMS lockFlags;
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.ReadOnly = 1;
    lockFlags.TiledAsTiled = 1; // Bypass GMM CPU blit due to some issues in GMM CpuBlt function

    uint8_t *lockedAddr = (uint8_t *)m_osInterface->pfnLockResource(m_osInterface, &surface->OsResource, &lockFlags);
    CODECHAL_DEBUG_CHK_NULL(lockedAddr)

    // Always use MOS swizzle instead of GMM Cpu blit
    uint32_t sizeMain = (uint32_t)(surface->OsResource.pGmmResInfo->GetSizeMainSurface());
    uint8_t *surfBaseAddr = (uint8_t*)MOS_AllocMemory(sizeMain);
    CODECHAL_DEBUG_CHK_NULL(surfBaseAddr);

    if (DumpIsEnabled(CodechalDbgAttr::attrForceYUVDumpWithMemcpy))
    {
        MOS_SecureMemcpy(surfBaseAddr, sizeMain, lockedAddr, sizeMain); // Firstly, copy to surfBaseAddr to faster unlock resource
        m_osInterface->pfnUnlockResource(m_osInterface, &surface->OsResource);
        lockedAddr = surfBaseAddr;
        surfBaseAddr = (uint8_t*)MOS_AllocMemory(sizeMain);
        CODECHAL_DEBUG_CHK_NULL(surfBaseAddr);
    }

    Mos_SwizzleData(lockedAddr, surfBaseAddr, surface->TileType, MOS_TILE_LINEAR, sizeMain / surface->dwPitch, surface->dwPitch, 0);

    uint8_t *data = surfBaseAddr;
    data += surface->dwOffset + surface->YPlaneOffset.iYOffset * surface->dwPitch;

    uint32_t width  = width_in ? width_in : surface->dwWidth;
    uint32_t height = height_in? height_in : surface->dwHeight;

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

    const char *funcName = (m_codecFunction == CODECHAL_FUNCTION_DECODE) ? "_DEC" :(m_codecFunction == CODECHAL_FUNCTION_CENC_DECODE ? "_DEC" : "_ENC");
    std::string bufName  = std::string(surfName) + "_w[" + std::to_string(surface->dwWidth) + "]_h[" + std::to_string(surface->dwHeight) + "]_p[" + std::to_string(pitch) + "]";

    const char *filePath = CreateFileName(funcName, bufName.c_str(), CodechalDbgExtType::yuv);

    std::ofstream ofs(filePath, std::ios_base::out | std::ios_base::binary);
    if (ofs.fail())
    {
        return MOS_STATUS_UNKNOWN;
    }

    // write luma data to file
    for (uint32_t h = 0; h < height; h++)
    {
        ofs.write((char* )data, width);
        data += pitch;
    }

    switch (surface->Format)
    {
    case Format_NV12:
    case Format_P010:
    case Format_P016:
        height >>= 1;
        break;
    case  Format_Y416:
    case  Format_AUYV:
    case  Format_Y410: //444 10bit
    case  Format_R10G10B10A2:
        height *= 2;
        break;
    case  Format_YUY2:
    case  Format_YUYV:
    case  Format_YUY2V:
    case  Format_Y216V:
    case  Format_YVYU:
    case  Format_UYVY:
    case  Format_VYUY:
    case  Format_Y216: //422 16bit
    case  Format_Y210: //422 10bit
    case  Format_P208: //422 8bit
        break;
    case Format_422V:
    case Format_IMC3:
        height = height / 2;
        break;
    case  Format_AYUV:
    default:
        height = 0;
        break;
    }

uint8_t *vPlaneData = surfBaseAddr;
#ifdef LINUX
    data = surfBaseAddr + surface->UPlaneOffset.iSurfaceOffset;
    if (surface->Format == Format_422V
        || surface->Format == Format_IMC3)
    {
        vPlaneData = surfBaseAddr + surface->VPlaneOffset.iSurfaceOffset;
    }
#else
    data = surfBaseAddr + surface->UPlaneOffset.iLockSurfaceOffset;
    if (surface->Format == Format_422V
        || surface->Format == Format_IMC3)
    {
        vPlaneData = surfBaseAddr + surface->VPlaneOffset.iLockSurfaceOffset;
    }

#endif

    // write chroma data to file
    for (uint32_t h = 0; h < height; h++)
    {
        ofs.write((char*)data, width);
        data += pitch;
    }

    // write v planar data to file
    if (surface->Format == Format_422V
        || surface->Format == Format_IMC3)
    {
        for (uint32_t h = 0; h < height; h++)
        {
            ofs.write((char *)vPlaneData, width);
            vPlaneData += pitch;
        }
    }

    ofs.close();

    if (DumpIsEnabled(CodechalDbgAttr::attrForceYUVDumpWithMemcpy))
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

MOS_STATUS CodechalDebugInterface::DumpBuffer(
    PMOS_RESOURCE             resource,
    const char *              attrName,
    const char *              bufferName,
    uint32_t                  size,
    uint32_t                  offset,
    CODECHAL_MEDIA_STATE_TYPE mediaState)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    CODECHAL_DEBUG_CHK_NULL(resource);
    CODECHAL_DEBUG_CHK_NULL(bufferName);

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
    CODECHAL_DEBUG_CHK_NULL(data);
    data += offset;

    const char *fileName;
    bool binaryDump = m_configMgr->AttrIsEnabled(CodechalDbgAttr::attrDumpBufferInBinary);
    const char* extType = binaryDump ? CodechalDbgExtType::dat : CodechalDbgExtType::txt;

    if (mediaState == CODECHAL_NUM_MEDIA_STATES)
    {
        fileName = CreateFileName(bufferName, attrName, extType);
    }
    else
    {
        std::string kernelName = m_configMgr->GetMediaStateStr(mediaState);
        fileName           = CreateFileName(kernelName.c_str(), bufferName, extType);
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

MOS_STATUS CodechalDebugInterface::DumpSurface(
    PMOS_SURFACE              surface,
    const char *              attrName,
    const char *              surfaceName,
    CODECHAL_MEDIA_STATE_TYPE mediaState)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    CODECHAL_DEBUG_CHK_NULL(surface);
    CODECHAL_DEBUG_CHK_NULL(attrName);
    CODECHAL_DEBUG_CHK_NULL(surfaceName);

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

    bool binaryDump = m_configMgr->AttrIsEnabled(CodechalDbgAttr::attrDumpBufferInBinary);
    const char* extType = binaryDump ? CodechalDbgExtType::dat : CodechalDbgExtType::txt;

    MOS_LOCK_PARAMS lockFlags;
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.ReadOnly = 1;
    uint8_t *data      = (uint8_t *)m_osInterface->pfnLockResource(m_osInterface, &surface->OsResource, &lockFlags);
    CODECHAL_DEBUG_CHK_NULL(data);
    
    std::string bufName  = std::string(surfaceName) + "_w[" + std::to_string(surface->dwWidth) + "]_h[" + std::to_string(surface->dwHeight) + "]_p[" + std::to_string(surface->dwPitch) + "]";
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
        status = DumpBufferInHexDwords(data, surface->dwHeight*surface->dwPitch);
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
    CODECHAL_DEBUG_FUNCTION_ENTER;

    CODECHAL_DEBUG_CHK_NULL(data);
    CODECHAL_DEBUG_CHK_NULL(attrName);
    CODECHAL_DEBUG_CHK_NULL(bufferName);

    if (!m_configMgr->AttrIsEnabled(attrName))
    {
        return MOS_STATUS_SUCCESS;
    }

    bool binaryDump = m_configMgr->AttrIsEnabled(CodechalDbgAttr::attrDumpBufferInBinary);
    const char *fileName = CreateFileName(bufferName, nullptr,
                                          binaryDump ? CodechalDbgExtType::dat : CodechalDbgExtType::txt);

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

MOS_STATUS CodechalDebugInterface::DumpHucDmem(
    PMOS_RESOURCE             dmemResource,
    uint32_t                  dmemSize,
    uint32_t                  hucPassNum,
    CodechalHucRegionDumpType dumpType)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (!m_configMgr->AttrIsEnabled(CodechalDbgAttr::attrHuCDmem))
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

    std::string dmemName = CodechalDbgBufferType::bufHucDmem;
    std::string passName = std::to_string(hucPassNum);
    switch (dumpType)
    {
    case hucRegionDumpInit:
        funcName = funcName + dmemName + "_InitPass" + passName;
        break;
    case hucRegionDumpUpdate:
        funcName = funcName + dmemName + "_UpdatePass" + passName;
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
    case hucRegionDumpBackAnnotation:
        funcName = funcName + dmemName + "_BackAnnotationPass" + passName;
        break;
    default:
        funcName = funcName + dmemName + "_Pass" + passName;
        break;
    }

    return DumpBuffer(dmemResource, nullptr, funcName.c_str(), dmemSize);
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

    if (!m_configMgr->AttrIsEnabled(CodechalDbgAttr::attrHucRegions))
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
        funcName = "DEC_Cenc_";
    }
    else
    {
        funcName = "ENC_";
    }

    std::string bufName       = CodechalDbgBufferType::bufHucRegion;
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

    return DumpBuffer(region, nullptr, funcName.c_str(), regionSize, regionOffset);
}

MOS_STATUS CodechalDebugInterface::DumpBltOutput(
    PMOS_SURFACE              surface,
    const char *              attrName)
{
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalDebugInterface::DeleteCfgLinkNode(uint32_t frameIdx)
{
   return m_configMgr->DeleteCfgNode(frameIdx);
}

MOS_STATUS CodechalDebugInterface::DumpBufferInBinary(uint8_t *data, uint32_t size)
{
    CODECHAL_DEBUG_CHK_NULL(data);

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

    ofs.write((char*)data, size);
    ofs.close();
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalDebugInterface::Dump2DBufferInBinary(
    uint8_t *   data,
    uint32_t    width,
    uint32_t    height,
    uint32_t    pitch)
{
    CODECHAL_DEBUG_CHK_NULL(data);

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
        ofs.write((char*)data, width);
        data += pitch;
    }

    ofs.close();
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalDebugInterface::DumpBufferInHexDwords(uint8_t *data, uint32_t size)
{
    CODECHAL_DEBUG_CHK_NULL(data);

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

#endif  // USE_CODECHAL_DEBUG_TOOL
