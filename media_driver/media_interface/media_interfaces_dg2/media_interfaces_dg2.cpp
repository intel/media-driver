/*===================== begin_copyright_notice ==================================

# Copyright (c) 2021-2024, Intel Corporation

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
//! \file     media_interfaces_dg2.cpp

//! \brief    Helps with DG2 factory creation.
//!

#include "media_interfaces_dg2.h"
#include "codechal.h"
#include "codechal_debug.h"
#if defined(ENABLE_KERNELS) && defined(_MEDIA_RESERVED)
#include "cm_gpucopy_kernel_xe_hpm.h"
#include "cm_gpuinit_kernel_xe_hpm.h"
#else
unsigned int iGPUCopy_kernel_isa_size_dg2 = 0;
unsigned int iGPUInit_kernel_isa_size_dg2 = 0;
unsigned char *pGPUCopy_kernel_isa_dg2 = nullptr;
unsigned char *pGPUInit_kernel_isa_dg2 = nullptr;
#endif
#include "vp_pipeline_adapter_xe_hpm.h"
#include "vp_platform_interface_xe_hpm.h"
#include "vp_kernel_config_g12_base.h"
#include "encode_av1_vdenc_pipeline_adapter_xe_hpm.h"

#if defined(ENABLE_KERNELS)
#include "igvpkrn_xe_hpg.h"
#include "igvpkrn_xe_hpg_cmfcpatch.h"
#if !defined(_FULL_OPEN_SOURCE)
#include "igvpkrn_isa_xe_hpg.h"
#endif
#endif
#include "codechal_hw_next_xe_hpm.h"

using namespace mhw::vdbox::avp::xe_hpm;
using namespace mhw::vdbox::vdenc::xe_hpm;
using namespace mhw::vdbox::huc::xe_hpm;

extern template class MediaFactory<uint32_t, MhwInterfaces>;
extern template class MediaFactory<uint32_t, MmdDevice>;
extern template class MediaFactory<uint32_t, McpyDevice>;
extern template class MediaFactory<uint32_t, CodechalDevice>;
extern template class MediaFactory<uint32_t, CodechalDeviceNext>;
extern template class MediaFactory<uint32_t, CMHalDevice>;
extern template class MediaFactory<uint32_t, VphalDevice>;
extern template class MediaFactory<uint32_t, RenderHalDevice>;
extern template class MediaFactory<uint32_t, Nv12ToP010Device>;
extern template class MediaFactory<uint32_t, DecodeHistogramDevice>;
extern template class MediaFactory<uint32_t, MediaInterfacesHwInfoDevice>;

// Swith to use new media factory template
extern template class MediaFactory<uint32_t, MhwInterfacesNext>;

static bool dg2RegisteredVphal =
MediaFactory<uint32_t, VphalDevice>::
Register<VphalInterfacesXe_Hpm>((uint32_t)IGFX_DG2);

MOS_STATUS VphalInterfacesXe_Hpm::Initialize(
    PMOS_INTERFACE  osInterface,
    bool            bInitVphalState,
    MOS_STATUS      *eStatus,
    bool            clearViewMode)
{
    bool bApogeiosEnable = true;
    MOS_USER_FEATURE_VALUE_DATA         UserFeatureData;
    MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));

    UserFeatureData.i32Data = bApogeiosEnable;
    UserFeatureData.i32DataFlag = MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE;

    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_APOGEIOS_ENABLE_ID,
        &UserFeatureData,
        osInterface->pOsContext);
    bApogeiosEnable = UserFeatureData.bData ? true : false;
    if (bApogeiosEnable)
    {
        vp::VpPlatformInterface *vpPlatformInterface = MOS_New(vp::VpPlatformInterfaceXe_Hpm, osInterface);
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
            VpPipelineAdapterXe_Hpm,
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
    }
    else
    {
        m_vpBase = MOS_New(
            VphalState,
            osInterface,
            eStatus);
    }

    return *eStatus;
}

MOS_STATUS VphalInterfacesXe_Hpm::CreateVpPlatformInterface(
    PMOS_INTERFACE osInterface,
    MOS_STATUS *   eStatus)
{
    vp::VpPlatformInterface *vpPlatformInterface = MOS_New(vp::VpPlatformInterfaceXe_Hpm, osInterface);
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

void VphalInterfacesXe_Hpm::InitPlatformKernelBinary(
    vp::VpPlatformInterface  *&vpPlatformInterface)
{
    static vp::VpKernelConfigG12_Base kernelConfig;
    vpPlatformInterface->SetKernelConfig(&kernelConfig);
#if defined(ENABLE_KERNELS)
    vpPlatformInterface->SetVpFCKernelBinary(
                        IGVPKRN_XE_HPG,
                        IGVPKRN_XE_HPG_SIZE,
                        IGVPKRN_XE_HPG_CMFCPATCH,
                        IGVPKRN_XE_HPG_CMFCPATCH_SIZE);
#if !defined(_FULL_OPEN_SOURCE)
    vpPlatformInterface->AddVpIsaKernelEntryToList(IGVP3DLUT_GENERATION_XE_HPG, IGVP3DLUT_GENERATION_XE_HPG_SIZE);
    vpPlatformInterface->AddVpIsaKernelEntryToList(IGVPHVS_DENOISE_XE_HPG, IGVPHVS_DENOISE_XE_HPG_SIZE);
#endif
#endif
}

static bool dg2RegisteredMhw =
    MediaFactory<uint32_t, MhwInterfaces>::
    Register<MhwInterfacesDg2>((uint32_t)IGFX_DG2);

#define PLATFORM_INTEL_DG2    22
#define GENX_XEHP              11
#define GENX_DG2              13

MOS_STATUS MhwInterfacesDg2::Initialize(
    CreateParams params,
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

    if ((params.m_isCp == false) && (params.Flags.m_value == 0))
    {
        MHW_ASSERTMESSAGE("No MHW interfaces were requested for creation.");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    // MHW_CP and MHW_MI must always be created
    MOS_STATUS status;
    m_cpInterface = osInterface->pfnCreateMhwCpInterface(osInterface);
    MHW_MI_CHK_NULL(m_cpInterface);
    m_miInterface = MOS_New(Mi, m_cpInterface, osInterface);

    if (params.Flags.m_render)
    {
        m_renderInterface =
            MOS_New(Render, m_miInterface, osInterface, gtSystemInfo, params.m_heapMode);
    }
    if (params.Flags.m_stateHeap)
    {
        m_stateHeapInterface =
            MOS_New(StateHeap, osInterface, params.m_heapMode);
    }
    if (params.Flags.m_sfc)
    {
        m_sfcInterface = MOS_New(Sfc, osInterface);
    }
    if (params.Flags.m_vebox)
    {
        m_veboxInterface = MOS_New(Vebox, osInterface);
    }

    if (params.Flags.m_vdboxAll || params.Flags.m_mfx)
    {
        m_mfxInterface =
            MOS_New(Mfx, osInterface, m_miInterface, m_cpInterface, params.m_isDecode);
    }
    if (params.Flags.m_vdboxAll || params.Flags.m_hcp)
    {
        m_hcpInterface =
            MOS_New(Hcp, osInterface, m_miInterface, m_cpInterface, params.m_isDecode);
    }
    if (params.Flags.m_vdboxAll || params.Flags.m_avp)
    {
        m_avpInterface =
            MOS_New(Avp, osInterface, m_miInterface, m_cpInterface, params.m_isDecode);
    }
    if (params.Flags.m_vdboxAll || params.Flags.m_huc)
    {
        m_hucInterface = MOS_New(Huc, osInterface, m_miInterface, m_cpInterface);
    }
    if (params.Flags.m_vdboxAll || params.Flags.m_vdenc)
    {
        m_vdencInterface = MOS_New(Vdenc, osInterface);
    }
    if (params.Flags.m_blt)
    {
        m_bltInterface = MOS_New(Blt, osInterface);
    }

    return MOS_STATUS_SUCCESS;
}

#ifdef _MMC_SUPPORTED
static bool dg2RegisteredMmd =
    MediaFactory<uint32_t, MmdDevice>::
    Register<MmdDeviceXe_Hpm>((uint32_t)IGFX_DG2);

MOS_STATUS MmdDeviceXe_Hpm::Initialize(
    PMOS_INTERFACE osInterface,
    MhwInterfaces *mhwInterfaces)
{
    MHW_FUNCTION_ENTER;

    if (mhwInterfaces->m_miInterface == nullptr || mhwInterfaces->m_veboxInterface == nullptr)
    {
        if (osInterface != nullptr)
        {
            if (osInterface->pfnDestroy)
            {
                osInterface->pfnDestroy(osInterface, false);
            }
            MOS_FreeMemory(osInterface);
        }
        return MOS_STATUS_NULL_POINTER;
    }

    Mmd *device = nullptr;
    device = MOS_New(Mmd);

    if (device == nullptr)
    {
        if (osInterface != nullptr)
        {
            if (osInterface->pfnDestroy)
            {
                osInterface->pfnDestroy(osInterface, false);
            }
            MOS_FreeMemory(osInterface);
        }
        return MOS_STATUS_NO_SPACE;
    }

    // transfer ownership of osinterface to device. device will have exclusive ownership of osinterface and free it.
    if (device->Initialize(
        osInterface,
        mhwInterfaces->m_cpInterface,
        mhwInterfaces->m_miInterface,
        mhwInterfaces->m_veboxInterface) != MOS_STATUS_SUCCESS)
    {
        // Vebox/mi/cp interface will gove control to mmd device, will release will be in destructure func
        // set as null to avoid double free in driver
        mhwInterfaces->m_cpInterface = nullptr;
        mhwInterfaces->m_miInterface = nullptr;
        mhwInterfaces->m_veboxInterface = nullptr;
        MOS_Delete(device);
        return MOS_STATUS_UNKNOWN;
    }

    m_mmdDevice = device;

    return MOS_STATUS_SUCCESS;
}

MhwInterfaces* MmdDeviceXe_Hpm::CreateMhwInterface(
    PMOS_INTERFACE osInterface)
{
    MhwInterfaces::CreateParams params;
    params.Flags.m_vebox = true;

    // the destroy of interfaces happens when the mmd deviced deconstructor funcs
    MhwInterfaces *mhw = MhwInterfaces::CreateFactory(params, osInterface);

    return mhw;
}
#endif

static bool dg2RegisteredMcpy =
    MediaFactory<uint32_t, McpyDevice>::
    Register<McpyDeviceXe_Hpm>((uint32_t)IGFX_DG2);

MOS_STATUS McpyDeviceXe_Hpm::Initialize(
    PMOS_INTERFACE osInterface)
{
    MHW_FUNCTION_ENTER;

    Mcpy *device = nullptr;
    MhwInterfaces* mhwInterfaces = nullptr;

    auto deleterOnFailure = [&](bool deleteOsInterface, bool deleteMhwInterface){
        if (deleteOsInterface && osInterface != nullptr)
        {
            if (osInterface->pfnDestroy)
            {
                osInterface->pfnDestroy(osInterface, false);
            }
            MOS_FreeMemory(osInterface);
        }

        if (deleteMhwInterface && mhwInterfaces != nullptr)
        {
            mhwInterfaces->Destroy();
            MOS_Delete(mhwInterfaces);
        }

        MOS_Delete(device);
    };

    device = MOS_New(Mcpy);
    if (device == nullptr)
    {
        deleterOnFailure(true, false);
        return MOS_STATUS_NO_SPACE;
    }

    mhwInterfaces = CreateMhwInterface(osInterface);
    if (mhwInterfaces->m_miInterface == nullptr ||
        mhwInterfaces->m_veboxInterface == nullptr ||
        mhwInterfaces->m_bltInterface == nullptr)
    {
        deleterOnFailure(true, true);
        return MOS_STATUS_NO_SPACE;
    }

    if (device->Initialize(
        osInterface, mhwInterfaces) != MOS_STATUS_SUCCESS)
    {
        deleterOnFailure(false, false);
        MOS_OS_CHK_STATUS_RETURN(MOS_STATUS_UNINITIALIZED);
    }

    m_mcpyDevice = device;

    return MOS_STATUS_SUCCESS;
}

MhwInterfaces* McpyDeviceXe_Hpm::CreateMhwInterface(
    PMOS_INTERFACE osInterface)
{
    MhwInterfaces::CreateParams params;
    params.Flags.m_vebox = true;
    params.Flags.m_blt   = true;

    // the destroy of interfaces happens when the mcpy deviced deconstructor funcs
    MhwInterfaces *mhw = MhwInterfaces::CreateFactory(params, osInterface);

    return mhw;
}

static bool dg2RegisteredNv12ToP010 =
    MediaFactory<uint32_t, Nv12ToP010Device>::
    Register<Nv12ToP010DeviceXe_Hpm>((uint32_t)IGFX_DG2);

MOS_STATUS Nv12ToP010DeviceXe_Hpm::Initialize(
    PMOS_INTERFACE            osInterface)
{
    CODECHAL_PUBLIC_ASSERTMESSAGE("Not support Nv12 to P010 interfaces.")

    return MOS_STATUS_INVALID_PARAMETER;
}

static bool dg2RegisteredCodecHal =
    MediaFactory<uint32_t, CodechalDevice>::
    Register<CodechalInterfacesXe_Hpm>((uint32_t)IGFX_DG2);

static bool dg2RegisteredCodecHalNext =
    MediaFactory<uint32_t, CodechalDeviceNext>::
        Register<CodechalInterfacesNextXe_Hpm>((uint32_t)IGFX_DG2);

static bool dg2RegisteredMhwNext =
    MediaFactory<uint32_t, MhwInterfacesNext>::
        Register<MhwInterfacesDg2_Next>((uint32_t)IGFX_DG2);

MOS_STATUS MhwInterfacesDg2_Next::Initialize(
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

    if ((params.m_isCp == false) && (params.Flags.m_value == 0))
    {
        MHW_ASSERTMESSAGE("No MHW interfaces were requested for creation.");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    // MHW_CP and MHW_MI must always be created
    MOS_STATUS status;
    m_cpInterface = osInterface->pfnCreateMhwCpInterface(osInterface);
    MHW_MI_CHK_NULL(m_cpInterface);
    m_miInterface = std::make_shared<Mi>(m_cpInterface, osInterface);

    auto ptr = std::make_shared<mhw::mi::xe_xpm_base::Impl>(osInterface);
    m_miItf       = std::static_pointer_cast<mhw::mi::Itf>(ptr);
    ptr->SetCpInterface(m_cpInterface, m_miItf);

    if (params.Flags.m_render)
    {
        m_renderInterface =
            MOS_New(Render, m_miInterface.get(), osInterface, gtSystemInfo, params.m_heapMode);
        auto renderPtr = std::make_shared<mhw::render::xe_hpg::Impl>(osInterface);
        m_renderItf    = std::static_pointer_cast<mhw::render::Itf>(renderPtr);
    }
    if (params.Flags.m_stateHeap)
    {
        m_stateHeapInterface =
            MOS_New(StateHeap, osInterface, params.m_heapMode);
    }
    if (params.Flags.m_sfc)
    {
        m_sfcInterface = MOS_New(Sfc, osInterface);
    }
    if (params.Flags.m_vebox)
    {
        m_veboxInterface = MOS_New(Vebox, osInterface);
    }

    if (params.Flags.m_vdboxAll || params.Flags.m_mfx)
    {
        m_mfxItf = nullptr;
    }
    if (params.Flags.m_vdboxAll || params.Flags.m_hcp)
    {
        m_hcpItf = std::make_shared<mhw::vdbox::hcp::xe_xpm_base::xe_hpm::Impl>(osInterface);
    }
    if (params.Flags.m_vdboxAll)
    {
        auto ptr = std::make_shared<mhw::vdbox::avp::xe_hpm::Impl>(osInterface);
        m_avpItf = std::static_pointer_cast<mhw::vdbox::avp::Itf>(ptr);
    }
    if (params.Flags.m_vdboxAll || params.Flags.m_huc)
    {
        auto ptr = std::make_shared<mhw::vdbox::huc::xe_hpm::Impl>(osInterface, m_cpInterface);
        m_hucItf = std::static_pointer_cast<mhw::vdbox::huc::Itf>(ptr);
    }
    if (params.Flags.m_vdboxAll || params.Flags.m_vdenc)
    {
        auto ptr = std::make_shared<mhw::vdbox::vdenc::xe_hpm::Impl>(osInterface);
        m_vdencItf = std::static_pointer_cast<mhw::vdbox::vdenc::Itf>(ptr);
    }
    if (params.Flags.m_blt)
    {
        m_bltInterface = MOS_New(Blt, osInterface);
    }

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Destroys all created MHW interfaces
//! \details  If the HAL creation fails, this is used for cleanup
//!
void MhwInterfacesDg2_Next::Destroy()
{
    MhwInterfacesNext::Destroy();
    MOS_Delete(m_sfcInterface);
    MOS_Delete(m_veboxInterface);
    MOS_Delete(m_bltInterface);
    if (m_renderInterface != nullptr)
    {
        MOS_Delete(m_renderInterface);
    }
}

MOS_STATUS CodechalInterfacesXe_Hpm::Initialize(
    void *standardInfo,
    void *settings,
    MhwInterfaces *mhwInterfaces,
    PMOS_INTERFACE osInterface)
{
    if (standardInfo    == nullptr ||
        mhwInterfaces   == nullptr ||
        osInterface     == nullptr)
    {
        CODECHAL_PUBLIC_ASSERTMESSAGE("CodecHal device is not valid!");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    PCODECHAL_STANDARD_INFO info = ((PCODECHAL_STANDARD_INFO)standardInfo);
    CODECHAL_FUNCTION CodecFunction = info->CodecFunction;

    bool disableScalability = false;
    PLATFORM platform = {};
    osInterface->pfnGetPlatform(osInterface, &platform);
    if((platform.usDeviceID >= 0x56C0)
        && (platform.usDeviceID <= 0x56CF))
    {
        disableScalability = true;
    }

    CodechalHwInterface    *hwInterface    = nullptr;
    CodechalDebugInterface *debugInterface = nullptr;
    CodechalHwInterfaceNext    *hwInterface_next    = nullptr;
    
    auto release_func = [&]() 
    {
        MOS_Delete(hwInterface);
#if USE_CODECHAL_DEBUG_TOOL
        MOS_Delete(debugInterface);
#endif  // USE_CODECHAL_DEBUG_TOOL
    };

    if (CodecHalIsDecode(CodecFunction))
    {
        if(osInterface->bHcpDecScalabilityMode == MOS_SCALABILITY_ENABLE_MODE_USER_FORCE)
        {
            disableScalability = false;
        }
        CODECHAL_PUBLIC_CHK_STATUS_RETURN(CreateCodecHalInterface(mhwInterfaces, hwInterface, debugInterface, osInterface, CodecFunction, disableScalability));

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
            m_codechalDevice = MOS_New(Decode::Vc1, hwInterface, debugInterface, info);
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
            m_codechalDevice = MOS_New(Decode::Vp8, hwInterface, debugInterface, info);
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
        MhwInterfacesNext      *mhwInterfacesNext = nullptr;

        #define RETURN_STATUS_WITH_DELETE(stmt)    \
        {                                          \
            MOS_Delete(mhwInterfacesNext);         \
            return stmt;                           \
        }

        auto release_func_next = [&]() 
        {
            MOS_Delete(mhwInterfacesNext);
            MOS_Delete(hwInterface_next);
#if USE_CODECHAL_DEBUG_TOOL
            MOS_Delete(debugInterface);
#endif  // USE_CODECHAL_DEBUG_TOOL
        };

        CodechalEncoderState *encoder = nullptr;

#if defined (_AVC_ENCODE_VDENC_SUPPORTED)
        if (info->Mode == CODECHAL_ENCODE_MODE_AVC)
        {
            CODECHAL_PUBLIC_CHK_STATUS_RETURN(CreateCodecHalInterface(mhwInterfaces, hwInterface, debugInterface, osInterface, CodecFunction, disableScalability));

            if (CodecHalUsesVdencEngine(info->CodecFunction))
            {
                encoder = MOS_New(Encode::AvcVdenc, hwInterface, debugInterface, info);
            }
            else
            {
                CODECHAL_PUBLIC_ASSERTMESSAGE("Encode allocation failed, AVC VME Encoder is not supported, please use AVC LowPower Encoder instead!");
                CODECHAL_PUBLIC_CHK_STATUS_WITH_DESTROY_RETURN(MOS_STATUS_INVALID_PARAMETER, release_func);
            }
            if (encoder == nullptr)
            {
                CODECHAL_PUBLIC_ASSERTMESSAGE("Encode state creation failed!");
                CODECHAL_PUBLIC_CHK_STATUS_WITH_DESTROY_RETURN(MOS_STATUS_INVALID_PARAMETER, release_func);
            }
            else
            {
                m_codechalDevice = encoder;
            }
        }
        else
#endif
#ifdef _JPEG_ENCODE_SUPPORTED
        if (info->Mode == CODECHAL_ENCODE_MODE_JPEG)
        {
            CODECHAL_PUBLIC_CHK_STATUS_RETURN(CreateCodecHalInterface(mhwInterfaces, hwInterface, debugInterface, osInterface, CodecFunction, disableScalability));

            encoder = MOS_New(Encode::Jpeg, hwInterface, debugInterface, info);
            if (encoder == nullptr)
            {
                CODECHAL_PUBLIC_ASSERTMESSAGE("Encode state creation failed!");
                CODECHAL_PUBLIC_CHK_STATUS_WITH_DESTROY_RETURN(MOS_STATUS_INVALID_PARAMETER, release_func);
            }
            else
            {
                m_codechalDevice = encoder;
            }
            encoder->m_vdboxOneDefaultUsed = true;
        }
        else
#endif
        if (info->Mode == CODECHAL_ENCODE_MODE_MPEG2)
        {
            CODECHAL_PUBLIC_ASSERTMESSAGE("Encode allocation failed, MPEG2 Encoder is not supported!");
            CODECHAL_PUBLIC_CHK_STATUS_WITH_DESTROY_RETURN(MOS_STATUS_INVALID_PARAMETER, release_func);
        }
        else
#ifdef _VP9_ENCODE_VDENC_SUPPORTED
        if (info->Mode == CODECHAL_ENCODE_MODE_VP9)
        {
            CODECHAL_PUBLIC_CHK_STATUS_RETURN(CreateCodecHalInterface(mhwInterfaces, hwInterface, debugInterface, osInterface, CodecFunction, disableScalability));

            encoder = MOS_New(Encode::Vp9, hwInterface, debugInterface, info);

            if (encoder == nullptr)
            {
                CODECHAL_PUBLIC_ASSERTMESSAGE("Encode state creation failed!");
                CODECHAL_PUBLIC_CHK_STATUS_WITH_DESTROY_RETURN(MOS_STATUS_INVALID_PARAMETER, release_func);
            }
            else
            {
                m_codechalDevice = encoder;
            }
        }
        else
#endif
#if defined (_AV1_ENCODE_VDENC_SUPPORTED)
        if (info->Mode == codechalEncodeModeAv1)
        {
            CODECHAL_PUBLIC_CHK_STATUS_RETURN(CreateCodecHalInterface(mhwInterfaces, mhwInterfacesNext, hwInterface_next, debugInterface, osInterface, CodecFunction, disableScalability));

            if (CodecHalUsesVdencEngine(info->CodecFunction))
            {
                m_codechalDevice = MOS_New(Encode::Av1Vdenc, hwInterface_next, debugInterface);
                if (m_codechalDevice == nullptr)
                {
                    CODECHAL_PUBLIC_CHK_STATUS_WITH_DESTROY_RETURN(MOS_STATUS_NULL_POINTER, release_func_next);
                }
                else
                {
                    RETURN_STATUS_WITH_DELETE(MOS_STATUS_SUCCESS);
                }                
            }
            else
            {
                CODECHAL_PUBLIC_CHK_STATUS_WITH_DESTROY_RETURN(MOS_STATUS_INVALID_PARAMETER, release_func_next);
            }
        }
        else
#endif
#if defined (_HEVC_ENCODE_VDENC_SUPPORTED)
        if (info->Mode == CODECHAL_ENCODE_MODE_HEVC)
        {
            CODECHAL_PUBLIC_CHK_STATUS_RETURN(CreateCodecHalInterface(mhwInterfaces, mhwInterfacesNext, hwInterface_next, debugInterface, osInterface, CodecFunction, disableScalability));

            if (CodecHalUsesVdencEngine(info->CodecFunction))
            {
                m_codechalDevice = MOS_New(Encode::HevcVdenc, hwInterface_next, debugInterface);
                if (m_codechalDevice == nullptr)
                {
                    CODECHAL_PUBLIC_ASSERTMESSAGE("Encode state creation failed!");
                    CODECHAL_PUBLIC_CHK_STATUS_WITH_DESTROY_RETURN(MOS_STATUS_INVALID_PARAMETER, release_func_next);
                }
                RETURN_STATUS_WITH_DELETE(MOS_STATUS_SUCCESS);
            }
        }
        else
#endif
        {
            CODECHAL_PUBLIC_ASSERTMESSAGE("Unsupported encode function requested.");
            CODECHAL_PUBLIC_CHK_STATUS_WITH_DESTROY_RETURN(MOS_STATUS_INVALID_PARAMETER, release_func);
        }

        if (mhwInterfacesNext != nullptr)
        {
            MOS_Delete(mhwInterfacesNext);
        }
    }
    else
    {
        CODECHAL_PUBLIC_ASSERTMESSAGE("Unsupported codec function requested.");
        CODECHAL_PUBLIC_CHK_STATUS_WITH_DESTROY_RETURN(MOS_STATUS_INVALID_PARAMETER, release_func);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalInterfacesNextXe_Hpm::Initialize(
    void *standardInfo,
    void *settings,
    MhwInterfacesNext *mhwInterfaces,
    PMOS_INTERFACE osInterface)
{
     bool mdfSupported = true;
#if (_DEBUG || _RELEASE_INTERNAL)
    MOS_USER_FEATURE_VALUE_DATA UserFeatureData;
    MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_MDF_DISABLE_ID,
        &UserFeatureData,
        osInterface->pOsContext);
    mdfSupported = (UserFeatureData.i32Data == 1) ? false : true;
#endif  // (_DEBUG || _RELEASE_INTERNAL)
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

    bool disableScalability = false;
#ifdef _VP9_ENCODE_VDENC_SUPPORTED
    if (info->Mode == CODECHAL_ENCODE_MODE_VP9)
        disableScalability = true;
#endif
    CodechalHwInterfaceNext *hwInterface = MOS_New(Hw, osInterface, CodecFunction, mhwInterfaces, disableScalability);

    if (hwInterface == nullptr)
    {
        CODECHAL_PUBLIC_ASSERTMESSAGE("hwInterface is not valid!");
        return MOS_STATUS_NO_SPACE;
    }

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
        CODECHAL_PUBLIC_ASSERTMESSAGE("Decode allocation failed, Decoder with CodechalDeviceNext is not supported!");
        CODECHAL_PUBLIC_CHK_STATUS_WITH_DESTROY_RETURN(MOS_STATUS_INVALID_PARAMETER, release_func);
 
    }
    else if (CodecHalIsEncode(CodecFunction))
    {
        CodechalEncoderState *encoder = nullptr;

#if defined (_AV1_ENCODE_VDENC_SUPPORTED)
        if (info->Mode == codechalEncodeModeAv1)
        {
            //TODO, use create 
            if (CodecHalUsesVdencEngine(info->CodecFunction))
            {
                m_codechalDevice = MOS_New(Encode::Av1Vdenc, hwInterface, debugInterface);
                if (m_codechalDevice == nullptr)
                {
                    CODECHAL_PUBLIC_CHK_STATUS_WITH_DESTROY_RETURN(MOS_STATUS_NULL_POINTER, release_func);
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

#ifdef _MEDIA_RESERVED
static bool dg2RegisteredCMHal =
    MediaFactory<uint32_t, CMHalDevice>::
    Register<CMHalInterfacesXe_Hpm>((uint32_t)IGFX_DG2);
MOS_STATUS CMHalInterfacesXe_Hpm::Initialize(CM_HAL_STATE *pCmState)
{
    if (pCmState == nullptr)
    {
        MHW_ASSERTMESSAGE("pCmState is nullptr.")
        return MOS_STATUS_INVALID_PARAMETER;
    }
    CMHal *device = MOS_New(CMHal, pCmState);
    if (device == nullptr)
    {
        MHW_ASSERTMESSAGE("Create CM Hal interfaces failed.")
        return MOS_STATUS_NO_SPACE;
    }
    device->SetCopyKernelIsa((void*)pGPUCopy_kernel_isa_dg2, iGPUCopy_kernel_isa_size_dg2);
    device->SetInitKernelIsa((void*)pGPUInit_kernel_isa_dg2, iGPUInit_kernel_isa_size_dg2);

    m_cmhalDevice = device;
    m_cmhalDevice->SetGenPlatformInfo(PLATFORM_INTEL_DG2, PLATFORM_INTEL_GT2, "DG2");
    uint32_t cisaIDs[] = { GENX_DG2 , GENX_XEHP };
    m_cmhalDevice->AddSupportedCisaIDs(cisaIDs, sizeof(cisaIDs)/sizeof(uint32_t));

    if (pCmState->skuTable && MEDIA_IS_SKU(pCmState->skuTable, FtrCCSNode))
    {
        m_cmhalDevice->SetRedirectRcsToCcs(true);
    }

    m_cmhalDevice->SetDefaultMOCS(MOS_CM_RESOURCE_USAGE_L1_Enabled_SurfaceState);
    m_cmhalDevice->m_l3Plane = DG2_L3_PLANES;
    m_cmhalDevice->m_l3ConfigCount = DG2_L3_CONFIG_COUNT;
    return MOS_STATUS_SUCCESS;
}
#endif

static bool dg2RegisteredRenderHal =
    MediaFactory<uint32_t, RenderHalDevice>::
    Register<RenderHalInterfacesXe_Hpg>((uint32_t)IGFX_DG2);
MOS_STATUS RenderHalInterfacesXe_Hpg::Initialize()
{
    m_renderhalDevice = MOS_New(XRenderHal);
    if (m_renderhalDevice == nullptr)
    {
        MHW_ASSERTMESSAGE("Create Render Hal interfaces failed.")
        return MOS_STATUS_NO_SPACE;
    }
    return MOS_STATUS_SUCCESS;
}
static bool dg2RegisteredDecodeHistogram =
MediaFactory<uint32_t, DecodeHistogramDevice>::
Register<DecodeHistogramDeviceXe_Hpm>((uint32_t)IGFX_DG2);
MOS_STATUS DecodeHistogramDeviceXe_Hpm::Initialize(
    CodechalHwInterface       *hwInterface,
    PMOS_INTERFACE            osInterface)
{
    m_decodeHistogramDevice = MOS_New(DecodeHistogramG12, hwInterface, osInterface);
    if (m_decodeHistogramDevice == nullptr)
    {
        MHW_ASSERTMESSAGE("Create decode histogram  interfaces failed.")
            return MOS_STATUS_NO_SPACE;
    }

    return MOS_STATUS_SUCCESS;
}

static bool dg2RegisteredHwInfo =
    MediaFactory<uint32_t, MediaInterfacesHwInfoDevice>::Register<MediaInterfacesHwInfoDeviceDg2>((uint32_t)IGFX_DG2);

#define IP_VERSION_XE_HPM      0x1207

MOS_STATUS MediaInterfacesHwInfoDeviceDg2::RefreshRevId(PLATFORM &platform, MEDIA_WA_TABLE *waTable)
{
    if (waTable == nullptr)
    {
        CODECHAL_PUBLIC_ASSERTMESSAGE("waTable is null!");
        return MOS_STATUS_INVALID_PARAMETER;
    }
    if (!MEDIA_IS_WA(waTable, WaEnableOnlyASteppingFeatures) && (platform.usRevId == 0 || platform.usRevId == 1))
    {
        platform.usRevId = 4;
    }

    return MOS_STATUS_SUCCESS;
};

MOS_STATUS MediaInterfacesHwInfoDeviceDg2::Initialize(PLATFORM platform)
{
    m_hwInfo.SetDeviceInfo(IP_VERSION_XE_HPM, platform.usRevId);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalInterfacesXe_Hpm::CreateCodecHalInterface(MhwInterfaces *mhwInterfaces,
                                                             CodechalHwInterface    *&pHwInterface,
                                                             CodechalDebugInterface *&pDebugInterface,
                                                             PMOS_INTERFACE         osInterface,
                                                             CODECHAL_FUNCTION      CodecFunction,
                                                             bool                   disableScalability)
{
    pHwInterface = MOS_New(Hw, osInterface, CodecFunction, mhwInterfaces, disableScalability);
    if (pHwInterface == nullptr)
    {
        CODECHAL_PUBLIC_ASSERTMESSAGE("hwInterface is not valid!");
        return MOS_STATUS_NO_SPACE;
    }
    pHwInterface->m_hwInterfaceNext                            = MOS_New(CodechalHwInterfaceNext, osInterface, pHwInterface->IsDisableScalability());
    if (pHwInterface->m_hwInterfaceNext == nullptr)
    {
        MOS_Delete(pHwInterface);
        CODECHAL_PUBLIC_ASSERTMESSAGE("hwInterfaceNext is not valid!");
        return MOS_STATUS_NO_SPACE;
    }
    pHwInterface->m_hwInterfaceNext->pfnCreateDecodeSinglePipe = decode::DecodeScalabilitySinglePipe::CreateDecodeSinglePipe;
    pHwInterface->m_hwInterfaceNext->pfnCreateDecodeMultiPipe  = decode::DecodeScalabilityMultiPipe::CreateDecodeMultiPipe;
    pHwInterface->m_hwInterfaceNext->SetMediaSfcInterface(pHwInterface->GetMediaSfcInterface());

#if USE_CODECHAL_DEBUG_TOOL
    pDebugInterface = MOS_New(CodechalDebugInterface);
    if (pDebugInterface == nullptr)
    {
        MOS_Delete(pHwInterface);
        CODECHAL_PUBLIC_ASSERTMESSAGE("debugInterface is not valid!");
        return MOS_STATUS_NO_SPACE;
    }
    if ((pDebugInterface)->Initialize(pHwInterface, CodecFunction) != MOS_STATUS_SUCCESS)
    {
        MOS_Delete(pHwInterface);
        MOS_Delete(pDebugInterface);
        CODECHAL_PUBLIC_ASSERTMESSAGE("Debug interface creation failed!");
        return MOS_STATUS_INVALID_PARAMETER;
    }
#endif // USE_CODECHAL_DEBUG_TOOL
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalInterfacesXe_Hpm::CreateCodecHalInterface(MhwInterfaces *mhwInterfaces,
    MhwInterfacesNext                                                     *&pMhwInterfacesNext,
    CodechalHwInterfaceNext                                               *&pHwInterface,
    CodechalDebugInterface                                                *&pDebugInterface,
    PMOS_INTERFACE                                                          osInterface,
    CODECHAL_FUNCTION                                                       CodecFunction,
    bool                                                                    disableScalability)
{
    if (mhwInterfaces != nullptr)
    {
        if (((MhwInterfacesDg2 *)mhwInterfaces)->m_avpInterface != nullptr)
        {
            MOS_Delete(((MhwInterfacesDg2 *)mhwInterfaces)->m_avpInterface);
        }
        mhwInterfaces->Destroy();
    }

    pMhwInterfacesNext = nullptr;
    MhwInterfacesNext::CreateParams params;
    MOS_ZeroMemory(&params, sizeof(params));
    params.Flags.m_render   = true;
    params.Flags.m_sfc      = true;
    params.Flags.m_vdboxAll = true;
    params.Flags.m_vebox    = true;
    params.m_heapMode       = (uint8_t)2;
    params.m_isDecode       = CodecHalIsDecode(CodecFunction);
    pMhwInterfacesNext      = MhwInterfacesNext::CreateFactory(params, osInterface);

    if (pMhwInterfacesNext == nullptr)
    {
        CODECHAL_PUBLIC_ASSERTMESSAGE("mhwInterfacesNext is not valid!");
        return MOS_STATUS_NO_SPACE;
    }

    pHwInterface = MOS_New(CodechalHwInterfaceNextXe_Hpm, osInterface, CodecFunction, pMhwInterfacesNext, disableScalability);

    if (pHwInterface == nullptr)
    {
        CODECHAL_PUBLIC_ASSERTMESSAGE("hwInterface is not valid!");
        return MOS_STATUS_NO_SPACE;
    }
#if USE_CODECHAL_DEBUG_TOOL
    pDebugInterface = MOS_New(CodechalDebugInterface);
    if (pDebugInterface == nullptr)
    {
        MOS_Delete(pHwInterface);
        CODECHAL_PUBLIC_ASSERTMESSAGE("debugInterface is not valid!");
        return MOS_STATUS_NO_SPACE;
    }
    if ((pDebugInterface)->Initialize(pHwInterface, CodecFunction) != MOS_STATUS_SUCCESS)
    {
        MOS_Delete(pHwInterface);
        MOS_Delete(pDebugInterface);
        CODECHAL_PUBLIC_ASSERTMESSAGE("Debug interface creation failed!");
        return MOS_STATUS_INVALID_PARAMETER;
    }
#endif  // USE_CODECHAL_DEBUG_TOOL
    return MOS_STATUS_SUCCESS;
}