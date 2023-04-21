/*
* Copyright (c) 2018, Intel Corporation
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
//! \file     codechal_fei_hevc_g9_skl.cpp
//! \brief    HEVC FEI dual-pipe encoder for GEN9 SKL.
//!

#include "codechal_fei_hevc_g9_skl.h"
#include "igcodeckrn_g9.h"
#include "codeckrnheader.h"

#define GPUMMU_WA_PADDING                               (64 * 1024)

//! HEVC encoder kernel header structure for G9 SKL
struct CODECHAL_ENC_HEVC_KERNEL_HEADER_FEI_G9_SKL
{
    int nKernelCount;                                                       //!< Total number of kernels

    CODECHAL_KERNEL_HEADER Hevc_FEI_LCUEnc_I_2xDownSampling_Kernel;             //!< 2x down sampling kernel
    CODECHAL_KERNEL_HEADER Hevc_FEI_LCUEnc_I_32x32_PU_ModeDecision_Kernel;      //!< Intra 32x32 PU mode decision kernel
    CODECHAL_KERNEL_HEADER Hevc_FEI_LCUEnc_I_16x16_PU_SADComputation_Kernel;    //!< Intra 16x16 PU SAD computation kernel
    CODECHAL_KERNEL_HEADER Hevc_FEI_LCUEnc_I_16x16_PU_ModeDecision_Kernel;      //!< Intra 16x16 PU mode decision kernel
    CODECHAL_KERNEL_HEADER Hevc_FEI_LCUEnc_I_8x8_PU_Kernel;                     //!< Intra 8x8 PU mode decision kernel
    CODECHAL_KERNEL_HEADER Hevc_FEI_LCUEnc_I_8x8_PU_FMode_Kernel;               //!< Intra 8x8 PU final mode decision kernel
    CODECHAL_KERNEL_HEADER Hevc_FEI_LCUEnc_PB_32x32_PU_IntraCheck;              //!< P/B 32x32 PU intra mode check kernel
    CODECHAL_KERNEL_HEADER HEVC_FEI_LCUEnc_PB_MB;                               //!< P/B MbEnc Kernel
    CODECHAL_KERNEL_HEADER Hevc_FEI_LCUEnc_I_DS4HME;                            //!< 4x Scaling kernel
    CODECHAL_KERNEL_HEADER Hevc_FEI_LCUEnc_P_HME;                               //!< P frame HME kernel
    CODECHAL_KERNEL_HEADER Hevc_FEI_LCUEnc_B_HME;                               //!< B frame HME kernel
    CODECHAL_KERNEL_HEADER Hevc_FEI_LCUEnc_I_COARSE;                            //!< Intra coarse kernel
    CODECHAL_KERNEL_HEADER Hevc_FEI_LCUEnc_PB_Pak;                              //!< P/B frame PAK kernel
    CODECHAL_KERNEL_HEADER Hevc_FEI_LCUEnc_BRC_Blockcopy;                       //!< BRC blockcopy kerenel
    CODECHAL_KERNEL_HEADER Hevc_FEI_LCUEnc_DS_Combined;                         //!< Down scale and format conversion kernel for 10 bit for KBL
    CODECHAL_KERNEL_HEADER HEVC_FEI_LCUEnc_P_MB;                                //!< P frame MbEnc kernel
};

using PCODECHAL_ENC_HEVC_KERNEL_HEADER_FEI_G9_SKL = struct CODECHAL_ENC_HEVC_KERNEL_HEADER_FEI_G9_SKL*;

//! HEVC encoder FEI intra 8x8 PU final mode decision kernel curbe for GEN9
struct CODECHAL_FEI_HEVC_I_8x8_PU_FMODE_CURBE_G9
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
        // For inter frame or enable statictics data dump
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
        //Output (for inter and statictics data dump only)
        struct {
            uint32_t       BTI_Haar_Dist16x16;
        };
        uint32_t Value;
    } DW26;

    union {
        // This surface should take the statistics surface from Hevc_LCUEnc_I_32x32_PU_ModeDecision as input
        struct {
            uint32_t       BTI_Stats_Data;
        };
        uint32_t Value;
    } DW27;

    union {
        // Frame level Statistics data surface
        struct {
            uint32_t       BTI_Frame_Stats_Data;
        };
        uint32_t Value;
    } DW28;

    union {
        // Frame level CTB Distortion data surface
        struct {
            uint32_t       BTI_CTB_Distortion_Surface;
        };
        uint32_t Value;
    } DW29;

    union {
        struct {
            uint32_t       BTI_Debug;
        };
        uint32_t Value;
    } DW30;
};

using PCODECHAL_FEI_HEVC_I_8x8_PU_FMODE_CURBE_G9 = struct CODECHAL_FEI_HEVC_I_8x8_PU_FMODE_CURBE_G9*;
C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(CODECHAL_FEI_HEVC_I_8x8_PU_FMODE_CURBE_G9)) == 31);

//! HEVC encoder FEI B 32x32 PU intra check kernel curbe for GEN9
struct CODECHAL_FEI_HEVC_B_32x32_PU_INTRA_CHECK_CURBE_G9
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

using PCODECHAL_FEI_HEVC_B_32x32_PU_INTRA_CHECK_CURBE_G9 = struct CODECHAL_FEI_HEVC_B_32x32_PU_INTRA_CHECK_CURBE_G9;
C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(CODECHAL_FEI_HEVC_B_32x32_PU_INTRA_CHECK_CURBE_G9)) == 18);

//! HEVC encoder FEI B Pak kernel curbe for GEN9
struct CODECHAL_FEI_HEVC_B_PAK_CURBE_G9
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
            uint32_t   EnableWA                         : MOS_BITFIELD_BIT(     2);
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
            uint32_t  Value;
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

using PCODECHAL_FEI_HEVC_B_PAK_CURBE_G9 = struct CODECHAL_FEI_HEVC_B_PAK_CURBE_G9*;
C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(CODECHAL_FEI_HEVC_B_PAK_CURBE_G9)) == 27);

//! HEVC encoder B MBEnc kernel curbe for GEN9
struct CODECHAL_FEI_HEVC_B_MB_ENC_CURBE_G9
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
            uint32_t   Res_16_22                        : MOS_BITFIELD_RANGE(16,22);
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
            uint32_t       TransformThreshold0          : MOS_BITFIELD_RANGE(0, 15);
            uint32_t       TransformThreshold1          : MOS_BITFIELD_RANGE(16, 31);
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
        struct
        {
            uint32_t   NumMVPredictorsL0                : MOS_BITFIELD_RANGE(0, 3);
            uint32_t   NumMVPredictorsL1                : MOS_BITFIELD_RANGE(4, 7);
            uint32_t   Res_8                            : MOS_BITFIELD_BIT(8);
            uint32_t   PerLCUQP                         : MOS_BITFIELD_BIT(9);
            uint32_t   PerCTBInput                      : MOS_BITFIELD_BIT(10);
            uint32_t   CTBDistortionOutput              : MOS_BITFIELD_BIT(11);
            uint32_t   MVPredictorBlockSize             : MOS_BITFIELD_RANGE(12, 14);
            uint32_t   Res_15                           : MOS_BITFIELD_BIT(15);
            uint32_t   MultiPredL0                      : MOS_BITFIELD_RANGE(16, 19);
            uint32_t   MultiPredL1                      : MOS_BITFIELD_RANGE(20, 23);
            uint32_t   Res_24_31                        : MOS_BITFIELD_RANGE(24, 31);
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
        // this surface need to take same surface name from Hevc_LCUEnc_I_8x8_PU_FMode_inLCU as input
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
            uint32_t       BTI_MVPredictor_Surface;
        };
        uint32_t Value;
    } DW78;

    union {
        //For B frame
        struct {
            uint32_t       BTI_MVPredictor_Surface;
        };
        //For P frame
        struct {
            uint32_t       BTI_CTB_Input_Surface;
        };
        uint32_t Value;
    } DW79;

    union {
        //For B frame
        struct {
            uint32_t       BTI_CTB_Input_Surface;
        };
        //For P frame
        struct {
            uint32_t       BTI_CTB_Distortion_Output_Surface;
        };
        uint32_t Value;
    } DW80;

    union {
        //For B frame
        struct {
            uint32_t       BTI_CTB_Distortion_Output_Surface;
        };
        //For P frame
        struct {
            uint32_t       BTI_Debug;
        };
        uint32_t Value;
    } DW81;

    union {
        //For B frame
        struct {
            uint32_t       BTI_Debug;
        };
        uint32_t Value;
    } DW82;
};

using PCODECHAL_FEI_HEVC_B_MB_ENC_CURBE_G9 = struct CODECHAL_FEI_HEVC_B_MB_ENC_CURBE_G9*;
C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(CODECHAL_FEI_HEVC_B_MB_ENC_CURBE_G9)) == 83 );

MOS_STATUS CodechalFeiHevcStateG9Skl::SetMbEncKernelParams(MHW_KERNEL_PARAM* kernelParams, uint32_t idx)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(kernelParams);

    auto curbeAlignment = m_stateHeapInterface->pStateHeapInterface->GetCurbeAlignment();

    kernelParams->iThreadCount = m_renderEngineInterface->GetHwCaps()->dwMaxThreads;
    kernelParams->iIdCount     = 1;

    switch (idx)
    {
    case CODECHAL_HEVC_MBENC_2xSCALING:
        kernelParams->iBTCount = CODECHAL_HEVC_FEI_SCALING_FRAME_END - CODECHAL_HEVC_FEI_SCALING_FRAME_BEGIN;
        kernelParams->iCurbeLength = MOS_ALIGN_CEIL(sizeof(MEDIA_OBJECT_DOWNSCALING_2X_STATIC_DATA_G9), curbeAlignment);
        kernelParams->iBlockWidth = 32;
        kernelParams->iBlockHeight = 32;
        break;

    case CODECHAL_HEVC_MBENC_32x32MD:
        kernelParams->iBTCount = CODECHAL_HEVC_FEI_32x32_PU_END - CODECHAL_HEVC_FEI_32x32_PU_BEGIN;
        kernelParams->iCurbeLength = MOS_ALIGN_CEIL(sizeof(CODECHAL_FEI_HEVC_I_32x32_PU_MODE_DECISION_CURBE_G9), curbeAlignment);
        kernelParams->iBlockWidth = 32;
        kernelParams->iBlockHeight = 32;
        break;

    case CODECHAL_HEVC_MBENC_16x16SAD:
        kernelParams->iBTCount = CODECHAL_HEVC_FEI_16x16_PU_SAD_END - CODECHAL_HEVC_FEI_16x16_PU_SAD_BEGIN;
        kernelParams->iCurbeLength = MOS_ALIGN_CEIL(sizeof(CODECHAL_ENC_HEVC_I_16x16_SAD_CURBE_G9), curbeAlignment);
        kernelParams->iBlockWidth = 16;
        kernelParams->iBlockHeight = 16;
        break;

    case CODECHAL_HEVC_MBENC_16x16MD:
        kernelParams->iBTCount = CODECHAL_HEVC_FEI_16x16_PU_MD_END - CODECHAL_HEVC_FEI_16x16_PU_MD_BEGIN;
        kernelParams->iCurbeLength = MOS_ALIGN_CEIL(sizeof(CODECHAL_FEI_HEVC_I_16x16_PU_MODEDECISION_CURBE_G9), curbeAlignment);
        kernelParams->iBlockWidth = 32;
        kernelParams->iBlockHeight = 32;
        break;

    case CODECHAL_HEVC_MBENC_8x8PU:
        kernelParams->iBTCount = CODECHAL_HEVC_FEI_8x8_PU_END - CODECHAL_HEVC_FEI_8x8_PU_BEGIN;
        kernelParams->iCurbeLength = MOS_ALIGN_CEIL(sizeof(CODECHAL_FEI_HEVC_I_8x8_PU_CURBE_G9), curbeAlignment);
        kernelParams->iBlockWidth = 8;
        kernelParams->iBlockHeight = 8;
        break;

    case CODECHAL_HEVC_MBENC_8x8FMODE:
        kernelParams->iBTCount = CODECHAL_HEVC_FEI_8x8_PU_FMODE_END - CODECHAL_HEVC_FEI_8x8_PU_FMODE_BEGIN;
        kernelParams->iCurbeLength = MOS_ALIGN_CEIL(sizeof(CODECHAL_FEI_HEVC_I_8x8_PU_FMODE_CURBE_G9), curbeAlignment);
        kernelParams->iBlockWidth = 32;
        kernelParams->iBlockHeight = 32;
        break;

    case CODECHAL_HEVC_MBENC_32x32INTRACHECK:
        kernelParams->iBTCount = CODECHAL_HEVC_FEI_B_32x32_PU_END - CODECHAL_HEVC_FEI_B_32x32_PU_BEGIN;
        kernelParams->iCurbeLength = MOS_ALIGN_CEIL(sizeof(CODECHAL_ENC_HEVC_B_32x32_PU_INTRA_CHECK_CURBE_G9), curbeAlignment);
        kernelParams->iBlockWidth = 32;
        kernelParams->iBlockHeight = 32;
        break;

    case CODECHAL_HEVC_FEI_MBENC_BENC:
        kernelParams->iBTCount = CODECHAL_HEVC_FEI_B_MBENC_END - CODECHAL_HEVC_FEI_B_MBENC_BEGIN;
        kernelParams->iCurbeLength = MOS_ALIGN_CEIL(sizeof(CODECHAL_FEI_HEVC_B_MB_ENC_CURBE_G9), curbeAlignment);
        kernelParams->iBlockWidth = 16;
        kernelParams->iBlockHeight = 16;
        break;

    case CODECHAL_HEVC_FEI_MBENC_BPAK:
        kernelParams->iBTCount = CODECHAL_HEVC_FEI_B_PAK_END - CODECHAL_HEVC_FEI_B_PAK_BEGIN;
        kernelParams->iCurbeLength = MOS_ALIGN_CEIL(sizeof(CODECHAL_ENC_HEVC_B_PAK_CURBE_G9), curbeAlignment);
        kernelParams->iBlockWidth = 32;
        kernelParams->iBlockHeight = 32;
        break;

    case CODECHAL_HEVC_FEI_MBENC_DS_COMBINED:
        if (MEDIA_IS_SKU(m_skuTable, FtrEncodeHEVC10bit))
        {
            kernelParams->iBTCount = CODECHAL_HEVC_FEI_DS_COMBINED_END - CODECHAL_HEVC_FEI_DS_COMBINED_BEGIN;
            uint32_t uiDSCombinedKernelCurbeSize = sizeof(CODECHAL_ENC_HEVC_DS_COMBINED_CURBE_G9);
            kernelParams->iCurbeLength = MOS_ALIGN_CEIL(uiDSCombinedKernelCurbeSize, curbeAlignment);
            kernelParams->iBlockWidth = 8;
            kernelParams->iBlockHeight = 8;
        }
        else
        {
            CODECHAL_ENCODE_ASSERT(false);
            eStatus = MOS_STATUS_INVALID_PARAMETER;
        }
        break;

    case CODECHAL_HEVC_FEI_MBENC_PENC:
        kernelParams->iBTCount = CODECHAL_HEVC_FEI_P_MBENC_END - CODECHAL_HEVC_FEI_P_MBENC_BEGIN;
        //P MBEnc curbe has one less DWord than B MBEnc curbe
        kernelParams->iCurbeLength = MOS_ALIGN_CEIL(sizeof(CODECHAL_FEI_HEVC_B_MB_ENC_CURBE_G9) - sizeof(uint32_t), (size_t)curbeAlignment);
        kernelParams->iBlockWidth = 16;
        kernelParams->iBlockHeight = 16;
        break;

    default:
        CODECHAL_ENCODE_ASSERT(false);
        eStatus = MOS_STATUS_INVALID_PARAMETER;
    }

    return eStatus;
}

MOS_STATUS CodechalFeiHevcStateG9Skl::SetMbEncBindingTable(PCODECHAL_ENCODE_BINDING_TABLE_GENERIC bindingTable, uint32_t idx)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(bindingTable);

    MOS_ZeroMemory(bindingTable, sizeof(*bindingTable));
    bindingTable->dwMediaState = ConvertKrnOpsToMediaState(ENC_MBENC, idx);

    switch (idx)
    {
    case CODECHAL_HEVC_MBENC_2xSCALING:
        bindingTable->dwNumBindingTableEntries = CODECHAL_HEVC_FEI_SCALING_FRAME_END - CODECHAL_HEVC_FEI_SCALING_FRAME_BEGIN;
        bindingTable->dwBindingTableStartOffset = CODECHAL_HEVC_FEI_SCALING_FRAME_BEGIN;
        break;

    case CODECHAL_HEVC_MBENC_32x32MD:
        bindingTable->dwNumBindingTableEntries = CODECHAL_HEVC_FEI_32x32_PU_END - CODECHAL_HEVC_FEI_32x32_PU_BEGIN;
        bindingTable->dwBindingTableStartOffset = CODECHAL_HEVC_FEI_32x32_PU_BEGIN;
        break;

    case CODECHAL_HEVC_MBENC_16x16SAD:
        bindingTable->dwNumBindingTableEntries = CODECHAL_HEVC_FEI_16x16_PU_SAD_END - CODECHAL_HEVC_FEI_16x16_PU_SAD_BEGIN;
        bindingTable->dwBindingTableStartOffset = CODECHAL_HEVC_FEI_16x16_PU_SAD_BEGIN;
        break;

    case CODECHAL_HEVC_MBENC_16x16MD:
        bindingTable->dwNumBindingTableEntries = CODECHAL_HEVC_FEI_16x16_PU_MD_END - CODECHAL_HEVC_FEI_16x16_PU_MD_BEGIN;
        bindingTable->dwBindingTableStartOffset = CODECHAL_HEVC_FEI_16x16_PU_MD_BEGIN;
        break;

    case CODECHAL_HEVC_MBENC_8x8PU:
        bindingTable->dwNumBindingTableEntries = CODECHAL_HEVC_FEI_8x8_PU_END - CODECHAL_HEVC_FEI_8x8_PU_BEGIN;
        bindingTable->dwBindingTableStartOffset = CODECHAL_HEVC_FEI_8x8_PU_BEGIN;
        break;

    case CODECHAL_HEVC_MBENC_8x8FMODE:
        bindingTable->dwNumBindingTableEntries = CODECHAL_HEVC_FEI_8x8_PU_FMODE_END - CODECHAL_HEVC_FEI_8x8_PU_FMODE_BEGIN;
        bindingTable->dwBindingTableStartOffset = CODECHAL_HEVC_FEI_8x8_PU_FMODE_BEGIN;
        break;

    case CODECHAL_HEVC_MBENC_32x32INTRACHECK:
        bindingTable->dwNumBindingTableEntries = CODECHAL_HEVC_FEI_B_32x32_PU_END - CODECHAL_HEVC_FEI_B_32x32_PU_BEGIN;
        bindingTable->dwBindingTableStartOffset = CODECHAL_HEVC_FEI_B_32x32_PU_BEGIN;
        break;

    case CODECHAL_HEVC_FEI_MBENC_BENC:
        bindingTable->dwNumBindingTableEntries = CODECHAL_HEVC_FEI_B_MBENC_END - CODECHAL_HEVC_FEI_B_MBENC_BEGIN;
        bindingTable->dwBindingTableStartOffset = CODECHAL_HEVC_FEI_B_MBENC_BEGIN;
        break;

    case CODECHAL_HEVC_FEI_MBENC_BPAK:
        bindingTable->dwNumBindingTableEntries = CODECHAL_HEVC_FEI_B_PAK_END - CODECHAL_HEVC_FEI_B_PAK_BEGIN;
        bindingTable->dwBindingTableStartOffset = CODECHAL_HEVC_FEI_B_PAK_BEGIN;
        break;

    case CODECHAL_HEVC_FEI_MBENC_DS_COMBINED:
        bindingTable->dwNumBindingTableEntries = CODECHAL_HEVC_FEI_DS_COMBINED_END - CODECHAL_HEVC_FEI_DS_COMBINED_BEGIN;
        bindingTable->dwBindingTableStartOffset = CODECHAL_HEVC_FEI_DS_COMBINED_BEGIN;
        break;

    case CODECHAL_HEVC_FEI_MBENC_PENC:
        bindingTable->dwNumBindingTableEntries = CODECHAL_HEVC_FEI_P_MBENC_END - CODECHAL_HEVC_FEI_P_MBENC_BEGIN;
        bindingTable->dwBindingTableStartOffset = CODECHAL_HEVC_FEI_P_MBENC_BEGIN;
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

MOS_STATUS CodechalFeiHevcStateG9Skl::EndKernelCall(
    CODECHAL_MEDIA_STATE_TYPE       mediaStateType,
    PMHW_KERNEL_STATE               kernelState,
    PMOS_COMMAND_BUFFER             cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncHevcStateG9::EndKernelCall(mediaStateType, kernelState, cmdBuffer));

    // skip haar distortion surface, statstics data dump surface
    // and frame level statstics data surface because they are not used
#if 0
    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpSurface(
            &m_encStatsBuffers.m_puStatsSurface,
            CodechalDbgAttr::attrOutput,
            "HEVC_B_MBENC_PU_StatsSurface",
            CODECHAL_MEDIA_STATE_HEVC_B_MBENC));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpSurface(
            &m_encStatsBuffers.m_8x8PuHaarDist,
            CodechalDbgAttr::attrOutput,
            "HEVC_B_MBENC_8X8_PU_HaarDistSurface",
            CODECHAL_MEDIA_STATE_HEVC_B_MBENC));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &m_encStatsBuffers.m_8x8PuFrameStats.sResource,
            "HEVC_B_MBENC_ConstantData_In",
            CodechalDbgAttr::attrOutput,
            m_encStatsBuffers.m_8x8PuFrameStats.dwSize,
            0,
            CODECHAL_MEDIA_STATE_HEVC_B_MBENC));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpSurface(
            &m_encStatsBuffers.m_mbEncStatsSurface,
            CodechalDbgAttr::attrOutput,
            "HEVC_B_MBENC_MB_ENC_StatsSurface",
            CODECHAL_MEDIA_STATE_HEVC_B_MBENC));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &m_encStatsBuffers.m_mbEncFrameStats.sResource,
            "HEVC_B_MBENC_ConstantData_In",
            CodechalDbgAttr::attrOutput,
            m_encStatsBuffers.m_mbEncFrameStats.dwSize,
            0,
            CODECHAL_MEDIA_STATE_HEVC_B_MBENC));
    )
#endif
    return eStatus;
}

MOS_STATUS CodechalFeiHevcStateG9Skl::InitKernelState()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    // InitKernelStateMbEnc
    m_numMbEncEncKrnStates = CODECHAL_HEVC_FEI_MBENC_NUM_BXT_SKL;

    m_mbEncKernelStates = MOS_NewArray(MHW_KERNEL_STATE, m_numMbEncEncKrnStates);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_mbEncKernelStates);

    m_mbEncKernelBindingTable = (PCODECHAL_ENCODE_BINDING_TABLE_GENERIC)MOS_AllocAndZeroMemory(
        sizeof(GenericBindingTable) * m_numMbEncEncKrnStates);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_mbEncKernelBindingTable);

    auto krnStateIdx = m_mbEncKernelStates;

    for (uint32_t KrnStateIdx = 0; KrnStateIdx < m_numMbEncEncKrnStates; KrnStateIdx++)
    {
        auto kernelSize = m_combinedKernelSize;
        CODECHAL_KERNEL_HEADER currKrnHeader;

        if (KrnStateIdx == CODECHAL_HEVC_FEI_MBENC_DS_COMBINED &&
            m_numMbEncEncKrnStates == CODECHAL_HEVC_FEI_MBENC_NUM_BXT_SKL)  //Ignore. It isn't used on BXT.
        {
            krnStateIdx++;
            continue;
        }

        CODECHAL_ENCODE_CHK_STATUS_RETURN(pfnGetKernelHeaderAndSize(
            m_kernelBinary,
            ENC_MBENC,
            KrnStateIdx,
            &currKrnHeader,
            &kernelSize));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetMbEncKernelParams(
            &krnStateIdx->KernelParams,
            KrnStateIdx));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetMbEncBindingTable(
            &m_mbEncKernelBindingTable[KrnStateIdx], KrnStateIdx));

        krnStateIdx->dwCurbeOffset = m_stateHeapInterface->pStateHeapInterface->GetSizeofCmdInterfaceDescriptorData();
        krnStateIdx->KernelParams.pBinary = m_kernelBinary + (currKrnHeader.KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);
        krnStateIdx->KernelParams.iSize = kernelSize;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnCalculateSshAndBtSizesRequested(
            m_stateHeapInterface,
            krnStateIdx->KernelParams.iBTCount,
            &krnStateIdx->dwSshSize,
            &krnStateIdx->dwBindingTableSize));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->MhwInitISH(m_stateHeapInterface, krnStateIdx));

        krnStateIdx++;
    }

    return eStatus;
}

MOS_STATUS CodechalFeiHevcStateG9Skl::GetKernelHeaderAndSize(
    void                           *binary,
    EncOperation                   operation,
    uint32_t                       krnStateIdx,
    void                           *krnHeader,
    uint32_t                       *krnSize)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(binary);
    CODECHAL_ENCODE_CHK_NULL_RETURN(krnHeader);
    CODECHAL_ENCODE_CHK_NULL_RETURN(krnSize);

    PCODECHAL_ENC_HEVC_KERNEL_HEADER_FEI_G9_SKL kernelHeaderTable = (PCODECHAL_ENC_HEVC_KERNEL_HEADER_FEI_G9_SKL)binary;
    PCODECHAL_KERNEL_HEADER currKrnHeader = nullptr;

    if (operation == ENC_SCALING4X)
    {
        currKrnHeader = &kernelHeaderTable->Hevc_FEI_LCUEnc_I_DS4HME;
    }
    else if (operation == ENC_ME)
    {
        // SKL supports P frame. P HME index CODECHAL_ENCODE_ME_IDX_P is 0 and B HME index CODECHAL_ENCODE_ME_IDX_B is 1
        if (krnStateIdx == 0)
        {
            currKrnHeader = &kernelHeaderTable->Hevc_FEI_LCUEnc_P_HME;
        }
        else
        {
            currKrnHeader = &kernelHeaderTable->Hevc_FEI_LCUEnc_B_HME;
        }
    }
    else if (operation == ENC_BRC)
    {
        switch (krnStateIdx)
        {
        case CODECHAL_HEVC_BRC_COARSE_INTRA:
            currKrnHeader = &kernelHeaderTable->Hevc_FEI_LCUEnc_I_COARSE;
            break;

        default:
            CODECHAL_ENCODE_ASSERTMESSAGE("Unsupported BRC mode requested");
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            return eStatus;
        }
    }
    else if (operation == ENC_MBENC)
    {
        switch (krnStateIdx)
        {
            case CODECHAL_HEVC_MBENC_2xSCALING:
            case CODECHAL_HEVC_MBENC_32x32MD:
            case CODECHAL_HEVC_MBENC_16x16SAD:
            case CODECHAL_HEVC_MBENC_16x16MD:
            case CODECHAL_HEVC_MBENC_8x8PU:
            case CODECHAL_HEVC_MBENC_8x8FMODE:
            case CODECHAL_HEVC_MBENC_32x32INTRACHECK:
            case CODECHAL_HEVC_FEI_MBENC_BENC:
                currKrnHeader = &kernelHeaderTable->Hevc_FEI_LCUEnc_I_2xDownSampling_Kernel;
                currKrnHeader += krnStateIdx;
                break;

            case CODECHAL_HEVC_FEI_MBENC_BPAK:
                currKrnHeader = &kernelHeaderTable->Hevc_FEI_LCUEnc_PB_Pak;
                break;

            case CODECHAL_HEVC_FEI_MBENC_DS_COMBINED:
                currKrnHeader = &kernelHeaderTable->Hevc_FEI_LCUEnc_DS_Combined;
                break;

            case CODECHAL_HEVC_FEI_MBENC_PENC:
                currKrnHeader = &kernelHeaderTable->HEVC_FEI_LCUEnc_P_MB;
                break;

            default:
                CODECHAL_ENCODE_ASSERTMESSAGE("Unsupported ENC mode requested");
                eStatus = MOS_STATUS_INVALID_PARAMETER;
                return eStatus;
        }
    }
    else
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Unsupported ENC mode requested");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    *((PCODECHAL_KERNEL_HEADER)krnHeader) = *currKrnHeader;

    PCODECHAL_KERNEL_HEADER nextKrnHeader = (currKrnHeader + 1);
    PCODECHAL_KERNEL_HEADER invalidEntry = (PCODECHAL_KERNEL_HEADER)(((uint8_t*)binary) + sizeof(*kernelHeaderTable));
    uint32_t nextKrnOffset = *krnSize;

    if (nextKrnHeader < invalidEntry)
    {
        nextKrnOffset = nextKrnHeader->KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT;
    }
    *krnSize = nextKrnOffset - (currKrnHeader->KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);

    return eStatus;
}

#ifndef HEVC_FEI_ENABLE_CMRT

MOS_STATUS CodechalFeiHevcStateG9Skl::Encode2xScalingKernel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    PerfTagSetting perfTag;
    CODECHAL_ENCODE_SET_PERFTAG_INFO(perfTag, CODECHAL_ENCODE_PERFTAG_CALL_SCALING_KERNEL);

    uint32_t krnIdx = CODECHAL_HEVC_MBENC_2xSCALING;
    auto kernelState = &m_mbEncKernelStates[krnIdx];
    auto pScalingBindingTable = &m_mbEncKernelBindingTable[krnIdx];
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
    curbe->DW8.BTI_Src_Y    = pScalingBindingTable->dwBindingTableEntries[startBTI++];
    curbe->DW9.BTI_Dst_Y    = pScalingBindingTable->dwBindingTableEntries[startBTI++];

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
        &pScalingBindingTable->dwBindingTableEntries[startBTI++]
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
        &pScalingBindingTable->dwBindingTableEntries[startBTI++]
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

MOS_STATUS CodechalFeiHevcStateG9Skl::Encode32x32PuModeDecisionKernel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    PerfTagSetting perfTag;
    CODECHAL_ENCODE_SET_PERFTAG_INFO(perfTag, CODECHAL_ENCODE_PERFTAG_CALL_32X32_PU_MD);

    uint32_t krnIdx = CODECHAL_HEVC_MBENC_32x32MD;
    auto kernelState = &m_mbEncKernelStates[krnIdx];
    auto bindingTable = &m_mbEncKernelBindingTable[krnIdx];
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

    CODECHAL_FEI_HEVC_I_32x32_PU_MODE_DECISION_CURBE_G9 cmd, *curbe = &cmd;
    MOS_ZeroMemory(curbe, sizeof(*curbe));
    curbe->DW0.FrameWidth      = MOS_ALIGN_CEIL(m_frameWidth, CODECHAL_MACROBLOCK_WIDTH);
    curbe->DW0.FrameHeight     = MOS_ALIGN_CEIL(m_frameHeight, CODECHAL_MACROBLOCK_HEIGHT);

    curbe->DW1.EnableDebugDump = false;
    curbe->DW1.LCUType         = (log2MaxCUSize==6)? 0 /*64x64*/ : 1 /*32x32*/;
    curbe->DW1.PuType          = 0; // 32x32 PU
    curbe->DW1.BRCEnable       = m_encodeParams.bMbQpDataEnabled || m_brcEnabled;
    curbe->DW1.LCUBRCEnable    = m_encodeParams.bMbQpDataEnabled || m_lcuBrcEnabled;
    curbe->DW1.SliceType       = PicCodingTypeToSliceType(m_hevcPicParams->CodingType);
    curbe->DW1.FASTSurveillanceFlag = (m_hevcPicParams->CodingType == I_TYPE) ? 0 : m_hevcSeqParams->bVideoSurveillance;
    curbe->DW1.ROIEnable            = (m_hevcPicParams->NumROI > 0);
    curbe->DW1.SliceQp         = sliceQp;
    curbe->DW1.EnableStatsDataDump = m_encodeParams.bReportStatisticsEnabled;
    curbe->DW1.EnableQualityImprovement = m_encodeParams.bQualityImprovementEnable;

    curbe->DW2.Lambda          = m_fixedPointLambda;

    curbe->DW3.ModeCost32x32   = 0;

    curbe->DW4.EarlyExit       = (uint32_t)-1;
    if (curbe->DW1.EnableStatsDataDump)
    {
        double lambdaMd;
        float hadBias = 2.0f;

        lambdaMd = m_qpLambdaMd[curbe->DW1.SliceType][sliceQp];
        lambdaMd = lambdaMd * hadBias;
        curbe->DW5.NewLambdaForHaarTransform = (uint32_t)(lambdaMd*(1<<10));
    }

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

    // skip statstics data dump surface because it is not used

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

