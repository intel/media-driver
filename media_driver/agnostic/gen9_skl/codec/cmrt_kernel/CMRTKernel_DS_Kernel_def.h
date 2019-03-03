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
//! \file     CMRTKernel_DS_Kernel_def.h
//! \brief    HEVC FEI 2xScaling parameter definition for GEN9 SKL.
//!

#ifndef _CMRTKERNEL_DS_KERNEL_DEF_
#define _CMRTKERNEL_DS_KERNEL_DEF_

#include "Hme_Downscale_gen9.h"

#define  HEVCENCKERNELNAME_2xDS_FRAME      "hme_frame_downscale2"

typedef struct
{
    void     *m_cmSurfDS_TopIn;
    void     *m_cmSurfDS_TopOut;
    void     *m_cmSurfTopVProc;
} DownScalingKernelParams;

#endif
