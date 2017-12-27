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
//! \file     codechal_vdenc_avc_g9_bxt.cpp
//! \brief    This file implements the C++ class/interface for BXT AVC 
//!           VDEnc encoding to be used CODECHAL components.
//!

#include "codechal_vdenc_avc_g9_bxt.h"
#if USE_CODECHAL_DEBUG_TOOL
#include "mhw_vdbox_mfx_hwcmd_g9_bxt.h"
#include "mhw_vdbox_vdenc_hwcmd_g9_bxt.h"
#endif

typedef struct _CODECHAL_ENCODE_AVC_KERNEL_HEADER_G9_BXT {
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

    // BRC Init frame
    CODECHAL_KERNEL_HEADER InitFrameBRC;

    // FrameBRC Update
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
} CODECHAL_ENCODE_AVC_KERNEL_HEADER_G9_BXT, *PCODECHAL_ENCODE_AVC_KERNEL_HEADER_G9_BXT;

typedef struct _CODECHAL_VDENC_AVC_BRC_INIT_DMEM_G9_BXT
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
    uint8_t     RSVD2[61];                            // must be zero
} CODECHAL_VDENC_AVC_BRC_INIT_DMEM_G9_BXT, *PCODECHAL_VDENC_AVC_BRC_INIT_DMEM_G9_BXT;

typedef struct _CODECHAL_VDENC_AVC_BRC_UPDATE_DMEM_G9_BXT
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
    uint16_t    UPD_SkipFrameSize_U16;                // Recording the skip frame size for one frame. =NumMBs * 1, assuming one bit per mb for skip frame.
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
    int8_t      UPD_ROIQpDelta_I8[4];                 // Application specified ROI QP Adjustment for Zone0, Zone1, Zone2 and Zone3.
    uint8_t     UPD_CQP_QpValue_U8;                   // Application specified target QP in BRC_ICQ mode
    uint8_t     UPD_CQP_FracQp_U8;                    // Application specified fine position in BRC_ICQ mode
    uint8_t     UPD_HMEDetectionEnable_U8;            // 0: default, 1: HuC BRC kernel requires information from HME detection kernel output
    uint8_t     UPD_HMECostEnable_U8;                 // 0: default, 1: driver provides HME cost table
    uint8_t     UPD_DisablePFrame8x8Transform_U8;     // 0: enable, 1: disable
    uint8_t     UPD_BxtCabacWAEnable_U8;              // 0: disable, 1: enable
    uint8_t     UPD_ROISource_U8;                     // =0: disable, 1: ROIMap from HME Static Region or from App dirty rectangle, 2: ROIMap from App
    uint8_t     UPD_SLCSZ_ConsertativeThreshold_U8;   // =0, 0: do not set conservative threshold (suggested for video conference) 1: set conservative threshold for non-video conference
    uint16_t    UPD_TargetSliceSize_U16;              // default: 1498, max target slice size from app DDI
    uint16_t    UPD_MaxNumSliceAllowed_U16;           // computed by driver based on level idc
    uint16_t    UPD_SLBB_Size_U16;                    // second level batch buffer (SLBB) size in bytes, the input buffer will contain two SLBBs A and B, A followed by B, A and B have the same structure. 
    uint16_t    UPD_SLBB_B_Offset_U16;                // offset in bytes from the beginning of the input buffer, it points to the start of SLBB B, set by driver for skip frame support
    uint16_t    UPD_AvcImgStateOffset_U16;            // offset in bytes from the beginning of SLBB A 

    /* HME distortion based QP adjustment */
    uint16_t    AveHmeDist_U16;
    uint8_t     HmeDistAvailable_U8; // 0: disabled, 1: enabled

    uint16_t    AdditionalFrameSize_U16; // for slice size control improvement

    uint8_t     RSVD2[61];
} CODECHAL_VDENC_AVC_BRC_UPDATE_DMEM_G9_BXT, *PCODECHAL_VDENC_AVC_BRC_UPDATE_DMEM_G9_BXT;