MOS_STATUS CodechalFeiHevcStateG9Skl::Encode16x16SadPuComputationKernel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    PerfTagSetting perfTag;
    CODECHAL_ENCODE_SET_PERFTAG_INFO(perfTag, CODECHAL_ENCODE_PERFTAG_CALL_16X16_SAD);

    uint32_t krnIdx = CODECHAL_HEVC_MBENC_16x16SAD;
    auto kernelState = &m_mbEncKernelStates[krnIdx];
    auto bindingTable = &m_mbEncKernelBindingTable[krnIdx];
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

    curbe->DW1.Log2MaxCUSize   = m_hevcSeqParams->log2_max_coding_block_size_minus3 + 3;
    curbe->DW1.Log2MinCUSize   = m_hevcSeqParams->log2_min_coding_block_size_minus3 + 3;
    curbe->DW1.Log2MinTUSize   = m_hevcSeqParams->log2_min_transform_block_size_minus2 + 2;
    curbe->DW1.EnableIntraEarlyExit = (m_hevcSeqParams->TargetUsage == 0x04) ? ((m_hevcPicParams->CodingType == I_TYPE) ? 0 : 1) : 0;

    curbe->DW2.SliceType       = PicCodingTypeToSliceType(m_hevcPicParams->CodingType);
    curbe->DW2.SimFlagForInter = false;
    if(m_hevcPicParams->CodingType != I_TYPE)
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

