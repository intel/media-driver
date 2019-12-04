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
//! \file     codechal_vdenc_avc_g12.cpp
//! \brief    This file implements the C++ class/interface for Gen12 platform's AVC
//!           VDEnc encoding to be used CODECHAL components.
//!

#include "codechal_vdenc_avc_g12.h"
#include "codechal_mmc_encode_avc_g12.h"
#include "codechal_kernel_header_g12.h"
#include "codechal_kernel_hme_g12.h"
#include "mhw_vdbox_vdenc_g12_X.h"
#include "mhw_vdbox_g12_X.h"
#include "mhw_render_g12_X.h"
#include "mos_util_user_interface_g12.h"
#include "codeckrnheader.h"
#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
#include "igcodeckrn_g12.h"
#endif
#if USE_CODECHAL_DEBUG_TOOL
#include "codechal_debug_encode_par_g12.h"
#include "mhw_vdbox_mfx_hwcmd_g12_X.h"
#include "mhw_vdbox_vdenc_hwcmd_g12_X.h"
#endif
#include "hal_oca_interface.h"

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

struct BrcInitDmem
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
    uint8_t     Reserved_u8;                                // must be zero
    uint8_t     INIT_SinglePassOnly;                  // 0: disabled, 1: enabled
    uint8_t     RSVD2[56];                            // must be zero
};
using PBrcInitDmem = struct BrcInitDmem*;

struct BrcUpdateDmem
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
    uint8_t     RSVD2[28];
};
using PBrcUpdateDmem = struct BrcUpdateDmem*;

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

    CODECHAL_ENCODE_ASSERT(m_osInterface);

    // Virtual Engine is enabled in default.
    Mos_SetVirtualEngineSupported(m_osInterface, true);

    Mos_CheckVirtualEngineSupported(m_osInterface, false, true);

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
    m_vdencBrcNumOfSliceOffset = CODECHAL_OFFSETOF(BrcUpdateDmem, NumOfSlice);

    // One Gen12, avc vdenc ref index need to be one on one mapping
    m_oneOnOneMapping = true;

    m_vdboxOneDefaultUsed = true;

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

    CODECHAL_DEBUG_TOOL(
        DestroyAvcPar();
        MOS_Delete(m_encodeParState);
    )
}

MOS_STATUS CodechalVdencAvcStateG12::InitializeState()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalVdencAvcState::InitializeState());

    m_sliceSizeStreamoutSupported      = true;
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
    CodecHalEncode_WriteKey(__MEDIA_USER_FEATURE_VALUE_ENABLE_ENCODE_VE_CTXSCHEDULING_ID, MOS_VE_CTXBASEDSCHEDULING_SUPPORTED(m_osInterface));

#endif // _DEBUG || _RELEASE_INTERNAL
    return eStatus;
}

MOS_STATUS CodechalVdencAvcStateG12::SubmitCommandBuffer(
    PMOS_COMMAND_BUFFER cmdBuffer,
    int32_t             nullRendering)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);

    HalOcaInterface::On1stLevelBBEnd(*cmdBuffer, *m_osInterface->pOsContext);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetAndPopulateVEHintParams(cmdBuffer));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(m_osInterface, cmdBuffer, nullRendering));
    return eStatus;
}

MOS_STATUS CodechalVdencAvcStateG12::InitKernelStateSFD()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    auto renderEngineInterface = m_hwInterface->GetRenderInterface();
    auto stateHeapInterface    = m_renderEngineInterface->m_stateHeapInterface;
    CODECHAL_ENCODE_CHK_NULL_RETURN(stateHeapInterface);

    m_sfdKernelState = MOS_New(MHW_KERNEL_STATE);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_sfdKernelState);

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
        __MEDIA_USER_FEATURE_VALUE_VDENC_ULTRA_MODE_ENABLE_ID_G12,
        &userFeatureData);
    m_vdencUltraModeEnable = userFeatureData.bData == 1;

    return eStatus;
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

