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
//! \file     codechal_encode_avc_g9_kbl.h
//! \brief    AVC dual-pipe encoder for GEN9 KBL & GML.
//!

#ifndef __CODECHAL_ENCODE_AVC_G9_KBL_H__
#define __CODECHAL_ENCODE_AVC_G9_KBL_H__

#include "codechal_encode_avc.h"
#include "codechal_encode_avc_g9.h"

typedef struct _CODECHAL_ENCODE_AVC_BRC_INIT_RESET_CURBE_G95
{
    union
    {
        struct
        {
            uint32_t   ProfileLevelMaxFrame : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   InitBufFullInBits : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   BufSizeInBits : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   AverageBitRate : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   MaxBitRate : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   MinBitRate : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   FrameRateM : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   FrameRateD : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   BRCFlag : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   GopP : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   GopB : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   FrameWidthInBytes : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   FrameHeightInBytes : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   AVBRAccuracy : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   AVBRConvergence : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   MinQP : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   MaxQP : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   NoSlices : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   InstantRateThreshold0ForP : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   InstantRateThreshold1ForP : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   InstantRateThreshold2ForP : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   InstantRateThreshold3ForP : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   InstantRateThreshold0ForB : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   InstantRateThreshold1ForB : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   InstantRateThreshold2ForB : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   InstantRateThreshold3ForB : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   InstantRateThreshold0ForI : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   InstantRateThreshold1ForI : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   InstantRateThreshold2ForI : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   InstantRateThreshold3ForI : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   DeviationThreshold0ForPandB : MOS_BITFIELD_RANGE(0, 7);     // Signed byte
            uint32_t   DeviationThreshold1ForPandB : MOS_BITFIELD_RANGE(8, 15);     // Signed byte
            uint32_t   DeviationThreshold2ForPandB : MOS_BITFIELD_RANGE(16, 23);     // Signed byte
            uint32_t   DeviationThreshold3ForPandB : MOS_BITFIELD_RANGE(24, 31);     // Signed byte
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
            uint32_t   DeviationThreshold4ForPandB : MOS_BITFIELD_RANGE(0, 7);     // Signed byte
            uint32_t   DeviationThreshold5ForPandB : MOS_BITFIELD_RANGE(8, 15);     // Signed byte
            uint32_t   DeviationThreshold6ForPandB : MOS_BITFIELD_RANGE(16, 23);     // Signed byte
            uint32_t   DeviationThreshold7ForPandB : MOS_BITFIELD_RANGE(24, 31);     // Signed byte
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
            uint32_t   DeviationThreshold0ForVBR : MOS_BITFIELD_RANGE(0, 7);     // Signed byte
            uint32_t   DeviationThreshold1ForVBR : MOS_BITFIELD_RANGE(8, 15);     // Signed byte
            uint32_t   DeviationThreshold2ForVBR : MOS_BITFIELD_RANGE(16, 23);     // Signed byte
            uint32_t   DeviationThreshold3ForVBR : MOS_BITFIELD_RANGE(24, 31);     // Signed byte
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
            uint32_t   DeviationThreshold4ForVBR : MOS_BITFIELD_RANGE(0, 7);     // Signed byte
            uint32_t   DeviationThreshold5ForVBR : MOS_BITFIELD_RANGE(8, 15);     // Signed byte
            uint32_t   DeviationThreshold6ForVBR : MOS_BITFIELD_RANGE(16, 23);     // Signed byte
            uint32_t   DeviationThreshold7ForVBR : MOS_BITFIELD_RANGE(24, 31);     // Signed byte
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
            uint32_t   DeviationThreshold0ForI : MOS_BITFIELD_RANGE(0, 7);        // Signed byte
            uint32_t   DeviationThreshold1ForI : MOS_BITFIELD_RANGE(8, 15);       // Signed byte
            uint32_t   DeviationThreshold2ForI : MOS_BITFIELD_RANGE(16, 23);      // Signed byte
            uint32_t   DeviationThreshold3ForI : MOS_BITFIELD_RANGE(24, 31);      // Signed byte
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
            uint32_t   DeviationThreshold4ForI : MOS_BITFIELD_RANGE(0, 7);      // Signed byte
            uint32_t   DeviationThreshold5ForI : MOS_BITFIELD_RANGE(8, 15);      // Signed byte
            uint32_t   DeviationThreshold6ForI : MOS_BITFIELD_RANGE(16, 23);      // Signed byte
            uint32_t   DeviationThreshold7ForI : MOS_BITFIELD_RANGE(24, 31);      // Signed byte
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
            uint32_t   InitialQPForI : MOS_BITFIELD_RANGE(0, 7);     // Signed byte
            uint32_t   InitialQPForP : MOS_BITFIELD_RANGE(8, 15);     // Signed byte
            uint32_t   InitialQPForB : MOS_BITFIELD_RANGE(16, 23);     // Signed byte
            uint32_t   SlidingWindowSize : MOS_BITFIELD_RANGE(24, 31);     // unsigned byte
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
            uint32_t   ACQP : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   LongTermInterval : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   Reserved : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   Reserved : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   Reserved : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   Reserved : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW27;

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
    } DW28;

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
    } DW29;

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
    } DW30;

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
    } DW31;

    union
    {
        struct
        {
            uint32_t   SurfaceIndexhistorybuffer : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };

    } DW32;

    union
    {
        struct
        {
            uint32_t  SurfaceIndexdistortionbuffer : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW33;
} CODECHAL_ENCODE_AVC_BRC_INIT_RESET_CURBE_G95, *PCODECHAL_ENCODE_AVC_BRC_INIT_RESET_CURBE_G95;
C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(CODECHAL_ENCODE_AVC_BRC_INIT_RESET_CURBE_G95)) == 34);