MOS_STATUS CodechalFeiHevcStateG9Skl::Encode16x16PuModeDecisionKernel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    PerfTagSetting perfTag;
    CODECHAL_ENCODE_SET_PERFTAG_INFO(perfTag, CODECHAL_ENCODE_PERFTAG_CALL_16X16_PU_MD);

    uint32_t krnIdx = CODECHAL_HEVC_MBENC_16x16MD;
    auto kernelState = &m_mbEncKernelStates[krnIdx];
    auto bindingTable = &m_mbEncKernelBindingTable[krnIdx];
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

    CODECHAL_FEI_HEVC_I_16x16_PU_MODEDECISION_CURBE_G9 cmd, *curbe = &cmd;
    MOS_ZeroMemory(curbe, sizeof(*curbe));

    uint32_t log2MaxCUSize        = m_hevcSeqParams->log2_max_coding_block_size_minus3 + 3;
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
    curbe->DW3.EnableQualityImprovement = m_encodeParams.bQualityImprovementEnable;

    curbe->DW4.PenaltyForIntra8x8NonDCPredMode = 0;
    curbe->DW4.IntraComputeType                = 1;
    curbe->DW4.AVCIntra8x8Mask                 = 0;
    curbe->DW4.IntraSadAdjust                  = 2;

    double lambdaMd       = sqrt(0.57*pow(2.0, ((double)sliceQp-12.0)/3));
    squredLambda          = lambdaMd * lambdaMd;
    uint32_t newLambda      = (uint32_t)(squredLambda*(1<<10));
    curbe->DW5.FixedPoint_Lambda_CU_Mode_for_Cost_Calculation = newLambda;

    curbe->DW6.ScreenContentFlag    = m_hevcPicParams->bScreenContent;

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
    if (m_encodeParams.bReportStatisticsEnabled)
    {
        curbe->DW10.HaarTransformMode  = true;
    }
    else
    {
        curbe->DW10.HaarTransformMode = (m_hevcPicParams->CodingType == I_TYPE)? false: true;
    }

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

MOS_STATUS CodechalFeiHevcStateG9Skl::Encode8x8PUKernel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    PerfTagSetting perfTag;
    CODECHAL_ENCODE_SET_PERFTAG_INFO(perfTag, CODECHAL_ENCODE_PERFTAG_CALL_8X8_PU);

    uint32_t krnIdx = CODECHAL_HEVC_MBENC_8x8PU;
    auto kernelState = &m_mbEncKernelStates[krnIdx];
    auto bindingTable = &m_mbEncKernelBindingTable[krnIdx];
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
    uint32_t log2MaxCUSize = m_hevcSeqParams->log2_max_coding_block_size_minus3 + 3;
    CODECHAL_FEI_HEVC_I_8x8_PU_CURBE_G9 cmd, *curbe = &cmd;
    MOS_ZeroMemory(curbe, sizeof(*curbe));

    curbe->DW0.FrameWidth          = MOS_ALIGN_CEIL(m_frameWidth, CODECHAL_MACROBLOCK_WIDTH);
    curbe->DW0.FrameHeight         = MOS_ALIGN_CEIL(m_frameHeight, CODECHAL_MACROBLOCK_HEIGHT);
    curbe->DW1.SliceType       = PicCodingTypeToSliceType(m_hevcPicParams->CodingType);
    curbe->DW1.PuType          = 2; // 8x8
    curbe->DW1.DcFilterFlag    = true;
    curbe->DW1.AngleRefineFlag = true;
    curbe->DW1.LCUType         = (log2MaxCUSize==6)? 0 /*64x64*/ : 1 /*32x32*/;
    curbe->DW1.ScreenContentFlag = m_hevcPicParams->bScreenContent;
    curbe->DW1.EnableIntraEarlyExit = (m_hevcSeqParams->TargetUsage == 0x04) ? ((m_hevcPicParams->CodingType == I_TYPE) ? 0 : 1) : 0;
    curbe->DW1.EnableDebugDump = false;
    curbe->DW1.BRCEnable       = m_encodeParams.bMbQpDataEnabled || m_brcEnabled;
    curbe->DW1.LCUBRCEnable    = m_encodeParams.bMbQpDataEnabled || m_lcuBrcEnabled;
    curbe->DW1.ROIEnable       = (m_hevcPicParams->NumROI > 0);
    curbe->DW1.FASTSurveillanceFlag = (m_hevcPicParams->CodingType == I_TYPE) ? 0 : m_hevcSeqParams->bVideoSurveillance;
    curbe->DW1.EnableQualityImprovement = m_encodeParams.bQualityImprovementEnable;
    curbe->DW1.QPValue = CalSliceQp();
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

    if (m_encodeParams.bReportStatisticsEnabled)
    {
        curbe->DW4.HaarTransformFlag   = true;
    }
    else
    {
        curbe->DW4.HaarTransformFlag   = (m_hevcPicParams->CodingType == I_TYPE) ? false : true;
    }
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
            SendKernelCmdsParams sendKernelCmdsParams = SendKernelCmdsParams();
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

MOS_STATUS CodechalFeiHevcStateG9Skl::Encode8x8PUFMODEKernel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    PerfTagSetting perfTag;
    CODECHAL_ENCODE_SET_PERFTAG_INFO(perfTag, CODECHAL_ENCODE_PERFTAG_CALL_8X8_FMODE);

    uint32_t krnIdx = CODECHAL_HEVC_MBENC_8x8FMODE;
    auto kernelState = &m_mbEncKernelStates[krnIdx];
    auto bindingTable = &m_mbEncKernelBindingTable[krnIdx];
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

    CODECHAL_FEI_HEVC_I_8x8_PU_FMODE_CURBE_G9 cmd, *curbe = &cmd;
    MOS_ZeroMemory(curbe, sizeof(*curbe));
    curbe->DW0.FrameWidth                  = MOS_ALIGN_CEIL(m_frameWidth, CODECHAL_MACROBLOCK_WIDTH);
    curbe->DW0.FrameHeight                 = MOS_ALIGN_CEIL(m_frameHeight, CODECHAL_MACROBLOCK_HEIGHT);

    curbe->DW1.SliceType                   = PicCodingTypeToSliceType(m_hevcPicParams->CodingType);
    curbe->DW1.PuType                      = 2;
    curbe->DW1.PakReordingFlag             = (m_hevcPicParams->CodingType == I_TYPE)? true : false;
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
    curbe->DW1.EnableQualityImprovement    = m_encodeParams.bQualityImprovementEnable;
    curbe->DW2.LambdaForLuma               = m_fixedPointLambdaForLuma;

    if (m_hevcPicParams->CodingType != I_TYPE ||
            m_encodeParams.bReportStatisticsEnabled)
    {
        float hadBias = 2.0f;

        double lambdaMd = m_qpLambdaMd[curbe->DW1.SliceType][sliceQp];
        lambdaMd = lambdaMd * hadBias;
        curbe->DW3.LambdaForDistCalculation = (uint32_t)(lambdaMd*(1<<10));
    }
    curbe->DW4.ModeCostFor8x8PU_TU8      = 0;
    curbe->DW5.ModeCostFor8x8PU_TU4      = 0;
    curbe->DW6.SATD16x16PuThreshold      = MOS_MAX(200 * ((int32_t)sliceQp - 12), 0);
    curbe->DW6.BiasFactorToward8x8       = (m_hevcPicParams->bScreenContent) ? 1024 : 1126+102;
    curbe->DW7.Qp                        = sliceQp;
    curbe->DW7.QpForInter                = 0;
    curbe->DW8.SimplifiedFlagForInter    = false;
    curbe->DW8.EnableStatsDataDump       = m_encodeParams.bReportStatisticsEnabled;
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
    curbe->DW29.BTI_CTB_Distortion_Surface = 0;
    startBTI++;
    curbe->DW30.BTI_Debug                = bindingTable->dwBindingTableEntries[startBTI++];

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

    // skip haar distortion surface, statstics data dump surface
    // and frame level statstics data surface because they are not used

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

MOS_STATUS CodechalFeiHevcStateG9Skl::Encode32X32BIntraCheckKernel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    PerfTagSetting perfTag;
    CODECHAL_ENCODE_SET_PERFTAG_INFO(perfTag, CODECHAL_ENCODE_PERFTAG_CALL_32X32_B_IC);

    uint32_t krnIdx = CODECHAL_HEVC_MBENC_32x32INTRACHECK;
    auto kernelState = &m_mbEncKernelStates[krnIdx];
    auto bindingTable = &m_mbEncKernelBindingTable[krnIdx];

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

    CODECHAL_FEI_HEVC_B_32x32_PU_INTRA_CHECK_CURBE_G9 cmd, *curbe = &cmd;
    MOS_ZeroMemory(curbe, sizeof(*curbe));
    curbe->DW0.FrameWidth      = MOS_ALIGN_CEIL(m_frameWidth, CODECHAL_MACROBLOCK_WIDTH);
    curbe->DW0.FrameHeight     = MOS_ALIGN_CEIL(m_frameHeight, CODECHAL_MACROBLOCK_HEIGHT);

    curbe->DW1.EnableDebugDump = false;
    curbe->DW1.EnableIntraEarlyExit = (m_hevcPicParams->CodingType == I_TYPE) ? 0 : 1;
    curbe->DW1.Flags           = 0;
    curbe->DW1.Log2MinTUSize   = m_hevcSeqParams->log2_min_transform_block_size_minus2 + 2;
    curbe->DW1.SliceType       = m_hevcSliceParams->slice_type;
    curbe->DW1.HMEEnable       = 0;
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

    // skip SURFACE_HME_MVP and SURFACE_HME_DIST from HME since FEI alsways disables HME
    startIndex += 2;

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

MOS_STATUS CodechalFeiHevcStateG9Skl::Encode8x8BPakKernel(
    PCODECHAL_FEI_HEVC_B_MB_ENC_CURBE_G9 pEncBCurbe)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(pEncBCurbe);

    PerfTagSetting perfTag;
    CODECHAL_ENCODE_SET_PERFTAG_INFO(perfTag, CODECHAL_ENCODE_PERFTAG_CALL_PAK_KERNEL);

    uint32_t krnIdx = CODECHAL_HEVC_FEI_MBENC_BPAK;
    auto kernelState = &m_mbEncKernelStates[krnIdx];
    auto bindingTable = &m_mbEncKernelBindingTable[krnIdx];
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
    CODECHAL_FEI_HEVC_B_PAK_CURBE_G9  cmd, *curbe = &cmd;
    MOS_ZeroMemory(curbe, sizeof(*curbe));
    curbe->DW0.FrameWidth = MOS_ALIGN_CEIL(m_frameWidth, CODECHAL_MACROBLOCK_WIDTH);
    curbe->DW0.FrameHeight = MOS_ALIGN_CEIL(m_frameHeight, CODECHAL_MACROBLOCK_HEIGHT);

    curbe->DW1.MaxVmvR                 = pEncBCurbe->DW44.MaxVmvR;
    curbe->DW1.Qp                      = pEncBCurbe->DW13.QpPrimeY;
    curbe->DW2.BrcEnable               = pEncBCurbe->DW36.BRCEnable;
    curbe->DW2.LcuBrcEnable            = pEncBCurbe->DW36.LCUBRCEnable;
    curbe->DW2.ScreenContent           = pEncBCurbe->DW47.ScreenContentFlag;
    curbe->DW2.SimplestIntraEnable     = pEncBCurbe->DW47.SkipIntraKrnFlag;
    curbe->DW2.SliceType               = pEncBCurbe->DW4.SliceType;
    curbe->DW2.EnableWA                = 0;
    curbe->DW2.ROIEnable               = (m_hevcPicParams->NumROI > 0);
    curbe->DW2.FASTSurveillanceFlag    = (m_hevcPicParams->CodingType == I_TYPE) ? 0 : m_hevcSeqParams->bVideoSurveillance;
    // KBLControlFlag determines the PAK OBJ format as it varies from Gen9 to Gen9.5+
    curbe->DW2.KBLControlFlag          = UsePlatformControlFlag();
    curbe->DW2.EnableRollingIntra      = m_hevcPicParams->bEnableRollingIntraRefresh;
    curbe->DW2.EnableQualityImprovement = m_encodeParams.bQualityImprovementEnable;
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

    // skip 8 and 9 for SURFACE_WA_CU_RECORD and SURFACE_WA_HCP_PAK

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

