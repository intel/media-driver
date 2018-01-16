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
//! \file     codechal_encode_hevc_g9.h
//! \brief    HEVC dual-pipe encoder for GEN9 platform.
//!

#ifndef __CODECHAL_ENCODE_HEVC_G9_H__
#define __CODECHAL_ENCODE_HEVC_G9_H__

#include "codechal_encode_hevc.h"

enum CODECHAL_ENCODE_BINDING_TABLE_OFFSET_ME_CM_G9
{
    CODECHAL_ENCODE_ME_MV_DATA_SURFACE_CM_G9 = 0,
    CODECHAL_ENCODE_16xME_MV_DATA_SURFACE_CM_G9 = 1,
    CODECHAL_ENCODE_32xME_MV_DATA_SURFACE_CM_G9 = 1,
    CODECHAL_ENCODE_ME_DISTORTION_SURFACE_CM_G9 = 2,
    CODECHAL_ENCODE_ME_BRC_DISTORTION_CM_G9 = 3,
    CODECHAL_ENCODE_ME_RESERVED0_CM_G9 = 4,
    CODECHAL_ENCODE_ME_CURR_FOR_FWD_REF_CM_G9 = 5,
    CODECHAL_ENCODE_ME_FWD_REF_IDX0_CM_G9 = 6,
    CODECHAL_ENCODE_ME_RESERVED1_CM_G9 = 7,
    CODECHAL_ENCODE_ME_FWD_REF_IDX1_CM_G9 = 8,
    CODECHAL_ENCODE_ME_RESERVED2_CM_G9 = 9,
    CODECHAL_ENCODE_ME_FWD_REF_IDX2_CM_G9 = 10,
    CODECHAL_ENCODE_ME_RESERVED3_CM_G9 = 11,
    CODECHAL_ENCODE_ME_FWD_REF_IDX3_CM_G9 = 12,
    CODECHAL_ENCODE_ME_RESERVED4_CM_G9 = 13,
    CODECHAL_ENCODE_ME_FWD_REF_IDX4_CM_G9 = 14,
    CODECHAL_ENCODE_ME_RESERVED5_CM_G9 = 15,
    CODECHAL_ENCODE_ME_FWD_REF_IDX5_CM_G9 = 16,
    CODECHAL_ENCODE_ME_RESERVED6_CM_G9 = 17,
    CODECHAL_ENCODE_ME_FWD_REF_IDX6_CM_G9 = 18,
    CODECHAL_ENCODE_ME_RESERVED7_CM_G9 = 19,
    CODECHAL_ENCODE_ME_FWD_REF_IDX7_CM_G9 = 20,
    CODECHAL_ENCODE_ME_RESERVED8_CM_G9 = 21,
    CODECHAL_ENCODE_ME_CURR_FOR_BWD_REF_CM_G9 = 22,
    CODECHAL_ENCODE_ME_BWD_REF_IDX0_CM_G9 = 23,
    CODECHAL_ENCODE_ME_RESERVED9_CM_G9 = 24,
    CODECHAL_ENCODE_ME_BWD_REF_IDX1_CM_G9 = 25,
    CODECHAL_ENCODE_ME_VDENC_STREAMIN_CM_G9 = 26,
    CODECHAL_ENCODE_ME_NUM_SURFACES_CM_G9 = 27
};

//! Intra transform type
enum
{
    INTRA_TRANSFORM_REGULAR = 0,
    INTRA_TRANSFORM_RESERVED = 1,
    INTRA_TRANSFORM_HAAR = 2,
    INTRA_TRANSFORM_HADAMARD = 3
};

// Downscaling 2x kernels for Ultra HME
struct MEDIA_OBJECT_DOWNSCALING_2X_STATIC_DATA_G9
{
    union {
        struct {
            uint32_t       PicWidth : MOS_BITFIELD_RANGE(0, 15);
            uint32_t       PicHeight : MOS_BITFIELD_RANGE(16, 31);
        };
        uint32_t Value;
    } DW0;

    union {
        struct {
            uint32_t       Reserved;
        };
        uint32_t Value;
    } DW1;

    union {
        struct {
            uint32_t       Reserved;
        };
        uint32_t Value;
    } DW2;

    union {
        struct {
            uint32_t       Reserved;
        };
        uint32_t Value;
    } DW3;

    union {
        struct {
            uint32_t       Reserved;
        };
        uint32_t Value;
    } DW4;

    union {
        struct {
            uint32_t       Reserved;
        };
        uint32_t Value;
    } DW5;

    union {
        struct {
            uint32_t       Reserved;
        };
        uint32_t Value;
    } DW6;

    union {
        struct {
            uint32_t       Reserved;
        };
        uint32_t Value;
    } DW7;

    union {
        struct {
            uint32_t       BTI_Src_Y;
        };
        uint32_t Value;
    } DW8;

    union {
        struct {
            uint32_t       BTI_Dst_Y;
        };
        uint32_t Value;
    } DW9;
};


//! HEVC encoder intra 32x32 PU mode decision kernel curbe for GEN9
struct CODECHAL_ENC_HEVC_I_32x32_PU_MODE_DECISION_CURBE_G9
{
    union {
        struct {
            uint32_t       FrameWidth                   : MOS_BITFIELD_RANGE(0, 15);
            uint32_t       FrameHeight                  : MOS_BITFIELD_RANGE(16, 31);
        };
        uint32_t Value;
    } DW0;

    union {
        struct {
            uint32_t       SliceType                    : MOS_BITFIELD_RANGE(0, 1);
            uint32_t       PuType                       : MOS_BITFIELD_RANGE(2, 3);
            uint32_t       EnableStatsDataDump          : MOS_BITFIELD_BIT(4);
            uint32_t       LCUType                      : MOS_BITFIELD_BIT(5);
            uint32_t       Res_6_15                     : MOS_BITFIELD_RANGE(6, 15);
            uint32_t       SliceQP                      : MOS_BITFIELD_RANGE(16, 23);
            uint32_t       BRCEnable                    : MOS_BITFIELD_BIT(24);
            uint32_t       LCUBRCEnable                 : MOS_BITFIELD_BIT(25);
            uint32_t       ROIEnable                    : MOS_BITFIELD_BIT(26);
            uint32_t       FASTSurveillanceFlag         : MOS_BITFIELD_BIT(27);
            uint32_t       Res_28                       : MOS_BITFIELD_BIT(28);
            uint32_t       EnableFlexibleParam          : MOS_BITFIELD_BIT(29);
            uint32_t       EnableQualityImprovement     : MOS_BITFIELD_BIT(30);
            uint32_t       EnableDebugDump              : MOS_BITFIELD_BIT(31);
        };
        uint32_t Value;
    } DW1;

    union {
        struct {
            uint32_t       Lambda;
        };
        uint32_t Value;
    } DW2;

    union {
        struct {
            uint32_t       ModeCost32x32;
        };
        uint32_t Value;
    } DW3;

    union {
        struct {
            uint32_t       EarlyExit;
        };
        uint32_t Value;
    } DW4;

    union {
        struct {
            uint32_t       Reserved;
        };
        uint32_t Value;
    } DW5;

    union {
        struct {
            uint32_t       Reserved;
        };
        uint32_t Value;
    } DW6;

    union {
        struct {
            uint32_t       Reserved;
        };
        uint32_t Value;
    } DW7;

    union {
        struct {
            uint32_t       BTI_32x32PU_Output;
        };
        uint32_t Value;
    } DW8;

    union {
        struct {
            uint32_t       BTI_Src_Y;
        };
        uint32_t Value;
    } DW9;

    union {
        struct {
            uint32_t       BTI_Src_Y2x;
        };
        uint32_t Value;
    } DW10;

    union {
        struct {
            uint32_t       BTI_Slice_Map;
        };
        uint32_t Value;
    } DW11;

    union {
        struct {
            uint32_t       BTI_Src_Y2x_VME;
        };
        uint32_t Value;
    } DW12;

