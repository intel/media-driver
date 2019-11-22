/*
* Copyright (c) 2017, Intel Corporation
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
//! \file     codechal_encode_hevc_base.h
//! \brief    Defines base class for HEVC encoder.
//!

#ifndef __CODECHAL_ENCODE_HEVC_BASE_H__
#define __CODECHAL_ENCODE_HEVC_BASE_H__

#include "codechal_encoder_base.h"
#if USE_CODECHAL_DEBUG_TOOL
#include "codechal_debug_encode_par.h"
#endif
//*------------------------------------------------------------------------------
//* Codec Definitions
//*------------------------------------------------------------------------------
#define CODECHAL_HEVC_MAX_SPS_NUM       16
#define CODECHAL_HEVC_MAX_PPS_NUM       64
#define CODECHAL_VDENC_HEVC_MAX_SLICE_NUM   70

#define CODECHAL_HEVC_MAX_LCU_SIZE_G9          32
#define CODECHAL_HEVC_MAX_LCU_SIZE_G10         64
#define CODECHAL_HEVC_MIN_LCU_SIZE             16
#define CODECHAL_HEVC_MIN_CU_SIZE              8
#define CODECHAL_HEVC_MIN_TILE_SIZE            128
#define CODECHAL_HEVC_VDENC_MIN_TILE_WIDTH_SIZE      256
#define CODECHAL_HEVC_VDENC_MIN_TILE_HEIGHT_SIZE     128
#define CODECHAL_HEVC_FRAME_BRC_BLOCK_SIZE     32
#define CODECHAL_HEVC_LCU_BRC_BLOCK_SIZE       128

#define CODECHAL_ENCODE_HEVC_NUM_MAX_VME_L0_REF_G9  3
#define CODECHAL_ENCODE_HEVC_NUM_MAX_VME_L1_REF_G9  1

#define CODECHAL_ENCODE_HEVC_NUM_MAX_VME_L0_REF_G10 4
#define CODECHAL_ENCODE_HEVC_NUM_MAX_VME_L1_REF_G10 4

#define CODECHAL_ENCODE_HEVC_MAX_NUM_WEIGHTED_PRED_L0 1
#define CODECHAL_ENCODE_HEVC_MAX_NUM_WEIGHTED_PRED_L1 1

#define CODECHAL_HEVC_MAX_NUM_HCP_PIPE      8
#define CODECHAL_HEVC_MAX_NUM_BRC_PASSES    4 // It doesn't include the 5th PAK pass for the BRC panic mode

#define CODECHAL_HEVC_SAO_STRMOUT_SIZE_PERLCU   16

#define ENCODE_HEVC_4K_PIC_WIDTH     3840
#define ENCODE_HEVC_4K_PIC_HEIGHT    2160

#define ENCODE_HEVC_5K_PIC_WIDTH     5120
#define ENCODE_HEVC_5K_PIC_HEIGHT    2880

#define ENCODE_HEVC_8K_PIC_WIDTH     7680
#define ENCODE_HEVC_8K_PIC_HEIGHT    4320

#define ENCODE_HEVC_16K_PIC_WIDTH    16384
#define ENCODE_HEVC_16K_PIC_HEIGHT   4096

// Max supported resolution for HEVC encode is 4K X 2K
#define ENCODE_HEVC_MAX_4K_PIC_WIDTH     4096
#define ENCODE_HEVC_MAX_4K_PIC_HEIGHT    2176

#define ENCODE_HEVC_MAX_8K_PIC_WIDTH     8192
#define ENCODE_HEVC_MAX_8K_PIC_HEIGHT    8192

#define ENCODE_HEVC_MAX_16K_PIC_WIDTH    16384
#define ENCODE_HEVC_MAX_16K_PIC_HEIGHT   16384

// HEVC has only 3 target usage modes
#define CODECHAL_ENCODE_HEVC_TARGET_USAGE_MODE_PERFORMANCE  0
#define CODECHAL_ENCODE_HEVC_TARGET_USAGE_MODE_NORMAL       1
#define CODECHAL_ENCODE_HEVC_TARGET_USAGE_MODE_QUALITY      2

#define HEVC_BRC_CONSTANT_SURFACE_WIDTH_G9          (64)
#define HEVC_BRC_CONSTANT_SURFACE_HEIGHT_G9         (53)
#define HEVC_BRC_HISTORY_BUFFER_SIZE_G9             (576)
#define HEVC_BRC_HISTORY_BUFFER_SIZE_G10            (832)
#define HEVC_BRC_HISTORY_BUFFER_SIZE_G12            (1088)
#define HEVC_BRC_CONSTANT_SURFACE_HEIGHT_G10        (35)

#define CODECHAL_HEVC_NUM_PAK_SLICE_BATCH_BUFFERS   (3)
#define HEVC_BRC_PAK_STATISTCS_SIZE                 (32)
#define HEVC_CONCURRENT_SURFACE_HEIGHT              (32)
#define HEVC_BRC_SKIP_VAL_TABLE_SIZE                (128)
#define HEVC_BRC_LAMBDA_TABLE_SIZE                  (1024)
#define HEVC_BRC_MVMODE_TABLE_SIZE                  (1664)
#define CODECHAL_INIT_DSH_SIZE_HEVC_ENC             (MHW_PAGE_SIZE * 2)
#define HEVC_START_CODE_NAL_OFFSET                  (2)
#define HEVC_PAK_STATISTICS_SSE_OFFSET              (32)

#define CODECHAL_ENCODE_HEVC_MAX_SLICE_QP               51

#define CODECHAL_ENCODE_HEVC_MIN_ICQ_QUALITYFACTOR      1
#define CODECHAL_ENCODE_HEVC_MAX_ICQ_QUALITYFACTOR      51

#define CODECHAL_ENCODE_HEVC_NUM_SYNC_TAGS              36

#define CODECHAL_ENCODE_HEVC_DEFAULT_AVBR_ACCURACY      30
#define CODECHAL_ENCODE_HEVC_DEFAULT_AVBR_CONVERGENCE   150

// Hevc always uses maxMvLen corresponding to AVC level 51. Refer GetMaxMvLen from AVC.
#define CODECHAL_ENCODE_HEVC_MAX_MV_LEN_AVC_LEVEL_51    511

#define CODECHAL_ENCODE_HEVC_GET_SIZE_IN_LCU(var, lcu_size)         \
    ((var + (lcu_size - 1)) / lcu_size)

#define CODECHAL_HEVC_PAK_STREAMOUT_SIZE 0x500000  //size is accounted for 4Kx4K with all 8x8 CU,based on streamout0 and streamout1 requirements
//(4096*4096)/64 *16 (streamout0) + 1MB(streamout 1). there is scope to reduce streamout1 size. Need to check with HW team.
// 8K is just an estimation
#define CODECHAL_HEVC_FRAME_HEADER_SIZE   8192

#define CODECHAL_HEVC_BRC_QP_ADJUST_SIZE    576

#define CODECHAL_ENCODE_HEVC_NUM_MAX_VDENC_L0_REF_G10  3 // multiref, hevc vdenc
#define CODECHAL_ENCODE_HEVC_NUM_MAX_VDENC_L1_REF_G10  3 // multiref, hevc vdenc

#define CODECHAL_VDENC_HEVC_BRC_HISTORY_BUF_SIZE             964
#define CODECHAL_VDENC_HEVC_BRC_DEBUG_BUF_SIZE               0x1000 // 0x1000 = 4096
#define CODECHAL_VDENC_HEVC_BRC_HUC_STATUS_REENCODE_MASK     (1<<31)

#define CODECHAL_VDENC_HEVC_BRC_PAK_STATS_BUF_SIZE                  464     // 116 DWORDs HEVC Frame Statistics
#define CODECHAL_HEVC_VDENC_STATS_SIZE                              1216    // VDEnc Statistic: 48DWs (3CLs) of HMDC Frame Stats + 256 DWs (16CLs) of Histogram Stats = 1216 bytes

#define CODECHAL_HEVC_I_SLICE          2
#define CODECHAL_HEVC_P_SLICE          1
#define CODECHAL_HEVC_B_SLICE          0
#define CODECHAL_HEVC_NUM_SLICE_TYPES  3

#define CODECHAL_HEVC_NUM_MODE_MV_COSTS             171
#define CODECHAL_HEVC_MAX_BSTRUCTRUE_GOP_SIZE       16
#define CODECHAL_HEVC_MAX_BSTRUCTRUE_REF_NUM        8
#define CODECHAL_HEVC_BRC_MAX_QUALITY_LAYER         16
#define CODECHAL_HEVC_BRC_MAX_TEMPORAL_LAYER        3

struct MultiPassConfig
{
    uint32_t SliceMaxQPOffset0;
    uint32_t SliceMaxQPOffset1;
    uint32_t SliceMaxQPOffset2;
    uint32_t SliceMaxQPOffset3;

    int32_t SliceMinQPOffset0;
    int32_t SliceMinQPOffset1;
    int32_t SliceMinQPOffset2;
    int32_t SliceMinQPOffset3;
    uint32_t MaxIntraConformanceLimit;
    uint32_t MaxInterConformanceLimit;
    uint32_t FrameRateCtrlFlag;
    uint32_t InterMbMaxBitFlag;
    uint32_t IntraMbMaxBitFlag;
    uint32_t FrameMaxBitRate;
    uint32_t FrameMinBitRate;
    uint32_t MinFrameRateDelta;
    uint32_t MaxFrameRateDelta;
    int16_t NumberofPasses;
    uint32_t MbRateCtrlFlag;
    uint8_t FrameSzOverFlag;
    uint8_t FrameSzUnderFlag;
    uint8_t FrameMinBitRateUnit;
    uint8_t FrameMaxBitRateUnit;
    uint8_t StreamOutEnable;


    MultiPassConfig() :
        SliceMaxQPOffset0(0),
        SliceMaxQPOffset1(0),
        SliceMaxQPOffset2(0), // Set this to Default Max Limit as per Spec.,
        SliceMaxQPOffset3(0),
        SliceMinQPOffset0(0),
        SliceMinQPOffset1(0),
        SliceMinQPOffset2(0), // Set this to Default Max Limit as per Spec.,
        SliceMinQPOffset3(0),
        MaxIntraConformanceLimit(3180),
        MaxInterConformanceLimit(3180),
        FrameRateCtrlFlag(0),
        InterMbMaxBitFlag(0),
        IntraMbMaxBitFlag(0),
        MinFrameRateDelta(0),
        MaxFrameRateDelta(0),
        MbRateCtrlFlag(0),
        FrameSzOverFlag(0),
        FrameSzUnderFlag(0),
        FrameMinBitRateUnit(0),
        FrameMaxBitRateUnit(0),
        StreamOutEnable(0)
    {}
};

struct EncodeHevcPar
{
    uint32_t NumFrames;
    uint32_t Width;
    uint32_t Height;
    uint32_t NumP;
    uint32_t NumB;
    uint32_t NumSlices;
    uint32_t SliceStartLCU;
    uint32_t InputChromaFormatIDC;
    uint32_t ChromaFormatIDC;
    uint32_t InputBitDepthLuma;
    uint32_t InputBitDepthChroma;
    uint32_t OutputBitDepthLuma;
    uint32_t OutputBitDepthChroma;
    uint32_t InternalBitDepthLuma;
    uint32_t InternalBitDepthChroma;
    uint32_t ISliceQP;
    uint32_t PSliceQP;
    uint32_t BSliceQP;
    uint32_t BoxFilter;
    uint32_t WeightedPred;
    uint32_t WeightedBiPred;
    uint32_t MaxRefIdxL0;
    uint32_t MaxRefIdxL1;
    uint32_t MaxBRefIdxL0;
    uint32_t EnableBAsRefs;
    uint32_t LowDelay;
    uint32_t ChangePtoRefB;
    uint32_t InputRgbToGbr;

    uint32_t StartFrameNum;
    uint32_t FrameRateCode;
    uint32_t FrameRateM;    //frame rate multipier (nominator)
    uint32_t FrameRateD;    //frame rate divider (denominator)
    uint8_t  numTemporalLayers;

    /* BRC parameters */
    uint32_t BRCMethod;
    uint32_t BRCType;
    uint32_t CuRC;
    uint32_t LookAheadFrames;
    uint32_t VideoTestData;
    uint32_t AvbrAccuracy;  //for AVBR 
    uint32_t AvbrConvergence;  //for AVBR 
    uint32_t usePrevQP;  //for fast mode 
    uint32_t RateControl; //obsolete

    uint32_t BitRate;
    uint32_t MaxBitRate; // for VBR support  
    uint32_t VbvSzInBit;
    uint32_t InitVbvFullnessInBit;
    uint32_t UserMaxIFrame; //user defined frame rate
    uint32_t UserMaxPBFrame; //user defined frame rate
    uint32_t EnableMultipass;
    uint32_t MaxNumPakPassesI;
    uint32_t MaxNumPakPassesPB;

    uint32_t QPMax; // user defined QP max (default=51)
    uint32_t QPMin; // user defined QP min (default=1)
    uint32_t BitRateQT[CODECHAL_HEVC_BRC_MAX_QUALITY_LAYER][CODECHAL_HEVC_BRC_MAX_TEMPORAL_LAYER];
    uint32_t MaxBitRateQT[CODECHAL_HEVC_BRC_MAX_QUALITY_LAYER][CODECHAL_HEVC_BRC_MAX_TEMPORAL_LAYER]; // for VBR support
    uint32_t VbvSzInBitQT[CODECHAL_HEVC_BRC_MAX_QUALITY_LAYER][CODECHAL_HEVC_BRC_MAX_TEMPORAL_LAYER];
    uint32_t InitVbvFullnessInBitQT[CODECHAL_HEVC_BRC_MAX_QUALITY_LAYER][CODECHAL_HEVC_BRC_MAX_TEMPORAL_LAYER];
    uint32_t UserMaxFrameQT[CODECHAL_HEVC_BRC_MAX_QUALITY_LAYER][CODECHAL_HEVC_BRC_MAX_TEMPORAL_LAYER]; //user defined frame rate

    uint32_t CRFQualityFactor;

    uint32_t SlidingWindowRCEnable;
    uint32_t SlidingWindowSize;
    uint32_t SlidingWindowMaxRateRatio;

    //reset test, for base layer
    int32_t reset_frame_num;
    int32_t new_br;
    int32_t new_maxbr;
    int16_t new_gopp;
    int16_t new_gopi;
    uint32_t new_frM; //frame rate multipier (nominator)
    uint32_t new_frD; //frame rate divider (denominator)
    uint32_t new_userMaxFrame; //user defined frame rate 
                          //end reset test

    MultiPassConfig MultiPassParam;

    /* HEVC parameters */
    uint32_t ProfileIDC;
    uint32_t LevelIDC;
    uint32_t Log2MaxCUSize;
    uint32_t Log2MinCUSize;
    uint32_t Log2MaxTUSize;
    uint32_t Log2MinTUSize;
    uint32_t Log2TUMaxDepthInter;
    uint32_t Log2TUMaxDepthIntra;
    uint32_t Log2ParallelMergeLevel;
    uint32_t TransquantBypassEnableFlag;
    uint32_t TransformSkipEnabledFlag;
    uint32_t PCMEnabledFlag;
    uint32_t Log2MaxPCMSize;
    uint32_t Log2MinPCMSize;
    uint32_t TemporalMvpEnableFlag;
    uint32_t CollocatedFromL0Flag;
    uint32_t CollocatedRefIdx;
    uint32_t MaxNumMergeCand;
    uint32_t MvdL1ZeroFlag;
    uint32_t ConstrainedIntraPred;
    uint32_t StrongIntraSmoothingEnableFlag;
    int32_t ChromaCbQpOffset;
    int32_t ChromaCrQpOffset;
    uint8_t  ScalingList[3 * 6 + 2][64]; /* 0-5: 4x4; 6-11: 8x8; 12-17: 16x16; 18-19: 32x32 */
    uint8_t  ScalingListDCCoef[2][6];

    // 1st index L0/L1, 2nd index weight/offset, 3rd index: Luma, cb, cr
    int32_t WeightOffset[2][2][3][16];
    uint32_t Log2LumaDenom;
    uint32_t Log2ChromaDenom;
    int32_t EnableSEI;
    uint32_t DeblockingIDC;
    uint32_t LoopFilterAcrossSlicesEnabledFlag;
    uint32_t LoopFilterAcrossTilesEnabledFlag;
    int32_t DeblockingTc;
    int32_t DeblockingBeta;
    uint32_t PCMLoopFilterDisableFlag;
    uint32_t AmpEnabledFlag;
    uint32_t SignDataHidingFlag;
    uint32_t SAOEnabledFlag;
    uint32_t SAOLumaEnabledFlag;
    uint32_t SAOChromaEnabledFlag;
    uint32_t ForceSAOToZero;
    uint32_t AdaptiveSAOMethod; /*0: always two passes, 1: if BRC has two pass, sao will be single pass, 2: Periodically enabling sao*/
    uint32_t CuQpDeltaEnabledFlag;
    uint32_t DiffCuQpDeltaDepth;
    uint32_t ScalingListDataPresentFlag;
    uint32_t CabacInitFlag;
    uint32_t RoundingInter;
    uint32_t RoundingIntra;
    uint32_t UseLongTermPic;
    uint32_t LogTermPicInterval;
    uint32_t Seed;

    uint32_t Log2MaxTUSkipBlockSize;
    uint32_t ExplicitRdpcmEnabledFlag;
    uint32_t ImplicitRdpcmEnabledFlag;
    uint32_t TransformSkipRotationEnabledFlag;
    uint32_t TransformSkipContextEnabledFlag;
    uint32_t CrossComponentPredictionEnabledFlag;
    uint32_t ExtendedPrecisionProcessingFlag;
    uint32_t IntraSmoothingDisabledFlag;
    uint32_t HighPrecisionOffsetsEnabledFlag;
    uint32_t PersistentRiceAdaptationEnabledFlag;
    uint32_t CabacBypassAlignmentEnabledFlag;
    uint32_t SAOOffsetScaleLuma;
    uint32_t SAOOffsetScaleChroma;

    uint32_t TilesEnabledFlag;
    uint32_t UniformSpacingFlag;
    uint32_t NumTileColumns;
    uint32_t NumTileRows;
    uint32_t TileColumnsWidth[HEVC_NUM_MAX_TILE_COLUMN];
    uint32_t TileRowsHeight[HEVC_NUM_MAX_TILE_ROW];

    uint32_t SetLongTermAsRef0;

    uint32_t TSDecisionEnabledFlag;
    uint32_t IntraFrameRDOQEnabledFlag;
    uint32_t InterFrameRDOQEnabledFlag;
    uint32_t IntraRDOQEnableFlag;
    uint32_t InterRDOQEnableFlag;
    uint32_t AdaptiveRDOQFlag;
    uint32_t AdaptiveRDOQIntraPctThreshold;
    uint32_t AdaptiveRDOQIntraDistThreshold;
    uint32_t IntraTuCountBasedRDOQDisable;
    uint32_t RDOQIntraTUThreshold[4];
    int32_t RDOQIntraTUThresholdInPercents;
    uint32_t ScalabilityMode;
    uint32_t RhoDomainEnabledFlag;

    uint32_t SseStatsEnable;
    uint32_t PAKStatsEnable;

    uint32_t AddEoStreamNAL;
    uint32_t AddEoSequenceNAL;

    //VME HW control
    uint32_t IntraPrediction;
    uint32_t IntraSADMeasure;
    uint32_t InterSADMeasure;
    uint32_t BmeDisable;
    uint32_t BilinearEnable;
    uint32_t SubPelMode;
    uint32_t ImeRefWindowSize;
    uint32_t MaxNumSU;
    uint32_t LenSP;
    uint32_t EarlyImeSuccessEn;
    uint32_t EarlyImeStopThres;
    uint32_t AdaptiveEn;
    uint32_t RefIDCostMode;
    uint32_t DisablePIntra;
    uint32_t  InterShapeMask;
    uint32_t IntraChromaModeMask;
    uint32_t Intra32x32ModeMask;
    uint32_t Intra16x16ModeMask;
    uint32_t Intra8x8ModeMask;

    int32_t HmeGlobalOffsetX[2][2];
    int32_t HmeGlobalOffsetY[2][2];
    uint32_t HmeMvCostingQP;
    uint32_t HMECoarseRefPic;
    uint32_t HMEStage1Disable;
    uint32_t HMEStage2Disable;
    uint32_t HMEDualWinner;
    uint32_t HMERef1Disable;

    uint32_t Ime16x16MvCostShift;
    uint32_t Ime32x32AddMvCostShift;

    uint32_t ImeStage3MvThreshold;
    uint32_t MaxNumImePredictor;
    uint32_t ImePredictorPriority[24];

    //ENC SW control
    uint32_t Enable64x64Skip;
    uint32_t HevcMaxCuControl;

    uint32_t MaxIntra8x8TuSize;
    uint32_t MaxIntra16x16TuSize;
    uint32_t MaxIntra32x32TuSize;
    uint32_t MaxIntra64x64TuSize;
    uint32_t MaxInter8x8TuSize;
    uint32_t MaxInter16x16TuSize;
    uint32_t MaxInter32x32TuSize;
    uint32_t MaxInter64x64TuSize;

    uint32_t NumBetaPreditors;
    uint32_t NumMergeCandidateCu8x8;
    uint32_t NumMergeCandidateCu16x16;
    uint32_t NumMergeCandidateCu32x32;
    uint32_t NumMergeCandidateCu64x64;

    uint32_t IntraChromaSadWeight;

    uint32_t DisableIntraLuma4x4Tu;

    uint32_t RandEncModeEnabled;

    //B Frame Coding Structure
    uint32_t BGOPSize;
    int32_t  IntraPeriod;
    uint32_t PerBFramePOC[CODECHAL_HEVC_MAX_BSTRUCTRUE_GOP_SIZE];
    uint32_t PerBFrameQPOffset[CODECHAL_HEVC_MAX_BSTRUCTRUE_GOP_SIZE];
    uint32_t PerBFrameRoundingInter[CODECHAL_HEVC_MAX_BSTRUCTRUE_GOP_SIZE];
    uint32_t PerBFrameRoundingIntra[CODECHAL_HEVC_MAX_BSTRUCTRUE_GOP_SIZE];
    double PerBFrameQPFactor[CODECHAL_HEVC_MAX_BSTRUCTRUE_GOP_SIZE];
    uint32_t PerBFrameTcOffsetDiv2[CODECHAL_HEVC_MAX_BSTRUCTRUE_GOP_SIZE];
    uint32_t PerBFrameBetaOffsetDiv2[CODECHAL_HEVC_MAX_BSTRUCTRUE_GOP_SIZE];
    uint32_t PerBFrameTemporalID[CODECHAL_HEVC_MAX_BSTRUCTRUE_GOP_SIZE];
    uint32_t PerBFrameNumRefPicsActiveL0[CODECHAL_HEVC_MAX_BSTRUCTRUE_GOP_SIZE];
    uint32_t PerBFrameNumRefPicsActiveL1[CODECHAL_HEVC_MAX_BSTRUCTRUE_GOP_SIZE];
    uint32_t PerBFrameNumRefPics[CODECHAL_HEVC_MAX_BSTRUCTRUE_GOP_SIZE];
    int32_t PerBFrameRefPics[CODECHAL_HEVC_MAX_BSTRUCTRUE_GOP_SIZE*CODECHAL_HEVC_MAX_BSTRUCTRUE_REF_NUM];
    uint32_t PerBFramePredict[CODECHAL_HEVC_MAX_BSTRUCTRUE_GOP_SIZE];
    uint32_t PerBFrameDeltaRPS[CODECHAL_HEVC_MAX_BSTRUCTRUE_GOP_SIZE];
    uint32_t PerBFrameNumRefIdcs[CODECHAL_HEVC_MAX_BSTRUCTRUE_GOP_SIZE];
    uint32_t PefBFrameRefIdcs[CODECHAL_HEVC_MAX_BSTRUCTRUE_GOP_SIZE*CODECHAL_HEVC_MAX_BSTRUCTRUE_REF_NUM];

    //CU QP Adjustment
    uint32_t DisableCuQpAdj;
    uint32_t MaxDeltaQP;
    uint32_t SadMidPointThreshold;
    uint32_t Zone01SadThreshold;
    uint32_t Zone12SadThreshold;
    uint32_t Zone23SadThreshold;
    int32_t DeltaQPForSadZone0;
    int32_t DeltaQPForSadZone1;
    int32_t DeltaQPForSadZone2;
    int32_t DeltaQPForSadZone3;
    uint32_t Zone01MvThreshold;
    uint32_t Zone12MvThreshold;
    int32_t DeltaQPForMvZone0;
    int32_t DeltaQPForMvZone1;
    int32_t DeltaQPForMvZone2;
    int32_t DeltaQPForMvZero;
    int32_t DeltaQPForAngIntra;
    int32_t DeltaQPForNonAngIntra;
    int32_t DeltaQPForTU32x32;
    int32_t DeltaQPForTU16x16;
    int32_t DeltaQPForTU8x8;
    int32_t DeltaQPForTU4x4;
    int32_t DeltaQPForROIZone0;
    int32_t DeltaQPForROIZone1;
    int32_t DeltaQPForROIZone2;
    int32_t DeltaQPForROIZone3;
    int32_t DeltaQPForROIZone4;
    int32_t DeltaQPForROIZone5;
    int32_t DeltaQPForROIZone6;
    int32_t DeltaQPForROIZone7;

    uint32_t RfcThresIntraLow;
    uint32_t RfcThresIntraHigh;
    uint32_t RfcThresIntraChromaLow;
    uint32_t RfcThresIntraChromaHigh;
    uint32_t RfcThresInterLow;
    uint32_t RfcThresInterHigh;
    uint32_t RfcIntra64x64Depth;
    uint32_t RfcIntra32x32Depth;
    uint32_t RfcIntra16x16Depth;
    uint32_t RfcIntra8x8Depth;
    uint32_t RfcInter64x64Depth;
    uint32_t RfcInter32x32Depth;
    uint32_t RfcInter16x16Depth;
    uint32_t RfcInter8x8Depth;

    uint32_t InterTBSThresHigh;
    uint32_t InterTBSThresLow;
    uint32_t IntraLumaTBSThresHigh;
    uint32_t IntraLumaTBSThresLow;
    uint32_t IntraChromaTBSThresHigh;
    uint32_t IntraChromaTBSThresLow;

    uint32_t SseWeightStrength;
    uint32_t SseWeightForZCOnly;

    uint32_t VDEncMode;

    uint32_t OutputQualityType;

    //VDEnc BRC
    uint32_t ReEncodePositiveQPDeltaThr;
    uint32_t ReEncodeNegativeQPDeltaThr;
    uint32_t PrevIntraPctThresh;
    uint32_t CurIntraPctThresh;
    uint32_t TopQPDeltaThrforAdaptive2Pass;
    uint32_t BotQPDeltaThrforAdaptive2Pass;
    uint32_t TopFrmSzPctThrforAdaptive2Pass;
    uint32_t BotFrmSzPctThrforAdaptive2Pass;
    uint32_t QPSelectMethodforFirstPass;
    uint32_t MBHeaderCompensation;
    uint32_t OverShootCarryFlag;
    uint32_t OverShootSkipFramePct;

    //mode/mv costs
    uint32_t  UseExtModeMvCosts;
    double CostTable[CODECHAL_HEVC_NUM_MODE_MV_COSTS];
    //end mode/mv costs

    //StreamIn
    uint32_t StreamInEn;
    uint32_t StreamInPanicEn;
    uint32_t StreamInROIEn;
    uint32_t StreamInROIviaForceQPEn;
    uint32_t StreamInDirtyRectEn;
    uint32_t StreamInMvPredictorRef;
    uint32_t StaticFrameZMVPercent;
    uint32_t HMEStreamInRefCost;


    //rolling I parameters
    uint32_t IntraRefreshEnable; //0: disable 1: enable
    uint32_t IntraRefreshMode; //0:row based 1: col based
    uint32_t IntraRefreshSizeIn32x32; //refresh region
    int32_t IntraRefreshDeltaQP;//delta QP for refresh region

                             //slice size control
    uint32_t SliceSizeCtrl;
    uint32_t EncLcuLag;
    uint32_t SliceSizeThreshold;
    uint32_t MaxSliceSize;
    uint32_t CoeffBitsWeight;
    uint32_t LCUBitSizeAdjustment;
    uint32_t MaxNumSlicesCheckEnable;

    //Left Recon Support
    uint32_t LeftLCUReconEnable;
    uint32_t LeftLCUReconEnableforRollingI;

    // validate for RTL coverage
    uint32_t NoOutputPriorPicsEnable;

    //fade detection
    uint32_t FadeDetectionEnable;
    uint32_t RefListModforWeightPred;

    //VJ : for supporting Chroma formats
    uint32_t InputColorFormat;
    uint32_t ChromaDownSamplingFilterType;

    //Adaptive Rounding
    uint32_t RoundThreshold[3][3]; //Dim0: CUSize (8x8, 16x16, 32x32 Dim1:CU Type (Intra, Inter, Merge/SKip), note 64x64 CUSize will use 32x32 threshold <<2;
    uint32_t RoundValues[3][3][2]; //Dim0: CUSize (8x8, 16x16, 32x32 Dim1:CU Type (Intra, Inter, Merge/SKip)

    //BRC adaptive bitstream fallback
    uint32_t AdaptiveBSSelection;

    //Kernel HME
    uint32_t KernelHierarchicalMEFlag;
    uint32_t KHMECoarseShape;
    uint32_t SuperKHME;
    uint32_t UltraKHME;
    uint32_t EnableKHMEMVCost;
    uint32_t KHMEUseNeighborPredictor;
    uint32_t KernelInterSADTransform;
    uint32_t KernelIntraSADTransform;
    uint32_t KHMESubPelMode;
    int32_t KMeSearchX;
    int32_t KMeSearchY;
    uint32_t KHMECombineOverlap;
    uint32_t KHMECoarseRefPic;
    uint32_t KHMECombineLen;
    uint32_t KHMEBCombineLen;
    uint32_t SuperKCombineDist;
    //Timing budget overflow check
    uint32_t TimeBudgetEnable;
    uint32_t TimeBudgetInitial;
    uint32_t TimeBudgetLCU;
    uint32_t TimeBudgetDisableLCUNum;

    //Panic mode
    uint32_t PanicEnable;

    uint32_t TransformType;

    uint32_t ReconforIntraPrediciton;

    //Stats dump
    uint32_t EnableStatistics;

    uint32_t NetworkTraceEnable;
    uint32_t SceneChangeXFrameSizeEnable;
    uint32_t SceneChangeXFrameSize;

    uint32_t UseDefaultCosts;
    uint32_t UseDefault_SAD_RDO_Lambdas;
    uint32_t UseDefaulRDOQLambdas;
};

