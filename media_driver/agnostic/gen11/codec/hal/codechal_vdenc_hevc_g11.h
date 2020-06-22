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
//! \file     codechal_vdenc_hevc_g11.h
//! \brief    HEVC VDEnc encoder for GEN11 platform.
//!

#ifndef __CODECHAL_VDENC_HEVC__G11_H__
#define __CODECHAL_VDENC_HEVC__G11_H__

#include "codechal_vdenc_hevc.h"
#include "mhw_vdbox_g11_X.h"
#include "codechal_encode_singlepipe_virtualengine.h"
#include "codechal_encode_scalability.h"

#define  VDBOX_HUC_PAK_INTEGRATION_KERNEL_DESCRIPTOR 15

struct CODECHAL_VDENC_HEVC_HUC_BRC_INIT_DMEM_G11
{
    uint32_t    BRCFunc_U32;                  // 0: Init; 2: Reset
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
    uint8_t     RDOQ_AdaptationEnable_U8;           // 0-disabled, 1-enabled
    uint8_t     RDOQ_IntraPctThreshold_U8;          // 10
    uint8_t     RDOQ_HighIntraDistanceThreshold_U8; // 1
    uint8_t     TopQPDeltaThrForAdapt2Pass_U8;      // 2
    uint8_t     BotQPDeltaThrForAdapt2Pass_U8;      // 1
    uint8_t     RESERVED[2];
    uint8_t     NetworkTraceEnable_U8;                  // 0-disabled, 1-enabled
    uint8_t     LowDelaySceneChangeXFrameSizeEnable_U8; // 0-disabled, 1-enabled
    uint32_t    ACQP_U32;                               // 1
    uint32_t    SlidingWindow_Size_U32;                 // 30
    uint8_t     SlidingWindow_MaxRateRatio_U8;
    uint8_t     RSVD2;

    int8_t      CbQPOffset;                             // -1
    int8_t      CrQPOffset;                             // -1

    uint32_t    ProfileLevelMaxFramePB_U32;
    uint8_t     BRCPyramidEnable_U8;
    uint8_t     LongTermRefEnable_U8;
    uint16_t    LongTermRefInterval_U16;
    uint32_t    RSVD1[6];
};
C_ASSERT(192 == sizeof(CODECHAL_VDENC_HEVC_HUC_BRC_INIT_DMEM_G11));

using PCODECHAL_VDENC_HEVC_HUC_BRC_INIT_DMEM_G11 = CODECHAL_VDENC_HEVC_HUC_BRC_INIT_DMEM_G11*;

struct CODECHAL_VDENC_HEVC_HUC_BRC_UPDATE_DMEM_G11
{
    uint32_t    TARGETSIZE_U32;
    uint32_t    FrameID_U32;                    // frame number
    uint32_t    Ref_L0_FrameID_U32[8];
    uint32_t    Ref_L1_FrameID_U32[8];
    uint16_t    startGAdjFrame_U16[4];          // 10, 50, 100, 150
    uint16_t    TargetSliceSize_U16;
    uint16_t    SLB_Data_SizeInBytes;           // Region12 group 3 batch buffer data size
    uint16_t    PIC_STATE_StartInBytes;         // PIC_STATE starts in byte. 0xFFFF means not available in SLB
    uint16_t    CMD2_StartInBytes;
    uint16_t    CMD1_StartInBytes;
    uint16_t    PIPE_MODE_SELECT_StartInBytes;  // PIPE Mode select starts in byte. 0xFFFF means not available in SLB
    uint16_t    Current_Data_Offset;            // Data block offset of current picture from beginning of the data buffer (region 9)
    uint16_t    Ref_Data_Offset[3];             // Data block offset of ref pictures from beginning of the data buffer (region 9)
    uint16_t    MaxNumSliceAllowed_U16;
    uint8_t     OpMode_U8;                      // 1: BRC, 2: Weighted prediction
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
    uint8_t     RDOQ_Enable_U8;             // 0-disabled, 1-enabled
    uint16_t    NetworkTraceEntry_U16;              // default: 0
    uint16_t    LowDelaySceneChangeXFrameSize_U16;  // default: 0
    int8_t      ReEncodePositiveQPDeltaThr_S8;      // default: 4
    int8_t      ReEncodeNegativeQPDeltaThr_S8;      // default: -10
    uint32_t    NumFrameSkipped;
    uint32_t    SkipFrameSize;
    uint32_t    SliceHeaderSize;
    int8_t      EnableMotionAdaptive;
    uint8_t     RSVD[15];  // 64-byte alignment
};
C_ASSERT(192 == sizeof(CODECHAL_VDENC_HEVC_HUC_BRC_UPDATE_DMEM_G11));

