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
//! \file     media_debug_utils.h
//! \brief    Defines the debug interface utilities shared by all of Media.
//! \details  The debug interface dumps output from Media based on in input config file.
//!
#ifndef __MEDIA_DEBUG_UTILS_H__
#define __MEDIA_DEBUG_UTILS_H__

#if USE_MEDIA_DEBUG_TOOL
#include <string>
#include <map>
#include <vector>
#include "mos_defs.h"
#include "media_class_trace.h"

namespace MediaDbgAttr
{
//Common Attr
static const char *attrDumpBufferInBinary    = "DumpBufferInBinary";
static const char *attrDumpToThreadFolder    = "DumpToThreadFolder";
static const char *attrDumpCmdBufInBinary    = "DumpCmdBufInBinary";
static const char *attrEnableFastDump        = "EnableFastDump";

//Codec Attr 
static const char *attrPicParams               = "PicParams";
static const char *attrSubsetsParams           = "SubsetsParams";
static const char *attrFeiPicParams            = "FeiPicParams";
static const char *attrMvcExtPicParams         = "MvcExtPicParams";
static const char *attrSegmentParams           = "SegmentParams";
static const char *attrSlcParams               = "SlcParams";
static const char *attrMbParams                = "MbParams";
static const char *attrIqParams                = "IqParams";
static const char *attrSeqParams               = "SeqParams";
static const char *attrVuiParams               = "VuiParams";
static const char *attrDecodeBitstream         = "DecodeBitstream";             // Decode Bitstream
static const char *attrBitstream               = "Bitstream";                   // Encode Bitstream
static const char *attrStreamOut               = "StreamOut";
static const char *attrStreamIn                = "StreamIn";
static const char *attrResidualDifference      = "ResidualDifference";
static const char *attrDeblocking              = "Deblocking";
static const char *attrMvData                  = "MvData";
static const char *attrVc1Bitplane             = "Vc1Bitplane";
static const char *attrCoefProb                = "CoefProb";
static const char *attrSegId                   = "SegId";
static const char *attrHucRegions              = "HucRegions";
static const char *attrHuCDmem                 = "HucDmem";
static const char *attrCmdBufferMfx            = "CmdBufferMfx";
static const char *attr2ndLvlBatchMfx          = "2ndLvlBatchMfx";
static const char *attrSurfaceInfo             = "SurfaceInfo";
static const char *attrDelayForDumpOutput      = "DelayForDumpOutput";
static const char *attrDecodeOutputSurface     = "DecodeOutputSurface";
static const char *attrDecodeCompSurface       = "DumpCompressedSurface";
static const char *attrDecodeAuxSurface        = "DumpAuxsurface";
static const char *attrDecodeBltOutput         = "DumpBltOutput";
static const char *attrSfcOutputSurface        = "SfcOutputSurface";
static const char *attrSfcBuffers              = "SfcBuffers";
static const char *attrDecodeReferenceSurfaces = "DecodeReferenceSurfaces";     // Decode Reference Surfaces
static const char *attrReferenceSurfaces       = "ReferenceSurfaces";           // Encode Reference Surfaces
static const char *attrEncodeRawInputSurface   = "EncodeRawInputSurface";
static const char *attrReconstructedSurface    = "ReconstructedSurface";
static const char *attrPakInput                = "PakInput";
static const char *attrPakOutput               = "PakOutput";
static const char *attrUpsamlingInput          = "UpsamplingInput";
static const char *attrResidualSurface         = "ResidualSurface";
static const char *attrStCoeff                 = "StCoeff";
static const char *attrCoeffPredCs             = "CoeffPredCs";
static const char *attrMbRecord                = "MbRecord";
static const char *attrOverwriteCommands       = "OverwriteCommands";
static const char *attrHuffmanTbl              = "HuffmanTbl";
static const char *attrScanParams              = "ScanParams";
static const char *attrDriverUltDump           = "DriverUltDump";

static const char *attrDumpEncodePar          = "DumpEncodePar";
static const char *attrForceCmdDumpLvl        = "ForceCmdDumpLvl";
static const char *attrForceCurbeDumpLvl      = "ForceCurbeDumpLvl";
static const char *attrForceYUVDumpWithMemcpy = "ForceYUVDumpWithMemcpy";
static const char *attrDisableSwizzleForDumps = "DisableSwizzleForDumps";
static const char *attrVdencOutput            = "VdencOutput";
static const char *attrDecodeProcParams       = "DecodeProcParams";
static const char *attrFrameState             = "FrameState";
static const char *attrCUStreamout            = "CUStreamout";
static const char *attrBrcPakStats            = "BrcPakStats";
static const char *attrImageState             = "ImageState";
static const char *attrSliceSizeStreamout     = "SliceSizeStreamout";
static const char *attrCoeffProb              = "PakHwCoeffProbs";
static const char *attrStatusReport           = "StatusReport";
static const char *attrPakObjStreamout        = "PakObjStreamOut";
static const char *attrTileBasedStats         = "TileBasedStats";
static const char *attrROISurface             = "ROIInput";
static const char *attrHuCStitchDataBuf       = "HuCStitchDataBuffer";
static const char *attrInitSWScoreboard       = "InitSWScoreboard";
static const char *attrSfcHistogram           = "SfcHistogram";
static const char *attrFilmGrain              = "FilmGrain";
static const char *attrQualityReport          = "QualityReport";

static const char *attrAlfData                = "AlfData";
static const char *attrLmcsData               = "LmcsData";
static const char *attrTileParams             = "TileParams";
static const char *attrSubpicParams           = "SubpicParams";
static const char *attrSliceStruct            = "SliceStruct";
static const char *attrRplStruct              = "RplStruct";

// MD5 attributes
static const char *attrMD5HashEnable    = "MD5HasEnable";
static const char *attrMD5FlushInterval = "MD5FlushInterval";
static const char *attrMD5PicWidth      = "MD5PicWidth";
static const char *attrMD5PicHeight     = "MD5PicHeight";
// kernel attributes
static const char *attrDsh         = "DSH";
static const char *attrIsh         = "ISH";
static const char *attrSsh         = "SSH";
static const char *attrCurbe       = "Curbe";
static const char *attrCmdBuffer   = "CmdBuffer";
static const char *attr2ndLvlBatch = "2ndLvlBath";
static const char *attrInput       = "Input";
static const char *attrOutput      = "Output";
};

