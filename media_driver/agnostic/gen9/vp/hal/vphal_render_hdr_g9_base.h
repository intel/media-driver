/*
* Copyright (c) 2010-2019, Intel Corporation
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
//! \file      vphal_render_hdr_g9_base.h
//! \brief         Rendering definitions for Unified VP HAL HDR processing
//!
//!
//! \file     vphal_render_hdr_g9_base.h
//! \brief    Common interface and structure used in HDR for GEN9
//! \details  Common interface and structure used in HDR which are for for GEN9 platform
//!

#ifndef __VPHAL_RENDER_HDR_G9_BASE_H__
#define __VPHAL_RENDER_HDR_G9_BASE_H__

#define KERNEL_HDR_MANDATORY_G9 0
#define KERNEL_HDR_PREPROCESS_G9 4

#define VPHAL_HDR_BTINDEX_EOTF1DLUT_OFFSET_G9 3
#define VPHAL_HDR_BTINDEX_OETF1DLUT_OFFSET_G9 4
#define VPHAL_HDR_BTINDEX_CRI3DLUT_OFFSET_G9  3

#define VPHAL_HDR_BTINDEX_LAYER0_G9          16
#define VPHAL_HDR_BTINDEX_PER_LAYER0_G9      5
#define VPHAL_HDR_BTINDEX_RENDERTARGET_G9    56
#define VPHAL_HDR_BTINDEX_PER_TARGET_G9      3
#define VPHAL_HDR_BTINDEX_COEFF_G9     59

#define VPHAL_HDR_AVS_SAMPLER_STATE_NEAREST   1
#define VPHAL_HDR_AVS_SAMPLER_STATE_ADAPTIVE  3
#define VPHAL_HDR_3D_SAMPLER_STATE_NEAREST    13
#define VPHAL_HDR_3D_SAMPLER_STATE_BILINEAR   14

#define VPHAL_HDR_COEF_SURFACE_WIDTH_G9            8
#define VPHAL_HDR_COEF_SURFACE_HEIGHT_BASIC_G9     66
#define VPHAL_HDR_COEF_SURFACE_HEIGHT_EXT_G9       32
#define VPHAL_HDR_COEF_SURFACE_HEIGHT_G9           (VPHAL_HDR_COEF_SURFACE_HEIGHT_BASIC_G9 + VPHAL_HDR_COEF_SURFACE_HEIGHT_EXT_G9)
#define VPHAL_HDR_COEF_SURFACE_PITCH_G9            64
#define VPHAL_HDR_COEF_LINES_PER_LAYER_BASIC_G9    8
#define VPHAL_HDR_COEF_LINES_PER_LAYER_EXT_G9      4
#define VPHAL_HDR_COEF_EOTF_OFFSET                 6
#define VPHAL_HDR_COEF_PIVOT_POINT_LINE_OFFSET     6
#define VPHAL_HDR_COEF_SLOPE_INTERCEPT_LINE_OFFSET 7
#define VPHAL_HDR_COEF_CCMEXT_OFFSET               6
#define VPHAL_HDR_COEF_CLAMP_OFFSET                7

#define VPHAL_HDR_EOTF_COEFF1_TRADITIONNAL_GAMMA_G9 0.081f
#define VPHAL_HDR_EOTF_COEFF2_TRADITIONNAL_GAMMA_G9 (1.0f / 4.5f)
#define VPHAL_HDR_EOTF_COEFF3_TRADITIONNAL_GAMMA_G9 (1.0f / 1.099f)
#define VPHAL_HDR_EOTF_COEFF4_TRADITIONNAL_GAMMA_G9 (0.099f / 1.099f)
#define VPHAL_HDR_EOTF_COEFF5_TRADITIONNAL_GAMMA_G9 (1.0f / 0.45f)

#define VPHAL_HDR_OETF_COEFF1_TRADITIONNAL_GAMMA_G9 0.018f
#define VPHAL_HDR_OETF_COEFF2_TRADITIONNAL_GAMMA_G9 4.5f
#define VPHAL_HDR_OETF_COEFF3_TRADITIONNAL_GAMMA_G9 1.099f
#define VPHAL_HDR_OETF_COEFF4_TRADITIONNAL_GAMMA_G9 (-0.099f)
#define VPHAL_HDR_OETF_COEFF5_TRADITIONNAL_GAMMA_G9 0.45f

#define VPHAL_HDR_EOTF_COEFF1_TRADITIONNAL_GAMMA_BT1886_G9 (-0.0f)
#define VPHAL_HDR_EOTF_COEFF2_TRADITIONNAL_GAMMA_BT1886_G9 0.0f
#define VPHAL_HDR_EOTF_COEFF3_TRADITIONNAL_GAMMA_BT1886_G9 1.0f
#define VPHAL_HDR_EOTF_COEFF4_TRADITIONNAL_GAMMA_BT1886_G9 0.0f
#define VPHAL_HDR_EOTF_COEFF5_TRADITIONNAL_GAMMA_BT1886_G9 2.4f

#define VPHAL_HDR_EOTF_COEFF1_TRADITIONNAL_GAMMA_SRGB_G9 0.04045f
#define VPHAL_HDR_EOTF_COEFF2_TRADITIONNAL_GAMMA_SRGB_G9 (1.0f / 12.92f)
#define VPHAL_HDR_EOTF_COEFF3_TRADITIONNAL_GAMMA_SRGB_G9 (1.0f / 1.055f)
#define VPHAL_HDR_EOTF_COEFF4_TRADITIONNAL_GAMMA_SRGB_G9 (0.055f / 1.055f)
#define VPHAL_HDR_EOTF_COEFF5_TRADITIONNAL_GAMMA_SRGB_G9 2.4f

#define VPHAL_HDR_OETF_COEFF1_TRADITIONNAL_GAMMA_SRGB_G9 0.0031308f
#define VPHAL_HDR_OETF_COEFF2_TRADITIONNAL_GAMMA_SRGB_G9 12.92f
#define VPHAL_HDR_OETF_COEFF3_TRADITIONNAL_GAMMA_SRGB_G9 1.055f
#define VPHAL_HDR_OETF_COEFF4_TRADITIONNAL_GAMMA_SRGB_G9 (-0.055f)
#define VPHAL_HDR_OETF_COEFF5_TRADITIONNAL_GAMMA_SRGB_G9 (1.0f / 2.4f)

#define VPHAL_HDR_EOTF_COEFF1_SMPTE_ST2084_G9 -0.8359375f
#define VPHAL_HDR_EOTF_COEFF2_SMPTE_ST2084_G9 18.8515625f
#define VPHAL_HDR_EOTF_COEFF3_SMPTE_ST2084_G9 -18.6875f
#define VPHAL_HDR_EOTF_COEFF4_SMPTE_ST2084_G9 6.277394636015326f
#define VPHAL_HDR_EOTF_COEFF5_SMPTE_ST2084_G9 0.012683313515656f

#define VPHAL_HDR_OETF_COEFF1_SMPTE_ST2084_G9 0.8359375f
#define VPHAL_HDR_OETF_COEFF2_SMPTE_ST2084_G9 18.8515625f
#define VPHAL_HDR_OETF_COEFF3_SMPTE_ST2084_G9 18.6875f
#define VPHAL_HDR_OETF_COEFF4_SMPTE_ST2084_G9 0.1593017578125f
#define VPHAL_HDR_OETF_COEFF5_SMPTE_ST2084_G9 78.84375f

#define HDR_MANDATORY_KERNEL_BLOCK_WIDTH  16
#define HDR_MANDATORY_KERNEL_BLOCK_HEIGHT 8

#define MAX_CSC_COEFF_VAL_ICL 3.9921875  // (4.0 * 511.0 / 512.0)
#define ARRAY_SIZE(a)  (sizeof(a) / sizeof(a[0]))

typedef struct
{
    uint32_t reserved : 2;
    uint32_t mantissa : 9;
    uint32_t exponent : 3;
    uint32_t sign : 1;
} CSC_COEFF_FORMAT;

extern CSC_COEFF_FORMAT Convert_CSC_Coeff_To_Register_Format(double coeff);
extern double Convert_CSC_Coeff_Register_Format_To_Double(CSC_COEFF_FORMAT regVal);
extern float LimitFP32PrecisionToF3_9(float fp);
extern void LimitFP32ArrayPrecisionToF3_9(float fps[], size_t size);

#if __cplusplus
extern "C"
{
#endif // __cplusplus

//!
//! \brief    Gen9 Hdr kernel params
//!
const RENDERHAL_KERNEL_PARAM g_Hdr_KernelParam_g9[KERNEL_HDR_MAX] =
{
    /*    GRF_Count
        |  BT_Count
        |  |    Sampler_Count
        |  |    |  Thread_Count
        |  |    |  |                            GRF_Start_Register
        |  |    |  |                            |   CURBE_Length (in 256-bit blocks)
        |  |    |  |                            |   |   block_width
        |  |    |  |                            |   |   |     block_height
        |  |    |  |                            |   |   |     |   blocks_x
        |  |    |  |                            |   |   |     |   |   blocks_y
        |  |    |  |                            |   |   |     |   |   |*/
    {8, 40, 4, VPHAL_USE_MEDIA_THREADS_MAX, 0, 8, 16, 8, 1, 1},
    {8, 40, 4, VPHAL_USE_MEDIA_THREADS_MAX, 0, 8, 16, 8, 1, 1},
    {8, 40, 4, VPHAL_USE_MEDIA_THREADS_MAX, 0, 4, 16, 8, 1, 1},
    {8, 40, 4, VPHAL_USE_MEDIA_THREADS_MAX, 0, 4, 16, 8, 1, 1},
    {8, 40, 0, VPHAL_USE_MEDIA_THREADS_MAX, 0, 4, 1, 1, 1, 1},
};

