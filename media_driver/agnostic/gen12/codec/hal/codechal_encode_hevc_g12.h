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
//! \file     codechal_encode_hevc_g12.h
//! \brief    HEVC dual-pipe encoder for GEN12 platform.
//!

#ifndef __CODECHAL_ENCODE_HEVC_G12_H__
#define __CODECHAL_ENCODE_HEVC_G12_H__

#include "codechal_encode_hevc.h"
#include "codechal_encode_sw_scoreboard_g12.h"
#include "codechal_encode_sw_scoreboard_mdf_g12.h"
#include "codechal_encode_wp_mdf_g12.h"
#include "mhw_vdbox_g12_X.h"
#include "codechal_kernel_intra_dist.h"
#include "codechal_encode_hevc_brc_g12.h"
#include "codechal_encode_singlepipe_virtualengine.h"
#include "codechal_encode_scalability.h"

#define  VDBOX_HUC_PAK_INTEGRATION_KERNEL_DESCRIPTOR 15
#define  BRC_IMG_STATE_SIZE_PER_PASS_G12             192
#define  HEVC_BRC_LONG_TERM_REFRENCE_FLAG            0x8000
#define  MAX_CONCURRENT_GROUP                        4

//!
//! \struct HucPakStitchDmemEncG12
//! \brief  The struct of Huc Com Dmem
//!
struct HucPakStitchDmemEncG12
{
    uint32_t     TileSizeRecord_offset[5];  // Tile Size Records, start offset  in byte, 0xffffffff means unavailable
    uint32_t     VDENCSTAT_offset[5];      // needed for HEVC VDEnc, VP9 VDEnc, start offset  in byte, 0xffffffff means unavailable
    uint32_t     HEVC_PAKSTAT_offset[5]; //needed for HEVC VDEnc, start offset  in byte, 0xffffffff means unavailable
    uint32_t     HEVC_Streamout_offset[5]; //needed for HEVC VDEnc, start offset  in byte, 0xffffffff means unavailable
    uint32_t     VP9_PAK_STAT_offset[5]; //needed for VP9 VDEnc, start offset  in byte, 0xffffffff means unavailable
    uint32_t     Vp9CounterBuffer_offset[5];    //needed for VP9 VDEnc, start offset  in byte, 0xffffffff means unavailable
    uint32_t     LastTileBS_StartInBytes;// last tile in bitstream for region 4 and region 5
    uint32_t     SliceHeaderSizeinBits;// needed for HEVC dual pipe BRC
    uint16_t     TotalSizeInCommandBuffer; // Total size in bytes of valid data in the command buffer
    uint16_t     OffsetInCommandBuffer; // Byte  offset of the to-be-updated Length (uint32_t) in the command buffer, 0xffff means unavailable
    uint16_t     PicWidthInPixel;   // Picture width in pixel
    uint16_t     PicHeightInPixel;  // Picture hieght in pixel
    uint16_t     TotalNumberOfPAKs; // [2..4]
    uint16_t     NumSlices[4];  // this is number of slices from each PAK
    uint16_t     NumTiles[4];  // this is number of tiles from each PAK
    uint16_t     PIC_STATE_StartInBytes;// offset for  region 7 and region 8
    uint8_t      Codec;             // 1: HEVC DP; 2: HEVC VDEnc; 3: VP9 VDEnc
    uint8_t      MAXPass;           // Max number of BRC pass >=1
    uint8_t      CurrentPass;       // Current BRC pass [1..MAXPass]
    uint8_t      MinCUSize;      // Minimum CU size (3: 8x8, 4:16x16), HEVC only.
    uint8_t      CabacZeroWordFlag; // cabac zero flag, HEVC only
    uint8_t      bitdepth_luma;     // luma bitdepth, HEVC only
    uint8_t      bitdepth_chroma;   // chroma bitdepth, HEVC only
    uint8_t      ChromaFormatIdc;   // chroma format idc, HEVC only
    uint8_t      currFrameBRClevel;  // Hevc dual pipe only
    uint8_t      brcUnderFlowEnable; // Hevc dual pipe only    
    uint8_t      StitchEnable;// enable stitch cmd for Hevc dual pipe
    uint8_t      reserved1;
    uint16_t     StitchCommandOffset; // offset in region 10 which is the second level batch buffer
    uint16_t     reserved2;
    uint32_t     BBEndforStitch;
    uint8_t      RSVD[16];    //mbz
};

//!
//! \struct HucInputCmdG12
//! \brief  The struct of Huc input command
//!
struct HucInputCmdG12
{
    uint8_t  SelectionForIndData    = 0;
    uint8_t  CmdMode                = 0;
    uint16_t LengthOfTable          = 0;

    uint32_t SrcBaseOffset          = 0;
    uint32_t DestBaseOffset         = 0;

    uint32_t Reserved[3]            = { 0 };

    uint32_t CopySize               = 0;    // use this as indicator of size for copy base addr cmd. Since encode will not implement CopySize for copy cmd

    uint32_t ReservedCounter[4]     = {0};

    uint32_t SrcAddrBottom          = 0;
    uint32_t SrcAddrTop             = 0;
    uint32_t DestAddrBottom         = 0;
    uint32_t DestAddrTop            = 0;
};

//!  HEVC dual-pipe encoder class for GEN12
/*!
This class defines the member fields, functions for GEN12 platform
*/
class CodechalEncHevcStateG12 : public CodechalEncHevcState
{
public:

    //!< Constants for mode bits look-up-tables
    enum
    {
        LUTMODEBITS_INTRA_64X64       = 0x00,
        LUTMODEBITS_INTRA_32X32       = 0x01,
        LUTMODEBITS_INTRA_16X16       = 0x02,
        LUTMODEBITS_INTRA_8X8         = 0x03,
        LUTMODEBITS_INTRA_NXN         = 0x04,
        LUTMODEBITS_INTRA_MPM         = 0x07,
        LUTMODEBITS_INTRA_CHROMA      = 0x08,
        LUTMODEBITS_INTRA_DC_32X32    = 0x09,
        LUTMODEBITS_INTRA_DC_8X8      = 0x0A,
        LUTMODEBITS_INTRA_NONDC_32X32 = 0x0B,
        LUTMODEBITS_INTRA_NONDC_16X16 = 0x0C, // only used by CRE
        LUTMODEBITS_INTRA_NONDC_8X8   = 0x0D,
        LUTMODEBITS_INTER_64X64       = 0x0E, // only used by Kernel
        LUTMODEBITS_INTER_64X32       = 0x0F,
        LUTMODEBITS_INTER_32X64       = 0x0F,
        LUTMODEBITS_INTER_32X32       = 0x10,
        LUTMODEBITS_INTER_32X16       = 0x11,
        LUTMODEBITS_INTER_16X32       = 0x11,
        LUTMODEBITS_INTER_AMP         = 0x11,
        LUTMODEBITS_INTER_16X8        = 0x13,
        LUTMODEBITS_INTER_8X16        = 0x13,
        LUTMODEBITS_INTER_8X8         = 0x14,
        LUTMODEBITS_INTER_BIDIR       = 0x15,
        LUTMODEBITS_INTER_REFID       = 0x16,
        LUTMODEBITS_MERGE_64X64       = 0x17,
        LUTMODEBITS_MERGE_32X32       = 0x18,
        LUTMODEBITS_MERGE_16X16       = 0x19,
        LUTMODEBITS_MERGE_8X8         = 0x1A,
        LUTMODEBITS_INTER_SKIP        = 0x1B, // only used by CRE
        LUTMODEBITS_SKIP_64X64        = 0x1C,
        LUTMODEBITS_SKIP_32X32        = 0x1D,
        LUTMODEBITS_SKIP_16X16        = 0x1E,
        LUTMODEBITS_SKIP_8X8          = 0x1F,
        LUTMODEBITS_TU_DEPTH_0        = 0x23, // shared by HEVC & VP9
        LUTMODEBITS_TU_DEPTH_1        = 0x24, // shared by HEVC & VP9
        LUTMODEBITS_INTER_16X16       = 0x12,
        LUTMODEBITS_CBF               = 0x26,
        LUTMODEBITS_INTRA_CBF_32X32   = LUTMODEBITS_CBF + 0,
        LUTMODEBITS_INTRA_CBF_16X16   = LUTMODEBITS_CBF + 1,
        LUTMODEBITS_INTRA_CBF_8X8     = LUTMODEBITS_CBF + 2,
        LUTMODEBITS_INTRA_CBF_4X4     = LUTMODEBITS_CBF + 3,
        LUTMODEBITS_INTER_CBF_32X32   = LUTMODEBITS_CBF + 4,
        LUTMODEBITS_INTER_CBF_16X16   = LUTMODEBITS_CBF + 5,
        LUTMODEBITS_INTER_CBF_8X8     = LUTMODEBITS_CBF + 6,
        LUTMODEBITS_INTER_CBF_4X4     = LUTMODEBITS_CBF + 7,
        NUM_LUTMODEBITS               = 46
    };

    //!< Constants for CRE costing look-up-tables
    enum
    {
        LUTCREMODE_INTRA_NONPRED     = 0x00, // MPM
        LUTCREMODE_INTRA_32X32       = 0x01,
        LUTCREMODE_INTRA_16X16       = 0x02,
        LUTCREMODE_INTRA_8X8         = 0x03,
        LUTCREMODE_INTER_32X16       = 0x04,
        LUTCREMODE_INTER_16X32       = 0x04,
        LUTCREMODE_INTER_AMP         = 0x04,
        LUTCREMODE_INTER_16X16       = 0x05,
        LUTCREMODE_INTER_16X8        = 0x06,
        LUTCREMODE_INTER_8X16        = 0x06,
        LUTCREMODE_INTER_8X8         = 0x07,
        LUTCREMODE_INTER_32X32       = 0x08,
        LUTCREMODE_INTER_BIDIR       = 0x09,
        LUTCREMODE_REF_ID            = 0x0A,
        LUTCREMODE_INTRA_CHROMA      = 0x0B,
        LUTCREMODE_INTER_SKIP        = 0x0C,
        LUTCREMODE_INTRA_NONDC_32X32 = 0x0D,
        LUTCREMODE_INTRA_NONDC_16X16 = 0x0E,
        LUTCREMODE_INTRA_NONDC_8X8   = 0x0F,
        NUM_LUTCREMODE               = 16
    };

