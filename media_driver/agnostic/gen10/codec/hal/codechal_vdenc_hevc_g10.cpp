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
//! \file     codechal_vdenc_hevc_g10.cpp
//! \brief    HEVC VDEnc encoder for GEN10.
//!

#include "codechal_vdenc_hevc_g10.h"
#include "codeckrnheader.h"
#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
#include "igcodeckrn_g10.h"
#endif
#include "codechal_huc_cmd_initializer.h"

struct CODECHAL_HEVC_VP9_VDENC_KERNEL_HEADER_G10
{
    int nKernelCount;

    union
    {
        struct
        {
            CODECHAL_KERNEL_HEADER Gen10_HEVC_VP9_VDEnc_DS4X_Frame;
            CODECHAL_KERNEL_HEADER Gen10_HEVC_VP9_VDEnc_DS4X_Field;
            CODECHAL_KERNEL_HEADER Gen10_HEVC_VP9_VDEnc_DS2X_Frame;
            CODECHAL_KERNEL_HEADER Gen10_HEVC_VP9_VDEnc_DS2X_Field;
            CODECHAL_KERNEL_HEADER Gen10_HEVC_VP9_VDEnc_HME_P;
            CODECHAL_KERNEL_HEADER Gen10_HEVC_VP9_VDEnc_HME_B;
            CODECHAL_KERNEL_HEADER Gen10_HEVC_VP9_VDEnc_HME_Streamin;
            CODECHAL_KERNEL_HEADER Gen10_HEVC_VP9_VDEnc_HME_HEVC_Streamin;
            CODECHAL_KERNEL_HEADER Gen10_HEVC_VP9_VDEnc_HMEDetection;
        };
    };

};

struct CODECHAL_VDENC_HEVC_HUC_BRC_INIT_DMEM_G10
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
    uint8_t     ASAO_U8;
    uint8_t     CuQpCtrl_U8;        // 0=No CUQP; 1=CUQP for I-frame; 2=CUQP for P/B-frame

    uint8_t     RSVD0;
    uint8_t     NumSlices_U8;
    uint8_t     ReEncode_U8;
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
    uint8_t     Panic_Enable_U8;                    // 0-disabled, 1-enabled
    uint8_t     TimingBudget_Enable_U8;             // 0-disabled, 1-enabled
    uint8_t     RDOQ_AdaptationEnable_U8;           // 0-disabled, 1-enabled
    uint8_t     RDOQ_IntraPctThreshold_U8;          // 10
    uint8_t     RDOQ_HighIntraDistanceThreshold_U8; // 1
    uint8_t     SAO_DistanceThreshold_U8;           // 5
    uint8_t     TopQPDeltaThrForAdapt2Pass_U8;      // 2
    uint8_t     BotQPDeltaThrForAdapt2Pass_U8;      // 1
    uint8_t     SlidingWindow_MaxRateRatio_U8;      // 120
    uint8_t     RESERVED;
    uint32_t    ACQP_U32;                           // 1
    uint32_t    SlidingWindow_Size_U32;             // 30
    int8_t      CbQPOffset;                         // -1
    int8_t      CrQPOffset;                         // -1
    int8_t      RSVD2[2];
    uint32_t    RSVD1[8];
};
C_ASSERT(192 == sizeof(CODECHAL_VDENC_HEVC_HUC_BRC_INIT_DMEM_G10));

using PCODECHAL_VDENC_HEVC_HUC_BRC_INIT_DMEM_G10 = CODECHAL_VDENC_HEVC_HUC_BRC_INIT_DMEM_G10*;

struct CODECHAL_VDENC_HEVC_HUC_BRC_UPDATE_DMEM_G10
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
    int8_t      ReEncodePositiveQPDeltaThr_S8;      // default: 4
    int8_t      ReEncodeNegativeQPDeltaThr_S8;      // default: -10
    int8_t      RESERVED;
    int32_t     SliceHeaderSize;
    uint8_t     RSVD[28];
};
C_ASSERT(192 == sizeof(CODECHAL_VDENC_HEVC_HUC_BRC_UPDATE_DMEM_G10));

using PCODECHAL_VDENC_HEVC_HUC_BRC_UPDATE_DMEM_G10 = CODECHAL_VDENC_HEVC_HUC_BRC_UPDATE_DMEM_G10*;

struct CODECHAL_VDENC_HEVC_HUC_BRC_CONSTANT_DATA_G10
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
        uint16_t    HaveSliceSaoChromaFlag : 1;             // 0: no slice_sao_chroma_flag in the slice header
        uint16_t    SliceSaoLumaFlag_StartInBits : 15;      // number of bits from beginning of slice header, 0xffff means not awailable
        uint16_t    WeightTable_StartInBits;                // number of bits from beginning of slice header for weight table first bit, 0xffff means not awailable
        uint16_t    WeightTable_EndInBits;                  // number of bits from beginning of slice header for weight table last bit, 0xffff means not awailable
    } Slice[CODECHAL_VDENC_HEVC_MAX_SLICE_NUM];
};

using PCODECHAL_VDENC_HEVC_HUC_BRC_CONSTANT_DATA_G10 = CODECHAL_VDENC_HEVC_HUC_BRC_CONSTANT_DATA_G10*;


const double CodechalVdencHevcStateG10::m_devThreshIFPNEG[] = {
    0.80, 0.60, 0.34, 0.2,
};

const double CodechalVdencHevcStateG10::m_devThreshIFPPOS[] = {
    0.2, 0.4 , 0.66, 0.9,
};

const double CodechalVdencHevcStateG10::m_devThreshPBFPNEG[] = {
    0.90, 0.66, 0.46, 0.3,
};

const double CodechalVdencHevcStateG10::m_devThreshPBFPPOS[] = {
    0.3, 0.46, 0.70, 0.90,
};

const double CodechalVdencHevcStateG10::m_devThreshVBRNEG[] = {
    0.90, 0.70, 0.50, 0.3,
};

const double CodechalVdencHevcStateG10::m_devThreshVBRPOS[] = {
    0.4, 0.5, 0.75, 0.90,
};

const int8_t CodechalVdencHevcStateG10::m_lowdelayDevThreshPB[] = {
    -45, -33, -23, -15, -8, 0, 15, 25,
};
const int8_t CodechalVdencHevcStateG10::m_lowdelayDevThreshVBR[] = {
    -45, -35, -25, -15, -8, 0, 20, 40,
};
const int8_t CodechalVdencHevcStateG10::m_lowdelayDevThreshI[] = {
    -40, -30, -17, -10, -5, 0, 10, 20,
};

const int8_t CodechalVdencHevcStateG10::m_lowdelayDeltaFrmszI[][8] = {
    { 0,  0, -8, -12, -16, -20, -28, -36 },
    { 0,  0, -4, -8, -12,  -16, -24, -32 },
    { 4,  2,  0, -1, -3,  -8, -16, -24 },
    { 8,  4,  2,  0, -1,  -4,  -8, -16 },
    { 20, 16,  4,  0, -1,  -4,  -8, -16 },
    { 24, 20, 16,  8,  4,   0,  -4, -8 },
    { 28, 24, 20, 16,  8,   4,  0, -8 },
    { 32, 24, 20, 16, 8,   4,   0, -4 },
    { 64, 48, 28, 20, 16,  12,  8,  4 },
};

const int8_t CodechalVdencHevcStateG10::m_lowdelayDeltaFrmszP[][8] = {
    { -8,  -24, -32, -40, -44, -48, -52, -80 },
    { -8,  -16, -32, -40, -40,  -44, -44, -56 },
    { 0,    0,  -12, -20, -24,  -28, -32, -36 },
    { 8,   4,  0,   0,    -8,   -16,  -24, -32 },
    { 32,  16,  8, 4,    -4,   -8,  -16,  -20 },
    { 36,  24,  16, 8,    4,    -2,  -4, -8 },
    { 40, 36, 24,   20, 16,  8,  0, -8 },
    { 48, 40, 28,  24, 20,  12,  0, -4 },
    { 64, 48, 28, 20, 16,  12,  8,  4 },
};

const int8_t CodechalVdencHevcStateG10::m_lowdelayDeltaFrmszB[][8] = {
    { 0, -4, -8, -16, -24, -32, -40, -48 },
    { 1,  0, -4, -8, -16,  -24, -32, -40 },
    { 4,  2,  0, -1, -3,  -8, -16, -24 },
    { 8,  4,  2,  0, -1,  -4,  -8, -16 },
    { 20, 16,  4,  0, -1,  -4,  -8, -16 },
    { 24, 20, 16,  8,  4,   0,  -4, -8 },
    { 28, 24, 20, 16,  8,   4,  0, -8 },
    { 32, 24, 20, 16, 8,   4,   0, -4 },
    { 64, 48, 28, 20, 16,  12,  8,  4 },
};

const uint32_t CodechalVdencHevcStateG10::m_hucConstantData[] = {
    0x00030003, 0x00030003, 0x00030003, 0x00030003, 0x00030003, 0x00030003, 0x00030003, 0x00040004,
    0x00060005, 0x00070006, 0x00090008, 0x000B000A, 0x000E000C, 0x00120010, 0x00160014, 0x001C0019,
    0x0023001F, 0x002C0027, 0x00380032, 0x0046003E, 0x0058004F, 0x006F0063, 0x008C007D, 0x00B1009D,
    0x00DF00C6, 0x011800FA, 0x00030003, 0x00030003, 0x00030003, 0x00030003, 0x00030003, 0x00030003,
    0x00040003, 0x00050004, 0x00060005, 0x00070006, 0x00090008, 0x000B000A, 0x000E000D, 0x00120010,
    0x00170014, 0x001D001A, 0x00240021, 0x002E0029, 0x003A0034, 0x00490041, 0x005C0052, 0x00740067,
    0x00920082, 0x00B800A4, 0x00E800CE, 0x01240104, 0x00020002, 0x00020002, 0x00020002, 0x00020002,
    0x00020002, 0x00020002, 0x00030002, 0x00050004, 0x00080006, 0x000C000A, 0x0013000F, 0x001E0018,
    0x00300026, 0x004D003D, 0x007A0061, 0x00C2009A, 0x013300F4, 0x01E80183, 0x03060266, 0x04CD03CF,
    0x079F060C, 0x0C18099A, 0x13330F3D, 0x1E7A1831, 0x30622666, 0x4CCD3CF5, 0x00030003, 0x00030003,
    0x00030003, 0x00030003, 0x00030003, 0x00030003, 0x00030003, 0x00050004, 0x00080007, 0x000D000A,
    0x00150011, 0x0021001A, 0x0034002A, 0x00530042, 0x00840069, 0x00D200A6, 0x014D0108, 0x021001A3,
    0x0347029A, 0x05330421, 0x0841068D, 0x0D1A0A66, 0x14CD1082, 0x21051A35, 0x346A299A, 0x53334209,
    0x01900190, 0x01900190, 0x01900190, 0x01900190, 0x01900190, 0x012c012c, 0x012c012c, 0x012c012c,
    0x012c012c, 0x012c012c, 0x00c800c8, 0x00c800c8, 0x00c800c8, 0x00c800c8, 0x00c800c8, 0x00640064,
    0x00640064, 0x00640064, 0x00640064, 0x00640064, 0x00640064, 0x00640064, 0x00640064, 0x00640064,
    0x00640064, 0x00640064, 0x01900190, 0x01900190, 0x01900190, 0x01900190, 0x01900190, 0x012c012c,
    0x012c012c, 0x012c012c, 0x012c012c, 0x012c012c, 0x00c800c8, 0x00c800c8, 0x00c800c8, 0x00c800c8,
    0x00c800c8, 0x00640064, 0x00640064, 0x00640064, 0x00640064, 0x00640064, 0x00640064, 0x00640064,
    0x00640064, 0x00640064, 0x00640064, 0x00640064, 0x503c1e04, 0xffc88c78, 0x3c1e0400, 0xc88c7850,
    0x140200ff, 0xa0824628, 0x0000ffc8, 0x00000000, 0x04030302, 0x00000000, 0x03030200, 0x0000ff04,
    0x02020000, 0xffff0303, 0x01000000, 0xff020202, 0x0000ffff, 0x02020100, 0x00fffffe, 0x01010000,
    0xfffffe02, 0x010000ff, 0xfefe0201, 0x0000ffff, 0xfe010100, 0x00fffffe, 0x01010000, 0x00000000,
    0x03030200, 0x00000004, 0x03020000, 0x00ff0403, 0x02000000, 0xff030302, 0x000000ff, 0x02020201,
    0x00ffffff, 0x02010000, 0xfffffe02, 0x01000000, 0xfffe0201, 0x0000ffff, 0xfe020101, 0x00fffffe,
    0x01010000, 0xfffffefe, 0x01000000, 0x00000001, 0x03020000, 0x00000403, 0x02000000, 0xff040303,
    0x00000000, 0x03030202, 0x0000ffff, 0x02020100, 0xffffff02, 0x01000000, 0xfffe0202, 0x000000ff,
    0xfe020101, 0x00ffffff, 0x02010100, 0xfffffefe, 0x01000000, 0xfffefe01, 0x000000ff, 0xe0e00101,
    0xc0d0d0d0, 0xe0e0b0c0, 0xd0d0d0e0, 0xf0f0c0d0, 0xd0e0e0e0, 0x0408d0d0, 0xe8f0f800, 0x1820dce0,
    0xf8fc0210, 0x2024ecf0, 0x0008101c, 0x2428f8fc, 0x08101418, 0x2830f800, 0x0c14181c, 0x3040fc00,
    0x0c10141c, 0xe8f80408, 0xc8d0d4e0, 0xf0f8b0c0, 0xccd4d8e0, 0x0000c0c8, 0xd8dce4f0, 0x0408d0d4,
    0xf0f80000, 0x0808dce8, 0xf0f80004, 0x0810dce8, 0x00080808, 0x0810f8fc, 0x08080808, 0x1010f800,
    0x08080808, 0x1020fc00, 0x08080810, 0xfc000408, 0xe0e8f0f8, 0x0001d0d8, 0xe8f0f8fc, 0x0204d8e0,
    0xf8fdff00, 0x0408e8f0, 0xfcff0002, 0x1014f0f8, 0xfcff0004, 0x1418f0f8, 0x00040810, 0x181cf8fc,
    0x04081014, 0x1820f800, 0x04081014, 0x3040fc00, 0x0c10141c, 0x40300408, 0x80706050, 0x30a0a090,
    0x70605040, 0xa0a09080, 0x60504030, 0xa0908070, 0x040201a0, 0x18141008, 0x02012420, 0x0a080604,
    0x01101010, 0x0c080402, 0x10101010, 0x05030201, 0x02010106, 0x00000503, 0xff030201, 0x02010000,
    0x000000ff, 0xfffefe01, 0xfdfd0100, 0xfb00ffff, 0xfffffefd, 0xfefdfbfa, 0x030201ff, 0x01010605,
    0x00050302, 0x03020101, 0x010000ff, 0x0000ff02, 0xffff0100, 0xfe0100ff, 0x00ffffff, 0xfffffefc,
    0xfefcfb00, 0x0101ffff, 0x01050402, 0x04020101, 0x01010000, 0x0000ff02, 0x00ff0101, 0xff000000,
    0x0100ffff, 0xfffffffe, 0xfffefd00, 0xfcfb00ff, 0x1efffffe, 0x070d0e10, 0x00003207, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000,
    0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10,
    0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000,
    0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10,
    0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000,
    0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10,
    0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000,
    0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10,
    0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000,
    0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10,
    0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000,
    0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10,
    0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0xffff0030, 0x01acffff, 0xffffffff, 0x00000000, 0xffffffff, 0xffff0000, 0xffffffff,
    0xffffffff, 0x00000000, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0x00000000, 0xffffffff,
    0xffff0000, 0xffffffff, 0xffffffff, 0x00000000, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff,
    0x00000000, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0x00000000, 0xffffffff, 0xffff0000,
    0xffffffff, 0xffffffff, 0x00000000, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0x00000000,
    0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0x00000000, 0xffffffff, 0xffff0000, 0xffffffff,
    0xffffffff, 0x00000000, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0x00000000, 0xffffffff,
    0xffff0000, 0xffffffff, 0xffffffff, 0x00000000, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff,
    0x00000000, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0x00000000, 0xffffffff, 0xffff0000,
    0xffffffff, 0xffffffff, 0x00000000, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0x00000000,
    0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0x00000000, 0xffffffff, 0xffff0000, 0xffffffff,
    0xffffffff, 0x00000000, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0x00000000, 0xffffffff,
    0xffff0000, 0xffffffff, 0xffffffff, 0x00000000, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff,
    0x00000000, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0x00000000, 0xffffffff, 0xffff0000,
    0xffffffff, 0xffffffff, 0x00000000, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0x00000000,
    0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0x00000000, 0xffffffff, 0xffff0000, 0xffffffff,
    0xffffffff, 0x00000000, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0x00000000, 0xffffffff,
    0xffff0000, 0xffffffff, 0xffffffff, 0x00000000, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff,
    0x00000000, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0x00000000, 0xffffffff, 0xffff0000,
    0xffffffff, 0xffffffff, 0x00000000, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0x00000000,
    0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0x00000000, 0xffffffff, 0xffff0000, 0xffffffff,
    0xffffffff, 0x00000000, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0x00000000, 0xffffffff,
    0xffff0000, 0xffffffff, 0xffffffff, 0x00000000, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff,
    0x00000000, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0x00000000, 0xffffffff, 0xffff0000,
    0xffffffff, 0xffffffff, 0x00000000, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0x00000000,
    0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0x00000000, 0xffffffff, 0xffff0000, 0xffffffff,
    0xffffffff, 0x00000000, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0x00000000, 0xffffffff,
    0xffff0000, 0xffffffff, 0xffffffff, 0x00000000, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff,
    0x00000000, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0x00000000, 0xffffffff, 0xffff0000,
    0xffffffff, 0xffffffff, 0x00000000, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0x00000000,
    0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0x00000000, 0xffffffff, 0xffff0000, 0xffffffff,
    0xffffffff, 0x00000000, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0x00000000, 0xffffffff,
    0xffff0000, 0xffffffff, 0xffffffff, 0x00000000, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff,
    0x00000000, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0x00000000, 0xffffffff, 0xffff0000,
    0xffffffff, 0xffffffff, 0x00000000, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0x00000000,
    0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0x00000000, 0xffffffff, 0xffff0000, 0xffffffff,
    0xffffffff, 0x00000000, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0x00000000, 0xffffffff,
    0xffff0000, 0xffffffff, 0xffffffff, 0x00000000, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff,
    0x00000000, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0x00000000, 0xffffffff, 0xffff0000,
    0xffffffff, 0xffffffff, 0x00000000, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0x00000000,
    0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0x00000000, 0xffffffff, 0xffff0000, 0xffffffff,
    0xffffffff, 0x00000000, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0x00000000, 0xffffffff,
    0xffff0000, 0xffffffff, 0xffffffff, 0x00000000, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff,
    0x00000000, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0x00000000, 0xffffffff
};

