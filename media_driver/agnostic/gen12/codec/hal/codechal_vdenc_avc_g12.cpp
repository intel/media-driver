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
//! \file     codechal_vdenc_avc_g12.cpp
//! \brief    This file implements the C++ class/interface for Gen12 platform's AVC
//!           VDEnc encoding to be used CODECHAL components.
//!

#include "codechal_vdenc_avc_g12.h"
#include "codechal_mmc_encode_avc_g12.h"
#include "codechal_kernel_header_g12.h"
#include "codechal_kernel_hme_g12.h"
#include "mhw_mi_g12_X.h"
#include "mhw_vdbox_vdenc_g12_X.h"
#include "mhw_vdbox_g12_X.h"
#include "mhw_render_g12_X.h"
#include "codeckrnheader.h"
#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
#include "igcodeckrn_g12.h"
#endif
#if USE_CODECHAL_DEBUG_TOOL
#include "codechal_debug_encode_par_g12.h"
#include "mhw_vdbox_mfx_hwcmd_g12_X.h"
#include "mhw_vdbox_vdenc_hwcmd_g12_X.h"
#include <iomanip>
#endif
#include "hal_oca_interface.h"

#define CODEC_AVC_MIN_BLOCK_HEIGHT 16

enum SfdBindingTableOffset
{
    sfdVdencInputImageState = 0,
    sfdMvDataSurface = 1,
    sfdInterDistortionSurface = 2,
    sfdOutputDataSurface = 3,
    sfdVdencOutputImageState = 4,
    sfdNumSurfaces = 5
};

// clang-format off
// CURBE for Static Frame Detection kernel
class CodechalVdencAvcStateG12::SfdCurbe
{
   public:

    union
    {
        struct
        {
            uint32_t m_vdencModeDisable                     : MOS_BITFIELD_BIT(0);
            uint32_t m_brcModeEnable                        : MOS_BITFIELD_BIT(1);
            uint32_t m_sliceType                            : MOS_BITFIELD_RANGE(2, 3);
            uint32_t                                        : MOS_BITFIELD_BIT(4);
            uint32_t m_streamInType                         : MOS_BITFIELD_RANGE(5, 8);
            uint32_t m_enableAdaptiveMvStreamIn             : MOS_BITFIELD_BIT(9);
            uint32_t                                        : MOS_BITFIELD_BIT(10);
            uint32_t m_enableIntraCostScalingForStaticFrame : MOS_BITFIELD_BIT(11);
            uint32_t m_reserved                             : MOS_BITFIELD_RANGE(12, 31);
        };
        struct
        {
            uint32_t m_value;
        };
    } m_dw0;

    union
    {
        struct
        {
            uint32_t m_qpValue            : MOS_BITFIELD_RANGE(0, 7);
            uint32_t m_numOfRefs          : MOS_BITFIELD_RANGE(8, 15);
            uint32_t m_hmeStreamInRefCost : MOS_BITFIELD_RANGE(16, 23);
            uint32_t m_reserved           : MOS_BITFIELD_RANGE(24, 31);
        };
        struct
        {
            uint32_t m_value;
        };
    } m_dw1;

    union
    {
        struct
        {
            uint32_t m_frameWidthInMBs  : MOS_BITFIELD_RANGE(0, 15);   // round-up to 4-MB aligned
            uint32_t m_frameHeightInMBs : MOS_BITFIELD_RANGE(16, 31);  // round-up to 4-MB aligned
        };
        struct
        {
            uint32_t m_value;
        };
    } m_dw2;

    union
    {
        struct
        {
            uint32_t m_largeMvThresh;
        };
        struct
        {
            uint32_t m_value;
        };
    } m_dw3;

    union
    {
        struct
        {
            uint32_t m_totalLargeMvThreshold;
        };
        struct
        {
            uint32_t m_value;
        };
    } m_dw4;

    union
    {
        struct
        {
            uint32_t m_zMVThreshold;
        };
        struct
        {
            uint32_t m_value;
        };
    } m_dw5;

    union
    {
        struct
        {
            uint32_t m_totalZMVThreshold;
        };
        struct
        {
            uint32_t m_value;
        };
    } m_dw6;

    union
    {
        struct
        {
            uint32_t m_minDistThreshold;
        };
        struct
        {
            uint32_t m_value;
        };
    } m_dw7;

    uint8_t m_costTable[52];

    union
    {
        struct
        {
            uint32_t m_actualWidthInMB  : MOS_BITFIELD_RANGE(0, 15);
            uint32_t m_actualHeightInMB : MOS_BITFIELD_RANGE(16, 31);
        };
        struct
        {
            uint32_t m_value;
        };
    } m_dw21;

    union
    {
        struct
        {
            uint32_t m_reserved;
        };
        struct
        {
            uint32_t m_value;
        };
    } m_dw22;

    union
    {
        struct
        {
            uint32_t m_reserved;
        };
        struct
        {
            uint32_t m_value;
        };
    } m_dw23;

    union
    {
        struct
        {
            uint32_t m_vdencInputImagStateIndex;  // used in VDEnc CQP mode
        };
        struct
        {
            uint32_t m_value;
        };
    } m_dw24;

    union
    {
        struct
        {
            uint32_t m_reserved;
        };
        struct
        {
            uint32_t m_value;
        };
    } m_dw25;

    union
    {
        struct
        {
            uint32_t m_mvDataSurfaceIndex;  // contains HME MV Data generated by HME kernel
        };
        struct
        {
            uint32_t m_value;
        };
    } m_dw26;

    union
    {
        struct
        {
            uint32_t m_interDistortionSurfaceIndex;  // contains HME Inter Distortion generated by HME kernel
        };
        struct
        {
            uint32_t m_value;
        };
    } m_dw27;

    union
    {
        struct
        {
            uint32_t m_outputDataSurfaceIndex;
        };
        struct
        {
            uint32_t m_value;
        };
    } m_dw28;

    union
    {
        ;
        struct
        {
            uint32_t m_vdencOutputImagStateIndex;
        };
        struct
        {
            uint32_t m_value;
        };
    } m_dw29;

    SfdCurbe()
    {
        m_dw0.m_value = 0;
        m_dw1.m_value = 0;
        m_dw2.m_value = 0;
        m_dw3.m_value = 0;
        m_dw4.m_value = 0;
        m_dw5.m_value = 0;
        m_dw6.m_value = 0;
        m_dw7.m_value = 0;
        m_dw21.m_value = 0;
        m_dw22.m_value = 0;
        m_dw23.m_value = 0;
        m_dw24.m_value = 0;
        m_dw25.m_value = 0;
        m_dw26.m_value = 0;
        m_dw27.m_value = 0;
        m_dw28.m_value = 0;
        m_dw29.m_value = 0;

        for (uint8_t i = 0; i < 52; i++)
            m_costTable[i] = 0;
    }
};
// clang-format on

struct CodechalVdencAvcStateG12::BrcInitDmem
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

struct CodechalVdencAvcStateG12::BrcUpdateDmem
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
    uint8_t      UPD_EnableFineGrainLA;
    int8_t       UPD_DeltaQpDcOffset;
    uint16_t     UPD_NumSlicesForRounding;
    uint32_t     UPD_UserMaxFramePB;        // In Bytes
    uint8_t      RSVD2[4];
};

// clang-format off
const uint32_t CodechalVdencAvcStateG12::m_mvCostSkipBiasQPel[3][8] =
{
    // for normal case
    { 0, 6, 6, 9, 10, 13, 14, 16 },
    // for QP = 47,48,49
    { 0, 6, 6, 6, 6, 7, 8, 8 },
    // for QP = 50,51
    { 0, 6, 6, 6, 6, 7, 7, 7 }
};

const uint32_t CodechalVdencAvcStateG12::m_hmeCostDisplayRemote[8][CODEC_AVC_NUM_QP] =
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

const uint32_t CodechalVdencAvcStateG12::m_hmeCost[8][CODEC_AVC_NUM_QP] =
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

const int8_t CodechalVdencAvcStateG12::m_brcInitDistQpDeltaI8[4] =
{
    0, 0, 0, 0
};

const int8_t CodechalVdencAvcStateG12::m_brcInitDistQpDeltaI8LowDelay[4] =
{
    -5, -2, 2, 5
};
// clang-format on

CodechalVdencAvcStateG12::CodechalVdencAvcStateG12(
        CodechalHwInterface *   hwInterface,
        CodechalDebugInterface *debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo) : CodechalVdencAvcState(hwInterface, debugInterface, standardInfo), m_sinlgePipeVeState(nullptr)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_NO_STATUS_RETURN(m_osInterface);

    // Virtual Engine is enabled in default.
    Mos_SetVirtualEngineSupported(m_osInterface, true);

    m_osInterface->pfnVirtualEngineSupported(m_osInterface, false, true);

#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
    m_kernelBase = (uint8_t*)IGCODECKRN_G12;
#endif
    m_kuidCommon = IDR_CODEC_HME_DS_SCOREBOARD_KERNEL;
    AddIshSize(m_kuidCommon, m_kernelBase);

    m_cmKernelEnable   = true;
    m_mbStatsSupported = true;

    pfnGetKernelHeaderAndSize    = nullptr;

    m_vdencBrcInitDmemBufferSize   = sizeof(BrcInitDmem);
    m_vdencBrcUpdateDmemBufferSize = sizeof(BrcUpdateDmem);
    m_vdencBrcNumOfSliceOffset     = (m_waTable && MEDIA_IS_WA(m_waTable, Wa_22010554215)) ? 0 : CODECHAL_OFFSETOF(BrcUpdateDmem, NumOfSlice);

    // One Gen12, avc vdenc ref index need to be one on one mapping
    m_oneOnOneMapping = true;

    m_vdboxOneDefaultUsed = true;
    m_nonNativeBrcRoiSupported = true;
    m_brcAdaptiveRegionBoostSupported = true;

    m_hmeSupported   = true;
    m_16xMeSupported = true;
    m_32xMeSupported = true;

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_NULL_NO_STATUS_RETURN(m_encodeParState = MOS_New(CodechalDebugEncodeParG12, this));
        CreateAvcPar();
    )
}

CodechalVdencAvcStateG12::~CodechalVdencAvcStateG12()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (m_sinlgePipeVeState)
    {
        MOS_FreeMemAndSetNull(m_sinlgePipeVeState);
    }
    MOS_SafeFreeMemory(m_pMBQPShadowBuffer);

    if (!m_swBrcMode && m_singleTaskPhaseSupported)
    {
        m_osInterface->pfnFreeResource(m_osInterface, &m_resPakOutputViaMmioBuffer);
    }

    CODECHAL_DEBUG_TOOL(
        DestroyAvcPar();
        MOS_Delete(m_encodeParState);
    )
}

