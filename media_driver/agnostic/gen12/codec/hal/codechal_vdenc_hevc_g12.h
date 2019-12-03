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
//! \file     codechal_vdenc_hevc_g12.h
//! \brief    HEVC VDEnc encoder for GEN12 platform.
//!

#ifndef __CODECHAL_VDENC_HEVC_G12_H__
#define __CODECHAL_VDENC_HEVC_G12_H__

#include "codechal_vdenc_hevc.h"
#include "mhw_vdbox_g12_X.h"
#include "codechal_debug_encode_par_g12.h"
#include "codechal_encode_singlepipe_virtualengine.h"
#include "codechal_encode_scalability.h"
#ifdef _ENCODE_VDENC_RESERVED
  #include "codechal_vdenc_hevc_g12_rsvd.h"
#endif

#define  HUC_CMD_LIST_MODE 1
#define  HUC_BATCH_BUFFER_END 0x05000000
#define  VDBOX_HUC_PAK_INTEGRATION_KERNEL_DESCRIPTOR 15
#define  CODECHAL_ENCODE_HEVC_VDENC_WP_DATA_BLOCK_NUMBER 6

struct CODECHAL_VDENC_HEVC_HUC_BRC_INIT_DMEM_G12
{
    uint32_t    BRCFunc_U32;                  // 0: Init; 2: Reset, bit7 0: frame-based; 1: tile-based
    uint32_t    UserMaxFrame;                 // ProfileLevelMaxFrame_U32
    uint32_t    InitBufFull_U32;
    uint32_t    BufSize_U32;
    uint32_t    TargetBitrate_U32;
    uint32_t    MaxRate_U32;
    uint32_t    MinRate_U32;
    uint32_t    FrameRateM_U32;
    uint32_t    FrameRateD_U32;
    uint32_t    LumaLog2WeightDenom_U32;
    uint32_t    ChromaLog2WeightDenom_U32;
    uint8_t     BRCFlag : 7;         // ACQP/ICQ=0, CBR=1, VBR=2, VCM=3, LOWDELAY=4
    uint8_t     SSCFlag : 1;         // SSC: 0x80
    uint8_t     Reserved;
    uint16_t    GopP_U16;
    uint16_t    GopB_U16;
    uint16_t    FrameWidth_U16;
    uint16_t    FrameHeight_U16;
    uint16_t    GopB1_U16;
    uint16_t    GopB2_U16;
    uint8_t     MinQP_U8;
    uint8_t     MaxQP_U8;
    uint8_t     MaxBRCLevel_U8;
    uint8_t     LumaBitDepth_U8;
    uint8_t     ChromaBitDepth_U8;
    uint8_t     CuQpCtrl_U8;        // 0=No CUQP; 1=CUQP for I-frame; 2=CUQP for P/B-frame

    uint8_t     RSVD0[4];
    int8_t      DevThreshPB0_S8[8];
    int8_t      DevThreshVBR0_S8[8];
    int8_t      DevThreshI0_S8[8];
    int8_t      InstRateThreshP0_S8[4];
    int8_t      InstRateThreshB0_S8[4];
    int8_t      InstRateThreshI0_S8[4];
    uint8_t     LowDelayMode_U8;
    uint8_t     InitQPIP_U8;
    uint8_t     InitQPB_U8;                    // In CQP mode, InitQPB_U8= InitQPIP_U8
    uint8_t     QPDeltaThrForAdapt2Pass_U8;
    uint8_t     TopFrmSzThrForAdapt2Pass_U8;
    uint8_t     BotFrmSzThrForAdapt2Pass_U8;
    uint8_t     QPSelectForFirstPass_U8;
    uint8_t     MBHeaderCompensation_U8;
    uint8_t     OverShootCarryFlag_U8;
    uint8_t     OverShootSkipFramePct_U8;
    uint8_t     EstRateThreshP0_U8[7];
    uint8_t     EstRateThreshB0_U8[7];

    uint8_t     EstRateThreshI0_U8[7];
    uint8_t     QPP_U8;
    uint8_t     StreamInSurfaceEnable_U8;           // 0-disabled, 1-enabled
    uint8_t     StreamInROIEnable_U8;               // 0-disabled, 1-enabled
    uint8_t     TimingBudget_Enable_U8;             // 0-disabled, 1-enabled
    uint8_t     TopQPDeltaThrForAdapt2Pass_U8;      // 2
    uint8_t     BotQPDeltaThrForAdapt2Pass_U8;      // 1
    uint8_t     RESERVED;
    uint8_t     NetworkTraceEnable_U8;                  // 0-disabled, 1-enabled
    uint8_t     LowDelaySceneChangeXFrameSizeEnable_U8; // 0-disabled, 1-enabled
    uint32_t    ACQP_U32;                               // 1
    uint32_t    SlidingWindow_Size_U32;                 // 30

    uint8_t     SLIDINGWINDOW_MaxRateRatio;
    uint8_t     RSVD2;
    int8_t      CbQPOffset;
    int8_t      CrQPOffset;

    uint32_t    ProfileLevelMaxFramePB_U32;

    // tile-based BRC
    uint16_t    SlideWindowRC;                          // Reserved now
    uint16_t    MaxLogCUSize;

    uint16_t    FrameWidthInLCU;
    uint16_t    FrameHeightInLCU;
 
    uint8_t     BRCPyramidEnable_U8;
    uint8_t     LongTermRefEnable_U8;
    uint16_t    LongTermRefInterval_U16;
    uint8_t     LongTermRefMsdk_U8;
    uint8_t     IsLowDelay_U8;
    uint16_t    RSVD3;
    uint32_t    RSVD1[4];                               // 64 bytes aligned
};
C_ASSERT(192 == sizeof(CODECHAL_VDENC_HEVC_HUC_BRC_INIT_DMEM_G12));

using PCODECHAL_VDENC_HEVC_HUC_BRC_INIT_DMEM_G12 = CODECHAL_VDENC_HEVC_HUC_BRC_INIT_DMEM_G12*;

struct CODECHAL_VDENC_HEVC_HUC_BRC_UPDATE_DMEM_G12
{
    uint32_t    TARGETSIZE_U32;
    uint32_t    FrameID_U32;                    // frame number
    uint32_t    Ref_L0_FrameID_U32[8];
    uint32_t    Ref_L1_FrameID_U32[8];
    uint16_t    startGAdjFrame_U16[4];          // 10, 50, 100, 150
    uint16_t    TargetSliceSize_U16;
    uint16_t    SLB_Data_SizeInBytes;
    uint16_t    PIC_STATE_StartInBytes;         // PIC_STATE starts in byte. 0xFFFF means not available in SLB
    uint16_t    CMD2_StartInBytes;
    uint16_t    CMD1_StartInBytes;
    uint16_t    PIPE_MODE_SELECT_StartInBytes;  // PIPE Mode select starts in byte. 0xFFFF means not available in SLB
    uint16_t    Current_Data_Offset;            // Data block offset of current picture from beginning of the data buffer (region 9)
    uint16_t    Ref_Data_Offset[5];             // Data block offset of ref pictures from beginning of the data buffer (region 9)
    uint16_t    MaxNumSliceAllowed_U16;
    uint8_t     OpMode_U8;                      // 1: frame-based BRC (including ACQP), 2: Weighted prediction, Weighted prediction should not be enabled in first pass.
                                                // Same as other common flags, this is a bit operation. Each bit is zero for disabling and 1 for enabling. i.e. 01: BRC, 10: WP - never used; 11: BRC+WP, 4: tile-based BRC (frame level), 8: tile-based BRC (tile level)
    uint8_t     CurrentFrameType_U8;
    uint8_t     Num_Ref_L0_U8;
    uint8_t     Num_Ref_L1_U8;
    uint8_t     Num_Slices;
    uint8_t     CQP_QPValue_U8;                 // CQP QP value (needed for ICQ and ACQP)
    uint8_t     CQP_FracQP_U8;
    uint8_t     MaxNumPass_U8;                  // max number of BRC passes (SAO second pass is not included.)
    uint8_t     gRateRatioThreshold_U8[7];
    uint8_t     startGAdjMult_U8[5];
    uint8_t     startGAdjDiv_U8[5];
    uint8_t     gRateRatioThresholdQP_U8[8];
    uint8_t     SceneChgPrevIntraPctThreshold_U8;
    uint8_t     SceneChgCurIntraPctThreshold_U8;
    uint8_t     IPAverageCoeff_U8;
    uint8_t     CurrentPass_U8;
    int8_t      DeltaQPForMvZero_S8;
    int8_t      DeltaQPForMvZone0_S8;
    int8_t      DeltaQPForMvZone1_S8;
    int8_t      DeltaQPForMvZone2_S8;
    int8_t      DeltaQPForSadZone0_S8;
    int8_t      DeltaQPForSadZone1_S8;
    int8_t      DeltaQPForSadZone2_S8;
    int8_t      DeltaQPForSadZone3_S8;
    int8_t      DeltaQPForROI0_S8;
    int8_t      DeltaQPForROI1_S8;
    int8_t      DeltaQPForROI2_S8;
    int8_t      DeltaQPForROI3_S8;
    int8_t      LumaLog2WeightDenom_S8;     // default: 6
    int8_t      ChromaLog2WeightDenom_S8;   // default: 6
    uint8_t     DisabledFeature_U8;
    uint8_t     SlidingWindow_Enable_U8;    // 0-disabled, 1-enabled
    uint8_t     LOG_LCU_Size_U8;            // 6
    uint16_t    NetworkTraceEntry_U16;              // default: 0
    uint16_t    LowDelaySceneChangeXFrameSize_U16;  // default: 0
    int8_t      ReEncodePositiveQPDeltaThr_S8;      // default: 4
    int8_t      ReEncodeNegativeQPDeltaThr_S8;      // default: -10