using PCODECHAL_VDENC_HEVC_HUC_BRC_UPDATE_DMEM_G11 = CODECHAL_VDENC_HEVC_HUC_BRC_UPDATE_DMEM_G11*;

struct CODECHAL_VDENC_HEVC_HUC_BRC_CONSTANT_DATA_G11
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
    uint8_t     DistQPAdjTabI[81];
    uint8_t     DistQPAdjTabP[81];
    uint8_t     DistQPAdjTabB[81];
    int8_t      FrmSzAdjTabI_S8[72];
    int8_t      FrmSzAdjTabP_S8[72];
    int8_t      FrmSzAdjTabB_S8[72];
    uint8_t     FrmSzMaxTabI[9];
    uint8_t     FrmSzMaxTabP[9];
    uint8_t     FrmSzMaxTabB[9];
    uint8_t     FrmSzMinTabI[9];
    uint8_t     FrmSzMinTabP[9];
    uint8_t     FrmSzMinTabB[9];
    uint8_t     QPAdjTabI[45];
    uint8_t     QPAdjTabP[45];
    uint8_t     QPAdjTabB[45];
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
    // motion adaptive
    uint8_t    QPAdaptiveWeight[52];
    uint8_t    boostTable[52];
    uint8_t    PenaltyForIntraNonDC32x32PredMode[52];
};

using PCODECHAL_VDENC_HEVC_HUC_BRC_CONSTANT_DATA_G11 = CODECHAL_VDENC_HEVC_HUC_BRC_CONSTANT_DATA_G11*;

//!
//! \struct HucPakStitchDmemVdencG11
//! \brief  The struct of Huc Com Dmem
//!
struct HucPakStitchDmemVdencG11
{
    uint32_t     TileSizeRecord_offset[5];  // Tile Size Records, start offset  in byte, 0xffffffff means unavailable
    uint32_t     VDENCSTAT_offset[5];      // needed for HEVC VDEnc, VP9 VDEnc, start offset  in byte, 0xffffffff means unavailable
    uint32_t     HEVC_PAKSTAT_offset[5]; //needed for HEVC VDEnc, start offset  in byte, 0xffffffff means unavailable
    uint32_t     HEVC_Streamout_offset[5]; //needed for HEVC VDEnc, start offset  in byte, 0xffffffff means unavailable
    uint32_t     VP9_PAK_STAT_offset[5]; //needed for VP9 VDEnc, start offset  in byte, 0xffffffff means unavailable
    uint32_t     Vp9CounterBuffer_offset[5];    //needed for VP9 VDEnc, start offset  in byte, 0xffffffff means unavailable
    uint32_t     LastTileBS_StartInBytes;// last tile in bitstream for region 4 and region 5
    uint32_t     SliceHeaderSizeinBits;           // needed for HEVC dual pipe BRC
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
    uint8_t      RSVD[16];
};

const uint32_t ME_CURBE_INIT[48] =
{
    0x00000000, 0x00200010, 0x00003939, 0x77a43000, 0x00000000, 0x28300000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000200,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
};
//!  HEVC VDEnc encoder class for GEN11
/*!
This class defines the member fields, functions for GEN11 platform
*/
class CodechalVdencHevcStateG11 : public CodechalVdencHevcState
{
public:
    static const uint32_t       m_minScaledSurfaceSize = 64;           //!< Minimum scaled surface size
    static const uint32_t       m_brcPakStatsBufSize = 512;            //!< Pak statistic buffer size
    static const uint32_t       m_brcStatsBufSize = 1216;              //!< BRC Statistic buf size: 48DWs (3CLs) of HMDC Frame Stats + 256 DWs (16CLs) of Histogram Stats = 1216 bytes
    static const uint32_t       m_brcConstantSurfaceWidth = 64;        //!< BRC constant surface width
    static const uint32_t       m_brcConstantSurfaceHeight = 35;       //!< BRC constant surface height
    static const uint32_t       m_brcHistoryBufferSize = 832;          //!< BRC history buffer size
    static const uint32_t       m_bframeMeBidirectionalWeight = 32;    //!< B frame bidirection weight
    static const uint32_t       m_insertOffsetAfterCMD1       = 120;   //!< Huc Initializer CMD1 delta
    static const uint32_t       m_insertOffsetAfterCMD2       = 148;   //!< Huc Initializer CMD2 delta