    //!< Constants for RDE costing look-up-tables
    enum
    {
        LUTRDEMODE_INTRA_64X64       = 0x00,
        LUTRDEMODE_INTRA_32X32       = 0x01,
        LUTRDEMODE_INTRA_16X16       = 0x02,
        LUTRDEMODE_INTRA_8X8         = 0x03,
        LUTRDEMODE_INTRA_NXN         = 0x04,
        LUTRDEMODE_INTRA_MPM         = 0x07,
        LUTRDEMODE_INTRA_DC_32X32    = 0x08,
        LUTRDEMODE_INTRA_DC_8X8      = 0x09,
        LUTRDEMODE_INTRA_NONDC_32X32 = 0x0A,
        LUTRDEMODE_INTRA_NONDC_8X8   = 0x0B,
        LUTRDEMODE_INTER_BIDIR       = 0x0C,
        LUTRDEMODE_INTER_REFID       = 0x0D,
        LUTRDEMODE_SKIP_64X64        = 0x0E,
        LUTRDEMODE_SKIP_32X32        = 0x0F,
        LUTRDEMODE_SKIP_16X16        = 0x10,
        LUTRDEMODE_SKIP_8X8          = 0x11,
        LUTRDEMODE_MERGE_64X64       = 0x12,
        LUTRDEMODE_MERGE_32X32       = 0x13,
        LUTRDEMODE_MERGE_16X16       = 0x14,
        LUTRDEMODE_MERGE_8X8         = 0x15,
        LUTRDEMODE_INTER_32X32       = 0x16,
        LUTRDEMODE_INTER_32X16       = 0x17,
        LUTRDEMODE_INTER_16X32       = 0x17,
        LUTRDEMODE_INTER_AMP         = 0x17,
        LUTRDEMODE_INTER_16X16       = 0x18,
        LUTRDEMODE_INTER_16X8        = 0x19,
        LUTRDEMODE_INTER_8X16        = 0x19,
        LUTRDEMODE_INTER_8X8         = 0x1A,
        LUTRDEMODE_TU_DEPTH_0        = 0x1E,
        LUTRDEMODE_TU_DEPTH_1        = 0x1F,
        LUTRDEMODE_CBF               = 0x21,
        LUTRDEMODE_INTRA_CBF_32X32   = LUTRDEMODE_CBF+0,
        LUTRDEMODE_INTRA_CBF_16X16   = LUTRDEMODE_CBF+1,
        LUTRDEMODE_INTRA_CBF_8X8     = LUTRDEMODE_CBF+2,
        LUTRDEMODE_INTRA_CBF_4X4     = LUTRDEMODE_CBF+3,
        LUTRDEMODE_INTER_CBF_32X32   = LUTRDEMODE_CBF+4,
        LUTRDEMODE_INTER_CBF_16X16   = LUTRDEMODE_CBF+5,
        LUTRDEMODE_INTER_CBF_8X8     = LUTRDEMODE_CBF+6,
        LUTRDEMODE_INTER_CBF_4X4     = LUTRDEMODE_CBF+7,
        NUM_LUTRDEMODE               = 41,
    };

    //!< MBENC kernel index
    enum
    {
        MBENC_LCU32_KRNIDX = 0,
        MBENC_LCU64_KRNIDX = 1,
        MBENC_NUM_KRN
    };

    //!< Binding table offset
    enum
    {
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
        BRC_LCU_UPDATE_ROI,
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
        MBENC_I_FRAME_CURR_Y_WITH_RECON_BOUNDARY_PIX,
        MBENC_I_FRAME_INTERMEDIATE_CU_RECORD,
        MBENC_I_FRAME_PAK_OBJ,
        MBENC_I_FRAME_PAK_CU_RECORD,
        MBENC_I_FRAME_SW_SCOREBOARD,
        MBENC_I_FRAME_SCRATCH_SURFACE,
        MBENC_I_FRAME_CU_QP_DATA,
        MBENC_I_FRAME_LCU_LEVEL_DATA_INPUT,
        MBENC_I_FRAME_ENC_CONST_TABLE,
        MBENC_I_FRAME_CONCURRENT_TG_DATA,
        MBENC_I_FRAME_BRC_COMBINED_ENC_PARAMETER_SURFACE,
        MBENC_I_FRAME_DEBUG_DUMP,
        MBENC_I_FRAME_END,

        //MBEnc B-Kernel
        MBENC_B_FRAME_BEGIN = 0,
        MBENC_B_FRAME_ENCODER_COMBINED_BUFFER1 = MBENC_B_FRAME_BEGIN,
        MBENC_B_FRAME_ENCODER_COMBINED_BUFFER2,
        MBENC_B_FRAME_VME_PRED_CURR_PIC_IDX0,
        MBENC_B_FRAME_VME_PRED_FWD_PIC_IDX0,
        MBENC_B_FRAME_VME_PRED_BWD_PIC_IDX0,
        MBENC_B_FRAME_VME_PRED_FWD_PIC_IDX1,
        MBENC_B_FRAME_VME_PRED_BWD_PIC_IDX1,
        MBENC_B_FRAME_VME_PRED_FWD_PIC_IDX2,
        MBENC_B_FRAME_VME_PRED_BWD_PIC_IDX2,
        MBENC_B_FRAME_VME_PRED_FWD_PIC_IDX3,
        MBENC_B_FRAME_VME_PRED_BWD_PIC_IDX3,
        MBENC_B_FRAME_CURR_Y,
        MBENC_B_FRAME_CURR_UV,
        MBENC_B_FRAME_CURR_Y_WITH_RECON_BOUNDARY_PIX,
        MBENC_B_FRAME_ENC_CU_RECORD,
        MBENC_B_FRAME_PAK_OBJ,
        MBENC_B_FRAME_PAK_CU_RECORD,
        MBENC_B_FRAME_SW_SCOREBOARD,
        MBENC_B_FRAME_SCRATCH_SURFACE,
        MBENC_B_FRAME_CU_QP_DATA,
        MBENC_B_FRAME_LCU_LEVEL_DATA_INPUT,
        MBENC_B_FRAME_ENC_CONST_TABLE,
        MBENC_B_FRAME_COLOCATED_CU_MV_DATA,
        MBENC_B_FRAME_HME_MOTION_PREDICTOR_DATA,
        MBENC_B_FRAME_CONCURRENT_TG_DATA,
        MBENC_B_FRAME_BRC_COMBINED_ENC_PARAMETER_SURFACE,
        MBENC_B_FRAME_VME_PRED_FOR_2X_DS_CURR,
        MBENC_B_FRAME_VME_PRED_FOR_2X_DS_FWD_PIC_IDX0,
        MBENC_B_FRAME_VME_PRED_FOR_2X_DS_BWD_PIC_IDX0,
        MBENC_B_FRAME_VME_PRED_FOR_2X_DS_FWD_PIC_IDX1,
        MBENC_B_FRAME_VME_PRED_FOR_2X_DS_BWD_PIC_IDX1,
        MBENC_B_FRAME_VME_PRED_FOR_2X_DS_FWD_PIC_IDX2,
        MBENC_B_FRAME_VME_PRED_FOR_2X_DS_BWD_PIC_IDX2,
        MBENC_B_FRAME_VME_PRED_FOR_2X_DS_FWD_PIC_IDX3,
        MBENC_B_FRAME_VME_PRED_FOR_2X_DS_BWD_PIC_IDX3,
        MBENC_B_FRAME_ENCODER_HISTORY_INPUT_BUFFER,
        MBENC_B_FRAME_ENCODER_HISTORY_OUTPUT_BUFFER,
        MBENC_B_FRAME_DEBUG_SURFACE,
        MBENC_B_FRAME_DEBUG_SURFACE1,
        MBENC_B_FRAME_DEBUG_SURFACE2,
        MBENC_B_FRAME_DEBUG_SURFACE3,
        MBENC_B_FRAME_END,
    };

    //!< Constants for TU based params
    enum
    {
        IntraSpotCheckFlagTuParam,
        EnableCu64CheckTuParam,
        DynamicOrderThTuParam,
        Dynamic64ThTuParam,
        Dynamic64OrderTuParam,
        Dynamic64EnableTuParam,
        IncreaseExitThreshTuParam,
        Log2TUMaxDepthInterTuParam,
        Log2TUMaxDepthIntraTuParam,
        MaxNumIMESearchCenterTuParam,
        Fake32EnableTuParam,
        Dynamic64Min32,
        TotalTuParams
    };

//! \cond SKIP_DOXYGEN
    //! Kernel Header structure
    struct CODECHAL_HEVC_KERNEL_HEADER
    {
        int nKernelCount;
        union
        {
            struct
            {
                CODECHAL_KERNEL_HEADER HEVC_Enc_LCU32;
                CODECHAL_KERNEL_HEADER HEVC_Enc_LCU64;
                CODECHAL_KERNEL_HEADER HEVC_brc_init;
                CODECHAL_KERNEL_HEADER HEVC_brc_reset;
                CODECHAL_KERNEL_HEADER HEVC_brc_update;
                CODECHAL_KERNEL_HEADER HEVC_brc_lcuqp;
            };
        };
    };
    using PCODECHAL_HEVC_KERNEL_HEADER = CODECHAL_HEVC_KERNEL_HEADER*;

    static const uint32_t MAX_MULTI_FRAME_NUMBER = 4;

    //! MBENC LCU32 kernel BTI structure
    struct MBENC_LCU32_BTI
    {
        uint32_t Combined1DSurIndexMF1[MAX_MULTI_FRAME_NUMBER];
        uint32_t Combined1DSurIndexMF2[MAX_MULTI_FRAME_NUMBER];
        uint32_t VMEInterPredictionSurfIndexMF[MAX_MULTI_FRAME_NUMBER];
        uint32_t SrcSurfIndexMF[MAX_MULTI_FRAME_NUMBER];
        uint32_t SrcReconSurfIndexMF[MAX_MULTI_FRAME_NUMBER];
        uint32_t CURecordSurfIndexMF[MAX_MULTI_FRAME_NUMBER];
        uint32_t PAKObjectSurfIndexMF[MAX_MULTI_FRAME_NUMBER];
        uint32_t CUPacketSurfIndexMF[MAX_MULTI_FRAME_NUMBER];
        uint32_t SWScoreBoardSurfIndexMF[MAX_MULTI_FRAME_NUMBER];
        uint32_t QPCU16SurfIndexMF[MAX_MULTI_FRAME_NUMBER];
        uint32_t LCULevelDataSurfIndexMF[MAX_MULTI_FRAME_NUMBER];
        uint32_t TemporalMVSurfIndexMF[MAX_MULTI_FRAME_NUMBER];
        uint32_t HmeDataSurfIndexMF[MAX_MULTI_FRAME_NUMBER];
        uint32_t HEVCCnstLutSurfIndex;
        uint32_t LoadBalenceSurfIndex;
        uint32_t ReservedBTI0;
        uint32_t ReservedBTI1;
        uint32_t DebugSurfIndexMF[MAX_MULTI_FRAME_NUMBER];
    };
    using PMBENC_LCU32_BTI = MBENC_LCU32_BTI*;

    //! MBENC LCU64 kernel BTI structure
    struct MBENC_LCU64_BTI
    {
        uint32_t Combined1DSurIndexMF1[MAX_MULTI_FRAME_NUMBER];
        uint32_t Combined1DSurIndexMF2[MAX_MULTI_FRAME_NUMBER];
        uint32_t VMEInterPredictionSurfIndexMF[MAX_MULTI_FRAME_NUMBER];
        uint32_t SrcSurfIndexMF[MAX_MULTI_FRAME_NUMBER];
        uint32_t SrcReconSurfIndexMF[MAX_MULTI_FRAME_NUMBER];
        uint32_t CURecordSurfIndexMF[MAX_MULTI_FRAME_NUMBER];
        uint32_t PAKObjectSurfIndexMF[MAX_MULTI_FRAME_NUMBER];
        uint32_t CUPacketSurfIndexMF[MAX_MULTI_FRAME_NUMBER];
        uint32_t SWScoreBoardSurfIndexMF[MAX_MULTI_FRAME_NUMBER];
        uint32_t QPCU16SurfIndexMF[MAX_MULTI_FRAME_NUMBER];
        uint32_t LCULevelDataSurfIndexMF[MAX_MULTI_FRAME_NUMBER];
        uint32_t TemporalMVSurfIndexMF[MAX_MULTI_FRAME_NUMBER];
        uint32_t HmeDataSurfIndexMF[MAX_MULTI_FRAME_NUMBER];
        uint32_t VME2XInterPredictionSurfIndexMF[MAX_MULTI_FRAME_NUMBER];
        uint32_t HEVCCnstLutSurfIndex;
        uint32_t LoadBalenceSurfIndex;
        uint32_t DebugSurfIndexMF[MAX_MULTI_FRAME_NUMBER];
    };
    using PMBENC_LCU64_BTI = MBENC_LCU64_BTI*;