    // tile-based BRC
    uint8_t     MaxNumTileHuCCallMinus1;            // maximal tile row
    uint8_t     TileHucCallIndex;
    uint8_t     TileHuCCallPassIndex;               // Start from 1
    uint8_t     TileHuCCallPassMax;                 // Reserved now
    uint16_t    TileSizeInLCU;

    uint32_t    TxSizeInBitsPerFrame;

    uint8_t     StartTileIdx;
    uint8_t     EndTileIdx;

    uint16_t    NumFrameSkipped;
    uint32_t    SkipFrameSize;

    uint32_t    SliceHeaderSize;
    uint8_t     IsLongTermRef;

    uint8_t     RSVD[3];                           // 64 bytes aligned
};
C_ASSERT(192 == sizeof(CODECHAL_VDENC_HEVC_HUC_BRC_UPDATE_DMEM_G12));

using PCODECHAL_VDENC_HEVC_HUC_BRC_UPDATE_DMEM_G12 = CODECHAL_VDENC_HEVC_HUC_BRC_UPDATE_DMEM_G12*;

struct CODECHAL_VDENC_HEVC_HUC_BRC_CONSTANT_DATA_G12
{
    uint16_t    SADQPLambdaI[52];
    uint16_t    SADQPLambdaP[52];
    uint16_t    RDQPLambdaI[52];
    uint16_t    RDQPLambdaP[52];
    uint16_t    SLCSZ_THRDELTAI_U16[52];
    uint16_t    SLCSZ_THRDELTAP_U16[52];
    uint8_t     DistThreshldI[9];
    uint8_t     DistThreshldP[9];
    uint8_t     DistThreshldB[9];
    int8_t      DistQPAdjTabI[81];
    int8_t      DistQPAdjTabP[81];
    int8_t      DistQPAdjTabB[81];
    int8_t      FrmSzAdjTabI_S8[72];
    int8_t      FrmSzAdjTabP_S8[72];
    int8_t      FrmSzAdjTabB_S8[72];
    uint8_t     FrmSzMaxTabI[9];
    uint8_t     FrmSzMaxTabP[9];
    uint8_t     FrmSzMaxTabB[9];
    uint8_t     FrmSzMinTabI[9];
    uint8_t     FrmSzMinTabP[9];
    uint8_t     FrmSzMinTabB[9];
    int8_t      QPAdjTabI[45];
    int8_t      QPAdjTabP[45];
    int8_t      QPAdjTabB[45];
    struct
    {
        uint8_t     I_INTRA_64X64DC;    // added later since I frame needs to be setup differently
        uint8_t     I_INTRA_32x32;
        uint8_t     I_INTRA_16x16;
        uint8_t     I_INTRA_8x8;
        uint8_t     I_INTRA_SADMPM;
        uint8_t     I_INTRA_RDEMPM;
        uint8_t     I_INTRA_NxN;
        uint8_t     INTRA_64X64DC;
        uint8_t     INTRA_32x32;
        uint8_t     INTRA_16x16;
        uint8_t     INTRA_8x8;
        uint8_t     INTRA_SADMPM;
        uint8_t     INTRA_RDEMPM;
        uint8_t     INTRA_NxN;
        uint8_t     INTER_32x32;
        uint8_t     INTER_32x16;
        uint8_t     INTER_16x16;
        uint8_t     INTER_16x8;
        uint8_t     INTER_8x8;
        uint8_t     REF_ID;
        uint8_t     MERGE_64X64;
        uint8_t     MERGE_32X32;
        uint8_t     MERGE_16x16;
        uint8_t     MERGE_8x8;
        uint8_t     SKIP_64X64;
        uint8_t     SKIP_32X32;
        uint8_t     SKIP_16x16;
        uint8_t     SKIP_8x8;
    } ModeCosts[52];
    struct
    {
        // Unit in Bytes
        uint16_t    SizeOfCMDs;
        uint16_t    HcpWeightOffsetL0_StartInBytes;         // HCP_WEIGHTOFFSET_L0 starts in bytes from beginning of the SLB. 0xFFFF means unavailable in SLB
        uint16_t    HcpWeightOffsetL1_StartInBytes;         // HCP_WEIGHTOFFSET_L1 starts in bytes from beginning of the SLB. 0xFFFF means unavailable in SLB
        uint16_t    SliceState_StartInBytes;
        uint16_t    SliceHeaderPIO_StartInBytes;
        uint16_t    VdencWeightOffset_StartInBytes;
        // Unit in Bits
        uint16_t    SliceHeader_SizeInBits;
        uint16_t    WeightTable_StartInBits;                // number of bits from beginning of slice header for weight table first bit, 0xffff means not awailable
        uint16_t    WeightTable_EndInBits;                  // number of bits from beginning of slice header for weight table last bit, 0xffff means not awailable
    } Slice[CODECHAL_VDENC_HEVC_MAX_SLICE_NUM];
    uint8_t     PenaltyForIntraNonDC32x32PredMode[52];
};

using PCODECHAL_VDENC_HEVC_HUC_BRC_CONSTANT_DATA_G12 = CODECHAL_VDENC_HEVC_HUC_BRC_CONSTANT_DATA_G12*;

//!
//! \struct HucPakStitchDmemVdencG12
//! \brief  The struct of Huc Com Dmem
//!
struct HucPakStitchDmemVdencG12
{
    uint32_t     TileSizeRecord_offset[5];  // Tile Size Records, start offset  in byte, 0xffffffff means unavailable
    uint32_t     VDENCSTAT_offset[5];      // needed for HEVC VDEnc, VP9 VDEnc, start offset  in byte, 0xffffffff means unavailable
    uint32_t     HEVC_PAKSTAT_offset[5]; //needed for HEVC VDEnc, start offset  in byte, 0xffffffff means unavailable
    uint32_t     HEVC_Streamout_offset[5]; //needed for HEVC VDEnc, start offset  in byte, 0xffffffff means unavailable
    uint32_t     VP9_PAK_STAT_offset[5]; //needed for VP9 VDEnc, start offset  in byte, 0xffffffff means unavailable
    uint32_t     Vp9CounterBuffer_offset[5];    //needed for VP9 VDEnc, start offset  in byte, 0xffffffff means unavailable
    uint32_t     LastTileBS_StartInBytes;// last tile in bitstream for region 4 and region 5
    uint32_t     SliceHeaderSizeinBits;              // needed for HEVC dual pipe BRC
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
    uint8_t         StitchEnable;// enable stitch cmd for Hevc dual pipe
    uint8_t      reserved1;
    uint16_t     StitchCommandOffset; // offset in region 10 which is the second level batch buffer
    uint16_t     reserved2;
    uint32_t     BBEndforStitch;
    uint8_t      RSVD[16];
};

//!
//! \struct HucInputCmdG12
//! \brief  The struct of Huc input command
//!
struct HucInputCmdVdencG12
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

//!
//! \struct HucCommandData
//! \brief  The struct of Huc commands data
//!
struct HucCommandDataVdencG12
{
    uint32_t        TotalCommands;       //!< Total Commands in the Data buffer
    struct
    {
        uint16_t    ID;              //!< Command ID, defined and order must be same as that in DMEM
        uint16_t    SizeOfData;      //!< data size in uint32_t
        uint32_t    data[40];
    } InputCOM[10];
};

