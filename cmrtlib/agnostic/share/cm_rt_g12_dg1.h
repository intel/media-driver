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
//! \file      cm_rt_g12_dg1.h
//! \brief     Contains Definitions for CM on Gen 12 DG1
//!

#ifndef __CM_RT_G12_DG1_H__
#define __CM_RT_G12_DG1_H__

#define DG1_L3_PLANE_DEFAULT    CM_L3_PLANE_DEFAULT
#define DG1_L3_PLANE_1          CM_L3_PLANE_1
#define DG1_L3_PLANE_2          CM_L3_PLANE_2
#define DG1_L3_CONFIG_COUNT     3

// 16KB per Way for DG1, two Way per section
static const L3ConfigRegisterValues DG1_L3_PLANES[DG1_L3_CONFIG_COUNT] =
{                                      //  Rest  DC    RO  Z    Color  UTC  CB  Sum (in KB)
    {0x00000200, 0,          0, 0},    //  2048  0     0   0    0      0    0   2048
    {0x80000000, 0x7C000020, 0, 0},    //  1024  0     0   0    0      992  32  2048
    {0x0101F000, 0x00000020, 0, 0},    //  0     1024  992 0    0      0    32  2048
};

#endif //__CM_RT_G12_DG1_H__
