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
//! \file     codechal_encode_avc_base.h
//! \brief    Defines base class for AVC encoder.
//!

#ifndef __CODECHAL_ENCODE_AVC_BASE_H__
#define __CODECHAL_ENCODE_AVC_BASE_H__

#include "codechal_encoder_base.h"
#include "codechal_kernel_hme.h"
#if USE_CODECHAL_DEBUG_TOOL
#include "codechal_debug_encode_par.h"
#endif

#define CODECHAL_ENCODE_AVC_INVALID_ROUNDING                0xFF
#define CODECHAL_ENCODE_AVC_NUM_SYNC_TAGS                   64
#define CODECHAL_ENCODE_AVC_INIT_DSH_SIZE                   MHW_PAGE_SIZE * 3
#define CODECHAL_ENCODE_AVC_MAX_SLICE_QP                    (CODEC_AVC_NUM_QP - 1) // 0 - 51 inclusive
#define CODECHAL_ENCODE_AVC_MIN_ICQ_QUALITYFACTOR           1
#define CODECHAL_ENCODE_AVC_MAX_ICQ_QUALITYFACTOR           51
#define CODECHAL_ENCODE_AVC_MAX_SLICES_SUPPORTED            256
#define CODECHAL_ENCODE_AVC_DEFAULT_AVBR_ACCURACY           30
#define CODECHAL_ENCODE_AVC_MIN_AVBR_CONVERGENCE            100
#define CODECHAL_ENCODE_AVC_DEFAULT_AVBR_CONVERGENCE        150
#define CODECHAL_ENCODE_AVC_REF_PIC_SELECT_ENTRIES          (CODEC_AVC_MAX_NUM_REF_FRAME + 1) // one extra for current picture
// Invalid AVC PicID
#define CODECHAL_ENCODE_AVC_INVALID_PIC_ID                      CODEC_AVC_NUM_UNCOMPRESSED_SURFACE

#define CODECHAL_ENCODE_AVC_SFD_OUTPUT_BUFFER_SIZE_COMMON   128

#if USE_CODECHAL_DEBUG_TOOL
#define CODECHAL_DEBUG_ENCODE_AVC_NAL_START_CODE_SEI        0x00000106
#define CODECHAL_DEBUG_ENCODE_AVC_NAL_START_CODE_PPS        0x00000128

struct EncodeAvcPar
{
    // DDI Params
    uint8_t                     ProfileIDC;
    uint8_t                     LevelIDC;
    uint8_t                     CabacInitIDC;
    uint8_t                     ChromaFormatIDC;
    uint8_t                     PictureCodingType;
    uint8_t                     MaxRefIdxL0;
    uint8_t                     MaxRefIdxL1;
    uint8_t                     MaxBRefIdxL0;
    uint8_t                     BRCMethod;
    uint8_t                     BRCType;
    uint8_t                     DeblockingIDC;
    uint8_t                     WeightedPred;
    uint8_t                     WeightedBiPred;
    char                        ChromaQpOffset;
    char                        SecondChromaQpOffset;
    char                        ISliceQP;
    char                        PSliceQP;
    char                        BSliceQP;
    char                        DeblockingFilterAlpha;
    char                        DeblockingFilterBeta;
    bool                        EnableSEI;
    bool                        DisableVUIHeader;
    bool                        EnableWeightPredictionDetection;
    bool                        EntropyCodingMode;
    bool                        DirectInference;
    bool                        Transform8x8Mode;
    bool                        UseOrigAsRef;
    bool                        BiSubMbPartMask;
    uint16_t                    NumP;
    uint16_t                    NumB;
    uint16_t                    FrameRateM;
    uint16_t                    FrameRateD;
    uint16_t                    CRFQualityFactor;
    uint32_t                    NumSlices;
    bool                        ConstrainedIntraPred;
    uint8_t                     SliceMode;
    int16_t                     hme0XOffset;
    int16_t                     hme0YOffset;
    int16_t                     hme1XOffset;
    int16_t                     hme1YOffset;

    // HME Params
    uint8_t                     SuperCombineDist;
    bool                        SuperHME;
    bool                        UltraHME;
    bool                        StreamInEnable;
    uint8_t                     StreamInL0FromNewRef;
    uint8_t                     StreamInL1FromNewRef;

    // BRC init Params
    bool                        MBBRCEnable;
    bool                        MBRC;
    uint16_t                    AvbrAccuracy;
    uint16_t                    AvbrConvergence;
    uint32_t                    BitRate;
    uint32_t                    InitVbvFullnessInBit;
    uint32_t                    MaxBitRate;
    uint32_t                    VbvSzInBit;
    uint32_t                    SlidingWindowEnable;
    uint32_t                    SlidingWindowSize;
    uint32_t                    SlidingWindowMaxRateRatio;
    uint32_t                    LowDelayGoldenFrameBoost;
    uint32_t                    TopQPDeltaThrforAdaptive2Pass;
    uint32_t                    BotQPDeltaThrforAdaptive2Pass;
    uint32_t                    TopFrmSzPctThrforAdaptive2Pass;
    uint32_t                    BotFrmSzPctThrforAdaptive2Pass;
    uint32_t                    MBHeaderCompensation;
    uint32_t                    QPSelectMethodforFirstPass;
    bool                        MBQpCtrl;
    bool                        ICQReEncode;
    bool                        AdaptiveCostAdjustEnable;
    bool                        AdaptiveHMEExtension;
    uint32_t                    QPMin;
    uint32_t                    QPMax;
    bool                        HrdConformanceCheckDisable;
    uint8_t                     StreamInStaticRegion;
    uint8_t                     ScenarioInfo;
    bool                        SliceSizeWA;
    uint32_t                    INumMbsLag;
    uint32_t                    PNumMbsLag;
    uint32_t                    LongTermInterval;

    // BRC frame update Params
    bool                        EnableMultipass;
    uint8_t                     MaxNumPakPasses;
    uint32_t                    UserMaxFrame;
    uint32_t                    UserMaxFrameP;
    bool                        FrameSkipEnable;
    bool                        SceneChgDetectEn;
    uint32_t                    SceneChgPrevIntraPctThresh;
    uint32_t                    SceneChgCurIntraPctThresh;
    uint32_t                    SceneChgWidth0;
    uint32_t                    SceneChgWidth1;
    bool                        Transform8x8PDisable;
    uint32_t                    SliceSizeThr;
    uint32_t                    SliceMaxSize;