typedef struct _CODECHAL_ENCODE_AVC_FRAME_BRC_UPDATE_CURBE_G95
{
    union
    {
        struct
        {
            uint32_t   TargetSize : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   FrameNumber : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   SizeofPicHeaders : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   startGAdjFrame0 : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   startGAdjFrame1 : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   startGAdjFrame2 : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   startGAdjFrame3 : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   TargetSizeFlag : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   BRCFlag : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   MaxNumPAKs : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   CurrFrameType : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   NumSkipFrames : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   MinimumQP : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   MaximumQP : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   EnableForceToSkip : MOS_BITFIELD_BIT(24);
            uint32_t   EnableSlidingWindow : MOS_BITFIELD_BIT(25);
            uint32_t   EnableExtremLowDelay : MOS_BITFIELD_BIT(26);
            uint32_t   DisableVarCompute : MOS_BITFIELD_BIT(27);
            uint32_t   Reserved : MOS_BITFIELD_RANGE(28, 31);
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
            uint32_t    SizeSkipFrames : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   StartGlobalAdjustMult0 : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   StartGlobalAdjustMult1 : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   StartGlobalAdjustMult2 : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   StartGlobalAdjustMult3 : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   StartGlobalAdjustMult4 : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   StartGlobalAdjustDiv0 : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   StartGlobalAdjustDiv1 : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   StartGlobalAdjustDiv2 : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   StartGlobalAdjustDiv3 : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   StartGlobalAdjustDiv4 : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   QPThreshold0 : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   QPThreshold1 : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   QPThreshold2 : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   QPThreshold3 : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   gRateRatioThreshold0 : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   gRateRatioThreshold1 : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   gRateRatioThreshold2 : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   gRateRatioThreshold3 : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   gRateRatioThreshold4 : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   gRateRatioThreshold5 : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   gRateRatioThresholdQP0 : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   gRateRatioThresholdQP1 : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   gRateRatioThresholdQP2 : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   gRateRatioThresholdQP3 : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   gRateRatioThresholdQP4 : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   gRateRatioThresholdQP5 : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   gRateRatioThresholdQP6 : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   QPIndexOfCurPic : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   Reserved : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   EnableROI : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   RoundingIntra : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   RoundingInter : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   Reserved : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   Reserved : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   Reserved : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   UserMaxFrame : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   Reserved : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   Reserved : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   Reserved : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   Reserved : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   SurfaceIndexBRChistorybuffer : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   SurfaceIndexPreciousPAKstatisticsoutputbuffer : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   SurfaceIndexAVCIMGstateinputbuffer : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t  SurfaceIndexAVCIMGstateoutputbuffer : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t  Value;
        };
    } DW27;

    union
    {
        struct
        {
            uint32_t  SurfaceIndexAVC_Encbuffer : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t  Value;
        };
    } DW28;

    union
    {
        struct
        {
            uint32_t  SurfaceIndexAVCDISTORTIONbuffer : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t  Value;
        };
    } DW29;

    union
    {
        struct
        {
            uint32_t  SurfaceIndexBRCconstdatabuffer : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t  Value;
        };
    } DW30;

    union
    {
        struct
        {
            uint32_t  SurfaceIndexMBStatsBuffer : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t  Value;
        };
    } DW31;

    union
    {
        struct
        {
            uint32_t  SurfaceIndexMotionvectorbuffer : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t  Value;
        };
    } DW32;
} CODECHAL_ENCODE_AVC_BRC_UPDATE_CURBE_G95, *PCODECHAL_ENCODE_AVC_BRC_UPDATE_CURBE_G95;
C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(CODECHAL_ENCODE_AVC_BRC_UPDATE_CURBE_G95)) == 33);

