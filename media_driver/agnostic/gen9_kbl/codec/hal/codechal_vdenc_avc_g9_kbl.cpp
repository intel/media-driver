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
//! \file     codechal_vdenc_avc_g9_kbl.cpp
//! \brief    This file implements the C++ class/interface for KBL 's AVC
//!           VDEnc encoding to be used CODECHAL components.
//!

#include "codechal_vdenc_avc_g9_kbl.h"
#if USE_CODECHAL_DEBUG_TOOL
#include "mhw_vdbox_mfx_hwcmd_g9_kbl.h"
#include "mhw_vdbox_vdenc_hwcmd_g9_kbl.h"
#endif

typedef struct _CODECHAL_ENCODE_AVC_KERNEL_HEADER_G9_KBL {
    int nKernelCount;

    // Quality mode for Frame/Field
    CODECHAL_KERNEL_HEADER AVCMBEnc_Qlty_I;
    CODECHAL_KERNEL_HEADER AVCMBEnc_Qlty_P;
    CODECHAL_KERNEL_HEADER AVCMBEnc_Qlty_B;
    // Normal mode for Frame/Field
    CODECHAL_KERNEL_HEADER AVCMBEnc_Norm_I;
    CODECHAL_KERNEL_HEADER AVCMBEnc_Norm_P;
    CODECHAL_KERNEL_HEADER AVCMBEnc_Norm_B;
    // Performance modes for Frame/Field
    CODECHAL_KERNEL_HEADER AVCMBEnc_Perf_I;
    CODECHAL_KERNEL_HEADER AVCMBEnc_Perf_P;
    CODECHAL_KERNEL_HEADER AVCMBEnc_Perf_B;
    // Modes for Frame/Field
    CODECHAL_KERNEL_HEADER AVCMBEnc_Adv_I;
    CODECHAL_KERNEL_HEADER AVCMBEnc_Adv_P;
    CODECHAL_KERNEL_HEADER AVCMBEnc_Adv_B;

    // HME
    CODECHAL_KERNEL_HEADER AVC_ME_P;
    CODECHAL_KERNEL_HEADER AVC_ME_B;

    // DownScaling
    CODECHAL_KERNEL_HEADER PLY_DScale_PLY;
    CODECHAL_KERNEL_HEADER PLY_DScale_2f_PLY_2f;

    // BRC init frame
    CODECHAL_KERNEL_HEADER InitFrameBRC;

    // Frame BRC update
    CODECHAL_KERNEL_HEADER FrameENCUpdate;

    // BRC Reset frame
    CODECHAL_KERNEL_HEADER BRC_ResetFrame;

    // BRC I Frame Distortion
    CODECHAL_KERNEL_HEADER BRC_IFrame_Dist;

    // BRCBlockCopy
    CODECHAL_KERNEL_HEADER BRCBlockCopy;

    // MbBRC Update
    CODECHAL_KERNEL_HEADER MbBRCUpdate;

    // 2x DownScaling
    CODECHAL_KERNEL_HEADER PLY_2xDScale_PLY;
    CODECHAL_KERNEL_HEADER PLY_2xDScale_2f_PLY_2f;

    //Motion estimation kernel for the VDENC StreamIN
    CODECHAL_KERNEL_HEADER AVC_ME_VDENC;

    //Weighted Prediction Kernel
    CODECHAL_KERNEL_HEADER AVC_WeightedPrediction;

    // Static frame detection Kernel
    CODECHAL_KERNEL_HEADER AVC_StaticFrameDetection;

} CODECHAL_ENCODE_AVC_KERNEL_HEADER_G9_KBL, *PCODECHAL_ENCODE_AVC_KERNEL_HEADER_G9_KBL;

