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
//! \file     codechal_encode_avc_g8.h
//! \brief    This file defines the C++ class/interface for Gen8 platform's AVC 
//!           DualPipe encoding to be used across CODECHAL components.
//!

#ifndef __CODECHAL_ENCODE_AVC_G8_H__
#define __CODECHAL_ENCODE_AVC_G8_H__

#include "codechal_encode_avc.h"

typedef enum _WP_BINDING_TABLE_OFFSET
{
    WP_INPUT_REF_SURFACE                 = 0,
    WP_OUTPUT_SCALED_SURFACE             = 1,
    WP_NUM_SURFACES                      = 2
} WP_BINDING_TABLE_OFFSET;

typedef enum _ME_BINDING_TABLE_OFFSET_CM
{
    ME_MV_DATA_SURFACE_CM       = 0,
    ME_16x_MV_DATA_SURFACE_CM   = 1,
    ME_32x_MV_DATA_SURFACE_CM   = 1,
    ME_DISTORTION_SURFACE_CM    = 2,
    ME_BRC_DISTORTION_CM        = 3,
    ME_RESERVED0_CM             = 4,
    ME_CURR_FOR_FWD_REF_CM      = 5,
    ME_FWD_REF_IDX0_CM          = 6,
    ME_RESERVED1_CM             = 7,
    ME_FWD_REF_IDX1_CM          = 8,
    ME_RESERVED2_CM             = 9,
    ME_FWD_REF_IDX2_CM          = 10,
    ME_RESERVED3_CM             = 11,
    ME_FWD_REF_IDX3_CM          = 12,
    ME_RESERVED4_CM             = 13,
    ME_FWD_REF_IDX4_CM          = 14,
    ME_RESERVED5_CM             = 15,
    ME_FWD_REF_IDX5_CM          = 16,
    ME_RESERVED6_CM             = 17,
    ME_FWD_REF_IDX6_CM          = 18,
    ME_RESERVED7_CM             = 19,
    ME_FWD_REF_IDX7_CM          = 20,
    ME_RESERVED8_CM             = 21,
    ME_CURR_FOR_BWD_REF_CM      = 22,
    ME_BWD_REF_IDX0_CM          = 23,
    ME_RESERVED9_CM             = 24,
    ME_BWD_REF_IDX1_CM          = 25,
    ME_RESERVED10_CM            = 26,
    ME_NUM_SURFACES_CM          = 27
} ME_DING_TABLE_OFFSET_CM;

typedef enum _BINDING_TABLE_OFFSET_MBENC_CM
{
    MBENC_MFC_AVC_PAK_OBJ_CM                    =  0,
    MBENC_IND_MV_DATA_CM                        =  1,
    MBENC_BRC_DISTORTION_CM                     =  2,    // For BRC distortion for I
    MBENC_CURR_Y_CM                             =  3,
    MBENC_CURR_UV_CM                            =  4,
    MBENC_MB_SPECIFIC_DATA_CM                   =  5,
    MBENC_AUX_VME_OUT_CM                        =  6,
    MBENC_REFPICSELECT_L0_CM                    =  7,
    MBENC_MV_DATA_FROM_ME_CM                    =  8,
    MBENC_4xME_DISTORTION_CM                    =  9,
    MBENC_SLICEMAP_DATA_CM                      = 10,
    MBENC_FWD_MB_DATA_CM                        = 11,
    MBENC_FWD_MV_DATA_CM                        = 12,
    MBENC_MBQP_CM                               = 13,
    MBENC_MBBRC_CONST_DATA_CM                   = 14,
    MBENC_VME_INTER_PRED_CURR_PIC_IDX_0_CM      = 15,
    MBENC_VME_INTER_PRED_FWD_PIC_IDX0_CM        = 16,
    MBENC_VME_INTER_PRED_BWD_PIC_IDX0_0_CM      = 17,
    MBENC_VME_INTER_PRED_FWD_PIC_IDX1_CM        = 18,
    MBENC_VME_INTER_PRED_BWD_PIC_IDX1_0_CM      = 19,
    MBENC_VME_INTER_PRED_FWD_PIC_IDX2_CM        = 20,
    MBENC_RESERVED0_CM                          = 21,
    MBENC_VME_INTER_PRED_FWD_PIC_IDX3_CM        = 22,
    MBENC_RESERVED1_CM                          = 23,
    MBENC_VME_INTER_PRED_FWD_PIC_IDX4_CM        = 24,
    MBENC_RESERVED2_CM                          = 25,
    MBENC_VME_INTER_PRED_FWD_PIC_IDX5_CM        = 26,
    MBENC_RESERVED3_CM                          = 27,
    MBENC_VME_INTER_PRED_FWD_PIC_IDX6_CM        = 28,
    MBENC_RESERVED4_CM                          = 29,
    MBENC_VME_INTER_PRED_FWD_PIC_IDX7_CM        = 30,
    MBENC_RESERVED5_CM                          = 31,
    MBENC_VME_INTER_PRED_CURR_PIC_IDX_1_CM      = 32,
    MBENC_VME_INTER_PRED_BWD_PIC_IDX0_1_CM      = 33,
    MBENC_RESERVED6_CM                          = 34,
    MBENC_VME_INTER_PRED_BWD_PIC_IDX1_1_CM      = 35,
    MBENC_RESERVED7_CM                          = 36,
    MBENC_FLATNESS_CHECK_CM                     = 37,
    MBENC_MAD_DATA_CM                           = 38,
    MBENC_FORCE_NONSKIP_MB_MAP_CM               = 39,
    MBENC_ADV_WA_DATA_CM                        = 40,
    MBENC_BRC_CURBE_DATA_CM                     = 41,
    MBENC_STATIC_FRAME_DETECTION_OUTPUT_CM      = 42,
    MBENC_NUM_SURFACES_CM                       = 43
} BINDING_TABLE_OFFSET_MBENC_CM;