    // HuC tables, refer to HEVC HuC.
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

    struct CODECHAL_HEVC_VIRTUAL_ENGINE_OVERRIDE
    {
        union {
            uint8_t       VdBox[MOS_MAX_ENGINE_INSTANCE_PER_CLASS];
            uint64_t      Value;
        };
    };

    PMHW_VDBOX_HCP_TILE_CODING_PARAMS_G11       m_tileParams = nullptr;       //!< Pointer to the Tile params

    bool                        m_enableTileStitchByHW = false;          //!< Enable HW to stitch commands in scalable mode
    bool                        m_enableHWSemaphore = false;             //!< Enable HW semaphore
    bool                        m_enableVdBoxHWSemaphore = false;        //!< Enable VDBOX HW semaphore
    // scalability
    unsigned char                         m_numPipe            = 1;         //!< Number of pipes
    unsigned char                         m_numPipePre         = 1;         //!< Number of pipes of previous frame
    unsigned char                         m_numPassesInOnePipe = 1;         //!< Number of PAK passes in one pipe
    CODECHAL_ENCODE_BUFFER                m_resPakSliceLevelStreamoutData;  //!< Surface for slice level stream out data from PAK
    CODECHAL_HEVC_VIRTUAL_ENGINE_OVERRIDE m_kmdVeOveride;                   //!< KMD override virtual engine index
    uint32_t                              m_numTiles = 1;                   //!< Number of tiles
    uint32_t                              m_numLcu = 1;                     //!< LCU number
    CODECHAL_ENCODE_BUFFER                m_resHcpScalabilitySyncBuffer;    //!< Hcp sync buffer for scalability
    CODECHAL_ENCODE_BUFFER                m_resTileBasedStatisticsBuffer[CODECHAL_NUM_UNCOMPRESSED_SURFACE_HEVC];
    CODECHAL_ENCODE_BUFFER                m_tileRecordBuffer[CODECHAL_NUM_UNCOMPRESSED_SURFACE_HEVC];
    CODECHAL_ENCODE_BUFFER                m_resHuCPakAggregatedFrameStatsBuffer;
    HEVC_TILE_STATS_INFO                  m_hevcTileStatsOffset = {};       //!< Page aligned offsets used to program HCP / VDEnc pipe and HuC PAK Integration kernel input
    HEVC_TILE_STATS_INFO                  m_hevcFrameStatsOffset = {};      //!< Page aligned offsets used to program HuC PAK Integration kernel output, HuC BRC kernel input
    HEVC_TILE_STATS_INFO                  m_hevcStatsSize = {};             //!< HEVC Statistics size
    bool                                  m_enableTestMediaReset = 0;  //!< enable media reset test. driver will send cmd to make hang happens

    // HuC PAK stitch kernel
    bool                                        m_hucPakStitchEnabled = false;                                  //!< HuC PAK stitch enabled flag
    MOS_RESOURCE                                m_resHucPakStitchDmemBuffer[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM][CODECHAL_VDENC_BRC_NUM_OF_PASSES];  //!< HuC Pak Integration Dmem data for each pass
    MOS_RESOURCE                                m_resBrcDataBuffer;                                             //!< Resource of bitrate control data buffer

    // virtual engine
    bool                   m_useVirtualEngine = false;                                                                                                 //!< Virtual engine enable flag
    MOS_COMMAND_BUFFER     m_veBatchBuffer[CODECHAL_NUM_UNCOMPRESSED_SURFACE_HEVC][CODECHAL_HEVC_MAX_NUM_HCP_PIPE][CODECHAL_HEVC_MAX_NUM_BRC_PASSES];  //!< Virtual engine batch buffers
    MOS_COMMAND_BUFFER     m_realCmdBuffer;                                                                                                            //!< Virtual engine command buffer
    uint32_t               m_sizeOfVeBatchBuffer  = 0;                                                                                                 //!< Virtual engine batch buffer size
    CODECHAL_ENCODE_BUFFER m_resBrcSemaphoreMem[CODECHAL_HEVC_MAX_NUM_HCP_PIPE];                                                                       //!< BRC HW semaphore
    CODECHAL_ENCODE_BUFFER m_resVdBoxSemaphoreMem[CODECHAL_HEVC_MAX_NUM_HCP_PIPE];                                                                     //!< VDBox HW semaphore
    CODECHAL_ENCODE_BUFFER m_resBrcPakSemaphoreMem;                                                                                                    //!< BRC PAK HW semaphore
    MOS_RESOURCE           m_resPipeStartSemaMem;                                                                                                      //!< HW semaphore for scalability pipe start at the same time
    MOS_RESOURCE           m_resSyncSemaMem;                                                                                                           //!< HW semaphore for sync

