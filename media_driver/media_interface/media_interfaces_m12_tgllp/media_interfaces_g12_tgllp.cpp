/*
* Copyright (c) 2017-2024, Intel Corporation
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
//! \file     media_interfaces_g12_tgllp.cpp
//! \brief    Helps with TGL factory creation.
//!

#include "media_interfaces_g12_tgllp.h"
#include "codechal_debug.h"
#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
#include "igcodeckrn_g12.h"
#endif
#include "vp_feature_manager.h"
#include "vp_platform_interface_g12_tgllp.h"

extern template class MediaFactory<uint32_t, MhwInterfaces>;
extern template class MediaFactory<uint32_t, MmdDevice>;
extern template class MediaFactory<uint32_t, McpyDevice>;
extern template class MediaFactory<uint32_t, CodechalDevice>;
extern template class MediaFactory<uint32_t, CMHalDevice>;
extern template class MediaFactory<uint32_t, VphalDevice>;
extern template class MediaFactory<uint32_t, RenderHalDevice>;
extern template class MediaFactory<uint32_t, Nv12ToP010Device>;
extern template class MediaFactory<uint32_t, DecodeHistogramDevice>;

static bool tgllpRegisteredVphal =
MediaFactory<uint32_t, VphalDevice>::
Register<VphalInterfacesG12Tgllp>((uint32_t)IGFX_TIGERLAKE_LP);

MOS_STATUS VphalInterfacesG12Tgllp::Initialize(
    PMOS_INTERFACE  osInterface,
    bool            bInitVphalState,
    MOS_STATUS      *eStatus,
    bool            clearViewMode)
{
    MOS_OS_CHK_NULL_RETURN(eStatus);
    MOS_OS_CHK_NULL_RETURN(osInterface);
#if LINUX
    bool bApogeiosEnable = true;
    MOS_USER_FEATURE_VALUE_DATA         UserFeatureData;
    MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));

    UserFeatureData.i32Data = bApogeiosEnable;
    UserFeatureData.i32DataFlag = MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE;

    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_APOGEIOS_ENABLE_ID,
        &UserFeatureData,
        osInterface ? osInterface->pOsContext : nullptr);
    bApogeiosEnable = UserFeatureData.bData ? true : false;
    if (bApogeiosEnable)
    {
        vp::VpPlatformInterface *vpPlatformInterface = MOS_New(vp::VpPlatformInterfaceG12Tgllp, osInterface);
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
            VpPipelineG12Adapter,
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
#endif
    {
        m_vpBase = MOS_New(
        VphalState,
        osInterface,
        eStatus);
    }

    return *eStatus;
}

MOS_STATUS VphalInterfacesG12Tgllp::CreateVpPlatformInterface(
    PMOS_INTERFACE osInterface,
    MOS_STATUS *   eStatus)
{
    vp::VpPlatformInterface *vpPlatformInterface = MOS_New(vp::VpPlatformInterfaceG12Tgllp, osInterface);
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

static bool tgllpRegisteredMhw =
    MediaFactory<uint32_t, MhwInterfaces>::
    Register<MhwInterfacesG12Tgllp>((uint32_t)IGFX_TIGERLAKE_LP);

#define PLATFORM_INTEL_TGLLP 15
#define GENX_TGLLP           12

MOS_STATUS MhwInterfacesG12Tgllp::Initialize(
    CreateParams params,
    PMOS_INTERFACE osInterface)
{
#ifdef IGFX_VDENC_INTERFACE_EXT_SUPPORT
    bool useBaseVdencInterface = false;
#if (_DEBUG || _RELEASE_INTERNAL)
    MOS_USER_FEATURE_VALUE_DATA     UserFeatureData;
    MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_MHW_BASE_VDENC_INTERFACE_ID,
        &UserFeatureData,
        osInterface ? osInterface->pOsContext : nullptr);
    useBaseVdencInterface = (UserFeatureData.i32Data == 1) ? true : false;
#endif
#endif
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
    MOS_STATUS status = MOS_STATUS_SUCCESS;
    m_cpInterface = osInterface->pfnCreateMhwCpInterface(osInterface);
    if(m_cpInterface == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("new osInterface failed");
        status = MOS_STATUS_NULL_POINTER;
        goto finish;
    }
    m_miInterface = MOS_New(Mi, m_cpInterface, osInterface);
    if (m_miInterface == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("new MI interface failed");
        status = MOS_STATUS_NULL_POINTER;
        goto finish;
    }

    if (params.Flags.m_render)
    {
        m_renderInterface =
            MOS_New(Render, m_miInterface, osInterface, gtSystemInfo, params.m_heapMode);
        if (m_renderInterface == nullptr  || m_renderInterface->m_stateHeapInterface == nullptr)
        {
            MOS_OS_ASSERTMESSAGE("new m_renderInterface failed");
            status = MOS_STATUS_NULL_POINTER;
            goto finish;
        }
    }
    if (params.Flags.m_stateHeap)
    {
        m_stateHeapInterface =
            MOS_New(StateHeap, osInterface, params.m_heapMode);
        if (m_stateHeapInterface == nullptr)
        {
            MOS_OS_ASSERTMESSAGE("new m_stateHeapInterface failed");
            status = MOS_STATUS_NULL_POINTER;
            goto finish;
        }
    }
    if (params.Flags.m_sfc)
    {
        m_sfcInterface = MOS_New(Sfc, osInterface);
        if (m_sfcInterface == nullptr)
        {
            MOS_OS_ASSERTMESSAGE("new m_sfcInterface failed");
            status = MOS_STATUS_NULL_POINTER;
            goto finish;
        }
    }
    if (params.Flags.m_vebox)
    {
        m_veboxInterface = MOS_New(Vebox, osInterface);
        if (m_veboxInterface == nullptr)
        {
            MOS_OS_ASSERTMESSAGE("new m_veboxInterface failed");
            status = MOS_STATUS_NULL_POINTER;
            goto finish;
        }
    }

    if (params.Flags.m_vdboxAll || params.Flags.m_mfx)
    {
        m_mfxInterface =
            MOS_New(Mfx, osInterface, m_miInterface, m_cpInterface, params.m_isDecode);
        if (m_mfxInterface == nullptr)
        {
            MOS_OS_ASSERTMESSAGE("new m_mfxInterface failed");
            status = MOS_STATUS_NULL_POINTER;
            goto finish;
        }
    }
    if (params.Flags.m_vdboxAll || params.Flags.m_hcp)
    {
        m_hcpInterface =
            MOS_New(Hcp, osInterface, m_miInterface, m_cpInterface, params.m_isDecode);
        if (m_hcpInterface == nullptr)
        {
            MOS_OS_ASSERTMESSAGE("new m_hcpInterface failed");
            status = MOS_STATUS_NULL_POINTER;
            goto finish;
        }
    }
    if (params.Flags.m_vdboxAll || params.Flags.m_avp)
    {
        m_avpInterface =
            MOS_New(Avp, osInterface, m_miInterface, m_cpInterface, params.m_isDecode);
        if (m_avpInterface == nullptr)
        {
            MOS_OS_ASSERTMESSAGE("new m_avpInterface failed");
            status = MOS_STATUS_NULL_POINTER;
            goto finish;
        }
    }
    if (params.Flags.m_vdboxAll || params.Flags.m_huc)
    {
        m_hucInterface = MOS_New(Huc, osInterface, m_miInterface, m_cpInterface);
        if (m_hucInterface == nullptr)
        {
            MOS_OS_ASSERTMESSAGE("new m_hucInterface failed");
            status = MOS_STATUS_NULL_POINTER;
            goto finish;
        }
    }
    if (params.Flags.m_vdboxAll || params.Flags.m_vdenc)
    {
#ifdef IGFX_VDENC_INTERFACE_EXT_SUPPORT
#if (_DEBUG || _RELEASE_INTERNAL)
        if(useBaseVdencInterface)
        {
            m_vdencInterface = MOS_New(MhwVdboxVdencInterfaceG12X, osInterface);
            if (m_vdencInterface == nullptr)
            {
                MOS_OS_ASSERTMESSAGE("new m_vdencInterface failed");
                status = MOS_STATUS_NULL_POINTER;
                goto finish;
            }
            goto finish;
        }
#endif
#endif
        m_vdencInterface = MOS_New(Vdenc, osInterface);
        if (m_vdencInterface == nullptr)
        {
            MOS_OS_ASSERTMESSAGE("new m_vdencInterface failed");
            status = MOS_STATUS_NULL_POINTER;
            goto finish;
        }
    }

    if (params.Flags.m_blt)
     {
        m_bltInterface = MOS_New(Blt, osInterface);
        if (m_bltInterface == nullptr)
        {
            MOS_OS_ASSERTMESSAGE("new m_bltInterface failed");
            status = MOS_STATUS_NULL_POINTER;
            goto finish;
        }
 }
finish:
    if (status != MOS_STATUS_SUCCESS)
    {
        Destroy();
    }
    return status;
}

void MhwInterfacesG12Tgllp::Destroy()
{
    MhwInterfaces::Destroy();

    bool isMhwDestroyed = GetDestroyState();
    if (!isMhwDestroyed)
    {
        MOS_Delete(m_avpInterface);
    }
}

#ifdef _MMC_SUPPORTED
static bool tgllpRegisteredMmd =
    MediaFactory<uint32_t, MmdDevice>::
    Register<MmdDeviceG12Tgllp>((uint32_t)IGFX_TIGERLAKE_LP);

MOS_STATUS MmdDeviceG12Tgllp::Initialize(
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

MhwInterfaces* MmdDeviceG12Tgllp::CreateMhwInterface(
    PMOS_INTERFACE osInterface)
{
    MhwInterfaces::CreateParams params;
    params.Flags.m_vebox = true;

    // the destroy of interfaces happens when the mmd deviced deconstructor funcs
    MhwInterfaces *mhw = MhwInterfaces::CreateFactory(params, osInterface);

    return mhw;
}
#endif

static bool tgllpRegisteredMcpy =
    MediaFactory<uint32_t, McpyDevice>::
    Register<McpyDeviceG12Tgllp>((uint32_t)IGFX_TIGERLAKE_LP);

MOS_STATUS McpyDeviceG12Tgllp::Initialize(
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

MhwInterfaces* McpyDeviceG12Tgllp::CreateMhwInterface(
    PMOS_INTERFACE osInterface)
{
    MhwInterfaces::CreateParams params;
    params.Flags.m_vebox  = true;
    params.Flags.m_blt    = true;

    // the destroy of interfaces happens when the mcpy deviced deconstructor funcs
    MhwInterfaces *mhw = MhwInterfaces::CreateFactory(params, osInterface);

    return mhw;
}

static bool tgllpRegisteredNv12ToP010 =
    MediaFactory<uint32_t, Nv12ToP010Device>::
    Register<Nv12ToP010DeviceG12Tgllp>((uint32_t)IGFX_TIGERLAKE_LP);

MOS_STATUS Nv12ToP010DeviceG12Tgllp::Initialize(
    PMOS_INTERFACE            osInterface)
{
    CODECHAL_PUBLIC_ASSERTMESSAGE("Not support Nv12 to P010 interfaces.")
    
    return MOS_STATUS_INVALID_PARAMETER;
}

static bool tglRegisteredCodecHal =
    MediaFactory<uint32_t, CodechalDevice>::
    Register<CodechalInterfacesG12Tgllp>((uint32_t)IGFX_TIGERLAKE_LP);

MOS_STATUS CodechalInterfacesG12Tgllp::Initialize(
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
    if(MEDIA_IS_SKU(osInterface->pfnGetSkuTable(osInterface), FtrLocalMemory))
    {
        PLATFORM platform = {};
        osInterface->pfnGetPlatform(osInterface, &platform);
        if((platform.usDeviceID != 0x4905)
            && (platform.usDeviceID != 0x4906)
            && (platform.usDeviceID != 0x4908))
        {
            disableScalability = true;
        }
    }
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
        #ifdef _APOGEIOS_SUPPORTED
            bool apogeiosEnable = false;
            MOS_USER_FEATURE_VALUE_DATA         userFeatureData;
            MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));

            userFeatureData.i32Data = apogeiosEnable;
            userFeatureData.i32DataFlag = MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE;
            MOS_UserFeature_ReadValue_ID(
                nullptr,
                __MEDIA_USER_FEATURE_VALUE_APOGEIOS_MPEG2D_ENABLE_ID,
                &userFeatureData,
                hwInterface->GetOsInterface()->pOsContext);
            apogeiosEnable = userFeatureData.bData ? true : false;

            if (apogeiosEnable)
            {
                m_codechalDevice = MOS_New(DecodeMpeg2PipelineAdapterM12, hwInterface, debugInterface);
            }
            else
        #endif
            {
                m_codechalDevice = MOS_New(Decode::Mpeg2, hwInterface, debugInterface, info);
            }
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
            bool apogeiosEnable = false;
        #ifdef _APOGEIOS_SUPPORTED
            MOS_USER_FEATURE_VALUE_DATA         userFeatureData;
            MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));

            userFeatureData.i32Data = apogeiosEnable;
            userFeatureData.i32DataFlag = MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE;
            MOS_UserFeature_ReadValue_ID(
                nullptr,
                __MEDIA_USER_FEATURE_VALUE_APOGEIOS_AVCD_ENABLE_ID,
                &userFeatureData,
                hwInterface->GetOsInterface()->pOsContext);
            apogeiosEnable = userFeatureData.bData ? true : false;

            if (apogeiosEnable)
            {
                m_codechalDevice = MOS_New(DecodeAvcPipelineAdapterM12, hwInterface, debugInterface);
            }
            else
        #endif
            {
                m_codechalDevice = MOS_New(Decode::Avc, hwInterface, debugInterface, info);
            }

            if (m_codechalDevice == nullptr)
            {
                CODECHAL_PUBLIC_ASSERTMESSAGE("Failed to create decode device!");
                CODECHAL_PUBLIC_CHK_STATUS_WITH_DESTROY_RETURN(MOS_STATUS_NO_SPACE, release_func);
            }
 #ifdef _DECODE_PROCESSING_SUPPORTED

#if defined(ENABLE_KERNELS) && defined(_FULL_OPEN_SOURCE)
    ((CodechalSetting *)settings)->downsamplingHinted = false;
#endif

            if (settings != nullptr && ((CodechalSetting *)settings)->downsamplingHinted && !apogeiosEnable)
            {
                CodechalDecode *decoder = dynamic_cast<CodechalDecode *>(m_codechalDevice);
                if (decoder == nullptr)
                {
                    CODECHAL_PUBLIC_ASSERTMESSAGE("Failed to create decode device!");
                    CODECHAL_PUBLIC_CHK_STATUS_WITH_DESTROY_RETURN(MOS_STATUS_NO_SPACE, release_func);
                }
                FieldScalingInterface *fieldScalingInterface =
                    MOS_New(Decode::FieldScaling, hwInterface);
                if (fieldScalingInterface == nullptr)
                {
                    CODECHAL_PUBLIC_ASSERTMESSAGE("Failed to create field scaling interface!");
                    CODECHAL_PUBLIC_CHK_STATUS_WITH_DESTROY_RETURN(MOS_STATUS_NO_SPACE, release_func);
                }
                decoder->m_fieldScalingInterface = fieldScalingInterface;
            }
#endif
        }
        else
    #endif
    #ifdef _JPEG_DECODE_SUPPORTED
        if (info->Mode == CODECHAL_DECODE_MODE_JPEG)
        {
            bool apogeiosEnable = false;
#ifdef _APOGEIOS_SUPPORTED
            MOS_USER_FEATURE_VALUE_DATA userFeatureData;
            MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));

            userFeatureData.i32Data     = apogeiosEnable;
            userFeatureData.i32DataFlag = MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE;
            MOS_UserFeature_ReadValue_ID(
                nullptr,
                __MEDIA_USER_FEATURE_VALUE_APOGEIOS_JPEGD_ENABLE_ID,
                &userFeatureData,
                hwInterface->GetOsInterface()->pOsContext);
            apogeiosEnable = userFeatureData.bData ? true : false;

            if (apogeiosEnable)
            {
                m_codechalDevice = MOS_New(DecodeJpegPipelineAdapterM12, hwInterface, debugInterface);
            }
            else
#endif
            {
                m_codechalDevice = MOS_New(Decode::Jpeg, hwInterface, debugInterface, info);
            }
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
            bool apogeiosEnable = false;
            MOS_USER_FEATURE_VALUE_DATA         userFeatureData;
            MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));

            userFeatureData.i32Data = apogeiosEnable;
            userFeatureData.i32DataFlag = MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE;
            MOS_UserFeature_ReadValue_ID(
                nullptr,
                __MEDIA_USER_FEATURE_VALUE_APOGEIOS_HEVCD_ENABLE_ID,
                &userFeatureData,
                hwInterface->GetOsInterface()->pOsContext);
            apogeiosEnable = userFeatureData.bData ? true : false;

            if (apogeiosEnable)
            {
                m_codechalDevice = MOS_New(DecodeHevcPipelineAdapterM12, hwInterface, debugInterface);
            }
            else
            #endif
            {
                m_codechalDevice = MOS_New(Decode::Hevc, hwInterface, debugInterface, info);
            }
        }
        else
    #endif
    #ifdef _VP9_DECODE_SUPPORTED
        if (info->Mode == CODECHAL_DECODE_MODE_VP9VLD)
        {
#ifdef _APOGEIOS_SUPPORTED
            bool                        apogeiosEnable = false;
            MOS_USER_FEATURE_VALUE_DATA userFeatureData;
            MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));

            userFeatureData.i32Data     = apogeiosEnable;
            userFeatureData.i32DataFlag = MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE;
            MOS_UserFeature_ReadValue_ID(
                nullptr,
                __MEDIA_USER_FEATURE_VALUE_APOGEIOS_VP9D_ENABLE_ID,
                &userFeatureData,
                hwInterface->GetOsInterface()->pOsContext);
        
             apogeiosEnable = userFeatureData.bData ? true : false;

            if (apogeiosEnable)
            {
                m_codechalDevice = MOS_New(DecodeVp9PipelineAdapterG12, hwInterface, debugInterface);
            }
            else
#endif
            {
                m_codechalDevice = MOS_New(Decode::Vp9, hwInterface, debugInterface, info);
            }
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
        CodechalEncoderState *encoder = nullptr;
        bool mdfSupported = true;
#if (_DEBUG || _RELEASE_INTERNAL)
        MOS_USER_FEATURE_VALUE_DATA     UserFeatureData;
        MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_MDF_DISABLE_ID,
            &UserFeatureData,
            hwInterface->GetOsInterface()->pOsContext);
        mdfSupported = (UserFeatureData.i32Data == 1) ? false : true;
#endif // (_DEBUG || _RELEASE_INTERNAL)
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
                CODECHAL_PUBLIC_CHK_STATUS_WITH_DESTROY_RETURN(MOS_STATUS_INVALID_PARAMETER, release_func);
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
                CODECHAL_PUBLIC_CHK_STATUS_WITH_DESTROY_RETURN(MOS_STATUS_INVALID_PARAMETER, release_func);
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
                CODECHAL_PUBLIC_CHK_STATUS_WITH_DESTROY_RETURN(MOS_STATUS_INVALID_PARAMETER, release_func);
            }
            else
            {
                m_codechalDevice = encoder;
            }
#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
            encoder->m_kernelBase = (uint8_t*)IGCODECKRN_G12;
#endif
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
                CODECHAL_PUBLIC_CHK_STATUS_WITH_DESTROY_RETURN(MOS_STATUS_INVALID_PARAMETER, release_func);
            }
            else
            {
                m_codechalDevice = encoder;
            }
#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
            encoder->m_kernelBase = (uint8_t*)IGCODECKRN_G12;
#endif
        }
        else
#endif
        {
            CODECHAL_PUBLIC_ASSERTMESSAGE("Unsupported encode function requested.");
            CODECHAL_PUBLIC_CHK_STATUS_WITH_DESTROY_RETURN(MOS_STATUS_INVALID_PARAMETER, release_func);
        }
#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
        if (info->Mode != CODECHAL_ENCODE_MODE_JPEG)
        {
            // use MDF RT to program CSC kernel for HEVC dual pipe 
            if (((mdfSupported && info->Mode == CODECHAL_ENCODE_MODE_HEVC)) && !CodecHalUsesVdencEngine(info->CodecFunction))
            {
                if ((encoder->m_cscDsState = MOS_New(Encode::CscDsMdf, encoder)) == nullptr)
                {
                    CODECHAL_PUBLIC_CHK_STATUS_WITH_DESTROY_RETURN(MOS_STATUS_INVALID_PARAMETER, release_func);
                }
            }
            else
            {
                // Create CSC and Downscaling interface
                if ((encoder->m_cscDsState = MOS_New(Encode::CscDs, encoder)) == nullptr)
                {
                    CODECHAL_PUBLIC_CHK_STATUS_WITH_DESTROY_RETURN(MOS_STATUS_INVALID_PARAMETER, release_func);
                }
            }
        }
#endif
    }
    else
    {
        CODECHAL_PUBLIC_ASSERTMESSAGE("Unsupported codec function requested.");
        CODECHAL_PUBLIC_CHK_STATUS_WITH_DESTROY_RETURN(MOS_STATUS_INVALID_PARAMETER, release_func);
    }

    return MOS_STATUS_SUCCESS;
}

static bool tgllpRegisteredCMHal =
    MediaFactory<uint32_t, CMHalDevice>::
    Register<CMHalInterfacesG12Tgllp>((uint32_t)IGFX_TIGERLAKE_LP);

MOS_STATUS CMHalInterfacesG12Tgllp::Initialize(CM_HAL_STATE *pCmState)
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

    m_cmhalDevice->SetGenPlatformInfo(PLATFORM_INTEL_TGLLP, PLATFORM_INTEL_GT2, "TGLLP");
    uint32_t cisaIDs[] = {GENX_TGLLP};
    m_cmhalDevice->AddSupportedCisaIDs(cisaIDs, sizeof(cisaIDs)/sizeof(uint32_t));
    m_cmhalDevice->m_l3Plane = TGL_L3_PLANE;
    m_cmhalDevice->m_l3ConfigCount = TGL_L3_CONFIG_NUM;
    return MOS_STATUS_SUCCESS;
}

static bool tgllpRegisteredRenderHal =
    MediaFactory<uint32_t, RenderHalDevice>::
    Register<RenderHalInterfacesG12Tgllp>((uint32_t)IGFX_TIGERLAKE_LP);

MOS_STATUS RenderHalInterfacesG12Tgllp::Initialize()
{
    m_renderhalDevice = MOS_New(XRenderHal);
    if (m_renderhalDevice == nullptr)
    {
        MHW_ASSERTMESSAGE("Create Render Hal interfaces failed.")
        return MOS_STATUS_NO_SPACE;
    }
    return MOS_STATUS_SUCCESS;
}

static bool tgllpRegisteredDecodeHistogram =
MediaFactory<uint32_t, DecodeHistogramDevice>::
Register<DecodeHistogramDeviceG12Tgllp>((uint32_t)IGFX_TIGERLAKE_LP);

MOS_STATUS DecodeHistogramDeviceG12Tgllp::Initialize(
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

#define IP_VERSION_M12_0       0x1200
static bool tglRegisteredHwInfo =
    MediaFactory<uint32_t, MediaInterfacesHwInfoDevice>::Register<MediaInterfacesHwInfoDeviceG12Tgllp>((uint32_t)IGFX_TIGERLAKE_LP);
MOS_STATUS MediaInterfacesHwInfoDeviceG12Tgllp::Initialize(PLATFORM platform)
{
    m_hwInfo.SetDeviceInfo(IP_VERSION_M12_0, platform.usRevId);
    return MOS_STATUS_SUCCESS;
}