// AVC Gen 8 WP kernel CURBE
typedef struct _WP_CURBE
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
            uint32_t   Log2WeightDenom      : MOS_BITFIELD_RANGE(  0,2  );
            uint32_t   reserve1             : MOS_BITFIELD_RANGE(  3,7  );
            uint32_t   ROI_enabled          : MOS_BITFIELD_RANGE(  8,8  );
            uint32_t   reserve2             : MOS_BITFIELD_RANGE(  9,31 );
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
            uint32_t   InputSurface          : MOS_BITFIELD_RANGE(  0,31 );
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
            uint32_t   OutputSurface         : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW51;

} WP_CURBE, *PWP_CURBE;

C_ASSERT(SIZE32(WP_CURBE) == 52);

struct CodechalEncodeAvcEncG8 : public CodechalEncodeAvcEnc
{
    static const uint32_t m_mbencNumTargetUsagesCommon = 3;
    static const uint32_t m_brcConstantSurfaceEarlySkipTableSizeCommon = 128;
    static const uint32_t m_brcConstantSurfaceRefCostSizeCommon = 128;
    static const uint32_t m_brcConstantSurfaceModeMvCosttSizeCommon = 1664;

    static const uint32_t m_mbencCurbeSizeInDword = 96;
    static const uint32_t m_mbencNumTargetUsagesCm = 3;
    static const uint32_t m_hmeFirstStep           = 0;
    
    static const uint32_t m_hmeFollowingStep = 1;
    static const uint32_t m_mvShiftFactor32x = 1;
    static const uint32_t m_mvShiftFactor16x = 2;
    static const uint32_t m_mvShiftFactor4x  = 2;
    static const uint32_t m_prevMvReadPosition16x = 1;
    static const uint32_t m_prevMvReadPosition8x = 0;
    
    // BRC Constant Surface
    static const uint32_t m_brcConstantSurfaceQpList0 = 32;
    static const uint32_t m_brcConstantSurfaceQpList0Reserved = 32;
    static const uint32_t m_brcConstantSurfaceQpList1 = 32;
    static const uint32_t m_brcConstantSurfaceQpList1Reserved = 160;
    
    static const uint32_t m_brcConstantSurfaceWidth = 64;
    static const uint32_t m_brcConstantSurfaceHeight = 44;
    
    static const uint32_t m_initBrcHistoryBufferSize  = 864;
    static const uint32_t m_brcConstantSurfaceIntraCostScalingFactor = 64;
    
    static const uint32_t m_sfdOutputBufferSize = 128;
    
    static const uint32_t m_cabacZone0Threshold = 128;
    static const uint32_t m_cabacZone1Threshold = 384;
    static const uint32_t m_cabacZone2Threshold = 768;
    static const uint32_t m_cabacZone3Threshold = 65535;
    
    static const uint32_t m_cabacWaZone0IMinQp  = 10;
    static const uint32_t m_cabacWaZone1IMinQp  = 12;
    static const uint32_t m_cabacWaZone2IMinQp  = 14;
    static const uint32_t m_cabacWaZone3IMinQp  = 16;
    
    static const uint32_t m_cabacWaZone0PMinQp  = 4;
    static const uint32_t m_cabacWaZone1PMinQp  = 6;
    static const uint32_t m_cabacWaZone2PMinQp  = 8;
    static const uint32_t m_cabacWaZone3PMinQp  = 10;

