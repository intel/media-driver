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

/*
*  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
*
*  Use of this source code is governed by a BSD-style license
*  that can be found in the LICENSE file in the root of the source
*  tree. An additional intellectual property rights grant can be found
*  in the file PATENTS.  All contributing project authors may
*  be found in the AUTHORS file in the root of the source tree.
*/

//!
//! \file     codechal_vdenc_vp9_base.h
//! \brief    Defines base class for VP9 VDENC encoder.
//!

#ifndef __CODECHAL_VDENC_VP9_BASE_H__
#define __CODECHAL_VDENC_VP9_BASE_H__

#include "codechal_encoder_base.h"
#include "codechal_huc_cmd_initializer.h"
#include "codec_def_vp9_probs.h"
#include "codechal_debug.h"

#define CODECHAL_ENCODE_VP9_MAX_NUM_HCP_PIPE                    4
#define CODECHAL_VP9_ENCODE_RECYCLED_BUFFER_NUM                 (CODECHAL_ENCODE_RECYCLED_BUFFER_NUM * CODECHAL_ENCODE_VP9_MAX_NUM_HCP_PIPE) // for salability, need 1 buffer per pipe,
#define CODECHAL_ENCODE_VP9_NUM_SYNC_TAGS                       36
#define CODECHAL_ENCODE_VP9_INIT_DSH_SIZE                       (MHW_PAGE_SIZE * 3)
#define CODECHAL_ENCODE_VP9_SUPERFRAME_REPEATED_HEADER_SIZE     1
#define CODECHAL_ENCODE_VP9_SUPERFRAME_MARKER_HEADER_SIZE       1
#define CODECHAL_ENCODE_VP9_HUC_BRC_DATA_BUFFER_SIZE            (16*4)
#define CODECHAL_ENCODE_VP9_PAK_INSERT_UNCOMPRESSED_HEADER      80
#define CODECHAL_ENCODE_VP9_BRC_SUPER_FRAME_BUFFER_SIZE         MOS_ALIGN_CEIL(3 + 2 * sizeof(uint32_t), sizeof(uint32_t))
#define CODECHAL_ENCODE_VP9_VDENC_DATA_EXTENSION_SIZE           32
#define CODECHAL_ENCODE_VP9_PIC_STATE_BUFFER_SIZE_PER_PASS      192 // 42 DWORDs for Pic State one uint32_t for BB End + 5 uint32_tS reserved to make it aligned for kernel read
#define CODECHAL_ENCODE_VP9_MIN_TILE_SIZE_WIDTH                 256
#define CODECHAL_ENCODE_VP9_MIN_TILE_SIZE_HEIGHT                128
#define CODECHAL_ENCODE_VP9_HUC_SUPERFRAME_PASS                 2
#define CODECHAL_ENCODE_VP9_REF_SEGMENT_DISABLED                0xFF
#define CODECHAL_ENCODE_VP9_BRC_MAX_NUM_OF_PASSES               4
#define CODECHAL_ENCODE_VP9_CQP_NUM_OF_PASSES                   2
#define CODECHAL_ENCODE_VP9_BRC_DEFAULT_NUM_OF_PASSES           2   // 2 Passes minimum so HuC is Run twice, second PAK is conditional.
#define CODECHAL_ENCODE_VP9_BRC_HISTORY_BUFFER_SIZE             768
#define CODECHAL_ENCODE_VP9_BRC_CONSTANTSURFACE_SIZE            17792
#define CODECHAL_ENCODE_VP9_SEGMENT_STATE_BUFFER_SIZE           256
#define CODECHAL_ENCODE_VP9_BRC_BITSTREAM_SIZE_BUFFER_SIZE      16
#define CODECHAL_ENCODE_VP9_BRC_MSDK_PAK_BUFFER_SIZE            64
#define __MEDIA_USER_FEATURE_VALUE_VP9_ENCODE_ENABLE_BRC_DLL                 5158
#define __MEDIA_USER_FEATURE_VALUE_VP9_ENCODE_BRC_DLL_PATH                   5159
#define __MEDIA_USER_FEATURE_VALUE_VP9_ENCODE_ENABLE_BRC_DLL_CUSTOMPATH      5160

#define VP9SWBRCLIB  "VP9BRCDLL.dll"

typedef struct _HUC_AUX_BUFFER
{
    uint32_t Function;   // reserved for function related flags
    uint32_t HuCStatus;  // HuC Status
    uint8_t  BRC_PrevSceneChgType_U8;
    uint8_t  BRC_PrevSceneChgFrmAway_U8;
    uint8_t  rsvd2;
    uint8_t  rsvd3;
    uint32_t RSVD[13];
} HUC_AUX_BUFFER, *PHUC_AUX_BUFFER;
#define CODECHAL_ENCODE_VP9_FRAME_HEADER_SIZE                   4096
#define CODECHAL_ENCODE_VP9_MAX_NAL_UNIT_TYPE                   1   // only support one NAL unit for uncompressed header
#define ENCODE_VP9_8K_PIC_WIDTH     8192
#define ENCODE_VP9_8K_PIC_HEIGHT    8192
#define ENCODE_VP9_16K_PIC_WIDTH     16384
#define ENCODE_VP9_16K_PIC_HEIGHT    16384

extern const uint8_t Keyframe_Default_Probs[2048];
extern const uint8_t Inter_Default_Probs[2048];
extern const uint8_t LF_VALUE_QP_LOOKUP[256];

//!
//! \struct    BRC_BITSTREAM_SIZE_BUFFER
//! \brief     Brc bitstream size buffer
//!
struct BRC_BITSTREAM_SIZE_BUFFER
{
    uint32_t dwHcpBitstreamByteCountFrame;
    uint32_t dwHcpImageStatusControl;
    uint32_t Reserved[2];
};

//!
//! \struct    CU_DATA
//! \brief     CU data
//!
struct CU_DATA
{
    // DW0
    uint32_t        cu_size : 2;
    uint32_t        Res_DW0_2_3 : 2;
    uint32_t        cu_part_mode : 2;    // 0=2Nx2N,1=2NxN,2=Nx2N,3=NxN(8x8 only)
    uint32_t        Res_DW0_6_7 : 2;
    uint32_t        intra_chroma_mode0 : 4;    // 0=DC,1=V,2=H,3=TM,4=D45,5=D135,6=D117,7=D153,8=D207,9=D63
    uint32_t        Res_DW0_12_15 : 4;
    uint32_t        intra_chroma_mode1 : 4;    // 0=DC,1=V,2=H,3=TM,4=D45,5=D135,6=D117,7=D153,8=D207,9=D63
    uint32_t        cu_pred_mode0 : 1;    // 1=Intra,0=Inter
    uint32_t        cu_pred_mode1 : 1;
    uint32_t        Res_DW0_23_22 : 2;
    uint32_t        interpred_comp0 : 1;    // 0=single,1=compound
    uint32_t        interpred_comp1 : 1;
    uint32_t        Res_DW0_31_26 : 6;

    //DW1
    uint32_t        intra_mode0 : 4;    // 0=DC,1=V,2=H,3=TM,4=D45,5=D135,6=D117,7=D153,8=D207,9=D63
    uint32_t        Res_DW1_4_7 : 4;
    uint32_t        intra_mode1 : 4;    // 0=DC,1=V,2=H,3=TM,4=D45,5=D135,6=D117,7=D153,8=D207,9=D63
    uint32_t        Res_DW1_12_15 : 4;
    uint32_t        intra_mode2 : 4;    // 0=DC,1=V,2=H,3=TM,4=D45,5=D135,6=D117,7=D153,8=D207,9=D63
    uint32_t        Res_DW1_20_23 : 4;
    uint32_t        intra_mode3 : 4;    // 0=DC,1=V,2=H,3=TM,4=D45,5=D135,6=D117,7=D153,8=D207,9=D63
    uint32_t        Res_DW1_28_31 : 4;

    //DW2
    int16_t        mvx_l0_part0 : 16;
    int16_t        mvy_l0_part0 : 16;

    //DW3
    int16_t        mvx_l0_part1 : 16;
    int16_t        mvy_l0_part1 : 16;

    //DW4
    int16_t        mvx_l0_part2 : 16;
    int16_t        mvy_l0_part2 : 16;

    //DW5
    int16_t        mvx_l0_part3 : 16;
    int16_t        mvy_l0_part3 : 16;

    //DW6
    int16_t        mvx_l1_part0 : 16;
    int16_t        mvy_l1_part0 : 16;

    //DW7
    int16_t        mvx_l1_part1 : 16;
    int16_t        mvy_l1_part1 : 16;

    //DW8
    int16_t        mvx_l1_part2 : 16;
    int16_t        mvy_l1_part2 : 16;

    //DW9
    int16_t        mvx_l1_part3 : 16;
    int16_t        mvy_l1_part3 : 16;

    //DW10
    uint32_t        refframe_part0_l0 : 2;    // 0=intra,1=last,2=golden,3=altref
    uint32_t        Res_DW10_2_3 : 2;
    uint32_t        refframe_part1_l0 : 2;    // 0=intra,1=last,2=golden,3=altref
    uint32_t        Res_DW10_6_7 : 2;
    uint32_t        refframe_part0_l1 : 2;    // 0=intra,1=last,2=golden,3=altref
    uint32_t        Res_DW10_10_11 : 2;
    uint32_t        refframe_part1_l1 : 2;    // 0=intra,1=last,2=golden,3=altref
    uint32_t        Res_DW10_14_15 : 2;
    uint32_t        round_part0 : 3;
    uint32_t        Res_DW10_19 : 1;
    uint32_t        round_part1 : 3;
    uint32_t        Res_DW10_23_31 : 9;

    //DW11
    uint32_t        tu_size0 : 2;
    uint32_t        tu_size1 : 2;
    uint32_t        Res_DW11_4_13 : 10;
    uint32_t        segidx_pred0 : 1;
    uint32_t        segidx_pred1 : 1;
    uint32_t        segidx_part0 : 3;
    uint32_t        segidx_part1 : 3;
    uint32_t        mc_filtertype_part0 : 2;
    uint32_t        mc_filtertype_part1 : 2;
    uint32_t        Res_DW11_26_31 : 6;