typedef struct _CODECHAL_ENCODE_AVC_MB_BRC_UPDATE_CURBE_G95
{
    union
    {
        struct
        {
            uint32_t   CurrFrameType : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   EnableROI : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   ROIRatio : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   Reserved : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   Reserved : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   Reserved : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   Reserved : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   Reserved : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   Reserved : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   Reserved : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t Reserved : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t  Value;
        };
    } DW7;

    union
    {
        struct
        {
            uint32_t HistorybufferIndex : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t Value;
        };
    } DW8;

    union
    {
        struct
        {
            uint32_t MBQPbufferIndex : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t Value;
        };
    } DW9;

    union
    {
        struct
        {
            uint32_t ROIbufferIndex : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t Value;
        };
    } DW10;

    union
    {
        struct
        {
            uint32_t MBstatisticalbufferIndex : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t Value;
        };
    }DW11;

} CODECHAL_ENCODE_AVC_MB_BRC_UPDATE_CURBE_G95, *PCODECHAL_ENCODE_AVC_MB_BRC_UPDATE_CURBE_G95;
C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(CODECHAL_ENCODE_AVC_MB_BRC_UPDATE_CURBE_G95)) == 12);

// AVC Gen 9 WP kernel CURBE
typedef struct _CODECHAL_ENCODE_AVC_WP_CURBE_G9_KBL
{
    // DW0
    union
    {
        struct
        {
            uint32_t   DefaultWeight       : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   DefaultOffset       : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI0_X_left         : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI0_Y_top          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI0_X_right        : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI0_Y_bottom       : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI0Weight          : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI0Offset          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI1_X_left         : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI1_Y_top          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI1_X_right        : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI1_Y_bottom       : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI1Weight          : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI1Offset          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI2_X_left         : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI2_Y_top          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI2_X_right        : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI2_Y_bottom       : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI2Weight          : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI2Offset          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI3_X_left         : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI3_Y_top          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI3_X_right        : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI3_Y_bottom       : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI3Weight          : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI3Offset          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI4_X_left         : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI4_Y_top          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI4_X_right        : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI4_Y_bottom       : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI4Weight          : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI4Offset          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI5_X_left         : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI5_Y_top          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI5_X_right        : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI5_Y_bottom       : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI5Weight          : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI5Offset          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI6_X_left         : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI6_Y_top          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI6_X_right        : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI6_Y_bottom       : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI6Weight          : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI6Offset          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI7_X_left         : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI7_Y_top          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI7_X_right        : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI7_Y_bottom       : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI7Weight          : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI7Offset          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI8_X_left         : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI8_Y_top          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI8_X_right        : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI8_Y_bottom       : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI8Weight          : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI8Offset          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI9_X_left         : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI9_Y_top          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI9_X_right        : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI9_Y_bottom       : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI9Weight          : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI9Offset          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI10_X_left         : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI10_Y_top          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI10_X_right        : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI10_Y_bottom       : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI10Weight          : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI10Offset          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI11_X_left         : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI11_Y_top          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI11_X_right        : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI11_Y_bottom       : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI11Weight          : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI11Offset          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI12_X_left         : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI12_Y_top          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI12_X_right        : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI12_Y_bottom       : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI12Weight          : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI12Offset          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI13_X_left         : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI13_Y_top          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI13_X_right        : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI13_Y_bottom       : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI13Weight          : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI13Offset          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI14_X_left         : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI14_Y_top          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI14_X_right        : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI14_Y_bottom       : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI14Weight          : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI14Offset          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI15_X_left         : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI15_Y_top          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI15_X_right        : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI15_Y_bottom       : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW47;

    // DW48
    union
    {
        struct
        {
            uint32_t   ROI15Weight          : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI15Offset          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   InputSurface          : MOS_BITFIELD_RANGE(  0,31 );
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
            uint32_t   OutputSurface         : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW50;

} CODECHAL_ENCODE_AVC_WP_CURBE_G9_KBL, *PCODECHAL_ENCODE_AVC_WP_CURBE_G9_KBL;

C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(CODECHAL_ENCODE_AVC_WP_CURBE_G9_KBL)) == 51);

class CodechalEncodeAvcEncG9Kbl : public CodechalEncodeAvcEncG9
{
public:

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
    //! \brief    Constructor
    //!
    CodechalEncodeAvcEncG9Kbl(
        CodechalHwInterface *   hwInterface,
        CodechalDebugInterface *debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo);

    //!
    //! \brief    Destructor
    //!
    ~CodechalEncodeAvcEncG9Kbl() {};

    //!
    //! \brief    Set MbEnc kernel curbe data
    //!
    //! \param    [in] params
    //!           Pointer to CODECHAL_ENCODE_AVC_MBENC_CURBE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetCurbeAvcMbEnc(
        PCODECHAL_ENCODE_AVC_MBENC_CURBE_PARAMS params);

    //!
    //! \brief    Get Trellis Quantization mode/value enable or not.
    //!
    //! \param    [in] params
    //!           Pointer to CODECHAL_ENCODE_AVC_TQ_INPUT_PARAMS.
    //! \param    [out] trellisQuantParams
    //!           Pointer to CODECHAL_ENCODE_AVC_TQ_PARAMS, mode & value setup.
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetTrellisQuantization(
        PCODECHAL_ENCODE_AVC_TQ_INPUT_PARAMS    params,
        PCODECHAL_ENCODE_AVC_TQ_PARAMS          trellisQuantParams);

    //!
    //! \brief    Initialize brc constant buffer
    //!
    //! \param    [in] params
    //!           Pointer to CODECHAL_ENCODE_AVC_INIT_BRC_CONSTANT_BUFFER_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitBrcConstantBuffer(
        PCODECHAL_ENCODE_AVC_INIT_BRC_CONSTANT_BUFFER_PARAMS        params);

    // state related funcs
    //!
    //! \brief    Initialize encode state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitializeState();

    //!
    //! \brief    Init MbEnc kernel state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitKernelStateMbEnc();

    //!
    //! \brief    Init BRC kernel state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitKernelStateBrc();

    //!
    //! \brief    Init WeightPrediction kernel state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetCurbeAvcWP(
        PCODECHAL_ENCODE_AVC_WP_CURBE_PARAMS params);

    //!
    //! \brief  Realize the scene change report
    //! \param  [in] cmdBuffer
    //!         Command buffer
    //!         [in]  params
    //!           Pointer to the CODECHAL_ENCODE_AVC_GENERIC_PICTURE_LEVEL_PARAMS
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS SceneChangeReport(
        PMOS_COMMAND_BUFFER       cmdBuffer,
        PCODECHAL_ENCODE_AVC_GENERIC_PICTURE_LEVEL_PARAMS   params);

    //!
    //! \brief    Set Brc InitReset Curbe
    //!
    //! \param    [in] params
    //!           Pointer to PCODECHAL_ENCODE_AVC_BRC_INIT_RESET_CURBE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetCurbeAvcBrcInitReset(
        PCODECHAL_ENCODE_AVC_BRC_INIT_RESET_CURBE_PARAMS params);

    //!
    //! \brief    Set Brc update Curbe
    //!
    //! \param    [in] params
    //!           Pointer to PCODECHAL_ENCODE_AVC_BRC_UPDATE_CURBE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetCurbeAvcFrameBrcUpdate(
        PCODECHAL_ENCODE_AVC_BRC_UPDATE_CURBE_PARAMS params);

    //!
    //! \brief    Set MBBrc update Curbe
    //!
    //! \param    [in] params
    //!           Pointer to PCODECHAL_ENCODE_AVC_BRC_UPDATE_CURBE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetCurbeAvcMbBrcUpdate(
        PCODECHAL_ENCODE_AVC_BRC_UPDATE_CURBE_PARAMS params);