const uint32_t CodechalVdencHevcStateG10::m_meCurbeInit[48] =
{
    0x00000000, 0x00200010, 0x00003939, 0x77a43000, 0x00000000, 0x28300000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000200,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
};

MOS_STATUS CodechalVdencHevcStateG10::GetKernelHeaderAndSize(
    void                           *binary,
    EncOperation                   operation,
    uint32_t                       krnStateIdx,
    void                           *krnHeader,
    uint32_t                       *krnSize)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(binary);
    CODECHAL_ENCODE_CHK_NULL_RETURN(krnHeader);
    CODECHAL_ENCODE_CHK_NULL_RETURN(krnSize);

    CODECHAL_HEVC_VP9_VDENC_KERNEL_HEADER_G10* kernelHeaderTable = (CODECHAL_HEVC_VP9_VDENC_KERNEL_HEADER_G10*)binary;
    PCODECHAL_KERNEL_HEADER currKrnHeader = nullptr;

    switch (operation)
    {
    case ENC_SCALING4X:
        currKrnHeader = &kernelHeaderTable->Gen10_HEVC_VP9_VDEnc_DS4X_Frame;
        break;
    case VDENC_ME_P:
        currKrnHeader = &kernelHeaderTable->Gen10_HEVC_VP9_VDEnc_HME_P;
        break;
    case VDENC_ME_B:
        currKrnHeader = &kernelHeaderTable->Gen10_HEVC_VP9_VDEnc_HME_B;
        break;
    case VDENC_STREAMIN:
        currKrnHeader = &kernelHeaderTable->Gen10_HEVC_VP9_VDEnc_HME_Streamin;
        break;
    case VDENC_STREAMIN_HEVC:
        currKrnHeader = &kernelHeaderTable->Gen10_HEVC_VP9_VDEnc_HME_HEVC_Streamin;
        break;
    default:
        CODECHAL_ENCODE_ASSERTMESSAGE("Unsupported ENC mode requested");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    *((PCODECHAL_KERNEL_HEADER)krnHeader) = *currKrnHeader;

    PCODECHAL_KERNEL_HEADER nextKrnHeader = (currKrnHeader + 1);
    PCODECHAL_KERNEL_HEADER invalidEntry = (PCODECHAL_KERNEL_HEADER)(((uint8_t *)binary) + sizeof(*kernelHeaderTable));
    uint32_t nextKrnOffset = *krnSize;

    if (nextKrnHeader < invalidEntry)
    {
        nextKrnOffset = nextKrnHeader->KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT;
    }
    *krnSize = nextKrnOffset - (currKrnHeader->KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);

    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG10::SetKernelParams(
    EncOperation     operation,
    MHW_KERNEL_PARAM *kernelParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(kernelParams);

    auto curbeAlignment = m_stateHeapInterface->pStateHeapInterface->GetCurbeAlignment();

    kernelParams->iThreadCount = m_renderEngineInterface->GetHwCaps()->dwMaxThreads;
    kernelParams->iIdCount = 1;

    switch (operation)
    {
    case VDENC_ME_P:
        kernelParams->iBTCount = CODECHAL_VDENC_HME_END_G10 - CODECHAL_VDENC_HME_BEGIN_G10;
        kernelParams->iCurbeLength = MOS_ALIGN_CEIL(sizeof(MEDIA_OBJECT_HEVC_VP9_VDENC_ME_CURBE_G10), (size_t)curbeAlignment);
        kernelParams->iBlockWidth = 32;
        kernelParams->iBlockHeight = 32;
        break;
    case VDENC_ME_B:
        kernelParams->iBTCount = CODECHAL_VDENC_HME_END_G10 - CODECHAL_VDENC_HME_BEGIN_G10;
        kernelParams->iCurbeLength = MOS_ALIGN_CEIL(sizeof(MEDIA_OBJECT_HEVC_VP9_VDENC_ME_CURBE_G10), (size_t)curbeAlignment);
        kernelParams->iBlockWidth = 32;
        kernelParams->iBlockHeight = 32;
        break;
    case VDENC_STREAMIN:
        kernelParams->iBTCount = CODECHAL_VDENC_HME_END_G10 - CODECHAL_VDENC_HME_BEGIN_G10;
        kernelParams->iCurbeLength = MOS_ALIGN_CEIL(sizeof(MEDIA_OBJECT_HEVC_VP9_VDENC_ME_CURBE_G10), (size_t)curbeAlignment);
        kernelParams->iBlockWidth = 32;
        kernelParams->iBlockHeight = 32;
        break;
    case VDENC_STREAMIN_HEVC:
        kernelParams->iBTCount = CODECHAL_VDENC_HME_END_G10 - CODECHAL_VDENC_HME_BEGIN_G10;
        kernelParams->iCurbeLength = MOS_ALIGN_CEIL(sizeof(MEDIA_OBJECT_HEVC_VP9_VDENC_ME_CURBE_G10), (size_t)curbeAlignment);
        kernelParams->iBlockWidth = 32;
        kernelParams->iBlockHeight = 32;
        break;
    default:
        CODECHAL_ENCODE_ASSERTMESSAGE("Unsupported ENC mode requested");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
    }

    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG10::SetBindingTable(
    EncOperation operation,
    PCODECHAL_ENCODE_BINDING_TABLE_GENERIC bindingTable)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(bindingTable);

    MOS_ZeroMemory(bindingTable, sizeof(*bindingTable));

    switch (operation)
    {
    case VDENC_ME_P:
        bindingTable->dwNumBindingTableEntries = CODECHAL_VDENC_HME_END_G10 - CODECHAL_VDENC_HME_BEGIN_G10;
        bindingTable->dwBindingTableStartOffset = CODECHAL_VDENC_HME_BEGIN_G10;
        break;
    case VDENC_ME_B:
        bindingTable->dwNumBindingTableEntries = CODECHAL_VDENC_HME_END_G10 - CODECHAL_VDENC_HME_BEGIN_G10;
        bindingTable->dwBindingTableStartOffset = CODECHAL_VDENC_HME_BEGIN_G10;
        break;
    case VDENC_STREAMIN:
        bindingTable->dwNumBindingTableEntries = CODECHAL_VDENC_HME_END_G10 - CODECHAL_VDENC_HME_BEGIN_G10;
        bindingTable->dwBindingTableStartOffset = CODECHAL_VDENC_HME_BEGIN_G10;
        break;
    case VDENC_STREAMIN_HEVC:
        bindingTable->dwNumBindingTableEntries = CODECHAL_VDENC_HME_END_G10 - CODECHAL_VDENC_HME_BEGIN_G10;
        bindingTable->dwBindingTableStartOffset = CODECHAL_VDENC_HME_BEGIN_G10;
        break;
    default:
        CODECHAL_ENCODE_ASSERTMESSAGE("Unsupported ENC mode requested");
        return MOS_STATUS_INVALID_PARAMETER;

    }

    for (uint32_t i = 0; i < bindingTable->dwNumBindingTableEntries; i++)
    {
        bindingTable->dwBindingTableEntries[i] = i;
    }

    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG10::InitKernelStateMe()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    auto kernelSize = m_combinedKernelSize;
    CODECHAL_KERNEL_HEADER currKrnHeader;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pfnGetKernelHeaderAndSize(
        m_kernelBinary,
        VDENC_ME_P,
        0,
        &currKrnHeader,
        &kernelSize));

    PMHW_KERNEL_STATE kernelStatePtr = &m_vdencMeKernelState;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetKernelParams(
        VDENC_ME_P,
        &kernelStatePtr->KernelParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetBindingTable(
        VDENC_ME_P,
        &m_vdencMeKernelBindingTable));

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

MOS_STATUS CodechalVdencHevcStateG10::InitKernelStateStreamIn()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    auto kernelSize = m_combinedKernelSize;
    CODECHAL_KERNEL_HEADER currKrnHeader;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pfnGetKernelHeaderAndSize(
        m_kernelBinary,
        VDENC_STREAMIN_HEVC,
        0,
        &currKrnHeader,
        &kernelSize));

    auto kernelStatePtr = &m_vdencStreaminKernelState;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetKernelParams(
        VDENC_STREAMIN_HEVC,
        &kernelStatePtr->KernelParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetBindingTable(
        VDENC_STREAMIN_HEVC,
        &m_vdencStreaminKernelBindingTable));

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

MOS_STATUS CodechalVdencHevcStateG10::InitKernelState()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitKernelStateMe());
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitKernelStateStreamIn());
#endif

    return eStatus;
}

uint32_t CodechalVdencHevcStateG10::GetMaxBtCount()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    uint32_t maxBtCount = 0;

#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
    auto btIdxAlignment = m_stateHeapInterface->pStateHeapInterface->GetBtIdxAlignment();

    // 4x, 16x DS, 4x ME, 16x ME
    uint32_t maxBtCount1 = 2 * (MOS_ALIGN_CEIL(m_scaling4xKernelStates->KernelParams.iBTCount, btIdxAlignment) +
        MOS_ALIGN_CEIL(m_vdencStreaminKernelState.KernelParams.iBTCount, btIdxAlignment) +
        MOS_ALIGN_CEIL(m_vdencMeKernelState.KernelParams.iBTCount, btIdxAlignment));

     // CSC DsCopy Kernel
    uint32_t maxBtCount2 = 2 * (MOS_ALIGN_CEIL(m_cscDsState->GetBTCount(), btIdxAlignment));

    maxBtCount  = MOS_MAX(maxBtCount1, maxBtCount2);
#endif

    return maxBtCount;
}

MOS_STATUS CodechalVdencHevcStateG10::AllocatePakResources()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalVdencHevcState::AllocatePakResources());

    MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
    MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
    allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
    allocParamsForBufferLinear.Format = Format_Buffer;

    // Allocate SSE Source Pixel Row Store Buffer
    m_sizeOfSseSrcPixelRowStoreBufferPerLcu = CODECHAL_CACHELINE_SIZE * (4 + 4) << 1;
    allocParamsForBufferLinear.dwBytes      = m_sizeOfSseSrcPixelRowStoreBufferPerLcu * (m_widthAlignedMaxLcu + 2);
    allocParamsForBufferLinear.pBufName = "SseSrcPixelRowStoreBuffer";

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_resSseSrcPixelRowStoreBuffer));

    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG10::FreePakResources()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    m_osInterface->pfnFreeResource(m_osInterface, &m_resSseSrcPixelRowStoreBuffer);

    return CodechalVdencHevcState::FreePakResources();
}

