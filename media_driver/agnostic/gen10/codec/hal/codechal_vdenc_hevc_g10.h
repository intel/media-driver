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
//! \file     codechal_vdenc_hevc_g10.h
//! \brief    HEVC VDEnc encoder for GEN10 platform.
//!

#ifndef __CODECHAL_VDENC_HEVC_G10_H__
#define __CODECHAL_VDENC_HEVC_G10_H__

#include "codechal_vdenc_hevc.h"
#include "codechal_debug_encode_par_g10.h"

enum CODECHAL_BINDING_TABLE_OFFSET_HEVC_VP9_VDENC_KERNEL_G10
{
    // VDEnc HME kernel
    CODECHAL_VDENC_HME_BEGIN_G10 = 0,
    CODECHAL_VDENC_HME_MV_DATA_SURFACE_CM_G10 = CODECHAL_VDENC_HME_BEGIN_G10,
    CODECHAL_VDENC_16xME_MV_DATA_SURFACE_CM_G10,
    CODECHAL_VDENC_32xME_MV_DATA_SURFACE_CM_G10 = CODECHAL_VDENC_16xME_MV_DATA_SURFACE_CM_G10,
    CODECHAL_VDENC_HME_DISTORTION_SURFACE_CM_G10,
    CODECHAL_VDENC_HME_BRC_DISTORTION_CM_G10,
    CODECHAL_VDENC_HME_CURR_FOR_FWD_REF_CM_G10,
    CODECHAL_VDENC_HME_FWD_REF_IDX0_CM_G10,
    CODECHAL_VDENC_HME_RESERVED1_CM_G10,
    CODECHAL_VDENC_HME_FWD_REF_IDX1_CM_G10,
    CODECHAL_VDENC_HME_RESERVED2_CM_G10,
    CODECHAL_VDENC_HME_FWD_REF_IDX2_CM_G10,
    CODECHAL_VDENC_HME_RESERVED3_CM_G10,
    CODECHAL_VDENC_HME_FWD_REF_IDX3_CM_G10,
    CODECHAL_VDENC_HME_RESERVED4_CM_G10,
    CODECHAL_VDENC_HME_FWD_REF_IDX4_CM_G10,
    CODECHAL_VDENC_HME_RESERVED5_CM_G10,
    CODECHAL_VDENC_HME_FWD_REF_IDX5_CM_G10,
    CODECHAL_VDENC_HME_RESERVED6_CM_G10,
    CODECHAL_VDENC_HME_FWD_REF_IDX6_CM_G10,
    CODECHAL_VDENC_HME_RESERVED7_CM_G10,
    CODECHAL_VDENC_HME_FWD_REF_IDX7_CM_G10,
    CODECHAL_VDENC_HME_RESERVED8_CM_G10,
    CODECHAL_VDENC_HME_CURR_FOR_BWD_REF_CM_G10,
    CODECHAL_VDENC_HME_BWD_REF_IDX0_CM_G10,
    CODECHAL_VDENC_HME_RESERVED9_CM_G10,
    CODECHAL_VDENC_HME_BWD_REF_IDX1_CM_G10,
    CODECHAL_VDENC_HME_RESERVED10_CM_G10,
    CODECHAL_VDENC_HME_VDENC_STREAMIN_OUTPUT_CM_G10,
    CODECHAL_VDENC_HME_VDENC_STREAMIN_INPUT_CM_G10,
    CODECHAL_VDENC_HME_END_G10,
};

