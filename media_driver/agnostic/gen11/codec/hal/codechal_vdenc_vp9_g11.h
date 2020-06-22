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

/*
*  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
*
*  Use of this source code is governed by a BSD-style license
*  that can be found in the LICENSE file in the root of the source
*  tree. An additional intellectual property rights grant can be found
*  in the file PATENTS.  All contributing project authors may
*  be found in the AUTHORS file in the root of the source tree.
*/

//!
//! \file     codechal_vdenc_vp9_g11.h
//! \brief    VP9 VDENC encoder for GEN11 platform.
//!

#ifndef __CODECHAL_VDENC_VP9_G11_H__
#define __CODECHAL_VDENC_VP9_G11_H__

#include "codechal.h"
#include "codechal_hw.h"
#include "codechal_vdenc_vp9_base.h"
#include "mhw_vdbox_g11_X.h"
#include "codechal_encode_singlepipe_virtualengine.h"
#include "codechal_encode_scalability.h"

#define CODECHAL_ENCODE_HME_FIRST_STEP_G11 0
#define CODECHAL_ENCODE_HME_FOLLOWING_STEP_G11 1
#define CODECHAL_ENCODE_MV_SHIFT_FACTOR_32x_G11 1
#define CODECHAL_ENCODE_MV_SHIFT_FACTOR_16x_G11 2
#define CODECHAL_ENCODE_MV_SHIFT_FACTOR_4x_G11 2
#define CODECHAL_ENCODE_PREV_MV_READ_POSITION_16x_G11 1
#define CODECHAL_ENCODE_PREV_MV_READ_POSITION_4x_G11 0

#define __MEDIA_USER_FEATURE_VALUE_VP9_ENCODE_VDBOX_NUM_ID      5155
#define __MEDIA_USER_FEATURE_VALUE_VP9_ENCODE_ENABLE_VE_ID      5156
#define __MEDIA_USER_FEATURE_VALUE_VP9_ENCODE_ENABLE_HW_STITCH  5157

class CodechalVdencVp9StateG11 : public CodechalVdencVp9State
{
public:
    enum MeBindingTableOffset
    {
        CODECHAL_ENCODE_ME_MV_DATA_SURFACE_G11       = 0,
        CODECHAL_ENCODE_16xME_MV_DATA_SURFACE_G11    = 1,
        CODECHAL_ENCODE_32xME_MV_DATA_SURFACE_G11    = 1,
        CODECHAL_ENCODE_ME_DISTORTION_SURFACE_G11    = 2,
        CODECHAL_ENCODE_ME_BRC_DISTORTION_G11        = 3,
        CODECHAL_ENCODE_ME_RESERVED0_G11             = 4,
        CODECHAL_ENCODE_ME_CURR_FOR_FWD_REF_G11      = 5,
        CODECHAL_ENCODE_ME_FWD_REF_IDX0_G11          = 6,
        CODECHAL_ENCODE_ME_RESERVED1_G11             = 7,
        CODECHAL_ENCODE_ME_FWD_REF_IDX1_G11          = 8,
        CODECHAL_ENCODE_ME_RESERVED2_G11             = 9,
        CODECHAL_ENCODE_ME_FWD_REF_IDX2_G11          = 10,
        CODECHAL_ENCODE_ME_RESERVED3_G11             = 11,
        CODECHAL_ENCODE_ME_FWD_REF_IDX3_G11          = 12,
        CODECHAL_ENCODE_ME_RESERVED4_G11             = 13,
        CODECHAL_ENCODE_ME_FWD_REF_IDX4_G11          = 14,
        CODECHAL_ENCODE_ME_RESERVED5_G11             = 15,
        CODECHAL_ENCODE_ME_FWD_REF_IDX5_G11          = 16,
        CODECHAL_ENCODE_ME_RESERVED6_G11             = 17,
        CODECHAL_ENCODE_ME_FWD_REF_IDX6_G11          = 18,
        CODECHAL_ENCODE_ME_RESERVED7_G11             = 19,
        CODECHAL_ENCODE_ME_FWD_REF_IDX7_G11          = 20,
        CODECHAL_ENCODE_ME_RESERVED8_G11             = 21,
        CODECHAL_ENCODE_ME_CURR_FOR_BWD_REF_G11      = 22,
        CODECHAL_ENCODE_ME_BWD_REF_IDX0_G11          = 23,
        CODECHAL_ENCODE_ME_RESERVED9_G11             = 24,
        CODECHAL_ENCODE_ME_BWD_REF_IDX1_G11          = 25,
        CODECHAL_ENCODE_ME_VDENC_STREAMIN_OUTPUT_G11 = 26,
        CODECHAL_ENCODE_ME_VDENC_STREAMIN_INPUT_G11  = 27,
        CODECHAL_ENCODE_ME_NUM_SURFACES_G11          = 28
    };

    struct StatsInfo
    {
        uint32_t tileSizeRecord;
        uint32_t vdencStats;
        uint32_t pakStats;
        uint32_t counterBuffer;
    };
    
