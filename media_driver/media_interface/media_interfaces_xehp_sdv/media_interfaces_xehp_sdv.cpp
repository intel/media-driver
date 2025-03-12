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
//! \file     media_interfaces_xehp_sdv.cpp

//! \brief    Helps with TGL factory creation.
//!

#include "media_interfaces_xehp_sdv.h"
#include "codechal_debug.h"
#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
#include "igcodeckrn_g12.h"
#endif
#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE) && defined(IGFX_XEHP_SDV_ENABLE_NON_UPSTREAM)
#include "cm_gpucopy_kernel_xe_xpm.h"
#include "cm_gpuinit_kernel_xe_xpm.h"
#else
unsigned int iGPUCopy_kernel_isa_size_xe_xpm = 0;
unsigned int iGPUInit_kernel_isa_size_xe_xpm = 0;
unsigned char *pGPUCopy_kernel_isa_xe_xpm = nullptr;
unsigned char *pGPUInit_kernel_isa_xe_xpm = nullptr;
#endif
#include "vp_pipeline_adapter_xe_xpm.h"
#include "vp_platform_interface_xe_xpm.h"

extern template class MediaFactory<uint32_t, MhwInterfaces>;
extern template class MediaFactory<uint32_t, MmdDevice>;
extern template class MediaFactory<uint32_t, McpyDevice>;
extern template class MediaFactory<uint32_t, CodechalDevice>;
extern template class MediaFactory<uint32_t, CMHalDevice>;
extern template class MediaFactory<uint32_t, VphalDevice>;
extern template class MediaFactory<uint32_t, RenderHalDevice>;
extern template class MediaFactory<uint32_t, Nv12ToP010Device>;
extern template class MediaFactory<uint32_t, DecodeHistogramDevice>;
extern template class MediaFactory<uint32_t, MediaInterfacesHwInfoDevice>;

#define IP_VERSION_XE_XPM 0x1205

static bool xehpRegisteredVphal =
MediaFactory<uint32_t, VphalDevice>::
Register<VphalInterfacesXe_Xpm>((uint32_t)IGFX_XE_HP_SDV);

MOS_STATUS VphalInterfacesXe_Xpm::Initialize(
    PMOS_INTERFACE  osInterface,
    bool            bInitVphalState,
    MOS_STATUS      *eStatus,
    bool            clearViewMode)
{
    MOS_OS_CHK_NULL_RETURN(osInterface);
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
        vp::VpPlatformInterface *vpPlatformInterface = MOS_New(vp::VpPlatformInterfaceXe_Xpm, osInterface);
        if (nullptr == vpPlatformInterface)
        {
            *eStatus = MOS_STATUS_NULL_POINTER;
            return *eStatus;
        }

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
            VpPipelineAdapterXe_Xpm,
            osInterface,
            *vpPlatformInterface,
            *eStatus);
        if (nullptr == m_vpBase)
        {
            MOS_Delete(vpPlatformInterface);
            *eStatus = MOS_STATUS_NULL_POINTER;
            return *eStatus;
        }
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

MOS_STATUS VphalInterfacesXe_Xpm::CreateVpPlatformInterface(
    PMOS_INTERFACE osInterface,
    MOS_STATUS *   eStatus)
{
    vp::VpPlatformInterface *vpPlatformInterface = MOS_New(vp::VpPlatformInterfaceXe_Xpm, osInterface);
    if (nullptr == vpPlatformInterface)
    {
        *eStatus = MOS_STATUS_NULL_POINTER;
    }
    else
    {
        m_vpPlatformInterface = vpPlatformInterface;
        *eStatus              = MOS_STATUS_SUCCESS;
    }
    return *eStatus;
}

static bool xehpRegisteredMhw =
    MediaFactory<uint32_t, MhwInterfaces>::
    Register<MhwInterfacesXehp_Sdv>((uint32_t)IGFX_XE_HP_SDV);