struct MEDIA_OBJECT_HEVC_VP9_VDENC_ME_CURBE_G10
{
    // DW0
    union
    {
        struct
        {
            uint32_t   SkipModeEn : MOS_BITFIELD_BIT(0);
            uint32_t   AdaptiveEn : MOS_BITFIELD_BIT(1);
            uint32_t   BiMixDis : MOS_BITFIELD_BIT(2);
        uint32_t: MOS_BITFIELD_RANGE(3, 4);
            uint32_t   EarlyImeSuccessEn : MOS_BITFIELD_BIT(5);
        uint32_t: MOS_BITFIELD_BIT(6);
            uint32_t   T8x8FlagForInterEn : MOS_BITFIELD_BIT(7);
        uint32_t: MOS_BITFIELD_RANGE(8, 23);
            uint32_t   EarlyImeStop : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   MaxNumMVs : MOS_BITFIELD_RANGE(0, 5);
        uint32_t: MOS_BITFIELD_RANGE(6, 15);
            uint32_t   BiWeight : MOS_BITFIELD_RANGE(16, 21);
        uint32_t: MOS_BITFIELD_RANGE(22, 27);
            uint32_t   UniMixDisable : MOS_BITFIELD_BIT(28);
        uint32_t: MOS_BITFIELD_RANGE(29, 31);
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
            uint32_t   MaxLenSP : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   MaxNumSU : MOS_BITFIELD_RANGE(8, 15);
        uint32_t: MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   SrcSize : MOS_BITFIELD_RANGE(0, 1);
        uint32_t: MOS_BITFIELD_RANGE(2, 3);
            uint32_t   MbTypeRemap : MOS_BITFIELD_RANGE(4, 5);
            uint32_t   SrcAccess : MOS_BITFIELD_BIT(6);
            uint32_t   RefAccess : MOS_BITFIELD_BIT(7);
            uint32_t   SearchCtrl : MOS_BITFIELD_RANGE(8, 10);
            uint32_t   DualSearchPathOption : MOS_BITFIELD_BIT(11);
            uint32_t   SubPelMode : MOS_BITFIELD_RANGE(12, 13);
            uint32_t   SkipType : MOS_BITFIELD_BIT(14);
            uint32_t   DisableFieldCacheAlloc : MOS_BITFIELD_BIT(15);
            uint32_t   InterChromaMode : MOS_BITFIELD_BIT(16);
            uint32_t   FTEnable : MOS_BITFIELD_BIT(17);
            uint32_t   BMEDisableFBR : MOS_BITFIELD_BIT(18);
            uint32_t   BlockBasedSkipEnable : MOS_BITFIELD_BIT(19);
            uint32_t   InterSAD : MOS_BITFIELD_RANGE(20, 21);
            uint32_t   IntraSAD : MOS_BITFIELD_RANGE(22, 23);
            uint32_t   SubMbPartMask : MOS_BITFIELD_RANGE(24, 30);
        uint32_t: MOS_BITFIELD_BIT(31);
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
        uint32_t: MOS_BITFIELD_RANGE(0, 7);
            uint32_t   PictureHeightMinus1 : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   PictureWidth : MOS_BITFIELD_RANGE(16, 23);
        uint32_t: MOS_BITFIELD_RANGE(24, 31);
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
        uint32_t: MOS_BITFIELD_RANGE(0, 7);
            uint32_t   QpPrimeY : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   RefWidth : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   RefHeight : MOS_BITFIELD_RANGE(24, 31);
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
        uint32_t: MOS_BITFIELD_BIT(0);
            uint32_t   InputStreamInEn : MOS_BITFIELD_BIT(1);
            uint32_t   LCUSize : MOS_BITFIELD_BIT(2);
            uint32_t   WriteDistortions : MOS_BITFIELD_BIT(3);
            uint32_t   UseMvFromPrevStep : MOS_BITFIELD_BIT(4);
        uint32_t: MOS_BITFIELD_RANGE(5, 7);
            uint32_t   SuperCombineDist : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   MaxVmvR : MOS_BITFIELD_RANGE(16, 31);
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
        uint32_t: MOS_BITFIELD_RANGE(0, 15);
            uint32_t   MVCostScaleFactor : MOS_BITFIELD_RANGE(16, 17);
            uint32_t   BilinearEnable : MOS_BITFIELD_BIT(18);
            uint32_t   SrcFieldPolarity : MOS_BITFIELD_BIT(19);
            uint32_t   WeightedSADHAAR : MOS_BITFIELD_BIT(20);
            uint32_t   AConlyHAAR : MOS_BITFIELD_BIT(21);
            uint32_t   RefIDCostMode : MOS_BITFIELD_BIT(22);
        uint32_t: MOS_BITFIELD_BIT(23);
            uint32_t   SkipCenterMask : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   Mode0Cost : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   Mode1Cost : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   Mode2Cost : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   Mode3Cost : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   Mode4Cost : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   Mode5Cost : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   Mode6Cost : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   Mode7Cost : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   Mode8Cost : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   Mode9Cost : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   RefIDCost : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   ChromaIntraModeCost : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   MV0Cost : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   MV1Cost : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   MV2Cost : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   MV3Cost : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   MV4Cost : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   MV5Cost : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   MV6Cost : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   MV7Cost : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   NumRefIdxL0MinusOne : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   NumRefIdxL1MinusOne : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   RefStreaminCost : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   ROIEnable : MOS_BITFIELD_RANGE(24, 26);
        uint32_t: MOS_BITFIELD_RANGE(27, 31);
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
            uint32_t   List0RefID0FieldParity : MOS_BITFIELD_BIT(0);
            uint32_t   List0RefID1FieldParity : MOS_BITFIELD_BIT(1);
            uint32_t   List0RefID2FieldParity : MOS_BITFIELD_BIT(2);
            uint32_t   List0RefID3FieldParity : MOS_BITFIELD_BIT(3);
            uint32_t   List0RefID4FieldParity : MOS_BITFIELD_BIT(4);
            uint32_t   List0RefID5FieldParity : MOS_BITFIELD_BIT(5);
            uint32_t   List0RefID6FieldParity : MOS_BITFIELD_BIT(6);
            uint32_t   List0RefID7FieldParity : MOS_BITFIELD_BIT(7);
            uint32_t   List1RefID0FieldParity : MOS_BITFIELD_BIT(8);
            uint32_t   List1RefID1FieldParity : MOS_BITFIELD_BIT(9);
        uint32_t: MOS_BITFIELD_RANGE(10, 31);
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
            uint32_t   PrevMvReadPosFactor : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   MvShiftFactor : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   Reserved : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   ActualMBWidth : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   ActualMBHeight : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   RoiCtrl : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   MaxTuSize : MOS_BITFIELD_RANGE(8, 9);
            uint32_t   MaxCuSize : MOS_BITFIELD_RANGE(10, 11);
            uint32_t   NumImePredictors : MOS_BITFIELD_RANGE(12, 15);
            uint32_t   Reserved : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   PuTypeCtrl : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   ForceMvx0 : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   ForceMvy0 : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   ForceMvx1 : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   ForceMvy1 : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   ForceMvx2 : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   ForceMvy2 : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   ForceMvx3 : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   ForceMvy3 : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   ForceRefIdx0 : MOS_BITFIELD_RANGE(0, 3);
            uint32_t   ForceRefIdx1 : MOS_BITFIELD_RANGE(4, 7);
            uint32_t   ForceRefIdx2 : MOS_BITFIELD_RANGE(8, 11);
            uint32_t   ForceRefIdx3 : MOS_BITFIELD_RANGE(12, 15);
            uint32_t   NumMergeCandCu8x8 : MOS_BITFIELD_RANGE(16, 19);
            uint32_t   NumMergeCandCu16x16 : MOS_BITFIELD_RANGE(20, 23);
            uint32_t   NumMergeCandCu32x32 : MOS_BITFIELD_RANGE(24, 27);
            uint32_t   NumMergeCandCu64x64 : MOS_BITFIELD_RANGE(28, 31);
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
            uint32_t   SegID : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   QpEnable : MOS_BITFIELD_RANGE(16, 19);
            uint32_t   SegIDEnable : MOS_BITFIELD_BIT(20);
            uint32_t   Reserved : MOS_BITFIELD_RANGE(21, 22);
            uint32_t   ForceRefIdEnable : MOS_BITFIELD_BIT(23);
            uint32_t   Reserved1 : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   ForceQp0 : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   ForceQp1 : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   ForceQp2 : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   ForceQp3 : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   Reserved : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   _4xMeMvOutputDataSurfIndex : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   _16xOr32xMeMvInputDataSurfIndex : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   _4xMeOutputDistSurfIndex : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   _4xMeOutputBrcDistSurfIndex : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   VMEFwdInterPredictionSurfIndex : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   VMEBwdInterPredictionSurfIndex : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   VDEncStreamInOutputSurfIndex : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   VDEncStreamInInputSurfIndex : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW47;
};
C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(MEDIA_OBJECT_HEVC_VP9_VDENC_ME_CURBE_G10)) == 48);