    //!
    //! \brief    Constructor
    //!
    CodechalVdencHevcStateG11(CodechalHwInterface* hwInterface,
        CodechalDebugInterface* debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo);

    //!
    //! \brief    Destructor
    //!
    ~CodechalVdencHevcStateG11();
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
    int GetCurrentPass()
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
    bool IsFirstPass()
    {
        return GetCurrentPass() == 0 ? true : false;
    }

    //!
    //! \brief    Help function to check if current PAK pass is last pass
    //!
    //! \return   True if current PAK pass is last pass, otherwise return false
    //!
    bool IsLastPass()
    {
        return GetCurrentPass() == m_numPassesInOnePipe ? true : false;
    }

    //!
    //! \brief    Help function to check if legacy command buffer is used
    //!
    //! \return   True if using legacy command buffer, otherwise return false
    //!
    bool UseLegacyCommandBuffer()
    {
        return (m_numPipe == 1) ||
               (m_osInterface->pfnGetGpuContext(m_osInterface) == m_renderContext);
    }

    // inherited virtual functions
    uint32_t GetMaxBtCount();
    bool CheckSupportedFormat(PMOS_SURFACE surface);
    void SetHcpSliceStateCommonParams(MHW_VDBOX_HEVC_SLICE_STATE& sliceStateParams);
    void SetHcpPipeModeSelectParams(MHW_VDBOX_PIPE_MODE_SELECT_PARAMS& pipeModeSelectParams);
    MOS_STATUS Initialize(CodechalSetting * settings);
    MOS_STATUS InitKernelState();
    MOS_STATUS PlatformCapabilityCheck();
    MOS_STATUS AllocatePakResources();
    MOS_STATUS FreePakResources();
    MOS_STATUS AllocateEncResources();
    MOS_STATUS FreeEncResources();
    MOS_STATUS AllocateBrcResources();
    MOS_STATUS FreeBrcResources();
    MOS_STATUS InitializePicture(const EncoderParams& params);
    MOS_STATUS SetPictureStructs();
    MOS_STATUS GetStatusReport(EncodeStatus* encodeStatus, EncodeStatusReport* encodeStatusReport);
    MOS_STATUS UserFeatureKeyReport();
    MOS_STATUS EncodeKernelFunctions();
    MOS_STATUS ExecutePictureLevel();
    MOS_STATUS ExecuteSliceLevel();
    MOS_STATUS ConstructBatchBufferHuCCQP(PMOS_RESOURCE batchBuffer);
    MOS_STATUS ConstructBatchBufferHuCBRC(PMOS_RESOURCE batchBuffer);
    MOS_STATUS ConstructBatchBufferHuCBRCForGroup3(PMOS_RESOURCE batchBuffer);
    MOS_STATUS ConstructHucCmdForBRC(PMOS_RESOURCE batchBuffer);
    MOS_STATUS SetDmemHuCBrcInitReset();
    MOS_STATUS SetConstDataHuCBrcUpdate();
    MOS_STATUS SetDmemHuCBrcUpdate();
    MOS_STATUS SetRegionsHuCBrcUpdate(PMHW_VDBOX_HUC_VIRTUAL_ADDR_PARAMS virtualAddrParams);
    MOS_STATUS SetDmemHuCPakIntegrate(PMHW_VDBOX_HUC_DMEM_STATE_PARAMS dmemParams);
    MOS_STATUS SetRegionsHuCPakIntegrate(PMHW_VDBOX_HUC_VIRTUAL_ADDR_PARAMS virtualAddrParams);
    MOS_STATUS PrepareVDEncStreamInData();
    MOS_STATUS SetGpuCtxCreatOption();
    MOS_STATUS ReadSliceSize(PMOS_COMMAND_BUFFER cmdBuffer);
    void GetTileInfo(uint32_t xPosition, uint32_t yPosition, uint32_t* tileId, uint32_t* tileEndLCUX, uint32_t* tileEndLCUY);
    void SetStreaminDataPerRegion(
        uint32_t streamInWidth,
        uint32_t top,
        uint32_t bottom,
        uint32_t left,
        uint32_t right,
        PMHW_VDBOX_VDENC_STREAMIN_STATE_PARAMS streaminParams,
        void* streaminData);
    void SetBrcRoiDeltaQpMap(
        uint32_t streamInWidth,
        uint32_t top,
        uint32_t bottom,
        uint32_t left,
        uint32_t right,
        uint8_t regionId,
        PDeltaQpForROI deltaQpMap);
    void CreateMhwParams();
    void SetStreaminDataPerLcu(
        PMHW_VDBOX_VDENC_STREAMIN_STATE_PARAMS streaminParams,
        void* streaminData);

