/*
* Copyright (c) 2021-2023, Intel Corporation
*
* Permission is hereby granted, free of int8_tge, to any person obtaining a
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
//! \file     codec_def_decode_vvc.h
//! \brief    Defines decode VVC types and macros shared by CodecHal, MHW, and DDI layer
//! \details  Applies to VVC decode only. Should not contain any DDI specific code.
//!
#ifndef __CODEC_DEF_DECODE_VVC_H__
#define __CODEC_DEF_DECODE_VVC_H__

#include "codec_def_common_vvc.h"
#include "codec_def_common.h"

#define CODEC_MAX_DPB_NUM_VVC                 127                                // Maximum number of uncompressed decoded buffers
#define CODEC_VVC_BUFFER_ARRAY_SIZE           32
#define CODEC_BITSTREAM_ALIGN_SIZE_VVC        128                                // Total quantity of data in Bitstream buffer shall be an integer multiple of 128 bytes
#define CODECHAL_VVC_MAX_NUM_SLICES_LVL_6     600
#define CODECHAL_VVC_MAX_NUM_SLICES_LVL_5     200
#define CODECHAL_VVC_NUM_DMEM_BUFFERS         32

#define CODEC_16K_VVC_MAX_PIC_WIDTH  16888
#define CODEC_16K_VVC_MAX_PIC_HEIGHT 16888

#define Pack4Bytes2DW(byte0, byte1, byte2, byte3) (((byte0)&0xff) | (((byte1)&0xff)<<8) | (((byte2)&0xff)<<16) | (((byte3)&0xff)<<24))
#define CHECK_RANGE(var, minVal, maxVal) \
    if((var) < (minVal) || (var) > (maxVal))\
    {\
        DECODE_ASSERTMESSAGE("Error detected: %s = %d out of range [%d, %d].\n", #var, var, minVal, maxVal);\
        res = MOS_STATUS_INVALID_PARAMETER;\
    }\

static const uint32_t vvcMaxSliceNum             = 600;          // Max slice number per frame, level 6.2
static const uint32_t vvcMaxSubpicNum            = 600;          // Per Spec, same with MaxSlicesPerAu =vvcMaxSliceNum
static const uint32_t vvcMaxLmcsNum              = 4;            // Max LMCS structures
static const uint32_t vvcMaxAlfNum               = 8;            // Max ALF structures
static const uint32_t vvcMaxScalingMatrixNum     = 8;            // Max Scaling Matrix structures
static const uint32_t vvcMaxTileParamsNum        = 441;          // Max Tile Params structures
static const uint32_t vvcMaxRplNum               = 130;          // Max RPL structures 128 sps rpls + 2 PH rpls
static const uint32_t vvcSpsCandidateRpl1Offset  = 64;           // vvcSpsCandidateRpl0(64) + vvcSpsCandidateRpl1(64) + PH rpl0(1) + PH rpl1(1) = 130
static const uint32_t vvcPhRpl0Offset            = 128;
static const uint32_t vvcPhRpl1Offset            = 129;          // vvcPhRpl1Offset must be vvcPhRpl0Offset + 1
static const uint32_t vvcMaxNumRefFrame          = 15;           // Max reference frame number
static const uint32_t vvcPicCodeCwBins           = 16;           // PIC_CODE_CW_BINS
static const uint32_t vvcFpPrec                  = 11;           // FP_PREC
static const uint32_t vvcCscaleFpPrec            = 11;           // CSCALE_FP_PREC
static const uint32_t vvcNumInitialMvBuffers     = 6;            // initial MV buffer number to allocate
static const uint32_t vvcMaxTileColNum           = 20;           // Max number of tile columns in a picture
static const uint32_t vvcMaxTileRowNum           = 440;          // Max number of tile rows in a picture
static const uint32_t vvcMaxTileNum              = 440;          // Max number of tiles in a picture


//VVC parameters definition

//!
//! \struct TileColDesc
//! \brief Define the Tile Column descriptor for VVC
//!
struct TileColDesc
{
    uint16_t m_startCtbX;
    uint16_t m_endCtbX;
    uint16_t m_widthInCtb;
};

//!
//! \struct TileRowDesc
//! \brief Define the Tile Row descriptor for VVC
//!
struct TileRowDesc
{
    uint16_t m_startCtbY;
    uint16_t m_endCtbY;
    uint16_t m_heightInCtb;
};

//!
//! \struct SliceDescriptor
//! \brief Define the Slice descriptor for VVC
//!
struct SliceDescriptor
{
    int32_t          m_numCtusInCurrSlice;                //!< number of CTUs in current slice

    //Rect slices params
    uint16_t         m_tileIdx;                           //!< tile index corresponding to the first CTU in the slice
    uint32_t         m_numSlicesInTile;                   //!< number of slices in current tile for the special case of multiple slices inside a single tile
    int16_t          m_sliceWidthInTiles;                 //!< slice width in units of tiles
    int16_t          m_sliceHeightInTiles;                //!< slice height in units of tiles
    uint16_t         m_sliceHeightInCtu;                  //!< slice height in units of CTUs for the special case of multiple slices inside a single tile

    //Additional params on SubPic v.s. Slice
    uint32_t         m_sliceStartCtbx;
    uint32_t         m_sliceStartCtby;
    uint16_t         m_subPicIdx;
    uint16_t         m_sliceIdxInSubPic;
    uint32_t         m_sliceEndCtbx;                      //! valid only for rect slice mode
    uint32_t         m_sliceEndCtby;                      //! valid only for rect slice mode

    //Only used for single slice per subpic
    uint32_t         m_multiSlicesInTileFlag;
    uint16_t         m_startTileX;
    uint16_t         m_startTileY;

    //for command programming
    uint32_t         m_topSliceInTileFlag;                //! valid only for multiple slices in one tile case, others ignore
    uint32_t         m_bottomSliceInTileFlag;             //! valid only for multiple slices in one tile case, others ignore

    //for error handling
    int16_t          m_sliceCtrlIdx;                      //! index of slice control params that current slice corresponds to
    bool             m_sliceAvailableFlag;                //! flag to indicate if current slice is available
};

//!
//! \struct ApsLmcsReshapeInfo
//! \brief Define the APS LMCS Reshape info for VVC
//!
struct ApsLmcsReshapeInfo
{
    uint16_t m_lmcsCW[16];
    int32_t  m_scaleCoeff[16];
    int32_t  m_invScaleCoeff[16];
    int32_t  m_chromaScaleCoeff[16];
    int16_t  m_lmcsPivot[17];
};

//!
//! \struct VvcWeightedPredInfo
//! \brief Define the weighted prediction info for VVC
//!
struct VvcWeightedPredInfo
{
    uint8_t         m_lumaLog2WeightDenom;              // [0..7]
    int8_t          m_deltaChromaLog2WeightDenom;       // [-7..7]
    uint8_t         m_numL0Weights;                     // [0..15]
    uint8_t         m_lumaWeightL0Flag[15];             // [0..1]
    uint8_t         m_chromaWeightL0Flag[15];           // [0..1]
    int8_t          m_deltaLumaWeightL0[15];            // [-128..127]
    int8_t          m_lumaOffsetL0[15];                 // [-128..127]
    int8_t          m_deltaChromaWeightL0[15][2];       // [-128..127]
    int16_t         m_deltaChromaOffsetL0[15][2];       // [-512..508]
    uint8_t         m_numL1Weights;                     // [0..15]
    uint8_t         m_lumaWeightL1Flag[15];             // [0..1]
    uint8_t         m_chromaWeightL1Flag[15];           // [0..1]
    int8_t          m_deltaLumaWeightL1[15];            // [-128..127]
    int8_t          m_lumaOffsetL1[15];                 // [-128..127]
    int8_t          m_deltaChromaWeightL1[15][2];       // [-128..127]
    int16_t         m_deltaChromaOffsetL1[15][2];       // [-512..508]
    uint32_t        m_reserved32b[4];
};

//!
//! \struct CodecVvcPicParams
//! \brief Define VVC picture parameters
//!
struct CodecVvcPicParams
{
    // SPS info
    uint16_t       m_spsPicWidthMaxInLumaSamples;              // [8..16888]
    uint16_t       m_spsPicHeightMaxInLumaSamples;             // [8..16888]

    uint16_t       m_spsNumSubpicsMinus1;                      // [0..599]
    uint8_t        m_spsSubpicIdLenMinus1;                     // [0..15]
    uint8_t        m_spsChromaFormatIdc;                       // [1]
    uint8_t        m_spsBitdepthMinus8;                        // [0..2]
    uint8_t        m_spsLog2CtuSizeMinus5;                     // [0..2]
    uint8_t        m_spsLog2MaxPicOrderCntLsbMinus4;           // [0..12]
    uint8_t        m_spsLog2MinLumaCodingBlockSizeMinus2;      // [0..4]
    uint8_t        m_spsPocMsbCycleLenMinus1;                  // [0..27]
    uint8_t        m_numExtraPhBits;                           // [0..15]
    uint8_t        m_numExtraShBits;                           // [0..15]
    uint8_t        m_spsLog2TransformSkipMaxSizeMinus2;        // [0..3]

    int8_t         m_chromaQpTable[3][112];                     // [-12..63]

    uint8_t        m_spsNumRefPicLists[2];                     // [0..64]
    uint8_t        m_spsSixMinusMaxNumMergeCand;               // [0..5]
    uint8_t        m_spsFiveMinusMaxNumSubblockMergeCand;      // [0..5]
    uint8_t        m_spsMaxNumMergeCandMinusMaxNumGpmCand;     // [0..4]
    uint8_t        m_spsLog2ParallelMergeLevelMinus2;          // [0..5]
    uint8_t        m_spsMinQpPrimeTs;                          // [0..8]
    uint8_t        m_spsSixMinusMaxNumIbcMergeCand;            // [0..5]
    uint8_t        m_spsNumLadfIntervalsMinus2;                // [0..3]
    int8_t         m_spsLadfLowestIntervalQpOffset;            // [-63..63]
    int8_t         m_spsLadfQpOffset[4];                       // [-63..63]
    uint16_t       m_spsLadfDeltaThresholdMinus1[4];           // [0..1021]
    uint8_t        m_spsNumVerVirtualBoundaries;               // [0..3]
    uint8_t        m_spsNumHorVirtualBoundaries;               // [0..3]
    uint8_t        m_spsLog2DiffMinQtMinCbIntraSliceLuma;      // [0..4]
    uint8_t        m_spsMaxMttHierarchyDepthIntraSliceLuma;    // [0..10]
    uint16_t       m_spsVirtualBoundaryPosXMinus1[3];          // [0..2109]
    uint16_t       m_spsVirtualBoundaryPosYMinus1[3];          // [0..2109]
    uint8_t        m_spsLog2DiffMaxBtMinQtIntraSliceLuma;      // [0..5]
    uint8_t        m_spsLog2DiffMaxTtMinQtIntraSliceLuma;      // [0..4]
    uint8_t        m_spsLog2DiffMinQtMinCbIntraSliceChroma;    // [0..4]
    uint8_t        m_spsMaxMttHierarchyDepthIntraSliceChroma;  // [0..10]
    uint8_t        m_spsLog2DiffMaxBtMinQtIntraSliceChroma;    // [0..4]
    uint8_t        m_spsLog2DiffMaxTtMinQtIntraSliceChroma;    // [0..4]
    uint8_t        m_spsLog2DiffMinQtMinCbInterSlice;          // [0..4]
    uint8_t        m_spsMaxMttHierarchyDepthInterSlice;        // [0..10]
    uint8_t        m_spsLog2DiffMaxBtMinQtInterSlice;          // [0..5]
    uint8_t        m_spsLog2DiffMaxTtMinQtInterSlice;          // [0..4]
    uint32_t       m_reserved32b0;

    union
    {
        struct
        {
            uint32_t        m_spsSubpicInfoPresentFlag                       : 1;    // [0..1]
            uint32_t        m_spsIndependentSubpicsFlag                      : 1;    // [0..1]
            uint32_t        m_spsSubpicSameSizeFlag                          : 1;    // [0..1]
            uint32_t        m_spsEntropyCodingSyncEnabledFlag                : 1;    // [0..1]
            uint32_t        m_spsEntryPointOffsetsPresentFlag                : 1;    // [0..1]
            uint32_t        m_spsPocMsbCycleFlag                             : 1;    // [0..1]
            uint32_t        m_spsPartitionConstraintsOverrideEnabledFlag     : 1;    // [0..1]
            uint32_t        m_spsQtbttDualTreeIntraFlag                      : 1;    // [0..1]
            uint32_t        m_spsMaxLumaTransformSize64Flag                  : 1;    // [0..1]
            uint32_t        m_spsTransformSkipEnabledFlag                    : 1;    // [0..1]
            uint32_t        m_spsBdpcmEnabledFlag                            : 1;    // [0..1]
            uint32_t        m_spsMtsEnabledFlag                              : 1;    // [0..1]
            uint32_t        m_spsExplicitMtsIntraEnabledFlag                 : 1;    // [0..1]
            uint32_t        m_spsExplicitMtsInterEnabledFlag                 : 1;    // [0..1]
            uint32_t        m_spsLfnstEnabledFlag                            : 1;    // [0..1]
            uint32_t        m_spsJointCbcrEnabledFlag                        : 1;    // [0..1]
            uint32_t        m_spsSameQpTableForChromaFlag                    : 1;    // [0..1]
            uint32_t        m_spsSaoEnabledFlag                              : 1;    // [0..1]
            uint32_t        m_spsAlfEnabledFlag                              : 1;    // [0..1]
            uint32_t        m_spsCcalfEnabledFlag                            : 1;    // [0..1]
            uint32_t        m_spsLmcsEnabledFlag                             : 1;    // [0..1]
            uint32_t        m_spsLongTermRefPicsFlag                         : 1;    // [0..1]
            uint32_t        m_spsInterLayerPredictionEnabledFlag             : 1;    // [0..1]
            uint32_t        m_spsIdrRplPresentFlag                           : 1;    // [0..1]
            uint32_t        m_reservedBits                                   : 8;    // [0]
        } m_fields;
        uint32_t     m_value;
    } m_spsFlags0;

    union
    {
        struct
        {
            uint32_t        m_spsTemporalMvpEnabledFlag         : 1;     // [0..1]
            uint32_t        m_spsSbtmvpEnabledFlag              : 1;     // [0..1]
            uint32_t        m_spsAmvrEnabledFlag                : 1;     // [0..1]
            uint32_t        m_spsBdofEnabledFlag                : 1;     // [0..1]
            uint32_t        m_spsBdofControlPresentInPhFlag     : 1;     // [0..1]
            uint32_t        m_spsSmvdEnabledFlag                : 1;     // [0..1]
            uint32_t        m_spsDmvrEnabledFlag                : 1;     // [0..1]
            uint32_t        m_spsDmvrControlPresentInPhFlag     : 1;     // [0..1]
            uint32_t        m_spsMmvdEnabledFlag                : 1;     // [0..1]
            uint32_t        m_spsMmvdFullpelOnlyEnabledFlag     : 1;     // [0..1]
            uint32_t        m_spsSbtEnabledFlag                 : 1;     // [0..1]
            uint32_t        m_spsAffineEnabledFlag              : 1;     // [0..1]
            uint32_t        m_sps6paramAffineEnabledFlag        : 1;     // [0..1]
            uint32_t        m_spsAffineAmvrEnabledFlag          : 1;     // [0..1]
            uint32_t        m_spsAffineProfEnabledFlag          : 1;     // [0..1]
            uint32_t        m_spsProfControlPresentInPhFlag     : 1;     // [0..1]
            uint32_t        m_spsBcwEnabledFlag                 : 1;     // [0..1]
            uint32_t        m_spsCiipEnabledFlag                : 1;     // [0..1]
            uint32_t        m_spsGpmEnabledFlag                 : 1;     // [0..1]
            uint32_t        m_spsIspEnabledFlag                 : 1;     // [0..1]
            uint32_t        m_spsMrlEnabledFlag                 : 1;     // [0..1]
            uint32_t        m_spsMipEnabledFlag                 : 1;     // [0..1]
            uint32_t        m_spsCclmEnabledFlag                : 1;     // [0..1]
            uint32_t        m_spsChromaHorizontalCollocatedFlag : 1;     // [0..1]
            uint32_t        m_spsChromaVerticalCollocatedFlag   : 1;     // [0..1]

            uint32_t        m_reservedBits                      : 7;    // [0]
        } m_fields;
        uint32_t     m_value;
    } m_spsFlags1;

    union
    {
        struct
        {
            uint32_t        m_spsPaletteEnabledFlag                                     : 1;     // [0..1]
            uint32_t        m_spsActEnabledFlag                                         : 1;     // [0..1]
            uint32_t        m_spsIbcEnabledFlag                                         : 1;     // [0..1]
            uint32_t        m_spsLadfEnabledFlag                                        : 1;     // [0..1]
            uint32_t        m_spsExplicitScalingListEnabledFlag                         : 1;     // [0..1]
            uint32_t        m_spsScalingMatrixForLfnstDisabledFlag                      : 1;     // [0..1]
            uint32_t        m_spsScalingMatrixForAlternativeColourSpaceDisabledFlag     : 1;     // [0..1]
            uint32_t        m_spsScalingMatrixDesignatedColourSpaceFlag                 : 1;     // [0..1]
            uint32_t        m_spsDepQuantEnabledFlag                                    : 1;     // [0..1]
            uint32_t        m_spsSignDataHidingEnabledFlag                              : 1;     // [0..1]
            uint32_t        m_spsVirtualBoundariesEnabledFlag                           : 1;     // [0..1]
            uint32_t        m_spsVirtualBoundariesPresentFlag                           : 1;     // [0..1]
            uint32_t        m_spsWeightedPredFlag                                       : 1;     // [0..1]
            uint32_t        m_spsWeightedBipredFlag                                     : 1;     // [0..1]
            uint32_t        m_reservedBits                                              : 18;    // [0]
        } m_fields;
        uint32_t     m_value;
    } m_spsFlags2;

    // PPS info
    uint16_t        m_ppsPicWidthInLumaSamples;             // [8..16888]
    uint16_t        m_ppsPicHeightInLumaSamples;            // [8..16888]
    uint8_t         m_numVerVirtualBoundaries;              // [0..3]
    uint8_t         m_numHorVirtualBoundaries;              // [0..3]
    uint16_t        m_virtualBoundaryPosX[3];               // [0..16880]
    uint16_t        m_virtualBoundaryPosY[3];               // [0..16880]

    int32_t         m_ppsScalingWinLeftOffset;
    int32_t         m_ppsScalingWinRightOffset;
    int32_t         m_ppsScalingWinTopOffset;
    int32_t         m_ppsScalingWinBottomOffset;

    uint8_t         m_ppsNumExpTileColumnsMinus1;            // [0..19]
    uint16_t        m_ppsNumExpTileRowsMinus1;               // [0..439]
    uint16_t        m_ppsNumSlicesInPicMinus1;               // [0..599]
    uint8_t         m_ppsNumRefIdxDefaultActiveMinus1[2];    // [0..14]
    uint16_t        m_ppsPicWidthMinusWraparoundOffset;      // [0..4188]
    int8_t          m_ppsInitQpMinus26;                      // [-38..37]
    int8_t          m_ppsCbQpOffset;                         // [-12..12]
    int8_t          m_ppsCrQpOffset;                         // [-12..12]
    int8_t          m_ppsJointCbcrQpOffsetValue;             // [-12..12]
    uint8_t         m_ppsChromaQpOffsetListLenMinus1;        // [0..5]
    int8_t          m_ppsCbQpOffsetList[ 6 ];                // [-12..12]
    int8_t          m_ppsCrQpOffsetList[ 6 ];                // [-12..12]
    int8_t          m_ppsJointCbcrQpOffsetList[ 6 ];         // [-12..12]
    int8_t          m_ppsLumaBetaOffsetDiv2;                 // [-12..12]
    int8_t          m_ppsLumaTcOffsetDiv2;                   // [-12..12]
    int8_t          m_ppsCbBetaOffsetDiv2;                   // [-12..12]
    int8_t          m_ppsCbTcOffsetDiv2;                     // [-12..12]
    int8_t          m_ppsCrBetaOffsetDiv2;                   // [-12..12]
    int8_t          m_ppsCrTcOffsetDiv2;                     // [-12..12]
    uint16_t        m_reserved16b;                           // [0]
    uint8_t         m_numScalingMatrixBuffers;               // [0..8]
    uint8_t         m_numAlfBuffers;                         // [0..8]
    uint8_t         m_numLmcsBuffers;                        // [0..4]
    uint8_t         m_numRefPicListStructs;                  // [0..128]
    uint16_t        m_numSliceStructsMinus1;                 // [0..599]

    union
    {
        struct
        {
            uint32_t        m_ppsOutputFlagPresentFlag                  : 1;     // [0..1]
            uint32_t        m_ppsLoopFilterAcrossTilesEnabledFlag       : 1;     // [0..1]
            uint32_t        m_ppsRectSliceFlag                          : 1;     // [0..1]
            uint32_t        m_ppsSingleSlicePerSubpicFlag               : 1;     // [0..1]
            uint32_t        m_ppsLoopFilterAcrossSlicesEnabledFlag      : 1;     // [0..1]
            uint32_t        m_ppsCabacInitPresentFlag                   : 1;     // [0..1]
            uint32_t        m_ppsRpl1IdxPresentFlag                     : 1;     // [0..1]
            uint32_t        m_ppsWeightedPredFlag                       : 1;     // [0..1]
            uint32_t        m_ppsWeightedBipredFlag                     : 1;     // [0..1]
            uint32_t        m_ppsRefWraparoundEnabledFlag               : 1;     // [0..1]
            uint32_t        m_ppsCuQpDeltaEnabledFlag                   : 1;     // [0..1]
            uint32_t        m_ppsChroma_toolOffsetsPresentFlag          : 1;     // [0..1]
            uint32_t        m_ppsSliceChromaQpOffsetsPresentFlag        : 1;     // [0..1]
            uint32_t        m_ppsCuChromaQpOffsetListEnabledFlag        : 1;     // [0..1]
            uint32_t        m_ppsDeblockingFilterOverrideEnabledFlag    : 1;     // [0..1]
            uint32_t        m_ppsDeblockingFilterDisabledFlag           : 1;     // [0..1]
            uint32_t        m_ppsDbfInfoInPhFlag                        : 1;     // [0..1]
            uint32_t        m_ppsRplInfoInPhFlag                        : 1;     // [0..1]
            uint32_t        m_ppsSaoInfoInPhFlag                        : 1;     // [0..1]
            uint32_t        m_ppsAlfInfoInPhFlag                        : 1;     // [0..1]
            uint32_t        m_ppsWpInfoInPhFlag                         : 1;     // [0..1]
            uint32_t        m_ppsQpDeltaInfoInPhFlag                    : 1;     // [0..1]
            uint32_t        m_ppsPictureHeaderExtensionPresentFlag      : 1;     // [0..1]
            uint32_t        m_ppsSliceHeaderExtensionPresentFlag        : 1;     // [0..1]
            uint32_t        m_reservedBits                              : 8;     // [0]
        } m_fields;
        uint32_t         m_value;
    } m_ppsFlags;

    // PH info
    uint8_t        m_phNumAlfApsIdsLuma;                        // [0..7]
    uint8_t        m_phAlfApsIdLuma[7];                         // [0..7]
    uint8_t        m_phAlfApsIdChroma;                          // [0..7]
    uint8_t        m_phAlfCcCbApsId;                            // [0..7]
    uint8_t        m_phAlfCcCrApsId;                            // [0..7]
    uint8_t        m_phLmcsApsId;                               // [0..3]
    uint8_t        m_phScalingListApsId;                        // [0..7]
    uint8_t        m_phLog2DiffMinQtMinCbIntraSliceLuma;        // [0..4]
    uint8_t        m_phMaxMtt_hierarchyDepthIntraSliceLuma;     // [0..10]
    uint8_t        m_phLog2DiffMaxBtMinQtIntraSliceLuma;        // [0..5]
    uint8_t        m_phLog2DiffMax_ttMinQtIntraSliceLuma;       // [0..4]
    uint8_t        m_phLog2DiffMinQtMinCbIntraSliceChroma;      // [0..4]
    uint8_t        m_phMaxMtt_hierarchyDepthIntraSliceChroma;   // [0..10]
    uint8_t        m_phLog2DiffMaxBtMinQtIntraSliceChroma;      // [0..4]
    uint8_t        m_phLog2DiffMax_ttMinQtIntraSliceChroma;     // [0..4]
    uint8_t        m_phCuQpDeltaSubdivIntraSlice;               // [0..30]
    uint8_t        m_phCuChromaQpOffsetSubdivIntraSlice;        // [0..30]
    uint8_t        m_phLog2DiffMinQtMinCbInterSlice;            // [0..4]
    uint8_t        m_phMaxMtt_hierarchyDepthInterSlice;         // [0..10]
    uint8_t        m_phLog2DiffMaxBtMinQtInterSlice;            // [0..5]
    uint8_t        m_phLog2DiffMax_ttMinQtInterSlice;           // [0..4]
    uint8_t        m_phCuQpDeltaSubdivInterSlice;               // [0..30]
    uint8_t        m_phCuChromaQpOffsetSubdivInterSlice;        // [0..30]
    uint8_t        m_phCollocatedRefIdx;                        // [0..14]
    int8_t         m_phQpDelta;                                 // [-75..75]
    int8_t         m_phLumaBetaOffsetDiv2;                      // [-12..12]
    int8_t         m_phLumaTcOffsetDiv2;                        // [-12..12]
    int8_t         m_phCbBetaOffsetDiv2;                        // [-12..12]
    int8_t         m_phCbTcOffsetDiv2;                          // [-12..12]
    int8_t         m_phCrBetaOffsetDiv2;                        // [-12..12]
    int8_t         m_phCrTcOffsetDiv2;                          // [-12..12]

    // weighted prediction info
    VvcWeightedPredInfo    m_wpInfo;

    union
    {
        struct
        {
            uint32_t        m_phNonRefPicFlag                       : 1;     // [0..1]
            uint32_t        m_phInterSliceAllowedFlag               : 1;     // [0..1]
            uint32_t        m_phAlfEnabledFlag                      : 1;     // [0..1]
            uint32_t        m_phAlfCbEnabledFlag                    : 1;     // [0..1]
            uint32_t        m_phAlfCrEnabledFlag                    : 1;     // [0..1]
            uint32_t        m_phAlfCcCbEnabledFlag                  : 1;     // [0..1]
            uint32_t        m_phAlfCcCrEnabledFlag                  : 1;     // [0..1]
            uint32_t        m_phLmcsEnabledFlag                     : 1;     // [0..1]
            uint32_t        m_phChromaResidualScaleFlag             : 1;     // [0..1]
            uint32_t        m_phExplicitScalingListEnabledFlag      : 1;     // [0..1]
            uint32_t        m_phVirtualBoundariesPresentFlag        : 1;     // [0..1]
            uint32_t        m_reserved1b                            : 1;     // [0..1]
            uint32_t        m_phTemporalMvpEnabledFlag              : 1;     // [0..1]
            uint32_t        m_numRefEntries0RplIdx0LargerThan0      : 1;     // [0..1]
            uint32_t        m_numRefEntries1RplIdx1LargerThan0      : 1;     // [0..1]
            uint32_t        m_phCollocatedFromL0Flag                : 1;     // [0..1]
            uint32_t        m_phMmvdFullpelOnlyFlag                 : 1;     // [0..1]
            uint32_t        m_phMvdL1ZeroFlag                       : 1;     // [0..1]
            uint32_t        m_phBdofDisabledFlag                    : 1;     // [0..1]
            uint32_t        m_phDmvrDisabledFlag                    : 1;     // [0..1]
            uint32_t        m_phProfDisabledFlag                    : 1;     // [0..1]
            uint32_t        m_phJointCbcrSignFlag                   : 1;     // [0..1]
            uint32_t        m_phSaoLumaEnabledFlag                  : 1;     // [0..1]
            uint32_t        m_phSaoChromaEnabledFlag                : 1;     // [0..1]
            uint32_t        m_phDeblockingFilterDisabledFlag        : 1;     // [0..1]
            uint32_t        m_rplSpsFlag0                           : 1;     // [0..1]
            uint32_t        m_rplSpsFlag1                           : 1;     // [0..1]
            uint32_t        m_reservedBits                          : 5;     // [0]
        } m_fields;
        uint32_t     m_value;
    } m_phFlags;

    // reference lists
    CODEC_PICTURE                m_currPic;
    int32_t                      m_picOrderCntVal;                          // [-2^31 .. 2^31-1]
    int32_t                      m_refFramePocList [vvcMaxNumRefFrame];     // [-2^31 .. 2^31-1]
    CODEC_PICTURE                m_refFrameList [vvcMaxNumRefFrame];
    CODEC_PICTURE                m_refPicList [2][vvcMaxNumRefFrame];

    union
    {
        struct
        {
            uint32_t        m_intraPicFlag                    : 1;          // [0..1]
            uint32_t        m_reservedBits3                   : 31;
        } m_fields;
        uint32_t    m_value;
    } m_picMiscFlags;

    uint32_t        m_statusReportFeedbackNumber;
    uint32_t        m_rplSpsIndex0;
    uint32_t        m_rplSpsIndex1;
    uint32_t        m_reserved32b[16];

    // MultiLayer params
    uint8_t         m_refFrameListNuhLayerId[15];           // [0..55]
    uint8_t         m_nuhLayerId;                           // [0..55]
    uint8_t         m_vpsMaxLayersMinus1;                   // [0..55]
    uint8_t         m_reserved[3];
    uint8_t         m_vpsLayerId[56];                       // [0..55]
    uint8_t         m_vpsDirectRefLayerFlag[56][7];
    uint32_t        m_multilayerReserved32b[16];            // [0]
};

//!
//! \struct CodecVvcAlfData
//! \brief Define the ALF data for VVC
//!
struct CodecVvcAlfData
{
    uint8_t        m_apsAdaptationParameterSetId;          // [0..7]
    uint8_t        m_alfLumaNumFiltersSignalledMinus1;     // [0..24]
    uint8_t        m_alfLumaCoeffDeltaIdx[25];             // [0..24]
    int8_t         m_alfCoeffL[25][12];                    // [-128..127]
    uint8_t        m_alfLumaClipIdx[25][12];               // [0..3]
    uint8_t        m_alfChromaNumAltFiltersMinus1;         // [0..7]
    int8_t         m_alfCoeffC[8][6];                      // [-128..127]
    uint8_t        m_alfChromaClipIdx[8][6];               // [0..3]
    uint8_t        m_alfCcCbFiltersSignalledMinus1;        // [0..3]
    int8_t         m_ccAlfApsCoeffCb[4][7];                // [-64..64]
    uint8_t        m_alfCcCrFiltersSignalledMinus1;        // [0..3]
    int8_t         m_ccAlfApsCoeffCr[4][7];                // [-64..64]

    union
    {
        struct
        {
            uint32_t        m_alfLumaFilterSignalFlag        : 1;    // [0..1]
            uint32_t        m_alfChromaFilterSignalFlag      : 1;    // [0..1]
            uint32_t        m_alfCcCbFilterSignalFlag        : 1;    // [0..1]
            uint32_t        m_alfCcCrFilterSignalFlag        : 1;    // [0..1]
            uint32_t        m_alfLumaClipFlag                : 1;    // [0..1]
            uint32_t        m_alfChromaClipFlag              : 1;    // [0..1]
            uint32_t        m_reservedBits                   : 26;   // [0]
        } m_fields;
        uint32_t     m_value;
    } m_alfFlags;

    uint32_t        m_reserved32b[8];     // [0]
};

//!
//! \struct CodecVvcLmcsData
//! \brief Define the LMCS data for VVC
//!
struct CodecVvcLmcsData
{
    uint8_t        m_apsAdaptationParameterSetId;      // [0..7]
    uint8_t        m_reserved8b;                       // [0]
    uint8_t        m_lmcsMinBinIdx;                    // [0..15]
    uint8_t        m_lmcsDeltaMaxBinIdx;               // [0..15]
    int16_t        m_lmcsDeltaCW[16];                  // [-32767..32767]
    int8_t         m_lmcsDeltaCrs;                     // [-7..7]
    uint32_t       m_reserved32b[8];                   // [0]
};

//!
//! \struct CodecVvcQmData
//! \brief Define the Quantization Matrix data for VVC
//!
struct CodecVvcQmData
{
    uint8_t        m_apsAdaptationParameterSetId;       // [0..7]
    uint8_t        m_reserved8b;
    uint8_t        m_scalingMatrixDCRec[14];            // [1..255]
    uint8_t        m_scalingMatrixRec2x2[2][2][2];      // [1..255]
    uint8_t        m_scalingMatrixRec4x4[6][4][4];      // [1..255]
    uint8_t        m_scalingMatrixRec8x8[20][8][8];     // [1..255]
    uint32_t       m_reserved32b[8];
};

//!
//! \struct CodecVvcTileParam
//! \brief Define the Tile Parameter for VVC
//!
struct CodecVvcTileParam
{
    uint16_t  m_tileDimension;                         // [0..262]
};

//!
//! \struct CodecVvcSubpicParam
//! \brief Define the SubPic Parameter for VVC
//!
struct CodecVvcSubpicParam
{
    uint16_t        m_spsSubpicCtuTopLeftX;            // [0..526]
    uint16_t        m_spsSubpicCtuTopLeftY;            // [0..526]
    uint16_t        m_spsSubpicWidthMinus1;            // [0..511]
    uint16_t        m_spsSubpicHeightMinus1;           // [0..511]
    uint16_t        m_subpicIdVal;                     // [0..2^16-1]

    union
    {
        struct
        {
            uint16_t    m_spsSubpicTreatedAsPicFlag                 : 1;     // [0..1]
            uint16_t    m_spsLoopFilterAcrossSubpicEnabledFlag      : 1;     // [0..1]
            uint16_t    m_reserved14bits                            : 14;    // [0]
        } m_fields;
        uint16_t        m_value;
    } m_subPicFlags;

    //Additional params to reconstruct partition for Rect slice mode and none-single slice per subpic case
    uint16_t        m_endCtbX;      //SubPic range right border in CTU column
    uint16_t        m_endCtbY;      //SubPic range bottom border in CTU row
    int16_t         m_numSlices;    //Accumulated slice number in current subpic
    uint16_t        *m_sliceIdx;    //Pointer to an array of slice index in scan order in picture
};

//!
//! \struct CodecVvcSliceStructure
//! \brief Define the Slice Structure for VVC
//!
struct CodecVvcSliceStructure
{
    uint16_t        m_sliceTopLeftTileIdx;              // [0..439]
    uint16_t        m_ppsSliceWidthInTilesMinus1;       // [0..19]
    uint16_t        m_ppsSliceHeightInTilesMinus1;      // [0..439]
    uint16_t        m_ppsExpSliceHeightInCtusMinus1;    // [0..526]
    uint16_t        m_reserved16b;                      // [0]
    uint32_t        m_reserved32b[4];                   // [0]
};

//!
//! \struct CodecVvcRplStructure
//! \brief Define the RPL Structure for VVC
//!
struct CodecVvcRplStructure
{
    uint8_t        m_listIdx;                  // [0..1]
    uint8_t        m_rplsIdx;                  // [0.. spsNumRefPicLists[listIdx]]
    uint8_t        m_numRefEntries;            // [0..29]
    uint8_t        m_ltrpInHeaderFlag;         // [0..1]
    uint8_t        m_stRefPicFlag[29];         // [0..1]
    uint8_t        m_reserved8b[3];            // [0]
    int16_t        m_deltaPocSt[29];           // [-2^15..2^15 - 1]
    uint16_t       m_rplsPocLsbLt[29];         // [0..2^16-1]
    uint8_t        m_interLayerRefPicFlag[29]; // [0..1]
    uint8_t        m_ilrpIdx[29];              // [0..55]
    uint16_t       m_reserved16b;              // [0]
    uint32_t       m_reserved32b[8];           // [0]  leave space for multilayer enabling
};

//!
//! \struct CodecVvcSliceParams
//! \brief Define the Slice Parameters for VVC
//!
struct CodecVvcSliceParams
{
    uint32_t    m_bSNALunitDataLocation;
    uint32_t    m_sliceBytesInBuffer;
    uint16_t    m_wBadSliceChopping;

    uint16_t      m_shSubpicId;                        // [0..65535]
    uint16_t      m_shSliceAddress;                    // [0..599]
    uint16_t      m_shNumTilesInSliceMinus1;           // [0..439]
    uint8_t       m_shSliceType;                       // [0..2]
    uint8_t       m_shNumAlfApsIdsLuma;                // [0..7]
    uint8_t       m_shAlfApsIdLuma[ 7 ];               // [0..7]
    uint8_t       m_shAlfApsIdChroma;                  // [0..7]
    uint8_t       m_shAlfCcCbApsId;                    // [0..7]
    uint8_t       m_shAlfCcCrApsId;                    // [0..7]
    int8_t        m_numRefIdxActive[ 2 ];              // [0..14]
    uint8_t       m_shCollocatedRefIdx;                // [0..14]
    int8_t        m_sliceQpY;                          // [-12..63]
    int8_t        m_shCbQpOffset;                      // [-12..12]
    int8_t        m_shCrQpOffset;                      // [-12..12]
    int8_t        m_shJointCbcrQpOffset;               // [-12..12]
    int8_t        m_shLumaBetaOffsetDiv2;              // [-12..12]
    int8_t        m_shLumaTcOffsetDiv2;                // [-12..12]
    int8_t        m_shCbBetaOffsetDiv2;                // [-12..12]
    int8_t        m_shCbTcOffsetDiv2;                  // [-12..12]
    int8_t        m_shCrBetaOffsetDiv2;                // [-12..12]
    int8_t        m_shCrTcOffsetDiv2;                  // [-12..12]

    uint32_t      m_byteOffsetToSliceData;
    uint16_t      m_numEntryPoints;                    // [0..512]

    CODEC_PICTURE            m_refPicList[2][15];
    VvcWeightedPredInfo      m_wpInfo;

    union
    {
        struct
        {
            uint32_t        m_shAlfEnabledFlag                  : 1;    // [0..1]
            uint32_t        m_shAlfCbEnabledFlag                : 1;    // [0..1]
            uint32_t        m_shAlfCrEnabledFlag                : 1;    // [0..1]
            uint32_t        m_shAlfCcCbEnabledFlag              : 1;    // [0..1]
            uint32_t        m_shAlfCcCrEnabledFlag              : 1;    // [0..1]
            uint32_t        m_shLmcsUsedFlag                    : 1;    // [0..1]
            uint32_t        m_shExplicitScalingListUsedFlag     : 1;    // [0..1]
            uint32_t        m_shCabacInitFlag                   : 1;    // [0..1]
            uint32_t        m_shCollocatedFromL0Flag            : 1;    // [0..1]
            uint32_t        m_shCuChromaQpOffsetEnabledFlag     : 1;    // [0..1]
            uint32_t        m_shSaoLumaUsedFlag                 : 1;    // [0..1]
            uint32_t        m_shSaoChromaUsedFlag               : 1;    // [0..1]
            uint32_t        m_shDeblockingFilterDisabledFlag    : 1;    // [0..1]
            uint32_t        m_shDepQuantUsedFlag                : 1;    // [0..1]
            uint32_t        m_shSignDataHidingUsedFlag          : 1;    // [0..1]
            uint32_t        m_shTsResidualCodingDisabledFlag    : 1;    // [0..1]
            uint32_t        m_lastSliceOfPic                    : 1;    // [0..1]
            uint32_t        m_noBackwardPredFlag                : 1;    // [0..1]
            uint32_t        m_reserved                          : 15;   // [0]
        } m_fields;
        uint32_t        m_value;
    } m_longSliceFlags;

    uint32_t        m_reserved32b[8];
};

#endif  // __CODEC_DEF_DECODE_VVC_H__