void CodechalVdencAvcStateG12::InitializeDataMember()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;
    CodechalVdencAvcState::InitializeDataMember();
    if (!m_swBrcMode && m_singleTaskPhaseSupported)
    {
        MOS_ZeroMemory(&m_resPakOutputViaMmioBuffer, sizeof(MOS_RESOURCE));
    }
}

MOS_STATUS CodechalVdencAvcStateG12::InitializeState()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalVdencAvcState::InitializeState());

    m_sliceSizeStreamoutSupported = MEDIA_IS_WA(m_waTable, Wa_22010554215) ? false : true;
    m_useHwScoreboard        = false;
    m_useCommonKernel        = true;

    if (MOS_VE_SUPPORTED(m_osInterface))
    {
        m_sinlgePipeVeState = (PCODECHAL_ENCODE_SINGLEPIPE_VIRTUALENGINE_STATE)MOS_AllocAndZeroMemory(sizeof(CODECHAL_ENCODE_SINGLEPIPE_VIRTUALENGINE_STATE));
        CODECHAL_ENCODE_CHK_NULL_RETURN(m_sinlgePipeVeState);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalEncodeSinglePipeVE_InitInterface(m_hwInterface, m_sinlgePipeVeState));
    }

    return eStatus;
}

MOS_STATUS CodechalVdencAvcStateG12::AllocateResources()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalVdencAvcState::AllocateResources());

    if (!m_swBrcMode && m_singleTaskPhaseSupported)
    {
        // Initiate allocation parameters and lock flags
        MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
        MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
        allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
        allocParamsForBufferLinear.Format = Format_Buffer;

        MOS_LOCK_PARAMS lockFlagsWriteOnly;
        MOS_ZeroMemory(&lockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
        lockFlagsWriteOnly.WriteOnly = 1;

        // PAK statistics buffer
        allocParamsForBufferLinear.dwBytes = CODECHAL_PAGE_SIZE;
        allocParamsForBufferLinear.pBufName = "VDENC PAK Statistics MMIO Registers Output Buffer";

        CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBufferLinear,
            &m_resPakOutputViaMmioBuffer),
            "%s: Failed to allocate '%s'\n",
            __FUNCTION__,
            allocParamsForBufferLinear.pBufName);

        uint8_t* data = (uint8_t*)m_osInterface->pfnLockResource(
            m_osInterface,
            &(m_resPakOutputViaMmioBuffer),
            &lockFlagsWriteOnly);

        if (data == nullptr)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Failed to Lock '%s'", allocParamsForBufferLinear.pBufName);
            return MOS_STATUS_UNKNOWN;
        }

        MOS_ZeroMemory(data, allocParamsForBufferLinear.dwBytes);
        m_osInterface->pfnUnlockResource(m_osInterface, &m_resPakOutputViaMmioBuffer);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalVdencAvcStateG12::SetSequenceStructs()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    m_forcedTCBRC = false;
    // For g12+ tcbrc is used instead of LowDelayBRC,
    // also needs TargetFrameSize in PPS.
    if (m_avcSeqParam->FrameSizeTolerance == EFRAMESIZETOL_EXTREMELY_LOW && !m_avcSeqParam->LookaheadDepth)
    {
        CODECHAL_ENCODE_NORMALMESSAGE("LDBRC switched to TCBRC\n");
        m_forcedTCBRC = true;
        m_avcSeqParam->FrameSizeTolerance = EFRAMESIZETOL_NORMAL;
        m_avcSeqParam->MBBRC              = mbBrcDisabled; // no need with ARB
    }

    return CodechalVdencAvcState::SetSequenceStructs();
}

MOS_STATUS CodechalVdencAvcStateG12::SetPictureStructs()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    // TCBRC forced from LowDelayBRC also needs TargetFrameSize
    if (m_forcedTCBRC)
    {
        if (m_avcPicParam->NumDirtyROI || m_avcPicParam->NumROI)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("ROI/DirtyROI disabled for TCBRC\n");
            m_avcPicParam->NumDirtyROI = m_avcPicParam->NumROI = 0;
        }
        if (m_avcSeqParam->FramesPer100Sec == 0)
        {
            return MOS_STATUS_INVALID_PARAMETER;
        }
        m_avcPicParam->TargetFrameSize = uint32_t(m_avcSeqParam->TargetBitRate * (100. / 8) / m_avcSeqParam->FramesPer100Sec);
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalVdencAvcState::SetPictureStructs());

    if (m_encodeParams.bMbQpDataEnabled)
    {
        if (m_avcPicParam->NumDirtyROI || m_avcPicParam->NumROI)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("MBQP feature is not compatible with ROI/DirtyROI\n");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetupMBQPStreamIn(
            &(m_resVdencStreamInBuffer[m_currRecycledBufIdx])));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalVdencAvcStateG12::ExecutePictureLevel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

#if MHW_HWCMDPARSER_ENABLED
    char frameType = '\0';
    switch (m_avcPicParam->CodingType)
    {
    case I_TYPE:
        frameType = 'I';
        break;
    case P_TYPE:
        frameType = 'P';
        break;
    case B_TYPE:
        frameType = m_avcPicParam->RefPicFlag ? 'B' : 'b';
        break;
    }

    auto instance = mhw::HwcmdParser::GetInstance();
    if (instance)
    {
        instance->Update(frameType, nullptr);
    }