const uint32_t CodechalVdencAvcStateG9Bxt::MV_Cost_SkipBias_QPel[8] =
{
    //PREDSLICE
    0, 6, 6, 9, 10, 13, 14, 16
};

const uint32_t CodechalVdencAvcStateG9Bxt::HmeCost[8][CODEC_AVC_NUM_QP] =
{
    //mv=0
    {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     //QP=[0 ~12]
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     //QP=[13 ~25]
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     //QP=[26 ~38]
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     //QP=[39 ~51]
    },
    //mv<=16
    {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     //QP=[0 ~12]
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     //QP=[13 ~25]
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     //QP=[26 ~38]
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     //QP=[39 ~51]
    },
    //mv<=32
    {
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,     //QP=[0 ~12]
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,     //QP=[13 ~25]
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,     //QP=[26 ~38]
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,     //QP=[39 ~51]
    },
    //mv<=64
    {
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,     //QP=[0 ~12]
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,     //QP=[13 ~25]
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,     //QP=[26 ~38]
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,     //QP=[39 ~51]
    },
    //mv<=128
    {
        10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,     //QP=[0 ~12]
        10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,     //QP=[13 ~25]
        10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,     //QP=[26 ~38]
        10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,     //QP=[39 ~51]
    },
    //mv<=256
    {
        10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,     //QP=[0 ~12]
        10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,     //QP=[13 ~25]
        10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,     //QP=[26 ~38]
        10, 10, 10, 10, 20, 30, 40, 50, 50, 50, 50, 50, 50,     //QP=[39 ~51]
    },
    //mv<=512
    {
        20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,         //QP=[0 ~12]
        20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,         //QP=[13 ~25]
        20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,         //QP=[26 ~38]
        20, 20, 20, 40, 60, 80, 100, 100, 100, 100, 100, 100, 100,  //QP=[39 ~51]
    },
    //mv<=1024
    {
        20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,             //QP=[0 ~12]
        20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,             //QP=[13 ~25]
        20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,             //QP=[26 ~38]
        20, 20, 30, 50, 100, 200, 200, 200, 200, 200, 200, 200, 200,    //QP=[39 ~51]
    },
};

CodechalVdencAvcStateG9Bxt::CodechalVdencAvcStateG9Bxt(
        CodechalHwInterface *   hwInterface,
        CodechalDebugInterface *debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo) : CodechalVdencAvcStateG9(hwInterface, debugInterface, standardInfo)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    this->pfnGetKernelHeaderAndSize = this->EncodeGetKernelHeaderAndSize;
    dwVdencBrcInitDmemBufferSize = sizeof(CODECHAL_VDENC_AVC_BRC_INIT_DMEM_G9_BXT);
    dwVdencBrcUpdateDmemBufferSize = sizeof(CODECHAL_VDENC_AVC_BRC_UPDATE_DMEM_G9_BXT);
}

CodechalVdencAvcStateG9Bxt::~CodechalVdencAvcStateG9Bxt()
{
}

MOS_STATUS CodechalVdencAvcStateG9Bxt::EncodeGetKernelHeaderAndSize(void *pvBinary, EncOperation Operation, uint32_t dwKrnStateIdx, void *pvKrnHeader, uint32_t *pdwKrnSize)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(pvBinary);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pvKrnHeader);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pdwKrnSize);

    auto pKernelHeaderTable = (PCODECHAL_ENCODE_AVC_KERNEL_HEADER_G9_BXT)pvBinary;
    auto pInvalidEntry = &(pKernelHeaderTable->AVC_StaticFrameDetection) + 1;
    uint32_t dwNextKrnOffset = *pdwKrnSize;

    PCODECHAL_KERNEL_HEADER pCurrKrnHeader;
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

    PCODECHAL_KERNEL_HEADER pNextKrnHeader = (pCurrKrnHeader + 1);
    if (pNextKrnHeader < pInvalidEntry)
    {
        dwNextKrnOffset = pNextKrnHeader->KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT;
    }
    *pdwKrnSize = dwNextKrnOffset - (pCurrKrnHeader->KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);

    return eStatus;
}