namespace MediaDbgFieldType
{
static const char *topField = "_Top";
static const char *botField = "_Bot";
static const char *frame    = "_Frame";
};  // namespace MediaDbgFieldType

namespace MediaDbgExtType
{
static const char *dat  = ".dat";
static const char *txt  = ".txt";
static const char *par  = ".par";
static const char *y    = ".Y";
static const char *uv   = ".UV";
static const char *yuv  = ".yuv";
static const char *yuy2 = ".yuy2";
static const char *uyvy = ".uyvy";
};  // namespace MediaDbgExtType

namespace MediaDbgSurfaceType
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

namespace MediaDbgBufferType
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
static const char *bufAlfData       = "AlfData";
static const char *bufLmcsData      = "LmcsData";
static const char *bufTileParams    = "TileParams";
static const char *bufSubpicParams  = "SubpicParams";
static const char *bufSliceStruct   = "SliceStruct";
static const char *bufRplStruct     = "RplStruct";
};  // namespace MediaDbgBufferType

//------------------------------------------------------------------------------
// Macros specific to MOS_OS_SUBCOMP_DEBUG sub-comp
//------------------------------------------------------------------------------
#define MEDIA_DEBUG_ASSERT(_expr) \
    MOS_ASSERT(MOS_COMPONENT_OS, MOS_SUBCOMP_SELF, _expr)

#define MEDIA_DEBUG_ASSERTMESSAGE(_message, ...) \
    MOS_ASSERTMESSAGE(MOS_COMPONENT_OS, MOS_SUBCOMP_SELF, _message, ##__VA_ARGS__)

#define MEDIA_DEBUG_NORMALMESSAGE(_message, ...) \
    MOS_NORMALMESSAGE(MOS_COMPONENT_OS, MOS_SUBCOMP_SELF, _message, ##__VA_ARGS__)

#define MEDIA_DEBUG_VERBOSEMESSAGE(_message, ...) \
    MOS_VERBOSEMESSAGE(MOS_COMPONENT_OS, MOS_SUBCOMP_SELF, _message, ##__VA_ARGS__)

#define MEDIA_DEBUG_FUNCTION_ENTER \
    MOS_FUNCTION_ENTER(MOS_COMPONENT_OS, MOS_SUBCOMP_SELF)

#define MEDIA_DEBUG_CHK_STATUS(_stmt) \
    MOS_CHK_STATUS_RETURN(MOS_COMPONENT_OS, MOS_SUBCOMP_SELF, _stmt)

#define MEDIA_DEBUG_CHK_STATUS_MESSAGE(_stmt, _message, ...) \
    MOS_CHK_STATUS_MESSAGE_RETURN(MOS_COMPONENT_OS, MOS_SUBCOMP_SELF, _stmt, _message, ##__VA_ARGS__)

#define MEDIA_DEBUG_CHK_NULL(_ptr) \
    MOS_CHK_NULL_RETURN(MOS_COMPONENT_OS, MOS_SUBCOMP_SELF, _ptr)

#define MEDIA_DEBUG_CHK_NULL_NO_STATUS(_ptr) \
    MOS_CHK_NULL_NO_STATUS_RETURN(MOS_COMPONENT_OS, MOS_SUBCOMP_SELF, _ptr)

#define MEDIA_DEBUG_CODECHAL_DUMP_OUTPUT_FOLDER "\\codechaldump\\"
#define MEDIA_DEBUG_VPHAL_DUMP_OUTPUT_FOLDER "\\vphaldump\\"

#endif  //USE_MEDIA_DEBUG_TOOL
#endif  /* __MEDIA_DEBUG_UTILS_H__ */