const char QPcTable[22] = { 29, 30, 31, 32, 32, 33, 34, 34, 35, 35, 36, 36, 37, 37, 37, 38, 38, 38, 39, 39, 39, 39 };

const unsigned char g_cInit_HEVC_BRC_QP_ADJUST[CODECHAL_HEVC_BRC_QP_ADJUST_SIZE] = {
    0x01, 0x02, 0x03, 0x05, 0x06, 0x01, 0x01, 0x02, 0x03, 0x05, 0x00, 0x00, 0x01, 0x02, 0x03, 0xff,
    0x00, 0x00, 0x01, 0x02, 0xff, 0x00, 0x00, 0x00, 0x01, 0xfe, 0xfe, 0xff, 0x00, 0x01, 0xfd, 0xfd,
    0xff, 0xff, 0x00, 0xfb, 0xfd, 0xfe, 0xff, 0xff, 0xfa, 0xfb, 0xfd, 0xfe, 0xff, 0x00, 0x04, 0x1e,
    0x3c, 0x50, 0x78, 0x8c, 0xc8, 0xff, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00,

    0x01, 0x02, 0x03, 0x05, 0x06, 0x01, 0x01, 0x02, 0x03, 0x05, 0x00, 0x01, 0x01, 0x02, 0x03, 0xff,
    0x00, 0x00, 0x01, 0x02, 0xff, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0x00, 0x01, 0xfe, 0xff,
    0xff, 0xff, 0x00, 0xfc, 0xfe, 0xff, 0xff, 0x00, 0xfb, 0xfc, 0xfe, 0xff, 0xff, 0x00, 0x04, 0x1e,
    0x3c, 0x50, 0x78, 0x8c, 0xc8, 0xff, 0x04, 0x05, 0x06, 0x06, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00,

    0x01, 0x01, 0x02, 0x04, 0x05, 0x01, 0x01, 0x01, 0x02, 0x04, 0x00, 0x00, 0x01, 0x01, 0x02, 0xff,
    0x00, 0x00, 0x01, 0x01, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x01, 0xfe, 0xff,
    0xff, 0xff, 0x00, 0xfd, 0xfe, 0xff, 0xff, 0x00, 0xfb, 0xfc, 0xfe, 0xff, 0xff, 0x00, 0x02, 0x14,
    0x28, 0x46, 0x82, 0xa0, 0xc8, 0xff, 0x04, 0x04, 0x05, 0x05, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00,

    0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x03, 0x03, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x03,
    0x03, 0x04, 0xff, 0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 0x03, 0x03, 0xff, 0xff, 0x00, 0x00, 0x00,
    0x01, 0x02, 0x02, 0x02, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x01, 0x02, 0x02, 0xfe, 0xff, 0xff,
    0x00, 0x00, 0x00, 0x01, 0x01, 0x02, 0xfe, 0xff, 0xff, 0xff, 0x00, 0x00, 0x01, 0x01, 0x02, 0xfe,
    0xfe, 0xff, 0xff, 0x00, 0x00, 0x00, 0x01, 0x01, 0xfe, 0xfe, 0xff, 0xff, 0x00, 0x00, 0x00, 0x01,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x03, 0x03, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x03,
    0x03, 0x04, 0xff, 0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 0x03, 0x03, 0xff, 0xff, 0x00, 0x00, 0x00,
    0x01, 0x02, 0x02, 0x02, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x01, 0x02, 0x02, 0xfe, 0xff, 0xff,
    0x00, 0x00, 0x00, 0x01, 0x01, 0x02, 0xfe, 0xff, 0xff, 0xff, 0x00, 0x00, 0x01, 0x01, 0x02, 0xfe,
    0xfe, 0xff, 0xff, 0x00, 0x00, 0x00, 0x01, 0x01, 0xfe, 0xfe, 0xff, 0xff, 0x00, 0x00, 0x00, 0x01,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x03, 0x03, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x03,
    0x03, 0x04, 0xff, 0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 0x03, 0x03, 0xff, 0xff, 0x00, 0x00, 0x00,
    0x01, 0x02, 0x02, 0x02, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x01, 0x02, 0x02, 0xfe, 0xff, 0xff,
    0x00, 0x00, 0x00, 0x01, 0x01, 0x02, 0xfe, 0xff, 0xff, 0xff, 0x00, 0x00, 0x01, 0x01, 0x02, 0xfe,
    0xfe, 0xff, 0xff, 0x00, 0x00, 0x00, 0x01, 0x01, 0xfe, 0xfe, 0xff, 0xff, 0x00, 0x00, 0x00, 0x01,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const uint16_t SLCSZ_THRDELTAI_U16[52] =
{   //assuming Mb lag = 8
    100, 100, 100, 100, 100, 100, 100, 100, 100, 100,   //[0-9]
    100, 100, 100, 100, 100, 100, 100, 100, 100, 100,   //[10-19]
    100, 100, 100, 100, 100, 100, 100, 100, 100, 100,   //[20-29]
    100, 100, 100, 100, 100, 100, 100, 100, 100, 100,   //[30-39]
    100, 100, 100, 100, 100, 100, 100, 100, 100, 100,   //[40-49]
    100, 100                                            //[50-51]
};

const uint16_t SLCSZ_THRDELTAP_U16[52] =
{   //assuming Mb lag = 6
    100, 100, 100, 100, 100, 100, 100, 100, 100, 100,   //[0-9]
    100, 100, 100, 100, 100, 100, 100, 100, 100, 100,   //[10-19]
    100, 100, 100, 100, 100, 100, 100, 100, 100, 100,   //[20-29]
    100, 100, 100, 100, 100, 100, 100, 100, 100, 100,   //[30-39]
    100, 100, 100, 100, 100, 100, 100, 100, 100, 100,   //[40-49]
    100, 100                                            //[50-51]
};

enum
{
    // 2x down-scaling kernel
    CODECHAL_HEVC_SCALING_FRAME_BEGIN = 0,
    CODECHAL_HEVC_SCALING_FRAME_SRC_Y = CODECHAL_HEVC_SCALING_FRAME_BEGIN + 0,
    CODECHAL_HEVC_SCALING_FRAME_DST_Y = CODECHAL_HEVC_SCALING_FRAME_BEGIN + 1,
    CODECHAL_HEVC_SCALING_FRAME_END = CODECHAL_HEVC_SCALING_FRAME_BEGIN + 2,

    // 32x32 PU mode decision kernel
    CODECHAL_HEVC_32x32_PU_BEGIN = CODECHAL_HEVC_SCALING_FRAME_END,
    CODECHAL_HEVC_32x32_PU_OUTPUT = CODECHAL_HEVC_32x32_PU_BEGIN + 0,
    CODECHAL_HEVC_32x32_PU_SRC_Y = CODECHAL_HEVC_32x32_PU_BEGIN + 1,
    CODECHAL_HEVC_32x32_PU_SRC_UV = CODECHAL_HEVC_32x32_PU_BEGIN + 2,
    CODECHAL_HEVC_32x32_PU_SRC_Y2x = CODECHAL_HEVC_32x32_PU_BEGIN + 3,
    CODECHAL_HEVC_32x32_PU_SLICE_MAP = CODECHAL_HEVC_32x32_PU_BEGIN + 4,
    CODECHAL_HEVC_32x32_PU_SRC_Y2x_VME = CODECHAL_HEVC_32x32_PU_BEGIN + 5,
    CODECHAL_HEVC_32x32_PU_BRC_Input = CODECHAL_HEVC_32x32_PU_BEGIN + 6,
    CODECHAL_HEVC_32x32_PU_LCU_QP = CODECHAL_HEVC_32x32_PU_BEGIN + 7,
    CODECHAL_HEVC_32x32_PU_BRC_DATA = CODECHAL_HEVC_32x32_PU_BEGIN + 8,
    CODECHAL_HEVC_32x32_PU_STATS_DATA = CODECHAL_HEVC_32x32_PU_BEGIN + 9,
    CODECHAL_HEVC_32x32_PU_DEBUG = CODECHAL_HEVC_32x32_PU_BEGIN + 10,
    CODECHAL_HEVC_32x32_PU_END = CODECHAL_HEVC_32x32_PU_BEGIN + 11,

    // 16x16 SAD computation kernel
    CODECHAL_HEVC_16x16_PU_SAD_BEGIN = CODECHAL_HEVC_32x32_PU_END,
    CODECHAL_HEVC_16x16_PU_SAD_SRC_Y = CODECHAL_HEVC_16x16_PU_SAD_BEGIN + 0,
    CODECHAL_HEVC_16x16_PU_SAD_SRC_UV = CODECHAL_HEVC_16x16_PU_SAD_BEGIN + 1,
    CODECHAL_HEVC_16x16_PU_SAD_OUTPUT = CODECHAL_HEVC_16x16_PU_SAD_BEGIN + 2,
    CODECHAL_HEVC_16x16_PU_SAD_32x32_MD_DATA = CODECHAL_HEVC_16x16_PU_SAD_BEGIN + 3,
    CODECHAL_HEVC_16x16_PU_SLICE_MAP = CODECHAL_HEVC_16x16_PU_SAD_BEGIN + 4,
    CODECHAL_HEVC_16x16_PU_Simplest_Intra = CODECHAL_HEVC_16x16_PU_SAD_BEGIN + 5,
    CODECHAL_HEVC_16x16_PU_SAD_DEBUG = CODECHAL_HEVC_16x16_PU_SAD_BEGIN + 6,
    CODECHAL_HEVC_16x16_PU_SAD_END = CODECHAL_HEVC_16x16_PU_SAD_BEGIN + 7,

    // 16x16 PU mode decision kernel
    CODECHAL_HEVC_16x16_PU_MD_BEGIN = CODECHAL_HEVC_16x16_PU_SAD_END,
    CODECHAL_HEVC_16x16_PU_MD_SRC_Y = CODECHAL_HEVC_16x16_PU_MD_BEGIN + 0,
    CODECHAL_HEVC_16x16_PU_MD_SRC_UV = CODECHAL_HEVC_16x16_PU_MD_BEGIN + 1,
    CODECHAL_HEVC_16x16_PU_MD_16x16_SAD_DATA = CODECHAL_HEVC_16x16_PU_MD_BEGIN + 2,
    CODECHAL_HEVC_16x16_PU_MD_PAK_OBJ = CODECHAL_HEVC_16x16_PU_MD_BEGIN + 3,
    CODECHAL_HEVC_16x16_PU_MD_32x32_MD_DATA = CODECHAL_HEVC_16x16_PU_MD_BEGIN + 4,
    CODECHAL_HEVC_16x16_PU_MD_VME_8x8_MD_DATA = CODECHAL_HEVC_16x16_PU_MD_BEGIN + 5,
    CODECHAL_HEVC_16x16_PU_MD_SLICE_MAP = CODECHAL_HEVC_16x16_PU_MD_BEGIN + 6,
    CODECHAL_HEVC_16x16_PU_MD_VME_SRC = CODECHAL_HEVC_16x16_PU_MD_BEGIN + 7,
    CODECHAL_HEVC_16x16_PU_MD_BRC_Input = CODECHAL_HEVC_16x16_PU_MD_BEGIN + 8,
    CODECHAL_HEVC_16x16_PU_MD_Simplest_Intra = CODECHAL_HEVC_16x16_PU_MD_BEGIN + 9,
    CODECHAL_HEVC_16x16_PU_MD_LCU_QP = CODECHAL_HEVC_16x16_PU_MD_BEGIN + 10,
    CODECHAL_HEVC_16x16_PU_MD_BRC_Data = CODECHAL_HEVC_16x16_PU_MD_BEGIN + 11,
    CODECHAL_HEVC_16x16_PU_MD_DEBUG = CODECHAL_HEVC_16x16_PU_MD_BEGIN + 12,
    CODECHAL_HEVC_16x16_PU_MD_END = CODECHAL_HEVC_16x16_PU_MD_BEGIN + 13,

    // 8x8 PU kernel
    CODECHAL_HEVC_8x8_PU_BEGIN = CODECHAL_HEVC_16x16_PU_MD_END,
    CODECHAL_HEVC_8x8_PU_SRC_Y = CODECHAL_HEVC_8x8_PU_BEGIN + 0,
    CODECHAL_HEVC_8x8_PU_SRC_UV = CODECHAL_HEVC_8x8_PU_BEGIN + 1,
    CODECHAL_HEVC_8x8_PU_SLICE_MAP = CODECHAL_HEVC_8x8_PU_BEGIN + 2,
    CODECHAL_HEVC_8x8_PU_VME_8x8_MODE = CODECHAL_HEVC_8x8_PU_BEGIN + 3,
    CODECHAL_HEVC_8x8_PU_INTRA_MODE = CODECHAL_HEVC_8x8_PU_BEGIN + 4,
    CODECHAL_HEVC_8x8_PU_BRC_Input = CODECHAL_HEVC_8x8_PU_BEGIN + 5,
    CODECHAL_HEVC_8x8_PU_Simplest_Intra = CODECHAL_HEVC_8x8_PU_BEGIN + 6,
    CODECHAL_HEVC_8x8_PU_LCU_QP = CODECHAL_HEVC_8x8_PU_BEGIN + 7,
    CODECHAL_HEVC_8x8_PU_BRC_Data = CODECHAL_HEVC_8x8_PU_BEGIN + 8,
    CODECHAL_HEVC_8x8_PU_DEBUG = CODECHAL_HEVC_8x8_PU_BEGIN + 9,
    CODECHAL_HEVC_8x8_PU_END = CODECHAL_HEVC_8x8_PU_BEGIN + 10,

    // 8x8 PU FMODE kernel
    CODECHAL_HEVC_8x8_PU_FMODE_BEGIN = CODECHAL_HEVC_8x8_PU_END,
    CODECHAL_HEVC_8x8_PU_FMODE_PAK_OBJECT = CODECHAL_HEVC_8x8_PU_FMODE_BEGIN + 0,
    CODECHAL_HEVC_8x8_PU_FMODE_VME_8x8_MODE = CODECHAL_HEVC_8x8_PU_FMODE_BEGIN + 1,
    CODECHAL_HEVC_8x8_PU_FMODE_INTRA_MODE = CODECHAL_HEVC_8x8_PU_FMODE_BEGIN + 2,
    CODECHAL_HEVC_8x8_PU_FMODE_PAK_COMMAND = CODECHAL_HEVC_8x8_PU_FMODE_BEGIN + 3,
    CODECHAL_HEVC_8x8_PU_FMODE_SLICE_MAP = CODECHAL_HEVC_8x8_PU_FMODE_BEGIN + 4,
    CODECHAL_HEVC_8x8_PU_FMODE_INTRADIST = CODECHAL_HEVC_8x8_PU_FMODE_BEGIN + 5,
    CODECHAL_HEVC_8x8_PU_FMODE_BRC_Input = CODECHAL_HEVC_8x8_PU_FMODE_BEGIN + 6,
    CODECHAL_HEVC_8x8_PU_FMODE_Simplest_Intra = CODECHAL_HEVC_8x8_PU_FMODE_BEGIN + 7,
    CODECHAL_HEVC_8x8_PU_FMODE_LCU_QP = CODECHAL_HEVC_8x8_PU_FMODE_BEGIN + 8,
    CODECHAL_HEVC_8x8_PU_FMODE_BRC_Data = CODECHAL_HEVC_8x8_PU_FMODE_BEGIN + 9,
    CODECHAL_HEVC_8x8_PU_FMODE_HAAR_DIST16x16 = CODECHAL_HEVC_8x8_PU_FMODE_BEGIN + 10,
    CODECHAL_HEVC_8x8_PU_FMODE_STATS_DATA = CODECHAL_HEVC_8x8_PU_FMODE_BEGIN + 11,
    CODECHAL_HEVC_8x8_PU_FMODE_FRAME_STATS_DATA = CODECHAL_HEVC_8x8_PU_FMODE_BEGIN + 12,
    CODECHAL_HEVC_8x8_PU_FMODE_DEBUG = CODECHAL_HEVC_8x8_PU_FMODE_BEGIN + 13,
    CODECHAL_HEVC_8x8_PU_FMODE_END = CODECHAL_HEVC_8x8_PU_FMODE_BEGIN + 14,

    // B 32x32 PU intra check kernel
    CODECHAL_HEVC_B_32x32_PU_BEGIN = CODECHAL_HEVC_8x8_PU_FMODE_END,
    CODECHAL_HEVC_B_32x32_PU_OUTPUT = CODECHAL_HEVC_B_32x32_PU_BEGIN + 0,
    CODECHAL_HEVC_B_32x32_PU_SRC_Y = CODECHAL_HEVC_B_32x32_PU_BEGIN + 1,
    CODECHAL_HEVC_B_32x32_PU_SRC_UV = CODECHAL_HEVC_B_32x32_PU_BEGIN + 2,
    CODECHAL_HEVC_B_32x32_PU_SRC_Y2x = CODECHAL_HEVC_B_32x32_PU_BEGIN + 3,
    CODECHAL_HEVC_B_32x32_PU_SLICE_MAP = CODECHAL_HEVC_B_32x32_PU_BEGIN + 4,
    CODECHAL_HEVC_B_32x32_PU_SRC_Y2x_VME = CODECHAL_HEVC_B_32x32_PU_BEGIN + 5,
    CODECHAL_HEVC_B_32x32_PU_Simplest_Intra = CODECHAL_HEVC_B_32x32_PU_BEGIN + 6,
    CODECHAL_HEVC_B_32x32_PU_HME_MVP = CODECHAL_HEVC_B_32x32_PU_BEGIN + 7,
    CODECHAL_HEVC_B_32x32_PU_HME_DIST = CODECHAL_HEVC_B_32x32_PU_BEGIN + 8,
    CODECHAL_HEVC_B_32x32_PU_LCU_SKIP = CODECHAL_HEVC_B_32x32_PU_BEGIN + 9,
    CODECHAL_HEVC_B_32x32_PU_DEBUG = CODECHAL_HEVC_B_32x32_PU_BEGIN + 10,
    CODECHAL_HEVC_B_32x32_PU_END = CODECHAL_HEVC_B_32x32_PU_BEGIN + 11,

    // B MB ENC kernel
    CODECHAL_HEVC_B_MBENC_BEGIN = CODECHAL_HEVC_B_32x32_PU_END,
    CODECHAL_HEVC_B_MBENC_CU_RECORD = CODECHAL_HEVC_B_MBENC_BEGIN + 0,
    CODECHAL_HEVC_B_MBENC_PAK_CMD = CODECHAL_HEVC_B_MBENC_BEGIN + 1,
    CODECHAL_HEVC_B_MBENC_SRC_Y = CODECHAL_HEVC_B_MBENC_BEGIN + 2,
    CODECHAL_HEVC_B_MBENC_SRC_UV = CODECHAL_HEVC_B_MBENC_BEGIN + 3,
    CODECHAL_HEVC_B_MBENC_INTRA_DIST = CODECHAL_HEVC_B_MBENC_BEGIN + 4,
    CODECHAL_HEVC_B_MBENC_MIN_DIST = CODECHAL_HEVC_B_MBENC_BEGIN + 5,
    CODECHAL_HEVC_B_MBENC_HME_MVP = CODECHAL_HEVC_B_MBENC_BEGIN + 6,
    CODECHAL_HEVC_B_MBENC_HME_DIST = CODECHAL_HEVC_B_MBENC_BEGIN + 7,
    CODECHAL_HEVC_B_MBENC_SLICE_MAP = CODECHAL_HEVC_B_MBENC_BEGIN + 8,
    CODECHAL_HEVC_B_MBENC_VME_UNISIC_DATA = CODECHAL_HEVC_B_MBENC_BEGIN + 9,
    CODECHAL_HEVC_B_MBENC_Simplest_Intra = CODECHAL_HEVC_B_MBENC_BEGIN + 10,
    CODECHAL_HEVC_B_MBENC_REF_COLLOC = CODECHAL_HEVC_B_MBENC_BEGIN + 11,
    CODECHAL_HEVC_B_MBENC_Reserved = CODECHAL_HEVC_B_MBENC_BEGIN + 12,
    CODECHAL_HEVC_B_MBENC_BRC_Input = CODECHAL_HEVC_B_MBENC_BEGIN + 13,
    CODECHAL_HEVC_B_MBENC_LCU_QP = CODECHAL_HEVC_B_MBENC_BEGIN + 14,
    CODECHAL_HEVC_B_MBENC_BRC_DATA = CODECHAL_HEVC_B_MBENC_BEGIN + 15,
    //
    CODECHAL_HEVC_B_MBENC_VME_CURRENT = CODECHAL_HEVC_B_MBENC_BEGIN + 16,
    CODECHAL_HEVC_B_MBENC_VME_FORWARD_0 = CODECHAL_HEVC_B_MBENC_BEGIN + 17,
    CODECHAL_HEVC_B_MBENC_VME_BACKWARD_0 = CODECHAL_HEVC_B_MBENC_BEGIN + 18,
    CODECHAL_HEVC_B_MBENC_VME_FORWARD_1 = CODECHAL_HEVC_B_MBENC_BEGIN + 19,
    CODECHAL_HEVC_B_MBENC_VME_BACKWARD_1 = CODECHAL_HEVC_B_MBENC_BEGIN + 20,
    CODECHAL_HEVC_B_MBENC_VME_FORWARD_2 = CODECHAL_HEVC_B_MBENC_BEGIN + 21,
    CODECHAL_HEVC_B_MBENC_VME_BACKWARD_2 = CODECHAL_HEVC_B_MBENC_BEGIN + 22,
    CODECHAL_HEVC_B_MBENC_VME_FORWARD_3 = CODECHAL_HEVC_B_MBENC_BEGIN + 23,
    CODECHAL_HEVC_B_MBENC_VME_BACKWARD_3 = CODECHAL_HEVC_B_MBENC_BEGIN + 24,
    CODECHAL_HEVC_B_MBENC_VME_FORWARD_4 = CODECHAL_HEVC_B_MBENC_BEGIN + 25,
    CODECHAL_HEVC_B_MBENC_VME_BACKWARD_4 = CODECHAL_HEVC_B_MBENC_BEGIN + 26,
    CODECHAL_HEVC_B_MBENC_VME_FORWARD_5 = CODECHAL_HEVC_B_MBENC_BEGIN + 27,
    CODECHAL_HEVC_B_MBENC_VME_BACKWARD_5 = CODECHAL_HEVC_B_MBENC_BEGIN + 28,
    CODECHAL_HEVC_B_MBENC_VME_FORWARD_6 = CODECHAL_HEVC_B_MBENC_BEGIN + 29,
    CODECHAL_HEVC_B_MBENC_VME_BACKWARD_6 = CODECHAL_HEVC_B_MBENC_BEGIN + 30,
    CODECHAL_HEVC_B_MBENC_VME_FORWARD_7 = CODECHAL_HEVC_B_MBENC_BEGIN + 31,
    CODECHAL_HEVC_B_MBENC_VME_BACKWARD_7 = CODECHAL_HEVC_B_MBENC_BEGIN + 32,
    //
    CODECHAL_HEVC_B_MBENC_VME_MUL_CURRENT = CODECHAL_HEVC_B_MBENC_BEGIN + 33,
    CODECHAL_HEVC_B_MBENC_VME_MUL_BACKWARD_0 = CODECHAL_HEVC_B_MBENC_BEGIN + 34,
    CODECHAL_HEVC_B_MBENC_VME_MUL_NOUSE_0 = CODECHAL_HEVC_B_MBENC_BEGIN + 35,
    CODECHAL_HEVC_B_MBENC_VME_MUL_BACKWARD_1 = CODECHAL_HEVC_B_MBENC_BEGIN + 36,
    CODECHAL_HEVC_B_MBENC_VME_MUL_NOUSE_1 = CODECHAL_HEVC_B_MBENC_BEGIN + 37,
    CODECHAL_HEVC_B_MBENC_VME_MUL_BACKWARD_2 = CODECHAL_HEVC_B_MBENC_BEGIN + 38,
    CODECHAL_HEVC_B_MBENC_VME_MUL_NOUSE_2 = CODECHAL_HEVC_B_MBENC_BEGIN + 39,
    CODECHAL_HEVC_B_MBENC_VME_MUL_BACKWARD_3 = CODECHAL_HEVC_B_MBENC_BEGIN + 40,
    CODECHAL_HEVC_B_MBENC_VME_MUL_NOUSE_3 = CODECHAL_HEVC_B_MBENC_BEGIN + 41,
    //
    CODECHAL_HEVC_B_MBENC_CONCURRENT_THD_MAP = CODECHAL_HEVC_B_MBENC_BEGIN + 42,
    CODECHAL_HEVC_B_MBENC_MV_IDX = CODECHAL_HEVC_B_MBENC_BEGIN + 43,
    CODECHAL_HEVC_B_MBENC_MVP_IDX = CODECHAL_HEVC_B_MBENC_BEGIN + 44,
    CODECHAL_HEVC_B_MBENC_HAAR_DIST16x16 = CODECHAL_HEVC_B_MBENC_BEGIN + 45,
    CODECHAL_HEVC_B_MBENC_STATS_DATA = CODECHAL_HEVC_B_MBENC_BEGIN + 46,
    CODECHAL_HEVC_B_MBENC_FRAME_STATS_DATA = CODECHAL_HEVC_B_MBENC_BEGIN + 47,
    CODECHAL_HEVC_B_MBENC_DEBUG = CODECHAL_HEVC_B_MBENC_BEGIN + 48,
    //
    CODECHAL_HEVC_B_MBENC_END = CODECHAL_HEVC_B_MBENC_BEGIN + 49,

    //Coarse Intra distortion
    CODECHAL_HEVC_COARSE_INTRA_BEGIN = CODECHAL_HEVC_B_MBENC_END,
    CODECHAL_HEVC_COARSE_INTRA_Y4 = CODECHAL_HEVC_COARSE_INTRA_BEGIN + 0,
    CODECHAL_HEVC_COARSE_INTRA_DISTORTION = CODECHAL_HEVC_COARSE_INTRA_BEGIN + 1,
    CODECHAL_HEVC_COARSE_INTRA_VME_Y4 = CODECHAL_HEVC_COARSE_INTRA_BEGIN + 2,
    CODECHAL_HEVC_COARSE_INTRA_END = CODECHAL_HEVC_COARSE_INTRA_BEGIN + 3,

    // HEVC B PAK kernel
    CODECHAL_HEVC_B_PAK_BEGIN = CODECHAL_HEVC_COARSE_INTRA_END,
    CODECHAL_HEVC_B_PAK_CU_RECORD = CODECHAL_HEVC_B_PAK_BEGIN + 0,
    CODECHAL_HEVC_B_PAK_PAK_OBJ = CODECHAL_HEVC_B_PAK_BEGIN + 1,
    CODECHAL_HEVC_B_PAK_SLICE_MAP = CODECHAL_HEVC_B_PAK_BEGIN + 2,
    CODECHAL_HEVC_B_PAK_BRC_INPUT = CODECHAL_HEVC_B_PAK_BEGIN + 3,
    CODECHAL_HEVC_B_PAK_LCU_QP = CODECHAL_HEVC_B_PAK_BEGIN + 4,
    CODECHAL_HEVC_B_PAK_BRC_DATA = CODECHAL_HEVC_B_PAK_BEGIN + 5,
    CODECHAL_HEVC_B_PAK_MB_DATA = CODECHAL_HEVC_B_PAK_BEGIN + 6,
    CODECHAL_HEVC_B_PAK_MVP_DATA = CODECHAL_HEVC_B_PAK_BEGIN + 7,
    CODECHAL_HEVC_B_PAK_WA_PAK_DATA = CODECHAL_HEVC_B_PAK_BEGIN + 8,
    CODECHAL_HEVC_B_PAK_WA_PAK_OBJ = CODECHAL_HEVC_B_PAK_BEGIN + 9,
    CODECHAL_HEVC_B_PAK_DEBUG = CODECHAL_HEVC_B_PAK_BEGIN + 10,
    CODECHAL_HEVC_B_PAK_END = CODECHAL_HEVC_B_PAK_BEGIN + 11,

    //HEVC FORMAT CONVERSION AND DOWNSCALING KERNEL
    CODECHAL_HEVC_DS_COMBINED_BEGIN = CODECHAL_HEVC_B_PAK_END,
    CODECHAL_HEVC_DS_COMBINED_10BIT_Y = CODECHAL_HEVC_DS_COMBINED_BEGIN + 0,
    CODECHAL_HEVC_DS_COMBINED_10BIT_UV = CODECHAL_HEVC_DS_COMBINED_BEGIN + 1,
    CODECHAL_HEVC_DS_COMBINED_8BIT_Y = CODECHAL_HEVC_DS_COMBINED_BEGIN + 2,
    CODECHAL_HEVC_DS_COMBINED_8BIT_UV = CODECHAL_HEVC_DS_COMBINED_BEGIN + 3,
    CODECHAL_HEVC_DS_COMBINED_4xDOWNSCALE = CODECHAL_HEVC_DS_COMBINED_BEGIN + 4,
    CODECHAL_HEVC_DS_COMBINED_MB_STATS = CODECHAL_HEVC_DS_COMBINED_BEGIN + 5,
    CODECHAL_HEVC_DS_COMBINED_2xDOWNSCALE = CODECHAL_HEVC_DS_COMBINED_BEGIN + 6,
    CODECHAL_HEVC_DS_COMBINED_END = CODECHAL_HEVC_DS_COMBINED_BEGIN + 7,

    //BRC Init/Reset
    CODECHAL_HEVC_BRC_INIT_RESET_BEGIN = CODECHAL_HEVC_DS_COMBINED_END,
    CODECHAL_HEVC_BRC_INIT_RESET_HISTORY = CODECHAL_HEVC_BRC_INIT_RESET_BEGIN + 0,
    CODECHAL_HEVC_BRC_INIT_RESET_DISTORTION = CODECHAL_HEVC_BRC_INIT_RESET_BEGIN + 1,
    CODECHAL_HEVC_BRC_INIT_RESET_END = CODECHAL_HEVC_BRC_INIT_RESET_BEGIN + 2,

    //BRC Update (frame based)
    CODECHAL_HEVC_BRC_UPDATE_BEGIN = CODECHAL_HEVC_BRC_INIT_RESET_END,
    CODECHAL_HEVC_BRC_UPDATE_HISTORY = CODECHAL_HEVC_BRC_UPDATE_BEGIN + 0,
    CODECHAL_HEVC_BRC_UPDATE_PREV_PAK = CODECHAL_HEVC_BRC_UPDATE_BEGIN + 1,
    CODECHAL_HEVC_BRC_UPDATE_PIC_STATE_R = CODECHAL_HEVC_BRC_UPDATE_BEGIN + 2,
    CODECHAL_HEVC_BRC_UPDATE_PIC_STATE_W = CODECHAL_HEVC_BRC_UPDATE_BEGIN + 3,
    CODECHAL_HEVC_BRC_UPDATE_ENC_OUTPUT = CODECHAL_HEVC_BRC_UPDATE_BEGIN + 4,
    CODECHAL_HEVC_BRC_UPDATE_DISTORTION = CODECHAL_HEVC_BRC_UPDATE_BEGIN + 5,
    CODECHAL_HEVC_BRC_UPDATE_BRCDATA = CODECHAL_HEVC_BRC_UPDATE_BEGIN + 6,
    CODECHAL_HEVC_BRC_UPDATE_END = CODECHAL_HEVC_BRC_UPDATE_BEGIN + 7,

    //BRC Update (LCU-based)
    CODECHAL_HEVC_BRC_LCU_UPDATE_BEGIN = CODECHAL_HEVC_BRC_UPDATE_END,
    CODECHAL_HEVC_BRC_LCU_UPDATE_HISTORY = CODECHAL_HEVC_BRC_LCU_UPDATE_BEGIN + 0,
    CODECHAL_HEVC_BRC_LCU_UPDATE_HME_DISTORTION = CODECHAL_HEVC_BRC_LCU_UPDATE_BEGIN + 1,
    CODECHAL_HEVC_BRC_LCU_UPDATE_INTRA_DISTORTION = CODECHAL_HEVC_BRC_LCU_UPDATE_BEGIN + 2,
    CODECHAL_HEVC_BRC_LCU_UPDATE_HME_MVP = CODECHAL_HEVC_BRC_LCU_UPDATE_BEGIN + 3,
    CODECHAL_HEVC_BRC_LCU_UPDATE_LCU_QP = CODECHAL_HEVC_BRC_LCU_UPDATE_BEGIN + 4,
    CODECHAL_HEVC_BRC_LCU_UPDATE_ROI = CODECHAL_HEVC_BRC_LCU_UPDATE_BEGIN + 5,
    CODECHAL_HEVC_BRC_LCU_UPDATE_END = CODECHAL_HEVC_BRC_LCU_UPDATE_BEGIN + 6,

    // P MB ENC kernel
    CODECHAL_HEVC_P_MBENC_BEGIN = CODECHAL_HEVC_BRC_LCU_UPDATE_END,
    CODECHAL_HEVC_P_MBENC_CU_RECORD = CODECHAL_HEVC_P_MBENC_BEGIN + 0,
    CODECHAL_HEVC_P_MBENC_PAK_CMD = CODECHAL_HEVC_P_MBENC_BEGIN + 1,
    CODECHAL_HEVC_P_MBENC_SRC_Y = CODECHAL_HEVC_P_MBENC_BEGIN + 2,
    CODECHAL_HEVC_P_MBENC_SRC_UV = CODECHAL_HEVC_P_MBENC_BEGIN + 3,
    CODECHAL_HEVC_P_MBENC_INTRA_DIST = CODECHAL_HEVC_P_MBENC_BEGIN + 4,
    CODECHAL_HEVC_P_MBENC_MIN_DIST = CODECHAL_HEVC_P_MBENC_BEGIN + 5,
    CODECHAL_HEVC_P_MBENC_HME_MVP = CODECHAL_HEVC_P_MBENC_BEGIN + 6,
    CODECHAL_HEVC_P_MBENC_HME_DIST = CODECHAL_HEVC_P_MBENC_BEGIN + 7,
    CODECHAL_HEVC_P_MBENC_SLICE_MAP = CODECHAL_HEVC_P_MBENC_BEGIN + 8,
    CODECHAL_HEVC_P_MBENC_VME_UNISIC_DATA = CODECHAL_HEVC_P_MBENC_BEGIN + 9,
    CODECHAL_HEVC_P_MBENC_Simplest_Intra = CODECHAL_HEVC_P_MBENC_BEGIN + 10,
    CODECHAL_HEVC_P_MBENC_REF_COLLOC = CODECHAL_HEVC_P_MBENC_BEGIN + 11,
    CODECHAL_HEVC_P_MBENC_Reserved = CODECHAL_HEVC_P_MBENC_BEGIN + 12,
    CODECHAL_HEVC_P_MBENC_BRC_Input = CODECHAL_HEVC_P_MBENC_BEGIN + 13,
    CODECHAL_HEVC_P_MBENC_LCU_QP = CODECHAL_HEVC_P_MBENC_BEGIN + 14,
    CODECHAL_HEVC_P_MBENC_BRC_DATA = CODECHAL_HEVC_P_MBENC_BEGIN + 15,
    //
    CODECHAL_HEVC_P_MBENC_VME_CURRENT = CODECHAL_HEVC_P_MBENC_BEGIN + 16,
    CODECHAL_HEVC_P_MBENC_VME_FORWARD_0 = CODECHAL_HEVC_P_MBENC_BEGIN + 17,
    CODECHAL_HEVC_P_MBENC_VME_FORWARD_1 = CODECHAL_HEVC_P_MBENC_BEGIN + 19,
    CODECHAL_HEVC_P_MBENC_VME_FORWARD_2 = CODECHAL_HEVC_P_MBENC_BEGIN + 21,
    CODECHAL_HEVC_P_MBENC_VME_FORWARD_3 = CODECHAL_HEVC_P_MBENC_BEGIN + 23,
    CODECHAL_HEVC_P_MBENC_VME_FORWARD_4 = CODECHAL_HEVC_P_MBENC_BEGIN + 25,
    CODECHAL_HEVC_P_MBENC_VME_FORWARD_5 = CODECHAL_HEVC_P_MBENC_BEGIN + 27,
    CODECHAL_HEVC_P_MBENC_VME_FORWARD_6 = CODECHAL_HEVC_P_MBENC_BEGIN + 29,
    CODECHAL_HEVC_P_MBENC_VME_FORWARD_7 = CODECHAL_HEVC_P_MBENC_BEGIN + 31,

    //
    CODECHAL_HEVC_P_MBENC_CONCURRENT_THD_MAP = CODECHAL_HEVC_P_MBENC_BEGIN + 33,
    CODECHAL_HEVC_P_MBENC_MV_IDX = CODECHAL_HEVC_P_MBENC_BEGIN + 34,
    CODECHAL_HEVC_P_MBENC_MVP_IDX = CODECHAL_HEVC_P_MBENC_BEGIN + 35,
    CODECHAL_HEVC_P_MBENC_DEBUG = CODECHAL_HEVC_P_MBENC_BEGIN + 36,
    //
    CODECHAL_HEVC_P_MBENC_END = CODECHAL_HEVC_P_MBENC_BEGIN + 37,

    CODECHAL_HEVC_NUM_SURFACES = CODECHAL_HEVC_P_MBENC_END
};

enum
{
    // 2x down-scaling kernel
    CODECHAL_HEVC_FEI_SCALING_FRAME_BEGIN = 0,
    CODECHAL_HEVC_FEI_SCALING_FRAME_SRC_Y = CODECHAL_HEVC_FEI_SCALING_FRAME_BEGIN + 0,
    CODECHAL_HEVC_FEI_SCALING_FRAME_DST_Y = CODECHAL_HEVC_FEI_SCALING_FRAME_BEGIN + 1,
    CODECHAL_HEVC_FEI_SCALING_FRAME_END = CODECHAL_HEVC_FEI_SCALING_FRAME_BEGIN + 2,

    // 32x32 PU mode decision kernel
    CODECHAL_HEVC_FEI_32x32_PU_BEGIN = CODECHAL_HEVC_FEI_SCALING_FRAME_END,
    CODECHAL_HEVC_FEI_32x32_PU_OUTPUT = CODECHAL_HEVC_FEI_32x32_PU_BEGIN + 0,
    CODECHAL_HEVC_FEI_32x32_PU_SRC_Y = CODECHAL_HEVC_FEI_32x32_PU_BEGIN + 1,
    CODECHAL_HEVC_FEI_32x32_PU_SRC_UV = CODECHAL_HEVC_FEI_32x32_PU_BEGIN + 2,
    CODECHAL_HEVC_FEI_32x32_PU_SRC_Y2x = CODECHAL_HEVC_FEI_32x32_PU_BEGIN + 3,
    CODECHAL_HEVC_FEI_32x32_PU_SLICE_MAP = CODECHAL_HEVC_FEI_32x32_PU_BEGIN + 4,
    CODECHAL_HEVC_FEI_32x32_PU_SRC_Y2x_VME = CODECHAL_HEVC_FEI_32x32_PU_BEGIN + 5,
    CODECHAL_HEVC_FEI_32x32_PU_BRC_Input = CODECHAL_HEVC_FEI_32x32_PU_BEGIN + 6,
    CODECHAL_HEVC_FEI_32x32_PU_LCU_QP = CODECHAL_HEVC_FEI_32x32_PU_BEGIN + 7,
    CODECHAL_HEVC_FEI_32x32_PU_BRC_DATA = CODECHAL_HEVC_FEI_32x32_PU_BEGIN + 8,
    CODECHAL_HEVC_FEI_32x32_PU_STATS_DATA = CODECHAL_HEVC_FEI_32x32_PU_BEGIN + 9,
    CODECHAL_HEVC_FEI_32x32_PU_DEBUG = CODECHAL_HEVC_FEI_32x32_PU_BEGIN + 10,
    CODECHAL_HEVC_FEI_32x32_PU_END = CODECHAL_HEVC_FEI_32x32_PU_BEGIN + 11,

    // 16x16 SAD computation kernel
    CODECHAL_HEVC_FEI_16x16_PU_SAD_BEGIN = CODECHAL_HEVC_FEI_32x32_PU_END,
    CODECHAL_HEVC_FEI_16x16_PU_SAD_SRC_Y = CODECHAL_HEVC_FEI_16x16_PU_SAD_BEGIN + 0,
    CODECHAL_HEVC_FEI_16x16_PU_SAD_SRC_UV = CODECHAL_HEVC_FEI_16x16_PU_SAD_BEGIN + 1,
    CODECHAL_HEVC_FEI_16x16_PU_SAD_OUTPUT = CODECHAL_HEVC_FEI_16x16_PU_SAD_BEGIN + 2,
    CODECHAL_HEVC_FEI_16x16_PU_SAD_32x32_MD_DATA = CODECHAL_HEVC_FEI_16x16_PU_SAD_BEGIN + 3,
    CODECHAL_HEVC_FEI_16x16_PU_SLICE_MAP = CODECHAL_HEVC_FEI_16x16_PU_SAD_BEGIN + 4,
    CODECHAL_HEVC_FEI_16x16_PU_Simplest_Intra = CODECHAL_HEVC_FEI_16x16_PU_SAD_BEGIN + 5,
    CODECHAL_HEVC_FEI_16x16_PU_SAD_DEBUG = CODECHAL_HEVC_FEI_16x16_PU_SAD_BEGIN + 6,
    CODECHAL_HEVC_FEI_16x16_PU_SAD_END = CODECHAL_HEVC_FEI_16x16_PU_SAD_BEGIN + 7,

    // 16x16 PU mode decision kernel
    CODECHAL_HEVC_FEI_16x16_PU_MD_BEGIN = CODECHAL_HEVC_FEI_16x16_PU_SAD_END,
    CODECHAL_HEVC_FEI_16x16_PU_MD_SRC_Y = CODECHAL_HEVC_FEI_16x16_PU_MD_BEGIN + 0,
    CODECHAL_HEVC_FEI_16x16_PU_MD_SRC_UV = CODECHAL_HEVC_FEI_16x16_PU_MD_BEGIN + 1,
    CODECHAL_HEVC_FEI_16x16_PU_MD_16x16_SAD_DATA = CODECHAL_HEVC_FEI_16x16_PU_MD_BEGIN + 2,
    CODECHAL_HEVC_FEI_16x16_PU_MD_PAK_OBJ = CODECHAL_HEVC_FEI_16x16_PU_MD_BEGIN + 3,
    CODECHAL_HEVC_FEI_16x16_PU_MD_32x32_MD_DATA = CODECHAL_HEVC_FEI_16x16_PU_MD_BEGIN + 4,
    CODECHAL_HEVC_FEI_16x16_PU_MD_VME_8x8_MD_DATA = CODECHAL_HEVC_FEI_16x16_PU_MD_BEGIN + 5,
    CODECHAL_HEVC_FEI_16x16_PU_MD_SLICE_MAP = CODECHAL_HEVC_FEI_16x16_PU_MD_BEGIN + 6,
    CODECHAL_HEVC_FEI_16x16_PU_MD_VME_SRC = CODECHAL_HEVC_FEI_16x16_PU_MD_BEGIN + 7,
    CODECHAL_HEVC_FEI_16x16_PU_MD_BRC_Input = CODECHAL_HEVC_FEI_16x16_PU_MD_BEGIN + 8,
    CODECHAL_HEVC_FEI_16x16_PU_MD_Simplest_Intra = CODECHAL_HEVC_FEI_16x16_PU_MD_BEGIN + 9,
    CODECHAL_HEVC_FEI_16x16_PU_MD_LCU_QP = CODECHAL_HEVC_FEI_16x16_PU_MD_BEGIN + 10,
    CODECHAL_HEVC_FEI_16x16_PU_MD_BRC_Data = CODECHAL_HEVC_FEI_16x16_PU_MD_BEGIN + 11,
    CODECHAL_HEVC_FEI_16x16_PU_MD_DEBUG = CODECHAL_HEVC_FEI_16x16_PU_MD_BEGIN + 12,
    CODECHAL_HEVC_FEI_16x16_PU_MD_END = CODECHAL_HEVC_FEI_16x16_PU_MD_BEGIN + 13,

    // 8x8 PU kernel
    CODECHAL_HEVC_FEI_8x8_PU_BEGIN = CODECHAL_HEVC_FEI_16x16_PU_MD_END,
    CODECHAL_HEVC_FEI_8x8_PU_SRC_Y = CODECHAL_HEVC_FEI_8x8_PU_BEGIN + 0,
    CODECHAL_HEVC_FEI_8x8_PU_SRC_UV = CODECHAL_HEVC_FEI_8x8_PU_BEGIN + 1,
    CODECHAL_HEVC_FEI_8x8_PU_SLICE_MAP = CODECHAL_HEVC_FEI_8x8_PU_BEGIN + 2,
    CODECHAL_HEVC_FEI_8x8_PU_VME_8x8_MODE = CODECHAL_HEVC_FEI_8x8_PU_BEGIN + 3,
    CODECHAL_HEVC_FEI_8x8_PU_INTRA_MODE = CODECHAL_HEVC_FEI_8x8_PU_BEGIN + 4,
    CODECHAL_HEVC_FEI_8x8_PU_BRC_Input = CODECHAL_HEVC_FEI_8x8_PU_BEGIN + 5,
    CODECHAL_HEVC_FEI_8x8_PU_Simplest_Intra = CODECHAL_HEVC_FEI_8x8_PU_BEGIN + 6,
    CODECHAL_HEVC_FEI_8x8_PU_LCU_QP = CODECHAL_HEVC_FEI_8x8_PU_BEGIN + 7,
    CODECHAL_HEVC_FEI_8x8_PU_BRC_Data = CODECHAL_HEVC_FEI_8x8_PU_BEGIN + 8,
    CODECHAL_HEVC_FEI_8x8_PU_DEBUG = CODECHAL_HEVC_FEI_8x8_PU_BEGIN + 9,
    CODECHAL_HEVC_FEI_8x8_PU_END = CODECHAL_HEVC_FEI_8x8_PU_BEGIN + 10,

    // 8x8 PU FMODE kernel
    CODECHAL_HEVC_FEI_8x8_PU_FMODE_BEGIN = CODECHAL_HEVC_FEI_8x8_PU_END,
    CODECHAL_HEVC_FEI_8x8_PU_FMODE_PAK_OBJECT = CODECHAL_HEVC_FEI_8x8_PU_FMODE_BEGIN + 0,
    CODECHAL_HEVC_FEI_8x8_PU_FMODE_VME_8x8_MODE = CODECHAL_HEVC_FEI_8x8_PU_FMODE_BEGIN + 1,
    CODECHAL_HEVC_FEI_8x8_PU_FMODE_INTRA_MODE = CODECHAL_HEVC_FEI_8x8_PU_FMODE_BEGIN + 2,
    CODECHAL_HEVC_FEI_8x8_PU_FMODE_PAK_COMMAND = CODECHAL_HEVC_FEI_8x8_PU_FMODE_BEGIN + 3,
    CODECHAL_HEVC_FEI_8x8_PU_FMODE_SLICE_MAP = CODECHAL_HEVC_FEI_8x8_PU_FMODE_BEGIN + 4,
    CODECHAL_HEVC_FEI_8x8_PU_FMODE_INTRADIST = CODECHAL_HEVC_FEI_8x8_PU_FMODE_BEGIN + 5,
    CODECHAL_HEVC_FEI_8x8_PU_FMODE_BRC_Input = CODECHAL_HEVC_FEI_8x8_PU_FMODE_BEGIN + 6,
    CODECHAL_HEVC_FEI_8x8_PU_FMODE_Simplest_Intra = CODECHAL_HEVC_FEI_8x8_PU_FMODE_BEGIN + 7,
    CODECHAL_HEVC_FEI_8x8_PU_FMODE_LCU_QP = CODECHAL_HEVC_FEI_8x8_PU_FMODE_BEGIN + 8,
    CODECHAL_HEVC_FEI_8x8_PU_FMODE_BRC_Data = CODECHAL_HEVC_FEI_8x8_PU_FMODE_BEGIN + 9,
    CODECHAL_HEVC_FEI_8x8_PU_FMODE_HAAR16x16 = CODECHAL_HEVC_FEI_8x8_PU_FMODE_BEGIN + 10,
    CODECHAL_HEVC_FEI_8x8_PU_FMODE_STATS_DATA = CODECHAL_HEVC_FEI_8x8_PU_FMODE_BEGIN + 11,
    CODECHAL_HEVC_FEI_8X8_PU_FMODE_FRAME_STATS_DATA = CODECHAL_HEVC_FEI_8x8_PU_FMODE_BEGIN + 12,
    CODECHAL_HEVC_FEI_8X8_PU_FMODE_CTB_DISTORTION = CODECHAL_HEVC_FEI_8x8_PU_FMODE_BEGIN + 13,
    CODECHAL_HEVC_FEI_8x8_PU_FMODE_DEBUG = CODECHAL_HEVC_FEI_8x8_PU_FMODE_BEGIN + 14,
    CODECHAL_HEVC_FEI_8x8_PU_FMODE_END = CODECHAL_HEVC_FEI_8x8_PU_FMODE_BEGIN + 15,

    // B 32x32 PU intra check kernel
    CODECHAL_HEVC_FEI_B_32x32_PU_BEGIN = CODECHAL_HEVC_FEI_8x8_PU_FMODE_END,
    CODECHAL_HEVC_FEI_B_32x32_PU_OUTPUT = CODECHAL_HEVC_FEI_B_32x32_PU_BEGIN + 0,
    CODECHAL_HEVC_FEI_B_32x32_PU_SRC_Y = CODECHAL_HEVC_FEI_B_32x32_PU_BEGIN + 1,
    CODECHAL_HEVC_FEI_B_32x32_PU_SRC_UV = CODECHAL_HEVC_FEI_B_32x32_PU_BEGIN + 2,
    CODECHAL_HEVC_FEI_B_32x32_PU_SRC_Y2x = CODECHAL_HEVC_FEI_B_32x32_PU_BEGIN + 3,
    CODECHAL_HEVC_FEI_B_32x32_PU_SLICE_MAP = CODECHAL_HEVC_FEI_B_32x32_PU_BEGIN + 4,
    CODECHAL_HEVC_FEI_B_32x32_PU_SRC_Y2x_VME = CODECHAL_HEVC_FEI_B_32x32_PU_BEGIN + 5,
    CODECHAL_HEVC_FEI_B_32x32_PU_Simplest_Intra = CODECHAL_HEVC_FEI_B_32x32_PU_BEGIN + 6,
    CODECHAL_HEVC_FEI_B_32x32_PU_HME_MVP = CODECHAL_HEVC_FEI_B_32x32_PU_BEGIN + 7,
    CODECHAL_HEVC_FEI_B_32x32_PU_HME_DIST = CODECHAL_HEVC_FEI_B_32x32_PU_BEGIN + 8,
    CODECHAL_HEVC_FEI_B_32x32_PU_LCU_SKIP = CODECHAL_HEVC_FEI_B_32x32_PU_BEGIN + 9,
    CODECHAL_HEVC_FEI_B_32x32_PU_DEBUG = CODECHAL_HEVC_FEI_B_32x32_PU_BEGIN + 10,
    CODECHAL_HEVC_FEI_B_32x32_PU_END = CODECHAL_HEVC_FEI_B_32x32_PU_BEGIN + 11,

    // B MB ENC kernel
    CODECHAL_HEVC_FEI_B_MBENC_BEGIN = CODECHAL_HEVC_FEI_B_32x32_PU_END,
    CODECHAL_HEVC_FEI_B_MBENC_CU_RECORD = CODECHAL_HEVC_FEI_B_MBENC_BEGIN + 0,
    CODECHAL_HEVC_FEI_B_MBENC_PAK_CMD = CODECHAL_HEVC_FEI_B_MBENC_BEGIN + 1,
    CODECHAL_HEVC_FEI_B_MBENC_SRC_Y = CODECHAL_HEVC_FEI_B_MBENC_BEGIN + 2,
    CODECHAL_HEVC_FEI_B_MBENC_SRC_UV = CODECHAL_HEVC_FEI_B_MBENC_BEGIN + 3,
    CODECHAL_HEVC_FEI_B_MBENC_INTRA_DIST = CODECHAL_HEVC_FEI_B_MBENC_BEGIN + 4,
    CODECHAL_HEVC_FEI_B_MBENC_MIN_DIST = CODECHAL_HEVC_FEI_B_MBENC_BEGIN + 5,
    CODECHAL_HEVC_FEI_B_MBENC_HME_MVP = CODECHAL_HEVC_FEI_B_MBENC_BEGIN + 6,
    CODECHAL_HEVC_FEI_B_MBENC_HME_DIST = CODECHAL_HEVC_FEI_B_MBENC_BEGIN + 7,
    CODECHAL_HEVC_FEI_B_MBENC_SLICE_MAP = CODECHAL_HEVC_FEI_B_MBENC_BEGIN + 8,
    CODECHAL_HEVC_FEI_B_MBENC_VME_UNISIC_DATA = CODECHAL_HEVC_FEI_B_MBENC_BEGIN + 9,
    CODECHAL_HEVC_FEI_B_MBENC_Simplest_Intra = CODECHAL_HEVC_FEI_B_MBENC_BEGIN + 10,
    CODECHAL_HEVC_FEI_B_MBENC_REF_COLLOC = CODECHAL_HEVC_FEI_B_MBENC_BEGIN + 11,
    CODECHAL_HEVC_FEI_B_MBENC_Reserved = CODECHAL_HEVC_FEI_B_MBENC_BEGIN + 12,
    CODECHAL_HEVC_FEI_B_MBENC_BRC_Input = CODECHAL_HEVC_FEI_B_MBENC_BEGIN + 13,
    CODECHAL_HEVC_FEI_B_MBENC_LCU_QP = CODECHAL_HEVC_FEI_B_MBENC_BEGIN + 14,
    CODECHAL_HEVC_FEI_B_MBENC_BRC_DATA = CODECHAL_HEVC_FEI_B_MBENC_BEGIN + 15,
    //
    CODECHAL_HEVC_FEI_B_MBENC_VME_CURRENT = CODECHAL_HEVC_FEI_B_MBENC_BEGIN + 16,
    CODECHAL_HEVC_FEI_B_MBENC_VME_FORWARD_0 = CODECHAL_HEVC_FEI_B_MBENC_BEGIN + 17,
    CODECHAL_HEVC_FEI_B_MBENC_VME_BACKWARD_0 = CODECHAL_HEVC_FEI_B_MBENC_BEGIN + 18,
    CODECHAL_HEVC_FEI_B_MBENC_VME_FORWARD_1 = CODECHAL_HEVC_FEI_B_MBENC_BEGIN + 19,
    CODECHAL_HEVC_FEI_B_MBENC_VME_BACKWARD_1 = CODECHAL_HEVC_FEI_B_MBENC_BEGIN + 20,
    CODECHAL_HEVC_FEI_B_MBENC_VME_FORWARD_2 = CODECHAL_HEVC_FEI_B_MBENC_BEGIN + 21,
    CODECHAL_HEVC_FEI_B_MBENC_VME_BACKWARD_2 = CODECHAL_HEVC_FEI_B_MBENC_BEGIN + 22,
    CODECHAL_HEVC_FEI_B_MBENC_VME_FORWARD_3 = CODECHAL_HEVC_FEI_B_MBENC_BEGIN + 23,
    CODECHAL_HEVC_FEI_B_MBENC_VME_BACKWARD_3 = CODECHAL_HEVC_FEI_B_MBENC_BEGIN + 24,
    CODECHAL_HEVC_FEI_B_MBENC_VME_FORWARD_4 = CODECHAL_HEVC_FEI_B_MBENC_BEGIN + 25,
    CODECHAL_HEVC_FEI_B_MBENC_VME_BACKWARD_4 = CODECHAL_HEVC_FEI_B_MBENC_BEGIN + 26,
    CODECHAL_HEVC_FEI_B_MBENC_VME_FORWARD_5 = CODECHAL_HEVC_FEI_B_MBENC_BEGIN + 27,
    CODECHAL_HEVC_FEI_B_MBENC_VME_BACKWARD_5 = CODECHAL_HEVC_FEI_B_MBENC_BEGIN + 28,
    CODECHAL_HEVC_FEI_B_MBENC_VME_FORWARD_6 = CODECHAL_HEVC_FEI_B_MBENC_BEGIN + 29,
    CODECHAL_HEVC_FEI_B_MBENC_VME_BACKWARD_6 = CODECHAL_HEVC_FEI_B_MBENC_BEGIN + 30,
    CODECHAL_HEVC_FEI_B_MBENC_VME_FORWARD_7 = CODECHAL_HEVC_FEI_B_MBENC_BEGIN + 31,
    CODECHAL_HEVC_FEI_B_MBENC_VME_BACKWARD_7 = CODECHAL_HEVC_FEI_B_MBENC_BEGIN + 32,
    //
    CODECHAL_HEVC_FEI_B_MBENC_VME_MUL_CURRENT = CODECHAL_HEVC_FEI_B_MBENC_BEGIN + 33,
    CODECHAL_HEVC_FEI_B_MBENC_VME_MUL_BACKWARD_0 = CODECHAL_HEVC_FEI_B_MBENC_BEGIN + 34,
    CODECHAL_HEVC_FEI_B_MBENC_VME_MUL_NOUSE_0 = CODECHAL_HEVC_FEI_B_MBENC_BEGIN + 35,
    CODECHAL_HEVC_FEI_B_MBENC_VME_MUL_BACKWARD_1 = CODECHAL_HEVC_FEI_B_MBENC_BEGIN + 36,
    CODECHAL_HEVC_FEI_B_MBENC_VME_MUL_NOUSE_1 = CODECHAL_HEVC_FEI_B_MBENC_BEGIN + 37,
    CODECHAL_HEVC_FEI_B_MBENC_VME_MUL_BACKWARD_2 = CODECHAL_HEVC_FEI_B_MBENC_BEGIN + 38,
    CODECHAL_HEVC_FEI_B_MBENC_VME_MUL_NOUSE_2 = CODECHAL_HEVC_FEI_B_MBENC_BEGIN + 39,
    CODECHAL_HEVC_FEI_B_MBENC_VME_MUL_BACKWARD_3 = CODECHAL_HEVC_FEI_B_MBENC_BEGIN + 40,
    CODECHAL_HEVC_FEI_B_MBENC_VME_MUL_NOUSE_3 = CODECHAL_HEVC_FEI_B_MBENC_BEGIN + 41,
    //
    CODECHAL_HEVC_FEI_B_MBENC_CONCURRENT_THD_MAP = CODECHAL_HEVC_FEI_B_MBENC_BEGIN + 42,
    CODECHAL_HEVC_FEI_B_MBENC_MV_IDX = CODECHAL_HEVC_FEI_B_MBENC_BEGIN + 43,
    CODECHAL_HEVC_FEI_B_MBENC_MVP_IDX = CODECHAL_HEVC_FEI_B_MBENC_BEGIN + 44,
    CODECHAL_HEVC_FEI_B_MBENC_HAAR_DIST16x16 = CODECHAL_HEVC_FEI_B_MBENC_BEGIN + 45,
    CODECHAL_HEVC_FEI_B_MBENC_STATS_DATA = CODECHAL_HEVC_FEI_B_MBENC_BEGIN + 46,
    CODECHAL_HEVC_FEI_B_MBENC_FRAME_STATS_DATA = CODECHAL_HEVC_FEI_B_MBENC_BEGIN + 47,
    //
    CODECHAL_HEVC_FEI_B_MBENC_EXTERNAL_MVP = CODECHAL_HEVC_FEI_B_MBENC_BEGIN + 48,
    CODECHAL_HEVC_FEI_B_MBENC_PER_CTB_CTRL = CODECHAL_HEVC_FEI_B_MBENC_BEGIN + 49,
    CODECHAL_HEVC_FEI_B_MBENC_CTB_DISTORTION = CODECHAL_HEVC_FEI_B_MBENC_BEGIN + 50,
    CODECHAL_HEVC_FEI_B_MBENC_DEBUG = CODECHAL_HEVC_FEI_B_MBENC_BEGIN + 51,
    //
    CODECHAL_HEVC_FEI_B_MBENC_END = CODECHAL_HEVC_FEI_B_MBENC_BEGIN + 52,

    // HEVC B PAK kernel
    CODECHAL_HEVC_FEI_B_PAK_BEGIN = CODECHAL_HEVC_FEI_B_MBENC_END,
    CODECHAL_HEVC_FEI_B_PAK_CU_RECORD = CODECHAL_HEVC_FEI_B_PAK_BEGIN + 0,
    CODECHAL_HEVC_FEI_B_PAK_PAK_OBJ = CODECHAL_HEVC_FEI_B_PAK_BEGIN + 1,
    CODECHAL_HEVC_FEI_B_PAK_SLICE_MAP = CODECHAL_HEVC_FEI_B_PAK_BEGIN + 2,
    CODECHAL_HEVC_FEI_B_PAK_BRC_INPUT = CODECHAL_HEVC_FEI_B_PAK_BEGIN + 3,
    CODECHAL_HEVC_FEI_B_PAK_LCU_QP = CODECHAL_HEVC_FEI_B_PAK_BEGIN + 4,
    CODECHAL_HEVC_FEI_B_PAK_BRC_DATA = CODECHAL_HEVC_FEI_B_PAK_BEGIN + 5,
    CODECHAL_HEVC_FEI_B_PAK_MB_DATA = CODECHAL_HEVC_FEI_B_PAK_BEGIN + 6,
    CODECHAL_HEVC_FEI_B_PAK_MVP_DATA = CODECHAL_HEVC_FEI_B_PAK_BEGIN + 7,
    CODECHAL_HEVC_FEI_B_PAK_WA_PAK_DATA = CODECHAL_HEVC_FEI_B_PAK_BEGIN + 8,
    CODECHAL_HEVC_FEI_B_PAK_WA_PAK_OBJ = CODECHAL_HEVC_FEI_B_PAK_BEGIN + 9,
    CODECHAL_HEVC_FEI_B_PAK_DEBUG = CODECHAL_HEVC_FEI_B_PAK_BEGIN + 10,
    CODECHAL_HEVC_FEI_B_PAK_END = CODECHAL_HEVC_FEI_B_PAK_BEGIN + 11,

    //HEVC FORMAT CONVERSION AND DOWNSCALING KERNEL
    CODECHAL_HEVC_FEI_DS_COMBINED_BEGIN = CODECHAL_HEVC_FEI_B_PAK_END,
    CODECHAL_HEVC_FEI_DS_COMBINED_10BIT_Y = CODECHAL_HEVC_FEI_DS_COMBINED_BEGIN + 0,
    CODECHAL_HEVC_FEI_DS_COMBINED_10BIT_UV = CODECHAL_HEVC_FEI_DS_COMBINED_BEGIN + 1,
    CODECHAL_HEVC_FEI_DS_COMBINED_8BIT_Y = CODECHAL_HEVC_FEI_DS_COMBINED_BEGIN + 2,
    CODECHAL_HEVC_FEI_DS_COMBINED_8BIT_UV = CODECHAL_HEVC_FEI_DS_COMBINED_BEGIN + 3,
    CODECHAL_HEVC_FEI_DS_COMBINED_4xDOWNSCALE = CODECHAL_HEVC_FEI_DS_COMBINED_BEGIN + 4,
    CODECHAL_HEVC_FEI_DS_COMBINED_MB_STATS = CODECHAL_HEVC_FEI_DS_COMBINED_BEGIN + 5,
    CODECHAL_HEVC_FEI_DS_COMBINED_2xDOWNSCALE = CODECHAL_HEVC_FEI_DS_COMBINED_BEGIN + 6,
    CODECHAL_HEVC_FEI_DS_COMBINED_END = CODECHAL_HEVC_FEI_DS_COMBINED_BEGIN + 7,

    // P MB ENC kernel
    CODECHAL_HEVC_FEI_P_MBENC_BEGIN = CODECHAL_HEVC_FEI_DS_COMBINED_END,
    CODECHAL_HEVC_FEI_P_MBENC_CU_RECORD = CODECHAL_HEVC_FEI_P_MBENC_BEGIN + 0,
    CODECHAL_HEVC_FEI_P_MBENC_PAK_CMD = CODECHAL_HEVC_FEI_P_MBENC_BEGIN + 1,
    CODECHAL_HEVC_FEI_P_MBENC_SRC_Y = CODECHAL_HEVC_FEI_P_MBENC_BEGIN + 2,
    CODECHAL_HEVC_FEI_P_MBENC_SRC_UV = CODECHAL_HEVC_FEI_P_MBENC_BEGIN + 3,
    CODECHAL_HEVC_FEI_P_MBENC_INTRA_DIST = CODECHAL_HEVC_FEI_P_MBENC_BEGIN + 4,
    CODECHAL_HEVC_FEI_P_MBENC_MIN_DIST = CODECHAL_HEVC_FEI_P_MBENC_BEGIN + 5,
    CODECHAL_HEVC_FEI_P_MBENC_HME_MVP = CODECHAL_HEVC_FEI_P_MBENC_BEGIN + 6,
    CODECHAL_HEVC_FEI_P_MBENC_HME_DIST = CODECHAL_HEVC_FEI_P_MBENC_BEGIN + 7,
    CODECHAL_HEVC_FEI_P_MBENC_SLICE_MAP = CODECHAL_HEVC_FEI_P_MBENC_BEGIN + 8,
    CODECHAL_HEVC_FEI_P_MBENC_VME_UNISIC_DATA = CODECHAL_HEVC_FEI_P_MBENC_BEGIN + 9,
    CODECHAL_HEVC_FEI_P_MBENC_Simplest_Intra = CODECHAL_HEVC_FEI_P_MBENC_BEGIN + 10,
    CODECHAL_HEVC_FEI_P_MBENC_REF_COLLOC = CODECHAL_HEVC_FEI_P_MBENC_BEGIN + 11,
    CODECHAL_HEVC_FEI_P_MBENC_Reserved = CODECHAL_HEVC_FEI_P_MBENC_BEGIN + 12,
    CODECHAL_HEVC_FEI_P_MBENC_BRC_Input = CODECHAL_HEVC_FEI_P_MBENC_BEGIN + 13,
    CODECHAL_HEVC_FEI_P_MBENC_LCU_QP = CODECHAL_HEVC_FEI_P_MBENC_BEGIN + 14,
    CODECHAL_HEVC_FEI_P_MBENC_BRC_DATA = CODECHAL_HEVC_FEI_P_MBENC_BEGIN + 15,
    //
    CODECHAL_HEVC_FEI_P_MBENC_VME_CURRENT = CODECHAL_HEVC_FEI_P_MBENC_BEGIN + 16,
    CODECHAL_HEVC_FEI_P_MBENC_VME_FORWARD_0 = CODECHAL_HEVC_FEI_P_MBENC_BEGIN + 17,
    CODECHAL_HEVC_FEI_P_MBENC_VME_FORWARD_1 = CODECHAL_HEVC_FEI_P_MBENC_BEGIN + 19,
    CODECHAL_HEVC_FEI_P_MBENC_VME_FORWARD_2 = CODECHAL_HEVC_FEI_P_MBENC_BEGIN + 21,
    CODECHAL_HEVC_FEI_P_MBENC_VME_FORWARD_3 = CODECHAL_HEVC_FEI_P_MBENC_BEGIN + 23,
    CODECHAL_HEVC_FEI_P_MBENC_VME_FORWARD_4 = CODECHAL_HEVC_FEI_P_MBENC_BEGIN + 25,
    CODECHAL_HEVC_FEI_P_MBENC_VME_FORWARD_5 = CODECHAL_HEVC_FEI_P_MBENC_BEGIN + 27,
    CODECHAL_HEVC_FEI_P_MBENC_VME_FORWARD_6 = CODECHAL_HEVC_FEI_P_MBENC_BEGIN + 29,
    CODECHAL_HEVC_FEI_P_MBENC_VME_FORWARD_7 = CODECHAL_HEVC_FEI_P_MBENC_BEGIN + 31,
    //
    CODECHAL_HEVC_FEI_P_MBENC_CONCURRENT_THD_MAP = CODECHAL_HEVC_FEI_P_MBENC_BEGIN + 33,
    CODECHAL_HEVC_FEI_P_MBENC_MV_IDX = CODECHAL_HEVC_FEI_P_MBENC_BEGIN + 34,
    CODECHAL_HEVC_FEI_P_MBENC_MVP_IDX = CODECHAL_HEVC_FEI_P_MBENC_BEGIN + 35,
    CODECHAL_HEVC_FEI_P_MBENC_DEBUG = CODECHAL_HEVC_FEI_P_MBENC_BEGIN + 36,
    //
    CODECHAL_HEVC_FEI_P_MBENC_END = CODECHAL_HEVC_FEI_P_MBENC_BEGIN + 37,

    CODECHAL_HEVC_FEI_NUM_SURFACES = CODECHAL_HEVC_FEI_P_MBENC_END
};

//!
//! \enum     HEVC_BRC_FRAME_TYPE
//! \brief    HEVC BRC frame type
//!
enum HEVC_BRC_FRAME_TYPE
{
    HEVC_BRC_FRAME_TYPE_P_OR_LB = 0,
    HEVC_BRC_FRAME_TYPE_B = 1,
    HEVC_BRC_FRAME_TYPE_I = 2,
    HEVC_BRC_FRAME_TYPE_B1 = 3,
    HEVC_BRC_FRAME_TYPE_B2 = 4,
    HEVC_BRC_FRAME_TYPE_INVALID

};

//!
//! \enum     HEVC_ME_DIST_TYPE
//! \brief    HEVC me dist type
//!
enum HEVC_ME_DIST_TYPE
{
    HEVC_ME_DIST_TYPE_INTRA = 0,
    HEVC_ME_DIST_TYPE_INTRA_BRC_DIST,
    HEVC_ME_DIST_TYPE_INTER_BRC_DIST
};

//!
//! \enum     CODECHAL_ENCODE_HEVC_MULTIPRED
//! \brief    Codechal encode HEVC multipred
//!
enum CODECHAL_ENCODE_HEVC_MULTIPRED
{
    CODECHAL_ENCODE_HEVC_MULTIPRED_ENABLE = 0x01,
    CODECHAL_ENCODE_HEVC_MULTIPRED_DISABLE = 0x80
} ;

//!
//! \struct   CODECHAL_ENCODE_HEVC_SLICE_MAP
//! \brief    Codechal encode HEVC slice map
//!
struct CODECHAL_ENCODE_HEVC_SLICE_MAP
{
    uint8_t   ucSliceID;
    uint8_t   Reserved[3];
};
using PCODECHAL_ENCODE_HEVC_SLICE_MAP = CODECHAL_ENCODE_HEVC_SLICE_MAP*;

//!
//! \struct   CODECHAL_ENCODE_HEVC_WALKING_CONTROL_REGION
//! \brief    Codechal encode HEVC walking control region
//!
struct CODECHAL_ENCODE_HEVC_WALKING_CONTROL_REGION
{
    uint16_t  Reserve0[2];
    uint16_t  StartYCurrentSlice;
    uint16_t  StartYNextSlice;
    uint16_t  Xoffset;
    uint16_t  Reserve1[2];
    uint16_t  Yoffset;
    uint32_t  Reserve2[4];
    uint32_t  alignment[8];
};
using PCODECHAL_ENCODE_HEVC_WALKING_CONTROL_REGION = CODECHAL_ENCODE_HEVC_WALKING_CONTROL_REGION*;

//!
//! \struct   CODECHAL_ENCODE_HEVC_REFFRAME_SYNC_OBJ
//! \brief    Codechal encode HEVC reference frame sync object
//!
struct CODECHAL_ENCODE_HEVC_REFFRAME_SYNC_OBJ
{
    uint32_t                uiSemaphoreObjCount;
    MOS_RESOURCE            resSyncObject;
    bool                    bInUsed;
    CODECHAL_ENCODE_BUFFER  resSemaphoreMem;
};
using PCODECHAL_ENCODE_HEVC_REFFRAME_SYNC_OBJ = CODECHAL_ENCODE_HEVC_REFFRAME_SYNC_OBJ*;

C_ASSERT((sizeof(CODECHAL_ENCODE_HEVC_WALKING_CONTROL_REGION)) == 64);

//!
//! \struct   CODECHAL_ENCODE_HEVC_WALKINGPATTERN_PARAM
//! \brief    Codechal encode HEVC walking pattern parameter
//!
struct CODECHAL_ENCODE_HEVC_WALKINGPATTERN_PARAM
{
    // Media Walker setting
    MHW_WALKER_PARAMS           MediaWalker;
    MHW_VFE_SCOREBOARD          ScoreBoard;

    int                         Offset_Y;
    int                         Offset_Delta;
    uint32_t                    dwNumRegion;
    uint32_t                    dwMaxHeightInRegion;
    uint32_t                    dwNumUnitsInRegion;
};
using PCODECHAL_ENCODE_HEVC_WALKINGPATTERN_PARAM = CODECHAL_ENCODE_HEVC_WALKINGPATTERN_PARAM*;

//!
//! \struct   HEVC_TILE_STATS_INFO
//! \brief    HEVC tiles states info
//!
struct HEVC_TILE_STATS_INFO
{
    uint32_t uiTileSizeRecord;
    uint32_t uiHevcPakStatistics;
    uint32_t uiVdencStatistics;
    uint32_t uiHevcSliceStreamout;
};
using PHEVC_TILE_STATS_INFO = HEVC_TILE_STATS_INFO*;

//!
//! \struct   CODECHAL_ENCODE_HEVC_PAK_STATS_BUFFER
//! \brief    Codechal encode HEVC PAK States buffer
//!
struct CODECHAL_ENCODE_HEVC_PAK_STATS_BUFFER
{
    uint32_t HCP_BITSTREAM_BYTECOUNT_FRAME;
    uint32_t HCP_BITSTREAM_BYTECOUNT_FRAME_NOHEADER;
    uint32_t HCP_IMAGE_STATUS_CONTROL;
    uint32_t Reserved0;
    uint32_t HCP_IMAGE_STATUS_CONTROL_FOR_LAST_PASS;
    uint32_t Reserved1[3];
};

//!
//! \enum     CODECHAL_HEVC_MBENC_KRNIDX
//! \brief    Codechal HEVC MBENC KRNIDX
//!
enum CODECHAL_HEVC_MBENC_KRNIDX
{
    CODECHAL_HEVC_MBENC_2xSCALING = 0,
    CODECHAL_HEVC_MBENC_32x32MD,
    CODECHAL_HEVC_MBENC_16x16SAD,
    CODECHAL_HEVC_MBENC_16x16MD,
    CODECHAL_HEVC_MBENC_8x8PU,
    CODECHAL_HEVC_MBENC_8x8FMODE,
    CODECHAL_HEVC_MBENC_32x32INTRACHECK,
    CODECHAL_HEVC_MBENC_BENC,
    CODECHAL_HEVC_MBENC_BPAK,
    CODECHAL_HEVC_MBENC_ADV,
    CODECHAL_HEVC_MBENC_NUM,
    CODECHAL_HEVC_MBENC_DS_COMBINED = CODECHAL_HEVC_MBENC_NUM, //this will be added for 10 bit for KBL
    CODECHAL_HEVC_MBENC_NUM_KBL,
    CODECHAL_HEVC_MBENC_PENC = CODECHAL_HEVC_MBENC_NUM_KBL,
    CODECHAL_HEVC_MBENC_ADV_P,
    CODECHAL_HEVC_MBENC_NUM_BXT_SKL //Only BXT and SKL support HEVC P frame
};

//!
//! \enum     CODECHAL_HEVC_FEI_MBENC_KRNIDX
//! \brief    Codechal HEVC FEI MBENC KRNIDX
//!
enum CODECHAL_HEVC_FEI_MBENC_KRNIDX
{
    CODECHAL_HEVC_FEI_MBENC_BENC = CODECHAL_HEVC_MBENC_BENC,
    CODECHAL_HEVC_FEI_MBENC_BPAK,
    CODECHAL_HEVC_FEI_MBENC_NUM,
    CODECHAL_HEVC_FEI_MBENC_DS_COMBINED = CODECHAL_HEVC_FEI_MBENC_NUM, //this will be added for 10 bit for KBL
    CODECHAL_HEVC_FEI_MBENC_NUM_KBL,
    CODECHAL_HEVC_FEI_MBENC_PENC = CODECHAL_HEVC_FEI_MBENC_NUM_KBL,
    CODECHAL_HEVC_FEI_MBENC_NUM_BXT_SKL //Only BXT and SKL support HEVC P frame
} ;

//!
//! \enum     CODECHAL_HEVC_BRC_KRNIDX
//! \brief    Codechal HEVC BRC KRNIDX
//!
enum CODECHAL_HEVC_BRC_KRNIDX
{
    CODECHAL_HEVC_BRC_COARSE_INTRA = 0,
    CODECHAL_HEVC_BRC_INIT,
    CODECHAL_HEVC_BRC_RESET,
    CODECHAL_HEVC_BRC_FRAME_UPDATE,
    CODECHAL_HEVC_BRC_LCU_UPDATE,
    CODECHAL_HEVC_BRC_NUM
};

//! \class    CodechalEncodeHevcBase
//! \brief    HEVC encoder base class
//! \details  This class defines the base class for HEVC encoder, it includes
//!           common member fields, functions, interfaces etc shared by both dual-pipe and VDEnc for all Gens.
//!
//!           To create a HEVC encoder instance, client needs to call CodechalEncodeHevcBase::CreateHevcState()
//!
class CodechalEncodeHevcBase : public CodechalEncoderState
{
public:
    static constexpr uint32_t                   MAX_LCU_SIZE = 64;                              //!< Max LCU size 64
    static constexpr uint32_t                   QP_NUM = 52;                                    //!< Number of QP values

    static const uint8_t                        TransformSkipCoeffsTable[4][2][2][2][2];        //!< Transform skip related coeff table
    static const uint16_t                       TransformSkipLambdaTable[QP_NUM];               //!< Transform skip related lambda table

    static const uint8_t CC_BYPASS = 0x0;                                                       //!< MFX Video Copy Mode
    static const uint8_t CC_LIST_MODE = 0x1;                                                    //!< MFX Video CP Copy Mode

    //! HEVC encoder slice type enum
    enum
    {
        CODECHAL_ENCODE_HEVC_B_SLICE = 0,
        CODECHAL_ENCODE_HEVC_P_SLICE = 1,
        CODECHAL_ENCODE_HEVC_I_SLICE = 2,
        CODECHAL_ENCODE_HEVC_NUM_SLICE_TYPES =  3
    };

    // Parameters passed from application
    PCODEC_HEVC_ENCODE_PICTURE_PARAMS  m_hevcPicParams      = nullptr;  //!< Pointer to picture parameter
    PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS m_hevcSeqParams      = nullptr;  //!< Pointer to sequence parameter
    PCODEC_HEVC_ENCODE_SLICE_PARAMS    m_hevcSliceParams    = nullptr;  //!< Pointer to slice parameter
    CodecEncodeHevcFeiPicParams *      m_hevcFeiPicParams   = nullptr;  //!< Pointer to FEI picture parameter
    PCODECHAL_HEVC_IQ_MATRIX_PARAMS    m_hevcIqMatrixParams = nullptr;  //!< Pointer to IQ matrix parameter

    uint32_t m_widthAlignedMaxLcu  = 0;  //!< Picture width aligned in max LCU size
    uint32_t m_heightAlignedMaxLcu = 0;  //!< Picture height aligned in max LCU size

    // PAK resources
    MOS_RESOURCE     m_resDeblockingFilterRowStoreScratchBuffer;                            //!< Deblocking filter row store scratch data buffer
    MOS_RESOURCE     m_resDeblockingFilterTileRowStoreScratchBuffer;                        //!< Deblocking filter tile row store Scratch data buffer
    MOS_RESOURCE     m_resDeblockingFilterColumnRowStoreScratchBuffer;                      //!< Deblocking filter column row Store scratch data buffer
    MOS_RESOURCE     m_resMetadataLineBuffer;                                               //!< Metadata line data buffer
    MOS_RESOURCE     m_resMetadataTileLineBuffer;                                           //!< Metadata tile line data buffer
    MOS_RESOURCE     m_resMetadataTileColumnBuffer;                                         //!< Metadata tile column data buffer
    MOS_RESOURCE     m_resSaoLineBuffer;                                                    //!< SAO line data buffer
    MOS_RESOURCE     m_resSaoTileLineBuffer;                                                //!< SAO tile line data buffer
    MOS_RESOURCE     m_resSaoTileColumnBuffer;                                              //!< SAO tile column data buffer
    MOS_RESOURCE     m_resLcuBaseAddressBuffer;                                             //!< LCU base address buffer
    MOS_RESOURCE     m_resLcuIldbStreamOutBuffer;                                           //!< LCU ILDB streamout buffer
    MOS_RESOURCE     m_resSaoStreamOutBuffer;                                               //!< SAO streamout buffer
    MOS_RESOURCE     m_resFrameStatStreamOutBuffer;                                         //!< Frame statistics streamout buffer
    MOS_RESOURCE     m_resSseSrcPixelRowStoreBuffer;                                        //!< SSE src pixel row store buffer
    MHW_BATCH_BUFFER m_batchBufferForPakSlices[CODECHAL_HEVC_NUM_PAK_SLICE_BATCH_BUFFERS];  //!< Batch buffer for pak slice commands
    uint32_t         m_currPakSliceIdx                    = 0;                              //!< Current pak slice index
    bool             m_useBatchBufferForPakSlices         = false;                          //!< Flag to indicate if batch buffer is used for PAK slice level commands
    uint32_t         m_batchBufferForPakSlicesStartOffset = 0;                              //!< Pak slice command start offset within batch buffer

    uint32_t m_defaultPictureStatesSize    = 0;  //!< Picture state command size
    uint32_t m_defaultPicturePatchListSize = 0;  //!< Picture state patch list size
    uint32_t m_defaultSliceStatesSize      = 0;  //!< Slice state command size
    uint32_t m_defaultSlicePatchListSize   = 0;  //!< Slice state patch list size

    unsigned char m_uc2NdSaoPass      = 0;      //!< Second SAO pass number
    bool          m_b2NdSaoPassNeeded = false;  //!< Second SAO pass enable flag

    CODECHAL_ENCODE_HEVC_REFFRAME_SYNC_OBJ  m_refSync[CODEC_NUM_TRACKED_BUFFERS];  //!< Reference frame sync object
    PCODECHAL_ENCODE_HEVC_REFFRAME_SYNC_OBJ m_currRefSync           = nullptr;     //!< Pointer to current frame sync object
    unsigned char                           m_lastMbCodeIndex       = 0;           //!< Used in the non-parallel BRC to check if the previous PAK is ready
    unsigned char                           m_currMinus2MbCodeIndex = 0;           //!< Used in the parallel BRC to check if the (N-2)th PAK is ready, where N is the frame number

    int8_t        m_refIdxMapping[CODEC_MAX_NUM_REF_FRAME_HEVC];                          //!< Reference frame index mapping
    bool          m_lowDelay                              = false;                        //!< Low delay flag
    bool          m_sameRefList                           = false;                        //!< Flag to specify if ref list L0 and L1 are same
    bool          m_isMaxLcu64                            = false;                        //!< Flag to specify if max LCU size is 64
    uint32_t      m_sizeOfMvTemporalBuffer                = 0;                            //!< MV temporal buffer size
    bool          m_encode4KSequence                      = false;                        //!< Flag to specify if input sequence is 4k size
    bool          m_encode16KSequence                     = false;                        //!< Flag to specify if input sequence is 16k size
    bool          m_hevcRdoqEnabled                       = false;                        //!< RDOQ enable flag
    uint32_t      m_rdoqIntraTuThreshold                  = 0;                            //!< RDOQ intra threshold
    bool          m_hevcIFrameRdoqEnabled                 = true;                        //!< Control intra frame RDOQ enable/disable
#if (_DEBUG || _RELEASE_INTERNAL)
    bool          m_rdoqIntraTuOverride                   = false;                        //!< Override RDOQ intra TU or not
    bool          m_rdoqIntraTuDisableOverride            = false;                        //!< Override RDOQ intra TU disable 
    uint16_t      m_rdoqIntraTuThresholdOverride          = 0;                            //!< Override RDOQ intra TU threshold
#endif
    bool          m_is10BitHevc                           = false;                        //!< 10bit encoding flag
    unsigned char m_chromaFormat                          = HCP_CHROMA_FORMAT_YUV420;     //!< Chroma format(420, 422 etc)
    unsigned char m_bitDepth                              = 8;                            //!< Bit depth
    uint32_t      m_maxNumSlicesSupported                 = CODECHAL_HEVC_MAX_NUM_SLICES_LVL_5;  //!< Maximal number of slices supported
    uint32_t      m_sizeOfSseSrcPixelRowStoreBufferPerLcu = 0;                            //!< Size of SSE row store buffer per LCU
    uint32_t      m_sizeOfHcpPakFrameStats                = 0;                            //!> Size of HEVC PAK frame statistics
    uint8_t       m_roundingInter                         = 4;                            // the value is from prototype
    uint8_t       m_roundingIntra                         = 10;                           // the value is from prototype

    bool                       m_currUsedRefPic[CODEC_MAX_NUM_REF_FRAME_HEVC];     //!< Reference picture usage array
    CODEC_PIC_ID               m_picIdx[CODEC_MAX_NUM_REF_FRAME_HEVC];             //!< Reference picture index array
    PCODEC_REF_LIST            m_refList[CODECHAL_NUM_UNCOMPRESSED_SURFACE_HEVC];  //!< Pointer to reference pictures
    PCODECHAL_NAL_UNIT_PARAMS *m_nalUnitParams          = nullptr;                 //!< Pointer to NAL unit parameters
    bool                       m_enable26WalkingPattern = false;                   //!< 26 walking pattern enable flag

    // ME
    bool        m_hmeEnabled    = false;                     //!< HME enable flag
    bool        m_b16XMeEnabled = false;                     //!< 16xME enable flag
    bool        m_b32XMeEnabled = false;                     //!< 32xME enable flag
    MOS_SURFACE m_s4XMeMvDataBuffer;                         //!< 4xME mv data buffer
    MOS_SURFACE m_s16XMeMvDataBuffer;                        //!< 16xME mv data buffer
    MOS_SURFACE m_s32XMeMvDataBuffer;                        //!< 32xME mv data buffer
    MOS_SURFACE m_s4XMeDistortionBuffer;                     //!< 4xME distortion buffer
    bool        m_brcDistortionBufferSupported   = false;    //!< Brc distorion supported flag
    bool        m_b4XMeDistortionBufferSupported = false;    //!< 4xME distorion supported flag
    uint8_t *   m_bmeMethodTable                 = nullptr;  //!< Pointer for ME method table based on TargetUsage
    uint8_t *   m_meMethodTable                  = nullptr;  //!< Pointer for ME method table based on TargetUsage

    // BRC
    uint16_t m_usAvbrAccuracy                     = 0;      //!< AVBR accuracy
    uint16_t m_usAvbrConvergence                  = 0;      //!< AVBR convergence
    double   m_dBrcInitCurrentTargetBufFullInBits = 0.0;    //!< Initial value of target buffer fullness in bits
    double   m_dBrcInitResetInputBitsPerFrame     = 0.0;    //!< Input bits per frame
    uint32_t m_brcInitResetBufSizeInBits          = 0;      //!< Target buffer size in bits
    uint32_t m_hevcBrcPakStatisticsSize           = 0;      //!< BRC PAK statistics size
    bool     m_brcEnabled                         = false;  //!< BRC enable flag
    bool     m_lcuBrcEnabled                      = false;  //!< LCU BRC enable flag
    bool     m_brcInit                            = true;   //!< BRC init flag
    bool     m_brcReset                           = false;  //!< BRC reset flag
    bool     m_brcRoiEnabled                      = false;  //!< BRC Roi flag
    bool     m_roiValueInDeltaQp                  = true;   //!< ROI Value in deltaQP or priority flag

    CODECHAL_ENCODE_BUFFER m_resPakcuLevelStreamoutData;  //!< PAK LCU level stream out data buffer

    // Mb Qp Data
    bool            m_mbQpDataEnabled = false;      //!< Mb Qp Data Enable Flag.
    MOS_SURFACE     m_mbQpDataSurface;              //!< Pointer to MOS_SURFACE of Mb Qp data surface, provided by DDI.

protected:
    //!
    //! \brief    Constructor
    //!
    CodechalEncodeHevcBase(CodechalHwInterface* hwInterface,
        CodechalDebugInterface* debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo);

    uint8_t*                                    m_kernelBinary = nullptr;               //!< Pointer to the kernel binary
    uint32_t                                    m_combinedKernelSize = 0;               //!< Combined kernel binary size
    PMHW_VDBOX_HEVC_SLICE_STATE                 m_sliceStateParams = nullptr;           //!< Slice state parameters
    PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS          m_pipeModeSelectParams = nullptr;       //!< Pipe mode select parameters
    PMHW_VDBOX_PIPE_BUF_ADDR_PARAMS             m_pipeBufAddrParams = nullptr;          //!< Pipe buffer addr parameters

public:
    //!
    //! \brief    Destructor
    //!
    virtual ~CodechalEncodeHevcBase() {};

    //!
    //! \brief    Help function to check if the rate control method is BRC
    //!
    //! \param    [in] rc
    //!           Rate control method
    //!
    //! \return   True if using BRC , else return false 
    //!
    bool IsRateControlBrc(uint8_t rc)
    {
        return  (rc == RATECONTROL_CBR) ||
                (rc == RATECONTROL_VBR) ||
                (rc == RATECONTROL_AVBR) ||
                (rc == RATECONTROL_VCM) ||
                (rc == RATECONTROL_ICQ) ||
                (rc == RATECONTROL_QVBR);
    }

    //!
    //! \brief    Help function to calculate the slice QP
    //!
    //! \return   Slice QP value
    //!
    int CalSliceQp()
    {
        CODECHAL_ENCODE_ASSERT(m_hevcSliceParams);
        CODECHAL_ENCODE_ASSERT(m_hevcPicParams);

        int qp = m_hevcSliceParams->slice_qp_delta + m_hevcPicParams->QpY;
        CODECHAL_ENCODE_ASSERT(qp >= 0 && qp < QP_NUM);
        return qp;
    }

    //!
    //! \brief    Help function to get current PAK pass
    //!
    //! \return   Current PAK pass
    //!
    virtual int GetCurrentPass()
    {
        return m_currPass;
    }

    //!
    //! \brief    Help function to check if current PAK pass is first pass
    //!
    //! \return   True if current PAK pass is first pass, otherwise return false
    //!
    virtual bool IsFirstPass()
    {
        return GetCurrentPass() == 0 ? true : false;
    }

    //!
    //! \brief    Help function to check if current PAK pass is last pass
    //!
    //! \return   True if current PAK pass is last pass, otherwise return false
    //!
    virtual bool IsLastPass()
    {
        return GetCurrentPass() == m_numPasses ? true : false;
    }

    //!
    //! \brief    Help function to check if legacy command buffer is used
    //!
    //! \return   True if using legacy command buffer, otherwise return false
    //!
    bool UseRenderCommandBuffer()
    {
        return (m_osInterface->pfnGetGpuContext(m_osInterface) == m_renderContext);
    }

    //!
    //! \brief    Help function to retrieve a command buffer
    //!
    //! \param    [in, out] cmdBuffer
    //!           Pointer to command buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetCommandBuffer(PMOS_COMMAND_BUFFER cmdBuffer);

    //!
    //! \brief    Help function to return a command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Pointer to command buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS ReturnCommandBuffer(PMOS_COMMAND_BUFFER cmdBuffer);

    //!
    //! \brief    Help function to submit a command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Pointer to command buffer
    //! \param    [in] nullRendering
    //!           Null rendering flag
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SubmitCommandBuffer(
        PMOS_COMMAND_BUFFER cmdBuffer,
        bool                nullRendering);

    //!
    //! \brief    Help function to verify command buffer size
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS VerifyCommandBufferSize();

    //!
    //! \brief    Help function to send prolog with frame tracking information
    //!
    //! \param    [in] cmdBuffer
    //!           Pointer to command buffer
    //! \param    [in] frameTrackingRequested
    //!           True if frame tracking info is needed, false otherwise
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SendPrologWithFrameTracking(
        PMOS_COMMAND_BUFFER cmdBuffer,
        bool frameTrackingRequested,
        MHW_MI_MMIOREGISTERS *mmioRegister = nullptr);

    //!
    //! \brief    Wait for dependent VDBOX to get ready
    //!
    //! \param    [in] cmdBuffer
    //!           Pointer to command buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS WaitForVDBOX(PMOS_COMMAND_BUFFER cmdBuffer);

    //!
    //! \brief    Add MI_SEMAPHORE_WAIT command in command buffer
    //!
    //! \param    [in] semaphoreMem
    //!           Pointer to semaphore resource
    //! \param    [in] cmdBuffer
    //!           Pointer to command buffer
    //! \param    [in] semValue
    //!           Value to wait on semaphore memory
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SendHWWaitCommand(
        PMOS_RESOURCE semaphoreMem,
        PMOS_COMMAND_BUFFER cmdBuffer,
        uint32_t semValue);

    //!
    //! \brief      Send MI atomic command
    //!
    //! \param    [in] semaMem
    //!           Pointer to semaphore resource
    //! \param    [in] ImmData
    //!           immediate data for atomic operation
    //! \param    [in] opCode
    //!           enum value of MHW_COMMON_MI_ATOMIC_OPCODE
    //! \param    [in] cmdBuffer
    //!           Pointer to command buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SendMIAtomicCmd(
        PMOS_RESOURCE               semaMem,
        uint32_t                    ImmData,
        MHW_COMMON_MI_ATOMIC_OPCODE opCode,
        PMOS_COMMAND_BUFFER         cmdBuffer);

    //!
    //! \brief    Store data to semaphore memory
    //!
    //! \param    [in] semaphoreMem
    //!           Pointer to semaphore resource
    //! \param    [in] cmdBuffer
    //!           Pointer to command buffer
    //! \param    [in] value
    //!           Value to store in semaphore memory
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetSemaphoreMem(
        PMOS_RESOURCE semaphoreMem,
        PMOS_COMMAND_BUFFER cmdBuffer,
        uint32_t value);

    //!
    //! \brief    Allocate resources for encoder instance
    //! \details  It is invoked when initializing encoder instance and it would call #AllocateEncResources(), #AllocateBrcResources(), 
    //!           #AllocatePakResources() and #InitSurfaceInfoTable()
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateResources();

    //!
    //! \brief    Free encoder resources
    //! \details  It is invoked when destorying encoder instance and it would call #FreeEncResources(), #FreeBrcResources()
    //!           and #FreePakResources()
    //!
    //! \return   void
    //!
    void FreeResources();

    //!
    //! \brief    Help function to allocate a 1D buffer
    //!
    //! \param    [in,out] buffer
    //!           Pointer to allocated buffer
    //! \param    [in] size
    //!           Buffer size
    //! \param    [in] name
    //!           Buffer name
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateBuffer(
        PCODECHAL_ENCODE_BUFFER buffer,
        uint32_t size,
        const char* name);

    //!
    //! \brief    Help function to allocate a generic 2D surface
    //!
    //! \param    [in,out] surface
    //!           Pointer to allocated surface
    //! \param    [in] width
    //!           Surface width
    //! \param    [in] height
    //!           Surface height
    //! \param    [in] name
    //!           Surface name
    //! \param    [in] tileType
    //!           Tile type
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateBuffer2D(
        PMOS_SURFACE surface,
        uint32_t width,
        uint32_t height,
        const char* name,
        MOS_TILE_TYPE tileType = MOS_TILE_LINEAR);

    //!
    //! \brief    Help function to allocate a NV12 TILE_Y surface
    //!
    //! \param    [in,out] surface
    //!           Pointer to allocated surface
    //! \param    [in] width
    //!           Surface width
    //! \param    [in] height
    //!           Surface height
    //! \param    [in] name
    //!           Surface name
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateSurface(
        PMOS_SURFACE surface,
        uint32_t width,
        uint32_t height,
        const char* name);

    //!
    //! \brief    Help function to allocate PAK slice level batch buffers 
    //!
    //! \param    [in] numSlices
    //!           Number of slices
    //! \param    [in] numPakPasses
    //!           Number of PAK passes
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateBatchBufferForPakSlices(
        uint32_t numSlices,
        unsigned char numPakPasses);

    //! \brief    Calculates the PSNR values for luma/ chroma 
    //!
    //! \param    [in, out] encodeStatus
    //!           Pointer to encoder status
    //! \param    [in, out] encodeStatusReport
    //!           Pointer to encoder status report
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS CalculatePSNR(
        EncodeStatus        *encodeStatus,
        EncodeStatusReport  *encodeStatusReport);

    //! \brief    Copy sum square error for luma/ chroma channel from 
    //!           frame statistics report into encodeStatus buffer
    //!
    //! \param    [in, out] cmdBuffer
    //!           Pointer to command buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS ReadSseStatistics(PMOS_COMMAND_BUFFER cmdBuffer);

    //!
    //! \brief    Help function to release PAK slice level batch buffers 
    //!
    //! \param    [in] index
    //!           Index of batch buffer to be released
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ReleaseBatchBufferForPakSlices(uint32_t index);

    //!
    //! \brief    Help function to get the MaxMBPS for speicifc level Id
    //!
    //! \param    [in] levelIdc
    //!           HEVC level Id
    //! \param    [in] maxMBPS
    //!           Max MBPS
    //! \param    [in] maxBytePerPic
    //!           Max byte per picture
    //!
    //! \return   MaxMBPS(max number of MB per second) for input level Id
    //!
    MOS_STATUS GetMaxMBPS(uint32_t levelIdc, uint32_t* maxMBPS, uint64_t* maxBytePerPic);

    //!
    //! \brief    Help function to calcuate max frame size corresponding to the input profile/level
    //!
    //! \return   Max frame size in bytes 
    //!
    uint32_t GetProfileLevelMaxFrameSize();

    //!
    //! \brief    Help function to create a flat scaling list (when scaling list is not passed in sequence parameter)
    //!
    //! \return   void
    //!
    void CreateFlatScalingList();

    //!
    //! \brief    Help function to create a default scaling list (when scaling list data is not presented in picture parameter)
    //!
    //! \return   void
    //!
    void CreateDefaultScalingList();

    //!
    //! \brief    Validate low delay mode for B frame
    //!
    //! \param    [in] slcParams
    //!           Pointer to slice parameter
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ValidateLowDelayBFrame(PCODEC_HEVC_ENCODE_SLICE_PARAMS slcParams);

    //!
    //! \brief    Validate if reference list L0 and L1 are same 
    //!
    //! \param    [in] slcParams
    //!           Pointer to slice parameter
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ValidateSameRefInL0L1(PCODEC_HEVC_ENCODE_SLICE_PARAMS slcParams);

    //!
    //! \brief    Verify slice SAO state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS VerifySliceSAOState();

    //!
    //! \brief    Update recon surface to Variant format for 422 10-bit
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS UpdateYUY2SurfaceInfo(
        PMOS_SURFACE        surface,
        bool                is10Bit);

    //!
    //! \brief    Check if the color format of the surface is supported
    //!
    //! \details  If the color format is not supported, and the Codec/Gen supports color space conversion feature
    //            then Csc+Ds+Conversion kernel will be called to convert raw surface to supported color format
    //!
    //! \param    [in] surface
    //!           pointer to surface
    //!
    //! \return   bool
    //!           true if the color format is supported, false otherwise
    //!
    virtual bool CheckSupportedFormat(PMOS_SURFACE surface) { return true; }

    //!
    //! \brief    Initialize encoder instance with the provided settings
    //! \details  When derived class overwrite this function to do its own initialization,
    //            it should call #CodechalEncodeHevcBase::Initialize() first
    //            to do common initializations
    //!
    //! \param    [in] settings
    //!           Encoder settings
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Initialize(CodechalSetting * settings);

    //!
    //! \brief    Initialize surface info
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitSurfaceInfoTable()
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Setup/configure encoder based on sequence parameter set
    //! \details  It is invoked when the encoder receives a new sequence parameter set and it would
    //!           set up and configure the encoder state that used for the sequence
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetSequenceStructs();

    //!
    //! \brief    Setup/configure encoder based on picture parameter set
    //! \details  It is invoked for every picture and it would set up and configure the
    //            encoder state that used for current picture
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetPictureStructs();

    //!
    //! \brief    Calculate maximum bitsize allowed for LCU
    //! \details  Calculate LCU max coding size according to log2_max_coding_block_size_minus3
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS CalcLCUMaxCodingSize();

    //!
    //! \brief    Setup/configure encoder based on slice parameter set
    //! \details  It is invoked for every picture and and it would set up and configure the
    //            encoder state that used for current picture and slices
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetSliceStructs();

    //!
    //! \brief    Calculate transform skip parameters
    //!
    //! \param    [in, out] params
    //!           Transform skip parameters
    //!
    //! \return   void
    //!
    virtual void CalcTransformSkipParameters(MHW_VDBOX_ENCODE_HEVC_TRANSFORM_SKIP_PARAMS& params);

    //!
    //! \brief    Retreive image status information
    //!
    //! \param    [in] cmdBuffer
    //!           Pointer to command buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS ReadImageStatus(PMOS_COMMAND_BUFFER cmdBuffer);

    //!
    //! \brief    Retreive HCP status
    //!
    //! \param    [in] cmdBuffer
    //!           Pointer to command buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS ReadHcpStatus(PMOS_COMMAND_BUFFER cmdBuffer);

    //!
    //! \brief    Retreive BRC Pak statistics
    //!
    //! \param    [in] cmdBuffer
    //!           Pointer to command buffer
    //! \param    [in] params
    //!           BRC pak statistics parameters
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS ReadBrcPakStatistics(
        PMOS_COMMAND_BUFFER cmdBuffer,
        EncodeReadBrcPakStatsParams* params);

    //!
    //! \brief    Get encoder status report
    //!
    //! \param    [in, out] encodeStatus
    //!           Pointer to encoder status
    //! \param    [in, out] encodeStatusReport
    //!           Pointer to encoder status report
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetStatusReport(
        EncodeStatus       *encodeStatus,
        EncodeStatusReport *encodeStatusReport);

    //!
    //! \brief    User Feature key report
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS UserFeatureKeyReport();

    //!
    //! \brief    Initialize encoder at picture level
    //!
    //! \param    [in] params
    //!           Picture encoding parameters
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitializePicture(const EncoderParams& params);

    //!
    //! \brief    Perform platform capability check
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS PlatformCapabilityCheck()
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Set batch buffer for PAK slices
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetBatchBufferForPakSlices();

    // HCP/PAK functions

    //!
    //! \brief    Set HCP_PIPE_MODE_SELECT parameters
    //!
    //! \param    [in, out] pipeModeSelectParams
    //!           HCP_PIPE_MODE_SELECT parameters
    //!
    //! \return   void
    //!
    virtual void SetHcpPipeModeSelectParams(MHW_VDBOX_PIPE_MODE_SELECT_PARAMS& pipeModeSelectParams);

    //!
    //! \brief    Set HCP_SURFACE_PARAMS for source picture
    //!
    //! \param    [in, out] srcSurfaceParams
    //!           HCP_SURFACE_PARAMS for source picture
    //!
    //! \return   void
    //!
    virtual void SetHcpSrcSurfaceParams(MHW_VDBOX_SURFACE_PARAMS& srcSurfaceParams);

    //!
    //! \brief    Set HCP_SURFACE_PARAMS for recon picture
    //!
    //! \param    [in, out] reconSurfaceParams
    //!           HCP_SURFACE_PARAMS for recon picture
    //!
    //! \return   void
    //!
    virtual void SetHcpReconSurfaceParams(MHW_VDBOX_SURFACE_PARAMS& reconSurfaceParams);

    //!
    //! \brief    Set HCP_PIPE_BUF_ADDR parameters
    //!
    //! \param    [in, out] pipeBufAddrParams
    //!           HCP_PIPE_BUF_ADDR parameters
    //!
    //! \return   void
    //!
    virtual void SetHcpPipeBufAddrParams(MHW_VDBOX_PIPE_BUF_ADDR_PARAMS& pipeBufAddrParams);

    //!
    //! \brief    Set HCP_IND_OBJ_BASE_ADDR parameters
    //!
    //! \param    [in, out] indObjBaseAddrParams
    //!           HCP_IND_OBJ_BASE_ADDR parameters
    //!
    //! \return   void
    //!
    virtual void SetHcpIndObjBaseAddrParams(MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS& indObjBaseAddrParams);

    //!
    //! \brief    Set HCP_QM_STATE and HCP_FQM_STATE parameters
    //!
    //! \param    [in, out] fqmParams
    //!           HCP_FQM_STATE parameters
    //! \param    [in, out] qmParams
    //!           HCP_QM_STATE parameters
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual void SetHcpQmStateParams(MHW_VDBOX_QM_PARAMS& fqmParams, MHW_VDBOX_QM_PARAMS& qmParams);

    //!
    //! \brief    Set HCP_PIC_STATE parameters
    //!
    //! \param    [in, out] picStateParams
    //!           HCP_PIC_STATE parameters
    //!
    //! \return   void
    //!
    virtual void SetHcpPicStateParams(MHW_VDBOX_HEVC_PIC_STATE& picStateParams);

    //!
    //! \brief    Set HCP_SLICE_STATE parameters shared by all slices
    //!
    //! \param    [in, out] sliceStateParams
    //!           HCP_SLICE_STATE parameters
    //!
    //! \return   void
    //!
    virtual void SetHcpSliceStateCommonParams(MHW_VDBOX_HEVC_SLICE_STATE& sliceStateParams);

    //!
    //! \brief    Create HCP_SLICE_STATE parameters shared by all slices
    //!
    //! \return   void
    //!
    virtual void CreateMhwParams();

    //!
    //! \brief    Set HCP_SLICE_STATE parameters that are different at slice level
    //!
    //! \param    [in, out] sliceStateParams
    //!           HCP_SLICE_STATE parameters
    //! \param    [in] slcData
    //!           Pointer to CODEC_ENCODE_SLCDATA
    //! \param    [in] currSlcIdx
    //!           Current slice index
    //!
    //! \return   void
    //!
    virtual void SetHcpSliceStateParams(
        MHW_VDBOX_HEVC_SLICE_STATE& sliceStateParams,
        PCODEC_ENCODER_SLCDATA slcData,
        uint32_t currSlcIdx);

    //!
    //! \brief    Add HCP_REF_IDX command to command buffer or batch buffer
    //!
    //! \param    [in, out] cmdBuffer
    //!           Pointer to the command buffer
    //! \param    [in, out] batchBuffer
    //!           Pointer to the batch buffer
    //! \param    [in] params
    //!           Pointer to MHW_VDBOX_HEVC_SLICE_STATE parameters
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddHcpRefIdxCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_BATCH_BUFFER batchBuffer,
        PMHW_VDBOX_HEVC_SLICE_STATE params);

    //!
    //! \brief    Add HCP_PAK_INSERT_OBJECT commands for NALUs to command buffer or batch buffer
    //! \details  This function would add multiple HCP_PAK_INSERT_OBJECT, one of each NALU 
    //!
    //! \param    [in, out] cmdBuffer
    //!           Pointer to the command buffer
    //! \param    [in, out] batchBuffer
    //!           Pointer to the batch buffer
    //! \param    [in] params
    //!           Pointer to MHW_VDBOX_HEVC_SLICE_STATE parameters
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddHcpPakInsertNALUs(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_BATCH_BUFFER batchBuffer,
        PMHW_VDBOX_HEVC_SLICE_STATE params);

    //!
    //! \brief    Add HCP_PAK_INSERT_OBJECT commands for slice header to command buffer or batch buffer
    //!
    //! \param    [in, out] cmdBuffer
    //!           Pointer to the command buffer
    //! \param    [in, out] batchBuffer
    //!           Pointer to the batch buffer
    //! \param    [in] params
    //!           Pointer to MHW_VDBOX_HEVC_SLICE_STATE parameters
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddHcpPakInsertSliceHeader(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_BATCH_BUFFER batchBuffer,
        PMHW_VDBOX_HEVC_SLICE_STATE params);

    //! \brief    Initialize kernel state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitKernelState() = 0;

    //!
    //! \brief    Get max binding table count required
    //!
    //! \return   Value of max binding table count
    //!
    virtual uint32_t GetMaxBtCount() = 0;

    //!
    //! \brief    Allocate resources for PAK
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AllocatePakResources();

    //!
    //! \brief    Free PAK resources
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS FreePakResources();

    //!
    //! \brief    Allocate resources for ENC 
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AllocateEncResources() = 0;

    //!
    //! \brief    Free ENC resources
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS FreeEncResources() = 0;

    //!
    //! \brief    Allocate BRC resources
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AllocateBrcResources() = 0;

    //!
    //! \brief    Free BRC resources
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS FreeBrcResources() = 0;

    //!
    //! \brief    Calculate down scaled dimensions
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS CalcScaledDimensions() = 0;

    //!
    //! \brief    Validate ref frame data
    //!
    //! \param    [in] slcParams
    //!           Slice parameters
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS ValidateRefFrameData(PCODEC_HEVC_ENCODE_SLICE_PARAMS slcParams) = 0;

    //!
    //! \brief    Encode kernel functions
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS EncodeKernelFunctions() = 0;

    //!
    //! \brief    Execute kernel functions
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ExecuteKernelFunctions();

    //!
    //! \brief    Encode command at picture level
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS ExecutePictureLevel() = 0;

    //!
    //! \brief    Encode command at slice level
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS ExecuteSliceLevel() = 0;

    //!
    //! \brief    Calculate picture state command size
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS CalculatePictureStateCommandSize();

    //!
    //! \brief    Calculate picture state command size
    //!
    //! \param    [in, out] cmdBuffer
    //!           Pointer to the command buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddHcpPipeBufAddrCmd(
        PMOS_COMMAND_BUFFER  cmdBuffer);

    //!
    //! \brief    Initialize MMC state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS InitMmcState();

    //!
    //! \brief    Compute Temporal Different
    //!
    //! \return   short
    //!           return the current picordercnt difference
    //!
    short ComputeTemporalDifferent(
        CODEC_PICTURE    refPic);

    //!
    //! \brief    Init surface codec params 1D
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    MOS_STATUS InitSurfaceCodecParams1D(
        CODECHAL_SURFACE_CODEC_PARAMS* p,
        PMOS_RESOURCE   buffer,
        uint32_t        size,
        uint32_t        offset,
        uint32_t        cacheabilityControl,
        uint32_t        bindingTableOffset,
        bool            isWritable);

    //!
    //! \brief    Init surface codec params 2D
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    MOS_STATUS InitSurfaceCodecParams2D(
        CODECHAL_SURFACE_CODEC_PARAMS* p,
        PMOS_SURFACE    surface,
        uint32_t        cacheabilityControl,
        uint32_t        bindingTableOffset,
        uint32_t        verticalLineStride,
        bool            isWritable);

    //!
    //! \brief    Update the slice count according to the slice shutdown policy
    //!
    virtual void UpdateSSDSliceCount() { return; };

    void MotionEstimationDisableCheck();

    //!
    //! \brief    Allocate 4x ME resources
    //!
    //! \param    [in] param
    //!           Pointer to parameters
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateResources4xME(
        HmeParams* param);

    //!
    //! \brief    Allocate 16x ME resources
    //!
    //! \param    [in] param
    //!           Pointer to parameters
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateResources16xME(
        HmeParams* param);

    //!
    //! \brief    Allocate 32x ME resources
    //!
    //! \param    [in] param
    //!           Pointer to parameters
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateResources32xME(
        HmeParams* param);

    //!
    //! \brief    Destroy ME resources
    //!
    //! \param    [in] param
    //!           Pointer to parameters
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DestroyMEResources(
        HmeParams* param);