struct CODECHAL_VDENC_HEVC_STREAMIN_STATE_G10
{
    // DWORD 0
    union {
        struct {
            uint32_t       RoiCtrl                              : MOS_BITFIELD_RANGE(  0,7 );
            uint32_t       MaxTuSize                            : MOS_BITFIELD_RANGE(  8,9 );
            uint32_t       MaxCuSize                            : MOS_BITFIELD_RANGE(  10,11 );
            uint32_t       NumImePredictors                     : MOS_BITFIELD_RANGE(  12,15 );
            uint32_t       Reserved_0                           : MOS_BITFIELD_RANGE(  16,23 );
            uint32_t       PuTypeCtrl                           : MOS_BITFIELD_RANGE(  24,31 );
        };
        uint32_t Value;
    } DW0;

    // DWORD 1-4
    union {
        struct {
            uint32_t       ForceMvX                            : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t       ForceMvY                            : MOS_BITFIELD_RANGE(  16,31 );
        };
        uint32_t Value;
    } DW1[4];

    // DWORD 5
    union {
        struct {
            uint32_t       Reserved                             : MOS_BITFIELD_RANGE(  0,31 );
        };
        uint32_t Value;
    } DW5;

    // DWORD 6
    union {
        struct {
            uint32_t       ForceRefIdx                          : MOS_BITFIELD_RANGE(  0,15 ); //4-bits per 16x16 block
            uint32_t       NumMergeCandidateCu8x8               : MOS_BITFIELD_RANGE(  16,19 );
            uint32_t       NumMergeCandidateCu16x16             : MOS_BITFIELD_RANGE(  20,23 );
            uint32_t       NumMergeCandidateCu32x32             : MOS_BITFIELD_RANGE(  24,27 );
            uint32_t       NumMergeCandidateCu64x64             : MOS_BITFIELD_RANGE(  28,31 );
        };
        uint32_t Value;
    } DW6;