#endif

    MHW_BATCH_BUFFER batchBuffer;
    MOS_ZeroMemory(&batchBuffer, sizeof(batchBuffer));
    batchBuffer.dwOffset     = m_currPass * BRC_IMG_STATE_SIZE_PER_PASS;
    batchBuffer.bSecondLevel = true;

    CODECHAL_ENCODE_AVC_GENERIC_PICTURE_LEVEL_PARAMS encodePictureLevelParams;
    MOS_ZeroMemory(&encodePictureLevelParams, sizeof(encodePictureLevelParams));
    encodePictureLevelParams.psPreDeblockSurface  = &m_reconSurface;
    encodePictureLevelParams.psPostDeblockSurface = &m_reconSurface;
    encodePictureLevelParams.bBrcEnabled          = false;
    encodePictureLevelParams.pImgStateBatchBuffer = &batchBuffer;

    bool suppressReconPic =
        ((!m_refList[m_currReconstructedPic.FrameIdx]->bUsedAsRef) && m_suppressReconPicSupported);
    encodePictureLevelParams.bDeblockerStreamOutEnable = 0;
    encodePictureLevelParams.bPreDeblockOutEnable      = !m_deblockingEnabled && !suppressReconPic;
    encodePictureLevelParams.bPostDeblockOutEnable     = m_deblockingEnabled && !suppressReconPic;
    encodePictureLevelParams.bPerMBStreamOutEnable     = m_perMBStreamOutEnable;
    if (!m_staticFrameDetectionInUse)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(LoadCosts(m_avcPicParam->CodingType,
            m_avcPicParam->QpY + m_avcSliceParams->slice_qp_delta));

        m_vdencHmeMvCostTbl = m_vdEncHmeMvCost;
        m_vdencModeCostTbl  = m_vdEncModeCost;
        m_vdencMvCostTbl    = m_vdEncMvCost;
    }

    // VDEnc HuC BRC
    if (m_vdencBrcEnabled)
    {
        PerfTagSetting perfTag;
        perfTag.Value             = 0;
        perfTag.Mode              = (uint16_t)m_mode & CODECHAL_ENCODE_MODE_BIT_MASK;
        perfTag.CallType          = CODECHAL_ENCODE_PERFTAG_CALL_BRC_INIT_RESET;
        perfTag.PictureCodingType = m_pictureCodingType;
        m_osInterface->pfnSetPerfTag(m_osInterface, perfTag.Value);

        SetBufferToStorePakStatistics();

        // Invoke BRC init/reset FW
        if (m_brcInit || m_brcReset)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(HuCBrcInitReset());
        }

        perfTag.CallType = m_currPass == 0 ? CODECHAL_ENCODE_PERFTAG_CALL_BRC_UPDATE : CODECHAL_ENCODE_PERFTAG_CALL_BRC_UPDATE_SECOND_PASS;
        m_osInterface->pfnSetPerfTag(m_osInterface, perfTag.Value);

        // Invoke BRC update FW
        CODECHAL_ENCODE_CHK_STATUS_RETURN(HuCBrcUpdate());
        m_brcInit = m_brcReset = false;
    }

    PerfTagSetting perfTag;
    perfTag.Value             = 0;
    perfTag.Mode              = (uint16_t)m_mode & CODECHAL_ENCODE_MODE_BIT_MASK;
    perfTag.CallType          = m_currPass == 0 ? CODECHAL_ENCODE_PERFTAG_CALL_PAK_ENGINE : CODECHAL_ENCODE_PERFTAG_CALL_PAK_ENGINE_SECOND_PASS;
    perfTag.PictureCodingType = m_pictureCodingType;
    m_osInterface->pfnSetPerfTag(m_osInterface, perfTag.Value);

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    // PAK cmd buffer header insertion for 1) non STF 2) STF (except VDEnc BRC case inserted in HuC cmd buffer)
    if (!m_singleTaskPhaseSupported || (m_firstTaskInPhase && (!m_vdencBrcEnabled)))
    {
        bool requestFrameTracking = false;

        m_hwInterface->m_numRequestedEuSlices = ((m_frameHeight * m_frameWidth) >= m_ssdResolutionThreshold &&
                                                    m_targetUsage <= m_ssdTargetUsageThreshold)
                                                    ? m_sliceShutdownRequestState
                                                    : m_sliceShutdownDefaultState;

        MHW_MI_MMIOREGISTERS mmioRegister;
        bool validMmio = m_mfxInterface->ConvertToMiRegister(m_vdboxIndex, mmioRegister);

        // Send command buffer header at the beginning (OS dependent)
        // frame tracking tag is only added in the last command buffer header
        requestFrameTracking = m_singleTaskPhaseSupported ? m_firstTaskInPhase : m_lastTaskInPhase;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SendPrologWithFrameTracking(&cmdBuffer, requestFrameTracking, validMmio ? &mmioRegister : nullptr));

        m_hwInterface->m_numRequestedEuSlices = CODECHAL_SLICE_SHUTDOWN_DEFAULT;
    }

    // Set TBL distribution to VMC = 240 for VDEnc performance
    if (MEDIA_IS_WA(m_waTable, WaTlbAllocationForAvcVdenc) &&
        (!m_singleTaskPhaseSupported || !m_currPass))
    {
        TLBAllocationParams tlbAllocationParams;
        tlbAllocationParams.presTlbMmioBuffer     = &m_vdencTlbMmioBuffer;
        tlbAllocationParams.dwMmioMfxLra0Override = m_mmioMfxLra0Override;
        tlbAllocationParams.dwMmioMfxLra1Override = m_mmioMfxLra1Override;
        tlbAllocationParams.dwMmioMfxLra2Override = m_mmioMfxLra2Override;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetTLBAllocation(&cmdBuffer, &tlbAllocationParams));
    }

    MHW_MI_CONDITIONAL_BATCH_BUFFER_END_PARAMS miConditionalBatchBufferEndParams;
    if (m_vdencBrcEnabled && !m_swBrcMode)
    {
        // Insert conditional batch buffer end for HuC valid IMEM loaded check
        MOS_ZeroMemory(&miConditionalBatchBufferEndParams, sizeof(MHW_MI_CONDITIONAL_BATCH_BUFFER_END_PARAMS));
        miConditionalBatchBufferEndParams.presSemaphoreBuffer = &m_resHucStatus2Buffer;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(
            m_miInterface->AddMiConditionalBatchBufferEndCmd(
                &cmdBuffer,
                &miConditionalBatchBufferEndParams));
    }

    if (m_currPass)
    {
        if (m_inlineEncodeStatusUpdate && m_vdencBrcEnabled)
        {
            // inc dwStoreData conditionaly
            UpdateEncodeStatus(&cmdBuffer, false);
        }

        // Insert conditional batch buffer end
        MOS_ZeroMemory(&miConditionalBatchBufferEndParams, sizeof(MHW_MI_CONDITIONAL_BATCH_BUFFER_END_PARAMS));

        if (!m_vdencBrcEnabled)
        {
            miConditionalBatchBufferEndParams.presSemaphoreBuffer = &m_encodeStatusBuf.resStatusBuffer;
            miConditionalBatchBufferEndParams.dwOffset =
                (m_encodeStatusBuf.wCurrIndex * m_encodeStatusBuf.dwReportSize) +
                m_encodeStatusBuf.dwImageStatusMaskOffset + (sizeof(uint32_t) * 2);
        }
        else
        {
            // VDENC uses HuC BRC FW generated semaphore for conditional 2nd pass
            miConditionalBatchBufferEndParams.presSemaphoreBuffer = &m_resPakMmioBuffer;
        }
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiConditionalBatchBufferEndCmd(
            &cmdBuffer,
            &miConditionalBatchBufferEndParams));
    }

    if (!m_currPass && m_osInterface->bTagResourceSync)
    {
        // This is a short term solution to solve the sync tag issue: the sync tag write for PAK is inserted at the end of 2nd pass PAK BB
        // which may be skipped in multi-pass PAK enabled case. The idea here is to insert the previous frame's tag at the beginning
        // of the BB and keep the current frame's tag at the end of the BB. There will be a delay for tag update but it should be fine
        // as long as Dec/VP/Enc won't depend on this PAK so soon.
        PMOS_RESOURCE globalGpuContextSyncTagBuffer = nullptr;
        CODECHAL_HW_CHK_STATUS_RETURN(m_osInterface->pfnGetGpuStatusBufferResource(
            m_osInterface,
            globalGpuContextSyncTagBuffer));
        CODECHAL_ENCODE_CHK_NULL_RETURN(globalGpuContextSyncTagBuffer);

        uint32_t                 value = m_osInterface->pfnGetGpuStatusTag(m_osInterface, m_osInterface->CurrentGpuContextOrdinal);
        MHW_MI_STORE_DATA_PARAMS params;
        params.pOsResource      = globalGpuContextSyncTagBuffer;
        params.dwResourceOffset = m_osInterface->pfnGetGpuStatusTagOffset(m_osInterface, m_osInterface->CurrentGpuContextOrdinal);
        params.dwValue          = (value > 0) ? (value - 1) : 0;
        CODECHAL_HW_CHK_STATUS_RETURN(m_miInterface->AddMiStoreDataImmCmd(&cmdBuffer, &params));
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(StartStatusReport(&cmdBuffer, CODECHAL_NUM_MEDIA_STATES));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdencControlStateCmd(&cmdBuffer));

    // set MFX_SURFACE_STATE values
    // Ref surface
    MHW_VDBOX_SURFACE_PARAMS reconSurfaceParams;
    MOS_ZeroMemory(&reconSurfaceParams, sizeof(reconSurfaceParams));
    reconSurfaceParams.Mode             = m_mode;
    reconSurfaceParams.ucSurfaceStateId = CODECHAL_MFX_REF_SURFACE_ID;
    reconSurfaceParams.psSurface        = &m_reconSurface;
    CODECHAL_DEBUG_TOOL(m_debugInterface->DumpSurfaceInfo(&m_reconSurface, "ReconSurface"));

    // Src surface
    MHW_VDBOX_SURFACE_PARAMS surfaceParams;
    MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
    surfaceParams.Mode                  = m_mode;
    surfaceParams.ucSurfaceStateId      = CODECHAL_MFX_SRC_SURFACE_ID;
    surfaceParams.psSurface             = m_rawSurfaceToPak;
    surfaceParams.dwActualHeight        = m_avcSeqParam->FrameHeight;
    surfaceParams.dwActualWidth         = m_avcSeqParam->FrameWidth;
    surfaceParams.bDisplayFormatSwizzle = m_avcPicParam->bDisplayFormatSwizzle;
    surfaceParams.bColorSpaceSelection  = (m_avcSeqParam->InputColorSpace == ECOLORSPACE_P709) ? 1 : 0;
    CODECHAL_DEBUG_TOOL(m_debugInterface->DumpSurfaceInfo(m_rawSurfaceToPak, "RawSurfaceToPak"));

    MHW_VDBOX_PIPE_BUF_ADDR_PARAMS pipeBufAddrParams;
    pipeBufAddrParams.pRawSurfParam      = &surfaceParams;
    pipeBufAddrParams.pDecodedReconParam = &reconSurfaceParams;
    SetMfxPipeBufAddrStateParams(encodePictureLevelParams, pipeBufAddrParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_mmcState);
    m_mmcState->SetPipeBufAddr(&pipeBufAddrParams, &cmdBuffer);

    PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS pipeModeSelectParams = m_vdencInterface->CreateMhwVdboxPipeModeSelectParams();
    CODECHAL_ENCODE_CHK_NULL_RETURN(pipeModeSelectParams);

    //add fill_pad_with_value
    if (MEDIA_IS_WA(m_waTable, Wa_AvcUnalignedHeight))
    {
        if (m_avcSeqParam->frame_cropping_flag)
        {
            m_frame_crop_bottom_offset = m_avcSeqParam->frame_crop_bottom_offset;
            m_frame_mbs_only_flag      = m_avcSeqParam->frame_mbs_only_flag;
            uint32_t crop_unit_y    = 2 * (2 - m_frame_mbs_only_flag);
            uint32_t real_height    = m_oriFrameHeight - (m_frame_crop_bottom_offset * crop_unit_y);
            uint32_t aligned_height = MOS_ALIGN_CEIL(real_height, CODEC_AVC_MIN_BLOCK_HEIGHT);
            fill_pad_with_value(m_rawSurfaceToPak, real_height, aligned_height);
        }
    }

    auto release_func = [&]()
    {
        m_vdencInterface->ReleaseMhwVdboxPipeModeSelectParams(pipeModeSelectParams);
        pipeModeSelectParams = nullptr;
    };

    SetMfxPipeModeSelectParams(encodePictureLevelParams, *pipeModeSelectParams);
    CODECHAL_ENCODE_CHK_STATUS_WITH_DESTROY_RETURN(m_mfxInterface->AddMfxPipeModeSelectCmd(&cmdBuffer, pipeModeSelectParams), release_func);

    CODECHAL_ENCODE_CHK_STATUS_WITH_DESTROY_RETURN(m_mfxInterface->AddMfxSurfaceCmd(&cmdBuffer, &reconSurfaceParams), release_func);

    CODECHAL_ENCODE_CHK_STATUS_WITH_DESTROY_RETURN(m_mfxInterface->AddMfxSurfaceCmd(&cmdBuffer, &surfaceParams), release_func);

    // 4xDS surface
    MHW_VDBOX_SURFACE_PARAMS dsSurfaceParams;
    MOS_ZeroMemory(&dsSurfaceParams, sizeof(dsSurfaceParams));
    dsSurfaceParams.Mode             = m_mode;
    dsSurfaceParams.ucSurfaceStateId = CODECHAL_MFX_DSRECON_SURFACE_ID;
    dsSurfaceParams.psSurface        = m_trackedBuf->Get4xDsReconSurface(CODEC_CURR_TRACKED_BUFFER);
    CODECHAL_DEBUG_TOOL(m_debugInterface->DumpSurfaceInfo(dsSurfaceParams.psSurface, "4xDsReconSurface"));
    CODECHAL_ENCODE_CHK_STATUS_WITH_DESTROY_RETURN(m_mfxInterface->AddMfxSurfaceCmd(&cmdBuffer, &dsSurfaceParams), release_func);
    CODECHAL_ENCODE_CHK_STATUS_WITH_DESTROY_RETURN(m_mfxInterface->AddMfxPipeBufAddrCmd(&cmdBuffer, &pipeBufAddrParams), release_func);

    MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS indObjBaseAddrParams;
    SetMfxIndObjBaseAddrStateParams(indObjBaseAddrParams);
    CODECHAL_ENCODE_CHK_STATUS_WITH_DESTROY_RETURN(m_mfxInterface->AddMfxIndObjBaseAddrCmd(&cmdBuffer, &indObjBaseAddrParams), release_func);

    MHW_VDBOX_BSP_BUF_BASE_ADDR_PARAMS bspBufBaseAddrParams;
    SetMfxBspBufBaseAddrStateParams(bspBufBaseAddrParams);
    CODECHAL_ENCODE_CHK_STATUS_WITH_DESTROY_RETURN(m_mfxInterface->AddMfxBspBufBaseAddrCmd(&cmdBuffer, &bspBufBaseAddrParams), release_func);

    if (m_avcPicParam->StatusReportEnable.fields.FrameStats)
    {
        pipeModeSelectParams->bFrameStatisticsStreamOutEnable = true;
    }
    CODECHAL_ENCODE_CHK_STATUS_WITH_DESTROY_RETURN(m_vdencInterface->AddVdencPipeModeSelectCmd(&cmdBuffer, pipeModeSelectParams), release_func);
    m_vdencInterface->ReleaseMhwVdboxPipeModeSelectParams(pipeModeSelectParams);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdencSrcSurfaceStateCmd(&cmdBuffer, &surfaceParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdencRefSurfaceStateCmd(&cmdBuffer, &reconSurfaceParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdencDsRefSurfaceStateCmd(&cmdBuffer, &dsSurfaceParams, 1));

    // PerfMode is enabled only on BXT, KBL+, replace all 4x Ds refs with the 1st L0 ref
    if (m_vdencInterface->IsPerfModeSupported() && m_perfModeEnabled[m_avcSeqParam->TargetUsage] &&
        pipeBufAddrParams.dwNumRefIdxL0ActiveMinus1 == 0)
    {
        pipeBufAddrParams.dwNumRefIdxL0ActiveMinus1 = 1;
        pipeBufAddrParams.presVdencReferences[1]    = nullptr;
        pipeBufAddrParams.presVdenc4xDsSurface[1]   = pipeBufAddrParams.presVdenc4xDsSurface[0];
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdencPipeBufAddrCmd(&cmdBuffer, &pipeBufAddrParams));

    MHW_VDBOX_VDENC_CQPT_STATE_PARAMS vdencCQPTStateParams;
    SetVdencCqptStateParams(vdencCQPTStateParams);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdencConstQPStateCmd(&cmdBuffer, &vdencCQPTStateParams));

    if (encodePictureLevelParams.bBrcEnabled && m_avcSeqParam->RateControlMethod != RATECONTROL_ICQ)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferStartCmd(
            &cmdBuffer,
            encodePictureLevelParams.pImgStateBatchBuffer));
    }
    else
    {
        //Set MFX_AVC_IMG_STATE command
        PMHW_VDBOX_AVC_IMG_PARAMS imageStateParams = CreateMhwVdboxAvcImgParams();
        CODECHAL_ENCODE_CHK_NULL_RETURN(imageStateParams);
        SetMfxAvcImgStateParams(*imageStateParams);

        PMHW_BATCH_BUFFER secondLevelBatchBufferUsed = nullptr;

        // VDENC CQP case
        if (!m_vdencBrcEnabled)
        {
            // VDENC case uses multiple buffers for concurrency between SFD and VDENC
            secondLevelBatchBufferUsed = &(m_batchBufferForVdencImgStat[m_currRecycledBufIdx]);

            if (!m_staticFrameDetectionInUse)
            {
                // CQP case, driver programs the 2nd Level BB
                CODECHAL_ENCODE_CHK_STATUS_RETURN(Mhw_LockBb(m_osInterface, secondLevelBatchBufferUsed));

                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxAvcImgCmd(nullptr, secondLevelBatchBufferUsed, imageStateParams));

                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdencAvcCostStateCmd(nullptr, secondLevelBatchBufferUsed, imageStateParams));

                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdencCmd3Cmd(nullptr, secondLevelBatchBufferUsed, imageStateParams));

                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdencImgStateCmd(nullptr, secondLevelBatchBufferUsed, imageStateParams));

                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(nullptr, secondLevelBatchBufferUsed));

            #if MHW_HWCMDPARSER_ENABLED
                auto instance = mhw::HwcmdParser::GetInstance();
                if (instance)
                {
                    instance->ParseCmdBuf(IGFX_UNKNOWN, (uint32_t *)(secondLevelBatchBufferUsed->pData),
                        secondLevelBatchBufferUsed->iCurrent / sizeof(uint32_t));
                }
            #endif

                CODECHAL_DEBUG_TOOL(
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(PopulatePakParam(
                        nullptr,
                        secondLevelBatchBufferUsed));

                    CODECHAL_ENCODE_CHK_STATUS_RETURN(PopulateEncParam(
                        0,
                        nullptr));

                    CODECHAL_ENCODE_CHK_STATUS_RETURN(DumpEncodeImgStats(nullptr));)

                CODECHAL_ENCODE_CHK_STATUS_RETURN(Mhw_UnlockBb(m_osInterface, secondLevelBatchBufferUsed, true));
            }
            else
            {
                // SFD enabled, SFD kernel updates VDENC IMG STATE
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxAvcImgCmd(&cmdBuffer, nullptr, imageStateParams));
//#if (_DEBUG || _RELEASE_INTERNAL)
                //secondLevelBatchBufferUsed->iLastCurrent = CODECHAL_ENCODE_VDENC_IMG_STATE_CMD_SIZE + CODECHAL_ENCODE_MI_BATCH_BUFFER_END_CMD_SIZE;
