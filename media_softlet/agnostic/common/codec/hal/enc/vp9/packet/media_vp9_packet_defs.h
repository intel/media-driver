/*
* Copyright (c) 2019-2021, Intel Corporation
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
//! \file     media_vp9_packet_defs.h
//! \brief    Defines for vp9 packets
//!

#ifndef __MEDIA_VP9_PACKET_DEFS_H__
#define __MEDIA_VP9_PACKET_DEFS_H__
#include "media_feature_manager.h"

namespace encode
{
//!
//! \struct    HcpPakObject
//! \brief     HCP pak object
//!
struct HcpPakObject
{
    // DW0
    struct
    {
        uint32_t DwordLength : 16;   //[15:0]
        uint32_t SubOp       : 7;    //[22:16]
        uint32_t Opcode      : 6;    //[28:23]
        uint32_t Type        : 3;    //[31:29]
    } DW0;

    //DW1
    struct
    {
        uint32_t Split_flag_level2_level1part0 : 4;
        uint32_t Split_flag_level2_level1part1 : 4;
        uint32_t Split_flag_level2_level1part2 : 4;
        uint32_t Split_flag_level2_level1part3 : 4;
        uint32_t Split_flag_level1             : 4;
        uint32_t Split_flag_level0             : 1;
        uint32_t Reserved21_23                 : 3;
        uint32_t CU_count_minus1               : 6;
        uint32_t IsLastSBFrameflag             : 1;
        uint32_t IsLastSBTileflag              : 1;
    } DW1;

    //DW2
    struct
    {
        uint32_t Current_SB_X_Addr : 16;
        uint32_t Current_SB_Y_Addr : 16;
    } DW2;

    //DW3
    uint32_t Reserved_DW03 : 32;
};
C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(HcpPakObject)) == 4);

//!
//! \struct    HucFrameCtrl
//! \brief     HUC frame contol
//!
struct HucFrameCtrl
{
    uint32_t FrameType;             //0:INTRA, 1:INTER     // DW15
    uint32_t ShowFrame;             // DW16
    uint32_t ErrorResilientMode;    // DW17
    uint32_t IntraOnly;             // DW18
    uint32_t ContextReset;          // DW19
    uint32_t LastRefFrameBias;      // DW20
    uint32_t GoldenRefFrameBias;    // DW21
    uint32_t AltRefFrameBias;       // DW22
    uint32_t AllowHighPrecisionMv;  // DW23
    uint32_t McompFilterMode;       // DW24
    uint32_t TxMode;                // DW25
    uint32_t RefreshFrameContext;   // DW26
    uint32_t FrameParallelDecode;   // DW27
    uint32_t CompPredMode;          // DW28
    uint32_t FrameContextIdx;       // DW29
    uint32_t SharpnessLevel;        // DW30
    uint32_t SegOn;                 // DW31
    uint32_t SegMapUpdate;          // DW32
    uint32_t SegUpdateData;         // DW33
    uint8_t  Rsvd[13];              // DW34-36, first byte of 37
    uint8_t  log2TileCols;          // DW37
    uint8_t  log2TileRows;          // DW37
    uint8_t  Reserved[5];           // DW37 last byte, DW38
};

//!
//! \struct    HucPrevFrameInfo
//! \brief     HUC definition: PrevFrameInfo
//!
struct HucPrevFrameInfo
{
    uint32_t IntraOnly;    // DW39
    uint32_t FrameWidth;   // DW40
    uint32_t FrameHeight;  // DW41
    uint32_t KeyFrame;     // DW42
    uint32_t ShowFrame;    // DW43
};

//!
//! \struct    HucProbDmem
//! \brief     HUC prob dmem
//!
struct HucProbDmem
{
    uint32_t         HuCPassNum;
    uint32_t         FrameWidth;
    uint32_t         FrameHeight;
    uint32_t         Rsvd32[6];
    char             SegmentRef[CODEC_VP9_MAX_SEGMENTS];
    uint8_t          SegmentSkip[CODEC_VP9_MAX_SEGMENTS];
    uint8_t          SegCodeAbs;
    uint8_t          SegTemporalUpdate;
    uint8_t          LastRefIndex;
    uint8_t          GoldenRefIndex;
    uint8_t          AltRefIndex;
    uint8_t          RefreshFrameFlags;
    uint8_t          RefFrameFlags;
    uint8_t          ContextFrameTypes;
    HucFrameCtrl     FrameCtrl;
    HucPrevFrameInfo PrevFrameInfo;
    uint8_t          Rsvd[2];
    uint8_t          FrameToShow;
    uint8_t          LoadKeyFrameDefaultProbs;
    uint32_t         FrameSize;
    uint32_t         VDEncImgStateOffset;
    uint32_t         RePak;
    uint16_t         LFLevelBitOffset;
    uint16_t         QIndexBitOffset;
    uint16_t         SegBitOffset;
    uint16_t         SegLengthInBits;
    uint16_t         UnCompHdrTotalLengthInBits;
    uint16_t         SegUpdateDisable;
    int32_t          RePakThreshold[256];
    uint16_t         PicStateOffset;
    uint16_t         SLBBSize;
    uint8_t          StreamInEnable;
    uint8_t          StreamInSegEnable;
    uint8_t          DisableDMA;
    uint8_t          IVFHeaderSize;
    uint8_t          PakOnlyEnable;
    uint8_t          Reserved[43];
};

//!
//! \struct    HucBrcInitDmem
//! \brief     HUC brc init dmem
//!
struct HucBrcInitDmem
{
    uint32_t BRCFunc;               // 0: Init; 2: Reset
    uint32_t ProfileLevelMaxFrame;  // Limit on maximum frame size based on selected profile and level, and can be user defined
    uint32_t InitBufFullness;       // Initial buffer fullness
    uint32_t BufSize;               // Buffer size
    uint32_t TargetBitrate;         // Average(target) bit rate
    uint32_t MaxRate;               // Maximum bit rate in bits per second (bps).
    uint32_t MinRate;               // Minimum bit rate
    uint32_t FrameRateM;            // Framerate numerator
    uint32_t FrameRateD;            // Framerate denominator
    uint32_t RSVD32[4];             // Reserved, MBZ

    uint16_t BRCFlag;               // BRC flag
    uint16_t GopP;                  // number of P frames in a GOP
    uint16_t Reserved;
    uint16_t FrameWidth;            // Frame width
    uint16_t FrameHeight;           // Frame height
    uint16_t MinQP;                 // Minimum QP
    uint16_t MaxQP;                 // Maximum QP
    uint16_t LevelQP;               // Level QP
    uint16_t GoldenFrameInterval;   // Golden frame interval
    uint16_t EnableScaling;         // Enable resolution scaling
    uint16_t OvershootCBR;          // default: 115, CBR overshoot percentage
    uint16_t RSVD16[5];             // Reserved, MBZ

    int8_t InstRateThreshP0[4];     // Instant rate threshold for P frame
    int8_t Reserved2[4];
    int8_t InstRateThreshI0[4];
    int8_t DevThreshPB0[8];         // Deviation threshold for P and B frame
    int8_t DevThreshVBR0[8];        // Deviation threshold for VBR control
    int8_t DevThreshI0[8];          // Deviation threshold for I frame

    uint8_t InitQPP;
    uint8_t InitQPI;
    uint8_t RSVD3;
    uint8_t Total_Level;
    uint8_t MaxLevel_Ratio[16];
    uint8_t SlidingWindowEnable;
    uint8_t SlidingWindowSize;
    uint8_t RSVD8[47];              // Reserved, MBZ
};

//!
//! \struct    HucBrcUpdateDmem
//! \brief     HUC brc update dmem
//!
struct HucBrcUpdateDmem
{
    int32_t  UPD_TARGET_BUF_FULLNESS_U32;      // Passed by the driver
    uint32_t UPD_FRAMENUM_U32;                 // Passed by the driver
    int32_t  UPD_HRD_BUFF_FULLNESS_UPPER_I32;  // Passed by the driver
    int32_t  UPD_HRD_BUFF_FULLNESS_LOWER_I32;  // Passed by the driver
    uint32_t RSVD32[7];                        // mbz

    uint16_t UPD_startGAdjFrame_U16[4];        // Start global adjust frame (4 items)
    uint16_t UPD_CurWidth_U16;                 // Current width
    uint16_t UPD_CurHeight_U16;                // Current height
    uint16_t UPD_Asyn_U16;
    uint16_t UPD_VDEncImgStateOffset;          // The image state start position in bytes from the begining of Second Level BB
    uint16_t UPD_SLBBSize;                     // Second level batch buffer total size in bytes
    uint16_t UPD_PicStateOffset;               // The pic state offset in bytes from the beginning of second level batch buffer
    uint16_t RSVD16[6];                        // mbz

    uint8_t UPD_OVERFLOW_FLAG_U8;              // Passed by the driver
    uint8_t UPD_BRCFlag_U8;                    // BRC flag, 0 - nothing to report, others - BRCPIC\BRCCUR flag defines 1 - scene change, etc // RSVD on G10, remove when G11 drops dependency
    uint8_t UPD_MaxNumPAKs_U8;                 // Maximum number of PAKs (default set to 4)
    int8_t  UPD_CurrFrameType_U8;              // Current frame type (0:P, 1:B, 2:I)
    uint8_t UPD_QPThreshold_U8[4];             // QP threshold (4 entries)
    uint8_t UPD_gRateRatioThreshold_U8[6];     // Global rate ratio threshold (6 items)
    int8_t  UPD_startGAdjMult_U8[5];           // Start global adjust mult (5 items)
    int8_t  UPD_startGAdjDiv_U8[5];            // Start global adjust div (5 items)
    int8_t  UPD_gRateRatioThresholdQP_U8[7];   // Global rate ratio threshold QP (7 items)
    uint8_t UPD_DistThreshldI_U8[9];           // (N_DISTORION_THRESHLDS+1) distortion thresholds for I frames
    uint8_t UPD_DistThreshldP_U8[9];           // (N_DISTORION_THRESHLDS+1) distortion thresholds for P frames
    uint8_t UPD_DistThreshldB_U8[9];           // (N_DISTORION_THRESHLDS+1) distortion thresholds for B frames; no needed for Vp8 - to clean up
    int8_t  UPD_MaxFrameThreshI_U8[5];         // Num qp threshld + 1 of multiplyers
    int8_t  UPD_MaxFrameThreshP_U8[5];         // Num qp threshld + 1 of multiplyers
    int8_t  UPD_MaxFrameThreshB_U8[5];         // Num qp threshld + 1 of multiplyers; no needed for Vp8 - to clean up
    uint8_t UPD_PAKPassNum_U8;                 // Current pak pass number
    uint8_t UPD_ACQQp_U8;
    int8_t  UPD_DeltaQPForSadZone0_I8;
    int8_t  UPD_DeltaQPForSadZone1_I8;
    int8_t  UPD_DeltaQPForSadZone2_I8;
    int8_t  UPD_DeltaQPForSadZone3_I8;
    int8_t  UPD_DeltaQPForMvZero_I8;
    int8_t  UPD_DeltaQPForMvZone0_I8;
    int8_t  UPD_DeltaQPForMvZone1_I8;
    int8_t  UPD_DeltaQPForMvZone2_I8;
    uint8_t UPD_Temporal_Level_U8;
    uint8_t UPD_SegMapGenerating_U8;           // Default 0: HuC does not update segmentation state; 1: HuC updates all 8 segmentation states in second level batch buffer
    uint8_t RSVD8[95];                         // mbz
};

}

#endif  // !__MEDIA_VP9_PACKET_DEFS_H__
