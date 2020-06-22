/*
* Copyright (c) 2019, Intel Corporation
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

#ifndef _MDF_FC_COMMON_H_
#define _MDF_FC_COMMON_H_

#define MDF_FC_BLOCK_WIDTH                16
#define MDF_FC_BLOCK_HEIGHT               16
#define MDF_FC_SAMPLER_UNORM_WIDTH        8
#define MDF_FC_SAMPLER_UNORM_HEIGHT       4
#define MDF_FC_MAX_INPUT_LAYER_NUM        8
#define MDF_FC_MAX_OUTPUT_NUM             2
#define MDF_FC_INPUT_BTI_PER_LAYER        3
#define MDF_FC_OUTPUT_BTI_PER_LAYER       3
#define SURFACE_FORMAT_CHANNEL_SWAP       (1 << 11)
#define MDF_FC_UV_PLANE_BTI_OFFSET        1
#define MDF_FC_U_PLANE_BTI_OFFSET         1
#define MDF_FC_V_PLANE_BTI_OFFSET         2
#define MDF_FC_CSC_COEFF_WIDTH            24
#define MDF_FC_INPUT_BTI_CSC_COEFF_OFFSET 34
#define MDF_FC_GAMMA_LUT_BTI_OFFSET       32
#define MDF_FC_COLORE_LUT_BTI_OFFSET      32
#define MDF_FC_LUT_ENABLE_COLORE          0x01
#define MDF_FC_LUT_ENABLE_GAMMA           0x02
#define MDF_FC_NORMALIZE_FACTOR           65280.0f
#define MDF_FC_SAMPLER_BIAS               0.015625f
#define MDF_FC_IEF_BYPASS_SHFIT           27
#define MDF_FC_IEF_BYPASS_MASK            (1 << MDF_FC_IEF_BYPASS_SHFIT)
#define MDF_FC_COMBINED_KERNEL_MASK       0x0
#define MDF_FC_SINGLE_KERNEL_MASK         0x01

#ifdef CMFC_ULT
#define MDF_FC_INPUT_BTI_START                      16
#define MDF_FC_3D_SAMPLER_SI_Y                      13
#define MDF_FC_3D_SAMPLER_SI_U                      13
#define MDF_FC_3D_SAMPLER_SI_V                      13
#define MDF_FC_COLORE_nSI_LINEAR                    13 // ColorE Sampler index left shifted by 8
#define MDF_FC_AVS_SI_Y                             1
#define MDF_FC_AVS_SI_U                             1
#define MDF_FC_AVS_SI_V                             1
#define MDF_FC_CSC_COEFF_HEIGHT                     1
#define MDF_FC_INPUT_BTI_START_INTERLACED_F2_OFFSET 35
#define MDF_FC_SECURITY_COPY_INPUT_BTI MDF_FC_INPUT_BTI_START
#define MDF_FC_SECURITY_COPY_OUTPUT_BTI MDF_FC_INPUT_BTI_START + MDF_FC_MAX_INPUT_LAYER_NUM * MDF_FC_INPUT_BTI_PER_LAYER
#else
#define MDF_FC_INPUT_BTI_START                      0
#define MDF_FC_3D_SAMPLER_SI_Y                      1
#define MDF_FC_3D_SAMPLER_SI_U                      2
#define MDF_FC_3D_SAMPLER_SI_V                      3
#define MDF_FC_COLORE_nSI_LINEAR                    0x000 // ColorE Sampler index left shifted by 8
#define MDF_FC_AVS_SI_Y                             1
#define MDF_FC_AVS_SI_U                             3
#define MDF_FC_AVS_SI_V                             3
#define MDF_FC_CSC_COEFF_HEIGHT                     8
#define MDF_FC_INPUT_BTI_START_INTERLACED_F2_OFFSET 48
#define MDF_FC_SECURITY_COPY_INPUT_BTI              0
#define MDF_FC_SECURITY_COPY_OUTPUT_BTI             1
#endif

#define MDF_FC_OUTPUT_BTI_START     MDF_FC_INPUT_BTI_START + MDF_FC_MAX_INPUT_LAYER_NUM * MDF_FC_INPUT_BTI_PER_LAYER
#define MDF_FC_CSC_COEFF_BTI        MDF_FC_INPUT_BTI_START + MDF_FC_INPUT_BTI_CSC_COEFF_OFFSET
#define MDF_FC_INPUT_BTI_F2         MDF_FC_INPUT_BTI_START + MDF_FC_INPUT_BTI_START_INTERLACED_F2_OFFSET
#define MDF_FC_GAMMA_LUT_BTI        MDF_FC_INPUT_BTI_START + MDF_FC_GAMMA_LUT_BTI_OFFSET
#define MDF_FC_COLORE_LUT_BTI       MDF_FC_INPUT_BTI_START + MDF_FC_COLORE_LUT_BTI_OFFSET

typedef enum _MDF_FC_FORMAT
{
    FORMAT_R32G32B32A32_FLOAT                 = 0,
    FORMAT_B32G32R32A32_FLOAT                 = 0 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_R32G32B32A32_UINT                  = 1,
    FORMAT_B32G32R32A32_UINT                  = 1 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_R32G32B32A32_SINT                  = 2,
    FORMAT_B32G32R32A32_SINT                  = 2 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_R32G32B32X32_FLOAT                 = 3,
    FORMAT_B32G32R32X32_FLOAT                 = 3 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_R32G32B32X32_UINT                  = 4,
    FORMAT_B32G32R32X32_UINT                  = 4 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_R32G32B32X32_SINT                  = 5,
    FORMAT_B32G32R32X32_SINT                  = 5 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_R32G32B32_FLOAT                    = 6,
    FORMAT_B32G32R32_FLOAT                    = 6 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_R32G32B32_UINT                     = 7,
    FORMAT_B32G32R32_UINT                     = 7 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_R32G32B32_SINT                     = 8,
    FORMAT_B32G32R32_SINT                     = 8 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_R32G32_FLOAT                       = 9,
    FORMAT_R32G32_UINT                        = 10,
    FORMAT_R32G32_SINT                        = 11,
    FORMAT_L32A32_FLOAT                       = 12,
    FORMAT_L32A32_UINT                        = 13,
    FORMAT_L32A32_SINT                        = 14,
    FORMAT_A32X32_FLOAT                       = 15,
    FORMAT_A32X32_UINT                        = 16,
    FORMAT_A32X32_SINT                        = 17,
    FORMAT_L32X32_FLOAT                       = 18,
    FORMAT_L32X32_UINT                        = 19,
    FORMAT_L32X32_SINT                        = 20,
    FORMAT_I32X32_FLOAT                       = 21,
    FORMAT_I32X32_UINT                        = 22,
    FORMAT_I32X32_SINT                        = 23,
    FORMAT_R32_FLOAT                          = 24,
    FORMAT_R32_UINT                           = 25,
    FORMAT_R32_SINT                           = 26,
    FORMAT_A32_FLOAT                          = 27,
    FORMAT_A32_UINT                           = 28,
    FORMAT_A32_SINT                           = 29,
    FORMAT_L32_FLOAT                          = 30,
    FORMAT_L32_UINT                           = 31,
    FORMAT_L32_SINT                           = 32,
    FORMAT_I32_FLOAT                          = 33,
    FORMAT_I32_UINT                           = 34,
    FORMAT_I32_SINT                           = 35,
    FORMAT_R24_UNORM_X8_TYPELESS              = 36,
    FORMAT_R24_SNORM_X8_TYPELESS              = 37,
    FORMAT_I24X8_UNORM                        = 38,
    FORMAT_I24X8_SNORM                        = 39,
    FORMAT_L24X8_UNORM                        = 40,
    FORMAT_L24X8_SNORM                        = 41,
    FORMAT_A24X8_UNORM                        = 42,
    FORMAT_A24X8_SNORM                        = 43,
    FORMAT_R16G16B16A16_FLOAT                 = 44,
    FORMAT_B16G16R16A16_FLOAT                 = 44 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_R16G16B16A16_UNORM                 = 45,
    FORMAT_B16G16R16A16_UNORM                 = 45 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_R16G16B16A16_UINT                  = 46,
    FORMAT_B16G16R16A16_UINT                  = 46 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_R16G16B16A16_SNORM                 = 47,
    FORMAT_B16G16R16A16_SNORM                 = 47 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_R16G16B16A16_SINT                  = 48,
    FORMAT_B16G16R16A16_SINT                  = 48 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_R16G16B16X16_FLOAT                 = 49,
    FORMAT_B16G16R16X16_FLOAT                 = 49 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_R16G16B16X16_UNORM                 = 50,
    FORMAT_B16G16R16X16_UNORM                 = 50 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_R16G16B16X16_UINT                  = 51,
    FORMAT_B16G16R16X16_UINT                  = 51 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_R16G16B16X16_SNORM                 = 52,
    FORMAT_B16G16R16X16_SNORM                 = 52 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_R16G16B16X16_SINT                  = 53,
    FORMAT_B16G16R16X16_SINT                  = 53 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_R16G16B16_FLOAT                    = 54,
    FORMAT_B16G16R16_FLOAT                    = 54 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_R16G16B16_UNORM                    = 55,
    FORMAT_B16G16R16_UNORM                    = 55 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_R16G16B16_UINT                     = 56,
    FORMAT_B16G16R16_UINT                     = 56 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_R16G16B16_SNORM                    = 57,
    FORMAT_B16G16R16_SNORM                    = 57 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_R16G16B16_SINT                     = 58,
    FORMAT_B16G16R16_SINT                     = 58 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_R16G16_FLOAT                       = 59,
    FORMAT_R16G16_UNORM                       = 60,
    FORMAT_R16G16_UINT                        = 61,
    FORMAT_R16G16_SNORM                       = 62,
    FORMAT_R16G16_SINT                        = 63,
    FORMAT_L16A16_FLOAT                       = 64,
    FORMAT_L16A16_UNORM                       = 65,
    FORMAT_L16A16_UINT                        = 66,
    FORMAT_L16A16_SNORM                       = 67,
    FORMAT_L16A16_SINT                        = 68,
    FORMAT_R16_FLOAT                          = 69,
    FORMAT_R16_UNORM                          = 70,
    FORMAT_R16_UINT                           = 71,
    FORMAT_R16_SNORM                          = 72,
    FORMAT_R16_SINT                           = 73,
    FORMAT_A16_FLOAT                          = 74,
    FORMAT_A16_UNORM                          = 75,
    FORMAT_A16_UINT                           = 76,
    FORMAT_A16_SNORM                          = 77,
    FORMAT_A16_SINT                           = 78,
    FORMAT_L16_FLOAT                          = 79,
    FORMAT_L16_UNORM                          = 80,
    FORMAT_L16_UINT                           = 81,
    FORMAT_L16_SNORM                          = 82,
    FORMAT_L16_SINT                           = 83,
    FORMAT_I16_FLOAT                          = 84,
    FORMAT_I16_UNORM                          = 85,
    FORMAT_I16_UINT                           = 86,
    FORMAT_I16_SNORM                          = 87,
    FORMAT_I16_SINT                           = 88,
    FORMAT_R10G10B10A2_UNORM                  = 89,
    FORMAT_B10G10R10A2_UNORM                  = 89 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_R10G10B10A2_UNORM_SRGB             = 90,
    FORMAT_B10G10R10A2_UNORM_SRGB             = 90 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_R10G10B10A2_UINT                   = 91,
    FORMAT_B10G10R10A2_UINT                   = 91 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_R10G10B10_SNORM_A2_UNORM           = 92,
    FORMAT_B10G10R10_SNORM_A2_UNORM           = 92 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_R10G10B10_XR_BIAS_A2_UNORM         = 93,
    FORMAT_B10G10R10_XR_BIAS_A2_UNORM         = 93 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_R10G10B10X2_UNORM                  = 94,
    FORMAT_B10G10R10X2_UNORM                  = 94 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_R10G10B10X2_UNORM_SRGB             = 95,
    FORMAT_B10G10R10X2_UNORM_SRGB             = 95 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_R10G10B10X2_UINT                   = 96,
    FORMAT_B10G10R10X2_UINT                   = 96 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_R10G10B10X2_SNORM                  = 97,
    FORMAT_B10G10R10X2_SNORM                  = 97 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_R10G10B10_XR_BIAS_X2_TYPELESS      = 98,
    FORMAT_B10G10R10_XR_BIAS_X2_TYPELESS      = 98 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_R11G11B10_FLOAT                    = 99,
    FORMAT_R9G9B9E5_SHAREDEXP                 = 100,
    FORMAT_R8G8B8A8_UNORM                     = 101,
    FORMAT_B8G8R8A8_UNORM                     = 101 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_R8G8B8A8_UNORM_SRGB                = 102,
    FORMAT_B8G8R8A8_UNORM_SRGB                = 102 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_R8G8B8A8_UINT                      = 103,
    FORMAT_B8G8R8A8_UINT                      = 103 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_R8G8B8A8_SNORM                     = 104,
    FORMAT_B8G8R8A8_SNORM                     = 104 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_R8G8B8A8_SINT                      = 105,
    FORMAT_B8G8R8A8_SINT                      = 105 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_R8G8B8X8_UNORM                     = 106,
    FORMAT_B8G8R8X8_UNORM                     = 106 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_R8G8B8X8_UNORM_SRGB                = 107,
    FORMAT_B8G8R8X8_UNORM_SRGB                = 107 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_R8G8B8X8_UINT                      = 108,
    FORMAT_B8G8R8X8_UINT                      = 108 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_R8G8B8X8_SNORM                     = 109,
    FORMAT_B8G8R8X8_SNORM                     = 109 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_R8G8B8X8_SINT                      = 110,
    FORMAT_B8G8R8X8_SINT                      = 110 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_R8G8B8_UNORM                       = 111,
    FORMAT_B8G8R8_UNORM                       = 111 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_R8G8B8_UNORM_SRGB                  = 112,
    FORMAT_B8G8R8_UNORM_SRGB                  = 112 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_R8G8B8_UINT                        = 113,
    FORMAT_B8G8R8_UINT                        = 113 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_R8G8B8_SNORM                       = 114,
    FORMAT_B8G8R8_SNORM                       = 114 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_R8G8B8_SINT                        = 115,
    FORMAT_B8G8R8_SINT                        = 115 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_R8G8_B8G8_UNORM                    = 116,
    FORMAT_B8G8_R8G8_UNORM                    = 116 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_R8G8_B8G8_UNORM_SRGB               = 117,
    FORMAT_B8G8_R8G8_UNORM_SRGB               = 117 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_R8G8_B8G8_UINT                     = 118,
    FORMAT_B8G8_R8G8_UINT                     = 118 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_R8G8_B8G8_SNORM                    = 119,
    FORMAT_B8G8_R8G8_SNORM                    = 119 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_R8G8_B8G8_SINT                     = 120,
    FORMAT_B8G8_R8G8_SINT                     = 120 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_G8R8_G8B8_UNORM                    = 121,
    FORMAT_G8B8_G8R8_UNORM                    = 121 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_G8R8_G8B8_UNORM_SRGB               = 122,
    FORMAT_G8B8_G8R8_UNORM_SRGB               = 122 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_G8R8_G8B8_UINT                     = 123,
    FORMAT_G8B8_G8R8_UINT                     = 123 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_G8R8_G8B8_SNORM                    = 124,
    FORMAT_G8B8_G8R8_SNORM                    = 124 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_G8R8_G8B8_SINT                     = 125,
    FORMAT_G8B8_G8R8_SINT                     = 125 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_R8G8_UNORM                         = 126,
    FORMAT_R8G8_UINT                          = 127,
    FORMAT_R8G8_SNORM                         = 128,
    FORMAT_R8G8_SINT                          = 129,
    FORMAT_L8A8_UNORM                         = 130,
    FORMAT_L8A8_UNORM_SRGB                    = 131,
    FORMAT_L8A8_UINT                          = 132,
    FORMAT_L8A8_SNORM                         = 133,
    FORMAT_L8A8_SINT                          = 134,
    FORMAT_R8_UNORM                           = 135,
    FORMAT_R8_UINT                            = 136,
    FORMAT_R8_SNORM                           = 137,
    FORMAT_R8_SINT                            = 138,
    FORMAT_A8_UNORM                           = 139,
    FORMAT_A8_UINT                            = 140,
    FORMAT_A8_SNORM                           = 141,
    FORMAT_A8_SINT                            = 142,
    FORMAT_L8_UNORM                           = 143,
    FORMAT_L8_UNORM_SRGB                      = 144,
    FORMAT_L8_UINT                            = 145,
    FORMAT_L8_SNORM                           = 146,
    FORMAT_L8_SINT                            = 147,
    FORMAT_I8_UNORM                           = 148,
    FORMAT_I8_UINT                            = 149,
    FORMAT_I8_SNORM                           = 150,
    FORMAT_I8_SINT                            = 151,
    FORMAT_R5G6B5_UNORM                       = 152,
    FORMAT_B5G6R5_UNORM                       = 152 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_R5G6B5_UNORM_SRGB                  = 153,
    FORMAT_B5G6R5_UNORM_SRGB                  = 153 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_R5G5B5A1_UNORM                     = 154,
    FORMAT_B5G5R5A1_UNORM                     = 154 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_R5G5B5A1_UNORM_SRGB                = 155,
    FORMAT_B5G5R5A1_UNORM_SRGB                = 155 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_A1R5G5B5_UNORM                     = 156,
    FORMAT_A1B5G5R5_UNORM                     = 156 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_A1R5G5B5_UNORM_SRGB                = 157,
    FORMAT_A1B5G5R5_UNORM_SRGB                = 157 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_R4G4B4A4_UNORM                     = 158,
    FORMAT_B4G4R4A4_UNORM                     = 158 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_R4G4B4A4_UNORM_SRGB                = 159,
    FORMAT_B4G4R4A4_UNORM_SRGB                = 159 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_A4R4G4B4_UNORM                     = 160,
    FORMAT_A4B4G4R4_UNORM                     = 160 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_A4R4G4B4_UNORM_SRGB                = 161,
    FORMAT_A4B4G4R4_UNORM_SRGB                = 161 + SURFACE_FORMAT_CHANNEL_SWAP,

    FORMAT_AYUV                               = 200,
    FORMAT_AYVU                               = 200 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_YUY2                               = 201,
    FORMAT_YVYU                               = 201 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_YCRCB_NORMAL                       = 201,
    FORMAT_YCRCB_SWAPUV                       = 201 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_VYUY                               = 202,
    FORMAT_UYVY                               = 202 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_YCRCB_SWAPUYV                      = 202,
    FORMAT_YCRCB_SWAPY                        = 202 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_Y410                               = 203,
    FORMAT_Y416                               = 204,
    FORMAT_Y210                               = 205,
    FORMAT_Y216                               = 206,
    FORMAT_HDMI                               = 207,
    FORMAT_DP                                 = 208,

    FORMAT_420_OPAQUE                         = 219,
    FORMAT_NV12                               = 220,
    FORMAT_NV21                               = 220 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_PLANAR_420_8                       = 220,
    FORMAT_NV11                               = 221,
    FORMAT_P010                               = 222,
    FORMAT_PLANAR_420_16                      = 223,
    FORMAT_P016                               = 223,
    FORMAT_P210                               = 224,
    FORMAT_P216                               = 225,
    FORMAT_P208                               = 226,

    FORMAT_I420                               = 240,
    FORMAT_YV12                               = 240 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_IMC3                               = 241,
    FORMAT_IMC1                               = 241 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_IMC4                               = 242,
    FORMAT_IMC2                               = 242 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_RGBP                               = 243,
    FORMAT_BGRP                               = 243 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_400P                               = 244,
    FORMAT_NV12_2PLANES                       = 245,
    FORMAT_YV12_3PLANES                       = 246,
    FORMAT_Y210_2PLANES                       = 247,
    FORMAT_P010_2PLANES                       = 248,
    FORMAT_BASE_MASK                          = 255,
    FORMAT_MASK                               = 255 + SURFACE_FORMAT_CHANNEL_SWAP,
    FORMAT_UNKNOWN                            = 0x7FFFFFFF,
} MDF_FC_FORMAT;

//!
//! \brief Color Spaces enum
//!
typedef enum _MDF_FC_PLANE_TYPE
{
    SINGLE_PLANE,
    TWO_PLANES_FIRST,
    TWO_PLANES_SECOND,
    THREE_PLANES_FIRST,
    THREE_PLANES_SECOND,
    THREE_PLANES_THIRD,
    PLANE_TYPE_NUM
} MDF_FC_PLANE_TYPE;

//!
//! \brief Color Spaces enum
//!
typedef enum _MDF_FC_COLOR_SPACE
{
    CSpace_None     = -5        ,   //!< Unidentified
    CSpace_Source   = -4        ,   //!< Current source Color Space

    // Groups of Color Spaces
    CSpace_RGB      = -3        ,   //!< sRGB
    CSpace_YUV      = -2        ,   //!< YUV BT601 or BT709 - non xvYCC
    CSpace_Gray     = -1        ,   //!< Gray scale image with only Y component
    CSpace_Any      =  0        ,   //!< Any

    // Specific Color Spaces
    CSpace_sRGB                 ,   //!< RGB - sRGB       -   RGB[0,255]
    CSpace_stRGB                ,   //!< RGB - stRGB      -   RGB[16,235]
    CSpace_BT601                ,   //!< YUV BT.601 Y[16,235] UV[16,240]
    CSpace_BT601_FullRange      ,   //!< YUV BT.601 Y[0,255]  UV[-128,+127]
    CSpace_BT709                ,   //!< YUV BT.709 Y[16,235] UV[16,240]
    CSpace_BT709_FullRange      ,   //!< YUV BT.709 Y[0,255]  UV[-128,+127]
    CSpace_xvYCC601             ,   //!< xvYCC 601 Y[16,235]  UV[16,240]
    CSpace_xvYCC709             ,   //!< xvYCC 709 Y[16,235]  UV[16,240]
    CSpace_BT601Gray            ,   //!< BT.601 Y[16,235]
    CSpace_BT601Gray_FullRange  ,   //!< BT.601 Y[0,255]
    CSpace_BT2020               ,   //!< BT.2020 YUV Limited Range 10bit Y[64, 940] UV[64, 960]
    CSpace_BT2020_FullRange     ,   //!< BT.2020 YUV Full Range 10bit [0, 1023] 
    CSpace_BT2020_RGB           ,   //!< BT.2020 RGB Full Range 10bit [0, 1023]  
    CSpace_BT2020_stRGB         ,   //!< BT.2020 RGB Studio Range 10bit [64, 940]
    CSpace_Count                    //!< Keep this at the end
} MDF_FC_COLOR_SPACE;

//!
//! \brief Chroma siting enum
//!
typedef enum _MDF_FC_CHROMA_SITING
{
    CHROMA_SUBSAMPLING_TOP_CENTER = 0,
    CHROMA_SUBSAMPLING_CENTER_CENTER = 0x1000000,
    CHROMA_SUBSAMPLING_BOTTOM_CENTER = 0x2000000,
    CHROMA_SUBSAMPLING_TOP_LEFT      = 0x3000000,
    CHROMA_SUBSAMPLING_CENTER_LEFT   = 0x4000000,
    CHROMA_SUBSAMPLING_BOTTOM_LEFT   = 0x5000000
} MDF_FC_CHROMA_SITING;


//!
//! \brief Vphal Rotation Mode enum
//!
typedef enum _MDF_FC_ROTATION
{
    MDF_FC_ROTATION_IDENTITY = 0,       //!< Rotation 0 degrees
    MDF_FC_ROTATION_90,                 //!< Rotation 90 degrees
    MDF_FC_ROTATION_180,                //!< Rotation 180 degrees
    MDF_FC_ROTATION_270,                //!< Rotation 270 degrees
    MDF_FC_MIRROR_HORIZONTAL,           //!< Horizontal Mirror 
    MDF_FC_MIRROR_VERTICAL,             //!< Vertical Mirror
    MDF_FC_ROTATE_90_MIRROR_VERTICAL,   //!< 90 + V Mirror
    MDF_FC_ROTATE_90_MIRROR_HORIZONTAL  //!< 90 + H Mirror
} MDF_FC_ROTATION;

#endif