MOS_STATUS CodechalFeiHevcStateG9Skl::Encode8x8PBMbEncKernel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    PerfTagSetting perfTag;
    CODECHAL_ENCODE_SET_PERFTAG_INFO(perfTag, CODECHAL_ENCODE_PERFTAG_CALL_MBENC_KERNEL);

    uint32_t krnIdx = CODECHAL_HEVC_FEI_MBENC_BENC;
    if (m_pictureCodingType == P_TYPE)
    {
        //krnIdx = m_hevcPicParams->bEnableRollingIntraRefresh ? CODECHAL_HEVC_FEI_MBENC_ADV_P : CODECHAL_HEVC_FEI_MBENC_PENC;
        krnIdx = CODECHAL_HEVC_FEI_MBENC_PENC;
    }
    else if (m_pictureCodingType == B_TYPE)
    {
        // In TU7, we still need the original ENC B kernel to process the I frame
        //krnIdx = m_hevcPicParams->bEnableRollingIntraRefresh ? CODECHAL_HEVC_FEI_MBENC_ADV : CODECHAL_HEVC_FEI_MBENC_BENC;
        krnIdx = CODECHAL_HEVC_FEI_MBENC_BENC;
    }

    auto kernelState = &m_mbEncKernelStates[krnIdx];
    auto bindingTable = &m_mbEncKernelBindingTable[krnIdx];
    if (m_firstTaskInPhase || !m_singleTaskPhaseSupported)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(RequestSshAndVerifyCommandBufferSize(kernelState));
    }

    int32_t sliceQp = CalSliceQp();
    uint8_t sliceType = PicCodingTypeToSliceType(m_pictureCodingType);

    if (m_feiPicParams->FastIntraMode)
    {
        // When TU=7, lambda is not computed in the 32x32 MD stage for it is skipped.
        CalcLambda(sliceType, INTRA_TRANSFORM_HAAR);
    }
    LoadCosts(sliceType, (uint8_t)sliceQp, INTRA_TRANSFORM_REGULAR);

    uint8_t mbCodeIdxForTempMVP = 0xFF;
    if(m_pictureCodingType != I_TYPE)
    {
        if(m_hevcPicParams->CollocatedRefPicIndex != 0xFF && m_hevcPicParams->CollocatedRefPicIndex < CODEC_MAX_NUM_REF_FRAME_HEVC)
        {
            uint8_t FrameIdx = m_hevcPicParams->RefFrameList[m_hevcPicParams->CollocatedRefPicIndex].FrameIdx;

            mbCodeIdxForTempMVP = m_refList[FrameIdx]->ucScalingIdx;
        }

        if(mbCodeIdxForTempMVP == 0xFF && m_hevcSliceParams->slice_temporal_mvp_enable_flag)
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
    uint8_t forwardTransformThd[7] = { 0 };
    CalcForwardCoeffThd(forwardTransformThd, sliceQp);

    uint32_t curbeSize = 0;
    void *defaultCurbe = (void *)GetDefaultCurbeEncBKernel(curbeSize);
    CODECHAL_ENCODE_ASSERT(defaultCurbe);

    CODECHAL_FEI_HEVC_B_MB_ENC_CURBE_G9 cmd, *curbe = &cmd;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(curbe, sizeof(cmd), defaultCurbe, curbeSize));

    bool transform_8x8_mode_flag = true;
    uint32_t SearchPath  = (m_feiPicParams->SearchWindow == 5) ? 2 : 1; // 2 means full search, 1 means diamand search
    uint32_t LenSP       = m_feiPicParams->LenSP;
    uint32_t RefWidth    = m_feiPicParams->RefWidth;
    uint32_t RefHeight   = m_feiPicParams->RefHeight;

    switch (m_feiPicParams->SearchWindow)
    {
    case 0:
        // not use predefined search window
        if((m_feiPicParams->SearchPath != 0) && (m_feiPicParams->SearchPath != 1) && (m_feiPicParams->SearchPath != 2))
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Invalid picture FEI MB ENC input SearchPath for SearchWindow=0 case!!!.");
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            return eStatus;
        }
        SearchPath = m_feiPicParams->SearchPath;
        if(((RefWidth * RefHeight) > 2048) || (RefWidth > 64) || (RefHeight > 64))
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Invalid picture FEI MB ENC input RefWidth/RefHeight size for SearchWindow=0 case!!!.");
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            return eStatus;
        }
        break;
    case 1:
        // Tiny SUs 24x24 window
        RefWidth  = 24;
        RefHeight = 24;
        LenSP     = 4;
        break;
    case 2:
        // Small SUs 28x28 window
        RefWidth  = 28;
        RefHeight = 28;
        LenSP     = 9;
        break;
    case 3:
        // Diamond SUs 48x40 window
        RefWidth  = 48;
        RefHeight = 40;
        LenSP     = 16;
        break;
    case 4:
        // Large Diamond SUs 48x40 window
        RefWidth  = 48;
        RefHeight = 40;
        LenSP     = 32;
        break;
    case 5:
        // Exhaustive SUs 48x40 window
        RefWidth  = 48;
        RefHeight = 40;
        LenSP     = 48;
        if (m_hevcSeqParams->TargetUsage != 7) 
        {
            if (m_pictureCodingType == B_TYPE)
            {
                LenSP = 48;
            } else {
                LenSP = 57;
            }
        } else {
            LenSP = 25;
        }
        break;
    default:
        CODECHAL_ENCODE_ASSERTMESSAGE("Invalid picture FEI MB ENC SearchWindow value for HEVC FEI on SKL!!!.");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    if((m_pictureCodingType == B_TYPE) && (curbe->DW3.BMEDisableFBR == 0))
    {
        if(RefWidth > 32)
        {
            RefWidth  = 32;
        }
        if(RefHeight > 32)
        {
            RefHeight = 32;
        }
    }

    curbe->DW0.AdaptiveEn  = m_feiPicParams->AdaptiveSearch;
    curbe->DW0.T8x8FlagForInterEn = transform_8x8_mode_flag;
    curbe->DW2.PicWidth    = m_picWidthInMb;
    curbe->DW2.LenSP       = LenSP;
    curbe->DW3.SrcAccess   = curbe->DW3.RefAccess = 0;
    if (m_feiPicParams->FastIntraMode)
    {
        curbe->DW3.FTEnable    = (m_ftqBasedSkip[0x07] >> 1) & 0x01;
    }
    else
    {
        curbe->DW3.FTEnable    = (m_ftqBasedSkip[0x04] >> 1) & 0x01;
    }
    curbe->DW3.SubPelMode  = m_feiPicParams->SubPelMode;

    curbe->DW4.PicHeightMinus1               = m_picHeightInMb - 1;
    curbe->DW4.EnableStatsDataDump           = m_encodeParams.bReportStatisticsEnabled;
    curbe->DW4.HMEEnable                     = 0;
    curbe->DW4.SliceType                     = sliceType;
    curbe->DW4.EnableQualityImprovement      = m_encodeParams.bQualityImprovementEnable;
    curbe->DW4.UseActualRefQPValue           = false;

    curbe->DW5.RefWidth                      = RefWidth;
    curbe->DW5.RefHeight                     = RefHeight;

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
    int32_t qPi = (int32_t)CodecHal_Clip3((-qpBdOffsetC), 51, (sliceQp + m_hevcPicParams->pps_cb_qp_offset));
    int32_t QPc = (qPi < 30) ? qPi : QPcTable[qPi - 30];
    curbe->DW13.QpPrimeCb= QPc + qpBdOffsetC;
    qPi = (int32_t)CodecHal_Clip3((-qpBdOffsetC), 51, (sliceQp + m_hevcPicParams->pps_cr_qp_offset));
    QPc = (qPi < 30) ? qPi : QPcTable[qPi - 30];
    curbe->DW13.QpPrimeCr= QPc;

    curbe->DW14.SICFwdTransCoeffThreshold_0 = forwardTransformThd[0];
    curbe->DW14.SICFwdTransCoeffThreshold_1 = forwardTransformThd[1];
    curbe->DW14.SICFwdTransCoeffThreshold_2 = forwardTransformThd[2];

    curbe->DW15.SICFwdTransCoeffThreshold_3 = forwardTransformThd[3];
    curbe->DW15.SICFwdTransCoeffThreshold_4 = forwardTransformThd[4];
    curbe->DW15.SICFwdTransCoeffThreshold_5 = forwardTransformThd[5];
    curbe->DW15.SICFwdTransCoeffThreshold_6 = forwardTransformThd[6];

    if (SearchPath == 1)
    {
        // diamond search
        if (m_pictureCodingType == P_TYPE)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(&(curbe->DW16), 14 * sizeof(uint32_t), &(m_encBTu7PCurbeInit[16]), 14 * sizeof(uint32_t)));
        }
        else if (m_pictureCodingType == B_TYPE)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(&(curbe->DW16), 14 * sizeof(uint32_t), &(m_encBTu7BCurbeInit[16]), 14 * sizeof(uint32_t)));
        }
    }
    else if((SearchPath != 0) && (SearchPath != 2))
    {
        // default 0 and 2 are full sparil search
        CODECHAL_ENCODE_ASSERT(false);
    }

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

    curbe->DW36.NumRefIdxL0MinusOne = m_hevcSliceParams->num_ref_idx_l0_active_minus1;
    curbe->DW36.NumRefIdxL1MinusOne = m_hevcSliceParams->num_ref_idx_l1_active_minus1;
    curbe->DW36.BRCEnable           = m_encodeParams.bMbQpDataEnabled || m_brcEnabled;
    curbe->DW36.LCUBRCEnable        = m_encodeParams.bMbQpDataEnabled || m_lcuBrcEnabled;
    curbe->DW36.PowerSaving         = m_powerSavingEnabled;
    curbe->DW36.ROIEnable           = (m_hevcPicParams->NumROI > 0);
    curbe->DW36.FASTSurveillanceFlag= (m_hevcPicParams->CodingType == I_TYPE) ? 0 : m_hevcSeqParams->bVideoSurveillance;

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
            float lambda_me = (float)m_qpLambdaMe[CODECHAL_ENCODE_HEVC_B_SLICE][sliceQp];
            if (m_encodeParams.bQualityImprovementEnable)
            {
                curbe->DW40.TransformThreshold0 = (uint16_t) (lambda_me * 56.25 + 0.5);
                curbe->DW40.TransformThreshold1 = (uint16_t) (lambda_me * 21 + 0.5);
                curbe->DW41.TransformThreshold2 = (uint16_t) (lambda_me * 9 + 0.5);
            }
        }
    }

    curbe->DW42.NumMVPredictorsL0      = m_feiPicParams->NumMVPredictorsL0;
    curbe->DW42.NumMVPredictorsL1      = m_feiPicParams->NumMVPredictorsL1;
    curbe->DW42.PerLCUQP               = m_encodeParams.bMbQpDataEnabled;
    curbe->DW42.PerCTBInput            = m_feiPicParams->bPerCTBInput;
    curbe->DW42.CTBDistortionOutput    = m_feiPicParams->bDistortionEnable;
    curbe->DW42.MultiPredL0            = m_feiPicParams->MultiPredL0;
    curbe->DW42.MultiPredL1            = m_feiPicParams->MultiPredL1;
    curbe->DW42.MVPredictorBlockSize   = m_feiPicParams->MVPredictorInput;

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

    curbe->DW46.Log2MaxTUSize          = m_hevcSeqParams->log2_max_transform_block_size_minus2 + 2;
    curbe->DW46.Log2MinTUSize          = m_hevcSeqParams->log2_min_transform_block_size_minus2 + 2;
    curbe->DW46.Log2MaxCUSize          = m_hevcSeqParams->log2_max_coding_block_size_minus3 + 3;
    curbe->DW46.Log2MinCUSize          = m_hevcSeqParams->log2_min_coding_block_size_minus3 + 3;

    curbe->DW47.NumRegionsInSlice      = m_numRegionsInSlice;
    curbe->DW47.TypeOfWalkingPattern   = m_enable26WalkingPattern;
    curbe->DW47.ChromaFlatnessCheckFlag= (m_feiPicParams->FastIntraMode) ? 0 : 1;
    curbe->DW47.EnableIntraEarlyExit   = (m_feiPicParams->FastIntraMode) ? 0 : 1;
    curbe->DW47.SkipIntraKrnFlag       = (m_feiPicParams->FastIntraMode) ? 1 : 0;
    curbe->DW47.CollocatedFromL0Flag   = m_hevcSliceParams->collocated_from_l0_flag;
    curbe->DW47.IsLowDelay             = m_lowDelay;
    curbe->DW47.ScreenContentFlag      = m_hevcPicParams->bScreenContent;
    curbe->DW47.MultiSliceFlag         = (m_numSlices > 1);
    curbe->DW47.ArbitarySliceFlag      = m_arbitraryNumMbsInSlice;
    curbe->DW47.NumRegionMinus1        = m_walkingPatternParam.dwNumRegion - 1;

    if(m_pictureCodingType != I_TYPE)
    {
        curbe->DW48.CurrentTdL0_0          = ComputeTemporalDifference(m_hevcSliceParams->RefPicList[0][0]);
        curbe->DW48.CurrentTdL0_1          = ComputeTemporalDifference(m_hevcSliceParams->RefPicList[0][1]);
        curbe->DW49.CurrentTdL0_2          = ComputeTemporalDifference(m_hevcSliceParams->RefPicList[0][2]);
        curbe->DW49.CurrentTdL0_3          = ComputeTemporalDifference(m_hevcSliceParams->RefPicList[0][3]);
        if(m_pictureCodingType == B_TYPE) {
            curbe->DW50.CurrentTdL1_0          = ComputeTemporalDifference(m_hevcSliceParams->RefPicList[1][0]);
            curbe->DW50.CurrentTdL1_1          = ComputeTemporalDifference(m_hevcSliceParams->RefPicList[1][1]);
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
        curbe->DW78.BTI_MVPredictor_Surface= bindingTable->dwBindingTableEntries[startBTI++];
        curbe->DW79.BTI_CTB_Input_Surface  = bindingTable->dwBindingTableEntries[startBTI++];
        curbe->DW80.BTI_CTB_Distortion_Output_Surface = bindingTable->dwBindingTableEntries[startBTI++];
        curbe->DW81.BTI_Debug              = bindingTable->dwBindingTableEntries[startBTI++];
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
        curbe->DW79.BTI_MVPredictor_Surface= bindingTable->dwBindingTableEntries[startBTI++];
        curbe->DW80.BTI_CTB_Input_Surface  = bindingTable->dwBindingTableEntries[startBTI++];
        curbe->DW81.BTI_CTB_Distortion_Output_Surface = bindingTable->dwBindingTableEntries[startBTI++];
        curbe->DW82.BTI_Debug              = bindingTable->dwBindingTableEntries[startBTI++];
    }

    // Intra refresh is enabled. Program related CURBE fields
    if (m_hevcPicParams->bEnableRollingIntraRefresh)
    {
        curbe->DW35.IntraRefreshEn     = true;
        curbe->DW35.FirstIntraRefresh  = m_firstIntraRefresh;
        curbe->DW35.HalfUpdateMixedLCU     = 0;
        curbe->DW35.EnableRollingIntra     = true;

        curbe->DW38.NumFrameInGOB            = m_frameNumInGob;
        curbe->DW38.NumIntraRefreshOffFrames = m_frameNumWithoutIntraRefresh;

        curbe->DW51.IntraRefreshQPDelta        = m_hevcPicParams->QpDeltaForInsertedIntra;
        curbe->DW51.IntraRefreshMBNum          = m_hevcPicParams->IntraInsertionLocation;
        curbe->DW51.IntraRefreshUnitInMB       = m_hevcPicParams->IntraInsertionSize;

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

    // 6 and 7, skip SURFACE_HME_MVP and SURFACE_HME_DIST from HME since FEI alsways disables HME
    startBTI += 2;

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
        if(!CodecHal_PictureIsInvalid(refPic) &&
            !CodecHal_PictureIsInvalid(m_hevcPicParams->RefFrameList[refPic.FrameIdx]))
        {
            uint8_t idx = m_hevcPicParams->RefFrameList[refPic.FrameIdx].FrameIdx;

            // Picture Y VME
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
        if(!CodecHal_PictureIsInvalid(refPic) &&
            !CodecHal_PictureIsInvalid(m_hevcPicParams->RefFrameList[refPic.FrameIdx]))
        {
            uint8_t idx = m_hevcPicParams->RefFrameList[refPic.FrameIdx].FrameIdx;

            // Picture Y VME
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
            if(!CodecHal_PictureIsInvalid(refPic) &&
                !CodecHal_PictureIsInvalid(m_hevcPicParams->RefFrameList[refPic.FrameIdx]))
            {
                uint8_t idx = m_hevcPicParams->RefFrameList[refPic.FrameIdx].FrameIdx;

                // Picture Y VME
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

    // skip three BTI for haar distortion surface, statstics data dump surface
    // and frame level statstics data surface because they are not used
    startBTI += 3;

    // 48: FEI external MVPredictor surface
    if (m_feiPicParams->MVPredictorInput)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
                    kernelState,
                    &cmdBuffer,
                    SURFACE_FEI_EXTERNAL_MVP,
                    &bindingTable->dwBindingTableEntries[startBTI++]));
    }
    else
    {
        startBTI++;
    }

    if (m_feiPicParams->bPerCTBInput)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSurfacesState(
                    kernelState,
                    &cmdBuffer,
                    SURFACE_FEI_PER_CTB_CTRL,
                    &bindingTable->dwBindingTableEntries[startBTI++]));
    }
    else
    {
        startBTI ++;
    }
    startBTI += 1;

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

#else

MOS_STATUS CodechalFeiHevcStateG9Skl::Encode2xScalingKernel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    PerfTagSetting perfTag;
    CODECHAL_ENCODE_SET_PERFTAG_INFO(perfTag, CODECHAL_ENCODE_PERFTAG_CALL_SCALING_KERNEL);

    //Setup CURBE
    MEDIA_OBJECT_DOWNSCALING_2X_STATIC_DATA_G9  cmd, *curbe = &cmd;
    MOS_ZeroMemory(curbe, sizeof(*curbe));
    curbe->DW0.PicWidth  = MOS_ALIGN_CEIL(m_frameWidth, CODECHAL_MACROBLOCK_WIDTH);
    curbe->DW0.PicHeight    = MOS_ALIGN_CEIL(m_frameHeight, CODECHAL_MACROBLOCK_HEIGHT);

    DownScalingKernelParams scalingParams;
    MOS_ZeroMemory(&scalingParams, sizeof(scalingParams));

    scalingParams.m_cmSurfDS_TopIn = &m_rawSurfaceToEnc->OsResource;
    scalingParams.m_cmSurfDS_TopOut = &m_scaled2xSurface.OsResource;
    scalingParams.m_cmSurfTopVProc = nullptr;

    if (m_cmKernelMap.count("2xScaling") == 0)
    {
        m_cmKernelMap["2xScaling"] = new CMRTKernelDownScalingUMD();
        m_cmKernelMap["2xScaling"]->Init((void *)m_osInterface->pOsContext);
    }

    m_cmKernelMap["2xScaling"]->SetupCurbe(curbe);

    CODECHAL_MEDIA_STATE_TYPE encFunctionType = CODECHAL_MEDIA_STATE_2X_SCALING;
    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpMDFCurbe(
            encFunctionType,
            (uint8_t *)curbe, sizeof(*curbe)));
    )

    m_cmKernelMap["2xScaling"]->AllocateSurfaces(&scalingParams);

    //No need to wait for task finished
    m_cmEvent = CM_NO_EVENT;
    m_cmKernelMap["2xScaling"]->CreateAndDispatchKernel(m_cmEvent, false, (!m_singleTaskPhaseSupported));

    return eStatus;
}

