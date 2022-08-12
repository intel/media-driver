/*===================== begin_copyright_notice ==================================

# Copyright (c) 2021, Intel Corporation

# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.

======================= end_copyright_notice ==================================*/

//!
//! \file     mhw_render_xe_hpg.h
//! \brief    Defines functions for constructing  render engine commands on Xe_HPM platforms
//!

#ifndef __MHW_RENDER_XE_HPM_H__
#define __MHW_RENDER_XE_HPM_H__

#include "mhw_render_generic.h"
#include "mhw_render_hwcmd_xe_hp_base.h"
#include "mhw_render_xe_hp_base.h"
#include "mhw_mmio_g12.h"
#include "mhw_render_itf.h"
#include "mhw_render_xe_hpg_impl.h"
#include <memory>

class MhwRenderInterfaceXe_Hpg : public MhwRenderInterfaceXe_Xpm_Base
{
public:
    MhwRenderInterfaceXe_Hpg(
        MhwMiInterface          *miInterface,
        PMOS_INTERFACE          osInterface,
        MEDIA_SYSTEM_INFO       *gtSystemInfo,
        uint8_t                 newStateHeapManagerRequested) :
        MhwRenderInterfaceXe_Xpm_Base(miInterface, osInterface, gtSystemInfo, newStateHeapManagerRequested)
    {
        MHW_FUNCTION_ENTER;
    }

    virtual ~MhwRenderInterfaceXe_Hpg() { MHW_FUNCTION_ENTER; }

    virtual MOS_STATUS AddCfeStateCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VFE_PARAMS params) override;

    virtual MOS_STATUS AddComputeWalkerCmd(MOS_COMMAND_BUFFER *cmdBuffer,
                                           MHW_GPGPU_WALKER_PARAMS *gpgpuWalkerParams,
                                           MHW_ID_ENTRY_PARAMS *interfaceDescriptorParams,
                                           MOS_RESOURCE *postsyncResource,
                                           uint32_t resourceOffset) override;

    virtual MOS_STATUS AddChromaKeyCmd(
            PMOS_COMMAND_BUFFER     cmdBuffer,
            PMHW_CHROMAKEY_PARAMS   params) override;

    std::shared_ptr<mhw::render::Itf> GetNewRenderInterface() override
    {
        if (!m_renderItfNew)
        {
            auto ptr = std::make_shared<mhw::render::xe_hpg::Impl>(m_osInterface);
            m_renderItfNew = ptr;
        }

        return m_renderItfNew;
    }
};
#endif