typedef struct _CODECHAL_VDENC_AVC_BRC_INIT_DMEM_G9_KBL
{
    uint8_t     BRCFunc_U8;                           // 0: Init; 2: Reset
    uint8_t     OpenSourceEnable_U8;                  // 0: disable opensource, 1: enable opensource
    uint8_t     RVSD[2];
    uint16_t    INIT_BRCFlag_U16;                     // ICQ or CQP with slice size control: 0x00 CBR: 0x10; VBR: 0x20; VCM: 0x40; LOWDELAY: 0x80.
    uint16_t    Reserved;
    uint16_t    INIT_FrameWidth_U16;                  // Luma width in bytes
    uint16_t    INIT_FrameHeight_U16;                 // Luma height in bytes
    uint32_t    INIT_TargetBitrate_U32;               // target bitrate, set by application
    uint32_t    INIT_MinRate_U32;                     // 0
    uint32_t    INIT_MaxRate_U32;                     // Maximum bit rate in bits per second (bps).
    uint32_t    INIT_BufSize_U32;                     // buffer size
    uint32_t    INIT_InitBufFull_U32;                 // initial buffer fullness
    uint32_t    INIT_ProfileLevelMaxFrame_U32;        // user defined. refer to AVC BRC for conformance check and correction
    uint32_t    INIT_FrameRateM_U32;                  // FrameRateM is the number of frames in FrameRateD
    uint32_t    INIT_FrameRateD_U32;                  // If driver gets this FrameRateD from VUI, it is the num_units_in_tick field (32 bits unsigned integer).
    uint16_t    INIT_GopP_U16;                        // number of P frames in a GOP
    uint16_t    INIT_GopB_U16;                        // number of B frames in a GOP
    uint16_t    INIT_MinQP_U16;                       // 10
    uint16_t    INIT_MaxQP_U16;                       // 51
    int8_t      INIT_DevThreshPB0_S8[8];              // lowdelay ? (-45, -33, -23, -15, -8, 0, 15, 25) : (-46, -38, -30, -23, 23, 30, 40, 46)
    int8_t      INIT_DevThreshVBR0_S8[8];             // lowdelay ? (-45, -35, -25, -15, -8, 0, 20, 40) : (-46, -40, -32, -23, 56, 64, 83, 93)
    int8_t      INIT_DevThreshI0_S8[8];               // lowdelay ? (-40, -30, -17, -10, -5, 0, 10, 20) : (-43, -36, -25, -18, 18, 28, 38, 46)
    uint8_t     INIT_InitQPIP;                        // Initial QP for I and P

    uint8_t     INIT_NotUseRhoDm_U8;                  // Reserved
    uint8_t     INIT_InitQPB;                         // Initial QP for B
    uint8_t     INIT_MbQpCtrl_U8;                     // Enable MB level QP control (global)
    uint8_t     INIT_SliceSizeCtrlEn_U8;              // Enable slice size control
    int8_t      INIT_IntraQPDelta_I8[3];              // set to zero for all by default
    int8_t      INIT_SkipQPDelta_I8;                  // Reserved
    int8_t      INIT_DistQPDelta_I8[4];               // lowdelay ? (-5, -2, 2, 5) : (0, 0, 0, 0)
    uint8_t     INIT_OscillationQpDelta_U8;           // BRCFLAG_ISVCM ? 16 : 0
    uint8_t     INIT_HRDConformanceCheckDisable_U8;   // BRCFLAG_ISAVBR ? 1 : 0
    uint8_t     INIT_SkipFrameEnableFlag;
    uint8_t     INIT_TopQPDeltaThrForAdapt2Pass_U8;   // =1. QP Delta threshold for second pass.
    uint8_t     INIT_TopFrmSzThrForAdapt2Pass_U8;     // lowdelay ? 10 : 50. Top frame size threshold for second pass
    uint8_t     INIT_BotFrmSzThrForAdapt2Pass_U8;     // lowdelay ? 10 : 200. Bottom frame size threshold for second pass
    uint8_t     INIT_QPSelectForFirstPass_U8;         // lowdelay ? 0 : 1. =0 to use previous frame final QP; or =1 to use (targetQP + previousQP) / 2.
    uint8_t     INIT_MBHeaderCompensation_U8;         // Reserved
    uint8_t     INIT_OverShootCarryFlag_U8;           // set to zero by default
    uint8_t     INIT_OverShootSkipFramePct_U8;        // set to zero by default
    uint8_t     INIT_EstRateThreshP0_U8[7];           // 4, 8, 12, 16, 20, 24, 28
    uint8_t     INIT_EstRateThreshB0_U8[7];           // 4, 8, 12, 16, 20, 24, 28
    uint8_t     INIT_EstRateThreshI0_U8[7];           // 4, 8, 12, 16, 20, 24, 28
    uint8_t     INIT_FracQPEnable_U8;                 // ExtendedRhoDomainEn from par file
    uint8_t     INIT_ScenarioInfo_U8;                 // 0: UNKNOWN, 1: DISPLAYREMOTING, 2: VIDEOCONFERENCE, 3: ARCHIVE, 4: LIVESTREAMING.
    uint8_t     INIT_StaticRegionStreamIn_U8;         // should be programmed from par file
    uint8_t     INIT_DeltaQP_Adaptation_U8;           // =1, should be programmed from par file
    uint8_t     INIT_MaxCRFQualityFactor_U8;          // =52, should be programmed from par file
    uint8_t     INIT_CRFQualityFactor_U8;             // =25, should be programmed from par file
    uint8_t     INIT_BotQPDeltaThrForAdapt2Pass_U8;   // =1. QP Delta threshold for second pass.
    uint8_t     INIT_SlidingWindowSize_U8;            // =30, the window size (in frames) used to compute bit rate
    uint8_t     INIT_SlidingWidowRCEnable_U8;         // =0, sliding window based rate control (SWRC) disabled, 1: enabled
    uint8_t     INIT_SlidingWindowMaxRateRatio_U8;    // =120, ratio between the max rate within the window and average target bitrate
    uint8_t     INIT_LowDelayGoldenFrameBoost_U8;     // only for lowdelay mode, 0 (default): no boost for I and scene change frames, 1: boost
    uint8_t     INIT_AdaptiveCostEnable_U8;           // 0: disabled, 1: enabled
    uint8_t     INIT_AdaptiveHMEExtensionEnable_U8;   // 0: disabled, 1: enabled
    uint8_t     INIT_ICQReEncode_U8;                  // 0: disabled, 1: enabled
    uint8_t     INIT_SliceSizeCtrlWA;                 // 0: disabled, 1: enabled
    uint8_t     INIT_SinglePassOnly;                  // 0: disabled, 1: enabled
    uint8_t     RSVD2[56];                            // must be zero
} CODECHAL_VDENC_AVC_BRC_INIT_DMEM_G9_KBL, *PCODECHAL_VDENC_AVC_BRC_INIT_DMEM_G9_KBL;

