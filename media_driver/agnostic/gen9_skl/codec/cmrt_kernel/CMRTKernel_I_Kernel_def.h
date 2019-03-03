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
//! \file     CMRTKernel_I_Kernel_def.h
//! \brief    HEVC FEI I parameter definition for GEN9 SKL.
//!

#ifndef _CMRTKERNEL_I_KERNEL_DEF_
#define _CMRTKERNEL_I_KERNEL_DEF_

#include "HevcEncFei_I_gen9.h"

#define NUM_MBENC_I_32x32_SURFACES                10
#define NUM_MBENC_I_16x16_SAD_SURFACES             6
#define NUM_MBENC_I_16X16_MODE_SURFACES           12
#define NUM_MBENC_I_8x8_PU_SURFACES                9
#define NUM_MBENC_I_8X8_PU_MODE_LCU_SURFACES      15

#define CURBEDATA_SIZE_I_32X32_PU_MODE_DECISION   32
#define CURBEDATA_SIZE_I_16X16_SAD_COMPUTE        32
#define CURBEDATA_SIZE_I_16X16_MODE_DECISION      64
#define CURBEDATA_SIZE_I_8X8_PU                   32
#define CURBEDATA_SIZE_I_8X8_PU_MODE_LCU          64

#define HEVCENCKERNELNAME_I_32x32                 "Hevc_LCUEnc_I_32x32_PU_ModeDecision"
#define HEVCENCKERNELNAME_I_16x16SAD              "HEVC_LCUEnc_I_16x16_PU_SADComputation"
#define HEVCENCKERNELNAME_I_16x16MODE             "HEVC_LCUEnc_I_16x16_PU_ModeDecision"
#define HEVCENCKERNELNAME_I_8x8                   "Hevc_LCUEnc_I_8x8_PU"
#define HEVCENCKERNELNAME_I_8x8MODE               "Hevc_LCUEnc_I_8x8_PU_FMode_inLCU"

typedef struct
{
    void     *m_cmSurfPer32x32PUDataOut;
    void     *m_cmSurfCurrY;
    void     *m_cmSurfCurrY2;
    void     *m_cmSurfSliceMap;
    void     *m_cmSurfCombinedQP;
    void     *m_cmLCUQPSurf;
    void     *m_cmBRCConstSurf;
    void     *m_cmSurfSAD16x16;
    void     *m_cmSurfSIF;
    void     *m_cmSurfPOCDbuf;
    void     *m_cmSurfVMEMode;
    void     *m_cmSurfMode;
    void     *m_cmSurfIntraDist;
    void     *m_cmSurfHaarDist;
    void     *m_cmSurfFrameStats;
    void     *m_cmSurfStats;
    uint32_t  m_bufSize;
    uint32_t  m_bufOffset;
} IFrameKernelParams;

#endif
