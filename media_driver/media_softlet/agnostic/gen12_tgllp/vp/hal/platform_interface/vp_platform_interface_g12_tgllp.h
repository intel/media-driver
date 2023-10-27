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
//! \file     vp_platform_interface_g12_tgllp.h
//! \brief    platform specific vp interfaces.
//!
#ifndef __VP_PLATFORM_INTERFACE_G12_TGLLP_H__
#define __VP_PLATFORM_INTERFACE_G12_TGLLP_H__

#include "vp_platform_interface.h"

namespace vp
{
#define VEBOX_KERNEL_BASE_MAX_G12 2
//!
//! \brief Vebox Statistics Surface definition for TGL
//!
#define VP_VEBOX_STATISTICS_SIZE_G12                          (32 * 8)
#define VP_VEBOX_STATISTICS_PER_FRAME_SIZE_G12                (32 * sizeof(uint32_t))
#define VP_VEBOX_STATISTICS_SURFACE_FMD_OFFSET_G12            0
#define VP_VEBOX_STATISTICS_SURFACE_GNE_OFFSET_G12            0x2C
#define VP_VEBOX_STATISTICS_SURFACE_STD_OFFSET_G12            0x44

class VpPlatformInterfaceG12Tgllp : public VpPlatformInterface
{
public:

    VpPlatformInterfaceG12Tgllp(PMOS_INTERFACE pOsInterface)
        : VpPlatformInterface(pOsInterface)
    {
    }

    virtual ~VpPlatformInterfaceG12Tgllp()
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

    virtual uint32_t VeboxQueryStaticSurfaceSize()
    {
        return VP_VEBOX_STATISTICS_SIZE_G12;
    }

    virtual MOS_STATUS ConfigVirtualEngine()
    {
        return MOS_STATUS_SUCCESS;    // the config would be set in MhwVeboxInterfaceG12::CreateGpuContext().
    }

    virtual bool IsGpuContextCreatedInPipelineInit()
    {
        return false;
    }

    virtual bool IsAdvanceNativeKernelSupported()
    {
        return false;
    }

    virtual bool IsLegacyEuCountInUse()
    {
        return true;
    }

    virtual MOS_STATUS GetInputFrameWidthHeightAlignUnit(
        PVP_MHWINTERFACE          pvpMhwInterface,
        uint32_t                 &widthAlignUnit,
        uint32_t                 &heightAlignUnit,
        bool                      bVdbox,
        CODECHAL_STANDARD         codecStandard,
        CodecDecodeJpegChromaType jpegChromaType);

    virtual MOS_STATUS GetVeboxHeapInfo(
        PVP_MHWINTERFACE          pvpMhwInterface,
        const MHW_VEBOX_HEAP    **ppVeboxHeap);

    virtual bool IsVeboxScalabilityWith4KNotSupported(
        VP_MHWINTERFACE           vpMhwInterface);

    virtual MOS_STATUS ConfigureVpScalability(VP_MHWINTERFACE &vpMhwInterface);

    MEDIA_CLASS_DEFINE_END(vp__VpPlatformInterfaceG12Tgllp)
};

}
#endif // !__VP_PLATFORM_INTERFACE_G12_TGLLP_H__