    // DWORD 7
    union {
        struct {
            uint32_t       SegID                                : MOS_BITFIELD_RANGE(  0,15 ); //4-bits per 16x16 block
            uint32_t       QpEnable                             : MOS_BITFIELD_RANGE(  16,19 );
            uint32_t       SegIDEnable                          : MOS_BITFIELD_RANGE(  20,20 );
            uint32_t       Reserved                             : MOS_BITFIELD_RANGE(  21,22 );
            uint32_t       ForceRefIdEnable                     : MOS_BITFIELD_RANGE(  23,23 );
            uint32_t       ImePredictorSelect                   : MOS_BITFIELD_RANGE(  24,31 );
        };
        uint32_t Value;
    } DW7;

    // DWORD 8-11
    union {
        struct {
            uint32_t       ImePredictorMvX                      : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t       ImePredictorMvY                      : MOS_BITFIELD_RANGE(  16,31 );
        };
        uint32_t Value;
    } DW8[4];

    // DWORD 12
    union {
        struct {
            uint32_t       ImePredictorRefIdx                   : MOS_BITFIELD_RANGE(  0,15 ); //4-bits per 16x16 block
            uint32_t       Reserved                             : MOS_BITFIELD_RANGE(  16,31 );
        };
        uint32_t Value;
    } DW12;

    // DWORD 13
    union {
        struct {
            uint32_t       PanicModeLCUThreshold                : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t       Reserved                             : MOS_BITFIELD_RANGE(  16,31 );
        };
        uint32_t Value;
    } DW13;

    // DWORD 14
    union {
        struct {
            uint32_t       ForceQp_0                            : MOS_BITFIELD_RANGE(  0,7 );
            uint32_t       ForceQp_1                            : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t       ForceQp_2                            : MOS_BITFIELD_RANGE(  16,23 );
            uint32_t       ForceQp_3                            : MOS_BITFIELD_RANGE(  24,31 );
        };
        uint32_t Value;
    } DW14;

    // DWORD 15
    union {
        struct {
            uint32_t       Reserved                             : MOS_BITFIELD_RANGE(  0,31 );
        };
        uint32_t Value;
    } DW15;
};
C_ASSERT(SIZE32(CODECHAL_VDENC_HEVC_STREAMIN_STATE_G10) == 16);

using PCODECHAL_VDENC_HEVC_STREAMIN_STATE_G10 = CODECHAL_VDENC_HEVC_STREAMIN_STATE_G10*;