MOS_STATUS CodechalFeiHevcStateG9Skl::Encode32x32PuModeDecisionKernel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    PerfTagSetting perfTag;
    CODECHAL_ENCODE_SET_PERFTAG_INFO(perfTag, CODECHAL_ENCODE_PERFTAG_CALL_32X32_PU_MD);

    //Setup CURBE
    uint32_t log2MaxCUSize = m_hevcSeqParams->log2_max_coding_block_size_minus3 + 3;

    CalcLambda(CODECHAL_ENCODE_HEVC_I_SLICE, INTRA_TRANSFORM_HAAR);
    int32_t sliceQp = CalSliceQp();

    double lambdaScalingFactor = 1.0;
    double qpLambda = m_qpLambdaMd[CODECHAL_ENCODE_HEVC_I_SLICE][sliceQp];
    double squaredQpLambda = qpLambda * qpLambda;
    m_fixedPointLambda = (uint32_t)(lambdaScalingFactor * squaredQpLambda * (1<<10));

    CODECHAL_FEI_HEVC_I_32x32_PU_MODE_DECISION_CURBE_G9 cmd, *curbe = &cmd;
    MOS_ZeroMemory(curbe, sizeof(*curbe));
    curbe->DW0.FrameWidth      = MOS_ALIGN_CEIL(m_frameWidth, CODECHAL_MACROBLOCK_WIDTH);
    curbe->DW0.FrameHeight     = MOS_ALIGN_CEIL(m_frameHeight, CODECHAL_MACROBLOCK_HEIGHT);

    curbe->DW1.EnableDebugDump = false;
    curbe->DW1.LCUType         = (log2MaxCUSize==6)? 0 /*64x64*/ : 1 /*32x32*/;
    curbe->DW1.PuType          = 0; // 32x32 PU
    curbe->DW1.BRCEnable                 = m_encodeParams.bMbQpDataEnabled || m_brcEnabled;
    curbe->DW1.LCUBRCEnable              = m_encodeParams.bMbQpDataEnabled || m_lcuBrcEnabled;
    curbe->DW1.SliceType                 = PicCodingTypeToSliceType(m_hevcPicParams->CodingType);
    curbe->DW1.FASTSurveillanceFlag      = (m_hevcPicParams->CodingType == I_TYPE) ? 0 : m_hevcSeqParams->bVideoSurveillance;
    curbe->DW1.ROIEnable                 = (m_hevcPicParams->NumROI > 0);
    curbe->DW1.SliceQp         = sliceQp;
    curbe->DW1.EnableStatsDataDump = m_encodeParams.bReportStatisticsEnabled;
    curbe->DW1.EnableQualityImprovement = m_encodeParams.bQualityImprovementEnable;

    curbe->DW2.Lambda          = m_fixedPointLambda;

    curbe->DW3.ModeCost32x32   = 0;

    curbe->DW4.EarlyExit       = (uint32_t)-1;
    if (curbe->DW1.EnableStatsDataDump)
    {
        double lambdaMd;
        float hadBias = 2.0f;

        lambdaMd = m_qpLambdaMd[curbe->DW1.SliceType][sliceQp];
        lambdaMd = lambdaMd * hadBias;
        curbe->DW5.NewLambdaForHaarTransform = (uint32_t)(lambdaMd*(1<<10));
    }

    IFrameKernelParams I32x32Params;
    MOS_ZeroMemory(&I32x32Params, sizeof(I32x32Params));

    I32x32Params.m_cmSurfPer32x32PUDataOut = &m_32x32PuOutputData.sResource;
    I32x32Params.m_cmSurfCurrY = &m_rawSurfaceToEnc->OsResource;
    I32x32Params.m_cmSurfCurrY2 = &m_scaled2xSurface.OsResource;
    I32x32Params.m_cmSurfSliceMap = &m_sliceMapSurface.OsResource;
    I32x32Params.m_cmSurfCombinedQP = (MOS_RESOURCE*)m_allocator->GetResource(m_standard, brcInputForEncKernel);
    I32x32Params.m_cmLCUQPSurf = &m_lcuQP.OsResource;
    I32x32Params.m_cmBRCConstSurf          = &m_brcBuffers.sBrcConstantDataBuffer[m_currRecycledBufIdx].OsResource;

    if (m_cmKernelMap.count("I_32X32") == 0)
    {
        m_cmKernelMap["I_32X32"] = new CMRTKernelI32x32UMD();
        m_cmKernelMap["I_32X32"]->Init(nullptr, m_cmKernelMap["2xScaling"]->m_cmDev, m_cmKernelMap["2xScaling"]->m_cmQueue, m_cmKernelMap["2xScaling"]->m_cmTask, nullptr);
    }

    m_cmKernelMap["I_32X32"]->SetupCurbe(curbe);

    CODECHAL_MEDIA_STATE_TYPE encFunctionType = CODECHAL_MEDIA_STATE_32x32_PU_MODE_DECISION;
    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpMDFCurbe(
            encFunctionType,
            (uint8_t *)curbe, sizeof(*curbe)));
    )

    m_cmKernelMap["I_32X32"]->AllocateSurfaces(&I32x32Params);

    //No need to wait for task finished
    m_cmEvent = CM_NO_EVENT;
    m_cmKernelMap["I_32X32"]->CreateAndDispatchKernel(m_cmEvent, false, (!m_singleTaskPhaseSupported));

    return eStatus;
}

MOS_STATUS CodechalFeiHevcStateG9Skl::Encode16x16SadPuComputationKernel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    PerfTagSetting perfTag;
    CODECHAL_ENCODE_SET_PERFTAG_INFO(perfTag, CODECHAL_ENCODE_PERFTAG_CALL_16X16_SAD);

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

    IFrameKernelParams I16x16SadParams;
    MOS_ZeroMemory(&I16x16SadParams, sizeof(I16x16SadParams));

    I16x16SadParams.m_cmSurfCurrY = &m_rawSurfaceToEnc->OsResource;
    I16x16SadParams.m_cmSurfPer32x32PUDataOut = &m_32x32PuOutputData.sResource;
    I16x16SadParams.m_cmSurfSAD16x16 = &m_sad16x16Pu.sResource;
    I16x16SadParams.m_cmSurfSliceMap = &m_sliceMapSurface.OsResource;
    I16x16SadParams.m_cmSurfSIF = &m_simplestIntraSurface.OsResource;

    //in case I_32x32 isn't initialized when using FastIntraMode for per-frame control (I: enable; P/B: disable)
    if (m_cmKernelMap.count("I_32X32") == 0)
    {
        m_cmKernelMap["I_32X32"] = new CMRTKernelI32x32UMD();
        m_cmKernelMap["I_32X32"]->Init(nullptr, m_cmKernelMap["2xScaling"]->m_cmDev, m_cmKernelMap["2xScaling"]->m_cmQueue, m_cmKernelMap["2xScaling"]->m_cmTask, nullptr);
    }

    if (m_cmKernelMap.count("I_16X16_SAD") == 0)
    {
        m_cmKernelMap["I_16X16_SAD"] = new CMRTKernelI16x16SadUMD();
        m_cmKernelMap["I_16X16_SAD"]->Init(nullptr, m_cmKernelMap["2xScaling"]->m_cmDev, m_cmKernelMap["2xScaling"]->m_cmQueue, m_cmKernelMap["2xScaling"]->m_cmTask, m_cmKernelMap["I_32X32"]->m_cmProgram);
    }

    m_cmKernelMap["I_16X16_SAD"]->SetupCurbe(curbe);

    CODECHAL_MEDIA_STATE_TYPE encFunctionType = CODECHAL_MEDIA_STATE_16x16_PU_SAD;
    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpMDFCurbe(
            encFunctionType,
            (uint8_t *)curbe, sizeof(*curbe)));
    )

    m_cmKernelMap["I_16X16_SAD"]->AllocateSurfaces(&I16x16SadParams);

    //No need to wait for task finished
    m_cmEvent = CM_NO_EVENT;
    m_cmKernelMap["I_16X16_SAD"]->CreateAndDispatchKernel(m_cmEvent, false, (!m_singleTaskPhaseSupported));

    return eStatus;
}

MOS_STATUS CodechalFeiHevcStateG9Skl::Encode16x16PuModeDecisionKernel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    PerfTagSetting perfTag;
    CODECHAL_ENCODE_SET_PERFTAG_INFO(perfTag, CODECHAL_ENCODE_PERFTAG_CALL_16X16_PU_MD);

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

    CODECHAL_FEI_HEVC_I_16x16_PU_MODEDECISION_CURBE_G9 cmd, *curbe = &cmd;
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
    curbe->DW3.IntraRefreshEn            = m_hevcPicParams->bEnableRollingIntraRefresh;
    curbe->DW3.HalfUpdateMixedLCU     = 0;
    curbe->DW3.EnableQualityImprovement = m_encodeParams.bQualityImprovementEnable;

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
    if (m_encodeParams.bReportStatisticsEnabled)
    {
        curbe->DW10.HaarTransformMode  = true;
    }
    else
    {
        curbe->DW10.HaarTransformMode = (m_hevcPicParams->CodingType == I_TYPE) ? false : true;
    }

    IFrameKernelParams I16x16ModeParams;
    MOS_ZeroMemory(&I16x16ModeParams, sizeof(I16x16ModeParams));

    I16x16ModeParams.m_cmSurfCurrY = &m_rawSurfaceToEnc->OsResource;
    I16x16ModeParams.m_cmSurfSAD16x16 = &m_sad16x16Pu.sResource;
    I16x16ModeParams.m_cmSurfPOCDbuf = &m_resMbCodeSurface;
    I16x16ModeParams.m_bufSize = m_mbCodeSize - m_mvOffset;
    I16x16ModeParams.m_bufOffset = m_mvOffset;
    I16x16ModeParams.m_cmSurfPer32x32PUDataOut = &m_32x32PuOutputData.sResource;
    I16x16ModeParams.m_cmSurfVMEMode = &m_vme8x8Mode.sResource;
    I16x16ModeParams.m_cmSurfSliceMap = &m_sliceMapSurface.OsResource;
    I16x16ModeParams.m_cmSurfSIF = &m_simplestIntraSurface.OsResource;
    I16x16ModeParams.m_cmSurfCombinedQP = (MOS_RESOURCE*)m_allocator->GetResource(m_standard, brcInputForEncKernel);
    I16x16ModeParams.m_cmLCUQPSurf = &m_lcuQP.OsResource;
    I16x16ModeParams.m_cmBRCConstSurf          = &m_brcBuffers.sBrcConstantDataBuffer[m_currRecycledBufIdx].OsResource;

    if (m_cmKernelMap.count("I_16X16_MODE") == 0)
    {
        m_cmKernelMap["I_16X16_MODE"] = new CMRTKernelI16x16ModeUMD();
        m_cmKernelMap["I_16X16_MODE"]->Init(nullptr, m_cmKernelMap["2xScaling"]->m_cmDev, m_cmKernelMap["2xScaling"]->m_cmQueue, m_cmKernelMap["2xScaling"]->m_cmTask, m_cmKernelMap["I_32X32"]->m_cmProgram);
    }

    m_cmKernelMap["I_16X16_MODE"]->SetupCurbe(curbe);

    CODECHAL_MEDIA_STATE_TYPE encFunctionType = CODECHAL_MEDIA_STATE_16x16_PU_MODE_DECISION;
    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpMDFCurbe(
            encFunctionType,
            (uint8_t *)curbe, sizeof(*curbe)));
    )

    m_cmKernelMap["I_16X16_MODE"]->AllocateSurfaces(&I16x16ModeParams);

    //No need to wait for task finished
    m_cmEvent = CM_NO_EVENT;
    m_cmKernelMap["I_16X16_MODE"]->CreateAndDispatchKernel(m_cmEvent, false, (!m_singleTaskPhaseSupported));

    return eStatus;
}

MOS_STATUS CodechalFeiHevcStateG9Skl::Encode8x8PUKernel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    PerfTagSetting perfTag;
    CODECHAL_ENCODE_SET_PERFTAG_INFO(perfTag, CODECHAL_ENCODE_PERFTAG_CALL_8X8_PU);

    // Setup CURBE
    uint32_t                            log2MaxCUSize = m_hevcSeqParams->log2_max_coding_block_size_minus3 + 3;
    CODECHAL_FEI_HEVC_I_8x8_PU_CURBE_G9 cmd, *curbe = &cmd;
    MOS_ZeroMemory(curbe, sizeof(*curbe));

    curbe->DW0.FrameWidth          = MOS_ALIGN_CEIL(m_frameWidth, CODECHAL_MACROBLOCK_WIDTH);
    curbe->DW0.FrameHeight         = MOS_ALIGN_CEIL(m_frameHeight, CODECHAL_MACROBLOCK_HEIGHT);

    curbe->DW1.SliceType           = PicCodingTypeToSliceType(m_hevcPicParams->CodingType);
    curbe->DW1.PuType          = 2; // 8x8
    curbe->DW1.DcFilterFlag    = true;
    curbe->DW1.AngleRefineFlag = true;
    curbe->DW1.LCUType         = (log2MaxCUSize==6)? 0 : 1;
    curbe->DW1.ScreenContentFlag         = m_hevcPicParams->bScreenContent;
    curbe->DW1.EnableIntraEarlyExit      = (m_hevcSeqParams->TargetUsage == 0x04) ? ((m_hevcPicParams->CodingType == I_TYPE) ? 0 : 1) : 0;
    curbe->DW1.EnableDebugDump = false;
    curbe->DW1.BRCEnable                 = m_encodeParams.bMbQpDataEnabled || m_brcEnabled;
    curbe->DW1.LCUBRCEnable              = m_encodeParams.bMbQpDataEnabled || m_lcuBrcEnabled;
    curbe->DW1.ROIEnable                 = (m_hevcPicParams->NumROI > 0);
    curbe->DW1.FASTSurveillanceFlag      = (m_hevcPicParams->CodingType == I_TYPE) ? 0 : m_hevcSeqParams->bVideoSurveillance;
    curbe->DW1.EnableQualityImprovement  = m_encodeParams.bQualityImprovementEnable;
    curbe->DW1.QPValue = CalSliceQp();
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

    if (m_encodeParams.bReportStatisticsEnabled)
    {
        curbe->DW4.HaarTransformFlag   = true;
    }
    else
    {
        curbe->DW4.HaarTransformFlag = (m_hevcPicParams->CodingType == I_TYPE) ? false : true;
    }
    curbe->DW4.SimplifiedFlagForInter  = false;

    IFrameKernelParams I8x8Params;
    MOS_ZeroMemory(&I8x8Params, sizeof(I8x8Params));

    I8x8Params.m_cmSurfCurrY = &m_rawSurfaceToEnc->OsResource;
    I8x8Params.m_cmSurfSliceMap = &m_sliceMapSurface.OsResource;
    I8x8Params.m_cmSurfVMEMode = &m_vme8x8Mode.sResource;
    I8x8Params.m_cmSurfMode = &m_intraMode.sResource;
    I8x8Params.m_cmSurfCombinedQP = (MOS_RESOURCE*)m_allocator->GetResource(m_standard, brcInputForEncKernel);
    I8x8Params.m_cmSurfSIF = &m_simplestIntraSurface.OsResource;
    I8x8Params.m_cmLCUQPSurf = &m_lcuQP.OsResource;
    I8x8Params.m_cmBRCConstSurf   = &m_brcBuffers.sBrcConstantDataBuffer[m_currRecycledBufIdx].OsResource;

    if (m_cmKernelMap.count("I_8X8") == 0)
    {
        m_cmKernelMap["I_8X8"] = new CMRTKernelI8x8UMD();
        m_cmKernelMap["I_8X8"]->Init(nullptr, m_cmKernelMap["2xScaling"]->m_cmDev, m_cmKernelMap["2xScaling"]->m_cmQueue, m_cmKernelMap["2xScaling"]->m_cmTask, m_cmKernelMap["I_32X32"]->m_cmProgram);
    }

    m_cmKernelMap["I_8X8"]->SetupCurbe(curbe);

    CODECHAL_MEDIA_STATE_TYPE encFunctionType = CODECHAL_MEDIA_STATE_8x8_PU;
    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpMDFCurbe(
            encFunctionType,
            (uint8_t *)curbe, sizeof(*curbe)));
    )

    m_cmKernelMap["I_8X8"]->AllocateSurfaces(&I8x8Params);

    //No need to wait for task finished
    m_cmEvent = CM_NO_EVENT;
    m_cmKernelMap["I_8X8"]->CreateAndDispatchKernel(m_cmEvent, false, (!m_singleTaskPhaseSupported));

    return eStatus;
}

MOS_STATUS CodechalFeiHevcStateG9Skl::Encode8x8PUFMODEKernel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    PerfTagSetting perfTag;
    CODECHAL_ENCODE_SET_PERFTAG_INFO(perfTag, CODECHAL_ENCODE_PERFTAG_CALL_8X8_FMODE);

    // Setup CURBE
    int32_t qp = CalSliceQp();
    uint32_t sliceQp = (uint32_t)qp;
    uint32_t log2MaxCUSize = m_hevcSeqParams->log2_max_coding_block_size_minus3 + 3;

    CODECHAL_FEI_HEVC_I_8x8_PU_FMODE_CURBE_G9 cmd, *curbe = &cmd;
    MOS_ZeroMemory(curbe, sizeof(*curbe));
    curbe->DW0.FrameWidth                  = MOS_ALIGN_CEIL(m_frameWidth, CODECHAL_MACROBLOCK_WIDTH);
    curbe->DW0.FrameHeight                 = MOS_ALIGN_CEIL(m_frameHeight, CODECHAL_MACROBLOCK_HEIGHT);

    curbe->DW1.SliceType                   = PicCodingTypeToSliceType(m_hevcPicParams->CodingType);
    curbe->DW1.PuType                      = 2;
    curbe->DW1.PakReordingFlag             = (m_hevcPicParams->CodingType == I_TYPE) ? true : false;
    curbe->DW1.LCUType                     = (log2MaxCUSize == 6)? 0 : 1;
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
    curbe->DW1.EnableQualityImprovement    = m_encodeParams.bQualityImprovementEnable;
    curbe->DW2.LambdaForLuma               = m_fixedPointLambdaForLuma;
    if (m_hevcPicParams->CodingType != I_TYPE ||
        m_encodeParams.bReportStatisticsEnabled)
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
    curbe->DW8.EnableStatsDataDump       = m_encodeParams.bReportStatisticsEnabled;
    // KBLControlFlag determines the PAK OBJ format as it varies from Gen9 to Gen9.5+
    curbe->DW8.KBLControlFlag            = UsePlatformControlFlag();
    curbe->DW9.IntraRefreshMBNum         = m_hevcPicParams->IntraInsertionLocation;
    curbe->DW9.IntraRefreshQPDelta       = m_hevcPicParams->QpDeltaForInsertedIntra;
    curbe->DW9.IntraRefreshUnitInMB      = m_hevcPicParams->IntraInsertionSize;

    IFrameKernelParams I8x8ModeParams;
    MOS_ZeroMemory(&I8x8ModeParams, sizeof(I8x8ModeParams));

    I8x8ModeParams.m_cmSurfPOCDbuf = &m_resMbCodeSurface;
    I8x8ModeParams.m_bufSize = m_mbCodeSize - m_mvOffset;
    I8x8ModeParams.m_bufOffset = m_mvOffset;
    I8x8ModeParams.m_cmSurfVMEMode = &m_vme8x8Mode.sResource;
    I8x8ModeParams.m_cmSurfMode = &m_intraMode.sResource;
    I8x8ModeParams.m_cmSurfSliceMap = &m_sliceMapSurface.OsResource;
    I8x8ModeParams.m_cmSurfIntraDist = &m_intraDist.sResource;
    I8x8ModeParams.m_cmSurfSIF = &m_simplestIntraSurface.OsResource;
    I8x8ModeParams.m_cmSurfCombinedQP = (MOS_RESOURCE*)m_allocator->GetResource(m_standard, brcInputForEncKernel);
    I8x8ModeParams.m_cmLCUQPSurf = &m_lcuQP.OsResource;
    I8x8ModeParams.m_cmBRCConstSurf   = &m_brcBuffers.sBrcConstantDataBuffer[m_currRecycledBufIdx].OsResource;

    if (m_cmKernelMap.count("I_8X8_MODE") == 0)
    {
        m_cmKernelMap["I_8X8_MODE"] = new CMRTKernelI8x8ModeUMD();
        m_cmKernelMap["I_8X8_MODE"]->Init(nullptr, m_cmKernelMap["2xScaling"]->m_cmDev, m_cmKernelMap["2xScaling"]->m_cmQueue, m_cmKernelMap["2xScaling"]->m_cmTask, m_cmKernelMap["I_32X32"]->m_cmProgram);
    }

    m_cmKernelMap["I_8X8_MODE"]->SetupCurbe(curbe);

    CODECHAL_MEDIA_STATE_TYPE encFunctionType = CODECHAL_MEDIA_STATE_8x8_PU_FMODE;
    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpMDFCurbe(
            encFunctionType,
            (uint8_t *)curbe, sizeof(*curbe)));
    )

    m_cmKernelMap["I_8X8_MODE"]->AllocateSurfaces(&I8x8ModeParams);

    //No need to wait for task finished
    m_cmEvent = CM_NO_EVENT;
    m_cmKernelMap["I_8X8_MODE"]->CreateAndDispatchKernel(m_cmEvent, false, ((!m_singleTaskPhaseSupported)|| m_lastTaskInPhase));

    return eStatus;
}