    union {
        struct {
            uint32_t       BTI_Brc_Input;
        };
        uint32_t Value;
    } DW13;

    union {
        struct {
            uint32_t       BTI_LCU_Qp_Surface;
        };
        uint32_t Value;
    } DW14;

    union {
        struct {
            uint32_t       BTI_Brc_Data;
        };
        uint32_t Value;
    } DW15;

    union {
        struct {
            uint32_t       BTI_Stats_Data;
        };
        uint32_t Value;
    } DW16;

    union {
        struct {
            uint32_t       BTI_Kernel_Debug;
        };
        uint32_t Value;
    } DW17;
};

using PCODECHAL_ENC_HEVC_I_32x32_PU_MODE_DECISION_CURBE_G9 = struct CODECHAL_ENC_HEVC_I_32x32_PU_MODE_DECISION_CURBE_G9*;
C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(CODECHAL_ENC_HEVC_I_32x32_PU_MODE_DECISION_CURBE_G9)) == 18);

//! HEVC encoder intra 16x16 SAD kernel curbe for GEN9
struct CODECHAL_ENC_HEVC_I_16x16_SAD_CURBE_G9
{
    union {
        struct {
            uint32_t       FrameWidth                   : MOS_BITFIELD_RANGE(0, 15);
            uint32_t       FrameHeight                  : MOS_BITFIELD_RANGE(16, 31);
        };
        uint32_t Value;
    } DW0;

    union {
        struct {
            uint32_t       Log2MaxCUSize                : MOS_BITFIELD_RANGE(0, 7);
            uint32_t       Log2MinCUSize                : MOS_BITFIELD_RANGE(8, 15);
            uint32_t       Log2MinTUSize                : MOS_BITFIELD_RANGE(16, 23);
            uint32_t       EnableIntraEarlyExit         : MOS_BITFIELD_BIT(24);
            uint32_t       Res_25_31                    : MOS_BITFIELD_RANGE(25, 31);
        };
        uint32_t Value;
    } DW1;

    union {
        struct {
            uint32_t       SliceType                    : MOS_BITFIELD_RANGE(0, 1);
            uint32_t       SimFlagForInter              : MOS_BITFIELD_BIT(2);
            uint32_t       FASTSurveillanceFlag         : MOS_BITFIELD_BIT(3);
            uint32_t       Res_4_31                     : MOS_BITFIELD_RANGE(4, 31);
        };
        uint32_t Value;
    } DW2;

    union {
        struct {
            uint32_t       Reserved;
        };
        uint32_t Value;
    } DW3;

    union {
        struct {
            uint32_t       Reserved;
        };
        uint32_t Value;
    } DW4;

    union {
        struct {
            uint32_t       Reserved;
        };
        uint32_t Value;
    } DW5;

    union {
        struct {
            uint32_t       Reserved;
        };
        uint32_t Value;
    } DW6;

    union {
        struct {
            uint32_t       Reserved;
        };
        uint32_t Value;
    } DW7;

    union {
        struct {
            uint32_t       BTI_Src_Y;
        };
        uint32_t Value;
    } DW8;

    union {
        struct {
            uint32_t       BTI_Sad_16x16_PU_Output;
        };
        uint32_t Value;
    } DW9;

    union {
        struct {
            uint32_t       BTI_32x32_Pu_ModeDecision;
        };
        uint32_t Value;
    } DW10;

    union {
        struct {
            uint32_t       BTI_Slice_Map;
        };
        uint32_t Value;
    } DW11;

    union {
        struct {
            uint32_t       BTI_Simplest_Intra;
        };
        uint32_t Value;
    } DW12;

    union {
        struct {
            uint32_t       BTI_Debug;
        };
        uint32_t Value;
    } DW13;
};

using PCODECHAL_ENC_HEVC_I_16x16_SAD_CURBE_G9 = struct CODECHAL_ENC_HEVC_I_16x16_SAD_CURBE_G9*;
C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(CODECHAL_ENC_HEVC_I_16x16_SAD_CURBE_G9)) == 14);

//! HEVC encoder intra 16x16 PU mode decision kernel curbe for GEN9
struct CODECHAL_ENC_HEVC_I_16x16_PU_MODEDECISION_CURBE_G9
{
    union {
        struct {
            uint32_t       FrameWidth                   : MOS_BITFIELD_RANGE(0, 15);
            uint32_t       FrameHeight                  : MOS_BITFIELD_RANGE(16, 31);
        };
        uint32_t Value;
    } DW0;

    union {
        struct {
            uint32_t       Log2MaxCUSize                : MOS_BITFIELD_RANGE(0, 7);
            uint32_t       Log2MinCUSize                : MOS_BITFIELD_RANGE(8, 15);
            uint32_t       Log2MinTUSize                : MOS_BITFIELD_RANGE(16, 23);
            uint32_t       SliceQp                      : MOS_BITFIELD_RANGE(24, 31);
        };
        uint32_t Value;
    } DW1;

    union {
        struct {
            uint32_t       FixedPoint_Lambda_PredMode;
        };
        uint32_t Value;
    } DW2;

    union {
        struct {
            uint32_t       LambdaScalingFactor          : MOS_BITFIELD_RANGE(0, 7);
            uint32_t       SliceType                    : MOS_BITFIELD_RANGE(8, 9);
            uint32_t       Reserved_10_15               : MOS_BITFIELD_RANGE(10, 15);
            uint32_t       IntraRefreshEn               : MOS_BITFIELD_RANGE(16, 17);
            uint32_t       EnableRollingIntra           : MOS_BITFIELD_BIT(18);
            uint32_t       HalfUpdateMixedLCU           : MOS_BITFIELD_BIT(19);
            uint32_t       Reserved_20_23               : MOS_BITFIELD_RANGE(20, 23);
            uint32_t       EnableIntraEarlyExit         : MOS_BITFIELD_BIT(24);
            uint32_t       BRCEnable                    : MOS_BITFIELD_BIT(25);
            uint32_t       LCUBRCEnable                 : MOS_BITFIELD_BIT(26);
            uint32_t       ROIEnable                    : MOS_BITFIELD_BIT(27);
            uint32_t       FASTSurveillanceFlag         : MOS_BITFIELD_BIT(28);
            uint32_t       EnableQualityImprovement     : MOS_BITFIELD_BIT(29);
            uint32_t       EnableFlexibleParam          : MOS_BITFIELD_BIT(30);
            uint32_t       Reserved_31                  : MOS_BITFIELD_BIT(31);
        };
        uint32_t Value;
    } DW3;

    union {
        struct {
            uint32_t       PenaltyForIntra8x8NonDCPredMode : MOS_BITFIELD_RANGE(0, 7);
            uint32_t       IntraComputeType             : MOS_BITFIELD_RANGE(8, 15);
            uint32_t       AVCIntra8x8Mask              : MOS_BITFIELD_RANGE(16, 23);
            uint32_t       IntraSadAdjust               : MOS_BITFIELD_RANGE(24, 31);
        };
        uint32_t Value;
    } DW4;

    union {
        struct {
            uint32_t       FixedPoint_Lambda_CU_Mode_for_Cost_Calculation;
        };
        uint32_t Value;
    } DW5;

    union {
        struct {
            uint32_t       ScreenContentFlag            : MOS_BITFIELD_BIT(0);
            uint32_t       Reserved                     : MOS_BITFIELD_RANGE(1, 31);
        };
        uint32_t Value;
    } DW6;

    union {
        struct {
            uint32_t       ModeCostIntraNonPred         : MOS_BITFIELD_RANGE(0, 7);
            uint32_t       ModeCostIntra16x16           : MOS_BITFIELD_RANGE(8, 15);
            uint32_t       ModeCostIntra8x8             : MOS_BITFIELD_RANGE(16, 23);
            uint32_t       ModeCostIntra4x4             : MOS_BITFIELD_RANGE(24, 31);
        };
        uint32_t Value;
    } DW7;

