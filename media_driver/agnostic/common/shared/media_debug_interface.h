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
//! \file     media_debug_interface.h
//! \brief    Defines the debug interface shared by all of Media.
//! \details  The debug interface dumps output from Media based on input config file.
//!
#ifndef __MEDIA_DEBUG_INTERFACE_H__
#define __MEDIA_DEBUG_INTERFACE_H__

#if (_DEBUG || _RELEASE_INTERNAL)
#define USE_MEDIA_DEBUG_TOOL 1
#define MEDIA_DEBUG_TOOL(expr) expr;

#include "BRCIF.h"
#include "mhw_utilities.h"
#include "mhw_state_heap.h"
#include "media_debug_config_manager.h"
#include "media_debug_utils.h"
#include <sstream>
#include <fstream>

class MediaDebugInterface
{
public:
    MediaDebugInterface();
    virtual ~MediaDebugInterface();

    MOS_STATUS InitDumpLocation();

    bool DumpIsEnabled(
        const char *           attr,
        MEDIA_DEBUG_STATE_TYPE mediaState = CODECHAL_NUM_MEDIA_STATES);

    const char *CreateFileName(
        const char *funcName,
        const char *bufType,
        const char *extType);

    MOS_STATUS SetOutputFilePath();

    MOS_STATUS DumpCmdBuffer(
        PMOS_COMMAND_BUFFER    cmdBuffer,
        MEDIA_DEBUG_STATE_TYPE mediaState,
        const char *           cmdName = nullptr);

    MOS_STATUS Dump2ndLvlBatch(
        PMHW_BATCH_BUFFER      batchBuffer,
        MEDIA_DEBUG_STATE_TYPE mediaState,
        const char *           batchName = nullptr);

    MOS_STATUS DumpCurbe(
        MEDIA_DEBUG_STATE_TYPE mediaState,
        PMHW_KERNEL_STATE      kernelState);

    MOS_STATUS DumpMDFCurbe(
        MEDIA_DEBUG_STATE_TYPE mediaState,
        uint8_t *              curbeBuffer,
        uint32_t               curbeSize);

    MOS_STATUS DumpKernelRegion(
        MEDIA_DEBUG_STATE_TYPE mediaState,
        MHW_STATE_HEAP_TYPE    stateHeapType,
        PMHW_KERNEL_STATE      kernelState);

    virtual MOS_STATUS DumpYUVSurface(
        PMOS_SURFACE           surface,
        const char *           attrName,
        const char *           surfName,
        MEDIA_DEBUG_STATE_TYPE mediaState = CODECHAL_NUM_MEDIA_STATES,
        uint32_t               width_in   = 0,
        uint32_t               height_in  = 0);

    MOS_STATUS DumpBuffer(
        PMOS_RESOURCE          resource,
        const char *           attrName,
        const char *           bufferName,
        uint32_t               size,
        uint32_t               offset = 0,
        MEDIA_DEBUG_STATE_TYPE mediaState = CODECHAL_NUM_MEDIA_STATES);

    MOS_STATUS DumpSurface(
        PMOS_SURFACE           surface,
        const char *           attrName,
        const char *           surfaceName,
        MEDIA_DEBUG_STATE_TYPE mediaState = CODECHAL_NUM_MEDIA_STATES);

    MOS_STATUS DumpData(
        void       *data,
        uint32_t   size,
        const char *attrName,
        const char *bufferName);

    MOS_STATUS DumpBufferInHexDwords(
        uint8_t *data,
        uint32_t size);

    virtual MOS_STATUS DumpBltOutput(
        PMOS_SURFACE surface,
        const char * attrName);

    MOS_STATUS DeleteCfgLinkNode(uint32_t frameIdx);

    std::string m_ddiFileName;
    std::string m_outputFilePath;

    MOS_SURFACE       m_temp2DSurfForCopy = {};
    PMOS_INTERFACE    m_osInterface       = nullptr;
    MhwCpInterface   *m_cpInterface       = nullptr;
    MhwMiInterface   *m_miInterface       = nullptr;
    MediaDbgFunction  m_mediafunction     = MEDIA_FUNCTION_DEFAULT;
    CODEC_PICTURE     m_currPic;
    uint32_t          m_scaledBottomFieldOffset = 0;
    uint16_t          m_frameType               = 0;
    uint32_t          m_sliceId                 = 0;  // used for constructing debug file name
    char              m_fileName[MOS_MAX_PATH_LENGTH + 1];
    char              m_path[MOS_MAX_PATH_LENGTH + 1];
    bool              m_secondField              = false;
    bool              m_hybridPakP1              = false;
    bool              m_hybridVp8EncodeBrcEnable = false;
    bool              m_hybridVp9EncodeEnable    = false;
    bool              m_vdboxContextCreated      = false;
    uint16_t          m_preIndex                 = 0;
    uint16_t          m_refIndex                 = 0;
    uint32_t          m_bufferDumpFrameNum       = 0;
    uint32_t          m_decodeSurfDumpFrameNum   = 0;
    uint32_t          m_streamId                 = 0;

protected:
    MOS_STATUS ReAllocateSurface(
        PMOS_SURFACE    pSurface,
        PMOS_SURFACE    pSrcSurf,
        PCCHAR          pSurfaceName,
        MOS_GFXRES_TYPE DefaultResType);

    MOS_STATUS CopySurfaceData_Vdbox(
        uint32_t      dwDataSize,
        PMOS_RESOURCE presSourceSurface,
        PMOS_RESOURCE presCopiedSurface);

    MOS_STATUS DumpNotSwizzled(
        std::string  surfName,
        MOS_SURFACE &surf,
        uint8_t *    lockedAddr,
        int32_t      size);

    MOS_STATUS DumpBufferInBinary(
        uint8_t *data,
        uint32_t size);

    MOS_STATUS Dump2DBufferInBinary(
        uint8_t *data,
        uint32_t width,
        uint32_t height,
        uint32_t pitch);

    virtual MOS_USER_FEATURE_VALUE_ID SetOutputPathKey()  = 0;
    virtual MOS_USER_FEATURE_VALUE_ID InitDefaultOutput() = 0;

    std::string          m_outputFileName;
    MediaDebugConfigMgr *m_configMgr = nullptr;
};

#else
#define USE_MEDIA_DEBUG_TOOL 0
#define MEDIA_DEBUG_TOOL(expr) ;

#endif  // (_DEBUG || _RELEASE_INTERNAL)
#endif /* __MEDIA_DEBUG_INTERFACE_H__ */