#define CONFIG_ENTRY_INITIALIZER(CCM, PWLF, CCMExt1, GamutClamp1, CCMExt2, GamutClamp2, Invalid) \
    ((CCM) | (PWLF) << 3 | (CCMExt1) << 6 | (GamutClamp1) << 9 | (CCMExt2) << 10 | (GamutClamp2) << 13 | (Invalid) << 15)

const uint16_t HDRStageConfigTable_g9[HDR_STAGES_CONFIG_TABLE_SIZE] =
{
    // CCM & CCMExt1 & CCMExt2 mode(should keep consistent with enum definition VPHAL_HDR_CCM_TYPE):
    // 0 - VPHAL_HDR_CCM_NONE
    // 1 - VPHAL_HDR_CCM_BT2020_TO_BT601_BT709_MATRIX
    // 2 - VPHAL_HDR_CCM_BT601_BT709_TO_BT2020_MATRIX
    // 3 - VPHAL_HDR_CCM_BT2020_TO_MONITOR_MATRIX
    // 4 - VPHAL_HDR_CCM_MONITOR_TO_BT2020_MATRIX
    // 5 - VPHAL_HDR_CCM_MONITOR_TO_BT709_MATRIX
    //
    // PWLF mode(should keep consistent with enum definition VPHAL_HDR_MODE):
    // 0 - VPHAL_HDR_MODE_NONE
    // 1 - VPHAL_HDR_MODE_TONE_MAPPING
    // 2 - VPHAL_HDR_MODE_INVERSE_TONE_MAPPING
    // 3 - VPHAL_HDR_MODE_H2H
    // 4 - VPHAL_HDR_MODE_S2S
    //
    //               Result: CCM  PWLF  CCMExt1  GamutClamp1 CCMExt2  GamutClamp2 Invalid   Case id: OutputLinear OutputGamut OutputXDR InputGamut InputXDR
    CONFIG_ENTRY_INITIALIZER(0, 0, 0, 0, 0, 0, 0),  //           0           0           0         0         0
    CONFIG_ENTRY_INITIALIZER(2, 1, 1, 0, 0, 0, 0),  //           0           0           0         0         1
    CONFIG_ENTRY_INITIALIZER(1, 0, 0, 0, 0, 0, 0),  //           0           0           0         1         0
    CONFIG_ENTRY_INITIALIZER(0, 1, 1, 0, 0, 0, 0),  //           0           0           0         1         1
    CONFIG_ENTRY_INITIALIZER(0, 0, 0, 0, 0, 0, 1),  //           0           0           1         0         0
    CONFIG_ENTRY_INITIALIZER(0, 0, 0, 0, 0, 0, 1),  //           0           0           1         0         1
    CONFIG_ENTRY_INITIALIZER(0, 0, 0, 0, 0, 0, 1),  //           0           0           1         1         0
    CONFIG_ENTRY_INITIALIZER(0, 0, 0, 0, 0, 0, 1),  //           0           0           1         1         1
    CONFIG_ENTRY_INITIALIZER(2, 0, 0, 0, 0, 0, 0),  //           0           1           0         0         0
    CONFIG_ENTRY_INITIALIZER(2, 1, 0, 0, 0, 0, 0),  //           0           1           0         0         1
    CONFIG_ENTRY_INITIALIZER(0, 0, 0, 0, 0, 0, 0),  //           0           1           0         1         0
    CONFIG_ENTRY_INITIALIZER(0, 1, 0, 0, 0, 0, 0),  //           0           1           0         1         1
    CONFIG_ENTRY_INITIALIZER(0, 2, 2, 0, 0, 0, 0),  //           0           1           1         0         0
    CONFIG_ENTRY_INITIALIZER(2, 3, 3, 1, 4, 0, 0),  //           0           1           1         0         1
    CONFIG_ENTRY_INITIALIZER(0, 2, 3, 1, 4, 0, 0),  //           0           1           1         1         0
    CONFIG_ENTRY_INITIALIZER(0, 3, 0, 0, 0, 0, 0),  //           0           1           1         1         1
    CONFIG_ENTRY_INITIALIZER(0, 0, 0, 0, 0, 0, 0),  //           1           0           0         0         0
    CONFIG_ENTRY_INITIALIZER(2, 1, 1, 1, 0, 0, 0),  //           1           0           0         0         1
    CONFIG_ENTRY_INITIALIZER(1, 0, 0, 0, 0, 0, 0),  //           1           0           0         1         0
    CONFIG_ENTRY_INITIALIZER(0, 1, 1, 1, 0, 0, 0),  //           1           0           0         1         1
    CONFIG_ENTRY_INITIALIZER(0, 2, 0, 0, 0, 0, 0),  //           1           0           1         0         0
    CONFIG_ENTRY_INITIALIZER(2, 3, 3, 1, 5, 0, 0),  //           1           0           1         0         1
    CONFIG_ENTRY_INITIALIZER(0, 2, 3, 1, 5, 0, 0),  //           1           0           1         1         0
    CONFIG_ENTRY_INITIALIZER(0, 3, 5, 0, 0, 0, 0),  //           1           0           1         1         1
    CONFIG_ENTRY_INITIALIZER(0, 0, 0, 0, 0, 0, 1),  //           1           1           0         0         0
    CONFIG_ENTRY_INITIALIZER(0, 0, 0, 0, 0, 0, 1),  //           1           1           0         0         1
    CONFIG_ENTRY_INITIALIZER(0, 0, 0, 0, 0, 0, 1),  //           1           1           0         1         0
    CONFIG_ENTRY_INITIALIZER(0, 0, 0, 0, 0, 0, 1),  //           1           1           0         1         1
    CONFIG_ENTRY_INITIALIZER(0, 0, 0, 0, 0, 0, 1),  //           1           1           1         0         0
    CONFIG_ENTRY_INITIALIZER(0, 0, 0, 0, 0, 0, 1),  //           1           1           1         0         1
    CONFIG_ENTRY_INITIALIZER(0, 0, 0, 0, 0, 0, 1),  //           1           1           1         1         0
    CONFIG_ENTRY_INITIALIZER(0, 0, 0, 0, 0, 0, 1)   //           1           1           1         1         1
};

