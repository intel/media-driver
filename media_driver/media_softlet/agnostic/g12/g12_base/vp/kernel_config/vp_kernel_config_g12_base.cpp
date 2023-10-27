/*
* Copyright (c) 2021, Intel Corporation
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
//! \file     vp_kernel_config_g12_base.cpp
//! \brief    vp kernel config
//! \details  vp kernel config
//!

#include "vp_kernel_config_g12_base.h"
#include "vp_utils.h"
#include "vphal.h"

using namespace vp;

VpKernelConfigG12_Base::VpKernelConfigG12_Base()
{
    InitKernelParams();
}

VpKernelConfigG12_Base::~VpKernelConfigG12_Base()
{
}

void VpKernelConfigG12_Base::InitKernelParams()
{
    ///*                                             GRF_Count
    //                                               |  BT_Count
    //                                               |  |    Sampler_Count
    //                                               |  |    |  Thread_Count
    //                                               |  |    |  |                          GRF_Start_Register
    //                                               |  |    |  |                          |   CURBE_Length
    //                                               |  |    |  |                          |   |    block_width
    //                                               |  |    |  |                          |   |    |                       block_height
    //                                               |  |    |  |                          |   |    |                       |                       blocks_x
    //                                               |  |    |  |                          |   |    |                       |                       |   blocks_y
    //                                               |  |    |  |                          |   |    |                       |                       |   |*/
    ADD_VP_KERNEL_PARAMS(kernelCombinedFc,           7, 40,  3, VP_USE_MEDIA_THREADS_MAX,  0,  6,   VP_COMP_BLOCK_WIDTH,    VP_COMP_BLOCK_HEIGHT,   1,  1);
    ADD_VP_KERNEL_PARAMS(kernelVeboxUpdateDnState,   4, 34,  0, VP_USE_MEDIA_THREADS_MAX,  0,  2,   64,                     8,                      1,  1);
    ADD_VP_KERNEL_PARAMS(kernelHdrMandatory,         8, 40,  4, VP_USE_MEDIA_THREADS_MAX,  0,  8,   16,                     8,                      1,  1);
    ADD_VP_KERNEL_PARAMS(kernelHdr3DLutCalc,         4, 34,  0, VP_USE_MEDIA_THREADS_MAX,  0, 44,   64,                     8,                      1,  1);
}