struct CODECHAL_VDENC_HEVC_ME_CURBE_G10
{
    // DW0
    union
    {
        struct
        {
            uint32_t   SkipModeEn                               : MOS_BITFIELD_BIT(       0 );
            uint32_t   AdaptiveEn                               : MOS_BITFIELD_BIT(       1 );
            uint32_t   BiMixDis                                 : MOS_BITFIELD_BIT(       2 );
            uint32_t                                            : MOS_BITFIELD_RANGE(  3, 4 );
            uint32_t   EarlyImeSuccessEn                        : MOS_BITFIELD_BIT(       5 );
            uint32_t                                            : MOS_BITFIELD_BIT(       6 );
            uint32_t   T8x8FlagForInterEn                       : MOS_BITFIELD_BIT(       7 );
            uint32_t                                            : MOS_BITFIELD_RANGE(  8,23 );
            uint32_t   EarlyImeStop                             : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   MaxNumMVs                                : MOS_BITFIELD_RANGE(  0, 5 );
            uint32_t                                            : MOS_BITFIELD_RANGE(  6,15 );
            uint32_t   BiWeight                                 : MOS_BITFIELD_RANGE( 16,21 );
            uint32_t                                            : MOS_BITFIELD_RANGE( 22,27 );
            uint32_t   UniMixDisable                            : MOS_BITFIELD_BIT(      28 );
            uint32_t                                            : MOS_BITFIELD_RANGE( 29,31 );
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
            uint32_t   MaxLenSP                                 : MOS_BITFIELD_RANGE( 0, 7 );
            uint32_t   MaxNumSU                                 : MOS_BITFIELD_RANGE( 8, 15 );
            uint32_t                                            : MOS_BITFIELD_RANGE( 16, 31);
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
            uint32_t   SrcSize                                  : MOS_BITFIELD_RANGE(  0, 1 );
            uint32_t                                            : MOS_BITFIELD_RANGE(  2, 3 );
            uint32_t   MbTypeRemap                              : MOS_BITFIELD_RANGE(  4, 5 );
            uint32_t   SrcAccess                                : MOS_BITFIELD_BIT(       6 );
            uint32_t   RefAccess                                : MOS_BITFIELD_BIT(       7 );
            uint32_t   SearchCtrl                               : MOS_BITFIELD_RANGE(  8,10 );
            uint32_t   DualSearchPathOption                     : MOS_BITFIELD_BIT(      11 );
            uint32_t   SubPelMode                               : MOS_BITFIELD_RANGE( 12,13 );
            uint32_t   SkipType                                 : MOS_BITFIELD_BIT(      14 );
            uint32_t   DisableFieldCacheAlloc                   : MOS_BITFIELD_BIT(      15 );
            uint32_t   InterChromaMode                          : MOS_BITFIELD_BIT(      16 );
            uint32_t   FTEnable                                 : MOS_BITFIELD_BIT(      17 );
            uint32_t   BMEDisableFBR                            : MOS_BITFIELD_BIT(      18 );
            uint32_t   BlockBasedSkipEnable                     : MOS_BITFIELD_BIT(      19 );
            uint32_t   InterSAD                                 : MOS_BITFIELD_RANGE( 20,21 );
            uint32_t   IntraSAD                                 : MOS_BITFIELD_RANGE( 22,23 );
            uint32_t   SubMbPartMask                            : MOS_BITFIELD_RANGE( 24,30 );
            uint32_t                                            : MOS_BITFIELD_BIT(      31 );
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
            uint32_t                                            : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   PictureHeightMinus1                      : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   PictureWidth                             : MOS_BITFIELD_RANGE(16, 23);
            uint32_t                                            : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t                                            : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   QpPrimeY                                 : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   RefWidth                                 : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   RefHeight                                : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t                                            : MOS_BITFIELD_BIT(0);
            uint32_t   InputStreamInEn                          : MOS_BITFIELD_BIT(1);
            uint32_t   LCUSize                                  : MOS_BITFIELD_BIT(2);
            uint32_t   WriteDistortions                         : MOS_BITFIELD_BIT(3);
            uint32_t   UseMvFromPrevStep                        : MOS_BITFIELD_BIT(4);
            uint32_t                                            : MOS_BITFIELD_RANGE(5, 7);
            uint32_t   SuperCombineDist                         : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   MaxVmvR                                  : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t                                            : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   MVCostScaleFactor                        : MOS_BITFIELD_RANGE( 16,17 );
            uint32_t   BilinearEnable                           : MOS_BITFIELD_BIT(      18 );
            uint32_t   SrcFieldPolarity                         : MOS_BITFIELD_BIT(      19 );
            uint32_t   WeightedSADHAAR                          : MOS_BITFIELD_BIT(      20 );
            uint32_t   AConlyHAAR                               : MOS_BITFIELD_BIT(      21 );
            uint32_t   RefIDCostMode                            : MOS_BITFIELD_BIT(      22 );
            uint32_t                                            : MOS_BITFIELD_BIT(      23 );
            uint32_t   SkipCenterMask                           : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   Mode0Cost                                : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   Mode1Cost                                : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   Mode2Cost                                : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   Mode3Cost                                : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   Mode4Cost                                : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   Mode5Cost                                : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   Mode6Cost                                : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   Mode7Cost                                : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   Mode8Cost                                : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   Mode9Cost                                : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   RefIDCost                                : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   ChromaIntraModeCost                      : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   MV0Cost                                  : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   MV1Cost                                  : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   MV2Cost                                  : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   MV3Cost                                  : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   MV4Cost                                  : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   MV5Cost                                  : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   MV6Cost                                  : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   MV7Cost                                  : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   NumRefIdxL0MinusOne                      : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   NumRefIdxL1MinusOne                      : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   RefStreaminCost                          : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   ROIEnable                                : MOS_BITFIELD_RANGE(24, 26);
            uint32_t                                            : MOS_BITFIELD_RANGE(27, 31);
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
            uint32_t   List0RefID0FieldParity                   : MOS_BITFIELD_BIT(0);
            uint32_t   List0RefID1FieldParity                   : MOS_BITFIELD_BIT(1);
            uint32_t   List0RefID2FieldParity                   : MOS_BITFIELD_BIT(2);
            uint32_t   List0RefID3FieldParity                   : MOS_BITFIELD_BIT(3);
            uint32_t   List0RefID4FieldParity                   : MOS_BITFIELD_BIT(4);
            uint32_t   List0RefID5FieldParity                   : MOS_BITFIELD_BIT(5);
            uint32_t   List0RefID6FieldParity                   : MOS_BITFIELD_BIT(6);
            uint32_t   List0RefID7FieldParity                   : MOS_BITFIELD_BIT(7);
            uint32_t   List1RefID0FieldParity                   : MOS_BITFIELD_BIT(8);
            uint32_t   List1RefID1FieldParity                   : MOS_BITFIELD_BIT(9);
            uint32_t                                            : MOS_BITFIELD_RANGE(10, 31);
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
            uint32_t   PrevMvReadPosFactor                      : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   MvShiftFactor                            : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   Reserved                                 : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   ActualMBWidth                            : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   ActualMBHeight                           : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   RoiCtrl                                  : MOS_BITFIELD_RANGE( 0, 7 );
            uint32_t   MaxTuSize                                : MOS_BITFIELD_RANGE( 8, 9 );
            uint32_t   MaxCuSize                                : MOS_BITFIELD_RANGE( 10, 11 );
            uint32_t   NumImePredictors                         : MOS_BITFIELD_RANGE( 12, 15 );
            uint32_t   Reserved                                 : MOS_BITFIELD_RANGE( 16, 23 );
            uint32_t   PuTypeCtrl                               : MOS_BITFIELD_RANGE( 24, 31 );
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
            uint32_t   ForceMvx0                                : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   ForceMvy0                                : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   ForceMvx1                                : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   ForceMvy1                                : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   ForceMvx2                                : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   ForceMvy2                                : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   ForceMvx3                                : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   ForceMvy3                                : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   ForceRefIdx0                             : MOS_BITFIELD_RANGE(0, 3);
            uint32_t   ForceRefIdx1                             : MOS_BITFIELD_RANGE(4, 7);
            uint32_t   ForceRefIdx2                             : MOS_BITFIELD_RANGE(8, 11);
            uint32_t   ForceRefIdx3                             : MOS_BITFIELD_RANGE(12, 15);
            uint32_t   NumMergeCandCu8x8                        : MOS_BITFIELD_RANGE(16, 19);
            uint32_t   NumMergeCandCu16x16                      : MOS_BITFIELD_RANGE(20, 23);
            uint32_t   NumMergeCandCu32x32                      : MOS_BITFIELD_RANGE(24, 27);
            uint32_t   NumMergeCandCu64x64                      : MOS_BITFIELD_RANGE(28, 31);
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
            uint32_t   SegID                                    : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   QpEnable                                 : MOS_BITFIELD_RANGE(16, 19);
            uint32_t   SegIDEnable                              : MOS_BITFIELD_BIT(20);
            uint32_t   Reserved                                 : MOS_BITFIELD_RANGE(21, 22);
            uint32_t   ForceRefIdEnable                         : MOS_BITFIELD_BIT(23);
            uint32_t   Reserved1                                : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   ForceQp0                                 : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   ForceQp1                                 : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   ForceQp2                                 : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   ForceQp3                                 : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   Reserved                                 : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   _4xMeMvOutputDataSurfIndex               : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   _16xOr32xMeMvInputDataSurfIndex          : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   _4xMeOutputDistSurfIndex                 : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   _4xMeOutputBrcDistSurfIndex              : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   VMEFwdInterPredictionSurfIndex           : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   VMEBwdInterPredictionSurfIndex           : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   VDEncStreamInOutputSurfIndex             : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   VDEncStreamInInputSurfIndex              : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW47;
};
C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(CODECHAL_VDENC_HEVC_ME_CURBE_G10)) == 48);