//#endif
                CODECHAL_DEBUG_TOOL(
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(PopulatePakParam(
                        &cmdBuffer,
                        nullptr));
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(DumpEncodeImgStats(&cmdBuffer));)
            }
        }
        else
        {
            // current location to add cmds in 2nd level batch buffer
            m_batchBufferForVdencImgStat[0].iCurrent = 0;
            // reset starting location (offset) executing 2nd level batch buffer for each frame & each pass
            m_batchBufferForVdencImgStat[0].dwOffset = 0;
            secondLevelBatchBufferUsed = &(m_batchBufferForVdencImgStat[0]);
        }
        MOS_Delete(imageStateParams);

        HalOcaInterface::OnSubLevelBBStart(cmdBuffer, (MOS_CONTEXT_HANDLE)m_osInterface->pOsContext, &secondLevelBatchBufferUsed->OsResource, 0, true, 0);

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferStartCmd(&cmdBuffer, secondLevelBatchBufferUsed));

        CODECHAL_DEBUG_TOOL(
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->Dump2ndLvlBatch(
                secondLevelBatchBufferUsed,
                CODECHAL_MEDIA_STATE_ENC_NORMAL,
                nullptr));)
    }

    MHW_VDBOX_QM_PARAMS qmParams;
    MHW_VDBOX_QM_PARAMS fqmParams;
    SetMfxQmStateParams(qmParams, fqmParams);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxQmCmd(&cmdBuffer, &qmParams));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxFqmCmd(&cmdBuffer, &fqmParams));

    if (m_pictureCodingType == B_TYPE)
    {
        // Add AVC Direct Mode command
        MHW_VDBOX_AVC_DIRECTMODE_PARAMS directmodeParams;
        MOS_ZeroMemory(&directmodeParams, sizeof(directmodeParams));
        directmodeParams.CurrPic = m_avcPicParam->CurrReconstructedPic;
        directmodeParams.isEncode = true;
        directmodeParams.uiUsedForReferenceFlags = 0xFFFFFFFF;
        directmodeParams.pAvcPicIdx = &(m_picIdx[0]);
        directmodeParams.avcRefList = (void**)m_refList;
        directmodeParams.bPicIdRemappingInUse = false;
        directmodeParams.bDisableDmvBuffers = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxAvcDirectmodeCmd(&cmdBuffer, &directmodeParams));
    }

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    return eStatus;
}

MOS_STATUS CodechalVdencAvcStateG12::SetAndPopulateVEHintParams(
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

MOS_STATUS CodechalVdencAvcStateG12::SetupMBQPStreamIn(
    PMOS_RESOURCE vdencStreamIn)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(vdencStreamIn);

    m_vdencStreamInEnabled = true;

    MOS_LOCK_PARAMS lockFlags;
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.WriteOnly = true;

    auto pData = (CODECHAL_VDENC_STREAMIN_STATE*)m_osInterface->pfnLockResource(
        m_osInterface,
        vdencStreamIn,
        &lockFlags);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pData);

    MOS_ZeroMemory(pData, m_picHeightInMb * m_picWidthInMb * CODECHAL_CACHELINE_SIZE);

    MOS_LOCK_PARAMS lockFlagsReadOnly;
    MOS_ZeroMemory(&lockFlagsReadOnly, sizeof(MOS_LOCK_PARAMS));
    lockFlagsReadOnly.ReadOnly = true;

    auto pMBQPBuffer = (uint8_t*)m_osInterface->pfnLockResource(
        m_osInterface,
        &(m_encodeParams.psMbQpDataSurface->OsResource),
        &lockFlagsReadOnly);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pMBQPBuffer);

    uint32_t uiSize = (uint32_t)m_encodeParams.psMbQpDataSurface->OsResource.pGmmResInfo->GetSizeSurface();
    uint32_t uiAlign = 64;
    if (uiSize + uiAlign > m_uiMBQPShadowBufferSize)
    {
        m_uiMBQPShadowBufferSize = uiSize + uiAlign;
        m_pMBQPShadowBuffer = (uint8_t*)MOS_ReallocMemory(m_pMBQPShadowBuffer, m_uiMBQPShadowBufferSize);
    }
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_pMBQPShadowBuffer);

    auto pMBQPShadowBufferBase = (uint8_t*)((((uint64_t)(m_pMBQPShadowBuffer) + uiAlign - 1) / uiAlign) * uiAlign);
    MOS_SecureMemcpy(pMBQPShadowBufferBase, uiSize, pMBQPBuffer, uiSize);

    CopyMBQPDataToStreamIn(pData, pMBQPShadowBufferBase);

    m_osInterface->pfnUnlockResource(
        m_osInterface,
        vdencStreamIn);
    m_osInterface->pfnUnlockResource(
        m_osInterface,
        &m_encodeParams.psMbQpDataSurface->OsResource);

    return eStatus;
}

MOS_STATUS CodechalVdencAvcStateG12::SetGpuCtxCreatOption()
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

MOS_STATUS CodechalVdencAvcStateG12::UserFeatureKeyReport()
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

void CodechalVdencAvcStateG12::SetBufferToStorePakStatistics()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (!m_swBrcMode && m_singleTaskPhaseSupported)
    {
        // Store PAK statistics after encode Frame_N into separate internal buffer to get rid of
        // dependency with the DMEM buffer for Frame_N+1
        //
        // This data will be copied into DMEM for Frame_N+1 at the start of CMD buffer for Frame_N+1
        // using MI_COPY_MEM_MEM cmd
        m_resVdencBrcUpdateDmemBufferPtr[0] = &m_resPakOutputViaMmioBuffer;
        m_resVdencBrcUpdateDmemBufferPtr[1] = nullptr;
    }
    else
    {
        CodechalVdencAvcState::SetBufferToStorePakStatistics();
    }
}

MOS_STATUS CodechalVdencAvcStateG12::AddMiStoreForHWOutputToHucDmem(PMOS_COMMAND_BUFFER cmdBuffer)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (!m_swBrcMode && m_singleTaskPhaseSupported)
    {
        // Copy PAK statistics data from internal buffer to DMEM
        MHW_MI_COPY_MEM_MEM_PARAMS copyMemMemParams = {};
        copyMemMemParams.presSrc = &m_resPakOutputViaMmioBuffer;
        copyMemMemParams.presDst = &(m_resVdencBrcUpdateDmemBuffer[m_currRecycledBufIdx][m_currPass]);

        copyMemMemParams.dwSrcOffset = copyMemMemParams.dwDstOffset = CODECHAL_OFFSETOF(BrcUpdateDmem, FrameByteCount);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiCopyMemMemCmd(
            cmdBuffer,
            &copyMemMemParams));

        copyMemMemParams.dwSrcOffset = copyMemMemParams.dwDstOffset = CODECHAL_OFFSETOF(BrcUpdateDmem, ImgStatusCtrl);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiCopyMemMemCmd(
            cmdBuffer,
            &copyMemMemParams));

        copyMemMemParams.dwSrcOffset = copyMemMemParams.dwDstOffset = m_vdencBrcNumOfSliceOffset;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiCopyMemMemCmd(
            cmdBuffer,
            &copyMemMemParams));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalVdencAvcStateG12::SubmitCommandBuffer(
    PMOS_COMMAND_BUFFER cmdBuffer,
    bool             bNullRendering)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);

    HalOcaInterface::On1stLevelBBEnd(*cmdBuffer, *m_osInterface);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetAndPopulateVEHintParams(cmdBuffer));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(m_osInterface, cmdBuffer, bNullRendering));
    return eStatus;
}