    // Enc Params
    uint8_t                     SubPelMode;
    uint8_t                     MaxLenSP;
    uint8_t                     MEMethod;
    uint8_t                     BMEMethod;
    uint8_t                     HMECombineOverlap;
    uint8_t                     SearchX;
    uint8_t                     SearchY;
    uint8_t                     BSearchX;
    uint8_t                     BSearchY;
    uint8_t                     SearchControl;
    uint8_t                     BSearchControl;
    uint8_t                     BiMixDisable;
    uint32_t                    BiWeight;
    bool                        DisableExtendedMvCostRange;
    bool                        EnableAdaptiveSearch;
    bool                        EnableFBRBypass;
    bool                        BlockBasedSkip;
    bool                        MADEnableFlag;
    bool                        EnableMBFlatnessCheckOptimization;
    bool                        EnableArbitrarySliceSize;
    bool                        EnableWavefrontOptimization;
    bool                        BSkipType;
    bool                        EnableAdaptiveTxDecision;
    bool                        EnablePerMBStaticCheck;
    bool                        EnableAdaptiveSearchWindowSize;
    bool                        EnableIntraCostScalingForStaticFrame;
    uint32_t                    StaticFrameZMVPercent;
    uint32_t                    StaticFrameIntraCostScalingRatioP;
    uint32_t                    StaticFrameIntraCostScalingRatioB;
    bool                        AdaptiveMvStreamIn;
    uint32_t                    LargeMvThresh;
    uint32_t                    LargeMvPctThreshold;
    bool                        UniMixDisable;
    bool                        DirectMode;
    uint16_t                    RefThresh;
    uint16_t                    MBTextureThreshold;
    uint16_t                    TxDecisionThr;
    uint32_t                    MRDisableQPCheck;
    uint32_t                    AllFractional;
    uint32_t                    DisableAllFractionalCheckForHighRes;
    uint32_t                    HMECombineLen;
    uint32_t                    HMEBCombineLen;
    uint32_t                    FTQBasedSkip;
    uint32_t                    MultiplePred;
    uint32_t                    EnableAdaptiveIntraScaling;
    uint32_t                    SurvivedSkipCost;
    bool                        VDEncPerfMode;
    bool                        VdencExtPakObjDisable;
    bool                        PPMVDisable;
    uint8_t                     LeftNbrPelMode;
    uint8_t                     DisPSubPartMask;
    uint8_t                     DisPSubMbMask;
    uint8_t                     DisBSubPartMask;
    uint8_t                     DisBSubMbMask;
    uint8_t                     ImePredOverlapThr;
    uint8_t                     MBSizeEstScalingRatioINTRA;
    bool                        IntraMBHdrScaleFactor;
    uint8_t                     MBSizeEstScalingRatioINTER;
    bool                        InterMBHdrScaleFactor;
    uint8_t                     PFrameMaxNumImePred;
    uint8_t                     BFrameMaxNumImePred;
    uint8_t                     HMERefWindowSize;
    uint8_t                     IMELeftPredDep;
    uint8_t                     NumFMECandCheck;
    uint8_t                     PFrameImePredLargeSW;
    uint8_t                     BFrameImePredLargeSW;
    bool                        RdoChromaEnable;
    uint16_t                    Intra4x4ModeMask;
    uint16_t                    Intra8x8ModeMask;
    bool                        RdoIntraChromaSearch;
    uint8_t                     Intra16x16ModeMask;
    uint8_t                     InitMBBudgetTr4x4;
    bool                        ROIEnable;
    uint8_t                     PFrameZeroCbfEn;
    uint8_t                     BFrameZeroCbfEn;
    uint8_t                     ForceIPCMMinQP;
    uint8_t                     IntraTr4x4Percent;
    bool                        MultiPassHmeEnable;

    // PAK Params
    uint8_t                     RoundingIntra;
    bool                        RoundingIntraEnabled;
    bool                        EnableAdaptiveRounding;
    bool                        RoundingInterEnabled;
    bool                        EnableAdaptiveTrellisQuantization;
    bool                        TrellisQuantizationEnable;
    bool                        TrellisQuantizationChromaDisable;
    uint32_t                    RoundingInter;
    uint32_t                    RoundingInterB;
    uint32_t                    TrellisQuantizationRounding;
    uint8_t                     FrmHdrEncodingFrequency;
    bool                        ExtendedRhoDomainEn;
};
#endif

static const uint8_t CodecHal_TargetUsageToMode_AVC[NUM_TARGET_USAGE_MODES] =
{ encodeNormalMode,  encodeQualityMode,  encodeQualityMode, encodeNormalMode,
encodeNormalMode, encodeNormalMode, encodeNormalMode, encodePerformanceMode };

typedef struct _CODEC_AVC_REF_PIC_SELECT_LIST
{
    uint8_t                 FrameIdx;
    MOS_SURFACE             sBuffer;
} CODEC_AVC_REF_PIC_SELECT_LIST, *PCODEC_AVC_REF_PIC_SELECT_LIST;

typedef struct _CODECHAL_ENCODE_AVC_TQ_PARAMS
{
    uint32_t    dwTqEnabled;
    uint32_t    dwTqRounding;
} CODECHAL_ENCODE_AVC_TQ_PARAMS, *PCODECHAL_ENCODE_AVC_TQ_PARAMS;

typedef struct _CODECHAL_ENCODE_AVC_GENERIC_PICTURE_LEVEL_PARAMS
{
    PMOS_SURFACE                psPreDeblockSurface;
    PMOS_SURFACE                psPostDeblockSurface;
    PMOS_RESOURCE               presMacroblockIldbStreamOutBuffer1;
    PMOS_RESOURCE               presMacroblockIldbStreamOutBuffer2;

    bool                        bBrcEnabled;
    PMHW_BATCH_BUFFER           pImgStateBatchBuffer;
    PMOS_RESOURCE               presBrcHistoryBuffer;

    bool                        bDeblockerStreamOutEnable;
    bool                        bPostDeblockOutEnable;
    bool                        bPreDeblockOutEnable;
} CODECHAL_ENCODE_AVC_GENERIC_PICTURE_LEVEL_PARAMS, *PCODECHAL_ENCODE_AVC_GENERIC_PICTURE_LEVEL_PARAMS;