    struct MeCurbe
    {
        // DW0
        union
        {
            struct
            {
                uint32_t SkipModeEn : MOS_BITFIELD_BIT(0);
                uint32_t AdaptiveEn : MOS_BITFIELD_BIT(1);
                uint32_t BiMixDis : MOS_BITFIELD_BIT(2);
                uint32_t : MOS_BITFIELD_RANGE(3, 4);
                uint32_t EarlyImeSuccessEn : MOS_BITFIELD_BIT(5);
                uint32_t : MOS_BITFIELD_BIT(6);
                uint32_t T8x8FlagForInterEn : MOS_BITFIELD_BIT(7);
                uint32_t : MOS_BITFIELD_RANGE(8, 23);
                uint32_t EarlyImeStop : MOS_BITFIELD_RANGE(24, 31);
            };
            struct
            {
                uint32_t Value;
            };
        } DW0;

        // DW1
        union
        {
            struct
            {
                uint32_t MaxNumMVs : MOS_BITFIELD_RANGE(0, 5);
                uint32_t : MOS_BITFIELD_RANGE(6, 15);
                uint32_t BiWeight : MOS_BITFIELD_RANGE(16, 21);
                uint32_t : MOS_BITFIELD_RANGE(22, 27);
                uint32_t UniMixDisable : MOS_BITFIELD_BIT(28);
                uint32_t : MOS_BITFIELD_RANGE(29, 31);
            };
            struct
            {
                uint32_t Value;
            };
        } DW1;

        // DW2
        union
        {
            struct
            {
                uint32_t MaxLenSP : MOS_BITFIELD_RANGE(0, 7);
                uint32_t MaxNumSU : MOS_BITFIELD_RANGE(8, 15);
                uint32_t : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t Value;
            };
        } DW2;

        // DW3
        union
        {
            struct
            {
                uint32_t SrcSize : MOS_BITFIELD_RANGE(0, 1);
                uint32_t : MOS_BITFIELD_RANGE(2, 3);
                uint32_t MbTypeRemap : MOS_BITFIELD_RANGE(4, 5);
                uint32_t SrcAccess : MOS_BITFIELD_BIT(6);
                uint32_t RefAccess : MOS_BITFIELD_BIT(7);
                uint32_t SearchCtrl : MOS_BITFIELD_RANGE(8, 10);
                uint32_t DualSearchPathOption : MOS_BITFIELD_BIT(11);
                uint32_t SubPelMode : MOS_BITFIELD_RANGE(12, 13);
                uint32_t SkipType : MOS_BITFIELD_BIT(14);
                uint32_t DisableFieldCacheAlloc : MOS_BITFIELD_BIT(15);
                uint32_t InterChromaMode : MOS_BITFIELD_BIT(16);
                uint32_t FTEnable : MOS_BITFIELD_BIT(17);
                uint32_t BMEDisableFBR : MOS_BITFIELD_BIT(18);
                uint32_t BlockBasedSkipEnable : MOS_BITFIELD_BIT(19);
                uint32_t InterSAD : MOS_BITFIELD_RANGE(20, 21);
                uint32_t IntraSAD : MOS_BITFIELD_RANGE(22, 23);
                uint32_t SubMbPartMask : MOS_BITFIELD_RANGE(24, 30);
                uint32_t : MOS_BITFIELD_BIT(31);
            };
            struct
            {
                uint32_t Value;
            };
        } DW3;

        // DW4
        union
        {
            struct
            {
                uint32_t : MOS_BITFIELD_RANGE(0, 7);
                uint32_t PictureHeightMinus1 : MOS_BITFIELD_RANGE(8, 15);
                uint32_t PictureWidth : MOS_BITFIELD_RANGE(16, 23);
                uint32_t : MOS_BITFIELD_RANGE(24, 31);
            };
            struct
            {
                uint32_t Value;
            };
        } DW4;

        // DW5
        union
        {
            struct
            {
                uint32_t : MOS_BITFIELD_RANGE(0, 7);
                uint32_t QpPrimeY : MOS_BITFIELD_RANGE(8, 15);
                uint32_t RefWidth : MOS_BITFIELD_RANGE(16, 23);
                uint32_t RefHeight : MOS_BITFIELD_RANGE(24, 31);
            };
            struct
            {
                uint32_t Value;
            };
        } DW5;

        // DW6
        union
        {
            struct
            {
                uint32_t : MOS_BITFIELD_BIT(0);
                uint32_t InputStreamInSurfaceEnable : MOS_BITFIELD_BIT(1);
                uint32_t LCUSize : MOS_BITFIELD_BIT(2);
                uint32_t WriteDistortions : MOS_BITFIELD_BIT(3);
                uint32_t UseMvFromPrevStep : MOS_BITFIELD_BIT(4);
                uint32_t : MOS_BITFIELD_RANGE(5, 7);
                uint32_t SuperCombineDist : MOS_BITFIELD_RANGE(8, 15);
                uint32_t MaxVmvR : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t Value;
            };
        } DW6;

