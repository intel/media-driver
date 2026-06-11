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
//! \file     media_interfaces_nvl.cpp

//! \brief    Helps with NVL factory creation.
//!

#include "media_interfaces_nvl.h"
#include "vp_platform_interface_xe3p_lpm.h"
#include "vp_kernel_config_xe3p_lpg_base.h"
#include "mhw_mi_xe3p_lpm_impl.h"
#include "mhw_render_xe3p_lpg_impl.h"

#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
#include "igvpfc_common_xe3p.h"
#include "igvpfc_fp_xe3p.h"
#include "igvpfc_420PL3_input_xe3p.h"
#include "igvpfc_420PL3_output_xe3p.h"
#include "igvpfc_444PL3_input_xe3p.h"
#include "igvpfc_444PL3_output_xe3p.h"
#include "igvpfc_422HV_input_xe3p.h"
#include "igvp3dlut_xe3p.h"
#include "igvpHdrRender_xe3p.h"
#endif

extern template class MediaFactory<uint32_t, CodechalDeviceNext>;
extern template class MediaFactory<uint32_t, VphalDevice>;
extern template class MediaFactory<uint32_t, RenderHalDevice>;
extern template class MediaFactory<uint32_t, MediaInterfacesHwInfoDevice>;
extern template class MediaFactory<uint32_t, HucKernelSourceDevice>;

// Swith to use new media factory template
extern template class MediaFactory<uint32_t, MhwInterfacesNext>;

static bool nvlRegisteredVphal =
    MediaFactory<uint32_t, VphalDevice>::
        Register<VphalInterfacesNvl>((uint32_t)IGFX_NVL);

void VphalInterfacesNvl::InitPlatformKernelBinary(
    vp::VpPlatformInterface *&vpPlatformInterface)
{
    static vp::VpKernelConfigXe3P_Lpg_Base kernelConfig;
    vpPlatformInterface->SetKernelConfig(&kernelConfig);
#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
    vpPlatformInterface->SetOclKernelEnable();
    AddVpNativeKernelEntryToListFc_commonXe3p(*vpPlatformInterface);
    AddVpNativeKernelEntryToListFc_fpXe3p(*vpPlatformInterface);
    AddVpNativeKernelEntryToListFc_444pl3_inputXe3p(*vpPlatformInterface);
    AddVpNativeKernelEntryToListFc_444pl3_outputXe3p(*vpPlatformInterface);
    AddVpNativeKernelEntryToListFc_420pl3_inputXe3p(*vpPlatformInterface);
    AddVpNativeKernelEntryToListFc_420pl3_outputXe3p(*vpPlatformInterface);
    AddVpNativeKernelEntryToList3dlutXe3p(*vpPlatformInterface);
    AddVpNativeKernelEntryToListFc_422hv_inputXe3p(*vpPlatformInterface);
    AddVpNativeKernelEntryToListHdrrenderXe3p(*vpPlatformInterface);
#endif
}

static bool nvlRegisteredMhwNext =
    MediaFactory<uint32_t, MhwInterfacesNext>::
        Register<MhwInterfacesNvl>((uint32_t)IGFX_NVL);

void MhwInterfacesNvl::InitializeRenderComponent(
    CreateParams   params,
    PMOS_INTERFACE osInterface)
{
    if (params.Flags.m_render)
    {
        auto renderPtr = std::make_shared<mhw::render::xe3p_lpg::Impl>(osInterface);
        m_renderItf    = std::static_pointer_cast<mhw::render::Itf>(renderPtr);
    }
    if (params.Flags.m_stateHeap)
    {
        m_stateHeapInterface =
            MOS_New(StateHeap, osInterface, params.m_heapMode);
    }
}

static bool nvlRegisteredCodecHal =
    MediaFactory<uint32_t, CodechalDeviceNext>::
        Register<CodechalInterfacesXe3P_Lpm>((uint32_t)IGFX_NVL);

static bool nvlRegisteredRenderHal =
    MediaFactory<uint32_t, RenderHalDevice>::
        Register<RenderHalInterfacesNvl>((uint32_t)IGFX_NVL);

MOS_STATUS RenderHalInterfacesNvl::Initialize()
{
    m_renderhalDevice = MOS_New(XRenderHal);
    if (m_renderhalDevice == nullptr)
    {
        MHW_ASSERTMESSAGE("Create Render Hal interfaces failed.")
        return MOS_STATUS_NO_SPACE;
    }
    return MOS_STATUS_SUCCESS;
}

static bool nvlRegisteredHwInfo =
    MediaFactory<uint32_t, MediaInterfacesHwInfoDevice>::Register<MediaInterfacesHwInfoDeviceXe3P_Lpm>((uint32_t)IGFX_NVL);

static bool nvlRegisteredMcpy =
    MediaFactory<uint32_t, McpyDeviceNext>::
        Register<McpyDeviceXe3P_Lpm_Base>((uint32_t)IGFX_NVL);

static bool nvlRegisteredHucKernelSource =
    MediaFactory<uint32_t, HucKernelSourceDevice>::
        Register<HucKernelSourceDeviceXe3P_Lpm>((uint32_t)IGFX_NVL);