MOS_STATUS CodechalFeiHevcStateG9Skl::Encode32X32BIntraCheckKernel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    PerfTagSetting perfTag;
    CODECHAL_ENCODE_SET_PERFTAG_INFO(perfTag, CODECHAL_ENCODE_PERFTAG_CALL_32X32_B_IC);

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

    CODECHAL_FEI_HEVC_B_32x32_PU_INTRA_CHECK_CURBE_G9 cmd, *curbe = &cmd;
    MOS_ZeroMemory(curbe, sizeof(*curbe));
    curbe->DW0.FrameWidth      = MOS_ALIGN_CEIL(m_frameWidth, CODECHAL_MACROBLOCK_WIDTH);
    curbe->DW0.FrameHeight     = MOS_ALIGN_CEIL(m_frameHeight, CODECHAL_MACROBLOCK_HEIGHT);

    curbe->DW1.EnableDebugDump = false;
    curbe->DW1.EnableIntraEarlyExit = (m_hevcPicParams->CodingType == I_TYPE) ? 0 : 1;
    curbe->DW1.Flags           = 0;
    curbe->DW1.Log2MinTUSize        = m_hevcSeqParams->log2_min_transform_block_size_minus2 + 2;
    curbe->DW1.SliceType            = m_hevcSliceParams->slice_type;
    curbe->DW1.HMEEnable       = 0;
    curbe->DW1.FASTSurveillanceFlag = (m_hevcPicParams->CodingType == I_TYPE) ? 0 : m_hevcSeqParams->bVideoSurveillance;

    curbe->DW2.QpMultiplier    = 100;
    curbe->DW2.QpValue         = 0;     // MBZ

    PBFrameKernelParams PB32x32Params;
    MOS_ZeroMemory(&PB32x32Params, sizeof(PB32x32Params));

    PB32x32Params.m_cmSurfPer32x32ICOut = &m_32x32PuOutputData.sResource;
    PB32x32Params.m_cmSurfCurrY = &m_rawSurfaceToEnc->OsResource;
    PB32x32Params.m_cmSurfCurrY2 = &m_scaled2xSurface.OsResource;
    PB32x32Params.m_cmSurfSliceMap = &m_sliceMapSurface.OsResource;
    PB32x32Params.m_cmSurfSIF = &m_simplestIntraSurface.OsResource;
    PB32x32Params.m_cmLCUQPSurf = &m_lcuQP.OsResource;

    if (m_cmKernelMap.count("PB_32x32") == 0)
    {
        m_cmKernelMap["PB_32x32"] = new CMRTKernelPB32x32UMD();
        m_cmKernelMap["PB_32x32"]->Init(nullptr, m_cmKernelMap["2xScaling"]->m_cmDev, m_cmKernelMap["2xScaling"]->m_cmQueue, m_cmKernelMap["2xScaling"]->m_cmTask, nullptr);
    }

    m_cmKernelMap["PB_32x32"]->SetupCurbe(curbe);

    CODECHAL_MEDIA_STATE_TYPE encFunctionType = CODECHAL_MEDIA_STATE_32x32_B_INTRA_CHECK;
    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpMDFCurbe(
            encFunctionType,
            (uint8_t *)curbe, sizeof(*curbe)));
    )

    m_cmKernelMap["PB_32x32"]->AllocateSurfaces(&PB32x32Params);

    //No need to wait for task finished
    m_cmEvent = CM_NO_EVENT;
    m_cmKernelMap["PB_32x32"]->CreateAndDispatchKernel(m_cmEvent, false, (!m_singleTaskPhaseSupported));

    return eStatus;
}

MOS_STATUS CodechalFeiHevcStateG9Skl::Encode8x8BPakKernel(
    PCODECHAL_FEI_HEVC_B_MB_ENC_CURBE_G9 pEncBCurbe)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(pEncBCurbe);

    PerfTagSetting perfTag;
    CODECHAL_ENCODE_SET_PERFTAG_INFO(perfTag, CODECHAL_ENCODE_PERFTAG_CALL_PAK_KERNEL);

    //Setup CURBE
    CODECHAL_FEI_HEVC_B_PAK_CURBE_G9  cmd, *curbe = &cmd;
    MOS_ZeroMemory(curbe, sizeof(*curbe));
    curbe->DW0.FrameWidth = MOS_ALIGN_CEIL(m_frameWidth, CODECHAL_MACROBLOCK_WIDTH);
    curbe->DW0.FrameHeight = MOS_ALIGN_CEIL(m_frameHeight, CODECHAL_MACROBLOCK_HEIGHT);

    curbe->DW1.MaxVmvR                 = pEncBCurbe->DW44.MaxVmvR;
    curbe->DW1.Qp                      = pEncBCurbe->DW13.QpPrimeY;
    curbe->DW2.BrcEnable               = pEncBCurbe->DW36.BRCEnable;
    curbe->DW2.LcuBrcEnable            = pEncBCurbe->DW36.LCUBRCEnable;
    curbe->DW2.ScreenContent           = pEncBCurbe->DW47.ScreenContentFlag;
    curbe->DW2.SimplestIntraEnable     = pEncBCurbe->DW47.SkipIntraKrnFlag;
    curbe->DW2.SliceType               = pEncBCurbe->DW4.SliceType;
    curbe->DW2.EnableWA                = 0;
    curbe->DW2.ROIEnable               = (m_hevcPicParams->NumROI > 0);
    curbe->DW2.FASTSurveillanceFlag    = (m_hevcPicParams->CodingType == I_TYPE) ? 0 : m_hevcSeqParams->bVideoSurveillance;
    // KBLControlFlag determines the PAK OBJ format as it varies from Gen9 to Gen9.5+
    curbe->DW2.KBLControlFlag          = UsePlatformControlFlag();
    curbe->DW2.EnableRollingIntra        = m_hevcPicParams->bEnableRollingIntraRefresh;
    curbe->DW2.EnableQualityImprovement  = m_encodeParams.bQualityImprovementEnable;
    curbe->DW3.IntraRefreshQPDelta       = m_hevcPicParams->QpDeltaForInsertedIntra;
    curbe->DW3.IntraRefreshMBNum         = m_hevcPicParams->IntraInsertionLocation;
    curbe->DW3.IntraRefreshUnitInMB      = m_hevcPicParams->IntraInsertionSize;

    PBFrameKernelParams PB8x8PakParams;
    MOS_ZeroMemory(&PB8x8PakParams, sizeof(PB8x8PakParams));

    PB8x8PakParams.m_cmSurfPOCDbuf = &m_resMbCodeSurface;
    PB8x8PakParams.m_bufSize = m_mbCodeSize - m_mvOffset;
    PB8x8PakParams.m_bufOffset = m_mvOffset;
    PB8x8PakParams.m_cmSurfSliceMap = &m_sliceMapSurface.OsResource;
    PB8x8PakParams.m_cmSurfCombinedQP = (MOS_RESOURCE*)m_allocator->GetResource(m_standard, brcInputForEncKernel);
    PB8x8PakParams.m_cmLCUQPSurf = &m_lcuQP.OsResource;
    PB8x8PakParams.m_cmBRCConstSurf   = &m_brcBuffers.sBrcConstantDataBuffer[m_currRecycledBufIdx].OsResource;
    PB8x8PakParams.m_cmSurfMVIndex = &m_mvIndex.sResource;
    PB8x8PakParams.m_cmSurfMVPred = &m_mvpIndex.sResource;

    if (m_cmKernelMap.count("PB_8x8_PAK") == 0)
    {
        m_cmKernelMap["PB_8x8_PAK"] = new CMRTKernelPB8x8PakUMD();
        m_cmKernelMap["PB_8x8_PAK"]->Init(nullptr, m_cmKernelMap["2xScaling"]->m_cmDev, m_cmKernelMap["2xScaling"]->m_cmQueue, m_cmKernelMap["2xScaling"]->m_cmTask, m_cmKernelMap["PB_32x32"]->m_cmProgram);
    }

    m_cmKernelMap["PB_8x8_PAK"]->SetupCurbe(curbe);

    CODECHAL_MEDIA_STATE_TYPE encFunctionType = CODECHAL_MEDIA_STATE_HEVC_B_PAK;
    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpMDFCurbe(
            encFunctionType,
            (uint8_t *)curbe, sizeof(*curbe)));
    )

    m_cmKernelMap["PB_8x8_PAK"]->AllocateSurfaces(&PB8x8PakParams);

    //No need to wait for task finished
    m_cmEvent = CM_NO_EVENT;
    m_cmKernelMap["PB_8x8_PAK"]->CreateAndDispatchKernel(m_cmEvent, false, ((!m_singleTaskPhaseSupported)|| m_lastTaskInPhase));

    return eStatus;
}

