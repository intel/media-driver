/*
* Copyright (c) 2020-2023, Intel Corporation
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
//! \file     mhw_vdbox_vdenc_cmdpar.h
//! \brief    MHW command parameters
//! \details
//!

#ifndef __MHW_VDBOX_VDENC_CMDPAR_H__
#define __MHW_VDBOX_VDENC_CMDPAR_H__

#include <functional>
#include "mhw_vdbox_cmdpar.h"
#include "codec_def_common.h"
#include "codec_def_common_encode.h"

#if _MEDIA_RESERVED
#include "mhw_vdbox_vdenc_cmdpar_ext.h"
#define __MHW_VDBOX_VDENC_WRAPPER(STUFF)
#define __MHW_VDBOX_VDENC_WRAPPER_EXT(STUFF) STUFF
#else
#define __MHW_VDBOX_VDENC_WRAPPER(STUFF) STUFF
#define __MHW_VDBOX_VDENC_WRAPPER_EXT(STUFF)
#endif  // _MEDIA_RESERVED

namespace mhw
{
namespace vdbox
{
namespace vdenc
{
static constexpr uint32_t MAX_REF_LIST_NUM = 2;
static constexpr uint32_t MAX_REF_NUM_L0   = 4;
static constexpr uint32_t MAX_REF_NUM_L1   = 4;  //  low delay B may have 3 L1 refs
static constexpr uint32_t MAX_REF_NUM      = MAX_REF_NUM_L0 > MAX_REF_NUM_L1 ? MAX_REF_NUM_L0 : MAX_REF_NUM_L1;
static constexpr uint32_t MAX_REF_NUM_L0L1 = MAX_REF_NUM_L0 + MAX_REF_NUM_L1;

enum class SurfaceFormat
{
    yuv422          = 0x0,
    rgba4444        = 0x1,
    yuv444          = 0x2,
    p010Variant     = 0x3,
    planar4208      = 0x4,
    yCrCbSwapY422   = 0x5,
    yCrCbSwapUv422  = 0x6,
    yCrCbSwapUvy422 = 0x7,
    y216            = 0x8,
    y210            = 0x8,
    r10g10b10a2     = 0x9,
    y410            = 0xa,
    nv21            = 0xb,
    y416            = 0xc,
    p010            = 0xd,
    planarP016      = 0xe,
    y8Unorm         = 0xf,
    y16             = 0x10,
    y216Variant     = 0x11,
    y416Variant     = 0x12,
    yuyvVariant     = 0x13,
    ayuvVariant     = 0x14,
};

struct RowStorePar
{
    enum Codec
    {
        AVC,
        HEVC,
        VP9,
        AV1
    };

    enum ChromaFormat
    {
        MONOCHROME,
        YUV420,
        YUV422,
        YUV444
    };

    enum BitDepth
    {
        DEPTH_8,
        DEPTH_10,
        DEPTH_12
    };

    enum LcuSize
    {
        SIZE_32,
        SIZE_64,
        SIZE_OTHER,
    };

    Codec        mode       = AVC;
    ChromaFormat format     = MONOCHROME;
    BitDepth     bitDepth   = DEPTH_8;
    LcuSize      lcuSize    = SIZE_32;
    bool         isField    = false;
    uint32_t     frameWidth = 0;
};

struct _MHW_PAR_T(VDENC_STREAMIN_STATE)
{
    uint8_t maxTuSize;
    uint8_t maxCuSize;
    uint8_t numImePredictors;
    uint8_t numMergeCandidateCu64x64;
    uint8_t numMergeCandidateCu32x32;
    uint8_t numMergeCandidateCu16x16;
    uint8_t numMergeCandidateCu8x8;
    bool    setQpRoiCtrl;
    int8_t  forceQp[4];
    uint8_t roiCtrl;
    uint8_t puTypeCtrl;
};

struct _MHW_PAR_T(VDENC_CONTROL_STATE)
{
    bool vdencInitialization = true;
};

struct _MHW_PAR_T(VDENC_PIPE_MODE_SELECT)
{
    uint8_t standardSelect           = 0;
    bool    scalabilityMode          = false;
    bool    frameStatisticsStreamOut = false;
    bool    pakObjCmdStreamOut       = false;
    bool    tlbPrefetch              = false;
    bool    dynamicSlice             = false;
    bool    streamIn                 = false;
    uint8_t bitDepthMinus8           = 0;
    uint8_t chromaType               = 0;
    uint8_t outputRangeControlCsc    = 1;
    bool    tileBasedReplayMode      = false;
    bool    randomAccess             = false;
    bool    rgbEncodingMode          = false;
    bool    hmeRegionPrefetch        = true;
    uint8_t topPrefetchEnableMode    = 0;
    uint8_t leftPrefetchAtWrapAround = 1;
    uint8_t verticalShift32Minus1    = 2;
    uint8_t hzShift32Minus1          = 3;
    uint8_t numVerticalReqMinus1     = 6;
    uint8_t numHzReqMinus1           = 2;
    uint8_t prefetchOffset           = 0;
    uint8_t captureMode              = 0;
    uint8_t wirelessSessionId        = 0;
    uint8_t tailPointerReadFrequency = 0;
    uint8_t streamingBufferConfig    = 0;
    bool    bt2020RGB2YUV            = 0;
    uint8_t rgbInputStudioRange      = 0;
    uint8_t convertedYUVStudioRange  = 0;
    uint8_t VdencPipeModeSelectPar0  = 0;
    uint8_t VdencPipeModeSelectPar1  = 0;
    uint8_t VdencPipeModeSelectPar2  = 0;
    uint8_t VdencPipeModeSelectPar3  = 0;
    uint8_t VdencPipeModeSelectPar4  = 0;
    uint8_t VdencPipeModeSelectPar5  = 0;
    uint8_t VdencPipeModeSelectPar6  = 0;
    uint8_t VdencPipeModeSelectPar7  = 0;
    uint8_t VdencPipeModeSelectPar8  = 0;
    bool    fastPassEn               = false;
    uint8_t fastPassScale            = 0;
    uint8_t DownScaleType            = 0;
    uint8_t VdencPipeModeSelectPar12 = 0;
    uint8_t VdencPipeModeSelectPar13 = 0;
};

struct _MHW_PAR_T(VDENC_SRC_SURFACE_STATE)
{
    uint32_t          width                         = 0;
    uint32_t          height                        = 0;
    uint32_t          pitch                         = 0;
    uint32_t          uOffset                       = 0;
    uint32_t          vOffset                       = 0;
    MOS_TILE_TYPE     tileType                      = MOS_TILE_X;
    MOS_TILE_MODE_GMM tileModeGmm                   = MOS_TILE_LINEAR_GMM;
    MOS_FORMAT        format                        = Format_Any;
    bool              gmmTileEn                     = false;
    bool              colorSpaceSelection           = false;
    bool              displayFormatSwizzle          = false;
    uint32_t          chromaDownsampleFilterControl = 0;
    uint8_t           vDirection                    = 0;
};

struct _MHW_PAR_T(VDENC_REF_SURFACE_STATE)
{
    uint32_t          width       = 0;
    uint32_t          height      = 0;
    uint32_t          pitch       = 0;
    uint32_t          uOffset     = 0;
    uint32_t          vOffset     = 0;
    MOS_TILE_TYPE     tileType    = MOS_TILE_X;
    MOS_TILE_MODE_GMM tileModeGmm = MOS_TILE_LINEAR_GMM;
    MOS_FORMAT        format      = Format_Any;
    bool              gmmTileEn   = false;
    uint8_t           vDirection  = 0;
};

struct _MHW_PAR_T(VDENC_DS_REF_SURFACE_STATE)
{
    uint8_t           vDirectionStage1  = 0;
    uint8_t           vDirectionStage2  = 0;
    uint32_t          widthStage1       = 0;
    uint32_t          widthStage2       = 0;
    uint32_t          heightStage1      = 0;
    uint32_t          heightStage2      = 0;
    uint32_t          pitchStage1       = 0;
    uint32_t          pitchStage2       = 0;
    uint32_t          uOffsetStage1     = 0;
    uint32_t          uOffsetStage2     = 0;
    uint32_t          vOffsetStage1     = 0;
    uint32_t          vOffsetStage2     = 0;
    MOS_TILE_TYPE     tileTypeStage1    = MOS_TILE_X;
    MOS_TILE_TYPE     tileTypeStage2    = MOS_TILE_X;
    MOS_TILE_MODE_GMM tileModeGmmStage1 = MOS_TILE_LINEAR_GMM;
    MOS_TILE_MODE_GMM tileModeGmmStage2 = MOS_TILE_LINEAR_GMM;
    bool              gmmTileEnStage1   = false;
    bool              gmmTileEnStage2   = false;
};

struct _MHW_PAR_T(VDENC_PIPE_BUF_ADDR_STATE)
{
    PMOS_SURFACE      surfaceRaw                        = nullptr;
    MOS_MEMCOMP_STATE mmcStateRaw                       = MOS_MEMCOMP_DISABLED;
    uint32_t          compressionFormatRaw              = 0;
    uint32_t          compressionFormatRecon            = 0;
    PMOS_RESOURCE     intraRowStoreScratchBuffer        = nullptr;
    PMOS_RESOURCE     streamOutBuffer                   = nullptr;
    uint32_t          streamOutOffset                   = 0;
    PMOS_RESOURCE     streamInBuffer                    = nullptr;
    uint32_t          numActiveRefL0                    = 0;
    uint32_t          numActiveRefL1                    = 0;
    PMOS_RESOURCE     refs[MAX_REF_NUM_L0L1]            = {};
    PMOS_RESOURCE     refsDsStage1[MAX_REF_NUM_L0L1]    = {};
    PMOS_RESOURCE     refsDsStage2[MAX_REF_NUM_L0L1]    = {};
    MOS_MEMCOMP_STATE mmcStatePreDeblock                = MOS_MEMCOMP_DISABLED;
    MOS_MEMCOMP_STATE mmcStatePostDeblock               = MOS_MEMCOMP_DISABLED;
    PMOS_SURFACE      surfaceDsStage1                   = nullptr;
    MOS_MEMCOMP_STATE mmcStateDsStage1                  = MOS_MEMCOMP_DISABLED;
    PMOS_SURFACE      surfaceDsStage2                   = nullptr;
    MOS_MEMCOMP_STATE mmcStateDsStage2                  = MOS_MEMCOMP_DISABLED;
    uint8_t           mmcSkipMask                       = 0;
    bool              mmcEnabled                        = false;
    bool              lowDelayB                         = false;
    bool              isPFrame                          = false;  //only HEVC should touch this flag
    PMOS_RESOURCE     colocatedMvReadBuffer             = nullptr;
    PMOS_RESOURCE     colMvTempBuffer[MAX_REF_NUM_L0L1] = {};
    PMOS_RESOURCE     pakObjCmdStreamOutBuffer          = nullptr;
    PMOS_RESOURCE     segmentMapStreamInBuffer          = nullptr;
    PMOS_RESOURCE     segmentMapStreamOutBuffer         = nullptr;
    PMOS_RESOURCE     tileRowStoreBuffer                = nullptr;
    PMOS_RESOURCE     mfdIntraRowStoreScratchBuffer     = nullptr;
    PMOS_RESOURCE     cumulativeCuCountStreamOutBuffer  = nullptr;
    PMOS_RESOURCE     colocatedMvWriteBuffer            = nullptr;
    PMOS_RESOURCE     vdencPipeBufAddrStatePar0         = nullptr;
    PMOS_RESOURCE     vdencPipeBufAddrStatePar1         = nullptr;
    PMOS_RESOURCE     vdencPipeBufAddrStatePar2         = nullptr;
};

struct _MHW_PAR_T(VDENC_WEIGHTSOFFSETS_STATE)
{
    int8_t   weightsLuma[MAX_REF_LIST_NUM][MAX_REF_NUM]      = {};
    int16_t  offsetsLuma[MAX_REF_LIST_NUM][MAX_REF_NUM]      = {};
    uint32_t denomLuma                                       = 0;
    int8_t   weightsChroma[MAX_REF_LIST_NUM][MAX_REF_NUM][2] = {};
    int16_t  offsetsChroma[MAX_REF_LIST_NUM][MAX_REF_NUM][2] = {};
    uint32_t denomChroma                                     = 0;
};

struct _MHW_PAR_T(VDENC_HEVC_VP9_TILE_SLICE_STATE)
{
    bool     tileEnable                    = false;
    bool     tileRowStoreSelect            = false;
    uint32_t tileWidth                     = 0;
    uint32_t tileHeight                    = 0;
    uint32_t numPipe                       = 0;
    uint32_t tileId                        = 0;
    uint32_t tileStartLCUX                 = 0;
    uint32_t tileStartLCUY                 = 0;
    uint32_t ctbSize                       = 0;
    uint32_t tileStreamInOffset            = 0;
    uint32_t tileLCUStreamOutOffset        = 0;
    uint32_t log2WeightDenomLuma           = 0;
    uint32_t log2WeightDenomChroma         = 0;
    uint32_t hevcVp9Log2WeightDenomLuma    = 0;
    uint32_t paletteModeEnable             = 0;
    uint32_t ibcControl                    = 0;
    uint32_t VdencHEVCVP9TileSlicePar0     = 0;
    uint32_t VdencHEVCVP9TileSlicePar1     = 0;
    uint32_t VdencHEVCVP9TileSlicePar2     = 0;
    uint32_t VdencHEVCVP9TileSlicePar3     = 0;
    uint32_t VdencHEVCVP9TileSlicePar4     = 0;
    uint32_t VdencHEVCVP9TileSlicePar5     = 0;
    uint32_t VdencHEVCVP9TileSlicePar6     = 0;
    uint32_t VdencHEVCVP9TileSlicePar7     = 0;
    uint32_t VdencHEVCVP9TileSlicePar8     = 0;
    uint32_t VdencHEVCVP9TileSlicePar9     = 0;
    uint32_t VdencHEVCVP9TileSlicePar10    = 0;
    uint32_t VdencHEVCVP9TileSlicePar11    = 0;
    uint32_t VdencHEVCVP9TileSlicePar12    = 0;
    uint32_t VdencHEVCVP9TileSlicePar13    = 0;
    uint32_t VdencHEVCVP9TileSlicePar14    = 0;
    uint32_t VdencHEVCVP9TileSlicePar15    = 0;
    uint8_t  VdencHEVCVP9TileSlicePar16[3] = {};
    uint32_t VdencHEVCVP9TileSlicePar17[3] = {};
    bool     VdencHEVCVP9TileSlicePar18    = false;
    uint32_t VdencHEVCVP9TileSlicePar19    = 0;
    uint32_t VdencHEVCVP9TileSlicePar22    = 0;
    uint32_t VdencHEVCVP9TileSlicePar23    = 0;
    uint32_t VdencHEVCVP9TileSlicePar24    = 0;
    uint32_t tileRowstoreOffset            = 0;
};

struct _MHW_PAR_T(VDENC_WALKER_STATE)
{
    bool     firstSuperSlice          = true;
    uint32_t tileSliceStartLcuMbX     = 0;
    uint32_t tileSliceStartLcuMbY     = 0;
    uint32_t nextTileSliceStartLcuMbX = 0;
    uint32_t nextTileSliceStartLcuMbY = 0;
};

struct _MHW_PAR_T(VD_PIPELINE_FLUSH)
{
    bool waitDoneHEVC             = false;
    bool waitDoneVDENC            = false;
    bool waitDoneMFL              = false;
    bool waitDoneMFX              = false;
    bool waitDoneVDCmdMsgParser   = false;
    bool flushHEVC                = false;
    bool flushVDENC               = false;
    bool flushMFL                 = false;
    bool flushMFX                 = false;
    bool waitDoneAV1              = false;
    bool flushAV1                 = false;
    bool waitDoneVDAQM            = false;
    bool flushVDAQM               = false;
    bool vvcpPipelineDone         = false;
    bool vvcpPipelineCommandFlush = false;

    __MHW_VDBOX_VDENC_WRAPPER_EXT(VD_PIPELINE_FLUSH_CMDPAR_EXT);
};

struct _MHW_PAR_T(VDENC_AVC_SLICE_STATE)
{
    uint8_t roundIntra          = 0;
    uint8_t roundIntraEnable    = 1;
    uint8_t roundInter          = 0;
    uint8_t roundInterEnable    = 0;
    uint8_t log2WeightDenomLuma = 0;
};

struct _MHW_PAR_T(VDENC_AVC_IMG_STATE)
{
    uint8_t  pictureType                            = 0;
    uint8_t  transform8X8Flag                       = 0;
    bool     colMVWriteEnable                       = false;
    uint8_t  subpelMode                             = 3;
    bool     colMVReadEnable                        = false;
    uint8_t  bidirectionalWeight                    = 0;
    uint16_t pictureHeightMinusOne                  = 0;
    uint16_t pictureWidth                           = 0;
    uint8_t  fwdRefIdx0ReferencePicture             = 0;
    uint8_t  bwdRefIdx0ReferencePicture             = 0;
    uint8_t  fwdRefIdx1ReferencePicture             = 0;
    uint8_t  fwdRefIdx2ReferencePicture             = 0;
    uint8_t  numberOfL0ReferencesMinusOne           = 0;
    uint8_t  numberOfL1ReferencesMinusOne           = 0;
    uint8_t  intraRefreshMbPos                      = 0;
    uint8_t  intraRefreshMbSizeMinusOne             = 0;
    uint8_t  intraRefreshEnableRollingIEnable       = 0;
    uint8_t  intraRefreshMode                       = 0;
    uint8_t  qpAdjustmentForRollingI                = 0;
    uint8_t  roiQpAdjustmentForZone0                = 0;
    uint8_t  roiQpAdjustmentForZone1                = 0;
    uint8_t  roiQpAdjustmentForZone2                = 0;
    uint8_t  roiQpAdjustmentForZone3                = 0;
    uint8_t  minQp                                  = 0xa;
    uint8_t  maxQp                                  = 0x33;
    bool     roiEnable                              = false;
    bool     mbLevelQpEnable                        = false;
    bool     mbLevelDeltaQpEnable                   = false;
    bool     longtermReferenceFrameBwdRef0Indicator = false;
    uint8_t  qpPrimeY                               = 0;
    uint8_t  trellisQuantEn                         = 0;
    uint8_t  pocNumberForCurrentPicture             = 0;
    uint8_t  pocNumberForFwdRef0                    = 0;
    uint8_t  pocNumberForFwdRef1                    = 0;
    uint8_t  pocNumberForFwdRef2                    = 0;
    uint8_t  pocNumberForBwdRef0                    = 0;

    __MHW_VDBOX_VDENC_WRAPPER(
        std::vector<std::function<MOS_STATUS(uint32_t *)>> extSettings);
    __MHW_VDBOX_VDENC_WRAPPER_EXT(
        VDENC_AVC_IMG_STATE_CMDPAR_EXT);
};

struct _MHW_PAR_T(VDENC_CMD1)
{
    uint16_t vdencCmd1Par0      = 0;
    uint16_t vdencCmd1Par1      = 0;
    uint8_t  vdencCmd1Par2[8]   = {};
    uint8_t  vdencCmd1Par3[12]  = {};
    uint8_t  vdencCmd1Par4[12]  = {};
    uint8_t  vdencCmd1Par5      = 0;
    uint8_t  vdencCmd1Par6      = 0;
    uint8_t  vdencCmd1Par7      = 0;
    uint8_t  vdencCmd1Par8[4]   = {};
    uint8_t  vdencCmd1Par9[4]   = {};
    uint8_t  vdencCmd1Par10[4]  = {};
    uint8_t  vdencCmd1Par11[4]  = {};
    uint8_t  vdencCmd1Par12[4]  = {};
    uint8_t  vdencCmd1Par13[4]  = {};
    uint8_t  vdencCmd1Par14[4]  = {};
    uint8_t  vdencCmd1Par15[4]  = {};
    uint8_t  vdencCmd1Par16     = 0;
    uint8_t  vdencCmd1Par17     = 0;
    uint8_t  vdencCmd1Par18     = 0;
    uint8_t  vdencCmd1Par19     = 0;
    uint8_t  vdencCmd1Par20     = 0;
    uint8_t  vdencCmd1Par21     = 0;
    uint8_t  vdencCmd1Par22     = 0;
    uint8_t  vdencCmd1Par23     = 0;
    uint8_t  vdencCmd1Par24     = 0;
    uint8_t  vdencCmd1Par25     = 0;
    uint8_t  vdencCmd1Par26     = 0;
    uint8_t  vdencCmd1Par27     = 0;
    uint8_t  vdencCmd1Par28     = 0;
    uint8_t  vdencCmd1Par29     = 0;
    uint8_t  vdencCmd1Par30     = 0;
    uint8_t  vdencCmd1Par31     = 0;
    uint8_t  vdencCmd1Par32     = 0;
    uint8_t  vdencCmd1Par33     = 0;
    uint8_t  vdencCmd1Par34     = 0;
    uint8_t  vdencCmd1Par35     = 0;
    uint8_t  vdencCmd1Par36     = 0;
    uint8_t  vdencCmd1Par37     = 0;
    uint8_t  vdencCmd1Par38     = 0;
    uint8_t  vdencCmd1Par39     = 0;
    uint8_t  vdencCmd1Par40     = 0;
    uint8_t  vdencCmd1Par41     = 0;
    uint8_t  vdencCmd1Par42     = 0;
    uint8_t  vdencCmd1Par43     = 0;
    uint8_t  vdencCmd1Par44     = 0;
    uint8_t  vdencCmd1Par45     = 0;
    uint8_t  vdencCmd1Par46     = 0;
    uint8_t  vdencCmd1Par47     = 0;
    uint8_t  vdencCmd1Par48     = 0;
    uint8_t  vdencCmd1Par49     = 0;
    uint8_t  vdencCmd1Par50     = 0;
    uint8_t  vdencCmd1Par51     = 0;
    uint8_t  vdencCmd1Par52     = 0;
    uint8_t  vdencCmd1Par53     = 0;
    uint8_t  vdencCmd1Par54     = 0;
    uint8_t  vdencCmd1Par55     = 0;
    uint8_t  vdencCmd1Par56     = 0;
    uint8_t  vdencCmd1Par57     = 0;
    uint8_t  vdencCmd1Par58     = 0;
    uint8_t  vdencCmd1Par59     = 0;
    uint8_t  vdencCmd1Par60     = 0;
    uint8_t  vdencCmd1Par61     = 0;
    uint8_t  vdencCmd1Par62     = 0;
    uint8_t  vdencCmd1Par63     = 0;
    uint8_t  vdencCmd1Par64     = 0;
    uint8_t  vdencCmd1Par65     = 0;
    uint8_t  vdencCmd1Par66     = 0;
    uint8_t  vdencCmd1Par67     = 0;
    uint8_t  vdencCmd1Par68     = 0;
    uint8_t  vdencCmd1Par69     = 0;
    uint8_t  vdencCmd1Par70     = 0;
    uint8_t  vdencCmd1Par71     = 0;
    uint8_t  vdencCmd1Par72     = 0;
    uint8_t  vdencCmd1Par73     = 0;
    uint8_t  vdencCmd1Par74     = 0;
    uint8_t  vdencCmd1Par75     = 0;
    uint8_t  vdencCmd1Par76     = 0;
    uint8_t  vdencCmd1Par77     = 0;
    uint8_t  vdencCmd1Par78     = 0;
    uint8_t  vdencCmd1Par79     = 0;
    uint8_t  vdencCmd1Par80     = 0;
    uint8_t  vdencCmd1Par81     = 0;
    uint8_t  vdencCmd1Par82     = 0;
    uint8_t  vdencCmd1Par83     = 0;
    uint8_t  vdencCmd1Par84     = 0;
    uint8_t  vdencCmd1Par85     = 0;
    uint8_t  vdencCmd1Par86     = 0;
    uint8_t  vdencCmd1Par87     = 0;
    uint8_t  vdencCmd1Par88     = 0;
    uint8_t  vdencCmd1Par89     = 0;
    uint8_t  vdencCmd1Par90     = 0;
    uint8_t  vdencCmd1Par91     = 0;
    uint8_t  vdencCmd1Par92     = 0;
    uint8_t  vdencCmd1Par93     = 0;
    uint8_t  vdencCmd1Par94     = 0;
    uint8_t  vdencCmd1Par95     = 0;
    uint8_t  vdnecCmd1Par96[4]  = {};
    uint8_t  vdnecCmd1Par97[4]  = {};
    uint8_t  vdnecCmd1Par98[4]  = {};
    uint8_t  vdnecCmd1Par99[4]  = {};
    uint8_t  vdnecCmd1Par100[2] = {};
    uint8_t  vdnecCmd1Par101[2] = {};
    uint8_t  vdnecCmd1Par102[2] = {};
    uint8_t  vdnecCmd1Par103[2] = {};
    uint8_t  vdencCmd1Par104[4] = {};
    uint8_t  vdencCmd1Par105[4] = {};
    uint8_t  vdencCmd1Par106[4] = {};
    uint8_t  vdencCmd1Par107    = 0;
};

struct _MHW_PAR_T(VDENC_CMD2)
{
    uint32_t width                            = 0;
    uint32_t height                           = 0;
    bool     constrainedIntraPred             = false;
    uint8_t  pictureType                      = 0;
    bool     temporalMvp                      = false;
    bool     collocatedFromL0                 = false;
    uint8_t  longTermReferenceFlagsL0         = 0;
    uint8_t  longTermReferenceFlagsL1         = 0;
    bool     transformSkip                    = false;
    int8_t   pocL0Ref0                        = 1;
    int8_t   pocL1Ref0                        = -1;
    int8_t   pocL0Ref1                        = 2;
    int8_t   pocL1Ref1                        = -2;
    int8_t   pocL0Ref2                        = 3;
    int8_t   pocL1Ref2                        = -3;
    int8_t   pocL0Ref3                        = 4;
    int8_t   pocL1Ref3                        = -4;
    bool     roiStreamIn                      = false;
    uint8_t  numRefL0                         = 0;
    uint8_t  numRefL1                         = 0;
    bool     segmentation                     = false;
    bool     segmentationTemporal             = false;
    bool     tiling                           = false;
    bool     vdencStreamIn                    = false;
    bool     pakOnlyMultiPass                 = false;
    uint8_t  frameIdxL0Ref0                   = 0;
    uint8_t  frameIdxL0Ref1                   = 0;
    uint8_t  frameIdxL0Ref2                   = 0;
    uint8_t  frameIdxL1Ref0                   = 0;
    uint8_t  minQp                            = 0;
    uint8_t  maxQp                            = 255;
    bool     temporalMvEnableForIntegerSearch = false;
    uint16_t intraRefreshPos                  = 0;
    uint8_t  intraRefreshMbSizeMinus1         = 1;
    uint8_t  intraRefreshMode                 = 0;
    bool     intraRefresh                     = false;
    uint8_t  qpAdjustmentForRollingI          = 0;
    uint8_t  qpForSegs[8]                     = {};
    bool     vp9DynamicSlice                  = false;
    uint8_t  qpPrimeYDc                       = 0;
    uint8_t  qpPrimeYAc                       = 0;
    uint32_t intraRefreshBoundary[3]          = {};
    uint8_t  av1RefId[2][4]                   = {{1, 1, 1, 1}, {1, 1, 1, 1}};
    uint8_t  subPelMode                       = 3;

    __MHW_VDBOX_VDENC_WRAPPER(
        std::vector<std::function<MOS_STATUS(uint32_t *)>> extSettings);
    __MHW_VDBOX_VDENC_WRAPPER_EXT(VDENC_CMD2_CMDPAR_EXT);
};

struct _MHW_PAR_T(VDENC_CMD3)
{
    uint8_t  vdencCmd3Par0[8]  = {};
    uint8_t  vdencCmd3Par1[12] = {};
    uint8_t  vdencCmd3Par2[12] = {};
    uint8_t  vdencCmd3Par3     = 0;
    uint8_t  vdencCmd3Par4     = 0;
    uint8_t  vdencCmd3Par5     = 0;
    uint8_t  vdencCmd3Par6     = 0;
    uint8_t  vdencCmd3Par7     = 0;
    uint8_t  vdencCmd3Par8     = 0;
    uint8_t  vdencCmd3Par9     = 0;
    uint8_t  vdencCmd3Par10    = 0;
    uint8_t  vdencCmd3Par11    = 0;
    uint8_t  vdencCmd3Par12    = 0;
    uint8_t  vdencCmd3Par13    = 0;
    uint8_t  vdencCmd3Par14    = 0;
    uint8_t  vdencCmd3Par15    = 0;
    uint8_t  vdencCmd3Par16    = 0;
    uint8_t  vdencCmd3Par17    = 0;
    uint8_t  vdencCmd3Par18    = 0;
    uint8_t  vdencCmd3Par19    = 0;
    uint8_t  vdencCmd3Par20    = 0;
    uint8_t  vdencCmd3Par21    = 0;
    uint8_t  vdencCmd3Par22    = 0;
    uint8_t  vdencCmd3Par23    = 0;
    uint8_t  vdencCmd3Par24    = 0;
    uint8_t  vdencCmd3Par25    = 0;
    uint8_t  vdencCmd3Par26    = 0;
    uint8_t  vdencCmd3Par27    = 0;
    uint8_t  vdencCmd3Par28    = 0;
    uint8_t  vdencCmd3Par29    = 0;
    uint8_t  vdencCmd3Par30    = 0;
    uint16_t vdencCmd3Par31    = 0;
    uint16_t vdencCmd3Par32    = 0;
};
}  // namespace vdenc
}  // namespace vdbox
}  // namespace mhw

#endif  // __MHW_VDBOX_VDENC_CMDPAR_H__
