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
//! \file     codechal_encode_hevc_g10.cpp
//! \brief    HEVC dual-pipe encoder for GEN10.
//!

#include "codechal_encode_hevc_g10.h"
#ifndef _FULL_OPEN_SOURCE
#include "igcodeckrn_g10.h"
#endif
#include "codeckrnheader.h"

//! HME mode
enum
{
    HME_STAGE_4x_NO_16x = 0,
    HME_STAGE_4x_AFTER_16x,
    HME_STAGE_16x
};

//! MBENC kernel index
enum CODECHAL_ENC_HEVC_MBENC_KRNIDX_G10
{
    MBENC_I_KRNIDX = 0,
    MBENC_B_LCU32_KRNIDX,
    MBENC_B_LCU64_KRNIDX,
    MBENC_NUM_KRN
};

//! Index for the m_modeCost LUT
enum {
    LUTMODE_INTRA_NONPRED = 0,      //!< extra penalty for non-predicted modes
    LUTMODE_INTRA_32x32,
    LUTMODE_INTRA_16x16,
    LUTMODE_INTRA_8x8,
    LUTMODE_INTER_32x16,
    LUTMODE_INTER_16x32 = 4,
    LUTMODE_INTER_AMP = 4,          //!< All asymmetrical shapes
    LUTMODE_INTER_16x16,
    LUTMODE_INTER_16x8,
    LUTMODE_INTER_8x16 = 6,
    LUTMODE_INTER_8x8,
    LUTMODE_INTER_32x32,
    LUTMODE_INTER_BIDIR,
    LUTMODE_REF_ID,
    LUTMODE_INTRA_CHROMA
};

//! Binding table offset for all kernels
enum
{
    // DownScaling And Conversion Kernel
    SCALING_CONVERSION_BEGIN = 0,
    SCALING_CONVERSION_10BIT_Y = SCALING_CONVERSION_BEGIN,
    SCALING_CONVERSION_10BIT_UV,
    SCALING_CONVERSION_8BIT_Y,
    SCALING_CONVERSION_8BIT_UV,
    SCALING_CONVERSION_4xDS,
    SCALING_CONVERSION_MB_STATS,
    SCALING_CONVERSION_2xDS,
    SCALING_CONVERSION_MB_SPLIT_SURFACE,
    SCALING_CONVERSION_LCU32_JOB_QUEUE_SCRATCH_SURFACE,
    SCALING_CONVERSION_LCU64_JOB_QUEUE_SCRATCH_SURFACE,
    SCALING_CONVERSION_LCU64_64x64_DISTORTION_SURFACE,
    SCALING_CONVERSION_END,

    // Hme Kernel
    HME_BEGIN = 0,
    HME_OUTPUT_MV_DATA = HME_BEGIN,
    HME_16xINPUT_MV_DATA,
    HME_4xOUTPUT_DISTORTION,
    HME_VME_PRED_CURR_PIC_IDX0,
    HME_VME_PRED_FWD_PIC_IDX0,
    HME_VME_PRED_BWD_PIC_IDX0,
    HME_VME_PRED_FWD_PIC_IDX1,
    HME_VME_PRED_BWD_PIC_IDX1,
    HME_VME_PRED_FWD_PIC_IDX2,
    HME_VME_PRED_BWD_PIC_IDX2,
    HME_VME_PRED_FWD_PIC_IDX3,
    HME_VME_PRED_BWD_PIC_IDX3,
    HME_4xDS_INPUT,
    HME_BRC_DISTORTION,
    HME_MV_AND_DISTORTION_SUM,
    HME_END,

    //BRC Init/Reset
    BRC_INIT_RESET_BEGIN = 0,
    BRC_INIT_RESET_HISTORY = BRC_INIT_RESET_BEGIN,
    BRC_INIT_RESET_DISTORTION,
    BRC_INIT_RESET_END,

    //BRC Update (frame based)
    BRC_UPDATE_BEGIN = 0,
    BRC_UPDATE_HISTORY = BRC_UPDATE_BEGIN,
    BRC_UPDATE_PREV_PAK,
    BRC_UPDATE_PIC_STATE_R,
    BRC_UPDATE_PIC_STATE_W,
    BRC_UPDATE_ENC_OUTPUT,
    BRC_UPDATE_DISTORTION,
    BRC_UPDATE_BRCDATA,
    BRC_UPDATE_MB_STATS,
    BRC_UPDATE_MV_AND_DISTORTION_SUM,
    BRC_UPDATE_END,

    //BRC Update (LCU-based)
    BRC_LCU_UPDATE_BEGIN = 0,
    BRC_LCU_UPDATE_HISTORY = BRC_LCU_UPDATE_BEGIN,
    BRC_LCU_UPDATE_DISTORTION,
    BRC_LCU_UPDATE_MB_STATS,
    BRC_LCU_UPDATE_MB_QP,
    BRC_LCU_UPDATE_MB_SPLIT_SURFACE,
    BRC_LCU_UPDATE_INTRA_DISTORTION,
    BRC_LCU_UPDATE_CU_SPLIT_SURFACE,
    BRC_LCU_UPDATE_END,

    // MBEnc I-kernel
    MBENC_I_FRAME_BEGIN = 0,
    MBENC_I_FRAME_VME_PRED_CURR_PIC_IDX0 = MBENC_I_FRAME_BEGIN,
    MBENC_I_FRAME_VME_PRED_FWD_PIC_IDX0,
    MBENC_I_FRAME_VME_PRED_BWD_PIC_IDX0,
    MBENC_I_FRAME_VME_PRED_FWD_PIC_IDX1,
    MBENC_I_FRAME_VME_PRED_BWD_PIC_IDX1,
    MBENC_I_FRAME_VME_PRED_FWD_PIC_IDX2,
    MBENC_I_FRAME_VME_PRED_BWD_PIC_IDX2,
    MBENC_I_FRAME_VME_PRED_FWD_PIC_IDX3,
    MBENC_I_FRAME_VME_PRED_BWD_PIC_IDX3,
    MBENC_I_FRAME_CURR_Y,
    MBENC_I_FRAME_CURR_UV,
    MBENC_I_FRAME_INTERMEDIATE_CU_RECORD,
    MBENC_I_FRAME_PAK_OBJ,
    MBENC_I_FRAME_PAK_CU_RECORD,
    MBENC_I_FRAME_SCRATCH_SURFACE,
    MBENC_I_FRAME_CU_QP_DATA,
    MBENC_I_FRAME_CONST_DATA_LUT,
    MBENC_I_FRAME_LCU_LEVEL_DATA_INPUT,
    MBENC_I_FRAME_CONCURRENT_TG_DATA,
    MBENC_I_FRAME_BRC_COMBINED_ENC_PARAMETER_SURFACE,
    MBENC_I_FRAME_CU_SPLIT_SURFACE,
    MBENC_I_FRAME_DEBUG_DUMP,
    MBENC_I_FRAME_END,

    // MBEnc B-kernel -- Both for LCU32
    MBENC_B_FRAME_LCU32_BEGIN = 0,
    MBENC_B_FRAME_LCU32_CURR_Y = MBENC_B_FRAME_LCU32_BEGIN,
    MBENC_B_FRAME_LCU32_CURR_UV,
    MBENC_B_FRAME_LCU32_ENC_CU_RECORD,
    MBENC_B_FRAME_LCU32_PAK_OBJ,
    MBENC_B_FRAME_LCU32_PAK_CU_RECORD,
    MBENC_B_FRAME_LCU32_VME_PRED_CURR_PIC_IDX0,
    MBENC_B_FRAME_LCU32_VME_PRED_FWD_PIC_IDX0,
    MBENC_B_FRAME_LCU32_VME_PRED_BWD_PIC_IDX0,
    MBENC_B_FRAME_LCU32_VME_PRED_FWD_PIC_IDX1,
    MBENC_B_FRAME_LCU32_VME_PRED_BWD_PIC_IDX1,
    MBENC_B_FRAME_LCU32_VME_PRED_FWD_PIC_IDX2,
    MBENC_B_FRAME_LCU32_VME_PRED_BWD_PIC_IDX2,
    MBENC_B_FRAME_LCU32_VME_PRED_FWD_PIC_IDX3,
    MBENC_B_FRAME_LCU32_VME_PRED_BWD_PIC_IDX3,
    MBENC_B_FRAME_LCU32_CU16x16_QP_DATA,
    MBENC_B_FRAME_LCU32_ENC_CONST_TABLE,
    MBENC_B_FRAME_LCU32_COLOCATED_CU_MV_DATA,
    MBENC_B_FRAME_LCU32_HME_MOTION_PREDICTOR_DATA,
    MBENC_B_FRAME_LCU32_LCU_LEVEL_DATA_INPUT,
    MBENC_B_FRAME_LCU32_LCU_ENC_SCRATCH_SURFACE,
    MBENC_B_FRAME_LCU32_CONCURRENT_TG_DATA,
    MBENC_B_FRAME_LCU32_BRC_COMBINED_ENC_PARAMETER_SURFACE,
    MBENC_B_FRAME_LCU32_JOB_QUEUE_SCRATCH_SURFACE,
    MBENC_B_FRAME_LCU32_CU_SPLIT_DATA_SURFACE,
    MBENC_B_FRAME_LCU32_RESIDUAL_DATA_SCRATCH_SURFACE,
    MBENC_B_FRAME_LCU32_DEBUG_SURFACE,
    MBENC_B_FRAME_LCU32_END,

    // MBEnc B-kernel -- Both for LCU64
    MBENC_B_FRAME_LCU64_BEGIN = 0,
    MBENC_B_FRAME_LCU64_CURR_Y = MBENC_B_FRAME_LCU64_BEGIN,
    MBENC_B_FRAME_LCU64_CURR_UV,
    MBENC_B_FRAME_LCU64_CU32_ENC_CU_RECORD,
    MBENC_B_FRAME_LCU64_SECOND_CU32_ENC_CU_RECORD,
    MBENC_B_FRAME_LCU64_PAK_OBJ,
    MBENC_B_FRAME_LCU64_PAK_CU_RECORD,
    MBENC_B_FRAME_LCU64_VME_PRED_CURR_PIC_IDX0,
    MBENC_B_FRAME_LCU64_VME_PRED_FWD_PIC_IDX0,
    MBENC_B_FRAME_LCU64_VME_PRED_BWD_PIC_IDX0,
    MBENC_B_FRAME_LCU64_VME_PRED_FWD_PIC_IDX1,
    MBENC_B_FRAME_LCU64_VME_PRED_BWD_PIC_IDX1,
    MBENC_B_FRAME_LCU64_VME_PRED_FWD_PIC_IDX2,
    MBENC_B_FRAME_LCU64_VME_PRED_BWD_PIC_IDX2,
    MBENC_B_FRAME_LCU64_VME_PRED_FWD_PIC_IDX3,
    MBENC_B_FRAME_LCU64_VME_PRED_BWD_PIC_IDX3,
    MBENC_B_FRAME_LCU64_CU16x16_QP_DATA,
    MBENC_B_FRAME_LCU64_CU32_ENC_CONST_TABLE,
    MBENC_B_FRAME_LCU64_COLOCATED_CU_MV_DATA,
    MBENC_B_FRAME_LCU64_HME_MOTION_PREDICTOR_DATA,
    MBENC_B_FRAME_LCU64_LCU_LEVEL_DATA_INPUT,
    MBENC_B_FRAME_LCU64_CU32_LCU_ENC_SCRATCH_SURFACE,
    MBENC_B_FRAME_LCU64_64X64_DISTORTION_SURFACE,
    MBENC_B_FRAME_LCU64_CONCURRENT_TG_DATA,
    MBENC_B_FRAME_LCU64_BRC_COMBINED_ENC_PARAMETER_SURFACE,
    MBENC_B_FRAME_LCU64_CU32_JOB_QUEUE_1D_SURFACE,
    MBENC_B_FRAME_LCU64_CU32_JOB_QUEUE_2D_SURFACE,
    MBENC_B_FRAME_LCU64_CU32_RESIDUAL_DATA_SCRATCH_SURFACE,
    MBENC_B_FRAME_LCU64_CU_SPLIT_DATA_SURFACE,
    MBENC_B_FRAME_LCU64_CURR_Y_2xDS,
    MBENC_B_FRAME_LCU64_INTERMEDIATE_CU_RECORD,
    MBENC_B_FRAME_LCU64_CONST64_DATA_LUT,
    MBENC_B_FRAME_LCU64_LCU_STORAGE_SURFACE,
    MBENC_B_FRAME_LCU64_VME_PRED_CURR_PIC_2xDS_IDX0,
    MBENC_B_FRAME_LCU64_VME_PRED_FWD_PIC_2xDS_IDX0,
    MBENC_B_FRAME_LCU64_VME_PRED_BWD_PIC_2xDS_IDX0,
    MBENC_B_FRAME_LCU64_VME_PRED_FWD_PIC_2xDS_IDX1,
    MBENC_B_FRAME_LCU64_VME_PRED_BWD_PIC_2xDS_IDX1,
    MBENC_B_FRAME_LCU64_VME_PRED_FWD_PIC_2xDS_IDX2,
    MBENC_B_FRAME_LCU64_VME_PRED_BWD_PIC_2xDS_IDX2,
    MBENC_B_FRAME_LCU64_VME_PRED_FWD_PIC_2xDS_IDX3,
    MBENC_B_FRAME_LCU64_VME_PRED_BWD_PIC_2xDS_IDX3,
    MBENC_B_FRAME_LCU64_JOB_QUEUE_1D_SURFACE,
    MBENC_B_FRAME_LCU64_JOB_QUEUE_2D_SURFACE,
    MBENC_B_FRAME_LCU64_RESIDUAL_DATA_SCRATCH_SURFACE,
    MBENC_B_FRAME_LCU64_DEBUG_SURFACE,
    MBENC_B_FRAME_LCU64_END,
};

//! \cond SKIP_DOXYGEN
//! Kernel header structure
struct CODECHAL_ENC_HEVC_KERNEL_HEADER_G10 {
    int32_t nKernelCount;

    union
    {
        // HEVC
        struct
        {
            CODECHAL_KERNEL_HEADER Gen10_HEVC_Intra;
            CODECHAL_KERNEL_HEADER Gen10_HEVC_Enc_B;
            CODECHAL_KERNEL_HEADER Gen10_HEVC_DS_Convert;
            CODECHAL_KERNEL_HEADER Gen10_HEVC_HME;
            CODECHAL_KERNEL_HEADER Gen10_HEVC_Enc_LCU64_B;
            CODECHAL_KERNEL_HEADER Gen10_HEVC_brc_init;
            CODECHAL_KERNEL_HEADER Gen10_HEVC_brc_lcuqp;
            CODECHAL_KERNEL_HEADER Gen10_HEVC_brc_reset;
            CODECHAL_KERNEL_HEADER Gen10_HEVC_brc_update;
            CODECHAL_KERNEL_HEADER Gen10_HEVC_brc_blockcopy;    // not used so far
        };
    };
};

using PCODECHAL_ENC_HEVC_KERNEL_HEADER_G10 = struct CODECHAL_ENC_HEVC_KERNEL_HEADER_G10*;

//! Structure for LCU level data
struct CODECHAL_ENC_HEVC_LCU_LEVEL_DATA_G10
{
    uint16_t SliceStartLcuIndex;
    uint16_t SliceEndLcuIndex;
    uint16_t SliceId;
    uint16_t SliceLevelQP;
    uint16_t Reserved[4];
};
using PCODECHAL_ENC_HEVC_LCU_LEVEL_DATA_G10 = struct CODECHAL_ENC_HEVC_LCU_LEVEL_DATA_G10*;

//! Concurrent thread group data structure
struct CODECHAL_ENC_HEVC_CONCURRENT_THREAD_GROUP_DATA_G10
{
    uint16_t CurrTgStartLcuIndex;
    uint16_t CurrTgEndLcuIndex;
    uint16_t CurrTgIndex;
    uint16_t Reserved0;
    uint16_t CurrWfLcuIndex_x;
    uint16_t CurrWfLcuIndex_y;
    uint16_t CurrWfLcuIndex1_y;
    uint16_t NextWfLcuIndex_x;
    uint16_t CurrWfYoffset;
    uint16_t Reserved[23];
};

using PCODECHAL_ENC_HEVC_CONCURRENT_THREAD_GROUP_DATA_G10 = struct CODECHAL_ENC_HEVC_CONCURRENT_THREAD_GROUP_DATA_G10*;

//! curbe structure for ME kernel
struct CODECHAL_ENC_HEVC_ME_CURBE_G10
{
    // DWORD 0
    uint32_t   DW0_RoundedFrameWidthInMvUnitsfor4X     : MOS_BITFIELD_RANGE( 0, 15);
    uint32_t   DW0_RoundedFrameHeightInMvUnitsfor4X    : MOS_BITFIELD_RANGE(16, 31);

    // DWORD 1
    uint32_t   DW1_Reserved_0                          : MOS_BITFIELD_RANGE( 0, 15); // MBZ
    uint32_t   DW1_MvCostScaleFactor                   : MOS_BITFIELD_RANGE(16, 17); // This parameter allows the user to redefine the precision of the lookup into the LUT_MV based on the MV cost difference from the cost center
    uint32_t   DW1_Reserved_1                          : MOS_BITFIELD_RANGE(18, 31); // Default value to enable Fractional Motion Estimation: 0x11[3]

    // DWORD 2
    uint32_t   DW2_Reserved_0                          : MOS_BITFIELD_RANGE( 0, 15); // MBZ
    uint32_t   DW2_SubPelMode                          : MOS_BITFIELD_RANGE(16, 17);
    uint32_t   DW2_BmeDisableFbr                       : MOS_BITFIELD_BIT(      18);
    uint32_t   DW2_Reserved_1                          : MOS_BITFIELD_BIT(      19); // MBZ
    uint32_t   DW2_InterSADMeasureAdjustment           : MOS_BITFIELD_RANGE(20, 21); // This field specifies distortion measure adjustments used for the motion search SAD comparison.
    uint32_t   DW2_Reserved_2                          : MOS_BITFIELD_RANGE(22, 31); // MBZ

    // DWORD 3
    uint32_t   DW3_Reserved_0                          : MOS_BITFIELD_BIT(       0); // MBZ
    uint32_t   DW3_AdaptiveSearchEnable                : MOS_BITFIELD_BIT(       1); // This field determines whether adaptive search is enabled or not.
    uint32_t   DW3_Reserved_1                          : MOS_BITFIELD_RANGE( 2, 15); // MBZ
    uint32_t   DW3_ImeRefWindowSize                    : MOS_BITFIELD_RANGE(16, 17);
    uint32_t   DW3_Reserved_2                          : MOS_BITFIELD_RANGE(18, 31); // MBZ

    // DWORD 4
    uint32_t   DW4_Reserved_0                          : MOS_BITFIELD_RANGE( 0,  7); // MBZ
    uint32_t   DW4_QuarterQuadTreeCandidate            : MOS_BITFIELD_RANGE( 8, 12); // This parameter indicates the current 32x32 block CU candidate that is being checked.
    uint32_t   DW4_Reserved_1                          : MOS_BITFIELD_RANGE(13, 15); // MBZ
    uint32_t   DW4_BidirectionalWeight                 : MOS_BITFIELD_RANGE(16, 21); // This field defines the weighting for the backward and forward terms to generate the bidirectional term.
    uint32_t   DW4_Reserved_2                          : MOS_BITFIELD_RANGE(22, 31); // MBZ

    // DWORD 5
    uint32_t   DW5_LenSP                               : MOS_BITFIELD_RANGE( 0,  7); // Maximum Fixed Search Path Length. This field determines the maximum number of SUs per reference which are evaluated by predetermined SUs.
    uint32_t   DW5_MaxNumSU                            : MOS_BITFIELD_RANGE( 8, 15); // Maximum Search Path Length. This field determines the maximum number of SUs per reference including predetermined SUs and the adaptively generated SUs.
    uint32_t   DW5_StartCenter0_X                      : MOS_BITFIELD_RANGE(16, 19); // This field defines the Y position of Search Path 1 relative to the reference X location.
    uint32_t   DW5_StartCenter0_Y                      : MOS_BITFIELD_RANGE(20, 23); // This field defines the Y position of Search Path 1 relative to the reference Y location.
    uint32_t   DW5_Reserved_0                          : MOS_BITFIELD_RANGE(24, 31); // MBZ

    // DWORD 6
    uint32_t   DW6_Reserved_0                          : MOS_BITFIELD_BIT(       0); // MBZ
    uint32_t   DW6_SliceType                           : MOS_BITFIELD_BIT(       1);
    uint32_t   DW6_HmeStage                            : MOS_BITFIELD_RANGE( 2,  3);
    uint32_t   DW6_NumRefL0                            : MOS_BITFIELD_RANGE( 4,  5); // Valid Number of Forward L0 references
    uint32_t   DW6_NumRefL1                            : MOS_BITFIELD_RANGE( 6,  7); // Valid Number of Backward L1 references
    uint32_t   DW6_Reserved_1                          : MOS_BITFIELD_RANGE( 8, 31); // MBZ

    // DWORD 7
    uint32_t   DW7_RoundedFrameWidthInMvUnitsFor16x    : MOS_BITFIELD_RANGE( 0, 15);
    uint32_t   DW7_RoundedFrameHeightInMvUnitsfor16X   : MOS_BITFIELD_RANGE(16, 31);

    // DWORD 8
    uint32_t   DW8_ImeSearchPath_0_3                   : MOS_BITFIELD_RANGE( 0, 31);

    // DWORD 9
    uint32_t   DW9_ImeSearchPath_4_7                   : MOS_BITFIELD_RANGE( 0, 31);

    // DWORD 10
    uint32_t   DW10_ImeSearchPath_8_11                 : MOS_BITFIELD_RANGE( 0, 31);

    // DWORD 11
    uint32_t   DW11_ImeSearchPath_12_15                : MOS_BITFIELD_RANGE( 0, 31);

    // DWORD 12
    uint32_t   DW12_ImeSearchPath_16_19                : MOS_BITFIELD_RANGE( 0, 31);

    // DWORD 13
    uint32_t   DW13_ImeSearchPath_20_23                : MOS_BITFIELD_RANGE( 0, 31);

    // DWORD 14
    uint32_t   DW14_ImeSearchPath_24_27                : MOS_BITFIELD_RANGE( 0, 31);

    // DWORD 15
    uint32_t   DW15_ImeSearchPath_28_31                : MOS_BITFIELD_RANGE( 0, 31);

    // DWORD 16
    uint32_t   DW16_ImeSearchPath_32_35                : MOS_BITFIELD_RANGE( 0, 31);

    // DWORD 17
    uint32_t   DW17_ImeSearchPath_36_39                : MOS_BITFIELD_RANGE( 0, 31);

    // DWORD 18
    uint32_t   DW18_ImeSearchPath_40_43                : MOS_BITFIELD_RANGE( 0, 31);

    // DWORD 19
    uint32_t   DW19_ImeSearchPath_44_47                : MOS_BITFIELD_RANGE( 0, 31);

    // DWORD 20
    uint32_t   DW20_ImeSearchPath_48_51                : MOS_BITFIELD_RANGE( 0, 31);

    // DWORD 21
    uint32_t   DW21_ImeSearchPath_52_55                : MOS_BITFIELD_RANGE( 0, 31);

    // DWORD 22
    uint32_t   DW22_ImeSearchPath_56_59                : MOS_BITFIELD_RANGE( 0, 31);

    // DWORD 23
    uint32_t   DW23_ImeSearchPath_60_63                : MOS_BITFIELD_RANGE( 0, 31);

    // DWORD 24
    uint32_t   DW24_Reserved_0                         : MOS_BITFIELD_RANGE( 0,  5); // MBZ
    uint32_t   DW24_CodingUnitSize                     : MOS_BITFIELD_RANGE( 6,  7);
    uint32_t   DW24_Reserved_1                         : MOS_BITFIELD_RANGE( 8, 11); // MBZ
    uint32_t   DW24_CodingUnitPartitionMode            : MOS_BITFIELD_RANGE(12, 14);
    uint32_t   DW24_CodingUnitPredictionMode           : MOS_BITFIELD_BIT(      15);
    uint32_t   DW24_Reserved_2                         : MOS_BITFIELD_RANGE(16, 31); // MBZ

    // DWORD 25
    uint32_t   DW25_FrameWidthInSamplesOfCurrentStage  : MOS_BITFIELD_RANGE( 0, 15);
    uint32_t   DW25_FrameHeightInSamplesOfCurrentStage : MOS_BITFIELD_RANGE(16, 31);

    // DWORD 26
    uint32_t   DW26_Intra8x8ModeMask                   : MOS_BITFIELD_RANGE( 0,  9);
    uint32_t   DW26_Reserved_0                         : MOS_BITFIELD_RANGE(10, 15); // MBZ
    uint32_t   DW26_Intra16x16ModeMask                 : MOS_BITFIELD_RANGE(16, 24);
    uint32_t   DW26_Reserved_1                         : MOS_BITFIELD_RANGE(25, 31);

    // DWORD 27
    uint32_t   DW27_Intra32x32ModeMask                 : MOS_BITFIELD_RANGE( 0,  3);
    uint32_t   DW27_IntraChromaModeMask                : MOS_BITFIELD_RANGE( 4,  8);
    uint32_t   DW27_IntraComputeType                   : MOS_BITFIELD_RANGE( 9, 10);
    uint32_t   DW27_Reserved_0                         : MOS_BITFIELD_RANGE(11, 31); // MBZ

    // DWORD 28
    uint32_t   DW28_Reserved_0                         : MOS_BITFIELD_RANGE( 0,  7); // MBZ
    uint32_t   DW28_PenaltyIntra32x32NonDC             : MOS_BITFIELD_RANGE( 8, 15);
    uint32_t   DW28_PenaltyIntra16x16NonDC             : MOS_BITFIELD_RANGE(16, 23);
    uint32_t   DW28_PenaltyIntra8x8NonDC               : MOS_BITFIELD_RANGE(24, 31);

    // DWORD 29
    uint32_t   DW29_Mode0Cost                          : MOS_BITFIELD_RANGE( 0,  7);
    uint32_t   DW29_Mode1Cost                          : MOS_BITFIELD_RANGE( 8, 15);
    uint32_t   DW29_Mode2Cost                          : MOS_BITFIELD_RANGE(16, 23);
    uint32_t   DW29_Mode3Cost                          : MOS_BITFIELD_RANGE(24, 31);

    // DWORD 30
    uint32_t   DW30_Mode4Cost                          : MOS_BITFIELD_RANGE( 0,  7);
    uint32_t   DW30_Mode5Cost                          : MOS_BITFIELD_RANGE( 8, 15);
    uint32_t   DW30_Mode6Cost                          : MOS_BITFIELD_RANGE(16, 23);
    uint32_t   DW30_Mode7Cost                          : MOS_BITFIELD_RANGE(24, 31);

    // DWORD 31
    uint32_t   DW31_Mode8Cost                          : MOS_BITFIELD_RANGE( 0,  7);
    uint32_t   DW31_Mode9Cost                          : MOS_BITFIELD_RANGE( 8, 15);
    uint32_t   DW31_Reserved_0                         : MOS_BITFIELD_RANGE(16, 23); // MBZ
    uint32_t   DW31_ChromaIntraModeCost                : MOS_BITFIELD_RANGE(24, 31);

    // DWORD 32
    uint32_t   DW32_Reserved_0                         : MOS_BITFIELD_RANGE( 0,  7); // MBZ
    uint32_t   DW32_SicIntraNeighborAvailableFlag      : MOS_BITFIELD_RANGE( 8, 13);
    uint32_t   DW32_Reserved_1                         : MOS_BITFIELD_RANGE(14, 19); // MBZ
    uint32_t   DW32_SicInterSadMeasure                 : MOS_BITFIELD_RANGE(20, 21);
    uint32_t   DW32_SicIntraSadMeasure                 : MOS_BITFIELD_RANGE(22, 23);
    uint32_t   DW32_Reserved_2                         : MOS_BITFIELD_RANGE(24, 31); // MBZ

    // DWORD 33
    uint32_t   DW33_SicLog2MinCuSize                   : MOS_BITFIELD_RANGE( 0,  7);
    uint32_t   DW33_Reserved_0                         : MOS_BITFIELD_RANGE( 8, 19); // MBZ
    uint32_t   DW33_SicAcOnlyHaar                      : MOS_BITFIELD_BIT(      20);
    uint32_t   DW33_Reserved_1                         : MOS_BITFIELD_RANGE(21, 23); // MBZ
    uint32_t   DW33_SicHevcQuarterQuadtree             : MOS_BITFIELD_RANGE(24, 28);
    uint32_t   DW33_Reserved_2                         : MOS_BITFIELD_RANGE(29, 31); // MBZ

    // DWORD 34
    uint32_t   DW34_BTI_HmeOutputMvDataSurface         : MOS_BITFIELD_RANGE( 0, 31);

    // DWORD 35
    uint32_t   DW35_BTI_16xInputMvDataSurface          : MOS_BITFIELD_RANGE( 0, 31); // Only applicable for 4x Stage

    // DWORD 36
    uint32_t   DW36_BTI_4xOutputDistortionSurface      : MOS_BITFIELD_RANGE( 0, 31);

    // DWORD 37
    uint32_t   DW37_BTI_VmeSurfaceIndex                : MOS_BITFIELD_RANGE( 0, 31);

    // DWORD 38
    uint32_t   DW38_BTI_4xDsSurface                    : MOS_BITFIELD_RANGE( 0, 31);

    // DWORD 39
    uint32_t   DW39_BTI_BrcDistortionSurface           : MOS_BITFIELD_RANGE( 0, 31);

    // DWORD 40
    uint32_t   DW40_BTI_Mv_And_Distortion_SumSurface   : MOS_BITFIELD_RANGE( 0, 31);
};

C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(CODECHAL_ENC_HEVC_ME_CURBE_G10)) == 41);

//! curbe structure for BRC InitReset kernel
struct CODECHAL_ENC_HEVC_BRC_INITRESET_CURBE_G10
{
    // DWORD 0
   uint32_t   DW0_ProfileLevelMaxFrame                : MOS_BITFIELD_RANGE( 0, 31);

    // DWORD 1
   uint32_t   DW1_InitBufFull                         : MOS_BITFIELD_RANGE( 0, 31);

    // DWORD 2
   uint32_t   DW2_BufSize                             : MOS_BITFIELD_RANGE( 0, 31);

    // DWORD 3
   uint32_t   DW3_TargetBitRate                       : MOS_BITFIELD_RANGE( 0, 31);

    // DWORD 4
   uint32_t   DW4_MaximumBitRate                      : MOS_BITFIELD_RANGE( 0, 31);

    // DWORD 5
   uint32_t   DW5_MinimumBitRate                      : MOS_BITFIELD_RANGE( 0, 31);

    // DWORD 6
   uint32_t   DW6_FrameRateM                          : MOS_BITFIELD_RANGE( 0, 31);

    // DWORD 7
   uint32_t   DW7_FrameRateD                          : MOS_BITFIELD_RANGE( 0, 31);

    // DWORD 8
   uint32_t   DW8_BRCFlag                             : MOS_BITFIELD_RANGE( 0, 15);
   uint32_t   DW8_BRC_Param_A                         : MOS_BITFIELD_RANGE(16, 31);

    // DWORD 9
   uint32_t   DW9_BRC_Param_B                         : MOS_BITFIELD_RANGE( 0, 15);
   uint32_t   DW9_FrameWidth                          : MOS_BITFIELD_RANGE(16, 31);

    // DWORD 10
   uint32_t   DW10_FrameHeight                        : MOS_BITFIELD_RANGE( 0, 15);
   uint32_t   DW10_AVBRAccuracy                       : MOS_BITFIELD_RANGE(16, 31);

    // DWORD 11
   uint32_t   DW11_AVBRConvergence                    : MOS_BITFIELD_RANGE( 0, 15);
   uint32_t   DW11_MinimumQP                          : MOS_BITFIELD_RANGE(16, 31);

    // DWORD 12
   uint32_t   DW12_MaximumQP                          : MOS_BITFIELD_RANGE( 0, 15);
   uint32_t   DW12_NumberSlice                        : MOS_BITFIELD_RANGE(16, 31);

    // DWORD 13
   uint32_t   DW13_Reserved_0                         : MOS_BITFIELD_RANGE( 0, 15);
   uint32_t   DW13_BRC_Param_C                        : MOS_BITFIELD_RANGE(16, 31);

    // DWORD 14
   uint32_t   DW14_BRC_Param_D                        : MOS_BITFIELD_RANGE( 0, 15);
   uint32_t   DW14_MaxBRCLevel                        : MOS_BITFIELD_RANGE(16, 31);

    // DWORD 15
   uint32_t   DW15_LongTermInterval                   : MOS_BITFIELD_RANGE( 0, 15);
   uint32_t   DW15_Reserved_0                         : MOS_BITFIELD_RANGE( 16, 31);

    // DWORD 16
   uint32_t   DW16_InstantRateThreshold0_Pframe       : MOS_BITFIELD_RANGE( 0,  7);
   uint32_t   DW16_InstantRateThreshold1_Pframe       : MOS_BITFIELD_RANGE( 8, 15);
   uint32_t   DW16_InstantRateThreshold2_Pframe       : MOS_BITFIELD_RANGE(16, 23);
   uint32_t   DW16_InstantRateThreshold3_Pframe       : MOS_BITFIELD_RANGE(24, 31);

    // DWORD 17
   uint32_t   DW17_InstantRateThreshold0_Bframe       : MOS_BITFIELD_RANGE( 0,  7);
   uint32_t   DW17_InstantRateThreshold1_Bframe       : MOS_BITFIELD_RANGE( 8, 15);
   uint32_t   DW17_InstantRateThreshold2_Bframe       : MOS_BITFIELD_RANGE(16, 23);
   uint32_t   DW17_InstantRateThreshold3_Bframe       : MOS_BITFIELD_RANGE(24, 31);

    // DWORD 18
   uint32_t   DW18_InstantRateThreshold0_Iframe       : MOS_BITFIELD_RANGE( 0,  7);
   uint32_t   DW18_InstantRateThreshold1_Iframe       : MOS_BITFIELD_RANGE( 8, 15);
   uint32_t   DW18_InstantRateThreshold2_Iframe       : MOS_BITFIELD_RANGE(16, 23);
   uint32_t   DW18_InstantRateThreshold3_Iframe       : MOS_BITFIELD_RANGE(24, 31);

    // DWORD 19
   uint32_t   DW19_DeviationThreshold0_PBframe        : MOS_BITFIELD_RANGE( 0,  7);
   uint32_t   DW19_DeviationThreshold1_PBframe        : MOS_BITFIELD_RANGE( 8, 15);
   uint32_t   DW19_DeviationThreshold2_PBframe        : MOS_BITFIELD_RANGE(16, 23);
   uint32_t   DW19_DeviationThreshold3_PBframe        : MOS_BITFIELD_RANGE(24, 31);

    // DWORD 20
   uint32_t   DW20_DeviationThreshold4_PBframe        : MOS_BITFIELD_RANGE( 0,  7);
   uint32_t   DW20_DeviationThreshold5_PBframe        : MOS_BITFIELD_RANGE( 8, 15);
   uint32_t   DW20_DeviationThreshold6_PBframe        : MOS_BITFIELD_RANGE(16, 23);
   uint32_t   DW20_DeviationThreshold7_PBframe        : MOS_BITFIELD_RANGE(24, 31);

    // DWORD 21
   uint32_t   DW21_DeviationThreshold0_VBRcontrol     : MOS_BITFIELD_RANGE( 0,  7);
   uint32_t   DW21_DeviationThreshold1_VBRcontrol     : MOS_BITFIELD_RANGE( 8, 15);
   uint32_t   DW21_DeviationThreshold2_VBRcontrol     : MOS_BITFIELD_RANGE(16, 23);
   uint32_t   DW21_DeviationThreshold3_VBRcontrol     : MOS_BITFIELD_RANGE(24, 31);

    // DWORD 22
   uint32_t   DW22_DeviationThreshold4_VBRcontrol     : MOS_BITFIELD_RANGE( 0,  7);
   uint32_t   DW22_DeviationThreshold5_VBRcontrol     : MOS_BITFIELD_RANGE( 8, 15);
   uint32_t   DW22_DeviationThreshold6_VBRcontrol     : MOS_BITFIELD_RANGE(16, 23);
   uint32_t   DW22_DeviationThreshold7_VBRcontrol     : MOS_BITFIELD_RANGE(24, 31);

    // DWORD 23
   uint32_t   DW23_DeviationThreshold0_Iframe         : MOS_BITFIELD_RANGE( 0,  7);
   uint32_t   DW23_DeviationThreshold1_Iframe         : MOS_BITFIELD_RANGE( 8, 15);
   uint32_t   DW23_DeviationThreshold2_Iframe         : MOS_BITFIELD_RANGE(16, 23);
   uint32_t   DW23_DeviationThreshold3_Iframe         : MOS_BITFIELD_RANGE(24, 31);

    // DWORD 24
   uint32_t   DW24_DeviationThreshold4_Iframe         : MOS_BITFIELD_RANGE( 0,  7);
   uint32_t   DW24_DeviationThreshold5_Iframe         : MOS_BITFIELD_RANGE( 8, 15);
   uint32_t   DW24_DeviationThreshold6_Iframe         : MOS_BITFIELD_RANGE(16, 23);
   uint32_t   DW24_DeviationThreshold7_Iframe         : MOS_BITFIELD_RANGE(24, 31);

    // DWORD 25
   uint32_t   DW25_ACQPBuffer                         : MOS_BITFIELD_RANGE( 0,  7);
   uint32_t   DW25_IntraSADTransform                  : MOS_BITFIELD_RANGE( 8, 15);
   uint32_t   DW25_Log2MaxCuSize                      : MOS_BITFIELD_RANGE(16, 23);
   uint32_t   DW25_SlidingWindowSize                  : MOS_BITFIELD_RANGE(24, 31);

    // DWORD 26
   uint32_t   DW26_BGOPSize                           : MOS_BITFIELD_RANGE( 0,  7);
   uint32_t   DW26_Reserved_0                         : MOS_BITFIELD_RANGE( 8, 31);

    // DWORD 27
   uint32_t   DW27_Reserved_0                         : MOS_BITFIELD_RANGE( 0, 31);

    // DWORD 28
   uint32_t   DW28_Reserved_0                         : MOS_BITFIELD_RANGE( 0, 31);

    // DWORD 29
   uint32_t   DW29_Reserved_0                         : MOS_BITFIELD_RANGE( 0, 31);

    // DWORD 30
   uint32_t   DW30_Reserved_0                         : MOS_BITFIELD_RANGE( 0, 31);

    // DWORD 31
   uint32_t   DW31_Reserved_0                         : MOS_BITFIELD_RANGE( 0, 31);
};

C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(CODECHAL_ENC_HEVC_BRC_INITRESET_CURBE_G10)) == 32);

//! curbe structure for BRC Updtae kernel
struct CODECHAL_ENC_HEVC_BRC_UPDATE_CURBE_G10
{
    // DWORD 0
    uint32_t   DW0_TargetSize                          : MOS_BITFIELD_RANGE( 0, 31);

    // DWORD 1
    uint32_t   DW1_FrameNumber                         : MOS_BITFIELD_RANGE( 0, 31);

    // DWORD 2
    uint32_t   DW2_PictureHeaderSize                   : MOS_BITFIELD_RANGE( 0, 31);

    // DWORD 3
    uint32_t   DW3_StartGAdjFrame0                     : MOS_BITFIELD_RANGE( 0, 15);
    uint32_t   DW3_StartGAdjFrame1                     : MOS_BITFIELD_RANGE(16, 31);

    // DWORD 4
    uint32_t   DW4_StartGAdjFrame2                     : MOS_BITFIELD_RANGE( 0, 15);
    uint32_t   DW4_StartGAdjFrame3                     : MOS_BITFIELD_RANGE(16, 31);

    // DWORD 5
    uint32_t   DW5_TargetSize_Flag                     : MOS_BITFIELD_RANGE( 0,  7);
    uint32_t   DW5_Reserved_0                          : MOS_BITFIELD_RANGE( 8, 15);
    uint32_t   DW5_MaxNumPAKs                          : MOS_BITFIELD_RANGE(16, 23);
    uint32_t   DW5_CurrFrameBrcLevel                   : MOS_BITFIELD_RANGE(24, 31);

    // DWORD 6
    uint32_t   DW6_NumSkippedFrames                    : MOS_BITFIELD_RANGE( 0,  7);
    uint32_t   DW6_CqpValue                            : MOS_BITFIELD_RANGE( 8, 15);
    uint32_t   DW6_ROIEnable                           : MOS_BITFIELD_RANGE(16, 16);
    uint32_t   DW6_BRCROIEnable                        : MOS_BITFIELD_RANGE(17, 17);
    uint32_t   DW6_LCUQPAverageEnable                  : MOS_BITFIELD_RANGE(18, 18);
    uint32_t   DW6_Reserved1                           : MOS_BITFIELD_RANGE(19, 19);
    uint32_t   DW6_SlidingWindowEnable                 : MOS_BITFIELD_RANGE(20, 20);
    uint32_t   DW6_Reserved2                           : MOS_BITFIELD_RANGE(21, 23);
    uint32_t   DW6_RoiRatio                            : MOS_BITFIELD_RANGE(24, 31);

    // DWORD 7
    uint32_t   DW7_Reserved_0                          : MOS_BITFIELD_RANGE( 0, 31);

    // DWORD 8
    uint32_t   DW8_StartGlobalAdjustMult0              : MOS_BITFIELD_RANGE( 0,  7);
    uint32_t   DW8_StartGlobalAdjustMult1              : MOS_BITFIELD_RANGE( 8, 15);
    uint32_t   DW8_StartGlobalAdjustMult2              : MOS_BITFIELD_RANGE(16, 23);
    uint32_t   DW8_StartGlobalAdjustMult3              : MOS_BITFIELD_RANGE(24, 31);

    // DWORD 9
    uint32_t   DW9_StartGlobalAdjustMult4              : MOS_BITFIELD_RANGE( 0,  7);
    uint32_t   DW9_StartGlobalAdjustDivd0              : MOS_BITFIELD_RANGE( 8, 15);
    uint32_t   DW9_StartGlobalAdjustDivd1              : MOS_BITFIELD_RANGE(16, 23);
    uint32_t   DW9_StartGlobalAdjustDivd2              : MOS_BITFIELD_RANGE(24, 31);

    // DWORD 10
    uint32_t   DW10_StartGlobalAdjustDivd3             : MOS_BITFIELD_RANGE( 0,  7);
    uint32_t   DW10_StartGlobalAdjustDivd4             : MOS_BITFIELD_RANGE( 8, 15);
    uint32_t   DW10_QPThreshold0                       : MOS_BITFIELD_RANGE(16, 23);
    uint32_t   DW10_QPThreshold1                       : MOS_BITFIELD_RANGE(24, 31);

    // DWORD 11
    uint32_t   DW11_QPThreshold2                       : MOS_BITFIELD_RANGE( 0,  7);
    uint32_t   DW11_QPThreshold3                       : MOS_BITFIELD_RANGE( 8, 15);
    uint32_t   DW11_gRateRatioThreshold0               : MOS_BITFIELD_RANGE(16, 23);
    uint32_t   DW11_gRateRatioThreshold1               : MOS_BITFIELD_RANGE(24, 31);

    // DWORD 12
    uint32_t   DW12_gRateRatioThreshold2               : MOS_BITFIELD_RANGE( 0,  7);
    uint32_t   DW12_gRateRatioThreshold3               : MOS_BITFIELD_RANGE( 8, 15);
    uint32_t   DW12_gRateRatioThreshold4               : MOS_BITFIELD_RANGE(16, 23);
    uint32_t   DW12_gRateRatioThreshold5               : MOS_BITFIELD_RANGE(24, 31);

    // DWORD 13
    uint32_t   DW13_gRateRatioThreshold6               : MOS_BITFIELD_RANGE( 0,  7);
    uint32_t   DW13_gRateRatioThreshold7               : MOS_BITFIELD_RANGE( 8, 15);
    uint32_t   DW13_gRateRatioThreshold8               : MOS_BITFIELD_RANGE(16, 23);
    uint32_t   DW13_gRateRatioThreshold9               : MOS_BITFIELD_RANGE(24, 31);

    // DWORD 14
    uint32_t   DW14_gRateRatioThreshold10              : MOS_BITFIELD_RANGE( 0,  7);
    uint32_t   DW14_gRateRatioThreshold11              : MOS_BITFIELD_RANGE( 8, 15);
    uint32_t   DW14_gRateRatioThreshold12              : MOS_BITFIELD_RANGE(16, 23);
    uint32_t   DW14_ParallelMode                       : MOS_BITFIELD_RANGE(24, 31);

    // DWORD 15
    uint32_t   DW15_SizeOfSkippedFrames                : MOS_BITFIELD_RANGE( 0, 31);
};

C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(CODECHAL_ENC_HEVC_BRC_UPDATE_CURBE_G10)) == 16);

//! curbe structure for MBENC I kernel
struct CODECHAL_ENC_HEVC_MBENC_I_CURBE_G10
{
    // DWORD 0
    uint32_t   DW0_FrameWidthInSamples                     : MOS_BITFIELD_RANGE( 0, 15); // PicW should be a multiple of 8
    uint32_t   DW0_FrameHeightInSamples                    : MOS_BITFIELD_RANGE(16, 31); // PicH should be a multiple of 8

    // DWORD 1
    uint32_t   DW1_Reserved_0                              : MOS_BITFIELD_RANGE( 0,  7); // MBZ
    uint32_t   DW1_PenaltyForIntra32x32NonDCPredMode       : MOS_BITFIELD_RANGE( 8, 15);
    uint32_t   DW1_PenaltyForIntra16x16NonDCPredMode       : MOS_BITFIELD_RANGE(16, 23);
    uint32_t   DW1_PenaltyForIntra8x8NonDCPredMode         : MOS_BITFIELD_RANGE(24, 31);

    // DWORD 2
    uint32_t   DW2_Reserved_0                              : MOS_BITFIELD_RANGE( 0,  5); // MBZ
    uint32_t   DW2_IntraSADMeasureAdj                      : MOS_BITFIELD_RANGE( 6,  7);
    uint32_t   DW2_IntraPrediction                         : MOS_BITFIELD_RANGE( 8, 10);
    uint32_t   DW2_Reserved_1                              : MOS_BITFIELD_RANGE(11, 31); // MBZ

    // DWORD 3
    uint32_t   DW3_ModeCost_0                              : MOS_BITFIELD_RANGE( 0,  7); // MODE_INTRA_NONPRED
    uint32_t   DW3_ModeCost_1                              : MOS_BITFIELD_RANGE( 8, 15); // MODE_INTRA_32x32
    uint32_t   DW3_ModeCost_2                              : MOS_BITFIELD_RANGE(16, 23); // MODE_INTRA_16x16
    uint32_t   DW3_ModeCost_3                              : MOS_BITFIELD_RANGE(24, 31); // MODE_INTRA_8x8

    // DWORD 4
    uint32_t   DW4_ModeCost_4                              : MOS_BITFIELD_RANGE( 0,  7); // MODE_INTER_32x16, MODE_INTER_16x32, MODE_INTER_AMP shapes
    uint32_t   DW4_ModeCost_5                              : MOS_BITFIELD_RANGE( 8, 15); // MODE_INTER_16x16
    uint32_t   DW4_ModeCost_6                              : MOS_BITFIELD_RANGE(16, 23); // MODE_INTER_16x8, MODE_INTER_8x16
    uint32_t   DW4_ModeCost_7                              : MOS_BITFIELD_RANGE(24, 31); // MODE_INTER_8x8

    // DWORD 5
    uint32_t   DW5_ModeCost_8                              : MOS_BITFIELD_RANGE( 0,  7); // MODE_INTER_32x32
    uint32_t   DW5_ModeCost_9                              : MOS_BITFIELD_RANGE( 8, 15); // MODE_INTER_BIDIR
    uint32_t   DW5_RefIDCost                               : MOS_BITFIELD_RANGE(16, 23); // RefID costing based penalty.
    uint32_t   DW5_ChromaIntraModeCost                     : MOS_BITFIELD_RANGE(24, 31); // Penalty for chroma intra modes.

    // DWORD 6
    uint32_t   DW6_Log2MaxCUSize                           : MOS_BITFIELD_RANGE( 0,  3);
    uint32_t   DW6_Log2MinCUSize                           : MOS_BITFIELD_RANGE( 4,  7);
    uint32_t   DW6_Log2MaxTUSize                           : MOS_BITFIELD_RANGE( 8, 11);
    uint32_t   DW6_Log2MinTUSize                           : MOS_BITFIELD_RANGE(12, 15);
    uint32_t   DW6_MaxTransformDepthIntra                  : MOS_BITFIELD_RANGE(16, 19);
    uint32_t   DW6_TuSplitFlag                             : MOS_BITFIELD_BIT(      20);
    uint32_t   DW6_TuBasedCostSetting                      : MOS_BITFIELD_RANGE(21, 23);
    uint32_t   DW6_Reserved_0                              : MOS_BITFIELD_RANGE(24, 31); // MBZ

    // DWORD 7
    uint32_t   DW7_ConcurrentGroupNum                      : MOS_BITFIELD_RANGE( 0,  7); // MBZ
    uint32_t   DW7_EncTuDecisionMode                       : MOS_BITFIELD_RANGE( 8,  9);
    uint32_t   DW7_Reserved_0                              : MOS_BITFIELD_RANGE(10, 23); // MBZ
    uint32_t   DW7_SliceQP                                 : MOS_BITFIELD_RANGE(24, 31);

    // DWORD 8
    uint32_t   DW8_Lambda_Rd                               : MOS_BITFIELD_RANGE( 0, 31); // Derived from QP value and used for TU decision

    // DWORD 9
    uint32_t   DW9_Lambda_Md                               : MOS_BITFIELD_RANGE( 0, 15); // Derived from QP value and used for distortion related calc
    uint32_t   DW9_Reserved_0                              : MOS_BITFIELD_RANGE(16, 31); // MBZ

    // DWORD 10
    uint32_t   DW10_IntraTuDThres                          : MOS_BITFIELD_RANGE( 0, 31); // Intra TU Distortion Threshold

    // DWORD 11
    uint32_t   DW11_SliceType                              : MOS_BITFIELD_RANGE( 0,  1);
    uint32_t   DW11_QPType                                 : MOS_BITFIELD_RANGE( 2,  3);
    uint32_t   DW11_CheckPcmModeFlag                       : MOS_BITFIELD_BIT(       4);
    uint32_t   DW11_EnableIntra4x4PU                       : MOS_BITFIELD_BIT(       5);
    uint32_t   DW11_EncQtDecisionMode                      : MOS_BITFIELD_BIT(       6);
    uint32_t   DW11_Reserved_0                             : MOS_BITFIELD_RANGE( 7, 31); // MBZ

    // DWORD 12
    uint32_t   DW12_PCM_8x8_SAD_Threshold                  : MOS_BITFIELD_RANGE( 0, 15);
    uint32_t   DW12_Reserved_0                             : MOS_BITFIELD_RANGE(16, 31); // MBZ

    // DWORD 13
    uint32_t   DW13_Reserved_0                             : MOS_BITFIELD_RANGE( 0, 31); // MBZ

    // DWORD 14
    uint32_t   DW14_Reserved_0                             : MOS_BITFIELD_RANGE( 0, 31); // MBZ

    // DWORD 15
    uint32_t   DW15_Reserved_0                             : MOS_BITFIELD_RANGE( 0, 31); // MBZ

    // DWORD 16
    uint32_t   DW16_BTI_VmeIntraPredictionSurface          : MOS_BITFIELD_RANGE( 0, 31); // Current pixel surface accessed by VME hardware

    // DWORD 17
    uint32_t   DW17_BTI_CurrentPictureY                    : MOS_BITFIELD_RANGE( 0, 31); // Current Y pixel surface accessed thorough data port by kernel

    // DWORD 18
    uint32_t   DW18_BTI_EncCuRecordSurface                 : MOS_BITFIELD_RANGE( 0, 31); // Surface to store intermediate CU records for kernel usage

    // DWORD 19
    uint32_t   DW19_BTI_PakObjectCommandSurface            : MOS_BITFIELD_RANGE( 0, 31); // Surface to write final PAK object commands

    // DWORD 20
    uint32_t   DW20_BTI_CuPacketForPakSurface              : MOS_BITFIELD_RANGE( 0, 31); // Surface to write CU packet

    // DWORD 21
    uint32_t   DW21_BTI_InternalScratchSurface             : MOS_BITFIELD_RANGE( 0, 31);

    // DWORD 22
    uint32_t   DW22_BTI_CuBasedQpSurface                   : MOS_BITFIELD_RANGE( 0, 31);

    // DWORD 23
    uint32_t   DW23_BTI_ConstantDataLutSurface             : MOS_BITFIELD_RANGE( 0, 31);

    // DWORD 24
    uint32_t   DW24_BTI_LcuLevelDataInputSurface           : MOS_BITFIELD_RANGE( 0, 31);

    // DWORD 25
    uint32_t   DW25_BTI_ConcurrentThreadGroupDataSurface   : MOS_BITFIELD_RANGE( 0, 31);

    // DWORD 26
    uint32_t   DW26_BTI_BrcCombinedEncParameterSurface     : MOS_BITFIELD_RANGE( 0, 31);

    // DWORD 27
    uint32_t   DW27_BTI_CuSplitSurface                     : MOS_BITFIELD_RANGE( 0, 31);

    // DWORD 28
    uint32_t   DW28_BTI_DebugSurface                       : MOS_BITFIELD_RANGE( 0, 31); // Used for debug purposes
};

C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(CODECHAL_ENC_HEVC_MBENC_I_CURBE_G10)) == 29);

//! curbe structure for MBENC B kernel
struct CODECHAL_ENC_HEVC_MBENC_B_CURBE_G10
{
    // DWORD 0
    uint32_t   DW0_FrameWidthInSamples                                     : MOS_BITFIELD_RANGE( 0, 15); // Input picture horizontal size in pixel. It should be the actual "to-be-encoded" size.
    uint32_t   DW0_FrameHeightInSamples                                    : MOS_BITFIELD_RANGE(16, 31); // Input picture vertical size in pixel. It should be the actual "to-be-encoded" size.

    // DWORD 1
    uint32_t   DW1_Log2MaxCUSize                                           : MOS_BITFIELD_RANGE( 0,  3);
    uint32_t   DW1_Log2MinCUSize                                           : MOS_BITFIELD_RANGE( 4,  7);
    uint32_t   DW1_Log2MaxTUSize                                           : MOS_BITFIELD_RANGE( 8, 11);
    uint32_t   DW1_Log2MinTUSize                                           : MOS_BITFIELD_RANGE(12, 15);
    uint32_t   DW1_MaxTransformDepthInter                                  : MOS_BITFIELD_RANGE(16, 19);
    uint32_t   DW1_MaxTransformDepthIntra                                  : MOS_BITFIELD_RANGE(20, 23);
    uint32_t   DW1_Log2ParallelMergeLevel                                  : MOS_BITFIELD_RANGE(24, 27);
    uint32_t   DW1_MaxNumIMESearchCenter                                   : MOS_BITFIELD_RANGE(28, 31);

    // DWORD 2
    uint32_t   DW2_TransquantBypassEnableFlag                              : MOS_BITFIELD_BIT(       0);
    uint32_t   DW2_CuQpDeltaEnabledFlag                                    : MOS_BITFIELD_BIT(       1);
    uint32_t   DW2_PCMEnabledFlag                                          : MOS_BITFIELD_BIT(       2);
    uint32_t   DW2_EnableCu64Check                                         : MOS_BITFIELD_BIT(       3);
    uint32_t   DW2_EnableIntra4x4PU                                        : MOS_BITFIELD_BIT(       4);
    uint32_t   DW2_ChromaSkipCheck                                         : MOS_BITFIELD_BIT(       5);
    uint32_t   DW2_EncTransformSimplify                                    : MOS_BITFIELD_RANGE( 6,  7);
    uint32_t   DW2_HMEFlag                                                 : MOS_BITFIELD_RANGE( 8,  9); // 2 bit flag for enabling hierarchical ME,    bit0  is for P slice and bit1 is for B slice
    uint32_t   DW2_HMECoarseShape                                          : MOS_BITFIELD_RANGE(10, 11);
    uint32_t   DW2_HMESubPelMode                                           : MOS_BITFIELD_RANGE(12, 13);
    uint32_t   DW2_SuperHME                                                : MOS_BITFIELD_BIT(      14);
    uint32_t   DW2_RegionsInSliceEnable                                    : MOS_BITFIELD_BIT(      15);
    uint32_t   DW2_EncTuDecisionMode                                       : MOS_BITFIELD_RANGE(16, 17);
    uint32_t   DW2_EncTuDecisionForAllQt                                   : MOS_BITFIELD_BIT(      18);
    uint32_t   DW2_CoefBitEstMode                                          : MOS_BITFIELD_BIT(      19);
    uint32_t   DW2_EncSkipDecisionMode                                     : MOS_BITFIELD_RANGE(20, 21);
    uint32_t   DW2_EncQtDecisionMode                                       : MOS_BITFIELD_BIT(      22);
    uint32_t   DW2_LCU32_EncRdDecisionModeForAllQt                         : MOS_BITFIELD_BIT(      23); //MBZ for LCU64
    uint32_t   DW2_QpType                                                  : MOS_BITFIELD_RANGE(24, 25);
    uint32_t   DW2_LCU64_Cu64SkipCheckOnly                                 : MOS_BITFIELD_BIT(      26); // Used by LCU64-B
    uint32_t   DW2_SICDynamicRunPathMode                                   : MOS_BITFIELD_RANGE(27, 28);
    uint32_t   DW2_Reserved_0                                              : MOS_BITFIELD_RANGE(29, 31); // MBZ

    // DWORD 3
    uint32_t   DW3_ActiveNumChildThreads_CU64                              : MOS_BITFIELD_RANGE( 0,  3); // only used by LCU64-B kernel, MBZ for LCU32-B kernel
    uint32_t   DW3_ActiveNumChildThreads_CU32_0                            : MOS_BITFIELD_RANGE( 4,  7); // only used by LCU64-B kernel, MBZ for LCU32-B kernel
    uint32_t   DW3_ActiveNumChildThreads_CU32_1                            : MOS_BITFIELD_RANGE( 8, 11); // only used by LCU64-B kernel, MBZ for LCU32-B kernel
    uint32_t   DW3_ActiveNumChildThreads_CU32_2                            : MOS_BITFIELD_RANGE(12, 15); // only used by LCU64-B kernel, MBZ for LCU32-B kernel
    uint32_t   DW3_ActiveNumChildThreads_CU32_3                            : MOS_BITFIELD_RANGE(16, 19); // only used by LCU64-B kernel, MBZ for LCU32-B kernel
    uint32_t   DW3_Reserved_0                                              : MOS_BITFIELD_RANGE(20, 23);
    uint32_t   DW3_SliceQp                                                 : MOS_BITFIELD_RANGE(24, 31);

    // DWORD 4
    uint32_t   DW4_SkipModeEn                                              : MOS_BITFIELD_BIT(       0); // This field specifies whether the skip mode checking is performed before the motion search.
    uint32_t   DW4_AdaptiveEn                                              : MOS_BITFIELD_BIT(       1); // This field defines whether adaptive searching is enabled for IME.
    uint32_t   DW4_Reserved_0                                              : MOS_BITFIELD_BIT(       2); // MBZ
    uint32_t   DW4_HEVCMinCUControl                                        : MOS_BITFIELD_RANGE( 3,  4); // These bits define the lowest CU split allowed.
    uint32_t   DW4_EarlyImeSuccessEn                                       : MOS_BITFIELD_BIT(       5); // This field specifies whether the Early Success may terminate the IME search.
    uint32_t   DW4_Reserved_1                                              : MOS_BITFIELD_BIT(       6); // MBZ
    uint32_t   DW4_IMECostCentersSel                                       : MOS_BITFIELD_BIT(       7); // This field determines the CostCenters that need to be used for motion vector costing purposes in IME.
    uint32_t   DW4_RefPixelOffset                                          : MOS_BITFIELD_RANGE( 8, 15); // The amount the reference pixels are offset.
    uint32_t   DW4_IMERefWindowSize                                        : MOS_BITFIELD_RANGE(16, 17);
    uint32_t   DW4_ResidualPredDatatypeCtrl                                : MOS_BITFIELD_BIT(      18);
    uint32_t   DW4_ResidualPredInterChromaCtrl                             : MOS_BITFIELD_BIT(      19); // This parameter indicates if RPM is generating luma predicted/residual samples or chroma predicted/residual samples
    uint32_t   DW4_ResidualPred16x16SelCtrl                                : MOS_BITFIELD_RANGE(20, 21); // Residual Prediction 16x16 Selection Control.
    uint32_t   DW4_Reserved_2                                              : MOS_BITFIELD_RANGE(22, 23); // MBZ
    uint32_t   DW4_EarlyImeStop                                            : MOS_BITFIELD_RANGE(24, 31); // Early IME Successful Stop Threshold

    // DWORD 5
    uint32_t   DW5_SubPelMode                                              : MOS_BITFIELD_RANGE( 0,  1); // This field defines the half/quarter pel modes.
    uint32_t   DW5_Reserved_0                                              : MOS_BITFIELD_RANGE( 2,  3); // MBZ
    uint32_t   DW5_InterSADMeasure                                         : MOS_BITFIELD_RANGE( 4,  5); // This field specifies distortion measure adjustments used for the motion search SAD comparison.
    uint32_t   DW5_IntraSADMeasure                                         : MOS_BITFIELD_RANGE( 6,  7); // This field specifies distortion measure adjustments used for the motion search SAD comparison.
    uint32_t   DW5_LenSP                                                   : MOS_BITFIELD_RANGE( 8, 15); // This field defines the maximum number of SUs per reference which are evaluated by the predetermined SUs.
    uint32_t   DW5_MaxNumSU                                                : MOS_BITFIELD_RANGE(16, 23); // This field defines the maximum number of SUs per reference including the predetermined SUs and the adaptively generated SUs.
    uint32_t   DW5_IntraPredictionMask                                     : MOS_BITFIELD_RANGE(24, 26); // This field specifies which Luma Intra partition is enabled/disabled for intra mode decision.
    uint32_t   DW5_RefIDCostMode                                           : MOS_BITFIELD_BIT(      27); // Selects the RefID costing mode.
    uint32_t   DW5_DisablePIntra                                           : MOS_BITFIELD_BIT(      28);
    uint32_t   DW5_TuBasedCostSetting                                      : MOS_BITFIELD_RANGE(29, 31);

    // DWORD 6
    uint32_t   DW6_Reserved_0                                              : MOS_BITFIELD_RANGE( 0, 31); // MBZ

    // DWORD 7
    uint32_t   DW7_SliceType                                               : MOS_BITFIELD_RANGE( 0,  1);
    uint32_t   DW7_TemporalMvpEnableFlag                                   : MOS_BITFIELD_BIT(       2);
    uint32_t   DW7_CollocatedFromL0Flag                                    : MOS_BITFIELD_BIT(       3); // Reference index of the picture that contains the collocated partitions.
    uint32_t   DW7_TheSameRefList                                          : MOS_BITFIELD_BIT(       4);
    uint32_t   DW7_IsLowDelay                                              : MOS_BITFIELD_BIT(       5); // Reserved for LCU64_CU32
    uint32_t   DW7_Reserved_0                                              : MOS_BITFIELD_RANGE( 6,  7);
    uint32_t   DW7_MaxNumMergeCand                                         : MOS_BITFIELD_RANGE( 8, 15); // Max number of merge candidates  allowed.
    uint32_t   DW7_NumRefIdxL0                                             : MOS_BITFIELD_RANGE(16, 23); // Actual number of reference frames for FWD prediction.
    uint32_t   DW7_NumRefIdxL1                                             : MOS_BITFIELD_RANGE(24, 31); // Actual number of reference frames for BWD prediction.

    // DWORD 8
    uint32_t   DW8_FwdPocNumber_L0_mTb_0                                   : MOS_BITFIELD_RANGE( 0,  7); // FWD POC Number for RefID 0 in L0
    uint32_t   DW8_BwdPocNumber_L1_mTb_0                                   : MOS_BITFIELD_RANGE( 8, 15); // BWD POC Number for RefID 0 in L1
    uint32_t   DW8_FwdPocNumber_L0_mTb_1                                   : MOS_BITFIELD_RANGE(16, 23); // FWD POC Number for RefID 1 in L0
    uint32_t   DW8_BwdPocNumber_L1_mTb_1                                   : MOS_BITFIELD_RANGE(24, 31); // BWD POC Number for RefID 1 in L1

    // DWORD 9
    uint32_t   DW9_FwdPocNumber_L0_mTb_2                                   : MOS_BITFIELD_RANGE( 0,  7); // FWD POC Number for RefID 2 in L0
    uint32_t   DW9_BwdPocNumber_L1_mTb_2                                   : MOS_BITFIELD_RANGE( 8, 15); // BWD POC Number for RefID 2 in L1
    uint32_t   DW9_FwdPocNumber_L0_mTb_3                                   : MOS_BITFIELD_RANGE(16, 23); // FWD POC Number for RefID 3 in L0
    uint32_t   DW9_BwdPocNumber_L1_mTb_3                                   : MOS_BITFIELD_RANGE(24, 31); // BWD POC Number for RefID 3 in L1

    // DWORD 10
    uint32_t   DW10_FwdPocNumber_L0_mTb_4                                  : MOS_BITFIELD_RANGE( 0,  7); // FWD POC Number for RefID 4 in L0
    uint32_t   DW10_BwdPocNumber_L1_mTb_4                                  : MOS_BITFIELD_RANGE( 8, 15); // BWD POC Number for RefID 4 in L1
    uint32_t   DW10_FwdPocNumber_L0_mTb_5                                  : MOS_BITFIELD_RANGE(16, 23); // FWD POC Number for RefID 5 in L0
    uint32_t   DW10_BwdPocNumber_L1_mTb_5                                  : MOS_BITFIELD_RANGE(24, 31); // BWD POC Number for RefID 5 in L1

    // DWORD 11
    uint32_t   DW11_FwdPocNumber_L0_mTb_6                                  : MOS_BITFIELD_RANGE( 0,  7); // FWD POC Number for RefID 6 in L0
    uint32_t   DW11_BwdPocNumber_L1_mTb_6                                  : MOS_BITFIELD_RANGE( 8, 15); // BWD POC Number for RefID 6 in L1
    uint32_t   DW11_FwdPocNumber_L0_mTb_7                                  : MOS_BITFIELD_RANGE(16, 23); // FWD POC Number for RefID 7 in L0
    uint32_t   DW11_BwdPocNumber_L1_mTb_7                                  : MOS_BITFIELD_RANGE(24, 31); // BWD POC Number for RefID 7 in L1

    // DWORD 12
    uint32_t   DW12_LongTermReferenceFlags_L0                              : MOS_BITFIELD_RANGE( 0, 15); // Bit0~Bit7 indicates if Ref0~Ref7 in L0 is long term reference.
    uint32_t   DW12_LongTermReferenceFlags_L1                              : MOS_BITFIELD_RANGE(16, 31); // Bit16~Bit23 indicates if Ref0~Ref7 in L1 is long term reference.

    // DWORD 13
    uint32_t   DW13_RefFrameHorizontalSize                                 : MOS_BITFIELD_RANGE( 0, 15);
    uint32_t   DW13_RefFrameVerticalSize                                   : MOS_BITFIELD_RANGE(16, 31);

    // DWORD 14
    uint32_t   DW14_KernelDebugDW                                          : MOS_BITFIELD_RANGE( 0, 31); // not used in release kernel, MBZ

    // DWORD 15
    uint32_t   DW15_ConcurrentGroupNum                                     : MOS_BITFIELD_RANGE( 0,  7);
    uint32_t   DW15_TotalThreadNumPerLCU                                   : MOS_BITFIELD_RANGE( 8, 15);
    uint32_t   DW15_NumRegions                                             : MOS_BITFIELD_RANGE(16, 23);
    uint32_t   DW15_Reserved_0                                             : MOS_BITFIELD_RANGE(24, 31);

    // DWORD 16
    uint32_t   DW16_BTI_CurrentPictureY                                    : MOS_BITFIELD_RANGE( 0, 31); // Source Pixel Y Surface index

    // DWORD 17
    uint32_t   DW17_BTI_EncCuRecordSurface                                 : MOS_BITFIELD_RANGE( 0, 31); // Each 32x32 LCU will need 32x16 size 2D window

    // DWORD 18
    union
    {
        uint32_t   DW18_BTI_LCU32_PAKObjectCommandSurface                  : MOS_BITFIELD_RANGE( 0, 31); // Output data for PAK Engine input, each LCU has 4 DWs (16 bytes)  PAK Object Command
        uint32_t   DW18_BTI_LCU64_SecEncCuRecordSurface                    : MOS_BITFIELD_RANGE( 0, 31);
    };

    // DWORD 19
    union
    {
        uint32_t   DW19_BTI_LCU32_PAKCURecordSurface                       : MOS_BITFIELD_RANGE( 0, 31); // Output CURecord data for PAK Engine input. Each LCU32x32 will output 512 bytes CU packet data.
        uint32_t   DW19_BTI_LCU64_PAKObjectCommandSurface                  : MOS_BITFIELD_RANGE( 0, 31); // Output data for PAK Engine input, each LCU has 4 DWs (16 bytes)  PAK Object Command
    };

    // DWORD 20
    union
    {
        uint32_t   DW20_BTI_LCU32_VMEIntra_InterPredictionSurface          : MOS_BITFIELD_RANGE( 0, 31); // Output CURecord data for PAK Engine input. Each LCU32x32 will output 512 bytes CU packet data.
        uint32_t   DW20_BTI_LCU64_PAKCURecordSurface                       : MOS_BITFIELD_RANGE( 0, 31); // Output CURecord data for PAK Engine input. Each LCU32x32 will output 512 bytes CU packet data.
    };

    // DWORD 21
    union
    {
        uint32_t   DW21_BTI_LCU32_CU16x16QpDataInputSurface                : MOS_BITFIELD_RANGE( 0, 31); // Each 16x16 block has one byte QP data. Each LCU32 has 4 bytes. Used only when  CuQpDeltaEnabledFlag=1
        uint32_t   DW21_BTI_LCU64_VMEIntra_InterPredictionSurface          : MOS_BITFIELD_RANGE( 0, 31); // Output CURecord data for PAK Engine input. Each LCU32x32 will output 512 bytes CU packet data.
    };

    // DWORD 22
    union
    {
        uint32_t   DW22_BTI_LCU32_HEVCEncConstantTableSurface              : MOS_BITFIELD_RANGE( 0, 31); // This surface contains all constants used by kernel. Data will be provided during kernel release.
        uint32_t   DW22_BTI_LCU64_CU16x16QpDataInputSurface                : MOS_BITFIELD_RANGE( 0, 31); // Each 16x16 block has one byte QP data. Each LCU32 has 4 bytes. Used only when  CuQpDeltaEnabledFlag=1
    };

    // DWORD 23
    union
    {
        uint32_t   DW23_BTI_LCU32_ColocatedCUMotionVectorDataSurface       : MOS_BITFIELD_RANGE( 0, 31); // Each CU 16x16 has 4 DWs ColMV data. Each LCU32 has 64 bytes. Used only when  TemporalMvpEnableFlag=1, temporal predicton is enabled.
        uint32_t   DW23_BTI_LCU64_CU32_HEVCEncConstantTableSurface         : MOS_BITFIELD_RANGE( 0, 31); // This surface contains all constants used by kernel. Data will be provided during kernel release.
    };

    // DWORD 24
    union
    {
        uint32_t   DW24_BTI_LCU32_HmeMotionPredictorDataSurface            : MOS_BITFIELD_RANGE( 0, 31); // Each 32x32 block has 1 pair of  FWD  MV and 1 pair of BDW MV, total 8 bytes of data.
        uint32_t   DW24_BTI_LCU64_ColocatedCUMotionVectorDataSurface       : MOS_BITFIELD_RANGE( 0, 31); // Each CU 16x16 has 4 DWs ColMV data. Each LCU32 has 64 bytes. Used only when  TemporalMvpEnableFlag=1, temporal predicton is enabled.
    };

    // DWORD 25
    union
    {
        uint32_t   DW25_BTI_LCU32_LcuLevelDataInputSurface                 : MOS_BITFIELD_RANGE( 0, 31); // Each LCU block has one 32 bytes  data, including SliceQP and slice astart/end address
        uint32_t   DW25_BTI_LCU64_HmeMotionPredictorDataSurface            : MOS_BITFIELD_RANGE( 0, 31); // Each 32x32 block has 1 pair of  FWD  MV and 1 pair of BDW MV, total 8 bytes of data.
    };

    // DWORD 26
    union
    {
        uint32_t   DW26_BTI_LCU32_LcuEncodingScratchSurface                : MOS_BITFIELD_RANGE( 0, 31); // Each LCU32 block has about 9k byte scratch space to store temporary data
        uint32_t   DW26_BTI_LCU64_LcuLevelDataInputSurface                 : MOS_BITFIELD_RANGE( 0, 31); // Each LCU block has one 32 bytes  data, including SliceQP and slice astart/end address
    };

    // DWORD 27
    union
    {
        uint32_t   DW27_BTI_LCU32_ConcurrentThreadGroupDataSurface         : MOS_BITFIELD_RANGE( 0, 31); // Concurrent Thread Group Data Surface **LCU32 kernel**
        uint32_t   DW27_BTI_LCU64_CU32_LcuEncodingScratchSurface           : MOS_BITFIELD_RANGE( 0, 31); // Each LCU32 block has about 9k byte scratch space to store temporary data
    };

    // DWORD 28
    union
    {
        uint32_t   DW28_BTI_LCU32_BrcCombinedEncParameterSurface           : MOS_BITFIELD_RANGE( 0, 31); // Brc Combined Enc Parameter Surface **LCU32 kernel**
        uint32_t   DW28_BTI_LCU64_64x64_DistortionSurface                  : MOS_BITFIELD_RANGE( 0, 31); // Each LCU64_CU32 block has about 9k byte scratch space to store temporary data **LCU64_CU32 kernel**
    };

    // DWORD 29
    union
    {
        uint32_t   DW29_BTI_LCU32_JobQueueScratchBufferSurface             : MOS_BITFIELD_RANGE( 0, 31); // Surface for Multi-thread implementation **LCU32 kernel**
        uint32_t   DW29_BTI_LCU64_ConcurrentThreadGroupDataSurface         : MOS_BITFIELD_RANGE( 0, 31); // Concurrent Thread Group Data Surface **LCU64_CU32 kernel**
    };

    //DWORD 30
    union
    {
        uint32_t   DW30_BTI_LCU32_CuSplitDataSurface                       : MOS_BITFIELD_RANGE( 0, 31); // Reserved for debug kernel. Not available for released kernel. **LCU32 kernel**
        uint32_t   DW30_BTI_LCU64_BrcCombinedEncParameterSurface           : MOS_BITFIELD_RANGE( 0, 31); // Brc Combined Enc Parameter Surface. **LCU64_CU32 kernel**
    };

    //DWORD 31
    union
    {
        uint32_t   DW31_BTI_LCU32_ResidualDataScratchSurface               : MOS_BITFIELD_RANGE( 0, 31);
        uint32_t   DW31_BTI_LCU64_CU32_JobQueue1DBufferSurface             : MOS_BITFIELD_RANGE( 0, 31); // Surface for Multi-thread implementation **LCU64_CU32 kernel**
    };

    //DWORD 32
    union
    {
        uint32_t   DW32_BTI_LCU32_DebugSurface                             : MOS_BITFIELD_RANGE( 0, 31);
        uint32_t   DW32_BTI_LCU64_CU32_JobQueue2DBufferSurface             : MOS_BITFIELD_RANGE( 0, 31);
    };

    // DWORD 33
    uint32_t   DW33_BTI_LCU64_CU32_ResidualDataScratchSurface              : MOS_BITFIELD_RANGE( 0, 31);

    // DWORD 34
    uint32_t   DW34_BTI_LCU64_CuSplitSurface                               : MOS_BITFIELD_RANGE( 0, 31);

    // DWORD 35
    uint32_t   DW35_BTI_LCU64_CurrentPictureY2xDS                          : MOS_BITFIELD_RANGE( 0, 31); // Source Pixel Y Downscaled by 2 Surface index

    // DWORD 36
    uint32_t   DW36_BTI_LCU64_IntermediateCuRecordSurface                  : MOS_BITFIELD_RANGE( 0, 31); // Each 64x64 LCU will need 32x16x4 size 2D window

    // DWORD 37
    uint32_t   DW37_BTI_Lcu64_ConstantDataLutSurface                       : MOS_BITFIELD_RANGE( 0, 31);

    // DWORD 38
    uint32_t   DW38_BTI_LCU64_LcuDataStorageSurface                        : MOS_BITFIELD_RANGE( 0, 31);

    // DWORD 39
    uint32_t   DW39_BTI_LCU64_VmeInterPredictionSurface2xDS                : MOS_BITFIELD_RANGE( 0, 31); // This is current downscaled by 2 pixel surface accessed by the VME hardware

    // DWORD 40
    uint32_t   DW40_BTI_LCU64_JobQueue1DBufferSurface                      : MOS_BITFIELD_RANGE( 0, 31); // Surface for Multi-thread implementation

    // DWORD 41
    uint32_t   DW41_BTI_LCU64_JobQueue2DBufferSurface                      : MOS_BITFIELD_RANGE( 0, 31); // Surface for Multi-thread implementation

    // DWORD 42
    uint32_t   DW42_BTI_LCU64_ResidualDataScratchSurface                   : MOS_BITFIELD_RANGE(0, 31);

    // DWORD 43
    uint32_t   DW43_BTI_LCU64_DebugFeatureSurface                          : MOS_BITFIELD_RANGE( 0, 31);
};

C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(CODECHAL_ENC_HEVC_MBENC_B_CURBE_G10)) == 44);

const CODECHAL_ENC_HEVC_ME_CURBE_G10 CodechalEncHevcStateG10::m_meCurbeInit = {
    0,          //DW0_RoundedFrameWidthInMvUnitsfor4X
    0,          //DW0_RoundedFrameHeightInMvUnitsfor4X
    0,          //DW1_Reserved_0
    0,          //DW1_MvCostScaleFactor
    0,          //DW1_Reserved_1
    0,          //DW2_Reserved_0
    0x3,        //DW2_SubPelMode
    1,          //DW2_BmeDisableFbr
    0,          //DW2_Reserved_1
    0x2,        //DW2_InterSADMeasureAdjustment
    0,          //DW2_Reserved_2
    0,          //DW3_Reserved_0
    1,          //DW3_AdaptiveSearchEnable
    0,          //DW3_Reserved_1
    0x1,        //DW3_ImeRefWindowSize
    0,          //DW3_Reserved_2
    0,          //DW4_Reserved_0
    0x1,        //DW4_QuarterQuadTreeCandidate -- Coarseshape 16x16
    0,          //DW4_Reserved_1
    32,         //DW4_BidirectionalWeight
    0,          //DW4_Reserved_2
    0x3F,       //DW5_LenSP
    0x3F,       //DW5_MaxNumSU
    0x4,        //DW5_StartCenter0_X
    0x4,        //DW5_StartCenter0_Y
    0,          //DW5_Reserved_0
    0,          //DW6_Reserved_0
    1,          //DW6_SliceType
    0,          //DW6_HmeStage
    0,          //DW6_NumRefL0
    0,          //DW6_NumRefL1
    0,          //DW6_Reserved_1
    0,          //DW7_RoundedFrameWidthInMvUnitsFor16x
    0,          //DW7_RoundedFrameHeightInMvUnitsfor16X
    0x0101F00F, //DW8_ImeSearchPath_0_3
    0x0F0F1010, //DW9_ImeSearchPath_4_7
    0xF0F0F00F, //DW10_ImeSearchPath_8_11
    0x01010101, //DW11_ImeSearchPath_12_15
    0x10101010, //DW12_ImeSearchPath_16_19
    0x0F0F0F0F, //DW13_ImeSearchPath_20_23
    0xF0F0F00F, //DW14_ImeSearchPath_24_27
    0x0101F0F0, //DW15_ImeSearchPath_28_31
    0x01010101, //DW16_ImeSearchPath_32_35
    0x10101010, //DW17_ImeSearchPath_36_39
    0x0F0F1010, //DW18_ImeSearchPath_40_43
    0x0F0F0F0F, //DW19_ImeSearchPath_44_47
    0xF0F0F00F, //DW20_ImeSearchPath_48_51
    0xF0F0F0F0, //DW21_ImeSearchPath_52_55
    0x01010101, //DW22_ImeSearchPath_56_59
    0x01010101, //DW23_ImeSearchPath_60_63
    0,          //DW24_Reserved_0
    1,          //DW24_CodingUnitSize -- CoarseShape = 16x16
    0,          //DW24_Reserved_1
    0,          //DW24_CodingUnitPartitionMode
    1,          //DW24_CodingUnitPredictionMode
    0,          //DW24_Reserved_2
    0,          //DW25_FrameWidthInSamplesOfCurrentStage
    0,          //DW25_FrameHeightInSamplesOfCurrentStage
    0,          //DW26_Intra8x8ModeMask
    0,          //DW26_Reserved_0
    0,          //DW26_Intra16x16ModeMask
    0,          //DW26_Reserved_1
    0,          //DW27_Intra32x32ModeMask
    0,          //DW27_IntraChromaModeMask
    1,          //DW27_IntraComputeType
    0,          //DW27_Reserved_0
    0,          //DW28_Reserved_0
    36,         //DW28_PenaltyIntra32x32NonDC
    12,         //DW28_PenaltyIntra16x16NonDC
    4,          //DW28_PenaltyIntra8x8NonDC
    0,          //DW29_Mode0Cost
    0,          //DW29_Mode1Cost
    0,          //DW29_Mode2Cost
    0,          //DW29_Mode3Cost
    13,         //DW30_Mode4Cost
    9,          //DW30_Mode5Cost
    13,         //DW30_Mode6Cost
    3,          //DW30_Mode7Cost
    9,          //DW31_Mode8Cost
    0,          //DW31_Mode9Cost
    0,          //DW31_Reserved_0
    0,          //DW31_ChromaIntraModeCost
    0,          //DW32_Reserved_0
    0x3F,       //DW32_SicIntraNeighborAvailableFlag
    0,          //DW32_Reserved_1
    0x2,        //DW32_SicInterSadMeasure
    0x2,        //DW32_SicIntraSadMeasure
    0,          //DW32_Reserved_2
    3,          //DW33_SicLog2MinCuSize
    0,          //DW33_Reserved_0
    0,          //DW33_SicAcOnlyHaar
    0,          //DW33_Reserved_1
    0,          //DW33_SicHevcQuarterQuadtree
    0,          //DW33_Reserved_2
    0xFFFF,     //DW34_BTI_HmeOutputMvDataSurface
    0xFFFF,     //DW35_BTI_16xInputMvDataSurface
    0xFFFF,     //DW36_BTI_4xOutputDistortionSurface
    0xFFFF,     //DW37_BTI_VmeSurfaceIndex
    0xFFFF,     //DW38_BTI_4xDsSurface
    0xFFFF,     //DW39_BTI_BrcDistortionSurface
    0
};

const CODECHAL_ENC_HEVC_BRC_INITRESET_CURBE_G10 CodechalEncHevcStateG10::m_brcInitResetCurbeInit = {
    0,          //DW0_ProfileLevelMaxFrame
    0,          //DW1_InitBufFull
    0,          //DW2_BufSize
    0,          //DW3_TargetBitRate
    0,          //DW4_MaximumBitRate
    0,          //DW5_MinimumBitRate
    0,          //DW6_FrameRateM
    1,          //DW7_FrameRateD
    0,          //DW8_BRCFlag
    0,          //DW8_BRC_Param_A
    0,          //DW9_BRC_Param_B
    0,          //DW9_FrameWidth
    0,          //DW10_FrameHeight
    0,          //DW10_AVBRAccuracy
    0,          //DW11_AVBRConvergence
    1,          //DW11_MinimumQP
    51,         //DW12_MaximumQP
    0,          //DW12_NumberSlice
    0,          //DW13_Reserved_0
    0,          //DW13_BRC_Param_C
    0,          //DW14_BRC_Param_D
    0,          //DW14_MaxBRCLevel
    0,          //DW15_LongTermInterval
    0,          //DW15_Reserved_0
    40,         //DW16_InstantRateThreshold0_Pframe
    60,         //DW16_InstantRateThreshold1_Pframe
    80,         //DW16_InstantRateThreshold2_Pframe
    120,        //DW16_InstantRateThreshold3_Pframe
    35,         //DW17_InstantRateThreshold0_Bframe
    60,         //DW17_InstantRateThreshold1_Bframe
    80,         //DW17_InstantRateThreshold2_Bframe
    120,        //DW17_InstantRateThreshold3_Bframe
    40,         //DW18_InstantRateThreshold0_Iframe
    60,         //DW18_InstantRateThreshold1_Iframe
    90,         //DW18_InstantRateThreshold2_Iframe
    115,        //DW18_InstantRateThreshold3_Iframe
    0,          //DW19_DeviationThreshold0_PBframe
    0,          //DW19_DeviationThreshold1_PBframe
    0,          //DW19_DeviationThreshold2_PBframe
    0,          //DW19_DeviationThreshold3_PBframe
    0,          //DW20_DeviationThreshold4_PBframe
    0,          //DW20_DeviationThreshold5_PBframe
    0,          //DW20_DeviationThreshold6_PBframe
    0,          //DW20_DeviationThreshold7_PBframe
    0,          //DW21_DeviationThreshold0_VBRcontrol
    0,          //DW21_DeviationThreshold1_VBRcontrol
    0,          //DW21_DeviationThreshold2_VBRcontrol
    0,          //DW21_DeviationThreshold3_VBRcontrol
    0,          //DW22_DeviationThreshold4_VBRcontrol
    0,          //DW22_DeviationThreshold5_VBRcontrol
    0,          //DW22_DeviationThreshold6_VBRcontrol
    0,          //DW22_DeviationThreshold7_VBRcontrol
    0,          //DW23_DeviationThreshold0_Iframe
    0,          //DW23_DeviationThreshold1_Iframe
    0,          //DW23_DeviationThreshold2_Iframe
    0,          //DW23_DeviationThreshold3_Iframe
    0,          //DW24_DeviationThreshold4_Iframe
    0,          //DW24_DeviationThreshold5_Iframe
    0,          //DW24_DeviationThreshold6_Iframe
    0,          //DW24_DeviationThreshold7_Iframe
    0,          //DW25_ACQPBuffer
    0,          //DW25_IntraSADTransform
    5,          //DW25_Log2MaxCuSize
    30,         //DW25_SlidingWindowSize
    0,          //DW26_BGOPSize
    0,          //DW26_Reserved_0
    0,          //DW27_Reserved_0
    0,          //DW28_Reserved_0
    0,          //DW29_Reserved_0
    0,          //DW30_Reserved_0
    0,          //DW31_Reserved_0
};

const CODECHAL_ENC_HEVC_BRC_UPDATE_CURBE_G10 CodechalEncHevcStateG10::m_brcUpdateCurbeInit = {
    0,          // DW0_TargetSize
    0,          // DW1_FrameNumber
    0,          // DW2_PictureHeaderSize
    10,         // DW3_StartGAdjFrame0
    50,         // DW3_StartGAdjFrame1
    100,        // DW4_StartGAdjFrame2
    150,        // DW4_StartGAdjFrame3
    0,          // DW5_TargetSize_Flag
    0,          // DW5_Reserved_0
    4,          // DW5_MaxNumPAKs
    2,          // DW5_CurrFrameBrcLevel
    0,          // DW6_NumSkippedFrames
    0,          // DW6_CqpValue
    0,          // DW6_ROIEnable
    0,          // DW6_BRCROIEnable
    1,          // DW6_LCUQPAverageEnable
    0,          // DW6_Reserved1
    0,          // DW6_SlidingWindowEnable
    0,          // DW6_Reserved2
    0,          // DW6_RoiRatio
    0,          // DW7_Reserved_0
    1,          // DW8_StartGlobalAdjustMult0
    1,          // DW8_StartGlobalAdjustMult1
    3,          // DW8_StartGlobalAdjustMult2
    2,          // DW8_StartGlobalAdjustMult3
    1,          // DW9_StartGlobalAdjustMult4
    40,         // DW9_StartGlobalAdjustDivd0
    5,          // DW9_StartGlobalAdjustDivd1
    5,          // DW9_StartGlobalAdjustDivd2
    3,          // DW10_StartGlobalAdjustDivd3
    1,          // DW10_StartGlobalAdjustDivd4
    7,          // DW10_QPThreshold0
    18,         // DW10_QPThreshold1
    25,         // DW11_QPThreshold2
    37,         // DW11_QPThreshold3
    40,         // DW11_gRateRatioThreshold0
    75,         // DW11_gRateRatioThreshold1
    97,         // DW12_gRateRatioThreshold2
    103,        // DW12_gRateRatioThreshold3
    125,        // DW12_gRateRatioThreshold4
    160,        // DW12_gRateRatioThreshold5
    MOS_BITFIELD_VALUE((uint32_t)-3, 8), // DW13_gRateRatioThreshold6
    MOS_BITFIELD_VALUE((uint32_t)-2, 8), // DW13_gRateRatioThreshold7
    MOS_BITFIELD_VALUE((uint32_t)-1, 8), // DW13_gRateRatioThreshold8
    0,          // DW13_gRateRatioThreshold9
    1,          // DW14_gRateRatioThreshold10
    2,          // DW14_gRateRatioThreshold11
    3,          // DW14_gRateRatioThreshold12
    4,          // DW14_ParallelMode
    0,          // DW15_SizeOfSkippedFrames
};

const CODECHAL_ENC_HEVC_MBENC_I_CURBE_G10 CodechalEncHevcStateG10::m_mbencICurbeInit = {
    0,      //DW0_FrameWidthInSamples
    0,      //DW0_FrameHeightInSamples
    0,      //DW1_Reserved_0
    36,     //DW1_PenaltyForIntra32x32NonDCPredMode
    12,     //DW1_PenaltyForIntra16x16NonDCPredMode
    4,      //DW1_PenaltyForIntra8x8NonDCPredMode
    0,      //DW2_Reserved_0
    2,      //DW2_IntraSADMeasureAdj
    0,      //DW2_IntraPrediction
    0,      //DW2_Reserved_1
    43,     //DW3_ModeCost_0
    60,     //DW3_ModeCost_1
    60,     //DW3_ModeCost_2
    60,     //DW3_ModeCost_3
    0,      //DW4_ModeCost_4
    0,      //DW4_ModeCost_5
    0,      //DW4_ModeCost_6
    0,      //DW4_ModeCost_7
    0,      //DW5_ModeCost_8
    0,      //DW5_ModeCost_9
    0,      //DW5_RefIDCost
    25,     //DW5_ChromaIntraModeCost
    5,      //DW6_Log2MaxCUSize
    3,      //DW6_Log2MinCUSize
    5,      //DW6_Log2MaxTUSize
    2,      //DW6_Log2MinTUSize
    1,      //DW6_MaxTransformDepthIntra
    1,      //DW6_TuSplitFlag
    0,      //DW6_TuBasedCostSetting
    0,      //DW6_Reserved_0
    0,      //DW7_ConcurrentGroupNum .. 0 treated same as 1
    1,      //DW7_EncTuDecisionMode
    0,      //DW7_Reserved_0
    27,     //DW7_SliceQP
    40960,  //DW8_Lambda_Rd
    3238,   //DW9_Lambda_Md
    0,      //DW9_Reserved_0
    534,    //DW10_IntraTuDThres
    2,      //DW11_SliceType
    0,      //DW11_QPType
    0,      //DW11_CheckPcmModeFlag
    0,      //DW11_EnableIntra4x4PU
    1,      //DW11_EncQtDecisionMode
    0,      //DW11_Reserved_0
    4700,   //DW12_PCM_8x8_SAD_Threshold
    0,      //DW12_Reserved_0
    0,      //DW13_Reserved_0
    0,      //DW14_Reserved_0
    0,      //DW15_Reserved_0
    0xFFFF, //DW16_BTI_VmeIntraPredictionSurface
    0xFFFF, //DW17_BTI_CurrentPictureY
    0xFFFF, //DW18_BTI_EncCuRecordSurface
    0xFFFF, //DW19_BTI_PakObjectCommandSurface
    0xFFFF, //DW20_BTI_CuPacketForPakSurface
    0xFFFF, //DW21_BTI_InternalScratchSurface
    0xFFFF, //DW22_BTI_CuBasedQpSurface
    0xFFFF, //DW23_BTI_ConstantDataLutSurface
    0xFFFF, //DW24_BTI_LcuLevelDataInputSurface
    0xFFFF, //DW25_BTI_ConcurrentThreadGroupDataSurface
    0xFFFF, //DW26_BTI_BrcCombinedEncParameterSurface
    0xFFFF, //DW27_BTI_CuSplitSurface
    0xFFFF, //DW28_BTI_DebugSurface
};

const CODECHAL_ENC_HEVC_MBENC_B_CURBE_G10 CodechalEncHevcStateG10::m_mbencBCurbeInit = {
    0,      //DW0_FrameWidthInSamples
    0,      //DW0_FrameHeightInSamples
    5,      //DW1_Log2MaxCUSize
    3,      //DW1_Log2MinCUSize
    5,      //DW1_Log2MaxTUSize
    2,      //DW1_Log2MinTUSize
    0,      //DW1_MaxTransformDepthInter
    0,      //DW1_MaxTransformDepthIntra
    2,      //DW1_Log2ParallelMergeLevel
    6,      //DW1_MaxNumIMESearchCenter
    0,      //DW2_TransquantBypassEnableFlag
    0,      //DW2_CuQpDeltaEnabledFlag
    0,      //DW2_PCMEnabledFlag
    0,      //DW2_EnableCu64Check
    0,      //DW2_EnableIntra4x4PU
    0,      //DW2_ChromaSkipCheck
    0,      //DW2_EncTransformSimplify
    0,      //DW2_HMEFlag
    0,      //DW2_HMECoarseShape
    0,      //DW2_HMESubPelMode
    0,      //DW2_SuperHME
    0,      //DW2_RegionsInSliceEnable
    0,      //DW2_EncTuDecisionMode
    0,      //DW2_EncTuDecisionForAllQt
    0,      //DW2_CoefBitEstMode
    0,      //DW2_EncSkipDecisionMode
    0,      //DW2_EncQtDecisionMode
    0,      //DW2_LCU32_EncRdDecisionModeForAllQt
    0,      //DW2_QpType
    0,      //DW2_LCU64_Cu64SkipCheckOnly
    0,      //DW2_SICDynamicRunPathMode
    0,      //DW2_Reserved_0
    0,      //DW3_ActiveNumChildThreads_CU64
    0,      //DW3_ActiveNumChildThreads_CU32_0
    0,      //DW3_ActiveNumChildThreads_CU32_1
    0,      //DW3_ActiveNumChildThreads_CU32_2
    0,      //DW3_ActiveNumChildThreads_CU32_3
    0,      //DW3_Reserved_0
    0,      //DW3_SliceQp
    1,      //DW4_SkipModeEn
    1,      //DW4_AdaptiveEn
    0,      //DW4_Reserved_0
    0,      //DW4_HEVCMinCUControl
    0,      //DW4_EarlyImeSuccessEn
    0,      //DW4_Reserved_1
    0,      //DW4_IMECostCentersSel
    0,      //DW4_RefPixelOffset
    1,      //DW4_IMERefWindowSize
    0,      //DW4_ResidualPredDatatypeCtrl
    0,      //DW4_ResidualPredInterChromaCtrl
    0,      //DW4_ResidualPred16x16SelCtrl
    0,      //DW4_Reserved_2
    0,      //DW4_EarlyImeStop
    0x3,    //DW5_SubPelMode
    0,      //DW5_Reserved_0
    2,      //DW5_InterSADMeasure
    2,      //DW5_IntraSADMeasure
    63,     //DW5_LenSP
    63,     //DW5_MaxNumSU
    0,      //DW5_IntraPredictionMask
    1,      //DW5_RefIDCostMode
    0,      //DW5_DisablePIntra
    0,      //DW5_TuBasedCostSetting
    0,      //DW6_Reserved_0
    0,      //DW7_SliceType
    0,      //DW7_TemporalMvpEnableFlag
    0,      //DW7_CollocatedFromL0Flag
    0,      //DW7_TheSameRefList
    0,      //DW7_IsLowDelay
    0,      //DW7_Reserved_0
    4,      //DW7_MaxNumMergeCand
    0,      //DW7_NumRefIdxL0
    0,      //DW7_NumRefIdxL1
    0,      //DW8_FwdPocNumber_L0_mTb_0
    0,      //DW8_BwdPocNumber_L1_mTb_0
    0,      //DW8_FwdPocNumber_L0_mTb_1
    0,      //DW8_BwdPocNumber_L1_mTb_1
    0,      //DW9_FwdPocNumber_L0_mTb_2
    0,      //DW9_BwdPocNumber_L1_mTb_2
    0,      //DW9_FwdPocNumber_L0_mTb_3
    0,      //DW9_BwdPocNumber_L1_mTb_3
    0,      //DW10_FwdPocNumber_L0_mTb_4
    0,      //DW10_BwdPocNumber_L1_mTb_4
    0,      //DW10_FwdPocNumber_L0_mTb_5
    0,      //DW10_BwdPocNumber_L1_mTb_5
    0,      //DW11_FwdPocNumber_L0_mTb_6
    0,      //DW11_BwdPocNumber_L1_mTb_6
    0,      //DW11_FwdPocNumber_L0_mTb_7
    0,      //DW11_BwdPocNumber_L1_mTb_7
    0,      //DW12_LongTermReferenceFlags_L0
    0,      //DW12_LongTermReferenceFlags_L1
    0,      //DW13_RefFrameVerticalSize
    0,      //DW13_RefFrameHorizontalSize
    0,      //DW14_KernelDebugDW
    0,      //DW15_ConcurrentGroupNum. 0 is treated same as 1.
    8,      //DW15_TotalThreadNumPerLCU
    0,      //DW15_NumRegions
    0,      //DW15_Reserved_0
    0xFFFF, //DW16_BTI_CurrentPictureY
    0xFFFF, //DW17_BTI_EncCuRecordSurface
    { 0xFFFF }, //DW18_BTI_PAKObjectCommandSurface
    { 0xFFFF }, //DW19_BTI_PAKCURecordSurface
    { 0xFFFF }, //DW20_BTI_VMEIntra_InterPredictionSurface
    { 0xFFFF }, //DW21_BTI_CU16x16QpDataInputSurface
    { 0xFFFF }, //DW22_BTI_LCU32_HEVCEncConstantTableSurface
    { 0xFFFF }, //DW23_BTI_ColocatedCUMotionVectorDataSurface
    { 0xFFFF }, //DW24_BTI_HmeMotionPredictorDataSurface
    { 0xFFFF }, //DW25_BTI_LcuLevelDataInputSurface
    { 0xFFFF }, //DW26_BTI_LCU32_LcuEncodingScratchSurface
    { 0xFFFF }, //DW27_BTI_LCU32_ConcurrentThreadGroupDataSurface / DW27_BTI_LCU64_64x64_DistortionSurface
    { 0xFFFF }, //DW28_BTI_LCU32_BrcCombinedEncParameterSurface / DW28_BTI_LCU64_ConcurrentThreadGroupDataSurface
    { 0xFFFF }, //DW29_BTI_LCU32_JobQueueScratchBufferSurface / DW29_BTI_LCU64_BrcCombinedEncParameterSurface
    { 0xFFFF }, //DW30_BTI_LCU32_CuSplitDataSurface / DW30_BTI_LCU64_CU32_JobQueueScratchBufferSurface
    { 0xFFFF }, //DW31_BTI_LCU32_ResidualDataScratchSurface
    { 0xFFFF }, //DW32_BTI_LCU32_DebugSurface / DW32_BIT_LCU64_CuSplitSurface
    0xFFFF, //DW33_BTI_LCU64_CurrentPictureY2xDS
    0xFFFF, //DW34_BTI_LCU64_IntermediateCuRecordSurface
    0xFFFF, //DW35_BTI_Lcu64_ConstantDataLutSurface
    0xFFFF, //DW36_BTI_LCU64_LcuDataStorageSurface
    0xFFFF, //DW37_BTI_LCU64_VmeInterPredictionSurface2xDS
    0xFFFF, //DW38_BTI_LCU64_JobQueueScratchBufferSurface
    0xFFFF, //DW39_BTI_LCU64_ResidualDataScratchSurface
    0xFFFF, //DW40_BTI_LCU64_DebugFeatureSurface
    0xFFFF, //DW41
    0xFFFF, //DW42
    0xFFFF  //DW43
};
//! \endcond

MOS_STATUS CodechalEncHevcStateG10::SetSequenceStructs()
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncHevcState::SetSequenceStructs());

    m_cqpEnabled = (m_hevcSeqParams->RateControlMethod == RATECONTROL_CQP);

    // threads per LCU
    m_totalNumThreadsPerLcu = m_tuSettings[TotalThreadNumPerLCUTuParam][((m_hevcSeqParams->TargetUsage + 1) >> 2) % 3];

    // Gen10 optimal wave-front no. and threads assigned wrt TU and Pic width for each LCU type
    if (m_numRegionsInSlice > 1)  //check if wf is enabled
    {
        if (m_isMaxLcu64)
        {
            m_numRegionsInSlice = (m_frameWidth >= 640) ? m_tuSettings[NumRegionLCU64][((m_hevcSeqParams->TargetUsage + 1) >> 2) % 3] : 2;
        }

        else
        {
            if (m_encode4KSequence)
            {
                m_totalNumThreadsPerLcu = m_tuSettings[TotalThreadNumPerLCUTuParamFor4KOnly][((m_hevcSeqParams->TargetUsage + 1) >> 2) % 3];
            }
        }
     }

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG10::CalcScaledDimensions()
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    // 2x Scaling WxH
    m_downscaledWidth2x                  =
        CODECHAL_GET_2xDS_SIZE_32ALIGNED(m_frameWidth);
    m_downscaledHeight2x                 =
        CODECHAL_GET_2xDS_SIZE_32ALIGNED(m_frameHeight);

    // HME Scaling WxH
    m_downscaledWidth4x                   =
        CODECHAL_GET_4xDS_SIZE_32ALIGNED(m_frameWidth);
    m_downscaledHeight4x                  =
        CODECHAL_GET_4xDS_SIZE_32ALIGNED(m_frameHeight);
    m_downscaledWidthInMb4x               =
        CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_downscaledWidth4x);
    m_downscaledHeightInMb4x              =
        CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_downscaledHeight4x);

    // SuperHME Scaling WxH
    m_downscaledWidth16x                  =
        CODECHAL_GET_4xDS_SIZE_32ALIGNED(m_downscaledWidth4x);
    m_downscaledHeight16x                 =
        CODECHAL_GET_4xDS_SIZE_32ALIGNED(m_downscaledHeight4x);
    m_downscaledWidthInMb16x              =
        CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_downscaledWidth16x);
    m_downscaledHeightInMb16x             =
        CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_downscaledHeight16x);

    // UltraHME Scaling WxH
    m_downscaledWidth32x                  =
        CODECHAL_GET_2xDS_SIZE_32ALIGNED(m_downscaledWidth16x);
    m_downscaledHeight32x                 =
        CODECHAL_GET_2xDS_SIZE_32ALIGNED(m_downscaledHeight16x);
    m_downscaledWidthInMb32x              =
        CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_downscaledWidth32x);
    m_downscaledHeightInMb32x             =
        CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_downscaledHeight32x);

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG10::SetupBrcConstantTable(PMOS_SURFACE brcConstantData)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(brcConstantData);

    MOS_LOCK_PARAMS lockFlags;
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.WriteOnly = 1;

    uint8_t* data = (uint8_t*)m_osInterface->pfnLockResource(m_osInterface, &brcConstantData->OsResource, &lockFlags);
    CODECHAL_ENCODE_CHK_NULL_RETURN(data);

    uint32_t size = brcConstantData->dwHeight * brcConstantData->dwWidth;
    MOS_SecureMemcpy(data, size, g_cInit_HEVC_BRC_QP_ADJUST, sizeof(g_cInit_HEVC_BRC_QP_ADJUST));
    data += sizeof(g_cInit_HEVC_BRC_QP_ADJUST);
    size -= sizeof(g_cInit_HEVC_BRC_QP_ADJUST);

    //lambda and mode cost
    if (m_isMaxLcu64)
    {
        MOS_SecureMemcpy(data, size, m_brcLcu64x64LambdaModeCost, sizeof(m_brcLcu64x64LambdaModeCost));
    }
    else
    {
        MOS_SecureMemcpy(data, size, m_brcLcu32x32LambdaModeCost, sizeof(m_brcLcu32x32LambdaModeCost));
    }

    const uint32_t sizeLambaModeCostTable = m_brcLambdaModeCostTableSize;
    data += sizeLambaModeCostTable;
    size -= sizeLambaModeCostTable;

    m_osInterface->pfnUnlockResource(m_osInterface, &brcConstantData->OsResource);

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG10::AllocateEncResourcesLCU64()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    uint32_t width = 0, height = 0, size = 0;
    // Surfaces used only by LCU64 B-kernel
    // Intermediate CU Record Surface for LCU64
    if (Mos_ResourceIsNull(&m_intermediateCuRecordSurfaceLcu64B.OsResource))
    {
        width  = m_widthAlignedMaxLcu;
        height = m_heightAlignedMaxLcu >> 1;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer2D(
            &m_intermediateCuRecordSurfaceLcu64B,
            width,
            height,
            "Intermediate CU record Surface For Lcu64 B-kernel"));
    }

    // Scratch Surface for Internal use
    if( Mos_ResourceIsNull(&m_lcuEncodingScratchSurfaceLcu64B.sResource))
    {
        size = 13312 * ((m_widthAlignedMaxLcu >> 6) << 1) * ((m_heightAlignedMaxLcu >> 6) << 1);

        CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer(
            &m_lcuEncodingScratchSurfaceLcu64B,
            size,
            "Lcu 64 B Encoding Scratch Surface"));
    }

    // Enc constant table for B LCU64
    if( Mos_ResourceIsNull(&m_encConstantTableForLcu64B.sResource))
    {
        size = m_encBConstantDataLutLcu64Size;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer(
            &m_encConstantTableForLcu64B,
            size,
            "Enc Constant Table Surface For B LCU64"));

        // Initialize the Enc Constant Table Surface
        MOS_LOCK_PARAMS lockFlags;
        MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
        lockFlags.WriteOnly = 1;

        uint8_t* data = (uint8_t*)m_osInterface->pfnLockResource(
            m_osInterface,
            &m_encConstantTableForLcu64B.sResource,
            &lockFlags);
        CODECHAL_ENCODE_CHK_NULL_RETURN(data);

        MOS_SecureMemcpy(data, m_encBConstantDataLutLcu64Size, (const void*) m_encBConstantDataLutLcu64, m_encBConstantDataLutLcu64Size);

        m_osInterface->pfnUnlockResource(
            m_osInterface,
            &m_encConstantTableForLcu64B.sResource);

    }

    // Job Queue Scratch Surface for multi-threading for LCU64 B-kernel
    // free the JobQ surface allocated for LCU32
    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_jobQueueHeaderSurfaceForB.sResource);

    size = (m_widthAlignedMaxLcu >> 5) * (m_heightAlignedMaxLcu >> 5) * 32;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer(
        &m_jobQueueHeaderSurfaceForB,
        size,
        "Job Queue Header Surface for multi-thread LCU64 B"));

    // Job Queue Data Surface for LCU64 CU32
    if (Mos_ResourceIsNull(&m_jobQueueDataSurfaceForBLcu64Cu32.OsResource))
    {
        width  = (m_widthAlignedMaxLcu >> 5) * 32;
        height = (m_heightAlignedMaxLcu >> 5) * 58;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer2D(
            &m_jobQueueDataSurfaceForBLcu64Cu32,
            width,
            height,
            "Job Queue Data Surface for LCU64 CU32"));
    }

    // Job Queue Data Surface for LCU64
    if (Mos_ResourceIsNull(&m_jobQueueDataSurfaceForBLcu64.OsResource))
    {
        width  = (m_widthAlignedMaxLcu >> 6) * 32;
        height = (m_heightAlignedMaxLcu >> 6) * 66;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer2D(
            &m_jobQueueDataSurfaceForBLcu64,
            width,
            height,
            "Job Queue Data Surface for LCU64"));
    }

    // Residual Data Scratch Surface LCU64
    if(Mos_ResourceIsNull(&m_residualDataScratchSurfaceForBLcu64.OsResource))
    {
        width  = (m_widthAlignedLcu32 << 1);
        height = (m_heightAlignedLcu32 << 2);

        CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer2D(
            &m_residualDataScratchSurfaceForBLcu64,
            width,
            height,
            "Residual Data Scratch Surface"));
    }

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG10::AllocateMeResources()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    if(!m_encEnabled || !m_hmeSupported)
    {
        return eStatus;
    }

    uint32_t width = 0, height = 0;
    if (Mos_ResourceIsNull(&m_s4XMeMvDataBuffer.OsResource))
    {
        //each 8x8 block has a MV hence div by 8, 2 for interleaved L0 and L1, 4 bytes per MV, 4 maximum number of ref frame for both FWD and BWD references possible
        width  = (m_downscaledWidth4x >> 3) * 2 * 4 * 4;
        height = (m_downscaledHeight4x >> 3);

        CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer2D(
            &m_s4XMeMvDataBuffer,
            width,
            height,
            "4xME MV Data Buffer"));
    }

    if (Mos_ResourceIsNull(&m_s4XMeDistortionBuffer.OsResource))
    {
        width  = (m_downscaledWidth4x >> 3) * 4 * 2; //4 for interleaved L0,L1,Intra distortion and padding, 2 bytes of data
        height = (m_downscaledHeight4x >> 3) * 4; // maximum number of ref frame for both FWD and BWD references possible

        CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer2D(
            &m_s4XMeDistortionBuffer,
            width,
            height,
            "4xME Distortion Buffer"));
    }

    if (m_16xMeSupported)
    {
        if (Mos_ResourceIsNull(&m_s16XMeMvDataBuffer.OsResource))
        {
            //each 8x8 block has a MV hence div by 8, 2 for interleaved L0 and L1, 4 bytes per MV, 4 maximum number of ref frame for both FWD and BWD references possible
            width  = (m_downscaledWidth16x >> 3) * 2 * 4 * 4;
            height = (m_downscaledHeight16x >> 3);

            CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer2D(
                &m_s16XMeMvDataBuffer,
                width,
                height,
                "16xME MV Data Buffer"));
        }
    }

    // Mv and Distortion Summation Surface
    if( Mos_ResourceIsNull(&m_mvAndDistortionSumSurface.sResource))
    {
        uint32_t size = 32;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer(
            &m_mvAndDistortionSumSurface,
            size,
            "Mv and Distortion Summation Surface"));
    }

    // BRC Distortion Surface
    if (Mos_ResourceIsNull(&m_brcBuffers.sMeBrcDistortionBuffer.OsResource))
    {
        uint32_t width = MOS_ALIGN_CEIL((m_picWidthInMb << 3), 64);
        uint32_t height = MOS_ALIGN_CEIL((m_picHeightInMb << 2), 8);

        CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer2D(
            &m_brcBuffers.sMeBrcDistortionBuffer,
            width,
            height,
            "Brc Distortion Surface Buffer"));
    }

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG10::AllocateEncResources()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    uint32_t width = 0, height = 0, size = 0;
    // Surfaces used by I and B-kernels

    // Intermediate CU Record Surface
    // LCU64 aligned allocation. While setting up surface state in some cases the width and height will be set LCU32 aligned.
    if (Mos_ResourceIsNull(&m_intermediateCuRecordSurfaceLcu32.OsResource))
    {
        width  = m_widthAlignedMaxLcu;
        height = m_heightAlignedMaxLcu >> 1;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer2D(
            &m_intermediateCuRecordSurfaceLcu32,
            width,
            height,
            "Intermediate CU record Surface"));
    }

    // LCU Level Input Data
    if (Mos_ResourceIsNull(&m_lcuLevelInputData.sResource))
    {
        size = 16 * ((m_widthAlignedMaxLcu >> 6) << 1) * ((m_heightAlignedMaxLcu >> 6) << 1);

        CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer(
            &m_lcuLevelInputData,
            size,
            "Lcu Level Data Input Surface"));
    }

    // Concurrent Thread Group Data Surface
    if (Mos_ResourceIsNull(&m_concurrentThreadGroupData.sResource))
    {
        // Maximum of 16 Thread Groups, Each Thread Group has 64 bytes of data
        size = CODECHAL_MEDIA_WALKER_MAX_COLORS * sizeof(CODECHAL_ENC_HEVC_CONCURRENT_THREAD_GROUP_DATA_G10);

        CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer(
            &m_concurrentThreadGroupData,
            size,
            "Concurrent Thread Group Data Input Surface"));
    }

    // Cu Split Surface
    if (Mos_ResourceIsNull(&m_cuSplitSurface.OsResource))
    {
        width  = (m_widthAlignedMaxLcu >> 4);   // ((W + 63)/64)*4
        height = (m_heightAlignedMaxLcu >> 4);  // ((H + 63)/64)*4

        CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer2D(
            &m_cuSplitSurface,
            width,
            height,
            "Cu Split Surface"));
    }

    // Kernel Debug Surface for B-kernel
    if (Mos_ResourceIsNull(&m_kernelDebug.sResource))
    {
        size = CODECHAL_PAGE_SIZE; // 4K bytes for the debug surface

        CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer(
            &m_kernelDebug,
            size,
            "Kernel 1D Debug Surface"));
    }

    m_allocator->AllocateResource(m_standard, m_brcCombinedEncBufferSize, 1, brcInputForEncKernel, "brcInputForEncKernel", true);

    // Surfaces used by I-kernel
    // Enc Constant Table for I
    if (Mos_ResourceIsNull(&m_encConstantTableForI.sResource))
    {
        size = m_encIConstantDataLutSize;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer(
            &m_encConstantTableForI,
            size,
            "Enc Constant Table Surface For I"));

        // Initialize the Enc Constant Table Surface
        MOS_LOCK_PARAMS lockFlags;
        MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
        lockFlags.WriteOnly = 1;
        uint8_t* data = (uint8_t*)m_osInterface->pfnLockResource(
            m_osInterface,
            &m_encConstantTableForI.sResource,
            &lockFlags);
        CODECHAL_ENCODE_CHK_NULL_RETURN(data);

        MOS_SecureMemcpy(data, m_encIConstantDataLutSize, (const void*) m_encIConstantDataLut, m_encIConstantDataLutSize);

        m_osInterface->pfnUnlockResource(
            m_osInterface,
            &m_encConstantTableForI.sResource);
    }

    // Scartch Surface for I-kernel
    if (Mos_ResourceIsNull(&m_scratchSurface.OsResource))
    {
        width  = m_widthAlignedLcu32 >> 3;
        height = m_heightAlignedLcu32 >> 5;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer2D(
            &m_scratchSurface,
            width,
            height,
            "Scratch Surface for I Kernel"));
    }

    // Surfaces used by B-kernel

    // Second Intermediate CU Record Surface
    if (Mos_ResourceIsNull(&m_secondIntermediateCuRecordSurfaceLcu32.OsResource))
    {
        width  = m_widthAlignedMaxLcu;
        height = m_heightAlignedMaxLcu >> 1;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer2D(
            &m_secondIntermediateCuRecordSurfaceLcu32,
            width,
            height,
            "Second Intermediate CU record Surface"));
    }

    // Scratch Surface for Internal use
    if (Mos_ResourceIsNull(&m_lcuEncodingScratchSurface.sResource))
    {
        size = 13312 * ((m_widthAlignedMaxLcu >> 6) << 1) * ((m_heightAlignedMaxLcu >> 6) << 1);

        CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer(
            &m_lcuEncodingScratchSurface,
            size,
            "Lcu Encoding Scratch Surface"));
    }

    // Distortion surface for 64x64
    if (Mos_ResourceIsNull(&m_64x64DistortionSurface.sResource))
    {
        size = (m_widthAlignedMaxLcu >> 6) * (m_heightAlignedMaxLcu >> 6) * 32;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer(
            &m_64x64DistortionSurface,
            size,
            "Distortion surface for 64x64"));
    }

    // Enc constant table for B LCU32
    if (Mos_ResourceIsNull(&m_encConstantTableForB.sResource))
    {
        size = m_encBConstantDataLutSize;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer(
            &m_encConstantTableForB,
            size,
            "Enc Constant Table Surface For B LCU32"));

        // Initialize the Enc Constant Table Surface
        MOS_LOCK_PARAMS lockFlags;
        MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
        lockFlags.WriteOnly = 1;
        uint8_t* data = (uint8_t*)m_osInterface->pfnLockResource(
            m_osInterface,
            &m_encConstantTableForB.sResource,
            &lockFlags);
        CODECHAL_ENCODE_CHK_NULL_RETURN(data);

        MOS_SecureMemcpy(data, m_encBConstantDataLutSize, (const void*) m_encBConstantDataLut, m_encBConstantDataLutSize);

        m_osInterface->pfnUnlockResource(
            m_osInterface,
            &m_encConstantTableForB.sResource);
    }

    // Job Queue Scratch Surface for multi-threading for LCU32 B-kernel
    if( Mos_ResourceIsNull(&m_jobQueueHeaderSurfaceForB.sResource) )
    {
        size = (m_widthAlignedMaxLcu >> 5) * (m_heightAlignedMaxLcu >> 5) * m_jobQueueSizeFor32x32Block;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer(
            &m_jobQueueHeaderSurfaceForB,
            size,
            "Job Queue Header Surface for multi-thread LCU32 B"));
    }

    // Job Queue Header Surface for multi-threading for LCU64 B-kernel
    if (Mos_ResourceIsNull(&m_jobQueueHeaderSurfaceForBLcu64.sResource))
    {
        size = (m_widthAlignedMaxLcu >> 5) * (m_heightAlignedMaxLcu >> 5) * 32;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer(
            &m_jobQueueHeaderSurfaceForBLcu64,
            size,
            "Job Queue Header Surface for multi-thread LCU64 B"));
    }

    // Residual Data Scratch Surface LCU32
    if (Mos_ResourceIsNull(&m_residualDataScratchSurfaceForBLcu32.OsResource))
    {
        width  = (m_widthAlignedLcu32 << 1);
        height = (m_heightAlignedLcu32 << 2);

        CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer2D(
            &m_residualDataScratchSurfaceForBLcu32,
            width,
            height,
            "Residual Data Scratch Surface"));
    }

    // MB statistics surface
    if (Mos_ResourceIsNull(&m_mbStatisticsSurface.OsResource))
    {
        width  = MOS_ALIGN_CEIL(m_picWidthInMb * 4, 64);
        height = 2 * MOS_ALIGN_CEIL(m_picHeightInMb, 8);

        CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer2D(
            &m_mbStatisticsSurface,
            width,
            height,
            "MB stats surface"));
    }

    // For 10 bit HEVC support
    if (m_is10BitHevc)
    {
        //Output surface for format conversion from 10bit to 8 bit
        for(uint32_t i = 0 ; i < NUM_FORMAT_CONV_FRAMES ; i++)
        {
            if (Mos_ResourceIsNull(&m_formatConvertedSurface[i].OsResource))
            {
                width = m_frameWidth;
                height = m_frameHeight;

                CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateSurface(
                    &m_formatConvertedSurface[i],
                    width,
                    height,
                    "Format Converted Surface"));
            }
        }
    }

    // MB split surface
    if(Mos_ResourceIsNull(&m_mbSplitSurface.OsResource))
    {
        width  = m_widthAlignedMaxLcu >> 2;
        height = m_heightAlignedMaxLcu >> 4;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer2D(
            &m_mbSplitSurface,
            width,
            height,
            "MB split surface"));
    }

    if (m_hmeSupported)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateMeResources());
    }

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG10::DestroyMeResources()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_s16XMeMvDataBuffer.OsResource);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_s4XMeDistortionBuffer.OsResource);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_s4XMeMvDataBuffer.OsResource);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_mvAndDistortionSumSurface.sResource);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_brcBuffers.sMeBrcDistortionBuffer.OsResource);

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG10::FreeEncResources()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_Delete(m_scalingAndConversionKernelState);
    m_scalingAndConversionKernelState = nullptr;
    MOS_FreeMemory(m_scalingAndConversionKernelBindingTable);
    m_scalingAndConversionKernelBindingTable = nullptr;

    MOS_Delete(m_meKernelState);
    m_meKernelState = nullptr;
    MOS_FreeMemory(m_meKernelBindingTable);
    m_meKernelBindingTable = nullptr;

    MOS_DeleteArray(m_brcKernelStates);
    m_brcKernelStates = nullptr;
    MOS_FreeMemory(m_brcKernelBindingTable);
    m_brcKernelBindingTable = nullptr;

    MOS_DeleteArray(m_mbEncKernelStates);
    m_mbEncKernelStates = nullptr;
    MOS_FreeMemory(m_mbEncKernelBindingTable);
    m_mbEncKernelBindingTable = nullptr;

    // Surfaces used by I and B-kernels
    // Release Intermediate CU Record Surface
    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_intermediateCuRecordSurfaceLcu32.OsResource);

    // Release LCU Level Input Data
    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_lcuLevelInputData.sResource);

    // Release Concurrent Thread Group Input Data
    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_concurrentThreadGroupData.sResource);

    // Release CU split surface
    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_cuSplitSurface.OsResource);

    // Release Kernel Debug Surface
    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_kernelDebug.sResource);

    // Surfaces used by I-kernel
    // Release Enc Constant Table for I
    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_encConstantTableForI.sResource);

    // Release Scartch Surface for I-kernel
    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_scratchSurface.OsResource);

    // Surfaces used by B-kernel
    // Release Second Intermediate CU Record Surface
    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_secondIntermediateCuRecordSurfaceLcu32.OsResource);

    // Release Intermediate CU Record Surface for Lcu64 B-kernel
    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_intermediateCuRecordSurfaceLcu64B.OsResource);

    // Release Scratch Surface for Internal use
    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_lcuEncodingScratchSurface.sResource);

    // Release Scratch Surface for Internal use Lcu64B
    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_lcuEncodingScratchSurfaceLcu64B.sResource);

    // Release Enc constant table for B LCU64
    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_encConstantTableForLcu64B.sResource);

    // Release Distortion surface for 64x64
    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_64x64DistortionSurface.sResource);

    // Release Enc constant table for B LCU32
    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_encConstantTableForB.sResource);

    // Release Job Queue Header Surface for multi-threading for LCU 32 B-kernel
    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_jobQueueHeaderSurfaceForB.sResource);

    // Release Job Queue Header Surface for multi-threading for LCU 64 B-kernel
    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_jobQueueHeaderSurfaceForBLcu64.sResource);

    // Release Job Queue Data Surface for LCU64 CU32
    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_jobQueueDataSurfaceForBLcu64Cu32.OsResource);

    // Release Job Queue Data Surface for LCU64
    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_jobQueueDataSurfaceForBLcu64.OsResource);

    // Release Residual Data Scratch Surface for LCU 32 B-kernel
    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_residualDataScratchSurfaceForBLcu32.OsResource);

    // Release Residual Data Scratch Surface for LCU 64 B-kernel
    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_residualDataScratchSurfaceForBLcu64.OsResource);

    // Release Output surfaces for format conversion from 10bit to 8 bit
    for(uint32_t i = 0 ; i < NUM_FORMAT_CONV_FRAMES; i++)
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &m_formatConvertedSurface[i].OsResource);
    }

    // Release Mb Statistics Surface
    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_mbStatisticsSurface.OsResource);

    // Release Mb Split Surface
    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_mbSplitSurface.OsResource);

    // Release Frame Statistics Streamout Data Destination Buffer
    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_resFrameStatStreamOutBuffer);

    // Release SSE Source Pixel Row Store Buffer
    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_resSseSrcPixelRowStoreBuffer);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(DestroyMeResources());

#if (_DEBUG || _RELEASE_INTERNAL)
    if (m_swBrcMode != nullptr)
    {
        m_osInterface->pfnFreeLibrary(m_swBrcMode);
        m_swBrcMode = nullptr;
    }
#endif // (_DEBUG || _RELEASE_INTERNAL)

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG10::AllocatePakResources()
{
    MOS_STATUS eStatus = CodechalEncHevcState::AllocatePakResources();

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate PAK resources");
        return eStatus;
    }

    MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
    MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
    allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
    allocParamsForBufferLinear.Format = Format_Buffer;

    // Allocate Frame Statistics Streamout Data Destination Buffer = HEVC Frame Statistics
    uint32_t size                       = m_sizeOfHcpPakFrameStats;
    allocParamsForBufferLinear.dwBytes = size;
    allocParamsForBufferLinear.pBufName = "FrameStatStreamOutBuffer";

    eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_resFrameStatStreamOutBuffer);
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate FrameStat StreamOut Buffer.");
        return eStatus;
    }

    // Allocate SSE Source Pixel Row Store Buffer
    size = ((m_widthAlignedMaxLcu + 2) * 64) * (4 + 4);
    size <<= 1;
    MOS_ALIGN_CEIL(size, CODECHAL_CACHELINE_SIZE);
    allocParamsForBufferLinear.dwBytes = size;
    allocParamsForBufferLinear.pBufName = "SseSrcPixelRowStoreBuffer";

    eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_resSseSrcPixelRowStoreBuffer);
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate SseSrcPixelRowStore Buffer.");
        return eStatus;
    }

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG10::FreePakResources()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncHevcState::FreePakResources());

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_resFrameStatStreamOutBuffer);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_resSseSrcPixelRowStoreBuffer);

    return eStatus;
}

void CodechalEncHevcStateG10::LoadCosts(uint8_t sliceType, uint8_t qp, uint16_t *lambdaMd, uint32_t *lambdaRd, uint32_t *tuSadThreshold)
{
    if (sliceType >= CODECHAL_ENCODE_HEVC_NUM_SLICE_TYPES)
    {
        CODECHAL_ENCODE_NORMALMESSAGE("Invalid slice type");
        sliceType = CODECHAL_ENCODE_HEVC_I_SLICE;
    }

    int32_t qpMinus12 = qp - 12;
    double qpScale = (sliceType == CODECHAL_ENCODE_HEVC_I_SLICE) ? 5.0 : 0.55;
    double lambda = sqrt(qpScale * pow(2.0, MOS_MAX(0, qpMinus12) / 3.0));
    double costScale = (sliceType == CODECHAL_ENCODE_HEVC_B_SLICE) ? 2.0 : 1.0;

    *lambdaMd = (uint16_t)(lambda * 256 + 0.5);
    *lambdaRd = (uint32_t)(qpScale * pow(2.0, MOS_MAX(0, qpMinus12) / 3.0) * 256 + 0.5);
    *tuSadThreshold = (uint32_t)(sqrt(0.85 * pow(2.0, MOS_MAX(0, qpMinus12) / 3.0)) * 0.4 * 256 + 0.5);

    uint8_t lcuIdx                    = ((m_hevcSeqParams->log2_max_coding_block_size_minus3 + 3) == 6) ? 1 : 0;
    double interWeighingFactor = costScale * lambda;
    double intraWeighingFactor = interWeighingFactor * m_lambdaScaling[sliceType][qp];
    m_modeCost[LUTMODE_INTRA_NONPRED] = Map44LutValue((uint32_t)(intraWeighingFactor * m_modeBits[lcuIdx][sliceType][LUTMODE_INTRA_NONPRED]), 0x6f);
    m_modeCost[LUTMODE_INTRA_32x32]   = Map44LutValue((uint32_t)(intraWeighingFactor * m_modeBits[lcuIdx][sliceType][LUTMODE_INTRA_32x32]), 0x8f);
    m_modeCost[LUTMODE_INTRA_16x16]   = Map44LutValue((uint32_t)(intraWeighingFactor * m_modeBits[lcuIdx][sliceType][LUTMODE_INTRA_16x16]), 0x8f);
    m_modeCost[LUTMODE_INTRA_8x8]     = Map44LutValue((uint32_t)(intraWeighingFactor * m_modeBits[lcuIdx][sliceType][LUTMODE_INTRA_8x8]), 0x8f);
    m_modeCost[LUTMODE_INTRA_CHROMA]  = Map44LutValue((uint32_t)(intraWeighingFactor * m_modeBits[lcuIdx][sliceType][LUTMODE_INTRA_CHROMA]), 0x6f);

    m_modeCost[LUTMODE_INTER_32x32]   = Map44LutValue((uint32_t)(interWeighingFactor * m_modeBits[lcuIdx][sliceType][LUTMODE_INTER_32x32]), 0x8f);
    m_modeCost[LUTMODE_INTER_32x16]   = Map44LutValue((uint32_t)(interWeighingFactor * m_modeBits[lcuIdx][sliceType][LUTMODE_INTER_32x16]), 0x8f);
    m_modeCost[LUTMODE_INTER_16x16]   = Map44LutValue((uint32_t)(interWeighingFactor * m_modeBits[lcuIdx][sliceType][LUTMODE_INTER_16x16]), 0x6f);
    m_modeCost[LUTMODE_INTER_16x8]    = Map44LutValue((uint32_t)(interWeighingFactor * m_modeBits[lcuIdx][sliceType][LUTMODE_INTER_16x8]), 0x6f);
    m_modeCost[LUTMODE_INTER_8x8]     = Map44LutValue((uint32_t)(0.45 * m_modeBits[lcuIdx][sliceType][LUTMODE_INTER_8x8]), 0x6f);

    m_modeCost[LUTMODE_INTER_BIDIR]   = Map44LutValue((uint32_t)(interWeighingFactor * m_modeBits[lcuIdx][sliceType][LUTMODE_INTER_BIDIR]), 0x6f);
    if (!m_hevcSliceParams->num_ref_idx_l0_active_minus1)
    {
        m_modeCost[LUTMODE_REF_ID]    = Map44LutValue((uint32_t)(interWeighingFactor * m_modeBits[lcuIdx][sliceType][LUTMODE_REF_ID]), 0x6f);
    }

    return;
}

// ------------------------------------------------------------------------------
//| Purpose:    Setup curbe for HEVC ME Kernels
//| Return:     N/A
//------------------------------------------------------------------------------
MOS_STATUS CodechalEncHevcStateG10::SetCurbeMe(
    HmeLevel hmeLevel,
    HEVC_ME_DIST_TYPE distType)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_meKernelState);

    // Initialize the CURBE data
    CODECHAL_ENC_HEVC_ME_CURBE_G10 curbe = m_meCurbeInit;

    //Mostly using defaults as of now.
    curbe.DW0_RoundedFrameWidthInMvUnitsfor4X       = m_downscaledWidth4x >> 3;
    curbe.DW0_RoundedFrameHeightInMvUnitsfor4X      = m_downscaledHeight4x >> 3;

    curbe.DW3_ImeRefWindowSize                      = IME_REF_WINDOW_MODE_BIG;

    curbe.DW5_StartCenter0_X                        = ((m_imeRefWindowSize[curbe.DW3_ImeRefWindowSize][0] - 32) >> 3) & 0xF;
    curbe.DW5_StartCenter0_Y                        = ((m_imeRefWindowSize[curbe.DW3_ImeRefWindowSize][1] - 32) >> 3) & 0xF;

    curbe.DW6_SliceType                             = (distType == HEVC_ME_DIST_TYPE_INTER_BRC_DIST) ? 1 : 0;
    curbe.DW6_HmeStage                              = (distType != HEVC_ME_DIST_TYPE_INTER_BRC_DIST) ?
                                                        HME_STAGE_4x_NO_16x :
                                                        ((hmeLevel == HME_LEVEL_4x) ?
                                                         (m_16xMeSupported ? HME_STAGE_4x_AFTER_16x : HME_STAGE_4x_NO_16x) :
                                                         HME_STAGE_16x);
    curbe.DW6_NumRefL0 = m_hevcSliceParams->num_ref_idx_l0_active_minus1 + 1;
    curbe.DW6_NumRefL1 = !m_lowDelay ? (m_hevcSliceParams->num_ref_idx_l1_active_minus1 + 1) : 0;

    curbe.DW7_RoundedFrameWidthInMvUnitsFor16x      = m_downscaledWidth16x >> 3;
    curbe.DW7_RoundedFrameHeightInMvUnitsfor16X     = m_downscaledHeight16x >> 3;

    curbe.DW25_FrameWidthInSamplesOfCurrentStage    = (hmeLevel == HME_LEVEL_4x) ? (m_frameWidth >> 2) : (m_frameWidth >> 4);
    curbe.DW25_FrameHeightInSamplesOfCurrentStage   = (hmeLevel == HME_LEVEL_4x) ? (m_frameHeight >> 2) : (m_frameHeight >> 4);

    curbe.DW33_SicLog2MinCuSize = m_hevcSeqParams->log2_min_coding_block_size_minus3 + 3;

    curbe.DW34_BTI_HmeOutputMvDataSurface           = HME_OUTPUT_MV_DATA;
    curbe.DW35_BTI_16xInputMvDataSurface            = HME_16xINPUT_MV_DATA;
    curbe.DW36_BTI_4xOutputDistortionSurface        = HME_4xOUTPUT_DISTORTION;
    curbe.DW37_BTI_VmeSurfaceIndex                  = HME_VME_PRED_CURR_PIC_IDX0;
    curbe.DW38_BTI_4xDsSurface                      = HME_4xDS_INPUT;
    curbe.DW39_BTI_BrcDistortionSurface             = HME_BRC_DISTORTION;
    curbe.DW40_BTI_Mv_And_Distortion_SumSurface     = HME_MV_AND_DISTORTION_SUM;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_meKernelState->m_dshRegion.AddData(
        &curbe,
        m_meKernelState->dwCurbeOffset,
        sizeof(curbe)));

    return eStatus;
}

// ------------------------------------------------------------------------------
//| Purpose:    Setup curbe for HEVC BrcInitReset Kernel
//| Return:     N/A
//------------------------------------------------------------------------------
MOS_STATUS CodechalEncHevcStateG10::SetCurbeBrcInitReset(CODECHAL_HEVC_BRC_KRNIDX brcKrnIdx)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_ASSERT(brcKrnIdx == CODECHAL_HEVC_BRC_INIT || brcKrnIdx == CODECHAL_HEVC_BRC_RESET);

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_brcKernelStates);

    // Initialize the CURBE data
    CODECHAL_ENC_HEVC_BRC_INITRESET_CURBE_G10 curbe = m_brcInitResetCurbeInit;

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

    curbe.DW0_ProfileLevelMaxFrame  = GetProfileLevelMaxFrameSize();
    curbe.DW1_InitBufFull           = m_hevcSeqParams->InitVBVBufferFullnessInBit;
    curbe.DW2_BufSize               = m_hevcSeqParams->VBVBufferSizeInBit;
    curbe.DW3_TargetBitRate         = m_hevcSeqParams->TargetBitRate * CODECHAL_ENCODE_BRC_KBPS;
    curbe.DW4_MaximumBitRate        = m_hevcSeqParams->MaxBitRate * CODECHAL_ENCODE_BRC_KBPS;
    curbe.DW5_MinimumBitRate        = 0;
    curbe.DW6_FrameRateM            = m_hevcSeqParams->FrameRate.Numerator;
    curbe.DW7_FrameRateD            = m_hevcSeqParams->FrameRate.Denominator;
    curbe.DW8_BRCFlag               = 0;
    curbe.DW8_BRCFlag               = (m_lcuBrcEnabled) ? 0 : BRCINIT_DISABLE_MBBRC;
    curbe.DW25_ACQPBuffer           = 1;
    curbe.DW25_Log2MaxCuSize        = (m_isMaxLcu64) ? 6 : 5;
    curbe.DW25_SlidingWindowSize    = m_slidingWindowSize;
    curbe.DW8_BRCFlag              |= BRCINIT_IGNORE_PICTURE_HEADER_SIZE;

    if (m_hevcSeqParams->RateControlMethod == RATECONTROL_CBR)
    {
        curbe.DW4_MaximumBitRate    = curbe.DW3_TargetBitRate;
        curbe.DW8_BRCFlag          |= BRCINIT_ISCBR;
    }
    else if (m_hevcSeqParams->RateControlMethod == RATECONTROL_VBR)
    {
        if (curbe.DW4_MaximumBitRate < curbe.DW3_TargetBitRate)
        {
            curbe.DW4_MaximumBitRate = 2 * curbe.DW3_TargetBitRate;
        }
        curbe.DW8_BRCFlag          |= BRCINIT_ISVBR;
    }
    else if (m_hevcSeqParams->RateControlMethod == RATECONTROL_AVBR)
    {
        curbe.DW8_BRCFlag          |= BRCINIT_ISAVBR;
        // For AVBR, max bitrate = target bitrate,
        curbe.DW3_TargetBitRate  = m_hevcSeqParams->TargetBitRate * CODECHAL_ENCODE_BRC_KBPS;
        curbe.DW4_MaximumBitRate = m_hevcSeqParams->TargetBitRate * CODECHAL_ENCODE_BRC_KBPS;
    }
    else if (m_hevcSeqParams->RateControlMethod == RATECONTROL_ICQ)
    {
        curbe.DW8_BRCFlag          |= BRCINIT_ISICQ;
        curbe.DW25_ACQPBuffer = m_hevcSeqParams->ICQQualityFactor;
    }
    else if (m_hevcSeqParams->RateControlMethod == RATECONTROL_VCM)
    {
        curbe.DW4_MaximumBitRate    = curbe.DW3_TargetBitRate;
        curbe.DW8_BRCFlag          |= BRCINIT_ISVCM;
    }
    else if (m_hevcSeqParams->RateControlMethod == RATECONTROL_CQP)
    {
        curbe.DW8_BRCFlag           = BRCINIT_ISCQP;
    }

    curbe.DW9_FrameWidth            = m_oriFrameWidth;
    curbe.DW10_FrameHeight          = m_oriFrameHeight;
    curbe.DW10_AVBRAccuracy         = m_usAvbrAccuracy;
    curbe.DW11_AVBRConvergence      = m_usAvbrConvergence;

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
        curbe.DW8_BRC_Param_A  = ((m_hevcSeqParams->GopPicSize) / m_hevcSeqParams->GopRefDist);
        curbe.DW9_BRC_Param_B  = curbe.DW8_BRC_Param_A;
        curbe.DW13_BRC_Param_C = curbe.DW8_BRC_Param_A * 2;
        curbe.DW14_BRC_Param_D = ((m_hevcSeqParams->GopPicSize) - (curbe.DW8_BRC_Param_A) - (curbe.DW13_BRC_Param_C) - (curbe.DW9_BRC_Param_B));
        // B1 Level GOP
        if (m_hevcSeqParams->NumOfBInGop[2] == 0)
        {
            curbe.DW14_MaxBRCLevel = 3;
        }
        // B2 Level GOP
        else
        {
            curbe.DW14_MaxBRCLevel = 4;
        }
    }
    // For Regular GOP - No BPyramid
    else
    {
        curbe.DW14_MaxBRCLevel = 1;
        curbe.DW8_BRC_Param_A  = (m_hevcSeqParams->GopRefDist) ? ((m_hevcSeqParams->GopPicSize - 1) / m_hevcSeqParams->GopRefDist) : 0;
        curbe.DW9_BRC_Param_B  = m_hevcSeqParams->GopPicSize - 1 - curbe.DW8_BRC_Param_A;
    }

    // Set dynamic thresholds
    double inputBitsPerFrame =
        ((double)(curbe.DW4_MaximumBitRate) * (double)(curbe.DW7_FrameRateD) /
        (double)(curbe.DW6_FrameRateM));

    if (curbe.DW2_BufSize < (uint32_t)inputBitsPerFrame * 4)
    {
        curbe.DW2_BufSize = (uint32_t)inputBitsPerFrame * 4;
    }

    if (curbe.DW1_InitBufFull == 0)
    {
        curbe.DW1_InitBufFull = 7 * curbe.DW2_BufSize/8;
    }
    if (curbe.DW1_InitBufFull < (uint32_t)(inputBitsPerFrame*2))
    {
        curbe.DW1_InitBufFull = (uint32_t)(inputBitsPerFrame*2);
    }
    if (curbe.DW1_InitBufFull > curbe.DW2_BufSize)
    {
        curbe.DW1_InitBufFull = curbe.DW2_BufSize;
    }

    if (m_hevcSeqParams->RateControlMethod == RATECONTROL_AVBR)
    {
        // For AVBR, Buffer size =  2*Bitrate, InitVBV = 0.75 * BufferSize
        curbe.DW2_BufSize     = 2 * m_hevcSeqParams->TargetBitRate * CODECHAL_ENCODE_BRC_KBPS;
        curbe.DW1_InitBufFull = (uint32_t)(0.75 * curbe.DW2_BufSize);
    }

    double bpsRatio = inputBitsPerFrame / ((double)(curbe.DW2_BufSize)/30);
    bpsRatio = (bpsRatio < 0.1) ? 0.1 : (bpsRatio > 3.5) ? 3.5 : bpsRatio;

    curbe.DW19_DeviationThreshold0_PBframe      = (uint32_t) (-50 * pow(0.90, bpsRatio));
    curbe.DW19_DeviationThreshold1_PBframe      = (uint32_t) (-50 * pow(0.66, bpsRatio));
    curbe.DW19_DeviationThreshold2_PBframe      = (uint32_t) (-50 * pow(0.46, bpsRatio));
    curbe.DW19_DeviationThreshold3_PBframe      = (uint32_t) (-50 * pow(0.3, bpsRatio));

    curbe.DW20_DeviationThreshold4_PBframe      = (uint32_t) (50 * pow(0.3, bpsRatio));
    curbe.DW20_DeviationThreshold5_PBframe      = (uint32_t) (50 * pow(0.46, bpsRatio));
    curbe.DW20_DeviationThreshold6_PBframe      = (uint32_t) (50 * pow(0.7, bpsRatio));
    curbe.DW20_DeviationThreshold7_PBframe      = (uint32_t) (50 * pow(0.9, bpsRatio));

    curbe.DW21_DeviationThreshold0_VBRcontrol   = (uint32_t) (-50 * pow(0.9, bpsRatio));
    curbe.DW21_DeviationThreshold1_VBRcontrol   = (uint32_t) (-50 * pow(0.7, bpsRatio));
    curbe.DW21_DeviationThreshold2_VBRcontrol   = (uint32_t) (-50 * pow(0.5, bpsRatio));
    curbe.DW21_DeviationThreshold3_VBRcontrol   = (uint32_t) (-50 * pow(0.3, bpsRatio));

    curbe.DW22_DeviationThreshold4_VBRcontrol   = (uint32_t) (100 * pow(0.4, bpsRatio));
    curbe.DW22_DeviationThreshold5_VBRcontrol   = (uint32_t) (100 * pow(0.5, bpsRatio));
    curbe.DW22_DeviationThreshold6_VBRcontrol   = (uint32_t) (100 * pow(0.75, bpsRatio));
    curbe.DW22_DeviationThreshold7_VBRcontrol   = (uint32_t) (100 * pow(0.9, bpsRatio));

    curbe.DW23_DeviationThreshold0_Iframe       = (uint32_t) (-50 * pow(0.8, bpsRatio));
    curbe.DW23_DeviationThreshold1_Iframe       = (uint32_t) (-50 * pow(0.6, bpsRatio));
    curbe.DW23_DeviationThreshold2_Iframe       = (uint32_t) (-50 * pow(0.34, bpsRatio));
    curbe.DW23_DeviationThreshold3_Iframe       = (uint32_t) (-50 * pow(0.2, bpsRatio));

    curbe.DW24_DeviationThreshold4_Iframe       = (uint32_t) (50 * pow(0.2, bpsRatio));
    curbe.DW24_DeviationThreshold5_Iframe       = (uint32_t) (50 * pow(0.4, bpsRatio));
    curbe.DW24_DeviationThreshold6_Iframe       = (uint32_t) (50 * pow(0.66, bpsRatio));
    curbe.DW24_DeviationThreshold7_Iframe       = (uint32_t) (50 * pow(0.9, bpsRatio));

    if (m_brcInit)
    {
        m_dBrcInitCurrentTargetBufFullInBits = curbe.DW1_InitBufFull;
    }

    m_brcInitResetBufSizeInBits      = curbe.DW2_BufSize;
    m_dBrcInitResetInputBitsPerFrame = inputBitsPerFrame;

    PMHW_KERNEL_STATE kernelState = &m_brcKernelStates[brcKrnIdx];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(kernelState->m_dshRegion.AddData(
        &curbe,
        kernelState->dwCurbeOffset,
        sizeof(curbe)));

    return eStatus;
}

// ------------------------------------------------------------------------------
//| Purpose:    Setup curbe for HEVC BrcUpdate Kernel
//| Return:     N/A
//------------------------------------------------------------------------------
MOS_STATUS CodechalEncHevcStateG10::SetCurbeBrcUpdate(CODECHAL_HEVC_BRC_KRNIDX brcKrnIdx)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_ASSERT(brcKrnIdx == CODECHAL_HEVC_BRC_FRAME_UPDATE || brcKrnIdx == CODECHAL_HEVC_BRC_LCU_UPDATE);

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_brcKernelStates);

    // Initialize the CURBE data
    CODECHAL_ENC_HEVC_BRC_UPDATE_CURBE_G10 curbe = m_brcUpdateCurbeInit;

    curbe.DW5_TargetSize_Flag = 0;

    if (m_dBrcInitCurrentTargetBufFullInBits > (double)m_brcInitResetBufSizeInBits)
    {
        m_dBrcInitCurrentTargetBufFullInBits -= (double)m_brcInitResetBufSizeInBits;
        curbe.DW5_TargetSize_Flag = 1;
    }

    if (m_numSkipFrames)
    {
        // pass num/size of skipped frames to update BRC
        curbe.DW6_NumSkippedFrames      = m_numSkipFrames;
        curbe.DW15_SizeOfSkippedFrames  = m_sizeSkipFrames;

        // account for skipped frame in calculating CurrentTargetBufFullInBits
        m_dBrcInitCurrentTargetBufFullInBits += m_dBrcInitResetInputBitsPerFrame * m_numSkipFrames;
    }

    curbe.DW0_TargetSize        = (uint32_t)(m_dBrcInitCurrentTargetBufFullInBits);
    curbe.DW1_FrameNumber       = m_storeData - 1;

    curbe.DW2_PictureHeaderSize = GetPicHdrSize();
    curbe.DW5_CurrFrameBrcLevel = m_currFrameBrcLevel;
    curbe.DW5_MaxNumPAKs        = m_mfxInterface->GetBrcNumPakPasses();

    if (m_hevcSeqParams->RateControlMethod == RATECONTROL_CQP)
    {
        curbe.DW6_CqpValue = m_hevcPicParams->QpY + m_hevcSliceParams->slice_qp_delta;
    }

    curbe.DW6_LCUQPAverageEnable  = (m_hevcPicParams->diff_cu_qp_delta_depth == 0) ? 1 : 0;
    curbe.DW6_SlidingWindowEnable = (m_hevcSeqParams->FrameSizeTolerance == EFRAMESIZETOL_LOW);
    curbe.DW14_ParallelMode       = m_hevcSeqParams->ParallelBRC;

    if (brcKrnIdx == CODECHAL_HEVC_BRC_LCU_UPDATE)
    {
        m_dBrcInitCurrentTargetBufFullInBits += m_dBrcInitResetInputBitsPerFrame;
    }

    if (m_hevcSeqParams->RateControlMethod == RATECONTROL_AVBR)
    {
        curbe.DW3_StartGAdjFrame0 = (uint32_t)((10 * m_usAvbrConvergence) / (double)150);
        curbe.DW3_StartGAdjFrame1 = (uint32_t)((50 * m_usAvbrConvergence) / (double)150);
        curbe.DW4_StartGAdjFrame2 = (uint32_t)((100 * m_usAvbrConvergence) / (double)150);
        curbe.DW4_StartGAdjFrame3 = (uint32_t)((150 * m_usAvbrConvergence) / (double)150);

        curbe.DW11_gRateRatioThreshold0 =
            (uint32_t)((100 - (m_usAvbrAccuracy / (double)30) * (100 - 40)));
        curbe.DW11_gRateRatioThreshold1 =
            (uint32_t)((100 - (m_usAvbrAccuracy / (double)30) * (100 - 75)));
        curbe.DW12_gRateRatioThreshold2 = (uint32_t)((100 - (m_usAvbrAccuracy / (double)30) * (100 - 97)));
        curbe.DW12_gRateRatioThreshold3 = (uint32_t)((100 + (m_usAvbrAccuracy / (double)30) * (103 - 100)));
        curbe.DW12_gRateRatioThreshold4 = (uint32_t)((100 + (m_usAvbrAccuracy / (double)30) * (125 - 100)));
        curbe.DW12_gRateRatioThreshold5 = (uint32_t)((100 + (m_usAvbrAccuracy / (double)30) * (160 - 100)));
    }

    PMHW_KERNEL_STATE kernelState = &m_brcKernelStates[brcKrnIdx];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(kernelState->m_dshRegion.AddData(
        &curbe,
        kernelState->dwCurbeOffset,
        sizeof(curbe)));

    return eStatus;
}

// ------------------------------------------------------------------------------
//| Purpose:    Setup curbe for HEVC MbEnc I Kernels
//| Return:     N/A
//------------------------------------------------------------------------------
MOS_STATUS CodechalEncHevcStateG10::SetCurbeMbEncIKernel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_mbEncKernelStates);

    // Initialize the CURBE data
    CODECHAL_ENC_HEVC_MBENC_I_CURBE_G10 curbe = m_mbencICurbeInit;

    curbe.DW0_FrameWidthInSamples   = m_frameWidth;
    curbe.DW0_FrameHeightInSamples  = m_frameHeight;

    uint32_t sliceQp = m_hevcSliceParams->slice_qp_delta + m_hevcPicParams->QpY;
    CODECHAL_ENCODE_ASSERT(sliceQp < QP_NUM);

    uint16_t lambdaMd = 0;
    uint32_t lambdaRd = 0, tuSadThreshold = 0;
    LoadCosts(CODECHAL_ENCODE_HEVC_I_SLICE, (uint8_t)sliceQp, &lambdaMd, &lambdaRd, &tuSadThreshold);

    curbe.DW3_ModeCost_0             = m_modeCost[0];
    curbe.DW3_ModeCost_1             = m_modeCost[1];
    curbe.DW3_ModeCost_2             = m_modeCost[2];
    curbe.DW3_ModeCost_3             = m_modeCost[3];

    curbe.DW4_ModeCost_4             = m_modeCost[4];
    curbe.DW4_ModeCost_5             = m_modeCost[5];
    curbe.DW4_ModeCost_6             = m_modeCost[6];
    curbe.DW4_ModeCost_7             = m_modeCost[7];

    curbe.DW5_ModeCost_8             = m_modeCost[8];
    curbe.DW5_ModeCost_9             = m_modeCost[9];
    curbe.DW5_RefIDCost              = m_modeCost[10];
    curbe.DW5_ChromaIntraModeCost    = m_modeCost[11];

    uint8_t tuMapping                = ((m_hevcSeqParams->TargetUsage + 1) / 4) % 3;  // Map TU 1,4,7 to 0,1,2
    curbe.DW6_Log2MaxCUSize          = m_hevcSeqParams->log2_max_coding_block_size_minus3 + 3;
    curbe.DW6_Log2MinCUSize          = m_hevcSeqParams->log2_min_coding_block_size_minus3 + 3;
    curbe.DW6_Log2MaxTUSize          = m_hevcSeqParams->log2_max_transform_block_size_minus2 + 2;
    curbe.DW6_Log2MinTUSize          = m_hevcSeqParams->log2_min_transform_block_size_minus2 + 2;
    curbe.DW6_MaxTransformDepthIntra = m_hevcSeqParams->max_transform_hierarchy_depth_intra ? m_tuSettings[Log2TUMaxDepthIntraTuParam][tuMapping] : 0;  // 2 is not supported in Enc

    curbe.DW7_ConcurrentGroupNum     = 1;
    curbe.DW7_SliceQP                = sliceQp;

    curbe.DW8_Lambda_Rd              = lambdaRd;
    curbe.DW9_Lambda_Md              = lambdaMd;
    curbe.DW10_IntraTuDThres         = tuSadThreshold;

    curbe.DW11_SliceType             = CODECHAL_ENCODE_HEVC_I_SLICE;

    if (m_hevcSeqParams->RateControlMethod == RATECONTROL_CQP)
    {
        curbe.DW11_QPType            = QP_TYPE_CONSTANT;
    }
    else
    {
        curbe.DW11_QPType = m_lcuBrcEnabled ? QP_TYPE_CU_LEVEL : QP_TYPE_FRAME;
    }

    //TU based settings
    curbe.DW7_EncTuDecisionMode      = m_tuSettings[EncTuDecisionModeTuParam][tuMapping];
    curbe.DW11_EncQtDecisionMode     = m_tuSettings[EncQtDecisionModeTuParam][tuMapping];

    curbe.DW16_BTI_VmeIntraPredictionSurface        = MBENC_I_FRAME_VME_PRED_CURR_PIC_IDX0;
    curbe.DW17_BTI_CurrentPictureY                  = MBENC_I_FRAME_CURR_Y;
    curbe.DW18_BTI_EncCuRecordSurface               = MBENC_I_FRAME_INTERMEDIATE_CU_RECORD;
    curbe.DW19_BTI_PakObjectCommandSurface          = MBENC_I_FRAME_PAK_OBJ;
    curbe.DW20_BTI_CuPacketForPakSurface            = MBENC_I_FRAME_PAK_CU_RECORD;
    curbe.DW21_BTI_InternalScratchSurface           = MBENC_I_FRAME_SCRATCH_SURFACE;
    curbe.DW22_BTI_CuBasedQpSurface                 = MBENC_I_FRAME_CU_QP_DATA;
    curbe.DW23_BTI_ConstantDataLutSurface           = MBENC_I_FRAME_CONST_DATA_LUT;
    curbe.DW24_BTI_LcuLevelDataInputSurface         = MBENC_I_FRAME_LCU_LEVEL_DATA_INPUT;
    curbe.DW25_BTI_ConcurrentThreadGroupDataSurface = MBENC_I_FRAME_CONCURRENT_TG_DATA;
    curbe.DW26_BTI_BrcCombinedEncParameterSurface   = MBENC_I_FRAME_BRC_COMBINED_ENC_PARAMETER_SURFACE;
    curbe.DW27_BTI_CuSplitSurface                   = MBENC_I_FRAME_CU_SPLIT_SURFACE,
    curbe.DW28_BTI_DebugSurface                     = MBENC_I_FRAME_DEBUG_DUMP;

    PMHW_KERNEL_STATE kernelState = &m_mbEncKernelStates[MBENC_I_KRNIDX];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(kernelState->m_dshRegion.AddData(
        &curbe,
        kernelState->dwCurbeOffset,
        sizeof(curbe)));

    return eStatus;
}

// ------------------------------------------------------------------------------
//| Purpose:    Setup curbe for HEVC MbEnc B LCU32 and LCU64_32 Kernels
//| Return:     N/A
//------------------------------------------------------------------------------
MOS_STATUS CodechalEncHevcStateG10::SetCurbeMbEncBKernel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_mbEncKernelStates);

    // Initialize the CURBE data
    CODECHAL_ENC_HEVC_MBENC_B_CURBE_G10 curbe = m_mbencBCurbeInit;

    curbe.DW0_FrameWidthInSamples                     = m_frameWidth;
    curbe.DW0_FrameHeightInSamples                    = m_frameHeight;

    uint8_t tuMapping                = ((m_hevcSeqParams->TargetUsage + 1) / 4) % 3;  // Map TU 1,4,7 to 0,1,2
    curbe.DW1_Log2MaxCUSize          = m_hevcSeqParams->log2_max_coding_block_size_minus3 + 3;
    curbe.DW1_Log2MinCUSize          = m_hevcSeqParams->log2_min_coding_block_size_minus3 + 3;
    curbe.DW1_Log2MaxTUSize          = m_hevcSeqParams->log2_max_transform_block_size_minus2 + 2;
    curbe.DW1_Log2MinTUSize          = m_hevcSeqParams->log2_min_transform_block_size_minus2 + 2;
    curbe.DW1_MaxTransformDepthInter = m_hevcSeqParams->max_transform_hierarchy_depth_inter ? m_tuSettings[Log2TUMaxDepthInterTuParam][tuMapping] : 0;
    curbe.DW1_MaxTransformDepthIntra = m_hevcSeqParams->max_transform_hierarchy_depth_intra ? m_tuSettings[Log2TUMaxDepthIntraTuParam][tuMapping] : 0;  // 2 is not supported in Enc

    curbe.DW2_HMECoarseShape                          = 1;
    curbe.DW2_HMESubPelMode                           = 3;
    curbe.DW2_RegionsInSliceEnable                    = (m_numRegionsInSlice > 1);

    if (m_hevcSeqParams->RateControlMethod == RATECONTROL_CQP)
    {
        curbe.DW2_QpType            = QP_TYPE_CONSTANT;
    }
    else
    {
        curbe.DW2_QpType = m_lcuBrcEnabled ? QP_TYPE_CU_LEVEL : QP_TYPE_FRAME;
    }

    curbe.DW2_HMEFlag                                 = m_hmeSupported ? 0x3 : 0;
    curbe.DW2_SuperHME                                = m_16xMeSupported;

    // activeNumChildThreads params are used to control the active number of child threads per CU64/CU32 to fine-tune the performance on silicon.
    // Set zero (not used) by default for the Beta6 release.
    curbe.DW3_ActiveNumChildThreads_CU64              = 0;
    curbe.DW3_ActiveNumChildThreads_CU32_0            = 0;
    curbe.DW3_ActiveNumChildThreads_CU32_1            = 0;
    curbe.DW3_ActiveNumChildThreads_CU32_2            = 0;
    curbe.DW3_ActiveNumChildThreads_CU32_3            = 0;
    curbe.DW3_SliceQp                                 = m_hevcPicParams->QpY + m_hevcSliceParams->slice_qp_delta;

    curbe.DW4_SkipModeEn                              = 1;
    curbe.DW4_HEVCMinCUControl                        = m_hevcSeqParams->log2_min_coding_block_size_minus3;

    curbe.DW7_SliceType                               = CODECHAL_ENCODE_HEVC_B_SLICE;
    curbe.DW7_TemporalMvpEnableFlag                   = m_hevcSeqParams->sps_temporal_mvp_enable_flag;
    curbe.DW7_CollocatedFromL0Flag                    = m_hevcSliceParams->collocated_from_l0_flag;
    curbe.DW7_TheSameRefList                          = m_sameRefList;
    curbe.DW7_IsLowDelay                              = m_lowDelay;
    curbe.DW7_MaxNumMergeCand                         = m_hevcSliceParams->MaxNumMergeCand;
    curbe.DW7_NumRefIdxL0                             = m_hevcSliceParams->num_ref_idx_l0_active_minus1 + 1;
    curbe.DW7_NumRefIdxL1                             = m_hevcSliceParams->num_ref_idx_l1_active_minus1 + 1;

    curbe.DW8_FwdPocNumber_L0_mTb_0 = ComputeTemporalDifferent(m_hevcSliceParams->RefPicList[0][0]);
    curbe.DW8_BwdPocNumber_L1_mTb_0 = ComputeTemporalDifferent(m_hevcSliceParams->RefPicList[1][0]);
    curbe.DW8_FwdPocNumber_L0_mTb_1 = ComputeTemporalDifferent(m_hevcSliceParams->RefPicList[0][1]);
    curbe.DW8_BwdPocNumber_L1_mTb_1 = ComputeTemporalDifferent(m_hevcSliceParams->RefPicList[1][1]);
    curbe.DW9_FwdPocNumber_L0_mTb_2 = ComputeTemporalDifferent(m_hevcSliceParams->RefPicList[0][2]);
    curbe.DW9_BwdPocNumber_L1_mTb_2 = ComputeTemporalDifferent(m_hevcSliceParams->RefPicList[1][2]);
    curbe.DW9_FwdPocNumber_L0_mTb_3 = ComputeTemporalDifferent(m_hevcSliceParams->RefPicList[0][3]);
    curbe.DW9_BwdPocNumber_L1_mTb_3 = ComputeTemporalDifferent(m_hevcSliceParams->RefPicList[1][3]);

    curbe.DW13_RefFrameHorizontalSize                 = m_frameWidth;
    curbe.DW13_RefFrameVerticalSize                   = m_frameHeight;

    curbe.DW15_ConcurrentGroupNum                     = m_walkingPatternParam.dwNumRegion;
    curbe.DW15_TotalThreadNumPerLCU                   = m_totalNumThreadsPerLcu;
    curbe.DW15_NumRegions                             = m_numRegionsInSlice;

    // TU based settings
    curbe.DW1_MaxNumIMESearchCenter                   = m_tuSettings[MaxNumIMESearchCenterTuParam][tuMapping];
    curbe.DW2_EnableCu64Check                         = m_isMaxLcu64 ? m_tuSettings[EnableCu64CheckTuParam][tuMapping] : 0;
    curbe.DW2_EncTransformSimplify                    = m_tuSettings[EncTransformSimplifyTuParam][tuMapping];
    curbe.DW2_EncTuDecisionMode                       = m_tuSettings[EncTuDecisionModeTuParam][tuMapping];
    curbe.DW2_EncTuDecisionForAllQt                   = m_tuSettings[EncTuDecisionForAllQtTuParam][tuMapping];
    curbe.DW2_CoefBitEstMode                          = m_tuSettings[CoefBitEstModeTuParam][tuMapping];
    curbe.DW2_EncSkipDecisionMode                     = m_tuSettings[EncSkipDecisionModeTuParam][tuMapping];
    curbe.DW2_EncQtDecisionMode                       = m_tuSettings[EncQtDecisionModeTuParam][tuMapping];
    curbe.DW2_LCU32_EncRdDecisionModeForAllQt         = m_tuSettings[EncRdDecisionModeForAllQtTuParam][tuMapping];
    curbe.DW2_LCU64_Cu64SkipCheckOnly                 = (tuMapping == 1);  // 1 for TU4, 0 for others
    curbe.DW2_SICDynamicRunPathMode                   = m_tuSettings[SICDynamicRunPathMode][tuMapping];  // 0,1 for mode 1, 2 for mode 2, 3 for mode 3 --- TU1,TU4 :0 ; TU7: 2 -- as per beta6

    if (m_isMaxLcu64)
    {
        curbe.DW16_BTI_CurrentPictureY                          = MBENC_B_FRAME_LCU64_CURR_Y;
        curbe.DW17_BTI_EncCuRecordSurface                       = MBENC_B_FRAME_LCU64_CU32_ENC_CU_RECORD;
        curbe.DW18_BTI_LCU64_SecEncCuRecordSurface              = MBENC_B_FRAME_LCU64_SECOND_CU32_ENC_CU_RECORD;
        curbe.DW19_BTI_LCU64_PAKObjectCommandSurface            = MBENC_B_FRAME_LCU64_PAK_OBJ;
        curbe.DW20_BTI_LCU64_PAKCURecordSurface                 = MBENC_B_FRAME_LCU64_PAK_CU_RECORD;
        curbe.DW21_BTI_LCU64_VMEIntra_InterPredictionSurface    = MBENC_B_FRAME_LCU64_VME_PRED_CURR_PIC_IDX0;
        curbe.DW22_BTI_LCU64_CU16x16QpDataInputSurface          = MBENC_B_FRAME_LCU64_CU16x16_QP_DATA;
        curbe.DW23_BTI_LCU64_CU32_HEVCEncConstantTableSurface   = MBENC_B_FRAME_LCU64_CU32_ENC_CONST_TABLE;
        curbe.DW24_BTI_LCU64_ColocatedCUMotionVectorDataSurface = MBENC_B_FRAME_LCU64_COLOCATED_CU_MV_DATA;
        curbe.DW25_BTI_LCU64_HmeMotionPredictorDataSurface      = MBENC_B_FRAME_LCU64_HME_MOTION_PREDICTOR_DATA;
        curbe.DW26_BTI_LCU64_LcuLevelDataInputSurface           = MBENC_B_FRAME_LCU64_LCU_LEVEL_DATA_INPUT;
        curbe.DW27_BTI_LCU64_CU32_LcuEncodingScratchSurface     = MBENC_B_FRAME_LCU64_CU32_LCU_ENC_SCRATCH_SURFACE;
        curbe.DW28_BTI_LCU64_64x64_DistortionSurface            = MBENC_B_FRAME_LCU64_64X64_DISTORTION_SURFACE;
        curbe.DW29_BTI_LCU64_ConcurrentThreadGroupDataSurface   = MBENC_B_FRAME_LCU64_CONCURRENT_TG_DATA;
        curbe.DW30_BTI_LCU64_BrcCombinedEncParameterSurface     = MBENC_B_FRAME_LCU64_BRC_COMBINED_ENC_PARAMETER_SURFACE;
        curbe.DW31_BTI_LCU64_CU32_JobQueue1DBufferSurface       = MBENC_B_FRAME_LCU64_CU32_JOB_QUEUE_1D_SURFACE;
        curbe.DW32_BTI_LCU64_CU32_JobQueue2DBufferSurface       = MBENC_B_FRAME_LCU64_CU32_JOB_QUEUE_2D_SURFACE;
        curbe.DW33_BTI_LCU64_CU32_ResidualDataScratchSurface    = MBENC_B_FRAME_LCU64_CU32_RESIDUAL_DATA_SCRATCH_SURFACE;
        curbe.DW34_BTI_LCU64_CuSplitSurface                     = MBENC_B_FRAME_LCU64_CU_SPLIT_DATA_SURFACE;
        curbe.DW35_BTI_LCU64_CurrentPictureY2xDS                = MBENC_B_FRAME_LCU64_CURR_Y_2xDS;
        curbe.DW36_BTI_LCU64_IntermediateCuRecordSurface        = MBENC_B_FRAME_LCU64_INTERMEDIATE_CU_RECORD;
        curbe.DW37_BTI_Lcu64_ConstantDataLutSurface             = MBENC_B_FRAME_LCU64_CONST64_DATA_LUT;
        curbe.DW38_BTI_LCU64_LcuDataStorageSurface              = MBENC_B_FRAME_LCU64_LCU_STORAGE_SURFACE;
        curbe.DW39_BTI_LCU64_VmeInterPredictionSurface2xDS      = MBENC_B_FRAME_LCU64_VME_PRED_CURR_PIC_2xDS_IDX0;
        curbe.DW40_BTI_LCU64_JobQueue1DBufferSurface            = MBENC_B_FRAME_LCU64_JOB_QUEUE_1D_SURFACE;
        curbe.DW41_BTI_LCU64_JobQueue2DBufferSurface            = MBENC_B_FRAME_LCU64_JOB_QUEUE_2D_SURFACE;
        curbe.DW42_BTI_LCU64_ResidualDataScratchSurface         = MBENC_B_FRAME_LCU64_RESIDUAL_DATA_SCRATCH_SURFACE;
        curbe.DW43_BTI_LCU64_DebugFeatureSurface                = MBENC_B_FRAME_LCU64_DEBUG_SURFACE;
    }
    else
    {
        curbe.DW16_BTI_CurrentPictureY                          = MBENC_B_FRAME_LCU32_CURR_Y;
        curbe.DW17_BTI_EncCuRecordSurface                       = MBENC_B_FRAME_LCU32_ENC_CU_RECORD;
        curbe.DW18_BTI_LCU32_PAKObjectCommandSurface            = MBENC_B_FRAME_LCU32_PAK_OBJ;
        curbe.DW19_BTI_LCU32_PAKCURecordSurface                 = MBENC_B_FRAME_LCU32_PAK_CU_RECORD;
        curbe.DW20_BTI_LCU32_VMEIntra_InterPredictionSurface    = MBENC_B_FRAME_LCU32_VME_PRED_CURR_PIC_IDX0;
        curbe.DW21_BTI_LCU32_CU16x16QpDataInputSurface          = MBENC_B_FRAME_LCU32_CU16x16_QP_DATA;
        curbe.DW22_BTI_LCU32_HEVCEncConstantTableSurface        = MBENC_B_FRAME_LCU32_ENC_CONST_TABLE;
        curbe.DW23_BTI_LCU32_ColocatedCUMotionVectorDataSurface = MBENC_B_FRAME_LCU32_COLOCATED_CU_MV_DATA;
        curbe.DW24_BTI_LCU32_HmeMotionPredictorDataSurface      = MBENC_B_FRAME_LCU32_HME_MOTION_PREDICTOR_DATA;
        curbe.DW25_BTI_LCU32_LcuLevelDataInputSurface           = MBENC_B_FRAME_LCU32_LCU_LEVEL_DATA_INPUT;
        curbe.DW26_BTI_LCU32_LcuEncodingScratchSurface          = MBENC_B_FRAME_LCU32_LCU_ENC_SCRATCH_SURFACE;
        curbe.DW27_BTI_LCU32_ConcurrentThreadGroupDataSurface   = MBENC_B_FRAME_LCU32_CONCURRENT_TG_DATA;
        curbe.DW28_BTI_LCU32_BrcCombinedEncParameterSurface     = MBENC_B_FRAME_LCU32_BRC_COMBINED_ENC_PARAMETER_SURFACE;
        curbe.DW29_BTI_LCU32_JobQueueScratchBufferSurface       = MBENC_B_FRAME_LCU32_JOB_QUEUE_SCRATCH_SURFACE;
        curbe.DW30_BTI_LCU32_CuSplitDataSurface                 = MBENC_B_FRAME_LCU32_CU_SPLIT_DATA_SURFACE,
        curbe.DW31_BTI_LCU32_ResidualDataScratchSurface         = MBENC_B_FRAME_LCU32_RESIDUAL_DATA_SCRATCH_SURFACE,
        curbe.DW32_BTI_LCU32_DebugSurface                       = MBENC_B_FRAME_LCU32_DEBUG_SURFACE;
    }

    PMHW_KERNEL_STATE kernelState = m_isMaxLcu64 ? &m_mbEncKernelStates[MBENC_B_LCU64_KRNIDX] : &m_mbEncKernelStates[MBENC_B_LCU32_KRNIDX];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(kernelState->m_dshRegion.AddData(
        &curbe,
        kernelState->dwCurbeOffset,
        sizeof(curbe)));

    return eStatus;
}

//------------------------------------------------------------------------------------
// Send surfaces for the scaling kernel
//------------------------------------------------------------------------------------
MOS_STATUS CodechalEncHevcStateG10::SendScalingAndConversionSurfaces(
    PMOS_COMMAND_BUFFER cmdBuffer, SurfaceParamsDsConv* params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params);

    uint32_t startBti = 0;
    CODECHAL_SURFACE_CODEC_PARAMS surfaceCodecParams;

    PMHW_KERNEL_STATE kernelState = params->pKernelState;
    CODECHAL_ENCODE_CHK_NULL_RETURN(kernelState);

    PCODECHAL_ENCODE_BINDING_TABLE_GENERIC bindingTable = params->pBindingTable;
    CODECHAL_ENCODE_CHK_NULL_RETURN(bindingTable);

    MOS_SURFACE inputSurface = *params->psInputSurface;
    inputSurface.dwWidth = params->dwInputFrameWidth;
    inputSurface.dwHeight = params->dwInputFrameHeight;
    inputSurface.UPlaneOffset.iYOffset = inputSurface.dwHeight;

    if(params->downScaleConversionType & convFromOrig)
    {
        params->psOutputConvertedSurface->dwWidth                 = params->dwOutputConvertedFrameWidth;
        params->psOutputConvertedSurface->dwHeight                = params->dwOutputConvertedFrameHeight;
        params->psOutputConvertedSurface->UPlaneOffset.iYOffset   = params->psOutputConvertedSurface->dwHeight;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
            &surfaceCodecParams,
            &inputSurface,
            m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_HME_DOWNSAMPLED_ENCODE].Value,
            bindingTable->dwBindingTableEntries[startBti++],
            0,
            false));

        surfaceCodecParams.bUseUVPlane              = true;
        surfaceCodecParams.dwUVBindingTableOffset   = bindingTable->dwBindingTableEntries[startBti++];
        surfaceCodecParams.bUse32UnormSurfaceFormat = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));

        //Source Y and UV
        CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
            &surfaceCodecParams,
            params->psOutputConvertedSurface,
            m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_HME_DOWNSAMPLED_ENCODE].Value,
            bindingTable->dwBindingTableEntries[startBti++],
            0,
            true));
        surfaceCodecParams.bUseUVPlane              = true;
        surfaceCodecParams.dwUVBindingTableOffset   = bindingTable->dwBindingTableEntries[startBti++];
        surfaceCodecParams.bUse32UnormSurfaceFormat = false;
        surfaceCodecParams.bUse16UnormSurfaceFormat = false;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));

    }
    else
    {
        // Increment the binding table index
        startBti += 2;

        //Source Y and UV
        CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
            &surfaceCodecParams,
            &inputSurface,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_HME_DOWNSAMPLED_ENCODE].Value,
            bindingTable->dwBindingTableEntries[startBti++],
            0,
            false));

        surfaceCodecParams.bUseUVPlane = !(params->downScaleConversionType & ds16xFromOrig); // UV plane not available for 16x DS
        surfaceCodecParams.dwUVBindingTableOffset   = bindingTable->dwBindingTableEntries[startBti++];
        surfaceCodecParams.bUse32UnormSurfaceFormat = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    switch (DsStage(params->downScaleConversionType & ~convFromOrig))
    {
    case ds4xFromOrig:
    case ds2x4xFromOrig:
    case ds16xFromOrig:
    {
        PMOS_SURFACE scaledSurface4x = params->psOutputScaledSurface4x;
        scaledSurface4x->dwWidth = params->dwOutputScaledFrameWidth4x;
        scaledSurface4x->dwHeight = params->dwOutputScaledFrameHeight4x;

        //Dest Y
        CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
            &surfaceCodecParams,
            scaledSurface4x,
            m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_HME_DOWNSAMPLED_ENCODE_DST].Value,
            bindingTable->dwBindingTableEntries[startBti++],
            0,
            true));

        surfaceCodecParams.bUse32UnormSurfaceFormat = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
        break;
    }
    case ds2xFromOrig:
    case dsConvUnknown:

        startBti++;
        break;

    default:
        CODECHAL_ENCODE_ASSERTMESSAGE("Unsupported Scaling/Conversion type");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    // MB stats surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
        &surfaceCodecParams,
        &m_mbStatisticsSurface,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_HME_DOWNSAMPLED_ENCODE_DST].Value,
        bindingTable->dwBindingTableEntries[startBti++],
        0,
        true));

    surfaceCodecParams.bRawSurface = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    if(params->downScaleConversionType & ds2xFromOrig)
    {
        PMOS_SURFACE scaledSurface2x = params->psOutputScaledSurface2x;
        scaledSurface2x->dwWidth = params->dwOutputScaledFrameWidth2x;
        scaledSurface2x->dwHeight = params->dwOutputScaledFrameHeight2x;

        // 2xDS Dest Y
        CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
            &surfaceCodecParams,
            scaledSurface2x,
            m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_HME_DOWNSAMPLED_ENCODE_DST].Value,
            bindingTable->dwBindingTableEntries[startBti++],
            0,
            true));

        surfaceCodecParams.bUse32UnormSurfaceFormat = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }
    else
    {
        startBti++;
    }

    // MB split surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
            &surfaceCodecParams,
            &m_mbSplitSurface,
            m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_HME_DOWNSAMPLED_ENCODE].Value,
            bindingTable->dwBindingTableEntries[startBti++],
            0,
            true));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Job Queue Header Surface for multi-threading for LCU32 B-kernel
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        &surfaceCodecParams,
        &m_jobQueueHeaderSurfaceForB.sResource,
        MOS_BYTES_TO_DWORDS(m_jobQueueHeaderSurfaceForB.dwSize),
        0,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_HME_DOWNSAMPLED_ENCODE].Value,
        bindingTable->dwBindingTableEntries[startBti++],
        true));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Job Queue Header Surface for multi-threading for LCU64 B-kernel
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        &surfaceCodecParams,
        &m_jobQueueHeaderSurfaceForBLcu64.sResource,
        MOS_BYTES_TO_DWORDS(m_jobQueueHeaderSurfaceForBLcu64.dwSize),
        0,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_HME_DOWNSAMPLED_ENCODE].Value,
        bindingTable->dwBindingTableEntries[startBti++],
        true));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Distortion surface for 64x64
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        &surfaceCodecParams,
        &m_64x64DistortionSurface.sResource,
        MOS_BYTES_TO_DWORDS(m_64x64DistortionSurface.dwSize),
        0,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_HME_DOWNSAMPLED_ENCODE].Value,
        bindingTable->dwBindingTableEntries[startBti++],
        true));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG10::SendMeSurfaces(
    PMOS_COMMAND_BUFFER                 cmdBuffer,
    HmeLevel                            hmeLevel,
    HEVC_ME_DIST_TYPE                   distType)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_meKernelState);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_meKernelBindingTable);

    PMOS_SURFACE currScaledSurface = nullptr, meMvDataBuffer = nullptr;
    bool use16xMvInputMvDataFor4x = false, is4xHmeStage = false;
    switch(hmeLevel)
    {
        case HME_LEVEL_4x:
            currScaledSurface     = m_trackedBuf->Get4xDsSurface(CODEC_CURR_TRACKED_BUFFER);
            meMvDataBuffer           = &m_s4XMeMvDataBuffer;
            use16xMvInputMvDataFor4x = m_b16XMeEnabled;
            is4xHmeStage             = true;
            break;
        case HME_LEVEL_16x:
            currScaledSurface     = m_trackedBuf->Get16xDsSurface(CODEC_CURR_TRACKED_BUFFER);
            meMvDataBuffer        = &m_s16XMeMvDataBuffer;
            break;
        default:
            CODECHAL_ENCODE_ASSERTMESSAGE("Unsupported HME level");
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            return eStatus;
    }

    uint32_t startBti = 0;
    PMHW_KERNEL_STATE                      kernelState  = m_meKernelState;
    PCODECHAL_ENCODE_BINDING_TABLE_GENERIC bindingTable = m_meKernelBindingTable;
    CODECHAL_SURFACE_CODEC_PARAMS surfaceCodecParams;

    // HME Output MV Data Surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
        &surfaceCodecParams,
        meMvDataBuffer,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value,
        bindingTable->dwBindingTableEntries[startBti++],
        0,
        true));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    if(use16xMvInputMvDataFor4x)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
            &surfaceCodecParams,
            &m_s16XMeMvDataBuffer,
            m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value,
            bindingTable->dwBindingTableEntries[startBti++],
            0,
            false));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }
    else
    {
        startBti++;
    }

    if(is4xHmeStage)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
            &surfaceCodecParams,
            &m_s4XMeDistortionBuffer,
            m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ME_DISTORTION_ENCODE].Value,
            bindingTable->dwBindingTableEntries[startBti++],
            0,
            true));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }
    else
    {
        startBti++;
    }

    // VME surfaces
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParamsVME(
        &surfaceCodecParams,
        currScaledSurface,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value,
        bindingTable->dwBindingTableEntries[startBti++]));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    for(auto surface_idx = 0; surface_idx < 4; surface_idx++)
    {
        CODEC_PICTURE refPic = m_hevcSliceParams->RefPicList[LIST_0][surface_idx];
        if (!CodecHal_PictureIsInvalid(refPic) && m_picIdx[refPic.FrameIdx].bValid)
        {
            uint8_t refPicIdx = m_picIdx[refPic.FrameIdx].ucPicIdx;
            uint8_t scaledIdx = m_refList[refPicIdx]->ucScalingIdx;

            // Picture Y VME
            CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParamsVME(
                &surfaceCodecParams,
                (hmeLevel == HME_LEVEL_4x) ? m_trackedBuf->Get4xDsSurface(scaledIdx) : m_trackedBuf->Get16xDsSurface(scaledIdx),
                m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value,
                bindingTable->dwBindingTableEntries[startBti++]));

            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                m_hwInterface,
                cmdBuffer,
                &surfaceCodecParams,
                kernelState));
        }
        else
        {
            // Providing Dummy surface as per VME requirement.
            CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParamsVME(
                &surfaceCodecParams,
                currScaledSurface,
                m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value,
                bindingTable->dwBindingTableEntries[startBti++]));

            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                m_hwInterface,
                cmdBuffer,
                &surfaceCodecParams,
                kernelState));
        }

        refPic = m_hevcSliceParams->RefPicList[LIST_1][surface_idx];
        if (!CodecHal_PictureIsInvalid(refPic) &&
            !CodecHal_PictureIsInvalid(m_hevcPicParams->RefFrameList[refPic.FrameIdx]))
        {
            uint8_t refPicIdx = m_picIdx[refPic.FrameIdx].ucPicIdx;
            uint8_t scaledIdx = m_refList[refPicIdx]->ucScalingIdx;

            // Picture Y VME
            CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParamsVME(
                &surfaceCodecParams,
                (hmeLevel == HME_LEVEL_4x) ? m_trackedBuf->Get4xDsSurface(scaledIdx) : m_trackedBuf->Get16xDsSurface(scaledIdx),
                m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value,
                bindingTable->dwBindingTableEntries[startBti++]));

            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                m_hwInterface,
                cmdBuffer,
                &surfaceCodecParams,
                kernelState));
        }
        else
        {
            // Providing Dummy surface as per VME requirement.
            CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParamsVME(
                &surfaceCodecParams,
                currScaledSurface,
                m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value,
                bindingTable->dwBindingTableEntries[startBti++]));

            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                m_hwInterface,
                cmdBuffer,
                &surfaceCodecParams,
                kernelState));
        }
    }

    if(is4xHmeStage)
    {
        // 4x Downscaled Surface
        CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
            &surfaceCodecParams,
            currScaledSurface,
            m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value,
            bindingTable->dwBindingTableEntries[startBti++],
            0,
            false));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));

        // BRC Distortion Surface
        CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
            &surfaceCodecParams,
            distType ? &m_brcBuffers.sMeBrcDistortionBuffer : &m_brcBuffers.sBrcIntraDistortionBuffer,
            m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_BRC_ME_DISTORTION_ENCODE].Value,
            bindingTable->dwBindingTableEntries[startBti++],
            0,
            true));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    // Mv and Distortion summation surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        &surfaceCodecParams,
        &m_mvAndDistortionSumSurface.sResource,
        m_mvAndDistortionSumSurface.dwSize,
        0,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
        bindingTable->dwBindingTableEntries[startBti++],
        true));

    surfaceCodecParams.bRawSurface = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG10::SendBrcInitResetSurfaces(
    PMOS_COMMAND_BUFFER         cmdBuffer,
    CODECHAL_HEVC_BRC_KRNIDX    krnIdx)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_ASSERT(krnIdx == CODECHAL_HEVC_BRC_INIT || krnIdx == CODECHAL_HEVC_BRC_RESET);

    CODECHAL_SURFACE_CODEC_PARAMS surfaceCodecParams;
    uint32_t startBti = 0;
    PMHW_KERNEL_STATE                      kernelState  = &m_brcKernelStates[krnIdx];
    PCODECHAL_ENCODE_BINDING_TABLE_GENERIC bindingTable = &m_brcKernelBindingTable[krnIdx];

    // BRC History Buffer
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        &surfaceCodecParams,
        &m_brcBuffers.resBrcHistoryBuffer,
        MOS_BYTES_TO_DWORDS(m_brcHistoryBufferSize),
        0,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
        bindingTable->dwBindingTableEntries[startBti++],
        true));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // BRC Distortion Surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
        &surfaceCodecParams,
        &m_brcBuffers.sMeBrcDistortionBuffer,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_BRC_ME_DISTORTION_ENCODE].Value,
        bindingTable->dwBindingTableEntries[startBti++],
        0,
        true));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG10::SendBrcFrameUpdateSurfaces(PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    PMOS_RESOURCE brcHcpStateReadBuffer = &m_brcBuffers.resBrcImageStatesReadBuffer[m_currRecycledBufIdx];
    PMOS_SURFACE  brcConstantData       = &m_brcBuffers.sBrcConstantDataBuffer[m_currRecycledBufIdx];

    uint32_t startBti = 0;
    CODECHAL_SURFACE_CODEC_PARAMS surfaceCodecParams;
    PMHW_KERNEL_STATE                      kernelState  = &m_brcKernelStates[CODECHAL_HEVC_BRC_FRAME_UPDATE];
    PCODECHAL_ENCODE_BINDING_TABLE_GENERIC bindingTable = &m_brcKernelBindingTable[CODECHAL_HEVC_BRC_FRAME_UPDATE];

    // BRC History Buffer
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        &surfaceCodecParams,
        &m_brcBuffers.resBrcHistoryBuffer,
        MOS_BYTES_TO_DWORDS(m_brcHistoryBufferSize),
        0,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
        bindingTable->dwBindingTableEntries[startBti++],
        true));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // BRC Prev PAK statistics output buffer
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        &surfaceCodecParams,
        &m_brcBuffers.resBrcPakStatisticBuffer[m_brcBuffers.uiCurrBrcPakStasIdxForRead],
        MOS_BYTES_TO_DWORDS(m_hevcBrcPakStatisticsSize),
        0,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
        bindingTable->dwBindingTableEntries[startBti++],
        false));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // BRC HCP_PIC_STATE read
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        &surfaceCodecParams,
        brcHcpStateReadBuffer,
        MOS_BYTES_TO_DWORDS(m_brcBuffers.dwBrcHcpPicStateSize),
        0,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
        bindingTable->dwBindingTableEntries[startBti++],
        false));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // BRC HCP_PIC_STATE write
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        &surfaceCodecParams,
        &m_brcBuffers.resBrcImageStatesWriteBuffer[m_currRecycledBufIdx],
        MOS_BYTES_TO_DWORDS(m_brcBuffers.dwBrcHcpPicStateSize),
        0,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
        bindingTable->dwBindingTableEntries[startBti++],
        true));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Combined ENC-parameter buffer
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        &surfaceCodecParams,
        (MOS_RESOURCE*)m_allocator->GetResource(m_standard, brcInputForEncKernel),
        MOS_BYTES_TO_DWORDS(m_allocator->GetResourceSize(m_standard, brcInputForEncKernel)),
        0,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
        bindingTable->dwBindingTableEntries[startBti++],
        true));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // BRC Distortion Surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
        &surfaceCodecParams,
        &m_brcBuffers.sMeBrcDistortionBuffer,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_BRC_ME_DISTORTION_ENCODE].Value,
        bindingTable->dwBindingTableEntries[startBti++],
        0,
        true));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // BRC Data Surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
        &surfaceCodecParams,
        brcConstantData,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
        bindingTable->dwBindingTableEntries[startBti++],
        0,
        false));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Pixel MB Statistics surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
        &surfaceCodecParams,
        &m_mbStatisticsSurface,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
        bindingTable->dwBindingTableEntries[startBti++],
        0,
        false));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Mv and Distortion summation surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        &surfaceCodecParams,
        &m_mvAndDistortionSumSurface.sResource,
        MOS_BYTES_TO_DWORDS(m_mvAndDistortionSumSurface.dwSize),
        0,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
        bindingTable->dwBindingTableEntries[startBti++],
        false));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG10::SendBrcLcuUpdateSurfaces(PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    uint32_t startBti = 0;
    CODECHAL_SURFACE_CODEC_PARAMS surfaceCodecParams;
    PMHW_KERNEL_STATE                      kernelState  = &m_brcKernelStates[CODECHAL_HEVC_BRC_LCU_UPDATE];
    PCODECHAL_ENCODE_BINDING_TABLE_GENERIC bindingTable = &m_brcKernelBindingTable[CODECHAL_HEVC_BRC_LCU_UPDATE];

    // BRC History Buffer
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        &surfaceCodecParams,
        &m_brcBuffers.resBrcHistoryBuffer,
        MOS_BYTES_TO_DWORDS(m_brcHistoryBufferSize),
        0,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
        bindingTable->dwBindingTableEntries[startBti++],
        true));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // BRC Distortion Surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
        &surfaceCodecParams,
        &m_brcBuffers.sMeBrcDistortionBuffer,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_BRC_ME_DISTORTION_ENCODE].Value,
        bindingTable->dwBindingTableEntries[startBti++],
        0,
        true));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Pixel MB Statistics surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
        &surfaceCodecParams,
        &m_mbStatisticsSurface,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
        bindingTable->dwBindingTableEntries[startBti++],
        0,
        false));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // MB QP surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
        &surfaceCodecParams,
        &m_brcBuffers.sBrcMbQpBuffer,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
        bindingTable->dwBindingTableEntries[startBti++],
        0,
        true));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // MB Split surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
        &surfaceCodecParams,
        &m_mbSplitSurface,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
        bindingTable->dwBindingTableEntries[startBti++],
        0,
        false));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Intra DISTORTION surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
        &surfaceCodecParams,
        &m_brcBuffers.sBrcIntraDistortionBuffer,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_BRC_ME_DISTORTION_ENCODE].Value,
        bindingTable->dwBindingTableEntries[startBti++],
        0,
        false));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // CU Split Surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
        &surfaceCodecParams,
        &m_cuSplitSurface,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_BRC_ME_DISTORTION_ENCODE].Value,
        bindingTable->dwBindingTableEntries[startBti++],
        0,
        true));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG10::SendMbEncSurfacesIKernel(PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    uint32_t startBti = 0;
    CODECHAL_SURFACE_CODEC_PARAMS surfaceCodecParams;
    PMHW_KERNEL_STATE                      kernelState  = &m_mbEncKernelStates[MBENC_I_KRNIDX];
    PCODECHAL_ENCODE_BINDING_TABLE_GENERIC bindingTable = &m_mbEncKernelBindingTable[MBENC_I_KRNIDX];
    PMOS_SURFACE                           inputSurface = m_is10BitHevc ? &m_formatConvertedSurface[0] : m_rawSurfaceToEnc;

    // VME surfaces
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParamsVME(
        &surfaceCodecParams,
        inputSurface,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value,
        bindingTable->dwBindingTableEntries[startBti++]));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Programming dummy surfaces even if not used (VME requirement), currently setting to input surface
    for (auto surface_idx = 0; surface_idx < 8; surface_idx++)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParamsVME(
            &surfaceCodecParams,
            inputSurface,
            m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value,
            bindingTable->dwBindingTableEntries[startBti++]));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    //Source Y and UV
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
        &surfaceCodecParams,
        inputSurface,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value,
        bindingTable->dwBindingTableEntries[startBti++],
        0,
        false));

    surfaceCodecParams.bUseUVPlane              = true;
    surfaceCodecParams.dwUVBindingTableOffset   = bindingTable->dwBindingTableEntries[startBti++];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Intermediate cu record surface -- changing Width and height to LCU32 aligned
    MOS_SURFACE tempSurface = m_intermediateCuRecordSurfaceLcu32;
    tempSurface.dwWidth     = m_widthAlignedLcu32;
    tempSurface.dwHeight    = m_heightAlignedLcu32 >> 1;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
        &surfaceCodecParams,
        &tempSurface,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_PAK_OBJECT_ENCODE].Value,
        bindingTable->dwBindingTableEntries[startBti++],
        0,
        true));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // PAK object command surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        &surfaceCodecParams,
        &m_resMbCodeSurface,
        MOS_BYTES_TO_DWORDS(m_mvOffset),
        0,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_PAK_OBJECT_ENCODE].Value,
        bindingTable->dwBindingTableEntries[startBti++],
        true));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // CU packet for PAK surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        &surfaceCodecParams,
        &m_resMbCodeSurface,
        MOS_BYTES_TO_DWORDS(m_mbCodeSize - m_mvOffset),
        m_mvOffset,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_PAK_OBJECT_ENCODE].Value,
        bindingTable->dwBindingTableEntries[startBti++],
        true));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Scratch Surface for Internal Use Only
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
        &surfaceCodecParams,
        &m_scratchSurface,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
        bindingTable->dwBindingTableEntries[startBti++],
        0,
        true));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // MB QP data input surface from Output of LCU BRC
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
        &surfaceCodecParams,
        &m_brcBuffers.sBrcMbQpBuffer,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
        bindingTable->dwBindingTableEntries[startBti++],
        0,
        false));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Enc I Constant Table Surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        &surfaceCodecParams,
        &m_encConstantTableForI.sResource,
        MOS_BYTES_TO_DWORDS(m_encConstantTableForI.dwSize),
        0,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
        bindingTable->dwBindingTableEntries[startBti++],
        false));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Lcu level data input
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        &surfaceCodecParams,
        &m_lcuLevelInputData.sResource,
        MOS_BYTES_TO_DWORDS(m_lcuLevelInputData.dwSize),
        0,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
        bindingTable->dwBindingTableEntries[startBti++],
        false));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Concurrent Thread Group Data Surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        &surfaceCodecParams,
        &m_concurrentThreadGroupData.sResource,
        MOS_BYTES_TO_DWORDS(m_concurrentThreadGroupData.dwSize),
        0,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
        bindingTable->dwBindingTableEntries[startBti++],
        false));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Brc Combined Enc parameter Surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        &surfaceCodecParams,
        (MOS_RESOURCE*)m_allocator->GetResource(m_standard, brcInputForEncKernel),
        MOS_BYTES_TO_DWORDS(m_allocator->GetResourceSize(m_standard, brcInputForEncKernel)),
        0,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
        bindingTable->dwBindingTableEntries[startBti++],
        false));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // CU Split Surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
        &surfaceCodecParams,
        &m_cuSplitSurface,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_BRC_ME_DISTORTION_ENCODE].Value,
        bindingTable->dwBindingTableEntries[startBti++],
        0,
        false));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Kernel debug surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        &surfaceCodecParams,
        &m_kernelDebug.sResource,
        MOS_BYTES_TO_DWORDS(m_kernelDebug.dwSize),
        0,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
        bindingTable->dwBindingTableEntries[startBti++],
        true));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG10::SendMbEncSurfacesBKernel(PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_mbEncKernelStates);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_mbEncKernelBindingTable);

    uint32_t startBti = 0;
    CODECHAL_SURFACE_CODEC_PARAMS surfaceCodecParams;
    PMOS_SURFACE                           inputSurface = m_is10BitHevc ? &m_formatConvertedSurface[0] : m_rawSurfaceToEnc;
    CODECHAL_ENC_HEVC_MBENC_KRNIDX_G10     krnIdx       = m_isMaxLcu64 ? MBENC_B_LCU64_KRNIDX : MBENC_B_LCU32_KRNIDX;
    PMHW_KERNEL_STATE                      kernelState  = &m_mbEncKernelStates[krnIdx];
    PCODECHAL_ENCODE_BINDING_TABLE_GENERIC bindingTable = &m_mbEncKernelBindingTable[krnIdx];

    //Source Y and UV
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
        &surfaceCodecParams,
        inputSurface,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value,
        bindingTable->dwBindingTableEntries[startBti++],
        0,
        false));

    surfaceCodecParams.bUseUVPlane = true;
    surfaceCodecParams.dwUVBindingTableOffset = bindingTable->dwBindingTableEntries[startBti++];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    MOS_SURFACE tempSurface = m_intermediateCuRecordSurfaceLcu32;

    if (!m_isMaxLcu64)
    {
        // Intermediate cu record surface -- changing Width and height to LCU32 aligned
        tempSurface.dwWidth  = m_widthAlignedLcu32;
        tempSurface.dwHeight = m_heightAlignedLcu32 >> 1;

        // Intermediate/Enc cu record surface
        CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
            &surfaceCodecParams,
            &tempSurface,
            m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_PAK_OBJECT_ENCODE].Value,
            bindingTable->dwBindingTableEntries[startBti++],
            0,
            true));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }
    else
    {
        // Intermediate/Enc cu record surface
        CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
            &surfaceCodecParams,
            &tempSurface,
            m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_PAK_OBJECT_ENCODE].Value,
            bindingTable->dwBindingTableEntries[startBti++],
            0,
            true));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));

        // Second Intermediate/Enc cu record surface
        CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
            &surfaceCodecParams,
            &m_secondIntermediateCuRecordSurfaceLcu32,
            m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_PAK_OBJECT_ENCODE].Value,
            bindingTable->dwBindingTableEntries[startBti++],
            0,
            true));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    // PAK object command surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        &surfaceCodecParams,
        &m_resMbCodeSurface,
        m_mvOffset,
        0,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_PAK_OBJECT_ENCODE].Value,
        bindingTable->dwBindingTableEntries[startBti++],
        true));

    surfaceCodecParams.bRawSurface = m_isMaxLcu64;
    surfaceCodecParams.dwSize      = surfaceCodecParams.bRawSurface ? surfaceCodecParams.dwSize : MOS_BYTES_TO_DWORDS(surfaceCodecParams.dwSize);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // CU packet for PAK surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        &surfaceCodecParams,
        &m_resMbCodeSurface,
        MOS_BYTES_TO_DWORDS(m_mbCodeSize - m_mvOffset),
        m_mvOffset,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_PAK_OBJECT_ENCODE].Value,
        bindingTable->dwBindingTableEntries[startBti++],
        true));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // VME surfaces
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParamsVME(
        &surfaceCodecParams,
        inputSurface,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value,
        bindingTable->dwBindingTableEntries[startBti++]));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    for(auto surface_idx = 0; surface_idx < 4; surface_idx++)
    {
        CODEC_PICTURE refPic = m_hevcSliceParams->RefPicList[LIST_0][surface_idx];
        if (!CodecHal_PictureIsInvalid(refPic) &&
            !CodecHal_PictureIsInvalid(m_hevcPicParams->RefFrameList[refPic.FrameIdx]))
        {
            uint8_t idx = m_hevcPicParams->RefFrameList[refPic.FrameIdx].FrameIdx;

            // Picture Y VME
            CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParamsVME(
                &surfaceCodecParams,
                &m_refList[idx]->sRefBuffer,
                m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value,
                bindingTable->dwBindingTableEntries[startBti++]));

            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                m_hwInterface,
                cmdBuffer,
                &surfaceCodecParams,
                kernelState));

            CODECHAL_DEBUG_TOOL(
                m_debugInterface->m_refIndex = (uint16_t)refPic.FrameIdx;
            std::string refSurfName = "RefSurf" + std::to_string(static_cast<uint32_t>(m_debugInterface->m_refIndex));
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
                &m_refList[idx]->sRefBuffer,
                CodechalDbgAttr::attrReferenceSurfaces,
                refSurfName.data())));

        }
        else
        {
            // Providing Dummy surface as per VME requirement.
            CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParamsVME(
                &surfaceCodecParams,
                inputSurface,
                m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value,
                bindingTable->dwBindingTableEntries[startBti++]));

            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                m_hwInterface,
                cmdBuffer,
                &surfaceCodecParams,
                kernelState));
        }

        refPic = m_hevcSliceParams->RefPicList[LIST_1][surface_idx];
        if (!CodecHal_PictureIsInvalid(refPic) &&
            !CodecHal_PictureIsInvalid(m_hevcPicParams->RefFrameList[refPic.FrameIdx]))
        {
            uint8_t idx = m_hevcPicParams->RefFrameList[refPic.FrameIdx].FrameIdx;

            // Picture Y VME
            CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParamsVME(
                &surfaceCodecParams,
                &m_refList[idx]->sRefBuffer,
                m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value,
                bindingTable->dwBindingTableEntries[startBti++]));

            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                m_hwInterface,
                cmdBuffer,
                &surfaceCodecParams,
                kernelState));

            CODECHAL_DEBUG_TOOL(
                m_debugInterface->m_refIndex = (uint16_t)refPic.FrameIdx;
            std::string refSurfName = "RefSurf" + std::to_string(static_cast<uint32_t>(m_debugInterface->m_refIndex));
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
                &m_refList[idx]->sRefBuffer,
                CodechalDbgAttr::attrReferenceSurfaces,
                refSurfName.data())));

        }
        else
        {
            // Providing Dummy surface as per VME requirement.
            CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParamsVME(
                &surfaceCodecParams,
                inputSurface,
                m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value,
                bindingTable->dwBindingTableEntries[startBti++]));

            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                m_hwInterface,
                cmdBuffer,
                &surfaceCodecParams,
                kernelState));
        }
    }

    // MB QP data input surface from Output of LCU BRC
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
        &surfaceCodecParams,
        &m_brcBuffers.sBrcMbQpBuffer,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
        bindingTable->dwBindingTableEntries[startBti++],
        0,
        false));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Enc B 32x32 Constant Table Surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        &surfaceCodecParams,
        &m_encConstantTableForB.sResource,
        MOS_BYTES_TO_DWORDS(m_encConstantTableForB.dwSize),
        0,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
        bindingTable->dwBindingTableEntries[startBti++],
        false));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Colocated CU Motion Vector Data Surface
    uint8_t mbCodeIdxForTempMVP = 0xFF;
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

    if(mbCodeIdxForTempMVP == 0xFF)
    {
        startBti++;
    }
    else
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
            &surfaceCodecParams,
            m_trackedBuf->GetMvTemporalBuffer(mbCodeIdxForTempMVP),
            MOS_BYTES_TO_DWORDS(m_sizeOfMvTemporalBuffer),
            0,
            m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value,
            bindingTable->dwBindingTableEntries[startBti++],
            false));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    // HME motion predictor data
    if(m_hmeSupported)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
            &surfaceCodecParams,
            &m_s4XMeMvDataBuffer,
            m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value,
            bindingTable->dwBindingTableEntries[startBti++],
            0,
            false));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }
    else
    {
        startBti++;
    }

    // Lcu level data input
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        &surfaceCodecParams,
        &m_lcuLevelInputData.sResource,
        MOS_BYTES_TO_DWORDS(m_lcuLevelInputData.dwSize),
        0,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
        bindingTable->dwBindingTableEntries[startBti++],
        false));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Lcu encoding scratch surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        &surfaceCodecParams,
        &m_lcuEncodingScratchSurface.sResource,
        MOS_BYTES_TO_DWORDS(m_lcuEncodingScratchSurface.dwSize),
        0,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
        bindingTable->dwBindingTableEntries[startBti++],
        true));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    if (m_isMaxLcu64)
    {
        // 64x64 Distortion Surface
        CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
            &surfaceCodecParams,
            &m_64x64DistortionSurface.sResource,
            m_64x64DistortionSurface.dwSize,
            0,
            m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
            bindingTable->dwBindingTableEntries[startBti++],
            true));

        surfaceCodecParams.bRawSurface = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    // Concurrent Thread Group Data Surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        &surfaceCodecParams,
        &m_concurrentThreadGroupData.sResource,
        MOS_BYTES_TO_DWORDS(m_concurrentThreadGroupData.dwSize),
        0,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
        bindingTable->dwBindingTableEntries[startBti++],
        false));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Brc Combined Enc parameter Surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        &surfaceCodecParams,
        (MOS_RESOURCE*)m_allocator->GetResource(m_standard, brcInputForEncKernel),
        MOS_BYTES_TO_DWORDS(m_allocator->GetResourceSize(m_standard, brcInputForEncKernel)),
        0,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
        bindingTable->dwBindingTableEntries[startBti++],
        false));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Job Queue Header buffer surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        &surfaceCodecParams,
        &m_jobQueueHeaderSurfaceForB.sResource,
        m_jobQueueHeaderSurfaceForB.dwSize,
        0,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
        bindingTable->dwBindingTableEntries[startBti++],
        true));

    surfaceCodecParams.bRawSurface = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    if (m_isMaxLcu64)
    {
        // Job Queue Data buffer surface for CU32
        CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
            &surfaceCodecParams,
            &m_jobQueueDataSurfaceForBLcu64Cu32,
            m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
            bindingTable->dwBindingTableEntries[startBti++],
            0,
            true));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));

        // Residual Data Scratch Surface LCU32
        CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
            &surfaceCodecParams,
            &m_residualDataScratchSurfaceForBLcu32,
            m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
            bindingTable->dwBindingTableEntries[startBti++],
            0,
            true));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));

        // CU Split Surface
        CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
            &surfaceCodecParams,
            &m_cuSplitSurface,
            m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
            bindingTable->dwBindingTableEntries[startBti++],
            0,
            false));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }
    else
    {
        // CU Split Surface
        CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
            &surfaceCodecParams,
            &m_cuSplitSurface,
            m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
            bindingTable->dwBindingTableEntries[startBti++],
            0,
            false));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));

        // Residual Data Scratch Surface LCU32
        CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
            &surfaceCodecParams,
            &m_residualDataScratchSurfaceForBLcu32,
            m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
            bindingTable->dwBindingTableEntries[startBti++],
            0,
            true));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    if (m_isMaxLcu64)
    {
        //Source Y 2xDS
        CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
            &surfaceCodecParams,
            m_trackedBuf->Get2xDsSurface(CODEC_CURR_TRACKED_BUFFER),
            m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_HME_DOWNSAMPLED_ENCODE].Value,
            bindingTable->dwBindingTableEntries[startBti++],
            0,
            false));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));

        // Intermediate/Enc cu record surface
        CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
            &surfaceCodecParams,
            &m_intermediateCuRecordSurfaceLcu64B,
            m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_PAK_OBJECT_ENCODE].Value,
            bindingTable->dwBindingTableEntries[startBti++],
            0,
            true));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));

        // Lcu64 constant data surface
        CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
            &surfaceCodecParams,
            &m_encConstantTableForLcu64B.sResource,
            MOS_BYTES_TO_DWORDS(m_encConstantTableForLcu64B.dwSize),
            0,
            m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
            bindingTable->dwBindingTableEntries[startBti++],
            false));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));

        // Lcu storage surface
        CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
            &surfaceCodecParams,
            &m_lcuEncodingScratchSurfaceLcu64B.sResource,
            MOS_BYTES_TO_DWORDS(m_lcuEncodingScratchSurfaceLcu64B.dwSize),
            0,
            m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
            bindingTable->dwBindingTableEntries[startBti++],
            true));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));

        // VME surfaces

        // 2xDS Source VME surface
        CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParamsVME(
            &surfaceCodecParams,
            m_trackedBuf->Get2xDsSurface(CODEC_CURR_TRACKED_BUFFER),
            m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value,
            bindingTable->dwBindingTableEntries[startBti++]));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));

        for(auto surface_idx = 0; surface_idx < 4; surface_idx++)
        {
            CODEC_PICTURE refPic = m_hevcSliceParams->RefPicList[LIST_0][surface_idx];
            if (!CodecHal_PictureIsInvalid(refPic) &&
                !CodecHal_PictureIsInvalid(m_hevcPicParams->RefFrameList[refPic.FrameIdx]))
            {
                uint8_t idx       = m_hevcPicParams->RefFrameList[refPic.FrameIdx].FrameIdx;
                uint8_t scaledIdx = m_refList[idx]->ucScalingIdx;

                // Picture Y VME
                CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParamsVME(
                    &surfaceCodecParams,
                    m_trackedBuf->Get2xDsSurface(scaledIdx),
                    m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value,
                    bindingTable->dwBindingTableEntries[startBti++]));

                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                    m_hwInterface,
                    cmdBuffer,
                    &surfaceCodecParams,
                    kernelState));
            }
            else
            {
                // Providing Dummy surface as per VME requirement.
                CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParamsVME(
                    &surfaceCodecParams,
                    m_trackedBuf->Get2xDsSurface(CODEC_CURR_TRACKED_BUFFER),
                    m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value,
                    bindingTable->dwBindingTableEntries[startBti++]));

                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                    m_hwInterface,
                    cmdBuffer,
                    &surfaceCodecParams,
                    kernelState));
            }

            refPic = m_hevcSliceParams->RefPicList[LIST_1][surface_idx];
            if (!CodecHal_PictureIsInvalid(refPic) &&
                !CodecHal_PictureIsInvalid(m_hevcPicParams->RefFrameList[refPic.FrameIdx]))
            {
                uint8_t idx       = m_hevcPicParams->RefFrameList[refPic.FrameIdx].FrameIdx;
                uint8_t scaledIdx = m_refList[idx]->ucScalingIdx;

                // Picture Y VME
                CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParamsVME(
                    &surfaceCodecParams,
                    m_trackedBuf->Get2xDsSurface(scaledIdx),
                    m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value,
                    bindingTable->dwBindingTableEntries[startBti++]));

                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                    m_hwInterface,
                    cmdBuffer,
                    &surfaceCodecParams,
                    kernelState));
            }
            else
            {
                // Providing Dummy surface as per VME requirement.
                CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParamsVME(
                    &surfaceCodecParams,
                    m_trackedBuf->Get2xDsSurface(CODEC_CURR_TRACKED_BUFFER),
                    m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value,
                    bindingTable->dwBindingTableEntries[startBti++]));

                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                    m_hwInterface,
                    cmdBuffer,
                    &surfaceCodecParams,
                    kernelState));
            }
        }

        // Job Queue Header buffer surface
        CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
            &surfaceCodecParams,
            &m_jobQueueHeaderSurfaceForBLcu64.sResource,
            m_jobQueueHeaderSurfaceForBLcu64.dwSize,
            0,
            m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
            bindingTable->dwBindingTableEntries[startBti++],
            true));

        surfaceCodecParams.bRawSurface = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));

        // Job Queue Data buffer surface LCU64
        CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
            &surfaceCodecParams,
            &m_jobQueueDataSurfaceForBLcu64,
            m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
            bindingTable->dwBindingTableEntries[startBti++],
            0,
            true));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));

        // Residual Data Scratch Surface LCU64
        CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
            &surfaceCodecParams,
            &m_residualDataScratchSurfaceForBLcu64,
            m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
            bindingTable->dwBindingTableEntries[startBti++],
            0,
            true));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    // Kernel debug surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        &surfaceCodecParams,
        &m_kernelDebug.sResource,
        MOS_BYTES_TO_DWORDS(m_kernelDebug.dwSize),
        0,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
        bindingTable->dwBindingTableEntries[startBti++],
        true));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG10::GenerateLcuLevelData()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    if(!Mos_ResourceIsNull(&m_lcuLevelInputData.sResource))
    {
        MOS_LOCK_PARAMS lockFlags;
        MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
        lockFlags.WriteOnly = 1;
        PCODECHAL_ENC_HEVC_LCU_LEVEL_DATA_G10 pLcuLevelData = (PCODECHAL_ENC_HEVC_LCU_LEVEL_DATA_G10) m_osInterface->pfnLockResource(
            m_osInterface,
            &m_lcuLevelInputData.sResource,
            &lockFlags);
        CODECHAL_ENCODE_CHK_NULL_RETURN(pLcuLevelData);

        PCODEC_HEVC_ENCODE_SLICE_PARAMS slcPrams = m_hevcSliceParams;
        for (uint32_t startLcu = 0, slcCount = 0; slcCount < m_numSlices; slcCount++, slcPrams++)
        {
            for (uint32_t i = 0; i < slcPrams->NumLCUsInSlice; i++, pLcuLevelData++)
            {
                pLcuLevelData->SliceStartLcuIndex = (uint16_t ) startLcu;
                pLcuLevelData->SliceEndLcuIndex   = (uint16_t ) (startLcu + slcPrams->NumLCUsInSlice); // this should be next slice start index
                pLcuLevelData->SliceId            = (uint16_t ) slcCount + 1;
                pLcuLevelData->SliceLevelQP       = (uint16_t)(m_hevcPicParams->QpY + slcPrams->slice_qp_delta);
            }

            startLcu += slcPrams->NumLCUsInSlice;
        }

        m_osInterface->pfnUnlockResource(
            m_osInterface,
            &m_lcuLevelInputData.sResource);
    }
    else
    {
        eStatus = MOS_STATUS_NULL_POINTER;
        CODECHAL_ENCODE_ASSERTMESSAGE("Null pointer exception\n");
        return eStatus;
    }

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG10::SetCurbeScalingAndConversion(
    CodechalEncodeCscDs::CurbeParams* params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pKernelState);

    // Initialize the curbe data
    DsConvCurbeDataG10 cmd = DsConvCurbeDataG10();

    cmd.DW1_ConvertFlag                 = params->bConvertFlag;
    cmd.DW1_DownscaleStage              = params->downscaleStage;
    cmd.DW1_MbStatisticsDumpFlag        = !m_brcEnabled ? 0 : ((params->downscaleStage == dsStage4x || params->downscaleStage == dsStage2x4x) ? 1 : 0);
    if( !params->bUseLCU32 )
    {
        cmd.DW1_LcuSize                 = 0;  // LCU64
        cmd.DW1_JobQueueSize            = 32;
    }
    else
    {
        cmd.DW1_LcuSize                 = 1;  // LCU32
    }

    cmd.DW2_OriginalPicWidthInSamples   = params->dwInputPictureWidth;
    cmd.DW2_OriginalPicHeightInSamples  = params->dwInputPictureHeight;

    cmd.DW3_BTI_InputConversionSurface                   = SCALING_CONVERSION_10BIT_Y;
    cmd.DW4_BTI_Value                                    = SCALING_CONVERSION_8BIT_Y;
    cmd.DW5_BTI_4xDsSurface                              = SCALING_CONVERSION_4xDS; // 4xDS for both 4x and 16x
    cmd.DW6_BTI_MBStatsSurface                           = SCALING_CONVERSION_MB_STATS;
    cmd.DW7_BTI_2xDsSurface                              = SCALING_CONVERSION_2xDS;
    cmd.DW8_BTI_MB_Split_Surface                         = SCALING_CONVERSION_MB_SPLIT_SURFACE;
    cmd.DW9_BTI_LCU32_JobQueueScratchBufferSurface       = SCALING_CONVERSION_LCU32_JOB_QUEUE_SCRATCH_SURFACE;
    cmd.DW10_BTI_LCU64_CU32_JobQueueScratchBufferSurface = SCALING_CONVERSION_LCU64_JOB_QUEUE_SCRATCH_SURFACE;
    cmd.DW11_BTI_LCU64_CU32_64x64_DistortionSurface      = SCALING_CONVERSION_LCU64_64x64_DISTORTION_SURFACE;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(params->pKernelState->m_dshRegion.AddData(
        &cmd,
        params->pKernelState->dwCurbeOffset,
        sizeof(cmd)));

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG10::EncodeScalingAndConversionKernel(
    CodechalEncodeCscDs::KernelParams* params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_scalingAndConversionKernelState);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_scalingAndConversionKernelBindingTable);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_refList);

    if (!m_firstField)
    {
        // Both fields are scaled when the first field comes in, no need to scale again
        return eStatus;
    }

    if (m_scalingEnabled)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_trackedBuf->AllocateSurfaceDS());
    }

    if (m_2xScalingEnabled)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_trackedBuf->AllocateSurface2xDS());
    }

    PerfTagSetting perfTag;
    CODECHAL_ENCODE_SET_PERFTAG_INFO(perfTag, CODECHAL_ENCODE_PERFTAG_CALL_SCALING_KERNEL);

    PMHW_KERNEL_STATE kernelState = m_scalingAndConversionKernelState;

    // If Single Task Phase is not enabled, use BT count for the kernel state.
    if (m_firstTaskInPhase || !m_singleTaskPhaseSupported)
    {
        uint32_t maxBtCount = m_singleTaskPhaseSupported ?
            m_maxBtCount : kernelState->KernelParams.iBTCount;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnRequestSshSpaceForCmdBuf(
            m_stateHeapInterface,
            maxBtCount));
        m_vmeStatesSize = m_hwInterface->GetKernelLoadCommandSize(maxBtCount);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(VerifySpaceAvailable());
    }

    CODECHAL_MEDIA_STATE_TYPE encFunctionType;
    PCODEC_REF_LIST           currRefList = m_refList[m_currReconstructedPic.FrameIdx];
    DsStage downscaleStage;
    bool convertFlag = false;
    uint32_t inputFrameWidth = 0, inputFrameHeight = 0;
    uint32_t outputConvertedFrameWidth = 0, outputConvertedFrameHeight = 0;
    uint32_t outputScaledFrameWidth2x = 0, outputScaledFrameHeight2x = 0;
    uint32_t outputScaledFrameWidth4x = 0, outputScaledFrameHeight4x = 0;
    PMOS_SURFACE inputSurface = nullptr, outputConvertedSurface = nullptr, outputScaledSurface4x = nullptr, outputScaledSurface2x = nullptr;

    switch(params->stageDsConversion)
    {
        case convDs2xFromOrig:
            convertFlag = true;
            outputConvertedSurface     = params->psFormatConvertedSurface;
            outputConvertedFrameWidth  = m_oriFrameWidth;
            outputConvertedFrameHeight = m_oriFrameHeight;
            // break omitted on purpose
        case ds2xFromOrig:
            encFunctionType         = CODECHAL_MEDIA_STATE_2X_SCALING;
            downscaleStage          = dsStage2x;

            inputSurface = m_rawSurfaceToEnc;
            inputFrameWidth = m_oriFrameWidth;
            inputFrameHeight = m_oriFrameHeight;

            outputScaledSurface2x     = m_trackedBuf->Get2xDsSurface(CODEC_CURR_TRACKED_BUFFER);
            outputScaledFrameWidth2x  = m_downscaledWidth2x;
            outputScaledFrameHeight2x = m_downscaledHeight2x;

            currRefList->b2xScalingUsed = true;

            break;
        case convDs2x4xFromOrig:
            convertFlag = true;
            outputConvertedSurface     = params->psFormatConvertedSurface;
            outputConvertedFrameWidth  = m_oriFrameWidth;
            outputConvertedFrameHeight = m_oriFrameHeight;
            // break omitted on purpose
        case ds2x4xFromOrig:
            encFunctionType         = CODECHAL_MEDIA_STATE_2X_4X_SCALING;
            downscaleStage          = dsStage2x4x;

            inputSurface = m_rawSurfaceToEnc;
            inputFrameWidth = m_oriFrameWidth;
            inputFrameHeight = m_oriFrameHeight;

            outputScaledSurface4x = m_trackedBuf->Get4xDsSurface(CODEC_CURR_TRACKED_BUFFER);
            outputScaledFrameWidth4x  = m_downscaledWidth4x;
            outputScaledFrameHeight4x = m_downscaledHeight4x;

            outputScaledSurface2x = m_trackedBuf->Get2xDsSurface(CODEC_CURR_TRACKED_BUFFER);
            outputScaledFrameWidth2x  = m_downscaledWidth2x;
            outputScaledFrameHeight2x = m_downscaledHeight2x;

            currRefList->b4xScalingUsed = true;
            currRefList->b2xScalingUsed = true;

            break;
        case convDs4xFromOrig:
            convertFlag = true;
            outputConvertedSurface = params->psFormatConvertedSurface;
            outputConvertedFrameWidth  = m_oriFrameWidth;
            outputConvertedFrameHeight = m_oriFrameHeight;
            // break omitted on purpose
        case ds4xFromOrig:
            encFunctionType         = CODECHAL_MEDIA_STATE_4X_SCALING;
            downscaleStage          = dsStage4x;

            inputSurface = m_rawSurfaceToEnc;
            inputFrameWidth = m_oriFrameWidth;
            inputFrameHeight = m_oriFrameHeight;

            outputScaledSurface4x = m_trackedBuf->Get4xDsSurface(CODEC_CURR_TRACKED_BUFFER);
            outputScaledFrameWidth4x  = m_downscaledWidth4x;
            outputScaledFrameHeight4x = m_downscaledHeight4x;

            currRefList->b4xScalingUsed = true;

            break;
        case ds16xFromOrig:
            encFunctionType         = CODECHAL_MEDIA_STATE_16X_SCALING;
            downscaleStage          = dsStage16x;

            inputSurface = m_trackedBuf->Get4xDsSurface(CODEC_CURR_TRACKED_BUFFER);
            inputFrameWidth = m_downscaledWidth4x;
            inputFrameHeight = m_downscaledHeight4x;

            outputScaledSurface4x = m_trackedBuf->Get16xDsSurface(CODEC_CURR_TRACKED_BUFFER);
            outputScaledFrameWidth4x  = m_downscaledWidth16x;
            outputScaledFrameHeight4x = m_downscaledHeight16x;

            currRefList->b16xScalingUsed = true;

            break;
        case convFromOrig:
            encFunctionType = CODECHAL_MEDIA_STATE_2X_SCALING;
            downscaleStage  = dsDisabled;
            convertFlag = true;

            inputSurface = params->psFormatConversionOnlyInputSurface;
            inputFrameWidth = m_oriFrameWidth;
            inputFrameHeight = m_oriFrameHeight;

            outputConvertedSurface = params->psFormatConvertedSurface;
            outputConvertedFrameWidth  = m_oriFrameWidth;
            outputConvertedFrameHeight = m_oriFrameHeight;

            break;
        default:
            CODECHAL_ENCODE_ASSERTMESSAGE("Unsupported DownScale or Conversion type requested");
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            return eStatus;
    }

    if(convertFlag)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalGetResourceInfo(
                m_osInterface,
                params->psFormatConvertedSurface));
    }

    //Setup Scaling DSH
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->AssignDshAndSshSpace(
        m_stateHeapInterface,
        kernelState,
        false,
        0,
        false,
        m_storeData));

    MHW_INTERFACE_DESCRIPTOR_PARAMS idParams;
    MOS_ZeroMemory(&idParams, sizeof(idParams));
    idParams.pKernelState = kernelState;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSetInterfaceDescriptor(
        m_stateHeapInterface,
        1,
        &idParams));

    // Call Scaling-Conversion curbe
    CodechalEncodeCscDs::CurbeParams scalingConversionCurbeParams;
    MOS_ZeroMemory(&scalingConversionCurbeParams, sizeof(scalingConversionCurbeParams));
    scalingConversionCurbeParams.pKernelState = kernelState;
    scalingConversionCurbeParams.bConvertFlag = convertFlag;
    scalingConversionCurbeParams.bUseLCU32            = !m_isMaxLcu64;
    scalingConversionCurbeParams.downscaleStage = downscaleStage;
    scalingConversionCurbeParams.dwInputPictureWidth = m_oriFrameWidth;
    scalingConversionCurbeParams.dwInputPictureHeight = m_oriFrameHeight;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetCurbeScalingAndConversion(&scalingConversionCurbeParams));

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            encFunctionType,
            MHW_DSH_TYPE,
            kernelState));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCurbe(
            encFunctionType,
            kernelState));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            encFunctionType,
            MHW_ISH_TYPE,
            kernelState));
     )

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    SendKernelCmdsParams sendKernelCmdsParams = SendKernelCmdsParams();
    sendKernelCmdsParams.EncFunctionType = encFunctionType;
    sendKernelCmdsParams.pKernelState = kernelState;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendGenericKernelCmds(&cmdBuffer, &sendKernelCmdsParams));

    // Add binding table
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSetBindingTable(
        m_stateHeapInterface,
        kernelState));

    // set binding table surfaces
    SurfaceParamsDsConv scalingConversionSurfaceParams;
    MOS_ZeroMemory(&scalingConversionSurfaceParams, sizeof(scalingConversionSurfaceParams));
    scalingConversionSurfaceParams.psInputSurface               = inputSurface;
    scalingConversionSurfaceParams.dwInputFrameWidth            = inputFrameWidth;
    scalingConversionSurfaceParams.dwInputFrameHeight           = inputFrameHeight;
    scalingConversionSurfaceParams.psOutputScaledSurface2x      = outputScaledSurface2x;
    scalingConversionSurfaceParams.dwOutputScaledFrameWidth2x   = outputScaledFrameWidth2x;
    scalingConversionSurfaceParams.dwOutputScaledFrameHeight2x  = outputScaledFrameHeight2x;
    scalingConversionSurfaceParams.psOutputScaledSurface4x      = outputScaledSurface4x;
    scalingConversionSurfaceParams.dwOutputScaledFrameWidth4x   = outputScaledFrameWidth4x;
    scalingConversionSurfaceParams.dwOutputScaledFrameHeight4x  = outputScaledFrameHeight4x;
    scalingConversionSurfaceParams.pKernelState                 = kernelState;
    scalingConversionSurfaceParams.pBindingTable                = m_scalingAndConversionKernelBindingTable;
    scalingConversionSurfaceParams.downScaleConversionType      = params->stageDsConversion;

    if(convertFlag)
    {
        scalingConversionSurfaceParams.psOutputConvertedSurface       = outputConvertedSurface;
        scalingConversionSurfaceParams.dwOutputConvertedFrameWidth    = outputConvertedFrameWidth;
        scalingConversionSurfaceParams.dwOutputConvertedFrameHeight   = outputConvertedFrameHeight;
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendScalingAndConversionSurfaces(
        &cmdBuffer,
        &scalingConversionSurfaceParams));

    // Add dump for scaling surface state heap here
    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            encFunctionType,
            MHW_SSH_TYPE,
            kernelState));
    )

    //Walker programming goes here
    if (!m_hwWalker)
    {
        eStatus = MOS_STATUS_UNKNOWN;
        CODECHAL_ENCODE_ASSERTMESSAGE("HW walker should be enabled.");
        return eStatus;
    }

    uint32_t resolutionX = (params->stageDsConversion == ds16xFromOrig) ? m_downscaledWidth16x : m_downscaledWidth4x;
    uint32_t resolutionY = (params->stageDsConversion == ds16xFromOrig) ? m_downscaledHeight16x : m_downscaledHeight4x;

    CODECHAL_WALKER_CODEC_PARAMS walkerCodecParams;
    MOS_ZeroMemory(&walkerCodecParams, sizeof(walkerCodecParams));
    walkerCodecParams.WalkerMode                    = m_walkerMode;
    walkerCodecParams.dwResolutionX                 = resolutionX >> 3; /* looping for Walker is needed at 8x8 block level */
    walkerCodecParams.dwResolutionY                 = resolutionY >> 3;
    walkerCodecParams.bNoDependency                 = true;     /* Enforce no dependency dispatch order for Scaling kernel,  */
    walkerCodecParams.bGroupIdSelectSupported       = m_groupIdSelectSupported;
    walkerCodecParams.ucGroupId                     = m_groupId;

    MHW_WALKER_PARAMS walkerParams;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalInitMediaObjectWalkerParams(
        m_hwInterface,
        &walkerParams,
        &walkerCodecParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_renderEngineInterface->AddMediaObjectWalkerCmd(
        &cmdBuffer,
        &walkerParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(EndStatusReport(&cmdBuffer, encFunctionType));

    // Add dump for scaling surface state heap here
    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            encFunctionType,
            MHW_SSH_TYPE,
            kernelState));
    )

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSubmitBlocks(
        m_stateHeapInterface,
        kernelState));
    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnUpdateGlobalCmdBufId(
            m_stateHeapInterface));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(&cmdBuffer, nullptr));
    }

    CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
        &cmdBuffer,
        encFunctionType,
        nullptr)));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->UpdateSSEuForCmdBuffer(&cmdBuffer, m_singleTaskPhaseSupported, m_lastTaskInPhase));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, m_renderContextUsesNullHw);
        m_lastTaskInPhase = false;
    }

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG10::PerformScalingAndConversion()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    // Walker must be used for HME call and scaling one
    CODECHAL_ENCODE_ASSERT(m_hwWalker);

    // Scaling occurs regardless of whether HME is in use for the current frame
    CodechalEncodeCscDs::KernelParams params;
    MOS_ZeroMemory(&params, sizeof(params));
    params.stageDsConversion = m_hmeSupported ? (m_isMaxLcu64 ? ds2x4xFromOrig : ds4xFromOrig) : (m_isMaxLcu64 ? ds2xFromOrig : dsConvUnknown);

    if (m_hevcSeqParams->bit_depth_luma_minus8)
    {
        params.stageDsConversion = DsStage(params.stageDsConversion | convFromOrig);
        params.psFormatConvertedSurface = &m_formatConvertedSurface[0];
        if(params.stageDsConversion == convFromOrig)
        {
            params.psFormatConversionOnlyInputSurface = m_rawSurfaceToEnc;
        }
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(EncodeScalingAndConversionKernel(&params));

    if(m_16xMeSupported)
    {
        params.stageDsConversion = ds16xFromOrig;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(EncodeScalingAndConversionKernel(&params));
    }

    return eStatus;
}

bool CodechalEncHevcStateG10::CheckSupportedFormat(PMOS_SURFACE surface)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    bool isColorFormatSupported = false;

    if (nullptr == surface)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Invalid (nullptr) Pointer.");
        return isColorFormatSupported;
    }

    switch (surface->Format)
    {
    case Format_NV12:
        isColorFormatSupported = IS_Y_MAJOR_TILE_FORMAT(surface->TileType);
        break;
    case Format_P010:
        isColorFormatSupported = true;
    case Format_YUY2:
    case Format_YUYV:
    case Format_A8R8G8B8:
        break;
    default:
        CODECHAL_ENCODE_ASSERTMESSAGE("Input surface color format = %d not supported!", surface->Format);
        break;
    }

    return isColorFormatSupported;
}

MOS_STATUS CodechalEncHevcStateG10::EncodeMeKernel(
    HmeLevel                    hmeLevel,
    HEVC_ME_DIST_TYPE           distType)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    PerfTagSetting perfTag;
    CODECHAL_ENCODE_SET_PERFTAG_INFO(perfTag, CODECHAL_ENCODE_PERFTAG_CALL_ME_KERNEL);

    CODECHAL_MEDIA_STATE_TYPE encFunctionType = (hmeLevel == HME_LEVEL_4x) ? CODECHAL_MEDIA_STATE_4X_ME : CODECHAL_MEDIA_STATE_16X_ME;

    // Initialize DSH kernel state
    PMHW_KERNEL_STATE kernelState = m_meKernelState;
    // If Single Task Phase is not enabled, use BT count for the kernel state.
    if (m_firstTaskInPhase || !m_singleTaskPhaseSupported)
    {
        uint32_t maxBtCount = m_singleTaskPhaseSupported ?
            m_maxBtCount : kernelState->KernelParams.iBTCount;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnRequestSshSpaceForCmdBuf(
            m_stateHeapInterface,
            maxBtCount));
        m_vmeStatesSize = m_hwInterface->GetKernelLoadCommandSize(maxBtCount);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(VerifySpaceAvailable());
    }

    // Set up the DSH/SSH as normal
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->AssignDshAndSshSpace(
        m_stateHeapInterface,
        kernelState,
        false,
        0,
        false,
        m_storeData));

    MHW_INTERFACE_DESCRIPTOR_PARAMS idParams;
    MOS_ZeroMemory(&idParams, sizeof(idParams));
    idParams.pKernelState = kernelState;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSetInterfaceDescriptor(
        m_stateHeapInterface,
        1,
        &idParams));

    // Setup curbe for Me kernel
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetCurbeMe(
        hmeLevel,
        distType));

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            encFunctionType,
            MHW_DSH_TYPE,
            kernelState));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCurbe(
            encFunctionType,
            kernelState));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            encFunctionType,
            MHW_ISH_TYPE,
            kernelState));
    )

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    SendKernelCmdsParams sendKernelCmdsParams = SendKernelCmdsParams();
    sendKernelCmdsParams.EncFunctionType = encFunctionType;
    sendKernelCmdsParams.pKernelState = kernelState;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendGenericKernelCmds(&cmdBuffer, &sendKernelCmdsParams));

    // Add binding table
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSetBindingTable(
        m_stateHeapInterface,
        kernelState));

    // Send surfaces for Me Kernel
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendMeSurfaces(&cmdBuffer, hmeLevel, distType));

    uint32_t resolutionX = (hmeLevel == HME_LEVEL_4x) ? m_downscaledWidth4x : m_downscaledWidth16x;
    uint32_t resolutionY = (hmeLevel == HME_LEVEL_4x) ? m_downscaledHeight4x : m_downscaledHeight16x;

    // Work on 32x32 blocks
    resolutionX >>= 5;
    resolutionY >>= 5;

    CODECHAL_WALKER_CODEC_PARAMS walkerCodecParams;
    MOS_ZeroMemory(&walkerCodecParams, sizeof(walkerCodecParams));
    walkerCodecParams.WalkerMode = m_walkerMode;
    walkerCodecParams.dwResolutionX = resolutionX;
    walkerCodecParams.dwResolutionY = resolutionY;
    walkerCodecParams.bNoDependency = true;
    walkerCodecParams.bGroupIdSelectSupported = m_groupIdSelectSupported;
    walkerCodecParams.ucGroupId = m_groupId;

    MHW_WALKER_PARAMS walkerParams;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalInitMediaObjectWalkerParams(
        m_hwInterface,
        &walkerParams,
        &walkerCodecParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_renderEngineInterface->AddMediaObjectWalkerCmd(
        &cmdBuffer,
        &walkerParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(EndStatusReport(&cmdBuffer, encFunctionType));

    // Add dump for Me surface state heap here
    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            encFunctionType,
            MHW_SSH_TYPE,
            kernelState));
    )

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSubmitBlocks(
        m_stateHeapInterface,
        kernelState));
    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnUpdateGlobalCmdBufId(
            m_stateHeapInterface));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(
            &cmdBuffer,
            nullptr));
    }

    CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
        &cmdBuffer,
        encFunctionType,
        nullptr)));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->UpdateSSEuForCmdBuffer(&cmdBuffer, m_singleTaskPhaseSupported, m_lastTaskInPhase));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, m_renderContextUsesNullHw);
        m_lastTaskInPhase = false;
    }

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG10::EncodeBrcInitResetKernel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    PerfTagSetting perfTag;
    CODECHAL_ENCODE_SET_PERFTAG_INFO(perfTag, CODECHAL_ENCODE_PERFTAG_CALL_BRC_INIT_RESET);

    // Initialize DSH kernel state
    CODECHAL_HEVC_BRC_KRNIDX brcKrnIdx   = m_brcInit ? CODECHAL_HEVC_BRC_INIT : CODECHAL_HEVC_BRC_RESET;
    PMHW_KERNEL_STATE        kernelState = &m_brcKernelStates[brcKrnIdx];
    // If Single Task Phase is not enabled, use BT count for the kernel state.
    if (m_firstTaskInPhase || !m_singleTaskPhaseSupported)
    {
        uint32_t maxBtCount = m_singleTaskPhaseSupported ?
            m_maxBtCount : kernelState->KernelParams.iBTCount;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnRequestSshSpaceForCmdBuf(
            m_stateHeapInterface,
            maxBtCount));
        m_vmeStatesSize = m_hwInterface->GetKernelLoadCommandSize(maxBtCount);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(VerifySpaceAvailable());
    }

    // Set up the DSH/SSH as normal
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->AssignDshAndSshSpace(
        m_stateHeapInterface,
        kernelState,
        false,
        0,
        false,
        m_storeData));

    MHW_INTERFACE_DESCRIPTOR_PARAMS idParams;
    MOS_ZeroMemory(&idParams, sizeof(idParams));
    idParams.pKernelState = kernelState;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSetInterfaceDescriptor(
        m_stateHeapInterface,
        1,
        &idParams));

    // Setup curbe for BrcInitReset kernel
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetCurbeBrcInitReset(brcKrnIdx));

    CODECHAL_MEDIA_STATE_TYPE encFunctionType = CODECHAL_MEDIA_STATE_BRC_INIT_RESET;
    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            encFunctionType,
            MHW_DSH_TYPE,
            kernelState));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCurbe(
            encFunctionType,
            kernelState));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            encFunctionType,
            MHW_ISH_TYPE,
            kernelState));
    )

//#if (_DEBUG || _RELEASE_INTERNAL)
//        if (m_swBrcMode != nullptr)
//        {
//            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_DbgCallHevcSwBrcImpl(
//                m_debugInterface,
//                encFunctionType,
//                this,
//                bBrcReset,
//                kernelState,
//                kernelState));
//
//            return eStatus;
//        }
//#endif // (_DEBUG || _RELEASE_INTERNAL)

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    SendKernelCmdsParams sendKernelCmdsParams = SendKernelCmdsParams();
    sendKernelCmdsParams.EncFunctionType = encFunctionType;
    sendKernelCmdsParams.pKernelState = kernelState;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendGenericKernelCmds(&cmdBuffer, &sendKernelCmdsParams));

    // Add binding table
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSetBindingTable(
        m_stateHeapInterface,
        kernelState));

    // Send surfaces for BrcInitReset Kernel
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendBrcInitResetSurfaces(&cmdBuffer, brcKrnIdx));

    MediaObjectInlineData mediaObjectInlineData;
    MHW_MEDIA_OBJECT_PARAMS mediaObjectParams;
    MOS_ZeroMemory(&mediaObjectParams, sizeof(mediaObjectParams));
    MOS_ZeroMemory(&mediaObjectInlineData, sizeof(mediaObjectInlineData));
    mediaObjectParams.pInlineData = &mediaObjectInlineData;
    mediaObjectParams.dwInlineDataSize = sizeof(mediaObjectInlineData);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_renderEngineInterface->AddMediaObject(
        &cmdBuffer,
        nullptr,
        &mediaObjectParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(EndStatusReport(&cmdBuffer, encFunctionType));

    // Add dump for BrcInitReset surface state heap here
    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            encFunctionType,
            MHW_SSH_TYPE,
            kernelState));
    )

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSubmitBlocks(
        m_stateHeapInterface,
        kernelState));
    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnUpdateGlobalCmdBufId(
            m_stateHeapInterface));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(
            &cmdBuffer,
            nullptr));
    }

    CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
        &cmdBuffer,
        encFunctionType,
        nullptr)));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->UpdateSSEuForCmdBuffer(&cmdBuffer, m_singleTaskPhaseSupported, m_lastTaskInPhase));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, m_renderContextUsesNullHw);
        m_lastTaskInPhase = false;
    }

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG10::EncodeBrcFrameUpdateKernel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    PerfTagSetting perfTag;
    CODECHAL_ENCODE_SET_PERFTAG_INFO(perfTag, CODECHAL_ENCODE_PERFTAG_CALL_BRC_UPDATE);

    // Initialize DSH kernel state
    CODECHAL_HEVC_BRC_KRNIDX brcKrnIdx = CODECHAL_HEVC_BRC_FRAME_UPDATE;
    PMHW_KERNEL_STATE        kernelState = &m_brcKernelStates[brcKrnIdx];
    // If Single Task Phase is not enabled, use BT count for the kernel state.
    if (m_firstTaskInPhase || !m_singleTaskPhaseSupported)
    {
        uint32_t maxBtCount = m_singleTaskPhaseSupported ?
            m_maxBtCount : kernelState->KernelParams.iBTCount;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnRequestSshSpaceForCmdBuf(
            m_stateHeapInterface,
            maxBtCount));
        m_vmeStatesSize = m_hwInterface->GetKernelLoadCommandSize(maxBtCount);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(VerifySpaceAvailable());
    }

    // Fill HCP_IMG_STATE so that BRC kernel can use it to generate the write buffer for PAK
    MHW_VDBOX_HEVC_PIC_STATE mhwHevcPicState;
    mhwHevcPicState.pHevcEncSeqParams = m_hevcSeqParams;
    mhwHevcPicState.pHevcEncPicParams = m_hevcPicParams;
    mhwHevcPicState.bUseVDEnc = false;
    mhwHevcPicState.brcNumPakPasses = m_mfxInterface->GetBrcNumPakPasses();
    mhwHevcPicState.bSAOEnable = m_hevcSeqParams->SAO_enabled_flag ? (m_hevcSliceParams->slice_sao_luma_flag || m_hevcSliceParams->slice_sao_chroma_flag) : 0;

    PMOS_RESOURCE brcHcpStateReadBuffer = &m_brcBuffers.resBrcImageStatesReadBuffer[m_currRecycledBufIdx];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpHevcPicBrcBuffer(brcHcpStateReadBuffer, &mhwHevcPicState));

    PMOS_SURFACE brcConstantData = &m_brcBuffers.sBrcConstantDataBuffer[m_currRecycledBufIdx];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetupBrcConstantTable(brcConstantData));

    // Set up the DSH/SSH as normal
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->AssignDshAndSshSpace(
        m_stateHeapInterface,
        kernelState,
        false,
        0,
        false,
        m_storeData));

    MHW_INTERFACE_DESCRIPTOR_PARAMS idParams;
    MOS_ZeroMemory(&idParams, sizeof(idParams));
    idParams.pKernelState = kernelState;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSetInterfaceDescriptor(
        m_stateHeapInterface,
        1,
        &idParams));

    // Setup curbe for BrcFrameUpdate kernel
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetCurbeBrcUpdate(brcKrnIdx));

    CODECHAL_MEDIA_STATE_TYPE encFunctionType = CODECHAL_MEDIA_STATE_BRC_UPDATE;
    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            encFunctionType,
            MHW_DSH_TYPE,
            kernelState));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCurbe(
            encFunctionType,
            kernelState));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            encFunctionType,
            MHW_ISH_TYPE,
            kernelState));
    )

//#if (_DEBUG || _RELEASE_INTERNAL)
//        if (m_swBrcMode != nullptr)
//        {
//            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_DbgCallHevcSwBrcImpl(
//                m_debugInterface,
//                encFunctionType,
//                this,
//                false,
//                kernelState,
//                kernelState));
//            return eStatus;
//        }
//#endif // (_DEBUG || _RELEASE_INTERNAL)

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    SendKernelCmdsParams sendKernelCmdsParams = SendKernelCmdsParams();
    sendKernelCmdsParams.EncFunctionType = encFunctionType;
    sendKernelCmdsParams.pKernelState = kernelState;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendGenericKernelCmds(&cmdBuffer, &sendKernelCmdsParams));

    // Add binding table
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSetBindingTable(
        m_stateHeapInterface,
        kernelState));

    // Send surfaces for BrcFrameUpdate Kernel
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendBrcFrameUpdateSurfaces(&cmdBuffer));

    CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
        &m_brcBuffers.resBrcHistoryBuffer,
        CodechalDbgAttr::attrOutput,
        "Input_HistoryBuffer",
        m_brcHistoryBufferSize,
        0,
        CODECHAL_MEDIA_STATE_BRC_UPDATE)));

    CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
        &m_brcBuffers.resBrcPakStatisticBuffer[m_brcBuffers.uiCurrBrcPakStasIdxForRead],
        CodechalDbgAttr::attrOutput,
        "Input_PakStats",
        m_brcPakStatisticsSize,
        0,
        CODECHAL_MEDIA_STATE_BRC_UPDATE)));

    CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
        &m_brcBuffers.resBrcImageStatesReadBuffer[m_currRecycledBufIdx],
        CodechalDbgAttr::attrOutput,
        "Input_ImgStateRead",
        m_brcBuffers.dwBrcHcpPicStateSize,
        0,
        CODECHAL_MEDIA_STATE_BRC_UPDATE)));

    CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
        &m_brcBuffers.resBrcImageStatesWriteBuffer[m_currRecycledBufIdx],
        CodechalDbgAttr::attrOutput,
        "Input_ImgStateWrite",
        m_brcBuffers.dwBrcHcpPicStateSize,
        0,
        CODECHAL_MEDIA_STATE_BRC_UPDATE)));

    CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
        (MOS_RESOURCE*)m_allocator->GetResource(m_standard, brcInputForEncKernel),
        CodechalDbgAttr::attrOutput,
        "Output_CombinedEnc",
        m_brcCombinedEncBufferSize,
        0,
        CODECHAL_MEDIA_STATE_BRC_UPDATE)));

    CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpSurface(
        &m_brcBuffers.sBrcConstantDataBuffer[m_currRecycledBufIdx],
        CodechalDbgAttr::attrOutput,
        "Input_ConstData",
        CODECHAL_MEDIA_STATE_BRC_UPDATE)));

    CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpSurface(
        &m_brcBuffers.sMeBrcDistortionBuffer,
        CodechalDbgAttr::attrOutput,
        "Input_Distortion",
        CODECHAL_MEDIA_STATE_BRC_UPDATE)));

    MHW_MEDIA_OBJECT_PARAMS mediaObjectParams;
    MediaObjectInlineData mediaObjectInlineData;
    MOS_ZeroMemory(&mediaObjectParams, sizeof(mediaObjectParams));
    MOS_ZeroMemory(&mediaObjectInlineData, sizeof(mediaObjectInlineData));
    mediaObjectParams.pInlineData = &mediaObjectInlineData;
    mediaObjectParams.dwInlineDataSize = sizeof(mediaObjectInlineData);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_renderEngineInterface->AddMediaObject(
        &cmdBuffer,
        nullptr,
        &mediaObjectParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(EndStatusReport(&cmdBuffer, encFunctionType));

    // Add dump for BrcFrameUpdate surface state heap here
    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            encFunctionType,
            MHW_SSH_TYPE,
            kernelState));
    )

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSubmitBlocks(
        m_stateHeapInterface,
        kernelState));
    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnUpdateGlobalCmdBufId(
            m_stateHeapInterface));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(
            &cmdBuffer,
            nullptr));
    }

    CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
        &cmdBuffer,
        encFunctionType,
        nullptr)));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->UpdateSSEuForCmdBuffer(&cmdBuffer, m_singleTaskPhaseSupported, m_lastTaskInPhase));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, m_renderContextUsesNullHw);
        m_lastTaskInPhase = false;
    }

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG10::EncodeBrcLcuUpdateKernel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    PerfTagSetting perfTag;
    CODECHAL_ENCODE_SET_PERFTAG_INFO(perfTag, CODECHAL_ENCODE_PERFTAG_CALL_BRC_UPDATE_LCU);

    CODECHAL_MEDIA_STATE_TYPE encFunctionType = CODECHAL_MEDIA_STATE_HEVC_BRC_LCU_UPDATE;

    // Initialize DSH kernel state
    CODECHAL_HEVC_BRC_KRNIDX brcKrnIdx = CODECHAL_HEVC_BRC_LCU_UPDATE;
    PMHW_KERNEL_STATE        kernelState = &m_brcKernelStates[brcKrnIdx];

    // If Single Task Phase is not enabled, use BT count for the kernel state.
    if (m_firstTaskInPhase || !m_singleTaskPhaseSupported)
    {
        uint32_t maxBtCount = m_singleTaskPhaseSupported ?
            m_maxBtCount : kernelState->KernelParams.iBTCount;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnRequestSshSpaceForCmdBuf(
            m_stateHeapInterface,
            maxBtCount));
        m_vmeStatesSize = m_hwInterface->GetKernelLoadCommandSize(maxBtCount);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(VerifySpaceAvailable());
    }

    // Set up the DSH/SSH as normal
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->AssignDshAndSshSpace(
        m_stateHeapInterface,
        kernelState,
        false,
        0,
        false,
        m_storeData));

    MHW_INTERFACE_DESCRIPTOR_PARAMS idParams;
    MOS_ZeroMemory(&idParams, sizeof(idParams));
    idParams.pKernelState = kernelState;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSetInterfaceDescriptor(
        m_stateHeapInterface,
        1,
        &idParams));

    // Setup curbe for BrcLcuUpdate kernel
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetCurbeBrcUpdate(brcKrnIdx));

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            encFunctionType,
            MHW_DSH_TYPE,
            kernelState));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCurbe(
            encFunctionType,
            kernelState));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            encFunctionType,
            MHW_ISH_TYPE,
            kernelState));
    )

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    SendKernelCmdsParams sendKernelCmdsParams = SendKernelCmdsParams();
    sendKernelCmdsParams.EncFunctionType = encFunctionType;
    sendKernelCmdsParams.pKernelState = kernelState;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendGenericKernelCmds(&cmdBuffer, &sendKernelCmdsParams));

    // Add binding table
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSetBindingTable(
        m_stateHeapInterface,
        kernelState));

    // Send surfaces for BrcFrameUpdate Kernel
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendBrcLcuUpdateSurfaces(&cmdBuffer));

    // For CNL thread space is 16x8 MB (regardless of LCU32 or LCU64)
    uint32_t resolutionX = CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_frameWidth);
    resolutionX = MOS_ROUNDUP_SHIFT(resolutionX, 4);
    uint32_t resolutionY = CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameHeight);
    resolutionY = MOS_ROUNDUP_SHIFT(resolutionY, 3);

    CODECHAL_WALKER_CODEC_PARAMS walkerCodecParams;
    MOS_ZeroMemory(&walkerCodecParams, sizeof(walkerCodecParams));
    walkerCodecParams.WalkerMode                = m_walkerMode;
    walkerCodecParams.dwResolutionX             = resolutionX;
    walkerCodecParams.dwResolutionY             = resolutionY;
    walkerCodecParams.bNoDependency             = true;
    walkerCodecParams.bGroupIdSelectSupported   = m_groupIdSelectSupported;
    walkerCodecParams.ucGroupId                 = m_groupId;
    walkerCodecParams.wPictureCodingType        = m_pictureCodingType;
    walkerCodecParams.bUseScoreboard            = m_useHwScoreboard;

    MHW_WALKER_PARAMS walkerParams;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalInitMediaObjectWalkerParams(
        m_hwInterface,
        &walkerParams,
        &walkerCodecParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_renderEngineInterface->AddMediaObjectWalkerCmd(
        &cmdBuffer,
        &walkerParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(EndStatusReport(&cmdBuffer, encFunctionType));

    // Add dump for BrcFrameUpdate surface state heap here
    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            encFunctionType,
            MHW_SSH_TYPE,
            kernelState));
    )

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSubmitBlocks(
        m_stateHeapInterface,
        kernelState));
    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnUpdateGlobalCmdBufId(
            m_stateHeapInterface));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(
            &cmdBuffer,
            nullptr));
    }

    CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
        &cmdBuffer,
        encFunctionType,
        nullptr)));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->UpdateSSEuForCmdBuffer(&cmdBuffer, m_singleTaskPhaseSupported, m_lastTaskInPhase));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, m_renderContextUsesNullHw);
        m_lastTaskInPhase = false;
    }

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG10::GenerateWalkingControlRegion()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    uint32_t frameWidthInUnits = 0, frameHeightInUnits = 0;
    int32_t copyBlockSize = 0, log2LCUSize = 0;
    if (!m_isMaxLcu64)
    {
        frameWidthInUnits = CODECHAL_ENCODE_HEVC_GET_SIZE_IN_LCU(m_frameWidth, 32);
        frameHeightInUnits = CODECHAL_ENCODE_HEVC_GET_SIZE_IN_LCU(m_frameHeight, 32);
        log2LCUSize = 5;
        copyBlockSize = 18;
    }
    else
    {
        frameWidthInUnits = CODECHAL_ENCODE_HEVC_GET_SIZE_IN_LCU(m_frameWidth, 64);
        frameHeightInUnits = CODECHAL_ENCODE_HEVC_GET_SIZE_IN_LCU(m_frameHeight, 64);
        log2LCUSize = 6;
        copyBlockSize = 22;
    }
    CODECHAL_ENCODE_CHK_COND_RETURN(frameWidthInUnits > 0, "invalid frameWidthInUnits");
    CODECHAL_ENCODE_CHK_COND_RETURN(frameHeightInUnits > 0, "invalid frameHeightInUnits");
    int32_t sliceStartY[CODECHAL_HEVC_MAX_NUM_SLICES_LVL_5 + 1] = { 0 }; // Allocate +1 of max num slices
    bool isArbitrarySlices = false;
    for (uint32_t slice = 0; slice < m_numSlices; slice++)
    {
        if (m_hevcSliceParams[slice].slice_segment_address % frameWidthInUnits)
        {
            isArbitrarySlices = true;
        }
        else
        {
            sliceStartY[slice] = m_hevcSliceParams[slice].slice_segment_address / frameWidthInUnits;
        }
    }

    sliceStartY[m_numSlices] = frameHeightInUnits;

    const uint32_t regionStartYOffset = 32;
    uint16_t regionsStartTable[64] = { 0 };
    uint32_t numRegions = 1;
    int32_t maxHeight = 0;
    uint32_t numUnitInRegion = 0, height = 0, numSlices = 0;

    if (isArbitrarySlices)
    {
        height = frameHeightInUnits;
        numSlices = 1;
        maxHeight = height;
        if (m_numRegionsInSlice > 1)
        {
            numUnitInRegion =
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

        numUnitInRegion =
            (frameWidthInUnits + 2 * (maxHeight - 1) + m_numRegionsInSlice - 1) / m_numRegionsInSlice;

        numRegions = numSlices * m_numRegionsInSlice;

        CODECHAL_ENCODE_ASSERT(numRegions != 0 && numRegions <= CODECHAL_MEDIA_WALKER_MAX_COLORS); // Making sure that the number of regions is at least 1

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

    uint16_t datatmp[32][32] = { {0} };
    for (uint32_t k = 0; k < numSlices; k++)
    {
        int32_t nearestReg = 0;
        int32_t minDelta = m_frameHeight;
        int32_t curLcuPelY  = regionsStartTable[regionStartYOffset + (k * m_numRegionsInSlice)] << log2LCUSize;
        int32_t tsWidth = frameWidthInUnits;
        int32_t tsHeight = height;
        int32_t offsetY = -((tsWidth + 1) >> 1);
        int32_t offsetDelta = ((tsWidth + ((tsHeight - 1) << 1)) + (m_numRegionsInSlice - 1)) / (m_numRegionsInSlice);

        for (int32_t i = 0; i < (int32_t)numRegions; i++)
        {
            if (regionsStartTable[i] == 0)
            {
                int32_t delta = curLcuPelY - (regionsStartTable[regionStartYOffset + i] << log2LCUSize);

                if (delta >= 0)
                {
                    if (delta < minDelta)
                    {
                        minDelta = delta;
                        nearestReg = i;
                    }
                }
            }
        }

        for (uint32_t i = 0; i < m_numRegionsInSlice; i++)
        {
            datatmp[k * m_numRegionsInSlice + i][0] = (uint16_t)(sliceStartY[k] * frameWidthInUnits);
            datatmp[k * m_numRegionsInSlice + i][1] = (uint16_t)((k == (numSlices - 1)) ? (frameWidthInUnits * frameHeightInUnits) : sliceStartY[k + 1] * frameWidthInUnits);  //m_info.SliceStartAddr[k+1]
            datatmp[k * m_numRegionsInSlice + i][2] = (uint16_t)(k * m_numRegionsInSlice + i);
            if (!m_isMaxLcu64 && m_numRegionsInSlice == 1)
            {
                continue;
            }
            datatmp[k * m_numRegionsInSlice + i][3] = (uint16_t)height;
            datatmp[k * m_numRegionsInSlice + i][4] = regionsStartTable[nearestReg + i];
            datatmp[k * m_numRegionsInSlice + i][5] = regionsStartTable[regionStartYOffset + (nearestReg + i)];
            datatmp[k * m_numRegionsInSlice + i][6] = regionsStartTable[regionStartYOffset + nearestReg];
            int32_t tmpY                            = regionsStartTable[regionStartYOffset + (nearestReg + m_numRegionsInSlice)];
            datatmp[k * m_numRegionsInSlice + i][7] = (uint16_t)((tmpY != 0) ? tmpY : (frameHeightInUnits));
            datatmp[k * m_numRegionsInSlice + i][8] = (uint16_t)(offsetY + regionsStartTable[regionStartYOffset + nearestReg] + ((i * offsetDelta) >> 1));
            if (m_isMaxLcu64)
            {
                datatmp[k * m_numRegionsInSlice + i][9]  = (uint16_t)((frameWidthInUnits + 2 * (maxHeight - 1) + m_numRegionsInSlice - 1) / m_numRegionsInSlice);
                datatmp[k * m_numRegionsInSlice + i][10] = (uint16_t)numRegions;
            }
         }
    }

    MOS_LOCK_PARAMS lockFlags;
    MOS_ZeroMemory(&lockFlags, sizeof(lockFlags));
    lockFlags.WriteOnly = true;

    PCODECHAL_ENC_HEVC_CONCURRENT_THREAD_GROUP_DATA_G10 region = (PCODECHAL_ENC_HEVC_CONCURRENT_THREAD_GROUP_DATA_G10)m_osInterface->pfnLockResource(
        m_osInterface,
        &m_concurrentThreadGroupData.sResource,
        &lockFlags);

    CODECHAL_ENCODE_CHK_NULL_RETURN(region);

    MOS_ZeroMemory(region, m_concurrentThreadGroupData.dwSize);

    //Concurrent Thread Group Surface size = 16*64
    for (auto i = 0; i < CODECHAL_MEDIA_WALKER_MAX_COLORS; i++)
    {
        MOS_SecureMemcpy((uint8_t*)region, copyBlockSize, (uint8_t*)datatmp[i], copyBlockSize);
        region++;
    }

    MOS_ZeroMemory(&m_walkingPatternParam, sizeof(m_walkingPatternParam));
    m_walkingPatternParam.dwMaxHeightInRegion = maxHeight;
    m_walkingPatternParam.dwNumRegion = numRegions;
    m_walkingPatternParam.dwNumUnitsInRegion  = (frameWidthInUnits + 2 * (maxHeight - 1) + m_numRegionsInSlice - 1) / m_numRegionsInSlice;

    m_osInterface->pfnUnlockResource(
        m_osInterface,
        &m_concurrentThreadGroupData.sResource);

    CODECHAL_DEBUG_TOOL(
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
        &m_concurrentThreadGroupData.sResource,
        CodechalDbgAttr::attrInput,
        "ConcurrentThreadGroupData_In",
        m_concurrentThreadGroupData.dwSize,
        0,
        m_pictureCodingType == I_TYPE ? CODECHAL_MEDIA_STATE_HEVC_I_MBENC : CODECHAL_MEDIA_STATE_HEVC_B_MBENC));
    )
    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG10::GetCustomDispatchPattern(
    PMHW_WALKER_PARAMS              walkerParams,
    PMHW_VFE_SCOREBOARD             scoreBoard,
    PCODECHAL_WALKER_CODEC_PARAMS   walkerCodecParams)
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(walkerParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(scoreBoard);
    CODECHAL_ENCODE_CHK_NULL_RETURN(walkerCodecParams);

    MOS_ZeroMemory(walkerParams, sizeof(*walkerParams));
    walkerParams->ColorCountMinusOne = m_walkingPatternParam.dwNumRegion - 1;
    CODECHAL_ENCODE_ASSERT(walkerParams->ColorCountMinusOne <= CODECHAL_MEDIA_WALKER_MAX_COLORS);

    walkerParams->WalkerMode               =
        (MHW_WALKER_MODE)walkerCodecParams->WalkerMode;
    walkerParams->UseScoreboard            = walkerCodecParams->bUseScoreboard;

    walkerParams->dwLocalLoopExecCount     = 0xFFF;  //MAX VALUE
    walkerParams->dwGlobalLoopExecCount    = 0xFFF;  //MAX VALUE

    MOS_ZeroMemory(scoreBoard, sizeof(*scoreBoard));
    switch(walkerCodecParams->WalkerDegree)
    {
    case CODECHAL_26_DEGREE:
        // Walker Params
        if (m_numRegionsInSlice > 1)
        {
            int32_t threadSpaceWidth  = walkerCodecParams->dwResolutionX;
            int32_t threadSpaceHeight = m_walkingPatternParam.dwMaxHeightInRegion;
            int32_t tsWidth  = threadSpaceWidth;
            int32_t tsHeight = threadSpaceHeight;
            int32_t tmpHeight = (tsHeight + 1) & 0xfffe;
            tsHeight = tmpHeight;
            tmpHeight                 = ((tsWidth + 1) >> 1) + ((tsWidth + ((tmpHeight - 1) << 1)) + (2 * m_numRegionsInSlice - 1)) / (2 * m_numRegionsInSlice);

            walkerParams->BlockResolution.x           = tsWidth;
            walkerParams->BlockResolution.y           = tmpHeight;

            walkerParams->GlobalStart.x               = 0;
            walkerParams->GlobalStart.y               = 0;

            walkerParams->GlobalResolution.x          = tsWidth;
            walkerParams->GlobalResolution.y          = tmpHeight;

            walkerParams->LocalStart.x                = (tsWidth + 1) & 0xfffe;
            walkerParams->LocalStart.y                = 0;

            walkerParams->LocalEnd.x                  = 0;
            walkerParams->LocalEnd.y                  = 0;

            walkerParams->GlobalOutlerLoopStride.x    = tsWidth;
            walkerParams->GlobalOutlerLoopStride.y    = 0;

            walkerParams->GlobalInnerLoopUnit.x       = 0;
            walkerParams->GlobalInnerLoopUnit.y       = tmpHeight;

            // 26 degree walking pattern
            walkerParams->ScoreboardMask              = 0x7F;
            walkerParams->LocalOutLoopStride.x        = 1;
            walkerParams->LocalOutLoopStride.y        = 0;
            walkerParams->LocalInnerLoopUnit.x        = MOS_BITFIELD_VALUE((uint32_t)-2, 16);    // Gen9: 0xFFE Gen6,8: 0x3FE
            walkerParams->LocalInnerLoopUnit.y        = 1;

            walkerParams->dwGlobalLoopExecCount       = 0;
            walkerParams->dwLocalLoopExecCount        = (threadSpaceWidth + (tsHeight - 1) * 2 + m_numRegionsInSlice - 1) / m_numRegionsInSlice;
        }
        else
        {
            walkerParams->BlockResolution.x        = walkerCodecParams->dwResolutionX;
            walkerParams->BlockResolution.y        = walkerCodecParams->dwResolutionY;

            walkerParams->GlobalResolution.x       = walkerParams->BlockResolution.x;
            walkerParams->GlobalResolution.y       = walkerParams->BlockResolution.y;

            walkerParams->GlobalOutlerLoopStride.x = walkerParams->BlockResolution.x;
            walkerParams->GlobalOutlerLoopStride.y = 0;

            walkerParams->GlobalInnerLoopUnit.x    = 0;
            walkerParams->GlobalInnerLoopUnit.y    = walkerParams->BlockResolution.y;

            // 26 degree walking pattern
            walkerParams->ScoreboardMask         = 0x7F;
            walkerParams->LocalOutLoopStride.x   = 1;
            walkerParams->LocalOutLoopStride.y   = 0;
            walkerParams->LocalInnerLoopUnit.x   = MOS_BITFIELD_VALUE((uint32_t)-2, 16);     // Gen9: 0xFFE Gen6,8: 0x3FE
            walkerParams->LocalInnerLoopUnit.y   = 1;
        }

        // Scoreboard Settings
        scoreBoard->ScoreboardMask       = 0x7F;
        scoreBoard->ScoreboardEnable     = true;

        // Scoreboard 0
        scoreBoard->ScoreboardDelta[0].x = MOS_BITFIELD_VALUE((uint32_t)-1, 4);
        scoreBoard->ScoreboardDelta[0].y =  0;

        // Scoreboard 1
        scoreBoard->ScoreboardDelta[1].x = MOS_BITFIELD_VALUE((uint32_t)-1, 4);
        scoreBoard->ScoreboardDelta[1].y = MOS_BITFIELD_VALUE((uint32_t)-1, 4);

        // Scoreboard 2
        scoreBoard->ScoreboardDelta[2].x =  0;
        scoreBoard->ScoreboardDelta[2].y = MOS_BITFIELD_VALUE((uint32_t)-1, 4);

        // Scoreboard 3
        scoreBoard->ScoreboardDelta[3].x =  1;
        scoreBoard->ScoreboardDelta[3].y = MOS_BITFIELD_VALUE((uint32_t)-1, 4);

        // Scoreboard 4
        scoreBoard->ScoreboardDelta[4].x =  0;
        scoreBoard->ScoreboardDelta[4].y =  0;

        // Scoreboard 5
        scoreBoard->ScoreboardDelta[5].x =  0;
        scoreBoard->ScoreboardDelta[5].y =  0;

        // Scoreboard 6
        scoreBoard->ScoreboardDelta[6].x =  0;
        scoreBoard->ScoreboardDelta[6].y =  0;

        // Scoreboard 7
        scoreBoard->ScoreboardDelta[7].x =  0;
        scoreBoard->ScoreboardDelta[7].y =  0;

        break;
    case CODECHAL_26Z_DEGREE:
        // Walker Params
        // 26z degree walking pattern used for HEVC
        walkerParams->ScoreboardMask           = 0x7f;

        walkerParams->GlobalResolution.x       = walkerCodecParams->dwResolutionX;
        walkerParams->GlobalResolution.y       = walkerCodecParams->dwResolutionY;

        // 26 degree in the global loop
        walkerParams->GlobalOutlerLoopStride.x = 2;
        walkerParams->GlobalOutlerLoopStride.y = 0;

        walkerParams->GlobalInnerLoopUnit.x    = 0xFFF -4 + 1; // -4 in 2's compliment format
        walkerParams->GlobalInnerLoopUnit.y    = 2;

        // z-order in the local loop
        walkerParams->LocalOutLoopStride.x     = 0;
        walkerParams->LocalOutLoopStride.y     = 1;
        walkerParams->LocalInnerLoopUnit.x     = 1;
        walkerParams->LocalInnerLoopUnit.y     = 0;

        // dispatch 4 threads together in one LCU
        walkerParams->BlockResolution.x        = 2;
        walkerParams->BlockResolution.y        = 2;

        // Scoreboard Settings
        scoreBoard->ScoreboardType             = m_hwScoreboardType;
        scoreBoard->ScoreboardMask             = 0x7F;
        scoreBoard->ScoreboardEnable           = true;

        // Scoreboard 0
        scoreBoard->ScoreboardDelta[0].x       = MOS_BITFIELD_VALUE((uint32_t)-1, 4);
        scoreBoard->ScoreboardDelta[0].y       =  1;

        // Scoreboard 1
        scoreBoard->ScoreboardDelta[1].x       = MOS_BITFIELD_VALUE((uint32_t)-1, 4);
        scoreBoard->ScoreboardDelta[1].y       =  0;

        // Scoreboard 2
        scoreBoard->ScoreboardDelta[2].x       = MOS_BITFIELD_VALUE((uint32_t)-1, 4);
        scoreBoard->ScoreboardDelta[2].y       = MOS_BITFIELD_VALUE((uint32_t)-1, 4);

        // Scoreboard 3
        scoreBoard->ScoreboardDelta[3].x       =  0;
        scoreBoard->ScoreboardDelta[3].y       = MOS_BITFIELD_VALUE((uint32_t)-1, 4);

        // Scoreboard 4
        scoreBoard->ScoreboardDelta[4].x       =  1;
        scoreBoard->ScoreboardDelta[4].y       = MOS_BITFIELD_VALUE((uint32_t)-1, 4);

        break;
    case CODECHAL_26X_DEGREE:
        // Walker Params
        if (m_numRegionsInSlice > 1)
        {
            int32_t threadSpaceWidth  = CODECHAL_GET_WIDTH_IN_BLOCKS(m_frameWidth, 32);
            int32_t tsWidth = threadSpaceWidth;
            int32_t tsHeight = m_walkingPatternParam.dwMaxHeightInRegion;
            int32_t tmpHeight = (tsHeight + 1) & 0xfffe;
            tsHeight =  tmpHeight;
            tmpHeight                 = ((tsWidth + 1) >> 1) + ((tsWidth + ((tmpHeight - 1) << 1)) + (2 * m_numRegionsInSlice - 1)) / (2 * m_numRegionsInSlice);
            tmpHeight *= (walkerCodecParams->usTotalThreadNumPerLcu);

            walkerParams->ScoreboardMask                   = 0xff;

            walkerParams->GlobalResolution.x               = tsWidth;
            walkerParams->GlobalResolution.y               = tmpHeight;

            walkerParams->GlobalStart.x                    = 0;
            walkerParams->GlobalStart.y                    = 0;

            walkerParams->LocalStart.x                     = (tsWidth + 1) & 0xfffe;
            walkerParams->LocalStart.y                     = 0;

            walkerParams->LocalEnd.x                       = 0;
            walkerParams->LocalEnd.y                       = 0;

            walkerParams->GlobalOutlerLoopStride.x         = tsWidth;
            walkerParams->GlobalOutlerLoopStride.y         = 0;

            walkerParams->GlobalInnerLoopUnit.x            = 0;
            walkerParams->GlobalInnerLoopUnit.y            = tmpHeight;

            walkerParams->LocalOutLoopStride.x             = 1;
            walkerParams->LocalOutLoopStride.y             = 0;
            walkerParams->LocalInnerLoopUnit.x             = MOS_BITFIELD_VALUE((uint32_t)-2, 16);
            walkerParams->LocalInnerLoopUnit.y             = walkerCodecParams->usTotalThreadNumPerLcu;
            walkerParams->MiddleLoopExtraSteps             = walkerCodecParams->usTotalThreadNumPerLcu - 1;
            walkerParams->MidLoopUnitX                     = 0;
            walkerParams->MidLoopUnitY                     = 1;

            walkerParams->BlockResolution.x                = walkerParams->GlobalResolution.x;
            walkerParams->BlockResolution.y                = walkerParams->GlobalResolution.y;

            walkerParams->dwGlobalLoopExecCount            = 0;
            walkerParams->dwLocalLoopExecCount             = (threadSpaceWidth + (tsHeight - 1) * 2 + m_numRegionsInSlice - 1) / m_numRegionsInSlice;
        }
        else
        {
            walkerParams->ScoreboardMask           = 0xff;

            walkerParams->GlobalResolution.x       = walkerCodecParams->dwResolutionX;
            walkerParams->GlobalResolution.y       = walkerCodecParams->dwResolutionY * walkerCodecParams->usTotalThreadNumPerLcu;

            walkerParams->GlobalOutlerLoopStride.x = walkerParams->GlobalResolution.x;
            walkerParams->GlobalOutlerLoopStride.y = 0;

            walkerParams->GlobalInnerLoopUnit.x    = 0;
            walkerParams->GlobalInnerLoopUnit.y    = walkerParams->GlobalResolution.y;

            walkerParams->LocalOutLoopStride.x     = 1;
            walkerParams->LocalOutLoopStride.y     = 0;
            walkerParams->LocalInnerLoopUnit.x     = 0xFFF - 2 + 1; // -2 in 2's compliment format;
            walkerParams->LocalInnerLoopUnit.y     = walkerCodecParams->usTotalThreadNumPerLcu;
            walkerParams->MiddleLoopExtraSteps     = walkerCodecParams->usTotalThreadNumPerLcu - 1;
            walkerParams->MidLoopUnitX             = 0;
            walkerParams->MidLoopUnitY             = 1;

            walkerParams->BlockResolution.x        = walkerParams->GlobalResolution.x;
            walkerParams->BlockResolution.y        = walkerParams->GlobalResolution.y;
        }

        // Scoreboard Settings
        scoreBoard->ScoreboardType             = m_hwScoreboardType;
        scoreBoard->ScoreboardMask             = 0xff;
        scoreBoard->ScoreboardEnable           = true;

        // Scoreboard 0
        scoreBoard->ScoreboardDelta[0].x       = MOS_BITFIELD_VALUE((uint32_t)-1, 4);
        scoreBoard->ScoreboardDelta[0].y       =  walkerCodecParams->usTotalThreadNumPerLcu - 1;

        // Scoreboard 1
        scoreBoard->ScoreboardDelta[1].x       = MOS_BITFIELD_VALUE((uint32_t)-1, 4);
        scoreBoard->ScoreboardDelta[1].y       = MOS_BITFIELD_VALUE((uint32_t)-1, 4);

        // Scoreboard 2
        scoreBoard->ScoreboardDelta[2].x       =  0;
        scoreBoard->ScoreboardDelta[2].y       = MOS_BITFIELD_VALUE((uint32_t)-1, 4);

        // Scoreboard 3
        scoreBoard->ScoreboardDelta[3].x       =  1;
        scoreBoard->ScoreboardDelta[3].y       = MOS_BITFIELD_VALUE((uint32_t)-1, 4);

        // Scoreboard 4
        scoreBoard->ScoreboardDelta[4].x       = 0;
        scoreBoard->ScoreboardDelta[4].y       = - walkerCodecParams->usTotalThreadNumPerLcu;

        // Scoreboard 5
        scoreBoard->ScoreboardDelta[5].x       = 0;
        scoreBoard->ScoreboardDelta[5].y       = MOS_BITFIELD_VALUE((uint32_t)-2, 4);

        // Scoreboard 6
        scoreBoard->ScoreboardDelta[6].x       = 0;
        scoreBoard->ScoreboardDelta[6].y       = MOS_BITFIELD_VALUE((uint32_t)-3, 4);

        // Scoreboard 7
        scoreBoard->ScoreboardDelta[7].x       = 0;
        scoreBoard->ScoreboardDelta[7].y       = MOS_BITFIELD_VALUE((uint32_t)-4, 4);

        break;
    case CODECHAL_26ZX_DEGREE:
    {
        const int32_t Mw_26zx_H_Factor = 5;
        // Walker Params
        if (m_numRegionsInSlice > 1)
        {
            int32_t threadSpaceWidth = CODECHAL_GET_WIDTH_IN_BLOCKS(m_frameWidth, 64);
            int32_t threadSpaceHeight = (m_walkingPatternParam.dwMaxHeightInRegion);
            int32_t spWidth = (threadSpaceWidth + 1) & 0xfffe;
            int32_t spHeight = (threadSpaceHeight + 1) & 0xfffe;
            int32_t numUnitInRegion   = (spWidth + (spHeight - 1) * 2 + m_numRegionsInSlice - 1) / m_numRegionsInSlice;
            spHeight                  = ((spWidth + 1) >> 1) + ((spWidth + ((spHeight - 1) << 1)) + (2 * m_numRegionsInSlice - 1)) / (2 * m_numRegionsInSlice);
            int32_t tsWidth = spWidth * Mw_26zx_H_Factor;
            int32_t tsHeight = spHeight * (walkerCodecParams->usTotalThreadNumPerLcu);

            walkerParams->ScoreboardMask = 0xff;

            walkerParams->GlobalResolution.x = tsWidth;
            walkerParams->GlobalResolution.y = tsHeight;

            walkerParams->GlobalStart.x = 0;
            walkerParams->GlobalStart.y = 0;

            walkerParams->LocalStart.x = walkerParams->GlobalResolution.x;
            walkerParams->LocalStart.y = 0;

            walkerParams->LocalEnd.x = 0;
            walkerParams->LocalEnd.y = 0;

            walkerParams->GlobalOutlerLoopStride.x = walkerParams->GlobalResolution.x;
            walkerParams->GlobalOutlerLoopStride.y = 0;

            walkerParams->GlobalInnerLoopUnit.x = 0;
            walkerParams->GlobalInnerLoopUnit.y = walkerParams->GlobalResolution.y;

            walkerParams->LocalOutLoopStride.x = 1;
            walkerParams->LocalOutLoopStride.y = 0;
            walkerParams->LocalInnerLoopUnit.x = 0xFFFF - Mw_26zx_H_Factor * 2 + 1;
            walkerParams->LocalInnerLoopUnit.y = walkerCodecParams->usTotalThreadNumPerLcu;
            walkerParams->MiddleLoopExtraSteps = walkerCodecParams->usTotalThreadNumPerLcu - 1;
            walkerParams->MidLoopUnitX = 0;
            walkerParams->MidLoopUnitY = 1;

            walkerParams->BlockResolution.x = walkerParams->GlobalResolution.x;
            walkerParams->BlockResolution.y = walkerParams->GlobalResolution.y;

            walkerParams->dwGlobalLoopExecCount = 0;
            walkerParams->dwLocalLoopExecCount = (numUnitInRegion + 1) * Mw_26zx_H_Factor;
        }
        else
        {
            walkerParams->ScoreboardMask = 0xff;

            walkerParams->GlobalResolution.x = walkerCodecParams->dwResolutionX * Mw_26zx_H_Factor;
            walkerParams->GlobalResolution.y = walkerCodecParams->dwResolutionY * walkerCodecParams->usTotalThreadNumPerLcu;

            walkerParams->GlobalOutlerLoopStride.x = walkerParams->GlobalResolution.x;
            walkerParams->GlobalOutlerLoopStride.y = 0;

            walkerParams->GlobalInnerLoopUnit.x = 0;
            walkerParams->GlobalInnerLoopUnit.y = walkerParams->GlobalResolution.y;

            walkerParams->LocalOutLoopStride.x = 1;
            walkerParams->LocalOutLoopStride.y = 0;
            walkerParams->LocalInnerLoopUnit.x = 0xFFF - 10 + 1; // -10 in 2's compliment format;
            walkerParams->LocalInnerLoopUnit.y = walkerCodecParams->usTotalThreadNumPerLcu;
            walkerParams->MiddleLoopExtraSteps = walkerCodecParams->usTotalThreadNumPerLcu - 1;
            walkerParams->MidLoopUnitX = 0;
            walkerParams->MidLoopUnitY = 1;

            walkerParams->BlockResolution.x = walkerParams->GlobalResolution.x;
            walkerParams->BlockResolution.y = walkerParams->GlobalResolution.y;
        }

        // Scoreboard Settings
        scoreBoard->ScoreboardType = m_hwScoreboardType;
        scoreBoard->ScoreboardMask = 0xff;
        scoreBoard->ScoreboardEnable = true;

        // Scoreboard 0
        scoreBoard->ScoreboardDelta[0].x = MOS_BITFIELD_VALUE((uint32_t)-5, 4);
        scoreBoard->ScoreboardDelta[0].y = MOS_BITFIELD_VALUE((uint32_t)-1, 4);

        // Scoreboard 1
        scoreBoard->ScoreboardDelta[1].x = MOS_BITFIELD_VALUE((uint32_t)-2, 4);
        scoreBoard->ScoreboardDelta[1].y = MOS_BITFIELD_VALUE((uint32_t)-1, 4);

        // Scoreboard 2
        scoreBoard->ScoreboardDelta[2].x = 3;
        scoreBoard->ScoreboardDelta[2].y = MOS_BITFIELD_VALUE((uint32_t)-1, 4);

        // Scoreboard 3
        scoreBoard->ScoreboardDelta[3].x = MOS_BITFIELD_VALUE((uint32_t)-1, 4);
        scoreBoard->ScoreboardDelta[3].y = 0;

        // Scoreboard 4
        scoreBoard->ScoreboardDelta[4].x = MOS_BITFIELD_VALUE((uint32_t)-2, 4);
        scoreBoard->ScoreboardDelta[4].y = walkerCodecParams->usTotalThreadNumPerLcu - 1;

        // Scoreboard 5
        scoreBoard->ScoreboardDelta[5].x = MOS_BITFIELD_VALUE((uint32_t)-5, 4);
        scoreBoard->ScoreboardDelta[5].y = walkerCodecParams->usTotalThreadNumPerLcu - 1;

        // Scoreboard 6
        scoreBoard->ScoreboardDelta[6].x = 0;
        scoreBoard->ScoreboardDelta[6].y = MOS_BITFIELD_VALUE((uint32_t)-1, 4);

        // Scoreboard 7
        scoreBoard->ScoreboardDelta[7].x = 5;
        scoreBoard->ScoreboardDelta[7].y = MOS_BITFIELD_VALUE((uint32_t)-1, 4);

        break;
    }
    default:
        eStatus = MOS_STATUS_INVALID_PARAMETER;
    }

#if 0
    // leave this for future debug
    CODECHAL_ENCODE_NORMALMESSAGE("Regions= = %d", dwNumRegionsInSlice);
    CODECHAL_ENCODE_NORMALMESSAGE("threads = %d", walkerCodecParams->usTotalThreadNumPerLcu);
    CODECHAL_ENCODE_NORMALMESSAGE("width,height = %d,%d", walkerCodecParams->dwResolutionX, walkerCodecParams->dwResolutionY);
    CODECHAL_ENCODE_NORMALMESSAGE("InterfaceDescriptorOffset = %d", walkerParams->InterfaceDescriptorOffset);
    CODECHAL_ENCODE_NORMALMESSAGE("UseScoreboard = %d", walkerParams->UseScoreboard);
    CODECHAL_ENCODE_NORMALMESSAGE("ScoreboardMask = %d", walkerParams->ScoreboardMask);
    CODECHAL_ENCODE_NORMALMESSAGE("ColorCountMinusOne = %d", walkerParams->ColorCountMinusOne);
    CODECHAL_ENCODE_NORMALMESSAGE("GroupIdLoopSelect = %d", walkerParams->GroupIdLoopSelect);
    CODECHAL_ENCODE_NORMALMESSAGE("GlobalResolution.x = %d", walkerParams->GlobalResolution.x);
    CODECHAL_ENCODE_NORMALMESSAGE("GlobalResolution.y = %d", walkerParams->GlobalResolution.y);
    CODECHAL_ENCODE_NORMALMESSAGE("GlobalStart.x = %d", walkerParams->GlobalStart.x);
    CODECHAL_ENCODE_NORMALMESSAGE("GlobalStart.y = %d", walkerParams->GlobalStart.y);
    CODECHAL_ENCODE_NORMALMESSAGE("LocalStart.x = %d", walkerParams->LocalStart.x);
    CODECHAL_ENCODE_NORMALMESSAGE("LocalStart.y = %d", walkerParams->LocalStart.y);
    CODECHAL_ENCODE_NORMALMESSAGE("LocalEnd.x = %d", walkerParams->LocalEnd.x);
    CODECHAL_ENCODE_NORMALMESSAGE("LocalEnd.y = %d", walkerParams->LocalEnd.y);
    CODECHAL_ENCODE_NORMALMESSAGE("GlobalOutlerLoopStride.x = %d", walkerParams->GlobalOutlerLoopStride.x);
    CODECHAL_ENCODE_NORMALMESSAGE("GlobalOutlerLoopStride.y = %d", walkerParams->GlobalOutlerLoopStride.y);
    CODECHAL_ENCODE_NORMALMESSAGE("GlobalInnerLoopUnit.x = %d", walkerParams->GlobalInnerLoopUnit.x);
    CODECHAL_ENCODE_NORMALMESSAGE("GlobalInnerLoopUnit.y = %d", walkerParams->GlobalInnerLoopUnit.y);
    CODECHAL_ENCODE_NORMALMESSAGE("LocalOutLoopStride.x = %d", walkerParams->LocalOutLoopStride.x);
    CODECHAL_ENCODE_NORMALMESSAGE("LocalOutLoopStride.y = %d", walkerParams->LocalOutLoopStride.y);
    CODECHAL_ENCODE_NORMALMESSAGE("LocalInnerLoopUnit.x = %d", walkerParams->LocalInnerLoopUnit.x);
    CODECHAL_ENCODE_NORMALMESSAGE("LocalInnerLoopUnit.y = %d", walkerParams->LocalInnerLoopUnit.y);
    CODECHAL_ENCODE_NORMALMESSAGE("MiddleLoopExtraSteps = %d", walkerParams->MiddleLoopExtraSteps);
    CODECHAL_ENCODE_NORMALMESSAGE("MidLoopUnitX = %d", walkerParams->MidLoopUnitX);
    CODECHAL_ENCODE_NORMALMESSAGE("MidLoopUnitY = %d", walkerParams->MidLoopUnitY);
    CODECHAL_ENCODE_NORMALMESSAGE("BlockResolution.x = %d", walkerParams->BlockResolution.x);
    CODECHAL_ENCODE_NORMALMESSAGE("BlockResolution.y = %d", walkerParams->BlockResolution.y);
    CODECHAL_ENCODE_NORMALMESSAGE("dwGlobalLoopExecCount = %d", walkerParams->dwGlobalLoopExecCount);
    CODECHAL_ENCODE_NORMALMESSAGE("dwLocalLoopExecCount = %d", walkerParams->dwLocalLoopExecCount);
    CODECHAL_ENCODE_NORMALMESSAGE("scoreBoard->ScoreboardDelta[0].x = %d", scoreBoard->ScoreboardDelta[0].x);
    CODECHAL_ENCODE_NORMALMESSAGE("scoreBoard->ScoreboardDelta[0].y = %d", scoreBoard->ScoreboardDelta[0].y);
    CODECHAL_ENCODE_NORMALMESSAGE("scoreBoard->ScoreboardDelta[1].x = %d", scoreBoard->ScoreboardDelta[1].x);
    CODECHAL_ENCODE_NORMALMESSAGE("scoreBoard->ScoreboardDelta[1].y = %d", scoreBoard->ScoreboardDelta[1].y);
    CODECHAL_ENCODE_NORMALMESSAGE("scoreBoard->ScoreboardDelta[2].x = %d", scoreBoard->ScoreboardDelta[2].x);
    CODECHAL_ENCODE_NORMALMESSAGE("scoreBoard->ScoreboardDelta[2].y = %d", scoreBoard->ScoreboardDelta[2].y);
    CODECHAL_ENCODE_NORMALMESSAGE("scoreBoard->ScoreboardDelta[3].x = %d", scoreBoard->ScoreboardDelta[3].x);
    CODECHAL_ENCODE_NORMALMESSAGE("scoreBoard->ScoreboardDelta[3].y = %d", scoreBoard->ScoreboardDelta[3].y);
    CODECHAL_ENCODE_NORMALMESSAGE("scoreBoard->ScoreboardDelta[4].x = %d", scoreBoard->ScoreboardDelta[4].x);
    CODECHAL_ENCODE_NORMALMESSAGE("scoreBoard->ScoreboardDelta[4].y = %d", scoreBoard->ScoreboardDelta[4].y);
    CODECHAL_ENCODE_NORMALMESSAGE("scoreBoard->ScoreboardDelta[5].x = %d", scoreBoard->ScoreboardDelta[5].x);
    CODECHAL_ENCODE_NORMALMESSAGE("scoreBoard->ScoreboardDelta[5].y = %d", scoreBoard->ScoreboardDelta[5].y);
    CODECHAL_ENCODE_NORMALMESSAGE("scoreBoard->ScoreboardDelta[6].x = %d", scoreBoard->ScoreboardDelta[6].x);
    CODECHAL_ENCODE_NORMALMESSAGE("scoreBoard->ScoreboardDelta[6].y = %d", scoreBoard->ScoreboardDelta[6].y);
    CODECHAL_ENCODE_NORMALMESSAGE("scoreBoard->ScoreboardDelta[7].x = %d", scoreBoard->ScoreboardDelta[7].x);
    CODECHAL_ENCODE_NORMALMESSAGE("scoreBoard->ScoreboardDelta[7].y = %d", scoreBoard->ScoreboardDelta[7].y);
#endif

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG10::EncodeMbEncKernel(CODECHAL_MEDIA_STATE_TYPE encFunctionType)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    PerfTagSetting perfTag;
    CODECHAL_ENCODE_SET_PERFTAG_INFO(perfTag, CODECHAL_ENCODE_PERFTAG_CALL_MBENC_KERNEL);

    // Initialize DSH kernel state
    PMHW_KERNEL_STATE kernelState = nullptr;
    CODECHAL_WALKER_DEGREE walkerDegree = CODECHAL_NO_DEGREE;
    uint32_t totalThreadNumPerLcu = 1;
    bool customDispatchPattern = false, verticalDispatch = false;
    uint32_t walkerResolutionX = 0, walkerResolutionY = 0;
    uint32_t               numRegionsInSliceSave = m_numRegionsInSlice;  // Save the original value in HEVC state

    switch(encFunctionType)
    {
    case CODECHAL_MEDIA_STATE_HEVC_I_MBENC:
        kernelState = &m_mbEncKernelStates[MBENC_I_KRNIDX];
        if (m_isMaxLcu64)
        {
            m_numRegionsInSlice   = 1;
            walkerDegree = CODECHAL_26_DEGREE;
            customDispatchPattern = false;
        }
        else
        {
            verticalDispatch = true;
        }
        walkerResolutionX = MOS_ALIGN_CEIL(m_frameWidth, 32) >> 5;
        walkerResolutionY = MOS_ALIGN_CEIL(m_frameHeight, 32) >> 5;
        break;

    case CODECHAL_MEDIA_STATE_HEVC_LCU64_B_MBENC:
        kernelState           = &m_mbEncKernelStates[MBENC_B_LCU64_KRNIDX];
        walkerDegree = CODECHAL_26ZX_DEGREE;
        walkerResolutionX = MOS_ALIGN_CEIL(m_frameWidth, MAX_LCU_SIZE) >> 6;
        walkerResolutionY = MOS_ALIGN_CEIL(m_frameHeight, MAX_LCU_SIZE) >> 6;
        totalThreadNumPerLcu = m_totalNumThreadsPerLcu;
        customDispatchPattern = true;
        break;

    case CODECHAL_MEDIA_STATE_HEVC_B_MBENC:
        kernelState           = &m_mbEncKernelStates[MBENC_B_LCU32_KRNIDX];
        walkerDegree          = (m_hevcSeqParams->TargetUsage == 7) ? CODECHAL_26_DEGREE : CODECHAL_26X_DEGREE;
        walkerResolutionX = MOS_ALIGN_CEIL(m_frameWidth, 32) >> 5;
        walkerResolutionY = MOS_ALIGN_CEIL(m_frameHeight, 32) >> 5;
        totalThreadNumPerLcu = m_totalNumThreadsPerLcu;
        customDispatchPattern = true;
        break;

    default:
        CODECHAL_ENCODE_ASSERTMESSAGE("Unsupported MB Enc Media State type");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    CODECHAL_WALKER_CODEC_PARAMS walkerCodecParams;
    MOS_ZeroMemory(&walkerCodecParams, sizeof(walkerCodecParams));
    walkerCodecParams.WalkerMode             = m_walkerMode;
    walkerCodecParams.bUseScoreboard         = m_useHwScoreboard;
    walkerCodecParams.dwResolutionX          = walkerResolutionX;
    walkerCodecParams.dwResolutionY          = walkerResolutionY;
    walkerCodecParams.dwNumSlices            = m_numSlices;
    walkerCodecParams.WalkerDegree           = walkerDegree;
    walkerCodecParams.bUseVerticalRasterScan = verticalDispatch;
    walkerCodecParams.usTotalThreadNumPerLcu = (uint16_t)totalThreadNumPerLcu;

    MHW_WALKER_PARAMS walkerParams;
    MHW_VFE_SCOREBOARD scoreBoard;

    m_numRegionsInSlice = numRegionsInSliceSave;  // Restore the original value to HEVC state

    if(customDispatchPattern)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(GetCustomDispatchPattern(&walkerParams, &scoreBoard, &walkerCodecParams));
    }
    else
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalInitMediaObjectWalkerParams(
            m_hwInterface,
            &walkerParams,
            &walkerCodecParams));
    }

    // If Single Task Phase is not enabled, use BT count for the kernel state.
    if (m_firstTaskInPhase || !m_singleTaskPhaseSupported)
    {
        uint32_t maxBtCount = m_singleTaskPhaseSupported ?
            m_maxBtCount : kernelState->KernelParams.iBTCount;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnRequestSshSpaceForCmdBuf(
            m_stateHeapInterface,
            maxBtCount));
        m_vmeStatesSize = m_hwInterface->GetKernelLoadCommandSize(maxBtCount);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(VerifySpaceAvailable());
    }

    // Set up the DSH/SSH as normal
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->AssignDshAndSshSpace(
        m_stateHeapInterface,
        kernelState,
        false,
        0,
        false,
        m_storeData));

    MHW_INTERFACE_DESCRIPTOR_PARAMS idParams;
    MOS_ZeroMemory(&idParams, sizeof(idParams));
    idParams.pKernelState = kernelState;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSetInterfaceDescriptor(
        m_stateHeapInterface,
        1,
        &idParams));

    // Generate Lcu Level Data
    CODECHAL_ENCODE_CHK_STATUS_RETURN(GenerateLcuLevelData());

    // setup curbe
    switch(encFunctionType)
    {
    case CODECHAL_MEDIA_STATE_HEVC_I_MBENC:
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetCurbeMbEncIKernel());
        break;

    case CODECHAL_MEDIA_STATE_HEVC_LCU64_B_MBENC:
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetCurbeMbEncBKernel());
        break;

    case CODECHAL_MEDIA_STATE_HEVC_B_MBENC:
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetCurbeMbEncBKernel());
        break;

    default:
        CODECHAL_ENCODE_ASSERTMESSAGE("Unsupported MBENC type requested");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            encFunctionType,
            MHW_DSH_TYPE,
            kernelState));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCurbe(
            encFunctionType,
            kernelState));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            encFunctionType,
            MHW_ISH_TYPE,
            kernelState));
    )

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    SendKernelCmdsParams sendKernelCmdsParams       = SendKernelCmdsParams();
    sendKernelCmdsParams.EncFunctionType            = encFunctionType;
    sendKernelCmdsParams.pKernelState               = kernelState;
    sendKernelCmdsParams.bEnableCustomScoreBoard    = customDispatchPattern;
    sendKernelCmdsParams.pCustomScoreBoard          = customDispatchPattern ? &scoreBoard : nullptr;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendGenericKernelCmds(&cmdBuffer, &sendKernelCmdsParams));

    // Add binding table
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSetBindingTable(
        m_stateHeapInterface,
        kernelState));

    // send surfaces
    switch(encFunctionType)
    {
    case CODECHAL_MEDIA_STATE_HEVC_I_MBENC:
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SendMbEncSurfacesIKernel(&cmdBuffer));
        break;

    case CODECHAL_MEDIA_STATE_HEVC_LCU64_B_MBENC:
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SendMbEncSurfacesBKernel(&cmdBuffer));
        break;

    case CODECHAL_MEDIA_STATE_HEVC_B_MBENC:
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SendMbEncSurfacesBKernel(&cmdBuffer));
        break;

    default:
        CODECHAL_ENCODE_ASSERTMESSAGE("Unsupported MBENC type requested");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    CODECHAL_DEBUG_TOOL(
        const uint8_t dumpBufNum = 2;
        PCODECHAL_ENCODE_BUFFER outBufs[dumpBufNum];
        outBufs[0] = &m_lcuLevelInputData;
        outBufs[1] = &m_concurrentThreadGroupData;

        if (m_pictureCodingType == I_TYPE)
        {
            const char * bufNames[dumpBufNum];
            bufNames[0] = "HEVC_I_MBENC_LcuLevelData_In";
            bufNames[1] = "HEVC_I_MBENC_ConcurrentThreadGroupData_In";

            for (uint8_t i = 0; i < dumpBufNum; i++)
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                    &outBufs[i]->sResource,
                    CodechalDbgAttr::attrInput,
                    bufNames[i],
                    outBufs[i]->dwSize,
                    0,
                    CODECHAL_MEDIA_STATE_HEVC_I_MBENC));
            }
        }
        else
        {
            const char * bufNames[dumpBufNum];
            bufNames[0] = "HEVC_B_MBENC_LcuLevelData_In";
            bufNames[1] = "HEVC_B_MBENC_ConcurrentThreadGroupData_In";

            for (uint8_t i = 0; i < dumpBufNum; i++)
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                    &outBufs[i]->sResource,
                    CodechalDbgAttr::attrInput,
                    bufNames[i],
                    outBufs[i]->dwSize,
                    0,
                    CODECHAL_MEDIA_STATE_HEVC_B_MBENC));
            }
        }
    )

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_renderEngineInterface->AddMediaObjectWalkerCmd(
        &cmdBuffer,
        &walkerParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(EndStatusReport(&cmdBuffer, encFunctionType));

    // Add dump for MBEnc surface state heap here
    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            encFunctionType,
            MHW_SSH_TYPE,
            kernelState));
    )

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSubmitBlocks(
        m_stateHeapInterface,
        kernelState));
    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnUpdateGlobalCmdBufId(
            m_stateHeapInterface));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(
            &cmdBuffer,
            nullptr));
    }
    CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
        &cmdBuffer,
        encFunctionType,
        nullptr)));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->UpdateSSEuForCmdBuffer(&cmdBuffer, m_singleTaskPhaseSupported, m_lastTaskInPhase));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, m_renderContextUsesNullHw);
        m_lastTaskInPhase = false;
    }

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG10::EncodeKernelFunctions()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (m_pictureCodingType == P_TYPE)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("GEN10 HEVC VME does not support P slice");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    if (m_cscDsState->RequireCsc())
    {
        m_firstTaskInPhase = true;
        CodechalEncodeCscDs::KernelParams cscScalingKernelParams;
        // Csc ARGB linear to NV12 Tile Y studio range
        MOS_ZeroMemory(&cscScalingKernelParams, sizeof(cscScalingKernelParams));
        cscScalingKernelParams.cscOrCopyOnly = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cscDsState->CscKernel(&cscScalingKernelParams));
    }

    CODECHAL_DEBUG_TOOL(
        if (!m_is10BitHevc){
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
                m_rawSurfaceToEnc,
                CodechalDbgAttr::attrEncodeRawInputSurface,
                "SrcSurf"))})

    if (m_pakOnlyTest)
    {
        // Skip all ENC kernel operations for now it is in the PAK only test mode.
        // PAK and CU records will be passed via the app
        return eStatus;
    }

    // BRC init/reset needs to be called before HME since it will reset the Brc Distortion surface
    // BRC init is called even for CQP mode hence also checking for first frame flag
    if (m_brcInit || m_brcReset || m_firstFrame)
    {
        if (!m_cscDsState->RequireCsc())
        {
            m_firstTaskInPhase = m_lastTaskInPhase=true;
        }

        CODECHAL_ENCODE_CHK_STATUS_RETURN(EncodeBrcInitResetKernel());
        m_brcInit = m_brcReset = false;
    }

    CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
        &m_brcBuffers.resBrcHistoryBuffer,
        CodechalDbgAttr::attrOutput,
        "Output_History",
        m_brcHistoryBufferSize,
        0,
        CODECHAL_MEDIA_STATE_BRC_INIT_RESET)));

    CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpSurface(
        &m_brcBuffers.sMeBrcDistortionBuffer,
        CodechalDbgAttr::attrOutput,
        "Output_Distortion",
        CODECHAL_MEDIA_STATE_BRC_INIT_RESET)));

    // Scaled surfaces are required to run both HME and IFrameDist
    bool scalingEnabled = (m_scalingEnabled || m_isMaxLcu64);

    if (scalingEnabled || m_is10BitHevc)
    {
        //Use a different performance tag ID for scaling and HME
        m_osInterface->pfnResetPerfBufferID(m_osInterface);

        m_firstTaskInPhase = true;
        m_lastTaskInPhase  = false;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(PerformScalingAndConversion());
    }

    if (m_hmeEnabled)
    {
        if (m_b16XMeEnabled)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(EncodeMeKernel(HME_LEVEL_16x, HEVC_ME_DIST_TYPE_INTER_BRC_DIST));
        }
        CODECHAL_ENCODE_CHK_STATUS_RETURN(EncodeMeKernel(HME_LEVEL_4x, HEVC_ME_DIST_TYPE_INTER_BRC_DIST));
    }

    // Getting Intra distortion for I-frame
    if(m_pictureCodingType == I_TYPE)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(EncodeMeKernel(HME_LEVEL_4x, HEVC_ME_DIST_TYPE_INTRA_BRC_DIST));
    }

    // Calling the Me Kernel for both Intra and Inter Frames to get Intra distortion
    m_lastTaskInPhase = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(EncodeMeKernel(HME_LEVEL_4x, HEVC_ME_DIST_TYPE_INTRA));

    CODECHAL_DEBUG_TOOL(
        if (m_is10BitHevc) {
            //Dump format converted surface
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
                &m_formatConvertedSurface[0],
                CodechalDbgAttr::attrEncodeRawInputSurface,
                "SrcSurf"))
        }

        if (m_hmeEnabled) {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                &m_s4XMeMvDataBuffer.OsResource,
                CodechalDbgAttr::attrOutput,
                "MvData",
                m_s4XMeMvDataBuffer.dwHeight * m_s4XMeMvDataBuffer.dwPitch,
                CodecHal_PictureIsBottomField(m_currOriginalPic) ? MOS_ALIGN_CEIL((m_downscaledWidthInMb4x * 32), 64) * (m_downscaledFrameFieldHeightInMb4x * 4) : 0,
                CODECHAL_MEDIA_STATE_4X_ME));

            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                &m_s4XMeDistortionBuffer.OsResource,
                CodechalDbgAttr::attrOutput,
                "MeDist",
                m_s4XMeDistortionBuffer.dwHeight * m_s4XMeDistortionBuffer.dwPitch,
                CodecHal_PictureIsBottomField(m_currOriginalPic) ? MOS_ALIGN_CEIL((m_downscaledWidthInMb4x * 8), 64) * MOS_ALIGN_CEIL((m_downscaledFrameFieldHeightInMb4x * 4 * 10), 8) : 0,
                CODECHAL_MEDIA_STATE_4X_ME));

            if (m_b16XMeEnabled)
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                    &m_s16XMeMvDataBuffer.OsResource,
                    CodechalDbgAttr::attrOutput,
                    "MvData",
                    m_s16XMeMvDataBuffer.dwHeight * m_s16XMeMvDataBuffer.dwPitch,
                    CodecHal_PictureIsBottomField(m_currOriginalPic) ? MOS_ALIGN_CEIL((m_downscaledWidthInMb16x * 32), 64) * (m_downscaledFrameFieldHeightInMb16x * 4) : 0,
                    CODECHAL_MEDIA_STATE_16X_ME));

                if (m_b32XMeEnabled)
                {
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                        &m_s32XMeMvDataBuffer.OsResource,
                        CodechalDbgAttr::attrOutput,
                        "MvData",
                        m_s32XMeMvDataBuffer.dwHeight * m_s32XMeMvDataBuffer.dwPitch,
                        CodecHal_PictureIsBottomField(m_currOriginalPic) ? MOS_ALIGN_CEIL((m_downscaledWidthInMb32x * 32), 64) * (m_downscaledFrameFieldHeightInMb32x * 4) : 0,
                        CODECHAL_MEDIA_STATE_32X_ME));
                }
            }
        })

    //Reset to use a different performance tag ID
    m_osInterface->pfnResetPerfBufferID(m_osInterface);

    m_firstTaskInPhase = true;
    m_lastTaskInPhase  = false;

    // Wait for PAK if necessary
    CODECHAL_ENCODE_CHK_STATUS_RETURN(WaitForPak());

    // BrcFrameEncUpdate and BrcLcuUpdate kernels are called even if CQP
    CODECHAL_ENCODE_CHK_STATUS_RETURN(EncodeBrcFrameUpdateKernel());
    CODECHAL_ENCODE_CHK_STATUS_RETURN(EncodeBrcLcuUpdateKernel());

    // Reset to use a different performance tag ID
    m_osInterface->pfnResetPerfBufferID(m_osInterface);

    m_lastTaskInPhase  = true;

    if (m_hevcPicParams->CodingType == I_TYPE)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(EncodeMbEncKernel(CODECHAL_MEDIA_STATE_HEVC_I_MBENC));
    }
    else
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(GenerateWalkingControlRegion());

        CODECHAL_ENCODE_CHK_STATUS_RETURN(EncodeMbEncKernel(
            m_isMaxLcu64 ? CODECHAL_MEDIA_STATE_HEVC_LCU64_B_MBENC : CODECHAL_MEDIA_STATE_HEVC_B_MBENC));
    }

    // Notify PAK engine once ENC is done
    if (!m_pakOnlyTest && !Mos_ResourceIsNull(&m_resSyncObjectRenderContextInUse))
    {
        MOS_SYNC_PARAMS syncParams = g_cInitSyncParams;
        syncParams.GpuContext = m_renderContext;
        syncParams.presSyncResource = &m_resSyncObjectRenderContextInUse;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineSignal(m_osInterface, &syncParams));
    }

    CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
        &m_brcBuffers.resBrcHistoryBuffer,
        CodechalDbgAttr::attrOutput,
        "Output_HistoryBuffer",
        m_brcHistoryBufferSize,
        0,
        CODECHAL_MEDIA_STATE_BRC_UPDATE)));

    CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
        &m_brcBuffers.resBrcPakStatisticBuffer[m_brcBuffers.uiCurrBrcPakStasIdxForRead],
        CodechalDbgAttr::attrOutput,
        "Output_PakStats",
        m_brcPakStatisticsSize,
        0,
        CODECHAL_MEDIA_STATE_BRC_UPDATE)));

    CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
        &m_brcBuffers.resBrcImageStatesReadBuffer[m_currRecycledBufIdx],
        CodechalDbgAttr::attrOutput,
        "Output_ImgStateRead",
        m_brcBuffers.dwBrcHcpPicStateSize,
        0,
        CODECHAL_MEDIA_STATE_BRC_UPDATE)));

    CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
        &m_brcBuffers.resBrcImageStatesWriteBuffer[m_currRecycledBufIdx],
        CodechalDbgAttr::attrOutput,
        "Output_ImgStateWrite",
        m_brcBuffers.dwBrcHcpPicStateSize,
        0,
        CODECHAL_MEDIA_STATE_BRC_UPDATE)));

    CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
        (MOS_RESOURCE*)m_allocator->GetResource(m_standard, brcInputForEncKernel),
        CodechalDbgAttr::attrOutput,
        "Output_CombinedEnc",
        m_brcCombinedEncBufferSize,
        0,
        CODECHAL_MEDIA_STATE_BRC_UPDATE)));

    CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpSurface(
        &m_brcBuffers.sBrcConstantDataBuffer[m_currRecycledBufIdx],
        CodechalDbgAttr::attrOutput,
        "Output_ConstData",
        CODECHAL_MEDIA_STATE_BRC_UPDATE)));

    CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpSurface(
        &m_brcBuffers.sMeBrcDistortionBuffer,
        CodechalDbgAttr::attrOutput,
        "Output_Distortion",
        CODECHAL_MEDIA_STATE_BRC_UPDATE)));

    CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpSurface(
        &m_cuSplitSurface,
        CodechalDbgAttr::attrOutput,
        "CUSplitSurface",
        CODECHAL_MEDIA_STATE_HEVC_B_MBENC)));

    CODECHAL_DEBUG_TOOL(
        const uint8_t dumpBufNum = 5;
        PCODECHAL_ENCODE_BUFFER outBufs[dumpBufNum];
            outBufs[0] = &m_64x64DistortionSurface;
            outBufs[1] = &m_encConstantTableForB;
            outBufs[2] = &m_jobQueueHeaderSurfaceForBLcu64;
            outBufs[3] = &m_jobQueueHeaderSurfaceForB;
            outBufs[4] = &m_encConstantTableForLcu64B;

        if (m_pictureCodingType == I_TYPE)
        {
            const char * bufNames[dumpBufNum];
            bufNames[0] = "DIST_64x64";
            bufNames[1] = "LUT_LCU32";
            bufNames[2] = "JobQueueSurfaceHeader_BLcu64";
            bufNames[3] = "JobQueueSurfaceHeader_B";
            bufNames[4] = "LUT_LCU64";

            for (uint8_t i = 0; i < dumpBufNum; i++)
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                    &outBufs[i]->sResource,
                    CodechalDbgAttr::attrOutput,
                    bufNames[i],
                    outBufs[i]->dwSize,
                    0,
                    CODECHAL_MEDIA_STATE_HEVC_I_MBENC));
            }

            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpSurface(
                &m_jobQueueDataSurfaceForBLcu64,
                CodechalDbgAttr::attrOutput,
                "JobQueueDataSurface_BLcu64",
                CODECHAL_MEDIA_STATE_HEVC_I_MBENC));
        }
        else
        {

            const char * bufNames[dumpBufNum];
            bufNames[0] = "DIST_64x64";
            bufNames[1] = "LUT_LCU32";
            bufNames[2] = "JobQueueSurfaceHeader_BLcu64";
            bufNames[3] = "JobQueueSurfaceHeader_B";
            bufNames[4] = "LUT_LCU64";

            for (uint8_t i = 0; i < dumpBufNum; i++)
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                    &outBufs[i]->sResource,
                    CodechalDbgAttr::attrOutput,
                    bufNames[i],
                    outBufs[i]->dwSize,
                    0,
                    CODECHAL_MEDIA_STATE_HEVC_B_MBENC));
            }

            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpSurface(
                &m_jobQueueDataSurfaceForBLcu64,
                CodechalDbgAttr::attrOutput,
                "JobQueueDataSurface_BLcu64",
                CODECHAL_MEDIA_STATE_HEVC_B_MBENC));

            if (m_isMaxLcu64)
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpSurface(
                    &m_jobQueueDataSurfaceForBLcu64Cu32,
                    CodechalDbgAttr::attrOutput,
                    "JobQueueDataSurface_Lcu64_Cu32",
                    CODECHAL_MEDIA_STATE_HEVC_B_MBENC));
            }
        }
    )

    CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpSurface(
        &m_mbStatisticsSurface,
        CodechalDbgAttr::attrOutput,
        "Output_mbstats",
        CODECHAL_MEDIA_STATE_HEVC_BRC_LCU_UPDATE)));

    CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpSurface(
        &m_mbSplitSurface,
        CodechalDbgAttr::attrOutput,
        "Output_mbsplit",
        CODECHAL_MEDIA_STATE_HEVC_BRC_LCU_UPDATE)));

    CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
        &m_mvAndDistortionSumSurface.sResource,
        CodechalDbgAttr::attrOutput,
        "Output_hmemv",
        m_mvAndDistortionSumSurface.dwSize,
        0,
        CODECHAL_MEDIA_STATE_HEVC_BRC_LCU_UPDATE)));

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG10::GetKernelHeaderAndSize(
    void                           *binary,
    EncOperation                   operation,
    uint32_t                       krnStateIdx,
    void                           *krnHeader,
    uint32_t                       *krnSize)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(binary);
    CODECHAL_ENCODE_CHK_NULL_RETURN(krnHeader);
    CODECHAL_ENCODE_CHK_NULL_RETURN(krnSize);

    PCODECHAL_ENC_HEVC_KERNEL_HEADER_G10 kernelHeaderTable = (PCODECHAL_ENC_HEVC_KERNEL_HEADER_G10)binary;
    PCODECHAL_KERNEL_HEADER currKrnHeader = nullptr;
    switch (operation)
    {
    case ENC_SCALING_CONVERSION:
        currKrnHeader = &kernelHeaderTable->Gen10_HEVC_DS_Convert;
        break;

    case ENC_ME:
        currKrnHeader = &kernelHeaderTable->Gen10_HEVC_HME;
        break;

    case ENC_MBENC:
        {
            switch (krnStateIdx)
            {
            case MBENC_I_KRNIDX:
                currKrnHeader = &kernelHeaderTable->Gen10_HEVC_Intra;
                break;

            case MBENC_B_LCU32_KRNIDX:
                currKrnHeader = &kernelHeaderTable->Gen10_HEVC_Enc_B;
                break;

            case MBENC_B_LCU64_KRNIDX:
                currKrnHeader = &kernelHeaderTable->Gen10_HEVC_Enc_LCU64_B;
                break;

            default:
                CODECHAL_ENCODE_ASSERTMESSAGE("Unsupported ENC mode requested");
                eStatus = MOS_STATUS_INVALID_PARAMETER;
                return eStatus;
            }
        }
        break;

    case ENC_BRC:
        {
            switch (krnStateIdx)
            {
            case CODECHAL_HEVC_BRC_INIT:
                currKrnHeader = &kernelHeaderTable->Gen10_HEVC_brc_init;
                break;

            case CODECHAL_HEVC_BRC_RESET:
                currKrnHeader = &kernelHeaderTable->Gen10_HEVC_brc_reset;
                break;

            case CODECHAL_HEVC_BRC_FRAME_UPDATE:
                currKrnHeader = &kernelHeaderTable->Gen10_HEVC_brc_update;
                break;

            case CODECHAL_HEVC_BRC_LCU_UPDATE:
                currKrnHeader = &kernelHeaderTable->Gen10_HEVC_brc_lcuqp;
                break;

            default:
                CODECHAL_ENCODE_ASSERTMESSAGE("Unsupported BRC mode requested");
                eStatus = MOS_STATUS_INVALID_PARAMETER;
                return eStatus;
            }
        }
        break;

    default:
        CODECHAL_ENCODE_ASSERTMESSAGE("Unsupported ENC mode requested");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    *((PCODECHAL_KERNEL_HEADER)krnHeader) = *currKrnHeader;

    PCODECHAL_KERNEL_HEADER nextKrnHeader = (currKrnHeader + 1);
    PCODECHAL_KERNEL_HEADER invalidEntry = &(kernelHeaderTable->Gen10_HEVC_brc_blockcopy) + 1;
    uint32_t nextKrnOffset = *krnSize;

    if (nextKrnHeader < invalidEntry)
    {
        nextKrnOffset = nextKrnHeader->KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT;
    }
    *krnSize = nextKrnOffset - (currKrnHeader->KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG10::SetKernelParams(
    EncOperation      encOperation,
    PMHW_KERNEL_PARAM kernelParams,
    uint32_t          idx)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(kernelParams);

    kernelParams->iThreadCount = m_renderEngineInterface->GetHwCaps()->dwMaxThreads;
    kernelParams->iIdCount     = 1;

    auto curbeAlignment = m_stateHeapInterface->pStateHeapInterface->GetCurbeAlignment();
    switch (encOperation)
    {
    case ENC_SCALING_CONVERSION:
        kernelParams->iBTCount     = SCALING_CONVERSION_END - SCALING_CONVERSION_BEGIN;
        kernelParams->iCurbeLength = MOS_ALIGN_CEIL(sizeof(DsConvCurbeDataG10), (size_t)curbeAlignment);
        kernelParams->iBlockWidth  = 8;
        kernelParams->iBlockHeight = 8;
        break;
    case ENC_ME:
        kernelParams->iBTCount     = HME_END - HME_BEGIN;
        kernelParams->iCurbeLength = MOS_ALIGN_CEIL(sizeof(CODECHAL_ENC_HEVC_ME_CURBE_G10), (size_t)curbeAlignment);
        kernelParams->iBlockWidth  = 32;
        kernelParams->iBlockHeight = 32;
        break;
    case ENC_MBENC:
        {
            switch (idx)
            {
            case MBENC_I_KRNIDX:
                kernelParams->iBTCount     = MBENC_I_FRAME_END - MBENC_I_FRAME_BEGIN;
                kernelParams->iCurbeLength = MOS_ALIGN_CEIL(sizeof(CODECHAL_ENC_HEVC_MBENC_I_CURBE_G10), (size_t)curbeAlignment);
                kernelParams->iBlockWidth  = 32;
                kernelParams->iBlockHeight = 32;
                break;
            case MBENC_B_LCU32_KRNIDX:
                kernelParams->iBTCount     = MBENC_B_FRAME_LCU32_END - MBENC_B_FRAME_LCU32_BEGIN;
                kernelParams->iCurbeLength = MOS_ALIGN_CEIL(sizeof(CODECHAL_ENC_HEVC_MBENC_B_CURBE_G10), (size_t)curbeAlignment);
                kernelParams->iBlockWidth  = 32;
                kernelParams->iBlockHeight = 32;
                break;
            case MBENC_B_LCU64_KRNIDX:
                kernelParams->iBTCount     = MBENC_B_FRAME_LCU64_END - MBENC_B_FRAME_LCU64_BEGIN;
                kernelParams->iCurbeLength = MOS_ALIGN_CEIL(sizeof(CODECHAL_ENC_HEVC_MBENC_B_CURBE_G10), (size_t)curbeAlignment);
                kernelParams->iBlockWidth  = 64;
                kernelParams->iBlockHeight = 64;
                break;
            default:
                CODECHAL_ENCODE_ASSERT(false);
                eStatus = MOS_STATUS_INVALID_PARAMETER;
            }
        }
        break;
    case ENC_BRC:
        {
            switch (idx)
            {
            case CODECHAL_HEVC_BRC_INIT:
            case CODECHAL_HEVC_BRC_RESET:
                kernelParams->iBTCount     = BRC_INIT_RESET_END - BRC_INIT_RESET_BEGIN;
                kernelParams->iCurbeLength = MOS_ALIGN_CEIL(sizeof(CODECHAL_ENC_HEVC_BRC_INITRESET_CURBE_G10), (size_t)curbeAlignment);
                kernelParams->iBlockWidth  = CODECHAL_HEVC_FRAME_BRC_BLOCK_SIZE;
                kernelParams->iBlockHeight = CODECHAL_HEVC_FRAME_BRC_BLOCK_SIZE;
                break;
            case CODECHAL_HEVC_BRC_FRAME_UPDATE:
                kernelParams->iBTCount     = BRC_UPDATE_END - BRC_UPDATE_BEGIN;
                kernelParams->iCurbeLength = MOS_ALIGN_CEIL(sizeof(CODECHAL_ENC_HEVC_BRC_UPDATE_CURBE_G10), (size_t)curbeAlignment);
                kernelParams->iBlockWidth  = CODECHAL_HEVC_FRAME_BRC_BLOCK_SIZE;
                kernelParams->iBlockHeight = CODECHAL_HEVC_FRAME_BRC_BLOCK_SIZE;
                break;
            case CODECHAL_HEVC_BRC_LCU_UPDATE:
                kernelParams->iBTCount     = BRC_LCU_UPDATE_END - BRC_LCU_UPDATE_BEGIN;
                kernelParams->iCurbeLength = MOS_ALIGN_CEIL(sizeof(CODECHAL_ENC_HEVC_BRC_UPDATE_CURBE_G10), (size_t)curbeAlignment);
                kernelParams->iBlockWidth  = CODECHAL_HEVC_LCU_BRC_BLOCK_SIZE;
                kernelParams->iBlockHeight = CODECHAL_HEVC_LCU_BRC_BLOCK_SIZE;
                break;
            default:
                CODECHAL_ENCODE_ASSERT(false);
                eStatus = MOS_STATUS_INVALID_PARAMETER;
            }
        }
        break;
    default:
        CODECHAL_ENCODE_ASSERTMESSAGE("Unsupported ENC mode requested");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG10::SetBindingTable(
    EncOperation encOperation,
    PCODECHAL_ENCODE_BINDING_TABLE_GENERIC bindingTable,
    uint32_t idx)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(bindingTable);

    MOS_ZeroMemory(bindingTable, sizeof(*bindingTable));

    switch(encOperation)
    {
    case ENC_SCALING_CONVERSION:
        bindingTable->dwNumBindingTableEntries  = SCALING_CONVERSION_END - SCALING_CONVERSION_BEGIN;
        bindingTable->dwBindingTableStartOffset = SCALING_CONVERSION_BEGIN;
        break;
    case ENC_ME:
        bindingTable->dwNumBindingTableEntries  = HME_END - HME_BEGIN;
        bindingTable->dwBindingTableStartOffset = HME_BEGIN;
        break;
    case ENC_MBENC:
        {
            switch (idx)
            {
            case MBENC_I_KRNIDX:
                bindingTable->dwNumBindingTableEntries  = MBENC_I_FRAME_END - MBENC_I_FRAME_BEGIN;
                bindingTable->dwBindingTableStartOffset = MBENC_I_FRAME_BEGIN;
                break;
            case MBENC_B_LCU32_KRNIDX:
                bindingTable->dwNumBindingTableEntries  = MBENC_B_FRAME_LCU32_END - MBENC_B_FRAME_LCU32_BEGIN;
                bindingTable->dwBindingTableStartOffset = MBENC_B_FRAME_LCU32_BEGIN;
                break;
            case MBENC_B_LCU64_KRNIDX:
                bindingTable->dwNumBindingTableEntries  = MBENC_B_FRAME_LCU64_END - MBENC_B_FRAME_LCU64_BEGIN;
                bindingTable->dwBindingTableStartOffset = MBENC_B_FRAME_LCU64_BEGIN;
                break;
            default:
                CODECHAL_ENCODE_ASSERT(false);
                eStatus = MOS_STATUS_INVALID_PARAMETER;
                return eStatus;
            }
        }
        break;
    case ENC_BRC:
        {
            switch(idx)
            {
            case CODECHAL_HEVC_BRC_INIT:
                bindingTable->dwNumBindingTableEntries  = BRC_INIT_RESET_END - BRC_INIT_RESET_BEGIN;
                bindingTable->dwBindingTableStartOffset = BRC_INIT_RESET_BEGIN;
                break;
            case CODECHAL_HEVC_BRC_RESET:
                bindingTable->dwNumBindingTableEntries  = BRC_INIT_RESET_END - BRC_INIT_RESET_BEGIN;
                bindingTable->dwBindingTableStartOffset = BRC_INIT_RESET_BEGIN;
                break;
            case CODECHAL_HEVC_BRC_FRAME_UPDATE:
                bindingTable->dwNumBindingTableEntries  = BRC_UPDATE_END - BRC_UPDATE_BEGIN;
                bindingTable->dwBindingTableStartOffset = BRC_UPDATE_BEGIN;
                break;
            case CODECHAL_HEVC_BRC_LCU_UPDATE:
                bindingTable->dwNumBindingTableEntries  = BRC_LCU_UPDATE_END - BRC_LCU_UPDATE_BEGIN;
                bindingTable->dwBindingTableStartOffset = BRC_LCU_UPDATE_BEGIN;
                break;
            default:
                CODECHAL_ENCODE_ASSERT(false);
                eStatus = MOS_STATUS_INVALID_PARAMETER;
            }
        }
        break;
    default:
        CODECHAL_ENCODE_ASSERTMESSAGE("Unsupported ENC mode requested");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    for (uint32_t i = 0; i < bindingTable->dwNumBindingTableEntries; i++)
    {
        bindingTable->dwBindingTableEntries[i] = i;
    }

    return eStatus;
}

uint32_t CodechalEncHevcStateG10::GetMaxBtCount()
{
    auto btIdxAlignment = m_stateHeapInterface->pStateHeapInterface->GetBtIdxAlignment();

    uint32_t btCountPhase1 = MOS_ALIGN_CEIL(
        m_brcKernelStates[CODECHAL_HEVC_BRC_INIT].KernelParams.iBTCount,
        btIdxAlignment);

    // 4x, 16x DS, 4x ME, 16x ME. Me in Intra Mode
    uint32_t btCountPhase2 = 2 * (MOS_ALIGN_CEIL(m_scalingAndConversionKernelState->KernelParams.iBTCount, btIdxAlignment) +
                                     MOS_ALIGN_CEIL(m_meKernelState->KernelParams.iBTCount, btIdxAlignment)) +
                             MOS_ALIGN_CEIL(m_meKernelState->KernelParams.iBTCount, btIdxAlignment);

    // If 10 bit HEVC is supported, it might require 10 bit to 8 bit conversion of reference surface
    if (m_is10BitHevc)
    {
        btCountPhase2 += MOS_ALIGN_CEIL(m_scalingAndConversionKernelState->KernelParams.iBTCount, btIdxAlignment);
    }

    // I kernel
    uint32_t btCountPhase3 = MOS_ALIGN_CEIL(m_brcKernelStates[CODECHAL_HEVC_BRC_FRAME_UPDATE].KernelParams.iBTCount, btIdxAlignment) +
                             MOS_ALIGN_CEIL(m_brcKernelStates[CODECHAL_HEVC_BRC_LCU_UPDATE].KernelParams.iBTCount, btIdxAlignment) +
                             MOS_ALIGN_CEIL(m_mbEncKernelStates[MBENC_I_KRNIDX].KernelParams.iBTCount, btIdxAlignment);

    // B kernel
    uint32_t btCountPhase4 = MOS_ALIGN_CEIL(m_brcKernelStates[CODECHAL_HEVC_BRC_FRAME_UPDATE].KernelParams.iBTCount, btIdxAlignment) +
                             MOS_ALIGN_CEIL(m_brcKernelStates[CODECHAL_HEVC_BRC_LCU_UPDATE].KernelParams.iBTCount, btIdxAlignment) +
                             MOS_MAX(MOS_ALIGN_CEIL(m_mbEncKernelStates[MBENC_B_LCU64_KRNIDX].KernelParams.iBTCount, btIdxAlignment),
                                 MOS_ALIGN_CEIL(m_mbEncKernelStates[MBENC_B_LCU32_KRNIDX].KernelParams.iBTCount, btIdxAlignment));

    uint32_t maxBtCount = MOS_MAX(btCountPhase1, btCountPhase2);
    maxBtCount = MOS_MAX(maxBtCount, btCountPhase3);
    maxBtCount = MOS_MAX(maxBtCount, btCountPhase4);

    return maxBtCount;
}

MOS_STATUS CodechalEncHevcStateG10::InitKernelStateScalingAndConversion()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    m_scalingAndConversionKernelState = MOS_New(MHW_KERNEL_STATE);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_scalingAndConversionKernelState);

    m_scalingAndConversionKernelBindingTable = (PCODECHAL_ENCODE_BINDING_TABLE_GENERIC)MOS_AllocAndZeroMemory(sizeof(GenericBindingTable));
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_scalingAndConversionKernelBindingTable);

    PMHW_KERNEL_STATE kernelStatePtr = m_scalingAndConversionKernelState;
    uint32_t kernelSize = m_combinedKernelSize;

    CODECHAL_KERNEL_HEADER currKrnHeader;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(GetKernelHeaderAndSize(
        m_kernelBinary,
        ENC_SCALING_CONVERSION,
        0,
        &currKrnHeader,
        &kernelSize));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetKernelParams(
        ENC_SCALING_CONVERSION,
        &kernelStatePtr->KernelParams,
        0));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetBindingTable(
        ENC_SCALING_CONVERSION,
        m_scalingAndConversionKernelBindingTable,
        0));

    kernelStatePtr->dwCurbeOffset = m_stateHeapInterface->pStateHeapInterface->GetSizeofCmdInterfaceDescriptorData();
    kernelStatePtr->KernelParams.pBinary = m_kernelBinary + (currKrnHeader.KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);
    kernelStatePtr->KernelParams.iSize = kernelSize;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnCalculateSshAndBtSizesRequested(
        m_stateHeapInterface,
        kernelStatePtr->KernelParams.iBTCount,
        &kernelStatePtr->dwSshSize,
        &kernelStatePtr->dwBindingTableSize));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->MhwInitISH(m_stateHeapInterface, kernelStatePtr));

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG10::InitKernelStateMe()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    m_meKernelState = MOS_New(MHW_KERNEL_STATE);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_meKernelState);

    m_meKernelBindingTable = (PCODECHAL_ENCODE_BINDING_TABLE_GENERIC)MOS_AllocAndZeroMemory(sizeof(GenericBindingTable));
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_meKernelBindingTable);

    uint32_t kernelSize = m_combinedKernelSize;
    CODECHAL_KERNEL_HEADER currKrnHeader;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(GetKernelHeaderAndSize(
        m_kernelBinary,
        ENC_ME,
        0,
        &currKrnHeader,
        &kernelSize));

    PMHW_KERNEL_STATE kernelStatePtr = m_meKernelState;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetKernelParams(
        ENC_ME,
        &kernelStatePtr->KernelParams,
        0));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetBindingTable(
        ENC_ME,
        m_meKernelBindingTable,
        0));

    kernelStatePtr->dwCurbeOffset = m_stateHeapInterface->pStateHeapInterface->GetSizeofCmdInterfaceDescriptorData();
    kernelStatePtr->KernelParams.pBinary = m_kernelBinary + (currKrnHeader.KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);
    kernelStatePtr->KernelParams.iSize = kernelSize;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnCalculateSshAndBtSizesRequested(
        m_stateHeapInterface,
        kernelStatePtr->KernelParams.iBTCount,
        &kernelStatePtr->dwSshSize,
        &kernelStatePtr->dwBindingTableSize));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->MhwInitISH(m_stateHeapInterface, kernelStatePtr));

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG10::InitKernelStateBrc()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    m_numBrcKrnStates = CODECHAL_HEVC_BRC_NUM;
    m_brcKernelStates = MOS_NewArray(MHW_KERNEL_STATE, m_numBrcKrnStates);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_brcKernelStates);

    PMHW_KERNEL_STATE kernelStatePtr = m_brcKernelStates;
    kernelStatePtr++; // Skipping CODECHAL_HEVC_BRC_COARSE_INTRA as it not in Gen10

    m_brcKernelBindingTable = (PCODECHAL_ENCODE_BINDING_TABLE_GENERIC)MOS_AllocAndZeroMemory(
        sizeof(GenericBindingTable) * m_numBrcKrnStates);

    // krnStateIdx initialization starts at 1 as Gen10 does not support CODECHAL_HEVC_BRC_COARSE_INTRA kernel in BRC
    for (uint32_t krnStateIdx = 1; krnStateIdx < m_numBrcKrnStates; krnStateIdx++)
    {
        uint32_t kernelSize = m_combinedKernelSize;

        CODECHAL_KERNEL_HEADER currKrnHeader;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(GetKernelHeaderAndSize(
            m_kernelBinary,
            ENC_BRC,
            krnStateIdx,
            &currKrnHeader,
            &kernelSize));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetKernelParams(
            ENC_BRC,
            &kernelStatePtr->KernelParams,
            krnStateIdx));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetBindingTable(
            ENC_BRC,
            &m_brcKernelBindingTable[krnStateIdx],
            krnStateIdx));

        kernelStatePtr->dwCurbeOffset          = m_stateHeapInterface->pStateHeapInterface->GetSizeofCmdInterfaceDescriptorData();
        kernelStatePtr->KernelParams.pBinary   = m_kernelBinary + (currKrnHeader.KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);
        kernelStatePtr->KernelParams.iSize     = kernelSize;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnCalculateSshAndBtSizesRequested(
            m_stateHeapInterface,
            kernelStatePtr->KernelParams.iBTCount,
            &kernelStatePtr->dwSshSize,
            &kernelStatePtr->dwBindingTableSize));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->MhwInitISH(m_stateHeapInterface, kernelStatePtr));

        kernelStatePtr++;
    }

    kernelStatePtr = m_mbEncKernelStates;

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG10::InitKernelStateMbEnc()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    m_numMbEncEncKrnStates = MBENC_NUM_KRN;

    m_mbEncKernelStates = MOS_NewArray(MHW_KERNEL_STATE, m_numMbEncEncKrnStates);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_mbEncKernelStates);

    m_mbEncKernelBindingTable = (PCODECHAL_ENCODE_BINDING_TABLE_GENERIC)MOS_AllocAndZeroMemory(
        sizeof(GenericBindingTable) *
        m_numMbEncEncKrnStates);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_mbEncKernelBindingTable);

    PMHW_KERNEL_STATE kernelStatePtr = m_mbEncKernelStates;

    for (uint32_t krnStateIdx = 0; krnStateIdx < m_numMbEncEncKrnStates; krnStateIdx++)
    {
        uint32_t kernelSize = m_combinedKernelSize;
        CODECHAL_KERNEL_HEADER currKrnHeader;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(GetKernelHeaderAndSize(
            m_kernelBinary,
            ENC_MBENC,
            krnStateIdx,
            &currKrnHeader,
            &kernelSize));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetKernelParams(
            ENC_MBENC,
            &kernelStatePtr->KernelParams,
            krnStateIdx));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetBindingTable(
            ENC_MBENC,
            &m_mbEncKernelBindingTable[krnStateIdx],
            krnStateIdx));

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

void CodechalEncHevcStateG10::GetMaxRefFrames(uint8_t& maxNumRef0, uint8_t& maxNumRef1)
{
    maxNumRef0 = CODECHAL_ENCODE_HEVC_NUM_MAX_VME_L0_REF_G10;
    maxNumRef1 = CODECHAL_ENCODE_HEVC_NUM_MAX_VME_L1_REF_G10;

    return;
}

CodechalEncHevcStateG10::CodechalEncHevcStateG10(
    CodechalHwInterface* hwInterface,
    CodechalDebugInterface* debugInterface,
    PCODECHAL_STANDARD_INFO standardInfo)
    :CodechalEncHevcState(hwInterface, debugInterface, standardInfo)
{
    m_combinedDownScaleAndDepthConversion = true;
    m_2xMeSupported                       = true;
    m_fieldScalingOutputInterleaved       = false;
    m_brcHistoryBufferSize                = HEVC_BRC_HISTORY_BUFFER_SIZE_G10;
#ifndef _FULL_OPEN_SOURCE
    m_kernelBase                          = (uint8_t*)IGCODECKRN_G10;
#else
    m_kernelBase                          = nullptr;
#endif
    pfnGetKernelHeaderAndSize             = GetKernelHeaderAndSize;

    MOS_ZeroMemory(&m_kernelDebug, sizeof(m_kernelDebug));
    MOS_ZeroMemory(&m_intermediateCuRecordSurfaceLcu32, sizeof(m_intermediateCuRecordSurfaceLcu32));
    MOS_ZeroMemory(&m_secondIntermediateCuRecordSurfaceLcu32, sizeof(m_secondIntermediateCuRecordSurfaceLcu32));
    MOS_ZeroMemory(&m_intermediateCuRecordSurfaceLcu64B, sizeof(m_intermediateCuRecordSurfaceLcu64B));
    MOS_ZeroMemory(&m_encConstantTableForI, sizeof(m_encConstantTableForI));
    MOS_ZeroMemory(&m_encConstantTableForB, sizeof(m_encConstantTableForB));
    MOS_ZeroMemory(&m_encConstantTableForLcu64B, sizeof(m_encConstantTableForLcu64B));
    MOS_ZeroMemory(&m_lcuLevelInputData, sizeof(m_lcuLevelInputData));
    MOS_ZeroMemory(&m_lcuEncodingScratchSurface, sizeof(m_lcuEncodingScratchSurface));
    MOS_ZeroMemory(&m_lcuEncodingScratchSurfaceLcu64B, sizeof(m_lcuEncodingScratchSurfaceLcu64B));
    MOS_ZeroMemory(&m_64x64DistortionSurface, sizeof(m_64x64DistortionSurface));
    MOS_ZeroMemory(&m_scratchSurface, sizeof(m_scratchSurface));
    MOS_ZeroMemory(&m_concurrentThreadGroupData, sizeof(m_concurrentThreadGroupData));
    MOS_ZeroMemory(&m_jobQueueHeaderSurfaceForB      , sizeof(m_jobQueueHeaderSurfaceForB));       // when used by LCU64 kernel, it is the 1D header surface with smaller size
    MOS_ZeroMemory(&m_jobQueueHeaderSurfaceForBLcu64, sizeof(m_jobQueueHeaderSurfaceForBLcu64));
    MOS_ZeroMemory(&m_jobQueueDataSurfaceForBLcu64Cu32, sizeof(m_jobQueueDataSurfaceForBLcu64Cu32));
    MOS_ZeroMemory(&m_jobQueueDataSurfaceForBLcu64, sizeof(m_jobQueueDataSurfaceForBLcu64));
    MOS_ZeroMemory(&m_cuSplitSurface, sizeof(m_cuSplitSurface));
    MOS_ZeroMemory(&m_mbStatisticsSurface, sizeof(m_mbStatisticsSurface));
    MOS_ZeroMemory(&m_mbSplitSurface, sizeof(m_mbSplitSurface));
    MOS_ZeroMemory(&m_residualDataScratchSurfaceForBLcu32, sizeof(m_residualDataScratchSurfaceForBLcu32));
    MOS_ZeroMemory(&m_residualDataScratchSurfaceForBLcu64, sizeof(m_residualDataScratchSurfaceForBLcu64));
    MOS_ZeroMemory(&m_mvAndDistortionSumSurface, sizeof(m_mvAndDistortionSumSurface));
    MOS_ZeroMemory(&m_walkingPatternParam, sizeof(m_walkingPatternParam));

    m_hwInterface->GetStateHeapSettings()->dwNumSyncTags = CODECHAL_ENCODE_HEVC_NUM_SYNC_TAGS;
    m_hwInterface->GetStateHeapSettings()->dwDshSize = CODECHAL_INIT_DSH_SIZE_HEVC_ENC;

    m_kuid = IDR_CODEC_AllHEVCEnc;
    MOS_STATUS eStatus = CodecHalGetKernelBinaryAndSize(
        m_kernelBase,
        m_kuid,
        &m_kernelBinary,
        &m_combinedKernelSize);
    CODECHAL_ENCODE_ASSERT(eStatus == MOS_STATUS_SUCCESS);

    m_hwInterface->GetStateHeapSettings()->dwIshSize +=
        MOS_ALIGN_CEIL(m_combinedKernelSize, (1 << MHW_KERNEL_OFFSET_SHIFT));
}

MOS_STATUS CodechalEncHevcStateG10::Initialize(CodechalSetting * settings)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    // Common initialization
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncHevcState::Initialize(settings));

    m_b2NdSaoPassNeeded                     = true;
    m_brcBuffers.dwBrcConstantSurfaceWidth  = HEVC_BRC_CONSTANT_SURFACE_WIDTH_G9;
    m_brcBuffers.dwBrcConstantSurfaceHeight = m_brcConstantSurfaceHeight;
    m_maxNumSlicesSupported                 = CODECHAL_HEVC_MAX_NUM_SLICES_LVL_5;
    m_brcBuffers.dwBrcHcpPicStateSize       = BRC_IMG_STATE_SIZE_PER_PASS_G10 * CODECHAL_ENCODE_BRC_MAXIMUM_NUM_PASSES;
    m_brcBuffers.pMbStatisticsSurface       = &m_mbStatisticsSurface;
    m_brcBuffers.pMvAndDistortionSumSurface = &m_mvAndDistortionSumSurface;

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
        __MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_REGION_NUMBER_ID,
        &userFeatureData,
        m_osInterface->pOsContext);
    // Region number must be greater than 1
    m_numRegionsInSlice = (userFeatureData.i32Data < 1) ? 1 : userFeatureData.i32Data;

    if (m_numRegionsInSlice > 16)
    {
        // Region number cannot be larger than 16
        m_numRegionsInSlice = 16;
    }

    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_26Z_ENABLE_ID,
        &userFeatureData,
        m_osInterface->pOsContext);
    m_enable26WalkingPattern = (userFeatureData.i32Data) ? false : true;

    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_RDOQ_ENABLE_ID,
        &userFeatureData,
        m_osInterface->pOsContext);
    m_hevcRdoqEnabled = userFeatureData.i32Data ? true : false;

    m_hwScoreboardType = 1;

    if (m_codecFunction != CODECHAL_FUNCTION_PAK)
    {
        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
        userFeatureData.i32Data = 1;
        userFeatureData.i32DataFlag = MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE;
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_ME_ENABLE_ID,
            &userFeatureData,
            m_osInterface->pOsContext);
        m_hmeSupported = (userFeatureData.i32Data) ? true : false;

        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
        userFeatureData.i32Data = 1;
        userFeatureData.i32DataFlag = MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE;
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_16xME_ENABLE_ID,
            &userFeatureData,
            m_osInterface->pOsContext);
        m_16xMeSupported = (userFeatureData.i32Data) ? true : false;

        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_HEVC_NUM_THREADS_PER_LCU_ID,
            &userFeatureData,
            m_osInterface->pOsContext);
        m_totalNumThreadsPerLcu = userFeatureData.i32Data;

        if(m_totalNumThreadsPerLcu < m_minThreadsPerLcuB || m_totalNumThreadsPerLcu > m_maxThreadsPerLcuB)
        {
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            return eStatus;
        }
    }

    // Overriding the defaults here with 32 aligned dimensions
    // 2x Scaling WxH
    m_downscaledWidth2x                  =
        CODECHAL_GET_2xDS_SIZE_32ALIGNED(m_frameWidth);
    m_downscaledHeight2x                 =
        CODECHAL_GET_2xDS_SIZE_32ALIGNED(m_frameHeight);

    // HME Scaling WxH
    m_downscaledWidth4x                   =
        CODECHAL_GET_4xDS_SIZE_32ALIGNED(m_frameWidth);
    m_downscaledHeight4x                  =
        CODECHAL_GET_4xDS_SIZE_32ALIGNED(m_frameHeight);
    m_downscaledWidthInMb4x               =
        CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_downscaledWidth4x);
    m_downscaledHeightInMb4x              =
        CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_downscaledHeight4x);

    // SuperHME Scaling WxH
    m_downscaledWidth16x                  =
        CODECHAL_GET_4xDS_SIZE_32ALIGNED(m_downscaledWidth4x);
    m_downscaledHeight16x                 =
        CODECHAL_GET_4xDS_SIZE_32ALIGNED(m_downscaledHeight4x);
    m_downscaledWidthInMb16x              =
        CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_downscaledWidth16x);
    m_downscaledHeightInMb16x             =
        CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_downscaledHeight16x);

    // UltraHME Scaling WxH
    m_downscaledWidth32x                  =
        CODECHAL_GET_2xDS_SIZE_32ALIGNED(m_downscaledWidth16x);
    m_downscaledHeight32x                 =
        CODECHAL_GET_2xDS_SIZE_32ALIGNED(m_downscaledHeight16x);
    m_downscaledWidthInMb32x              =
        CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_downscaledWidth32x);
    m_downscaledHeightInMb32x             =
        CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_downscaledHeight32x);

    // Overriding default minimum scaled dimension allowed with VME restriction
    m_minScaledDimension     = m_minScaledSurfaceSize;
    m_minScaledDimensionInMb = (m_minScaledSurfaceSize + 15) >> 4;

    if (m_frameWidth < 128 || m_frameHeight < 128)
    {
        m_16xMeSupported = false;
        m_32xMeSupported = false;
    }
    else if (m_frameWidth < 512 || m_frameHeight < 512)
    {
        m_16xMeSupported = true;
        m_32xMeSupported = false;
    }
    else
    {
        m_16xMeSupported = true;
        m_32xMeSupported = false;  //disabling since uhme is not supported on CNL
    }

    // disable MMCD if we enable Codechal dump. Because dump code changes the surface state from compressed to uncompressed,
    // this causes mis-match issue between dump is enabled or disabled.
    CODECHAL_DEBUG_TOOL(
        if (m_mmcState && m_mmcState->IsMmcEnabled() && m_debugInterface && m_debugInterface->m_dbgCfgHead) {
            m_mmcState->SetMmcDisabled();
        })

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG10::InitKernelState()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    // Init kernel state
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitKernelStateScalingAndConversion());
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitKernelStateMe());
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitKernelStateMbEnc());
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitKernelStateBrc());

    return eStatus;
}