    union {
        struct {
            uint32_t       FixedPoint_Lambda_CU_Mode_for_Luma;
        };
        uint32_t Value;
    } DW8;

    union {
        struct {
            uint32_t       IntraRefreshMBNum            : MOS_BITFIELD_RANGE(0, 15);
            uint32_t       IntraRefreshUnitInMB         : MOS_BITFIELD_RANGE(16, 23);
            uint32_t       IntraRefreshQPDelta          : MOS_BITFIELD_RANGE(24, 31);
        };
        uint32_t Value;
    } DW9;

    union {
        struct {
            uint32_t       HaarTransformMode            : MOS_BITFIELD_RANGE(0, 1);
            uint32_t       SimplifiedFlagForInter       : MOS_BITFIELD_BIT(2);
            uint32_t       Reserved                     : MOS_BITFIELD_RANGE(3, 31);
        };
        uint32_t Value;
    } DW10;

    union {
        struct {
            uint32_t       Reserved;
        };
        uint32_t Value;
    } DW11;

    union {
        struct {
            uint32_t       Reserved;
        };
        uint32_t Value;
    } DW12;

    union {
        struct {
            uint32_t       Reserved;
        };
        uint32_t Value;
    } DW13;

    union {
        struct {
            uint32_t       Reserved;
        };
        uint32_t Value;
    } DW14;

    union {
        struct {
            uint32_t       Reserved;
        };
        uint32_t Value;
    } DW15;

    union {
        struct {
            uint32_t       BTI_Src_Y;
        };
        uint32_t Value;
    } DW16;

    union {
        struct {
            uint32_t       BTI_Sad_16x16_PU;
        };
        uint32_t Value;
    } DW17;

    union {
        struct {
            uint32_t       BTI_PAK_Object;
        };
        uint32_t Value;
    } DW18;

    union {
        struct {
            uint32_t       BTI_SAD_32x32_PU_mode;
        };
        uint32_t Value;
    } DW19;

    union {
        struct {
            uint32_t       BTI_VME_Mode_8x8;
        };
        uint32_t Value;
    } DW20;

    union {
        struct {
            uint32_t       BTI_Slice_Map;
        };
        uint32_t Value;
    } DW21;

    union {
        struct {
            uint32_t       BTI_VME_Src;
        };
        uint32_t Value;
    } DW22;

    union {
        struct {
            uint32_t       BTI_BRC_Input;
        };
        uint32_t Value;
    } DW23;

    union {
        struct {
            uint32_t       BTI_Simplest_Intra;
        };
        uint32_t Value;
    } DW24;

    union {
        struct {
            uint32_t       BTI_LCU_Qp_Surface;
        };
        uint32_t Value;
    } DW25;

    union {
        struct {
            uint32_t       BTI_BRC_Data;
        };
        uint32_t Value;
    } DW26;

    union {
        struct {
            uint32_t       BTI_Debug;
        };
        uint32_t Value;
    } DW27;
};

using PCODECHAL_ENC_HEVC_I_16x16_PU_MODEDECISION_CURBE_G9 = struct CODECHAL_ENC_HEVC_I_16x16_PU_MODEDECISION_CURBE_G9*;
C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(CODECHAL_ENC_HEVC_I_16x16_PU_MODEDECISION_CURBE_G9)) == 28);

//! HEVC encoder intra 8x8 PU kernel curbe for GEN9
struct CODECHAL_ENC_HEVC_I_8x8_PU_CURBE_G9
{
    union {
        struct {
            uint32_t       FrameWidth                   : MOS_BITFIELD_RANGE(0, 15);
            uint32_t       FrameHeight                  : MOS_BITFIELD_RANGE(16, 31);
        };
        uint32_t Value;
    } DW0;

    union {
        struct {
            uint32_t       SliceType                    : MOS_BITFIELD_RANGE(0, 1);
            uint32_t       PuType                       : MOS_BITFIELD_RANGE(2, 3);
            uint32_t       DcFilterFlag                 : MOS_BITFIELD_BIT(4);
            uint32_t       AngleRefineFlag              : MOS_BITFIELD_BIT(5);
            uint32_t       LCUType                      : MOS_BITFIELD_BIT(6);
            uint32_t       ScreenContentFlag            : MOS_BITFIELD_BIT(7);
            uint32_t       IntraRefreshEn               : MOS_BITFIELD_RANGE(8, 9);
            uint32_t       EnableRollingIntra           : MOS_BITFIELD_BIT(10);
            uint32_t       HalfUpdateMixedLCU           : MOS_BITFIELD_BIT(11);
            uint32_t       Reserved_12_15               : MOS_BITFIELD_RANGE(12, 15);
            uint32_t       QPValue                      : MOS_BITFIELD_RANGE(16, 23);
            uint32_t       EnableIntraEarlyExit         : MOS_BITFIELD_BIT(24);
            uint32_t       BRCEnable                    : MOS_BITFIELD_BIT(25);
            uint32_t       LCUBRCEnable                 : MOS_BITFIELD_BIT(26);
            uint32_t       ROIEnable                    : MOS_BITFIELD_BIT(27);
            uint32_t       FASTSurveillanceFlag         : MOS_BITFIELD_BIT(28);
            uint32_t       EnableFlexibleParam          : MOS_BITFIELD_BIT(29);
            uint32_t       EnableQualityImprovement     : MOS_BITFIELD_BIT(30);
            uint32_t       EnableDebugDump              : MOS_BITFIELD_BIT(31);
        };
        uint32_t Value;
    } DW1;

    union {
        struct {
            uint32_t       LumaLambda;
        };
        uint32_t Value;
    } DW2;

    union {
        struct {
            uint32_t       ChromaLambda;
        };
        uint32_t Value;
    } DW3;

    union {
        struct {
            uint32_t       HaarTransformFlag            : MOS_BITFIELD_RANGE(0, 1);
            uint32_t       SimplifiedFlagForInter       : MOS_BITFIELD_BIT(2);
            uint32_t       Reserved                     : MOS_BITFIELD_RANGE(3, 31);
        };
        uint32_t Value;
    } DW4;

    union {
        struct {
            uint32_t       IntraRefreshMBNum            : MOS_BITFIELD_RANGE(0, 15);
            uint32_t       IntraRefreshUnitInMB         : MOS_BITFIELD_RANGE(16, 23);
            uint32_t       IntraRefreshQPDelta          : MOS_BITFIELD_RANGE(24, 31);
        };
        uint32_t Value;
    } DW5;

    union {
        struct {
            uint32_t       Reserved;
        };
        uint32_t Value;
    } DW6;

    union {
        struct {
            uint32_t       Reserved;
        };
        uint32_t Value;
    } DW7;

    union {
        struct {
            uint32_t       BTI_Src_Y;
        };
        uint32_t Value;
    } DW8;

    union {
        struct {
            uint32_t       BTI_Slice_Map;
        };
        uint32_t Value;
    } DW9;

    union {
        struct {
            uint32_t       BTI_VME_8x8_Mode;
        };
        uint32_t Value;
    } DW10;

    union {
        struct {
            uint32_t       BTI_Intra_Mode;
        };
        uint32_t Value;
    } DW11;

    union {
        struct {
            uint32_t       BTI_BRC_Input;
        };
        uint32_t Value;
    } DW12;

    union {
        struct {
            uint32_t       BTI_Simplest_Intra;
        };
        uint32_t Value;
    } DW13;

    union {
        struct {
            uint32_t       BTI_LCU_Qp_Surface;
        };
        uint32_t Value;
    } DW14;

    union {
        struct {
            uint32_t       BTI_BRC_Data;
        };
        uint32_t Value;
    } DW15;

    union {
        struct {
            uint32_t       BTI_Debug;
        };
        uint32_t Value;
    } DW16;
};

using PCODECHAL_ENC_HEVC_I_8x8_PU_CURBE_G9 = struct CODECHAL_ENC_HEVC_I_8x8_PU_CURBE_G9*;
C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(CODECHAL_ENC_HEVC_I_8x8_PU_CURBE_G9)) == 17);

