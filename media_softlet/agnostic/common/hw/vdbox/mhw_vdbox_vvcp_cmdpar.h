/*
* Copyright (c) 2022, Intel Corporation
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
//! \file     mhw_vdbox_vvcp_cmdpar.h
//! \brief    MHW VDBOX VVCP command parameters
//! \details
//!

#ifndef __MHW_VDBOX_VVCP_CMDPAR_H__
#define __MHW_VDBOX_VVCP_CMDPAR_H__

#include "codec_def_decode_vvc.h"
#include "mhw_vdbox.h"
#include "mhw_vdbox_cmdpar.h"

namespace mhw
{
namespace vdbox
{
namespace vvcp
{

// this enum should align with hwcmd defination
enum CODEC_STANDARD_SELECT
{
    CODEC_STANDARD_SELECT_VVC = 3,  //!< No additional details
};

enum CommandsNumberOfAddresses
{
    MI_BATCH_BUFFER_START_CMD_NUMBER_OF_ADDRESSES           = 1,  //  2 DW for  1 address field
    MI_STORE_DATA_IMM_CMD_NUMBER_OF_ADDRESSES               = 1,  //  2 DW for  1 address field
    MI_FLUSH_DW_CMD_NUMBER_OF_ADDRESSES                     = 1,  //  2 DW for  1 address field
    MI_CONDITIONAL_BATCH_BUFFER_END_CMD_NUMBER_OF_ADDRESSES = 1,  //  2 DW for  1 address field
    MI_STORE_REGISTER_MEM_CMD_NUMBER_OF_ADDRESSES           = 1,  //  2 DW for  1 address field
    MI_COPY_MEM_MEM_CMD_NUMBER_OF_ADDRESSES                 = 4,  //  4 DW for  2 address fields
    MI_SEMAPHORE_WAIT_CMD_NUMBER_OF_ADDRESSES               = 1,  //  2 DW for  1 address field
    MI_ATOMIC_CMD_NUMBER_OF_ADDRESSES                       = 1,  //  2 DW for  1 address field

    MFX_WAIT_CMD_NUMBER_OF_ADDRESSES = 0,  //  0 DW for    address fields

    VVCP_PIPE_MODE_SELECT_CMD_NUMBER_OF_ADDRESSES        = 0,   //  0 DW for    address fields
    VVCP_SURFACE_STATE_CMD_NUMBER_OF_ADDRESSES           = 0,   //  0 DW for    address fields
    VVCP_PIPE_BUF_ADDR_STATE_CMD_NUMBER_OF_ADDRESSES     = 61,  //           61 address fields
    VVCP_IND_OBJ_BASE_ADDR_STATE_CMD_NUMBER_OF_ADDRESSES = 2,   //            2 address fields
    VVCP_PIC_STATE_CMD_NUMBER_OF_ADDRESSES               = 0,   //  0 DW for    address fields
    VVCP_REF_IDX_STATE_CMD_NUMBER_OF_ADDRESSES           = 0,   //  0 DW for    address fields
    VVCP_DPB_STATE_CMD_NUMBER_OF_ADDRESSES               = 0,   //  0 DW for    address fields
    VVCP_TILE_CODING_CMD_NUMBER_OF_ADDRESSES             = 0,   //  0 DW for    address fields
    VVCP_BSD_OBJECT_CMD_NUMBER_OF_ADDRESSES              = 0,   //  0 DW for    address fields
    VVCP_MEM_DATA_ACCESS_CMD_NUMBER_OF_ADDRESSES         = 0,   //  0 DW for    address fields
    VVCP_SLICE_STATE_CMD_NUMBER_OF_ADDRESSES             = 0,   //  0 DW for    address fields
    VVCP_VD_CONTROL_STATE_CMD_NUMBER_OF_ADDRESSES        = 0,   //  0 DW for    address fields
    VVCP_WEIGHTOFFSET_STATE_CMD_NUMBER_OF_ADDRESSES      = 0,   //  0 DW for    address fields

    VD_PIPELINE_FLUSH_CMD_NUMBER_OF_ADDRESSES = 0,  //  0 DW for  0 address fields
};

enum VvcpBufferType
{
    //Rowstore
    vcedLineBuffer = 0,
    vcmvLineBuffer,
    vcprLineBuffer,
    vclfYLineBuffer,
    vclfULineBuffer,
    vclfVLineBuffer,
    vcSaoYLineBuffer,
    vcSaoULineBuffer,
    vcSaoVLineBuffer,
    vcAlfLineBuffer,

    //Tile (boundary) storage
    vclfYTileRowBuffer,
    vclfYTileColumnBuffer,
    vclfUTileRowBuffer,
    vclfUTileColumnBuffer,
    vclfVTileRowBuffer,
    vclfVTileColumnBuffer,

    vcSaoYTileRowBuffer,
    vcSaoYTileColumnBuffer,
    vcSaoUTileRowBuffer,
    vcSaoUTileColumnBuffer,
    vcSaoVTileRowBuffer,
    vcSaoVTileColumnBuffer,

    vcAlfTileRowBuffer,
    vcAlfYTileColumnBuffer,
    vcAlfUTileColumnBuffer,
    vcAlfVTileColumnBuffer,

    vcMvTemporalBuffer,

    vvcpInternalBufMax = 27
};

struct VvcpBufferSizePar
{
    uint32_t m_picWidth;
    uint32_t m_picHeight;
    uint16_t m_maxTileWidthInCtus;
    uint8_t  m_bitDepthIdc;
    uint8_t  m_spsChromaFormatIdc;
    uint32_t m_spsLog2CtuSizeMinus5;
    uint32_t m_bufferSize;
};

// VVCP internal buffer size table [buffer_index][CTU][bitDepth]
struct VvcBufSizeClsPerCtu
{
    uint8_t m_clPerCtu[3][2];
    uint8_t m_extraCl;
};

static const VvcBufSizeClsPerCtu CodecVvcBufferSize[2][vvcpInternalBufMax-1] =
{
    //400
    {
        { 1,  1,  1,  1,  2,  2,  0 }, // VCED Line Buffer (EDLB)
        { 2,  2,  4,  4,  8,  8,  0 }, // VCMV Line Buffer (MVLB)
        { 1,  1,  1,  2,  2,  4,  0 }, // VCPR Line Buffer (PRLB)
        { 6,  6, 10, 11, 18, 20,  0 }, // VCLF Y Line Buffer (LFYLB)
        { 0,  0,  0,  0,  0,  0,  0 }, // VCLF U Line Buffer (LFULB)
        { 0,  0,  0,  0,  0,  0,  0 }, // VCLF V Line Buffer (LFVLB)
        { 2,  2,  3,  3,  6,  6,  0 }, // VCSAO Y Line Buffer (SAYLB)
        { 0,  0,  0,  0,  0,  0,  0 }, // VCSAO Y Line Buffer (SAULB)
        { 0,  0,  0,  0,  0,  0,  0 }, // VCSAO Y Line Buffer (SAVLB)
        { 1,  1,  1,  1,  1,  1,  0 }, // VCALF Line Buffer (ALFLB)

        { 6,  6, 10, 11, 18, 20,  0 }, // VCLF Y Tile Row Buffer (LFYTR)
        { 9, 10, 17, 19, 34, 38,  0 }, // VCLF Y Tile Column Buffer (LFYTC)
        { 0,  0,  0,  0,  0,  0,  0 }, // VCLF U Tile Row Buffer (LFUTR)
        { 0,  0,  0,  0,  0,  0,  0 }, // VCLF U Tile Column Buffer (LFUTC)
        { 0,  0,  0,  0,  0,  0,  0 }, // VCLF V Tile Row Buffer (LFVTR)
        { 0,  0,  0,  0,  0,  0,  0 }, // VCLF V Tile Column Buffer (LFVTC)
        { 2,  2,  3,  3,  6,  6,  0 }, // VCSAO Y Tile Row Buffer (SAYTR)
        { 2,  2,  3,  3,  6,  6,  0 }, // VCSAO Y Tile Column Buffer (SAYTC)
        { 0,  0,  0,  0,  0,  0,  0 }, // VCSAO U Tile Row Buffer (SAUTR)
        { 0,  0,  0,  0,  0,  0,  0 }, // VCSAO U Tile Column Buffer (SAUTC)
        { 0,  0,  0,  0,  0,  0,  0 }, // VCSAO V Tile Row Buffer (SAVTR)
        { 0,  0,  0,  0,  0,  0,  0 }, // VCSAO V Tile Column Buffer (SAVTC)
        { 1,  1,  1,  1,  1,  1,  0 }, // VCALF Tile Row Buffer (ALFTR)
        { 6,  7, 11, 13, 21, 26,  1 }, // VCALF Y Tile Column Buffer (ALYTC)
        { 0,  0,  0,  0,  0,  0,  0 }, // VCALF U Tile Column Buffer (ALUTC)
        { 0,  0,  0,  0,  0,  0,  0 }, // VCALF V Tile Column Buffer (ALVTC)
    },

    //420
    {
        { 1,  1,  1,  1,  2,  2,  0 }, // VCED Line Buffer (EDLB)
        { 2,  2,  4,  4,  8,  8,  0 }, // VCMV Line Buffer (MVLB)
        { 2,  2,  2,  4,  4,  8,  0 }, // VCPR Line Buffer (PRLB)
        { 6,  6, 10, 11, 18, 20,  0 }, // VCLF Y Line Buffer (LFYLB)
        { 2,  2,  3,  3,  5,  5,  0 }, // VCLF U Line Buffer (LFULB)
        { 2,  2,  3,  3,  5,  5,  0 }, // VCLF V Line Buffer (LFVLB)
        { 2,  2,  3,  3,  6,  6,  1 }, // VCSAO Y Line Buffer (SAYLB)
        { 1,  1,  2,  2,  3,  3,  1 }, // VCSAO U Line Buffer (SAULB)
        { 1,  1,  2,  2,  3,  3,  1 }, // VCSAO V Line Buffer (SAVLB)
        { 1,  1,  1,  1,  1,  1,  0 }, // VCALF Line Buffer (ALFLB)

        { 6,  6, 10, 11, 18, 20,  0 }, // VCLF Y Tile Row Buffer (LFYTR)
        { 9, 10, 17, 19, 34, 38,  0 }, // VCLF Y Tile Column Buffer (LFYTC)
        { 2,  2,  3,  3,  5,  5,  0 }, // VCLF U Tile Row Buffer (LFUTR)
        { 2,  3,  4,  5,  8,  9,  0 }, // VCLF U Tile Column Buffer (LFUTC)
        { 2,  2,  3,  3,  5,  5,  0 }, // VCLF V Tile Row Buffer (LFVTR)
        { 2,  3,  4,  5,  8,  9,  0 }, // VCLF V Tile Column Buffer (LFVTC)
        { 2,  2,  3,  3,  6,  6,  1 }, // VCSAO Y Tile Row Buffer (SAYTR)
        { 2,  2,  3,  3,  6,  6,  1 }, // VCSAO Y Tile Column Buffer (SAYTC)
        { 1,  1,  2,  2,  3,  3,  1 }, // VCSAO U Tile Row Buffer (SAUTR)
        { 1,  1,  2,  2,  3,  3,  1 }, // VCSAO U Tile Column Buffer (SAUTC)
        { 1,  1,  2,  2,  3,  3,  1 }, // VCSAO V Tile Row Buffer (SAVTR)
        { 1,  1,  2,  2,  3,  3,  1 }, // VCSAO V Tile Column Buffer (SAVTC)
        { 1,  1,  1,  1,  1,  1,  0 }, // VCALF Tile Row Buffer (ALFTR)
        { 6,  7, 11, 13, 21, 26,  1 }, // VCALF Y Tile Column Buffer (ALYTC)
        { 2,  2,  3,  4,  6,  7,  1 }, // VCALF U Tile Column Buffer (ALUTC)
        { 2,  2,  3,  4,  6,  7,  1 }, // VCALF V Tile Column Buffer (ALVTC)
    }
};


//! \brief SURFACE_FORMAT
//! \details
//!     Specifies the format of the surface.
//!
enum SURFACE_FORMAT
{
    SURFACE_FORMAT_PLANAR4208 = 4,   //!< No additional details
    SURFACE_FORMAT_P010       = 13,  //!< This format can be used for 8, 9, 10 bit 420 format
};

struct _MHW_PAR_T(VVCP_VD_CONTROL_STATE)
{
    bool pipelineInitialization     = false;
    bool memoryImplicitFlush        = false;
    bool pipeScalableModePipeLock   = false;
    bool pipeScalableModePipeUnlock = false;
};

struct _MHW_PAR_T(VVCP_PIPE_MODE_SELECT)
{
    uint8_t   codecSelect                = 0;
    bool      picStatusErrorReportEnable = false;
    uint8_t   codecStandardSelect        = 0;
    uint32_t  picStatusErrorReportId     = 0;
};

struct _MHW_PAR_T(VVCP_SURFACE_STATE)
{
    uint8_t  surfaceId            = 0;
    uint32_t surfacePitchMinus1   = 0;
    uint8_t  surfaceFormat        = 0;
    uint32_t yOffsetForUCbInPixel = 0;
    uint32_t compressionFormat    = 0;
};

struct _MHW_PAR_T(VVCP_PIPE_BUF_ADDR_STATE)
{
    MOS_SURFACE  *decodedPic               = nullptr;  //!< Decoded Output Frame Buffer
    MOS_RESOURCE *references[15]           = {};       //!< Reference Frame Buffer
    MOS_RESOURCE *colMvTemporalBuffer[15]  = {};       //!< Collocated MV temporal buffer
    MOS_RESOURCE *curMvTemporalBuffer      = nullptr;  //!< Current MV temporal buffer
    MOS_RESOURCE *apsScalingListDataBuffer = nullptr;
    MOS_RESOURCE *apsAlfBuffer             = nullptr;
    MOS_RESOURCE *spsChromaQpTableBuffer   = nullptr;

    MOS_RESOURCE *vcedLineBuffer = nullptr;
    MOS_RESOURCE *vcmvLineBuffer = nullptr;
    MOS_RESOURCE *vcprLineBuffer = nullptr;

    MOS_RESOURCE *vclfYLineBuffer       = nullptr;
    MOS_RESOURCE *vclfYTileRowBuffer    = nullptr;
    MOS_RESOURCE *vclfYTileColumnBuffer = nullptr;
    MOS_RESOURCE *vclfULineBuffer       = nullptr;
    MOS_RESOURCE *vclfUTileRowBuffer    = nullptr;
    MOS_RESOURCE *vclfUTileColumnBuffer = nullptr;
    MOS_RESOURCE *vclfVLineBuffer       = nullptr;
    MOS_RESOURCE *vclfVTileRowBuffer    = nullptr;
    MOS_RESOURCE *vclfVTileColumnBuffer = nullptr;

    MOS_RESOURCE *vcSaoYLineBuffer       = nullptr;
    MOS_RESOURCE *vcSaoYTileRowBuffer    = nullptr;
    MOS_RESOURCE *vcSaoYTileColumnBuffer = nullptr;
    MOS_RESOURCE *vcSaoULineBuffer       = nullptr;
    MOS_RESOURCE *vcSaoUTileRowBuffer    = nullptr;
    MOS_RESOURCE *vcSaoUTileColumnBuffer = nullptr;
    MOS_RESOURCE *vcSaoVLineBuffer       = nullptr;
    MOS_RESOURCE *vcSaoVTileRowBuffer    = nullptr;
    MOS_RESOURCE *vcSaoVTileColumnBuffer = nullptr;

    MOS_RESOURCE *vcAlfLineBuffer        = nullptr;
    MOS_RESOURCE *vcAlfTileRowBuffer     = nullptr;
    MOS_RESOURCE *vcAlfYTileColumnBuffer = nullptr;
    MOS_RESOURCE *vcAlfUTileColumnBuffer = nullptr;
    MOS_RESOURCE *vcAlfVTileColumnBuffer = nullptr;
};

struct _MHW_PAR_T(VVCP_IND_OBJ_BASE_ADDR_STATE)
{
    uint32_t      dwDataSize     = 0;
    uint32_t      dwDataOffset   = 0;
    PMOS_RESOURCE presDataBuffer = nullptr;
};

struct _MHW_PAR_T(VVCP_PIC_STATE)
{
    bool spsSubpicInfoPresentFlag        = false;
    bool spsIndependentSubpicsFlag       = false;
    bool spsSubpicSameSizeFlag           = false;
    bool spsEntropyCodingSyncEnabledFlag = false;
    bool spsQtbttDualTreeIntraFlag       = false;
    bool spsMaxLumaTransformSize64Flag   = false;
    bool spsTransformSkipEnabledFlag     = false;
    bool spsBdpcmEnabledFlag             = false;
    bool spsMtsEnabledFlag               = false;
    bool spsExplicitMtsIntraEnabledFlag  = false;
    bool spsExplicitMtsInterEnabledFlag  = false;
    bool spsLfnstEnabledFlag             = false;
    bool spsJointCbcrEnabledFlag         = false;
    bool spsSameQpTableForChromaFlag     = false;
    bool dLmcsDisabledFlag               = false;
    bool dDblkDisabledFlag               = false;
    bool dSaoLumaDisabledFlag            = false;
    bool dSaoChromaDisabledFlag          = false;
    bool dAlfDisabledFlag                = false;
    bool dAlfCbDisabledFlag              = false;
    bool dAlfCrDisabledFlag              = false;
    bool dAlfCcCbDisabledFlag            = false;
    bool dAlfCcCrDisabledFlag            = false;
    bool dSingleSliceFrameFlag           = false;

    bool spsSbtmvpEnabledFlag                                 = false;
    bool spsAmvrEnabledFlag                                   = false;
    bool spsSmvdEnabledFlag                                   = false;
    bool spsMmvdEnabledFlag                                   = false;
    bool spsSbtEnabledFlag                                    = false;
    bool spsAffineEnabledFlag                                 = false;
    bool sps6ParamAffineEnabledFlag                           = false;
    bool spsAffineAmvrEnabledFlag                             = false;
    bool spsBcwEnabledFlag                                    = false;
    bool spsCiipEnabledFlag                                   = false;
    bool spsGpmEnabledFlag                                    = false;
    bool spsIspEnabledFlag                                    = false;
    bool spsMrlEnabledFlag                                    = false;
    bool spsMipEnabledFlag                                    = false;
    bool spsCclmEnabledFlag                                   = false;
    bool spsChromaHorizontalCollocatedFlag                    = false;
    bool spsChromaVerticalCollocatedFlag                      = false;
    bool spsTemporalMvpEnabledFlag                            = false;
    bool spsPaletteEnabledFlag                                = false;
    bool spsActEnabledFlag                                    = false;
    bool spsIbcEnabledFlag                                    = false;
    bool spsLadfEnabledFlag                                   = false;
    bool spsScalingMatrixForLfnstDisabledFlag                 = false;
    bool spsScalingMatrixForAlternativeColorSpaceDisabledFlag = false;
    bool spsScalingMatrixDesignatedColorSpaceFlag             = false;

    bool ppsLoopFilterAcrossTilesEnabledFlag  = false;
    bool ppsRectSliceFlag                     = false;
    bool ppsSingleSlicePerSubpicFlag          = false;
    bool ppsLoopFilterAcrossSlicesEnabledFlag = false;
    bool ppsWeightedPredFlag                  = false;
    bool ppsWeightedBipredFlag                = false;
    bool ppsRefWraparoundEnabledFlag          = false;
    bool ppsCuQpDeltaEnabledFlag              = false;
    bool virtualboundariespresentflag         = false;
    bool phNonRefPicFlag                      = false;
    bool phChromaResidualScaleFlag            = false;
    bool phTemporalMvpEnabledFlag             = false;
    bool phMmvdFullpelOnlyFlag                = false;
    bool phMvdL1ZeroFlag                      = false;
    bool phBdofDisabledFlag                   = false;
    bool phDmvrDisabledFlag                   = false;
    bool phProfDisabledFlag                   = false;
    bool phJointCbcrSignFlag                  = false;

    uint8_t  spsChromaFormatIdc                  = 0;
    uint8_t  spsLog2CtuSizeMinus5                = 0;
    uint8_t  spsBitdepthMinus8                   = 0;
    uint8_t  spsLog2MinLumaCodingBlockSizeMinus2 = 0;
    uint16_t spsNumSubpicsMinus1                 = 0;

    uint8_t spsLog2TransformSkipMaxSizeMinus2   = 0;
    uint8_t spsSixMinusMaxNumMergeCand          = 0;
    uint8_t spsFiveMinusMaxNumSubblockMergeCand = 0;
    uint8_t dMaxNumGpmMergeCand                 = 0;
    uint8_t spsLog2ParallelMergeLevelMinus2     = 0;
    uint8_t spsMinQpPrimeTs                     = 0;
    uint8_t spsSixMinusMaxNumIbcMergeCand       = 0;

    int8_t spsLadfQpOffset0 = 0;
    int8_t spsLadfQpOffset1 = 0;
    int8_t spsLadfQpOffset2 = 0;
    int8_t spsLadfQpOffset3 = 0;

    uint16_t spsLadfDeltaThresholdMinus10  = 0;
    uint16_t spsLadfDeltaThresholdMinus11  = 0;
    int8_t   spsLadfLowestIntervalQpOffset = 0;

    uint16_t spsLadfDeltaThresholdMinus12 = 0;
    uint16_t spsLadfDeltaThresholdMinus13 = 0;
    uint8_t  spsNumLadfIntervalsMinus2    = 0;

    uint16_t ppsPicWidthInLumaSamples = 0;
    uint16_t ppsPicHeightInLumaSamples = 0;

    int32_t ppsScalingWinLeftOffset   = 0;
    int32_t ppsScalingWinRightOffset  = 0;
    int32_t ppsScalingWinTopOffset    = 0;
    int32_t ppsScalingWinBottomOffset = 0;

    uint16_t dNumtilerowsminus1    = 0;
    uint16_t dNumtilecolumnsminus1 = 0;

    int8_t  ppsCbQpOffset                  = 0;
    int8_t  ppsCrQpOffset                  = 0;
    int8_t  ppsJointCbcrQpOffsetValue      = 0;
    uint8_t ppsChromaQpOffsetListLenMinus1 = 0;

    int8_t ppsCbQpOffsetList0 = 0;
    int8_t ppsCbQpOffsetList1 = 0;
    int8_t ppsCbQpOffsetList2 = 0;
    int8_t ppsCbQpOffsetList3 = 0;

    int8_t   ppsCbQpOffsetList4               = 0;
    int8_t   ppsCbQpOffsetList5               = 0;
    uint16_t ppsPicWidthMinusWraparoundOffset = 0;

    int8_t ppsCrQpOffsetList0 = 0;
    int8_t ppsCrQpOffsetList1 = 0;
    int8_t ppsCrQpOffsetList2 = 0;
    int8_t ppsCrQpOffsetList3 = 0;

    int8_t ppsCrQpOffsetList4 = 0;
    int8_t ppsCrQpOffsetList5 = 0;

    int8_t ppsJointCbcrQpOffsetList0 = 0;
    int8_t ppsJointCbcrQpOffsetList1 = 0;
    int8_t ppsJointCbcrQpOffsetList2 = 0;
    int8_t ppsJointCbcrQpOffsetList3 = 0;

    int8_t ppsJointCbcrQpOffsetList4 = 0;
    int8_t ppsJointCbcrQpOffsetList5 = 0;

    uint8_t numvervirtualboundaries                = 0;
    uint8_t numhorvirtualboundaries                = 0;
    uint8_t phLog2DiffMinQtMinCbIntraSliceLuma     = 0;
    uint8_t phMaxMttHierarchyDepthIntraSliceLuma   = 0;
    uint8_t phLog2DiffMaxBtMinQtIntraSliceLuma     = 0;
    uint8_t phLog2DiffMaxTtMinQtIntraSliceLuma     = 0;
    uint8_t phLog2DiffMinQtMinCbIntraSliceChroma   = 0;
    uint8_t phMaxMttHierarchyDepthIntraSliceChroma = 0;

    uint16_t dVirtualboundaryposxminus10 = 0;
    uint16_t dVirtualboundaryposyminus10 = 0;
    uint16_t dVirtualboundaryposxminus11 = 0;
    uint16_t dVirtualboundaryposyminus11 = 0;
    uint16_t dVirtualboundaryposxminus12 = 0;
    uint16_t dVirtualboundaryposyminus12 = 0;

    uint8_t phLog2DiffMaxBtMinQtIntraSliceChroma = 0;
    uint8_t phLog2DiffMaxTtMinQtIntraSliceChroma = 0;
    uint8_t phCuQpDeltaSubdivIntraSlice          = 0;
    uint8_t phCuChromaQpOffsetSubdivIntraSlice   = 0;
    uint8_t phLog2DiffMinQtMinCbInterSlice       = 0;
    uint8_t phMaxMttHierarchyDepthInterSlice     = 0;

    uint8_t phLog2DiffMaxBtMinQtInterSlice     = 0;
    uint8_t phLog2DiffMaxTtMinQtInterSlice     = 0;
    uint8_t phCuQpDeltaSubdivInterSlice        = 0;
    uint8_t phCuChromaQpOffsetSubdivInterSlice = 0;

    uint8_t         dActiveapsid   = 0;
    CodecVvcAlfData alfApsArray[8] = {};
};

struct _MHW_PAR_T(VVCP_DPB_STATE)
{
    VvcRefFrameAttributes refFrameAttr[15]      = {};
    uint16_t              refPicScaleWidth[15]  = {};
    uint16_t              refPicScaleHeight[15] = {};
};

struct _MHW_PAR_T(VVCP_SLICE_STATE)
{
    bool shAlfEnabledFlag                   = false;
    bool shAlfCbEnabledFlag                 = false;
    bool shAlfCrEnabledFlag                 = false;
    bool shAlfCcCbEnabledFlag               = false;
    bool shAlfCcCrEnabledFlag               = false;
    bool shLmcsUsedFlag                     = false;
    bool shExplicitScalingListUsedFlag      = false;
    bool shCabacInitFlag                    = false;
    bool shCollocatedFromL0Flag             = false;
    bool shCuChromaQpOffsetEnabledFlag      = false;
    bool shSaoLumaUsedFlag                  = false;
    bool shSaoChromaUsedFlag                = false;
    bool shDeblockingFilterDisabledFlag     = false;
    bool shDepQuantUsedFlag                 = false;
    bool shSignDataHidingUsedFlag           = false;
    bool shTsResidualCodingDisabledFlag     = false;
    bool nobackwardpredflag                 = false;
    bool pVvcpDebugEnable                   = false;
    bool dMultipleSlicesInTileFlag          = false;
    bool dIsbottommostsliceoftileFlag       = false;
    bool dIstopmostsliceoftileFlag          = false;
    bool dSubpicTreatedAsPicFlag            = false;
    bool dLoopFilterAcrossSubpicEnabledFlag = false;
    bool dIsRightMostSliceOfSubpicFlag      = false;
    bool dIsLeftMostSliceOfSubpicFlag       = false;
    bool dIsBottomMostSliceOfSubpicFlag     = false;
    bool dIsTopMostSliceOfSubpicFlag        = false;
    bool dLastsliceofpicFlag                = false;

    uint32_t numctusincurrslice = 0;

    uint16_t shNumTilesInSliceMinus1       = 0;
    uint8_t  shSliceType                   = 0;
    uint8_t  shNumAlfApsIdsLuma            = 0;
    uint8_t  alfChromaNumAltFiltersMinus1  = 0;
    uint8_t  alfCcCbFiltersSignalledMinus1 = 0;
    uint8_t  alfCcCrFiltersSignalledMinus1 = 0;

    uint8_t shAlfApsIdLuma0  = 0;
    uint8_t shAlfApsIdLuma1  = 0;
    uint8_t shAlfApsIdLuma2  = 0;
    uint8_t shAlfApsIdLuma3  = 0;
    uint8_t shAlfApsIdLuma4  = 0;
    uint8_t shAlfApsIdLuma5  = 0;
    uint8_t shAlfApsIdLuma6  = 0;

    uint8_t shAlfApsIdChroma   = 0;
    uint8_t shAlfCcCbApsId     = 0;
    uint8_t shAlfCcCrApsId     = 0;
    uint8_t numrefidxactive0   = 0;
    uint8_t numrefidxactive1   = 0;
    uint8_t shCollocatedRefIdx = 0;

    uint8_t sliceqpy            = 0;
    uint8_t shCbQpOffset        = 0;
    uint8_t shCrQpOffset        = 0;
    uint8_t shJointCbcrQpOffset = 0;

    uint8_t shLumaBetaOffsetDiv2 = 0;
    uint8_t shLumaTcOffsetDiv2   = 0;
    uint8_t shCbBetaOffsetDiv2   = 0;
    uint8_t shCbTcOffsetDiv2     = 0;
    uint8_t shCrBetaOffsetDiv2   = 0;
    uint8_t shCrTcOffsetDiv2     = 0;

    uint16_t dSubpicCtuTopLeftX  = 0;
    uint16_t dSubpicCtuTopLeftY  = 0;
    uint16_t dSubpicWidthMinus1  = 0;
    uint16_t dSubpicHeightMinus1 = 0;

    uint16_t dSliceheightinctus  = 0;

    uint16_t dToplefttilex   = 0;
    uint16_t dToplefttiley   = 0;
    uint32_t dSlicestartctbx = 0;
    uint32_t dSlicestartctby = 0;

    // LMCS
    bool               spsLmcsEnabledFlag = false;
    uint8_t            phLmcsApsId        = 0;
    CodecVvcLmcsData   *vvcLmcsData       = nullptr;
    ApsLmcsReshapeInfo *vvcLmcsShapeInfo  = nullptr;
};

struct _MHW_PAR_T(VVCP_BSD_OBJECT)
{
    uint32_t bsdDataLength      = 0;
    uint32_t bsdDataStartOffset = 0;
};

struct _MHW_PAR_T(VVCP_REF_IDX_STATE)
{
    uint8_t listIdx       = 0;
    uint8_t numRefForList = 0;

    int8_t        refIdxSymLx[2]                                 = {};
    CODEC_PICTURE refPicList[2][vvcMaxNumRefFrame]               = {};
    bool          stRefPicFlag[2][vvcMaxNumRefFrame]             = {};
    bool          rprConstraintsActiveFlag[2][vvcMaxNumRefFrame] = {};
    bool          unavailableRefPic[2][vvcMaxNumRefFrame]        = {};
    int16_t       diffPicOrderCnt[2][vvcMaxNumRefFrame]          = {};
};

struct _MHW_PAR_T(VVCP_WEIGHTOFFSET_STATE)
{
    VvcWeightedPredInfo *wpInfo    = nullptr;
    int32_t             listIdx    = 0;
};

struct _MHW_PAR_T(VVCP_TILE_CODING)
{
    uint16_t tilecolbdval              = 0;
    uint16_t tilerowbdval              = 0;
    uint16_t colwidthval               = 0;
    uint16_t rowheightval              = 0;
    uint16_t currenttilecolumnposition = 0;
    uint16_t currenttilerowposition    = 0;
    struct
    {
        uint32_t m_isrightmosttileofsliceFlag : 1;
        uint32_t m_isleftmosttileofsliceFlag : 1;
        uint32_t m_isbottommosttileofsliceFlag : 1;
        uint32_t m_istopmosttileofsliceFlag : 1;
        uint32_t m_isrightmosttileofframeFlag : 1;
        uint32_t m_isleftmosttileofframeFlag : 1;
        uint32_t m_isbottommosttileofframeFlag : 1;
        uint32_t m_istopmosttileofframeFlag : 1;
        uint32_t m_reserved : 24;
    } flags;
};

}  // namespace vvcp
}  // namespace vdbox
}  // namespace mhw

#endif  // __MHW_VDBOX_VVCP_CMDPAR_H__