// Static Data for Gen9 HDR PreProcess kernel
typedef struct _MEDIA_WALKER_HDR_PREPROCESS_STATIC_DATA_G9
{
    // uint32_t 0-7 - GRF R1.0-R1.7
    uint32_t uiTMMode[VPHAL_MAX_HDR_INPUT_LAYER] = { 0 };
    // uint32_t 8-15 - GRF R2.0-R2.7
    uint32_t uiMaxCLL[VPHAL_MAX_HDR_INPUT_LAYER] = { 0 };
    // uint32_t 16-23 - GRF R3.0-R3.7
    uint32_t uiMaxDLL[VPHAL_MAX_HDR_INPUT_LAYER] = { 0 };
    // uint32_t 16
    uint32_t OutputCoeffIndex = 0;
}MEDIA_WALKER_HDR_PREPROCESS_STATIC_DATA_G9, *PMEDIA_WALKER_HDR_PREPROCESS_STATIC_DATA_G9;
C_ASSERT(SIZE32(MEDIA_WALKER_HDR_PREPROCESS_STATIC_DATA_G9) == 25);

// Static Data for Gen9 HDR kernel
typedef struct _MEDIA_WALKER_HDR_STATIC_DATA_G9
{
    // uint32_t 0 - GRF R1.0
    union
    {
        struct {
            float HorizontalFrameOriginLayer0;
        };
        float       Value;
    } DW0;

    // uint32_t 1 - GRF R1.1
    union
    {
        struct {
            float HorizontalFrameOriginLayer1;
        };
        float       Value;
    } DW1;

    // uint32_t 2 - GRF R1.2
    union
    {
        struct {
            float HorizontalFrameOriginLayer2;
        };
        float       Value;
    } DW2;

    // uint32_t 3 - GRF R1.3
    union
    {
        struct {
            float HorizontalFrameOriginLayer3;
        };
        float       Value;
    } DW3;

    // uint32_t 4 - GRF R1.4
    union
    {
        struct {
            float HorizontalFrameOriginLayer4;
        };
        float       Value;
    } DW4;

    // uint32_t 5 - GRF R1.5
    union
    {
        struct {
            float HorizontalFrameOriginLayer5;
        };
        float       Value;
    } DW5;

    // uint32_t 6 - GRF R1.6
    union
    {
        struct {
            float HorizontalFrameOriginLayer6;
        };
        float       Value;
    } DW6;

    // uint32_t 7 - GRF R1.7
    union
    {
        struct {
            float HorizontalFrameOriginLayer7;
        };
        float       Value;
    } DW7;

    // uint32_t 8 - GRF R2.0
    union
    {
        struct {
            float VerticalFrameOriginLayer0;
        };
        float       Value;
    } DW8;

    // uint32_t 9 - GRF R2.1
    union
    {
        struct {
            float VerticalFrameOriginLayer1;
        };
        float       Value;
    } DW9;

    // uint32_t 10 - GRF R2.2
    union
    {
        struct {
            float VerticalFrameOriginLayer2;
        };
        float       Value;
    } DW10;

    // uint32_t 11 - GRF R2.3
    union
    {
        struct {
            float VerticalFrameOriginLayer3;
        };
        float       Value;
    } DW11;

    // uint32_t 12 - GRF R2.4
    union
    {
        struct {
            float VerticalFrameOriginLayer4;
        };
        float       Value;
    } DW12;

    // uint32_t 13 - GRF R2.5
    union
    {
        struct {
            float VerticalFrameOriginLayer5;
        };
        float       Value;
    } DW13;

    // uint32_t 14 - GRF R2.6
    union
    {
        struct {
            float VerticalFrameOriginLayer6;
        };
        float       Value;
    } DW14;

    // uint32_t 15 - GRF R2.7
    union
    {
        struct {
            float VerticalFrameOriginLayer7;
        };
        float       Value;
    } DW15;

    // uint32_t 16 - GRF R3.0
    union
    {
        struct {
            float HorizontalScalingStepRatioLayer0;
        };
        float       Value;
    } DW16;

    // uint32_t 17 - GRF R3.1
    union
    {
        struct {
            float HorizontalScalingStepRatioLayer1;
        };
        float       Value;
    } DW17;

    // uint32_t 18 - GRF R3.2
    union
    {
        struct {
            float HorizontalScalingStepRatioLayer2;
        };
        float       Value;
    } DW18;

    // uint32_t 19 - GRF R3.3
    union
    {
        struct {
            float HorizontalScalingStepRatioLayer3;
        };
        float       Value;
    } DW19;

    // uint32_t 20 - GRF R3.4
    union
    {
        struct {
            float HorizontalScalingStepRatioLayer4;
        };
        float       Value;
    } DW20;

    // uint32_t 21 - GRF R3.5
    union
    {
        struct {
            float HorizontalScalingStepRatioLayer5;
        };
        float       Value;
    } DW21;

    // uint32_t 22 - GRF R3.6
    union
    {
        struct {
            float HorizontalScalingStepRatioLayer6;
        };
        float       Value;
    } DW22;

    // uint32_t 23 - GRF R3.7
    union
    {
        struct {
            float HorizontalScalingStepRatioLayer7;
        };
        float       Value;
    } DW23;

    // uint32_t 24 - GRF R4.0
    union
    {
        struct {
            float VerticalScalingStepRatioLayer0;
        };
        float       Value;
    } DW24;

    // uint32_t 25 - GRF R4.1
    union
    {
        struct {
            float VerticalScalingStepRatioLayer1;
        };
        float       Value;
    } DW25;

    // uint32_t 26 - GRF R4.2
    union
    {
        struct {
            float VerticalScalingStepRatioLayer2;
        };
        float       Value;
    } DW26;

    // uint32_t 27 - GRF R4.3
    union
    {
        struct {
            float VerticalScalingStepRatioLayer3;
        };
        float       Value;
    } DW27;

    // uint32_t 28 - GRF R4.4
    union
    {
        struct {
            float VerticalScalingStepRatioLayer4;
        };
        float       Value;
    } DW28;

    // uint32_t 29 - GRF R4.5
    union
    {
        struct {
            float VerticalScalingStepRatioLayer5;
        };
        float       Value;
    } DW29;

    // uint32_t 30 - GRF R4.6
    union
    {
        struct {
            float VerticalScalingStepRatioLayer6;
        };
        float       Value;
    } DW30;

    // uint32_t 31 - GRF R4.7
    union
    {
        struct {
            float VerticalScalingStepRatioLayer7;
        };
        float       Value;
    } DW31;

    // uint32_t 32 - GRF R5.0
    union
    {
        struct {
            uint32_t LeftCoordinateRectangleLayer0 : BITFIELD_RANGE(  0,15 );
            uint32_t TopCoordinateRectangleLayer0  : BITFIELD_RANGE( 16,31 );
        };
        uint32_t       Value;
    } DW32;

    // uint32_t 33 - GRF R5.1
    union
    {
        struct {
            uint32_t LeftCoordinateRectangleLayer1 : BITFIELD_RANGE(  0,15 );
            uint32_t TopCoordinateRectangleLayer1  : BITFIELD_RANGE( 16,31 );
        };
        uint32_t       Value;
    } DW33;

    // uint32_t 34 - GRF R5.2
    union
    {
        struct {
            uint32_t LeftCoordinateRectangleLayer2 : BITFIELD_RANGE(  0,15 );
            uint32_t TopCoordinateRectangleLayer2  : BITFIELD_RANGE( 16,31 );
        };
        uint32_t       Value;
    } DW34;

    // uint32_t 35 - GRF R5.3
    union
    {
        struct {
            uint32_t LeftCoordinateRectangleLayer3 : BITFIELD_RANGE(  0,15 );
            uint32_t TopCoordinateRectangleLayer3  : BITFIELD_RANGE( 16,31 );
        };
        uint32_t       Value;
    } DW35;

    // uint32_t 36 - GRF R5.4
    union
    {
        struct {
            uint32_t LeftCoordinateRectangleLayer4 : BITFIELD_RANGE(  0,15 );
            uint32_t TopCoordinateRectangleLayer4  : BITFIELD_RANGE( 16,31 );
        };
        uint32_t       Value;
    } DW36;

    // uint32_t 37 - GRF R5.5
    union
    {
        struct {
            uint32_t LeftCoordinateRectangleLayer5 : BITFIELD_RANGE(  0,15 );
            uint32_t TopCoordinateRectangleLayer5  : BITFIELD_RANGE( 16,31 );
        };
        uint32_t       Value;
    } DW37;

    // uint32_t 38 - GRF R5.6
    union
    {
        struct {
            uint32_t LeftCoordinateRectangleLayer6 : BITFIELD_RANGE(  0,15 );
            uint32_t TopCoordinateRectangleLayer6  : BITFIELD_RANGE( 16,31 );
        };
        uint32_t       Value;
    } DW38;

    // uint32_t 39 - GRF R5.7
    union
    {
        struct {
            uint32_t LeftCoordinateRectangleLayer7 : BITFIELD_RANGE(  0,15 );
            uint32_t TopCoordinateRectangleLayer7  : BITFIELD_RANGE( 16,31 );
        };
        uint32_t       Value;
    } DW39;

    // uint32_t 40 - GRF R6.0
    union
    {
        struct {
            uint32_t RightCoordinateRectangleLayer0  : BITFIELD_RANGE(  0,15 );
            uint32_t BottomCoordinateRectangleLayer0 : BITFIELD_RANGE( 16,31 );
        };
        uint32_t       Value;
    } DW40;

    // uint32_t 41 - GRF R6.1
    union
    {
        struct {
            uint32_t RightCoordinateRectangleLayer1  : BITFIELD_RANGE(  0,15 );
            uint32_t BottomCoordinateRectangleLayer1 : BITFIELD_RANGE( 16,31 );
        };
        uint32_t       Value;
    } DW41;

    // uint32_t 42 - GRF R6.2
    union
    {
        struct {
            uint32_t RightCoordinateRectangleLayer2  : BITFIELD_RANGE(  0,15 );
            uint32_t BottomCoordinateRectangleLayer2 : BITFIELD_RANGE( 16,31 );
        };
        uint32_t       Value;
    } DW42;

    // uint32_t 43 - GRF R6.3
    union
    {
        struct {
            uint32_t RightCoordinateRectangleLayer3  : BITFIELD_RANGE(  0,15 );
            uint32_t BottomCoordinateRectangleLayer3 : BITFIELD_RANGE( 16,31 );
        };
        uint32_t       Value;
    } DW43;

    // uint32_t 44 - GRF R6.4
    union
    {
        struct {
            uint32_t RightCoordinateRectangleLayer4  : BITFIELD_RANGE(  0,15 );
            uint32_t BottomCoordinateRectangleLayer4 : BITFIELD_RANGE( 16,31 );
        };
        uint32_t       Value;
    } DW44;

    // uint32_t 45 - GRF R6.5
    union
    {
        struct {
            uint32_t RightCoordinateRectangleLayer5  : BITFIELD_RANGE(  0,15 );
            uint32_t BottomCoordinateRectangleLayer5 : BITFIELD_RANGE( 16,31 );
        };
        uint32_t       Value;
    } DW45;

    // uint32_t 46 - GRF R6.6
    union
    {
        struct {
            uint32_t RightCoordinateRectangleLayer6  : BITFIELD_RANGE(  0,15 );
            uint32_t BottomCoordinateRectangleLayer6 : BITFIELD_RANGE( 16,31 );
        };
        uint32_t       Value;
    } DW46;

    // uint32_t 47 - GRF R6.7
    union
    {
        struct {
            uint32_t RightCoordinateRectangleLayer7  : BITFIELD_RANGE(  0,15 );
            uint32_t BottomCoordinateRectangleLayer7 : BITFIELD_RANGE( 16,31 );
        };
        uint32_t       Value;
    } DW47;

    // uint32_t 48 - GRF R7.0
    union
    {
        struct {
            uint32_t FormatDescriptorLayer0                : BITFIELD_RANGE(  0,7  );
            uint32_t ChromaSittingLocationLayer0           : BITFIELD_RANGE(  8,10 );
            uint32_t ChannelSwapEnablingFlagLayer0         : BITFIELD_RANGE( 11,11 );
            uint32_t IEFBypassEnablingFlagLayer0           : BITFIELD_RANGE( 12,12 );
            uint32_t RotationAngleMirrorDirectionLayer0    : BITFIELD_RANGE( 13,15 );
            uint32_t SamplerIndexFirstPlaneLayer0          : BITFIELD_RANGE( 16,19 );
            uint32_t SamplerIndexSecondThirdPlaneLayer0    : BITFIELD_RANGE( 20,23 );
            uint32_t CCMExtensionEnablingFlagLayer0        : BITFIELD_RANGE( 24,24 );
            uint32_t ToneMappingEnablingFlagLayer0         : BITFIELD_RANGE( 25,25 );
            uint32_t PriorCSCEnablingFlagLayer0            : BITFIELD_RANGE( 26,26 );
            uint32_t EOTF1DLUTEnablingFlagLayer0           : BITFIELD_RANGE( 27,27 );
            uint32_t CCMEnablingFlagLayer0                 : BITFIELD_RANGE( 28,28 );
            uint32_t OETF1DLUTEnablingFlagLayer0           : BITFIELD_RANGE( 29,29 );
            uint32_t PostCSCEnablingFlagLayer0             : BITFIELD_RANGE( 30,30 );
            uint32_t Enabling3DLUTFlagLayer0               : BITFIELD_RANGE( 31,31 );
        };
        uint32_t       Value;
    } DW48;

    // uint32_t 49 - GRF R7.1
    union
    {
        struct {
            uint32_t FormatDescriptorLayer1                : BITFIELD_RANGE(  0,7  );
            uint32_t ChromaSittingLocationLayer1           : BITFIELD_RANGE(  8,10 );
            uint32_t ChannelSwapEnablingFlagLayer1         : BITFIELD_RANGE( 11,11 );
            uint32_t IEFBypassEnablingFlagLayer1           : BITFIELD_RANGE( 12,12 );
            uint32_t RotationAngleMirrorDirectionLayer1    : BITFIELD_RANGE( 13,15 );
            uint32_t SamplerIndexFirstPlaneLayer1          : BITFIELD_RANGE( 16,19 );
            uint32_t SamplerIndexSecondThirdPlaneLayer1    : BITFIELD_RANGE( 20,23 );
            uint32_t CCMExtensionEnablingFlagLayer1        : BITFIELD_RANGE( 24,24 );
            uint32_t ToneMappingEnablingFlagLayer1         : BITFIELD_RANGE( 25,25 );
            uint32_t PriorCSCEnablingFlagLayer1            : BITFIELD_RANGE( 26,26 );
            uint32_t EOTF1DLUTEnablingFlagLayer1           : BITFIELD_RANGE( 27,27 );
            uint32_t CCMEnablingFlagLayer1                 : BITFIELD_RANGE( 28,28 );
            uint32_t OETF1DLUTEnablingFlagLayer1           : BITFIELD_RANGE( 29,29 );
            uint32_t PostCSCEnablingFlagLayer1             : BITFIELD_RANGE( 30,30 );
            uint32_t Enabling3DLUTFlagLayer1               : BITFIELD_RANGE( 31,31 );
        };
        uint32_t       Value;
    } DW49;

    // uint32_t 50 - GRF R7.2
    union
    {
        struct {
            uint32_t FormatDescriptorLayer2                : BITFIELD_RANGE(  0,7  );
            uint32_t ChromaSittingLocationLayer2           : BITFIELD_RANGE(  8,10 );
            uint32_t ChannelSwapEnablingFlagLayer2         : BITFIELD_RANGE( 11,11 );
            uint32_t IEFBypassEnablingFlagLayer2           : BITFIELD_RANGE( 12,12 );
            uint32_t RotationAngleMirrorDirectionLayer2    : BITFIELD_RANGE( 13,15 );
            uint32_t SamplerIndexFirstPlaneLayer2          : BITFIELD_RANGE( 16,19 );
            uint32_t SamplerIndexSecondThirdPlaneLayer2    : BITFIELD_RANGE( 20,23 );
            uint32_t CCMExtensionEnablingFlagLayer2        : BITFIELD_RANGE( 24,24 );
            uint32_t ToneMappingEnablingFlagLayer2         : BITFIELD_RANGE( 25,25 );
            uint32_t PriorCSCEnablingFlagLayer2            : BITFIELD_RANGE( 26,26 );
            uint32_t EOTF1DLUTEnablingFlagLayer2           : BITFIELD_RANGE( 27,27 );
            uint32_t CCMEnablingFlagLayer2                 : BITFIELD_RANGE( 28,28 );
            uint32_t OETF1DLUTEnablingFlagLayer2           : BITFIELD_RANGE( 29,29 );
            uint32_t PostCSCEnablingFlagLayer2             : BITFIELD_RANGE( 30,30 );
            uint32_t Enabling3DLUTFlagLayer2               : BITFIELD_RANGE( 31,31 );
        };
        uint32_t       Value;
    } DW50;

    // uint32_t 51 - GRF R7.3
    union
    {
        struct {
            uint32_t FormatDescriptorLayer3                : BITFIELD_RANGE(  0,7  );
            uint32_t ChromaSittingLocationLayer3           : BITFIELD_RANGE(  8,10 );
            uint32_t ChannelSwapEnablingFlagLayer3         : BITFIELD_RANGE( 11,11 );
            uint32_t IEFBypassEnablingFlagLayer3           : BITFIELD_RANGE( 12,12 );
            uint32_t RotationAngleMirrorDirectionLayer3    : BITFIELD_RANGE( 13,15 );
            uint32_t SamplerIndexFirstPlaneLayer3          : BITFIELD_RANGE( 16,19 );
            uint32_t SamplerIndexSecondThirdPlaneLayer3    : BITFIELD_RANGE( 20,23 );
            uint32_t CCMExtensionEnablingFlagLayer3        : BITFIELD_RANGE( 24,24 );
            uint32_t ToneMappingEnablingFlagLayer3         : BITFIELD_RANGE( 25,25 );
            uint32_t PriorCSCEnablingFlagLayer3            : BITFIELD_RANGE( 26,26 );
            uint32_t EOTF1DLUTEnablingFlagLayer3           : BITFIELD_RANGE( 27,27 );
            uint32_t CCMEnablingFlagLayer3                 : BITFIELD_RANGE( 28,28 );
            uint32_t OETF1DLUTEnablingFlagLayer3           : BITFIELD_RANGE( 29,29 );
            uint32_t PostCSCEnablingFlagLayer3             : BITFIELD_RANGE( 30,30 );
            uint32_t Enabling3DLUTFlagLayer3               : BITFIELD_RANGE( 31,31 );
        };
        uint32_t       Value;
    } DW51;

    // uint32_t 52 - GRF R7.4
    union
    {
        struct {
            uint32_t FormatDescriptorLayer4                : BITFIELD_RANGE(  0,7  );
            uint32_t ChromaSittingLocationLayer4           : BITFIELD_RANGE(  8,10 );
            uint32_t ChannelSwapEnablingFlagLayer4         : BITFIELD_RANGE( 11,11 );
            uint32_t IEFBypassEnablingFlagLayer4           : BITFIELD_RANGE( 12,12 );
            uint32_t RotationAngleMirrorDirectionLayer4    : BITFIELD_RANGE( 13,15 );
            uint32_t SamplerIndexFirstPlaneLayer4          : BITFIELD_RANGE( 16,19 );
            uint32_t SamplerIndexSecondThirdPlaneLayer4    : BITFIELD_RANGE( 20,23 );
            uint32_t CCMExtensionEnablingFlagLayer4        : BITFIELD_RANGE( 24,24 );
            uint32_t ToneMappingEnablingFlagLayer4         : BITFIELD_RANGE( 25,25 );
            uint32_t PriorCSCEnablingFlagLayer4            : BITFIELD_RANGE( 26,26 );
            uint32_t EOTF1DLUTEnablingFlagLayer4           : BITFIELD_RANGE( 27,27 );
            uint32_t CCMEnablingFlagLayer4                 : BITFIELD_RANGE( 28,28 );
            uint32_t OETF1DLUTEnablingFlagLayer4           : BITFIELD_RANGE( 29,29 );
            uint32_t PostCSCEnablingFlagLayer4             : BITFIELD_RANGE( 30,30 );
            uint32_t Enabling3DLUTFlagLayer4               : BITFIELD_RANGE( 31,31 );
        };
        uint32_t       Value;
    } DW52;

    // uint32_t 53 - GRF R7.5
    union
    {
        struct {
            uint32_t FormatDescriptorLayer5                : BITFIELD_RANGE(  0,7  );
            uint32_t ChromaSittingLocationLayer5           : BITFIELD_RANGE(  8,10 );
            uint32_t ChannelSwapEnablingFlagLayer5         : BITFIELD_RANGE( 11,11 );
            uint32_t IEFBypassEnablingFlagLayer5           : BITFIELD_RANGE( 12,12 );
            uint32_t RotationAngleMirrorDirectionLayer5    : BITFIELD_RANGE( 13,15 );
            uint32_t SamplerIndexFirstPlaneLayer5          : BITFIELD_RANGE( 16,19 );
            uint32_t SamplerIndexSecondThirdPlaneLayer5    : BITFIELD_RANGE( 20,23 );
            uint32_t CCMExtensionEnablingFlagLayer5        : BITFIELD_RANGE( 24,24 );
            uint32_t ToneMappingEnablingFlagLayer5         : BITFIELD_RANGE( 25,25 );
            uint32_t PriorCSCEnablingFlagLayer5            : BITFIELD_RANGE( 26,26 );
            uint32_t EOTF1DLUTEnablingFlagLayer5           : BITFIELD_RANGE( 27,27 );
            uint32_t CCMEnablingFlagLayer5                 : BITFIELD_RANGE( 28,28 );
            uint32_t OETF1DLUTEnablingFlagLayer5           : BITFIELD_RANGE( 29,29 );
            uint32_t PostCSCEnablingFlagLayer5             : BITFIELD_RANGE( 30,30 );
            uint32_t Enabling3DLUTFlagLayer5               : BITFIELD_RANGE( 31,31 );
        };
        uint32_t       Value;
    } DW53;

    // uint32_t 54 - GRF R7.6
    union
    {
        struct {
            uint32_t FormatDescriptorLayer6                : BITFIELD_RANGE(  0,7  );
            uint32_t ChromaSittingLocationLayer6           : BITFIELD_RANGE(  8,10 );
            uint32_t ChannelSwapEnablingFlagLayer6         : BITFIELD_RANGE( 11,11 );
            uint32_t IEFBypassEnablingFlagLayer6           : BITFIELD_RANGE( 12,12 );
            uint32_t RotationAngleMirrorDirectionLayer6    : BITFIELD_RANGE( 13,15 );
            uint32_t SamplerIndexFirstPlaneLayer6          : BITFIELD_RANGE( 16,19 );
            uint32_t SamplerIndexSecondThirdPlaneLayer6    : BITFIELD_RANGE( 20,23 );
            uint32_t CCMExtensionEnablingFlagLayer6        : BITFIELD_RANGE( 24,24 );
            uint32_t ToneMappingEnablingFlagLayer6         : BITFIELD_RANGE( 25,25 );
            uint32_t PriorCSCEnablingFlagLayer6            : BITFIELD_RANGE( 26,26 );
            uint32_t EOTF1DLUTEnablingFlagLayer6           : BITFIELD_RANGE( 27,27 );
            uint32_t CCMEnablingFlagLayer6                 : BITFIELD_RANGE( 28,28 );
            uint32_t OETF1DLUTEnablingFlagLayer6           : BITFIELD_RANGE( 29,29 );
            uint32_t PostCSCEnablingFlagLayer6             : BITFIELD_RANGE( 30,30 );
            uint32_t Enabling3DLUTFlagLayer6               : BITFIELD_RANGE( 31,31 );
        };
        uint32_t       Value;
    } DW54;

    // uint32_t 55 - GRF R7.7
    union
    {
        struct {
            uint32_t FormatDescriptorLayer7                : BITFIELD_RANGE(  0,7  );
            uint32_t ChromaSittingLocationLayer7           : BITFIELD_RANGE(  8,10 );
            uint32_t ChannelSwapEnablingFlagLayer7         : BITFIELD_RANGE( 11,11 );
            uint32_t IEFBypassEnablingFlagLayer7           : BITFIELD_RANGE( 12,12 );
            uint32_t RotationAngleMirrorDirectionLayer7    : BITFIELD_RANGE( 13,15 );
            uint32_t SamplerIndexFirstPlaneLayer7          : BITFIELD_RANGE( 16,19 );
            uint32_t SamplerIndexSecondThirdPlaneLayer7    : BITFIELD_RANGE( 20,23 );
            uint32_t CCMExtensionEnablingFlagLayer7        : BITFIELD_RANGE( 24,24 );
            uint32_t ToneMappingEnablingFlagLayer7         : BITFIELD_RANGE( 25,25 );
            uint32_t PriorCSCEnablingFlagLayer7            : BITFIELD_RANGE( 26,26 );
            uint32_t EOTF1DLUTEnablingFlagLayer7           : BITFIELD_RANGE( 27,27 );
            uint32_t CCMEnablingFlagLayer7                 : BITFIELD_RANGE( 28,28 );
            uint32_t OETF1DLUTEnablingFlagLayer7           : BITFIELD_RANGE( 29,29 );
            uint32_t PostCSCEnablingFlagLayer7             : BITFIELD_RANGE( 30,30 );
            uint32_t Enabling3DLUTFlagLayer7               : BITFIELD_RANGE( 31,31 );
        };
        uint32_t       Value;
    } DW55;

    // uint32_t 56 - GRF R8.0
    union
    {
        struct {
            uint32_t ConstantBlendingAlphaFillColorLayer0  : BITFIELD_RANGE(  0,7  );
            uint32_t ConstantBlendingAlphaFillColorLayer1  : BITFIELD_RANGE(  8,15 );
            uint32_t ConstantBlendingAlphaFillColorLayer2  : BITFIELD_RANGE( 16,23 );
            uint32_t ConstantBlendingAlphaFillColorLayer3  : BITFIELD_RANGE( 24,31 );
        };
        uint32_t       Value;
    } DW56;

    // uint32_t 57 - GRF R8.1
    union
    {
        struct {
            uint32_t ConstantBlendingAlphaFillColorLayer4  : BITFIELD_RANGE(  0,7  );
            uint32_t ConstantBlendingAlphaFillColorLayer5  : BITFIELD_RANGE(  8,15 );
            uint32_t ConstantBlendingAlphaFillColorLayer6  : BITFIELD_RANGE( 16,23 );
            uint32_t ConstantBlendingAlphaFillColorLayer7  : BITFIELD_RANGE( 24,31 );
        };
        uint32_t       Value;
    } DW57;

    // uint32_t 58 - GRF R8.2
    union
    {
        struct {
            uint32_t TwoLayerOperationLayer0 : BITFIELD_RANGE(  0,7  );
            uint32_t TwoLayerOperationLayer1 : BITFIELD_RANGE(  8,15 );
            uint32_t TwoLayerOperationLayer2 : BITFIELD_RANGE( 16,23 );
            uint32_t TwoLayerOperationLayer3 : BITFIELD_RANGE( 24,31 );
        };
        uint32_t       Value;
    } DW58;

    // uint32_t 59 - GRF R8.3
    union
    {
        struct {
            uint32_t TwoLayerOperationLayer4 : BITFIELD_RANGE(  0,7  );
            uint32_t TwoLayerOperationLayer5 : BITFIELD_RANGE(  8,15 );
            uint32_t TwoLayerOperationLayer6 : BITFIELD_RANGE( 16,23 );
            uint32_t TwoLayerOperationLayer7 : BITFIELD_RANGE( 24,31 );
        };
        uint32_t       Value;
    } DW59;

    // uint32_t 60 - GRF R8.4
    union
    {
        struct {
            uint32_t FixedPointFillColorRVChannel : BITFIELD_RANGE(  0,15 );
            uint32_t FixedPointFillColorGYChannel : BITFIELD_RANGE( 16,31 );
        };
        uint32_t       Value;
    } DW60;

    // uint32_t 61 - GRF R8.5
    union
    {
        struct {
            uint32_t FixedPointFillColorBUChannel    : BITFIELD_RANGE(  0,15 );
            uint32_t FixedPointFillColorAlphaChannel : BITFIELD_RANGE( 16,31 );
        };
        uint32_t       Value;
    } DW61;

    // uint32_t 62 - GRF R8.6
    union
    {
        struct {
            uint32_t DestinationWidth  : BITFIELD_RANGE(  0,15 );
            uint32_t DestinationHeight : BITFIELD_RANGE( 16,31 );
        };
        uint32_t       Value;
    } DW62;

    // uint32_t 63 - GRF R8.7
    union
    {
        struct {
            uint32_t TotalNumberInputLayers                    : BITFIELD_RANGE(  0,15 );
            uint32_t FormatDescriptorDestination               : BITFIELD_RANGE( 16,23 );
            uint32_t ChromaSittingLocationDestination          : BITFIELD_RANGE( 24,26 );
            uint32_t ChannelSwapEnablingFlagDestination        : BITFIELD_RANGE( 27,27 );
            uint32_t DstCSCEnablingFlagDestination             : BITFIELD_RANGE( 28,28 );
            uint32_t Reserved                                  : BITFIELD_RANGE( 29,29 );
            uint32_t DitherRoundEnablingFlagDestinationSurface : BITFIELD_RANGE( 30,31 );
        };
        uint32_t       Value;
    } DW63;
} MEDIA_WALKER_HDR_STATIC_DATA_G9, * PMEDIA_WALKER_HDR_STATIC_DATA_G9;
C_ASSERT(SIZE32(MEDIA_WALKER_HDR_STATIC_DATA_G9) == 64);