    uint32_t        Res_DW12 : 32;

    uint32_t        Res_DW13 : 32;

    uint32_t        Res_DW14 : 32;

    uint32_t        Res_DW15 : 32;

};
C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(CU_DATA)) == 16);

//!
//! \enum     VP9_MBBRC_MODE
//! \brief    VP9 mbbrc mode
//!
enum VP9_MBBRC_MODE
{
    //Target usage determines whether MBBRC is enabled or not.
    //Currently for all the target usages it is enabled.
    //once the performance is measured for performance TU mode, decision will be taken
    //whether to enable or disable MBBRC.
    MBBRC_ENABLED_TU_DEPENDENCY = 0,
    MBBRC_ENABLED = 1,
    MBBRC_DISABLED = 2

};

//!
//! \enum     TU_MODE
//! \brief    TU mode
//!
enum TU_MODE
{
    TU_QUALITY = 1,
    TU_NORMAL = 4,
    TU_PERFORMANCE = 7
};

//!
//! \enum     DYS_REF_FLAGS
//! \brief    DYS reference flags
//!
enum DYS_REF_FLAGS
{
    DYS_REF_NONE = 0,
    DYS_REF_LAST = (1 << 0),
    DYS_REF_GOLDEN = (1 << 1),
    DYS_REF_ALT = (1 << 2),
};

//!
//! \enum     PRED_MODE
//! \brief    Pred mode
//!
enum PRED_MODE
{
    PRED_MODE_SINGLE = 0,
    PRED_MODE_COMPOUND = 1,
    PRED_MODE_HYBRID = 2
};

//!
//! \class    CodechalVdencVp9State
//! \brief    Codechal Vdenc Vp9 state
//!
class CodechalVdencVp9State : public CodechalEncoderState
{
public:
    //!
    //! \struct    Compressed Header
    //! \brief     Compressed header
    //!
    struct CompressedHeader
    {
        union {
            struct {
                uint8_t valid         : 1;  // valid =1, invalid = 0
                uint8_t bin_probdiff  : 1;  // 1= bin, 0 = prob diff
                uint8_t prob          : 1;  // 0 = 128, 1 = 252
                uint8_t bin           : 1;
                uint8_t reserved      : 4;
            } fields;
            uint8_t value;
        };
    };

    //!
    //! \struct    DysSamplerStateParams
    //! \brief     Dys sampler state parameters
    //!
    struct DysSamplerStateParams
    {
        PMHW_KERNEL_STATE      pKernelState;
    };

    //!
    //! \struct    DysCurbeParams
    //! \brief     Dys curbe parameters
    //!
    struct DysCurbeParams
    {
        uint32_t                                dwInputWidth;
        uint32_t                                dwInputHeight;
        uint32_t                                dwOutputWidth;
        uint32_t                                dwOutputHeight;
        PMHW_KERNEL_STATE                       pKernelState;
    };

    //!
    //! \struct    DysKernelParams
    //! \brief     Dys kernel parameters
    //!
    struct DysKernelParams
    {
        uint32_t            dwInputWidth;
        uint32_t            dwInputHeight;
        uint32_t            dwOutputWidth;
        uint32_t            dwOutputHeight;
        PMOS_SURFACE        psInputSurface;
        PMOS_SURFACE        psOutputSurface;
    };

    //!
    //! \struct    HcpPakObject
    //! \brief     HCP pak object
    //!
    struct HcpPakObject
    {
        // DW0
        struct
        {
            uint32_t    DwordLength                     : 16; //[15:0]
            uint32_t    SubOp                           : 7; //[22:16]
            uint32_t    Opcode                          : 6; //[28:23]
            uint32_t    Type                            : 3; //[31:29]
        } DW0;

        //DW1
        struct
        {
            uint32_t    Split_flag_level2_level1part0   : 4;
            uint32_t    Split_flag_level2_level1part1   : 4;
            uint32_t    Split_flag_level2_level1part2   : 4;
            uint32_t    Split_flag_level2_level1part3   : 4;
            uint32_t    Split_flag_level1               : 4;
            uint32_t    Split_flag_level0               : 1;
            uint32_t    Reserved21_23                   : 3;
            uint32_t    CU_count_minus1                 : 6;
            uint32_t    IsLastSBFrameflag               : 1;
            uint32_t    IsLastSBTileflag                : 1;
        } DW1;

        //DW2
        struct
        {
            uint32_t    Current_SB_X_Addr               : 16;
            uint32_t    Current_SB_Y_Addr               : 16;
        } DW2;

