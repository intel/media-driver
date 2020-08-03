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
//! \file     vp_platform_interface.h
//! \brief    platform specific vp interfaces.
//!
#ifndef __VP_PLATFORM_INTERFACE_H__
#define __VP_PLATFORM_INTERFACE_H__

namespace vp
{
class VPFeatureManager;
class SfcRenderBase;

class VpPlatformInterface
{
public:

    VpPlatformInterface(PMOS_INTERFACE pOsInterface)
    {
        m_pOsInterface = pOsInterface;
    }

    virtual ~VpPlatformInterface()
    {
    }

    virtual MOS_STATUS InitVpVeboxSfcHwCaps(VP_VEBOX_ENTRY_REC *veboxHwEntry, uint32_t veboxEntryCount, VP_SFC_ENTRY_REC *sfcHwEntry, uint32_t sfcEntryCount)
    {
        return MOS_STATUS_UNIMPLEMENTED;
    }
    virtual MOS_STATUS InitVpRenderHwCaps()
    {
        return MOS_STATUS_UNIMPLEMENTED;
    }
    virtual VPFeatureManager *CreateFeatureChecker(_VP_MHWINTERFACE *hwInterface)
    {
        return nullptr;
    }
    virtual VpCmdPacket *CreateVeboxPacket(MediaTask * task, _VP_MHWINTERFACE *hwInterface, VpAllocator *&allocator, VPMediaMemComp *mmc)
    {
        return nullptr;
    }
    virtual VpCmdPacket *CreateRenderPacket(MediaTask * task, _VP_MHWINTERFACE *hwInterface, VpAllocator *&allocator, VPMediaMemComp *mmc)
    {
        return nullptr;
    }
    virtual MOS_STATUS CreateSfcRender(SfcRenderBase *&sfcRender, VP_MHWINTERFACE &vpMhwinterface, PVpAllocator allocator)
    {
        return MOS_STATUS_UNIMPLEMENTED;
    }
protected:
    PMOS_INTERFACE m_pOsInterface = nullptr;
};

}
#endif // !__VP_PLATFORM_INTERFACE_H__