typedef struct _CODECHAL_ENCODE_AVC_ME_CURBE
{
    // DW0
    union
    {
        struct
        {
            uint32_t   SkipModeEn                          : MOS_BITFIELD_BIT(       0 );
            uint32_t   AdaptiveEn                          : MOS_BITFIELD_BIT(       1 );
            uint32_t   BiMixDis                            : MOS_BITFIELD_BIT(       2 );
            uint32_t                                       : MOS_BITFIELD_RANGE(  3, 4 );
            uint32_t   EarlyImeSuccessEn                   : MOS_BITFIELD_BIT(       5 );
            uint32_t                                       : MOS_BITFIELD_BIT(       6 );
            uint32_t   T8x8FlagForInterEn                  : MOS_BITFIELD_BIT(       7 );
            uint32_t                                       : MOS_BITFIELD_RANGE(  8,23 );
            uint32_t   EarlyImeStop                        : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW0;

    // DW1
    union
    {
        struct
        {
            uint32_t   MaxNumMVs                           : MOS_BITFIELD_RANGE(  0, 5 );
            uint32_t                                       : MOS_BITFIELD_RANGE(  6,15 );
            uint32_t   BiWeight                            : MOS_BITFIELD_RANGE( 16,21 );
            uint32_t                                       : MOS_BITFIELD_RANGE( 22,27 );
            uint32_t   UniMixDisable                       : MOS_BITFIELD_BIT(      28 );
            uint32_t                                       : MOS_BITFIELD_RANGE( 29,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW1;

    // DW2
    union
    {
        struct
        {
            uint32_t   MaxLenSP                            : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   MaxNumSU                            : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t                                       : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW2;

    // DW3
    union
    {
        struct
        {
            uint32_t   SrcSize                             : MOS_BITFIELD_RANGE(  0, 1 );
            uint32_t                                       : MOS_BITFIELD_RANGE(  2, 3 );
            uint32_t   MbTypeRemap                         : MOS_BITFIELD_RANGE(  4, 5 );
            uint32_t   SrcAccess                           : MOS_BITFIELD_BIT(       6 );
            uint32_t   RefAccess                           : MOS_BITFIELD_BIT(       7 );
            uint32_t   SearchCtrl                          : MOS_BITFIELD_RANGE(  8,10 );
            uint32_t   DualSearchPathOption                : MOS_BITFIELD_BIT(      11 );
            uint32_t   SubPelMode                          : MOS_BITFIELD_RANGE( 12,13 );
            uint32_t   SkipType                            : MOS_BITFIELD_BIT(      14 );
            uint32_t   DisableFieldCacheAlloc              : MOS_BITFIELD_BIT(      15 );
            uint32_t   InterChromaMode                     : MOS_BITFIELD_BIT(      16 );
            uint32_t   FTEnable                            : MOS_BITFIELD_BIT(      17 );
            uint32_t   BMEDisableFBR                       : MOS_BITFIELD_BIT(      18 );
            uint32_t   BlockBasedSkipEnable                : MOS_BITFIELD_BIT(      19 );
            uint32_t   InterSAD                            : MOS_BITFIELD_RANGE( 20,21 );
            uint32_t   IntraSAD                            : MOS_BITFIELD_RANGE( 22,23 );
            uint32_t   SubMbPartMask                       : MOS_BITFIELD_RANGE( 24,30 );
            uint32_t                                       : MOS_BITFIELD_BIT(      31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW3;

    // DW4
    union
    {
        struct
        {
            uint32_t                                       : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   PictureHeightMinus1                 : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   PictureWidth                        : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t                                       : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW4;

    // DW5
    union
    {
        struct
        {
            uint32_t                                       : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   QpPrimeY                            : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   RefWidth                            : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   RefHeight                           : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW5;

    // DW6
    union
    {
        struct
        {
            uint32_t                                       : MOS_BITFIELD_RANGE(  0, 2 );
            uint32_t   WriteDistortions                    : MOS_BITFIELD_BIT(       3 );
            uint32_t   UseMvFromPrevStep                   : MOS_BITFIELD_BIT(       4 );
            uint32_t                                       : MOS_BITFIELD_RANGE(  5, 7 );
            uint32_t   SuperCombineDist                    : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   MaxVmvR                             : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW6;

    // DW7
    union
    {
        struct
        {
            uint32_t                                       : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   MVCostScaleFactor                   : MOS_BITFIELD_RANGE( 16,17 );
            uint32_t   BilinearEnable                      : MOS_BITFIELD_BIT(      18 );
            uint32_t   SrcFieldPolarity                    : MOS_BITFIELD_BIT(      19 );
            uint32_t   WeightedSADHAAR                     : MOS_BITFIELD_BIT(      20 );
            uint32_t   AConlyHAAR                          : MOS_BITFIELD_BIT(      21 );
            uint32_t   RefIDCostMode                       : MOS_BITFIELD_BIT(      22 );
            uint32_t                                       : MOS_BITFIELD_BIT(      23 );
            uint32_t   SkipCenterMask                      : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW7;

    // DW8
    union
    {
        struct
        {
            uint32_t   Mode0Cost                           : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   Mode1Cost                           : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   Mode2Cost                           : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   Mode3Cost                           : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW8;

    // DW9
    union
    {
        struct
        {
            uint32_t   Mode4Cost                           : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   Mode5Cost                           : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   Mode6Cost                           : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   Mode7Cost                           : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW9;

    // DW10
    union
    {
        struct
        {
            uint32_t   Mode8Cost                           : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   Mode9Cost                           : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   RefIDCost                           : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   ChromaIntraModeCost                 : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW10;

    // DW11
    union
    {
        struct
        {
            uint32_t   MV0Cost                             : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   MV1Cost                             : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   MV2Cost                             : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   MV3Cost                             : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW11;

    // DW12
    union
    {
        struct
        {
            uint32_t   MV4Cost                             : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   MV5Cost                             : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   MV6Cost                             : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   MV7Cost                             : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW12;

    // DW13
    union
    {
        struct
        {
            uint32_t   NumRefIdxL0MinusOne                 : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   NumRefIdxL1MinusOne                 : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   RefStreaminCost                     : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   ROIEnable                           : MOS_BITFIELD_RANGE( 24,26 );
            uint32_t                                       : MOS_BITFIELD_RANGE( 27,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW13;

    // DW14
    union
    {
        struct
        {
            uint32_t   List0RefID0FieldParity              : MOS_BITFIELD_BIT(       0 );
            uint32_t   List0RefID1FieldParity              : MOS_BITFIELD_BIT(       1 );
            uint32_t   List0RefID2FieldParity              : MOS_BITFIELD_BIT(       2 );
            uint32_t   List0RefID3FieldParity              : MOS_BITFIELD_BIT(       3 );
            uint32_t   List0RefID4FieldParity              : MOS_BITFIELD_BIT(       4 );
            uint32_t   List0RefID5FieldParity              : MOS_BITFIELD_BIT(       5 );
            uint32_t   List0RefID6FieldParity              : MOS_BITFIELD_BIT(       6 );
            uint32_t   List0RefID7FieldParity              : MOS_BITFIELD_BIT(       7 );
            uint32_t   List1RefID0FieldParity              : MOS_BITFIELD_BIT(       8 );
            uint32_t   List1RefID1FieldParity              : MOS_BITFIELD_BIT(       9 );
            uint32_t                                       : MOS_BITFIELD_RANGE( 10,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW14;

    // DW15
    union
    {
        struct
        {
            uint32_t   PrevMvReadPosFactor                 : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   MvShiftFactor                       : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   Reserved                            : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW15;

    struct
    {
        // DW16
        union
        {
            struct
            {
                SearchPathDelta   SPDelta_0;
                SearchPathDelta   SPDelta_1;
                SearchPathDelta   SPDelta_2;
                SearchPathDelta   SPDelta_3;
            };
            struct
            {
                uint32_t   Value;
            };
        } DW16;

        // DW17
        union
        {
            struct
            {
                SearchPathDelta   SPDelta_4;
                SearchPathDelta   SPDelta_5;
                SearchPathDelta   SPDelta_6;
                SearchPathDelta   SPDelta_7;
            };
            struct
            {
                uint32_t   Value;
            };
        } DW17;

        // DW18
        union
        {
            struct
            {
                SearchPathDelta   SPDelta_8;
                SearchPathDelta   SPDelta_9;
                SearchPathDelta   SPDelta_10;
                SearchPathDelta   SPDelta_11;
            };
            struct
            {
                uint32_t   Value;
            };
        } DW18;

        // DW19
        union
        {
            struct
            {
                SearchPathDelta   SPDelta_12;
                SearchPathDelta   SPDelta_13;
                SearchPathDelta   SPDelta_14;
                SearchPathDelta   SPDelta_15;
            };
            struct
            {
                uint32_t   Value;
            };
        } DW19;

        // DW20
        union
        {
            struct
            {
                SearchPathDelta   SPDelta_16;
                SearchPathDelta   SPDelta_17;
                SearchPathDelta   SPDelta_18;
                SearchPathDelta   SPDelta_19;
            };
            struct
            {
                uint32_t   Value;
            };
        } DW20;

        // DW21
        union
        {
            struct
            {
                SearchPathDelta   SPDelta_20;
                SearchPathDelta   SPDelta_21;
                SearchPathDelta   SPDelta_22;
                SearchPathDelta   SPDelta_23;
            };
            struct
            {
                uint32_t   Value;
            };
        } DW21;

        // DW22
        union
        {
            struct
            {
                SearchPathDelta   SPDelta_24;
                SearchPathDelta   SPDelta_25;
                SearchPathDelta   SPDelta_26;
                SearchPathDelta   SPDelta_27;
            };
            struct
            {
                uint32_t   Value;
            };
        } DW22;

        // DW23
        union
        {
            struct
            {
                SearchPathDelta   SPDelta_28;
                SearchPathDelta   SPDelta_29;
                SearchPathDelta   SPDelta_30;
                SearchPathDelta   SPDelta_31;
            };
            struct
            {
                uint32_t   Value;
            };
        } DW23;

        // DW24
        union
        {
            struct
            {
                SearchPathDelta   SPDelta_32;
                SearchPathDelta   SPDelta_33;
                SearchPathDelta   SPDelta_34;
                SearchPathDelta   SPDelta_35;
            };
            struct
            {
                uint32_t   Value;
            };
        } DW24;

        // DW25
        union
        {
            struct
            {
                SearchPathDelta   SPDelta_36;
                SearchPathDelta   SPDelta_37;
                SearchPathDelta   SPDelta_38;
                SearchPathDelta   SPDelta_39;
            };
            struct
            {
                uint32_t   Value;
            };
        } DW25;

        // DW26
        union
        {
            struct
            {
                SearchPathDelta   SPDelta_40;
                SearchPathDelta   SPDelta_41;
                SearchPathDelta   SPDelta_42;
                SearchPathDelta   SPDelta_43;
            };
            struct
            {
                uint32_t   Value;
            };
        } DW26;

        // DW27
        union
        {
            struct
            {
                SearchPathDelta   SPDelta_44;
                SearchPathDelta   SPDelta_45;
                SearchPathDelta   SPDelta_46;
                SearchPathDelta   SPDelta_47;
            };
            struct
            {
                uint32_t   Value;
            };
        } DW27;

        // DW28
        union
        {
            struct
            {
                SearchPathDelta   SPDelta_48;
                SearchPathDelta   SPDelta_49;
                SearchPathDelta   SPDelta_50;
                SearchPathDelta   SPDelta_51;
            };
            struct
            {
                uint32_t   Value;
            };
        } DW28;

        // DW29
        union
        {
            struct
            {
                SearchPathDelta   SPDelta_52;
                SearchPathDelta   SPDelta_53;
                SearchPathDelta   SPDelta_54;
                SearchPathDelta   SPDelta_55;
            };
            struct
            {
                uint32_t   Value;
            };
        } DW29;
    } SPDelta;

    // DW30
    union
    {
        struct
        {
            uint32_t   ActualMBWidth                            : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ActualMBHeight                           : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW30;

    // DW31
    union
    {
        struct
        {
            uint32_t   Reserved                                 : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW31;

    // DW32
    union
    {
        struct
        {
            uint32_t   _4xMeMvOutputDataSurfIndex               : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW32;

    // DW33
    union
    {
        struct
        {
            uint32_t   _16xOr32xMeMvInputDataSurfIndex          : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW33;

    // DW34
    union
    {
        struct
        {
            uint32_t   _4xMeOutputDistSurfIndex                 : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW34;

    // DW35
    union
    {
        struct
        {
            uint32_t   _4xMeOutputBrcDistSurfIndex              : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW35;

    // DW36
    union
    {
        struct
        {
            uint32_t   VMEFwdInterPredictionSurfIndex           : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW36;

    // DW37
    union
    {
        struct
        {
            uint32_t   VMEBwdInterPredictionSurfIndex           : MOS_BITFIELD_RANGE( 0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW37;

    // DW38
    union
    {
        struct
        {
            uint32_t   VDEncStreamInSurfIndex                   : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW38;

} CODECHAL_ENCODE_AVC_ME_CURBE, *PCODECHAL_ENCODE_AVC_ME_CURBE;
C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(CODECHAL_ENCODE_AVC_ME_CURBE)) == 39);

// AVC ME CURBE init data for G9 CM Kernel
static const uint32_t g_cInit_CODECHAL_ENCODE_AVC_ME_CURBE[39] =
{
    0x00000000, 0x00200010, 0x00003939, 0x77a43000, 0x00000000, 0x28300000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
};

// CURBE for Static Frame Detection kernel
typedef struct _CODECHAL_ENCODE_AVC_SFD_CURBE_COMMON
{
    union
    {
        struct
        {
            uint32_t   VDEncModeDisable                            : MOS_BITFIELD_BIT(        0 );
            uint32_t   BRCModeEnable                               : MOS_BITFIELD_BIT(        1 );
            uint32_t   SliceType                                   : MOS_BITFIELD_RANGE(  2,  3 );
            uint32_t                                               : MOS_BITFIELD_BIT(        4 );
            uint32_t   StreamInType                                : MOS_BITFIELD_RANGE(  5,  8 );
            uint32_t   EnableAdaptiveMvStreamIn                    : MOS_BITFIELD_BIT(        9 );
            uint32_t                                               : MOS_BITFIELD_BIT(       10 );
            uint32_t   EnableIntraCostScalingForStaticFrame        : MOS_BITFIELD_BIT(       11 );
            uint32_t   Reserved                                    : MOS_BITFIELD_RANGE( 12, 31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW0;

    union
    {
        struct
        {
            uint32_t   QPValue                                     : MOS_BITFIELD_RANGE(  0,  7 );
            uint32_t   NumOfRefs                                   : MOS_BITFIELD_RANGE(  8, 15 );
            uint32_t   HMEStreamInRefCost                          : MOS_BITFIELD_RANGE( 16, 23 );
            uint32_t   Reserved                                    : MOS_BITFIELD_RANGE( 24, 31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW1;

    union
    {
        struct
        {
            uint32_t   FrameWidthInMBs                             : MOS_BITFIELD_RANGE(  0, 15 );     // round-up to 4-MB aligned
            uint32_t   FrameHeightInMBs                            : MOS_BITFIELD_RANGE( 16, 31 );     // round-up to 4-MB aligned
        };
        struct
        {
            uint32_t   Value;
        };
    } DW2;

    union
    {
        struct
        {
            uint32_t   LargeMvThresh                               : MOS_BITFIELD_RANGE( 0, 31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW3;

    union
    {
        struct
        {
            uint32_t   TotalLargeMvThreshold                       : MOS_BITFIELD_RANGE( 0, 31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW4;

    union
    {
        struct
        {
            uint32_t   ZMVThreshold                                : MOS_BITFIELD_RANGE( 0, 31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW5;

    union
    {
        struct
        {
            uint32_t   TotalZMVThreshold                           : MOS_BITFIELD_RANGE( 0, 31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW6;

    union
    {
        struct
        {
            uint32_t   MinDistThreshold                            : MOS_BITFIELD_RANGE( 0, 31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW7;

    uint8_t                                                        CostTable[52];

    union
    {
        struct
        {
            uint32_t   ActualWidthInMB                             : MOS_BITFIELD_RANGE(  0, 15 );
            uint32_t   ActualHeightInMB                            : MOS_BITFIELD_RANGE( 16, 31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW21;

    union
    {
        struct
        {
            uint32_t   Reserved                                    : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW22;

    union
    {
        struct
        {
            uint32_t   Reserved                                    : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW23;

    union
    {
        struct
        {
            uint32_t   VDEncInputImagStateIndex                    : MOS_BITFIELD_RANGE( 0, 31 );      // used in VDEnc CQP mode
        };
        struct
        {
            uint32_t   Value;
        };
    } DW24;

    union
    {
        struct
        {
            uint32_t   Reserved                                    : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW25;

    union
    {
        struct
        {
            uint32_t   MVDataSurfaceIndex                          : MOS_BITFIELD_RANGE( 0, 31 );      // contains HME MV Data generated by HME kernel
        };
        struct
        {
            uint32_t   Value;
        };
    } DW26;

    union
    {
        struct
        {
            uint32_t   InterDistortionSurfaceIndex                 : MOS_BITFIELD_RANGE( 0, 31 );      // contains HME Inter Distortion generated by HME kernel
        };
        struct
        {
            uint32_t   Value;
        };
    } DW27;

    union
    {
        struct
        {
            uint32_t   OutputDataSurfaceIndex                      : MOS_BITFIELD_RANGE( 0, 31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW28;

    union
    {
        struct
        {
            uint32_t   VDEncOutputImagStateIndex                   : MOS_BITFIELD_RANGE( 0, 31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW29;

} CODECHAL_ENCODE_AVC_SFD_CURBE_COMMON, *PCODECHAL_ENCODE_AVC_SFD_CURBE_COMMON;
C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(CODECHAL_ENCODE_AVC_SFD_CURBE_COMMON)) == 30);

typedef struct _CODECHAL_ENCODE_AVC_SFD_SURFACE_PARAMS
{
    uint32_t                                dwDownscaledWidthInMb4x;
    uint32_t                                dwDownscaledHeightInMb4x;
    uint32_t                                dwMeMvBottomFieldOffset;
    uint32_t                                dwMeDistortionBottomFieldOffset;
    PMOS_RESOURCE                           presVDEncImageStateInputBuffer;
    PMOS_SURFACE                            psMeMvDataSurface;
    PMOS_SURFACE                            psMeDistortionSurface;
    PMOS_RESOURCE                           presOutputBuffer;
    PMOS_RESOURCE                           presVDEncImageStateOutputBuffer;
    PMHW_KERNEL_STATE                       pKernelState;
    bool                                    bVdencActive;
    bool                                    bVdencBrcEnabled;
} CODECHAL_ENCODE_AVC_SFD_SURFACE_PARAMS, *PCODECHAL_ENCODE_AVC_SFD_SURFACE_PARAMS;

typedef struct _CODECHAL_ENCODE_AVC_PACK_PIC_HEADER_PARAMS
{
    PBSBuffer                               pBsBuffer;
    PCODEC_AVC_ENCODE_PIC_PARAMS            pPicParams;     // pAvcPicParams[ucPPSIdx]
    PCODEC_AVC_ENCODE_SEQUENCE_PARAMS       pSeqParams;     // pAvcSeqParams[ucSPSIdx]
    PCODECHAL_ENCODE_AVC_VUI_PARAMS         pAvcVuiParams;
    PCODEC_AVC_IQ_MATRIX_PARAMS          pAvcIQMatrixParams;
    PCODECHAL_NAL_UNIT_PARAMS               *ppNALUnitParams;
    CodechalEncodeSeiData*                  pSeiData;
    uint32_t                                dwFrameHeight;
    uint32_t                                dwOriFrameHeight;
    uint16_t                                wPictureCodingType;
    bool                                    bNewSeq;
    bool                                   *pbNewPPSHeader;
    bool                                   *pbNewSeqHeader;
} CODECHAL_ENCODE_AVC_PACK_PIC_HEADER_PARAMS, *PCODECHAL_ENCODE_AVC_PACK_PIC_HEADER_PARAMS;

typedef struct _CODECHAL_ENCODE_AVC_PACK_SLC_HEADER_PARAMS
{
    PBSBuffer                               pBsBuffer;
    PCODEC_AVC_ENCODE_PIC_PARAMS            pPicParams;     // pAvcPicParams[ucPPSIdx]
    PCODEC_AVC_ENCODE_SEQUENCE_PARAMS       pSeqParams;     // pAvcSeqParams[ucSPSIdx]
    PCODEC_AVC_ENCODE_SLICE_PARAMS          pAvcSliceParams;
    PCODEC_REF_LIST                         *ppRefList;
    CODEC_PICTURE                           CurrPic;
    CODEC_PICTURE                           CurrReconPic;
    CODEC_AVC_ENCODE_USER_FLAGS             UserFlags;
    CODECHAL_ENCODE_AVC_NAL_UNIT_TYPE       NalUnitType;
    uint16_t                                wPictureCodingType;
    bool                                    bVdencEnabled;
} CODECHAL_ENCODE_AVC_PACK_SLC_HEADER_PARAMS, *PCODECHAL_ENCODE_AVC_PACK_SLC_HEADER_PARAMS;

typedef struct _CODECHAL_ENCODE_AVC_VALIDATE_NUM_REFS_PARAMS
{
    PCODEC_AVC_ENCODE_SEQUENCE_PARAMS       pSeqParams;     // pAvcSeqParams[ucSPSIdx]
    PCODEC_AVC_ENCODE_PIC_PARAMS            pPicParams;
    PCODEC_AVC_ENCODE_SLICE_PARAMS          pAvcSliceParams;
    uint16_t                                wPictureCodingType;
    uint16_t                                wPicHeightInMB;
    uint16_t                                wFrameFieldHeightInMB;
    bool                                    bFirstFieldIPic;
    bool                                    bVDEncEnabled;
    bool                                    bPAKonly;
} CODECHAL_ENCODE_AVC_VALIDATE_NUM_REFS_PARAMS, *PCODECHAL_ENCODE_AVC_VALIDATE_NUM_REFS_PARAMS;

typedef struct _CODECHAL_ENCODE_AVC_TQ_INPUT_PARAMS
{
    uint16_t  wPictureCodingType;
    uint8_t   ucTargetUsage;
    uint8_t   ucQP;
    bool      bBrcEnabled;
    bool      bVdEncEnabled;
} CODECHAL_ENCODE_AVC_TQ_INPUT_PARAMS, *PCODECHAL_ENCODE_AVC_TQ_INPUT_PARAMS;

typedef struct _CODECHAL_ENCODE_AVC_SFD_CURBE_PARAMS
{
    PMHW_KERNEL_STATE                       pKernelState;
} CODECHAL_ENCODE_AVC_SFD_CURBE_PARAMS, *PCODECHAL_ENCODE_AVC_SFD_CURBE_PARAMS;

     //!
     //! \brief    Vertical MV component range based on levelIdc
     //! \details  VDBOX private function to get vertical MV componet range
     //! \param    [in] levelIdc
     //!           AVC level
     //! \return   uint32_t
     //!           return the max vertical mv component base on input level
     //!
     uint32_t CodecHalAvcEncode_GetMaxVmvR(uint8_t levelIdc);

     //!
     //! \brief    Get MV component range based on levelIdc
     //! \details  VDBOX private function to get MV componet range
     //! \param    [in] levelIdc
     //!           AVC level
     //! \return   uint32_t
     //!           return the max mv component base on input level
     //!
     uint32_t CodecHalAvcEncode_GetMaxMvLen(uint8_t levelIdc);

     //!
     //! \brief    Get the filed parity: Top filed or Bottom filed
     //! \details  Client facing function to get the filed parity: Top filed or Bottom filed
     //! \param    [in] params
     //!           PCODEC_AVC_ENCODE_SLICE_PARAMS pSlcParams
     //! \param    [list] list
     //!           forword or backword reference
     //! \param    [in] index
     //!           reference frame index
     //! \return   uint32_t
     //!           Bottom field or Top field
     //!
     bool CodecHalAvcEncode_GetFieldParity(
        PCODEC_AVC_ENCODE_SLICE_PARAMS      params,
        uint32_t                            list,
        uint32_t                            index);

     //!
     //! \brief    Get profile level max frame size
     //! \param    [in] seqParams
     //!           Encoder Sequence params
     //! \param    [in] encoder
     //!           Encoder structure
     //! \param    [in] profileLevelMaxFrame
     //!           Profile Level Max Frame
     //! \return   MOS_STATUS
     //!           MOS_STATUS_SUCCESS if success, else fail reason
     //!
     MOS_STATUS CodecHalAvcEncode_GetProfileLevelMaxFrameSize(
        PCODEC_AVC_ENCODE_SEQUENCE_PARAMS   seqParams,
        CodechalEncoderState*               encoder,
        uint32_t*                           profileLevelMaxFrame);

     //! \brief    Get the max number of allowed slice
     //! \param    [in] profileIdc
     //!           AVC profile idc
     //! \param    [in] levelIdc
     //!           AVC level idc
     //! \param    [in] framesPer100Sec
     //!           frame Per 100Sec
     //! \return   uint16_t
     //!           return uiMaxAllowedNumSlices
     //!
     uint16_t CodecHalAvcEncode_GetMaxNumSlicesAllowed(
        CODEC_AVC_PROFILE_IDC profileIdc,
        CODEC_AVC_LEVEL_IDC   levelIdc,
        uint32_t                 framesPer100Sec);

     //!
     //! \brief    Use to pack picture header related params
     //! \param    [in] params
     //!           picture header pack params
     //! \return   MOS_STATUS
     //!           MOS_STATUS_SUCCESS if success, else fail reason
     //!
     MOS_STATUS CodecHalAvcEncode_PackPictureHeader(
        PCODECHAL_ENCODE_AVC_PACK_PIC_HEADER_PARAMS    params);

     //!
     //! \brief    Use to pack slice header related params
     //! \param    [in] params
     //!           slice header pack params
     //! \return   MOS_STATUS
     //!           MOS_STATUS_SUCCESS if success, else fail reason
     //!
     MOS_STATUS CodecHalAvcEncode_PackSliceHeader(
        PCODECHAL_ENCODE_AVC_PACK_SLC_HEADER_PARAMS    params);

     //!
     //! \brief    Use to get picture num of slice header to be packed
     //! \param    [in] params
     //!           pack slice header pack params
     //! \param    [in] list
     //!           forword or backword reference
     //! \return   void
     //!
     static void CodecHal_PackSliceHeader_GetPicNum (
        PCODECHAL_ENCODE_AVC_PACK_SLC_HEADER_PARAMS     params,
        uint8_t                                         list);

//!
//! \class    CodechalEncodeAvcBase
//! \brief    CodechalE encode Avc base
//!
class CodechalEncodeAvcBase : public CodechalEncoderState
{
public:
    // AvcState functions.
    //!
    //! \brief    Constructor
    //!
    CodechalEncodeAvcBase(
        CodechalHwInterface *   hwInterface,
        CodechalDebugInterface *debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo);

    //!
    //! \brief    Copy constructor
    //!
    CodechalEncodeAvcBase(const CodechalEncodeAvcBase&) = delete;

    //!
    //! \brief    Copy assignment operator
    //!
    CodechalEncodeAvcBase& operator=(const CodechalEncodeAvcBase&) = delete;

    //!
    //! \brief    Destructor
    //!
    virtual ~CodechalEncodeAvcBase();

    //Encode interface definitions
    //!
    //! \brief    Initialize standard related members.
    //! \details  Initialize members, set proper value
    //!           to involved variables, allocate resources.
    //! \param    [in] settings
    //!           Encode settings.
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS Initialize(CodechalSetting * settings);

    virtual MOS_STATUS GetStatusReport(
        EncodeStatus* encodeStatus,
        EncodeStatusReport* encodeStatusReport)
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Encode User Feature Key Report.
    //! \details  Report user feature values set by encode.
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS UserFeatureKeyReport();

    //!
    //! \brief    Initialize Encode ME kernel state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS InitKernelStateMe() = 0;

    //!
    //! \brief    Set Encode ME kernel Curbe data.
    //!
    //! \param    [in] params
    //!           Pointer to the MeCurbeParams
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS SetCurbeMe( MeCurbeParams* params ) = 0;

    //!
    //! \brief    Set Encode ME kernel Surfaces
    //!
    //! \param    [in] cmdBuffer
    //!           Pointer to the MOS_COMMAND_BUFFER
    //! \param    [in] params
    //!           Pointer to the CODECHAL_ME_SURFACE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS SendMeSurfaces(
        PMOS_COMMAND_BUFFER         cmdBuffer,
        MeSurfaceParams* params) = 0;
    //!
    //! \brief    AVC State Initialization.
    //! 
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS Initialize();


    //!
    //! \brief    AVC Resource Allocation for ENC.
    //! 
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS AllocateEncResources();

    //!
    //! \brief    AVC Resource Allocation for Encoder.
    //! 
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS AllocateResources();

    //!
    //! \brief    Set AVC Sequence Structs.
    //! 
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS SetSequenceStructs();

    //!
    //! \brief    Set AVC Picture Structs.
    //! 
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS SetPictureStructs();

    //!
    //! \brief    Set AVC Slice Structs.
    //! 
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS SetSliceStructs();

    //!
    //! \brief    Run Encode ME kernel
    //!
    //! \param    [in] brcBuffers
    //!           Pointer to the EncodeBrcBuffers
    //! \param    [in] hmeLevel
    //!           Hme level
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS EncodeMeKernel(
        EncodeBrcBuffers* brcBuffers,
        HmeLevel          hmeLevel);

    //!
    //! \brief    Allocate Batch Buffer For PakSlice.
    //! 
    //! \param    [in] numSlices
    //!           Number of Slice
    //! \param    [in] numPakPasses
    //!           Number of PAK pass.
    //! \param    [in] currRecycledBufIdx
    //!           Current Recycle Buffer Index.
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS AllocateBatchBufferForPakSlices(
        uint32_t numSlices,
        uint8_t numPakPasses,
        uint8_t currRecycledBufIdx);

    //!
    //! \brief    Release Batch Buffer For PakSlice.
    //! 
    //! \param    [in] currRecycledBufIdx
    //!           Current Recycle Buffer Index.
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS ReleaseBatchBufferForPakSlices( uint8_t currRecycledBufIdx);

    //!
    //! \brief    Initialize kernel binary size info.
    //! 
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    MOS_STATUS AddIshSize(uint32_t kuid, uint8_t* kernelBase);

    //!
    //! \brief    Calculate scaling list
    //!
    //! \return   void
    //!
    void ScalingListFlat();

    //!
    //! \brief    Calculate scaling list
    //!
    //! \return   void
    //!
    void ScalingListFallbackRuleA();

    //!
    //! \brief    Get Dist Scale factor
    //!
    //! \return   void
    //!
    void GetDistScaleFactor();

    //!
    //! \brief    Initialize MMC state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS InitMmcState();

    //!
    //! \brief    Get bidirectional weight
    //!
    //! \param    [in] distScaleFactorRefID0List0
    //!           DistScaleFactorRefID0List0
    //! \param    [in] weightedBiPredIdc
    //!           Same as AVC syntax element.
    //!
    //! \return   int32_t
    //!           Bidirectional weight
    //!
    int32_t GetBiWeight(
        uint32_t distScaleFactorRefID0List0,
        uint16_t weightedBiPredIdc);

    //!
    //! \brief    Update the slice count according to the slice shutdown policy
    //!
    virtual void UpdateSSDSliceCount();

    //!
    //! \brief    Build slices with header insertion
    //! \param    [in] cmdBuffer
    //!           command buffer
    //! \param    [in] params
    //!           VDBOX AVC slice state params
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SendSlice(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_VDBOX_AVC_SLICE_STATE      params);

    //!
    //! \brief      Store number passes
    //! 
    //! \param      [in] encodeStatusBuf
    //!             Encode status buffer
    //! \param      [in] miInterface
    //!             Mi interface
    //! \param      [in] cmdBuffer
    //!             Command buffer
    //! \param      [in] currPass
    //!             Curr pass
    //!
    //! \return     MOS_STATUS
    //!             MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS StoreNumPasses(
        EncodeStatusBuffer             *encodeStatusBuf,
        MhwMiInterface                 *miInterface,
        PMOS_COMMAND_BUFFER            cmdBuffer,
        uint32_t                       currPass);

#if USE_CODECHAL_DEBUG_TOOL
    MOS_STATUS DumpSeqParams(
        PCODEC_AVC_ENCODE_SEQUENCE_PARAMS seqParams,
        PCODEC_AVC_IQ_MATRIX_PARAMS    matrixParams);

    MOS_STATUS DumpPicParams(
        PCODEC_AVC_ENCODE_PIC_PARAMS   picParams,
        PCODEC_AVC_IQ_MATRIX_PARAMS matrixParams);

    MOS_STATUS DumpFeiPicParams(
        CodecEncodeAvcFeiPicParams *feiPicParams);

    MOS_STATUS DumpSliceParams(
        PCODEC_AVC_ENCODE_SLICE_PARAMS sliceParams,
        PCODEC_AVC_ENCODE_PIC_PARAMS   picParams);

    MOS_STATUS DumpVuiParams(
        PCODECHAL_ENCODE_AVC_VUI_PARAMS avcVuiParams);

#endif

protected:
    //!
    //! \brief    Set MFX_PIPE_MODE_SELECT parameter
    //!
    //! \param    [in] genericParam
    //!           CODECHAL_ENCODE_AVC_GENERIC_PICTURE_LEVEL_PARAMS
    //! \param    [out] param
    //!           reference to MHW_VDBOX_PIPE_MODE_SELECT_PARAMS
    //!
    //! \return   void
    //!
    virtual void SetMfxPipeModeSelectParams(
        const CODECHAL_ENCODE_AVC_GENERIC_PICTURE_LEVEL_PARAMS& genericParam,
        MHW_VDBOX_PIPE_MODE_SELECT_PARAMS& param);

    //!
    //! \brief    set MFX_PIPE_BUF_ADDR_STATE parameter
    //!
    //! \param    [in] genericParam
    //!           CODECHAL_ENCODE_AVC_GENERIC_PICTURE_LEVEL_PARAMS
    //! \param    [out] param
    //!           reference to MHW_VDBOX_PIPE_BUF_ADDR_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS SetMfxPipeBufAddrStateParams(
        CODECHAL_ENCODE_AVC_GENERIC_PICTURE_LEVEL_PARAMS genericParam,
        MHW_VDBOX_PIPE_BUF_ADDR_PARAMS& param);

    //!
    //! \brief    Set MFX_IND_OBJ_BASE_ADDR_STATE parameter
    //!
    //! \param    [out] param
    //!           reference to MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS
    //!
    //! \return   void
    //!
    virtual void SetMfxIndObjBaseAddrStateParams(MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS& param);

    //!
    //! \brief    Set MHW_VDBOX_BSP_BUF_BASE_ADDR_STATE parameter
    //!
    //! \param    [out] param
    //!           reference to MHW_VDBOX_BSP_BUF_BASE_ADDR_PARAMS
    //!
    //! \return   void
    //!
    virtual void SetMfxBspBufBaseAddrStateParams(MHW_VDBOX_BSP_BUF_BASE_ADDR_PARAMS& param);

    //!
    //! \brief    Set MHW_VDBOX_QM_STATE parameter
    //!
    //! \param    [out] qmParams
    //!           reference to MHW_VDBOX_QM_PARAMS
    //! \param    [out] fqmParams
    //!           reference to MHW_VDBOX_QM_PARAMS
    //!
    //! \return   void
    //!
    virtual void SetMfxQmStateParams(MHW_VDBOX_QM_PARAMS& qmParams, MHW_VDBOX_QM_PARAMS & fqmParams);

    //!
    //! \brief    Set MHW_VDBOX_AVC_IMG_STATE parameter
    //!
    //! \param    [out] param
    //!           reference to MHW_VDBOX_AVC_IMG_PARAMS
    //!
    //! \return   void
    //!
    virtual void SetMfxAvcImgStateParams(MHW_VDBOX_AVC_IMG_PARAMS& param);

public:
    PMOS_INTERFACE                              m_origOsInterface           = nullptr;    //!< Os Interface
    CodechalHwInterface                         *m_origHwInterface          = nullptr;    //!< Hw Interface
    PMHW_STATE_HEAP_INTERFACE                   m_origStateHeapInterface    = nullptr;    //!< StateHeap Interface

    // Parameters passed by application
    PCODEC_AVC_ENCODE_PIC_PARAMS                m_avcPicParams[CODEC_AVC_MAX_PPS_NUM];    //!< Pointer to array of picture parameter, could be removed
    PCODEC_AVC_ENCODE_SEQUENCE_PARAMS           m_avcSeqParams[CODEC_AVC_MAX_SPS_NUM];    //!< Pointer to array of sequence parameter, could be removed
    PCODEC_AVC_ENCODE_PIC_PARAMS                m_avcPicParam           = nullptr;  //!< Pointer to AVC picture parameter
    PCODEC_AVC_ENCODE_SEQUENCE_PARAMS           m_avcSeqParam           = nullptr;  //!< Pointer to AVC sequence parameter
    PCODECHAL_ENCODE_AVC_QUALITY_CTRL_PARAMS    m_avcQCParams           = nullptr;  //!< Pointer to video quality control parameter
    PCODECHAL_ENCODE_AVC_ROUNDING_PARAMS        m_avcRoundingParams     = nullptr;  //!< Pointer to AVC rounding parameter
    PCODECHAL_ENCODE_AVC_VUI_PARAMS             m_avcVuiParams          = nullptr;  //!< Pointer to AVC Uvi parameter
    PCODEC_AVC_ENCODE_SLICE_PARAMS              m_avcSliceParams        = nullptr;  //!< Pointer to AVC slice parameter
    CodecEncodeAvcFeiPicParams                  *m_avcFeiPicParams       = nullptr;  //!< Pointer to FEI picture parameter
    PCODEC_AVC_IQ_MATRIX_PARAMS              m_avcIQMatrixParams     = nullptr;  //!< Pointer to IQMaxtrix parameter
    PCODEC_AVC_ENCODE_IQ_WEIGTHSCALE_LISTS      m_avcIQWeightScaleLists = nullptr;  //!< Pointer to IQWidght ScaleLists
    CODEC_AVC_ENCODE_USER_FLAGS                 m_userFlags;                        //!< Encoder user flag settings

    CODEC_PIC_ID                                m_picIdx[CODEC_AVC_MAX_NUM_REF_FRAME];              //!< Picture index
    PCODEC_REF_LIST                             m_refList[CODEC_AVC_NUM_UNCOMPRESSED_SURFACE];   //!< Pointer to reference list
    CODEC_AVC_FRAME_STORE_ID                 m_avcFrameStoreID[CODEC_AVC_MAX_NUM_REF_FRAME];     //!< Refer to CODEC_AVC_FRAME_STORE_ID
    CODECHAL_ENCODE_AVC_NAL_UNIT_TYPE           m_nalUnitType;                      //!< Nal unit type
    PCODECHAL_NAL_UNIT_PARAMS                   *m_nalUnitParams    = nullptr;      //!< Pointers to NAL unit parameters array
    uint16_t                                    m_sliceHeight       = 0;            //!< Slice height
    uint8_t                                     m_biWeightNumB      = 0;            //!< Bi direction Weight B frames num
    bool                                        m_deblockingEnabled = false;        //!< Enable deblocking flag
    bool                                        m_mbaffEnabled      = false;        //!< Enable MBAFF flag
    bool                                        m_firstFieldIdrPic  = false;        //!< First field IDR flag
    bool                                        m_hmeEnabled        = false;        //!< Enable HME flag
    bool                                        m_16xMeEnabled      = false;        //!< Enable 16x ME flag
    bool                                        m_32xMeEnabled      = false;        //!< Enable 32x ME flag
    bool                                        m_skipBiasAdjustmentEnable  = false;//!< Enable SkipBiasAdjustment flag
    bool                                        m_staticFrameDetectionInUse = false;//!< Enable Static Frame Detection flag
    uint32_t                                    m_sliceStructCaps = 0;              //!< Slice struct

    CODECHAL_ENCODE_AVC_TQ_PARAMS               m_trellisQuantParams;               //!< Trellis Quantization params

    // B-frame
    uint32_t                                    m_distScaleFactorList0[CODEC_AVC_MAX_NUM_REF_FRAME * 2];    //!< the DistScaleFactor used to derive temporal direct motion vector
    uint32_t                                    m_biWeight = 0;                     //!< Bidirectional Weight

    // Batch Buffers
    MHW_BATCH_BUFFER                            m_batchBufferForVdencImgStat[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM]; //!< VDEnc image state batch buffers

    // ME
    CodechalKernelHme                           *m_hmeKernel = nullptr;           //!< ME kernel object
    MOS_SURFACE                                 m_4xMeMvDataBuffer;               //!< 4x motion estimation MV data buffer
    uint32_t                                    m_meMvBottomFieldOffset = 0;      //!< ME MV bottom filed offset
    MOS_SURFACE                                 m_16xMeMvDataBuffer;              //!< 16x motion estimation MV data buffer
    uint32_t                                    m_meMv16xBottomFieldOffset = 0;   //!< 16x motion estimation MV bottom filed offset
    MOS_SURFACE                                 m_32xMeMvDataBuffer;              //!< 32x motion estimation MV data buffer
    uint32_t                                    m_meMv32xBottomFieldOffset = 0;   //!< 32x motion estimation MV bottom filed offset
    MOS_SURFACE                                 m_4xMeDistortionBuffer;           //!< 4x ME distortion buffer
    uint32_t                                    m_meDistortionBottomFieldOffset = 0; //!< ME distortion bottom field offset
    uint8_t                                     *m_bmeMethodTable = nullptr;      //!< Point to BME Method table
    uint8_t                                     *m_meMethodTable  = nullptr;      //!< Pointer to ME Method table

    // PAK Scratch Buffers
    MOS_RESOURCE                                m_intraRowStoreScratchBuffer;                                   //!< Handle of intra row store surface
    MHW_BATCH_BUFFER                            m_batchBufferForPakSlices[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM]; //!< PAK Slice batch buffers
    uint32_t                                    m_pakSliceSize = 0;               //!< PAK Slice Size
    uint32_t                                    m_pakSlicePatchListSize = 0;      //!< PAK Slice patch list size
    uint32_t                                    m_currPakSliceIdx = 0;            //!< Current PAK slice index

    bool                                        m_4xMeDistortionBufferSupported = false;  //!< Generation Specific Support Flags & User Feature Key Reads

    bool                                        m_sliceSizeStreamoutSupported = false;    //!< Enable PAK slice size streamout, valid for AVC VDEnc on KBL/CNL+
    MOS_RESOURCE                                m_pakSliceSizeStreamoutBuffer;            //!< PAK slice size streamout buffer

    bool                                        m_crePrefetchEnable = false;              //!< Enable CRE prefetch flag
    bool                                        m_tlbPrefetchEnable = false;              //!< Enable TLB prefetch flag

    const uint8_t m_codechalEncodeAvcSfdCostTablePFrame[CODEC_AVC_NUM_QP] =
        {
            44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 60, 60, 60, 60, 73, 73, 73, 76, 76, 76, 88, 89, 89, 91, 92, 93, 104, 104, 106, 107, 108, 109, 120, 120, 122, 123, 124, 125, 136, 136, 138, 139, 140, 141, 143, 143};

    const uint8_t m_codechalEncodeAvcSfdCostTableBFrame[CODEC_AVC_NUM_QP] =
        {
            57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 73, 73, 73, 73, 77, 77, 77, 89, 89, 89, 91, 93, 93, 95, 105, 106, 107, 108, 110, 111, 121, 122, 123, 124, 125, 127, 137, 138, 139, 140, 142, 143, 143, 143, 143, 143};

    const uint8_t m_codechalEncodeAvcSfdCostTableVdEnc[CODEC_AVC_NUM_QP] =
        {
            45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 47, 56, 57, 59, 44, 45, 47, 56, 44, 47, 47, 45, 47, 47, 47, 47, 45, 47, 47, 56, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47};

#if USE_CODECHAL_DEBUG_TOOL
protected:
    //!
    //! \brief    Create AVC PAR
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS CreateAvcPar();

    //!
    //! \brief    Destroy AVC PAR
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DestroyAvcPar();

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
    MOS_STATUS PopulateConstParam();

    //!
    //! \brief    Populate target usage as the first parameter of dumped par file
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS PopulateTargetUsage();

    //!
    //! \brief    Set MHW_VDBOX_AVC_IMG_STATE parameter
    //!
    //! \param    [in] avcSeqParams
    //!           pointer to AVC encode sequence parameters
    //! \param    [in] avcPicParams
    //!           pointer to AVC encode picture parameters
    //! \param    [in] avcSlcParams
    //!           pointer to AVC encode slice parameters
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS PopulateDdiParam(
        PCODEC_AVC_ENCODE_SEQUENCE_PARAMS avcSeqParams,
        PCODEC_AVC_ENCODE_PIC_PARAMS      avcPicParams,
        PCODEC_AVC_ENCODE_SLICE_PARAMS    avcSlcParams);

    //!
    //! \brief    Set MHW_VDBOX_AVC_IMG_STATE parameter
    //!
    //! \param    [in] adaptiveRoundingInterEnable
    //!           adaptive rounding inter enable flag
    //! \param    [in] sliceState
    //!           pointer to slice state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS PopulateSliceStateParam(
        bool                       adaptiveRoundingInterEnable,
        PMHW_VDBOX_AVC_SLICE_STATE sliceState);

    //!
    //! \brief    Set MHW_VDBOX_AVC_IMG_STATE parameter
    //!
    //! \param    [in] *cmd
    //!           pointer to command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS PopulateSfdParam(
        void *cmd);

    //!
    //! \brief    Set MHW_VDBOX_AVC_IMG_STATE parameter
    //!
    //! \param    [in] is16xMeEnabled
    //!           16x ME enabled flag
    //! \param    [in] is32xMeEnabled
    //!           32x ME enabled flag
    //! \param    [in] meMethod
    //!           ME method
    //! \param    [in] *cmd
    //!           pointer to command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS PopulateHmeParam(
        bool    is16xMeEnabled,
        bool    is32xMeEnabled,
        uint8_t meMethod,
        void    *cmd) { return MOS_STATUS_SUCCESS; }

    //!
    //! \brief    Set MHW_VDBOX_AVC_IMG_STATE parameter
    //!
    //! \param    [in] *cmd
    //!           pointer to command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS PopulateBrcInitParam(
        void *cmd) { return MOS_STATUS_SUCCESS; }

    //!
    //! \brief    Set MHW_VDBOX_AVC_IMG_STATE parameter
    //!
    //! \param    [in] *cmd
    //!           pointer to command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS PopulateBrcUpdateParam(
        void *cmd) { return MOS_STATUS_SUCCESS; }

    //!
    //! \brief    Set MHW_VDBOX_AVC_IMG_STATE parameter
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
        void    *cmd) { return MOS_STATUS_SUCCESS; }

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
        PMHW_BATCH_BUFFER   secondLevelBatchBuffer) { return MOS_STATUS_SUCCESS; }

    EncodeAvcPar *m_avcPar             = nullptr;  //!< AVC PAR parameters
    bool         m_populateTargetUsage = false;
#endif

    //!
    //! \brief    Set frame store Id for avc codec.
    //! \details
    //! \return   frameIdx
    //!           [in] frame index
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetFrameStoreIds(uint8_t frameIdx);
};
#endif // __CODECHAL_ENCODE_AVC_BASE_H__
