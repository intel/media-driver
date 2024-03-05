/*
* Copyright (c) 2018, Intel Corporation
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
//! \file     codechal_encode_vp8_g9.cpp
//! \brief    VP8 dual-pipe encoder for GEN9.
//!

#include "codechal_encode_vp8_g9.h"
#include "codeckrnheader.h"
#ifndef _FULL_OPEN_SOURCE
#include "igcodeckrn_g9.h"
#endif
#include "mhw_vdbox_mfx_hwcmd_g9_kbl.h"

#define     INTRA_PROBABILIY                 63
#define     INTER_LAST_PROBABILIY            255
#define     INTER_GOLD_PROBABILIY            128
#define     NAX_NUM_TEMPORAL_LAYERS          4

enum CodechalBindingTableOffsetVp8BrcInitResetG9
{
    CODECHAL_VP8_BRC_INIT_RESET_HISTORY_G9       = 0,
    CODECHAL_VP8_BRC_INIT_RESET_DISTORTION_G9      = 1,
    CODECHAL_VP8_BRC_INIT_RESET_NUM_SURFACES_G9    = 2
};

enum CodechalBindingTableOffsetVp8BrcUpdateG9
{
    CODECHAL_VP8_BRC_UPDATE_HISTORY_G9                = 1,
    CODECHAL_VP8_BRC_UPDATE_PAK_STATISTICS_OUTPUT_G9  = 2,
    CODECHAL_VP8_BRC_UPDATE_MFX_ENCODER_CFG_READ_G9   = 3,
    CODECHAL_VP8_BRC_UPDATE_MFX_ENCODER_CFG_WRITE_G9  = 4,
    CODECHAL_VP8_BRC_UPDATE_MBENC_CURBE_READ_G9       = 5,
    CODECHAL_VP8_BRC_UPDATE_MBENC_CURBE_WRITE_G9      = 6,
    CODECHAL_VP8_BRC_UPDATE_DISTORTION_SURFACE_G9     = 7,
    CODECHAL_VP8_BRC_UPDATE_CONSTANT_DATA_G9          = 8,
    CODECHAL_VP8_BRC_UPDATE_SEGMENT_MAP_G9            = 9,
    CODECHAL_VP8_BRC_UPDATE_MPU_CURBE_READ_G9         = 10,
    CODECHAL_VP8_BRC_UPDATE_MPU_CURBE_WRITE_G9        = 11,
    CODECHAL_VP8_BRC_UPDATE_TPU_CURBE_READ_G9         = 12,
    CODECHAL_VP8_BRC_UPDATE_TPU_CURBE_WRITE_G9        = 13,
    CODECHAL_VP8_BRC_UPDATE_NUM_SURFACES_G9           = 14
};

enum CodechalBindingTableOffsetVp8MeG9
{
    CODECHAL_VP8_ME_MV_DATA_G9                = 0,
    CODECHAL_VP8_16xME_MV_DATA_G9             = 2,
    CODECHAL_VP8_ME_DISTORTION_G9             = 3,
    CODECHAL_VP8_ME_MIN_DIST_BRC_DATA_G9      = 4,
    CODECHAL_VP8_VME_INTER_PRED_G9            = 5,
    CODECHAL_VP8_ME_REF1_PIC_G9               = 6,
    CODECHAL_VP8_ME_REF2_PIC_G9               = 8,
    CODECHAL_VP8_ME_REF3_PIC_G9               = 10,
    CODECHAL_VP8_ME_NUM_SURFACES_G9           = 11
};

enum CodechalBindingTableOffsetVp8MbencG9
{
    CODECHAL_VP8_MBENC_PER_MB_OUT_G9          = 0,
    CODECHAL_VP8_MBENC_CURR_Y_G9              = 1,
    CODECHAL_VP8_MBENC_CURR_UV_G9             = 2,
};

enum CodechalBindingTableOffsetVp8MbencIFrameG9
{
    CODECHAL_VP8_MBENC_MB_MODE_COST_LUMA_G9      = 3,
    CODECHAL_VP8_MBENC_BLOCK_MODE_COST_G9        = 4,
    CODECHAL_VP8_MBENC_CHROMA_RECON_G9           = 5,
    CODECHAL_VP8_MBENC_SEGMENTATION_MAP_G9       = 6,
    CODECHAL_VP8_MBENC_HISTOGRAM_G9              = 7,
    CODECHAL_VP8_MBENC_I_VME_DEBUG_STREAMOUT_G9  = 8,
    CODECHAL_VP8_MBENC_VME_G9                    = 9,
    CODECHAL_VP8_MBENC_IDIST_G9                  = 10,
    CODECHAL_VP8_MBENC_CURR_Y_DOWNSCALED_G9      = 11,
    CODECHAL_VP8_MBENC_VME_Coarse_Intra_G9       = 12
};

enum CodechalBindingTableOffsetVp8MbencPFrameG9
{
    CODECHAL_VP8_MBENC_MV_DATA_FROM_ME_G9         = 3,
    CODECHAL_VP8_MBENC_IND_MV_DATA_G9             = 4,
    CODECHAL_VP8_MBENC_REF_MB_COUNT_G9            = 5,
    CODECHAL_VP8_MBENC_INTER_PRED_G9              = 8,
    CODECHAL_VP8_MBENC_REF1_PIC_G9                = 9,
    CODECHAL_VP8_MBENC_REF2_PIC_G9                = 11,
    CODECHAL_VP8_MBENC_REF3_PIC_G9                = 13,
    CODECHAL_VP8_MBENC_P_PER_MB_QUANT_G9          = 14,
    CODECHAL_VP8_MBEBC_INTER_PRED_DISTORTION_G9   = 15,
    CODECHAL_VP8_MBEBC_PER_MV_DATA_G9             = 16,
    CODECHAL_VP8_MBENC_MODE_COST_UPDATE_G9        = 17,
    CODECHAL_VP8_MBENC_P_VME_DEBUG_STREAMOUT_G9   = 18,
    CODECHAL_VP8_MBENC_NUM_SURFACES_G9            = 19
};

enum CodechalBindingTableOffsetVp8MpuFhbG9
{
    CODECHAL_VP8_MPU_FHB_HISTOGRAM_G9                 = 0,
    CODECHAL_VP8_MPU_FHB_REF_MODE_PROBABILITY_G9      = 1,
    CODECHAL_VP8_MPU_FHB_CURR_MODE_PROBABILITY_G9     = 2,
    CODECHAL_VP8_MPU_FHB_REF_TOKEN_PROBABILITY_G9     = 3,
    CODECHAL_VP8_MPU_FHB_CURR_TOKEN_PROBABILITY_G9    = 4,
    CODECHAL_VP8_MPU_FHB_HEADER_BITSTREAM_G9          = 5,
    CODECHAL_VP8_MPU_FHB_HEADER_METADATA_G9           = 6,
    CODECHAL_VP8_MPU_FHB_PICTURE_STATE_G9             = 7,
    CODECHAL_VP8_MPU_FHB_MPU_BITSTREAM_G9             = 8,
    CODECHAL_VP8_MPU_FHB_TOKEN_BITS_DATA_TABLE_G9     = 9,
    CODECHAL_VP8_MPU_FHB_VME_DEBUG_STREAMOUT_G9       = 10,
    CODECHAL_VP8_MPU_FHB_ENTROPY_COST_TABLE_G9        = 11,
    CODECHAL_VP8_MPU_MODE_COST_UPDATE_G9              = 12,
    CODECHAL_VP8_MPU_FHB_NUM_SURFACES_G9              = 13
};

enum CodechalBindingTableOffsetVp8TpuFhbG9
{
    CODECHAL_VP8_TPU_FHB_PAK_TOKEN_STATISTICS_G9      = 0,
    CODECHAL_VP8_TPU_FHB_TOKEN_UPDATE_FLAGS_G9        = 1,
    CODECHAL_VP8_TPU_FHB_ENTROPY_COST_TABLE_G9        = 2,
    CODECHAL_VP8_TPU_FHB_HEADER_BITSTREAM_G9          = 3,
    CODECHAL_VP8_TPU_FHB_DEFAULT_TOKEN_PROBABILITY_G9 = 4,
    CODECHAL_VP8_TPU_FHB_PICTURE_STATE_G9             = 5,
    CODECHAL_VP8_TPU_FHB_MPU_CURBE_DATA_G9            = 6,
    CODECHAL_VP8_TPU_FHB_HEADER_METADATA_G9           = 7,
    CODECHAL_VP8_TPU_FHB_TOKEN_PROBABILITY_G9         = 8,
    CODECHAL_VP8_TPU_FHB_PAK_HW_PASS1_PROBABILITY_G9  = 9,
    CODECHAL_VP8_TPU_FHB_KEY_TOKEN_PROBABILITY_G9     = 10,
    CODECHAL_VP8_TPU_FHB_UPDATED_TOKEN_PROBABILITY_G9 = 11,
    CODECHAL_VP8_TPU_FHB_PAK_HW_PASS2_PROBABILITY_G9  = 12,
    CODECHAL_VP8_TPU_FHB_VME_DEBUG_STREAMOUT_G9       = 13,
    CODECHAL_VP8_TPU_FHB_REPAK_DECISION_G9            = 14,
    CODECHAL_VP8_TPU_FHB_NUM_SURFACES_G9              = 15
};

struct MediaObjectVp8BrcInitResetStaticDataG9
{
    union
    {
        struct
        {
            uint32_t   ProfileLevelMaxFrame           : 32;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW0;

    union
    {
        struct
        {
            uint32_t   InitBufFullInBits              : 32;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW1;

    union
    {
        struct
        {
            uint32_t   BufSizeInBits                 : 32;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW2;

    union
    {
        struct
        {
            uint32_t   AverageBitRate                 : 32;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW3;

    union
    {
        struct
        {
            uint32_t   MaxBitRate                     : 32;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW4;

    union
    {
        struct
        {
            uint32_t   MinBitRate                     : 32;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW5;

    union
    {
        struct
        {
            uint32_t   FrameRateM                     : 32;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW6;

    union
    {
        struct
        {
            uint32_t   FrameRateD                     : 32;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW7;

    union
    {
        struct
        {
            uint32_t   BRCFlag                        : 16;
            uint32_t   GopP                           : 16;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW8;

    union
    {
        struct
        {
            uint32_t   Reserved                        : 16;
            uint32_t   FrameWidthInBytes               : 16;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW9;

    union
    {
        struct
        {
            uint32_t   FrameHeightInBytes             : 16;
            uint32_t   AVBRAccuracy                   : 16;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW10;

    union
    {
        struct
        {
            uint32_t   AVBRConvergence                : 16;
            uint32_t   MinQP                          : 16;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW11;

    union
    {
        struct
        {
            uint32_t   MaxQP                          : 16;
            uint32_t   LevelQP                       : 16;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW12;

    union
    {
        struct
        {
            uint32_t   MaxSection_pct                : 16;
            uint32_t   OverShootCBR_pct              : 16;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW13;

    union
    {
        struct
        {
            uint32_t   VBRBias_pct                   : 16;
            uint32_t   MinSection_pct                : 16;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW14;

    union
    {
        struct
        {
            uint32_t   InstantRateThreshold0ForP      : 8;
            uint32_t   InstantRateThreshold1ForP      : 8;
            uint32_t   InstantRateThreshold2ForP      : 8;
            uint32_t   InstantRateThreshold3ForP      : 8;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW15;

    union
    {
        struct
        {
            uint32_t   Reserved                       : 32;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW16;

    union
    {
        struct
        {
            uint32_t   InstantRateThreshold0ForI      : 8;
            uint32_t   InstantRateThreshold1ForI      : 8;
            uint32_t   InstantRateThreshold2ForI      : 8;
            uint32_t   InstantRateThreshold3ForI      : 8;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW17;

    union
    {
        struct
        {
            uint32_t   DeviationThreshold0ForP         : 8;     // Signed byte
            uint32_t   DeviationThreshold1ForP         : 8;     // Signed byte
            uint32_t   DeviationThreshold2ForP        : 8;     // Signed byte
            uint32_t   DeviationThreshold3ForP        : 8;     // Signed byte
        };
        struct
        {
            uint32_t   Value;
        };
    } DW18;

    union
    {
        struct
        {
            uint32_t   DeviationThreshold4ForP          : 8;     // Signed byte
            uint32_t   DeviationThreshold5ForP          : 8;     // Signed byte
            uint32_t   DeviationThreshold6ForP          : 8;     // Signed byte
            uint32_t   DeviationThreshold7ForP          : 8;     // Signed byte
        };
        struct
        {
            uint32_t   Value;
        };
    } DW19;

    union
    {
        struct
        {
            uint32_t   DeviationThreshold0ForVBR      : 8;     // Signed byte
            uint32_t   DeviationThreshold1ForVBR      : 8;     // Signed byte
            uint32_t   DeviationThreshold2ForVBR      : 8;     // Signed byte
            uint32_t   DeviationThreshold3ForVBR      : 8;     // Signed byte
        };
        struct
        {
            uint32_t   Value;
        };
    } DW20;

    union
    {
        struct
        {
            uint32_t   DeviationThreshold4ForVBR      : 8;     // Signed byte
            uint32_t   DeviationThreshold5ForVBR      : 8;     // Signed byte
            uint32_t   DeviationThreshold6ForVBR      : 8;     // Signed byte
            uint32_t   DeviationThreshold7ForVBR      : 8;     // Signed byte
        };
        struct
        {
            uint32_t   Value;
        };
    } DW21;

    union
    {
        struct
        {
            uint32_t   DeviationThreshold0ForI        : 8;     // Signed byte
            uint32_t   DeviationThreshold1ForI        : 8;     // Signed byte
            uint32_t   DeviationThreshold2ForI        : 8;     // Signed byte
            uint32_t   DeviationThreshold3ForI        : 8;     // Signed byte
        };
        struct
        {
            uint32_t   Value;
        };
    } DW22;

    union
    {
        struct
        {
            uint32_t   DeviationThreshold4ForI        : 8;     // Signed byte
            uint32_t   DeviationThreshold5ForI        : 8;     // Signed byte
            uint32_t   DeviationThreshold6ForI        : 8;     // Signed byte
            uint32_t   DeviationThreshold7ForI        : 8;     // Signed byte
        };
        struct
        {
            uint32_t   Value;
        };
    } DW23;

    union
    {
        struct
        {
            uint32_t   NumTLevels                          : 8;
            uint32_t   INITBCK_MaxLevel_Ratio_U8_Layer0    : 8;
            uint32_t   INITBCK_MaxLevel_Ratio_U8_Layer1    : 8;
            uint32_t   INITBCK_MaxLevel_Ratio_U8_Layer2    : 8;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW24;

    union
    {
        struct
        {
            uint32_t   INITBCK_MaxLevel_Ratio_U8_Layer3    : 8;
            uint32_t   Reserved                            : 24;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW25;

    union
    {
        struct
        {
            uint32_t   HistoryBufferBTI             : 32;     // Signed byte
        };
        struct
        {
            uint32_t   Value;
        };
    } DW26;

    union
    {
        struct
        {
            uint32_t   DistortionBufferBTI              : 32;     // Signed byte
        };
        struct
        {
            uint32_t   Value;
        };
    } DW27;

};

struct MediaObjectVp8BrcUpdateStaticDataG9
{
    union
    {
        struct
        {
            uint32_t   TargetSize                     : 32;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW0;

    union
    {
        struct
        {
            uint32_t   FrameNumber                    : 32;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW1;

    union
    {
        struct
        {
            uint32_t   PictureHeaderSize              : 32;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW2;

    union
    {
        struct
        {
            uint32_t   startGAdjFrame0                : 16;
            uint32_t   startGAdjFrame1                : 16;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW3;

    union
    {
        struct
        {
            uint32_t   startGAdjFrame2                : 16;
            uint32_t   startGAdjFrame3                : 16;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW4;

    union
    {
        struct
        {
            uint32_t   TargetSizeFlag                 : 8;
            uint32_t   BRCFlag                        : 8;
            uint32_t   MaxNumPAKs                     : 8;
            uint32_t   CurrFrameType                  : 8;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW5;

    // This offset indicates the byte position of the q_scale_type bit
    // in the 2nd level batch buffer containing the INSERT_OBJ command
    // for inserting the picture header data into the bitstream.
    // This offset includes the 8 bytes of the INSERT command at the
    // beginning of the buffer.
    // Similarly for the VbvDelay field.
    union
    {
        struct
        {
            uint32_t TID                                           : 8;
            uint32_t NumTLevels                                    : 8;
            uint32_t Reserved                                      : 16;
        };
        struct
        {
            uint32_t Value;
        };
    } DW6;

    // This size is the size of the entire 2nd level batch buffer
    // containing the INSERT_OBJ command for inserting the
    // picture header data into the bitstream. It includes the batch buffer end
    // command at the end of the buffer.
    union
    {
        struct
        {
            uint32_t Reserved0         : 32;
        };
        struct
        {
            uint32_t Value;
        };

    } DW7;

    union
    {
        struct
        {
            uint32_t   StartGlobalAdjustMult0         : 8;
            uint32_t   StartGlobalAdjustMult1         : 8;
            uint32_t   StartGlobalAdjustMult2         : 8;
            uint32_t   StartGlobalAdjustMult3         : 8;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW8;

    union
    {
        struct
        {
            uint32_t   StartGlobalAdjustMult4        : 8;
            uint32_t   StartGlobalAdjustDiv0         : 8;
            uint32_t   StartGlobalAdjustDiv1         : 8;
            uint32_t   StartGlobalAdjustDiv2         : 8;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW9;

    union
    {
        struct
        {
            uint32_t   StartGlobalAdjustDiv3         : 8;
            uint32_t   StartGlobalAdjustDiv4         : 8;
            uint32_t   QPThreshold0                  : 8;
            uint32_t   QPThreshold1                   : 8;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW10;

    union
    {
        struct
        {
            uint32_t   QPThreshold2                  : 8;
            uint32_t   QPThreshold3                  : 8;
            uint32_t   gRateRatioThreshold0          : 8;
            uint32_t   gRateRatioThreshold1          : 8;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW11;

    union
    {
        struct
        {
            uint32_t   gRateRatioThreshold2          : 8;
            uint32_t   gRateRatioThreshold3          : 8;
            uint32_t   gRateRatioThreshold4          : 8;
            uint32_t   gRateRatioThreshold5          : 8;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW12;

    union
    {
        struct
        {
            uint32_t   gRateRatioThresholdQP0        : 8;
            uint32_t   gRateRatioThresholdQP1        : 8;
            uint32_t   gRateRatioThresholdQP2        : 8;
            uint32_t   gRateRatioThresholdQP3        : 8;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW13;

    union
    {
        struct
        {
            uint32_t   gRateRatioThresholdQP4        : 8;
            uint32_t   gRateRatioThresholdQP5        : 8;
            uint32_t   gRateRatioThresholdQP6        : 8;
            uint32_t   IndexOfPreviousQP             : 8;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW14;

    union
    {
        struct
        {
            uint32_t FrameWidthInMB                                : 16;
            uint32_t FrameHeightInMB                               : 16;
        };
        struct
        {
            uint32_t Value;
        };
    } DW15;

    union
    {
        struct
        {
            uint32_t PFrameQPSeg0                     : 8;
            uint32_t PFrameQPSeg1                     : 8;
            uint32_t PFrameQPSeg2                     : 8;
            uint32_t PFrameQPSeg3                     : 8;
        };
        struct
        {
            uint32_t Value;
        };
    } DW16;

    union
    {
        struct
        {
            uint32_t KeyFrameQPSeg0               : 8;
            uint32_t KeyFrameQPSeg1               : 8;
            uint32_t KeyFrameQPSeg2               : 8;
            uint32_t KeyFrameQPSeg3               : 8;
        };
        struct
        {
            uint32_t Value;
        };
    } DW17;

    union
    {
        struct
        {
            uint32_t QDeltaPlane0                     : 8;
            uint32_t QDeltaPlane1                     : 8;
            uint32_t QDeltaPlane2                     : 8;
            uint32_t QDeltaPlane3                     : 8;
        };
        struct
        {
            uint32_t Value;
        };
    } DW18;

    union
    {
        struct
        {
            uint32_t QDeltaPlane4  : 8;
            uint32_t QIndex        : 8;
            uint32_t MainRef       : 8;
            uint32_t RefFrameFlags : 8;
        };
        struct
        {
            uint32_t Value;
        };
    } DW19;

    union
    {
        struct
        {
            uint32_t SegOn                            : 8;
            uint32_t MBRC                             : 8;
            uint32_t BRCMethod                        : 8;
            uint32_t VMEIntraPrediction           : 8;
        };
        struct
        {
            uint32_t Value;
        };
    } DW20;

    union
    {
        struct
        {
            uint32_t CurrentFrameQPIndex               : 8;
            uint32_t LastFrameQPIndex                  : 8;
            uint32_t GoldFrameQPIndex                  : 8;
            uint32_t AltFrameQPIndex                   : 8;
        };
        struct
        {
            uint32_t Value;
        };
    } DW21;

    union
    {
        struct
        {
            uint32_t HistorytBufferBTI                  : 32;
        };
        struct
        {
            uint32_t Value;
        };
    } DW22;

    union
    {
        struct
        {
            uint32_t PakStatisticsBTI                   : 32;
        };
        struct
        {
            uint32_t Value;
        };
    } DW23;

    union
    {
        struct
        {
            uint32_t MfxVp8EncoderCfgReadBTI            :32;
        };
        struct
        {
            uint32_t Value;
        };
    } DW24;

    union
    {
        struct
        {
            uint32_t MfxVp8EncoderCfgWriteBTI                   : 32;
        };
        struct
        {
            uint32_t Value;
        };
    } DW25;

    union
    {
        struct
        {
            uint32_t MBEncCurbeReadBTI                  : 32;
        };
        struct
        {
            uint32_t Value;
        };
    } DW26;

    union
    {
        struct
        {
            uint32_t MBEncCurbeWriteBTI                 : 32;
        };
        struct
        {
            uint32_t Value;
        };
    } DW27;

    union
    {
        struct
        {
            uint32_t DistortionBTI                  : 32;
        };
        struct
        {
            uint32_t Value;
        };
    } DW28;

    union
    {
        struct
        {
            uint32_t ConstantDataBTI                    : 32;
        };
        struct
        {
            uint32_t Value;
        };
    } DW29;

    union
    {
        struct
        {
            uint32_t SegmentMapBTI                  : 32;
        };
        struct
        {
            uint32_t Value;
        };
    } DW30;

    union
    {
        struct
        {
            uint32_t MpuCurbeReadBTI                    : 32;
        };
        struct
        {
            uint32_t Value;
        };
    } DW31;

    union
    {
        struct
        {
            uint32_t MpuCurbeWriteBTI                   : 32;
        };
        struct
        {
            uint32_t Value;
        };
    } DW32;

    union
    {
        struct
        {
            uint32_t TpuCurbeReadBTI                    : 32;
        };
        struct
        {
            uint32_t Value;
        };
    } DW33;

    union
    {
        struct
        {
            uint32_t TpuCurbeWriteBTI                   : 32;
        };
        struct
        {
            uint32_t Value;
        };
    } DW34;

};

struct MediaObjectVp8MeStaticDataG9
{
    // DW0
    union
    {
        struct
        {
            uint32_t   SkipModeEn              : MOS_BITFIELD_BIT(0);
            uint32_t   AdaptiveEn              : MOS_BITFIELD_BIT(1);
            uint32_t   BiMixDis                : MOS_BITFIELD_BIT(2);
            uint32_t   Reserved0               : MOS_BITFIELD_RANGE(3, 4);
            uint32_t   EarlyImeSuccessEn       : MOS_BITFIELD_BIT(5);
            uint32_t   Reserved1               : MOS_BITFIELD_BIT(6);
            uint32_t   T8x8FlagForInterEn      : MOS_BITFIELD_BIT(7);
            uint32_t   Reserved2               : MOS_BITFIELD_RANGE(8,23);
            uint32_t   EarlyImeStop            : MOS_BITFIELD_RANGE(24,31);
        };
        struct
        {
            uint32_t       Value;
        };
    } DW0;

    // DW1
    union
    {
        struct
        {
            uint32_t   MaxNumMVs               : MOS_BITFIELD_RANGE(0,5);
            uint32_t   Reserved0               : MOS_BITFIELD_RANGE(6,15);
            uint32_t   BiWeight                : MOS_BITFIELD_RANGE(16,21);
            uint32_t   Reserved1               : MOS_BITFIELD_RANGE(22,27);
            uint32_t   UniMixDisable           : MOS_BITFIELD_BIT(28);
            uint32_t   Reserved2               : MOS_BITFIELD_RANGE(29,31);
        };
        struct
        {
            uint32_t       Value;
        };
    } DW1;

    // DW2
    union
    {
        struct
        {
            uint32_t   MaxLenSP                : MOS_BITFIELD_RANGE(0,7);
            uint32_t   MaxNumSU                : MOS_BITFIELD_RANGE(8,15);
            uint32_t   Reserved0               : MOS_BITFIELD_RANGE(16,31);
        };
        struct
        {
            uint32_t       Value;
        };
    } DW2;

    // DW3
    union
    {
        struct
        {
            uint32_t   SrcSize                 : MOS_BITFIELD_RANGE(0,1);
            uint32_t   Reserved0               : MOS_BITFIELD_RANGE(2,3);
            uint32_t   MbTypeRemap             : MOS_BITFIELD_RANGE(4,5);
            uint32_t   SrcAccess               : MOS_BITFIELD_BIT(6);
            uint32_t   RefAccess               : MOS_BITFIELD_BIT(7);
            uint32_t   SearchCtrl              : MOS_BITFIELD_RANGE(8,10);
            uint32_t   DualSearchPathOption    : MOS_BITFIELD_BIT(11);
            uint32_t   SubPelMode              : MOS_BITFIELD_RANGE(12,13);
            uint32_t   SkipType                : MOS_BITFIELD_BIT(14);
            uint32_t   DisableFieldCacheAlloc  : MOS_BITFIELD_BIT(15);
            uint32_t   InterChromaMode         : MOS_BITFIELD_BIT(16);
            uint32_t   FTEnable                : MOS_BITFIELD_BIT(17);
            uint32_t   BMEDisableFBR           : MOS_BITFIELD_BIT(18);
            uint32_t   BlockBasedSkipEnable    : MOS_BITFIELD_BIT(19);
            uint32_t   InterSAD                : MOS_BITFIELD_RANGE(20,21);
            uint32_t   IntraSAD                : MOS_BITFIELD_RANGE(22,23);
            uint32_t   SubMbPartMask           : MOS_BITFIELD_RANGE(24,30);
            uint32_t   Reserved1               : MOS_BITFIELD_BIT(31);
        };
        struct
        {
            uint32_t       Value;
        };
    } DW3;

    // DW4
    union
    {
        struct
        {
            uint32_t   Reserved0               : MOS_BITFIELD_RANGE(0,7);
            uint32_t   PictureHeightMinus1     : MOS_BITFIELD_RANGE(8,15);
            uint32_t   PictureWidth            : MOS_BITFIELD_RANGE(16,23);
            uint32_t   Reserved1               : MOS_BITFIELD_RANGE(24,31);
        };
        struct
        {
            uint32_t       Value;
        };
    } DW4;

    // DW5
    union
    {
        struct
        {
            uint32_t   Reserved0               : MOS_BITFIELD_RANGE(0,7);
            uint32_t   QpPrimeY                : MOS_BITFIELD_RANGE(8,15);
            uint32_t   RefWidth                : MOS_BITFIELD_RANGE(16,23);
            uint32_t   RefHeight               : MOS_BITFIELD_RANGE(24,31);

        };
        struct
        {
            uint32_t       Value;
        };
    } DW5;

    // DW6
    union
    {
        struct
        {
            uint32_t   Reserved0               : MOS_BITFIELD_RANGE(0,2);
            uint32_t   MEModes                 : MOS_BITFIELD_RANGE(3,4);
            uint32_t   Reserved1               : MOS_BITFIELD_RANGE(5,7);
            uint32_t   SuperCombineDist        : MOS_BITFIELD_RANGE(8,15);
            uint32_t   MaxVmvR                 : MOS_BITFIELD_RANGE(16,31);
        };
        struct
        {
            uint32_t       Value;
        };
    } DW6;

    // DW7
    union
    {
        struct
        {
            uint32_t   Reserved0               : MOS_BITFIELD_RANGE(0,15);
            uint32_t   MVCostScaleFactor       : MOS_BITFIELD_RANGE(16,17);
            uint32_t   BilinearEnable          : MOS_BITFIELD_BIT(18);
            uint32_t   SrcFieldPolarity        : MOS_BITFIELD_BIT(19);
            uint32_t   WeightedSADHAAR         : MOS_BITFIELD_BIT(20);
            uint32_t   AConlyHAAR              : MOS_BITFIELD_BIT(21);
            uint32_t   RefIDCostMode           : MOS_BITFIELD_BIT(22);
            uint32_t   Reserved1               : MOS_BITFIELD_BIT(23);
            uint32_t   SkipCenterMask          : MOS_BITFIELD_RANGE(24,31);
        };
        struct
        {
            uint32_t       Value;
        };
    } DW7;

    // DW8
    union
    {
        struct
        {
            uint32_t   Mode0Cost               : MOS_BITFIELD_RANGE(0,7);
            uint32_t   Mode1Cost               : MOS_BITFIELD_RANGE(8,15);
            uint32_t   Mode2Cost               : MOS_BITFIELD_RANGE(16,23);
            uint32_t   Mode3Cost               : MOS_BITFIELD_RANGE(24,31);
        };
        struct
        {
            uint32_t       Value;
        };
    } DW8;

    // DW9
    union
    {
        struct
        {
            uint32_t   Mode4Cost               : MOS_BITFIELD_RANGE(0,7);
            uint32_t   Mode5Cost               : MOS_BITFIELD_RANGE(8,15);
            uint32_t   Mode6Cost               : MOS_BITFIELD_RANGE(16,23);
            uint32_t   Mode7Cost               : MOS_BITFIELD_RANGE(24,31);
        };
        struct
        {
            uint32_t       Value;
        };
    } DW9;

    // DW10
    union
    {
        struct
        {
            uint32_t   Mode8Cost               : MOS_BITFIELD_RANGE(0,7);
            uint32_t   Mode9Cost               : MOS_BITFIELD_RANGE(8,15);
            uint32_t   RefIDCost               : MOS_BITFIELD_RANGE(16,23);
            uint32_t   ChromaIntraModeCost     : MOS_BITFIELD_RANGE(24,31);
        };
        struct
        {
            uint32_t       Value;
        };
    } DW10;

    // DW11
    union
    {
        struct
        {
            uint32_t   MV0Cost                 : MOS_BITFIELD_RANGE(0,7);
            uint32_t   MV1Cost                 : MOS_BITFIELD_RANGE(8,15);
            uint32_t   MV2Cost                 : MOS_BITFIELD_RANGE(16,23);
            uint32_t   MV3Cost                 : MOS_BITFIELD_RANGE(24,31);
        };
        struct
        {
            uint32_t       Value;
        };
    } DW11;

    // DW12
    union
    {
        struct
        {
            uint32_t   MV4Cost                 : MOS_BITFIELD_RANGE(0,7);
            uint32_t   MV5Cost                 : MOS_BITFIELD_RANGE(8,15);
            uint32_t   MV6Cost                 : MOS_BITFIELD_RANGE(16,23);
            uint32_t   MV7Cost                 : MOS_BITFIELD_RANGE(24,31);
        };
        struct
        {
            uint32_t       Value;
        };
    } DW12;

    // DW13
    union
    {
        struct
        {
            uint32_t   NumRefIdxL0MinusOne     : MOS_BITFIELD_RANGE(0,7);
            uint32_t   NumRefIdxL1MinusOne     : MOS_BITFIELD_RANGE(8,15);
            uint32_t   ActualMBWidth           : MOS_BITFIELD_RANGE(16,23);
            uint32_t   ActualMBHeight          : MOS_BITFIELD_RANGE(24,31);
        };
        struct
        {
            uint32_t       Value;
        };
    } DW13;

    // DW14
    union
    {
        struct
        {
            uint32_t   L0RefPicPolarityBits    : MOS_BITFIELD_RANGE(0,7);
            uint32_t   L1RefPicPolarityBits    : MOS_BITFIELD_RANGE(8,9);
            uint32_t   Reserved                : MOS_BITFIELD_RANGE(10,31);
        };
        struct
        {
            uint32_t       Value;
        };
    } DW14;

    // DW15
    union
    {
        struct
        {
            uint32_t   Reserved                : MOS_BITFIELD_RANGE(0,31);
        };
        struct
        {
            uint32_t       Value;
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
                uint32_t       Value;
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
                uint32_t       Value;
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
                uint32_t       Value;
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
                uint32_t       Value;
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
                uint32_t       Value;
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
                uint32_t       Value;
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
                uint32_t       Value;
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
                uint32_t       Value;
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
                uint32_t       Value;
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
                uint32_t       Value;
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
                uint32_t       Value;
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
                uint32_t       Value;
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
                uint32_t       Value;
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
                uint32_t       Value;
            };
        } DW29;
    } SpDelta;

    // DW30
    union
    {
        struct
        {
            uint32_t   Reserved0  : MOS_BITFIELD_RANGE(0,31);
        };
        struct
        {
            uint32_t       Value;
        };
    } DW30;

    // DW31
    union
    {
        struct
        {
            uint32_t   Reserved0  : MOS_BITFIELD_RANGE(0,31);
        };
        struct
        {
            uint32_t       Value;
        };
    } DW31;

    // DW32
    union
    {
        struct
        {
            uint32_t   VP8MeMVOutputDataBTI        : MOS_BITFIELD_RANGE(0,31);
        };
        struct
        {
            uint32_t       Value;
        };
    } DW32;

    // DW33
    union
    {
        struct
        {
            uint32_t   VP8MeMVInputDataBTI         : MOS_BITFIELD_RANGE(0,31);
        };
        struct
        {
            uint32_t       Value;
        };
    } DW33;

    // DW34
    union
    {
        struct
        {
            uint32_t   VP8MeDistortionBTI          : MOS_BITFIELD_RANGE(0,31);
        };
        struct
        {
            uint32_t       Value;
        };
    } DW34;

    // DW35
    union
    {
        struct
        {
            uint32_t   VP8MeMinDistBrcBTI          : MOS_BITFIELD_RANGE(0,31);
        };
        struct
        {
            uint32_t       Value;
        };
    } DW35;

    // DW36
    union
    {
        struct
        {
            uint32_t   ForwardRefBTI               : MOS_BITFIELD_RANGE(0,31);
        };
        struct
        {
            uint32_t       Value;
        };
    } DW36;

    // DW37
    union
    {
        struct
        {
            uint32_t   BackwardRefBTI              : MOS_BITFIELD_RANGE(0,31);
        };
        struct
        {
            uint32_t       Value;
        };
    } DW37;
};
C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(struct MediaObjectVp8MeStaticDataG9)) == 38);

struct MediaObjectVp8MbencIStaticDataG9
{
    // DW0
    union
    {
        struct
        {
            uint32_t   FrameWidth                : 16;
            uint32_t   FrameHeight               : 16;
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
            uint32_t   FrameType                       : 1;
            uint32_t   EnableSegmentation              : 1;
            uint32_t   EnableHWIntraPrediction         : 1;
            uint32_t   EnableDebugDumps                : 1;
            uint32_t   EnableCoeffClamp                : 1;
            uint32_t   EnableEnableChromaIPEnhancement : 1;
            uint32_t   EnableMPUHistogramUpdate        : 1;
            uint32_t   ReservedMBZ                     : 1;
            uint32_t   VMEEnableTMCheck                : 1;
            uint32_t   VMEDistortionMeasure            : 2;
            uint32_t                                   : 21;
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
            uint32_t   LambdaSegment0                  : 16;
            uint32_t   LambdaSegment1                  : 16;
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
            uint32_t   LambdaSegment2                  : 16;
            uint32_t   LambdaSegment3                  : 16;
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
            uint32_t   AllDCBiasSegment0               : 16;
            uint32_t   AllDCBiasSegment1               : 16;
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
            uint32_t   AllDCBiasSegment2               : 16;
            uint32_t   AllDCBiasSegment3               : 16;
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
            uint32_t   ChromaDCDeQuantSegment0         : 16;
            uint32_t   ChromaDCDeQuantSegment1         : 16;
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
            uint32_t   ChromaDCDeQuantSegment2         : 16;
            uint32_t   ChromaDCDeQuantSegment3         : 16;
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
            uint32_t   ChromaACDeQuantSegment0         : 16;
            uint32_t   ChromaACDeQuantSegment1         : 16;
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
            uint32_t   ChromaACDeQuantSegment2         : 16;
            uint32_t   ChromaACDeQuantSegment3         : 16;
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
            uint32_t   ChromaAC0Threshold0Segment0     : 16;
            uint32_t   ChromaAC0Threshold1Segment0     : 16;
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
            uint32_t   ChromaAC0Threshold0Segment1     : 16;
            uint32_t   ChromaAC0Threshold1Segment1     : 16;
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
            uint32_t   ChromaAC0Threshold0Segment2     : 16;
            uint32_t   ChromaAC0Threshold1Segment2     : 16;
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
            uint32_t   ChromaAC0Threshold0Segment3     : 16;
            uint32_t   ChromaAC0Threshold1Segment3     : 16;
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
            uint32_t   ChromaDCThreshold0Segment0      : 16;
            uint32_t   ChromaDCThreshold1Segment0      : 16;
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
            uint32_t   ChromaDCThreshold2Segment0      : 16;
            uint32_t   ChromaDCThreshold3Segment0      : 16;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW15;

    // DW16
    union
    {
        struct
        {
            uint32_t   ChromaDCThreshold0Segment1      : 16;
            uint32_t   ChromaDCThreshold1Segment1      : 16;
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
            uint32_t   ChromaDCThreshold2Segment1      : 16;
            uint32_t   ChromaDCThreshold3Segment1      : 16;
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
            uint32_t   ChromaDCThreshold0Segment2      : 16;
            uint32_t   ChromaDCThreshold1Segment2      : 16;
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
            uint32_t   ChromaDCThreshold2Segment2      : 16;
            uint32_t   ChromaDCThreshold3Segment2      : 16;
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
            uint32_t   ChromaDCThreshold0Segment3      : 16;
            uint32_t   ChromaDCThreshold1Segment3      : 16;
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
            uint32_t   ChromaDCThreshold2Segment3      : 16;
            uint32_t   ChromaDCThreshold3Segment3      : 16;
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
            uint32_t   ChromaAC1ThresholdSegment0      : 16;
            uint32_t   ChromaAC1ThresholdSegment1      : 16;
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
            uint32_t   ChromaAC1ThresholdSegment2      : 16;
            uint32_t   ChromaAC1ThresholdSegment3      : 16;
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
            uint32_t   VME16x16CostSegment0            : 8;
            uint32_t   VME16x16CostSegment1            : 8;
            uint32_t   VME16x16CostSegment2            : 8;
            uint32_t   VME16x16CostSegment3            : 8;
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
            uint32_t   VME4x4CostSegment0              : 8;
            uint32_t   VME4x4CostSegment1              : 8;
            uint32_t   VME4x4CostSegment2              : 8;
            uint32_t   VME4x4CostSegment3              : 8;
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
            uint32_t   VME16x16NonDCPenaltySegment0    : 8;
            uint32_t   VME16x16NonDCPenaltySegment1    : 8;
            uint32_t   VME16x16NonDCPenaltySegment2    : 8;
            uint32_t   VME16x16NonDCPenaltySegment3    : 8;
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
            uint32_t   VME4x4NonDCPenaltySegment0      : 8;
            uint32_t   VME4x4NonDCPenaltySegment1      : 8;
            uint32_t   VME4x4NonDCPenaltySegment2      : 8;
            uint32_t   VME4x4NonDCPenaltySegment3      : 8;
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
            uint32_t                                   : 32;
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
            uint32_t                                   : 32;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW29;

    // DW30
    union
    {
        struct
        {
            uint32_t                                   : 32;
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
            uint32_t                                   : 32;
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
            uint32_t   MBEncPerMBOutDataSurfBTI        : 32;
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
            uint32_t   MBEncCurrYBTI                   : 32;
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
            uint32_t   MBEncCurrUVBTI                  : 32;
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
            uint32_t   MBModeCostLumaBTI               : 32;
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
            uint32_t   MBEncBlockModeCostBTI           : 32;
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
            uint32_t   ChromaReconSurfBTI              : 32;
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
            uint32_t   SegmentationMapBTI              : 32;
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
            uint32_t   HistogramBTI                    : 32;
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
            uint32_t   MBEncVMEDebugStreamOutBTI       : 32;
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
            uint32_t   VmeBTI                          : 32;
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
            uint32_t   IDistortionSurfaceBTI           : 32;
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
            uint32_t   MBEncCurrYDownScaledBTI          : 32;
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
            uint32_t   MBEncVMECoarseIntraBTI           : 32;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW44;
} ;
C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(struct MediaObjectVp8MbencIStaticDataG9)) == 45);

struct MediaObjectVp8MbencPStaticDataG9
{
    // DW0
    union
    {
        struct
        {
            uint32_t   FrameWidth                : 16;
            uint32_t   FrameHeight               : 16;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW0;
        //DW1
    union
    {
        struct
        {
            uint32_t FrameType                         :1;
            uint32_t MultiplePred                      :2;
            uint32_t HMEEnable                         :1;
            uint32_t HMECombineOverlap                 :2;
            uint32_t AllFractional                     :1;
            uint32_t EnableTemporalScalability         :1;
            uint32_t HMECombinedExtraSU                :8;
            uint32_t RefFrameFlags                     :4;
            uint32_t EnableSegmentation                :1;
            uint32_t EnableSegmentationInfoUpdate      :1;
            uint32_t EnableCoeffClamp                  :1;
            uint32_t MultiReferenceQPCheck             :1;
            uint32_t ModecostEnableFlag                :1;
            uint32_t MainRef                           :6;
            uint32_t EnableDebugDumps                  :1;
        };
        struct
        {
            uint32_t Value;
        };

    } DW1;

    //DW2
    union
    {
        struct
        {
            uint32_t LambdaIntraSegment0               :16;
            uint32_t LambdaInterSegment0               :16;
        };
        struct
        {
            uint32_t Value;
        };
    } DW2;

    //DW3
    union
    {
        struct
        {
            uint32_t LambdaIntraSegment1               :16;
            uint32_t LambdaInterSegment1               :16;
        };
        struct
        {
            uint32_t Value;
        };
    } DW3;

    //DW4
    union
    {
        struct
        {
            uint32_t LambdaIntraSegment2               :16;
            uint32_t LambdaInterSegment2               :16;
        };
        struct
        {
            uint32_t Value;
        };
    } DW4;

    //DW5
    union
    {
        struct
        {
            uint32_t LambdaIntraSegment3               :16;
            uint32_t LambdaInterSegment3               :16;
        };
        struct
        {
            uint32_t Value;
        };
    } DW5;

    // DW6
    union
    {
        struct
        {
            uint32_t ReferenceFrameSignBias_0          : 8;
            uint32_t ReferenceFrameSignBias_1          : 8;
            uint32_t ReferenceFrameSignBias_2          : 8;
            uint32_t ReferenceFrameSignBias_3          : 8;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW6;

    //DW7
    union
    {
        struct
        {
            uint32_t RawDistThreshold                  :16;
            uint32_t TemporalLayerID                   :8;
            uint32_t ReservedMBZ                       :8;
        };
        struct
        {
            uint32_t Value;
        };
    } DW7;

    //DW8
    union
    {
        struct
        {
            uint32_t SkipModeEnable                    :1;
            uint32_t AdaptiveSearchEnable              :1;
            uint32_t BidirectionalMixDisbale           :1;
            uint32_t ReservedMBZ1                      :2;
            uint32_t EarlyIMESuccessEnable             :1;
            uint32_t ReservedMBZ2                      :1;
            uint32_t Transform8x8FlagForInterEnable    :1;
            uint32_t ReservedMBZ3                      :16;
            uint32_t EarlyIMESuccessfulStopThreshold   :8;
        };
        struct
        {
            uint32_t Value;
        };
    } DW8;

    //DW9
    union
    {
        struct
        {
            uint32_t MaximumNumberOfMotionVectors      :6;
            uint32_t ReservedMBZ1                      :2;
            uint32_t RefIDPolarityBits                 :8;
            uint32_t BidirectionalWeight               :6;
            uint32_t ReservedMBZ2                      :6;
            uint32_t UnidirectionMixEnable             :1;
            uint32_t RefPixelBiasEnable                :1;
            uint32_t ReservedMBZ3                      :2;
        };
        struct
        {
            uint32_t Value;
        };
    } DW9;

    //DW10
    union
    {
        struct
        {
            uint32_t MaxFixedSearchPathLength          :8;
            uint32_t MaximumSearchPathLength           :8;
            uint32_t ReservedMBZ                       :16;
        };
        struct
        {
            uint32_t Value;
        };
    } DW10;

    //DW11
    union
    {
        struct
        {
            uint32_t SourceBlockSize                   :2;
            uint32_t ReservedMBZ1                      :2;
            uint32_t InterMbTypeRoadMap                :2;
            uint32_t SourceAccess                      :1;
            uint32_t ReferenceAccess                   :1;
            uint32_t SearchControl                     :3;
            uint32_t DualSearchPathOption              :1;
            uint32_t SubPelMode                        :2;
            uint32_t SkipModeType                      :1;
            uint32_t DisableFieldCacheAllocation       :1;
            uint32_t ProcessInterChromaPixelsMode      :1;
            uint32_t ForwardTransformSkipCheckEnable   :1;
            uint32_t BMEdisableforFBRMessage           :1;
            uint32_t BlockBasedSkipEnable              :1;
            uint32_t InterSADMeasureAdjustment         :2;
            uint32_t IntraSADMeasureAdjustment         :2;
            uint32_t SubMacroBlockSubPartitionMask     :6;
            uint32_t ReservedMBZ2                      :1;
        };
        struct
        {
            uint32_t Value;
        };
    } DW11;

    //DW12
    union
    {
        struct
        {
            uint32_t ReservedMBZ                       :16;
            uint32_t ReferenceSearchWindowsWidth       :8;
            uint32_t ReferenceSearchWindowsHeight      :8;
        };
        struct
        {
            uint32_t Value;
        };
    } DW12;

    // DW13
    union
    {
        struct
        {
            uint32_t Mode0CostSegment0             :8;
            uint32_t Mode1CostSegment0             :8;
            uint32_t Mode2CostSegment0             :8;
            uint32_t Mode3CostSegment0             :8;
        };
        struct
        {
            uint32_t Value;
        };
    } DW13;

    // DW14
    union
    {
        struct
        {
            uint32_t Mode4CostSegment0             :8;
            uint32_t Mode5CostSegment0             :8;
            uint32_t Mode6CostSegment0             :8;
            uint32_t Mode7CostSegment0             :8;
        };
        struct
        {
            uint32_t Value;
        };
    } DW14;

    // DW15
    union
    {
        struct
        {
            uint32_t Mode8CostSegment0             :8;
            uint32_t Mode9CostSegment0             :8;
            uint32_t RefIDCostSegment0             :8;
            uint32_t ChromaCostSegment0            :8;
        };
        struct
        {
            uint32_t Value;
        };
    } DW15;

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
            uint32_t Value;
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
            uint32_t Value;
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
            uint32_t Value;
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
            uint32_t Value;
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
            uint32_t Value;
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
            uint32_t Value;
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
            uint32_t Value;
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
            uint32_t Value;
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
            uint32_t Value;
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
            uint32_t Value;
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
            uint32_t Value;
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
            uint32_t Value;
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
            uint32_t Value;
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
            uint32_t Value;
        };
    } DW29;

    // DW30
    union
    {
        struct
        {
            uint32_t MV0CostSegment0             :8;
            uint32_t MV1CostSegment0             :8;
            uint32_t MV2CostSegment0             :8;
            uint32_t MV3CostSegment0             :8;
        };
        struct
        {
            uint32_t Value;
        };
    } DW30;

    // DW31
    union
    {
        struct
        {
            uint32_t MV4CostSegment0            :8;
            uint32_t MV5CostSegment0            :8;
            uint32_t MV6CostSegment0            :8;
            uint32_t MV7CostSegment0            :8;
        };
        struct
        {
            uint32_t Value;
        };
    } DW31;

    // DW32
    union
    {
        struct
        {
            uint32_t Intra16x16NoDCPenaltySegment0 :8;
            uint32_t Intra16x16NoDCPenaltySegment1 :8;
            uint32_t ReservedMBZ1                  :7;
            uint32_t BilinearEnable                :1;
            uint32_t ReservedMBZ2                  :8;
        };
        struct
        {
            uint32_t Value;
        };
    } DW32;

    // DW33
    union
    {
        struct
        {
            uint32_t HMECombineLen                 :16;
            uint32_t Intra16x16NoDCPenaltySegment2 :8;
            uint32_t Intra16x16NoDCPenaltySegment3 :8;

        };
        struct
        {
            uint32_t Value;
        };
    } DW33;

    // DW34
    union
    {
        struct
        {
            uint32_t MvRefCostContext_0_0_0         : 16;
            uint32_t MvRefCostContext_0_0_1         : 16;
        };
        struct
        {
            uint32_t Value;
        };
    } DW34;

    // DW35
    union
    {
        struct
        {
            uint32_t MvRefCostContext_0_1_0         : 16;
            uint32_t MvRefCostContext_0_1_1         : 16;
        };
        struct
        {
            uint32_t Value;
        };
    } DW35;

    // DW36
    union
    {
        struct
        {
            uint32_t MvRefCostContext_0_2_0         : 16;
            uint32_t MvRefCostContext_0_2_1         : 16;
        };
        struct
        {
            uint32_t Value;
        };
    } DW36;

    // DW37
    union
    {
        struct
        {
            uint32_t MvRefCostContext_0_3_0         : 16;
            uint32_t MvRefCostContext_0_3_1         : 16;
        };
        struct
        {
            uint32_t Value;
        };
    } DW37;

    // DW38
    union
    {
        struct
        {
            uint32_t MvRefCostContext_1_0_0         : 16;
            uint32_t MvRefCostContext_1_0_1         : 16;
        };
        struct
        {
            uint32_t Value;
        };
    } DW38;

    // DW39
    union
    {
        struct
        {
            uint32_t MvRefCostContext_1_1_0         : 16;
            uint32_t MvRefCostContext_1_1_1         : 16;
        };
        struct
        {
            uint32_t Value;
        };
    } DW39;

    // DW40
    union
    {
        struct
        {
            uint32_t MvRefCostContext_1_2_0         : 16;
            uint32_t MvRefCostContext_1_2_1         : 16;
        };
        struct
        {
            uint32_t Value;
        };
    } DW40;

    // DW41
    union
    {
        struct
        {
            uint32_t MvRefCostContext_1_3_0         : 16;
            uint32_t MvRefCostContext_1_3_1         : 16;
        };
        struct
        {
            uint32_t Value;
        };
    } DW41;

    // DW42
    union
    {
        struct
        {
            uint32_t MvRefCostContext_2_0_0         : 16;
            uint32_t MvRefCostContext_2_0_1         : 16;
        };
        struct
        {
            uint32_t Value;
        };
    } DW42;

    // DW43
    union
    {
        struct
        {
            uint32_t MvRefCostContext_2_1_0         : 16;
            uint32_t MvRefCostContext_2_1_1         : 16;
        };
        struct
        {
            uint32_t Value;
        };
    } DW43;

    // DW44
    union
    {
        struct
        {
            uint32_t MvRefCostContext_2_2_0         : 16;
            uint32_t MvRefCostContext_2_2_1         : 16;
        };
        struct
        {
            uint32_t Value;
        };
    } DW44;

    // DW45
    union
    {
        struct
        {
            uint32_t MvRefCostContext_2_3_0         : 16;
            uint32_t MvRefCostContext_2_3_1         : 16;
        };
        struct
        {
            uint32_t Value;
        };
    } DW45;

    // DW46
    union
    {
        struct
        {
            uint32_t MvRefCostContext_3_0_0         : 16;
            uint32_t MvRefCostContext_3_0_1         : 16;
        };
        struct
        {
            uint32_t Value;
        };
    } DW46;

    // DW47
    union
    {
        struct
        {
            uint32_t MvRefCostContext_3_1_0         : 16;
            uint32_t MvRefCostContext_3_1_1         : 16;
        };
        struct
        {
            uint32_t Value;
        };
    } DW47;

    // DW48
    union
    {
        struct
        {
            uint32_t MvRefCostContext_3_2_0         : 16;
            uint32_t MvRefCostContext_3_2_1         : 16;
        };
        struct
        {
            uint32_t Value;
        };
    } DW48;

    // DW49
    union
    {
        struct
        {
            uint32_t MvRefCostContext_3_3_0         : 16;
            uint32_t MvRefCostContext_3_3_1         : 16;
        };
        struct
        {
            uint32_t Value;
        };
    } DW49;

    // DW50
    union
    {
        struct
        {
            uint32_t MvRefCostContext_4_0_0         : 16;
            uint32_t MvRefCostContext_4_0_1         : 16;
        };
        struct
        {
            uint32_t Value;
        };
    } DW50;

    // DW51
    union
    {
        struct
        {
            uint32_t MvRefCostContext_4_1_0         : 16;
            uint32_t MvRefCostContext_4_1_1         : 16;
        };
        struct
        {
            uint32_t Value;
        };
    } DW51;

    // DW52
    union
    {
        struct
        {
            uint32_t MvRefCostContext_4_2_0         : 16;
            uint32_t MvRefCostContext_4_2_1         : 16;
        };
        struct
        {
            uint32_t Value;
        };
    } DW52;

    // DW53
    union
    {
        struct
        {
            uint32_t MvRefCostContext_4_3_0         : 16;
            uint32_t MvRefCostContext_4_3_1         : 16;
        };
        struct
        {
            uint32_t Value;
        };
    } DW53;

    // DW54
    union
    {
        struct
        {
            uint32_t MvRefCostContext_5_0_0         : 16;
            uint32_t MvRefCostContext_5_0_1         : 16;
        };
        struct
        {
            uint32_t Value;
        };
    } DW54;

    // DW55
    union
    {
        struct
        {
            uint32_t MvRefCostContext_5_1_0         : 16;
            uint32_t MvRefCostContext_5_1_1         : 16;
        };
        struct
        {
            uint32_t Value;
        };
    } DW55;

    // DW56
    union
    {
        struct
        {
            uint32_t MvRefCostContext_5_2_0         : 16;
            uint32_t MvRefCostContext_5_2_1         : 16;
        };
        struct
        {
            uint32_t Value;
        };
    } DW56;

    // DW57
    union
    {
        struct
        {
            uint32_t MvRefCostContext_5_3_0         : 16;
            uint32_t MvRefCostContext_5_3_1         : 16;
        };
        struct
        {
            uint32_t Value;
        };
    } DW57;

    // DW58
    union
    {
        struct
        {
            uint32_t EncCost16x16                   : 16;
            uint32_t EncCost16x8                    : 16;
        };
        struct
        {
            uint32_t Value;
        };
    } DW58;

    // DW59
    union
    {
        struct
        {
            uint32_t EncCost8x8                     : 16;
            uint32_t EncCost4x4                     : 16;
        };
        struct
        {
            uint32_t Value;
        };
    } DW59;

    // DW60
    union
    {
        struct
        {
            uint32_t FrameCountProbabilityRefFrameCost_0       : 16;
            uint32_t FrameCountProbabilityRefFrameCost_1       : 16;
        };
        struct
        {
            uint32_t Value;
        };
    } DW60;

    // DW61
    union
    {
        struct
        {
            uint32_t FrameCountProbabilityRefFrameCost_2       : 16;
            uint32_t FrameCountProbabilityRefFrameCost_3       : 16;
        };
        struct
        {
            uint32_t Value;
        };
    } DW61;

    // DW62
    union
    {
        struct
        {
            uint32_t AverageQPOfLastRefFrame   :8;
            uint32_t AverageQPOfGoldRefFrame   :8;
            uint32_t AverageQPOfAltRefFrame    :8;
            uint32_t ReservedMBZ               :8;
        };
        struct
        {
            uint32_t Value;
        };

    } DW62;

    // DW63
    union
    {
        struct
        {
            uint32_t Intra4x4NoDCPenaltySegment0   :8;
            uint32_t Intra4x4NoDCPenaltySegment1   :8;
            uint32_t Intra4x4NoDCPenaltySegment2   :8;
            uint32_t Intra4x4NoDCPenaltySegment3   :8;
        };
        struct
        {
            uint32_t Value;
        };

    } DW63;

    // DW64
    union
    {
        struct
        {
            uint32_t Mode0CostSegment1         :8;
            uint32_t Mode1CostSegment1         :8;
            uint32_t Mode2CostSegment1         :8;
            uint32_t Mode3CostSegment1         :8;
        };
        struct
        {
            uint32_t Value;
        };
    } DW64;

    // DW65
    union
    {
        struct
        {
            uint32_t Mode4CostSegment1         :8;
            uint32_t Mode5CostSegment1         :8;
            uint32_t Mode6CostSegment1         :8;
            uint32_t Mode7CostSegment1         :8;
        };
        struct
        {
            uint32_t Value;
        };
    } DW65;

    // DW66
    union
    {
        struct
        {
            uint32_t Mode8CostSegment1         :8;
            uint32_t Mode9CostSegment1         :8;
            uint32_t RefIDCostSegment1         :8;
            uint32_t ChromaCostSegment1        :8;
        };
        struct
        {
            uint32_t Value;
        };
    } DW66;

    // DW67
    union
    {
        struct
        {
            uint32_t MV0CostSegment1           :8;
            uint32_t MV1CostSegment1           :8;
            uint32_t MV2CostSegment1           :8;
            uint32_t MV3CostSegment1           :8;
        };
        struct
        {
            uint32_t Value;
        };
    } DW67;

    // DW68
    union
    {
        struct
        {
            uint32_t MV4CostSegment1           :8;
            uint32_t MV5CostSegment1           :8;
            uint32_t MV6CostSegment1           :8;
            uint32_t MV7CostSegment1           :8;
        };
        struct
        {
            uint32_t Value;
        };
    } DW68;

    // DW69
    union
    {
        struct
        {
            uint32_t Mode0CostSegment2         :8;
            uint32_t Mode1CostSegment2         :8;
            uint32_t Mode2CostSegment2         :8;
            uint32_t Mode3CostSegment2         :8;
        };
        struct
        {
            uint32_t Value;
        };
    } DW69;

    // DW70
    union
    {
        struct
        {
            uint32_t Mode4CostSegment2         :8;
            uint32_t Mode5CostSegment2         :8;
            uint32_t Mode6CostSegment2         :8;
            uint32_t Mode7CostSegment2         :8;
        };
        struct
        {
            uint32_t Value;
        };
    } DW70;

    // DW71
    union
    {
        struct
        {
            uint32_t Mode8CostSegment2         :8;
            uint32_t Mode9CostSegment2         :8;
            uint32_t RefIDCostSegment2         :8;
            uint32_t ChromaCostSegment2        :8;
        };
        struct
        {
            uint32_t Value;
        };
    } DW71;

    // DW72
    union
    {
        struct
        {
            uint32_t MV0CostSegment2           :8;
            uint32_t MV1CostSegment2           :8;
            uint32_t MV2CostSegment2           :8;
            uint32_t MV3CostSegment2           :8;
        };
        struct
        {
            uint32_t Value;
        };
    } DW72;

    // DW73
    union
    {
        struct
        {
            uint32_t MV4CostSegment2           :8;
            uint32_t MV5CostSegment2           :8;
            uint32_t MV6CostSegment2           :8;
            uint32_t MV7CostSegment2           :8;
        };
        struct
        {
            uint32_t Value;
        };
    } DW73;

    // DW74
    union
    {
        struct
        {
            uint32_t Mode0CostSegment3         :8;
            uint32_t Mode1CostSegment3         :8;
            uint32_t Mode2CostSegment3         :8;
            uint32_t Mode3CostSegment3         :8;
        };
        struct
        {
            uint32_t Value;
        };
    } DW74;

    // DW75
    union
    {
        struct
        {
            uint32_t Mode4CostSegment3         :8;
            uint32_t Mode5CostSegment3         :8;
            uint32_t Mode6CostSegment3         :8;
            uint32_t Mode7CostSegment3         :8;
        };
        struct
        {
            uint32_t Value;
        };
    } DW75;

    // DW76
    union
    {
        struct
        {
            uint32_t Mode8CostSegment3         :8;
            uint32_t Mode9CostSegment3         :8;
            uint32_t RefIDCostSegment3         :8;
            uint32_t ChromaCostSegment3        :8;
        };
        struct
        {
            uint32_t Value;
        };
    } DW76;

    // DW77
    union
    {
        struct
        {
            uint32_t MV0CostSegment3             :8;
            uint32_t MV1CostSegment3             :8;
            uint32_t MV2CostSegment3             :8;
            uint32_t MV3CostSegment3             :8;
        };
        struct
        {
            uint32_t Value;
        };
    } DW77;

    // DW78
    union
    {
        struct
        {
            uint32_t MV4CostSegment3            :8;
            uint32_t MV5CostSegment3            :8;
            uint32_t MV6CostSegment3            :8;
            uint32_t MV7CostSegment3            :8;
        };
        struct
        {
            uint32_t Value;
        };
    } DW78;

    // DW79
    union
    {
        struct
        {
            uint32_t NewMVSkipThresholdSegment0 :16;
            uint32_t NewMVSkipThresholdSegment1 :16;
        };
        struct
        {
            uint32_t Value;
        };
    } DW79;

    // DW80
    union
    {
        struct
        {
            uint32_t NewMVSkipThresholdSegment2 :16;
            uint32_t NewMVSkipThresholdSegment3 :16;
        };
        struct
        {
            uint32_t Value;
        };
    } DW80;

    // DW81
    union
    {
        struct
        {
            uint32_t PerMbOutputDataSurfaceBTI                 :32;
        };
        struct
        {
            uint32_t Value;
        };
    } DW81;

    // DW82
    union
    {
        struct
        {
            uint32_t CurrentPictureYSurfaceBTI                 :32;
        };
        struct
        {
            uint32_t Value;
        };
    } DW82;

    // DW83
    union
    {
        struct
        {
            uint32_t CurrentPictureInterleavedUVSurfaceBTI     :32;
        };
        struct
        {
            uint32_t Value;
        };
    } DW83;

    // DW84
    union
    {
        struct
        {
            uint32_t HMEMVDataSurfaceBTI                       :32;
        };
        struct
        {
            uint32_t Value;
        };
    } DW84;

    // DW85
    union
    {
        struct
        {
            uint32_t MVDataSurfaceBTI                          :32;
        };
        struct
        {
            uint32_t Value;
        };
    } DW85;

    // DW86
    union
    {
        struct
        {
            uint32_t MbCountPerReferenceFrameBTI               :32;
        };
        struct
        {
            uint32_t Value;
        };
    } DW86;

    // DW87
    union
    {
        struct
        {
            uint32_t VMEInterPredictionBTI                     :32;
        };
        struct
        {
            uint32_t Value;
        };
    } DW87;

    // DW88
    union
    {
        struct
        {
            uint32_t ActiveRef1BTI                             : 32;
        };
        struct
        {
            uint32_t Value;
        };
    } DW88;

    // DW89
    union
    {
        struct
        {
            uint32_t ActiveRef2BTI                             : 32;
        };
        struct
        {
            uint32_t Value;
        };
    } DW89;

    // DW90
    union
    {
        struct
        {
            uint32_t ActiveRef3BTI                             : 32;
        };
        struct
        {
            uint32_t Value;
        };
    } DW90;

    // DW91
    union
    {
        struct
        {
            uint32_t PerMbQuantDataBTI                         :32;
        };
        struct
        {
            uint32_t Value;
        };
    } DW91;

    // DW92
    union
    {
        struct
        {
            uint32_t SegmentMapBTI                             :32;
        };
        struct
        {
            uint32_t Value;
        };
    } DW92;

    // DW93
    union
    {
        struct
        {
            uint32_t InterPredictionDistortionBTI              :32;
        };
        struct
        {
            uint32_t Value;
        };
    } DW93;

    // DW94
    union
    {
        struct
        {
            uint32_t HistogramBTI                              :32;
        };
        struct
        {
            uint32_t Value;
        };
    } DW94;

    // DW95
    union
    {
        struct
        {
            uint32_t PredMVDataBTI                             :32;
        };
        struct
        {
            uint32_t Value;
        };
    } DW95;

    // DW96
    union
    {
        struct
        {
            uint32_t ModeCostUpdateBTI       :32;
        };
        struct
        {
            uint32_t Value;
        };
    } DW96;

    // DW97
    union
    {
        struct
        {
            uint32_t KernelDebugDumpBTI       :32;
        };
        struct
        {
            uint32_t Value;
        };
    } DW97;
};
C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(struct MediaObjectVp8MbencPStaticDataG9)) == 98);

struct MediaObjectVp8MpuFhbStaticDataG9
{
    // uint32_t 0
    union {
        struct {
            uint32_t       FrameWidth                           : MOS_BITFIELD_RANGE(  0,15 );   //
            uint32_t       FrameHeight                          : MOS_BITFIELD_RANGE( 16,31 );   //
        };
        uint32_t Value;
    } DW0;

    // uint32_t 1
    union {
        struct {
            uint32_t       FrameType                            : MOS_BITFIELD_BIT(       0 );   //
            uint32_t       Version                              : MOS_BITFIELD_RANGE(  1, 3 );   //
            uint32_t       ShowFrame                            : MOS_BITFIELD_BIT(       4 );   //
            uint32_t       HorizontalScaleCode                  : MOS_BITFIELD_RANGE(  5, 6 );   //
            uint32_t       VerticalScaleCode                    : MOS_BITFIELD_RANGE(  7, 8 );   //
            uint32_t       ColorSpaceType                       : MOS_BITFIELD_BIT(       9 );   //
            uint32_t       ClampType                            : MOS_BITFIELD_BIT(      10 );   //
            uint32_t       PartitionNumL2                       : MOS_BITFIELD_RANGE( 11,12 );   //
            uint32_t       EnableSegmentation                   : MOS_BITFIELD_BIT(      13 );   //
            uint32_t       SegMapUpdate                         : MOS_BITFIELD_BIT(      14 );   //
            uint32_t       SegmentationFeatureUpdate            : MOS_BITFIELD_BIT(      15 );   //
            uint32_t       SegmentationFeatureMode              : MOS_BITFIELD_BIT(      16 );   //
            uint32_t       LoopFilterType                       : MOS_BITFIELD_BIT(      17 );   //
            uint32_t       SharpnessLevel                       : MOS_BITFIELD_RANGE( 18,20 );   //
            uint32_t       LoopFilterAdjustmentOn               : MOS_BITFIELD_BIT(      21 );   //
            uint32_t       MBNoCoeffiscientSkip                 : MOS_BITFIELD_BIT(      22 );   //
            uint32_t       GoldenReferenceCopyFlag              : MOS_BITFIELD_RANGE( 23,24 );   //
            uint32_t       AlternateReferenceCopyFlag           : MOS_BITFIELD_RANGE( 25,26 );   //
            uint32_t       LastFrameUpdate                      : MOS_BITFIELD_BIT(      27 );   //
            uint32_t       SignBiasGolden                       : MOS_BITFIELD_BIT(      28 );   //
            uint32_t       SignBiasAltRef                       : MOS_BITFIELD_BIT(      29 );   //
            uint32_t       RefreshEntropyP                      : MOS_BITFIELD_BIT(      30 );   //
            uint32_t       ForcedLFUpdateForKeyFrame            : MOS_BITFIELD_BIT(      31 );   //
        };
        uint32_t Value;
    } DW1;

    // uint32_t 2
    union {
        struct {
            uint32_t       LoopFilterLevel                      : MOS_BITFIELD_RANGE(  0, 5 );   //
            uint32_t                                            : MOS_BITFIELD_RANGE(  6, 7 );   //
            uint32_t       Qindex                               : MOS_BITFIELD_RANGE(  8,14 );   //
            uint32_t                                            : MOS_BITFIELD_BIT(      15 );   //
            uint32_t       Y1DCQindex                           : MOS_BITFIELD_RANGE( 16,23 );   //
            uint32_t       Y2DCQindex                           : MOS_BITFIELD_RANGE( 24,31 );   //
        };
        uint32_t Value;
    } DW2;

    // uint32_t 3
    union {
        struct {
            uint32_t       Y2ACQindex                           : MOS_BITFIELD_RANGE(  0, 7 );   //
            uint32_t       UVDCQindex                           : MOS_BITFIELD_RANGE(  8,15 );   //
            uint32_t       UVACQindex                           : MOS_BITFIELD_RANGE( 16,23 );   //
            uint32_t       FeatureData0Segment0                 : MOS_BITFIELD_RANGE( 24,31 );   //
        };
        uint32_t Value;
    } DW3;

    // uint32_t 4
    union {
        struct {
            uint32_t       FeatureData0Segment1                 : MOS_BITFIELD_RANGE(  0, 7 );   //
            uint32_t       FeatureData0Segment2                 : MOS_BITFIELD_RANGE(  8,15 );   //
            uint32_t       FeatureData0Segment3                 : MOS_BITFIELD_RANGE( 16,23 );   //
            uint32_t       FeatureData1Segment0                 : MOS_BITFIELD_RANGE( 24,31 );   //
        };
        uint32_t Value;
    } DW4;

    // uint32_t 5
    union {
        struct {
            uint32_t       FeatureData1Segment1                 : MOS_BITFIELD_RANGE(  0, 7 );   //
            uint32_t       FeatureData1Segment2                 : MOS_BITFIELD_RANGE(  8,15 );   //
            uint32_t       FeatureData1Segment3                 : MOS_BITFIELD_RANGE( 16,23 );   //
            uint32_t       RefLFDelta0                          : MOS_BITFIELD_RANGE( 24,31 );   //
        };
        uint32_t Value;
    } DW5;

    // uint32_t 6
    union {
        struct {
            uint32_t       RefLFDelta1                          : MOS_BITFIELD_RANGE(  0, 7 );   //
            uint32_t       RefLFDelta2                          : MOS_BITFIELD_RANGE(  8,15 );   //
            uint32_t       RefLFDelta3                          : MOS_BITFIELD_RANGE( 16,23 );   //
            uint32_t       ModeLFDelta0                         : MOS_BITFIELD_RANGE( 24,31 );   //
        };
        uint32_t Value;
    } DW6;

    // uint32_t 7
    union {
        struct {
            uint32_t       ModeLFDelta1                         : MOS_BITFIELD_RANGE(  0, 7 );   //
            uint32_t       ModeLFDelta2                         : MOS_BITFIELD_RANGE(  8,15 );   //
            uint32_t       ModeLFDelta3                         : MOS_BITFIELD_RANGE( 16,23 );   //
            uint32_t       ForcedTokenSurfaceRead               : MOS_BITFIELD_BIT(      24 );
            uint32_t       ModecostEnableFlag                   : MOS_BITFIELD_BIT(      25 );
            uint32_t        MCFilterSelect                       : MOS_BITFIELD_BIT(      26 );
            uint32_t        ChromaFullPixelMCFilterMode : MOS_BITFIELD_BIT(27);
            uint32_t       MaxNumPakPasses : MOS_BITFIELD_RANGE(28, 31);   //

        };
        uint32_t Value;
    } DW7;

    // uint32_t 8
    union {
        struct {
            uint32_t       TemporalLayerID : MOS_BITFIELD_RANGE(0, 7);   //
            uint32_t       NumTLevels      : MOS_BITFIELD_RANGE(8, 15);   //
            uint32_t       ReservedMBZ     : MOS_BITFIELD_RANGE(16, 31);   //
        };
        uint32_t Value;
    } DW8;

    // uint32_t 9
    union {
        struct {
            uint32_t       ReservedMBZ     : MOS_BITFIELD_RANGE(0, 31);   //
        };
        uint32_t Value;
    } DW9;

    // uint32_t 10
    union {
        struct {
            uint32_t       ReservedMBZ     : MOS_BITFIELD_RANGE(0, 31);   //
        };
        uint32_t Value;
    } DW10;

    // uint32_t 11
    union {
        struct {
            uint32_t       ReservedMBZ     : MOS_BITFIELD_RANGE(0, 31);   //
        };
        uint32_t Value;
    } DW11;

    // uint32_t 12
    union {
        struct {
            uint32_t       HistogramBTI                        : MOS_BITFIELD_RANGE(  0, 31);   //
        };
        uint32_t Value;
    } DW12;

    // uint32_t 13
    union {
        struct {
            uint32_t       ReferenceModeProbabilityBTI         : MOS_BITFIELD_RANGE(  0, 31);   //
        };
        uint32_t Value;
    } DW13;

    // uint32_t 14
    union {
        struct {
            uint32_t       ModeProbabilityBTI                  : MOS_BITFIELD_RANGE(  0, 31);   //
        };
        uint32_t Value;
    } DW14;

    // uint32_t 15
    union {
        struct {
            uint32_t       ReferenceTokenProbabilityBTI        : MOS_BITFIELD_RANGE(  0, 31);   //
        };
        uint32_t Value;
    } DW15;

    // uint32_t 16
    union {
        struct {
            uint32_t       TokenProbabilityBTI                 : MOS_BITFIELD_RANGE(  0, 31);   //
        };
        uint32_t Value;
    } DW16;

    // uint32_t 17
    union {
        struct {
            uint32_t       FrameHeaderBitstreamBTI             : MOS_BITFIELD_RANGE(  0, 31);   //
        };
        uint32_t Value;
    } DW17;

    // uint32_t 18
    union {
        struct {
            uint32_t       HeaderMetaDataBTI                   : MOS_BITFIELD_RANGE(  0, 31);   //
        };
        uint32_t Value;
    } DW18;

    // uint32_t 19
    union {
        struct {
            uint32_t       PictureStateBTI                     : MOS_BITFIELD_RANGE(  0, 31);   //
        };
        uint32_t Value;
    } DW19;

    // uint32_t 20
    union {
        struct {
            uint32_t       MPUBitStreamBTI                     : MOS_BITFIELD_RANGE(  0, 31);   //
        };
        uint32_t Value;
    } DW20;

    union {
        struct {
            uint32_t       TokenBitsDataBTI                    : MOS_BITFIELD_RANGE(  0, 31);   //
        };
        uint32_t Value;
    } DW21;
    union {
        struct {
            uint32_t       KernelDebugDumpBTI                  : MOS_BITFIELD_RANGE(  0, 31);   //
        };
        uint32_t Value;
    } DW22;
    union {
        struct {
            uint32_t       EntropyCostBTI                      : MOS_BITFIELD_RANGE(  0, 31);   //
        };
        uint32_t Value;
    } DW23;
    union {
        struct {
            uint32_t       ModeCostUpdateBTI                   : MOS_BITFIELD_RANGE(  0, 31);   //
        };
        uint32_t Value;
    } DW24;
};
C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(struct MediaObjectVp8MpuFhbStaticDataG9)) == 25);

struct MediaObjectVp8TpuFhbStaticDataG9
{
// uint32_t 0
    union {
        struct {
            uint32_t       MBsInFrame                           : MOS_BITFIELD_RANGE(  0, 31);   //
        };
        uint32_t Value;
    } DW0;

    // uint32_t 1
    union {
        struct {
            uint32_t       FrameType                            : MOS_BITFIELD_BIT(       0 );   //
            uint32_t       EnableSegmentation                   : MOS_BITFIELD_BIT(       1 );   //
            uint32_t       RebinarizationFrameHdr               : MOS_BITFIELD_BIT(       2 );   //
            uint32_t       RefreshEntropyP                      : MOS_BITFIELD_BIT(       3 );   //
            uint32_t       MBNoCoeffiscientSkip                 : MOS_BITFIELD_BIT(       4 );   //
            uint32_t                                            : MOS_BITFIELD_RANGE(  5,31 );   //
        };
        uint32_t Value;
    } DW1;

    // uint32_t 2
    union {
        struct {
            uint32_t       TokenProbabilityStatOffset           : MOS_BITFIELD_RANGE(  0,15 );   //
            uint32_t       TokenProbabilityEndOffset            : MOS_BITFIELD_RANGE( 16,31 );   //
        };
        uint32_t Value;
    } DW2;

    // uint32_t 3
    union {
        struct {
            uint32_t       FrameHeaderBitCount                  : MOS_BITFIELD_RANGE(  0,15 );   //
            uint32_t       MaxQP                                : MOS_BITFIELD_RANGE( 16,23 );   //
            uint32_t       MinQP                                : MOS_BITFIELD_RANGE( 24,31 );   //
        };
        uint32_t Value;
    } DW3;

    // uint32_t 4
    union {
        struct {
            uint32_t       LoopFilterLevelSegment0              : MOS_BITFIELD_RANGE(  0, 7 );   //
            uint32_t       LoopFilterLevelSegment1              : MOS_BITFIELD_RANGE(  8,15 );   //
            uint32_t       LoopFilterLevelSegment2              : MOS_BITFIELD_RANGE( 16,23 );   //
            uint32_t       LoopFilterLevelSegment3              : MOS_BITFIELD_RANGE( 24,31 );   //
        };
        uint32_t Value;
    } DW4;

    // uint32_t 5
    union {
        struct {
            uint32_t       QuantizationIndexSegment0            : MOS_BITFIELD_RANGE(  0, 7 );   //
            uint32_t       QuantizationIndexSegment1            : MOS_BITFIELD_RANGE(  8,15 );   //
            uint32_t       QuantizationIndexSegment2            : MOS_BITFIELD_RANGE( 16,23 );   //
            uint32_t       QuantizationIndexSegment3            : MOS_BITFIELD_RANGE( 24,31 );   //
        };
        uint32_t Value;
    } DW5;

    // uint32_t 6
    union {
        struct {
            uint32_t        PakPassNum                           : MOS_BITFIELD_RANGE(  0, 31);   //
        };
        uint32_t Value;
    } DW6;

    // uint32_t 7
    union {
        struct {
            uint32_t        TokenCostDeltaThreshold                    : MOS_BITFIELD_RANGE(0, 15);   //
            uint32_t        SkipCostDeltaThreshold                     : MOS_BITFIELD_RANGE(16, 31);   //
        };
        uint32_t Value;
    } DW7;

    // uint32_t 8
    union {
        struct {
            uint32_t       CumulativeDQIndex01                : MOS_BITFIELD_RANGE(  0, 31);   //
        };
        uint32_t Value;
    } DW8;

    // uint32_t 9
    union {
        struct {
            uint32_t       CumulativeDQIndex02                 : MOS_BITFIELD_RANGE(  0, 31);   //
        };
        uint32_t Value;
    } DW9;

    // uint32_t 10
    union {
        struct {
            uint32_t       CumulativeLoopFilter01              : MOS_BITFIELD_RANGE(  0, 31);   //
        };
        uint32_t Value;
    } DW10;

    // uint32_t 11
    union {
        struct {
            uint32_t       CumulativeLoopFilter02              : MOS_BITFIELD_RANGE(  0, 31);   //
        };
        uint32_t Value;
    } DW11;

    // uint32_t 12
    union {
        struct {
            uint32_t       PakTokenStatisticsBTI                : MOS_BITFIELD_RANGE(  0, 31);   //
        };
        uint32_t Value;
    } DW12;

    // uint32_t 13
    union {
        struct {
            uint32_t       TokenUpdateFlagsBTI                  : MOS_BITFIELD_RANGE(  0, 31);   //
        };
        uint32_t Value;
    } DW13;

    // uint32_t 14
    union {
        struct {
            uint32_t       EntropyCostTableBTI                  : MOS_BITFIELD_RANGE(  0, 31);   //
        };
        uint32_t Value;
    } DW14;

    // uint32_t 15
    union {
        struct {
            uint32_t       FrameHeaderBitstreamBTI              : MOS_BITFIELD_RANGE(  0, 31);   //
        };
        uint32_t Value;
    } DW15;
    // uint32_t 16
    union {
        struct {
            uint32_t       DefaultTokenProbabilityBTI           : MOS_BITFIELD_RANGE(  0, 31);   //
        };
        uint32_t Value;
    } DW16;

    // uint32_t 17
    union {
        struct {
            uint32_t       PictureStateBTI                      : MOS_BITFIELD_RANGE(  0, 31);   //
        };
        uint32_t Value;
    } DW17;

    // uint32_t 18
    union {
        struct {
            uint32_t       MpuCurbeDataBTI                      : MOS_BITFIELD_RANGE(  0, 31);   //
        };
        uint32_t Value;
    } DW18;

    // uint32_t 19
    union {
        struct {
            uint32_t       HeaderMetaDataBTI                    : MOS_BITFIELD_RANGE(  0, 31);   //
        };
        uint32_t Value;
    } DW19;

    // uint32_t 20
    union {
        struct {
            uint32_t       TokenProbabilityBTI                  : MOS_BITFIELD_RANGE(  0, 31);   //
        };
        uint32_t Value;
    } DW20;

    // uint32_t 21
    union {
        struct {
            uint32_t       PakHardwareTokenProbabilityPass1BTI  : MOS_BITFIELD_RANGE(  0, 31);   //
        };
        uint32_t Value;
    } DW21;

    // uint32_t 22
    union {
        struct {
            uint32_t       KeyFrameTokenProbabilityBTI          : MOS_BITFIELD_RANGE(  0, 31);   //
        };
        uint32_t Value;
    } DW22;

    // uint32_t 23
    union {
        struct {
            uint32_t       UpdatedTokenProbabilityBTI           : MOS_BITFIELD_RANGE(  0, 31);   //
        };
        uint32_t Value;
    } DW23;

    // uint32_t 24
    union {
        struct {
            uint32_t       PakHardwareTokenProbabilityPass2BTI  : MOS_BITFIELD_RANGE(  0, 31);   //
        };
        uint32_t Value;
    } DW24;

    // uint32_t 25
    union {
        struct {
            uint32_t       KernelDebugDumpBTI                   : MOS_BITFIELD_RANGE(  0, 31);   //
        };
        uint32_t Value;
    } DW25;

    // uint32_t 26
    union {
    struct {
            uint32_t       RepakDecisionSurfaceBTI               : MOS_BITFIELD_RANGE(0, 31);   //
        };
        uint32_t Value;
    } DW26;
};
C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(struct MediaObjectVp8TpuFhbStaticDataG9)) == 27);

struct CodechalVp8KernelHeaderG9 {
    int nKernelCount;

    // Normal mode
    CODECHAL_KERNEL_HEADER VP8MBEnc_Norm_Frm_I;
    CODECHAL_KERNEL_HEADER VP8MBEnc_Norm_Frm_P;

    // MPU/FHB
    CODECHAL_KERNEL_HEADER VP8_MPU;

    // TPU
    CODECHAL_KERNEL_HEADER VP8_TPU;

    // Intra prediction mode search  for only luma components of key-frame.
    // To be used when HW intra prediction is disabled.
    CODECHAL_KERNEL_HEADER VP8MBEnc_I_Luma;

    // HME
    CODECHAL_KERNEL_HEADER VP8_ME_P;

    // DownScaling
    CODECHAL_KERNEL_HEADER PLY_DScale_PLY;

    // MBEnc I Dist
    CODECHAL_KERNEL_HEADER VP8MBEnc_I_Dist;

    // BRC Init
    CODECHAL_KERNEL_HEADER VP8_BRC_InitFrame;

    // BRC Reset
    CODECHAL_KERNEL_HEADER VP8_BRC_ResetFrame;

    // BRC Update
    CODECHAL_KERNEL_HEADER VP8_BRC_FrameEncUpdate;
};

const uint8_t VP8_IFRAME_VME_COSTS_G9[128][4] =
{
    {0x05, 0x1f, 0x02, 0x09},
    {0x05, 0x1f, 0x02, 0x09},
    {0x08, 0x2b, 0x03, 0x0e},
    {0x08, 0x2b, 0x03, 0x0e},
    {0x0a, 0x2f, 0x04, 0x12},
    {0x0a, 0x2f, 0x04, 0x12},
    {0x0d, 0x39, 0x05, 0x17},
    {0x0d, 0x39, 0x05, 0x17},
    {0x0d, 0x39, 0x05, 0x17},
    {0x0f, 0x3b, 0x06, 0x1b},
    {0x0f, 0x3b, 0x06, 0x1b},
    {0x19, 0x3d, 0x07, 0x20},
    {0x19, 0x3d, 0x07, 0x20},
    {0x1a, 0x3f, 0x08, 0x24},
    {0x1a, 0x3f, 0x08, 0x24},
    {0x1a, 0x3f, 0x08, 0x24},
    {0x1b, 0x48, 0x09, 0x29},
    {0x1b, 0x48, 0x09, 0x29},
    {0x1d, 0x49, 0x09, 0x2d},
    {0x1d, 0x49, 0x09, 0x2d},
    {0x1d, 0x49, 0x09, 0x2d},
    {0x1d, 0x49, 0x09, 0x2d},
    {0x1e, 0x4a, 0x0a, 0x32},
    {0x1e, 0x4a, 0x0a, 0x32},
    {0x1e, 0x4a, 0x0a, 0x32},
    {0x1e, 0x4a, 0x0a, 0x32},
    {0x1f, 0x4b, 0x0b, 0x36},
    {0x1f, 0x4b, 0x0b, 0x36},
    {0x1f, 0x4b, 0x0b, 0x36},
    {0x28, 0x4c, 0x0c, 0x3b},
    {0x28, 0x4c, 0x0c, 0x3b},
    {0x29, 0x4d, 0x0d, 0x3f},
    {0x29, 0x4d, 0x0d, 0x3f},
    {0x29, 0x4e, 0x0e, 0x44},
    {0x29, 0x4e, 0x0e, 0x44},
    {0x2a, 0x4f, 0x0f, 0x48},
    {0x2a, 0x4f, 0x0f, 0x48},
    {0x2b, 0x58, 0x10, 0x4d},
    {0x2b, 0x58, 0x10, 0x4d},
    {0x2b, 0x58, 0x11, 0x51},
    {0x2b, 0x58, 0x11, 0x51},
    {0x2b, 0x58, 0x11, 0x51},
    {0x2c, 0x58, 0x12, 0x56},
    {0x2c, 0x58, 0x12, 0x56},
    {0x2c, 0x59, 0x13, 0x5a},
    {0x2c, 0x59, 0x13, 0x5a},
    {0x2d, 0x59, 0x14, 0x5f},
    {0x2d, 0x59, 0x14, 0x5f},
    {0x2e, 0x5a, 0x15, 0x63},
    {0x2e, 0x5a, 0x15, 0x63},
    {0x2e, 0x5a, 0x16, 0x68},
    {0x2e, 0x5a, 0x16, 0x68},
    {0x2e, 0x5a, 0x16, 0x68},
    {0x2f, 0x5b, 0x17, 0x6c},
    {0x2f, 0x5b, 0x17, 0x6c},
    {0x38, 0x5b, 0x18, 0x71},
    {0x38, 0x5b, 0x18, 0x71},
    {0x38, 0x5c, 0x19, 0x76},
    {0x38, 0x5c, 0x19, 0x76},
    {0x38, 0x5c, 0x1a, 0x7a},
    {0x38, 0x5c, 0x1a, 0x7a},
    {0x39, 0x5d, 0x1a, 0x7f},
    {0x39, 0x5d, 0x1a, 0x7f},
    {0x39, 0x5d, 0x1b, 0x83},
    {0x39, 0x5d, 0x1b, 0x83},
    {0x39, 0x5e, 0x1c, 0x88},
    {0x39, 0x5e, 0x1c, 0x88},
    {0x3a, 0x5e, 0x1d, 0x8c},
    {0x3a, 0x5e, 0x1d, 0x8c},
    {0x3a, 0x5f, 0x1e, 0x91},
    {0x3a, 0x5f, 0x1e, 0x91},
    {0x3a, 0x5f, 0x1f, 0x95},
    {0x3a, 0x5f, 0x1f, 0x95},
    {0x3a, 0x68, 0x20, 0x9a},
    {0x3a, 0x68, 0x20, 0x9a},
    {0x3b, 0x68, 0x21, 0x9e},
    {0x3b, 0x68, 0x21, 0x9e},
    {0x3b, 0x68, 0x22, 0xa3},
    {0x3b, 0x68, 0x22, 0xa3},
    {0x3b, 0x68, 0x23, 0xa7},
    {0x3b, 0x68, 0x23, 0xa7},
    {0x3c, 0x68, 0x24, 0xac},
    {0x3c, 0x68, 0x24, 0xac},
    {0x3c, 0x68, 0x24, 0xac},
    {0x3c, 0x69, 0x25, 0xb0},
    {0x3c, 0x69, 0x25, 0xb0},
    {0x3c, 0x69, 0x26, 0xb5},
    {0x3c, 0x69, 0x26, 0xb5},
    {0x3d, 0x69, 0x27, 0xb9},
    {0x3d, 0x69, 0x27, 0xb9},
    {0x3d, 0x69, 0x28, 0xbe},
    {0x3d, 0x69, 0x28, 0xbe},
    {0x3d, 0x6a, 0x29, 0xc2},
    {0x3d, 0x6a, 0x29, 0xc2},
    {0x3e, 0x6a, 0x2a, 0xc7},
    {0x3e, 0x6a, 0x2a, 0xc7},
    {0x3e, 0x6a, 0x2b, 0xcb},
    {0x3e, 0x6a, 0x2b, 0xd0},
    {0x3f, 0x6b, 0x2c, 0xd4},
    {0x3f, 0x6b, 0x2d, 0xd9},
    {0x3f, 0x6b, 0x2e, 0xdd},
    {0x48, 0x6b, 0x2f, 0xe2},
    {0x48, 0x6b, 0x2f, 0xe2},
    {0x48, 0x6c, 0x30, 0xe6},
    {0x48, 0x6c, 0x31, 0xeb},
    {0x48, 0x6c, 0x32, 0xf0},
    {0x48, 0x6c, 0x33, 0xf4},
    {0x48, 0x6c, 0x34, 0xf9},
    {0x49, 0x6d, 0x35, 0xfd},
    {0x49, 0x6d, 0x36, 0xff},
    {0x49, 0x6d, 0x37, 0xff},
    {0x49, 0x6d, 0x38, 0xff},
    {0x49, 0x6e, 0x3a, 0xff},
    {0x49, 0x6e, 0x3b, 0xff},
    {0x4a, 0x6e, 0x3c, 0xff},
    {0x4a, 0x6f, 0x3d, 0xff},
    {0x4a, 0x6f, 0x3d, 0xff},
    {0x4a, 0x6f, 0x3e, 0xff},
    {0x4a, 0x6f, 0x3f, 0xff},
    {0x4a, 0x6f, 0x40, 0xff},
    {0x4b, 0x78, 0x41, 0xff},
    {0x4b, 0x78, 0x42, 0xff},
    {0x4b, 0x78, 0x43, 0xff},
    {0x4b, 0x78, 0x44, 0xff},
    {0x4b, 0x78, 0x46, 0xff},
    {0x4c, 0x78, 0x47, 0xff},
    {0x4c, 0x79, 0x49, 0xff},
    {0x4c, 0x79, 0x4a, 0xff}
};

const uint32_t VP8_NewMVSkipThreshold_G9[128] =
{
    111, 120, 129, 137, 146, 155, 163, 172, 180, 189, 198, 206, 215, 224, 232, 241,
    249, 258, 267, 275, 284, 293, 301, 310, 318, 327, 336, 344, 353, 362, 370, 379,
    387, 396, 405, 413, 422, 431, 439, 448, 456, 465, 474, 482, 491, 500, 508, 517,
    525, 534, 543, 551, 560, 569, 577, 586, 594, 603, 612, 620, 629, 638, 646, 655,
    663, 672, 681, 689, 698, 707, 715, 724, 733, 741, 750, 758, 767, 776, 784, 793,
    802, 810, 819, 827, 836, 845, 853, 862, 871, 879, 888, 896, 905, 914, 922, 931,
    940, 948, 957, 965, 974, 983, 991, 1000, 1009, 1017, 1026, 1034, 1043, 1052, 1060, 1069,
    1078, 1086, 1095, 1103, 1112, 1121, 1129, 1138, 1147, 1155, 1164, 1172, 1181, 1190, 1198, 1208
};

const uint32_t VP8_COST_TABLE_G9[128][7] =
{
    {0x398f0500, 0x6f6f6f6f, 0x0000006f, 0x06040402, 0x1a0c0907, 0x08, 0x0e},
    {0x3b8f0600, 0x6f6f6f6f, 0x0000006f, 0x06040402, 0x1a0c0907, 0x0a, 0x11},
    {0x3e8f0700, 0x6f6f6f6f, 0x0000006f, 0x06040402, 0x1a0c0907, 0x0c, 0x14},
    {0x488f0800, 0x6f6f6f6f, 0x0000006f, 0x06040402, 0x1a0c0907, 0x0f, 0x18},
    {0x498f0a00, 0x6f6f6f6f, 0x0000006f, 0x0d080805, 0x291b190e, 0x11, 0x1b},
    {0x4a8f0b00, 0x6f6f6f6f, 0x0000006f, 0x0d080805, 0x291b190e, 0x13, 0x1e},
    {0x4b8f0c00, 0x6f6f6f6f, 0x0000006f, 0x0d080805, 0x291b190e, 0x15, 0x22},
    {0x4b8f0c00, 0x6f6f6f6f, 0x0000006f, 0x0d080805, 0x291b190e, 0x15, 0x22},
    {0x4d8f0d00, 0x6f6f6f6f, 0x0000006f, 0x0d080805, 0x291b190e, 0x17, 0x25},
    {0x4e8f0e00, 0x6f6f6f6f, 0x0000006f, 0x190b0c07, 0x2e281e1a, 0x19, 0x29},
    {0x4f8f0f00, 0x6f6f6f6f, 0x0000006f, 0x190b0c07, 0x2e281e1a, 0x1b, 0x2c},
    {0x588f1800, 0x6f6f6f6f, 0x0000006f, 0x190b0c07, 0x2e281e1a, 0x1d, 0x2f},
    {0x588f1900, 0x6f6f6f6f, 0x0000006f, 0x190b0c07, 0x2e281e1a, 0x1f, 0x33},
    {0x598f1900, 0x6f6f6f6f, 0x0000006f, 0x1c0f0f0a, 0x392b291e, 0x21, 0x36},
    {0x5a8f1a00, 0x6f6f6f6f, 0x0000006f, 0x1c0f0f0a, 0x392b291e, 0x23, 0x3a},
    {0x5a8f1a00, 0x6f6f6f6f, 0x0000006f, 0x1c0f0f0a, 0x392b291e, 0x23, 0x3a},
    {0x5a8f1a00, 0x6f6f6f6f, 0x0000006f, 0x1c0f0f0a, 0x392b291e, 0x25, 0x3d},
    {0x5b8f1b00, 0x6f6f6f6f, 0x0000006f, 0x1c0f0f0a, 0x392b291e, 0x27, 0x40},
    {0x5b8f1c00, 0x6f6f6f6f, 0x0000006f, 0x2819190c, 0x3c2e2b29, 0x2a, 0x44},
    {0x5b8f1c00, 0x6f6f6f6f, 0x0000006f, 0x2819190c, 0x3c2e2b29, 0x2a, 0x44},
    {0x5c8f1c00, 0x6f6f6f6f, 0x0000006f, 0x2819190c, 0x3c2e2b29, 0x2c, 0x47},
    {0x5c8f1c00, 0x6f6f6f6f, 0x0000006f, 0x2819190c, 0x3c2e2b29, 0x2c, 0x47},
    {0x5d8f1d00, 0x6f6f6f6f, 0x0000006f, 0x2819190c, 0x3c2e2b29, 0x2e, 0x4a},
    {0x5d8f1d00, 0x6f6f6f6f, 0x0000006f, 0x2819190c, 0x3c2e2b29, 0x2e, 0x4a},
    {0x5d8f1d00, 0x6f6f6f6f, 0x0000006f, 0x2819190c, 0x3c2e2b29, 0x30, 0x4e},
    {0x5d8f1d00, 0x6f6f6f6f, 0x0000006f, 0x2819190c, 0x3c2e2b29, 0x30, 0x4e},
    {0x5e8f1e00, 0x6f6f6f6f, 0x0000006f, 0x291b1b0f, 0x3e382e2a, 0x32, 0x51},
    {0x5e8f1f00, 0x6f6f6f6f, 0x0000006f, 0x291b1b0f, 0x3e382e2a, 0x34, 0x55},
    {0x5e8f1f00, 0x6f6f6f6f, 0x0000006f, 0x291b1b0f, 0x3e382e2a, 0x34, 0x55},
    {0x5f8f1f00, 0x6f6f6f6f, 0x0000006f, 0x291b1b0f, 0x3e382e2a, 0x36, 0x58},
    {0x688f2800, 0x6f6f6f6f, 0x0000006f, 0x291b1b0f, 0x3e382e2a, 0x38, 0x5b},
    {0x688f2800, 0x6f6f6f6f, 0x0000006f, 0x2b1d1d18, 0x483a382c, 0x3a, 0x5f},
    {0x688f2800, 0x6f6f6f6f, 0x0000006f, 0x2b1d1d18, 0x483a382c, 0x3c, 0x62},
    {0x688f2900, 0x6f6f6f6f, 0x0000006f, 0x2b1d1d18, 0x483a382c, 0x3e, 0x65},
    {0x698f2900, 0x6f6f6f6f, 0x0000006f, 0x2b1d1d18, 0x483a382c, 0x40, 0x69},
    {0x698f2900, 0x6f6f6f6f, 0x0000006f, 0x2c1f1f19, 0x493b392e, 0x43, 0x6c},
    {0x698f2900, 0x6f6f6f6f, 0x0000006f, 0x2c1f1f19, 0x493b392e, 0x45, 0x70},
    {0x6a8f2a00, 0x6f6f6f6f, 0x0000006f, 0x2c1f1f19, 0x493b392e, 0x47, 0x73},
    {0x6a8f2a00, 0x6f6f6f6f, 0x0000006f, 0x2c1f1f19, 0x493b392e, 0x49, 0x76},
    {0x6a8f2a00, 0x6f6f6f6f, 0x0000006f, 0x2e28281b, 0x4b3d3a38, 0x4b, 0x7a},
    {0x6b8f2b00, 0x6f6f6f6f, 0x0000006f, 0x2e28281b, 0x4b3d3a38, 0x4d, 0x7d},
    {0x6b8f2b00, 0x6f6f6f6f, 0x0000006f, 0x2e28281b, 0x4b3d3a38, 0x4d, 0x7d},
    {0x6b8f2b00, 0x6f6f6f6f, 0x0000006f, 0x2e28281b, 0x4b3d3a38, 0x4f, 0x81},
    {0x6b8f2b00, 0x6f6f6f6f, 0x0000006f, 0x2e28281b, 0x4b3d3a38, 0x51, 0x84},
    {0x6b8f2c00, 0x6f6f6f6f, 0x0000006f, 0x2f29291c, 0x4c3e3b38, 0x53, 0x87},
    {0x6c8f2c00, 0x6f6f6f6f, 0x0000006f, 0x2f29291c, 0x4c3e3b38, 0x55, 0x8b},
    {0x6c8f2c00, 0x6f6f6f6f, 0x0000006f, 0x2f29291c, 0x4c3e3b38, 0x57, 0x8e},
    {0x6c8f2c00, 0x6f6f6f6f, 0x0000006f, 0x2f29291c, 0x4c3e3b38, 0x59, 0x91},
    {0x6d8f2d00, 0x6f6f6f6f, 0x0000006f, 0x382a2a1d, 0x4d483c39, 0x5b, 0x95},
    {0x6d8f2d00, 0x6f6f6f6f, 0x0000006f, 0x382a2a1d, 0x4d483c39, 0x5e, 0x98},
    {0x6d8f2d00, 0x6f6f6f6f, 0x0000006f, 0x382a2a1d, 0x4d483c39, 0x60, 0x9c},
    {0x6d8f2d00, 0x6f6f6f6f, 0x0000006f, 0x382a2a1d, 0x4d483c39, 0x60, 0x9c},
    {0x6d8f2e00, 0x6f6f6f6f, 0x0000006f, 0x382a2a1d, 0x4d483c39, 0x62, 0x9f},
    {0x6e8f2e00, 0x6f6f6f6f, 0x0000006f, 0x392b2b1e, 0x4e483e3a, 0x64, 0xa2},
    {0x6e8f2e00, 0x6f6f6f6f, 0x0000006f, 0x392b2b1e, 0x4e483e3a, 0x66, 0xa6},
    {0x6e8f2e00, 0x6f6f6f6f, 0x0000006f, 0x392b2b1e, 0x4e483e3a, 0x68, 0xa9},
    {0x6f8f2f00, 0x6f6f6f6f, 0x0000006f, 0x392b2b1e, 0x4e483e3a, 0x6a, 0xad},
    {0x6f8f2f00, 0x6f6f6f6f, 0x0000006f, 0x3a2c2c1f, 0x4f493f3b, 0x6c, 0xb0},
    {0x6f8f2f00, 0x6f6f6f6f, 0x0000006f, 0x3a2c2c1f, 0x4f493f3b, 0x6e, 0xb3},
    {0x788f3800, 0x6f6f6f6f, 0x0000006f, 0x3a2c2c1f, 0x4f493f3b, 0x70, 0xb7},
    {0x788f3800, 0x6f6f6f6f, 0x0000006f, 0x3a2c2c1f, 0x4f493f3b, 0x72, 0xba},
    {0x788f3800, 0x6f6f6f6f, 0x0000006f, 0x3b2d2d28, 0x584a483c, 0x74, 0xbd},
    {0x788f3800, 0x6f6f6f6f, 0x0000006f, 0x3b2d2d28, 0x584a483c, 0x76, 0xc1},
    {0x788f3800, 0x6f6f6f6f, 0x0000006f, 0x3b2d2d28, 0x584a483c, 0x79, 0xc4},
    {0x788f3800, 0x6f6f6f6f, 0x0000006f, 0x3b2d2d28, 0x584a483c, 0x7b, 0xc8},
    {0x788f3800, 0x6f6f6f6f, 0x0000006f, 0x3b2e2e29, 0x594b483d, 0x7d, 0xcb},
    {0x798f3900, 0x6f6f6f6f, 0x0000006f, 0x3b2e2e29, 0x594b483d, 0x7f, 0xce},
    {0x798f3900, 0x6f6f6f6f, 0x0000006f, 0x3b2e2e29, 0x594b483d, 0x81, 0xd2},
    {0x798f3900, 0x6f6f6f6f, 0x0000006f, 0x3b2e2e29, 0x594b483d, 0x83, 0xd5},
    {0x798f3900, 0x6f6f6f6f, 0x0000006f, 0x3c2f2f29, 0x594b493e, 0x85, 0xd9},
    {0x798f3900, 0x6f6f6f6f, 0x0000006f, 0x3c2f2f29, 0x594b493e, 0x87, 0xdc},
    {0x798f3900, 0x6f6f6f6f, 0x0000006f, 0x3c2f2f29, 0x594b493e, 0x89, 0xdf},
    {0x798f3a00, 0x6f6f6f6f, 0x0000006f, 0x3c2f2f29, 0x594b493e, 0x8b, 0xe3},
    {0x7a8f3a00, 0x6f6f6f6f, 0x0000006f, 0x3d38382a, 0x5a4c493f, 0x8d, 0xe6},
    {0x7a8f3a00, 0x6f6f6f6f, 0x0000006f, 0x3d38382a, 0x5a4c493f, 0x8f, 0xe9},
    {0x7a8f3a00, 0x6f6f6f6f, 0x0000006f, 0x3d38382a, 0x5a4c493f, 0x91, 0xed},
    {0x7a8f3a00, 0x6f6f6f6f, 0x0000006f, 0x3d38382a, 0x5a4c493f, 0x94, 0xf0},
    {0x7a8f3a00, 0x6f6f6f6f, 0x0000006f, 0x3e38382b, 0x5b4d4a48, 0x96, 0xf4},
    {0x7a8f3a00, 0x6f6f6f6f, 0x0000006f, 0x3e38382b, 0x5b4d4a48, 0x98, 0xf7},
    {0x7b8f3b00, 0x6f6f6f6f, 0x0000006f, 0x3e38382b, 0x5b4d4a48, 0x9a, 0xfa},
    {0x7b8f3b00, 0x6f6f6f6f, 0x0000006f, 0x3e38382b, 0x5b4d4a48, 0x9c, 0xfe},
    {0x7b8f3b00, 0x6f6f6f6f, 0x0000006f, 0x3f38392b, 0x5b4d4b48, 0x9e, 0xff},
    {0x7b8f3b00, 0x6f6f6f6f, 0x0000006f, 0x3f38392b, 0x5b4d4b48, 0x9e, 0xff},
    {0x7b8f3b00, 0x6f6f6f6f, 0x0000006f, 0x3f38392b, 0x5b4d4b48, 0xa0, 0xff},
    {0x7b8f3b00, 0x6f6f6f6f, 0x0000006f, 0x3f38392b, 0x5b4d4b48, 0xa2, 0xff},
    {0x7b8f3b00, 0x6f6f6f6f, 0x0000006f, 0x3f38392b, 0x5b4d4b48, 0xa4, 0xff},
    {0x7b8f3b00, 0x6f6f6f6f, 0x0000006f, 0x3f39392c, 0x5c4e4b48, 0xa6, 0xff},
    {0x7c8f3c00, 0x6f6f6f6f, 0x0000006f, 0x3f39392c, 0x5c4e4b48, 0xa8, 0xff},
    {0x7c8f3c00, 0x6f6f6f6f, 0x0000006f, 0x3f39392c, 0x5c4e4b48, 0xaa, 0xff},
    {0x7c8f3c00, 0x6f6f6f6f, 0x0000006f, 0x3f39392c, 0x5c4e4b48, 0xac, 0xff},
    {0x7c8f3c00, 0x6f6f6f6f, 0x0000006f, 0x48393a2c, 0x5c4f4c49, 0xaf, 0xff},
    {0x7c8f3c00, 0x6f6f6f6f, 0x0000006f, 0x48393a2c, 0x5c4f4c49, 0xb1, 0xff},
    {0x7c8f3c00, 0x6f6f6f6f, 0x0000006f, 0x48393a2c, 0x5c4f4c49, 0xb3, 0xff},
    {0x7c8f3c00, 0x6f6f6f6f, 0x0000006f, 0x48393a2c, 0x5c4f4c49, 0xb5, 0xff},
    {0x7d8f3d00, 0x6f6f6f6f, 0x0000006f, 0x483a3a2d, 0x5d584c49, 0xb7, 0xff},
    {0x7d8f3d00, 0x6f6f6f6f, 0x0000006f, 0x483a3a2d, 0x5d584c49, 0xb9, 0xff},
    {0x7d8f3d00, 0x6f6f6f6f, 0x0000006f, 0x483a3a2d, 0x5d584c49, 0xbd, 0xff},
    {0x7d8f3d00, 0x6f6f6f6f, 0x0000006f, 0x493a3b2e, 0x5e584d4a, 0xc1, 0xff},
    {0x7e8f3e00, 0x6f6f6f6f, 0x0000006f, 0x493a3b2e, 0x5e584d4a, 0xc5, 0xff},
    {0x7e8f3e00, 0x6f6f6f6f, 0x0000006f, 0x493b3b2e, 0x5e584e4a, 0xc8, 0xff},
    {0x7e8f3e00, 0x6f6f6f6f, 0x0000006f, 0x493b3b2e, 0x5e584e4a, 0xcc, 0xff},
    {0x7e8f3e00, 0x6f6f6f6f, 0x0000006f, 0x493b3c2f, 0x5f594e4b, 0xd0, 0xff},
    {0x7f8f3f00, 0x6f6f6f6f, 0x0000006f, 0x493b3c2f, 0x5f594e4b, 0xd2, 0xff},
    {0x7f8f3f00, 0x6f6f6f6f, 0x0000006f, 0x493b3c2f, 0x5f594e4b, 0xd4, 0xff},
    {0x7f8f3f00, 0x6f6f6f6f, 0x0000006f, 0x4a3c3c2f, 0x5f594f4b, 0xd8, 0xff},
    {0x7f8f3f00, 0x6f6f6f6f, 0x0000006f, 0x4a3c3c2f, 0x5f594f4b, 0xdc, 0xff},
    {0x888f4800, 0x6f6f6f6f, 0x0000006f, 0x4a3c3d38, 0x68594f4c, 0xe0, 0xff},
    {0x888f4800, 0x6f6f6f6f, 0x0000006f, 0x4a3c3d38, 0x68594f4c, 0xe5, 0xff},
    {0x888f4800, 0x6f6f6f6f, 0x0000006f, 0x4b3d3d38, 0x685a584c, 0xe9, 0xff},
    {0x888f4800, 0x6f6f6f6f, 0x0000006f, 0x4b3d3d38, 0x685a584c, 0xed, 0xff},
    {0x888f4800, 0x6f6f6f6f, 0x0000006f, 0x4b3d3e38, 0x685a584c, 0xf1, 0xff},
    {0x888f4800, 0x6f6f6f6f, 0x0000006f, 0x4b3d3e38, 0x685a584c, 0xf5, 0xff},
    {0x898f4900, 0x6f6f6f6f, 0x0000006f, 0x4b3e3e39, 0x695b584d, 0xfe, 0xff},
    {0x898f4900, 0x6f6f6f6f, 0x0000006f, 0x4c3e3e39, 0x695b594d, 0xff, 0xff},
    {0x898f4900, 0x6f6f6f6f, 0x0000006f, 0x4c3e3e39, 0x695b594d, 0xff, 0xff},
    {0x898f4900, 0x6f6f6f6f, 0x0000006f, 0x4c3f3f39, 0x695b594e, 0xff, 0xff},
    {0x898f4900, 0x6f6f6f6f, 0x0000006f, 0x4c3f3f39, 0x695b594e, 0xff, 0xff},
    {0x898f4900, 0x6f6f6f6f, 0x0000006f, 0x4d3f3f3a, 0x6a5c594e, 0xff, 0xff},
    {0x898f4900, 0x6f6f6f6f, 0x0000006f, 0x4d3f3f3a, 0x6a5c594e, 0xff, 0xff},
    {0x8a8f4a00, 0x6f6f6f6f, 0x0000006f, 0x4d48483a, 0x6a5c594f, 0xff, 0xff},
    {0x8a8f4a00, 0x6f6f6f6f, 0x0000006f, 0x4d48483a, 0x6a5c594f, 0xff, 0xff},
    {0x8a8f4a00, 0x6f6f6f6f, 0x0000006f, 0x4d48483a, 0x6a5c5a4f, 0xff, 0xff},
    {0x8a8f4a00, 0x6f6f6f6f, 0x0000006f, 0x4d48483a, 0x6a5c5a4f, 0xff, 0xff},
    {0x8a8f4a00, 0x6f6f6f6f, 0x0000006f, 0x4e48483a, 0x6a5d5a58, 0xff, 0xff},
    {0x8b8f4b00, 0x6f6f6f6f, 0x0000006f, 0x4e48483b, 0x6b5d5a58, 0xff, 0xff},
    {0x8b8f4b00, 0x6f6f6f6f, 0x0000006f, 0x4e48483b, 0x6b5d5a58, 0xff, 0xff},
    {0x8b8f4b00, 0x6f6f6f6f, 0x0000006f, 0x4f48493b, 0x6b5d5b58, 0xff, 0xff},
    {0x8b8f4b00, 0x6f6f6f6f, 0x0000006f, 0x4f49493b, 0x6b5e5b58, 0xff, 0xff}
};

const uint32_t VP8_SINGLESU[56] =
{
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000
};

const uint8_t VP8_FULLSPIRAL_48x40[56] =
{
    // L -> U -> R -> D
    0x0F,
    0xF0,
    0x01, 0x01,
    0x10, 0x10,
    0x0F, 0x0F, 0x0F,
    0xF0, 0xF0, 0xF0,
    0x01, 0x01, 0x01, 0x01,
    0x10, 0x10, 0x10, 0x10,
    0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
    0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10,      // The last 0x10 steps outside the search window.
    0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, // These are outside the search window.
    0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0
};

const uint8_t VP8_RASTERSCAN_48x40[56] =
{
    0x11, 0x01, 0x01, 0x01,
    0x11, 0x01, 0x01, 0x01,
    0x11, 0x01, 0x01, 0x01,
    0x11, 0x01, 0x01, 0x01,
    0x11, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
};

const uint8_t VP8_DIAMOND[56] =
{
    0x0F, 0xF1, 0x0F, 0x12,//5
    0x0D, 0xE2, 0x22, 0x1E,//9
    0x10, 0xFF, 0xE2, 0x20,//13
    0xFC, 0x06, 0xDD,//16
    0x2E, 0xF1, 0x3F, 0xD3, 0x11, 0x3D, 0xF3, 0x1F,//24
    0xEB, 0xF1, 0xF1, 0xF1,//28
    0x4E, 0x11, 0x12, 0xF2, 0xF1,//33
    0xE0, 0xFF, 0xFF, 0x0D, 0x1F, 0x1F,//39
    0x20, 0x11, 0xCF, 0xF1, 0x05, 0x11,//45
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,//51
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const uint8_t VP8_MAINREF_TABLE_G9[8] =
{
    0, 1, 2, 9, 3, 13, 14, 57
};

const uint8_t VP8_NUM_REFS_G9[8] =
{
    0, 1, 1, 2, 1, 2, 2, 3
};

const uint8_t VP8_BRC_IFRAME_COST_TABLE_G9[128][4] =
{
    { 0x5,     0x5,     0x8,     0x8},
    { 0xa,     0xa,     0xd,     0xd},
    { 0xd,     0xf,     0xf,    0x19},
    {0x19,    0x1a,    0x1a,    0x1a},
    {0x1b,    0x1b,    0x1d,    0x1d},
    {0x1d,    0x1d,    0x1e,    0x1e},
    {0x1e,    0x1e,    0x1f,    0x1f},
    {0x1f,    0x28,    0x28,    0x29},
    {0x29,    0x29,    0x29,    0x2a},
    {0x2a,    0x2b,    0x2b,    0x2b},
    {0x2b,    0x2b,    0x2c,    0x2c},
    {0x2c,    0x2c,    0x2d,    0x2d},
    {0x2e,    0x2e,    0x2e,    0x2e},
    {0x2e,    0x2f,    0x2f,    0x38},
    {0x38,    0x38,    0x38,    0x38},
    {0x38,    0x39,    0x39,    0x39},
    {0x39,    0x39,    0x39,    0x3a},
    {0x3a,    0x3a,    0x3a,    0x3a},
    {0x3a,    0x3a,    0x3a,    0x3b},
    {0x3b,    0x3b,    0x3b,    0x3b},
    {0x3b,    0x3c,    0x3c,    0x3c},
    {0x3c,    0x3c,    0x3c,    0x3c},
    {0x3d,    0x3d,    0x3d,    0x3d},
    {0x3d,    0x3d,    0x3e,    0x3e},
    {0x3e,    0x3e,    0x3f,    0x3f},
    {0x3f,    0x48,    0x48,    0x48},
    {0x48,    0x48,    0x48,    0x48},
    {0x49,    0x49,    0x49,    0x49},
    {0x49,    0x49,    0x4a,    0x4a},
    {0x4a,    0x4a,    0x4a,    0x4a},
    {0x4b,    0x4b,    0x4b,    0x4b},
    {0x4b,    0x4c,    0x4c,    0x4c},
    {0x1f,    0x1f,    0x2b,    0x2b},
    {0x2f,    0x2f,    0x39,    0x39},
    {0x39,    0x3b,    0x3b,    0x3d},
    {0x3d,    0x3f,    0x3f,    0x3f},
    {0x48,    0x48,    0x49,    0x49},
    {0x49,    0x49,    0x4a,    0x4a},
    {0x4a,    0x4a,    0x4b,    0x4b},
    {0x4b,    0x4c,    0x4c,    0x4d},
    {0x4d,    0x4e,    0x4e,    0x4f},
    {0x4f,    0x58,    0x58,    0x58},
    {0x58,    0x58,    0x58,    0x58},
    {0x59,    0x59,    0x59,    0x59},
    {0x5a,    0x5a,    0x5a,    0x5a},
    {0x5a,    0x5b,    0x5b,    0x5b},
    {0x5b,    0x5c,    0x5c,    0x5c},
    {0x5c,    0x5d,    0x5d,    0x5d},
    {0x5d,    0x5e,    0x5e,    0x5e},
    {0x5e,    0x5f,    0x5f,    0x5f},
    {0x5f,    0x68,    0x68,    0x68},
    {0x68,    0x68,    0x68,    0x68},
    {0x68,    0x68,    0x68,    0x68},
    {0x69,    0x69,    0x69,    0x69},
    {0x69,    0x69,    0x69,    0x69},
    {0x6a,    0x6a,    0x6a,    0x6a},
    {0x6a,    0x6a,    0x6b,    0x6b},
    {0x6b,    0x6b,    0x6b,    0x6c},
    {0x6c,    0x6c,    0x6c,    0x6c},
    {0x6d,    0x6d,    0x6d,    0x6d},
    {0x6e,    0x6e,    0x6e,    0x6f},
    {0x6f,    0x6f,    0x6f,    0x6f},
    {0x78,    0x78,    0x78,    0x78},
    {0x78,    0x78,    0x79,    0x79},
    { 0x2,     0x2,     0x3,     0x3},
    { 0x4,     0x4,     0x5,     0x5},
    { 0x5,     0x6,     0x6,     0x7},
    { 0x7,     0x8,     0x8,     0x8},
    { 0x9,     0x9,     0x9,     0x9},
    { 0x9,     0x9,     0xa,     0xa},
    { 0xa,     0xa,     0xb,     0xb},
    { 0xb,     0xc,     0xc,     0xd},
    { 0xd,     0xe,     0xe,     0xf},
    { 0xf,    0x10,    0x10,    0x11},
    {0x11,    0x11,    0x12,    0x12},
    {0x13,    0x13,    0x14,    0x14},
    {0x15,    0x15,    0x16,    0x16},
    {0x16,    0x17,    0x17,    0x18},
    {0x18,    0x19,    0x19,    0x1a},
    {0x1a,    0x1a,    0x1a,    0x1b},
    {0x1b,    0x1c,    0x1c,    0x1d},
    {0x1d,    0x1e,    0x1e,    0x1f},
    {0x1f,    0x20,    0x20,    0x21},
    {0x21,    0x22,    0x22,    0x23},
    {0x23,    0x24,    0x24,    0x24},
    {0x25,    0x25,    0x26,    0x26},
    {0x27,    0x27,    0x28,    0x28},
    {0x29,    0x29,    0x2a,    0x2a},
    {0x2b,    0x2b,    0x2c,    0x2d},
    {0x2e,    0x2f,    0x2f,    0x30},
    {0x31,    0x32,    0x33,    0x34},
    {0x35,    0x36,    0x37,    0x38},
    {0x3a,    0x3b,    0x3c,    0x3d},
    {0x3d,    0x3e,    0x3f,    0x40},
    {0x41,    0x42,    0x43,    0x44},
    {0x46,    0x47,    0x49,    0x4a},
    { 0x9,     0x9,     0xe,     0xe},
    {0x12,    0x12,    0x17,    0x17},
    {0x17,    0x1b,    0x1b,    0x20},
    {0x20,    0x24,    0x24,    0x24},
    {0x29,    0x29,    0x2d,    0x2d},
    {0x2d,    0x2d,    0x32,    0x32},
    {0x32,    0x32,    0x36,    0x36},
    {0x36,    0x3b,    0x3b,    0x3f},
    {0x3f,    0x44,    0x44,    0x48},
    {0x48,    0x4d,    0x4d,    0x51},
    {0x51,    0x51,    0x56,    0x56},
    {0x5a,    0x5a,    0x5f,    0x5f},
    {0x63,    0x63,    0x68,    0x68},
    {0x68,    0x6c,    0x6c,    0x71},
    {0x71,    0x76,    0x76,    0x7a},
    {0x7a,    0x7f,    0x7f,    0x83},
    {0x83,    0x88,    0x88,    0x8c},
    {0x8c,    0x91,    0x91,    0x95},
    {0x95,    0x9a,    0x9a,    0x9e},
    {0x9e,    0xa3,    0xa3,    0xa7},
    {0xa7,    0xac,    0xac,    0xac},
    {0xb0,    0xb0,    0xb5,    0xb5},
    {0xb9,    0xb9,    0xbe,    0xbe},
    {0xc2,    0xc2,    0xc7,    0xc7},
    {0xcb,    0xd0,    0xd4,    0xd9},
    {0xdd,    0xe2,    0xe2,    0xe6},
    {0xeb,    0xf0,    0xf4,    0xf9},
    {0xfd,    0xff,    0xff,    0xff},
    {0xff,    0xff,    0xff,    0xff},
    {0xff,    0xff,    0xff,    0xff},
    {0xff,    0xff,    0xff,    0xff},
    {0xff,    0xff,    0xff,    0xff}
};

const uint32_t VP8_BRC_PFRAME_COST_TABLE_G9[256] =
{
    0x06040402,
    0x06040402,
    0x06040402,
    0x06040402,
    0x0d080805,
    0x0d080805,
    0x0d080805,
    0x0d080805,
    0x0d080805,
    0x190b0c07,
    0x190b0c07,
    0x190b0c07,
    0x190b0c07,
    0x1c0f0f0a,
    0x1c0f0f0a,
    0x1c0f0f0a,
    0x1c0f0f0a,
    0x1c0f0f0a,
    0x2819190c,
    0x2819190c,
    0x2819190c,
    0x2819190c,
    0x2819190c,
    0x2819190c,
    0x2819190c,
    0x2819190c,
    0x291b1b0f,
    0x291b1b0f,
    0x291b1b0f,
    0x291b1b0f,
    0x291b1b0f,
    0x2b1d1d18,
    0x2b1d1d18,
    0x2b1d1d18,
    0x2b1d1d18,
    0x2c1f1f19,
    0x2c1f1f19,
    0x2c1f1f19,
    0x2c1f1f19,
    0x2e28281b,
    0x2e28281b,
    0x2e28281b,
    0x2e28281b,
    0x2e28281b,
    0x2f29291c,
    0x2f29291c,
    0x2f29291c,
    0x2f29291c,
    0x382a2a1d,
    0x382a2a1d,
    0x382a2a1d,
    0x382a2a1d,
    0x382a2a1d,
    0x392b2b1e,
    0x392b2b1e,
    0x392b2b1e,
    0x392b2b1e,
    0x3a2c2c1f,
    0x3a2c2c1f,
    0x3a2c2c1f,
    0x3a2c2c1f,
    0x3b2d2d28,
    0x3b2d2d28,
    0x3b2d2d28,
    0x3b2d2d28,
    0x3b2e2e29,
    0x3b2e2e29,
    0x3b2e2e29,
    0x3b2e2e29,
    0x3c2f2f29,
    0x3c2f2f29,
    0x3c2f2f29,
    0x3c2f2f29,
    0x3d38382a,
    0x3d38382a,
    0x3d38382a,
    0x3d38382a,
    0x3e38382b,
    0x3e38382b,
    0x3e38382b,
    0x3e38382b,
    0x3f38392b,
    0x3f38392b,
    0x3f38392b,
    0x3f38392b,
    0x3f38392b,
    0x3f39392c,
    0x3f39392c,
    0x3f39392c,
    0x3f39392c,
    0x48393a2c,
    0x48393a2c,
    0x48393a2c,
    0x48393a2c,
    0x483a3a2d,
    0x483a3a2d,
    0x483a3a2d,
    0x493a3b2e,
    0x493a3b2e,
    0x493b3b2e,
    0x493b3b2e,
    0x493b3c2f,
    0x493b3c2f,
    0x493b3c2f,
    0x4a3c3c2f,
    0x4a3c3c2f,
    0x4a3c3d38,
    0x4a3c3d38,
    0x4b3d3d38,
    0x4b3d3d38,
    0x4b3d3e38,
    0x4b3d3e38,
    0x4b3e3e39,
    0x4c3e3e39,
    0x4c3e3e39,
    0x4c3f3f39,
    0x4c3f3f39,
    0x4d3f3f3a,
    0x4d3f3f3a,
    0x4d48483a,
    0x4d48483a,
    0x4d48483a,
    0x4d48483a,
    0x4e48483a,
    0x4e48483b,
    0x4e48483b,
    0x4f48493b,
    0x4f49493b,
    0x1a0c0907,
    0x1a0c0907,
    0x1a0c0907,
    0x1a0c0907,
    0x291b190e,
    0x291b190e,
    0x291b190e,
    0x291b190e,
    0x291b190e,
    0x2e281e1a,
    0x2e281e1a,
    0x2e281e1a,
    0x2e281e1a,
    0x392b291e,
    0x392b291e,
    0x392b291e,
    0x392b291e,
    0x392b291e,
    0x3c2e2b29,
    0x3c2e2b29,
    0x3c2e2b29,
    0x3c2e2b29,
    0x3c2e2b29,
    0x3c2e2b29,
    0x3c2e2b29,
    0x3c2e2b29,
    0x3e382e2a,
    0x3e382e2a,
    0x3e382e2a,
    0x3e382e2a,
    0x3e382e2a,
    0x483a382c,
    0x483a382c,
    0x483a382c,
    0x483a382c,
    0x493b392e,
    0x493b392e,
    0x493b392e,
    0x493b392e,
    0x4b3d3a38,
    0x4b3d3a38,
    0x4b3d3a38,
    0x4b3d3a38,
    0x4b3d3a38,
    0x4c3e3b38,
    0x4c3e3b38,
    0x4c3e3b38,
    0x4c3e3b38,
    0x4d483c39,
    0x4d483c39,
    0x4d483c39,
    0x4d483c39,
    0x4d483c39,
    0x4e483e3a,
    0x4e483e3a,
    0x4e483e3a,
    0x4e483e3a,
    0x4f493f3b,
    0x4f493f3b,
    0x4f493f3b,
    0x4f493f3b,
    0x584a483c,
    0x584a483c,
    0x584a483c,
    0x584a483c,
    0x594b483d,
    0x594b483d,
    0x594b483d,
    0x594b483d,
    0x594b493e,
    0x594b493e,
    0x594b493e,
    0x594b493e,
    0x5a4c493f,
    0x5a4c493f,
    0x5a4c493f,
    0x5a4c493f,
    0x5b4d4a48,
    0x5b4d4a48,
    0x5b4d4a48,
    0x5b4d4a48,
    0x5b4d4b48,
    0x5b4d4b48,
    0x5b4d4b48,
    0x5b4d4b48,
    0x5b4d4b48,
    0x5c4e4b48,
    0x5c4e4b48,
    0x5c4e4b48,
    0x5c4e4b48,
    0x5c4f4c49,
    0x5c4f4c49,
    0x5c4f4c49,
    0x5c4f4c49,
    0x5d584c49,
    0x5d584c49,
    0x5d584c49,
    0x5e584d4a,
    0x5e584d4a,
    0x5e584e4a,
    0x5e584e4a,
    0x5f594e4b,
    0x5f594e4b,
    0x5f594e4b,
    0x5f594f4b,
    0x5f594f4b,
    0x68594f4c,
    0x68594f4c,
    0x685a584c,
    0x685a584c,
    0x685a584c,
    0x685a584c,
    0x695b584d,
    0x695b594d,
    0x695b594d,
    0x695b594e,
    0x695b594e,
    0x6a5c594e,
    0x6a5c594e,
    0x6a5c594f,
    0x6a5c594f,
    0x6a5c5a4f,
    0x6a5c5a4f,
    0x6a5d5a58,
    0x6b5d5a58,
    0x6b5d5a58,
    0x6b5d5b58,
    0x6b5e5b58,
};

const uint16_t VP8_MvRefCostContext_G9[6][4][2] =
{
    { { 1328, 10 },
    { 2047, 1 },
    { 2047, 1 },
    { 214, 304 },
    },
    { { 1072, 21 },
    { 979, 27 },
    { 1072, 21 },
    { 321, 201 },
    },
    { { 235, 278 },
    { 511, 107 },
    { 553, 93 },
    { 488, 115 },
    },
    { { 534, 99 },
    { 560, 92 },
    { 255, 257 },
    { 505, 109 },
    },
    { { 174, 361 },
    { 238, 275 },
    { 255, 257 },
    { 744, 53 },
    },
    { { 32, 922 },
    { 113, 494 },
    { 255, 257 },
    { 816, 43 },
    },
};

const uint16_t VP8_QUANT_DC_G9[CODECHAL_VP8_MAX_QP] =
{
    4, 5, 6, 7, 8, 9, 10, 10, 11, 12, 13, 14, 15, 16, 17, 17,
    18, 19, 20, 20, 21, 21, 22, 22, 23, 23, 24, 25, 25, 26, 27, 28,
    29, 30, 31, 32, 33, 34, 35, 36, 37, 37, 38, 39, 40, 41, 42, 43,
    44, 45, 46, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58,
    59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74,
    75, 76, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89,
    91, 93, 95, 96, 98, 100, 101, 102, 104, 106, 108, 110, 112, 114, 116, 118,
    122, 124, 126, 128, 130, 132, 134, 136, 138, 140, 143, 145, 148, 151, 154, 157
};

const uint16_t VP8_QUANT_AC_G9[CODECHAL_VP8_MAX_QP/* + 1 + 32*/] =
{
    /*4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,*/
    4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
    20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
    36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,
    52, 53, 54, 55, 56, 57, 58, 60, 62, 64, 66, 68, 70, 72, 74, 76,
    78, 80, 82, 84, 86, 88, 90, 92, 94, 96, 98, 100, 102, 104, 106, 108,
    110, 112, 114, 116, 119, 122, 125, 128, 131, 134, 137, 140, 143, 146, 149, 152,
    155, 158, 161, 164, 167, 170, 173, 177, 181, 185, 189, 193, 197, 201, 205, 209,
    213, 217, 221, 225, 229, 234, 239, 245, 249, 254, 259, 264, 269, 274, 279, 284/*,
    284, 284, 284, 284, 284, 284, 284, 284, 284, 284, 284, 284, 284, 284, 284, 284*/
};

extern const uint16_t VP8_MB_MODE_COST_LUMA_G9[10] =
{   657,    869,    915,    917,    208,    0,      0,      0,      0,      0};

static const uint16_t VP8_BLOCK_MODE_COST_G9[10][10][10] =
{
    {       {37,  1725,  1868,  1151,  1622,  2096,  2011,  1770,  2218,  2128  },
    {139,  759,  1683,  911,  1455,  1846,  1570,  1295,  1792,  1648   },
    {560,  1383,  408,  639,  1612,  1174,  1562,  1736,  847,  991     },
    {191,  1293,  1299,  466,  1774,  1840,  1784,  1691,  1698,  1505  },
    {211,  1624,  1294,  779,  714,  1622,  2222,  1554,  1706,  903    },
    {297,  1259,  1098,  1062,  1583,  618,  1053,  1889,  851,  1127   },
    {275,  703,  1356,  1111,  1597,  1075,  656,  1529,  1531,  1275   },
    {150,  1046,  1760,  1039,  1353,  1981,  2174,  728,  1730,  1379  },
    {516,  1414,  741,  1045,  1495,  738,  1288,  1619,  442,  1200    },
    {424,  1365,  706,  825,  1197,  1453,  1191,  1462,  1186,  519    },

    },
    {       {393,  515,  1491,  549,  1598,  1524,  964,  1126,  1651,  2172    },
    {693,  237,  1954,  641,  1525,  2073,  1183,  971,  1973,  2235    },
    {560,  739,  855,  836,  1224,  1115,  966,  839,  1076,  767       },
    {657,  368,  1406,  425,  1672,  1853,  1210,  1125,  1969,  1542   },
    {321,  1056,  1776,  774,  803,  3311,  1265,  1177,  1366,  636    },
    {693,  510,  949,  877,  1049,  658,  882,  1178,  1515,  1111      },
    {744,  377,  1278,  958,  1576,  1168,  477,  1146,  1838,  1501    },
    {488,  477,  1767,  973,  1107,  1511,  1773,  486,  1527,  1449    },
    {744,  1004,  695,  1012,  1326,  834,  1215,  774,  724,  704      },
    {522,  567,  1036,  1082,  1039,  1333,  873,  1135,  1189,  677    },

    },
    {       {103,  1441,  1000,  864,  1513,  1928,  1832,  1916,  1663,  1567  },
    {304,  872,  1100,  515,  1416,  1417,  3463,  1051,  1305,  1227   },
    {684,  2176,  242,  729,  1867,  1496,  2056,  1544,  1038,  930    },
    {534,  1198,  669,  300,  1805,  1377,  2165,  1894,  1249,  1153   },
    {346,  1602,  1178,  612,  997,  3381,  1335,  1328,  997,  646     },
    {393,  1027,  649,  813,  1276,  945,  1545,  1278,  875,  1031     },
    {528,  996,  930,  617,  1086,  1190,  621,  2760,  787,  1347      },
    {216,  873,  1595,  738,  1339,  3896,  3898,  743,  1343,  1605    },
    {675,  1580,  543,  749,  1859,  1245,  1589,  2377,  384,  1075    },
    {594,  1163,  415,  684,  1474,  1080,  1491,  1478,  1077,  801    },
    },
    {       {238,  1131,  1483,  398,  1510,  1651,  1495,  1545,  1970,  2090  },
    {499,  456,  1499,  449,  1558,  1691,  1272,  969,  2114,  2116    },
    {675,  1386,  318,  645,  1449,  1588,  1666,  1925,  979,  859     },
    {467,  957,  1223,  238,  1825,  1704,  1608,  1560,  1665,  1376   },
    {331,  1460,  1238,  627,  787,  1882,  3928,  1544,  1897,  579    },
    {457,  1038,  903,  784,  1158,  725,  955,  1517,  842,  1016      },
    {505,  497,  1131,  812,  1508,  1206,  703,  1072,  1254,  1256    },
    {397,  741,  1336,  642,  1506,  1852,  1340,  599,  1854,  1000    },
    {625,  1212,  597,  750,  1291,  1057,  1401,  1401,  527,  954     },
    {499,  1041,  654,  752,  1299,  1217,  1605,  1424,  1377,  505    },
    },
    {       {263,  1094,  1218,  602,  938,  1487,  1231,  1016,  1724,  1448   },
    {452,  535,  1728,  562,  1008,  1471,  1473,  873,  3182,  1136    },
    {553,  1570,  935,  1093,  826,  1339,  879,  1007,  1006,  476     },
    {365,  900,  1050,  582,  866,  1398,  1236,  1123,  1608,  1039    },
    {294,  2044,  1790,  1143,  430,  1642,  3688,  1549,  2080,  704   },
    {703,  1210,  958,  815,  1211,  960,  623,  2455,  815,  559       },
    {675,  574,  862,  1261,  866,  864,  761,  1267,  1014,  936       },
    {342,  1254,  1857,  989,  612,  1856,  1858,  553,  1840,  1037    },
    {553,  1316,  811,  1072,  1068,  728,  1328,  1317,  1064,  475    },
    {288,  1303,  1167,  1167,  823,  1634,  1636,  2497,  1294,  491   },
    },
    {       {227,  1059,  1369,  1066,  1505,  740,  970,  1511,  972,  1775    },
    {516,  587,  1033,  646,  1188,  748,  978,  1445,  1294,  1450     },
    {684,  1048,  663,  747,  1126,  826,  1386,  1128,  635,  924      },
    {494,  814,  933,  510,  1606,  951,  878,  1344,  1031,  1347      },
    {553,  1071,  1327,  726,  809,  3376,  1330,  1324,  1062,  407    },
    {625,  1120,  988,  1121,  1197,  347,  1064,  1308,  862,  1206    },
    {633,  853,  1657,  1073,  1662,  634,  460,  1405,  811,  1155     },
    {505,  621,  1394,  876,  1394,  876,  878,  795,  878,  1399       },
    {684,  1302,  968,  1704,  1280,  561,  972,  1713,  387,  1104     },
    {397,  1447,  1060,  867,  957,  1058,  749,  1475,  1210,  660     },
    },
    {       {331,  933,  1647,  761,  1647,  998,  513,  1402,  1461,  2219     },
    {573,  485,  1968,  641,  1570,  1198,  588,  1086,  1382,  1982    },
    {790,  942,  570,  790,  1607,  1005,  938,  1193,  714,  751       },
    {511,  745,  1152,  492,  1878,  1206,  596,  1867,  1617,  1157    },
    {452,  1308,  896,  896,  451,  1308,  3354,  1301,  1306,  794     },
    {693,  670,  1072,  1020,  1687,  566,  488,  1432,  1096,  3142    },
    {778,  566,  1993,  1283,  3139,  1251,  227,  1378,  1784,  1447   },
    {393,  937,  1091,  934,  939,  1348,  1092,  579,  1351,  1095     },
    {560,  1013,  1007,  1014,  1011,  644,  1165,  1155,  605,  1016   },
    {567,  627,  997,  793,  2562,  998,  849,  1260,  922,  748        },
    },
    {       {338,  762,  1868,  717,  1247,  1757,  1263,  535,  1751,  2162    },
    {488,  442,  3235,  756,  1658,  1814,  1264,  528,  1857,  2119    },
    {522,  1087,  840,  1103,  843,  1354,  1098,  888,  946,  588      },
    {483,  688,  1502,  651,  1213,  1446,  1397,  491,  1908,  1253    },
    {452,  1386,  1910,  1175,  298,  1507,  3553,  930,  1904,  905    },
    {713,  839,  716,  715,  932,  719,  931,  848,  3088,  1042        },
    {516,  495,  1331,  1340,  1331,  1069,  665,  702,  1593,  1337    },
    {401,  977,  2167,  1537,  1069,  1764,  3810,  259,  3624,  1578   },
    {560,  1104,  601,  1371,  965,  658,  2704,  779,  967,  969       },
    {547,  1057,  801,  1141,  1133,  1397,  937,  605,  1252,  631     },
    },
    {       {163,  1240,  925,  983,  1653,  1321,  1353,  1566,  946,  1601    },
    {401,  726,  758,  836,  1241,  926,  1656,  795,  1394,  1396      },
    {905,  1073,  366,  876,  1436,  1576,  1732,  2432,  459,  1019    },
    {594,  922,  835,  417,  1387,  1124,  1098,  2042,  843,  1023     },
    {415,  1262,  860,  1274,  758,  1272,  3318,  1010,  1276,  503    },
    {641,  1018,  1020,  1095,  1619,  667,  1371,  2348,  397,  849    },
    {560,  817,  903,  1014,  1420,  695,  756,  904,  821,  1421       },
    {406,  596,  1001,  993,  1257,  1258,  1260,  746,  1002,  1264    },
    {979,  1371,  780,  1188,  1693,  1024,  1286,  1699,  183,  1405   },
    {733,  1292,  458,  884,  1554,  889,  1151,  1286,  738,  740      },
    },
    {       {109,  1377,  1177,  933,  1140,  1928,  1639,  1705,  1861,  1292  },
    {342,  570,  1081,  638,  1154,  1231,  1339,  1342,  1750,  1494   },
    {560,  1203,  345,  767,  1325,  1681,  1425,  1905,  1205,  786    },
    {406,  1027,  1011,  410,  1306,  1901,  1389,  1636,  1493,  776   },
    {206,  1329,  1337,  1037,  802,  1600,  3646,  1451,  1603,  693   },
    {472,  1167,  758,  911,  1424,  703,  2749,  1428,  703,  764      },
    {342,  780,  1139,  889,  1290,  1139,  781,  1544,  957,  1042     },
    {227,  888,  1039,  929,  988,  3753,  1707,  818,  1710,  1306     },
    {767,  1055,  627,  725,  1312,  980,  1065,  1324,  599,  811      },
    {304,  1372,  888,  1173,  979,  1578,  1580,  1974,  1318,  482    },
    }
};

// VP8 BRC
extern const uint8_t VP8_BRC_IFRAME_COST_TABLE_G9[128][4];
extern const uint32_t VP8_BRC_PFRAME_COST_TABLE_G9[256];
extern const uint32_t VP8_NewMVSkipThreshold_G9[128];

const uint8_t VP8_BRC_QPAdjustment_DistThreshold_MaxFrameThreshold_DistQPAdjustment_IPB_G9[576] =
{
    0x01, 0x03, 0x05, 0x07, 0x09, 0x01, 0x02, 0x03, 0x05, 0x07, 0x00, 0x00, 0x01, 0x02, 0x04, 0x00,
    0x00, 0x00, 0x01, 0x02, 0xff, 0x00, 0x00, 0x00, 0x01, 0xfd, 0xfe, 0xff, 0x00, 0x00, 0xfb, 0xfc,
    0xfe, 0xff, 0x00, 0xf9, 0xfa, 0xfc, 0xfe, 0xff, 0xf7, 0xf9, 0xfb, 0xfe, 0xff, 0x00, 0x04, 0x1e,
    0x3c, 0x50, 0x78, 0x8c, 0xc8, 0xff, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x02, 0x05, 0x08, 0x0a, 0x01, 0x02, 0x04, 0x06, 0x08, 0x00, 0x01, 0x02, 0x04, 0x06, 0x00,
    0x00, 0x00, 0x01, 0x02, 0xff, 0x00, 0x00, 0x00, 0x01, 0xfe, 0xff, 0xff, 0x00, 0x00, 0xfd, 0xfe,
    0xff, 0xff, 0x00, 0xfb, 0xfd, 0xfe, 0xff, 0x00, 0xf9, 0xfa, 0xfc, 0xfe, 0xff, 0x00, 0x04, 0x1e,
    0x3c, 0x50, 0x78, 0x8c, 0xc8, 0xff, 0x04, 0x05, 0x06, 0x06, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x02, 0x05, 0x08, 0x0a, 0x01, 0x02, 0x04, 0x06, 0x08, 0x00, 0x01, 0x02, 0x04, 0x06, 0x00,
    0x00, 0x00, 0x01, 0x02, 0xff, 0x00, 0x00, 0x00, 0x01, 0xfe, 0xff, 0xff, 0x00, 0x00, 0xfd, 0xfe,
    0xff, 0xff, 0x00, 0xfb, 0xfd, 0xfe, 0xff, 0x00, 0xf9, 0xfa, 0xfc, 0xfe, 0xff, 0x00, 0x02, 0x14,
    0x28, 0x46, 0x82, 0xa0, 0xc8, 0xff, 0x04, 0x05, 0x06, 0x06, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x06, 0x08, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x05,
    0x07, 0x09, 0xff, 0x00, 0x00, 0x00, 0x00, 0x03, 0x04, 0x06, 0x07, 0xfe, 0xff, 0x00, 0x00, 0x00,
    0x01, 0x02, 0x03, 0x05, 0xfd, 0xfe, 0xff, 0x00, 0x00, 0x00, 0x01, 0x03, 0x05, 0xfc, 0xfe, 0xff,
    0x00, 0x00, 0x00, 0x01, 0x03, 0x05, 0xfb, 0xfd, 0xfe, 0xff, 0x00, 0x00, 0x01, 0x03, 0x05, 0xfa,
    0xfc, 0xfe, 0xff, 0x00, 0x00, 0x01, 0x03, 0x05, 0xfa, 0xfc, 0xfe, 0xff, 0x00, 0x00, 0x01, 0x03,
    0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x05, 0x07, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x05,
    0x06, 0x08, 0xff, 0x00, 0x00, 0x00, 0x00, 0x03, 0x05, 0x07, 0x08, 0xfe, 0xff, 0x00, 0x00, 0x00,
    0x02, 0x04, 0x05, 0x06, 0xfd, 0xfe, 0xff, 0x00, 0x00, 0x00, 0x01, 0x04, 0x05, 0xfc, 0xfe, 0xff,
    0x00, 0x00, 0x00, 0x01, 0x04, 0x05, 0xfc, 0xfe, 0xff, 0xff, 0x00, 0x00, 0x00, 0x04, 0x05, 0xfc,
    0xfd, 0xfe, 0xff, 0x00, 0x00, 0x00, 0x01, 0x05, 0xfb, 0xfc, 0xfe, 0xff, 0x00, 0x00, 0x00, 0x01,
    0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x05, 0x07, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x05,
    0x06, 0x08, 0xff, 0x00, 0x00, 0x00, 0x00, 0x03, 0x05, 0x07, 0x08, 0xfe, 0xff, 0x00, 0x00, 0x00,
    0x02, 0x04, 0x05, 0x06, 0xfd, 0xfe, 0xff, 0x00, 0x00, 0x00, 0x01, 0x04, 0x05, 0xfc, 0xfe, 0xff,
    0x00, 0x00, 0x00, 0x01, 0x04, 0x05, 0xfc, 0xfe, 0xff, 0xff, 0x00, 0x00, 0x00, 0x04, 0x05, 0xfc,
    0xfd, 0xfe, 0xff, 0x00, 0x00, 0x00, 0x01, 0x05, 0xfb, 0xfc, 0xfe, 0xff, 0x00, 0x00, 0x00, 0x01,
    0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const uint16_t VP8_BRC_QUANT_DC_TABLE [128] =
{
      4,   5,   6,   7,   8,   9,  10,  10,  11,  12,  13,  14,  15,  16,  17,  17,
     18,  19,  20,  20,  21,  21,  22,  22,  23,  23,  24,  25,  25,  26,  27,  28,
     29,  30,  31,  32,  33,  34,  35,  36,  37,  37,  38,  39,  40,  41,  42,  43,
     44,  45,  46,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,
     59,  60,  61,  62,  63,  64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,
     75,  76,  76,  77,  78,  79,  80,  81,  82,  83,  84,  85,  86,  87,  88,  89,
     91,  93,  95,  96,  98, 100, 101, 102, 104, 106, 108, 110, 112, 114, 116, 118,
    122, 124, 126, 128, 130, 132, 134, 136, 138, 140, 143, 145, 148, 151, 154, 157
};

const uint16_t VP8_BRC_QUANT_AC_TABLE [128] =
{
      4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,  16,  17,  18,  19,
     20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,
     36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51,
     52,  53,  54,  55,  56,  57,  58,  60,  62,  64,  66,  68,  70,  72,  74,  76,
     78,  80,  82,  84,  86,  88,  90,  92,  94,  96,  98, 100, 102, 104, 106, 108,
    110, 112, 114, 116, 119, 122, 125, 128, 131, 134, 137, 140, 143, 146, 149, 152,
    155, 158, 161, 164, 167, 170, 173, 177, 181, 185, 189, 193, 197, 201, 205, 209,
    213, 217, 221, 225, 229, 234, 239, 245, 249, 254, 259, 264, 269, 274, 279, 284
};

const uint16_t VP8_BRC_SKIP_MV_THRESHOLD_TABLE [256] =
{
    111,  120,  129,  137,  146,  155,  163,  172,  180,  189,  198,  206,  215,  224,  232,  241,
    249,  258,  267,  275,  284,  293,  301,  310,  318,  327,  336,  344,  353,  362,  370,  379,
    387,  396,  405,  413,  422,  431,  439,  448,  456,  465,  474,  482,  491,  500,  508,  517,
    525,  534,  543,  551,  560,  569,  577,  586,  594,  603,  612,  620,  629,  638,  646,  655,
    663,  672,  681,  689,  698,  707,  715,  724,  733,  741,  750,  758,  767,  776,  784,  793,
    802,  810,  819,  827,  836,  845,  853,  862,  871,  879,  888,  896,  905,  914,  922,  931,
    940,  948,  957,  965,  974,  983,  991, 1000, 1009, 1017, 1026, 1034, 1043, 1052, 1060, 1069,
    1078,1086, 1095, 1103, 1112, 1121, 1129, 1138, 1147, 1155, 1164, 1172, 1181, 1190, 1198, 1208
};

extern const uint32_t VP8_MODE_MV_COST_TABLE_G9[128][5] =
{
    {0x008f0000, 0x006f0000, 0x00000000, 0x09060400, 0x000e000a},
    {0x008f0000, 0x006f0000, 0x00000000, 0x09000400, 0x0f000c00},
    {0x008f0000, 0x006f0000, 0x00000000, 0x09000400, 0x0f000c00},
    {0x008f0000, 0x006f0000, 0x00000000, 0x09000400, 0x0f000c00},
    {0x008f0000, 0x006f0000, 0x00000000, 0x12000900, 0x1e001800},
    {0x008f0000, 0x006f0000, 0x00000000, 0x12000900, 0x1e001800},
    {0x008f0000, 0x006f0000, 0x00000000, 0x12000900, 0x1e001800},
    {0x008f0000, 0x006f0000, 0x00000000, 0x12000900, 0x1e001800},
    {0x008f0000, 0x006f0000, 0x00000000, 0x12000900, 0x1e001800},
    {0x008f0000, 0x006f0000, 0x00000000, 0x1b000d00, 0x2d002400},
    {0x008f0000, 0x006f0000, 0x00000000, 0x1b000d00, 0x2d002400},
    {0x008f0000, 0x006f0000, 0x00000000, 0x1b000d00, 0x2d002400},
    {0x468f0000, 0x006f0200, 0x05000000, 0x00000000, 0x00000000},
    {0x468f0000, 0x006f0200, 0x05000000, 0x00000000, 0x00000000},
    {0x468f0000, 0x006f0200, 0x05000000, 0x00000000, 0x00000000},
    {0x468f0000, 0x006f0200, 0x05000000, 0x00000000, 0x00000000},
    {0x468f0000, 0x006f0200, 0x05000000, 0x00000000, 0x00000000},
    {0x468f0000, 0x006f0200, 0x05000000, 0x00000000, 0x00000000},
    {0x468f0000, 0x006f0200, 0x05000000, 0x00000000, 0x00000000},
    {0x468f0000, 0x006f0200, 0x05000000, 0x00000000, 0x00000000},
    {0x468f0000, 0x006f0200, 0x05000000, 0x00000000, 0x00000000},
    {0x468f0000, 0x006f0200, 0x05000000, 0x00000000, 0x00000000},
    {0x468f0000, 0x006f0200, 0x05000000, 0x00000000, 0x00000000},
    {0x468f0000, 0x006f0200, 0x05000000, 0x00000000, 0x00000000},
    {0x468f0000, 0x006f0200, 0x05000000, 0x00000000, 0x00000000},
    {0x468f0000, 0x006f0200, 0x05000000, 0x00000000, 0x00000000},
    {0x468f0000, 0x006f0200, 0x05000000, 0x00000000, 0x00000000},
    {0x468f0000, 0x006f0200, 0x05000000, 0x00000000, 0x00000000},
    {0x468f0000, 0x006f0200, 0x05000000, 0x00000000, 0x00000000},
    {0x468f0000, 0x006f0200, 0x05000000, 0x00000000, 0x00000000},
    {0x468f0000, 0x006f0200, 0x05000000, 0x00000000, 0x00000000},
    {0x468f0000, 0x006f0200, 0x05000000, 0x00000000, 0x00000000},
    {0x6f8f0000, 0x006f0500, 0x0b000000, 0x00000000, 0x00000000},
    {0x6f8f0000, 0x006f0500, 0x0b000000, 0x00000000, 0x00000000},
    {0x6f8f0000, 0x006f0500, 0x0b000000, 0x00000000, 0x00000000},
    {0x6f8f0000, 0x006f0500, 0x0b000000, 0x00000000, 0x00000000},
    {0x6f8f0000, 0x006f0500, 0x0b000000, 0x00000000, 0x00000000},
    {0x6f8f0000, 0x006f0500, 0x0b000000, 0x00000000, 0x00000000},
    {0x6f8f0000, 0x006f0500, 0x0b000000, 0x00000000, 0x00000000},
    {0x6f8f0000, 0x006f0500, 0x0b000000, 0x00000000, 0x00000000},
    {0x6f8f0000, 0x006f0500, 0x0b000000, 0x00000000, 0x00000000},
    {0x6f8f0000, 0x006f0500, 0x0b000000, 0x00000000, 0x00000000},
    {0x6f8f0000, 0x006f0500, 0x0b000000, 0x00000000, 0x00000000},
    {0x6f8f0000, 0x006f0500, 0x0b000000, 0x00000000, 0x00000000},
    {0x6f8f0000, 0x006f0500, 0x0b000000, 0x00000000, 0x00000000},
    {0x6f8f0000, 0x006f0500, 0x0b000000, 0x00000000, 0x00000000},
    {0x6f8f0000, 0x006f0500, 0x0b000000, 0x00000000, 0x00000000},
    {0x6f8f0010, 0x006f0700, 0x18000000, 0x00000000, 0x00000000},
    {0x6f8f0010, 0x006f0700, 0x18000000, 0x00000000, 0x00000000},
    {0x6f8f0010, 0x006f0700, 0x18000000, 0x00000000, 0x00000000},
    {0x6f8f0010, 0x006f0700, 0x18000000, 0x00000000, 0x00000000},
    {0x6f8f0010, 0x006f0700, 0x18000000, 0x00000000, 0x00000000},
    {0x6f8f0010, 0x006f0700, 0x18000000, 0x00000000, 0x00000000},
    {0x6f8f0010, 0x006f0700, 0x18000000, 0x00000000, 0x00000000},
    {0x6f8f0010, 0x006f0700, 0x18000000, 0x00000000, 0x00000000},
    {0x6f8f0010, 0x006f0700, 0x18000000, 0x00000000, 0x00000000},
    {0x6f8f0010, 0x006f0700, 0x18000000, 0x00000000, 0x00000000},
    {0x6f8f0010, 0x006f0700, 0x18000000, 0x00000000, 0x00000000},
    {0x6f8f0010, 0x006f0700, 0x18000000, 0x00000000, 0x00000000},
    {0x6f8f0010, 0x006f0700, 0x18000000, 0x00000000, 0x00000000},
    {0x6f8f0010, 0x006f0700, 0x18000000, 0x00000000, 0x00000000},
    {0x6f8f0010, 0x006f0700, 0x18000000, 0x00000000, 0x00000000},
    {0x188f0010, 0x006f0a00, 0x16000000, 0x00000000, 0x00000000},
    {0x188f0010, 0x006f0a00, 0x16000000, 0x00000000, 0x00000000},
    {0x188f0010, 0x006f0a00, 0x16000000, 0x00000000, 0x00000000},
    {0x188f0010, 0x006f0a00, 0x16000000, 0x00000000, 0x00000000},
    {0x188f0010, 0x006f0a00, 0x16000000, 0x00000000, 0x00000000},
    {0x188f0010, 0x006f0a00, 0x16000000, 0x00000000, 0x00000000},
    {0x188f0010, 0x006f0a00, 0x16000000, 0x00000000, 0x00000000},
    {0x188f0010, 0x006f0a00, 0x16000000, 0x00000000, 0x00000000},
    {0x188f0010, 0x006f0a00, 0x16000000, 0x00000000, 0x00000000},
    {0x188f0010, 0x006f0a00, 0x16000000, 0x00000000, 0x00000000},
    {0x188f0010, 0x006f0a00, 0x16000000, 0x00000000, 0x00000000},
    {0x188f0010, 0x006f0a00, 0x16000000, 0x00000000, 0x00000000},
    {0x188f0010, 0x006f0a00, 0x16000000, 0x00000000, 0x00000000},
    {0x188f0010, 0x006f0a00, 0x16000000, 0x00000000, 0x00000000},
    {0x5e8f0020, 0x006f0d00, 0x1b000000, 0x00000000, 0x00000000},
    {0x5e8f0020, 0x006f0d00, 0x1b000000, 0x00000000, 0x00000000},
    {0x5e8f0020, 0x006f0d00, 0x1b000000, 0x00000000, 0x00000000},
    {0x5e8f0020, 0x006f0d00, 0x1b000000, 0x00000000, 0x00000000},
    {0x5e8f0020, 0x006f0d00, 0x1b000000, 0x00000000, 0x00000000},
    {0x5e8f0020, 0x006f0d00, 0x1b000000, 0x00000000, 0x00000000},
    {0x5e8f0020, 0x006f0d00, 0x1b000000, 0x00000000, 0x00000000},
    {0x5e8f0020, 0x006f0d00, 0x1b000000, 0x00000000, 0x00000000},
    {0x5e8f0020, 0x006f0d00, 0x1b000000, 0x00000000, 0x00000000},
    {0x5e8f0020, 0x006f0d00, 0x1b000000, 0x00000000, 0x00000000},
    {0x5e8f0020, 0x006f0d00, 0x1b000000, 0x00000000, 0x00000000},
    {0x5e8f0020, 0x006f0d00, 0x1b000000, 0x00000000, 0x00000000},
    {0x5e8f0020, 0x006f0d00, 0x1b000000, 0x00000000, 0x00000000},
    {0x5e8f0020, 0x006f0d00, 0x1b000000, 0x00000000, 0x00000000},
    {0x5e8f0020, 0x006f0d00, 0x1b000000, 0x00000000, 0x00000000},
    {0x6f8f0020, 0x006f0f00, 0x21000000, 0x00000000, 0x00000000},
    {0x6f8f0020, 0x006f0f00, 0x21000000, 0x00000000, 0x00000000},
    {0x6f8f0020, 0x006f0f00, 0x21000000, 0x00000000, 0x00000000},
    {0x6f8f0020, 0x006f0f00, 0x21000000, 0x00000000, 0x00000000},
    {0x6f8f0020, 0x006f0f00, 0x21000000, 0x00000000, 0x00000000},
    {0x6f8f0020, 0x006f0f00, 0x21000000, 0x00000000, 0x00000000},
    {0x6f8f0020, 0x006f0f00, 0x21000000, 0x00000000, 0x00000000},
    {0x6f8f0020, 0x006f0f00, 0x21000000, 0x00000000, 0x00000000},
    {0x6f8f0020, 0x006f0f00, 0x21000000, 0x00000000, 0x00000000},
    {0x6f8f0020, 0x006f0f00, 0x21000000, 0x00000000, 0x00000000},
    {0xea8f0030, 0x006f1200, 0x26000000, 0x00000000, 0x00000000},
    {0xea8f0030, 0x006f1200, 0x26000000, 0x00000000, 0x00000000},
    {0xea8f0030, 0x006f1200, 0x26000000, 0x00000000, 0x00000000},
    {0xea8f0030, 0x006f1200, 0x26000000, 0x00000000, 0x00000000},
    {0xea8f0030, 0x006f1200, 0x26000000, 0x00000000, 0x00000000},
    {0xea8f0030, 0x006f1200, 0x26000000, 0x00000000, 0x00000000},
    {0xea8f0030, 0x006f1200, 0x26000000, 0x00000000, 0x00000000},
    {0xea8f0030, 0x006f1200, 0x26000000, 0x00000000, 0x00000000},
    {0x388f0030, 0x006f1500, 0x2c000000, 0x00000000, 0x00000000},
    {0x388f0030, 0x006f1500, 0x2c000000, 0x00000000, 0x00000000},
    {0x388f0030, 0x006f1500, 0x2c000000, 0x00000000, 0x00000000},
    {0x388f0030, 0x006f1500, 0x2c000000, 0x00000000, 0x00000000},
    {0x388f0030, 0x006f1500, 0x2c000000, 0x00000000, 0x00000000},
    {0x388f0030, 0x006f1500, 0x2c000000, 0x00000000, 0x00000000},
    {0x768f0030, 0x006f1700, 0x31000000, 0x00000000, 0x00000000},
    {0x768f0030, 0x006f1700, 0x31000000, 0x00000000, 0x00000000},
    {0x768f0030, 0x006f1700, 0x31000000, 0x00000000, 0x00000000},
    {0x768f0030, 0x006f1700, 0x31000000, 0x00000000, 0x00000000},
    {0x768f0030, 0x006f1700, 0x31000000, 0x00000000, 0x00000000},
    {0x768f0030, 0x006f1700, 0x31000000, 0x00000000, 0x00000000},
    {0x768f0030, 0x006f1700, 0x31000000, 0x00000000, 0x00000000},
    {0x6f8f0040, 0x006f1a00, 0x37000000, 0x00000000, 0x00000000},
    {0x6f8f0040, 0x006f1a00, 0x37000000, 0x00000000, 0x00000000},
    {0x6f8f0040, 0x006f1a00, 0x37000000, 0x00000000, 0x00000000},
    {0x6f8f0040, 0x006f1a00, 0x37000000, 0x00000000, 0x00000000},
    {0x6f8f0040, 0x006f1a00, 0x37000000, 0x00000000, 0x00000000},
    {0x028f0040, 0x006f1d00, 0x3c000000, 0x00000000, 0x00000000},
};

extern const uint8_t VP8_DEFAULT_COEFF_PROBS_G9[4][8][3][11] =
{
    {
        {
            { 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128},
            { 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128},
            { 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128}
        },
        {
            { 253, 136, 254, 255, 228, 219, 128, 128, 128, 128, 128},
            { 189, 129, 242, 255, 227, 213, 255, 219, 128, 128, 128},
            { 106, 126, 227, 252, 214, 209, 255, 255, 128, 128, 128}
        },
        {
            { 1, 98, 248, 255, 236, 226, 255, 255, 128, 128, 128},
            { 181, 133, 238, 254, 221, 234, 255, 154, 128, 128, 128},
            { 78, 134, 202, 247, 198, 180, 255, 219, 128, 128, 128}
        },
        {
            { 1, 185, 249, 255, 243, 255, 128, 128, 128, 128, 128},
            { 184, 150, 247, 255, 236, 224, 128, 128, 128, 128, 128},
            { 77, 110, 216, 255, 236, 230, 128, 128, 128, 128, 128}
        },
        {
            { 1, 101, 251, 255, 241, 255, 128, 128, 128, 128, 128},
            { 170, 139, 241, 252, 236, 209, 255, 255, 128, 128, 128},
            { 37, 116, 196, 243, 228, 255, 255, 255, 128, 128, 128}
        },
        {
            { 1, 204, 254, 255, 245, 255, 128, 128, 128, 128, 128},
            { 207, 160, 250, 255, 238, 128, 128, 128, 128, 128, 128},
            { 102, 103, 231, 255, 211, 171, 128, 128, 128, 128, 128}
        },
        {
            { 1, 152, 252, 255, 240, 255, 128, 128, 128, 128, 128},
            { 177, 135, 243, 255, 234, 225, 128, 128, 128, 128, 128},
            { 80, 129, 211, 255, 194, 224, 128, 128, 128, 128, 128}
        },
        {
            { 1, 1, 255, 128, 128, 128, 128, 128, 128, 128, 128},
            { 246, 1, 255, 128, 128, 128, 128, 128, 128, 128, 128},
            { 255, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128}
        }
    },
    {
        {
            { 198, 35, 237, 223, 193, 187, 162, 160, 145, 155, 62},
            { 131, 45, 198, 221, 172, 176, 220, 157, 252, 221, 1},
            { 68, 47, 146, 208, 149, 167, 221, 162, 255, 223, 128}
        },
        {
            { 1, 149, 241, 255, 221, 224, 255, 255, 128, 128, 128},
            { 184, 141, 234, 253, 222, 220, 255, 199, 128, 128, 128},
            { 81, 99, 181, 242, 176, 190, 249, 202, 255, 255, 128}
        },
        {
            { 1, 129, 232, 253, 214, 197, 242, 196, 255, 255, 128},
            { 99, 121, 210, 250, 201, 198, 255, 202, 128, 128, 128},
            { 23, 91, 163, 242, 170, 187, 247, 210, 255, 255, 128}
        },
        {
            { 1, 200, 246, 255, 234, 255, 128, 128, 128, 128, 128},
            { 109, 178, 241, 255, 231, 245, 255, 255, 128, 128, 128},
            { 44, 130, 201, 253, 205, 192, 255, 255, 128, 128, 128}
        },
        {
            { 1, 132, 239, 251, 219, 209, 255, 165, 128, 128, 128},
            { 94, 136, 225, 251, 218, 190, 255, 255, 128, 128, 128},
            { 22, 100, 174, 245, 186, 161, 255, 199, 128, 128, 128}
        },
        {
            { 1, 182, 249, 255, 232, 235, 128, 128, 128, 128, 128},
            { 124, 143, 241, 255, 227, 234, 128, 128, 128, 128, 128},
            { 35, 77, 181, 251, 193, 211, 255, 205, 128, 128, 128}
        },
        {
            { 1, 157, 247, 255, 236, 231, 255, 255, 128, 128, 128},
            { 121, 141, 235, 255, 225, 227, 255, 255, 128, 128, 128},
            { 45, 99, 188, 251, 195, 217, 255, 224, 128, 128, 128}
        },
        {
            { 1, 1, 251, 255, 213, 255, 128, 128, 128, 128, 128},
            { 203, 1, 248, 255, 255, 128, 128, 128, 128, 128, 128},
            { 137, 1, 177, 255, 224, 255, 128, 128, 128, 128, 128}
        }
    },
    {
        {
            { 253, 9, 248, 251, 207, 208, 255, 192, 128, 128, 128},
            { 175, 13, 224, 243, 193, 185, 249, 198, 255, 255, 128},
            { 73, 17, 171, 221, 161, 179, 236, 167, 255, 234, 128}
        },
        {
            { 1, 95, 247, 253, 212, 183, 255, 255, 128, 128, 128},
            { 239, 90, 244, 250, 211, 209, 255, 255, 128, 128, 128},
            { 155, 77, 195, 248, 188, 195, 255, 255, 128, 128, 128}
        },
        {
            { 1, 24, 239, 251, 218, 219, 255, 205, 128, 128, 128},
            { 201, 51, 219, 255, 196, 186, 128, 128, 128, 128, 128},
            { 69, 46, 190, 239, 201, 218, 255, 228, 128, 128, 128}
        },
        {
            { 1, 191, 251, 255, 255, 128, 128, 128, 128, 128, 128},
            { 223, 165, 249, 255, 213, 255, 128, 128, 128, 128, 128},
            { 141, 124, 248, 255, 255, 128, 128, 128, 128, 128, 128}
        },
        {
            { 1, 16, 248, 255, 255, 128, 128, 128, 128, 128, 128},
            { 190, 36, 230, 255, 236, 255, 128, 128, 128, 128, 128},
            { 149, 1, 255, 128, 128, 128, 128, 128, 128, 128, 128}
        },
        {
            { 1, 226, 255, 128, 128, 128, 128, 128, 128, 128, 128},
            { 247, 192, 255, 128, 128, 128, 128, 128, 128, 128, 128},
            { 240, 128, 255, 128, 128, 128, 128, 128, 128, 128, 128}
        },
        {
            { 1, 134, 252, 255, 255, 128, 128, 128, 128, 128, 128},
            { 213, 62, 250, 255, 255, 128, 128, 128, 128, 128, 128},
            { 55, 93, 255, 128, 128, 128, 128, 128, 128, 128, 128}
        },
        {
            { 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128},
            { 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128},
            { 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128}
        }
    },
    {
        {
            { 202, 24, 213, 235, 186, 191, 220, 160, 240, 175, 255},
            { 126, 38, 182, 232, 169, 184, 228, 174, 255, 187, 128},
            { 61, 46, 138, 219, 151, 178, 240, 170, 255, 216, 128}
        },
        {
            { 1, 112, 230, 250, 199, 191, 247, 159, 255, 255, 128},
            { 166, 109, 228, 252, 211, 215, 255, 174, 128, 128, 128},
            { 39, 77, 162, 232, 172, 180, 245, 178, 255, 255, 128}
        },
        {
            { 1, 52, 220, 246, 198, 199, 249, 220, 255, 255, 128},
            { 124, 74, 191, 243, 183, 193, 250, 221, 255, 255, 128},
            { 24, 71, 130, 219, 154, 170, 243, 182, 255, 255, 128}
        },
        {
            { 1, 182, 225, 249, 219, 240, 255, 224, 128, 128, 128},
            { 149, 150, 226, 252, 216, 205, 255, 171, 128, 128, 128},
            { 28, 108, 170, 242, 183, 194, 254, 223, 255, 255, 128}
        },
        {
            { 1, 81, 230, 252, 204, 203, 255, 192, 128, 128, 128},
            { 123, 102, 209, 247, 188, 196, 255, 233, 128, 128, 128},
            { 20, 95, 153, 243, 164, 173, 255, 203, 128, 128, 128}
        },
        {
            { 1, 222, 248, 255, 216, 213, 128, 128, 128, 128, 128},
            { 168, 175, 246, 252, 235, 205, 255, 255, 128, 128, 128},
            { 47, 116, 215, 255, 211, 212, 255, 255, 128, 128, 128}
        },
        {
            { 1, 121, 236, 253, 212, 214, 255, 255, 128, 128, 128},
            { 141, 84, 213, 252, 201, 202, 255, 219, 128, 128, 128},
            { 42, 80, 160, 240, 162, 185, 255, 205, 128, 128, 128}
        },
        {
            { 1, 1, 255, 128, 128, 128, 128, 128, 128, 128, 128},
            { 244, 1, 255, 128, 128, 128, 128, 128, 128, 128, 128},
            { 238, 1, 255, 128, 128, 128, 128, 128, 128, 128, 128}
        }
    }
};

const uint16_t VP8_C0_TABLE[256] =
{
    2047, 2047, 1791, 1641, 1535, 1452, 1385, 1328, 1279, 1235, 1196, 1161, 1129, 1099, 1072, 1046,
    1023, 1000,  979,  959,  940,  922,  905,  889,  873,  858,  843,  829,  816,  803,  790,  778,
    767,  755,  744,  733,  723,  713,  703,  693,  684,  675,  666,  657,  649,  641,  633,  625,
    617,  609,  602,  594,  587,  580,  573,  567,  560,  553,  547,  541,  534,  528,  522,  516,
    511,  505,  499,  494,  488,  483,  477,  472,  467,  462,  457,  452,  447,  442,  437,  433,
    428,  424,  419,  415,  410,  406,  401,  397,  393,  389,  385,  381,  377,  373,  369,  365,
    361,  357,  353,  349,  346,  342,  338,  335,  331,  328,  324,  321,  317,  314,  311,  307,
    304,  301,  297,  294,  291,  288,  285,  281,  278,  275,  272,  269,  266,  263,  260,  257,
    255,  252,  249,  246,  243,  240,  238,  235,  232,  229,  227,  224,  221,  219,  216,  214,
    211,  208,  206,  203,  201,  198,  196,  194,  191,  189,  186,  184,  181,  179,  177,  174,
    172,  170,  168,  165,  163,  161,  159,  156,  154,  152,  150,  148,  145,  143,  141,  139,
    137,  135,  133,  131,  129,  127,  125,  123,  121,  119,  117,  115,  113,  111,  109,  107,
    105,  103,  101,   99,   97,   95,   93,   92,   90,   88,   86,   84,   82,   81,   79,   77,
    75,   73,   72,   70,   68,   66,   65,   63,   61,   60,   58,   56,   55,   53,   51,   50,
    48,   46,   45,   43,   41,   40,   38,   37,   35,   33,   32,   30,   29,   27,   25,   24,
    22,   21,   19,   18,   16,   15,   13,   12,   10,    9,    7,    6,    4,    3,    1,   1
};

extern const uint8_t VP8_PROBABILITY_UPDATE_FLAGS_G9 [VP8_NUM_COEFF_PLANES][VP8_NUM_COEFF_BANDS][VP8_NUM_LOCAL_COMPLEXITIES][VP8_NUM_COEFF_NODES]= {

    {
        {{0,0,0,0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0,0,0,0}},
        {{1,1,1,0,0,0,0,0,0,0,0}, {1,1,1,1,0,0,0,0,0,0,0}, {1,1,1,1,1,0,0,0,0,0,0}},
        {{0,1,1,1,0,0,0,0,0,0,0}, {1,1,1,0,0,0,0,0,0,0,0}, {1,1,1,0,0,0,0,0,0,0,0}},
        {{0,1,1,1,0,0,0,0,0,0,0}, {1,1,1,0,0,0,0,0,0,0,0}, {1,1,1,0,0,0,0,0,0,0,0}},
        {{0,1,1,1,0,0,0,0,0,0,0}, {1,1,1,0,0,0,0,0,0,0,0}, {1,1,1,0,0,0,0,0,0,0,0}},
        {{0,1,1,1,0,0,0,0,0,0,0}, {1,1,1,0,0,0,0,0,0,0,0}, {1,1,1,0,0,0,0,0,0,0,0}},
        {{0,1,1,1,0,0,0,0,0,0,0}, {1,1,1,0,0,0,0,0,0,0,0}, {1,1,1,0,0,0,0,0,0,0,0}},
        {{0,0,0,0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0,0,0,0}}
    },

    {
        {{1,1,0,0,0,0,0,0,0,0,0}, {1,1,0,0,0,0,0,0,0,0,0}, {1,1,0,0,0,0,0,0,0,0,0}},
        {{0,1,1,0,0,0,0,0,0,0,0}, {1,1,1,0,0,0,0,0,0,0,0}, {1,1,1,1,0,0,0,0,0,0,0}},
        {{0,1,1,0,0,0,0,0,0,0,0}, {1,1,0,0,0,0,0,0,0,0,0}, {1,1,0,0,0,0,0,0,0,0,0}},
        {{0,1,1,0,0,0,0,0,0,0,0}, {1,1,0,0,0,0,0,0,0,0,0}, {1,1,0,0,0,0,0,0,0,0,0}},
        {{0,1,1,0,0,0,0,0,0,0,0}, {1,1,0,0,0,0,0,0,0,0,0}, {1,1,0,0,0,0,0,0,0,0,0}},
        {{0,1,1,0,0,0,0,0,0,0,0}, {1,1,0,0,0,0,0,0,0,0,0}, {1,1,0,0,0,0,0,0,0,0,0}},
        {{0,1,1,0,0,0,0,0,0,0,0}, {1,1,0,0,0,0,0,0,0,0,0}, {1,1,0,0,0,0,0,0,0,0,0}},
        {{0,0,0,0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0,0,0,0}}
    },

    {
        {{1,1,0,0,0,0,0,0,0,0,0}, {1,1,0,0,0,0,0,0,0,0,0}, {1,1,0,0,0,0,0,0,0,0,0}},
        {{0,1,1,0,0,0,0,0,0,0,0}, {1,1,0,0,0,0,0,0,0,0,0}, {1,1,0,0,0,0,0,0,0,0,0}},
        {{0,1,1,0,0,0,0,0,0,0,0}, {1,1,0,0,0,0,0,0,0,0,0}, {1,1,0,0,0,0,0,0,0,0,0}},
        {{0,1,1,0,0,0,0,0,0,0,0}, {1,1,0,0,0,0,0,0,0,0,0}, {1,1,0,0,0,0,0,0,0,0,0}},
        {{0,1,1,0,0,0,0,0,0,0,0}, {1,1,0,0,0,0,0,0,0,0,0}, {1,1,0,0,0,0,0,0,0,0,0}},
        {{0,1,1,0,0,0,0,0,0,0,0}, {1,1,0,0,0,0,0,0,0,0,0}, {1,1,0,0,0,0,0,0,0,0,0}},
        {{0,1,1,0,0,0,0,0,0,0,0}, {1,1,0,0,0,0,0,0,0,0,0}, {1,1,0,0,0,0,0,0,0,0,0}},
        {{0,0,0,0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0,0,0,0}}
    },

    {
        {{1,1,0,0,0,0,0,0,0,0,0}, {1,1,0,0,0,0,0,0,0,0,0}, {1,1,0,0,0,0,0,0,0,0,0}},
        {{0,1,1,0,0,0,0,0,0,0,0}, {1,1,1,0,0,0,0,0,0,0,0}, {1,1,1,1,0,0,0,0,0,0,0}},
        {{0,1,1,0,0,0,0,0,0,0,0}, {1,1,0,0,0,0,0,0,0,0,0}, {1,1,0,0,0,0,0,0,0,0,0}},
        {{0,1,1,0,0,0,0,0,0,0,0}, {1,1,0,0,0,0,0,0,0,0,0}, {1,1,0,0,0,0,0,0,0,0,0}},
        {{0,1,1,0,0,0,0,0,0,0,0}, {1,1,0,0,0,0,0,0,0,0,0}, {1,1,0,0,0,0,0,0,0,0,0}},
        {{0,1,1,0,0,0,0,0,0,0,0}, {1,1,0,0,0,0,0,0,0,0,0}, {1,1,0,0,0,0,0,0,0,0,0}},
        {{0,1,1,0,0,0,0,0,0,0,0}, {1,1,0,0,0,0,0,0,0,0,0}, {1,1,0,0,0,0,0,0,0,0,0}},
        {{0,0,0,0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0,0,0,0}}
    }
};

extern const uint8_t VP8_COEFF_UPDATE_PROBS_G9[4][8][3][11] =
{
  {
    {
      { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255},
      { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255},
      { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255}
    },
    {
      { 176, 246, 255, 255, 255, 255, 255, 255, 255, 255, 255},
      { 223, 241, 252, 255, 255, 255, 255, 255, 255, 255, 255},
      { 249, 253, 253, 255, 255, 255, 255, 255, 255, 255, 255}
    },
    {
      { 255, 244, 252, 255, 255, 255, 255, 255, 255, 255, 255},
      { 234, 254, 254, 255, 255, 255, 255, 255, 255, 255, 255},
      { 253, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255}
    },
    {
      { 255, 246, 254, 255, 255, 255, 255, 255, 255, 255, 255},
      { 239, 253, 254, 255, 255, 255, 255, 255, 255, 255, 255},
      { 254, 255, 254, 255, 255, 255, 255, 255, 255, 255, 255}
    },
    {
      { 255, 248, 254, 255, 255, 255, 255, 255, 255, 255, 255},
      { 251, 255, 254, 255, 255, 255, 255, 255, 255, 255, 255},
      { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255}
    },
    {
      { 255, 253, 254, 255, 255, 255, 255, 255, 255, 255, 255},
      { 251, 254, 254, 255, 255, 255, 255, 255, 255, 255, 255},
      { 254, 255, 254, 255, 255, 255, 255, 255, 255, 255, 255}
    },
    {
      { 255, 254, 253, 255, 254, 255, 255, 255, 255, 255, 255},
      { 250, 255, 254, 255, 254, 255, 255, 255, 255, 255, 255},
      { 254, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255}
    },
    {
      { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255},
      { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255},
      { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255}
    }
  },
  {
    {
      { 217, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255},
      { 225, 252, 241, 253, 255, 255, 254, 255, 255, 255, 255},
      { 234, 250, 241, 250, 253, 255, 253, 254, 255, 255, 255}
    },
    {
      { 255, 254, 255, 255, 255, 255, 255, 255, 255, 255, 255},
      { 223, 254, 254, 255, 255, 255, 255, 255, 255, 255, 255},
      { 238, 253, 254, 254, 255, 255, 255, 255, 255, 255, 255}
    },
    {
      { 255, 248, 254, 255, 255, 255, 255, 255, 255, 255, 255},
      { 249, 254, 255, 255, 255, 255, 255, 255, 255, 255, 255},
      { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255}
    },
    {
      { 255, 253, 255, 255, 255, 255, 255, 255, 255, 255, 255},
      { 247, 254, 255, 255, 255, 255, 255, 255, 255, 255, 255},
      { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255}
    },
    {
      { 255, 253, 254, 255, 255, 255, 255, 255, 255, 255, 255},
      { 252, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255},
      { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255}
    },
    {
      { 255, 254, 254, 255, 255, 255, 255, 255, 255, 255, 255},
      { 253, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255},
      { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255}
    },
    {
      { 255, 254, 253, 255, 255, 255, 255, 255, 255, 255, 255},
      { 250, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255},
      { 254, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255}
    },
    {
      { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255},
      { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255},
      { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255}
    }
  },
  {
    {
      { 186, 251, 250, 255, 255, 255, 255, 255, 255, 255, 255},
      { 234, 251, 244, 254, 255, 255, 255, 255, 255, 255, 255},
      { 251, 251, 243, 253, 254, 255, 254, 255, 255, 255, 255}
    },
    {
      { 255, 253, 254, 255, 255, 255, 255, 255, 255, 255, 255},
      { 236, 253, 254, 255, 255, 255, 255, 255, 255, 255, 255},
      { 251, 253, 253, 254, 254, 255, 255, 255, 255, 255, 255}
    },
    {
      { 255, 254, 254, 255, 255, 255, 255, 255, 255, 255, 255},
      { 254, 254, 254, 255, 255, 255, 255, 255, 255, 255, 255},
      { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255}
    },
    {
      { 255, 254, 255, 255, 255, 255, 255, 255, 255, 255, 255},
      { 254, 254, 255, 255, 255, 255, 255, 255, 255, 255, 255},
      { 254, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255}
    },
    {
      { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255},
      { 254, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255},
      { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255}
    },
    {
      { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255},
      { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255},
      { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255}
    },
    {
      { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255},
      { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255},
      { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255}
    },
    {
      { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255},
      { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255},
      { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255}
    }
  },
  {
    {
      { 248, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255},
      { 250, 254, 252, 254, 255, 255, 255, 255, 255, 255, 255},
      { 248, 254, 249, 253, 255, 255, 255, 255, 255, 255, 255}
    },
    {
      { 255, 253, 253, 255, 255, 255, 255, 255, 255, 255, 255},
      { 246, 253, 253, 255, 255, 255, 255, 255, 255, 255, 255},
      { 252, 254, 251, 254, 254, 255, 255, 255, 255, 255, 255}
    },
    {
      { 255, 254, 252, 255, 255, 255, 255, 255, 255, 255, 255},
      { 248, 254, 253, 255, 255, 255, 255, 255, 255, 255, 255},
      { 253, 255, 254, 254, 255, 255, 255, 255, 255, 255, 255}
    },
    {
      { 255, 251, 254, 255, 255, 255, 255, 255, 255, 255, 255},
      { 245, 251, 254, 255, 255, 255, 255, 255, 255, 255, 255},
      { 253, 253, 254, 255, 255, 255, 255, 255, 255, 255, 255}
    },
    {
      { 255, 251, 253, 255, 255, 255, 255, 255, 255, 255, 255},
      { 252, 253, 254, 255, 255, 255, 255, 255, 255, 255, 255},
      { 255, 254, 255, 255, 255, 255, 255, 255, 255, 255, 255}
    },
    {
      { 255, 252, 255, 255, 255, 255, 255, 255, 255, 255, 255},
      { 249, 255, 254, 255, 255, 255, 255, 255, 255, 255, 255},
      { 255, 255, 254, 255, 255, 255, 255, 255, 255, 255, 255}
    },
    {
      { 255, 255, 253, 255, 255, 255, 255, 255, 255, 255, 255},
      { 250, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255},
      { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255}
    },
    {
      { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255},
      { 254, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255},
      { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255}
    }
  }
};

static const int32_t iMbEncCurbeSize_G9[CODECHAL_ENCODE_VP8_MBENC_IDX_NUM] = {
    sizeof(struct MediaObjectVp8MbencIStaticDataG9),
    sizeof(struct MediaObjectVp8MbencIStaticDataG9),
    sizeof(struct MediaObjectVp8MbencPStaticDataG9)
};

static const int32_t iBrcBtCount_G9[CODECHAL_ENCODE_VP8_BRC_IDX_NUM] = {
    CODECHAL_VP8_MBENC_NUM_SURFACES_G9,
    CODECHAL_VP8_BRC_INIT_RESET_NUM_SURFACES_G9,
    CODECHAL_VP8_BRC_INIT_RESET_NUM_SURFACES_G9,
    CODECHAL_VP8_BRC_UPDATE_NUM_SURFACES_G9
};

static const int32_t iBrcCurbeSize_G9[CODECHAL_ENCODE_VP8_BRC_IDX_NUM] = {
    sizeof(struct MediaObjectVp8MbencIStaticDataG9),
    sizeof(struct MediaObjectVp8BrcInitResetStaticDataG9),
    sizeof(struct MediaObjectVp8BrcInitResetStaticDataG9),
    sizeof(struct MediaObjectVp8BrcUpdateStaticDataG9)
};

CodechalEncodeVp8G9::CodechalEncodeVp8G9(
    CodechalHwInterface*    hwInterface,
    CodechalDebugInterface* debugInterface,
    PCODECHAL_STANDARD_INFO standardInfo)
    :CodechalEncodeVp8(hwInterface, debugInterface, standardInfo)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    pfnGetKernelHeaderAndSize         = GetKernelHeaderAndSize;

    m_kuid = IDR_CODEC_AllVP8Enc;
#ifndef _FULL_OPEN_SOURCE
    m_kernelBase = (uint8_t*)IGCODECKRN_G9;
#else
    m_kernelBase = nullptr;
#endif

    CodecHalGetKernelBinaryAndSize(
        m_kernelBase,
        m_kuid,
        &m_kernelBinary,
        &m_combinedKernelSize);

    m_hwInterface->GetStateHeapSettings()->dwNumSyncTags = CODECHAL_NUM_VP8_ENC_SYNC_TAGS;
    m_hwInterface->GetStateHeapSettings()->dwIshSize +=
        MOS_ALIGN_CEIL(m_combinedKernelSize, (1 << MHW_KERNEL_OFFSET_SHIFT));
    m_hwInterface->GetStateHeapSettings()->dwDshSize = CODECHAL_INIT_DSH_SIZE_VP8_ENC;

    m_brcDistortionBufferSupported = true;
    m_brcConstantBufferSupported   = true;
    m_brcConstantSurfaceWidth      = CODECHAL_ENCODE_VP8_BRC_CONSTANTSURFACE_WIDTH;
    m_brcConstantSurfaceHeight     = CODECHAL_ENCODE_VP8_BRC_CONSTANTSURFACE_HEIGHT;
}

MOS_STATUS CodechalEncodeVp8G9::Initialize(CodechalSetting * codecHalSettings)
{
    MOS_STATUS status = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    // common initilization
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncodeVp8::Initialize(codecHalSettings));

    return status;
}

MOS_STATUS CodechalEncodeVp8G9::InitKernelState()
{
    MOS_STATUS status = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    // Init kernel state
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitKernelStateMe());
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitKernelStateMbEnc());
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitKernelStateBrc());

    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitKernelStateTpu());
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitKernelStateMpu());

    return status;
}

MOS_STATUS CodechalEncodeVp8G9::GetKernelHeaderAndSize(
    void                          *binary,
    EncOperation                  operation,
    uint32_t                      krnStateIdx,
    void                          *krnHeader,
    uint32_t                      *krnSize)
{
    struct CodechalVp8KernelHeaderG9 * kernelHeaderTable;
    PCODECHAL_KERNEL_HEADER          currKrnHeader, invalidEntry, nextKrnHeader;
    uint32_t                         nextKrnOffset;
    MOS_STATUS                       status = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(binary);
    CODECHAL_ENCODE_CHK_NULL_RETURN(krnHeader);
    CODECHAL_ENCODE_CHK_NULL_RETURN(krnSize);

    kernelHeaderTable = (struct CodechalVp8KernelHeaderG9*)binary;
    invalidEntry = &(kernelHeaderTable->VP8_BRC_FrameEncUpdate) + 1;
    nextKrnOffset = *krnSize;

    if (operation == ENC_SCALING4X)
    {
        currKrnHeader = &kernelHeaderTable->PLY_DScale_PLY;
    }
    else if (operation == ENC_ME)
    {
        currKrnHeader = &kernelHeaderTable->VP8_ME_P;
    }
    else if (operation == ENC_BRC)
    {
        currKrnHeader = &kernelHeaderTable->VP8MBEnc_I_Dist;
    }
    else if (operation == ENC_MBENC_I_LUMA)
    {
        currKrnHeader = &kernelHeaderTable->VP8MBEnc_I_Luma;
    }
    else if (operation == ENC_MBENC)
    {
        currKrnHeader = &kernelHeaderTable->VP8MBEnc_Norm_Frm_I;
    }
    else if (operation == ENC_MPU)
    {
        currKrnHeader = &kernelHeaderTable->VP8_MPU;
    }
    else if (operation == ENC_TPU)
    {
        currKrnHeader = &kernelHeaderTable->VP8_TPU;
    }
    else
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Unsupported ENC mode requested");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    currKrnHeader += krnStateIdx;
    *((PCODECHAL_KERNEL_HEADER)krnHeader) = *currKrnHeader;

    nextKrnHeader = (currKrnHeader + 1);
    if (nextKrnHeader < invalidEntry)
    {
        nextKrnOffset = nextKrnHeader->KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT;
    }
    *krnSize = nextKrnOffset - (currKrnHeader->KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);

    return status;
}

MOS_STATUS CodechalEncodeVp8G9::InitKernelStateHelper(
    struct CodechalEncodeVp8InitKernelStateParams * params)
{
    PMHW_STATE_HEAP_INTERFACE               stateHeapInterface;
    PMHW_KERNEL_STATE                       kernelStatePtr;
    CODECHAL_KERNEL_HEADER                  currKrnHeader;
    uint32_t                                kernelSize;
    MOS_STATUS                              status = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pKernelState);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pRenderEngineInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pRenderEngineInterface->GetHwCaps());
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pRenderEngineInterface->m_stateHeapInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pui8Binary);

    stateHeapInterface = params->pRenderEngineInterface->m_stateHeapInterface;
    kernelStatePtr     = params->pKernelState;
    kernelSize        = params->dwCombinedKernelSize;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(pfnGetKernelHeaderAndSize(
        params->pui8Binary,
        params->Operation,
        params->dwKrnStateIdx,
        &currKrnHeader,
        &kernelSize));

    kernelStatePtr->KernelParams.iBTCount = params->iBtCount;
    kernelStatePtr->KernelParams.iThreadCount = params->pRenderEngineInterface->GetHwCaps()->dwMaxThreads;
    kernelStatePtr->KernelParams.iCurbeLength = params->iCurbeCount;
    kernelStatePtr->KernelParams.iBlockWidth = CODECHAL_MACROBLOCK_WIDTH;
    kernelStatePtr->KernelParams.iBlockHeight = CODECHAL_MACROBLOCK_HEIGHT;
    kernelStatePtr->KernelParams.iIdCount = 1;

    kernelStatePtr->dwCurbeOffset = stateHeapInterface->pStateHeapInterface->GetSizeofCmdInterfaceDescriptorData();
    kernelStatePtr->KernelParams.pBinary =
        params->pui8Binary +
        (currKrnHeader.KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);
    kernelStatePtr->KernelParams.iSize = kernelSize;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(stateHeapInterface->pfnCalculateSshAndBtSizesRequested(
        stateHeapInterface,
        kernelStatePtr->KernelParams.iBTCount,
        &kernelStatePtr->dwSshSize,
        &kernelStatePtr->dwBindingTableSize));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->MhwInitISH(stateHeapInterface, kernelStatePtr));

    return status;
}

MOS_STATUS CodechalEncodeVp8G9::InitKernelStateMbEnc()
{
    MhwRenderInterface                          *renderEngineInterface;
    PMHW_STATE_HEAP_INTERFACE                   stateHeapInterface;
    PMHW_KERNEL_STATE                           kernelStatePtr;
    uint32_t                                    combinedKernelSize;
    uint32_t                                    krnStateIdx;
    struct CodechalBindingTableVp8Mbenc*        bindingTable;
    uint8_t                                     *binary;
    struct CodechalEncodeVp8InitKernelStateParams    initKernelStateParams;
    MOS_STATUS                                  status = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface->GetRenderInterface());

    renderEngineInterface  = m_hwInterface->GetRenderInterface();
    stateHeapInterface     = renderEngineInterface->m_stateHeapInterface;
    CODECHAL_ENCODE_CHK_NULL_RETURN(stateHeapInterface);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalGetKernelBinaryAndSize(
        m_kernelBase,
        m_kuid,
        &binary,
        (uint32_t*)&combinedKernelSize));

    for (krnStateIdx = 0; krnStateIdx < CODECHAL_ENCODE_VP8_MBENC_IDX_NUM; krnStateIdx++)
    {
        kernelStatePtr = &m_mbEncKernelStates[krnStateIdx];

        MOS_ZeroMemory(&initKernelStateParams, sizeof(initKernelStateParams));
        initKernelStateParams.pKernelState              = kernelStatePtr;
        initKernelStateParams.pRenderEngineInterface    = renderEngineInterface;
        initKernelStateParams.pui8Binary                = binary;
        if (krnStateIdx == CODECHAL_ENCODE_VP8_MBENC_IDX_P)
        {
            initKernelStateParams.Operation             = ENC_MBENC;
            initKernelStateParams.dwKrnStateIdx         = 1; // VP8MBEnc_Norm_Frm_P
        }
        else
        {
            initKernelStateParams.dwKrnStateIdx         = 0;
            if (krnStateIdx == CODECHAL_ENCODE_VP8_MBENC_IDX_I_CHROMA)
            {
                initKernelStateParams.Operation         = ENC_MBENC; // VP8MBEnc_Norm_Frm_I
            }
            else // CODECHAL_ENCODE_VP8_MBENC_IDX_I_LUMA
            {
                initKernelStateParams.Operation         = ENC_MBENC_I_LUMA; // VP8MBEnc_I_Luma
            }
        }
        initKernelStateParams.dwCombinedKernelSize      = combinedKernelSize;
        initKernelStateParams.iBtCount                  = CODECHAL_VP8_MBENC_NUM_SURFACES_G9;
        initKernelStateParams.iCurbeCount               = iMbEncCurbeSize_G9[krnStateIdx];
        CODECHAL_ENCODE_CHK_STATUS_RETURN(InitKernelStateHelper(
            &initKernelStateParams));

        if (krnStateIdx != CODECHAL_ENCODE_VP8_MBENC_IDX_P)
        {
            // I luma and chroma kernels share a single CURBE, so the DSH contains 2 IDs and 1 CURBE
            kernelStatePtr->dwCurbeOffset              =
                stateHeapInterface->pStateHeapInterface->GetSizeofCmdInterfaceDescriptorData() * 2;

            if (krnStateIdx == CODECHAL_ENCODE_VP8_MBENC_IDX_I_CHROMA)
            {
                // The luma kernel ID will occur before the chroma kernel ID
                kernelStatePtr->dwIdOffset             =
                    stateHeapInterface->pStateHeapInterface->GetSizeofCmdInterfaceDescriptorData();
            }
        }
    }

    m_mbEncIFrameDshSize =
        (stateHeapInterface->pStateHeapInterface->GetSizeofCmdInterfaceDescriptorData() * 2) +
        MOS_ALIGN_CEIL(m_mbEncKernelStates[CODECHAL_ENCODE_VP8_MBENC_IDX_I_LUMA].KernelParams.iCurbeLength,
            stateHeapInterface->pStateHeapInterface->GetCurbeAlignment());

    // Until a better way can be found, maintain old binding table structures
    bindingTable = &m_mbEncBindingTable;

    bindingTable->dwVp8MBEncMBOut              = CODECHAL_VP8_MBENC_PER_MB_OUT_G9;
    bindingTable->dwVp8MBEncCurrY              = CODECHAL_VP8_MBENC_CURR_Y_G9;
    bindingTable->dwVp8MBEncCurrUV             = CODECHAL_VP8_MBENC_CURR_UV_G9;
    bindingTable->dwVp8MBEncMVDataFromME       = CODECHAL_VP8_MBENC_MV_DATA_FROM_ME_G9;
    bindingTable->dwVp8MBEncIndMVData          = CODECHAL_VP8_MBENC_IND_MV_DATA_G9;
    bindingTable->dwVp8MBEncRef1Pic            = CODECHAL_VP8_MBENC_REF1_PIC_G9;
    bindingTable->dwVp8MBEncRef2Pic            = CODECHAL_VP8_MBENC_REF2_PIC_G9;
    bindingTable->dwVp8MBEncRef3Pic            = CODECHAL_VP8_MBENC_REF3_PIC_G9;
    bindingTable->dwVp8MBEncRefMBCount         = CODECHAL_VP8_MBENC_REF_MB_COUNT_G9;
    bindingTable->dwVp8MBEncMBModeCostLuma     = CODECHAL_VP8_MBENC_MB_MODE_COST_LUMA_G9;
    bindingTable->dwVp8MBEncBlockModeCost      = CODECHAL_VP8_MBENC_BLOCK_MODE_COST_G9;
    bindingTable->dwVp8MBEncChromaRecon        = CODECHAL_VP8_MBENC_CHROMA_RECON_G9;
    bindingTable->dwVp8MBEncHistogram          = CODECHAL_VP8_MBENC_HISTOGRAM_G9;
    bindingTable->dwVp8MBEncSegmentationMap    = CODECHAL_VP8_MBENC_SEGMENTATION_MAP_G9;
    bindingTable->dwVp8MBEncBRCDist            = CODECHAL_VP8_MBENC_IDIST_G9;
    bindingTable->dwVp8MbEncCurrYDownscaled    = CODECHAL_VP8_MBENC_CURR_Y_DOWNSCALED_G9;
    bindingTable->dwVp8MBEncVMECoarseIntra     = CODECHAL_VP8_MBENC_VME_Coarse_Intra_G9;
    bindingTable->dwVp8InterPredDistortion     = CODECHAL_VP8_MBEBC_INTER_PRED_DISTORTION_G9;
    bindingTable->dwVp8PerMVDataSurface        = CODECHAL_VP8_MBEBC_PER_MV_DATA_G9;
    bindingTable->dwVp8MBEncPerMBQuantDataP    = CODECHAL_VP8_MBENC_P_PER_MB_QUANT_G9;
    bindingTable->dwVp8MBEncVMEInterPred       = CODECHAL_VP8_MBENC_INTER_PRED_G9;
    bindingTable->dwVp8MBEncVMEDebugStreamoutI = CODECHAL_VP8_MBENC_I_VME_DEBUG_STREAMOUT_G9;
    bindingTable->dwVp8MBModeCostUpdateSurface = CODECHAL_VP8_MBENC_MODE_COST_UPDATE_G9;
    bindingTable->dwVp8MBEncVMEDebugStreamoutP = CODECHAL_VP8_MBENC_P_VME_DEBUG_STREAMOUT_G9;
    bindingTable->dwVp8MBEncVME                = CODECHAL_VP8_MBENC_VME_G9;

    return status;
}

MOS_STATUS CodechalEncodeVp8G9::InitKernelStateBrc()
{
    MhwRenderInterface                          *renderEngineInterface;
    PMHW_KERNEL_STATE                           kernelStatePtr;
    uint32_t                                    combinedKernelSize;
    uint32_t                                    krnStateIdx;
    struct CodechalBindingTableVp8BrcUpdate*    bindingTable;
    uint8_t                                     *binary;
    struct CodechalEncodeVp8InitKernelStateParams    initKernelStateParams;
    MOS_STATUS                                  status = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface->GetRenderInterface());

    renderEngineInterface  = m_hwInterface->GetRenderInterface();

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalGetKernelBinaryAndSize(
        m_kernelBase,
        m_kuid,
        &binary,
        (uint32_t*)&combinedKernelSize));

    for (krnStateIdx = 0; krnStateIdx < CODECHAL_ENCODE_VP8_BRC_IDX_NUM; krnStateIdx++)
    {
        kernelStatePtr = &m_brcKernelStates[krnStateIdx];

        MOS_ZeroMemory(&initKernelStateParams, sizeof(initKernelStateParams));
        initKernelStateParams.pKernelState              = kernelStatePtr;
        initKernelStateParams.pRenderEngineInterface    = renderEngineInterface;
        initKernelStateParams.pui8Binary                = binary;
        initKernelStateParams.Operation                 = ENC_BRC;
        initKernelStateParams.dwKrnStateIdx             = krnStateIdx;
        initKernelStateParams.dwCombinedKernelSize      = combinedKernelSize;
        initKernelStateParams.iBtCount                  = iBrcBtCount_G9[krnStateIdx];
        initKernelStateParams.iCurbeCount               = iBrcCurbeSize_G9[krnStateIdx];
        CODECHAL_ENCODE_CHK_STATUS_RETURN(InitKernelStateHelper(
            &initKernelStateParams));
    }

    // Until a better way can be found, maintain old binding table structures
    bindingTable                                   = &m_brcUpdateBindingTable;
    bindingTable->dwBrcHistoryBuffer               = CODECHAL_VP8_BRC_UPDATE_HISTORY_G9;
    bindingTable->dwBrcPakStatisticsOutputBuffer   = CODECHAL_VP8_BRC_UPDATE_PAK_STATISTICS_OUTPUT_G9;
    bindingTable->dwBrcEncoderCfgReadBuffer        = CODECHAL_VP8_BRC_UPDATE_MFX_ENCODER_CFG_READ_G9;
    bindingTable->dwBrcEncoderCfgWriteBuffer       = CODECHAL_VP8_BRC_UPDATE_MFX_ENCODER_CFG_WRITE_G9;
    bindingTable->dwBrcMbEncCurbeReadBuffer        = CODECHAL_VP8_BRC_UPDATE_MBENC_CURBE_READ_G9;
    bindingTable->dwBrcMbEncCurbeWriteData         = CODECHAL_VP8_BRC_UPDATE_MBENC_CURBE_WRITE_G9;
    bindingTable->dwBrcDistortionBuffer            = CODECHAL_VP8_BRC_UPDATE_DISTORTION_SURFACE_G9;
    bindingTable->dwBrcConstantData                = CODECHAL_VP8_BRC_UPDATE_CONSTANT_DATA_G9;
    bindingTable->dwVp8BrcSegmentationMap          = CODECHAL_VP8_BRC_UPDATE_SEGMENT_MAP_G9;
    bindingTable->dwBrcMpuCurbeReadBuffer          = CODECHAL_VP8_BRC_UPDATE_MPU_CURBE_READ_G9;
    bindingTable->dwBrcMpuCurbeWriteData           = CODECHAL_VP8_BRC_UPDATE_MPU_CURBE_WRITE_G9;
    bindingTable->dwBrcTpuCurbeReadBuffer          = CODECHAL_VP8_BRC_UPDATE_TPU_CURBE_READ_G9;
    bindingTable->dwBrcTpuCurbeWriteData           = CODECHAL_VP8_BRC_UPDATE_TPU_CURBE_WRITE_G9;

    return status;
}

MOS_STATUS CodechalEncodeVp8G9::InitKernelStateMe()
{
    MhwRenderInterface                          *renderEngineInterface;
    uint32_t                                    combinedKernelSize;
    struct CodechalBindingTableVp8Me*           bindingTable;
    uint8_t                                     *binary;
    struct CodechalEncodeVp8InitKernelStateParams    initKernelStateParams;
    MOS_STATUS                                  status = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface->GetRenderInterface());

    renderEngineInterface  = m_hwInterface->GetRenderInterface();

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalGetKernelBinaryAndSize(
        m_kernelBase,
        m_kuid,
        &binary,
        (uint32_t*)&combinedKernelSize));

    MOS_ZeroMemory(&initKernelStateParams, sizeof(initKernelStateParams));
    initKernelStateParams.pKernelState              = &m_meKernelState;
    initKernelStateParams.pRenderEngineInterface    = renderEngineInterface;
    initKernelStateParams.pui8Binary                = binary;
    initKernelStateParams.Operation                 = ENC_ME;
    initKernelStateParams.dwCombinedKernelSize      = combinedKernelSize;
    initKernelStateParams.iBtCount                  = CODECHAL_VP8_ME_NUM_SURFACES_G9;
    initKernelStateParams.iCurbeCount               = sizeof(struct MediaObjectVp8MeStaticDataG9);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitKernelStateHelper(
        &initKernelStateParams));

    // Until a better way can be found, maintain old binding table structures
    bindingTable = &m_meBindingTable;

    bindingTable->dwVp8MEMVDataSurface     = CODECHAL_VP8_ME_MV_DATA_G9;
    bindingTable->dwVp816xMEMVDataSurface  = CODECHAL_VP8_16xME_MV_DATA_G9;
    bindingTable->dwVp8MeDist              = CODECHAL_VP8_ME_DISTORTION_G9;
    bindingTable->dwVp8MeBrcDist           = CODECHAL_VP8_ME_MIN_DIST_BRC_DATA_G9;
    bindingTable->dwVp8MeCurrPic           = CODECHAL_VP8_VME_INTER_PRED_G9;
    bindingTable->dwVp8MeRef1Pic           = CODECHAL_VP8_ME_REF1_PIC_G9;
    bindingTable->dwVp8MeRef2Pic           = CODECHAL_VP8_ME_REF2_PIC_G9;
    bindingTable->dwVp8MeRef3Pic           = CODECHAL_VP8_ME_REF3_PIC_G9;

    return status;
}

MOS_STATUS CodechalEncodeVp8G9::InitKernelStateMpu()
{
    MhwRenderInterface                          *renderEngineInterface;
    uint32_t                                    combinedKernelSize;
    struct CodechalBindingTableVp8Mpu*          bindingTable;
    uint8_t                                     *binary;
    struct CodechalEncodeVp8InitKernelStateParams    initKernelStateParams;
    MOS_STATUS                                  status = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface->GetRenderInterface());

    renderEngineInterface  = m_hwInterface->GetRenderInterface();

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalGetKernelBinaryAndSize(
        m_kernelBase,
        m_kuid,
        &binary,
        (uint32_t*)&combinedKernelSize));

    MOS_ZeroMemory(&initKernelStateParams, sizeof(initKernelStateParams));
    initKernelStateParams.pKernelState              = &m_mpuKernelState;
    initKernelStateParams.pRenderEngineInterface    = renderEngineInterface;
    initKernelStateParams.pui8Binary                = binary;
    initKernelStateParams.Operation                 = ENC_MPU;
    initKernelStateParams.dwCombinedKernelSize      = combinedKernelSize;
    initKernelStateParams.iBtCount                  = CODECHAL_VP8_MPU_FHB_NUM_SURFACES_G9;
    initKernelStateParams.iCurbeCount               = sizeof(struct MediaObjectVp8MpuFhbStaticDataG9);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitKernelStateHelper(
        &initKernelStateParams));

    // Until a better way can be found, maintain old binding table structures
    bindingTable = &m_mpuBindingTable;

    bindingTable->dwVp8MpuHistogram                 = CODECHAL_VP8_MPU_FHB_HISTOGRAM_G9;
    bindingTable->dwVp8MpuReferenceModeProbability  = CODECHAL_VP8_MPU_FHB_REF_MODE_PROBABILITY_G9;
    bindingTable->dwVp8MpuModeProbability           = CODECHAL_VP8_MPU_FHB_CURR_MODE_PROBABILITY_G9;
    bindingTable->dwVp8MpuReferenceTokenProbability = CODECHAL_VP8_MPU_FHB_REF_TOKEN_PROBABILITY_G9;
    bindingTable->dwVp8MpuTokenProbability          = CODECHAL_VP8_MPU_FHB_CURR_TOKEN_PROBABILITY_G9;
    bindingTable->dwVp8MpuFrameHeaderBitstream      = CODECHAL_VP8_MPU_FHB_HEADER_BITSTREAM_G9;
    bindingTable->dwVp8MpuHeaderMetaData            = CODECHAL_VP8_MPU_FHB_HEADER_METADATA_G9;
    bindingTable->dwVp8MpuPictureState              = CODECHAL_VP8_MPU_FHB_PICTURE_STATE_G9;
    bindingTable->dwVp8MpuMpuBitstream              = CODECHAL_VP8_MPU_FHB_MPU_BITSTREAM_G9;
    bindingTable->dwVp8MpuTokenBitsData             = CODECHAL_VP8_MPU_FHB_TOKEN_BITS_DATA_TABLE_G9;
    bindingTable->dwVp8MpuKernelDebugDump           = CODECHAL_VP8_MPU_FHB_VME_DEBUG_STREAMOUT_G9;
    bindingTable->dwVp8MpuEntropyCost               = CODECHAL_VP8_MPU_FHB_ENTROPY_COST_TABLE_G9;
    bindingTable->dwVp8MpuModeCostUpdateSurface     = CODECHAL_VP8_MPU_MODE_COST_UPDATE_G9;

    return status;
}

MOS_STATUS CodechalEncodeVp8G9::InitKernelStateTpu()
{
    MhwRenderInterface                          *renderEngineInterface;
    uint32_t                                    combinedKernelSize;
    struct CodechalBindingTableVp8Tpu*          bindingTable;
    uint8_t                                     *binary;
    struct CodechalEncodeVp8InitKernelStateParams    initKernelStateParams;
    MOS_STATUS                                  status = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface->GetRenderInterface());

    renderEngineInterface  = m_hwInterface->GetRenderInterface();

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalGetKernelBinaryAndSize(
        m_kernelBase,
        m_kuid,
        &binary,
        (uint32_t*)&combinedKernelSize));

    MOS_ZeroMemory(&initKernelStateParams, sizeof(initKernelStateParams));
    initKernelStateParams.pKernelState              = &m_tpuKernelState;
    initKernelStateParams.pRenderEngineInterface    = renderEngineInterface;
    initKernelStateParams.pui8Binary                = binary;
    initKernelStateParams.Operation                 = ENC_TPU;
    initKernelStateParams.dwCombinedKernelSize      = combinedKernelSize;
    initKernelStateParams.iBtCount                  = CODECHAL_VP8_TPU_FHB_NUM_SURFACES_G9;
    initKernelStateParams.iCurbeCount               = sizeof(struct MediaObjectVp8TpuFhbStaticDataG9);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitKernelStateHelper(
        &initKernelStateParams));

    // Until a better way can be found, maintain old binding table structures
    bindingTable = &m_tpuBindingTable;

    bindingTable->dwVp8TpuPakTokenStatistics               = CODECHAL_VP8_TPU_FHB_PAK_TOKEN_STATISTICS_G9;
    bindingTable->dwVp8TpuTokenUpdateFlags                 = CODECHAL_VP8_TPU_FHB_TOKEN_UPDATE_FLAGS_G9;
    bindingTable->dwVp8TpuEntropyCost                      = CODECHAL_VP8_TPU_FHB_ENTROPY_COST_TABLE_G9;
    bindingTable->dwVp8TpuFrameHeaderBitstream             = CODECHAL_VP8_TPU_FHB_HEADER_BITSTREAM_G9;
    bindingTable->dwVp8TpuDefaultTokenProbability          = CODECHAL_VP8_TPU_FHB_DEFAULT_TOKEN_PROBABILITY_G9;
    bindingTable->dwVp8TpuPictureState                     = CODECHAL_VP8_TPU_FHB_PICTURE_STATE_G9;
    bindingTable->dwVp8TpuMpuCurbeData                     = CODECHAL_VP8_TPU_FHB_MPU_CURBE_DATA_G9;
    bindingTable->dwVp8TpuHeaderMetaData                   = CODECHAL_VP8_TPU_FHB_HEADER_METADATA_G9;
    bindingTable->dwVp8TpuTokenProbability                 = CODECHAL_VP8_TPU_FHB_TOKEN_PROBABILITY_G9;
    bindingTable->dwVp8TpuPakHardwareTokenProbabilityPass1 = CODECHAL_VP8_TPU_FHB_PAK_HW_PASS1_PROBABILITY_G9;
    bindingTable->dwVp8TpuKeyFrameTokenProbability         = CODECHAL_VP8_TPU_FHB_KEY_TOKEN_PROBABILITY_G9;
    bindingTable->dwVp8TpuUpdatedTokenProbability          = CODECHAL_VP8_TPU_FHB_UPDATED_TOKEN_PROBABILITY_G9;
    bindingTable->dwVp8TpuPakHardwareTokenProbabilityPass2 = CODECHAL_VP8_TPU_FHB_PAK_HW_PASS2_PROBABILITY_G9;
    bindingTable->dwVp8TpuKernelDebugDump                  = CODECHAL_VP8_TPU_FHB_VME_DEBUG_STREAMOUT_G9;
    bindingTable->dwVp8TpuRepakDecision                    = CODECHAL_VP8_TPU_FHB_REPAK_DECISION_G9;

    return status;
}

MOS_STATUS CodechalEncodeVp8G9::InitBrcConstantBuffer(struct CodechalVp8InitBrcConstantBufferParams* params)
{
    uint8_t*        data;
    MOS_LOCK_PARAMS lockFlags;
    MOS_STATUS      status = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pOsInterface);

    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.WriteOnly = 1;
    data = (uint8_t *)params->pOsInterface->pfnLockResource(
        params->pOsInterface,
        &params->resBrcConstantDataBuffer,
        &lockFlags);
    CODECHAL_ENCODE_CHK_NULL_RETURN(data);

    MOS_ZeroMemory(data, BRC_CONSTANTSURFACE_VP8);

    // Fill surface with QP Adjustment table, Distortion threshold table, MaxFrame threshold table, Distortion QP Adjustment Table for I frame
    status = MOS_SecureMemcpy(
        data,
        sizeof(VP8_BRC_QPAdjustment_DistThreshold_MaxFrameThreshold_DistQPAdjustment_IPB_G9),
        (void *)VP8_BRC_QPAdjustment_DistThreshold_MaxFrameThreshold_DistQPAdjustment_IPB_G9,
        sizeof(VP8_BRC_QPAdjustment_DistThreshold_MaxFrameThreshold_DistQPAdjustment_IPB_G9));
    if(status != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
        return status;
    }

    data += sizeof(VP8_BRC_QPAdjustment_DistThreshold_MaxFrameThreshold_DistQPAdjustment_IPB_G9);

    status = MOS_SecureMemcpy(
        data,
        sizeof(VP8_BRC_IFRAME_COST_TABLE_G9),
        (void *)VP8_BRC_IFRAME_COST_TABLE_G9,
        sizeof(VP8_BRC_IFRAME_COST_TABLE_G9));
    if(status != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
        return status;
    }

    data += sizeof(VP8_BRC_IFRAME_COST_TABLE_G9);

    status = MOS_SecureMemcpy(
        data,
        sizeof(VP8_BRC_PFRAME_COST_TABLE_G9),
        (void *)VP8_BRC_PFRAME_COST_TABLE_G9,
        sizeof(VP8_BRC_PFRAME_COST_TABLE_G9));
    if(status != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
        return status;
    }

    data += sizeof(VP8_BRC_PFRAME_COST_TABLE_G9);

    status = MOS_SecureMemcpy(
        data,
        sizeof(VP8_BRC_QUANT_DC_TABLE),
        (void *)VP8_BRC_QUANT_DC_TABLE,
        sizeof(VP8_BRC_QUANT_DC_TABLE));
    if(status != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
        return status;
    }

    data += sizeof(VP8_BRC_QUANT_DC_TABLE);

    status = MOS_SecureMemcpy(
        data,
        sizeof(VP8_BRC_QUANT_AC_TABLE),
        (void *)VP8_BRC_QUANT_AC_TABLE,
        sizeof(VP8_BRC_QUANT_AC_TABLE));
    if(status != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
        return status;
    }

    data += sizeof(VP8_BRC_QUANT_AC_TABLE);

    status = MOS_SecureMemcpy(
        data,
        sizeof(VP8_BRC_SKIP_MV_THRESHOLD_TABLE),
        (void *)VP8_BRC_SKIP_MV_THRESHOLD_TABLE,
        sizeof(VP8_BRC_SKIP_MV_THRESHOLD_TABLE));
    if(status != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
        return status;
    }

    params->pOsInterface->pfnUnlockResource(
        params->pOsInterface,
        &params->resBrcConstantDataBuffer);

    return status;
}

MOS_STATUS CodechalEncodeVp8G9::InitBrcDistortionBuffer()
{
    uint8_t                         *data;
    MOS_STATUS                      status = MOS_STATUS_SUCCESS;
    MOS_LOCK_PARAMS                 lockFlagsWriteOnly;
    uint32_t                        size, width, height;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_osInterface);

    data = (uint8_t *)m_osInterface->pfnLockResource(
        m_osInterface,
        &(m_brcBuffers.sMeBrcDistortionBuffer.OsResource),
        &lockFlagsWriteOnly);

    if (data == nullptr)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to Lock ME BRC Distortion Buffer.");
        status = MOS_STATUS_UNKNOWN;
        return status;
    }
    width = MOS_ALIGN_CEIL((m_downscaledWidthInMb4x * 8), 64);
    height = 2 * MOS_ALIGN_CEIL((m_downscaledHeightInMb4x * 4), 8);
    size = width * height;

    MOS_ZeroMemory(data,size);
    m_osInterface->pfnUnlockResource(
        m_osInterface, &m_brcBuffers.sMeBrcDistortionBuffer.OsResource);

    return status;
}

MOS_STATUS CodechalEncodeVp8G9::InitMBEncConstantBuffer(struct CodechalVp8InitMbencConstantBufferParams* params)
{
    uint8_t         *data;
    MOS_LOCK_PARAMS lockFlagsWriteOnly;
    MOS_STATUS      status = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pOsInterface);

    MOS_ZeroMemory(&lockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
    lockFlagsWriteOnly.WriteOnly = 1;

    /* First copy MB_MODE_COST_LUMA table */
    data = (uint8_t *)params->pOsInterface->pfnLockResource(
        params->pOsInterface,
        &params->sMBModeCostLumaBuffer.OsResource,
        &lockFlagsWriteOnly);
    CODECHAL_ENCODE_CHK_NULL_RETURN(data);

    MOS_ZeroMemory(data, params->sMBModeCostLumaBuffer.dwPitch * params->sMBModeCostLumaBuffer.dwHeight);

    // Fill surface with VP8_MB_MODE_COST_LUMA table for I frame
    status = MOS_SecureMemcpy(
        data,
        sizeof(VP8_MB_MODE_COST_LUMA_G9),
        (void *)VP8_MB_MODE_COST_LUMA_G9,
        sizeof(VP8_MB_MODE_COST_LUMA_G9));
    if(status != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
        return status;
    }

    params->pOsInterface->pfnUnlockResource(
        params->pOsInterface,
        &params->sMBModeCostLumaBuffer.OsResource);

    /* Now copy BLOCK_MODE_COST table */
    data = (uint8_t *)params->pOsInterface->pfnLockResource(
        params->pOsInterface,
        &params->sBlockModeCostBuffer.OsResource,
        &lockFlagsWriteOnly);
    CODECHAL_ENCODE_CHK_NULL_RETURN(data);

    MOS_ZeroMemory(data, params->sBlockModeCostBuffer.dwPitch * params->sBlockModeCostBuffer.dwHeight);

    // Fill surface with VP8_BLOCK_MODE_COST table for I frame
    status = MOS_SecureMemcpy(
        data,
        sizeof(VP8_BLOCK_MODE_COST_G9),
        (void *)VP8_BLOCK_MODE_COST_G9,
        sizeof(VP8_BLOCK_MODE_COST_G9));
    if(status != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
        return status;
    }

    params->pOsInterface->pfnUnlockResource(
        params->pOsInterface,
        &params->sBlockModeCostBuffer.OsResource);

    return status;
}

MOS_STATUS CodechalEncodeVp8G9::InitMpuTpuBuffer()
{
    uint8_t         *data = nullptr;
    uint8_t         *origData = nullptr;
    MOS_LOCK_PARAMS lockFlagsWriteOnly;
    MOS_STATUS      status = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_osInterface);

    MOS_ZeroMemory(&lockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
    lockFlagsWriteOnly.WriteOnly = 1;

    // Last 12 bytes of LFDeltas to be initialized by kernel
    origData = data = (uint8_t *)m_osInterface->pfnLockResource(
        m_osInterface,
        &m_mpuTpuBuffers.resModeProbs,
        &lockFlagsWriteOnly);
    CODECHAL_ENCODE_CHK_NULL_RETURN(data);

    MOS_ZeroMemory(data, MODE_PROPABILITIES_SIZE);

    m_osInterface->pfnUnlockResource(
        m_osInterface,
        &m_mpuTpuBuffers.resModeProbs);

    data = nullptr;

    data = (uint8_t *)m_osInterface->pfnLockResource(
        m_osInterface,
        &m_mpuTpuBuffers.resRefModeProbs,
        &lockFlagsWriteOnly);
    CODECHAL_ENCODE_CHK_NULL_RETURN(data);

    MOS_ZeroMemory(data, MODE_PROPABILITIES_SIZE);

    m_osInterface->pfnUnlockResource(
        m_osInterface,
        &m_mpuTpuBuffers.resRefModeProbs);

    data = nullptr;

    // Fill surface with VP8_DEFAULT_COEFF_PROBS_G9 table
    data = (uint8_t *)m_osInterface->pfnLockResource(
        m_osInterface,
        &m_mpuTpuBuffers.resRefCoeffProbs,
        &lockFlagsWriteOnly);
    CODECHAL_ENCODE_CHK_NULL_RETURN(data);

    status = MOS_SecureMemcpy(
        data,
        sizeof(VP8_DEFAULT_COEFF_PROBS_G9),
        (void *)VP8_DEFAULT_COEFF_PROBS_G9,
        sizeof(VP8_DEFAULT_COEFF_PROBS_G9));
    if(status != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
        return status;
    }

    m_osInterface->pfnUnlockResource(
        m_osInterface,
        &m_mpuTpuBuffers.resRefCoeffProbs);

    data = nullptr;

    // Init Entropy cost surface
    data = (uint8_t *)m_osInterface->pfnLockResource(
        m_osInterface,
        &m_mpuTpuBuffers.resEntropyCostTable,
        &lockFlagsWriteOnly);
    CODECHAL_ENCODE_CHK_NULL_RETURN(data);

    status = MOS_SecureMemcpy(
        data,
        sizeof(VP8_C0_TABLE),
        (void *)VP8_C0_TABLE,
        sizeof(VP8_C0_TABLE));
    if(status != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
        return status;
    }

    m_osInterface->pfnUnlockResource(
        m_osInterface,
        &m_mpuTpuBuffers.resEntropyCostTable);

    data = nullptr;

    // Init Token update flags surface
    data = (uint8_t *)m_osInterface->pfnLockResource(
        m_osInterface,
        &m_mpuTpuBuffers.resPakTokenUpdateFlags,
        &lockFlagsWriteOnly);
    CODECHAL_ENCODE_CHK_NULL_RETURN(data);

    status = MOS_SecureMemcpy(
        data,
        sizeof(VP8_PROBABILITY_UPDATE_FLAGS_G9),
        (void *)VP8_PROBABILITY_UPDATE_FLAGS_G9,
        sizeof(VP8_PROBABILITY_UPDATE_FLAGS_G9));
    if(status != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
        return status;
    }

    m_osInterface->pfnUnlockResource(
        m_osInterface,
        &m_mpuTpuBuffers.resPakTokenUpdateFlags);

    data = nullptr;

    // Init Token update flags surface
    data = (uint8_t *)m_osInterface->pfnLockResource(
        m_osInterface,
        &m_mpuTpuBuffers.resDefaultTokenProbability,
        &lockFlagsWriteOnly);
    CODECHAL_ENCODE_CHK_NULL_RETURN(data);

    status = MOS_SecureMemcpy(
        data,
        sizeof(VP8_COEFF_UPDATE_PROBS_G9),
        (void *)VP8_COEFF_UPDATE_PROBS_G9,
        sizeof(VP8_COEFF_UPDATE_PROBS_G9));

    m_osInterface->pfnUnlockResource(
        m_osInterface,
        &m_mpuTpuBuffers.resDefaultTokenProbability);

    data = nullptr;

    // Init key frame prob
    data = (uint8_t *)m_osInterface->pfnLockResource(
        m_osInterface,
        &m_mpuTpuBuffers.resKeyFrameTokenProbability,
        &lockFlagsWriteOnly);
    CODECHAL_ENCODE_CHK_NULL_RETURN(data);

    status = MOS_SecureMemcpy(
        data,
        sizeof(VP8_DEFAULT_COEFF_PROBS_G9),
        (void *)VP8_DEFAULT_COEFF_PROBS_G9,
        sizeof(VP8_DEFAULT_COEFF_PROBS_G9));
    if(status != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
        return status;
    }

    m_osInterface->pfnUnlockResource(
        m_osInterface,
        &m_mpuTpuBuffers.resKeyFrameTokenProbability);

    data = nullptr;

    // Init updated frame token probability
    data = (uint8_t *)m_osInterface->pfnLockResource(
        m_osInterface,
        &m_mpuTpuBuffers.resUpdatedTokenProbability,
        &lockFlagsWriteOnly);
    CODECHAL_ENCODE_CHK_NULL_RETURN(data);

    status = MOS_SecureMemcpy(
        data,
        sizeof(VP8_DEFAULT_COEFF_PROBS_G9),
        (void *)VP8_DEFAULT_COEFF_PROBS_G9,
        sizeof(VP8_DEFAULT_COEFF_PROBS_G9));

    if (status != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
        return status;
    }

    m_osInterface->pfnUnlockResource(
        m_osInterface,
        &m_mpuTpuBuffers.resUpdatedTokenProbability);

    data = nullptr;

    return status;
}

MOS_STATUS CodechalEncodeVp8G9::KeyFrameUpdateMpuTpuBuffer(struct CodechalVp8UpdateMpuTpuBufferParams* params)
{
    uint8_t         *data=nullptr;
    uint8_t         *origData=nullptr;
    MOS_LOCK_PARAMS lockFlagsWriteOnly;
    MOS_STATUS      status = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pOsInterface);

    MOS_ZeroMemory(&lockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
    lockFlagsWriteOnly.WriteOnly = 1;

    // Copy Key frame surface to cur probability surface
    origData = (uint8_t *)params->pOsInterface->pfnLockResource(
        params->pOsInterface,
        params->presKeyFrameTokenProbability,
        &lockFlagsWriteOnly);
    CODECHAL_ENCODE_CHK_NULL_RETURN(origData);

    data = (uint8_t *)params->pOsInterface->pfnLockResource(
        params->pOsInterface,
        params->presCurrFrameTokenProbability,
        &lockFlagsWriteOnly);
    CODECHAL_ENCODE_CHK_NULL_RETURN(data);

    status = MOS_SecureMemcpy(
        data,
        params->dwCoeffProbsSize,
        origData,
        params->dwCoeffProbsSize);

    if(status != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
        return status;
    }

    params->pOsInterface->pfnUnlockResource(
        params->pOsInterface,
        params->presCurrFrameTokenProbability);

    params->pOsInterface->pfnUnlockResource(
        params->pOsInterface,
        params->presKeyFrameTokenProbability);

    data = nullptr;

    // Fill surface with VP8_DEFAULT_COEFF_PROBS_G9 table
    data = (uint8_t *)params->pOsInterface->pfnLockResource(
        params->pOsInterface,
        params->presHwTokenProbabilityPass1,
        &lockFlagsWriteOnly);
    CODECHAL_ENCODE_CHK_NULL_RETURN(data);

    status = MOS_SecureMemcpy(
        data,
        sizeof(VP8_DEFAULT_COEFF_PROBS_G9),
        (void *)VP8_DEFAULT_COEFF_PROBS_G9,
        sizeof(VP8_DEFAULT_COEFF_PROBS_G9));

    if(status != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
        return status;
    }

    params->pOsInterface->pfnUnlockResource(
        params->pOsInterface,
        params->presHwTokenProbabilityPass1);
    data = nullptr;

    data = (uint8_t *)params->pOsInterface->pfnLockResource(
        params->pOsInterface,
        params->presHwTokenProbabilityPass2,
        &lockFlagsWriteOnly);
    CODECHAL_ENCODE_CHK_NULL_RETURN(data);

    status = MOS_SecureMemcpy(
        data,
        sizeof(VP8_DEFAULT_COEFF_PROBS_G9),
        (void *)VP8_DEFAULT_COEFF_PROBS_G9,
        sizeof(VP8_DEFAULT_COEFF_PROBS_G9));

    if(status != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
        return status;
    }

    params->pOsInterface->pfnUnlockResource(
        params->pOsInterface,
        params->presHwTokenProbabilityPass2);
    data = nullptr;

    return status;
}

MOS_STATUS CodechalEncodeVp8G9::SetBrcInitResetCurbe(struct CodechalVp8BrcInitResetCurbeParams* params)
{
    struct MediaObjectVp8BrcInitResetStaticDataG9      cmd;
    PCODEC_VP8_ENCODE_PIC_PARAMS                        picParams;
    PCODEC_VP8_ENCODE_SEQUENCE_PARAMS                   seqParams;
    PMHW_STATE_HEAP_INTERFACE                           stateHeapInterface;
    double                                              inputBitsPerFrame, bpsRatio;
    MOS_STATUS                                          status = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface->GetRenderInterface());
    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pPicParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pSeqParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pKernelState);

    stateHeapInterface = m_hwInterface->GetRenderInterface()->m_stateHeapInterface;
    CODECHAL_ENCODE_CHK_NULL_RETURN(stateHeapInterface);

    picParams = params->pPicParams;
    seqParams = params->pSeqParams;

    MOS_ZeroMemory(&cmd, sizeof(cmd));

    cmd.DW1.InitBufFullInBits = seqParams->InitVBVBufferFullnessInBit;
    cmd.DW2.BufSizeInBits = seqParams->VBVBufferSizeInBit;
    cmd.DW3.AverageBitRate = seqParams->TargetBitRate[seqParams->NumTemporalLayersMinus1] * CODECHAL_ENCODE_BRC_KBPS;    //DDI in Kbits
    cmd.DW4.MaxBitRate = seqParams->MaxBitRate * CODECHAL_ENCODE_BRC_KBPS;
    cmd.DW8.GopP = seqParams->GopPicSize - 1;
    cmd.DW9.FrameWidthInBytes = params->dwFrameWidth;
    cmd.DW10.FrameHeightInBytes = params->dwFrameHeight;
    cmd.DW10.AVBRAccuracy = 30;

    // change FrameRateD from 1 to 100 instead of dividing FramesPer100Sec by 100 in DDI
    // this is needed to handle precision issue with decimal frame rate ex) 29.97 fps
    cmd.DW6.FrameRateM = seqParams->FramesPer100Sec[seqParams->NumTemporalLayersMinus1];
    cmd.DW7.FrameRateD = 100;   // change FrameRateD from 1 to 100 instead of dividing FramesPer100Sec by 100 in DDI
    cmd.DW8.BRCFlag = 0;

    if (seqParams->RateControlMethod == RATECONTROL_CBR)
    {
        cmd.DW4.MaxBitRate = cmd.DW3.AverageBitRate;
        cmd.DW8.BRCFlag = cmd.DW8.BRCFlag | CODECHAL_ENCODE_BRCINIT_ISCBR;
    }
    else if (seqParams->RateControlMethod == RATECONTROL_VBR)
    {
        if (cmd.DW4.MaxBitRate < cmd.DW3.AverageBitRate)
        {
            cmd.DW4.MaxBitRate = 2 * cmd.DW3.AverageBitRate;
        }
        cmd.DW8.BRCFlag = cmd.DW8.BRCFlag | CODECHAL_ENCODE_BRCINIT_ISVBR;
    }

    // Profile & level max frame size
    // DW0 is DDI param seq param.
    cmd.DW0.ProfileLevelMaxFrame = params->dwFrameWidth * params->dwFrameHeight;

    // Set dynamic thresholds
    inputBitsPerFrame =
        ((double)(cmd.DW4.MaxBitRate) * (double)(cmd.DW7.FrameRateD) /
        (double)(cmd.DW6.FrameRateM));

    if (cmd.DW2.BufSizeInBits < (uint32_t)inputBitsPerFrame * 4)
    {
        cmd.DW2.BufSizeInBits = (uint32_t)inputBitsPerFrame * 4;
    }

    if (cmd.DW1.InitBufFullInBits == 0)
    {
        cmd.DW1.InitBufFullInBits = 7 * cmd.DW2.BufSizeInBits / 8;
    }
    if (cmd.DW1.InitBufFullInBits < (uint32_t)(inputBitsPerFrame * 2))
    {
        cmd.DW1.InitBufFullInBits = (uint32_t)(inputBitsPerFrame * 2);
    }
    if (cmd.DW1.InitBufFullInBits > cmd.DW2.BufSizeInBits)
    {
        cmd.DW1.InitBufFullInBits = cmd.DW2.BufSizeInBits;
    }

    bpsRatio = inputBitsPerFrame / ((double)(cmd.DW2.BufSizeInBits) / 30);
    bpsRatio = (bpsRatio < 0.1) ? 0.1 : (bpsRatio > 3.5) ? 3.5 : bpsRatio;

    cmd.DW9.FrameWidthInBytes = seqParams->FrameWidth;
    cmd.DW10.FrameHeightInBytes = seqParams->FrameHeight;
    cmd.DW10.AVBRAccuracy = 30;
    cmd.DW11.AVBRConvergence = 150;
    cmd.DW11.MinQP = picParams->ClampQindexLow;
    cmd.DW12.MaxQP = picParams->ClampQindexHigh;

    cmd.DW12.LevelQP = 60;
    // DW13 default 100
    cmd.DW13.MaxSection_pct = 100;
    cmd.DW13.OverShootCBR_pct = 115;

    // DW14 default 100
    cmd.DW14.MinSection_pct = 100;
    cmd.DW14.VBRBias_pct = 100;
    cmd.DW15.InstantRateThreshold0ForP = 30;
    cmd.DW15.InstantRateThreshold1ForP = 50;
    cmd.DW15.InstantRateThreshold2ForP = 70;
    cmd.DW15.InstantRateThreshold3ForP = 120;

    cmd.DW17.InstantRateThreshold0ForI = 30;
    cmd.DW17.InstantRateThreshold1ForI = 50;
    cmd.DW17.InstantRateThreshold2ForI = 90;
    cmd.DW17.InstantRateThreshold3ForI = 115;
    cmd.DW18.DeviationThreshold0ForP = (uint32_t)(-50 * pow(0.9, bpsRatio));
    cmd.DW18.DeviationThreshold1ForP = (uint32_t)(-50 * pow(0.66, bpsRatio));
    cmd.DW18.DeviationThreshold2ForP = (uint32_t)(-50 * pow(0.46, bpsRatio));
    cmd.DW18.DeviationThreshold3ForP = (uint32_t)(-50 * pow(0.3, bpsRatio));
    cmd.DW19.DeviationThreshold4ForP = (uint32_t)(50 * pow(0.3, bpsRatio));
    cmd.DW19.DeviationThreshold5ForP = (uint32_t)(50 * pow(0.46, bpsRatio));
    cmd.DW19.DeviationThreshold6ForP = (uint32_t)(50 * pow(0.7, bpsRatio));
    cmd.DW19.DeviationThreshold7ForP = (uint32_t)(50 * pow(0.9, bpsRatio));
    cmd.DW20.DeviationThreshold0ForVBR = (uint32_t)(-50 * pow(0.9, bpsRatio));
    cmd.DW20.DeviationThreshold1ForVBR = (uint32_t)(-50 * pow(0.7, bpsRatio));
    cmd.DW20.DeviationThreshold2ForVBR = (uint32_t)(-50 * pow(0.5, bpsRatio));
    cmd.DW20.DeviationThreshold3ForVBR = (uint32_t)(-50 * pow(0.3, bpsRatio));
    cmd.DW21.DeviationThreshold4ForVBR = (uint32_t)(100 * pow(0.4, bpsRatio));
    cmd.DW21.DeviationThreshold5ForVBR = (uint32_t)(100 * pow(0.5, bpsRatio));
    cmd.DW21.DeviationThreshold6ForVBR = (uint32_t)(100 * pow(0.75, bpsRatio));
    cmd.DW21.DeviationThreshold7ForVBR = (uint32_t)(100 * pow(0.9, bpsRatio));
    cmd.DW22.DeviationThreshold0ForI = (uint32_t)(-50 * pow(0.8, bpsRatio));
    cmd.DW22.DeviationThreshold1ForI = (uint32_t)(-50 * pow(0.6, bpsRatio));
    cmd.DW22.DeviationThreshold2ForI = (uint32_t)(-50 * pow(0.34, bpsRatio));
    cmd.DW22.DeviationThreshold3ForI = (uint32_t)(-50 * pow(0.2, bpsRatio));
    cmd.DW23.DeviationThreshold4ForI = (uint32_t)(50 * pow(0.2, bpsRatio));
    cmd.DW23.DeviationThreshold5ForI = (uint32_t)(50 * pow(0.4, bpsRatio));
    cmd.DW23.DeviationThreshold6ForI = (uint32_t)(50 * pow(0.66, bpsRatio));
    cmd.DW23.DeviationThreshold7ForI = (uint32_t)(50 * pow(0.9, bpsRatio));
    cmd.DW24.NumTLevels = seqParams->NumTemporalLayersMinus1 + 1;

    if( seqParams->NumTemporalLayersMinus1 > 0)
    {
        uint32_t tempBitRate[NAX_NUM_TEMPORAL_LAYERS];

        CODECHAL_ENCODE_CHK_STATUS_RETURN(CalMaxLevelRatioForTL(
            seqParams->FramesPer100Sec,
            seqParams->TargetBitRate,
            (uint32_t)seqParams->NumTemporalLayersMinus1,
            tempBitRate));

        cmd.DW24.INITBCK_MaxLevel_Ratio_U8_Layer0 = uint32_t(tempBitRate[0]);
        cmd.DW24.INITBCK_MaxLevel_Ratio_U8_Layer1 = uint32_t(tempBitRate[1]);
        cmd.DW24.INITBCK_MaxLevel_Ratio_U8_Layer2 = uint32_t(tempBitRate[2]);
        cmd.DW25.INITBCK_MaxLevel_Ratio_U8_Layer3 = uint32_t(tempBitRate[3]);
    }
    else
    {
        cmd.DW24.INITBCK_MaxLevel_Ratio_U8_Layer0 = 0;
        cmd.DW24.INITBCK_MaxLevel_Ratio_U8_Layer1 = 0;
        cmd.DW24.INITBCK_MaxLevel_Ratio_U8_Layer2 = 0;
        cmd.DW25.INITBCK_MaxLevel_Ratio_U8_Layer3 = 0;
    }

    if (params->bInitBrc)
    {
        *params->pdBrcInitCurrentTargetBufFullInBits = cmd.DW1.InitBufFullInBits;
    }

    *params->pdwBrcInitResetBufSizeInBits = cmd.DW2.BufSizeInBits;
    *params->pdBrcInitResetInputBitsPerFrame = inputBitsPerFrame;

    cmd.DW26.HistoryBufferBTI = CODECHAL_VP8_BRC_INIT_RESET_HISTORY_G9;
    cmd.DW27.DistortionBufferBTI = CODECHAL_VP8_BRC_INIT_RESET_DISTORTION_G9;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(params->pKernelState->m_dshRegion.AddData(
        &cmd,
        params->pKernelState->dwCurbeOffset,
        sizeof(cmd)));

    return status;
}

MOS_STATUS CodechalEncodeVp8G9::SendBrcInitResetSurfaces(
    PMOS_COMMAND_BUFFER                         cmdBuffer,
    struct CodechalVp8BrcInitResetSurfaceParams* params)
{
    uint32_t                        size;
    PMHW_KERNEL_STATE               kernelState;
    CODECHAL_SURFACE_CODEC_PARAMS   surfaceCodecParams;
    MOS_STATUS                      status = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->presBrcHistoryBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pKernelState);

    kernelState = params->pKernelState;

    // BRC history buffer
    size = ENCODE_VP8_BRC_HISTORY_BUFFER_SIZE;
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.bIsWritable = true;
    surfaceCodecParams.presBuffer = params->presBrcHistoryBuffer;
    surfaceCodecParams.dwSize = size;
    surfaceCodecParams.dwBindingTableOffset = CODECHAL_VP8_BRC_INIT_RESET_HISTORY_G9;
    surfaceCodecParams.bRenderTarget = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Distortion Surface (output)
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.bIs2DSurface = true;
    surfaceCodecParams.bMediaBlockRW = true;
    surfaceCodecParams.psSurface = params->psMeBrcDistortionBuffer;
    surfaceCodecParams.dwBindingTableOffset = CODECHAL_VP8_BRC_INIT_RESET_DISTORTION_G9;
    surfaceCodecParams.bIsWritable = true;
    surfaceCodecParams.bRenderTarget = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    return status;
}

MOS_STATUS CodechalEncodeVp8G9::SetBrcUpdateCurbe(struct CodechalVp8BrcUpdateCurbeParams* params)
{
    struct MediaObjectVp8BrcUpdateStaticDataG9     cmd;
    PCODEC_VP8_ENCODE_PIC_PARAMS                    picParams;
    PCODEC_VP8_ENCODE_SEQUENCE_PARAMS               seqParams;
    PCODEC_VP8_ENCODE_QUANT_DATA                    vp8QuantData;
    PMHW_STATE_HEAP_INTERFACE                       stateHeapInterface;
    MOS_STATUS                                      status = MOS_STATUS_SUCCESS;
    uint32_t                                        i;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface->GetRenderInterface());
    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pPicParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pSeqParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pVp8QuantData);

    stateHeapInterface =
        m_hwInterface->GetRenderInterface()->m_stateHeapInterface;
    CODECHAL_ENCODE_CHK_NULL_RETURN(stateHeapInterface);

    picParams = params->pPicParams;
    seqParams = params->pSeqParams;
    vp8QuantData = params->pVp8QuantData;

    MOS_ZeroMemory(&cmd, sizeof(cmd));

    cmd.DW0.TargetSize = 0;
    cmd.DW2.PictureHeaderSize = 0;   // matching kernel value
    cmd.DW5.TargetSizeFlag = 0;

    if (*params->pdBrcInitCurrentTargetBufFullInBits > (double)params->dwBrcInitResetBufSizeInBits)
    {
        *params->pdBrcInitCurrentTargetBufFullInBits -= (double)params->dwBrcInitResetBufSizeInBits;
        cmd.DW5.TargetSizeFlag = 1;
    }

    cmd.DW0.TargetSize = (uint32_t)(*params->pdBrcInitCurrentTargetBufFullInBits);

    cmd.DW5.CurrFrameType = (params->wPictureCodingType == I_TYPE) ? 2 : 0;
    cmd.DW5.BRCFlag = 16 * seqParams->RateControlMethod;
    cmd.DW5.MaxNumPAKs = params->dwVp8BrcNumPakPasses;

    cmd.DW6.TID = picParams->temporal_id;                          // Default: 0 - if temporal scalability is not supported
    cmd.DW6.NumTLevels = seqParams->NumTemporalLayersMinus1 + 1;   // Default: 1 - if temporal scalability is not supported

    cmd.DW8.StartGlobalAdjustMult0 = 1;
    cmd.DW8.StartGlobalAdjustMult1 = 1;
    cmd.DW8.StartGlobalAdjustMult2 = 3;
    cmd.DW8.StartGlobalAdjustMult3 = 2;

    cmd.DW9.StartGlobalAdjustDiv0 = 40;
    cmd.DW9.StartGlobalAdjustDiv1 = 5;
    cmd.DW9.StartGlobalAdjustDiv2 = 5;
    cmd.DW9.StartGlobalAdjustMult4 = 1;

    cmd.DW10.StartGlobalAdjustDiv3 = 3;
    cmd.DW10.StartGlobalAdjustDiv4 = 1;
    cmd.DW10.QPThreshold0 = 20;//7;
    cmd.DW10.QPThreshold1 = 40;//18;

    cmd.DW11.QPThreshold2 = 60;//25;
    cmd.DW11.QPThreshold3 = 90;//37;
    cmd.DW11.gRateRatioThreshold0 = 40;
    cmd.DW11.gRateRatioThreshold1 = 75;

    cmd.DW12.gRateRatioThreshold2 = 97;
    cmd.DW12.gRateRatioThreshold3 = 103;
    cmd.DW12.gRateRatioThreshold4 = 125;
    cmd.DW12.gRateRatioThreshold5 = 160;

    cmd.DW13.gRateRatioThresholdQP0 = MOS_BITFIELD_VALUE((uint32_t)-3, 8);
    cmd.DW13.gRateRatioThresholdQP1 = MOS_BITFIELD_VALUE((uint32_t)-2, 8);
    cmd.DW13.gRateRatioThresholdQP2 = MOS_BITFIELD_VALUE((uint32_t)-1, 8);
    cmd.DW13.gRateRatioThresholdQP3 = 0;

    cmd.DW14.gRateRatioThresholdQP4 = 1;
    cmd.DW14.gRateRatioThresholdQP5 = 2;
    cmd.DW14.gRateRatioThresholdQP6 = 3;
    cmd.DW14.IndexOfPreviousQP = 0;

    *params->pdBrcInitCurrentTargetBufFullInBits += params->dBrcInitResetInputBitsPerFrame;

    cmd.DW3.startGAdjFrame0 = 10;
    cmd.DW3.startGAdjFrame1 = 50;
    cmd.DW4.startGAdjFrame2 = 100;
    cmd.DW4.startGAdjFrame3 = 150;
    cmd.DW11.gRateRatioThreshold0 = 40;

    cmd.DW11.gRateRatioThreshold1 = 75;

    cmd.DW12.gRateRatioThreshold2 = 97;
    cmd.DW12.gRateRatioThreshold3 = 103;
    cmd.DW12.gRateRatioThreshold4 = 125;
    cmd.DW12.gRateRatioThreshold5 = 160;

    cmd.DW13.gRateRatioThresholdQP0 = MOS_BITFIELD_VALUE((uint32_t)-3, 8);
    cmd.DW13.gRateRatioThresholdQP1 = MOS_BITFIELD_VALUE((uint32_t)-2, 8);
    cmd.DW13.gRateRatioThresholdQP2 = MOS_BITFIELD_VALUE((uint32_t)-1, 8);
    cmd.DW13.gRateRatioThresholdQP3 = 0;

    cmd.DW14.gRateRatioThresholdQP4 = 1;
    cmd.DW14.gRateRatioThresholdQP5 = 2;
    cmd.DW14.gRateRatioThresholdQP6 = 3;
    cmd.DW14.IndexOfPreviousQP = 0;

    // 16bits are used instead of 8bits to fix 4K related issue
    cmd.DW15.FrameWidthInMB = params->dwFrameWidthInMB;
    cmd.DW15.FrameHeightInMB = params->dwFrameHeightInMB;

    cmd.DW16.PFrameQPSeg0 = vp8QuantData->QIndex[0];
    cmd.DW16.PFrameQPSeg1 = vp8QuantData->QIndex[1];
    cmd.DW16.PFrameQPSeg2 = vp8QuantData->QIndex[2];
    cmd.DW16.PFrameQPSeg3 = vp8QuantData->QIndex[3];

    cmd.DW17.KeyFrameQPSeg0 = vp8QuantData->QIndex[0];
    cmd.DW17.KeyFrameQPSeg1 = vp8QuantData->QIndex[1];
    cmd.DW17.KeyFrameQPSeg2 = vp8QuantData->QIndex[2];
    cmd.DW17.KeyFrameQPSeg3 = vp8QuantData->QIndex[3];

    cmd.DW18.QDeltaPlane0 = vp8QuantData->QIndexDelta[VP8_QINDEX_Y1_DC];
    cmd.DW18.QDeltaPlane1 = vp8QuantData->QIndexDelta[VP8_QINDEX_Y2_AC];
    cmd.DW18.QDeltaPlane2 = vp8QuantData->QIndexDelta[VP8_QINDEX_Y2_DC];
    cmd.DW18.QDeltaPlane3 = vp8QuantData->QIndexDelta[VP8_QINDEX_UV_AC];

    cmd.DW19.QDeltaPlane4 = vp8QuantData->QIndexDelta[VP8_QINDEX_UV_DC];

    // temporal scaliability is enable
    cmd.DW19.MainRef = 0;
    cmd.DW19.RefFrameFlags = 0;

    if (params->wPictureCodingType == P_TYPE) // P frame
    {
        if (seqParams->NumTemporalLayersMinus1 > 0)
        {
            uint32_t m_ref_frame_flags = picParams->ref_frame_ctrl;
            uint8_t m_rfo[3];

            m_rfo[2] = picParams->first_ref;
            m_rfo[1] = picParams->second_ref;
            m_rfo[0] = 6 - m_rfo[2] - m_rfo[1];

            uint32_t MainRef = 0;
            uint32_t k = 0;

            for (i = 0; i <3; i++)
            {
                if (m_ref_frame_flags & (0x1 << (m_rfo[i] - 1))) {
                    MainRef |= (m_rfo[i] << (2 * k));
                    k++;
                }
            }

            cmd.DW19.MainRef = MainRef;
            cmd.DW19.RefFrameFlags = m_ref_frame_flags;
        }
        else {
            // original one for MainRef and RefFlag
            cmd.DW19.MainRef = VP8_MAINREF_TABLE_G9[picParams->ref_frame_ctrl];
            cmd.DW19.RefFrameFlags = picParams->ref_frame_ctrl;
        }
    }

    cmd.DW20.SegOn = picParams->segmentation_enabled;
    cmd.DW20.BRCMethod = seqParams->RateControlMethod;
    // DDI Seq Parameter
    // 0: Default, decided internally based on target usage.
    // 1: MB BRC enabled.
    // 2: MB BRC disabled.
    // Curbe 1: MBRC on, 0: MBRC off
    cmd.DW20.MBRC = (seqParams->MBBRC == 1)? 1 : 0;
    cmd.DW20.VMEIntraPrediction = (params->ucKernelMode == encodePerformanceMode) ? 1 : 0;

    cmd.DW22.HistorytBufferBTI = CODECHAL_VP8_BRC_UPDATE_HISTORY_G9;
    cmd.DW23.PakStatisticsBTI = CODECHAL_VP8_BRC_UPDATE_PAK_STATISTICS_OUTPUT_G9;
    cmd.DW24.MfxVp8EncoderCfgReadBTI = CODECHAL_VP8_BRC_UPDATE_MFX_ENCODER_CFG_READ_G9;
    cmd.DW25.MfxVp8EncoderCfgWriteBTI = CODECHAL_VP8_BRC_UPDATE_MFX_ENCODER_CFG_WRITE_G9;
    cmd.DW26.MBEncCurbeReadBTI = CODECHAL_VP8_BRC_UPDATE_MBENC_CURBE_READ_G9;
    cmd.DW27.MBEncCurbeWriteBTI = CODECHAL_VP8_BRC_UPDATE_MBENC_CURBE_WRITE_G9;
    cmd.DW28.DistortionBTI = CODECHAL_VP8_BRC_UPDATE_DISTORTION_SURFACE_G9;
    cmd.DW29.ConstantDataBTI = CODECHAL_VP8_BRC_UPDATE_CONSTANT_DATA_G9;
    cmd.DW30.SegmentMapBTI = CODECHAL_VP8_BRC_UPDATE_SEGMENT_MAP_G9;
    cmd.DW31.MpuCurbeReadBTI = CODECHAL_VP8_BRC_UPDATE_MPU_CURBE_READ_G9;
    cmd.DW32.MpuCurbeWriteBTI = CODECHAL_VP8_BRC_UPDATE_MPU_CURBE_WRITE_G9;
    cmd.DW33.TpuCurbeReadBTI = CODECHAL_VP8_BRC_UPDATE_TPU_CURBE_READ_G9;
    cmd.DW34.TpuCurbeWriteBTI = CODECHAL_VP8_BRC_UPDATE_TPU_CURBE_WRITE_G9;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(
        m_brcKernelStates[CODECHAL_ENCODE_VP8_BRC_IDX_UPDATE].m_dshRegion.AddData(
            &cmd,
            m_brcKernelStates[CODECHAL_ENCODE_VP8_BRC_IDX_UPDATE].dwCurbeOffset,
            sizeof(cmd)));

    return status;
}

MOS_STATUS CodechalEncodeVp8G9::SetMbEncCurbe(struct CodechalVp8MbencCurbeParams* params)
{
    PCODEC_VP8_ENCODE_PIC_PARAMS                picParams;
    PCODEC_VP8_ENCODE_SEQUENCE_PARAMS           seqParams;
    PCODEC_VP8_ENCODE_QUANT_DATA                vp8QuantData;
    PMHW_STATE_HEAP_INTERFACE                   stateHeapInterface;
    MOS_STATUS                                  status = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface->GetRenderInterface());
    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pPicParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pSeqParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pVp8QuantData);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pKernelState);

    stateHeapInterface =
        m_hwInterface->GetRenderInterface()->m_stateHeapInterface;
    CODECHAL_ENCODE_CHK_NULL_RETURN(stateHeapInterface);

    picParams = params->pPicParams;
    seqParams = params->pSeqParams;
    vp8QuantData = params->pVp8QuantData;

    if (params->wPictureCodingType == I_TYPE)
    {
        struct MediaObjectVp8MbencIStaticDataG9 cmd;
        int16_t usQi_Y_Dc, usQi_UV_Dc, usQi_UV_Ac;

        MOS_ZeroMemory(&cmd, sizeof(cmd));

        cmd.DW0.FrameWidth = (seqParams->FrameWidth + 15) & (~0xF);  /* Kernel require MB boundary aligned dimensions */
        cmd.DW0.FrameHeight = (seqParams->FrameHeight + 15) & (~0xF);

        cmd.DW1.FrameType = 0; /* key frame I-Frame */
        cmd.DW1.EnableSegmentation = picParams->segmentation_enabled;
        cmd.DW1.EnableHWIntraPrediction = (params->ucKernelMode == encodePerformanceMode) ? 1 : 0;
        cmd.DW1.EnableEnableChromaIPEnhancement = 1; /* Always enabled and cannot be disabled */
        cmd.DW1.EnableDebugDumps = 0;
        cmd.DW1.EnableMPUHistogramUpdate = 1;
        cmd.DW1.VMEDistortionMeasure = 2;
        cmd.DW1.VMEEnableTMCheck = 0;

        usQi_Y_Dc = vp8QuantData->QIndex[0] + vp8QuantData->QIndexDelta[VP8_QINDEX_Y1_DC];
        usQi_Y_Dc = usQi_Y_Dc < 0 ? 0 : (usQi_Y_Dc > CODECHAL_VP8_QP_MAX ? CODECHAL_VP8_QP_MAX : usQi_Y_Dc);
        cmd.DW2.LambdaSegment0 = (uint16_t)((VP8_QUANT_DC_G9[usQi_Y_Dc] * VP8_QUANT_DC_G9[usQi_Y_Dc]) / 4);
        if (picParams->segmentation_enabled)
        {
            usQi_Y_Dc = vp8QuantData->QIndex[1] + vp8QuantData->QIndexDelta[VP8_QINDEX_Y1_DC];
            usQi_Y_Dc = usQi_Y_Dc < 0 ? 0 : (usQi_Y_Dc > CODECHAL_VP8_QP_MAX ? CODECHAL_VP8_QP_MAX : usQi_Y_Dc);
            cmd.DW2.LambdaSegment1 = (uint16_t)((VP8_QUANT_DC_G9[usQi_Y_Dc] * VP8_QUANT_DC_G9[usQi_Y_Dc]) / 4);

            usQi_Y_Dc = vp8QuantData->QIndex[2] + vp8QuantData->QIndexDelta[VP8_QINDEX_Y1_DC];
            usQi_Y_Dc = usQi_Y_Dc < 0 ? 0 : (usQi_Y_Dc > CODECHAL_VP8_QP_MAX ? CODECHAL_VP8_QP_MAX : usQi_Y_Dc);
            cmd.DW3.LambdaSegment2 = (uint16_t)((VP8_QUANT_DC_G9[usQi_Y_Dc] * VP8_QUANT_DC_G9[usQi_Y_Dc]) / 4);

            usQi_Y_Dc = vp8QuantData->QIndex[3] + vp8QuantData->QIndexDelta[VP8_QINDEX_Y1_DC];
            usQi_Y_Dc = usQi_Y_Dc < 0 ? 0 : (usQi_Y_Dc > CODECHAL_VP8_QP_MAX ? CODECHAL_VP8_QP_MAX : usQi_Y_Dc);
            cmd.DW3.LambdaSegment3 = (uint16_t)((VP8_QUANT_DC_G9[usQi_Y_Dc] * VP8_QUANT_DC_G9[usQi_Y_Dc]) / 4);
        }

        cmd.DW4.AllDCBiasSegment0 = VP8_ALL_DC_BIAS_DEFAULT;
        if (picParams->segmentation_enabled)
        {
            cmd.DW4.AllDCBiasSegment1 = VP8_ALL_DC_BIAS_DEFAULT;
            cmd.DW5.AllDCBiasSegment2 = VP8_ALL_DC_BIAS_DEFAULT;
            cmd.DW5.AllDCBiasSegment3 = VP8_ALL_DC_BIAS_DEFAULT;
        }

        usQi_UV_Dc = vp8QuantData->QIndex[0] + vp8QuantData->QIndexDelta[VP8_QINDEX_UV_DC];
        usQi_UV_Dc = usQi_UV_Dc < 0 ? 0 : (usQi_UV_Dc > CODECHAL_VP8_QP_MAX ? CODECHAL_VP8_QP_MAX : usQi_UV_Dc);
        cmd.DW6.ChromaDCDeQuantSegment0 = VP8_QUANT_DC_G9[usQi_UV_Dc];
        if (picParams->segmentation_enabled)
        {
            usQi_UV_Dc = vp8QuantData->QIndex[1] + vp8QuantData->QIndexDelta[VP8_QINDEX_UV_DC];
            usQi_UV_Dc = usQi_UV_Dc < 0 ? 0 : (usQi_UV_Dc > CODECHAL_VP8_QP_MAX ? CODECHAL_VP8_QP_MAX : usQi_UV_Dc);
            cmd.DW6.ChromaDCDeQuantSegment1 = VP8_QUANT_DC_G9[usQi_UV_Dc];
            usQi_UV_Dc = vp8QuantData->QIndex[2] + vp8QuantData->QIndexDelta[VP8_QINDEX_UV_DC];
            usQi_UV_Dc = usQi_UV_Dc < 0 ? 0 : (usQi_UV_Dc > CODECHAL_VP8_QP_MAX ? CODECHAL_VP8_QP_MAX : usQi_UV_Dc);
            cmd.DW7.ChromaDCDeQuantSegment2 = VP8_QUANT_DC_G9[usQi_UV_Dc];
            usQi_UV_Dc = vp8QuantData->QIndex[3] + vp8QuantData->QIndexDelta[VP8_QINDEX_UV_DC];
            usQi_UV_Dc = usQi_UV_Dc < 0 ? 0 : (usQi_UV_Dc > CODECHAL_VP8_QP_MAX ? CODECHAL_VP8_QP_MAX : usQi_UV_Dc);
            cmd.DW7.ChromaDCDeQuantSegment3 = VP8_QUANT_DC_G9[usQi_UV_Dc];
        }

        usQi_UV_Ac = vp8QuantData->QIndex[0] + vp8QuantData->QIndexDelta[VP8_QINDEX_UV_AC];
        usQi_UV_Ac = usQi_UV_Ac < 0 ? 0 : (usQi_UV_Ac > CODECHAL_VP8_QP_MAX ? CODECHAL_VP8_QP_MAX : usQi_UV_Ac);
        cmd.DW8.ChromaACDeQuantSegment0 = VP8_QUANT_AC_G9[usQi_UV_Ac];
        cmd.DW10.ChromaAC0Threshold0Segment0 = (uint16_t)((((((1) << 16) - 1)*1.0 / ((1 << 16) / VP8_QUANT_AC_G9[usQi_UV_Ac]) - ((48 * VP8_QUANT_AC_G9[usQi_UV_Ac]) >> 7))* (1 << 13) + 3400) / 2217.0);
        cmd.DW10.ChromaAC0Threshold1Segment0 = (uint16_t)((((((2) << 16) - 1)*1.0 / ((1 << 16) / VP8_QUANT_AC_G9[usQi_UV_Ac]) - ((48 * VP8_QUANT_AC_G9[usQi_UV_Ac]) >> 7))* (1 << 13) + 3400) / 2217.0);
        if (picParams->segmentation_enabled)
        {
            usQi_UV_Ac = vp8QuantData->QIndex[1] + vp8QuantData->QIndexDelta[VP8_QINDEX_UV_AC];
            usQi_UV_Ac = usQi_UV_Ac < 0 ? 0 : (usQi_UV_Ac > CODECHAL_VP8_QP_MAX ? CODECHAL_VP8_QP_MAX : usQi_UV_Ac);
            cmd.DW8.ChromaACDeQuantSegment1 = VP8_QUANT_AC_G9[usQi_UV_Ac];
            cmd.DW11.ChromaAC0Threshold0Segment1 = (uint16_t)((((((1) << 16) - 1)*1.0 / ((1 << 16) / VP8_QUANT_AC_G9[usQi_UV_Ac]) - ((48 * VP8_QUANT_AC_G9[usQi_UV_Ac]) >> 7))* (1 << 13) + 3400) / 2217.0);
            cmd.DW11.ChromaAC0Threshold1Segment1 = (uint16_t)((((((2) << 16) - 1)*1.0 / ((1 << 16) / VP8_QUANT_AC_G9[usQi_UV_Ac]) - ((48 * VP8_QUANT_AC_G9[usQi_UV_Ac]) >> 7))* (1 << 13) + 3400) / 2217.0);

            usQi_UV_Ac = vp8QuantData->QIndex[2] + vp8QuantData->QIndexDelta[VP8_QINDEX_UV_AC];
            usQi_UV_Ac = usQi_UV_Ac < 0 ? 0 : (usQi_UV_Ac > CODECHAL_VP8_QP_MAX ? CODECHAL_VP8_QP_MAX : usQi_UV_Ac);
            cmd.DW9.ChromaACDeQuantSegment2 = VP8_QUANT_AC_G9[usQi_UV_Ac];
            cmd.DW12.ChromaAC0Threshold0Segment2 = (uint16_t)((((((1) << 16) - 1)*1.0 / ((1 << 16) / VP8_QUANT_AC_G9[usQi_UV_Ac]) - ((48 * VP8_QUANT_AC_G9[usQi_UV_Ac]) >> 7))* (1 << 13) + 3400) / 2217.0);
            cmd.DW12.ChromaAC0Threshold1Segment2 = (uint16_t)((((((2) << 16) - 1)*1.0 / ((1 << 16) / VP8_QUANT_AC_G9[usQi_UV_Ac]) - ((48 * VP8_QUANT_AC_G9[usQi_UV_Ac]) >> 7))* (1 << 13) + 3400) / 2217.0);

            usQi_UV_Ac = vp8QuantData->QIndex[3] + vp8QuantData->QIndexDelta[VP8_QINDEX_UV_AC];
            usQi_UV_Ac = usQi_UV_Ac < 0 ? 0 : (usQi_UV_Ac > CODECHAL_VP8_QP_MAX ? CODECHAL_VP8_QP_MAX : usQi_UV_Ac);
            cmd.DW9.ChromaACDeQuantSegment3 = VP8_QUANT_AC_G9[usQi_UV_Ac];
            cmd.DW13.ChromaAC0Threshold0Segment3 = (uint16_t)((((((1) << 16) - 1)*1.0 / ((1 << 16) / VP8_QUANT_AC_G9[usQi_UV_Ac]) - ((48 * VP8_QUANT_AC_G9[usQi_UV_Ac]) >> 7))* (1 << 13) + 3400) / 2217.0);
            cmd.DW13.ChromaAC0Threshold1Segment3 = (uint16_t)((((((2) << 16) - 1)*1.0 / ((1 << 16) / VP8_QUANT_AC_G9[usQi_UV_Ac]) - ((48 * VP8_QUANT_AC_G9[usQi_UV_Ac]) >> 7))* (1 << 13) + 3400) / 2217.0);
        }

        usQi_UV_Dc = vp8QuantData->QIndex[0] + vp8QuantData->QIndexDelta[VP8_QINDEX_UV_DC];
        usQi_UV_Dc = usQi_UV_Dc < 0 ? 0 : (usQi_UV_Dc > CODECHAL_VP8_QP_MAX ? CODECHAL_VP8_QP_MAX : usQi_UV_Dc);
        cmd.DW14.ChromaDCThreshold0Segment0 = (((1) << 16) - 1) / ((1 << 16) / VP8_QUANT_DC_G9[usQi_UV_Dc]) - ((48 * VP8_QUANT_DC_G9[usQi_UV_Dc]) >> 7);
        cmd.DW14.ChromaDCThreshold1Segment0 = (((2) << 16) - 1) / ((1 << 16) / VP8_QUANT_DC_G9[usQi_UV_Dc]) - ((48 * VP8_QUANT_DC_G9[usQi_UV_Dc]) >> 7);
        cmd.DW15.ChromaDCThreshold2Segment0 = (((3) << 16) - 1) / ((1 << 16) / VP8_QUANT_DC_G9[usQi_UV_Dc]) - ((48 * VP8_QUANT_DC_G9[usQi_UV_Dc]) >> 7);
        cmd.DW15.ChromaDCThreshold3Segment0 = (((4) << 16) - 1) / ((1 << 16) / VP8_QUANT_DC_G9[usQi_UV_Dc]) - ((48 * VP8_QUANT_DC_G9[usQi_UV_Dc]) >> 7);
        if (picParams->segmentation_enabled)
        {
            usQi_UV_Dc = vp8QuantData->QIndex[1] + vp8QuantData->QIndexDelta[VP8_QINDEX_UV_DC];
            usQi_UV_Dc = usQi_UV_Dc < 0 ? 0 : (usQi_UV_Dc > CODECHAL_VP8_QP_MAX ? CODECHAL_VP8_QP_MAX : usQi_UV_Dc);
            cmd.DW16.ChromaDCThreshold0Segment1 = (((1) << 16) - 1) / ((1 << 16) / VP8_QUANT_DC_G9[usQi_UV_Dc]) - ((48 * VP8_QUANT_DC_G9[usQi_UV_Dc]) >> 7);
            cmd.DW16.ChromaDCThreshold1Segment1 = (((2) << 16) - 1) / ((1 << 16) / VP8_QUANT_DC_G9[usQi_UV_Dc]) - ((48 * VP8_QUANT_DC_G9[usQi_UV_Dc]) >> 7);
            cmd.DW17.ChromaDCThreshold2Segment1 = (((3) << 16) - 1) / ((1 << 16) / VP8_QUANT_DC_G9[usQi_UV_Dc]) - ((48 * VP8_QUANT_DC_G9[usQi_UV_Dc]) >> 7);
            cmd.DW17.ChromaDCThreshold3Segment1 = (((4) << 16) - 1) / ((1 << 16) / VP8_QUANT_DC_G9[usQi_UV_Dc]) - ((48 * VP8_QUANT_DC_G9[usQi_UV_Dc]) >> 7);

            usQi_UV_Dc = vp8QuantData->QIndex[2] + vp8QuantData->QIndexDelta[VP8_QINDEX_UV_DC];
            usQi_UV_Dc = usQi_UV_Dc < 0 ? 0 : (usQi_UV_Dc > CODECHAL_VP8_QP_MAX ? CODECHAL_VP8_QP_MAX : usQi_UV_Dc);
            cmd.DW18.ChromaDCThreshold0Segment2 = (((1) << 16) - 1) / ((1 << 16) / VP8_QUANT_DC_G9[usQi_UV_Dc]) - ((48 * VP8_QUANT_DC_G9[usQi_UV_Dc]) >> 7);
            cmd.DW18.ChromaDCThreshold1Segment2 = (((2) << 16) - 1) / ((1 << 16) / VP8_QUANT_DC_G9[usQi_UV_Dc]) - ((48 * VP8_QUANT_DC_G9[usQi_UV_Dc]) >> 7);
            cmd.DW19.ChromaDCThreshold2Segment2 = (((3) << 16) - 1) / ((1 << 16) / VP8_QUANT_DC_G9[usQi_UV_Dc]) - ((48 * VP8_QUANT_DC_G9[usQi_UV_Dc]) >> 7);
            cmd.DW19.ChromaDCThreshold3Segment2 = (((4) << 16) - 1) / ((1 << 16) / VP8_QUANT_DC_G9[usQi_UV_Dc]) - ((48 * VP8_QUANT_DC_G9[usQi_UV_Dc]) >> 7);

            usQi_UV_Dc = vp8QuantData->QIndex[3] + vp8QuantData->QIndexDelta[VP8_QINDEX_UV_DC];
            usQi_UV_Dc = usQi_UV_Dc < 0 ? 0 : (usQi_UV_Dc > CODECHAL_VP8_QP_MAX ? CODECHAL_VP8_QP_MAX : usQi_UV_Dc);
            cmd.DW20.ChromaDCThreshold0Segment3 = (((1) << 16) - 1) / ((1 << 16) / VP8_QUANT_DC_G9[usQi_UV_Dc]) - ((48 * VP8_QUANT_DC_G9[usQi_UV_Dc]) >> 7);
            cmd.DW20.ChromaDCThreshold1Segment3 = (((2) << 16) - 1) / ((1 << 16) / VP8_QUANT_DC_G9[usQi_UV_Dc]) - ((48 * VP8_QUANT_DC_G9[usQi_UV_Dc]) >> 7);
            cmd.DW21.ChromaDCThreshold2Segment3 = (((3) << 16) - 1) / ((1 << 16) / VP8_QUANT_DC_G9[usQi_UV_Dc]) - ((48 * VP8_QUANT_DC_G9[usQi_UV_Dc]) >> 7);
            cmd.DW21.ChromaDCThreshold3Segment3 = (((4) << 16) - 1) / ((1 << 16) / VP8_QUANT_DC_G9[usQi_UV_Dc]) - ((48 * VP8_QUANT_DC_G9[usQi_UV_Dc]) >> 7);
        }

        usQi_UV_Ac = vp8QuantData->QIndex[0] + vp8QuantData->QIndexDelta[VP8_QINDEX_UV_AC];
        usQi_UV_Ac = usQi_UV_Ac < 0 ? 0 : (usQi_UV_Ac > CODECHAL_VP8_QP_MAX ? CODECHAL_VP8_QP_MAX : usQi_UV_Ac);
        cmd.DW22.ChromaAC1ThresholdSegment0 = ((1 << (16)) - 1) / ((1 << 16) / VP8_QUANT_AC_G9[usQi_UV_Ac]) - ((48 * VP8_QUANT_AC_G9[usQi_UV_Ac]) >> 7);
        if (picParams->segmentation_enabled)
        {
            usQi_UV_Ac = vp8QuantData->QIndex[1] + vp8QuantData->QIndexDelta[VP8_QINDEX_UV_AC];
            usQi_UV_Ac = usQi_UV_Ac < 0 ? 0 : (usQi_UV_Ac > CODECHAL_VP8_QP_MAX ? CODECHAL_VP8_QP_MAX : usQi_UV_Ac);
            cmd.DW22.ChromaAC1ThresholdSegment1 = ((1 << (16)) - 1) / ((1 << 16) / VP8_QUANT_AC_G9[usQi_UV_Ac]) - ((48 * VP8_QUANT_AC_G9[usQi_UV_Ac]) >> 7);

            usQi_UV_Ac = vp8QuantData->QIndex[2] + vp8QuantData->QIndexDelta[VP8_QINDEX_UV_AC];
            usQi_UV_Ac = usQi_UV_Ac < 0 ? 0 : (usQi_UV_Ac > CODECHAL_VP8_QP_MAX ? CODECHAL_VP8_QP_MAX : usQi_UV_Ac);
            cmd.DW23.ChromaAC1ThresholdSegment2 = ((1 << (16)) - 1) / ((1 << 16) / VP8_QUANT_AC_G9[usQi_UV_Ac]) - ((48 * VP8_QUANT_AC_G9[usQi_UV_Ac]) >> 7);
            usQi_UV_Ac = vp8QuantData->QIndex[3] + vp8QuantData->QIndexDelta[VP8_QINDEX_UV_AC];
            usQi_UV_Ac = usQi_UV_Ac < 0 ? 0 : (usQi_UV_Ac > CODECHAL_VP8_QP_MAX ? CODECHAL_VP8_QP_MAX : usQi_UV_Ac);
            cmd.DW23.ChromaAC1ThresholdSegment3 = ((1 << (16)) - 1) / ((1 << 16) / VP8_QUANT_AC_G9[usQi_UV_Ac]) - ((48 * VP8_QUANT_AC_G9[usQi_UV_Ac]) >> 7);
        }

        usQi_Y_Dc = vp8QuantData->QIndex[0] + vp8QuantData->QIndexDelta[VP8_QINDEX_Y1_DC];
        usQi_Y_Dc = usQi_Y_Dc < 0 ? 0 : (usQi_Y_Dc > CODECHAL_VP8_QP_MAX ? CODECHAL_VP8_QP_MAX : usQi_Y_Dc);
        cmd.DW24.VME16x16CostSegment0 = VP8_IFRAME_VME_COSTS_G9[usQi_Y_Dc & 0x7F][0];
        cmd.DW25.VME4x4CostSegment0 = VP8_IFRAME_VME_COSTS_G9[usQi_Y_Dc & 0x7F][1];
        cmd.DW26.VME16x16NonDCPenaltySegment0 = VP8_IFRAME_VME_COSTS_G9[usQi_Y_Dc & 0x7F][2];
        cmd.DW27.VME4x4NonDCPenaltySegment0 = VP8_IFRAME_VME_COSTS_G9[usQi_Y_Dc & 0x7F][3];
        if (picParams->segmentation_enabled)
        {
            usQi_Y_Dc = vp8QuantData->QIndex[1] + vp8QuantData->QIndexDelta[VP8_QINDEX_Y1_DC];
            usQi_Y_Dc = usQi_Y_Dc < 0 ? 0 : (usQi_Y_Dc > CODECHAL_VP8_QP_MAX ? CODECHAL_VP8_QP_MAX : usQi_Y_Dc);
            cmd.DW24.VME16x16CostSegment1 = VP8_IFRAME_VME_COSTS_G9[usQi_Y_Dc & 0x7F][0];
            cmd.DW25.VME4x4CostSegment1 = VP8_IFRAME_VME_COSTS_G9[usQi_Y_Dc & 0x7F][1];
            cmd.DW26.VME16x16NonDCPenaltySegment1 = VP8_IFRAME_VME_COSTS_G9[usQi_Y_Dc & 0x7F][2];
            cmd.DW27.VME4x4NonDCPenaltySegment1 = VP8_IFRAME_VME_COSTS_G9[usQi_Y_Dc & 0x7F][3];

            usQi_Y_Dc = vp8QuantData->QIndex[2] + vp8QuantData->QIndexDelta[VP8_QINDEX_Y1_DC];
            usQi_Y_Dc = usQi_Y_Dc < 0 ? 0 : (usQi_Y_Dc > CODECHAL_VP8_QP_MAX ? CODECHAL_VP8_QP_MAX : usQi_Y_Dc);
            cmd.DW24.VME16x16CostSegment2 = VP8_IFRAME_VME_COSTS_G9[usQi_Y_Dc & 0x7F][0];
            cmd.DW25.VME4x4CostSegment2 = VP8_IFRAME_VME_COSTS_G9[usQi_Y_Dc & 0x7F][1];
            cmd.DW26.VME16x16NonDCPenaltySegment2 = VP8_IFRAME_VME_COSTS_G9[usQi_Y_Dc & 0x7F][2];
            cmd.DW27.VME4x4NonDCPenaltySegment2 = VP8_IFRAME_VME_COSTS_G9[usQi_Y_Dc & 0x7F][3];

            usQi_Y_Dc = vp8QuantData->QIndex[3] + vp8QuantData->QIndexDelta[VP8_QINDEX_Y1_DC];
            usQi_Y_Dc = usQi_Y_Dc < 0 ? 0 : (usQi_Y_Dc > CODECHAL_VP8_QP_MAX ? CODECHAL_VP8_QP_MAX : usQi_Y_Dc);
            cmd.DW24.VME16x16CostSegment3 = VP8_IFRAME_VME_COSTS_G9[usQi_Y_Dc & 0x7F][0];
            cmd.DW25.VME4x4CostSegment3 = VP8_IFRAME_VME_COSTS_G9[usQi_Y_Dc & 0x7F][1];
            cmd.DW26.VME16x16NonDCPenaltySegment3 = VP8_IFRAME_VME_COSTS_G9[usQi_Y_Dc & 0x7F][2];
            cmd.DW27.VME4x4NonDCPenaltySegment3 = VP8_IFRAME_VME_COSTS_G9[usQi_Y_Dc & 0x7F][3];
        }

        cmd.DW32.MBEncPerMBOutDataSurfBTI = CODECHAL_VP8_MBENC_PER_MB_OUT_G9;

        /* For CM kernels the Surface index of UV surface gets incremented by 1, so the Luma surface index itself is programmed in the Curbe data */
        cmd.DW33.MBEncCurrYBTI = CODECHAL_VP8_MBENC_CURR_Y_G9;
        cmd.DW34.MBEncCurrUVBTI = CODECHAL_VP8_MBENC_CURR_Y_G9;//CODECHAL_VP8_MBENC_CURR_UV_G9;
        cmd.DW35.MBModeCostLumaBTI = CODECHAL_VP8_MBENC_MB_MODE_COST_LUMA_G9;
        cmd.DW36.MBEncBlockModeCostBTI = CODECHAL_VP8_MBENC_BLOCK_MODE_COST_G9;
        cmd.DW37.ChromaReconSurfBTI = CODECHAL_VP8_MBENC_CHROMA_RECON_G9;
        cmd.DW38.SegmentationMapBTI = CODECHAL_VP8_MBENC_SEGMENTATION_MAP_G9;
        cmd.DW39.HistogramBTI = CODECHAL_VP8_MBENC_HISTOGRAM_G9;
        cmd.DW40.MBEncVMEDebugStreamOutBTI = CODECHAL_VP8_MBENC_I_VME_DEBUG_STREAMOUT_G9;
        cmd.DW41.VmeBTI = CODECHAL_VP8_MBENC_VME_G9;
        cmd.DW42.IDistortionSurfaceBTI = CODECHAL_VP8_MBENC_IDIST_G9;
        cmd.DW43.MBEncCurrYDownScaledBTI = CODECHAL_VP8_MBENC_CURR_Y_DOWNSCALED_G9;
        cmd.DW44.MBEncVMECoarseIntraBTI = CODECHAL_VP8_MBENC_VME_Coarse_Intra_G9;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(params->pKernelState->m_dshRegion.AddData(
            &cmd,
            params->pKernelState->dwCurbeOffset,
            sizeof(cmd)));
    }
    else
    {
        //P frame CURBE
        struct MediaObjectVp8MbencPStaticDataG9 cmd;
        int16_t usQi_Y_Dc_Seg0, usQi_Y_Dc_Seg1, usQi_Y_Dc_Seg2, usQi_Y_Dc_Seg3;
        int16_t usQp_Seg0, usQp_Seg1, usQp_Seg2, usQp_Seg3;

        MOS_ZeroMemory(&cmd, sizeof(cmd));

        usQi_Y_Dc_Seg0 = vp8QuantData->QIndex[0] + vp8QuantData->QIndexDelta[VP8_QINDEX_Y1_DC];/* TBC */
        usQi_Y_Dc_Seg1 = vp8QuantData->QIndex[1] + vp8QuantData->QIndexDelta[VP8_QINDEX_Y1_DC];
        usQi_Y_Dc_Seg2 = vp8QuantData->QIndex[2] + vp8QuantData->QIndexDelta[VP8_QINDEX_Y1_DC];
        usQi_Y_Dc_Seg3 = vp8QuantData->QIndex[3] + vp8QuantData->QIndexDelta[VP8_QINDEX_Y1_DC];

        usQp_Seg0 = usQi_Y_Dc_Seg0 < 0 ? 0 : (usQi_Y_Dc_Seg0 > CODECHAL_VP8_QP_MAX ? CODECHAL_VP8_QP_MAX : usQi_Y_Dc_Seg0);
        usQp_Seg1 = usQi_Y_Dc_Seg1 < 0 ? 0 : (usQi_Y_Dc_Seg1 > CODECHAL_VP8_QP_MAX ? CODECHAL_VP8_QP_MAX : usQi_Y_Dc_Seg1);
        usQp_Seg2 = usQi_Y_Dc_Seg2 < 0 ? 0 : (usQi_Y_Dc_Seg2 > CODECHAL_VP8_QP_MAX ? CODECHAL_VP8_QP_MAX : usQi_Y_Dc_Seg2);
        usQp_Seg3 = usQi_Y_Dc_Seg3 < 0 ? 0 : (usQi_Y_Dc_Seg3 > CODECHAL_VP8_QP_MAX ? CODECHAL_VP8_QP_MAX : usQi_Y_Dc_Seg3);

        uint8_t MEMethod = (params->ucKernelMode == encodeNormalMode) ? 6 : 4;

        //DW0
        cmd.DW0.FrameWidth = (seqParams->FrameWidth + 15) & (~0xF);  /* Kernel require MB boundary aligned dimensions */
        cmd.DW0.FrameHeight = (seqParams->FrameHeight + 15) & (~0xF);
        // DW1
        cmd.DW1.FrameType                     = 1; // P-frame
        cmd.DW1.MultiplePred                  = (params->ucKernelMode == encodeNormalMode) ? 1 : ((params->ucKernelMode == encodePerformanceMode) ? 0 : 2);
        cmd.DW1.HMEEnable                     = params->bHmeEnabled;
        cmd.DW1.HMECombineOverlap             = 1;
        cmd.DW1.EnableTemporalScalability     = 0;
        cmd.DW1.RefFrameFlags                 = picParams->ref_frame_ctrl;
        cmd.DW1.EnableSegmentation            = picParams->segmentation_enabled;
        cmd.DW1.EnableSegmentationInfoUpdate  = 1;//dont care field hardcoding to match kernel- picParams->update_mb_segmentation_map;
        cmd.DW1.MultiReferenceQPCheck         = false;
        cmd.DW1.ModecostEnableFlag            = 1;

        // temporal scaliability is enable
        cmd.DW1.MainRef = 0;

        if (seqParams->NumTemporalLayersMinus1 > 0)
        {
            uint32_t m_ref_frame_flags = picParams->ref_frame_ctrl;
            uint8_t m_rfo[3];

            m_rfo[2] = picParams->first_ref;
            m_rfo[1] = picParams->second_ref;
            m_rfo[0] = 6 - m_rfo[2] - m_rfo[1];

            uint32_t MainRef = 0;
            uint32_t k = 0;
            uint32_t i;

            for (i = 0; i <3; i++)
            {
                if (m_ref_frame_flags & (0x1 << (m_rfo[i] - 1))) {
                    MainRef |= (m_rfo[i] << (2 * k));
                    k++;
                }
            }

            cmd.DW1.MainRef = MainRef;
        }
        else {
            // original one for MainRef and RefFlag
            cmd.DW1.MainRef = VP8_MAINREF_TABLE_G9[picParams->ref_frame_ctrl];
        }

        //DW2
        cmd.DW2.LambdaIntraSegment0 = VP8_QUANT_DC_G9[usQp_Seg0];
        cmd.DW2.LambdaInterSegment0 = (VP8_QUANT_DC_G9[usQp_Seg0] >> 2);
        //DW3
        cmd.DW3.LambdaIntraSegment1 = (VP8_QUANT_DC_G9[usQp_Seg1]);
        cmd.DW3.LambdaInterSegment1 = (VP8_QUANT_DC_G9[usQp_Seg1] >> 2);
        //DW4
        cmd.DW4.LambdaIntraSegment2 = (VP8_QUANT_DC_G9[usQp_Seg2]);
        cmd.DW4.LambdaInterSegment2 = (VP8_QUANT_DC_G9[usQp_Seg2] >> 2);
        //DW5
        cmd.DW5.LambdaIntraSegment3 = (VP8_QUANT_DC_G9[usQp_Seg3]);
        cmd.DW5.LambdaInterSegment3 = (VP8_QUANT_DC_G9[usQp_Seg3] >> 2);
        //DW6
        cmd.DW6.ReferenceFrameSignBias_3 = picParams->sign_bias_golden;
        cmd.DW6.ReferenceFrameSignBias_2 = picParams->sign_bias_alternate;
        cmd.DW6.ReferenceFrameSignBias_1 = picParams->sign_bias_golden ^ picParams->sign_bias_alternate;
        cmd.DW6.ReferenceFrameSignBias_0 = 0;
        //DW7
        cmd.DW7.RawDistThreshold = (params->ucKernelMode == encodeNormalMode) ? 50 : ((params->ucKernelMode == encodePerformanceMode) ? 0 : 100);
        cmd.DW7.TemporalLayerID = picParams->temporal_id;
        //DW8
        cmd.DW8.EarlyIMESuccessfulStopThreshold = 0;
        cmd.DW8.AdaptiveSearchEnable = (params->ucKernelMode != encodePerformanceMode) ? 1 : 0;
        cmd.DW8.SkipModeEnable = 1;
        cmd.DW8.BidirectionalMixDisbale = 0;
        cmd.DW8.Transform8x8FlagForInterEnable = 0;
        cmd.DW8.EarlyIMESuccessEnable = 0;
        //DW9
        cmd.DW9.RefPixelBiasEnable = 0;
        cmd.DW9.UnidirectionMixEnable = 0;
        cmd.DW9.BidirectionalWeight = 0;
        cmd.DW9.RefIDPolarityBits = 0;
        cmd.DW9.MaximumNumberOfMotionVectors = 0 /* 32 */; // from BDW
        //DW10
        cmd.DW10.MaxFixedSearchPathLength = (params->ucKernelMode == encodeNormalMode) ? 25 : ((params->ucKernelMode == encodePerformanceMode) ? 9 : 57);
        cmd.DW10.MaximumSearchPathLength = 57;
        //DW11
        cmd.DW11.SubMacroBlockSubPartitionMask = 0 /* 0x30 */; //from BDW
        cmd.DW11.IntraSADMeasureAdjustment = 2;
        cmd.DW11.InterSADMeasureAdjustment = 2;
        cmd.DW11.BlockBasedSkipEnable = 0;
        cmd.DW11.BMEdisableforFBRMessage = 0 /* 1 */; // from BDW
        cmd.DW11.ForwardTransformSkipCheckEnable = 0;
        cmd.DW11.ProcessInterChromaPixelsMode = 0;
        cmd.DW11.DisableFieldCacheAllocation = 0;
        cmd.DW11.SkipModeType = 0;
        cmd.DW11.SubPelMode = 3;
        cmd.DW11.DualSearchPathOption = 0;
        cmd.DW11.SearchControl = 0;
        cmd.DW11.ReferenceAccess = 0;
        cmd.DW11.SourceAccess = 0;
        cmd.DW11.InterMbTypeRoadMap = 0;
        cmd.DW11.SourceBlockSize = 0;
        //DW12
        cmd.DW12.ReferenceSearchWindowsHeight = (params->ucKernelMode != encodePerformanceMode) ? 40 : 28;
        cmd.DW12.ReferenceSearchWindowsWidth = (params->ucKernelMode != encodePerformanceMode) ? 48 : 28;
        //DW13
        cmd.DW13.Value = VP8_COST_TABLE_G9[usQp_Seg0][0];
        cmd.DW14.Value = VP8_COST_TABLE_G9[usQp_Seg0][1];
        cmd.DW15.Value = VP8_COST_TABLE_G9[usQp_Seg0][2];
        // which table to load is selected by MEMethod parameter determined by the driver based on the usage model (normal/quality/performance)
        switch (MEMethod)
        {
        case 2:
            CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(&(cmd.DW16), sizeof(VP8_SINGLESU), VP8_SINGLESU, sizeof(VP8_SINGLESU)));
            break;

        case 3:
            CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(&(cmd.DW16), sizeof(VP8_RASTERSCAN_48x40), VP8_RASTERSCAN_48x40, sizeof(VP8_RASTERSCAN_48x40)));
            break;

        case 4:
        case 5:
            CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(&(cmd.DW16), sizeof(VP8_FULLSPIRAL_48x40), VP8_FULLSPIRAL_48x40, sizeof(VP8_FULLSPIRAL_48x40)));
            break;

        case 6:
        default:
            CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(&(cmd.DW16), sizeof(VP8_DIAMOND), VP8_DIAMOND, sizeof(VP8_DIAMOND)));
            break;
        }
        //DW30
        cmd.DW30.Value = VP8_COST_TABLE_G9[usQp_Seg0][3];
        //DW31
        cmd.DW31.Value = VP8_COST_TABLE_G9[usQp_Seg0][4];
        //DW32
        cmd.DW32.BilinearEnable = 0;
        cmd.DW32.Intra16x16NoDCPenaltySegment0 = VP8_COST_TABLE_G9[usQp_Seg0][5];
        cmd.DW32.Intra16x16NoDCPenaltySegment1 = VP8_COST_TABLE_G9[usQp_Seg1][5];
        //DW33
        cmd.DW33.Intra16x16NoDCPenaltySegment2 = VP8_COST_TABLE_G9[usQp_Seg2][5];
        cmd.DW33.Intra16x16NoDCPenaltySegment3 = VP8_COST_TABLE_G9[usQp_Seg3][5];
        cmd.DW33.HMECombineLen = 8;//based on target usage part of par file param
        //DW34 to DW57
        CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(&(cmd.DW34), 24 * sizeof(uint32_t), VP8_MvRefCostContext_G9, 24 * sizeof(uint32_t)));
        //DW58
        cmd.DW58.EncCost16x16 = 0;
        cmd.DW58.EncCost16x8 = 0x73C;
        //DW59
        cmd.DW59.EncCost8x8 = 0x365;
        cmd.DW59.EncCost4x4 = 0xDC9;
        //DW60
        cmd.DW60.FrameCountProbabilityRefFrameCost_0 = 516;
        cmd.DW60.FrameCountProbabilityRefFrameCost_1 = 106;
        //DW61
        cmd.DW61.FrameCountProbabilityRefFrameCost_2 = 2407;
        cmd.DW61.FrameCountProbabilityRefFrameCost_3 = 2409;
        //DW62
        switch (m_pFramePositionInGop)
        {
            case 1:
                cmd.DW62.AverageQPOfLastRefFrame  = VP8_QUANT_DC_G9[m_averageKeyFrameQp];
                cmd.DW62.AverageQPOfGoldRefFrame  = cmd.DW62.AverageQPOfLastRefFrame;
                cmd.DW62.AverageQPOfAltRefFrame   = cmd.DW62.AverageQPOfLastRefFrame;
                break;
            case 2:
                cmd.DW62.AverageQPOfLastRefFrame  = VP8_QUANT_DC_G9[m_averagePFrameQp];
                cmd.DW62.AverageQPOfGoldRefFrame  = VP8_QUANT_DC_G9[m_averageKeyFrameQp];
                cmd.DW62.AverageQPOfAltRefFrame   = cmd.DW62.AverageQPOfGoldRefFrame;
                break;
            case 3:
                cmd.DW62.AverageQPOfLastRefFrame = VP8_QUANT_DC_G9[m_averagePFrameQp];
                cmd.DW62.AverageQPOfGoldRefFrame = VP8_QUANT_DC_G9[m_averagePFrameQp];
                cmd.DW62.AverageQPOfAltRefFrame  = VP8_QUANT_DC_G9[m_averageKeyFrameQp];
                break;
            default:
                cmd.DW62.AverageQPOfLastRefFrame  = VP8_QUANT_DC_G9[m_averagePFrameQp];
                cmd.DW62.AverageQPOfGoldRefFrame  = cmd.DW62.AverageQPOfLastRefFrame;
                cmd.DW62.AverageQPOfAltRefFrame   = cmd.DW62.AverageQPOfLastRefFrame;
                break;
        }

        //DW63
        cmd.DW63.Intra4x4NoDCPenaltySegment0 = VP8_COST_TABLE_G9[usQp_Seg0][6];
        cmd.DW63.Intra4x4NoDCPenaltySegment1 = VP8_COST_TABLE_G9[usQp_Seg1][6];
        cmd.DW63.Intra4x4NoDCPenaltySegment2 = VP8_COST_TABLE_G9[usQp_Seg2][6];
        cmd.DW63.Intra4x4NoDCPenaltySegment3 = VP8_COST_TABLE_G9[usQp_Seg3][6];
        //DW64
        cmd.DW64.Value = VP8_COST_TABLE_G9[usQp_Seg1][0];
        //DW65
        cmd.DW65.Value = VP8_COST_TABLE_G9[usQp_Seg1][1];
        //DW66
        cmd.DW66.Value = VP8_COST_TABLE_G9[usQp_Seg1][2];
        //DW67
        cmd.DW67.Value = VP8_COST_TABLE_G9[usQp_Seg1][3];
        //DW68
        cmd.DW68.Value = VP8_COST_TABLE_G9[usQp_Seg1][4];
        //DW69
        cmd.DW69.Value = VP8_COST_TABLE_G9[usQp_Seg2][0];
        //DW70
        cmd.DW70.Value = VP8_COST_TABLE_G9[usQp_Seg2][1];
        //DW71
        cmd.DW71.Value = VP8_COST_TABLE_G9[usQp_Seg2][2];
        //DW72
        cmd.DW72.Value = VP8_COST_TABLE_G9[usQp_Seg2][3];
        //DW73
        cmd.DW73.Value = VP8_COST_TABLE_G9[usQp_Seg2][4];
        //DW74
        cmd.DW74.Value = VP8_COST_TABLE_G9[usQp_Seg3][0];
        //DW75
        cmd.DW75.Value = VP8_COST_TABLE_G9[usQp_Seg3][1];
        //DW76
        cmd.DW76.Value = VP8_COST_TABLE_G9[usQp_Seg3][2];
        //DW77
        cmd.DW77.Value = VP8_COST_TABLE_G9[usQp_Seg3][3];
        //DW78
        cmd.DW78.Value = VP8_COST_TABLE_G9[usQp_Seg3][4];
        //DW79
        cmd.DW79.NewMVSkipThresholdSegment0 = VP8_NewMVSkipThreshold_G9[usQp_Seg0];
        cmd.DW79.NewMVSkipThresholdSegment1 = VP8_NewMVSkipThreshold_G9[usQp_Seg1];
        //DW80
        cmd.DW80.NewMVSkipThresholdSegment2 = VP8_NewMVSkipThreshold_G9[usQp_Seg2];
        cmd.DW80.NewMVSkipThresholdSegment3 = VP8_NewMVSkipThreshold_G9[usQp_Seg3];

        //setup binding table index entries
        cmd.DW81.PerMbOutputDataSurfaceBTI = CODECHAL_VP8_MBENC_PER_MB_OUT_G9;
        cmd.DW82.CurrentPictureYSurfaceBTI = CODECHAL_VP8_MBENC_CURR_Y_G9;
        cmd.DW83.CurrentPictureInterleavedUVSurfaceBTI = CODECHAL_VP8_MBENC_CURR_Y_G9;
        cmd.DW84.HMEMVDataSurfaceBTI = CODECHAL_VP8_MBENC_MV_DATA_FROM_ME_G9;
        cmd.DW85.MVDataSurfaceBTI = CODECHAL_VP8_MBENC_IND_MV_DATA_G9;
        cmd.DW86.MbCountPerReferenceFrameBTI = CODECHAL_VP8_MBENC_REF_MB_COUNT_G9;
        cmd.DW87.VMEInterPredictionBTI = CODECHAL_VP8_MBENC_INTER_PRED_G9;
        cmd.DW88.ActiveRef1BTI = CODECHAL_VP8_MBENC_REF1_PIC_G9;
        cmd.DW89.ActiveRef2BTI = CODECHAL_VP8_MBENC_REF2_PIC_G9;
        cmd.DW90.ActiveRef3BTI = CODECHAL_VP8_MBENC_REF3_PIC_G9;
        cmd.DW91.PerMbQuantDataBTI = CODECHAL_VP8_MBENC_P_PER_MB_QUANT_G9;
        cmd.DW92.SegmentMapBTI = CODECHAL_VP8_MBENC_SEGMENTATION_MAP_G9;
        cmd.DW93.InterPredictionDistortionBTI = CODECHAL_VP8_MBEBC_INTER_PRED_DISTORTION_G9;
        cmd.DW94.HistogramBTI = CODECHAL_VP8_MBENC_HISTOGRAM_G9;
        cmd.DW95.PredMVDataBTI = CODECHAL_VP8_MBEBC_PER_MV_DATA_G9;
        cmd.DW96.ModeCostUpdateBTI = CODECHAL_VP8_MBENC_MODE_COST_UPDATE_G9;
        cmd.DW97.KernelDebugDumpBTI = CODECHAL_VP8_MBENC_P_VME_DEBUG_STREAMOUT_G9;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(params->pKernelState->m_dshRegion.AddData(
            &cmd,
            params->pKernelState->dwCurbeOffset,
            sizeof(cmd)));
    }

    return status;
}

MOS_STATUS CodechalEncodeVp8G9::SetMeCurbe(struct CodechalVp8MeCurbeParams* params)
{
    struct MediaObjectVp8MeStaticDataG9    cmd;
    PCODEC_VP8_ENCODE_SEQUENCE_PARAMS       seqParams;
    PMHW_STATE_HEAP_INTERFACE               stateHeapInterface;
    CODECHAL_MEDIA_STATE_TYPE               encMediaStateType;
    uint8_t                                 meMode, meMethod, tableIdx;
    uint32_t                                scaleFactor;
    MOS_STATUS                              status = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface->GetRenderInterface());
    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pSeqParams);

    stateHeapInterface =
        m_hwInterface->GetRenderInterface()->m_stateHeapInterface;
    CODECHAL_ENCODE_CHK_NULL_RETURN(stateHeapInterface);

    seqParams = params->pSeqParams;

    MOS_ZeroMemory(&cmd, sizeof(cmd));

    meMode = params->b16xMeEnabled ? (params->b16xME ? CODECHAL_ENCODE_ME16X_BEFORE_ME4X : CODECHAL_ENCODE_ME4X_AFTER_ME16X) : CODECHAL_ENCODE_ME4X_ONLY;
    scaleFactor = (meMode == CODECHAL_ENCODE_ME16X_BEFORE_ME4X) ? 16 : 4;
    encMediaStateType = params->b16xME ? CODECHAL_MEDIA_STATE_16X_ME : CODECHAL_MEDIA_STATE_4X_ME;

    CODECHAL_ENCODE_ASSERT(seqParams->TargetUsage < NUM_TARGET_USAGE_MODES);

    cmd.DW1.MaxNumMVs = 0x10;
    cmd.DW1.BiWeight = 0;

    cmd.DW2.MaxNumSU = 57;
    // For Enc_P kernel MaxLenSP is supposed to have the following values
    // Normal mode = 25
    // Perf mode = 9
    // Quality mode = 57
    // For ME kernel, MaxLenSP is supposed to have the value 57 for all modes
    cmd.DW2.MaxLenSP = 57;

    cmd.DW3.SubMbPartMask = 0x77;
    cmd.DW3.InterSAD = 0;
    cmd.DW3.IntraSAD = 0;
    cmd.DW3.BMEDisableFBR = 1;
    cmd.DW3.SubPelMode = 3;

    cmd.DW4.PictureHeightMinus1 = CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(params->dwFrameFieldHeight / scaleFactor) - 1;
    cmd.DW4.PictureWidth = CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(params->dwFrameWidth / scaleFactor);

    cmd.DW4.PictureHeightMinus1   = cmd.DW4.PictureHeightMinus1 < 2 ? 2 : cmd.DW4.PictureHeightMinus1;
    cmd.DW4.PictureWidth          = cmd.DW4.PictureWidth < 3 ? 3 : cmd.DW4.PictureWidth;

    cmd.DW5.RefHeight = 40;
    cmd.DW5.RefWidth  = 48;

    cmd.DW6.MEModes           = meMode;
    cmd.DW6.SuperCombineDist = (params->ucKernelMode == encodeNormalMode) ? 5 : ((params->ucKernelMode == encodePerformanceMode) ? 0 : 1);
    cmd.DW6.MaxVmvR           = 0x7fc; /* For VP8, Luma motion vectors  in the range -2046 to +2046 (1/8 pel) */

    cmd.DW13.NumRefIdxL0MinusOne      = VP8_NUM_REFS_G9[params->pPicParams->ref_frame_ctrl] - 1;
    cmd.DW13.NumRefIdxL1MinusOne      = 0;

    tableIdx = 0; /* Only P pictures in VP8, no B pictures */
    meMethod = (params->ucKernelMode == encodeNormalMode) ? 6 : 4;
    status = MOS_SecureMemcpy((uint32_t*)(&cmd.SpDelta),
                            14 * sizeof(uint32_t),
                            m_encodeSearchPath[tableIdx][meMethod],
                            14 * sizeof(uint32_t));

    if (status != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
        return status;
    }
    cmd.DW32.VP8MeMVOutputDataBTI = CODECHAL_VP8_ME_MV_DATA_G9;
    cmd.DW33.VP8MeMVInputDataBTI  = CODECHAL_VP8_16xME_MV_DATA_G9;
    cmd.DW34.VP8MeDistortionBTI   = CODECHAL_VP8_ME_DISTORTION_G9;
    cmd.DW35.VP8MeMinDistBrcBTI   = CODECHAL_VP8_ME_MIN_DIST_BRC_DATA_G9;
    cmd.DW36.ForwardRefBTI        = CODECHAL_VP8_VME_INTER_PRED_G9;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_meKernelState.m_dshRegion.AddData(
        &cmd,
        m_meKernelState.dwCurbeOffset,
        sizeof(cmd)));

    return status;
}

MOS_STATUS CodechalEncodeVp8G9::SetMpuCurbe(struct CodechalVp8MpuCurbeParams* params)
{
    PCODEC_VP8_ENCODE_PIC_PARAMS                picParams;
    PCODEC_VP8_ENCODE_SEQUENCE_PARAMS           seqParams;
    PCODEC_VP8_ENCODE_QUANT_DATA                vp8QuantData;
    PMHW_STATE_HEAP_INTERFACE                   stateHeapInterface;
    struct MediaObjectVp8MpuFhbStaticDataG9    cmd;
    MOS_STATUS                                  status = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface->GetRenderInterface());
    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pPicParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pSeqParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pVp8QuantData);

    stateHeapInterface =
        m_hwInterface->GetRenderInterface()->m_stateHeapInterface;
    CODECHAL_ENCODE_CHK_NULL_RETURN(stateHeapInterface);

    picParams = params->pPicParams;
    seqParams = params->pSeqParams;
    vp8QuantData = params->pVp8QuantData;

    MOS_ZeroMemory(&cmd, sizeof(cmd));

    cmd.DW0.FrameWidth = (seqParams->FrameWidth + 15) & (~0xF);  /* Kernel require MB boundary aligned dimensions */
    cmd.DW0.FrameHeight = (seqParams->FrameHeight + 15) & (~0xF);

    cmd.DW1.FrameType = picParams->frame_type;
    cmd.DW1.Version = picParams->version;
    cmd.DW1.ShowFrame = picParams->show_frame;
    cmd.DW1.HorizontalScaleCode = seqParams->FrameWidthScale;
    cmd.DW1.VerticalScaleCode = seqParams->FrameHeightScale;
    cmd.DW1.ColorSpaceType = picParams->color_space;
    cmd.DW1.ClampType = picParams->clamping_type;
    cmd.DW1.PartitionNumL2 = picParams->CodedCoeffTokenPartition;
    cmd.DW1.EnableSegmentation = picParams->segmentation_enabled;
    cmd.DW1.SegMapUpdate =
        (picParams->segmentation_enabled) ? picParams->update_mb_segmentation_map : 0;
    cmd.DW1.SegmentationFeatureUpdate = picParams->update_segment_feature_data;    // setup whenever segmentation is 1
    cmd.DW1.SegmentationFeatureMode = 1;    // delta mode
    cmd.DW1.LoopFilterType = picParams->filter_type;
    cmd.DW1.SharpnessLevel = picParams->sharpness_level;
    cmd.DW1.LoopFilterAdjustmentOn = picParams->loop_filter_adj_enable;
    cmd.DW1.MBNoCoeffiscientSkip = picParams->mb_no_coeff_skip;
    cmd.DW1.ForcedLFUpdateForKeyFrame = picParams->forced_lf_adjustment;

    // DDI spec is not mapping to codechal directly. It should be mapping as below
    if (picParams->refresh_golden_frame == 1)
    {
        cmd.DW1.GoldenReferenceCopyFlag = 3;
    }
    else {
        cmd.DW1.GoldenReferenceCopyFlag = picParams->copy_buffer_to_golden;
    }
    if (picParams->refresh_alternate_frame == 1)
    {
        cmd.DW1.AlternateReferenceCopyFlag = 3;
    }
    else {
        cmd.DW1.AlternateReferenceCopyFlag = picParams->copy_buffer_to_alternate;
    }

    cmd.DW1.LastFrameUpdate = picParams->refresh_last;
    cmd.DW1.SignBiasGolden = picParams->sign_bias_golden;
    cmd.DW1.SignBiasAltRef = picParams->sign_bias_alternate;
    cmd.DW1.RefreshEntropyP = picParams->refresh_entropy_probs;

    cmd.DW2.LoopFilterLevel = picParams->version > 1 ? 0 : picParams->loop_filter_level[0];
    cmd.DW2.Qindex = vp8QuantData->QIndex[0];
    cmd.DW2.Y1DCQindex = vp8QuantData->QIndexDelta[VP8_QINDEX_Y1_DC];
    cmd.DW2.Y2DCQindex = vp8QuantData->QIndexDelta[VP8_QINDEX_Y2_DC];

    cmd.DW3.Y2ACQindex = vp8QuantData->QIndexDelta[VP8_QINDEX_Y2_AC];
    cmd.DW3.UVDCQindex = vp8QuantData->QIndexDelta[VP8_QINDEX_UV_DC];
    cmd.DW3.UVACQindex = vp8QuantData->QIndexDelta[VP8_QINDEX_UV_AC];
    cmd.DW3.FeatureData0Segment0 = vp8QuantData->QIndex[0];

    cmd.DW4.FeatureData0Segment1 = vp8QuantData->QIndex[1];
    cmd.DW4.FeatureData0Segment2 = vp8QuantData->QIndex[2];
    cmd.DW4.FeatureData0Segment3 = vp8QuantData->QIndex[3];
    cmd.DW4.FeatureData1Segment0 = picParams->loop_filter_level[0];

    cmd.DW5.FeatureData1Segment1 = picParams->loop_filter_level[1];
    cmd.DW5.FeatureData1Segment2 = picParams->loop_filter_level[2];
    cmd.DW5.FeatureData1Segment3 = picParams->loop_filter_level[3];
    cmd.DW5.RefLFDelta0 = picParams->ref_lf_delta[0];

    cmd.DW6.RefLFDelta1 = picParams->ref_lf_delta[1];
    cmd.DW6.RefLFDelta2 = picParams->ref_lf_delta[2];
    cmd.DW6.RefLFDelta3 = picParams->ref_lf_delta[3];
    cmd.DW6.ModeLFDelta0 = picParams->mode_lf_delta[0];

    cmd.DW7.ModeLFDelta1 = picParams->mode_lf_delta[1];
    cmd.DW7.ModeLFDelta2 = picParams->mode_lf_delta[2];
    cmd.DW7.ModeLFDelta3 = picParams->mode_lf_delta[3];
    cmd.DW7.MCFilterSelect = picParams->version > 0 ? 1 : 0;
    cmd.DW7.ChromaFullPixelMCFilterMode = picParams->version < 3 ? 0 : 1;
    cmd.DW7.MaxNumPakPasses = m_hwInterface->GetMfxInterface()->GetBrcNumPakPasses();     // Multi-Pass BRC
    cmd.DW7.ForcedTokenSurfaceRead = 1;

    cmd.DW7.ModecostEnableFlag      = 1;
    cmd.DW8.NumTLevels = seqParams->NumTemporalLayersMinus1 + 1;
    cmd.DW8.TemporalLayerID = picParams->temporal_id;

    cmd.DW12.HistogramBTI                     = CODECHAL_VP8_MPU_FHB_HISTOGRAM_G9;
    cmd.DW13.ReferenceModeProbabilityBTI      = CODECHAL_VP8_MPU_FHB_REF_MODE_PROBABILITY_G9;
    cmd.DW14.ModeProbabilityBTI               = CODECHAL_VP8_MPU_FHB_CURR_MODE_PROBABILITY_G9;
    cmd.DW15.ReferenceTokenProbabilityBTI     = CODECHAL_VP8_MPU_FHB_REF_TOKEN_PROBABILITY_G9;
    cmd.DW16.TokenProbabilityBTI              = CODECHAL_VP8_MPU_FHB_CURR_TOKEN_PROBABILITY_G9;
    cmd.DW17.FrameHeaderBitstreamBTI          = CODECHAL_VP8_MPU_FHB_HEADER_BITSTREAM_G9;
    cmd.DW18.HeaderMetaDataBTI                = CODECHAL_VP8_MPU_FHB_HEADER_METADATA_G9;
    cmd.DW19.PictureStateBTI                  = CODECHAL_VP8_MPU_FHB_PICTURE_STATE_G9;
    cmd.DW20.MPUBitStreamBTI                  = CODECHAL_VP8_MPU_FHB_MPU_BITSTREAM_G9;
    cmd.DW21.TokenBitsDataBTI                 = CODECHAL_VP8_MPU_FHB_TOKEN_BITS_DATA_TABLE_G9;
    cmd.DW22.KernelDebugDumpBTI               = CODECHAL_VP8_MPU_FHB_VME_DEBUG_STREAMOUT_G9;
    cmd.DW23.EntropyCostBTI                   = CODECHAL_VP8_MPU_FHB_ENTROPY_COST_TABLE_G9;
    cmd.DW24.ModeCostUpdateBTI                = CODECHAL_VP8_MPU_MODE_COST_UPDATE_G9;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mpuKernelState.m_dshRegion.AddData(
        &cmd,
        m_mpuKernelState.dwCurbeOffset,
        sizeof(cmd)));

    return status;
}

MOS_STATUS CodechalEncodeVp8G9::SendMpuSurfaces(
    PMOS_COMMAND_BUFFER cmdBuffer,
    struct CodechalVp8MpuSurfaceParams* params)
{
    uint32_t                        size;
    PMHW_KERNEL_STATE               kernelState;
    CODECHAL_SURFACE_CODEC_PARAMS   surfaceCodecParams;
    struct CodechalBindingTableVp8Mpu* vp8MpuBindingTable;
    MOS_STATUS                      status = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pKernelState);

    kernelState = params->pKernelState;
    vp8MpuBindingTable = &m_mpuBindingTable;

    // Histogram
    size = params->dwHistogramSize;
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.dwSize = size;
    surfaceCodecParams.presBuffer = params->presHistogram;
    surfaceCodecParams.dwBindingTableOffset = vp8MpuBindingTable->dwVp8MpuHistogram;
    surfaceCodecParams.bRenderTarget = true;
    surfaceCodecParams.bRawSurface = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Reference mode probability
    size = params->dwModeProbabilitySize;
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.dwSize = size;
    surfaceCodecParams.presBuffer = params->presRefModeProbability;
    surfaceCodecParams.dwBindingTableOffset = vp8MpuBindingTable->dwVp8MpuReferenceModeProbability;
    surfaceCodecParams.bRenderTarget = true;
    surfaceCodecParams.bRawSurface = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Mode probability
    size = params->dwModeProbabilitySize;
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.dwSize = size;
    //surfaceCodecParams.bMediaBlockRW              = true;
    surfaceCodecParams.presBuffer = params->presModeProbability;
    surfaceCodecParams.dwBindingTableOffset = vp8MpuBindingTable->dwVp8MpuModeProbability;
    surfaceCodecParams.bRenderTarget = true;
    surfaceCodecParams.bRawSurface = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Reference Token probability
    size = params->dwTokenProbabilitySize;
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.dwSize = size;
    //surfaceCodecParams.bMediaBlockRW              = true;
    surfaceCodecParams.presBuffer = params->presRefTokenProbability;
    surfaceCodecParams.dwBindingTableOffset = vp8MpuBindingTable->dwVp8MpuReferenceTokenProbability;
    surfaceCodecParams.bRenderTarget = true;
    surfaceCodecParams.bRawSurface = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Token probability
    size = params->dwTokenProbabilitySize;
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.dwSize = size;
    //surfaceCodecParams.bMediaBlockRW              = true;
    surfaceCodecParams.presBuffer = params->presTokenProbability;
    surfaceCodecParams.dwBindingTableOffset = vp8MpuBindingTable->dwVp8MpuTokenProbability;
    surfaceCodecParams.bRenderTarget = true;
    surfaceCodecParams.bRawSurface = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Frame header
    size = params->dwFrameHeaderSize;
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.dwSize = size;
    //surfaceCodecParams.bMediaBlockRW              = true;
    surfaceCodecParams.presBuffer = params->presFrameHeader;
    surfaceCodecParams.dwBindingTableOffset = vp8MpuBindingTable->dwVp8MpuFrameHeaderBitstream;
    surfaceCodecParams.bRenderTarget = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Header Metadata
    size = params->dwHeaderMetadataSize;
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.dwSize = size;
    surfaceCodecParams.dwOffset = params->dwHeaderMetaDataOffset;
    //surfaceCodecParams.bMediaBlockRW              = true;
    surfaceCodecParams.presBuffer = params->presHeaderMetadata;
    surfaceCodecParams.dwBindingTableOffset = vp8MpuBindingTable->dwVp8MpuHeaderMetaData;
    surfaceCodecParams.bRenderTarget = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Picture state
    size = mhw_vdbox_mfx_g9_kbl::MFX_VP8_PIC_STATE_CMD::byteSize;
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.dwSize = size;
    surfaceCodecParams.presBuffer = params->presPictureState;
    surfaceCodecParams.dwBindingTableOffset = vp8MpuBindingTable->dwVp8MpuPictureState;
    surfaceCodecParams.bRenderTarget = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Mpu Bitstream
    size = params->dwMpuBitstreamSize;
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.dwSize = size;
    //surfaceCodecParams.bMediaBlockRW              = true;
    surfaceCodecParams.presBuffer = params->presMpuBitstream;
    surfaceCodecParams.dwBindingTableOffset = vp8MpuBindingTable->dwVp8MpuMpuBitstream;
    surfaceCodecParams.bRenderTarget = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Token bits Data Surface
    size = params->dwTokenBitsDataSize;
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.dwSize = size;
    surfaceCodecParams.presBuffer = params->presTokenBitsData;
    surfaceCodecParams.dwBindingTableOffset = vp8MpuBindingTable->dwVp8MpuTokenBitsData;
    surfaceCodecParams.bRenderTarget = true;
    surfaceCodecParams.bRawSurface = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Entropy cost table
    size = params->dwEntropyCostTableSize;
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.dwSize = size;
    //surfaceCodecParams.bMediaBlockRW              = true;
    surfaceCodecParams.presBuffer = params->presEntropyCostTable;
    surfaceCodecParams.dwBindingTableOffset = vp8MpuBindingTable->dwVp8MpuEntropyCost;
    surfaceCodecParams.bRenderTarget = true;
    surfaceCodecParams.bRawSurface = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    //Mode Cost Update Surface
    size = sizeof(uint32_t)* 16;
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.dwSize = size;
    surfaceCodecParams.presBuffer = params->presModeCostUpdateBuffer;
    surfaceCodecParams.dwBindingTableOffset = vp8MpuBindingTable->dwVp8MpuModeCostUpdateSurface;
    surfaceCodecParams.bRenderTarget = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Kernel Debug Dump surface
    if (params->bVMEKernelDump)
    {
        size = params->dwKernelDumpSize;
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.presBuffer = params->presVmeKernelDumpBuffer;
        surfaceCodecParams.dwSize = size;
        surfaceCodecParams.dwBindingTableOffset = vp8MpuBindingTable->dwVp8MpuKernelDebugDump;
        surfaceCodecParams.bRenderTarget = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    return status;
}

MOS_STATUS CodechalEncodeVp8G9::SetTpuCurbe(struct CodechalVp8TpuCurbeParams* params)
{
    PCODEC_VP8_ENCODE_PIC_PARAMS                picParams;
    PCODEC_VP8_ENCODE_SEQUENCE_PARAMS           seqParams;
    PCODEC_VP8_ENCODE_QUANT_DATA                vp8QuantData;
    PMHW_STATE_HEAP_INTERFACE                   stateHeapInterface;
    struct MediaObjectVp8TpuFhbStaticDataG9    cmd;
    MOS_STATUS                                  status = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface->GetRenderInterface());
    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pPicParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pSeqParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pVp8QuantData);

    stateHeapInterface =
        m_hwInterface->GetRenderInterface()->m_stateHeapInterface;
    CODECHAL_ENCODE_CHK_NULL_RETURN(stateHeapInterface);

    picParams = params->pPicParams;
    seqParams = params->pSeqParams;
    vp8QuantData = params->pVp8QuantData;

    MOS_ZeroMemory(&cmd, sizeof(cmd));

    cmd.DW0.MBsInFrame              = (uint32_t)params->wPicWidthInMb * (uint32_t)params->wFieldFrameHeightInMb;

    cmd.DW1.FrameType               = picParams->frame_type;
    cmd.DW1.EnableSegmentation      = picParams->segmentation_enabled;
    cmd.DW1.RebinarizationFrameHdr  = (params->bRebinarizationFrameHdr ? 1 : 0);

    cmd.DW1.RefreshEntropyP         = picParams->refresh_entropy_probs;
    cmd.DW1.MBNoCoeffiscientSkip    = picParams->mb_no_coeff_skip;

    cmd.DW3.MaxQP                   = picParams->ClampQindexHigh;
    cmd.DW3.MinQP                   = picParams->ClampQindexLow;

    cmd.DW4.LoopFilterLevelSegment0 = picParams->loop_filter_level[0];
    cmd.DW4.LoopFilterLevelSegment1 = picParams->loop_filter_level[1];
    cmd.DW4.LoopFilterLevelSegment2 = picParams->loop_filter_level[2];
    cmd.DW4.LoopFilterLevelSegment3 = picParams->loop_filter_level[3];

    cmd.DW5.QuantizationIndexSegment0 = vp8QuantData->QIndex[0];
    cmd.DW5.QuantizationIndexSegment1 = vp8QuantData->QIndex[1];
    cmd.DW5.QuantizationIndexSegment2 = vp8QuantData->QIndex[2];
    cmd.DW5.QuantizationIndexSegment3 = vp8QuantData->QIndex[3];

    // This setup is only used for CQP = 1 << 8 (15:8 bits)
    // For BRC, this will be overwritten after Pak execution
    cmd.DW6.PakPassNum                  = m_hwInterface->GetMfxInterface()->GetBrcNumPakPasses() << 8;

    if (params->bAdaptiveRePak)
    {
        cmd.DW7.SkipCostDeltaThreshold      = 100;
        cmd.DW7.TokenCostDeltaThreshold     = 50;
    }
    else
    {
        cmd.DW7.SkipCostDeltaThreshold      = 0;    // change to 1 when temporal scalability is enabled
        cmd.DW7.TokenCostDeltaThreshold     = 0;    // change to 1 when temporal scalability is enabled
    }

    cmd.DW12.PakTokenStatisticsBTI                = CODECHAL_VP8_TPU_FHB_PAK_TOKEN_STATISTICS_G9;
    cmd.DW13.TokenUpdateFlagsBTI                  = CODECHAL_VP8_TPU_FHB_TOKEN_UPDATE_FLAGS_G9;
    cmd.DW14.EntropyCostTableBTI                  = CODECHAL_VP8_TPU_FHB_ENTROPY_COST_TABLE_G9;
    cmd.DW15.FrameHeaderBitstreamBTI              = CODECHAL_VP8_TPU_FHB_HEADER_BITSTREAM_G9;
    cmd.DW16.DefaultTokenProbabilityBTI           = CODECHAL_VP8_TPU_FHB_DEFAULT_TOKEN_PROBABILITY_G9;
    cmd.DW17.PictureStateBTI                      = CODECHAL_VP8_TPU_FHB_PICTURE_STATE_G9;
    cmd.DW18.MpuCurbeDataBTI                      = CODECHAL_VP8_TPU_FHB_MPU_CURBE_DATA_G9;
    cmd.DW19.HeaderMetaDataBTI                    = CODECHAL_VP8_TPU_FHB_HEADER_METADATA_G9;
    cmd.DW20.TokenProbabilityBTI                  = CODECHAL_VP8_TPU_FHB_TOKEN_PROBABILITY_G9;
    cmd.DW21.PakHardwareTokenProbabilityPass1BTI  = CODECHAL_VP8_TPU_FHB_PAK_HW_PASS1_PROBABILITY_G9;
    cmd.DW22.KeyFrameTokenProbabilityBTI          = CODECHAL_VP8_TPU_FHB_KEY_TOKEN_PROBABILITY_G9;
    cmd.DW23.UpdatedTokenProbabilityBTI           = CODECHAL_VP8_TPU_FHB_UPDATED_TOKEN_PROBABILITY_G9;
    cmd.DW24.PakHardwareTokenProbabilityPass2BTI  = CODECHAL_VP8_TPU_FHB_PAK_HW_PASS2_PROBABILITY_G9;
    cmd.DW25.KernelDebugDumpBTI                   = CODECHAL_VP8_TPU_FHB_VME_DEBUG_STREAMOUT_G9;
    cmd.DW26.RepakDecisionSurfaceBTI              = CODECHAL_VP8_TPU_FHB_REPAK_DECISION_G9;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_tpuKernelState.m_dshRegion.AddData(
        &cmd,
        m_tpuKernelState.dwCurbeOffset,
        sizeof(cmd)));

    return status;
}

MOS_STATUS CodechalEncodeVp8G9::CalMaxLevelRatioForTL(
    uint16_t *framesPer100Sec,
    uint32_t *targetBitRate,
    uint32_t numTemporalLayersMinus1,
    uint32_t *tempBitRate)
{
    MOS_STATUS              status = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    uint32_t ti, tj;
    uint32_t numLevel[NAX_NUM_TEMPORAL_LAYERS];
    uint32_t atempRatios[NAX_NUM_TEMPORAL_LAYERS];
    int  acculateTempBitRate;

    for( ti= 0;ti < numTemporalLayersMinus1+1;ti++)
    {
         atempRatios[ti]= (uint32_t)framesPer100Sec[numTemporalLayersMinus1]/(uint32_t)framesPer100Sec[ti]; // it should be integer
    }

    for( ti=0; ti < numTemporalLayersMinus1+1; ti++){
        numLevel[ti]= 0;
        for( tj=0; tj < atempRatios[0]; tj++)
        {
            if(tj%atempRatios[ti]==0)
                numLevel[ti] += 1; // ratio of framerate
        }
    }

    tempBitRate[0]= targetBitRate[0]*64/targetBitRate[numTemporalLayersMinus1];

    acculateTempBitRate= tempBitRate[0];
    for (ti=1; ti < (uint32_t)numTemporalLayersMinus1; ti++)
    {
        tempBitRate[ti]= (targetBitRate[ti] - targetBitRate[ti-1])*64/targetBitRate[numTemporalLayersMinus1];
        acculateTempBitRate += tempBitRate[ti];
    }

    tempBitRate[numTemporalLayersMinus1]= 64-acculateTempBitRate; // The sum of tempBitRate must be 64 because this will affect the QP directly

    for( ti=0; ti<numTemporalLayersMinus1+1; ti++)
    {
        int tem_level;
        if(ti==0)
            tem_level = numLevel[0];
        else
            tem_level = numLevel[ti] - numLevel[ti - 1];

        tempBitRate[ti] = atempRatios[0] * tempBitRate[ti]/tem_level;
    }

    return status;
}
