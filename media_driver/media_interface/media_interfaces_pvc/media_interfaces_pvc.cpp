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
//! \file     media_interfaces_pvc.cpp

//! \brief    Helps with PVC factory creation.
//!

#include "media_interfaces_pvc.h"
#include "codechal.h"
#include "codechal_debug.h"
#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
#include "igcodeckrn_g12.h"
#endif
#include "codechal_hw_next_xe_xpm_plus.h"
#include "mhw_render_xe_hpg_impl.h"

extern template class MediaFactory<uint32_t, MhwInterfaces>;
extern template class MediaFactory<uint32_t, MmdDevice>;
extern template class MediaFactory<uint32_t, McpyDevice>;
extern template class MediaFactory<uint32_t, CodechalDevice>;
extern template class MediaFactory<uint32_t, CMHalDevice>;
extern template class MediaFactory<uint32_t, VphalDevice>;
extern template class MediaFactory<uint32_t, RenderHalDevice>;
extern template class MediaFactory<uint32_t, Nv12ToP010Device>;
extern template class MediaFactory<uint32_t, DecodeHistogramDevice>;

// Swith to use new media factory template
extern template class MediaFactory<uint32_t, MhwInterfacesNext>;

static bool pvcRegisteredVphal =
MediaFactory<uint32_t, VphalDevice>::
Register<VphalInterfacesXe_Xpm_Plus>((uint32_t)IGFX_PVC);

MOS_STATUS VphalInterfacesXe_Xpm_Plus::Initialize(
    PMOS_INTERFACE  osInterface,
    bool            bInitVphalState,
    MOS_STATUS      *eStatus,
    bool            clearViewMode)
{
    m_vpBase = MOS_New(
        VphalState,
        osInterface,
        eStatus);
    return *eStatus;
}

static bool pvcRegisteredMhw =
    MediaFactory<uint32_t, MhwInterfaces>::
    Register<MhwInterfacesPvc>((uint32_t)IGFX_PVC);

#define PLATFORM_INTEL_PVC    18
#define GENX_XEHP              11
#define GENX_PVC              15

