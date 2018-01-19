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
//! \file     codechal_encode_avc_g9_skl.cpp
//! \brief    AVC dual-pipe encoder for GEN9 SKL & BXT.
//!
#include "codechal_encode_avc_g9_skl.h"
#include "igcodeckrn_g9.h"
#if USE_CODECHAL_DEBUG_TOOL
#include "mhw_vdbox_mfx_hwcmd_g9_skl.h"
#endif

#define MBENC_NUM_TARGET_USAGES_CM_G9_SKL       3

const uint32_t CodechalEncodeAvcEncG9Skl::TrellisQuantizationEnable[NUM_TARGET_USAGE_MODES] =
{
0, 1, 0, 0, 0, 0, 0, 0
};

const uint32_t CodechalEncodeAvcEncG9Skl::TrellisQuantizationRounding[NUM_TARGET_USAGE_MODES] =
{
0, 6, 0, 0, 0, 0, 0, 0
};

const uint32_t CodechalEncodeAvcEncG9Skl::EnableAdaptiveTrellisQuantization[NUM_TARGET_USAGE_MODES] =
{
0, 1, 0, 0, 0, 0, 0, 0
};

typedef struct _CODECHAL_ENCODE_AVC_MBENC_CURBE_CM_G9
{
    // DW0
    union
    {
        struct
        {
            uint32_t   SkipModeEn                      : MOS_BITFIELD_BIT(       0 );
            uint32_t   AdaptiveEn                      : MOS_BITFIELD_BIT(       1 );
            uint32_t   BiMixDis                        : MOS_BITFIELD_BIT(       2 );
            uint32_t                                   : MOS_BITFIELD_RANGE(  3, 4 );
            uint32_t   EarlyImeSuccessEn               : MOS_BITFIELD_BIT(       5 );
            uint32_t                                   : MOS_BITFIELD_BIT(       6 );
            uint32_t   T8x8FlagForInterEn              : MOS_BITFIELD_BIT(       7 );
            uint32_t                                   : MOS_BITFIELD_RANGE(  8,23 );
            uint32_t   EarlyImeStop                    : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   MaxNumMVs                       : MOS_BITFIELD_RANGE(  0, 5 );
            uint32_t                                   : MOS_BITFIELD_RANGE(  6,15 );
            uint32_t   BiWeight                        : MOS_BITFIELD_RANGE( 16,21 );
            uint32_t                                   : MOS_BITFIELD_RANGE( 22,27 );
            uint32_t   UniMixDisable                   : MOS_BITFIELD_BIT(      28 );
            uint32_t                                   : MOS_BITFIELD_RANGE( 29,31 );
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
            uint32_t   LenSP                           : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   MaxNumSU                        : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   PicWidth                        : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   SrcSize                         : MOS_BITFIELD_RANGE(  0, 1 );
            uint32_t                                   : MOS_BITFIELD_RANGE(  2, 3 );
            uint32_t   MbTypeRemap                     : MOS_BITFIELD_RANGE(  4, 5 );
            uint32_t   SrcAccess                       : MOS_BITFIELD_BIT(       6 );
            uint32_t   RefAccess                       : MOS_BITFIELD_BIT(       7 );
            uint32_t   SearchCtrl                      : MOS_BITFIELD_RANGE(  8,10 );
            uint32_t   DualSearchPathOption            : MOS_BITFIELD_BIT(      11 );
            uint32_t   SubPelMode                      : MOS_BITFIELD_RANGE( 12,13 );
            uint32_t   SkipType                        : MOS_BITFIELD_BIT(      14 );
            uint32_t   DisableFieldCacheAlloc          : MOS_BITFIELD_BIT(      15 );
            uint32_t   InterChromaMode                 : MOS_BITFIELD_BIT(      16 );
            uint32_t   FTEnable                        : MOS_BITFIELD_BIT(      17 );
            uint32_t   BMEDisableFBR                   : MOS_BITFIELD_BIT(      18 );
            uint32_t   BlockBasedSkipEnable            : MOS_BITFIELD_BIT(      19 );
            uint32_t   InterSAD                        : MOS_BITFIELD_RANGE( 20,21 );
            uint32_t   IntraSAD                        : MOS_BITFIELD_RANGE( 22,23 );
            uint32_t   SubMbPartMask                   : MOS_BITFIELD_RANGE( 24,30 );
            uint32_t                                   : MOS_BITFIELD_BIT(      31 );
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
            uint32_t   PicHeightMinus1                     : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   MvRestrictionInSliceEnable          : MOS_BITFIELD_BIT(      16 );
            uint32_t   DeltaMvEnable                       : MOS_BITFIELD_BIT(      17 );
            uint32_t   TrueDistortionEnable                : MOS_BITFIELD_BIT(      18 );
            uint32_t   EnableWavefrontOptimization         : MOS_BITFIELD_BIT(      19);
            uint32_t                                       : MOS_BITFIELD_BIT(      20);
            uint32_t   EnableIntraCostScalingForStaticFrame: MOS_BITFIELD_BIT(      21);
            uint32_t   EnableIntraRefresh                  : MOS_BITFIELD_BIT(      22);
            uint32_t   Reserved                            : MOS_BITFIELD_BIT(      23);
            uint32_t   EnableDirtyRect                     : MOS_BITFIELD_BIT(      24);
            uint32_t   bCurFldIDR                          : MOS_BITFIELD_BIT(      25 );
            uint32_t   ConstrainedIntraPredFlag            : MOS_BITFIELD_BIT(      26 );
            uint32_t   FieldParityFlag                     : MOS_BITFIELD_BIT(      27 );
            uint32_t   HMEEnable                           : MOS_BITFIELD_BIT(      28 );
            uint32_t   PictureType                         : MOS_BITFIELD_RANGE( 29,30 );
            uint32_t   UseActualRefQPValue                 : MOS_BITFIELD_BIT(      31 );
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
            uint32_t   SliceMbHeight                   : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   RefWidth                        : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   RefHeight                       : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   BatchBufferEnd                  : MOS_BITFIELD_RANGE(  0,31 );
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
            uint32_t   IntraPartMask                   : MOS_BITFIELD_RANGE(  0, 4 );
            uint32_t   NonSkipZMvAdded                 : MOS_BITFIELD_BIT(       5 );
            uint32_t   NonSkipModeAdded                : MOS_BITFIELD_BIT(       6 );
            uint32_t   LumaIntraSrcCornerSwap          : MOS_BITFIELD_BIT(       7 );
            uint32_t                                   : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   MVCostScaleFactor               : MOS_BITFIELD_RANGE( 16,17 );
            uint32_t   BilinearEnable                  : MOS_BITFIELD_BIT(      18 );
            uint32_t   SrcFieldPolarity                : MOS_BITFIELD_BIT(      19 );
            uint32_t   WeightedSADHAAR                 : MOS_BITFIELD_BIT(      20 );
            uint32_t   AConlyHAAR                      : MOS_BITFIELD_BIT(      21 );
            uint32_t   RefIDCostMode                   : MOS_BITFIELD_BIT(      22 );
            uint32_t                                   : MOS_BITFIELD_BIT(      23 );
            uint32_t   SkipCenterMask                  : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   Mode0Cost                       : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   Mode1Cost                       : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   Mode2Cost                       : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   Mode3Cost                       : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   Mode4Cost                       : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   Mode5Cost                       : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   Mode6Cost                       : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   Mode7Cost                       : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   Mode8Cost                       : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   Mode9Cost                       : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   RefIDCost                       : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   ChromaIntraModeCost             : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   MV0Cost                         : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   MV1Cost                         : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   MV2Cost                         : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   MV3Cost                         : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   MV4Cost                         : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   MV5Cost                         : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   MV6Cost                         : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   MV7Cost                         : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   QpPrimeY                        : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   QpPrimeCb                       : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   QpPrimeCr                       : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   TargetSizeInWord                : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   SICFwdTransCoeffThreshold_0     : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   SICFwdTransCoeffThreshold_1     : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   SICFwdTransCoeffThreshold_2     : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   SICFwdTransCoeffThreshold_3     : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   SICFwdTransCoeffThreshold_4     : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   SICFwdTransCoeffThreshold_5     : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   SICFwdTransCoeffThreshold_6     : MOS_BITFIELD_RANGE( 24,31 );    // Highest Freq
        };
        struct
        {
            uint32_t   Value;
        };
    } DW15;

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

    // DW30
    union
    {
        struct
        {
            uint32_t   Intra4x4ModeMask                : MOS_BITFIELD_RANGE(  0, 8 );
            uint32_t                                   : MOS_BITFIELD_RANGE(  9,15 );
            uint32_t   Intra8x8ModeMask                : MOS_BITFIELD_RANGE( 16,24 );
            uint32_t                                   : MOS_BITFIELD_RANGE( 25,31 );
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
            uint32_t   Intra16x16ModeMask              : MOS_BITFIELD_RANGE(  0, 3 );
            uint32_t   IntraChromaModeMask             : MOS_BITFIELD_RANGE(  4, 7 );
            uint32_t   IntraComputeType                : MOS_BITFIELD_RANGE(  8, 9 );
            uint32_t                                   : MOS_BITFIELD_RANGE( 10,31 );
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
            uint32_t   SkipVal                         : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   MultiPredL0Disable              : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   MultiPredL1Disable              : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   Intra16x16NonDCPredPenalty      : MOS_BITFIELD_RANGE(  0,7 );
            uint32_t   Intra8x8NonDCPredPenalty        : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   Intra4x4NonDCPredPenalty        : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t                                   : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   List0RefID0FieldParity          : MOS_BITFIELD_BIT(       0 );
            uint32_t   List0RefID1FieldParity          : MOS_BITFIELD_BIT(       1 );
            uint32_t   List0RefID2FieldParity          : MOS_BITFIELD_BIT(       2 );
            uint32_t   List0RefID3FieldParity          : MOS_BITFIELD_BIT(       3 );
            uint32_t   List0RefID4FieldParity          : MOS_BITFIELD_BIT(       4 );
            uint32_t   List0RefID5FieldParity          : MOS_BITFIELD_BIT(       5 );
            uint32_t   List0RefID6FieldParity          : MOS_BITFIELD_BIT(       6 );
            uint32_t   List0RefID7FieldParity          : MOS_BITFIELD_BIT(       7 );
            uint32_t   List1RefID0FrameFieldFlag       : MOS_BITFIELD_BIT(       8 );
            uint32_t   List1RefID1FrameFieldFlag       : MOS_BITFIELD_BIT(       9 );
            uint32_t   IntraRefreshEn                  : MOS_BITFIELD_RANGE( 10,11 );
            uint32_t   ArbitraryNumMbsPerSlice         : MOS_BITFIELD_BIT(      12 );
            uint32_t   ForceNonSkipMbEnable            : MOS_BITFIELD_BIT(      13 );
            uint32_t   DisableEncSkipCheck             : MOS_BITFIELD_BIT(      14 );
            uint32_t   EnableDirectBiasAdjustment      : MOS_BITFIELD_BIT(      15 );
            uint32_t   EnableGlobalMotionBiasAdjustment: MOS_BITFIELD_BIT(      16 );
            uint32_t   bForceToSkip                    : MOS_BITFIELD_BIT(      17 );
            uint32_t                                   : MOS_BITFIELD_RANGE( 18,23 );
            uint32_t   List1RefID0FieldParity          : MOS_BITFIELD_BIT(      24 );
            uint32_t   List1RefID1FieldParity          : MOS_BITFIELD_BIT(      25 );
            uint32_t   MADEnableFlag                   : MOS_BITFIELD_BIT(      26 );
            uint32_t   ROIEnableFlag                   : MOS_BITFIELD_BIT(      27 );
            uint32_t   EnableMBFlatnessChkOptimization : MOS_BITFIELD_BIT(      28 );
            uint32_t   bDirectMode                     : MOS_BITFIELD_BIT(      29 );
            uint32_t   MBBrcEnable                     : MOS_BITFIELD_BIT(      30 );
            uint32_t   bOriginalBff                    : MOS_BITFIELD_BIT(      31 );
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
            uint32_t   PanicModeMBThreshold            : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   SmallMbSizeInWord               : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   LargeMbSizeInWord               : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   NumRefIdxL0MinusOne             : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   HMECombinedExtraSUs             : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   NumRefIdxL1MinusOne             : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   EnableCABACAdvanced             : MOS_BITFIELD_BIT(      24 );
            uint32_t                                   : MOS_BITFIELD_RANGE( 25,27 );
            uint32_t   IsFwdFrameShortTermRef          : MOS_BITFIELD_BIT(      28 );
            uint32_t   CheckAllFractionalEnable        : MOS_BITFIELD_BIT(      29 );
            uint32_t   HMECombineOverlap               : MOS_BITFIELD_RANGE( 30,31 );
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
            uint32_t   SkipModeEn                      : MOS_BITFIELD_BIT(       0 );
            uint32_t   AdaptiveEn                      : MOS_BITFIELD_BIT(       1 );
            uint32_t   BiMixDis                        : MOS_BITFIELD_BIT(       2 );
            uint32_t                                   : MOS_BITFIELD_RANGE(  3, 4 );
            uint32_t   EarlyImeSuccessEn               : MOS_BITFIELD_BIT(       5 );
            uint32_t                                   : MOS_BITFIELD_BIT(       6 );
            uint32_t   T8x8FlagForInterEn              : MOS_BITFIELD_BIT(       7 );
            uint32_t                                   : MOS_BITFIELD_RANGE(  8,23 );
            uint32_t   EarlyImeStop                    : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   LenSP                           : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   MaxNumSU                        : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   RefThreshold                    : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW38;

    // DW39
    union
    {
        struct
        {
            uint32_t                                   : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   HMERefWindowsCombThreshold      : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   RefWidth                        : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   RefHeight                       : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW39;

    // DW40
    union
    {
        struct
        {
            uint32_t   DistScaleFactorRefID0List0      : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   DistScaleFactorRefID1List0      : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW40;

    // DW41
    union
    {
        struct
        {
            uint32_t   DistScaleFactorRefID2List0      : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   DistScaleFactorRefID3List0      : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW41;

    // DW42
    union
    {
        struct
        {
            uint32_t   DistScaleFactorRefID4List0      : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   DistScaleFactorRefID5List0      : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW42;

    // DW43
    union
    {
        struct
        {
            uint32_t   DistScaleFactorRefID6List0      : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   DistScaleFactorRefID7List0      : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW43;

    // DW44
    union
    {
        struct
        {
            uint32_t   ActualQPValueForRefID0List0     : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   ActualQPValueForRefID1List0     : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   ActualQPValueForRefID2List0     : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   ActualQPValueForRefID3List0     : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW44;

    // DW45
    union
    {
        struct
        {
            uint32_t   ActualQPValueForRefID4List0     : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   ActualQPValueForRefID5List0     : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   ActualQPValueForRefID6List0     : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   ActualQPValueForRefID7List0     : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW45;

    // DW46
    union
    {
        struct
        {
            uint32_t   ActualQPValueForRefID0List1     : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   ActualQPValueForRefID1List1     : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   RefCost                         : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW46;

    // DW47
    union
    {
        struct
        {
            uint32_t   MbQpReadFactor                  : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   IntraCostSF                     : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   MaxVmvR                         : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW47;

    //DW48
    union
    {
        struct
        {
            uint32_t   IntraRefreshMBx                 : MOS_BITFIELD_RANGE( 0, 15 );
            uint32_t   IntraRefreshUnitInMBMinus1      : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   IntraRefreshQPDelta             : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW48;

    // DW49
    union
    {
        struct
        {
            uint32_t   ROI1_X_left                     : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI1_Y_top                      : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW49;

    // DW50
    union
    {
        struct
        {
            uint32_t   ROI1_X_right                    : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI1_Y_bottom                   : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW50;

    // DW51
    union
    {
        struct
        {
            uint32_t   ROI2_X_left                     : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI2_Y_top                      : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW51;

    // DW52
    union
    {
        struct
        {
            uint32_t   ROI2_X_right                    : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI2_Y_bottom                   : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW52;

    // DW53
    union
    {
        struct
        {
            uint32_t   ROI3_X_left                     : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI3_Y_top                      : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW53;

    // DW54
    union
    {
        struct
        {
            uint32_t   ROI3_X_right                    : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI3_Y_bottom                   : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW54;

    // DW55
    union
    {
        struct
        {
            uint32_t   ROI4_X_left                     : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI4_Y_top                      : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW55;

    // DW56
    union
    {
        struct
        {
            uint32_t   ROI4_X_right                    : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI4_Y_bottom                   : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW56;

    // DW57
    union
    {
        struct
        {
            uint32_t   ROI1_dQpPrimeY                   : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   ROI2_dQpPrimeY                   : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   ROI3_dQpPrimeY                   : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   ROI4_dQpPrimeY                   : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW57;

    // DW58
    union
    {
        struct
        {
            uint32_t   HMEMVCostScalingFactor          : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   Reserved                        : MOS_BITFIELD_RANGE( 8, 15 );
            uint32_t   IntraRefreshMBy                 : MOS_BITFIELD_RANGE( 16, 31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW58;

    // DW59
    union
    {
        struct
        {
            uint32_t   Reserved                        : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW59;

    // DW60
    union
    {
        struct
        {
            uint32_t   CabacWaZone0Threshold           : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   CabacWaZone1Threshold           : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW60;

    // DW61
    union
    {
        struct
        {
            uint32_t   CabacWaZone2Threshold           : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   CabacWaZone3Threshold           : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW61;

    // DW62
    union
    {
        struct
        {
            uint32_t   CABACWAZone0IntraMinQP          : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   CABACWAZone1IntraMinQP          : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   CABACWAZone2IntraMinQP          : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   CABACWAZone3IntraMinQP          : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW62;

    // DW63
    union
    {
        struct
        {
            uint32_t   Reserved                        : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW63;

    // DW64
    union
    {
        struct
        {
            uint32_t   Reserved                        : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW64;

    // DW65
    union
    {
        struct
        {
            uint32_t   MBDataSurfIndex                 : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW65;

    // DW66
    union
    {
        struct
        {
            uint32_t   MVDataSurfIndex                 : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW66;

    // DW67
    union
    {
        struct
        {
            uint32_t   IDistSurfIndex                  : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW67;

    // DW68
    union
    {
        struct
        {
            uint32_t   SrcYSurfIndex                   : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW68;

    // DW69
    union
    {
        struct
        {
            uint32_t   MBSpecificDataSurfIndex         : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW69;

    // DW70
    union
    {
        struct
        {
            uint32_t   AuxVmeOutSurfIndex              : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW70;

    // DW71
    union
    {
        struct
        {
            uint32_t   CurrRefPicSelSurfIndex          : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW71;

    // DW72
    union
    {
        struct
        {
            uint32_t   HMEMVPredFwdBwdSurfIndex        : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW72;

    // DW73
    union
    {
        struct
        {
            uint32_t   HMEDistSurfIndex                : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW73;

    // DW74
    union
    {
        struct
        {
            uint32_t   SliceMapSurfIndex               : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW74;

    // DW75
    union
    {
        struct
        {
            uint32_t   FwdFrmMBDataSurfIndex           : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW75;

    // DW76
    union
    {
        struct
        {
            uint32_t   FwdFrmMVSurfIndex               : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW76;

    // DW77
    union
    {
        struct
        {
            uint32_t   MBQPBuffer                      : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW77;

    // DW78
    union
    {
        struct
        {
            uint32_t   MBBRCLut                        : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW78;

    // DW79
    union
    {
        struct
        {
            uint32_t   VMEInterPredictionSurfIndex     : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW79;

    // DW80
    union
    {
        struct
        {
            uint32_t   VMEInterPredictionMRSurfIndex   : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW80;

    // DW81
    union
    {
        struct
        {
            uint32_t   FlatnessChkSurfIndex            : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW81;

    // DW82
    union
    {
        struct
        {
            uint32_t   MADSurfIndex                    : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW82;

    // DW83
    union
    {
        struct
        {
            uint32_t   ForceNonSkipMBmapSurface        : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW83;

    // DW84
    union
    {
        struct
        {
            uint32_t   Reserved                        : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW84;

    // DW85
    union
    {
        struct
        {
            uint32_t   BRCCurbeSurfIndex               : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW85;

    // DW86
    union
    {
        struct
        {
            uint32_t   StaticDetectionOutputBufferIndex: MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW86;

    // DW87
    union
    {
        struct
        {
            uint32_t   Reserved                        : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW87;

} CODECHAL_ENCODE_AVC_MBENC_CURBE_CM_G9_SKL, *PCODECHAL_ENCODE_AVC_MBENC_CURBE_CM_G9_SKL;

C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(CODECHAL_ENCODE_AVC_MBENC_CURBE_CM_G9_SKL)) == 88);

typedef struct _CODECHAL_ENCODE_AVC_MFE_MBENC_CURBE_VME_BTI_G9
{
    // DW0
    struct
    {
        uint32_t    m_vmeInterPredictionSurfIndex;
    } DW0;
    // DW1
    struct
    {
        uint32_t    m_vmeInterPredictionMRSurfIndex;
    } DW1;
} CODECHAL_ENCODE_AVC_MFE_MBENC_CURBE_VME_BTI_G9;

typedef struct _CODECHAL_ENCODE_AVC_MFE_MBENC_CURBE_COMMON_BTI_G9
{
    // DW8
    struct
    {
        uint32_t    m_curbeDataSurfIndex;
    } DW8;
    // DW9
    struct
    {
        uint32_t    m_mbDataSurfIndex;
    } DW9;
    // DW10
    struct
    {
        uint32_t    m_mvDataSurfIndex;
    } DW10;
    // DW11
    struct
    {
        uint32_t    m_fwdFrmMBDataSurfIndex;
    } DW11;
    // DW12
    struct
    {
        uint32_t    m_fwdFrmMVSurfIndex;
    } DW12;
    // DW13
    struct
    {
        uint32_t    m_hmeMVPredFwdBwdSurfIndex;
    } DW13;
    // DW14
    struct
    {
        uint32_t    m_hmeDistSurfIndex;
    } DW14;
    // DW15
    struct
    {
        uint32_t    m_iDistSurfIndex;
    } DW15;
    // DW16
    struct
    {
        uint32_t    m_srcYSurfIndex;
    } DW16;
    // DW17
    struct
    {
        uint32_t    m_mbBRCLut;
    } DW17;
    // DW18
    struct
    {
        uint32_t    m_madSurfIndex;
    } DW18;
    // DW19
    struct
    {
        uint32_t    m_reservedIndex;
    } DW19;
    // DW20
    struct
    {
        uint32_t    m_staticDetectionCostTableIndex;
    } DW20;
    // DW21
    struct
    {
        uint32_t    m_currRefPicSelSurfIndex;
    } DW21;
    // DW22
    struct
    {
        uint32_t    m_mbStatsSurfIndex;
    } DW22;
    // DW23
    struct
    {
        uint32_t    m_mbSpecificDataSurfIndex;
    } DW23;
    // DW24
    struct
    {
        uint32_t    m_forceNonSkipMBmapSurface;
    } DW24;
    // DW25
    struct
    {
        uint32_t    m_sliceMapSurfIndex;
    } DW25;
    // DW26
    struct
    {
        uint32_t    m_mbQPBuffer;
    } DW26;
    // DW27
    struct
    {
        uint32_t    m_auxVmeOutSurfIndex;
    } DW27;
    // DW28
    struct
    {
        uint32_t    m_feiMVPredictorSurfIndex;
    } DW28;
    // DW29
    struct
    {
        uint32_t    m_reserved;
    } DW29;
    // DW30
    struct
    {
        uint32_t    m_reserved;
    } DW30;
    // DW31
    struct
    {
        uint32_t    m_reserved;
    } DW31;
} CODECHAL_ENCODE_AVC_MFE_MBENC_CURBE_COMMON_BTI_G9;

typedef struct _CODECHAL_ENCODE_AVC_MFE_MBENC_CURBE_G9
{
    CODECHAL_ENCODE_AVC_MFE_MBENC_CURBE_VME_BTI_G9    m_vme[CODECHAL_ENCODE_AVC_MFE_MAX_FRAMES_G9];
    CODECHAL_ENCODE_AVC_MFE_MBENC_CURBE_COMMON_BTI_G9 m_common[CODECHAL_ENCODE_AVC_MFE_MAX_FRAMES_G9];

} CODECHAL_ENCODE_AVC_MFE_MBENC_CURBE_G9, *PCODECHAL_ENCODE_AVC_MFE_MBENC_CURBE_G9;

// AVC MBEnc CURBE init data for G9 Kernel
const uint32_t CodechalEncodeAvcEncG9Skl::MBEnc_CURBE_normal_I_frame[MBENC_CURBE_SIZE_IN_DWORD_G9_SKL] =
{
    0x00000082, 0x00000000, 0x00003910, 0x00a83000, 0x00000000, 0x28300000, 0x05000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x80800000, 0x00040c24, 0x00000000, 0xffff00ff, 0x40000000, 0x00000080, 0x00003900, 0x28301000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000002,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
};

const uint32_t CodechalEncodeAvcEncG9Skl::MBEnc_CURBE_normal_I_field[MBENC_CURBE_SIZE_IN_DWORD_G9_SKL] =
{
    0x00000082, 0x00000000, 0x00003910, 0x00a830c0, 0x02000000, 0x28300000, 0x05000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x80800000, 0x00040c24, 0x00000000, 0xffff00ff, 0x40000000, 0x00000080, 0x00003900, 0x28301000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000002,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
};

const uint32_t CodechalEncodeAvcEncG9Skl::MBEnc_CURBE_normal_P_frame[MBENC_CURBE_SIZE_IN_DWORD_G9_SKL] =
{
    0x000000a3, 0x00000008, 0x00003910, 0x00ae3000, 0x30000000, 0x28300000, 0x05000000, 0x01400060,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x80010000, 0x00040c24, 0x00000000, 0xffff00ff, 0x60000000, 0x000000a1, 0x00003900, 0x28301000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x08000002,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
};

const uint32_t CodechalEncodeAvcEncG9Skl::MBEnc_CURBE_normal_P_field[MBENC_CURBE_SIZE_IN_DWORD_G9_SKL] =
{
    0x000000a3, 0x00000008, 0x00003910, 0x00ae30c0, 0x30000000, 0x28300000, 0x05000000, 0x01400060,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x80010000, 0x00040c24, 0x00000000, 0xffff00ff, 0x40000000, 0x000000a1, 0x00003900, 0x28301000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x04000002,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
};

const uint32_t CodechalEncodeAvcEncG9Skl::MBEnc_CURBE_normal_B_frame[MBENC_CURBE_SIZE_IN_DWORD_G9_SKL] =
{
    0x000000a3, 0x00200008, 0x00003910, 0x00aa7700, 0x50020000, 0x20200000, 0x05000000, 0xff400000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x01010000, 0x00040c24, 0x00000000, 0xffff00ff, 0x60000000, 0x000000a1, 0x00003900, 0x28301000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x08000002,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
};

const uint32_t CodechalEncodeAvcEncG9Skl::MBEnc_CURBE_normal_B_field[MBENC_CURBE_SIZE_IN_DWORD_G9_SKL] =
{
    0x000000a3, 0x00200008, 0x00003919, 0x00aa77c0, 0x50020000, 0x20200000, 0x05000000, 0xff400000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x01010000, 0x00040c24, 0x00000000, 0xffff00ff, 0x40000000, 0x000000a1, 0x00003900, 0x28301000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x04000002,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
};

// AVC I_DIST CURBE init data for G9 Kernel
const uint32_t CodechalEncodeAvcEncG9Skl::MBEnc_CURBE_I_frame_DIST[MBENC_CURBE_SIZE_IN_DWORD_G9_SKL] =
{
    0x00000082, 0x00200008, 0x001e3910, 0x00a83000, 0x90000000, 0x28300000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000100,
    0x80800000, 0x00000000, 0x00000800, 0xffff00ff, 0x40000000, 0x00000080, 0x00003900, 0x28300000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
};

const int32_t CodechalEncodeAvcEncG9Skl::BRC_BTCOUNTS[CODECHAL_ENCODE_BRC_IDX_NUM] = {
    CODECHAL_ENCODE_AVC_BRC_INIT_RESET_NUM_SURFACES,
    CODECHAL_ENCODE_AVC_FRAME_BRC_UPDATE_NUM_SURFACES_G9,
    CODECHAL_ENCODE_AVC_BRC_INIT_RESET_NUM_SURFACES,
    CODECHAL_ENCODE_AVC_MBENC_NUM_SURFACES_CM_G9,
    CODECHAL_ENCODE_AVC_BRC_BLOCK_COPY_NUM_SURFACES,
    CODECHAL_ENCODE_AVC_MB_BRC_UPDATE_NUM_SURFACES_G9      // MbBRCUpdate kernel starting GEN9
};

const int32_t CodechalEncodeAvcEncG9Skl::BRC_CURBE_SIZE[CODECHAL_ENCODE_BRC_IDX_NUM] = {
    (sizeof(CODECHAL_ENCODE_AVC_BRC_INIT_RESET_CURBE_G9)),
    (sizeof(CODECHAL_ENCODE_AVC_FRAME_BRC_UPDATE_CURBE_G9)),
    (sizeof(CODECHAL_ENCODE_AVC_BRC_INIT_RESET_CURBE_G9)),
    (sizeof(CODECHAL_ENCODE_AVC_MBENC_CURBE_G9)),
    0,
    (sizeof(CODECHAL_ENCODE_AVC_MB_BRC_UPDATE_CURBE_G9))     // MbBRCUpdate kernel starting GEN9
};

MOS_STATUS CodechalEncodeAvcEncG9Skl::GetKernelHeaderAndSize(void *pvBinary, EncOperation Operation, uint32_t dwKrnStateIdx, void *pvKrnHeader, uint32_t *pdwKrnSize)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(pvBinary);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pvKrnHeader);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pdwKrnSize);

    auto pKernelHeaderTable = (PCODECHAL_ENCODE_AVC_KERNEL_HEADER_G9)pvBinary;
    auto pInvalidEntry = &(pKernelHeaderTable->AVCMBEnc_Qlty_MFE) + 1;
    auto dwNextKrnOffset = *pdwKrnSize;

    PCODECHAL_KERNEL_HEADER                 pCurrKrnHeader;
    if (Operation == ENC_SCALING4X)
    {
        pCurrKrnHeader = &pKernelHeaderTable->PLY_DScale_PLY;
    }
    else if (Operation == ENC_SCALING2X)
    {
        pCurrKrnHeader = &pKernelHeaderTable->PLY_2xDScale_PLY;
    }
    else if (Operation == ENC_ME)
    {
        pCurrKrnHeader = &pKernelHeaderTable->AVC_ME_P;
    }
    else if (Operation == VDENC_ME)
    {
        pCurrKrnHeader = &pKernelHeaderTable->AVC_ME_VDENC;
    }
    else if (Operation == ENC_BRC)
    {
        pCurrKrnHeader = &pKernelHeaderTable->InitFrameBRC;
    }
    else if (Operation == ENC_MBENC)
    {
        pCurrKrnHeader = &pKernelHeaderTable->AVCMBEnc_Qlty_I;
    }
    else if (Operation == ENC_MBENC_ADV)
    {
        pCurrKrnHeader = &pKernelHeaderTable->AVCMBEnc_Adv_I;
    }
    else if (Operation == ENC_WP)
    {
        pCurrKrnHeader = &pKernelHeaderTable->AVC_WeightedPrediction;
    }
    else if (Operation == ENC_SFD)
    {
        pCurrKrnHeader = &pKernelHeaderTable->AVC_StaticFrameDetection;
    }
    else
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Unsupported ENC mode requested");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    pCurrKrnHeader += dwKrnStateIdx;
    *((PCODECHAL_KERNEL_HEADER)pvKrnHeader) = *pCurrKrnHeader;

    auto pNextKrnHeader = (pCurrKrnHeader + 1);
    if (pNextKrnHeader < pInvalidEntry)
    {
        dwNextKrnOffset = pNextKrnHeader->KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT;
    }
    *pdwKrnSize = dwNextKrnOffset - (pCurrKrnHeader->KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG9Skl::InitMfe()
{
    if (!m_mfeEnabled)
    {
        return MOS_STATUS_SUCCESS;
    }

    m_mfeLastStream   = (m_mfeEncodeParams.submitIndex == m_mfeEncodeParams.submitNumber - 1);
    m_mfeFirstStream  = (m_mfeEncodeParams.submitIndex == 0);

    // Defer allocate some MFE specific resources and init flags
    if (!m_mfeInitialized)
    {
        // MFE use surface as MbEnc curbe, which is inited by brc update kernel.
        m_mbencBrcBufferSize = sizeof(CODECHAL_ENCODE_AVC_MBENC_CURBE_G9);
        uint32_t size = MOS_ALIGN_CEIL(m_mbencBrcBufferSize,
                m_stateHeapInterface->pStateHeapInterface->GetCurbeAlignment());

        MOS_LOCK_PARAMS LockFlagsWriteOnly;
        MOS_ZeroMemory(&LockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
        LockFlagsWriteOnly.WriteOnly = 1;

        MOS_ALLOC_GFXRES_PARAMS AllocParamsForBufferLinear;
        MOS_ZeroMemory(&AllocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        AllocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
        AllocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
        AllocParamsForBufferLinear.Format = Format_Buffer;
        AllocParamsForBufferLinear.dwBytes = size;
        AllocParamsForBufferLinear.pBufName = "MbEnc BRC buffer";

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
            m_osInterface,
            &AllocParamsForBufferLinear,
            &BrcBuffers.resMbEncBrcBuffer));

        uint8_t* pData = (uint8_t*)m_osInterface->pfnLockResource(
            m_osInterface,
            &(BrcBuffers.resMbEncBrcBuffer),
            &LockFlagsWriteOnly);
        CODECHAL_ENCODE_CHK_NULL_RETURN(pData);

        MOS_ZeroMemory(pData, size);
        m_osInterface->pfnUnlockResource(m_osInterface, &BrcBuffers.resMbEncBrcBuffer);
        CODECHAL_DEBUG_TOOL(
            m_debugInterface->m_streamId = m_mfeEncodeParams.streamId;)

        // bookkeeping the orignal interfaces, which are changed during mfe mbenc kernel
        m_origHwInterface           = m_hwInterface;
        m_origOsInterface           = m_osInterface;
        m_origStateHeapInterface    = m_stateHeapInterface;

        // Whether mfe mbenc kernel is enabled or not
        MOS_USER_FEATURE_VALUE_DATA UserFeatureData;
        MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_MFE_MBENC_ENABLE_ID,
            &UserFeatureData);
        m_mfeMbEncEanbled = (UserFeatureData.i32Data) ? true : false;

        m_mfeInitialized = true;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeAvcEncG9Skl::SetCurbeAvcMfeMbEnc(PCODECHAL_ENCODE_AVC_MFE_MBENC_CURBE_PARAMS pParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;
    CODECHAL_ENCODE_AVC_MFE_MBENC_CURBE_G9 Cmd;
    PCODECHAL_ENCODE_AVC_BINDING_TABLE_MBENC    pBindingTable = pParams->pBindingTable;

    if (pParams->submitNumber > CODECHAL_ENCODE_AVC_MFE_MAX_FRAMES_G9)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("MFE submission number exceeds the threshold");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    MOS_ZeroMemory(&Cmd, sizeof(Cmd));

    for (uint32_t submitIdx = 0; submitIdx < pParams->submitNumber; submitIdx++)
    {
        uint32_t dwBindingTableBase = CODECHAL_ENCODE_AVC_MBENC_NUM_SURFACES_G9 * submitIdx;
        Cmd.m_vme[submitIdx].DW0.m_vmeInterPredictionSurfIndex       = pBindingTable->dwAvcMBEncCurrPicFrame[0] + dwBindingTableBase;
        Cmd.m_vme[submitIdx].DW1.m_vmeInterPredictionMRSurfIndex     = pBindingTable->dwAvcMBEncCurrPicFrame[1] + dwBindingTableBase;

        Cmd.m_common[submitIdx].DW8.m_curbeDataSurfIndex             = pBindingTable->dwAvcMbEncBRCCurbeData + dwBindingTableBase;
        Cmd.m_common[submitIdx].DW9.m_mbDataSurfIndex                = pBindingTable->dwAvcMBEncMfcAvcPakObj + dwBindingTableBase;
        Cmd.m_common[submitIdx].DW10.m_mvDataSurfIndex               = pBindingTable->dwAvcMBEncIndMVData + dwBindingTableBase;
        Cmd.m_common[submitIdx].DW11.m_fwdFrmMBDataSurfIndex         = pBindingTable->dwAvcMBEncBwdRefMBData + dwBindingTableBase;
        Cmd.m_common[submitIdx].DW12.m_fwdFrmMVSurfIndex             = pBindingTable->dwAvcMBEncBwdRefMVData + dwBindingTableBase;
        Cmd.m_common[submitIdx].DW13.m_hmeMVPredFwdBwdSurfIndex      = pBindingTable->dwAvcMBEncMVDataFromME + dwBindingTableBase;
        Cmd.m_common[submitIdx].DW14.m_hmeDistSurfIndex              = pBindingTable->dwAvcMBEncMEDist + dwBindingTableBase;
        Cmd.m_common[submitIdx].DW15.m_iDistSurfIndex                = pBindingTable->dwAvcMBEncBRCDist + dwBindingTableBase;
        Cmd.m_common[submitIdx].DW16.m_srcYSurfIndex                 = pBindingTable->dwAvcMBEncCurrY + dwBindingTableBase;
        Cmd.m_common[submitIdx].DW17.m_mbBRCLut                      = pBindingTable->dwAvcMBEncMbBrcConstData + dwBindingTableBase;
        Cmd.m_common[submitIdx].DW18.m_madSurfIndex                  = pBindingTable->dwAvcMBEncMADData + dwBindingTableBase;
        Cmd.m_common[submitIdx].DW19.m_reservedIndex                 = pBindingTable->dwAvcMBEncAdv + dwBindingTableBase;
        Cmd.m_common[submitIdx].DW20.m_staticDetectionCostTableIndex = pBindingTable->dwAvcMBEncStaticDetectionCostTable + dwBindingTableBase;
        Cmd.m_common[submitIdx].DW21.m_currRefPicSelSurfIndex        = pBindingTable->dwAvcMBEncRefPicSelectL0 + dwBindingTableBase;
        Cmd.m_common[submitIdx].DW22.m_mbStatsSurfIndex              = pBindingTable->dwAvcMBEncMBStats + dwBindingTableBase;
        Cmd.m_common[submitIdx].DW23.m_mbSpecificDataSurfIndex       = pBindingTable->dwAvcMBEncMbSpecificData + dwBindingTableBase;
        Cmd.m_common[submitIdx].DW24.m_forceNonSkipMBmapSurface      = pBindingTable->dwAvcMBEncMbNonSkipMap + dwBindingTableBase;
        Cmd.m_common[submitIdx].DW25.m_sliceMapSurfIndex             = pBindingTable->dwAvcMBEncSliceMapData + dwBindingTableBase;
        Cmd.m_common[submitIdx].DW26.m_mbQPBuffer                    = pBindingTable->dwAvcMBEncMbQpFrame + dwBindingTableBase;
    }
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pParams->pKernelState->m_dshRegion.AddData(
        &Cmd,
        pParams->pKernelState->dwCurbeOffset,
        sizeof(Cmd)));
    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG9Skl::UpdateMfeMbEncBindingTable(uint32_t submitIndex)
{
    auto pBindingTable = &MbEncBindingTable;

    uint32_t dwBindingTableBase = CODECHAL_ENCODE_AVC_MBENC_NUM_SURFACES_G9 * submitIndex;

    pBindingTable->dwAvcMBEncMfcAvcPakObj               = CODECHAL_ENCODE_AVC_MBENC_MFC_AVC_PAK_OBJ_G9 + dwBindingTableBase;
    pBindingTable->dwAvcMBEncIndMVData                  = CODECHAL_ENCODE_AVC_MBENC_IND_MV_DATA_G9 + dwBindingTableBase;
    pBindingTable->dwAvcMBEncBRCDist                    = CODECHAL_ENCODE_AVC_MBENC_BRC_DISTORTION_G9 + dwBindingTableBase;
    pBindingTable->dwAvcMBEncCurrY                      = CODECHAL_ENCODE_AVC_MBENC_CURR_Y_G9 + dwBindingTableBase;
    pBindingTable->dwAvcMBEncCurrUV                     = CODECHAL_ENCODE_AVC_MBENC_CURR_UV_G9 + dwBindingTableBase;
    pBindingTable->dwAvcMBEncMbSpecificData             = CODECHAL_ENCODE_AVC_MBENC_MB_SPECIFIC_DATA_G9 + dwBindingTableBase;
    pBindingTable->dwAvcMBEncRefPicSelectL0             = CODECHAL_ENCODE_AVC_MBENC_REFPICSELECT_L0_G9 + dwBindingTableBase;
    pBindingTable->dwAvcMBEncMVDataFromME               = CODECHAL_ENCODE_AVC_MBENC_MV_DATA_FROM_ME_G9 + dwBindingTableBase;
    pBindingTable->dwAvcMBEncMEDist                     = CODECHAL_ENCODE_AVC_MBENC_4xME_DISTORTION_G9 + dwBindingTableBase;
    pBindingTable->dwAvcMBEncSliceMapData               = CODECHAL_ENCODE_AVC_MBENC_SLICEMAP_DATA_G9 + dwBindingTableBase;
    pBindingTable->dwAvcMBEncBwdRefMBData               = CODECHAL_ENCODE_AVC_MBENC_FWD_MB_DATA_G9 + dwBindingTableBase;
    pBindingTable->dwAvcMBEncBwdRefMVData               = CODECHAL_ENCODE_AVC_MBENC_FWD_MV_DATA_G9 + dwBindingTableBase;
    pBindingTable->dwAvcMBEncMbBrcConstData             = CODECHAL_ENCODE_AVC_MBENC_MBBRC_CONST_DATA_G9 + dwBindingTableBase;
    pBindingTable->dwAvcMBEncMBStats                    = CODECHAL_ENCODE_AVC_MBENC_MB_STATS_G9 + dwBindingTableBase;
    pBindingTable->dwAvcMBEncMADData                    = CODECHAL_ENCODE_AVC_MBENC_MAD_DATA_G9 + dwBindingTableBase;
    pBindingTable->dwAvcMBEncMbNonSkipMap               = CODECHAL_ENCODE_AVC_MBENC_FORCE_NONSKIP_MB_MAP_G9 + dwBindingTableBase;
    pBindingTable->dwAvcMBEncAdv                        = CODECHAL_ENCODE_AVC_MBENC_ADV_WA_G9 + dwBindingTableBase;
    pBindingTable->dwAvcMbEncBRCCurbeData               = CODECHAL_ENCODE_AVC_MBENC_BRC_CURBE_DATA_G9 + dwBindingTableBase;
    pBindingTable->dwAvcMBEncFlatnessChk                = CODECHAL_ENCODE_AVC_MBENC_FLATNESS_CHECK_CM_G9 + dwBindingTableBase;
    pBindingTable->dwAvcMBEncStaticDetectionCostTable   = CODECHAL_ENCODE_AVC_MBENC_SFD_COST_TABLE_G9 + dwBindingTableBase;

    // Frame
    pBindingTable->dwAvcMBEncMbQpFrame          = CODECHAL_ENCODE_AVC_MBENC_MBQP_G9 + dwBindingTableBase;
    pBindingTable->dwAvcMBEncCurrPicFrame[0]    = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_CURR_PIC_IDX_0_G9 + dwBindingTableBase;
    pBindingTable->dwAvcMBEncFwdPicFrame[0]     = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX0_G9 + dwBindingTableBase;
    pBindingTable->dwAvcMBEncBwdPicFrame[0]     = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_BWD_PIC_IDX0_0_G9 + dwBindingTableBase;
    pBindingTable->dwAvcMBEncFwdPicFrame[1]     = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX1_G9 + dwBindingTableBase;
    pBindingTable->dwAvcMBEncBwdPicFrame[1]     = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_BWD_PIC_IDX1_0_G9 + dwBindingTableBase;
    pBindingTable->dwAvcMBEncFwdPicFrame[2]     = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX2_G9 + dwBindingTableBase;
    pBindingTable->dwAvcMBEncFwdPicFrame[3]     = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX3_G9 + dwBindingTableBase;
    pBindingTable->dwAvcMBEncFwdPicFrame[4]     = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX4_G9 + dwBindingTableBase;
    pBindingTable->dwAvcMBEncFwdPicFrame[5]     = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX5_G9 + dwBindingTableBase;
    pBindingTable->dwAvcMBEncFwdPicFrame[6]     = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX6_G9 + dwBindingTableBase;
    pBindingTable->dwAvcMBEncFwdPicFrame[7]     = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX7_G9 + dwBindingTableBase;
    pBindingTable->dwAvcMBEncCurrPicFrame[1]    = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_CURR_PIC_IDX_1_G9 + dwBindingTableBase;
    pBindingTable->dwAvcMBEncBwdPicFrame[2]     = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_BWD_PIC_IDX0_1_G9 + dwBindingTableBase;
    pBindingTable->dwAvcMBEncBwdPicFrame[3]     = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_BWD_PIC_IDX1_1_G9 + dwBindingTableBase;

    // Field
    pBindingTable->dwAvcMBEncMbQpField          = CODECHAL_ENCODE_AVC_MBENC_MBQP_G9 + dwBindingTableBase;
    pBindingTable->dwAvcMBEncFieldCurrPic[0]    = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_CURR_PIC_IDX_0_G9 + dwBindingTableBase;
    pBindingTable->dwAvcMBEncFwdPicTopField[0]  = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX0_G9 + dwBindingTableBase;
    pBindingTable->dwAvcMBEncBwdPicTopField[0]  = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_BWD_PIC_IDX0_0_G9 + dwBindingTableBase;
    pBindingTable->dwAvcMBEncFwdPicBotField[0]  = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX0_G9 + dwBindingTableBase;
    pBindingTable->dwAvcMBEncBwdPicBotField[0]  = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_BWD_PIC_IDX0_0_G9 + dwBindingTableBase;
    pBindingTable->dwAvcMBEncFwdPicTopField[1]  = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX1_G9 + dwBindingTableBase;
    pBindingTable->dwAvcMBEncBwdPicTopField[1]  = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_BWD_PIC_IDX1_0_G9 + dwBindingTableBase;
    pBindingTable->dwAvcMBEncFwdPicBotField[1]  = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX1_G9 + dwBindingTableBase;
    pBindingTable->dwAvcMBEncBwdPicBotField[1]  = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_BWD_PIC_IDX1_0_G9 + dwBindingTableBase;
    pBindingTable->dwAvcMBEncFwdPicTopField[2]  = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX2_G9 + dwBindingTableBase;
    pBindingTable->dwAvcMBEncFwdPicBotField[2]  = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX2_G9 + dwBindingTableBase;
    pBindingTable->dwAvcMBEncFwdPicTopField[3]  = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX3_G9 + dwBindingTableBase;
    pBindingTable->dwAvcMBEncFwdPicBotField[3]  = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX3_G9 + dwBindingTableBase;
    pBindingTable->dwAvcMBEncFwdPicTopField[4]  = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX4_G9 + dwBindingTableBase;
    pBindingTable->dwAvcMBEncFwdPicBotField[4]  = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX4_G9 + dwBindingTableBase;
    pBindingTable->dwAvcMBEncFwdPicTopField[5]  = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX5_G9 + dwBindingTableBase;
    pBindingTable->dwAvcMBEncFwdPicBotField[5]  = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX5_G9 + dwBindingTableBase;
    pBindingTable->dwAvcMBEncFwdPicTopField[6]  = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX6_G9 + dwBindingTableBase;
    pBindingTable->dwAvcMBEncFwdPicBotField[6]  = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX6_G9 + dwBindingTableBase;
    pBindingTable->dwAvcMBEncFwdPicTopField[7]  = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX7_G9 + dwBindingTableBase;
    pBindingTable->dwAvcMBEncFwdPicBotField[7]  = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX7_G9 + dwBindingTableBase;
    pBindingTable->dwAvcMBEncFieldCurrPic[1]    = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_CURR_PIC_IDX_1_G9 + dwBindingTableBase;
    pBindingTable->dwAvcMBEncBwdPicTopField[2]  = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_BWD_PIC_IDX0_1_G9 + dwBindingTableBase;
    pBindingTable->dwAvcMBEncBwdPicBotField[2]  = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_BWD_PIC_IDX0_1_G9 + dwBindingTableBase;
    pBindingTable->dwAvcMBEncBwdPicTopField[3]  = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_BWD_PIC_IDX1_1_G9 + dwBindingTableBase;
    pBindingTable->dwAvcMBEncBwdPicBotField[3]  = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_BWD_PIC_IDX1_1_G9 + dwBindingTableBase;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeAvcEncG9Skl::SetCurbeAvcMbEnc(PCODECHAL_ENCODE_AVC_MBENC_CURBE_PARAMS pParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams->pPicParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams->pSeqParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams->pSlcParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams->pdwBlockBasedSkipEn);

    auto pPicParams = pParams->pPicParams;
    auto pSeqParams = pParams->pSeqParams;
    auto pSlcParams = pParams->pSlcParams;
    CODECHAL_ENCODE_ASSERT(pSeqParams->TargetUsage < NUM_TARGET_USAGE_MODES);

    uint8_t ucMeMethod =
        (m_pictureCodingType == B_TYPE) ? m_bMeMethodGeneric[pSeqParams->TargetUsage] : m_meMethodGeneric[pSeqParams->TargetUsage];
    // set SliceQP to MAX_SLICE_QP for MbEnc Adv kernel, we can use it to verify whether QP is changed or not
    uint8_t SliceQP = (pParams->bUseMbEncAdvKernel && pParams->bBrcEnabled) ? CODECHAL_ENCODE_AVC_MAX_SLICE_QP : pPicParams->pic_init_qp_minus26 + 26 + pSlcParams->slice_qp_delta;
    bool bFramePicture = CodecHal_PictureIsFrame(pPicParams->CurrOriginalPic);
    bool bTopField = CodecHal_PictureIsTopField(pPicParams->CurrOriginalPic);
    bool bBottomField = CodecHal_PictureIsBottomField(pPicParams->CurrOriginalPic);

    CODECHAL_ENCODE_AVC_MBENC_CURBE_G9  Cmd;
    if (pParams->bMbEncIFrameDistEnabled)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
            &Cmd,
            sizeof(CODECHAL_ENCODE_AVC_MBENC_CURBE_G9),
            MBEnc_CURBE_I_frame_DIST,
            sizeof(CODECHAL_ENCODE_AVC_MBENC_CURBE_G9)));
    }
    else
    {
        switch (m_pictureCodingType)
        {
        case I_TYPE:
            if (bFramePicture)
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
                    &Cmd,
                    sizeof(CODECHAL_ENCODE_AVC_MBENC_CURBE_G9),
                    MBEnc_CURBE_normal_I_frame,
                    sizeof(CODECHAL_ENCODE_AVC_MBENC_CURBE_G9)));
            }
            else
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
                    &Cmd,
                    sizeof(CODECHAL_ENCODE_AVC_MBENC_CURBE_G9),
                    MBEnc_CURBE_normal_I_field,
                    sizeof(CODECHAL_ENCODE_AVC_MBENC_CURBE_G9)));
            }
            break;

        case P_TYPE:
            if (bFramePicture)
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
                    &Cmd,
                    sizeof(CODECHAL_ENCODE_AVC_MBENC_CURBE_G9),
                    MBEnc_CURBE_normal_P_frame,
                    sizeof(CODECHAL_ENCODE_AVC_MBENC_CURBE_G9)));
            }
            else
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
                    &Cmd,
                    sizeof(CODECHAL_ENCODE_AVC_MBENC_CURBE_G9),
                    MBEnc_CURBE_normal_P_field,
                    sizeof(CODECHAL_ENCODE_AVC_MBENC_CURBE_G9)));
            }
            break;

        case B_TYPE:
            if (bFramePicture)
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
                    &Cmd,
                    sizeof(CODECHAL_ENCODE_AVC_MBENC_CURBE_G9),
                    MBEnc_CURBE_normal_B_frame,
                    sizeof(CODECHAL_ENCODE_AVC_MBENC_CURBE_G9)));
            }
            else
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
                    &Cmd,
                    sizeof(CODECHAL_ENCODE_AVC_MBENC_CURBE_G9),
                    MBEnc_CURBE_normal_B_field,
                    sizeof(CODECHAL_ENCODE_AVC_MBENC_CURBE_G9)));
            }
            break;

        default:
            CODECHAL_ENCODE_ASSERTMESSAGE("Invalid picture coding type.");
            eStatus = MOS_STATUS_UNKNOWN;
            return eStatus;
        }
    }

    // r1
    Cmd.common.DW0.AdaptiveEn =
        Cmd.common.DW37.AdaptiveEn = EnableAdaptiveSearch[pSeqParams->TargetUsage];
    Cmd.common.DW0.T8x8FlagForInterEn =
        Cmd.common.DW37.T8x8FlagForInterEn = pPicParams->transform_8x8_mode_flag;
    Cmd.common.DW2.LenSP = MaxLenSP[pSeqParams->TargetUsage];
    Cmd.common.DW38.LenSP = 0; // MBZ
    Cmd.common.DW3.SrcAccess =
        Cmd.common.DW3.RefAccess = bFramePicture ? 0 : 1;
    if (m_pictureCodingType != I_TYPE && bFTQEnable)
    {
        if (pParams->pAvcQCParams && pParams->pAvcQCParams->FTQOverride)
        {
            Cmd.common.DW3.FTEnable = pParams->pAvcQCParams->FTQEnable;
        }
        else
        {
            if (m_pictureCodingType == P_TYPE)
            {
                Cmd.common.DW3.FTEnable = FTQBasedSkip[pSeqParams->TargetUsage] & 0x01;
            }
            else // B_TYPE
            {
                Cmd.common.DW3.FTEnable = (FTQBasedSkip[pSeqParams->TargetUsage] >> 1) & 0x01;
            }
        }
    }
    else
    {
        Cmd.common.DW3.FTEnable = 0;
    }
    if (pPicParams->UserFlags.bDisableSubMBPartition)
    {
        Cmd.common.DW3.SubMbPartMask = CODECHAL_ENCODE_AVC_DISABLE_4X4_SUB_MB_PARTITION | CODECHAL_ENCODE_AVC_DISABLE_4X8_SUB_MB_PARTITION | CODECHAL_ENCODE_AVC_DISABLE_8X4_SUB_MB_PARTITION;
    }

    if (pPicParams->bEnableSubMbPartMask)
    {
        Cmd.common.DW3.SubMbPartMask |= pPicParams->SubMbPartMask;
    }

    if (pPicParams->bEnableSubPelMode)
    {
        Cmd.common.DW3.SubPelMode = pPicParams->SubPelMode;
    }

    Cmd.common.DW2.PicWidth = pParams->wPicWidthInMb;
    Cmd.common.DW4.PicHeightMinus1 = pParams->wFieldFrameHeightInMb - 1;
    Cmd.common.DW4.EnableFBRBypass = bFBRBypassEnable;
    Cmd.common.DW4.EnableIntraCostScalingForStaticFrame = pParams->bStaticFrameDetectionEnabled;
    Cmd.common.DW4.FieldParityFlag = bBottomField;
    Cmd.common.DW4.bCurFldIDR = !bFramePicture && (pPicParams->bIdrPic || m_firstFieldIdrPic);
    Cmd.common.DW4.ConstrainedIntraPredFlag = pPicParams->constrained_intra_pred_flag;
    Cmd.common.DW4.HMEEnable = m_hmeEnabled;
    Cmd.common.DW4.PictureType = m_pictureCodingType - 1;
    Cmd.common.DW4.UseActualRefQPValue = m_hmeEnabled && (MRDisableQPCheck[pSeqParams->TargetUsage] == 0);
    Cmd.common.DW5.SliceMbHeight = pParams->usSliceHeight;
    Cmd.common.DW7.IntraPartMask = pPicParams->transform_8x8_mode_flag ? 0 : 0x2;  // Disable 8x8 if flag is not set
    Cmd.common.DW7.SrcFieldPolarity = bBottomField;

    // r2
    if (pParams->bMbEncIFrameDistEnabled)
    {
        Cmd.common.DW6.BatchBufferEnd = 0;
    }
    else
    {
        uint8_t ucTableIdx = m_pictureCodingType - 1;
        eStatus = MOS_SecureMemcpy(&(Cmd.common.DW8), 8 * sizeof(uint32_t), ModeMvCost_Cm[ucTableIdx][SliceQP], 8 * sizeof(uint32_t));
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
            return eStatus;
        }

        if (m_pictureCodingType == I_TYPE && bOldModeCostEnable)
        {
            // Old intra mode cost needs to be used if bOldModeCostEnable is 1
            Cmd.common.DW8.Value = OldIntraModeCost_Cm_Common[SliceQP];
        }
        else if (m_skipBiasAdjustmentEnable)
        {
            // Load different MvCost for P picture when SkipBiasAdjustment is enabled
            // No need to check for P picture as the flag is only enabled for P picture
            Cmd.common.DW11.Value = MvCost_PSkipAdjustment_Cm_Common[SliceQP];
        }
    }

    if (pParams->pAvcQCParams && pParams->pAvcQCParams->FTQSkipThresholdLUTInput)
    {
        Cmd.common.DW14.SICFwdTransCoeffThreshold_0 = pParams->pAvcQCParams->FTQSkipThresholdLUT[SliceQP];
        Cmd.common.DW14.SICFwdTransCoeffThreshold_1 = pParams->pAvcQCParams->FTQSkipThresholdLUT[SliceQP];
        Cmd.common.DW14.SICFwdTransCoeffThreshold_2 = pParams->pAvcQCParams->FTQSkipThresholdLUT[SliceQP];
        Cmd.common.DW15.SICFwdTransCoeffThreshold_3 = pParams->pAvcQCParams->FTQSkipThresholdLUT[SliceQP];
        Cmd.common.DW15.SICFwdTransCoeffThreshold_4 = pParams->pAvcQCParams->FTQSkipThresholdLUT[SliceQP];
        Cmd.common.DW15.SICFwdTransCoeffThreshold_5 = pParams->pAvcQCParams->FTQSkipThresholdLUT[SliceQP];
        Cmd.common.DW15.SICFwdTransCoeffThreshold_6 = pParams->pAvcQCParams->FTQSkipThresholdLUT[SliceQP];
    }

    // r3 & r4
    if (pParams->bMbEncIFrameDistEnabled)
    {
        Cmd.common.DW31.IntraComputeType = 1;
    }
    else
    {
        uint8_t ucTableIdx = (m_pictureCodingType == B_TYPE) ? 1 : 0;
        eStatus = MOS_SecureMemcpy(&(Cmd.common.DW16), 16 * sizeof(uint32_t), CodechalEncoderState::m_encodeSearchPath[ucTableIdx][ucMeMethod], 16 * sizeof(uint32_t));
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
            return eStatus;
        }
    }

    // r5
    if (m_pictureCodingType != I_TYPE && pParams->pAvcQCParams && pParams->pAvcQCParams->NonFTQSkipThresholdLUTInput)
    {
        Cmd.common.DW32.SkipVal = (uint16_t)CalcSkipVal(Cmd.common.DW3.BlockBasedSkipEnable, pPicParams->transform_8x8_mode_flag,
            pParams->pAvcQCParams->NonFTQSkipThresholdLUT[SliceQP]);

    }
    else
    {
        if (m_pictureCodingType == P_TYPE)
        {
            Cmd.common.DW32.SkipVal = SkipVal_P_Common
                [Cmd.common.DW3.BlockBasedSkipEnable]
            [pPicParams->transform_8x8_mode_flag]
            [SliceQP];
        }
        else if (m_pictureCodingType == B_TYPE)
        {
            Cmd.common.DW32.SkipVal = SkipVal_B_Common
                [Cmd.common.DW3.BlockBasedSkipEnable]
            [pPicParams->transform_8x8_mode_flag]
            [SliceQP];
        }
    }

    Cmd.common.DW13.QpPrimeY = SliceQP;
    // QpPrimeCb and QpPrimeCr are not used by Kernel. Following settings are for CModel matching.
    Cmd.common.DW13.QpPrimeCb = SliceQP;
    Cmd.common.DW13.QpPrimeCr = SliceQP;
    Cmd.common.DW13.TargetSizeInWord = 0xff; // hardcoded for BRC disabled

    if (bMultiPredEnable && (m_pictureCodingType != I_TYPE))
    {
        switch (MultiPred[pSeqParams->TargetUsage])
        {
        case 0: // Disable multipred for both P & B picture types
            Cmd.common.DW32.MultiPredL0Disable = CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;
            Cmd.common.DW32.MultiPredL1Disable = CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;
            break;

        case 1: // Enable multipred for P pictures only
            Cmd.common.DW32.MultiPredL0Disable = (m_pictureCodingType == P_TYPE) ? CODECHAL_ENCODE_AVC_MULTIPRED_ENABLE : CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;
            Cmd.common.DW32.MultiPredL1Disable = CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;
            break;

        case 2: // Enable multipred for B pictures only
            Cmd.common.DW32.MultiPredL0Disable = (m_pictureCodingType == B_TYPE) ?
            CODECHAL_ENCODE_AVC_MULTIPRED_ENABLE : CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;
            Cmd.common.DW32.MultiPredL1Disable = (m_pictureCodingType == B_TYPE) ?
            CODECHAL_ENCODE_AVC_MULTIPRED_ENABLE : CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;
            break;

        case 3: // Enable multipred for both P & B picture types
            Cmd.common.DW32.MultiPredL0Disable = CODECHAL_ENCODE_AVC_MULTIPRED_ENABLE;
            Cmd.common.DW32.MultiPredL1Disable = (m_pictureCodingType == B_TYPE) ?
            CODECHAL_ENCODE_AVC_MULTIPRED_ENABLE : CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;
            break;
        }
    }
    else
    {
        Cmd.common.DW32.MultiPredL0Disable = CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;
        Cmd.common.DW32.MultiPredL1Disable = CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;
    }

    if (!bFramePicture)
    {
        if (m_pictureCodingType != I_TYPE)
        {
            Cmd.common.DW34.List0RefID0FieldParity = CodecHalAvcEncode_GetFieldParity(pSlcParams, LIST_0, CODECHAL_ENCODE_REF_ID_0);
            Cmd.common.DW34.List0RefID1FieldParity = CodecHalAvcEncode_GetFieldParity(pSlcParams, LIST_0, CODECHAL_ENCODE_REF_ID_1);
            Cmd.common.DW34.List0RefID2FieldParity = CodecHalAvcEncode_GetFieldParity(pSlcParams, LIST_0, CODECHAL_ENCODE_REF_ID_2);
            Cmd.common.DW34.List0RefID3FieldParity = CodecHalAvcEncode_GetFieldParity(pSlcParams, LIST_0, CODECHAL_ENCODE_REF_ID_3);
            Cmd.common.DW34.List0RefID4FieldParity = CodecHalAvcEncode_GetFieldParity(pSlcParams, LIST_0, CODECHAL_ENCODE_REF_ID_4);
            Cmd.common.DW34.List0RefID5FieldParity = CodecHalAvcEncode_GetFieldParity(pSlcParams, LIST_0, CODECHAL_ENCODE_REF_ID_5);
            Cmd.common.DW34.List0RefID6FieldParity = CodecHalAvcEncode_GetFieldParity(pSlcParams, LIST_0, CODECHAL_ENCODE_REF_ID_6);
            Cmd.common.DW34.List0RefID7FieldParity = CodecHalAvcEncode_GetFieldParity(pSlcParams, LIST_0, CODECHAL_ENCODE_REF_ID_7);
        }
        if (m_pictureCodingType == B_TYPE)
        {
            Cmd.common.DW34.List1RefID0FieldParity = CodecHalAvcEncode_GetFieldParity(pSlcParams, LIST_1, CODECHAL_ENCODE_REF_ID_0);
            Cmd.common.DW34.List1RefID1FieldParity = CodecHalAvcEncode_GetFieldParity(pSlcParams, LIST_1, CODECHAL_ENCODE_REF_ID_1);
        }
    }
    Cmd.common.DW34.RemoveIntraRefreshOverlap = pPicParams->bDisableRollingIntraRefreshOverlap;
    if (m_adaptiveTransformDecisionEnabled)
    {
        if (m_pictureCodingType != I_TYPE)
        {
            Cmd.common.DW34.EnableAdaptiveTxDecision = true;
        }

        Cmd.common.DW58.TxDecisonThreshold = CODECHAL_ENCODE_AVC_ADAPTIVE_TX_DECISION_THRESHOLD_G9;
    }

    if (m_adaptiveTransformDecisionEnabled || m_flatnessCheckEnabled)
    {
        Cmd.common.DW58.MBTextureThreshold = CODECHAL_ENCODE_AVC_MB_TEXTURE_THRESHOLD_G9;
    }

    if (m_pictureCodingType == B_TYPE)
    {
        Cmd.common.DW34.List1RefID0FrameFieldFlag = GetRefPicFieldFlag(pParams, LIST_1, CODECHAL_ENCODE_REF_ID_0);
        Cmd.common.DW34.List1RefID1FrameFieldFlag = GetRefPicFieldFlag(pParams, LIST_1, CODECHAL_ENCODE_REF_ID_1);
        Cmd.common.DW34.bDirectMode = pSlcParams->direct_spatial_mv_pred_flag;
    }
    Cmd.common.DW34.bOriginalBff = bFramePicture ? 0 :
        ((m_firstField && (bBottomField)) || (!m_firstField && (!bBottomField)));
    Cmd.common.DW34.EnableMBFlatnessChkOptimization = m_flatnessCheckEnabled;
    Cmd.common.DW34.ROIEnableFlag = pParams->bRoiEnabled;
    Cmd.common.DW34.MADEnableFlag                   = m_madEnabled;
    Cmd.common.DW34.MBBrcEnable = bMbBrcEnabled || bMbQpDataEnabled;
    Cmd.common.DW34.ArbitraryNumMbsPerSlice = m_arbitraryNumMbsInSlice;
    Cmd.common.DW34.ForceNonSkipMbEnable = pParams->bMbDisableSkipMapEnabled;
    if (pParams->pAvcQCParams && !Cmd.common.DW34.ForceNonSkipMbEnable) // ignore DisableEncSkipCheck if Mb Disable Skip Map is available
    {
        Cmd.common.DW34.DisableEncSkipCheck = pParams->pAvcQCParams->skipCheckDisable;
    }
    Cmd.common.DW36.CheckAllFractionalEnable = bCAFEnable;
    Cmd.common.DW38.RefThreshold = m_refThreshold;
    Cmd.common.DW39.HMERefWindowsCombThreshold = (m_pictureCodingType == B_TYPE) ?
        HMEBCombineLen[pSeqParams->TargetUsage] : HMECombineLen[pSeqParams->TargetUsage];

    // Default:2 used for MBBRC (MB QP Surface width and height are 4x downscaled picture in MB unit * 4  bytes)
    // 0 used for MBQP data surface (MB QP Surface width and height are same as the input picture size in MB unit * 1bytes)
    // starting GEN9, BRC use split kernel, MB QP surface is same size as input picture
    Cmd.common.DW47.MbQpReadFactor = (bMbBrcEnabled || bMbQpDataEnabled) ? 0 : 2;

    // Those fields are not really used for I_dist kernel,
    // but set them to 0 to get bit-exact match with kernel prototype
    if (pParams->bMbEncIFrameDistEnabled)
    {
        Cmd.common.DW13.QpPrimeY = 0;
        Cmd.common.DW13.QpPrimeCb = 0;
        Cmd.common.DW13.QpPrimeCr = 0;
        Cmd.common.DW33.Intra16x16NonDCPredPenalty = 0;
        Cmd.common.DW33.Intra4x4NonDCPredPenalty = 0;
        Cmd.common.DW33.Intra8x8NonDCPredPenalty = 0;
    }

    //r6
    if (Cmd.common.DW4.UseActualRefQPValue)
    {
        Cmd.common.DW44.ActualQPValueForRefID0List0 = AVCGetQPValueFromRefList(pParams, LIST_0, CODECHAL_ENCODE_REF_ID_0);
        Cmd.common.DW44.ActualQPValueForRefID1List0 = AVCGetQPValueFromRefList(pParams, LIST_0, CODECHAL_ENCODE_REF_ID_1);
        Cmd.common.DW44.ActualQPValueForRefID2List0 = AVCGetQPValueFromRefList(pParams, LIST_0, CODECHAL_ENCODE_REF_ID_2);
        Cmd.common.DW44.ActualQPValueForRefID3List0 = AVCGetQPValueFromRefList(pParams, LIST_0, CODECHAL_ENCODE_REF_ID_3);
        Cmd.common.DW45.ActualQPValueForRefID4List0 = AVCGetQPValueFromRefList(pParams, LIST_0, CODECHAL_ENCODE_REF_ID_4);
        Cmd.common.DW45.ActualQPValueForRefID5List0 = AVCGetQPValueFromRefList(pParams, LIST_0, CODECHAL_ENCODE_REF_ID_5);
        Cmd.common.DW45.ActualQPValueForRefID6List0 = AVCGetQPValueFromRefList(pParams, LIST_0, CODECHAL_ENCODE_REF_ID_6);
        Cmd.common.DW45.ActualQPValueForRefID7List0 = AVCGetQPValueFromRefList(pParams, LIST_0, CODECHAL_ENCODE_REF_ID_7);
        Cmd.common.DW46.ActualQPValueForRefID0List1 = AVCGetQPValueFromRefList(pParams, LIST_1, CODECHAL_ENCODE_REF_ID_0);
        Cmd.common.DW46.ActualQPValueForRefID1List1 = AVCGetQPValueFromRefList(pParams, LIST_1, CODECHAL_ENCODE_REF_ID_1);
    }

    uint8_t ucTableIdx = m_pictureCodingType - 1;
    Cmd.common.DW46.RefCost = RefCost_MultiRefQp[ucTableIdx][SliceQP];

    // Picture Coding Type dependent parameters
    if (m_pictureCodingType == I_TYPE)
    {
        Cmd.common.DW0.SkipModeEn = 0;
        Cmd.common.DW37.SkipModeEn = 0;
        Cmd.common.DW36.HMECombineOverlap = 0;
        Cmd.common.DW47.IntraCostSF = 16; // This is not used but recommended to set this to 16 by Kernel team
        Cmd.common.DW34.EnableDirectBiasAdjustment = 0;
        Cmd.common.DW34.EnableGlobalMotionBiasAdjustment = 0;
    }
    else if (m_pictureCodingType == P_TYPE)
    {
        Cmd.common.DW1.MaxNumMVs = GetMaxMvsPer2Mb(pSeqParams->Level) / 2;
        Cmd.common.DW3.BMEDisableFBR = 1;
        Cmd.common.DW5.RefWidth = SearchX[pSeqParams->TargetUsage];
        Cmd.common.DW5.RefHeight = SearchY[pSeqParams->TargetUsage];
        Cmd.common.DW7.NonSkipZMvAdded = 1;
        Cmd.common.DW7.NonSkipModeAdded = 1;
        Cmd.common.DW7.SkipCenterMask = 1;
        Cmd.common.DW47.IntraCostSF =
            bAdaptiveIntraScalingEnable ?
            AdaptiveIntraScalingFactor_Cm_Common[SliceQP] :
            IntraScalingFactor_Cm_Common[SliceQP];
        Cmd.common.DW47.MaxVmvR = (bFramePicture) ? CodecHalAvcEncode_GetMaxMvLen(pSeqParams->Level) * 4 : (CodecHalAvcEncode_GetMaxMvLen(pSeqParams->Level) >> 1) * 4;
        Cmd.common.DW36.HMECombineOverlap = 1;
        Cmd.common.DW36.NumRefIdxL0MinusOne = bMultiPredEnable ? pSlcParams->num_ref_idx_l0_active_minus1 : 0;
        Cmd.common.DW39.RefWidth = SearchX[pSeqParams->TargetUsage];
        Cmd.common.DW39.RefHeight = SearchY[pSeqParams->TargetUsage];
        Cmd.common.DW34.EnableDirectBiasAdjustment = 0;
        if (pParams->pAvcQCParams)
        {
            Cmd.common.DW34.EnableGlobalMotionBiasAdjustment = pParams->pAvcQCParams->globalMotionBiasAdjustmentEnable;
            if (Cmd.common.DW34.EnableGlobalMotionBiasAdjustment)
            {
                Cmd.common.DW59.HMEMVCostScalingFactor = pParams->pAvcQCParams->HMEMVCostScalingFactor;
            }
        }
    }
    else
    {
        // B_TYPE
        Cmd.common.DW1.MaxNumMVs = GetMaxMvsPer2Mb(pSeqParams->Level) / 2;
        Cmd.common.DW1.BiWeight = m_biWeight;
        Cmd.common.DW3.SearchCtrl = 7;
        Cmd.common.DW3.SkipType = 1;
        Cmd.common.DW5.RefWidth = BSearchX[pSeqParams->TargetUsage];
        Cmd.common.DW5.RefHeight = BSearchY[pSeqParams->TargetUsage];
        Cmd.common.DW7.SkipCenterMask = 0xFF;
        Cmd.common.DW47.IntraCostSF =
            bAdaptiveIntraScalingEnable ?
            AdaptiveIntraScalingFactor_Cm_Common[SliceQP] :
            IntraScalingFactor_Cm_Common[SliceQP];
        Cmd.common.DW47.MaxVmvR = (bFramePicture) ? CodecHalAvcEncode_GetMaxMvLen(pSeqParams->Level) * 4 : (CodecHalAvcEncode_GetMaxMvLen(pSeqParams->Level) >> 1) * 4;
        Cmd.common.DW36.HMECombineOverlap = 1;
        // Checking if the forward frame (List 1 index 0) is a short term reference
        {
            CODEC_PICTURE CodecHalPic = pParams->pSlcParams->RefPicList[LIST_1][0];
            if (CodecHalPic.PicFlags != PICTURE_INVALID &&
                CodecHalPic.FrameIdx != CODECHAL_ENCODE_AVC_INVALID_PIC_ID &&
                pParams->pPicIdx[CodecHalPic.FrameIdx].bValid)
            {
                // Although its name is FWD, it actually means the future frame or the backward reference frame
                Cmd.common.DW36.IsFwdFrameShortTermRef = CodecHal_PictureIsShortTermRef(pParams->pPicParams->RefFrameList[CodecHalPic.FrameIdx]);
            }
            else
            {
                CODECHAL_ENCODE_ASSERTMESSAGE("Invalid backward reference frame.");
                eStatus = MOS_STATUS_INVALID_PARAMETER;
                return eStatus;
            }
        }
        Cmd.common.DW36.NumRefIdxL0MinusOne = bMultiPredEnable ? pSlcParams->num_ref_idx_l0_active_minus1 : 0;
        Cmd.common.DW36.NumRefIdxL1MinusOne = bMultiPredEnable ? pSlcParams->num_ref_idx_l1_active_minus1 : 0;
        Cmd.common.DW39.RefWidth = BSearchX[pSeqParams->TargetUsage];
        Cmd.common.DW39.RefHeight = BSearchY[pSeqParams->TargetUsage];
        Cmd.common.DW40.DistScaleFactorRefID0List0 = m_distScaleFactorList0[0];
        Cmd.common.DW40.DistScaleFactorRefID1List0 = m_distScaleFactorList0[1];
        Cmd.common.DW41.DistScaleFactorRefID2List0 = m_distScaleFactorList0[2];
        Cmd.common.DW41.DistScaleFactorRefID3List0 = m_distScaleFactorList0[3];
        Cmd.common.DW42.DistScaleFactorRefID4List0 = m_distScaleFactorList0[4];
        Cmd.common.DW42.DistScaleFactorRefID5List0 = m_distScaleFactorList0[5];
        Cmd.common.DW43.DistScaleFactorRefID6List0 = m_distScaleFactorList0[6];
        Cmd.common.DW43.DistScaleFactorRefID7List0 = m_distScaleFactorList0[7];
        if (pParams->pAvcQCParams)
        {
            Cmd.common.DW34.EnableDirectBiasAdjustment = pParams->pAvcQCParams->directBiasAdjustmentEnable;
            if (Cmd.common.DW34.EnableDirectBiasAdjustment)
            {
                Cmd.common.DW7.NonSkipModeAdded = 1;
                Cmd.common.DW7.NonSkipZMvAdded = 1;
            }

            Cmd.common.DW34.EnableGlobalMotionBiasAdjustment = pParams->pAvcQCParams->globalMotionBiasAdjustmentEnable;
            if (Cmd.common.DW34.EnableGlobalMotionBiasAdjustment)
            {
                Cmd.common.DW59.HMEMVCostScalingFactor = pParams->pAvcQCParams->HMEMVCostScalingFactor;
            }
        }
        {
            CODEC_PICTURE    RefPic;
            RefPic = pSlcParams->RefPicList[LIST_1][0];

            Cmd.common.DW64.L1ListRef0PictureCodingType = m_refList[m_picIdx[RefPic.FrameIdx].ucPicIdx]->ucAvcPictureCodingType;
            if(bFramePicture && ((Cmd.common.DW64.L1ListRef0PictureCodingType == CODEC_AVC_PIC_CODING_TYPE_TFF_FIELD) || (Cmd.common.DW64.L1ListRef0PictureCodingType == CODEC_AVC_PIC_CODING_TYPE_BFF_FIELD)))
            {
                uint16_t wFieldHeightInMb = (pParams->wFieldFrameHeightInMb + 1) >> 1;
                Cmd.common.DW66.BottomFieldOffsetL1ListRef0MV     = MOS_ALIGN_CEIL(wFieldHeightInMb * pParams->wPicWidthInMb * (32 * 4), 0x1000);
                Cmd.common.DW67.BottomFieldOffsetL1ListRef0MBCode = wFieldHeightInMb * pParams->wPicWidthInMb * 64;
            }
        }
    }

    *pParams->pdwBlockBasedSkipEn = Cmd.common.DW3.BlockBasedSkipEnable;

    if (pPicParams->EnableRollingIntraRefresh)
    {
        Cmd.common.DW34.IntraRefreshEn = pPicParams->EnableRollingIntraRefresh;

        /* Multiple predictor should be completely disabled for the RollingI feature. This does not lead to much quality drop for P frames especially for TU as 1 */
        Cmd.common.DW32.MultiPredL0Disable = CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;

        /* Pass the same IntraRefreshUnit to the kernel w/o the adjustment by -1, so as to have an overlap of one MB row or column of Intra macroblocks
        across one P frame to another P frame, as needed by the RollingI algo */
        Cmd.common.DW48.IntraRefreshMBNum = pPicParams->IntraRefreshMBNum; /* MB row or column number */
        Cmd.common.DW48.IntraRefreshUnitInMBMinus1 = pPicParams->IntraRefreshUnitinMB;
        Cmd.common.DW48.IntraRefreshQPDelta = pPicParams->IntraRefreshQPDelta;
    }
    else
    {
        Cmd.common.DW34.IntraRefreshEn = 0;
    }

    Cmd.common.DW34.EnablePerMBStaticCheck = pParams->bStaticFrameDetectionEnabled;
    Cmd.common.DW34.EnableAdaptiveSearchWindowSize = pParams->bApdatvieSearchWindowSizeEnabled;

    if (true == pParams->bRoiEnabled)
    {
        Cmd.common.DW49.ROI1_X_left = pPicParams->ROI[0].Left;
        Cmd.common.DW49.ROI1_Y_top = pPicParams->ROI[0].Top;
        Cmd.common.DW50.ROI1_X_right = pPicParams->ROI[0].Right;
        Cmd.common.DW50.ROI1_Y_bottom = pPicParams->ROI[0].Bottom;

        Cmd.common.DW51.ROI2_X_left = pPicParams->ROI[1].Left;
        Cmd.common.DW51.ROI2_Y_top = pPicParams->ROI[1].Top;
        Cmd.common.DW52.ROI2_X_right = pPicParams->ROI[1].Right;
        Cmd.common.DW52.ROI2_Y_bottom = pPicParams->ROI[1].Bottom;

        Cmd.common.DW53.ROI3_X_left = pPicParams->ROI[2].Left;
        Cmd.common.DW53.ROI3_Y_top = pPicParams->ROI[2].Top;
        Cmd.common.DW54.ROI3_X_right = pPicParams->ROI[2].Right;
        Cmd.common.DW54.ROI3_Y_bottom = pPicParams->ROI[2].Bottom;

        Cmd.common.DW55.ROI4_X_left = pPicParams->ROI[3].Left;
        Cmd.common.DW55.ROI4_Y_top = pPicParams->ROI[3].Top;
        Cmd.common.DW56.ROI4_X_right = pPicParams->ROI[3].Right;
        Cmd.common.DW56.ROI4_Y_bottom = pPicParams->ROI[3].Bottom;

        if (bBrcEnabled == false)
        {
            uint16_t numROI = pPicParams->NumROI;
            char priorityLevelOrDQp[CODECHAL_ENCODE_AVC_MAX_ROI_NUMBER] = { 0 };

            // cqp case
            for (unsigned int i = 0; i < numROI; i += 1)
            {
                char dQpRoi = pPicParams->ROI[i].PriorityLevelOrDQp;

                // clip qp roi in order to have (qp + qpY) in range [0, 51]
                priorityLevelOrDQp[i] = (char)CodecHal_Clip3(-SliceQP, CODECHAL_ENCODE_AVC_MAX_SLICE_QP - SliceQP, dQpRoi);
            }

            Cmd.common.DW57.ROI1_dQpPrimeY = priorityLevelOrDQp[0];
            Cmd.common.DW57.ROI2_dQpPrimeY = priorityLevelOrDQp[1];
            Cmd.common.DW57.ROI3_dQpPrimeY = priorityLevelOrDQp[2];
            Cmd.common.DW57.ROI4_dQpPrimeY = priorityLevelOrDQp[3];
        }
        else
        {
            // kernel does not support BRC case
            Cmd.common.DW34.ROIEnableFlag = 0;
        }
    }
    else if (pParams->bDirtyRoiEnabled)
    {
        // enable Dirty Rect flag
        Cmd.common.DW4.EnableDirtyRect = true;

        Cmd.common.DW49.ROI1_X_left = pParams->pPicParams->DirtyROI[0].Left;
        Cmd.common.DW49.ROI1_Y_top = pParams->pPicParams->DirtyROI[0].Top;
        Cmd.common.DW50.ROI1_X_right = pParams->pPicParams->DirtyROI[0].Right;
        Cmd.common.DW50.ROI1_Y_bottom = pParams->pPicParams->DirtyROI[0].Bottom;

        Cmd.common.DW51.ROI2_X_left = pParams->pPicParams->DirtyROI[1].Left;
        Cmd.common.DW51.ROI2_Y_top = pParams->pPicParams->DirtyROI[1].Top;
        Cmd.common.DW52.ROI2_X_right = pParams->pPicParams->DirtyROI[1].Right;
        Cmd.common.DW52.ROI2_Y_bottom = pParams->pPicParams->DirtyROI[1].Bottom;

        Cmd.common.DW53.ROI3_X_left = pParams->pPicParams->DirtyROI[2].Left;
        Cmd.common.DW53.ROI3_Y_top = pParams->pPicParams->DirtyROI[2].Top;
        Cmd.common.DW54.ROI3_X_right = pParams->pPicParams->DirtyROI[2].Right;
        Cmd.common.DW54.ROI3_Y_bottom = pParams->pPicParams->DirtyROI[2].Bottom;

        Cmd.common.DW55.ROI4_X_left = pParams->pPicParams->DirtyROI[3].Left;
        Cmd.common.DW55.ROI4_Y_top = pParams->pPicParams->DirtyROI[3].Top;
        Cmd.common.DW56.ROI4_X_right = pParams->pPicParams->DirtyROI[3].Right;
        Cmd.common.DW56.ROI4_Y_bottom = pParams->pPicParams->DirtyROI[3].Bottom;
    }

    //IPCM QP and threshold
    Cmd.common.DW60.IPCM_QP0 = IPCM_Threshold_Table[0].QP;
    Cmd.common.DW60.IPCM_QP1 = IPCM_Threshold_Table[1].QP;
    Cmd.common.DW60.IPCM_QP2 = IPCM_Threshold_Table[2].QP;
    Cmd.common.DW60.IPCM_QP3 = IPCM_Threshold_Table[3].QP;
    Cmd.common.DW61.IPCM_QP4 = IPCM_Threshold_Table[4].QP;

    Cmd.common.DW61.IPCM_Thresh0 = IPCM_Threshold_Table[0].Threshold;
    Cmd.common.DW62.IPCM_Thresh1 = IPCM_Threshold_Table[1].Threshold;
    Cmd.common.DW62.IPCM_Thresh2 = IPCM_Threshold_Table[2].Threshold;
    Cmd.common.DW63.IPCM_Thresh3 = IPCM_Threshold_Table[3].Threshold;
    Cmd.common.DW63.IPCM_Thresh4 = IPCM_Threshold_Table[4].Threshold;

    Cmd.common.DW64.EnableColorBleedWAforIntraSlice = 0;
    Cmd.common.DW64.MBInputEnable = bMbSpecificDataEnabled;

    if (IsMfeMbEncEnabled(pParams->bMbEncIFrameDistEnabled))
    {

        // MFE uses quality mode kernel for perf mode. It changes
        // some curbe settings to simulate the perf mode kernel
        // These settings include:
        //  MultipredictorL0EnableBit/MultipredictorL1EnableBit = 0, set by MultiPred[TargetUsage]
        //  AllFractional = 0, set by CODECHAL_ENCODE_AVC_AllFractional_Common[TargetUsage]
        //  EnableATD = 0, set by CODECHAL_ENCODE_AVC_EnableAdaptiveTxDecision_Common[TargetUsage]
        //  EnableFBRBypass = 0
        //  NumRefIdxL0MinusOne/NumRefIdxL1MinusOne = 0
        if (m_targetUsage == TARGETUSAGE_BEST_SPEED)
        {
            Cmd.common.DW4.EnableFBRBypass      = false;
            Cmd.common.DW36.NumRefIdxL0MinusOne = 0;
            Cmd.common.DW36.NumRefIdxL1MinusOne = 0;
        }

        MOS_LOCK_PARAMS LockFlagsWriteOnly;
        MOS_ZeroMemory(&LockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
        LockFlagsWriteOnly.WriteOnly = 1;

        uint8_t* pData = (uint8_t*)m_osInterface->pfnLockResource(
            m_osInterface,
            &(BrcBuffers.resMbEncBrcBuffer),
            &LockFlagsWriteOnly);
        CODECHAL_ENCODE_CHK_NULL_RETURN(pData);

        MOS_SecureMemcpy(pData, sizeof(Cmd.common), (void *)&Cmd, sizeof(Cmd.common));

        m_osInterface->pfnUnlockResource(m_osInterface, &BrcBuffers.resMbEncBrcBuffer);
    }
    else
    {
        Cmd.surfaces.DW80.MBDataSurfIndex                = CODECHAL_ENCODE_AVC_MBENC_MFC_AVC_PAK_OBJ_G9;
        Cmd.surfaces.DW81.MVDataSurfIndex                = CODECHAL_ENCODE_AVC_MBENC_IND_MV_DATA_G9;
        Cmd.surfaces.DW82.IDistSurfIndex                 = CODECHAL_ENCODE_AVC_MBENC_BRC_DISTORTION_G9;
        Cmd.surfaces.DW83.SrcYSurfIndex                  = CODECHAL_ENCODE_AVC_MBENC_CURR_Y_G9;
        Cmd.surfaces.DW84.MBSpecificDataSurfIndex        = CODECHAL_ENCODE_AVC_MBENC_MB_SPECIFIC_DATA_G9;
        Cmd.surfaces.DW85.AuxVmeOutSurfIndex             = CODECHAL_ENCODE_AVC_MBENC_AUX_VME_OUT_G9;
        Cmd.surfaces.DW86.CurrRefPicSelSurfIndex         = CODECHAL_ENCODE_AVC_MBENC_REFPICSELECT_L0_G9;
        Cmd.surfaces.DW87.HMEMVPredFwdBwdSurfIndex       = CODECHAL_ENCODE_AVC_MBENC_MV_DATA_FROM_ME_G9;
        Cmd.surfaces.DW88.HMEDistSurfIndex               = CODECHAL_ENCODE_AVC_MBENC_4xME_DISTORTION_G9;
        Cmd.surfaces.DW89.SliceMapSurfIndex              = CODECHAL_ENCODE_AVC_MBENC_SLICEMAP_DATA_G9;
        Cmd.surfaces.DW90.FwdFrmMBDataSurfIndex          = CODECHAL_ENCODE_AVC_MBENC_FWD_MB_DATA_G9;
        Cmd.surfaces.DW91.FwdFrmMVSurfIndex              = CODECHAL_ENCODE_AVC_MBENC_FWD_MV_DATA_G9;
        Cmd.surfaces.DW92.MBQPBuffer                     = CODECHAL_ENCODE_AVC_MBENC_MBQP_G9;
        Cmd.surfaces.DW93.MBBRCLut                       = CODECHAL_ENCODE_AVC_MBENC_MBBRC_CONST_DATA_G9;
        Cmd.surfaces.DW94.VMEInterPredictionSurfIndex    = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_CURR_PIC_IDX_0_G9;
        Cmd.surfaces.DW95.VMEInterPredictionMRSurfIndex  = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_CURR_PIC_IDX_1_G9;
        Cmd.surfaces.DW96.MbStatsSurfIndex               = CODECHAL_ENCODE_AVC_MBENC_MB_STATS_G9;
        Cmd.surfaces.DW97.MADSurfIndex                   = CODECHAL_ENCODE_AVC_MBENC_MAD_DATA_G9;
        Cmd.surfaces.DW98.ForceNonSkipMBmapSurface       = CODECHAL_ENCODE_AVC_MBENC_FORCE_NONSKIP_MB_MAP_G9;
        Cmd.surfaces.DW99.ReservedIndex                  = CODECHAL_ENCODE_AVC_MBENC_ADV_WA_G9;
        Cmd.surfaces.DW100.BRCCurbeSurfIndex             = CODECHAL_ENCODE_AVC_MBENC_BRC_CURBE_DATA_G9;
        Cmd.surfaces.DW101.StaticDetectionCostTableIndex = CODECHAL_ENCODE_AVC_MBENC_SFD_COST_TABLE_G9;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(pParams->pKernelState->m_dshRegion.AddData(
            &Cmd,
            pParams->pKernelState->dwCurbeOffset,
            sizeof(Cmd)));
    }

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(PopulateEncParam(
            ucMeMethod,
            &Cmd));
    )

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG9Skl::GetTrellisQuantization(PCODECHAL_ENCODE_AVC_TQ_INPUT_PARAMS pParams, PCODECHAL_ENCODE_AVC_TQ_PARAMS pTrellisQuantParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pTrellisQuantParams);

    pTrellisQuantParams->dwTqEnabled = TrellisQuantizationEnable[pParams->ucTargetUsage];
    pTrellisQuantParams->dwTqRounding =
        pTrellisQuantParams->dwTqEnabled ? TrellisQuantizationRounding[pParams->ucTargetUsage] : 0;

    // If AdaptiveTrellisQuantization is enabled then disable trellis quantization for
    // B-frames with QP > 26 only in CQP mode
    if (pTrellisQuantParams->dwTqEnabled
        && EnableAdaptiveTrellisQuantization[pParams->ucTargetUsage]
        && pParams->wPictureCodingType == B_TYPE
        && !pParams->bBrcEnabled && pParams->ucQP > 26)
    {
        pTrellisQuantParams->dwTqEnabled = 0;
        pTrellisQuantParams->dwTqRounding = 0;
    }
    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG9Skl::InitBrcConstantBufferMultiRefQP(PCODECHAL_ENCODE_AVC_INIT_BRC_CONSTANT_BUFFER_PARAMS pParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams->pOsInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams->pPicParams);

    uint8_t ucTableIdx = pParams->wPictureCodingType - 1;
    bool bBlockBasedSkipEn = pParams->dwMbEncBlockBasedSkipEn ? true : false;
    bool bTransform_8x8_mode_flag = pParams->pPicParams->transform_8x8_mode_flag ? true : false;

    if (ucTableIdx >= 3)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Invalid input parameter.");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    MOS_LOCK_PARAMS LockFlags;
    MOS_ZeroMemory(&LockFlags, sizeof(MOS_LOCK_PARAMS));
    LockFlags.WriteOnly = 1;
    uint8_t* pbData = (uint8_t*)pParams->pOsInterface->pfnLockResource(
        pParams->pOsInterface,
        &pParams->sBrcConstantDataBuffer.OsResource,
        &LockFlags);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pbData);

    MOS_ZeroMemory(pbData, pParams->sBrcConstantDataBuffer.dwWidth * pParams->sBrcConstantDataBuffer.dwHeight);

    // Fill surface with QP Adjustment table, Distortion threshold table, MaxFrame threshold table, Distortion QP Adjustment Table
    eStatus = MOS_SecureMemcpy(
        pbData,
        sizeof(m_qpDistMaxFrameAdjustmentCm),
        (void*)m_qpDistMaxFrameAdjustmentCm,
        sizeof(m_qpDistMaxFrameAdjustmentCm));
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
        return eStatus;
    }

    pbData += sizeof(m_qpDistMaxFrameAdjustmentCm);

    // Fill surface with Skip Threshold Table
    switch (pParams->wPictureCodingType)
    {
    case P_TYPE:
        eStatus = MOS_SecureMemcpy(
            pbData,
            m_brcConstantSurfaceEarlySkipTableSize,
            (void*)&SkipVal_P_Common[bBlockBasedSkipEn][bTransform_8x8_mode_flag][0],
            m_brcConstantSurfaceEarlySkipTableSize);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
            return eStatus;
        }
        break;
    case B_TYPE:
        eStatus = MOS_SecureMemcpy(
            pbData,
            m_brcConstantSurfaceEarlySkipTableSize,
            (void*)&SkipVal_B_Common[bBlockBasedSkipEn][bTransform_8x8_mode_flag][0],
            m_brcConstantSurfaceEarlySkipTableSize);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
            return eStatus;
        }
        break;
    default:
        // do nothing for I TYPE
        break;
    }

    if ((pParams->wPictureCodingType != I_TYPE) && (pParams->pAvcQCParams != nullptr) && (pParams->pAvcQCParams->NonFTQSkipThresholdLUTInput))
    {
        for (uint8_t ucQp = 0; ucQp < CODEC_AVC_NUM_QP; ucQp++)
        {
            *(pbData + 1 + (ucQp * 2)) = (uint8_t)CalcSkipVal((pParams->dwMbEncBlockBasedSkipEn ? true : false), (pParams->pPicParams->transform_8x8_mode_flag ? true : false), pParams->pAvcQCParams->NonFTQSkipThresholdLUT[ucQp]);
        }
    }

    pbData += m_brcConstantSurfaceEarlySkipTableSize;

    // Fill surface with QP list

    // Initialize to -1 (0xff)
    MOS_FillMemory(pbData, CODECHAL_ENCODE_AVC_BRC_CONSTANTSURFACE_QP_LIST_0_G9, 0xff);
    MOS_FillMemory(pbData
        + CODECHAL_ENCODE_AVC_BRC_CONSTANTSURFACE_QP_LIST_0_G9
        + CODECHAL_ENCODE_AVC_BRC_CONSTANTSURFACE_QP_LIST_0_RESERVED_G9,
        CODECHAL_ENCODE_AVC_BRC_CONSTANTSURFACE_QP_LIST_1_G9, 0xff);

    uint8_t           ucRefIdx;
    CODEC_PICTURE   RefPic;
    switch (pParams->wPictureCodingType)
    {
    case B_TYPE:
        pbData += (CODECHAL_ENCODE_AVC_BRC_CONSTANTSURFACE_QP_LIST_0_G9 + CODECHAL_ENCODE_AVC_BRC_CONSTANTSURFACE_QP_LIST_0_RESERVED_G9);

        for (ucRefIdx = 0; ucRefIdx <= pParams->pAvcSlcParams->num_ref_idx_l1_active_minus1; ucRefIdx++)
        {
            RefPic = pParams->pAvcSlcParams->RefPicList[LIST_1][ucRefIdx];
            if (!CodecHal_PictureIsInvalid(RefPic) && pParams->pAvcPicIdx[RefPic.FrameIdx].bValid)
            {
                *(pbData + ucRefIdx) = pParams->pAvcPicIdx[RefPic.FrameIdx].ucPicIdx;
            }
        }
        pbData -= (CODECHAL_ENCODE_AVC_BRC_CONSTANTSURFACE_QP_LIST_0_G9 + CODECHAL_ENCODE_AVC_BRC_CONSTANTSURFACE_QP_LIST_0_RESERVED_G9);
        // break statement omitted intentionally
    case P_TYPE:
        for (ucRefIdx = 0; ucRefIdx <= pParams->pAvcSlcParams->num_ref_idx_l0_active_minus1; ucRefIdx++)
        {
            RefPic = pParams->pAvcSlcParams->RefPicList[LIST_0][ucRefIdx];
            if (!CodecHal_PictureIsInvalid(RefPic) && pParams->pAvcPicIdx[RefPic.FrameIdx].bValid)
            {
                *(pbData + ucRefIdx) = pParams->pAvcPicIdx[RefPic.FrameIdx].ucPicIdx;
            }
        }
        break;
    default:
        // do nothing for I type
        break;
    }

    pbData += (CODECHAL_ENCODE_AVC_BRC_CONSTANTSURFACE_QP_LIST_0_G9 + CODECHAL_ENCODE_AVC_BRC_CONSTANTSURFACE_QP_LIST_0_RESERVED_G9
        + CODECHAL_ENCODE_AVC_BRC_CONSTANTSURFACE_QP_LIST_1_G9 + CODECHAL_ENCODE_AVC_BRC_CONSTANTSURFACE_QP_LIST_1_RESERVED_G9);

    // Fill surface with Mode cost and MV cost
    eStatus = MOS_SecureMemcpy(
        pbData,
        m_brcConstantSurfacModeMvCostSize,
        (void*)ModeMvCost_Cm[ucTableIdx],
        m_brcConstantSurfacModeMvCostSize);
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
        return eStatus;
    }

    // If old mode cost is used the update the table
    uint32_t* pdwDataTemp;
    if (pParams->wPictureCodingType == I_TYPE && pParams->bOldModeCostEnable)
    {
        pdwDataTemp = (uint32_t*)pbData;
        for (uint8_t ucQp = 0; ucQp < CODEC_AVC_NUM_QP; ucQp++)
        {
            // Writing to DW0 in each sub-array of 16 DWs
            *pdwDataTemp = (uint32_t)OldIntraModeCost_Cm_Common[ucQp];
            pdwDataTemp += 16;
        }
    }

    if (pParams->pAvcQCParams)
    {
        for (uint8_t ucQp = 0; ucQp < CODEC_AVC_NUM_QP; ucQp++)
        {
            if (pParams->pAvcQCParams->FTQSkipThresholdLUTInput)
            {
                *(pbData + (ucQp * 32) + 24) =
                    *(pbData + (ucQp * 32) + 25) =
                    *(pbData + (ucQp * 32) + 27) =
                    *(pbData + (ucQp * 32) + 28) =
                    *(pbData + (ucQp * 32) + 29) =
                    *(pbData + (ucQp * 32) + 30) =
                    *(pbData + (ucQp * 32) + 31) = pParams->pAvcQCParams->FTQSkipThresholdLUT[ucQp];
            }
        }
    }

    pbData += m_brcConstantSurfacModeMvCostSize;

    // Fill surface with Refcost
    eStatus = MOS_SecureMemcpy(
        pbData,
        m_brcConstantSurfaceRefCostSize,
        (void*)&RefCost_MultiRefQp[ucTableIdx][0],
        m_brcConstantSurfaceRefCostSize);
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
        return eStatus;
    }
    pbData += m_brcConstantSurfaceRefCostSize;

    //Fill surface with Intra cost scaling Factor
    if (pParams->bAdaptiveIntraScalingEnable)
    {
        eStatus = MOS_SecureMemcpy(
            pbData,
            CODECHAL_ENCODE_AVC_BRC_CONSTANTSURFACE_INTRACOST_SCALING_FACTOR_G9,
            (void*)&AdaptiveIntraScalingFactor_Cm_Common[0],
            CODECHAL_ENCODE_AVC_BRC_CONSTANTSURFACE_INTRACOST_SCALING_FACTOR_G9);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
            return eStatus;
        }
    }
    else
    {
        eStatus = MOS_SecureMemcpy(
            pbData,
            CODECHAL_ENCODE_AVC_BRC_CONSTANTSURFACE_INTRACOST_SCALING_FACTOR_G9,
            (void*)&IntraScalingFactor_Cm_Common[0],
            CODECHAL_ENCODE_AVC_BRC_CONSTANTSURFACE_INTRACOST_SCALING_FACTOR_G9);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
            return eStatus;
        }
    }

    pParams->pOsInterface->pfnUnlockResource(
        pParams->pOsInterface,
        &pParams->sBrcConstantDataBuffer.OsResource);

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG9Skl::InitBrcConstantBuffer(PCODECHAL_ENCODE_AVC_INIT_BRC_CONSTANT_BUFFER_PARAMS pParams)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (bMultiRefQpEnabled)
    {
        return InitBrcConstantBufferMultiRefQP(pParams);
    }
    else
    {
        return CodechalEncodeAvcEnc::InitBrcConstantBuffer(pParams);
    }
}

CodechalEncodeAvcEncG9Skl::CodechalEncodeAvcEncG9Skl(
        CodechalHwInterface *   hwInterface,
        CodechalDebugInterface *debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo): CodechalEncodeAvcEncG9(hwInterface, debugInterface, standardInfo)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    m_cmKernelEnable = true;
    bBrcSplitEnable = true;
    bBrcRoiSupported = true;
    bHighTextureModeCostEnable = true;

    this->pfnGetKernelHeaderAndSize = this->GetKernelHeaderAndSize;

    m_mbStatsSupported = true; //Starting from GEN9
    m_kernelBase = (uint8_t *)IGCODECKRN_G9;
    AddIshSize(m_kuid, m_kernelBase);
}

MOS_STATUS CodechalEncodeAvcEncG9Skl::InitializeState()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncodeAvcEncG9::InitializeState());

    m_brcHistoryBufferSize = CODECHAL_ENCODE_AVC_BRC_HISTORY_BUFFER_SIZE_G9;
    dwBrcConstantSurfaceWidth = CODECHAL_ENCODE_AVC_BRC_CONSTANTSURFACE_WIDTH_G9;
    dwBrcConstantSurfaceHeight = CODECHAL_ENCODE_AVC_BRC_CONSTANTSURFACE_HEIGHT_G9;

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG9Skl::InitKernelStateMbEnc()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    dwNumMbEncEncKrnStates =
        MBENC_TARGET_USAGE_CM * MBENC_NUM_TARGET_USAGES_CM_G9_SKL;
    dwNumMbEncEncKrnStates += MBENC_TARGET_USAGE_CM;
    pMbEncKernelStates =
        MOS_NewArray(MHW_KERNEL_STATE, dwNumMbEncEncKrnStates);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pMbEncKernelStates);

    PMHW_KERNEL_STATE                           pKernelStatePtr = pMbEncKernelStates;
    CODECHAL_KERNEL_HEADER                      CurrKrnHeader;

    uint8_t* kernelBinary;
    uint32_t kernelSize;

    MOS_STATUS status = CodecHalGetKernelBinaryAndSize(m_kernelBase, m_kuid, &kernelBinary, &kernelSize);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(status);

    for (uint32_t dwKrnStateIdx = 0; dwKrnStateIdx < dwNumMbEncEncKrnStates; dwKrnStateIdx++)
    {
        bool bKernelState = (dwKrnStateIdx >= MBENC_TARGET_USAGE_CM * MBENC_NUM_TARGET_USAGES_CM_G9_SKL);

        CODECHAL_ENCODE_CHK_STATUS_RETURN(GetKernelHeaderAndSize(
            kernelBinary,
            (bKernelState ? ENC_MBENC_ADV : ENC_MBENC),
            (bKernelState ? dwKrnStateIdx - MBENC_TARGET_USAGE_CM * MBENC_NUM_TARGET_USAGES_CM_G9_SKL : dwKrnStateIdx),
            (void*)&CurrKrnHeader,
            &kernelSize));

        pKernelStatePtr->KernelParams.iBTCount = CODECHAL_ENCODE_AVC_MBENC_NUM_SURFACES_G9;
        pKernelStatePtr->KernelParams.iThreadCount = m_renderEngineInterface->GetHwCaps()->dwMaxThreads;
        pKernelStatePtr->KernelParams.iCurbeLength = sizeof(CODECHAL_ENCODE_AVC_MBENC_CURBE_G9);
        pKernelStatePtr->KernelParams.iBlockWidth = CODECHAL_MACROBLOCK_WIDTH;
        pKernelStatePtr->KernelParams.iBlockHeight = CODECHAL_MACROBLOCK_HEIGHT;
        pKernelStatePtr->KernelParams.iIdCount = 1;

        pKernelStatePtr->dwCurbeOffset = m_stateHeapInterface->pStateHeapInterface->GetSizeofCmdInterfaceDescriptorData();
        pKernelStatePtr->KernelParams.pBinary = kernelBinary + (CurrKrnHeader.KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);
        pKernelStatePtr->KernelParams.iSize = kernelSize;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnCalculateSshAndBtSizesRequested(
            m_stateHeapInterface,
            pKernelStatePtr->KernelParams.iBTCount,
            &pKernelStatePtr->dwSshSize,
            &pKernelStatePtr->dwBindingTableSize));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->MhwInitISH(m_stateHeapInterface, pKernelStatePtr));

        pKernelStatePtr++;
    }

    // Until a better way can be found, maintain old binding table structures
    auto pBindingTable = &MbEncBindingTable;

    pBindingTable->dwAvcMBEncMfcAvcPakObj = CODECHAL_ENCODE_AVC_MBENC_MFC_AVC_PAK_OBJ_G9;
    pBindingTable->dwAvcMBEncIndMVData = CODECHAL_ENCODE_AVC_MBENC_IND_MV_DATA_G9;
    pBindingTable->dwAvcMBEncBRCDist = CODECHAL_ENCODE_AVC_MBENC_BRC_DISTORTION_G9;
    pBindingTable->dwAvcMBEncCurrY = CODECHAL_ENCODE_AVC_MBENC_CURR_Y_G9;
    pBindingTable->dwAvcMBEncCurrUV = CODECHAL_ENCODE_AVC_MBENC_CURR_UV_G9;
    pBindingTable->dwAvcMBEncMbSpecificData = CODECHAL_ENCODE_AVC_MBENC_MB_SPECIFIC_DATA_G9;

    pBindingTable->dwAvcMBEncRefPicSelectL0 = CODECHAL_ENCODE_AVC_MBENC_REFPICSELECT_L0_G9;
    pBindingTable->dwAvcMBEncMVDataFromME = CODECHAL_ENCODE_AVC_MBENC_MV_DATA_FROM_ME_G9;
    pBindingTable->dwAvcMBEncMEDist = CODECHAL_ENCODE_AVC_MBENC_4xME_DISTORTION_G9;
    pBindingTable->dwAvcMBEncSliceMapData = CODECHAL_ENCODE_AVC_MBENC_SLICEMAP_DATA_G9;
    pBindingTable->dwAvcMBEncBwdRefMBData = CODECHAL_ENCODE_AVC_MBENC_FWD_MB_DATA_G9;
    pBindingTable->dwAvcMBEncBwdRefMVData = CODECHAL_ENCODE_AVC_MBENC_FWD_MV_DATA_G9;
    pBindingTable->dwAvcMBEncMbBrcConstData = CODECHAL_ENCODE_AVC_MBENC_MBBRC_CONST_DATA_G9;
    pBindingTable->dwAvcMBEncMBStats = CODECHAL_ENCODE_AVC_MBENC_MB_STATS_G9;
    pBindingTable->dwAvcMBEncMADData = CODECHAL_ENCODE_AVC_MBENC_MAD_DATA_G9;
    pBindingTable->dwAvcMBEncMbNonSkipMap = CODECHAL_ENCODE_AVC_MBENC_FORCE_NONSKIP_MB_MAP_G9;
    pBindingTable->dwAvcMBEncAdv = CODECHAL_ENCODE_AVC_MBENC_ADV_WA_G9;
    pBindingTable->dwAvcMbEncBRCCurbeData = CODECHAL_ENCODE_AVC_MBENC_BRC_CURBE_DATA_G9;
    pBindingTable->dwAvcMBEncFlatnessChk = CODECHAL_ENCODE_AVC_MBENC_FLATNESS_CHECK_CM_G9;
    pBindingTable->dwAvcMBEncStaticDetectionCostTable = CODECHAL_ENCODE_AVC_MBENC_SFD_COST_TABLE_G9;

    // Frame
    pBindingTable->dwAvcMBEncMbQpFrame = CODECHAL_ENCODE_AVC_MBENC_MBQP_G9;
    pBindingTable->dwAvcMBEncCurrPicFrame[0] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_CURR_PIC_IDX_0_G9;
    pBindingTable->dwAvcMBEncFwdPicFrame[0] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX0_G9;
    pBindingTable->dwAvcMBEncBwdPicFrame[0] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_BWD_PIC_IDX0_0_G9;
    pBindingTable->dwAvcMBEncFwdPicFrame[1] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX1_G9;
    pBindingTable->dwAvcMBEncBwdPicFrame[1] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_BWD_PIC_IDX1_0_G9;
    pBindingTable->dwAvcMBEncFwdPicFrame[2] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX2_G9;
    pBindingTable->dwAvcMBEncFwdPicFrame[3] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX3_G9;
    pBindingTable->dwAvcMBEncFwdPicFrame[4] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX4_G9;
    pBindingTable->dwAvcMBEncFwdPicFrame[5] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX5_G9;
    pBindingTable->dwAvcMBEncFwdPicFrame[6] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX6_G9;
    pBindingTable->dwAvcMBEncFwdPicFrame[7] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX7_G9;
    pBindingTable->dwAvcMBEncCurrPicFrame[1] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_CURR_PIC_IDX_1_G9;
    pBindingTable->dwAvcMBEncBwdPicFrame[2] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_BWD_PIC_IDX0_1_G9;
    pBindingTable->dwAvcMBEncBwdPicFrame[3] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_BWD_PIC_IDX1_1_G9;

    // Field
    pBindingTable->dwAvcMBEncMbQpField = CODECHAL_ENCODE_AVC_MBENC_MBQP_G9;
    pBindingTable->dwAvcMBEncFieldCurrPic[0] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_CURR_PIC_IDX_0_G9;
    pBindingTable->dwAvcMBEncFwdPicTopField[0] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX0_G9;
    pBindingTable->dwAvcMBEncBwdPicTopField[0] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_BWD_PIC_IDX0_0_G9;
    pBindingTable->dwAvcMBEncFwdPicBotField[0] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX0_G9;
    pBindingTable->dwAvcMBEncBwdPicBotField[0] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_BWD_PIC_IDX0_0_G9;
    pBindingTable->dwAvcMBEncFwdPicTopField[1] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX1_G9;
    pBindingTable->dwAvcMBEncBwdPicTopField[1] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_BWD_PIC_IDX1_0_G9;
    pBindingTable->dwAvcMBEncFwdPicBotField[1] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX1_G9;
    pBindingTable->dwAvcMBEncBwdPicBotField[1] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_BWD_PIC_IDX1_0_G9;
    pBindingTable->dwAvcMBEncFwdPicTopField[2] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX2_G9;
    pBindingTable->dwAvcMBEncFwdPicBotField[2] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX2_G9;
    pBindingTable->dwAvcMBEncFwdPicTopField[3] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX3_G9;
    pBindingTable->dwAvcMBEncFwdPicBotField[3] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX3_G9;
    pBindingTable->dwAvcMBEncFwdPicTopField[4] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX4_G9;
    pBindingTable->dwAvcMBEncFwdPicBotField[4] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX4_G9;
    pBindingTable->dwAvcMBEncFwdPicTopField[5] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX5_G9;
    pBindingTable->dwAvcMBEncFwdPicBotField[5] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX5_G9;
    pBindingTable->dwAvcMBEncFwdPicTopField[6] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX6_G9;
    pBindingTable->dwAvcMBEncFwdPicBotField[6] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX6_G9;
    pBindingTable->dwAvcMBEncFwdPicTopField[7] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX7_G9;
    pBindingTable->dwAvcMBEncFwdPicBotField[7] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX7_G9;
    pBindingTable->dwAvcMBEncFieldCurrPic[1] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_CURR_PIC_IDX_1_G9;
    pBindingTable->dwAvcMBEncBwdPicTopField[2] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_BWD_PIC_IDX0_1_G9;
    pBindingTable->dwAvcMBEncBwdPicBotField[2] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_BWD_PIC_IDX0_1_G9;
    pBindingTable->dwAvcMBEncBwdPicTopField[3] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_BWD_PIC_IDX1_1_G9;
    pBindingTable->dwAvcMBEncBwdPicBotField[3] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_BWD_PIC_IDX1_1_G9;

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG9Skl::InitKernelStateMfeMbEnc()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_KERNEL_HEADER CurrKrnHeader;
    auto pKernelStatePtr = &mfeMbEncKernelState;

    uint8_t* kernelBinary;
    uint32_t kernelSize;

    MOS_STATUS status = CodecHalGetKernelBinaryAndSize(m_kernelBase, m_kuid, &kernelBinary, &kernelSize);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(status);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(GetKernelHeaderAndSize(
        kernelBinary,
        ENC_MBENC,
        CODECHAL_ENCODE_AVC_MFE_MBENC_KERNEL_IDX,
        (void*)&CurrKrnHeader,
        &kernelSize));

    pKernelStatePtr->KernelParams.iBTCount = CODECHAL_ENCODE_AVC_MBENC_NUM_SURFACES_G9 * CODECHAL_ENCODE_AVC_MFE_MAX_FRAMES_G9;
    pKernelStatePtr->KernelParams.iCurbeLength = sizeof(CODECHAL_ENCODE_AVC_MFE_MBENC_CURBE_G9);
    pKernelStatePtr->KernelParams.iThreadCount = m_renderEngineInterface->GetHwCaps()->dwMaxThreads;
    pKernelStatePtr->KernelParams.iBlockWidth = CODECHAL_MACROBLOCK_WIDTH;
    pKernelStatePtr->KernelParams.iBlockHeight = CODECHAL_MACROBLOCK_HEIGHT;
    pKernelStatePtr->KernelParams.iIdCount = 1;

    pKernelStatePtr->dwCurbeOffset = m_stateHeapInterface->pStateHeapInterface->GetSizeofCmdInterfaceDescriptorData();
    pKernelStatePtr->KernelParams.pBinary = kernelBinary + (CurrKrnHeader.KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);
    pKernelStatePtr->KernelParams.iSize = kernelSize;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnCalculateSshAndBtSizesRequested(
        m_stateHeapInterface,
        pKernelStatePtr->KernelParams.iBTCount,
        &pKernelStatePtr->dwSshSize,
        &pKernelStatePtr->dwBindingTableSize));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->MhwInitISH(m_stateHeapInterface, pKernelStatePtr));

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG9Skl::InitKernelStateBrc()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    uint8_t* kernelBinary;
    uint32_t kernelSize;

    MOS_STATUS status = CodecHalGetKernelBinaryAndSize(m_kernelBase, m_kuid, &kernelBinary, &kernelSize);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(status);

    CODECHAL_KERNEL_HEADER CurrKrnHeader;
    for (uint32_t dwKrnStateIdx = 0; dwKrnStateIdx < CODECHAL_ENCODE_BRC_IDX_NUM; dwKrnStateIdx++)
    {
        auto pKernelStatePtr = &BrcKernelStates[dwKrnStateIdx];
        CODECHAL_ENCODE_CHK_STATUS_RETURN(GetKernelHeaderAndSize(
            kernelBinary,
            ENC_BRC,
            dwKrnStateIdx,
            (void*)&CurrKrnHeader,
            &kernelSize));

        pKernelStatePtr->KernelParams.iBTCount = BRC_BTCOUNTS[dwKrnStateIdx];
        pKernelStatePtr->KernelParams.iThreadCount = m_renderEngineInterface->GetHwCaps()->dwMaxThreads;
        pKernelStatePtr->KernelParams.iCurbeLength = BRC_CURBE_SIZE[dwKrnStateIdx];
        pKernelStatePtr->KernelParams.iBlockWidth = CODECHAL_MACROBLOCK_WIDTH;
        pKernelStatePtr->KernelParams.iBlockHeight = CODECHAL_MACROBLOCK_HEIGHT;
        pKernelStatePtr->KernelParams.iIdCount = 1;

        pKernelStatePtr->dwCurbeOffset = m_stateHeapInterface->pStateHeapInterface->GetSizeofCmdInterfaceDescriptorData();
        pKernelStatePtr->KernelParams.pBinary = kernelBinary + (CurrKrnHeader.KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);
        pKernelStatePtr->KernelParams.iSize = kernelSize;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnCalculateSshAndBtSizesRequested(
            m_stateHeapInterface,
            pKernelStatePtr->KernelParams.iBTCount,
            &pKernelStatePtr->dwSshSize,
            &pKernelStatePtr->dwBindingTableSize));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->MhwInitISH(m_stateHeapInterface, pKernelStatePtr));
    }

    // Until a better way can be found, maintain old binding table structures
    auto pBindingTable = &BrcUpdateBindingTable;
    pBindingTable->dwFrameBrcHistoryBuffer = CODECHAL_ENCODE_AVC_FRAME_BRC_UPDATE_HISTORY_G9;
    pBindingTable->dwFrameBrcPakStatisticsOutputBuffer = CODECHAL_ENCODE_AVC_FRAME_BRC_UPDATE_PAK_STATISTICS_OUTPUT_G9;
    pBindingTable->dwFrameBrcImageStateReadBuffer = CODECHAL_ENCODE_AVC_FRAME_BRC_UPDATE_IMAGE_STATE_READ_G9;
    pBindingTable->dwFrameBrcImageStateWriteBuffer = CODECHAL_ENCODE_AVC_FRAME_BRC_UPDATE_IMAGE_STATE_WRITE_G9;
    pBindingTable->dwFrameBrcMbEncCurbeReadBuffer = CODECHAL_ENCODE_AVC_FRAME_BRC_UPDATE_MBENC_CURBE_READ_G9;
    pBindingTable->dwFrameBrcMbEncCurbeWriteData = CODECHAL_ENCODE_AVC_FRAME_BRC_UPDATE_MBENC_CURBE_WRITE_G9;
    pBindingTable->dwFrameBrcDistortionBuffer = CODECHAL_ENCODE_AVC_FRAME_BRC_UPDATE_DISTORTION_G9;
    pBindingTable->dwFrameBrcConstantData = CODECHAL_ENCODE_AVC_FRAME_BRC_UPDATE_CONSTANT_DATA_G9;
    pBindingTable->dwFrameBrcMbStatBuffer = CODECHAL_ENCODE_AVC_FRAME_BRC_UPDATE_MB_STAT_G9;
    // starting GEN9 BRC kernel has split into a frame level update, and an MB level update.
    // above is BTI for frame level, below is BTI for MB level
    pBindingTable->dwMbBrcHistoryBuffer = CODECHAL_ENCODE_AVC_MB_BRC_UPDATE_HISTORY_G9;
    pBindingTable->dwMbBrcMbQpBuffer = CODECHAL_ENCODE_AVC_MB_BRC_UPDATE_MB_QP_G9;
    pBindingTable->dwMbBrcROISurface = CODECHAL_ENCODE_AVC_MB_BRC_UPDATE_ROI_G9;
    pBindingTable->dwMbBrcMbStatBuffer = CODECHAL_ENCODE_AVC_MB_BRC_UPDATE_MB_STAT_G9;

    return eStatus;
}

void CodechalEncodeAvcEncG9Skl::UpdateSSDSliceCount()
{
    CodechalEncodeAvcBase::UpdateSSDSliceCount();

    uint32_t sliceCount;
    if (m_frameHeight * m_frameWidth >= 1920*1080 && m_targetUsage <= 4 ||
        m_frameHeight * m_frameWidth >= 1280*720 && m_targetUsage <= 2 ||
        m_frameHeight * m_frameWidth >= 3840*2160)
    {
        sliceCount = 2;
    }
    else
    {
        sliceCount = 1;
    }

    if (m_osInterface->pfnSetSliceCount)
    {
        m_osInterface->pfnSetSliceCount(m_osInterface, &sliceCount);
    }
}

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS CodechalEncodeAvcEncG9Skl::PopulatePakParam(
    PMOS_COMMAND_BUFFER cmdBuffer,
    PMHW_BATCH_BUFFER   secondLevelBatchBuffer)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    CODECHAL_DEBUG_CHK_NULL(m_debugInterface);

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDumpEncodePar))
    {
        return MOS_STATUS_SUCCESS;
    }

    uint8_t         *data = nullptr;
    MOS_LOCK_PARAMS lockFlags;
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.ReadOnly = 1;

    if (cmdBuffer != nullptr)
    {
        data = (uint8_t*)(cmdBuffer->pCmdPtr - (mhw_vdbox_mfx_g9_skl::MFX_AVC_IMG_STATE_CMD::byteSize / sizeof(uint32_t)));
    }
    else if (secondLevelBatchBuffer != nullptr)
    {
        data = secondLevelBatchBuffer->pData;
    }
    else
    {
        data = (uint8_t*)m_osInterface->pfnLockResource(m_osInterface, &BrcBuffers.resBrcImageStatesReadBuffer[m_currRecycledBufIdx], &lockFlags);
    }

    CODECHAL_DEBUG_CHK_NULL(data);

    mhw_vdbox_mfx_g9_skl::MFX_AVC_IMG_STATE_CMD mfxCmd;
    mfxCmd = *(mhw_vdbox_mfx_g9_skl::MFX_AVC_IMG_STATE_CMD *)(data);

    if (m_pictureCodingType == I_TYPE)
    {
        m_avcPar->TrellisQuantizationEnable         = mfxCmd.DW5.TrellisQuantizationEnabledTqenb;
        m_avcPar->EnableAdaptiveTrellisQuantization = mfxCmd.DW5.TrellisQuantizationEnabledTqenb;
        m_avcPar->TrellisQuantizationRounding       = mfxCmd.DW5.TrellisQuantizationRoundingTqr;
        m_avcPar->TrellisQuantizationChromaDisable  = mfxCmd.DW5.TrellisQuantizationChromaDisableTqchromadisable;
        m_avcPar->ExtendedRhoDomainEn               = mfxCmd.DW16_17.ExtendedRhodomainStatisticsEnable;
    }

    if (data && (cmdBuffer == nullptr) && (secondLevelBatchBuffer == nullptr))
    {
        m_osInterface->pfnUnlockResource(
            m_osInterface,
            &BrcBuffers.resBrcImageStatesReadBuffer[m_currRecycledBufIdx]);
    }

    return MOS_STATUS_SUCCESS;
}
#endif