MOS_STATUS CodechalVdencAvcStateG12::SetDmemHuCBrcInitReset()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    // Setup BRC DMEM
    MOS_LOCK_PARAMS lockFlagsWriteOnly;
    memset(&lockFlagsWriteOnly, 0, sizeof(MOS_LOCK_PARAMS));
    lockFlagsWriteOnly.WriteOnly = 1;
    auto hucVDEncBrcInitDmem     = (PBrcInitDmem)m_osInterface->pfnLockResource(
        m_osInterface, &m_resVdencBrcInitDmemBuffer[m_currRecycledBufIdx], &lockFlagsWriteOnly);

    CODECHAL_ENCODE_CHK_NULL_RETURN(hucVDEncBrcInitDmem);
    memset(hucVDEncBrcInitDmem, 0, sizeof(BrcInitDmem));

    SetDmemHuCBrcInitResetImpl<BrcInitDmem>(hucVDEncBrcInitDmem);

    // fractional QP enable for extended rho domain
    hucVDEncBrcInitDmem->INIT_FracQPEnable_U8 = (uint8_t)m_vdencInterface->IsRhoDomainStatsEnabled();

    hucVDEncBrcInitDmem->INIT_SinglePassOnly = m_vdencSinglePassEnable ? true : false;

    if (((m_avcSeqParam->TargetUsage & 0x07) == TARGETUSAGE_BEST_SPEED) &&
        (m_avcSeqParam->FrameWidth >= m_singlePassMinFrameWidth) &&
        (m_avcSeqParam->FrameHeight >= m_singlePassMinFrameHeight) &&
        (m_avcSeqParam->FramesPer100Sec >= m_singlePassMinFramePer100s))
    {
        hucVDEncBrcInitDmem->INIT_SinglePassOnly = true;
    }

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
        CODECHAL_ENCODE_CHK_STATUS_RETURN(PopulateBrcInitParam(
            hucVDEncBrcInitDmem));
    )

    m_osInterface->pfnUnlockResource(m_osInterface, &m_resVdencBrcInitDmemBuffer[m_currRecycledBufIdx]);

    return eStatus;
}

MOS_STATUS CodechalVdencAvcStateG12::SetDmemHuCBrcUpdate()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    // Program update DMEM
    MOS_LOCK_PARAMS lockFlags;
    memset(&lockFlags, 0, sizeof(MOS_LOCK_PARAMS));
    lockFlags.WriteOnly  = 1;
    auto hucVDEncBrcDmem = (PBrcUpdateDmem)m_osInterface->pfnLockResource(
        m_osInterface, &m_resVdencBrcUpdateDmemBuffer[m_currRecycledBufIdx][m_currPass], &lockFlags);
    CODECHAL_ENCODE_CHK_NULL_RETURN(hucVDEncBrcDmem);
    SetDmemHuCBrcUpdateImpl<BrcUpdateDmem>(hucVDEncBrcDmem);

    MOS_LOCK_PARAMS lockFlagsReadOnly;
    MOS_ZeroMemory(&lockFlagsReadOnly, sizeof(MOS_LOCK_PARAMS));
    lockFlagsReadOnly.ReadOnly = 1;
    auto initDmem              = (BrcInitDmem *)m_osInterface->pfnLockResource(
        m_osInterface, &m_resVdencBrcInitDmemBuffer[m_currRecycledBufIdx], &lockFlagsReadOnly);

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

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(PopulateBrcUpdateParam(
            hucVDEncBrcDmem));
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