MOS_STATUS MhwInterfacesPvc::Initialize(
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
    if (params.Flags.m_vdboxAll)
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

static bool pvcRegisteredMhwNext =
    MediaFactory<uint32_t, MhwInterfacesNext>::
        Register<MhwInterfacesPvc_Next>((uint32_t)IGFX_PVC);

MOS_STATUS MhwInterfacesPvc_Next::Initialize(
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

    auto ptr      = std::make_shared<mhw::mi::xe_xpm_base::Impl>(osInterface);
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
        m_hcpItf = std::make_shared<mhw::vdbox::hcp::xe_xpm_base::xe_xpm_plus::Impl>(osInterface);
    }
    if (params.Flags.m_vdboxAll || params.Flags.m_huc)
    {
        m_hucItf = std::make_shared<mhw::vdbox::huc::xe_xpm_plus::Impl>(osInterface, m_cpInterface);
    }
    if (params.Flags.m_vdboxAll || params.Flags.m_vdenc)
    {
#ifdef IGFX_PVC_ENABLE_NON_UPSTREAM
        m_vdencItf = std::make_shared<mhw::vdbox::vdenc::xe_xpm_plus::Impl>(osInterface);
#endif
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
void MhwInterfacesPvc_Next::Destroy()
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

#ifdef _MMC_SUPPORTED
static bool pvcRegisteredMmd =
    MediaFactory<uint32_t, MmdDevice>::
    Register<MmdDeviceXe_Xpm_Plus>((uint32_t)IGFX_PVC);

MOS_STATUS MmdDeviceXe_Xpm_Plus::Initialize(
    PMOS_INTERFACE osInterface,
    MhwInterfaces *mhwInterfaces)
{
#define MMD_FAILURE()                                       \
{                                                           \
    if (device != nullptr)                                  \
    {                                                       \
        MOS_Delete(device);                                 \
    }                                                       \
    return MOS_STATUS_NO_SPACE;                             \
}
    MHW_FUNCTION_ENTER;
    Mmd *device = nullptr;

    if (mhwInterfaces->m_miInterface == nullptr)
    {
        MMD_FAILURE();
    }
    if (mhwInterfaces->m_renderInterface == nullptr)
    {
        MMD_FAILURE();
    }
    device = MOS_New(Mmd);

    if (device == nullptr)
    {
        MMD_FAILURE();
    }

    if (device->Initialize(
            osInterface,
            mhwInterfaces->m_cpInterface,
            mhwInterfaces->m_miInterface,
            mhwInterfaces->m_renderInterface) != MOS_STATUS_SUCCESS)
    {
        // Vebox/mi/cp interface will gove control to mmd device, will release will be in destructure func
        // set as null to avoid double free in driver
        mhwInterfaces->m_cpInterface = nullptr;
        mhwInterfaces->m_miInterface = nullptr;
        mhwInterfaces->m_veboxInterface = nullptr;
        MMD_FAILURE();
    }

    m_mmdDevice = device;

    return MOS_STATUS_SUCCESS;
}

MhwInterfaces* MmdDeviceXe_Xpm_Plus::CreateMhwInterface(
    PMOS_INTERFACE osInterface)
{
    MhwInterfaces::CreateParams params;
    params.Flags.m_render = true;

    // the destroy of interfaces happens when the mmd deviced deconstructor funcs
    MhwInterfaces *mhw = MhwInterfaces::CreateFactory(params, osInterface);

    return mhw;
}
#endif

static bool pvcRegisteredMcpy =
    MediaFactory<uint32_t, McpyDevice>::
    Register<McpyDeviceXe_Xpm_Plus>((uint32_t)IGFX_PVC);

MOS_STATUS McpyDeviceXe_Xpm_Plus::Initialize(
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

MhwInterfaces* McpyDeviceXe_Xpm_Plus::CreateMhwInterface(
    PMOS_INTERFACE osInterface)
{
    MhwInterfaces::CreateParams params;
    params.Flags.m_blt    = true;
    params.Flags.m_render   = true;
    params.Flags.m_vebox = false;
    params.Flags.m_sfc = false;
    // the destroy of interfaces happens when the mcpy deviced deconstructor funcs
    MhwInterfaces *mhw = MhwInterfaces::CreateFactory(params, osInterface);

    return mhw;
}

static bool pvcRegisteredNv12ToP010 =
    MediaFactory<uint32_t, Nv12ToP010Device>::
    Register<Nv12ToP010DeviceXe_Xpm_Plus>((uint32_t)IGFX_PVC);

MOS_STATUS Nv12ToP010DeviceXe_Xpm_Plus::Initialize(
    PMOS_INTERFACE            osInterface)
{
    CODECHAL_PUBLIC_ASSERTMESSAGE("Not support Nv12 to P010 interfaces.")

    return MOS_STATUS_INVALID_PARAMETER;
}

static bool pvcRegisteredCodecHal =
    MediaFactory<uint32_t, CodechalDevice>::
    Register<CodechalInterfacesXe_Xpm_Plus>((uint32_t)IGFX_PVC);

MOS_STATUS CodechalInterfacesXe_Xpm_Plus::Initialize(
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

    bool mdfSupported = true;
#if (_DEBUG || _RELEASE_INTERNAL)
    MOS_USER_FEATURE_VALUE_DATA     UserFeatureData;
    MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_MDF_DISABLE_ID,
        &UserFeatureData,
        osInterface->pOsContext);
    mdfSupported = (UserFeatureData.i32Data == 1) ? false : true;
#endif // (_DEBUG || _RELEASE_INTERNAL)

    PCODECHAL_STANDARD_INFO info = ((PCODECHAL_STANDARD_INFO)standardInfo);
    CODECHAL_FUNCTION CodecFunction = info->CodecFunction;

    bool disableScalability = true;
    CodechalHwInterface *hwInterface = nullptr;
    CodechalDebugInterface *debugInterface = nullptr;
    CodechalHwInterfaceNext *hwInterface_next = nullptr;

    if (CodecHalIsDecode(CodecFunction))
    {
        CreateCodecHalInterface(mhwInterfaces, hwInterface, debugInterface, osInterface, CodecFunction, disableScalability);

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
            return MOS_STATUS_INVALID_PARAMETER;
        }

        if (m_codechalDevice == nullptr)
        {
            MOS_Delete(hwInterface);
            mhwInterfaces->SetDestroyState(true);
#if USE_CODECHAL_DEBUG_TOOL
            MOS_Delete(debugInterface);
#endif
            CODECHAL_PUBLIC_ASSERTMESSAGE("Decoder device creation failed!");
            return MOS_STATUS_NO_SPACE;
        }
    }
#ifdef IGFX_PVC_ENABLE_NON_UPSTREAM
    else if (CodecHalIsEncode(CodecFunction))
    {
        MhwInterfacesNext      *mhwInterfacesNext = nullptr;

        #define RETRUN_STATUS_WITH_DELETE(stmt)    \
        {                                          \
            MOS_Delete(mhwInterfacesNext);         \
            return stmt;                           \
        }

        CodechalEncoderState *encoder = nullptr;
#if defined (_AVC_ENCODE_VDENC_SUPPORTED)
        if (info->Mode == CODECHAL_ENCODE_MODE_AVC)
        {
            CreateCodecHalInterface(mhwInterfaces, hwInterface, debugInterface, osInterface, CodecFunction, disableScalability);
            if (CodecHalUsesVdencEngine(info->CodecFunction))
            {
                encoder = MOS_New(Encode::AvcVdenc, hwInterface, debugInterface, info);
            }
            else
            {
                CODECHAL_PUBLIC_ASSERTMESSAGE("Encode allocation failed, AVC VME Encoder is not supported, please use AVC LowPower Encoder instead!");
                return MOS_STATUS_INVALID_PARAMETER;
            }
            if (encoder == nullptr)
            {
                CODECHAL_PUBLIC_ASSERTMESSAGE("Encode state creation failed!");
                return MOS_STATUS_INVALID_PARAMETER;
            }
            else
            {
                m_codechalDevice = encoder;
            }
        }
        else
#endif
#ifdef _VP9_ENCODE_VDENC_SUPPORTED
        if (info->Mode == CODECHAL_ENCODE_MODE_VP9)
        {
            CreateCodecHalInterface(mhwInterfaces, hwInterface, debugInterface, osInterface, CodecFunction, disableScalability);
            encoder = MOS_New(Encode::Vp9, hwInterface, debugInterface, info);
            if (encoder == nullptr)
            {
                CODECHAL_PUBLIC_ASSERTMESSAGE("Encode state creation failed!");
                return MOS_STATUS_INVALID_PARAMETER;
            }
            else
            {
                m_codechalDevice = encoder;
            }
        }
        else
#endif
        if (info->Mode == CODECHAL_ENCODE_MODE_MPEG2)
        {
            CODECHAL_PUBLIC_ASSERTMESSAGE("Encode allocation failed, MPEG2 Encoder is not supported!");
            return MOS_STATUS_INVALID_PARAMETER;
        }
        else
#ifdef _JPEG_ENCODE_SUPPORTED
        if (info->Mode == CODECHAL_ENCODE_MODE_JPEG)
        {
            CreateCodecHalInterface(mhwInterfaces, hwInterface, debugInterface, osInterface, CodecFunction, disableScalability);
            encoder = MOS_New(Encode::Jpeg, hwInterface, debugInterface, info);
            if (encoder == nullptr)
            {
                CODECHAL_PUBLIC_ASSERTMESSAGE("Encode state creation failed!");
                return MOS_STATUS_INVALID_PARAMETER;
            }
            else
            {
                m_codechalDevice = encoder;
            }
            encoder->m_vdboxOneDefaultUsed = true;
        }
        else
#endif
#if defined (_HEVC_ENCODE_VME_SUPPORTED) || defined (_HEVC_ENCODE_VDENC_SUPPORTED)
        if (info->Mode == CODECHAL_ENCODE_MODE_HEVC)
        {
            if (CodecHalUsesVdencEngine(info->CodecFunction))
            {
            #ifdef _HEVC_ENCODE_VDENC_SUPPORTED
            #ifdef _APOGEIOS_SUPPORTED
                bool apogeiosEnable = true;
                MOS_USER_FEATURE_VALUE_DATA         userFeatureData;
                MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));

                userFeatureData.i32Data = apogeiosEnable;
                userFeatureData.i32DataFlag = MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE;
                MOS_UserFeature_ReadValue_ID(
                    nullptr,
                    __MEDIA_USER_FEATURE_VALUE_APOGEIOS_ENABLE_ID,
                    &userFeatureData,
                    osInterface->pOsContext);
                apogeiosEnable = (userFeatureData.i32Data) ? true : false;

                if (apogeiosEnable)
                {
                    CreateCodecHalInterface(mhwInterfaces, mhwInterfacesNext, hwInterface_next, debugInterface, osInterface, CodecFunction, disableScalability);
                    m_codechalDevice = MOS_New(EncodeHevcVdencPipelineAdapterXe_Xpm_Plus, hwInterface_next, debugInterface);
                    if (m_codechalDevice == nullptr)
                    {
                        CODECHAL_PUBLIC_ASSERTMESSAGE("Encode state creation failed!");
                        RETRUN_STATUS_WITH_DELETE(MOS_STATUS_INVALID_PARAMETER);
                    }
                    RETRUN_STATUS_WITH_DELETE(MOS_STATUS_SUCCESS);
                }
                else
            #endif
                {
                    CreateCodecHalInterface(mhwInterfaces, hwInterface, debugInterface, osInterface, CodecFunction, disableScalability);
                    encoder = MOS_New(Encode::HevcVdenc, hwInterface, debugInterface, info);
                }
            #endif
            }
            else
            {

                // disable HEVC encode MDF path.
                CreateCodecHalInterface(mhwInterfaces, hwInterface, debugInterface, osInterface, CodecFunction, disableScalability);
            #ifdef _HEVC_ENCODE_VME_SUPPORTED
                if (!mdfSupported)
                {
                    encoder = MOS_New(Encode::HevcEnc, hwInterface, debugInterface, info);
                }
                else
                {
                    encoder = MOS_New(Encode::HevcMbenc, hwInterface, debugInterface, info);
                }
            #endif
            }

            if (encoder == nullptr)
            {
                CODECHAL_PUBLIC_ASSERTMESSAGE("Encode state creation failed!");
                return MOS_STATUS_INVALID_PARAMETER;
            }
            else
            {
                m_codechalDevice = encoder;
            }

            encoder->m_kernelBase = (uint8_t*)IGCODECKRN_G12;
        }
        else
#endif
        {
            CODECHAL_PUBLIC_ASSERTMESSAGE("Unsupported encode function requested.");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        if (info->Mode != CODECHAL_ENCODE_MODE_JPEG)
        {
            if (mdfSupported && info->Mode == CODECHAL_ENCODE_MODE_HEVC && !CodecHalUsesVdencEngine(info->CodecFunction))
            {
                if ((encoder->m_cscDsState = MOS_New(Encode::CscDsMdf, encoder)) == nullptr)
                {
                    return MOS_STATUS_INVALID_PARAMETER;
                }
            }
            else
            {
                // Create CSC and Downscaling interface
                if ((encoder->m_cscDsState = MOS_New(Encode::CscDs, encoder)) == nullptr)
                {
                    return MOS_STATUS_INVALID_PARAMETER;
                }
            }
        }

        if (mhwInterfacesNext != nullptr)
        {
            MOS_Delete(mhwInterfacesNext);
        }
    }
#endif
    else
    {
        CODECHAL_PUBLIC_ASSERTMESSAGE("Unsupported codec function requested.");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    return MOS_STATUS_SUCCESS;
}

#ifdef IGFX_PVC_ENABLE_NON_UPSTREAM
static bool pvcRegisteredCMHal =
    MediaFactory<uint32_t, CMHalDevice>::
    Register<CMHalInterfacesXe_Xpm_Plus>((uint32_t)IGFX_PVC);
MOS_STATUS CMHalInterfacesXe_Xpm_Plus::Initialize(CM_HAL_STATE *pCmState)
{
    if (pCmState == nullptr)
    {
        MHW_ASSERTMESSAGE("pCmState is nullptr.")
        return MOS_STATUS_INVALID_PARAMETER;
    }
    m_cmhalDevice = MOS_New(CMHal, pCmState);
    if (m_cmhalDevice == nullptr)
    {
        MHW_ASSERTMESSAGE("Create CM Hal interfaces failed.")
        return MOS_STATUS_NO_SPACE;
    }
    m_cmhalDevice->SetGenPlatformInfo(PLATFORM_INTEL_PVC, PLATFORM_INTEL_GT2, "PVC");
    uint32_t cisaIDs[] = { GENX_PVC , GENX_XEHP };
    m_cmhalDevice->AddSupportedCisaIDs(cisaIDs, sizeof(cisaIDs)/sizeof(uint32_t));
    m_cmhalDevice->SetRedirectRcsToCcs(true);
    m_cmhalDevice->SetDefaultMOCS(MOS_CM_RESOURCE_USAGE_L1_Enabled_SurfaceState);
    m_cmhalDevice->m_l3Plane = PVC_L3_PLANES;
    m_cmhalDevice->m_l3ConfigCount = PVC_L3_CONFIG_COUNT;
    return MOS_STATUS_SUCCESS;
}
#endif

static bool pvcRegisteredRenderHal =
    MediaFactory<uint32_t, RenderHalDevice>::
    Register<RenderHalInterfacesXe_Hpc>((uint32_t)IGFX_PVC);
MOS_STATUS RenderHalInterfacesXe_Hpc::Initialize()
{
    m_renderhalDevice = MOS_New(XRenderHal);
    if (m_renderhalDevice == nullptr)
    {
        MHW_ASSERTMESSAGE("Create Render Hal interfaces failed.")
        return MOS_STATUS_NO_SPACE;
    }
    return MOS_STATUS_SUCCESS;
}
static bool pvcRegisteredDecodeHistogram =
MediaFactory<uint32_t, DecodeHistogramDevice>::
Register<DecodeHistogramDeviceXe_Xpm_Plus>((uint32_t)IGFX_PVC);
MOS_STATUS DecodeHistogramDeviceXe_Xpm_Plus::Initialize(
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

MOS_STATUS CodechalInterfacesXe_Xpm_Plus::CreateCodecHalInterface(MhwInterfaces          *mhwInterfaces,
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
    pHwInterface->m_hwInterfaceNext                            = MOS_New(CodechalHwInterfaceNext, osInterface);
    if (pHwInterface->m_hwInterfaceNext == nullptr)
    {
        MOS_Delete(pHwInterface);
        mhwInterfaces->SetDestroyState(true);
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
        mhwInterfaces->SetDestroyState(true);
        CODECHAL_PUBLIC_ASSERTMESSAGE("debugInterface is not valid!");
        return MOS_STATUS_NO_SPACE;
    }
    if (pDebugInterface->Initialize(pHwInterface, CodecFunction) != MOS_STATUS_SUCCESS)
    {
        MOS_Delete(pHwInterface);
        mhwInterfaces->SetDestroyState(true);
        MOS_Delete(pDebugInterface);
        CODECHAL_PUBLIC_ASSERTMESSAGE("Debug interface creation failed!");
        return MOS_STATUS_INVALID_PARAMETER;
    }
#endif // USE_CODECHAL_DEBUG_TOOL
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalInterfacesXe_Xpm_Plus::CreateCodecHalInterface(MhwInterfaces *mhwInterfaces,
    MhwInterfacesNext                                                          *&pMhwInterfacesNext,
    CodechalHwInterfaceNext                                                    *&pHwInterface,
    CodechalDebugInterface                                                     *&pDebugInterface,
    PMOS_INTERFACE                                                               osInterface,
    CODECHAL_FUNCTION                                                            CodecFunction,
    bool                                                                         disableScalability)
{
    if (mhwInterfaces != nullptr)
    {
        if (((MhwInterfacesPvc *)mhwInterfaces)->m_avpInterface != nullptr)
        {
            MOS_Delete(((MhwInterfacesPvc *)mhwInterfaces)->m_avpInterface);
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

    pHwInterface = MOS_New(CodechalHwInterfaceNextXe_Xpm_Plus, osInterface, CodecFunction, pMhwInterfacesNext, disableScalability);

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
        (pMhwInterfacesNext)->SetDestroyState(true);
        CODECHAL_PUBLIC_ASSERTMESSAGE("debugInterface is not valid!");
        return MOS_STATUS_NO_SPACE;
    }
    if ((pDebugInterface)->Initialize(pHwInterface, CodecFunction) != MOS_STATUS_SUCCESS)
    {
        MOS_Delete(pHwInterface);
        (pMhwInterfacesNext)->SetDestroyState(true);
        MOS_Delete(pDebugInterface);
        CODECHAL_PUBLIC_ASSERTMESSAGE("Debug interface creation failed!");
        return MOS_STATUS_INVALID_PARAMETER;
    }
#endif  // USE_CODECHAL_DEBUG_TOOL
    return MOS_STATUS_SUCCESS;
}