MOS_STATUS CodechalFeiHevcStateG9Skl::Encode8x8PBMbEncKernel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    PerfTagSetting perfTag;
    CODECHAL_ENCODE_SET_PERFTAG_INFO(perfTag, CODECHAL_ENCODE_PERFTAG_CALL_MBENC_KERNEL);

    int32_t sliceQp = CalSliceQp();
    uint8_t sliceType = PicCodingTypeToSliceType(m_pictureCodingType);

    if (m_feiPicParams->FastIntraMode)
    {
        // When TU=7, lambda is not computed in the 32x32 MD stage for it is skipped.
        CalcLambda(sliceType, INTRA_TRANSFORM_HAAR);
    }
    LoadCosts(sliceType, (uint8_t)sliceQp, INTRA_TRANSFORM_REGULAR);

    uint8_t mbCodeIdxForTempMVP = 0xFF;
    if(m_pictureCodingType != I_TYPE)
    {
        if (m_hevcPicParams->CollocatedRefPicIndex != 0xFF && m_hevcPicParams->CollocatedRefPicIndex < CODEC_MAX_NUM_REF_FRAME_HEVC)
        {
            uint8_t FrameIdx = m_hevcPicParams->RefFrameList[m_hevcPicParams->CollocatedRefPicIndex].FrameIdx;

            mbCodeIdxForTempMVP = m_refList[FrameIdx]->ucScalingIdx;
        }

        if (mbCodeIdxForTempMVP == 0xFF && m_hevcSliceParams->slice_temporal_mvp_enable_flag)
        {
            // Temporal reference MV index is invalid and so disable the temporal MVP
            CODECHAL_ENCODE_ASSERT(false);
            m_hevcSliceParams->slice_temporal_mvp_enable_flag = false;
        }
    }
    else
    {
        mbCodeIdxForTempMVP = 0;
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(GenerateWalkingControlRegion());

    //Setup CURBE
    uint8_t forwardTransformThd[7] = { 0 };
    CalcForwardCoeffThd(forwardTransformThd, sliceQp);

    uint32_t curbeSize = 0;
    void *defaultCurbe = (void *)GetDefaultCurbeEncBKernel(curbeSize);
    CODECHAL_ENCODE_ASSERT(defaultCurbe);

    CODECHAL_FEI_HEVC_B_MB_ENC_CURBE_G9 cmd, *curbe = &cmd;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(curbe, sizeof(cmd), defaultCurbe, curbeSize));

    bool transform_8x8_mode_flag = true;
    uint32_t SearchPath              = (m_feiPicParams->SearchWindow == 5) ? 2 : 1;  // 2 means full search, 1 means diamand search
    uint32_t LenSP                   = m_feiPicParams->LenSP;
    uint32_t RefWidth                = (m_feiPicParams->RefWidth < 20) ? 20 : m_feiPicParams->RefWidth;
    uint32_t RefHeight               = (m_feiPicParams->RefHeight < 20) ? 20 : m_feiPicParams->RefHeight;

    switch (m_feiPicParams->SearchWindow)
    {
    case 0:
        // not use predefined search window
        if ((m_feiPicParams->SearchPath != 0) && (m_feiPicParams->SearchPath != 1) && (m_feiPicParams->SearchPath != 2))
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Invalid picture FEI MB ENC input SearchPath for SearchWindow=0 case!!!.");
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            return eStatus;
        }
        SearchPath = m_feiPicParams->SearchPath;
        if(((RefWidth * RefHeight) > 2048) || (RefWidth > 64) || (RefHeight > 64))
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Invalid picture FEI MB ENC input RefWidth/RefHeight size for SearchWindow=0 case!!!.");
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            return eStatus;
        }
        break;
    case 1:
        // Tiny SUs 24x24 window
        RefWidth  = 24;
        RefHeight = 24;
        LenSP     = 4;
        break;
    case 2:
        // Small SUs 28x28 window
        RefWidth  = 28;
        RefHeight = 28;
        LenSP     = 9;
        break;
    case 3:
        // Diamond SUs 48x40 window
        RefWidth  = 48;
        RefHeight = 40;
        LenSP     = 16;
        break;
    case 4:
        // Large Diamond SUs 48x40 window
        RefWidth  = 48;
        RefHeight = 40;
        LenSP     = 32;
        break;
    case 5:
        // Exhaustive SUs 48x40 window
        RefWidth  = 48;
        RefHeight = 40;
        LenSP     = 48;
        if (m_hevcSeqParams->TargetUsage != 7)
        {
            if (m_pictureCodingType == B_TYPE)
            {
                LenSP = 48;
            } else {
                LenSP = 57;
            }
        } else {
            LenSP = 25;
        }
        break;
    default:
        CODECHAL_ENCODE_ASSERTMESSAGE("Invalid picture FEI MB ENC SearchWindow value for HEVC FEI on SKL!!!.");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    if((m_pictureCodingType == B_TYPE) && (curbe->DW3.BMEDisableFBR == 0))
    {
        if(RefWidth > 32)
        {
            RefWidth  = 32;
        }
        if(RefHeight > 32)
        {
            RefHeight = 32;
        }
    }

    curbe->DW0.AdaptiveEn         = m_feiPicParams->AdaptiveSearch;
    curbe->DW0.T8x8FlagForInterEn = transform_8x8_mode_flag;
    curbe->DW2.PicWidth    = m_picWidthInMb;
    curbe->DW2.LenSP       = LenSP;
    curbe->DW3.SrcAccess   = curbe->DW3.RefAccess = 0;
    if (m_feiPicParams->FastIntraMode)
    {
        curbe->DW3.FTEnable    = (m_ftqBasedSkip[0x07] >> 1) & 0x01;
    }
    else
    {
        curbe->DW3.FTEnable    = (m_ftqBasedSkip[0x04] >> 1) & 0x01;
    }
    curbe->DW3.SubPelMode                         = m_feiPicParams->SubPelMode;

    curbe->DW4.PicHeightMinus1               = m_picHeightInMb - 1;
    curbe->DW4.EnableStatsDataDump           = m_encodeParams.bReportStatisticsEnabled;
    curbe->DW4.HMEEnable                     = 0;
    curbe->DW4.SliceType                     = sliceType;
    curbe->DW4.EnableQualityImprovement      = m_encodeParams.bQualityImprovementEnable;
    curbe->DW4.UseActualRefQPValue           = false;

    curbe->DW5.RefWidth                      = RefWidth;
    curbe->DW5.RefHeight                     = RefHeight;

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
    int32_t qPi                  = (int32_t)CodecHal_Clip3((-qpBdOffsetC), 51, (sliceQp + m_hevcPicParams->pps_cb_qp_offset));
    int32_t QPc = (qPi < 30) ? qPi : QPcTable[qPi - 30];
    curbe->DW13.QpPrimeCb= QPc + qpBdOffsetC;
    qPi                          = (int32_t)CodecHal_Clip3((-qpBdOffsetC), 51, (sliceQp + m_hevcPicParams->pps_cr_qp_offset));
    QPc = (qPi < 30) ? qPi : QPcTable[qPi - 30];
    curbe->DW13.QpPrimeCr= QPc;

    curbe->DW14.SICFwdTransCoeffThreshold_0 = forwardTransformThd[0];
    curbe->DW14.SICFwdTransCoeffThreshold_1 = forwardTransformThd[1];
    curbe->DW14.SICFwdTransCoeffThreshold_2 = forwardTransformThd[2];

    curbe->DW15.SICFwdTransCoeffThreshold_3 = forwardTransformThd[3];
    curbe->DW15.SICFwdTransCoeffThreshold_4 = forwardTransformThd[4];
    curbe->DW15.SICFwdTransCoeffThreshold_5 = forwardTransformThd[5];
    curbe->DW15.SICFwdTransCoeffThreshold_6 = forwardTransformThd[6];

    if (SearchPath == 1)
    {
        // diamond search
        if (m_pictureCodingType == P_TYPE)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(&(curbe->DW16), 14 * sizeof(uint32_t), &(m_encBTu7PCurbeInit[16]), 14 * sizeof(uint32_t)));
        }
        else if (m_pictureCodingType == B_TYPE)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(&(curbe->DW16), 14 * sizeof(uint32_t), &(m_encBTu7BCurbeInit[16]), 14 * sizeof(uint32_t)));
        }
    }
    else if((SearchPath != 0) && (SearchPath != 2))
    {
        // default 0 and 2 are full sparil search
        CODECHAL_ENCODE_ASSERT(false);
    }

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
            float lambda_me = (float)m_qpLambdaMe[CODECHAL_ENCODE_HEVC_B_SLICE][sliceQp];
            if (m_encodeParams.bQualityImprovementEnable)
            {
                curbe->DW40.TransformThreshold0 = (uint16_t) (lambda_me * 56.25 + 0.5);
                curbe->DW40.TransformThreshold1 = (uint16_t) (lambda_me * 21 + 0.5);
                curbe->DW41.TransformThreshold2 = (uint16_t) (lambda_me * 9 + 0.5);
            }
        }
    }

    curbe->DW42.NumMVPredictorsL0      = m_feiPicParams->NumMVPredictorsL0;
    curbe->DW42.NumMVPredictorsL1      = m_feiPicParams->NumMVPredictorsL1;
    curbe->DW42.PerLCUQP               = m_encodeParams.bMbQpDataEnabled;
    curbe->DW42.PerCTBInput            = m_feiPicParams->bPerCTBInput;
    curbe->DW42.CTBDistortionOutput    = m_feiPicParams->bDistortionEnable;
    curbe->DW42.MultiPredL0            = m_feiPicParams->MultiPredL0;
    curbe->DW42.MultiPredL1            = m_feiPicParams->MultiPredL1;
    curbe->DW42.MVPredictorBlockSize   = m_feiPicParams->MVPredictorInput;

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

    curbe->DW47.NumRegionsInSlice      = m_numRegionsInSlice;
    curbe->DW47.TypeOfWalkingPattern   = m_enable26WalkingPattern;
    curbe->DW47.ChromaFlatnessCheckFlag= (m_feiPicParams->FastIntraMode) ? 0 : 1;
    curbe->DW47.EnableIntraEarlyExit   = (m_feiPicParams->FastIntraMode) ? 0 : 1;
    curbe->DW47.SkipIntraKrnFlag       = (m_feiPicParams->FastIntraMode) ? 1 : 0;
    curbe->DW47.CollocatedFromL0Flag   = m_hevcSliceParams->collocated_from_l0_flag;
    curbe->DW47.IsLowDelay             = m_lowDelay;
    curbe->DW47.ScreenContentFlag      = m_hevcPicParams->bScreenContent;
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

        m_firstIntraRefresh               = false;
        m_frameNumWithoutIntraRefresh     = 0;
    }
    else if (m_pictureCodingType != I_TYPE) // don't increment num frames w/o refresh in case of TU7 I frames
    {
        m_frameNumWithoutIntraRefresh++;
    }

    PBFrameKernelParams PB8x8MbEncParams;
    MOS_ZeroMemory(&PB8x8MbEncParams, sizeof(PB8x8MbEncParams));

    PB8x8MbEncParams.m_width = curbe->DW6.FrameWidth;
    PB8x8MbEncParams.m_height = curbe->DW6.FrameHeight;

    for(uint32_t surfaceIdx = 0; surfaceIdx < 8; surfaceIdx++)
    {
        CODEC_PICTURE refPic = m_hevcSliceParams->RefPicList[LIST_0][surfaceIdx];
        if (!CodecHal_PictureIsInvalid(refPic) &&
            !CodecHal_PictureIsInvalid(m_hevcPicParams->RefFrameList[refPic.FrameIdx]))
        {
            uint8_t idx                                                 = m_hevcPicParams->RefFrameList[refPic.FrameIdx].FrameIdx;
            PB8x8MbEncParams.m_cmSurfRef0[PB8x8MbEncParams.m_ucRefNum0] = &m_refList[idx]->sRefBuffer.OsResource;
            PB8x8MbEncParams.m_ucRefNum0++;
        }

        refPic = m_hevcSliceParams->RefPicList[LIST_1][surfaceIdx];
        if (!CodecHal_PictureIsInvalid(refPic) &&
            !CodecHal_PictureIsInvalid(m_hevcPicParams->RefFrameList[refPic.FrameIdx]))
        {
            uint8_t idx                                                 = m_hevcPicParams->RefFrameList[refPic.FrameIdx].FrameIdx;
            PB8x8MbEncParams.m_cmSurfRef1[PB8x8MbEncParams.m_ucRefNum1] = &m_refList[idx]->sRefBuffer.OsResource;
            PB8x8MbEncParams.m_ucRefNum1++;
        }
    }

    PB8x8MbEncParams.m_cmSurfCurrY = &m_rawSurfaceToEnc->OsResource;
    PB8x8MbEncParams.m_cmSurfPOCDbuf = &m_resMbCodeSurface;
    PB8x8MbEncParams.m_bufSize = m_mbCodeSize - m_mvOffset;
    PB8x8MbEncParams.m_bufOffset = m_mvOffset;
    if(mbCodeIdxForTempMVP == 0xFF)
    {
        PB8x8MbEncParams.m_cmSurfColRefData = nullptr;
    }
    else
    {
        PB8x8MbEncParams.m_cmSurfColRefData = m_trackedBuf->GetMvTemporalBuffer(mbCodeIdxForTempMVP);
    }
    PB8x8MbEncParams.m_cmSurfIntraDist = &m_intraDist.sResource;
    PB8x8MbEncParams.m_cmSurfMinDist = &m_minDistortion.OsResource;
    PB8x8MbEncParams.m_cmSurfSliceMap = &m_sliceMapSurface.OsResource;
    PB8x8MbEncParams.m_cmSurfVMEIN = &m_vmeSavedUniSic.sResource;
    PB8x8MbEncParams.m_cmSurfSIF = &m_simplestIntraSurface.OsResource;
    PB8x8MbEncParams.m_cmSurfCombinedQP = (MOS_RESOURCE*)m_allocator->GetResource(m_standard, brcInputForEncKernel);
    PB8x8MbEncParams.m_cmLCUQPSurf = &m_lcuQP.OsResource;
    PB8x8MbEncParams.m_cmBRCConstSurf   = &m_brcBuffers.sBrcConstantDataBuffer[m_currRecycledBufIdx].OsResource;
    PB8x8MbEncParams.m_cmWaveFrontMap = &m_concurrentThreadSurface[m_concurrentThreadIndex].OsResource;
    if (++m_concurrentThreadIndex >= NUM_CONCURRENT_THREAD)
    {
        m_concurrentThreadIndex = 0;
    }
    PB8x8MbEncParams.m_cmSurfMVIndex = &m_mvIndex.sResource;
    PB8x8MbEncParams.m_cmSurfMVPred = &m_mvpIndex.sResource;
    if (m_feiPicParams->MVPredictorInput)
    {
        PB8x8MbEncParams.m_cmSurfMVPredictor = &m_feiPicParams->resMVPredictor;
    }
    else
    {
        PB8x8MbEncParams.m_cmSurfMVPredictor = nullptr;
    }

    if (m_feiPicParams->bPerCTBInput)
    {
        PB8x8MbEncParams.m_cmSurfPerCTBInput = &m_feiPicParams->resCTBCtrl;
    }
    else
    {
        PB8x8MbEncParams.m_cmSurfPerCTBInput = nullptr;
    }

    //to avoid multi contexts in case per-frame control of FastIntraMode, always use 2xScaling kernel to initialize the context.
    if (m_cmKernelMap.count("2xScaling") == 0)
    {
        m_cmKernelMap["2xScaling"] = new CMRTKernelDownScalingUMD();
        m_cmKernelMap["2xScaling"]->Init((void *)m_osInterface->pOsContext);
    }

    //in case PB_32x32 isn't initialized when using FastIntraMode for per-frame control (I: disable; P/B: enable)
    if (m_cmKernelMap.count("PB_32x32") == 0)
    {
        m_cmKernelMap["PB_32x32"] = new CMRTKernelPB32x32UMD();
        m_cmKernelMap["PB_32x32"]->Init(nullptr, m_cmKernelMap["2xScaling"]->m_cmDev, m_cmKernelMap["2xScaling"]->m_cmQueue, m_cmKernelMap["2xScaling"]->m_cmTask, nullptr);
    }

    if (m_pictureCodingType == I_TYPE && m_feiPicParams->FastIntraMode)
    {
        if (m_cmKernelMap.count("I_8x8_MBENC") == 0)
        {
            m_cmKernelMap["I_8x8_MBENC"] = new CMRTKernelB8x8MbEncUMD();
            m_cmKernelMap["I_8x8_MBENC"]->Init(nullptr, m_cmKernelMap["2xScaling"]->m_cmDev, m_cmKernelMap["2xScaling"]->m_cmQueue, m_cmKernelMap["2xScaling"]->m_cmTask, m_cmKernelMap["PB_32x32"]->m_cmProgram);
        }

        m_cmKernelMap["I_8x8_MBENC"]->SetupCurbe(curbe);
        m_cmKernelMap["I_8x8_MBENC"]->AllocateSurfaces(&PB8x8MbEncParams);

        //No need to wait for task finished
        m_cmEvent = CM_NO_EVENT;
        m_cmKernelMap["I_8x8_MBENC"]->CreateAndDispatchKernel(m_cmEvent, false, (!m_singleTaskPhaseSupported));
    }
    else if (m_pictureCodingType == B_TYPE)
    {
        if (m_cmKernelMap.count("B_8x8_MBENC") == 0)
        {
            m_cmKernelMap["B_8x8_MBENC"] = new CMRTKernelB8x8MbEncUMD();
            m_cmKernelMap["B_8x8_MBENC"]->Init(nullptr, m_cmKernelMap["2xScaling"]->m_cmDev, m_cmKernelMap["2xScaling"]->m_cmQueue, m_cmKernelMap["2xScaling"]->m_cmTask, m_cmKernelMap["PB_32x32"]->m_cmProgram);
        }

        m_cmKernelMap["B_8x8_MBENC"]->SetupCurbe(curbe);
        m_cmKernelMap["B_8x8_MBENC"]->AllocateSurfaces(&PB8x8MbEncParams);

        //No need to wait for task finished
        m_cmEvent = CM_NO_EVENT;
        m_cmKernelMap["B_8x8_MBENC"]->CreateAndDispatchKernel(m_cmEvent, false, (!m_singleTaskPhaseSupported));
    }
    else if (m_pictureCodingType == P_TYPE)
    {
        if (m_cmKernelMap.count("P_8x8_MBENC") == 0)
        {
            m_cmKernelMap["P_8x8_MBENC"] = new CMRTKernelP8x8MbEncUMD();
            m_cmKernelMap["P_8x8_MBENC"]->Init(nullptr, m_cmKernelMap["2xScaling"]->m_cmDev, m_cmKernelMap["2xScaling"]->m_cmQueue, m_cmKernelMap["2xScaling"]->m_cmTask, m_cmKernelMap["PB_32x32"]->m_cmProgram);
        }
        m_cmKernelMap["P_8x8_MBENC"]->SetupCurbe(curbe);
        m_cmKernelMap["P_8x8_MBENC"]->AllocateSurfaces(&PB8x8MbEncParams);

        //No need to wait for task finished
        m_cmEvent = CM_NO_EVENT;
        m_cmKernelMap["P_8x8_MBENC"]->CreateAndDispatchKernel(m_cmEvent, false, (!m_singleTaskPhaseSupported));
    }

    CODECHAL_MEDIA_STATE_TYPE encFunctionType = CODECHAL_MEDIA_STATE_HEVC_B_MBENC;
    if (m_pictureCodingType == P_TYPE)
    {
        //P frame curbe only use the DW0~DW75
        CODECHAL_DEBUG_TOOL(
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpMDFCurbe(
                encFunctionType,
                (uint8_t *)curbe, sizeof(*curbe) - sizeof(uint32_t)));
        )
    }
    else
    {
        CODECHAL_DEBUG_TOOL(
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpMDFCurbe(
                encFunctionType,
                (uint8_t *)curbe, sizeof(*curbe)));
        )
    }

    m_lastTaskInPhase = true;
    eStatus = Encode8x8BPakKernel(curbe);
    return eStatus;
}

#endif

MOS_STATUS CodechalFeiHevcStateG9Skl::EncodeKernelFunctions()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    m_feiPicParams = (CodecEncodeHevcFeiPicParams *)m_encodeParams.pFeiPicParams;

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
            m_rawSurfaceToEnc,
            CodechalDbgAttr::attrEncodeRawInputSurface,
            "SrcSurf"));
    )

    if (m_pakOnlyTest)
    {
        // Skip all ENC kernel operations for now it is in the PAK only test mode.
        // PAK and CU records will be passed via the app
        return eStatus;
    }

    if (m_brcEnabled || m_hmeEnabled)
    {
        eStatus = MOS_STATUS_UNKNOWN;
        CODECHAL_ENCODE_ASSERTMESSAGE("HEVC FEI does not support BRC and HMEenabled.");
        return eStatus;
    }

    if(m_osInterface->bSimIsActive)
    {
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

    // config LCU QP input
    if (m_encodeParams.bMbQpDataEnabled)
    {
        // Setup Lamda/Cost table for LCU QP mode
        auto psBrcConstantData = &m_brcBuffers.sBrcConstantDataBuffer[m_currRecycledBufIdx];
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetupBrcConstantTable(psBrcConstantData));

        if (m_encodeParams.psMbQpDataSurface)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(Convert1byteTo2bytesQPperLCU(m_encodeParams.psMbQpDataSurface, &m_lcuQP));
            m_surfaceParams[SURFACE_LCU_QP].psSurface = &m_lcuQP;
        }
    }

    CODECHAL_DEBUG_TOOL(
        if (m_feiPicParams->bPerBlockQP) {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpSurface(
                &m_lcuQP,
                CodechalDbgAttr::attrInput,
                "HEVC_B_MBENC_MB_QP",
                CODECHAL_MEDIA_STATE_HEVC_B_MBENC));
        }

        if (m_feiPicParams->MVPredictorInput) {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                &m_feiPicParams->resMVPredictor,
                "HEVC_B_MBENC_ConstantData_In",
                CodechalDbgAttr::attrInput,
                m_feiPicParams->resMVPredictor.iSize,
                0,
                CODECHAL_MEDIA_STATE_HEVC_B_MBENC));
        })

    if(m_feiPicParams->FastIntraMode)
    {
        if (m_hevcPicParams->CodingType == I_TYPE)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(Encode8x8PBMbEncKernel());
        }
    }
    else
    {
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
                                "2xScaledSurf"))

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

        if (m_feiPicParams->MVPredictorInput)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
                &m_surfaceParams[SURFACE_FEI_EXTERNAL_MVP],
                &m_feiPicParams->resMVPredictor,
                m_feiPicParams->resMVPredictor.iSize,
                0,
                m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value,
                0,
                false));
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
                    CODECHAL_ENCODE_ASSERT(0);
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
#ifdef HEVC_FEI_ENABLE_CMRT

    for (CmKernelMapType::iterator it = m_cmKernelMap.begin(); it != m_cmKernelMap.end(); it++)
    {
        it->second->DestroySurfResources();
    }

#endif

    // Notify PAK engine once ENC is done
    if (!m_pakOnlyTest && !Mos_ResourceIsNull(&m_resSyncObjectRenderContextInUse))
    {
        MOS_SYNC_PARAMS syncParams = g_cInitSyncParams;
        syncParams.GpuContext = m_renderContext;
        syncParams.presSyncResource = &m_resSyncObjectRenderContextInUse;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineSignal(m_osInterface, &syncParams));
    }

    return eStatus;
}

