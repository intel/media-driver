/*
* Copyright (c) 2018, Intel Corporation
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
//! \file     vp_mem_compression.cpp
//! \brief    Defines the common interface for mmc
//! \details  The mmc is to handle mmc operations,
//! including compression and decompressin of vp and codec
//!
#include "mos_defs.h"
#include "vp_mem_compression.h"
#include "vp_platform_interface.h"

VPMediaMemComp::VPMediaMemComp(
    PMOS_INTERFACE    osInterface,
    VP_MHWINTERFACE  &vpInterface)
    :MediaMemComp(osInterface)
{
    m_mmcEnabledKey     = __VPHAL_ENABLE_MMC;
    m_mmcInUseKey       = __MEDIA_USER_FEATURE_VALUE_VP_MMC_IN_USE;

#if (LINUX) && (!WDDM_LINUX)
    m_bComponentMmcEnabled = !MEDIA_IS_WA(vpInterface.m_waTable, WaDisableVPMmc);
#else
    m_bComponentMmcEnabled = true;
#endif

    if (vpInterface.m_vpPlatformInterface &&
        !vpInterface.m_vpPlatformInterface->IsPlatformCompressionEnabled())
    {
        m_bComponentMmcEnabled = false;
    }

    InitMmcEnabled();
}

MOS_STATUS VPMediaMemComp::DecompressVPResource(PVPHAL_SURFACE surface)
{
    VP_FUNC_CALL();

    if (surface)
    {
        return DecompressResource(&surface->OsResource);
    }

    return MOS_STATUS_SUCCESS;
}
