/*
* Copyright (c) 2026, Intel Corporation
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
//! \file     media_interfaces_nvl.h
//! \brief    All interfaces used for NVL that require factory creation
//!

#ifndef __MEDIA_INTERFACES_XE3P_LPM_H__
#define __MEDIA_INTERFACES_XE3P_LPM_H__

#include "media_interfaces_nvls.h"
#include "mhw_state_heap_xe3p_lpg.h"
#include "renderhal_xe3p_lpg.h"

class MhwInterfacesNvl : public MhwInterfacesNvl_Xe3G
{
public:
    using StateHeap = MHW_STATE_HEAP_INTERFACE_XE3P_LPG;

protected:
    void InitializeRenderComponent(
        CreateParams   params,
        PMOS_INTERFACE osInterface) override;

    MEDIA_CLASS_DEFINE_END(MhwInterfacesNvl)
};

class VphalInterfacesNvl : public VphalInterfacesNvl_Xe3G
{
protected:
    void InitPlatformKernelBinary(
        vp::VpPlatformInterface *&vpPlatformInterface) override;

    MEDIA_CLASS_DEFINE_END(VphalInterfacesNvl)
};

class RenderHalInterfacesNvl : public RenderHalDevice
{
protected:
    using XRenderHal = XRenderHal_Interface_Xe3P_Lpg;
    MOS_STATUS Initialize();

    MEDIA_CLASS_DEFINE_END(RenderHalInterfacesNvl)
};

#endif  // __MEDIA_INTERFACES_XE3P_LPM_H__