typedef struct __CODECHAL_VDENC_AVC_BRC_UPDATE_DMEM_G9_KBL
{
    uint8_t     BRCFunc_U8;                           // =1 for Update, other values are reserved for future use
    uint8_t     RSVD[3];
    uint32_t    UPD_TARGETSIZE_U32;                   // refer to AVC BRC for calculation
    uint32_t    UPD_FRAMENUM_U32;                     // frame number
    uint32_t    UPD_PeakTxBitsPerFrame_U32;           // current global target bits - previous global target bits (global target bits += input bits per frame)
    uint32_t    UPD_FrameBudget_U32;                  // target time counter
    uint32_t    FrameByteCount;                       // PAK output via MMIO
    uint32_t    TimingBudgetOverflow;                 // PAK output via MMIO
    uint32_t    ImgStatusCtrl;                        // PAK output via MMIO
    uint32_t    IPCMNonConformant;                    // PAK output via MMIO

    uint16_t    UPD_startGAdjFrame_U16[4];            // 10, 50, 100, 150
    uint16_t    UPD_MBBudget_U16[52];                 // MB bugdet for QP 0 � 51.
    uint16_t    UPD_SLCSZ_TARGETSLCSZ_U16;            // target slice size
    uint16_t    UPD_SLCSZ_UPD_THRDELTAI_U16[42];      // slice size threshold delta for I frame
    uint16_t    UPD_SLCSZ_UPD_THRDELTAP_U16[42];      // slice size threshold delta for P frame
    uint16_t    UPD_NumOfFramesSkipped_U16;           // Recording how many frames have been skipped.
    uint16_t    UPD_SkipFrameSize_U16;                 // Recording the skip frame size for one frame. =NumMBs * 1, assuming one bit per mb for skip frame.
    uint16_t    UPD_StaticRegionPct_U16;              // One entry, recording the percentage of static region
    uint8_t     UPD_gRateRatioThreshold_U8[7];        // 80,95,99,101,105,125,160
    uint8_t     UPD_CurrFrameType_U8;                 // I frame: 2; P frame: 0; B frame: 1.
    uint8_t     UPD_startGAdjMult_U8[5];              // 1, 1, 3, 2, 1
    uint8_t     UPD_startGAdjDiv_U8[5];               // 40, 5, 5, 3, 1
    uint8_t     UPD_gRateRatioThresholdQP_U8[8];      // 253,254,255,0,1,1,2,3
    uint8_t     UPD_PAKPassNum_U8;                    // current pak pass number
    uint8_t     UPD_MaxNumPass_U8;                    // 2
    uint8_t     UPD_SceneChgWidth_U8[2];              // set both to MIN((NumP + 1) / 5, 6)
    uint8_t     UPD_SceneChgDetectEn_U8;              // Enable scene change detection
    uint8_t     UPD_SceneChgPrevIntraPctThreshold_U8; // =96. scene change previous intra percentage threshold
    uint8_t     UPD_SceneChgCurIntraPctThreshold_U8;  // =192. scene change current intra percentage threshold
    uint8_t     UPD_IPAverageCoeff_U8;                // lowdelay ? 0 : 128
    uint8_t     UPD_MinQpAdjustment_U8;               // Minimum QP increase step
    uint8_t     UPD_TimingBudgetCheck_U8;             // Flag indicating if kernel will check timing budget.
    int8_t      reserved_I8[4];                       // must be zero
    uint8_t     UPD_CQP_QpValue_U8;                   // Application specified target QP in BRC_ICQ mode
    uint8_t     UPD_CQP_FracQp_U8;                    // Application specified fine position in BRC_ICQ mode
    uint8_t     UPD_HMEDetectionEnable_U8;            // 0: default, 1: HuC BRC kernel requires information from HME detection kernel output
    uint8_t     UPD_HMECostEnable_U8;                 // 0: default, 1: driver provides HME cost table
    uint8_t     UPD_DisablePFrame8x8Transform_U8;     // 0: enable, 1: disable
    uint8_t     UPD_SklCabacWAEnable_U8;              // 0: disable, 1: enable
    uint8_t     UPD_ROISource_U8;                     // =0: disable, 1: ROIMap from HME Static Region or from App dirty rectangle, 2: ROIMap from App
    uint8_t     UPD_SLCSZ_ConsertativeThreshold_U8;   // =0, 0: do not set conservative threshold (suggested for video conference) 1: set conservative threshold for non-video conference
    uint16_t    UPD_TargetSliceSize_U16;              // default: 1498, max target slice size from app DDI
    uint16_t    UPD_MaxNumSliceAllowed_U16;           // computed by driver based on level idc
    uint16_t    UPD_SLBB_Size_U16;                    // second level batch buffer (SLBB) size in bytes, the input buffer will contain two SLBBs A and B, A followed by B, A and B have the same structure.
    uint16_t    UPD_SLBB_B_Offset_U16;                // offset in bytes from the beginning of the input buffer, it points to the start of SLBB B, set by driver for skip frame support
    uint16_t    UPD_AvcImgStateOffset_U16;            // offset in bytes from the beginning of SLBB A
    uint16_t    reserved_u16;
    uint32_t    NumOfSlice;                           // PAK output via MMIO

                                                    /* HME distortion based QP adjustment */
    uint16_t    AveHmeDist_U16;                       // default: 0, in HME detection kernel output
    uint8_t     HmeDistAvailable_U8;                  // 0: disabled, 1: enabled
    uint8_t     reserved_u8;                          // must be zero
    uint16_t    AdditionalFrameSize_U16;              // for slice size control improvement
    uint8_t     AddNALHeaderSizeInternally_U8;
    uint8_t     UPD_RoiQpViaForceQp_U8;               // HuC does not update StreamIn Buffer, 1: HuC updates StreamIn Buffer
    uint32_t    CABACZeroInsertionSize_U32;           // PAK output via MMIO
    uint32_t    MiniFramePaddingSize_U32;             // PAK output via MMIO
    uint16_t    UPD_WidthInMB_U16;                    // width in MB
    uint16_t    UPD_HeightInMB_U16;                   // height in MB
    int8_t      UPD_ROIQpDelta_I8[8];                 // Application specified ROI QP Adjustment for Zone0, Zone1, Zone2 and Zone3, Zone4, Zone5, Zone6 and Zone7.
    uint8_t     RSVD2[36];
} _CODECHAL_VDENC_AVC_BRC_UPDATE_DMEM_G9_KBL, *P_CODECHAL_VDENC_AVC_BRC_UPDATE_DMEM_G9_KBL;

const uint32_t CodechalVdencAvcStateG9Kbl::MV_Cost_SkipBias_QPel[3][8] =
{
    // for normal case
    { 0, 6, 6, 9, 10, 13, 14, 16 },
    // for QP = 47,48,49
    { 0, 6, 6, 6, 6, 7, 8, 8 },
    // for QP = 50,51
    { 0, 6, 6, 6, 6, 7, 7, 7 }
};

