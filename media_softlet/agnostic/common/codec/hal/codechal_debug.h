/*
* Copyright (c) 2011-2023, Intel Corporation
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
//! \brief    Defines the debug interface shared by codec only.
//! \details  The debug interface dumps output from Media based on in input config file.
//!
#ifndef __CODEC_DEBUG_H__
#define __CODEC_DEBUG_H__

#include "media_debug_interface.h"
#include "codec_hw_next.h"
#include "encode_status_report.h"
#include "codechal_event_debug.h"
#include "codec_def_decode_avc.h"
#include "codec_def_decode_av1.h"
#include "codec_def_decode_jpeg.h"
#include "codec_def_decode_mpeg2.h"
#include "codec_def_decode_vp8.h"

static uint32_t DecodeFrameIndex = 0;
static uint32_t DecodeOutputIndex = 0;

#if USE_MEDIA_DEBUG_TOOL

#define USE_CODECHAL_DEBUG_TOOL 1
#define CODECHAL_DEBUG_TOOL(expr) expr;

typedef struct _CODECHAL_DBG_CFG CODECHAL_DBG_CFG, *PCODECHAL_DBG_CFG;

namespace CodechalDbgFieldType   = MediaDbgFieldType;
namespace CodechalDbgExtType     = MediaDbgExtType;
namespace CodechalDbgSurfaceType = MediaDbgSurfaceType;
namespace CodechalDbgBufferType  = MediaDbgBufferType;
namespace CodechalDbgAttr        = MediaDbgAttr;

#define __MEDIA_USER_FEATURE_VALUE_DECODE_SFC_RGBFORMAT_OUTPUT_DEBUG "Decode SFC RGB Format Output"
#define __MEDIA_USER_FEATURE_VALUE_DECODE_SFC_LINEAR_OUTPUT_DEBUG "Decode SFC Linear Output Debug"
#define __MEDIA_USER_FEATURE_ENABLE_HW_DEBUG_HOOKS_DEBUG "Enable Media Debug Hooks"
#define __MEDIA_USER_FEATURE_VALUE_CODECHAL_FRAME_NUMBER_TO_STOP_DEBUG "Decode Stop To Frame"
#define __MEDIA_USER_FEATURE_VALUE_CODECHAL_ENABLE_SW_CRC_DEBUG "Enable SW CRC"
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

enum CodechalHucRegionDumpType
{
    hucRegionDumpDefault        = 0,
    hucRegionDumpInit           = 1,
    hucRegionDumpUpdate         = 2,
    hucRegionDumpLAUpdate       = 3,
    hucRegionDumpRegionLocked   = 4,
    hucRegionDumpCmdInitializer = 5,
    hucRegionDumpPakIntegrate   = 6,
    hucRegionDumpHpu            = 7,
    hucRegionDumpBackAnnotation = 8,
    hucRegionDumpHpuSuperFrame  = 9
};

typedef struct _CODECHAL_ME_OUTPUT_PARAMS
{
    PMOS_SURFACE  psMeMvBuffer;
    PMOS_SURFACE  psMeBrcDistortionBuffer;
    PMOS_SURFACE  psMeDistortionBuffer;
    PMOS_RESOURCE pResVdenStreamInBuffer;
    bool          b16xMeInUse;
    bool          b32xMeInUse;
    bool          bVdencStreamInInUse;
} CODECHAL_ME_OUTPUT_PARAMS, *PCODECHAL_ME_OUTPUT_PARAMS;

std::ostream &operator<<(std::ostream &oss, const CODEC_AVC_PIC_PARAMS &cr);
std::ostream &operator<<(std::ostream &oss, const CODEC_AVC_SF_SLICE_PARAMS &cr);
std::ostream &operator<<(std::ostream &oss, const CODEC_AVC_SLICE_PARAMS &cr);
std::ostream &operator<<(std::ostream &oss, const CODEC_AVC_IQ_MATRIX_PARAMS &cr);
std::ostream &operator<<(std::ostream &oss, const CODEC_HEVC_PIC_PARAMS &cr);
std::ostream &operator<<(std::ostream &oss, const CODEC_HEVC_EXT_PIC_PARAMS &cr);
std::ostream &operator<<(std::ostream &oss, const CODEC_HEVC_SCC_PIC_PARAMS &cr);
std::ostream &operator<<(std::ostream &oss, const CODEC_HEVC_SF_SLICE_PARAMS &cr);
std::ostream &operator<<(std::ostream &oss, const CODEC_HEVC_SLICE_PARAMS &cr);
std::ostream &operator<<(std::ostream &oss, const CODEC_HEVC_EXT_SLICE_PARAMS &cr);
std::ostream &operator<<(std::ostream &oss, const CODECHAL_HEVC_IQ_MATRIX_PARAMS &cr);
std::ostream &operator<<(std::ostream &oss, const CODEC_HEVC_SUBSET_PARAMS &cr);
std::ostream &operator<<(std::ostream &oss, const CODEC_VP9_PIC_PARAMS &cr);
std::ostream &operator<<(std::ostream &oss, const CODEC_VP9_SLICE_PARAMS &cr);
std::ostream &operator<<(std::ostream &oss, const CODEC_VP9_SEGMENT_PARAMS &cr);
std::ostream &operator<<(std::ostream &oss, const CodecAv1PicParams &cr);
std::ostream &operator<<(std::ostream &oss, const CodecAv1TileParams &cr);
std::ostream &operator<<(std::ostream &oss, const CodecDecodeJpegPicParams &cr);
std::ostream &operator<<(std::ostream &oss, const CodecDecodeJpegScanParameter &cr);
std::ostream &operator<<(std::ostream &oss, const CODECHAL_DECODE_JPEG_HUFFMAN_TABLE &cr);
std::ostream &operator<<(std::ostream &oss, const CodecJpegQuantMatrix &cr);
std::ostream &operator<<(std::ostream &oss, const CodecDecodeMpeg2PicParams &cr);
std::ostream &operator<<(std::ostream &oss, const CodecDecodeMpeg2SliceParams &cr);
std::ostream &operator<<(std::ostream &oss, const CodecDecodeMpeg2MbParams &cr);
std::ostream &operator<<(std::ostream &oss, const CodecMpeg2IqMatrix &cr);
std::ostream &operator<<(std::ostream &oss, const CODEC_VP8_PIC_PARAMS &cr);
std::ostream &operator<<(std::ostream &oss, const CODEC_VP8_SLICE_PARAMS &cr);
std::ostream &operator<<(std::ostream &oss, const CODEC_VP8_IQ_MATRIX_PARAMS &cr);

void DumpDecodeAvcPicParams(PCODEC_AVC_PIC_PARAMS picParams, std::string fileName);
void DumpDecodeAvcSliceParams(PCODEC_AVC_SLICE_PARAMS sliceParams, uint32_t numSlices, std::string fileName, bool shortFormatInUse);
void DumpDecodeAvcIQParams(PCODEC_AVC_IQ_MATRIX_PARAMS iqParams, std::string fileName);
void DumpDecodeHevcPicParams(PCODEC_HEVC_PIC_PARAMS picParams, std::string fileName);
void DumpDecodeHevcExtPicParams(PCODEC_HEVC_EXT_PIC_PARAMS extPicParams, std::string fileName);
void DumpDecodeHevcSccPicParams(PCODEC_HEVC_SCC_PIC_PARAMS sccPicParams, std::string fileName);
void DumpDecodeHevcSliceParams(PCODEC_HEVC_SLICE_PARAMS sliceParams, uint32_t numSlices, std::string fileName, bool shortFormatInUse);
void DumpDecodeHevcExtSliceParams(PCODEC_HEVC_EXT_SLICE_PARAMS extSliceParams, uint32_t numSlices, std::string fileName);
void DumpDecodeHevcIQParams(PCODECHAL_HEVC_IQ_MATRIX_PARAMS iqParams, std::string fileName);
void DumpDecodeHevcSubsetParams(PCODEC_HEVC_SUBSET_PARAMS subsetsParams, std::string fileName);
void DumpDecodeVp9PicParams(PCODEC_VP9_PIC_PARAMS picParams, std::string fileName);
void DumpDecodeVp9SliceParams(PCODEC_VP9_SLICE_PARAMS slcParams, std::string fileName);
void DumpDecodeVp9SegmentParams(PCODEC_VP9_SEGMENT_PARAMS segmentParams, std::string fileName);
void DumpDecodeAv1PicParams(CodecAv1PicParams *picParams, std::string fileName);
void DumpDecodeAv1TileParams(CodecAv1TileParams *tileParams, uint32_t tileNum, std::string fileName);
void DumpDecodeJpegPicParams(CodecDecodeJpegPicParams *picParams, std::string fileName);
void DumpDecodeJpegScanParams(CodecDecodeJpegScanParameter *scanParams, std::string fileName);
void DumpDecodeJpegHuffmanParams(PCODECHAL_DECODE_JPEG_HUFFMAN_TABLE huffmanTable, std::string fileName);
void DumpDecodeJpegIqParams(CodecJpegQuantMatrix *iqParams, std::string fileName);
void DumpDecodeMpeg2PicParams(CodecDecodeMpeg2PicParams *picParams, std::string fileName);
void DumpDecodeMpeg2SliceParams(CodecDecodeMpeg2SliceParams *sliceParams, uint32_t numSlices, std::string fileName);
void DumpDecodeMpeg2MbParams(CodecDecodeMpeg2MbParams *mbParams, uint32_t numMbs, std::string fileName);
void DumpDecodeMpeg2IqParams(CodecMpeg2IqMatrix *iqParams, std::string fileName);
void DumpDecodeVp8PicParams(PCODEC_VP8_PIC_PARAMS picParams, std::string fileName);
void DumpDecodeVp8SliceParams(PCODEC_VP8_SLICE_PARAMS sliceParams, std::string fileName);
void DumpDecodeVp8IqParams(PCODEC_VP8_IQ_MATRIX_PARAMS iqParams, std::string fileName);

class MediaDebugInterface;
class CodechalHwInterface;
class CodechalDebugInterface : public MediaDebugInterface
{
public:
    CodechalDebugInterface();
    virtual ~CodechalDebugInterface();

    static CodechalDebugInterface* Create();

    virtual MOS_STATUS Initialize(
        CodechalHwInterface *hwInterface,
        CODECHAL_FUNCTION    codecFunction,
        MediaCopyWrapper    *mediaCopyWrapper = nullptr);

    virtual MOS_STATUS Initialize(
        CodechalHwInterfaceNext *hwInterface,
        CODECHAL_FUNCTION        codecFunction,
        MediaCopyWrapper        *mediaCopyWrapper = nullptr);

    virtual MOS_STATUS DumpHucDmem(
        PMOS_RESOURCE             dmemResource,
        uint32_t                  dmemSize,
        uint32_t                  hucPassNum,
        CodechalHucRegionDumpType dumpType);

    virtual MOS_STATUS DumpHucRegion(
        PMOS_RESOURCE             region,
        uint32_t                  regionOffset,
        uint32_t                  regionSize,
        uint32_t                  regionNum,
        const char *              regionName,
        bool                      inputBuffer,
        uint32_t                  hucPassNum,
        CodechalHucRegionDumpType dumpType);

    MOS_STATUS DetectCorruptionSw(std::vector<MOS_RESOURCE>& vResource, PMOS_RESOURCE frameCntRes, uint8_t *buf, uint32_t &size, uint32_t frameNum);

    MOS_STATUS DetectCorruptionHw(void *hwInterface, PMOS_RESOURCE frameCntRes, uint32_t curIdx, uint32_t frameCrcOffset, std::vector<MOS_RESOURCE> &vStatusBuffer, PMOS_COMMAND_BUFFER pCmdBuffer, uint32_t frameNum);
    
    MOS_STATUS StoreNumFrame(PMHW_MI_INTERFACE pMiInterface, PMOS_RESOURCE pResource, int32_t frameNum, PMOS_COMMAND_BUFFER pCmdBuffer);

    MOS_STATUS DumpEncodeStatusReport(
        const struct EncodeStatusReport *report);

    MOS_STATUS DumpEncodeStatusReport(
        const struct encode::EncodeStatusReportData *report);

    void CheckGoldenReferenceExist();

    MOS_STATUS DumpRgbDataOnYUVSurface(
        PMOS_SURFACE               surface,
        const char                *attrName,
        const char                *surfName,
        CODECHAL_MEDIA_STATE_TYPE  mediaState = CODECHAL_NUM_MEDIA_STATES,
        uint32_t                   width_in   = 0,
        uint32_t                   height_in  = 0);

    MOS_STATUS DumpBltOutput(
        PMOS_SURFACE surface,
        const char  *attrName) override;

    virtual MOS_STATUS InitializeUserSetting() override;

    bool DumpIsEnabled(
        const char *              attr,
        CODECHAL_MEDIA_STATE_TYPE mediaState = CODECHAL_NUM_MEDIA_STATES);

    MOS_STATUS SetFastDumpConfig(MediaCopyWrapper *mediaCopyWrapper);

    const char *CreateFileName(
        const char *funcName,
        const char *bufType,
        const char *extType);

    MOS_STATUS DumpStringStream(
        std::stringstream& ss,
        const char*        bufferName,
        const char*        attrName);

    virtual MOS_STATUS DumpCmdBuffer(
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

    MOS_STATUS DumpYUVSurfaceToBuffer(PMOS_SURFACE surface, uint8_t *buffer, uint32_t &size);

    MOS_STATUS DumpYUVSurface(
        PMOS_SURFACE              surface,
        const char *              attrName,
        const char *              surfName,
        CODECHAL_MEDIA_STATE_TYPE mediaState = CODECHAL_NUM_MEDIA_STATES,
        uint32_t                  width_in   = 0,
        uint32_t                  height_in  = 0);

    virtual MOS_STATUS DumpBuffer(
        PMOS_RESOURCE             resource,
        const char *              attrName,
        const char *              bufferName,
        uint32_t                  size,
        uint32_t                  offset = 0,
        CODECHAL_MEDIA_STATE_TYPE mediaState = CODECHAL_NUM_MEDIA_STATES);

    MOS_STATUS DumpSurface(
        PMOS_SURFACE              surface,
        const char *              attrName,
        const char *              surfaceName,
        CODECHAL_MEDIA_STATE_TYPE mediaState = CODECHAL_NUM_MEDIA_STATES);

    MOS_STATUS DumpData(
        void       *data,
        uint32_t   size,
        const char *attrName,
        const char *bufferName);

    MOS_STATUS DumpSurfaceInfo(
        PMOS_SURFACE surface,
        const char*  surfaceName);

    CodechalHwInterface     *m_hwInterface      = nullptr;
    CodechalHwInterfaceNext *m_hwInterfaceNext  = nullptr;
    CODECHAL_FUNCTION        m_codecFunction    = CODECHAL_FUNCTION_INVALID;
    PCODECHAL_DBG_CFG        m_dbgCfgHead       = nullptr;
    CODEC_PICTURE            m_currPic          = {};

protected:
    std::string SetOutputPathKey() override;
    std::string InitDefaultOutput() override;
    uint8_t *m_decodeOutputBuf = nullptr;
    PLATFORM    m_platform        = {};

    std::function<
        MOS_STATUS(
            PMOS_SURFACE              surface,
            const char               *attrName,
            const char               *surfName,
            CODECHAL_MEDIA_STATE_TYPE mediaState,
            uint32_t                  width_in,
            uint32_t                  height_in)>
        m_dumpYUVSurface;

    std::function<
        MOS_STATUS(
            PMOS_SURFACE              surface,
            const char               *attrName,
            const char               *surfName,
            CODECHAL_MEDIA_STATE_TYPE mediaState,
            uint32_t                  width_in,
            uint32_t                  height_in)>
        m_dumpYUVSurfaceLegacy;

    std::function<
        MOS_STATUS(
            PMOS_RESOURCE             resource,
            const char               *attrName,
            const char               *bufferName,
            uint32_t                  size,
            uint32_t                  offset,
            CODECHAL_MEDIA_STATE_TYPE mediaState)>
        m_dumpBuffer;

    std::function<
        MOS_STATUS(
            PMOS_RESOURCE             resource,
            const char               *attrName,
            const char               *bufferName,
            uint32_t                  size,
            uint32_t                  offset,
            CODECHAL_MEDIA_STATE_TYPE mediaState)>
        m_dumpBufferLegacy;

private:
    MOS_STATUS VDBypassCopyResource(PMOS_RESOURCE src, PMOS_RESOURCE dst);
    MOS_STATUS AllocateLinearResource(
        MOS_GFXRES_TYPE eType,
        uint32_t        dwWidth,
        uint32_t        dwHeight,
        MOS_FORMAT      eFormat,
        PMOS_RESOURCE   pResource,
        bool            bSystemMem = false,
        bool            bLockable = false) const;
    MOS_STATUS FillResourceMemory(
        MOS_RESOURCE &sResource,
        uint32_t      uiSize,
        uint8_t       ucValue);

MEDIA_CLASS_DEFINE_END(CodechalDebugInterface)
};
#else
#define USE_CODECHAL_DEBUG_TOOL 0
#define CODECHAL_DEBUG_TOOL(expr) ;

#endif  // USE_MEDIA_DEBUG_TOOL
#endif  /* __MEDIA_DEBUG_H__ */