//! HEVC encoder intra 8x8 PU final mode decision kernel curbe for GEN9
struct CODECHAL_ENC_HEVC_I_8x8_PU_FMODE_CURBE_G9
{
    union {
        struct {
            uint32_t       FrameWidth                   : MOS_BITFIELD_RANGE(0, 15);
            uint32_t       FrameHeight                  : MOS_BITFIELD_RANGE(16, 31);
        };
        uint32_t Value;
    } DW0;

    union {
        struct {
            uint32_t       SliceType                    : MOS_BITFIELD_RANGE(0, 1);
            uint32_t       PuType                       : MOS_BITFIELD_RANGE(2, 3);
            uint32_t       PakReordingFlag              : MOS_BITFIELD_BIT(4);
            uint32_t       ReservedMBZ                  : MOS_BITFIELD_BIT(5);
            uint32_t       LCUType                      : MOS_BITFIELD_BIT(6);
            uint32_t       ScreenContentFlag            : MOS_BITFIELD_BIT(7);
            uint32_t       IntraRefreshEn               : MOS_BITFIELD_RANGE(8, 9);
            uint32_t       EnableRollingIntra           : MOS_BITFIELD_BIT(10);
            uint32_t       HalfUpdateMixedLCU           : MOS_BITFIELD_BIT(11);
            uint32_t       Reserved_12_23               : MOS_BITFIELD_RANGE(12, 23);
            uint32_t       EnableIntraEarlyExit         : MOS_BITFIELD_BIT(24);
            uint32_t       BRCEnable                    : MOS_BITFIELD_BIT(25);
            uint32_t       LCUBRCEnable                 : MOS_BITFIELD_BIT(26);
            uint32_t       ROIEnable                    : MOS_BITFIELD_BIT(27);
            uint32_t       FASTSurveillanceFlag         : MOS_BITFIELD_BIT(28);
            uint32_t       EnableFlexibleParam          : MOS_BITFIELD_BIT(29);
            uint32_t       EnableQualityImprovement     : MOS_BITFIELD_BIT(30);
            uint32_t       EnableDebugDump              : MOS_BITFIELD_BIT(31);
        };
        uint32_t Value;
    } DW1;

    union {
        struct {
            uint32_t       LambdaForLuma;
        };
        uint32_t Value;
    } DW2;

    union {
        struct {
            uint32_t       LambdaForDistCalculation;
        };
        uint32_t Value;
    } DW3;

    union {
        struct {
            uint32_t       ModeCostFor8x8PU_TU8;
        };
        uint32_t Value;
    } DW4;

    union {
        struct {
            uint32_t       ModeCostFor8x8PU_TU4;
        };
        uint32_t Value;
    } DW5;

    union {
        struct {
            uint32_t       SATD16x16PuThreshold         : MOS_BITFIELD_RANGE(0, 15);
            uint32_t       BiasFactorToward8x8          : MOS_BITFIELD_RANGE(16, 31);
        };
        uint32_t Value;
    } DW6;

    union {
        struct {
            uint32_t       Qp                           : MOS_BITFIELD_RANGE(0, 15);
            uint32_t       QpForInter                   : MOS_BITFIELD_RANGE(16, 31);
        };
        uint32_t Value;
    } DW7;

    union {
        struct {
            uint32_t       SimplifiedFlagForInter       : MOS_BITFIELD_BIT(0);
            uint32_t       EnableStatsDataDump          : MOS_BITFIELD_BIT(1);
            uint32_t       Reserved_2_7                 : MOS_BITFIELD_RANGE(2, 7);
            uint32_t       KBLControlFlag               : MOS_BITFIELD_BIT(8);
            uint32_t       Reserved_9_31                : MOS_BITFIELD_RANGE(9, 31);
        };
        uint32_t Value;
    } DW8;

    union {
        struct {
            uint32_t       IntraRefreshMBNum            : MOS_BITFIELD_RANGE(0, 15);
            uint32_t       IntraRefreshUnitInMB         : MOS_BITFIELD_RANGE(16, 23);
            uint32_t       IntraRefreshQPDelta          : MOS_BITFIELD_RANGE(24, 31);
        };
        uint32_t Value;
    } DW9;

    union {
        struct {
            uint32_t       Reserved;
        };
        uint32_t Value;
    } DW10;

    union {
        struct {
            uint32_t       Reserved;
        };
        uint32_t Value;
    } DW11;

    union {
        struct {
            uint32_t       Reserved;
        };
        uint32_t Value;
    } DW12;

    union {
        struct {
            uint32_t       Reserved;
        };
        uint32_t Value;
    } DW13;

    union {
        struct {
            uint32_t       Reserved;
        };
        uint32_t Value;
    } DW14;

    union {
        struct {
            uint32_t       Reserved;
        };
        uint32_t Value;
    } DW15;

    union {
        struct {
            uint32_t       BTI_PAK_Object;
        };
        uint32_t Value;
    } DW16;

    union {
        struct {
            uint32_t       BTI_VME_8x8_Mode;
        };
        uint32_t Value;
    } DW17;

    union {
        struct {
            uint32_t       BTI_Intra_Mode;
        };
        uint32_t Value;
    } DW18;

    union {
        struct {
            uint32_t       BTI_PAK_Command;
        };
        uint32_t Value;
    } DW19;

    union {
        struct {
            uint32_t       BTI_Slice_Map;
        };
        uint32_t Value;
    } DW20;

    union {
        struct {
            uint32_t       BTI_IntraDist;
        };
        uint32_t Value;
    } DW21;

    union {
        struct {
            uint32_t       BTI_BRC_Input;
        };
        uint32_t Value;
    } DW22;

    union {
        struct {
            uint32_t       BTI_Simplest_Intra;
        };
        uint32_t Value;
    } DW23;

    union {
        struct {
            uint32_t       BTI_LCU_Qp_Surface;
        };
        uint32_t Value;
    } DW24;

    union {
        struct {
            uint32_t       BTI_BRC_Data;
        };
        uint32_t Value;
    } DW25;

    union {
        struct {
            uint32_t       BTI_Haar_Dist16x16;
        };
        uint32_t Value;
    } DW26;

    union {
        struct {
            uint32_t       BTI_Stats_Data;
        };
        uint32_t Value;
    } DW27;

    union {
        struct {
            uint32_t       BTI_Frame_Stats_Data;
        };
        uint32_t Value;
    } DW28;

    union {
        struct {
            uint32_t       BTI_Debug;
        };
        uint32_t Value;
    } DW29;
};

using PCODECHAL_ENC_HEVC_I_8x8_PU_FMODE_CURBE_G9 = struct CODECHAL_ENC_HEVC_I_8x8_PU_FMODE_CURBE_G9*;
C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(CODECHAL_ENC_HEVC_I_8x8_PU_FMODE_CURBE_G9)) == 30);

//! HEVC encoder B 32x32 PU intra check kernel curbe for GEN9
struct CODECHAL_ENC_HEVC_B_32x32_PU_INTRA_CHECK_CURBE_G9
{
    union {
        struct {
            uint32_t       FrameWidth                   : MOS_BITFIELD_RANGE(0, 15);
            uint32_t       FrameHeight                  : MOS_BITFIELD_RANGE(16, 31);
        };
        uint32_t Value;
    } DW0;

    union {
        struct {
            uint32_t       SliceType                    : MOS_BITFIELD_RANGE(0, 1);
            uint32_t       Reserved                     : MOS_BITFIELD_RANGE(2, 7);
            uint32_t       Log2MinTUSize                : MOS_BITFIELD_RANGE(8, 15);
            uint32_t       Flags                        : MOS_BITFIELD_RANGE(16, 23);
            uint32_t       EnableIntraEarlyExit         : MOS_BITFIELD_BIT(24);
            uint32_t       HMEEnable                    : MOS_BITFIELD_BIT(25);
            uint32_t       FASTSurveillanceFlag         : MOS_BITFIELD_BIT(26);
            uint32_t       Res_27_30                    : MOS_BITFIELD_RANGE(27, 30);
            uint32_t       EnableDebugDump              : MOS_BITFIELD_BIT(31);
        };
        uint32_t Value;
    } DW1;

