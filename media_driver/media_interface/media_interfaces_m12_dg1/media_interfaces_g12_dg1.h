/*
* Copyright (c) 2011-2021, Intel Corporation
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
//! \file     media_interfaces_g12_dg1.h
//! \brief    All interfaces used for DG1 that require factory creation
//!

#ifndef __MEDIA_INTERFACES_G12_DG1_H__
#define __MEDIA_INTERFACES_G12_DG1_H__

#include "media_interfaces_g12_tgllp.h"
#include "renderhal_g12_1.h"

#define DG1_L3_CONFIG_NUM                      3
static const L3ConfigRegisterValues DG1_L3_PLANE[] = {
    //L3Alloc     TCCNTL                 //  Rest   DC  RO   Z    Color  UTC    CB    Sum (in KB)
    { 0x00000200, 0x0,          0, 0 },  //  2048   0   0    0    0       0      0    2048
    { 0x40000000, 0x3E000010,   0, 0 },  //  1024   0   0    0    0       992   32    2048
    { 0x0080F800, 0x00000010,   0, 0 },  //  0    1024  992  0    0       0     32    2048
};

class RenderHalInterfacesG12Dg1 : public RenderHalDevice
{
protected:
    using XRenderHal = XRenderHal_Interface_G12_1;
    MOS_STATUS Initialize();
};

class CMHalInterfacesG12Dg1 : public CMHalDevice
{
protected:
    using CMHal = CM_HAL_G12_X;
    MOS_STATUS Initialize(
        CM_HAL_STATE* cmState);
};

#endif // __MEDIA_INTERFACES_G12_DG1_H__
