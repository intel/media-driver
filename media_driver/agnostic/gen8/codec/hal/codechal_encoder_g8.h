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
//! \file        codechal_encoder_g8.h   
//! \brief        This modules implements Render interface layer for decoding to be used               on all operating systems/DDIs, across CODECHAL components.   
//!
#ifndef __CODECHAL_ENCODER_G75_H__
#define __CODECHAL_ENCODER_G75_H__

#include "codechal_encoder.h"

typedef enum _CODECHAL_ENCODE_BINDING_TABLE_OFFSET_SCALING_G75
{
    CODECHAL_ENCODE_SCALING_FRAME_SRC_Y_G75             = 0,
    CODECHAL_ENCODE_SCALING_FRAME_DST_Y_G75             = 24,
    CODECHAL_ENCODE_SCALING_FIELD_TOP_SRC_Y_G75         = 0,
    CODECHAL_ENCODE_SCALING_FIELD_BOT_SRC_Y_G75         = 3,
    CODECHAL_ENCODE_SCALING_FIELD_TOP_DST_Y_G75         = 24,
    CODECHAL_ENCODE_SCALING_FIELD_BOT_DST_Y_G75         = 27,
    CODECHAL_ENCODE_SCALING_NUM_SURFACES_G75            = 4
} CODECHAL_ENCODE_BINDING_TABLE_OFFSET_SCALING_G75;

typedef enum _CODECHAL_BINDING_TABLE_OFFSET_SCALING_CM_G75
{
    CODECHAL_SCALING_FRAME_SRC_Y_CM_G75                 = 0,
    CODECHAL_SCALING_FRAME_DST_Y_CM_G75                 = 1,
    CODECHAL_SCALING_FIELD_TOP_SRC_Y_CM_G75             = 0,
    CODECHAL_SCALING_FIELD_TOP_DST_Y_CM_G75             = 1,
    CODECHAL_SCALING_FIELD_BOT_SRC_Y_CM_G75             = 2,
    CODECHAL_SCALING_FIELD_BOT_DST_Y_CM_G75             = 3,
    CODECHAL_SCALING_FRAME_FLATNESS_DST_CM_G75          = 4,
    CODECHAL_SCALING_FIELD_TOP_FLATNESS_DST_CM_G75      = 4,
    CODECHAL_SCALING_FIELD_BOT_FLATNESS_DST_CM_G75      = 5,
    CODECHAL_SCALING_FRAME_MBVPROCSTATS_DST_CM_G75      = 6,
    CODECHAL_SCALING_FIELD_TOP_MBVPROCSTATS_DST_CM_G75  = 6,
    CODECHAL_SCALING_FIELD_BOT_MBVPROCSTATS_DST_CM_G75  = 7,
    CODECHAL_SCALING_NUM_SURFACES_CM_G75                = 8
} CODECHAL_BINDING_TABLE_OFFSET_SCALING_CM_G75;

typedef enum _CODECHAL_BINDING_TABLE_OFFSET_2xSCALING_CM_G75
{
    CODECHAL_2xSCALING_FRAME_SRC_Y_CM_G75            = 0,
    CODECHAL_2xSCALING_FRAME_DST_Y_CM_G75            = 1,
    CODECHAL_2xSCALING_FIELD_TOP_SRC_Y_CM_G75        = 0,
    CODECHAL_2xSCALING_FIELD_TOP_DST_Y_CM_G75        = 1,
    CODECHAL_2xSCALING_FIELD_BOT_SRC_Y_CM_G75        = 2,
    CODECHAL_2xSCALING_FIELD_BOT_DST_Y_CM_G75        = 3,
    CODECHAL_2xSCALING_NUM_SURFACES_CM_G75           = 4
}CODECHAL_BINDING_TABLE_OFFSET_2xSCALING_CM_G75;