        // DW7
        union
        {
            struct
            {
                uint32_t : MOS_BITFIELD_RANGE(0, 15);
                uint32_t MVCostScaleFactor : MOS_BITFIELD_RANGE(16, 17);
                uint32_t BilinearEnable : MOS_BITFIELD_BIT(18);
                uint32_t SrcFieldPolarity : MOS_BITFIELD_BIT(19);
                uint32_t WeightedSADHAAR : MOS_BITFIELD_BIT(20);
                uint32_t AConlyHAAR : MOS_BITFIELD_BIT(21);
                uint32_t RefIDCostMode : MOS_BITFIELD_BIT(22);
                uint32_t : MOS_BITFIELD_BIT(23);
                uint32_t SkipCenterMask : MOS_BITFIELD_RANGE(24, 31);
            };
            struct
            {
                uint32_t Value;
            };
        } DW7;

        // DW8
        union
        {
            struct
            {
                uint32_t Mode0Cost : MOS_BITFIELD_RANGE(0, 7);
                uint32_t Mode1Cost : MOS_BITFIELD_RANGE(8, 15);
                uint32_t Mode2Cost : MOS_BITFIELD_RANGE(16, 23);
                uint32_t Mode3Cost : MOS_BITFIELD_RANGE(24, 31);
            };
            struct
            {
                uint32_t Value;
            };
        } DW8;

        // DW9
        union
        {
            struct
            {
                uint32_t Mode4Cost : MOS_BITFIELD_RANGE(0, 7);
                uint32_t Mode5Cost : MOS_BITFIELD_RANGE(8, 15);
                uint32_t Mode6Cost : MOS_BITFIELD_RANGE(16, 23);
                uint32_t Mode7Cost : MOS_BITFIELD_RANGE(24, 31);
            };
            struct
            {
                uint32_t Value;
            };
        } DW9;

        // DW10
        union
        {
            struct
            {
                uint32_t Mode8Cost : MOS_BITFIELD_RANGE(0, 7);
                uint32_t Mode9Cost : MOS_BITFIELD_RANGE(8, 15);
                uint32_t RefIDCost : MOS_BITFIELD_RANGE(16, 23);
                uint32_t ChromaIntraModeCost : MOS_BITFIELD_RANGE(24, 31);
            };
            struct
            {
                uint32_t Value;
            };
        } DW10;

        // DW11
        union
        {
            struct
            {
                uint32_t MV0Cost : MOS_BITFIELD_RANGE(0, 7);
                uint32_t MV1Cost : MOS_BITFIELD_RANGE(8, 15);
                uint32_t MV2Cost : MOS_BITFIELD_RANGE(16, 23);
                uint32_t MV3Cost : MOS_BITFIELD_RANGE(24, 31);
            };
            struct
            {
                uint32_t Value;
            };
        } DW11;

        // DW12
        union
        {
            struct
            {
                uint32_t MV4Cost : MOS_BITFIELD_RANGE(0, 7);
                uint32_t MV5Cost : MOS_BITFIELD_RANGE(8, 15);
                uint32_t MV6Cost : MOS_BITFIELD_RANGE(16, 23);
                uint32_t MV7Cost : MOS_BITFIELD_RANGE(24, 31);
            };
            struct
            {
                uint32_t Value;
            };
        } DW12;

        // DW13
        union
        {
            struct
            {
                uint32_t NumRefIdxL0MinusOne : MOS_BITFIELD_RANGE(0, 7);
                uint32_t NumRefIdxL1MinusOne : MOS_BITFIELD_RANGE(8, 15);
                uint32_t RefStreaminCost : MOS_BITFIELD_RANGE(16, 23);
                uint32_t ROIEnable : MOS_BITFIELD_RANGE(24, 26);
                uint32_t : MOS_BITFIELD_RANGE(27, 31);
            };
            struct
            {
                uint32_t Value;
            };
        } DW13;

        // DW14
        union
        {
            struct
            {
                uint32_t List0RefID0FieldParity : MOS_BITFIELD_BIT(0);
                uint32_t List0RefID1FieldParity : MOS_BITFIELD_BIT(1);
                uint32_t List0RefID2FieldParity : MOS_BITFIELD_BIT(2);
                uint32_t List0RefID3FieldParity : MOS_BITFIELD_BIT(3);
                uint32_t List0RefID4FieldParity : MOS_BITFIELD_BIT(4);
                uint32_t List0RefID5FieldParity : MOS_BITFIELD_BIT(5);
                uint32_t List0RefID6FieldParity : MOS_BITFIELD_BIT(6);
                uint32_t List0RefID7FieldParity : MOS_BITFIELD_BIT(7);
                uint32_t List1RefID0FieldParity : MOS_BITFIELD_BIT(8);
                uint32_t List1RefID1FieldParity : MOS_BITFIELD_BIT(9);
                uint32_t : MOS_BITFIELD_RANGE(10, 31);
            };
            struct
            {
                uint32_t Value;
            };
        } DW14;