MOS_STATUS CodechalVdencHevcStateG10::AllocateEncResources()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalVdencHevcState::AllocateEncResources());

    if (m_hmeSupported)
    {
        HmeParams hmeParams;

        MOS_ZeroMemory(&hmeParams, sizeof(hmeParams));
        hmeParams.b4xMeDistortionBufferSupported = true;
        hmeParams.ps16xMeMvDataBuffer            = &m_s16XMeMvDataBuffer;
        hmeParams.ps32xMeMvDataBuffer            = &m_s32XMeMvDataBuffer;
        hmeParams.ps4xMeDistortionBuffer         = &m_s4XMeDistortionBuffer;
        hmeParams.ps4xMeMvDataBuffer             = &m_s4XMeMvDataBuffer;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateResources4xME(&hmeParams));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateResources16xME(&hmeParams));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateResources32xME(&hmeParams));
    }

    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG10::FreeEncResources()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    // Free ME resources
    HmeParams hmeParams;

    MOS_ZeroMemory(&hmeParams, sizeof(hmeParams));
    hmeParams.ps16xMeMvDataBuffer    = &m_s16XMeMvDataBuffer;
    hmeParams.ps32xMeMvDataBuffer    = &m_s32XMeMvDataBuffer;
    hmeParams.ps4xMeDistortionBuffer = &m_s4XMeDistortionBuffer;
    hmeParams.ps4xMeMvDataBuffer     = &m_s4XMeMvDataBuffer;
    DestroyMEResources(&hmeParams);

    return CodechalVdencHevcState::FreeEncResources();
}

MOS_STATUS CodechalVdencHevcStateG10::AllocateBrcResources()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalVdencHevcState::AllocateBrcResources());

    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG10::FreeBrcResources()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    return CodechalVdencHevcState::FreeBrcResources();
}

MOS_STATUS CodechalVdencHevcStateG10::PlatformCapabilityCheck()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;

    m_maxNumROI = ENCODE_VDENC_HEVC_MAX_ROI_NUMBER_G10;
    if (m_hevcPicParams->NumROI > m_maxNumROI)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(eStatus, "Number of ROI exceeded 8 regions");
    }

    return eStatus;
}

void CodechalVdencHevcStateG10::SetStreaminDataPerLcu(
    PMHW_VDBOX_VDENC_STREAMIN_STATE_PARAMS streaminParams,
    void* streaminData)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    PCODECHAL_VDENC_HEVC_STREAMIN_STATE_G10 data = (PCODECHAL_VDENC_HEVC_STREAMIN_STATE_G10) streaminData;
    if (streaminParams->setQpRoiCtrl)
    {
        if (m_vdencNativeROIEnabled)
        {
            data->DW0.RoiCtrl = streaminParams->roiCtrl;
        }
        else
        {
            data->DW7.QpEnable = 0xf;
            data->DW14.ForceQp_0 = streaminParams->forceQp[0];
            data->DW14.ForceQp_1 = streaminParams->forceQp[1];
            data->DW14.ForceQp_2 = streaminParams->forceQp[2];
            data->DW14.ForceQp_3 = streaminParams->forceQp[3];
        }
    }
    else
    {
        data->DW0.MaxTuSize                 = streaminParams->maxTuSize;
        data->DW0.MaxCuSize                 = streaminParams->maxCuSize;
        data->DW0.NumImePredictors          = streaminParams->numImePredictors;
        data->DW0.PuTypeCtrl                = streaminParams->puTypeCtrl;
        data->DW6.NumMergeCandidateCu64x64  = streaminParams->numMergeCandidateCu64x64;
        data->DW6.NumMergeCandidateCu32x32  = streaminParams->numMergeCandidateCu32x32;
        data->DW6.NumMergeCandidateCu16x16  = streaminParams->numMergeCandidateCu16x16;
        data->DW6.NumMergeCandidateCu8x8    = streaminParams->numMergeCandidateCu8x8;
    }
}

MOS_STATUS CodechalVdencHevcStateG10::InitializePicture(const EncoderParams& params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    // common initilization
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalVdencHevcState::InitializePicture(params));

    // HEVC VDEnc SAO + SSC
    // 1. Slice size control will always trigger HuC, even if it is in TU7.
    // 2. If SAO and SSC are both asked by app, driver needs to set SAO off and send slice header offsets to HuC.
    //    Driver does not need to return error.
    // 3. TU7 (or performance mode in VDEnc) does not mean HuC cannot be enabled.
    //    If there is no SSC, but only CQP, we don't enable HuC.
    //    But if SSC is on, we do enable.

    // On Gen10, SSC and SAO cannot be enabled at the same time
    // SAO cannot be enabled for 10 bit
    // temporarily disable SAO for BRC
    if (m_hevcSeqParams->SAO_enabled_flag && (m_hevcSeqParams->SourceBitDepth == 1 || m_hevcSeqParams->SliceSizeControl))
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        CODECHAL_ENCODE_ASSERTMESSAGE("Currently SAO + 10bit, SAO + SSC are not supported.");
    }
    else if (m_hevcSeqParams->SAO_enabled_flag && (m_hevcSeqParams->RateControlMethod == RATECONTROL_CBR || m_hevcSeqParams->RateControlMethod == RATECONTROL_VBR ||
                                                      m_hevcSeqParams->RateControlMethod == RATECONTROL_VCM || m_hevcSeqParams->RateControlMethod == RATECONTROL_QVBR))
    {
        m_hevcSeqParams->SAO_enabled_flag   = false;
        m_numPasses                         = m_numPasses - 1; // one more pass for the 2nd SAO, i.e., BRC0, BRC1, ..., BRCn, and SAOn+1
        m_uc2NdSaoPass                      = 0;
    }

    // TU configuration for RDOQ
    if (m_hevcRdoqEnabled)
    {
        switch (m_hevcSeqParams->TargetUsage)
        {
        case 1: // TU1 Quality
            m_hevcRdoqEnabled           = true;
            m_hevcRdoqAdaptationEnabled = false;
            break;
        case 4: // TU4 Normal
            m_hevcRdoqEnabled           = true;
            m_hevcRdoqAdaptationEnabled = true;
            break;
        case 7: // TU7 Performance
            m_hevcRdoqEnabled           = false;
            m_hevcRdoqAdaptationEnabled = false;
            break;
        }
    }

    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG10::SetMeCurbe(bool using4xMe)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_VDENC_HEVC_ME_CURBE_G10 curbe;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
        &curbe,
        sizeof(CODECHAL_VDENC_HEVC_ME_CURBE_G10),
        m_meCurbeInit,
        sizeof(CODECHAL_VDENC_HEVC_ME_CURBE_G10)));

    PMHW_KERNEL_STATE kernelState = using4xMe ? &m_vdencStreaminKernelState : &m_vdencMeKernelState;
    bool useMvFromPrevStep = using4xMe;
    bool writeDistortions = using4xMe;
    uint32_t scaleFactor = using4xMe ? SCALE_FACTOR_4x : SCALE_FACTOR_16x;

    curbe.DW3.SubPelMode = 3;
    if (m_fieldScalingOutputInterleaved)
    {
        curbe.DW3.SrcAccess =
            curbe.DW3.RefAccess = CodecHal_PictureIsField(m_currOriginalPic) ? 1 : 0;
        curbe.DW7.SrcFieldPolarity = CodecHal_PictureIsBottomField(m_currOriginalPic) ? 1 : 0;
    }

    curbe.DW4.PictureHeightMinus1 = CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameFieldHeight / scaleFactor) - 1;
    curbe.DW4.PictureWidth = CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameWidth / scaleFactor);
    curbe.DW5.QpPrimeY            = m_hevcPicParams->QpY + m_hevcSliceParams->slice_qp_delta;
    curbe.DW6.WriteDistortions = writeDistortions;
    curbe.DW6.UseMvFromPrevStep = useMvFromPrevStep;
    curbe.DW6.SuperCombineDist = 5;//SuperCombineDist_Generic[pHevcSeqParams->TargetUsage]; Harded coded in KCM
    curbe.DW6.MaxVmvR = 511 * 4;

    if (m_pictureCodingType == B_TYPE)
    {
        // This field is irrelevant since we are not using the bi-direct search.
        // set it to 32
        curbe.DW1.BiWeight = m_bframeMeBidirectionalWeight;
        curbe.DW13.NumRefIdxL1MinusOne = m_hevcSliceParams->num_ref_idx_l1_active_minus1;
    }

    if (m_pictureCodingType == P_TYPE || m_pictureCodingType == B_TYPE)
    {
        curbe.DW13.NumRefIdxL0MinusOne = m_hevcSliceParams->num_ref_idx_l0_active_minus1;
    }

    curbe.DW30.ActualMBHeight = (MOS_ALIGN_CEIL(m_frameHeight, CODEC_HEVC_VDENC_LCU_HEIGHT) / 32);
    curbe.DW30.ActualMBWidth = (MOS_ALIGN_CEIL(m_frameWidth, CODEC_HEVC_VDENC_LCU_WIDTH) / 32);
    curbe.DW13.RefStreaminCost = 0;
    // This flag is to indicate the ROI source type instead of indicating ROI is enabled or not
    curbe.DW13.ROIEnable = 0;

    uint8_t meMethod = (m_pictureCodingType == B_TYPE) ? m_bMeMethodGeneric[m_hevcSeqParams->TargetUsage] : m_meMethodGeneric[m_hevcSeqParams->TargetUsage];
    uint8_t tableIdx = (m_pictureCodingType == B_TYPE) ? 1 : 0;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(&(curbe.SPDelta), 14 * sizeof(uint32_t),
        m_encodeSearchPath[tableIdx][meMethod], 14 * sizeof(uint32_t)));

    if (using4xMe)
    {
        //StreamIn CURBE
        curbe.DW6.LCUSize = 1;//Only LCU64 supported by the VDEnc HW
        // Kernel should use driver-prepared stream-in surface during ROI/ MBQP(LCUQP)/ Dirty-Rect
        curbe.DW6.InputStreamInEn = (m_hevcPicParams->NumROI || m_encodeParams.bMbQpDataEnabled || (m_hevcPicParams->NumDirtyRects > 0 && (B_TYPE == m_hevcPicParams->CodingType)));
        curbe.DW31.NumImePredictors = m_imgStateImePredictors;
        curbe.DW31.MaxCuSize = 3;
        curbe.DW31.MaxTuSize = 3;
        switch (m_hevcSeqParams->TargetUsage)
        {
        case 1:
        case 4:
            curbe.DW36.NumMergeCandCu64x64 = 4;
            curbe.DW36.NumMergeCandCu32x32 = 3;
            curbe.DW36.NumMergeCandCu16x16 = 2;
            curbe.DW36.NumMergeCandCu8x8 = 1;
            break;
        case 7:
            curbe.DW36.NumMergeCandCu64x64 = 2;
            curbe.DW36.NumMergeCandCu32x32 = 2;
            curbe.DW36.NumMergeCandCu16x16 = 2;
            curbe.DW36.NumMergeCandCu8x8 = 0;
            break;
        }
    }

    curbe.DW40._4xMeMvOutputDataSurfIndex = CODECHAL_VDENC_HME_MV_DATA_SURFACE_CM_G10;
    curbe.DW41._16xOr32xMeMvInputDataSurfIndex = CODECHAL_VDENC_16xME_MV_DATA_SURFACE_CM_G10;
    curbe.DW42._4xMeOutputDistSurfIndex = CODECHAL_VDENC_HME_DISTORTION_SURFACE_CM_G10;
    curbe.DW43._4xMeOutputBrcDistSurfIndex = CODECHAL_VDENC_HME_BRC_DISTORTION_CM_G10;
    curbe.DW44.VMEFwdInterPredictionSurfIndex = CODECHAL_VDENC_HME_CURR_FOR_FWD_REF_CM_G10;
    curbe.DW45.VMEBwdInterPredictionSurfIndex = CODECHAL_VDENC_HME_CURR_FOR_BWD_REF_CM_G10;
    curbe.DW46.VDEncStreamInOutputSurfIndex = CODECHAL_VDENC_HME_VDENC_STREAMIN_OUTPUT_CM_G10;
    curbe.DW47.VDEncStreamInInputSurfIndex = CODECHAL_VDENC_HME_VDENC_STREAMIN_INPUT_CM_G10;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(kernelState->m_dshRegion.AddData(
        &curbe,
        kernelState->dwCurbeOffset,
        sizeof(curbe)));

    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG10::SendMeSurfaces(bool using4xMe, PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);

    PMOS_SURFACE meMvDataBuffer      = using4xMe ? &m_s4XMeMvDataBuffer : &m_s16XMeMvDataBuffer;
    uint32_t downscaledWidthInMb = using4xMe ?
        m_downscaledWidthInMb4x : m_downscaledWidthInMb16x;
    uint32_t downscaledHeightInMb = using4xMe ?
        m_downscaledHeightInMb4x : m_downscaledHeightInMb16x;
    uint32_t width = MOS_ALIGN_CEIL(downscaledWidthInMb * 32, 64);
    uint32_t height = downscaledHeightInMb * 4 * 10;
    // Force the values
    meMvDataBuffer->dwWidth = width;
    meMvDataBuffer->dwHeight = height;
    meMvDataBuffer->dwPitch = width;

    PMHW_KERNEL_STATE kernelState = using4xMe ? &m_vdencStreaminKernelState : &m_vdencMeKernelState;
    PCODECHAL_ENCODE_BINDING_TABLE_GENERIC bindingTable = using4xMe ?
        &m_vdencStreaminKernelBindingTable : &m_vdencMeKernelBindingTable;
    uint32_t meMvBottomFieldOffset = 0;
    CODECHAL_SURFACE_CODEC_PARAMS surfaceCodecParams;
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(surfaceCodecParams));
    surfaceCodecParams.bIs2DSurface = true;
    surfaceCodecParams.bMediaBlockRW = true;
    surfaceCodecParams.psSurface = meMvDataBuffer;
    surfaceCodecParams.dwOffset = meMvBottomFieldOffset;
    surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value;
    surfaceCodecParams.dwBindingTableOffset = bindingTable->dwBindingTableEntries[CODECHAL_VDENC_HME_MV_DATA_SURFACE_CM_G10];
    surfaceCodecParams.bIsWritable = true;
    surfaceCodecParams.bRenderTarget = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    if (using4xMe)
    {
        // Pass 16x MV to 4x ME operation
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(surfaceCodecParams));
        surfaceCodecParams.bIs2DSurface = true;
        surfaceCodecParams.bMediaBlockRW = true;
        surfaceCodecParams.psSurface             = &m_s16XMeMvDataBuffer;
        surfaceCodecParams.dwOffset = 0;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value;
        surfaceCodecParams.dwBindingTableOffset = bindingTable->dwBindingTableEntries[CODECHAL_VDENC_16xME_MV_DATA_SURFACE_CM_G10];
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));

        MOS_ZeroMemory(&surfaceCodecParams, sizeof(surfaceCodecParams));
        surfaceCodecParams.bIs2DSurface = true;
        surfaceCodecParams.bMediaBlockRW = true;
        surfaceCodecParams.psSurface             = &m_s4XMeDistortionBuffer;
        surfaceCodecParams.dwOffset = 0;
        surfaceCodecParams.dwBindingTableOffset = bindingTable->dwBindingTableEntries[CODECHAL_VDENC_HME_DISTORTION_SURFACE_CM_G10];
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ME_DISTORTION_ENCODE].Value;
        surfaceCodecParams.bIsWritable = true;
        surfaceCodecParams.bRenderTarget = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    PMOS_SURFACE currScaledSurface = using4xMe ? m_trackedBuf->Get4xDsSurface(CODEC_CURR_TRACKED_BUFFER) : m_trackedBuf->Get16xDsSurface(CODEC_CURR_TRACKED_BUFFER);
    MOS_SURFACE refScaledSurface = *currScaledSurface;
    bool currFieldPicture = CodecHal_PictureIsField(m_currOriginalPic) ? true : false;
    bool currBottomField = CodecHal_PictureIsBottomField(m_currOriginalPic) ? true : false;
    uint8_t currVDirection = (!currFieldPicture) ? CODECHAL_VDIRECTION_FRAME :
        ((currBottomField) ? CODECHAL_VDIRECTION_BOT_FIELD : CODECHAL_VDIRECTION_TOP_FIELD);
    uint32_t currScaledBottomFieldOffset = using4xMe ?
        (uint32_t)m_scaledBottomFieldOffset : (uint32_t)m_scaled16xBottomFieldOffset;

    // Setup references 1...n
    // LIST 0 references
    for (uint8_t refIdx = 0; refIdx <= m_hevcSliceParams->num_ref_idx_l0_active_minus1; refIdx++)
    {
        CODEC_PICTURE refPic = m_hevcSliceParams->RefPicList[LIST_0][refIdx];

        if (!CodecHal_PictureIsInvalid(refPic))
        {
            if (refIdx == 0)
            {
                // Current Picture Y - VME
                MOS_ZeroMemory(&surfaceCodecParams, sizeof(surfaceCodecParams));
                surfaceCodecParams.bUseAdvState = true;
                surfaceCodecParams.psSurface = currScaledSurface;
                surfaceCodecParams.dwOffset = currBottomField ? currScaledBottomFieldOffset : 0;
                surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value;
                surfaceCodecParams.dwBindingTableOffset = bindingTable->dwBindingTableEntries[CODECHAL_VDENC_HME_CURR_FOR_FWD_REF_CM_G10];
                surfaceCodecParams.ucVDirection = currVDirection;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                    m_hwInterface,
                    cmdBuffer,
                    &surfaceCodecParams,
                    kernelState));
            }

            bool refFieldPicture = CodecHal_PictureIsField(refPic) ? true : false;
            bool refBottomField = (CodecHal_PictureIsBottomField(refPic)) ? true : false;
            uint8_t refPicIdx       = m_picIdx[refPic.FrameIdx].ucPicIdx;
            uint8_t scaledIdx       = m_refList[refPicIdx]->ucScalingIdx;
            if (using4xMe)
            {
                MOS_SURFACE* p4xSurface = m_trackedBuf->Get4xDsSurface(scaledIdx);
                if (p4xSurface != nullptr)
                {
                    refScaledSurface.OsResource = p4xSurface->OsResource;
                }
                else
                {
                    CODECHAL_ENCODE_ASSERTMESSAGE("NULL pointer of DsSurface");
                }
            }
            else
            {
                MOS_SURFACE* p16xSurface = m_trackedBuf->Get16xDsSurface(scaledIdx);
                if (p16xSurface != nullptr)
                {
                    refScaledSurface.OsResource = p16xSurface->OsResource;
                }
                else
                {
                    CODECHAL_ENCODE_ASSERTMESSAGE("NULL pointer of DsSurface");
                }
            }
            uint32_t refScaledBottomFieldOffset = refBottomField ? currScaledBottomFieldOffset : 0;

            // L0 Reference Picture Y - VME
            MOS_ZeroMemory(&surfaceCodecParams, sizeof(surfaceCodecParams));
            surfaceCodecParams.bUseAdvState = true;
            surfaceCodecParams.psSurface = &refScaledSurface;
            surfaceCodecParams.dwOffset = refBottomField ? refScaledBottomFieldOffset : 0;
            surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value;
            surfaceCodecParams.dwBindingTableOffset = bindingTable->dwBindingTableEntries[CODECHAL_VDENC_HME_FWD_REF_IDX0_CM_G10 + (refIdx * 2)];
            surfaceCodecParams.ucVDirection = !currFieldPicture ? CODECHAL_VDIRECTION_FRAME :
                ((refBottomField) ? CODECHAL_VDIRECTION_BOT_FIELD : CODECHAL_VDIRECTION_TOP_FIELD);
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                m_hwInterface,
                cmdBuffer,
                &surfaceCodecParams,
                kernelState));
        }
    }

    //List1
    for (uint8_t refIdx = 0; refIdx <= m_hevcSliceParams->num_ref_idx_l1_active_minus1; refIdx++)
    {
        CODEC_PICTURE refPic = m_hevcSliceParams->RefPicList[LIST_1][refIdx];

        if (!CodecHal_PictureIsInvalid(refPic))
        {
            if (refIdx == 0)
            {
                // Current Picture Y - VME
                MOS_ZeroMemory(&surfaceCodecParams, sizeof(surfaceCodecParams));
                surfaceCodecParams.bUseAdvState = true;
                surfaceCodecParams.psSurface = currScaledSurface;
                surfaceCodecParams.dwOffset = currBottomField ? currScaledBottomFieldOffset : 0;
                surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value;
                surfaceCodecParams.dwBindingTableOffset = bindingTable->dwBindingTableEntries[CODECHAL_VDENC_HME_CURR_FOR_BWD_REF_CM_G10];
                surfaceCodecParams.ucVDirection = currVDirection;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                    m_hwInterface,
                    cmdBuffer,
                    &surfaceCodecParams,
                    kernelState));
            }

            bool refFieldPicture = CodecHal_PictureIsField(refPic) ? 1 : 0;
            bool refBottomField = (CodecHal_PictureIsBottomField(refPic)) ? 1 : 0;
            uint8_t refPicIdx       = m_picIdx[refPic.FrameIdx].ucPicIdx;
            uint8_t scaledIdx       = m_refList[refPicIdx]->ucScalingIdx;

            if (using4xMe)
            {
                MOS_SURFACE* p4xSurface = m_trackedBuf->Get4xDsSurface(scaledIdx);
                if (p4xSurface != nullptr)
                {
                    refScaledSurface.OsResource = p4xSurface->OsResource;
                }
                else
                {
                    CODECHAL_ENCODE_ASSERTMESSAGE("NULL pointer of DsSurface");
                }
            }
            else
            {
                MOS_SURFACE* p16xSurface = m_trackedBuf->Get16xDsSurface(scaledIdx);
                if (p16xSurface != nullptr)
                {
                    refScaledSurface.OsResource = p16xSurface->OsResource;
                }
                else
                {
                    CODECHAL_ENCODE_ASSERTMESSAGE("NULL pointer of DsSurface");
                }
            }
            uint32_t refScaledBottomFieldOffset = refBottomField ? currScaledBottomFieldOffset : 0;

            // L1 Reference Picture Y - VME
            MOS_ZeroMemory(&surfaceCodecParams, sizeof(surfaceCodecParams));
            surfaceCodecParams.bUseAdvState = true;
            surfaceCodecParams.psSurface = &refScaledSurface;
            surfaceCodecParams.dwOffset = refBottomField ? refScaledBottomFieldOffset : 0;
            surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value;
            surfaceCodecParams.dwBindingTableOffset = bindingTable->dwBindingTableEntries[CODECHAL_VDENC_HME_BWD_REF_IDX0_CM_G10 + (refIdx * 2)];
            surfaceCodecParams.ucVDirection = !currFieldPicture ? CODECHAL_VDIRECTION_FRAME :
                ((refBottomField) ? CODECHAL_VDIRECTION_BOT_FIELD : CODECHAL_VDIRECTION_TOP_FIELD);
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                m_hwInterface,
                cmdBuffer,
                &surfaceCodecParams,
                kernelState));
        }
    }

    if (using4xMe)
    {
        // Send driver-prepared stream-in surface as input during ROI/ MBQP(LCUQP)/ Dirty-Rect
        if (m_hevcPicParams->NumROI || m_encodeParams.bMbQpDataEnabled || (m_hevcPicParams->NumDirtyRects > 0 && (B_TYPE == m_hevcPicParams->CodingType)))
        {
            MOS_ZeroMemory(&surfaceCodecParams, sizeof(surfaceCodecParams));
            surfaceCodecParams.dwSize = MOS_BYTES_TO_DWORDS((MOS_ALIGN_CEIL(m_frameWidth, CODEC_HEVC_VDENC_LCU_WIDTH) / 32) * (MOS_ALIGN_CEIL(m_frameHeight, CODEC_HEVC_VDENC_LCU_HEIGHT) / 32) * CODECHAL_CACHELINE_SIZE);
            surfaceCodecParams.bIs2DSurface = false;
            surfaceCodecParams.presBuffer = &m_resVdencStreamInBuffer[m_currRecycledBufIdx];
            surfaceCodecParams.dwBindingTableOffset = bindingTable->dwBindingTableEntries[CODECHAL_VDENC_HME_VDENC_STREAMIN_INPUT_CM_G10];
            surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_VDENC_STREAMIN_CODEC].Value;
            surfaceCodecParams.bIsWritable = true;
            surfaceCodecParams.bRenderTarget = true;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                m_hwInterface,
                cmdBuffer,
                &surfaceCodecParams,
                kernelState));
        }
        else    // Clear stream-in surface otherwise
        {
            MOS_LOCK_PARAMS lockFlags;
            MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
            lockFlags.WriteOnly = true;

            auto data = (PCODECHAL_VDENC_HEVC_STREAMIN_STATE_G10)m_osInterface->pfnLockResource(
                m_osInterface,
                &m_resVdencStreamInBuffer[m_currRecycledBufIdx],
                &lockFlags);

            CODECHAL_ENCODE_CHK_NULL_RETURN(data);

            MOS_ZeroMemory(
                data,
                (MOS_ALIGN_CEIL(m_frameWidth, 64) / 32) * (MOS_ALIGN_CEIL(m_frameHeight, 64) / 32) * CODECHAL_CACHELINE_SIZE);

            m_osInterface->pfnUnlockResource(
                m_osInterface,
                &m_resVdencStreamInBuffer[m_currRecycledBufIdx]);
        }

        MOS_ZeroMemory(&surfaceCodecParams, sizeof(surfaceCodecParams));
        surfaceCodecParams.dwSize = MOS_BYTES_TO_DWORDS((MOS_ALIGN_CEIL(m_frameWidth, CODEC_HEVC_VDENC_LCU_WIDTH) / 32) * (MOS_ALIGN_CEIL(m_frameHeight, CODEC_HEVC_VDENC_LCU_HEIGHT) / 32) * CODECHAL_CACHELINE_SIZE);
        surfaceCodecParams.bIs2DSurface = false;
        surfaceCodecParams.presBuffer = &m_resVdencStreamInBuffer[m_currRecycledBufIdx];
        surfaceCodecParams.dwBindingTableOffset = bindingTable->dwBindingTableEntries[CODECHAL_VDENC_HME_VDENC_STREAMIN_OUTPUT_CM_G10];
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_VDENC_STREAMIN_CODEC].Value;
        surfaceCodecParams.bIsWritable = true;
        surfaceCodecParams.bRenderTarget = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    return eStatus;

}

