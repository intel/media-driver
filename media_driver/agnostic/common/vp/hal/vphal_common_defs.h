/*
* Copyright (c) 2022, Intel Corporation
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
//! \file     vphal_common_defs.h
//! \brief    clarify common utilities for vphal
//! \details  clarify common utilities for vphal including:
//!           some marcro, enum, union, structure, function
//!
#ifndef __VPHAL_COMMON_DEFS_H__
#define __VPHAL_COMMON_DEFS_H__

#include "vp_common.h"

//!
//! \brief Vphal Output chroma configuration enum
//!
typedef enum _VPHAL_CHROMA_SUBSAMPLING
{
    CHROMA_SUBSAMPLING_TOP_CENTER       = 0,
    CHROMA_SUBSAMPLING_CENTER_CENTER,
    CHROMA_SUBSAMPLING_BOTTOM_CENTER,
    CHROMA_SUBSAMPLING_TOP_LEFT,
    CHROMA_SUBSAMPLING_CENTER_LEFT,
    CHROMA_SUBSAMPLING_BOTTOM_LEFT
} VPHAL_CHROMA_SUBSAMPLING;

//!
//! \brief Vphal Gamma Values configuration enum
//!
typedef enum _VPHAL_GAMMA_VALUE
{
    GAMMA_1P0 = 0,
    GAMMA_2P2,
    GAMMA_2P6
} VPHAL_GAMMA_VALUE;

typedef enum _VPHAL_DP_ROTATION_MODE
{
    VPHAL_DP_ROTATION_NV12_AVG            = 0,   //!< nv12 -> yuy2 by chroma average
    VPHAL_DP_ROTATION_NV12_NV12              ,   //!< nv12 -> nv12
    VPHAL_DP_ROTATION_NV12_REP               ,   //!< nv12 -> yuy2 by chroma repeat
    VPHAL_DP_ROTATION_NV12_YUY2_NOT_SET          //!< nv12 -> yuy2 by chroma average or repeat, decided by scaling mode
} VPHAL_DP_ROTATION_MODE;

//!
//! Union   VPHAL_HALF_PRECISION_FLOAT
//! \brief  Vphal half precision float type
//!
typedef union _VPHAL_HALF_PRECISION_FLOAT
{
    struct
    {
        uint16_t      Mantissa : 10;
        uint16_t      Exponent : 5;
        uint16_t      Sign     : 1;
    };

    uint16_t value;
} VPHAL_HALF_PRECISION_FLOAT, PVPHAL_HALF_PRECISION_FLOAT;


//!
//! \brief Vphal 3DLUT Channel Mapping enum
//!
typedef enum _VPHAL_3DLUT_CHANNEL_MAPPING
{
    CHANNEL_MAPPING_RGB_RGB          = 0,
    CHANNEL_MAPPING_YUV_RGB          = 1 << 0,
    CHANNEL_MAPPING_VUY_RGB          = 1 << 1,
} VPHAL_3DLUT_CHANNEL_MAPPING;


//!
//! Structure VPHAL_GAMUT_PARAMS
//! \brief IECP Gamut Mapping Parameters
//!
typedef struct _VPHAL_GAMUT_PARAMS
{
    VPHAL_GAMUT_MODE    GCompMode;
    VPHAL_GAMUT_MODE    GExpMode;
    VPHAL_GAMMA_VALUE   GammaValue;
    uint32_t            dwAttenuation;       //!< U2.10 [0, 1024] 0 = No down scaling, 1024 = Full down scaling
    float               displayRGBW_x[4];
    float               displayRGBW_y[4];
} VPHAL_GAMUT_PARAMS, *PVPHAL_GAMUT_PARAMS;

//!
//! Structure VPHAL_CONSTRICTION_PARAMS
//! \brief Constriction parameters
//!
typedef struct _VPHAL_CONSTRICTION_PARAMS
{
    RECT                rcConstriction;
} VPHAL_CONSTRICTION_PARAMS, *PVPHAL_CONSTRICTION_PARAMS;

#endif  // __VPHAL_COMMON_DEFS_H__