    static const uint32_t m_initMBEncCurbeCmNormalIFrame[m_mbencCurbeSizeInDword];
    static const uint32_t m_initMBEncCurbeCmNormalIField[m_mbencCurbeSizeInDword];
    static const uint32_t m_initMBEncCurbeCmNormalPFrame[m_mbencCurbeSizeInDword];
    static const uint32_t m_initMBEncCurbeCmNormalPField[m_mbencCurbeSizeInDword];
    static const uint32_t m_initMBEncCurbeCmNormalBFrame[m_mbencCurbeSizeInDword];
    static const uint32_t m_initMBEncCurbeCmNormalBField[m_mbencCurbeSizeInDword];
    static const uint32_t m_initMBEncCurbeCmIFrameDist[m_mbencCurbeSizeInDword]; 
    static const uint32_t m_initMeCurbeCm[39];
    static const int32_t  m_brcBtCounts[CODECHAL_ENCODE_BRC_IDX_NUM];
    static const int32_t  m_brcCurbeSize[CODECHAL_ENCODE_BRC_IDX_NUM];
    static const uint32_t m_trellisQuantizationRounding[NUM_TARGET_USAGE_MODES];
    
      
    //!
    //! \brief    Constructor
    //!
    CodechalEncodeAvcEncG8(
        CodechalHwInterface *   hwInterface,
        CodechalDebugInterface *debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo);


    ~CodechalEncodeAvcEncG8();

    static MOS_STATUS GetKernelHeaderAndSize(
        void                           *pvBinary,
        EncOperation                   operation,
		uint32_t                       krnStateIdx,
        void                           *krnHeader,
		uint32_t                       *krnSize);

    void UpdateSSDSliceCount();
    
    virtual MOS_STATUS InitMbBrcConstantDataBuffer(
        PCODECHAL_ENCODE_AVC_INIT_MBBRC_CONSTANT_DATA_BUFFER_PARAMS params);

    virtual MOS_STATUS InitializeState();


    //!
    //! \brief    Init ME kernel state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitKernelStateMe();
    
    virtual MOS_STATUS InitKernelStateMbEnc();

    virtual MOS_STATUS InitKernelStateWP();

    virtual MOS_STATUS InitKernelStateBrc();

    virtual MOS_STATUS GetMbEncKernelStateIdx(
        CodechalEncodeIdOffsetParams*   params,
        uint32_t*                       kernelOffset);

    virtual MOS_STATUS InitBrcConstantBuffer(PCODECHAL_ENCODE_AVC_INIT_BRC_CONSTANT_BUFFER_PARAMS params);

    virtual MOS_STATUS GetTrellisQuantization(
        PCODECHAL_ENCODE_AVC_TQ_INPUT_PARAMS    params,
        PCODECHAL_ENCODE_AVC_TQ_PARAMS          trellisQuantParams);

    virtual MOS_STATUS SetCurbeAvcWP(
        PCODECHAL_ENCODE_AVC_WP_CURBE_PARAMS params);

    virtual MOS_STATUS SetCurbeAvcMbEnc(
        PCODECHAL_ENCODE_AVC_MBENC_CURBE_PARAMS params);

    virtual MOS_STATUS SetCurbeAvcBrcInitReset(
        PCODECHAL_ENCODE_AVC_BRC_INIT_RESET_CURBE_PARAMS params);

    virtual MOS_STATUS SetCurbeAvcFrameBrcUpdate(
        PCODECHAL_ENCODE_AVC_BRC_UPDATE_CURBE_PARAMS params);

    virtual MOS_STATUS SendAvcBrcFrameUpdateSurfaces(
        PMOS_COMMAND_BUFFER pCmdBuffer,
        PCODECHAL_ENCODE_AVC_BRC_UPDATE_SURFACE_PARAMS params);

    virtual MOS_STATUS SetCurbeAvcBrcBlockCopy(
        PCODECHAL_ENCODE_AVC_BRC_BLOCK_COPY_CURBE_PARAMS params);

    virtual MOS_STATUS SendAvcMbEncSurfaces(
        PMOS_COMMAND_BUFFER pCmdBuffer,
        PCODECHAL_ENCODE_AVC_MBENC_SURFACE_PARAMS params);

    virtual MOS_STATUS SendAvcWPSurfaces(
        PMOS_COMMAND_BUFFER pCmdBuffer,
        PCODECHAL_ENCODE_AVC_WP_SURFACE_PARAMS params);

    virtual MOS_STATUS SendMeSurfaces(
        PMOS_COMMAND_BUFFER pCmdBuffer,
        MeSurfaceParams* params);
        
    virtual MOS_STATUS SetCurbeMe(MeCurbeParams* params);
    //!
    //! \brief    initial multi ref Qp BRC constant Buffer.
    //!
    //! \param    [in] params
    //!           BRC init constant buffer params.
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason   
    MOS_STATUS InitBrcConstantBufferMultiRefQP(PCODECHAL_ENCODE_AVC_INIT_BRC_CONSTANT_BUFFER_PARAMS params);

    
};

using PCodechalEncodeAvcEncG8       = CodechalEncodeAvcEncG8*;

#endif  // __CODECHAL_ENCODE_AVC_G8_H__
