/*
* Copyright (c) 2020, Intel Corporation
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

#ifndef __MEDIA_COMMON_DEFS_H__
#define __MEDIA_COMMON_DEFS_H__

//!
//! \brief Color Spaces enum
//!
typedef enum _MEDIA_CSPACE
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
} MEDIA_CSPACE;
C_ASSERT(CSpace_Count == 15);       //!< When adding, update assert & vphal_solo_scenario.cpp

//!
//! \brief Rotation Mode enum
//!
typedef enum _MEDIA_ROTATION
{
    ROTATION_IDENTITY               = 0 ,   //!< Rotation 0 degrees
    ROTATION_90                         ,   //!< Rotation 90 degrees
    ROTATION_180                        ,   //!< Rotation 180 degrees
    ROTATION_270                        ,   //!< Rotation 270 degrees
    ROTATION_MIRROR_HORIZONTAL          ,   //!< Horizontal Mirror
    ROTATION_MIRROR_VERTICAL            ,   //!< Vertical Mirror
    ROTATION_90_MIRROR_VERTICAL         ,   //!< 90 + V Mirror
    ROTATION_90_MIRROR_HORIZONTAL           //!< 90 + H Mirror
} MEDIA_ROTATION;


#endif // __MEDIA_COMMON_DEFS_H__