    struct MBENC_COMBINED_BTI
    {
        MBENC_COMBINED_BTI()
        {
            MOS_ZeroMemory(this, sizeof(*this));
        }
        union
        {
            MBENC_LCU32_BTI BTI_LCU32;
            MBENC_LCU64_BTI BTI_LCU64;
        };
    };

    //! BRC Init/Reset kernel Curbe structure
    struct BRC_INITRESET_CURBE
    {
        // uint32_t 0
        uint32_t DW0_ProfileLevelMaxFrame : MOS_BITFIELD_RANGE(0, 31);

        // uint32_t 1
        uint32_t DW1_InitBufFull : MOS_BITFIELD_RANGE(0, 31);

        // uint32_t 2
        uint32_t DW2_BufSize : MOS_BITFIELD_RANGE(0, 31);

        // uint32_t 3
        uint32_t DW3_TargetBitRate : MOS_BITFIELD_RANGE(0, 31);

        // uint32_t 4
        uint32_t DW4_MaximumBitRate : MOS_BITFIELD_RANGE(0, 31);

        // uint32_t 5
        uint32_t DW5_MinimumBitRate : MOS_BITFIELD_RANGE(0, 31);

        // uint32_t 6
        uint32_t DW6_FrameRateM : MOS_BITFIELD_RANGE(0, 31);

        // uint32_t 7
        uint32_t DW7_FrameRateD : MOS_BITFIELD_RANGE(0, 31);

        // uint32_t 8
        uint32_t DW8_BRCFlag : MOS_BITFIELD_RANGE(0, 15);
        uint32_t DW8_BRCGopP : MOS_BITFIELD_RANGE(16, 31);

        // uint32_t 9
        uint32_t DW9_BRCGopB : MOS_BITFIELD_RANGE(0, 15);
        uint32_t DW9_FrameWidth : MOS_BITFIELD_RANGE(16, 31);

        // uint32_t 10
        uint32_t DW10_FrameHeight : MOS_BITFIELD_RANGE(0, 15);
        uint32_t DW10_AVBRAccuracy : MOS_BITFIELD_RANGE(16, 31);

        // uint32_t 11
        uint32_t DW11_AVBRConvergence : MOS_BITFIELD_RANGE(0, 15);
        uint32_t DW11_MinimumQP : MOS_BITFIELD_RANGE(16, 31);

        // uint32_t 12
        uint32_t DW12_MaximumQP : MOS_BITFIELD_RANGE(0, 15);
        uint32_t DW12_NumberSlice : MOS_BITFIELD_RANGE(16, 31);

        // uint32_t 13
        uint32_t DW13_Reserved_0 : MOS_BITFIELD_RANGE(0, 15);
        uint32_t DW13_BRCGopB1 : MOS_BITFIELD_RANGE(16, 31);

        // uint32_t 14
        uint32_t DW14_BRCGopB2 : MOS_BITFIELD_RANGE(0, 15);
        uint32_t DW14_MaxBRCLevel : MOS_BITFIELD_RANGE(16, 31);

        // uint32_t 15
        uint32_t DW15_LongTermInterval : MOS_BITFIELD_RANGE(0, 15);
        uint32_t DW15_Reserved_0 : MOS_BITFIELD_RANGE(16, 31);

        // uint32_t 16
        uint32_t DW16_InstantRateThreshold0_Pframe : MOS_BITFIELD_RANGE(0, 7);
        uint32_t DW16_InstantRateThreshold1_Pframe : MOS_BITFIELD_RANGE(8, 15);
        uint32_t DW16_InstantRateThreshold2_Pframe : MOS_BITFIELD_RANGE(16, 23);
        uint32_t DW16_InstantRateThreshold3_Pframe : MOS_BITFIELD_RANGE(24, 31);

        // uint32_t 17
        uint32_t DW17_InstantRateThreshold0_Bframe : MOS_BITFIELD_RANGE(0, 7);
        uint32_t DW17_InstantRateThreshold1_Bframe : MOS_BITFIELD_RANGE(8, 15);
        uint32_t DW17_InstantRateThreshold2_Bframe : MOS_BITFIELD_RANGE(16, 23);
        uint32_t DW17_InstantRateThreshold3_Bframe : MOS_BITFIELD_RANGE(24, 31);

        // uint32_t 18
        uint32_t DW18_InstantRateThreshold0_Iframe : MOS_BITFIELD_RANGE(0, 7);
        uint32_t DW18_InstantRateThreshold1_Iframe : MOS_BITFIELD_RANGE(8, 15);
        uint32_t DW18_InstantRateThreshold2_Iframe : MOS_BITFIELD_RANGE(16, 23);
        uint32_t DW18_InstantRateThreshold3_Iframe : MOS_BITFIELD_RANGE(24, 31);

        // uint32_t 19
        uint32_t DW19_DeviationThreshold0_PBframe : MOS_BITFIELD_RANGE(0, 7);
        uint32_t DW19_DeviationThreshold1_PBframe : MOS_BITFIELD_RANGE(8, 15);
        uint32_t DW19_DeviationThreshold2_PBframe : MOS_BITFIELD_RANGE(16, 23);
        uint32_t DW19_DeviationThreshold3_PBframe : MOS_BITFIELD_RANGE(24, 31);

        // uint32_t 20
        uint32_t DW20_DeviationThreshold4_PBframe : MOS_BITFIELD_RANGE(0, 7);
        uint32_t DW20_DeviationThreshold5_PBframe : MOS_BITFIELD_RANGE(8, 15);
        uint32_t DW20_DeviationThreshold6_PBframe : MOS_BITFIELD_RANGE(16, 23);
        uint32_t DW20_DeviationThreshold7_PBframe : MOS_BITFIELD_RANGE(24, 31);

        // uint32_t 21
        uint32_t DW21_DeviationThreshold0_VBRcontrol : MOS_BITFIELD_RANGE(0, 7);
        uint32_t DW21_DeviationThreshold1_VBRcontrol : MOS_BITFIELD_RANGE(8, 15);
        uint32_t DW21_DeviationThreshold2_VBRcontrol : MOS_BITFIELD_RANGE(16, 23);
        uint32_t DW21_DeviationThreshold3_VBRcontrol : MOS_BITFIELD_RANGE(24, 31);

        // uint32_t 22
        uint32_t DW22_DeviationThreshold4_VBRcontrol : MOS_BITFIELD_RANGE(0, 7);
        uint32_t DW22_DeviationThreshold5_VBRcontrol : MOS_BITFIELD_RANGE(8, 15);
        uint32_t DW22_DeviationThreshold6_VBRcontrol : MOS_BITFIELD_RANGE(16, 23);
        uint32_t DW22_DeviationThreshold7_VBRcontrol : MOS_BITFIELD_RANGE(24, 31);

        // uint32_t 23
        uint32_t DW23_DeviationThreshold0_Iframe : MOS_BITFIELD_RANGE(0, 7);
        uint32_t DW23_DeviationThreshold1_Iframe : MOS_BITFIELD_RANGE(8, 15);
        uint32_t DW23_DeviationThreshold2_Iframe : MOS_BITFIELD_RANGE(16, 23);
        uint32_t DW23_DeviationThreshold3_Iframe : MOS_BITFIELD_RANGE(24, 31);

        // uint32_t 24
        uint32_t DW24_DeviationThreshold4_Iframe : MOS_BITFIELD_RANGE(0, 7);
        uint32_t DW24_DeviationThreshold5_Iframe : MOS_BITFIELD_RANGE(8, 15);
        uint32_t DW24_DeviationThreshold6_Iframe : MOS_BITFIELD_RANGE(16, 23);
        uint32_t DW24_DeviationThreshold7_Iframe : MOS_BITFIELD_RANGE(24, 31);

        // uint32_t 25
        uint32_t DW25_ACQPBuffer : MOS_BITFIELD_RANGE(0, 7);
        uint32_t DW25_IntraSADTransform : MOS_BITFIELD_RANGE(8, 15);
        uint32_t DW25_Log2MaxCuSize : MOS_BITFIELD_RANGE(16, 23);
        uint32_t DW25_SlidingWindowSize : MOS_BITFIELD_RANGE(24, 31);

        // uint32_t 26
        uint32_t DW26_BGOPSize : MOS_BITFIELD_RANGE(0, 7);
        uint32_t DW26_RandomAccess : MOS_BITFIELD_RANGE(8, 15);
        uint32_t DW26_Reserved_0 : MOS_BITFIELD_RANGE(16, 31);

        // uint32_t 27
        uint32_t DW27_Reserved_0 : MOS_BITFIELD_RANGE(0, 31);

        // uint32_t 28
        uint32_t DW28_Reserved_0 : MOS_BITFIELD_RANGE(0, 31);

        // uint32_t 29
        uint32_t DW29_Reserved_0 : MOS_BITFIELD_RANGE(0, 31);

        // uint32_t 30
        uint32_t DW30_Reserved_0 : MOS_BITFIELD_RANGE(0, 31);