const uint32_t CodechalVdencAvcStateG9Kbl::HmeCost_DisplayRemote[8][CODEC_AVC_NUM_QP] =
{
    //mv=0
    {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     //QP=[0 ~12]
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     //QP=[13 ~25]
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     //QP=[26 ~38]
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0      //QP=[39 ~51]
    },
    //mv<=16
    {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     //QP=[0 ~12]
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     //QP=[13 ~25]
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     //QP=[26 ~38]
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0      //QP=[39 ~51]
    },
    //mv<=32
    {
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,     //QP=[0 ~12]
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,     //QP=[13 ~25]
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,     //QP=[26 ~38]
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1      //QP=[39 ~51]
    },
    //mv<=64
    {
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,     //QP=[0 ~12]
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,     //QP=[13 ~25]
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,     //QP=[26 ~38]
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5      //QP=[39 ~51]
    },
    //mv<=128
    {
        10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,     //QP=[0 ~12]
        10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,     //QP=[13 ~25]
        10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,     //QP=[26 ~38]
        10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10      //QP=[39 ~51]
    },
    //mv<=256
    {
        10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,     //QP=[0 ~12]
        10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,     //QP=[13 ~25]
        10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,     //QP=[26 ~38]
        10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10      //QP=[39 ~51]
    },
    //mv<=512
    {
        20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,     //QP=[0 ~12]
        20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,     //QP=[13 ~25]
        20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,     //QP=[26 ~38]
        20, 20, 20, 20, 20, 30, 30, 30, 30, 30, 30, 30, 30      //QP=[39 ~51]
    },
    //mv<=1024
    {
        20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,     //QP=[0 ~12]
        20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,     //QP=[13 ~25]
        20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,     //QP=[26 ~38]
        20, 20, 20, 30, 40, 50, 50, 50, 50, 50, 50, 50, 50      //QP=[39 ~51]
    }
};

const uint32_t CodechalVdencAvcStateG9Kbl::HmeCost[8][CODEC_AVC_NUM_QP] =
{
    //mv=0
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,       //QP=[0 ~12]
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,         //QP=[13 ~25]
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,         //QP=[26 ~38]
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0          //QP=[39 ~51]
    },
    //mv<=16
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,       //QP=[0 ~12]
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,         //QP=[13 ~25]
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,         //QP=[26 ~38]
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0          //QP=[39 ~51]
    },
    //mv<=32
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,       //QP=[0 ~12]
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,         //QP=[13 ~25]
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,         //QP=[26 ~38]
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1          //QP=[39 ~51]
    },
    //mv<=64
    { 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,       //QP=[0 ~12]
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,         //QP=[13 ~25]
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,         //QP=[26 ~38]
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5          //QP=[39 ~51]
    },
    //mv<=128
    { 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,       //QP=[0 ~12]
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,         //QP=[13 ~25]
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,         //QP=[26 ~38]
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10          //QP=[39 ~51]
    },
    //mv<=256
    { 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,       //QP=[0 ~12]
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,         //QP=[13 ~25]
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,         //QP=[26 ~38]
    10, 10, 10, 10, 20, 30, 40, 50, 50, 50, 50, 50, 50          //QP=[39 ~51]
    },
    //mv<=512
    { 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,       //QP=[0 ~12]
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,         //QP=[13 ~25]
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,         //QP=[26 ~38]
    20, 20, 20, 40, 60, 80, 100, 100, 100, 100, 100, 100, 100   //QP=[39 ~51]
    },
    //mv<=1024
    { 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,       //QP=[0 ~12]
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,         //QP=[13 ~25]
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,         //QP=[26 ~38]
    20, 20, 30, 50, 100, 200, 200, 200, 200, 200, 200, 200, 200 //QP=[39 ~51]
    }
};

const int8_t CodechalVdencAvcStateG9Kbl::BRC_INIT_DistQPDelta_I8[4] =
{
    0, 0, 0, 0
};

const int8_t CodechalVdencAvcStateG9Kbl::BRC_INIT_DistQPDelta_I8_LowDelay[4] =
{
    -5, -2, 2, 5
};

CodechalVdencAvcStateG9Kbl::CodechalVdencAvcStateG9Kbl(
        CodechalHwInterface *   hwInterface,
        CodechalDebugInterface *debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo) : CodechalVdencAvcStateG9(hwInterface, debugInterface, standardInfo)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    this->pfnGetKernelHeaderAndSize = EncodeGetKernelHeaderAndSize;

    m_vdencBrcInitDmemBufferSize   = sizeof(CODECHAL_VDENC_AVC_BRC_INIT_DMEM_G9_KBL);
    m_vdencBrcUpdateDmemBufferSize = sizeof(_CODECHAL_VDENC_AVC_BRC_UPDATE_DMEM_G9_KBL);
    m_vdencBrcNumOfSliceOffset = CODECHAL_OFFSETOF(_CODECHAL_VDENC_AVC_BRC_UPDATE_DMEM_G9_KBL, NumOfSlice);
}

CodechalVdencAvcStateG9Kbl::~CodechalVdencAvcStateG9Kbl()
{
}

MOS_STATUS CodechalVdencAvcStateG9Kbl::InitializeState()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalVdencAvcState::InitializeState());

    m_sliceSizeStreamoutSupported = true;

    return eStatus;
}