MOS_STATUS CodechalVdencAvcStateG9Bxt::GetTrellisQuantization(PCODECHAL_ENCODE_AVC_TQ_INPUT_PARAMS pParams, PCODECHAL_ENCODE_AVC_TQ_PARAMS pTrellisQuantParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pTrellisQuantParams);

    pTrellisQuantParams->dwTqEnabled = TrellisQuantizationEnable[pParams->ucTargetUsage];
    pTrellisQuantParams->dwTqRounding =
        pTrellisQuantParams->dwTqEnabled ? TrellisQuantizationRounding[pParams->ucTargetUsage] : 0;

    return eStatus;
}

MOS_STATUS CodechalVdencAvcStateG9Bxt::SetDmemHuCBrcInitReset()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    // Setup BRC DMEM
    MOS_LOCK_PARAMS LockFlagsWriteOnly;
    MOS_ZeroMemory(&LockFlagsWriteOnly, sizeof(LockFlagsWriteOnly));
    LockFlagsWriteOnly.WriteOnly = 1;
    auto pHucVDEncBrcInitDmem = (PCODECHAL_VDENC_AVC_BRC_INIT_DMEM_G9_BXT)m_osInterface->pfnLockResource(
        m_osInterface, &resVdencBrcInitDmemBuffer[m_currRecycledBufIdx], &LockFlagsWriteOnly);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pHucVDEncBrcInitDmem);
    SetDmemHuCBrcInitResetImpl<CODECHAL_VDENC_AVC_BRC_INIT_DMEM_G9_BXT>(pHucVDEncBrcInitDmem);

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(PopulateBrcInitParam(
            pHucVDEncBrcInitDmem));
    )

    m_osInterface->pfnUnlockResource(m_osInterface, &resVdencBrcInitDmemBuffer[m_currRecycledBufIdx]);

    return eStatus;
}

MOS_STATUS CodechalVdencAvcStateG9Bxt::SetDmemHuCBrcUpdate()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    // Program update DMEM
    MOS_LOCK_PARAMS LockFlags;
    MOS_ZeroMemory(&LockFlags, sizeof(MOS_LOCK_PARAMS));
    LockFlags.WriteOnly = 1;
    auto pHucVDEncBrcDmem = (PCODECHAL_VDENC_AVC_BRC_UPDATE_DMEM_G9_BXT)m_osInterface->pfnLockResource(
        m_osInterface, &resVdencBrcUpdateDmemBuffer[m_currRecycledBufIdx][m_currPass], &LockFlags);

    CODECHAL_ENCODE_CHK_NULL_RETURN(pHucVDEncBrcDmem);
    SetDmemHuCBrcUpdateImpl<CODECHAL_VDENC_AVC_BRC_UPDATE_DMEM_G9_BXT>(pHucVDEncBrcDmem);

    if (m_avcSeqParam->EnableSliceLevelRateCtrl)
    {
        pHucVDEncBrcDmem->UPD_SLCSZ_ConsertativeThreshold_U8 = (uint8_t)(m_avcSeqParam->RateControlMethod != RATECONTROL_VCM);
    }
    else
    {
        pHucVDEncBrcDmem->UPD_SLCSZ_ConsertativeThreshold_U8 = 0;
    }

    if (m_vdencEnabled && m_16xMeSupported && (m_pictureCodingType == P_TYPE))
    {
        pHucVDEncBrcDmem->HmeDistAvailable_U8 = 1;
    }
    else
    {
        pHucVDEncBrcDmem->HmeDistAvailable_U8 = 0;
    }

    pHucVDEncBrcDmem->AdditionalFrameSize_U16 = 0; // need to update from DDI

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(PopulateBrcUpdateParam(
            pHucVDEncBrcDmem));
    )

    m_osInterface->pfnUnlockResource(m_osInterface, &(resVdencBrcUpdateDmemBuffer[m_currRecycledBufIdx][m_currPass]));

    return eStatus;
}

