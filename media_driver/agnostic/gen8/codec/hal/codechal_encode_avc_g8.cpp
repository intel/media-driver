/*
* Copyright (c) 2011-2017, Intel Corporation
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
//! \file     codechal_encode_avc_g8.cpp
//! \brief    This file implements the C++ class/interface for Gen8 platform's AVC
//!           DualPipe encoding to be used across CODECHAL components.
//!

#include "codechal_encode_avc_g8.h"
#include "igcodeckrn_g8.h"

//enums begin
typedef enum _BINDING_TABLE_OFFSET_BRC_UPDATE
{
    BRC_UPDATE_HISTORY                  = 0,
    BRC_UPDATE_PAK_STATISTICS_OUTPUT    = 1,
    BRC_UPDATE_IMAGE_STATE_READ         = 2,
    BRC_UPDATE_IMAGE_STATE_WRITE        = 3,
    BRC_UPDATE_MBENC_CURBE_READ         = 4,
    BRC_UPDATE_MBENC_CURBE_WRITE        = 5,
    BRC_UPDATE_DISTORTION               = 6,
    BRC_UPDATE_CONSTANT_DATA            = 7,
    BRC_UPDATE_MB_QP                    = 8,
    BRC_UPDATE_NUM_SURFACES             = 9
} BINDING_TABLE_OFFSET_BRC_UPDATE;
//enums end

//structure begin
typedef struct _KERNEL_HEADER_CM {
    int nKernelCount;

    // Quality mode for Frame/Field
    CODECHAL_KERNEL_HEADER AVCMBEnc_Qlty_I;
    CODECHAL_KERNEL_HEADER AVCMBEnc_Qlty_P;
    CODECHAL_KERNEL_HEADER AVCMBEnc_Qlty_B;
    // Normal mode for Frame/Field
    CODECHAL_KERNEL_HEADER AVCMBEnc_Norm_I;
    CODECHAL_KERNEL_HEADER AVCMBEnc_Norm_P;
    CODECHAL_KERNEL_HEADER AVCMBEnc_Norm_B;
    // Performance modes for Frame/Field
    CODECHAL_KERNEL_HEADER AVCMBEnc_Perf_I;
    CODECHAL_KERNEL_HEADER AVCMBEnc_Perf_P;
    CODECHAL_KERNEL_HEADER AVCMBEnc_Perf_B;
    // Modes for Frame/Field
    CODECHAL_KERNEL_HEADER AVCMBEnc_Adv_I;
    CODECHAL_KERNEL_HEADER AVCMBEnc_Adv_P;
    CODECHAL_KERNEL_HEADER AVCMBEnc_Adv_B;
    // HME
    CODECHAL_KERNEL_HEADER AVC_ME_P;
    CODECHAL_KERNEL_HEADER AVC_ME_B;
    // DownScaling
    CODECHAL_KERNEL_HEADER PLY_DScale_PLY;
    CODECHAL_KERNEL_HEADER PLY_DScale_2f_PLY_2f;
    // BRC frame
    CODECHAL_KERNEL_HEADER InitFrameBRC;
    CODECHAL_KERNEL_HEADER FrameENCUpdate;
    // BRC Reset frame
    CODECHAL_KERNEL_HEADER BRC_ResetFrame;
    // BRC I Frame Distortion
    CODECHAL_KERNEL_HEADER BRC_IFrame_Dist;
    // BRCBlockCopy
    CODECHAL_KERNEL_HEADER BRCBlockCopy;
    // 2x DownScaling
    CODECHAL_KERNEL_HEADER PLY_2xDScale_PLY;
    CODECHAL_KERNEL_HEADER PLY_2xDScale_2f_PLY_2f;
    // Static frame detection Kernel
    CODECHAL_KERNEL_HEADER AVC_StaticFrameDetection;
} KERNEL_HEADER_CM, *PKERNEL_HEADER_CM;

typedef struct _BRC_INIT_RESET_CURBE
{
    union
    {
        struct
        {
            uint32_t   ProfileLevelMaxFrame                        : MOS_BITFIELD_RANGE(  0,31 );
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
            uint32_t   InitBufFullInBits                           : MOS_BITFIELD_RANGE(  0,31 );
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
            uint32_t   BufSizeInBits                               : MOS_BITFIELD_RANGE(  0,31 );
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
            uint32_t   AverageBitRate                              : MOS_BITFIELD_RANGE(  0,31 );
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
            uint32_t   MaxBitRate                                  : MOS_BITFIELD_RANGE(  0,31 );
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
            uint32_t   MinBitRate                                  : MOS_BITFIELD_RANGE(  0,31 );
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
            uint32_t   FrameRateM                                  : MOS_BITFIELD_RANGE(  0,31 );
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
            uint32_t   FrameRateD                                  : MOS_BITFIELD_RANGE(  0,31 );
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
            uint32_t   BRCFlag                                     : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   GopP                                        : MOS_BITFIELD_RANGE( 16,31 );
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
           uint32_t   GopB                                         : MOS_BITFIELD_RANGE(  0,15 );
           uint32_t   FrameWidthInBytes                            : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   FrameHeightInBytes                          : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   AVBRAccuracy                                : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   AVBRConvergence                             : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   MinQP                                       : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   MaxQP                                       : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   NoSlices                                    : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   InstantRateThreshold0ForP                   : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   InstantRateThreshold1ForP                   : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   InstantRateThreshold2ForP                   : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   InstantRateThreshold3ForP                   : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   InstantRateThreshold0ForB                   : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   InstantRateThreshold1ForB                   : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   InstantRateThreshold2ForB                   : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   InstantRateThreshold3ForB                   : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   InstantRateThreshold0ForI                   : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   InstantRateThreshold1ForI                   : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   InstantRateThreshold2ForI                   : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   InstantRateThreshold3ForI                   : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   DeviationThreshold0ForPandB                 : MOS_BITFIELD_RANGE(  0, 7 );     // Signed byte
            uint32_t   DeviationThreshold1ForPandB                 : MOS_BITFIELD_RANGE(  8,15 );     // Signed byte
            uint32_t   DeviationThreshold2ForPandB                 : MOS_BITFIELD_RANGE( 16,23 );     // Signed byte
            uint32_t   DeviationThreshold3ForPandB                 : MOS_BITFIELD_RANGE( 24,31 );     // Signed byte
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
            uint32_t   DeviationThreshold4ForPandB                 : MOS_BITFIELD_RANGE(  0, 7 );     // Signed byte
            uint32_t   DeviationThreshold5ForPandB                 : MOS_BITFIELD_RANGE(  8,15 );     // Signed byte
            uint32_t   DeviationThreshold6ForPandB                 : MOS_BITFIELD_RANGE( 16,23 );     // Signed byte
            uint32_t   DeviationThreshold7ForPandB                 : MOS_BITFIELD_RANGE( 24,31 );     // Signed byte
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
            uint32_t   DeviationThreshold0ForVBR                   : MOS_BITFIELD_RANGE(  0, 7 );     // Signed byte
            uint32_t   DeviationThreshold1ForVBR                   : MOS_BITFIELD_RANGE(  8,15 );     // Signed byte
            uint32_t   DeviationThreshold2ForVBR                   : MOS_BITFIELD_RANGE( 16,23 );     // Signed byte
            uint32_t   DeviationThreshold3ForVBR                   : MOS_BITFIELD_RANGE( 24,31 );     // Signed byte
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
            uint32_t   DeviationThreshold4ForVBR                   : MOS_BITFIELD_RANGE(  0, 7 );     // Signed byte
            uint32_t   DeviationThreshold5ForVBR                   : MOS_BITFIELD_RANGE(  8,15 );     // Signed byte
            uint32_t   DeviationThreshold6ForVBR                   : MOS_BITFIELD_RANGE( 16,23 );     // Signed byte
            uint32_t   DeviationThreshold7ForVBR                   : MOS_BITFIELD_RANGE( 24,31 );     // Signed byte
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
            uint32_t   DeviationThreshold0ForI                     : MOS_BITFIELD_RANGE(0, 7);        // Signed byte
            uint32_t   DeviationThreshold1ForI                     : MOS_BITFIELD_RANGE(8, 15);       // Signed byte
            uint32_t   DeviationThreshold2ForI                     : MOS_BITFIELD_RANGE(16, 23);      // Signed byte
            uint32_t   DeviationThreshold3ForI                     : MOS_BITFIELD_RANGE(24, 31);      // Signed byte
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
            uint32_t   DeviationThreshold4ForI                     : MOS_BITFIELD_RANGE(  0, 7 );      // Signed byte
            uint32_t   DeviationThreshold5ForI                     : MOS_BITFIELD_RANGE(  8,15 );      // Signed byte
            uint32_t   DeviationThreshold6ForI                     : MOS_BITFIELD_RANGE( 16,23 );      // Signed byte
            uint32_t   DeviationThreshold7ForI                     : MOS_BITFIELD_RANGE( 24,31 );      // Signed byte
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
            uint32_t   InitialQPForI                               : MOS_BITFIELD_RANGE(  0, 7 );     // Signed byte
            uint32_t   InitialQPForP                               : MOS_BITFIELD_RANGE(  8,15 );     // Signed byte
            uint32_t   InitialQPForB                               : MOS_BITFIELD_RANGE( 16,23 );     // Signed byte
            uint32_t   SlidingWindowSize                           : MOS_BITFIELD_RANGE( 24,31 );     // unsigned byte
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
            uint32_t   ACQP                                        : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW23;
} BRC_INIT_RESET_CURBE, *PBRC_INIT_RESET_CURBE;
C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(BRC_INIT_RESET_CURBE)) == 24);

typedef struct _BRC_UPDATE_CURBE_G8
{
    union
    {
        struct
        {
            uint32_t   TargetSize                                  : MOS_BITFIELD_RANGE(  0,31 );
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
            uint32_t   FrameNumber                                 : MOS_BITFIELD_RANGE(  0,31 );
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
            uint32_t   SizeofPicHeaders                            : MOS_BITFIELD_RANGE(  0,31 );
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
            uint32_t   startGAdjFrame0                             : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   startGAdjFrame1                             : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   startGAdjFrame2                             : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   startGAdjFrame3                             : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   TargetSizeFlag                              : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   BRCFlag                                     : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   MaxNumPAKs                                  : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   CurrFrameType                               : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   NumSkipFrames                               : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   MinimumQP                                   : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   MaximumQP                                   : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   EnableForceToSkip                           : MOS_BITFIELD_BIT(      24 );
            uint32_t   EnableSlidingWindow                         : MOS_BITFIELD_BIT(      25 );
            uint32_t   Reserved                                    : MOS_BITFIELD_RANGE( 26,31 );
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
            uint32_t    SizeSkipFrames                             : MOS_BITFIELD_RANGE(  0,31 );
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
            uint32_t   StartGlobalAdjustMult0                      : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   StartGlobalAdjustMult1                      : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   StartGlobalAdjustMult2                      : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   StartGlobalAdjustMult3                      : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   StartGlobalAdjustMult4                      : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   StartGlobalAdjustDiv0                       : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   StartGlobalAdjustDiv1                       : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   StartGlobalAdjustDiv2                       : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   StartGlobalAdjustDiv3                       : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   StartGlobalAdjustDiv4                       : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   QPThreshold0                                : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   QPThreshold1                                : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   QPThreshold2                                : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   QPThreshold3                                : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   gRateRatioThreshold0                        : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   gRateRatioThreshold1                        : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   gRateRatioThreshold2                        : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   gRateRatioThreshold3                        : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   gRateRatioThreshold4                        : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   gRateRatioThreshold5                        : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   gRateRatioThresholdQP0                      : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   gRateRatioThresholdQP1                      : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   gRateRatioThresholdQP2                      : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   gRateRatioThresholdQP3                      : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   gRateRatioThresholdQP4                      : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   gRateRatioThresholdQP5                      : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   gRateRatioThresholdQP6                      : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   QPIndexOfCurPic                             : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   QPIntraRefresh                              : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   IntraRefreshMode                            : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   Reserved1                                   : MOS_BITFIELD_RANGE(  16,23 );
            uint32_t   Reserved2                                   : MOS_BITFIELD_RANGE(  24,31 );
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
            uint32_t   IntraRefreshYPos                            : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   IntraRefreshXPos                            : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t    IntraRefreshHeight                         : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t    IntraRefreshWidth                          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t    IntraRefreshOffFrames                      : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t    Reserved                                   : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   UserMaxFrame                                : MOS_BITFIELD_RANGE(  0,31 );
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
            uint32_t   Reserved                                    : MOS_BITFIELD_RANGE(  0,31 );
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
            uint32_t   Reserved                                    : MOS_BITFIELD_RANGE(  0,31 );
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
            uint32_t   Reserved                                    : MOS_BITFIELD_RANGE(  0,31 );
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
            uint32_t   Reserved                                    : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW23;
} BRC_UPDATE_CURBE, *PBRC_UPDATE_CURBE;
C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(BRC_UPDATE_CURBE)) == 24);

typedef struct _AVC_ME_CURBE_CM
{
    // DW0
    union
    {
        struct
        {
            uint32_t   SkipModeEn                      : MOS_BITFIELD_BIT(       0 );
            uint32_t   AdaptiveEn                      : MOS_BITFIELD_BIT(       1 );
            uint32_t   BiMixDis                        : MOS_BITFIELD_BIT(       2 );
            uint32_t                                   : MOS_BITFIELD_RANGE(  3, 4 );
            uint32_t   EarlyImeSuccessEn               : MOS_BITFIELD_BIT(       5 );
            uint32_t                                   : MOS_BITFIELD_BIT(       6 );
            uint32_t   T8x8FlagForInterEn              : MOS_BITFIELD_BIT(       7 );
            uint32_t                                   : MOS_BITFIELD_RANGE(  8,23 );
            uint32_t   EarlyImeStop                    : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   MaxNumMVs                       : MOS_BITFIELD_RANGE(  0, 5 );
            uint32_t                                   : MOS_BITFIELD_RANGE(  6,15 );
            uint32_t   BiWeight                        : MOS_BITFIELD_RANGE( 16,21 );
            uint32_t                                   : MOS_BITFIELD_RANGE( 22,27 );
            uint32_t   UniMixDisable                   : MOS_BITFIELD_BIT(      28 );
            uint32_t                                   : MOS_BITFIELD_RANGE( 29,31 );
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
            uint32_t   MaxLenSP                        : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   MaxNumSU                        : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t                                   : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   SrcSize                         : MOS_BITFIELD_RANGE(  0, 1 );
            uint32_t                                   : MOS_BITFIELD_RANGE(  2, 3 );
            uint32_t   MbTypeRemap                     : MOS_BITFIELD_RANGE(  4, 5 );
            uint32_t   SrcAccess                       : MOS_BITFIELD_BIT(       6 );
            uint32_t   RefAccess                       : MOS_BITFIELD_BIT(       7 );
            uint32_t   SearchCtrl                      : MOS_BITFIELD_RANGE(  8,10 );
            uint32_t   DualSearchPathOption            : MOS_BITFIELD_BIT(      11 );
            uint32_t   SubPelMode                      : MOS_BITFIELD_RANGE( 12,13 );
            uint32_t   SkipType                        : MOS_BITFIELD_BIT(      14 );
            uint32_t   DisableFieldCacheAlloc          : MOS_BITFIELD_BIT(      15 );
            uint32_t   InterChromaMode                 : MOS_BITFIELD_BIT(      16 );
            uint32_t   FTEnable                        : MOS_BITFIELD_BIT(      17 );
            uint32_t   BMEDisableFBR                   : MOS_BITFIELD_BIT(      18 );
            uint32_t   BlockBasedSkipEnable            : MOS_BITFIELD_BIT(      19 );
            uint32_t   InterSAD                        : MOS_BITFIELD_RANGE( 20,21 );
            uint32_t   IntraSAD                        : MOS_BITFIELD_RANGE( 22,23 );
            uint32_t   SubMbPartMask                   : MOS_BITFIELD_RANGE( 24,30 );
            uint32_t                                   : MOS_BITFIELD_BIT(      31 );
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
            uint32_t                                   : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   PictureHeightMinus1             : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   PictureWidth                    : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t                                   : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t                                   : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   QpPrimeY                        : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   RefWidth                        : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   RefHeight                       : MOS_BITFIELD_RANGE( 24,31 );

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
            uint32_t                                   : MOS_BITFIELD_RANGE(  0, 2 );
            uint32_t   WriteDistortions                : MOS_BITFIELD_BIT(       3 );
            uint32_t   UseMvFromPrevStep               : MOS_BITFIELD_BIT(       4 );
            uint32_t                                   : MOS_BITFIELD_RANGE(  5, 7 );
            uint32_t   SuperCombineDist                : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   MaxVmvR                         : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t                                   : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   MVCostScaleFactor               : MOS_BITFIELD_RANGE( 16,17 );
            uint32_t   BilinearEnable                  : MOS_BITFIELD_BIT(      18 );
            uint32_t   SrcFieldPolarity                : MOS_BITFIELD_BIT(      19 );
            uint32_t   WeightedSADHAAR                 : MOS_BITFIELD_BIT(      20 );
            uint32_t   AConlyHAAR                      : MOS_BITFIELD_BIT(      21 );
            uint32_t   RefIDCostMode                   : MOS_BITFIELD_BIT(      22 );
            uint32_t                                   : MOS_BITFIELD_BIT(      23 );
            uint32_t   SkipCenterMask                  : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   Mode0Cost                       : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   Mode1Cost                       : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   Mode2Cost                       : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   Mode3Cost                       : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   Mode4Cost                       : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   Mode5Cost                       : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   Mode6Cost                       : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   Mode7Cost                       : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   Mode8Cost                       : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   Mode9Cost                       : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   RefIDCost                       : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   ChromaIntraModeCost             : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   MV0Cost                         : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   MV1Cost                         : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   MV2Cost                         : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   MV3Cost                         : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   MV4Cost                         : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   MV5Cost                         : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   MV6Cost                         : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   MV7Cost                         : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   NumRefIdxL0MinusOne             : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   NumRefIdxL1MinusOne             : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   ActualMBWidth                   : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   ActualMBHeight                  : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   List0RefID0FieldParity          : MOS_BITFIELD_BIT(       0 );
            uint32_t   List0RefID1FieldParity          : MOS_BITFIELD_BIT(       1 );
            uint32_t   List0RefID2FieldParity          : MOS_BITFIELD_BIT(       2 );
            uint32_t   List0RefID3FieldParity          : MOS_BITFIELD_BIT(       3 );
            uint32_t   List0RefID4FieldParity          : MOS_BITFIELD_BIT(       4 );
            uint32_t   List0RefID5FieldParity          : MOS_BITFIELD_BIT(       5 );
            uint32_t   List0RefID6FieldParity          : MOS_BITFIELD_BIT(       6 );
            uint32_t   List0RefID7FieldParity          : MOS_BITFIELD_BIT(       7 );
            uint32_t   List1RefID0FieldParity          : MOS_BITFIELD_BIT(       8 );
            uint32_t   List1RefID1FieldParity          : MOS_BITFIELD_BIT(       9 );
            uint32_t                                   : MOS_BITFIELD_RANGE( 10,31 );
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
            uint32_t   PrevMvReadPosFactor             : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   MvShiftFactor                   : MOS_BITFIELD_RANGE( 8, 15 );
            uint32_t   Reserved                        : MOS_BITFIELD_RANGE( 16,31 );
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
    } SPDelta;

    // DW30
    union
    {
        struct
        {
            uint32_t   Reserved                        : MOS_BITFIELD_RANGE(  0,31 );
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
            uint32_t   Reserved                        : MOS_BITFIELD_RANGE(  0,31 );
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
            uint32_t   _4xMeMvOutputDataSurfIndex      : MOS_BITFIELD_RANGE(  0,31 );
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
            uint32_t   _16xOr32xMeMvInputDataSurfIndex : MOS_BITFIELD_RANGE(  0,31 );
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
            uint32_t   _4xMeOutputDistSurfIndex        : MOS_BITFIELD_RANGE(  0,31 );
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
            uint32_t   _4xMeOutputBrcDistSurfIndex     : MOS_BITFIELD_RANGE(  0,31 );
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
            uint32_t   VMEFwdInterPredictionSurfIndex  : MOS_BITFIELD_RANGE(  0,31 );
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
            uint32_t   VMEBwdInterPredictionSurfIndex  : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW37;

    // DW38
    union
    {
        struct
        {
            uint32_t   Reserved                        : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW38;

} ME_CURBE_CM, *PME_CURBE_CM;
C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(ME_CURBE_CM)) == 39);

typedef struct _MBENC_CURBE_CM
{
    // DW0
    union
    {
        struct
        {
            uint32_t   SkipModeEn                      : MOS_BITFIELD_BIT(       0 );
            uint32_t   AdaptiveEn                      : MOS_BITFIELD_BIT(       1 );
            uint32_t   BiMixDis                        : MOS_BITFIELD_BIT(       2 );
            uint32_t                                   : MOS_BITFIELD_RANGE(  3, 4 );
            uint32_t   EarlyImeSuccessEn               : MOS_BITFIELD_BIT(       5 );
            uint32_t                                   : MOS_BITFIELD_BIT(       6 );
            uint32_t   T8x8FlagForInterEn              : MOS_BITFIELD_BIT(       7 );
            uint32_t                                   : MOS_BITFIELD_RANGE(  8,23 );
            uint32_t   EarlyImeStop                    : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   MaxNumMVs                       : MOS_BITFIELD_RANGE(  0, 5 );
            uint32_t                                   : MOS_BITFIELD_RANGE(  6,15 );
            uint32_t   BiWeight                        : MOS_BITFIELD_RANGE( 16,21 );
            uint32_t                                   : MOS_BITFIELD_RANGE( 22,27 );
            uint32_t   UniMixDisable                   : MOS_BITFIELD_BIT(      28 );
            uint32_t                                   : MOS_BITFIELD_RANGE( 29,31 );
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
            uint32_t   LenSP                           : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   MaxNumSU                        : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   PicWidth                        : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   SrcSize                         : MOS_BITFIELD_RANGE(  0, 1 );
            uint32_t                                   : MOS_BITFIELD_RANGE(  2, 3 );
            uint32_t   MbTypeRemap                     : MOS_BITFIELD_RANGE(  4, 5 );
            uint32_t   SrcAccess                       : MOS_BITFIELD_BIT(       6 );
            uint32_t   RefAccess                       : MOS_BITFIELD_BIT(       7 );
            uint32_t   SearchCtrl                      : MOS_BITFIELD_RANGE(  8,10 );
            uint32_t   DualSearchPathOption            : MOS_BITFIELD_BIT(      11 );
            uint32_t   SubPelMode                      : MOS_BITFIELD_RANGE( 12,13 );
            uint32_t   SkipType                        : MOS_BITFIELD_BIT(      14 );
            uint32_t   DisableFieldCacheAlloc          : MOS_BITFIELD_BIT(      15 );
            uint32_t   InterChromaMode                 : MOS_BITFIELD_BIT(      16 );
            uint32_t   FTEnable                        : MOS_BITFIELD_BIT(      17 );
            uint32_t   BMEDisableFBR                   : MOS_BITFIELD_BIT(      18 );
            uint32_t   BlockBasedSkipEnable            : MOS_BITFIELD_BIT(      19 );
            uint32_t   InterSAD                        : MOS_BITFIELD_RANGE( 20,21 );
            uint32_t   IntraSAD                        : MOS_BITFIELD_RANGE( 22,23 );
            uint32_t   SubMbPartMask                   : MOS_BITFIELD_RANGE( 24,30 );
            uint32_t                                   : MOS_BITFIELD_BIT(      31 );
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
            uint32_t   PicHeightMinus1                     : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   MvRestrictionInSliceEnable          : MOS_BITFIELD_BIT(      16 );
            uint32_t   DeltaMvEnable                       : MOS_BITFIELD_BIT(      17 );
            uint32_t   TrueDistortionEnable                : MOS_BITFIELD_BIT(      18 );
            uint32_t   EnableWavefrontOptimization         : MOS_BITFIELD_BIT(      19);
            uint32_t                                       : MOS_BITFIELD_BIT(      20);
            uint32_t   EnableIntraCostScalingForStaticFrame: MOS_BITFIELD_BIT(      21);
            uint32_t   EnableIntraRefresh                  : MOS_BITFIELD_BIT(      22);
            uint32_t   Reserved                            : MOS_BITFIELD_BIT(      23);
            uint32_t   EnableDirtyRect                     : MOS_BITFIELD_BIT(      24);
            uint32_t   bCurFldIDR                          : MOS_BITFIELD_BIT(      25 );
            uint32_t   ConstrainedIntraPredFlag            : MOS_BITFIELD_BIT(      26 );
            uint32_t   FieldParityFlag                     : MOS_BITFIELD_BIT(      27 );
            uint32_t   HMEEnable                           : MOS_BITFIELD_BIT(      28 );
            uint32_t   PictureType                         : MOS_BITFIELD_RANGE( 29,30 );
            uint32_t   UseActualRefQPValue                 : MOS_BITFIELD_BIT(      31 );
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
            uint32_t   SliceMbHeight                   : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   RefWidth                        : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   RefHeight                       : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   BatchBufferEnd                  : MOS_BITFIELD_RANGE(  0,31 );
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
            uint32_t   IntraPartMask                   : MOS_BITFIELD_RANGE(  0, 4 );
            uint32_t   NonSkipZMvAdded                 : MOS_BITFIELD_BIT(       5 );
            uint32_t   NonSkipModeAdded                : MOS_BITFIELD_BIT(       6 );
            uint32_t   LumaIntraSrcCornerSwap          : MOS_BITFIELD_BIT(       7 );
            uint32_t                                   : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   MVCostScaleFactor               : MOS_BITFIELD_RANGE( 16,17 );
            uint32_t   BilinearEnable                  : MOS_BITFIELD_BIT(      18 );
            uint32_t   SrcFieldPolarity                : MOS_BITFIELD_BIT(      19 );
            uint32_t   WeightedSADHAAR                 : MOS_BITFIELD_BIT(      20 );
            uint32_t   AConlyHAAR                      : MOS_BITFIELD_BIT(      21 );
            uint32_t   RefIDCostMode                   : MOS_BITFIELD_BIT(      22 );
            uint32_t                                   : MOS_BITFIELD_BIT(      23 );
            uint32_t   SkipCenterMask                  : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW7;

    struct
    {
        // DW8
        union
        {
            struct
            {
                uint32_t   Mode0Cost                       : MOS_BITFIELD_RANGE(  0, 7 );
                uint32_t   Mode1Cost                       : MOS_BITFIELD_RANGE(  8,15 );
                uint32_t   Mode2Cost                       : MOS_BITFIELD_RANGE( 16,23 );
                uint32_t   Mode3Cost                       : MOS_BITFIELD_RANGE( 24,31 );
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
                uint32_t   Mode4Cost                       : MOS_BITFIELD_RANGE(  0, 7 );
                uint32_t   Mode5Cost                       : MOS_BITFIELD_RANGE(  8,15 );
                uint32_t   Mode6Cost                       : MOS_BITFIELD_RANGE( 16,23 );
                uint32_t   Mode7Cost                       : MOS_BITFIELD_RANGE( 24,31 );
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
                uint32_t   Mode8Cost                       : MOS_BITFIELD_RANGE(  0, 7 );
                uint32_t   Mode9Cost                       : MOS_BITFIELD_RANGE(  8,15 );
                uint32_t   RefIDCost                       : MOS_BITFIELD_RANGE( 16,23 );
                uint32_t   ChromaIntraModeCost             : MOS_BITFIELD_RANGE( 24,31 );
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
                uint32_t   MV0Cost                         : MOS_BITFIELD_RANGE(  0, 7 );
                uint32_t   MV1Cost                         : MOS_BITFIELD_RANGE(  8,15 );
                uint32_t   MV2Cost                         : MOS_BITFIELD_RANGE( 16,23 );
                uint32_t   MV3Cost                         : MOS_BITFIELD_RANGE( 24,31 );
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
                uint32_t   MV4Cost                         : MOS_BITFIELD_RANGE(  0, 7 );
                uint32_t   MV5Cost                         : MOS_BITFIELD_RANGE(  8,15 );
                uint32_t   MV6Cost                         : MOS_BITFIELD_RANGE( 16,23 );
                uint32_t   MV7Cost                         : MOS_BITFIELD_RANGE( 24,31 );
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
                uint32_t   QpPrimeY                        : MOS_BITFIELD_RANGE(  0, 7 );
                uint32_t   QpPrimeCb                       : MOS_BITFIELD_RANGE(  8,15 );
                uint32_t   QpPrimeCr                       : MOS_BITFIELD_RANGE( 16,23 );
                uint32_t   TargetSizeInWord                : MOS_BITFIELD_RANGE( 24,31 );
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
                uint32_t   SICFwdTransCoeffThreshold_0     : MOS_BITFIELD_RANGE(  0,15 );
                uint32_t   SICFwdTransCoeffThreshold_1     : MOS_BITFIELD_RANGE( 16,23 );
                uint32_t   SICFwdTransCoeffThreshold_2     : MOS_BITFIELD_RANGE( 24,31 );
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
                uint32_t   SICFwdTransCoeffThreshold_3     : MOS_BITFIELD_RANGE(  0, 7 );
                uint32_t   SICFwdTransCoeffThreshold_4     : MOS_BITFIELD_RANGE(  8,15 );
                uint32_t   SICFwdTransCoeffThreshold_5     : MOS_BITFIELD_RANGE( 16,23 );
                uint32_t   SICFwdTransCoeffThreshold_6     : MOS_BITFIELD_RANGE( 24,31 );    // Highest Freq
            };
            struct
            {
                uint32_t   Value;
            };
        } DW15;
    } ModeMvCost;

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

        // DW30
        union
        {
            struct
            {
                uint32_t   Intra4x4ModeMask                : MOS_BITFIELD_RANGE(  0, 8 );
                uint32_t                                   : MOS_BITFIELD_RANGE(  9,15 );
                uint32_t   Intra8x8ModeMask                : MOS_BITFIELD_RANGE( 16,24 );
                uint32_t                                   : MOS_BITFIELD_RANGE( 25,31 );
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
                uint32_t   Intra16x16ModeMask              : MOS_BITFIELD_RANGE(  0, 3 );
                uint32_t   IntraChromaModeMask             : MOS_BITFIELD_RANGE(  4, 7 );
                uint32_t   IntraComputeType                : MOS_BITFIELD_RANGE(  8, 9 );
                uint32_t                                   : MOS_BITFIELD_RANGE( 10,31 );
            };
            struct
            {
                uint32_t   Value;
            };
        } DW31;
    } SPDelta;

    // DW32
    union
    {
        struct
        {
            uint32_t   SkipVal                         : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   MultiPredL0Disable              : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   MultiPredL1Disable              : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   Intra16x16NonDCPredPenalty      : MOS_BITFIELD_RANGE(  0,7 );
            uint32_t   Intra8x8NonDCPredPenalty        : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   Intra4x4NonDCPredPenalty        : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t                                   : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   List0RefID0FieldParity          : MOS_BITFIELD_BIT(       0 );
            uint32_t   List0RefID1FieldParity          : MOS_BITFIELD_BIT(       1 );
            uint32_t   List0RefID2FieldParity          : MOS_BITFIELD_BIT(       2 );
            uint32_t   List0RefID3FieldParity          : MOS_BITFIELD_BIT(       3 );
            uint32_t   List0RefID4FieldParity          : MOS_BITFIELD_BIT(       4 );
            uint32_t   List0RefID5FieldParity          : MOS_BITFIELD_BIT(       5 );
            uint32_t   List0RefID6FieldParity          : MOS_BITFIELD_BIT(       6 );
            uint32_t   List0RefID7FieldParity          : MOS_BITFIELD_BIT(       7 );
            uint32_t   List1RefID0FrameFieldFlag       : MOS_BITFIELD_BIT(       8 );
            uint32_t   List1RefID1FrameFieldFlag       : MOS_BITFIELD_BIT(       9 );
            uint32_t   IntraRefreshEn                  : MOS_BITFIELD_RANGE( 10,11 );
            uint32_t   ArbitraryNumMbsPerSlice         : MOS_BITFIELD_BIT(      12 );
            uint32_t   ForceNonSkipMbEnable            : MOS_BITFIELD_BIT(      13 );
            uint32_t   DisableEncSkipCheck             : MOS_BITFIELD_BIT(      14 );
            uint32_t   EnableDirectBiasAdjustment      : MOS_BITFIELD_BIT(      15 );
            uint32_t   EnableGlobalMotionBiasAdjustment: MOS_BITFIELD_BIT(      16 );
            uint32_t   bForceToSkip                    : MOS_BITFIELD_BIT(      17 );
            uint32_t                                   : MOS_BITFIELD_RANGE( 18,23 );
            uint32_t   List1RefID0FieldParity          : MOS_BITFIELD_BIT(      24 );
            uint32_t   List1RefID1FieldParity          : MOS_BITFIELD_BIT(      25 );
            uint32_t   MADEnableFlag                   : MOS_BITFIELD_BIT(      26 );
            uint32_t   ROIEnableFlag                   : MOS_BITFIELD_BIT(      27 );
            uint32_t   EnableMBFlatnessChkOptimization : MOS_BITFIELD_BIT(      28 );
            uint32_t   bDirectMode                     : MOS_BITFIELD_BIT(      29 );
            uint32_t   MBBrcEnable                     : MOS_BITFIELD_BIT(      30 );
            uint32_t   bOriginalBff                    : MOS_BITFIELD_BIT(      31 );
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
            uint32_t   PanicModeMBThreshold            : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   SmallMbSizeInWord               : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   LargeMbSizeInWord               : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   NumRefIdxL0MinusOne             : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   HMECombinedExtraSUs             : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   NumRefIdxL1MinusOne             : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t                                   : MOS_BITFIELD_RANGE( 24,27 );
            uint32_t   IsFwdFrameShortTermRef          : MOS_BITFIELD_BIT(      28 );
            uint32_t   CheckAllFractionalEnable        : MOS_BITFIELD_BIT(      29 );
            uint32_t   HMECombineOverlap               : MOS_BITFIELD_RANGE( 30,31 );
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
            uint32_t   SkipModeEn                      : MOS_BITFIELD_BIT(       0 );
            uint32_t   AdaptiveEn                      : MOS_BITFIELD_BIT(       1 );
            uint32_t   BiMixDis                        : MOS_BITFIELD_BIT(       2 );
            uint32_t                                   : MOS_BITFIELD_RANGE(  3, 4 );
            uint32_t   EarlyImeSuccessEn               : MOS_BITFIELD_BIT(       5 );
            uint32_t                                   : MOS_BITFIELD_BIT(       6 );
            uint32_t   T8x8FlagForInterEn              : MOS_BITFIELD_BIT(       7 );
            uint32_t                                   : MOS_BITFIELD_RANGE(  8,23 );
            uint32_t   EarlyImeStop                    : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   LenSP                           : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   MaxNumSU                        : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   RefThreshold                    : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t                                   : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   HMERefWindowsCombThreshold      : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   RefWidth                        : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   RefHeight                       : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   DistScaleFactorRefID0List0      : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   DistScaleFactorRefID1List0      : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   DistScaleFactorRefID2List0      : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   DistScaleFactorRefID3List0      : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   DistScaleFactorRefID4List0      : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   DistScaleFactorRefID5List0      : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   DistScaleFactorRefID6List0      : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   DistScaleFactorRefID7List0      : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ActualQPValueForRefID0List0     : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   ActualQPValueForRefID1List0     : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   ActualQPValueForRefID2List0     : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   ActualQPValueForRefID3List0     : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   ActualQPValueForRefID4List0     : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   ActualQPValueForRefID5List0     : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   ActualQPValueForRefID6List0     : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   ActualQPValueForRefID7List0     : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   ActualQPValueForRefID0List1     : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   ActualQPValueForRefID1List1     : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   RefCost                         : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   MbQpReadFactor                  : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   IntraCostSF                     : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   MaxVmvR                         : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW47;

    //DW48
    union
    {
        struct
        {
            uint32_t   IntraRefreshMBx                 : MOS_BITFIELD_RANGE( 0, 15 );
            uint32_t   IntraRefreshUnitInMBMinus1      : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   IntraRefreshQPDelta             : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW48;

    // DW49
    union
    {
        struct
        {
            uint32_t   ROI1_X_left                     : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI1_Y_top                      : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW49;

    // DW50
    union
    {
        struct
        {
            uint32_t   ROI1_X_right                    : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI1_Y_bottom                   : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW50;

    // DW51
    union
    {
        struct
        {
            uint32_t   ROI2_X_left                     : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI2_Y_top                      : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW51;

    // DW52
    union
    {
        struct
        {
            uint32_t   ROI2_X_right                    : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI2_Y_bottom                   : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW52;

    // DW53
    union
    {
        struct
        {
            uint32_t   ROI3_X_left                     : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI3_Y_top                      : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW53;

    // DW54
    union
    {
        struct
        {
            uint32_t   ROI3_X_right                    : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI3_Y_bottom                   : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW54;

    // DW55
    union
    {
        struct
        {
            uint32_t   ROI4_X_left                     : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI4_Y_top                      : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW55;

    // DW56
    union
    {
        struct
        {
            uint32_t   ROI4_X_right                    : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI4_Y_bottom                   : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW56;

    // DW57
    union
    {
        struct
        {
            uint32_t   ROI1_dQpPrimeY                   : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   ROI2_dQpPrimeY                   : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   ROI3_dQpPrimeY                   : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   ROI4_dQpPrimeY                   : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW57;

    // DW58
    union
    {
        struct
        {
            uint32_t   HMEMVCostScalingFactor          : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   Reserved                        : MOS_BITFIELD_RANGE( 8, 15 );
            uint32_t   IntraRefreshMBy                 : MOS_BITFIELD_RANGE( 16, 31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW58;

    // DW59
    union
    {
        struct
        {
            uint32_t   Reserved                        : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW59;

    // DW60
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
    } DW60;

    // DW61
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
    } DW61;

    // DW62
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
    } DW62;

    // DW63
    union
    {
        struct
        {
            uint32_t   Reserved                        : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW63;

    // DW64
    union
    {
        struct
        {
            uint32_t   L1ListRef0PictureCodingType     : MOS_BITFIELD_RANGE(  0, 1 ); // 0-invalid, 1-TFF, 2-invalid, 3-BFF
            uint32_t   Reserved                        : MOS_BITFIELD_RANGE(  2,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW64;

    // DW65
    union
    {
        struct
        {
            uint32_t   IPCM_QP0                        : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   IPCM_QP1                        : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   IPCM_QP2                        : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   IPCM_QP3                        : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW65;

    // DW66
    union
    {
        struct
        {
            uint32_t   IPCM_QP4                        : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   Reserved                        : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   IPCM_Thresh0                    : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW66;

    // DW67
    union
    {
        struct
        {
            uint32_t   IPCM_Thresh1                    : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   IPCM_Thresh2                    : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW67;

    // DW68
    union
    {
        struct
        {
            uint32_t   IPCM_Thresh3                    : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   IPCM_Thresh4                    : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW68;

    // DW69
    union
    {
        struct
        {
            uint32_t   BottomFieldOffsetL1ListRef0MV   : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW69;

    // DW70
    union
    {
        struct
        {
            uint32_t   BottomFieldOffsetL1ListRef0MBCode : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW70;

    // DW71
    union
    {
        struct
        {
            uint32_t   Reserved                        : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW71;

    // DW72
    union
    {
        struct
        {
            uint32_t   Reserved                        : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW72;

    // DW73
    union
    {
        struct
        {
            uint32_t   MBDataSurfIndex                 : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW73;

    // DW74
    union
    {
        struct
        {
            uint32_t   MVDataSurfIndex                 : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW74;

    // DW75
    union
    {
        struct
        {
            uint32_t   IDistSurfIndex                  : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW75;

    // DW76
    union
    {
        struct
        {
            uint32_t   SrcYSurfIndex                   : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW76;

    // DW77
    union
    {
        struct
        {
            uint32_t   MBSpecificDataSurfIndex         : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW77;

    // DW78
    union
    {
        struct
        {
            uint32_t   AuxVmeOutSurfIndex              : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW78;

    // DW79
    union
    {
        struct
        {
            uint32_t   CurrRefPicSelSurfIndex          : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW79;

    // DW80
    union
    {
        struct
        {
            uint32_t   HMEMVPredFwdBwdSurfIndex        : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW80;

    // DW81
    union
    {
        struct
        {
            uint32_t   HMEDistSurfIndex                : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW81;

    // DW82
    union
    {
        struct
        {
            uint32_t   SliceMapSurfIndex               : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW82;

    // DW83
    union
    {
        struct
        {
            uint32_t   FwdFrmMBDataSurfIndex           : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW83;

    // DW84
    union
    {
        struct
        {
            uint32_t   FwdFrmMVSurfIndex               : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW84;

    // DW85
    union
    {
        struct
        {
            uint32_t   MBQPBuffer                      : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW85;

    // DW86
    union
    {
        struct
        {
            uint32_t   MBBRCLut                        : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW86;

    // DW87
    union
    {
        struct
        {
            uint32_t   VMEInterPredictionSurfIndex     : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW87;

    // DW88
    union
    {
        struct
        {
            uint32_t   VMEInterPredictionMRSurfIndex   : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW88;

    // DW89
    union
    {
        struct
        {
            uint32_t   FlatnessChkSurfIndex            : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW89;

    // DW90
    union
    {
        struct
        {
            uint32_t   MADSurfIndex                    : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW90;

    // DW91
    union
    {
        struct
        {
            uint32_t   ForceNonSkipMBmapSurface        : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW91;

    // DW92
    union
    {
        struct
        {
            uint32_t   ReservedIndex                   : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW92;

    // DW93
    union
    {
        struct
        {
            uint32_t   BRCCurbeSurfIndex               : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW93;

    // DW94
    union
    {
        struct
        {
            uint32_t   StaticDetectionOutputBufferIndex: MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW94;

    // DW95
    union
    {
        struct
        {
            uint32_t   Reserved                        : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW95;

} MBENC_CURBE_CM, *PMBENC_CURBE_CM;
C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(MBENC_CURBE_CM)) == CodechalEncodeAvcEncG8::m_mbencCurbeSizeInDword);

typedef struct _BRC_BLOCK_COPY_CURBE_CM
{
    // uint32_t 0
    union
    {
        struct
        {
            uint32_t   BlockHeight         : 16;
            uint32_t   BufferOffset        : 16;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW0;

    // uint32_t 1
    union
    {
        struct
        {
            uint32_t   SrcSurfaceIndex;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW1;

    // uint32_t 2
    union
    {
        struct
        {
            uint32_t  DstSurfaceIndex;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW2;

    // uint64_t PADDING
    struct
    {
        uint32_t  Reserved;
    } PADDING;
} BRC_BLOCK_COPY_CURBE_CM, *PBRC_BLOCK_COPY_CURBE_CM;
//structure end

//viarables begin
const uint32_t CodechalEncodeAvcEncG8::m_initMBEncCurbeCmNormalIFrame[m_mbencCurbeSizeInDword] =
{
    0x00000082, 0x00000000, 0x00003910, 0x00a83000, 0x00000000, 0x28300000, 0x05000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x80800000, 0x00040c24, 0x00000000, 0xffff00ff, 0x40000000, 0x00000080, 0x00003900, 0x28301000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000002,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
};

const uint32_t CodechalEncodeAvcEncG8::m_initMBEncCurbeCmNormalIField[m_mbencCurbeSizeInDword] =
{
    0x00000082, 0x00000000, 0x00003910, 0x00a830c0, 0x02000000, 0x28300000, 0x05000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x80800000, 0x00040c24, 0x00000000, 0xffff00ff, 0x40000000, 0x00000080, 0x00003900, 0x28301000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000002,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
};

const uint32_t CodechalEncodeAvcEncG8::m_initMBEncCurbeCmNormalPFrame[m_mbencCurbeSizeInDword] =
{
    0x000000a3, 0x00000008, 0x00003910, 0x00ae3000, 0x30000000, 0x28300000, 0x05000000, 0x01400060,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x80010000, 0x00040c24, 0x00000000, 0xffff00ff, 0x60000000, 0x000000a1, 0x00003900, 0x28301000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x08000002,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
};

const uint32_t CodechalEncodeAvcEncG8::m_initMBEncCurbeCmNormalPField[m_mbencCurbeSizeInDword] =
{
    0x000000a3, 0x00000008, 0x00003910, 0x00ae30c0, 0x30000000, 0x28300000, 0x05000000, 0x01400060,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x80010000, 0x00040c24, 0x00000000, 0xffff00ff, 0x40000000, 0x000000a1, 0x00003900, 0x28301000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x04000002,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
};

const uint32_t CodechalEncodeAvcEncG8:: m_initMBEncCurbeCmNormalBFrame[m_mbencCurbeSizeInDword] =
{
    0x000000a3, 0x00200008, 0x00003910, 0x00aa7700, 0x50020000, 0x20200000, 0x05000000, 0xff400000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x01010000, 0x00040c24, 0x00000000, 0xffff00ff, 0x60000000, 0x000000a1, 0x00003900, 0x28301000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x08000002,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
};

const uint32_t CodechalEncodeAvcEncG8:: m_initMBEncCurbeCmNormalBField[m_mbencCurbeSizeInDword] =
{
    0x000000a3, 0x00200008, 0x00003919, 0x00aa77c0, 0x50020000, 0x20200000, 0x05000000, 0xff400000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x01010000, 0x00040c24, 0x00000000, 0xffff00ff, 0x40000000, 0x000000a1, 0x00003900, 0x28301000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x04000002,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
};

// AVC I_DIST CURBE init data for G8 CM Kernel
const uint32_t CodechalEncodeAvcEncG8::m_initMBEncCurbeCmIFrameDist[m_mbencCurbeSizeInDword] =
{
    0x00000082, 0x00200008, 0x001e3910, 0x00a83000, 0x90000000, 0x28300000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000100,
    0x80800000, 0x00000000, 0x00000800, 0xffff00ff, 0x40000000, 0x00000080, 0x00003900, 0x28300000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
};

// AVC ME CURBE init data for G8 CM Kernel
const uint32_t CodechalEncodeAvcEncG8::m_initMeCurbeCm[39] =
{
    0x00000000, 0x00200010, 0x00003939, 0x77a43000, 0x00000000, 0x28300000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
};

const int32_t CodechalEncodeAvcEncG8::m_brcBtCounts[CODECHAL_ENCODE_BRC_IDX_NUM] = {
    CODECHAL_ENCODE_AVC_BRC_INIT_RESET_NUM_SURFACES,
    BRC_UPDATE_NUM_SURFACES,
    CODECHAL_ENCODE_AVC_BRC_INIT_RESET_NUM_SURFACES,
    MBENC_NUM_SURFACES_CM,
    CODECHAL_ENCODE_AVC_BRC_BLOCK_COPY_NUM_SURFACES,
    0                                           // MbBRCUpdate kernel starting GEN9
};

const int32_t CodechalEncodeAvcEncG8::m_brcCurbeSize[CODECHAL_ENCODE_BRC_IDX_NUM] = {
    (sizeof(BRC_INIT_RESET_CURBE)),
    (sizeof(BRC_UPDATE_CURBE)),
    (sizeof(BRC_INIT_RESET_CURBE)),
    (sizeof(MBENC_CURBE_CM)),
    (sizeof(BRC_BLOCK_COPY_CURBE_CM)),
    0                                           // MbBRCUpdate kernel starting GEN9
};

const uint32_t CodechalEncodeAvcEncG8::m_trellisQuantizationRounding[NUM_TARGET_USAGE_MODES] =
{
    0, 6, 0, 0, 0, 0, 0, 0
};

static const CODECHAL_ENCODE_AVC_IPCM_THRESHOLD initIpcmThresholdTable[5] =
{
    { 2,    3000 },
    { 4,    3600 },
    { 6,    5000 },
    { 10,   7500 },
    { 18,   9000 },
};

static const BRC_INIT_RESET_CURBE initBrcInitResetCurbe =
{
    // uint32_t 0
    {
        {
            0
        }
    },

    // uint32_t 1
    {
        {
            0
        }
    },

    // uint32_t 2
    {
        {
            0
        }
    },

    // uint32_t 3
    {
        {
            0
        }
    },

    // uint32_t 4
    {
        {
            0
        }
    },

    // uint32_t 5
    {
        {
            0
        }
    },

    // uint32_t 6
    {
        {
            0
        }
    },

    // uint32_t 7
    {
        {
            0
        }
    },

    // uint32_t 8
    {
        {
            0,
            0
        }
    },

    // uint32_t 9
    {
        {
            0,
            0
        }
    },

    // uint32_t 10
    {
        {
            0,
            0
        }
    },

    // uint32_t 11
    {
        {
            0,
            1
        }
    },

    // uint32_t 12
    {
        {
            51,
            0
        }
    },

    // uint32_t 13
    {
        {
            40,
            60,
            80,
            120
        }
    },

    // uint32_t 14
    {
        {
            35,
            60,
            80,
            120
        }
    },

    // uint32_t 15
    {
        {
            40,
            60,
            90,
            115
        }
    },

    // uint32_t 16
    {
        {
            0,
            0,
            0,
            0
        }
    },

    // uint32_t 17
    {
        {
            0,
            0,
            0,
            0
        }
    },

    // uint32_t 18
    {
        {
            0,
            0,
            0,
            0
        }
    },

    // uint32_t 19
    {
        {
            0,
            0,
            0,
            0
        }
    },

    // uint32_t 20
    {
        {
            0,
            0,
            0,
            0
        }
    },

    // uint32_t 21
    {
        {
            0,
            0,
            0,
            0
        }
    },

    // uint32_t 22
    {
        {
            0,
            0,
            0,
            0
        }
    },

    // uint32_t 23
    {
        {
            0
        }
    }
};

static const BRC_UPDATE_CURBE initBrcUpdateCurbe =
{
    // uint32_t 0
    {
        {
            0
        }
    },

    // uint32_t 1
    {
        {
            0
        }
    },

    // uint32_t 2
    {
        {
            0
        }
    },

    // uint32_t 3
    {
        {
            10,
            50
        }
    },

    // uint32_t 4
    {
        {
            100,
            150
        }
    },

    // uint32_t 5
    {
        {
            0,
            0,
            0,
            0
        }
    },

    // uint32_t 6
    {
        {
            0,
            0,
            0,
            0,
            0,
            0
        }
    },

    // uint32_t 7
    {
        {
            0
        }
    },

    // uint32_t 8
    {
        {
            1,
            1,
            3,
            2
        }
    },

    // uint32_t 9
    {
        {
            1,
            40,
            5,
            5
        }
    },

    // uint32_t 10
    {
        {
            3,
            1,
            7,
            18
        }
    },

    // uint32_t 11
    {
        {
            25,
            37,
            40,
            75
        }
    },

    // uint32_t 12
    {
        {
            97,
            103,
            125,
            160
        }
    },

    // uint32_t 13
    {
        {
            MOS_BITFIELD_VALUE((uint32_t)-3, 8),
            MOS_BITFIELD_VALUE((uint32_t)-2, 8),
            MOS_BITFIELD_VALUE((uint32_t)-1, 8),
            0
        }
    },

    // uint32_t 14
    {
        {
            1,
            2,
            3,
            0xff
        }
    },

    // uint32_t 15
    {
        {
            0,
            0,
            0,
            0
        }
    },

    // uint32_t 16
    {
        {
            0,
            0
        }
    },

    // uint32_t 17
    {
        {
            0,
            0
        }
    },

    // uint32_t 18
    {
        {
            0,
            0
        }
    },

    // uint32_t 19
    {
        {
            0
        }
    },

    // uint32_t 20
    {
        {
            0
        }
    },

    // uint32_t 21
    {
        {
            0
        }
    },

    // uint32_t 22
    {
        {
            0
        }
    },

    // uint32_t 23
    {
        {
            0
        }
    },
};
//variables end
CodechalEncodeAvcEncG8::CodechalEncodeAvcEncG8(
        CodechalHwInterface *   hwInterface,
        CodechalDebugInterface *debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo) : CodechalEncodeAvcEnc(hwInterface, debugInterface, standardInfo)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    m_kernelBase = (uint8_t *)IGCODECKRN_G8;
    AddIshSize(m_kuid, m_kernelBase);

    m_cmKernelEnable           = true;
    bBrcSplitEnable            = false;
    bHighTextureModeCostEnable = false;
    m_feiEnable                = CodecHalIsFeiEncode(m_codecFunction);

    this->pfnGetKernelHeaderAndSize = this->GetKernelHeaderAndSize;

    m_needCheckCpEnabled = true;
}

MOS_STATUS CodechalEncodeAvcEncG8::GetKernelHeaderAndSize(
    void                           *pvBinary,
    EncOperation                   operation,
    uint32_t                       krnStateIdx,
    void                           *pvKrnHeader,
    uint32_t                       *pdwKrnSize)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(pvBinary);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pvKrnHeader);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pdwKrnSize);

    auto kernelHeaderTable = (PKERNEL_HEADER_CM)pvBinary;
    auto invalidaEntry = &(kernelHeaderTable->AVC_StaticFrameDetection) + 1;
    uint32_t nextKrnOffset = *pdwKrnSize;

    PCODECHAL_KERNEL_HEADER currKrnHeader = nullptr, nextKrnHeader = nullptr;

    if (operation == ENC_SCALING4X)
    {
        currKrnHeader = &kernelHeaderTable->PLY_DScale_PLY;
    }
    else if (operation == ENC_SCALING2X)
    {
        currKrnHeader = &kernelHeaderTable->PLY_2xDScale_PLY;
    }
    else if (operation == ENC_ME)
    {
        currKrnHeader = &kernelHeaderTable->AVC_ME_P;
    }
    else if (operation == ENC_BRC)
    {
        currKrnHeader = &kernelHeaderTable->InitFrameBRC;
    }
    else if (operation == ENC_MBENC)
    {
        currKrnHeader = &kernelHeaderTable->AVCMBEnc_Qlty_I;
    }
    else if (operation == ENC_MBENC_ADV)
    {
        currKrnHeader = &kernelHeaderTable->AVCMBEnc_Adv_I;
    }
    else if (operation == ENC_SFD)
    {
        currKrnHeader = &kernelHeaderTable->AVC_StaticFrameDetection;
    }
    else
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Unsupported ENC mode requested");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
    }

    if (currKrnHeader == nullptr)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    currKrnHeader += krnStateIdx;
    *((PCODECHAL_KERNEL_HEADER)pvKrnHeader) = *currKrnHeader;

    nextKrnHeader = (currKrnHeader + 1);
    if (nextKrnHeader < invalidaEntry)
    {
        nextKrnOffset = nextKrnHeader->KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT;
    }
    *pdwKrnSize = nextKrnOffset - (currKrnHeader->KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);

    return eStatus;
}

CodechalEncodeAvcEncG8::~CodechalEncodeAvcEncG8()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;
}

MOS_STATUS CodechalEncodeAvcEncG8::InitializeState()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncodeAvcEnc::InitializeState());

    MOS_USER_FEATURE_VALUE_DATA     userFeatureData;
    MOS_STATUS                      regReadStatus;

    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
     regReadStatus = MOS_UserFeature_ReadValue_ID(
      nullptr,
      __MEDIA_USER_FEATURE_VALUE_AVC_ENCODE_INTRA_REFRESH_QP_THRESHOLD_ID,
      &userFeatureData);
    dwIntraRefreshQpThreshold = (MOS_STATUS_SUCCESS == regReadStatus) ? userFeatureData.i32Data : (CODEC_AVC_NUM_QP - 1);

    bWeightedPredictionSupported = false;
    m_brcHistoryBufferSize = m_initBrcHistoryBufferSize;
    dwBrcConstantSurfaceWidth = m_brcConstantSurfaceWidth;
    dwBrcConstantSurfaceHeight = m_brcConstantSurfaceHeight;
    bPerMbSFD = false;
    m_forceBrcMbStatsEnabled     = false;
    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG8::InitMbBrcConstantDataBuffer(PCODECHAL_ENCODE_AVC_INIT_MBBRC_CONSTANT_DATA_BUFFER_PARAMS params)
{
    MOS_STATUS      eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pOsInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->presBrcConstantDataBuffer);

    MOS_LOCK_PARAMS lockFlags;
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.WriteOnly = 1;
    auto data = (uint32_t*)params->pOsInterface->pfnLockResource(
        params->pOsInterface,
        params->presBrcConstantDataBuffer,
        &lockFlags);
    CODECHAL_ENCODE_CHK_NULL_RETURN(data);

    // 16 DWs per QP value
    uint32_t size = 16 * CODEC_AVC_NUM_QP;
    if (params->bPreProcEnable)
    {
        eStatus = MOS_SecureMemcpy(data, size * sizeof(uint32_t), (void*)PreProcFtqLut_Cm_Common, size * sizeof(uint32_t));
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
            return eStatus;
        }
    }
    else
    {
        CODECHAL_ENCODE_CHK_NULL_RETURN(params->pPicParams);

        uint8_t tableIdx = params->wPictureCodingType - 1;
        bool blockBaseSkipEn = params->dwMbEncBlockBasedSkipEn ? true : false;
        bool transform8x8ModeFlag = params->pPicParams->transform_8x8_mode_flag ? true : false;

        if (tableIdx >= 3)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Invalid input parameter.");
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            return eStatus;
        }

        eStatus = MOS_SecureMemcpy(data, size * sizeof(uint32_t), (void*)MBBrcConstantData_Cm_Common[tableIdx], size * sizeof(uint32_t));
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
            return eStatus;
        }

        auto dataBk = data;
        switch (params->wPictureCodingType)
        {
        case I_TYPE:
            for (uint8_t qp = 0; qp < CODEC_AVC_NUM_QP; qp++)
            {
                // Writing to DW0 in each sub-array of 16 DWs
                if(params->bOldModeCostEnable)
                {
                    *data = (uint32_t)OldIntraModeCost_Cm_Common[qp];
                }
                data += 16;
            }
            break;
        case P_TYPE:
        case B_TYPE:
            for (uint8_t qp = 0; qp < CODEC_AVC_NUM_QP; qp++)
            {
                if(params->wPictureCodingType == P_TYPE)
                {
                    // Writing to DW3 in each sub-array of 16 DWs
                    if (params->bSkipBiasAdjustmentEnable)
                    {
                        *(data + 3) = (uint32_t)MvCost_PSkipAdjustment_Cm_Common[qp];
                    }
                }

                // Writing to DW9 in each sub-array of 16 DWs
                if(params->pAvcQCParams && params->pAvcQCParams->NonFTQSkipThresholdLUTInput)
                {
                    *(data + 9) = (uint32_t)CalcSkipVal((params->dwMbEncBlockBasedSkipEn ? true : false),
                                                                (transform8x8ModeFlag ? true : false),
                                                                (uint16_t)(params->pAvcQCParams->NonFTQSkipThresholdLUT[qp]));
                }
                else if(params->wPictureCodingType == P_TYPE)
                {
                    *(data + 9) = (uint32_t)SkipVal_P_Common[blockBaseSkipEn][transform8x8ModeFlag][qp];
                }
                else
                {
                    *(data + 9) = (uint32_t)SkipVal_B_Common[blockBaseSkipEn][transform8x8ModeFlag][qp];
                }

                // Writing to DW10 in each sub-array of 16 DWs
                if (params->bAdaptiveIntraScalingEnable)
                {
                    *(data + 10) = (uint32_t)AdaptiveIntraScalingFactor_Cm_Common[qp];
                }
                else
                {
                    *(data + 10) = (uint32_t)IntraScalingFactor_Cm_Common[qp];
                }
                data += 16;
            }
            break;
        default:
            break;
        }

        data = dataBk;
        for (uint8_t qp = 0; qp < CODEC_AVC_NUM_QP; qp++)
        {
            if(params->pAvcQCParams && params->pAvcQCParams->FTQSkipThresholdLUTInput)
            {
                *(data + 6) = ((uint32_t)params->pAvcQCParams->FTQSkipThresholdLUT[qp])
                             | (((uint32_t)params->pAvcQCParams->FTQSkipThresholdLUT[qp]) << 16)
                             | (((uint32_t)params->pAvcQCParams->FTQSkipThresholdLUT[qp]) << 24);
                *(data + 7) = ((uint32_t)params->pAvcQCParams->FTQSkipThresholdLUT[qp])
                             | (((uint32_t)params->pAvcQCParams->FTQSkipThresholdLUT[qp]) << 8)
                             | (((uint32_t)params->pAvcQCParams->FTQSkipThresholdLUT[qp]) << 16)
                             | (((uint32_t)params->pAvcQCParams->FTQSkipThresholdLUT[qp]) << 24);
            }

            if (params->bEnableKernelTrellis)
            {
                // Writing uint32_t 11 and uint32_t 12 with Lambda values
                *(data + 11) = (uint32_t)params->Lambda[qp][0];
                *(data + 12) = (uint32_t)params->Lambda[qp][1];
            }
            data += 16;
        }
    }

    params->pOsInterface->pfnUnlockResource(
        params->pOsInterface,
        params->presBrcConstantDataBuffer);

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG8::InitKernelStateWP()
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_kernelBase);

    uint8_t* kernelBinary;
    uint32_t kernelSize;

    MOS_STATUS status = CodecHalGetKernelBinaryAndSize(m_kernelBase, m_kuid, &kernelBinary, &kernelSize);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(status);

    CODECHAL_KERNEL_HEADER currKrnHeader;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(GetKernelHeaderAndSize(
        kernelBinary,
        ENC_WP,
        0,
        &currKrnHeader,
        &kernelSize));

    pWPKernelState= MOS_New(MHW_KERNEL_STATE);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pWPKernelState);

    auto kernelStatePtr = pWPKernelState;

    kernelStatePtr->KernelParams.iBTCount = WP_NUM_SURFACES;
    kernelStatePtr->KernelParams.iThreadCount = m_renderEngineInterface->GetHwCaps()->dwMaxThreads;
    kernelStatePtr->KernelParams.iCurbeLength = sizeof(WP_CURBE);
    kernelStatePtr->KernelParams.iBlockWidth = CODECHAL_MACROBLOCK_WIDTH;
    kernelStatePtr->KernelParams.iBlockHeight = CODECHAL_MACROBLOCK_HEIGHT;
    kernelStatePtr->KernelParams.iIdCount = 1;
    kernelStatePtr->KernelParams.iInlineDataLength = 0;
    kernelStatePtr->dwCurbeOffset = m_stateHeapInterface->pStateHeapInterface->m_wSizeOfCmdInterfaceDescriptorData;
    kernelStatePtr->KernelParams.pBinary = kernelBinary + (currKrnHeader.KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);
    kernelStatePtr->KernelParams.iSize = kernelSize;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnCalculateSshAndBtSizesRequested(
        m_stateHeapInterface,
        kernelStatePtr->KernelParams.iBTCount,
        &kernelStatePtr->dwSshSize,
        &kernelStatePtr->dwBindingTableSize));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->MhwInitISH(m_stateHeapInterface, kernelStatePtr));
    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG8::GetMbEncKernelStateIdx(CodechalEncodeIdOffsetParams* params, uint32_t* kernelOffset)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(kernelOffset);

    *kernelOffset = MBENC_I_OFFSET_CM;

    if (params->EncFunctionType == CODECHAL_MEDIA_STATE_ENC_ADV)
    {
        *kernelOffset +=
            MBENC_TARGET_USAGE_CM * m_mbencNumTargetUsages;
    }
    else
    {
        if (params->EncFunctionType == CODECHAL_MEDIA_STATE_ENC_NORMAL)
        {
            *kernelOffset += MBENC_TARGET_USAGE_CM;
        }
        else if (params->EncFunctionType == CODECHAL_MEDIA_STATE_ENC_PERFORMANCE)
        {
            *kernelOffset += MBENC_TARGET_USAGE_CM * 2;
        }
    }

    if (params->wPictureCodingType == P_TYPE)
    {
        *kernelOffset += MBENC_P_OFFSET_CM;
    }
    else if (params->wPictureCodingType == B_TYPE)
    {
        *kernelOffset += MBENC_B_OFFSET_CM;
    }

    return eStatus;

}

MOS_STATUS CodechalEncodeAvcEncG8::InitBrcConstantBuffer(
        PCODECHAL_ENCODE_AVC_INIT_BRC_CONSTANT_BUFFER_PARAMS        params)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (bMultiRefQpEnabled)
    {
        return InitBrcConstantBufferMultiRefQP(params);
    }
    else
    {
        return CodechalEncodeAvcEnc::InitBrcConstantBuffer(params);
    }
}

MOS_STATUS CodechalEncodeAvcEncG8::InitBrcConstantBufferMultiRefQP(
        PCODECHAL_ENCODE_AVC_INIT_BRC_CONSTANT_BUFFER_PARAMS        params)
{

    MOS_STATUS          eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pOsInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pPicParams);

    uint8_t tableIdx  = params->wPictureCodingType - 1;
    if(tableIdx >= 3)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Invalid input parameter.");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }
    MOS_LOCK_PARAMS     lockFlags;
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.WriteOnly = 1;
    auto data = (uint8_t*)params->pOsInterface->pfnLockResource(
        params->pOsInterface,
        &params->sBrcConstantDataBuffer.OsResource,
        &lockFlags);
    CODECHAL_ENCODE_CHK_NULL_RETURN(data);

    MOS_ZeroMemory(data, params->sBrcConstantDataBuffer.dwWidth * params->sBrcConstantDataBuffer.dwHeight);

    // Fill surface with QP Adjustment table, Distortion threshold table, MaxFrame threshold table, Distortion QP Adjustment Table
    eStatus = MOS_SecureMemcpy(
        data,
        sizeof(m_qpDistMaxFrameAdjustmentCm),
        (void*)m_qpDistMaxFrameAdjustmentCm,
        sizeof(m_qpDistMaxFrameAdjustmentCm));
    if(eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
        return eStatus;
    }

    data += sizeof(m_qpDistMaxFrameAdjustmentCm);
    bool blockBaseSkipEn = params->dwMbEncBlockBasedSkipEn ? true : false;
    bool transform8x8ModeFlag    = params->pPicParams->transform_8x8_mode_flag ? true : false;
    // Fill surface with Skip Threshold Table
    switch(params->wPictureCodingType)
    {
        case P_TYPE:
            eStatus = MOS_SecureMemcpy(
                data,
                m_brcConstantSurfaceEarlySkipTableSize,
                (void*)&SkipVal_P_Common[blockBaseSkipEn][transform8x8ModeFlag][0],
                m_brcConstantSurfaceEarlySkipTableSize);
            if(eStatus != MOS_STATUS_SUCCESS)
            {
                CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
                return eStatus;
            }
            break;
        case B_TYPE:
            eStatus = MOS_SecureMemcpy(
                data,
                m_brcConstantSurfaceEarlySkipTableSize,
                (void*)&SkipVal_B_Common[blockBaseSkipEn][transform8x8ModeFlag][0],
                m_brcConstantSurfaceEarlySkipTableSize);
            if(eStatus != MOS_STATUS_SUCCESS)
            {
                CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
                return eStatus;
            }
            break;
        default:
            // do nothing for I TYPE
            break;
    }

    if((params->wPictureCodingType != I_TYPE) && (params->pAvcQCParams!= nullptr) && (params->pAvcQCParams->NonFTQSkipThresholdLUTInput))
    {
        for(uint8_t qp = 0; qp < CODEC_AVC_NUM_QP; qp++)
        {
            *(data+ 1 + (qp * 2)) = (uint8_t) CalcSkipVal((params->dwMbEncBlockBasedSkipEn ? true : false),
                (params->pPicParams->transform_8x8_mode_flag ? true : false), (uint16_t)(params->pAvcQCParams->NonFTQSkipThresholdLUT[qp]));
        }
    }

    data += m_brcConstantSurfaceEarlySkipTableSize;

    // Fill surface with QP list

    // Initialize to -1 (0xff)
    MOS_FillMemory(data, m_brcConstantSurfaceQpList0, 0xff);
    MOS_FillMemory(data
                    + m_brcConstantSurfaceQpList0
                    + m_brcConstantSurfaceQpList0Reserved,
                    m_brcConstantSurfaceQpList1, 0xff);

    switch(params->wPictureCodingType)
    {
        case B_TYPE:
            data += (m_brcConstantSurfaceQpList0 + m_brcConstantSurfaceQpList0Reserved);

            for (uint8_t refIdx = 0;refIdx <= params->pAvcSlcParams->num_ref_idx_l1_active_minus1;refIdx++)
            {
                CODEC_PICTURE refPic = params->pAvcSlcParams->RefPicList[LIST_1][refIdx];
                if (!CodecHal_PictureIsInvalid(refPic) && params->pAvcPicIdx[refPic.FrameIdx].bValid)
                {
                    *(data + refIdx) = params->pAvcPicIdx[refPic.FrameIdx].ucPicIdx;
                }
            }
            data -= (m_brcConstantSurfaceQpList0 + m_brcConstantSurfaceQpList0Reserved);
            // break statement omitted intentionally
        case P_TYPE:
            for (uint8_t refIdx = 0;refIdx <= params->pAvcSlcParams->num_ref_idx_l0_active_minus1;refIdx++)
            {
                CODEC_PICTURE refPic = params->pAvcSlcParams->RefPicList[LIST_0][refIdx];
                if (!CodecHal_PictureIsInvalid(refPic) && params->pAvcPicIdx[refPic.FrameIdx].bValid)
                {
                    *(data + refIdx) = params->pAvcPicIdx[refPic.FrameIdx].ucPicIdx;
                }
            }
            break;
        default:
            // do nothing for I type
            break;
    }

    data += (m_brcConstantSurfaceQpList0 + m_brcConstantSurfaceQpList0Reserved
                + m_brcConstantSurfaceQpList1 + m_brcConstantSurfaceQpList1Reserved);

    // Fill surface with Mode cost and MV cost
    eStatus = MOS_SecureMemcpy(
        data,
        m_brcConstantSurfacModeMvCostSize,
        (void*)ModeMvCost_Cm[tableIdx],
        m_brcConstantSurfacModeMvCostSize);
    if(eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
        return eStatus;
    }

    // If old mode cost is used the update the table
    if(params->wPictureCodingType == I_TYPE && params->bOldModeCostEnable)
    {
        auto pdwDataTemp = (uint32_t*) data;
        for (uint8_t qp = 0; qp < CODEC_AVC_NUM_QP; qp++)
        {
            // Writing to DW0 in each sub-array of 16 DWs
            *pdwDataTemp = (uint32_t)OldIntraModeCost_Cm_Common[qp];
            pdwDataTemp += 16;
        }
    }

    if(params->pAvcQCParams)
    {
        for(uint8_t qp = 0; qp < CODEC_AVC_NUM_QP; qp++)
        {
            if(params->pAvcQCParams->FTQSkipThresholdLUTInput)
            {
                *(data + (qp *32) + 24) =
                *(data + (qp *32) + 25) =
                *(data + (qp *32) + 27) =
                *(data + (qp *32) + 28) =
                *(data + (qp *32) + 29) =
                *(data + (qp *32) + 30) =
                *(data + (qp *32) + 31) = params->pAvcQCParams->FTQSkipThresholdLUT[qp];
            }
        }
    }

    data += m_brcConstantSurfacModeMvCostSize;

    // Fill surface with Refcost
    eStatus = MOS_SecureMemcpy(
        data,
        m_brcConstantSurfaceRefCostSize,
        (void*)&RefCost_MultiRefQp[tableIdx][0],
        m_brcConstantSurfaceRefCostSize);
    if(eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
        return eStatus;
    }
    data += m_brcConstantSurfaceRefCostSize;

    //Fill surface with Intra cost scaling Factor
    if(params->bAdaptiveIntraScalingEnable)
    {
        eStatus = MOS_SecureMemcpy(
        data,
        m_brcConstantSurfaceIntraCostScalingFactor,
        (void*)&AdaptiveIntraScalingFactor_Cm_Common[0],
        m_brcConstantSurfaceIntraCostScalingFactor);
        if(eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
            return eStatus;
        }
    }
    else
    {
        eStatus = MOS_SecureMemcpy(
        data,
        m_brcConstantSurfaceIntraCostScalingFactor,
        (void*)&IntraScalingFactor_Cm_Common[0],
        m_brcConstantSurfaceIntraCostScalingFactor);
        if(eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
            return eStatus;
        }
    }

    params->pOsInterface->pfnUnlockResource(
        params->pOsInterface,
        &params->sBrcConstantDataBuffer.OsResource);

    return eStatus;

}

MOS_STATUS CodechalEncodeAvcEncG8::GetTrellisQuantization(
        PCODECHAL_ENCODE_AVC_TQ_INPUT_PARAMS    params,
        PCODECHAL_ENCODE_AVC_TQ_PARAMS          trellisQuantParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(trellisQuantParams);

    trellisQuantParams->dwTqEnabled    = TrellisQuantizationEnable[params->ucTargetUsage];
    trellisQuantParams->dwTqRounding   =
        trellisQuantParams->dwTqEnabled ? m_trellisQuantizationRounding[params->ucTargetUsage] : 0;
    // If AdaptiveTrellisQuantization is enabled then disable trellis quantization for
    // B-frames with QP > 26 only in CQP mode
    if(trellisQuantParams->dwTqEnabled
        && EnableAdaptiveTrellisQuantization[params->ucTargetUsage]
        && params->wPictureCodingType == B_TYPE
        && !params->bBrcEnabled && params->ucQP > 26)
    {
        trellisQuantParams->dwTqEnabled  = 0;
        trellisQuantParams->dwTqRounding = 0;
    }
    return eStatus;

}

MOS_STATUS CodechalEncodeAvcEncG8::InitKernelStateMe()
{
    MOS_STATUS                          eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;
    uint8_t* kernelBinary;
    uint32_t kernelSize;

    MOS_STATUS status = CodecHalGetKernelBinaryAndSize(m_kernelBase, m_kuid, &kernelBinary, &kernelSize);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(status);

    for (uint32_t krnStateIdx = 0; krnStateIdx < 2; krnStateIdx++)
    {
        auto kernelStatePtr = &m_meKernelStates[krnStateIdx];

        CODECHAL_KERNEL_HEADER   currKrnHeader;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pfnGetKernelHeaderAndSize(
            kernelBinary,
            ENC_ME,
            krnStateIdx,
            &currKrnHeader,
            &kernelSize));

        kernelStatePtr->KernelParams.iBTCount = ME_NUM_SURFACES_CM;
        kernelStatePtr->KernelParams.iThreadCount = m_renderEngineInterface->GetHwCaps()->dwMaxThreads;
        kernelStatePtr->KernelParams.iCurbeLength = sizeof(ME_CURBE_CM);
        kernelStatePtr->KernelParams.iBlockWidth = CODECHAL_MACROBLOCK_WIDTH;
        kernelStatePtr->KernelParams.iBlockHeight = CODECHAL_MACROBLOCK_HEIGHT;
        kernelStatePtr->KernelParams.iIdCount = 1;

        kernelStatePtr->dwCurbeOffset = m_stateHeapInterface->pStateHeapInterface->GetSizeofCmdInterfaceDescriptorData();
        kernelStatePtr->KernelParams.pBinary = kernelBinary + (currKrnHeader.KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);
        kernelStatePtr->KernelParams.iSize = kernelSize;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnCalculateSshAndBtSizesRequested(
            m_stateHeapInterface,
            kernelStatePtr->KernelParams.iBTCount,
            &kernelStatePtr->dwSshSize,
            &kernelStatePtr->dwBindingTableSize));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->MhwInitISH(m_stateHeapInterface, kernelStatePtr));
    }

    // Until a better way can be found, maintain old binding table structures
    auto bindingTable = &m_meBindingTable;
    bindingTable->dwMEMVDataSurface    = ME_MV_DATA_SURFACE_CM;
    bindingTable->dw16xMEMVDataSurface = ME_16x_MV_DATA_SURFACE_CM;
    bindingTable->dw32xMEMVDataSurface = ME_32x_MV_DATA_SURFACE_CM;
    bindingTable->dwMEDist             = ME_DISTORTION_SURFACE_CM;
    bindingTable->dwMEBRCDist          = ME_BRC_DISTORTION_CM;
    bindingTable->dwMECurrForFwdRef    = ME_CURR_FOR_FWD_REF_CM;
    bindingTable->dwMEFwdRefPicIdx[0]  = ME_FWD_REF_IDX0_CM;
    bindingTable->dwMEFwdRefPicIdx[1]  = ME_FWD_REF_IDX1_CM;
    bindingTable->dwMEFwdRefPicIdx[2]  = ME_FWD_REF_IDX2_CM;
    bindingTable->dwMEFwdRefPicIdx[3]  = ME_FWD_REF_IDX3_CM;
    bindingTable->dwMEFwdRefPicIdx[4]  = ME_FWD_REF_IDX4_CM;
    bindingTable->dwMEFwdRefPicIdx[5]  = ME_FWD_REF_IDX5_CM;
    bindingTable->dwMEFwdRefPicIdx[6]  = ME_FWD_REF_IDX6_CM;
    bindingTable->dwMEFwdRefPicIdx[7]  = ME_FWD_REF_IDX7_CM;
    bindingTable->dwMECurrForBwdRef    = ME_CURR_FOR_BWD_REF_CM;
    bindingTable->dwMEBwdRefPicIdx[0]  = ME_BWD_REF_IDX0_CM;
    bindingTable->dwMEBwdRefPicIdx[1]  = ME_BWD_REF_IDX1_CM;

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG8::InitKernelStateMbEnc()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    dwNumMbEncEncKrnStates = MBENC_TARGET_USAGE_CM * m_mbencNumTargetUsagesCm;
    dwNumMbEncEncKrnStates += MBENC_TARGET_USAGE_CM;
    pMbEncKernelStates =
        MOS_NewArray(MHW_KERNEL_STATE, dwNumMbEncEncKrnStates);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pMbEncKernelStates);

    uint8_t* kernelBinary;
    uint32_t kernelSize;

    MOS_STATUS status = CodecHalGetKernelBinaryAndSize(m_kernelBase, m_kuid, &kernelBinary, &kernelSize);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(status);

    auto kernelStatePtr   = pMbEncKernelStates;
    for (uint32_t krnStateIdx = 0; krnStateIdx < dwNumMbEncEncKrnStates; krnStateIdx++)
    {
        bool kernelState = (krnStateIdx >= MBENC_TARGET_USAGE_CM * m_mbencNumTargetUsagesCm);
        CODECHAL_KERNEL_HEADER currKrnHeader;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pfnGetKernelHeaderAndSize(
            kernelBinary,
            (kernelState ? ENC_MBENC_ADV : ENC_MBENC),
            (kernelState ? krnStateIdx - MBENC_TARGET_USAGE_CM * m_mbencNumTargetUsagesCm : krnStateIdx),
            &currKrnHeader,
            &kernelSize));

        kernelStatePtr->KernelParams.iBTCount = MBENC_NUM_SURFACES_CM;
        auto renderEngineInterface  = m_hwInterface->GetRenderInterface();
        kernelStatePtr->KernelParams.iThreadCount = m_renderEngineInterface->GetHwCaps()->dwMaxThreads;
        kernelStatePtr->KernelParams.iCurbeLength = sizeof(MBENC_CURBE_CM);
        kernelStatePtr->KernelParams.iBlockWidth = CODECHAL_MACROBLOCK_WIDTH;
        kernelStatePtr->KernelParams.iBlockHeight = CODECHAL_MACROBLOCK_HEIGHT;
        kernelStatePtr->KernelParams.iIdCount = 1;

        auto stateHeapInterface = m_renderEngineInterface->m_stateHeapInterface;
        kernelStatePtr->dwCurbeOffset = stateHeapInterface->pStateHeapInterface->GetSizeofCmdInterfaceDescriptorData();
        kernelStatePtr->KernelParams.pBinary = kernelBinary + (currKrnHeader.KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);
        kernelStatePtr->KernelParams.iSize = kernelSize;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(stateHeapInterface->pfnCalculateSshAndBtSizesRequested(
            stateHeapInterface,
            kernelStatePtr->KernelParams.iBTCount,
            &kernelStatePtr->dwSshSize,
            &kernelStatePtr->dwBindingTableSize));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->MhwInitISH(stateHeapInterface, kernelStatePtr));

        kernelStatePtr++;
    }

    // Until a better way can be found, maintain old binding table structures
    auto bindingTable = &MbEncBindingTable;

    bindingTable->dwAvcMBEncMfcAvcPakObj       = MBENC_MFC_AVC_PAK_OBJ_CM;
    bindingTable->dwAvcMBEncIndMVData          = MBENC_IND_MV_DATA_CM;
    bindingTable->dwAvcMBEncBRCDist            = MBENC_BRC_DISTORTION_CM;
    bindingTable->dwAvcMBEncCurrY              = MBENC_CURR_Y_CM;
    bindingTable->dwAvcMBEncCurrUV             = MBENC_CURR_UV_CM;
    bindingTable->dwAvcMBEncMbSpecificData     = MBENC_MB_SPECIFIC_DATA_CM;

    bindingTable->dwAvcMBEncRefPicSelectL0     = MBENC_REFPICSELECT_L0_CM;
    bindingTable->dwAvcMBEncMVDataFromME       = MBENC_MV_DATA_FROM_ME_CM;
    bindingTable->dwAvcMBEncMEDist             = MBENC_4xME_DISTORTION_CM;
    bindingTable->dwAvcMBEncSliceMapData       = MBENC_SLICEMAP_DATA_CM;
    bindingTable->dwAvcMBEncBwdRefMBData       = MBENC_FWD_MB_DATA_CM;
    bindingTable->dwAvcMBEncBwdRefMVData       = MBENC_FWD_MV_DATA_CM;
    bindingTable->dwAvcMBEncMbBrcConstData     = MBENC_MBBRC_CONST_DATA_CM;
    bindingTable->dwAvcMBEncFlatnessChk        = MBENC_FLATNESS_CHECK_CM;
    bindingTable->dwAvcMBEncMADData            = MBENC_MAD_DATA_CM;
    bindingTable->dwAvcMBEncAdv                = MBENC_ADV_WA_DATA_CM;
    bindingTable->dwAvcMBEncMbNonSkipMap       = MBENC_FORCE_NONSKIP_MB_MAP_CM;
    bindingTable->dwAvcMbEncBRCCurbeData       = MBENC_BRC_CURBE_DATA_CM;

    // Frame
    bindingTable->dwAvcMBEncMbQpFrame          = MBENC_MBQP_CM;
    bindingTable->dwAvcMBEncCurrPicFrame[0]    = MBENC_VME_INTER_PRED_CURR_PIC_IDX_0_CM;
    bindingTable->dwAvcMBEncFwdPicFrame[0]     = MBENC_VME_INTER_PRED_FWD_PIC_IDX0_CM;
    bindingTable->dwAvcMBEncBwdPicFrame[0]     = MBENC_VME_INTER_PRED_BWD_PIC_IDX0_0_CM;
    bindingTable->dwAvcMBEncFwdPicFrame[1]     = MBENC_VME_INTER_PRED_FWD_PIC_IDX1_CM;
    bindingTable->dwAvcMBEncBwdPicFrame[1]     = MBENC_VME_INTER_PRED_BWD_PIC_IDX1_0_CM;
    bindingTable->dwAvcMBEncFwdPicFrame[2]     = MBENC_VME_INTER_PRED_FWD_PIC_IDX2_CM;
    bindingTable->dwAvcMBEncFwdPicFrame[3]     = MBENC_VME_INTER_PRED_FWD_PIC_IDX3_CM;
    bindingTable->dwAvcMBEncFwdPicFrame[4]     = MBENC_VME_INTER_PRED_FWD_PIC_IDX4_CM;
    bindingTable->dwAvcMBEncFwdPicFrame[5]     = MBENC_VME_INTER_PRED_FWD_PIC_IDX5_CM;
    bindingTable->dwAvcMBEncFwdPicFrame[6]     = MBENC_VME_INTER_PRED_FWD_PIC_IDX6_CM;
    bindingTable->dwAvcMBEncFwdPicFrame[7]     = MBENC_VME_INTER_PRED_FWD_PIC_IDX7_CM;
    bindingTable->dwAvcMBEncCurrPicFrame[1]    = MBENC_VME_INTER_PRED_CURR_PIC_IDX_1_CM;
    bindingTable->dwAvcMBEncBwdPicFrame[2]     = MBENC_VME_INTER_PRED_BWD_PIC_IDX0_1_CM;
    bindingTable->dwAvcMBEncBwdPicFrame[3]     = MBENC_VME_INTER_PRED_BWD_PIC_IDX1_1_CM;

    // Field
    bindingTable->dwAvcMBEncMbQpField          = MBENC_MBQP_CM;
    bindingTable->dwAvcMBEncFieldCurrPic[0]    = MBENC_VME_INTER_PRED_CURR_PIC_IDX_0_CM;
    bindingTable->dwAvcMBEncFwdPicTopField[0]  = MBENC_VME_INTER_PRED_FWD_PIC_IDX0_CM;
    bindingTable->dwAvcMBEncBwdPicTopField[0]  = MBENC_VME_INTER_PRED_BWD_PIC_IDX0_0_CM;
    bindingTable->dwAvcMBEncFwdPicBotField[0]  = MBENC_VME_INTER_PRED_FWD_PIC_IDX0_CM;
    bindingTable->dwAvcMBEncBwdPicBotField[0]  = MBENC_VME_INTER_PRED_BWD_PIC_IDX0_0_CM;
    bindingTable->dwAvcMBEncFwdPicTopField[1]  = MBENC_VME_INTER_PRED_FWD_PIC_IDX1_CM;
    bindingTable->dwAvcMBEncBwdPicTopField[1]  = MBENC_VME_INTER_PRED_BWD_PIC_IDX1_0_CM;
    bindingTable->dwAvcMBEncFwdPicBotField[1]  = MBENC_VME_INTER_PRED_FWD_PIC_IDX1_CM;
    bindingTable->dwAvcMBEncBwdPicBotField[1]  = MBENC_VME_INTER_PRED_BWD_PIC_IDX1_0_CM;
    bindingTable->dwAvcMBEncFwdPicTopField[2]  = MBENC_VME_INTER_PRED_FWD_PIC_IDX2_CM;
    bindingTable->dwAvcMBEncFwdPicBotField[2]  = MBENC_VME_INTER_PRED_FWD_PIC_IDX2_CM;
    bindingTable->dwAvcMBEncFwdPicTopField[3]  = MBENC_VME_INTER_PRED_FWD_PIC_IDX3_CM;
    bindingTable->dwAvcMBEncFwdPicBotField[3]  = MBENC_VME_INTER_PRED_FWD_PIC_IDX3_CM;
    bindingTable->dwAvcMBEncFwdPicTopField[4]  = MBENC_VME_INTER_PRED_FWD_PIC_IDX4_CM;
    bindingTable->dwAvcMBEncFwdPicBotField[4]  = MBENC_VME_INTER_PRED_FWD_PIC_IDX4_CM;
    bindingTable->dwAvcMBEncFwdPicTopField[5]  = MBENC_VME_INTER_PRED_FWD_PIC_IDX5_CM;
    bindingTable->dwAvcMBEncFwdPicBotField[5]  = MBENC_VME_INTER_PRED_FWD_PIC_IDX5_CM;
    bindingTable->dwAvcMBEncFwdPicTopField[6]  = MBENC_VME_INTER_PRED_FWD_PIC_IDX6_CM;
    bindingTable->dwAvcMBEncFwdPicBotField[6]  = MBENC_VME_INTER_PRED_FWD_PIC_IDX6_CM;
    bindingTable->dwAvcMBEncFwdPicTopField[7]  = MBENC_VME_INTER_PRED_FWD_PIC_IDX7_CM;
    bindingTable->dwAvcMBEncFwdPicBotField[7]  = MBENC_VME_INTER_PRED_FWD_PIC_IDX7_CM;
    bindingTable->dwAvcMBEncFieldCurrPic[1]    = MBENC_VME_INTER_PRED_CURR_PIC_IDX_1_CM;
    bindingTable->dwAvcMBEncBwdPicTopField[2]  = MBENC_VME_INTER_PRED_BWD_PIC_IDX0_1_CM;
    bindingTable->dwAvcMBEncBwdPicBotField[2]  = MBENC_VME_INTER_PRED_BWD_PIC_IDX0_1_CM;
    bindingTable->dwAvcMBEncBwdPicTopField[3]  = MBENC_VME_INTER_PRED_BWD_PIC_IDX1_1_CM;
    bindingTable->dwAvcMBEncBwdPicBotField[3]  = MBENC_VME_INTER_PRED_BWD_PIC_IDX1_1_CM;

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG8::SetCurbeAvcWP(PCODECHAL_ENCODE_AVC_WP_CURBE_PARAMS params)
{
    MOS_STATUS                eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);

    auto seqParams = m_avcSeqParam;
    CODECHAL_ENCODE_ASSERT(seqParams->TargetUsage < NUM_TARGET_USAGE_MODES);

    WP_CURBE     cmd;
    MOS_ZeroMemory(&cmd, sizeof(WP_CURBE));
    // Weights[i][j][k][m] is interpreted as:
    //    i refers to reference picture list 0 or 1
    //    j refers to reference list entry 0-31;
    //    k refers to data for the luma (Y) component when it is 0, the Cb chroma component when it is 1 and the Cr chroma component when it is 2;
    //    m refers to weight when it is 0 and offset when it is 1
    //
    auto slcParams = m_avcSliceParams;
    cmd.DW0.DefaultWeight = slcParams->Weights[params->RefPicListIdx][params->WPIdx][0][0];
    cmd.DW0.DefaultOffset = slcParams->Weights[params->RefPicListIdx][params->WPIdx][0][1];

    cmd.DW49.Log2WeightDenom = slcParams->luma_log2_weight_denom;
    cmd.DW49.ROI_enabled = 0;

    cmd.DW50.InputSurface = WP_INPUT_REF_SURFACE;
    cmd.DW51.OutputSurface = WP_OUTPUT_SCALED_SURFACE;
    auto stateHeapInterface = m_hwInterface->GetRenderInterface()->m_stateHeapInterface;
    auto kernelState = pWPKernelState;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(kernelState->m_dshRegion.AddData(
        &cmd,
        kernelState->dwCurbeOffset,
        sizeof(cmd)));

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG8::SetCurbeAvcMbEnc(
        PCODECHAL_ENCODE_AVC_MBENC_CURBE_PARAMS params)
{
    MOS_STATUS                    eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pPicParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pSeqParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pSlcParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pdwBlockBasedSkipEn);

    auto picParams = params->pPicParams;
    auto seqParams = params->pSeqParams;
    auto slcParams = params->pSlcParams;

    CODECHAL_ENCODE_ASSERT(seqParams->TargetUsage < NUM_TARGET_USAGE_MODES);

    // set sliceQp to MAX_SLICE_QP for  MbEnc kernel, we can use it to verify whether QP is changed or not
    uint8_t sliceQp = (params->bUseMbEncAdvKernel && params->bBrcEnabled) ? CODECHAL_ENCODE_AVC_MAX_SLICE_QP : picParams->pic_init_qp_minus26 + 26 + slcParams->slice_qp_delta;
    bool framePicture = CodecHal_PictureIsFrame(picParams->CurrOriginalPic);
    bool bottomField = CodecHal_PictureIsBottomField(picParams->CurrOriginalPic);

    MBENC_CURBE_CM cmd;
    if (params->bMbEncIFrameDistEnabled)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
        &cmd,
        sizeof(MBENC_CURBE_CM),
        m_initMBEncCurbeCmIFrameDist,
        sizeof(MBENC_CURBE_CM)));
    }
    else
    {
        switch (m_pictureCodingType)
        {
        case I_TYPE:
        if (framePicture)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
            &cmd,
            sizeof(MBENC_CURBE_CM),
            m_initMBEncCurbeCmNormalIFrame,
            sizeof(MBENC_CURBE_CM)));
        }
        else
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
            &cmd,
            sizeof(MBENC_CURBE_CM),
            m_initMBEncCurbeCmNormalIField,
            sizeof(MBENC_CURBE_CM)));
        }
        break;

        case P_TYPE:
        if (framePicture)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
            &cmd,
            sizeof(MBENC_CURBE_CM),
            m_initMBEncCurbeCmNormalPFrame,
            sizeof(MBENC_CURBE_CM)));
        }
        else
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
            &cmd,
            sizeof(MBENC_CURBE_CM),
            m_initMBEncCurbeCmNormalPField,
            sizeof(MBENC_CURBE_CM)));
        }
        break;

        case B_TYPE:
        if (framePicture)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
            &cmd,
            sizeof(MBENC_CURBE_CM),
            m_initMBEncCurbeCmNormalBFrame,
            sizeof(MBENC_CURBE_CM)));
        }
        else
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
            &cmd,
            sizeof(MBENC_CURBE_CM),
            m_initMBEncCurbeCmNormalBField,
            sizeof(MBENC_CURBE_CM)));
        }
        break;

        default:
        CODECHAL_ENCODE_ASSERTMESSAGE("Invalid picture coding type.");
        eStatus = MOS_STATUS_UNKNOWN;
        return eStatus;
        }
    }

    // r1
    cmd.DW0.AdaptiveEn =
        cmd.DW37.AdaptiveEn = EnableAdaptiveSearch[seqParams->TargetUsage];
    cmd.DW0.T8x8FlagForInterEn =
        cmd.DW37.T8x8FlagForInterEn = picParams->transform_8x8_mode_flag;
    cmd.DW2.LenSP = MaxLenSP[seqParams->TargetUsage];
    cmd.DW38.LenSP = 0; // MBZ
    cmd.DW3.SrcAccess =
        cmd.DW3.RefAccess = framePicture ? 0 : 1;
    if (m_pictureCodingType != I_TYPE &&     bFTQEnable)
    {
        if(params->pAvcQCParams && params->pAvcQCParams->FTQOverride)
        {
        cmd.DW3.FTEnable = params->pAvcQCParams->FTQEnable;
        }
        else
        {
        if (m_pictureCodingType == P_TYPE)
        {
            cmd.DW3.FTEnable = FTQBasedSkip[seqParams->TargetUsage] & 0x01;
        }
        else // B_TYPE
        {
            cmd.DW3.FTEnable = (FTQBasedSkip[seqParams->TargetUsage] >> 1) & 0x01;
        }
        }
    }
    else
    {
        cmd.DW3.FTEnable = 0;
    }
    if (picParams->UserFlags.bDisableSubMBPartition)
    {
        cmd.DW3.SubMbPartMask = CODECHAL_ENCODE_AVC_DISABLE_4X4_SUB_MB_PARTITION | CODECHAL_ENCODE_AVC_DISABLE_4X8_SUB_MB_PARTITION | CODECHAL_ENCODE_AVC_DISABLE_8X4_SUB_MB_PARTITION;
    }
    cmd.DW2.PicWidth = params->wPicWidthInMb;
    cmd.DW4.PicHeightMinus1 = params->wFieldFrameHeightInMb - 1;
    cmd.DW4.EnableIntraCostScalingForStaticFrame = params->bStaticFrameDetectionEnabled;
    cmd.DW4.FieldParityFlag              = bottomField;
    cmd.DW4.bCurFldIDR                 = !framePicture && (picParams->bIdrPic || m_firstFieldIdrPic);
    cmd.DW4.ConstrainedIntraPredFlag = picParams->constrained_intra_pred_flag;
    cmd.DW4.HMEEnable = m_hmeEnabled;
    cmd.DW4.PictureType = m_pictureCodingType - 1;
    cmd.DW4.UseActualRefQPValue          = m_hmeEnabled && (MRDisableQPCheck[seqParams->TargetUsage] == 0);
    cmd.DW5.SliceMbHeight = params->usSliceHeight;
    cmd.DW7.IntraPartMask = picParams->transform_8x8_mode_flag ? 0 : 0x2;    // Disable 8x8 if flag is not set
    cmd.DW7.SrcFieldPolarity             = bottomField;

    uint8_t tableIdx;
    // r2
    if (params->bMbEncIFrameDistEnabled)
    {
        cmd.DW6.BatchBufferEnd = 0;
    }
    else
    {
        tableIdx = m_pictureCodingType - 1;
        eStatus = MOS_SecureMemcpy(&(cmd.ModeMvCost), 8 * sizeof(uint32_t), ModeMvCost_Cm[tableIdx][sliceQp], 8 * sizeof(uint32_t));
        if (eStatus != MOS_STATUS_SUCCESS)
        {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
        return eStatus;
        }

        if (m_pictureCodingType == I_TYPE &&     bOldModeCostEnable)
        {
        // Old intra mode cost needs to be used if bOldModeCostEnable is 1
        cmd.ModeMvCost.DW8.Value = OldIntraModeCost_Cm_Common[sliceQp];
        }
        else if (m_skipBiasAdjustmentEnable)
        {
        // Load different MvCost for P picture when SkipBiasAdjustment is enabled
        // No need to check for P picture as the flag is only enabled for P picture
        cmd.ModeMvCost.DW11.Value = MvCost_PSkipAdjustment_Cm_Common[sliceQp];
        }
    }

    if(params->pAvcQCParams && params->pAvcQCParams->FTQSkipThresholdLUTInput)
    {
        cmd.ModeMvCost.DW14.SICFwdTransCoeffThreshold_0 = params->pAvcQCParams->FTQSkipThresholdLUT[sliceQp];
        cmd.ModeMvCost.DW14.SICFwdTransCoeffThreshold_1 = params->pAvcQCParams->FTQSkipThresholdLUT[sliceQp];
        cmd.ModeMvCost.DW14.SICFwdTransCoeffThreshold_2 = params->pAvcQCParams->FTQSkipThresholdLUT[sliceQp];
        cmd.ModeMvCost.DW15.SICFwdTransCoeffThreshold_3 = params->pAvcQCParams->FTQSkipThresholdLUT[sliceQp];
        cmd.ModeMvCost.DW15.SICFwdTransCoeffThreshold_4 = params->pAvcQCParams->FTQSkipThresholdLUT[sliceQp];
        cmd.ModeMvCost.DW15.SICFwdTransCoeffThreshold_5 = params->pAvcQCParams->FTQSkipThresholdLUT[sliceQp];
        cmd.ModeMvCost.DW15.SICFwdTransCoeffThreshold_6 = params->pAvcQCParams->FTQSkipThresholdLUT[sliceQp];
    }

    // r3 & r4
    if (params->bMbEncIFrameDistEnabled)
    {
        cmd.SPDelta.DW31.IntraComputeType = 1;
    }
    else
    {
        tableIdx = (m_pictureCodingType == B_TYPE) ? 1 : 0;
        uint8_t meMethod =
            (m_pictureCodingType == B_TYPE) ? m_bMeMethodGeneric[seqParams->TargetUsage] : m_meMethodGeneric[seqParams->TargetUsage];
        eStatus = MOS_SecureMemcpy(&(cmd.SPDelta), 16 * sizeof(uint32_t), m_encodeSearchPath[tableIdx][meMethod], 16 * sizeof(uint32_t));
        if (eStatus != MOS_STATUS_SUCCESS)
        {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
        return eStatus;
        }
    }

    // r5
    if(m_pictureCodingType != I_TYPE && params->pAvcQCParams && params->pAvcQCParams->NonFTQSkipThresholdLUTInput)
    {
        cmd.DW32.SkipVal = (uint16_t) CalcSkipVal(cmd.DW3.BlockBasedSkipEnable, picParams->transform_8x8_mode_flag,
                                params->pAvcQCParams->NonFTQSkipThresholdLUT[sliceQp]);

    }
    else
    {
        if (m_pictureCodingType == P_TYPE)
        {
        cmd.DW32.SkipVal = SkipVal_P_Common
            [cmd.DW3.BlockBasedSkipEnable]
        [picParams->transform_8x8_mode_flag]
        [sliceQp];
        }
        else if (m_pictureCodingType == B_TYPE)
        {
        cmd.DW32.SkipVal = SkipVal_B_Common
            [cmd.DW3.BlockBasedSkipEnable]
        [picParams->transform_8x8_mode_flag]
        [sliceQp];
        }
    }

    cmd.ModeMvCost.DW13.QpPrimeY = sliceQp;
    // QpPrimeCb and QpPrimeCr are not used by Kernel. Following settings are for CModel matching.
    cmd.ModeMvCost.DW13.QpPrimeCb = sliceQp;
    cmd.ModeMvCost.DW13.QpPrimeCr = sliceQp;
    cmd.ModeMvCost.DW13.TargetSizeInWord = 0xff; // hardcoded for BRC disabled

    if (bMultiPredEnable && (m_pictureCodingType != I_TYPE))
    {
        switch (MultiPred[seqParams->TargetUsage])
        {
        case 0: // Disable multipred for both P & B picture types
        cmd.DW32.MultiPredL0Disable = CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;
        cmd.DW32.MultiPredL1Disable = CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;
        break;

        case 1: // Enable multipred for P pictures only
        cmd.DW32.MultiPredL0Disable = (m_pictureCodingType == P_TYPE) ? CODECHAL_ENCODE_AVC_MULTIPRED_ENABLE : CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;
        cmd.DW32.MultiPredL1Disable = CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;
        break;

        case 2: // Enable multipred for B pictures only
        cmd.DW32.MultiPredL0Disable = (m_pictureCodingType == B_TYPE) ?
        CODECHAL_ENCODE_AVC_MULTIPRED_ENABLE : CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;
        cmd.DW32.MultiPredL1Disable = (m_pictureCodingType == B_TYPE) ?
        CODECHAL_ENCODE_AVC_MULTIPRED_ENABLE : CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;
        break;

        case 3: // Enable multipred for both P & B picture types
        cmd.DW32.MultiPredL0Disable = CODECHAL_ENCODE_AVC_MULTIPRED_ENABLE;
        cmd.DW32.MultiPredL1Disable = (m_pictureCodingType == B_TYPE) ?
        CODECHAL_ENCODE_AVC_MULTIPRED_ENABLE : CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;
        break;
        }
    }
    else
    {
        cmd.DW32.MultiPredL0Disable = CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;
        cmd.DW32.MultiPredL1Disable = CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;
    }

    if (!framePicture)
    {
        if (m_pictureCodingType != I_TYPE)
        {
        cmd.DW34.List0RefID0FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_0);
        cmd.DW34.List0RefID1FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_1);
        cmd.DW34.List0RefID2FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_2);
        cmd.DW34.List0RefID3FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_3);
        cmd.DW34.List0RefID4FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_4);
        cmd.DW34.List0RefID5FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_5);
        cmd.DW34.List0RefID6FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_6);
        cmd.DW34.List0RefID7FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_7);
        }
        if (m_pictureCodingType == B_TYPE)
        {
        cmd.DW34.List1RefID0FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_1, CODECHAL_ENCODE_REF_ID_0);
        cmd.DW34.List1RefID1FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_1, CODECHAL_ENCODE_REF_ID_1);
        }
    }

    if (m_pictureCodingType == B_TYPE)
    {
        cmd.DW34.List1RefID0FrameFieldFlag = GetRefPicFieldFlag(params, LIST_1, CODECHAL_ENCODE_REF_ID_0);
        cmd.DW34.List1RefID1FrameFieldFlag = GetRefPicFieldFlag(params, LIST_1, CODECHAL_ENCODE_REF_ID_1);
        cmd.DW34.bDirectMode = slcParams->direct_spatial_mv_pred_flag;
    }
    cmd.DW34.bOriginalBff = framePicture ? 0 :
        ((m_firstField && (bottomField)) || (!m_firstField && (!bottomField)));
    cmd.DW34.EnableMBFlatnessChkOptimization = m_flatnessCheckEnabled;
    cmd.DW34.ROIEnableFlag             = params->bRoiEnabled;
    cmd.DW34.MADEnableFlag                   = m_madEnabled;
    cmd.DW34.MBBrcEnable =     bMbBrcEnabled ||     bMbQpDataEnabled;
    cmd.DW34.ArbitraryNumMbsPerSlice = m_arbitraryNumMbsInSlice;
    cmd.DW34.ForceNonSkipMbEnable = params->bMbDisableSkipMapEnabled;
    if(params->pAvcQCParams && !cmd.DW34.ForceNonSkipMbEnable) // ignore DisableEncSkipCheck if Mb Disable Skip Map is available
    {
        cmd.DW34.DisableEncSkipCheck = params->pAvcQCParams->skipCheckDisable;
    }
    cmd.DW36.CheckAllFractionalEnable =     bCAFEnable;
    cmd.DW38.RefThreshold = m_refThreshold;
    cmd.DW39.HMERefWindowsCombThreshold = (m_pictureCodingType == B_TYPE) ?
        HMEBCombineLen[seqParams->TargetUsage] : HMECombineLen[seqParams->TargetUsage];

    // Default:2 used for MBBRC (MB QP Surface width and height are 4x downscaled picture in MB unit * 4  bytes)
    // 0 used for MBQP data surface (MB QP Surface width and height are same as the input picture size in MB unit * 1bytes)
    cmd.DW47.MbQpReadFactor =     bMbQpDataEnabled ? 0 : 2;

    // Those fields are not really used for I_dist kernel,
    // but set them to 0 to get bit-exact match with kernel prototype
    if (params->bMbEncIFrameDistEnabled)
    {
        cmd.ModeMvCost.DW13.QpPrimeY = 0;
        cmd.ModeMvCost.DW13.QpPrimeCb = 0;
        cmd.ModeMvCost.DW13.QpPrimeCr = 0;
        cmd.DW33.Intra16x16NonDCPredPenalty = 0;
        cmd.DW33.Intra4x4NonDCPredPenalty = 0;
        cmd.DW33.Intra8x8NonDCPredPenalty = 0;
    }

    //r6
    if (cmd.DW4.UseActualRefQPValue)
    {
        cmd.DW44.ActualQPValueForRefID0List0 = AVCGetQPValueFromRefList(params, LIST_0, CODECHAL_ENCODE_REF_ID_0);
        cmd.DW44.ActualQPValueForRefID1List0 = AVCGetQPValueFromRefList(params, LIST_0, CODECHAL_ENCODE_REF_ID_1);
        cmd.DW44.ActualQPValueForRefID2List0 = AVCGetQPValueFromRefList(params, LIST_0, CODECHAL_ENCODE_REF_ID_2);
        cmd.DW44.ActualQPValueForRefID3List0 = AVCGetQPValueFromRefList(params, LIST_0, CODECHAL_ENCODE_REF_ID_3);
        cmd.DW45.ActualQPValueForRefID4List0 = AVCGetQPValueFromRefList(params, LIST_0, CODECHAL_ENCODE_REF_ID_4);
        cmd.DW45.ActualQPValueForRefID5List0 = AVCGetQPValueFromRefList(params, LIST_0, CODECHAL_ENCODE_REF_ID_5);
        cmd.DW45.ActualQPValueForRefID6List0 = AVCGetQPValueFromRefList(params, LIST_0, CODECHAL_ENCODE_REF_ID_6);
        cmd.DW45.ActualQPValueForRefID7List0 = AVCGetQPValueFromRefList(params, LIST_0, CODECHAL_ENCODE_REF_ID_7);
        cmd.DW46.ActualQPValueForRefID0List1 = AVCGetQPValueFromRefList(params, LIST_1, CODECHAL_ENCODE_REF_ID_0);
        cmd.DW46.ActualQPValueForRefID1List1 = AVCGetQPValueFromRefList(params, LIST_1, CODECHAL_ENCODE_REF_ID_1);
    }

    tableIdx = m_pictureCodingType - 1;
    cmd.DW46.RefCost = RefCost_MultiRefQp[tableIdx][sliceQp];

    // Picture Coding Type dependent parameters
    if (m_pictureCodingType == I_TYPE)
    {
        cmd.DW0.SkipModeEn = 0;
        cmd.DW37.SkipModeEn = 0;
        cmd.DW36.HMECombineOverlap = 0;
        cmd.DW47.IntraCostSF = 16; // This is not used but recommended to set this to 16 by Kernel team
        cmd.DW34.EnableDirectBiasAdjustment = 0;
        cmd.DW34.EnableGlobalMotionBiasAdjustment = 0;
    }
    else if (m_pictureCodingType == P_TYPE)
    {
        cmd.DW1.MaxNumMVs = GetMaxMvsPer2Mb(seqParams->Level) / 2;
        cmd.DW3.BMEDisableFBR = 1;
        cmd.DW5.RefWidth = SearchX[seqParams->TargetUsage];
        cmd.DW5.RefHeight = SearchY[seqParams->TargetUsage];
        cmd.DW7.NonSkipZMvAdded = 1;
        cmd.DW7.NonSkipModeAdded = 1;
        cmd.DW7.SkipCenterMask = 1;
        cmd.DW47.IntraCostSF =
            bAdaptiveIntraScalingEnable ?
        AdaptiveIntraScalingFactor_Cm_Common[sliceQp] :
        IntraScalingFactor_Cm_Common[sliceQp];
        cmd.DW47.MaxVmvR = (framePicture) ? CodecHalAvcEncode_GetMaxMvLen(seqParams->Level) * 4 : (CodecHalAvcEncode_GetMaxMvLen(seqParams->Level) >> 1) * 4;
        cmd.DW36.HMECombineOverlap = 1;
        cmd.DW36.NumRefIdxL0MinusOne =     bMultiPredEnable ? slcParams->num_ref_idx_l0_active_minus1 : 0;
        cmd.DW39.RefWidth = SearchX[seqParams->TargetUsage];
        cmd.DW39.RefHeight = SearchY[seqParams->TargetUsage];
        cmd.DW34.EnableDirectBiasAdjustment = 0;
        if(params->pAvcQCParams)
        {
        cmd.DW34.EnableGlobalMotionBiasAdjustment = params->pAvcQCParams->globalMotionBiasAdjustmentEnable;
        if(cmd.DW34.EnableGlobalMotionBiasAdjustment)
        {
            cmd.DW58.HMEMVCostScalingFactor = params->pAvcQCParams->HMEMVCostScalingFactor;
        }
        }
    }
    else
    {
        // B_TYPE
        cmd.DW1.MaxNumMVs = GetMaxMvsPer2Mb(seqParams->Level) / 2;
        cmd.DW1.BiWeight = m_biWeight;
        cmd.DW3.SearchCtrl = 7;
        cmd.DW3.SkipType = 1;
        cmd.DW5.RefWidth = BSearchX[seqParams->TargetUsage];
        cmd.DW5.RefHeight = BSearchY[seqParams->TargetUsage];
        cmd.DW7.SkipCenterMask = 0xFF;
        cmd.DW47.IntraCostSF =
            bAdaptiveIntraScalingEnable ?
        AdaptiveIntraScalingFactor_Cm_Common[sliceQp] :
        IntraScalingFactor_Cm_Common[sliceQp];
        cmd.DW47.MaxVmvR = (framePicture) ? CodecHalAvcEncode_GetMaxMvLen(seqParams->Level) * 4 : (CodecHalAvcEncode_GetMaxMvLen(seqParams->Level) >> 1) * 4;
        cmd.DW36.HMECombineOverlap = 1;
        // Checking if the forward frame (List 1 index 0) is a short term reference
        {
        CODEC_PICTURE codecHalPic = params->pSlcParams->RefPicList[LIST_1][0];
        if (codecHalPic.PicFlags != PICTURE_INVALID &&
            codecHalPic.FrameIdx != CODECHAL_ENCODE_AVC_INVALID_PIC_ID &&
            params->pPicIdx[codecHalPic.FrameIdx].bValid)
        {
            // Although its name is FWD, it actually means the future frame or the backward reference frame
            cmd.DW36.IsFwdFrameShortTermRef = CodecHal_PictureIsShortTermRef(params->pPicParams->RefFrameList[codecHalPic.FrameIdx]);
        }
        else
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Invalid backward reference frame.");
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            return eStatus;
        }
        }
        cmd.DW36.NumRefIdxL0MinusOne =     bMultiPredEnable ? slcParams->num_ref_idx_l0_active_minus1 : 0;
        cmd.DW36.NumRefIdxL1MinusOne =     bMultiPredEnable ? slcParams->num_ref_idx_l1_active_minus1 : 0;
        cmd.DW39.RefWidth = BSearchX[seqParams->TargetUsage];
        cmd.DW39.RefHeight = BSearchY[seqParams->TargetUsage];
        cmd.DW40.DistScaleFactorRefID0List0 = m_distScaleFactorList0[0];
        cmd.DW40.DistScaleFactorRefID1List0 = m_distScaleFactorList0[1];
        cmd.DW41.DistScaleFactorRefID2List0 = m_distScaleFactorList0[2];
        cmd.DW41.DistScaleFactorRefID3List0 = m_distScaleFactorList0[3];
        cmd.DW42.DistScaleFactorRefID4List0 = m_distScaleFactorList0[4];
        cmd.DW42.DistScaleFactorRefID5List0 = m_distScaleFactorList0[5];
        cmd.DW43.DistScaleFactorRefID6List0 = m_distScaleFactorList0[6];
        cmd.DW43.DistScaleFactorRefID7List0 = m_distScaleFactorList0[7];

        if(params->pAvcQCParams)
        {
        cmd.DW34.EnableDirectBiasAdjustment     = params->pAvcQCParams->directBiasAdjustmentEnable;
        if(cmd.DW34.EnableDirectBiasAdjustment)
        {
            cmd.DW7.NonSkipModeAdded = 1;
            cmd.DW7.NonSkipZMvAdded  = 1;
        }

        cmd.DW34.EnableGlobalMotionBiasAdjustment    = params->pAvcQCParams->globalMotionBiasAdjustmentEnable;
        if(cmd.DW34.EnableGlobalMotionBiasAdjustment)
        {
            cmd.DW58.HMEMVCostScalingFactor = params->pAvcQCParams->HMEMVCostScalingFactor;
        }
        }
        {
        CODEC_PICTURE     refPic;
        refPic = slcParams->RefPicList[LIST_1][0];
        cmd.DW64.L1ListRef0PictureCodingType = m_refList[m_picIdx[refPic.FrameIdx].ucPicIdx]->ucAvcPictureCodingType;
        if(framePicture && ((cmd.DW64.L1ListRef0PictureCodingType == CODEC_AVC_PIC_CODING_TYPE_TFF_FIELD) || (cmd.DW64.L1ListRef0PictureCodingType == CODEC_AVC_PIC_CODING_TYPE_BFF_FIELD)))
        {
            uint16_t fieldHeightInMb = (params->wFieldFrameHeightInMb + 1) >> 1;
            cmd.DW69.BottomFieldOffsetL1ListRef0MV       = MOS_ALIGN_CEIL(fieldHeightInMb * params->wPicWidthInMb * (32 * 4), 0x1000);
            cmd.DW70.BottomFieldOffsetL1ListRef0MBCode = fieldHeightInMb * params->wPicWidthInMb * 64;
        }
        }
    }

    *params->pdwBlockBasedSkipEn = cmd.DW3.BlockBasedSkipEnable;

    if (ROLLING_I_SQUARE == picParams->EnableRollingIntraRefresh && !params->bSquareRollingIEnabled)
    {
        picParams->EnableRollingIntraRefresh = false;
    }

    if (picParams->EnableRollingIntraRefresh)
    {
        /* Multiple predictor should be completely disabled for the RollingI feature. This does not lead to much quality drop for P frames especially for TU as 1 */
        cmd.DW32.MultiPredL0Disable = CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;

        /* Pass the same IntraRefreshUnit to the kernel w/o the adjustment by -1, so as to have an overlap of one MB row or column of Intra macroblocks
        across one P frame to another P frame, as needed by the RollingI algo */
        if (ROLLING_I_SQUARE == picParams->EnableRollingIntraRefresh && RATECONTROL_CQP != seqParams->RateControlMethod)
        {
        /*BRC update kernel updates these CURBE to MBEnc*/
        cmd.DW4.EnableIntraRefresh          = false;
        cmd.DW34.IntraRefreshEn             = ROLLING_I_DISABLED;
        cmd.DW48.IntraRefreshMBx            = 0; /* MB column number */
        cmd.DW58.IntraRefreshMBy            = 0; /* MB row number */
        }
        else
        {
        cmd.DW4.EnableIntraRefresh          = true;
        cmd.DW34.IntraRefreshEn             = picParams->EnableRollingIntraRefresh;
        cmd.DW48.IntraRefreshMBx            = picParams->IntraRefreshMBx; /* MB column number */
        cmd.DW58.IntraRefreshMBy            = picParams->IntraRefreshMBy; /* MB row number */
        }
        cmd.DW48.IntraRefreshUnitInMBMinus1 = picParams->IntraRefreshUnitinMB;
        cmd.DW48.IntraRefreshQPDelta        = picParams->IntraRefreshQPDelta;
    }
    else
    {
        cmd.DW34.IntraRefreshEn             = 0;
    }

    if (true == params->bRoiEnabled)
    {
        cmd.DW49.ROI1_X_left = picParams->ROI[0].Left;
        cmd.DW49.ROI1_Y_top = picParams->ROI[0].Top;
        cmd.DW50.ROI1_X_right = picParams->ROI[0].Right;
        cmd.DW50.ROI1_Y_bottom = picParams->ROI[0].Bottom;

        cmd.DW51.ROI2_X_left = picParams->ROI[1].Left;
        cmd.DW51.ROI2_Y_top = picParams->ROI[1].Top;
        cmd.DW52.ROI2_X_right = picParams->ROI[1].Right;
        cmd.DW52.ROI2_Y_bottom = picParams->ROI[1].Bottom;

        cmd.DW53.ROI3_X_left = picParams->ROI[2].Left;
        cmd.DW53.ROI3_Y_top = picParams->ROI[2].Top;
        cmd.DW54.ROI3_X_right = picParams->ROI[2].Right;
        cmd.DW54.ROI3_Y_bottom = picParams->ROI[2].Bottom;

        cmd.DW55.ROI4_X_left = picParams->ROI[3].Left;
        cmd.DW55.ROI4_Y_top = picParams->ROI[3].Top;
        cmd.DW56.ROI4_X_right = picParams->ROI[3].Right;
        cmd.DW56.ROI4_Y_bottom = picParams->ROI[3].Bottom;

        if (    bBrcEnabled == false)
        {
        uint16_t numROI = picParams->NumROI;
        char priorityLevelOrDQp[CODECHAL_ENCODE_AVC_MAX_ROI_NUMBER] = { 0 };

        // cqp case
        for (uint32_t i = 0; i < numROI; i += 1)
        {
            char dQpRoi = picParams->ROI[i].PriorityLevelOrDQp;

            // clip qp roi in order to have (qp + qpY) in range [0, 51]
            priorityLevelOrDQp[i] = (char)CodecHal_Clip3(-sliceQp, CODECHAL_ENCODE_AVC_MAX_SLICE_QP - sliceQp, dQpRoi);
        }

        cmd.DW57.ROI1_dQpPrimeY = priorityLevelOrDQp[0];
        cmd.DW57.ROI2_dQpPrimeY = priorityLevelOrDQp[1];
        cmd.DW57.ROI3_dQpPrimeY = priorityLevelOrDQp[2];
        cmd.DW57.ROI4_dQpPrimeY = priorityLevelOrDQp[3];
        }
        else
        {
        // kernel does not support BRC case
        cmd.DW34.ROIEnableFlag = 0;
        }
    }
    else if( params->bDirtyRoiEnabled )
    {
        // enable  Dirty Rect flag
        cmd.DW4.EnableDirtyRect = true;

        cmd.DW49.ROI1_X_left      = params->pPicParams->DirtyROI[0].Left;
        cmd.DW49.ROI1_Y_top       = params->pPicParams->DirtyROI[0].Top;
        cmd.DW50.ROI1_X_right      = params->pPicParams->DirtyROI[0].Right;
        cmd.DW50.ROI1_Y_bottom      = params->pPicParams->DirtyROI[0].Bottom;

        cmd.DW51.ROI2_X_left      = params->pPicParams->DirtyROI[1].Left;
        cmd.DW51.ROI2_Y_top       = params->pPicParams->DirtyROI[1].Top;
        cmd.DW52.ROI2_X_right      = params->pPicParams->DirtyROI[1].Right;
        cmd.DW52.ROI2_Y_bottom      = params->pPicParams->DirtyROI[1].Bottom;

        cmd.DW53.ROI3_X_left      = params->pPicParams->DirtyROI[2].Left;
        cmd.DW53.ROI3_Y_top       = params->pPicParams->DirtyROI[2].Top;
        cmd.DW54.ROI3_X_right      = params->pPicParams->DirtyROI[2].Right;
        cmd.DW54.ROI3_Y_bottom      = params->pPicParams->DirtyROI[2].Bottom;

        cmd.DW55.ROI4_X_left      = params->pPicParams->DirtyROI[3].Left;
        cmd.DW55.ROI4_Y_top       = params->pPicParams->DirtyROI[3].Top;
        cmd.DW56.ROI4_X_right      = params->pPicParams->DirtyROI[3].Right;
        cmd.DW56.ROI4_Y_bottom      = params->pPicParams->DirtyROI[3].Bottom;
    }

    //IPCM QP and threshold
    cmd.DW65.IPCM_QP0            = initIpcmThresholdTable[0].QP;
    cmd.DW65.IPCM_QP1            = initIpcmThresholdTable[1].QP;
    cmd.DW65.IPCM_QP2            = initIpcmThresholdTable[2].QP;
    cmd.DW65.IPCM_QP3            = initIpcmThresholdTable[3].QP;
    cmd.DW66.IPCM_QP4            = initIpcmThresholdTable[4].QP;
    cmd.DW66.IPCM_Thresh0            = initIpcmThresholdTable[0].Threshold;
    cmd.DW67.IPCM_Thresh1            = initIpcmThresholdTable[1].Threshold;
    cmd.DW67.IPCM_Thresh2            = initIpcmThresholdTable[2].Threshold;
    cmd.DW68.IPCM_Thresh3            = initIpcmThresholdTable[3].Threshold;
    cmd.DW68.IPCM_Thresh4            = initIpcmThresholdTable[4].Threshold;

    cmd.DW73.MBDataSurfIndex        = MBENC_MFC_AVC_PAK_OBJ_CM;
    cmd.DW74.MVDataSurfIndex        = MBENC_IND_MV_DATA_CM;
    cmd.DW75.IDistSurfIndex         = MBENC_BRC_DISTORTION_CM;
    cmd.DW76.SrcYSurfIndex            = MBENC_CURR_Y_CM;
    cmd.DW77.MBSpecificDataSurfIndex    = MBENC_MB_SPECIFIC_DATA_CM;
    cmd.DW78.AuxVmeOutSurfIndex         = MBENC_AUX_VME_OUT_CM;
    cmd.DW79.CurrRefPicSelSurfIndex     = MBENC_REFPICSELECT_L0_CM;
    cmd.DW80.HMEMVPredFwdBwdSurfIndex    = MBENC_MV_DATA_FROM_ME_CM;
    cmd.DW81.HMEDistSurfIndex        = MBENC_4xME_DISTORTION_CM;
    // NNDOTO: DW68 works for SKL too as recpicselectl0 and slicemap both have BTI 10,
    // this comment can be removed once SKL kernels are upto date with all BDW kernel features
    cmd.DW82.SliceMapSurfIndex        = MBENC_SLICEMAP_DATA_CM;
    cmd.DW83.FwdFrmMBDataSurfIndex        = MBENC_FWD_MB_DATA_CM;
    cmd.DW84.FwdFrmMVSurfIndex        = MBENC_FWD_MV_DATA_CM;
    cmd.DW85.MBQPBuffer             = MBENC_MBQP_CM;
    cmd.DW86.MBBRCLut            = MBENC_MBBRC_CONST_DATA_CM;
    cmd.DW87.VMEInterPredictionSurfIndex    = MBENC_VME_INTER_PRED_CURR_PIC_IDX_0_CM;
    cmd.DW88.VMEInterPredictionMRSurfIndex    = MBENC_VME_INTER_PRED_CURR_PIC_IDX_1_CM;
    cmd.DW89.FlatnessChkSurfIndex        = MBENC_FLATNESS_CHECK_CM;
    cmd.DW90.MADSurfIndex            = MBENC_MAD_DATA_CM;
    cmd.DW91.ForceNonSkipMBmapSurface    = MBENC_FORCE_NONSKIP_MB_MAP_CM;
    cmd.DW92.ReservedIndex        = MBENC_ADV_WA_DATA_CM;
    cmd.DW93.BRCCurbeSurfIndex        = MBENC_BRC_CURBE_DATA_CM;
    cmd.DW94.StaticDetectionOutputBufferIndex = MBENC_STATIC_FRAME_DETECTION_OUTPUT_CM;

    auto stateHeapInterface =     m_hwInterface->GetRenderInterface()->m_stateHeapInterface;
    CODECHAL_ENCODE_CHK_NULL_RETURN(stateHeapInterface);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(params->pKernelState->m_dshRegion.AddData(
        &cmd,
        params->pKernelState->dwCurbeOffset,
        sizeof(cmd)));

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG8::SetCurbeAvcBrcInitReset(PCODECHAL_ENCODE_AVC_BRC_INIT_RESET_CURBE_PARAMS params)
{

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    CODECHAL_ENCODE_FUNCTION_ENTER;
    CODECHAL_ENCODE_CHK_NULL_RETURN(params);

    auto picParams          = m_avcPicParam;
    auto seqParams          = m_avcSeqParam;
    auto vuiParams          = m_avcVuiParams;

    BRC_INIT_RESET_CURBE cmd = initBrcInitResetCurbe;

    uint32_t  profileLevelMaxFrame;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalAvcEncode_GetProfileLevelMaxFrameSize(
        seqParams,
        this,
        &profileLevelMaxFrame));
    cmd.DW0.ProfileLevelMaxFrame = profileLevelMaxFrame;
    cmd.DW1.InitBufFullInBits    = seqParams->InitVBVBufferFullnessInBit;
    cmd.DW2.BufSizeInBits        = seqParams->VBVBufferSizeInBit;
    cmd.DW3.AverageBitRate       = seqParams->TargetBitRate;
    cmd.DW4.MaxBitRate           = seqParams->MaxBitRate;
    cmd.DW8.GopP                 =
        (seqParams->GopRefDist) ? ((seqParams->GopPicSize - 1) / seqParams->GopRefDist) : 0;
    cmd.DW9.GopB                 = seqParams->GopPicSize - 1 - cmd.DW8.GopP;
    cmd.DW9.FrameWidthInBytes    = m_frameWidth;
    cmd.DW10.FrameHeightInBytes  = m_frameHeight;
    cmd.DW12.NoSlices            = m_numSlices;

    // if VUI present, VUI data has high priority
    if (seqParams->vui_parameters_present_flag && seqParams->RateControlMethod != RATECONTROL_AVBR)
    {
        cmd.DW4.MaxBitRate =
            ((vuiParams->bit_rate_value_minus1[0] + 1) << (6 + vuiParams->bit_rate_scale));

        if (seqParams->RateControlMethod == RATECONTROL_CBR)
        {
            cmd.DW3.AverageBitRate = cmd.DW4.MaxBitRate;
        }
    }

    cmd.DW6.FrameRateM = seqParams->FramesPer100Sec;
    cmd.DW7.FrameRateD = 100;
    cmd.DW8.BRCFlag    = (CodecHal_PictureIsFrame(m_currOriginalPic)) ? 0 : CODECHAL_ENCODE_BRCINIT_FIELD_PIC;
    // MBBRC should be skipped when BRC ROI is on
    cmd.DW8.BRCFlag   |= (    bMbBrcEnabled && !    bBrcRoiEnabled) ? 0 : CODECHAL_ENCODE_BRCINIT_DISABLE_MBBRC;

    if (seqParams->RateControlMethod == RATECONTROL_CBR)
    {
        cmd.DW4.MaxBitRate = cmd.DW3.AverageBitRate;
        cmd.DW8.BRCFlag = cmd.DW8.BRCFlag | CODECHAL_ENCODE_BRCINIT_ISCBR;
    }
    else if (seqParams->RateControlMethod == RATECONTROL_VBR)
    {
        if (cmd.DW4.MaxBitRate < cmd.DW3.AverageBitRate)
        {
            cmd.DW3.AverageBitRate = cmd.DW4.MaxBitRate; // Use max bit rate for HRD compliance
        }
        cmd.DW8.BRCFlag = cmd.DW8.BRCFlag | CODECHAL_ENCODE_BRCINIT_ISVBR;
    }
    else if (seqParams->RateControlMethod == RATECONTROL_AVBR)
    {
        cmd.DW8.BRCFlag = cmd.DW8.BRCFlag | CODECHAL_ENCODE_BRCINIT_ISAVBR;
        // For AVBR, max bitrate = target bitrate,
        cmd.DW4.MaxBitRate = cmd.DW3.AverageBitRate;
    }
    else if (seqParams->RateControlMethod == RATECONTROL_ICQ)
    {
        cmd.DW8.BRCFlag = cmd.DW8.BRCFlag | CODECHAL_ENCODE_BRCINIT_ISICQ;
        cmd.DW23.ACQP = seqParams->ICQQualityFactor;
    }
    else if (seqParams->RateControlMethod == RATECONTROL_VCM)
    {
        cmd.DW8.BRCFlag = cmd.DW8.BRCFlag | CODECHAL_ENCODE_BRCINIT_ISVCM;
    }
    else if (seqParams->RateControlMethod == RATECONTROL_QVBR)
    {
        if (cmd.DW4.MaxBitRate < cmd.DW3.AverageBitRate)
        {
            cmd.DW3.AverageBitRate = cmd.DW4.MaxBitRate; // Use max bit rate for HRD compliance
        }
        cmd.DW8.BRCFlag = cmd.DW8.BRCFlag | CODECHAL_ENCODE_BRCINIT_ISQVBR;
        // use ICQQualityFactor to determine the larger Qp for each MB
        cmd.DW23.ACQP = seqParams->ICQQualityFactor;
    }

    cmd.DW10.AVBRAccuracy    =     usAVBRAccuracy;
    cmd.DW11.AVBRConvergence =     usAVBRConvergence;

    // Set dynamic thresholds
    double inputBitsPerFrame =
        ((double)(cmd.DW4.MaxBitRate) * (double)(cmd.DW7.FrameRateD) /
        (double)(cmd.DW6.FrameRateM));
    if (CodecHal_PictureIsField(m_currOriginalPic))
    {
        inputBitsPerFrame *= 0.5;
    }

    if (cmd.DW2.BufSizeInBits == 0)
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

    if (seqParams->RateControlMethod == RATECONTROL_AVBR)
    {
        // For AVBR, Buffer size =  2*Bitrate, InitVBV = 0.75 * BufferSize
        cmd.DW2.BufSizeInBits     = 2 * seqParams->TargetBitRate;
        cmd.DW1.InitBufFullInBits = (uint32_t)(0.75 * cmd.DW2.BufSizeInBits);
    }

    double bpsRatio = inputBitsPerFrame / ((double)(cmd.DW2.BufSizeInBits) / 30);
    bpsRatio = (bpsRatio < 0.1) ? 0.1 : (bpsRatio > 3.5) ? 3.5 : bpsRatio;

    cmd.DW16.DeviationThreshold0ForPandB = (uint32_t)(-50 * pow(0.90, bpsRatio));
    cmd.DW16.DeviationThreshold1ForPandB = (uint32_t)(-50 * pow(0.66, bpsRatio));
    cmd.DW16.DeviationThreshold2ForPandB = (uint32_t)(-50 * pow(0.46, bpsRatio));
    cmd.DW16.DeviationThreshold3ForPandB = (uint32_t)(-50 * pow(0.3, bpsRatio));
    cmd.DW17.DeviationThreshold4ForPandB = (uint32_t)(50 * pow(0.3, bpsRatio));
    cmd.DW17.DeviationThreshold5ForPandB = (uint32_t)(50 * pow(0.46, bpsRatio));
    cmd.DW17.DeviationThreshold6ForPandB = (uint32_t)(50 * pow(0.7, bpsRatio));
    cmd.DW17.DeviationThreshold7ForPandB = (uint32_t)(50 * pow(0.9, bpsRatio));
    cmd.DW18.DeviationThreshold0ForVBR   = (uint32_t)(-50 * pow(0.9, bpsRatio));
    cmd.DW18.DeviationThreshold1ForVBR   = (uint32_t)(-50 * pow(0.7, bpsRatio));
    cmd.DW18.DeviationThreshold2ForVBR   = (uint32_t)(-50 * pow(0.5, bpsRatio));
    cmd.DW18.DeviationThreshold3ForVBR   = (uint32_t)(-50 * pow(0.3, bpsRatio));
    cmd.DW19.DeviationThreshold4ForVBR   = (uint32_t)(100 * pow(0.4, bpsRatio));
    cmd.DW19.DeviationThreshold5ForVBR   = (uint32_t)(100 * pow(0.5, bpsRatio));
    cmd.DW19.DeviationThreshold6ForVBR   = (uint32_t)(100 * pow(0.75, bpsRatio));
    cmd.DW19.DeviationThreshold7ForVBR   = (uint32_t)(100 * pow(0.9, bpsRatio));
    cmd.DW20.DeviationThreshold0ForI     = (uint32_t)(-50 * pow(0.8, bpsRatio));
    cmd.DW20.DeviationThreshold1ForI     = (uint32_t)(-50 * pow(0.6, bpsRatio));
    cmd.DW20.DeviationThreshold2ForI     = (uint32_t)(-50 * pow(0.34, bpsRatio));
    cmd.DW20.DeviationThreshold3ForI     = (uint32_t)(-50 * pow(0.2, bpsRatio));
    cmd.DW21.DeviationThreshold4ForI     = (uint32_t)(50 * pow(0.2, bpsRatio));
    cmd.DW21.DeviationThreshold5ForI     = (uint32_t)(50 * pow(0.4, bpsRatio));
    cmd.DW21.DeviationThreshold6ForI     = (uint32_t)(50 * pow(0.66, bpsRatio));
    cmd.DW21.DeviationThreshold7ForI     = (uint32_t)(50 * pow(0.9, bpsRatio));

    cmd.DW22.SlidingWindowSize =     dwSlidingWindowSize;

    if (    bBrcInit)
    {
        *params->pdBrcInitCurrentTargetBufFullInBits = cmd.DW1.InitBufFullInBits;
    }

    *params->pdwBrcInitResetBufSizeInBits    = cmd.DW2.BufSizeInBits;
    *params->pdBrcInitResetInputBitsPerFrame = inputBitsPerFrame;
    auto stateHeapInterface = m_hwInterface->GetRenderInterface()->m_stateHeapInterface;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(params->pKernelState->m_dshRegion.AddData(
        &cmd,
        params->pKernelState->dwCurbeOffset,
        sizeof(cmd)));

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG8::SetCurbeAvcFrameBrcUpdate(PCODECHAL_ENCODE_AVC_BRC_UPDATE_CURBE_PARAMS params)
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pKernelState);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_avcPicParam);

    BRC_UPDATE_CURBE cmd = initBrcUpdateCurbe;

    cmd.DW5.TargetSizeFlag = 0;
    if (*params->pdBrcInitCurrentTargetBufFullInBits > (double)dwBrcInitResetBufSizeInBits)
    {
        *params->pdBrcInitCurrentTargetBufFullInBits -= (double)dwBrcInitResetBufSizeInBits;
        cmd.DW5.TargetSizeFlag = 1;
    }

    // skipped frame handling
    if (params->dwNumSkipFrames)
    {
        // pass num/size of skipped frames to update BRC
        cmd.DW6.NumSkipFrames = params->dwNumSkipFrames;
        cmd.DW7.SizeSkipFrames = params->dwSizeSkipFrames;

        // account for skipped frame in calculating CurrentTargetBufFullInBits
        *params->pdBrcInitCurrentTargetBufFullInBits += dBrcInitResetInputBitsPerFrame * params->dwNumSkipFrames;
    }

    cmd.DW0.TargetSize = (uint32_t)(*params->pdBrcInitCurrentTargetBufFullInBits);
    cmd.DW1.FrameNumber = m_storeData - 1;
    cmd.DW2.SizeofPicHeaders = m_headerBytesInserted << 3;   // kernel uses how many bits instead of bytes
    cmd.DW5.CurrFrameType =
        ((m_pictureCodingType - 2) < 0) ? 2 : (m_pictureCodingType - 2);
    cmd.DW5.BRCFlag =
        (CodecHal_PictureIsTopField(m_currOriginalPic)) ? brcUpdateIsField :
        ((CodecHal_PictureIsBottomField(m_currOriginalPic)) ? (brcUpdateIsField | brcUpdateIsBottomField) : 0);
    cmd.DW5.BRCFlag |= (m_refList[m_currReconstructedPic.FrameIdx]->bUsedAsRef) ?
        brcUpdateIsReference : 0;

    if (bMultiRefQpEnabled)
    {
        cmd.DW5.BRCFlag |= brcUpdateIsActualQp;
        cmd.DW14.QPIndexOfCurPic = m_currOriginalPic.FrameIdx;
    }

    cmd.DW5.MaxNumPAKs = m_hwInterface->GetMfxInterface()->GetBrcNumPakPasses();

    cmd.DW6.MinimumQP           = params->ucMinQP;
    cmd.DW6.MaximumQP           = params->ucMaxQP;
    cmd.DW6.EnableForceToSkip   = (bForceToSkipEnable && !m_avcPicParam->bDisableFrameSkip);
    auto seqParams = m_avcSeqParam;
    cmd.DW6.EnableSlidingWindow = (seqParams->FrameSizeTolerance == EFRAMESIZETOL_LOW);

    *params->pdBrcInitCurrentTargetBufFullInBits += dBrcInitResetInputBitsPerFrame;

    if (seqParams->RateControlMethod == RATECONTROL_AVBR)
    {
        cmd.DW3.startGAdjFrame0 = (uint32_t)((10 * usAVBRConvergence) / (double)150);
        cmd.DW3.startGAdjFrame1 = (uint32_t)((50 * usAVBRConvergence) / (double)150);
        cmd.DW4.startGAdjFrame2 = (uint32_t)((100 * usAVBRConvergence) / (double)150);
        cmd.DW4.startGAdjFrame3 = (uint32_t)((150 * usAVBRConvergence) / (double)150);
        cmd.DW11.gRateRatioThreshold0 =
            (uint32_t)((100 - (usAVBRAccuracy / (double)30)*(100 - 40)));
        cmd.DW11.gRateRatioThreshold1 =
            (uint32_t)((100 - (usAVBRAccuracy / (double)30)*(100 - 75)));
        cmd.DW12.gRateRatioThreshold2 = (uint32_t)((100 - (usAVBRAccuracy / (double)30)*(100 - 97)));
        cmd.DW12.gRateRatioThreshold3 = (uint32_t)((100 + (usAVBRAccuracy / (double)30)*(103 - 100)));
        cmd.DW12.gRateRatioThreshold4 = (uint32_t)((100 + (usAVBRAccuracy / (double)30)*(125 - 100)));
        cmd.DW12.gRateRatioThreshold5 = (uint32_t)((100 + (usAVBRAccuracy / (double)30)*(160 - 100)));
    }

    auto picParams = m_avcPicParam;
    //Rolling Intra Refresh
    //BRC controls only for square region rolling I, it does not control for others (Row and Column)
    if (ROLLING_I_SQUARE == picParams->EnableRollingIntraRefresh && params->bSquareRollingIEnabled)
    {
        cmd.DW15.IntraRefreshMode      = BRC_ROLLING_I_SQUARE;
        cmd.DW15.QPIntraRefresh        = params->dwIntraRefreshQpThreshold;

        //BRC kernel tracks x and y pos
        cmd.DW16.IntraRefreshXPos      = 0;
        cmd.DW16.IntraRefreshYPos      = 0;
        cmd.DW17.IntraRefreshHeight    = picParams->IntraRefreshUnitinMB;
        cmd.DW17.IntraRefreshWidth     = picParams->IntraRefreshUnitinMB;
    }
    else
    {
        cmd.DW15.IntraRefreshMode      = BRC_ROLLING_I_DISABLED;
    }
    uint32_t profileLevelMaxFrame;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalAvcEncode_GetProfileLevelMaxFrameSize(
        seqParams,
        this,
        &profileLevelMaxFrame));

    cmd.DW19.UserMaxFrame = profileLevelMaxFrame;

    auto stateHeapInterface =     m_hwInterface->GetRenderInterface()->m_stateHeapInterface;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(params->pKernelState->m_dshRegion.AddData(
        &cmd,
        params->pKernelState->dwCurbeOffset,
        sizeof(cmd)));

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG8::SendAvcBrcFrameUpdateSurfaces(PMOS_COMMAND_BUFFER cmdBuffer, PCODECHAL_ENCODE_AVC_BRC_UPDATE_SURFACE_PARAMS params)
{
    MOS_STATUS                                      eStatus = MOS_STATUS_SUCCESS;
    CODECHAL_ENCODE_FUNCTION_ENTER;

    m_stateHeapInterface = m_hwInterface->GetRenderInterface()->m_stateHeapInterface;

    auto avcBrcUpdateBindingTable = params->pBrcUpdateBindingTable;
    auto mbencKernelState = params->pBrcBuffers->pMbEncKernelStateInUse;
    auto kernelState = params->pKernelState;

    CODECHAL_SURFACE_CODEC_PARAMS                   surfaceCodecParams;
    // BRC history buffer
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.presBuffer = &params->pBrcBuffers->resBrcHistoryBuffer;
    surfaceCodecParams.dwSize = MOS_BYTES_TO_DWORDS(params->dwBrcHistoryBufferSize);
    surfaceCodecParams.dwBindingTableOffset = avcBrcUpdateBindingTable->dwFrameBrcHistoryBuffer;
    surfaceCodecParams.bIsWritable = true;
    surfaceCodecParams.bRenderTarget = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // PAK Statistics buffer
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.presBuffer = &params->pBrcBuffers->resBrcPakStatisticBuffer[0];
    surfaceCodecParams.dwSize = MOS_BYTES_TO_DWORDS(params->dwBrcPakStatisticsSize);
    surfaceCodecParams.dwBindingTableOffset = avcBrcUpdateBindingTable->dwFrameBrcPakStatisticsOutputBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // PAK IMG_STATEs buffer - read only
    uint32_t size = MOS_BYTES_TO_DWORDS(BRC_IMG_STATE_SIZE_PER_PASS * m_hwInterface->GetMfxInterface()->GetBrcNumPakPasses());
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.presBuffer =
        &params->pBrcBuffers->resBrcImageStatesReadBuffer[params->ucCurrRecycledBufIdx];
    surfaceCodecParams.dwSize = size;
    surfaceCodecParams.dwBindingTableOffset = avcBrcUpdateBindingTable->dwFrameBrcImageStateReadBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // PAK IMG_STATEs buffer - write only
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.presBuffer = &params->pBrcBuffers->resBrcImageStatesWriteBuffer;
    surfaceCodecParams.dwSize = size;
    surfaceCodecParams.dwBindingTableOffset = avcBrcUpdateBindingTable->dwFrameBrcImageStateWriteBuffer;
    surfaceCodecParams.bIsWritable = true;
    surfaceCodecParams.bRenderTarget = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    MOS_RESOURCE *dsh = nullptr;
    CODECHAL_ENCODE_CHK_NULL_RETURN(dsh = mbencKernelState->m_dshRegion.GetResource());

    // BRC ENC CURBE Buffer - read only
    size = MOS_ALIGN_CEIL(
        mbencKernelState->KernelParams.iCurbeLength,
        m_hwInterface->GetRenderInterface()->m_stateHeapInterface->pStateHeapInterface->GetCurbeAlignment());
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.presBuffer = dsh;
    surfaceCodecParams.dwOffset =
        mbencKernelState->m_dshRegion.GetOffset() +
        mbencKernelState->dwCurbeOffset;
    surfaceCodecParams.dwSize = MOS_BYTES_TO_DWORDS(size);
    surfaceCodecParams.dwBindingTableOffset = avcBrcUpdateBindingTable->dwFrameBrcMbEncCurbeReadBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // BRC ENC CURBE Buffer - write only
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    if (params->bUseAdvancedDsh)
    {
        surfaceCodecParams.presBuffer = params->presMbEncCurbeBuffer;
    }
    else
    {
        surfaceCodecParams.presBuffer = dsh;
        surfaceCodecParams.dwOffset =
            mbencKernelState->m_dshRegion.GetOffset() +
            mbencKernelState->dwCurbeOffset;
    }
    surfaceCodecParams.dwSize = MOS_BYTES_TO_DWORDS(size);
    surfaceCodecParams.dwBindingTableOffset = avcBrcUpdateBindingTable->dwFrameBrcMbEncCurbeWriteData;
    surfaceCodecParams.bRenderTarget = true;
    surfaceCodecParams.bIsWritable = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // AVC_ME BRC Distortion data buffer - input/output
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.bIs2DSurface = true;
    surfaceCodecParams.bMediaBlockRW = true;
    surfaceCodecParams.psSurface = &params->pBrcBuffers->sMeBrcDistortionBuffer;
    surfaceCodecParams.dwOffset = params->pBrcBuffers->dwMeBrcDistortionBottomFieldOffset;
    surfaceCodecParams.dwBindingTableOffset = avcBrcUpdateBindingTable->dwFrameBrcDistortionBuffer;
    surfaceCodecParams.bRenderTarget = true;
    surfaceCodecParams.bIsWritable = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // BRC Constant Data Surface
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.bIs2DSurface = true;
    surfaceCodecParams.bMediaBlockRW = true;
    surfaceCodecParams.psSurface =
        &params->pBrcBuffers->sBrcConstantDataBuffer[params->ucCurrRecycledBufIdx];
    surfaceCodecParams.dwBindingTableOffset = avcBrcUpdateBindingTable->dwFrameBrcConstantData;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // AVC MB QP data buffer
    if (params->bMbBrcEnabled)
    {
        params->pBrcBuffers->sBrcMbQpBuffer.dwWidth = MOS_ALIGN_CEIL((params->dwDownscaledWidthInMb4x * 4), 64);
        params->pBrcBuffers->sBrcMbQpBuffer.dwHeight =
            MOS_ALIGN_CEIL((params->dwDownscaledFrameFieldHeightInMb4x * 4), 8);
        params->pBrcBuffers->sBrcMbQpBuffer.dwPitch = params->pBrcBuffers->sBrcMbQpBuffer.dwWidth;

        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bIs2DSurface = true;
        surfaceCodecParams.bMediaBlockRW = true;
        surfaceCodecParams.psSurface = &params->pBrcBuffers->sBrcMbQpBuffer;
        surfaceCodecParams.dwOffset = params->pBrcBuffers->dwBrcMbQpBottomFieldOffset;
        surfaceCodecParams.dwBindingTableOffset = avcBrcUpdateBindingTable->dwMbBrcMbQpBuffer;
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

MOS_STATUS CodechalEncodeAvcEncG8::SetCurbeAvcBrcBlockCopy(PCODECHAL_ENCODE_AVC_BRC_BLOCK_COPY_CURBE_PARAMS params)
{
    MOS_STATUS                                        eStatus = MOS_STATUS_SUCCESS;
    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pKernelState);

    BRC_BLOCK_COPY_CURBE_CM cmd;
    MOS_ZeroMemory(&cmd, sizeof(cmd));

    cmd.DW0.BufferOffset    = params->dwBufferOffset;
    cmd.DW0.BlockHeight     = params->dwBlockHeight;
    cmd.DW1.SrcSurfaceIndex = 0x00;
    cmd.DW2.DstSurfaceIndex = 0x01;

    auto stateHeapInterface = m_hwInterface->GetRenderInterface()->m_stateHeapInterface;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(params->pKernelState->m_dshRegion.AddData(
        &cmd,
        params->pKernelState->dwCurbeOffset,
        sizeof(cmd)));

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG8::SendAvcMbEncSurfaces(PMOS_COMMAND_BUFFER cmdBuffer, PCODECHAL_ENCODE_AVC_MBENC_SURFACE_PARAMS params)
{
    MOS_STATUS                eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pAvcSlcParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->ppRefList);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pCurrOriginalPic);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pCurrReconstructedPic);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->psCurrPicSurface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pAvcPicIdx);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pMbEncBindingTable);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pKernelState);

    bool currBottomField = CodecHal_PictureIsBottomField(*(params->pCurrOriginalPic)) ? 1 : 0;

    uint8_t vDirection;

    if (params->bMbEncIFrameDistInUse)
    {
        vDirection = CODECHAL_VDIRECTION_FRAME;
    }
    else
    {
        vDirection = (CodecHal_PictureIsFrame(*(params->pCurrOriginalPic))) ? CODECHAL_VDIRECTION_FRAME :
        (currBottomField) ? CODECHAL_VDIRECTION_BOT_FIELD : CODECHAL_VDIRECTION_TOP_FIELD;
    }

    // PAK Obj command buffer

    CODECHAL_SURFACE_CODEC_PARAMS        surfaceCodecParams;
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    auto currPicRefListEntry = params->ppRefList[params->pCurrReconstructedPic->FrameIdx];
    auto mbCodeBuffer = &currPicRefListEntry->resRefMbCodeBuffer;
    auto avcMbEncBindingTable = params->pMbEncBindingTable;
    uint32_t size = params->dwFrameWidthInMb * params->dwFrameFieldHeightInMb * 16 * 4;    // 11DW + 5DW padding
    surfaceCodecParams.presBuffer = mbCodeBuffer;
    surfaceCodecParams.dwSize = size;
    surfaceCodecParams.dwOffset = params->dwMbCodeBottomFieldOffset;
    surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncMfcAvcPakObj;
    surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_PAK_OBJECT_ENCODE].Value;
    surfaceCodecParams.bRenderTarget = true;
    surfaceCodecParams.bIsWritable = true;
    auto kernelState = params->pKernelState;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // MV data buffer
    size = params->dwFrameWidthInMb * params->dwFrameFieldHeightInMb * 32 * 4;
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    auto mvDataBuffer = &currPicRefListEntry->resRefMvDataBuffer;
    surfaceCodecParams.presBuffer = mvDataBuffer;
    surfaceCodecParams.dwSize = size;
    surfaceCodecParams.dwOffset = params->dwMvBottomFieldOffset;
    surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value;
    surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncIndMVData;
    surfaceCodecParams.bRenderTarget = true;
    surfaceCodecParams.bIsWritable = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Current Picture Y
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.bIs2DSurface = true;
    surfaceCodecParams.bMediaBlockRW = true; // Use media block RW for DP 2D surface access
    surfaceCodecParams.bUseUVPlane = true;
    surfaceCodecParams.psSurface = params->psCurrPicSurface;
    surfaceCodecParams.dwOffset = params->dwCurrPicSurfaceOffset;
    surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value;
    surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncCurrY;
    surfaceCodecParams.dwUVBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncCurrUV;
    surfaceCodecParams.dwVerticalLineStride = params->dwVerticalLineStride;
    surfaceCodecParams.dwVerticalLineStrideOffset = params->dwVerticalLineStrideOffset;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // AVC_ME MV data buffer
    if (params->bHmeEnabled)
    {
        CODECHAL_ENCODE_CHK_NULL_RETURN(params->ps4xMeMvDataBuffer);

        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bIs2DSurface = true;
        surfaceCodecParams.bMediaBlockRW = true;
        surfaceCodecParams.psSurface = params->ps4xMeMvDataBuffer;
        surfaceCodecParams.dwOffset = params->dwMeMvBottomFieldOffset;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value;
        surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncMVDataFromME;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

        CODECHAL_ENCODE_CHK_NULL_RETURN(params->ps4xMeDistortionBuffer);

        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bIs2DSurface = true;
        surfaceCodecParams.bMediaBlockRW = true;
        surfaceCodecParams.psSurface = params->ps4xMeDistortionBuffer;
        surfaceCodecParams.dwOffset = params->dwMeDistortionBottomFieldOffset;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ME_DISTORTION_ENCODE].Value;
        surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncMEDist;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));
    }

    if (params->bMbConstDataBufferInUse)
    {
        // 16 DWs per QP value
        size = 16 * 52 * sizeof(uint32_t);

        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.presBuffer = params->presMbBrcConstDataBuffer;
        surfaceCodecParams.dwSize = size;
        surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncMbBrcConstData;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));
    }
    bool currFieldPicture = CodecHal_PictureIsField(*(params->pCurrOriginalPic)) ? 1 : 0;
    if (params->bMbQpBufferInUse)
    {
        // AVC MB BRC QP buffer
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bIs2DSurface = true;
        surfaceCodecParams.bMediaBlockRW = true;
        surfaceCodecParams.psSurface = params->psMbQpBuffer;
        surfaceCodecParams.dwOffset = params->dwMbQpBottomFieldOffset;
        surfaceCodecParams.dwBindingTableOffset = currFieldPicture ? avcMbEncBindingTable->dwAvcMBEncMbQpField :
        avcMbEncBindingTable->dwAvcMBEncMbQpFrame;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));
    }

    // Current Picture Y - VME
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.bUseAdvState = true;
    surfaceCodecParams.psSurface = params->psCurrPicSurface;
    surfaceCodecParams.dwOffset = params->dwCurrPicSurfaceOffset;
    surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value;
    surfaceCodecParams.dwBindingTableOffset = currFieldPicture ?
        avcMbEncBindingTable->dwAvcMBEncFieldCurrPic[0] : avcMbEncBindingTable->dwAvcMBEncCurrPicFrame[0];
    surfaceCodecParams.ucVDirection = vDirection;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    surfaceCodecParams.dwBindingTableOffset = currFieldPicture ?
        avcMbEncBindingTable->dwAvcMBEncFieldCurrPic[1] : avcMbEncBindingTable->dwAvcMBEncCurrPicFrame[1];
    surfaceCodecParams.ucVDirection = vDirection;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    uint32_t   refBindingTableOffset;
    uint8_t    refVDirection;
    // Setup references 1...n
    // LIST 0 references
    for (uint8_t refIdx = 0; refIdx <= params->pAvcSlcParams->num_ref_idx_l0_active_minus1; refIdx++)
    {
        CODEC_PICTURE refPic = params->pAvcSlcParams->RefPicList[LIST_0][refIdx];
        if (!CodecHal_PictureIsInvalid(refPic) && params->pAvcPicIdx[refPic.FrameIdx].bValid)
        {
        uint8_t refPicIdx = params->pAvcPicIdx[refPic.FrameIdx].ucPicIdx;
        bool refBottomField = (CodecHal_PictureIsBottomField(refPic)) ? 1 : 0;
        // Program the surface based on current picture's field/frame mode
        if (currFieldPicture) // if current picture is field
        {
            if (refBottomField)
            {
            refVDirection = CODECHAL_VDIRECTION_BOT_FIELD;
            refBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncFwdPicBotField[refIdx];
            }
            else
            {
            refVDirection = CODECHAL_VDIRECTION_TOP_FIELD;
            refBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncFwdPicTopField[refIdx];
            }
        }
        else // if current picture is frame
        {
            refVDirection = CODECHAL_VDIRECTION_FRAME;
            refBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncFwdPicFrame[refIdx];
        }

        // Picture Y VME
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bUseAdvState = true;
        surfaceCodecParams.dwWidthInUse = params->dwFrameWidthInMb * 16;
        surfaceCodecParams.dwHeightInUse = params->dwFrameHeightInMb * 16;
        if((params->bUseWeightedSurfaceForL0) &&
           (params->pAvcSlcParams->luma_weight_flag[LIST_0] & (1<<refIdx)) &&
           (refIdx < CODEC_AVC_MAX_FORWARD_WP_FRAME))
        {
            surfaceCodecParams.psSurface = &params->pWeightedPredOutputPicSelectList[CODEC_AVC_WP_OUTPUT_L0_START + refIdx].sBuffer;
        }
        else
        {
            surfaceCodecParams.psSurface = &params->ppRefList[refPicIdx]->sRefBuffer;
        }
        surfaceCodecParams.dwBindingTableOffset = refBindingTableOffset;
        surfaceCodecParams.ucVDirection = refVDirection;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
        }
    }
    // Setup references 1...n
    // LIST 1 references
    for (uint8_t refIdx = 0; refIdx <= params->pAvcSlcParams->num_ref_idx_l1_active_minus1; refIdx++)
    {
        uint32_t refMbCodeBottomFieldOffsetUsed;
        uint32_t refMvBottomFieldOffsetUsed;
        if (!currFieldPicture && refIdx > 0)
        {
        // Only 1 LIST 1 reference required here since only single ref is supported in frame case
        break;
        }

        CODEC_PICTURE refPic = params->pAvcSlcParams->RefPicList[LIST_1][refIdx];
        if (!CodecHal_PictureIsInvalid(refPic) && params->pAvcPicIdx[refPic.FrameIdx].bValid)
        {
        uint8_t refPicIdx = params->pAvcPicIdx[refPic.FrameIdx].ucPicIdx;
        bool refBottomField = (CodecHal_PictureIsBottomField(refPic)) ? 1 : 0;
        // Program the surface based on current picture's field/frame mode
        if (currFieldPicture) // if current picture is field
        {
            if (refBottomField)
            {
            refVDirection = CODECHAL_VDIRECTION_BOT_FIELD;
            uint32_t refMbCodeBottomFieldOffset =     params->dwFrameFieldHeightInMb * params->dwFrameWidthInMb * 64;
            refMbCodeBottomFieldOffsetUsed = refMbCodeBottomFieldOffset;
            uint32_t refMvBottomFieldOffset =     MOS_ALIGN_CEIL(params->dwFrameFieldHeightInMb * params->dwFrameWidthInMb * (32 * 4), 0x1000);
            refMvBottomFieldOffsetUsed = refMvBottomFieldOffset;
            refBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncBwdPicBotField[refIdx];
            }
            else
            {
            refVDirection = CODECHAL_VDIRECTION_TOP_FIELD;
            refMbCodeBottomFieldOffsetUsed = 0;
            refMvBottomFieldOffsetUsed = 0;
            refBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncBwdPicTopField[refIdx];
            }
        }
        else // if current picture is frame
        {
            refVDirection = CODECHAL_VDIRECTION_FRAME;
            refMbCodeBottomFieldOffsetUsed = 0;
            refMvBottomFieldOffsetUsed = 0;
            refBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncBwdPicFrame[refIdx];
        }

        // Picture Y VME
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bUseAdvState = true;
        surfaceCodecParams.dwWidthInUse = params->dwFrameWidthInMb * 16;
        surfaceCodecParams.dwHeightInUse = params->dwFrameHeightInMb * 16;
        if((params->bUseWeightedSurfaceForL1) &&
           (params->pAvcSlcParams->luma_weight_flag[LIST_1] & (1<<refIdx)) &&
           (refIdx < CODEC_AVC_MAX_BACKWARD_WP_FRAME))
        {
            surfaceCodecParams.psSurface = &params->pWeightedPredOutputPicSelectList[CODEC_AVC_WP_OUTPUT_L1_START + refIdx].sBuffer;
        }
        else
        {
            surfaceCodecParams.psSurface = &params->ppRefList[refPicIdx]->sRefBuffer;
        }
        surfaceCodecParams.dwBindingTableOffset = refBindingTableOffset;
        surfaceCodecParams.ucVDirection = refVDirection;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));

        if (refIdx == 0)
        {
            if(currFieldPicture && (params->ppRefList[refPicIdx]->ucAvcPictureCodingType == CODEC_AVC_PIC_CODING_TYPE_FRAME || params->ppRefList[refPicIdx]->ucAvcPictureCodingType == CODEC_AVC_PIC_CODING_TYPE_INVALID))
            {
            refMbCodeBottomFieldOffsetUsed = 0;
            refMvBottomFieldOffsetUsed     = 0;
            }
            // MB data buffer
            size = params->dwFrameWidthInMb * params->dwFrameFieldHeightInMb * 16 * 4;
            MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
            surfaceCodecParams.dwSize = size;
            surfaceCodecParams.presBuffer = &params->ppRefList[refPicIdx]->resRefMbCodeBuffer;
            surfaceCodecParams.dwOffset = refMbCodeBottomFieldOffsetUsed;
            surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_PAK_OBJECT_ENCODE].Value;
            surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncBwdRefMBData;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));

            // MV data buffer
            size = params->dwFrameWidthInMb * params->dwFrameFieldHeightInMb * 32 * 4;
            MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
            surfaceCodecParams.dwSize = size;
            surfaceCodecParams.presBuffer = &params->ppRefList[refPicIdx]->resRefMvDataBuffer;
            surfaceCodecParams.dwOffset = refMvBottomFieldOffsetUsed;
            surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value;
            surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncBwdRefMVData;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
        }

        if (refIdx < CODECHAL_ENCODE_NUM_MAX_VME_L1_REF)
        {
            if (currFieldPicture)
            {
            // The binding table contains multiple entries for IDX0 backwards references
            if (refBottomField)
            {
                refBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncBwdPicBotField[refIdx + CODECHAL_ENCODE_NUM_MAX_VME_L1_REF];
            }
            else
            {
                refBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncBwdPicTopField[refIdx + CODECHAL_ENCODE_NUM_MAX_VME_L1_REF];
            }
            }
            else
            {
            refBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncBwdPicFrame[refIdx + CODECHAL_ENCODE_NUM_MAX_VME_L1_REF];
            }

            // Picture Y VME
            MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
            surfaceCodecParams.bUseAdvState = true;
            surfaceCodecParams.dwWidthInUse = params->dwFrameWidthInMb * 16;
            surfaceCodecParams.dwHeightInUse = params->dwFrameHeightInMb * 16;
            surfaceCodecParams.psSurface = &params->ppRefList[refPicIdx]->sRefBuffer;
            surfaceCodecParams.dwBindingTableOffset = refBindingTableOffset;
            surfaceCodecParams.ucVDirection = refVDirection;
            surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value;

            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
        }
        }
    }

    // BRC distortion data buffer for I frame
    if (params->bMbEncIFrameDistInUse)
    {
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bIs2DSurface = true;
        surfaceCodecParams.bMediaBlockRW = true;
        surfaceCodecParams.psSurface = params->psMeBrcDistortionBuffer;
        surfaceCodecParams.dwOffset = params->dwMeBrcDistortionBottomFieldOffset;
        surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncBRCDist;
        surfaceCodecParams.bIsWritable = true;
        surfaceCodecParams.bRenderTarget = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));
    }

    // RefPicSelect of Current Picture
    if (params->bUsedAsRef)
    {
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bIs2DSurface = true;
        surfaceCodecParams.bMediaBlockRW = true;
        surfaceCodecParams.psSurface = &currPicRefListEntry->pRefPicSelectListEntry->sBuffer;
        surfaceCodecParams.psSurface->dwHeight = MOS_ALIGN_CEIL(surfaceCodecParams.psSurface->dwHeight, 8);
        surfaceCodecParams.dwOffset = params->dwRefPicSelectBottomFieldOffset;
        surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncRefPicSelectL0;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value;
        surfaceCodecParams.bRenderTarget = true;
        surfaceCodecParams.bIsWritable = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));
    }

    if (params->bFlatnessCheckEnabled)
    {
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bIs2DSurface = true;
        surfaceCodecParams.bMediaBlockRW = true;
        surfaceCodecParams.psSurface = params->psFlatnessCheckSurface;
        surfaceCodecParams.dwOffset = currBottomField ? params->dwFlatnessCheckBottomFieldOffset : 0;
        surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncFlatnessChk;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_FLATNESS_CHECK_ENCODE].Value;
        surfaceCodecParams.bRenderTarget = true;
        surfaceCodecParams.bIsWritable = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));
    }

    if (params->bMADEnabled)
    {
        size = CODECHAL_MAD_BUFFER_SIZE;

        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bRawSurface = true;
        surfaceCodecParams.dwSize = size;
        surfaceCodecParams.presBuffer = params->presMADDataBuffer;
        surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncMADData;
        surfaceCodecParams.bRenderTarget = true;
        surfaceCodecParams.bIsWritable = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));
    }

    if (params->bUseMbEncAdvKernel)
    {
        auto stateHeapInterface = m_hwInterface->GetRenderInterface()->m_stateHeapInterface;
        // For BRC the new BRC surface is used
        if (params->bUseAdvancedDsh)
        {
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.presBuffer = params->presMbEncCurbeBuffer;
        uint32_t curbeSize = MOS_ALIGN_CEIL(
            params->pKernelState->KernelParams.iCurbeLength,
            m_hwInterface->GetRenderInterface()->m_stateHeapInterface->pStateHeapInterface->GetCurbeAlignment());
        surfaceCodecParams.dwSize = MOS_BYTES_TO_DWORDS(curbeSize);
        surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMbEncBRCCurbeData;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MBENC_CURBE_ENCODE].Value;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
        }
        else // For CQP the DSH CURBE is used
        {
            MOS_RESOURCE *dsh = nullptr;
            CODECHAL_ENCODE_CHK_NULL_RETURN(dsh = params->pKernelState->m_dshRegion.GetResource());
            MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
            surfaceCodecParams.presBuffer = dsh;
            surfaceCodecParams.dwOffset =
                params->pKernelState->m_dshRegion.GetOffset() +
                params->pKernelState->dwCurbeOffset;
            uint32_t curbeSize = MOS_ALIGN_CEIL(
                params->pKernelState->KernelParams.iCurbeLength,
                m_hwInterface->GetRenderInterface()->m_stateHeapInterface->pStateHeapInterface->GetCurbeAlignment());
            surfaceCodecParams.dwSize = MOS_BYTES_TO_DWORDS(curbeSize);
            surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMbEncBRCCurbeData;
            surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MBENC_CURBE_ENCODE].Value;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                m_hwInterface,
                cmdBuffer,
                &surfaceCodecParams,
                kernelState));
        }
    }

    if (params->bArbitraryNumMbsInSlice)
    {
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bIs2DSurface = true;
        surfaceCodecParams.bMediaBlockRW = true;
        surfaceCodecParams.psSurface = params->psSliceMapSurface;
        surfaceCodecParams.bRenderTarget = false;
        surfaceCodecParams.bIsWritable = false;
        surfaceCodecParams.dwOffset = currBottomField ? params->dwSliceMapBottomFieldOffset : 0;
        surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncSliceMapData;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));
    }

    if(!params->bMbEncIFrameDistInUse)
    {
        if( params->bMbDisableSkipMapEnabled )
        {
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bIs2DSurface           = true;
        surfaceCodecParams.bMediaBlockRW          = true;
        surfaceCodecParams.psSurface          = params->psMbDisableSkipMapSurface;
        surfaceCodecParams.dwOffset           = 0;
        surfaceCodecParams.dwBindingTableOffset       = avcMbEncBindingTable->dwAvcMBEncMbNonSkipMap;
        surfaceCodecParams.dwCacheabilityControl      =
            m_hwInterface->ComposeSurfaceCacheabilityControl(
            MOS_CODEC_RESOURCE_USAGE_MBDISABLE_SKIPMAP_CODEC,
            codechalLLC);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
        }

        if( params->bStaticFrameDetectionEnabled )
        {
        // static frame detection output surface
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.presBuffer         = params->presSFDOutputBuffer;
        surfaceCodecParams.dwSize         = MOS_BYTES_TO_DWORDS( m_sfdOutputBufferSize );
        surfaceCodecParams.dwOffset          = 0;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ME_DISTORTION_ENCODE].Value;
        surfaceCodecParams.dwBindingTableOffset  = MBENC_STATIC_FRAME_DETECTION_OUTPUT_CM;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
        }
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG8::SendAvcWPSurfaces(PMOS_COMMAND_BUFFER cmdBuffer, PCODECHAL_ENCODE_AVC_WP_SURFACE_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pKernelState);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->psInputRefBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->psOutputScaledBuffer);

    CODECHAL_SURFACE_CODEC_PARAMS surfaceParams;
    MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
    surfaceParams.bIs2DSurface = true;
    surfaceParams.bMediaBlockRW = true;
    surfaceParams.psSurface = params->psInputRefBuffer;// Input surface
    surfaceParams.bIsWritable = false;
    surfaceParams.bRenderTarget = false;
    surfaceParams.dwBindingTableOffset = WP_INPUT_REF_SURFACE;
    surfaceParams.dwVerticalLineStride = params->dwVerticalLineStride;
    surfaceParams.dwVerticalLineStrideOffset = params->dwVerticalLineStrideOffset;
    surfaceParams.ucVDirection = params->ucVDirection;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceParams,
        params->pKernelState));

    MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
    surfaceParams.bIs2DSurface = true;
    surfaceParams.bMediaBlockRW = true;
    surfaceParams.psSurface = params->psOutputScaledBuffer;// output surface
    surfaceParams.bIsWritable = true;
    surfaceParams.bRenderTarget = true;
    surfaceParams.dwBindingTableOffset = WP_OUTPUT_SCALED_SURFACE;
    surfaceParams.dwVerticalLineStride = params->dwVerticalLineStride;
    surfaceParams.dwVerticalLineStrideOffset = params->dwVerticalLineStrideOffset;
    surfaceParams.ucVDirection = params->ucVDirection;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceParams,
        params->pKernelState));

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG8::SendMeSurfaces (
        PMOS_COMMAND_BUFFER cmdBuffer,
        MeSurfaceParams* params)
{
    MOS_STATUS            eStatus = MOS_STATUS_SUCCESS;
    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pKernelState);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pCurrOriginalPic);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->ps4xMeMvDataBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->psMeBrcDistortionBuffer);

    CODECHAL_MEDIA_STATE_TYPE encMediaStateType = (params->b32xMeInUse) ? CODECHAL_MEDIA_STATE_32X_ME :
        params->b16xMeInUse ? CODECHAL_MEDIA_STATE_16X_ME : CODECHAL_MEDIA_STATE_4X_ME;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pMeBindingTable);
    auto meBindingTable = params->pMeBindingTable;

    bool currFieldPicture = CodecHal_PictureIsField(*(params->pCurrOriginalPic)) ? 1 : 0;
    bool currBottomField = CodecHal_PictureIsBottomField(*(params->pCurrOriginalPic)) ? 1 : 0;
    uint8_t currVDirection = (!currFieldPicture) ? CODECHAL_VDIRECTION_FRAME :
        ((currBottomField) ? CODECHAL_VDIRECTION_BOT_FIELD : CODECHAL_VDIRECTION_TOP_FIELD);

    PMOS_SURFACE currScaledSurface = nullptr;
    PMOS_SURFACE meMvDataBuffer = nullptr;
    uint32_t meMvBottomFieldOffset;
    uint32_t currScaledBottomFieldOffset;

    if (params->b32xMeInUse)
    {
        CODECHAL_ENCODE_CHK_NULL_RETURN(params->ps32xMeMvDataBuffer);
        currScaledSurface = m_trackedBuf->Get32xDsSurface(CODEC_CURR_TRACKED_BUFFER);
        meMvDataBuffer = params->ps32xMeMvDataBuffer;
        meMvBottomFieldOffset = params->dw32xMeMvBottomFieldOffset;
        currScaledBottomFieldOffset = params->dw32xScaledBottomFieldOffset;
    }
    else if (params->b16xMeInUse)
    {
        CODECHAL_ENCODE_CHK_NULL_RETURN(params->ps16xMeMvDataBuffer);
        currScaledSurface = m_trackedBuf->Get16xDsSurface(CODEC_CURR_TRACKED_BUFFER);
        meMvDataBuffer = params->ps16xMeMvDataBuffer;
        meMvBottomFieldOffset = params->dw16xMeMvBottomFieldOffset;
        currScaledBottomFieldOffset = params->dw16xScaledBottomFieldOffset;
    }
    else
    {
        currScaledSurface = m_trackedBuf->Get4xDsSurface(CODEC_CURR_TRACKED_BUFFER);
        meMvDataBuffer = params->ps4xMeMvDataBuffer;
        meMvBottomFieldOffset = params->dw4xMeMvBottomFieldOffset;
        currScaledBottomFieldOffset = params->dw4xScaledBottomFieldOffset;
    }

    // Reference height and width information should be taken from the current scaled surface rather
    // than from the reference scaled surface in the case of PAFF.
    auto refScaledSurface = *currScaledSurface;

    uint32_t width = MOS_ALIGN_CEIL(params->dwDownscaledWidthInMb * 32, 64);
    uint32_t height = params->dwDownscaledHeightInMb * 4 * CODECHAL_ENCODE_ME_DATA_SIZE_MULTIPLIER;

    // Force the values
    meMvDataBuffer->dwWidth = width;
    meMvDataBuffer->dwHeight = height;
    meMvDataBuffer->dwPitch = width;
    CODECHAL_SURFACE_CODEC_PARAMS surfaceParams;
    MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
    surfaceParams.bIs2DSurface = true;
    surfaceParams.bMediaBlockRW = true;
    surfaceParams.psSurface = meMvDataBuffer;
    surfaceParams.dwOffset = meMvBottomFieldOffset;
    surfaceParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value;
    surfaceParams.dwBindingTableOffset = meBindingTable->dwMEMVDataSurface;
    surfaceParams.bIsWritable = true;
    surfaceParams.bRenderTarget = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceParams,
        params->pKernelState));

    if (params->b16xMeInUse && params->b32xMeEnabled)
    {
        // Pass 32x MV to 16x ME operation
        MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
        surfaceParams.bIs2DSurface = true;
        surfaceParams.bMediaBlockRW = true;
        surfaceParams.psSurface = params->ps32xMeMvDataBuffer;
        surfaceParams.dwOffset =
        currBottomField ? params->dw32xMeMvBottomFieldOffset : 0;
        surfaceParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value;
        surfaceParams.dwBindingTableOffset = meBindingTable->dw32xMEMVDataSurface;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceParams,
        params->pKernelState));
    }
    else if (!params->b32xMeInUse && params->b16xMeEnabled)
    {
        // Pass 16x MV to 4x ME operation
        MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
        surfaceParams.bIs2DSurface = true;
        surfaceParams.bMediaBlockRW = true;
        surfaceParams.psSurface = params->ps16xMeMvDataBuffer;
        surfaceParams.dwOffset =
        currBottomField ? params->dw16xMeMvBottomFieldOffset : 0;
        surfaceParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value;
        surfaceParams.dwBindingTableOffset = meBindingTable->dw16xMEMVDataSurface;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceParams,
        params->pKernelState));
    }

    // Insert Distortion buffers only for 4xMe case
    if (!params->b32xMeInUse && !params->b16xMeInUse)
    {
        MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
        surfaceParams.bIs2DSurface = true;
        surfaceParams.bMediaBlockRW = true;
        surfaceParams.psSurface = params->psMeBrcDistortionBuffer;
        surfaceParams.dwOffset = params->dwMeBrcDistortionBottomFieldOffset;
        surfaceParams.dwBindingTableOffset = meBindingTable->dwMEBRCDist;
        surfaceParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_BRC_ME_DISTORTION_ENCODE].Value;
        surfaceParams.bIsWritable = true;
        surfaceParams.bRenderTarget = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceParams,
        params->pKernelState));

        MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
        surfaceParams.bIs2DSurface = true;
        surfaceParams.bMediaBlockRW = true;
        surfaceParams.psSurface = params->psMeDistortionBuffer;
        surfaceParams.dwOffset = params->dwMeDistortionBottomFieldOffset;
        surfaceParams.dwBindingTableOffset = meBindingTable->dwMEDist;
        surfaceParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ME_DISTORTION_ENCODE].Value;
        surfaceParams.bIsWritable = true;
        surfaceParams.bRenderTarget = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceParams,
        params->pKernelState));
    }
    uint32_t  refScaledBottomFieldOffset;
    // Setup references 1...n
    // LIST 0 references
    for (uint8_t refIdx = 0; refIdx <= params->dwNumRefIdxL0ActiveMinus1; refIdx++)
    {
        auto refPic = params->pL0RefFrameList[refIdx];

        if (!CodecHal_PictureIsInvalid(refPic) && params->pPicIdx[refPic.FrameIdx].bValid)
        {
        if (refIdx == 0)
        {
            // Current Picture Y - VME
            MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
            surfaceParams.bUseAdvState = true;
            surfaceParams.psSurface = currScaledSurface;
            surfaceParams.dwOffset = currBottomField ? currScaledBottomFieldOffset : 0;
            surfaceParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value;
            surfaceParams.dwBindingTableOffset = meBindingTable->dwMECurrForFwdRef;
            surfaceParams.ucVDirection = currVDirection;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceParams,
            params->pKernelState));
        }

        bool refFieldPicture = CodecHal_PictureIsField(refPic) ? 1 : 0;
        bool refBottomField = (CodecHal_PictureIsBottomField(refPic)) ? 1 : 0;
        uint8_t refPicIdx = params->pPicIdx[refPic.FrameIdx].ucPicIdx;
        uint8_t scaledIdx = params->ppRefList[refPicIdx]->ucScalingIdx;
        if (params->b32xMeInUse)
        {
            PMOS_SURFACE surface = nullptr;
            CODECHAL_ENCODE_CHK_NULL_RETURN(surface = m_trackedBuf->Get32xDsSurface(scaledIdx));
            refScaledSurface.OsResource = surface->OsResource;
            refScaledBottomFieldOffset = refBottomField ? currScaledBottomFieldOffset : 0;
        }
        else if (params->b16xMeInUse)
        {
            PMOS_SURFACE surface = nullptr;
            CODECHAL_ENCODE_CHK_NULL_RETURN(surface = m_trackedBuf->Get16xDsSurface(scaledIdx));
            refScaledSurface.OsResource = surface->OsResource;
            refScaledBottomFieldOffset = refBottomField ? currScaledBottomFieldOffset : 0;
        }
        else
        {
            PMOS_SURFACE surface = nullptr;
            CODECHAL_ENCODE_CHK_NULL_RETURN(surface = m_trackedBuf->Get4xDsSurface(scaledIdx));
            refScaledSurface.OsResource = surface->OsResource;
            refScaledBottomFieldOffset = refBottomField ? currScaledBottomFieldOffset : 0;
        }

        // L0 Reference Picture Y - VME
        MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
        surfaceParams.bUseAdvState = true;
        surfaceParams.psSurface = &refScaledSurface;
        surfaceParams.dwOffset = refBottomField ? refScaledBottomFieldOffset : 0;
        surfaceParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value;
        surfaceParams.dwBindingTableOffset = meBindingTable->dwMEFwdRefPicIdx[refIdx];
        surfaceParams.ucVDirection = !currFieldPicture ? CODECHAL_VDIRECTION_FRAME :
            ((refBottomField) ? CODECHAL_VDIRECTION_BOT_FIELD : CODECHAL_VDIRECTION_TOP_FIELD);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceParams,
            params->pKernelState));
        }
    }

    // Setup references 1...n
    // LIST 1 references
    for (uint8_t refIdx = 0; refIdx <= params->dwNumRefIdxL1ActiveMinus1; refIdx++)
    {
        auto refPic = params->pL1RefFrameList[refIdx];

        if (!CodecHal_PictureIsInvalid(refPic) && params->pPicIdx[refPic.FrameIdx].bValid)
        {
        if (refIdx == 0)
        {
            // Current Picture Y - VME
            MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
            surfaceParams.bUseAdvState = true;
            surfaceParams.psSurface = currScaledSurface;
            surfaceParams.dwOffset = currBottomField ? currScaledBottomFieldOffset : 0;
            surfaceParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value;
            surfaceParams.dwBindingTableOffset = meBindingTable->dwMECurrForBwdRef;
            surfaceParams.ucVDirection = currVDirection;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceParams,
            params->pKernelState));
        }

        bool refFieldPicture = CodecHal_PictureIsField(refPic) ? 1 : 0;
        bool refBottomField = (CodecHal_PictureIsBottomField(refPic)) ? 1 : 0;
        uint8_t refPicIdx = params->pPicIdx[refPic.FrameIdx].ucPicIdx;
        uint8_t scaledIdx = params->ppRefList[refPicIdx]->ucScalingIdx;
        if (params->b32xMeInUse)
        {
            PMOS_SURFACE surface = nullptr;
            CODECHAL_ENCODE_CHK_NULL_RETURN(surface = m_trackedBuf->Get32xDsSurface(scaledIdx));
            refScaledSurface.OsResource = surface->OsResource;
            refScaledBottomFieldOffset = refBottomField ? currScaledBottomFieldOffset : 0;
        }
        else if (params->b16xMeInUse)
        {
            PMOS_SURFACE surface = nullptr;
            CODECHAL_ENCODE_CHK_NULL_RETURN(surface = m_trackedBuf->Get16xDsSurface(scaledIdx));
            refScaledSurface.OsResource = surface->OsResource;
            refScaledBottomFieldOffset = refBottomField ? currScaledBottomFieldOffset : 0;
        }
        else
        {
            PMOS_SURFACE surface = nullptr;
            CODECHAL_ENCODE_CHK_NULL_RETURN(surface = m_trackedBuf->Get4xDsSurface(scaledIdx));
            refScaledSurface.OsResource = surface->OsResource;
            refScaledBottomFieldOffset = refBottomField ? currScaledBottomFieldOffset : 0;
        }

        // L1 Reference Picture Y - VME
        MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
        surfaceParams.bUseAdvState = true;
        surfaceParams.psSurface = &refScaledSurface;
        surfaceParams.dwOffset = refBottomField ? refScaledBottomFieldOffset : 0;
        surfaceParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value;
        surfaceParams.dwBindingTableOffset = meBindingTable->dwMEBwdRefPicIdx[refIdx];
        surfaceParams.ucVDirection = (!currFieldPicture) ? CODECHAL_VDIRECTION_FRAME :
            ((refBottomField) ? CODECHAL_VDIRECTION_BOT_FIELD : CODECHAL_VDIRECTION_TOP_FIELD);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceParams,
            params->pKernelState));
        }
    }
    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG8::InitKernelStateBrc()
{
    MOS_STATUS                              eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    uint8_t* kernelBinary;
    uint32_t kernelSize;

    MOS_STATUS status = CodecHalGetKernelBinaryAndSize(m_kernelBase, m_kuid, &kernelBinary, &kernelSize);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(status);

    // no extra MbBRCUpdate kernel for GEN8
    for (uint32_t krnStateIdx = 0; krnStateIdx < CODECHAL_ENCODE_BRC_IDX_NUM - 1; krnStateIdx++)
    {
        auto kernelStatePtr = &BrcKernelStates[krnStateIdx];

        CODECHAL_KERNEL_HEADER currKrnHeader;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pfnGetKernelHeaderAndSize(
            kernelBinary,
            ENC_BRC,
            krnStateIdx,
            &currKrnHeader,
            &kernelSize));

        auto renderEngineInterface  =     m_hwInterface->GetRenderInterface();
        auto stateHeapInterface     = m_renderEngineInterface->m_stateHeapInterface;
        CODECHAL_ENCODE_CHK_NULL_RETURN(stateHeapInterface);

        kernelStatePtr->KernelParams.iBTCount = m_brcBtCounts[krnStateIdx];
        kernelStatePtr->KernelParams.iThreadCount = m_renderEngineInterface->GetHwCaps()->dwMaxThreads;
        kernelStatePtr->KernelParams.iCurbeLength = m_brcCurbeSize[krnStateIdx];
        kernelStatePtr->KernelParams.iBlockWidth = CODECHAL_MACROBLOCK_WIDTH;
        kernelStatePtr->KernelParams.iBlockHeight = CODECHAL_MACROBLOCK_HEIGHT;
        kernelStatePtr->KernelParams.iIdCount = 1;

        kernelStatePtr->dwCurbeOffset = stateHeapInterface->pStateHeapInterface->GetSizeofCmdInterfaceDescriptorData();
        kernelStatePtr->KernelParams.pBinary = kernelBinary + (currKrnHeader.KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);
        kernelStatePtr->KernelParams.iSize = kernelSize;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(stateHeapInterface->pfnCalculateSshAndBtSizesRequested(
            stateHeapInterface,
            kernelStatePtr->KernelParams.iBTCount,
            &kernelStatePtr->dwSshSize,
            &kernelStatePtr->dwBindingTableSize));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->MhwInitISH(stateHeapInterface, kernelStatePtr));
    }

    // Until a better way can be found, maintain old binding table structures
    auto bindingTable = &BrcUpdateBindingTable;
    bindingTable->dwFrameBrcHistoryBuffer               = BRC_UPDATE_HISTORY;
    bindingTable->dwFrameBrcPakStatisticsOutputBuffer   = BRC_UPDATE_PAK_STATISTICS_OUTPUT;
    bindingTable->dwFrameBrcImageStateReadBuffer        = BRC_UPDATE_IMAGE_STATE_READ;
    bindingTable->dwFrameBrcImageStateWriteBuffer       = BRC_UPDATE_IMAGE_STATE_WRITE;
    bindingTable->dwFrameBrcMbEncCurbeReadBuffer        = BRC_UPDATE_MBENC_CURBE_READ;
    bindingTable->dwFrameBrcMbEncCurbeWriteData         = BRC_UPDATE_MBENC_CURBE_WRITE;
    bindingTable->dwFrameBrcDistortionBuffer            = BRC_UPDATE_DISTORTION;
    bindingTable->dwFrameBrcConstantData                = BRC_UPDATE_CONSTANT_DATA;
    bindingTable->dwMbBrcMbQpBuffer                     = BRC_UPDATE_MB_QP;

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG8::SetCurbeMe(MeCurbeParams* params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pKernelState);
    CODECHAL_ENCODE_ASSERT(m_avcSeqParam->TargetUsage <= NUM_TARGET_USAGE_MODES);

    uint8_t mvShiftFactor = 0;
    uint8_t prevMvReadPosFactor = 0;
    bool  useMvFromPrevStep;
    bool  writeDistortions;
    uint32_t scaleFactor;
    switch (params->hmeLvl)
    {
    case HME_LEVEL_32x:
        useMvFromPrevStep = m_hmeFirstStep;
        writeDistortions = false;
        scaleFactor = SCALE_FACTOR_32x;
        mvShiftFactor = m_mvShiftFactor32x;
        break;
    case HME_LEVEL_16x:
        useMvFromPrevStep = (m_32xMeEnabled) ? m_hmeFollowingStep : m_hmeFirstStep;
        writeDistortions = false;
        scaleFactor = SCALE_FACTOR_16x;
        mvShiftFactor = m_mvShiftFactor16x;
        prevMvReadPosFactor = m_prevMvReadPosition16x;
        break;
    case HME_LEVEL_4x:
        useMvFromPrevStep = (m_16xMeEnabled) ? m_hmeFollowingStep : m_hmeFirstStep;
        writeDistortions = true;
        scaleFactor = SCALE_FACTOR_4x;
        mvShiftFactor = m_mvShiftFactor4x;
        prevMvReadPosFactor = m_prevMvReadPosition8x;
        break;
    default:
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
        break;
    }

    ME_CURBE_CM  cmd;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
        &cmd,
        sizeof(ME_CURBE_CM),
        m_initMeCurbeCm,
        sizeof(ME_CURBE_CM)));

    // r1
    //cmd.DW0.T8x8FlagForInterEn  = picParams->transform_8x8_mode_flag;
    cmd.DW3.SubPelMode = 3;
    if (m_fieldScalingOutputInterleaved)
    {
        cmd.DW3.SrcAccess =
            cmd.DW3.RefAccess = CodecHal_PictureIsField(m_avcPicParam->CurrOriginalPic) ? 1 : 0;
        cmd.DW7.SrcFieldPolarity = CodecHal_PictureIsBottomField(m_avcPicParam->CurrOriginalPic) ? 1 : 0;
    }

    cmd.DW4.PictureHeightMinus1 =
        CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameFieldHeight / scaleFactor) - 1;
    cmd.DW4.PictureWidth =
        CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameWidth / scaleFactor);

    char qpPrimeY =
        (m_avcPicParam->pic_init_qp_minus26 + 26) +
        m_avcSliceParams->slice_qp_delta;

    cmd.DW5.QpPrimeY = qpPrimeY;
    cmd.DW6.WriteDistortions = writeDistortions;
    cmd.DW6.UseMvFromPrevStep = useMvFromPrevStep;

    cmd.DW6.SuperCombineDist = m_superCombineDistGeneric[m_avcSeqParam->TargetUsage];
    bool framePicture = CodecHal_PictureIsFrame(m_avcPicParam->CurrOriginalPic);
    cmd.DW6.MaxVmvR = (framePicture) ?
        CodecHalAvcEncode_GetMaxMvLen(m_avcSeqParam->Level) * 4 :
        (CodecHalAvcEncode_GetMaxMvLen(m_avcSeqParam->Level) >> 1) * 4;

    if (m_pictureCodingType == B_TYPE)
    {
        // This field is irrelevant since we are not using the bi-direct search.
        // set it to 32
        cmd.DW1.BiWeight = 32;
        cmd.DW13.NumRefIdxL1MinusOne =
            m_avcSliceParams->num_ref_idx_l1_active_minus1;
    }

    if (m_pictureCodingType == P_TYPE ||
        m_pictureCodingType == B_TYPE)
    {
        cmd.DW13.NumRefIdxL0MinusOne =
            m_avcSliceParams->num_ref_idx_l0_active_minus1;
    }

    if (!framePicture)
    {
        auto slcParams = m_avcSliceParams;
        if (m_pictureCodingType != I_TYPE)
        {
            cmd.DW14.List0RefID0FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_0);
            cmd.DW14.List0RefID1FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_1);
            cmd.DW14.List0RefID2FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_2);
            cmd.DW14.List0RefID3FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_3);
            cmd.DW14.List0RefID4FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_4);
            cmd.DW14.List0RefID5FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_5);
            cmd.DW14.List0RefID6FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_6);
            cmd.DW14.List0RefID7FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_7);
        }
        if (m_pictureCodingType == B_TYPE)
        {
            cmd.DW14.List1RefID0FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_1, CODECHAL_ENCODE_REF_ID_0);
            cmd.DW14.List1RefID1FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_1, CODECHAL_ENCODE_REF_ID_1);
        }
    }

    cmd.DW15.MvShiftFactor = mvShiftFactor;
    cmd.DW15.PrevMvReadPosFactor = prevMvReadPosFactor;

    // r3 & r4
    uint8_t meMethod = (m_pictureCodingType == B_TYPE) ? m_bMeMethodGeneric[m_avcSeqParam->TargetUsage] : m_meMethodGeneric[m_avcSeqParam->TargetUsage];
    uint8_t tableIdx = (m_pictureCodingType == B_TYPE) ? 1 : 0;
    eStatus = MOS_SecureMemcpy(&(cmd.SPDelta), 14 * sizeof(uint32_t), m_encodeSearchPath[tableIdx][meMethod], 14 * sizeof(uint32_t));
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
        return eStatus;
    }

    // r5
    cmd.DW32._4xMeMvOutputDataSurfIndex         = ME_MV_DATA_SURFACE_CM;
    cmd.DW33._16xOr32xMeMvInputDataSurfIndex    = (params->hmeLvl == HME_LEVEL_32x) ?
        ME_32x_MV_DATA_SURFACE_CM : ME_16x_MV_DATA_SURFACE_CM;
    cmd.DW34._4xMeOutputDistSurfIndex           = ME_DISTORTION_SURFACE_CM;
    cmd.DW35._4xMeOutputBrcDistSurfIndex        = ME_BRC_DISTORTION_CM;
    cmd.DW36.VMEFwdInterPredictionSurfIndex     = ME_CURR_FOR_FWD_REF_CM;
    cmd.DW37.VMEBwdInterPredictionSurfIndex     = ME_CURR_FOR_BWD_REF_CM;
    cmd.DW38.Value = 0; //MBZ for BDW
    CODECHAL_ENCODE_CHK_STATUS_RETURN(params->pKernelState->m_dshRegion.AddData(
        &cmd,
        params->pKernelState->dwCurbeOffset,
        sizeof(cmd)));

    return eStatus;
}

void CodechalEncodeAvcEncG8::UpdateSSDSliceCount()
{
    CodechalEncodeAvcBase::UpdateSSDSliceCount();

    uint32_t sliceCount;
    if ((m_frameHeight * m_frameWidth >= 1920*1080 && m_targetUsage <= 4) ||
        (m_frameHeight * m_frameWidth >= 1280*720 && m_targetUsage <= 2) ||
        (m_frameHeight * m_frameWidth >= 3840*2160))
    {
        sliceCount = 2;
    }
    else
    {
        sliceCount = 1;
    }

    if (m_osInterface->pfnSetSliceCount)
    {
        m_osInterface->pfnSetSliceCount(m_osInterface, &sliceCount);
    }
}