        // uint32_t 31
        uint32_t DW31_Reserved_0 : MOS_BITFIELD_RANGE(0, 31);

    };
    C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(BRC_INITRESET_CURBE)) == 32);
    using PBRC_INITRESET_CURBE = BRC_INITRESET_CURBE*;

    //! BRC Update kernel Curbe structure
    struct BRCUPDATE_CURBE
    {
        // uint32_t 0
        uint32_t DW0_TargetSize : MOS_BITFIELD_RANGE(0, 31);

        // uint32_t 1
        uint32_t DW1_FrameNumber : MOS_BITFIELD_RANGE(0, 31);

        // uint32_t 2
        uint32_t DW2_PictureHeaderSize : MOS_BITFIELD_RANGE(0, 31);

        // uint32_t 3
        uint32_t DW3_StartGAdjFrame0 : MOS_BITFIELD_RANGE(0, 15);
        uint32_t DW3_StartGAdjFrame1 : MOS_BITFIELD_RANGE(16, 31);

        // uint32_t 4
        uint32_t DW4_StartGAdjFrame2 : MOS_BITFIELD_RANGE(0, 15);
        uint32_t DW4_StartGAdjFrame3 : MOS_BITFIELD_RANGE(16, 31);

        // uint32_t 5
        uint32_t DW5_TargetSize_Flag : MOS_BITFIELD_RANGE(0, 7);
        uint32_t DW5_Reserved_0 : MOS_BITFIELD_RANGE(8, 15);
        uint32_t DW5_MaxNumPAKs : MOS_BITFIELD_RANGE(16, 23);
        uint32_t DW5_CurrFrameBrcLevel : MOS_BITFIELD_RANGE(24, 31);

        // uint32_t 6
        uint32_t   DW6_NumSkippedFrames    : MOS_BITFIELD_RANGE(0, 7);
        uint32_t   DW6_CqpValue            : MOS_BITFIELD_RANGE(8, 15);
        uint32_t   DW6_ROIEnable           : MOS_BITFIELD_RANGE(16, 16);
        uint32_t   DW6_BRCROIEnable        : MOS_BITFIELD_RANGE(17, 17);
        uint32_t   DW6_LowDelayEnable      : MOS_BITFIELD_RANGE(18, 18);
        uint32_t   DW6_Reserved1           : MOS_BITFIELD_RANGE(19, 19);
        uint32_t   DW6_SlidingWindowEnable : MOS_BITFIELD_RANGE(20, 20);
        uint32_t   DW6_Reserved2           : MOS_BITFIELD_RANGE(21, 23);
        uint32_t   DW6_RoiRatio            : MOS_BITFIELD_RANGE(24, 31);

        // uint32_t 7
        uint32_t   DW7_Reserved_0 : MOS_BITFIELD_RANGE(0, 15);
        uint32_t   DW7_FrameMinQP : MOS_BITFIELD_RANGE(16, 23);
        uint32_t   DW7_FrameMaxQP : MOS_BITFIELD_RANGE(24, 31);

        // uint32_t 8
        uint32_t DW8_StartGlobalAdjustMult0 : MOS_BITFIELD_RANGE(0, 7);
        uint32_t DW8_StartGlobalAdjustMult1 : MOS_BITFIELD_RANGE(8, 15);
        uint32_t DW8_StartGlobalAdjustMult2 : MOS_BITFIELD_RANGE(16, 23);
        uint32_t DW8_StartGlobalAdjustMult3 : MOS_BITFIELD_RANGE(24, 31);

        // uint32_t 9
        uint32_t DW9_StartGlobalAdjustMult4 : MOS_BITFIELD_RANGE(0, 7);
        uint32_t DW9_StartGlobalAdjustDivd0 : MOS_BITFIELD_RANGE(8, 15);
        uint32_t DW9_StartGlobalAdjustDivd1 : MOS_BITFIELD_RANGE(16, 23);
        uint32_t DW9_StartGlobalAdjustDivd2 : MOS_BITFIELD_RANGE(24, 31);

        // uint32_t 10
        uint32_t DW10_StartGlobalAdjustDivd3 : MOS_BITFIELD_RANGE(0, 7);
        uint32_t DW10_StartGlobalAdjustDivd4 : MOS_BITFIELD_RANGE(8, 15);
        uint32_t DW10_QPThreshold0 : MOS_BITFIELD_RANGE(16, 23);
        uint32_t DW10_QPThreshold1 : MOS_BITFIELD_RANGE(24, 31);

        // uint32_t 11
        uint32_t DW11_QPThreshold2 : MOS_BITFIELD_RANGE(0, 7);
        uint32_t DW11_QPThreshold3 : MOS_BITFIELD_RANGE(8, 15);
        uint32_t DW11_gRateRatioThreshold0 : MOS_BITFIELD_RANGE(16, 23);
        uint32_t DW11_gRateRatioThreshold1 : MOS_BITFIELD_RANGE(24, 31);

        // uint32_t 12
        uint32_t DW12_gRateRatioThreshold2 : MOS_BITFIELD_RANGE(0, 7);
        uint32_t DW12_gRateRatioThreshold3 : MOS_BITFIELD_RANGE(8, 15);
        uint32_t DW12_gRateRatioThreshold4 : MOS_BITFIELD_RANGE(16, 23);
        uint32_t DW12_gRateRatioThreshold5 : MOS_BITFIELD_RANGE(24, 31);

        // uint32_t 13
        uint32_t DW13_gRateRatioThreshold6 : MOS_BITFIELD_RANGE(0, 7);
        uint32_t DW13_gRateRatioThreshold7 : MOS_BITFIELD_RANGE(8, 15);
        uint32_t DW13_gRateRatioThreshold8 : MOS_BITFIELD_RANGE(16, 23);
        uint32_t DW13_gRateRatioThreshold9 : MOS_BITFIELD_RANGE(24, 31);

        // uint32_t 14
        uint32_t DW14_gRateRatioThreshold10 : MOS_BITFIELD_RANGE(0, 7);
        uint32_t DW14_gRateRatioThreshold11 : MOS_BITFIELD_RANGE(8, 15);
        uint32_t DW14_gRateRatioThreshold12 : MOS_BITFIELD_RANGE(16, 23);
        uint32_t DW14_ParallelMode : MOS_BITFIELD_RANGE(24, 31);

        // uint32_t 15
        uint32_t   DW15_SizeOfSkippedFrames : MOS_BITFIELD_RANGE(0, 31);

        // uint32_t 16
        uint32_t   DW16_UserMaxFrameSize : MOS_BITFIELD_RANGE(0, 31);

        // uint32_t 17
        uint32_t   DW17_LongTerm_Current : MOS_BITFIELD_RANGE(0, 7);
        uint32_t   DW17_Reserved_0 : MOS_BITFIELD_RANGE(8, 31);
        
        // uint32_t 18 - 23 reserved
        uint32_t   DW18_Reserved_0 : MOS_BITFIELD_RANGE(0, 31);
        uint32_t   DW19_Reserved_0 : MOS_BITFIELD_RANGE(0, 31);
        uint32_t   DW20_Reserved_0 : MOS_BITFIELD_RANGE(0, 31);
        uint32_t   DW21_Reserved_0 : MOS_BITFIELD_RANGE(0, 31);
        uint32_t   DW22_Reserved_0 : MOS_BITFIELD_RANGE(0, 31);
        uint32_t   DW23_Reserved_0 : MOS_BITFIELD_RANGE(0, 31);
    };
    C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(BRCUPDATE_CURBE)) == 24);
    using PBRCUPDATE_CURBE = BRCUPDATE_CURBE*;

    //! LCU level data structure
    struct LCU_LEVEL_DATA
    {
        uint16_t SliceStartLcuIndex;
        uint16_t SliceEndLcuIndex;
        uint16_t TileId;
        uint16_t SliceId;
        uint16_t TileStartCoordinateX;
        uint16_t TileStartCoordinateY;
        uint16_t TileEndCoordinateX;
        uint16_t TileEndCoordinateY;
    };
    C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(LCU_LEVEL_DATA)) == 4);
    using PLCU_LEVEL_DATA = LCU_LEVEL_DATA*;

    //! Concurrent thread group data structure
    struct CONCURRENT_THREAD_GROUP_DATA
    {
        uint16_t CurrSliceStartLcuX;
        uint16_t CurrSliceStartLcuY;
        uint16_t CurrSliceEndLcuX;
        uint16_t CurrSliceEndLcuY;
        uint16_t CurrTgStartLcuX;
        uint16_t CurrTgStartLcuY;
        uint16_t CurrTgEndLcuX;
        uint16_t CurrTgEndLcuY;
        uint16_t Reserved[24];
    };
    C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(CONCURRENT_THREAD_GROUP_DATA)) == 16);
    using PCONCURRENT_THREAD_GROUP_DATA = CONCURRENT_THREAD_GROUP_DATA*;

    struct MBENC_CURBE
    {
        MBENC_CURBE()
        {
            MOS_SecureMemcpy(this, sizeof(*this), m_mbencCurbeInit, sizeof(m_mbencCurbeInit));
        }

        //R1.0 //DW0
        union
        {
            uint32_t   R1_0;
            struct
            {
                uint32_t  FrameWidthInSamples : 16;
                uint32_t  FrameHeightInSamples : 16;
            };
        };

        //R1.1 //DW1
        union
        {
            uint32_t   R1_1;
            struct
            {
                uint32_t Log2MaxCUSize : 4;
                uint32_t Log2MinCUSize : 4;
                uint32_t Log2MaxTUSize : 4;
                uint32_t Log2MinTUSize : 4;
                uint32_t MaxNumIMESearchCenter : 3;
                uint32_t MaxIntraRdeIter : 3;
                uint32_t ROIEnable : 1;
                uint32_t QPType : 2;
                uint32_t MaxTransformDepthInter : 2;        //<=    Log2TUMaxDepthInter
                uint32_t MaxTransformDepthIntra : 2;        //<=    Log2TUMaxDepthIntra
                uint32_t Log2ParallelMergeLevel : 3;

            };
        };

        //R1.2    //DW2
        union
        {
            uint32_t   R1_2;
            struct
            {
                uint32_t CornerNeighborPixel : 8;
                uint32_t IntraNeighborAvailFlags : 6;
                uint32_t ChromaFormatType : 2;
                uint32_t SubPelMode : 2;        //Unin R0.3[16:17] SubPelMode
                uint32_t IntraSpotCheck : 2;
                uint32_t InterSADMeasure : 2;   //Unin R0.3[20:21] InterSADMeasure
                uint32_t IntraSADMeasure : 2;   //Unin R0.3[22:23] IntraSADMeasureAdj
                uint32_t IntraPrediction : 3;   //UninR0.1 LumaIntraPartMask
                uint32_t RefIDCostMode : 1;     //UninR0.1[22] RefIDCostMode
                uint32_t TUBasedCostSetting : 3;
                uint32_t MBZ_1_2_1 : 1;
            };
        };

        //R1.3    //DW3
        union
        {
            uint32_t   R1_3;
            struct
            {                                              //UniversalInputSegmentPhase0_2: DW R1.0
                uint32_t    ExplictModeEn : 1;             // [0]
                uint32_t    AdaptiveEn : 1;                // [1]      ImageState.AdaptiveEn
                uint32_t    MBZ_1_3_1 : 3;                 // [4:2]
                uint32_t    EarlyImeSuccessEn : 1;         // [5]      imageState.EarlyImeSuccessEn
                uint32_t    IntraSpeedMode : 1;            // [6]
                uint32_t    IMECostCentersSel : 1;         // [7]      L0/L1

                uint32_t    RDEQuantRoundValue : 8;        // [15:8]   0

                uint32_t    IMERefWindowSize : 2;          // [17:16]  m_ImageState.ImeRefWindowSize
                uint32_t    IntraComputeType : 1;          // [18]     0
                uint32_t    Depth0IntraPredition : 1;      // [19]     0
                uint32_t    TUDepthControl : 2;            // [21:20]
                uint32_t    IntraTuRecFeedbackDisable : 1; // [22]
                uint32_t    MergeListBiDisable : 1;        // [23]

                uint32_t    EarlyImeStop : 8;              // [31:24]  imageState->EarlyImeStopThres
            };
        };

        //R1.4    //DW4
        union
        {
            uint32_t   R1_4;
            struct
            {
                uint32_t FrameQP : 7;
                uint32_t FrameQPSign : 1;
                uint32_t ConcurrentGroupNum : 8;
                uint32_t NumofUnitInWaveFront : 16;
            };
        };

        //R1.5    //DW5
        union
        {
            uint32_t   R1_5;
            struct
            {
                uint32_t LoadBalenceEnable : 1;
                uint32_t NumberofMultiFrame : 3;
                uint32_t MBZ_1_4_1 : 4;
                uint32_t Degree45 : 1;
                uint32_t Break12Dependency : 1;
                uint32_t Fake32Enable : 1;
                uint32_t MBZ_1_4_2 : 5;
                uint32_t ThreadNumber : 8;
                uint32_t MBZ_1_4_3 : 8;
            };
        };

        //R1.6 - R2.7    //DW6 - DW15
        uint32_t Reserved1[10];

        //R3.0    //DW16
        union
        {
            uint32_t   R3_0;
            struct
            {
                uint32_t Pic_init_qp_B : 8;
                uint32_t Pic_init_qp_P : 8;
                uint32_t Pic_init_qp_I : 8;
                uint32_t MBZ_3_0_0 : 8;
            };
        };

        //R3.1    //DW17
        union
        {
            uint32_t   R3_1;
            struct
            {
                uint32_t MBZ_3_1_0 : 16;
                uint32_t NumofRowTile : 8;
                uint32_t NumofColumnTile : 8;
            };
        };

        //R3.2 //DW18
        union
        {
            uint32_t   R3_2;
            struct
            {
                uint32_t TransquantBypassEnableFlag : 1;        //<=    EnableTransquantBypass  (need in Pak data setup)
                uint32_t PCMEnabledFlag : 1;        //<=    EnableIPCM
                uint32_t MBZ_3_2_0 : 2;        //reserved
                uint32_t CuQpDeltaEnabledFlag : 1;        //<=    CuQpDeltaEnabledFlag
                uint32_t Stepping : 2;
                uint32_t WaveFrontSplitsEnable : 1;
                uint32_t HMEFlag : 2;
                uint32_t SuperHME : 1;
                uint32_t UltraHME : 1;
                uint32_t MBZ_3_2_2 : 4;        //reserved
                uint32_t Cu64SkipCheckOnly : 1;
                uint32_t EnableCu64Check : 1;
                uint32_t Cu642Nx2NCheckOnly : 1;
                uint32_t EnableCu64AmpCheck : 1;
                uint32_t MBZ_3_2_3 : 1;        //reserved
                uint32_t DisablePIntra : 1;
                uint32_t DisableIntraTURec : 1;
                uint32_t InheritIntraModeFromTU0 : 1;
                uint32_t MBZ_3_2_4 : 3;        //reserved
                uint32_t CostScalingForRA : 1;
                uint32_t DisableIntraNxN : 1;
                uint32_t MBZ_3_2_5 : 3;        //reserved
            };
        };

        //R3.3 //DW19
        union
        {
            uint32_t   R3_3;
            struct
            {
                uint32_t MaxRefIdxL0 : 8;
                uint32_t MaxRefIdxL1 : 8;
                uint32_t MaxBRefIdxL0 : 8;
                uint32_t MBZ_3_3_0 : 8;
            };
        };

        //R3.4 //DW20
        union
        {
            uint32_t   R3_4;
            struct
            {
                uint32_t SkipEarlyTermination : 2;
                uint32_t SkipEarlyTermSize : 2;
                uint32_t Dynamic64Enable : 2;
                uint32_t Dynamic64Order : 2;
                uint32_t Dynamic64Th : 4;
                uint32_t DynamicOrderTh : 4;
                uint32_t PerBFrameQPOffset : 8;
                uint32_t IncreaseExitThresh : 4;
                uint32_t Dynamic64Min32 : 2;
                uint32_t MBZ_3_4_0 : 1;        //reserved
                uint32_t LastFrameIsIntra : 1;
            };
        };

        //R3.5 //DW21
        union
        {
            uint32_t   R3_5;
            struct
            {
                uint32_t LenSP : 8; //Unin R1.2[16:23] LenSP
                uint32_t MaxNumSU : 8; //Unin R1.2[24:31] MaxNumSU
                uint32_t MBZ_3_5_1 : 16;
            };
        };

        //R3.6 //DW22
        union
        {
            uint32_t   R3_6;
            struct
            {
                uint32_t CostTableIndex : 8;
                uint32_t MBZ_3_6_1 : 24;
            };
        };

        //R3.7    //DW23
        union
        {
            uint32_t   R3_7;
            struct
            {
                uint32_t SliceType : 2;
                uint32_t TemporalMvpEnableFlag : 1;        //<=    EnableTemporalMvp
                uint32_t CollocatedFromL0Flag : 1;
                uint32_t theSameRefList : 1;
                uint32_t IsLowDelay : 1;
                uint32_t DisableTemporal16and8 : 1;
                uint32_t MBZ_3_7_1 : 1;
                uint32_t MaxNumMergeCand : 8;
                uint32_t NumRefIdxL0 : 8;
                uint32_t NumRefIdxL1 : 8;

            };
        };

        //R4.0 //DW24
        union
        {
            uint32_t   R4_0;
            struct
            {
                uint32_t     FwdPocNumber_L0_mTb_0 : 8;
                uint32_t     BwdPocNumber_L1_mTb_0 : 8;
                uint32_t     FwdPocNumber_L0_mTb_1 : 8;
                uint32_t     BwdPocNumber_L1_mTb_1 : 8;
            };
        };

        //R4.1 //DW25
        union
        {
            uint32_t   R4_1;
            struct
            {
                uint32_t     FwdPocNumber_L0_mTb_2 : 8;
                uint32_t     BwdPocNumber_L1_mTb_2 : 8;
                uint32_t     FwdPocNumber_L0_mTb_3 : 8;
                uint32_t     BwdPocNumber_L1_mTb_3 : 8;
            };
        };

        //R4.2 //DW26
        union
        {
            uint32_t   R4_2;
            struct
            {
                uint32_t     FwdPocNumber_L0_mTb_4 : 8;
                uint32_t     BwdPocNumber_L1_mTb_4 : 8;
                uint32_t     FwdPocNumber_L0_mTb_5 : 8;
                uint32_t     BwdPocNumber_L1_mTb_5 : 8;
            };
        };

        //R4.3 //DW27
        union
        {
            uint32_t   R4_3;
            struct
            {
                uint32_t     FwdPocNumber_L0_mTb_6 : 8;
                uint32_t     BwdPocNumber_L1_mTb_6 : 8;
                uint32_t     FwdPocNumber_L0_mTb_7 : 8;
                uint32_t     BwdPocNumber_L1_mTb_7 : 8;
            };
        };

        //R4.4 //DW28
        union
        {
            uint32_t   R4_4;
            struct
            {
                uint32_t     LongTermReferenceFlags_L0 : 16;
                uint32_t     LongTermReferenceFlags_L1 : 16;
            };
        };

        //R4.5 //DW29
        union
        {
            uint32_t   R4_5;
            struct
            {
                uint32_t RefFrameWinWidth : 16;
                uint32_t RefFrameWinHeight : 16;
            };
        };

        //R4.6 //DW30
        union
        {
            uint32_t   R4_6;
            struct
            {
                uint32_t RoundingInter : 8;
                uint32_t RoundingIntra : 8;
                uint32_t MaxThreadWidth : 8;
                uint32_t MaxThreadHeight : 8;
            };
        };

        //R4.7 - R5.7
        uint32_t Reserved2[9];
    };

    static uint32_t const hevcCurbeBufferConstSize = 256;
    C_ASSERT(hevcCurbeBufferConstSize > sizeof(MBENC_CURBE));

    static const uint32_t maxColorBitSupport = 256;

    struct CONCURRENT_THREAD_GROUP_DATA_BUF
    {
        CONCURRENT_THREAD_GROUP_DATA item[maxColorBitSupport];
    };

    struct MBENC_COMBINED_BUFFER1
    {
        MBENC_COMBINED_BUFFER1()
        {
            MOS_ZeroMemory(&(this->concurrent), sizeof(this->concurrent));
        }
        union
        {
            MBENC_CURBE Curbe;
            uint8_t     Data[hevcCurbeBufferConstSize];
        };
        CONCURRENT_THREAD_GROUP_DATA_BUF concurrent;
    };
    using PMBENC_COMBINED_BUFFER1 = MBENC_COMBINED_BUFFER1*;

    static const uint32_t  HEVC_HISTORY_BUF_CONST_SIZE     = 64;
    static const uint32_t  HEVC_FRAMEBRC_BUF_CONST_SIZE    = 1024;

    struct MBENC_COMBINED_BUFFER2
    {
        uint8_t   ucBrcCombinedEncBuffer[HEVC_FRAMEBRC_BUF_CONST_SIZE];
        uint8_t   ucHistoryInBuffer[HEVC_HISTORY_BUF_CONST_SIZE];
    };
    using PMBENC_COMBINED_BUFFER2 = MBENC_COMBINED_BUFFER2*;

    struct CODECHAL_HEVC_VIRTUAL_ENGINE_OVERRIDE
    {
        union {
            uint8_t       VdBox[MOS_MAX_ENGINE_INSTANCE_PER_CLASS];
            uint64_t      Value;
        };
    };