//!
//! \brief hdr kernel eotf/oetf type definition enum
//!
typedef enum _VPHAL_HDR_KERNEL_EOTF_TYPE_G9
{
    VPHAL_HDR_KERNEL_EOTF_TRADITIONAL_GAMMA_G9 = 0,
    VPHAL_HDR_KERNEL_SMPTE_ST2084_G9           = 1
} VPHAL_HDR_KERNEL_EOTF_TYPE_G9;

//!
//! \brief HDR Format Descriptor enum
//!
typedef enum _VPHAL_HDR_FORMAT_DESCRIPTOR_G9
{
    VPHAL_HDR_FORMAT_DESCRIPTOR_UNKNOW_G9 = -1,
    VPHAL_HDR_FORMAT_R16G16B16A16_FLOAT_G9 = 44,
    VPHAL_HDR_FORMAT_DESCRIPTOR_R16G16_UNORM_G9 = 60,
    VPHAL_HDR_FORMAT_DESCRIPTOR_R16_UNORM_G9 = 70,
    VPHAL_HDR_FORMAT_DESCRIPTOR_R10G10B10A2_UNORM_G9 = 89,
    VPHAL_HDR_FORMAT_DESCRIPTOR_R8G8B8A8_UNORM_G9 = 101,
    VPHAL_HDR_FORMAT_DESCRIPTOR_YUY2_G9 = 201,
    VPHAL_HDR_FORMAT_DESCRIPTOR_NV12_G9 = 220,
    VPHAL_HDR_FORMAT_DESCRIPTOR_P010_G9 = 222,
    VPHAL_HDR_FORMAT_DESCRIPTOR_P016_G9 = 223
} VPHAL_HDR_FORMAT_DESCRIPTOR_G9;