MOS_STATUS CodechalVdencAvcStateG12::InitKernelStateSFD()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    auto renderEngineInterface = m_hwInterface->GetRenderInterface();
    auto stateHeapInterface    = m_renderEngineInterface->m_stateHeapInterface;
    CODECHAL_ENCODE_CHK_NULL_RETURN(stateHeapInterface);

    uint8_t* kernelBinary;
    uint32_t kernelSize;

    MOS_STATUS status = CodecHalGetKernelBinaryAndSize(m_kernelBase, m_kuidCommon, &kernelBinary, &kernelSize);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(status);

    CODECHAL_KERNEL_HEADER currKrnHeader;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(GetCommonKernelHeaderAndSizeG12(
        kernelBinary,
        ENC_SFD,
        0,
        (void*)&currKrnHeader,
        &kernelSize));

    auto kernelStatePtr                            = m_sfdKernelState;
    kernelStatePtr->KernelParams.iBTCount          = sfdNumSurfaces;
    kernelStatePtr->KernelParams.iThreadCount      = m_renderEngineInterface->GetHwCaps()->dwMaxThreads;
    kernelStatePtr->KernelParams.iCurbeLength      = sizeof(SfdCurbe);
    kernelStatePtr->KernelParams.iBlockWidth       = CODECHAL_MACROBLOCK_WIDTH;
    kernelStatePtr->KernelParams.iBlockHeight      = CODECHAL_MACROBLOCK_HEIGHT;
    kernelStatePtr->KernelParams.iIdCount          = 1;
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

MOS_STATUS CodechalVdencAvcStateG12::Initialize(CodechalSetting * settings)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalVdencAvcState::Initialize(settings));

    MOS_USER_FEATURE_VALUE_DATA userFeatureData;
    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_VDENC_ULTRA_MODE_ENABLE_ID,
        &userFeatureData,
        m_osInterface->pOsContext);
    m_vdencUltraModeEnable = userFeatureData.bData == 1;

    return eStatus;
}

bool CodechalVdencAvcStateG12::ProcessRoiDeltaQp()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    // Intialize ROIDistinctDeltaQp to be min expected delta qp, setting to -128
    // Check if forceQp is needed or not
    // forceQp is enabled if there are greater than 3 distinct delta qps or if the deltaqp is beyond range (-8, 7)
    for (auto k = 0; k < m_maxNumRoi; k++)
    {
        m_avcPicParam->ROIDistinctDeltaQp[k] = -128;
    }

    int32_t numQp = 0;
    for (int32_t i = 0; i < m_avcPicParam->NumROI; i++)
    {
        bool dqpNew = true;

        //Get distinct delta Qps among all ROI regions, index 0 having the lowest delta qp
        int32_t k = numQp - 1;
        for (; k >= 0; k--)
        {
            if (m_avcPicParam->ROI[i].PriorityLevelOrDQp == m_avcPicParam->ROIDistinctDeltaQp[k] ||
                m_avcPicParam->ROI[i].PriorityLevelOrDQp == 0)
            {
                dqpNew = false;
                break;
            }
            else if (m_avcPicParam->ROI[i].PriorityLevelOrDQp < m_avcPicParam->ROIDistinctDeltaQp[k])
            {
                continue;
            }
            else
            {
                break;
            }
        }

        if (dqpNew)
        {
            for (int32_t j = numQp - 1; (j >= k + 1 && j >= 0); j--)
            {
                m_avcPicParam->ROIDistinctDeltaQp[j + 1] = m_avcPicParam->ROIDistinctDeltaQp[j];
            }
            m_avcPicParam->ROIDistinctDeltaQp[k + 1] = m_avcPicParam->ROI[i].PriorityLevelOrDQp;
            numQp++;
        }
    }

    //Set the ROI DeltaQp to zero for remaining array elements
    for (auto k = numQp; k < m_maxNumRoi; k++)
    {
        m_avcPicParam->ROIDistinctDeltaQp[k] = 0;
    }
    m_avcPicParam->NumROIDistinctDeltaQp = (int8_t)numQp;

    // return whether is native ROI or not
    return !(numQp > m_maxNumNativeRoi || m_avcPicParam->ROIDistinctDeltaQp[0] < -8 || m_avcPicParam->ROIDistinctDeltaQp[numQp - 1] > 7);
}

bool CodechalVdencAvcStateG12::IsMBBRCControlEnabled()
{
    return m_mbBrcEnabled;
}


bool CodechalVdencAvcStateG12::CheckSupportedFormat(PMOS_SURFACE surface)
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

MOS_STATUS CodechalVdencAvcStateG12::GetTrellisQuantization(PCODECHAL_ENCODE_AVC_TQ_INPUT_PARAMS params, PCODECHAL_ENCODE_AVC_TQ_PARAMS trellisQuantParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(trellisQuantParams);

    trellisQuantParams->dwTqEnabled  = TrellisQuantizationEnable[params->ucTargetUsage];
    trellisQuantParams->dwTqRounding = trellisQuantParams->dwTqEnabled ? TrellisQuantizationRounding[params->ucTargetUsage] : 0;

    return eStatus;
}

MOS_STATUS CodechalVdencAvcStateG12::AddHucOutputRegistersHandling(
    MmioRegistersHuc*   mmioRegisters,
    PMOS_COMMAND_BUFFER cmdBuffer,
    bool                addToEncodeStatus)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(mmioRegisters);
    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(StoreHucErrorStatus(mmioRegisters, cmdBuffer, addToEncodeStatus));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InsertConditionalBBEndWithHucErrorStatus(cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalVdencAvcStateG12::InsertConditionalBBEndWithHucErrorStatus(PMOS_COMMAND_BUFFER cmdBuffer)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);

    MHW_MI_ENHANCED_CONDITIONAL_BATCH_BUFFER_END_PARAMS  miEnhancedConditionalBatchBufferEndParams;

    MOS_ZeroMemory(
        &miEnhancedConditionalBatchBufferEndParams,
        sizeof(MHW_MI_ENHANCED_CONDITIONAL_BATCH_BUFFER_END_PARAMS));

    miEnhancedConditionalBatchBufferEndParams.presSemaphoreBuffer = &m_resHucErrorStatusBuffer;

    miEnhancedConditionalBatchBufferEndParams.dwParamsType = MHW_MI_ENHANCED_CONDITIONAL_BATCH_BUFFER_END_PARAMS::ENHANCED_PARAMS;
    miEnhancedConditionalBatchBufferEndParams.enableEndCurrentBatchBuffLevel = false;
    miEnhancedConditionalBatchBufferEndParams.compareOperation = MAD_EQUAL_IDD;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiConditionalBatchBufferEndCmd(
        cmdBuffer,
        (PMHW_MI_CONDITIONAL_BATCH_BUFFER_END_PARAMS)(&miEnhancedConditionalBatchBufferEndParams)));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalVdencAvcStateG12::SetDmemHuCBrcInitReset()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    // Setup BRC DMEM
    MOS_LOCK_PARAMS lockFlagsWriteOnly;
    memset(&lockFlagsWriteOnly, 0, sizeof(MOS_LOCK_PARAMS));
    lockFlagsWriteOnly.WriteOnly = 1;
    auto hucVDEncBrcInitDmem     = (BrcInitDmem *)m_osInterface->pfnLockResource(
        m_osInterface, &m_resVdencBrcInitDmemBuffer[m_currRecycledBufIdx], &lockFlagsWriteOnly);

    CODECHAL_ENCODE_CHK_NULL_RETURN(hucVDEncBrcInitDmem);
    memset(hucVDEncBrcInitDmem, 0, sizeof(BrcInitDmem));

    SetDmemHuCBrcInitResetImpl<BrcInitDmem>(hucVDEncBrcInitDmem);

    // enable fractional QP by extended rho domain setting
    hucVDEncBrcInitDmem->INIT_FracQPEnable_U8 = (uint8_t)m_vdencInterface->IsRhoDomainStatsEnabled();
    // enable fractional QP for TCBRC
    if ((m_avcPicParam->TargetFrameSize > 0) && (m_lookaheadDepth == 0))
        hucVDEncBrcInitDmem->INIT_FracQPEnable_U8 = 1;

    hucVDEncBrcInitDmem->INIT_SinglePassOnly = m_vdencSinglePassEnable ? true : false;

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
            hucVDEncBrcInitDmem->INIT_DeltaQP_Adaptation_U8 = 0;
        }

        hucVDEncBrcInitDmem->INIT_New_DeltaQP_Adaptation_U8 = 1;
    }

    if (((m_avcSeqParam->TargetUsage & 0x07) == TARGETUSAGE_BEST_SPEED) &&
        (m_avcSeqParam->FrameWidth >= m_singlePassMinFrameWidth) &&
        (m_avcSeqParam->FrameHeight >= m_singlePassMinFrameHeight) &&
        (m_avcSeqParam->FramesPer100Sec >= m_singlePassMinFramePer100s))
    {
        hucVDEncBrcInitDmem->INIT_SinglePassOnly = true;
    }

    hucVDEncBrcInitDmem->INIT_LookaheadDepth_U8 = m_lookaheadDepth;

    //Override the DistQPDelta setting
    if (m_mbBrcEnabled)
    {
        if (m_avcSeqParam->FrameSizeTolerance == EFRAMESIZETOL_EXTREMELY_LOW)
        {
            MOS_SecureMemcpy(hucVDEncBrcInitDmem->INIT_DistQPDelta_I8, 4 * sizeof(int8_t), (void*)m_brcInitDistQpDeltaI8LowDelay, 4 * sizeof(int8_t));
        }
        else
        {
            MOS_SecureMemcpy(hucVDEncBrcInitDmem->INIT_DistQPDelta_I8, 4 * sizeof(int8_t), (void*)m_brcInitDistQpDeltaI8, 4 * sizeof(int8_t));
        }
    }

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(PopulateBrcInitParam(hucVDEncBrcInitDmem));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(DumpParsedBRCInitDmem(hucVDEncBrcInitDmem));
    )

    m_osInterface->pfnUnlockResource(m_osInterface, &m_resVdencBrcInitDmemBuffer[m_currRecycledBufIdx]);

    return eStatus;
}