MOS_STATUS CodechalVdencAvcStateG9Kbl::EncodeGetKernelHeaderAndSize(void *binary, EncOperation operation, uint32_t krnStateIdx, void *krnHeader, uint32_t *krnSize)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(binary);
    CODECHAL_ENCODE_CHK_NULL_RETURN(krnHeader);
    CODECHAL_ENCODE_CHK_NULL_RETURN(krnSize);

    auto kernelHeaderTable = (PCODECHAL_ENCODE_AVC_KERNEL_HEADER_G9_KBL)binary;
    PCODECHAL_KERNEL_HEADER invalidEntry = &(kernelHeaderTable->AVC_StaticFrameDetection) + 1;
    PCODECHAL_KERNEL_HEADER nextKrnHeader = nullptr;
    PCODECHAL_KERNEL_HEADER currKrnHeader = nullptr;
    if (operation == ENC_SCALING4X)
    {
        currKrnHeader = &kernelHeaderTable->PLY_DScale_PLY;
    }
    else if (operation == ENC_SCALING2X)
    {
        currKrnHeader = &kernelHeaderTable->PLY_2xDScale_PLY;
    }
    else if (operation == ENC_ME)
    {
        currKrnHeader = &kernelHeaderTable->AVC_ME_P;
    }
    else if (operation == VDENC_ME)
    {
        currKrnHeader = &kernelHeaderTable->AVC_ME_VDENC;
    }
    else if (operation == ENC_SFD)
    {
        currKrnHeader = &kernelHeaderTable->AVC_StaticFrameDetection;
    }
    else
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Unsupported ENC mode requested");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    currKrnHeader += krnStateIdx;
    *((PCODECHAL_KERNEL_HEADER)krnHeader) = *currKrnHeader;

    nextKrnHeader = (currKrnHeader + 1);
    uint32_t nextKrnOffset = *krnSize;
    if (nextKrnHeader < invalidEntry)
    {
        nextKrnOffset = nextKrnHeader->KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT;
    }
    *krnSize = nextKrnOffset - (currKrnHeader->KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);

    return eStatus;
}

bool CodechalVdencAvcStateG9Kbl::CheckSupportedFormat(PMOS_SURFACE surface)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    bool isColorFormatSupported = true;
    if (IS_Y_MAJOR_TILE_FORMAT(surface->TileType))
    {
        switch (surface->Format)
        {
        case Format_NV12:
            break;
        default:
            isColorFormatSupported = false;
            break;
        }
    }
    else if (surface->TileType == MOS_TILE_LINEAR)
    {
        switch (surface->Format)
        {
        case Format_NV12:
        case Format_YUY2:
        case Format_YUYV:
        case Format_YVYU:
        case Format_UYVY:
        case Format_VYUY:
        case Format_AYUV:
            break;
        default:
            isColorFormatSupported = false;
            break;
        }
    }
    else
    {
        isColorFormatSupported = false;
    }

    return isColorFormatSupported;
}

MOS_STATUS CodechalVdencAvcStateG9Kbl::GetTrellisQuantization(PCODECHAL_ENCODE_AVC_TQ_INPUT_PARAMS params, PCODECHAL_ENCODE_AVC_TQ_PARAMS trellisQuantParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL(params);
    CODECHAL_ENCODE_CHK_NULL(trellisQuantParams);

    trellisQuantParams->dwTqEnabled = TrellisQuantizationEnable[params->ucTargetUsage];
    trellisQuantParams->dwTqRounding =
        trellisQuantParams->dwTqEnabled ? TrellisQuantizationRounding[params->ucTargetUsage] : 0;

finish:
    return eStatus;
}

MOS_STATUS CodechalVdencAvcStateG9Kbl::SetDmemHuCBrcInitReset()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    // Setup BRC DMEM
    MOS_LOCK_PARAMS lockFlagsWriteOnly;
    MOS_ZeroMemory(&lockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
    lockFlagsWriteOnly.WriteOnly = 1;
    auto pHucVDEncBrcInitDmem    = (PCODECHAL_VDENC_AVC_BRC_INIT_DMEM_G9_KBL)m_osInterface->pfnLockResource(
        m_osInterface, &m_resVdencBrcInitDmemBuffer[m_currRecycledBufIdx], &lockFlagsWriteOnly);

    CODECHAL_ENCODE_CHK_NULL_RETURN(pHucVDEncBrcInitDmem);
    MOS_ZeroMemory(pHucVDEncBrcInitDmem, sizeof(CODECHAL_VDENC_AVC_BRC_INIT_DMEM_G9_KBL));

    SetDmemHuCBrcInitResetImpl<CODECHAL_VDENC_AVC_BRC_INIT_DMEM_G9_KBL>(pHucVDEncBrcInitDmem);

    // fractional QP enable for extended rho domain
    pHucVDEncBrcInitDmem->INIT_FracQPEnable_U8 = (uint8_t)m_vdencInterface->IsRhoDomainStatsEnabled();
    pHucVDEncBrcInitDmem->INIT_SliceSizeCtrlWA = 1;

    pHucVDEncBrcInitDmem->INIT_SinglePassOnly = m_vdencSinglePassEnable ? true : false;

    if (((m_avcSeqParam->TargetUsage & 0x07) == TARGETUSAGE_BEST_SPEED) &&
        (m_avcSeqParam->FrameWidth >= SINGLE_PASS_MIN_FRAME_WIDTH) &&
        (m_avcSeqParam->FrameHeight >= SINGLE_PASS_MIN_FRAME_HEIGHT) &&
        (m_avcSeqParam->FramesPer100Sec >= SINGLE_PASS_MIN_FRAME_PER100S))
    {
        pHucVDEncBrcInitDmem->INIT_SinglePassOnly = true;
    }

    //Override the DistQPDelta setting.
    if (m_mbBrcEnabled)
    {
        if (m_avcSeqParam->FrameSizeTolerance == EFRAMESIZETOL_EXTREMELY_LOW)
        {
            MOS_SecureMemcpy(pHucVDEncBrcInitDmem->INIT_DistQPDelta_I8, 4 * sizeof(int8_t), (void*)BRC_INIT_DistQPDelta_I8_LowDelay, 4 * sizeof(int8_t));
        }
        else
        {
            MOS_SecureMemcpy(pHucVDEncBrcInitDmem->INIT_DistQPDelta_I8, 4 * sizeof(int8_t), (void*)BRC_INIT_DistQPDelta_I8, 4 * sizeof(int8_t));
        }
    }

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(PopulateBrcInitParam(
            pHucVDEncBrcInitDmem));
    )

    m_osInterface->pfnUnlockResource(m_osInterface, &m_resVdencBrcInitDmemBuffer[m_currRecycledBufIdx]);

    return eStatus;
}

