/*
* Copyright (c) 2020-2024, Intel Corporation
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
//! \file     mhw_vdbox_avp_cmdpar.h
//! \brief    MHW command parameters
//! \details
//!

#ifndef __MHW_VDBOX_AVP_CMDPAR_H__
#define __MHW_VDBOX_AVP_CMDPAR_H__

#include "codec_def_common_encode.h"
#include "codec_def_common_av1.h"
#include "codec_def_decode_av1.h"
#include "mhw_vdbox.h"
#include "mhw_vdbox_cmdpar.h"

#ifdef IGFX_AVP_INTERFACE_EXT_SUPPORT
#include "mhw_vdbox_avp_cmdpar_ext.h"
#define __MHW_VDBOX_AVP_WRAPPER(STUFF)
#define __MHW_VDBOX_AVP_WRAPPER_EXT(STUFF) STUFF
#else
#define __MHW_VDBOX_AVP_WRAPPER(STUFF) STUFF
#define __MHW_VDBOX_AVP_WRAPPER_EXT(STUFF)
#endif

namespace mhw
{
namespace vdbox
{
namespace avp
{

enum class SURFACE_FORMAT
{
    SURFACE_FORMAT_P010VARIANT = 3,
    SURFACE_FORMAT_PLANAR4208  = 4,
    SURFACE_FORMAT_P010        = 13,
};

enum AvpBufferType
{
    segmentIdBuffer        = 0,    //!< segment ID temporal buffers
    mvTemporalBuffer,              //!< MV temporal buffers of both current and collocated
    bsdLineBuffer,                 //!< bitstream decode line buffer
    bsdTileLineBuffer,             //!< bitstream decode tile line buffer
    intraPredLineBuffer,           //!< intra prediction line buffer
    intraPredTileLineBuffer,
    spatialMvLineBuffer,
    spatialMvTileLineBuffer,
    lrMetaTileColBuffer,           //!< Loop Restoration Meta Tile Column Read/Write Buffer Address
    lrTileLineYBuffer,             //!< Loop Restoration Filter Tile Read/Write Line Y Buffer Address
    lrTileLineUBuffer,             //!< Loop Restoration Filter Tile Read/Write Line U Buffer Address
    lrTileLineVBuffer,             //!< Loop Restoration Filter Tile Read/Write Line V Buffer Address
    deblockLineYBuffer,
    deblockLineUBuffer,
    deblockLineVBuffer,
    deblockTileLineYBuffer,
    deblockTileLineVBuffer,
    deblockTileLineUBuffer,
    deblockTileColYBuffer,
    deblockTileColUBuffer,
    deblockTileColVBuffer,
    cdefLineBuffer,
    cdefTileLineBuffer,
    cdefTileColBuffer,
    cdefMetaTileLineBuffer,
    cdefMetaTileColBuffer,
    cdefTopLeftCornerBuffer,
    superResTileColYBuffer,
    superResTileColUBuffer,
    superResTileColVBuffer,
    lrTileColYBuffer,
    lrTileColUBuffer,
    lrTileColVBuffer,
    frameStatusErrBuffer,
    dbdStreamoutBuffer,
    fgTileColBuffer,
    fgSampleTmpBuffer,
    lrTileColAlignBuffer,
    tileSzStreamOutBuffer,
    tileStatStreamOutBuffer,
    cuStreamoutBuffer,
    sseLineBuffer,
    sseTileLineBuffer,
    avpInternalBufferMax
};

struct AvpBufferSizePar
{
    uint8_t      bitDepthIdc;
    uint32_t     width;
    uint32_t     height;
    uint32_t     tileWidth;
    uint32_t     bufferSize;
    bool         isSb128x128;
    uint32_t     curFrameTileNum;
    uint32_t     numTileCol;
    uint8_t      numOfActivePipes;
    uint16_t     chromaFormat;
};

struct AvpVdboxRowStorePar
{
    uint32_t mode;
    uint32_t picWidth;
    uint32_t mbaff;
    bool     isFrame;
    uint8_t  bitDepthMinus8;
    uint8_t  chromaFormat;
    uint8_t  lcuSize;
};

struct _MHW_PAR_T(AVP_PIPE_MODE_SELECT)
{
    uint8_t                         codecSelect                       = 0;
    bool                            cdefOutputStreamoutEnableFlag     = false;
    bool                            lrOutputStreamoutEnableFlag       = false;
    bool                            picStatusErrorReportEnable        = false;
    uint8_t                         codecStandardSelect               = 0;
    MHW_VDBOX_HCP_MULTI_ENGINE_MODE multiEngineMode                   = MHW_VDBOX_HCP_MULTI_ENGINE_MODE_FE_LEGACY;
    MHW_VDBOX_HCP_PIPE_WORK_MODE    pipeWorkingMode                   = MHW_VDBOX_HCP_PIPE_WORK_MODE_LEGACY;
    bool                            tileBasedReplayMode               = false;
    bool                            picStatusErrorReportId            = false;
    uint8_t                         phaseIndicator                    = 0;
    bool                            frameReconDisable                 = false;
    bool                            vdencMode                         = false;
    bool                            tileStatsStreamoutEnable          = false;
    bool                            motionCompMemTrackerCounterEnable = false;
    bool                            pakFrameLevelStreamOutEnable      = false;
    bool                            motionCompMemoryTrackerCntEnable  = false;
    uint8_t                         srcPixelPrefetchLen               = 0;
    bool                            srcPixelPrefetchEnable            = false;
    bool                            sseEnable                         = false;
};

struct _MHW_PAR_T(AVP_PIC_STATE)
{
    uint32_t frameWidthMinus1                    = 0;
    uint32_t frameHeightMinus1                   = 0;

    uint8_t  frameType                           = 0;
    uint8_t  primaryRefFrame                     = 0;
    bool     applyFilmGrainFlag                  = 0;
    bool     deltaQPresentFlag                   = false;
    uint8_t  log2DeltaQRes                       = 0;
    bool     codedLossless                       = false;
    uint16_t baseQindex                          = 0;
    int8_t   yDcDeltaQ                           = 0;
    int8_t   uDcDeltaQ                           = 0;
    int8_t   uAcDeltaQ                           = 0;
    int8_t   vDcDeltaQ                           = 0;
    int8_t   vAcDeltaQ                           = 0;
    bool     allowHighPrecisionMV                = false;
    bool     referenceSelect                     = false;
    uint8_t  interpFilter                        = 0;
    uint16_t currentOrderHint                    = 0;
    bool     reducedTxSetUsed                    = false;
    uint8_t  txMode                              = 0;
    bool     skipModePresent                     = false;
    uint8_t  globalMotionType[7]                 = {};
    uint8_t  refFrameIdx[8]                      = {};

    CodecAv1SegmentsParams segmentParams         = {};

    uint8_t  bitDepthIdc                         = 0;
    uint8_t  chromaFormat                        = 0;
    uint32_t superblockSizeUsed                  = 0;
    uint8_t  skipModeFrame[2]                    = {};
    uint8_t  refFrameSide                        = 0;
    uint8_t  refFrameBiasFlag                    = 0;
    uint8_t  frameLevelGlobalMotionInvalidFlags  = 0;
    uint32_t warpParamsArrayProjection[21]       = {};
    uint32_t refFrameRes[8]                      = {};
    uint32_t refScaleFactor[8]                   = {};
    uint8_t  refOrderHints[8]                    = {};
    uint32_t errorResilientMode                  = 0;

    bool     enableOrderHint                     = false;
    bool     enableCDEF                          = false;
    bool     enableSuperres                      = false;
    bool     enableRestoration                   = false;
    bool     enableFilterIntra                   = false;
    bool     enableIntraEdgeFilter               = false;
    bool     enableDualFilter                    = false;
    bool     enableInterIntraCompound            = false;
    bool     enableMaskedCompound                = false;
    bool     enableJointCompound                 = false;
    bool     forceIntegerMv                      = false;
    bool     allowWarpedMotion                   = false;
    bool     enableLargeScaleTile                = false;
    bool     motionModeSwitchable                = false;
    bool     useReferenceFrameMvSet              = false;

    uint8_t  orderHintBitsMinus1                 = 0;

    bool     notFirstPass                        = false;
    bool     vdencPackOnlyPass                   = false;
    bool     frameBitRateMaxReportMask           = false;
    bool     frameBitRateMinReportMask           = false;
    bool     headerPresent                       = false;

    uint32_t frameBitRateMax                     = 0;
    uint32_t frameBitRateMaxUnit                 = 0;  // select unit - 0 : 32B, 1 : 4KB
    uint32_t frameBitRateMin                     = 0;
    uint32_t frameBitRateMinUnit                 = 0;  // select unit - 0 : 32B, 1 : 4KB

    uint32_t frameDeltaQindexMax[2]              = {};
    uint32_t frameDeltaQindexMin                 = 0;

    uint32_t frameDeltaLFMax[2]                  = {};
    uint32_t frameDeltaLFMin                     = 0;

    uint32_t frameDeltaQindexLFMaxRange[2]       = {};
    uint32_t frameDeltaQindexLFMinRange          = 0;

    uint32_t minFramSize                         = 0;
    uint32_t minFramSizeUnits                    = 0;

    uint32_t bitOffsetForFirstPartitionSize      = 0;

    uint32_t class0_SSE_Threshold0               = 0;
    uint32_t class0_SSE_Threshold1               = 0;

    uint32_t rdmult                              = 0;

    int32_t  sbMaxBitSizeAllowed                 = 0;
    bool     sbMaxSizeReportMask                 = false;

    bool     autoBistreamStitchingInHardware     = false;

    bool     postCdefReconPixelStreamoutEn       = false;

    bool     allowScreenContentTools             = false;
    bool     allowIntraBC                        = false;
    bool     VdaqmEnable                         = false;

    __MHW_VDBOX_AVP_WRAPPER_EXT(AVP_PIC_STATE_CMDPAR_EXT);
};

struct _MHW_PAR_T(AVP_INLOOP_FILTER_STATE)
{
    uint8_t  loopFilterLevel[4]                  = {};
    uint8_t  loopFilterSharpness                 = 0;
    bool     loopFilterDeltaEnabled              = 0;
    uint8_t  deltaLfRes                          = 0;
    uint8_t  deltaLfMulti                        = 0;
    bool     loopFilterDeltaUpdate               = false;

    int8_t   loopFilterRefDeltas[8]              = {};
    int8_t   loopFilterModeDeltas[2]             = {};

    uint8_t  cdefYStrength[8]                    = {};
    uint8_t  cdefUVStrength[8]                   = {};
    uint8_t  cdefBits                            = 0;
    uint8_t  cdefDampingMinus3                   = 0;

    //super-resolution;
    uint32_t superresUpscaledWidthMinus1         = 0;
    uint8_t  superresDenom                       = 0;
    int32_t lumaPlaneXStepQn                     = 0;
    int32_t lumaPlaneX0Qn                        = 0;
    int32_t chromaPlaneXStepQn                   = 0;
    int32_t chromaPlaneX0Qn                      = 0;

    //loop restoration;
    uint8_t  LoopRestorationType[3]              = {};
    uint8_t  LoopRestorationSizeLuma             = 0;
    bool     UseSameLoopRestorationSizeForChroma = false;
};

struct _MHW_PAR_T(AVP_TILE_CODING)
{
    uint16_t tileId                                  = 0;
    uint16_t tgTileNum                                 = 0;  //!< Tile ID in its Tile group
    uint16_t tileGroupId                             = 0;

    uint16_t tileColPositionInSb                     = 0;
    uint16_t tileRowPositionInSb                     = 0;

    uint16_t tileWidthInSbMinus1                     = 0;  //!< Tile width minus 1 in SB unit
    uint16_t tileHeightInSbMinus1                    = 0;  //!< Tile height minus 1 in SB unit

    bool     tileRowIndependentFlag                  = false;
    bool     firstTileInAFrame                       = false;
    bool     lastTileOfColumn                        = false;
    bool     lastTileOfRow                           = false;
    bool     firstTileOfTileGroup                    = false;
    bool     lastTileOfTileGroup                     = false;
    bool     lastTileOfFrame                         = false;
    bool     disableCdfUpdateFlag                    = false;
    bool     disableFrameContextUpdateFlag           = false;
#if (_DEBUG || _RELEASE_INTERNAL)
    bool     enableAvpDebugMode                      = false;
#endif

    uint8_t  numOfActiveBePipes                      = 0;
    uint16_t numOfTileColumnsInFrame                 = 0;
    uint16_t numOfTileRowsInFrame                    = 0;
    uint16_t outputDecodedTileColPos                 = 0;
    uint16_t outputDecodedTileRowPos                 = 0;
};

struct _MHW_PAR_T(AVP_SEGMENT_STATE)
{
    uint8_t                 numSegments       = 1;
    CodecAv1SegmentsParams  av1SegmentParams  = {};
    uint8_t                 currentSegmentId  = 0;
};

struct _MHW_PAR_T(AVP_PIPE_BUF_ADDR_STATE)
{
    PMOS_RESOURCE     refs[8]                          = {};
    MOS_MEMCOMP_STATE mmcStatePreDeblock               = MOS_MEMCOMP_DISABLED;
    MOS_MEMCOMP_STATE mmcStateRawSurf                  = MOS_MEMCOMP_DISABLED;
    PMOS_SURFACE      decodedPic                       = nullptr;
    PMOS_RESOURCE     intrabcDecodedOutputFrameBuffer  = nullptr;
    PMOS_RESOURCE     cdfTableInitBuffer               = nullptr;
    uint32_t          cdfTableInitBufferOffset         = 0;
    PMOS_RESOURCE     cdfTableBwdAdaptBuffer           = nullptr;
    PMOS_RESOURCE     segmentIdReadBuffer              = nullptr;
    PMOS_RESOURCE     segmentIdWriteBuffer             = nullptr;
    PMOS_RESOURCE     colMvTempBuffer[9]               = {};
    PMOS_RESOURCE     curMvTempBuffer                  = nullptr;
    PMOS_RESOURCE     bsLineRowstoreBuffer             = nullptr;
    PMOS_RESOURCE     bsTileLineRowstoreBuffer         = nullptr;
    PMOS_RESOURCE     intraPredLineRowstoreBuffer      = nullptr;
    PMOS_RESOURCE     intraPredTileLineRowstoreBuffer  = nullptr;
    PMOS_RESOURCE     spatialMVLineBuffer              = nullptr;
    PMOS_RESOURCE     spatialMVCodingTileLineBuffer    = nullptr;
    PMOS_RESOURCE     lrMetaTileColumnBuffer           = nullptr;
    PMOS_RESOURCE     lrTileLineYBuffer                = nullptr;
    PMOS_RESOURCE     lrTileLineUBuffer                = nullptr;
    PMOS_RESOURCE     lrTileLineVBuffer                = nullptr;
    PMOS_RESOURCE     deblockLineYBuffer               = nullptr;
    PMOS_RESOURCE     deblockLineUBuffer               = nullptr;
    PMOS_RESOURCE     deblockLineVBuffer               = nullptr;
    PMOS_RESOURCE     deblockTileLineYBuffer           = nullptr;
    PMOS_RESOURCE     deblockTileLineVBuffer           = nullptr;
    PMOS_RESOURCE     deblockTileLineUBuffer           = nullptr;
    PMOS_RESOURCE     deblockTileColumnYBuffer         = nullptr;
    PMOS_RESOURCE     deblockTileColumnUBuffer         = nullptr;
    PMOS_RESOURCE     deblockTileColumnVBuffer         = nullptr;
    PMOS_RESOURCE     cdefLineBuffer                   = nullptr;
    PMOS_RESOURCE     cdefTileLineBuffer               = nullptr;
    PMOS_RESOURCE     cdefTileColumnBuffer             = nullptr;
    PMOS_RESOURCE     cdefMetaTileLineBuffer           = nullptr;
    PMOS_RESOURCE     cdefMetaTileColumnBuffer         = nullptr;
    PMOS_RESOURCE     cdefTopLeftCornerBuffer          = nullptr;
    PMOS_RESOURCE     superResTileColumnYBuffer        = nullptr;
    PMOS_RESOURCE     superResTileColumnUBuffer        = nullptr;
    PMOS_RESOURCE     superResTileColumnVBuffer        = nullptr;
    PMOS_RESOURCE     lrTileColumnYBuffer              = nullptr;
    PMOS_RESOURCE     lrTileColumnUBuffer              = nullptr;
    PMOS_RESOURCE     lrTileColumnVBuffer              = nullptr;
    PMOS_RESOURCE     lrTileColumnAlignBuffer          = nullptr;
    PMOS_RESOURCE     decodedFrameStatusErrorBuffer    = nullptr;
    PMOS_RESOURCE     decodedBlockDataStreamoutBuffer  = nullptr;
    PMOS_RESOURCE     originalPicSourceBuffer          = nullptr;
    PMOS_RESOURCE     dsPictureSourceBuffer            = nullptr;
    PMOS_RESOURCE     tileSizeStreamoutBuffer          = nullptr;
    uint32_t          tileSizeStreamoutBufferOffset    = 0;
    PMOS_RESOURCE     tileStatisticsPakStreamoutBuffer = nullptr;
    PMOS_RESOURCE     cuStreamoutBuffer                = nullptr;
    PMOS_RESOURCE     sseLineBuffer                    = nullptr;
    PMOS_RESOURCE     sseTileLineBuffer                = nullptr;
    PMOS_SURFACE      postCDEFpixelsBuffer             = nullptr;
    MOS_MEMCOMP_STATE postCdefSurfMmcState             = MOS_MEMCOMP_DISABLED;

    PMOS_RESOURCE     filmGrainTileColumnDataBuffer    = nullptr;
    PMOS_RESOURCE     filmGrainSampleTemplateBuffer    = nullptr;
    PMOS_RESOURCE     filmGrainOutputSurface           = nullptr;

    PMOS_RESOURCE     AvpPipeBufAddrStatePar0          = nullptr;
};

struct _MHW_PAR_T(AVP_INTER_PRED_STATE)
{
    uint8_t savedRefOrderHints[7][7];
    uint8_t refMaskMfProj;
};

struct _MHW_PAR_T(AVP_IND_OBJ_BASE_ADDR_STATE)
{
    uint32_t      Mode                      = 0;
    PMOS_RESOURCE dataBuffer                = nullptr;
    uint32_t      dataSize                  = 0;
    uint32_t      dataOffset                = 0;
    PMOS_RESOURCE mvObjectBuffer            = nullptr;
    uint32_t      mvObjectSize              = 0;
    uint32_t      mvObjectOffset            = 0;
    PMOS_RESOURCE pakBaseObjectBuffer       = nullptr;
    uint32_t      pakBaseObjectSize         = 0;
    uint32_t      pakBaseObjectOffset       = 0;
    PMOS_RESOURCE pakTileSizeStasBuffer     = nullptr;
    uint32_t      pakTileSizeStasBufferSize = 0;
    uint32_t      pakTileSizeRecordOffset   = 0;
};

struct _MHW_PAR_T(AVP_SURFACE_STATE)
{
    uint32_t          pitch              = 0;
    uint32_t          uOffset            = 0;
    uint32_t          vOffset            = 0;
    uint8_t           surfaceStateId     = 0;
    uint8_t           bitDepthLumaMinus8 = 0;
    MOS_MEMCOMP_STATE mmcState[av1TotalRefsPerFrame] = { MOS_MEMCOMP_DISABLED };
    uint32_t          compressionFormat  = 0;
    SURFACE_FORMAT    srcFormat          = SURFACE_FORMAT::SURFACE_FORMAT_PLANAR4208;
    uint32_t          uvPlaneAlignment   = 0;
};

struct _MHW_PAR_T(AVP_BSD_OBJECT)
{
    uint32_t         bsdDataLength      = 0;
    uint32_t         bsdDataStartOffset = 0;
};

struct _MHW_PAR_T(AVP_PAK_INSERT_OBJECT)
{
    PBSBuffer bsBuffer                           = nullptr;
    // also reuse dwBitSize for passing SrcDataEndingBitInclusion when (pEncoder->bLastPicInStream || pEncoder->bLastPicInSeq)
    uint32_t          bitSize                    = 0;
    uint32_t          offset                     = 0;
    uint32_t          skipEmulationCheckCount    = 0;
    bool              lastPicInSeq               = false;
    bool              lastPicInStream            = false;
    bool              lastHeader                 = false;
    bool              emulationByteBitsInsert    = false;
    bool              setLastPicInStreamData     = false;
    bool              sliceHeaderIndicator       = false;
    bool              headerLengthExcludeFrmSize = false;
    bool              bResetBitstreamStartingPos = false;
    bool              endOfHeaderInsertion       = false;
    uint32_t          lastPicInSeqData           = 0;
    uint32_t          lastPicInStreamData        = 0;
    PMHW_BATCH_BUFFER batchBufferForPakSlices    = nullptr;
};

struct _MHW_PAR_T(AVP_FILM_GRAIN_STATE)
{
    uint16_t    grainRandomSeed            = 0;
    uint8_t     clipToRestrictedRange      = 0;
    uint8_t     numOfYPoints               = 0;
    uint8_t     numOfCbPoints              = 0;
    uint8_t     numOfCrPoints              = 0;
    uint8_t     matrixCoefficients         = 0;
    uint8_t     grainScalingMinus8         = 0;
    uint8_t     arCoeffLag                 = 0;
    uint32_t    arCoeffShiftMinus6         = 0;
    uint32_t    grainScaleShift            = 0;
    uint32_t    chromaScalingFromLuma      = 0;
    uint32_t    grainNoiseOverlap          = 0;
                                               
    uint8_t     pointYValue[14]            = {};
    uint8_t     pointYScaling[14]          = {};
    uint8_t     pointCbValue[10]           = {};
    uint8_t     pointCbScaling[10]         = {};
    uint8_t     pointCrValue[10]           = {};
    uint8_t     pointCrScaling[10]         = {};
                                               
    int8_t      arCoeffsY[24]              = {};
    int8_t      arCoeffsCb[25]             = {};
    int8_t      arCoeffsCr[25]             = {};
                                               
    uint8_t     cbMult                     = 0;
    uint8_t     cbLumaMult                 = 0;
    uint16_t    cbOffset                   = 0;
    uint8_t     crMult                     = 0;
    uint8_t     crLumaMult                 = 0;
    uint16_t    crOffset                   = 0;
};

}  // namespace avp
}  // namespace vdbox
}  // namespace mhw

#endif  // __MHW_VDBOX_AVP_CMDPAR_H__