        // DW15
        union
        {
            struct
            {
                uint32_t PrevMvReadPosFactor : MOS_BITFIELD_RANGE(0, 7);
                uint32_t MvShiftFactor : MOS_BITFIELD_RANGE(8, 15);
                uint32_t Reserved : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t Value;
            };
        } DW15;

        struct
        {
            // DW16
            union
            {
                struct
                {
                    SearchPathDelta SPDelta_0;
                    SearchPathDelta SPDelta_1;
                    SearchPathDelta SPDelta_2;
                    SearchPathDelta SPDelta_3;
                };
                struct
                {
                    uint32_t Value;
                };
            } DW16;

            // DW17
            union
            {
                struct
                {
                    SearchPathDelta SPDelta_4;
                    SearchPathDelta SPDelta_5;
                    SearchPathDelta SPDelta_6;
                    SearchPathDelta SPDelta_7;
                };
                struct
                {
                    uint32_t Value;
                };
            } DW17;

            // DW18
            union
            {
                struct
                {
                    SearchPathDelta SPDelta_8;
                    SearchPathDelta SPDelta_9;
                    SearchPathDelta SPDelta_10;
                    SearchPathDelta SPDelta_11;
                };
                struct
                {
                    uint32_t Value;
                };
            } DW18;

            // DW19
            union
            {
                struct
                {
                    SearchPathDelta SPDelta_12;
                    SearchPathDelta SPDelta_13;
                    SearchPathDelta SPDelta_14;
                    SearchPathDelta SPDelta_15;
                };
                struct
                {
                    uint32_t Value;
                };
            } DW19;

            // DW20
            union
            {
                struct
                {
                    SearchPathDelta SPDelta_16;
                    SearchPathDelta SPDelta_17;
                    SearchPathDelta SPDelta_18;
                    SearchPathDelta SPDelta_19;
                };
                struct
                {
                    uint32_t Value;
                };
            } DW20;

            // DW21
            union
            {
                struct
                {
                    SearchPathDelta SPDelta_20;
                    SearchPathDelta SPDelta_21;
                    SearchPathDelta SPDelta_22;
                    SearchPathDelta SPDelta_23;
                };
                struct
                {
                    uint32_t Value;
                };
            } DW21;

            // DW22
            union
            {
                struct
                {
                    SearchPathDelta SPDelta_24;
                    SearchPathDelta SPDelta_25;
                    SearchPathDelta SPDelta_26;
                    SearchPathDelta SPDelta_27;
                };
                struct
                {
                    uint32_t Value;
                };
            } DW22;

            // DW23
            union
            {
                struct
                {
                    SearchPathDelta SPDelta_28;
                    SearchPathDelta SPDelta_29;
                    SearchPathDelta SPDelta_30;
                    SearchPathDelta SPDelta_31;
                };
                struct
                {
                    uint32_t Value;
                };
            } DW23;

            // DW24
            union
            {
                struct
                {
                    SearchPathDelta SPDelta_32;
                    SearchPathDelta SPDelta_33;
                    SearchPathDelta SPDelta_34;
                    SearchPathDelta SPDelta_35;
                };
                struct
                {
                    uint32_t Value;
                };
            } DW24;

            // DW25
            union
            {
                struct
                {
                    SearchPathDelta SPDelta_36;
                    SearchPathDelta SPDelta_37;
                    SearchPathDelta SPDelta_38;
                    SearchPathDelta SPDelta_39;
                };
                struct
                {
                    uint32_t Value;
                };
            } DW25;

            // DW26
            union
            {
                struct
                {
                    SearchPathDelta SPDelta_40;
                    SearchPathDelta SPDelta_41;
                    SearchPathDelta SPDelta_42;
                    SearchPathDelta SPDelta_43;
                };
                struct
                {
                    uint32_t Value;
                };
            } DW26;

            // DW27
            union
            {
                struct
                {
                    SearchPathDelta SPDelta_44;
                    SearchPathDelta SPDelta_45;
                    SearchPathDelta SPDelta_46;
                    SearchPathDelta SPDelta_47;
                };
                struct
                {
                    uint32_t Value;
                };
            } DW27;

            // DW28
            union
            {
                struct
                {
                    SearchPathDelta SPDelta_48;
                    SearchPathDelta SPDelta_49;
                    SearchPathDelta SPDelta_50;
                    SearchPathDelta SPDelta_51;
                };
                struct
                {
                    uint32_t Value;
                };
            } DW28;

            // DW29
            union
            {
                struct
                {
                    SearchPathDelta SPDelta_52;
                    SearchPathDelta SPDelta_53;
                    SearchPathDelta SPDelta_54;
                    SearchPathDelta SPDelta_55;
                };
                struct
                {
                    uint32_t Value;
                };
            } DW29;
        } SPDelta;

        // DW30
        union
        {
            struct
            {
                uint32_t ActualMBWidth : MOS_BITFIELD_RANGE(0, 15);
                uint32_t ActualMBHeight : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t Value;
            };
        } DW30;