//! \endcond

    static const uint8_t m_maxNumVmeL0Ref     = 4;    //!< Maximum number of L0 references
    static const uint8_t m_maxNumVmeL1Ref     = 4;    //!< Maximum number of L1 references
    static const uint16_t m_maxThreadsPerLcuB = 8;    //!< Maximum number of threads per LCU B
    static const uint16_t m_minThreadsPerLcuB = 3;    //!< Minimum number of threads per LCU B
    static const uint8_t m_maxMultiFrames = 4;        //!< Maximum number of supported multi-frames
    static const uint8_t m_maxMfeSurfaces = 32;       //!< Maximum number of surfaces per mfe stream

    static const uint32_t m_encConstantDataLutSize = 81920;            //!< Constant data LUT size in ints for B-kernel
    static const uint32_t m_brcBufferSize          = 1024;             //!< Size of the BRC buffer for Enc kernel
    static const uint32_t m_debugSurfaceSize       = (8192 * 1024);    //!< 8MB for the debug surface
    static const uint32_t m_maxThreadGprs          = 256;              //!< Maximum number of thread groups
    static const uint32_t m_brcLambdaModeCostTableSize = 416;          //!< Size in DWs of Lambda Mode cost table for BRC
    static const uint32_t m_mvdistSummationSurfSize = 32;              //!< Size of MV distortion summation surface
    static const uint8_t  m_sumMVThreshold = 16;
    uint8_t m_hevcThreadTaskDataNum = 2;
    uint32_t m_maxWavefrontsforTU1 = 2;
    uint32_t m_maxWavefrontsforTU4 = 4;
    static const uint32_t m_loadBalanceSize = (256 * 16);               //!< Load balance size used for load balance array.
    uint32_t m_alignReconFactor = 1;
    uint32_t m_threadMapSize = (256 * 16);                              //!< Thread map surface size will be updated depending on various gen

    static const uint8_t m_meMethod[NUM_TARGET_USAGE_MODES];             //!< ME method
    static const uint8_t m_aStep     = 1;                                //!< A Stepping

    static const uint32_t m_encLcu32ConstantDataLut[m_encConstantDataLutSize/sizeof(uint32_t)];  //!< Constant data table for B kernel
    static const uint32_t m_encLcu64ConstantDataLut[m_encConstantDataLutSize/sizeof(uint32_t)];  //!< Constant data table for B kernel
    static const uint32_t m_brcLcu32x32LambdaModeCostInit[m_brcLambdaModeCostTableSize];         //!< Lambda mode cost table for BRC LCU32x32
    static const uint32_t m_brcLcu64x64LambdaModeCostInit[m_brcLambdaModeCostTableSize];         //!< Lambda mode cost table for BRC LCU64x64

    static const uint32_t m_mbencCurbeInit[40];                 //!< Initialization data for MBENC B kernel
    static const BRC_INITRESET_CURBE m_brcInitResetCurbeInit;   //!< Initialization data for BRC Init/Reset kernel
    static const BRCUPDATE_CURBE m_brcUpdateCurbeInit;          //!< Initialization data for BRC update kernel
    static const uint8_t m_tuSettings[TotalTuParams][3];        //!< Table for TU based settings for different params

    static const double m_modeBits[2][46][3];                   //!< Mode bits LUT based on LCUType/Mode/SliceType
    static const double m_modeBitsScale[46][3];                 //!< Mode bits LUT based on [mode][SliceType]

    MOS_SURFACE             m_currPicWithReconBoundaryPix;      //!< Current Picture with Reconstructed boundary pixels
    MOS_SURFACE             m_lcuLevelInputDataSurface[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM]; //!< In Gen11 Lculevel Data is a 2D surface instead of Buffer
    MOS_SURFACE             m_encoderHistoryInputBuffer;        //!< Encoder History Input Data
    MOS_SURFACE             m_encoderHistoryOutputBuffer;       //!< Encoder History Output Data
    MOS_SURFACE             m_intermediateCuRecordSurfaceLcu32; //!< Intermediate CU Record surface for I and B kernel
    MOS_SURFACE             m_scratchSurface;                   //!< Scratch surface for I-kernel
    MOS_SURFACE             m_16x16QpInputData;                 //!< CU 16x16 QP data input surface
    MOS_RESOURCE            m_SAORowStoreBuffer = {};                //!< SAO RowStore buffer
    CODECHAL_ENCODE_BUFFER  m_debugSurface[4];                  //!< Debug surface used in MBENC kernels
    CODECHAL_ENCODE_BUFFER  m_encConstantTableForB;             //!< Enc constant table for B LCU32
    CODECHAL_ENCODE_BUFFER  m_mvAndDistortionSumSurface;        //!< Mv and Distortion summation surface

    PMHW_VDBOX_HCP_TILE_CODING_PARAMS_G12        m_tileParams = nullptr;       //!< Pointer to the Tile params
    PCODECHAL_ENCODE_SCALABILITY_STATE           m_scalabilityState = nullptr;   //!< Scalability state

    bool        m_colorBitMfeEnabled = false;        //!< enable color bit for MFE: combine frames into one mbenc kernel
    bool        m_multiWalkerEnabled = false;        //!< enable multiple hw walkder
    bool        m_useMdf = false;                    //!< Use MDF for MBEnc kernels.
    bool        m_enableTileStitchByHW = false;      //!< Enable HW to stitch commands in scalable mode
    bool        m_enableHWSemaphore = false;         //!< Enable HW semaphore
    bool        m_weightedPredictionSupported = false;    //!< Enable WP support
    bool        m_useWeightedSurfaceForL0 = false; //!< Flag indicating if L0 Ref using weighted reference frame
    bool        m_useWeightedSurfaceForL1 = false; //!< Flag indicating if L1 Ref using weighted reference frame
    bool        m_sseEnabled = false;                //!< Flag indicating if SSE is enabled in PAK
    bool        m_degree45Needed = false;                  //!< Flag indicating if 45 degree dispatch pattern is used
    bool        m_pakPiplStrmOutEnable = false;
    bool        m_loadKernelInput = false;
    char        m_loadKernelInputDataFolder[MOS_USER_CONTROL_MAX_DATA_SIZE] = {0};    //!< kernel input load from data folder name
    bool        m_HierchGopBRCEnabled                                       = false;   //!< Flag indicating if hierarchical Gop is enabled in BRC

    uint16_t      m_totalNumThreadsPerLcu = 0; //!< Total number of threads per LCU
    uint8_t       m_modeCostRde[42] = { 0 };   //!< RDE cost
    uint8_t       m_modeCostCre[16] = { 0 };   //!< CRE cost
    uint32_t      m_lambdaRD = 0;              //!< Lambda value to multiply the RD  costs

    uint8_t                 m_numberEncKernelSubThread = m_hevcThreadTaskDataNum;
    uint32_t                m_numberConcurrentGroup = MAX_CONCURRENT_GROUP;    // can dividie one picture into several groups
    uint32_t                m_numWavefrontInOneRegion = 0;
    uint16_t                m_lastPictureCodingType = I_TYPE;
    uint8_t*                m_swScoreboard = nullptr;
    bool                    m_useSwInitScoreboard = false;
    CODECHAL_ENCODE_BUFFER  m_encBCombinedBuffer1[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM];
    CODECHAL_ENCODE_BUFFER  m_encBCombinedBuffer2[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM];
    PCODECHAL_ENCODE_BUFFER m_brcInputForEncKernelBuffer = nullptr;
    uint8_t                 m_lastRecycledBufIdx = CODECHAL_ENCODE_RECYCLED_BUFFER_NUM - 1;
    uint32_t                m_historyOutBufferSize = 0;
    uint32_t                m_historyOutBufferOffset = 0;
    uint32_t                m_threadTaskBufferSize = 0;
    uint32_t                m_threadTaskBufferOffset = 0;
    bool                    m_initEncConstTable = true;
    bool                    m_enableBrcLTR = 1;  //!< flag to enable long term reference BRC feature.
    bool                    m_isFrameLTR = 0;    //!<flag to check if current frame is set as long term reference
    uint32_t                m_ltrInterval = 0;   //!< long term reference interval

    CodechalKernelIntraDist *m_intraDistKernel = nullptr;
    CodechalEncodeSwScoreboardG12 *m_swScoreboardState = nullptr;    //!< pointer to SW scoreboard ini state.
    CodecHalHevcBrcG12* m_hevcBrcG12 = nullptr;

    // scalability
    unsigned char                         m_numPipe            = 1;         //!< Number of pipes
    unsigned char                         m_numPassesInOnePipe = 1;         //!< Number of PAK passes in one pipe
    CODECHAL_ENCODE_BUFFER                m_resPakSliceLevelStreamoutData;  //!< Surface for slice level stream out data from PAK
    CODECHAL_HEVC_VIRTUAL_ENGINE_OVERRIDE m_kmdVeOveride;                   //!< KMD override virtual engine index
    uint32_t                              m_numTiles = 1;                   //!< Number of tiles
    CODECHAL_ENCODE_BUFFER                m_resHcpScalabilitySyncBuffer;    //!< Hcp sync buffer for scalability
    CODECHAL_ENCODE_BUFFER                m_resTileBasedStatisticsBuffer[CODECHAL_NUM_UNCOMPRESSED_SURFACE_HEVC];
    CODECHAL_ENCODE_BUFFER                m_resHuCPakAggregatedFrameStatsBuffer;
    CODECHAL_ENCODE_BUFFER                m_tileRecordBuffer[CODECHAL_NUM_UNCOMPRESSED_SURFACE_HEVC];
    HEVC_TILE_STATS_INFO                  m_hevcTileStatsOffset = {};       //!< Page aligned offsets used to program HCP / VDEnc pipe and HuC PAK Integration kernel input
    HEVC_TILE_STATS_INFO                  m_hevcFrameStatsOffset = {};      //!< Page aligned offsets used to program HuC PAK Integration kernel output, HuC BRC kernel input
    HEVC_TILE_STATS_INFO                  m_hevcStatsSize = {};             //!< HEVC Statistics size
    bool                                  m_enableTestMediaReset = 0;  //!< enable media reset test. driver will send cmd to make hang happens
    bool                                  m_forceScalability = false;  //!< force scalability for resolution < 4K if other checking for scalability passed
    bool                                  m_enableFramePanicMode = true;   //!< Flag to control frame panic feature

    // HuC PAK stitch kernel
    bool                                        m_hucPakStitchEnabled = false;                                //!< HuC PAK stitch enabled flag
    MOS_RESOURCE                                m_resHucPakStitchDmemBuffer[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM][CODECHAL_DP_MAX_NUM_BRC_PASSES];  //!< HuC Pak Integration Dmem data for each pass
    MOS_RESOURCE                                m_resBrcDataBuffer;                                           //!< Resource of bitrate control data buffer
    MOS_RESOURCE                                m_resHucStitchDataBuffer[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM][CODECHAL_HEVC_MAX_NUM_BRC_PASSES];  // data buffer for huc input cmd generation
    MHW_BATCH_BUFFER                            m_HucStitchCmdBatchBuffer = {};             //!< SLB for huc stitch cmd

    // BRC panic mode
    struct SkipFrameInfo
    {
        uint32_t numSlices = 0;
        MOS_RESOURCE m_resMbCodeSkipFrameSurface;  //!< PAK obj and CU records
    }
    m_skipFrameInfo;

    // virtual engine
                                                                                                // virtual engine
    bool                   m_useVirtualEngine = false;                                                                                                 //!< Virtual engine enable flag
    MOS_COMMAND_BUFFER     m_veBatchBuffer[CODECHAL_NUM_UNCOMPRESSED_SURFACE_HEVC][CODECHAL_HEVC_MAX_NUM_HCP_PIPE][CODECHAL_HEVC_MAX_NUM_BRC_PASSES];  //!< Virtual engine batch buffers
    MOS_COMMAND_BUFFER     m_realCmdBuffer;                                                                                                            //!< Virtual engine command buffer
    uint32_t               m_sizeOfVeBatchBuffer  = 0;                                                                                                 //!< Virtual engine batch buffer size
    unsigned char          m_virtualEngineBbIndex = 0;                                                                                                 //!< Virtual engine batch buffer index
    CODECHAL_ENCODE_BUFFER m_resBrcSemaphoreMem[CODECHAL_HEVC_MAX_NUM_HCP_PIPE];                                                                       //!< BRC HW semaphore
    CODECHAL_ENCODE_BUFFER m_resBrcPakSemaphoreMem;                                                                                                    //!< BRC PAK HW semaphore
    MOS_RESOURCE           m_resPipeStartSemaMem;                                                                                                      //!< HW semaphore for scalability pipe start at the same time
    MOS_RESOURCE           m_resPipeCompleteSemaMem; 
    MOS_RESOURCE           m_resDelayMinus = {};
    uint32_t               m_numDelay = 0;


    // the following constant integers and tables are from the kernel for score board computation
    static uint32_t const m_ct = 3;
    static uint32_t const m_maxNumDependency = 32;
    static uint32_t const m_numDependencyHorizontal = 1;
    static uint32_t const m_numDependencyVertical = 1;
    static uint32_t const m_numDependency45Degree = 2;
    static uint32_t const m_numDependency26Degree = 3;
    static uint32_t const m_numDependency45xDegree = 3 + (m_ct - 1);
    static uint32_t const m_numDependency26xDegree = 4 + (m_ct - 1);
    static uint32_t const m_numDependency45xDegreeAlt = 2;
    static uint32_t const m_numDependency26xDegreeAlt = 3;
    static uint32_t const m_numDependency45xVp9Degree = 4;
    static uint32_t const m_numDependency26zDegree = 5;
    static uint32_t const m_numDependency26ZigDegree = 6;
    static uint32_t const m_numDependencyNone = 0;
    static uint32_t const m_numDependencyCustom = 0;
    static const char m_dxWavefrontHorizontal[m_maxNumDependency];
    static const char m_dyWavefrontHorizontal[m_maxNumDependency];
    static const char m_dxWavefrontVertical[m_maxNumDependency];
    static const char m_dyWavefrontVertical[m_maxNumDependency];
    static const char m_dxWavefront45Degree[m_maxNumDependency];
    static const char m_dyWavefront45Degree[m_maxNumDependency];
    static const char m_dxWavefront26Degree[m_maxNumDependency];
    static const char m_dyWavefront26Degree[m_maxNumDependency];
    static const char m_dxWavefront45xDegree[m_maxNumDependency];
    static const char m_dyWavefront45xDegree[m_maxNumDependency];
    static const char m_dxWavefront26xDegree[m_maxNumDependency];
    static const char m_dyWavefront26xDegree[m_maxNumDependency];
    static const char m_dxWavefront45xDegreeAlt[m_maxNumDependency];
    static const char m_dyWavefront45xDegreeAlt[m_maxNumDependency];
    static const char m_dxWavefront26xDegreeAlt[m_maxNumDependency];
    static const char m_dyWavefront26xDegreeAlt[m_maxNumDependency];
    static const char m_dxWavefront45xVp9Degree[m_maxNumDependency];
    static const char m_dyWavefront45xVp9Degree[m_maxNumDependency];
    static const char m_dxWavefront26zDegree[m_maxNumDependency];
    static const char m_dyWavefront26zDegree[m_maxNumDependency];
    static const char m_dxWavefront26ZigDegree[m_maxNumDependency];
    static const char m_dyWavefront26ZigDegree[m_maxNumDependency];
    static const char m_dxWavefrontNone[m_maxNumDependency];
    static const char m_dyWavefrontNone[m_maxNumDependency];
    static const char m_dxWavefrontCustom[m_maxNumDependency];
    static const char m_dyWavefrontCustom[m_maxNumDependency];

    //!
    //! \brief    Constructor
    //!
    CodechalEncHevcStateG12(CodechalHwInterface* hwInterface,
        CodechalDebugInterface* debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo);

    //!
    //! \brief    Copy constructor
    //!
    CodechalEncHevcStateG12(const CodechalEncHevcStateG12&) = delete;

    //!
    //! \brief    Copy assignment operator
    //!
    CodechalEncHevcStateG12& operator=(const CodechalEncHevcStateG12&) = delete;

    //!
    //! \brief    Destructor
    //!
    virtual ~CodechalEncHevcStateG12();

    //!
    //! \brief  Entry to allocate and intialize the encode instance
    //! \param  [in] codecHalSettings
    //!         The settings to inialize the encode instance
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Allocate(CodechalSetting * codecHalSettings) override;


    //!
    //! \brief    Help function to get current pipe
    //!
    //! \return   Current pipe value
    //!
    int GetCurrentPipe()
    {
        if (m_numPipe <= 1)
        {
            return 0;
        }

        return (int)(m_currPass) % (int)m_numPipe;
    }

    //!
    //! \brief    Help function to get current PAK pass
    //!
    //! \return   Current PAK pass
    //!
    int GetCurrentPass() override
    {
        if (m_numPipe <= 1)
        {
            return m_currPass;
        }

        return (int)(m_currPass) / (int)m_numPipe;
    }

    //!
    //! \brief    Help function to check if current pipe is first pipe
    //!
    //! \return   True if current pipe is first pipe, otherwise return false
    //!
    bool IsFirstPipe()
    {
        return GetCurrentPipe() == 0 ? true : false;
    }

    //!
    //! \brief    Help function to check if current pipe is last pipe
    //!
    //! \return   True if current pipe is last pipe, otherwise return false
    //!
    bool IsLastPipe()
    {
        return GetCurrentPipe() == m_numPipe - 1 ? true : false;
    }

    //!
    //! \brief    Help function to check if current PAK pass is first pass
    //!
    //! \return   True if current PAK pass is first pass, otherwise return false
    //!
    bool IsFirstPass() override
    {
        return GetCurrentPass() == 0 ? true : false;
    }

    //!
    //! \brief    Help function to check if current PAK pass is last pass
    //!
    //! \return   True if current PAK pass is last pass, otherwise return false
    //!
    bool IsLastPass() override
    {
        return GetCurrentPass() == m_numPassesInOnePipe ? true : false;
    }

    //!
    //! \brief    Help function to check if current PAK pass is the panic mode pass
    //!
    //! \return   True if current PAK pass is the panic mode pass, otherwise return false
    //!
    bool IsPanicModePass()
    {
        return GetCurrentPass() == CODECHAL_HEVC_MAX_NUM_BRC_PASSES ? true : false;
    }

    // inherited virtual functions
    MOS_STATUS SetPictureStructs() override;

    MOS_STATUS CalcScaledDimensions() override;
    
    MOS_STATUS InitializePicture(const EncoderParams& params) override;
    
    MOS_STATUS ExecutePictureLevel() override;

    MOS_STATUS ExecuteSliceLevel() override;

    MOS_STATUS Initialize(CodechalSetting * settings) override;

    virtual MOS_STATUS InitKernelState() override;

    virtual uint32_t GetMaxBtCount() override;

    bool CheckSupportedFormat(PMOS_SURFACE surface) override;

    MOS_STATUS EncodeKernelFunctions() override;
    
    MOS_STATUS EncodeMeKernel() override;
    
    virtual MOS_STATUS EncodeIntraDistKernel();

    virtual MOS_STATUS AllocateEncResources() override;

    virtual MOS_STATUS FreeEncResources() override;

    MOS_STATUS AllocatePakResources() override;

    MOS_STATUS FreePakResources() override;

    virtual MOS_STATUS GetFrameBrcLevel() override;

    void CreateMhwParams() override;

    void GetMaxRefFrames(uint8_t& maxNumRef0, uint8_t& maxNumRef1) override;

    void SetHcpSliceStateCommonParams(MHW_VDBOX_HEVC_SLICE_STATE& sliceStateParams) override;

    MOS_STATUS PlatformCapabilityCheck() override;

    MOS_STATUS GetStatusReport(
        EncodeStatus *encodeStatus,
        EncodeStatusReport *encodeStatusReport) override;
    MOS_STATUS SetRegionsHuCPakIntegrate(PMHW_VDBOX_HUC_VIRTUAL_ADDR_PARAMS virtualAddrParams);
    MOS_STATUS SetDmemHuCPakIntegrate(PMHW_VDBOX_HUC_DMEM_STATE_PARAMS dmemParams);
    MOS_STATUS SetRegionsHuCPakIntegrateCqp(PMHW_VDBOX_HUC_VIRTUAL_ADDR_PARAMS virtualAddrParams);
    MOS_STATUS SetDmemHuCPakIntegrateCqp(PMHW_VDBOX_HUC_DMEM_STATE_PARAMS dmemParams);
    MOS_STATUS ReadBrcPakStatisticsForScalability(PMOS_COMMAND_BUFFER   cmdBuffer);