MOS_STATUS CodechalVdencAvcStateG9Kbl::SetDmemHuCBrcUpdate()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    // Program update DMEM
    MOS_LOCK_PARAMS lockFlags;
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.WriteOnly = 1;
    auto pHucVDEncBrcDmem = (P_CODECHAL_VDENC_AVC_BRC_UPDATE_DMEM_G9_KBL)m_osInterface->pfnLockResource(
        m_osInterface, &m_resVdencBrcUpdateDmemBuffer[m_currRecycledBufIdx][m_currPass], &lockFlags);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pHucVDEncBrcDmem);
    SetDmemHuCBrcUpdateImpl<_CODECHAL_VDENC_AVC_BRC_UPDATE_DMEM_G9_KBL>(pHucVDEncBrcDmem);

    if (m_avcSeqParam->EnableSliceLevelRateCtrl)
    {
        pHucVDEncBrcDmem->UPD_SLCSZ_ConsertativeThreshold_U8 = (uint8_t)(m_avcSeqParam->RateControlMethod != RATECONTROL_VCM);
    }
    else
    {
        pHucVDEncBrcDmem->UPD_SLCSZ_ConsertativeThreshold_U8 = 0;
    }

    if (m_16xMeSupported && (m_pictureCodingType == P_TYPE))
    {
        pHucVDEncBrcDmem->HmeDistAvailable_U8 = 1;
    }
    pHucVDEncBrcDmem->UPD_WidthInMB_U16 = m_picWidthInMb;
    pHucVDEncBrcDmem->UPD_HeightInMB_U16 = m_picHeightInMb;

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(PopulateBrcUpdateParam(
            pHucVDEncBrcDmem));
    )

    m_osInterface->pfnUnlockResource(m_osInterface, &(m_resVdencBrcUpdateDmemBuffer[m_currRecycledBufIdx][m_currPass]));

    return eStatus;
}

MOS_STATUS CodechalVdencAvcStateG9Kbl::LoadMvCost(uint8_t qp)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    m_vdEncMvCost[0] = Map44LutValue((uint32_t)(MV_Cost_SkipBias_QPel[0][0]), 0x6f);
    m_vdEncMvCost[1] = Map44LutValue((uint32_t)(MV_Cost_SkipBias_QPel[0][1]), 0x6f);
    m_vdEncMvCost[2] = Map44LutValue((uint32_t)(MV_Cost_SkipBias_QPel[0][2]), 0x6f);
    m_vdEncMvCost[3] = Map44LutValue((uint32_t)(MV_Cost_SkipBias_QPel[0][3]), 0x6f);
    m_vdEncMvCost[4] = Map44LutValue((uint32_t)(MV_Cost_SkipBias_QPel[0][4]), 0x6f);
    m_vdEncMvCost[5] = Map44LutValue((uint32_t)(MV_Cost_SkipBias_QPel[0][5]), 0x6f);
    m_vdEncMvCost[6] = Map44LutValue((uint32_t)(MV_Cost_SkipBias_QPel[0][6]), 0x6f);
    m_vdEncMvCost[7] = Map44LutValue((uint32_t)(MV_Cost_SkipBias_QPel[0][7]), 0x6f);

    if (!m_vdencBrcEnabled)
    {
        if (qp == 47 || qp == 48 || qp == 49)
        {
            m_vdEncMvCost[3] = Map44LutValue((uint32_t)(MV_Cost_SkipBias_QPel[1][3]), 0x6f);
            m_vdEncMvCost[4] = Map44LutValue((uint32_t)(MV_Cost_SkipBias_QPel[1][4]), 0x6f);
            m_vdEncMvCost[5] = Map44LutValue((uint32_t)(MV_Cost_SkipBias_QPel[1][5]), 0x6f);
            m_vdEncMvCost[6] = Map44LutValue((uint32_t)(MV_Cost_SkipBias_QPel[1][6]), 0x6f);
            m_vdEncMvCost[7] = Map44LutValue((uint32_t)(MV_Cost_SkipBias_QPel[1][7]), 0x6f);
        }
        if (qp == 50 || qp == 51)
        {
            m_vdEncMvCost[3] = Map44LutValue((uint32_t)(MV_Cost_SkipBias_QPel[2][3]), 0x6f);
            m_vdEncMvCost[4] = Map44LutValue((uint32_t)(MV_Cost_SkipBias_QPel[2][4]), 0x6f);
            m_vdEncMvCost[5] = Map44LutValue((uint32_t)(MV_Cost_SkipBias_QPel[2][5]), 0x6f);
            m_vdEncMvCost[6] = Map44LutValue((uint32_t)(MV_Cost_SkipBias_QPel[2][6]), 0x6f);
            m_vdEncMvCost[7] = Map44LutValue((uint32_t)(MV_Cost_SkipBias_QPel[2][7]), 0x6f);
        }
    }

    return eStatus;
}