    union {
        struct {
            uint32_t       QpValue                      : MOS_BITFIELD_RANGE(0, 15);
            uint32_t       QpMultiplier                 : MOS_BITFIELD_RANGE(16, 31);
        };
        uint32_t Value;
    } DW2;

    union {
        struct {
            uint32_t       Reserved;
        };
        uint32_t Value;
    } DW3;

    union {
        struct {
            uint32_t       Reserved;
        };
        uint32_t Value;
    } DW4;

    union {
        struct {
            uint32_t       Reserved;
        };
        uint32_t Value;
    } DW5;

    union {
        struct {
            uint32_t       Reserved;
        };
        uint32_t Value;
    } DW6;

    union {
        struct {
            uint32_t       Reserved;
        };
        uint32_t Value;
    } DW7;

    union {
        struct {
            uint32_t       BTI_Per32x32PuIntraCheck;
        };
        uint32_t Value;
    } DW8;

    union {
        struct {
            uint32_t       BTI_Src_Y;
        };
        uint32_t Value;
    } DW9;

    union {
        struct {
            uint32_t       BTI_Src_Y2X;
        };
        uint32_t Value;
    } DW10;

    union {
        struct {
            uint32_t       BTI_Slice_Map;
        };
        uint32_t Value;
    } DW11;

    union {
        struct {
            uint32_t       BTI_VME_Y2X;
        };
        uint32_t Value;
    } DW12;

    union {
        struct {
            uint32_t       BTI_Simplest_Intra;   // output only
        };
        uint32_t Value;
    } DW13;

    union {
        struct {
            uint32_t       BTI_HME_MVPred;
        };
        uint32_t Value;
    } DW14;

    union {
        struct {
            uint32_t       BTI_HME_Dist;
        };
        uint32_t Value;
    } DW15;

    union {
        struct {
            uint32_t       BTI_LCU_Skip;
        };
        uint32_t Value;
    } DW16;

    union {
        struct {
            uint32_t       BTI_Debug;
        };
        uint32_t Value;
    } DW17;
};

using PCODECHAL_ENC_HEVC_B_32x32_PU_INTRA_CHECK_CURBE_G9 = struct CODECHAL_ENC_HEVC_B_32x32_PU_INTRA_CHECK_CURBE_G9;
C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(CODECHAL_ENC_HEVC_B_32x32_PU_INTRA_CHECK_CURBE_G9)) == 18);

//! HEVC encoder B Pak kernel curbe for GEN9
struct CODECHAL_ENC_HEVC_B_PAK_CURBE_G9
{
    union
    {
        struct
        {
            uint32_t   FrameWidth                       : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   FrameHeight                      : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   Qp                               : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   Res_8_15                         : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   MaxVmvR                          : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   SliceType                        : MOS_BITFIELD_RANGE(0, 1);
            uint32_t   EnableEmptyCURecordsDump         : MOS_BITFIELD_BIT(2);
            uint32_t   Res_3_7                          : MOS_BITFIELD_RANGE(3, 7);
            uint32_t   SimplestIntraEnable              : MOS_BITFIELD_BIT(8);
            uint32_t   BrcEnable                        : MOS_BITFIELD_BIT(9);
            uint32_t   LcuBrcEnable                     : MOS_BITFIELD_BIT(10);
            uint32_t   ROIEnable                        : MOS_BITFIELD_BIT(11);
            uint32_t   FASTSurveillanceFlag             : MOS_BITFIELD_BIT(12);
            uint32_t   EnableRollingIntra               : MOS_BITFIELD_BIT(13);
            uint32_t   Res_14                           : MOS_BITFIELD_BIT(14);
            uint32_t   EnableQualityImprovement         : MOS_BITFIELD_BIT(15);
            uint32_t   KBLControlFlag                   : MOS_BITFIELD_BIT(16);
            uint32_t   Res_17_30                        : MOS_BITFIELD_RANGE(17, 30);
            uint32_t   ScreenContent                    : MOS_BITFIELD_BIT(31);
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
            uint32_t   IntraRefreshMBNum                : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   IntraRefreshUnitInMB             : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   IntraRefreshQPDelta              : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t Reserved;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW4_15[12];

    union
    {
        struct
        {
            uint32_t  BTI_CU_Record;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW16;

    union
    {
        struct
        {
            uint32_t  BTI_PAK_Obj;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW17;

    union
    {
        struct
        {
            uint32_t  BTI_Slice_Map;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW18;

    union
    {
        struct
        {
            uint32_t  BTI_Brc_Input;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW19;

    union
    {
        struct
        {
            uint32_t  BTI_LCU_Qp;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW20;

    union
    {
        struct
        {
            uint32_t  BTI_Brc_Data;
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
            uint32_t  BTI_MB_Data;
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
            uint32_t  BTI_MVP_Surface;
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
            uint32_t  BTI_WA_PAK_Data;
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
            uint32_t  BTI_WA_PAK_Obj;
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
            uint32_t  BTI_Debug;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW26;

};

using PCODECHAL_ENC_HEVC_B_PAK_CURBE_G9 = struct CODECHAL_ENC_HEVC_B_PAK_CURBE_G9*;
C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(CODECHAL_ENC_HEVC_B_PAK_CURBE_G9)) == 27);

struct CODECHAL_ENC_HEVC_DS_COMBINED_CURBE_G9
{
    union {
        struct {
            uint32_t      Pak_BitDepth_Chroma              : MOS_BITFIELD_RANGE(0, 7);
            uint32_t      Pak_BitDepth_Luma                : MOS_BITFIELD_RANGE(8, 15);
            uint32_t      Enc_BitDepth_Chroma              : MOS_BITFIELD_RANGE(16, 23);
            uint32_t      Enc_BitDepth_Luma                : MOS_BITFIELD_RANGE(24, 30);
            uint32_t      Rounding_Value                   : MOS_BITFIELD_BIT(31);
        };

        uint32_t Value;
    } DW0;

    union {
        struct {
            uint32_t      PicFormat                         : MOS_BITFIELD_RANGE(0, 7);
            uint32_t      PicConvertFlag                    : MOS_BITFIELD_BIT(8);
            uint32_t      PicDownscale                      : MOS_BITFIELD_RANGE(9, 11);
            uint32_t      PicMBStatOutputCntrl              : MOS_BITFIELD_BIT(12);
            uint32_t      MBZ                               : MOS_BITFIELD_RANGE(13, 31);
        };
        uint32_t Value;
    } DW1;

    union {
        struct {
            uint32_t      OrigPicWidth                     : MOS_BITFIELD_RANGE(0, 15);
            uint32_t      OrigPicHeight                    : MOS_BITFIELD_RANGE(16, 31);
        };
        uint32_t Value;
    } DW2;

    union {
        struct {
            uint32_t      BTI_Surface_P010                 : MOS_BITFIELD_RANGE(0, 31);
        };
        uint32_t Value;
    } DW3;

    union {
        struct {
            uint32_t      BTI_Surface_NV12                 : MOS_BITFIELD_RANGE(0, 31);
        };
        uint32_t Value;
    } DW4;

    union {
        struct {
            uint32_t      BTI_Src_Y_4xDownScaled           : MOS_BITFIELD_RANGE(0, 31);
        };
        uint32_t Value;
    } DW5;

    union {
        struct {
            uint32_t      BTI_Surf_MBState                 : MOS_BITFIELD_RANGE(0, 31);
        };
        uint32_t Value;
    } DW6;

