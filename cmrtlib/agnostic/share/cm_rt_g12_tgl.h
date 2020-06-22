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
//!
//! \file      cm_rt_g12_tgl.h
//! \brief     Contains Definitions for CM on Gen 12 TGL
//!

#ifndef __CM_RT_G12_TGL_H__
#define __CM_RT_G12_TGL_H__

#define TGL_L3_PLANE_DEFAULT    CM_L3_PLANE_DEFAULT
#define TGL_L3_PLANE_1          CM_L3_PLANE_1
#define TGL_L3_PLANE_2          CM_L3_PLANE_2
#define TGL_L3_PLANE_3          CM_L3_PLANE_3
#define TGL_L3_PLANE_4          CM_L3_PLANE_4
#define TGL_L3_PLANE_5          CM_L3_PLANE_5
#define TGL_L3_PLANE_6          CM_L3_PLANE_6
#define TGL_L3_CONFIG_COUNT     7


static const L3ConfigRegisterValues TGL_L3_PLANES[TGL_L3_CONFIG_COUNT] =
{                                    //  URB  Rest  DC  RO   Z    Color  UTC  CB  Sum (in KB)
    {0xD0000020, 0x4,        0, 0},  //  64   416   0   0    0    0      0    0   480
    {0x78000040, 0x306044,   0, 0},  //  128  240   0   0    48   48     0    16  480
    {0x21E020,   0x408044,   0, 0},  //  64   0     32  240  64   64     0    16  480
    {0x48000020, 0x810044,   0, 0},  //  64   144   0   0    128  128    0    16  480
    {0x18000020, 0xB0000044, 0, 0},  //  64   48    0   0    0    0      352  16  480
    {0x88000020, 0x40000044, 0, 0},  //  64   272   0   0    0    0      128  16  480
    {0xC8000020, 0x44,       0, 0},  //  64   400   0   0    0    0      0    16  480
};

#endif //__CM_RT_G12_TGL_H__
