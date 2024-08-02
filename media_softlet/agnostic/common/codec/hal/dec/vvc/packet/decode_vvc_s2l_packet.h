/*
* Copyright (c) 2022-2023, Intel Corporation
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
//! \file     decode_vvc_s2l_packet.h
//! \brief    Defines the implementation of VVC decode S2L packet
//!

#ifndef __DECODE_VVC_S2L_PACKET_H__
#define __DECODE_VVC_S2L_PACKET_H__

#include "media_cmd_packet.h"
#include "decode_huc.h"
#include "media_pipeline.h"
#include "codec_hw_next.h"
#include "decode_utils.h"
#include "decode_vvc_pipeline.h"
#include "decode_vvc_basic_feature.h"

#include "mhw_vdbox_huc_cmdpar.h"
#include "mhw_vdbox_huc_itf.h"
#include "mhw_cmdpar.h"
#include "decode_vvc_packet.h"

namespace decode
{

enum DMAReadWriteStatus
{
    DMAReadWriteDisabled = 0,
    DMAReadWriteEnabled  = 1,
};

struct VvcS2lSliceBsParam
{
    uint32_t BSNALunitDataLocation;
    uint32_t SliceBytesInBuffer;

    struct
    {
        uint32_t reserve_0;
        uint32_t reserve_1;
        uint32_t reserve_2;
        uint32_t reserve_3;
    } reserve;
};

struct VvcS2lSliceBbParam
{
    uint32_t SliceCmdBatchOffset;
};
struct PicHeaderRplParam
{
    uint8_t       numRefForList0 = 0;
    uint8_t       numRefForList1 = 0;
    uint8_t       refPicListFrameIdx[2][vvcMaxNumRefFrame]       = {};
    uint16_t      refPicListFrameFlag[2][vvcMaxNumRefFrame]      = {};
    uint8_t       stRefPicFlag[2][vvcMaxNumRefFrame]             = {};
    uint8_t       rprConstraintsActiveFlag[2][vvcMaxNumRefFrame] = {};
    uint8_t       unavailableRefPic[2][vvcMaxNumRefFrame]        = {};
    int16_t       diffPicOrderCnt[2][vvcMaxNumRefFrame]          = {};
};

struct VvcS2lSubpicParam
{
    uint16_t m_spsSubpicCtuTopLeftX;   // [0..526]
    uint16_t m_spsSubpicCtuTopLeftY;   // [0..526]
    uint16_t m_spsSubpicWidthMinus1;   // [0..511]
    uint16_t m_spsSubpicHeightMinus1;  // [0..511]
    uint16_t m_subpicIdVal;            // [0..2^16-1]

    union
    {
        struct
        {
            uint16_t m_spsSubpicTreatedAsPicFlag : 1;             // [0..1]
            uint16_t m_spsLoopFilterAcrossSubpicEnabledFlag : 1;  // [0..1]
            uint16_t m_reserved14bits : 14;                       // [0]
        } m_fields;
        uint16_t m_value;
    } m_subPicFlags;

    //Additional params to reconstruct partition for Rect slice mode and none-single slice per subpic case
    uint16_t m_endCtbX;    //SubPic range right border in CTU column
    uint16_t m_endCtbY;    //SubPic range bottom border in CTU row
    int16_t  m_numSlices;  //Accumulated slice number in current subpic
    //uint16_t        *m_sliceIdx;    //Pointer to an array of slice index in scan order in picture
}; 

struct HucVvcS2lPicBss
{
    // SPS info
    uint16_t m_spsPicWidthMaxInLumaSamples;   // [8..16888]
    uint16_t m_spsPicHeightMaxInLumaSamples;  // [8..16888]

    uint16_t m_spsNumSubpicsMinus1;                  // [0..599]
    uint8_t  m_spsSubpicIdLenMinus1;                 // [0..15]
    uint8_t  m_spsChromaFormatIdc;                   // [1]
    uint8_t  m_spsBitdepthMinus8;                    // [0..2]
    uint8_t  m_spsLog2CtuSizeMinus5;                 // [0..2]
    uint8_t  m_spsLog2MaxPicOrderCntLsbMinus4;       // [0..12]
    uint8_t  m_spsLog2MinLumaCodingBlockSizeMinus2;  // [0..4]
    uint8_t  m_spsPocMsbCycleLenMinus1;              // [0..27]
    uint8_t  m_numExtraPhBits;                       // [0..15]
    uint8_t  m_numExtraShBits;                       // [0..15]
    uint8_t  m_spsLog2TransformSkipMaxSizeMinus2;    // [0..3]

    int8_t m_chromaQpTable[3][76];  // [-12..63]

    uint8_t  m_spsNumRefPicLists[2];                     // [0..64]
    uint8_t  m_spsSixMinusMaxNumMergeCand;               // [0..5]
    uint8_t  m_spsFiveMinusMaxNumSubblockMergeCand;      // [0..5]
    uint8_t  m_spsMaxNumMergeCandMinusMaxNumGpmCand;     // [0..4]
    uint8_t  m_spsLog2ParallelMergeLevelMinus2;          // [0..5]
    uint8_t  m_spsMinQpPrimeTs;                          // [0..8]
    uint8_t  m_spsSixMinusMaxNumIbcMergeCand;            // [0..5]
    uint8_t  m_spsNumLadfIntervalsMinus2;                // [0..3]
    int8_t   m_spsLadfLowestIntervalQpOffset;            // [-63..63]
    int8_t   m_spsLadfQpOffset[4];                       // [-63..63]
    uint16_t m_spsLadfDeltaThresholdMinus1[4];           // [0..1021]
    uint8_t  m_spsNumVerVirtualBoundaries;               // [0..3]
    uint8_t  m_spsNumHorVirtualBoundaries;               // [0..3]
    uint8_t  m_spsLog2DiffMinQtMinCbIntraSliceLuma;      // [0..4]
    uint8_t  m_spsMaxMttHierarchyDepthIntraSliceLuma;    // [0..10]
    uint16_t m_spsVirtualBoundaryPosXMinus1[3];          // [0..2109]
    uint16_t m_spsVirtualBoundaryPosYMinus1[3];          // [0..2109]
    uint8_t  m_spsLog2DiffMaxBtMinQtIntraSliceLuma;      // [0..5]
    uint8_t  m_spsLog2DiffMaxTtMinQtIntraSliceLuma;      // [0..4]
    uint8_t  m_spsLog2DiffMinQtMinCbIntraSliceChroma;    // [0..4]
    uint8_t  m_spsMaxMttHierarchyDepthIntraSliceChroma;  // [0..10]
    uint8_t  m_spsLog2DiffMaxBtMinQtIntraSliceChroma;    // [0..4]
    uint8_t  m_spsLog2DiffMaxTtMinQtIntraSliceChroma;    // [0..4]
    uint8_t  m_spsLog2DiffMinQtMinCbInterSlice;          // [0..4]
    uint8_t  m_spsMaxMttHierarchyDepthInterSlice;        // [0..10]
    uint8_t  m_spsLog2DiffMaxBtMinQtInterSlice;          // [0..5]
    uint8_t  m_spsLog2DiffMaxTtMinQtInterSlice;          // [0..4]
    union
    {
        struct
        {
            uint32_t m_spsSubpicInfoPresentFlag : 1;                    // [0..1]
            uint32_t m_spsIndependentSubpicsFlag : 1;                   // [0..1]
            uint32_t m_spsSubpicSameSizeFlag : 1;                       // [0..1]
            uint32_t m_spsEntropyCodingSyncEnabledFlag : 1;             // [0..1]
            uint32_t m_spsEntryPointOffsetsPresentFlag : 1;             // [0..1]
            uint32_t m_spsPocMsbCycleFlag : 1;                          // [0..1]
            uint32_t m_spsPartitionConstraintsOverrideEnabledFlag : 1;  // [0..1]
            uint32_t m_spsQtbttDualTreeIntraFlag : 1;                   // [0..1]
            uint32_t m_spsMaxLumaTransformSize64Flag : 1;               // [0..1]
            uint32_t m_spsTransformSkipEnabledFlag : 1;                 // [0..1]
            uint32_t m_spsBdpcmEnabledFlag : 1;                         // [0..1]
            uint32_t m_spsMtsEnabledFlag : 1;                           // [0..1]
            uint32_t m_spsExplicitMtsIntraEnabledFlag : 1;              // [0..1]
            uint32_t m_spsExplicitMtsInterEnabledFlag : 1;              // [0..1]
            uint32_t m_spsLfnstEnabledFlag : 1;                         // [0..1]
            uint32_t m_spsJointCbcrEnabledFlag : 1;                     // [0..1]
            uint32_t m_spsSameQpTableForChromaFlag : 1;                 // [0..1]
            uint32_t m_spsSaoEnabledFlag : 1;                           // [0..1]
            uint32_t m_spsAlfEnabledFlag : 1;                           // [0..1]
            uint32_t m_spsCcalfEnabledFlag : 1;                         // [0..1]
            uint32_t m_spsLmcsEnabledFlag : 1;                          // [0..1]
            uint32_t m_spsLongTermRefPicsFlag : 1;                      // [0..1]
            uint32_t m_spsInterLayerPredictionEnabledFlag : 1;          // [0..1]
            uint32_t m_spsIdrRplPresentFlag : 1;                        // [0..1]

            uint32_t m_spsTemporalMvpEnabledFlag : 1;          // [0..1]
            uint32_t m_spsSbtmvpEnabledFlag : 1;               // [0..1]
            uint32_t m_spsAmvrEnabledFlag : 1;                 // [0..1]
            uint32_t m_spsBdofEnabledFlag : 1;                 // [0..1]
            uint32_t m_spsBdofControlPresentInPhFlag : 1;      // [0..1]
            uint32_t m_spsSmvdEnabledFlag : 1;                 // [0..1]
            uint32_t m_spsDmvrEnabledFlag : 1;                 // [0..1]
            uint32_t m_spsDmvrControlPresentInPhFlag : 1;      // [0..1]
            uint32_t m_spsMmvdEnabledFlag : 1;                 // [0..1]
            uint32_t m_spsMmvdFullpelOnlyEnabledFlag : 1;      // [0..1]
            uint32_t m_spsSbtEnabledFlag : 1;                  // [0..1]
            uint32_t m_spsAffineEnabledFlag : 1;               // [0..1]
            uint32_t m_sps6paramAffineEnabledFlag : 1;         // [0..1]
            uint32_t m_spsAffineAmvrEnabledFlag : 1;           // [0..1]
            uint32_t m_spsAffineProfEnabledFlag : 1;           // [0..1]
            uint32_t m_spsProfControlPresentInPhFlag : 1;      // [0..1]
            uint32_t m_spsBcwEnabledFlag : 1;                  // [0..1]
            uint32_t m_spsCiipEnabledFlag : 1;                 // [0..1]
            uint32_t m_spsGpmEnabledFlag : 1;                  // [0..1]
            uint32_t m_spsIspEnabledFlag : 1;                  // [0..1]
            uint32_t m_spsMrlEnabledFlag : 1;                  // [0..1]
            uint32_t m_spsMipEnabledFlag : 1;                  // [0..1]
            uint32_t m_spsCclmEnabledFlag : 1;                 // [0..1]
            uint32_t m_spsChromaHorizontalCollocatedFlag : 1;  // [0..1]
            uint32_t m_spsChromaVerticalCollocatedFlag : 1;    // [0..1]

            uint32_t m_spsPaletteEnabledFlag : 1;                                  // [0..1]
            uint32_t m_spsActEnabledFlag : 1;                                      // [0..1]
            uint32_t m_spsIbcEnabledFlag : 1;                                      // [0..1]
            uint32_t m_spsLadfEnabledFlag : 1;                                     // [0..1]
            uint32_t m_spsExplicitScalingListEnabledFlag : 1;                      // [0..1]
            uint32_t m_spsScalingMatrixForLfnstDisabledFlag : 1;                   // [0..1]
            uint32_t m_spsScalingMatrixForAlternativeColourSpaceDisabledFlag : 1;  // [0..1]
            uint32_t m_spsScalingMatrixDesignatedColourSpaceFlag : 1;              // [0..1]
            uint32_t m_spsDepQuantEnabledFlag : 1;                                 // [0..1]
            uint32_t m_spsSignDataHidingEnabledFlag : 1;                           // [0..1]
            uint32_t m_spsVirtualBoundariesEnabledFlag : 1;                        // [0..1]
            uint32_t m_spsVirtualBoundariesPresentFlag : 1;                        // [0..1]
            uint32_t m_spsWeightedPredFlag : 1;                                    // [0..1]
            uint32_t m_spsWeightedBipredFlag : 1;                                  // [0..1]
            uint32_t m_spsExtensionFlag : 1;                                       // [0..1]
        };
        uint32_t value[2];
    }m_picSpsFlags;
    

    // PPS info
    uint16_t m_ppsPicWidthInLumaSamples;   // [8..16888]
    uint16_t m_ppsPicHeightInLumaSamples;  // [8..16888]
    uint8_t  m_numVerVirtualBoundaries;    // [0..3]
    uint8_t  m_numHorVirtualBoundaries;    // [0..3]
    uint16_t m_virtualBoundaryPosX[3];     // [0..16880]
    uint16_t m_virtualBoundaryPosY[3];     // [0..16880]

    int32_t m_ppsScalingWinLeftOffset;
    int32_t m_ppsScalingWinRightOffset;
    int32_t m_ppsScalingWinTopOffset;
    int32_t m_ppsScalingWinBottomOffset;

    uint8_t  m_ppsNumExpTileColumnsMinus1;          // [0..19]
    uint16_t m_ppsNumExpTileRowsMinus1;             // [0..439]
    uint16_t m_ppsNumSlicesInPicMinus1;             // [0..599]
    uint8_t  m_ppsNumRefIdxDefaultActiveMinus1[2];  // [0..14]
    uint16_t m_ppsPicWidthMinusWraparoundOffset;    // [0..4188]
    int8_t   m_ppsInitQpMinus26;                    // [-38..37]
    int8_t   m_ppsCbQpOffset;                       // [-12..12]
    int8_t   m_ppsCrQpOffset;                       // [-12..12]
    int8_t   m_ppsJointCbcrQpOffsetValue;           // [-12..12]
    uint8_t  m_ppsChromaQpOffsetListLenMinus1;      // [0..5]
    int8_t   m_ppsCbQpOffsetList[6];                // [-12..12]
    int8_t   m_ppsCrQpOffsetList[6];                // [-12..12]
    int8_t   m_ppsJointCbcrQpOffsetList[6];         // [-12..12]
    int8_t   m_ppsLumaBetaOffsetDiv2;               // [-12..12]
    int8_t   m_ppsLumaTcOffsetDiv2;                 // [-12..12]
    int8_t   m_ppsCbBetaOffsetDiv2;                 // [-12..12]
    int8_t   m_ppsCbTcOffsetDiv2;                   // [-12..12]
    int8_t   m_ppsCrBetaOffsetDiv2;                 // [-12..12]
    int8_t   m_ppsCrTcOffsetDiv2;                   // [-12..12]
    uint16_t m_reserved16b;                         // [0]
    uint8_t  m_numScalingMatrixBuffers;             // [0..8]
    uint8_t  m_numAlfBuffers;                       // [0..8]
    uint8_t  m_numLmcsBuffers;                      // [0..4]
    uint8_t  m_numRefPicListStructs;                // [0..128]
    uint16_t m_numSliceStructsMinus1;               // [0..599]

    union
    {
        struct
        {
            uint32_t m_ppsOutputFlagPresentFlag : 1;                // [0..1]
            uint32_t m_ppsLoopFilterAcrossTilesEnabledFlag : 1;     // [0..1]
            uint32_t m_ppsRectSliceFlag : 1;                        // [0..1]
            uint32_t m_ppsSingleSlicePerSubpicFlag : 1;             // [0..1]
            uint32_t m_ppsLoopFilterAcrossSlicesEnabledFlag : 1;    // [0..1]
            uint32_t m_ppsCabacInitPresentFlag : 1;                 // [0..1]
            uint32_t m_ppsRpl1IdxPresentFlag : 1;                   // [0..1]
            uint32_t m_ppsWeightedPredFlag : 1;                     // [0..1]
            uint32_t m_ppsWeightedBipredFlag : 1;                   // [0..1]
            uint32_t m_ppsRefWraparoundEnabledFlag : 1;             // [0..1]
            uint32_t m_ppsCuQpDeltaEnabledFlag : 1;                 // [0..1]
            uint32_t m_ppsChroma_toolOffsetsPresentFlag : 1;        // [0..1]
            uint32_t m_ppsSliceChromaQpOffsetsPresentFlag : 1;      // [0..1]
            uint32_t m_ppsCuChromaQpOffsetListEnabledFlag : 1;      // [0..1]
            uint32_t m_ppsDeblockingFilterOverrideEnabledFlag : 1;  // [0..1]
            uint32_t m_ppsDeblockingFilterDisabledFlag : 1;         // [0..1]
            uint32_t m_ppsDbfInfoInPhFlag : 1;                      // [0..1]
            uint32_t m_ppsRplInfoInPhFlag : 1;                      // [0..1]
            uint32_t m_ppsSaoInfoInPhFlag : 1;                      // [0..1]
            uint32_t m_ppsAlfInfoInPhFlag : 1;                      // [0..1]
            uint32_t m_ppsWpInfoInPhFlag : 1;                       // [0..1]
            uint32_t m_ppsQpDeltaInfoInPhFlag : 1;                  // [0..1]
            uint32_t m_ppsPictureHeaderExtensionPresentFlag : 1;    // [0..1]
            uint32_t m_ppsSliceHeaderExtensionPresentFlag : 1;      // [0..1]
        };
        uint32_t value;
    }m_picPpsFlags;

    // PH info
    uint8_t m_phNumAlfApsIdsLuma;                       // [0..7]
    uint8_t m_phAlfApsIdLuma[7];                        // [0..7]
    uint8_t m_phAlfApsIdChroma;                         // [0..7]
    uint8_t m_phAlfCcCbApsId;                           // [0..7]
    uint8_t m_phAlfCcCrApsId;                           // [0..7]
    uint8_t m_phLmcsApsId;                              // [0..3]
    uint8_t m_phScalingListApsId;                       // [0..7]
    uint8_t m_phLog2DiffMinQtMinCbIntraSliceLuma;       // [0..4]
    uint8_t m_phMaxMtt_hierarchyDepthIntraSliceLuma;    // [0..10]
    uint8_t m_phLog2DiffMaxBtMinQtIntraSliceLuma;       // [0..5]
    uint8_t m_phLog2DiffMax_ttMinQtIntraSliceLuma;      // [0..4]
    uint8_t m_phLog2DiffMinQtMinCbIntraSliceChroma;     // [0..4]
    uint8_t m_phMaxMtt_hierarchyDepthIntraSliceChroma;  // [0..10]
    uint8_t m_phLog2DiffMaxBtMinQtIntraSliceChroma;     // [0..4]
    uint8_t m_phLog2DiffMax_ttMinQtIntraSliceChroma;    // [0..4]
    uint8_t m_phCuQpDeltaSubdivIntraSlice;              // [0..30]
    uint8_t m_phCuChromaQpOffsetSubdivIntraSlice;       // [0..30]
    uint8_t m_phLog2DiffMinQtMinCbInterSlice;           // [0..4]
    uint8_t m_phMaxMtt_hierarchyDepthInterSlice;        // [0..10]
    uint8_t m_phLog2DiffMaxBtMinQtInterSlice;           // [0..5]
    uint8_t m_phLog2DiffMax_ttMinQtInterSlice;          // [0..4]
    uint8_t m_phCuQpDeltaSubdivInterSlice;              // [0..30]
    uint8_t m_phCuChromaQpOffsetSubdivInterSlice;       // [0..30]
    uint8_t m_phCollocatedRefIdx;                       // [0..14]
    int8_t  m_phQpDelta;                                // [-75..75]
    int8_t  m_phLumaBetaOffsetDiv2;                     // [-12..12]
    int8_t  m_phLumaTcOffsetDiv2;                       // [-12..12]
    int8_t  m_phCbBetaOffsetDiv2;                       // [-12..12]
    int8_t  m_phCbTcOffsetDiv2;                         // [-12..12]
    int8_t  m_phCrBetaOffsetDiv2;                       // [-12..12]
    int8_t  m_phCrTcOffsetDiv2;                         // [-12..12]

    union
    {
        struct
        {
            uint32_t m_phNonRefPicFlag : 1;                   // [0..1]
            uint32_t m_phInterSliceAllowedFlag : 1;           // [0..1]
            uint32_t m_phAlfEnabledFlag : 1;                  // [0..1]
            uint32_t m_phAlfCbEnabledFlag : 1;                // [0..1]
            uint32_t m_phAlfCrEnabledFlag : 1;                // [0..1]
            uint32_t m_phAlfCcCbEnabledFlag : 1;              // [0..1]
            uint32_t m_phAlfCcCrEnabledFlag : 1;              // [0..1]
            uint32_t m_phLmcsEnabledFlag : 1;                 // [0..1]
            uint32_t m_phChromaResidualScaleFlag : 1;         // [0..1]
            uint32_t m_phExplicitScalingListEnabledFlag : 1;  // [0..1]
            uint32_t m_phVirtualBoundariesPresentFlag : 1;    // [0..1]
            uint32_t m_phTemporalMvpEnabledFlag : 1;          // [0..1]
            uint32_t m_numRefEntries0RplIdx0LargerThan0 : 1;  // [0..1]
            uint32_t m_numRefEntries1RplIdx1LargerThan0 : 1;  // [0..1]
            uint32_t m_phCollocatedFromL0Flag : 1;            // [0..1]
            uint32_t m_phMmvdFullpelOnlyFlag : 1;             // [0..1]
            uint32_t m_phMvdL1ZeroFlag : 1;                   // [0..1]
            uint32_t m_phBdofDisabledFlag : 1;                // [0..1]
            uint32_t m_phDmvrDisabledFlag : 1;                // [0..1]
            uint32_t m_phProfDisabledFlag : 1;                // [0..1]
            uint32_t m_phJointCbcrSignFlag : 1;               // [0..1]
            uint32_t m_phSaoLumaEnabledFlag : 1;              // [0..1]
            uint32_t m_phSaoChromaEnabledFlag : 1;            // [0..1]
            uint32_t m_phDeblockingFilterDisabledFlag : 1;    // [0..1]
        };
        uint32_t value;
    } m_picPhFlags;

    // reference lists
    int32_t  m_picOrderCntVal;       // [-2^31 .. 2^31-1]
    int32_t  m_refFramePocList[15];  // [-2^31 .. 2^31-1]
    uint32_t m_rprConstraintsActiveFlag[2][15];
    uint32_t unavailableRefPic[2][15];

    uint32_t m_numSlices; //Slice Num of each Pic [1...600]
    uint16_t m_picWidthInCtu;
    uint16_t m_picHeightInCtu;

    struct
    {
        uint16_t    m_tileRows;
        uint16_t    m_tileCols;
        TileRowDesc m_tileRow[440];
        TileColDesc m_tileCol[20];
    } tileCodingParam;

    struct
    {
        uint8_t  m_apsAdaptationParameterSetId;  // [0..7]
        uint8_t  m_reserved8b;                   // [0]
        uint8_t  m_lmcsMinBinIdx;                // [0..15]
        uint8_t  m_lmcsDeltaMaxBinIdx;           // [0..15]
        int16_t  m_lmcsDeltaCW[16];              // [-32767..32767]
        int8_t   m_lmcsDeltaCrs;                 // [-7..7]
        uint32_t m_reserved32b[8];               // [0]
    } m_vvcLmcsData;

    struct
    {
        uint16_t m_lmcsCW[16];
        int32_t  m_scaleCoeff[16];
        int32_t  m_invScaleCoeff[16];
        int32_t  m_chromaScaleCoeff[16];
        int16_t  m_lmcsPivot[17];
    } m_vvcLmcsShapeInfo;

    PicHeaderRplParam m_phRplInfoParam;

    uint8_t            m_alfChromaNumAltFiltersMinus1[8];
    uint8_t            m_alfCcCbFiltersSignalledMinus1[8];
    uint8_t            m_alfCcCrFiltersSignalledMinus1[8];
    uint8_t            m_isMultiSubPicParam;

    struct
    {
        uint8_t  reserve_0;
        uint16_t reserve_1;
        uint32_t reserve_2;
        uint32_t reserve_3;
    } reserve;
};

struct VvcS2lBss
{
    uint32_t m_dmemSize;
    // Platfrom information
    uint32_t ProductFamily;
    uint16_t RevId;

    uint8_t Reserved;
    uint8_t isDmaCopyEnable;
    uint8_t isCp;

    // Picture level DMEM data
    HucVvcS2lPicBss VvcPictureBss;

    uint32_t sliceBsParamOffset;         // offset for slice bitstream param in sliceLvlParam
    uint32_t sliceBsParamNumber;         // number of slice bitstream param in sliceLvlParam
    uint32_t sliceBbParamOffset;         // offset for slice batch buffer param in sliceLvlParam
    uint32_t sliceBbParamNumber;         // number of slice batch buffer param in sliceLvlParam
    uint32_t subPicParamOffset;          // offset for sub pic param in sliceLvlParam
    uint32_t subPicParamNumber;          // number of sub pic param in sliceLvlParam
    uint32_t slicePartitionParamNumber;  // number of slice partition param in sliceLvlParam

    // Slice level DMEM data, includes below data in order
    // 1. slice bitstream param
    // 2. slice batch buffer param
    // 3. sub picture param
    // 4. slice partition praram
    uint32_t sliceLvlParam[1];
};

struct VvcS2lExtraBss
{
    union
    {
        struct VvcS2lBssRplInfo
        {
            uint8_t  m_listIdx;                // [0..1]
            uint8_t  m_rplsIdx;                // [0.. spsNumRefPicLists[listIdx]]
            uint8_t  m_numRefEntries;          // [0..29]
            uint8_t  m_ltrpInHeaderFlag;       // [0..1]
            uint8_t  m_stRefPicFlag[29];       // [0..1]
            uint16_t m_absDeltaPocSt[29];      // [0.. 2^15 - 1]
            uint8_t  m_strpEntrySignFlag[29];  // [0..1]
            uint16_t m_rplsPocLsbLt[29];       // [0..2^16-1]

            uint32_t m_reserved32b[16];  // [0]  leave space for multilayer enabling
        } rplInfo;
        uint8_t align64[MOS_ALIGN_CEIL(sizeof(struct VvcS2lBssRplInfo), 64)];
    } m_rplInfo[vvcMaxRplNum];

    union
    {
        struct VvcS2lBssPartitionInfo
        {
            int32_t m_numCtusInCurrSlice;  //!< number of CTUs in current slice

            //Rect slices params
            uint16_t m_tileIdx;             //!< tile index corresponding to the first CTU in the slice
            uint32_t m_numSlicesInTile;     //!< number of slices in current tile for the special case of multiple slices inside a single tile
            uint16_t m_sliceWidthInTiles;   //!< slice width in units of tiles
            uint16_t m_sliceHeightInTiles;  //!< slice height in units of tiles
            uint16_t m_sliceHeightInCtu;    //!< slice height in units of CTUs for the special case of multiple slices inside a single tile

            //Additional params on SubPic v.s. Slice
            uint32_t m_sliceStartCtbx;
            uint32_t m_sliceStartCtby;
            uint16_t m_subPicIdx;
            uint16_t m_sliceIdxInSubPic;
            uint32_t m_sliceEndCtbx;  //! valid only for rect slice mode
            uint32_t m_sliceEndCtby;  //! valid only for rect slice mode

            //Only used for rec slice mode
            uint32_t m_multiSlicesInTileFlag;
            uint16_t m_startTileX;
            uint16_t m_startTileY;

            //for command programming
            uint32_t m_topSliceInTileFlag;     //! valid only for multiple slices in one tile case, others ignore
            uint32_t m_bottomSliceInTileFlag;  //! valid only for multiple slices in one tile case, others ignore
        }partitionInfo;
        uint8_t align64[MOS_ALIGN_CEIL(sizeof(struct VvcS2lBssPartitionInfo), 64)];
    } m_slicePartitionParam[CODECHAL_VVC_MAX_NUM_SLICES_LVL_6];

    union
    {
        struct VvcWeightedPredInfoInPH
        {
            uint8_t m_lumaLog2WeightDenom;         // [0..7]
            int8_t  m_deltaChromaLog2WeightDenom;  // [-7..7]
            uint8_t m_numL0Weights;                // [0..15]
            uint8_t m_lumaWeightL0Flag[15];        // [0..1]
            uint8_t m_chromaWeightL0Flag[15];      // [0..1]
            int8_t  m_deltaLumaWeightL0[15];       // [-128..127]
            int8_t  m_lumaOffsetL0[15];            // [-128..127]
            int8_t  m_deltaChromaWeightL0[15][2];  // [-128..127]
            int16_t m_deltaChromaOffsetL0[15][2];  // [-512..508]
            uint8_t m_numL1Weights;                // [0..15]
            uint8_t m_lumaWeightL1Flag[15];        // [0..1]
            uint8_t m_chromaWeightL1Flag[15];      // [0..1]
            int8_t  m_deltaLumaWeightL1[15];       // [-128..127]
            int8_t  m_lumaOffsetL1[15];            // [-128..127]
            int8_t  m_deltaChromaWeightL1[15][2];  // [-128..127]
            int16_t m_deltaChromaOffsetL1[15][2];  // [-512..508]
        }paramField;
        uint8_t align64[MOS_ALIGN_CEIL(sizeof(struct VvcWeightedPredInfoInPH), 64)];
    }m_wpInfoinPH;
};

class VvcDecodeS2LPkt : public DecodeHucBasic, public mhw::vdbox::huc::Itf::ParSetting
{
public:
    VvcDecodeS2LPkt(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterfaceNext *hwInterface)
        : DecodeHucBasic(pipeline, task, hwInterface)
    {
        if (pipeline != nullptr)
        {
            m_statusReport   = pipeline->GetStatusReportInstance();
            m_vvcPipeline    = dynamic_cast<VvcPipeline *>(pipeline);
        }
        if (m_vvcPipeline != nullptr)
        {
            m_featureManager = m_vvcPipeline->GetFeatureManager();
            m_allocator      = m_vvcPipeline->GetDecodeAllocator();
            m_decodecp       = m_vvcPipeline->GetDecodeCp();
        }

        if (hwInterface != nullptr)
        {
            m_hwInterface = hwInterface;
            m_osInterface = hwInterface->GetOsInterface();
            m_miItf       = std::static_pointer_cast<mhw::mi::Itf>(hwInterface->GetMiInterfaceNext());
            m_vdencItf    = std::static_pointer_cast<mhw::vdbox::vdenc::Itf>(hwInterface->GetVdencInterfaceNext());
            m_hucItf      = std::static_pointer_cast<mhw::vdbox::huc::Itf>(hwInterface->GetHucInterfaceNext());

            if (m_hwInterface != nullptr)
            {
                m_vvcpItf = std::static_pointer_cast<mhw::vdbox::vvcp::Itf>(m_hwInterface->GetVvcpInterfaceNext());
            }
        }
    }
    virtual ~VvcDecodeS2LPkt() {}

    //!
    //! \brief  Add the command sequence into the commandBuffer and
    //!         and return to the caller task
    //! \param  [in] commandBuffer
    //!         Pointer to the command buffer which is allocated by caller
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Submit(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase = otherPacket) override;

     //!
    //! \brief  Initialize the media packet, allocate required resources
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Init() override;

    //!
    //! \brief  Prepare interal parameters, should be invoked for each frame
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Prepare() override;

    //!
    //! \brief  Calculate Command Size
    //!
    //! \param  [in, out] commandBufferSize
    //!         requested size
    //! \param  [in, out] requestedPatchListSize
    //!         requested size
    //! \return MOS_STATUS
    //!         status
    //!
    virtual MOS_STATUS CalculateCommandSize(
        uint32_t &commandBufferSize,
        uint32_t &requestedPatchListSize) override;

    uint32_t GetSliceBatchOffset(uint32_t sliceNum);

    //!
    //! \brief  Get Packet Name
    //! \return std::string
    //!
    virtual std::string GetPacketName() override
    {
        return "VVC_S2L";
    }

    virtual MHW_SETPAR_DECL_HDR(HUC_DMEM_STATE);
    virtual MHW_SETPAR_DECL_HDR(HUC_IND_OBJ_BASE_ADDR_STATE);
    virtual MHW_SETPAR_DECL_HDR(HUC_VIRTUAL_ADDR_STATE);

    virtual MOS_STATUS AddCmd_HUC_STREAM_OBJECT(MOS_COMMAND_BUFFER &cmdBuffer, CodecVvcSliceParams sliceParams);
    virtual MOS_STATUS AddCmd_HUC_START(MOS_COMMAND_BUFFER &cmdBuffer, bool laststreamobject);
    virtual MOS_STATUS AddCmd_HUC_PIPE_MODE_SELECT(MOS_COMMAND_BUFFER &cmdBuffer);
    virtual MOS_STATUS AddCmd_HUC_IMEM_STATE(MOS_COMMAND_BUFFER &cmdBuffer);

    virtual MOS_STATUS Execute(MOS_COMMAND_BUFFER &cmdBuffer, bool prologNeeded) = 0;

    virtual MOS_STATUS AllocateResources();
    
    virtual MOS_STATUS PackPictureLevelCmds(MOS_COMMAND_BUFFER &cmdBuffer);

    virtual MOS_STATUS PackSliceLevelCmds(MOS_COMMAND_BUFFER &cmdBuffer);

    virtual MOS_STATUS ConstructLmcsReshaper() const;

    MOS_STATUS VdPipelineFlush(MOS_COMMAND_BUFFER &cmdBuffer);

    MOS_STATUS SetDmemBuffer();

    virtual MOS_STATUS Destroy() override;
    
    virtual uint32_t GetHucStatusVvcS2lFailureMask()
    {
        return m_hucStatusVvcS2lFailureMask;
    }

    PMOS_BUFFER GetS2lDmemBuffer();

protected:
    //!
    //! \brief  Calculate Command Buffer Size
    //!
    //! \return uint32_t
    //!         Command buffer size calculated
    //!
    virtual uint32_t CalculateCommandBufferSize();

    //!
    //! \brief  Calculate Patch List Size
    //!
    //! \return uint32_t
    //!         Patchlist size calculated
    //!
    virtual uint32_t CalculatePatchListSize();

    MOS_STATUS SetRefIdxStateCmd(PicHeaderRplParam& phRplInfo);  //This Only Used in RPL info in PH mode in S2L

    void CalculateVvcSliceLvlCmdSize();

    virtual MOS_STATUS SetHucDmemPictureBss(HucVvcS2lPicBss &hucVvcS2LPicBss);
    virtual MOS_STATUS SetHucDmemSliceBss(
        VvcS2lBss* VvcS2lBss);
    virtual MOS_STATUS SetExtraDataBuffer();
    MOS_STATUS AddHucCpState(MOS_COMMAND_BUFFER &cmdBuffer, uint32_t index, CodecVvcSliceParams &sliceParams);
    MOS_STATUS FillPhRplInfoArray(uint8_t listIdx, uint8_t entryCounts, PicHeaderRplParam &phRplInfo);

#if USE_CODECHAL_DEBUG_TOOL
    virtual MOS_STATUS DumpHucS2l();
#endif

    VvcPipeline                          *m_vvcPipeline     = nullptr;
    VvcBasicFeature                      *m_vvcBasicFeature = nullptr;
    const CodecVvcPicParams              *m_vvcPicParams    = nullptr;  //!< Pointer to picture parameter
    CodecVvcSliceParams                  *m_vvcSliceParams  = nullptr;
    MediaFeatureManager                  *m_featureManager  = nullptr;
    DecodeAllocator                      *m_allocator       = nullptr;
    PMOS_INTERFACE                        m_osInterface     = nullptr;
    CodechalHwInterfaceNext              *m_hwInterface     = nullptr;
    DecodeBasicFeature                   *m_basicFeature    = nullptr;
    DecodeCpInterface                    *m_decodecp        = nullptr;
    std::shared_ptr<mhw::vdbox::huc::Itf> m_hucItf          = nullptr;

    uint32_t                         m_dmemBufferSize       = 0;        //!< Size of DMEM buffer
    uint32_t                         m_dmemTransferSize     = 0;        //!< Transfer size of current DMEM buffer

    uint32_t m_pictureStatesSize    = 0;
    uint32_t m_picturePatchListSize = 0;
    uint32_t m_sliceStatesSize      = 0;
    uint32_t m_slicePatchListSize   = 0;

    MOS_BUFFER  *m_vvcS2lExtraDataBuffer = nullptr;
    
    BufferArray *m_vvcS2lExtraBufferArray = nullptr;
    BufferArray *m_vvcS2lDmemBufferArray  = nullptr;

    static constexpr uint32_t m_vdboxHucVvcS2lKernelDescriptor = 20; //!< Huc VVC S2L Kernel descriptor

    static const uint32_t m_hucStatusInvalidMask = 0;                       //!< Invalid mask of Huc status MMIO
    uint32_t              m_hucStatusMask        = m_hucStatusInvalidMask;  //!< MMIO mask for HuC status
    uint32_t              m_hucStatus2Mask       = m_hucStatusInvalidMask;  //!< MMIO mask for HuC status2

    static const uint32_t m_hucStatusVvcS2lFailureMask = 0x8000;  //!< HuC Status VVC short to long failure mask
                                                           //!< bit14: uKernal uOS Status, FW will write 0 if has critical error


    uint32_t m_sliceBsParamOffset = 0;         // offset for slice bitstream param in sliceLvlParam
    uint32_t m_sliceBsParamNumber = 0;         // number of slice bitstream param in sliceLvlParam
    uint32_t m_sliceBbParamOffset = 0;         // offset for slice batch buffer param in sliceLvlParam
    uint32_t m_sliceBbParamNumber = 0;         // number of slice batch buffer param in sliceLvlParam
    uint32_t m_subPicParamOffset  = 0;         // offset for sub pic param in sliceLvlParam
    uint32_t m_subPicParamNumber  = 0;         // number of sub pic param in sliceLvlParam
    uint32_t m_slicePartitionParamNumber = 0;  // number of slice partition param in sliceLvlParam
    uint32_t m_sliceParamDynamicSize     = 0;

    uint32_t m_vvcpSliceCmdSize  = 0;           //Slice Lvl Command Size
    uint32_t m_tailingBsReadSize = 0;           //SW WA for HuC Emu Byte Removal HW issue

    bool     m_isMultiSubPicParam = false;

    std::shared_ptr<mhw::vdbox::vvcp::Itf> m_vvcpItf = nullptr;

    MEDIA_CLASS_DEFINE_END(decode__VvcDecodeS2LPkt)
};



}  // namespace decode
#endif  // !__DECODE_VVC_S2L_PACKET_H__