MOS_STATUS CodechalVdencAvcStateG12::DeltaQPUpdate(uint8_t QpModulationStrength)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    m_qpModulationStrength = QpModulationStrength;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalVdencAvcStateG12::SetDmemHuCBrcUpdate()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    // Program update DMEM
    MOS_LOCK_PARAMS lockFlags;
    memset(&lockFlags, 0, sizeof(MOS_LOCK_PARAMS));
    lockFlags.WriteOnly  = 1;
    auto hucVDEncBrcDmem = (BrcUpdateDmem *)m_osInterface->pfnLockResource(
        m_osInterface, &m_resVdencBrcUpdateDmemBuffer[m_currRecycledBufIdx][m_currPass], &lockFlags);
    CODECHAL_ENCODE_CHK_NULL_RETURN(hucVDEncBrcDmem);
    SetDmemHuCBrcUpdateImpl<BrcUpdateDmem>(hucVDEncBrcDmem);

    if (hucVDEncBrcDmem->UPD_CurrFrameType_U8 == 1 && m_avcPicParam->RefPicFlag == 1)
        hucVDEncBrcDmem->UPD_CurrFrameType_U8 = 3;  // separated type for reference B

    MOS_LOCK_PARAMS lockFlagsReadOnly;
    MOS_ZeroMemory(&lockFlagsReadOnly, sizeof(MOS_LOCK_PARAMS));
    lockFlagsReadOnly.ReadOnly = 1;
    auto initDmem              = (BrcInitDmem *)m_osInterface->pfnLockResource(
        m_osInterface, &m_resVdencBrcInitDmemBuffer[m_currRecycledBufIdx], &lockFlagsReadOnly);
    CODECHAL_ENCODE_CHK_NULL_RETURN(initDmem);

    if (initDmem->INIT_AdaptiveHMEExtensionEnable_U8)
    {
        hucVDEncBrcDmem->HME0XOffset_I8 = 32;
        hucVDEncBrcDmem->HME0YOffset_I8 = 24;
        hucVDEncBrcDmem->HME1XOffset_I8 = -32;
        hucVDEncBrcDmem->HME1YOffset_I8 = -24;
    }

    m_osInterface->pfnUnlockResource(m_osInterface, &m_resVdencBrcInitDmemBuffer[m_currRecycledBufIdx]);

    if (m_16xMeSupported && (m_pictureCodingType == P_TYPE))
    {
        hucVDEncBrcDmem->HmeDistAvailable_U8 = 1;
    }
    hucVDEncBrcDmem->UPD_WidthInMB_U16  = m_picWidthInMb;
    hucVDEncBrcDmem->UPD_HeightInMB_U16 = m_picHeightInMb;

    hucVDEncBrcDmem->MOTION_ADAPTIVE_G4 = (m_avcSeqParam->ScenarioInfo == ESCENARIO_GAMESTREAMING) || ((m_avcPicParam->TargetFrameSize > 0) && (m_lookaheadDepth == 0));  // GS or TCBRC
    hucVDEncBrcDmem->UPD_CQMEnabled_U8  = m_avcSeqParam->seq_scaling_matrix_present_flag || m_avcPicParam->pic_scaling_matrix_present_flag;

    hucVDEncBrcDmem->UPD_LA_TargetSize_U32 = m_avcPicParam->TargetFrameSize << 3;

    if (m_lookaheadDepth > 0)
    {
        DeltaQPUpdate(m_avcPicParam->QpModulationStrength);
        hucVDEncBrcDmem->EnableLookAhead = 1;
        hucVDEncBrcDmem->UPD_LA_TargetFulness_U32 = m_targetBufferFulness;
        hucVDEncBrcDmem->UPD_Delta_U8 = m_qpModulationStrength;
    }

    // Temporal fix because of DDI flag deprication
    // Use Cloud Gaming mode by default
    hucVDEncBrcDmem->UPD_TCBRC_SCENARIO_U8 = 0;

    hucVDEncBrcDmem->UPD_NumSlicesForRounding = GetAdaptiveRoundingNumSlices();
    hucVDEncBrcDmem->UPD_UserMaxFramePB       = 2 * m_avcPicParam->TargetFrameSize;

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(PopulateBrcUpdateParam(hucVDEncBrcDmem));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(DumpParsedBRCUpdateDmem(hucVDEncBrcDmem));
    )

    m_osInterface->pfnUnlockResource(m_osInterface, &(m_resVdencBrcUpdateDmemBuffer[m_currRecycledBufIdx][m_currPass]));

    return eStatus;
}

MOS_STATUS CodechalVdencAvcStateG12::LoadMvCost(uint8_t qp)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    for (uint8_t i = 0; i < 8; i++)
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

MOS_STATUS CodechalVdencAvcStateG12::LoadHmeMvCost(uint8_t qp)
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

MOS_STATUS CodechalVdencAvcStateG12::LoadHmeMvCostTable(PCODEC_AVC_ENCODE_SEQUENCE_PARAMS seqParams, uint8_t hmeMVCostTable[8][42])
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

    for (uint8_t i = 0; i < 8; i++)
    {
        for (uint8_t j = 0; j < 42; j++)
        {
            hmeMVCostTable[i][j] = Map44LutValue(*(vdencHmeCostTable[i] + j + 10), 0x6f);
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalVdencAvcStateG12::AddVdencWalkerStateCmd(
    PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    MHW_VDBOX_VDENC_WALKER_STATE_PARAMS_G12 vdencWalkerStateParams;
    auto avcSlcParams = m_avcSliceParams;
    auto avcPicParams = m_avcPicParams[avcSlcParams->pic_parameter_set_id];
    auto avcSeqParams = m_avcSeqParams[avcPicParams->seq_parameter_set_id];

    vdencWalkerStateParams.Mode = CODECHAL_ENCODE_MODE_AVC;
    vdencWalkerStateParams.pAvcSeqParams = avcSeqParams;
    vdencWalkerStateParams.pAvcSlcParams = m_avcSliceParams;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdencWalkerStateCmd(cmdBuffer, &vdencWalkerStateParams));

    return eStatus;
}

MOS_STATUS CodechalVdencAvcStateG12::CalculateVdencCommandsSize()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    MHW_VDBOX_STATE_CMDSIZE_PARAMS_G12 stateCmdSizeParams;
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

#if USE_CODECHAL_DEBUG_TOOL
    // for ModifyEncodedFrameSizeWithFakeHeaderSize
    // total sum is 368 (108*2 + 152)
    if (m_hucInterface && m_enableFakeHrdSize)
        m_pictureStatesSize +=
        // 2x AddBufferWithIMMValue to change frame size
            (
                mhw_mi_g12_X::MI_FLUSH_DW_CMD::byteSize +
                mhw_mi_g12_X::MI_LOAD_REGISTER_MEM_CMD::byteSize +
                mhw_mi_g12_X::MI_LOAD_REGISTER_IMM_CMD::byteSize * 3 +
                mhw_mi_g12_X::MI_MATH_CMD::byteSize + sizeof(MHW_MI_ALU_PARAMS) * 4 +
                mhw_mi_g12_X::MI_STORE_REGISTER_MEM_CMD::byteSize
            ) * 2 +
            // SetBufferWithIMMValueU16 to change header size
            (   mhw_mi_g12_X::MI_FLUSH_DW_CMD::byteSize +
                mhw_mi_g12_X::MI_LOAD_REGISTER_MEM_CMD::byteSize +
                mhw_mi_g12_X::MI_LOAD_REGISTER_IMM_CMD::byteSize * 5 +
                2 * (mhw_mi_g12_X::MI_MATH_CMD::byteSize + sizeof(MHW_MI_ALU_PARAMS) * 4) +
                mhw_mi_g12_X::MI_STORE_REGISTER_MEM_CMD::byteSize);
#endif

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

MOS_STATUS CodechalVdencAvcStateG12::SendPrologWithFrameTracking(
    PMOS_COMMAND_BUFFER         cmdBuffer,
    bool                        frameTracking,
    MHW_MI_MMIOREGISTERS       *mmioRegister)
{
    MHW_MI_FORCE_WAKEUP_PARAMS forceWakeupParams;
    MOS_ZeroMemory(&forceWakeupParams, sizeof(MHW_MI_FORCE_WAKEUP_PARAMS));
    forceWakeupParams.bMFXPowerWellControl = true;
    forceWakeupParams.bMFXPowerWellControlMask = true;
    forceWakeupParams.bHEVCPowerWellControl = false;
    forceWakeupParams.bHEVCPowerWellControlMask = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiForceWakeupCmd(
        cmdBuffer,
        &forceWakeupParams));

    if (MOS_VE_SUPPORTED(m_osInterface) && cmdBuffer->Attributes.pAttriVe)
    {
        PMOS_CMD_BUF_ATTRI_VE attriExt =
                (PMOS_CMD_BUF_ATTRI_VE)(cmdBuffer->Attributes.pAttriVe);
        attriExt->bUseVirtualEngineHint = true;
        attriExt->VEngineHintParams.NeedSyncWithPrevious = 1;
    }

    return CodechalVdencAvcState::SendPrologWithFrameTracking(cmdBuffer, frameTracking, mmioRegister);
}

MOS_STATUS CodechalVdencAvcStateG12::InitMmcState()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;
#ifdef _MMC_SUPPORTED
    m_mmcState = MOS_New(CodechalMmcEncodeAvcG12, m_hwInterface, this);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_mmcState);
#endif
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalVdencAvcStateG12::CheckResChangeAndCsc()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (m_cscDsState && m_rawSurface.Format == Format_A8R8G8B8)
    {
        uint64_t alignedSize = MOS_MAX((uint64_t)m_picWidthInMb * CODECHAL_MACROBLOCK_WIDTH * 4, (uint64_t)m_rawSurface.dwPitch) *
                               ((uint64_t)m_picHeightInMb * CODECHAL_MACROBLOCK_HEIGHT);

        if (m_rawSurface.OsResource.iSize < alignedSize)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cscDsState->SurfaceNeedsExtraCopy());
        }
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncoderState::CheckResChangeAndCsc());
    return MOS_STATUS_SUCCESS;
}

void CodechalVdencAvcStateG12::SetMfxAvcImgStateParams(MHW_VDBOX_AVC_IMG_PARAMS& param)
{
    CodechalVdencAvcState::SetMfxAvcImgStateParams(param);

    PMHW_VDBOX_AVC_IMG_PARAMS_G12 paramsG12 = static_cast<PMHW_VDBOX_AVC_IMG_PARAMS_G12>(&param);

    paramsG12->bVDEncUltraModeEnabled = m_vdencUltraModeEnable;
    param.bPerMBStreamOut = m_perMBStreamOutEnable;
    if (((m_avcSeqParam->TargetUsage & 0x07) == TARGETUSAGE_BEST_SPEED) &&
        (m_avcSeqParam->FrameWidth >= m_singlePassMinFrameWidth) &&
        (m_avcSeqParam->FrameHeight >= m_singlePassMinFrameHeight) &&
        (m_avcSeqParam->FramesPer100Sec >=m_singlePassMinFramePer100s))
    {
        paramsG12->bVDEncUltraModeEnabled = true;
    }
    paramsG12->oneOnOneMapping = m_oneOnOneMapping;
    paramsG12->bStreamInMbQpEnabled = m_encodeParams.bMbQpDataEnabled;
}

