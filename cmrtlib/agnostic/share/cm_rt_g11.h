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
//! \file      cm_rt_g11.h
//! \brief     Contains Definitions for CM on Gen 11
//!

#ifndef __CM_RT_G11_H__
#define __CM_RT_G11_H__

#define ICL_L3_PLANE_DEFAULT    CM_L3_PLANE_DEFAULT
#define ICL_L3_PLANE_1          CM_L3_PLANE_1
#define ICL_L3_PLANE_2          CM_L3_PLANE_2
#define ICL_L3_PLANE_3          CM_L3_PLANE_3
#define ICL_L3_PLANE_4          CM_L3_PLANE_4
#define ICL_L3_PLANE_5          CM_L3_PLANE_5
#define ICL_L3_CONFIG_COUNT     6

#define ICLLP_L3_PLANE_DEFAULT  CM_L3_PLANE_DEFAULT
#define ICLLP_L3_PLANE_1        CM_L3_PLANE_1
#define ICLLP_L3_PLANE_2        CM_L3_PLANE_2
#define ICLLP_L3_PLANE_3        CM_L3_PLANE_3
#define ICLLP_L3_PLANE_4        CM_L3_PLANE_4
#define ICLLP_L3_PLANE_5        CM_L3_PLANE_5
#define ICLLP_L3_PLANE_6        CM_L3_PLANE_6
#define ICLLP_L3_PLANE_7        CM_L3_PLANE_7
#define ICLLP_L3_PLANE_8        CM_L3_PLANE_8
#define ICLLP_L3_CONFIG_COUNT   9

static const L3ConfigRegisterValues ICLLP_L3_PLANES[ICLLP_L3_CONFIG_COUNT] =
{                                    //  URB  Rest  DC  RO   Z    Color  UTC  CB  Sum (in KB)
    {0x80000080, 0xD,        0, 0},  //  128  128   0   0    0    0      0    0   256
    {0x70000080, 0x40804D,   0, 0},  //  128  112   0   0    64   64     0    16  384
    {0x41C060,   0x40804D,   0, 0},  //  96   0     32  112  64   64     0    16  384
    {0x2C040,    0x20C04D,   0, 0},  //  64   0     0   176  32   96     0    16  384
    {0x30000040, 0x81004D,   0, 0},  //  64   48    0   0    128  128    0    16  384
    {0xC040,     0x8000004D, 0, 0},  //  64   0     0   48   0    0      256  16  384
    {0xA0000420, 0xD,        0, 0},  //  64   320   0   0    0    0      0    0   384
    {0xC0000040, 0x4000000D, 0, 0},  //  64   192   0   0    0    0      128  0   384
    {0xB0000040, 0x4000004D, 0, 0},  //  64   176   0   0    0    0      128  16  384
};

#endif //__CM_RT_G11_H__