MOS_STATUS CodechalVdencAvcStateG9Kbl::LoadHmeMvCost(uint8_t qp)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    PCODEC_AVC_ENCODE_SEQUENCE_PARAMS avcSeqParams = m_avcSeqParam;
    const uint32_t(*puiVdencHmeCostTable)[CODEC_AVC_NUM_QP];
    if (avcSeqParams->ScenarioInfo == ESCENARIO_DISPLAYREMOTING)
    {
        puiVdencHmeCostTable = HmeCost_DisplayRemote;
    }
    else
    {
        puiVdencHmeCostTable = HmeCost;
    }

    m_vdEncHmeMvCost[0] = Map44LutValue(*(puiVdencHmeCostTable[0] + qp), 0x6f);
    m_vdEncHmeMvCost[1] = Map44LutValue(*(puiVdencHmeCostTable[1] + qp), 0x6f);
    m_vdEncHmeMvCost[2] = Map44LutValue(*(puiVdencHmeCostTable[2] + qp), 0x6f);
    m_vdEncHmeMvCost[3] = Map44LutValue(*(puiVdencHmeCostTable[3] + qp), 0x6f);
    m_vdEncHmeMvCost[4] = Map44LutValue(*(puiVdencHmeCostTable[4] + qp), 0x6f);
    m_vdEncHmeMvCost[5] = Map44LutValue(*(puiVdencHmeCostTable[5] + qp), 0x6f);
    m_vdEncHmeMvCost[6] = Map44LutValue(*(puiVdencHmeCostTable[6] + qp), 0x6f);
    m_vdEncHmeMvCost[7] = Map44LutValue(*(puiVdencHmeCostTable[7] + qp), 0x6f);

    return eStatus;
}