        // DW31
        union
        {
            struct
            {
                uint32_t RoiCtrl : MOS_BITFIELD_RANGE(0, 7);
                uint32_t MaxTuSize : MOS_BITFIELD_RANGE(8, 9);
                uint32_t MaxCuSize : MOS_BITFIELD_RANGE(10, 11);
                uint32_t NumImePredictors : MOS_BITFIELD_RANGE(12, 15);
                uint32_t : MOS_BITFIELD_RANGE(16, 23);
                uint32_t PuTypeCtrl : MOS_BITFIELD_RANGE(24, 31);
            };
            struct
            {
                uint32_t Value;
            };
        } DW31;

        // DW32
        union
        {
            struct
            {
                uint32_t ForceMvx0 : MOS_BITFIELD_RANGE(0, 15);
                uint32_t ForceMvy0 : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t Value;
            };
        } DW32;

        // DW33
        union
        {
            struct
            {
                uint32_t ForceMvx1 : MOS_BITFIELD_RANGE(0, 15);
                uint32_t ForceMvy1 : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t Value;
            };
        } DW33;

        // DW34
        union
        {
            struct
            {
                uint32_t ForceMvx2 : MOS_BITFIELD_RANGE(0, 15);
                uint32_t ForceMvy2 : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t Value;
            };
        } DW34;

        // DW35
        union
        {
            struct
            {
                uint32_t ForceMvx3 : MOS_BITFIELD_RANGE(0, 15);
                uint32_t ForceMvy3 : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t Value;
            };
        } DW35;

        // DW36
        union
        {
            struct
            {
                uint32_t ForceRefIdx0 : MOS_BITFIELD_RANGE(0, 3);
                uint32_t ForceRefIdx1 : MOS_BITFIELD_RANGE(4, 7);
                uint32_t ForceRefIdx2 : MOS_BITFIELD_RANGE(8, 11);
                uint32_t ForceRefIdx3 : MOS_BITFIELD_RANGE(12, 15);
                uint32_t NumMergeCandidateCu8x8 : MOS_BITFIELD_RANGE(16, 19);
                uint32_t NumMergeCandidateCu16x16 : MOS_BITFIELD_RANGE(20, 23);
                uint32_t NumMergeCandidateCu32x32 : MOS_BITFIELD_RANGE(24, 27);
                uint32_t NumMergeCandidateCu64x64 : MOS_BITFIELD_RANGE(28, 31);
            };
            struct
            {
                uint32_t Value;
            };
        } DW36;

        // DW37
        union
        {
            struct
            {
                uint32_t SegID : MOS_BITFIELD_RANGE(0, 15);
                uint32_t QpEnable : MOS_BITFIELD_RANGE(16, 19);
                uint32_t SegIDEnable : MOS_BITFIELD_BIT(20);
                uint32_t : MOS_BITFIELD_RANGE(21, 22);
                uint32_t ForceRefIdEnable : MOS_BITFIELD_BIT(23);
                uint32_t : MOS_BITFIELD_RANGE(24, 31);
            };
            struct
            {
                uint32_t Value;
            };
        } DW37;

        // DW38
        union
        {
            struct
            {
                uint32_t ForceQp0 : MOS_BITFIELD_RANGE(0, 7);
                uint32_t ForceQp1 : MOS_BITFIELD_RANGE(8, 15);
                uint32_t ForceQp2 : MOS_BITFIELD_RANGE(16, 23);
                uint32_t ForceQp3 : MOS_BITFIELD_RANGE(24, 31);
            };
            struct
            {
                uint32_t Value;
            };
        } DW38;

        // DW39
        union
        {
            struct
            {
                uint32_t : MOS_BITFIELD_RANGE(0, 31);
            };
            struct
            {
                uint32_t Value;
            };
        } DW39;

        // DW40
        union
        {
            struct
            {
                uint32_t _4xMeMvOutputDataSurfIndex : MOS_BITFIELD_RANGE(0, 31);
            };
            struct
            {
                uint32_t Value;
            };
        } DW40;

        // DW41
        union
        {
            struct
            {
                uint32_t _16xOr32xMeMvInputDataSurfIndex : MOS_BITFIELD_RANGE(0, 31);
            };
            struct
            {
                uint32_t Value;
            };
        } DW41;

        // DW42
        union
        {
            struct
            {
                uint32_t _4xMeOutputDistSurfIndex : MOS_BITFIELD_RANGE(0, 31);
            };
            struct
            {
                uint32_t Value;
            };
        } DW42;

        // DW43
        union
        {
            struct
            {
                uint32_t _4xMeOutputBrcDistSurfIndex : MOS_BITFIELD_RANGE(0, 31);
            };
            struct
            {
                uint32_t Value;
            };
        } DW43;

        // DW44
        union
        {
            struct
            {
                uint32_t VMEFwdInterPredictionSurfIndex : MOS_BITFIELD_RANGE(0, 31);
            };
            struct
            {
                uint32_t Value;
            };
        } DW44;

        // DW45
        union
        {
            struct
            {
                uint32_t VMEBwdInterPredictionSurfIndex : MOS_BITFIELD_RANGE(0, 31);
            };
            struct
            {
                uint32_t Value;
            };
        } DW45;