    MOS_STATUS VerifyCommandBufferSize();

    MOS_STATUS GetCommandBuffer(PMOS_COMMAND_BUFFER cmdBuffer);

    MOS_STATUS ReturnCommandBuffer(PMOS_COMMAND_BUFFER cmdBuffer);

    MOS_STATUS SubmitCommandBuffer(
        PMOS_COMMAND_BUFFER cmdBuffer,
        bool  nullRendering);

    MOS_STATUS SendPrologWithFrameTracking(
        PMOS_COMMAND_BUFFER         cmdBuffer,
        bool                        frameTrackingRequested,
        MHW_MI_MMIOREGISTERS       *mmioRegister = nullptr);

    MOS_STATUS SetSliceStructs();

    MOS_STATUS AllocateTileStatistics();

    void SetHcpIndObjBaseAddrParams(MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS& indObjBaseAddrParams);

    void SetHcpPipeBufAddrParams(MHW_VDBOX_PIPE_BUF_ADDR_PARAMS& pipeBufAddrParams);

    MOS_STATUS ReadSseStatistics(PMOS_COMMAND_BUFFER cmdBuffer);

    MOS_STATUS HuCBrcInitReset();

    MOS_STATUS HuCLookaheadInit();

    MOS_STATUS HuCLookaheadUpdate();

    void SetVdencPipeBufAddrParams(
        MHW_VDBOX_PIPE_BUF_ADDR_PARAMS& pipeBufAddrParams);

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
        PMHW_VDBOX_HCP_TILE_CODING_PARAMS_G11 tileCodingParams,
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
        MHW_KERNEL_PARAM* kernelParams);

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
    //! \brief    Dump HuC based debug output buffers
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DumpHucDebugOutputBuffers();

    MOS_STATUS AddVdencWalkerStateCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_HEVC_SLICE_STATE params);

    MOS_STATUS CalculatePictureStateCommandSize();

protected:

    //!
    //! \brief    Set pipe buffer address parameter
    //! \details  Set pipe buffer address parameter in MMC case
    //!
    //! \param    [in] cmdBuffer
    //!           Pointer to MOS command buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetPipeBufAddr(
        PMOS_COMMAND_BUFFER cmdBuffer);

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
        PMHW_VDBOX_HCP_TILE_CODING_PARAMS_G11   currentTile,
        bool                                   *sliceInTile,
        bool                                   *lastSliceInTile);

    //!
    //! \brief    Invoke HuC BRC update
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS HuCBrcUpdate();

    //!
    //! \brief    Set tile data
    //!
    //! \param    [in] tileCodingParams
    //!           Pointer to tile coding params
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetTileData(MHW_VDBOX_HCP_TILE_CODING_PARAMS_G11*  tileCodingParams);

    //!
    //! \brief    Check whether Scalability is enabled or not,
    //!           Set number of VDBoxes accordingly
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS GetSystemPipeNumberCommon();

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

    MOS_STATUS UpdateCmdBufAttribute(
        PMOS_COMMAND_BUFFER cmdBuffer,
        bool                renderEngineInUse);

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

    //!
    //! \brief    Lookahead analysis
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AnalyzeLookaheadStats();

#if USE_CODECHAL_DEBUG_TOOL
    virtual MOS_STATUS DumpHucPakIntegrate();
    virtual MOS_STATUS DumpVdencOutputs();
    virtual MOS_STATUS DumpHucCqp();
#endif

};

//! \brief  typedef of class CodechalVdencHevcStateG11*
using PCODECHAL_VDENC_HEVC__STATE_G11 = class CodechalVdencHevcStateG11*;

#endif  // __CODECHAL_VDENC_HEVC__G11_H__
