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

static constexpr uint32_t VDENC_ROWSTORE_BASEADDRESS      = 2370;
static constexpr uint32_t VDENC_IPDL_ROWSTORE_BASEADDRESS = 384;

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

struct _MHW_CMD_PAR_T(VDENC_CONTROL_STATE)
{
    virtual ~_MHW_CMD_PAR_T(VDENC_CONTROL_STATE)() = default;

    bool vdencInitialization = true;
};

struct _MHW_CMD_PAR_T(VDENC_PIPE_MODE_SELECT)
{
    virtual ~_MHW_CMD_PAR_T(VDENC_PIPE_MODE_SELECT)() = default;

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
};

struct _MHW_CMD_PAR_T(VDENC_SRC_SURFACE_STATE)
{
    virtual ~_MHW_CMD_PAR_T(VDENC_SRC_SURFACE_STATE)() = default;

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

struct _MHW_CMD_PAR_T(VDENC_REF_SURFACE_STATE)
{
    virtual ~_MHW_CMD_PAR_T(VDENC_REF_SURFACE_STATE)() = default;

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

struct _MHW_CMD_PAR_T(VDENC_DS_REF_SURFACE_STATE)
{
    virtual ~_MHW_CMD_PAR_T(VDENC_DS_REF_SURFACE_STATE)() = default;

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

struct _MHW_CMD_PAR_T(VDENC_PIPE_BUF_ADDR_STATE)
{
    virtual ~_MHW_CMD_PAR_T(VDENC_PIPE_BUF_ADDR_STATE)() = default;

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
    PMOS_RESOURCE     colocatedMvReadBuffer             = nullptr;
    PMOS_RESOURCE     colMvTempBuffer[MAX_REF_NUM_L0L1] = {};
    PMOS_RESOURCE     pakObjCmdStreamOutBuffer          = nullptr;
    PMOS_RESOURCE     segmentMapStreamInBuffer          = nullptr;
    PMOS_RESOURCE     segmentMapStreamOutBuffer         = nullptr;
    PMOS_RESOURCE     tileRowStoreBuffer                = nullptr;
    PMOS_RESOURCE     mfdIntraRowStoreScratchBuffer     = nullptr;
    PMOS_RESOURCE     cumulativeCuCountStreamOutBuffer  = nullptr;
    PMOS_RESOURCE     colocatedMvWriteBuffer            = nullptr;
};

struct _MHW_CMD_PAR_T(VDENC_WEIGHTSOFFSETS_STATE)
{
    virtual ~_MHW_CMD_PAR_T(VDENC_WEIGHTSOFFSETS_STATE)() = default;

    int8_t   weightsLuma[MAX_REF_LIST_NUM][MAX_REF_NUM]      = {};
    int16_t  offsetsLuma[MAX_REF_LIST_NUM][MAX_REF_NUM]      = {};
    uint32_t denomLuma                                       = 0;
    int8_t   weightsChroma[MAX_REF_LIST_NUM][MAX_REF_NUM][2] = {};
    int16_t  offsetsChroma[MAX_REF_LIST_NUM][MAX_REF_NUM][2] = {};
    uint32_t denomChroma                                     = 0;
};

struct _MHW_CMD_PAR_T(VDENC_HEVC_VP9_TILE_SLICE_STATE)
{
    virtual ~_MHW_CMD_PAR_T(VDENC_HEVC_VP9_TILE_SLICE_STATE)() = default;

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
};

struct _MHW_CMD_PAR_T(VDENC_WALKER_STATE)
{
    virtual ~_MHW_CMD_PAR_T(VDENC_WALKER_STATE)() = default;

    bool     firstSuperSlice          = true;
    uint32_t tileSliceStartLcuMbX     = 0;
    uint32_t tileSliceStartLcuMbY     = 0;
    uint32_t nextTileSliceStartLcuMbX = 0;
    uint32_t nextTileSliceStartLcuMbY = 0;
};

struct _MHW_CMD_PAR_T(VD_PIPELINE_FLUSH)
{
    virtual ~_MHW_CMD_PAR_T(VD_PIPELINE_FLUSH)() = default;

    bool waitDoneHEVC           = false;
    bool waitDoneVDENC          = false;
    bool waitDoneMFL            = false;
    bool waitDoneMFX            = false;
    bool waitDoneVDCmdMsgParser = false;
    bool flushHEVC              = false;
    bool flushVDENC             = false;
    bool flushMFL               = false;
    bool flushMFX               = false;
};

class CmdPar
{
public:
    virtual ~CmdPar() = default;

    _MHW_CMD_PAR_SET_DEF(VDENC_CONTROL_STATE);
    _MHW_CMD_PAR_SET_DEF(VDENC_PIPE_MODE_SELECT);
    _MHW_CMD_PAR_SET_DEF(VDENC_SRC_SURFACE_STATE);
    _MHW_CMD_PAR_SET_DEF(VDENC_REF_SURFACE_STATE);
    _MHW_CMD_PAR_SET_DEF(VDENC_DS_REF_SURFACE_STATE);
    _MHW_CMD_PAR_SET_DEF(VDENC_PIPE_BUF_ADDR_STATE);
    _MHW_CMD_PAR_SET_DEF(VDENC_WEIGHTSOFFSETS_STATE);
    _MHW_CMD_PAR_SET_DEF(VDENC_HEVC_VP9_TILE_SLICE_STATE);
    _MHW_CMD_PAR_SET_DEF(VDENC_WALKER_STATE);
    _MHW_CMD_PAR_SET_DEF(VD_PIPELINE_FLUSH);
};
}  // namespace vdenc
}  // namespace vdbox
}  // namespace mhw

#ifndef MHW_VDBOX_VDENC_CMDPAR_T
#define MHW_VDBOX_VDENC_CMDPAR_T mhw::vdbox::vdenc::CmdPar
#endif

#ifndef MHW_VDBOX_VDENC_CMD_PAR_T
#define MHW_VDBOX_VDENC_CMD_PAR_T(cmd) mhw::vdbox::vdenc::_MHW_CMD_PAR_T(cmd)
#endif

#define MHW_VDBOX_VDENC_TO_ACTUAL_CMD_PAR(cmd)                                  \
    MHW_FUNCTION_ENTER;                                                         \
    auto params = mhw::DynamicPointerCast<MHW_VDBOX_VDENC_CMD_PAR_T(cmd)>(par); \
    MHW_CHK_NULL_RETURN(params)

#endif  // __MHW_VDBOX_VDENC_CMDPAR_H__