using PCODECHAL_VDENC_HEVC_ME_CURBE_G10 = CODECHAL_VDENC_HEVC_ME_CURBE_G10*;

//!  HEVC VDEnc encoder class for GEN10
/*!
This class defines the member fields, functions for GEN10 platform
*/
class CodechalVdencHevcStateG10 : public CodechalVdencHevcState
{
public:
    static constexpr uint32_t   m_minScaledSurfaceSize = 64;           //!< Minimum scaled surface size
    static constexpr uint32_t   m_brcConstantSurfaceWidth = 64;        //!< BRC constant surface width
    static constexpr uint32_t   m_brcConstantSurfaceHeight = 35;       //!< BRC constant surface height
    static constexpr uint32_t   m_brcHistoryBufferSize = 1016;          //!< BRC history buffer size
    static constexpr uint32_t   m_bframeMeBidirectionalWeight = 32;    //!< B frame bidirection weight
    static constexpr uint32_t   m_insertOffsetAfterCMD1 = 120;        //!< Huc Initializer CMD1 delta
    static constexpr uint32_t   m_insertOffsetAfterCMD2 = 148;        //!< Huc Initializer CMD2 delta

    // HuC tables.
    // These Values are diff for each Gen
    static constexpr uint32_t    m_numDevThreshlds = 8;
    static constexpr double      m_devStdFPS = 30.0;
    static constexpr double      m_bpsRatioLow = 0.1;
    static constexpr double      m_bpsRatioHigh = 3.5;
    static constexpr int32_t     m_postMultPB = 50;
    static constexpr int32_t     m_negMultPB = -50;
    static constexpr int32_t     m_posMultVBR = 100;
    static constexpr int32_t     m_negMultVBR = -50;

