/*===================== begin_copyright_notice ==================================
Copyright (c) 2024, Intel Corporation

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
//! \file     media_interfaces_ptl.cpp

//! \brief    Helps with PTL factory creation.
//!

#include "media_interfaces_ptl.h"

#include "codechal_common.h"
#include "codechal_debug.h"

#include "vp_pipeline_adapter_xe3_lpm.h"
#include "vp_platform_interface_xe3_lpm.h"
#include "vp_kernel_config_xe2_hpg.h"
#include "mhw_mi_xe3_lpm_base_impl.h"
#include "mhw_sfc_xe3_lpm_base_impl.h"
#include "mhw_vebox_xe3_lpm_base_impl.h"
#include "mhw_render_xe2_hpg_next_impl.h"
#include "mhw_blt_xe3_lpm_impl.h"

#include "media_interfaces_mcpy_next.h"

#include "mhw_vdbox_vdenc_impl_xe_lpm_plus.h"

#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
#include "igvpkrn_xe2_hpg.h"
#include "igvpkrn_xe2_hpg_cmfcpatch.h"
#include "igvpkrn_l0_xe2_hpg.h"
#include "igvpfc_common_xe2.h"
#include "igvpfc_fp_xe2.h"
#include "igvpfc_444PL3_input_xe2.h"
#include "igvpfc_444PL3_output_xe2.h"
#include "igvpfc_420PL3_input_xe2.h"
#include "igvpfc_420PL3_output_xe2.h"
#include "igvp3dlut_xe2.h"
#include "igvpfc_422HV_input_xe2.h"
#endif

using namespace mhw::vdbox::avp::xe3_lpm_base;
using namespace mhw::vdbox::vdenc::xe3_lpm_base;
using namespace mhw::vdbox::huc::xe3_lpm_base;

extern template class MediaFactory<uint32_t, CodechalDeviceNext>;
extern template class MediaFactory<uint32_t, VphalDevice>;
extern template class MediaFactory<uint32_t, RenderHalDevice>;
extern template class MediaFactory<uint32_t, MediaInterfacesHwInfoDevice>;


extern template class MediaFactory<uint32_t, MhwInterfacesNext>;

static bool ptlRegisteredVphal =
    MediaFactory<uint32_t, VphalDevice>::
        Register<VphalInterfacesXe3_Lpm>((uint32_t)IGFX_PTL);

MOS_STATUS VphalInterfacesXe3_Lpm::Initialize(
    PMOS_INTERFACE osInterface,
    bool           bInitVphalState,
    MOS_STATUS *   eStatus,
    bool           clearViewMode)
{
    vp::VpPlatformInterface* vpPlatformInterface = MOS_New(vp::VpPlatformInterfacsXe3_Lpm, osInterface);
    if (nullptr == vpPlatformInterface)
    {
        *eStatus = MOS_STATUS_NULL_POINTER;
        return *eStatus;
    }

    InitPlatformKernelBinary(vpPlatformInterface);

    if (!bInitVphalState)
    {
        m_vpPipeline = MOS_New(vp::VpPipeline, osInterface);
        if (nullptr == m_vpPipeline)
        {
            MOS_Delete(vpPlatformInterface);
            MOS_OS_CHK_NULL_RETURN(m_vpPipeline);
        }
        m_vpPlatformInterface = vpPlatformInterface;
        *eStatus = MOS_STATUS_SUCCESS;
        return *eStatus;
    }

    m_vpBase = MOS_New(
        VpPipelineAdapterXe3_Lpm,
        osInterface,
        *vpPlatformInterface,
        *eStatus);
    if (nullptr == m_vpBase)
    {
        MOS_Delete(vpPlatformInterface);
        *eStatus = MOS_STATUS_NULL_POINTER;
        return *eStatus;
    }
    m_isNextEnabled = true;

    return *eStatus;
}

MOS_STATUS VphalInterfacesXe3_Lpm::CreateVpPlatformInterface(
    PMOS_INTERFACE osInterface,
    MOS_STATUS *   eStatus)
{
    vp::VpPlatformInterface *vpPlatformInterface = MOS_New(vp::VpPlatformInterfacsXe3_Lpm, osInterface);
    if (nullptr == vpPlatformInterface)
    {
        *eStatus = MOS_STATUS_NULL_POINTER;
    }
    else
    {
        InitPlatformKernelBinary(vpPlatformInterface);

        m_vpPlatformInterface = vpPlatformInterface;
        *eStatus              = MOS_STATUS_SUCCESS;
    }
    return *eStatus;
}

void VphalInterfacesXe3_Lpm::InitPlatformKernelBinary(
    vp::VpPlatformInterface *&vpPlatformInterface)
{
    static vp::VpKernelConfigXe2_Hpg kernelConfig;
    vpPlatformInterface->SetKernelConfig(&kernelConfig);
#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
    vpPlatformInterface->SetVpFCKernelBinary(
                        IGVPKRN_XE2_HPG,
                        IGVPKRN_XE2_HPG_SIZE,
                        IGVPKRN_XE2_HPG_CMFCPATCH,
                        IGVPKRN_XE2_HPG_CMFCPATCH_SIZE);
    vpPlatformInterface->AddVpNativeAdvKernelEntryToList(IGVP3DLUT_GENERATION_XE2_HPG, IGVP3DLUT_GENERATION_XE2_HPG_SIZE, "hdr_3dlut_l0");
    // Need to SetOclKernelEnable in platform interface for ocl kernels
    vpPlatformInterface->SetOclKernelEnable();
    AddVpNativeKernelEntryToListFc_commonXe2(*vpPlatformInterface);
    AddVpNativeKernelEntryToListFc_fpXe2(*vpPlatformInterface);
    AddVpNativeKernelEntryToListFc_444pl3_inputXe2(*vpPlatformInterface);
    AddVpNativeKernelEntryToListFc_444pl3_outputXe2(*vpPlatformInterface);
    AddVpNativeKernelEntryToListFc_420pl3_inputXe2(*vpPlatformInterface);
    AddVpNativeKernelEntryToListFc_420pl3_outputXe2(*vpPlatformInterface);
    AddVpNativeKernelEntryToList3dlutXe2(*vpPlatformInterface);
    AddVpNativeKernelEntryToListFc_422hv_inputXe2(*vpPlatformInterface);
#endif
}

static bool ptlRegisteredMhwNext =
    MediaFactory<uint32_t, MhwInterfacesNext>::
        Register<MhwInterfacesPtl_Next>((uint32_t)IGFX_PTL);

MOS_STATUS MhwInterfacesPtl_Next::Initialize(
    CreateParams   params,
    PMOS_INTERFACE osInterface)
{
    if (osInterface == nullptr)
    {
        MHW_ASSERTMESSAGE("The OS interface is not valid!");
        return MOS_STATUS_INVALID_PARAMETER;
    }
    m_osInterface = osInterface;

    auto gtSystemInfo = osInterface->pfnGetGtSystemInfo(osInterface);
    if (gtSystemInfo == nullptr)
    {
        MHW_ASSERTMESSAGE("The OS interface is not valid!");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if ((params.m_isCp == false) && (params.Flags.m_value == 0)  && (params.m_isMos) == 0)
    {
        MHW_ASSERTMESSAGE("No MHW interfaces were requested for creation.");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    //MHW_MI must always be created
    auto ptr      = std::make_shared<mhw::mi::xe3_lpm_base::Impl>(osInterface);
    m_miItf       = std::static_pointer_cast<mhw::mi::Itf>(ptr);
    //For mos, need miInterface only
    if(params.m_isMos)
    {
        return MOS_STATUS_SUCCESS;
    }

    // MHW_CP must always be created
    MOS_STATUS status;
    m_cpInterface = osInterface->pfnCreateMhwCpInterface(osInterface);
    MHW_MI_CHK_NULL(m_cpInterface);

    ptr->SetCpInterface(m_cpInterface, m_miItf);

    if (params.Flags.m_render)
    {
        auto renderPtr = std::make_shared<mhw::render::xe2_hpg_next::Impl>(osInterface);
        m_renderItf    = std::static_pointer_cast<mhw::render::Itf>(renderPtr);
    }
    if (params.Flags.m_stateHeap)
    {
        m_stateHeapInterface =
            MOS_New(StateHeap, osInterface, params.m_heapMode);
    }
    if (params.Flags.m_sfc)
    {
        auto sfcPtr    = std::make_shared<mhw::sfc::xe3_lpm_base::Impl>(osInterface);
        m_sfcItf       = std::static_pointer_cast<mhw::sfc::Itf>(sfcPtr);
    }
    if (params.Flags.m_vebox)
    {
        auto veboxPtr    = std::make_shared<mhw::vebox::xe3_lpm_base::Impl>(osInterface);
        m_veboxItf       = std::static_pointer_cast<mhw::vebox::Itf>(veboxPtr);
    }

    if (params.Flags.m_vdboxAll || params.Flags.m_mfx)
    {
        auto ptr = std::make_shared<mhw::vdbox::mfx::xe3_lpm_base::xe3_lpm::Impl>(osInterface, m_cpInterface);
        m_mfxItf = std::static_pointer_cast<mhw::vdbox::mfx::Itf>(ptr);
    }
    if (params.Flags.m_vdboxAll || params.Flags.m_hcp)
    {
        m_hcpItf = std::make_shared<mhw::vdbox::hcp::xe3_lpm_base::xe3_lpm::Impl>(osInterface);
    }
    if (params.Flags.m_vdboxAll)  
    {
        auto ptr = std::make_shared<mhw::vdbox::avp::xe3_lpm_base::xe3_lpm::Impl>(osInterface);
        m_avpItf = std::static_pointer_cast<mhw::vdbox::avp::Itf>(ptr);
    }
    if (params.Flags.m_vdboxAll)
    {
        auto ptr  = std::make_shared<mhw::vdbox::vvcp::xe3_lpm_base::xe3_lpm::Impl>(osInterface, m_cpInterface);
        m_vvcpItf = std::static_pointer_cast<mhw::vdbox::vvcp::Itf>(ptr);
    }
    if (params.Flags.m_vdboxAll || params.Flags.m_huc)
    {
        auto ptr = std::make_shared<mhw::vdbox::huc::xe3_lpm_base::xe3_lpm::Impl>(osInterface, m_cpInterface);
        m_hucItf = std::static_pointer_cast<mhw::vdbox::huc::Itf>(ptr);
    }
    if (params.Flags.m_vdboxAll || params.Flags.m_vdenc)
    {
        auto ptr = std::make_shared<mhw::vdbox::vdenc::xe3_lpm_base::xe3_lpm::Impl>(osInterface);
        m_vdencItf = std::static_pointer_cast<mhw::vdbox::vdenc::Itf>(ptr);
    }
    if (params.Flags.m_blt)
    {
        auto bltptr = std::make_shared<mhw::blt::xe3_lpm::Impl>(osInterface);
        m_bltItf    = std::static_pointer_cast<mhw::blt::Itf>(bltptr);
    }

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Destroys all created MHW interfaces
//! \details  If the HAL creation fails, this is used for cleanup
//!
void MhwInterfacesPtl_Next::Destroy()
{
    MhwInterfacesNext::Destroy();
}

static bool ptlRegisteredCodecHal =
    MediaFactory<uint32_t, CodechalDeviceNext>::
        Register<CodechalInterfacesXe3_Lpm>((uint32_t)IGFX_PTL);

MOS_STATUS CodechalInterfacesXe3_Lpm::Initialize(
    void *         standardInfo,
    void *         settings,
    MhwInterfacesNext *mhwInterfaces,
    PMOS_INTERFACE osInterface)
{
    if (standardInfo == nullptr ||
        mhwInterfaces == nullptr ||
        osInterface == nullptr)
    {
        CODECHAL_PUBLIC_ASSERTMESSAGE("CodecHal device is not valid!");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    // This part should be moved back to media_intefaces.cpp for softlet build
    PCODECHAL_STANDARD_INFO info          = ((PCODECHAL_STANDARD_INFO)standardInfo);
    CODECHAL_FUNCTION       CodecFunction = info->CodecFunction;

#ifdef _VP9_ENCODE_VDENC_SUPPORTED  
    if (mhwInterfaces != nullptr && info->Mode == CODECHAL_ENCODE_MODE_VP9)
    {
        auto ptr                      = std::make_shared<mhw::vdbox::vdenc::xe_lpm_plus_base::v0::Impl>(osInterface);
        mhwInterfaces->m_vdencItf = std::dynamic_pointer_cast<mhw::vdbox::vdenc::Itf>(ptr);
        CODECHAL_PUBLIC_CHK_NULL_RETURN(mhwInterfaces->m_vdencItf);
    }
#endif

    bool disableScalability = true;

    CodechalHwInterfaceNext *hwInterface = MOS_New(Hw, osInterface, CodecFunction, mhwInterfaces, disableScalability);

    if (hwInterface == nullptr)
    {
        CODECHAL_PUBLIC_ASSERTMESSAGE("hwInterface is not valid!");
        return MOS_STATUS_NO_SPACE;
    }
    hwInterface->pfnCreateDecodeSinglePipe = decode::DecodeScalabilitySinglePipeNext::CreateDecodeSinglePipe;
    hwInterface->pfnCreateDecodeMultiPipe  = decode::DecodeScalabilityMultiPipeNext::CreateDecodeMultiPipe;

#if USE_CODECHAL_DEBUG_TOOL
    CodechalDebugInterface *debugInterface = MOS_New(CodechalDebugInterface);
    if (debugInterface == nullptr)
    {
        MOS_Delete(hwInterface);
        CODECHAL_PUBLIC_ASSERTMESSAGE("debugInterface is not valid!");
        return MOS_STATUS_NO_SPACE;
    }
    if (debugInterface->Initialize(hwInterface, CodecFunction) != MOS_STATUS_SUCCESS)
    {
        MOS_Delete(hwInterface);
        MOS_Delete(debugInterface);
        CODECHAL_PUBLIC_ASSERTMESSAGE("Debug interface creation failed!");
        return MOS_STATUS_INVALID_PARAMETER;
    }
#else
    CodechalDebugInterface *debugInterface = nullptr;
#endif  // USE_CODECHAL_DEBUG_TOOL

    auto release_func = [&]() 
    {
        MOS_Delete(hwInterface);
#if USE_CODECHAL_DEBUG_TOOL
        MOS_Delete(debugInterface);
#endif  // USE_CODECHAL_DEBUG_TOOL
    };

    if (CodecHalIsDecode(CodecFunction))
    {
#ifdef _MPEG2_DECODE_SUPPORTED
        if (info->Mode == CODECHAL_DECODE_MODE_MPEG2IDCT ||
            info->Mode == CODECHAL_DECODE_MODE_MPEG2VLD)
        {
            m_codechalDevice = MOS_New(Decode::Mpeg2, hwInterface, debugInterface);
        }
        else
#endif
#ifdef _VC1_DECODE_SUPPORTED
        if (info->Mode == CODECHAL_DECODE_MODE_VC1IT ||
            info->Mode == CODECHAL_DECODE_MODE_VC1VLD)
        {
            CODECHAL_PUBLIC_ASSERTMESSAGE("PTL doesn't support VC1!");
        }
        else
#endif
#ifdef _AVC_DECODE_SUPPORTED
        if (info->Mode == CODECHAL_DECODE_MODE_AVCVLD)
        {
           m_codechalDevice = MOS_New(Decode::Avc, hwInterface, debugInterface);
        }
        else
#endif
#ifdef _JPEG_DECODE_SUPPORTED
        if (info->Mode == CODECHAL_DECODE_MODE_JPEG)
        {
            m_codechalDevice = MOS_New(Decode::Jpeg, hwInterface, debugInterface);
        }
        else
#endif
#ifdef _VP8_DECODE_SUPPORTED
        if (info->Mode == CODECHAL_DECODE_MODE_VP8VLD)
        {
            m_codechalDevice = MOS_New(Decode::Vp8, hwInterface, debugInterface);
        }
        else
#endif
#ifdef _HEVC_DECODE_SUPPORTED
        if (info->Mode == CODECHAL_DECODE_MODE_HEVCVLD)
        {
            m_codechalDevice = MOS_New(Decode::Hevc, hwInterface, debugInterface);
        }
        else
#endif
#ifdef _VP9_DECODE_SUPPORTED
        if (info->Mode == CODECHAL_DECODE_MODE_VP9VLD)
        {
            m_codechalDevice = MOS_New(Decode::Vp9, hwInterface, debugInterface);
        }
        else
#endif
#ifdef _AV1_DECODE_SUPPORTED
        if (info->Mode == CODECHAL_DECODE_MODE_AV1VLD)
        {
            m_codechalDevice = MOS_New(Decode::Av1, hwInterface, debugInterface);
        }
        else
#endif
#ifdef _VVC_DECODE_SUPPORTED
        if (info->Mode == CODECHAL_DECODE_MODE_VVCVLD) //VVC
        {
            m_codechalDevice = MOS_New(Decode::Vvc, hwInterface, debugInterface);
        }
        else
#endif
        {
            CODECHAL_PUBLIC_ASSERTMESSAGE("Decode mode requested invalid!");
            CODECHAL_PUBLIC_CHK_STATUS_WITH_DESTROY_RETURN(MOS_STATUS_INVALID_PARAMETER, release_func);
        }

        if (m_codechalDevice == nullptr)
        {
            CODECHAL_PUBLIC_ASSERTMESSAGE("Decoder device creation failed!");
            CODECHAL_PUBLIC_CHK_STATUS_WITH_DESTROY_RETURN(MOS_STATUS_NO_SPACE, release_func);
        }
    }
    else if (CodecHalIsEncode(CodecFunction))
    {
#if defined(_AVC_ENCODE_VDENC_SUPPORTED) && defined(_MEDIA_RESERVED)
        if (info->Mode == CODECHAL_ENCODE_MODE_AVC)
        {
            if (CodecHalUsesVdencEngine(info->CodecFunction))
            {
                m_codechalDevice = MOS_New(EncodeAvcVdencPipelineAdapterXe3_Lpm, hwInterface, debugInterface);
                if (m_codechalDevice == nullptr)
                {
                    CODECHAL_PUBLIC_ASSERTMESSAGE("Encode state creation failed!");
                    CODECHAL_PUBLIC_CHK_STATUS_WITH_DESTROY_RETURN(MOS_STATUS_INVALID_PARAMETER, release_func);
                }
                return MOS_STATUS_SUCCESS;
            }
        }
        else
#endif
#if defined(_VP9_ENCODE_VDENC_SUPPORTED) && defined(_MEDIA_RESERVED)
        if (info->Mode == CODECHAL_ENCODE_MODE_VP9)
        {
            m_codechalDevice = MOS_New(EncodeVp9VdencPipelineAdapterXe2_Lpm, hwInterface, debugInterface);
            if (m_codechalDevice == nullptr)
            {
                CODECHAL_PUBLIC_ASSERTMESSAGE("Encode state creation failed!");
                CODECHAL_PUBLIC_CHK_STATUS_WITH_DESTROY_RETURN(MOS_STATUS_INVALID_PARAMETER, release_func);
            }
            return MOS_STATUS_SUCCESS;
        }
        else
#endif
#if defined(_JPEG_ENCODE_SUPPORTED) && defined(_MEDIA_RESERVED)
        if (info->Mode == CODECHAL_ENCODE_MODE_JPEG)
        {
            m_codechalDevice = MOS_New(EncodeJpegPipelineAdapterXe3_Lpm_Base, hwInterface, debugInterface);
            if (m_codechalDevice == nullptr)
            {
                CODECHAL_PUBLIC_ASSERTMESSAGE("Encode state creation failed!");
                CODECHAL_PUBLIC_CHK_STATUS_WITH_DESTROY_RETURN(MOS_STATUS_INVALID_PARAMETER, release_func);
            }
            return MOS_STATUS_SUCCESS;
        }
        else
#endif
#if defined(_AV1_ENCODE_VDENC_SUPPORTED) && defined(_MEDIA_RESERVED)
        if (info->Mode == codechalEncodeModeAv1)
        {
            if (CodecHalUsesVdencEngine(info->CodecFunction))
            {
                m_codechalDevice = MOS_New(EncodeAv1VdencPipelineAdapterXe3_Lpm, hwInterface, debugInterface);
                if (m_codechalDevice == nullptr)
                {
                    CODECHAL_PUBLIC_ASSERTMESSAGE("Encode state creation failed!");
                    CODECHAL_PUBLIC_CHK_STATUS_WITH_DESTROY_RETURN(MOS_STATUS_INVALID_PARAMETER, release_func);
                }
                return MOS_STATUS_SUCCESS;
            }
            else
            {
                CODECHAL_PUBLIC_CHK_STATUS_WITH_DESTROY_RETURN(MOS_STATUS_INVALID_PARAMETER, release_func);
            }
        }
        else
#endif
#if defined(_HEVC_ENCODE_VDENC_SUPPORTED) && defined(_MEDIA_RESERVED)
        if (info->Mode == CODECHAL_ENCODE_MODE_HEVC)
        {
            if (CodecHalUsesVdencEngine(info->CodecFunction))
            {
                m_codechalDevice = MOS_New(EncodeHevcVdencPipelineAdapterXe3_Lpm_Base, hwInterface, debugInterface);
                if (m_codechalDevice == nullptr)
                {
                    CODECHAL_PUBLIC_ASSERTMESSAGE("Encode state creation failed!");
                    CODECHAL_PUBLIC_CHK_STATUS_WITH_DESTROY_RETURN(MOS_STATUS_INVALID_PARAMETER, release_func);
                }
                return MOS_STATUS_SUCCESS;
            }
        }
        else
#endif
        {
            CODECHAL_PUBLIC_ASSERTMESSAGE("Unsupported encode function requested.");
            CODECHAL_PUBLIC_CHK_STATUS_WITH_DESTROY_RETURN(MOS_STATUS_INVALID_PARAMETER, release_func);
        }
    }
    else
    {
        CODECHAL_PUBLIC_ASSERTMESSAGE("Unsupported codec function requested.");
        CODECHAL_PUBLIC_CHK_STATUS_WITH_DESTROY_RETURN(MOS_STATUS_INVALID_PARAMETER, release_func);
    }

    return MOS_STATUS_SUCCESS;
}

static bool ptlRegisteredRenderHal =
    MediaFactory<uint32_t, RenderHalDevice>::
        Register<RenderHalInterfacesXe3_Lpg>((uint32_t)IGFX_PTL);

MOS_STATUS RenderHalInterfacesXe3_Lpg::Initialize()
{
    m_renderhalDevice = MOS_New(XRenderHal);
    if (m_renderhalDevice == nullptr)
    {
        MHW_ASSERTMESSAGE("Create Render Hal interfaces failed.")
        return MOS_STATUS_NO_SPACE;
    }
    return MOS_STATUS_SUCCESS;
}

#define IP_VERSION_XE3_LPM 0x1600

static bool ptlRegisteredHwInfo =
    MediaFactory<uint32_t, MediaInterfacesHwInfoDevice>::Register<MediaInterfacesHwInfoDeviceXe3_Lpm>((uint32_t)IGFX_PTL);

MOS_STATUS MediaInterfacesHwInfoDeviceXe3_Lpm::Initialize(PLATFORM platform)
{
    m_hwInfo.SetDeviceInfo(IP_VERSION_XE3_LPM, platform.usRevId);
    return MOS_STATUS_SUCCESS;
}

static bool ptlRegisteredMcpy =
    MediaFactory<uint32_t, McpyDeviceNext>::
        Register<McpyDeviceXe3_Lpm_Base>((uint32_t)IGFX_PTL);