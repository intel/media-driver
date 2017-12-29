/*
* Copyright (c) 2017, Intel Corporation
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

CodechalDebugInterface::CodechalDebugInterface()
{
    memset(&CurrPic, 0, sizeof(CODEC_PICTURE));
    memset(sFileName, 0, sizeof(sFileName));
    memset(sPath, 0, sizeof(sPath));
    memset(sDdiFileName, 0, sizeof(sDdiFileName));
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
    MOS_USER_FEATURE_VALUE_DATA userFeatureData;
    char                        stringData[MOS_MAX_PATH_LENGTH + 1];

    CODECHAL_DEBUG_FUNCTION_ENTER;

    CODECHAL_DEBUG_CHK_NULL(hwInterface);

    pHwInterface  = hwInterface;
    pOsInterface  = pHwInterface->GetOsInterface();

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
        m_outputFilePath = MOS_DEBUG_DEFAULT_OUTPUT_LOCATION;
    }
    CodecFunction = codecFunction;
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
        << " = \"" << sFileName << "\"" << std::endl;
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
    if (wFrameType == I_TYPE)
    {
        frameType = 'I';
    }
    else if (wFrameType == P_TYPE)
    {
        frameType = 'P';
    }
    else if (wFrameType == B_TYPE)
    {
        frameType = 'B';
    }
    else if (wFrameType == MIXED_TYPE)
    {
        frameType = 'M';
    }

    const char *fieldOrder;
    // Sets the Field Order label
    if (CodecHal_PictureIsTopField(CurrPic))
    {
        fieldOrder = CodechalDbgFieldType::topField;
    }
    else if (CodecHal_PictureIsBottomField(CurrPic))
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
                   std::to_string(dwBufferDumpFrameNum) + '-' +
                   std::to_string(dwStreamId) + '_' +
                   std::to_string(slice_id + 1) + 
                   funcName + '_' + bufType + '_' + frameType + fieldOrder + extType;

    }
    else if (bufType != nullptr &&
        !strncmp(bufType, CodechalDbgBufferType::bufEncodePar, sizeof(CodechalDbgBufferType::bufEncodePar) - 1))
    {
        if (!strncmp(funcName, "EncodeSequence", sizeof("EncodeSequence") - 1))
        {
            m_outputFileName = m_outputFilePath +
                std::to_string(dwStreamId) + '_' +
                funcName + extType;
        }
        else
        {
            m_outputFileName = m_outputFilePath +
                std::to_string(dwBufferDumpFrameNum) + '-' +
                std::to_string(dwStreamId) + '_' +
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
                       std::to_string(dwBufferDumpFrameNum) + '-' +
                       std::to_string(dwStreamId) + '_' +
                       funcName + '_' + bufType + '_' +frameType + fieldOrder + extType;
        }
        else
        {
            m_outputFileName = m_outputFilePath +
                       std::to_string(dwBufferDumpFrameNum) + '-' +
                       std::to_string(dwStreamId) + '_'+
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

    CODECHAL_DEBUG_CHK_NULL(pHwInterface);

    bool        batchLockedForDebug = !batchBuffer->bLocked;
    std::string funcName            = batchName ? batchName : m_configMgr->GetMediaStateStr(mediaState);

    if (batchLockedForDebug)
    {
        (Mhw_LockBb(pOsInterface, batchBuffer));
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
        (Mhw_UnlockBb(pOsInterface, batchBuffer, false));
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

        CODECHAL_DEBUG_CHK_NULL(pOsInterface);
        CODECHAL_DEBUG_CHK_STATUS(pOsInterface->pfnGetIndirectStatePointer(
            pOsInterface,
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
    CODECHAL_MEDIA_STATE_TYPE mediaState)
{
    if (!DumpIsEnabled(attrName, mediaState))
    {
        return MOS_STATUS_SUCCESS;
    }

    MOS_LOCK_PARAMS lockFlags;
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.ReadOnly = 1;

    uint8_t *surfBaseAddr = (uint8_t *)pOsInterface->pfnLockResource(pOsInterface, &surface->OsResource, &lockFlags);

    surfBaseAddr += surface->dwOffset + surface->YPlaneOffset.iYOffset * surface->dwPitch;
    uint8_t *data   = surfBaseAddr;
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
    case Format_AYUV:  //444 8bit  -- A[31:24]:Y[23:16]:U[15:8]:V[7:0] = 32bits per pixel = 4Bytes per pixel
        width = width << 2;
        break;
    default:
        break;
    }

    uint32_t pitch = surface->dwPitch;
    if (surface->Format == Format_UYVY)
        pitch = width;

    if (CodecHal_PictureIsBottomField(CurrPic))
    {
        data += pitch;
    }

    if (CodecHal_PictureIsField(CurrPic))
    {
        pitch *= 2;
        height /= 2;
    }

    const char *funcName = CodecFunction == CODECHAL_FUNCTION_DECODE ? "_DEC" : "_ENC";
    std::string bufName  = std::string(surfName) + "_w[" + std::to_string(width) + "]_h[" + std::to_string(height) + "]_p[" + std::to_string(pitch) + "]";

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
    case  Format_AYUV:
    case  Format_AUYV:
    case  Format_Y410: //444 10bit
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
        break;
    default:
        height = 0;
        break;
    }

    data = surfBaseAddr + surface->UPlaneOffset.iLockSurfaceOffset;

    // write chroma data to file
    for (uint32_t h = 0; h < height; h++)
    {
        ofs.write((char*)data, width);
        data += pitch;
    }

    ofs.close();

    if (surfBaseAddr)
    {
        pOsInterface->pfnUnlockResource(pOsInterface, &surface->OsResource);
    }

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
    uint8_t *data      = (uint8_t *)pOsInterface->pfnLockResource(pOsInterface, resource, &lockFlags);
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
        pOsInterface->pfnUnlockResource(pOsInterface, resource);
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
    uint8_t *data      = (uint8_t *)pOsInterface->pfnLockResource(pOsInterface, &surface->OsResource, &lockFlags);
    CODECHAL_DEBUG_CHK_NULL(data);

    const char *fileName;
    if (mediaState == CODECHAL_NUM_MEDIA_STATES)
    {
        fileName = CreateFileName(surfaceName, nullptr, extType);
    }
    else
    {
        std::string kernelName = m_configMgr->GetMediaStateStr(mediaState);
        fileName               = CreateFileName(kernelName.c_str(), surfaceName, extType);
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
        pOsInterface->pfnUnlockResource(pOsInterface, &surface->OsResource);
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
    if (CodecFunction == CODECHAL_FUNCTION_DECODE)
    {
        funcName = "DEC_";
    }
    else if (CodecFunction == CODECHAL_FUNCTION_CENC_DECODE)
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
    if (CodecFunction == CODECHAL_FUNCTION_DECODE)
    {
        funcName = "DEC_";
    }
    else if (CodecFunction == CODECHAL_FUNCTION_CENC_DECODE)
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
    default:
        funcName = funcName + inputName + bufName + regionNumName + regionName + "_Pass" + passName;
        break;
    }
 
    return DumpBuffer(region, nullptr, funcName.c_str(), regionSize, regionOffset);
}


MOS_STATUS CodechalDebugInterface::DeleteCfgLinkNode(uint32_t frameIdx)
{
   return m_configMgr->DeleteCfgNode(frameIdx);
}

MOS_STATUS CodechalDebugInterface::DumpVp9EncodeSeqParams(
    PCODEC_VP9_ENCODE_SEQUENCE_PARAMS seqParams)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (!m_configMgr->AttrIsEnabled(CodechalDbgAttr::attrSeqParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DEBUG_CHK_NULL(seqParams);

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    oss << "# DDI Parameters:" << std::endl;
    oss << "MaxFrameWidth = " << std::dec << +seqParams->wMaxFrameWidth << std::endl;
    oss << "MaxFrameHeight = " << std::dec << +seqParams->wMaxFrameHeight << std::endl;
    oss << "GopPicSize = " << std::dec << +seqParams->GopPicSize << std::endl;
    oss << "TargetUsage = " << std::dec << +seqParams->TargetUsage << std::endl;
    oss << "RateControlMethod = " << std::dec << +seqParams->RateControlMethod << std::endl;

    for (uint8_t i = 0; i < 8; i++)
    {
        oss << "TargetBitRate[" << +i << "] = " << std::dec << +seqParams->TargetBitRate[i] << std::endl;
    }
    oss << "MaxBitRate = " << std::dec << +seqParams->MaxBitRate << std::endl;
    oss << "MinBitRate = " << std::dec << +seqParams->MinBitRate << std::endl;
    oss << "InitVBVBufferFullnessInBit = " << +seqParams->InitVBVBufferFullnessInBit << std::endl;
    oss << "VBVBufferSizeInBit = " << std::dec << +seqParams->VBVBufferSizeInBit << std::endl;
    oss << "OptimalVBVBufferLevelInBit = " << std::dec << +seqParams->OptimalVBVBufferLevelInBit << std::endl;
    oss << "UpperVBVBufferLevelThresholdInBit = " << std::dec << +seqParams->UpperVBVBufferLevelThresholdInBit << std::endl;
    oss << "LowerVBVBufferLevelThresholdInBit = " << std::dec << +seqParams->LowerVBVBufferLevelThresholdInBit << std::endl;
    oss << "DisplayFormatSwizzle = " << std::dec << +seqParams->SeqFlags.fields.DisplayFormatSwizzle << std::endl;
    // begining of union/struct
    oss << "# bResetBRC = " << std::dec << +seqParams->SeqFlags.fields.bResetBRC << std::endl;
    oss << "# bNoFrameHeaderInsertion = " << std::dec << +seqParams->SeqFlags.fields.bNoFrameHeaderInsertion << std::endl;
    // Next 5 fields not currently implemented.  nullptr output
    oss << "# UseRawReconRef = " << std::dec << +seqParams->SeqFlags.fields.bUseRawReconRef << std::endl;
    oss << "# MBBRC = " << std::dec << +seqParams->SeqFlags.fields.MBBRC << std::endl;
    oss << "EnableDynamicScaling = " << std::dec << +seqParams->SeqFlags.fields.EnableDynamicScaling << std::endl;
    // end of union/struct

    oss << "UserMaxFrameSize = " << std::dec << +seqParams->UserMaxFrameSize << std::endl;
    for (uint8_t i = 0; i < 8; i++)
    {
        oss << "FrameRateNumerator[" << +i << "] = " << std::dec << +seqParams->FrameRate[i].uiNumerator << std::endl;
        oss << "FrameRateDenominator[" << +i << "] = " << std::dec << +seqParams->FrameRate[i].uiDenominator << std::endl;
    }

    oss << "NumTemporalLayersMinus1 = " << std::dec << +seqParams->NumTemporalLayersMinus1 << std::endl;

    const char *fileName = CreateFileName(
        "_DDIEnc",
        CodechalDbgBufferType::bufSeqParams,
        CodechalDbgExtType::txt);

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();

    if (m_configMgr->AttrIsEnabled(CodechalDbgAttr::attrDriverUltDump))
    {
        if (!m_ddiFileName.empty())
        {
            std::ofstream ofs(m_ddiFileName, std::ios::app);
            ofs << "SeqParamFile"
                << " = \"" << sFileName << "\"" << std::endl;
            ofs.close();
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalDebugInterface::DumpVp9EncodePicParams(
    PCODEC_VP9_ENCODE_PIC_PARAMS picParams)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (!m_configMgr->AttrIsEnabled(CodechalDbgAttr::attrPicParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DEBUG_CHK_NULL(picParams);

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    oss << "# DDI Parameters:" << std::endl;
    oss << "SrcFrameHeightMinus1 = " << std::dec << +picParams->SrcFrameHeightMinus1 << std::endl;
    oss << "SrcFrameWidthMinus1 = " << std::dec << +picParams->SrcFrameWidthMinus1 << std::endl;
    oss << "DstFrameHeightMinus1 = " << std::dec << +picParams->DstFrameHeightMinus1 << std::endl;
    oss << "DstFrameWidthMinus1 = " << std::dec << +picParams->DstFrameWidthMinus1 << std::endl;
    oss << "CurrOriginalPic = " << std::dec << +picParams->CurrOriginalPic.FrameIdx << std::endl;
    oss << "CurrReconstructedPic = " << std::dec << +picParams->CurrReconstructedPic.FrameIdx << std::endl;

    for (uint16_t i = 0; i < CODEC_VP9_NUM_REF_FRAMES; ++i)
    {
        oss << "RefFrameList[" << +i << "] = " << std::dec << +picParams->RefFrameList[i].FrameIdx << std::endl;
    }
    oss << "frame_type = " << std::dec << +picParams->PicFlags.fields.frame_type << std::endl;
    oss << "show_frame = " << std::dec << +picParams->PicFlags.fields.show_frame << std::endl;
    oss << "error_resilient_mode = " << std::dec << +picParams->PicFlags.fields.error_resilient_mode << std::endl;
    oss << "intra_only = " << std::dec << +picParams->PicFlags.fields.intra_only << std::endl;
    oss << "allow_high_precision_mv = " << std::dec << +picParams->PicFlags.fields.allow_high_precision_mv << std::endl;
    oss << "mcomp_filter_type = " << std::dec << +picParams->PicFlags.fields.mcomp_filter_type << std::endl;
    oss << "frame_parallel_decoding_mode = " << std::dec << +picParams->PicFlags.fields.frame_parallel_decoding_mode << std::endl;
    oss << "segmentation_enabled = " << std::dec << +picParams->PicFlags.fields.segmentation_enabled << std::endl;
    oss << "segmentation_temporal_update = " << std::dec << +picParams->PicFlags.fields.segmentation_temporal_update << std::endl;
    oss << "segmentation_update_map = " << std::dec << +picParams->PicFlags.fields.segmentation_update_map << std::endl;
    oss << "reset_frame_context = " << std::dec << +picParams->PicFlags.fields.reset_frame_context << std::endl;
    oss << "refresh_frame_context = " << std::dec << +picParams->PicFlags.fields.refresh_frame_context << std::endl;
    oss << "frame_context_idx = " << std::dec << +picParams->PicFlags.fields.frame_context_idx << std::endl;
    oss << "LosslessFlag = " << std::dec << +picParams->PicFlags.fields.LosslessFlag << std::endl;
    oss << "comp_prediction_mode = " << std::dec << +picParams->PicFlags.fields.comp_prediction_mode << std::endl;
    oss << "super_frame = " << std::dec << +picParams->PicFlags.fields.super_frame << std::endl;
    oss << "seg_id_block_size = " << std::dec << +picParams->PicFlags.fields.seg_id_block_size << std::endl;
    oss << "seg_update_data = " << std::dec << +picParams->PicFlags.fields.seg_update_data << std::endl;
    oss << "LastRefIdx = " << std::dec << +picParams->RefFlags.fields.LastRefIdx << std::endl;
    oss << "LastRefSignBias = " << std::dec << +picParams->RefFlags.fields.LastRefSignBias << std::endl;
    oss << "GoldenRefIdx = " << std::dec << +picParams->RefFlags.fields.GoldenRefIdx << std::endl;
    oss << "GoldenRefSignBias = " << std::dec << +picParams->RefFlags.fields.GoldenRefSignBias << std::endl;
    oss << "AltRefIdx = " << std::dec << +picParams->RefFlags.fields.AltRefIdx << std::endl;
    oss << "AltRefSignBias = " << std::dec << +picParams->RefFlags.fields.AltRefSignBias << std::endl;
    oss << "ref_frame_ctrl_l0 = " << std::dec << +picParams->RefFlags.fields.ref_frame_ctrl_l0 << std::endl;
    oss << "ref_frame_ctrl_l1 = " << std::dec << +picParams->RefFlags.fields.ref_frame_ctrl_l1 << std::endl;
    oss << "refresh_frame_flags = " << std::dec << +picParams->RefFlags.fields.refresh_frame_flags << std::endl;
    oss << "LumaACQIndex = " << std::dec << +picParams->LumaACQIndex << std::endl;
    oss << "LumaDCQIndexDelta = " << std::dec << +picParams->LumaDCQIndexDelta << std::endl;
    oss << "ChromaACQIndexDelta = " << std::dec << +picParams->ChromaACQIndexDelta << std::endl;
    oss << "ChromaDCQIndexDelta = " << std::dec << +picParams->ChromaDCQIndexDelta << std::endl;
    oss << "filter_level = " << std::dec << +picParams->filter_level << std::endl;
    oss << "sharpness_level = " << std::dec << +picParams->sharpness_level << std::endl;

    for (uint8_t i = 0; i < 4; ++i)
    {
        oss << "LFRefDelta[" << +i << "] = " << std::dec << +picParams->LFRefDelta[i] << std::endl;
    }

    for (uint8_t i = 0; i < 2; ++i)
    {
        oss << "LFModeDelta[" << +i << "] = " << std::dec << +picParams->LFModeDelta[i] << std::endl;
    }

    oss << "BitOffsetForLFRefDelta = " << std::dec << +picParams->BitOffsetForLFRefDelta << std::endl;
    oss << "BitOffsetForLFModeDelta = " << std::dec << +picParams->BitOffsetForLFModeDelta << std::endl;
    oss << "BitOffsetForLFLevel = " << std::dec << +picParams->BitOffsetForLFLevel << std::endl;
    oss << "BitOffsetForQIndex = " << std::dec << +picParams->BitOffsetForQIndex << std::endl;
    oss << "BitOffsetForFirstPartitionSize = " << std::dec << +picParams->BitOffsetForFirstPartitionSize << std::endl;
    oss << "BitOffsetForSegmentation = " << std::dec << +picParams->BitOffsetForSegmentation << std::endl;
    oss << "BitSizeForSegmentation = " << std::dec << +picParams->BitSizeForSegmentation << std::endl;
    oss << "log2_tile_rows = " << std::dec << +picParams->log2_tile_rows << std::endl;
    oss << "log2_tile_columns = " << std::dec << +picParams->log2_tile_columns << std::endl;
    oss << "temporal_id = " << std::dec << +picParams->temporal_id << std::endl;
    oss << "StatusReportFeedbackNumber = " << std::dec << +picParams->StatusReportFeedbackNumber << std::endl;
    oss << "SkipFrameFlag = " << std::dec << +picParams->SkipFrameFlag << std::endl;
    oss << "NumSkipFrames = " << std::dec << +picParams->NumSkipFrames << std::endl;
    oss << "SizeSkipFrames = " << std::dec << +picParams->SizeSkipFrames << std::endl;

    const char *fileName = CreateFileName(
        "_DDIEnc",
        CodechalDbgBufferType::bufPicParams,
        CodechalDbgExtType::txt);

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();

    if (m_configMgr->AttrIsEnabled(CodechalDbgAttr::attrDriverUltDump))
    {
        if (!m_ddiFileName.empty())
        {
            std::ofstream ofs(m_ddiFileName, std::ios::app);
            ofs << "PicNum"
                << " = " << dwBufferDumpFrameNum << std::endl;
            ofs << "PicParamFile"
                << " = \"" << sFileName << "\"" << std::endl;
            ofs.close();
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalDebugInterface::DumpVp9EncodeSegmentParams(
    PCODEC_VP9_ENCODE_SEGMENT_PARAMS segmentParams)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (!m_configMgr->AttrIsEnabled(CodechalDbgAttr::attrSegmentParams))
    {
        return MOS_STATUS_SUCCESS;
    }
    CODECHAL_DEBUG_CHK_NULL(segmentParams);

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    for (uint8_t i = 0; i < 8; ++i)
    {
        oss << "Segment_id = " << std::dec << +i << std::endl;
        oss << "SegmentReferenceEnabled = " << std::dec << +segmentParams->SegData[i].SegmentFlags.fields.SegmentReferenceEnabled << std::endl;
        oss << "SegmentReference = " << std::dec << +segmentParams->SegData[i].SegmentFlags.fields.SegmentReference << std::endl;
        oss << "SegmentSkipped = " << std::dec << +segmentParams->SegData[i].SegmentFlags.fields.SegmentSkipped << std::endl;
        oss << "SegmentLFLevelDelta = " << std::dec << +segmentParams->SegData[i].SegmentLFLevelDelta << std::endl;
        oss << "SegmentQIndexDelta = " << std::dec << +segmentParams->SegData[i].SegmentQIndexDelta << std::endl;
    }

    if (m_configMgr->AttrIsEnabled(CodechalDbgAttr::attrDriverUltDump))
    {
        if (!m_ddiFileName.empty())
        {
            std::ofstream ofs(m_ddiFileName, std::ios::app);
            ofs << "SegmentParamFileParamFile"
                << " = \"" << sFileName << "\"" << std::endl;
            ofs.close();
        }
    }

    const char *fileName = CreateFileName(
        "_DDIEnc",
        CodechalDbgBufferType::bufSegmentParams,
        CodechalDbgExtType::txt);

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();

    return MOS_STATUS_SUCCESS;
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