#define PLATFORM_INTEL_TGL 14
#define GENX_XEHP           11

MOS_STATUS MhwInterfacesXehp_Sdv::Initialize(
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

#ifdef _MMC_SUPPORTED
static bool xehpRegisteredMmd =
    MediaFactory<uint32_t, MmdDevice>::
    Register<MmdDeviceXe_Xpm>((uint32_t)IGFX_XE_HP_SDV);

MOS_STATUS MmdDeviceXe_Xpm::Initialize(
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

    if (mhwInterfaces->m_veboxInterface == nullptr)
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
        mhwInterfaces->m_veboxInterface) != MOS_STATUS_SUCCESS)
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

MhwInterfaces* MmdDeviceXe_Xpm::CreateMhwInterface(
    PMOS_INTERFACE osInterface)
{
    MhwInterfaces::CreateParams params;
    params.Flags.m_vebox = true;

    // the destroy of interfaces happens when the mmd deviced deconstructor funcs
    MhwInterfaces *mhw = MhwInterfaces::CreateFactory(params, osInterface);

    return mhw;
}
#endif

static bool xehpRegisteredMcpy =
    MediaFactory<uint32_t, McpyDevice>::
    Register<McpyDeviceXe_Xpm>((uint32_t)IGFX_XE_HP_SDV);