PMHW_VDBOX_STATE_CMDSIZE_PARAMS CodechalVdencAvcStateG12::CreateMhwVdboxStateCmdsizeParams()
{
    PMHW_VDBOX_STATE_CMDSIZE_PARAMS cmdSizeParams = MOS_New(MHW_VDBOX_STATE_CMDSIZE_PARAMS_G12);

    return cmdSizeParams;
}

PMHW_VDBOX_AVC_IMG_PARAMS CodechalVdencAvcStateG12::CreateMhwVdboxAvcImgParams()
{
    PMHW_VDBOX_AVC_IMG_PARAMS avcImgParams = MOS_New(MHW_VDBOX_AVC_IMG_PARAMS_G12);

    return avcImgParams;
}

PMHW_VDBOX_VDENC_WALKER_STATE_PARAMS CodechalVdencAvcStateG12::CreateMhwVdboxVdencWalkerStateParams()
{
    PMHW_VDBOX_VDENC_WALKER_STATE_PARAMS vdencWalkerStateParams = MOS_New(MHW_VDBOX_VDENC_WALKER_STATE_PARAMS_G12);

    return vdencWalkerStateParams;
}

MOS_STATUS CodechalVdencAvcStateG12::InitKernelStateMe()
{
#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
    m_hmeKernel = MOS_New(CodechalKernelHmeG12, this);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hmeKernel);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hmeKernel->Initialize(
        GetCommonKernelHeaderAndSizeG12,
        m_kernelBase,
        m_kuidCommon));
