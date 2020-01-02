/*
* Copyright (c) 2009-2018, Intel Corporation
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
//! \file        mos_resource_defs.h 
//! \brief 
//!
//!
//! \file     mos_resource_defs.h
//! \brief    Defines and enums used by os resource
//! \details  Defines and enums used by os resource
//!

#ifndef __MOS_RESOURCE_DEFS_H__
#define __MOS_RESOURCE_DEFS_H__

//!
//! \brief Enum for Surface Format (for sampling)
//!
typedef enum _MOS_FORMAT
{
    Format_Invalid            = -14,     //!< Invalid format
    Format_Source             = -13,     //!< Current source format

    // Generic Formats
    Format_420O               = -12,     //!< Generic Render Target Format (NV12, IMC3, 422H, 422V, 411P, 444P, RGBP, BGRP, 400P)
    Format_RGB_Swap           = -11,     //!< RGB Formats need HW Channel swap (A8R8G8B8, X8R8G8B8)
    Format_RGB_No_Swap        = -10,     //!< RGB Formats don't need HW Channel swap (A8B8G8R8)
    Format_RGB                = -9,      //!< Generic RGB input (A8R8G8B, X8R8G8B8, R5G6B5)
    Format_RGB32              = -8,      //!< Generic RGB32 input (A8R8G8B, X8R8G8B8)
    Format_PA                 = -7,      //!< Generic packed YUV input (YUY2, YUYV, YVYU, UYVY, VYUY)
    Format_PL2                = -6,      //!< Hybrid YUV input (NV12, NV11, P208)
    Format_PL2_UnAligned      = -5,      //!< Width or Height is not a multiple of 4
    Format_PL3                = -4,      //!< Planar YUV input (IMC1, IMC2, IMC3, IMC4)
    Format_PL3_RGB            = -3,      //!< Planar RGB input (RGBP, BGRP)
    Format_PAL                = -2,      //!< Generic Palletized input (AI44, IA44, P8)

    // Auxiliary Formats
    Format_None               = -1,      //!< No format (colorfill)
    Format_Any                =  0,      //!< Any format

    // RGB formats
    Format_A8R8G8B8    ,          //!< ARGB 32bpp (A = 255)
    Format_X8R8G8B8    ,          //!< XRGB 32bpp (X = 0)
    Format_A8B8G8R8    ,          //!< ABGR 32bpp
    Format_X8B8G8R8    ,          //!< XBGR 32bpp
    Format_A16B16G16R16,          //!< ABGR 64bpp
    Format_A16R16G16B16,          //!< ARGB 64bpp
    Format_R5G6B5      ,          //!< RGB 16bpp
    Format_R32U        ,          //!< R32U 32bpp
    Format_R32F        ,          //!< R32F 32bpp
    Format_R8G8B8      ,          //!< RGB 24bpp

    // Planar RGB formats
    Format_RGBP        ,          //!< R Plane, G Plane, B Plane
    Format_BGRP        ,          //!< B Plane, G Plane, R Plane

    // YUV packed 4:2:2 formats
    Format_YUY2        ,
    Format_YUYV        ,
    Format_YVYU        ,
    Format_UYVY        ,
    Format_VYUY        ,
    Format_Y216        , //422 16bit
    Format_Y210        , //422 10bit
    // YUV packed 4:4:4 formats
    Format_Y416        ,
    Format_AYUV        ,
    Format_AUYV        ,
    Format_Y410        , //444 10bit

    // Gray scale image with only Y plane
    Format_400P        ,

    // YUV PL2 formats
    Format_NV12               ,
    Format_NV12_UnAligned     ,
    Format_NV21               ,
    Format_NV11               ,
    Format_NV11_UnAligned     ,
    Format_P208               ,
    Format_P208_UnAligned     ,

    // YUV PL3 formats
    Format_IMC1        ,
    Format_IMC2        ,
    Format_IMC3        ,
    Format_IMC4        ,
    Format_422H        ,
    Format_422V        ,
    Format_444P        ,
    Format_411P        ,
    Format_411R        ,
    Format_I420        ,
    Format_IYUV        ,
    Format_YV12        ,
    Format_YVU9        ,

    // Palletized formats (RGB/YUV)
    Format_AI44        ,
    Format_IA44        ,
    Format_P8          ,
    Format_A8P8        ,

    // Alpha + Intensity
    Format_A8          ,
    Format_L8          ,
    Format_A4L4        ,
    Format_A8L8        ,

    // Bayer
    Format_IRW0        ,          //!< BGGR 10/12 bit depth [16bit aligned]
    Format_IRW1        ,          //!< RGGB 10/12 bit depth [16bit aligned]
    Format_IRW2        ,          //!< GRBG 10/12 bit depth [16bit aligned]
    Format_IRW3        ,          //!< GBRG 10/12 bit depth [16bit aligned]
    Format_IRW4        ,          //!< BGGR 8 bit depth
    Format_IRW5        ,          //!< RGGB 8 bit depth
    Format_IRW6        ,          //!< GRBG 8 bit depth
    Format_IRW7        ,          //!< GBRG 8 bit depth

    Format_STMM,

    Format_Buffer      ,          //!< Used for creating buffer resource (linear)
    Format_Buffer_2D   ,          //!< Encoder 2D linear buffer.

    Format_V8U8        ,

    Format_R32S        ,          //!< R32S 32bpp
    Format_R8U         ,          //!< R8 Uint
    Format_R8G8UN      ,          //!< R8G8 UNorm
    Format_R8G8SN      ,          //!< R8G8 SNorm
    Format_G8R8_G8B8   ,
    Format_R16U        ,
    Format_R16S        ,
    Format_R16UN       ,          //!< R16_UNorm
    Format_RAW         ,          //!< HW RAW format

    Format_Y8          ,          // R16F 16bpp
    Format_Y1          ,          // R16F 16bpp
    Format_Y16U        ,          // R16F 16bpp
    Format_Y16S        ,          // R16F 16bpp

    Format_L16         ,
    Format_D16         ,
    Format_R10G10B10A2 ,
    Format_B10G10R10A2 ,

    Format_P016        ,
    Format_P010        ,
    Format_YV12_Planar ,
    Format_A16B16G16R16F,          //!< ABGR 64bpp
    Format_R16G16UN     ,
    Format_R16F         ,
    Format_P210         ,
    Format_P216         ,
    Format_A16R16G16B16F,          //!< ARGB 64bpp
    Format_YUY2V        ,
    Format_Y216V        ,
    Format_D32F         ,
    Format_D24S8UN      ,
    Format_D32S8X24_FLOAT ,

    Format_R16          ,
    Format_R16G16S      ,
    Format_R24G8        ,
    Format_R32          ,
    Format_R32G8X24     ,
    Format_R8UN         ,           //!< R8 UNORM
    Format_R32G32B32A32F,           //ARGB 128bpp
    // Last Format
    Format_Count
} MOS_FORMAT, *PMOS_FORMAT;
C_ASSERT(Format_Count == 103); //!< When adding, update assert & vphal_solo_scenario.cpp::VpFromXml_GetFormat() & hal_kerneldll.c::g_cIsFormatYUV.

//!
//! \brief Macros for format checking
//!
#define IS_PAL_FORMAT(format)            \
            ( (format == Format_AI44) || \
              (format == Format_IA44) || \
              (format == Format_P8)   || \
              (format == Format_A8P8))

#define CASE_PAL_FORMAT  \
    case Format_AI44:    \
    case Format_IA44:    \
    case Format_P8:      \
    case Format_A8P8

#define IS_ALPHA4_FORMAT(format)         \
            ( (format == Format_AI44) || \
              (format == Format_IA44) )

#define IS_ALPHA_FORMAT(format)                   \
            ( (format == Format_A8R8G8B8)      || \
              (format == Format_A8B8G8R8)      || \
              (format == Format_R10G10B10A2)   || \
              (format == Format_B10G10R10A2)   || \
              (format == Format_A16B16G16R16)  || \
              (format == Format_A16R16G16B16)  || \
              (format == Format_A16B16G16R16F) || \
              (format == Format_A16R16G16B16F) || \
              (format == Format_Y410)          || \
              (format == Format_Y416)          || \
              (format == Format_AYUV) )

#define IS_PL2_FORMAT(format)            \
            ( (format == Format_PL2)  || \
              (format == Format_NV12) || \
              (format == Format_NV21) || \
              (format == Format_NV11) || \
              (format == Format_P208) || \
              (format == Format_P010) || \
              (format == Format_P016) )

#define IS_PL2_FORMAT_UnAligned(format)    \
            ( (format == Format_PL2_UnAligned)  || \
              (format == Format_NV12_UnAligned) || \
              (format == Format_NV11_UnAligned) || \
              (format == Format_P208_UnAligned) )

#define CASE_PL2_FORMAT  \
    case Format_PL2:     \
    case Format_NV12:    \
    case Format_NV21:    \
    case Format_NV11:    \
    case Format_P208:    \
    case Format_P010:    \
    case Format_P016

#define IS_PL3_FORMAT(format)            \
            ( (format == Format_PL3)  || \
              (format == Format_IMC1) || \
              (format == Format_IMC2) || \
              (format == Format_IMC3) || \
              (format == Format_IMC4) || \
              (format == Format_I420) || \
              (format == Format_IYUV) || \
              (format == Format_YV12) || \
              (format == Format_YVU9) || \
              (format == Format_422H) || \
              (format == Format_422V) || \
              (format == Format_411P) || \
              (format == Format_411R) || \
              (format == Format_444P) )

#define IS_PL3_RGB_FORMAT(format)        \
            ( (format == Format_RGBP) || \
              (format == Format_BGRP) )

#define CASE_PL3_FORMAT  \
    case Format_PL3:     \
    case Format_IMC1:    \
    case Format_IMC2:    \
    case Format_IMC3:    \
    case Format_IMC4:    \
    case Format_I420:    \
    case Format_IYUV:    \
    case Format_YV12:    \
    case Format_YVU9:    \
    case Format_422H:    \
    case Format_422V:    \
    case Format_411P:    \
    case Format_411R:    \
    case Format_444P

#define CASE_PL3_RGB_FORMAT  \
    case Format_RGBP:        \
    case Format_BGRP

#define IS_PA_FORMAT(format)             \
            ( (format == Format_PA)   || \
              (format == Format_YUY2) || \
              (format == Format_YUYV) || \
              (format == Format_YVYU) || \
              (format == Format_UYVY) || \
              (format == Format_VYUY) || \
              (format == Format_Y210) || \
              (format == Format_Y216) || \
              (format == Format_Y410) || \
              (format == Format_Y416) )

#define CASE_PA_FORMAT    \
    case Format_PA:       \
    case Format_YUY2:     \
    case Format_YUYV:     \
    case Format_YVYU:     \
    case Format_UYVY:     \
    case Format_VYUY

#define IS_YUV_FORMAT(format)              \
          ( IS_PL2_FORMAT(format)       || \
            IS_PL3_FORMAT(format)       || \
            IS_PA_FORMAT(format)        || \
            (format == Format_400P))

#define CASE_YUV_FORMAT  \
    CASE_PL2_FORMAT:     \
    CASE_PL3_FORMAT:     \
    CASE_PA_FORMAT:      \
    case Format_400P

#define IS_RGB64_FLOAT_FORMAT(format)             \
            ( (format == Format_A16B16G16R16F) || \
              (format == Format_A16R16G16B16F) )

#define IS_RGB64_FORMAT(format)                  \
            ( (format == Format_A16B16G16R16) || \
              (format == Format_A16R16G16B16))

#define IS_RGB32_FORMAT(format)                  \
            ( (format == Format_A8R8G8B8)     || \
              (format == Format_X8R8G8B8)     || \
              (format == Format_A8B8G8R8)     || \
              (format == Format_X8B8G8R8)     || \
              (format == Format_R10G10B10A2)  || \
              (format == Format_B10G10R10A2)  || \
              (format == Format_RGB32) )

#define IS_RGB16_FORMAT(format)              \
            (format == Format_R5G6B5)

#define IS_RGB24_FORMAT(format)              \
            (format == Format_R8G8B8)

#define IS_RGB_FORMAT(format)              \
            ( IS_RGB64_FORMAT(format)   || \
              IS_RGB32_FORMAT(format)   || \
              IS_RGB16_FORMAT(format)   || \
              IS_RGB24_FORMAT(format)   || \
              IS_PL3_RGB_FORMAT(format) || \
              (format == Format_RGB)    || \
              IS_RGB64_FLOAT_FORMAT(format) )

#define IS_RGB_NO_SWAP(format)                    \
            ( (format == Format_A8B8G8R8)      || \
              (format == Format_X8B8G8R8)      || \
              (format == Format_A16B16G16R16)  || \
              (format == Format_A16B16G16R16F) || \
              (format == Format_R32G32B32A32F) || \
              (format == Format_R10G10B10A2) )

#define IS_RGB_SWAP(format)                  \
            ( IS_RGB_FORMAT(format)       && \
              !(IS_RGB_NO_SWAP(format)) )

#define IS_RGB128_FORMAT(format)             \
              (format == Format_R32G32B32A32F)

#define CASE_RGB32_FORMAT    \
    case Format_A8R8G8B8:    \
    case Format_X8R8G8B8:    \
    case Format_A8B8G8R8:    \
    case Format_X8B8G8R8:    \
    case Format_R10G10B10A2: \
    case Format_B10G10R10A2: \
    case Format_RGB32

#define CASE_RGB24_FORMAT \
        case Format_R8G8B8

#define CASE_RGB16_FORMAT \
    case Format_R5G6B5

#define CASE_RGB_FORMAT   \
    CASE_RGB32_FORMAT:    \
    CASE_RGB24_FORMAT:    \
    CASE_RGB16_FORMAT:    \
    CASE_PL3_RGB_FORMAT:  \
    case Format_RGB

#define IS_BT601_CSPACE(format)                     \
        ( (format == CSpace_BT601)               || \
          (format == CSpace_xvYCC601)            || \
          (format == CSpace_BT601Gray)           || \
          (format == CSpace_BT601_FullRange)     || \
          (format == CSpace_BT601Gray_FullRange) )

#define IS_BT709_CSPACE(format)                     \
        ( (format == CSpace_BT709)               || \
          (format == CSpace_xvYCC709)            || \
          (format == CSpace_BT709_FullRange) )

#define IS_YUV_CSPACE_FULLRANGE(format)             \
        ( (format == CSpace_BT601_FullRange)     || \
          (format == CSpace_BT601Gray_FullRange) || \
          (format == CSpace_BT709_FullRange) )

#define CASE_YUV_CSPACE_FULLRANGE                   \
    case CSpace_BT601_FullRange:                    \
    case CSpace_BT601Gray_FullRange:                \
    case CSpace_BT709_FullRange

#define CASE_YUV_CSPACE_LIMITEDRANGE                \
    case CSpace_BT601:                              \
    case CSpace_BT601Gray:                          \
    case CSpace_xvYCC601:                           \
    case CSpace_BT709:                              \
    case CSpace_xvYCC709

#define IS_RGB_CSPACE_FULLRANGE(format)             \
        ( (format == CSpace_sRGB) )

#define IS_YUV_CSPACE(format)                       \
        ( IS_BT601_CSPACE(format)                || \
          IS_BT709_CSPACE(format) )

#define IS_RGB_CSPACE(format)                       \
        ( (format == CSpace_sRGB)                || \
          (format == CSpace_stRGB) )

#define IS_BAYER16_FORMAT(format)          \
            ( (format == Format_IRW0)   || \
              (format == Format_IRW1)   || \
              (format == Format_IRW2)   || \
              (format == Format_IRW3) )

#define CASE_BAYER16_FORMAT \
    case Format_IRW0:       \
    case Format_IRW1:       \
    case Format_IRW2:       \
    case Format_IRW3

#define IS_BAYER8_FORMAT(format)           \
            ( (format == Format_IRW4)   || \
              (format == Format_IRW5)   || \
              (format == Format_IRW6)   || \
              (format == Format_IRW7) )

#define CASE_BAYER8_FORMAT  \
    case Format_IRW4:       \
    case Format_IRW5:       \
    case Format_IRW6:       \
    case Format_IRW7

#define IS_BAYER_FORMAT(format)            \
            ( IS_BAYER16_FORMAT(format) || \
              IS_BAYER8_FORMAT(format) )

#define IS_BAYER_GRBG_FORMAT(format)       \
            ( (format == Format_IRW2)   || \
              (format == Format_IRW6) )

#define IS_BAYER_GBRG_FORMAT(format)       \
            ( (format == Format_IRW3)   || \
              (format == Format_IRW7) )

//!
//! \brief Tiling Enumeration, match GFX3DSTATE_TILEWALK
//!
#define MOS_YTILE_H_ALIGNMENT  32         // For Tile-Y and Tile-Yf
#define MOS_YTILE_W_ALIGNMENT  128
#define MOS_XTILE_H_ALIGNMENT  8
#define MOS_XTILE_W_ALIGNMENT  512
#define MOS_YSTILE_H_ALIGNMENT 256        // For Tile-Ys
#define MOS_YSTILE_W_ALIGNMENT 256

//!
//! \brief Enum for tile type
//!
typedef enum _MOS_TILE_TYPE
{
    MOS_TILE_X,
    MOS_TILE_Y,
    MOS_TILE_YF,            // 4KB tile
    MOS_TILE_YS,            // 64KB tile
    MOS_TILE_LINEAR,
    MOS_TILE_INVALID
} MOS_TILE_TYPE;
C_ASSERT(MOS_TILE_LINEAR == 4); //!< When adding, update assert

//!
//! \brief Enum for tile mode from GMM
//!
typedef enum _MOS_TILE_MODE_GMM : uint8_t
{
    MOS_TILE_LINEAR_GMM = 0,
    MOS_TILE_64_GMM,
    MOS_TILE_X_GMM,
    MOS_TILE_4_GMM,
} MOS_TILE_MODE_GMM;
C_ASSERT(MOS_TILE_4_GMM == 3);

#define IS_TILE_FORMAT(TileType)              \
            ( (MOS_TILE_X  == TileType) ||    \
              (MOS_TILE_Y  == TileType) ||    \
              (MOS_TILE_YF == TileType) ||    \
              (MOS_TILE_YS == TileType) )

#define IS_Y_MAJOR_TILE_FORMAT(TileType)      \
            ( (MOS_TILE_Y  == TileType) ||    \
              (MOS_TILE_YF == TileType) ||    \
              (MOS_TILE_YS == TileType) )

//!
//! \brief Structure to resource type
//!
typedef enum _MOS_GFXRES_TYPE
{
    MOS_GFXRES_INVALID = -1,
    MOS_GFXRES_BUFFER,  //!< Think malloc. This resource is a series of bytes. Is not 2 dimensional.
    MOS_GFXRES_2D,      //!< 2 dimensional resource w/ width and height. 1D is a subset of 2D.
    MOS_GFXRES_VOLUME,  //!< 3 dimensional resource w/ depth.
    MOS_GFXRES_SCRATCH, //!< scratch space buffer.
} MOS_GFXRES_TYPE;

//!
//! \brief Enum for Memory Compression Mode
//!
typedef enum _MOS_RESOURCE_MMC_MODE
{
    MOS_MMC_DISABLED,
    MOS_MMC_HORIZONTAL,
    MOS_MMC_VERTICAL,
    MOS_MMC_MC,
    MOS_MMC_RC
} MOS_RESOURCE_MMC_MODE;

#endif //__MOS_RESOURCE_DEFS_H__