MOS_STATUS McpyDeviceXe_Xpm::Initialize(
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

MhwInterfaces* McpyDeviceXe_Xpm::CreateMhwInterface(
    PMOS_INTERFACE osInterface)
{
    MhwInterfaces::CreateParams params;
    params.Flags.m_vebox = true;
    params.Flags.m_blt   = true;

    // the destroy of interfaces happens when the mcpy deviced deconstructor funcs
    MhwInterfaces *mhw = MhwInterfaces::CreateFactory(params, osInterface);

    return mhw;
}

static bool xehpRegisteredNv12ToP010 =
    MediaFactory<uint32_t, Nv12ToP010Device>::
    Register<Nv12ToP010DeviceXe_Xpm>((uint32_t)IGFX_XE_HP_SDV);

MOS_STATUS Nv12ToP010DeviceXe_Xpm::Initialize(
    PMOS_INTERFACE            osInterface)
{
    CODECHAL_PUBLIC_ASSERTMESSAGE("Not support Nv12 to P010 interfaces.")

    return MOS_STATUS_INVALID_PARAMETER;
}

static bool xehpRegisteredCodecHal =
    MediaFactory<uint32_t, CodechalDevice>::
    Register<CodechalInterfacesXe_Xpm>((uint32_t)IGFX_XE_HP_SDV);

MOS_STATUS CodechalInterfacesXe_Xpm::Initialize(
    void *standardInfo,
    void *settings,
    MhwInterfaces *mhwInterfaces,
    PMOS_INTERFACE osInterface)
{
    if (standardInfo == nullptr ||
        mhwInterfaces == nullptr ||
        osInterface == nullptr)
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
    CodechalHwInterface *hwInterface = MOS_New(Hw, osInterface, CodecFunction, mhwInterfaces, disableScalability);
    if (hwInterface == nullptr)
    {
        CODECHAL_PUBLIC_ASSERTMESSAGE("hwInterface is not valid!");
        return MOS_STATUS_NO_SPACE;
    }
    hwInterface->m_hwInterfaceNext                            = MOS_New(CodechalHwInterfaceNext, osInterface);
    if (hwInterface->m_hwInterfaceNext == nullptr)
    {
        MOS_Delete(hwInterface);
        mhwInterfaces->SetDestroyState(true);
        CODECHAL_PUBLIC_ASSERTMESSAGE("hwInterfaceNext is not valid!");
        return MOS_STATUS_NO_SPACE;
    }

    hwInterface->m_hwInterfaceNext->pfnCreateDecodeSinglePipe = decode::DecodeScalabilitySinglePipe::CreateDecodeSinglePipe;
    hwInterface->m_hwInterfaceNext->pfnCreateDecodeMultiPipe  = decode::DecodeScalabilityMultiPipe::CreateDecodeMultiPipe;
    hwInterface->m_hwInterfaceNext->SetMediaSfcInterface(hwInterface->GetMediaSfcInterface());

#if USE_CODECHAL_DEBUG_TOOL
    CodechalDebugInterface *debugInterface = MOS_New(CodechalDebugInterface);
    if (debugInterface == nullptr)
    {
        MOS_Delete(hwInterface);
        mhwInterfaces->SetDestroyState(true);
        CODECHAL_PUBLIC_ASSERTMESSAGE("debugInterface is not valid!");
        return MOS_STATUS_NO_SPACE;
    }
    if (debugInterface->Initialize(hwInterface, CodecFunction) != MOS_STATUS_SUCCESS)
    {
        MOS_Delete(hwInterface);
        mhwInterfaces->SetDestroyState(true);
        MOS_Delete(debugInterface);
        CODECHAL_PUBLIC_ASSERTMESSAGE("Debug interface creation failed!");
        return MOS_STATUS_INVALID_PARAMETER;
    }
#else
    CodechalDebugInterface *debugInterface = nullptr;
#endif // USE_CODECHAL_DEBUG_TOOL

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
        #ifdef _APOGEIOS_SUPPORTED
            bool apogeiosEnable = true;
            MOS_USER_FEATURE_VALUE_DATA         userFeatureData;
            MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));

            userFeatureData.i32Data = apogeiosEnable;
            userFeatureData.i32DataFlag = MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE;
            MOS_UserFeature_ReadValue_ID(
                nullptr,
                __MEDIA_USER_FEATURE_VALUE_APOGEIOS_HEVCD_ENABLE_ID,
                &userFeatureData,
                hwInterface->GetOsInterface()->pOsContext);
            apogeiosEnable = (userFeatureData.i32Data) ? true : false;

            if (apogeiosEnable)
            {
                m_codechalDevice = MOS_New(DecodeHevcPipelineAdapterM12, hwInterface, debugInterface);
            }
            else
        #endif
            {
                m_codechalDevice = MOS_New(Decode::Hevc, hwInterface, debugInterface);
            }
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
#ifdef IGFX_XEHP_SDV_ENABLE_NON_UPSTREAM
    else if (CodecHalIsEncode(CodecFunction))
    {
        CodechalEncoderState *encoder = nullptr;
#if defined (_AVC_ENCODE_VME_SUPPORTED) || defined (_AVC_ENCODE_VDENC_SUPPORTED)
        if (info->Mode == CODECHAL_ENCODE_MODE_AVC)
        {
            if (CodecHalUsesVdencEngine(info->CodecFunction))
            {
            #ifdef _AVC_ENCODE_VDENC_SUPPORTED
                encoder = MOS_New(Encode::AvcVdenc, hwInterface, debugInterface, info);
            #endif
            }
            else
            {
            #ifdef _AVC_ENCODE_VME_SUPPORTED
                encoder = MOS_New(Encode::AvcEnc, hwInterface, debugInterface, info);
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
        }
        else
#endif
#ifdef _VP9_ENCODE_VDENC_SUPPORTED
        if (info->Mode == CODECHAL_ENCODE_MODE_VP9)
        {
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
#ifdef _MPEG2_ENCODE_VME_SUPPORTED
        if (info->Mode == CODECHAL_ENCODE_MODE_MPEG2)
        {
            // Setup encode interface functions
            encoder = MOS_New(Encode::Mpeg2, hwInterface, debugInterface, info);
            if (encoder == nullptr)
            {
                CODECHAL_PUBLIC_ASSERTMESSAGE("Encode allocation failed!");
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
#ifdef _JPEG_ENCODE_SUPPORTED
        if (info->Mode == CODECHAL_ENCODE_MODE_JPEG)
        {
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
                encoder = MOS_New(Encode::HevcVdenc, hwInterface, debugInterface, info);
            #endif
            }
            else
            {

                // disable HEVC encode MDF path.

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
    }
#endif
    else
    {
        CODECHAL_PUBLIC_ASSERTMESSAGE("Unsupported codec function requested.");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    return MOS_STATUS_SUCCESS;
}

#ifdef IGFX_XEHP_SDV_ENABLE_NON_UPSTREAM
static bool xehpRegisteredCMHal =
    MediaFactory<uint32_t, CMHalDevice>::
    Register<CMHalInterfacesXe_Xpm>((uint32_t)IGFX_XE_HP_SDV);

MOS_STATUS CMHalInterfacesXe_Xpm::Initialize(CM_HAL_STATE *pCmState)
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
    device->SetCopyKernelIsa(pGPUCopy_kernel_isa_xe_xpm, iGPUCopy_kernel_isa_size_xe_xpm);
    device->SetInitKernelIsa(pGPUInit_kernel_isa_xe_xpm, iGPUInit_kernel_isa_size_xe_xpm);

    m_cmhalDevice = device;
    m_cmhalDevice->SetGenPlatformInfo(PLATFORM_INTEL_TGL, PLATFORM_INTEL_GT2, "TGL");
    uint32_t cisaIDs[] = {GENX_XEHP};
    m_cmhalDevice->AddSupportedCisaIDs(cisaIDs, sizeof(cisaIDs)/sizeof(uint32_t));
    m_cmhalDevice->SetRedirectRcsToCcs(true);
    m_cmhalDevice->m_l3Plane = XEHP_L3_PLANES;
    m_cmhalDevice->m_l3ConfigCount = XEHP_L3_CONFIG_NUM;
    return MOS_STATUS_SUCCESS;
}
#endif

static bool xehpRegisteredRenderHal =
    MediaFactory<uint32_t, RenderHalDevice>::
    Register<RenderHalInterfacesXe_Xpm>((uint32_t)IGFX_XE_HP_SDV);

MOS_STATUS RenderHalInterfacesXe_Xpm::Initialize()
{
    m_renderhalDevice = MOS_New(XRenderHal);
    if (m_renderhalDevice == nullptr)
    {
        MHW_ASSERTMESSAGE("Create Render Hal interfaces failed.")
        return MOS_STATUS_NO_SPACE;
    }
    return MOS_STATUS_SUCCESS;
}

static bool xehpRegisteredDecodeHistogram =
MediaFactory<uint32_t, DecodeHistogramDevice>::
Register<DecodeHistogramDeviceXe_Xpm>((uint32_t)IGFX_XE_HP_SDV);

MOS_STATUS DecodeHistogramDeviceXe_Xpm::Initialize(
    CodechalHwInterface       *hwInterface,
    PMOS_INTERFACE            osInterface)
{
    // For Gen12+, histogram is rendered inline SFC and just in the same context as decode.
    // Create a bass class instance and will do nothing.
    m_decodeHistogramDevice = MOS_New(DecodeHistogramG12, hwInterface, osInterface);

    if (m_decodeHistogramDevice == nullptr)
    {
        MHW_ASSERTMESSAGE("Create decode histogram  interfaces failed.")
            return MOS_STATUS_NO_SPACE;
    }

    return MOS_STATUS_SUCCESS;
}

#define IP_VERSION_XE_XPM      0x1205
static bool xehpRegisteredHwInfo =
    MediaFactory<uint32_t, MediaInterfacesHwInfoDevice>::Register<MediaInterfacesHwInfoDeviceXe_Xpm>((uint32_t)IGFX_XE_HP_SDV);

MOS_STATUS MediaInterfacesHwInfoDeviceXe_Xpm::Initialize(PLATFORM platform)
{
    m_hwInfo.SetDeviceInfo(IP_VERSION_XE_XPM, platform.usRevId);
    return MOS_STATUS_SUCCESS;
}