#if USE_CODECHAL_DEBUG_TOOL
    MOS_STATUS DumpSeqParams(
        PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS seqParams);

    MOS_STATUS DumpPicParams(
        PCODEC_HEVC_ENCODE_PICTURE_PARAMS picParams);

    MOS_STATUS DumpFeiPicParams(
        CodecEncodeHevcFeiPicParams *feiPicParams);

    MOS_STATUS DumpSliceParams(
        PCODEC_HEVC_ENCODE_SLICE_PARAMS   sliceParams,
        PCODEC_HEVC_ENCODE_PICTURE_PARAMS picParams);

    MOS_STATUS DumpMbEncPakOutput(PCODEC_REF_LIST currRefList, CodechalDebugInterface* debugInterface);

    MOS_STATUS DumpFrameStatsBuffer(CodechalDebugInterface* debugInterface);

    //!
    //! \brief    Create HEVC PAR
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS CreateHevcPar();

    //!
    //! \brief    Destroy HEVC PAR
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DestroyHevcPar();

    //!
    //! \brief    Dump sequence PAR file
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS DumpSeqParFile() { return MOS_STATUS_SUCCESS; }

    //!
    //! \brief    Dump frame PAR file
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS DumpFrameParFile() { return MOS_STATUS_SUCCESS; }

    //!
    //! \brief    Populate const parameters
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS PopulateConstParam();

    //!
    //! \brief    Set MHW_VDBOX_AVC_IMG_STATE parameter
    //!
    //! \param    [in] hevcSeqParams
    //!           pointer to HEVC encode sequence parameters
    //! \param    [in] hevcPicParams
    //!           pointer to HEVC encode picture parameters
    //! \param    [in] hevcSlcParams
    //!           pointer to HEVC encode slice parameters
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS PopulateDdiParam(
        PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS hevcSeqParams,
        PCODEC_HEVC_ENCODE_PICTURE_PARAMS  hevcPicParams,
        PCODEC_HEVC_ENCODE_SLICE_PARAMS    hevcSlcParams);

    //!
    //! \brief    Set PMHW_VDBOX_HEVC_SLICE_STATE parameter
    //!
    //! \param    [in] adaptiveRoundingInterEnable
    //!           adaptive rounding inter enable flag
    //! \param    [in] sliceState
    //!           pointer to slice state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS PopulateSliceStateParam(
        bool                       adaptiveRoundingInterEnable,
        PMHW_VDBOX_HEVC_SLICE_STATE sliceState)
    {
        return MOS_STATUS_SUCCESS;
    };

    //!
    //! \brief    Set BRC init parameter
    //!
    //! \param    [in] *cmd
    //!           pointer to command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS PopulateBrcInitParam(void *cmd)
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Set BRC update parameter
    //!
    //! \param    [in] *cmd
    //!           pointer to command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS PopulateBrcUpdateParam(void *cmd) 
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Set encode parameter
    //!
    //! \param    [in] meMethod
    //!           ME method
    //! \param    [in] *cmd
    //!           pointer to command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS PopulateEncParam(
        uint8_t meMethod,
        void    *cmd) 
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Set MHW_VDBOX_AVC_IMG_STATE parameter
    //!
    //! \param    [in] cmdBuffer
    //!           pointer to command buffer
    //! \param    [in] secondLevelBatchBuffer
    //!           pointer to second level batch buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS PopulatePakParam(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_BATCH_BUFFER   secondLevelBatchBuffer) {
        return MOS_STATUS_SUCCESS;
    }

    EncodeHevcPar *m_hevcPar = nullptr;  //!< HEVC PAR parameters

#endif
};

//!
//! \brief    Get bitstream buffer size
//!
//! \param    [in] frameWidth
//!           The width of frame
//! \param    [in] frameHeight
//!           The height of frame
//! \param    [in] chromaFormat
//!           Chroma format
//! \param    [in] is10Bits
//!           Check if is 10bits
//! \return   uint32_t
//!           Bitstream buffer size
//!
uint32_t CodecHalHevcEncode_GetBitstreamBufferSize(
    uint32_t frameWidth,
    uint32_t frameHeight,
    uint8_t  chromaFormat,
    bool     is10Bits);

#endif  // __CODECHAL_ENCODE_HEVC_BASE_H__