#endif
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalVdencAvcStateG12::ExecuteMeKernel()
{
    #if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
    if (m_hmeKernel && m_hmeKernel->Is4xMeEnabled())
    {
        CodechalKernelHme::CurbeParam curbeParam = {};
        curbeParam.subPelMode = 3;
        curbeParam.currOriginalPic = m_avcPicParam->CurrOriginalPic;
        curbeParam.qpPrimeY = m_avcPicParam->pic_init_qp_minus26 + 26 + m_avcSliceParams->slice_qp_delta;
        curbeParam.targetUsage = m_avcSeqParam->TargetUsage;
        curbeParam.maxMvLen = CodecHalAvcEncode_GetMaxMvLen(m_avcSeqParam->Level);

        AdjustNumRefIdx(curbeParam.numRefIdxL0Minus1, curbeParam.numRefIdxL1Minus1);

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
        surfaceParam.meVdencStreamInBuffer = &m_resVdencStreamInBuffer[m_currRecycledBufIdx];
        surfaceParam.vdencStreamInSurfaceSize = MOS_BYTES_TO_DWORDS(m_picHeightInMb * m_picWidthInMb * 64);

        if (m_hmeKernel->Is16xMeEnabled())
        {
            m_lastTaskInPhase = false;
            if (m_hmeKernel->Is32xMeEnabled())
            {
                surfaceParam.downScaledWidthInMb = m_downscaledWidthInMb32x;
                surfaceParam.downScaledHeightInMb = m_downscaledFrameFieldHeightInMb32x;
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
    #endif
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalVdencAvcStateG12::UpdateCmdBufAttribute(
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

MOS_STATUS CodechalVdencAvcStateG12::AddMediaVfeCmd(
    PMOS_COMMAND_BUFFER cmdBuffer,
    SendKernelCmdsParams *params)
{
    CODECHAL_ENCODE_CHK_NULL_RETURN(params);

    MHW_VFE_PARAMS_G12 vfeParams = {};
    vfeParams.pKernelState              = params->pKernelState;
    vfeParams.eVfeSliceDisable          = MHW_VFE_SLICE_ALL;
    vfeParams.dwMaximumNumberofThreads  = m_encodeVfeMaxThreads;
    vfeParams.bFusedEuDispatch          = false; // legacy mode

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_renderEngineInterface->AddMediaVfeCmd(cmdBuffer, &vfeParams));

    return MOS_STATUS_SUCCESS;
}

void CodechalVdencAvcStateG12::SetMfxPipeModeSelectParams(
    const CODECHAL_ENCODE_AVC_GENERIC_PICTURE_LEVEL_PARAMS& genericParam,
    MHW_VDBOX_PIPE_MODE_SELECT_PARAMS& param)
{
    CodechalEncodeAvcBase::SetMfxPipeModeSelectParams(genericParam, param);

    auto avcPicParams = m_avcPicParams[m_avcSliceParams->pic_parameter_set_id];
    auto avcSeqParams = m_avcSeqParams[avcPicParams->seq_parameter_set_id];
    auto paramGen12   = ((MHW_VDBOX_PIPE_MODE_SELECT_PARAMS_G12 *)&param);

    if (avcSeqParams->EnableStreamingBufferLLC || avcSeqParams->EnableStreamingBufferDDR)
    {
        paramGen12->bStreamingBufferEnabled = true;
    }
}

void CodechalVdencAvcStateG12::CopyMBQPDataToStreamIn(CODECHAL_VDENC_STREAMIN_STATE* pData, uint8_t* pInputData)
{
    for (uint32_t curY = 0; curY < m_picHeightInMb; curY++)
    {
        for (uint32_t curX = 0; curX < m_picWidthInMb; curX++)
        {
            pData->DW0.RegionOfInterestRoiSelection = 0;
            pData->DW1.Qpprimey = *(pInputData + m_encodeParams.psMbQpDataSurface->dwPitch * curY + curX);
            pData++;
        }
    }
}

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS CodechalVdencAvcStateG12::PopulateBrcInitParam(
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

MOS_STATUS CodechalVdencAvcStateG12::PopulateBrcUpdateParam(
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

MOS_STATUS CodechalVdencAvcStateG12::PopulateEncParam(
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
        data = data + mhw_vdbox_mfx_g12_X::MFX_AVC_IMG_STATE_CMD::byteSize;
    }
    else
    {
        // CQP case: VDENC IMG STATE is updated by driver or SFD kernel
        if (!m_staticFrameDetectionInUse)
        {
            data = m_batchBufferForVdencImgStat[m_currRecycledBufIdx].pData;
            data = data + mhw_vdbox_mfx_g12_X::MFX_AVC_IMG_STATE_CMD::byteSize;
        }
        else
        {
            data = (uint8_t *)m_osInterface->pfnLockResource(m_osInterface, &m_resVdencSfdImageStateReadBuffer, &lockFlags);
        }
    }

    CODECHAL_DEBUG_CHK_NULL(data);

    mhw_vdbox_vdenc_g12_X::VDENC_IMG_STATE_CMD vdencCmd;
    vdencCmd = *(mhw_vdbox_vdenc_g12_X::VDENC_IMG_STATE_CMD *)(data);

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

MOS_STATUS CodechalVdencAvcStateG12::PopulatePakParam(
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
        data = (uint8_t*)(cmdBuffer->pCmdPtr - (mhw_vdbox_mfx_g12_X::MFX_AVC_IMG_STATE_CMD::byteSize / sizeof(uint32_t)));
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

    mhw_vdbox_mfx_g12_X::MFX_AVC_IMG_STATE_CMD mfxCmd;
    mfxCmd = *(mhw_vdbox_mfx_g12_X::MFX_AVC_IMG_STATE_CMD *)(data);

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

#define FIELD_TO_SS(field_name) ss << std::setfill(' ') << std::setw(25) << std::left << std::string(#field_name) + ": " << (int64_t)dmem->field_name << std::endl;
#define ARRAY_TO_SS(arr_name)                                                              \
{                                                                                           \
    size_t size = sizeof(dmem->arr_name);                                                   \
    ss << std::setfill(' ') << std::setw(25) << std::left << std::string(#arr_name) + ": "; \
    ss << "{ " << (int64_t)dmem->arr_name[0];                                               \
    for (size_t i = 1; i < sizeof(dmem->arr_name); ++i)                                     \
        ss << ", " << (int64_t)dmem->arr_name[i];                                           \
    ss << " };" << std::endl;                                                               \
}

MOS_STATUS CodechalVdencAvcStateG12::DumpParsedBRCInitDmem(BrcInitDmem* dmem)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;
    CODECHAL_DEBUG_CHK_NULL(m_debugInterface);

    // To make sure that DMEM doesn't changed and parsed dump contains all DMEM fields
    CODECHAL_DEBUG_ASSERT(sizeof(dmem->RSVD2) == 55);

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrHuCDmem))
    {
        return MOS_STATUS_SUCCESS;
    }

    std::stringstream ss;

    FIELD_TO_SS(BRCFunc_U8);
    FIELD_TO_SS(OpenSourceEnable_U8);
    ARRAY_TO_SS(RVSD);
    FIELD_TO_SS(INIT_BRCFlag_U16);
    FIELD_TO_SS(Reserved);
    FIELD_TO_SS(INIT_FrameWidth_U16);
    FIELD_TO_SS(INIT_FrameHeight_U16);
    FIELD_TO_SS(INIT_TargetBitrate_U32);
    FIELD_TO_SS(INIT_MinRate_U32);
    FIELD_TO_SS(INIT_MaxRate_U32);
    FIELD_TO_SS(INIT_BufSize_U32);
    FIELD_TO_SS(INIT_InitBufFull_U32);
    FIELD_TO_SS(INIT_ProfileLevelMaxFrame_U32);
    FIELD_TO_SS(INIT_FrameRateM_U32);
    FIELD_TO_SS(INIT_FrameRateD_U32);
    FIELD_TO_SS(INIT_GopP_U16);
    FIELD_TO_SS(INIT_GopB_U16);
    FIELD_TO_SS(INIT_MinQP_U16);
    FIELD_TO_SS(INIT_MaxQP_U16);
    ARRAY_TO_SS(INIT_DevThreshPB0_S8);
    ARRAY_TO_SS(INIT_DevThreshVBR0_S8);
    ARRAY_TO_SS(INIT_DevThreshI0_S8);
    FIELD_TO_SS(INIT_InitQPIP);

    FIELD_TO_SS(INIT_NotUseRhoDm_U8);
    FIELD_TO_SS(INIT_InitQPB);
    FIELD_TO_SS(INIT_MbQpCtrl_U8);
    FIELD_TO_SS(INIT_SliceSizeCtrlEn_U8);
    ARRAY_TO_SS(INIT_IntraQPDelta_I8);
    FIELD_TO_SS(INIT_SkipQPDelta_I8);
    ARRAY_TO_SS(INIT_DistQPDelta_I8);
    FIELD_TO_SS(INIT_OscillationQpDelta_U8);
    FIELD_TO_SS(INIT_HRDConformanceCheckDisable_U8);
    FIELD_TO_SS(INIT_SkipFrameEnableFlag);
    FIELD_TO_SS(INIT_TopQPDeltaThrForAdapt2Pass_U8);
    FIELD_TO_SS(INIT_TopFrmSzThrForAdapt2Pass_U8);
    FIELD_TO_SS(INIT_BotFrmSzThrForAdapt2Pass_U8);
    FIELD_TO_SS(INIT_QPSelectForFirstPass_U8);
    FIELD_TO_SS(INIT_MBHeaderCompensation_U8);
    FIELD_TO_SS(INIT_OverShootCarryFlag_U8);
    FIELD_TO_SS(INIT_OverShootSkipFramePct_U8);
    ARRAY_TO_SS(INIT_EstRateThreshP0_U8);
    ARRAY_TO_SS(INIT_EstRateThreshB0_U8);
    ARRAY_TO_SS(INIT_EstRateThreshI0_U8);
    FIELD_TO_SS(INIT_FracQPEnable_U8);
    FIELD_TO_SS(INIT_ScenarioInfo_U8);
    FIELD_TO_SS(INIT_StaticRegionStreamIn_U8);
    FIELD_TO_SS(INIT_DeltaQP_Adaptation_U8);
    FIELD_TO_SS(INIT_MaxCRFQualityFactor_U8);
    FIELD_TO_SS(INIT_CRFQualityFactor_U8);
    FIELD_TO_SS(INIT_BotQPDeltaThrForAdapt2Pass_U8);
    FIELD_TO_SS(INIT_SlidingWindowSize_U8);
    FIELD_TO_SS(INIT_SlidingWidowRCEnable_U8);
    FIELD_TO_SS(INIT_SlidingWindowMaxRateRatio_U8);
    FIELD_TO_SS(INIT_LowDelayGoldenFrameBoost_U8);
    FIELD_TO_SS(INIT_AdaptiveCostEnable_U8);
    FIELD_TO_SS(INIT_AdaptiveHMEExtensionEnable_U8);
    FIELD_TO_SS(INIT_ICQReEncode_U8);
    FIELD_TO_SS(INIT_LookaheadDepth_U8);
    FIELD_TO_SS(INIT_SinglePassOnly);
    FIELD_TO_SS(INIT_New_DeltaQP_Adaptation_U8);
    ARRAY_TO_SS(RSVD2);

    std::string bufName = std::string("ENC-HucDmemInit_Parsed_PASS") + std::to_string((uint32_t)m_currPass);
    CODECHAL_DEBUG_CHK_STATUS(m_debugInterface->DumpStringStream(ss, bufName.c_str(), MediaDbgAttr::attrHuCDmem));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalVdencAvcStateG12::DumpParsedBRCUpdateDmem(BrcUpdateDmem* dmem)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;
    CODECHAL_DEBUG_CHK_NULL(m_debugInterface);

    // To make sure that DMEM doesn't changed and parsed dump contains all DMEM fields
    CODECHAL_DEBUG_ASSERT(sizeof(dmem->RSVD2) == 4);

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrHuCDmem))
    {
        return MOS_STATUS_SUCCESS;
    }

    std::stringstream ss;

    FIELD_TO_SS(BRCFunc_U8);
    ARRAY_TO_SS(RSVD);
    FIELD_TO_SS(UPD_TARGETSIZE_U32);
    FIELD_TO_SS(UPD_FRAMENUM_U32);
    FIELD_TO_SS(UPD_PeakTxBitsPerFrame_U32);
    FIELD_TO_SS(UPD_FrameBudget_U32);
    FIELD_TO_SS(FrameByteCount);
    FIELD_TO_SS(TimingBudgetOverflow);
    FIELD_TO_SS(ImgStatusCtrl);
    FIELD_TO_SS(IPCMNonConformant);

    ARRAY_TO_SS(UPD_startGAdjFrame_U16);
    ARRAY_TO_SS(UPD_MBBudget_U16);
    FIELD_TO_SS(UPD_SLCSZ_TARGETSLCSZ_U16);
    ARRAY_TO_SS(UPD_SLCSZ_UPD_THRDELTAI_U16);
    ARRAY_TO_SS(UPD_SLCSZ_UPD_THRDELTAP_U16);
    FIELD_TO_SS(UPD_NumOfFramesSkipped_U16);
    FIELD_TO_SS(UPD_SkipFrameSize_U16);
    FIELD_TO_SS(UPD_StaticRegionPct_U16);
    ARRAY_TO_SS(UPD_gRateRatioThreshold_U8);
    FIELD_TO_SS(UPD_CurrFrameType_U8);
    ARRAY_TO_SS(UPD_startGAdjMult_U8);
    ARRAY_TO_SS(UPD_startGAdjDiv_U8);
    ARRAY_TO_SS(UPD_gRateRatioThresholdQP_U8);
    FIELD_TO_SS(UPD_PAKPassNum_U8);
    FIELD_TO_SS(UPD_MaxNumPass_U8);
    ARRAY_TO_SS(UPD_SceneChgWidth_U8);
    FIELD_TO_SS(UPD_SceneChgDetectEn_U8);
    FIELD_TO_SS(UPD_SceneChgPrevIntraPctThreshold_U8);
    FIELD_TO_SS(UPD_SceneChgCurIntraPctThreshold_U8);
    FIELD_TO_SS(UPD_IPAverageCoeff_U8);
    FIELD_TO_SS(UPD_MinQpAdjustment_U8);
    FIELD_TO_SS(UPD_TimingBudgetCheck_U8);
    ARRAY_TO_SS(reserved_I8);
    FIELD_TO_SS(UPD_CQP_QpValue_U8);
    FIELD_TO_SS(UPD_CQP_FracQp_U8);
    FIELD_TO_SS(UPD_HMEDetectionEnable_U8);
    FIELD_TO_SS(UPD_HMECostEnable_U8);
    FIELD_TO_SS(UPD_DisablePFrame8x8Transform_U8);
    FIELD_TO_SS(RSVD3);
    FIELD_TO_SS(UPD_ROISource_U8);
    FIELD_TO_SS(RSVD4);
    FIELD_TO_SS(UPD_TargetSliceSize_U16);
    FIELD_TO_SS(UPD_MaxNumSliceAllowed_U16);
    FIELD_TO_SS(UPD_SLBB_Size_U16);
    FIELD_TO_SS(UPD_SLBB_B_Offset_U16);
    FIELD_TO_SS(UPD_AvcImgStateOffset_U16);
    FIELD_TO_SS(reserved_u16);
    FIELD_TO_SS(NumOfSlice);

    FIELD_TO_SS(AveHmeDist_U16);
    FIELD_TO_SS(HmeDistAvailable_U8);
    FIELD_TO_SS(DisableDMA);
    FIELD_TO_SS(AdditionalFrameSize_U16);
    FIELD_TO_SS(AddNALHeaderSizeInternally_U8);
    FIELD_TO_SS(UPD_RoiQpViaForceQp_U8);
    FIELD_TO_SS(CABACZeroInsertionSize_U32);
    FIELD_TO_SS(MiniFramePaddingSize_U32);
    FIELD_TO_SS(UPD_WidthInMB_U16);
    FIELD_TO_SS(UPD_HeightInMB_U16);
    ARRAY_TO_SS(UPD_ROIQpDelta_I8);

    FIELD_TO_SS(HME0XOffset_I8);
    FIELD_TO_SS(HME0YOffset_I8);
    FIELD_TO_SS(HME1XOffset_I8);
    FIELD_TO_SS(HME1YOffset_I8);
    FIELD_TO_SS(MOTION_ADAPTIVE_G4);
    FIELD_TO_SS(EnableLookAhead);
    FIELD_TO_SS(UPD_LA_Data_Offset_U8);
    FIELD_TO_SS(UPD_CQMEnabled_U8);
    FIELD_TO_SS(UPD_LA_TargetSize_U32);
    FIELD_TO_SS(UPD_LA_TargetFulness_U32);
    FIELD_TO_SS(UPD_Delta_U8);
    FIELD_TO_SS(UPD_ROM_CURRENT_U8);
    FIELD_TO_SS(UPD_ROM_ZERO_U8);
    FIELD_TO_SS(UPD_TCBRC_SCENARIO_U8);
    FIELD_TO_SS(UPD_EnableFineGrainLA);
    FIELD_TO_SS(UPD_DeltaQpDcOffset);
    FIELD_TO_SS(UPD_NumSlicesForRounding);
    FIELD_TO_SS(UPD_UserMaxFramePB);
    ARRAY_TO_SS(RSVD2);

    std::string bufName = std::string("ENC-HucDmemUpdate_Parsed_PASS") + std::to_string((uint32_t)m_currPass);
    CODECHAL_DEBUG_CHK_STATUS(m_debugInterface->DumpStringStream(ss, bufName.c_str(), MediaDbgAttr::attrHuCDmem));

    return MOS_STATUS_SUCCESS;
}
#undef FIELD_TO_SS
#undef ARRAY_TO_SS

MOS_STATUS CodechalVdencAvcStateG12::ModifyEncodedFrameSizeWithFakeHeaderSize( PMOS_COMMAND_BUFFER cmdBuffer)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (!m_fakeIFrameHrdSize && !m_fakePBFrameHrdSize)
        return MOS_STATUS_SUCCESS;

    uint32_t fakeHeaderSizeInBytes = (m_pictureCodingType == I_TYPE) ? m_fakeIFrameHrdSize : m_fakePBFrameHrdSize;

    //calculate all frame headers size, including 1st slice header
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_encodeParams.pBSBuffer);
    uint32_t totalHeaderSize = uint32_t(m_encodeParams.pBSBuffer->pCurrent - m_encodeParams.pBSBuffer->pBase);

    // change encdode frame size for next frame and next pass
    for (int i = 0; i < 2; i++)
    {
        if (m_resVdencBrcUpdateDmemBufferPtr[i] == nullptr)
            continue;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(AddBufferWithIMMValue(
            cmdBuffer,
            m_resVdencBrcUpdateDmemBufferPtr[i],
            sizeof(uint32_t) * 5,
            fakeHeaderSizeInBytes - totalHeaderSize,
            true));
    }

    // change headers size (U16)
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetBufferWithIMMValueU16(
        cmdBuffer,
        m_resPakStatsBuffer,
        0,
        fakeHeaderSizeInBytes * 8,
        0)); // second or first word in dword

    return MOS_STATUS_SUCCESS;
}
#endif
