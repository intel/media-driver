/*
* Copyright (c) 2017-2018, Intel Corporation
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
//! \file     codechal_encode_hevc_g9.cpp
//! \brief    HEVC dual-pipe encoder for GEN9.
//!

#include "codechal_encode_hevc_g9.h"
#include "codechal_kernel_hme_g9.h"
#include "igcodeckrn_g9.h"
#include "codeckrnheader.h"
#include "mhw_mmio_g9.h"

#define CS_ALU_COMMAND_LOAD(bSrcRegA, GprReg)           ((0x80 << 20) | (((bSrcRegA) ? 0x20 : 0x21) << 10) | ((GprReg) & 0x0F))
#define CS_ALU_COMMAND_STORE_ACCU(GprReg)               ((0x180 << 20) | (((GprReg) & 0x0F) << 10) | 0x31)


#define GPUMMU_WA_PADDING                               (64 * 1024)

//! HME step
enum
{
    HME_FIRST_STEP = 0,
    HME_FOLLOWING_STEP = 1
};

//! Motion vector shift factor
enum
{
    MV_SHIFT_FACTOR_32x = 1,
    MV_SHIFT_FACTOR_16x = 2,
    MV_SHIFT_FACTOR_4x = 2
};

//! Previous motion vector read position
enum
{
    PREV_MV_READ_POSITION_16x = 1,
    PREV_MV_READ_POSITION_4x = 0
};

//! ALU Opcode
enum
{
    CS_ALU_COMMAND_ADD = ((0x100) << 20),
    CS_ALU_COMMAND_SUB = ((0x101) << 20),
    CS_ALU_COMMAND_AND = ((0x102) << 20),
    CS_ALU_COMMAND_OR = ((0x103) << 20),
    CS_ALU_COMMAND_XOR = ((0x104) << 20)
};

// This ROI structure is defined in kernel for ROI surface calculations
// and differs slightly from the ENCODE_ROI structure used by DDI/App, so this
// will be used only for ROI surface loading
struct CODECHAL_ENC_HEVC_ROI_G9
{
    uint32_t Top;
    uint32_t Left;
    uint32_t Bottom;
    uint32_t Right;
    int32_t QPDelta;
    int32_t ROI_Level;
};

//! HEVC encoder ME kernel curbe for GEN9
struct CODECHAL_ENC_HEVC_ME_CURBE_G9
{
    // DW0
    union
    {
        struct
        {
            uint32_t   SkipModeEn                       : MOS_BITFIELD_BIT(0);
            uint32_t   AdaptiveEn                       : MOS_BITFIELD_BIT(1);
            uint32_t   BiMixDis                         : MOS_BITFIELD_BIT(2);
            uint32_t                                    : MOS_BITFIELD_RANGE(3, 4);
            uint32_t   EarlyImeSuccessEn                : MOS_BITFIELD_BIT(5);
            uint32_t                                    : MOS_BITFIELD_BIT(6);
            uint32_t   T8x8FlagForInterEn               : MOS_BITFIELD_BIT(7);
            uint32_t                                    : MOS_BITFIELD_RANGE(8, 23);
            uint32_t   EarlyImeStop                     : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   MaxNumMVs                        : MOS_BITFIELD_RANGE(0, 5);
            uint32_t                                    : MOS_BITFIELD_RANGE(6, 15);
            uint32_t   BiWeight                         : MOS_BITFIELD_RANGE(16, 21);
            uint32_t                                    : MOS_BITFIELD_RANGE(22, 27);
            uint32_t   UniMixDisable                    : MOS_BITFIELD_BIT(28);
            uint32_t                                    : MOS_BITFIELD_RANGE(29, 31);
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
            uint32_t   MaxLenSP                         : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   MaxNumSU                         : MOS_BITFIELD_RANGE(8, 15);
            uint32_t                                    : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   SrcSize                          : MOS_BITFIELD_RANGE(0, 1);
            uint32_t                                    : MOS_BITFIELD_RANGE(2, 3);
            uint32_t   MbTypeRemap                      : MOS_BITFIELD_RANGE(4, 5);
            uint32_t   SrcAccess                        : MOS_BITFIELD_BIT(6);
            uint32_t   RefAccess                        : MOS_BITFIELD_BIT(7);
            uint32_t   SearchCtrl                       : MOS_BITFIELD_RANGE(8, 10);
            uint32_t   DualSearchPathOption             : MOS_BITFIELD_BIT(11);
            uint32_t   SubPelMode                       : MOS_BITFIELD_RANGE(12, 13);
            uint32_t   SkipType                         : MOS_BITFIELD_BIT(14);
            uint32_t   DisableFieldCacheAlloc           : MOS_BITFIELD_BIT(15);
            uint32_t   InterChromaMode                  : MOS_BITFIELD_BIT(16);
            uint32_t   FTEnable                         : MOS_BITFIELD_BIT(17);
            uint32_t   BMEDisableFBR                    : MOS_BITFIELD_BIT(18);
            uint32_t   BlockBasedSkipEnable             : MOS_BITFIELD_BIT(19);
            uint32_t   InterSAD                         : MOS_BITFIELD_RANGE(20, 21);
            uint32_t   IntraSAD                         : MOS_BITFIELD_RANGE(22, 23);
            uint32_t   SubMbPartMask                    : MOS_BITFIELD_RANGE(24, 30);
            uint32_t                                    : MOS_BITFIELD_BIT(31);
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
            uint32_t                                    : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   PictureHeightMinus1              : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   PictureWidth                     : MOS_BITFIELD_RANGE(16, 23);
            uint32_t                                    : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t                                    : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   QpPrimeY                         : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   RefWidth                         : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   RefHeight                        : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t                                    : MOS_BITFIELD_RANGE(0, 2);
            uint32_t   WriteDistortions                 : MOS_BITFIELD_BIT(3);
            uint32_t   UseMvFromPrevStep                : MOS_BITFIELD_BIT(4);
            uint32_t                                    : MOS_BITFIELD_RANGE(5, 7);
            uint32_t   SuperCombineDist                 : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   MaxVmvR                          : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t                                    : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   MVCostScaleFactor                : MOS_BITFIELD_RANGE(16, 17);
            uint32_t   BilinearEnable                   : MOS_BITFIELD_BIT(18);
            uint32_t   SrcFieldPolarity                 : MOS_BITFIELD_BIT(19);
            uint32_t   WeightedSADHAAR                  : MOS_BITFIELD_BIT(20);
            uint32_t   AConlyHAAR                       : MOS_BITFIELD_BIT(21);
            uint32_t   RefIDCostMode                    : MOS_BITFIELD_BIT(22);
            uint32_t                                    : MOS_BITFIELD_BIT(23);
            uint32_t   SkipCenterMask                   : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   Mode0Cost                        : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   Mode1Cost                        : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   Mode2Cost                        : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   Mode3Cost                        : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   Mode4Cost                        : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   Mode5Cost                        : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   Mode6Cost                        : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   Mode7Cost                        : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   Mode8Cost                        : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   Mode9Cost                        : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   RefIDCost                        : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   ChromaIntraModeCost              : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   MV0Cost                          : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   MV1Cost                          : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   MV2Cost                          : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   MV3Cost                          : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   MV4Cost                          : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   MV5Cost                          : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   MV6Cost                          : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   MV7Cost                          : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   NumRefIdxL0MinusOne              : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   NumRefIdxL1MinusOne              : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   RefStreaminCost                  : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   ROIEnable                        : MOS_BITFIELD_RANGE(24, 26);
            uint32_t                                    : MOS_BITFIELD_RANGE(27, 31);
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
            uint32_t   List0RefID0FieldParity           : MOS_BITFIELD_BIT(0);
            uint32_t   List0RefID1FieldParity           : MOS_BITFIELD_BIT(1);
            uint32_t   List0RefID2FieldParity           : MOS_BITFIELD_BIT(2);
            uint32_t   List0RefID3FieldParity           : MOS_BITFIELD_BIT(3);
            uint32_t   List0RefID4FieldParity           : MOS_BITFIELD_BIT(4);
            uint32_t   List0RefID5FieldParity           : MOS_BITFIELD_BIT(5);
            uint32_t   List0RefID6FieldParity           : MOS_BITFIELD_BIT(6);
            uint32_t   List0RefID7FieldParity           : MOS_BITFIELD_BIT(7);
            uint32_t   List1RefID0FieldParity           : MOS_BITFIELD_BIT(8);
            uint32_t   List1RefID1FieldParity           : MOS_BITFIELD_BIT(9);
            uint32_t                                    : MOS_BITFIELD_RANGE(10, 31);
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
            uint32_t   PrevMvReadPosFactor              : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   MvShiftFactor                    : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   Reserved                         : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   ActualMBWidth                    : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   ActualMBHeight                   : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   Reserved;
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
            uint32_t   _4xMeMvOutputDataSurfIndex;
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
            uint32_t   _16xOr32xMeMvInputDataSurfIndex;
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
            uint32_t   _4xMeOutputDistSurfIndex;
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
            uint32_t   _4xMeOutputBrcDistSurfIndex;
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
            uint32_t   VMEFwdInterPredictionSurfIndex;
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
            uint32_t   VMEBwdInterPredictionSurfIndex;
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
            uint32_t   VDEncStreamInSurfIndex;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW38;
};

using PCODECHAL_ENC_HEVC_ME_CURBE_G9 = struct CODECHAL_ENC_HEVC_ME_CURBE_G9*;
C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(CODECHAL_ENC_HEVC_ME_CURBE_G9)) == 39);

//! HEVC encoder B MBEnc kernel curbe for GEN9
struct CODECHAL_ENC_HEVC_B_MB_ENC_CURBE_G9
{
    // DW0
    union
    {
        struct
        {
            uint32_t   SkipModeEn                       : MOS_BITFIELD_BIT(0);
            uint32_t   AdaptiveEn                       : MOS_BITFIELD_BIT(1);
            uint32_t   BiMixDis                         : MOS_BITFIELD_BIT(2);
            uint32_t                                    : MOS_BITFIELD_RANGE(3, 4);
            uint32_t   EarlyImeSuccessEn                : MOS_BITFIELD_BIT(5);
            uint32_t                                    : MOS_BITFIELD_BIT(6);
            uint32_t   T8x8FlagForInterEn               : MOS_BITFIELD_BIT(7);
            uint32_t                                    : MOS_BITFIELD_RANGE(8, 23);
            uint32_t   EarlyImeStop                     : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   MaxNumMVs                        : MOS_BITFIELD_RANGE(0, 5);
            uint32_t                                    : MOS_BITFIELD_RANGE(6, 15);
            uint32_t   BiWeight                         : MOS_BITFIELD_RANGE(16, 21);
            uint32_t                                    : MOS_BITFIELD_RANGE(22, 27);
            uint32_t   UniMixDisable                    : MOS_BITFIELD_BIT(28);
            uint32_t                                    : MOS_BITFIELD_RANGE(29, 31);
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
            uint32_t   LenSP                            : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   MaxNumSU                         : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   PicWidth                         : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   SrcSize                          : MOS_BITFIELD_RANGE(0, 1);
            uint32_t                                    : MOS_BITFIELD_RANGE(2, 3);
            uint32_t   MbTypeRemap                      : MOS_BITFIELD_RANGE(4, 5);
            uint32_t   SrcAccess                        : MOS_BITFIELD_BIT(6);
            uint32_t   RefAccess                        : MOS_BITFIELD_BIT(7);
            uint32_t   SearchCtrl                       : MOS_BITFIELD_RANGE(8, 10);
            uint32_t   DualSearchPathOption             : MOS_BITFIELD_BIT(11);
            uint32_t   SubPelMode                       : MOS_BITFIELD_RANGE(12, 13);
            uint32_t   SkipType                         : MOS_BITFIELD_BIT(14);
            uint32_t   DisableFieldCacheAlloc           : MOS_BITFIELD_BIT(15);
            uint32_t   InterChromaMode                  : MOS_BITFIELD_BIT(16);
            uint32_t   FTEnable                         : MOS_BITFIELD_BIT(17);
            uint32_t   BMEDisableFBR                    : MOS_BITFIELD_BIT(18);
            uint32_t   BlockBasedSkipEnable             : MOS_BITFIELD_BIT(19);
            uint32_t   InterSAD                         : MOS_BITFIELD_RANGE(20, 21);
            uint32_t   IntraSAD                         : MOS_BITFIELD_RANGE(22, 23);
            uint32_t   SubMbPartMask                    : MOS_BITFIELD_RANGE(24, 30);
            uint32_t                                    : MOS_BITFIELD_BIT(31);
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
            uint32_t   PicHeightMinus1                  : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   Res_16_22                        : MOS_BITFIELD_RANGE(16, 22);
            uint32_t   EnableQualityImprovement         : MOS_BITFIELD_BIT(23);
            uint32_t   EnableDebug                      : MOS_BITFIELD_BIT(24);
            uint32_t   EnableFlexibleParam              : MOS_BITFIELD_BIT(25);
            uint32_t   EnableStatsDataDump              : MOS_BITFIELD_BIT(26);
            uint32_t   Res_27                           : MOS_BITFIELD_BIT(27);
            uint32_t   HMEEnable                        : MOS_BITFIELD_BIT(28);
            uint32_t   SliceType                        : MOS_BITFIELD_RANGE(29, 30);
            uint32_t   UseActualRefQPValue              : MOS_BITFIELD_BIT(31);
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
            uint32_t   Res_0_15                         : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   RefWidth                         : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   RefHeight                        : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   FrameWidth                       : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   FrameHeight                      : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   IntraPartMask                    : MOS_BITFIELD_RANGE(0, 4);
            uint32_t   NonSkipZMvAdded                  : MOS_BITFIELD_BIT(5);
            uint32_t   NonSkipModeAdded                 : MOS_BITFIELD_BIT(6);
            uint32_t   LumaIntraSrcCornerSwap           : MOS_BITFIELD_BIT(7);
            uint32_t                                    : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   MVCostScaleFactor                : MOS_BITFIELD_RANGE(16, 17);
            uint32_t   BilinearEnable                   : MOS_BITFIELD_BIT(18);
            uint32_t   Res_19                           : MOS_BITFIELD_BIT(19);
            uint32_t   WeightedSADHAAR                  : MOS_BITFIELD_BIT(20);
            uint32_t   AConlyHAAR                       : MOS_BITFIELD_BIT(21);
            uint32_t   RefIDCostMode                    : MOS_BITFIELD_BIT(22);
            uint32_t                                    : MOS_BITFIELD_BIT(23);
            uint32_t   SkipCenterMask                   : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   Mode0Cost                        : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   Mode1Cost                        : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   Mode2Cost                        : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   Mode3Cost                        : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   Mode4Cost                        : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   Mode5Cost                        : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   Mode6Cost                        : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   Mode7Cost                        : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   Mode8Cost                        : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   Mode9Cost                        : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   RefIDCost                        : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   ChromaIntraModeCost              : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   MV0Cost                          : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   MV1Cost                          : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   MV2Cost                          : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   MV3Cost                          : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   MV4Cost                          : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   MV5Cost                          : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   MV6Cost                          : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   MV7Cost                          : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   QpPrimeY                         : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   QpPrimeCb                        : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   QpPrimeCr                        : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   TargetSizeInWord                 : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   SICFwdTransCoeffThreshold_0      : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   SICFwdTransCoeffThreshold_1      : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   SICFwdTransCoeffThreshold_2      : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   SICFwdTransCoeffThreshold_3      : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   SICFwdTransCoeffThreshold_4      : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   SICFwdTransCoeffThreshold_5      : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   SICFwdTransCoeffThreshold_6      : MOS_BITFIELD_RANGE(24, 31);    // Highest Freq
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
            uint32_t   Intra4x4ModeMask                 : MOS_BITFIELD_RANGE(0, 8);
            uint32_t                                    : MOS_BITFIELD_RANGE(9, 15);
            uint32_t   Intra8x8ModeMask                 : MOS_BITFIELD_RANGE(16, 24);
            uint32_t                                    : MOS_BITFIELD_RANGE(25, 31);
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
            uint32_t   Intra16x16ModeMask               : MOS_BITFIELD_RANGE(0, 3);
            uint32_t   IntraChromaModeMask              : MOS_BITFIELD_RANGE(4, 7);
            uint32_t   IntraComputeType                 : MOS_BITFIELD_RANGE(8, 9);
            uint32_t                                    : MOS_BITFIELD_RANGE(10, 31);
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
            uint32_t   SkipVal                          : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   MultiPredL0Disable               : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   MultiPredL1Disable               : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   Intra16x16NonDCPredPenalty       : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   Intra8x8NonDCPredPenalty         : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   Intra4x4NonDCPredPenalty         : MOS_BITFIELD_RANGE(16, 23);
            uint32_t                                    : MOS_BITFIELD_RANGE(24, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW33;

    union {
        struct {
            uint32_t       LambdaME;
        };
        uint32_t Value;
    } DW34;

    union {
        struct {
            uint32_t       SimpIntraInterThreshold      : MOS_BITFIELD_RANGE(0, 15);
            uint32_t       ModeCostSp                   : MOS_BITFIELD_RANGE(16, 23);
            uint32_t       IntraRefreshEn               : MOS_BITFIELD_RANGE(24, 25);
            uint32_t       FirstIntraRefresh            : MOS_BITFIELD_BIT(26);
            uint32_t       EnableRollingIntra           : MOS_BITFIELD_BIT(27);
            uint32_t       HalfUpdateMixedLCU           : MOS_BITFIELD_BIT(28);
            uint32_t       Res_29_31                    : MOS_BITFIELD_RANGE(29, 31);
        };
        uint32_t Value;
    } DW35;

    union {
        struct {
            uint32_t       NumRefIdxL0MinusOne          : MOS_BITFIELD_RANGE(0, 7);
            uint32_t       HMECombinedExtraSUs          : MOS_BITFIELD_RANGE(8, 15);
            uint32_t       NumRefIdxL1MinusOne          : MOS_BITFIELD_RANGE(16, 23);
            uint32_t       PowerSaving                  : MOS_BITFIELD_BIT(24);
            uint32_t       BRCEnable                    : MOS_BITFIELD_BIT(25);
            uint32_t       LCUBRCEnable                 : MOS_BITFIELD_BIT(26);
            uint32_t       ROIEnable                    : MOS_BITFIELD_BIT(27);
            uint32_t       FASTSurveillanceFlag         : MOS_BITFIELD_BIT(28);
            uint32_t       CheckAllFractionalEnable     : MOS_BITFIELD_BIT(29);
            uint32_t       HMECombinedOverlap           : MOS_BITFIELD_RANGE(30, 31);
        };
        uint32_t Value;
    } DW36;

    union {
        struct {
            uint32_t       ActualQpRefID0List0          : MOS_BITFIELD_RANGE(0, 7);
            uint32_t       ActualQpRefID1List0          : MOS_BITFIELD_RANGE(8, 15);
            uint32_t       ActualQpRefID2List0          : MOS_BITFIELD_RANGE(16, 23);
            uint32_t       ActualQpRefID3List0          : MOS_BITFIELD_RANGE(24, 31);
        };
        uint32_t Value;
    } DW37;

    union {
        struct {
            uint32_t       NumIntraRefreshOffFrames     : MOS_BITFIELD_RANGE(0, 15);
            uint32_t       NumFrameInGOB                : MOS_BITFIELD_RANGE(16, 31);
        };
        uint32_t Value;
    } DW38;

    union {
        struct {
            uint32_t       ActualQpRefID0List1          : MOS_BITFIELD_RANGE(0, 7);
            uint32_t       ActualQpRefID1List1          : MOS_BITFIELD_RANGE(8, 15);
            uint32_t       RefCost                      : MOS_BITFIELD_RANGE(16, 31);
        };
        uint32_t Value;
    } DW39;

    union {
        struct {
            uint32_t       Reserved;
        };
        uint32_t Value;
    } DW40;

    union {
        struct {
            uint32_t       TransformThreshold2          : MOS_BITFIELD_RANGE(0, 15);
            uint32_t       TextureIntraCostThreshold    : MOS_BITFIELD_RANGE(16, 31);
        };
        uint32_t Value;
    } DW41;

    union {
        struct {
            uint32_t       Reserved;
        };
        uint32_t Value;
    } DW42;

    union {
        struct {
            uint32_t       Reserved;
        };
        uint32_t Value;
    } DW43;

    union {
        struct {
            uint32_t       MaxNumMergeCandidates        : MOS_BITFIELD_RANGE(0, 3);
            uint32_t       MaxNumRefList0               : MOS_BITFIELD_RANGE(4, 7);
            uint32_t       MaxNumRefList1               : MOS_BITFIELD_RANGE(8, 11);
            uint32_t       Res_12_15                    : MOS_BITFIELD_RANGE(12, 15);
            uint32_t       MaxVmvR                      : MOS_BITFIELD_RANGE(16, 31);
        };
        uint32_t Value;
    } DW44;

    union {
        struct {
            uint32_t       TemporalMvpEnableFlag        : MOS_BITFIELD_BIT(0);
            uint32_t       Res_1_7                      : MOS_BITFIELD_RANGE(1, 7);
            uint32_t       Log2ParallelMergeLevel       : MOS_BITFIELD_RANGE(8, 15);
            uint32_t       HMECombineLenPslice          : MOS_BITFIELD_RANGE(16, 23);
            uint32_t       HMECombineLenBslice          : MOS_BITFIELD_RANGE(24, 31);
        };
        uint32_t Value;
    } DW45;

    union {
        struct {
            uint32_t       Log2MinTUSize                : MOS_BITFIELD_RANGE(0, 7);
            uint32_t       Log2MaxTUSize                : MOS_BITFIELD_RANGE(8, 15);
            uint32_t       Log2MinCUSize                : MOS_BITFIELD_RANGE(16, 23);
            uint32_t       Log2MaxCUSize                : MOS_BITFIELD_RANGE(24, 31);
        };
        uint32_t Value;
    } DW46;

    union {
        struct {
            uint32_t       NumRegionsInSlice            : MOS_BITFIELD_RANGE(0, 7);
            uint32_t       TypeOfWalkingPattern         : MOS_BITFIELD_RANGE(8, 11);
            uint32_t       ChromaFlatnessCheckFlag      : MOS_BITFIELD_BIT(12);
            uint32_t       EnableIntraEarlyExit         : MOS_BITFIELD_BIT(13);
            uint32_t       SkipIntraKrnFlag             : MOS_BITFIELD_BIT(14);
            uint32_t       ScreenContentFlag            : MOS_BITFIELD_BIT(15);
            uint32_t       IsLowDelay                   : MOS_BITFIELD_BIT(16);
            uint32_t       CollocatedFromL0Flag         : MOS_BITFIELD_BIT(17);
            uint32_t       ArbitarySliceFlag            : MOS_BITFIELD_BIT(18);
            uint32_t       MultiSliceFlag               : MOS_BITFIELD_BIT(19);
            uint32_t       Res_20_23                    : MOS_BITFIELD_RANGE(20, 23);
            uint32_t       isCurrRefL0LongTerm          : MOS_BITFIELD_BIT(24);
            uint32_t       isCurrRefL1LongTerm          : MOS_BITFIELD_BIT(25);
            uint32_t       NumRegionMinus1              : MOS_BITFIELD_RANGE(26, 31);
        };
        uint32_t Value;
    } DW47;

    union {
        struct {
            uint32_t       CurrentTdL0_0                : MOS_BITFIELD_RANGE(0, 15);
            uint32_t       CurrentTdL0_1                : MOS_BITFIELD_RANGE(16, 31);
        };
        uint32_t Value;
    } DW48;

    union {
        struct {
            uint32_t       CurrentTdL0_2                : MOS_BITFIELD_RANGE(0, 15);
            uint32_t       CurrentTdL0_3                : MOS_BITFIELD_RANGE(16, 31);
        };
        uint32_t Value;
    } DW49;

    union {
        struct {
            uint32_t       CurrentTdL1_0                : MOS_BITFIELD_RANGE(0, 15);
            uint32_t       CurrentTdL1_1                : MOS_BITFIELD_RANGE(16, 31);
        };
        uint32_t Value;
    } DW50;

    union {
        struct {
            uint32_t       IntraRefreshMBNum            : MOS_BITFIELD_RANGE(0, 15);
            uint32_t       IntraRefreshUnitInMB         : MOS_BITFIELD_RANGE(16, 23);
            uint32_t       IntraRefreshQPDelta          : MOS_BITFIELD_RANGE(24, 31);
        };
        uint32_t Value;
    } DW51;

    union {
        struct {
            uint32_t       NumofUnitInRegion            : MOS_BITFIELD_RANGE(0, 15);
            uint32_t       MaxHeightInRegion            : MOS_BITFIELD_RANGE(16, 31);
        };
        uint32_t Value;
    } DW52;

    union {
        struct {
            uint32_t       IntraRefreshRefWidth         : MOS_BITFIELD_RANGE(0, 7);
            uint32_t       IntraRefreshRefHeight        : MOS_BITFIELD_RANGE(8, 15);
            uint32_t       Res_16_31                    : MOS_BITFIELD_RANGE(16, 31);
        };
        uint32_t Value;
    } DW53;

    union {
        struct {
            uint32_t       Reserved;
        };
        uint32_t Value;
    } DW54;

    union {
        struct {
            uint32_t       Reserved;
        };
        uint32_t Value;
    } DW55;

    union {
        struct {
            uint32_t       BTI_CU_Record;
        };
        uint32_t Value;
    } DW56;

    union {
        struct {
            uint32_t       BTI_PAK_Cmd;
        };
        uint32_t Value;
    } DW57;

    union {
        struct {
            uint32_t       BTI_Src_Y;
        };
        uint32_t Value;
    } DW58;

    union {
        struct {
            uint32_t       BTI_Intra_Dist;
        };
        uint32_t Value;
    } DW59;

    union {
        struct {
            uint32_t       BTI_Min_Dist;
        };
        uint32_t Value;
    } DW60;

    union {
        struct {
            uint32_t       BTI_HMEMVPredFwdBwdSurfIndex;
        };
        uint32_t Value;
    } DW61;

    union {
        struct {
            uint32_t       BTI_HMEDistSurfIndex;
        };
        uint32_t Value;
    } DW62;

    union {
        struct {
            uint32_t       BTI_Slice_Map;
        };
        uint32_t Value;
    } DW63;

    union {
        struct {
            uint32_t       BTI_VME_Saved_UNI_SIC;
        };
        uint32_t Value;
    } DW64;

    union {
        struct {
            uint32_t       BTI_Simplest_Intra;
        };
        uint32_t Value;
    } DW65;

    union {
        struct {
            uint32_t       BTI_Collocated_RefFrame;
        };
        uint32_t Value;
    } DW66;

    union {
        struct {
            uint32_t       BTI_Reserved;
        };
        uint32_t Value;
    } DW67;

    union {
        struct {
            uint32_t       BTI_BRC_Input;
        };
        uint32_t Value;
    } DW68;

    union {
        struct {
            uint32_t       BTI_LCU_QP;
        };
        uint32_t Value;
    } DW69;

    union {
        struct {
            uint32_t       BTI_BRC_Data;
        };
        uint32_t Value;
    } DW70;

    union {
        struct {
            uint32_t       BTI_VMEInterPredictionSurfIndex;
        };
        uint32_t Value;
    } DW71;

    union {
        //For B frame
        struct {
            uint32_t       BTI_VMEInterPredictionBSurfIndex;
        };
        //For P frame
        struct {
            uint32_t       BTI_ConcurrentThreadMap;
        };
        uint32_t Value;
    } DW72;

    union {
        //For B frame
        struct {
            uint32_t       BTI_ConcurrentThreadMap;
        };
        //For P frame
        struct {
            uint32_t       BTI_MB_Data_CurFrame;
        };
        uint32_t Value;
    } DW73;

    union {
        //For B frame
        struct {
            uint32_t       BTI_MB_Data_CurFrame;
        };
        //For P frame
        struct {
            uint32_t       BTI_MVP_CurFrame;
        };
        uint32_t Value;
    } DW74;

    union {
        //For B frame
        struct {
            uint32_t       BTI_MVP_CurFrame;
        };
        //For P frame
        struct {
            uint32_t       BTI_Haar_Dist16x16;
        };
        uint32_t Value;
    } DW75;

    union {
        //For B frame
        struct {
            uint32_t       BTI_Haar_Dist16x16;
        };
        //For P frame
        struct {
            uint32_t       BTI_Stats_Data;
        };
        uint32_t Value;
    } DW76;

    union {
        //For B frame
        struct {
            uint32_t       BTI_Stats_Data;
        };
        //For P frame
        struct {
            uint32_t       BTI_Frame_Stats_Data;
        };
        uint32_t Value;
    } DW77;

    union {
        //For B frame
        struct {
            uint32_t       BTI_Frame_Stats_Data;
        };
        //For P frame
        struct {
            uint32_t       BTI_Debug;
        };
        uint32_t Value;
    } DW78;

    union {
        struct {
            uint32_t       BTI_Debug;
        };
        uint32_t Value;
    } DW79;
};

using PCODECHAL_ENC_HEVC_B_MB_ENC_CURBE_G9 = struct CODECHAL_ENC_HEVC_B_MB_ENC_CURBE_G9*;
C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(CODECHAL_ENC_HEVC_B_MB_ENC_CURBE_G9)) == 80);

//! HEVC encoder BRC init/reset curbe for GEN9
struct CODECHAL_ENC_HEVC_BRC_INITRESET_CURBE_G9
{
    union
    {
        struct
        {
            uint32_t   ProfileLevelMaxFrame;
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
            uint32_t   InitBufFull;
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
            uint32_t   BufSize;
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
            uint32_t   TargetBitRate;
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
            uint32_t   MaximumBitRate;
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
            uint32_t   MinimumBitRate;
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
            uint32_t   FrameRateM;
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
            uint32_t   FrameRateD;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW7;

    union
    {
        struct
        {
            uint32_t   BRCFlag                          : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   BRC_Param_A                      : MOS_BITFIELD_RANGE(16, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW8;

    union
    {
        struct
        {
            uint32_t   BRC_Param_B                      : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   FrameWidth                       : MOS_BITFIELD_RANGE(16, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW9;

    union
    {
        struct
        {
            uint32_t   FrameHeight                      : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   AVBRAccuracy                     : MOS_BITFIELD_RANGE(16, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW10;

    union
    {
        struct
        {
            uint32_t   AVBRConvergence                  : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   MinimumQP                        : MOS_BITFIELD_RANGE(16, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW11;

    union
    {
        struct
        {
            uint32_t   MaximumQP                        : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   NumberSlice                      : MOS_BITFIELD_RANGE(16, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW12;

    union
    {
        struct
        {
            uint32_t   reserved                         : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   BRC_Param_C                      : MOS_BITFIELD_RANGE(16, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW13;

    union
    {
        struct
        {
            uint32_t   BRC_Param_D                      : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   MaxBRCLevel                      : MOS_BITFIELD_RANGE(16, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW14;

    union
    {
        struct
        {
            uint32_t   reserved;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW15;

    union
    {
        struct
        {
            uint32_t   InstantRateThreshold0_Pframe     : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   InstantRateThreshold1_Pframe     : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   InstantRateThreshold2_Pframe     : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   InstantRateThreshold3_Pframe     : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   InstantRateThreshold0_Bframe     : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   InstantRateThreshold1_Bframe     : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   InstantRateThreshold2_Bframe     : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   InstantRateThreshold3_Bframe     : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   InstantRateThreshold0_Iframe     : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   InstantRateThreshold1_Iframe     : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   InstantRateThreshold2_Iframe     : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   InstantRateThreshold3_Iframe     : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   DeviationThreshold0_PBframe      : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   DeviationThreshold1_PBframe      : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   DeviationThreshold2_PBframe      : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   DeviationThreshold3_PBframe      : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   DeviationThreshold4_PBframe      : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   DeviationThreshold5_PBframe      : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   DeviationThreshold6_PBframe      : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   DeviationThreshold7_PBframe      : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   DeviationThreshold0_VBRcontrol   : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   DeviationThreshold1_VBRcontrol   : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   DeviationThreshold2_VBRcontrol   : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   DeviationThreshold3_VBRcontrol   : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   DeviationThreshold4_VBRcontrol   : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   DeviationThreshold5_VBRcontrol   : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   DeviationThreshold6_VBRcontrol   : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   DeviationThreshold7_VBRcontrol   : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   DeviationThreshold0_Iframe       : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   DeviationThreshold1_Iframe       : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   DeviationThreshold2_Iframe       : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   DeviationThreshold3_Iframe       : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   DeviationThreshold4_Iframe       : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   DeviationThreshold5_Iframe       : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   DeviationThreshold6_Iframe       : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   DeviationThreshold7_Iframe       : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   ACQPBuffer                       : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   IntraSADTransform                : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   Reserved0                        : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   Reserved1                        : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   reserved;
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
            uint32_t   reserved;
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
            uint32_t   reserved;
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
            uint32_t   reserved;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW29;

    union
    {
        struct
        {
            uint32_t   reserved;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW30;

    union
    {
        struct
        {
            uint32_t   reserved;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW31;
};

using PCODECHAL_ENC_HEVC_BRC_INITRESET_CURBE_G9 = struct CODECHAL_ENC_HEVC_BRC_INITRESET_CURBE_G9*;
C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(CODECHAL_ENC_HEVC_BRC_INITRESET_CURBE_G9)) == 32 );

//! HEVC encoder BRC update kernel curbe for GEN9
struct CODECHAL_ENC_HEVC_BRC_UPDATE_CURBE_G9
{
    union
    {
        struct
        {
            uint32_t   TARGETSIZE;
        };
        struct
        {
            uint32_t   Value;
        };
    }DW0;

    union
    {
        struct
        {
            uint32_t   FrameNumber;
        };
        struct
        {
            uint32_t   Value;
        };
    }DW1;

    union
    {
        struct
        {
            uint32_t   PictureHeaderSize;
        };
        struct
        {
            uint32_t   Value;
        };
    }DW2;

    union
    {
        struct
        {
            uint32_t   startGAdjFrame0                  : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   startGAdjFrame1                  : MOS_BITFIELD_RANGE(16, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    }DW3;

    union
    {
        struct
        {
            uint32_t   startGAdjFrame2                  : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   startGAdjFrame3                  : MOS_BITFIELD_RANGE(16, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    }DW4;

    union
    {
        struct
        {
            uint32_t   TARGETSIZE_FLAG                  : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   BRCFlag                          : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   MaxNumPAKs                       : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   CurrFrameType                    : MOS_BITFIELD_RANGE(24, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    }DW5;

    union
    {
        struct
        {
            uint32_t   NumSkippedFrames                 : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   CQPValue                         : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   ROIFlag                          : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   ROIRatio                         : MOS_BITFIELD_RANGE(24, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    }DW6;

    union
    {
        struct
        {
            uint32_t   FrameWidthInLCU                  : MOS_BITFIELD_RANGE(0,   7);
            uint32_t   Res_8_14                         : MOS_BITFIELD_RANGE(8,  14);
            uint32_t   KernelBuildControl               : MOS_BITFIELD_BIT(      15);
            uint32_t   ucMinQp                          : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   ucMaxQp                          : MOS_BITFIELD_RANGE(24, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    }DW7;

    union
    {
        struct
        {
            uint32_t   StartGlobalAdjustMult0           : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   StartGlobalAdjustMult1           : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   StartGlobalAdjustMult2           : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   StartGlobalAdjustMult3           : MOS_BITFIELD_RANGE(24, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    }DW8;

    union
    {
        struct
        {
            uint32_t   StartGlobalAdjustMult4           : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   StartGlobalAdjustDivd0           : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   StartGlobalAdjustDivd1           : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   StartGlobalAdjustDivd2           : MOS_BITFIELD_RANGE(24, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    }DW9;

    union
    {
        struct
        {
            uint32_t   StartGlobalAdjustDivd3           : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   StartGlobalAdjustDivd4           : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   QPThreshold0                     : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   QPThreshold1                     : MOS_BITFIELD_RANGE(24, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    }DW10;

    union
    {
        struct
        {
            uint32_t   QPThreshold2                     : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   QPThreshold3                     : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   gRateRatioThreshold0             : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   gRateRatioThreshold1             : MOS_BITFIELD_RANGE(24, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    }DW11;

    union
    {
        struct
        {
            uint32_t   gRateRatioThreshold2             : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   gRateRatioThreshold3             : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   gRateRatioThreshold4             : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   gRateRatioThreshold5             : MOS_BITFIELD_RANGE(24, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    }DW12;

    union
    {
        struct
        {
            uint32_t   gRateRatioThreshold6             : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   gRateRatioThreshold7             : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   gRateRatioThreshold8             : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   gRateRatioThreshold9             : MOS_BITFIELD_RANGE(24, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    }DW13;

    union
    {
        struct
        {
            uint32_t   gRateRatioThreshold10            : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   gRateRatioThreshold11            : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   gRateRatioThreshold12            : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   ParallelMode                     : MOS_BITFIELD_RANGE(24, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    }DW14;

    union
    {
        struct
        {
            uint32_t   SizeOfSkippedFrames;
        };
        struct
        {
            uint32_t   Value;
        };
    }DW15;
};

using PCODECHAL_ENC_HEVC_BRC_UPDATE_CURBE_G9 = struct CODECHAL_ENC_HEVC_BRC_UPDATE_CURBE_G9*;
C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(CODECHAL_ENC_HEVC_BRC_UPDATE_CURBE_G9)) == 16);

//! HEVC encoder coarse intra kernel curbe for GEN9
struct CODECHAL_ENC_HEVC_COARSE_INTRA_CURBE_G9
{
    union
    {
        struct
        {
            uint32_t   PictureWidthInLumaSamples        : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   PictureHeightInLumaSamples       : MOS_BITFIELD_RANGE(16, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    }DW0;

    union
    {
        struct
        {
            uint32_t   SrcSize                          : MOS_BITFIELD_RANGE(0, 1);
            uint32_t   Reserved0                        : MOS_BITFIELD_RANGE(2, 13);
            uint32_t   SkipType                         : MOS_BITFIELD_BIT(14);
            uint32_t   Reserved1                        : MOS_BITFIELD_BIT(15);
            uint32_t   InterChromaMode                  : MOS_BITFIELD_BIT(16);
            uint32_t   FTEnable                         : MOS_BITFIELD_BIT(17);
            uint32_t   Reserved2                        : MOS_BITFIELD_BIT(18);
            uint32_t   BlkSkipEnabled                   : MOS_BITFIELD_BIT(19);
            uint32_t   InterSAD                         : MOS_BITFIELD_RANGE(20, 21);
            uint32_t   IntraSAD                         : MOS_BITFIELD_RANGE(22, 23);
            uint32_t   Reserved3                        : MOS_BITFIELD_RANGE(24, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    }DW1;

    union
    {
        struct
        {
            uint32_t   IntraPartMask                    : MOS_BITFIELD_RANGE(0, 4);
            uint32_t   NonSkipZMvAdded                  : MOS_BITFIELD_BIT(5);
            uint32_t   NonSkipModeAdded                 : MOS_BITFIELD_BIT(6);
            uint32_t   IntraCornerSwap                  : MOS_BITFIELD_BIT(7);
            uint32_t   Reserved0                        : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   MVCostScaleFactor                : MOS_BITFIELD_RANGE(16, 17);
            uint32_t   BilinearEnable                   : MOS_BITFIELD_BIT(18);
            uint32_t   Reserved1                        : MOS_BITFIELD_BIT(19);
            uint32_t   WeightedSADHAAR                  : MOS_BITFIELD_BIT(20);
            uint32_t   AConlyHAAR                       : MOS_BITFIELD_BIT(21);
            uint32_t   RefIDCostMode                    : MOS_BITFIELD_BIT(22);
            uint32_t   Reserved2                        : MOS_BITFIELD_BIT(23);
            uint32_t   SkipCenterMask                   : MOS_BITFIELD_RANGE(24, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    }DW2;

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
    }DW3;

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
    }DW4;

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
    }DW5;

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
    }DW6;

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
    }DW7;

    union
    {
        struct
        {
            uint32_t BTI_Src_Y4;
        };
        struct
        {
            uint32_t   Value;
        };
    }DW8;

    union
    {
        struct
        {
            uint32_t BTI_Intra_Dist;
        };
        struct
        {
            uint32_t   Value;
        };
    }DW9;

    union
    {
        struct
        {
            uint32_t BTI_VME_Intra;
        };
        struct
        {
            uint32_t   Value;
        };
    }DW10;
};

using PCODECHAL_ENC_HEVC_COARSE_INTRA_CURBE_G9 = struct CODECHAL_ENC_HEVC_COARSE_INTRA_CURBE_G9*;
C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(CODECHAL_ENC_HEVC_COARSE_INTRA_CURBE_G9)) == 11 );

const uint8_t CodechalEncHevcStateG9::m_ftqBasedSkip[NUM_TARGET_USAGE_MODES] =
{
    0, 3, 3, 3, 3, 3, 3, 0
};

const uint8_t CodechalEncHevcStateG9::m_meMethod[NUM_TARGET_USAGE_MODES] =
{
    0, 4, 4, 4, 4, 4, 4, 6
};

const uint8_t CodechalEncHevcStateG9::m_superCombineDist[NUM_TARGET_USAGE_MODES + 1] =
{
    0, 1, 1, 5, 5, 5, 9, 9, 0
};

const uint16_t CodechalEncHevcStateG9::m_skipValB[2][2][64] =
{
    {
        // Block Based Skip = 0 and Transform Flag = 0
        {
            0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0024,
            0x0024, 0x0060, 0x0060, 0x0099, 0x0099, 0x00cf, 0x00cf, 0x0105,
            0x0105, 0x0141, 0x0141, 0x0183, 0x0183, 0x01ce, 0x01ce, 0x0228,
            0x0228, 0x0291, 0x0291, 0x030c, 0x030c, 0x039f, 0x039f, 0x0447,
            0x0447, 0x050d, 0x050d, 0x05f1, 0x05f1, 0x06f6, 0x06f6, 0x0822,
            0x0822, 0x0972, 0x0972, 0x0aef, 0x0aef, 0x0c96, 0x0c96, 0x0e70,
            0x0e70, 0x107a, 0x107a, 0x1284, 0x0000, 0x0000, 0x0000, 0x0000,
            0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
        },
        // Block Based Skip = 0 and Transform Flag = 1
        {
            0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0024,
            0x0024, 0x0060, 0x0060, 0x0099, 0x0099, 0x00cf, 0x00cf, 0x0105,
            0x0105, 0x0141, 0x0141, 0x0183, 0x0183, 0x01ce, 0x01ce, 0x0228,
            0x0228, 0x0291, 0x0291, 0x030c, 0x030c, 0x039f, 0x039f, 0x0447,
            0x0447, 0x050d, 0x050d, 0x05f1, 0x05f1, 0x06f6, 0x06f6, 0x0822,
            0x0822, 0x0972, 0x0972, 0x0aef, 0x0aef, 0x0c96, 0x0c96, 0x0e70,
            0x0e70, 0x107a, 0x107a, 0x1284, 0x0000, 0x0000, 0x0000, 0x0000,
            0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
        }
    },
    {
        // Block Based Skip = 1 and Transform Flag = 0
        {
            0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0006,
            0x0006, 0x0010, 0x0010, 0x0019, 0x0019, 0x0022, 0x0022, 0x002b,
            0x002b, 0x0035, 0x0035, 0x0040, 0x0040, 0x004d, 0x004d, 0x005c,
            0x005c, 0x006d, 0x006d, 0x0082, 0x0082, 0x009a, 0x009a, 0x00b6,
            0x00b6, 0x00d7, 0x00d7, 0x00fd, 0x00fd, 0x0129, 0x0129, 0x015b,
            0x015b, 0x0193, 0x0193, 0x01d2, 0x01d2, 0x0219, 0x0219, 0x0268,
            0x0268, 0x02bf, 0x02bf, 0x0316, 0x0000, 0x0000, 0x0000, 0x0000,
            0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
        },
        // Block Based Skip = 1 and Transform Flag = 1
        {
            0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x000c,
            0x000c, 0x0020, 0x0020, 0x0033, 0x0033, 0x0045, 0x0045, 0x0057,
            0x0057, 0x006b, 0x006b, 0x0081, 0x0081, 0x009a, 0x009a, 0x00b8,
            0x00b8, 0x00db, 0x00db, 0x0104, 0x0104, 0x0135, 0x0135, 0x016d,
            0x016d, 0x01af, 0x01af, 0x01fb, 0x01fb, 0x0252, 0x0252, 0x02b6,
            0x02b6, 0x0326, 0x0326, 0x03a5, 0x03a5, 0x0432, 0x0432, 0x04d0,
            0x04d0, 0x057e, 0x057e, 0x062c, 0x0000, 0x0000, 0x0000, 0x0000,
            0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
        }
    }
};

const double CodechalEncHevcStateG9::m_modeCostLut[3][12] = {
    //BPREDSLICE
    { 3.5, 4, 14, 40, 6.0, 3.25, 4.25, 0, 3.0, 1.0, 2.0, 0.0 },
    //PREDSLICE
    { 3.5, 4, 14, 35, 4.5, 1.32, 2.32, 0, 2.75, 0.0, 2.0, 0.0 },
    //INTRASLICE
    { 3.5, 0, 10.0, 30, 0, 0, 0, 0, 0, 0, 0, 0 }
};

const double CodechalEncHevcStateG9::m_mvCostLut[3][8] = {
    //BPREDSLICE
    { 0.0, 1.0, 1.0, 3.0, 5.0, 6.0, 7.0, 8.0 },
    //PREDSLICE
    { 0.0, 2.0, 2.5, 4.5, 5.0, 6.0, 7.0, 7.5 },
    //INTRASLICE
    { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 }
};

const uint32_t CodechalEncHevcStateG9::m_brcMvCostHaar[][416] =
{
    // I
    {
        0x0d040001, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff000000, 0x3e6c0535, 0x0d040001,
        0x0f050001, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff010101, 0x3e847641, 0x0f050001,
        0x19050002, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff020202, 0x3e94aefa, 0x19050002,
        0x1a060002, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff030303, 0x3ea6e43f, 0x1a060002,
        0x1b070002, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff040404, 0x3ebb5458, 0x1b070002,
        0x1c080002, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff050505, 0x3ed2452d, 0x1c080002,
        0x1e090003, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff060606, 0x3eec0535, 0x1e090003,
        0x280a0003, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff070707, 0x3f047641, 0x280a0003,
        0x290b0004, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff080808, 0x3f14aefa, 0x290b0004,
        0x2a0d0004, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff090909, 0x3f26e43f, 0x2a0d0004,
        0x2b0e0005, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff0a0a0a, 0x3f3b5458, 0x2b0e0005,
        0x2c180005, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff0b0b0b, 0x3f52452d, 0x2c180005,
        0x2e190006, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff0c0c0c, 0x3f6c0535, 0x2e190006,
        0x381a0007, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff0d0d0d, 0x3f847641, 0x381a0007,
        0x391c0008, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff0e0e0e, 0x3f94aefa, 0x391c0008,
        0x3a1d0009, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff0f0f0f, 0x3fa6e43f, 0x3a1d0009,
        0x3b1f000a, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff101010, 0x3fbb5458, 0x3b1f000a,
        0x3c28000b, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff111111, 0x3fd2452d, 0x3c28000b,
        0x3e29000c, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff121212, 0x3fec0535, 0x3e29000c,
        0x482a000e, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff131313, 0x40047641, 0x482a000e,
        0x492c0018, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff141414, 0x4014aefa, 0x492c0018,
        0x4a2d0019, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff151515, 0x4026e43f, 0x4a2d0019,
        0x4b2f001a, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff161616, 0x403b5458, 0x4b2f001a,
        0x4c38001b, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff171717, 0x4052452d, 0x4c38001b,
        0x4e39001d, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff181818, 0x406c0535, 0x4e39001d,
        0x583a001e, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff191919, 0x40847641, 0x583a001e,
        0x593c0028, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff1a1a1a, 0x4094aefa, 0x593c0028,
        0x5a3d0029, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff1b1b1b, 0x40a6e43f, 0x5a3d0029,
        0x5b3f002a, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff1c1c1c, 0x40bb5458, 0x5b3f002a,
        0x5c48002b, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff1d1d1d, 0x40d2452d, 0x5c48002b,
        0x5e49002d, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff1e1e1e, 0x40ec0535, 0x5e49002d,
        0x684a002e, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff1f1f1f, 0x41047641, 0x684a002e,
        0x694c0038, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff202020, 0x4114aefa, 0x694c0038,
        0x6a4d0039, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff212121, 0x4126e43f, 0x6a4d0039,
        0x6b4f003a, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff222222, 0x413b5458, 0x6b4f003a,
        0x6c58003b, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff232323, 0x4152452d, 0x6c58003b,
        0x6e59003d, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff242424, 0x416c0535, 0x6e59003d,
        0x785a003e, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff252525, 0x41847641, 0x785a003e,
        0x795c0048, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff262626, 0x4194aefa, 0x795c0048,
        0x7a5d0049, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff272727, 0x41a6e43f, 0x7a5d0049,
        0x7b5f004a, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff282828, 0x41bb5458, 0x7b5f004a,
        0x7c68004b, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff292929, 0x41d2452d, 0x7c68004b,
        0x7e69004d, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff2a2a2a, 0x41ec0535, 0x7e69004d,
        0x886a004e, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff2b2b2b, 0x42047641, 0x886a004e,
        0x896c0058, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff2c2c2c, 0x4214aefa, 0x896c0058,
        0x8a6d0059, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff2d2d2d, 0x4226e43f, 0x8a6d0059,
        0x8b6f005a, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff2e2e2e, 0x423b5458, 0x8b6f005a,
        0x8c78005b, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff2f2f2f, 0x4252452d, 0x8c78005b,
        0x8e79005d, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff303030, 0x426c0535, 0x8e79005d,
        0x8f7a005e, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff313131, 0x42847641, 0x8f7a005e,
        0x8f7c0068, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff323232, 0x4294aefa, 0x8f7c0068,
        0x8f7d0069, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff333333, 0x42a6e43f, 0x8f7d0069
    },
    // P
    {
        0x391e0807, 0x00040209, 0x00040005, 0x09050400, 0x0f0e0c0a, 0x003b003e, 0x3f800000, 0x391e0807,
        0x391e0807, 0x00040209, 0x00040005, 0x09050400, 0x0f0e0c0a, 0x013b003e, 0x3f800000, 0x391e0807,
        0x391e0807, 0x00040209, 0x00040005, 0x09050400, 0x0f0e0c0a, 0x023b003e, 0x3f800000, 0x391e0807,
        0x391e0807, 0x00040209, 0x00040005, 0x09050400, 0x0f0e0c0a, 0x033b003e, 0x3f800000, 0x391e0807,
        0x391e0807, 0x00040209, 0x00040005, 0x09050400, 0x0f0e0c0a, 0x043b003e, 0x3f800000, 0x391e0807,
        0x391e0807, 0x00040209, 0x00040005, 0x09050400, 0x0f0e0c0a, 0x053b003e, 0x3f800000, 0x391e0807,
        0x391e0807, 0x00040209, 0x00040005, 0x09050400, 0x0f0e0c0a, 0x063b003e, 0x3f800000, 0x391e0807,
        0x391e0807, 0x00040209, 0x00040005, 0x09050400, 0x0f0e0c0a, 0x073b003e, 0x3f800000, 0x391e0807,
        0x391e0807, 0x00040209, 0x00040005, 0x09050400, 0x0f0e0c0a, 0x083b003e, 0x3f800000, 0x391e0807,
        0x391e0807, 0x00040209, 0x00040005, 0x09050400, 0x0f0e0c0a, 0x093b003e, 0x3f800000, 0x391e0807,
        0x391e0807, 0x00040209, 0x00040005, 0x09050400, 0x0f0e0c0a, 0x0a3b003e, 0x3f800000, 0x391e0807,
        0x391e0807, 0x00040209, 0x00040005, 0x09050400, 0x0f0e0c0a, 0x0b3b003e, 0x3f800000, 0x391e0807,
        0x391e0807, 0x00040209, 0x00040005, 0x09050400, 0x0f0e0c0a, 0x0c3b003e, 0x3f800000, 0x391e0807,
        0x391e0807, 0x00040209, 0x00040005, 0x09050400, 0x0f0e0c0a, 0x0d3b003e, 0x3f800000, 0x391e0807,
        0x391e0807, 0x00040209, 0x00040005, 0x09050400, 0x0f0e0c0a, 0x0e3b003e, 0x3f800000, 0x391e0807,
        0x391e0807, 0x00040209, 0x00040005, 0x09050400, 0x0f0e0c0a, 0x0f3b003e, 0x3f800000, 0x391e0807,
        0x492e180e, 0x00090519, 0x0008000b, 0x190a0800, 0x1f1e1c1a, 0x104b007c, 0x40000000, 0x492e180e,
        0x492e180e, 0x00090519, 0x0008000b, 0x190a0800, 0x1f1e1c1a, 0x114b007c, 0x40000000, 0x492e180e,
        0x492e180e, 0x00090519, 0x0008000b, 0x190a0800, 0x1f1e1c1a, 0x124b007c, 0x40000000, 0x492e180e,
        0x492e180e, 0x00090519, 0x0008000b, 0x190a0800, 0x1f1e1c1a, 0x134b007c, 0x40000000, 0x492e180e,
        0x4d3b1c1b, 0x000d071e, 0x000c0018, 0x1e0f0c00, 0x2b2b291f, 0x145800ba, 0x40400000, 0x4d3b1c1b,
        0x4d3b1c1b, 0x000d071e, 0x000c0018, 0x1e0f0c00, 0x2b2b291f, 0x155800ba, 0x40400000, 0x4d3b1c1b,
        0x4d3b1c1b, 0x000d071e, 0x000c0018, 0x1e0f0c00, 0x2b2b291f, 0x165800ba, 0x40400000, 0x4d3b1c1b,
        0x593e281e, 0x00190a29, 0x0018001b, 0x291a1800, 0x2f2e2c2a, 0x175b00f8, 0x40800000, 0x593e281e,
        0x593e281e, 0x00190a29, 0x0018001b, 0x291a1800, 0x2f2e2c2a, 0x185b00f8, 0x40800000, 0x593e281e,
        0x593e281e, 0x00190a29, 0x0018001b, 0x291a1800, 0x2f2e2c2a, 0x195b00f8, 0x40800000, 0x593e281e,
        0x5b492a29, 0x001c0d2b, 0x001a001e, 0x2b1d1a00, 0x39392f2d, 0x1a5e0136, 0x40a00000, 0x5b492a29,
        0x5d4b2c2b, 0x001e0f2e, 0x001c0028, 0x2e1f1c00, 0x3b3b392f, 0x1b680174, 0x40c00000, 0x5d4b2c2b,
        0x5d4b2c2b, 0x001e0f2e, 0x001c0028, 0x2e1f1c00, 0x3b3b392f, 0x1c680174, 0x40c00000, 0x5d4b2c2b,
        0x5f4c2e2c, 0x00281938, 0x001e002a, 0x38291e00, 0x3d3c3b39, 0x1d6a01b2, 0x40e00000, 0x5f4c2e2c,
        0x694e382e, 0x00291b39, 0x0028002b, 0x392a2800, 0x3f3e3c3a, 0x1e6b01f0, 0x41000000, 0x694e382e,
        0x6a583938, 0x002a1c3a, 0x0029002c, 0x3a2b2900, 0x48483e3b, 0x1f6d022e, 0x41100000, 0x6a583938,
        0x6b593a39, 0x002c1d3b, 0x002a002e, 0x3b2d2a00, 0x49493f3d, 0x206e026c, 0x41200000, 0x6b593a39,
        0x6c5a3b3a, 0x002d1f3c, 0x002b002f, 0x3c2e2b00, 0x4a4a483e, 0x216f02aa, 0x41300000, 0x6c5a3b3a,
        0x6e5b3d3b, 0x002f293f, 0x002d0039, 0x3f382d00, 0x4c4b4a48, 0x22790326, 0x41500000, 0x6e5b3d3b,
        0x6f5c3e3c, 0x00382948, 0x002e003a, 0x48392e00, 0x4d4c4b49, 0x237a0364, 0x41600000, 0x6f5c3e3c,
        0x795e483e, 0x00392b49, 0x0038003b, 0x493a3800, 0x4f4e4c4a, 0x247b03e0, 0x41800000, 0x795e483e,
        0x7a684948, 0x003a2c4a, 0x0039003c, 0x4a3b3900, 0x58584e4b, 0x257d045c, 0x41900000, 0x7a684948,
        0x7b694a49, 0x003c2d4b, 0x003a003e, 0x4b3d3a00, 0x59594f4d, 0x267e04d8, 0x41a00000, 0x7b694a49,
        0x7d6a4c4a, 0x003d2f4d, 0x003c0048, 0x4d3e3c00, 0x5b5a594e, 0x27880592, 0x41b80000, 0x7d6a4c4a,
        0x7e6b4d4b, 0x003e384e, 0x003d0049, 0x4e483d00, 0x5c5b5958, 0x2889060e, 0x41c80000, 0x7e6b4d4b,
        0x886d4f4d, 0x00483a58, 0x003f004a, 0x58493f00, 0x5e5d5b59, 0x298a0706, 0x41e80000, 0x886d4f4d,
        0x896e584e, 0x00493b59, 0x0048004b, 0x594a4800, 0x5f5e5c5a, 0x2a8b07c0, 0x42000000, 0x896e584e,
        0x8a785958, 0x004a3c5a, 0x0049004c, 0x5a4b4900, 0x68685e5b, 0x2b8d08b8, 0x42100000, 0x8a785958,
        0x8b795a59, 0x004c3d5b, 0x004a004e, 0x5b4d4a00, 0x69695f5d, 0x2c8e09b0, 0x42200000, 0x8b795a59,
        0x8c7a5b5a, 0x004d3f5d, 0x004b004f, 0x5d4e4b00, 0x6b6a685e, 0x2d8f0ae6, 0x42340000, 0x8c7a5b5a,
        0x8e7b5d5b, 0x004f485e, 0x004d0059, 0x5e584d00, 0x6c6b6a68, 0x2e8f0c5a, 0x424c0000, 0x8e7b5d5b,
        0x8f7c5e5c, 0x00584968, 0x004e005a, 0x68594e00, 0x6d6c6b69, 0x2f8f0dce, 0x42640000, 0x8f7c5e5c,
        0x8f7e685e, 0x00594b69, 0x0058005b, 0x695a5800, 0x6f6e6c6a, 0x308f0f80, 0x42800000, 0x8f7e685e,
        0x8f886968, 0x005a4c6a, 0x0059005c, 0x6a5b5900, 0x6f6f6e6b, 0x318f1170, 0x42900000, 0x8f886968,
        0x8f896a69, 0x005c4d6b, 0x005a005e, 0x6b5d5a00, 0x6f6f6f6d, 0x328f139e, 0x42a20000, 0x8f896a69,
        0x8f8a6b6a, 0x005d4f6d, 0x005b0068, 0x6d5e5b00, 0x6f6f6f6e, 0x338f160a, 0x42b60000, 0x8f8a6b6a
    },
    // B
    {
        0x3a1e0807, 0x0008060c, 0x00040206, 0x06020200, 0x180e0c0a, 0x003b0048, 0x3f800000, 0x3a1e0807,
        0x3a1e0807, 0x0008060c, 0x00040206, 0x06020200, 0x180e0c0a, 0x013b0048, 0x3f800000, 0x3a1e0807,
        0x3a1e0807, 0x0008060c, 0x00040206, 0x06020200, 0x180e0c0a, 0x023b0048, 0x3f800000, 0x3a1e0807,
        0x3a1e0807, 0x0008060c, 0x00040206, 0x06020200, 0x180e0c0a, 0x033b0048, 0x3f800000, 0x3a1e0807,
        0x3a1e0807, 0x0008060c, 0x00040206, 0x06020200, 0x180e0c0a, 0x043b0048, 0x3f800000, 0x3a1e0807,
        0x3a1e0807, 0x0008060c, 0x00040206, 0x06020200, 0x180e0c0a, 0x053b0048, 0x3f800000, 0x3a1e0807,
        0x3a1e0807, 0x0008060c, 0x00040206, 0x06020200, 0x180e0c0a, 0x063b0048, 0x3f800000, 0x3a1e0807,
        0x3a1e0807, 0x0008060c, 0x00040206, 0x06020200, 0x180e0c0a, 0x073b0048, 0x3f800000, 0x3a1e0807,
        0x3a1e0807, 0x0008060c, 0x00040206, 0x06020200, 0x180e0c0a, 0x083b0048, 0x3f800000, 0x3a1e0807,
        0x3a1e0807, 0x0008060c, 0x00040206, 0x06020200, 0x180e0c0a, 0x093b0048, 0x3f800000, 0x3a1e0807,
        0x3a1e0807, 0x0008060c, 0x00040206, 0x06020200, 0x180e0c0a, 0x0a3b0048, 0x3f800000, 0x3a1e0807,
        0x3a1e0807, 0x0008060c, 0x00040206, 0x06020200, 0x180e0c0a, 0x0b3b0048, 0x3f800000, 0x3a1e0807,
        0x3a1e0807, 0x0008060c, 0x00040206, 0x06020200, 0x180e0c0a, 0x0c3b0048, 0x3f800000, 0x3a1e0807,
        0x3a1e0807, 0x0008060c, 0x00040206, 0x06020200, 0x180e0c0a, 0x0d3b0048, 0x3f800000, 0x3a1e0807,
        0x3a1e0807, 0x0008060c, 0x00040206, 0x06020200, 0x180e0c0a, 0x0e3b0048, 0x3f800000, 0x3a1e0807,
        0x3a1e0807, 0x0008060c, 0x00040206, 0x06020200, 0x180e0c0a, 0x0f3b0048, 0x3f800000, 0x3a1e0807,
        0x4a2e180e, 0x00190d1c, 0x0008040c, 0x0c040400, 0x281e1c1a, 0x104b0090, 0x40000000, 0x4a2e180e,
        0x4a2e180e, 0x00190d1c, 0x0008040c, 0x0c040400, 0x281e1c1a, 0x114b0090, 0x40000000, 0x4a2e180e,
        0x4a2e180e, 0x00190d1c, 0x0008040c, 0x0c040400, 0x281e1c1a, 0x124b0090, 0x40000000, 0x4a2e180e,
        0x4a2e180e, 0x00190d1c, 0x0008040c, 0x0c040400, 0x281e1c1a, 0x134b0090, 0x40000000, 0x4a2e180e,
        0x4f3b1c1b, 0x001d1a29, 0x000c0619, 0x19060600, 0x2c2b291f, 0x145800d8, 0x40400000, 0x4f3b1c1b,
        0x4f3b1c1b, 0x001d1a29, 0x000c0619, 0x19060600, 0x2c2b291f, 0x155800d8, 0x40400000, 0x4f3b1c1b,
        0x4f3b1c1b, 0x001d1a29, 0x000c0619, 0x19060600, 0x2c2b291f, 0x165800d8, 0x40400000, 0x4f3b1c1b,
        0x5a3e281e, 0x00291d2c, 0x0018081c, 0x1c080800, 0x382e2c2a, 0x175b0120, 0x40800000, 0x5a3e281e,
        0x5a3e281e, 0x00291d2c, 0x0018081c, 0x1c080800, 0x382e2c2a, 0x185b0120, 0x40800000, 0x5a3e281e,
        0x5a3e281e, 0x00291d2c, 0x0018081c, 0x1c080800, 0x382e2c2a, 0x195b0120, 0x40800000, 0x5a3e281e,
        0x5d492a29, 0x002b282f, 0x001a0a1f, 0x1f0a0a00, 0x3a392f2d, 0x1a5e0168, 0x40a00000, 0x5d492a29,
        0x5f4b2c2b, 0x002d2a39, 0x001c0c29, 0x290c0c00, 0x3c3b392f, 0x1b6801b0, 0x40c00000, 0x5f4b2c2b,
        0x5f4b2c2b, 0x002d2a39, 0x001c0c29, 0x290c0c00, 0x3c3b392f, 0x1c6801b0, 0x40c00000, 0x5f4b2c2b,
        0x694c2e2c, 0x002f2b3b, 0x001e0e2b, 0x2b0e0e00, 0x3e3c3b39, 0x1d6a01f8, 0x40e00000, 0x694c2e2c,
        0x6a4e382e, 0x00392d3c, 0x0028182c, 0x2c181800, 0x483e3c3a, 0x1e6b0240, 0x41000000, 0x6a4e382e,
        0x6b583938, 0x003a2f3e, 0x0029192e, 0x2e191900, 0x49483e3b, 0x1f6d0288, 0x41100000, 0x6b583938,
        0x6d593a39, 0x003b383f, 0x002a1a2f, 0x2f1a1a00, 0x4a493f3d, 0x206e02d0, 0x41200000, 0x6d593a39,
        0x6e5a3b3a, 0x003c3948, 0x002b1b38, 0x381b1b00, 0x4b4a483e, 0x216f0318, 0x41300000, 0x6e5a3b3a,
        0x785b3d3b, 0x003e3b4a, 0x002d1d3a, 0x3a1d1d00, 0x4d4b4a48, 0x227903a8, 0x41500000, 0x785b3d3b,
        0x795c3e3c, 0x003f3b4b, 0x002e1e3b, 0x3b1e1e00, 0x4e4c4b49, 0x237a03f0, 0x41600000, 0x795c3e3c,
        0x7a5e483e, 0x00493d4c, 0x0038283c, 0x3c282800, 0x584e4c4a, 0x247b0480, 0x41800000, 0x7a5e483e,
        0x7b684948, 0x004a3f4e, 0x0039293e, 0x3e292900, 0x59584e4b, 0x257d0510, 0x41900000, 0x7b684948,
        0x7d694a49, 0x004b484f, 0x003a2a3f, 0x3f2a2a00, 0x5a594f4d, 0x267e05a0, 0x41a00000, 0x7d694a49,
        0x7e6a4c4a, 0x004c4959, 0x003c2c49, 0x492c2c00, 0x5c5a594e, 0x27880678, 0x41b80000, 0x7e6a4c4a,
        0x886b4d4b, 0x004d4a59, 0x003d2d49, 0x492d2d00, 0x5d5b5958, 0x28890708, 0x41c80000, 0x886b4d4b,
        0x896d4f4d, 0x004f4c5b, 0x003f2f4b, 0x4b2f2f00, 0x5f5d5b59, 0x298a0828, 0x41e80000, 0x896d4f4d,
        0x8a6e584e, 0x00594d5c, 0x0048384c, 0x4c383800, 0x685e5c5a, 0x2a8b0900, 0x42000000, 0x8a6e584e,
        0x8b785958, 0x005a4f5e, 0x0049394e, 0x4e393900, 0x69685e5b, 0x2b8d0a20, 0x42100000, 0x8b785958,
        0x8d795a59, 0x005b585f, 0x004a3a4f, 0x4f3a3a00, 0x6a695f5d, 0x2c8e0b40, 0x42200000, 0x8d795a59,
        0x8e7a5b5a, 0x005c5968, 0x004b3b58, 0x583b3b00, 0x6b6a685e, 0x2d8f0ca8, 0x42340000, 0x8e7a5b5a,
        0x8f7b5d5b, 0x005e5a6a, 0x004d3d5a, 0x5a3d3d00, 0x6d6b6a68, 0x2e8f0e58, 0x424c0000, 0x8f7b5d5b,
        0x8f7c5e5c, 0x005f5c6b, 0x004e3e5b, 0x5b3e3e00, 0x6e6c6b69, 0x2f8f1008, 0x42640000, 0x8f7c5e5c,
        0x8f7e685e, 0x00695d6c, 0x0058485c, 0x5c484800, 0x6f6e6c6a, 0x308f1200, 0x42800000, 0x8f7e685e,
        0x8f886968, 0x006a5f6e, 0x0059495e, 0x5e494900, 0x6f6f6e6b, 0x318f1440, 0x42900000, 0x8f886968,
        0x8f896a69, 0x006b686f, 0x005a4a5f, 0x5f4a4a00, 0x6f6f6f6d, 0x328f16c8, 0x42a20000, 0x8f896a69,
        0x8f8a6b6a, 0x006c6979, 0x005b4b69, 0x694b4b00, 0x6f6f6f6e, 0x338f1998, 0x42b60000, 0x8f8a6b6a
    }
};

const uint32_t CodechalEncHevcStateG9::m_brcLambdaHaar[QP_NUM * 4] = {
    0x00000036, 0x00000024, 0x00000075, 0x00000800, 0x00000044, 0x0000002d, 0x00000084, 0x00000800,
    0x00000056, 0x00000039, 0x00000094, 0x00000800, 0x0000006c, 0x00000048, 0x000000a6, 0x00000800,
    0x00000089, 0x0000005b, 0x000000ba, 0x00000800, 0x000000ac, 0x00000073, 0x000000d1, 0x00000800,
    0x000000d9, 0x00000091, 0x000000eb, 0x00000800, 0x00000112, 0x000000b7, 0x00000108, 0x00000800,
    0x00000159, 0x000000e7, 0x00000128, 0x00000800, 0x000001b3, 0x00000123, 0x0000014d, 0x00000800,
    0x00000224, 0x0000016f, 0x00000175, 0x00000800, 0x000002b2, 0x000001cf, 0x000001a3, 0x00000800,
    0x00000366, 0x00000247, 0x000001d7, 0x00000800, 0x00000448, 0x000002df, 0x00000210, 0x00000800,
    0x00000565, 0x0000039e, 0x00000251, 0x00000800, 0x000006cc, 0x0000048f, 0x0000029a, 0x00000800,
    0x00000891, 0x000005be, 0x000002eb, 0x00001000, 0x00000acb, 0x0000073d, 0x00000347, 0x00001000,
    0x00000d99, 0x0000091e, 0x000003ae, 0x00001000, 0x00001122, 0x00000b7d, 0x00000421, 0x00001000,
    0x00001596, 0x00000e7a, 0x000004a2, 0x00001800, 0x00001b33, 0x0000123d, 0x00000534, 0x00001800,
    0x00002245, 0x000016fb, 0x000005d7, 0x00001800, 0x00002b2d, 0x00001cf4, 0x000014cf, 0x00002000,
    0x00003666, 0x0000247a, 0x0000275c, 0x00002000, 0x0000448a, 0x00002df6, 0x00003e23, 0x00002000,
    0x0000565a, 0x000039e8, 0x000059e8, 0x00002800, 0x00006ccc, 0x000048f5, 0x00007b8b, 0x00003000,
    0x00008914, 0x00005bec, 0x0000a412, 0x00003000, 0x0000acb5, 0x000073d1, 0x0000d4ac, 0x00003800,
    0x0000d999, 0x000091eb, 0x00010eb8, 0x00004000, 0x00011228, 0x0000b7d9, 0x000153ca, 0x00004800,
    0x0001596b, 0x0000e7a2, 0x0001a5b8, 0x00005000, 0x0001b333, 0x000123d7, 0x0002069e, 0x00005800,
    0x00022451, 0x00016fb2, 0x000278ed, 0x00006800, 0x0002b2d6, 0x0001cf44, 0x0002ff74, 0x00007000,
    0x00036666, 0x000247ae, 0x00039d70, 0x00008000, 0x000448a2, 0x0002df64, 0x00043590, 0x00009000,
    0x000565ac, 0x00039e88, 0x0004b986, 0x0000a000, 0x0006cccc, 0x00048f5c, 0x00054da5, 0x0000b800,
    0x00089145, 0x0005bec8, 0x0005f3e7, 0x0000c800, 0x000acb59, 0x00073d11, 0x0006ae86, 0x0000e800,
    0x000d9999, 0x00091eb8, 0x00078000, 0x00010000, 0x0011228a, 0x000b7d90, 0x00086b20, 0x00012000,
    0x001596b2, 0x000e7a23, 0x0009730c, 0x00014000, 0x001b3333, 0x00123d70, 0x000a9b4a, 0x00016800,
    0x00224515, 0x0016fb20, 0x000be7cf, 0x00019800, 0x002b2d64, 0x001cf446, 0x000d5d0d, 0x0001c800,
    0x00366666, 0x00247ae1, 0x000f0000, 0x00020000, 0x00448a2a, 0x002df640, 0x0010d641, 0x00024000,
    0x00565ac8, 0x0039e88c, 0x0012e618, 0x00028800, 0x006ccccc, 0x0048f5c2, 0x00153694, 0x0002d800
};

const uint16_t CodechalEncHevcStateG9::m_skipThread[][QP_NUM] = {
    {
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0024,
        0x0024, 0x0060, 0x0060, 0x0099, 0x0099, 0x00cf, 0x00cf, 0x0105,
        0x0105, 0x0141, 0x0141, 0x0183, 0x0183, 0x01ce, 0x01ce, 0x0228,
        0x0228, 0x0291, 0x0291, 0x030c, 0x030c, 0x039f, 0x039f, 0x0447,
        0x0447, 0x050d, 0x050d, 0x05f1, 0x05f1, 0x06f6, 0x06f6, 0x0822,
        0x0822, 0x0972, 0x0972, 0x0aef, 0x0aef, 0x0c96, 0x0c96, 0x0e70,
        0x0e70, 0x107a, 0x107a, 0x1284
    },

    {
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x000c,
        0x000c, 0x0020, 0x0020, 0x0033, 0x0033, 0x0045, 0x0045, 0x0057,
        0x0057, 0x006b, 0x006b, 0x0081, 0x0081, 0x009a, 0x009a, 0x00b8,
        0x00b8, 0x00db, 0x00db, 0x0104, 0x0104, 0x0135, 0x0135, 0x016d,
        0x016d, 0x01af, 0x01af, 0x01fb, 0x01fb, 0x0252, 0x0252, 0x02b6,
        0x02b6, 0x0326, 0x0326, 0x03a5, 0x03a5, 0x0432, 0x0432, 0x04d0,
        0x04d0, 0x057e, 0x057e, 0x062c
    }
};

const double CodechalEncHevcStateG9::m_qpLambdaMdLut[3][QP_NUM] =      // default lambda = pow(2, (qp-12)/6)
{
    //BREDSLICE
    {
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,                //QP=[0 ~12]
        1.0, 1.0, 1.0, 2.0, 2.0, 2.0, 2.0, 3.0, 3.0, 3.0, 4.0, 4.0, 4.0,                //QP=[13~25]
        5.0, 6.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 13.0, 14.0, 16.0, 18.0, 20.0,         //QP=[26~38]
        23.0, 25.0, 29.0, 32.0, 36.0, 40.0, 45.0, 51.0, 57.0, 64.0, 72.0, 81.0, 91.0    //QP=[39~51]
    },
    //PREDSLICE
    {
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,                //QP=[0 ~12]
        1.0, 1.0, 1.0, 2.0, 2.0, 2.0, 2.0, 3.0, 3.0, 3.0, 4.0, 4.0, 4.0,                //QP=[13~25]
        5.0, 6.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 13.0, 14.0, 16.0, 18.0, 20.0,         //QP=[26~38]
        23.0, 25.0, 29.0, 32.0, 36.0, 40.0, 45.0, 51.0, 57.0, 64.0, 72.0, 81.0, 91.0    //QP=[39~51]
    },
    //INTRASLICE
    {
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,                //QP=[0 ~12]
        1.0, 1.0, 1.0, 2.0, 2.0, 2.0, 2.0, 3.0, 3.0, 3.0, 4.0, 4.0, 4.0,                //QP=[13~25]
        5.0, 6.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 13.0, 14.0, 16.0, 18.0, 20.0,         //QP=[26~38]
        23.0, 25.0, 29.0, 32.0, 36.0, 40.0, 45.0, 51.0, 57.0, 64.0, 72.0, 81.0, 91.0    //QP=[39~51]
    }
};

const double CodechalEncHevcStateG9::m_qpLambdaMeLut[3][QP_NUM] =      // default lambda = pow(2, (qp-12)/6)
{
    //BREDSLICE
    {
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,                //QP=[0 ~12]
        1.0, 1.0, 1.0, 2.0, 2.0, 2.0, 2.0, 3.0, 3.0, 3.0, 4.0, 4.0, 4.0,                //QP=[13~25]
        5.0, 6.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 13.0, 14.0, 16.0, 18.0, 20.0,         //QP=[26~38]
        23.0, 25.0, 29.0, 32.0, 36.0, 40.0, 45.0, 51.0, 57.0, 64.0, 72.0, 81.0, 91.0    //QP=[39~51]
    },
    //PREDSLICE
    {
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,                //QP=[0 ~12]
        1.0, 1.0, 1.0, 2.0, 2.0, 2.0, 2.0, 3.0, 3.0, 3.0, 4.0, 4.0, 4.0,                //QP=[13~25]
        5.0, 6.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 13.0, 14.0, 16.0, 18.0, 20.0,         //QP=[26~38]
        23.0, 25.0, 29.0, 32.0, 36.0, 40.0, 45.0, 51.0, 57.0, 64.0, 72.0, 81.0, 91.0    //QP=[39~51]
    },
    //INTRASLICE
    {
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,                //QP=[0 ~12]
        1.0, 1.0, 1.0, 2.0, 2.0, 2.0, 2.0, 3.0, 3.0, 3.0, 4.0, 4.0, 4.0,                //QP=[13~25]
        5.0, 6.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 13.0, 14.0, 16.0, 18.0, 20.0,         //QP=[26~38]
        23.0, 25.0, 29.0, 32.0, 36.0, 40.0, 45.0, 51.0, 57.0, 64.0, 72.0, 81.0, 91.0    //QP=[39~51]
    }
};

const uint32_t CodechalEncHevcStateG9::m_encBTu1BCurbeInit[56] =
{
    0x000000a3, 0x00200008, 0x00143939, 0x00a27700, 0x1000000f, 0x20200000, 0x01000140, 0x00400003,
    0x4f3b1c1b, 0x001d1a29, 0x000c0619, 0x19060600, 0x2c2b291f, 0x00161616, 0x13130013, 0x13131313,
    0x0101f00f, 0x0f0f1010, 0xf0f0f00f, 0x01010101, 0x10101010, 0x0f0f0f0f, 0xf0f0f00f, 0x0101f0f0,
    0x01010101, 0x10101010, 0x0f0f1010, 0x0f0f0f0f, 0xf0f0f00f, 0xf0f0f0f0, 0x00000000, 0x00000000,
    0x010101ce, 0x00040c24, 0x40400000, 0x005800d8, 0x40000001, 0x00001616, 0x00000000, 0x00000016,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x07fc0125, 0x08080201, 0x05030502, 0x00031101,
    0x00020001, 0x00000000, 0x00000001, 0x00000000, 0x00100014, 0x00000000, 0x00000000, 0x00000000
};

const uint32_t CodechalEncHevcStateG9::m_encBTu4BCurbeInit[56] =
{
    0x000000a3, 0x00200008, 0x00143939, 0x00a27700, 0x1000000f, 0x20200000, 0x01000140, 0x00400003,
    0x4f3b1c1b, 0x001d1a29, 0x000c0619, 0x19060600, 0x2c2b291f, 0x00161616, 0x13130013, 0x13131313,
    0x0101f00f, 0x0f0f1010, 0xf0f0f00f, 0x01010101, 0x10101010, 0x0f0f0f0f, 0xf0f0f00f, 0x0101f0f0,
    0x01010101, 0x10101010, 0x0f0f1010, 0x0f0f0f0f, 0xf0f0f00f, 0xf0f0f0f0, 0x00000000, 0x00000000,
    0x010101ce, 0x00040c24, 0x40400000, 0x005800d8, 0x40000001, 0x00001616, 0x00000000, 0x00000016,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x07fc0125, 0x08080201, 0x05030502, 0x0c033104,
    0x00020001, 0x00000000, 0x00000001, 0x00000000, 0x0010000d, 0x00000000, 0x00000000, 0x00000000
};

const uint32_t CodechalEncHevcStateG9::m_encBTu7BCurbeInit[56] =
{
    0x000000a3, 0x00200008, 0x00143919, 0x00a27700, 0x1000000f, 0x20200000, 0x01000140, 0x00400003,
    0x5f4b2c2b, 0x002d2a39, 0x001c0c29, 0x290c0c00, 0x3c3b392f, 0x001b1b1b, 0x1e1e001e, 0x1e1e1e1e,
    0x120ff10f, 0x1e22e20d, 0x20e2ff10, 0x2edd06fc, 0x11d33ff1, 0xeb1ff33d, 0x4ef1f1f1, 0xf1f21211,
    0x0dffffe0, 0x11201f1f, 0x1105f1cf, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x0101030c, 0x00040c24, 0x40c00000, 0x006801b0, 0x40000000, 0x0000001b, 0x00000000, 0x0000001b,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x07fc0115, 0x08080201, 0x05030502, 0x0c034104,
    0x00000001, 0x00000000, 0x00000001, 0x00000000, 0x0010000d, 0x00000000, 0x00000000, 0x00000000
};

const uint32_t CodechalEncHevcStateG9::m_encBTu1PCurbeInit[56] =
{
    0x000000a3, 0x00200008, 0x000b3919, 0x00a63000, 0x30000008, 0x28300000, 0x009000b0, 0x00400063,
    0x5d4b2c2b, 0x001e0f2e, 0x001c0028, 0x2e1f1c00, 0x3b3b392f, 0x001b1b1b, 0x1e1e001e, 0x1e1e1e1e,
    0x0101f00f, 0x0f0f1010, 0xf0f0f00f, 0x01010101, 0x10101010, 0x0f0f0f0f, 0xf0f0f00f, 0x0101f0f0,
    0x01010101, 0x10101010, 0x0f0f1010, 0x0f0f0f0f, 0xf0f0f00f, 0xf0f0f0f0, 0x00000000, 0x00000000,
    0x80010165, 0x00040c24, 0x40c00000, 0x04680174, 0x41000002, 0x001b1b1b, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x07fc0035, 0x08080201, 0x05030502, 0x00032000,
    0x00020001, 0x00000003, 0x00000000, 0x00000000, 0x000a000e, 0x00002830, 0x00000000, 0x00000000

};

const uint32_t CodechalEncHevcStateG9::m_encBTu4PCurbeInit[56] =
{
    0x000000a3, 0x00200008, 0x000b3919, 0x00a63000, 0x30000008, 0x28300000, 0x009000b0, 0x00400063,
    0x5d4b2c2b, 0x001e0f2e, 0x001c0028, 0x2e1f1c00, 0x3b3b392f, 0x001b1b1b, 0x1e1e001e, 0x1e1e1e1e,
    0x0101f00f, 0x0f0f1010, 0xf0f0f00f, 0x01010101, 0x10101010, 0x0f0f0f0f, 0xf0f0f00f, 0x0101f0f0,
    0x01010101, 0x10101010, 0x0f0f1010, 0x0f0f0f0f, 0xf0f0f00f, 0xf0f0f0f0, 0x00000000, 0x00000000,
    0x80010165, 0x00040c24, 0x40c00000, 0x04680174, 0x41000002, 0x001b1b1b, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x07fc0035, 0x08080201, 0x05030502, 0x00032000,
    0x00020001, 0x00000003, 0x00000000, 0x00000000, 0x000a000e, 0x00002830, 0x00000000, 0x00000000

};

const uint32_t CodechalEncHevcStateG9::m_encBTu7PCurbeInit[56] =
{
    0x000000a3, 0x00200008, 0x000b3919, 0x00a63000, 0x30000008, 0x28300000, 0x009000b0, 0x00400063,
    0x5d4b2c2b, 0x001e0f2e, 0x001c0028, 0x2e1f1c00, 0x3b3b392f, 0x001b1b1b, 0x1e1e001e, 0x1e1e1e1e,
    0x120ff10f, 0x1e22e20d, 0x20e2ff10, 0x2edd06fc, 0x11d33ff1, 0xeb1ff33d, 0x4ef1f1f1, 0xf1f21211,
    0x0dffffe0, 0x11201f1f, 0x1105f1cf, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x80010165, 0x00040c24, 0x40c00000, 0x04680174, 0x41000002, 0x001b1b1b, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x07fc0035, 0x08080201, 0x05030502, 0x00032000,
    0x00020001, 0x00000003, 0x00000000, 0x00000000, 0x000a000e, 0x00002830, 0x00000000, 0x00000000
};

const uint32_t CodechalEncHevcStateG9::m_encBTu7ICurbeInit[56] =
{
    0x000000a2, 0x00200008, 0x00143919, 0x00a03000, 0x5000000f, 0x28300000, 0x01000140, 0x00000003,
    0x5a3d0029, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x001b1b1b, 0x1e1e001e, 0x1e1e1e1e,
    0x120ff10f, 0x1e22e20d, 0x20e2ff10, 0x2edd06fc, 0x11d33ff1, 0xeb1ff33d, 0x4ef1f1f1, 0xf1f21211,
    0x0dffffe0, 0x11201f1f, 0x1105f1cf, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x8080030c, 0x00040c24, 0x40a6e43f, 0x005f0139, 0x40000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x07fc0005, 0x08080201, 0x05030502, 0x0c034104,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x0010000d, 0x00000000, 0x00000000, 0x00000000
};

const uint32_t CodechalEncHevcStateG9::m_brcInitCurbeInit[32] =
{
    0x000a8c00, 0x0112a880, 0x016e3600, 0x00b71b00, 0x00b71b00, 0x00000000, 0x0000001e, 0x00000001,
    0x000a0040, 0x05000000, 0x001e02d0, 0x000100c8, 0x00010033, 0x00000000, 0x00010000, 0x00000000,
    0x78503c28, 0x78503c23, 0x735a3c28, 0xe5dfd8d1, 0x2f29211b, 0xe5ddd7d1, 0x5e56463f, 0xeae3dad4,
    0x2f281f16, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
};

const uint32_t CodechalEncHevcStateG9::m_brcUpdateCurbeInit[16] =
{
    0x0112a880, 0x00000000, 0x00000230, 0x0042000d, 0x00c80085, 0x02044000, 0x00000000, 0x00000000,
    0x02030101, 0x05052801, 0x12070103, 0x4b282519, 0xa07d6761, 0x00fffefd, 0x00030201, 0x00000000
};

const uint32_t CodechalEncHevcStateG9::m_meCurbeInit[39] =
{
    0x00000000, 0x00200008, 0x00003939, 0x77a43000, 0x00000000, 0x28300000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
};

uint8_t CodechalEncHevcStateG9::PicCodingTypeToSliceType(uint16_t pictureCodingType)
{
    uint8_t sliceType = 0;

    switch (pictureCodingType)
    {
    case I_TYPE:
        sliceType = CODECHAL_ENCODE_HEVC_I_SLICE;
        break;
    case P_TYPE:
        sliceType = CODECHAL_ENCODE_HEVC_P_SLICE;
        break;
    case B_TYPE:
    case B1_TYPE:
    case B2_TYPE:
        sliceType = CODECHAL_ENCODE_HEVC_B_SLICE;
        break;
    default:
        CODECHAL_ENCODE_ASSERT(false);
    }
    return sliceType;
}

MOS_STATUS CodechalEncHevcStateG9::GenerateSliceMap()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (m_numSlices > 1 && m_sliceMap)
    {
        uint32_t log2LcuSize = m_hevcSeqParams->log2_max_coding_block_size_minus3 + 3;
        CODECHAL_ENCODE_ASSERT(log2LcuSize == 5);

        uint32_t W = MOS_ALIGN_CEIL(m_frameWidth, (1 << log2LcuSize)) >> log2LcuSize;
        uint32_t H = MOS_ALIGN_CEIL(m_frameHeight, (1 << log2LcuSize)) >> log2LcuSize;
        if (m_sliceMapSurface.dwPitch < W * sizeof(m_sliceMap[0]))
        {
            eStatus = MOS_STATUS_MORE_DATA;
            return eStatus;
        }

        MOS_LOCK_PARAMS lockFlags;
        MOS_ZeroMemory(&lockFlags, sizeof(lockFlags));
        lockFlags.WriteOnly = true;

        uint8_t*  surface = (uint8_t* )m_osInterface->pfnLockResource(
            m_osInterface,
            &m_sliceMapSurface.OsResource,
            &lockFlags);

        if (surface == nullptr)
        {
            eStatus = MOS_STATUS_NULL_POINTER;
            return eStatus;
        }

        for (uint32_t h = 0; h < H; h++, surface += m_sliceMapSurface.dwPitch)
        {
            PCODECHAL_ENCODE_HEVC_SLICE_MAP map = (PCODECHAL_ENCODE_HEVC_SLICE_MAP)surface;
            for (uint32_t w = 0; w < W; w++)
            {
                map[w] = m_sliceMap[h * W + w];
            }
        }

        m_osInterface->pfnUnlockResource(
            m_osInterface,
            &m_sliceMapSurface.OsResource);
    }
    else if (m_numSlices == 1 && m_lastNumSlices != m_numSlices)
    {
        // Reset slice map surface
        MOS_LOCK_PARAMS lockFlags;
        MOS_ZeroMemory(&lockFlags, sizeof(lockFlags));
        lockFlags.WriteOnly = true;

        uint8_t*  surface = (uint8_t* )m_osInterface->pfnLockResource(
            m_osInterface,
            &m_sliceMapSurface.OsResource,
            &lockFlags);

        if (surface == nullptr)
        {
            eStatus = MOS_STATUS_NULL_POINTER;
            return eStatus;
        }

        MOS_ZeroMemory(surface,
            m_sliceMapSurface.dwWidth * m_sliceMapSurface.dwHeight);

        m_osInterface->pfnUnlockResource(
            m_osInterface,
            &m_sliceMapSurface.OsResource);
    }

    m_lastNumSlices = m_numSlices;

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG9::SetSliceStructs()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncHevcState::SetSliceStructs());

    // setup slice map
    PCODEC_HEVC_ENCODE_SLICE_PARAMS slcParams = m_hevcSliceParams;
    for (uint32_t startLCU = 0, slcCount = 0; slcCount < m_numSlices; slcCount++, slcParams++)
    {
        if (!m_hevcPicParams->tiles_enabled_flag)
        {
            CODECHAL_ENCODE_ASSERT(slcParams->slice_segment_address == startLCU);

            // process slice map
            for (uint32_t i = 0; i < slcParams->NumLCUsInSlice; i++)
            {
                m_sliceMap[startLCU + i].ucSliceID = (uint8_t)slcCount;
            }

            startLCU += slcParams->NumLCUsInSlice;
        }
    }

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG9::SetSequenceStructs()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncHevcState::SetSequenceStructs());

    // TU1 has no wave-front split
    if (m_hevcSeqParams->TargetUsage == 1 && m_numRegionsInSlice != 1)
    {
        m_numRegionsInSlice = 1;
    }

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG9::SetPictureStructs()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncHevcState::SetPictureStructs());

    /* dwOriFrameWidth and dwOriFrameHeight must be CU-aligned in HEVC. Set the recon and raw surface resolution as
    the actual encoding resolution.
    */
    m_rawSurface.dwWidth = m_reconSurface.dwWidth = m_oriFrameWidth;
    m_rawSurface.dwHeight = m_reconSurface.dwHeight = m_oriFrameHeight;

    m_firstIntraRefresh = true;
    m_frameNumInGob = (m_pictureCodingType == I_TYPE) ? 0 : (m_frameNumInGob + 1);

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG9::CalcScaledDimensions()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    m_downscaledWidthInMb4x               =
        CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_frameWidth / SCALE_FACTOR_4x);

    if (MEDIA_IS_SKU(m_skuTable, FtrEncodeHEVC10bit) && m_hevcSeqParams->bit_depth_luma_minus8)
    {
        uint32_t downscaledSurfaceWidth4x = MOS_ALIGN_CEIL((m_downscaledWidthInMb4x* CODECHAL_MACROBLOCK_WIDTH), (CODECHAL_MACROBLOCK_WIDTH * 2));
        m_downscaledWidthInMb4x = CODECHAL_GET_WIDTH_IN_MACROBLOCKS(downscaledSurfaceWidth4x);
    }

    m_downscaledHeightInMb4x              =
        CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameHeight / SCALE_FACTOR_4x);
    m_downscaledWidth4x =
        m_downscaledWidthInMb4x * CODECHAL_MACROBLOCK_WIDTH;
    m_downscaledHeight4x =
        m_downscaledHeightInMb4x * CODECHAL_MACROBLOCK_HEIGHT;

    // SuperHME Scaling WxH
    m_downscaledWidthInMb16x              =
        CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_frameWidth / SCALE_FACTOR_16x);
    m_downscaledHeightInMb16x             =
        CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameHeight / SCALE_FACTOR_16x);
    m_downscaledWidth16x =
        m_downscaledWidthInMb16x * CODECHAL_MACROBLOCK_WIDTH;
    m_downscaledHeight16x =
        m_downscaledHeightInMb16x * CODECHAL_MACROBLOCK_HEIGHT;

    // UltraHME Scaling WxH
    m_downscaledWidthInMb32x              =
        CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_frameWidth / SCALE_FACTOR_32x);
    m_downscaledHeightInMb32x             =
        CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameHeight / SCALE_FACTOR_32x);
    m_downscaledWidth32x =
        m_downscaledWidthInMb32x * CODECHAL_MACROBLOCK_WIDTH;
    m_downscaledHeight32x =
        m_downscaledHeightInMb32x * CODECHAL_MACROBLOCK_HEIGHT;

    return MOS_STATUS_SUCCESS;
}

void CodechalEncHevcStateG9::LoadCosts(
    uint8_t sliceType,
    uint8_t qp,
    uint8_t intraSADTransform)
{
    float hadBias = 2.0f;

    if (intraSADTransform == INTRA_TRANSFORM_HADAMARD)
    {
        hadBias = 1.67f;
    }

    double lambdaMd = m_qpLambdaMd[sliceType][qp];
    double lambdaMe = m_qpLambdaMe[sliceType][qp];

    m_modeCost[0] = Map44LutValue((uint32_t)(lambdaMd * m_modeCostLut[sliceType][0] * hadBias), 0x6f);
    m_modeCost[1] = Map44LutValue((uint32_t)(lambdaMd * m_modeCostLut[sliceType][1] * hadBias), 0x8f);
    m_modeCost[2] = Map44LutValue((uint32_t)(lambdaMd * m_modeCostLut[sliceType][2] * hadBias), 0x8f);
    m_modeCost[3] = Map44LutValue((uint32_t)(lambdaMd * m_modeCostLut[sliceType][3] * hadBias), 0x8f);
    m_modeCost[4] = Map44LutValue((uint32_t)(lambdaMd * m_modeCostLut[sliceType][4] * hadBias), 0x8f);
    m_modeCost[5] = Map44LutValue((uint32_t)(lambdaMd * m_modeCostLut[sliceType][5] * hadBias), 0x6f);
    m_modeCost[6] = Map44LutValue((uint32_t)(lambdaMd * m_modeCostLut[sliceType][6] * hadBias), 0x6f);
    m_modeCost[7] = Map44LutValue((uint32_t)(lambdaMd * m_modeCostLut[sliceType][7] * hadBias), 0x6f);
    m_modeCost[8] = Map44LutValue((uint32_t)(lambdaMd * m_modeCostLut[sliceType][8] * hadBias), 0x8f);
    m_modeCost[9] = Map44LutValue((uint32_t)(lambdaMd * m_modeCostLut[sliceType][9] * hadBias), 0x6f);
    m_modeCost[10] = Map44LutValue((uint32_t)(lambdaMd * m_modeCostLut[sliceType][10] * hadBias), 0x6f);
    m_modeCost[11] = Map44LutValue((uint32_t)(lambdaMd * m_modeCostLut[sliceType][11] * hadBias), 0x6f);

    m_mvCost[0] = Map44LutValue((uint32_t)(lambdaMe * m_mvCostLut[sliceType][0] * hadBias), 0x6f);
    m_mvCost[1] = Map44LutValue((uint32_t)(lambdaMe * m_mvCostLut[sliceType][1] * hadBias), 0x6f);
    m_mvCost[2] = Map44LutValue((uint32_t)(lambdaMe * m_mvCostLut[sliceType][2] * hadBias), 0x6f);
    m_mvCost[3] = Map44LutValue((uint32_t)(lambdaMe * m_mvCostLut[sliceType][3] * hadBias), 0x6f);
    m_mvCost[4] = Map44LutValue((uint32_t)(lambdaMe * m_mvCostLut[sliceType][4] * hadBias), 0x6f);
    m_mvCost[5] = Map44LutValue((uint32_t)(lambdaMe * m_mvCostLut[sliceType][5] * hadBias), 0x6f);
    m_mvCost[6] = Map44LutValue((uint32_t)(lambdaMe * m_mvCostLut[sliceType][6] * hadBias), 0x6f);
    m_mvCost[7] = Map44LutValue((uint32_t)(lambdaMe * m_mvCostLut[sliceType][7] * hadBias), 0x6f);

    double m_lambdaMd = lambdaMd * hadBias;
    m_simplestIntraInterThreshold = 0;
    if (m_modeCostLut[sliceType][1] < m_modeCostLut[sliceType][3])
    {
        m_simplestIntraInterThreshold = (uint32_t)(m_lambdaMd * (m_modeCostLut[sliceType][3] - m_modeCostLut[sliceType][1]) + 0.5);
    }

    m_modeCostSp = Map44LutValue((uint32_t)(lambdaMd * 45 * hadBias), 0x8f);
}

void CodechalEncHevcStateG9::CalcForwardCoeffThd(uint8_t* forwardCoeffThresh, int32_t qp)
{
    static const uint8_t FTQ25I[27] =
    {
        0, 0, 0, 0,
        1, 3, 6, 8, 11,
        13, 16, 19, 22, 26,
        30, 34, 39, 44, 50,
        56, 62, 69, 77, 85,
        94, 104, 115
    };

    uint8_t idx = (qp + 1) >> 1;

    forwardCoeffThresh[0] =
    forwardCoeffThresh[1] =
    forwardCoeffThresh[2] =
    forwardCoeffThresh[3] =
    forwardCoeffThresh[4] =
    forwardCoeffThresh[5] =
    forwardCoeffThresh[6] = FTQ25I[idx];
}

uint8_t CodechalEncHevcStateG9::GetQPValueFromRefList(uint32_t list, uint32_t index)
{
    CODECHAL_ENCODE_ASSERT(list == LIST_0 || list == LIST_1);
    CODECHAL_ENCODE_ASSERT(index < CODEC_MAX_NUM_REF_FRAME_HEVC);

    CODEC_PICTURE picture = m_hevcSliceParams->RefPicList[list][index];

    if (!CodecHal_PictureIsInvalid(picture) && m_picIdx[picture.FrameIdx].bValid)
    {
        auto picIdx = m_picIdx[picture.FrameIdx].ucPicIdx;
        return m_refList[picIdx]->ucQPValue[0];
    }
    else
    {
        return 0;
    }
}

void CodechalEncHevcStateG9::GetMaxRefFrames(uint8_t& maxNumRef0, uint8_t& maxNumRef1)
{
    maxNumRef0 = CODECHAL_ENCODE_HEVC_NUM_MAX_VME_L0_REF_G9;
    maxNumRef1 = CODECHAL_ENCODE_HEVC_NUM_MAX_VME_L1_REF_G9;

    return;
}

void CodechalEncHevcStateG9::InitParamForWalkerVfe26z(
    uint32_t numRegionsInSlice,
    uint32_t maxSliceHeight)
{
    int32_t width = CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_frameWidth);
    int32_t height = maxSliceHeight * 2;
    int32_t tsWidth = ((width + 3) & 0xfffc) >> 1;
    int32_t lcuWidth = (width + 1) >> 1;
    int32_t lcuHeight = (height + 1) >> 1;
    int32_t tmp1 = ((lcuWidth + 1) >> 1) + ((lcuWidth + ((lcuHeight - 1) << 1)) + (2 * numRegionsInSlice - 1)) / (2 * numRegionsInSlice);

    m_walkingPatternParam.MediaWalker.UseScoreboard = m_useHwScoreboard;
    m_walkingPatternParam.MediaWalker.ScoreboardMask = 0xFF;
    m_walkingPatternParam.MediaWalker.GlobalResolution.x = tsWidth;
    m_walkingPatternParam.MediaWalker.GlobalResolution.y = 4 * tmp1;

    m_walkingPatternParam.MediaWalker.GlobalStart.x = 0;
    m_walkingPatternParam.MediaWalker.GlobalStart.y = 0;

    m_walkingPatternParam.MediaWalker.GlobalOutlerLoopStride.x = tsWidth;
    m_walkingPatternParam.MediaWalker.GlobalOutlerLoopStride.y = 0;

    m_walkingPatternParam.MediaWalker.GlobalInnerLoopUnit.x = 0;
    m_walkingPatternParam.MediaWalker.GlobalInnerLoopUnit.y = 4 * tmp1;

    m_walkingPatternParam.MediaWalker.BlockResolution.x = tsWidth;
    m_walkingPatternParam.MediaWalker.BlockResolution.y = 4 * tmp1;

    m_walkingPatternParam.MediaWalker.LocalStart.x = tsWidth;
    m_walkingPatternParam.MediaWalker.LocalStart.y = 0;

    m_walkingPatternParam.MediaWalker.LocalEnd.x = 0;
    m_walkingPatternParam.MediaWalker.LocalEnd.y = 0;

    m_walkingPatternParam.MediaWalker.LocalOutLoopStride.x = 1;
    m_walkingPatternParam.MediaWalker.LocalOutLoopStride.y = 0;

    m_walkingPatternParam.MediaWalker.LocalInnerLoopUnit.x = MOS_BITFIELD_VALUE((uint32_t)-2, 16);
    m_walkingPatternParam.MediaWalker.LocalInnerLoopUnit.y = 4;

    m_walkingPatternParam.MediaWalker.MiddleLoopExtraSteps = 3;

    m_walkingPatternParam.MediaWalker.MidLoopUnitX = 0;
    m_walkingPatternParam.MediaWalker.MidLoopUnitY = 1;

    m_walkingPatternParam.MediaWalker.dwGlobalLoopExecCount = 0;
    m_walkingPatternParam.MediaWalker.dwLocalLoopExecCount = 2 * ((lcuWidth + (lcuHeight - 1) * 2 + 2 * numRegionsInSlice - 1) / (2 * numRegionsInSlice)) - 1;

    m_walkingPatternParam.ScoreBoard.ScoreboardEnable = m_useHwScoreboard;
    m_walkingPatternParam.ScoreBoard.ScoreboardType = m_hwScoreboardType;
    m_walkingPatternParam.ScoreBoard.ScoreboardMask = 0xff;

    m_walkingPatternParam.ScoreBoard.ScoreboardDelta[0].x = MOS_BITFIELD_VALUE((uint32_t)-1, 4);
    m_walkingPatternParam.ScoreBoard.ScoreboardDelta[0].y = 3;

    m_walkingPatternParam.ScoreBoard.ScoreboardDelta[1].x = MOS_BITFIELD_VALUE((uint32_t)-1, 4);
    m_walkingPatternParam.ScoreBoard.ScoreboardDelta[1].y = 1;

    m_walkingPatternParam.ScoreBoard.ScoreboardDelta[2].x = MOS_BITFIELD_VALUE((uint32_t)-1, 4);
    m_walkingPatternParam.ScoreBoard.ScoreboardDelta[2].y = MOS_BITFIELD_VALUE((uint32_t)-1, 4);

    m_walkingPatternParam.ScoreBoard.ScoreboardDelta[3].x = 0;
    m_walkingPatternParam.ScoreBoard.ScoreboardDelta[3].y = MOS_BITFIELD_VALUE((uint32_t)-1, 4);

    m_walkingPatternParam.ScoreBoard.ScoreboardDelta[4].x = 0;
    m_walkingPatternParam.ScoreBoard.ScoreboardDelta[4].y = MOS_BITFIELD_VALUE((uint32_t)-2, 4);

    m_walkingPatternParam.ScoreBoard.ScoreboardDelta[5].x = 0;
    m_walkingPatternParam.ScoreBoard.ScoreboardDelta[5].y = MOS_BITFIELD_VALUE((uint32_t)-3, 4);

    m_walkingPatternParam.ScoreBoard.ScoreboardDelta[6].x = 1;
    m_walkingPatternParam.ScoreBoard.ScoreboardDelta[6].y = MOS_BITFIELD_VALUE((uint32_t)-2, 4);

    m_walkingPatternParam.ScoreBoard.ScoreboardDelta[7].x = 1;
    m_walkingPatternParam.ScoreBoard.ScoreboardDelta[7].y = MOS_BITFIELD_VALUE((uint32_t)-3, 4);

    m_walkingPatternParam.Offset_Y = -4 * ((lcuWidth + 1) >> 1);
    m_walkingPatternParam.Offset_Delta = ((lcuWidth + ((lcuHeight - 1) << 1)) + (numRegionsInSlice - 1)) / (numRegionsInSlice);
}

void CodechalEncHevcStateG9::InitParamForWalkerVfe26(
    uint32_t numRegionsInSlice,
    uint32_t maxSliceHeight)
{
    int32_t width = CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_frameWidth);
    int32_t height = maxSliceHeight;
    int32_t tsWidth = (width + 1) & 0xfffe;
    int32_t tsHeight = (height + 1) & 0xfffe;
    int32_t tmp1 = ((tsWidth + 1) >> 1) + ((tsWidth + ((tsHeight - 1) << 1)) + (2 * numRegionsInSlice - 1)) / (2 * numRegionsInSlice);

    m_walkingPatternParam.MediaWalker.UseScoreboard = m_useHwScoreboard;
    m_walkingPatternParam.MediaWalker.ScoreboardMask = 0x0F;
    m_walkingPatternParam.MediaWalker.GlobalResolution.x = tsWidth;
    m_walkingPatternParam.MediaWalker.GlobalResolution.y = tmp1;    // tsHeight;

    m_walkingPatternParam.MediaWalker.GlobalStart.x = 0;
    m_walkingPatternParam.MediaWalker.GlobalStart.y = 0;

    m_walkingPatternParam.MediaWalker.GlobalOutlerLoopStride.x = tsWidth;
    m_walkingPatternParam.MediaWalker.GlobalOutlerLoopStride.y = 0;

    m_walkingPatternParam.MediaWalker.GlobalInnerLoopUnit.x = 0;
    m_walkingPatternParam.MediaWalker.GlobalInnerLoopUnit.y = tmp1;

    m_walkingPatternParam.MediaWalker.BlockResolution.x = tsWidth;
    m_walkingPatternParam.MediaWalker.BlockResolution.y = tmp1;

    m_walkingPatternParam.MediaWalker.LocalStart.x = tsWidth;
    m_walkingPatternParam.MediaWalker.LocalStart.y = 0;

    m_walkingPatternParam.MediaWalker.LocalEnd.x = 0;
    m_walkingPatternParam.MediaWalker.LocalEnd.y = 0;

    m_walkingPatternParam.MediaWalker.LocalOutLoopStride.x = 1;
    m_walkingPatternParam.MediaWalker.LocalOutLoopStride.y = 0;

    m_walkingPatternParam.MediaWalker.LocalInnerLoopUnit.x = MOS_BITFIELD_VALUE((uint32_t)-2, 16);
    m_walkingPatternParam.MediaWalker.LocalInnerLoopUnit.y = 1;

    m_walkingPatternParam.MediaWalker.MiddleLoopExtraSteps = 0;

    m_walkingPatternParam.MediaWalker.MidLoopUnitX = 0;
    m_walkingPatternParam.MediaWalker.MidLoopUnitY = 0;

    m_walkingPatternParam.MediaWalker.dwGlobalLoopExecCount = 0;

    m_walkingPatternParam.MediaWalker.dwLocalLoopExecCount = (width + (height - 1) * 2 + numRegionsInSlice - 1) / numRegionsInSlice;

    m_walkingPatternParam.ScoreBoard.ScoreboardEnable = m_useHwScoreboard;
    m_walkingPatternParam.ScoreBoard.ScoreboardType = m_hwScoreboardType;
    m_walkingPatternParam.ScoreBoard.ScoreboardMask = 0x0f;

    m_walkingPatternParam.ScoreBoard.ScoreboardDelta[0].x = MOS_BITFIELD_VALUE((uint32_t)-1, 4);
    m_walkingPatternParam.ScoreBoard.ScoreboardDelta[0].y = 0;

    m_walkingPatternParam.ScoreBoard.ScoreboardDelta[1].x = MOS_BITFIELD_VALUE((uint32_t)-1, 4);
    m_walkingPatternParam.ScoreBoard.ScoreboardDelta[1].y = MOS_BITFIELD_VALUE((uint32_t)-1, 4);

    m_walkingPatternParam.ScoreBoard.ScoreboardDelta[2].x = 0;
    m_walkingPatternParam.ScoreBoard.ScoreboardDelta[2].y = MOS_BITFIELD_VALUE((uint32_t)-1, 4);

    m_walkingPatternParam.ScoreBoard.ScoreboardDelta[3].x = 1;
    m_walkingPatternParam.ScoreBoard.ScoreboardDelta[3].y = MOS_BITFIELD_VALUE((uint32_t)-1, 4);

    m_walkingPatternParam.Offset_Y = -((width + 1) >> 1);
    m_walkingPatternParam.Offset_Delta = ((width + ((height - 1) << 1)) + (numRegionsInSlice - 1)) / (numRegionsInSlice);
}

MOS_STATUS CodechalEncHevcStateG9::GenerateWalkingControlRegion()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_ZeroMemory(&m_walkingPatternParam, sizeof(m_walkingPatternParam));

    if (m_numRegionsInSlice < 1)
    {
        // Region number cannot be smaller than 1
        m_numRegionsInSlice = 1;
    }

    if (m_numRegionsInSlice > 16)
    {
        // Region number cannot be larger than 16
        m_numRegionsInSlice = 16;
    }

    uint32_t frameWidthInUnits = 0, frameHeightInUnits = 0;
    if (m_enable26WalkingPattern) /* 26 degree walking pattern */
    {
        frameWidthInUnits = CODECHAL_ENCODE_HEVC_GET_SIZE_IN_LCU(m_frameWidth, 16);
        frameHeightInUnits = CODECHAL_ENCODE_HEVC_GET_SIZE_IN_LCU(m_frameHeight, 16);
    }
    else /* 26z walking pattern */
    {
        frameWidthInUnits = CODECHAL_ENCODE_HEVC_GET_SIZE_IN_LCU(m_frameWidth, 32);
        frameHeightInUnits = CODECHAL_ENCODE_HEVC_GET_SIZE_IN_LCU(m_frameHeight, 32);
    }

    // THE FOLLOWING CODE FOR SLICE MERGING / CONCURRENT THREAD GENERATION IS PORTED FROM THE
    // SKL HEVC KRN CMODEL (v8992). FOR FIXES VERIFY THAT PROBLEM DOESN'T EXIST THERE TOO.
    bool isArbitrarySlices = false;
    int32_t sliceStartY[CODECHAL_HEVC_MAX_NUM_SLICES_LVL_5 + 1] = { 0 };
    for (uint32_t slice = 0; slice < m_numSlices; slice++)
    {
        if (m_hevcSliceParams[slice].slice_segment_address % CODECHAL_ENCODE_HEVC_GET_SIZE_IN_LCU(m_frameWidth, 32))
        {
            isArbitrarySlices = true;
        }
        else
        {
            sliceStartY[slice] = m_hevcSliceParams[slice].slice_segment_address / CODECHAL_ENCODE_HEVC_GET_SIZE_IN_LCU(m_frameWidth, 32);

            /* 26 degree walking pattern */
            if (m_enable26WalkingPattern)
            {
                sliceStartY[slice] *= 2;
            }
        }
    }

    sliceStartY[m_numSlices] = frameHeightInUnits;

    const uint32_t regionStartYOffset = 32;
    uint32_t numRegions = 1;
    uint32_t numSlices = 0, height = 0;
    int32_t maxHeight = 0;
    uint16_t regionsStartTable[64] = { 0 };

    if (isArbitrarySlices)
    {
        height = frameHeightInUnits;
        numSlices = 1;
        maxHeight = height;
        if (m_numRegionsInSlice > 1)
        {
            uint32_t numUnitInRegion =
                (frameWidthInUnits + 2 * (frameHeightInUnits - 1) + m_numRegionsInSlice - 1) / m_numRegionsInSlice;

            numRegions = m_numRegionsInSlice;

            for (uint32_t i = 1; i < m_numRegionsInSlice; i++)
            {
                uint32_t front = i*numUnitInRegion;

                if (front < frameWidthInUnits)
                {
                    regionsStartTable[i] = (uint16_t)front;
                }
                else if (((front - frameWidthInUnits + 1) & 1) == 0)
                {
                    regionsStartTable[i] = (uint16_t)frameWidthInUnits - 1;
                }
                else
                {
                    regionsStartTable[i] = (uint16_t)frameWidthInUnits - 2;
                }

                regionsStartTable[regionStartYOffset + i] = (uint16_t)((front - regionsStartTable[i]) >> 1);
            }
        }
    }
    else
    {
        maxHeight = 0;
        numSlices = m_numSlices;

        for (uint32_t slice = 0; slice < numSlices; slice++)
        {
            int32_t sliceHeight = sliceStartY[slice + 1] - sliceStartY[slice];
            if (sliceHeight > maxHeight)
            {
                maxHeight = sliceHeight;
            }
        }

        bool sliceIsMerged = false;
        while (!sliceIsMerged)
        {
            int32_t newNumSlices = 1;
            int32_t startY = 0;

            for (uint32_t slice = 1; slice < numSlices; slice++)
            {
                if ((sliceStartY[slice + 1] - startY) <= maxHeight)
                {
                    sliceStartY[slice] = -1;
                }
                else
                {
                    startY = sliceStartY[slice];
                }
            }

            for (uint32_t slice = 1; slice < numSlices; slice++)
            {
                if (sliceStartY[slice] > 0)
                {
                    sliceStartY[newNumSlices] = sliceStartY[slice];
                    newNumSlices++;
                }
            }

            numSlices = newNumSlices;
            sliceStartY[numSlices] = frameHeightInUnits;

            /* very rough estimation */
            if (numSlices * m_numRegionsInSlice <= CODECHAL_MEDIA_WALKER_MAX_COLORS)
            {
                sliceIsMerged = true;
            }
            else
            {
                int32_t num = 1;

                maxHeight = frameHeightInUnits;

                for (uint32_t slice = 0; slice < numSlices - 1; slice++)
                {
                    if ((sliceStartY[slice + 2] - sliceStartY[slice]) <= maxHeight)
                    {
                        maxHeight = sliceStartY[slice + 2] - sliceStartY[slice];
                        num = slice + 1;
                    }
                }

                for (uint32_t slice = num; slice < numSlices; slice++)
                {
                    sliceStartY[slice] = sliceStartY[slice + 1];
                }

                numSlices--;
            }
        }

        uint32_t numUnitInRegion =
            (frameWidthInUnits + 2 * (maxHeight - 1) + m_numRegionsInSlice - 1) / m_numRegionsInSlice;

        numRegions = numSlices * m_numRegionsInSlice;

        CODECHAL_ENCODE_ASSERT(numRegions != 0); // Making sure that the number of regions is at least 1

        for (uint32_t slice = 0; slice < numSlices; slice++)
        {
            regionsStartTable[slice * m_numRegionsInSlice]                        = 0;
            regionsStartTable[regionStartYOffset + (slice * m_numRegionsInSlice)] = (uint16_t)sliceStartY[slice];

            for (uint32_t i = 1; i < m_numRegionsInSlice; i++)
            {
                uint32_t front = i*numUnitInRegion;

                if (front < frameWidthInUnits)
                {
                    regionsStartTable[slice * m_numRegionsInSlice + i] = (uint16_t)front;
                }
                else if (((front - frameWidthInUnits + 1) & 1) == 0)
                {
                    regionsStartTable[slice * m_numRegionsInSlice + i] = (uint16_t)frameWidthInUnits - 1;
                }
                else
                {
                    regionsStartTable[slice * m_numRegionsInSlice + i] = (uint16_t)frameWidthInUnits - 2;
                }

                regionsStartTable[regionStartYOffset + (slice * m_numRegionsInSlice + i)] = (uint16_t)sliceStartY[slice] +
                                                                                            ((front - regionsStartTable[i]) >> 1);
            }
        }
        height = maxHeight;
    }

    CODECHAL_ENCODE_ASSERT(numSlices <= CODECHAL_MEDIA_WALKER_MAX_COLORS); // The merged slices should be within the max color limit

    uint16_t datatmp[32][32] = { 0 };
    uint32_t offsetToTheRegionStart[16] = { 0 };
    for (uint32_t k = 0; k < numSlices; k++)
    {
        int32_t nearestReg = 0;
        int32_t minDelta = m_frameHeight;

        /* 26 degree wave front */
        if (m_enable26WalkingPattern)
        {
            int32_t curLcuPelY  = regionsStartTable[regionStartYOffset + (k * m_numRegionsInSlice)] << 4;
            int32_t tsWidth = m_frameWidth >> 4;
            int32_t tsHeight = height;
            int32_t offsetY = -((tsWidth + 1) >> 1);
            int32_t offsetDelta = ((tsWidth + ((tsHeight - 1) << 1)) + (m_numRegionsInSlice - 1)) / (m_numRegionsInSlice);

            for (uint32_t i = 0; i < numRegions; i++)
            {
                if (regionsStartTable[i] == 0)
                {
                    int32_t delta = curLcuPelY - (regionsStartTable[regionStartYOffset + i] << 4);

                    if (delta >= 0)
                    {
                        if (delta < minDelta)
                        {
                            minDelta = delta;
                            nearestReg = i;
                        }
                    }
                }

                offsetToTheRegionStart[k] = 2 * regionsStartTable[regionStartYOffset + nearestReg];
            }
            for (uint32_t i = 0; i < m_numRegionsInSlice; i++)
            {
                datatmp[k * m_numRegionsInSlice + i][0] = regionsStartTable[nearestReg + i];
                datatmp[k * m_numRegionsInSlice + i][1] = regionsStartTable[regionStartYOffset + (nearestReg + i)];
                datatmp[k * m_numRegionsInSlice + i][2] = regionsStartTable[regionStartYOffset + nearestReg];
                int32_t tmpY                            = regionsStartTable[regionStartYOffset + (nearestReg + m_numRegionsInSlice)];
                datatmp[k * m_numRegionsInSlice + i][3] = (uint16_t)((tmpY != 0) ? tmpY : (m_frameHeight) >> 4);
                datatmp[k * m_numRegionsInSlice + i][4] = offsetToTheRegionStart[k] & 0x0FFFF;
                datatmp[k * m_numRegionsInSlice + i][5] = 0;
                datatmp[k * m_numRegionsInSlice + i][6] = 0;
                datatmp[k * m_numRegionsInSlice + i][7] = (uint16_t)(offsetY + regionsStartTable[regionStartYOffset + nearestReg] + ((i * offsetDelta) >> 1));
            }
        }
        else /* 26z walking pattern */
        {
            int32_t curLcuPelY  = regionsStartTable[regionStartYOffset + (k * m_numRegionsInSlice)] << 5;
            int32_t tsWidth = (m_frameWidth + 16) >> 5;
            int32_t tsHeight = height;
            int32_t offsetY = -4 * ((tsWidth + 1) >> 1);
            int32_t offsetDelta = ((tsWidth + ((tsHeight - 1) << 1)) + (m_numRegionsInSlice - 1)) / (m_numRegionsInSlice);

            for (uint32_t i = 0; i < numRegions; i++)
            {
                if (regionsStartTable[i] == 0)
                {
                    int32_t delta = curLcuPelY - (regionsStartTable[regionStartYOffset + i] << 5);

                    if (delta >= 0)
                    {
                        if (delta < minDelta)
                        {
                            minDelta = delta;
                            nearestReg = i;
                        }
                    }
                }

                offsetToTheRegionStart[k] = 2 * regionsStartTable[regionStartYOffset + nearestReg];
            }

            for (uint32_t i = 0; i < m_numRegionsInSlice; i++)
            {
                datatmp[k * m_numRegionsInSlice + i][0] = regionsStartTable[nearestReg + i];
                datatmp[k * m_numRegionsInSlice + i][1] = 2 * regionsStartTable[regionStartYOffset + (nearestReg + i)];
                datatmp[k * m_numRegionsInSlice + i][2] = 2 * regionsStartTable[regionStartYOffset + nearestReg];
                int32_t tmpY                            = 2 * regionsStartTable[regionStartYOffset + (nearestReg + m_numRegionsInSlice)];
                datatmp[k * m_numRegionsInSlice + i][3] = (uint16_t)((tmpY != 0) ? tmpY : (m_frameHeight) >> 4);
                datatmp[k * m_numRegionsInSlice + i][4] = offsetToTheRegionStart[k] & 0x0FFFF;
                datatmp[k * m_numRegionsInSlice + i][5] = 0;
                datatmp[k * m_numRegionsInSlice + i][6] = 0;
                datatmp[k * m_numRegionsInSlice + i][7] = (uint16_t)(offsetY + 4 * regionsStartTable[regionStartYOffset + nearestReg] + (4 * ((i * offsetDelta) >> 1)));
            }
        }
    }

    if (m_enable26WalkingPattern)
    {
        InitParamForWalkerVfe26(m_numRegionsInSlice, maxHeight);
    }
    else
    {
        InitParamForWalkerVfe26z(m_numRegionsInSlice, maxHeight);
    }

    MOS_LOCK_PARAMS lockFlags;
    MOS_ZeroMemory(&lockFlags, sizeof(lockFlags));
    lockFlags.WriteOnly = true;

    PCODECHAL_ENCODE_HEVC_WALKING_CONTROL_REGION region;
    region = (PCODECHAL_ENCODE_HEVC_WALKING_CONTROL_REGION)m_osInterface->pfnLockResource(
        m_osInterface,
        &m_concurrentThreadSurface[m_concurrentThreadIndex].OsResource,
        &lockFlags);

    if (region == nullptr)
    {
        eStatus = MOS_STATUS_NULL_POINTER;
        return eStatus;
    }

    MOS_ZeroMemory(region, sizeof(*region) * HEVC_CONCURRENT_SURFACE_HEIGHT);

    for (auto i = 0; i < 1024; i += 64)
    {
        MOS_SecureMemcpy(((uint8_t* )region) + i, 32, (uint8_t* )datatmp[i / 64], 32);
    }

    m_walkingPatternParam.dwMaxHeightInRegion = m_enable26WalkingPattern ? maxHeight : maxHeight * 2;
    ;
    m_walkingPatternParam.dwNumRegion = numRegions;
    m_walkingPatternParam.dwNumUnitsInRegion =
        (frameWidthInUnits + 2 * (maxHeight - 1) + m_numRegionsInSlice - 1) / m_numRegionsInSlice;

    m_osInterface->pfnUnlockResource(
        m_osInterface,
        &m_concurrentThreadSurface[m_concurrentThreadIndex].OsResource);

    CODECHAL_DEBUG_TOOL(
        eStatus = m_debugInterface->DumpSurface(
            &m_concurrentThreadSurface[m_concurrentThreadIndex],
            CodechalDbgAttr::attrOutput,
            "HEVC_B_MBENC_Out",
            CODECHAL_MEDIA_STATE_HEVC_B_MBENC);
    )

    return eStatus;
}

uint32_t CodechalEncHevcStateG9::GetMaxBtCount()
{
    auto btIdxAlignment = m_stateHeapInterface->pStateHeapInterface->GetBtIdxAlignment();

    // Init/Reset BRC kernel
    uint32_t btCountPhase1 = MOS_ALIGN_CEIL(
        m_brcKernelStates[CODECHAL_HEVC_BRC_INIT].KernelParams.iBTCount,
        btIdxAlignment);

    // 4x, 16x DS, 2x DS, 4x ME, 16x ME, 32x ME, and coarse intra kernel
    uint32_t btCountPhase2 = MOS_ALIGN_CEIL(m_brcKernelStates[CODECHAL_HEVC_BRC_COARSE_INTRA].KernelParams.iBTCount, btIdxAlignment) +  // coarse intra
                             2 * MOS_ALIGN_CEIL(m_scaling4xKernelStates[0].KernelParams.iBTCount, btIdxAlignment) +                     // 4x and 16x DS
                             MOS_ALIGN_CEIL(m_scaling2xKernelStates[0].KernelParams.iBTCount, btIdxAlignment) +                         // 2x DS
                             3 * MOS_ALIGN_CEIL(m_hmeKernel ? m_hmeKernel->GetBTCount() : 0, btIdxAlignment);                           // 4x, 16x, and 32x ME

    // BRC update kernels and 6 I kernels
    uint32_t btCountPhase3 = MOS_ALIGN_CEIL(m_brcKernelStates[CODECHAL_HEVC_BRC_FRAME_UPDATE].KernelParams.iBTCount, btIdxAlignment) +
                             MOS_ALIGN_CEIL(m_brcKernelStates[CODECHAL_HEVC_BRC_LCU_UPDATE].KernelParams.iBTCount, btIdxAlignment) +
                             MOS_ALIGN_CEIL(m_mbEncKernelStates[CODECHAL_HEVC_MBENC_2xSCALING].KernelParams.iBTCount, btIdxAlignment) +
                             MOS_ALIGN_CEIL(m_mbEncKernelStates[CODECHAL_HEVC_MBENC_16x16SAD].KernelParams.iBTCount, btIdxAlignment) +
                             MOS_ALIGN_CEIL(m_mbEncKernelStates[CODECHAL_HEVC_MBENC_16x16MD].KernelParams.iBTCount, btIdxAlignment) +
                             MOS_ALIGN_CEIL(m_mbEncKernelStates[CODECHAL_HEVC_MBENC_8x8PU].KernelParams.iBTCount, btIdxAlignment) +
                             MOS_ALIGN_CEIL(m_mbEncKernelStates[CODECHAL_HEVC_MBENC_8x8FMODE].KernelParams.iBTCount, btIdxAlignment);

    btCountPhase3 += MOS_MAX(
        MOS_ALIGN_CEIL(m_mbEncKernelStates[CODECHAL_HEVC_MBENC_32x32MD].KernelParams.iBTCount, btIdxAlignment),
        MOS_ALIGN_CEIL(m_mbEncKernelStates[CODECHAL_HEVC_MBENC_32x32INTRACHECK].KernelParams.iBTCount, btIdxAlignment));

    if (MEDIA_IS_SKU(m_skuTable, FtrEncodeHEVC10bit))
    {
        btCountPhase3 += MOS_ALIGN_CEIL(m_mbEncKernelStates[CODECHAL_HEVC_MBENC_DS_COMBINED].KernelParams.iBTCount, btIdxAlignment);
    }

    // BRC update kernels and two B kernels
    uint32_t btCountPhase4 = MOS_ALIGN_CEIL(m_brcKernelStates[CODECHAL_HEVC_BRC_FRAME_UPDATE].KernelParams.iBTCount, btIdxAlignment) +
                             MOS_ALIGN_CEIL(m_brcKernelStates[CODECHAL_HEVC_BRC_LCU_UPDATE].KernelParams.iBTCount, btIdxAlignment) +
                             MOS_ALIGN_CEIL(m_mbEncKernelStates[CODECHAL_HEVC_MBENC_BENC].KernelParams.iBTCount, btIdxAlignment) +
                             MOS_ALIGN_CEIL(m_mbEncKernelStates[CODECHAL_HEVC_MBENC_BPAK].KernelParams.iBTCount, btIdxAlignment);

    uint32_t maxBtCount = MOS_MAX(btCountPhase1, btCountPhase2);
    maxBtCount = MOS_MAX(maxBtCount, btCountPhase3);
    maxBtCount = MOS_MAX(maxBtCount, btCountPhase4);

    return maxBtCount;
}

MOS_STATUS CodechalEncHevcStateG9::AllocateEncResources()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    m_sliceMap = (PCODECHAL_ENCODE_HEVC_SLICE_MAP)MOS_AllocAndZeroMemory(
        m_widthAlignedMaxLcu * m_heightAlignedMaxLcu * sizeof(m_sliceMap[0]));
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_sliceMap);

    uint32_t downscaling2xWidth  = m_widthAlignedMaxLcu >> 1;
    uint32_t downscaling2xHeight = m_heightAlignedMaxLcu >> 1;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateSurface(
        &m_scaled2xSurface,
        downscaling2xWidth,
        downscaling2xHeight,
        "2x Downscaling"));

    uint32_t width  = m_widthAlignedMaxLcu >> 3;
    uint32_t height = m_heightAlignedMaxLcu >> 5;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer2D(
        &m_sliceMapSurface,
        width,
        height,
        "Slice Map"));

    uint32_t size = 32 * (m_widthAlignedMaxLcu >> 5) * (m_heightAlignedMaxLcu >> 5);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer(
        &m_32x32PuOutputData,
        size,
        "32x32 PU Output Data"));

    size = 8 * 4 * (m_widthAlignedMaxLcu >> 4) * (m_heightAlignedMaxLcu >> 4);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer(
        &m_sad16x16Pu,
        size,
        "SAD 16x16 PU"));

    size = 64 * (m_widthAlignedMaxLcu >> 4) * (m_heightAlignedMaxLcu >> 4);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer(
        &m_vme8x8Mode,
        size,
        "VME 8x8 mode"));

    size = 32 * (m_widthAlignedMaxLcu >> 3) * (m_heightAlignedMaxLcu >> 3);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer(
        &m_intraMode,
        size,
        "Intra mode"));

    size = 16 * (m_widthAlignedMaxLcu >> 4) * (m_heightAlignedMaxLcu >> 4);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer(
        &m_intraDist,
        size,
        "Intra dist"));

    // Change the surface size
    width  = m_widthAlignedMaxLcu >> 1;
    height = m_heightAlignedMaxLcu >> 4;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer2D(
        &m_minDistortion,
        width,
        height,
        "Min distortion surface"));

    width = sizeof(CODECHAL_ENCODE_HEVC_WALKING_CONTROL_REGION);
    height = HEVC_CONCURRENT_SURFACE_HEIGHT;
    for (uint32_t i = 0; i < NUM_CONCURRENT_THREAD; i++)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer2D(
            &m_concurrentThreadSurface[i],
            width,
            height,
            "Concurrent Thread"));
    }

    //size = (dwWidthAlignedMaxLCU * dwHeightAlignedMaxLCU / 4);
    size = (m_widthAlignedMaxLcu * m_heightAlignedMaxLcu / 4) + GPUMMU_WA_PADDING;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer(
        &m_mvIndex,
        size,
        "MV index surface"));

    //size = (dwWidthAlignedMaxLCU * dwHeightAlignedMaxLCU / 2);
    size = (m_widthAlignedMaxLcu * m_heightAlignedMaxLcu / 2) + GPUMMU_WA_PADDING;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer(
        &m_mvpIndex,
        size,
        "MVP index surface"));

    size = m_widthAlignedMaxLcu * m_heightAlignedMaxLcu;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer(
        &m_vmeSavedUniSic,
        size,
        "VME Saved UniSic surface"));

    width  = m_widthAlignedMaxLcu >> 3;
    height = m_heightAlignedMaxLcu >> 5;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer2D(
        &m_simplestIntraSurface,
        width,
        height,
        "Simplest Intra surface"));

    m_allocator->AllocateResource(m_standard, 1024, 1, brcInputForEncKernel, "brcInputForEncKernel", true);

    if (m_hmeKernel && m_hmeSupported)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hmeKernel->AllocateResources());
    }

    // BRC Distortion Surface which will be used in ME as the output, too
    // In addition, this surface should also be allocated as BRC resource once ENC is enabled
    width = MOS_ALIGN_CEIL((m_downscaledWidthInMb4x * 8), 64);
    height = MOS_ALIGN_CEIL((m_downscaledHeightInMb4x * 4), 8);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer2D(
        &m_brcBuffers.sMeBrcDistortionBuffer,
        width,
        height,
        "BRC distortion surface"));

    if (MEDIA_IS_SKU(m_skuTable, FtrEncodeHEVC10bit))
    {
        // adding 10 bit support for KBL : output surface for format conversion from 10bit to 8 bit
        for (uint32_t i = 0; i < NUM_FORMAT_CONV_FRAMES; i++)
        {
            if (Mos_ResourceIsNull(&m_formatConvertedSurface[i].OsResource))
            {
                width  = m_widthAlignedMaxLcu;
                height = m_heightAlignedMaxLcu;

                CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateSurface(
                    &m_formatConvertedSurface[i],
                    width,
                    height,
                    "Format Converted Surface"));
            }
        }

        if (Mos_ResourceIsNull(&m_resMbStatisticsSurface.sResource))
        {
            size = 52 * m_picWidthInMb * m_picHeightInMb; // 13 DWs or 52 bytes for statistics per MB

            CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer(
                &m_resMbStatisticsSurface,
                size,
                "MB stats surface"));
        }
    }

    // ROI
    // ROI buffer size uses MB units for HEVC, not LCU
    width  = MOS_ALIGN_CEIL(m_picWidthInMb * 4, 64);
    height = MOS_ALIGN_CEIL(m_picHeightInMb, 8);

    MOS_ZeroMemory(&m_roiSurface, sizeof(m_roiSurface));
    m_roiSurface.TileType       = MOS_TILE_LINEAR;
    m_roiSurface.bArraySpacing  = true;
    m_roiSurface.Format         = Format_Buffer_2D;
    m_roiSurface.dwWidth        = width;
    m_roiSurface.dwPitch        = width;
    m_roiSurface.dwHeight       = height;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer2D(
        &m_roiSurface,
        width,
        height,
        "ROI Buffer"));

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG9::FreeEncResources()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_Delete(m_meKernelState);
    m_meKernelState = nullptr;
    MOS_FreeMemory(m_meKernelBindingTable);
    m_meKernelBindingTable = nullptr;

    MOS_DeleteArray(m_mbEncKernelStates);
    m_mbEncKernelStates = nullptr;
    MOS_FreeMemory(m_mbEncKernelBindingTable);
    m_mbEncKernelBindingTable = nullptr;

    MOS_DeleteArray(m_brcKernelStates);
    m_brcKernelStates = nullptr;
    MOS_FreeMemory(m_brcKernelBindingTable);
    m_brcKernelBindingTable = nullptr;

    MOS_FreeMemory(m_surfaceParams); m_surfaceParams = nullptr;

    for (uint32_t i = 0; i < NUM_FORMAT_CONV_FRAMES; i++)
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &m_formatConvertedSurface[i].OsResource);
    }

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_scaled2xSurface.OsResource);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_resMbStatisticsSurface.sResource);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_sliceMapSurface.OsResource);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_32x32PuOutputData.sResource);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_sad16x16Pu.sResource);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_vme8x8Mode.sResource);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_intraMode.sResource);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_intraDist.sResource);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_mvIndex.sResource);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_mvpIndex.sResource);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_vmeSavedUniSic.sResource);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_minDistortion.OsResource);

    for (uint32_t i = 0; i < NUM_CONCURRENT_THREAD; i++)
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &m_concurrentThreadSurface[i].OsResource);
    }

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_simplestIntraSurface.OsResource);

    if (m_encEnabled)
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &m_brcBuffers.sMeBrcDistortionBuffer.OsResource);
    }

    MOS_FreeMemory(m_sliceMap);
    m_sliceMap = nullptr;

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_roiSurface.OsResource);

#if (_DEBUG || _RELEASE_INTERNAL)
    if (m_swBrcMode != nullptr)
    {
        m_osInterface->pfnFreeLibrary(m_swBrcMode);
        m_swBrcMode = nullptr;
    }
#endif // (_DEBUG || _RELEASE_INTERNAL)

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG9::SendMeSurfaces(
    CodechalHwInterface                 *hwInterface,
    PMOS_COMMAND_BUFFER                 cmdBuffer,
    MeSurfaceParams                     *params)
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pKernelState);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pCurrOriginalPic);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->ps4xMeMvDataBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->psMeDistortionBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->psMeBrcDistortionBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pMeBindingTable);

    PMOS_SURFACE currScaledSurface = nullptr, meMvDataBuffer = nullptr;
    if (params->b32xMeInUse)
    {
        CODECHAL_ENCODE_CHK_NULL_RETURN(params->ps32xMeMvDataBuffer);
        currScaledSurface = m_trackedBuf->Get32xDsSurface(CODEC_CURR_TRACKED_BUFFER);
        meMvDataBuffer = params->ps32xMeMvDataBuffer;
    }
    else if (params->b16xMeInUse)
    {
        CODECHAL_ENCODE_CHK_NULL_RETURN(params->ps16xMeMvDataBuffer);
        currScaledSurface = m_trackedBuf->Get16xDsSurface(CODEC_CURR_TRACKED_BUFFER);
        meMvDataBuffer = params->ps16xMeMvDataBuffer;
    }
    else
    {
        currScaledSurface = m_trackedBuf->Get4xDsSurface(CODEC_CURR_TRACKED_BUFFER);
        meMvDataBuffer = params->ps4xMeMvDataBuffer;
    }

    // Reference height and width information should be taken from the current scaled surface rather
    // than from the reference scaled surface in the case of PAFF.
    uint32_t width = MOS_ALIGN_CEIL(params->dwDownscaledWidthInMb * 32, 64);
    uint32_t height = params->dwDownscaledHeightInMb * 4 * CODECHAL_ENCODE_ME_DATA_SIZE_MULTIPLIER;
    // Force the values
    meMvDataBuffer->dwWidth = width;
    meMvDataBuffer->dwHeight = height;
    meMvDataBuffer->dwPitch = width;

    MeKernelBindingTable* meBindingTable = params->pMeBindingTable;
    CODECHAL_SURFACE_CODEC_PARAMS surfaceParams;
    MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
    surfaceParams.bIs2DSurface = true;
    surfaceParams.bMediaBlockRW = true;
    surfaceParams.psSurface = meMvDataBuffer;
    surfaceParams.dwOffset = 0;
    surfaceParams.dwCacheabilityControl = hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value;
    surfaceParams.dwBindingTableOffset = meBindingTable->dwMEMVDataSurface;
    surfaceParams.bIsWritable = true;
    surfaceParams.bRenderTarget = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        hwInterface,
        cmdBuffer,
        &surfaceParams,
        params->pKernelState));

    if (params->b16xMeInUse && params->b32xMeEnabled)
    {
        // Pass 32x MV to 16x ME operation
        MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
        surfaceParams.bIs2DSurface = true;
        surfaceParams.bMediaBlockRW = true;
        surfaceParams.psSurface = params->ps32xMeMvDataBuffer;
        surfaceParams.dwOffset = 0;
        surfaceParams.dwCacheabilityControl = hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value;
        surfaceParams.dwBindingTableOffset = meBindingTable->dw32xMEMVDataSurface;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            hwInterface,
            cmdBuffer,
            &surfaceParams,
            params->pKernelState));
    }
    else if (!params->b32xMeInUse && params->b16xMeEnabled)
    {
        // Pass 16x MV to 4x ME operation
        MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
        surfaceParams.bIs2DSurface = true;
        surfaceParams.bMediaBlockRW = true;
        surfaceParams.psSurface = params->ps16xMeMvDataBuffer;
        surfaceParams.dwOffset = 0;
        surfaceParams.dwCacheabilityControl = hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value;
        surfaceParams.dwBindingTableOffset = meBindingTable->dw16xMEMVDataSurface;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            hwInterface,
            cmdBuffer,
            &surfaceParams,
            params->pKernelState));
    }

    // Insert Distortion buffers only for 4xMe case
    if (!params->b32xMeInUse && !params->b16xMeInUse)
    {
        MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
        surfaceParams.bIs2DSurface = true;
        surfaceParams.bMediaBlockRW = true;
        surfaceParams.psSurface = params->psMeBrcDistortionBuffer;
        surfaceParams.dwOffset = 0;
        surfaceParams.dwBindingTableOffset = meBindingTable->dwMEBRCDist;
        surfaceParams.dwCacheabilityControl = hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_BRC_ME_DISTORTION_ENCODE].Value;
        surfaceParams.bIsWritable = true;
        surfaceParams.bRenderTarget = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            hwInterface,
            cmdBuffer,
            &surfaceParams,
            params->pKernelState));

        MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
        surfaceParams.bIs2DSurface = true;
        surfaceParams.bMediaBlockRW = true;
        surfaceParams.psSurface = params->psMeDistortionBuffer;
        surfaceParams.dwOffset = 0;
        surfaceParams.dwBindingTableOffset = meBindingTable->dwMEDist;
        surfaceParams.dwCacheabilityControl = hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ME_DISTORTION_ENCODE].Value;
        surfaceParams.bIsWritable = true;
        surfaceParams.bRenderTarget = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            hwInterface,
            cmdBuffer,
            &surfaceParams,
            params->pKernelState));
    }

    // Setup references 1...n
    // LIST 0 references
    const uint8_t currVDirection = CODECHAL_VDIRECTION_FRAME;     // Interlaced not supported
    for (uint8_t refIdx = 0; refIdx <= params->dwNumRefIdxL0ActiveMinus1; refIdx++)
    {
        CODEC_PICTURE refPic = params->pL0RefFrameList[refIdx];
        MOS_SURFACE refScaledSurface = *currScaledSurface;

        if (!CodecHal_PictureIsInvalid(refPic) && params->pPicIdx[refPic.FrameIdx].bValid)
        {
            if (refIdx == 0)
            {
                // Current picture Y - VME
                MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
                surfaceParams.bUseAdvState = true;
                surfaceParams.psSurface = currScaledSurface;
                surfaceParams.dwOffset = 0;
                surfaceParams.dwCacheabilityControl = hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value;
                surfaceParams.dwBindingTableOffset = meBindingTable->dwMECurrForFwdRef;
                surfaceParams.ucVDirection = currVDirection;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                    hwInterface,
                    cmdBuffer,
                    &surfaceParams,
                    params->pKernelState));
            }

            uint8_t refPicIdx = params->pPicIdx[refPic.FrameIdx].ucPicIdx;
            uint8_t scaledIdx = params->ppRefList[refPicIdx]->ucScalingIdx;
            if (params->b32xMeInUse)
            {
                MOS_SURFACE* p32xSurface = m_trackedBuf->Get32xDsSurface(scaledIdx);
                if (p32xSurface != nullptr)
                {
                    refScaledSurface.OsResource = p32xSurface->OsResource;
                }
                else
                {
                    CODECHAL_ENCODE_ASSERTMESSAGE("NULL pointer of DsSurface");
                }
            }
            else if (params->b16xMeInUse)
            {
                MOS_SURFACE* p16xSurface = m_trackedBuf->Get16xDsSurface(scaledIdx);
                if (p16xSurface != nullptr)
                {
                    refScaledSurface.OsResource = p16xSurface->OsResource;
                }
                else
                {
                    CODECHAL_ENCODE_ASSERTMESSAGE("NULL pointer of DsSurface");
                }
            }
            else
            {
                MOS_SURFACE* p4xSurface = m_trackedBuf->Get4xDsSurface(scaledIdx);
                if (p4xSurface != nullptr)
                {
                    refScaledSurface.OsResource = p4xSurface->OsResource;
                }
                else
                {
                    CODECHAL_ENCODE_ASSERTMESSAGE("NULL pointer of DsSurface");
                }
            }
            // L0 Reference picture Y - VME
            MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
            surfaceParams.bUseAdvState = true;
            surfaceParams.psSurface = &refScaledSurface;
            surfaceParams.dwOffset = 0;
            surfaceParams.dwCacheabilityControl = hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value;
            surfaceParams.dwBindingTableOffset = meBindingTable->dwMEFwdRefPicIdx[refIdx];
            surfaceParams.ucVDirection = CODECHAL_VDIRECTION_FRAME;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                hwInterface,
                cmdBuffer,
                &surfaceParams,
                params->pKernelState));
        }
    }

    // Setup references 1...n
    // LIST 1 references
    for (uint8_t refIdx = 0; refIdx <= params->dwNumRefIdxL1ActiveMinus1; refIdx++)
    {
        CODEC_PICTURE refPic = params->pL1RefFrameList[refIdx];
        MOS_SURFACE refScaledSurface = *currScaledSurface;

        if (!CodecHal_PictureIsInvalid(refPic) && params->pPicIdx[refPic.FrameIdx].bValid)
        {
            if (refIdx == 0)
            {
                // Current picture Y - VME
                MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
                surfaceParams.bUseAdvState = true;
                surfaceParams.psSurface = currScaledSurface;
                surfaceParams.dwOffset = 0;
                surfaceParams.dwCacheabilityControl = hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value;
                surfaceParams.dwBindingTableOffset = meBindingTable->dwMECurrForBwdRef;
                surfaceParams.ucVDirection = currVDirection;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                    hwInterface,
                    cmdBuffer,
                    &surfaceParams,
                    params->pKernelState));
            }

            uint8_t refPicIdx = params->pPicIdx[refPic.FrameIdx].ucPicIdx;
            uint8_t scaledIdx = params->ppRefList[refPicIdx]->ucScalingIdx;
            if (params->b32xMeInUse)
            {
                MOS_SURFACE* p32xSurface = m_trackedBuf->Get32xDsSurface(scaledIdx);
                if (p32xSurface != nullptr)
                {
                    refScaledSurface.OsResource = p32xSurface->OsResource;
                }
                else
                {
                    CODECHAL_ENCODE_ASSERTMESSAGE("NULL pointer of DsSurface");
                }
            }
            else if (params->b16xMeInUse)
            {
                MOS_SURFACE* p16xSurface = m_trackedBuf->Get16xDsSurface(scaledIdx);
                if (p16xSurface != nullptr)
                {
                    refScaledSurface.OsResource = p16xSurface->OsResource;
                }
                else
                {
                    CODECHAL_ENCODE_ASSERTMESSAGE("NULL pointer of DsSurface");
                }
            }
            else
            {
                MOS_SURFACE* p4xSurface = m_trackedBuf->Get4xDsSurface(scaledIdx);
                if (p4xSurface != nullptr)
                {
                    refScaledSurface.OsResource = p4xSurface->OsResource;
                }
                else
                {
                    CODECHAL_ENCODE_ASSERTMESSAGE("NULL pointer of DsSurface");
                }
            }
            // L1 Reference picture Y - VME
            MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
            surfaceParams.bUseAdvState = true;
            surfaceParams.psSurface = &refScaledSurface;
            surfaceParams.dwOffset = 0;
            surfaceParams.dwCacheabilityControl = hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value;
            surfaceParams.dwBindingTableOffset = meBindingTable->dwMEBwdRefPicIdx[refIdx];
            surfaceParams.ucVDirection = CODECHAL_VDIRECTION_FRAME;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                hwInterface,
                cmdBuffer,
                &surfaceParams,
                params->pKernelState));
        }
    }

    return eStatus;
}

//------------------------------------------------------------------------------
//| Purpose:    Setup curbe for HEVC ME kernels
//| Return:     N/A
//------------------------------------------------------------------------------
MOS_STATUS CodechalEncHevcStateG9::SetCurbeMe(
    MeCurbeParams* params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pKernelState);

    CODECHAL_ENCODE_ASSERT(params->TargetUsage <= NUM_TARGET_USAGE_MODES);

    uint8_t mvShiftFactor = 0, prevMvReadPosFactor = 0;
    bool useMvFromPrevStep= false, writeDistortions = false;
    uint32_t scaleFactor = 0;
    switch (params->hmeLvl)
    {
    case HME_LEVEL_32x:
        useMvFromPrevStep = HME_FIRST_STEP;
        writeDistortions = false;
        scaleFactor = SCALE_FACTOR_32x;
        mvShiftFactor = MV_SHIFT_FACTOR_32x;
        break;
    case HME_LEVEL_16x:
        useMvFromPrevStep   = (m_b32XMeEnabled) ? HME_FOLLOWING_STEP : HME_FIRST_STEP;
        writeDistortions = false;
        scaleFactor = SCALE_FACTOR_16x;
        mvShiftFactor = MV_SHIFT_FACTOR_16x;
        prevMvReadPosFactor = PREV_MV_READ_POSITION_16x;
        break;
    case HME_LEVEL_4x:
        useMvFromPrevStep   = (m_b16XMeEnabled) ? HME_FOLLOWING_STEP : HME_FIRST_STEP;
        writeDistortions = true;
        scaleFactor = SCALE_FACTOR_4x;
        mvShiftFactor = MV_SHIFT_FACTOR_4x;
        prevMvReadPosFactor = PREV_MV_READ_POSITION_4x;
        break;
    default:
        return MOS_STATUS_INVALID_PARAMETER;
        break;
    }

    CODECHAL_ENC_HEVC_ME_CURBE_G9 cmd;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
        &cmd,
        sizeof(CODECHAL_ENC_HEVC_ME_CURBE_G9),
        m_meCurbeInit,
        sizeof(CODECHAL_ENC_HEVC_ME_CURBE_G9)));

    cmd.DW3.SubPelMode = 3;
    cmd.DW4.PictureHeightMinus1 = CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameFieldHeight / scaleFactor) - 1;
    cmd.DW4.PictureWidth = CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameWidth / scaleFactor);
    cmd.DW5.QpPrimeY = params->pic_init_qp_minus26 + 26 + params->slice_qp_delta;
    cmd.DW6.WriteDistortions = writeDistortions;
    cmd.DW6.UseMvFromPrevStep = useMvFromPrevStep;

    cmd.DW6.SuperCombineDist = m_superCombineDist[params->TargetUsage];
    cmd.DW6.MaxVmvR = 512; // CModel always uses 512 for HME even though B_MB uses (levelIDC)*4

    if (m_pictureCodingType == B_TYPE)
    {
        // This field is irrelevant since we are not using the bi-direct search.
        // set it to 32
        cmd.DW1.BiWeight = 32;
        cmd.DW13.NumRefIdxL1MinusOne = params->num_ref_idx_l1_active_minus1;
        cmd.DW13.NumRefIdxL0MinusOne = params->num_ref_idx_l0_active_minus1;
    }

    cmd.DW15.MvShiftFactor = mvShiftFactor;
    cmd.DW15.PrevMvReadPosFactor = prevMvReadPosFactor;

    // r3 & r4
    uint8_t meMethod = m_meMethod[params->TargetUsage];

    eStatus = MOS_SecureMemcpy(&(cmd.SPDelta), 14 * sizeof(uint32_t), CodechalEncoderState::m_encodeSearchPath[0][meMethod], 14 * sizeof(uint32_t));
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
        return eStatus;
    }

    // r5
    cmd.DW32._4xMeMvOutputDataSurfIndex = CODECHAL_ENCODE_ME_MV_DATA_SURFACE_CM_G9;
    cmd.DW33._16xOr32xMeMvInputDataSurfIndex = (params->hmeLvl == HME_LEVEL_32x) ?
        CODECHAL_ENCODE_32xME_MV_DATA_SURFACE_CM_G9 : CODECHAL_ENCODE_16xME_MV_DATA_SURFACE_CM_G9;
    cmd.DW34._4xMeOutputDistSurfIndex = CODECHAL_ENCODE_ME_DISTORTION_SURFACE_CM_G9;
    cmd.DW35._4xMeOutputBrcDistSurfIndex = CODECHAL_ENCODE_ME_BRC_DISTORTION_CM_G9;
    cmd.DW36.VMEFwdInterPredictionSurfIndex = CODECHAL_ENCODE_ME_CURR_FOR_FWD_REF_CM_G9;
    cmd.DW37.VMEBwdInterPredictionSurfIndex = CODECHAL_ENCODE_ME_CURR_FOR_BWD_REF_CM_G9;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(params->pKernelState->m_dshRegion.AddData(
        &cmd,
        params->pKernelState->dwCurbeOffset,
        sizeof(cmd)));

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG9::SetMbEncKernelParams(MHW_KERNEL_PARAM* kernelParams, uint32_t idx)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(kernelParams);

    auto curbeAlignment = m_stateHeapInterface->pStateHeapInterface->GetCurbeAlignment();

    kernelParams->iThreadCount = m_renderEngineInterface->GetHwCaps()->dwMaxThreads;
    kernelParams->iIdCount     = 1;

    switch (idx)
    {
    case CODECHAL_HEVC_MBENC_2xSCALING:
        kernelParams->iBTCount = CODECHAL_HEVC_SCALING_FRAME_END - CODECHAL_HEVC_SCALING_FRAME_BEGIN;
        kernelParams->iCurbeLength = MOS_ALIGN_CEIL(sizeof(MEDIA_OBJECT_DOWNSCALING_2X_STATIC_DATA_G9), curbeAlignment);
        kernelParams->iBlockWidth = 32;
        kernelParams->iBlockHeight = 32;
        break;

    case CODECHAL_HEVC_MBENC_32x32MD:
        kernelParams->iBTCount = CODECHAL_HEVC_32x32_PU_END - CODECHAL_HEVC_32x32_PU_BEGIN;
        kernelParams->iCurbeLength = MOS_ALIGN_CEIL(sizeof(CODECHAL_ENC_HEVC_I_32x32_PU_MODE_DECISION_CURBE_G9), curbeAlignment);
        kernelParams->iBlockWidth = 32;
        kernelParams->iBlockHeight = 32;
        break;

    case CODECHAL_HEVC_MBENC_16x16SAD:
        kernelParams->iBTCount = CODECHAL_HEVC_16x16_PU_SAD_END - CODECHAL_HEVC_16x16_PU_SAD_BEGIN;
        kernelParams->iCurbeLength = MOS_ALIGN_CEIL(sizeof(CODECHAL_ENC_HEVC_I_16x16_SAD_CURBE_G9), curbeAlignment);
        kernelParams->iBlockWidth = 16;
        kernelParams->iBlockHeight = 16;
        break;

    case CODECHAL_HEVC_MBENC_16x16MD:
        kernelParams->iBTCount = CODECHAL_HEVC_16x16_PU_MD_END - CODECHAL_HEVC_16x16_PU_MD_BEGIN;
        kernelParams->iCurbeLength = MOS_ALIGN_CEIL(sizeof(CODECHAL_ENC_HEVC_I_16x16_PU_MODEDECISION_CURBE_G9), curbeAlignment);
        kernelParams->iBlockWidth = 32;
        kernelParams->iBlockHeight = 32;
        break;

    case CODECHAL_HEVC_MBENC_8x8PU:
        kernelParams->iBTCount = CODECHAL_HEVC_8x8_PU_END - CODECHAL_HEVC_8x8_PU_BEGIN;
        kernelParams->iCurbeLength = MOS_ALIGN_CEIL(sizeof(CODECHAL_ENC_HEVC_I_8x8_PU_CURBE_G9), curbeAlignment);
        kernelParams->iBlockWidth = 8;
        kernelParams->iBlockHeight = 8;
        break;

    case CODECHAL_HEVC_MBENC_8x8FMODE:
        kernelParams->iBTCount = CODECHAL_HEVC_8x8_PU_FMODE_END - CODECHAL_HEVC_8x8_PU_FMODE_BEGIN;
        kernelParams->iCurbeLength = MOS_ALIGN_CEIL(sizeof(CODECHAL_ENC_HEVC_I_8x8_PU_FMODE_CURBE_G9), curbeAlignment);
        kernelParams->iBlockWidth = 32;
        kernelParams->iBlockHeight = 32;
        break;

    case CODECHAL_HEVC_MBENC_32x32INTRACHECK:
        kernelParams->iBTCount = CODECHAL_HEVC_B_32x32_PU_END - CODECHAL_HEVC_B_32x32_PU_BEGIN;
        kernelParams->iCurbeLength = MOS_ALIGN_CEIL(sizeof(CODECHAL_ENC_HEVC_B_32x32_PU_INTRA_CHECK_CURBE_G9), curbeAlignment);
        kernelParams->iBlockWidth = 32;
        kernelParams->iBlockHeight = 32;
        break;

    case CODECHAL_HEVC_MBENC_BENC:
    case CODECHAL_HEVC_MBENC_ADV:
        kernelParams->iBTCount = CODECHAL_HEVC_B_MBENC_END - CODECHAL_HEVC_B_MBENC_BEGIN;
        kernelParams->iCurbeLength = MOS_ALIGN_CEIL(sizeof(CODECHAL_ENC_HEVC_B_MB_ENC_CURBE_G9), curbeAlignment);
        kernelParams->iBlockWidth = 16;
        kernelParams->iBlockHeight = 16;
        break;

    case CODECHAL_HEVC_MBENC_BPAK:
        kernelParams->iBTCount = CODECHAL_HEVC_B_PAK_END - CODECHAL_HEVC_B_PAK_BEGIN;
        kernelParams->iCurbeLength = MOS_ALIGN_CEIL(sizeof(CODECHAL_ENC_HEVC_B_PAK_CURBE_G9), curbeAlignment);
        kernelParams->iBlockWidth = 32;
        kernelParams->iBlockHeight = 32;
        break;

    case CODECHAL_HEVC_MBENC_DS_COMBINED:
        if (MEDIA_IS_SKU(m_skuTable, FtrEncodeHEVC10bit))
        {
            kernelParams->iBTCount = CODECHAL_HEVC_DS_COMBINED_END - CODECHAL_HEVC_DS_COMBINED_BEGIN;
            uint32_t dsCombinedKernelCurbeSize = sizeof(CODECHAL_ENC_HEVC_DS_COMBINED_CURBE_G9);
            kernelParams->iCurbeLength = MOS_ALIGN_CEIL(dsCombinedKernelCurbeSize, curbeAlignment);
            kernelParams->iBlockWidth = 8;
            kernelParams->iBlockHeight = 8;
        }
        else
        {
            CODECHAL_ENCODE_ASSERT(false);
            eStatus = MOS_STATUS_INVALID_PARAMETER;
        }
        break;

    case CODECHAL_HEVC_MBENC_PENC:
    case CODECHAL_HEVC_MBENC_ADV_P:
        kernelParams->iBTCount = CODECHAL_HEVC_P_MBENC_END - CODECHAL_HEVC_P_MBENC_BEGIN;
        //P MBEnc curbe has one less DWord than B MBEnc curbe
        kernelParams->iCurbeLength = MOS_ALIGN_CEIL(sizeof(CODECHAL_ENC_HEVC_B_MB_ENC_CURBE_G9) - sizeof(uint32_t), (size_t)curbeAlignment);
        kernelParams->iBlockWidth = 16;
        kernelParams->iBlockHeight = 16;
        break;

    default:
        CODECHAL_ENCODE_ASSERT(false);
        eStatus = MOS_STATUS_INVALID_PARAMETER;
    }

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG9::SetMbEncBindingTable(PCODECHAL_ENCODE_BINDING_TABLE_GENERIC bindingTable, uint32_t idx)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(bindingTable);

    MOS_ZeroMemory(bindingTable, sizeof(*bindingTable));
    bindingTable->dwMediaState = ConvertKrnOpsToMediaState(ENC_MBENC, idx);

    switch (idx)
    {
    case CODECHAL_HEVC_MBENC_2xSCALING:
        bindingTable->dwNumBindingTableEntries = CODECHAL_HEVC_SCALING_FRAME_END - CODECHAL_HEVC_SCALING_FRAME_BEGIN;
        bindingTable->dwBindingTableStartOffset = CODECHAL_HEVC_SCALING_FRAME_BEGIN;
        break;

    case CODECHAL_HEVC_MBENC_32x32MD:
        bindingTable->dwNumBindingTableEntries = CODECHAL_HEVC_32x32_PU_END - CODECHAL_HEVC_32x32_PU_BEGIN;
        bindingTable->dwBindingTableStartOffset = CODECHAL_HEVC_32x32_PU_BEGIN;
        break;

    case CODECHAL_HEVC_MBENC_16x16SAD:
        bindingTable->dwNumBindingTableEntries = CODECHAL_HEVC_16x16_PU_SAD_END - CODECHAL_HEVC_16x16_PU_SAD_BEGIN;
        bindingTable->dwBindingTableStartOffset = CODECHAL_HEVC_16x16_PU_SAD_BEGIN;
        break;

    case CODECHAL_HEVC_MBENC_16x16MD:
        bindingTable->dwNumBindingTableEntries = CODECHAL_HEVC_16x16_PU_MD_END - CODECHAL_HEVC_16x16_PU_MD_BEGIN;
        bindingTable->dwBindingTableStartOffset = CODECHAL_HEVC_16x16_PU_MD_BEGIN;
        break;

    case CODECHAL_HEVC_MBENC_8x8PU:
        bindingTable->dwNumBindingTableEntries = CODECHAL_HEVC_8x8_PU_END - CODECHAL_HEVC_8x8_PU_BEGIN;
        bindingTable->dwBindingTableStartOffset = CODECHAL_HEVC_8x8_PU_BEGIN;
        break;

    case CODECHAL_HEVC_MBENC_8x8FMODE:
        bindingTable->dwNumBindingTableEntries = CODECHAL_HEVC_8x8_PU_FMODE_END - CODECHAL_HEVC_8x8_PU_FMODE_BEGIN;
        bindingTable->dwBindingTableStartOffset = CODECHAL_HEVC_8x8_PU_FMODE_BEGIN;
        break;

    case CODECHAL_HEVC_MBENC_32x32INTRACHECK:
        bindingTable->dwNumBindingTableEntries = CODECHAL_HEVC_B_32x32_PU_END - CODECHAL_HEVC_B_32x32_PU_BEGIN;
        bindingTable->dwBindingTableStartOffset = CODECHAL_HEVC_B_32x32_PU_BEGIN;
        break;

    case CODECHAL_HEVC_MBENC_BENC:
    case CODECHAL_HEVC_MBENC_ADV:
        bindingTable->dwNumBindingTableEntries = CODECHAL_HEVC_B_MBENC_END - CODECHAL_HEVC_B_MBENC_BEGIN;
        bindingTable->dwBindingTableStartOffset = CODECHAL_HEVC_B_MBENC_BEGIN;
        break;

    case CODECHAL_HEVC_MBENC_BPAK:
        bindingTable->dwNumBindingTableEntries = CODECHAL_HEVC_B_PAK_END - CODECHAL_HEVC_B_PAK_BEGIN;
        bindingTable->dwBindingTableStartOffset = CODECHAL_HEVC_B_PAK_BEGIN;
        break;

    case CODECHAL_HEVC_MBENC_DS_COMBINED:
        bindingTable->dwNumBindingTableEntries = CODECHAL_HEVC_DS_COMBINED_END - CODECHAL_HEVC_DS_COMBINED_BEGIN;
        bindingTable->dwBindingTableStartOffset = CODECHAL_HEVC_DS_COMBINED_BEGIN;
        break;

    case CODECHAL_HEVC_MBENC_PENC:
    case CODECHAL_HEVC_MBENC_ADV_P:
        bindingTable->dwNumBindingTableEntries = CODECHAL_HEVC_P_MBENC_END - CODECHAL_HEVC_P_MBENC_BEGIN;
        bindingTable->dwBindingTableStartOffset = CODECHAL_HEVC_P_MBENC_BEGIN;
        break;

    default:
        CODECHAL_ENCODE_ASSERT(false);
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    for (uint32_t i = 0; i < bindingTable->dwNumBindingTableEntries; i++)
    {
        bindingTable->dwBindingTableEntries[i] = i;
    }

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG9::SetBrcKernelParams(MHW_KERNEL_PARAM* kernelParams, uint32_t idx)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(kernelParams);

    auto curbeAlignment = m_stateHeapInterface->pStateHeapInterface->GetCurbeAlignment();

    kernelParams->iThreadCount = m_renderEngineInterface->GetHwCaps()->dwMaxThreads;
    kernelParams->iIdCount = 1;

    // Only LCU-based update kernel is running at multple threads. Others run in the single thread.
    switch (idx)
    {
    case CODECHAL_HEVC_BRC_COARSE_INTRA:
        kernelParams->iBTCount     = CODECHAL_HEVC_COARSE_INTRA_END - CODECHAL_HEVC_COARSE_INTRA_BEGIN;
        kernelParams->iCurbeLength = MOS_ALIGN_CEIL(sizeof(CODECHAL_ENC_HEVC_COARSE_INTRA_CURBE_G9), curbeAlignment);
        kernelParams->iBlockWidth  = 32;
        kernelParams->iBlockHeight = 32;
        break;

    case CODECHAL_HEVC_BRC_INIT:
        kernelParams->iBTCount     = CODECHAL_HEVC_BRC_INIT_RESET_END - CODECHAL_HEVC_BRC_INIT_RESET_BEGIN;
        kernelParams->iCurbeLength = MOS_ALIGN_CEIL(sizeof(CODECHAL_ENC_HEVC_BRC_INITRESET_CURBE_G9), curbeAlignment);
        kernelParams->iBlockWidth  = 32;
        kernelParams->iBlockHeight = 32;
        break;

    case CODECHAL_HEVC_BRC_RESET:
        kernelParams->iBTCount     = CODECHAL_HEVC_BRC_INIT_RESET_END - CODECHAL_HEVC_BRC_INIT_RESET_BEGIN;
        kernelParams->iCurbeLength = MOS_ALIGN_CEIL(sizeof(CODECHAL_ENC_HEVC_BRC_INITRESET_CURBE_G9), curbeAlignment);
        kernelParams->iBlockWidth  = 32;
        kernelParams->iBlockHeight = 32;
        break;

    case CODECHAL_HEVC_BRC_FRAME_UPDATE:
        kernelParams->iBTCount     = CODECHAL_HEVC_BRC_UPDATE_END - CODECHAL_HEVC_BRC_UPDATE_BEGIN;
        kernelParams->iCurbeLength = MOS_ALIGN_CEIL(sizeof(CODECHAL_ENC_HEVC_BRC_UPDATE_CURBE_G9), curbeAlignment);
        kernelParams->iBlockWidth  = 32;
        kernelParams->iBlockHeight = 32;
        break;

    case CODECHAL_HEVC_BRC_LCU_UPDATE:
        kernelParams->iBTCount     = CODECHAL_HEVC_BRC_LCU_UPDATE_END - CODECHAL_HEVC_BRC_LCU_UPDATE_BEGIN;
        kernelParams->iCurbeLength = MOS_ALIGN_CEIL(sizeof(CODECHAL_ENC_HEVC_BRC_UPDATE_CURBE_G9), curbeAlignment);
        kernelParams->iBlockWidth  = 128;
        kernelParams->iBlockHeight = 128;
        break;

    default:
        CODECHAL_ENCODE_ASSERT(false);
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG9::SetBrcBindingTable(PCODECHAL_ENCODE_BINDING_TABLE_GENERIC bindingTable, uint32_t idx)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(bindingTable);

    MOS_ZeroMemory(bindingTable, sizeof(*bindingTable));
    bindingTable->dwMediaState = ConvertKrnOpsToMediaState(ENC_BRC, idx);

    switch (idx)
    {
    case CODECHAL_HEVC_BRC_COARSE_INTRA:
        bindingTable->dwNumBindingTableEntries  = CODECHAL_HEVC_COARSE_INTRA_END - CODECHAL_HEVC_COARSE_INTRA_BEGIN;
        bindingTable->dwBindingTableStartOffset = CODECHAL_HEVC_COARSE_INTRA_BEGIN;
        break;

    case CODECHAL_HEVC_BRC_INIT:
        bindingTable->dwNumBindingTableEntries  = CODECHAL_HEVC_BRC_INIT_RESET_END - CODECHAL_HEVC_BRC_INIT_RESET_BEGIN;
        bindingTable->dwBindingTableStartOffset = CODECHAL_HEVC_BRC_INIT_RESET_BEGIN;
        break;

    case CODECHAL_HEVC_BRC_RESET:
        bindingTable->dwNumBindingTableEntries  = CODECHAL_HEVC_BRC_INIT_RESET_END - CODECHAL_HEVC_BRC_INIT_RESET_BEGIN;
        bindingTable->dwBindingTableStartOffset = CODECHAL_HEVC_BRC_INIT_RESET_BEGIN;
        break;

    case CODECHAL_HEVC_BRC_FRAME_UPDATE:
        bindingTable->dwNumBindingTableEntries  = CODECHAL_HEVC_BRC_UPDATE_END - CODECHAL_HEVC_BRC_UPDATE_BEGIN;
        bindingTable->dwBindingTableStartOffset = CODECHAL_HEVC_BRC_UPDATE_BEGIN;
        break;

    case CODECHAL_HEVC_BRC_LCU_UPDATE:
        bindingTable->dwNumBindingTableEntries  = CODECHAL_HEVC_BRC_LCU_UPDATE_END - CODECHAL_HEVC_BRC_LCU_UPDATE_BEGIN;
        bindingTable->dwBindingTableStartOffset = CODECHAL_HEVC_BRC_LCU_UPDATE_BEGIN;
        break;

    default:
        CODECHAL_ENCODE_ASSERT(false);
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    for (uint32_t i = 0; i < bindingTable->dwNumBindingTableEntries; i++)
    {
        bindingTable->dwBindingTableEntries[i] = i;
    }

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG9::InitKernelStateBrc()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    m_numBrcKrnStates = CODECHAL_HEVC_BRC_NUM;

    m_brcKernelStates = MOS_NewArray(MHW_KERNEL_STATE, m_numBrcKrnStates);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_brcKernelStates);

    m_brcKernelBindingTable = (PCODECHAL_ENCODE_BINDING_TABLE_GENERIC)MOS_AllocAndZeroMemory(
        sizeof(GenericBindingTable) * m_numBrcKrnStates);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_brcKernelBindingTable);

    auto kernelStatePtr = m_brcKernelStates;

    for (uint32_t krnStateIdx = 0; krnStateIdx < m_numBrcKrnStates; krnStateIdx++)
    {
        auto kernelSize = m_combinedKernelSize;
        CODECHAL_KERNEL_HEADER currKrnHeader;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(pfnGetKernelHeaderAndSize(
            m_kernelBinary,
            ENC_BRC,
            krnStateIdx,
            &currKrnHeader,
            &kernelSize));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetBrcKernelParams(
            &kernelStatePtr->KernelParams,
            krnStateIdx));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetBrcBindingTable(
            &m_brcKernelBindingTable[krnStateIdx], krnStateIdx));

        kernelStatePtr->dwCurbeOffset = m_stateHeapInterface->pStateHeapInterface->GetSizeofCmdInterfaceDescriptorData();
        kernelStatePtr->KernelParams.pBinary = m_kernelBinary + (currKrnHeader.KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);
        kernelStatePtr->KernelParams.iSize = kernelSize;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnCalculateSshAndBtSizesRequested(
            m_stateHeapInterface,
            kernelStatePtr->KernelParams.iBTCount,
            &kernelStatePtr->dwSshSize,
            &kernelStatePtr->dwBindingTableSize));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->MhwInitISH(m_stateHeapInterface, kernelStatePtr));

        kernelStatePtr++;
    }

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG9::InitKernelStateMbEnc()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    if(MEDIA_IS_SKU(m_hwInterface->GetSkuTable(), FtrEncodeHEVC10bit) && m_is10BitHevc)
    {
        m_numMbEncEncKrnStates = CODECHAL_HEVC_MBENC_NUM_BXT_SKL;
    }
    else if (!m_noMeKernelForPFrame)
    {
        m_numMbEncEncKrnStates = CODECHAL_HEVC_MBENC_NUM_BXT_SKL;
    }
    else
    {
        m_numMbEncEncKrnStates = CODECHAL_HEVC_MBENC_NUM;
    }

    m_mbEncKernelStates = MOS_NewArray(MHW_KERNEL_STATE, m_numMbEncEncKrnStates);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_mbEncKernelStates);

    m_mbEncKernelBindingTable = (PCODECHAL_ENCODE_BINDING_TABLE_GENERIC)MOS_AllocAndZeroMemory(
        sizeof(GenericBindingTable) * m_numMbEncEncKrnStates);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_mbEncKernelBindingTable);

    auto kernelStatePtr = m_mbEncKernelStates;

    for (uint32_t krnStateIdx = 0; krnStateIdx < m_numMbEncEncKrnStates; krnStateIdx++)
    {
        auto kernelSize = m_combinedKernelSize;
        CODECHAL_KERNEL_HEADER currKrnHeader;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(pfnGetKernelHeaderAndSize(
            m_kernelBinary,
            ENC_MBENC,
            krnStateIdx,
            &currKrnHeader,
            &kernelSize));

        if (kernelSize == 0)  //Ignore. It isn't used on current platform.
        {
            kernelStatePtr++;
            continue;
        }

        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetMbEncKernelParams(
            &kernelStatePtr->KernelParams,
            krnStateIdx));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetMbEncBindingTable(
            &m_mbEncKernelBindingTable[krnStateIdx], krnStateIdx));

        kernelStatePtr->dwCurbeOffset = m_stateHeapInterface->pStateHeapInterface->GetSizeofCmdInterfaceDescriptorData();
        kernelStatePtr->KernelParams.pBinary = m_kernelBinary + (currKrnHeader.KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);
        kernelStatePtr->KernelParams.iSize = kernelSize;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnCalculateSshAndBtSizesRequested(
            m_stateHeapInterface,
            kernelStatePtr->KernelParams.iBTCount,
            &kernelStatePtr->dwSshSize,
            &kernelStatePtr->dwBindingTableSize));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->MhwInitISH(m_stateHeapInterface, kernelStatePtr));

        kernelStatePtr++;
    }

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG9::InitSurfaceInfoTable()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    m_surfaceParams = (PCODECHAL_SURFACE_CODEC_PARAMS)MOS_AllocAndZeroMemory(
        sizeof(*m_surfaceParams) * SURFACE_NUM_TOTAL);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_surfaceParams);

    PCODECHAL_SURFACE_CODEC_PARAMS param = &m_surfaceParams[SURFACE_RAW_Y];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
        param,
        m_rawSurfaceToEnc,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value,
        0,
        m_verticalLineStride,
        false));

    param = &m_surfaceParams[SURFACE_RAW_10bit_Y];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
        param,
        m_rawSurfaceToEnc,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value,
        0,
        m_verticalLineStride,
        false));

    // MB stats surface -- currently not used
    param = &m_surfaceParams[SURFACE_RAW_MBSTAT];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        param,
        &m_resMbStatisticsSurface.sResource,
        m_resMbStatisticsSurface.dwSize,
        0,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_HME_DOWNSAMPLED_ENCODE].Value,
        0,
        true));
    param->bRawSurface = true;

    param = &m_surfaceParams[SURFACE_RAW_FC_8bit_Y_UV];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
        param,
        &m_formatConvertedSurface[0],
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value,
        0,
        m_verticalLineStride,
        true));  //this should be writable as it is output of formatconversion
    param->bUseUVPlane = true;

    param = &m_surfaceParams[SURFACE_RAW_Y_UV];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
           param,
           m_rawSurfaceToEnc,
           m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value,
           0,
           m_verticalLineStride,
           false));
    param->bUseUVPlane    = true;

    param = &m_surfaceParams[SURFACE_RAW_10bit_Y_UV];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
            param,
            m_rawSurfaceToEnc,
            m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value,
            0,
            m_verticalLineStride,
            false));//this should be writable as it is output of formatconversion
    param->bUseUVPlane = true;

    param = &m_surfaceParams[SURFACE_Y_2X];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
        param,
        &m_scaled2xSurface,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_HME_DOWNSAMPLED_ENCODE].Value,
        0,
        m_verticalLineStride,
        false));

    param = &m_surfaceParams[SURFACE_32x32_PU_OUTPUT];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        param,
        &m_32x32PuOutputData.sResource,
        m_32x32PuOutputData.dwSize,
        0,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value,
        0,
        false));

    param = &m_surfaceParams[SURFACE_SLICE_MAP];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
        param,
        &m_sliceMapSurface,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_HME_DOWNSAMPLED_ENCODE].Value,
        0,
        m_verticalLineStride,
        false));

    param = &m_surfaceParams[SURFACE_Y_2X_VME];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParamsVME(
        param,
        &m_scaled2xSurface,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value,
        0));

    param = &m_surfaceParams[SURFACE_BRC_INPUT];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        param,
        (MOS_RESOURCE*)m_allocator->GetResource(m_standard, brcInputForEncKernel),
        m_allocator->GetResourceSize(m_standard, brcInputForEncKernel),
        0,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value,
        0,
        false));

    param = &m_surfaceParams[SURFACE_LCU_QP];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
        param,
        &m_brcBuffers.sBrcMbQpBuffer,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_HME_DOWNSAMPLED_ENCODE].Value,
        0,
        m_verticalLineStride,
        false));

    param = &m_surfaceParams[SURFACE_ROI];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
        param,
        &m_roiSurface,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_HME_DOWNSAMPLED_ENCODE].Value,
        0,
        m_verticalLineStride,
        false));

    param = &m_surfaceParams[SURFACE_BRC_DATA];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
        param,
        &m_brcBuffers.sBrcConstantDataBuffer[m_currRecycledBufIdx],
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_HME_DOWNSAMPLED_ENCODE].Value,
        0,
        m_verticalLineStride,
        false));

    param = &m_surfaceParams[SURFACE_SIMPLIFIED_INTRA];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
        param,
        &m_simplestIntraSurface,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_HME_DOWNSAMPLED_ENCODE].Value,
        0,
        m_verticalLineStride,
        false));

    param = &m_surfaceParams[SURFACE_HME_MVP];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
        param,
        m_hmeKernel->GetSurface(CodechalKernelHme::SurfaceId::me4xMvDataBuffer),
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_HME_DOWNSAMPLED_ENCODE].Value,
        0,
        m_verticalLineStride,
        false));

    param = &m_surfaceParams[SURFACE_HME_DIST];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
        param,
        m_hmeKernel->GetSurface(CodechalKernelHme::SurfaceId::me4xDistortionBuffer),
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ME_DISTORTION_ENCODE].Value,
        0,
        m_verticalLineStride,
        false));

    param = &m_surfaceParams[SURFACE_16x16PU_SAD];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        param,
        &m_sad16x16Pu.sResource,
        m_sad16x16Pu.dwSize,
        0,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value,
        0,
        false));

    param = &m_surfaceParams[SURFACE_RAW_VME];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParamsVME(
        param,
        m_rawSurfaceToEnc,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value,
        0));

    param = &m_surfaceParams[SURFACE_VME_8x8];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        param,
        &m_vme8x8Mode.sResource,
        m_vme8x8Mode.dwSize,
        0,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value,
        0,
        false));

    param = &m_surfaceParams[SURFACE_CU_RECORD];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        param,
        &m_resMbCodeSurface,
        m_mbCodeSize - m_mvOffset,
        m_mvOffset,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value,
        0,
        true));

    param = &m_surfaceParams[SURFACE_INTRA_MODE];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        param,
        &m_intraMode.sResource,
        m_intraMode.dwSize,
        0,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value,
        0,
        false));

    param = &m_surfaceParams[SURFACE_HCP_PAK];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        param,
        &m_resMbCodeSurface,
        m_mvOffset,
        0,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value,
        0,
        true));

    param = &m_surfaceParams[SURFACE_INTRA_DIST];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        param,
        &m_intraDist.sResource,
        m_intraDist.dwSize,
        0,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value,
        0,
        false));

    param = &m_surfaceParams[SURFACE_MIN_DIST];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
        param,
        &m_minDistortion,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ME_DISTORTION_ENCODE].Value,
        0,
        m_verticalLineStride,
        false));

    param = &m_surfaceParams[SURFACE_VME_UNI_SIC_DATA];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        param,
        &m_vmeSavedUniSic.sResource,
        m_vmeSavedUniSic.dwSize,
        0,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value,
        0,
        false));

    param = &m_surfaceParams[SURFACE_COL_MB_MV];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        param,
        nullptr,
        m_sizeOfMvTemporalBuffer,
        0,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value,
        0,
        false));

    m_concurrentThreadIndex = 0;
    for (auto i = 0; i < NUM_CONCURRENT_THREAD; i++)
    {
        param = &m_surfaceParams[SURFACE_CONCURRENT_THREAD + i];
        CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
            param,
            &m_concurrentThreadSurface[i],
            m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ME_DISTORTION_ENCODE].Value,
            0,
            m_verticalLineStride,
            false));
    }

    param = &m_surfaceParams[SURFACE_MB_MV_INDEX];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        param,
        &m_mvIndex.sResource,
        m_mvIndex.dwSize,
        0,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value,
        0,
        false));

    param = &m_surfaceParams[SURFACE_MVP_INDEX];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        param,
        &m_mvpIndex.sResource,
        m_mvpIndex.dwSize,
        0,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value,
        0,
        false));

    param = &m_surfaceParams[SURFACE_REF_FRAME_VME];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParamsVME(
        param,
        0,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value,
        0));

    param = &m_surfaceParams[SURFACE_Y_4X];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
        param,
        nullptr,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_HME_DOWNSAMPLED_ENCODE].Value,
        0,
        m_verticalLineStride,
        false));

    param = &m_surfaceParams[SURFACE_Y_4X_VME];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParamsVME(
        param,
        nullptr,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value,
        0));

    param = &m_surfaceParams[SURFACE_BRC_HISTORY];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        param,
        &m_brcBuffers.resBrcHistoryBuffer,
        m_brcHistoryBufferSize,
        0,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value,
        0,
        true));

    param = &m_surfaceParams[SURFACE_BRC_ME_DIST];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
        param,
        &m_brcBuffers.sMeBrcDistortionBuffer,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_HME_DOWNSAMPLED_ENCODE].Value,
        0,
        m_verticalLineStride,
        true));

    param = &m_surfaceParams[SURFACE_BRC_PAST_PAK_INFO];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        param,
        &m_brcBuffers.resBrcPakStatisticBuffer[0],
        m_hevcBrcPakStatisticsSize,
        0,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value,
        0,
        false));

    param = &m_surfaceParams[SURFACE_BRC_HCP_PIC_STATE];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        param,
        &m_brcBuffers.resBrcImageStatesWriteBuffer[0],
        m_brcBuffers.dwBrcHcpPicStateSize,
        0,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value,
        0,
        false));

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG9::RequestSshAndVerifyCommandBufferSize(PMHW_KERNEL_STATE kernelState)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(kernelState);

    auto maxBtCount = m_singleTaskPhaseSupported ?
        m_maxBtCount : kernelState->KernelParams.iBTCount;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnRequestSshSpaceForCmdBuf(
        m_stateHeapInterface,
        maxBtCount));

    m_vmeStatesSize = m_hwInterface->GetKernelLoadCommandSize(maxBtCount);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(VerifySpaceAvailable());

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG9::SendKernelCmdsAndBindingTable(
    PMOS_COMMAND_BUFFER                     cmdBuffer,
    PMHW_KERNEL_STATE                       kernelState,
    CODECHAL_MEDIA_STATE_TYPE               mediaStateType,
    PMHW_VFE_SCOREBOARD                     customScoreBoard)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(GetCommandBuffer(cmdBuffer));

    MHW_INTERFACE_DESCRIPTOR_PARAMS idParams;
    MOS_ZeroMemory(&idParams, sizeof(idParams));
    idParams.pKernelState = kernelState;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSetInterfaceDescriptor(
        m_stateHeapInterface,
        1,
        &idParams));

    // Program render engine pipe commands
    SendKernelCmdsParams sendKernelCmdsParams;
    sendKernelCmdsParams = SendKernelCmdsParams();
    sendKernelCmdsParams.EncFunctionType = mediaStateType;
    sendKernelCmdsParams.pKernelState = kernelState;
    sendKernelCmdsParams.bEnableCustomScoreBoard = customScoreBoard ? true : false;
    sendKernelCmdsParams.pCustomScoreBoard = customScoreBoard;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendGenericKernelCmds(cmdBuffer, &sendKernelCmdsParams));

    // Add binding table
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSetBindingTable(
        m_stateHeapInterface,
        kernelState));

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG9::EndKernelCall(
    CODECHAL_MEDIA_STATE_TYPE       mediaStateType,
    PMHW_KERNEL_STATE               kernelState,
    PMOS_COMMAND_BUFFER             cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_UNUSED(kernelState);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(EndStatusReport(cmdBuffer, mediaStateType));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSubmitBlocks(
        m_stateHeapInterface,
        kernelState));
    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnUpdateGlobalCmdBufId(
            m_stateHeapInterface));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(cmdBuffer, nullptr));
    }

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            mediaStateType,
            MHW_SSH_TYPE,
            kernelState));
        CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
            cmdBuffer,
            mediaStateType,
            nullptr)));

    )

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->UpdateSSEuForCmdBuffer(cmdBuffer, m_singleTaskPhaseSupported, m_lastTaskInPhase));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(ReturnCommandBuffer(cmdBuffer));

    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        m_osInterface->pfnSubmitCommandBuffer(m_osInterface, cmdBuffer, m_renderContextUsesNullHw);
        m_lastTaskInPhase = false;
    }

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG9::AddCurbeToStateHeap(
    PMHW_KERNEL_STATE               kernelState,
    CODECHAL_MEDIA_STATE_TYPE       mediaStateType,
    void*                           curbe,
    uint32_t                        curbeSize)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(kernelState);
    MOS_UNUSED(mediaStateType);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(kernelState->m_dshRegion.AddData(
        curbe,
        kernelState->dwCurbeOffset,
        curbeSize));

    CODECHAL_DEBUG_TOOL(

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            mediaStateType,
            MHW_DSH_TYPE,
            kernelState));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCurbe(
        mediaStateType,
        kernelState));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
        mediaStateType,
        MHW_ISH_TYPE,
        kernelState));
    )

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG9::SetSurfacesState(
    PMHW_KERNEL_STATE kernelState,
    PMOS_COMMAND_BUFFER cmdBuffer,
    SURFACE_ID surfaceId,
    uint32_t* bindingTableOffset,
    void* addr,
    uint32_t width,
    uint32_t height)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(bindingTableOffset);
    CODECHAL_ENCODE_CHK_NULL_RETURN(kernelState);

    auto surfaceCodecParams = &m_surfaceParams[surfaceId];
    surfaceCodecParams->dwBindingTableOffset = bindingTableOffset[0];

    if (addr)
    {
        if (surfaceCodecParams->bIs2DSurface || surfaceCodecParams->bUseAdvState)
        {
            surfaceCodecParams->psSurface = (PMOS_SURFACE)addr;
        }
        else
        {
            surfaceCodecParams->presBuffer = (PMOS_RESOURCE)addr;
        }
    }

    // Some surface states do not always use fixed graphic memory address
    switch (surfaceId)
    {
        case SURFACE_HME_MVP:
            surfaceCodecParams->psSurface = m_hmeKernel->GetSurface(CodechalKernelHme::SurfaceId::me4xMvDataBuffer);
            break;

        case SURFACE_HME_DIST:
            surfaceCodecParams->psSurface = m_hmeKernel->GetSurface(CodechalKernelHme::SurfaceId::me4xDistortionBuffer);
            break;

        case SURFACE_BRC_DATA:
            surfaceCodecParams->psSurface = &m_brcBuffers.sBrcConstantDataBuffer[m_currRecycledBufIdx];
            break;

        case SURFACE_CU_RECORD:
        case SURFACE_HCP_PAK:
            surfaceCodecParams->presBuffer = &m_resMbCodeSurface;
            break;

        case SURFACE_RAW_Y:
        case SURFACE_RAW_Y_UV:
        case SURFACE_RAW_VME:
            if (m_hevcSeqParams->bit_depth_luma_minus8)  // use format converted surface if input is 10 bit
                surfaceCodecParams->psSurface = &m_formatConvertedSurface[0];
            else
                surfaceCodecParams->psSurface = m_rawSurfaceToEnc;
            break;

        default:
            break;
    }

    if (surfaceCodecParams->bIs2DSurface && surfaceCodecParams->bUseUVPlane)
    {
        surfaceCodecParams->dwUVBindingTableOffset = bindingTableOffset[1];
    }

    surfaceCodecParams->dwWidthInUse  = width;
    surfaceCodecParams->dwHeightInUse = height;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        surfaceCodecParams,
        kernelState));

    if (surfaceId != SURFACE_KERNEL_DEBUG &&
        surfaceId != SURFACE_HCP_PAK      &&
        surfaceId != SURFACE_CU_RECORD    &&
        surfaceId != SURFACE_BRC_HISTORY  &&
        surfaceId != SURFACE_BRC_ME_DIST)
    {
        if (surfaceCodecParams->bIsWritable)
        {
            surfaceCodecParams->bIsWritable = false; // reset to the default value
        }

        if (surfaceCodecParams->bRenderTarget)
        {
            surfaceCodecParams->bRenderTarget = false; // reset to the default value
        }

        if (surfaceCodecParams->bUse16UnormSurfaceFormat)
        {
            surfaceCodecParams->bUse16UnormSurfaceFormat = false; // reset to the default value
        }
    }

    return eStatus;
}

uint32_t CodechalEncHevcStateG9::PicCodingTypeToFrameType(uint32_t picType)
{
    if (picType == I_TYPE)
    {
        return HEVC_BRC_FRAME_TYPE_I;
    }
    else if (picType == B_TYPE)
    {
        return (m_lowDelay) ? HEVC_BRC_FRAME_TYPE_P_OR_LB : HEVC_BRC_FRAME_TYPE_B;
    }
    else if (picType == B1_TYPE)
    {
        return HEVC_BRC_FRAME_TYPE_B1;
    }
    else if (picType == B2_TYPE)
    {
        return HEVC_BRC_FRAME_TYPE_B2;
    }
    else if (picType == P_TYPE && (!m_noMeKernelForPFrame))
    {
        m_lowDelay = true;
        return HEVC_BRC_FRAME_TYPE_P_OR_LB;
    }
    else
    {
        CODECHAL_ENCODE_ASSERT(false);
        return 0;
    }
}

/*
sliceType: 0 (Intra), 1 (Inter P), 2 (inter B).
intraSADTransform: 0-Regular, 1-Reserved, 2-HAAR, 3-HADAMARD
*/
void CodechalEncHevcStateG9::CalcLambda(uint8_t sliceType, uint8_t intraSADTransform)
{
    if (sliceType != CODECHAL_ENCODE_HEVC_I_SLICE)
    {
        MOS_SecureMemcpy(&m_qpLambdaMd[sliceType], sizeof(m_qpLambdaMd[sliceType]),
            &m_qpLambdaMdLut[sliceType], sizeof(m_qpLambdaMdLut[sliceType]));

        MOS_SecureMemcpy(&m_qpLambdaMe[sliceType], sizeof(m_qpLambdaMe[sliceType]),
            &m_qpLambdaMeLut[sliceType], sizeof(m_qpLambdaMeLut[sliceType]));
    }
    else
    {
        for (uint32_t qp = 0; qp < QP_NUM; qp++)
        {
            double qpTemp = (double)qp - 12;
            double lambdaMd = 0.85 * pow(2.0, qpTemp/3.0);

            if ((intraSADTransform != INTRA_TRANSFORM_HAAR) && (intraSADTransform != INTRA_TRANSFORM_HADAMARD))
            {
                lambdaMd *= 0.95;
            }

            m_qpLambdaMd[sliceType][qp] =
            m_qpLambdaMe[sliceType][qp] = sqrt(lambdaMd);
        }
    }
}

MOS_STATUS CodechalEncHevcStateG9::EncodeBrcInitResetKernel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    PerfTagSetting    perfTag;
    CODECHAL_ENCODE_SET_PERFTAG_INFO(perfTag, CODECHAL_ENCODE_PERFTAG_CALL_BRC_INIT_RESET);

    uint32_t krnIdx = m_brcInit ? CODECHAL_HEVC_BRC_INIT : CODECHAL_HEVC_BRC_RESET;

    auto kernelState  = &m_brcKernelStates[krnIdx];
    auto bindingTable = &m_brcKernelBindingTable[krnIdx];
    if (m_firstTaskInPhase || !m_singleTaskPhaseSupported)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(RequestSshAndVerifyCommandBufferSize(kernelState));
    }

    //Setup DSH
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->AssignDshAndSshSpace(
        m_stateHeapInterface,
        kernelState,
        false,
        0,
        false,
        m_storeData));

    //Setup CURBE
    CODECHAL_ENC_HEVC_BRC_INITRESET_CURBE_G9 cmd, *curbe = &cmd;
    MOS_SecureMemcpy(curbe, sizeof(cmd), m_brcInitCurbeInit, sizeof(m_brcInitCurbeInit));

    curbe->DW0.Value = GetProfileLevelMaxFrameSize();

    if (m_hevcSeqParams->RateControlMethod == RATECONTROL_CBR ||
        m_hevcSeqParams->RateControlMethod == RATECONTROL_VBR ||
        m_hevcSeqParams->RateControlMethod == RATECONTROL_AVBR)
    {
        if (m_hevcSeqParams->InitVBVBufferFullnessInBit == 0)
        {
            CODECHAL_ENCODE_ASSERT(false);
        }

        if (m_hevcSeqParams->VBVBufferSizeInBit == 0)
        {
            CODECHAL_ENCODE_ASSERT(false);
        }
    }

    curbe->DW1.InitBufFull           = m_hevcSeqParams->InitVBVBufferFullnessInBit;
    curbe->DW2.BufSize               = m_hevcSeqParams->VBVBufferSizeInBit;
    curbe->DW3.TargetBitRate         = m_hevcSeqParams->TargetBitRate * CODECHAL_ENCODE_BRC_KBPS;
    curbe->DW4.MaximumBitRate        = m_hevcSeqParams->MaxBitRate * CODECHAL_ENCODE_BRC_KBPS;
    curbe->DW9.FrameWidth            = m_oriFrameWidth;
    curbe->DW10.FrameHeight          = m_oriFrameHeight;
    curbe->DW12.NumberSlice          = m_numSlices;

    curbe->DW6.FrameRateM            = m_hevcSeqParams->FrameRate.Numerator;
    curbe->DW7.FrameRateD            = m_hevcSeqParams->FrameRate.Denominator;
    curbe->DW8.BRCFlag               = 0;
    curbe->DW8.BRCFlag |= (m_lcuBrcEnabled) ? 0 : CODECHAL_ENCODE_BRCINIT_DISABLE_MBBRC;
    // For non-ICQ, ACQP Buffer always set to 1
    curbe->DW25.ACQPBuffer           = 1;

    if (m_hevcSeqParams->RateControlMethod == RATECONTROL_CBR)
    {
        curbe->DW4.MaximumBitRate   = curbe->DW3.TargetBitRate;
        curbe->DW8.BRCFlag          |= curbe->DW8.BRCFlag | BRCINIT_ISCBR;
    }
    else if (m_hevcSeqParams->RateControlMethod == RATECONTROL_VBR)
    {
        if (curbe->DW4.MaximumBitRate < curbe->DW3.TargetBitRate)
        {
            curbe->DW4.MaximumBitRate = 2 * curbe->DW3.TargetBitRate;
        }
        curbe->DW8.BRCFlag          |= curbe->DW8.BRCFlag | BRCINIT_ISVBR;
    }
    else if (m_hevcSeqParams->RateControlMethod == RATECONTROL_AVBR)
    {
        curbe->DW8.BRCFlag          |= curbe->DW8.BRCFlag | BRCINIT_ISAVBR;
        // For AVBR, max bitrate = target bitrate,
        curbe->DW3.TargetBitRate  = m_hevcSeqParams->TargetBitRate * CODECHAL_ENCODE_BRC_KBPS;
        curbe->DW4.MaximumBitRate = m_hevcSeqParams->TargetBitRate * CODECHAL_ENCODE_BRC_KBPS;
    }
    else if (m_hevcSeqParams->RateControlMethod == RATECONTROL_ICQ)
    {
        curbe->DW8.BRCFlag           |= curbe->DW8.BRCFlag | BRCINIT_ISICQ;
        curbe->DW25.ACQPBuffer = m_hevcSeqParams->ICQQualityFactor;
    }
    else if (m_hevcSeqParams->RateControlMethod == RATECONTROL_VCM)
    {
        curbe->DW4.MaximumBitRate    = curbe->DW3.TargetBitRate;
        curbe->DW8.BRCFlag           |= curbe->DW8.BRCFlag | BRCINIT_ISVCM;
    }

    /**********************************************************************
    In case of non-HB/BPyramid Structure
    BRC_Param_A = GopP
    BRC_Param_B = GopB
    In case of HB/BPyramid GOP Structure
    BRC_Param_A, BRC_Param_B, BRC_Param_C, BRC_Param_D are
    BRC Parameters set as follows as per CModel equation
    ***********************************************************************/
    // BPyramid GOP
    if (m_hevcSeqParams->NumOfBInGop[1] != 0 || m_hevcSeqParams->NumOfBInGop[2] != 0)
    {
        curbe->DW8.BRC_Param_A   = ((m_hevcSeqParams->GopPicSize) / m_hevcSeqParams->GopRefDist);
        curbe->DW9.BRC_Param_B   = curbe->DW8.BRC_Param_A;
        curbe->DW13.BRC_Param_C  = curbe->DW8.BRC_Param_A * 2;
        curbe->DW14.BRC_Param_D  = ((m_hevcSeqParams->GopPicSize) - (curbe->DW8.BRC_Param_A) - (curbe->DW13.BRC_Param_C) - (curbe->DW9.BRC_Param_B));
        // B1 Level GOP
        if (m_hevcSeqParams->NumOfBInGop[2] == 0)
        {
            curbe->DW14.MaxBRCLevel = 3;
        }
        // B2 Level GOP
        else
        {
            curbe->DW14.MaxBRCLevel = 4;
        }
    }
    // For Regular GOP - No BPyramid
    else
    {
        curbe->DW14.MaxBRCLevel = 1;
        curbe->DW8.BRC_Param_A =
            (m_hevcSeqParams->GopRefDist) ? ((m_hevcSeqParams->GopPicSize - 1) / m_hevcSeqParams->GopRefDist) : 0;
        curbe->DW9.BRC_Param_B = m_hevcSeqParams->GopPicSize - 1 - curbe->DW8.BRC_Param_A;
    }

    curbe->DW10.AVBRAccuracy    = m_usAvbrAccuracy;
    curbe->DW11.AVBRConvergence = m_usAvbrConvergence;

    // Set dynamic thresholds
    double inputBitsPerFrame =
        ((double)(curbe->DW4.MaximumBitRate) * (double)(curbe->DW7.FrameRateD) /
        (double)(curbe->DW6.FrameRateM));

    if (curbe->DW2.BufSize < (uint32_t)inputBitsPerFrame * 4)
    {
        curbe->DW2.BufSize = (uint32_t)inputBitsPerFrame * 4;
    }

    if (curbe->DW1.InitBufFull == 0)
    {
        curbe->DW1.InitBufFull = 7 * curbe->DW2.BufSize/8;
    }
    if (curbe->DW1.InitBufFull < (uint32_t)(inputBitsPerFrame*2))
    {
        curbe->DW1.InitBufFull = (uint32_t)(inputBitsPerFrame*2);
    }
    if (curbe->DW1.InitBufFull > curbe->DW2.BufSize)
    {
        curbe->DW1.InitBufFull = curbe->DW2.BufSize;
    }

    if (m_hevcSeqParams->RateControlMethod == RATECONTROL_AVBR)
    {
        // For AVBR, Buffer size =  2*Bitrate, InitVBV = 0.75 * bufferSize
        curbe->DW2.BufSize     = 2 * m_hevcSeqParams->TargetBitRate * CODECHAL_ENCODE_BRC_KBPS;
        curbe->DW1.InitBufFull = (uint32_t)(0.75 * curbe->DW2.BufSize);
    }

    double bpsRatio = inputBitsPerFrame / ((double)(curbe->DW2.BufSize)/30);
    bpsRatio = (bpsRatio < 0.1) ? 0.1 : (bpsRatio > 3.5) ? 3.5 : bpsRatio;

    curbe->DW19.DeviationThreshold0_PBframe      = (uint32_t) (-50 * pow(0.90, bpsRatio));
    curbe->DW19.DeviationThreshold1_PBframe      = (uint32_t) (-50 * pow(0.66, bpsRatio));
    curbe->DW19.DeviationThreshold2_PBframe      = (uint32_t) (-50 * pow(0.46, bpsRatio));
    curbe->DW19.DeviationThreshold3_PBframe      = (uint32_t) (-50 * pow(0.3, bpsRatio));

    curbe->DW20.DeviationThreshold4_PBframe      = (uint32_t) (50 * pow(0.3, bpsRatio));
    curbe->DW20.DeviationThreshold5_PBframe      = (uint32_t) (50 * pow(0.46, bpsRatio));
    curbe->DW20.DeviationThreshold6_PBframe      = (uint32_t) (50 * pow(0.7, bpsRatio));
    curbe->DW20.DeviationThreshold7_PBframe      = (uint32_t) (50 * pow(0.9, bpsRatio));

    curbe->DW21.DeviationThreshold0_VBRcontrol   = (uint32_t) (-50 * pow(0.9, bpsRatio));
    curbe->DW21.DeviationThreshold1_VBRcontrol   = (uint32_t) (-50 * pow(0.7, bpsRatio));
    curbe->DW21.DeviationThreshold2_VBRcontrol   = (uint32_t) (-50 * pow(0.5, bpsRatio));
    curbe->DW21.DeviationThreshold3_VBRcontrol   = (uint32_t) (-50 * pow(0.3, bpsRatio));

    curbe->DW22.DeviationThreshold4_VBRcontrol   = (uint32_t) (100 * pow(0.4, bpsRatio));
    curbe->DW22.DeviationThreshold5_VBRcontrol   = (uint32_t) (100 * pow(0.5, bpsRatio));
    curbe->DW22.DeviationThreshold6_VBRcontrol   = (uint32_t) (100 * pow(0.75, bpsRatio));
    curbe->DW22.DeviationThreshold7_VBRcontrol   = (uint32_t) (100 * pow(0.9, bpsRatio));

    curbe->DW23.DeviationThreshold0_Iframe       = (uint32_t) (-50 * pow(0.8, bpsRatio));
    curbe->DW23.DeviationThreshold1_Iframe       = (uint32_t) (-50 * pow(0.6, bpsRatio));
    curbe->DW23.DeviationThreshold2_Iframe       = (uint32_t) (-50 * pow(0.34, bpsRatio));
    curbe->DW23.DeviationThreshold3_Iframe       = (uint32_t) (-50 * pow(0.2, bpsRatio));

    curbe->DW24.DeviationThreshold4_Iframe       = (uint32_t) (50 * pow(0.2, bpsRatio));
    curbe->DW24.DeviationThreshold5_Iframe       = (uint32_t) (50 * pow(0.4, bpsRatio));
    curbe->DW24.DeviationThreshold6_Iframe       = (uint32_t) (50 * pow(0.66, bpsRatio));
    curbe->DW24.DeviationThreshold7_Iframe       = (uint32_t) (50 * pow(0.9, bpsRatio));

    if (m_brcInit)
    {
        m_dBrcInitCurrentTargetBufFullInBits = curbe->DW1.InitBufFull;
    }

    m_brcInitResetBufSizeInBits      = curbe->DW2.BufSize;
    m_dBrcInitResetInputBitsPerFrame = inputBitsPerFrame;

    CODECHAL_MEDIA_STATE_TYPE encFunctionType = CODECHAL_MEDIA_STATE_BRC_INIT_RESET;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(AddCurbeToStateHeap(kernelState, encFunctionType, &cmd, sizeof(cmd)));

//#if (_DEBUG || _RELEASE_INTERNAL)
//    if (m_swBrcMode != nullptr)
//    {
//        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_DbgCallHevcSwBrcImpl(
//            m_debugInterface,
//            encFunctionType,
//            this,
//            bBrcReset,
//            kernelState,
//            kernelState));
//
//        return eStatus;
//    }
//#endif // (_DEBUG || _RELEASE_INTERNAL)

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendKernelCmdsAndBindingTable(
        &cmdBuffer,
        kernelState,
        encFunctionType,
        nullptr));

    //Add surface states
    uint32_t startIndex = 0;
    // BRC history buffer
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_BRC_HISTORY,
        &bindingTable->dwBindingTableEntries[startIndex++],
        &m_brcBuffers.resBrcHistoryBuffer));

    // Distortion data surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_BRC_ME_DIST,
        &bindingTable->dwBindingTableEntries[startIndex++],
        &m_brcBuffers.sMeBrcDistortionBuffer));

    CODECHAL_ENCODE_ASSERT(startIndex == bindingTable->dwNumBindingTableEntries);

    MHW_MEDIA_OBJECT_PARAMS    mediaObjectParams;
    MOS_ZeroMemory(&mediaObjectParams, sizeof(mediaObjectParams));

    MediaObjectInlineData mediaObjectInlineData;
    MOS_ZeroMemory(&mediaObjectInlineData, sizeof(mediaObjectInlineData));

    mediaObjectParams.pInlineData = &mediaObjectInlineData;
    mediaObjectParams.dwInlineDataSize = sizeof(mediaObjectInlineData);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_renderEngineInterface->AddMediaObject(
        &cmdBuffer,
        nullptr,
        &mediaObjectParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(EndKernelCall(
        encFunctionType,
        kernelState,
        &cmdBuffer));

    // debug dump
    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &m_brcBuffers.resBrcHistoryBuffer,
            CodechalDbgAttr::attrOutput,
            "HistoryWrite",
            m_brcHistoryBufferSize,
            0,
            CODECHAL_MEDIA_STATE_BRC_INIT_RESET));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpSurface(
            &m_brcBuffers.sMeBrcDistortionBuffer,
            CodechalDbgAttr::attrOutput,
            "BrcDist",
            CODECHAL_MEDIA_STATE_BRC_INIT_RESET)););

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG9::EncodeCoarseIntra16x16Kernel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    PerfTagSetting perfTag;
    CODECHAL_ENCODE_SET_PERFTAG_INFO(perfTag, CODECHAL_ENCODE_PERFTAG_CALL_INTRA_DIST);

    uint32_t krnIdx = CODECHAL_HEVC_BRC_COARSE_INTRA;

    auto kernelState  = &m_brcKernelStates[krnIdx];
    auto bindingTable = &m_brcKernelBindingTable[krnIdx];
    if (m_firstTaskInPhase || !m_singleTaskPhaseSupported)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(RequestSshAndVerifyCommandBufferSize(kernelState));
    }

    //Setup DSH
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->AssignDshAndSshSpace(
        m_stateHeapInterface,
        kernelState,
        false,
        0,
        false,
        m_storeData));

    //Setup CURBE
    CODECHAL_ENC_HEVC_COARSE_INTRA_CURBE_G9 cmd, *curbe = &cmd;
    MOS_ZeroMemory(curbe, sizeof(*curbe));

    // the width and height is the resolution of 4x down-scaled surface
    curbe->DW0.PictureWidthInLumaSamples   = m_downscaledWidthInMb4x  << 4;
    curbe->DW0.PictureHeightInLumaSamples  = m_downscaledHeightInMb4x << 4;

    curbe->DW1.InterSAD                    = 2;
    curbe->DW1.IntraSAD                    = 2;

    uint32_t startBTI = 0;
    curbe->DW8.BTI_Src_Y4                  = bindingTable->dwBindingTableEntries[startBTI++];
    curbe->DW9.BTI_Intra_Dist              = bindingTable->dwBindingTableEntries[startBTI++];
    curbe->DW10.BTI_VME_Intra              = bindingTable->dwBindingTableEntries[startBTI++];

    CODECHAL_ENCODE_ASSERT(startBTI == bindingTable->dwNumBindingTableEntries);

    CODECHAL_MEDIA_STATE_TYPE encFunctionType = CODECHAL_MEDIA_STATE_ENC_I_FRAME_DIST;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(
        AddCurbeToStateHeap(kernelState, encFunctionType, &cmd, sizeof(cmd))
    );

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendKernelCmdsAndBindingTable(
        &cmdBuffer,
        kernelState,
        encFunctionType,
        nullptr));

    //Add surface states
    startBTI = 0;
    //0: Source Y4
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_Y_4X,
        &bindingTable->dwBindingTableEntries[startBTI++],
        m_trackedBuf->Get4xDsSurface(CODEC_CURR_TRACKED_BUFFER)));

    //1: Intra distortion surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_BRC_ME_DIST,
        &bindingTable->dwBindingTableEntries[startBTI++],
        &m_brcBuffers.sBrcIntraDistortionBuffer));

    //2: Source Y4 for VME
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_Y_4X_VME,
        &bindingTable->dwBindingTableEntries[startBTI++],
        m_trackedBuf->Get4xDsSurface(CODEC_CURR_TRACKED_BUFFER)));

    CODECHAL_ENCODE_ASSERT(startBTI == bindingTable->dwNumBindingTableEntries);

    if (!m_hwWalker)
    {
        eStatus = MOS_STATUS_UNKNOWN;
        CODECHAL_ENCODE_ASSERTMESSAGE("Currently HW walker shall not be disabled for CM based down scaling kernel.");
        return eStatus;
    }

    CODECHAL_WALKER_CODEC_PARAMS walkerCodecParams;
    MOS_ZeroMemory(&walkerCodecParams, sizeof(walkerCodecParams));
    walkerCodecParams.WalkerMode        = m_walkerMode;
    walkerCodecParams.dwResolutionX     = m_downscaledWidthInMb4x;
    walkerCodecParams.dwResolutionY     = m_downscaledHeightInMb4x;
    walkerCodecParams.bNoDependency     = true;

    MHW_WALKER_PARAMS walkerParams;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalInitMediaObjectWalkerParams(
        m_hwInterface,
        &walkerParams,
        &walkerCodecParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_renderEngineInterface->AddMediaObjectWalkerCmd(
        &cmdBuffer,
        &walkerParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(EndKernelCall(
        encFunctionType,
        kernelState,
        &cmdBuffer));

    return eStatus;
}

uint32_t* CodechalEncHevcStateG9::GetDefaultCurbeEncBKernel(uint32_t& curbeSize)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (m_hevcSeqParams->TargetUsage == 0x07)
    {
        if(m_pictureCodingType == I_TYPE)
        {
            // When TU=7, there is no normal I kernel calls.
            // Instead, B kernel is used for I kernel function and a specfic CURBE setting needs to be used
            curbeSize = sizeof(m_encBTu7ICurbeInit);
            return (uint32_t*)m_encBTu7ICurbeInit;
        }
        else if (m_pictureCodingType == P_TYPE)
        {
            curbeSize = sizeof(m_encBTu7PCurbeInit);
            return (uint32_t*)m_encBTu7PCurbeInit;
        }
        else
        {
            curbeSize = sizeof(m_encBTu7BCurbeInit);
            return (uint32_t*)m_encBTu7BCurbeInit;
        }
    }
    else if (m_hevcSeqParams->TargetUsage == 0x04)
    {
        if (m_pictureCodingType == P_TYPE)
        {
            curbeSize = sizeof(m_encBTu4PCurbeInit);
            return (uint32_t*)m_encBTu4PCurbeInit;
        }
        else
        {
            curbeSize = sizeof(m_encBTu4BCurbeInit);
            return (uint32_t*)m_encBTu4BCurbeInit;
        }
    }
    else if (m_hevcSeqParams->TargetUsage == 0x01)
    {
        if (m_pictureCodingType == P_TYPE)
        {
            curbeSize = sizeof(m_encBTu1PCurbeInit);
            return (uint32_t*)m_encBTu1PCurbeInit;
        }
        else
        {
            curbeSize = sizeof(m_encBTu1BCurbeInit);
            return (uint32_t*)m_encBTu1BCurbeInit;
        }
    }
    else
    {
        CODECHAL_ENCODE_ASSERT(false);
    }

    return nullptr;
}

MOS_STATUS CodechalEncHevcStateG9::SetupROISurface()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    m_hevcPicParams->NumROI = MOS_MIN(m_hevcPicParams->NumROI, CODECHAL_ENCODE_HEVC_MAX_NUM_ROI);

    // Following code for configuring the ROI surface has been lifted from the CModel and
    // ported to work in the context of the driver instead.

    CODECHAL_ENC_HEVC_ROI_G9 currentROI[CODECHAL_ENCODE_HEVC_MAX_NUM_ROI] = { 0 };
    for (uint32_t i = 0; i < m_hevcPicParams->NumROI; ++i)
    {
        currentROI[i].Top    = m_hevcPicParams->ROI[i].Top;
        currentROI[i].Bottom = m_hevcPicParams->ROI[i].Bottom;
        currentROI[i].Left   = m_hevcPicParams->ROI[i].Left;
        currentROI[i].Right  = m_hevcPicParams->ROI[i].Right;
        if (m_brcEnabled && !m_roiValueInDeltaQp)
        {
            currentROI[i].ROI_Level = m_hevcPicParams->ROI[i].PriorityLevelOrDQp * 5;
        }
        else
        {
            currentROI[i].QPDelta = m_hevcPicParams->ROI[i].PriorityLevelOrDQp;
        }
    }

    MOS_LOCK_PARAMS lockParams;
    MOS_ZeroMemory(&lockParams, sizeof(lockParams));
    lockParams.ReadOnly = 1;
    uint32_t* data = (uint32_t*)m_osInterface->pfnLockResource(m_osInterface, &m_roiSurface.OsResource, &lockParams);
    if (!data)
    {
        eStatus = MOS_STATUS_INVALID_HANDLE;
        return eStatus;
    }

    uint32_t widthInMBsAligned = (m_picWidthInMb * 4 + 63) & ~63;
    uint32_t numMBs = m_picWidthInMb * m_picHeightInMb;
    for (uint32_t mb = 0 ; mb <= numMBs ; mb++)
    {
        int32_t curMbY = mb / m_picWidthInMb;
        int32_t curMbX = mb - curMbY * m_picWidthInMb;

        uint32_t outdata = 0;
        for (int32_t roi = (m_hevcPicParams->NumROI - 1); roi >= 0; roi--)
        {
            if ((currentROI[roi].ROI_Level == 0) && (currentROI[roi].QPDelta == 0))
            {
                continue;
            }

            if ((curMbX >= (int32_t)currentROI[roi].Left) && (curMbX < (int32_t)currentROI[roi].Right) &&
                (curMbY >= (int32_t)currentROI[roi].Top) && (curMbY < (int32_t)currentROI[roi].Bottom))
            {
                outdata = 15 | (((currentROI[roi].ROI_Level) & 0xFF) << 8) | ((currentROI[roi].QPDelta & 0xFF) << 16);
            }
            else if ((curMbX >= (int32_t)currentROI[roi].Left - 1) && (curMbX < (int32_t)currentROI[roi].Right + 1) &&
                (curMbY >= (int32_t)currentROI[roi].Top - 1) && (curMbY < (int32_t)currentROI[roi].Bottom + 1))
            {
                outdata = 14 | (((currentROI[roi].ROI_Level) & 0xFF) << 8) | ((currentROI[roi].QPDelta & 0xFF) << 16);
            }
            else if ((curMbX >= (int32_t)currentROI[roi].Left - 2) && (curMbX < (int32_t)currentROI[roi].Right + 2) &&
                (curMbY >= (int32_t)currentROI[roi].Top - 2) && (curMbY < (int32_t)currentROI[roi].Bottom + 2))
            {
                outdata = 13 | (((currentROI[roi].ROI_Level) & 0xFF) << 8) | ((currentROI[roi].QPDelta & 0xFF) << 16);
            }
            else if ((curMbX >= (int32_t)currentROI[roi].Left - 3) && (curMbX < (int32_t)currentROI[roi].Right + 3) &&
                (curMbY >= (int32_t)currentROI[roi].Top - 3) && (curMbY < (int32_t)currentROI[roi].Bottom + 3))
            {
                outdata = 12 | (((currentROI[roi].ROI_Level) & 0xFF) << 8) | ((currentROI[roi].QPDelta & 0xFF) << 16);
            }
        }
        data[(curMbY * (widthInMBsAligned>>2)) + curMbX] = outdata;
    }

    m_osInterface->pfnUnlockResource(m_osInterface, &m_roiSurface.OsResource);

    CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpSurface(
        &m_roiSurface,
        CodechalDbgAttr::attrInput,
        "BrcUpdate_ROI",
        CODECHAL_MEDIA_STATE_BRC_UPDATE)));

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG9::SetupBrcConstantTable(PMOS_SURFACE brcConstantData)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_LOCK_PARAMS lockFlags;
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.WriteOnly = true;
    uint8_t*  data = (uint8_t* )m_osInterface->pfnLockResource(m_osInterface, &brcConstantData->OsResource, &lockFlags);
    CODECHAL_ENCODE_CHK_NULL_RETURN(data);

    uint32_t size = brcConstantData->dwHeight * brcConstantData->dwWidth;
    // 576-byte of Qp adjust table
    MOS_SecureMemcpy(data, size, g_cInit_HEVC_BRC_QP_ADJUST, sizeof(g_cInit_HEVC_BRC_QP_ADJUST));
    data += sizeof(g_cInit_HEVC_BRC_QP_ADJUST);
    size -= sizeof(g_cInit_HEVC_BRC_QP_ADJUST);

    const uint32_t sizeSkipValTable = HEVC_BRC_SKIP_VAL_TABLE_SIZE;
    const uint32_t sizelambdaTable = HEVC_BRC_LAMBDA_TABLE_SIZE;

    // Skip thread table
    if(m_pictureCodingType == I_TYPE)
    {
        MOS_ZeroMemory(data, sizeSkipValTable);
    }
    else
    {
        uint32_t curbeSize = 0;
        PCODECHAL_ENC_HEVC_B_MB_ENC_CURBE_G9 curbe = (PCODECHAL_ENC_HEVC_B_MB_ENC_CURBE_G9)GetDefaultCurbeEncBKernel(curbeSize);

        if(curbe == nullptr)
        {
            eStatus = MOS_STATUS_NULL_POINTER;
            return eStatus;
        }

        if(curbe->DW3.BlockBasedSkipEnable)
        {
            MOS_SecureMemcpy(data, size, m_skipThread[1], sizeof(m_skipThread[1]));
        }
        else
        {
            MOS_SecureMemcpy(data, size, m_skipThread[0], sizeof(m_skipThread[0]));
        }
    }
    data += sizeSkipValTable;
    size -= sizeSkipValTable;

    //lambda value table
    MOS_SecureMemcpy(data, size, m_brcLambdaHaar, sizeof(m_brcLambdaHaar));
    data += sizelambdaTable;
    size -= sizelambdaTable;

    //Mv mode cost table
    if(m_pictureCodingType == I_TYPE)
    {
        MOS_SecureMemcpy(data, size, m_brcMvCostHaar[0], sizeof(m_brcMvCostHaar[0]));
    }
    else if (m_pictureCodingType == P_TYPE)
    {
        MOS_SecureMemcpy(data, size, m_brcMvCostHaar[1], sizeof(m_brcMvCostHaar[1]));
    }
    else
    {
        MOS_SecureMemcpy(data, size, m_brcMvCostHaar[2], sizeof(m_brcMvCostHaar[2]));
    }

    m_osInterface->pfnUnlockResource(m_osInterface, &brcConstantData->OsResource);

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG9::Convert1byteTo2bytesQPperLCU(PMOS_SURFACE lcuQPIn, PMOS_SURFACE lcuQPOut)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_LOCK_PARAMS lockFlagsIn;
    MOS_LOCK_PARAMS lockFlagsOut;
    MOS_ZeroMemory(&lockFlagsIn,  sizeof(MOS_LOCK_PARAMS));
    MOS_ZeroMemory(&lockFlagsOut, sizeof(MOS_LOCK_PARAMS));

    lockFlagsIn.ReadOnly = true;
    uint8_t*  dataIn = (uint8_t* )m_osInterface->pfnLockResource(m_osInterface, &lcuQPIn->OsResource, &lockFlagsIn);
    CODECHAL_ENCODE_CHK_NULL_RETURN(dataIn);

    lockFlagsOut.WriteOnly = true;
    uint8_t*  dataOut = (uint8_t* )m_osInterface->pfnLockResource(m_osInterface, &lcuQPOut->OsResource, &lockFlagsOut);
    CODECHAL_ENCODE_CHK_NULL_RETURN(dataOut);

    for(uint32_t h = 0; h < lcuQPIn->dwHeight; h++)
    {
        for(uint32_t w = 0; w < lcuQPIn->dwWidth; w++)
        {
            *(dataOut + h * lcuQPOut->dwPitch + 2 * w)     = *(dataIn + h * lcuQPIn->dwPitch + w);
            *(dataOut + h * lcuQPOut->dwPitch + 2 * w + 1) = 0;
        }
    }

    m_osInterface->pfnUnlockResource(m_osInterface, &lcuQPIn->OsResource);
    m_osInterface->pfnUnlockResource(m_osInterface, &lcuQPOut->OsResource);

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG9::SetupROICurbe(PCODECHAL_ENC_HEVC_BRC_UPDATE_CURBE_G9 curbe)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    curbe->DW6.CQPValue = 0;
    curbe->DW6.ROIFlag  = 0x1 | (m_brcEnabled << 1) | (m_hevcSeqParams->bVideoSurveillance << 2);

    uint32_t roiSize = 0;
    for (uint32_t i = 0; i < m_hevcPicParams->NumROI; ++i)
    {
        roiSize += (CODECHAL_MACROBLOCK_HEIGHT * MOS_ABS(m_hevcPicParams->ROI[i].Top - m_hevcPicParams->ROI[i].Bottom)) *
                   (CODECHAL_MACROBLOCK_WIDTH * MOS_ABS(m_hevcPicParams->ROI[i].Right - m_hevcPicParams->ROI[i].Left));
    }

    uint32_t roiRatio = 0;
    if (roiSize)
    {
        uint32_t numMBs = m_picWidthInMb * m_picHeightInMb;
        roiRatio = 2 * (numMBs * 256 / roiSize - 1);
        roiRatio = MOS_MIN(51, roiRatio); // clip QP from 0-51
    }

    curbe->DW6.ROIRatio        = roiRatio;
    curbe->DW7.FrameWidthInLCU = MOS_ALIGN_CEIL(m_frameWidth, 32) >> 5;

    // if the BRC update LCU kernel is being launched in CQP mode we need to add
    // the minimum required parameters it needs to run.  This is used in ROI CQP.
    // In the case of BRC the CURBE will already be set up from frame update setup.
    if (!m_brcEnabled)
    {
        curbe->DW1.FrameNumber     = m_storeData - 1;
        curbe->DW6.CQPValue        = CalSliceQp();
        curbe->DW5.CurrFrameType   = PicCodingTypeToFrameType(m_pictureCodingType);
    }

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG9::EncodeBrcUpdateLCUBasedKernel(PCODECHAL_ENC_HEVC_BRC_UPDATE_CURBE_G9 frameBasedBrcCurbe)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    PerfTagSetting perfTag;
    CODECHAL_ENCODE_SET_PERFTAG_INFO(perfTag, CODECHAL_ENCODE_PERFTAG_CALL_BRC_UPDATE_LCU);

    uint32_t krnIdx = CODECHAL_HEVC_BRC_LCU_UPDATE;
    auto     kernelState  = &m_brcKernelStates[krnIdx];
    auto     bindingTable = &m_brcKernelBindingTable[krnIdx];

    if (m_firstTaskInPhase || !m_singleTaskPhaseSupported)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(RequestSshAndVerifyCommandBufferSize(kernelState));
    }

    // Setup DSH
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->AssignDshAndSshSpace(
        m_stateHeapInterface,
        kernelState,
        false,
        0,
        false,
        m_storeData));

    // Setup Curbe
    CODECHAL_ENC_HEVC_BRC_UPDATE_CURBE_G9 cmd, *curbe = &cmd;
    if (m_brcEnabled)
    {
        MOS_SecureMemcpy(curbe, sizeof(cmd), frameBasedBrcCurbe, sizeof(*frameBasedBrcCurbe));
    }
    else
    {
        //confiure LCU BRC Update CURBE for CQP (used in ROI) here
        MOS_SecureMemcpy(curbe, sizeof(cmd), m_brcUpdateCurbeInit, sizeof(m_brcUpdateCurbeInit));
    }

    if (m_hevcPicParams->NumROI)
    {
        SetupROICurbe(&cmd);
        SetupROISurface();
    }

    CODECHAL_MEDIA_STATE_TYPE encFunctionType = CODECHAL_MEDIA_STATE_HEVC_BRC_LCU_UPDATE;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(
        AddCurbeToStateHeap(kernelState, encFunctionType, &cmd, sizeof(cmd)));

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendKernelCmdsAndBindingTable(
        &cmdBuffer,
        kernelState,
        encFunctionType,
        nullptr));

    //Add surface states
    uint32_t startIndex = 0;

    //0: BRC history buffer
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_BRC_HISTORY,
        &bindingTable->dwBindingTableEntries[startIndex++],
        &m_brcBuffers.resBrcHistoryBuffer));

    //1: BRC distortion data surface : when picture type is I-type, both inter and intra distortion are the same
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_BRC_ME_DIST,
        &bindingTable->dwBindingTableEntries[startIndex++],
        m_brcDistortion));

    //2: Intra distortion data surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_BRC_ME_DIST,
        &bindingTable->dwBindingTableEntries[startIndex++],
        &m_brcBuffers.sBrcIntraDistortionBuffer));

    if(m_hmeSupported)
    {
    //3: HME MV surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_HME_MVP,
        &bindingTable->dwBindingTableEntries[startIndex++]));
    }
    else
    {
        startIndex++;
    }

    //4: LCU Qp surface
    m_surfaceParams[SURFACE_LCU_QP].bIsWritable =
    m_surfaceParams[SURFACE_LCU_QP].bRenderTarget = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_LCU_QP,
        &bindingTable->dwBindingTableEntries[startIndex++],
        &m_brcBuffers.sBrcMbQpBuffer));

    //5:  ROI Surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_ROI,
        &bindingTable->dwBindingTableEntries[startIndex++],
        &m_roiSurface));

    CODECHAL_ENCODE_ASSERT(startIndex == bindingTable->dwNumBindingTableEntries);

    if (!m_hwWalker)
    {
        eStatus = MOS_STATUS_UNKNOWN;
        CODECHAL_ENCODE_ASSERTMESSAGE("Currently HW walker shall not be disabled for CM based down scaling kernel.");
        return eStatus;
    }

    // LCU-based kernel needs to be executed in 4x4 LCU mode (128x128 per block)
    CODECHAL_WALKER_CODEC_PARAMS walkerCodecParams;
    MOS_ZeroMemory(&walkerCodecParams, sizeof(walkerCodecParams));
    walkerCodecParams.WalkerMode            = m_walkerMode;
    walkerCodecParams.dwResolutionX         = MOS_ALIGN_CEIL(m_frameWidth, 128) >> 7;
    walkerCodecParams.dwResolutionY         = MOS_ALIGN_CEIL(m_frameHeight, 128) >> 7;
    /* Enforce no dependency dispatch order for LCU-based BRC update kernel  */
    walkerCodecParams.bNoDependency         = true;
    walkerCodecParams.wPictureCodingType    = m_pictureCodingType;
    walkerCodecParams.bUseScoreboard        = m_useHwScoreboard;

    MHW_WALKER_PARAMS walkerParams;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalInitMediaObjectWalkerParams(
        m_hwInterface,
        &walkerParams,
        &walkerCodecParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_renderEngineInterface->AddMediaObjectWalkerCmd(
        &cmdBuffer,
        &walkerParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(EndKernelCall(
        encFunctionType,
        kernelState,
        &cmdBuffer));

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG9::EncodeBrcUpdateKernel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    PerfTagSetting    perfTag;
    CODECHAL_ENCODE_SET_PERFTAG_INFO(perfTag, CODECHAL_ENCODE_PERFTAG_CALL_BRC_UPDATE);

    uint32_t krnIdx = CODECHAL_HEVC_BRC_FRAME_UPDATE;
    auto     kernelState  = &m_brcKernelStates[krnIdx];
    auto     bindingTable = &m_brcKernelBindingTable[krnIdx];
    if (m_firstTaskInPhase || !m_singleTaskPhaseSupported)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(RequestSshAndVerifyCommandBufferSize(kernelState));
    }

    // Fill HCP_IMG_STATE so that BRC kernel can use it to generate the write buffer for PAK
    auto                     brcHcpStateReadBuffer = &m_brcBuffers.resBrcImageStatesReadBuffer[m_currRecycledBufIdx];
    MHW_VDBOX_HEVC_PIC_STATE mhwHevcPicState;
    mhwHevcPicState.pHevcEncSeqParams = m_hevcSeqParams;
    mhwHevcPicState.pHevcEncPicParams = m_hevcPicParams;
    mhwHevcPicState.brcNumPakPasses = m_mfxInterface->GetBrcNumPakPasses();

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpHevcPicBrcBuffer(brcHcpStateReadBuffer, &mhwHevcPicState));

    auto brcConstantData = &m_brcBuffers.sBrcConstantDataBuffer[m_currRecycledBufIdx];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetupBrcConstantTable(brcConstantData));

    // debug dump
    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &m_brcBuffers.resBrcImageStatesReadBuffer[m_currRecycledBufIdx],
            CodechalDbgAttr::attrInput,
            "ImgStateRead",
            BRC_IMG_STATE_SIZE_PER_PASS * m_hwInterface->GetMfxInterface()->GetBrcNumPakPasses(),
            0,
            CODECHAL_MEDIA_STATE_BRC_UPDATE));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpSurface(
            &m_brcBuffers.sBrcConstantDataBuffer[m_currRecycledBufIdx],
            CodechalDbgAttr::attrInput,
            "ConstData",
            CODECHAL_MEDIA_STATE_BRC_UPDATE));

        // PAK statistics buffer is only dumped for BrcUpdate kernel input
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &m_brcBuffers.resBrcPakStatisticBuffer[m_brcBuffers.uiCurrBrcPakStasIdxForRead],
            CodechalDbgAttr::attrInput,
            "PakStats",
            HEVC_BRC_PAK_STATISTCS_SIZE,
            0,
            CODECHAL_MEDIA_STATE_BRC_UPDATE));
        // HEVC maintains a ptr to its own distortion surface, as it may be a couple different surfaces
        if (m_brcDistortion) {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(
                m_debugInterface->DumpBuffer(
                    &m_brcDistortion->OsResource,
                    CodechalDbgAttr::attrInput,
                    "BrcDist",
                    m_brcBuffers.sMeBrcDistortionBuffer.dwPitch * m_brcBuffers.sMeBrcDistortionBuffer.dwHeight,
                    m_brcBuffers.dwMeBrcDistortionBottomFieldOffset,
                    CODECHAL_MEDIA_STATE_BRC_UPDATE));
        } CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(&m_brcBuffers.resBrcHistoryBuffer,
            CodechalDbgAttr::attrInput,
            "HistoryRead",
            m_brcHistoryBufferSize,
            0,
            CODECHAL_MEDIA_STATE_BRC_UPDATE));
        if (m_brcBuffers.pMbEncKernelStateInUse) {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCurbe(
                CODECHAL_MEDIA_STATE_BRC_UPDATE,
                m_brcBuffers.pMbEncKernelStateInUse));
        } CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(&m_resMbStatsBuffer,
            CodechalDbgAttr::attrInput,
            "MBStatsSurf",
            m_hwInterface->m_avcMbStatBufferSize,
            0,
            CODECHAL_MEDIA_STATE_BRC_UPDATE));)
    // Setup DSH
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->AssignDshAndSshSpace(
        m_stateHeapInterface,
        kernelState,
        false,
        0,
        false,
        m_storeData));

    // Setup Curbe
    CODECHAL_ENC_HEVC_BRC_UPDATE_CURBE_G9 cmd, *curbe = &cmd;
    MOS_SecureMemcpy(curbe, sizeof(cmd), m_brcUpdateCurbeInit, sizeof(m_brcUpdateCurbeInit));

    curbe->DW5.TARGETSIZE_FLAG = 0;

    if (m_dBrcInitCurrentTargetBufFullInBits > (double)m_brcInitResetBufSizeInBits)
    {
        m_dBrcInitCurrentTargetBufFullInBits -= (double)m_brcInitResetBufSizeInBits;
        curbe->DW5.TARGETSIZE_FLAG = 1;
    }

    if (m_numSkipFrames)
    {
        // pass num/size of skipped frames to update BRC
        curbe->DW6.NumSkippedFrames      = m_numSkipFrames;
        curbe->DW15.SizeOfSkippedFrames  = m_sizeSkipFrames;

        // account for skipped frame in calculating CurrentTargetBufFullInBits
        m_dBrcInitCurrentTargetBufFullInBits += m_dBrcInitResetInputBitsPerFrame * m_numSkipFrames;
    }

    curbe->DW0.TARGETSIZE        = (uint32_t)(m_dBrcInitCurrentTargetBufFullInBits);
    curbe->DW1.FrameNumber       = m_storeData - 1;

    curbe->DW2.PictureHeaderSize = GetPicHdrSize();

    curbe->DW5.CurrFrameType     = PicCodingTypeToFrameType(m_pictureCodingType);

    // Only brc init uses BRCFlag, brc update does NOT use it (it's reserved bits)
    curbe->DW5.BRCFlag = 0;

    // Notes from BRC Kernel
    /***********************************************************************************************************
    * When update kernel Curbe GRF 1.7 bit 15 is set to 1:
    * BRC matched with Arch CModel SVN revision 13030 with part of HRD BRC fix in svn 14029 and svn 14228 [HRD]
    *
    * When update kernel Curbe GRF 1.7 bit 15 is set to 0:
    * BRC matched with Arch CModel SVN revision 13419 [HRD Fix] with svn 13833, svn 13827 [Quality] and
    * part of BRC fix in svn 14029, svn 14228, svn 13845 [HRD]
    ************************************************************************************************************/
    curbe->DW7.KernelBuildControl = 0;

    curbe->DW7.ucMinQp = m_hevcPicParams->BRCMinQp;
    curbe->DW7.ucMaxQp = m_hevcPicParams->BRCMaxQp;

    if (m_hevcPicParams->NumROI)
    {
        SetupROICurbe(&cmd);
    }
    curbe->DW14.ParallelMode = m_hevcSeqParams->ParallelBRC;

    curbe->DW5.MaxNumPAKs = m_mfxInterface->GetBrcNumPakPasses();

    m_dBrcInitCurrentTargetBufFullInBits += m_dBrcInitResetInputBitsPerFrame;

    if (m_hevcSeqParams->RateControlMethod == RATECONTROL_AVBR)
    {
        curbe->DW3.startGAdjFrame0 = (uint32_t)((10 * m_usAvbrConvergence) / (double)150);
        curbe->DW3.startGAdjFrame1 = (uint32_t)((50 * m_usAvbrConvergence) / (double)150);
        curbe->DW4.startGAdjFrame2 = (uint32_t)((100 * m_usAvbrConvergence) / (double)150);
        curbe->DW4.startGAdjFrame3 = (uint32_t)((150 * m_usAvbrConvergence) / (double)150);

        curbe->DW11.gRateRatioThreshold0 =
            (uint32_t)((100 - (m_usAvbrAccuracy / (double)30) * (100 - 40)));
        curbe->DW11.gRateRatioThreshold1 =
            (uint32_t)((100 - (m_usAvbrAccuracy / (double)30) * (100 - 75)));
        curbe->DW12.gRateRatioThreshold2 = (uint32_t)((100 - (m_usAvbrAccuracy / (double)30) * (100 - 97)));
        curbe->DW12.gRateRatioThreshold3 = (uint32_t)((100 + (m_usAvbrAccuracy / (double)30) * (103 - 100)));
        curbe->DW12.gRateRatioThreshold4 = (uint32_t)((100 + (m_usAvbrAccuracy / (double)30) * (125 - 100)));
        curbe->DW12.gRateRatioThreshold5 = (uint32_t)((100 + (m_usAvbrAccuracy / (double)30) * (160 - 100)));
    }
    else
    {
        // default CURBE setting is zero. So, driver needs to program them.
        curbe->DW3.startGAdjFrame0 = 10;
        curbe->DW3.startGAdjFrame1 = 50;
        curbe->DW4.startGAdjFrame2 = 100;
        curbe->DW4.startGAdjFrame3 = 150;

        curbe->DW11.gRateRatioThreshold0 = 40;
        curbe->DW11.gRateRatioThreshold1 = 75;
        curbe->DW12.gRateRatioThreshold2 = 97;
        curbe->DW12.gRateRatioThreshold3 = 103;
        curbe->DW12.gRateRatioThreshold4 = 125;
        curbe->DW12.gRateRatioThreshold5 = 160;
    }

    CODECHAL_MEDIA_STATE_TYPE encFunctionType = CODECHAL_MEDIA_STATE_BRC_UPDATE;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(
        AddCurbeToStateHeap(kernelState, encFunctionType, &cmd, sizeof(cmd)));

//#if (_DEBUG || _RELEASE_INTERNAL)
//    if (m_swBrcMode != nullptr)
//    {
//        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_DbgCallHevcSwBrcImpl(
//            m_debugInterface,
//            encFunctionType,
//            this,
//            false,
//            kernelState,
//            kernelState));
//
//        if (bLcuBrcEnabled || pHevcPicParams->NumROI)
//        {
//            // LCU-based BRC needs to have frame-based one to be call first in order to get HCP_IMG_STATE command result
//            CODECHAL_ENCODE_CHK_STATUS_RETURN(EncodeBrcUpdateLCUBasedKernel(curbe));
//        }
//        return eStatus;
//    }
//#endif // (_DEBUG || _RELEASE_INTERNAL)

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendKernelCmdsAndBindingTable(&cmdBuffer,
        kernelState,
        encFunctionType,
        nullptr));

    if (!m_singleTaskPhaseSupported || m_firstTaskInPhase || !m_singleTaskPhaseSupportedInPak)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CheckBrcPakStasBuffer(&cmdBuffer));
    }

    //Add surface states
    uint32_t startIndex = 0;
    //0: BRC history buffer
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_BRC_HISTORY,
        &bindingTable->dwBindingTableEntries[startIndex++],
        &m_brcBuffers.resBrcHistoryBuffer));

    //1: Previous PAK statistics output buffer
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_BRC_PAST_PAK_INFO,
        &bindingTable->dwBindingTableEntries[startIndex++],
        &m_brcBuffers.resBrcPakStatisticBuffer[m_brcBuffers.uiCurrBrcPakStasIdxForRead]));

    //2: HCP_PIC_STATE buffer for read
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_BRC_HCP_PIC_STATE,
        &bindingTable->dwBindingTableEntries[startIndex++],
        brcHcpStateReadBuffer));

    //3: HCP_PIC_STATE buffer for write
    m_surfaceParams[SURFACE_BRC_HCP_PIC_STATE].bIsWritable =
    m_surfaceParams[SURFACE_BRC_HCP_PIC_STATE].bRenderTarget = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_BRC_HCP_PIC_STATE,
        &bindingTable->dwBindingTableEntries[startIndex++],
        &m_brcBuffers.resBrcImageStatesWriteBuffer[m_currRecycledBufIdx]));

    //4: BRC input surface for ENC kernels (output of BRC kernel)
    m_surfaceParams[SURFACE_BRC_INPUT].bIsWritable =
    m_surfaceParams[SURFACE_BRC_INPUT].bRenderTarget = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_BRC_INPUT,
        &bindingTable->dwBindingTableEntries[startIndex++]));

    //5: Distortion data surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_BRC_ME_DIST,
        &bindingTable->dwBindingTableEntries[startIndex++],
        m_brcDistortion));

    //6: BRC data surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_BRC_DATA,
        &bindingTable->dwBindingTableEntries[startIndex++]));

    CODECHAL_ENCODE_ASSERT(startIndex == bindingTable->dwNumBindingTableEntries);

    MHW_MEDIA_OBJECT_PARAMS    mediaObjectParams;
    MOS_ZeroMemory(&mediaObjectParams, sizeof(mediaObjectParams));
    MediaObjectInlineData mediaObjectInlineData;
    MOS_ZeroMemory(&mediaObjectInlineData, sizeof(mediaObjectInlineData));
    mediaObjectParams.pInlineData = &mediaObjectInlineData;
    mediaObjectParams.dwInlineDataSize = sizeof(mediaObjectInlineData);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_renderEngineInterface->AddMediaObject(
        &cmdBuffer,
        nullptr,
        &mediaObjectParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(EndKernelCall(
        encFunctionType,
        kernelState,
        &cmdBuffer));

    if (m_lcuBrcEnabled || m_hevcPicParams->NumROI)
    {
        // LCU-based BRC needs to have frame-based one to be call first in order to get HCP_IMG_STATE command result
        CODECHAL_ENCODE_CHK_STATUS_RETURN(EncodeBrcUpdateLCUBasedKernel(curbe));
    }

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &m_brcBuffers.resBrcImageStatesWriteBuffer[m_currRecycledBufIdx],
            CodechalDbgAttr::attrOutput,
            "ImgStateWrite",
            BRC_IMG_STATE_SIZE_PER_PASS * m_hwInterface->GetMfxInterface()->GetBrcNumPakPasses(),
            0,
            CODECHAL_MEDIA_STATE_BRC_UPDATE));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &m_brcBuffers.resBrcHistoryBuffer,
            CodechalDbgAttr::attrOutput,
            "HistoryWrite",
            m_brcHistoryBufferSize,
            0,
            CODECHAL_MEDIA_STATE_BRC_UPDATE));
        if (!Mos_ResourceIsNull(&m_brcBuffers.sBrcMbQpBuffer.OsResource)) {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                &m_brcBuffers.sBrcMbQpBuffer.OsResource,
                CodechalDbgAttr::attrOutput,
                "MbQp",
                m_brcBuffers.sBrcMbQpBuffer.dwPitch * m_brcBuffers.sBrcMbQpBuffer.dwHeight,
                m_brcBuffers.dwBrcMbQpBottomFieldOffset,
                CODECHAL_MEDIA_STATE_BRC_UPDATE));
        } if (m_brcBuffers.pMbEncKernelStateInUse) {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCurbe(
                CODECHAL_MEDIA_STATE_BRC_UPDATE,
                m_brcBuffers.pMbEncKernelStateInUse));
        } if (m_mbencBrcBufferSize > 0) {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                &m_brcBuffers.resMbEncBrcBuffer,
                CodechalDbgAttr::attrOutput,
                "MbEncBRCWrite",
                m_mbencBrcBufferSize,
                0,
                CODECHAL_MEDIA_STATE_BRC_UPDATE));
        }

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            (MOS_RESOURCE *)m_allocator->GetResource(m_standard, brcInputForEncKernel),
            CodechalDbgAttr::attrOutput,
            "CombinedEnc",
            128,
            0,
            CODECHAL_MEDIA_STATE_BRC_UPDATE));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &m_brcBuffers.sBrcIntraDistortionBuffer.OsResource,
            CodechalDbgAttr::attrOutput,
            "IDistortion",
            m_brcBuffers.sBrcIntraDistortionBuffer.dwWidth * m_brcBuffers.sBrcIntraDistortionBuffer.dwHeight,
            0,
            CODECHAL_MEDIA_STATE_BRC_UPDATE));)

    //reset info of skip frame
    m_numSkipFrames  = 0;
    m_sizeSkipFrames = 0;
    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG9::Encode8x8BPakKernel(
    PCODECHAL_ENC_HEVC_B_MB_ENC_CURBE_G9 encBCurbe)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(encBCurbe);

    PerfTagSetting perfTag;
    CODECHAL_ENCODE_SET_PERFTAG_INFO(perfTag, CODECHAL_ENCODE_PERFTAG_CALL_PAK_KERNEL);

    uint32_t krnIdx = CODECHAL_HEVC_MBENC_BPAK;
    auto     kernelState  = &m_mbEncKernelStates[krnIdx];
    auto     bindingTable = &m_mbEncKernelBindingTable[krnIdx];
    if (m_firstTaskInPhase || !m_singleTaskPhaseSupported)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(RequestSshAndVerifyCommandBufferSize(kernelState));
    }

    //Setup DSH
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->AssignDshAndSshSpace(
        m_stateHeapInterface,
        kernelState,
        false,
        0,
        false,
        m_storeData));

    //Setup CURBE
    CODECHAL_ENC_HEVC_B_PAK_CURBE_G9  cmd, *curbe = &cmd;
    MOS_ZeroMemory(curbe, sizeof(*curbe));
    curbe->DW0.FrameWidth = MOS_ALIGN_CEIL(m_frameWidth, CODECHAL_MACROBLOCK_WIDTH);
    curbe->DW0.FrameHeight = MOS_ALIGN_CEIL(m_frameHeight, CODECHAL_MACROBLOCK_HEIGHT);

    curbe->DW1.MaxVmvR                 = encBCurbe->DW44.MaxVmvR;
    curbe->DW1.Qp                      = encBCurbe->DW13.QpPrimeY;
    curbe->DW2.BrcEnable               = encBCurbe->DW36.BRCEnable;
    curbe->DW2.LcuBrcEnable            = encBCurbe->DW36.LCUBRCEnable;
    curbe->DW2.ScreenContent           = encBCurbe->DW47.ScreenContentFlag;
    curbe->DW2.SimplestIntraEnable     = encBCurbe->DW47.SkipIntraKrnFlag;
    curbe->DW2.SliceType               = encBCurbe->DW4.SliceType;
    curbe->DW2.ROIEnable               = (m_hevcPicParams->NumROI > 0);
    curbe->DW2.FASTSurveillanceFlag    = (m_hevcPicParams->CodingType == I_TYPE) ? 0 : m_hevcSeqParams->bVideoSurveillance;
    // KBLControlFlag determines the PAK OBJ format as it varies from Gen9 to Gen9.5+
    curbe->DW2.KBLControlFlag          = UsePlatformControlFlag();
    curbe->DW2.EnableRollingIntra      = m_hevcPicParams->bEnableRollingIntraRefresh;
    curbe->DW3.IntraRefreshQPDelta     = m_hevcPicParams->QpDeltaForInsertedIntra;
    curbe->DW3.IntraRefreshMBNum       = m_hevcPicParams->IntraInsertionLocation;
    curbe->DW3.IntraRefreshUnitInMB    = m_hevcPicParams->IntraInsertionSize;

    uint32_t startBTI = 0;
    curbe->DW16.BTI_CU_Record          = bindingTable->dwBindingTableEntries[startBTI++];
    curbe->DW17.BTI_PAK_Obj            = bindingTable->dwBindingTableEntries[startBTI++];
    curbe->DW18.BTI_Slice_Map          = bindingTable->dwBindingTableEntries[startBTI++];
    curbe->DW19.BTI_Brc_Input          = bindingTable->dwBindingTableEntries[startBTI++];
    curbe->DW20.BTI_LCU_Qp             = bindingTable->dwBindingTableEntries[startBTI++];
    curbe->DW21.BTI_Brc_Data           = bindingTable->dwBindingTableEntries[startBTI++];
    curbe->DW22.BTI_MB_Data            = bindingTable->dwBindingTableEntries[startBTI++];
    curbe->DW23.BTI_MVP_Surface        = bindingTable->dwBindingTableEntries[startBTI++];
    curbe->DW24.BTI_WA_PAK_Data        = bindingTable->dwBindingTableEntries[startBTI++];
    curbe->DW25.BTI_WA_PAK_Obj         = bindingTable->dwBindingTableEntries[startBTI++];
    curbe->DW26.BTI_Debug              = bindingTable->dwBindingTableEntries[startBTI++];

    CODECHAL_ENCODE_ASSERT(startBTI == bindingTable->dwNumBindingTableEntries);

    CODECHAL_MEDIA_STATE_TYPE encFunctionType = CODECHAL_MEDIA_STATE_HEVC_B_PAK;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(
        AddCurbeToStateHeap(kernelState, encFunctionType, &cmd, sizeof(cmd)));

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendKernelCmdsAndBindingTable(
        &cmdBuffer,
        kernelState,
        encFunctionType,
        nullptr));

    //Add surface states
    startBTI = 0;
    //0: CU record
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_CU_RECORD,
        &bindingTable->dwBindingTableEntries[startBTI++]));

    //1: PAK command
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_HCP_PAK,
        &bindingTable->dwBindingTableEntries[startBTI++]));

    //2: slice map
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_SLICE_MAP,
        &bindingTable->dwBindingTableEntries[startBTI++]));

    // 3: BRC Input
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_BRC_INPUT,
        &bindingTable->dwBindingTableEntries[startBTI++]));

    // 4: LCU Qp
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_LCU_QP,
        &bindingTable->dwBindingTableEntries[startBTI++]));

    // 5: LCU BRC constant
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_BRC_DATA,
        &bindingTable->dwBindingTableEntries[startBTI++]));

    // 6: MV index buffer or MB data
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_MB_MV_INDEX,
        &bindingTable->dwBindingTableEntries[startBTI++]));

    // 7: MVP index buffer
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_MVP_INDEX,
        &bindingTable->dwBindingTableEntries[startBTI++]));

    if (!m_hwWalker)
    {
        eStatus = MOS_STATUS_UNKNOWN;
        CODECHAL_ENCODE_ASSERTMESSAGE("Currently HW walker shall not be disabled for CM based down scaling kernel.");
        return eStatus;
    }

    CODECHAL_WALKER_CODEC_PARAMS walkerCodecParams;
    MOS_ZeroMemory(&walkerCodecParams, sizeof(walkerCodecParams));
    walkerCodecParams.WalkerMode            = m_walkerMode;
    /* looping for Walker is needed at 8x8 block level */
    walkerCodecParams.dwResolutionX         = MOS_ALIGN_CEIL(m_frameWidth, 32) >> 5;
    walkerCodecParams.dwResolutionY         = MOS_ALIGN_CEIL(m_frameHeight, 32) >> 5;
    /* Enforce no dependency dispatch order for 32x32 B Intra Check kernel  */
    walkerCodecParams.bNoDependency         = true;
    walkerCodecParams.wPictureCodingType    = m_pictureCodingType;
    walkerCodecParams.bUseScoreboard        = m_useHwScoreboard;

    MHW_WALKER_PARAMS walkerParams;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalInitMediaObjectWalkerParams(
        m_hwInterface,
        &walkerParams,
        &walkerCodecParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_renderEngineInterface->AddMediaObjectWalkerCmd(
        &cmdBuffer,
        &walkerParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(EndKernelCall(
        encFunctionType,
        kernelState,
        &cmdBuffer));

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG9::Encode8x8PBMbEncKernel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    PerfTagSetting perfTag;
    CODECHAL_ENCODE_SET_PERFTAG_INFO(perfTag, CODECHAL_ENCODE_PERFTAG_CALL_MBENC_KERNEL);

    uint32_t krnIdx = CODECHAL_HEVC_MBENC_BENC;
    if (m_pictureCodingType == P_TYPE)
    {
        krnIdx = m_hevcPicParams->bEnableRollingIntraRefresh ? CODECHAL_HEVC_MBENC_ADV_P : CODECHAL_HEVC_MBENC_PENC;
    }
    else if (m_pictureCodingType == B_TYPE)
    {
        // In TU7, we still need the original ENC B kernel to process the I frame
        krnIdx = m_hevcPicParams->bEnableRollingIntraRefresh ? CODECHAL_HEVC_MBENC_ADV : CODECHAL_HEVC_MBENC_BENC;
    }

    auto kernelState  = &m_mbEncKernelStates[krnIdx];
    auto bindingTable = &m_mbEncKernelBindingTable[krnIdx];
    if (m_firstTaskInPhase || !m_singleTaskPhaseSupported)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(RequestSshAndVerifyCommandBufferSize(kernelState));
    }

    int32_t sliceQp = CalSliceQp();
    uint8_t sliceType = PicCodingTypeToSliceType(m_pictureCodingType);

    uint8_t tuMode = 0;
    if (m_hevcSeqParams->TargetUsage == 0x07)
    {
        // When TU=7, lambda is not computed in the 32x32 MD stage for it is skipped.
        CalcLambda(sliceType, INTRA_TRANSFORM_HAAR);
        tuMode = CODECHAL_ENCODE_HEVC_TARGET_USAGE_MODE_PERFORMANCE;
    }
    else if (m_hevcSeqParams->TargetUsage == 0x04)
    {
        tuMode = CODECHAL_ENCODE_HEVC_TARGET_USAGE_MODE_NORMAL;
    }
    else if (m_hevcSeqParams->TargetUsage == 0x01)
    {
        tuMode = CODECHAL_ENCODE_HEVC_TARGET_USAGE_MODE_QUALITY;
    }
    else
    {
        CODECHAL_ENCODE_ASSERT(false);
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    LoadCosts(sliceType, (uint8_t)sliceQp, INTRA_TRANSFORM_REGULAR);

    uint8_t mbCodeIdxForTempMVP = 0xFF;
    if(m_pictureCodingType != I_TYPE)
    {
        if (m_hevcPicParams->CollocatedRefPicIndex != 0xFF && m_hevcPicParams->CollocatedRefPicIndex < CODEC_MAX_NUM_REF_FRAME_HEVC)
        {
            uint8_t frameIdx = m_hevcPicParams->RefFrameList[m_hevcPicParams->CollocatedRefPicIndex].FrameIdx;

            mbCodeIdxForTempMVP = m_refList[frameIdx]->ucScalingIdx;
        }

        if (mbCodeIdxForTempMVP == 0xFF && m_hevcSliceParams->slice_temporal_mvp_enable_flag)
        {
            // Temporal reference MV index is invalid and so disable the temporal MVP
            CODECHAL_ENCODE_ASSERT(false);
            m_hevcSliceParams->slice_temporal_mvp_enable_flag = false;
        }
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(GenerateWalkingControlRegion());

    //Setup DSH
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->AssignDshAndSshSpace(
        m_stateHeapInterface,
        kernelState,
        false,
        0,
        false,
        m_storeData));

    //Setup CURBE
    uint8_t maxLenSP[] = { 25, 57, 57 };
    uint8_t forwardTransformThd[7] = { 0 };
    CalcForwardCoeffThd(forwardTransformThd, sliceQp);

    uint32_t curbeSize = 0;
    void* defaultCurbe = (void*)GetDefaultCurbeEncBKernel(curbeSize);
    CODECHAL_ENCODE_ASSERT(defaultCurbe);

    CODECHAL_ENC_HEVC_B_MB_ENC_CURBE_G9 cmd, *curbe = &cmd;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(curbe, sizeof(cmd), defaultCurbe, curbeSize));

    bool transform_8x8_mode_flag = true;

    curbe->DW0.AdaptiveEn  = 1;
    curbe->DW0.T8x8FlagForInterEn = transform_8x8_mode_flag;
    curbe->DW2.PicWidth    = m_picWidthInMb;
    curbe->DW2.LenSP       = maxLenSP[tuMode];
    curbe->DW3.SrcAccess   = curbe->DW3.RefAccess = 0;
    curbe->DW3.FTEnable                           = (m_ftqBasedSkip[m_hevcSeqParams->TargetUsage] >> 1) & 0x01;

    curbe->DW4.PicHeightMinus1               = m_picHeightInMb - 1;
    curbe->DW4.HMEEnable                     = m_hmeEnabled;
    curbe->DW4.SliceType                     = sliceType;
    curbe->DW4.UseActualRefQPValue           = false;

    curbe->DW7.IntraPartMask                 = 0x3;

    curbe->DW6.FrameWidth                    = m_picWidthInMb  * CODECHAL_MACROBLOCK_WIDTH;
    curbe->DW6.FrameHeight                   = m_picHeightInMb * CODECHAL_MACROBLOCK_HEIGHT;

    curbe->DW8.Mode0Cost = m_modeCost[0];
    curbe->DW8.Mode1Cost = m_modeCost[1];
    curbe->DW8.Mode2Cost = m_modeCost[2];
    curbe->DW8.Mode3Cost = m_modeCost[3];

    curbe->DW9.Mode4Cost = m_modeCost[4];
    curbe->DW9.Mode5Cost = m_modeCost[5];
    curbe->DW9.Mode6Cost = m_modeCost[6];
    curbe->DW9.Mode7Cost = m_modeCost[7];

    curbe->DW10.Mode8Cost= m_modeCost[8];
    curbe->DW10.Mode9Cost= m_modeCost[9];
    curbe->DW10.RefIDCost = m_modeCost[10];
    curbe->DW10.ChromaIntraModeCost = m_modeCost[11];

    curbe->DW11.MV0Cost  = m_mvCost[0];
    curbe->DW11.MV1Cost  = m_mvCost[1];
    curbe->DW11.MV2Cost  = m_mvCost[2];
    curbe->DW11.MV3Cost  = m_mvCost[3];

    curbe->DW12.MV4Cost  = m_mvCost[4];
    curbe->DW12.MV5Cost  = m_mvCost[5];
    curbe->DW12.MV6Cost  = m_mvCost[6];
    curbe->DW12.MV7Cost  = m_mvCost[7];

    curbe->DW13.QpPrimeY = sliceQp;
    uint8_t bitDepthChromaMinus8 = 0; // support 4:2:0 only
    int32_t qpBdOffsetC = 6 * bitDepthChromaMinus8;
    int32_t qpi                  = (int32_t)CodecHal_Clip3((-qpBdOffsetC), 51, (sliceQp + m_hevcPicParams->pps_cb_qp_offset));
    int32_t qpc = (qpi < 30) ? qpi : QPcTable[qpi - 30];
    curbe->DW13.QpPrimeCb= qpc + qpBdOffsetC;
    qpi                          = (int32_t)CodecHal_Clip3((-qpBdOffsetC), 51, (sliceQp + m_hevcPicParams->pps_cr_qp_offset));
    qpc = (qpi < 30) ? qpi : QPcTable[qpi - 30];
    curbe->DW13.QpPrimeCr= qpc;

    curbe->DW14.SICFwdTransCoeffThreshold_0 = forwardTransformThd[0];
    curbe->DW14.SICFwdTransCoeffThreshold_1 = forwardTransformThd[1];
    curbe->DW14.SICFwdTransCoeffThreshold_2 = forwardTransformThd[2];

    curbe->DW15.SICFwdTransCoeffThreshold_3 = forwardTransformThd[3];
    curbe->DW15.SICFwdTransCoeffThreshold_4 = forwardTransformThd[4];
    curbe->DW15.SICFwdTransCoeffThreshold_5 = forwardTransformThd[5];
    curbe->DW15.SICFwdTransCoeffThreshold_6 = forwardTransformThd[6];

    curbe->DW32.SkipVal = m_skipValB[curbe->DW3.BlockBasedSkipEnable][transform_8x8_mode_flag][sliceQp];

    if(m_pictureCodingType == I_TYPE)
    {
        *(float*)&(curbe->DW34.LambdaME) = 0.0;
    }
    else if (m_pictureCodingType == P_TYPE)
    {
        *(float*)&(curbe->DW34.LambdaME) = (float)m_qpLambdaMe[CODECHAL_ENCODE_HEVC_P_SLICE][sliceQp];
    }
    else
    {
        *(float*)&(curbe->DW34.LambdaME) = (float)m_qpLambdaMe[CODECHAL_ENCODE_HEVC_B_SLICE][sliceQp];
    }

    curbe->DW35.ModeCostSp                 = m_modeCostSp;
    curbe->DW35.SimpIntraInterThreshold    = m_simplestIntraInterThreshold;

    curbe->DW36.NumRefIdxL0MinusOne  = m_hevcSliceParams->num_ref_idx_l0_active_minus1;
    curbe->DW36.NumRefIdxL1MinusOne  = m_hevcSliceParams->num_ref_idx_l1_active_minus1;
    curbe->DW36.BRCEnable            = m_encodeParams.bMbQpDataEnabled || m_brcEnabled;
    curbe->DW36.LCUBRCEnable         = m_encodeParams.bMbQpDataEnabled || m_lcuBrcEnabled;
    curbe->DW36.PowerSaving         = m_powerSavingEnabled;
    curbe->DW36.ROIEnable            = (m_hevcPicParams->NumROI > 0);
    curbe->DW36.FASTSurveillanceFlag = (m_hevcPicParams->CodingType == I_TYPE) ? 0 : m_hevcSeqParams->bVideoSurveillance;

    if(m_pictureCodingType != I_TYPE)
    {
        curbe->DW37.ActualQpRefID0List0 = GetQPValueFromRefList(LIST_0, CODECHAL_ENCODE_REF_ID_0);
        curbe->DW37.ActualQpRefID1List0 = GetQPValueFromRefList(LIST_0, CODECHAL_ENCODE_REF_ID_1);
        curbe->DW37.ActualQpRefID2List0 = GetQPValueFromRefList(LIST_0, CODECHAL_ENCODE_REF_ID_2);
        curbe->DW37.ActualQpRefID3List0 = GetQPValueFromRefList(LIST_0, CODECHAL_ENCODE_REF_ID_3);
        curbe->DW41.TextureIntraCostThreshold = 500;
        if(m_pictureCodingType == B_TYPE) {
            curbe->DW39.ActualQpRefID0List1 = GetQPValueFromRefList(LIST_1, CODECHAL_ENCODE_REF_ID_0);
            curbe->DW39.ActualQpRefID1List1 = GetQPValueFromRefList(LIST_1, CODECHAL_ENCODE_REF_ID_1);
        }
    }

    curbe->DW44.MaxVmvR                = 511 * 4;
    curbe->DW44.MaxNumMergeCandidates  = m_hevcSliceParams->MaxNumMergeCand;

    if(m_pictureCodingType != I_TYPE)
    {
        curbe->DW44.MaxNumRefList0         = curbe->DW36.NumRefIdxL0MinusOne + 1;

        curbe->DW45.TemporalMvpEnableFlag  = m_hevcSliceParams->slice_temporal_mvp_enable_flag;
        curbe->DW45.HMECombineLenPslice    = 8;
        if(m_pictureCodingType == B_TYPE)
        {
            curbe->DW44.MaxNumRefList1         = curbe->DW36.NumRefIdxL1MinusOne + 1;
            curbe->DW45.HMECombineLenBslice    = 8;
        }
    }

    curbe->DW45.Log2ParallelMergeLevel = m_hevcPicParams->log2_parallel_merge_level_minus2 + 2;

    curbe->DW46.Log2MaxTUSize = m_hevcSeqParams->log2_max_transform_block_size_minus2 + 2;
    curbe->DW46.Log2MinTUSize = m_hevcSeqParams->log2_min_transform_block_size_minus2 + 2;
    curbe->DW46.Log2MaxCUSize = m_hevcSeqParams->log2_max_coding_block_size_minus3 + 3;
    curbe->DW46.Log2MinCUSize = m_hevcSeqParams->log2_min_coding_block_size_minus3 + 3;

    curbe->DW47.NumRegionsInSlice       = m_numRegionsInSlice;
    curbe->DW47.TypeOfWalkingPattern    = m_enable26WalkingPattern;
    curbe->DW47.ChromaFlatnessCheckFlag = (m_hevcSeqParams->TargetUsage == 0x07) ? 0 : 1;
    curbe->DW47.EnableIntraEarlyExit    = (m_hevcSeqParams->TargetUsage == 0x04);
    curbe->DW47.SkipIntraKrnFlag        = (m_hevcSeqParams->TargetUsage == 0x07);  // When TU=7, there is no intra kernel call
    curbe->DW47.CollocatedFromL0Flag    = m_hevcSliceParams->collocated_from_l0_flag;
    curbe->DW47.IsLowDelay              = m_lowDelay;
    curbe->DW47.ScreenContentFlag       = m_hevcPicParams->bScreenContent;
    curbe->DW47.MultiSliceFlag         = (m_numSlices > 1);
    curbe->DW47.ArbitarySliceFlag      = m_arbitraryNumMbsInSlice;
    curbe->DW47.NumRegionMinus1        = m_walkingPatternParam.dwNumRegion - 1;

    if(m_pictureCodingType != I_TYPE)
    {
        curbe->DW48.CurrentTdL0_0 = ComputeTemporalDifference(m_hevcSliceParams->RefPicList[0][0]);
        curbe->DW48.CurrentTdL0_1 = ComputeTemporalDifference(m_hevcSliceParams->RefPicList[0][1]);
        curbe->DW49.CurrentTdL0_2 = ComputeTemporalDifference(m_hevcSliceParams->RefPicList[0][2]);
        curbe->DW49.CurrentTdL0_3 = ComputeTemporalDifference(m_hevcSliceParams->RefPicList[0][3]);
        if(m_pictureCodingType == B_TYPE) {
            curbe->DW50.CurrentTdL1_0 = ComputeTemporalDifference(m_hevcSliceParams->RefPicList[1][0]);
            curbe->DW50.CurrentTdL1_1 = ComputeTemporalDifference(m_hevcSliceParams->RefPicList[1][1]);
        }
    }

    curbe->DW52.NumofUnitInRegion          = m_walkingPatternParam.dwNumUnitsInRegion;
    curbe->DW52.MaxHeightInRegion          = m_walkingPatternParam.dwMaxHeightInRegion;

    uint32_t startBTI = 0;
    curbe->DW56.BTI_CU_Record                  = bindingTable->dwBindingTableEntries[startBTI++];
    curbe->DW57.BTI_PAK_Cmd                    = bindingTable->dwBindingTableEntries[startBTI++];
    curbe->DW58.BTI_Src_Y                      = bindingTable->dwBindingTableEntries[startBTI++];
    startBTI++; //skip UV index
    curbe->DW59.BTI_Intra_Dist                 = bindingTable->dwBindingTableEntries[startBTI++];
    curbe->DW60.BTI_Min_Dist                   = bindingTable->dwBindingTableEntries[startBTI++];
    curbe->DW61.BTI_HMEMVPredFwdBwdSurfIndex   = bindingTable->dwBindingTableEntries[startBTI++];
    curbe->DW62.BTI_HMEDistSurfIndex           = bindingTable->dwBindingTableEntries[startBTI++];
    curbe->DW63.BTI_Slice_Map                  = bindingTable->dwBindingTableEntries[startBTI++];
    curbe->DW64.BTI_VME_Saved_UNI_SIC          = bindingTable->dwBindingTableEntries[startBTI++];
    curbe->DW65.BTI_Simplest_Intra             = bindingTable->dwBindingTableEntries[startBTI++];
    curbe->DW66.BTI_Collocated_RefFrame        = bindingTable->dwBindingTableEntries[startBTI++];
    curbe->DW67.BTI_Reserved                   = bindingTable->dwBindingTableEntries[startBTI++];
    curbe->DW68.BTI_BRC_Input                  = bindingTable->dwBindingTableEntries[startBTI++];
    curbe->DW69.BTI_LCU_QP                     = bindingTable->dwBindingTableEntries[startBTI++];
    curbe->DW70.BTI_BRC_Data                   = bindingTable->dwBindingTableEntries[startBTI++];
    curbe->DW71.BTI_VMEInterPredictionSurfIndex= bindingTable->dwBindingTableEntries[startBTI++];
    if(m_pictureCodingType == P_TYPE)
    {
        //P MBEnc curbe 72~75 are different from B frame.
        startBTI += (CODECHAL_HEVC_P_MBENC_CONCURRENT_THD_MAP - CODECHAL_HEVC_P_MBENC_VME_FORWARD_0);
        curbe->DW72.BTI_ConcurrentThreadMap= bindingTable->dwBindingTableEntries[startBTI++];
        curbe->DW73.BTI_MB_Data_CurFrame   = bindingTable->dwBindingTableEntries[startBTI++];
        curbe->DW74.BTI_MVP_CurFrame       = bindingTable->dwBindingTableEntries[startBTI++];
        curbe->DW75.BTI_Haar_Dist16x16     = bindingTable->dwBindingTableEntries[startBTI++];
        curbe->DW76.BTI_Stats_Data         = bindingTable->dwBindingTableEntries[startBTI++];
        curbe->DW77.BTI_Frame_Stats_Data   = bindingTable->dwBindingTableEntries[startBTI++];
        curbe->DW78.BTI_Debug              = bindingTable->dwBindingTableEntries[startBTI++];
    }
    else
    {
        startBTI += (CODECHAL_HEVC_B_MBENC_VME_BACKWARD_7 - CODECHAL_HEVC_B_MBENC_VME_FORWARD_0 + 1);

        curbe->DW72.BTI_VMEInterPredictionBSurfIndex = bindingTable->dwBindingTableEntries[startBTI++];
        startBTI += (CODECHAL_HEVC_B_MBENC_VME_MUL_NOUSE_3 - CODECHAL_HEVC_B_MBENC_VME_MUL_BACKWARD_0 + 1);

        curbe->DW73.BTI_ConcurrentThreadMap= bindingTable->dwBindingTableEntries[startBTI++];
        curbe->DW74.BTI_MB_Data_CurFrame   = bindingTable->dwBindingTableEntries[startBTI++];
        curbe->DW75.BTI_MVP_CurFrame       = bindingTable->dwBindingTableEntries[startBTI++];
        curbe->DW76.BTI_Haar_Dist16x16     = bindingTable->dwBindingTableEntries[startBTI++];
        curbe->DW77.BTI_Stats_Data         = bindingTable->dwBindingTableEntries[startBTI++];
        curbe->DW78.BTI_Frame_Stats_Data   = bindingTable->dwBindingTableEntries[startBTI++];
        curbe->DW79.BTI_Debug              = bindingTable->dwBindingTableEntries[startBTI++];
    }

    // Intra refresh is enabled. Program related CURBE fields
    if (m_hevcPicParams->bEnableRollingIntraRefresh)
    {
        curbe->DW35.IntraRefreshEn         = true;
        curbe->DW35.FirstIntraRefresh      = m_firstIntraRefresh;
        curbe->DW35.HalfUpdateMixedLCU     = 0;
        curbe->DW35.EnableRollingIntra     = true;

        curbe->DW38.NumFrameInGOB            = m_frameNumInGob;
        curbe->DW38.NumIntraRefreshOffFrames = m_frameNumWithoutIntraRefresh;

        curbe->DW51.IntraRefreshQPDelta  = m_hevcPicParams->QpDeltaForInsertedIntra;
        curbe->DW51.IntraRefreshMBNum    = m_hevcPicParams->IntraInsertionLocation;
        curbe->DW51.IntraRefreshUnitInMB = m_hevcPicParams->IntraInsertionSize;

        curbe->DW53.IntraRefreshRefHeight = 40;
        curbe->DW53.IntraRefreshRefWidth  = 48;

        m_firstIntraRefresh = false;
        m_frameNumWithoutIntraRefresh = 0;
    }
    else if (m_pictureCodingType != I_TYPE) // don't increment num frames w/o refresh in case of TU7 I frames
    {
        m_frameNumWithoutIntraRefresh++;
    }

    CODECHAL_ENCODE_ASSERT(startBTI == bindingTable->dwNumBindingTableEntries);

    CODECHAL_MEDIA_STATE_TYPE encFunctionType = CODECHAL_MEDIA_STATE_HEVC_B_MBENC;
    if (m_pictureCodingType == P_TYPE)
    {
        //P frame curbe only use the DW0~DW75
        CODECHAL_ENCODE_CHK_STATUS_RETURN(
            AddCurbeToStateHeap(kernelState, encFunctionType, &cmd, sizeof(cmd) - sizeof(uint32_t)));
    }
    else
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(
            AddCurbeToStateHeap(kernelState, encFunctionType, &cmd, sizeof(cmd)));
    }

    MOS_COMMAND_BUFFER cmdBuffer;
    if(m_numMbBKernelSplit == 0)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SendKernelCmdsAndBindingTable(&cmdBuffer,
            kernelState,
            encFunctionType,
            &m_walkingPatternParam.ScoreBoard));
    }
    else
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(GetCommandBuffer(&cmdBuffer));

        MHW_INTERFACE_DESCRIPTOR_PARAMS idParams;
        MOS_ZeroMemory(&idParams, sizeof(idParams));
        idParams.pKernelState = kernelState;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSetInterfaceDescriptor(
            m_stateHeapInterface,
            1,
            &idParams));

        // Add binding table
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSetBindingTable(
            m_stateHeapInterface,
            kernelState));
    }

    //Add surface states
    startBTI = 0;

    //0: CU record
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_CU_RECORD,
        &bindingTable->dwBindingTableEntries[startBTI++]));

    //1: PAK command
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_HCP_PAK,
        &bindingTable->dwBindingTableEntries[startBTI++]));

    //2 and 3 Source Y and UV
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_RAW_Y_UV,
        &bindingTable->dwBindingTableEntries[startBTI++]));
    startBTI++;

    //4: Intra dist
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_INTRA_DIST,
        &bindingTable->dwBindingTableEntries[startBTI++]));

    //5: min distortion
    m_surfaceParams[SURFACE_MIN_DIST].bIsWritable   =
    m_surfaceParams[SURFACE_MIN_DIST].bRenderTarget = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_MIN_DIST,
        &bindingTable->dwBindingTableEntries[startBTI++]));

    if(m_hmeSupported)
    {
    //6: MV predictor from HME
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_HME_MVP,
        &bindingTable->dwBindingTableEntries[startBTI++]));

    //7: distortion from HME
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_HME_DIST,
        &bindingTable->dwBindingTableEntries[startBTI++]));
    }
    else
    {
        startBTI += 2;
    }

    //8: slice map
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_SLICE_MAP,
        &bindingTable->dwBindingTableEntries[startBTI++]));

    //9: VME UNI and SIC data
    m_surfaceParams[SURFACE_VME_UNI_SIC_DATA].bIsWritable   =
    m_surfaceParams[SURFACE_VME_UNI_SIC_DATA].bRenderTarget = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_VME_UNI_SIC_DATA,
        &bindingTable->dwBindingTableEntries[startBTI++]));

    //10: Simplest Intra
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_SIMPLIFIED_INTRA,
        &bindingTable->dwBindingTableEntries[startBTI++]));

    // 11: Reference frame col-located data surface
    if(mbCodeIdxForTempMVP == 0xFF)
    {
        startBTI++;
    }
    else
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
            kernelState,
            &cmdBuffer,
            SURFACE_COL_MB_MV,
            &bindingTable->dwBindingTableEntries[startBTI++],
            m_trackedBuf->GetMvTemporalBuffer(mbCodeIdxForTempMVP)));
        }

    // 12: Current frame col-located data surface -- reserved now
    startBTI++;

    // 13: BRC Input
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_BRC_INPUT,
        &bindingTable->dwBindingTableEntries[startBTI++]));

    // 14: LCU Qp
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_LCU_QP,
        &bindingTable->dwBindingTableEntries[startBTI++]));

    // 15: LCU BRC constant
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_BRC_DATA,
        &bindingTable->dwBindingTableEntries[startBTI++]));

    // 16 - 32 Current plus forward and backward surface 0-7
    //16: Source Y for VME
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_RAW_VME,
        &bindingTable->dwBindingTableEntries[startBTI++]));

    for(uint32_t surfaceIdx = 0; surfaceIdx < 8; surfaceIdx++)
    {
        CODEC_PICTURE refPic = m_hevcSliceParams->RefPicList[LIST_0][surfaceIdx];
        if (!CodecHal_PictureIsInvalid(refPic) &&
            !CodecHal_PictureIsInvalid(m_hevcPicParams->RefFrameList[refPic.FrameIdx]))
        {
            uint8_t idx = m_hevcPicParams->RefFrameList[refPic.FrameIdx].FrameIdx;

            // picture Y VME
            CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
                kernelState,
                &cmdBuffer,
                SURFACE_REF_FRAME_VME,
                &bindingTable->dwBindingTableEntries[startBTI++],
                &m_refList[idx]->sRefBuffer,
                curbe->DW6.FrameWidth,
                curbe->DW6.FrameHeight));
        }
        else
        {
            // Skip the binding table index because it is not used
            startBTI++;
        }

        refPic = m_hevcSliceParams->RefPicList[LIST_1][surfaceIdx];
        if (!CodecHal_PictureIsInvalid(refPic) &&
            !CodecHal_PictureIsInvalid(m_hevcPicParams->RefFrameList[refPic.FrameIdx]))
        {
            uint8_t idx = m_hevcPicParams->RefFrameList[refPic.FrameIdx].FrameIdx;

            // picture Y VME
            CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
                kernelState,
                &cmdBuffer,
                SURFACE_REF_FRAME_VME,
                &bindingTable->dwBindingTableEntries[startBTI++],
                &m_refList[idx]->sRefBuffer,
                curbe->DW6.FrameWidth,
                curbe->DW6.FrameHeight));
        }
        else
        {
            // Skip the binding table index because it is not used
            startBTI++;
        }
    }
    CODECHAL_ENCODE_ASSERT(startBTI == CODECHAL_HEVC_B_MBENC_VME_BACKWARD_7 - CODECHAL_HEVC_B_MBENC_BEGIN + 1);

    if (m_pictureCodingType != P_TYPE)
    {
        //33-41 VME multi-ref BTI -- Current plus [backward, nil][0..3]
        //33: Current Y VME surface
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
            kernelState,
            &cmdBuffer,
            SURFACE_RAW_VME,
            &bindingTable->dwBindingTableEntries[startBTI++]));

        for(uint32_t surfaceIdx = 0; surfaceIdx < 4; surfaceIdx++)
        {
            CODEC_PICTURE refPic = m_hevcSliceParams->RefPicList[1][surfaceIdx];
            if (!CodecHal_PictureIsInvalid(refPic) &&
                !CodecHal_PictureIsInvalid(m_hevcPicParams->RefFrameList[refPic.FrameIdx]))
            {
                uint8_t idx = m_hevcPicParams->RefFrameList[refPic.FrameIdx].FrameIdx;

                // picture Y VME
                CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
                    kernelState,
                    &cmdBuffer,
                    SURFACE_REF_FRAME_VME,
                    &bindingTable->dwBindingTableEntries[startBTI++],
                    &m_refList[idx]->sRefBuffer,
                    curbe->DW6.FrameWidth,
                    curbe->DW6.FrameHeight));
            }
            else
            {
                // Skip the binding table index because it is not used
                startBTI++;
            }

            // Skip the binding table index because it is not used
            startBTI++;
        }
        CODECHAL_ENCODE_ASSERT(startBTI == CODECHAL_HEVC_B_MBENC_VME_MUL_NOUSE_3 - CODECHAL_HEVC_B_MBENC_BEGIN + 1);
    }

    // B 42 or P 33: Concurrent thread
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        (SURFACE_ID)(SURFACE_CONCURRENT_THREAD + m_concurrentThreadIndex),
        &bindingTable->dwBindingTableEntries[startBTI++]));

    if (++m_concurrentThreadIndex >= NUM_CONCURRENT_THREAD)
    {
        m_concurrentThreadIndex = 0;
    }

    // B 43 or P 34: MV index buffer
    m_surfaceParams[SURFACE_MB_MV_INDEX].bIsWritable   =
    m_surfaceParams[SURFACE_MB_MV_INDEX].bRenderTarget = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_MB_MV_INDEX,
        &bindingTable->dwBindingTableEntries[startBTI++]));

    // B 44: or P 35: MVP index buffer
    m_surfaceParams[SURFACE_MVP_INDEX].bIsWritable   =
    m_surfaceParams[SURFACE_MVP_INDEX].bRenderTarget = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_MVP_INDEX,
        &bindingTable->dwBindingTableEntries[startBTI++]));

    if (!m_hwWalker)
    {
        eStatus = MOS_STATUS_UNKNOWN;
        CODECHAL_ENCODE_ASSERTMESSAGE("Currently HW walker shall not be disabled for CM based down scaling kernel.");
        return eStatus;
    }

    if(m_numMbBKernelSplit == 0)
    {
        // always use customized media walker
        MHW_WALKER_PARAMS walkerParams;
        MOS_SecureMemcpy(&walkerParams, sizeof(walkerParams), &m_walkingPatternParam.MediaWalker, sizeof(m_walkingPatternParam.MediaWalker));
        walkerParams.ColorCountMinusOne = m_walkingPatternParam.dwNumRegion - 1;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_renderEngineInterface->AddMediaObjectWalkerCmd(
            &cmdBuffer,
            &walkerParams));
    }
    else
    {
        int32_t localOuterLoopExecCount = m_walkingPatternParam.MediaWalker.dwLocalLoopExecCount;
        int32_t localInitialStartPointY = m_walkingPatternParam.MediaWalker.LocalStart.y;
        int32_t phase = MOS_MIN(m_numMbBKernelSplit, MAX_NUM_KERNEL_SPLIT);
        int32_t totalExecCount = localOuterLoopExecCount + 1;
        int32_t deltaExecCount = (((totalExecCount+phase - 1) / phase) + 1) & 0xfffe;
        int32_t remainExecCount = totalExecCount;

        int32_t deltaY = 0;
        if (m_enable26WalkingPattern)
        {
            deltaY = deltaExecCount / 2;
        }
        else
        {
            deltaY = deltaExecCount * 2;
        }

        int32_t startPointY[MAX_NUM_KERNEL_SPLIT] = { 0 };
        int32_t currentExecCount[MAX_NUM_KERNEL_SPLIT] = { -1 };
        currentExecCount[0] = (remainExecCount > deltaExecCount)?(deltaExecCount-1) :  (remainExecCount-1);
        startPointY[0] = localInitialStartPointY;

        for (auto i = 1; i < phase; i++)
        {
            remainExecCount -= deltaExecCount;
            if (remainExecCount < 1)
            {
                remainExecCount = 1;
            }

            currentExecCount[i] = (remainExecCount > deltaExecCount)?(deltaExecCount-1) :  (remainExecCount-1);
            startPointY[i] = startPointY[i-1] + deltaY;
        }

        for(auto i = 0; i < phase; i++)
        {
            if(currentExecCount[i] < 0)
            {
                break;
            }

            // Program render engine pipe commands
            SendKernelCmdsParams sendKernelCmdsParams = SendKernelCmdsParams();
            sendKernelCmdsParams.EncFunctionType        = encFunctionType;
            sendKernelCmdsParams.pKernelState           = kernelState;
            sendKernelCmdsParams.bEnableCustomScoreBoard= true;
            sendKernelCmdsParams.pCustomScoreBoard      = &m_walkingPatternParam.ScoreBoard;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(SendGenericKernelCmds(&cmdBuffer, &sendKernelCmdsParams));

            // Change walker execution count and local start Y for different phases
            m_walkingPatternParam.MediaWalker.dwLocalLoopExecCount = currentExecCount[i];
            m_walkingPatternParam.MediaWalker.LocalStart.y = startPointY[i];

            // always use customized media walker
            MHW_WALKER_PARAMS walkerParams;
            MOS_SecureMemcpy(&walkerParams, sizeof(walkerParams), &m_walkingPatternParam.MediaWalker, sizeof(m_walkingPatternParam.MediaWalker));
            walkerParams.ColorCountMinusOne = m_walkingPatternParam.dwNumRegion - 1;

            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_renderEngineInterface->AddMediaObjectWalkerCmd(
                &cmdBuffer,
                &walkerParams));
        }
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(EndKernelCall(
        encFunctionType,
        kernelState,
        &cmdBuffer));

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &m_mvIndex.sResource,
            CodechalDbgAttr::attrOutput,
            "MbData",
            m_mvpIndex.dwSize,
            0,
            CODECHAL_MEDIA_STATE_HEVC_B_MBENC));

         CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &m_mvpIndex.sResource,
            CodechalDbgAttr::attrOutput,
            "MvData",
            m_mvpIndex.dwSize,
            0,
            CODECHAL_MEDIA_STATE_HEVC_B_MBENC));
    )

    m_lastTaskInPhase = true;
    eStatus = Encode8x8BPakKernel(curbe);

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG9::Encode2xScalingKernel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    PerfTagSetting perfTag;
    CODECHAL_ENCODE_SET_PERFTAG_INFO(perfTag, CODECHAL_ENCODE_PERFTAG_CALL_SCALING_KERNEL);

    uint32_t krnIdx = CODECHAL_HEVC_MBENC_2xSCALING;
    auto     kernelState         = &m_mbEncKernelStates[krnIdx];
    auto     scalingBindingTable = &m_mbEncKernelBindingTable[krnIdx];
    if (m_firstTaskInPhase || !m_singleTaskPhaseSupported)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(RequestSshAndVerifyCommandBufferSize(kernelState));
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalGetResourceInfo(
        m_osInterface,
        &m_scaled2xSurface));

    // Setup DSH
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->AssignDshAndSshSpace(
        m_stateHeapInterface,
        kernelState,
        false,
        0,
        false,
        m_storeData));

    //Setup CURBE
    MEDIA_OBJECT_DOWNSCALING_2X_STATIC_DATA_G9  cmd, *curbe = &cmd;
    MOS_ZeroMemory(curbe, sizeof(*curbe));
    curbe->DW0.PicWidth  = MOS_ALIGN_CEIL(m_frameWidth, CODECHAL_MACROBLOCK_WIDTH);
    curbe->DW0.PicHeight    = MOS_ALIGN_CEIL(m_frameHeight, CODECHAL_MACROBLOCK_HEIGHT);

    uint32_t startBTI = 0;
    curbe->DW8.BTI_Src_Y    = scalingBindingTable->dwBindingTableEntries[startBTI++];
    curbe->DW9.BTI_Dst_Y    = scalingBindingTable->dwBindingTableEntries[startBTI++];

    CODECHAL_MEDIA_STATE_TYPE encFunctionType = CODECHAL_MEDIA_STATE_2X_SCALING;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(
        AddCurbeToStateHeap(kernelState, encFunctionType, &cmd, sizeof(cmd)));

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendKernelCmdsAndBindingTable(
        &cmdBuffer,
        kernelState,
        encFunctionType,
        nullptr));

    // Add surface states, 2X scaling uses U16Norm surface format
    startBTI = 0;

    // Source surface/s
    auto surfaceCodecParams = &m_surfaceParams[SURFACE_RAW_Y];
    surfaceCodecParams->bUse16UnormSurfaceFormat = true;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_RAW_Y,
        &scalingBindingTable->dwBindingTableEntries[startBTI++]
    ));

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_mmcState);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mmcState->SetSurfaceParams(surfaceCodecParams));

    // Destination surface/s
    m_scaled2xSurface.dwWidth  = MOS_ALIGN_CEIL((m_frameWidth / SCALE_FACTOR_2x), CODECHAL_MACROBLOCK_WIDTH);
    m_scaled2xSurface.dwHeight = MOS_ALIGN_CEIL((m_frameHeight / SCALE_FACTOR_2x), CODECHAL_MACROBLOCK_HEIGHT);

    m_surfaceParams[SURFACE_Y_2X].bUse16UnormSurfaceFormat =
    m_surfaceParams[SURFACE_Y_2X].bIsWritable   =
    m_surfaceParams[SURFACE_Y_2X].bRenderTarget = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_Y_2X,
        &scalingBindingTable->dwBindingTableEntries[startBTI++]
        ));

    if (!m_hwWalker)
    {
        eStatus = MOS_STATUS_UNKNOWN;
        CODECHAL_ENCODE_ASSERTMESSAGE("Currently HW walker shall not be disabled for CM based down scaling kernel.");
        return eStatus;
    }

    CODECHAL_WALKER_CODEC_PARAMS walkerCodecParams;
    MOS_ZeroMemory(&walkerCodecParams, sizeof(walkerCodecParams));
    walkerCodecParams.WalkerMode        = m_walkerMode;
    // check kernel of Downscaling 2x kernels for Ultra HME.
    walkerCodecParams.dwResolutionX     = MOS_ALIGN_CEIL(m_frameWidth,  32) >> 5;
    // The frame kernel process 32x32 input pixels and output 16x16 down sampled pixels
    walkerCodecParams.dwResolutionY     = MOS_ALIGN_CEIL(m_frameHeight, 32) >> 5;
    /* Enforce no dependency dispatch order for Scaling kernel,  */
    walkerCodecParams.bNoDependency     = true;

    MHW_WALKER_PARAMS walkerParams;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalInitMediaObjectWalkerParams(
        m_hwInterface,
        &walkerParams,
        &walkerCodecParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_renderEngineInterface->AddMediaObjectWalkerCmd(
        &cmdBuffer,
        &walkerParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(EndKernelCall(
        encFunctionType,
        kernelState,
        &cmdBuffer));

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG9::Encode32x32PuModeDecisionKernel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    PerfTagSetting perfTag;
    CODECHAL_ENCODE_SET_PERFTAG_INFO(perfTag, CODECHAL_ENCODE_PERFTAG_CALL_32X32_PU_MD);

    uint32_t krnIdx = CODECHAL_HEVC_MBENC_32x32MD;
    auto     kernelState  = &m_mbEncKernelStates[krnIdx];
    auto     bindingTable = &m_mbEncKernelBindingTable[krnIdx];
    if (m_firstTaskInPhase || !m_singleTaskPhaseSupported)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(RequestSshAndVerifyCommandBufferSize(kernelState));
    }

    // Setup DSH
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->AssignDshAndSshSpace(
        m_stateHeapInterface,
        kernelState,
        false,
        0,
        false,
        m_storeData));

    //Setup CURBE
    uint32_t log2MaxCUSize = m_hevcSeqParams->log2_max_coding_block_size_minus3 + 3;

    CalcLambda(CODECHAL_ENCODE_HEVC_I_SLICE, INTRA_TRANSFORM_HAAR);
    int32_t sliceQp = CalSliceQp();

    double lambdaScalingFactor = 1.0;
    double qpLambda = m_qpLambdaMd[CODECHAL_ENCODE_HEVC_I_SLICE][sliceQp];
    double squaredQpLambda = qpLambda * qpLambda;
    m_fixedPointLambda = (uint32_t)(lambdaScalingFactor * squaredQpLambda * (1<<10));

    CODECHAL_ENC_HEVC_I_32x32_PU_MODE_DECISION_CURBE_G9 cmd, *curbe = &cmd;
    MOS_ZeroMemory(curbe, sizeof(*curbe));
    curbe->DW0.FrameWidth      = MOS_ALIGN_CEIL(m_frameWidth, CODECHAL_MACROBLOCK_WIDTH);
    curbe->DW0.FrameHeight     = MOS_ALIGN_CEIL(m_frameHeight, CODECHAL_MACROBLOCK_HEIGHT);

    curbe->DW1.EnableDebugDump = false;
    curbe->DW1.LCUType         = (log2MaxCUSize==6)? 0 /*64x64*/ : 1 /*32x32*/;
    curbe->DW1.PuType          = 0; // 32x32 PU
    curbe->DW1.BRCEnable            = m_encodeParams.bMbQpDataEnabled || m_brcEnabled;
    curbe->DW1.LCUBRCEnable         = m_encodeParams.bMbQpDataEnabled || m_lcuBrcEnabled;
    curbe->DW1.SliceType            = PicCodingTypeToSliceType(m_hevcPicParams->CodingType);
    curbe->DW1.FASTSurveillanceFlag = (m_hevcPicParams->CodingType == I_TYPE) ? 0 : m_hevcSeqParams->bVideoSurveillance;
    curbe->DW1.ROIEnable            = (m_hevcPicParams->NumROI > 0);

    curbe->DW2.Lambda          = m_fixedPointLambda;

    curbe->DW3.ModeCost32x32   = 0;

    curbe->DW4.EarlyExit       = (uint32_t)-1;

    uint32_t startIndex = 0;
    curbe->DW8.BTI_32x32PU_Output    = bindingTable->dwBindingTableEntries[startIndex++];
    curbe->DW9.BTI_Src_Y           = bindingTable->dwBindingTableEntries[startIndex++];
    startIndex++; // skip one BTI for Y and UV have the same BTI
    curbe->DW10.BTI_Src_Y2x        = bindingTable->dwBindingTableEntries[startIndex++];
    curbe->DW11.BTI_Slice_Map      = bindingTable->dwBindingTableEntries[startIndex++];
    curbe->DW12.BTI_Src_Y2x_VME    = bindingTable->dwBindingTableEntries[startIndex++];
    curbe->DW13.BTI_Brc_Input      = bindingTable->dwBindingTableEntries[startIndex++];
    curbe->DW14.BTI_LCU_Qp_Surface = bindingTable->dwBindingTableEntries[startIndex++];
    curbe->DW15.BTI_Brc_Data       = bindingTable->dwBindingTableEntries[startIndex++];
    curbe->DW16.BTI_Stats_Data     = bindingTable->dwBindingTableEntries[startIndex++];
    curbe->DW17.BTI_Kernel_Debug   = bindingTable->dwBindingTableEntries[startIndex++];

    CODECHAL_ENCODE_ASSERT(startIndex == bindingTable->dwNumBindingTableEntries);

    CODECHAL_MEDIA_STATE_TYPE encFunctionType = CODECHAL_MEDIA_STATE_32x32_PU_MODE_DECISION;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(
        AddCurbeToStateHeap(kernelState, encFunctionType, &cmd, sizeof(cmd)));

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendKernelCmdsAndBindingTable(
        &cmdBuffer,
        kernelState,
        encFunctionType,
        nullptr));

    //Add surface states
    startIndex = 0;

    // 32x32 PU output
    m_surfaceParams[SURFACE_32x32_PU_OUTPUT].bIsWritable   =
    m_surfaceParams[SURFACE_32x32_PU_OUTPUT].bRenderTarget = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_32x32_PU_OUTPUT,
        &bindingTable->dwBindingTableEntries[startIndex++]));

    // Source Y and UV
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_RAW_Y_UV,
        &bindingTable->dwBindingTableEntries[startIndex++]));
    startIndex ++; // UV index

    // Source Y2x
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_Y_2X,
        &bindingTable->dwBindingTableEntries[startIndex++]));

    // Slice map
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_SLICE_MAP,
        &bindingTable->dwBindingTableEntries[startIndex++]));

    // Source Y2x for VME
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_Y_2X_VME,
        &bindingTable->dwBindingTableEntries[startIndex++]));

    // BRC Input
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_BRC_INPUT,
        &bindingTable->dwBindingTableEntries[startIndex++]));

    // LCU Qp surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_LCU_QP,
        &bindingTable->dwBindingTableEntries[startIndex++]));

    // BRC data surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_BRC_DATA,
        &bindingTable->dwBindingTableEntries[startIndex++]));

    if (!m_hwWalker)
    {
        eStatus = MOS_STATUS_UNKNOWN;
        CODECHAL_ENCODE_ASSERTMESSAGE("Currently HW walker shall not be disabled for CM based down scaling kernel.");
        return eStatus;
    }

    CODECHAL_WALKER_CODEC_PARAMS walkerCodecParams;
    MOS_ZeroMemory(&walkerCodecParams, sizeof(walkerCodecParams));
    walkerCodecParams.WalkerMode            = m_walkerMode;
    walkerCodecParams.dwResolutionX         = MOS_ALIGN_CEIL(m_frameWidth,  32) >> 5; /* looping for Walker is needed at 8x8 block level */
    walkerCodecParams.dwResolutionY         = MOS_ALIGN_CEIL(m_frameHeight, 32) >> 5;
    walkerCodecParams.bNoDependency         = true;     /* Enforce no dependency dispatch order for 32x32 MD kernel  */

    MHW_WALKER_PARAMS walkerParams;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalInitMediaObjectWalkerParams(
        m_hwInterface,
        &walkerParams,
        &walkerCodecParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_renderEngineInterface->AddMediaObjectWalkerCmd(
        &cmdBuffer,
        &walkerParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(EndKernelCall(
        encFunctionType,
        kernelState,
        &cmdBuffer));

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG9::Encode32X32BIntraCheckKernel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    PerfTagSetting perfTag;
    CODECHAL_ENCODE_SET_PERFTAG_INFO(perfTag, CODECHAL_ENCODE_PERFTAG_CALL_32X32_B_IC);

    uint32_t krnIdx = CODECHAL_HEVC_MBENC_32x32INTRACHECK;
    auto     kernelState  = &m_mbEncKernelStates[krnIdx];
    auto     bindingTable = &m_mbEncKernelBindingTable[krnIdx];

    if (m_firstTaskInPhase || !m_singleTaskPhaseSupported)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(RequestSshAndVerifyCommandBufferSize(kernelState));
    }

    // Setup DSH
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->AssignDshAndSshSpace(
        m_stateHeapInterface,
        kernelState,
        false,
        0,
        false,
        m_storeData));

    // Setup CURBE
    if (m_pictureCodingType == P_TYPE)
    {
        CalcLambda(CODECHAL_ENCODE_HEVC_P_SLICE, INTRA_TRANSFORM_HAAR);
    }
    else
    {
        CalcLambda(CODECHAL_ENCODE_HEVC_B_SLICE, INTRA_TRANSFORM_HAAR);
    }
    int32_t sliceQp = CalSliceQp();

    double lambdaScalingFactor = 1.0;
    double qpLambda = m_qpLambdaMd[CODECHAL_ENCODE_HEVC_I_SLICE][sliceQp];
    double squaredQpLambda = qpLambda * qpLambda;
    m_fixedPointLambda = (uint32_t)(lambdaScalingFactor * squaredQpLambda * (1<<10));

    CODECHAL_ENC_HEVC_B_32x32_PU_INTRA_CHECK_CURBE_G9 cmd, *curbe = &cmd;
    MOS_ZeroMemory(curbe, sizeof(*curbe));
    curbe->DW0.FrameWidth      = MOS_ALIGN_CEIL(m_frameWidth, CODECHAL_MACROBLOCK_WIDTH);
    curbe->DW0.FrameHeight     = MOS_ALIGN_CEIL(m_frameHeight, CODECHAL_MACROBLOCK_HEIGHT);

    curbe->DW1.EnableDebugDump = false;
    curbe->DW1.EnableIntraEarlyExit = (m_hevcSeqParams->TargetUsage == 0x04) ? ((m_hevcPicParams->CodingType == I_TYPE) ? 0 : 1) : 0;
    curbe->DW1.Flags           = 0;
    curbe->DW1.Log2MinTUSize        = m_hevcSeqParams->log2_min_transform_block_size_minus2 + 2;
    curbe->DW1.SliceType            = m_hevcSliceParams->slice_type;
    curbe->DW1.HMEEnable            = m_hmeEnabled;
    curbe->DW1.FASTSurveillanceFlag = (m_hevcPicParams->CodingType == I_TYPE) ? 0 : m_hevcSeqParams->bVideoSurveillance;

    curbe->DW2.QpMultiplier    = 100;
    curbe->DW2.QpValue         = 0;     // MBZ

    uint32_t startIndex = 0;
    curbe->DW8.BTI_Per32x32PuIntraCheck    = bindingTable->dwBindingTableEntries[startIndex++];
    curbe->DW9.BTI_Src_Y            = bindingTable->dwBindingTableEntries[startIndex++];
    startIndex++; // skip one BTI for Y and UV have the same BTI
    curbe->DW10.BTI_Src_Y2X         = bindingTable->dwBindingTableEntries[startIndex++];
    curbe->DW11.BTI_Slice_Map       = bindingTable->dwBindingTableEntries[startIndex++];
    curbe->DW12.BTI_VME_Y2X         = bindingTable->dwBindingTableEntries[startIndex++];
    curbe->DW13.BTI_Simplest_Intra  = bindingTable->dwBindingTableEntries[startIndex++];
    curbe->DW14.BTI_HME_MVPred      = bindingTable->dwBindingTableEntries[startIndex++];
    curbe->DW15.BTI_HME_Dist        = bindingTable->dwBindingTableEntries[startIndex++];
    curbe->DW16.BTI_LCU_Skip        = bindingTable->dwBindingTableEntries[startIndex++];
    curbe->DW17.BTI_Debug           = bindingTable->dwBindingTableEntries[startIndex++];

    CODECHAL_ENCODE_ASSERT(startIndex == bindingTable->dwNumBindingTableEntries);

    CODECHAL_MEDIA_STATE_TYPE encFunctionType = CODECHAL_MEDIA_STATE_32x32_B_INTRA_CHECK;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(
        AddCurbeToStateHeap(kernelState, encFunctionType, &cmd, sizeof(cmd)));

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendKernelCmdsAndBindingTable(
        &cmdBuffer,
        kernelState,
        encFunctionType,
        nullptr));

    //Add surface states
    startIndex = 0;

    // 32x32 PU B Intra Check Output
    m_surfaceParams[SURFACE_32x32_PU_OUTPUT].bIsWritable   =
    m_surfaceParams[SURFACE_32x32_PU_OUTPUT].bRenderTarget = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_32x32_PU_OUTPUT,
        &bindingTable->dwBindingTableEntries[startIndex++]));

    // Source Y and UV
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_RAW_Y_UV,
        &bindingTable->dwBindingTableEntries[startIndex++]));
    startIndex++;

    // Source Y2x
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_Y_2X,
        &bindingTable->dwBindingTableEntries[startIndex++]));

    // Slice map
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_SLICE_MAP,
        &bindingTable->dwBindingTableEntries[startIndex++]));

    // Source Y2x for VME
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_Y_2X_VME,
        &bindingTable->dwBindingTableEntries[startIndex++]));

    // Simplest Intra
    m_surfaceParams[SURFACE_SIMPLIFIED_INTRA].bIsWritable   =
    m_surfaceParams[SURFACE_SIMPLIFIED_INTRA].bRenderTarget = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_SIMPLIFIED_INTRA,
        &bindingTable->dwBindingTableEntries[startIndex++]));

    if(m_hmeSupported)
    {
    //MV predictor from HME
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_HME_MVP,
        &bindingTable->dwBindingTableEntries[startIndex++]));

    //distortion from HME
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_HME_DIST,
        &bindingTable->dwBindingTableEntries[startIndex++]));
    }
    else
    {
        startIndex += 2;
    }

    // LCU Qp/Skip surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_LCU_QP,
        &bindingTable->dwBindingTableEntries[startIndex++]));

    if (!m_hwWalker)
    {
        eStatus = MOS_STATUS_UNKNOWN;
        CODECHAL_ENCODE_ASSERTMESSAGE("Currently HW walker shall not be disabled for CM based down scaling kernel.");
        return eStatus;
    }

    CODECHAL_WALKER_CODEC_PARAMS walkerCodecParams;
    MOS_ZeroMemory(&walkerCodecParams, sizeof(walkerCodecParams));
    walkerCodecParams.WalkerMode        = m_walkerMode;
    /* looping for Walker is needed at 8x8 block level */
    walkerCodecParams.dwResolutionX     = MOS_ALIGN_CEIL(m_frameWidth,  32) >> 5;
    walkerCodecParams.dwResolutionY     = MOS_ALIGN_CEIL(m_frameHeight, 32) >> 5;
    /* Enforce no dependency dispatch order for 32x32 B Intra Check kernel  */
    walkerCodecParams.bNoDependency     = true;

    MHW_WALKER_PARAMS walkerParams;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalInitMediaObjectWalkerParams(
        m_hwInterface,
        &walkerParams,
        &walkerCodecParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_renderEngineInterface->AddMediaObjectWalkerCmd(
        &cmdBuffer,
        &walkerParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(EndKernelCall(
        encFunctionType,
        kernelState,
        &cmdBuffer));

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG9::Encode16x16SadPuComputationKernel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    PerfTagSetting perfTag;
    CODECHAL_ENCODE_SET_PERFTAG_INFO(perfTag, CODECHAL_ENCODE_PERFTAG_CALL_16X16_SAD);

    uint32_t krnIdx = CODECHAL_HEVC_MBENC_16x16SAD;
    auto     kernelState  = &m_mbEncKernelStates[krnIdx];
    auto     bindingTable = &m_mbEncKernelBindingTable[krnIdx];
    if (m_firstTaskInPhase || !m_singleTaskPhaseSupported)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(RequestSshAndVerifyCommandBufferSize(kernelState));
    }

    //Setup DSH
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->AssignDshAndSshSpace(
        m_stateHeapInterface,
        kernelState,
        false,
        0,
        false,
        m_storeData));

    // Setup CURBE
    CODECHAL_ENC_HEVC_I_16x16_SAD_CURBE_G9 cmd, *curbe = &cmd;

    MOS_ZeroMemory(curbe, sizeof(*curbe));
    curbe->DW0.FrameWidth      = MOS_ALIGN_CEIL(m_frameWidth, CODECHAL_MACROBLOCK_WIDTH);
    curbe->DW0.FrameHeight     = MOS_ALIGN_CEIL(m_frameHeight, CODECHAL_MACROBLOCK_HEIGHT);

    curbe->DW1.Log2MaxCUSize        = m_hevcSeqParams->log2_max_coding_block_size_minus3 + 3;
    curbe->DW1.Log2MinCUSize        = m_hevcSeqParams->log2_min_coding_block_size_minus3 + 3;
    curbe->DW1.Log2MinTUSize        = m_hevcSeqParams->log2_min_transform_block_size_minus2 + 2;
    curbe->DW1.EnableIntraEarlyExit = (m_hevcSeqParams->TargetUsage == 0x04) ? ((m_hevcPicParams->CodingType == I_TYPE) ? 0 : 1) : 0;

    curbe->DW2.SliceType       = PicCodingTypeToSliceType(m_hevcPicParams->CodingType);
    curbe->DW2.SimFlagForInter = false;
    if (m_hevcPicParams->CodingType != I_TYPE)
    {
        curbe->DW2.FASTSurveillanceFlag = m_hevcSeqParams->bVideoSurveillance;
    }

    uint32_t startIndex = 0;
    curbe->DW8.BTI_Src_Y                   = bindingTable->dwBindingTableEntries[startIndex++];
    startIndex++; // skip UV BTI
    curbe->DW9.BTI_Sad_16x16_PU_Output     = bindingTable->dwBindingTableEntries[startIndex++];
    curbe->DW10.BTI_32x32_Pu_ModeDecision  = bindingTable->dwBindingTableEntries[startIndex++];
    curbe->DW11.BTI_Slice_Map              = bindingTable->dwBindingTableEntries[startIndex++];
    curbe->DW12.BTI_Simplest_Intra         = bindingTable->dwBindingTableEntries[startIndex++];
    curbe->DW13.BTI_Debug                  = bindingTable->dwBindingTableEntries[startIndex++];

    CODECHAL_ENCODE_ASSERT(startIndex == bindingTable->dwNumBindingTableEntries);

    CODECHAL_MEDIA_STATE_TYPE encFunctionType = CODECHAL_MEDIA_STATE_16x16_PU_SAD;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(
        AddCurbeToStateHeap(kernelState, encFunctionType, &cmd, sizeof(cmd)));

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendKernelCmdsAndBindingTable(
        &cmdBuffer,
        kernelState,
        encFunctionType,
        nullptr));

    //Add surface states
    startIndex = 0;

    // Source Y and UV
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_RAW_Y_UV,
        &bindingTable->dwBindingTableEntries[startIndex++]));
    startIndex++;

    // 16x16 PU SAD output
    m_surfaceParams[SURFACE_16x16PU_SAD].bIsWritable   =
    m_surfaceParams[SURFACE_16x16PU_SAD].bRenderTarget = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_16x16PU_SAD,
        &bindingTable->dwBindingTableEntries[startIndex++]));

    // 32x32 PU MD data
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_32x32_PU_OUTPUT,
        &bindingTable->dwBindingTableEntries[startIndex++]));

    // Slice map
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_SLICE_MAP,
        &bindingTable->dwBindingTableEntries[startIndex++]));

    // Simplest Intra
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_SIMPLIFIED_INTRA,
        &bindingTable->dwBindingTableEntries[startIndex++]));

    if (!m_hwWalker)
    {
        eStatus = MOS_STATUS_UNKNOWN;
        CODECHAL_ENCODE_ASSERTMESSAGE("Currently HW walker shall not be disabled for CM based down scaling kernel.");
        return eStatus;
    }

    CODECHAL_WALKER_CODEC_PARAMS walkerCodecParams;
    MOS_ZeroMemory(&walkerCodecParams, sizeof(walkerCodecParams));
    walkerCodecParams.WalkerMode        = m_walkerMode;
    /* looping for Walker is needed at 16x16 block level */
    walkerCodecParams.dwResolutionX     = MOS_ALIGN_CEIL(m_frameWidth,  16) >> 4;
    walkerCodecParams.dwResolutionY     = MOS_ALIGN_CEIL(m_frameHeight, 16) >> 4;
    /* Enforce no dependency dispatch order for the 16x16 SAD kernel  */
    walkerCodecParams.bNoDependency     = true;

    MHW_WALKER_PARAMS walkerParams;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalInitMediaObjectWalkerParams(
        m_hwInterface,
        &walkerParams,
        &walkerCodecParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_renderEngineInterface->AddMediaObjectWalkerCmd(
        &cmdBuffer,
        &walkerParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(EndKernelCall(
        encFunctionType,
        kernelState,
        &cmdBuffer));

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG9::Encode16x16PuModeDecisionKernel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    PerfTagSetting perfTag;
    CODECHAL_ENCODE_SET_PERFTAG_INFO(perfTag, CODECHAL_ENCODE_PERFTAG_CALL_16X16_PU_MD);

    uint32_t krnIdx = CODECHAL_HEVC_MBENC_16x16MD;
    auto     kernelState  = &m_mbEncKernelStates[krnIdx];
    auto     bindingTable = &m_mbEncKernelBindingTable[krnIdx];
    if (m_firstTaskInPhase || !m_singleTaskPhaseSupported)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(RequestSshAndVerifyCommandBufferSize(kernelState));
    }

    // Setup DSH
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->AssignDshAndSshSpace(
        m_stateHeapInterface,
        kernelState,
        false,
        0,
        false,
        m_storeData));

    // Setup CURBE
    int32_t sliceQp = CalSliceQp();
    uint8_t sliceType = PicCodingTypeToSliceType(m_hevcPicParams->CodingType);

    double lambdaScaleFactor = 0.46 + sliceQp - 22;
    if (lambdaScaleFactor < 0)
    {
        lambdaScaleFactor = 0.46;
    }

    if (lambdaScaleFactor > 15)
    {
        lambdaScaleFactor = 15;
    }

    double squredLambda = lambdaScaleFactor * pow(2.0, ((double)sliceQp-12.0)/6);
    m_fixedPointLambdaForLuma = (uint32_t)(squredLambda * (1<<10));

    double lambdaScalingFactor = 1.0;
    double qpLambda = m_qpLambdaMd[sliceType][sliceQp];
    double squaredQpLambda = qpLambda * qpLambda;
    m_fixedPointLambdaForChroma = (uint32_t)(lambdaScalingFactor * squaredQpLambda * (1<<10));

    LoadCosts(sliceType, (uint8_t)sliceQp, INTRA_TRANSFORM_HAAR);

    CODECHAL_ENC_HEVC_I_16x16_PU_MODEDECISION_CURBE_G9 cmd, *curbe = &cmd;
    MOS_ZeroMemory(curbe, sizeof(*curbe));

    uint32_t log2MaxCUSize         = m_hevcSeqParams->log2_max_coding_block_size_minus3 + 3;
    curbe->DW0.FrameWidth          = MOS_ALIGN_CEIL(m_frameWidth, CODECHAL_MACROBLOCK_WIDTH);
    curbe->DW0.FrameHeight         = MOS_ALIGN_CEIL(m_frameHeight, CODECHAL_MACROBLOCK_HEIGHT);

    curbe->DW1.Log2MaxCUSize       = log2MaxCUSize;
    curbe->DW1.Log2MinCUSize       = m_hevcSeqParams->log2_min_coding_block_size_minus3 + 3;
    curbe->DW1.Log2MinTUSize       = m_hevcSeqParams->log2_min_transform_block_size_minus2 + 2;
    curbe->DW1.SliceQp             = sliceQp;

    curbe->DW2.FixedPoint_Lambda_PredMode = m_fixedPointLambdaForChroma;

    curbe->DW3.LambdaScalingFactor    = 1;
    curbe->DW3.SliceType              = sliceType;
    curbe->DW3.EnableIntraEarlyExit   = (m_hevcSeqParams->TargetUsage == 0x04) ? ((m_hevcPicParams->CodingType == I_TYPE) ? 0 : 1) : 0;
    curbe->DW3.BRCEnable              = m_encodeParams.bMbQpDataEnabled || m_brcEnabled;
    curbe->DW3.LCUBRCEnable           = m_encodeParams.bMbQpDataEnabled || m_lcuBrcEnabled;
    curbe->DW3.ROIEnable              = (m_hevcPicParams->NumROI > 0);
    curbe->DW3.FASTSurveillanceFlag   = (m_hevcPicParams->CodingType == I_TYPE) ? 0 : m_hevcSeqParams->bVideoSurveillance;
    curbe->DW3.EnableRollingIntra     = m_hevcPicParams->bEnableRollingIntraRefresh;
    //Given only Column Rolling I is supported, if in future, Row Rolling I support to be added, then, need to make change here as per Kernel
    curbe->DW3.IntraRefreshEn         = m_hevcPicParams->bEnableRollingIntraRefresh;
    curbe->DW3.HalfUpdateMixedLCU     = 0;

    curbe->DW4.PenaltyForIntra8x8NonDCPredMode = 0;
    curbe->DW4.IntraComputeType                = 1;
    curbe->DW4.AVCIntra8x8Mask                 = 0;
    curbe->DW4.IntraSadAdjust                  = 2;

    double lambdaMd       = sqrt(0.57*pow(2.0, ((double)sliceQp-12.0)/3));
    squredLambda          = lambdaMd * lambdaMd;
    uint32_t newLambda      = (uint32_t)(squredLambda*(1<<10));
    curbe->DW5.FixedPoint_Lambda_CU_Mode_for_Cost_Calculation = newLambda;

    curbe->DW6.ScreenContentFlag = m_hevcPicParams->bScreenContent;

    curbe->DW7.ModeCostIntraNonPred = m_modeCost[0];
    curbe->DW7.ModeCostIntra16x16   = m_modeCost[1];
    curbe->DW7.ModeCostIntra8x8     = m_modeCost[2];
    curbe->DW7.ModeCostIntra4x4     = m_modeCost[3];

    curbe->DW8.FixedPoint_Lambda_CU_Mode_for_Luma = m_fixedPointLambdaForLuma;

    if (m_hevcPicParams->bEnableRollingIntraRefresh)
    {
        curbe->DW9.IntraRefreshMBNum    = m_hevcPicParams->IntraInsertionLocation;
        curbe->DW9.IntraRefreshQPDelta  = m_hevcPicParams->QpDeltaForInsertedIntra;
        curbe->DW9.IntraRefreshUnitInMB = m_hevcPicParams->IntraInsertionSize;
    }

    curbe->DW10.SimplifiedFlagForInter = 0;
    curbe->DW10.HaarTransformMode      = (m_hevcPicParams->CodingType == I_TYPE) ? false : true;

    uint32_t startBTI = 0;
    curbe->DW16.BTI_Src_Y              = bindingTable->dwBindingTableEntries[startBTI++];
    startBTI++; // skip UV BTI
    curbe->DW17.BTI_Sad_16x16_PU       = bindingTable->dwBindingTableEntries[startBTI++];
    curbe->DW18.BTI_PAK_Object         = bindingTable->dwBindingTableEntries[startBTI++];
    curbe->DW19.BTI_SAD_32x32_PU_mode  = bindingTable->dwBindingTableEntries[startBTI++];
    curbe->DW20.BTI_VME_Mode_8x8       = bindingTable->dwBindingTableEntries[startBTI++];
    curbe->DW21.BTI_Slice_Map          = bindingTable->dwBindingTableEntries[startBTI++];
    curbe->DW22.BTI_VME_Src            = bindingTable->dwBindingTableEntries[startBTI++];
    curbe->DW23.BTI_BRC_Input          = bindingTable->dwBindingTableEntries[startBTI++];
    curbe->DW24.BTI_Simplest_Intra     = bindingTable->dwBindingTableEntries[startBTI++];
    curbe->DW25.BTI_LCU_Qp_Surface     = bindingTable->dwBindingTableEntries[startBTI++];
    curbe->DW26.BTI_BRC_Data           = bindingTable->dwBindingTableEntries[startBTI++];
    curbe->DW27.BTI_Debug              = bindingTable->dwBindingTableEntries[startBTI++];

    CODECHAL_ENCODE_ASSERT(startBTI == bindingTable->dwNumBindingTableEntries);

    CODECHAL_MEDIA_STATE_TYPE encFunctionType = CODECHAL_MEDIA_STATE_16x16_PU_MODE_DECISION;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(
        AddCurbeToStateHeap(kernelState, encFunctionType, &cmd, sizeof(cmd)));

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendKernelCmdsAndBindingTable(
        &cmdBuffer,
        kernelState,
        encFunctionType,
        nullptr));

    //Add surface states
    startBTI = 0;

    // Source Y and UV:
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_RAW_Y_UV,
        &bindingTable->dwBindingTableEntries[startBTI++]));
    startBTI++;

    // 16x16 PU SAD output
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_16x16PU_SAD,
        &bindingTable->dwBindingTableEntries[startBTI++]));

    // PAK object output
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_CU_RECORD,
        &bindingTable->dwBindingTableEntries[startBTI++]));

    // 32x32 PU MD data
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_32x32_PU_OUTPUT,
        &bindingTable->dwBindingTableEntries[startBTI++]));

    // VME 8x8 mode
    m_surfaceParams[SURFACE_VME_8x8].bIsWritable   =
    m_surfaceParams[SURFACE_VME_8x8].bRenderTarget = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_VME_8x8,
        &bindingTable->dwBindingTableEntries[startBTI++]));

    // Slice map
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_SLICE_MAP,
        &bindingTable->dwBindingTableEntries[startBTI++]));

    // Source Y for VME
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_RAW_VME,
        &bindingTable->dwBindingTableEntries[startBTI++]));

    // BRC Input
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_BRC_INPUT,
        &bindingTable->dwBindingTableEntries[startBTI++]));

    // Simplest Intra
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_SIMPLIFIED_INTRA,
        &bindingTable->dwBindingTableEntries[startBTI++]));

    // LCU Qp surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_LCU_QP,
        &bindingTable->dwBindingTableEntries[startBTI++]));

    // BRC data surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_BRC_DATA,
        &bindingTable->dwBindingTableEntries[startBTI++]));

    if (!m_hwWalker)
    {
        eStatus = MOS_STATUS_UNKNOWN;
        CODECHAL_ENCODE_ASSERTMESSAGE("Currently HW walker shall not be disabled for CM based down scaling kernel.");
        return eStatus;
    }

    CODECHAL_WALKER_CODEC_PARAMS walkerCodecParams;
    MOS_ZeroMemory(&walkerCodecParams, sizeof(walkerCodecParams));
    walkerCodecParams.WalkerMode        = m_walkerMode;
    /* looping for Walker is needed at 32x32 block level in OPT case*/
    walkerCodecParams.dwResolutionX     = MOS_ALIGN_CEIL(m_frameWidth,  32) >> 5;
    walkerCodecParams.dwResolutionY     = MOS_ALIGN_CEIL(m_frameHeight, 32) >> 5;
    walkerCodecParams.bNoDependency     = true;

    MHW_WALKER_PARAMS walkerParams;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalInitMediaObjectWalkerParams(
        m_hwInterface,
        &walkerParams,
        &walkerCodecParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_renderEngineInterface->AddMediaObjectWalkerCmd(
        &cmdBuffer,
        &walkerParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(EndKernelCall(
        encFunctionType,
        kernelState,
        &cmdBuffer));

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG9::Encode8x8PUKernel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    PerfTagSetting perfTag;
    CODECHAL_ENCODE_SET_PERFTAG_INFO(perfTag, CODECHAL_ENCODE_PERFTAG_CALL_8X8_PU);

    uint32_t krnIdx = CODECHAL_HEVC_MBENC_8x8PU;
    auto     kernelState  = &m_mbEncKernelStates[krnIdx];
    auto     bindingTable = &m_mbEncKernelBindingTable[krnIdx];
    if (m_firstTaskInPhase || !m_singleTaskPhaseSupported)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(RequestSshAndVerifyCommandBufferSize(kernelState));
    }

    // Setup DSH
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->AssignDshAndSshSpace(
        m_stateHeapInterface,
        kernelState,
        false,
        0,
        false,
        m_storeData));

    // Setup CURBE
    uint32_t                            log2MaxCUSize = m_hevcSeqParams->log2_max_coding_block_size_minus3 + 3;
    CODECHAL_ENC_HEVC_I_8x8_PU_CURBE_G9 cmd, *curbe = &cmd;
    MOS_ZeroMemory(curbe, sizeof(*curbe));

    curbe->DW0.FrameWidth          = MOS_ALIGN_CEIL(m_frameWidth, CODECHAL_MACROBLOCK_WIDTH);
    curbe->DW0.FrameHeight         = MOS_ALIGN_CEIL(m_frameHeight, CODECHAL_MACROBLOCK_HEIGHT);

    curbe->DW1.SliceType            = (m_hevcPicParams->CodingType == I_TYPE) ? CODECHAL_ENCODE_HEVC_I_SLICE : CODECHAL_ENCODE_HEVC_B_SLICE;
    curbe->DW1.PuType          = 2; // 8x8
    curbe->DW1.DcFilterFlag    = true;
    curbe->DW1.AngleRefineFlag = true;
    curbe->DW1.LCUType         = (log2MaxCUSize==6)? 0 /*64x64*/ : 1 /*32x32*/;
    curbe->DW1.ScreenContentFlag    = m_hevcPicParams->bScreenContent;
    curbe->DW1.EnableIntraEarlyExit = (m_hevcSeqParams->TargetUsage == 0x04) ? ((m_hevcPicParams->CodingType == I_TYPE) ? 0 : 1) : 0;
    curbe->DW1.EnableDebugDump = false;
    curbe->DW1.BRCEnable            = m_encodeParams.bMbQpDataEnabled || m_brcEnabled;
    curbe->DW1.LCUBRCEnable         = m_encodeParams.bMbQpDataEnabled || m_lcuBrcEnabled;
    curbe->DW1.ROIEnable            = (m_hevcPicParams->NumROI > 0);
    curbe->DW1.FASTSurveillanceFlag = (m_hevcPicParams->CodingType == I_TYPE) ? 0 : m_hevcSeqParams->bVideoSurveillance;
    if (m_hevcPicParams->bEnableRollingIntraRefresh)
    {
        curbe->DW1.EnableRollingIntra   = true;
        curbe->DW1.IntraRefreshEn       = true;
        curbe->DW1.HalfUpdateMixedLCU   = 0;

        curbe->DW5.IntraRefreshMBNum    = m_hevcPicParams->IntraInsertionLocation;
        curbe->DW5.IntraRefreshQPDelta  = m_hevcPicParams->QpDeltaForInsertedIntra;
        curbe->DW5.IntraRefreshUnitInMB = m_hevcPicParams->IntraInsertionSize;

        int32_t qp = CalSliceQp();
        curbe->DW1.QPValue              = (uint32_t)qp;
    }

    curbe->DW2.LumaLambda      = m_fixedPointLambdaForLuma;

    curbe->DW3.ChromaLambda    = m_fixedPointLambdaForChroma;

    curbe->DW4.HaarTransformFlag       = (m_hevcPicParams->CodingType == I_TYPE) ? false : true;
    curbe->DW4.SimplifiedFlagForInter  = false;

    uint32_t startBTI = 0;
    curbe->DW8.BTI_Src_Y           = bindingTable->dwBindingTableEntries[startBTI++];
    startBTI++; // skip one BTI for Y and UV have the same BTI
    curbe->DW9.BTI_Slice_Map       = bindingTable->dwBindingTableEntries[startBTI++];
    curbe->DW10.BTI_VME_8x8_Mode    = bindingTable->dwBindingTableEntries[startBTI++];
    curbe->DW11.BTI_Intra_Mode     = bindingTable->dwBindingTableEntries[startBTI++];
    curbe->DW12.BTI_BRC_Input      = bindingTable->dwBindingTableEntries[startBTI++];
    curbe->DW13.BTI_Simplest_Intra = bindingTable->dwBindingTableEntries[startBTI++];
    curbe->DW14.BTI_LCU_Qp_Surface = bindingTable->dwBindingTableEntries[startBTI++];
    curbe->DW15.BTI_BRC_Data       = bindingTable->dwBindingTableEntries[startBTI++];
    curbe->DW16.BTI_Debug          = bindingTable->dwBindingTableEntries[startBTI++];

    CODECHAL_ENCODE_ASSERT(startBTI == bindingTable->dwNumBindingTableEntries);

    CODECHAL_MEDIA_STATE_TYPE encFunctionType = CODECHAL_MEDIA_STATE_8x8_PU;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(
        AddCurbeToStateHeap(kernelState, encFunctionType, &cmd, sizeof(cmd)));

    MOS_COMMAND_BUFFER cmdBuffer;
    if(m_numMb8x8IntraKernelSplit == 0)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SendKernelCmdsAndBindingTable(&cmdBuffer,
            kernelState,
            encFunctionType,
            nullptr));
    }
    else
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(GetCommandBuffer(&cmdBuffer));

        MHW_INTERFACE_DESCRIPTOR_PARAMS idParams;
        MOS_ZeroMemory(&idParams, sizeof(idParams));
        idParams.pKernelState = kernelState;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSetInterfaceDescriptor(
            m_stateHeapInterface,
            1,
            &idParams));

        // Add binding table
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSetBindingTable(
            m_stateHeapInterface,
            kernelState));
    }

    //Add surface states
    startBTI = 0;

    // Source Y and UV
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_RAW_Y_UV,
        &bindingTable->dwBindingTableEntries[startBTI++]));
    startBTI++;

    // Slice Map
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_SLICE_MAP,
        &bindingTable->dwBindingTableEntries[startBTI++]));

    // VME 8x8 mode
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_VME_8x8,
        &bindingTable->dwBindingTableEntries[startBTI++]));

    // Intra mode
    m_surfaceParams[SURFACE_INTRA_MODE].bIsWritable   =
    m_surfaceParams[SURFACE_INTRA_MODE].bRenderTarget = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_INTRA_MODE,
        &bindingTable->dwBindingTableEntries[startBTI++]));

    // BRC Input
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_BRC_INPUT,
        &bindingTable->dwBindingTableEntries[startBTI++]));

    // Simplest Intra
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_SIMPLIFIED_INTRA,
        &bindingTable->dwBindingTableEntries[startBTI++]));

    // LCU Qp surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_LCU_QP,
        &bindingTable->dwBindingTableEntries[startBTI++]));

    // BRC data surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_BRC_DATA,
        &bindingTable->dwBindingTableEntries[startBTI++]));

    if (!m_hwWalker)
    {
        eStatus = MOS_STATUS_UNKNOWN;
        CODECHAL_ENCODE_ASSERTMESSAGE("Currently HW walker shall not be disabled for CM based down scaling kernel.");
        return eStatus;
    }

    CODECHAL_WALKER_CODEC_PARAMS walkerCodecParams;
    MOS_ZeroMemory(&walkerCodecParams, sizeof(walkerCodecParams));
    walkerCodecParams.WalkerMode        = m_walkerMode;
    // each EU is based on one 8x8 block
    walkerCodecParams.dwResolutionX     = MOS_ALIGN_CEIL(m_frameWidth,    CODECHAL_MACROBLOCK_WIDTH)  >> 3;
    walkerCodecParams.dwResolutionY     = MOS_ALIGN_CEIL(m_frameHeight,   CODECHAL_MACROBLOCK_HEIGHT) >> 3;
    /* Enforce no dependency dispatch order for 8x8 PU kernel  */
    walkerCodecParams.bNoDependency     = true;

    if(m_numMb8x8IntraKernelSplit == 0)
    {
        MHW_WALKER_PARAMS walkerParams;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalInitMediaObjectWalkerParams(
            m_hwInterface,
            &walkerParams,
            &walkerCodecParams));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_renderEngineInterface->AddMediaObjectWalkerCmd(
            &cmdBuffer,
            &walkerParams));
    }
    else
    {
        uint32_t numRowPerSplit = (walkerCodecParams.dwResolutionY + m_numMb8x8IntraKernelSplit - 1) / m_numMb8x8IntraKernelSplit;
        uint32_t currentNumRow = 0;

        for(uint32_t i = 0; i < m_numMb8x8IntraKernelSplit; i++)
        {
            // Program render engine pipe commands
            SendKernelCmdsParams sendKernelCmdsParams;
            sendKernelCmdsParams                        = SendKernelCmdsParams();
            sendKernelCmdsParams.EncFunctionType        = encFunctionType;
            sendKernelCmdsParams.pKernelState           = kernelState;
            sendKernelCmdsParams.bEnableCustomScoreBoard= true;
            sendKernelCmdsParams.pCustomScoreBoard      = &m_walkingPatternParam.ScoreBoard;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(SendGenericKernelCmds(&cmdBuffer, &sendKernelCmdsParams));

            MHW_WALKER_PARAMS walkerParams;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalInitMediaObjectWalkerParams(
                m_hwInterface,
                &walkerParams,
                &walkerCodecParams));

            if(currentNumRow + numRowPerSplit >= walkerCodecParams.dwResolutionY)
            {
                // the last split may not have the same number of rows as previous splits
                numRowPerSplit = walkerCodecParams.dwResolutionY - currentNumRow;
            }

            walkerParams.LocalStart.y = currentNumRow;
            walkerParams.dwLocalLoopExecCount = numRowPerSplit * walkerCodecParams.dwResolutionX;

            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_renderEngineInterface->AddMediaObjectWalkerCmd(
                &cmdBuffer,
                &walkerParams));

            currentNumRow += numRowPerSplit;
            if(currentNumRow >= walkerCodecParams.dwResolutionY)
            {
                break;
            }
        }
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(EndKernelCall(
        encFunctionType,
        kernelState,
        &cmdBuffer));

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG9::Encode8x8PUFMODEKernel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    PerfTagSetting perfTag;
    CODECHAL_ENCODE_SET_PERFTAG_INFO(perfTag, CODECHAL_ENCODE_PERFTAG_CALL_8X8_FMODE);

    uint32_t krnIdx = CODECHAL_HEVC_MBENC_8x8FMODE;
    auto     kernelState  = &m_mbEncKernelStates[krnIdx];
    auto     bindingTable = &m_mbEncKernelBindingTable[krnIdx];
    if (m_firstTaskInPhase || !m_singleTaskPhaseSupported)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(RequestSshAndVerifyCommandBufferSize(kernelState));
    }

    // Setup DSH
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->AssignDshAndSshSpace(
        m_stateHeapInterface,
        kernelState,
        false,
        0,
        false,
        m_storeData));

    // Setup CURBE
    int32_t qp = CalSliceQp();
    uint32_t sliceQp = (uint32_t)qp;
    uint32_t log2MaxCUSize = m_hevcSeqParams->log2_max_coding_block_size_minus3 + 3;

    CODECHAL_ENC_HEVC_I_8x8_PU_FMODE_CURBE_G9 cmd, *curbe = &cmd;
    MOS_ZeroMemory(curbe, sizeof(*curbe));
    curbe->DW0.FrameWidth                  = MOS_ALIGN_CEIL(m_frameWidth, CODECHAL_MACROBLOCK_WIDTH);
    curbe->DW0.FrameHeight                 = MOS_ALIGN_CEIL(m_frameHeight, CODECHAL_MACROBLOCK_HEIGHT);

    curbe->DW1.SliceType                   = PicCodingTypeToSliceType(m_hevcPicParams->CodingType);
    curbe->DW1.PuType                      = 2;
    curbe->DW1.PakReordingFlag             = (m_hevcPicParams->CodingType == I_TYPE) ? true : false;
    curbe->DW1.LCUType                     = (log2MaxCUSize == 6)? 0 /*64x64*/: 1 /*32x32*/;
    curbe->DW1.ScreenContentFlag           = m_hevcPicParams->bScreenContent;
    curbe->DW1.EnableIntraEarlyExit        = (m_hevcSeqParams->TargetUsage == 0x04) ? ((m_hevcPicParams->CodingType == I_TYPE) ? 0 : 1) : 0;
    curbe->DW1.EnableDebugDump             = false;
    curbe->DW1.BRCEnable                   = m_encodeParams.bMbQpDataEnabled || m_brcEnabled;
    curbe->DW1.LCUBRCEnable                = m_encodeParams.bMbQpDataEnabled || m_lcuBrcEnabled;
    curbe->DW1.ROIEnable                   = (m_hevcPicParams->NumROI > 0);
    curbe->DW1.FASTSurveillanceFlag        = (m_hevcPicParams->CodingType == I_TYPE) ? 0 : m_hevcSeqParams->bVideoSurveillance;
    curbe->DW1.EnableRollingIntra          = m_hevcPicParams->bEnableRollingIntraRefresh;
    curbe->DW1.IntraRefreshEn              = m_hevcPicParams->bEnableRollingIntraRefresh;
    curbe->DW1.HalfUpdateMixedLCU          = 0;
    curbe->DW2.LambdaForLuma               = m_fixedPointLambdaForLuma;

    if (m_hevcPicParams->CodingType != I_TYPE)
    {
        float hadBias = 2.0f;

        double lambdaMd = m_qpLambdaMd[curbe->DW1.SliceType][sliceQp];
        lambdaMd = lambdaMd * hadBias;
        curbe->DW3.LambdaForDistCalculation = (uint32_t)(lambdaMd*(1<<10));
    }
    curbe->DW4.ModeCostFor8x8PU_TU8      = 0;
    curbe->DW5.ModeCostFor8x8PU_TU4      = 0;
    curbe->DW6.SATD16x16PuThreshold      = MOS_MAX(200 * ((int32_t)sliceQp - 12), 0);
    curbe->DW6.BiasFactorToward8x8       = (m_hevcPicParams->bScreenContent) ? 1024 : 1126 + 102;
    curbe->DW7.Qp                        = sliceQp;
    curbe->DW7.QpForInter                = 0;
    curbe->DW8.SimplifiedFlagForInter    = false;
    // KBLControlFlag determines the PAK OBJ format as it varies from Gen9 to Gen9.5+
    curbe->DW8.KBLControlFlag            = UsePlatformControlFlag();
    curbe->DW9.IntraRefreshMBNum         = m_hevcPicParams->IntraInsertionLocation;
    curbe->DW9.IntraRefreshQPDelta       = m_hevcPicParams->QpDeltaForInsertedIntra;
    curbe->DW9.IntraRefreshUnitInMB      = m_hevcPicParams->IntraInsertionSize;

    uint32_t startBTI = 0;
    curbe->DW16.BTI_PAK_Object           = bindingTable->dwBindingTableEntries[startBTI++];
    curbe->DW17.BTI_VME_8x8_Mode         = bindingTable->dwBindingTableEntries[startBTI++];
    curbe->DW18.BTI_Intra_Mode           = bindingTable->dwBindingTableEntries[startBTI++];
    curbe->DW19.BTI_PAK_Command          = bindingTable->dwBindingTableEntries[startBTI++];
    curbe->DW20.BTI_Slice_Map            = bindingTable->dwBindingTableEntries[startBTI++];
    curbe->DW21.BTI_IntraDist            = bindingTable->dwBindingTableEntries[startBTI++];
    curbe->DW22.BTI_BRC_Input            = bindingTable->dwBindingTableEntries[startBTI++];
    curbe->DW23.BTI_Simplest_Intra       = bindingTable->dwBindingTableEntries[startBTI++];
    curbe->DW24.BTI_LCU_Qp_Surface       = bindingTable->dwBindingTableEntries[startBTI++];
    curbe->DW25.BTI_BRC_Data             = bindingTable->dwBindingTableEntries[startBTI++];
    curbe->DW26.BTI_Haar_Dist16x16       = bindingTable->dwBindingTableEntries[startBTI++];
    curbe->DW27.BTI_Stats_Data           = bindingTable->dwBindingTableEntries[startBTI++];
    curbe->DW28.BTI_Frame_Stats_Data     = bindingTable->dwBindingTableEntries[startBTI++];
    curbe->DW29.BTI_Debug                = bindingTable->dwBindingTableEntries[startBTI++];

    CODECHAL_ENCODE_ASSERT(startBTI == bindingTable->dwNumBindingTableEntries);

    CODECHAL_MEDIA_STATE_TYPE encFunctionType = CODECHAL_MEDIA_STATE_8x8_PU_FMODE;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(
        AddCurbeToStateHeap(kernelState, encFunctionType, &cmd, sizeof(cmd)));

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendKernelCmdsAndBindingTable(
        &cmdBuffer,
        kernelState,
        encFunctionType,
        nullptr));

    //Add surface states
    startBTI = 0;

    // PAK object
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_CU_RECORD,
        &bindingTable->dwBindingTableEntries[startBTI++]));

    // VME 8x8 mode
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_VME_8x8,
        &bindingTable->dwBindingTableEntries[startBTI++]));

    // Intra mode
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_INTRA_MODE,
        &bindingTable->dwBindingTableEntries[startBTI++]));

    // PAK command
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_HCP_PAK,
        &bindingTable->dwBindingTableEntries[startBTI++]));

    // Slice Map
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_SLICE_MAP,
        &bindingTable->dwBindingTableEntries[startBTI++]));

    // Intra dist
    m_surfaceParams[SURFACE_INTRA_DIST].bIsWritable   =
    m_surfaceParams[SURFACE_INTRA_DIST].bRenderTarget = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_INTRA_DIST,
        &bindingTable->dwBindingTableEntries[startBTI++]));

    // BRC Input
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_BRC_INPUT,
        &bindingTable->dwBindingTableEntries[startBTI++]));

    // Simplest Intra
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_SIMPLIFIED_INTRA,
        &bindingTable->dwBindingTableEntries[startBTI++]));

    // LCU Qp surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_LCU_QP,
        &bindingTable->dwBindingTableEntries[startBTI++]));

    // BRC data surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_BRC_DATA,
        &bindingTable->dwBindingTableEntries[startBTI++]));

    if (!m_hwWalker)
    {
        eStatus = MOS_STATUS_UNKNOWN;
        CODECHAL_ENCODE_ASSERTMESSAGE("Currently HW walker shall not be disabled for CM based down scaling kernel.");
        return eStatus;
    }

    CODECHAL_WALKER_CODEC_PARAMS walkerCodecParams;
    MOS_ZeroMemory(&walkerCodecParams, sizeof(walkerCodecParams));
    walkerCodecParams.WalkerMode        = m_walkerMode;
    // each EU is based on one LCU
    walkerCodecParams.dwResolutionX     = MOS_ALIGN_CEIL(m_frameWidth,    (1<<log2MaxCUSize)) >> log2MaxCUSize;
    walkerCodecParams.dwResolutionY     = MOS_ALIGN_CEIL(m_frameHeight,   (1<<log2MaxCUSize)) >> log2MaxCUSize;
    /* Enforce no dependency dispatch order for 8x8 PU FMODE kernel  */
    walkerCodecParams.bNoDependency     = true;

    MHW_WALKER_PARAMS walkerParams;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalInitMediaObjectWalkerParams(
        m_hwInterface,
        &walkerParams,
        &walkerCodecParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_renderEngineInterface->AddMediaObjectWalkerCmd(
        &cmdBuffer,
        &walkerParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(EndKernelCall(
        encFunctionType,
        kernelState,
        &cmdBuffer));

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG9::EncodeDSCombinedKernel(
    DsStage downScaleStage,
    uint32_t index,
    uint32_t refListIdx)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (m_scalingEnabled)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_trackedBuf->AllocateSurfaceDS());
    }

    PerfTagSetting perfTag;
    perfTag.CallType = m_singleTaskPhaseSupported ? CODECHAL_ENCODE_PERFTAG_CALL_SCALING_KERNEL :
        CODECHAL_ENCODE_PERFTAG_CALL_DS_CONVERSION_KERNEL;
    CODECHAL_ENCODE_SET_PERFTAG_INFO(perfTag, perfTag.CallType);

    uint32_t krnIdx = CODECHAL_HEVC_MBENC_DS_COMBINED;
    auto     kernelState  = &m_mbEncKernelStates[krnIdx];
    auto     bindingTable = &m_mbEncKernelBindingTable[krnIdx];
    if (m_firstTaskInPhase || !m_singleTaskPhaseSupported)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(RequestSshAndVerifyCommandBufferSize(kernelState));
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalGetResourceInfo(
        m_osInterface,
        &m_scaled2xSurface));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalGetResourceInfo(
        m_osInterface,
        &m_formatConvertedSurface[index]));

    //Setup DSH
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->AssignDshAndSshSpace(
        m_stateHeapInterface,
        kernelState,
        false,
        0,
        false,
        m_storeData));

    //Setup Scaling CURBE
    CODECHAL_ENC_HEVC_DS_COMBINED_CURBE_G9 cmd, *curbe = &cmd;

    MOS_ZeroMemory(curbe, sizeof(*curbe));
    curbe->DW0.Pak_BitDepth_Chroma = 10;
    curbe->DW0.Pak_BitDepth_Luma = 10;
    curbe->DW0.Enc_BitDepth_Chroma = 8;
    curbe->DW0.Enc_BitDepth_Luma = 8;
    curbe->DW0.Rounding_Value = 1;

    curbe->DW1.PicFormat = 0;
    curbe->DW1.PicConvertFlag = 1;
    curbe->DW1.PicDownscale = downScaleStage;//Downscale stage
    curbe->DW1.PicMBStatOutputCntrl = 0;

    curbe->DW2.OrigPicWidth = m_frameWidth;
    curbe->DW2.OrigPicHeight = m_frameHeight;

    uint32_t startBTI = 0;
    curbe->DW3.BTI_Surface_P010 = bindingTable->dwBindingTableEntries[startBTI];
    startBTI += 2;   // increment by no of planes
    curbe->DW4.BTI_Surface_NV12 = bindingTable->dwBindingTableEntries[startBTI];
    startBTI += 2;  // increment by no of planes
    curbe->DW5.BTI_Src_Y_4xDownScaled = bindingTable->dwBindingTableEntries[startBTI++];
    curbe->DW6.BTI_Surf_MBState = bindingTable->dwBindingTableEntries[startBTI++];
    curbe->DW7.BTI_Src_Y_2xDownScaled = bindingTable->dwBindingTableEntries[startBTI++];

    CODECHAL_MEDIA_STATE_TYPE encFunctionType = CODECHAL_MEDIA_STATE_2X_SCALING;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(
        AddCurbeToStateHeap(kernelState, encFunctionType, &cmd, sizeof(cmd))
    );

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendKernelCmdsAndBindingTable(
        &cmdBuffer,
        kernelState,
        encFunctionType,
        nullptr));

    // Add surface states, 2X scaling uses U32Norm surface format for destination
    startBTI = 0;

    if (index == 0)
    {
        // Source surface/s  -- 10 bit YUV
        m_surfaceParams[SURFACE_RAW_10bit_Y_UV].bUseUVPlane = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
            kernelState,
            &cmdBuffer,
            SURFACE_RAW_10bit_Y_UV,
            &bindingTable->dwBindingTableEntries[startBTI],
            m_rawSurfaceToEnc
        ));
    }
    else
    {
        // Source surface/s  -- 10 bit YUV
        m_surfaceParams[SURFACE_RAW_10bit_Y_UV].bUseUVPlane = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
            kernelState,
            &cmdBuffer,
            SURFACE_RAW_10bit_Y_UV,
            &bindingTable->dwBindingTableEntries[startBTI],
            &(m_refList[refListIdx]->sRefReconBuffer)));
    }
    startBTI += 2; // advance binding table pointer to next surface setting

    // Destination surface/s  -- 8 bit Format converted surface
    m_formatConvertedSurface[index].dwWidth                            = m_frameWidth;
    m_formatConvertedSurface[index].dwHeight                           = m_frameHeight;
    m_surfaceParams[SURFACE_RAW_FC_8bit_Y_UV].bUse32UnormSurfaceFormat = false;
    m_surfaceParams[SURFACE_RAW_FC_8bit_Y_UV].bUse16UnormSurfaceFormat = false;
    m_surfaceParams[SURFACE_RAW_FC_8bit_Y_UV].bUseUVPlane = true;
    m_surfaceParams[SURFACE_RAW_FC_8bit_Y_UV].bIsWritable =
        m_surfaceParams[SURFACE_RAW_FC_8bit_Y_UV].bRenderTarget = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_RAW_FC_8bit_Y_UV,
        &bindingTable->dwBindingTableEntries[startBTI],
        &m_formatConvertedSurface[index]));

    startBTI += 2;

    // Destination surface/s  -- 4x downscaled luma only
    m_surfaceParams[SURFACE_Y_4X].bUse32UnormSurfaceFormat =
        m_surfaceParams[SURFACE_Y_4X].bIsWritable =
        m_surfaceParams[SURFACE_Y_4X].bRenderTarget = true;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_Y_4X,
        &bindingTable->dwBindingTableEntries[startBTI],
        m_trackedBuf->Get4xDsSurface(CODEC_CURR_TRACKED_BUFFER)));

    startBTI++;

    //Destination Surface  -- MB Stat surface 1D buffer
    m_surfaceParams[SURFACE_RAW_MBSTAT].bUse32UnormSurfaceFormat = false;
    m_surfaceParams[SURFACE_RAW_MBSTAT].bUse16UnormSurfaceFormat = false;
    m_surfaceParams[SURFACE_RAW_MBSTAT].bIsWritable =
        m_surfaceParams[SURFACE_RAW_MBSTAT].bRenderTarget = true;
    m_surfaceParams[SURFACE_RAW_MBSTAT].dwSize            = m_resMbStatisticsSurface.dwSize;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_RAW_MBSTAT,
        &bindingTable->dwBindingTableEntries[startBTI],
        &m_resMbStatisticsSurface.sResource));

    startBTI++;

    // Destination surface/s  -- 2x downscaled luma only
    m_scaled2xSurface.dwWidth = MOS_ALIGN_CEIL((m_frameWidth / SCALE_FACTOR_2x), (CODECHAL_MACROBLOCK_WIDTH * 2));
    m_scaled2xSurface.dwHeight = MOS_ALIGN_CEIL((m_frameHeight / SCALE_FACTOR_2x), (CODECHAL_MACROBLOCK_HEIGHT * 2));

    m_surfaceParams[SURFACE_Y_2X].bUse32UnormSurfaceFormat =
        m_surfaceParams[SURFACE_Y_2X].bIsWritable =
        m_surfaceParams[SURFACE_Y_2X].bRenderTarget = true;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
        kernelState,
        &cmdBuffer,
        SURFACE_Y_2X,
        &bindingTable->dwBindingTableEntries[startBTI]
    ));

    //move back to 16 aligned..
    m_scaled2xSurface.dwWidth = MOS_ALIGN_CEIL((m_frameWidth / SCALE_FACTOR_2x), (CODECHAL_MACROBLOCK_WIDTH));
    m_scaled2xSurface.dwHeight = MOS_ALIGN_CEIL((m_frameHeight / SCALE_FACTOR_2x), (CODECHAL_MACROBLOCK_HEIGHT));

    m_surfaceParams[SURFACE_Y_2X].bUse16UnormSurfaceFormat =
        m_surfaceParams[SURFACE_Y_2X].bIsWritable =
        m_surfaceParams[SURFACE_Y_2X].bRenderTarget = true;

    if (!m_hwWalker)
    {
        eStatus = MOS_STATUS_UNKNOWN;
        CODECHAL_ENCODE_ASSERTMESSAGE("Currently HW walker shall not be disabled for CM based down scaling kernel.");
        return eStatus;
    }

    CODECHAL_WALKER_CODEC_PARAMS walkerCodecParams;
    MOS_ZeroMemory(&walkerCodecParams, sizeof(walkerCodecParams));

    /* first stage of the downscale and convert kernel can do conversion + 4x + 2x */
    walkerCodecParams.WalkerMode = m_walkerMode;
    walkerCodecParams.dwResolutionX = MOS_ALIGN_CEIL((m_frameWidth >> 2), 32) >> 3;
    walkerCodecParams.dwResolutionY = MOS_ALIGN_CEIL((m_frameHeight >> 2), 32) >> 3;

    /* Enforce no dependency dispatch order for Scaling kernel,  */
    walkerCodecParams.bNoDependency = true;

    MHW_WALKER_PARAMS walkerParams;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalInitMediaObjectWalkerParams(
        m_hwInterface,
        &walkerParams,
        &walkerCodecParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_renderEngineInterface->AddMediaObjectWalkerCmd(
        &cmdBuffer,
        &walkerParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(EndKernelCall(
        encFunctionType,
        kernelState,
        &cmdBuffer));

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG9::EncodeDSKernel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    // Walker must be used for HME call and scaling one
    CODECHAL_ENCODE_ASSERT(m_hwWalker);

    //perform 4x down-scaling
    if (MEDIA_IS_SKU(m_hwInterface->GetSkuTable(), FtrEncodeHEVC10bit) && (m_hevcSeqParams->bit_depth_luma_minus8) && m_scalingEnabled)
    {
        m_lastTaskInPhase = !(m_16xMeSupported || m_hmeEnabled || m_brcEnabled);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(EncodeDSCombinedKernel(dsStage2x4x, 0, 0));

        //Dump format converted input surface
        CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
            &m_formatConvertedSurface[0],
            CodechalDbgAttr::attrEncodeRawInputSurface,
            "SrcSurf")));

        //Scaled surface
        CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
            m_trackedBuf->Get4xDsSurface(CODEC_CURR_TRACKED_BUFFER),
            CodechalDbgAttr::attrEncodeRawInputSurface,
            "SrcSurf")));

        //Scaled surface
        CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
            &m_scaled2xSurface,
            CodechalDbgAttr::attrEncodeRawInputSurface,
            "SrcSurf")));

        // call 16x/32x DS
        if (m_16xMeSupported)
        {
            m_lastTaskInPhase = !(m_32xMeSupported || m_hmeEnabled || m_brcEnabled);

            // 4x downscaled images used as the input for 16x downscaling
            CodechalEncodeCscDs::KernelParams cscScalingKernelParams;
            MOS_ZeroMemory(&cscScalingKernelParams, sizeof(cscScalingKernelParams));
            cscScalingKernelParams.b16xScalingInUse = true;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cscDsState->DsKernel(&cscScalingKernelParams));

            if (m_32xMeSupported)
            {
                m_lastTaskInPhase = !(m_hmeEnabled || m_brcEnabled);

                // 16x downscaled images used as the input for 32x downscaling
                cscScalingKernelParams.b32xScalingInUse = true;
                cscScalingKernelParams.b16xScalingInUse = false;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cscDsState->DsKernel(&cscScalingKernelParams));
            }
        }
    }
    else
    {
        // Csc, Downscaling, and/or 10-bit to 8-bit conversion
        CODECHAL_ENCODE_CHK_NULL_RETURN(m_cscDsState);

        CodechalEncodeCscDs::KernelParams cscScalingKernelParams;
        MOS_ZeroMemory(&cscScalingKernelParams, sizeof(cscScalingKernelParams));
        cscScalingKernelParams.bLastTaskInPhaseCSC =
            cscScalingKernelParams.bLastTaskInPhase4xDS = !(m_16xMeSupported || m_hmeEnabled || m_brcEnabled);
        cscScalingKernelParams.bLastTaskInPhase16xDS    = !(m_32xMeSupported || m_hmeEnabled || m_brcEnabled);
        cscScalingKernelParams.bLastTaskInPhase32xDS    = !(m_hmeEnabled || m_brcEnabled);

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cscDsState->KernelFunctions(&cscScalingKernelParams));
    }

    // wait on the current MbCode object if needed
    if (m_hevcPicParams->bUsedAsRef || (m_brcEnabled && !m_hevcSeqParams->ParallelBRC))
    {
        m_currRefSync = &m_refSync[m_currMbCodeIdx];

        // Check if the signal obj has been used before
        if (m_currRefSync->uiSemaphoreObjCount || m_currRefSync->bInUsed)
        {
            MOS_SYNC_PARAMS syncParams = g_cInitSyncParams;
            syncParams.GpuContext = m_renderContext;
            syncParams.presSyncResource = &m_currRefSync->resSyncObject;
            syncParams.uiSemaphoreCount = m_currRefSync->uiSemaphoreObjCount;

            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineWait(m_osInterface, &syncParams));
            m_currRefSync->uiSemaphoreObjCount = 0;
            m_currRefSync->bInUsed             = false;
        }
    }
    else
    {
        m_currRefSync = nullptr;
    }

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG9::EncodeKernelFunctions()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_DEBUG_TOOL(
        CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
            m_rawSurfaceToEnc,
            CodechalDbgAttr::attrEncodeRawInputSurface,
            "SrcSurf")));
    )

    if (m_pakOnlyTest)
    {
        // Skip all ENC kernel operations for now it is in the PAK only test mode.
        // PAK and CU records will be passed via the app
        return eStatus;
    }

    UpdateSSDSliceCount();

    // BRC init/reset needs to be called before HME since it will reset the Brc Distortion surface
    if (m_brcEnabled && (m_brcInit || m_brcReset))
    {
        m_firstTaskInPhase = m_lastTaskInPhase = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(EncodeBrcInitResetKernel());
        m_brcInit = m_brcReset = false;
    }

    // Scaled surfaces are required to run both HME and IFrameDist
    bool scalingEnabled = (m_hmeSupported || m_brcEnabled);
    if (scalingEnabled || m_cscDsState->RequireCsc())
    {
        //Use a different performance tag ID for scaling and HME
        m_osInterface->pfnResetPerfBufferID(m_osInterface);

        m_firstTaskInPhase = true;
        m_lastTaskInPhase  = false;

        if(m_hevcSeqParams->GopPicSize != 1 || m_brcEnabled || m_cscDsState->RequireCsc())
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(EncodeDSKernel());
        }

        if (m_brcEnabled)
        {
            // LCU-based BRC update kernel needs both intra and inter (from HME) distortion
            m_lastTaskInPhase = (m_pictureCodingType == I_TYPE);
            CODECHAL_ENCODE_CHK_STATUS_RETURN(EncodeCoarseIntra16x16Kernel());
        }

        // only need to call HME kernel when HME enabled and NOT I-frame
        if (m_hmeEnabled)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(EncodeMeKernel());
        }
    }

    if(m_osInterface->bSimIsActive)
    {
        // Clean MB code buffer to ensure there is no previous CU record and PAK command
        MOS_LOCK_PARAMS lockFlags;
        MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
        lockFlags.WriteOnly = 1;

        uint8_t*  data = (uint8_t* )m_osInterface->pfnLockResource(m_osInterface, &m_resMbCodeSurface, &lockFlags);
        if (data)
        {
            MOS_ZeroMemory(data, m_mbCodeSize);
            m_osInterface->pfnUnlockResource(m_osInterface, &m_resMbCodeSurface);
        }
    }

    // Generate slice map for kernel
    CODECHAL_ENCODE_CHK_STATUS_RETURN(GenerateSliceMap());

    //Reset to use a different performance tag ID for I kernels. Each kernel has a different buffer ID
    m_osInterface->pfnResetPerfBufferID(m_osInterface);

    m_firstTaskInPhase = true;
    m_lastTaskInPhase  = false;

    // ROI uses the BRC LCU update kernel, even in CQP.  So we will call it
    // first if in CQP.  It has no other kernel execution dependencies, even
    // that brc is not initialized is not a dependency
    if (m_hevcPicParams->NumROI && !m_brcEnabled)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(EncodeBrcUpdateLCUBasedKernel(nullptr));
    }

    /* When TU=7, fast encoding mode is ON, and I kernels are not needed.
    Instead, MB ENC B kernel is used to replace I kernels.
    */
    bool fastEncodingFlag  = (m_hevcSeqParams->TargetUsage == 0x7);
    bool brcUpdateComplete = false;

    if(fastEncodingFlag)
    {
        if (m_hevcPicParams->CodingType == I_TYPE)
        {
            // BRC and MbEnc are included in the same task phase
            if (m_brcEnabled && !brcUpdateComplete)
            {
                // BRC needs previous PAK result if not running in the parallel BRC mode
                // If yes, BRC is using the PAk result of the frame before the previous one
                CODECHAL_ENCODE_CHK_STATUS_RETURN(WaitForPak());

                CODECHAL_ENCODE_CHK_STATUS_RETURN(EncodeBrcUpdateKernel());

                // Reset buffer ID used for BRC kernel performance reports
                m_osInterface->pfnResetPerfBufferID(m_osInterface);
                brcUpdateComplete = true;
            }
            else if (!m_brcEnabled)
            {
                if (m_encodeParams.bMbQpDataEnabled && m_encodeParams.psMbQpDataSurface)
                {
                    auto brcConstantData = &m_brcBuffers.sBrcConstantDataBuffer[m_currRecycledBufIdx];
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetupBrcConstantTable(brcConstantData));
                    Convert1byteTo2bytesQPperLCU(m_encodeParams.psMbQpDataSurface, &m_brcBuffers.sBrcMbQpBuffer);
                }
            }

            CODECHAL_ENCODE_CHK_STATUS_RETURN(Encode8x8PBMbEncKernel());
        }
    }
    else
    {
        // BRC and MbEnc are included in the same task phase
        if (m_brcEnabled && !brcUpdateComplete)
        {
            // BRC needs previous PAK result
            CODECHAL_ENCODE_CHK_STATUS_RETURN(WaitForPak());

            CODECHAL_ENCODE_CHK_STATUS_RETURN(EncodeBrcUpdateKernel());

            // Reset buffer ID used for BRC kernel performance reports
            m_osInterface->pfnResetPerfBufferID(m_osInterface);
            brcUpdateComplete = true;
        }
        else if (!m_brcEnabled)
        {
            if (m_encodeParams.bMbQpDataEnabled && m_encodeParams.psMbQpDataSurface)
            {
                auto brcConstantData = &m_brcBuffers.sBrcConstantDataBuffer[m_currRecycledBufIdx];
                CODECHAL_ENCODE_CHK_STATUS_RETURN(SetupBrcConstantTable(brcConstantData));
                Convert1byteTo2bytesQPperLCU(m_encodeParams.psMbQpDataSurface, &m_brcBuffers.sBrcMbQpBuffer);
            }
        }

        //Step 1: perform 2:1 down-scaling
        if (m_hevcSeqParams->bit_depth_luma_minus8 == 0)  // use this for 8 bit only case.
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(Encode2xScalingKernel());
        }

        //Step 2: 32x32 PU Mode Decision or 32x32 PU Intra check kernel
        if (m_hevcPicParams->CodingType == I_TYPE)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(Encode32x32PuModeDecisionKernel());
        }
        else
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(Encode32X32BIntraCheckKernel());
        }

        //Step 3: 16x16 SAD Computation
        CODECHAL_ENCODE_CHK_STATUS_RETURN(Encode16x16SadPuComputationKernel());

        CODECHAL_DEBUG_TOOL(
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                &m_sad16x16Pu.sResource,
                CodechalDbgAttr::attrOutput,
                "HEVC_16x16_PU_SAD_Out",
                m_sad16x16Pu.dwSize,
                0,
                CODECHAL_MEDIA_STATE_16x16_PU_SAD));
        )

        //Step 4: 16x16 PU Mode Decision
        CODECHAL_ENCODE_CHK_STATUS_RETURN(Encode16x16PuModeDecisionKernel());

        CODECHAL_DEBUG_TOOL(
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                &m_vme8x8Mode.sResource,
                CodechalDbgAttr::attrOutput,
                "HEVC_16x16_PU_MD_Out",
                m_vme8x8Mode.dwSize,
                0,
                CODECHAL_MEDIA_STATE_16x16_PU_MODE_DECISION));
        )

        //Step 5: 8x8 PU
        CODECHAL_ENCODE_CHK_STATUS_RETURN(Encode8x8PUKernel());

        //Step 6: 8x8 PU FMODE
        m_lastTaskInPhase = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(Encode8x8PUFMODEKernel());

        CODECHAL_DEBUG_TOOL(
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
                &m_scaled2xSurface,
                CodechalDbgAttr::attrReferenceSurfaces,
                "2xScaledSurf"));

            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpSurface(
                &m_simplestIntraSurface,
                CodechalDbgAttr::attrOutput,
                "HEVC_32x32_SIF_Out",
                CODECHAL_MEDIA_STATE_32x32_B_INTRA_CHECK));

            if (m_pictureCodingType == I_TYPE)
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                    &m_32x32PuOutputData.sResource,
                    CodechalDbgAttr::attrOutput,
                    "HEVC_32x32_PU_MD_Out",
                    m_32x32PuOutputData.dwSize,
                    0,
                    CODECHAL_MEDIA_STATE_32x32_PU_MODE_DECISION));
            }
            else
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                    &m_32x32PuOutputData.sResource,
                    CodechalDbgAttr::attrOutput,
                    "HEVC_32x32_B_INTRA_CHECK_Out",
                    m_32x32PuOutputData.dwSize,
                    0,
                    CODECHAL_MEDIA_STATE_32x32_PU_MODE_DECISION));

            }

            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                &m_intraMode.sResource,
                CodechalDbgAttr::attrOutput,
                "HEVC_8x8_PU_MD_Out",
                m_intraMode.dwSize,
                0,
                CODECHAL_MEDIA_STATE_8x8_PU));

            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                &m_intraDist.sResource,
                CodechalDbgAttr::attrOutput,
                "HEVC_8x8_PU_FMOD_Out",
                m_intraDist.dwSize,
                0,
                CODECHAL_MEDIA_STATE_8x8_PU_FMODE));
        )
    }

    // Sync-wait can be executed after I-kernel is submitted before there is no dependency for I to wait for PAK to be ready
    CODECHAL_ENCODE_CHK_STATUS_RETURN(WaitForPak());

    //Step 7: B MB ENC kernel for B picture only
    if (m_hevcPicParams->CodingType != I_TYPE)
    {
        m_firstTaskInPhase = true;
        m_lastTaskInPhase = false;

        // BRC and MbEnc are included in the same task phase
        if (m_brcEnabled && !brcUpdateComplete)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(EncodeBrcUpdateKernel());

            // Reset buffer ID used for BRC kernel performance reports
            m_osInterface->pfnResetPerfBufferID(m_osInterface);
            brcUpdateComplete = true;
        }
        else if (!m_brcEnabled)
        {
            if (m_encodeParams.bMbQpDataEnabled && m_encodeParams.psMbQpDataSurface)
            {
                auto brcConstantData = &m_brcBuffers.sBrcConstantDataBuffer[m_currRecycledBufIdx];
                CODECHAL_ENCODE_CHK_STATUS_RETURN(SetupBrcConstantTable(brcConstantData));
                Convert1byteTo2bytesQPperLCU(m_encodeParams.psMbQpDataSurface, &m_brcBuffers.sBrcMbQpBuffer);
            }
        }

        if ((m_hevcSeqParams->bit_depth_luma_minus8))
        {
            bool formatConversionDone[NUM_FORMAT_CONV_FRAMES] = { false };
            formatConversionDone[0] = true; // always true since its for the input surface.

            for (auto i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
            {
                if (!m_picIdx[i].bValid || !m_currUsedRefPic[i])
                {
                    continue;
                }

                uint8_t picIdx = m_picIdx[i].ucPicIdx;
                CODECHAL_ENCODE_ASSERT(picIdx < 127);

                uint8_t frameStoreId = (uint8_t)m_refIdxMapping[i];

                if (frameStoreId >= CODECHAL_MAX_CUR_NUM_REF_FRAME_HEVC)
                {
                    CODECHAL_ENCODE_ASSERT(false);
                    eStatus = MOS_STATUS_INVALID_PARAMETER;
                    return eStatus;
                }

                if (formatConversionDone[frameStoreId + 1] != true)
                {
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(EncodeDSCombinedKernel(dsDisabled, (frameStoreId + 1), picIdx));
                    formatConversionDone[frameStoreId + 1] = true;
                    m_refList[picIdx]->sRefBuffer          = m_formatConvertedSurface[frameStoreId + 1];
                }
            }
        }

        CODECHAL_ENCODE_CHK_STATUS_RETURN(Encode8x8PBMbEncKernel());
    }

    // Notify PAK engine once ENC is done
    if (!m_pakOnlyTest && !Mos_ResourceIsNull(&m_resSyncObjectRenderContextInUse))
    {
        MOS_SYNC_PARAMS syncParams = g_cInitSyncParams;
        syncParams.GpuContext = m_renderContext;
        syncParams.presSyncResource = &m_resSyncObjectRenderContextInUse;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineSignal(m_osInterface, &syncParams));
    }

    if (m_brcEnabled && m_hevcSeqParams->ParallelBRC)
    {
        m_brcBuffers.uiCurrBrcPakStasIdxForRead = (m_brcBuffers.uiCurrBrcPakStasIdxForRead + 1) % CODECHAL_ENCODE_RECYCLED_BUFFER_NUM;
    }

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG9::CheckBrcPakStasBuffer(
    PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);

    auto brcPakStas = &m_brcBuffers.resBrcPakStatisticBuffer[m_brcBuffers.uiCurrBrcPakStasIdxForRead];

    /*
    1. The following assembly code is used to implement the following C statements.

            if( ((MHW_VDBOX_IMAGE_STATUS_CONTROL*)&(p->HCP_IMAGE_STATUS_CONTROL))->hcpCumulativeFrameDeltaQp <
                ((MHW_VDBOX_IMAGE_STATUS_CONTROL*)&(p->HCP_IMAGE_STATUS_CONTROL_FOR_LAST_PASS))->hcpCumulativeFrameDeltaQp)
            {
                (MHW_VDBOX_IMAGE_STATUS_CONTROL*)&(p->HCP_IMAGE_STATUS_CONTROL))->hcpCumulativeFrameDeltaQp =
                    MHW_VDBOX_IMAGE_STATUS_CONTROL*)&(p->HCP_IMAGE_STATUS_CONTROL_FOR_LAST_PASS))->hcpCumulativeFrameDeltaQp;
            }

    2. The if statement can be replaced by and-or statements. That is,
            (a) a = (a < b) ? b : a;
            (b) mask = (a - b) >> 32; a = (b & mask) | (a & !mask);
            where (a) and (b) are identical and each variable is assumed to be a 64-bit unsigned integer

    3. Totally there are 71 DWs
    */
    if(cmdBuffer->iRemaining < 71 * sizeof(uint32_t))
    {
        eStatus = MOS_STATUS_NO_SPACE;
        return eStatus;
    }

    // reg0 = p->HCP_IMAGE_STATUS_CONTROL
    MHW_MI_LOAD_REGISTER_MEM_PARAMS miLoadRegMemParams;
    miLoadRegMemParams.presStoreBuffer = brcPakStas;
    miLoadRegMemParams.dwOffset = CODECHAL_OFFSETOF(CODECHAL_ENCODE_HEVC_PAK_STATS_BUFFER, HCP_IMAGE_STATUS_CONTROL);
    miLoadRegMemParams.dwRegister = CS_GPR_REGISTER_INDEX(0);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiLoadRegisterMemCmd(cmdBuffer, &miLoadRegMemParams));

    MHW_MI_LOAD_REGISTER_IMM_PARAMS miLoadRegImmParams;
    miLoadRegImmParams.dwData = 0;
    miLoadRegImmParams.dwRegister = (CS_GPR_REGISTER_INDEX(0) + 4);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiLoadRegisterImmCmd(cmdBuffer, &miLoadRegImmParams));

    // reg1 = p->HCP_IMAGE_STATUS_CONTROL_FOR_LAST_PASS
    miLoadRegMemParams.presStoreBuffer = brcPakStas;
    miLoadRegMemParams.dwOffset = CODECHAL_OFFSETOF(CODECHAL_ENCODE_HEVC_PAK_STATS_BUFFER, HCP_IMAGE_STATUS_CONTROL_FOR_LAST_PASS);
    miLoadRegMemParams.dwRegister = CS_GPR_REGISTER_INDEX(1);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiLoadRegisterMemCmd(cmdBuffer, &miLoadRegMemParams));
    miLoadRegImmParams.dwData = 0;
    miLoadRegImmParams.dwRegister = (CS_GPR_REGISTER_INDEX(1) + 4);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiLoadRegisterImmCmd(cmdBuffer, &miLoadRegImmParams));

    // reg2 = 0xFF000000
    miLoadRegImmParams.dwData = 0xFF000000;
    miLoadRegImmParams.dwRegister = CS_GPR_REGISTER_INDEX(2);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiLoadRegisterImmCmd(cmdBuffer, &miLoadRegImmParams));
    miLoadRegImmParams.dwData = 0;
    miLoadRegImmParams.dwRegister = (CS_GPR_REGISTER_INDEX(2) + 4);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiLoadRegisterImmCmd(cmdBuffer, &miLoadRegImmParams));

    // reg3 = reg0 & 0xFF000000
    uint32_t csALUCmdNum = 0;
    MHW_MI_ALU_PARAMS miAluParams[64] = { 0 };

    // reg3 = reg0 & 0xFF000000
    miAluParams[csALUCmdNum++].Value = CS_ALU_COMMAND_LOAD(1, 0);     // load     srcA, reg0
    miAluParams[csALUCmdNum++].Value = CS_ALU_COMMAND_LOAD(0, 2);     // load     srcB, reg2
    miAluParams[csALUCmdNum++].Value = CS_ALU_COMMAND_AND;            // and      srcA, srcB
    miAluParams[csALUCmdNum++].Value = CS_ALU_COMMAND_STORE_ACCU(3);  // store    reg3, alu

    // reg4 = reg1 & 0xFF000000
    miAluParams[csALUCmdNum++].Value = CS_ALU_COMMAND_LOAD(1, 1);     // load     srcA, reg1
    miAluParams[csALUCmdNum++].Value = CS_ALU_COMMAND_LOAD(0, 2);     // load     srcB, reg2
    miAluParams[csALUCmdNum++].Value = CS_ALU_COMMAND_AND;            // and      srcA, srcB
    miAluParams[csALUCmdNum++].Value = CS_ALU_COMMAND_STORE_ACCU(4);  // store    reg4, alu

    // reg5 = reg3 - reg4
    miAluParams[csALUCmdNum++].Value = CS_ALU_COMMAND_LOAD(1, 3);     // load     srcA, reg3
    miAluParams[csALUCmdNum++].Value = CS_ALU_COMMAND_LOAD(0, 4);     // load     srcB, reg4
    miAluParams[csALUCmdNum++].Value = CS_ALU_COMMAND_SUB;            // sub      srcA, srcB
    miAluParams[csALUCmdNum++].Value = CS_ALU_COMMAND_STORE_ACCU(5);  // store    reg5, alu

    if (csALUCmdNum >= sizeof(miAluParams) / sizeof(miAluParams[0]))
    {
        eStatus = MOS_STATUS_NO_SPACE;
        return eStatus;
    }

    MHW_MI_MATH_PARAMS miMathParams;
    miMathParams.dwNumAluParams = csALUCmdNum;
    miMathParams.pAluPayload = miAluParams;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiMathCmd(cmdBuffer, &miMathParams));

    // reg5 = reg5 >> 32;
    MHW_MI_LOAD_REGISTER_REG_PARAMS miLoadRegRegParams;
    MOS_ZeroMemory(&miLoadRegRegParams, sizeof(miLoadRegRegParams));
    miLoadRegRegParams.dwSrcRegister = CS_GPR_REGISTER_INDEX(5) + 4;
    miLoadRegRegParams.dwDstRegister = CS_GPR_REGISTER_INDEX(5) + 0;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiLoadRegisterRegCmd(cmdBuffer, &miLoadRegRegParams));
    miLoadRegImmParams.dwData = 0;
    miLoadRegImmParams.dwRegister = (CS_GPR_REGISTER_INDEX(5) + 4);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiLoadRegisterImmCmd(cmdBuffer, &miLoadRegImmParams));

    // reg6 = 0x00000000FFFFFFFF;
    miLoadRegImmParams.dwData = 0xFFFFFFFF;
    miLoadRegImmParams.dwRegister = (CS_GPR_REGISTER_INDEX(6));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiLoadRegisterImmCmd(cmdBuffer, &miLoadRegImmParams));
    miLoadRegImmParams.dwData = 0;
    miLoadRegImmParams.dwRegister = (CS_GPR_REGISTER_INDEX(6) + 4);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiLoadRegisterImmCmd(cmdBuffer, &miLoadRegImmParams));

    csALUCmdNum = 0;
    MOS_ZeroMemory(miAluParams, sizeof(miAluParams));

    // reg6 = reg5 ^ 0x00000000FFFFFFFF;
    miAluParams[csALUCmdNum++].Value = CS_ALU_COMMAND_LOAD(1, 5);     // load     srcA, reg5
    miAluParams[csALUCmdNum++].Value = CS_ALU_COMMAND_LOAD(0, 6);     // load     srcB, reg6
    miAluParams[csALUCmdNum++].Value = CS_ALU_COMMAND_XOR;          // xor      srcA, srcB
    miAluParams[csALUCmdNum++].Value = CS_ALU_COMMAND_STORE_ACCU(6);  // store    reg6, alu

    // reg1 = reg1 & reg5
    miAluParams[csALUCmdNum++].Value = CS_ALU_COMMAND_LOAD(1, 1);     // load     srcA, reg1
    miAluParams[csALUCmdNum++].Value = CS_ALU_COMMAND_LOAD(0, 5);     // load     srcB, reg5
    miAluParams[csALUCmdNum++].Value = CS_ALU_COMMAND_AND;          // and      srcA, srcB
    miAluParams[csALUCmdNum++].Value = CS_ALU_COMMAND_STORE_ACCU(1);  // store    reg1, alu

    // reg0 = reg0 & reg6
    miAluParams[csALUCmdNum++].Value = CS_ALU_COMMAND_LOAD(1, 0);     // load     srcA, reg0
    miAluParams[csALUCmdNum++].Value = CS_ALU_COMMAND_LOAD(0, 6);     // load     srcB, reg6
    miAluParams[csALUCmdNum++].Value = CS_ALU_COMMAND_AND;          // and      srcA, srcB
    miAluParams[csALUCmdNum++].Value = CS_ALU_COMMAND_STORE_ACCU(0);  // store    reg0, alu

    // reg0 = reg0 | reg1
    miAluParams[csALUCmdNum++].Value = CS_ALU_COMMAND_LOAD(1, 0);     // load     srcA, reg0
    miAluParams[csALUCmdNum++].Value = CS_ALU_COMMAND_LOAD(0, 1);     // load     srcB, reg1
    miAluParams[csALUCmdNum++].Value = CS_ALU_COMMAND_OR;           //  or      srcA, srcB
    miAluParams[csALUCmdNum++].Value = CS_ALU_COMMAND_STORE_ACCU(0);  // store    reg0, alu

    if (csALUCmdNum >= sizeof(miAluParams) / sizeof(miAluParams[0]))
    {
        eStatus = MOS_STATUS_NO_SPACE;
        return eStatus;
    }

    miMathParams.dwNumAluParams = csALUCmdNum;
    miMathParams.pAluPayload = miAluParams;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiMathCmd(cmdBuffer, &miMathParams));

    // p->HCP_IMAGE_STATUS_CONTROL = reg0
    MHW_MI_STORE_REGISTER_MEM_PARAMS miStoreRegMemParams;
    MOS_ZeroMemory(&miStoreRegMemParams, sizeof(miStoreRegMemParams));
    miStoreRegMemParams.presStoreBuffer = brcPakStas;
    miStoreRegMemParams.dwOffset = CODECHAL_OFFSETOF(CODECHAL_ENCODE_HEVC_PAK_STATS_BUFFER, HCP_IMAGE_STATUS_CONTROL);
    miStoreRegMemParams.dwRegister = CS_GPR_REGISTER_INDEX(0);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));

    // p->HCP_IMAGE_STATUS_CONTROL_FOR_LAST_PASS = 0
    MHW_MI_STORE_DATA_PARAMS miStoreDataImmParams;
    miStoreDataImmParams.pOsResource = brcPakStas;
    miStoreDataImmParams.dwResourceOffset = CODECHAL_OFFSETOF(CODECHAL_ENCODE_HEVC_PAK_STATS_BUFFER, HCP_IMAGE_STATUS_CONTROL_FOR_LAST_PASS);
    miStoreDataImmParams.dwValue = 0;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreDataImmCmd(cmdBuffer, &miStoreDataImmParams));

    return eStatus;
}

CodechalEncHevcStateG9::CodechalEncHevcStateG9(
    CodechalHwInterface* hwInterface,
    CodechalDebugInterface* debugInterface,
    PCODECHAL_STANDARD_INFO standardInfo)
    :CodechalEncHevcState(hwInterface, debugInterface, standardInfo)
{
    m_fieldScalingOutputInterleaved = false;
    m_brcHistoryBufferSize          = BRC_HISTORY_BUFFER_SIZE;
    m_kuid                          = IDR_CODEC_HEVC_COMBINED_KENREL_INTEL;
    m_kernelBase                    = (uint8_t*)IGCODECKRN_G9;

    MOS_ZeroMemory(&m_scaled2xSurface, sizeof(m_scaled2xSurface));
    MOS_ZeroMemory(&m_sliceMapSurface, sizeof(m_sliceMapSurface));
    MOS_ZeroMemory(&m_32x32PuOutputData, sizeof(m_32x32PuOutputData));
    MOS_ZeroMemory(&m_sad16x16Pu, sizeof(m_sad16x16Pu));
    MOS_ZeroMemory(&m_vme8x8Mode, sizeof(m_vme8x8Mode));
    MOS_ZeroMemory(&m_intraMode, sizeof(m_intraMode));
    MOS_ZeroMemory(&m_intraDist, sizeof(m_intraDist));
    MOS_ZeroMemory(&m_simplestIntraSurface, sizeof(m_simplestIntraSurface));
    MOS_ZeroMemory(&m_roiSurface, sizeof(m_roiSurface));
    MOS_ZeroMemory(&m_concurrentThreadSurface, sizeof(m_concurrentThreadSurface));
    MOS_ZeroMemory(&m_walkingPatternParam, sizeof(m_walkingPatternParam));
    MOS_ZeroMemory(&m_minDistortion, sizeof(m_minDistortion));
    MOS_ZeroMemory(&m_vmeSavedUniSic, sizeof(m_vmeSavedUniSic));
    MOS_ZeroMemory(&m_mvIndex, sizeof(m_mvIndex));
    MOS_ZeroMemory(&m_mvpIndex, sizeof(m_mvpIndex));

    m_numRegionsInSlice = 4;
}

MOS_STATUS CodechalEncHevcStateG9::InitMhw()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    // MHW set-up
    m_hwInterface->GetStateHeapSettings()->dwNumSyncTags = CODECHAL_ENCODE_HEVC_NUM_SYNC_TAGS;
    m_hwInterface->GetStateHeapSettings()->dwDshSize = CODECHAL_INIT_DSH_SIZE_HEVC_ENC;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalGetKernelBinaryAndSize(
        m_kernelBase,
        m_kuid,
        &m_kernelBinary,
        &m_combinedKernelSize));

    m_hwInterface->GetStateHeapSettings()->dwIshSize +=
        MOS_ALIGN_CEIL(m_combinedKernelSize, (1 << MHW_KERNEL_OFFSET_SHIFT));

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG9::UserFeatureKeyReport()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncHevcState::UserFeatureKeyReport());

    CodecHalEncode_WriteKey(__MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_POWER_SAVING, m_powerSavingEnabled);
    CodecHalEncode_WriteKey(__MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_NUM_B_KERNEL_SPLIT, m_numMbBKernelSplit);
    CodecHalEncode_WriteKey(__MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_NUM_8x8_INTRA_KERNEL_SPLIT, m_numMb8x8IntraKernelSplit);

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG9::Initialize(CodechalSetting * settings)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    // common initilization
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncHevcState::Initialize(settings));

    m_bmeMethodTable = (uint8_t *)m_meMethod;
    m_meMethodTable  = (uint8_t *)m_meMethod;

    m_brcBuffers.dwBrcConstantSurfaceWidth  = BRC_CONSTANT_SURFACE_WIDTH;
    m_brcBuffers.dwBrcConstantSurfaceHeight = BRC_CONSTANT_SURFACE_HEIGHT;

    // LCU size is 32x32 in Gen9
    m_widthAlignedMaxLcu  = MOS_ALIGN_CEIL(m_frameWidth, 32);
    m_heightAlignedMaxLcu = MOS_ALIGN_CEIL(m_frameHeight, 32);

    // user feature key setup
    MOS_USER_FEATURE_VALUE_DATA userFeatureData;
    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_SINGLE_TASK_PHASE_ENABLE_ID,
        &userFeatureData);
    m_singleTaskPhaseSupported = (userFeatureData.i32Data) ? true : false;

    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_26Z_ENABLE_ID,
        &userFeatureData);
    m_enable26WalkingPattern = (userFeatureData.i32Data) ? false : true;

    if (m_codecFunction != CODECHAL_FUNCTION_PAK)
    {
        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_ME_ENABLE_ID,
            &userFeatureData);
        m_hmeSupported = (userFeatureData.i32Data) ? true : false;

        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_16xME_ENABLE_ID,
            &userFeatureData);
        m_16xMeSupported = (userFeatureData.i32Data) ? true : false;

        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_32xME_ENABLE_ID,
            &userFeatureData);

        if (userFeatureData.i32Data == 0 || userFeatureData.i32Data == 1)
        {
            m_32xMeUserfeatureControl = true;
            m_32xMeSupported = (userFeatureData.i32Data) ? true : false;
        }
        else
        {
            m_32xMeUserfeatureControl = false;
            m_32xMeSupported = true;
        }
    }

    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    eStatus = MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_REGION_NUMBER_ID,
        &userFeatureData);

    if (eStatus == MOS_STATUS_SUCCESS)
    {
        // Region number must be greater than 1
        m_numRegionsInSlice = (userFeatureData.i32Data < 1) ? 1 : userFeatureData.i32Data;
    }
    else
    {
        // Reset the status to success if user feature key is not set
        eStatus = MOS_STATUS_SUCCESS;
    }

    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_NUM_8x8_INTRA_KERNEL_SPLIT,
        &userFeatureData);
    m_numMb8x8IntraKernelSplit = (userFeatureData.i32Data < 0) ? 0 : userFeatureData.i32Data;

    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_NUM_B_KERNEL_SPLIT,
        &userFeatureData);
    m_numMbBKernelSplit = (userFeatureData.i32Data < 0) ? 0 : userFeatureData.i32Data;

    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_POWER_SAVING,
        &userFeatureData);
    m_powerSavingEnabled = (userFeatureData.i32Data) ? true : false;

    if (MEDIA_IS_SKU(m_skuTable, FtrEncodeHEVC10bit))
    {
        /* Make the width aligned to a multiple of 32 and then get the no of macroblocks.*/
        /* This is done to facilitate the use of format conversion kernel for downscaling to 4x and 2x along with formatconversion of 10 bit data to 8 bit data.
        Refer format conversion kernel for further details .
        We will use only 4x downscale for HME, Super and ultra HME use the traditional scaling kernels.
        */
        uint32_t downscaledSurfaceWidth4x = MOS_ALIGN_CEIL((m_downscaledWidthInMb4x* CODECHAL_MACROBLOCK_WIDTH), (CODECHAL_MACROBLOCK_WIDTH * 2));
        m_downscaledWidthInMb4x = CODECHAL_GET_WIDTH_IN_MACROBLOCKS(downscaledSurfaceWidth4x);

    }

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG9::InitKernelState()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    // Init kernel state
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitKernelStateMbEnc());
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitKernelStateBrc());

    // Create Hme kernel
    m_hmeKernel = MOS_New(CodechalKernelHmeG9, this);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hmeKernel);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hmeKernel->Initialize(
        pfnGetKernelHeaderAndSize,
        m_kernelBase,
        m_kuid));

    return eStatus;
}
