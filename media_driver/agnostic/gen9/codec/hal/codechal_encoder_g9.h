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
//! \file        codechal_encoder_g9.h   
//! \brief        This modules implements Render interface layer for decoding to be used               on all operating systems/DDIs, across CODECHAL components.   
//!
#ifndef __CODECHAL_ENCODER_G9_H__
#define __CODECHAL_ENCODER_G9_H__

#include "codechal_encoder.h"

typedef enum _CODECHAL_ENCODE_BINDING_TABLE_OFFSET_ME_CM_G9
{
    CODECHAL_ENCODE_ME_MV_DATA_SURFACE_CM_G9       = 0,
    CODECHAL_ENCODE_16xME_MV_DATA_SURFACE_CM_G9    = 1,
    CODECHAL_ENCODE_32xME_MV_DATA_SURFACE_CM_G9    = 1,
    CODECHAL_ENCODE_ME_DISTORTION_SURFACE_CM_G9    = 2,
    CODECHAL_ENCODE_ME_BRC_DISTORTION_CM_G9        = 3,
    CODECHAL_ENCODE_ME_RESERVED0_CM_G9             = 4,
    CODECHAL_ENCODE_ME_CURR_FOR_FWD_REF_CM_G9      = 5,
    CODECHAL_ENCODE_ME_FWD_REF_IDX0_CM_G9          = 6,
    CODECHAL_ENCODE_ME_RESERVED1_CM_G9             = 7,
    CODECHAL_ENCODE_ME_FWD_REF_IDX1_CM_G9          = 8,
    CODECHAL_ENCODE_ME_RESERVED2_CM_G9             = 9,
    CODECHAL_ENCODE_ME_FWD_REF_IDX2_CM_G9          = 10,
    CODECHAL_ENCODE_ME_RESERVED3_CM_G9             = 11,
    CODECHAL_ENCODE_ME_FWD_REF_IDX3_CM_G9          = 12,
    CODECHAL_ENCODE_ME_RESERVED4_CM_G9             = 13,
    CODECHAL_ENCODE_ME_FWD_REF_IDX4_CM_G9          = 14,
    CODECHAL_ENCODE_ME_RESERVED5_CM_G9             = 15,
    CODECHAL_ENCODE_ME_FWD_REF_IDX5_CM_G9          = 16,
    CODECHAL_ENCODE_ME_RESERVED6_CM_G9             = 17,
    CODECHAL_ENCODE_ME_FWD_REF_IDX6_CM_G9          = 18,
    CODECHAL_ENCODE_ME_RESERVED7_CM_G9             = 19,
    CODECHAL_ENCODE_ME_FWD_REF_IDX7_CM_G9          = 20,
    CODECHAL_ENCODE_ME_RESERVED8_CM_G9             = 21,
    CODECHAL_ENCODE_ME_CURR_FOR_BWD_REF_CM_G9      = 22,
    CODECHAL_ENCODE_ME_BWD_REF_IDX0_CM_G9          = 23,
    CODECHAL_ENCODE_ME_RESERVED9_CM_G9             = 24,
    CODECHAL_ENCODE_ME_BWD_REF_IDX1_CM_G9          = 25,
    CODECHAL_ENCODE_ME_VDENC_STREAMIN_CM_G9        = 26,
    CODECHAL_ENCODE_ME_NUM_SURFACES_CM_G9          = 27
} CODECHAL_ENCODE_BINDING_TABLE_OFFSET_ME_CM_G9;

// Defined in "Gen9 AVC Encoder Kernel" for Scaling Kernel
typedef struct _MEDIA_OBJECT_SCALING_STATIC_DATA_G9
{
    // DWORD 0 - GRF R1.0
    union
    {
        struct
        {
            uint32_t   InputPictureWidth                   : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   InputPictureHeight                  : MOS_BITFIELD_RANGE( 16,31 );
        };

        uint32_t       Value;
    } DW0;

    // DW1
    union
    {
        struct
        {
            uint32_t   InputYBTIFrame                      : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   InputYBTITopField                   : MOS_BITFIELD_RANGE(  0,31 );
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
            uint32_t   OutputYBTIFrame                     : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   OutputYBTITopField                  : MOS_BITFIELD_RANGE(  0,31 );
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
            uint32_t   InputYBTIBottomField                : MOS_BITFIELD_RANGE(  0,31 );
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
            uint32_t   OutputYBTIBottomField               : MOS_BITFIELD_RANGE(  0,31 );
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
            uint32_t   FlatnessThreshold                   : MOS_BITFIELD_RANGE(  0,31 );
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
            uint32_t   EnableMBFlatnessCheck               : MOS_BITFIELD_BIT(0);
            uint32_t   EnableMBVarianceOutput              : MOS_BITFIELD_BIT(1);
            uint32_t   EnableMBPixelAverageOutput          : MOS_BITFIELD_BIT(2);
            uint32_t   EnableBlock8x8StatisticsOutput      : MOS_BITFIELD_BIT(3);
            uint32_t                                       : MOS_BITFIELD_RANGE(4, 31);
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
            uint32_t   Reserved                            : MOS_BITFIELD_RANGE(1, 31);
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
            uint32_t   MBVProcStatsBTIFrame                : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   MBVProcStatsBTITopField             : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   MBVProcStatsBTIBottomField          : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   Reserved                            : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   Reserved                            : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };

    } DW11;

    //DW12
    union
    {
        struct
        {
            uint32_t   Reserved                            : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW12;

    //DW13
    union
    {
        struct
        {
            uint32_t   Reserved                            : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW13;

    //DW14
    union
    {
        struct
        {
            uint32_t   Reserved                            : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW14;

    //DW15
    union
    {
        struct
        {
            uint32_t   Reserved                            : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW15;

} MEDIA_OBJECT_SCALING_STATIC_DATA_G9, *PMEDIA_OBJECT_SCALING_STATIC_DATA_G9;

C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(MEDIA_OBJECT_SCALING_STATIC_DATA_G9)) == 16);

// Downscaling 2x kernels for Ultra HME
typedef struct _MEDIA_OBJECT_DOWNSCALING_2X_STATIC_DATA_G9
{
    union {
        struct {
            uint32_t       PicWidth                         : MOS_BITFIELD_RANGE(  0, 15 );   
            uint32_t       PicHeight                        : MOS_BITFIELD_RANGE(  16, 31 ); 
        };
        uint32_t Value;
    } DW0;

    union {
        struct {
            uint32_t       Reserved;   
        };
        uint32_t Value;
    } DW1;

    union {
        struct {
            uint32_t       Reserved;   
        };
        uint32_t Value;
    } DW2;

    union {
        struct {
            uint32_t       Reserved;   
        };
        uint32_t Value;
    } DW3;

    union {
        struct {
            uint32_t       Reserved;   
        };
        uint32_t Value;
    } DW4;

    union {
        struct {
            uint32_t       Reserved;   
        };
        uint32_t Value;
    } DW5;

    union {
        struct {
            uint32_t       Reserved;   
        };
        uint32_t Value;
    } DW6;

    union {
        struct {
            uint32_t       Reserved;   
        };
        uint32_t Value;
    } DW7;

    union {
        struct {
            uint32_t       BTI_Src_Y;   
        };
        uint32_t Value;
    } DW8;

    union {
        struct {
            uint32_t       BTI_Dst_Y;   
        };
        uint32_t Value;
    } DW9;
}MEDIA_OBJECT_DOWNSCALING_2X_STATIC_DATA_G9, *PMEDIA_OBJECT_DOWNSCALING_2X_STATIC_DATA_G9;

#endif  // __CODECHAL_ENCODER_G9_H__