    union {
        struct {
            uint32_t      BTI_Src_Y_2xDownScaled           : MOS_BITFIELD_RANGE(0, 31);
        };
        uint32_t Value;
    } DW7;
};

using PCODECHAL_ENC_HEVC_DS_COMBINED_CURBE_G9 = struct CODECHAL_ENC_HEVC_DS_COMBINED_CURBE_G9*;
C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(CODECHAL_ENC_HEVC_DS_COMBINED_CURBE_G9)) == 8 );

//!  HEVC dual-pipe encoder class for GEN9
/*!
This class defines the member fields, functions for GEN9 platform
*/
class CodechalEncHevcStateG9 : public CodechalEncHevcState
{
protected:
    static constexpr uint32_t                   NUM_CONCURRENT_THREAD = 2;              //!< Number of concurrent threads
    static constexpr uint32_t                   MAX_NUM_KERNEL_SPLIT = 8;               //!< Maximal kernel split number
    static constexpr uint32_t                   BRC_CONSTANT_SURFACE_WIDTH = 64;        //!< BRC constant surface width        
    static constexpr uint32_t                   BRC_CONSTANT_SURFACE_HEIGHT= 53;        //!< BRC constant surface height
    static constexpr uint32_t                   BRC_HISTORY_BUFFER_SIZE = 576;          //!< BRC history buffer size
    
    //! Encoder surface index 
    enum SURFACE_ID
    {
        SURFACE_RAW_Y = 0,
        SURFACE_RAW_Y_UV,
        SURFACE_Y_2X,
        SURFACE_32x32_PU_OUTPUT,
        SURFACE_SLICE_MAP,
        SURFACE_Y_2X_VME,
        SURFACE_BRC_INPUT,
        SURFACE_LCU_QP,
        SURFACE_ROI,
        SURFACE_BRC_DATA,
        SURFACE_KERNEL_DEBUG,
        SURFACE_SIMPLIFIED_INTRA,
        SURFACE_HME_MVP,
        SURFACE_HME_DIST,
        SURFACE_16x16PU_SAD,
        SURFACE_RAW_VME,
        SURFACE_VME_8x8,
        SURFACE_CU_RECORD,
        SURFACE_INTRA_MODE,
        SURFACE_HCP_PAK,
        SURFACE_INTRA_DIST,
        SURFACE_MIN_DIST,
        SURFACE_VME_UNI_SIC_DATA,
        SURFACE_COL_MB_MV,
        SURFACE_CONCURRENT_THREAD,
        SURFACE_MB_MV_INDEX = SURFACE_CONCURRENT_THREAD + NUM_CONCURRENT_THREAD,
        SURFACE_MVP_INDEX,
        SURFACE_REF_FRAME_VME,
        SURFACE_Y_4X,
        SURFACE_Y_4X_VME,
        SURFACE_BRC_HISTORY,
        SURFACE_BRC_ME_DIST,
        SURFACE_BRC_PAST_PAK_INFO,
        SURFACE_BRC_HCP_PIC_STATE,
        SURFACE_RAW_10bit_Y,
        SURFACE_RAW_10bit_Y_UV,
        SURFACE_RAW_FC_8bit_Y,
        SURFACE_RAW_FC_8bit_Y_UV,
        SURFACE_RAW_MBSTAT,
        // Statistics output 
        SURFACE_PU_STATS,
        SURFACE_8X8_PU_HAAR_DIST,
        SURFACE_8X8_PU_FRAME_STATS,
        SURFACE_MB_ENC_STATS,
        SURFACE_MB_ENC_FRAME_STATS,
        // HEVC FEI
        SURFACE_FEI_EXTERNAL_MVP,
        SURFACE_FEI_PER_LCU_QP,
        SURFACE_FEI_PER_CTB_CTRL,
        SURFACE_FEI_CTB_DISTORTION,
        SURFACE_NUM_TOTAL
    };
    
    static const uint8_t                        m_ftqBasedSkip[NUM_TARGET_USAGE_MODES];   //!< FTP Skip LUT
    static const uint8_t                        m_meMethod[NUM_TARGET_USAGE_MODES];       //!< ME method LUT
    static const uint8_t                        m_superCombineDist[NUM_TARGET_USAGE_MODES + 1]; //!< m_superCombineDist LUT
    static const uint16_t                       m_skipValB[2][2][64];                 //!< Skip value LUT
    static const double                         m_modeCostLut[3][12];                 //!< Mode cost LUT
    static const double                         m_mvCostLut[3][8];                    //!< MV cost LUT
    static const uint32_t                       m_brcMvCostHaar[][416];               //!< BRC MV cost LUT
    static const uint32_t                       m_brcLambdaHaar[QP_NUM * 4];          //!< BRC lambda LUT
    static const uint16_t                       m_skipThread[][QP_NUM];               //!< Skip thread LUT
    static const double                         m_qpLambdaMdLut[3][QP_NUM];           //!< Mode decision lambda LUT
    static const double                         m_qpLambdaMeLut[3][QP_NUM];           //!< ME lambda LUT
    static const uint32_t                       m_encBTu1BCurbeInit[56];              //!< Initialization ENC curbe for TU1 B frame
    static const uint32_t                       m_encBTu4BCurbeInit[56];              //!< Initialization ENC curbe for TU4 B frame
    static const uint32_t                       m_encBTu7BCurbeInit[56];              //!< Initialization ENC curbe for TU7 B frame
    static const uint32_t                       m_encBTu1PCurbeInit[56];              //!< Initialization ENC curbe for TU1 P frame
    static const uint32_t                       m_encBTu4PCurbeInit[56];              //!< Initialization ENC curbe for TU4 P frame
    static const uint32_t                       m_encBTu7PCurbeInit[56];              //!< Initialization ENC curbe for TU7 P frame
    static const uint32_t                       m_encBTu7ICurbeInit[56];              //!< Initialization ENC curbe for TU1 I frame
    static const uint32_t                       m_brcInitCurbeInit[32];               //!< Initialization BRCInit curbe
    static const uint32_t                       m_brcUpdateCurbeInit[16];             //!< Initialization BRCUpdate curbe
    static const uint32_t                       m_meCurbeInit[39];                    //!< Initialization HME curbe

    PCODECHAL_SURFACE_CODEC_PARAMS              m_surfaceParams = nullptr;            //!< Pointer to surface parameters

    uint32_t                                    m_numMb8x8IntraKernelSplit = 0;       //!< MB 8x8 intra kernel region number
    uint32_t                                    m_numMbBKernelSplit = 0;              //!< Mb B kernel region number

    PCODECHAL_ENCODE_HEVC_SLICE_MAP             m_sliceMap = nullptr;                  //!< Pointer to HEVC slice map
    uint32_t                                    m_lastNumSlices = 0;                   //!< Number of slices from previous frame
    uint8_t                                     m_modeCost[14] = { 0 };                //!< Mode Cost
    uint8_t                                     m_modeCostSp = 0;                      //!< Mode Cost Sp
    uint8_t                                     m_mvCost[8] = { 0 };                   //!< Motion vector cost
    uint32_t                                    m_simplestIntraInterThreshold = 0;     //!< Intra/Inter threshold
    uint32_t                                    m_fixedPointLambda = 0;                //!< Fixed point lambda value
    uint32_t                                    m_fixedPointLambdaForLuma = 0;         //!< Fixed point lambda value for luma
    uint32_t                                    m_fixedPointLambdaForChroma = 0;       //!< Fixed point lambda value for chroma
    double                                      m_qpLambdaMd[3][QP_NUM] = { { 0.0 } }; //!< Mode decision lambda table
    double                                      m_qpLambdaMe[3][QP_NUM] = { { 0.0 } }; //!< Motion search lambda table 

