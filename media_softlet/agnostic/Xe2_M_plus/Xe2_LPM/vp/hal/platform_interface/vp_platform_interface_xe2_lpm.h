/*
* Copyright (c) 2024, Intel Corporation
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
//! \file     vp_platform_interface_xe2_lpm.h
//! \brief    platform specific vp interfaces.
//!
#ifndef __VP_PLATFORM_INTERFACE_XE2_LPM_H__
#define __VP_PLATFORM_INTERFACE_XE2_LPM_H__

#include "vp_platform_interface.h"

//!
//! \brief Vebox Statistics Surface definition for Xe2_LPM
//!
#define VP_VEBOX_STATISTICS_SIZE_XE2_LPM                                (32 * 8)
#define VP_VEBOX_STATISTICS_PER_FRAME_SIZE_XE2_LPM                      (32 * sizeof(uint32_t))
#define VP_VEBOX_STATISTICS_SURFACE_FMD_OFFSET_XE2_LPM                   0
#define VP_VEBOX_STATISTICS_SURFACE_GNE_OFFSET_XE2_LPM                   0x2C
#define VP_VEBOX_STATISTICS_SURFACE_STD_OFFSET_XE2_LPM                   0x44
#define VP_VEBOX_STATISTICS_SURFACE_SW_WA_OFFSET_XE2_LPM                 -12

namespace vp
{

class VpPlatformInterfacsXe2_Lpm : public VpPlatformInterface
{
public:

    VpPlatformInterfacsXe2_Lpm(PMOS_INTERFACE pOsInterface, bool clearViewMode = false);

    virtual ~VpPlatformInterfacsXe2_Lpm()
    {}

    virtual MOS_STATUS InitVpVeboxSfcHwCaps(VP_VEBOX_ENTRY_REC *veboxHwEntry, uint32_t veboxEntryCount, VP_SFC_ENTRY_REC *sfcHwEntry, uint32_t sfcEntryCount);
    virtual MOS_STATUS InitVpRenderHwCaps();
    virtual VPFeatureManager *CreateFeatureChecker(_VP_MHWINTERFACE *hwInterface);
    virtual VpCmdPacket *CreateVeboxPacket(MediaTask * task, _VP_MHWINTERFACE *hwInterface, VpAllocator *&allocator, VPMediaMemComp *mmc);
    virtual MOS_STATUS CreateSfcRender(SfcRenderBase *&sfcRender, VP_MHWINTERFACE &vpMhwinterface, PVpAllocator allocator);
    virtual VpCmdPacket *CreateRenderPacket(MediaTask * task, _VP_MHWINTERFACE *hwInterface, VpAllocator *&allocator, VPMediaMemComp *mmc, VpKernelSet* kernel);
    virtual MOS_STATUS VeboxQueryStatLayout(
        VEBOX_STAT_QUERY_TYPE queryType,
        uint32_t* pQuery);
    virtual uint32_t        VeboxQueryStaticSurfaceSize()
    {
        return VP_VEBOX_STATISTICS_SIZE_XE2_LPM;
    }

    //Only for get kernel binary in legacy path not being used in APO path.
    virtual MOS_STATUS GetKernelBinary(const void *&kernelBin, uint32_t &kernelSize, const void *&patchKernelBin, uint32_t &patchKernelSize);

    virtual MOS_STATUS ConfigVirtualEngine();

    virtual MOS_STATUS ConfigureVpScalability(VP_MHWINTERFACE &vpMhwInterface);

    virtual MOS_STATUS InitPolicyRules(VP_POLICY_RULES &rules) override;

    virtual MOS_STATUS InitVpFeatureSupportBits() override;

protected:
    bool m_disableSfcDithering = false;

MEDIA_CLASS_DEFINE_END(vp__VpPlatformInterfacsXe2_Lpm)
};

}
#endif // !__VP_PLATFORM_INTERFACE_XE2_LPM_H__
