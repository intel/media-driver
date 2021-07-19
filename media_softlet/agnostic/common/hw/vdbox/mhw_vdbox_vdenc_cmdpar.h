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
//! \file     mhw_vdbox_vdenc_cmdpar.h
//! \brief    MHW command parameters
//! \details
//!

#ifndef __MHW_VDBOX_VDENC_CMDPAR_H__
#define __MHW_VDBOX_VDENC_CMDPAR_H__

#include "mhw_vdbox_cmdpar.h"
#include "codec_def_common.h"
#include "codec_def_common_encode.h"

#ifdef IGFX_VDENC_INTERFACE_EXT_SUPPORT
#include "mhw_vdbox_vdenc_cmdpar_ext.h"
#define __MHW_VDBOX_VDENC_WRAPPER(STUFF)
#define __MHW_VDBOX_VDENC_WRAPPER_EXT(STUFF) STUFF
#else
#define __MHW_VDBOX_VDENC_WRAPPER(STUFF) STUFF
#define __MHW_VDBOX_VDENC_WRAPPER_EXT(STUFF)
#endif

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
    uint8_t quantizationPrecision    = 0;
    uint8_t latencyTolerate          = 0;
    __MHW_VDBOX_VDENC_WRAPPER_EXT(VDENC_PIPE_MODE_SELECT_CMDPAR_EXT);
};

struct _MHW_PAR_T(VDENC_SRC_SURFACE_STATE)
{
    uint32_t          width                = 0;
    uint32_t          height               = 0;
    uint32_t          pitch                = 0;
    uint32_t          uOffset              = 0;
    uint32_t          vOffset              = 0;
    MOS_TILE_TYPE     tileType             = MOS_TILE_X;
    MOS_TILE_MODE_GMM tileModeGmm          = MOS_TILE_LINEAR_GMM;
    MOS_FORMAT        format               = Format_Any;
    bool              gmmTileEn            = false;
    bool              colorSpaceSelection  = false;
    bool              displayFormatSwizzle = false;
    uint8_t           vDirection           = 0;
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
    bool              lowDelayB                         = false;
    bool              isPFrame                          = false; //only HEVC should touch this flag 
    PMOS_RESOURCE     colocatedMvReadBuffer             = nullptr;
    PMOS_RESOURCE     colMvTempBuffer[MAX_REF_NUM_L0L1] = {};
    PMOS_RESOURCE     pakObjCmdStreamOutBuffer          = nullptr;
    PMOS_RESOURCE     segmentMapStreamInBuffer          = nullptr;
    PMOS_RESOURCE     segmentMapStreamOutBuffer         = nullptr;
    PMOS_RESOURCE     tileRowStoreBuffer                = nullptr;
    PMOS_RESOURCE     mfdIntraRowStoreScratchBuffer     = nullptr;
    PMOS_RESOURCE     cumulativeCuCountStreamOutBuffer  = nullptr;
    PMOS_RESOURCE     colocatedMvWriteBuffer            = nullptr;
    __MHW_VDBOX_VDENC_WRAPPER_EXT(VDENC_PIPE_BUF_ADDR_STATE_CMDPAR_EXT);
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
    bool     tileEnable                 = false;
    bool     tileRowStoreSelect         = false;
    uint32_t tileWidth                  = 0;
    uint32_t tileHeight                 = 0;
    uint32_t numPipe                    = 0;
    uint32_t tileId                     = 0;
    uint32_t tileStartLCUX              = 0;
    uint32_t tileStartLCUY              = 0;
    uint32_t ctbSize                    = 0;
    uint32_t tileStreamInOffset         = 0;
    uint32_t tileLCUStreamOutOffset     = 0;
    uint32_t cumulativeCUTileOffset     = 0;
    uint32_t log2WeightDenomLuma        = 0;
    uint32_t log2WeightDenomChroma      = 0;
    uint32_t hevcVp9Log2WeightDenomLuma = 0;
    __MHW_VDBOX_VDENC_WRAPPER_EXT(VDENC_HEVC_VP9_TILE_SLICE_STATE_CMDPAR_EXT);
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
    bool waitDoneHEVC           = false;
    bool waitDoneVDENC          = false;
    bool waitDoneMFL            = false;
    bool waitDoneMFX            = false;
    bool waitDoneVDCmdMsgParser = false;
    bool flushHEVC              = false;
    bool flushVDENC             = false;
    bool flushMFL               = false;
    bool flushMFX               = false;
    __MHW_VDBOX_VDENC_WRAPPER_EXT(VD_PIPELINE_FLUSH_CMDPAR_EXT);
};

struct _MHW_PAR_T(VDENC_CMD1)
{
    __MHW_VDBOX_VDENC_WRAPPER_EXT(VDENC_CMD1_CMDPAR_EXT);
};

struct _MHW_PAR_T(VDENC_CMD2)
{
    __MHW_VDBOX_VDENC_WRAPPER_EXT(VDENC_CMD2_CMDPAR_EXT);
};

struct _MHW_PAR_T(VDENC_CMD3)
{
    __MHW_VDBOX_VDENC_WRAPPER_EXT(VDENC_CMD3_CMDPAR_EXT);
};

struct _MHW_PAR_T(VDENC_CMD4)
{
    __MHW_VDBOX_VDENC_WRAPPER_EXT(VDENC_CMD4_CMDPAR_EXT);
};

struct _MHW_PAR_T(VDENC_CMD5)
{
    __MHW_VDBOX_VDENC_WRAPPER_EXT(VDENC_CMD5_CMDPAR_EXT);
};
}  // namespace vdenc
}  // namespace vdbox
}  // namespace mhw

#endif  // __MHW_VDBOX_VDENC_CMDPAR_H__