//!
//! \brief HDR Chroma Siting enum
//!
typedef enum _VPHAL_HDR_CHROMA_SITING_G9
{
    VPHAL_HDR_CHROMA_SITTING_A_G9 = 0, // Sample even index at even line
    VPHAL_HDR_CHROMA_SITTING_B_G9,     // Sample even index at odd line
    VPHAL_HDR_CHROMA_SITTING_AC_G9,    // Average consistent even index and odd index at even line
    VPHAL_HDR_CHROMA_SITTING_BD_G9,    // Average consistent even index and odd index at odd line
    VPHAL_HDR_CHROMA_SITTING_AB_G9,    // Average even index of even line and even index of odd line
    VPHAL_HDR_CHROMA_SITTING_ABCD_G9   // Average even and odd index at even line and odd line
} VPHAL_HDR_CHROMA_SITING_G9;

//!
//! \brief HDR Rotation enum
//!
typedef enum _VPHAL_HDR_ROTATION_G9
{
    VPHAL_HDR_LAYER_ROTATION_0_G9 = 0, // 0 degree rotation
    VPHAL_HDR_LAYER_ROTATION_90_G9,     // 90 degree CW rotation
    VPHAL_HDR_LAYER_ROTATION_180_G9,    // 180 degree rotation
    VPHAL_HDR_LAYER_ROTATION_270_G9,    // 270 degree CW rotation
    VPHAL_HDR_LAYER_MIRROR_H_G9,        // 0 degree rotation then mirror horizontally
    VPHAL_HDR_LAYER_ROT_90_MIR_H_G9,    // 90 degree CW rotation then mirror horizontally
    VPHAL_HDR_LAYER_MIRROR_V_G9,        // 180 degree rotation then mirror horizontally (vertical mirror)
    VPHAL_HDR_LAYER_ROT_90_MIR_V_G9     // 270 degree CW rotation then mirror horizontally (90 degree CW rotation then vertical mirror)
} VPHAL_HDR_ROTATION_G9;

