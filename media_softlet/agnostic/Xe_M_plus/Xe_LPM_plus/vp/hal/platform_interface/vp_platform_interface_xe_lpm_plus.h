/*
* Copyright (c) 2020-2021, Intel Corporation
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
//! \file     vp_platform_interface_xe_lpm_plus.h
//! \brief    platform specific vp interfaces.
//!
#ifndef __VP_PLATFORM_INTERFACE_XE_LPM_PLUS_H__
#define __VP_PLATFORM_INTERFACE_XE_LPM_PLUS_H__

#include "vp_platform_interface.h"

//!
//! \brief Vebox Statistics Surface definition for Xe_LPM_plus
//!
#define VP_VEBOX_STATISTICS_SIZE_XE_LPM_PLUS                             (32 * 8)
#define VP_VEBOX_STATISTICS_PER_FRAME_SIZE_XE_LPM_PLUS                   (32 * sizeof(uint32_t))
#define VP_VEBOX_STATISTICS_SURFACE_FMD_OFFSET_XE_LPM_PLUS               0
#define VP_VEBOX_STATISTICS_SURFACE_GNE_OFFSET_XE_LPM_PLUS               0x2C
#define VP_VEBOX_STATISTICS_SURFACE_STD_OFFSET_XE_LPM_PLUS               0x44
#define VP_VEBOX_STATISTICS_SURFACE_SW_WA_OFFSET_XE_LPM_PLUS             -12

namespace vp
{

class VpPlatformInterfacesXe_Lpm_Plus : public VpPlatformInterface
{
public:

    VpPlatformInterfacesXe_Lpm_Plus(PMOS_INTERFACE pOsInterface, bool clearViewMode = false);

    virtual ~VpPlatformInterfacesXe_Lpm_Plus()
    {}

    virtual MOS_STATUS InitVpVeboxSfcHwCaps(VP_VEBOX_ENTRY_REC *veboxHwEntry, uint32_t veboxEntryCount, VP_SFC_ENTRY_REC *sfcHwEntry, uint32_t sfcEntryCount);
    virtual VPFeatureManager *CreateFeatureChecker(_VP_MHWINTERFACE *hwInterface);
    virtual VpCmdPacket *CreateVeboxPacket(MediaTask * task, _VP_MHWINTERFACE *hwInterface, VpAllocator *&allocator, VPMediaMemComp *mmc);
    virtual MOS_STATUS CreateSfcRender(SfcRenderBase *&sfcRender, VP_MHWINTERFACE &vpMhwinterface, PVpAllocator allocator);
    virtual VpCmdPacket *CreateRenderPacket(MediaTask * task, _VP_MHWINTERFACE *hwInterface, VpAllocator *&allocator, VPMediaMemComp *mmc, VpKernelSet* kernel);
    virtual MOS_STATUS VeboxQueryStatLayout(
        VEBOX_STAT_QUERY_TYPE queryType,
        uint32_t* pQuery);
    virtual uint32_t        VeboxQueryStaticSurfaceSize()
    {
        return VP_VEBOX_STATISTICS_SIZE_XE_LPM_PLUS;
    }

    virtual MOS_STATUS ConfigVirtualEngine();

    virtual MOS_STATUS ConfigureVpScalability(VP_MHWINTERFACE &vpMhwInterface);

    virtual bool IsEufusionBypassWaEnabled()
    {
        return true;
    }

    virtual bool IsAdvanceNativeKernelSupported()
    {
        return false;
    }

    virtual bool IsRenderMMCLimitationCheckNeeded()
    {
        return true;
    }

    virtual bool IsDecompForInterlacedSurfWaEnabled()
    {
        return true;
    }

    virtual bool IsLegacyEuCountInUse()
    {
        return true;
    }

    virtual MOS_STATUS InitVpFeatureSupportBits() override;

protected:
    bool m_disableSfcDithering = false;

MEDIA_CLASS_DEFINE_END(vp__VpPlatformInterfacesXe_Lpm_Plus)
};

}
#endif // !__VP_PLATFORM_INTERFACE_XE_LPM_PLUS_H__