MOS_STATUS CodechalVdencAvcStateG9Kbl::LoadHmeMvCostTable(PCODEC_AVC_ENCODE_SEQUENCE_PARAMS seqParams, uint8_t HMEMVCostTable[8][42])
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    const uint32_t(*puiVdencHmeCostTable)[CODEC_AVC_NUM_QP];
    if ((m_avcSeqParam->ScenarioInfo == ESCENARIO_DISPLAYREMOTING) || (m_avcSeqParam->RateControlMethod == RATECONTROL_QVBR))
    {
        puiVdencHmeCostTable = HmeCost_DisplayRemote;
    }
    else
    {
        puiVdencHmeCostTable = HmeCost;
    }

    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 42; j++)
        {
            HMEMVCostTable[i][j] = Map44LutValue(*(puiVdencHmeCostTable[i] + j + 10), 0x6f);
        }
    }

    return eStatus;
}

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS CodechalVdencAvcStateG9Kbl::PopulateBrcInitParam(
    void *cmd)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    CODECHAL_DEBUG_CHK_NULL(m_debugInterface);

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDumpEncodePar))
    {
        return MOS_STATUS_SUCCESS;
    }

    PCODECHAL_VDENC_AVC_BRC_INIT_DMEM_G9_KBL dmem = (PCODECHAL_VDENC_AVC_BRC_INIT_DMEM_G9_KBL)cmd;

    if (m_pictureCodingType == I_TYPE)
    {
        m_avcPar->MBBRCEnable                    = m_mbBrcEnabled;
        m_avcPar->MBRC                           = m_mbBrcEnabled;
        m_avcPar->BitRate                        = dmem->INIT_TargetBitrate_U32;
        m_avcPar->InitVbvFullnessInBit           = dmem->INIT_InitBufFull_U32;
        m_avcPar->MaxBitRate                     = dmem->INIT_MaxRate_U32;
        m_avcPar->VbvSzInBit                     = dmem->INIT_BufSize_U32;
        m_avcPar->UserMaxFrame                   = dmem->INIT_ProfileLevelMaxFrame_U32;
        m_avcPar->SlidingWindowEnable            = dmem->INIT_SlidingWidowRCEnable_U8;
        m_avcPar->SlidingWindowSize              = dmem->INIT_SlidingWindowSize_U8;
        m_avcPar->SlidingWindowMaxRateRatio      = dmem->INIT_SlidingWindowMaxRateRatio_U8;
        m_avcPar->LowDelayGoldenFrameBoost       = dmem->INIT_LowDelayGoldenFrameBoost_U8;
        m_avcPar->TopQPDeltaThrforAdaptive2Pass  = dmem->INIT_TopQPDeltaThrForAdapt2Pass_U8;
        m_avcPar->BotQPDeltaThrforAdaptive2Pass  = dmem->INIT_BotQPDeltaThrForAdapt2Pass_U8;
        m_avcPar->TopFrmSzPctThrforAdaptive2Pass = dmem->INIT_TopFrmSzThrForAdapt2Pass_U8;
        m_avcPar->BotFrmSzPctThrforAdaptive2Pass = dmem->INIT_BotFrmSzThrForAdapt2Pass_U8;
        m_avcPar->MBHeaderCompensation           = dmem->INIT_MBHeaderCompensation_U8;
        m_avcPar->QPSelectMethodforFirstPass     = dmem->INIT_QPSelectForFirstPass_U8;
        m_avcPar->MBQpCtrl                       = (dmem->INIT_MbQpCtrl_U8 > 0) ? true : false;
        m_avcPar->QPMax                          = dmem->INIT_MaxQP_U16;
        m_avcPar->QPMin                          = dmem->INIT_MinQP_U16;
        m_avcPar->HrdConformanceCheckDisable     = (dmem->INIT_HRDConformanceCheckDisable_U8 > 0) ? true : false;
        m_avcPar->ICQReEncode                    = (dmem->INIT_ICQReEncode_U8 > 0) ? true : false;
        m_avcPar->AdaptiveCostAdjustEnable       = (dmem->INIT_AdaptiveCostEnable_U8 > 0) ? true : false;
        m_avcPar->AdaptiveHMEExtension           = (dmem->INIT_AdaptiveHMEExtensionEnable_U8 > 0) ? true : false;
        m_avcPar->StreamInStaticRegion           = dmem->INIT_StaticRegionStreamIn_U8;
        ;
        m_avcPar->ScenarioInfo = dmem->INIT_ScenarioInfo_U8;
        ;
        m_avcPar->SliceSizeWA = (dmem->INIT_SliceSizeCtrlWA > 0) ? true : false;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalVdencAvcStateG9Kbl::PopulateBrcUpdateParam(
    void *cmd)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    CODECHAL_DEBUG_CHK_NULL(m_debugInterface);

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDumpEncodePar))
    {
        return MOS_STATUS_SUCCESS;
    }

    P_CODECHAL_VDENC_AVC_BRC_UPDATE_DMEM_G9_KBL dmem = (P_CODECHAL_VDENC_AVC_BRC_UPDATE_DMEM_G9_KBL)cmd;

    if (m_pictureCodingType == I_TYPE)
    {
        m_avcPar->EnableMultipass            = (dmem->UPD_MaxNumPass_U8 > 0) ? true : false;
        m_avcPar->MaxNumPakPasses            = dmem->UPD_MaxNumPass_U8;
        m_avcPar->SceneChgDetectEn           = (dmem->UPD_SceneChgDetectEn_U8 > 0) ? true : false;
        m_avcPar->SceneChgPrevIntraPctThresh = dmem->UPD_SceneChgPrevIntraPctThreshold_U8;
        m_avcPar->SceneChgCurIntraPctThresh  = dmem->UPD_SceneChgCurIntraPctThreshold_U8;
        m_avcPar->SceneChgWidth0             = dmem->UPD_SceneChgWidth_U8[0];
        m_avcPar->SceneChgWidth1             = dmem->UPD_SceneChgWidth_U8[1];
        m_avcPar->SliceSizeThr               = dmem->UPD_SLCSZ_TARGETSLCSZ_U16;
        m_avcPar->SliceMaxSize               = dmem->UPD_TargetSliceSize_U16;
    }
    else if (m_pictureCodingType == P_TYPE)
    {
        m_avcPar->Transform8x8PDisable = (dmem->UPD_DisablePFrame8x8Transform_U8 > 0) ? true : false;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalVdencAvcStateG9Kbl::PopulateEncParam(
    uint8_t meMethod,
    void    *cmd)
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

    if (m_vdencBrcEnabled)
    {
        // BRC case: VDENC IMG STATE is updated by HuC FW
        data = (uint8_t *)m_osInterface->pfnLockResource(m_osInterface, &m_resVdencBrcImageStatesReadBuffer[m_currRecycledBufIdx], &lockFlags);
        data = data + mhw_vdbox_mfx_g9_kbl::MFX_AVC_IMG_STATE_CMD::byteSize;
    }
    else
    {
        // CQP case: VDENC IMG STATE is updated by driver or SFD kernel
        if (!m_staticFrameDetectionInUse)
        {
            data = m_batchBufferForVdencImgStat[m_currRecycledBufIdx].pData;
            data = data + mhw_vdbox_mfx_g9_kbl::MFX_AVC_IMG_STATE_CMD::byteSize;
        }
        else
        {
            data = (uint8_t *)m_osInterface->pfnLockResource(m_osInterface, &m_resVdencSfdImageStateReadBuffer, &lockFlags);
        }
    }

    CODECHAL_DEBUG_CHK_NULL(data);

    mhw_vdbox_vdenc_g9_kbl::VDENC_IMG_STATE_CMD vdencCmd;
    vdencCmd = *(mhw_vdbox_vdenc_g9_kbl::VDENC_IMG_STATE_CMD *)(data);

    if (m_pictureCodingType == I_TYPE)
    {
        m_avcPar->BlockBasedSkip = vdencCmd.DW4.BlockBasedSkipEnabled;
        m_avcPar->VDEncPerfMode  = vdencCmd.DW1.VdencPerfmode;
    }
    else if (m_pictureCodingType == P_TYPE)
    {
        m_avcPar->SubPelMode            = vdencCmd.DW4.SubPelMode;
        m_avcPar->FTQBasedSkip          = vdencCmd.DW4.ForwardTransformSkipCheckEnable;
        m_avcPar->BiMixDisable          = vdencCmd.DW1.BidirectionalMixDisable;
        m_avcPar->SurvivedSkipCost      = (vdencCmd.DW8.NonSkipZeroMvCostAdded << 1) + vdencCmd.DW8.NonSkipMbModeCostAdded;
        m_avcPar->UniMixDisable         = vdencCmd.DW2.UnidirectionalMixDisable;
        m_avcPar->VdencExtPakObjDisable = !vdencCmd.DW1.VdencExtendedPakObjCmdEnable;
        m_avcPar->PPMVDisable           = vdencCmd.DW34.PpmvDisable;
    }

    if (data)
    {
        if (m_vdencBrcEnabled)
        {
            m_osInterface->pfnUnlockResource(
                m_osInterface,
                &m_resVdencBrcImageStatesReadBuffer[m_currRecycledBufIdx]);
        }
        else
        {
            if (m_staticFrameDetectionInUse)
            {
                m_osInterface->pfnUnlockResource(
                    m_osInterface,
                    &m_resVdencSfdImageStateReadBuffer);
            }
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalVdencAvcStateG9Kbl::PopulatePakParam(
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
        data = (uint8_t*)(cmdBuffer->pCmdPtr - (mhw_vdbox_mfx_g9_kbl::MFX_AVC_IMG_STATE_CMD::byteSize / sizeof(uint32_t)));
    }
    else if (secondLevelBatchBuffer != nullptr)
    {
        data = secondLevelBatchBuffer->pData;
    }
    else
    {
        data = (uint8_t *)m_osInterface->pfnLockResource(m_osInterface, &m_resVdencBrcImageStatesReadBuffer[m_currRecycledBufIdx], &lockFlags);
    }

    CODECHAL_DEBUG_CHK_NULL(data);

    mhw_vdbox_mfx_g9_kbl::MFX_AVC_IMG_STATE_CMD mfxCmd;
    mfxCmd = *(mhw_vdbox_mfx_g9_kbl::MFX_AVC_IMG_STATE_CMD *)(data);

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
            &m_resVdencBrcImageStatesReadBuffer[m_currRecycledBufIdx]);
    }

    return MOS_STATUS_SUCCESS;
}
#endif