    // Resources for the render engine
    MOS_SURFACE                                 m_scaled2xSurface;                       //!< 2x scaled surfaces
    MOS_SURFACE                                 m_sliceMapSurface;                       //!< Slice map surface
    CODECHAL_ENCODE_BUFFER                      m_32x32PuOutputData;                   //!< 32x32 PU output buffer
    CODECHAL_ENCODE_BUFFER                      m_sad16x16Pu;                          //!< SAD 16x16 PU buffer 
    CODECHAL_ENCODE_BUFFER                      m_vme8x8Mode;                          //!< VME 8x8 mode buffer
    CODECHAL_ENCODE_BUFFER                      m_intraMode;                           //!< Intra mode buffer
    CODECHAL_ENCODE_BUFFER                      m_intraDist;                           //!< Intra distortion buffer
    MOS_SURFACE                                 m_simplestIntraSurface;                  //!< Simplest intra surface
    MOS_SURFACE                                 m_roiSurface;                            //!< ROI surface
    MOS_SURFACE                                 m_concurrentThreadSurface[NUM_CONCURRENT_THREAD]; //!< Concurrent thread surface
    uint32_t                                    m_concurrentThreadIndex = 0;            //!< Concurrent thread index
    bool                                        m_powerSavingEnabled = false;            //!< Power saving enable flag
    CODECHAL_ENCODE_HEVC_WALKINGPATTERN_PARAM   m_walkingPatternParam;                    //!< WalkingPattern parameter
    MOS_SURFACE                                 m_minDistortion;                         //!< Min distortion surface
    CODECHAL_ENCODE_BUFFER                      m_vmeSavedUniSic;                      //!< VME UniSic buffer
    CODECHAL_ENCODE_BUFFER                      m_mvIndex;                             //!< MV index buffer
    CODECHAL_ENCODE_BUFFER                      m_mvpIndex;                            //!< MVP index buffer

    // Rolling I or intra refresh 
    bool                                        m_firstIntraRefresh = false;         //!< First intra fresh flag
    uint32_t                                    m_frameNumInGob = 0;                 //!< Frame number in GOP
    uint32_t                                    m_frameNumWithoutIntraRefresh = 0;   //!< Frame number without intra fresh

public:
    //!
    //! \brief    Constructor
    //!
    CodechalEncHevcStateG9(CodechalHwInterface* hwInterface,
        CodechalDebugInterface* debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo);

    //!
    //! \brief    Destructor
    //!
    ~CodechalEncHevcStateG9() {};

protected:

    // inherited virtual functions
    virtual MOS_STATUS Initialize(CodechalSetting * settings);
    MOS_STATUS InitKernelState();
    uint32_t GetMaxBtCount();
    MOS_STATUS EncodeKernelFunctions();
    MOS_STATUS AllocateEncResources();
    MOS_STATUS FreeEncResources();
    MOS_STATUS SetSliceStructs();
    MOS_STATUS UserFeatureKeyReport();
    MOS_STATUS InitSurfaceInfoTable();
    MOS_STATUS SetSequenceStructs();
    MOS_STATUS SetPictureStructs();
    MOS_STATUS CalcScaledDimensions();
    void GetMaxRefFrames(uint8_t& maxNumRef0, uint8_t& maxNumRef1);

    //!
    //! \brief    Set ME kenrel surface state
    //!
    //! \param    [in] hwInterface
    //!           Pointer to HW interface
    //! \param    [in] cmdBuffer
    //!           Pointer to command buffer
    //! \param    [in] params
    //!           Pointer to ME surface parameters
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SendMeSurfaces(
        CodechalHwInterface         *hwInterface, 
        PMOS_COMMAND_BUFFER         cmdBuffer, 
        MeSurfaceParams             *params);

    //!
    //! \brief    Initialize MHW
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS InitMhw();

    //! \brief    Generate walking control region
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS GenerateWalkingControlRegion();

    //!
    //! \brief    Initialize media walker parameters for 26Z walking pattern
    //!
    //! \param    [in] numRegionInSlice
    //!           Number of Region
    //! \param    [in] maxSliceHeight
    //!           Max slice height
    //!
    //! \return   void
    //!
    void InitParamForWalkerVfe26z(uint32_t numRegionInSlice, uint32_t maxSliceHeight);

    //!
    //! \brief    Initialize media walker parameters for 26 walking pattern
    //!
    //! \param    [in] numRegionInSlice
    //!           Number of Region
    //! \param    [in] maxSliceHeight
    //!           Max slice height
    //!
    //! \return   void
    //!
    void InitParamForWalkerVfe26(uint32_t numRegionInSlice, uint32_t maxSliceHeight);

    //!
    //! \brief    Request surface state heap(SSH) and validate command buffer size
    //!
    //! \param    [in]  kernelState
    //!           Pointer to kernel state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS RequestSshAndVerifyCommandBufferSize(PMHW_KERNEL_STATE kernelState);

    //!
    //! \brief    Send generic kernel commands and set binding table
    //!
    //! \param    [in]  cmdBuffer
    //!           Pointer to command buffer
    //! \param    [in]  kernelState
    //!           Pointer to kernel state
    //! \param    [in]  mediaStateType
    //!           Media state type
    //! \param    [in]  customScoreBoard
    //!           Pointer to custom score board setting
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SendKernelCmdsAndBindingTable(
        PMOS_COMMAND_BUFFER                     cmdBuffer,
        PMHW_KERNEL_STATE                       kernelState,
        CODECHAL_MEDIA_STATE_TYPE               mediaStateType,
        PMHW_VFE_SCOREBOARD                     customScoreBoard);

    //!
    //! \brief    Add curbe data to dynamic state heap
    //!
    //! \param    [in]  kernelState
    //!           Pointer to kernel state
    //! \param    [in]  mediaStateType
    //!           Media state type
    //! \param    [in]  curbe
    //!           Pointer to curbe data
    //! \param    [in]  curbeSize
    //!           Curbe data size
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AddCurbeToStateHeap(
        PMHW_KERNEL_STATE               kernelState,
        CODECHAL_MEDIA_STATE_TYPE       mediaStateType,
        void*                           curbe,
        uint32_t                        curbeSize);

    //!
    //! \brief    End kernel function and submit command buffer
    //!
    //! \param    [in]  mediaStateType
    //!           Media state type
    //! \param    [in]  kernelState
    //!           Pointer to kernel state
    //! \param    [in]  cmdBuffer
    //!           Pointer to command buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS EndKernelCall(
        CODECHAL_MEDIA_STATE_TYPE       mediaStateType,
        PMHW_KERNEL_STATE               kernelState,
        PMOS_COMMAND_BUFFER             cmdBuffer);

    //!
    //! \brief    Initialize MbEnc kernel state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS InitKernelStateMbEnc();

    //!
    //! \brief    Initialize BRC kernel state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS InitKernelStateBrc();

    //!
    //! \brief    Set MbEnc kernel parameters
    //!
    //! \param    [in]  kernelParams
    //!           Pointer to kernel parameters
    //! \param    [in]  idx
    //!           MbEnc kernel index
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetMbEncKernelParams(MHW_KERNEL_PARAM* kernelParams, uint32_t idx);

    //!
    //! \brief    Set BRC kernel parameters
    //!
    //! \param    [in]  kernelParams
    //!           Pointer to kernel parameters
    //! \param    [in]  idx
    //!           BRC kernel index
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetBrcKernelParams(MHW_KERNEL_PARAM* kernelParams, uint32_t idx);

    //!
    //! \brief    Set MbEnc kernel binding table
    //!
    //! \param    [in]  bindingTable
    //!           Pointer to binding table
    //! \param    [in]  idx
    //!           MbEnc kernel index
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetMbEncBindingTable(PCODECHAL_ENCODE_BINDING_TABLE_GENERIC bindingTable, uint32_t idx);

    //!
    //! \brief    Set BRC kernel binding table
    //!
    //! \param    [in]  bindingTable
    //!           Pointer to binding table
    //! \param    [in]  idx
    //!           BRC kernel index
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetBrcBindingTable(PCODECHAL_ENCODE_BINDING_TABLE_GENERIC bindingTable, uint32_t idx);

    //!
    //! \brief    Get MbEnc B kernel default setting
    //!
    //! \param    [in, out]  curbeSize
    //!           Curbe data size
    //!
    //! \return   uint32_t*
    //!           Pointer to MbEnc B kernel default curbe setting
    //!
    uint32_t* GetDefaultCurbeEncBKernel(uint32_t& curbeSize);