MOS_STATUS CodechalVdencAvcStateG12::CalculateVdencPictureStateCommandSize()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    MHW_VDBOX_STATE_CMDSIZE_PARAMS_G12 stateCmdSizeParams;
    uint32_t vdencPictureStatesSize, vdencPicturePatchListSize;
    m_hwInterface->GetHxxStateCommandSize(
        CODECHAL_ENCODE_MODE_AVC,
        (uint32_t*)&vdencPictureStatesSize,
        (uint32_t*)&vdencPicturePatchListSize,
        &stateCmdSizeParams);

    m_pictureStatesSize += vdencPictureStatesSize;
    m_picturePatchListSize += vdencPicturePatchListSize;

    m_hwInterface->GetVdencStateCommandsDataSize(
        CODECHAL_ENCODE_MODE_AVC,
        (uint32_t*)&vdencPictureStatesSize,
        (uint32_t*)&vdencPicturePatchListSize);

    m_pictureStatesSize += vdencPictureStatesSize;
    m_picturePatchListSize += vdencPicturePatchListSize;

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

void CodechalVdencAvcStateG12::SetMfxAvcImgStateParams(MHW_VDBOX_AVC_IMG_PARAMS& param)
{
    PMHW_VDBOX_AVC_IMG_PARAMS_G12 paramsG12 = static_cast<PMHW_VDBOX_AVC_IMG_PARAMS_G12>(&param);

    CodechalEncodeAvcBase::SetMfxAvcImgStateParams(param);
    if (m_avcSeqParam->EnableSliceLevelRateCtrl)
    {
        param.dwMbSlcThresholdValue = m_mbSlcThresholdValue;
        param.dwSliceThresholdTable = m_sliceThresholdTable;
        param.dwVdencSliceMinusBytes = (m_pictureCodingType == I_TYPE) ?
            m_vdencSliceMinusI : m_vdencSliceMinusP;
    }

    param.bVdencEnabled = true;
    param.pVDEncModeCost = m_vdencModeCostTbl;
    param.pVDEncHmeMvCost = m_vdencHmeMvCostTbl;
    param.pVDEncMvCost = m_vdencMvCostTbl;
    param.bVDEncPerfModeEnabled =
        m_vdencInterface->IsPerfModeSupported() && m_perfModeEnabled[m_avcSeqParam->TargetUsage];
    paramsG12->bVDEncUltraModeEnabled = m_vdencUltraModeEnable;
    if (((m_avcSeqParam->TargetUsage & 0x07) == TARGETUSAGE_BEST_SPEED) &&
        (m_avcSeqParam->FrameWidth >= m_singlePassMinFrameWidth) &&
        (m_avcSeqParam->FrameHeight >= m_singlePassMinFrameHeight) &&
        (m_avcSeqParam->FramesPer100Sec >=m_singlePassMinFramePer100s))
    {
        paramsG12->bVDEncUltraModeEnabled = true;
    }
    paramsG12->oneOnOneMapping = m_oneOnOneMapping;
}

PMHW_VDBOX_STATE_CMDSIZE_PARAMS CodechalVdencAvcStateG12::CreateMhwVdboxStateCmdsizeParams()
{
    PMHW_VDBOX_STATE_CMDSIZE_PARAMS cmdSizeParams = MOS_New(MHW_VDBOX_STATE_CMDSIZE_PARAMS_G12);

    return cmdSizeParams;
}

PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS CodechalVdencAvcStateG12::CreateMhwVdboxPipeModeSelectParams()
{
    PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS pipeModeSelectParams = MOS_New(MHW_VDBOX_PIPE_MODE_SELECT_PARAMS_G12);

    return pipeModeSelectParams;
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
    m_hmeKernel = MOS_New(CodechalKernelHmeG12, this);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hmeKernel);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hmeKernel->Initialize(
        GetCommonKernelHeaderAndSizeG12,
        m_kernelBase,
        m_kuidCommon));
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalVdencAvcStateG12::ExecuteMeKernel()
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
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalVdencAvcStateG12::UpdateCmdBufAttribute(
    PMOS_COMMAND_BUFFER cmdBuffer,
    bool                renderEngineInUse)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    // should not be there. Will remove it in the next change
    CODECHAL_ENCODE_FUNCTION_ENTER;
    if (MOS_VE_SUPPORTED(m_osInterface))
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
#endif