    static const double          m_devThreshIFPNEG[m_numDevThreshlds / 2];
    static const double          m_devThreshIFPPOS[m_numDevThreshlds / 2];
    static const double          m_devThreshPBFPNEG[m_numDevThreshlds / 2];
    static const double          m_devThreshPBFPPOS[m_numDevThreshlds / 2];
    static const double          m_devThreshVBRNEG[m_numDevThreshlds / 2];
    static const double          m_devThreshVBRPOS[m_numDevThreshlds / 2];
    static const int8_t          m_lowdelayDevThreshPB[m_numDevThreshlds];
    static const int8_t          m_lowdelayDevThreshVBR[m_numDevThreshlds];
    static const int8_t          m_lowdelayDevThreshI[m_numDevThreshlds];
    static const int8_t          m_lowdelayDeltaFrmszI[][8];
    static const int8_t          m_lowdelayDeltaFrmszP[][8];
    static const int8_t          m_lowdelayDeltaFrmszB[][8];

    static const uint32_t       m_hucConstantData[];
    static const uint32_t       m_meCurbeInit[48];                      //!< Curbe initialization data for ME kernel

    //!
    //! \brief    Constructor
    //!
    CodechalVdencHevcStateG10(CodechalHwInterface* hwInterface,
        CodechalDebugInterface* debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo);