#if (_DEBUG || _RELEASE_INTERNAL)
    MOS_STATUS ResetImgCtrlRegInPAKStatisticsBuffer(PMOS_COMMAND_BUFFER   cmdBuffer);
#endif

    MOS_STATUS GetCommandBuffer(PMOS_COMMAND_BUFFER cmdBuffer) override;

    MOS_STATUS ReturnCommandBuffer(PMOS_COMMAND_BUFFER cmdBuffer) override;

    MOS_STATUS SubmitCommandBuffer(
        PMOS_COMMAND_BUFFER cmdBuffer,
        bool  nullRendering) override;

    MOS_STATUS SetSliceStructs() override;

    MOS_STATUS AllocateTileStatistics();

    void SetHcpIndObjBaseAddrParams(MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS& indObjBaseAddrParams) override;
    void SetHcpPipeBufAddrParams(MHW_VDBOX_PIPE_BUF_ADDR_PARAMS& pipeBufAddrParams) override;
    void SetHcpPicStateParams(MHW_VDBOX_HEVC_PIC_STATE& picStateParams) override;

    MOS_STATUS ReadSseStatistics(PMOS_COMMAND_BUFFER cmdBuffer) override;

    MOS_STATUS SetGpuCtxCreatOption() override;

    //!
    //! \brief    Decide number of pipes used for encoding
    //! \details  called inside PlatformCapabilityCheck
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DecideEncodingPipeNumber();

    //!
    //! \brief    Get U62 Mode bits
    //!
    //! \return   8 bit mode cost for RDE
    //!
    inline uint8_t GetU62ModeBits(float mcost)
    {
        return (uint8_t)(mcost * 4 + 0.5);
    }

    //!
    //! \brief    Update surface info for YUY2 input
    //!
    //! \param    [in] surface
    //!           Reference to input surface
    //! \param    [in] is10Bit
    //!           Flag to indicate if 10 bit
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS UpdateYUY2SurfaceInfo(
        MOS_SURFACE& surface,
        bool         is10Bit);

    //!
    //! \brief    Allocate ME resources
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateMeResources();

    //!
    //! \brief    Free ME resources
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS FreeMeResources();

    //!
    //! \brief    Encode command at tile level
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS EncTileLevel();

    //!
    //! \brief    Get encoder kernel header and kernel size
    //!
    //! \param    [in] binary
    //!           Pointer to kernel binary
    //! \param    [in] operation
    //!           Enc kernel operation
    //! \param    [in] krnStateIdx
    //!           Kernel state index
    //! \param    [out] krnHeader
    //!           Pointer to kernel header
    //! \param    [out] krnSize
    //!           Pointer to kernel size
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    static MOS_STATUS GetKernelHeaderAndSize(
        void*                           binary,
        EncOperation                    operation,
        uint32_t                        krnStateIdx,
        void*                           krnHeader,
        uint32_t*                       krnSize);

    //!
    //! \brief    Get encoder kernel header and kernel size
    //!
    //! \param    [in] encOperation
    //!           Specifies the media function type
    //! \param    [in] kernelParams
    //!           Pointer to kernel parameters
    //! \param    [in] idx
    //!           MbEnc/BRC kernel index
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetKernelParams(
        EncOperation                  encOperation,
        MHW_KERNEL_PARAM*             kernelParams,
        uint32_t                      idx);

    //!
    //! \brief    Set Binding table for different kernelsge
    //!
    //! \param    [in] encOperation
    //!           Specifies the media function type
    //! \param    [in] hevcEncBindingTable
    //!           Pointer to the binding table
    //! \param    [in] idx
    //!           MbEnc/BRC kernel index
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetBindingTable(
        EncOperation                            encOperation,
        PCODECHAL_ENCODE_BINDING_TABLE_GENERIC  hevcEncBindingTable,
        uint32_t                                idx);

    //!
    //! \brief    Initialize MbEnc kernel state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS InitKernelStateMbEnc();

    //!
    //! \brief    Initialize BRC kernel state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitKernelStateBrc();

    //!
    //! \brief    Invoke BRC Init/Reset kernel
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS EncodeBrcInitResetKernel();

    //!
    //! \brief    Send surfaces BRC Init/Reset kernel
    //!
    //! \param    [in]  cmdBuffer
    //!           Pointer to command buffer
    //! \param    [in]  krnIdx
    //!           Index of the BRC kernel for which surfaces are being sent
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SendBrcInitResetSurfaces(
        PMOS_COMMAND_BUFFER      cmdBuffer,
        CODECHAL_HEVC_BRC_KRNIDX krnIdx);

    //!
    //! \brief    Setup Curbe for BRC Init/Reset kernel
    //!
    //! \param    [in]  brcKrnIdx
    //!           Index of the BRC kernel for which Curbe is setup
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetCurbeBrcInitReset(CODECHAL_HEVC_BRC_KRNIDX brcKrnIdx);

    //!
    //! \brief    Invoke frame level BRC update kernel
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS EncodeBrcFrameUpdateKernel();

    //!
    //! \brief    Send surfaces for BRC Frame Update kernel
    //!
    //! \param    [in]  cmdBuffer
    //!           Pointer to command buffer
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SendBrcFrameUpdateSurfaces(PMOS_COMMAND_BUFFER cmdBuffer);

    //!
    //! \brief    Setup Curbe for BRC Update kernel
    //!
    //! \param    [in]  brcKrnIdx
    //!           Index of the BRC update kernel(frame or lcu) for which Curbe is setup
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetCurbeBrcUpdate(CODECHAL_HEVC_BRC_KRNIDX brcKrnIdx);

    //!
    //! \brief    Invoke LCU level BRC update kernel
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS EncodeBrcLcuUpdateKernel();

    //!
    //! \brief    Send surfaces for BRC LCU Update kernel
    //!
    //! \param    [in]  cmdBuffer
    //!           Pointer to command buffer
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SendBrcLcuUpdateSurfaces(PMOS_COMMAND_BUFFER cmdBuffer);

    //!
    //! \brief    Top level function for invoking MBenc kernel
    //! \details  I, B or LCU64_B MBEnc kernel, based on EncFunctionType
    //! \param    [in]  encFunctionType
    //!           Specifies the media state type
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS EncodeMbEncKernel(CODECHAL_MEDIA_STATE_TYPE encFunctionType);

    //!
    //! \brief    Send Surfaces for MbEnc I kernel
    //!
    //! \param    [in]  cmdBuffer
    //!           Pointer to command buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SendMbEncSurfacesIKernel(PMOS_COMMAND_BUFFER cmdBuffer);

    //!
    //! \brief    Setup Curbe for MbEnc I kernel
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetCurbeMbEncIKernel();

    //!
    //! \brief    Send Surfaces for MbEnc B kernel
    //!
    //! \param    [in]  cmdBuffer
    //!           Pointer to command buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SendMbEncSurfacesBKernel(PMOS_COMMAND_BUFFER cmdBuffer);

    //!
    //! \brief    Setup Curbe for MbEnc B LCU32 and LCU64_32 Kernels
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetCurbeMbEncBKernel();

    //!
    //! \brief    Generate LCU Level Data
    //!
    //! \param    [in]  lcuLevelInputDataSurfaceParam
    //!           input lcu surface
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS GenerateLcuLevelData(MOS_SURFACE &lcuLevelInputDataSurfaceParam);

     //!
     //! \brief    Generate 'Skip frame' mbCodeSurface
     //!
     //! \param    [in]  skipframeInfo
     //!           skip frame surface
     //! \return   MOS_STATUS
     //!           MOS_STATUS_SUCCESS if success, else fail reason
     //!
    MOS_STATUS GenerateSkipFrameMbCodeSurface(SkipFrameInfo &skipframeInfo);

    //!
    //! \brief    Convert from Y210 to Y210V format
    //!
    //! \param    [in]  source
    //!           Source surface
    //! \param    [out]  target
    //!           Destination surface
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ConvertY210ToY210V(
        PMOS_SURFACE source,
        PMOS_SURFACE target);

    //!
    //! \brief    Convert from P010 to P010V format
    //!
    //! \param    [in]  source
    //!           Source surface
    //! \param    [out]  target
    //!           Destination surface
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ConvertP010ToP010V(
        PMOS_SURFACE source,
        PMOS_SURFACE target);

    //!
    //! \brief    Convert from YUY2 to YUY2V format
    //!
    //! \param    [in]  source
    //!           Source surface
    //! \param    [out]  target
    //!           Destination surface
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ConvertYUY2ToYUY2V(
        PMOS_SURFACE source,
        PMOS_SURFACE target);

    //!
    //! \brief    Downscale input by 2X
    //!
    //! \param    [in]  source
    //!           Source surface
    //! \param    [out]  target
    //!           Destination surface
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DownScaling2X(
        PMOS_SURFACE source,
        PMOS_SURFACE target);

    //!
    //! \brief    Load cost table
    //!
    //! \param    [in]  sliceType
    //!           Slice Type
    //! \param    [in]  qp
    //!           QP value
    //!
    //! \return   void
    //!
    void LoadCosts(uint8_t sliceType, uint8_t qp);

    //!
    //! \brief    Prepare walker params for custom pattern thread dispatch
    //!
    //! \param    [in]  walkerParams
    //!           Pointer to HW walker params
    //! \param    [in]  walkerCodecParams
    //!           Input params to program the HW walker
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS GetCustomDispatchPattern(
        PMHW_WALKER_PARAMS            walkerParams,
        PCODECHAL_WALKER_CODEC_PARAMS walkerCodecParams);

    //!
    //! \brief    Generate concurrent thread group data
    //!
    //! \param    [in]  concurrentThreadGroupData
    //!           reference to the concurrentThreadGroupData surface
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS GenerateConcurrentThreadGroupData(MOS_RESOURCE & concurrentThreadGroupData);

    //!
    //! \brief    Load Pak command and CuRecord from file
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS LoadPakCommandAndCuRecordFromFile();

    //!
    //! \brief    Load source surface and 2xDownScaled references from file
    //!
    //! \param    [in] pRef2xSurface
    //!           reference surface to be dumped
    //! \param    [in] pSrc2xSurface
    //!           source surface to be dumped
    //! \param    [in] reflist
    //!           ref list, l0 or l1
    //! \param    [in] refIdx
    //!           ref index to the ref list
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS LoadSourceAndRef2xDSFromFile(PMOS_SURFACE pRef2xSurface, PMOS_SURFACE pSrc2xSurface, uint8_t reflist, uint8_t refIdx);

    //!
    //! \brief   Re-calculate buffer size and offets during resolution reset
    //!
    //! \return   void
    //!
    void ResizeBufferOffset();

    //!
    //! \brief    Set HCP_SLICE_STATE parameters that are different at slice level
    //!
    //! \param    [in, out] sliceState
    //!           HCP_SLICE_STATE parameters
    //! \param    [in] slcData
    //!           Pointer to CODEC_ENCODE_SLCDATA
    //! \param    [in] slcCount
    //!           Current slice index
    //! \param    [in] tileCodingParams
    //!           Pointer to TileCodingParams
    //! \param    [in] lastSliceInTile
    //!           Flag to indicate if slice is the last one in the tile
    //! \param    [in] idx
    //!           Index of the tile
    //!
    //! \return   void
    //!
    void SetHcpSliceStateParams(
        MHW_VDBOX_HEVC_SLICE_STATE&            sliceState,
        PCODEC_ENCODER_SLCDATA                 slcData,
        uint16_t                               slcCount,
        PMHW_VDBOX_HCP_TILE_CODING_PARAMS_G12  tileCodingParams,
        bool                                   lastSliceInTile,
        uint32_t                               idx);

    //!
    //! \brief     Set MFX_VIDEO_COPY commands for HW stitch in scalable mode
    //!
    //! \param    [in] cmdBuffer
    //!           Pointer to the command buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetMfxVideoCopyCmdParams(
        PMOS_COMMAND_BUFFER cmdBuffer);

    //!
    //! \brief    Setup BRC constant data
    //!
    //! \param    [in, out]  brcConstantData
    //!           Pointer to BRC constant data surface
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetupBrcConstantTable(PMOS_SURFACE brcConstantData);

    //!
    //! \brief    Check whether Scalability is enabled or not,
    //!           Set number of VDBoxes accordingly
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS GetSystemPipeNumberCommon(); 
    
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

    bool       IsDegree45Needed();

    virtual void       DecideConcurrentGroupAndWaveFrontNumber();
    virtual void       InitSwScoreBoardParams(CodechalEncodeSwScoreboard::KernelParams & swScoreboardKernelParames);

    MOS_STATUS UserFeatureKeyReport() override;

    MOS_STATUS SetupSwScoreBoard(CodechalEncodeSwScoreboard::KernelParams *params);
    void InitSWScoreboard(
        uint8_t* scoreboard,
        uint32_t scoreboardWidth,
        uint32_t scoreboardHeight,
        uint32_t dependencyPattern,
        char childThreadNumber);

    uint8_t PicCodingTypeToSliceType(uint16_t pictureCodingType);

    MOS_STATUS  InitMediaObjectWalker(
        uint32_t threadSpaceWidth,
        uint32_t threadSpaceHeight,
        uint32_t colorCountMinusOne,
        DependencyPattern dependencyPattern,
        uint32_t childThreadNumber,
        uint32_t localLoopExecCount,
        MHW_WALKER_PARAMS&  walkerParams);

    void SetDependency(uint8_t &numDependencies,
        char* scoreboardDeltaX,
        char* scoreboardDeltaY,
        uint32_t dependencyPattern,
        char childThreadNumber);

    //!
    //! \brief    Dump HuC based debug output buffers
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DumpHucDebugOutputBuffers();

    void SetHcpPipeModeSelectParams(MHW_VDBOX_PIPE_MODE_SELECT_PARAMS& pipeModeSelectParams) override;
    MOS_STATUS AddHcpPipeModeSelectCmd(MOS_COMMAND_BUFFER* cmdBuffer) override;
    MOS_STATUS AddHcpSurfaceStateCmds(MOS_COMMAND_BUFFER* cmdBuffer) override;
    MOS_STATUS AddHcpPictureStateCmd(MOS_COMMAND_BUFFER* cmdBuffer) override;

    MOS_STATUS CalculatePictureStateCommandSize() override;

    MOS_STATUS AddHcpPipeBufAddrCmd(
        PMOS_COMMAND_BUFFER  cmdBuffer) override;

    //!
    //! \brief    Is slice in the current tile
    //!
    //! \param    [in] sliceNumber
    //!           Slice number
    //! \param    [in] currentTile
    //!           Pointer to current tile coding params
    //! \param    [out] sliceInTile
    //!           Pointer to return if slice in tile
    //! \param    [out] lastSliceInTile
    //!           Pointer to return if last slice in tile
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS IsSliceInTile(
        uint32_t                                sliceNumber,
        PMHW_VDBOX_HCP_TILE_CODING_PARAMS_G12   currentTile,
        bool                                    *sliceInTile,
        bool                                    *lastSliceInTile);

    //!
    //! \brief    Set tile data
    //!
    //! \param    [in] tileCodingParams
    //!           Pointer to tile coding params
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetTileData(MHW_VDBOX_HCP_TILE_CODING_PARAMS_G12*    tileCodingParams, uint32_t bistreamBufSize);

    MOS_STATUS AddHcpRefIdxCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_BATCH_BUFFER batchBuffer,
        PMHW_VDBOX_HEVC_SLICE_STATE params) override;

    //!
    //! \brief    Help function to verify command buffer size
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS VerifyCommandBufferSize() override;

    //!
    //! \brief    Help function to send prolog with frame tracking information
    //!
    //! \param    [in] cmdBuffer
    //!           Pointer to command buffer
    //! \param    [in] frameTrackingRequested
    //!           True if frame tracking info is needed, false otherwise
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SendPrologWithFrameTracking(
        PMOS_COMMAND_BUFFER cmdBuffer,
        bool frameTrackingRequested,
        MHW_MI_MMIOREGISTERS *mmioRegister = nullptr) override;

    //!
    //! \brief    HuC PAK integrate
    //!
    //! \param    [in] cmdBuffer
    //!           Pointer to command buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS HucPakIntegrate(
        PMOS_COMMAND_BUFFER cmdBuffer);

    MOS_STATUS InitMmcState() override;

    MOS_STATUS UpdateCmdBufAttribute(
        PMOS_COMMAND_BUFFER cmdBuffer,
        bool                renderEngineInUse) override;
        
    //!
    //! \brief    Configue stitch data buffer as Huc Pak Integration input
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ConfigStitchDataBuffer();

    MOS_STATUS AddMediaVfeCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        SendKernelCmdsParams *params) override;    
    
    //!
    //! \brief    allocate resources with sizes varying from frame to frame
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateResourcesVariableSize();

#if USE_CODECHAL_DEBUG_TOOL
    //!
    //! \brief    Dump PAK output buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DumpPakOutput();

    MOS_STATUS DumpFrameStatsBuffer(CodechalDebugInterface *debugInterface) override;
#endif

    uint32_t CodecHalHevc_GetFileSize(char* fileName);
};

//! \brief  typedef of class CodechalEncHevcStateG12*
using PCODECHAL_ENC_HEVC_STATE_G12 = class CodechalEncHevcStateG12*;

#endif  // __CODECHAL_ENCODE_HEVC_G12_H__