//!
//! \brief Two Layer Option enum
//!
typedef enum _VPHAL_HDR_TWO_LAYER_OPTION_G9
{
    VPHAL_HDR_TWO_LAYER_OPTION_SBLEND_G9 = 0, // Source Blending
    VPHAL_HDR_TWO_LAYER_OPTION_CBLEND_G9,     // Constant Blending
    VPHAL_HDR_TWO_LAYER_OPTION_PBLEND_G9,     // Partial Blending
    VPHAL_HDR_TWO_LAYER_OPTION_CSBLEND_G9,    // Constant Source Blending
    VPHAL_HDR_TWO_LAYER_OPTION_CPBLEND_G9,    // Constant Partial Blending
    VPHAL_HDR_TWO_LAYER_OPTION_COMP_G9        // Composition
} VPHAL_HDR_TWO_LAYER_OPTION_G9;

//!
//! \brief sampler state index enum
//!
typedef enum _VPHAL_HDR_SAMPLER_STATE_INDEX_G9
{
    VPHAL_HDR_SAMPLER_STATE_AVS_NEAREST_INDEX_G9 = 1,
    VPHAL_HDR_SAMPLER_STATE_AVS_POLYPHASE_INDEX_G9 = 3,
    VPHAL_HDR_SAMPLER_STATE_3D_NEAREST_INDEX_G9 = 13,
    VPHAL_HDR_SAMPLER_STATE_3D_BILINEAR_INDEX_G9 = 14
} VPHAL_HDR_SAMPLER_STATE_INDEX_G9;

