/*
* Copyright (c) 2017-2019, Intel Corporation
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
//! \file     codechal_encode_hevc_mbenc_g12.h
//! \brief    HEVC dual-pipe base class kernel interface for GEN12 platform.
//!

#ifndef __CODECHAL_ENCODE_HEVC_MBENC_G12_H__
#define __CODECHAL_ENCODE_HEVC_MBENC_G12_H__

#include "codechal_encode_hevc_g12.h"
#include "codechal_kernel_intra_dist_mdf_g12.h"

//! MBENC B kernel Curbe structure
typedef struct _MbencBcurbeDataG12
{
    // DWORD 0
    uint32_t   DW0_FrameWidthInSamples : MOS_BITFIELD_RANGE(0, 15);   // PicW should be a multiple of 8
    uint32_t   DW0_FrameHeightInSamples : MOS_BITFIELD_RANGE(16, 31);   // PicH should be a multiple of 8

    // DWORD 1
    uint32_t   DW1_Log2MaxCUSize : MOS_BITFIELD_RANGE(0, 3);
    uint32_t   DW1_Log2MinCUSize : MOS_BITFIELD_RANGE(4, 7);
    uint32_t   DW1_Log2MaxTUSize : MOS_BITFIELD_RANGE(8, 11);
    uint32_t   DW1_Log2MinTUSize : MOS_BITFIELD_RANGE(12, 15);
    uint32_t   DW1_MaxNumIMESearchCenter : MOS_BITFIELD_RANGE(16, 18);
    uint32_t   DW1_MaxIntraRdeIter : MOS_BITFIELD_RANGE(19, 21);
    uint32_t   DW1_IntraRDOQEnable : MOS_BITFIELD_BIT(22);
    uint32_t   DW1_QPType : MOS_BITFIELD_RANGE(23, 24);
    uint32_t   DW1_MaxTransformDepthInter : MOS_BITFIELD_RANGE(25, 26);
    uint32_t   DW1_MaxTransformDepthIntra : MOS_BITFIELD_RANGE(27, 28);
    uint32_t   DW1_Log2ParallelMergeLevel : MOS_BITFIELD_RANGE(29, 31);

    // DWORD 2
    uint32_t   DW2_CornerNeighborPixel0 : MOS_BITFIELD_RANGE(0, 7);
    uint32_t   DW2_IntraNeighborAvailFlags : MOS_BITFIELD_RANGE(8, 13);
    uint32_t   DW2_ChromaFormatType : MOS_BITFIELD_RANGE(14, 15);
    uint32_t   DW2_SubPelMode : MOS_BITFIELD_RANGE(16, 17);
    uint32_t   DW2_Reserved_0 : MOS_BITFIELD_RANGE(18, 19);
    uint32_t   DW2_InterSADMeasure : MOS_BITFIELD_RANGE(20, 21);
    uint32_t   DW2_IntraSADMeasureAdj : MOS_BITFIELD_RANGE(22, 23);
    uint32_t   DW2_IntraPredictionMask : MOS_BITFIELD_RANGE(24, 26);
    uint32_t   DW2_RefIDCostMode : MOS_BITFIELD_BIT(27);
    uint32_t   DW2_TUBasedCostSetting : MOS_BITFIELD_RANGE(28, 30);
    uint32_t   DW2_Reserved_1 : MOS_BITFIELD_BIT(31);

    // DWORD 3
    uint32_t   DW3_ExplicitModeEn : MOS_BITFIELD_BIT(0);
    uint32_t   DW3_AdaptiveEn : MOS_BITFIELD_BIT(1);
    uint32_t   DW3_Reserved_0 : MOS_BITFIELD_RANGE(2, 4);
    uint32_t   DW3_EarlyImeSuccessEn : MOS_BITFIELD_BIT(5);
    uint32_t   DW3_IntraSpeedMode : MOS_BITFIELD_BIT(6);
    uint32_t   DW3_IMECostCentersSel : MOS_BITFIELD_BIT(7);
    uint32_t   DW3_RDEQuantRoundValue : MOS_BITFIELD_RANGE(8, 15);
    uint32_t   DW3_IMERefWindowSize : MOS_BITFIELD_RANGE(16, 17);
    uint32_t   DW3_IntraComputeType : MOS_BITFIELD_BIT(18);
    uint32_t   DW3_Depth0IntraPrediction : MOS_BITFIELD_BIT(19);
    uint32_t   DW3_TUDepthControl : MOS_BITFIELD_RANGE(20, 21);
    uint32_t   DW3_IntraTuRecFeedbackDisable : MOS_BITFIELD_BIT(22);
    uint32_t   DW3_MergeListBiDisable : MOS_BITFIELD_BIT(23);
    uint32_t   DW3_EarlyImeStop : MOS_BITFIELD_RANGE(24, 31);

    // DWORD 4
    uint32_t   DW4_SliceQP : MOS_BITFIELD_RANGE(0, 6);
    uint32_t   DW4_SliceQPSign : MOS_BITFIELD_BIT(7);
    uint32_t   DW4_ConcurrentGroupNum : MOS_BITFIELD_RANGE(8, 15);
    uint32_t   DW4_ModeIntra32x32Cost : MOS_BITFIELD_RANGE(16, 23);
    uint32_t   DW4_ModeIntraNonDC32x32Cost : MOS_BITFIELD_RANGE(24, 31);

    // DWORD 5
    uint32_t   DW5_ModeIntra16x16Cost : MOS_BITFIELD_RANGE(0, 7);
    uint32_t   DW5_ModeIntraNonDC16x16Cost : MOS_BITFIELD_RANGE(8, 15);
    uint32_t   DW5_ModeIntra8x8Cost : MOS_BITFIELD_RANGE(16, 23);
    uint32_t   DW5_ModeIntraNonDC8x8Cost : MOS_BITFIELD_RANGE(24, 31);

    // DWORD 6
    uint32_t   DW6_ModeIntraNonPred : MOS_BITFIELD_RANGE(0, 7);
    uint32_t   DW6_ModeIntraCUCost : MOS_BITFIELD_RANGE(8, 15);
    uint32_t   DW6_ModeIntraNonDCCost : MOS_BITFIELD_RANGE(16, 23);
    uint32_t   DW6_Reserved_0 : MOS_BITFIELD_RANGE(24, 31);

    // DWORD 7
    uint32_t   DW7_Reserved_0 : MOS_BITFIELD_RANGE(0, 23);
    uint32_t   DW7_ChromaIntraModeCost : MOS_BITFIELD_RANGE(24, 31);

    // DWORD 8
    uint32_t   DW8_IntraLumaModeMasks0_31 : MOS_BITFIELD_RANGE(0, 31);

    // DWORD 9
    uint32_t   DW9_IntraLumaModeMasks32_34 : MOS_BITFIELD_RANGE(0, 2);
    uint32_t   DW9_IntraChromaModeMask : MOS_BITFIELD_RANGE(3, 7);
    uint32_t   DW9_PenaltyForIntraNonDCPredMode : MOS_BITFIELD_RANGE(8, 15);
    uint32_t   DW9_NeighborPixelChromaValueCbCrPair : MOS_BITFIELD_RANGE(16, 31);

    // DWORD 10
    uint32_t   DW10_IntraPredModeLeftNbrBlk0 : MOS_BITFIELD_RANGE(0, 7);
    uint32_t   DW10_IntraPredModeLeftNbrBlk1 : MOS_BITFIELD_RANGE(8, 15);
    uint32_t   DW10_IntraPredModeLeftNbrBlk2 : MOS_BITFIELD_RANGE(16, 23);
    uint32_t   DW10_IntraPredModeLeftNbrBlk3 : MOS_BITFIELD_RANGE(24, 31);

    // DWORD 11
    uint32_t   DW11_IntraPredModeTopNbrBlk0 : MOS_BITFIELD_RANGE(0, 7);
    uint32_t   DW11_IntraPredModeTopNbrBlk1 : MOS_BITFIELD_RANGE(8, 15);
    uint32_t   DW11_IntraPredModeTopNbrBlk2 : MOS_BITFIELD_RANGE(16, 23);
    uint32_t   DW11_IntraPredModeTopNbrBlk3 : MOS_BITFIELD_RANGE(24, 31);

    // DWORD 12
    uint32_t   DW12_IntraModeCostMPM : MOS_BITFIELD_RANGE(0, 7);
    uint32_t   DW12_IntraPUModeCost : MOS_BITFIELD_RANGE(8, 15);
    uint32_t   DW12_IntraPUNxNCost : MOS_BITFIELD_RANGE(16, 23);
    uint32_t   DW12_Reserved_0 : MOS_BITFIELD_RANGE(24, 31);

    // DWORD 13
    uint32_t   DW13_IntraTUDept0Cost : MOS_BITFIELD_RANGE(0, 7);
    uint32_t   DW13_IntraTUDept1Cost : MOS_BITFIELD_RANGE(8, 15);
    uint32_t   DW13_IntraTUDept2Cost : MOS_BITFIELD_RANGE(16, 23);
    uint32_t   DW13_Reserved_0 : MOS_BITFIELD_RANGE(24, 31);

    // DWORD 14
    uint32_t   DW14_IntraTU4x4CBFCost : MOS_BITFIELD_RANGE(0, 7);
    uint32_t   DW14_IntraTU8x8CBFCost : MOS_BITFIELD_RANGE(8, 15);
    uint32_t   DW14_IntraTU16x16CBFCost : MOS_BITFIELD_RANGE(16, 23);
    uint32_t   DW14_IntraTU32x32CBFCost : MOS_BITFIELD_RANGE(24, 31);

    // DWORD 15
    uint32_t   DW15_LambdaRD : MOS_BITFIELD_RANGE(0, 15);
    uint32_t   DW15_Reserved_0 : MOS_BITFIELD_RANGE(16, 31);

    // DWORD 16
    uint32_t   DW16_PictureQp_B : MOS_BITFIELD_RANGE(0, 7);
    uint32_t   DW16_PictureQp_P : MOS_BITFIELD_RANGE(8, 15);
    uint32_t   DW16_PictureQp_I : MOS_BITFIELD_RANGE(16, 23);
    uint32_t   DW16_Reserved_0 : MOS_BITFIELD_RANGE(24, 31);

    // DWORD 17
    uint32_t   DW17_IntraNonDC8x8Penalty : MOS_BITFIELD_RANGE(0, 7);
    uint32_t   DW17_IntraNonDC32x32Penalty : MOS_BITFIELD_RANGE(8, 15);
    uint32_t   DW17_NumRowOfTile : MOS_BITFIELD_RANGE(16, 23);
    uint32_t   DW17_NumColOfTile : MOS_BITFIELD_RANGE(24, 31);

    //DWORD 18
    uint32_t   DW18_TransquantBypassEnableFlag : MOS_BITFIELD_BIT(0);
    uint32_t   DW18_PCMEnabledFlag : MOS_BITFIELD_BIT(1);
    uint32_t   DW18_Reserved_0 : MOS_BITFIELD_RANGE(2, 3);
    uint32_t   DW18_CuQpDeltaEnabledFlag : MOS_BITFIELD_BIT(4);
    uint32_t   DW18_SteppingSelection : MOS_BITFIELD_RANGE(5, 6);
    uint32_t   DW18_RegionsInSliceEnable : MOS_BITFIELD_BIT(7);
    uint32_t   DW18_HMEFlag : MOS_BITFIELD_RANGE(8, 9);
    uint32_t   DW18_Reserved_1 : MOS_BITFIELD_RANGE(10, 15);
    uint32_t   DW18_Cu64SkipCheckOnly : MOS_BITFIELD_BIT(16);
    uint32_t   DW18_EnableCu64Check : MOS_BITFIELD_BIT(17);
    uint32_t   DW18_Cu642Nx2NCheckOnly : MOS_BITFIELD_BIT(18);
    uint32_t   DW18_EnableCu64AmpCheck : MOS_BITFIELD_BIT(19);
    uint32_t   DW18_Reserved_2 : MOS_BITFIELD_BIT(20);
    uint32_t   DW18_DisablePIntra : MOS_BITFIELD_BIT(21);
    uint32_t   DW18_DisableIntraTURec : MOS_BITFIELD_BIT(22);
    uint32_t   DW18_InheritIntraModeFromTU0 : MOS_BITFIELD_BIT(23);
    uint32_t   DW18_Reserved_3 : MOS_BITFIELD_RANGE(24, 26);
    uint32_t   DW18_CostScalingForRA : MOS_BITFIELD_BIT(27);
    uint32_t   DW18_DisableIntraNxN : MOS_BITFIELD_BIT(28);
    uint32_t   DW18_Reserved_4 : MOS_BITFIELD_RANGE(29, 31);

    //DWORD 19
    uint32_t   DW19_MaxRefIdxL0 : MOS_BITFIELD_RANGE(0, 7);
    uint32_t   DW19_MaxRefIdxL1 : MOS_BITFIELD_RANGE(8, 15);
    uint32_t   DW19_MaxBRefIdxL0 : MOS_BITFIELD_RANGE(16, 23);
    uint32_t   DW19_Reserved_0 : MOS_BITFIELD_RANGE(24, 31);

    //DWORD 20
    uint32_t   DW20_EarlyTermination : MOS_BITFIELD_BIT(0);
    uint32_t   DW20_Skip : MOS_BITFIELD_BIT(1);
    uint32_t   DW20_SkipEarlyTermSize : MOS_BITFIELD_RANGE(2, 3);
    uint32_t   DW20_Dynamic64Enable : MOS_BITFIELD_RANGE(4, 5);
    uint32_t   DW20_Dynamic64Order : MOS_BITFIELD_RANGE(6, 7);
    uint32_t   DW20_Dynamic64Th : MOS_BITFIELD_RANGE(8, 11);
    uint32_t   DW20_DynamicOrderTh : MOS_BITFIELD_RANGE(12, 15);
    uint32_t   DW20_BFrameQPOffset : MOS_BITFIELD_RANGE(16, 23);
    uint32_t   DW20_IncreaseExitThresh : MOS_BITFIELD_RANGE(24, 27);
    uint32_t   DW20_Dynamic64Min32 : MOS_BITFIELD_RANGE(28, 29);
    uint32_t   DW20_Reserved_0 : MOS_BITFIELD_BIT(30);
    uint32_t   DW20_LastFrameIsIntra : MOS_BITFIELD_BIT(31);

    //DWORD 21
    uint32_t   DW21_LenSP : MOS_BITFIELD_RANGE(0, 7);
    uint32_t   DW21_MaxNumSU : MOS_BITFIELD_RANGE(8, 15);
    uint32_t   DW21_Reserved_0 : MOS_BITFIELD_RANGE(16, 31);

    //DWORD 22
    uint32_t   DW22_CostTableIndex : MOS_BITFIELD_RANGE(0, 7);
    uint32_t   DW22_NumRegions : MOS_BITFIELD_RANGE(8, 15);
    uint32_t   DW22_Reserved_0 : MOS_BITFIELD_RANGE(16, 31);

    //DWORD 23
    uint32_t   DW23_SliceType : MOS_BITFIELD_RANGE(0, 1);
    uint32_t   DW23_TemporalMvpEnableFlag : MOS_BITFIELD_BIT(2);
    uint32_t   DW23_CollocatedFromL0Flag : MOS_BITFIELD_BIT(3);
    uint32_t   DW23_TheSameRefList : MOS_BITFIELD_BIT(4);
    uint32_t   DW23_IsLowDelay : MOS_BITFIELD_BIT(5);
    uint32_t   DW23_Reserved_0 : MOS_BITFIELD_RANGE(6, 7);
    uint32_t   DW23_MaxNumMergeCand : MOS_BITFIELD_RANGE(8, 15);
    uint32_t   DW23_NumRefIdxL0 : MOS_BITFIELD_RANGE(16, 23);
    uint32_t   DW23_NumRefIdxL1 : MOS_BITFIELD_RANGE(24, 31);

    //DWORD 24
    uint32_t   DW24_FwdPocNumber_L0_mTb_0 : MOS_BITFIELD_RANGE(0, 7);
    uint32_t   DW24_BwdPocNumber_L1_mTb_0 : MOS_BITFIELD_RANGE(8, 15);
    uint32_t   DW24_FwdPocNumber_L0_mTb_1 : MOS_BITFIELD_RANGE(16, 23);
    uint32_t   DW24_BwdPocNumber_L1_mTb_1 : MOS_BITFIELD_RANGE(24, 31);

    //DWORD 25
    uint32_t   DW25_FwdPocNumber_L0_mTb_2 : MOS_BITFIELD_RANGE(0, 7);
    uint32_t   DW25_BwdPocNumber_L1_mTb_2 : MOS_BITFIELD_RANGE(8, 15);
    uint32_t   DW25_FwdPocNumber_L0_mTb_3 : MOS_BITFIELD_RANGE(16, 23);
    uint32_t   DW25_BwdPocNumber_L1_mTb_3 : MOS_BITFIELD_RANGE(24, 31);

    //DWORD 26
    uint32_t   DW26_FwdPocNumber_L0_mTb_4 : MOS_BITFIELD_RANGE(0, 7);
    uint32_t   DW26_BwdPocNumber_L1_mTb_4 : MOS_BITFIELD_RANGE(8, 15);
    uint32_t   DW26_FwdPocNumber_L0_mTb_5 : MOS_BITFIELD_RANGE(16, 23);
    uint32_t   DW26_BwdPocNumber_L1_mTb_5 : MOS_BITFIELD_RANGE(24, 31);

    //DWORD 27
    uint32_t   DW27_FwdPocNumber_L0_mTb_6 : MOS_BITFIELD_RANGE(0, 7);
    uint32_t   DW27_BwdPocNumber_L1_mTb_6 : MOS_BITFIELD_RANGE(8, 15);
    uint32_t   DW27_FwdPocNumber_L0_mTb_7 : MOS_BITFIELD_RANGE(16, 23);
    uint32_t   DW27_BwdPocNumber_L1_mTb_7 : MOS_BITFIELD_RANGE(24, 31);

    //DWORD 28
    uint32_t   DW28_LongTermReferenceFlags_L0 : MOS_BITFIELD_RANGE(0, 15);
    uint32_t   DW28_LongTermReferenceFlags_L1 : MOS_BITFIELD_RANGE(16, 31);

    //DWORD 29
    uint32_t   DW29_RefFrameVerticalSize : MOS_BITFIELD_RANGE(0, 15);
    uint32_t   DW29_RefFrameHorizontalSize : MOS_BITFIELD_RANGE(16, 31);

    //DWORD 30
    uint32_t   DW30_RoundingInter : MOS_BITFIELD_RANGE(0, 7);
    uint32_t   DW30_RoundingIntra : MOS_BITFIELD_RANGE(8, 15);
    uint32_t   DW30_Reserved_0 : MOS_BITFIELD_RANGE(16, 31);

    // DWORD 31
    uint32_t   DW31_Reserved_0 : MOS_BITFIELD_RANGE(0, 31);

    // DWORD 32
    uint32_t   DW32_Reserved_0 : MOS_BITFIELD_RANGE(0, 31);

    // DWORD 33
    uint32_t   DW33_Reserved_0 : MOS_BITFIELD_RANGE(0, 31);

    // DWORD 34
    uint32_t   DW34_Reserved_0 : MOS_BITFIELD_RANGE(0, 31);

    // DWORD 35
    uint32_t   DW35_Reserved_0 : MOS_BITFIELD_RANGE(0, 31);

    // DWORD 36
    uint32_t   DW36_Reserved_0 : MOS_BITFIELD_RANGE(0, 31);

    // DWORD 37
    uint32_t   DW37_Reserved_0 : MOS_BITFIELD_RANGE(0, 31);

    // DWORD 38
    uint32_t   DW38_Reserved_0 : MOS_BITFIELD_RANGE(0, 31);

    // DWORD 39
    uint32_t   DW39_Reserved_0 : MOS_BITFIELD_RANGE(0, 31);

} MbencBcurbeDataG12, *PMbencBcurbeDataG12;

C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(MbencBcurbeDataG12)) == 40);

const MbencBcurbeDataG12 initMbenBcurbeDataGen12 = {
    0,       //DW0_FrameWidthInSamples
    0,       //DW0_FrameHeightInSamples
    5,       //DW1_Log2MaxCUSize
    3,       //DW1_Log2MinCUSize
    5,       //DW1_Log2MaxTUSize
    2,       //DW1_Log2MinTUSize
    3,       //DW1_MaxNumIMESearchCenter
    1,       //DW1_MaxIntraRdeIter
    0,       //DW1_IntraRDOQEnable
    0,       //DW1_QPType
    0,       //DW1_MaxTransformDepthInter
    0,       //DW1_MaxTransformDepthIntra
    2,       //DW1_Log2ParallelMergeLevel
    0,       //DW2_CornerNeighborPixel0
    0,       //DW2_IntraNeighborAvailFlags
    1,       //DW2_ChromaFormatType
    0x3,     //DW2_SubPelMode
    0,       //DW2_Reserved_0
    2,       //DW2_InterSADMeasure
    2,       //DW2_IntraSADMeasureAdj
    0,       //DW2_IntraPredictionMask
    1,       //DW2_RefIDCostMode
    0,       //DW2_TUBasedCostSetting
    0,       //DW2_Reserved_1
    0,       //DW3_ExplicitModeEn
    1,       //DW3_AdaptiveEn
    0,       //DW3_Reserved_0
    0,       //DW3_EarlyImeSuccessEn
    0,       //DW3_IntraSpeedMode
    0,       //DW3_IMECostCentersSel
    176,     //DW3_RDEQuantRoundValue
    1,       //DW3_IMERefWindowSize
    0,       //DW3_IntraComputeType
    0,       //DW3_Depth0IntraPrediction
    0,       //DW3_TUDepthControl
    0,       //DW3_IntraTuRecFeedbackDisable
    0,       //DW3_MergeListBiDisable
    0,       //DW3_EarlyImeStop
    27,      //DW4_SliceQP
    0,       //DW4_SliceQPSign
    1,       //DW4_ConcurrentGroupNum
    40,      //DW4_ModeIntra32x32Cost
    0,       //DW4_ModeIntraNonDC32x32Cost
    40,      //DW5_ModeIntra16x16Cost
    0,       //DW5_ModeIntraNonDC16x16Cost
    41,      //DW5_ModeIntra8x8Cost
    0,       //DW5_ModeIntraNonDC8x8Cost
    15,      //DW6_ModeIntraNonPred
    0,       //DW6_ModeIntraCUCost
    0,       //DW6_ModeIntraNonDCCost
    0,       //DW6_Reserved_0
    0,       //DW7_Reserved_0
    40,      //DW7_ChromaIntraModeCost
    0,       //DW8_IntraLumaModeMasks0_31 - kernel will update this value
    0,       //DW9_IntraLumaModeMasks32_34 - kernel will update this value
    0,       //DW9_IntraChromaModeMask - kernel will update this value
    0,       //DW9_PenaltyForIntraNonDCPredMode - kernel will update this value
    0,       //DW9_NeighborPixelChromaValueCbCrPair - kernel will update this value
    0,       //DW10_IntraPredModeLeftNbrBlk0 - kernel will update this value
    0,       //DW10_IntraPredModeLeftNbrBlk1 - kernel will update this value
    0,       //DW10_IntraPredModeLeftNbrBlk2 - kernel will update this value
    0,       //DW10_IntraPredModeLeftNbrBlk3 - kernel will update this value
    0,       //DW11_IntraPredModeTopNbrBlk0 - kernel will update this value
    0,       //DW11_IntraPredModeTopNbrBlk1 - kernel will update this value
    0,       //DW11_IntraPredModeTopNbrBlk2 - kernel will update this value
    0,       //DW11_IntraPredModeTopNbrBlk3 - kernel will update this value
    0,       //DW12_IntraModeCostMPM
    0,       //DW12_IntraPUModeCost - kernel will update this value
    0,       //DW12_IntraPUNxNCost - kernel will update this value
    0,       //DW12_Reserved_0
    4,       //DW13_IntraTUDept0Cost
    20,      //DW13_IntraTUDept1Cost
    0,       //DW13_IntraTUDept2Cost
    0,       //DW13_Reserved_0
    12,      //DW14_IntraTU4x4CBFCost
    12,      //DW14_IntraTU8x8CBFCost
    12,      //DW14_IntraTU16x16CBFCost
    12,      //DW14_IntraTU32x32CBFCost
    77,      //DW15_LambdaRD
    0,       //DW15_Reserved_0
    0,       //DW16_PictureQp_B
    0,       //DW16_PictureQp_P
    0,       //DW16_PictureQp_I
    0,       //DW16_Reserved_0
    0,       //DW17_IntraNonDC8x8Penalty
    0,       //DW17_IntraNonDC32x32Penalty
    1,       //DW17_NumRowOfTile
    1,       //DW17_NumColOfTile
    0,       //DW18_TransquantBypassEnableFlag
    0,       //DW18_PCMEnabledFlag
    0,       //DW18_Reserved_0
    0,       //DW18_CuQpDeltaEnabledFlag
    1,       //DW18_SteppingSelection
    0,       //DW18_RegionsInSliceEnable
    0,       //DW18_HMEFlag
    0,       //DW18_Reserved_1
    0,       //DW18_Cu64SkipCheckOnly
    0,       //DW18_EnableCu64Check
    1,       //DW18_Cu642Nx2NCheckOnly
    0,       //DW18_EnableCu64AmpCheck
    0,       //DW18_Reserved_2
    0,       //DW18_DisablePIntra
    0,       //DW18_DisableIntraTURec
    0,       //DW18_InheritIntraModeFromTU0
    0,       //DW18_Reserved_3
    0,       //DW18_CostScalingForRA
    0,       //DW18_DisableIntraNxN
    0,       //DW18_Reserved_4
    0,       //DW19_MaxRefIdxL0
    0,       //DW19_MaxRefIdxL1
    0,       //DW19_MaxBRefIdxL0
    0,       //DW19_Reserved_0
    0,       //DW20_EarlyTermination
    0,       //DW20_Size
    0,       //DW20_SkipEarlyTermSize
    0,       //DW20_Dynamic64Enable
    0,       //DW20_Dynamic64Order
    0,       //DW20_Dynamic64Th
    0,       //DW20_DynamicOrderTh
    0,       //DW20_BFrameQpOffset
    0,       //DW20_IncreaseExitThresh
    3,       //DW20_Dynamic64Min32
    0,       //DW20_Reserved_0
    0,       //DW20_LastFrameIntra
    63,      //DW21_LenSP
    63,      //DW21_MaxNumSU
    0,       //DW21_Reserved_0
    0,       //DW22_CostTableIndex
    1,       //DW22_NumRegions
    0,       //DW22_Reserved_0
    0,       //DW23_SliceType
    0,       //DW23_TemporalMvpEnableFlag
    0,       //DW23_CollocatedFromL0Flag
    0,       //DW23_TheSameRefList
    0,       //DW23_IsLowDelay
    0,       //DW23_Reserved_0
    4,       //DW23_MaxNumMergeCand
    0,       //DW23_NumRefIdxL0
    0,       //DW23_NumRefIdxL1
    0,       //DW24_FwdPocNumber_L0_mTb_0
    0,       //DW24_BwdPocNumber_L1_mTb_0
    0,       //DW24_FwdPocNumber_L0_mTb_1
    0,       //DW24_BwdPocNumber_L1_mTb_1
    0,       //DW25_FwdPocNumber_L0_mTb_2
    0,       //DW25_BwdPocNumber_L1_mTb_2
    0,       //DW25_FwdPocNumber_L0_mTb_3
    0,       //DW25_BwdPocNumber_L1_mTb_3
    0,       //DW26_FwdPocNumber_L0_mTb_4
    0,       //DW26_BwdPocNumber_L1_mTb_4
    0,       //DW26_FwdPocNumber_L0_mTb_5
    0,       //DW26_BwdPocNumber_L1_mTb_5
    0,       //DW27_FwdPocNumber_L0_mTb_6
    0,       //DW27_BwdPocNumber_L1_mTb_6
    0,       //DW27_FwdPocNumber_L0_mTb_7
    0,       //DW27_BwdPocNumber_L1_mTb_7
    0,       //DW28_LongTermReferenceFlags_L0
    0,       //DW28_LongTermReferenceFlags_L1
    0,       //DW29_RefFrameVerticalSize
    0,       //DW29_RefFrameHorizontalSize
    0,       //DW30_RoundingInter
    0,       //DW30_RoundingIntra
    0,       //DW30_Reserved_0
    0,       //DW31_Reserved_0
    0,       //DW32_Reserved_0
    0,       //DW33_Reserved_0
    0,       //DW34_Reserved_0
    0,       //DW35_Reserved_0
    0,       //DW36_Reserved_0
    0,       //DW37_Reserved_0
    0,       //DW38_Reserved_0
    0,       //DW39_Reserved_0
};

namespace CMRT_UMD
{
    class CmDevice;
    class CmTask;
    class CmQueue;
    class CmThreadSpace;
    class CmKernel;
    class CmProgram;
    class SurfaceIndex;
    class CmSurface2D;
    class CmBuffer;
}

#define MAX_VME_BWD_REF           4
#define MAX_VME_FWD_REF           4

//!  HEVC dual-pipe encoder class for GEN12 HEVC MBenc kernel with MDF support
class CodecHalHevcMbencG12 : public CodechalEncHevcStateG12
{

protected:
    CmThreadSpace *    m_threadSpace  = nullptr;
    CmKernel *         m_cmKrnB       = nullptr;
    CmProgram *        m_cmProgramB   = nullptr;
    CmKernel *         m_cmKrnB64     = nullptr;
    CmProgram *        m_cmProgramB64 = nullptr; 
    static CmDevice *  m_mfeCmDev;
    MbencBcurbeDataG12 m_curbeDataB = {};

    typedef SurfaceIndex MBencSurfaceIndex[m_maxMfeSurfaces][m_maxMultiFrames];
    MBencSurfaceIndex * m_surfIndexArray = nullptr;

    uint8_t m_FrameBalance[m_loadBalanceSize];
    int32_t m_totalFrameAdj[m_maxMultiFrames] = {};

    static const uint8_t m_frameColorMapEntrySize = 16;
    static const uint8_t m_frameColorMapLocCurFrame = 0;
    static const uint8_t m_frameColorMapLocCurColor = 1;
    static const uint8_t m_frameColorMapLocTotalFrame = 2;
    static const uint8_t m_frameColorMapLocTotalColor = 3;
    static const uint8_t m_frameColorMapFrameInvalid = 0xFF;

    //Internal surfaces
    CmBuffer               *m_combinedBuffer1 = nullptr;                     //!< Combined buffer 1
    CmBuffer               *m_combinedBuffer2 = nullptr;                     //!< Combined buffer 2
    CmSurface2D *           m_reconWithBoundaryPix      = nullptr;                     //!< Recon surface with populated boundary pixels.
    CmSurface2D *           m_intermediateCuRecordLcu32 = nullptr;                     //!< Intermediate CU Record Surface for I and B kernel
    CmSurface2D *           m_scratchSurf               = nullptr;                     //!< Scartch Surface for I-kernel
    CmSurface2D *           m_cu16X16QpIn               = nullptr;                     //!< CU 16x16 QP data input surface
    CmSurface2D *           m_lcuLevelData              = nullptr;                     //!< Mv and Distortion summation surface
    CmBuffer *              m_constTableB               = nullptr;                     //!< Enc constant table for B LCU32
    CmBuffer *              m_cuSplitSurf               = nullptr;                     //!< CU split data surface
    CmBuffer *              m_loadBalance               = nullptr;                     //!< Enc constant table for B LCU32
    CmBuffer *              m_dbgSurface                = nullptr;                     //!< Debug surface
    
    CodechalKernelIntraDistMdfG12 *m_intraDistKernel = nullptr;

    //External surfaces
    SurfaceIndex *m_curVme            = nullptr;                      //!< Resource owned by DDI
    CmSurface2D * m_curSurf           = nullptr;                      //!< Resource owned by DDI
    CmBuffer *    m_mbCodeBuffer      = nullptr;                      //!< Resource owned by PAK
    SurfaceIndex *m_mbCodeSurfIdx     = nullptr;                      //!< Resource owned by PAK
    SurfaceIndex *m_mvDataSurfIdx     = nullptr;                      //!< Resource owned by PAK
    CmSurface2D * m_swScoreboardSurf  = nullptr;                      //!< Resource owned by SW Scoreboard Krn
    CmBuffer *    m_colocCumvData     = nullptr;                      //!< Resource owned by HME
    CmSurface2D * m_hmeMotionPredData = nullptr;                      //!< Resource owned by HME
    CmSurface2D * m_curSurf2X         = nullptr;                      //!< Resource owned by HME
    SurfaceIndex *m_cur2XVme          = nullptr;                      //!< Resource owned by HME
    CmSurface2D * m_histInBuffer      = nullptr;                      //!< Resource owned by BRC
    CmSurface2D * m_histOutBuffer     = nullptr;                      //!< Resource owned by BRC
    CmSurface2D * m_surfRefArray[MAX_VME_FWD_REF + MAX_VME_BWD_REF];  //!< Resource owned by DDI
    CmSurface2D * m_surf2XArray[MAX_VME_FWD_REF + MAX_VME_BWD_REF];   //!< Resource owned by DDI

public:
    BRC_INITRESET_CURBE curbe = {};
    BRCUPDATE_CURBE     curbeBrcUpdate = {};

    //!
    //! \brief    Constructor
    //!
    CodecHalHevcMbencG12(CodechalHwInterface* hwInterface,
        CodechalDebugInterface* debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo);

    //!
    //! \brief    Destructor
    //!
    virtual ~CodecHalHevcMbencG12();

    //!
    //! \brief    Allocate CM Device , CM Task and Cm Queue
    //! \brief    Will be used by all the kernels
    //!
    MOS_STATUS AllocateMDFResources() override;

    //!
    //! \brief    Function to destroy MDF required resources
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DestroyMDFResources() override;

    //!
    //! \brief    Allocate ME resources
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateMeResources();


   

protected:
    //!
    //! \brief    Allocate resources used by MBEnc
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AllocateEncResources()override;

    MOS_STATUS AllocateBrcResources()override;


    //!
    //! \brief    Free resources
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS FreeEncResources()override;

    virtual MOS_STATUS FreeBrcResources()override;
    
    //!
    //! \brief    Free resources
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS FreeMeResources();
    
    //!
    //! \brief    Top level function for invoking MBenc kernel
    //! \details  I, B or LCU64_B MBEnc kernel, based on EncFunctionType
    //! \param    [in]  encFunctionType
    //!           Specifies the media state type
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS EncodeMbEncKernel(CODECHAL_MEDIA_STATE_TYPE encFunctionType) override;

    //!
    //! \brief    Loads the ISA kernel
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitKernelState()override;

    uint32_t GetMaxBtCount()override;

    //!
    //! \brief    Setup Kernel Arguments for B frame
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetupKernelArgsB();

    //!
    //! \brief    Init Curbe Data for B frame
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS InitCurbeDataB();

    //!
    //! \brief    Function to setup external resources for B frames.
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetupSurfacesB();

    ////!
    ////! \brief    Top level function for IntraDist kernel
    ////!
    ////! \return   MOS_STATUS
    ////!           MOS_STATUS_SUCCESS if success, else fail reason
    ////! 
    MOS_STATUS EncodeIntraDistKernel()override;

    //!
    //! \brief    Top level function to set remap table
    //! \details  Used to setup load balance surface.
    //! \return   void
    //!
    void SetColorBitRemap(uint8_t * remapTable, int32_t multiFrameNumber, int32_t curColor, int32_t * totalColor, int32_t * totalFrameAdj);

    //!
    //! \brief    Get rounding inter/intra for current frame to use
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS GetRoundingIntraInterToUse() override;

};
//! \brief  typedef of class CodecHalHevcMbencG12*
using pCodecHalHevcMbencG12 = class CodecHalHevcMbencG12*;

#endif  // __CODECHAL_ENCODE_HEVC_MBENC_G12_H__
