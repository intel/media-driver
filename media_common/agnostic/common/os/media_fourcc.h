/*
* Copyright (c) 2017, Intel Corporation
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
//! \file      media_fourcc.h 
//! \brief     FourCC color format definitions.
//!
#ifndef _MEDIA_FOURCC_H
#define _MEDIA_FOURCC_H

#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3)  \
                  ((uint32_t)(uint8_t)(ch0) | ((uint32_t)(uint8_t)(ch1) << 8) |  \
                  ((uint32_t)(uint8_t)(ch2) << 16) | ((uint32_t)(uint8_t)(ch3) << 24 ))
#endif // MAKEFOURCC

// Use the above Equation to get the Fourcc Values

#define FOURCC_YUY2   0x32595559   // Normal YUV 4:2:2 format YUYV..
#define FOURCC_UYVY   0x59565955   // YUV 4:2:2 format Y Swapped UYVY..
#define FOURCC_VBID   0x44494256
#define FOURCC_YVYU   0x55595659   // YUV 4:2:2 format UV Swapped YVYU
#define FOURCC_I420   0x30323449   // YUV 4:2:0 format planner
#define FOURCC_IYUV   0x56555949   // YUV 4:2:0 format Same as I420
#define FOURCC_YV12   0x32315659   // Planner YUV 4:2:0 format UV Swapped
#define FOURCC_AI44   0x34344941   // Fourcc for the Motion Comp Alpha Blend Surface
#define FOURCC_IA44   0x34344149   // same as AI44
#define FOURCC_IF09   0x39304649   // Planar 4:1:0
#define FOURCC_YVU9   0x39555659   // Planar 4:1:0
#define FOURCC_VYUY   0x59555956   // YUV 4:2:2 format Y and UV Swapped YUYV
#define FOURCC_2YUY   0x59555932   // Swizzle format of YUY2
#define FOURCC_Y41P   0x50313459   // YUV 4:1:1 format packed
#define FOURCC_IMC1   0x31434D49   // as YV12, except the same UV stride as Y
#define FOURCC_IMC2   0x32434D49   // as IMC1, except UV lines are interleaved at half-stride boundaries
#define FOURCC_IMC3   0x33434D49   // as IMC1, except the UV are swaped
#define FOURCC_IMC4   0x34434D49   // as IMC2, except UV are swaped
#define FOURCC_422P   0x50323234   // YUV 4:2:2 planar, internal use only.
#define FOURCC_NV12   0x3231564e   // YUV 4:2:0 planar, for media
#define FOURCC_NV11   0x3131564e   // YUV 4:1:1 planar, for media
#define FOURCC_NV21   0x3132564e   // as NV12 with U and V reversed in the interleaved plane
#define FOURCC_P016   0x36313050   // YUV 4:2:0 planar, for media
#define FOURCC_P010   0x30313050   // YUV 4:2:0 planar, for media
#define FOURCC_P208   0x38303250   // YUV 4:2:2 planar, for media
#define FOURCC_AYUV   0x56555941   // Format packed for HD media
#define FOURCC_VXUY   0x59555856   // Swizzle format of AYUV
#define FOURCC_Y210   0x30313259   // YUV 4:2:2 packed, similar to Y216 but with 10 bits per channel
#define FOURCC_Y216   0x36313259   // YUV 4:2:2 packed, similar to YUYV but with 16 bits per channel
#define FOURCC_Y410   0x30313459   // YUV 4:4:4 packed, simple UYVA with 10 bits per channel and 2 for alpha
#define FOURCC_Y416   0x36313459   // YUV 4:4:4 packed, simple UYVA with 16 bits for all channels

// nullptr format indicates that the driver should not allocate any video memory
#define FOURCC_NULL   MAKEFOURCC( 'N', 'U', 'L', 'L' )

// Formats used for JPEG decode
#define FOURCC_422H    MAKEFOURCC('4','2','2','H')
#define FOURCC_422V    MAKEFOURCC('4','2','2','V')
#define FOURCC_411P    MAKEFOURCC('4','1','1','P')
#define FOURCC_411R    MAKEFOURCC('4','1','1','R')
#define FOURCC_444P    MAKEFOURCC('4','4','4','P')
#define FOURCC_RGBP    MAKEFOURCC('R','G','B','P')
#define FOURCC_BGRP    MAKEFOURCC('B','G','R','P')
#define FOURCC_400P    MAKEFOURCC('4','0','0','P')
#define FOURCC_420O    MAKEFOURCC('4','2','0','O')

// Bayer Pattern
#define FOURCC_IRW0    MAKEFOURCC('I','R','W','0')  // BGGR 10/12 bit depth [16bit aligned]
#define FOURCC_IRW1    MAKEFOURCC('I','R','W','1')  // RGGB 10/12 bit depth [16bit aligned]
#define FOURCC_IRW2    MAKEFOURCC('I','R','W','2')  // GRBG 10/12 bit depth [16bit aligned]
#define FOURCC_IRW3    MAKEFOURCC('I','R','W','3')  // GBRG 10/12 bit depth [16bit aligned]
#define FOURCC_IRW4    MAKEFOURCC('I','R','W','4')  // BGGR 8 bit depth
#define FOURCC_IRW5    MAKEFOURCC('I','R','W','5')  // RGGB 8 bit depth
#define FOURCC_IRW6    MAKEFOURCC('I','R','W','6')  // GRBG 8 bit depth
#define FOURCC_IRW7    MAKEFOURCC('I','R','W','7')  // GBRG 8 bit depth

#endif //_MEDIA_FOURCC_H