    //!
    //! \brief    Destructor
    //!
    ~CodechalVdencHevcStateG10()
    {
        CODECHAL_DEBUG_TOOL(
            DestroyHevcPar();
            MOS_Delete(m_encodeParState);
        )
    }

    //!
    //! \brief    Get encoder kernel header and kernel size
    //!
    //! \param    [in] binary
    //!           Pointer to kernel binary
    //! \param    [in] operation
    //!           Enc kernel operation
    //! \param    [in] krnStateIdx
    //!           Kernel state index
    //! \param    [out] krnHeader
    //!           Pointer to kernel header
    //! \param    [out] krnSize
    //!           Pointer to kernel size
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    static MOS_STATUS GetKernelHeaderAndSize(
        void                           *binary,
        EncOperation                   operation,
        uint32_t                       krnStateIdx,
        void                           *krnHeader,
        uint32_t                       *krnSize);

    //!
    //! \brief    Set kernel parameters for specific kernel operation
    //!
    //! \param    [in] operation
    //!           Kernel operation
    //! \param    [out] kernelParams
    //!           Pointer to kernel parameters
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetKernelParams(
        EncOperation     operation,
        MHW_KERNEL_PARAM *kernelParams);

    //!
    //! \brief    Set binding table for specific kernel operation
    //!
    //! \param    [in] operation
    //!           Kernel operation
    //! \param    [out] bindingTable
    //!           Pointer to binding table
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetBindingTable(
        EncOperation                           operation,
        PCODECHAL_ENCODE_BINDING_TABLE_GENERIC bindingTable);

    //!
    //! \brief    Init kernel state for HME kernel
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS InitKernelStateMe();

    //!
    //! \brief    Init kernel state for streamin kernel
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS InitKernelStateStreamIn();

    //!
    //! \brief    Invoke HME kernel
    //!
    //! \param    [in] using4xMe
    //!           True if 4xMe is invoked, otherwise 16xMe is invoked
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS EncodeMeKernel(bool using4xMe);

    //!
    //! \brief    Set curbe for HME kernel
    //!
    //! \param    [in] using4xMe
    //!           True if 4xMe is invoked, otherwise 16xMe is invoked
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetMeCurbe(bool using4xMe);

    //!
    //! \brief    Set surface state for HME kernel
    //!
    //! \param    [in] using4xMe
    //!           True if 4xMe is invoked, otherwise 16xMe is invoked
    //! \param    [in] cmdBuffer
    //!           Pointer to command buffer that surface states are added
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SendMeSurfaces(bool using4xMe, PMOS_COMMAND_BUFFER cmdBuffer);

    // inherited virtual functions
    uint32_t GetMaxBtCount();
    bool CheckSupportedFormat(PMOS_SURFACE surface);
    MOS_STATUS Initialize(CodechalSetting * settings);
    MOS_STATUS InitKernelState();
    MOS_STATUS AllocatePakResources();
    MOS_STATUS FreePakResources();
    MOS_STATUS AllocateEncResources();
    MOS_STATUS FreeEncResources();
    MOS_STATUS AllocateBrcResources();
    MOS_STATUS FreeBrcResources();
    MOS_STATUS InitializePicture(const EncoderParams& params);
    MOS_STATUS PlatformCapabilityCheck();
    void SetStreaminDataPerLcu(
        PMHW_VDBOX_VDENC_STREAMIN_STATE_PARAMS streaminParams,
        void* streaminData);
    MOS_STATUS EncodeKernelFunctions();
    MOS_STATUS ConstructBatchBufferHuCCQP(PMOS_RESOURCE batchBuffer);
    MOS_STATUS ConstructBatchBufferHuCBRC(PMOS_RESOURCE batchBuffer);
    MOS_STATUS SetDmemHuCBrcInitReset();
    MOS_STATUS SetConstDataHuCBrcUpdate();
    MOS_STATUS SetDmemHuCBrcUpdate();
    MOS_STATUS GetStatusReport(
        EncodeStatus *encodeStatus,
        EncodeStatusReport *encodeStatusReport);

};

//! \brief  typedef of class CodechalVdencHevcStateG10*
using PCODECHAL_VDENC_HEVC_STATE_G10 = class CodechalVdencHevcStateG10*;

#endif  // __CODECHAL_VDENC_HEVC_G10_H__