        //DW3
        uint32_t Reserved_DW03                          : 32;

    };
    C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(HcpPakObject)) == 4);

    //!
    //! \struct    HucFrameCtrl
    //! \brief     HUC frame contol
    //!
    struct HucFrameCtrl
    {
        uint32_t               FrameType;    //0:INTRA, 1:INTER                // DW15
        uint32_t               ShowFrame;                                      // DW16
        uint32_t               ErrorResilientMode;                             // DW17
        uint32_t               IntraOnly;                                      // DW18
        uint32_t               ContextReset;                                   // DW19
        uint32_t               LastRefFrameBias;                               // DW20
        uint32_t               GoldenRefFrameBias;                             // DW21
        uint32_t               AltRefFrameBias;                                // DW22
        uint32_t               AllowHighPrecisionMv;                           // DW23
        uint32_t               McompFilterMode;                                // DW24
        uint32_t               TxMode;                                         // DW25
        uint32_t               RefreshFrameContext;                            // DW26
        uint32_t               FrameParallelDecode;                            // DW27
        uint32_t               CompPredMode;                                   // DW28
        uint32_t               FrameContextIdx;                                // DW29
        uint32_t               SharpnessLevel;                                 // DW30
        uint32_t               SegOn;                                          // DW31
        uint32_t               SegMapUpdate;                                   // DW32
        uint32_t               SegUpdateData;                                  // DW33
        uint8_t                Rsvd[13];                                       // DW34-36, first byte of 37
        uint8_t                log2TileCols;                                   // DW37
        uint8_t                log2TileRows;                                   // DW37
        uint8_t                Reserved[5];                                    // DW37 last byte, DW38
    };

    //!
    //! \struct    HucBrcBuffers
    //! \brief     HUC brc buffers
    //!
    struct HucBrcBuffers
    {
        MOS_RESOURCE           resBrcHistoryBuffer;
        MOS_RESOURCE           resBrcConstantDataBuffer;
        MOS_RESOURCE           resBrcMsdkPakBuffer;
        MOS_RESOURCE           resBrcMbEncCurbeWriteBuffer;
        MOS_RESOURCE           resMbEncAdvancedDsh;
        MOS_RESOURCE           resPicStateBrcReadBuffer;
        MOS_RESOURCE           resPicStateBrcWriteHucReadBuffer;
        MOS_RESOURCE           resPicStateHucWriteBuffer;
        MOS_RESOURCE           resSegmentStateBrcReadBuffer;
        MOS_RESOURCE           resSegmentStateBrcWriteBuffer;
        MOS_RESOURCE           resBrcBitstreamSizeBuffer;
        MOS_RESOURCE           resBrcHucDataBuffer;
    };
    //!
    //! \struct    HucPrevFrameInfo
    //! \brief     HUC definition: PrevFrameInfo
    //!
    struct HucPrevFrameInfo
    {
        uint32_t               IntraOnly;                                      // DW39
        uint32_t               FrameWidth;                                     // DW40
        uint32_t               FrameHeight;                                    // DW41
        uint32_t               KeyFrame;                                       // DW42
        uint32_t               ShowFrame;                                      // DW43
    };

    //!
    //! \struct    HucProbDmem
    //! \brief     HUC prob dmem
    //!
    struct HucProbDmem
    {
        uint32_t                                    HuCPassNum;
        uint32_t                                    FrameWidth;
        uint32_t                                    FrameHeight;
        uint32_t                                    Rsvd32[6];
        char                                        SegmentRef[CODEC_VP9_MAX_SEGMENTS];
        uint8_t                                     SegmentSkip[CODEC_VP9_MAX_SEGMENTS];
        uint8_t                                     SegCodeAbs;
        uint8_t                                     SegTemporalUpdate;
        uint8_t                                     LastRefIndex;
        uint8_t                                     GoldenRefIndex;
        uint8_t                                     AltRefIndex;
        uint8_t                                     RefreshFrameFlags;
        uint8_t                                     RefFrameFlags;
        uint8_t                                     ContextFrameTypes;
        HucFrameCtrl                                FrameCtrl;
        HucPrevFrameInfo                            PrevFrameInfo;
        uint8_t                                     Rsvd[2];
        uint8_t                                     FrameToShow;
        uint8_t                                     LoadKeyFrameDefaultProbs;
        uint32_t                                    FrameSize;
        uint32_t                                    Reserved1;
        uint32_t                                    RePak;
        uint16_t                                    LFLevelBitOffset;
        uint16_t                                    QIndexBitOffset;
        uint16_t                                    SegBitOffset;
        uint16_t                                    SegLengthInBits;
        uint16_t                                    UnCompHdrTotalLengthInBits;
        uint16_t                                    SegUpdateDisable;
        int32_t                                     RePakThreshold[256];
        uint16_t                                    PicStateOffset;
        uint16_t                                    SLBBSize;
        uint8_t                                     StreamInEnable;
        uint8_t                                     StreamInSegEnable;
        uint8_t                                     DisableDMA;
        uint8_t                                     IVFHeaderSize;
        uint8_t                                     Reserved[44];
    };

    //!
    //! \struct    HucBrcInitDmem
    //! \brief     HUC brc init dmem
    //!
    struct HucBrcInitDmem
    {
        uint32_t    BRCFunc;                          // 0: Init; 2: Reset
        uint32_t    ProfileLevelMaxFrame;             // Limit on maximum frame size based on selected profile and level, and can be user defined
        uint32_t    InitBufFullness;                  // Initial buffer fullness
        uint32_t    BufSize;                          // Buffer size
        uint32_t    TargetBitrate;                    // Average(target) bit rate
        uint32_t    MaxRate;                          // Maximum bit rate in bits per second (bps).
        uint32_t    MinRate;                          // Minimum bit rate
        uint32_t    FrameRateM;                       // Framerate numerator
        uint32_t    FrameRateD;                       // Framerate denominator
        uint32_t    RSVD32[4];                        // Reserved, MBZ

        uint16_t    BRCFlag;                          // BRC flag
        uint16_t    GopP;                             // number of P frames in a GOP
        uint16_t    Reserved;
        uint16_t    FrameWidth;                       // Frame width
        uint16_t    FrameHeight;                      // Frame height
        uint16_t    MinQP;                            // Minimum QP
        uint16_t    MaxQP;                            // Maximum QP
        uint16_t    LevelQP;                          // Level QP
        uint16_t    GoldenFrameInterval;              // Golden frame interval
        uint16_t    EnableScaling;                    // Enable resolution scaling
        uint16_t    OvershootCBR;                     // default: 115, CBR overshoot percentage
        uint16_t    RSVD16[5];                        // Reserved, MBZ

        int8_t      InstRateThreshP0[4];              // Instant rate threshold for P frame
        int8_t      Reserved2[4];
        int8_t      InstRateThreshI0[4];
        int8_t      DevThreshPB0[8];                  // Deviation threshold for P and B frame
        int8_t      DevThreshVBR0[8];                 // Deviation threshold for VBR control
        int8_t      DevThreshI0[8];                   // Deviation threshold for I frame

        uint8_t     InitQPP;
        uint8_t     InitQPI;
        uint8_t     RSVD3;
        uint8_t     Total_Level;
        uint8_t     MaxLevel_Ratio[16];
        uint8_t     SlidingWindowEnable;
        uint8_t     SlidingWindowSize;
        uint8_t     RSVD8[47];                        // Reserved, MBZ
    };

    //!
    //! \struct    HucBrcUpdateDmem
    //! \brief     HUC brc update dmem
    //!
    struct HucBrcUpdateDmem
    {
        int32_t       UPD_TARGET_BUF_FULLNESS_U32;        //passed by the driver
        uint32_t      UPD_FRAMENUM_U32;                   //passed by the driver
        int32_t       UPD_HRD_BUFF_FULLNESS_UPPER_I32;    //passed by the driver
        int32_t       UPD_HRD_BUFF_FULLNESS_LOWER_I32;    //passed by the driver
        uint32_t      RSVD32[7];                          // mbz

        uint16_t      UPD_startGAdjFrame_U16[4];         // start global adjust frame (4 items)
        uint16_t      UPD_CurWidth_U16;                   // current width
        uint16_t      UPD_CurHeight_U16;                  // current height
        uint16_t      UPD_Asyn_U16;
        uint16_t      UPD_VDEncImgStateOffset;            // the image state start position in bytes from the begining of Second Level BB
        uint16_t      UPD_SLBBSize;                       // second level batch buffer total size in bytes
        uint16_t      UPD_PicStateOffset;                 // the pic state offset in bytes from the beginning of second level batch buffer
        uint16_t      RSVD16[6];                          // mbz

        uint8_t       UPD_OVERFLOW_FLAG_U8;               //passed by the driver
        uint8_t       UPD_BRCFlag_U8;                     //BRC flag, 0 - nothing to report, others - BRCPIC\BRCCUR flag defines 1 - scene change, etc // RSVD on G10, remove when G11 drops dependency
        uint8_t       UPD_MaxNumPAKs_U8;                  //maximum number of PAKs (default set to 4)
        int8_t        UPD_CurrFrameType_U8;               //current frame type (0:P, 1:B, 2:I)
        uint8_t       UPD_QPThreshold_U8[4];              // QP threshold (4 entries)
        uint8_t       UPD_gRateRatioThreshold_U8[6];      // global rate ratio threshold (6 items)
        int8_t        UPD_startGAdjMult_U8[5];            // start global adjust mult (5 items)
        int8_t        UPD_startGAdjDiv_U8[5];             // start global adjust div (5 items)
        int8_t        UPD_gRateRatioThresholdQP_U8[7];    // global rate ratio threshold QP (7 items)
        uint8_t       UPD_DistThreshldI_U8[9];            //(N_DISTORION_THRESHLDS+1) distortion thresholds for I frames
        uint8_t       UPD_DistThreshldP_U8[9];            //(N_DISTORION_THRESHLDS+1) distortion thresholds for P frames
        uint8_t       UPD_DistThreshldB_U8[9];            //(N_DISTORION_THRESHLDS+1) distortion thresholds for B frames; no needed for Vp8 - to clean up
        int8_t        UPD_MaxFrameThreshI_U8[5];          //num qp threshld + 1 of multiplyers
        int8_t        UPD_MaxFrameThreshP_U8[5];          //num qp threshld + 1 of multiplyers
        int8_t        UPD_MaxFrameThreshB_U8[5];          //num qp threshld + 1 of multiplyers; no needed for Vp8 - to clean up
        uint8_t       UPD_PAKPassNum_U8;                  // current pak pass number
        uint8_t       UPD_ACQQp_U8;
        int8_t        UPD_DeltaQPForSadZone0_I8;
        int8_t        UPD_DeltaQPForSadZone1_I8;
        int8_t        UPD_DeltaQPForSadZone2_I8;
        int8_t        UPD_DeltaQPForSadZone3_I8;
        int8_t        UPD_DeltaQPForMvZero_I8;
        int8_t        UPD_DeltaQPForMvZone0_I8;
        int8_t        UPD_DeltaQPForMvZone1_I8;
        int8_t        UPD_DeltaQPForMvZone2_I8;
        uint8_t       UPD_Temporal_Level_U8;
        uint8_t       UPD_SegMapGenerating_U8;            // Default 0: HuC does not update segmentation state; 1: HuC updates all 8 segmentation states in second level batch buffer
        uint8_t       RSVD8[95];                          // mbz
    };

    //!
    //! \struct    HucBrcDataBuffer
    //! \brief     HUC brc data buffer
    //!
    struct HucBrcDataBuffer
    {
        //DW0-DW4
        uint32_t   Reserved1[5];

        // DW5
        union
        {
            struct
            {
                uint32_t NextFrameWidth : MOS_BITFIELD_RANGE(0, 15);
                uint32_t NextFrameHeight : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW5;

        // DW6, DW7
        uint32_t Reserved2[2];
    };

    //!
    //! \struct    DysBindingTable
    //! \brief     Dys binding table
    //!
    struct DysBindingTable
    {
        uint32_t   dysInputFrameNv12 = 0;
        uint32_t   dysOutputFrameY = 1;
        uint32_t   dysOutputFrameUV = 2;
    };

    //!
    //! \struct    DysSurfaceParams
    //! \brief     Dys surface data
    //!
    struct DysSurfaceParams
    {
        PMOS_SURFACE        inputFrameSurface;
        PMOS_SURFACE        outputFrameSurface;
        uint32_t            verticalLineStride;
        uint32_t            verticalLineStrideOffset;
        DysBindingTable*    dysBindingTable;
        PMHW_KERNEL_STATE   kernelState;
    };

    //!
    //! \struct    DysStaticData
    //! \brief     Dys static data
    //!
    struct DysStaticData
    {
        // DW0
        union
        {
            struct
            {
                uint32_t   InputFrameWidth                 : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   InputFrameHeight                : MOS_BITFIELD_RANGE(16, 31);
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
                uint32_t   OutputFrameWidth                : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   OutputFrameHeight               : MOS_BITFIELD_RANGE(16, 31);
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
                float   DeltaU;
            };

            uint32_t       Value;
        } DW2;

        // DW3
        union
        {
            struct
            {
                float   DeltaV;
            };

            uint32_t       Value;
        } DW3;

        // DW4 - DW15
        uint32_t           Reserved[12];

        // DW16
        union
        {
            struct
            {
                uint32_t   InputFrameNV12SurfBTI           : MOS_BITFIELD_RANGE(0, 31);
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
                uint32_t   OutputFrameYSurfBTI             : MOS_BITFIELD_RANGE(0, 31);
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
                uint32_t   AVSSampleIdx                    : MOS_BITFIELD_RANGE(0, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW18;
    };
    C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(DysStaticData)) == 19);

    typedef enum VP9BufferType
    {
        eVp9UnknownBuff = 0,
        eVp9INLINE_DMEM,
        eVp9HISTORY_BUFF,
        eVp9VDENC_STATISTICS_BUFF,
        eVp9PAK_STATISTICS_BUFF,
        eVp9INPUT_SLBB_BUFF,
        eVp9BRC_DATA_BUFF,
        eVp9CONSTANT_DATA_BUFF,
        eVp9OUTPUT_SLBB_BUFF,
        eVp9PAKMMIO_BUFF,
        eVp9AUX_BUFF
    } VP9BufferType;

    //!
    //! \struct    VdencVmeState
    //! \brief     Vdenc vme state
    //!
    struct VdencVmeState
    {
        PCODEC_REF_LIST           pRefList[CODECHAL_NUM_UNCOMPRESSED_SURFACE_HEVC];
        CODEC_PIC_ID              PicIdx[CODEC_MAX_NUM_REF_FRAME_HEVC];
        MOS_SURFACE               s16xMeMvDataBuffer;
        MOS_SURFACE               s4xMeMvDataBuffer;
        MOS_SURFACE               s32xMeMvDataBuffer;
        MOS_SURFACE               s4xMeDistortionBuffer;
        uint8_t                   Level;
        uint16_t                  direct_spatial_mv_pred_flag;
        uint32_t                  dwBiWeight;
        bool                      bFirstFieldIdrPic;
        bool                      bMbaff;
        EncodeBrcBuffers          BrcBuffer;

        bool                      b16xMeInUse;
        bool                      b4xMeInUse;
        bool                      segmapProvided;

        //Sequence Params
        uint8_t                   TargetUsage;
        uint8_t                   GopRefDist;

        //Picture Params
        CODEC_PICTURE             CurrOriginalPic;
        int8_t                    QpY;

        //Slice Params
        CODEC_PICTURE             RefPicList[2][CODEC_MAX_NUM_REF_FRAME_HEVC];
        uint8_t                   num_ref_idx_l0_active_minus1;   // [0..15]
        uint8_t                   num_ref_idx_l1_active_minus1;   // [0..15]
        int8_t                    slice_qp_delta;
    };

    //! 
    //! \struct    VdencMeCurbe
    //! \brief     Vdenc Me curbe
    //!
    struct VdencMeCurbe
    {
        // DW0
        union
        {
            struct
            {
                uint32_t   SkipModeEn                          : MOS_BITFIELD_BIT(       0 );
                uint32_t   AdaptiveEn                          : MOS_BITFIELD_BIT(       1 );
                uint32_t   BiMixDis                            : MOS_BITFIELD_BIT(       2 );
                uint32_t                                       : MOS_BITFIELD_RANGE(  3, 4 );
                uint32_t   EarlyImeSuccessEn                   : MOS_BITFIELD_BIT(       5 );
                uint32_t                                       : MOS_BITFIELD_BIT(       6 );
                uint32_t   T8x8FlagForInterEn                  : MOS_BITFIELD_BIT(       7 );
                uint32_t                                       : MOS_BITFIELD_RANGE(  8,23 );
                uint32_t   EarlyImeStop                        : MOS_BITFIELD_RANGE( 24,31 );
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
                uint32_t   MaxNumMVs                           : MOS_BITFIELD_RANGE(  0, 5 );
                uint32_t                                       : MOS_BITFIELD_RANGE(  6,15 );
                uint32_t   BiWeight                            : MOS_BITFIELD_RANGE( 16,21 );
                uint32_t                                       : MOS_BITFIELD_RANGE( 22,27 );
                uint32_t   UniMixDisable                       : MOS_BITFIELD_BIT(      28 );
                uint32_t                                       : MOS_BITFIELD_RANGE( 29,31 );
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
                uint32_t   MaxLenSP                                    : MOS_BITFIELD_RANGE( 0, 7 );
                uint32_t   MaxNumSU                                    : MOS_BITFIELD_RANGE( 8, 15 );
                uint32_t                                               : MOS_BITFIELD_RANGE( 16, 31);
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
                uint32_t   SrcSize                             : MOS_BITFIELD_RANGE(  0, 1 );
                uint32_t                                       : MOS_BITFIELD_RANGE(  2, 3 );
                uint32_t   MbTypeRemap                         : MOS_BITFIELD_RANGE(  4, 5 );
                uint32_t   SrcAccess                           : MOS_BITFIELD_BIT(       6 );
                uint32_t   RefAccess                           : MOS_BITFIELD_BIT(       7 );
                uint32_t   SearchCtrl                          : MOS_BITFIELD_RANGE(  8,10 );
                uint32_t   DualSearchPathOption                : MOS_BITFIELD_BIT(      11 );
                uint32_t   SubPelMode                          : MOS_BITFIELD_RANGE( 12,13 );
                uint32_t   SkipType                            : MOS_BITFIELD_BIT(      14 );
                uint32_t   DisableFieldCacheAlloc              : MOS_BITFIELD_BIT(      15 );
                uint32_t   InterChromaMode                     : MOS_BITFIELD_BIT(      16 );
                uint32_t   FTEnable                            : MOS_BITFIELD_BIT(      17 );
                uint32_t   BMEDisableFBR                       : MOS_BITFIELD_BIT(      18 );
                uint32_t   BlockBasedSkipEnable                : MOS_BITFIELD_BIT(      19 );
                uint32_t   InterSAD                            : MOS_BITFIELD_RANGE( 20,21 );
                uint32_t   IntraSAD                            : MOS_BITFIELD_RANGE( 22,23 );
                uint32_t   SubMbPartMask                       : MOS_BITFIELD_RANGE( 24,30 );
                uint32_t                                       : MOS_BITFIELD_BIT(      31 );
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
                uint32_t                                               : MOS_BITFIELD_RANGE(0, 7);
                uint32_t   PictureHeightMinus1                         : MOS_BITFIELD_RANGE(8, 15);
                uint32_t   PictureWidth                                : MOS_BITFIELD_RANGE(16, 23);
                uint32_t                                               : MOS_BITFIELD_RANGE(24, 31);
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
                uint32_t                                               : MOS_BITFIELD_RANGE(0, 7);
                uint32_t   QpPrimeY                                    : MOS_BITFIELD_RANGE(8, 15);
                uint32_t   RefWidth                                    : MOS_BITFIELD_RANGE( 16,23 );
                uint32_t   RefHeight                                   : MOS_BITFIELD_RANGE( 24,31 );
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
                uint32_t                                               : MOS_BITFIELD_BIT(0);
                uint32_t   InputStreamInEn                             : MOS_BITFIELD_BIT(1);
                uint32_t   LCUSize                                     : MOS_BITFIELD_BIT(2);
                uint32_t   WriteDistortions                            : MOS_BITFIELD_BIT(3);
                uint32_t   UseMvFromPrevStep                           : MOS_BITFIELD_BIT(4);
                uint32_t                                               : MOS_BITFIELD_RANGE(5, 7);
                uint32_t   SuperCombineDist                            : MOS_BITFIELD_RANGE(8, 15);
                uint32_t   MaxVmvR                                     : MOS_BITFIELD_RANGE(16, 31);
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
                uint32_t                                       : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   MVCostScaleFactor                   : MOS_BITFIELD_RANGE( 16,17 );
                uint32_t   BilinearEnable                      : MOS_BITFIELD_BIT(      18 );
                uint32_t   SrcFieldPolarity                    : MOS_BITFIELD_BIT(      19 );
                uint32_t   WeightedSADHAAR                     : MOS_BITFIELD_BIT(      20 );
                uint32_t   AConlyHAAR                          : MOS_BITFIELD_BIT(      21 );
                uint32_t   RefIDCostMode                       : MOS_BITFIELD_BIT(      22 );
                uint32_t                                       : MOS_BITFIELD_BIT(      23 );
                uint32_t   SkipCenterMask                      : MOS_BITFIELD_RANGE( 24,31 );
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
                uint32_t   Mode0Cost                           : MOS_BITFIELD_RANGE(  0, 7 );
                uint32_t   Mode1Cost                           : MOS_BITFIELD_RANGE(  8,15 );
                uint32_t   Mode2Cost                           : MOS_BITFIELD_RANGE( 16,23 );
                uint32_t   Mode3Cost                           : MOS_BITFIELD_RANGE( 24,31 );
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
                uint32_t   Mode4Cost                           : MOS_BITFIELD_RANGE(  0, 7 );
                uint32_t   Mode5Cost                           : MOS_BITFIELD_RANGE(  8,15 );
                uint32_t   Mode6Cost                           : MOS_BITFIELD_RANGE( 16,23 );
                uint32_t   Mode7Cost                           : MOS_BITFIELD_RANGE( 24,31 );
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
                uint32_t   Mode8Cost                           : MOS_BITFIELD_RANGE(  0, 7 );
                uint32_t   Mode9Cost                           : MOS_BITFIELD_RANGE(  8,15 );
                uint32_t   RefIDCost                           : MOS_BITFIELD_RANGE( 16,23 );
                uint32_t   ChromaIntraModeCost                 : MOS_BITFIELD_RANGE( 24,31 );
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
                uint32_t   MV0Cost                             : MOS_BITFIELD_RANGE(  0, 7 );
                uint32_t   MV1Cost                             : MOS_BITFIELD_RANGE(  8,15 );
                uint32_t   MV2Cost                             : MOS_BITFIELD_RANGE( 16,23 );
                uint32_t   MV3Cost                             : MOS_BITFIELD_RANGE( 24,31 );
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
                uint32_t   MV4Cost                             : MOS_BITFIELD_RANGE(  0, 7 );
                uint32_t   MV5Cost                             : MOS_BITFIELD_RANGE(  8,15 );
                uint32_t   MV6Cost                             : MOS_BITFIELD_RANGE( 16,23 );
                uint32_t   MV7Cost                             : MOS_BITFIELD_RANGE( 24,31 );
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
                uint32_t   NumRefIdxL0MinusOne                         : MOS_BITFIELD_RANGE(0, 7);
                uint32_t   NumRefIdxL1MinusOne                         : MOS_BITFIELD_RANGE(8, 15);
                uint32_t   RefStreaminCost                             : MOS_BITFIELD_RANGE(16, 23);
                uint32_t   ROIEnable                                   : MOS_BITFIELD_RANGE(24, 26);
                uint32_t                                               : MOS_BITFIELD_RANGE(27, 31);
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
                uint32_t   List0RefID0FieldParity                      : MOS_BITFIELD_BIT(0);
                uint32_t   List0RefID1FieldParity                      : MOS_BITFIELD_BIT(1);
                uint32_t   List0RefID2FieldParity                      : MOS_BITFIELD_BIT(2);
                uint32_t   List0RefID3FieldParity                      : MOS_BITFIELD_BIT(3);
                uint32_t   List0RefID4FieldParity                      : MOS_BITFIELD_BIT(4);
                uint32_t   List0RefID5FieldParity                      : MOS_BITFIELD_BIT(5);
                uint32_t   List0RefID6FieldParity                      : MOS_BITFIELD_BIT(6);
                uint32_t   List0RefID7FieldParity                      : MOS_BITFIELD_BIT(7);
                uint32_t   List1RefID0FieldParity                      : MOS_BITFIELD_BIT(8);
                uint32_t   List1RefID1FieldParity                      : MOS_BITFIELD_BIT(9);
                uint32_t                                               : MOS_BITFIELD_RANGE(10, 31);
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
                uint32_t   PrevMvReadPosFactor                         : MOS_BITFIELD_RANGE(0, 7);
                uint32_t   MvShiftFactor                               : MOS_BITFIELD_RANGE(8, 15);
                uint32_t   Reserved                                    : MOS_BITFIELD_RANGE(16, 31);
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
                uint32_t   ActualMBWidth                            : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   ActualMBHeight                           : MOS_BITFIELD_RANGE(16, 31);
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
                uint32_t   RoiCtrl                                      : MOS_BITFIELD_RANGE( 0, 7 );
                uint32_t   MaxTuSize                                    : MOS_BITFIELD_RANGE( 8, 9 );
                uint32_t   MaxCuSize                                    : MOS_BITFIELD_RANGE( 10, 11 );
                uint32_t   NumImePredictors                             : MOS_BITFIELD_RANGE( 12, 15 );
                uint32_t   Reserved                                     : MOS_BITFIELD_RANGE( 16, 23 );
                uint32_t   PuTypeCtrl                                   : MOS_BITFIELD_RANGE( 24, 31 );
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
                uint32_t   ForceMvx0                                    : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   ForceMvy0                                    : MOS_BITFIELD_RANGE(16, 31);
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
                uint32_t   ForceMvx1                                    : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   ForceMvy1                                    : MOS_BITFIELD_RANGE(16, 31);
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
                uint32_t   ForceMvx2                                    : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   ForceMvy2                                    : MOS_BITFIELD_RANGE(16, 31);
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
                uint32_t   ForceMvx3                                    : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   ForceMvy3                                    : MOS_BITFIELD_RANGE(16, 31);
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
                uint32_t   ForceRefIdx0                                  : MOS_BITFIELD_RANGE(0, 3);
                uint32_t   ForceRefIdx1                                  : MOS_BITFIELD_RANGE(4, 7);
                uint32_t   ForceRefIdx2                                  : MOS_BITFIELD_RANGE(8, 11);
                uint32_t   ForceRefIdx3                                  : MOS_BITFIELD_RANGE(12, 15);
                uint32_t   NumMergeCandCu8x8                             : MOS_BITFIELD_RANGE(16, 19);
                uint32_t   NumMergeCandCu16x16                           : MOS_BITFIELD_RANGE(20, 23);
                uint32_t   NumMergeCandCu32x32                           : MOS_BITFIELD_RANGE(24, 27);
                uint32_t   NumMergeCandCu64x64                           : MOS_BITFIELD_RANGE(28, 31);
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
                uint32_t   SegID                                          : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   QpEnable                                       : MOS_BITFIELD_RANGE(16, 19);
                uint32_t   SegIDEnable                                    : MOS_BITFIELD_BIT(20);
                uint32_t   Reserved                                       : MOS_BITFIELD_RANGE(21, 22);
                uint32_t   ForceRefIdEnable                               : MOS_BITFIELD_BIT(23);
                uint32_t   Reserved1                                      : MOS_BITFIELD_RANGE(24, 31);
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
                uint32_t   ForceQp0                                       : MOS_BITFIELD_RANGE(0, 7);
                uint32_t   ForceQp1                                       : MOS_BITFIELD_RANGE(8, 15);
                uint32_t   ForceQp2                                       : MOS_BITFIELD_RANGE(16, 23);
                uint32_t   ForceQp3                                       : MOS_BITFIELD_RANGE(24, 31);
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
                uint32_t   Reserved                                       : MOS_BITFIELD_RANGE(0, 31);
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
                uint32_t   _4xMeMvOutputDataSurfIndex                  : MOS_BITFIELD_RANGE(0, 31);
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
                uint32_t   _16xOr32xMeMvInputDataSurfIndex              : MOS_BITFIELD_RANGE(0, 31);
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
                uint32_t   _4xMeOutputDistSurfIndex                    : MOS_BITFIELD_RANGE(0, 31);
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
                uint32_t   _4xMeOutputBrcDistSurfIndex                  : MOS_BITFIELD_RANGE(0, 31);
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
                uint32_t   VMEFwdInterPredictionSurfIndex              : MOS_BITFIELD_RANGE(0, 31);
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
                uint32_t   VMEBwdInterPredictionSurfIndex              : MOS_BITFIELD_RANGE(0, 31);
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
                uint32_t   VDEncStreamInOutputSurfIndex                 : MOS_BITFIELD_RANGE(0, 31);
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
                uint32_t   VDEncStreamInInputSurfIndex                   : MOS_BITFIELD_RANGE(0, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW47;
    };
    C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(VdencMeCurbe)) == 48);

    /* BinIdx for compressed header generation for PAK */
    /* The first value indicates previous SE index and second value indicates the size of the previous SE*/
    static constexpr uint32_t PAK_TX_MODE_IDX = 0;                                   //idx=0
    static constexpr uint32_t PAK_TX_MODE_SELECT_IDX = (PAK_TX_MODE_IDX + 2);                 //idx=2
    static constexpr uint32_t PAK_TX_8x8_PROB_IDX = (PAK_TX_MODE_SELECT_IDX + 1);          //idx=3
    static constexpr uint32_t PAK_TX_16x16_PROB_IDX = (PAK_TX_8x8_PROB_IDX + 4);             //idx=7
    static constexpr uint32_t PAK_TX_32x32_PROB_IDX = (PAK_TX_16x16_PROB_IDX + 8);           //idx=15
    static constexpr uint32_t PAK_TX_4x4_COEFF_PROB_IDX = (PAK_TX_32x32_PROB_IDX + 12);          //idx=27
    static constexpr uint32_t PAK_TX_8x8_COEFF_PROB_IDX = (PAK_TX_4x4_COEFF_PROB_IDX + 793);     //idx=820
    static constexpr uint32_t PAK_TX_16x16_COEFF_PROB_IDX = (PAK_TX_8x8_COEFF_PROB_IDX + 793);     //idx=1613
    static constexpr uint32_t PAK_TX_32x32_COEFF_PROB_IDX = (PAK_TX_16x16_COEFF_PROB_IDX + 793);   //idx=2406
    static constexpr uint32_t PAK_SKIP_CONTEXT_IDX = (PAK_TX_32x32_COEFF_PROB_IDX + 793);   //idx=3199
    static constexpr uint32_t PAK_INTER_MODE_CTX_IDX = (PAK_SKIP_CONTEXT_IDX + 6);            //idx=3205
    static constexpr uint32_t PAK_SWITCHABLE_FILTER_CTX_IDX = (PAK_INTER_MODE_CTX_IDX + 42);         //idx=3247
    static constexpr uint32_t PAK_INTRA_INTER_CTX_IDX = (PAK_SWITCHABLE_FILTER_CTX_IDX + 16);  //idx=3263
    static constexpr uint32_t PAK_COMPOUND_PRED_MODE_IDX = (PAK_INTRA_INTER_CTX_IDX + 8);         //idx=3271
    static constexpr uint32_t PAK_HYBRID_PRED_CTX_IDX = (PAK_COMPOUND_PRED_MODE_IDX + 2);      //idx=3273
    static constexpr uint32_t PAK_SINGLE_REF_PRED_CTX_IDX = (PAK_HYBRID_PRED_CTX_IDX + 10);        //idx=3283
    static constexpr uint32_t PAK_CMPUND_PRED_CTX_IDX = (PAK_SINGLE_REF_PRED_CTX_IDX + 20);    //idx=3303
    static constexpr uint32_t PAK_INTRA_MODE_PROB_CTX_IDX = (PAK_CMPUND_PRED_CTX_IDX + 10);        //idx=3313
    static constexpr uint32_t PAK_PARTITION_PROB_IDX = (PAK_INTRA_MODE_PROB_CTX_IDX + 72);    //idx=3385
    static constexpr uint32_t PAK_MVJOINTS_PROB_IDX = (PAK_PARTITION_PROB_IDX + 96);         //idx=3481
    static constexpr uint32_t PAK_MVCOMP0_IDX = (PAK_MVJOINTS_PROB_IDX + 24);          //idx=3505
    static constexpr uint32_t PAK_MVCOMP1_IDX = (PAK_MVCOMP0_IDX + 176);               //idx=3681
    static constexpr uint32_t PAK_MVFRAC_COMP0_IDX = (PAK_MVCOMP1_IDX + 176);               //idx=3857
    static constexpr uint32_t PAK_MVFRAC_COMP1_IDX = (PAK_MVFRAC_COMP0_IDX + 72);           //idx=3929
    static constexpr uint32_t PAK_MVHP_COMP0_IDX = (PAK_MVFRAC_COMP1_IDX + 72);           //idx=4001
    static constexpr uint32_t PAK_MVHP_COMP1_IDX = (PAK_MVHP_COMP0_IDX + 16);             //idx=4017
    static constexpr uint32_t PAK_COMPRESSED_HDR_SYNTAX_ELEMS = (PAK_MVHP_COMP1_IDX + 16);             //=4033

    static constexpr uint32_t m_dysStaticDataSize = sizeof(DysStaticData);

    static constexpr uint32_t m_brcMaxNumPasses = 3;
    static constexpr uint32_t m_numUncompressedSurface = 128;
    static constexpr uint32_t m_brcConstantSurfaceSize = 1664;
    static constexpr uint32_t m_segmentStateBlockSize = 32;
    static constexpr uint32_t m_probabilityCounterBufferSize = 193 * CODECHAL_CACHELINE_SIZE;

    static constexpr float m_devStdFps = 30.0;
    static constexpr float m_bpsRatioLow = 0.1f;
    static constexpr float m_bpsRatioHigh = 3.5;
    static constexpr int32_t m_numInstRateThresholds = 4;
    static constexpr int32_t m_numDevThresholds = 8;
    static constexpr int32_t m_posMultPb = 50;
    static constexpr int32_t m_negMultPb = -50;
    static constexpr int32_t m_posMultVbr = 100;
    static constexpr int32_t m_negMultVbr = -50;

    static constexpr uint32_t m_maxMvLen = 511;

    static constexpr int8_t m_instRateThresholdI[m_numInstRateThresholds] = { 30, 50, 90, 115 };
    static constexpr int8_t m_instRateThresholdP[m_numInstRateThresholds] = { 30, 50, 70, 120 };
    static constexpr double m_devThresholdFpNegI[m_numDevThresholds / 2] = { 0.80, 0.60, 0.34, 0.2 };
    static constexpr double m_devThresholdFpPosI[m_numDevThresholds / 2] = { 0.2, 0.4, 0.66, 0.9 };
    static constexpr double m_devThresholdFpNegPB[m_numDevThresholds / 2] = { 0.90, 0.66, 0.46, 0.3 };
    static constexpr double m_devThresholdFpPosPB[m_numDevThresholds / 2] = { 0.3, 0.46, 0.70, 0.90 };
    static constexpr double m_devThresholdVbrNeg[m_numDevThresholds / 2] = { 0.90, 0.70, 0.50, 0.3 };
    static constexpr double m_devThresholdVbrPos[m_numDevThresholds / 2] = { 0.4, 0.5, 0.75, 0.90 };

    static constexpr uint32_t m_vdboxHucVp9VdencBrcInitKernelDescriptor = 11;       //!< VDBox Huc VDEnc Brc init kernel descriptor
    static constexpr uint32_t m_vdboxHucVp9VdencBrcUpdateKernelDescriptor = 12;     //!< VDBox Huc VDEnc Brc init kernel descriptor
    static constexpr uint32_t m_vdboxHucVp9VdencProbKernelDescriptor = 13;          //!< VDBox Huc VDEnc prob kernel descriptor
    static constexpr uint32_t m_vdboxHucPakIntegrationKernelDescriptor = 15;     //!< VDBox Huc PAK integration kernel descriptor

    static const uint32_t m_vdencMeCurbeInit[48];
    static const uint32_t m_brcInitDmem[48];
    static const uint32_t m_brcUpdateDmem[64];
    static const uint32_t m_probDmem[320];
    static const uint32_t m_brcConstData[2][416];
    static const uint32_t m_samplerFilterCoeffs[32][6];

    static const uint32_t m_dysNumSurfaces = 3;

    static constexpr uint16_t m_cmd1Size = 120;
    static constexpr uint16_t m_cmd2Size = 148;

    // Parameters passed from application
    PCODEC_VP9_ENCODE_SEQUENCE_PARAMS m_vp9SeqParams     = nullptr;  //!< Pointer to sequence parameters
    PCODEC_VP9_ENCODE_PIC_PARAMS      m_vp9PicParams     = nullptr;  //!< Pointer to picture parameters
    PCODEC_VP9_ENCODE_SEGMENT_PARAMS  m_vp9SegmentParams = nullptr;  //!< Pointer to segment parameters

    CODEC_PIC_ID                                m_picIdx[CODEC_VP9_NUM_REF_FRAMES];
    PCODEC_REF_LIST                             m_refList[m_numUncompressedSurface];
    PCODECHAL_NAL_UNIT_PARAMS*                  m_nalUnitParams = nullptr;
    uint32_t                                    m_numNalUnit = 0;

    uint32_t                                    m_maxPicWidth = 0;
    uint32_t                                    m_maxPicHeight = 0;
    uint32_t                                    m_picWidthInSb = 0;
    uint32_t                                    m_picHeightInSb = 0;
    uint32_t                                    m_picSizeInSb = 0;

    uint8_t                                     m_txMode = 0;
    bool                                        m_hmeEnabled = false;
    bool                                        m_16xMeEnabled = false;
    bool                                        m_brcEnabled = false;
    bool                                        m_brcInit = false;
    bool                                        m_brcReset = false;
    bool                                        m_advancedDshInUse = false;
    bool                                        m_mbBrcEnabled = false;
    bool                                        m_waitForEnc = false;
    bool                                        m_adaptiveRepakSupported = false;
    bool                                        m_tsEnabled = false;
    bool                                        m_superFrameHucPass = false;
    bool                                        m_storeSingleTaskPhaseSupported = false; //For dynamic scaling, need to save the original state
    uint8_t                                     m_refFrameFlags = 0;
    uint8_t                                     m_numRefFrames = 0;
    uint8_t                                     m_dysRefFrameFlags = 0;
    uint8_t                                     m_dysCurrFrameFlag = 0;
    uint32_t                                    m_vdencPicStateSecondLevelBatchBufferSize = 0;

    MOS_RESOURCE m_resDeblockingFilterLineBuffer;
    MOS_RESOURCE m_resDeblockingFilterTileLineBuffer;
    MOS_RESOURCE m_resDeblockingFilterTileColumnBuffer;
    MOS_RESOURCE m_resMetadataLineBuffer;
    MOS_RESOURCE m_resMetadataTileLineBuffer;
    MOS_RESOURCE m_resMetadataTileColumnBuffer;
    MOS_RESOURCE m_resProbBuffer[CODEC_VP9_NUM_CONTEXTS];
    MOS_RESOURCE m_resSegmentIdBuffer;
    MOS_RESOURCE m_resHvcLineRowstoreBuffer;  // Handle of HVC Line Row Store surface
    MOS_RESOURCE m_resHvcTileRowstoreBuffer;  // Handle of HVC Tile Row Store surface
    MOS_RESOURCE m_resProbabilityDeltaBuffer;
    MOS_RESOURCE m_resTileRecordStrmOutBuffer;
    MOS_RESOURCE m_resCuStatsStrmOutBuffer;
    MOS_RESOURCE m_resCompressedHeaderBuffer;
    MOS_RESOURCE m_resProbabilityCounterBuffer;
    MOS_RESOURCE m_resModeDecision[2];
    MOS_RESOURCE m_resFrameStatStreamOutBuffer;
    MOS_RESOURCE m_resSseSrcPixelRowStoreBuffer;

    bool                                        m_clearAllToKey[CODEC_VP9_NUM_CONTEXTS] = { false };
    bool                                        m_isPreCtx0InterProbSaved                             = false;
    uint8_t                                     m_preCtx0InterProbSaved[CODECHAL_VP9_INTER_PROB_SIZE] = { 0 };

    HucPrevFrameInfo m_prevFrameInfo;

    uint8_t                                     m_contextFrameTypes[CODEC_VP9_NUM_CONTEXTS] = { 0 };
    uint8_t                                     m_currMvTemporalBufferIndex = 0;

    bool                                        m_hucEnabled = false;
    bool                                        m_segmentMapAllocated = false;
    MOS_RESOURCE                                m_resHucProbDmemBuffer[3];
    MOS_RESOURCE                                m_resHucDefaultProbBuffer;
    MOS_RESOURCE                                m_resHucProbOutputBuffer;
    MOS_RESOURCE                                m_resHucPakInsertUncompressedHeaderReadBuffer;
    MOS_RESOURCE                                m_resHucPakInsertUncompressedHeaderWriteBuffer;
    MOS_RESOURCE                                m_resHucPakMmioBuffer;
    MOS_RESOURCE                                m_resHucDebugOutputBuffer;
    MOS_SURFACE                                 m_mbSegmentMapSurface;
    MOS_SURFACE                                 m_output16X16InterModes;

    uint32_t                                    m_rePakThreshold[CODEC_VP9_QINDEX_RANGE] = { 0 };

    // ME
    MOS_SURFACE                                 m_4xMeMvDataBuffer;
    MOS_SURFACE                                 m_16xMeMvDataBuffer;
    MOS_SURFACE                                 m_4xMeDistortionBuffer;

    // BRC
    HucBrcBuffers                               m_brcBuffers;

    // DYS
    MHW_KERNEL_STATE                            m_dysKernelState;
    DysBindingTable                             m_dysBindingTable;
    uint32_t                                    m_dysDshSize = 0;

    // pointer to the reference surfaces
    PMOS_SURFACE                                m_lastRefPic = nullptr;
    PMOS_SURFACE                                m_goldenRefPic = nullptr;
    PMOS_SURFACE                                m_altRefPic = nullptr;

    bool                                        m_segmentMapProvided = false;
    bool                                        m_dysVdencMultiPassEnabled = false;
    bool                                        m_dysHucEnabled = false;
    bool                                        m_dysCqp = false;
    bool                                        m_dysBrc = false;
    bool                                        m_vdencPakonlyMultipassEnabled = false;
    bool                                        m_vdencPakObjCmdStreamOutEnabled = false;
    bool                                        m_prevFrameSegEnabled = false;
    uint32_t                                    m_vdencPictureState2ndLevelBatchBufferSize = 0;
    uint16_t                                    m_sadQpLambda = 0;
    uint16_t                                    m_rdQpLambda = 0;
    uint16_t                                    m_hucPicStateOffset = 0;
    uint16_t                                    m_hucSlbbSize = 0;
    uint8_t                                     m_vdencMvCosts[12] = { 0 };
    uint8_t                                     m_vdencRdMvCosts[12] = { 0 };
    uint8_t                                     m_vdencHmeMvCosts[8] = { 0 };
    uint8_t                                     m_vdencModeCosts[CODEC_VDENC_NUM_MODE_COST] = { 0 };
    uint32_t                                   *m_mapBuffer = nullptr;
    uint32_t                                    m_segStreamInHeight = 0;
    uint32_t                                    m_segStreamInWidth = 0;
    double                                      m_inputBitsPerFrame = 0.0;
    double                                      m_curTargetFullness = 0.0;
    uint16_t                                    m_slbbImgStateOffset = 0;
    uint32_t                                    m_defaultPictureStatesSize = 0;
    uint32_t                                    m_defaultPicturePatchListSize = 0;
    uint32_t                                    m_defaultHucCmdsSize = 0;
    uint32_t                                    m_defaultHucPatchListSize = 0;

    MOS_RESOURCE                                m_resVdencIntraRowStoreScratchBuffer;  // Handle of intra row store surface
    MOS_RESOURCE                                m_resVdencBrcStatsBuffer;
    MOS_RESOURCE                                m_resVdencSegmentMapStreamOut;
    MOS_RESOURCE                                m_resVdencPictureState2NdLevelBatchBufferRead[3][CODECHAL_VP9_ENCODE_RECYCLED_BUFFER_NUM];
    MOS_RESOURCE                                m_resVdencPictureState2NdLevelBatchBufferWrite[CODECHAL_VP9_ENCODE_RECYCLED_BUFFER_NUM];
    uint16_t                                    m_vdencPictureState2ndLevelBBIndex = 0;
    MOS_RESOURCE                                m_resVdencDysPictureState2NdLevelBatchBuffer;
    MOS_RESOURCE                                m_resVdencBrcInitDmemBuffer;
    MOS_RESOURCE                                m_resVdencBrcUpdateDmemBuffer[3];
    MOS_RESOURCE                                m_resVdencDataExtensionBuffer;
    CODECHAL_ENCODE_BUFFER                      m_resPakcuLevelStreamoutData;
    CODECHAL_ENCODE_BUFFER                      m_resPakSliceLevelStreamutData;

    uint32_t                                    m_maxTileNumber = 1;

    uint32_t                                    m_bitDepth = 0;
    uint8_t                                     m_chromaFormat = 0;
    uint32_t                                    m_sizeOfSseSrcPixelRowStoreBufferPerLcu = 0;
    PCODECHAL_CMD_INITIALIZER                   m_hucCmdInitializer = nullptr;

protected:
    //!
    //! \brief    Constructor
    //!
    CodechalVdencVp9State(CodechalHwInterface* hwInterface,
        CodechalDebugInterface* debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo);

    //!
    //! \brief    Set pipe buffer address parameter
    //! \details  Set pipe buffer address parameter in MMC case
    //!
    //! \param    [in,out] pipeBufAddrParams
    //!           Pointer to PMHW_VDBOX_PIPE_BUF_ADDR_PARAMS
    //! \param    [in] refSurface
    //!           Pointer to reference surfaces
    //! \param    [in] cmdBuffer
    //!           Pointer to MOS command buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetPipeBufAddr(
        PMHW_VDBOX_PIPE_BUF_ADDR_PARAMS pipeBufAddrParams,
        PMOS_SURFACE refSurface[3],
        PMOS_COMMAND_BUFFER cmdBuffer);

public:
    //!
    //! \brief    Destructor
    //!
    virtual ~CodechalVdencVp9State() {};

    virtual int GetCurrentPass()
    {
        return m_currPass;
    }

    virtual int GetNumPasses()
    {
        return m_numPasses;
    }

    virtual bool IsFirstPass()
    {
        return (m_currPass == 0) ? true : false;
    }

    virtual bool IsLastPass()
    {
        return (m_currPass == m_numPasses) ? true : false;
    }

    //!
    //! \brief    Perform platform capability check
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS PlatformCapabilityCheck()
    {
        return MOS_STATUS_SUCCESS;
    }

    virtual MOS_STATUS SetupSegmentationStreamIn() = 0;

    virtual void SetHcpPipeModeSelectParams(MHW_VDBOX_PIPE_MODE_SELECT_PARAMS& pipeModeSelectParams);

    virtual void SetHcpIndObjBaseAddrParams(MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS& indObjBaseAddrParams);

    virtual MOS_STATUS InitializePicture(const EncoderParams& params);

    virtual MOS_STATUS AllocateResources();

    virtual void FreeResources();

    virtual MOS_STATUS Initialize(CodechalSetting * settings);

    //!
    //! \brief      Execute kernel functions
    //!
    //! \return     MOS_STATUS
    //!             MOS_STATUS_SUCCESS if success, else fail reason 
    //!
    MOS_STATUS ExecuteKernelFunctions() = 0;

    //!
    //! \brief      Execute slice level
    //!
    //! \return     MOS_STATUS
    //!             MOS_STATUS_SUCCESS if success, else fail reason 
    //!
    MOS_STATUS ExecuteSliceLevel();

    virtual MOS_STATUS ExecuteDysSliceLevel();

    virtual MOS_STATUS HuCVp9Prob();

    virtual MOS_STATUS HuCBrcUpdate();

    virtual MOS_STATUS HuCBrcInitReset();

    virtual MOS_STATUS ExecutePictureLevel();

    virtual MOS_STATUS ExecuteDysPictureLevel();

    virtual MOS_STATUS SetHcpPipeBufAddrParams(MHW_VDBOX_PIPE_BUF_ADDR_PARAMS& pipeBufAddrParams,
        PMOS_SURFACE* refSurface,
        PMOS_SURFACE* refSurfaceNonScaled,
        PMOS_SURFACE* dsRefSurface4x,
        PMOS_SURFACE* dsRefSurface8x);

    virtual MOS_STATUS SetSequenceStructs();

    virtual MOS_STATUS SetPictureStructs();

    virtual MOS_STATUS SetRowstoreCachingOffsets();

    //!
    //! \brief      User feature key report
    //!
    //! \return     MOS_STATUS
    //!             MOS_STATUS_SUCCESS if success, else fail reason 
    //!
    MOS_STATUS UserFeatureKeyReport();

    //!
    //! \brief     Read Hcp status
    //!
    //! \param     [in] cmdBuffer
    //!            Pointer to command buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason 
    //!
    MOS_STATUS ReadHcpStatus(
        PMOS_COMMAND_BUFFER cmdBuffer);

    virtual MOS_STATUS ConstructPicStateBatchBuf(
        PMOS_RESOURCE picStateBuffer);

    //!
    //! \brief      Construct super frame
    //!
    //! \return     MOS_STATUS
    //!             MOS_STATUS_SUCCESS if success, else fail reason 
    //!
    MOS_STATUS ConstructSuperFrame();

    //!
    //! \brief      Set dmem HuC Vp9 Prob
    //!
    //! \return     MOS_STATUS
    //!             MOS_STATUS_SUCCESS if success, else fail reason 
    //!
    MOS_STATUS SetDmemHuCVp9Prob();

    //!
    //! \brief      Store HuC status to register 
    //!
    //! \return     MOS_STATUS
    //!             MOS_STATUS_SUCCESS if success, else fail reason 
    //!
    MOS_STATUS StoreHuCStatus2Register(PMOS_COMMAND_BUFFER cmdBuffer);

    //!
    //! \brief     Init brc constant buffer
    //!
    //! \param     [in] brcConstResource
    //!            Pointer to MOS resource
    //! \param     [in] pictureCodingType
    //!            Picture coding type
    //!
    //! \return    MOS_STATUS
    //!            MOS_STATUS_SUCCESS if success, else fail reason 
    //!
    MOS_STATUS InitBrcConstantBuffer(
        PMOS_RESOURCE brcConstResource,
        uint16_t pictureCodingType);

    //!
    //! \brief     Compute VD Encode BRC initQP
    //!
    //! \param     [in] initQpI
    //!            Init QPI
    //! \param     [in] initQpP
    //!            Init QPP
    //!
    //! \return    MOS_STATUS
    //!            MOS_STATUS_SUCCESS if success, else fail reason 
    //!
    MOS_STATUS ComputeVDEncBRCInitQP(
        int32_t* initQpI,
        int32_t* initQpP);

    //!
    //! \brief     Set dmem huc brc update
    //!
    //! \return    MOS_STATUS
    //!            MOS_STATUS_SUCCESS if success, else fail reason 
    //!
    MOS_STATUS SetDmemHuCBrcUpdate();

    //!
    //! \brief     Set dmem huc brc init reset 
    //!
    //! \return    MOS_STATUS
    //!            MOS_STATUS_SUCCESS if success, else fail reason 
    //!
    MOS_STATUS SetDmemHuCBrcInitReset();

    //!
    //! \brief     Software BRC
    //!
    //! \param     [in] update
    //!            Update status
    //!
    //! \return    MOS_STATUS
    //!            MOS_STATUS_SUCCESS if success, else fail reason 
    //!
    MOS_STATUS SoftwareBRC(bool update);

    //!
    //! \brief 
    //!
    //! \param      [in] idx
    //!             Index
    //! \param      [in] width
    //!             Width
    //! \param      [in] blockSize
    //!             Block size
    //! \param      [in] bufferPitch
    //!             Buffer pitch
    //!
    //! \return     uint32_t
    //!             Return 0if call success, else -1 if fail 
    //!
    uint32_t CalculateBufferOffset(
        uint32_t idx,
        uint32_t width,
        uint32_t blockSize,
        uint32_t bufferPitch);

    //!
    //! \brief      Pak construct picture state batch buffer
    //!
    //! \param      [in] picStateBuffer
    //!             Pointer to MOS surface
    //!
    //! \return     MOS_STATUS
    //!             MOS_STATUS_SUCCESS if success, else fail reason 
    //!
    MOS_STATUS PakConstructPicStateBatchBuf(
        PMOS_RESOURCE picStateBuffer);

    //!
    //! \brief      Return if this surface has to be compressed
    //!
    //! \param      [in] isDownScaledSurface
    //!             indicating if surface is downscaled
    //!
    //! \return     int32_t
    //!             1 if to be compressed
    //!             0 if not
    //!
    virtual bool IsToBeCompressed(bool isDownScaledSurface);

    //!
    //! \brief      Dys Reference frames
    //!
    //! \return     MOS_STATUS
    //!             MOS_STATUS_SUCCESS if success, else fail reason 
    //!
    MOS_STATUS DysRefFrames();

    //!
    //! \brief      Set sampler state Dys
    //!
    //! \param      [in] params
    //!             Pointer to Dys sampler state parameters
    //!
    //! \return     MOS_STATUS
    //!             MOS_STATUS_SUCCESS if success, else fail reason 
    //!
    MOS_STATUS SetSamplerStateDys(
        DysSamplerStateParams* params);

    //!
    //! \brief      Set curbe Dys
    //!
    //! \param      [in] params
    //!             Pointer to Dys curbe parameters
    //!
    //! \return     MOS_STATUS
    //!             MOS_STATUS_SUCCESS if success, else fail reason 
    //!
    MOS_STATUS SetCurbeDys(
        DysCurbeParams* params);

    //!
    //! \brief      Send Dys surfaces
    //!
    //! \param      [in] cmdBuffer
    //!             Pointer to MOS command buffer
    //! \param      [in] params
    //!             Pointer to Dys surface parameters
    //!
    //! \return     MOS_STATUS
    //!             MOS_STATUS_SUCCESS if success, else fail reason 
    //!
    MOS_STATUS SendDysSurfaces(
        PMOS_COMMAND_BUFFER cmdBuffer,
        DysSurfaceParams* params);

    //!
    //! \brief      Dys kernel
    //!
    //! \param      [in] dysKernelParams
    //!             Pointer to Dys kernel parameters
    //!
    //! \return     MOS_STATUS
    //!             MOS_STATUS_SUCCESS if success, else fail reason 
    //!
    virtual MOS_STATUS DysKernel(
        DysKernelParams*  dysKernelParams);

    //!
    //! \brief      Initalize ME state
    //!
    //! \param      [in] state
    //!             Pointer to Vdenc vme state
    //!
    //! \return     MOS_STATUS
    //!             MOS_STATUS_SUCCESS if success, else fail reason 
    //!
    MOS_STATUS InitMEState(VdencVmeState* state);

    //!
    //! \brief      Vdenc set curbe hme kernel
    //!
    //! \param      [in] state
    //!             Pointer to Vdenc vme state
    //!
    //! \return     MOS_STATUS
    //!             MOS_STATUS_SUCCESS if success, else fail reason 
    //!
    MOS_STATUS VdencSetCurbeHmeKernel(
        VdencVmeState* state);

    //!
    //! \brief    Sets the SurfaceStates for 16xMe and 4xME
    //! \details  Sets the Input and Output SurfaceStates for respective BTI for
    //!           16xMe and 4xME using the parameters from the input kernel state.
    //!
    //! \param    state
    //!           [in] Parameters used for setting up the CURBE
    //! \param    cmdBuffer
    //!           [in] Command buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS VdencSendHmeSurfaces(
        VdencVmeState* state,
        PMOS_COMMAND_BUFFER cmdBuffer);

    //!
    //! \brief      Vdenc hme kernel
    //!
    //! \param      [in] state
    //!             Pointer to Vdenc vme state
    //!
    //! \return     MOS_STATUS
    //!             MOS_STATUS_SUCCESS if success, else fail reason 
    //!
    MOS_STATUS VdencHmeKernel(
        VdencVmeState* state);

    virtual PMHW_VDBOX_PIPE_BUF_ADDR_PARAMS CreateHcpPipeBufAddrParams(PMHW_VDBOX_PIPE_BUF_ADDR_PARAMS pipeBufAddrParams);

    //!
    //! \brief      Set hcp ds surface params
    //!
    //! \param      [in] dsSurfaceParams
    //!             Pointer to MHW vdbox surface parameters
    //!
    //! \return     MOS_STATUS
    //!             MOS_STATUS_SUCCESS if success, else fail reason 
    //!
    void SetHcpDsSurfaceParams(MHW_VDBOX_SURFACE_PARAMS* dsSurfaceParams);

    //!
    //! \brief      Resize 4x and 8x DS recon Surfaces to VDEnc
    //!
    //! \param      [in] bufIdx
    //!             Index of the surface
    //!
    //! \return     MOS_STATUS
    //!             MOS_STATUS_SUCCESS if success, else fail reason 
    //!
    MOS_STATUS Resize4x8xforDS(
        uint8_t bufIdx);

    //!
    //! \brief      Set hcp source surface parameters
    //!
    //! \param      [in] surfaceParams
    //!             Pointer to MHW vdbox surface parameters
    //! \param      [in] refSurface
    //!             Pointer to MOS surface
    //! \param      [in] refSurfaceNonScaled
    //!             Pointer to MOS surface
    //! \param      [in] dsRefSurface4x
    //!             Pointer to MOS surface
    //! \param      [in] dsRefSurface8x
    //!             Pointer to MOS surface
    //!
    //! \return     MOS_STATUS
    //!             MOS_STATUS_SUCCESS if success, else fail reason 
    //!

    MOS_STATUS SetHcpSrcSurfaceParams(MHW_VDBOX_SURFACE_PARAMS* surfaceParams,
        PMOS_SURFACE* refSurface,
        PMOS_SURFACE* refSurfaceNonScaled,
        PMOS_SURFACE* dsRefSurface4x,
        PMOS_SURFACE* dsRefSurface8x);

    virtual MOS_STATUS GetStatusReport(
        EncodeStatus*       encodeStatus,
        EncodeStatusReport* encodeStatusReport);

    //!
    //! \brief      Get reference buffer slot index
    //!
    //! \param      [in] refreshFlags
    //!             Refresh flags
    //!
    //! \return     uint8_t
    //!             Return 0 if call success, else -1 if fail
    //!
    uint8_t GetReferenceBufferSlotIndex(uint8_t refreshFlags);

    //!
    //! \brief      Put data for compressed header
    //!
    //! \param      [in] compressedHdr
    //!             Compressed header
    //! \param      [in] bit
    //!             Bit
    //! \param      [in] prob
    //!             Prob
    //! \param      [in] binIdx
    //!             Bin index 
    //!
    void PutDataForCompressedHdr(
        CompressedHeader* compressedHdr,
        uint32_t bit,
        uint32_t prob,
        uint32_t binIdx);

    //!
    //! \brief      Allocate resources brc
    //!
    //! \return     MOS_STATUS
    //!             MOS_STATUS_SUCCESS if success, else fail reason 
    //!
    MOS_STATUS AllocateResourcesBrc();

    //!
    //! \brief      Release resources brc 
    //!
    void ReleaseResourcesBrc();

    //!
    //! \brief      Calculate temporal ratios
    //!
    //! \param      [in] numberOfLayers
    //!             Number of layers
    //! \param      [in] maxTemporalBitrate
    //!             Max temporal frame rate
    //! \param      [in] maxTemporalFrameRate
    //!             Frame rate
    //! \param      [in] maxLevelRatios
    //!             Max level ratios
    //!
    //! \return     MOS_STATUS
    //!             MOS_STATUS_SUCCESS if success, else fail reason 
    //!
    MOS_STATUS CalculateTemporalRatios(
        uint16_t numberOfLayers,
        uint32_t maxTemporalBitrate,
        FRAME_RATE maxTemporalFrameRate,
        uint8_t* maxLevelRatios);

    //!
    //! \brief      Calculate normalized denominator
    //!
    //! \param      [in] frameRates
    //!             Pointer to frame rate
    //! \param      [in] numberOfLayers
    //!             Number of layers
    //! \param      [in] normalizedDenominator
    //!             Normalized denominator
    //!
    //! \return     uint32_t
    //!             Return 0 if call success, else -1 if fail
    //!
    uint32_t CalculateNormalizedDenominator(
        FRAME_RATE* frameRates,
        uint16_t numberOfLayers,
        uint32_t normalizedDenominator);

    //!
    //! \brief    Calculate rePak thresholds
    //! \details
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS CalculateRePakThresholds();

    //!
    //! \brief    Construct Pak insert object batch buf
    //! \details
    //!
    //! \param    [in] pakInsertObjBuffer
    //!           Pointer to MOS resource
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ConstructPakInsertObjBatchBuf(
        PMOS_RESOURCE pakInsertObjBuffer);

    //!
    //! \brief     Refresh frame internal buffers
    //! \details
    //!
    //! \return    MOS_STATUS
    //!            MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS RefreshFrameInternalBuffers();

    //!
    //! \brief    Allocate Mb brc segment map surface
    //! \details
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateMbBrcSegMapSurface();

    //!
    //! \brief    Init context buffer 
    //! \details
    //! \param    [in,out] ctxBuffer 
    //!           Pointer to context buffer 
    //!
    //! \param    [in] setToKey 
    //!           Specify if it's key frame 
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ContextBufferInit(
            uint8_t *ctxBuffer,
            bool setToKey);

    //!
    //! \brief    Populate prob values which are different between Key and Non-Key frame 
    //! \details
    //! \param    [in,out] ctxBuffer 
    //!           Pointer to context buffer 
    //!
    //! \param    [in] setToKey 
    //!           Specify if it's key frame 
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS CtxBufDiffInit(
            uint8_t *ctxBuffer,
            bool setToKey);

    //!
    //! \brief    Calculate Vdenc Picture State CommandSize 
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS CalculateVdencPictureStateCommandSize();

    //!
    //! \brief    Initialize MMC state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS InitMmcState();

    virtual MOS_STATUS VerifyCommandBufferSize();

    virtual MOS_STATUS GetCommandBuffer(PMOS_COMMAND_BUFFER cmdBuffer);

    virtual MOS_STATUS ReturnCommandBuffer(PMOS_COMMAND_BUFFER cmdBuffer);

    virtual MOS_STATUS SubmitCommandBuffer(
        PMOS_COMMAND_BUFFER cmdBuffer,
        bool nullRendering);

#if USE_CODECHAL_DEBUG_TOOL
    MOS_STATUS DumpSegmentParams(
        PCODEC_VP9_ENCODE_SEGMENT_PARAMS segmentParams);

    MOS_STATUS DumpSeqParams(
        PCODEC_VP9_ENCODE_SEQUENCE_PARAMS seqParams);

    MOS_STATUS DumpPicParams(
        PCODEC_VP9_ENCODE_PIC_PARAMS picParams);
#endif
};

#endif  // __CODECHAL_VDENC_VP9_BASE_H__