#if USE_CODECHAL_DEBUG_TOOL
protected:
    virtual MOS_STATUS PopulateBrcInitParam(
        void *cmd);

    virtual MOS_STATUS PopulateBrcUpdateParam(
        void *cmd);

    virtual MOS_STATUS PopulateEncParam(
        uint8_t meMethod,
        void    *cmd);

    virtual MOS_STATUS PopulatePakParam(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_BATCH_BUFFER   secondLevelBatchBuffer);
#endif

private:
    static const uint16_t Lambda_data[256];
    static const uint8_t  FTQ25[64];

    static const uint32_t MBEnc_CURBE_normal_I_frame[88];
    static const uint32_t MBEnc_CURBE_normal_I_field[88];
    static const uint32_t MBEnc_CURBE_normal_P_frame[88];
    static const uint32_t MBEnc_CURBE_normal_P_field[88];
    static const uint32_t MBEnc_CURBE_normal_B_frame[88];
    static const uint32_t MBEnc_CURBE_normal_B_field[88];
    static const uint32_t MBEnc_CURBE_I_frame_DIST[88];
    static const int32_t  BRC_BTCOUNTS[CODECHAL_ENCODE_BRC_IDX_NUM];
    static const int32_t  BRC_CURBE_SIZE[CODECHAL_ENCODE_BRC_IDX_NUM];
};

#endif  // __CODECHAL_ENCODE_AVC_G9_KBL_H__
