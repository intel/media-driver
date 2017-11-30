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
//! \file      cm_rt_g10.h
//! \brief     Contains Definitions for CM on Gen 10
//!

#ifndef __CM_RT_G10_H__
#define __CM_RT_G10_H__

#include "cm_hw_vebox_cmd_g10.h"

#define CNL_L3_PLANE_DEFAULT    CM_L3_PLANE_DEFAULT
#define CNL_L3_PLANE_1          CM_L3_PLANE_1
#define CNL_L3_PLANE_2          CM_L3_PLANE_2
#define CNL_L3_PLANE_3          CM_L3_PLANE_3
#define CNL_L3_PLANE_4          CM_L3_PLANE_4
#define CNL_L3_PLANE_5          CM_L3_PLANE_5
#define CNL_L3_PLANE_6          CM_L3_PLANE_6
#define CNL_L3_PLANE_7          CM_L3_PLANE_7
#define CNL_L3_PLANE_8          CM_L3_PLANE_8
#define CNL_L3_CONFIG_COUNT     9

static const L3ConfigRegisterValues CNL_L3_PLANES[CNL_L3_CONFIG_COUNT] =
{                            // SLM  URB  Rest  DC   RO   Sum (in KB)
    {0, 0, 0, 0x80000080},   // 0    128  128   0    0    256
    {0, 0, 0, 0x418080},     // 0    128  0     32   96   256
    {0, 0, 0, 0x420060},     // 0    96   0     32   128  256
    {0, 0, 0, 0x30040},      // 0    64   0     0    192  256
    {0, 0, 0, 0xC0000040},   // 0    64   192   0    0    256
    {0, 0, 0, 0x428040},     // 0    64   0     32   160  256
    {0, 0, 0, 0xA0000021},   // 32   32   160   0    0    256
    {0, 0, 0, 0x1008021},    // 32   32   0     128  32   256
    {0, 0, 0, 0xC0000001}    // 32   0    192   0    0    256
};

typedef struct __CM_VEBOX_PARAM_G10
{
    unsigned char                padding1[4024];
    unsigned char                padding2[3732];
    PVEBOX_GAMUT_STATE_G75       pGamutState;
    unsigned char                padding3[3936];
    PVEBOX_VERTEX_TABLE_G75      pVertexTable;
    unsigned char                padding4[2048];

    CmHwVeboxCmdG10::VEBOX_CAPTURE_PIPE_STATE_CMD *pCapturePipe;
    CmHwVeboxCmdG10::VEBOX_DNDI_STATE_CMD         *pDndiState;
    CmHwVeboxCmdG10::VEBOX_IECP_STATE_CMD         *pIecpState;
}CM_VEBOX_PARAM_G10, PCM_VEBOX_PARAM_G10;

#endif //__CM_RT_G10_H__