MOS_STATUS CodechalVdencAvcStateG9Bxt::LoadMvCost(uint8_t QP)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    VDEncMvCost[0] = Map44LutValue((uint32_t)(MV_Cost_SkipBias_QPel[0]), 0x6f);
    VDEncMvCost[1] = Map44LutValue((uint32_t)(MV_Cost_SkipBias_QPel[1]), 0x6f);
    VDEncMvCost[2] = Map44LutValue((uint32_t)(MV_Cost_SkipBias_QPel[2]), 0x6f);
    VDEncMvCost[3] = Map44LutValue((uint32_t)(MV_Cost_SkipBias_QPel[3]), 0x6f);
    VDEncMvCost[4] = Map44LutValue((uint32_t)(MV_Cost_SkipBias_QPel[4]), 0x6f);
    VDEncMvCost[5] = Map44LutValue((uint32_t)(MV_Cost_SkipBias_QPel[5]), 0x6f);
    VDEncMvCost[6] = Map44LutValue((uint32_t)(MV_Cost_SkipBias_QPel[6]), 0x6f);
    VDEncMvCost[7] = Map44LutValue((uint32_t)(MV_Cost_SkipBias_QPel[7]), 0x6f);

    return eStatus;
}

MOS_STATUS CodechalVdencAvcStateG9Bxt::LoadHmeMvCost(uint8_t QP)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    VDEncHmeMvCost[0] = Map44LutValue(*(HmeCost[0] + QP), 0x6f);
    VDEncHmeMvCost[1] = Map44LutValue(*(HmeCost[1] + QP), 0x6f);
    VDEncHmeMvCost[2] = Map44LutValue(*(HmeCost[2] + QP), 0x6f);
    VDEncHmeMvCost[3] = Map44LutValue(*(HmeCost[3] + QP), 0x6f);
    VDEncHmeMvCost[4] = Map44LutValue(*(HmeCost[4] + QP), 0x6f);
    VDEncHmeMvCost[5] = Map44LutValue(*(HmeCost[5] + QP), 0x6f);
    VDEncHmeMvCost[6] = Map44LutValue(*(HmeCost[6] + QP), 0x6f);
    VDEncHmeMvCost[7] = Map44LutValue(*(HmeCost[7] + QP), 0x6f);

    return eStatus;
}