//!
//! \brief    Destroy interface for HDR
//! \details  Destroy interface for HDR which is Gen9 platform specific
//! \param    PVPHAL_HDR_STATE pHdrState
//!           [in] Pointer to HDR state
//! \return   MOS_STATUS
//!
MOS_STATUS VpHal_HdrDestroyInterface_g9(
    PVPHAL_HDR_STATE        pHdrState);

//!
//! \brief    Checks to see if HDR can be enabled for the formats
//! \details  Checks to see if HDR can be enabled for the formats
//! \param    PVPHAL_SURFACE pSrcSurface
//!           [in] Pointer to source surface
//! \param    bool* pbSupported
//!           [out] true supported false not supported
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_HdrIsInputFormatSupported_g9(
    PVPHAL_SURFACE              pSrcSurface,
    bool*                       pbSupported);

//!
//! \brief    Checks to see if HDR can be enabled for the formats
//! \details  Checks to see if HDR can be enabled for the formats
//! \param    PVPHAL_SURFACE pTargetSurface
//!           [in] Pointer to target surface
//! \param    bool* pbSupported
//!           [out] true supported false not supported
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_HdrIsOutputFormatSupported_g9(
    PVPHAL_SURFACE              pTargetSurface,
    bool*                       pbSupported);

//!
//! \brief    Allocate Resources for HDR
//! \details  Allocate Resources for HDR
//! \param    PVPHAL_HDR_STATE pHdrStatee
//!           [in] Pointer to HDR state
//! \return   void
//!
MOS_STATUS VpHal_HdrAllocateResources_g9(
    PVPHAL_HDR_STATE    pHdrState);

//!
//! \brief    HDR Surface State Setup
//! \details  Set up surface state used in HDR process, and bind the surface to pointed binding table entry.
//! \param    PVPHAL_HDR_STATE pHdrState
//            [in/out] Pointer to HDR state
//! \param    PVPHAL_HDR_RENDER_DATA pRenderData
//!           [in] Pointer to hdr render data.
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_HdrSetupSurfaceStates_g9(
    PVPHAL_HDR_STATE           pHdrState,
    PVPHAL_HDR_RENDER_DATA     pRenderData);

//!
//! \brief    Get the Kernel Params
//! \details  Get the Kernel Params, including kernel unique ID, KDT Index, and performance tag
//! \param    VPHAL_HDR_PHASE pHdrState
//!           [in] pointer to HDR State
//! \param    int32_t* pKUIDOut
//!           [out] Kernel unique ID
//! \param    int32_t* pKDTIndexOut
//!           [out] KDT index
//! \param    PVPHAL_PERFTAG pPerfTag
//!           [out] Performance tag
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_HdrGetKernelParam_g9(
    uint32_t                    HdrKernelID,
    int32_t*                    pKUIDOut,
    int32_t*                    pKDTIndexOut);

//!
//! \brief    Load CURBE for HDR kernel
//! \details  Load CURBE for Gen9 HDR kernel, but also expose it to be used by subsequent generations.
//! \param    PVPHAL_HDR_STATE pHdrState
//!           [in] Pointer to HDR state
//! \param    PVPHAL_HDR_RENDER_DATA pRenderData
//!           [in] Poniter to HDR render data
//! \param    int32_t* piCurbeOffsetOut
//!           [Out] Curbe offset
//! \return   MOS_STATUS
//!
MOS_STATUS VpHal_HdrLoadStaticData_g9(
    PVPHAL_HDR_STATE            pHdrState,
    PVPHAL_HDR_RENDER_DATA      pRenderData,
    int32_t*                    piCurbeOffsetOut);