MOS_STATUS CodechalVdencHevcStateG10::EncodeMeKernel(bool using4xMe)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    PerfTagSetting perfTag;
    perfTag.Value             = 0;
    perfTag.Mode              = (uint16_t)m_mode & CODECHAL_ENCODE_MODE_BIT_MASK;
    perfTag.CallType          = CODECHAL_ENCODE_PERFTAG_CALL_ME_KERNEL;
    perfTag.PictureCodingType = m_pictureCodingType;
    m_osInterface->pfnSetPerfTag(m_osInterface, perfTag.Value);

    PMHW_KERNEL_STATE kernelState = using4xMe ? &m_vdencStreaminKernelState : &m_vdencMeKernelState;
    CODECHAL_MEDIA_STATE_TYPE encFunctionType = using4xMe ? CODECHAL_MEDIA_STATE_4X_ME : CODECHAL_MEDIA_STATE_16X_ME;

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

    //Setup curbe for StreamIn Kernel
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetMeCurbe(using4xMe));

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

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendMeSurfaces(using4xMe, &cmdBuffer));

    // Dump SSH for ME kernel
    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            encFunctionType,
            MHW_SSH_TYPE,
            kernelState)));

    uint32_t scalingFactor = (using4xMe) ? SCALE_FACTOR_4x : SCALE_FACTOR_16x;
    uint32_t resolutionX = CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_frameWidth / scalingFactor);
    uint32_t resolutionY = CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameFieldHeight / scalingFactor);

    CODECHAL_WALKER_CODEC_PARAMS walkerCodecParams;
    MOS_ZeroMemory(&walkerCodecParams, sizeof(walkerCodecParams));
    walkerCodecParams.WalkerMode = m_walkerMode;
    walkerCodecParams.dwResolutionX = resolutionX;
    walkerCodecParams.dwResolutionY = resolutionY;
    walkerCodecParams.bNoDependency = true;
    walkerCodecParams.bMbaff = false;
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

    m_hwInterface->UpdateSSEuForCmdBuffer(&cmdBuffer, m_singleTaskPhaseSupported, m_lastTaskInPhase);

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, m_renderContextUsesNullHw);
        m_lastTaskInPhase = false;
    }

    if (!m_singleTaskPhaseSupported)
    {
        m_osInterface->pfnResetPerfBufferID(m_osInterface);
    }

    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG10::EncodeKernelFunctions()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
    if (m_pictureCodingType == P_TYPE)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("GEN10 HEVC VDENC does not support P slice");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    if (m_cscDsState->RequireCsc())
    {
        CODECHAL_ENCODE_CHK_NULL_RETURN(m_cscDsState);

        m_firstTaskInPhase = true;
        m_lastTaskInPhase  = true;
        CodechalEncodeCscDs::KernelParams cscScalingKernelParams;
        // Csc ARGB linear to NV12 Tile Y studio range
        MOS_ZeroMemory(&cscScalingKernelParams, sizeof(cscScalingKernelParams));
        cscScalingKernelParams.bLastTaskInPhaseCSC   = true;
        cscScalingKernelParams.bLastTaskInPhase4xDS  = false;
        cscScalingKernelParams.bLastTaskInPhase16xDS = false;
        cscScalingKernelParams.bLastTaskInPhase32xDS = false;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cscDsState->CscKernel(&cscScalingKernelParams));
    }

    CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
        m_rawSurfaceToEnc,
        CodechalDbgAttr::attrEncodeRawInputSurface,
        "SrcSurf")));
    bool singleTaskPhaseSupported = m_singleTaskPhaseSupported ? true : false;    // local variable to save current setting before overwriting
    if (m_16xMeSupported && (1 == m_hevcSeqParams->TargetUsage))
    {
        m_singleTaskPhaseSupported = false;

        CodechalEncodeCscDs::KernelParams cscScalingKernelParams;
        MOS_ZeroMemory(&cscScalingKernelParams, sizeof(cscScalingKernelParams));

        //4x Downscaling
        cscScalingKernelParams.b32xScalingInUse = false;
        cscScalingKernelParams.b16xScalingInUse = false;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cscDsState->DsKernel(&cscScalingKernelParams));

        //16x Downscaling - 4x downscaled images used as the input for 16x downscaling
        cscScalingKernelParams.b16xScalingInUse = true;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cscDsState->DsKernel(&cscScalingKernelParams));
    }

    if (m_b16XMeEnabled)
    {
        //HME_P kernel for 16xME
        CODECHAL_ENCODE_CHK_STATUS_RETURN(EncodeMeKernel(false));

        //StreamIn kernel, 4xME
        CODECHAL_ENCODE_CHK_STATUS_RETURN(EncodeMeKernel(true));
    }

    // retrieve SingleTaskPhase setting (SAO will need STP enabled setting)
    m_singleTaskPhaseSupported = singleTaskPhaseSupported;

    CODECHAL_DEBUG_TOOL(
        if (m_hmeEnabled) {
            CODECHAL_ME_OUTPUT_PARAMS meOutputParams;

            MOS_ZeroMemory(&meOutputParams, sizeof(meOutputParams));
            meOutputParams.psMeMvBuffer            = &m_s4XMeMvDataBuffer;
            meOutputParams.psMeBrcDistortionBuffer = nullptr;
            meOutputParams.psMeDistortionBuffer    = &m_s4XMeDistortionBuffer;
            meOutputParams.b16xMeInUse = false;
            meOutputParams.b32xMeInUse = false;

            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                &meOutputParams.psMeMvBuffer->OsResource,
                CodechalDbgAttr::attrOutput,
                "MvData",
                meOutputParams.psMeMvBuffer->dwHeight *meOutputParams.psMeMvBuffer->dwPitch,
                CodecHal_PictureIsBottomField(m_currOriginalPic) ? MOS_ALIGN_CEIL((m_downscaledWidthInMb4x * 32), 64) * (m_downscaledFrameFieldHeightInMb4x * 4) : 0,
                CODECHAL_MEDIA_STATE_4X_ME));

            if (meOutputParams.psMeBrcDistortionBuffer)
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                    &meOutputParams.psMeBrcDistortionBuffer->OsResource,
                    CodechalDbgAttr::attrOutput,
                    "BrcDist",
                    meOutputParams.psMeBrcDistortionBuffer->dwHeight *meOutputParams.psMeBrcDistortionBuffer->dwPitch,
                    CodecHal_PictureIsBottomField(m_currOriginalPic) ? MOS_ALIGN_CEIL((m_downscaledWidthInMb4x * 8), 64) * MOS_ALIGN_CEIL((m_downscaledFrameFieldHeightInMb4x * 4), 8) : 0,
                    CODECHAL_MEDIA_STATE_4X_ME));
            }
            if (meOutputParams.psMeDistortionBuffer)
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                    &meOutputParams.psMeDistortionBuffer->OsResource,
                    CodechalDbgAttr::attrOutput,
                    "MeDist",
                    meOutputParams.psMeDistortionBuffer->dwHeight *meOutputParams.psMeDistortionBuffer->dwPitch,
                    CodecHal_PictureIsBottomField(m_currOriginalPic) ? MOS_ALIGN_CEIL((m_downscaledWidthInMb4x * 8), 64) * MOS_ALIGN_CEIL((m_downscaledFrameFieldHeightInMb4x * 4 * 10), 8) : 0,
                    CODECHAL_MEDIA_STATE_4X_ME));
            }
            if (m_vdencStreamInEnabled)
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                    &m_resVdencStreamInBuffer[m_currRecycledBufIdx],
                    CodechalDbgAttr::attrOutput,
                    "MvData",
                    (MOS_ALIGN_CEIL(m_frameHeight, 32) * (MOS_ALIGN_CEIL(m_frameWidth, 32)) / 16),
                    0,
                    CODECHAL_MEDIA_STATE_ME_VDENC_STREAMIN));
            }
            if (m_b16XMeEnabled)
            {
                MOS_ZeroMemory(&meOutputParams, sizeof(meOutputParams));
                meOutputParams.psMeMvBuffer            = &m_s16XMeMvDataBuffer;
                meOutputParams.psMeBrcDistortionBuffer = nullptr;
                meOutputParams.psMeDistortionBuffer = nullptr;
                meOutputParams.b16xMeInUse = true;
                meOutputParams.b32xMeInUse = false;

                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                    &meOutputParams.psMeMvBuffer->OsResource,
                    CodechalDbgAttr::attrOutput,
                    "MvData",
                    meOutputParams.psMeMvBuffer->dwHeight *meOutputParams.psMeMvBuffer->dwPitch,
                    CodecHal_PictureIsBottomField(m_currOriginalPic) ? MOS_ALIGN_CEIL((m_downscaledWidthInMb16x * 32), 64) * (m_downscaledFrameFieldHeightInMb16x * 4) : 0,
                    CODECHAL_MEDIA_STATE_16X_ME));
            }
        })
