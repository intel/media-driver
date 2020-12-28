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
//!
//! \file     vp_render_kernel_obj.cpp
//! \brief    vp render kernel base object.
//! \details  vp render kernel base object will provided interface where sub kernels processing ways
//!
#include "vp_render_kernel_obj.h"
using namespace vp;

extern const vp::MEDIA_OBJECT_KA2_INLINE_DATA g_cInit_MEDIA_VP_OBJECT_KA2_INLINE_DATA =
{
    // DWORD 0
    {
        0,                                      // DestinationBlockHorizontalOrigin
        0                                       // DestinationBlockVerticalOrigin
    },

    // DWORD 1
    {
        0,                                      // HorizontalBlockCompositeMaskLayer0
        0                                       // VerticalBlockCompositeMaskLayer0
    },

    // DWORD 2
    {
        0,                                      // HorizontalBlockCompositeMaskLayer1
        0                                       // VerticalBlockCompositeMaskLayer1
    },

    // DWORD 3
    {
        0,                                      // HorizontalBlockCompositeMaskLayer2
        0                                       // VerticalBlockCompositeMaskLayer2
    },

    // DWORD 4
    0.0F,                                       // VideoXScalingStep

    // DWORD 5
    0.0F,                                       // VideoStepDelta

    // DWORD 6
    {
        0,                                      // VerticalBlockNumber
        0                                       // AreaOfInterest
    },

    // DWORD 7
    {
        0,                                      // GroupIDNumber
    },

    // DWORD 8
    {
        0,                                      // HorizontalBlockCompositeMaskLayer3
        0                                       // VerticalBlockCompositeMaskLayer3
    },

    // DWORD 9
    {
        0,                                      // HorizontalBlockCompositeMaskLayer4
        0                                       // VerticalBlockCompositeMaskLayer4
    },

    // DWORD 10
    {
        0,                                      // HorizontalBlockCompositeMaskLayer5
        0                                       // VerticalBlockCompositeMaskLayer5
    },

    // DWORD 11
    {
        0,                                      // HorizontalBlockCompositeMaskLayer6
        0                                       // VerticalBlockCompositeMaskLayer6
    },

    // DWORD 12
    {
        0,                                      // HorizontalBlockCompositeMaskLayer7
        0                                       // VerticalBlockCompositeMaskLayer7
    },

    // DWORD 13
    0,                                          // Reserved

    // DWORD 14
    0,                                          // Reserved

    // DWORD 15
    0                                           // Reserved
};


vp::VpRenderKernelObj::VpRenderKernelObj(PVP_MHWINTERFACE hwInterface) :
    m_hwInterface(hwInterface)
{
}