    //!
    //! \brief    Get reference picture QP value
    //!
    //! \param    [in]  list
    //!           Reference list number
    //! \param    [in]  index
    //!           Reference picture index
    //!
    //! \return   QP value of specified reference picture
    //!
    uint8_t GetQPValueFromRefList(uint32_t list, uint32_t index);

    //!
    //! \brief    Invoke down scaling kernel
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS EncodeDSKernel();

    //!
    //! \brief    Invoke BRC Init/Reset kernel
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS EncodeBrcInitResetKernel();

    //!
    //! \brief    Invoke BRC update kernel
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS EncodeBrcUpdateKernel();

    //!
    //! \brief    Invoke BRC update (LCU based) kernel
    //!
    //! \param    [in]  curbe
    //!           Pointer to BRC_UPDATE curbe structure
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS EncodeBrcUpdateLCUBasedKernel(struct CODECHAL_ENC_HEVC_BRC_UPDATE_CURBE_G9* curbe);

    //!
    //! \brief    Invoke coarse intra16x16 kernel
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS EncodeCoarseIntra16x16Kernel();

    //!
    //! \brief    Invoke 8x8 B MbEnc kernel
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Encode8x8PBMbEncKernel();

    //!
    //! \brief    Invoke 8x8 B Pak kernel
    //!
    //! \param    [in]  curbe
    //!           Pointer to B_MB_ENC curbe structure
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Encode8x8BPakKernel(struct CODECHAL_ENC_HEVC_B_MB_ENC_CURBE_G9* curbe);

    //!
    //! \brief    Invoke 2x down scaling kernel
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Encode2xScalingKernel();

    //!
    //! \brief    Invoke 32x32 PU mode decision kernel
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Encode32x32PuModeDecisionKernel();

    //!
    //! \brief    Invoke 32x32 B intra check kernel
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Encode32X32BIntraCheckKernel();

    //!
    //! \brief    Invoke 16x16 PU SAD computation kernel
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Encode16x16SadPuComputationKernel();

    //!
    //! \brief    Invoke 16x16 PU mode decision kernel
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Encode16x16PuModeDecisionKernel();

    //!
    //! \brief    Invoke 8x8 PU kernel
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Encode8x8PUKernel();

    //!
    //! \brief    Invoke 8x8 PU FMode kernel
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Encode8x8PUFMODEKernel();

    //!
    //! \brief   Invoke down scale and format conversion kernel
    //!
    //! \param    [in]  downScaleStage
    //!           Down scale stage
    //! \param    [in]  index
    //!           Index to formatConvertedSurface array
    //! \param    [in]  refListIdx
    //!           Reference list index
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS EncodeDSCombinedKernel(
        DsStage downScaleStage,
        uint32_t index,
        uint32_t refListIdx);
       
    //!
    //! \brief    Convert picture coding type to HEVC frame type
    //!
    //! \param    [in]  picType
    //!           Picture coding type
    //!
    //! \return   HEVC frame type
    //!
    uint32_t PicCodingTypeToFrameType(uint32_t picType);

    //!
    //! \brief    Calculate lambda value
    //!
    //! \param    [in]  sliceType
    //!           Slice Type
    //! \param    [in]  intraSADTransform
    //!           Intra SAD transform type
    //!
    //! \return   void
    //!
    void CalcLambda(uint8_t sliceType, uint8_t intraSADTransform);

    //!
    //! \brief    Load cost table
    //!
    //! \param    [in]  sliceType
    //!           Slice Type
    //! \param    [in]  qp
    //!           QP value
    //! \param    [in]  intraSADTransform
    //!           Intra SAD transform type
    //!
    //! \return   void
    //!
    void LoadCosts(uint8_t sliceType, uint8_t qp, uint8_t intraSADTransform);

    //!
    //! \brief    Calcuate forward transform coefficient threshold
    //!
    //! \param    [in, out] forwardCoeffThresh
    //!           Forward transform coefficient threshold 
    //! \param    [in]  qp
    //!           QP value
    //!
    //! \return   void
    //!
    void CalcForwardCoeffThd(uint8_t* forwardCoeffThresh, int32_t qp);

    //!
    //! \brief    Add checking BRC PAK statistics command to command buffer
    //!
    //! \param    [in]  cmdBuffer
    //!           Pointer to command buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS CheckBrcPakStasBuffer(PMOS_COMMAND_BUFFER cmdBuffer);

    //!
    //! \brief    Setup ROI surface
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetupROISurface();

    //!
    //! \brief    Setup ROI in BRC_UPDATE curbe 
    //!
    //! \param    [in]  curbe
    //!           Pointer to BRC_UPDATE curbe structure
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetupROICurbe(struct CODECHAL_ENC_HEVC_BRC_UPDATE_CURBE_G9* curbe);

    //!
    //! \brief    Setup BRC constant data 
    //!
    //! \param    [in, out]  brcConstantData
    //!           Pointer to BRC constant data surface
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetupBrcConstantTable(PMOS_SURFACE brcConstantData);

    //!
    //! \brief    Convert input 1 byte QP per LCU to 2 bytes per LCU
    //!
    //! \param    [in]  lcuQPIn
    //!           Pointer to input LCU QP data surface
    //! \param    [out]  lcuQPOut
    //!           Pointer to output LCU QP data surface
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Convert1byteTo2bytesQPperLCU(PMOS_SURFACE lcuQPIn, PMOS_SURFACE lcuQPOut);
    //!
    //! \brief    Generate slice map
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS GenerateSliceMap();

    //!
    //! \brief    Help function to set up surface state 
    //!
    //! \param    [in]  kernelState
    //!           Pointer to kernel state
    //! \param    [in]  cmdBuffer
    //!           Pointer to command buffer
    //! \param    [in]  surfaceId
    //!           Surface index
    //! \param    [in]  bindingTableOffset
    //!           Pointer to binding table offset
    //! \param    [in]  addr
    //!           Surface address (can be nullptr)
    //! \param    [in]  width
    //!           Width to be used of the Surface (can be 0, then the surface state would use surface width by default).
    //! \param    [in]  height
    //!           Height to be used of the Surface (can be 0, then the surface state would use surface height by default).
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetSurfacesState(
        PMHW_KERNEL_STATE kernelState,
        PMOS_COMMAND_BUFFER cmdBuffer,
        SURFACE_ID surfaceId,
        uint32_t *bindingTableOffset,
        void* addr = nullptr,
        uint32_t width = 0,
        uint32_t height = 0);

    //!
    //! \brief    Convert picture coding type to slice type
    //!
    //! \param    [in] pictureCodingType
    //!           Picture coding type
    //!
    //! \return   HEVC slice type
    //!
    uint8_t PicCodingTypeToSliceType(uint16_t pictureCodingType);

    //!
    //! \brief    Help function to convert kernel operation function to media state 
    //!
    //! \param    [in]  op
    //!           Kernel operation function
    //! \param    [in]  idx
    //!           Kernel state index
    //!
    //! \return   Kernel media state value
    //!
    inline uint32_t ConvertKrnOpsToMediaState(EncOperation op, uint32_t idx)
    {
        return ((uint32_t)op << 16) | idx;
    }

    //!
    //! \brief    Check if specific platform control flag needs to enabled for kernel setting
    //!
    //! \return   Value of platform control flag
    //!
    virtual bool UsePlatformControlFlag() = 0;

    //! \brief    Program Curbe for ME kernel
    //!
    //! \param    [in] params
    //!           Input params to program ME curbe 
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetCurbeMe(MeCurbeParams* params);
};

//! \brief  typedef of class CodechalEncHevcStateG9*
using PCODECHAL_ENC_HEVC_STATE_G9 = class CodechalEncHevcStateG9*;

#endif  // __CODECHAL_ENCODE_HEVC_G9_H__
