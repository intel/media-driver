/*
* Copyright (c) 2017-2020, Intel Corporation
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
//! \file     codechal_vdenc_avc_g11.cpp
//! \brief    This file implements the C++ class/interface for Gen10 platform's AVC
//!           VDEnc encoding to be used CODECHAL components.
//!

#include "codechal_vdenc_avc_g11.h"
#include "codechal_kernel_header_g11.h"
#include "codechal_kernel_hme_g11.h"
#include "mhw_vdbox_vdenc_g11_X.h"
#include "mhw_vdbox_g11_X.h"
#include "hal_oca_interface.h"
#include "mos_util_user_interface.h"
#if defined(ENABLE_KERNELS)
#include "igcodeckrn_g11.h"
#endif
#if USE_CODECHAL_DEBUG_TOOL
#include "codechal_debug_encode_par_g11.h"
#include "mhw_vdbox_mfx_hwcmd_g11_X.h"
#include "mhw_vdbox_vdenc_hwcmd_g11_X.h"
#endif

struct CodechalVdencAvcStateG11::KernelHeader
{
    int m_kernelCount;
    // Quality mode for Frame/Field
    CODECHAL_KERNEL_HEADER m_mbEncQltyI;
    CODECHAL_KERNEL_HEADER m_mbEncQltyP;
    CODECHAL_KERNEL_HEADER m_mbEncQltyB;
    // Normal mode for Frame/Field
    CODECHAL_KERNEL_HEADER m_mbEncNormI;
    CODECHAL_KERNEL_HEADER m_mbEncNormP;
    CODECHAL_KERNEL_HEADER m_mbEncNormB;
    // Performance modes for Frame/Field
    CODECHAL_KERNEL_HEADER m_mbEncPerfI;
    CODECHAL_KERNEL_HEADER m_mbEncPerfP;
    CODECHAL_KERNEL_HEADER m_mbEncPerfB;
    // Modes for Frame/Field
    CODECHAL_KERNEL_HEADER m_mbEncAdvI;
    CODECHAL_KERNEL_HEADER m_mbEncAdvP;
    CODECHAL_KERNEL_HEADER m_mbEncAdvB;

    // BRC init frame
    CODECHAL_KERNEL_HEADER m_initFrameBrc;
    // Frame BRC update
    CODECHAL_KERNEL_HEADER m_frameEncUpdate;
    // BRC Reset frame
    CODECHAL_KERNEL_HEADER m_brcResetFrame;
    // BRC I Frame Distortion
    CODECHAL_KERNEL_HEADER m_brcIFrameDist;
    // RRCBlockCopy
    CODECHAL_KERNEL_HEADER m_brcBlockCopy;
    // MbBRC Update
    CODECHAL_KERNEL_HEADER m_mbBrcUpdate;
    // 2x DownScaling
    //Weighted Prediction Kernel
    CODECHAL_KERNEL_HEADER m_weightedPrediction;
    // SW scoreboard initialization kernel
    CODECHAL_KERNEL_HEADER m_initSWScoreboard;

};

struct CodechalVdencAvcStateG11::BrcInitDmem
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
    uint8_t     INIT_LookaheadDepth_U8;               // Lookahead depth in unit of frames [0, 127]
    uint8_t     INIT_SinglePassOnly;                  // 0: disabled, 1: enabled
    uint8_t     INIT_New_DeltaQP_Adaptation_U8;       // = 1 to enable new delta QP adaption
    uint8_t     RSVD2[55];                            // must be zero
};

struct CodechalVdencAvcStateG11::BrcUpdateDmem
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
    uint8_t     RSVD3;                                // must be zero
    uint8_t     UPD_ROISource_U8;                     // =0: disable, 1: ROIMap from HME Static Region or from App dirty rectangle, 2: ROIMap from App
    uint8_t     RSVD4;                                // must be zero
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
    uint8_t     DisableDMA;                           // default =0, use DMA data transfer; =1, use regular region read/write
    uint16_t    AdditionalFrameSize_U16;              // for slice size control improvement
    uint8_t     AddNALHeaderSizeInternally_U8;
    uint8_t     UPD_RoiQpViaForceQp_U8;               // HuC does not update StreamIn Buffer, 1: HuC updates StreamIn Buffer
    uint32_t    CABACZeroInsertionSize_U32;           // PAK output via MMIO
    uint32_t    MiniFramePaddingSize_U32;             // PAK output via MMIO
    uint16_t    UPD_WidthInMB_U16;                    // width in MB
    uint16_t    UPD_HeightInMB_U16;                   // height in MB
    int8_t      UPD_ROIQpDelta_I8[8];                 // Application specified ROI QP Adjustment for Zone0, Zone1, Zone2 and Zone3, Zone4, Zone5, Zone6 and Zone7.

    //HME--Offset values need to be a multiple of 4 in order to be aligned to the 4x4 HME block for downscaled 4X HME precision and HME--Offset range is [-128,127]
    int8_t       HME0XOffset_I8;    // default = 32, Frame level X offset from the co-located (0, 0) location for HME0.
    int8_t       HME0YOffset_I8;    // default = 24, Frame level Y offset from the co-located (0, 0) location for HME0.
    int8_t       HME1XOffset_I8;    // default = -32, Frame level X offset from the co-located (0, 0) location for HME1.
    int8_t       HME1YOffset_I8;    // default = -24, Frame level Y offset from the co-located (0, 0) location for HME1.
    uint8_t      MOTION_ADAPTIVE_G4;
    uint8_t      EnableLookAhead;
    uint8_t      UPD_LA_Data_Offset_U8;
    uint8_t      UPD_CQMEnabled_U8;  // 0 indicates CQM is disabled for current frame; otherwise CQM is enabled.
    uint32_t     UPD_LA_TargetSize_U32;     // target frame size in lookahead BRC (if EnableLookAhead == 1) or TCBRC mode. If zero, lookahead BRC or TCBRC is disabled.
    uint32_t     UPD_LA_TargetFulness_U32;  // target VBV buffer fulness in lookahead BRC mode (if EnableLookAhead == 1).
    uint8_t      UPD_Delta_U8;              // delta QP of pyramid
    uint8_t      UPD_ROM_CURRENT_U8;        // ROM average of current frame
    uint8_t      UPD_ROM_ZERO_U8;           // ROM zero percentage (255 is 100%)
    uint8_t      UPD_TCBRC_SCENARIO_U8;
    uint8_t      RSVD2[12];
};

// CURBE for Static Frame Detection kernel
class CodechalVdencAvcStateG11::SfdCurbe
{
public:
    union
    {
        struct
        {
            uint32_t   VDEncModeDisable                    : MOS_BITFIELD_BIT(0);
            uint32_t   BRCModeEnable                       : MOS_BITFIELD_BIT(1);
            uint32_t   SliceType                           : MOS_BITFIELD_RANGE(2, 3);
            uint32_t                                       : MOS_BITFIELD_BIT(4);
            uint32_t   StreamInType                        : MOS_BITFIELD_RANGE(5, 8);
            uint32_t   EnableAdaptiveMvStreamIn            : MOS_BITFIELD_BIT(9);
            uint32_t                                       : MOS_BITFIELD_BIT(10);
            uint32_t   EnableIntraCostScalingForStaticFrame: MOS_BITFIELD_BIT(11);
            uint32_t   Reserved                            : MOS_BITFIELD_RANGE(12, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw0;

    union
    {
        struct
        {
            uint32_t   QPValue                             : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   NumOfRefs                           : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   HMEStreamInRefCost                  : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   Reserved                            : MOS_BITFIELD_RANGE(24, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw1;

    union
    {
        struct
        {
            uint32_t   FrameWidthInMBs                     : MOS_BITFIELD_RANGE(0, 15);     // round-up to 4-MB aligned
            uint32_t   FrameHeightInMBs                    : MOS_BITFIELD_RANGE(16, 31);     // round-up to 4-MB aligned
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw2;

    union
    {
        struct
        {
            uint32_t   LargeMvThresh                       : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw3;

    union
    {
        struct
        {
            uint32_t   TotalLargeMvThreshold               : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw4;

    union
    {
        struct
        {
            uint32_t   ZMVThreshold                        : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw5;

    union
    {
        struct
        {
            uint32_t   TotalZMVThreshold                   : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw6;

    union
    {
        struct
        {
            uint32_t   MinDistThreshold                    : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw7;

    uint8_t m_costTable[52];

    union
    {
        struct
        {
            uint32_t   ActualWidthInMB                     : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   ActualHeightInMB                    : MOS_BITFIELD_RANGE(16, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw21;

    union
    {
        struct
        {
            uint32_t   Reserved                            : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw22;

    union
    {
        struct
        {
            uint32_t   Reserved                            : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw23;

    union
    {
        struct
        {
            uint32_t   VDEncInputImagStateIndex            : MOS_BITFIELD_RANGE(0, 31);      // used in VDEnc CQP mode
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw24;

    union
    {
        struct
        {
            uint32_t   Reserved                            : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw25;

    union
    {
        struct
        {
            uint32_t   MVDataSurfaceIndex                  : MOS_BITFIELD_RANGE(0, 31);      // contains HME MV Data generated by HME kernel
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw26;

    union
    {
        struct
        {
            uint32_t   InterDistortionSurfaceIndex         : MOS_BITFIELD_RANGE(0, 31);      // contains HME Inter Distortion generated by HME kernel
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw27;

    union
    {
        struct
        {
            uint32_t   OutputDataSurfaceIndex              : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw28;

    union
    {
        struct
        {
            uint32_t   VDEncOutputImagStateIndex           : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw29;

    SfdCurbe()
    {
        m_dw0.Value  = 0;
        m_dw1.Value  = 0;
        m_dw2.Value  = 0;
        m_dw3.Value  = 0;
        m_dw4.Value  = 0;
        m_dw5.Value  = 0;
        m_dw6.Value  = 0;
        m_dw7.Value  = 0;
        m_dw21.Value = 0;
        m_dw22.Value = 0;
        m_dw23.Value = 0;
        m_dw24.Value = 0;
        m_dw25.Value = 0;
        m_dw26.Value = 0;
        m_dw27.Value = 0;
        m_dw28.Value = 0;
        m_dw29.Value = 0;

        for (uint8_t i = 0; i < 52; i++)
        {
            m_costTable[i] = 0;
        }
    };
};

enum SfdBindingTableOffset
{
    sfdVdencInputImageState = 0,
    sfdMvDataSurface = 1,
    sfdInterDistortionSurface = 2,
    sfdOutputDataSurface = 3,
    sfdVdencOutputImageState = 4,
    sfdNumSurfaces = 5
};

const uint32_t CodechalVdencAvcStateG11::m_mvCostSkipBiasQPel[3][8] =
{
    // for normal case
    { 0, 6, 6, 9, 10, 13, 14, 16 },
    // for QP = 47,48,49
    { 0, 6, 6, 6, 6, 7, 8, 8 },
    // for QP = 50,51
    { 0, 6, 6, 6, 6, 7, 7, 7 }
};

const uint32_t CodechalVdencAvcStateG11::m_hmeCostDisplayRemote[8][CODEC_AVC_NUM_QP] =
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

const uint32_t CodechalVdencAvcStateG11::m_hmeCost[8][CODEC_AVC_NUM_QP] =
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

const int8_t CodechalVdencAvcStateG11::m_brcInitDistQpDeltaI8[4] =
{
    0, 0, 0, 0
};

const int8_t CodechalVdencAvcStateG11::m_brcInitDistQpDeltaI8LowDelay[4] =
{
    -5, -2, 2, 5
};

MOS_STATUS CodechalVdencAvcStateG11::GetKernelHeaderAndSize(
    void                         *binary,
    EncOperation                 operation,
    uint32_t                     krnStateIdx,
    void                         *krnHeader,
    uint32_t                     *krnSize)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(binary);
    CODECHAL_ENCODE_CHK_NULL_RETURN(krnHeader);
    CODECHAL_ENCODE_CHK_NULL_RETURN(krnSize);

    auto kernelHeaderTable = (KernelHeader *)binary;
    PCODECHAL_KERNEL_HEADER invalidEntry = &(kernelHeaderTable->m_weightedPrediction) + 1;
    PCODECHAL_KERNEL_HEADER nextKrnHeader = nullptr;
    PCODECHAL_KERNEL_HEADER currKrnHeader = nullptr;

    if (operation == ENC_BRC)
    {
        currKrnHeader = &kernelHeaderTable->m_initFrameBrc;
    }
    else if (operation == ENC_MBENC)
    {
        currKrnHeader = &kernelHeaderTable->m_mbEncQltyI;
    }
    else if (operation == ENC_MBENC_ADV)
    {
        currKrnHeader = &kernelHeaderTable->m_mbEncAdvI;
    }
    else if (operation == ENC_WP)
    {
        currKrnHeader = &kernelHeaderTable->m_weightedPrediction;
    }
    else
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Unsupported ENC mode requested");
        return MOS_STATUS_INVALID_PARAMETER;
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

CodechalVdencAvcStateG11::CodechalVdencAvcStateG11(
        CodechalHwInterface *   hwInterface,
        CodechalDebugInterface *debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo)  : CodechalVdencAvcState(hwInterface, debugInterface, standardInfo),
    m_sinlgePipeVeState(nullptr)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_ASSERT(m_osInterface);

#if defined(ENABLE_KERNELS)
    m_kernelBase = (uint8_t*)IGCODECKRN_G11;
#endif
    m_cmKernelEnable = true;
    m_mbStatsSupported = true; //Starting from GEN9

    pfnGetKernelHeaderAndSize    = CodechalVdencAvcStateG11::GetKernelHeaderAndSize;

    m_vdencBrcInitDmemBufferSize   = sizeof(BrcInitDmem);
    m_vdencBrcUpdateDmemBufferSize = sizeof(BrcUpdateDmem);
    m_vdencBrcNumOfSliceOffset = CODECHAL_OFFSETOF(BrcUpdateDmem, NumOfSlice);

    // Virtual Engine is enabled in default.
    Mos_SetVirtualEngineSupported(m_osInterface, true);

    m_vdboxOneDefaultUsed = true;

    m_hmeSupported   = true;
    m_16xMeSupported = true;
    m_32xMeSupported = true;

    Mos_CheckVirtualEngineSupported(m_osInterface, false, true);

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_NULL_NO_STATUS_RETURN(m_encodeParState = MOS_New(CodechalDebugEncodeParG11, this));
        CreateAvcPar();
    )
}

CodechalVdencAvcStateG11::~CodechalVdencAvcStateG11()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (m_sinlgePipeVeState)
    {
        MOS_FreeMemAndSetNull(m_sinlgePipeVeState);
    }

    CODECHAL_DEBUG_TOOL(
        DestroyAvcPar();
        MOS_Delete(m_encodeParState);
    )
}

MOS_STATUS CodechalVdencAvcStateG11::InitializeState()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalVdencAvcState::InitializeState());

    m_sliceSizeStreamoutSupported = true;
    m_useHwScoreboard = false;
    m_useCommonKernel = true;

    if (MOS_VE_SUPPORTED(m_osInterface))
    {
        m_sinlgePipeVeState = (PCODECHAL_ENCODE_SINGLEPIPE_VIRTUALENGINE_STATE)MOS_AllocAndZeroMemory(sizeof(CODECHAL_ENCODE_SINGLEPIPE_VIRTUALENGINE_STATE));
        CODECHAL_ENCODE_CHK_NULL_RETURN(m_sinlgePipeVeState);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalEncodeSinglePipeVE_InitInterface(m_hwInterface, m_sinlgePipeVeState));
    }

    return eStatus;
}

MOS_STATUS CodechalVdencAvcStateG11::SetAndPopulateVEHintParams(
    PMOS_COMMAND_BUFFER  cmdBuffer)
{
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (!MOS_VE_SUPPORTED(m_osInterface))
    {
        return eStatus;
    }

    if (!MOS_VE_CTXBASEDSCHEDULING_SUPPORTED(m_osInterface))
    {
        MOS_VIRTUALENGINE_SET_PARAMS  vesetParams;
        MOS_ZeroMemory(&vesetParams, sizeof(vesetParams));
        vesetParams.bNeedSyncWithPrevious = true;
        vesetParams.bSFCInUse = false;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalEncodeSinglePipeVE_SetHintParams(m_sinlgePipeVeState, &vesetParams));
    }
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalEncodeSinglePipeVE_PopulateHintParams(m_sinlgePipeVeState, cmdBuffer, true));

    return eStatus;
}


MOS_STATUS CodechalVdencAvcStateG11::SetGpuCtxCreatOption()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (!MOS_VE_CTXBASEDSCHEDULING_SUPPORTED(m_osInterface))
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncoderState::SetGpuCtxCreatOption());
    }
    else
    {
        m_gpuCtxCreatOpt = MOS_New(MOS_GPUCTX_CREATOPTIONS_ENHANCED);
        CODECHAL_ENCODE_CHK_NULL_RETURN(m_gpuCtxCreatOpt);

        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalEncodeSinglePipeVE_ConstructParmsForGpuCtxCreation(
            m_sinlgePipeVeState,
            (PMOS_GPUCTX_CREATOPTIONS_ENHANCED)m_gpuCtxCreatOpt));
    }

    return eStatus;
}

MOS_STATUS CodechalVdencAvcStateG11::UserFeatureKeyReport()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalVdencAvcState::UserFeatureKeyReport());

#if (_DEBUG || _RELEASE_INTERNAL)

    // VE2.0 Reporting
    CodecHalEncode_WriteKey(__MEDIA_USER_FEATURE_VALUE_ENABLE_ENCODE_VE_CTXSCHEDULING_ID, MOS_VE_CTXBASEDSCHEDULING_SUPPORTED(m_osInterface), m_osInterface->pOsContext);

#endif // _DEBUG || _RELEASE_INTERNAL
    return eStatus;
}

MOS_STATUS CodechalVdencAvcStateG11::ExecuteSliceLevel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_osInterface->osCpInterface);

    auto cpInterface = m_hwInterface->GetCpInterface();
    auto avcSlcParams = m_avcSliceParams;
    auto avcPicParams = m_avcPicParams[avcSlcParams->pic_parameter_set_id];
    auto avcSeqParams = m_avcSeqParams[avcPicParams->seq_parameter_set_id];
    auto slcData = m_slcData;

    // For use with the single task phase implementation
    if (m_sliceStructCaps != CODECHAL_SLICE_STRUCT_ARBITRARYMBSLICE)
    {
        uint32_t numSlc = (m_frameFieldHeightInMb + m_sliceHeight - 1) / m_sliceHeight;

        if (numSlc != m_numSlices)
        {
            return MOS_STATUS_INVALID_PARAMETER;
        }
    }

    bool useBatchBufferForPakSlices = false;
    if (m_singleTaskPhaseSupported  && m_singleTaskPhaseSupportedInPak)
    {
        if (m_currPass == 0)
        {
            // The same buffer is used for all slices for all passes.
            uint32_t batchBufferForPakSlicesSize =
                (m_numPasses + 1) * m_numSlices * m_pakSliceSize;
            if (batchBufferForPakSlicesSize >
                (uint32_t)m_batchBufferForPakSlices[m_currRecycledBufIdx].iSize)
            {
                if (m_batchBufferForPakSlices[m_currRecycledBufIdx].iSize)
                {
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(ReleaseBatchBufferForPakSlices(m_currRecycledBufIdx));
                }

                CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBatchBufferForPakSlices(
                    m_numSlices,
                    m_numPasses,
                    m_currRecycledBufIdx));
            }
        }
        CODECHAL_ENCODE_CHK_STATUS_RETURN(Mhw_LockBb(
            m_osInterface,
            &m_batchBufferForPakSlices[m_currRecycledBufIdx]));
        useBatchBufferForPakSlices = true;
    }

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    if (m_osInterface->osCpInterface->IsCpEnabled())
    {
        MHW_CP_SLICE_INFO_PARAMS sliceInfoParam;
        sliceInfoParam.bLastPass = (m_currPass == m_numPasses) ? true : false;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cpInterface->SetMfxProtectionState(false, &cmdBuffer, nullptr, &sliceInfoParam));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(cpInterface->UpdateParams(false));
    }

    avcSlcParams = m_avcSliceParams;

    CODECHAL_ENCODE_AVC_PACK_SLC_HEADER_PARAMS packSlcHeaderParams;
    packSlcHeaderParams.pBsBuffer = &m_bsBuffer;
    packSlcHeaderParams.pPicParams = avcPicParams;
    packSlcHeaderParams.pSeqParams = m_avcSeqParam;
    packSlcHeaderParams.ppRefList = &(m_refList[0]);
    packSlcHeaderParams.CurrPic = m_currOriginalPic;
    packSlcHeaderParams.CurrReconPic = m_currReconstructedPic;
    packSlcHeaderParams.UserFlags = m_userFlags;
    packSlcHeaderParams.NalUnitType = m_nalUnitType;
    packSlcHeaderParams.wPictureCodingType = m_pictureCodingType;
    packSlcHeaderParams.bVdencEnabled = true;

    MHW_VDBOX_AVC_SLICE_STATE sliceState;
    MOS_ZeroMemory(&sliceState, sizeof(sliceState));
    sliceState.presDataBuffer = &m_resMbCodeSurface;
    sliceState.pAvcPicIdx = &(m_picIdx[0]);
    sliceState.pEncodeAvcSeqParams = m_avcSeqParam;
    sliceState.pEncodeAvcPicParams = avcPicParams;
    sliceState.pBsBuffer = &m_bsBuffer;
    sliceState.ppNalUnitParams = m_nalUnitParams;
    sliceState.bBrcEnabled = false;
    // Disable Panic mode when min/max QP control is on. kernel may disable it, but disable in driver also.
    sliceState.bRCPanicEnable = m_panicEnable && (!m_minMaxQpControlEnabled);
    sliceState.bAcceleratorHeaderPackingCaps = m_encodeParams.bAcceleratorHeaderPackingCaps;
    sliceState.wFrameFieldHeightInMB = m_frameFieldHeightInMb;

    MHW_VDBOX_VD_PIPE_FLUSH_PARAMS vdPipelineFlushParams;
    for (uint16_t slcCount = 0; slcCount < m_numSlices; slcCount++)
    {
        if (m_currPass == 0)
        {
            packSlcHeaderParams.pAvcSliceParams = &avcSlcParams[slcCount];
            if (m_acceleratorHeaderPackingCaps)
            {
                slcData[slcCount].SliceOffset = m_bsBuffer.SliceOffset;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalAvcEncode_PackSliceHeader(&packSlcHeaderParams));
                slcData[slcCount].BitSize = m_bsBuffer.BitSize;
            }
            if (m_sliceStructCaps != CODECHAL_SLICE_STRUCT_ARBITRARYMBSLICE)
            {
                slcData[slcCount].CmdOffset = slcCount * m_sliceHeight * m_picWidthInMb * 16 * 4;
            }
            else
            {
                slcData[slcCount].CmdOffset = packSlcHeaderParams.pAvcSliceParams->first_mb_in_slice * 16 * 4;
            }
        }

        sliceState.pEncodeAvcSliceParams = &avcSlcParams[slcCount];
        sliceState.dwDataBufferOffset =
            m_slcData[slcCount].CmdOffset + m_mbcodeBottomFieldOffset;
        sliceState.dwOffset = slcData[slcCount].SliceOffset;
        sliceState.dwLength = slcData[slcCount].BitSize;
        sliceState.uiSkipEmulationCheckCount = slcData[slcCount].SkipEmulationByteCount;
        sliceState.dwSliceIndex = (uint32_t)slcCount;
        sliceState.bFirstPass = (m_currPass == 0);
        sliceState.bLastPass = (m_currPass == m_numPasses);
        sliceState.bInsertBeforeSliceHeaders = (slcCount == 0);
        sliceState.bVdencInUse = true;
        // App handles tail insertion for VDEnc dynamic slice in non-cp case
        sliceState.bVdencNoTailInsertion = m_vdencNoTailInsertion;

        uint32_t batchBufferForPakSlicesStartOffset =
            (uint32_t)m_batchBufferForPakSlices[m_currRecycledBufIdx].iCurrent;

        if (useBatchBufferForPakSlices)
        {
            sliceState.pBatchBufferForPakSlices =
                &m_batchBufferForPakSlices[m_currRecycledBufIdx];
            sliceState.bSingleTaskPhaseSupported = true;
            sliceState.dwBatchBufferForPakSlicesStartOffset = batchBufferForPakSlicesStartOffset;
        }

        if (m_avcRoundingParams != nullptr && m_avcRoundingParams->bEnableCustomRoudingIntra)
        {
            sliceState.dwRoundingIntraValue = m_avcRoundingParams->dwRoundingIntra;
        }
        else
        {
            sliceState.dwRoundingIntraValue = 5;
        }
        if (m_avcRoundingParams != nullptr && m_avcRoundingParams->bEnableCustomRoudingInter)
        {
            sliceState.bRoundingInterEnable = true;
            sliceState.dwRoundingValue = m_avcRoundingParams->dwRoundingInter;
        }
        else
        {
            sliceState.bRoundingInterEnable = m_roundingInterEnable;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(GetInterRounding(&sliceState));
        }

        sliceState.oneOnOneMapping = m_oneOnOneMapping;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SendSlice(&cmdBuffer, &sliceState));

        // Add dumps for 2nd level batch buffer
        if (sliceState.bSingleTaskPhaseSupported && !sliceState.bVdencInUse)
        {
            CODECHAL_ENCODE_CHK_NULL_RETURN(sliceState.pBatchBufferForPakSlices);

            CODECHAL_DEBUG_TOOL(
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->Dump2ndLvlBatch(
                    sliceState.pBatchBufferForPakSlices,
                    CODECHAL_MEDIA_STATE_ENC_NORMAL,
                    nullptr));
            )
        }

        // For SKL, only the 1st slice state should be programmed for VDENC
        if (!m_hwInterface->m_isVdencSuperSliceEnabled)
        {
            break;
        }
        else // For CNL slice state is programmed per Super slice
        {
            MOS_ZeroMemory(&vdPipelineFlushParams, sizeof(vdPipelineFlushParams));
            // MfxPipeDone should be set for all super slices except the last super slice and should not be set for tail insertion.
            vdPipelineFlushParams.Flags.bWaitDoneMFX =
                (slcCount == (m_numSlices)-1) ? ((m_lastPicInStream || m_lastPicInSeq) ? 0 : 1) : 1;
            vdPipelineFlushParams.Flags.bWaitDoneVDENC = 1;
            vdPipelineFlushParams.Flags.bFlushVDENC = 1;
            vdPipelineFlushParams.Flags.bWaitDoneVDCmdMsgParser = 1;

            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdPipelineFlushCmd(&cmdBuffer, &vdPipelineFlushParams));

            //Do not send MI_FLUSH for last Super slice now
            if (slcCount != ((m_numSlices)-1))
            {
                // Send MI_FLUSH for every Super slice
                MHW_MI_FLUSH_DW_PARAMS flushDwParams;
                MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(
                    &cmdBuffer,
                    &flushDwParams));
            }
        }
    }

    if (useBatchBufferForPakSlices)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(Mhw_UnlockBb(
            m_osInterface,
            &m_batchBufferForPakSlices[m_currRecycledBufIdx],
            m_lastTaskInPhase));
    }

    //Send VDENC WALKER cmd per every frame for SKL
    if (!m_hwInterface->m_isVdencSuperSliceEnabled)
    {
        PMHW_VDBOX_VDENC_WALKER_STATE_PARAMS vdencWalkerStateParams = CreateMhwVdboxVdencWalkerStateParams();
        CODECHAL_ENCODE_CHK_NULL_RETURN(vdencWalkerStateParams);
        vdencWalkerStateParams->Mode = CODECHAL_ENCODE_MODE_AVC;
        vdencWalkerStateParams->pAvcSeqParams = avcSeqParams;
        vdencWalkerStateParams->pAvcSlcParams = avcSlcParams;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdencWalkerStateCmd(&cmdBuffer, vdencWalkerStateParams));
        MOS_Delete(vdencWalkerStateParams);

        MOS_ZeroMemory(&vdPipelineFlushParams, sizeof(vdPipelineFlushParams));
        // MFXPipeDone should not be set for tail insertion
        vdPipelineFlushParams.Flags.bWaitDoneMFX =
            (m_lastPicInStream || m_lastPicInSeq) ? 0 : 1;
        vdPipelineFlushParams.Flags.bWaitDoneVDENC = 1;
        vdPipelineFlushParams.Flags.bFlushVDENC = 1;
        vdPipelineFlushParams.Flags.bWaitDoneVDCmdMsgParser = 1;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdPipelineFlushCmd(&cmdBuffer, &vdPipelineFlushParams));
    }

    // Insert end of sequence/stream if set
    if (m_lastPicInStream || m_lastPicInSeq)
    {
        MHW_VDBOX_PAK_INSERT_PARAMS pakInsertObjectParams;
        MOS_ZeroMemory(&pakInsertObjectParams, sizeof(pakInsertObjectParams));
        pakInsertObjectParams.bLastPicInSeq = m_lastPicInSeq;
        pakInsertObjectParams.bLastPicInStream = m_lastPicInStream;
        pakInsertObjectParams.dwBitSize = 32;   // use dwBitSize for SrcDataEndingBitInclusion
        if (m_lastPicInSeq)
        {
            pakInsertObjectParams.dwLastPicInSeqData = (uint32_t)((1 << 16) | CODECHAL_ENCODE_AVC_NAL_UT_EOSEQ << 24);
        }
        if (m_lastPicInStream)
        {
            pakInsertObjectParams.dwLastPicInStreamData = (uint32_t)((1 << 16) | CODECHAL_ENCODE_AVC_NAL_UT_EOSTREAM << 24);
        }
        pakInsertObjectParams.bHeaderLengthExcludeFrmSize = true;
        if (pakInsertObjectParams.bEmulationByteBitsInsert)
        {
            //Does not matter here, but keeping for consistency
            CODECHAL_ENCODE_ASSERTMESSAGE("The emulation prevention bytes are not inserted by the app and are requested to be inserted by HW.");
        }
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxPakInsertObject(&cmdBuffer, nullptr, &pakInsertObjectParams));
    }

    if (m_hwInterface->m_isVdencSuperSliceEnabled)
    {
        // Send MI_FLUSH with bVideoPipelineCacheInvalidate set to true for last Super slice
        MHW_MI_FLUSH_DW_PARAMS flushDwParams;
        MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
        flushDwParams.bVideoPipelineCacheInvalidate = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(
            &cmdBuffer,
            &flushDwParams));
    }

#if defined(ENABLE_KERNELS)
    // On-demand sync for VDEnc StreamIn surface and CSC surface
    if (m_currPass == 0)
    {
        if (m_cscDsState->RequireCsc())
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cscDsState->WaitCscSurface(m_videoContext, true));
        }

        if (m_16xMeSupported)
        {
            auto syncParams = g_cInitSyncParams;
            syncParams.GpuContext = m_videoContext;
            syncParams.bReadOnly = true;
            syncParams.presSyncResource = &m_resVdencStreamInBuffer[m_currRecycledBufIdx];
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnResourceWait(m_osInterface, &syncParams));
            m_osInterface->pfnSetResourceSyncTag(m_osInterface, &syncParams);
        }
    }
#endif

    CODECHAL_ENCODE_CHK_STATUS_RETURN(ReadMfcStatus(&cmdBuffer));

    if (m_vdencBrcEnabled)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(StoreNumPasses(
            &(m_encodeStatusBuf),
            m_miInterface,
            &cmdBuffer,
            m_currPass));
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(EndStatusReport(&cmdBuffer, CODECHAL_NUM_MEDIA_STATES));

    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(&cmdBuffer, nullptr));
    }

    std::string pak_pass = "PAK_PASS" + std::to_string(static_cast<uint32_t>(m_currPass));
    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
            &cmdBuffer,
            CODECHAL_NUM_MEDIA_STATES,
            pak_pass.data()));

    //CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_DbgReplaceAllCommands(
    //    m_debugInterface,
    //    &cmdBuffer));
    )

        m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    bool renderingFlags = m_videoContextUsesNullHw;

    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        // Restore TLB allocation
        if (MEDIA_IS_WA(m_waTable, WaTlbAllocationForAvcVdenc))
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(RestoreTLBAllocation(&cmdBuffer, &m_vdencTlbMmioBuffer));
        }

        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetAndPopulateVEHintParams(&cmdBuffer));

        HalOcaInterface::On1stLevelBBEnd(cmdBuffer, *m_osInterface);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, renderingFlags));

        CODECHAL_DEBUG_TOOL(
            if (m_mmcState)
            {
                m_mmcState->UpdateUserFeatureKey(&m_reconSurface);
            }
        )

            if (m_sliceSizeStreamoutSupported)
            {
                CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                    &m_pakSliceSizeStreamoutBuffer,
                    CodechalDbgAttr::attrOutput,
                    "SliceSizeStreamout",
                    CODECHAL_ENCODE_SLICESIZE_BUF_SIZE,
                    0,
                    CODECHAL_NUM_MEDIA_STATES)));
            }

        if ((m_currPass == m_numPasses) &&
            m_signalEnc &&
            !Mos_ResourceIsNull(&m_resSyncObjectVideoContextInUse))
        {
            // Check if the signal obj count exceeds max value
            if (m_semaphoreObjCount == MOS_MIN(m_semaphoreMaxCount, MOS_MAX_OBJECT_SIGNALED))
            {
                auto syncParams = g_cInitSyncParams;
                syncParams.GpuContext = m_renderContext;
                syncParams.presSyncResource = &m_resSyncObjectVideoContextInUse;

                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineWait(m_osInterface, &syncParams));
                m_semaphoreObjCount--;
            }

            // signal semaphore
            auto syncParams = g_cInitSyncParams;
            syncParams.GpuContext = m_videoContext;
            syncParams.presSyncResource = &m_resSyncObjectVideoContextInUse;

            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineSignal(m_osInterface, &syncParams));
            m_semaphoreObjCount++;
        }
    }

    CODECHAL_DEBUG_TOOL(
        // here add the dump buffer for PAK statistics.
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &m_pakStatsBufferFull,
            CodechalDbgAttr::attrInput,
            "MB and FrameLevel PAK staistics vdenc",
            m_vdencBrcPakStatsBufferSize + m_picWidthInMb * m_picHeightInMb * 64,   //size
            0, //offset
            CODECHAL_MEDIA_STATE_16X_ME));
    )

    if (m_vdencBrcEnabled)
    {
        CODECHAL_DEBUG_TOOL(DumpHucBrcUpdate(false));
        CODECHAL_DEBUG_TOOL(DumpEncodeImgStats(nullptr));
    }

    // Reset parameters for next PAK execution
    if (m_currPass == m_numPasses)
    {
        if (!m_singleTaskPhaseSupported)
        {
            m_osInterface->pfnResetPerfBufferID(m_osInterface);
        }

        m_newPpsHeader = 0;
        m_newSeqHeader = 0;
    }

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(PopulateSliceStateParam(
            m_adaptiveRoundingInterEnable,
            &sliceState));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(DumpFrameParFile());)

        return eStatus;
}

MOS_STATUS CodechalVdencAvcStateG11::InitKernelStateSFD()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    auto renderEngineInterface = m_hwInterface->GetRenderInterface();
    auto stateHeapInterface = m_renderEngineInterface->m_stateHeapInterface;
    CODECHAL_ENCODE_CHK_NULL_RETURN(stateHeapInterface);

    uint8_t* kernelBinary;
    uint32_t kernelSize;

    MOS_STATUS status = CodecHalGetKernelBinaryAndSize(m_kernelBase, m_kuidCommon, &kernelBinary, &kernelSize);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(status);

    CODECHAL_KERNEL_HEADER currKrnHeader;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(GetCommonKernelHeaderAndSizeG11(
                kernelBinary,
                ENC_SFD,
                0,
                (void*)&currKrnHeader,
                &kernelSize));

    auto kernelStatePtr                            = m_sfdKernelState;
    kernelStatePtr->KernelParams.iBTCount = sfdNumSurfaces;
    kernelStatePtr->KernelParams.iThreadCount = m_renderEngineInterface->GetHwCaps()->dwMaxThreads;
    kernelStatePtr->KernelParams.iCurbeLength = sizeof(SfdCurbe);
    kernelStatePtr->KernelParams.iBlockWidth = CODECHAL_MACROBLOCK_WIDTH;
    kernelStatePtr->KernelParams.iBlockHeight = CODECHAL_MACROBLOCK_HEIGHT;
    kernelStatePtr->KernelParams.iIdCount = 1;
    kernelStatePtr->KernelParams.iInlineDataLength = 0;

    kernelStatePtr->dwCurbeOffset = stateHeapInterface->pStateHeapInterface->GetSizeofCmdInterfaceDescriptorData();
    kernelStatePtr->KernelParams.pBinary = kernelBinary + (currKrnHeader.KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);
    kernelStatePtr->KernelParams.iSize = kernelSize;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(stateHeapInterface->pfnCalculateSshAndBtSizesRequested(
                stateHeapInterface,
                kernelStatePtr->KernelParams.iBTCount,
                &kernelStatePtr->dwSshSize,
                &kernelStatePtr->dwBindingTableSize));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->MhwInitISH(stateHeapInterface, kernelStatePtr));

    return eStatus;
}

bool CodechalVdencAvcStateG11::CheckSupportedFormat(PMOS_SURFACE surface)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    bool colorFormatSupported = true;
    if (IS_Y_MAJOR_TILE_FORMAT(surface->TileType))
    {
        switch (surface->Format)
        {
        case Format_NV12:
            break;
        default:
            colorFormatSupported = false;
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
        case Format_A8R8G8B8:
        case Format_A8B8G8R8:
            break;
        default:
            colorFormatSupported = false;
            break;
        }
    }
    else
    {
        colorFormatSupported = false;
    }

    return colorFormatSupported;
}

MOS_STATUS CodechalVdencAvcStateG11::GetTrellisQuantization(PCODECHAL_ENCODE_AVC_TQ_INPUT_PARAMS params, PCODECHAL_ENCODE_AVC_TQ_PARAMS trellisQuantParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(trellisQuantParams);

    trellisQuantParams->dwTqEnabled = TrellisQuantizationEnable[params->ucTargetUsage];
    trellisQuantParams->dwTqRounding = trellisQuantParams->dwTqEnabled ? TrellisQuantizationRounding[params->ucTargetUsage] : 0;

    return eStatus;
}

MOS_STATUS CodechalVdencAvcStateG11::AddHucOutputRegistersHandling(
    MmioRegistersHuc*   mmioRegisters,
    PMOS_COMMAND_BUFFER cmdBuffer,
    bool                addToEncodeStatus)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(mmioRegisters);
    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);

    return StoreHucErrorStatus(mmioRegisters, cmdBuffer, addToEncodeStatus);
}

MOS_STATUS CodechalVdencAvcStateG11::SetDmemHuCBrcInitReset()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    // Setup BRC DMEM
    MOS_LOCK_PARAMS lockFlagsWriteOnly;
    MOS_ZeroMemory(&lockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
    lockFlagsWriteOnly.WriteOnly = 1;
    auto dmem                    = (BrcInitDmem *)m_osInterface->pfnLockResource(
        m_osInterface, &m_resVdencBrcInitDmemBuffer[m_currRecycledBufIdx], &lockFlagsWriteOnly);

    CODECHAL_ENCODE_CHK_NULL_RETURN(dmem);
    MOS_ZeroMemory(dmem, sizeof(BrcInitDmem));

    SetDmemHuCBrcInitResetImpl<BrcInitDmem>(dmem);

    // fractional QP enable for extended rho domain
    dmem->INIT_FracQPEnable_U8 = m_lookaheadDepth > 0 ? 0 : (uint8_t)m_vdencInterface->IsRhoDomainStatsEnabled();

    dmem->INIT_SinglePassOnly = m_vdencSinglePassEnable;

    if (m_avcSeqParam->ScenarioInfo == ESCENARIO_GAMESTREAMING)
    {
        if (m_avcSeqParam->RateControlMethod == RATECONTROL_VBR)
        {
            m_avcSeqParam->MaxBitRate = m_avcSeqParam->TargetBitRate;
        }

        // Disable delta QP adaption for non-VCM/ICQ/LowDelay until we have better algorithm
        if ((m_avcSeqParam->RateControlMethod != RATECONTROL_VCM) &&
            (m_avcSeqParam->RateControlMethod != RATECONTROL_ICQ) &&
            (m_avcSeqParam->FrameSizeTolerance != EFRAMESIZETOL_EXTREMELY_LOW))
        {
            dmem->INIT_DeltaQP_Adaptation_U8 = 0;
        }

        dmem->INIT_New_DeltaQP_Adaptation_U8 = 1;
    }

    if (((m_avcSeqParam->TargetUsage & 0x07) == TARGETUSAGE_BEST_SPEED) &&
        (m_avcSeqParam->FrameWidth >= m_singlePassMinFrameWidth) &&
        (m_avcSeqParam->FrameHeight >= m_singlePassMinFrameHeight) &&
        (m_avcSeqParam->FramesPer100Sec >= m_singlePassMinFramePer100s))
    {
        dmem->INIT_SinglePassOnly = true;
    }

    dmem->INIT_LookaheadDepth_U8 = m_lookaheadDepth;

    //Override the DistQPDelta.
    if (m_mbBrcEnabled)
    {
        if (m_avcSeqParam->FrameSizeTolerance == EFRAMESIZETOL_EXTREMELY_LOW)
        {
            MOS_SecureMemcpy(dmem->INIT_DistQPDelta_I8, 4 * sizeof(int8_t), (void*)m_brcInitDistQpDeltaI8LowDelay, 4 * sizeof(int8_t));
        }
        else
        {
            MOS_SecureMemcpy(dmem->INIT_DistQPDelta_I8, 4 * sizeof(int8_t), (void*)m_brcInitDistQpDeltaI8, 4 * sizeof(int8_t));
        }
    }

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(PopulateBrcInitParam(
            dmem));
    )

    m_osInterface->pfnUnlockResource(m_osInterface, &m_resVdencBrcInitDmemBuffer[m_currRecycledBufIdx]);

    return eStatus;
}

MOS_STATUS CodechalVdencAvcStateG11::SetDmemHuCBrcUpdate()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    // Program update DMEM
    MOS_LOCK_PARAMS lockFlags;
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.WriteOnly = 1;
    auto dmem           = (BrcUpdateDmem *)m_osInterface->pfnLockResource(
        m_osInterface, &m_resVdencBrcUpdateDmemBuffer[m_currRecycledBufIdx][m_currPass], &lockFlags);
    CODECHAL_ENCODE_CHK_NULL_RETURN(dmem);
    SetDmemHuCBrcUpdateImpl<BrcUpdateDmem>(dmem);

    MOS_LOCK_PARAMS lockFlagsReadOnly;
    MOS_ZeroMemory(&lockFlagsReadOnly, sizeof(MOS_LOCK_PARAMS));
    lockFlagsReadOnly.ReadOnly = 1;
    auto initDmem              = (BrcInitDmem *)m_osInterface->pfnLockResource(
        m_osInterface, &m_resVdencBrcInitDmemBuffer[m_currRecycledBufIdx], &lockFlagsReadOnly);

    if (initDmem->INIT_AdaptiveHMEExtensionEnable_U8)
    {
        dmem->HME0XOffset_I8 = 32;
        dmem->HME0YOffset_I8 = 24;
        dmem->HME1XOffset_I8 = -32;
        dmem->HME1YOffset_I8 = -24;
    }

    m_osInterface->pfnUnlockResource(m_osInterface, &m_resVdencBrcInitDmemBuffer[m_currRecycledBufIdx]);

    if (m_16xMeSupported && (m_pictureCodingType == P_TYPE))
    {
        dmem->HmeDistAvailable_U8 = 1;
    }
    dmem->UPD_WidthInMB_U16 = m_picWidthInMb;
    dmem->UPD_HeightInMB_U16 = m_picHeightInMb;

    dmem->MOTION_ADAPTIVE_G4 = (m_avcSeqParam->ScenarioInfo == ESCENARIO_GAMESTREAMING);
    dmem->UPD_CQMEnabled_U8  = m_avcSeqParam->seq_scaling_matrix_present_flag || m_avcPicParam->pic_scaling_matrix_present_flag;

    dmem->UPD_LA_TargetSize_U32 = m_avcPicParam->TargetFrameSize << 3;

    if (m_lookaheadDepth > 0)
    {
        dmem->EnableLookAhead = 1;
        dmem->UPD_LA_TargetFulness_U32 = m_targetBufferFulness;
        dmem->UPD_Delta_U8 = m_avcPicParam->QpModulationStrength;
    }

    dmem->UPD_TCBRC_SCENARIO_U8 = m_avcSeqParam->bAutoMaxPBFrameSizeForSceneChange;

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(PopulateBrcUpdateParam(
            dmem));
    )

    m_osInterface->pfnUnlockResource(m_osInterface, &(m_resVdencBrcUpdateDmemBuffer[m_currRecycledBufIdx][m_currPass]));

    return eStatus;
}

MOS_STATUS CodechalVdencAvcStateG11::LoadMvCost(uint8_t qp)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    for (uint8_t i=0; i< 8; i++)
    {
        m_vdEncMvCost[i] = Map44LutValue((uint32_t)(m_mvCostSkipBiasQPel[0][i]), 0x6f);
    }

    if (!m_vdencBrcEnabled)
    {
        if (qp == 47 || qp == 48 || qp == 49)
        {
            for (uint8_t i = 3; i < 8; i++)
            {
                m_vdEncMvCost[i] = Map44LutValue((uint32_t)(m_mvCostSkipBiasQPel[1][i]), 0x6f);
            }
        }
        if (qp == 50 || qp == 51)
        {
            for (uint8_t i = 3; i < 8; i++)
            {
                m_vdEncMvCost[i] = Map44LutValue((uint32_t)(m_mvCostSkipBiasQPel[2][i]), 0x6f);
            }
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalVdencAvcStateG11::LoadHmeMvCost(uint8_t qp)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    PCODEC_AVC_ENCODE_SEQUENCE_PARAMS avcSeqParams = m_avcSeqParam;
    const uint32_t(*vdencHmeCostTable)[CODEC_AVC_NUM_QP];
    if (avcSeqParams->ScenarioInfo == ESCENARIO_DISPLAYREMOTING)
    {
        vdencHmeCostTable = m_hmeCostDisplayRemote;
    }
    else
    {
        vdencHmeCostTable = m_hmeCost;
    }

    for (uint8_t i = 0; i < 8; i++)
    {
        m_vdEncHmeMvCost[i] = Map44LutValue(*(vdencHmeCostTable[i] + qp), 0x6f);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalVdencAvcStateG11::LoadHmeMvCostTable(PCODEC_AVC_ENCODE_SEQUENCE_PARAMS seqParams, uint8_t hmeMVCostTable[8][42])
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    const uint32_t(*vdencHmeCostTable)[CODEC_AVC_NUM_QP];
    if ((m_avcSeqParam->ScenarioInfo == ESCENARIO_DISPLAYREMOTING) || (m_avcSeqParam->RateControlMethod == RATECONTROL_QVBR))
    {
        vdencHmeCostTable = m_hmeCostDisplayRemote;
    }
    else
    {
        vdencHmeCostTable = m_hmeCost;
    }

    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 42; j++)
        {
            hmeMVCostTable[i][j] = Map44LutValue(*(vdencHmeCostTable[i] + j + 10), 0x6f);
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalVdencAvcStateG11::AddVdencWalkerStateCmd(
    PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    MHW_VDBOX_VDENC_WALKER_STATE_PARAMS_G11 vdencWalkerStateParams;
    auto avcSlcParams = m_avcSliceParams;
    auto avcPicParams = m_avcPicParams[avcSlcParams->pic_parameter_set_id];
    auto avcSeqParams = m_avcSeqParams[avcPicParams->seq_parameter_set_id];

    vdencWalkerStateParams.Mode = CODECHAL_ENCODE_MODE_AVC;
    vdencWalkerStateParams.pAvcSeqParams = avcSeqParams;
    vdencWalkerStateParams.pAvcSlcParams = m_avcSliceParams;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdencWalkerStateCmd(cmdBuffer, &vdencWalkerStateParams));

    return eStatus;
}

MOS_STATUS CodechalVdencAvcStateG11::CalculateVdencCommandsSize()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    MHW_VDBOX_STATE_CMDSIZE_PARAMS_G11 stateCmdSizeParams;
    uint32_t vdencPictureStatesSize, vdencPicturePatchListSize;
    uint32_t vdencSliceStatesSize, vdencSlicePatchListSize;
    m_hwInterface->GetHxxStateCommandSize(
        CODECHAL_ENCODE_MODE_AVC,
        (uint32_t*)&vdencPictureStatesSize,
        (uint32_t*)&vdencPicturePatchListSize,
        &stateCmdSizeParams);

    m_pictureStatesSize += vdencPictureStatesSize;
    m_picturePatchListSize += vdencPicturePatchListSize;

    // Picture Level Commands
    m_hwInterface->GetVdencStateCommandsDataSize(
        CODECHAL_ENCODE_MODE_AVC,
        (uint32_t*)&vdencPictureStatesSize,
        (uint32_t*)&vdencPicturePatchListSize);

    m_pictureStatesSize += vdencPictureStatesSize;
    m_picturePatchListSize += vdencPicturePatchListSize;

    // Slice Level Commands
    m_hwInterface->GetVdencPrimitiveCommandsDataSize(
        CODECHAL_ENCODE_MODE_AVC,
        (uint32_t*)&vdencSliceStatesSize,
        (uint32_t*)&vdencSlicePatchListSize
    );

    m_sliceStatesSize += vdencSliceStatesSize;
    m_slicePatchListSize += vdencSlicePatchListSize;

    return eStatus;
}

MOS_STATUS CodechalVdencAvcStateG11::SendPrologWithFrameTracking(
    PMOS_COMMAND_BUFFER         cmdBuffer,
    bool                        frameTracking,
    MHW_MI_MMIOREGISTERS       *mmioRegister)
{
    if (MOS_VE_SUPPORTED(m_osInterface) && cmdBuffer->Attributes.pAttriVe)
    {
        PMOS_CMD_BUF_ATTRI_VE attriExt =
                (PMOS_CMD_BUF_ATTRI_VE)(cmdBuffer->Attributes.pAttriVe);
        attriExt->bUseVirtualEngineHint = true;
        attriExt->VEngineHintParams.NeedSyncWithPrevious = 1;
    }

    return CodechalVdencAvcState::SendPrologWithFrameTracking(cmdBuffer, frameTracking, mmioRegister);
}

PMHW_VDBOX_STATE_CMDSIZE_PARAMS CodechalVdencAvcStateG11::CreateMhwVdboxStateCmdsizeParams()
{
    PMHW_VDBOX_STATE_CMDSIZE_PARAMS cmdSizeParams = MOS_New(MHW_VDBOX_STATE_CMDSIZE_PARAMS_G11);

    return cmdSizeParams;
}

PMHW_VDBOX_VDENC_WALKER_STATE_PARAMS CodechalVdencAvcStateG11::CreateMhwVdboxVdencWalkerStateParams()
{
    PMHW_VDBOX_VDENC_WALKER_STATE_PARAMS vdencWalkerStateParams = MOS_New(MHW_VDBOX_VDENC_WALKER_STATE_PARAMS_G11);
 
    return vdencWalkerStateParams;
}

MOS_STATUS CodechalVdencAvcStateG11::InitKernelStateMe()
{
    m_hmeKernel = MOS_New(CodechalKernelHmeG11, this);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hmeKernel);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hmeKernel->Initialize(
        GetCommonKernelHeaderAndSizeG11,
        m_kernelBase,
        m_kuidCommon));
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalVdencAvcStateG11::ExecuteMeKernel()
{
    if (m_hmeKernel && m_hmeKernel->Is4xMeEnabled())
    {
        CodechalKernelHme::CurbeParam curbeParam = {};
        curbeParam.subPelMode = 3;
        curbeParam.currOriginalPic = m_avcPicParam->CurrOriginalPic;
        curbeParam.qpPrimeY = m_avcPicParam->pic_init_qp_minus26 + 26 + m_avcSliceParams->slice_qp_delta;
        curbeParam.targetUsage = m_avcSeqParam->TargetUsage;
        curbeParam.maxMvLen = CodecHalAvcEncode_GetMaxMvLen(m_avcSeqParam->Level);
        curbeParam.numRefIdxL0Minus1 = m_avcSliceParams->num_ref_idx_l0_active_minus1;
        curbeParam.numRefIdxL1Minus1 = m_avcSliceParams->num_ref_idx_l1_active_minus1;

        auto slcParams = m_avcSliceParams;
        curbeParam.list0RefID0FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_0);
        curbeParam.list0RefID1FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_1);
        curbeParam.list0RefID2FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_2);
        curbeParam.list0RefID3FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_3);
        curbeParam.list0RefID4FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_4);
        curbeParam.list0RefID5FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_5);
        curbeParam.list0RefID6FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_6);
        curbeParam.list0RefID7FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_7);
        curbeParam.list1RefID0FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_1, CODECHAL_ENCODE_REF_ID_0);
        curbeParam.list1RefID1FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_1, CODECHAL_ENCODE_REF_ID_1);

        CodechalKernelHme::SurfaceParams surfaceParam = {};
        surfaceParam.mbaffEnabled = m_mbaffEnabled;
        surfaceParam.numRefIdxL0ActiveMinus1 = m_avcSliceParams->num_ref_idx_l0_active_minus1;
        surfaceParam.numRefIdxL1ActiveMinus1 = m_avcSliceParams->num_ref_idx_l1_active_minus1;
        surfaceParam.verticalLineStride = m_verticalLineStride;
        surfaceParam.verticalLineStrideOffset = m_verticalLineStrideOffset;
        surfaceParam.refList = &m_refList[0];
        surfaceParam.picIdx = &m_picIdx[0];
        surfaceParam.currOriginalPic = &m_currOriginalPic;
        surfaceParam.refL0List = &(m_avcSliceParams->RefPicList[LIST_0][0]);
        surfaceParam.refL1List = &(m_avcSliceParams->RefPicList[LIST_1][0]);
        surfaceParam.vdencStreamInEnabled = m_vdencEnabled && (m_16xMeSupported || m_staticFrameDetectionInUse);
        surfaceParam.meVdencStreamInBuffer= &m_resVdencStreamInBuffer[m_currRecycledBufIdx];
        surfaceParam.vdencStreamInSurfaceSize = MOS_BYTES_TO_DWORDS(m_picHeightInMb * m_picWidthInMb * 64);

        if (m_hmeKernel->Is16xMeEnabled())
        {
            m_lastTaskInPhase = false;
            if (m_hmeKernel->Is32xMeEnabled())
            {
                surfaceParam.downScaledWidthInMb = m_downscaledWidthInMb32x;
                surfaceParam.downScaledHeightInMb = m_downscaledFrameFieldHeightInMb16x;
                surfaceParam.downScaledBottomFieldOffset = m_scaled32xBottomFieldOffset;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hmeKernel->Execute(curbeParam, surfaceParam, CodechalKernelHme::HmeLevel::hmeLevel32x));
            }
            surfaceParam.downScaledWidthInMb = m_downscaledWidthInMb16x;
            surfaceParam.downScaledHeightInMb = m_downscaledFrameFieldHeightInMb16x;
            surfaceParam.downScaledBottomFieldOffset = m_scaled16xBottomFieldOffset;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hmeKernel->Execute(curbeParam, surfaceParam, CodechalKernelHme::HmeLevel::hmeLevel16x));
        }

        // On-demand sync for VDEnc SHME StreamIn surface
        auto syncParams = g_cInitSyncParams;
        syncParams.GpuContext = m_renderContext;
        syncParams.presSyncResource = &m_resVdencStreamInBuffer[m_currRecycledBufIdx];

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnResourceWait(m_osInterface, &syncParams));
        m_osInterface->pfnSetResourceSyncTag(m_osInterface, &syncParams);

        // HME StreamIn
        m_lastTaskInPhase = !m_staticFrameDetectionInUse;

        surfaceParam.downScaledWidthInMb = m_downscaledWidthInMb4x;
        surfaceParam.downScaledHeightInMb = m_downscaledFrameFieldHeightInMb4x;
        surfaceParam.downScaledBottomFieldOffset = m_scaledBottomFieldOffset;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hmeKernel->Execute(curbeParam, surfaceParam, CodechalKernelHme::HmeLevel::hmeLevel4x));
        m_vdencStreamInEnabled = true;
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalVdencAvcStateG11::UpdateCmdBufAttribute(
    PMOS_COMMAND_BUFFER cmdBuffer,
    bool                renderEngineInUse)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    // should not be there. Will remove it in the next change
    CODECHAL_ENCODE_FUNCTION_ENTER;
    if (MOS_VE_SUPPORTED(m_osInterface) && cmdBuffer->Attributes.pAttriVe)
    {
        PMOS_CMD_BUF_ATTRI_VE attriExt =
            (PMOS_CMD_BUF_ATTRI_VE)(cmdBuffer->Attributes.pAttriVe);

        memset(attriExt, 0, sizeof(MOS_CMD_BUF_ATTRI_VE));
        attriExt->bUseVirtualEngineHint =
            attriExt->VEngineHintParams.NeedSyncWithPrevious = !renderEngineInUse;
    }

    return eStatus;
}

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS CodechalVdencAvcStateG11::PopulateBrcInitParam(
    void *cmd)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    CODECHAL_DEBUG_CHK_NULL(m_debugInterface);

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDumpEncodePar))
    {
        return MOS_STATUS_SUCCESS;
    }

    BrcInitDmem * dmem = (BrcInitDmem *)cmd;

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
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalVdencAvcStateG11::PopulateBrcUpdateParam(
    void *cmd)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    CODECHAL_DEBUG_CHK_NULL(m_debugInterface);

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDumpEncodePar))
    {
        return MOS_STATUS_SUCCESS;
    }

    BrcUpdateDmem * dmem = (BrcUpdateDmem *)cmd;

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

MOS_STATUS CodechalVdencAvcStateG11::PopulateEncParam(
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
        data = data + mhw_vdbox_mfx_g11_X::MFX_AVC_IMG_STATE_CMD::byteSize;
    }
    else
    {
        // CQP case: VDENC IMG STATE is updated by driver or SFD kernel
        if (!m_staticFrameDetectionInUse)
        {
            data = m_batchBufferForVdencImgStat[m_currRecycledBufIdx].pData;
            data = data + mhw_vdbox_mfx_g11_X::MFX_AVC_IMG_STATE_CMD::byteSize;
        }
        else
        {
            data = (uint8_t *)m_osInterface->pfnLockResource(m_osInterface, &m_resVdencSfdImageStateReadBuffer, &lockFlags);
        }
    }

    CODECHAL_DEBUG_CHK_NULL(data);

    mhw_vdbox_vdenc_g11_X::VDENC_IMG_STATE_CMD vdencCmd;
    vdencCmd = *(mhw_vdbox_vdenc_g11_X::VDENC_IMG_STATE_CMD *)(data);

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

MOS_STATUS CodechalVdencAvcStateG11::PopulatePakParam(
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
        data = (uint8_t*)(cmdBuffer->pCmdPtr - (mhw_vdbox_mfx_g11_X::MFX_AVC_IMG_STATE_CMD::byteSize / sizeof(uint32_t)));
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

    mhw_vdbox_mfx_g11_X::MFX_AVC_IMG_STATE_CMD mfxCmd;
    mfxCmd = *(mhw_vdbox_mfx_g11_X::MFX_AVC_IMG_STATE_CMD *)(data);

    if (m_pictureCodingType == I_TYPE)
    {
        m_avcPar->TrellisQuantizationEnable         = mfxCmd.DW5.TrellisQuantizationEnabledTqenb;
        m_avcPar->EnableAdaptiveTrellisQuantization = mfxCmd.DW5.TrellisQuantizationEnabledTqenb;
        m_avcPar->TrellisQuantizationRounding       = mfxCmd.DW5.TrellisQuantizationRoundingTqr;
        m_avcPar->TrellisQuantizationChromaDisable  = mfxCmd.DW5.TrellisQuantizationChromaDisableTqchromadisable;
        m_avcPar->ExtendedRhoDomainEn               = mfxCmd.DW17.ExtendedRhodomainStatisticsEnable;
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