struct CODECHAL_VDENC_HEVC_STREAMIN_STATE_G12
{
    // DWORD 0
    union {
        struct {
            uint32_t       RoiCtrl                      : MOS_BITFIELD_RANGE(0, 7);
            uint32_t       MaxTuSize                    : MOS_BITFIELD_RANGE(8, 9);
            uint32_t       MaxCuSize                    : MOS_BITFIELD_RANGE(10, 11);
            uint32_t       NumImePredictors             : MOS_BITFIELD_RANGE(12, 15);
            uint32_t       Reserved_0                   : MOS_BITFIELD_RANGE(16, 20);
            uint32_t       ForceQPDelta                 : MOS_BITFIELD_BIT(21);
            uint32_t       PaletteDisable               : MOS_BITFIELD_BIT(22);
            uint32_t       Reserved_1                   : MOS_BITFIELD_BIT(23);
            uint32_t       PuTypeCtrl                   : MOS_BITFIELD_RANGE(24, 31);
        };
        uint32_t Value;
    } DW0;

    // DWORD 1-4
    union {
        struct {
            uint32_t       ForceMvX                     : MOS_BITFIELD_RANGE(0, 15);
            uint32_t       ForceMvY                     : MOS_BITFIELD_RANGE(16, 31);
        };
        uint32_t Value;
    } DW1[4];

    // DWORD 5
    union {
        struct {
            uint32_t       Reserved                     : MOS_BITFIELD_RANGE(0, 31);
        };
        uint32_t Value;
    } DW5;

    // DWORD 6
    union {
        struct {
            uint32_t       ForceRefIdx                  : MOS_BITFIELD_RANGE(0, 15); //4-bits per 16x16 block
            uint32_t       NumMergeCandidateCu8x8       : MOS_BITFIELD_RANGE(16, 19);
            uint32_t       NumMergeCandidateCu16x16     : MOS_BITFIELD_RANGE(20, 23);
            uint32_t       NumMergeCandidateCu32x32     : MOS_BITFIELD_RANGE(24, 27);
            uint32_t       NumMergeCandidateCu64x64     : MOS_BITFIELD_RANGE(28, 31);
        };
        uint32_t Value;
    } DW6;

    // DWORD 7
    union {
        struct {
            uint32_t       SegID                        : MOS_BITFIELD_RANGE(0, 15); //4-bits per 16x16 block
            uint32_t       QpEnable                     : MOS_BITFIELD_RANGE(16, 19);
            uint32_t       SegIDEnable                  : MOS_BITFIELD_RANGE(20, 20);
            uint32_t       Reserved                     : MOS_BITFIELD_RANGE(21, 22);
            uint32_t       ForceRefIdEnable             : MOS_BITFIELD_RANGE(23, 23);
            uint32_t       ImePredictorSelect           : MOS_BITFIELD_RANGE(24, 31);
        };
        uint32_t Value;
    } DW7;

    // DWORD 8-11
    union {
        struct {
            uint32_t       ImePredictorMvX              : MOS_BITFIELD_RANGE(0, 15);
            uint32_t       ImePredictorMvY              : MOS_BITFIELD_RANGE(16, 31);
        };
        uint32_t Value;
    } DW8[4];

    // DWORD 12
    union {
        struct {
            uint32_t       ImePredictorRefIdx           : MOS_BITFIELD_RANGE(0, 15); //4-bits per 16x16 block
            uint32_t       Reserved                     : MOS_BITFIELD_RANGE(16, 31);
        };
        uint32_t Value;
    } DW12;

    // DWORD 13
    union {
        struct {
            uint32_t       PanicModeLCUThreshold        : MOS_BITFIELD_RANGE(0, 15);
            uint32_t       Reserved                     : MOS_BITFIELD_RANGE(16, 31);
        };
        uint32_t Value;
    } DW13;

    // DWORD 14
    union {
        struct {
            uint32_t       ForceQp_0                    : MOS_BITFIELD_RANGE(0, 7);
            uint32_t       ForceQp_1                    : MOS_BITFIELD_RANGE(8, 15);
            uint32_t       ForceQp_2                    : MOS_BITFIELD_RANGE(16, 23);
            uint32_t       ForceQp_3                    : MOS_BITFIELD_RANGE(24, 31);
        };
        uint32_t Value;
    } DW14;

    // DWORD 15
    union {
        struct {
            uint32_t       Reserved                     : MOS_BITFIELD_RANGE(0, 31);
        };
        uint32_t Value;
    } DW15;
};
C_ASSERT(SIZE32(CODECHAL_VDENC_HEVC_STREAMIN_STATE_G12) == 16);

using PCODECHAL_VDENC_HEVC_STREAMIN_STATE_G12 = CODECHAL_VDENC_HEVC_STREAMIN_STATE_G12*;

const uint32_t ME_CURBE_INIT_G12[48] =
{
    0x00000000, 0x00200010, 0x00003939, 0x77a43000, 0x00000000, 0x28300000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000200,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
};

enum CODECHAL_BINDING_TABLE_OFFSET_HEVC_VP9_VDENC_KERNEL_G12
{
    // VDEnc HME kernel
    CODECHAL_VDENC_HME_BEGIN_G12 = 0,
    CODECHAL_VDENC_HME_MV_DATA_SURFACE_CM_G12 = CODECHAL_VDENC_HME_BEGIN_G12,
    CODECHAL_VDENC_16xME_MV_DATA_SURFACE_CM_G12,
    CODECHAL_VDENC_32xME_MV_DATA_SURFACE_CM_G12 = CODECHAL_VDENC_16xME_MV_DATA_SURFACE_CM_G12,
    CODECHAL_VDENC_HME_DISTORTION_SURFACE_CM_G12,
    CODECHAL_VDENC_HME_BRC_DISTORTION_CM_G12,
    CODECHAL_VDENC_HME_CURR_FOR_FWD_REF_CM_G12,
    CODECHAL_VDENC_HME_FWD_REF_IDX0_CM_G12,
    CODECHAL_VDENC_HME_RESERVED1_CM_G12,
    CODECHAL_VDENC_HME_FWD_REF_IDX1_CM_G12,
    CODECHAL_VDENC_HME_RESERVED2_CM_G12,
    CODECHAL_VDENC_HME_FWD_REF_IDX2_CM_G12,
    CODECHAL_VDENC_HME_RESERVED3_CM_G12,
    CODECHAL_VDENC_HME_FWD_REF_IDX3_CM_G12,
    CODECHAL_VDENC_HME_RESERVED4_CM_G12,
    CODECHAL_VDENC_HME_FWD_REF_IDX4_CM_G12,
    CODECHAL_VDENC_HME_RESERVED5_CM_G12,
    CODECHAL_VDENC_HME_FWD_REF_IDX5_CM_G12,
    CODECHAL_VDENC_HME_RESERVED6_CM_G12,
    CODECHAL_VDENC_HME_FWD_REF_IDX6_CM_G12,
    CODECHAL_VDENC_HME_RESERVED7_CM_G12,
    CODECHAL_VDENC_HME_FWD_REF_IDX7_CM_G12,
    CODECHAL_VDENC_HME_RESERVED8_CM_G12,
    CODECHAL_VDENC_HME_CURR_FOR_BWD_REF_CM_G12,
    CODECHAL_VDENC_HME_BWD_REF_IDX0_CM_G12,
    CODECHAL_VDENC_HME_RESERVED9_CM_G12,
    CODECHAL_VDENC_HME_BWD_REF_IDX1_CM_G12,
    CODECHAL_VDENC_HME_RESERVED10_CM_G12,
    CODECHAL_VDENC_HME_VDENC_STREAMIN_OUTPUT_CM_G12,
    CODECHAL_VDENC_HME_VDENC_STREAMIN_INPUT_CM_G12,
    CODECHAL_VDENC_HME_END_G12,
};