MOS_STATUS CodechalVdencAvcStateG9Bxt::LoadHmeMvCostTable(PCODEC_AVC_ENCODE_SEQUENCE_PARAMS pSeqParams, uint8_t HMEMVCostTable[8][42])
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;
    
    const uint32_t(*puiVdencHmeCostTable)[CODEC_AVC_NUM_QP];
    puiVdencHmeCostTable = HmeCost;

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
MOS_STATUS CodechalVdencAvcStateG9Bxt::PopulateBrcInitParam(
    void *cmd)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    CODECHAL_DEBUG_CHK_NULL(m_debugInterface);

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDumpEncodePar))
    {
        return MOS_STATUS_SUCCESS;
    }

    PCODECHAL_VDENC_AVC_BRC_INIT_DMEM_G9_BXT dmem = (PCODECHAL_VDENC_AVC_BRC_INIT_DMEM_G9_BXT)cmd;

    if (m_pictureCodingType == I_TYPE)
    {
        avcPar->MBBRCEnable                    = bMbBrcEnabled;
        avcPar->MBRC                           = bMbBrcEnabled;
        avcPar->BitRate                        = dmem->INIT_TargetBitrate_U32;
        avcPar->InitVbvFullnessInBit           = dmem->INIT_InitBufFull_U32;
        avcPar->MaxBitRate                     = dmem->INIT_MaxRate_U32;
        avcPar->VbvSzInBit                     = dmem->INIT_BufSize_U32;
        avcPar->UserMaxFrame                   = dmem->INIT_ProfileLevelMaxFrame_U32;
        avcPar->SlidingWindowEnable            = dmem->INIT_SlidingWidowRCEnable_U8;
        avcPar->SlidingWindowSize              = dmem->INIT_SlidingWindowSize_U8;
        avcPar->SlidingWindowMaxRateRatio      = dmem->INIT_SlidingWindowMaxRateRatio_U8;
        avcPar->LowDelayGoldenFrameBoost       = dmem->INIT_LowDelayGoldenFrameBoost_U8;
        avcPar->TopQPDeltaThrforAdaptive2Pass  = dmem->INIT_TopQPDeltaThrForAdapt2Pass_U8;
        avcPar->BotQPDeltaThrforAdaptive2Pass  = dmem->INIT_BotQPDeltaThrForAdapt2Pass_U8;
        avcPar->TopFrmSzPctThrforAdaptive2Pass = dmem->INIT_TopFrmSzThrForAdapt2Pass_U8;
        avcPar->BotFrmSzPctThrforAdaptive2Pass = dmem->INIT_BotFrmSzThrForAdapt2Pass_U8;
        avcPar->MBHeaderCompensation           = dmem->INIT_MBHeaderCompensation_U8;
        avcPar->QPSelectMethodforFirstPass     = dmem->INIT_QPSelectForFirstPass_U8;
        avcPar->MBQpCtrl                       = (dmem->INIT_MbQpCtrl_U8 > 0) ? true : false;
        avcPar->QPMax                          = dmem->INIT_MaxQP_U16;
        avcPar->QPMin                          = dmem->INIT_MinQP_U16;
        avcPar->HrdConformanceCheckDisable     = (dmem->INIT_HRDConformanceCheckDisable_U8 > 0) ? true : false;
        avcPar->StreamInStaticRegion           = dmem->INIT_StaticRegionStreamIn_U8;;
        avcPar->ScenarioInfo                   = dmem->INIT_ScenarioInfo_U8;;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalVdencAvcStateG9Bxt::PopulateBrcUpdateParam(
    void *cmd)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    CODECHAL_DEBUG_CHK_NULL(m_debugInterface);

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDumpEncodePar))
    {
        return MOS_STATUS_SUCCESS;
    }

    PCODECHAL_VDENC_AVC_BRC_UPDATE_DMEM_G9_BXT dmem = (PCODECHAL_VDENC_AVC_BRC_UPDATE_DMEM_G9_BXT)cmd;

    if (m_pictureCodingType == I_TYPE)
    {
        avcPar->EnableMultipass            = (dmem->UPD_MaxNumPass_U8 > 0) ? true : false;
        avcPar->MaxNumPakPasses            = dmem->UPD_MaxNumPass_U8;
        avcPar->SceneChgDetectEn           = (dmem->UPD_SceneChgDetectEn_U8 > 0) ? true : false;
        avcPar->SceneChgPrevIntraPctThresh = dmem->UPD_SceneChgPrevIntraPctThreshold_U8;
        avcPar->SceneChgCurIntraPctThresh  = dmem->UPD_SceneChgCurIntraPctThreshold_U8;
        avcPar->SceneChgWidth0             = dmem->UPD_SceneChgWidth_U8[0];
        avcPar->SceneChgWidth1             = dmem->UPD_SceneChgWidth_U8[1];
        avcPar->SliceSizeThr               = dmem->UPD_SLCSZ_TARGETSLCSZ_U16;
        avcPar->SliceMaxSize               = dmem->UPD_TargetSliceSize_U16;
    }
    else if (m_pictureCodingType == P_TYPE)
    {
        avcPar->Transform8x8PDisable       = (dmem->UPD_DisablePFrame8x8Transform_U8 > 0) ? true : false;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalVdencAvcStateG9Bxt::PopulateEncParam(
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
        data = (uint8_t*)m_osInterface->pfnLockResource(m_osInterface, &resVdencBrcImageStatesReadBuffer[m_currRecycledBufIdx], &lockFlags);
        data = data + mhw_vdbox_mfx_g9_bxt::MFX_AVC_IMG_STATE_CMD::byteSize;
    }
    else
    {
        // CQP case: VDENC IMG STATE is updated by driver or SFD kernel
        if (!m_staticFrameDetectionInUse)
        {
            data = m_batchBufferForVdencImgStat[m_currRecycledBufIdx].pData;
            data = data + mhw_vdbox_mfx_g9_bxt::MFX_AVC_IMG_STATE_CMD::byteSize;
        }
        else
        {
            data = (uint8_t*)m_osInterface->pfnLockResource(m_osInterface, &resVdencSFDImageStateReadBuffer, &lockFlags);
        }
    }

    CODECHAL_DEBUG_CHK_NULL(data);

    mhw_vdbox_vdenc_g9_bxt::VDENC_IMG_STATE_CMD vdencCmd;
    vdencCmd = *(mhw_vdbox_vdenc_g9_bxt::VDENC_IMG_STATE_CMD *)(data);

    if (m_pictureCodingType == I_TYPE)
    {
        avcPar->BlockBasedSkip        = vdencCmd.DW4.BlockBasedSkipEnabled;
    }
    else if (m_pictureCodingType == P_TYPE)
    {
        avcPar->SubPelMode            = vdencCmd.DW4.SubPelMode;
        avcPar->FTQBasedSkip          = vdencCmd.DW4.ForwardTransformSkipCheckEnable;
        avcPar->BiMixDisable          = vdencCmd.DW1.BidirectionalMixDisable;
        avcPar->SurvivedSkipCost      = (vdencCmd.DW8.NonSkipZeroMvCostAdded << 1) + vdencCmd.DW8.NonSkipMbModeCostAdded;
        avcPar->UniMixDisable         = vdencCmd.DW2.UnidirectionalMixDisable;
        avcPar->VdencExtPakObjDisable = !vdencCmd.DW1.VdencExtendedPakObjCmdEnable;
        avcPar->PPMVDisable           = vdencCmd.DW34.PpmvDisable;
    }

    if (data)
    {
        if (m_vdencBrcEnabled)
        {
            m_osInterface->pfnUnlockResource(
                m_osInterface,
                &resVdencBrcImageStatesReadBuffer[m_currRecycledBufIdx]);
        }
        else
        {
            if (m_staticFrameDetectionInUse)
            {
                m_osInterface->pfnUnlockResource(
                    m_osInterface,
                    &resVdencSFDImageStateReadBuffer);
            }
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalVdencAvcStateG9Bxt::PopulatePakParam(
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
        data = (uint8_t*)(cmdBuffer->pCmdPtr - (mhw_vdbox_mfx_g9_bxt::MFX_AVC_IMG_STATE_CMD::byteSize / sizeof(uint32_t)));
    }
    else if (secondLevelBatchBuffer != nullptr)
    {
        data = secondLevelBatchBuffer->pData;
    }
    else
    {
        data = (uint8_t*)m_osInterface->pfnLockResource(m_osInterface, &resVdencBrcImageStatesReadBuffer[m_currRecycledBufIdx], &lockFlags);
    }

    CODECHAL_DEBUG_CHK_NULL(data);

    mhw_vdbox_mfx_g9_bxt::MFX_AVC_IMG_STATE_CMD mfxCmd;
    mfxCmd = *(mhw_vdbox_mfx_g9_bxt::MFX_AVC_IMG_STATE_CMD *)(data);

    if (m_pictureCodingType == I_TYPE)
    {
        avcPar->TrellisQuantizationEnable         = mfxCmd.DW5.TrellisQuantizationEnabledTqenb;
        avcPar->EnableAdaptiveTrellisQuantization = mfxCmd.DW5.TrellisQuantizationEnabledTqenb;
        avcPar->TrellisQuantizationRounding       = mfxCmd.DW5.TrellisQuantizationRoundingTqr;
        avcPar->TrellisQuantizationChromaDisable  = mfxCmd.DW5.TrellisQuantizationChromaDisableTqchromadisable;
        avcPar->ExtendedRhoDomainEn               = mfxCmd.DW16_17.ExtendedRhodomainStatisticsEnable;
    }

    if (data && (cmdBuffer == nullptr) && (secondLevelBatchBuffer == nullptr))
    {
        m_osInterface->pfnUnlockResource(
            m_osInterface,
            &resVdencBrcImageStatesReadBuffer[m_currRecycledBufIdx]);
    }

    return MOS_STATUS_SUCCESS;
}
#endif
