/*
* Copyright (c) 2011-2018, Intel Corporation
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
//! \file     codechal_debug.h
//! \brief    Defines the debug interface shared by all of CodecHal.
//! \details  The debug interface dumps output from CodecHal based on in input config file.
//!
#ifndef __CODECHAL_DEBUG_H__
#define __CODECHAL_DEBUG_H__
#if (_DEBUG || _RELEASE_INTERNAL)

#define USE_CODECHAL_DEBUG_TOOL 1
#define CODECHAL_DEBUG_TOOL(expr)   expr;

#include "BRCIF.h"
#include "mhw_utilities.h"
#include "mhw_state_heap.h"
#include "codec_def_common.h"
#include "codechal_debug_config_manager.h"
#include <sstream>
#include <fstream>

#define CODECHAL_DEBUG_TOOL(expr)   expr;

//------------------------------------------------------------------------------
// Macros specific to MOS_CODEC_SUBCOMP_DEBUG sub-comp
//------------------------------------------------------------------------------
#define CODECHAL_DEBUG_ASSERT(_expr) \
    MOS_ASSERT(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_DEBUG, _expr)

#define CODECHAL_DEBUG_ASSERTMESSAGE(_message, ...) \
    MOS_ASSERTMESSAGE(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_DEBUG, _message, ##__VA_ARGS__)

#define CODECHAL_DEBUG_NORMALMESSAGE(_message, ...) \
    MOS_NORMALMESSAGE(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_DEBUG, _message, ##__VA_ARGS__)

#define CODECHAL_DEBUG_VERBOSEMESSAGE(_message, ...) \
    MOS_VERBOSEMESSAGE(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_DEBUG, _message, ##__VA_ARGS__)

#define CODECHAL_DEBUG_FUNCTION_ENTER \
    MOS_FUNCTION_ENTER(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_DEBUG)

#define CODECHAL_DEBUG_CHK_STATUS(_stmt) \
    MOS_CHK_STATUS_RETURN(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_DEBUG, _stmt)

#define CODECHAL_DEBUG_CHK_STATUS_MESSAGE(_stmt, _message, ...) \
    MOS_CHK_STATUS_MESSAGE_RETURN(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_DEBUG, _stmt, _message, ##__VA_ARGS__)

#define CODECHAL_DEBUG_CHK_NULL(_ptr) \
    MOS_CHK_NULL_RETURN(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_DEBUG, _ptr)

#define CODECHAL_DEBUG_CHK_NULL_NO_STATUS(_ptr) \
    MOS_CHK_NULL_NO_STATUS_RETURN(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_DEBUG, _ptr)

namespace CodechalDbgFieldType
{
static const char *topField = "_Top";
static const char *botField = "_Bot";
static const char *frame    = "_Frame";
};

namespace CodechalDbgExtType
{
static const char *dat  = ".dat";
static const char *txt  = ".txt";
static const char *par  = ".par";
static const char *y    = ".Y";
static const char *uv   = ".UV";
static const char *yuv  = ".yuv";
static const char *yuy2 = ".yuy2";
static const char *uyvy = ".uyvy";
};

namespace CodechalDbgSurfaceType
{
static const char *yuv444    = "_yuv444";
static const char *yuv411    = "_yuv411";
static const char *yuv420    = "_yuv420";
static const char *yuv400    = "_yuv400";
static const char *yuv422h2y = "_yuv422h_2y";
static const char *yuv422v2y = "_yuv422v_2y";
static const char *yuv422h4y = "_yuv422h_4y";
static const char *yuv422v4y = "_yuv422v_4y";
static const char *yuy2422   = "_yuy2422";
static const char *uyvy422   = "_uyvy422";
};

namespace CodechalDbgBufferType
{
static const char *bufCmd           = "CmdBuf";
static const char *buf2ndLvl        = "2ndLvl";
static const char *bufCurbe         = "Curbe";
static const char *bufISH           = "ISH";
static const char *bufDSH           = "DSH";
static const char *bufSSH           = "SSH";
static const char *bufSeqParams     = "SeqParams";
static const char *bufPicParams     = "PicParams";
static const char *bufSlcParams     = "SlcParams";
static const char *bufSubsetsParams = "SubsetsParams";
static const char *bufVuiParams     = "VuiParams";
static const char *bufBitstream     = "Bitstream";
static const char *bufFeiPicParams  = "FeiPicParams";
static const char *bufMadRead       = "MADRead";
static const char *bufMadWrite      = "MADWrite";
static const char *bufSegmentParams = "SegmentParams";
static const char *bufIqParams      = "IqParams";
static const char *bufHuffmanTbl    = "HuffmanTbl";
static const char *bufScanParams    = "ScanParams";
static const char *bufMvcPicParams  = "MvcPicParams";
static const char *bufMbParams      = "MbParams";
static const char *bufDecProcParams = "DecProcParams";
static const char *bufHucRegion     = "HucRegion";
static const char *bufHucDmem       = "HucDmem";
static const char *bufEncodePar     = "EncodePar";
};

namespace CodechalDbgAttr
{
static const char *attrInitSWScoreboard = "InitSWScoreboard";
static const char *attrSfcHistogram = "SfcHistogram";
};

enum CodechalHucRegionDumpType
{
    hucRegionDumpDefault        = 0,
    hucRegionDumpInit           = 1,
    hucRegionDumpUpdate         = 2,
    hucRegionDumpRegionLocked   = 3,
    hucRegionDumpCmdInitializer = 4,
    hucRegionDumpPakIntegrate   = 5,
    hucRegionDumpHpu            = 6,
    hucRegionDumpBackAnnotation = 7
};

struct _CODECHAL_DEBUG_INTERFACE;
typedef struct _CODECHAL_DBG_CFG    CODECHAL_DBG_CFG, *PCODECHAL_DBG_CFG;

typedef struct _CODECHAL_ME_OUTPUT_PARAMS
{
    PMOS_SURFACE                    psMeMvBuffer;
    PMOS_SURFACE                    psMeBrcDistortionBuffer;
    PMOS_SURFACE                    psMeDistortionBuffer;
    PMOS_RESOURCE                   pResVdenStreamInBuffer;
    bool                            b16xMeInUse;
    bool                            b32xMeInUse;
    bool                            bVdencStreamInInUse;
} CODECHAL_ME_OUTPUT_PARAMS, *PCODECHAL_ME_OUTPUT_PARAMS;

class CodechalDebugInterface
{
public:
    CodechalDebugInterface();
    virtual ~CodechalDebugInterface();

    MOS_STATUS Initialize(
        CodechalHwInterface *hwInterface,
        CODECHAL_FUNCTION      codecFunction);

    bool DumpIsEnabled(
        const char *              attr,
        CODECHAL_MEDIA_STATE_TYPE mediaState = CODECHAL_NUM_MEDIA_STATES);

    const char *CreateFileName(
        const char *funcName,
        const char *bufType,
        const char *extType);

    MOS_STATUS DumpCmdBuffer(
        PMOS_COMMAND_BUFFER       cmdBuffer,
        CODECHAL_MEDIA_STATE_TYPE mediaState,
        const char *              cmdName = nullptr);

    MOS_STATUS Dump2ndLvlBatch(
        PMHW_BATCH_BUFFER         batchBuffer,
        CODECHAL_MEDIA_STATE_TYPE mediaState,
        const char *              batchName = nullptr);

    MOS_STATUS DumpCurbe(
        CODECHAL_MEDIA_STATE_TYPE mediaState,
        PMHW_KERNEL_STATE         kernelState);

    MOS_STATUS DumpMDFCurbe(
        CODECHAL_MEDIA_STATE_TYPE mediaState,
        uint8_t *                 curbeBuffer,
        uint32_t                  curbeSize);

    MOS_STATUS DumpKernelRegion(
        CODECHAL_MEDIA_STATE_TYPE mediaState,
        MHW_STATE_HEAP_TYPE       stateHeapType,
        PMHW_KERNEL_STATE         kernelState);

    virtual MOS_STATUS DumpYUVSurface(
        PMOS_SURFACE              surface,
        const char *              attrName,
        const char *              surfName,
        CODECHAL_MEDIA_STATE_TYPE mediaState = CODECHAL_NUM_MEDIA_STATES,
        uint32_t                  width_in = 0,
        uint32_t                  height_in = 0);

    MOS_STATUS DumpBuffer(
        PMOS_RESOURCE             resource,
        const char *              attrName,
        const char *              bufferName,
        uint32_t                  size,
        uint32_t                  offset     = 0,
        CODECHAL_MEDIA_STATE_TYPE mediaState = CODECHAL_NUM_MEDIA_STATES);

    MOS_STATUS DumpSurface(
        PMOS_SURFACE              surface,
        const char *              attrName,
        const char *              surfaceName,
        CODECHAL_MEDIA_STATE_TYPE mediaState = CODECHAL_NUM_MEDIA_STATES);

    MOS_STATUS DumpData(
        void *      data,
        uint32_t    size,
        const char *attrName,
        const char *bufferName);

    MOS_STATUS DumpHucDmem(
        PMOS_RESOURCE             dmemResource,
        uint32_t                  dmemSize,
        uint32_t                  hucPassNum,
        CodechalHucRegionDumpType dumpType);

    MOS_STATUS DumpHucRegion(
        PMOS_RESOURCE             region,
        uint32_t                  regionOffset,
        uint32_t                  regionSize,
        uint32_t                  regionNum,
        const char *              regionName,
        bool                      inputBuffer,
        uint32_t                  hucPassNum,
        CodechalHucRegionDumpType dumpType);

    virtual MOS_STATUS DumpBltOutput(
        PMOS_SURFACE              surface,
        const char *              attrName);

    MOS_STATUS DeleteCfgLinkNode(uint32_t frameIdx);

    std::string             m_ddiFileName;
    std::string             m_outputFileName;

    CodechalHwInterface *m_hwInterface            = nullptr;
    PMOS_INTERFACE       m_osInterface            = nullptr;
    CODECHAL_FUNCTION    m_codecFunction          = CODECHAL_FUNCTION_INVALID;
    bool                 m_enableBinaryDebugDumps = false;
    bool                 m_enableEncodeDdiDump    = false;
    PCODECHAL_DBG_CFG    m_dbgCfgHead             = nullptr;
    CODEC_PICTURE        m_currPic;
    uint32_t             m_scaledBottomFieldOffset = 0;
    uint16_t             m_frameType               = 0;
    uint32_t             m_sliceId                 = 0;  // used for constructing debug file name
    char                 m_fileName[MOS_MAX_PATH_LENGTH + 1];
    char                 m_path[MOS_MAX_PATH_LENGTH + 1];
    bool                 m_secondField              = false;
    bool                 m_hybridPakP1              = false;
    bool                 m_hybridVp8EncodeBrcEnable = false;
    bool                 m_hybridVp9EncodeEnable    = false;
    uint16_t             m_preIndex                 = 0;
    uint16_t             m_refIndex                 = 0;
    uint32_t             m_bufferDumpFrameNum       = 0;
    uint32_t             m_decodeSurfDumpFrameNum   = 0;

    uint32_t             m_streamId = 0;

    MOS_STATUS DumpBufferInHexDwords(
        uint8_t *   data,
        uint32_t    size);

protected:
    MOS_STATUS DumpBufferInBinary(
        uint8_t *   data,
        uint32_t    size);

    MOS_STATUS Dump2DBufferInBinary(
        uint8_t *   data,
        uint32_t    width,
        uint32_t    height,
        uint32_t    pitch);

    CodechalDebugConfigMgr *m_configMgr = nullptr;
    std::string             m_outputFilePath;
};
#else
#define USE_CODECHAL_DEBUG_TOOL     0
#define CODECHAL_DEBUG_TOOL(expr)   ;
#endif  // (_DEBUG || _RELEASE_INTERNAL)

#endif  /* __CODECHAL_DEBUG_H__ */