#endif
    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG10::SetDmemHuCBrcInitReset()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_LOCK_PARAMS lockFlagsWriteOnly;
    MOS_ZeroMemory(&lockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
    lockFlagsWriteOnly.WriteOnly = true;

    PCODECHAL_VDENC_HEVC_HUC_BRC_INIT_DMEM_G10 hucVdencBrcInitDmem = (PCODECHAL_VDENC_HEVC_HUC_BRC_INIT_DMEM_G10)m_osInterface->pfnLockResource(
        m_osInterface, &m_vdencBrcInitDmemBuffer[m_currRecycledBufIdx], &lockFlagsWriteOnly);
    CODECHAL_ENCODE_CHK_NULL_RETURN(hucVdencBrcInitDmem);

    hucVdencBrcInitDmem->BRCFunc_U32 = 0;  // 0: Init, 1: Reset

    hucVdencBrcInitDmem->UserMaxFrame = GetProfileLevelMaxFrameSize();

    hucVdencBrcInitDmem->InitBufFull_U32 = MOS_MIN(m_hevcSeqParams->InitVBVBufferFullnessInBit, m_hevcSeqParams->VBVBufferSizeInBit);
    hucVdencBrcInitDmem->BufSize_U32     = m_hevcSeqParams->VBVBufferSizeInBit;

    hucVdencBrcInitDmem->TargetBitrate_U32 = m_hevcSeqParams->TargetBitRate * CODECHAL_ENCODE_BRC_KBPS;
    hucVdencBrcInitDmem->MaxRate_U32       = m_hevcSeqParams->MaxBitRate * CODECHAL_ENCODE_BRC_KBPS;
    hucVdencBrcInitDmem->MinRate_U32 = 0;

    hucVdencBrcInitDmem->FrameRateM_U32 = m_hevcSeqParams->FrameRate.Numerator;
    hucVdencBrcInitDmem->FrameRateD_U32 = m_hevcSeqParams->FrameRate.Denominator;

    if (m_brcEnabled)
    {
        switch (m_hevcSeqParams->RateControlMethod)
        {
        case RATECONTROL_ICQ:
            hucVdencBrcInitDmem->BRCFlag = 0;
            break;
        case RATECONTROL_CBR:
            hucVdencBrcInitDmem->BRCFlag = 1;
            break;
        case RATECONTROL_VBR:
            hucVdencBrcInitDmem->BRCFlag = 2;
            hucVdencBrcInitDmem->ACQP_U32 = 0;
            break;
        case RATECONTROL_VCM:
            hucVdencBrcInitDmem->BRCFlag = 3;
            break;
        case RATECONTROL_QVBR:
            hucVdencBrcInitDmem->BRCFlag = 2;
            hucVdencBrcInitDmem->ACQP_U32 = m_hevcSeqParams->ICQQualityFactor;
            break;
        default:
            break;
        }

        // Low Delay BRC
        if (m_hevcSeqParams->FrameSizeTolerance == EFRAMESIZETOL_EXTREMELY_LOW)
        {
            hucVdencBrcInitDmem->BRCFlag = 5;
        }
    }
    else if (m_hevcVdencAcqpEnabled)
    {
        hucVdencBrcInitDmem->BRCFlag = 0;
    }

    hucVdencBrcInitDmem->SSCFlag = m_hevcSeqParams->SliceSizeControl;

    hucVdencBrcInitDmem->GopP_U16 = m_hevcSeqParams->GopPicSize - m_hevcSeqParams->NumOfBInGop[0] - 1;
    hucVdencBrcInitDmem->GopB_U16 = (uint16_t)m_hevcSeqParams->NumOfBInGop[0];

    hucVdencBrcInitDmem->FrameWidth_U16 = (uint16_t)m_frameWidth;
    hucVdencBrcInitDmem->FrameHeight_U16 = (uint16_t)m_frameHeight;

    hucVdencBrcInitDmem->GopB1_U16 = (uint16_t)m_hevcSeqParams->NumOfBInGop[1];
    hucVdencBrcInitDmem->GopB2_U16 = (uint16_t)m_hevcSeqParams->NumOfBInGop[2];

    hucVdencBrcInitDmem->MinQP_U8 = m_hevcPicParams->BRCMinQp;
    hucVdencBrcInitDmem->MaxQP_U8 = m_hevcPicParams->BRCMaxQp;

    hucVdencBrcInitDmem->MaxBRCLevel_U8 = 1;
    hucVdencBrcInitDmem->LumaBitDepth_U8 = 8;    // default: 8
    hucVdencBrcInitDmem->ChromaBitDepth_U8 = 8;    // default: 8
    if (m_hevcSeqParams->SourceBitDepth == ENCODE_HEVC_BIT_DEPTH_10)
    {

        hucVdencBrcInitDmem->LumaBitDepth_U8 = 10;    // default: 8
        hucVdencBrcInitDmem->ChromaBitDepth_U8 = 10;    // default: 8
    }
    // 0=No SAO 1=Disable 2nd pass SAO; 2=Enable 2nd pass SAO; 3=Adaptive SAO
    hucVdencBrcInitDmem->ASAO_U8 = m_hevcSeqParams->SAO_enabled_flag ? 2 : 0;

    // 0=No CUQP; 1=CUQP for I-frame; 2=CUQP for P/B-frame
    // bit operation, bit 1 for I-frame, bit 2 for P/B frame
    // In VDENC mode, this field "Cu_Qp_Delta_Enabled_Flag" should always be set to 1.

    if (m_hevcSeqParams->QpAdjustment)
    {
        hucVdencBrcInitDmem->CuQpCtrl_U8 = 3;  // wPictureCodingType I:0, P:1, B:2
    }
    else
    {
        hucVdencBrcInitDmem->CuQpCtrl_U8 = 0;  // wPictureCodingType I:0, P:1, B:2
    }

    if ((hucVdencBrcInitDmem->LowDelayMode_U8 = (m_hevcSeqParams->FrameSizeTolerance == EFRAMESIZETOL_EXTREMELY_LOW)))  // Low Delay BRC
    {

        MOS_SecureMemcpy(hucVdencBrcInitDmem->DevThreshPB0_S8, 8 * sizeof(int8_t), (void *)m_lowdelayDevThreshPB, 8 * sizeof(int8_t));
        MOS_SecureMemcpy(hucVdencBrcInitDmem->DevThreshVBR0_S8, 8 * sizeof(int8_t), (void*)m_lowdelayDevThreshVBR, 8 * sizeof(int8_t));
        MOS_SecureMemcpy(hucVdencBrcInitDmem->DevThreshI0_S8, 8 * sizeof(int8_t), (void*)m_lowdelayDevThreshI, 8 * sizeof(int8_t));
    }
    else
    {
        uint64_t inputbitsperframe = uint64_t(hucVdencBrcInitDmem->MaxRate_U32*100. / (hucVdencBrcInitDmem->FrameRateM_U32 * 100.0 / hucVdencBrcInitDmem->FrameRateD_U32));
        uint64_t vbvsz = hucVdencBrcInitDmem->BufSize_U32;

        double bps_ratio = inputbitsperframe / (vbvsz / m_devStdFPS);
        if (bps_ratio < m_bpsRatioLow) bps_ratio = m_bpsRatioLow;
        if (bps_ratio > m_bpsRatioHigh) bps_ratio = m_bpsRatioHigh;

        for (uint32_t i = 0; i < m_numDevThreshlds / 2; i++) {
            hucVdencBrcInitDmem->DevThreshPB0_S8[i] = (signed char)(m_negMultPB*pow(m_devThreshPBFPNEG[i], bps_ratio));
            hucVdencBrcInitDmem->DevThreshPB0_S8[i + m_numDevThreshlds / 2] = (signed char)(m_postMultPB*pow(m_devThreshPBFPPOS[i], bps_ratio));

            hucVdencBrcInitDmem->DevThreshI0_S8[i] = (signed char)(m_negMultPB*pow(m_devThreshIFPNEG[i], bps_ratio));
            hucVdencBrcInitDmem->DevThreshI0_S8[i + m_numDevThreshlds / 2] = (signed char)(m_postMultPB*pow(m_devThreshIFPPOS[i], bps_ratio));

            hucVdencBrcInitDmem->DevThreshVBR0_S8[i] = (signed char)(m_negMultPB*pow(m_devThreshVBRNEG[i], bps_ratio));
            hucVdencBrcInitDmem->DevThreshVBR0_S8[i + m_numDevThreshlds / 2] = (signed char)(m_posMultVBR*pow(m_devThreshVBRPOS[i], bps_ratio));
        }
    }

    MOS_SecureMemcpy(hucVdencBrcInitDmem->InstRateThreshP0_S8, 4 * sizeof(int8_t), (void*)m_instRateThreshP0, 4 * sizeof(int8_t));
    MOS_SecureMemcpy(hucVdencBrcInitDmem->InstRateThreshB0_S8, 4 * sizeof(int8_t), (void*)m_instRateThreshB0, 4 * sizeof(int8_t));
    MOS_SecureMemcpy(hucVdencBrcInitDmem->InstRateThreshI0_S8, 4 * sizeof(int8_t), (void*)m_instRateThreshI0, 4 * sizeof(int8_t));

    if (m_brcEnabled)
    {
        // initQPIP, initQPB values will be used for BRC in the future
        int32_t initQPIP = 0, initQPB = 0;
        ComputeVDEncInitQP(initQPIP, initQPB);
        hucVdencBrcInitDmem->InitQPIP_U8 = (uint8_t)initQPIP;
        hucVdencBrcInitDmem->InitQPB_U8 = (uint8_t)initQPB;
    }
    else
    {
        hucVdencBrcInitDmem->InitQPIP_U8 = m_hevcPicParams->QpY + m_hevcSliceParams->slice_qp_delta;
        hucVdencBrcInitDmem->InitQPB_U8  = m_hevcPicParams->QpY + m_hevcSliceParams->slice_qp_delta;
    }

    // arch prototype recommendation
    hucVdencBrcInitDmem->TopFrmSzThrForAdapt2Pass_U8 = 32;
    hucVdencBrcInitDmem->BotFrmSzThrForAdapt2Pass_U8 = 24;

    MOS_SecureMemcpy(hucVdencBrcInitDmem->EstRateThreshP0_U8, 7 * sizeof(uint8_t), (void*)m_estRateThreshP0, 7 * sizeof(uint8_t));
    MOS_SecureMemcpy(hucVdencBrcInitDmem->EstRateThreshB0_U8, 7 * sizeof(uint8_t), (void*)m_estRateThreshB0, 7 * sizeof(uint8_t));
    MOS_SecureMemcpy(hucVdencBrcInitDmem->EstRateThreshI0_U8, 7 * sizeof(uint8_t), (void*)m_estRateThreshI0, 7 * sizeof(uint8_t));

    if (m_vdencStreamInEnabled && m_hevcPicParams->NumROI && !m_vdencNativeROIEnabled)
    {
        hucVdencBrcInitDmem->StreamInROIEnable_U8 = 1;
        hucVdencBrcInitDmem->StreamInSurfaceEnable_U8 = 1;
    }

    hucVdencBrcInitDmem->Panic_Enable_U8 = (m_brcEnabled) && (m_panicEnable);

    hucVdencBrcInitDmem->RDOQ_AdaptationEnable_U8 = (uint8_t)m_hevcRdoqAdaptationEnabled;
    hucVdencBrcInitDmem->RDOQ_IntraPctThreshold_U8 = 10;
    hucVdencBrcInitDmem->RDOQ_HighIntraDistanceThreshold_U8 = 2;   // 1-C model, 2-TU configuration table
    hucVdencBrcInitDmem->SAO_DistanceThreshold_U8 = 5;
    hucVdencBrcInitDmem->TopQPDeltaThrForAdapt2Pass_U8 = 2;
    hucVdencBrcInitDmem->BotQPDeltaThrForAdapt2Pass_U8 = 1;
    hucVdencBrcInitDmem->SlidingWindow_Size_U32 = 30;
    hucVdencBrcInitDmem->SlidingWindow_MaxRateRatio_U8 = 120;

    m_osInterface->pfnUnlockResource(m_osInterface, &m_vdencBrcInitDmemBuffer[m_currRecycledBufIdx]);

    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG10::SetConstDataHuCBrcUpdate()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_LOCK_PARAMS lockFlagsWriteOnly;
    MOS_ZeroMemory(&lockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
    lockFlagsWriteOnly.WriteOnly = true;

    PCODECHAL_VDENC_HEVC_HUC_BRC_CONSTANT_DATA_G10 hucConstData =
        (PCODECHAL_VDENC_HEVC_HUC_BRC_CONSTANT_DATA_G10)m_osInterface->pfnLockResource(m_osInterface, &m_vdencBrcConstDataBuffer[m_currRecycledBufIdx], &lockFlagsWriteOnly);
    CODECHAL_ENCODE_CHK_NULL_RETURN(hucConstData);
    MOS_SecureMemcpy(hucConstData, sizeof(m_hucConstantData), m_hucConstantData, sizeof(m_hucConstantData));

    if(m_hevcSeqParams->FrameSizeTolerance == EFRAMESIZETOL_EXTREMELY_LOW)
    {
        const int numEstrateThreshlds = 7;

        for (int i=0; i < numEstrateThreshlds +1; i++)
        {
            for (uint32_t j = 0; j < m_numDevThreshlds + 1; j++)
            {
                hucConstData->FrmSzAdjTabI_S8[(numEstrateThreshlds +1)*j+i]= m_lowdelayDeltaFrmszI[j][i];
                hucConstData->FrmSzAdjTabP_S8[(numEstrateThreshlds +1)*j+i]= m_lowdelayDeltaFrmszP[j][i];
                hucConstData->FrmSzAdjTabB_S8[(numEstrateThreshlds +1)*j+i]= m_lowdelayDeltaFrmszB[j][i];
            }
        }
    }

    // ModeCosts depends on frame type
    if (m_pictureCodingType == I_TYPE)
    {
        MOS_SecureMemcpy(hucConstData->ModeCosts, sizeof(m_hucModeCostsIFrame), m_hucModeCostsIFrame, sizeof(m_hucModeCostsIFrame));
    }
    else
    {
        MOS_SecureMemcpy(hucConstData->ModeCosts, sizeof(m_hucModeCostsPbFrame), m_hucModeCostsPbFrame, sizeof(m_hucModeCostsPbFrame));
    }

    // starting location in batch buffer for each slice
    uint32_t baseLocation = m_hwInterface->m_vdencBatchBuffer1stGroupSize + m_hwInterface->m_vdencBatchBuffer2ndGroupSize;
    uint32_t currentLocation = baseLocation;

    auto slcData = m_slcData;
    // HCP_WEIGHTSOFFSETS_STATE + HCP_SLICE_STATE + HCP_PAK_INSERT_OBJECT + VDENC_WEIGHT_OFFSETS_STATE
    for (uint32_t slcCount = 0; slcCount < m_numSlices; slcCount++)
    {
        auto hevcSlcParams = &m_hevcSliceParams[slcCount];
        // HuC FW require unit in Bytes
        hucConstData->Slice[slcCount].SizeOfCMDs
            = (uint16_t)(m_hwInterface->m_vdencBatchBufferPerSliceConstSize + m_vdencBatchBufferPerSliceVarSize[slcCount]);

        // HCP_WEIGHTOFFSET_STATE cmd
        if (m_hevcVdencWeightedPredEnabled)
        {
            // 1st HCP_WEIGHTOFFSET_STATE cmd - P & B
            if (hevcSlcParams->slice_type == CODECHAL_ENCODE_HEVC_P_SLICE || hevcSlcParams->slice_type == CODECHAL_ENCODE_HEVC_B_SLICE)
            {
                hucConstData->Slice[slcCount].HcpWeightOffsetL0_StartInBytes = (uint16_t)currentLocation;   // HCP_WEIGHTOFFSET_L0 starts in byte from beginning of the SLB. 0xFFFF means unavailable in SLB
                currentLocation += m_hcpWeightOffsetStateCmdSize;
            }

            // 2nd HCP_WEIGHTOFFSET_STATE cmd - B
            if (hevcSlcParams->slice_type == CODECHAL_ENCODE_HEVC_B_SLICE)
            {
                hucConstData->Slice[slcCount].HcpWeightOffsetL1_StartInBytes = (uint16_t)currentLocation; // HCP_WEIGHTOFFSET_L1 starts in byte from beginning of the SLB. 0xFFFF means unavailable in SLB
                currentLocation += m_hcpWeightOffsetStateCmdSize;
            }
        }
        else
        {
            // 0xFFFF means unavailable in SLB
            hucConstData->Slice[slcCount].HcpWeightOffsetL0_StartInBytes = 0xFFFF;
            hucConstData->Slice[slcCount].HcpWeightOffsetL1_StartInBytes = 0xFFFF;
        }

        // HCP_SLICE_STATE cmd
        hucConstData->Slice[slcCount].SliceState_StartInBytes = (uint16_t)currentLocation;  // HCP_WEIGHTOFFSET is not needed
        currentLocation += m_hcpSliceStateCmdSize;

        // VDENC_WEIGHT_OFFSETS_STATE cmd
        hucConstData->Slice[slcCount].VdencWeightOffset_StartInBytes                      // VdencWeightOffset cmd is the last one expect BatchBufferEnd cmd
            = (uint16_t)(baseLocation + hucConstData->Slice[slcCount].SizeOfCMDs - m_vdencWeightOffsetStateCmdSize - m_miBatchBufferEndCmdSize);

        // logic from PakInsertObject cmd
        uint32_t bitSize         = (m_hevcSeqParams->SliceSizeControl) ? (hevcSlcParams->BitLengthSliceHeaderStartingPortion) : slcData[slcCount].BitSize;  // 40 for HEVC VDEnc Dynamic Slice
        uint32_t byteSize = (bitSize + 7) >> 3;
        uint32_t sliceHeaderSize = (MOS_ALIGN_CEIL(byteSize, sizeof(uint32_t))) / sizeof(uint32_t) * 4;
        // 1st PakInsertObject cmd with AU, SPS, PPS headers only exists for the first slice
        if (slcCount == 0)
        {
            // assumes that there is no 3rd PakInsertObject cmd for SSC
            currentLocation += m_1stPakInsertObjectCmdSize;
        }

        hucConstData->Slice[slcCount].SliceHeaderPIO_StartInBytes = (uint16_t)currentLocation;

        // HuC FW require unit in Bits from here (also number of bits from the beginning of slice header)
        hucConstData->Slice[slcCount].SliceHeader_SizeInBits = (uint16_t)(sliceHeaderSize * 8);   // 8: translate from byte to bit

        hucConstData->Slice[slcCount].HaveSliceSaoChromaFlag = hevcSlcParams->slice_sao_chroma_flag;         // 0: no slice_sao_chroma_flag in the slice header
        if (hevcSlcParams->slice_sao_chroma_flag || hevcSlcParams->slice_sao_luma_flag)
        {
            hucConstData->Slice[slcCount].SliceSaoLumaFlag_StartInBits = hevcSlcParams->SliceSAOFlagBitOffset;   // number of bits from beginning of slice header, 0xffff means not awailable
        }
        else
        {
            hucConstData->Slice[slcCount].SliceSaoLumaFlag_StartInBits = (uint16_t)0x7FFF;
        }

        if (m_hevcVdencWeightedPredEnabled)
        {
            hucConstData->Slice[slcCount].WeightTable_StartInBits = (uint16_t)hevcSlcParams->PredWeightTableBitOffset;
            hucConstData->Slice[slcCount].WeightTable_EndInBits = (uint16_t)(hevcSlcParams->PredWeightTableBitOffset + (hevcSlcParams->PredWeightTableBitLength));
        }
        else
        {
            // number of bits from beginning of slice header, 0xffff means not awailable
            hucConstData->Slice[slcCount].WeightTable_StartInBits = 0xFFFF;
            hucConstData->Slice[slcCount].WeightTable_EndInBits = 0xFFFF;
        }

        baseLocation += hucConstData->Slice[slcCount].SizeOfCMDs;
        currentLocation = baseLocation;
    }

    m_osInterface->pfnUnlockResource(m_osInterface, &m_vdencBrcConstDataBuffer[m_currRecycledBufIdx]);

    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG10::SetDmemHuCBrcUpdate()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_LOCK_PARAMS lockFlagsWriteOnly;
    MOS_ZeroMemory(&lockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
    lockFlagsWriteOnly.WriteOnly = true;

    // Program update DMEM
    PCODECHAL_VDENC_HEVC_HUC_BRC_UPDATE_DMEM_G10 hucVdencBrcUpdateDmem = (PCODECHAL_VDENC_HEVC_HUC_BRC_UPDATE_DMEM_G10)m_osInterface->pfnLockResource(
        m_osInterface, &m_vdencBrcUpdateDmemBuffer[m_currRecycledBufIdx][GetCurrentPass()], &lockFlagsWriteOnly);
    CODECHAL_ENCODE_CHK_NULL_RETURN(hucVdencBrcUpdateDmem);

    hucVdencBrcUpdateDmem->TARGETSIZE_U32 = MOS_MIN(m_hevcSeqParams->InitVBVBufferFullnessInBit, m_hevcSeqParams->VBVBufferSizeInBit);
    hucVdencBrcUpdateDmem->FrameID_U32 = m_storeData;    // frame number

    MOS_SecureMemcpy(hucVdencBrcUpdateDmem->startGAdjFrame_U16, 4 * sizeof(uint16_t), (void*)m_startGAdjFrame, 4 * sizeof(uint16_t));
    hucVdencBrcUpdateDmem->TargetSliceSize_U16 = (uint16_t)m_hevcPicParams->MaxSliceSizeInBytes;

    m_vdenc2ndLevelBatchBufferSize[m_currRecycledBufIdx] = ((m_hwInterface->m_vdenc2ndLevelBatchBufferSize - m_hwInterface->m_vdencBatchBuffer1stGroupSize
        - m_hwInterface->m_vdencBatchBuffer2ndGroupSize)/ ENCODE_HEVC_VDENC_NUM_MAX_SLICES)*m_numSlices +
        m_hwInterface->m_vdencBatchBuffer1stGroupSize + m_hwInterface->m_vdencBatchBuffer2ndGroupSize;
    hucVdencBrcUpdateDmem->SLB_Data_SizeInBytes = (uint16_t)m_vdenc2ndLevelBatchBufferSize[m_currRecycledBufIdx];
    hucVdencBrcUpdateDmem->PIPE_MODE_SELECT_StartInBytes = 0;
    hucVdencBrcUpdateDmem->CMD1_StartInBytes = (uint16_t)m_hwInterface->m_vdencBatchBuffer1stGroupSize;
    hucVdencBrcUpdateDmem->PIC_STATE_StartInBytes = (uint16_t)m_picStateCmdStartInBytes;
    hucVdencBrcUpdateDmem->CMD2_StartInBytes = (uint16_t)m_cmd2StartInBytes;

    uint16_t circularFrameIdx = (m_storeData - 1) % 4;

    // initial order before circular shift: current, ref0, ref1, ref2 = 0, 3, 2, 1
    // different initial order can be used, but this order (0, 3, 2, 1) is kernel recommendation
    hucVdencBrcUpdateDmem->Current_Data_Offset = ((0 + circularFrameIdx) % 4) * m_weightHistSize;
    hucVdencBrcUpdateDmem->Ref_Data_Offset[0] = ((3 + circularFrameIdx) % 4) * m_weightHistSize;
    hucVdencBrcUpdateDmem->Ref_Data_Offset[1] = ((2 + circularFrameIdx) % 4) * m_weightHistSize;
    hucVdencBrcUpdateDmem->Ref_Data_Offset[2] = ((1 + circularFrameIdx) % 4) * m_weightHistSize;

    // temporarily set to 0 for CNL Beta. TEDDI/MSDK based tests need to send the values 30 * level .
    // Also need to remove this  pCodecHalSeqParams->Level = pDDISeqParams->general_level_idc / 3;
    hucVdencBrcUpdateDmem->MaxNumSliceAllowed_U16 = 0; //(uint16_t)GetMaxAllowedSlices(m_hevcSeqParams->Level);

    hucVdencBrcUpdateDmem->OpMode_U8         // 1: BRC (including ACQP), 2: Weighted prediction (should not be enabled in first pass)
        = (m_hevcVdencWeightedPredEnabled && !IsFirstPass()) ? 3 : 1;    // 01: BRC, 10: WP never used,  11: BRC + WP

    // LowDelay B needs to be considered as P frame although wPictureCodingType=3
    // wPictureCodingType I:1, P:2, B:3 -> CurrentFrameType I:2, P:0, B:1
    hucVdencBrcUpdateDmem->CurrentFrameType_U8 = (m_pictureCodingType == I_TYPE) ? 2 : 0;

    // Num_Ref_L1 should be always same as Num_Ref_L0
    hucVdencBrcUpdateDmem->Num_Ref_L0_U8 = m_hevcSliceParams->num_ref_idx_l0_active_minus1 + 1;
    hucVdencBrcUpdateDmem->Num_Ref_L1_U8 = m_hevcSliceParams->num_ref_idx_l1_active_minus1 + 1;

    hucVdencBrcUpdateDmem->Num_Slices = (uint8_t)m_hevcPicParams->NumSlices;

    // CQP_QPValue_U8 setting is needed since ACQP is also part of ICQ
    hucVdencBrcUpdateDmem->CQP_QPValue_U8 = m_hevcPicParams->QpY + m_hevcSliceParams->slice_qp_delta;
    hucVdencBrcUpdateDmem->CQP_FracQP_U8 = 0;
    if (m_hevcPicParams->BRCPrecision == 1)
    {
        hucVdencBrcUpdateDmem->MaxNumPass_U8 = 1;
    }
    else
    {
        hucVdencBrcUpdateDmem->MaxNumPass_U8 = CODECHAL_VDENC_BRC_NUM_OF_PASSES;
    }

    MOS_SecureMemcpy(hucVdencBrcUpdateDmem->gRateRatioThreshold_U8, 7 * sizeof(uint8_t), (void*)m_rateRatioThreshold, 7 * sizeof(uint8_t));
    MOS_SecureMemcpy(hucVdencBrcUpdateDmem->startGAdjMult_U8, 5 * sizeof(uint8_t), (void*)m_startGAdjMult, 5 * sizeof(uint8_t));
    MOS_SecureMemcpy(hucVdencBrcUpdateDmem->startGAdjDiv_U8, 5 * sizeof(uint8_t), (void*)m_startGAdjDiv, 5 * sizeof(uint8_t));
    MOS_SecureMemcpy(hucVdencBrcUpdateDmem->gRateRatioThresholdQP_U8, 8 * sizeof(uint8_t), (void*)m_rateRatioThresholdQP, 8 * sizeof(uint8_t));

    if ((m_hevcVdencAcqpEnabled && m_hevcSeqParams->QpAdjustment) || (m_brcEnabled && (m_hevcSeqParams->MBBRC != 2)))
    {
        if (m_hevcPicParams->CodingType == I_TYPE)
        {
            hucVdencBrcUpdateDmem->DeltaQPForSadZone0_S8 = -5;
            hucVdencBrcUpdateDmem->DeltaQPForSadZone1_S8 = -3;
            hucVdencBrcUpdateDmem->DeltaQPForSadZone2_S8 = 1;
            hucVdencBrcUpdateDmem->DeltaQPForSadZone3_S8 = 2;
            hucVdencBrcUpdateDmem->DeltaQPForMvZero_S8 = 0;
            hucVdencBrcUpdateDmem->DeltaQPForMvZone0_S8 = 0;
            hucVdencBrcUpdateDmem->DeltaQPForMvZone1_S8 = 0;
            hucVdencBrcUpdateDmem->DeltaQPForMvZone2_S8 = 0;
        }
        else // LDB frames
        {
            hucVdencBrcUpdateDmem->DeltaQPForSadZone0_S8 = -3;
            hucVdencBrcUpdateDmem->DeltaQPForSadZone1_S8 = -2;
            hucVdencBrcUpdateDmem->DeltaQPForSadZone2_S8 = 1;
            hucVdencBrcUpdateDmem->DeltaQPForSadZone3_S8 = 2;
            hucVdencBrcUpdateDmem->DeltaQPForMvZero_S8 = -3;
            hucVdencBrcUpdateDmem->DeltaQPForMvZone0_S8 = -1;
            hucVdencBrcUpdateDmem->DeltaQPForMvZone1_S8 = 0;
            hucVdencBrcUpdateDmem->DeltaQPForMvZone2_S8 = 1;
        }
    }

    hucVdencBrcUpdateDmem->CurrentPass_U8 = (uint8_t) GetCurrentPass();

    if (m_hevcVdencWeightedPredEnabled)
    {
        hucVdencBrcUpdateDmem->LumaLog2WeightDenom_S8 = 6;
        hucVdencBrcUpdateDmem->ChromaLog2WeightDenom_S8 = 6;
    }

    // chroma weights are not confirmed to be supported from HW team yet
    hucVdencBrcUpdateDmem->DisabledFeature_U8 = 0; // bit mask, 1 (bit0): disable chroma weight setting

    hucVdencBrcUpdateDmem->SlidingWindow_Enable_U8 = 0;    // 0-disabled, 1-enabled
    hucVdencBrcUpdateDmem->LOG_LCU_Size_U8 = 6;
    hucVdencBrcUpdateDmem->RDOQ_Enable_U8          = (uint8_t)m_hevcRdoqEnabled;  // 0-disabled, 1-enabled

    hucVdencBrcUpdateDmem->ReEncodePositiveQPDeltaThr_S8 = 4;
    hucVdencBrcUpdateDmem->ReEncodeNegativeQPDeltaThr_S8 = -10;
    hucVdencBrcUpdateDmem->SliceHeaderSize = 0;

    // reset skip frame statistics
    m_numSkipFrames = 0;
    m_sizeSkipFrames = 0;

    m_osInterface->pfnUnlockResource(m_osInterface, &(m_vdencBrcUpdateDmemBuffer[m_currRecycledBufIdx][GetCurrentPass()]));

    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG10::ConstructBatchBufferHuCCQP(PMOS_RESOURCE batchBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;
    CODECHAL_ENCODE_CHK_NULL_RETURN(batchBuffer);

    MOS_LOCK_PARAMS lockFlags;
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.WriteOnly = true;

    uint8_t *data = (uint8_t *)m_osInterface->pfnLockResource(m_osInterface, batchBuffer, &lockFlags);
    CODECHAL_ENCODE_CHK_NULL_RETURN(data);

    MOS_COMMAND_BUFFER constructedCmdBuf;
    MOS_ZeroMemory(&constructedCmdBuf, sizeof(constructedCmdBuf));

    constructedCmdBuf.pCmdBase = constructedCmdBuf.pCmdPtr = (uint32_t *)data;
    constructedCmdBuf.iRemaining = MOS_ALIGN_CEIL(m_hwInterface->m_vdencReadBatchBufferSize, CODECHAL_PAGE_SIZE);

    constructedCmdBuf.pCmdPtr += (m_insertOffsetAfterCMD1 / 4);
    constructedCmdBuf.iOffset += m_insertOffsetAfterCMD1;

    m_picStateCmdStartInBytes = constructedCmdBuf.iOffset;

    // set HCP_PIC_STATE command
    MHW_VDBOX_HEVC_PIC_STATE hevcPicState;
    SetHcpPicStateParams(hevcPicState);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpPicStateCmd(&constructedCmdBuf, &hevcPicState));
    m_cmd2StartInBytes = constructedCmdBuf.iOffset;

    constructedCmdBuf.pCmdPtr += (m_insertOffsetAfterCMD2 / 4);
    constructedCmdBuf.iOffset += m_insertOffsetAfterCMD2;

    // set MI_BATCH_BUFFER_END command
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(&constructedCmdBuf, nullptr));

    if (data)
    {
        m_osInterface->pfnUnlockResource(m_osInterface, batchBuffer);
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucCmdInitializer->CmdInitializerExecute(false, batchBuffer));

    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG10::ConstructBatchBufferHuCBRC(PMOS_RESOURCE batchBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_slcData);
    CODECHAL_ENCODE_CHK_NULL_RETURN(batchBuffer);

    MOS_LOCK_PARAMS lockFlags;
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.WriteOnly = true;

    uint8_t *data = (uint8_t *)m_osInterface->pfnLockResource(m_osInterface, batchBuffer, &lockFlags);
    CODECHAL_ENCODE_CHK_NULL_RETURN(data);

    MOS_COMMAND_BUFFER constructedCmdBuf;
    MOS_ZeroMemory(&constructedCmdBuf, sizeof(constructedCmdBuf));
    constructedCmdBuf.pCmdBase = constructedCmdBuf.pCmdPtr = (uint32_t *)data;
    constructedCmdBuf.iRemaining = MOS_ALIGN_CEIL(m_hwInterface->m_vdencReadBatchBufferSize, CODECHAL_PAGE_SIZE);

    // 1st Group : HCP_PIPE_MODE_SELECT
    // set HCP_PIPE_MODE_SELECT command
    MHW_VDBOX_PIPE_MODE_SELECT_PARAMS pipeModeSelectParams;
    pipeModeSelectParams.Mode = m_mode;
    pipeModeSelectParams.bVdencEnabled = true;
    pipeModeSelectParams.bStreamOutEnabled = true;
    pipeModeSelectParams.bAdvancedRateControlEnable = true;
    pipeModeSelectParams.bRdoqEnable                = m_hevcRdoqEnabled;
    if (m_hevcSeqParams->SAO_enabled_flag)
    {
        // GEN10 uses pipe mode select command to tell if this is the first or second pass of SAO
        pipeModeSelectParams.bSaoFirstPass = !IsLastPass();
    }
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpPipeModeSelectCmd(&constructedCmdBuf, &pipeModeSelectParams));
    // set MI_BATCH_BUFFER_END command
    int32_t cmdBufOffset = constructedCmdBuf.iOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(&constructedCmdBuf, nullptr));
    m_miBatchBufferEndCmdSize = constructedCmdBuf.iOffset - cmdBufOffset;

    CODECHAL_ENCODE_ASSERT(m_hwInterface->m_vdencBatchBuffer1stGroupSize == constructedCmdBuf.iOffset);

    constructedCmdBuf.pCmdPtr += (m_insertOffsetAfterCMD1 / 4);
    constructedCmdBuf.iOffset += m_insertOffsetAfterCMD1;

    m_picStateCmdStartInBytes = constructedCmdBuf.iOffset;

    // set HCP_PIC_STATE command
    MHW_VDBOX_HEVC_PIC_STATE hevcPicState;
    SetHcpPicStateParams(hevcPicState);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpPicStateCmd(&constructedCmdBuf, &hevcPicState));
    m_cmd2StartInBytes = constructedCmdBuf.iOffset;

    constructedCmdBuf.pCmdPtr += (m_insertOffsetAfterCMD2 / 4);
    constructedCmdBuf.iOffset += m_insertOffsetAfterCMD2;

    // set MI_BATCH_BUFFER_END command
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(&constructedCmdBuf, nullptr));

    CODECHAL_ENCODE_ASSERT(m_hwInterface->m_vdencBatchBuffer2ndGroupSize == constructedCmdBuf.iOffset - m_hwInterface->m_vdencBatchBuffer1stGroupSize);

    // 3rd Group : HCP_WEIGHTSOFFSETS_STATE + HCP_SLICE_STATE + HCP_PAK_INSERT_OBJECT + VDENC_WEIGHT_OFFSETS_STATE
    MHW_VDBOX_HEVC_SLICE_STATE sliceState;
    sliceState.presDataBuffer = &m_resMbCodeSurface;
    sliceState.pHevcPicIdx           = &m_picIdx[0];
    sliceState.pEncodeHevcSeqParams  = m_hevcSeqParams;
    sliceState.pEncodeHevcPicParams  = m_hevcPicParams;
    sliceState.pBsBuffer = &m_bsBuffer;
    sliceState.ppNalUnitParams       = m_nalUnitParams;
    sliceState.bBrcEnabled           = m_brcEnabled;
    sliceState.dwHeaderBytesInserted = 0;
    sliceState.dwHeaderDummyBytes = 0;
    sliceState.pRefIdxMapping        = m_refIdxMapping;
    sliceState.bIsLowDelay           = m_lowDelay;
    sliceState.bVdencInUse = true;
    // This bit disables top intra Reference pixel fetch in VDENC mode.
    // In PAK only second pass, this bit should be set to one.
    // "IntraRefFetchDisable" in HCP SLICE STATE should be set to 0 in first pass and 1 in subsequent passes.
    // For dynamic slice, 2nd pass is still VDEnc + PAK pass, not PAK only pass.
    sliceState.bIntraRefFetchDisable = m_pakOnlyPass;

    // slice level cmds for each slice
    PCODEC_ENCODER_SLCDATA slcData = m_slcData;
    for (uint32_t startLCU = 0, slcCount = 0; slcCount < m_numSlices; slcCount++)
    {
        if (IsFirstPass())
        {
            slcData[slcCount].CmdOffset = startLCU * (m_hcpInterface->GetHcpPakObjSize()) * sizeof(uint32_t);
        }
        sliceState.pEncodeHevcSliceParams    = &m_hevcSliceParams[slcCount];
        sliceState.dwDataBufferOffset = slcData[slcCount].CmdOffset;
        sliceState.dwOffset = slcData[slcCount].SliceOffset;
        sliceState.dwLength = slcData[slcCount].BitSize;
        sliceState.uiSkipEmulationCheckCount = slcData[slcCount].SkipEmulationByteCount;
        sliceState.dwSliceIndex = (uint32_t)slcCount;
        sliceState.bLastSlice = (slcCount == m_numSlices - 1);
        sliceState.bFirstPass = IsFirstPass();
        sliceState.bLastPass = IsLastPass();
        sliceState.bInsertBeforeSliceHeaders = (slcCount == 0);
        sliceState.bSaoLumaFlag              = (m_hevcSeqParams->SAO_enabled_flag) ? m_hevcSliceParams[slcCount].slice_sao_luma_flag : 0;
        sliceState.bSaoChromaFlag            = (m_hevcSeqParams->SAO_enabled_flag) ? m_hevcSliceParams[slcCount].slice_sao_chroma_flag : 0;

        if (m_hevcPicParams->transform_skip_enabled_flag)
        {
            CalcTransformSkipParameters(sliceState.EncodeHevcTransformSkipParams);
        }

        m_vdencBatchBufferPerSliceVarSize[slcCount] = 0;

        // set HCP_WEIGHTOFFSET_STATE command
        // This slice level command is issued, if the weighted_pred_flag or weighted_bipred_flag equals one.
        // If zero, then this command is not issued.
        if (m_hevcVdencWeightedPredEnabled)
        {
            MHW_VDBOX_HEVC_WEIGHTOFFSET_PARAMS hcpWeightOffsetParams;
            MOS_ZeroMemory(&hcpWeightOffsetParams, sizeof(hcpWeightOffsetParams));

            for (auto k = 0; k < 2; k++) // k=0: LIST_0, k=1: LIST_1
            {
                // Luma, Chroma offset
                for (auto i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
                {
                    hcpWeightOffsetParams.LumaOffsets[k][i] = (int16_t)m_hevcSliceParams->luma_offset[k][i];
                    // Cb, Cr
                    for (auto j = 0; j < 2; j++)
                    {
                        hcpWeightOffsetParams.ChromaOffsets[k][i][j] = (int16_t)m_hevcSliceParams->chroma_offset[k][i][j];
                    }
                }

                // Luma Weight
                CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
                    &hcpWeightOffsetParams.LumaWeights[k],
                    sizeof(hcpWeightOffsetParams.LumaWeights[k]),
                    &m_hevcSliceParams->delta_luma_weight[k],
                    sizeof(m_hevcSliceParams->delta_luma_weight[k])));
                // Chroma Weight
                CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
                    &hcpWeightOffsetParams.ChromaWeights[k],
                    sizeof(hcpWeightOffsetParams.ChromaWeights[k]),
                    &m_hevcSliceParams->delta_chroma_weight[k],
                    sizeof(m_hevcSliceParams->delta_chroma_weight[k])));
            }

            // 1st HCP_WEIGHTOFFSET_STATE cmd - P & B
            if (m_hevcSliceParams->slice_type == CODECHAL_ENCODE_HEVC_P_SLICE || m_hevcSliceParams->slice_type == CODECHAL_ENCODE_HEVC_B_SLICE)
            {
                hcpWeightOffsetParams.ucList = LIST_0;

                cmdBufOffset = constructedCmdBuf.iOffset;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpWeightOffsetStateCmd(&constructedCmdBuf, nullptr, &hcpWeightOffsetParams));
                m_hcpWeightOffsetStateCmdSize = constructedCmdBuf.iOffset - cmdBufOffset;
                // 1st HcpWeightOffset cmd is not always inserted (except weighted prediction + P, B slices)
                m_vdencBatchBufferPerSliceVarSize[slcCount] += m_hcpWeightOffsetStateCmdSize;
            }

            // 2nd HCP_WEIGHTOFFSET_STATE cmd - B only
            if (m_hevcSliceParams->slice_type == CODECHAL_ENCODE_HEVC_B_SLICE)
            {
                hcpWeightOffsetParams.ucList = LIST_1;

                cmdBufOffset = constructedCmdBuf.iOffset;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpWeightOffsetStateCmd(&constructedCmdBuf, nullptr, &hcpWeightOffsetParams));
                m_hcpWeightOffsetStateCmdSize = constructedCmdBuf.iOffset - cmdBufOffset;
                // 2nd HcpWeightOffset cmd is not always inserted (except weighted prediction + B slices)
                m_vdencBatchBufferPerSliceVarSize[slcCount] += m_hcpWeightOffsetStateCmdSize;
            }
        }

        // set HCP_SLICE_STATE command
        cmdBufOffset = constructedCmdBuf.iOffset;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpSliceStateCmd(&constructedCmdBuf, &sliceState));
        m_hcpSliceStateCmdSize = constructedCmdBuf.iOffset - cmdBufOffset;

        // set 1st HCP_PAK_INSERT_OBJECT command
        // insert AU, SPS, PPS headers before first slice header
        if (sliceState.bInsertBeforeSliceHeaders)
        {
            uint32_t maxBytesInPakInsertObjCmd = ((2 << 11) - 1) * 4; // 12 bits for DwordLength field in PAK_INSERT_OBJ cmd
            m_1stPakInsertObjectCmdSize = 0;

            for (auto i = 0; i < HEVC_MAX_NAL_UNIT_TYPE; i++)
            {
                uint32_t nalUnitPosiSize = sliceState.ppNalUnitParams[i]->uiSize;
                uint32_t nalUnitPosiOffset = sliceState.ppNalUnitParams[i]->uiOffset;

                while (nalUnitPosiSize > 0)
                {
                    bool insert = sliceState.ppNalUnitParams[i]->bInsertEmulationBytes ? true : false;
                    uint32_t bitSize = MOS_MIN(maxBytesInPakInsertObjCmd * 8, nalUnitPosiSize * 8);
                    uint32_t offSet = nalUnitPosiOffset;

                    MHW_VDBOX_PAK_INSERT_PARAMS pakInsertObjectParams;
                    MOS_ZeroMemory(&pakInsertObjectParams, sizeof(pakInsertObjectParams));
                    pakInsertObjectParams.bEmulationByteBitsInsert = insert;
                    pakInsertObjectParams.uiSkipEmulationCheckCount = sliceState.ppNalUnitParams[i]->uiSkipEmulationCheckCount;
                    pakInsertObjectParams.pBsBuffer = sliceState.pBsBuffer;
                    pakInsertObjectParams.dwBitSize = bitSize;
                    pakInsertObjectParams.dwOffset = offSet;

                    if (nalUnitPosiSize > maxBytesInPakInsertObjCmd)
                    {
                        nalUnitPosiSize -= maxBytesInPakInsertObjCmd;
                        nalUnitPosiOffset += maxBytesInPakInsertObjCmd;
                    }
                    else
                    {
                        nalUnitPosiSize = 0;
                    }

                    cmdBufOffset = constructedCmdBuf.iOffset;
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpPakInsertObject(&constructedCmdBuf, &pakInsertObjectParams));

                    // this info needed again in BrcUpdate HuC FW const
                    m_1stPakInsertObjectCmdSize += (constructedCmdBuf.iOffset - cmdBufOffset);
                }
            }
            // 1st PakInsertObject cmd is not always inserted for each slice
            m_vdencBatchBufferPerSliceVarSize[slcCount] += m_1stPakInsertObjectCmdSize;
        }

        // set 2nd HCP_PAK_INSERT_OBJECT command
        // Insert slice header
        MHW_VDBOX_PAK_INSERT_PARAMS pakInsertObjectParams;
        MOS_ZeroMemory(&pakInsertObjectParams, sizeof(pakInsertObjectParams));
        pakInsertObjectParams.bLastHeader = true;
        pakInsertObjectParams.bEmulationByteBitsInsert = true;

        // App does the slice header packing, set the skip count passed by the app
        pakInsertObjectParams.uiSkipEmulationCheckCount = sliceState.uiSkipEmulationCheckCount;
        pakInsertObjectParams.pBsBuffer = sliceState.pBsBuffer;
        pakInsertObjectParams.dwBitSize = sliceState.dwLength;
        pakInsertObjectParams.dwOffset = sliceState.dwOffset;

        // For HEVC VDEnc Dynamic Slice
        if (m_hevcSeqParams->SliceSizeControl)
        {
            pakInsertObjectParams.bLastHeader = false;
            pakInsertObjectParams.bEmulationByteBitsInsert = false;
            pakInsertObjectParams.dwBitSize                  = m_hevcSliceParams->BitLengthSliceHeaderStartingPortion;
            pakInsertObjectParams.bResetBitstreamStartingPos = true;
        }

        uint32_t byteSize = (pakInsertObjectParams.dwBitSize + 7) >> 3;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpPakInsertObject(
            &constructedCmdBuf,
            &pakInsertObjectParams));

        // 2nd PakInsertObject cmd is always inserted for each slice
        // so already reflected in dwVdencBatchBufferPerSliceConstSize
        m_vdencBatchBufferPerSliceVarSize[slcCount] += (MOS_ALIGN_CEIL(byteSize, sizeof(uint32_t))) / sizeof(uint32_t) * 4;

        // set 3rd HCP_PAK_INSERT_OBJECT command
        if (m_hevcSeqParams->SliceSizeControl)
        {
            // Send HCP_PAK_INSERT_OBJ command. For dynamic slice, we are skipping the beginning part of slice header.
            pakInsertObjectParams.bLastHeader = true;
            pakInsertObjectParams.dwBitSize   = sliceState.dwLength - m_hevcSliceParams->BitLengthSliceHeaderStartingPortion;
            pakInsertObjectParams.dwOffset += ((m_hevcSliceParams->BitLengthSliceHeaderStartingPortion + 7) / 8);  // Skips the first 5 bytes which is Start Code + Nal Unit Header
            pakInsertObjectParams.bResetBitstreamStartingPos = true;

            cmdBufOffset = constructedCmdBuf.iOffset;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpPakInsertObject(
                &constructedCmdBuf,
                &pakInsertObjectParams));
            // 3rd PakInsertObject cmd is not always inserted for each slice
            m_vdencBatchBufferPerSliceVarSize[slcCount] += (constructedCmdBuf.iOffset - cmdBufOffset);
        }

        // set VDENC_WEIGHT_OFFSETS_STATE command
        MHW_VDBOX_VDENC_WEIGHT_OFFSET_PARAMS vdencWeightOffsetParams;
        MOS_ZeroMemory(&vdencWeightOffsetParams, sizeof(vdencWeightOffsetParams));
        vdencWeightOffsetParams.bWeightedPredEnabled = m_hevcVdencWeightedPredEnabled;

        if (vdencWeightOffsetParams.bWeightedPredEnabled)
        {
            vdencWeightOffsetParams.dwDenom = 1 << (m_hevcSliceParams->luma_log2_weight_denom);

            // Luma Offsets
            for (auto i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
            {
                vdencWeightOffsetParams.LumaOffsets[0][i] = (int16_t)m_hevcSliceParams->luma_offset[0][i];
                vdencWeightOffsetParams.LumaOffsets[1][i] = (int16_t)m_hevcSliceParams->luma_offset[1][i];
            }

            // Luma Weights
            CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(MOS_SecureMemcpy(
                                                          &vdencWeightOffsetParams.LumaWeights[0],
                                                          sizeof(vdencWeightOffsetParams.LumaWeights[0]),
                                                          &m_hevcSliceParams->delta_luma_weight[0],
                                                          sizeof(m_hevcSliceParams->delta_luma_weight[0])),
                "Failed to copy luma weight 0 memory.");

            CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(MOS_SecureMemcpy(
                                                          &vdencWeightOffsetParams.LumaWeights[1],
                                                          sizeof(vdencWeightOffsetParams.LumaWeights[1]),
                                                          &m_hevcSliceParams->delta_luma_weight[1],
                                                          sizeof(m_hevcSliceParams->delta_luma_weight[1])),
                "Failed to copy luma weight 1 memory.");
        }

        cmdBufOffset = constructedCmdBuf.iOffset;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdencWeightsOffsetsStateCmd(
            &constructedCmdBuf,
            nullptr,
            &vdencWeightOffsetParams));
        m_vdencWeightOffsetStateCmdSize = constructedCmdBuf.iOffset - cmdBufOffset;

        // set MI_BATCH_BUFFER_END command
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(&constructedCmdBuf, nullptr));

        startLCU += m_hevcSliceParams[slcCount].NumLCUsInSlice;
    }

    if (data)
    {
        m_osInterface->pfnUnlockResource(m_osInterface, batchBuffer);
    }

    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG10::GetStatusReport(
    EncodeStatus *encodeStatus,
    EncodeStatusReport *encodeStatusReport)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    // common initilization
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalVdencHevcState::GetStatusReport(encodeStatus, encodeStatusReport));

    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG10::Initialize(CodechalSetting * settings)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    auto cmdInitializer = MOS_New(CodechalCmdInitializer, this);
    m_hucCmdInitializer = cmdInitializer;

    // common initilization
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalVdencHevcState::Initialize(settings));

    // Overriding the defaults here with 32 aligned dimensions
    // HME Scaling WxH
    m_downscaledWidthInMb4x =
        CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_frameWidth / SCALE_FACTOR_4x);
    m_downscaledHeightInMb4x =
        CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameHeight / SCALE_FACTOR_4x);
    m_downscaledWidth4x =
        m_downscaledWidthInMb4x * CODECHAL_MACROBLOCK_WIDTH;
    m_downscaledHeight4x =
        m_downscaledHeightInMb4x * CODECHAL_MACROBLOCK_HEIGHT;

    // SuperHME Scaling WxH
    m_downscaledWidthInMb16x =
        CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_frameWidth / SCALE_FACTOR_16x);
    m_downscaledHeightInMb16x =
        CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameHeight / SCALE_FACTOR_16x);
    m_downscaledWidth16x =
        m_downscaledWidthInMb16x * CODECHAL_MACROBLOCK_WIDTH;
    m_downscaledHeight16x =
        m_downscaledHeightInMb16x * CODECHAL_MACROBLOCK_HEIGHT;

    {
        m_minScaledDimension = m_minScaledSurfaceSize;
        m_minScaledDimensionInMb = (m_minScaledSurfaceSize + 15) >> 4;
    }

    return eStatus;
}