        // DW46
        union
        {
            struct
            {
                uint32_t VDEncStreamInOutputSurfIndex : MOS_BITFIELD_RANGE(0, 31);
            };
            struct
            {
                uint32_t Value;
            };
        } DW46;

        // DW47
        union
        {
            struct
            {
                uint32_t VDEncStreamInInputSurfIndex : MOS_BITFIELD_RANGE(0, 31);
            };
            struct
            {
                uint32_t Value;
            };
        } DW47;
    };
    C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(MeCurbe)) == 48);

    // ME CURBE init data for G11 Kernel
    static const uint32_t meCurbeInit[48];

    struct HucPakIntDmem
    {
        uint32_t  tileSizeRecordOffset[5];            // Tile Size Records, start offset  in byte, 0xffffffff means unavailable
        uint32_t  vdencStatOffset[5];                 // needed for HEVC VDEnc, VP9 VDEnc, start offset  in byte, 0xffffffff means unavailable
        uint32_t  hevcPakStatOffset[5];               // needed for HEVC VDEnc, start offset  in byte, 0xffffffff means unavailable
        uint32_t  hevcStreamoutOffset[5];             // needed for HEVC VDEnc, start offset  in byte, 0xffffffff means unavailable
        uint32_t  vp9PakStatOffset[5];                // needed for VP9 VDEnc, start offset  in byte, 0xffffffff means unavailable
        uint32_t  vp9CounterBufferOffset[5];          // needed for VP9 VDEnc, start offset  in byte, 0xffffffff means unavailable
        uint32_t  lastTileBSStartInBytes;            // last tile in bitstream for region 4 and region 5
        uint32_t  SliceHeaderSizeinBits;              // needed for HEVC dual pipe BRC
        uint16_t  totalSizeInCommandBuffer;           // Total size in bytes of valid data in the command buffer
        uint16_t  offsetInCommandBuffer;              // Byte  offset of the to-be-updated Length (uint32_t ) in the command buffer, 0xffff means unavailable
        uint16_t  picWidthInPixel;                    // Picture width in pixel
        uint16_t  picHeightInPixel;                   // Picture hieght in pixel
        uint16_t  totalNumberOfPaks;                  // [2..4] for Gen11
        uint16_t  numSlices[4];                       // this is number of slices in each PAK
        uint16_t  numTiles[4];                        // this is number of tiles from each PAK
        uint16_t  picStateStartInBytes;               // offset for  region 7 and region 8
        uint8_t   codec;                              // 1: HEVC DP; 2: HEVC VDEnc; 3: VP9 VDEnc
        uint8_t   maxPass;                            // Max number of BRC pass >=1
        uint8_t   currentPass;                        // Current BRC pass [1..MAXPass]
        uint8_t   minCUSize;                          // Minimum CU size (3: 8x8, 4:16x16), HEVC only.
        uint8_t   cabacZeroWordFlag;                  // Cabac zero flag, HEVC only
        uint8_t   bitdepthLuma;                       // luma bitdepth, HEVC only
        uint8_t   bitdepthChroma;                     // chroma bitdepth, HEVC only
        uint8_t   chromaFormatIdc;                    // chroma format idc, HEVC only
        uint8_t   currFrameBRClevel;  // Hevc dual pipe only
        uint8_t   brcUnderFlowEnable; // Hevc dual pipe only    
        uint8_t   StitchEnable;// enable stitch cmd for Hevc dual pipe
        uint8_t   reserved1;
        uint16_t  StitchCommandOffset; // offset in region 10 which is the second level batch buffer
        uint16_t  reserved2;
        uint32_t  BBEndforStitch;
        uint8_t   RSVD[16];
    };

    // Delay between semaphore(barrier) wait and reset
    static constexpr uint32_t m_numDelay = 15;
    // VDENC BRC related buffer size
    static constexpr uint32_t m_brcStatsBufSize = ((48 + 256) * sizeof(uint32_t));        // 48 DWs of statistics data at the end of every tile that will be used by the BRC kernel. 64 byte aligned. 256 DWs of Histogram data at an offset of 3 CLs (48 DWs)
    static constexpr uint32_t m_brcPakStatsBufSize = (64 * sizeof(uint32_t));
    static constexpr uint32_t m_brcHistoryBufSize = 1152;
    // VDENC Pak Int related constants
    static constexpr uint32_t m_pakIntDmemOffsetsSize = 120;
    static constexpr uint32_t m_pakIntVp9CodecId = 3;
    static constexpr uint32_t m_maxNumPipes = 4;
    // SHME related
    static constexpr uint32_t m_hmeMaxMvLength = 511;
    CODEC_PICTURE m_refPicList0[3];

    // Virtual engine
    //Scalability
    uint8_t                                     m_numPipe = 0;
    uint8_t                                     m_numPassesInOnePipe = 0;
    bool                                        m_scalableMode = false;
    bool                                        m_lastFrameScalableMode = false;
    bool                                        m_isTilingSupported = false;
    bool                                        m_enableTileStitchByHW = true;
    bool                                        m_useVirtualEngine = true;
    MOS_COMMAND_BUFFER                          m_veBatchBuffer[m_numUncompressedSurface][CODECHAL_ENCODE_VP9_MAX_NUM_HCP_PIPE][m_brcMaxNumPasses];
    MOS_COMMAND_BUFFER                          m_realCmdBuffer;
    uint32_t                                    m_sizeOfVEBatchBuffer = 0;
    uint8_t                                     m_virtualEngineBBIndex = 0;
    uint32_t                                    m_32BlocksRasterized = 0;
    PCODECHAL_ENCODE_SCALABILITY_STATE          m_scalabilityState = nullptr;   //!< Scalability state

    // Stats Integration regions
    CODECHAL_ENCODE_BUFFER                      m_hcpScalabilitySyncBuffer;
    CODECHAL_ENCODE_BUFFER                      m_tileRecordBuffer[m_numUncompressedSurface];
    CODECHAL_ENCODE_BUFFER                      m_tileStatsPakIntegrationBuffer[m_numUncompressedSurface];
    uint32_t                                    m_tileStatsPakIntegrationBufferSize = 0;
    CODECHAL_ENCODE_BUFFER                      m_frameStatsPakIntegrationBuffer;
    uint32_t                                    m_frameStatsPakIntegrationBufferSize = 0;
    MOS_RESOURCE                                m_hucPakIntDmemBuffer[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM][m_brcMaxNumPasses];
    MHW_VDBOX_HUC_VIRTUAL_ADDR_PARAMS           m_hpuVirtualAddrParams;
    MOS_RESOURCE                                m_hucPakIntDummyBuffer;
    MOS_RESOURCE                                m_hucPakIntBrcDataBuffer;
    StatsInfo                                   m_tileStatsOffset = {};  // Page aligned offsets for HuC PAK Integration kernel input
    StatsInfo                                   m_frameStatsOffset = {}; // Page aligned offsets for HuC PAK Integration kernel output
    StatsInfo                                   m_statsSize = {};        // Sizes for the stats for HuC PAK Integration kernel input
    // Semaphore memory for synchronizing
    MOS_RESOURCE                                m_resPipeStartSync;
    MOS_RESOURCE                                m_resFrameStartSync = {};
    CODECHAL_ENCODE_BUFFER                      m_stitchWaitSemaphoreMem[m_maxNumPipes];
    MOS_RESOURCE                                m_resDelayMinus;
    uint16_t                                    m_lastVdencPictureState2ndLevelBBIndex = 0;

    PMHW_VDBOX_HCP_TILE_CODING_PARAMS_G11 m_tileParams = nullptr;  //!< Pointer to the Tile params

    //!
    //! \brief    Constructor
    //!
    CodechalVdencVp9StateG11(CodechalHwInterface* hwInterface,
        CodechalDebugInterface* debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo);

    //!
    //! \brief    Destructor
    //!
    virtual ~CodechalVdencVp9StateG11();

    int GetCurrentPipe()
    {
        return (m_numPipe <= 1) ? 0 : (int)(m_currPass) % (int)m_numPipe;
    }

    int GetCurrentPass()
    {
        return (m_numPipe <= 1) ? m_currPass : (int)(m_currPass) / (int)m_numPipe;
    }

    int GetNumPasses()
    {
        return m_numPassesInOnePipe;
    }

    bool IsLastPipe()
    {
        return (GetCurrentPipe() == (m_numPipe - 1)) ? true : false;
    }

    bool IsFirstPipe()
    {
        return (GetCurrentPipe() == 0) ? true : false;
    }

    bool IsFirstPass()
    {
        return (GetCurrentPass() == 0) ? true : false;
    }

    bool IsLastPass()
    {
        return (GetCurrentPass() == m_numPassesInOnePipe) ? true : false;
    }

    bool UseLegacyCommandBuffer()
    {
        return ((!m_scalableMode) || (m_osInterface->pfnGetGpuContext(m_osInterface) == m_renderContext));
    }

    bool IsRenderContext()
    {
        return (m_osInterface->pfnGetGpuContext(m_osInterface) == m_renderContext);
    }
    MOS_STATUS VerifyCommandBufferSize();

    MOS_STATUS GetSystemPipeNumberCommon();

    MOS_STATUS InitKernelStateMe();

    MOS_STATUS SendMeSurfaces(
        PMOS_COMMAND_BUFFER cmdBuffer,
        MeSurfaceParams *   params);

    MOS_STATUS GetCommandBuffer(
        PMOS_COMMAND_BUFFER cmdBuffer);

    MOS_STATUS ReturnCommandBuffer(
        PMOS_COMMAND_BUFFER cmdBuffer);

    MOS_STATUS SubmitCommandBuffer(
        PMOS_COMMAND_BUFFER cmdBuffer,
        bool nullRendering);

    MOS_STATUS SetMeCurbeParams(
        MeCurbeParams* meParams);

    MOS_STATUS SetMeSurfaceParams(
        MeSurfaceParams* meSurfaceParams);

    MOS_STATUS SendPrologWithFrameTracking(
        PMOS_COMMAND_BUFFER cmdBuffer,
        bool frameTrackingRequested,
        MHW_MI_MMIOREGISTERS *mmioRegister = nullptr);

    MOS_STATUS SetSemaphoreMem(
        PMOS_RESOURCE semaphoreMem,
        PMOS_COMMAND_BUFFER cmdBuffer,
        uint32_t value);

    MOS_STATUS UserFeatureKeyReport();

    MOS_STATUS SendMIAtomicCmd(
        PMOS_RESOURCE               semaMem,
        uint32_t                    immData,
        MHW_COMMON_MI_ATOMIC_OPCODE opCode,
        PMOS_COMMAND_BUFFER         cmdBuffer
    );

    MOS_STATUS SendHWWaitCommand(
        PMOS_RESOURCE semaphoreMem,
        PMOS_COMMAND_BUFFER cmdBuffer,
        uint32_t value);

    MOS_STATUS ConstructPicStateBatchBuf(
        PMOS_RESOURCE picStateBuffer);

    MOS_STATUS SetDmemHuCPakInt();

    MOS_STATUS HuCVp9PakInt(
        PMOS_COMMAND_BUFFER cmdBuffer);

    MOS_STATUS HuCVp9Prob();

    MOS_STATUS HuCBrcUpdate();

    MOS_STATUS HuCBrcInitReset();

    MOS_STATUS SetHcpPipeBufAddrParams(MHW_VDBOX_PIPE_BUF_ADDR_PARAMS& pipeBufAddrParams,
        PMOS_SURFACE* refSurface,
        PMOS_SURFACE* refSurfaceNonScaled,
        PMOS_SURFACE* dsRefSurface4x,
        PMOS_SURFACE* dsRefSurface8x);

    uint16_t GetNumTilesInFrame();

    MOS_STATUS ExecutePictureLevel();

    MOS_STATUS SetSequenceStructs();

    MOS_STATUS SetPictureStructs();

    MOS_STATUS AllocateResources();

    void FreeResources();

    MOS_STATUS SetCurbeMe(
        MeCurbeParams *params);

    MOS_STATUS ExecuteKernelFunctions();

    MOS_STATUS ExecuteMeKernel(
        MeCurbeParams *  meParams,
        MeSurfaceParams *meSurfaceParams,
        HmeLevel         hmeLevel);

    MOS_STATUS SetupSegmentationStreamIn();

    void SetHcpPipeModeSelectParams(MHW_VDBOX_PIPE_MODE_SELECT_PARAMS& pipeModeSelectParams);

    void SetHcpIndObjBaseAddrParams(MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS& indObjBaseAddrParams);

    MOS_STATUS ExecuteSliceLevel();

    MOS_STATUS ExecuteDysSliceLevel();

    MOS_STATUS Initialize(CodechalSetting * settings);

    MOS_STATUS InitKernelStates();

    uint32_t GetMaxBtCount();

    MOS_STATUS InitKernelStateDys();

    MOS_STATUS ExecuteTileLevel();

    MOS_STATUS SetTileData();

    MOS_STATUS SetGpuCtxCreatOption();

    MOS_STATUS SetTileCommands(
        PMOS_COMMAND_BUFFER cmdBuffer);

    MOS_STATUS GetStatusReport(
        EncodeStatus*       encodeStatus,
        EncodeStatusReport* encodeStatusReport);

    MOS_STATUS DecideEncodingPipeNumber();

    MOS_STATUS PlatformCapabilityCheck();

    uint32_t GetSegmentBlockIndexInFrame(
        uint32_t frameWidth,
        uint32_t curr32XInTile,
        uint32_t curr32YInTile,
        uint32_t currTileStartY64aligned,
        uint32_t currTileStartX64aligned);

    MOS_STATUS InitZigZagToRasterLUTPerTile(
        uint32_t tileHeight,
        uint32_t tileWidth,
        uint32_t currTileStartYInFrame,
        uint32_t currTileStartXInFrame);

    MOS_STATUS CalculateVdencPictureStateCommandSize();

    PMHW_VDBOX_PIPE_BUF_ADDR_PARAMS CreateHcpPipeBufAddrParams(
        PMHW_VDBOX_PIPE_BUF_ADDR_PARAMS pipeBufAddrParams);

    MOS_STATUS UpdateCmdBufAttribute(
        PMOS_COMMAND_BUFFER cmdBuffer,
        bool                renderEngineInUse);

    //!
    //! \brief    Set And Populate VE Hint parameters
    //! \details  Set Virtual Engine hint parameter and populate it to primary cmd buffer attributes
    //! \param    [in] cmdBuffer
    //!               Pointer to primary cmd buffer
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS  SetAndPopulateVEHintParams(
        PMOS_COMMAND_BUFFER  cmdBuffer);

};
#endif  // __CODECHAL_VDENC_VP9_G11_H__