struct MEDIA_OBJECT_HEVC_VP9_VDENC_ME_CURBE_G12
{
    // DW0
    union
    {
        struct
        {
            uint32_t   SkipModeEn : MOS_BITFIELD_BIT(0);
            uint32_t   AdaptiveEn : MOS_BITFIELD_BIT(1);
            uint32_t   BiMixDis : MOS_BITFIELD_BIT(2);
            uint32_t            : MOS_BITFIELD_RANGE(3, 4);
            uint32_t   EarlyImeSuccessEn : MOS_BITFIELD_BIT(5);
            uint32_t            : MOS_BITFIELD_BIT(6);
            uint32_t   T8x8FlagForInterEn : MOS_BITFIELD_BIT(7);
            uint32_t            : MOS_BITFIELD_RANGE(8, 23);
            uint32_t   EarlyImeStop : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   MaxNumMVs : MOS_BITFIELD_RANGE(0, 5);
            uint32_t            : MOS_BITFIELD_RANGE(6, 15);
            uint32_t   BiWeight : MOS_BITFIELD_RANGE(16, 21);
            uint32_t            : MOS_BITFIELD_RANGE(22, 27);
            uint32_t   UniMixDisable : MOS_BITFIELD_BIT(28);
            uint32_t            : MOS_BITFIELD_RANGE(29, 31);
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
            uint32_t   MaxLenSP : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   MaxNumSU : MOS_BITFIELD_RANGE(8, 15);
            uint32_t            : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   SrcSize : MOS_BITFIELD_RANGE(0, 1);
            uint32_t            : MOS_BITFIELD_RANGE(2, 3);
            uint32_t   MbTypeRemap : MOS_BITFIELD_RANGE(4, 5);
            uint32_t   SrcAccess : MOS_BITFIELD_BIT(6);
            uint32_t   RefAccess : MOS_BITFIELD_BIT(7);
            uint32_t   SearchCtrl : MOS_BITFIELD_RANGE(8, 10);
            uint32_t   DualSearchPathOption : MOS_BITFIELD_BIT(11);
            uint32_t   SubPelMode : MOS_BITFIELD_RANGE(12, 13);
            uint32_t   SkipType : MOS_BITFIELD_BIT(14);
            uint32_t   DisableFieldCacheAlloc : MOS_BITFIELD_BIT(15);
            uint32_t   InterChromaMode : MOS_BITFIELD_BIT(16);
            uint32_t   FTEnable : MOS_BITFIELD_BIT(17);
            uint32_t   BMEDisableFBR : MOS_BITFIELD_BIT(18);
            uint32_t   BlockBasedSkipEnable : MOS_BITFIELD_BIT(19);
            uint32_t   InterSAD : MOS_BITFIELD_RANGE(20, 21);
            uint32_t   IntraSAD : MOS_BITFIELD_RANGE(22, 23);
            uint32_t   SubMbPartMask : MOS_BITFIELD_RANGE(24, 30);
            uint32_t            : MOS_BITFIELD_BIT(31);
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
            uint32_t    : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   PictureHeightMinus1 : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   PictureWidth : MOS_BITFIELD_RANGE(16, 23);
            uint32_t: MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t    : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   QpPrimeY : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   RefWidth : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   RefHeight : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t    : MOS_BITFIELD_BIT(0);
            uint32_t   InputStreamInEn : MOS_BITFIELD_BIT(1);
            uint32_t   LCUSize : MOS_BITFIELD_BIT(2);
            uint32_t   WriteDistortions : MOS_BITFIELD_BIT(3);
            uint32_t   UseMvFromPrevStep : MOS_BITFIELD_BIT(4);
            uint32_t    : MOS_BITFIELD_RANGE(5, 7);
            uint32_t   SuperCombineDist : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   MaxVmvR : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t    : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   MVCostScaleFactor : MOS_BITFIELD_RANGE(16, 17);
            uint32_t   BilinearEnable : MOS_BITFIELD_BIT(18);
            uint32_t   SrcFieldPolarity : MOS_BITFIELD_BIT(19);
            uint32_t   WeightedSADHAAR : MOS_BITFIELD_BIT(20);
            uint32_t   AConlyHAAR : MOS_BITFIELD_BIT(21);
            uint32_t   RefIDCostMode : MOS_BITFIELD_BIT(22);
            uint32_t    : MOS_BITFIELD_BIT(23);
            uint32_t   SkipCenterMask : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   Mode0Cost : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   Mode1Cost : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   Mode2Cost : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   Mode3Cost : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   Mode4Cost : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   Mode5Cost : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   Mode6Cost : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   Mode7Cost : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   Mode8Cost : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   Mode9Cost : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   RefIDCost : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   ChromaIntraModeCost : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   MV0Cost : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   MV1Cost : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   MV2Cost : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   MV3Cost : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   MV4Cost : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   MV5Cost : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   MV6Cost : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   MV7Cost : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   NumRefIdxL0MinusOne : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   NumRefIdxL1MinusOne : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   RefStreaminCost : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   ROIEnable : MOS_BITFIELD_RANGE(24, 26);
            uint32_t    : MOS_BITFIELD_RANGE(27, 31);
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
            uint32_t   List0RefID0FieldParity : MOS_BITFIELD_BIT(0);
            uint32_t   List0RefID1FieldParity : MOS_BITFIELD_BIT(1);
            uint32_t   List0RefID2FieldParity : MOS_BITFIELD_BIT(2);
            uint32_t   List0RefID3FieldParity : MOS_BITFIELD_BIT(3);
            uint32_t   List0RefID4FieldParity : MOS_BITFIELD_BIT(4);
            uint32_t   List0RefID5FieldParity : MOS_BITFIELD_BIT(5);
            uint32_t   List0RefID6FieldParity : MOS_BITFIELD_BIT(6);
            uint32_t   List0RefID7FieldParity : MOS_BITFIELD_BIT(7);
            uint32_t   List1RefID0FieldParity : MOS_BITFIELD_BIT(8);
            uint32_t   List1RefID1FieldParity : MOS_BITFIELD_BIT(9);
            uint32_t    : MOS_BITFIELD_RANGE(10, 31);
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
            uint32_t   PrevMvReadPosFactor : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   MvShiftFactor : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   Reserved : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   ActualMBWidth : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   ActualMBHeight : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   RoiCtrl : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   MaxTuSize : MOS_BITFIELD_RANGE(8, 9);
            uint32_t   MaxCuSize : MOS_BITFIELD_RANGE(10, 11);
            uint32_t   NumImePredictors : MOS_BITFIELD_RANGE(12, 15);
            uint32_t   Reserved : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   PuTypeCtrl : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   ForceMvx0 : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   ForceMvy0 : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   ForceMvx1 : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   ForceMvy1 : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   ForceMvx2 : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   ForceMvy2 : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   ForceMvx3 : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   ForceMvy3 : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   ForceRefIdx0 : MOS_BITFIELD_RANGE(0, 3);
            uint32_t   ForceRefIdx1 : MOS_BITFIELD_RANGE(4, 7);
            uint32_t   ForceRefIdx2 : MOS_BITFIELD_RANGE(8, 11);
            uint32_t   ForceRefIdx3 : MOS_BITFIELD_RANGE(12, 15);
            uint32_t   NumMergeCandCu8x8 : MOS_BITFIELD_RANGE(16, 19);
            uint32_t   NumMergeCandCu16x16 : MOS_BITFIELD_RANGE(20, 23);
            uint32_t   NumMergeCandCu32x32 : MOS_BITFIELD_RANGE(24, 27);
            uint32_t   NumMergeCandCu64x64 : MOS_BITFIELD_RANGE(28, 31);
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
            uint32_t   SegID : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   QpEnable : MOS_BITFIELD_RANGE(16, 19);
            uint32_t   SegIDEnable : MOS_BITFIELD_BIT(20);
            uint32_t   Reserved : MOS_BITFIELD_RANGE(21, 22);
            uint32_t   ForceRefIdEnable : MOS_BITFIELD_BIT(23);
            uint32_t   Reserved1 : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   ForceQp0 : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   ForceQp1 : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   ForceQp2 : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   ForceQp3 : MOS_BITFIELD_RANGE(24, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW38;

    // DW39
    union
    {
        struct
        {
            uint32_t   Reserved : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW39;

    // DW40
    union
    {
        struct
        {
            uint32_t   _4xMeMvOutputDataSurfIndex : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW40;

    // DW41
    union
    {
        struct
        {
            uint32_t   _16xOr32xMeMvInputDataSurfIndex : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW41;

    // DW42
    union
    {
        struct
        {
            uint32_t   _4xMeOutputDistSurfIndex : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW42;

    // DW43
    union
    {
        struct
        {
            uint32_t   _4xMeOutputBrcDistSurfIndex : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW43;

    // DW44
    union
    {
        struct
        {
            uint32_t   VMEFwdInterPredictionSurfIndex : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW44;

    // DW45
    union
    {
        struct
        {
            uint32_t   VMEBwdInterPredictionSurfIndex : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW45;

    // DW46
    union
    {
        struct
        {
            uint32_t   VDEncStreamInOutputSurfIndex : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW46;

    // DW47
    union
    {
        struct
        {
            uint32_t   VDEncStreamInInputSurfIndex : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW47;
};
C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(MEDIA_OBJECT_HEVC_VP9_VDENC_ME_CURBE_G12)) == 48);

struct CODECHAL_VDENC_HEVC_ME_CURBE_G12
{
    // DW0
    union
    {
        struct
        {
            uint32_t   SkipModeEn : MOS_BITFIELD_BIT(0);
            uint32_t   AdaptiveEn : MOS_BITFIELD_BIT(1);
            uint32_t   BiMixDis : MOS_BITFIELD_BIT(2);
            uint32_t    : MOS_BITFIELD_RANGE(3, 4);
            uint32_t   EarlyImeSuccessEn : MOS_BITFIELD_BIT(5);
            uint32_t    : MOS_BITFIELD_BIT(6);
            uint32_t   T8x8FlagForInterEn : MOS_BITFIELD_BIT(7);
            uint32_t    : MOS_BITFIELD_RANGE(8, 23);
            uint32_t   EarlyImeStop : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   MaxNumMVs : MOS_BITFIELD_RANGE(0, 5);
            uint32_t    : MOS_BITFIELD_RANGE(6, 15);
            uint32_t   BiWeight : MOS_BITFIELD_RANGE(16, 21);
            uint32_t    : MOS_BITFIELD_RANGE(22, 27);
            uint32_t   UniMixDisable : MOS_BITFIELD_BIT(28);
            uint32_t    : MOS_BITFIELD_RANGE(29, 31);
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
            uint32_t   MaxLenSP : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   MaxNumSU : MOS_BITFIELD_RANGE(8, 15);
            uint32_t    : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   SrcSize : MOS_BITFIELD_RANGE(0, 1);
            uint32_t    : MOS_BITFIELD_RANGE(2, 3);
            uint32_t   MbTypeRemap : MOS_BITFIELD_RANGE(4, 5);
            uint32_t   SrcAccess : MOS_BITFIELD_BIT(6);
            uint32_t   RefAccess : MOS_BITFIELD_BIT(7);
            uint32_t   SearchCtrl : MOS_BITFIELD_RANGE(8, 10);
            uint32_t   DualSearchPathOption : MOS_BITFIELD_BIT(11);
            uint32_t   SubPelMode : MOS_BITFIELD_RANGE(12, 13);
            uint32_t   SkipType : MOS_BITFIELD_BIT(14);
            uint32_t   DisableFieldCacheAlloc : MOS_BITFIELD_BIT(15);
            uint32_t   InterChromaMode : MOS_BITFIELD_BIT(16);
            uint32_t   FTEnable : MOS_BITFIELD_BIT(17);
            uint32_t   BMEDisableFBR : MOS_BITFIELD_BIT(18);
            uint32_t   BlockBasedSkipEnable : MOS_BITFIELD_BIT(19);
            uint32_t   InterSAD : MOS_BITFIELD_RANGE(20, 21);
            uint32_t   IntraSAD : MOS_BITFIELD_RANGE(22, 23);
            uint32_t   SubMbPartMask : MOS_BITFIELD_RANGE(24, 30);
            uint32_t    : MOS_BITFIELD_BIT(31);
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
            uint32_t    : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   PictureHeightMinus1 : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   PictureWidth : MOS_BITFIELD_RANGE(16, 23);
            uint32_t    : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t    : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   QpPrimeY : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   RefWidth : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   RefHeight : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t    : MOS_BITFIELD_BIT(0);
            uint32_t   InputStreamInEn : MOS_BITFIELD_BIT(1);
            uint32_t   LCUSize : MOS_BITFIELD_BIT(2);
            uint32_t   WriteDistortions : MOS_BITFIELD_BIT(3);
            uint32_t   UseMvFromPrevStep : MOS_BITFIELD_BIT(4);
            uint32_t    : MOS_BITFIELD_RANGE(5, 7);
            uint32_t   SuperCombineDist : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   MaxVmvR : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t    : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   MVCostScaleFactor : MOS_BITFIELD_RANGE(16, 17);
            uint32_t   BilinearEnable : MOS_BITFIELD_BIT(18);
            uint32_t   SrcFieldPolarity : MOS_BITFIELD_BIT(19);
            uint32_t   WeightedSADHAAR : MOS_BITFIELD_BIT(20);
            uint32_t   AConlyHAAR : MOS_BITFIELD_BIT(21);
            uint32_t   RefIDCostMode : MOS_BITFIELD_BIT(22);
            uint32_t    : MOS_BITFIELD_BIT(23);
            uint32_t   SkipCenterMask : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   Mode0Cost : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   Mode1Cost : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   Mode2Cost : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   Mode3Cost : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   Mode4Cost : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   Mode5Cost : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   Mode6Cost : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   Mode7Cost : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   Mode8Cost : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   Mode9Cost : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   RefIDCost : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   ChromaIntraModeCost : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   MV0Cost : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   MV1Cost : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   MV2Cost : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   MV3Cost : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   MV4Cost : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   MV5Cost : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   MV6Cost : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   MV7Cost : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   NumRefIdxL0MinusOne : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   NumRefIdxL1MinusOne : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   RefStreaminCost : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   ROIEnable : MOS_BITFIELD_RANGE(24, 26);
            uint32_t    : MOS_BITFIELD_RANGE(27, 31);
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
            uint32_t   List0RefID0FieldParity : MOS_BITFIELD_BIT(0);
            uint32_t   List0RefID1FieldParity : MOS_BITFIELD_BIT(1);
            uint32_t   List0RefID2FieldParity : MOS_BITFIELD_BIT(2);
            uint32_t   List0RefID3FieldParity : MOS_BITFIELD_BIT(3);
            uint32_t   List0RefID4FieldParity : MOS_BITFIELD_BIT(4);
            uint32_t   List0RefID5FieldParity : MOS_BITFIELD_BIT(5);
            uint32_t   List0RefID6FieldParity : MOS_BITFIELD_BIT(6);
            uint32_t   List0RefID7FieldParity : MOS_BITFIELD_BIT(7);
            uint32_t   List1RefID0FieldParity : MOS_BITFIELD_BIT(8);
            uint32_t   List1RefID1FieldParity : MOS_BITFIELD_BIT(9);
            uint32_t    : MOS_BITFIELD_RANGE(10, 31);
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
            uint32_t   PrevMvReadPosFactor : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   MvShiftFactor : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   Reserved : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   ActualMBWidth : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   ActualMBHeight : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   RoiCtrl : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   MaxTuSize : MOS_BITFIELD_RANGE(8, 9);
            uint32_t   MaxCuSize : MOS_BITFIELD_RANGE(10, 11);
            uint32_t   NumImePredictors : MOS_BITFIELD_RANGE(12, 15);
            uint32_t   Reserved : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   PuTypeCtrl : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   ForceMvx0 : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   ForceMvy0 : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   ForceMvx1 : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   ForceMvy1 : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   ForceMvx2 : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   ForceMvy2 : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   ForceMvx3 : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   ForceMvy3 : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   ForceRefIdx0 : MOS_BITFIELD_RANGE(0, 3);
            uint32_t   ForceRefIdx1 : MOS_BITFIELD_RANGE(4, 7);
            uint32_t   ForceRefIdx2 : MOS_BITFIELD_RANGE(8, 11);
            uint32_t   ForceRefIdx3 : MOS_BITFIELD_RANGE(12, 15);
            uint32_t   NumMergeCandCu8x8 : MOS_BITFIELD_RANGE(16, 19);
            uint32_t   NumMergeCandCu16x16 : MOS_BITFIELD_RANGE(20, 23);
            uint32_t   NumMergeCandCu32x32 : MOS_BITFIELD_RANGE(24, 27);
            uint32_t   NumMergeCandCu64x64 : MOS_BITFIELD_RANGE(28, 31);
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
            uint32_t   SegID : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   QpEnable : MOS_BITFIELD_RANGE(16, 19);
            uint32_t   SegIDEnable : MOS_BITFIELD_BIT(20);
            uint32_t   Reserved : MOS_BITFIELD_RANGE(21, 22);
            uint32_t   ForceRefIdEnable : MOS_BITFIELD_BIT(23);
            uint32_t   Reserved1 : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   ForceQp0 : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   ForceQp1 : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   ForceQp2 : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   ForceQp3 : MOS_BITFIELD_RANGE(24, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW38;

    // DW39
    union
    {
        struct
        {
            uint32_t   Reserved : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW39;

    // DW40
    union
    {
        struct
        {
            uint32_t   _4xMeMvOutputDataSurfIndex : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW40;

    // DW41
    union
    {
        struct
        {
            uint32_t   _16xOr32xMeMvInputDataSurfIndex : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW41;

    // DW42
    union
    {
        struct
        {
            uint32_t   _4xMeOutputDistSurfIndex : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW42;

    // DW43
    union
    {
        struct
        {
            uint32_t   _4xMeOutputBrcDistSurfIndex : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW43;

    // DW44
    union
    {
        struct
        {
            uint32_t   VMEFwdInterPredictionSurfIndex : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW44;

    // DW45
    union
    {
        struct
        {
            uint32_t   VMEBwdInterPredictionSurfIndex : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW45;

    // DW46
    union
    {
        struct
        {
            uint32_t   VDEncStreamInOutputSurfIndex : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW46;

    // DW47
    union
    {
        struct
        {
            uint32_t   VDEncStreamInInputSurfIndex : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW47;
};
C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(CODECHAL_VDENC_HEVC_ME_CURBE_G12)) == 48);

using PCODECHAL_VDENC_HEVC_ME_CURBE_G12 = CODECHAL_VDENC_HEVC_ME_CURBE_G12 * ;

//!  HEVC VDEnc encoder class for GEN12
/*!
This class defines the member fields, functions for GEN12 platform
*/
class CodechalVdencHevcStateG12 : public CodechalVdencHevcState
{
public:
    static const uint32_t       m_minScaledSurfaceSize = 64;           //!< Minimum scaled surface size
    static const uint32_t       m_brcPakStatsBufSize = 512;            //!< Pak statistic buffer size
    static const uint32_t       m_brcStatsBufSize = 1216;              //!< BRC Statistic buf size: 48DWs (3CLs) of HMDC Frame Stats + 256 DWs (16CLs) of Histogram Stats = 1216 bytes
    static const uint32_t       m_brcConstantSurfaceWidth = 64;        //!< BRC constant surface width
    static const uint32_t       m_brcConstantSurfaceHeight = 35;       //!< BRC constant surface height
    static const uint32_t       m_brcHistoryBufSize = 2304;            //!< BRC history buffer size
    static const uint32_t       m_bframeMeBidirectionalWeight = 32;    //!< B frame bidirection weight

    // HuC tables.
    // These Values are diff for each Gen
    static const int8_t         m_devThreshPB0[8];
    static const int8_t         m_lowDelayDevThreshPB0[8];
    static const int8_t         m_devThreshVBR0[8];
    static const int8_t         m_lowDelayDevThreshVBR0[8];
    static const int8_t         m_devThreshI0[8];
    static const int8_t         m_lowDelayDevThreshI0[8];
    static const uint32_t       m_hucConstantData[];

    static constexpr uint32_t    m_numDevThreshlds = 8;
    static constexpr double      m_devStdFPS = 30.0;
    static constexpr double      m_bpsRatioLow = 0.1;
    static constexpr double      m_bpsRatioHigh = 3.5;
    static constexpr int32_t     m_postMultPB = 50;
    static constexpr int32_t     m_negMultPB = -50;
    static constexpr int32_t     m_posMultVBR = 100;
    static constexpr int32_t     m_negMultVBR = -50;

    static const double          m_devThreshIFPNEG[m_numDevThreshlds / 2];
    static const double          m_devThreshIFPPOS[m_numDevThreshlds / 2];
    static const double          m_devThreshPBFPNEG[m_numDevThreshlds / 2];
    static const double          m_devThreshPBFPPOS[m_numDevThreshlds / 2];
    static const double          m_devThreshVBRNEG[m_numDevThreshlds / 2];
    static const double          m_devThreshVBRPOS[m_numDevThreshlds / 2];
    static const int8_t          m_lowdelayDevThreshPB[m_numDevThreshlds];
    static const int8_t          m_lowdelayDevThreshVBR[m_numDevThreshlds];
    static const int8_t          m_lowdelayDevThreshI[m_numDevThreshlds];
    static const int8_t          m_lowdelayDeltaFrmszI[][8];
    static const int8_t          m_lowdelayDeltaFrmszP[][8];
    static const int8_t          m_lowdelayDeltaFrmszB[][8];

    // VDENC Display interface related
    bool                        m_enableTileReplay = false;                        //!< TileReplay Enable
    uint8_t m_tileRowPass = 0;                                                     //!< Current tile row pass

    struct CODECHAL_HEVC_VIRTUAL_ENGINE_OVERRIDE
    {
        union {
            uint8_t       VdBox[MOS_MAX_ENGINE_INSTANCE_PER_CLASS];
            uint64_t      Value;
        };
    };

    PMHW_VDBOX_HCP_TILE_CODING_PARAMS_G12 m_tileParams[CODECHAL_NUM_UNCOMPRESSED_SURFACE_HEVC];  //!< Pointer to the Tile params

    // GEN12 specific resources
    MOS_RESOURCE                m_vdencTileRowStoreBuffer;             //!< Tile row store buffer
    MOS_RESOURCE                m_vdencCumulativeCuCountStreamoutSurface = {};             //!< Cumulative CU Count Streamout Surface
    MOS_RESOURCE                m_vdencPaletteModeStreamOutBuffer = {};                    //!< Palette mode stream out buffer
    MOS_RESOURCE                m_vdencSAORowStoreBuffer;              //!< SAO RowStore buffer
    MOS_RESOURCE                m_resHwCountTileReplay = {};                //!< Tile based HW Counter buffer

    bool                        m_enableTileStitchByHW = false;        //!< Enable HW to stitch commands in scalable mode
    bool                        m_enableHWSemaphore = false;           //!< Enable HW semaphore
    bool                        m_enableVdBoxHWSemaphore = false;      //!< Enable VDBOX HW semaphore

    unsigned char               m_slotForRecNotFiltered = 0;           //!< Slot for not filtered reconstructed surface

    // 3rd Level Batch buffer
    uint32_t                    m_thirdLBSize = 0;                        //!< Size of the 3rd level batch buffer
    MHW_BATCH_BUFFER            m_thirdLevelBatchBuffer;                  //!< 3rd level batch buffer

    // Tile level batch buffer
    uint32_t                    m_tileLevelBatchSize = 0;                 //!< Size of the 2rd level batch buffer for each tile
    uint32_t                    m_numTileBatchAllocated = 0;              //!< The number of allocated batch buffer for tiles
    PMHW_BATCH_BUFFER           m_tileLevelBatchBuffer[CODECHAL_VDENC_BRC_NUM_OF_PASSES];   //!< Tile level batch buffer for each tile

    // scalability
    unsigned char                               m_numPipe            = 1;         //!< Number of pipes
    unsigned char                               m_numPassesInOnePipe = 1;         //!< Number of PAK passes in one pipe
    CODECHAL_ENCODE_BUFFER                      m_resPakSliceLevelStreamoutData;  //!< Surface for slice level stream out data from PAK
    CODECHAL_HEVC_VIRTUAL_ENGINE_OVERRIDE       m_kmdVeOveride;                   //!< KMD override virtual engine index
    uint32_t                                    m_numTiles = 1;                   //!< Number of tiles
    uint32_t                                    m_numLcu = 1;                     //!< LCU number
    CODECHAL_ENCODE_BUFFER                      m_resHcpScalabilitySyncBuffer;    //!< Hcp sync buffer for scalability
    CODECHAL_ENCODE_BUFFER                      m_resTileBasedStatisticsBuffer[CODECHAL_NUM_UNCOMPRESSED_SURFACE_HEVC];
    CODECHAL_ENCODE_BUFFER                      m_resHuCPakAggregatedFrameStatsBuffer;
    CODECHAL_ENCODE_BUFFER                      m_tileRecordBuffer[CODECHAL_NUM_UNCOMPRESSED_SURFACE_HEVC];
    MOS_RESOURCE                                m_resHucPakStitchDmemBuffer[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM][CODECHAL_VDENC_BRC_NUM_OF_PASSES];  //!< HuC Pak Integration Dmem data for each pass
    MOS_RESOURCE                                m_resBrcDataBuffer;                             //!< Resource of bitrate control data buffer
    HEVC_TILE_STATS_INFO                        m_hevcTileStatsOffset = {};                     //!< Page aligned offsets used to program HCP / VDEnc pipe and HuC PAK Integration kernel input
    HEVC_TILE_STATS_INFO                        m_hevcFrameStatsOffset = {};                    //!< Page aligned offsets used to program HuC PAK Integration kernel output, HuC BRC kernel input
    HEVC_TILE_STATS_INFO                        m_hevcStatsSize = {};                           //!< HEVC Statistics size
    bool                                        m_enableTestMediaReset = 0;                     //!< enable media reset test. driver will send cmd to make hang happens
    bool                                        m_forceScalability = false;                     //!< force scalability for resolution < 4K/5K if other checking for scalability passed

    // HuC PAK stitch kernel
    MOS_RESOURCE     m_resHucStitchDataBuffer[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM][CODECHAL_HEVC_MAX_NUM_BRC_PASSES];  // data buffer for huc input cmd generation
    MHW_BATCH_BUFFER m_HucStitchCmdBatchBuffer = {};                                                                        //!< SLB for huc stitch cmd

    // virtual engine
    bool                   m_useVirtualEngine = false;                                                                                                 //!< Virtual engine enable flag
    MOS_COMMAND_BUFFER     m_veBatchBuffer[CODECHAL_NUM_UNCOMPRESSED_SURFACE_HEVC][CODECHAL_HEVC_MAX_NUM_HCP_PIPE][CODECHAL_HEVC_MAX_NUM_BRC_PASSES];  //!< Virtual engine batch buffers
    MOS_COMMAND_BUFFER     m_realCmdBuffer;                                                                                                            //!< Virtual engine command buffer
    uint32_t               m_sizeOfVeBatchBuffer  = 0;                                                                                                 //!< Virtual engine batch buffer size
    CODECHAL_ENCODE_BUFFER m_resBrcSemaphoreMem[CODECHAL_HEVC_MAX_NUM_HCP_PIPE];                                                                       //!< BRC HW semaphore
    CODECHAL_ENCODE_BUFFER m_resVdBoxSemaphoreMem[CODECHAL_HEVC_MAX_NUM_HCP_PIPE];                                                                     //!< VDBox HW semaphore
    CODECHAL_ENCODE_BUFFER m_resBrcPakSemaphoreMem;                                                                                                    //!< BRC PAK HW semaphore
    MOS_RESOURCE           m_resPipeStartSemaMem;                                                                                                      //!< HW semaphore for scalability pipe start at the same time

    int32_t m_curPicSlot = -1;        //!< Slot selected to store current Picutre data
    uint32_t m_prevStoreData = 0;     //!< Previous stored data

    uint32_t m_roundInterValue = 0;
    uint32_t m_roundIntraValue = 0;

    // Information for entry/slot in the picture data
    struct SlotInfo {
        uint32_t age;
        int32_t poc;
        bool isUsed;
        bool isRef;
    } slotInfo[CODECHAL_ENCODE_HEVC_VDENC_WP_DATA_BLOCK_NUMBER] = { { 0, 0, false, false } };

    //!
    //! \brief    Constructor
    //!
    CodechalVdencHevcStateG12(CodechalHwInterface* hwInterface,
        CodechalDebugInterface* debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo);

    //!
    //! \brief    Destructor
    //!
    ~CodechalVdencHevcStateG12();

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
    //! \brief    Help function to check if current PAK pass is first pass for tile replay
    //!
    //! \return   True if current PAK pass is first pass, otherwise return false
    //!
    bool IsFirstPassForTileReplay()
    {
        return (m_tileRowPass == 0) ? true : false;
    }

    //!
    //! \brief    Help function to check if current PAK pass is last pass for tile replay
    //!
    //! \return   True if current PAK pass is last pass for tile replay, otherwise return false
    //!
    bool IsLastPassForTileReplay()
    {
        return (m_tileRowPass == m_NumPassesForTileReplay - 1) ? true : false;
    }

    // inherited virtual functions
    uint32_t GetMaxBtCount() override;
    bool CheckSupportedFormat(PMOS_SURFACE surface) override;
    void SetHcpSliceStateCommonParams(MHW_VDBOX_HEVC_SLICE_STATE& sliceStateParams) override;
    void SetHcpPipeModeSelectParams(MHW_VDBOX_PIPE_MODE_SELECT_PARAMS& pipeModeSelectParams) override;
    void SetHcpPipeBufAddrParams(MHW_VDBOX_PIPE_BUF_ADDR_PARAMS& pipeBufAddrParams) override;
    void SetHcpPicStateParams(MHW_VDBOX_HEVC_PIC_STATE& picStateParams) override;
    MOS_STATUS AddHcpRefIdxCmd(PMOS_COMMAND_BUFFER cmdBuffer, PMHW_BATCH_BUFFER batchBuffer, PMHW_VDBOX_HEVC_SLICE_STATE params) override;
    void SetVdencPipeBufAddrParams(MHW_VDBOX_PIPE_BUF_ADDR_PARAMS& pipeBufAddrParams) override;
    void SetVdencPipeModeSelectParams(MHW_VDBOX_PIPE_MODE_SELECT_PARAMS& pipeModeSelectParams) override;
    MOS_STATUS Initialize(CodechalSetting * settings) override;
    MOS_STATUS InitKernelState() override;
    MOS_STATUS PlatformCapabilityCheck() override;
    MOS_STATUS AllocatePakResources() override;
    MOS_STATUS FreePakResources() override;
    MOS_STATUS AllocateEncResources() override;
    MOS_STATUS FreeEncResources() override;
    MOS_STATUS AllocateBrcResources() override;
    MOS_STATUS FreeBrcResources() override;
    MOS_STATUS InitializePicture(const EncoderParams& params) override;
    MOS_STATUS ValidateRefFrameData(PCODEC_HEVC_ENCODE_SLICE_PARAMS slcParams) override;
    MOS_STATUS SetPictureStructs() override;
    MOS_STATUS GetStatusReport(EncodeStatus* encodeStatus, EncodeStatusReport* encodeStatusReport) override;
    MOS_STATUS UserFeatureKeyReport() override;
    MOS_STATUS EncodeKernelFunctions() override;
    MOS_STATUS ExecutePictureLevel() override;
    MOS_STATUS ExecuteSliceLevel() override;
    MOS_STATUS ConstructBatchBufferHuCBRC(PMOS_RESOURCE batchBuffer) override;
    MOS_STATUS SetDmemHuCBrcInitReset() override;
    MOS_STATUS SetConstDataHuCBrcUpdate() override;
    MOS_STATUS SetDmemHuCBrcUpdate() override;
    MOS_STATUS SetRegionsHuCBrcUpdate(PMHW_VDBOX_HUC_VIRTUAL_ADDR_PARAMS virtualAddrParams) override;
    MOS_STATUS SetDmemHuCPakIntegrate(PMHW_VDBOX_HUC_DMEM_STATE_PARAMS dmemParams);
    MOS_STATUS SetRegionsHuCPakIntegrate(PMHW_VDBOX_HUC_VIRTUAL_ADDR_PARAMS virtualAddrParams);
    MOS_STATUS SetDmemHuCPakIntegrateStitch(PMHW_VDBOX_HUC_DMEM_STATE_PARAMS dmemParams);
    MOS_STATUS SetRegionsHuCPakIntegrateStitch(PMHW_VDBOX_HUC_VIRTUAL_ADDR_PARAMS virtualAddrParams);
    MOS_STATUS HucPakIntegrateStitch(PMOS_COMMAND_BUFFER cmdBuffer);
    MOS_STATUS PrepareVDEncStreamInData() override;
    MOS_STATUS SetGpuCtxCreatOption() override;
    MOS_STATUS ReadSliceSize(PMOS_COMMAND_BUFFER cmdBuffer) override;
    void GetTileInfo(uint32_t xPosition, uint32_t yPosition, uint32_t* tileId, uint32_t* tileEndLCUX, uint32_t* tileEndLCUY);
    void SetStreaminDataPerRegion(
        uint32_t streamInWidth,
        uint32_t top,
        uint32_t bottom,
        uint32_t left,
        uint32_t right,
        PMHW_VDBOX_VDENC_STREAMIN_STATE_PARAMS streaminParams,
        void* streaminData) override;
    void SetBrcRoiDeltaQpMap(
        uint32_t streamInWidth,
        uint32_t top,
        uint32_t bottom,
        uint32_t left,
        uint32_t right,
        uint8_t regionId,
        PDeltaQpForROI deltaQpMap) override;

    void CreateMhwParams() override;

    MOS_STATUS VerifyCommandBufferSize() override;

    MOS_STATUS GetCommandBuffer(PMOS_COMMAND_BUFFER cmdBuffer) override;

    MOS_STATUS ReturnCommandBuffer(PMOS_COMMAND_BUFFER cmdBuffer) override;

    MOS_STATUS SubmitCommandBuffer(
        PMOS_COMMAND_BUFFER cmdBuffer,
        bool  nullRendering) override;

    MOS_STATUS SendPrologWithFrameTracking(
        PMOS_COMMAND_BUFFER         cmdBuffer,
        bool                        frameTrackingRequested,
        MHW_MI_MMIOREGISTERS       *mmioRegister = nullptr) override;

    MOS_STATUS SetSliceStructs() override;

    MOS_STATUS AllocateTileStatistics();

    void SetHcpIndObjBaseAddrParams(MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS& indObjBaseAddrParams) override;

    MOS_STATUS ReadSseStatistics(PMOS_COMMAND_BUFFER cmdBuffer) override;

    MOS_STATUS HuCBrcInitReset() override;

    //!
    //! \brief    Encode at tile level
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS EncTileLevel();

    //!
    //! \brief    Decide number of pipes used for encoding
    //! \details  called inside PlatformCapabilityCheck
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DecideEncodingPipeNumber();

    // overload with more input params
    void SetHcpSliceStateParams(
        MHW_VDBOX_HEVC_SLICE_STATE&           sliceState,
        PCODEC_ENCODER_SLCDATA                slcData,
        uint16_t                              slcCount,
        PMHW_VDBOX_HCP_TILE_CODING_PARAMS_G12 tileCodingParams,
        bool                                  lastSliceInTile,
        uint32_t                              idx);

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
        void                           *binary,
        EncOperation                   operation,
        uint32_t                       krnStateIdx,
        void                           *krnHeader,
        uint32_t                       *krnSize);

    //!
    //! \brief    Init kernel state for HME kernel
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS InitKernelStateMe();

    //!
    //! \brief    Init kernel state for streamin kernel
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS InitKernelStateStreamIn();

    //!
    //! \brief    Set kernel parameters for specific kernel operation
    //!
    //! \param    [in] operation
    //!           Kernel operation
    //! \param    [out] kernelParams
    //!           Pointer to kernel parameters
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetKernelParams(
        EncOperation operation,
        MHW_KERNEL_PARAM *kernelParams);

    //!
    //! \brief    Set binding table for specific kernel operation
    //!
    //! \param    [in] operation
    //!           Kernel operation
    //! \param    [out] bindingTable
    //!           Pointer to binding table
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetBindingTable(
        EncOperation operation,
        PCODECHAL_ENCODE_BINDING_TABLE_GENERIC bindingTable);

    //!
    //! \brief    Invoke HME kernel
    //!
    //! \param    [in] hmeLevel
    //!           HME level like 4x, 16x 32x
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS EncodeMeKernel(
        HmeLevel hmeLevel);

    //!
    //! \brief    Set curbe for HME kernel
    //!
    //! \param    [in] hmeLevel
    //!           HME level like 4x, 16x 32x
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetMeCurbe(
        HmeLevel hmeLevel);
 
    //!
    //! \brief    Set surface state for HME kernel
    //!
    //! \param    [in] hmeLevel
    //!           HME level like 4x, 16x 32x
    //! \param    [in] cmdBuffer
    //!           Pointer to command buffer that surface states are added
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SendMeSurfaces(
        HmeLevel hmeLevel,
        PMOS_COMMAND_BUFFER cmdBuffer);

    //!
    //! \brief    Construct the 3rd level batch buffer
    //!
    //! \param    [in] thirdLevelBatchBuffer
    //!           Pointer to the 3rd level batch  buffer for each tile
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ConstructTLB(PMHW_BATCH_BUFFER thirdLevelBatchBuffer);

    MOS_STATUS AddVdencWalkerStateCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_HEVC_SLICE_STATE params) override;

    //! \brief    Allocate the batch buffer for each tile
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateTileLevelBatch();

    //!
    //! \brief    Free the batch buffer for each tile
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS FreeTileLevelBatch();

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
        bool                                   *sliceInTile,
        bool                                   *lastSliceInTile);

    //!
    //! \brief    Set tile data
    //!
    //! \param    [in] tileCodingParams
    //!           Pointer to tile coding params
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetTileData(MHW_VDBOX_HCP_TILE_CODING_PARAMS_G12*    tileCodingParams);

    //!
    //! \brief    Invoke HuC BRC update
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS HuCBrcUpdate() override;

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

    //! \brief    Check whether Scalability is enabled or not,
    //!           Set number of VDBoxes accordingly
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS GetSystemPipeNumberCommon();

    MOS_STATUS InitMmcState() override;

    MOS_STATUS InitReserveState(CodechalSetting * settings);

    MOS_STATUS UpdateCmdBufAttribute(
        PMOS_COMMAND_BUFFER cmdBuffer,
        bool                renderEngineInUse) override;

    MOS_STATUS AddMediaVfeCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        SendKernelCmdsParams *params) override;

    //!
    //! \brief  Calculate Command Buffer Size
    //!
    //! \return uint32_t
    //!         Command buffer size calculated
    //!
    uint32_t CalculateCommandBufferSize() override;

    //!
    //! \brief  Set Streamin Data Per Lcu
    //!
    //! \param    [in] streaminParams
    //!           Streamin parameters
    //! \param    [in] streaminData
    //!           Streamin data
    //!
    //! \return void
    //!
    void SetStreaminDataPerLcu(
        PMHW_VDBOX_VDENC_STREAMIN_STATE_PARAMS streaminParams,
        void* streaminData) override;

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

    PCODECHAL_ENCODE_SCALABILITY_STATE              m_scalabilityState;   //!< Scalability state

#if USE_CODECHAL_DEBUG_TOOL
    virtual MOS_STATUS DumpVdencOutputs() override;
#endif

    MOS_STATUS SetRoundingValues();

protected:
#ifdef _ENCODE_VDENC_RESERVED
    CodechalVdencHevcG12Rsvd *m_rsvdState = nullptr;
#endif

private:
    // tile row based BRC related
    bool                        m_FrameLevelBRCForTileRow  = false;         //!< Frame level BRC for tile row based encoding 
    bool                        m_TileRowLevelBRC = false;                  //!< Tile Row level BRC for tile row based encoding
    uint32_t                    m_CurrentTileRow  = 0;                      //!< Current tile row number, start from 0
    static const uint32_t       m_NumPassesForTileReplay = 2;               //!< Max number Passes for tile row based BRC
    uint32_t                    m_CurrentPassForTileReplay  = 0;            //!< Current BRC pass number for tile replay
    uint32_t                    m_CurrentPassForOverAll = 0;                //!< Current tile replay pass for overall
    PMHW_BATCH_BUFFER           m_TileRowBRCBatchBuffer[CODECHAL_VDENC_BRC_NUM_OF_PASSES];   //!< Tile level batch buffer HUC BRC Update
    uint32_t                    m_numTileRows = 1;                          //!< Total number of tile rows
    uint32_t                    m_numTileRowBRCBatchAllocated = 0;          //!< The number of allocated batch buffer for tile row BRC
    MOS_RESOURCE                m_resTileRowBRCsyncSemaphore;               //!< HW semaphore buffer for tile row BRC update
    bool                        m_RGBEncodingEnable = false;                //!< Enable RGB encoding
    bool                        m_CaptureModeEnable = false;                //!< Enable Capture mode with display

    // This may be removed to VDENC HW interface later
    static const uint32_t       m_VdboxVDENCRegBase[4];
    //!
    //! \brief    Encode at tile level with tile row based multiple passes
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS EncWithTileRowLevelBRC();

    //!
    //! \brief    Invoke HuC BRC update for tile row
    //!
    //! \param    [in] cmdBuffer
    //!           Pointer to command buffer
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS HuCBrcTileRowUpdate(PMOS_COMMAND_BUFFER cmdBuffer);

    
    //! \brief    Allocate the BRC batch buffer for each tile row
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateTileRowLevelBRCBatch();

    //!
    //! \brief    Free the batch buffer for each tile row level BRC
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS FreeTileRowLevelBRCBatch();

    //!
    //! \brief    Setup Virtual Address Regions for HuC Tile Row BRC update
    //!
    //! \param    [in] virtualAddrParams
    //!           Huc Virtual Address parameters
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetRegionsHuCTileRowBrcUpdate(PMHW_VDBOX_HUC_VIRTUAL_ADDR_PARAMS virtualAddrParams);

    //!
    //! \brief    Configue stitch data buffer as Huc Pak Integration input
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ConfigStitchDataBuffer();

    //!
    //! \brief    Dump HuC based debug output buffers
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DumpHucDebugOutputBuffers();

    MOS_STATUS SetAddCommands(uint32_t commandtype, PMOS_COMMAND_BUFFER cmdBuffer, bool addToBatchBufferHuCBRC, uint32_t roundInterValue, uint32_t roundIntraValue, bool isLowDelayB = true, int8_t *pRefIdxMapping = nullptr, int8_t recNotFilteredID = 0);
#if USE_CODECHAL_DEBUG_TOOL
    MOS_STATUS DumpHucPakIntegrate();
    MOS_STATUS DumpHucCqp();
#endif

};

//! \brief  typedef of class CodechalVdencHevcStateG12*
using PCODECHAL_VDENC_HEVC_STATE_G12 = class CodechalVdencHevcStateG12*;

#endif  // __CODECHAL_VDENC_HEVC_G12_H__
