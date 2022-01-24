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
//! \file     vphal_common_hdr.h
//! \brief    vphal HDR interface clarification
//! \details  vphal HDR interface clarification inlcuding:
//!           some marcro, enum, structure, function
//!
#ifndef __VPHAL_COMMON_HDR_H__
#define __VPHAL_COMMON_HDR_H__

#include "vp_common_hdr.h"

//!
//! Structure VPHAL_CCM_TYPE
//! \brief Color Correction Matrix Type
//!
typedef enum _VPHAL_HDR_CCM_TYPE
{
    VPHAL_HDR_CCM_NONE = 0,
    VPHAL_HDR_CCM_BT2020_TO_BT601_BT709_MATRIX,
    VPHAL_HDR_CCM_BT601_BT709_TO_BT2020_MATRIX,
    VPHAL_HDR_CCM_BT2020_TO_MONITOR_MATRIX,
    VPHAL_HDR_CCM_MONITOR_TO_BT2020_MATRIX,
    VPHAL_HDR_CCM_MONITOR_TO_BT709_MATRIX
} VPHAL_HDR_CCM_TYPE;


//!
//! Structure VPHAL_CSC_TYPE
//! \brief Color Space Coversion Type
//!
typedef enum _VPHAL_HDR_CSC_TYPE
{
    VPHAL_HDR_CSC_NONE = 0,
    VPHAL_HDR_CSC_YUV_TO_RGB_BT601,
    VPHAL_HDR_CSC_YUV_TO_RGB_BT709,
    VPHAL_HDR_CSC_YUV_TO_RGB_BT2020,
    VPHAL_HDR_CSC_RGB_TO_YUV_BT601,
    VPHAL_HDR_CSC_RGB_TO_YUV_BT709,
    VPHAL_HDR_CSC_RGB_TO_YUV_BT709_FULLRANGE,
    VPHAL_HDR_CSC_RGB_TO_YUV_BT2020
} VPHAL_HDR_CSC_TYPE;

#endif  // __VPHAL_COMMON_HDR_H__