bool CodechalVdencHevcStateG10::CheckSupportedFormat(PMOS_SURFACE surface)
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
    case Format_NV21:
    case Format_P010:
    case Format_A8B8G8R8:
    case Format_R10G10B10A2:// Packed RGB 4:4:4
    case Format_B10G10R10A2:// Packed RGB 4:4:4
        isColorFormatSupported = true;
        break;
    case Format_YUY2:
    case Format_YUYV:
    case Format_YVYU:
    case Format_UYVY:
    case Format_VYUY:
    case Format_AYUV:
        isColorFormatSupported = (MOS_TILE_LINEAR == surface->TileType);
        break;
    case Format_A8R8G8B8:
        // On CNL RGB conversion is not studio range, intentionally let RGB fall-thru so driver calls Csc+Ds+Conversion kernel
    default:
        break;
    }

    return isColorFormatSupported;
}

CodechalVdencHevcStateG10::CodechalVdencHevcStateG10(
    CodechalHwInterface* hwInterface,
    CodechalDebugInterface* debugInterface,
    PCODECHAL_STANDARD_INFO standardInfo)
    :CodechalVdencHevcState(hwInterface, debugInterface, standardInfo)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    pfnGetKernelHeaderAndSize = GetKernelHeaderAndSize;

    m_b2NdSaoPassNeeded            = true;
    m_vdencBrcInitDmemBufferSize   = sizeof(CODECHAL_VDENC_HEVC_HUC_BRC_INIT_DMEM_G10);
    m_vdencBrcUpdateDmemBufferSize = sizeof(CODECHAL_VDENC_HEVC_HUC_BRC_UPDATE_DMEM_G10);
    m_vdencBrcConstDataBufferSize  = sizeof(CODECHAL_VDENC_HEVC_HUC_BRC_CONSTANT_DATA_G10);
    m_brcRoiBufferSize             = m_roiStreamInBufferSize;
    m_deltaQpRoiBufferSize         = m_deltaQpBufferSize;
    m_maxNumSlicesSupported        = CODECHAL_VDENC_HEVC_MAX_SLICE_NUM;
#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
    m_kernelBase                   = (uint8_t*)IGCODECKRN_G10;
#endif

    m_hwInterface->GetStateHeapSettings()->dwNumSyncTags = CODECHAL_ENCODE_HEVC_NUM_SYNC_TAGS;
    m_hwInterface->GetStateHeapSettings()->dwDshSize = CODECHAL_INIT_DSH_SIZE_HEVC_ENC;

    m_kuid = IDR_CODEC_VDENC_HME;
    MOS_STATUS eStatus = CodecHalGetKernelBinaryAndSize(
        m_kernelBase,
        m_kuid,
        &m_kernelBinary,
        &m_combinedKernelSize);
    CODECHAL_ENCODE_ASSERT(eStatus == MOS_STATUS_SUCCESS);

    m_hwInterface->GetStateHeapSettings()->dwIshSize +=
        MOS_ALIGN_CEIL(m_combinedKernelSize, (1 << MHW_KERNEL_OFFSET_SHIFT));

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_NULL_NO_STATUS_RETURN(m_encodeParState = MOS_New(CodechalDebugEncodeParG10, this));
    )
}