//! \brief    Get the HDR format descriptor of a format
//! \details  Get the HDR format descriptor of a format and return.
//! \param    MOS_FORMAT Format
//!           [in] MOS_FORMAT of a surface
//! \return   VPHAL_HDR_FORMAT_DESCRIPTOR_G9
//!           HDR format descriptor
//!
VPHAL_HDR_FORMAT_DESCRIPTOR_G9 VpHal_HdrGetFormatDescriptor_g9(
    MOS_FORMAT      Format);

//! \brief    Get the HDR Chroma siting
//! \details  Get the HDR Chroma siting and return.
//! \param    uint32_t ChromaSiting
//!           [in] ChromaSiting of a surface
//! \return   VPHAL_HDR_CHROMA_SITING_G9
//!           HDR Chroma siting
//!
VPHAL_HDR_CHROMA_SITING_G9 VpHal_HdrGetHdrChromaSiting_g9(
    uint32_t      ChromaSiting);

//! \brief    Get the HDR rotation
//! \details  Get the HDR rotation and return.
//! \param    VPHAL_ROTATION Rotation
//!           [in] Rotation of a surface
//! \return   VPHAL_HDR_ROTATION_G9
//!           HDR Chroma siting
//!
VPHAL_HDR_ROTATION_G9 VpHal_HdrGetHdrRotation_g9(
    VPHAL_ROTATION      Rotation);

//!
//! \brief    Initializes interface for HDR
//! \details  Initializes interface for HDR which is Gen9 platform specific
//! \param    PVPHAL_HDR_STATE pHdrState
//!           [in] Pointer to HDR state
//! \return   MOS_STATUS
//!
MOS_STATUS VpHal_HdrInitInterface_g9(
        PVPHAL_HDR_STATE        pHdrState);

//!
//! \brief    Checks to see if HDR can be enabled for the formats
//! \details  Checks to see if HDR can be enabled for the formats
//! \param    PVPHAL_SURFACE pSrcSurface
//!           [in] Pointer to source surface
//! \param    bool* pbSupported
//!           [out] true supported false not supported
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_HdrIsInputFormatSupported_g9(
    PVPHAL_SURFACE              pSrcSurface,
    bool*                       pbSupported);

//!
//! \brief    Checks to see if HDR can be enabled for the formats
//! \details  Checks to see if HDR can be enabled for the formats
//! \param    PVPHAL_SURFACE pTargetSurface
//!           [in] Pointer to target surface
//! \param    bool* pbSupported
//!           [out] true supported false not supported
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_HdrIsOutputFormatSupported_g9(
    PVPHAL_SURFACE              pTargetSurface,
    bool*                       pbSupported);

//!
//! \brief    Initiate EOTF Surface for HDR
//! \details  Initiate EOTF Surface for HDR
//! \param    PVPHAL_HDR_STATE pHdrStatee
//!           [in] Pointer to HDR state
//! \param    int32_t iIndex
//!           [in] input surface index
//! \param    PVPHAL_SURFACE pOETF1DLUTSurface
//!           [in] Pointer to OETF 1D LUT Surface
//! \return   MOS_STATUS
//!
MOS_STATUS VpHal_HdrInitOETF1DLUT_g9(
    PVPHAL_HDR_STATE pHdrState,
    int32_t               iIndex,
    PVPHAL_SURFACE        pOETF1DLUTSurface);

//!
//! \brief    Set HDR Ief State
//! \details  Set HDR Ief State
//! \param    PVPHAL_HDR_STATE pHdrState
//!           [in] Pointer to HDR state
//! \param    PVPHAL_HDR_RENDER_DATA pRenderData
//!           [in] Pointer to render data
//! \param    PMHW_SAMPLER_STATE_PARAM    pSamplerStateParams
//!           [in] Pointer to Sampler State Parameters
//! \return   MOS_STATUS
//!
MOS_STATUS VpHal_HdrSetIefStates_g9(
    PVPHAL_HDR_STATE            pHdrState,
    PVPHAL_HDR_RENDER_DATA      pRenderData,
    PMHW_SAMPLER_STATE_PARAM    pSamplerStateParams);

//!
//! \brief    Set the Sampler States
//! \details  Set the Sampler States
//! \param    VPHAL_HDR_PHASE pHdrState
//!           [in] pointer to HDR State
//! \param    PVPHAL_HDR_RENDER_DATA
//!           [in] HDR render data
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_HdrSetSamplerStates_g9 (
    PVPHAL_HDR_STATE            pHdrState,
    PVPHAL_HDR_RENDER_DATA      pRenderData);

//!
//! \brief      Set Sampler8x8 Table for Gen9 Hdr AVS
//! \details    Set sampler8x8 table based on format, scale and chroma siting
//! \param      PRENDERHAL_INTERFACE pRenderHal
//!             [in]    Pointer to RenderHal interface
//! \param      PMHW_SAMPLER_STATE_PARAM pSamplerStateParams,
//!             [in]    Pointer to sampler state params
//! \param      PVPHAL_AVS_PARAMS pAvsParams
//!             [in]    Pointer to avs parameters
//! \param      MOS_FORMAT SrcFormat
//!             [in]    source format
//! \param      float   fScaleX
//!             [in]    Scale X
//! \param      float   fScaleY
//!             [in]    Scale Y
//! \param      uint32_t   dwChromaSiting
//!             [in]    Chroma siting
//! \return     void
//!
MOS_STATUS VpHal_HdrSetSamplerAvsTableParam_g9(
    PRENDERHAL_INTERFACE            pRenderHal,
    PMHW_SAMPLER_STATE_PARAM        pSamplerStateParams,
    PMHW_AVS_PARAMS                 pAvsParams,
    MOS_FORMAT                      SrcFormat,
    float                           fScaleX,
    float                           fScaleY,
    uint32_t                        dwChromaSiting);

//!
//! \brief    Get Hdr iTouch Split Frame Portion number
//! \details  Get Hdr iTouch Split Frame Portion number is Gen9 platform specific
//! \param    PVPHAL_HDR_STATE pHdrState
//!           [in/out] Pointer to HDR state
//! \return   MOS_STATUS
//!
MOS_STATUS VpHal_HdrGetSplitFramePortion_g9(
    PVPHAL_HDR_STATE        pHdrState);

//!
//! \brief    HDR PreProcess Surface State Setup
//! \details  Set up surface state used in HDR PreProcess, and bind the surface to pointed binding table entry.
//! \param    PVPHAL_HDR_STATE pHdrState
//            [in/out] Pointer to HDR state
//! \param    PVPHAL_HDR_RENDER_DATA pRenderData
//!           [in] Pointer to hdr render data.
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_HdrSetupPreProcessSurfaceStates_g9(
    PVPHAL_HDR_STATE           pHdrState,
    PVPHAL_HDR_RENDER_DATA     pRenderData);

//!
//! \brief    Setup HDR PreProcess CURBE data
//! \details  Setup HDR PreProcess CURBE data
//! \param    PVPHAL_HDR_STATE pHdrState
//!           [in] Poniter to HDR state
//! \param    PVPHAL_HDR_RENDER_DATA pRenderData
//!           [in] Poniter to HDR render data
//! \param    int32_t* piCurbeOffsetOut
//!           [Out] Curbe offset
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if successful, otherwise
//!
MOS_STATUS VpHal_HdrPreprocessLoadStaticData_g9(
    PVPHAL_HDR_STATE            pHdrState,
    PVPHAL_HDR_RENDER_DATA      pRenderData,
    int32_t*                    piCurbeOffsetOut);
#if __cplusplus
}
#endif // __cplusplus
#endif // __VPHAL_RENDER_HDR_G9_BASE_H__