typedef struct _MEDIA_OBJECT_SCALING_INLINE_DATA_G75
{
    // uint32_t 0 - GRF R7.0
    union
    {
        // All
        struct
        {
            uint32_t       DestinationBlockHorizontalOrigin : 16;
            uint32_t       DestinationBlockVerticalOrigin   : 16;
        };

        // Block Copy
        struct  
        {
            uint32_t       BlockHeight                     : 16;
            uint32_t       BufferOffset                    : 16;
        };

        // FMD Summation
        struct
        {
            uint32_t       StartRowOffset;
        };

        uint32_t       Value;
    } DW00;

    // uint32_t 1 - GRF R7.1
    union
    {
        // Composite
        struct
        {
            uint32_t       HorizontalBlockCompositeMaskLayer0 : 16;
            uint32_t       VerticalBlockCompositeMaskLayer0   : 16;
        };

        // FMD Summation
        struct
        {
            uint32_t       TotalRows;
        };

        uint32_t       Value;
    } DW01;

    // uint32_t 2 - GRF R7.2
    union
    {
        // Composite
        struct
        {
            uint32_t       HorizontalBlockCompositeMaskLayer1 : 16;
            uint32_t       VerticalBlockCompositeMaskLayer1   : 16;
        };

        // FMD Summation
        struct
        {
            uint32_t       StartColumnOffset;
        };

        uint32_t       Value;
    } DW02;

    // uint32_t 3 - GRF R7.3
    union
    {
        // Composite
        struct
        {
            uint32_t       HorizontalBlockCompositeMaskLayer2 : 16;
            uint32_t       VerticalBlockCompositeMaskLayer2   : 16;
        };

        // FMD Summation
        struct
        {
            uint32_t       TotalColumns;
        };

        uint32_t       Value;
    } DW03;

    // uint32_t 4 - GRF R7.4
    union
    {
        // Sampler Load
        struct
        {
            float       VideoXScalingStep;
        };

        uint32_t       Value;
    } DW04;

    // uint32_t 5 - GRF R7.5
    union
    {
        // NLAS
        struct
        {
            float       VideoStepDelta;
        };

        uint32_t       Value;
    } DW05;

    // uint32_t 6 - GRF R7.6
    union
    {
        // AVScaling
        struct
        {
            uint32_t       VerticalBlockNumber                :17;
            uint32_t       AreaOfInterest                     :1;
            uint32_t                                          :14;
        };

        uint32_t       Value;
    } DW06;

    // uint32_t 7 - GRF R7.7
    union
    {
        // AVScaling
        struct
        {
            uint32_t       GroupIDNumber;
        };

        uint32_t       Value;
    } DW07;

    // uint32_t 8 - GRF R8.0
    union
    {
        // Composite
        struct
        {
            uint32_t       HorizontalBlockCompositeMaskLayer3 : 16;
            uint32_t       VerticalBlockCompositeMaskLayer3   : 16;
        };

        uint32_t       Value;
    } DW08;

    // uint32_t 9 - GRF R8.1
    union
    {
        // Composite
        struct
        {
            uint32_t       HorizontalBlockCompositeMaskLayer4 : 16;
            uint32_t       VerticalBlockCompositeMaskLayer4   : 16;
        };

        uint32_t       Value;
    } DW09;

    // uint32_t 10 - GRF R8.2
    union
    {
        // Composite
        struct
        {
            uint32_t       HorizontalBlockCompositeMaskLayer5 : 16;
            uint32_t       VerticalBlockCompositeMaskLayer5   : 16;
        };

        uint32_t       Value;
    } DW10;

    // uint32_t 11 - GRF R8.3
    union
    {
        // Composite
        struct
        {
            uint32_t       HorizontalBlockCompositeMaskLayer6 : 16;
            uint32_t       VerticalBlockCompositeMaskLayer6   : 16;
        };

        uint32_t       Value;
    } DW11;

    // uint32_t 12 - GRF R8.4
    union
    {
        // Composite
        struct
        {
            uint32_t       HorizontalBlockCompositeMaskLayer7 : 16;
            uint32_t       VerticalBlockCompositeMaskLayer7   : 16;
        };

        uint32_t       Value;
    } DW12;

    // uint32_t 13 - GRF R8.5
    union
    {
        struct
        {
            uint32_t       Reserved;
        };

        uint32_t       Value;
    } DW13;

    // uint32_t 14 - GRF R8.6
    union
    {
        struct
        {
            uint32_t       Reserved;
        };

        uint32_t       Value;
    } DW14;

    // uint32_t 15 - GRF R8.7
    union
    {
        struct
        {
            uint32_t       Reserved;
        };

        uint32_t       Value;
    } DW15;
} MEDIA_OBJECT_SCALING_INLINE_DATA_G75, *PMEDIA_OBJECT_SCALING_INLINE_DATA_G75;

#endif  // __CODECHAL_ENCODER_G75_H__