MOS_STATUS CodechalFeiHevcStateG9Skl::Initialize(CodechalSetting * settings)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    // common initilization
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncHevcState::Initialize(settings));

    m_cscDsState->EnableMmc();

    m_brcBuffers.dwBrcConstantSurfaceWidth  = BRC_CONSTANT_SURFACE_WIDTH;
    m_brcBuffers.dwBrcConstantSurfaceHeight = BRC_CONSTANT_SURFACE_HEIGHT;

    // LCU size is 32x32 in Gen9
    m_widthAlignedMaxLcu  = MOS_ALIGN_CEIL(m_frameWidth, 32);
    m_heightAlignedMaxLcu = MOS_ALIGN_CEIL(m_frameHeight, 32);

    m_brcEnabled              = false;
    m_hmeEnabled              = false;
    m_hmeSupported            = false;
    m_16xMeUserfeatureControl = false;
    m_16xMeSupported          = false;
    m_32xMeUserfeatureControl = false;
    m_32xMeSupported          = false;

    // regkey setup
    MOS_USER_FEATURE_VALUE_DATA userFeatureData;
    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_SINGLE_TASK_PHASE_ENABLE_ID,
        &userFeatureData,
        m_osInterface->pOsContext);
    m_singleTaskPhaseSupported = (userFeatureData.i32Data) ? true : false;

    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_26Z_ENABLE_ID,
        &userFeatureData,
        m_osInterface->pOsContext);
    m_enable26WalkingPattern = (userFeatureData.i32Data) ? false : true;

    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    eStatus = MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_REGION_NUMBER_ID,
        &userFeatureData,
        m_osInterface->pOsContext);

    if (eStatus == MOS_STATUS_SUCCESS)
    {
        // Region number must be greater than 1
        m_numRegionsInSlice = (userFeatureData.i32Data < 1) ? 1 : userFeatureData.i32Data;
    }
    else
    {
        // Reset the status to success if regkey is not set
        eStatus = MOS_STATUS_SUCCESS;
    }

    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_NUM_8x8_INTRA_KERNEL_SPLIT,
        &userFeatureData,
        m_osInterface->pOsContext);
    m_numMb8x8IntraKernelSplit = (userFeatureData.i32Data < 0) ? 0 : userFeatureData.i32Data;

    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_NUM_B_KERNEL_SPLIT,
        &userFeatureData,
        m_osInterface->pOsContext);
    m_numMbBKernelSplit = (userFeatureData.i32Data < 0) ? 0 : userFeatureData.i32Data;

    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_POWER_SAVING,
        &userFeatureData,
        m_osInterface->pOsContext);
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

uint32_t CodechalFeiHevcStateG9Skl::GetMaxBtCount()
{
    auto wBtIdxAlignment = m_stateHeapInterface->pStateHeapInterface->GetBtIdxAlignment();

    // 6 I kernels
    uint32_t uiBtCountPhase1 = MOS_ALIGN_CEIL(m_mbEncKernelStates[CODECHAL_HEVC_MBENC_2xSCALING].KernelParams.iBTCount, wBtIdxAlignment) +
                               MOS_ALIGN_CEIL(m_mbEncKernelStates[CODECHAL_HEVC_MBENC_16x16SAD].KernelParams.iBTCount, wBtIdxAlignment) +
                               MOS_ALIGN_CEIL(m_mbEncKernelStates[CODECHAL_HEVC_MBENC_16x16MD].KernelParams.iBTCount, wBtIdxAlignment) +
                               MOS_ALIGN_CEIL(m_mbEncKernelStates[CODECHAL_HEVC_MBENC_8x8PU].KernelParams.iBTCount, wBtIdxAlignment) +
                               MOS_ALIGN_CEIL(m_mbEncKernelStates[CODECHAL_HEVC_MBENC_8x8FMODE].KernelParams.iBTCount, wBtIdxAlignment);

    uiBtCountPhase1 += MOS_MAX(
        MOS_ALIGN_CEIL(m_mbEncKernelStates[CODECHAL_HEVC_MBENC_32x32MD].KernelParams.iBTCount, wBtIdxAlignment),
        MOS_ALIGN_CEIL(m_mbEncKernelStates[CODECHAL_HEVC_MBENC_32x32INTRACHECK].KernelParams.iBTCount, wBtIdxAlignment));

    if (MEDIA_IS_SKU(m_skuTable, FtrEncodeHEVC10bit))
    {
        uiBtCountPhase1 += MOS_ALIGN_CEIL(m_mbEncKernelStates[CODECHAL_HEVC_FEI_MBENC_DS_COMBINED].KernelParams.iBTCount, wBtIdxAlignment);
    }

    // two B kernels
    uint32_t uiBtCountPhase2 = MOS_ALIGN_CEIL(m_mbEncKernelStates[CODECHAL_HEVC_FEI_MBENC_BENC].KernelParams.iBTCount, wBtIdxAlignment) +
                               MOS_ALIGN_CEIL(m_mbEncKernelStates[CODECHAL_HEVC_FEI_MBENC_BPAK].KernelParams.iBTCount, wBtIdxAlignment);

    uint32_t uiMaxBtCount = MOS_MAX(uiBtCountPhase1, uiBtCountPhase2);

    return uiMaxBtCount;
}

MOS_STATUS CodechalFeiHevcStateG9Skl::AllocateEncResources()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    m_sliceMap = (PCODECHAL_ENCODE_HEVC_SLICE_MAP)MOS_AllocAndZeroMemory(
        m_widthAlignedMaxLcu * m_heightAlignedMaxLcu * sizeof(m_sliceMap[0]));
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_sliceMap);

    uint32_t Downscaling2xWidth  = m_widthAlignedMaxLcu >> 1;
    uint32_t Downscaling2xHeight = m_heightAlignedMaxLcu >> 1;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateSurface(
        &m_scaled2xSurface,
        Downscaling2xWidth,
        Downscaling2xHeight,
        "2x Downscaling"));

    uint32_t uiWidth  = m_widthAlignedMaxLcu >> 3;
    uint32_t uiHeight = m_heightAlignedMaxLcu >> 5;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer2D(
        &m_sliceMapSurface,
        uiWidth,
        uiHeight,
        "Slice Map"));

    uint32_t uiSize = 32 * (m_widthAlignedMaxLcu >> 5) * (m_heightAlignedMaxLcu >> 5);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer(
        &m_32x32PuOutputData,
        uiSize,
        "32x32 PU Output Data"));

    uiSize = 8 * 4 * (m_widthAlignedMaxLcu >> 4) * (m_heightAlignedMaxLcu >> 4);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer(
        &m_sad16x16Pu,
        uiSize,
        "SAD 16x16 PU"));

    // need 64 bytes for statistics report .
    uiSize = 64 * (m_widthAlignedMaxLcu >> 4) * (m_heightAlignedMaxLcu >> 4);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer(
        &m_vme8x8Mode,
        uiSize,
        "VME 8x8 mode"));

    uiSize = 32 * (m_widthAlignedMaxLcu >> 3) * (m_heightAlignedMaxLcu >> 3);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer(
        &m_intraMode,
        uiSize,
        "Intra mode"));

    uiSize = 16 * (m_widthAlignedMaxLcu >> 4) * (m_heightAlignedMaxLcu >> 4);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer(
        &m_intraDist,
        uiSize,
        "Intra dist"));

    // Change the surface size
    uiWidth  = m_widthAlignedMaxLcu >> 1;
    uiHeight = m_heightAlignedMaxLcu >> 4;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer2D(
        &m_minDistortion,
        uiWidth,
        uiHeight,
        "Min distortion surface"));

    // Allocate FEI 2D 2bytes LCU QP surface
    uiWidth  = MOS_ALIGN_CEIL((m_widthAlignedMaxLcu >> 4), 64);
    uiHeight = MOS_ALIGN_CEIL((m_heightAlignedMaxLcu >> 5), 4);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer2D(
        &m_lcuQP,
        uiWidth,
        uiHeight,
        "LCU_QP surface"));

    uiWidth = sizeof(CODECHAL_ENCODE_HEVC_WALKING_CONTROL_REGION);
    uiHeight = HEVC_CONCURRENT_SURFACE_HEIGHT;
    for (uint32_t i = 0; i < NUM_CONCURRENT_THREAD; i++)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer2D(
            &m_concurrentThreadSurface[i],
            uiWidth,
            uiHeight,
            "Concurrent Thread"));
    }

    //uiSize = (dwWidthAlignedMaxLCU * dwHeightAlignedMaxLCU / 4);
    uiSize = (m_widthAlignedMaxLcu * m_heightAlignedMaxLcu / 4) + GPUMMU_WA_PADDING;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer(
        &m_mvIndex,
        uiSize,
        "MV index surface"));

    //uiSize = (dwWidthAlignedMaxLCU * dwHeightAlignedMaxLCU / 2);
    uiSize = (m_widthAlignedMaxLcu * m_heightAlignedMaxLcu / 2) + GPUMMU_WA_PADDING;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer(
        &m_mvpIndex,
        uiSize,
        "MVP index surface"));

    uiSize = m_widthAlignedMaxLcu * m_heightAlignedMaxLcu;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer(
        &m_vmeSavedUniSic,
        uiSize,
        "VME Saved UniSic surface"));

    uiWidth  = m_widthAlignedMaxLcu >> 3;
    uiHeight = m_heightAlignedMaxLcu >> 5;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer2D(
        &m_simplestIntraSurface,
        uiWidth,
        uiHeight,
        "Simplest Intra surface"));

    m_allocator->AllocateResource(m_standard, 1024, 1, brcInputForEncKernel, "brcInputForEncKernel", true);

    if (MEDIA_IS_SKU(m_skuTable, FtrEncodeHEVC10bit))
    {
        // adding 10 bit support for KBL : output surface for format conversion from 10bit to 8 bit
        for (uint32_t i = 0; i < NUM_FORMAT_CONV_FRAMES; i++)
        {
            if (Mos_ResourceIsNull(&m_formatConvertedSurface[i].OsResource))
            {
                uiWidth  = m_widthAlignedMaxLcu;
                uiHeight = m_heightAlignedMaxLcu;

                CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateSurface(
                    &m_formatConvertedSurface[i],
                    uiWidth,
                    uiHeight,
                    "Format Converted Surface"));
            }
        }

        if (Mos_ResourceIsNull(&m_resMbStatisticsSurface.sResource))
        {
            uiSize = 52 * m_picWidthInMb * m_picHeightInMb; // 13 DWs or 52 bytes for statistics per MB

            CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer(
                &m_resMbStatisticsSurface,
                uiSize,
                "MB stats surface"));
        }
    }

    // ROI
    // ROI buffer size uses MB units for HEVC, not LCU
    uiWidth  = MOS_ALIGN_CEIL(m_picWidthInMb * 4, 64);
    uiHeight = MOS_ALIGN_CEIL(m_picHeightInMb, 8);

    MOS_ZeroMemory(&m_roiSurface, sizeof(m_roiSurface));
    m_roiSurface.TileType       = MOS_TILE_LINEAR;
    m_roiSurface.bArraySpacing  = true;
    m_roiSurface.Format         = Format_Buffer_2D;
    m_roiSurface.dwWidth        = uiWidth;
    m_roiSurface.dwPitch        = uiWidth;
    m_roiSurface.dwHeight       = uiHeight;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer2D(
        &m_roiSurface,
        uiWidth,
        uiHeight,
        "ROI Buffer"));

    return eStatus;
}

MOS_STATUS CodechalFeiHevcStateG9Skl::FreeEncResources()
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

    for (auto i = 0; i < NUM_FORMAT_CONV_FRAMES; i++)
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

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_lcuQP.OsResource);

    for (uint32_t i = 0; i < NUM_CONCURRENT_THREAD; i++)
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &m_concurrentThreadSurface[i].OsResource);
    }

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_simplestIntraSurface.OsResource);

    MOS_FreeMemory(m_sliceMap);
    m_sliceMap = nullptr;

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_roiSurface.OsResource);

#ifdef HEVC_FEI_ENABLE_CMRT

     for (CmKernelMapType::iterator it = m_cmKernelMap.begin(); it != m_cmKernelMap.end(); it++)
     {
          it->second->DestroyKernelResources();
     }
     if (m_cmKernelMap.count("2xScaling"))
     {
         m_cmKernelMap["2xScaling"]->DestroyProgramResources();
     }
     if (m_cmKernelMap.count("I_32x32"))
     {
         m_cmKernelMap["I_32x32"]->DestroyProgramResources();
     }
     if (m_cmKernelMap.count("PB_32x32"))
     {
         m_cmKernelMap["PB_32x32"]->DestroyProgramResources();
     }
     if (m_cmKernelMap.count("2xScaling"))
     {
         m_cmKernelMap["2xScaling"]->Destroy();
     }

     for (CmKernelMapType::iterator it = m_cmKernelMap.begin(); it != m_cmKernelMap.end(); it++)
     {
         delete it->second;
     }

     m_cmKernelMap.clear();

#endif

    return eStatus;
}

MOS_STATUS CodechalFeiHevcStateG9Skl::InitSurfaceInfoTable()
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
        &m_lcuQP,
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

    // skip SURFACE_HME_MVP and SURFACE_HME_DIST from HME since FEI alsways disables HME

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
    for (uint32_t i = 0; i < NUM_CONCURRENT_THREAD; i++)
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

#if 0
    param = &m_surfaceParams[SURFACE_PU_STATS];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
                param,
                &m_encStatsBuffers.m_puStatsSurface,
                m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_HME_DOWNSAMPLED_ENCODE].Value,
                0,
                m_verticalLineStride,
                true));

    param = &m_surfaceParams[SURFACE_8X8_PU_HAAR_DIST];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
                param,
                &m_encStatsBuffers.m_8x8PuHaarDist,
                m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_HME_DOWNSAMPLED_ENCODE].Value,
                0,
                m_verticalLineStride,
                true));

    param = &m_surfaceParams[SURFACE_8X8_PU_FRAME_STATS];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
                param,
                &m_encStatsBuffers.m_8x8PuFrameStats.sResource,
                m_encStatsBuffers.m_8x8PuFrameStats.dwSize,
                0,
                m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value,
                0,
                true));

    param = &m_surfaceParams[SURFACE_MB_ENC_STATS];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
                param,
                &m_encStatsBuffers.m_mbEncStatsSurface,
                m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_HME_DOWNSAMPLED_ENCODE].Value,
                0,
                m_verticalLineStride,
                true));

    param = &m_surfaceParams[SURFACE_MB_ENC_FRAME_STATS];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
                param,
                &m_encStatsBuffers.m_mbEncFrameStats.sResource,
                m_encStatsBuffers.m_mbEncFrameStats.dwSize,
                0,
                m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value,
                0,
                true));

    param = &m_surfaceParams[SURFACE_FEI_EXTERNAL_MVP];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        param,
        &m_feiPicParams->resMVPredictor,
        m_feiPicParams->resMVPredictor.iSize,
        0,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value,
        0,
        false));

    param = &m_surfaceParams[SURFACE_FEI_PER_LCU_QP];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        param,
        &m_feiPicParams->resCTBQp,
        m_feiPicParams->resCTBQp.iSize,
        0,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value,
        0,
        false));

    param = &m_surfaceParams[SURFACE_FEI_PER_CTB_CTRL];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        param,
        &m_feiPicParams->resCTBCtrl,
        m_feiPicParams->resCTBCtrl.iSize,
        0,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value,
        0,
        false));

    param = &m_surfaceParams[SURFACE_FEI_CTB_DISTORTION];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        param,
        &m_feiPicParams->resDistortion,
        m_feiPicParams->resDistortion.iSize,
        0,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value,
        0,
        false));
#endif

    return eStatus;
}

MOS_STATUS CodechalFeiHevcStateG9Skl::SetSequenceStructs()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    m_feiPicParams = (CodecEncodeHevcFeiPicParams *)m_encodeParams.pFeiPicParams;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncHevcState::SetSequenceStructs());

    m_enable26WalkingPattern                 = m_feiPicParams->bForceLCUSplit;
    m_numRegionsInSlice                      = m_feiPicParams->NumConcurrentEncFramePartition;
    m_encodeParams.bReportStatisticsEnabled  = 0;
    m_encodeParams.bQualityImprovementEnable = 0;

    if (m_feiPicParams->FastIntraMode)
    {
        m_hevcSeqParams->TargetUsage = 0x07;
    }

    return eStatus;
}

CodechalFeiHevcStateG9Skl::CodechalFeiHevcStateG9Skl(CodechalHwInterface* hwInterface,
    CodechalDebugInterface* debugInterface,
    PCODECHAL_STANDARD_INFO standardInfo)
    :CodechalEncHevcStateG9(hwInterface, debugInterface, standardInfo)
{
    m_kernelBase = (uint8_t *)IGCODECKRN_G9;
    m_kuid = IDR_CODEC_HEVC_FEI_COMBINED_KENREL_INTEL;
    pfnGetKernelHeaderAndSize = GetKernelHeaderAndSize;
    m_noMeKernelForPFrame = false;
    m_feiEnable = true;

    MOS_STATUS eStatus = InitMhw();
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("HEVC FEI encoder MHW initialization failed.");
    }
}

