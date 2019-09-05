/*
* Copyright (c) 2017-2019, Intel Corporation
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
//! \file     media_interfaces_g9_skl.cpp
//! \brief    Helps with SKL factory creation.
//!

#include "media_interfaces_g9_skl.h"
#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
#include "igcodeckrn_g9.h"
#endif

#ifdef _HYBRID_HEVC_DECODE_SUPPORTED
#include "codechal_decode_hybrid_hevc.h"
#endif

#ifdef _HYBRID_VP9_DECODE_SUPPORTED
#include "codechal_decode_hybrid_vp9.h"
#endif

extern template class MediaInterfacesFactory<MhwInterfaces>;
extern template class MediaInterfacesFactory<MmdDevice>;
extern template class MediaInterfacesFactory<CodechalDevice>;
extern template class MediaInterfacesFactory<CMHalDevice>;
extern template class MediaInterfacesFactory<MosUtilDevice>;
extern template class MediaInterfacesFactory<VphalDevice>;
extern template class MediaInterfacesFactory<RenderHalDevice>;
extern template class MediaInterfacesFactory<Nv12ToP010Device>;
extern template class MediaInterfacesFactory<DecodeHistogramDevice>;

static bool sklRegisteredVphal =
    MediaInterfacesFactory<VphalDevice>::
    RegisterHal<VphalInterfacesG9Skl>((uint32_t)IGFX_SKYLAKE);

MOS_STATUS VphalInterfacesG9Skl::Initialize(
    PMOS_INTERFACE  osInterface,
    PMOS_CONTEXT    osDriverContext,
    MOS_STATUS      *eStatus)
{
    m_vphalState = MOS_New(
        VphalState,
        osInterface,
        osDriverContext,
        eStatus);

    return *eStatus;
}

static bool sklRegisteredMhw =
    MediaInterfacesFactory<MhwInterfaces>::
    RegisterHal<MhwInterfacesG9Skl>((uint32_t)IGFX_SKYLAKE);

#define PLATFORM_INTEL_SKL 7
#define GENX_SKL           5

MOS_STATUS MhwInterfacesG9Skl::Initialize(
    CreateParams params,
    PMOS_INTERFACE osInterface)
{
    if (osInterface == nullptr)
    {
        MHW_ASSERTMESSAGE("The OS interface is not valid!");
        return MOS_STATUS_INVALID_PARAMETER;
    }

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
    m_cpInterface = Create_MhwCpInterface(osInterface);
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
    if (params.Flags.m_vdboxAll || params.Flags.m_huc)
    {
        m_hucInterface = MOS_New(Huc, osInterface, m_miInterface, m_cpInterface);
    }
    if (params.Flags.m_vdboxAll || params.Flags.m_vdenc)
    {
        m_vdencInterface = MOS_New(Vdenc, osInterface);
    }

    return MOS_STATUS_SUCCESS;
}
#ifdef _MMC_SUPPORTED
static bool sklRegisteredMmd =
    MediaInterfacesFactory<MmdDevice>::
    RegisterHal<MmdDeviceG9Skl>((uint32_t)IGFX_SKYLAKE);

MOS_STATUS MmdDeviceG9Skl::Initialize(
    PMOS_INTERFACE osInterface,
    MhwInterfaces *mhwInterfaces)
{
#define MMD_FAILURE()                                  \
{                                                           \
    if (device != nullptr)                                  \
    {                                                       \
        MOS_Delete(device);                                 \
    }                                                       \
    return MOS_STATUS_NO_SPACE;                             \
}
    MHW_FUNCTION_ENTER;

    MhwMiInterface          *miInterface = nullptr;
    MhwRenderInterface *renderInterface = nullptr;
    Mmd *device = nullptr;

    if (mhwInterfaces->m_miInterface == nullptr)
    {
        MMD_FAILURE();
    }
    miInterface = mhwInterfaces->m_miInterface;

    if (mhwInterfaces->m_renderInterface == nullptr)
    {
        MMD_FAILURE();
    }
    renderInterface = mhwInterfaces->m_renderInterface;

    device = MOS_New(Mmd);
    
    if (device == nullptr)
    {
        MMD_FAILURE();
    }

    if (device->Initialize(
        osInterface,
        mhwInterfaces->m_cpInterface,
        miInterface,
        renderInterface) != MOS_STATUS_SUCCESS)
    {
        MMD_FAILURE();
    }

    m_mmdDevice = device;

    return MOS_STATUS_SUCCESS;
}
#endif
static bool sklRegisteredNv12ToP010 =
    MediaInterfacesFactory<Nv12ToP010Device>::
    RegisterHal<Nv12ToP010DeviceG9Skl>((uint32_t)IGFX_SKYLAKE);

MOS_STATUS Nv12ToP010DeviceG9Skl::Initialize(
    PMOS_INTERFACE            osInterface)
{
    m_nv12ToP010device = MOS_New(Nv12ToP010, osInterface);

    if (m_nv12ToP010device == nullptr)
    {
        MHW_ASSERTMESSAGE("Create Nv12 to P010 interfaces failed.")
        return MOS_STATUS_NO_SPACE;
    }
    
    return MOS_STATUS_SUCCESS;
}

static bool sklRegisteredCodecHal =
    MediaInterfacesFactory<CodechalDevice>::
    RegisterHal<CodechalInterfacesG9Skl>((uint32_t)IGFX_SKYLAKE);

MOS_STATUS CodechalInterfacesG9Skl::Initialize(
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

    CodechalHwInterface *hwInterface = MOS_New(Hw, osInterface, CodecFunction, mhwInterfaces);

    if (hwInterface == nullptr)
    {
        CODECHAL_PUBLIC_ASSERTMESSAGE("hwInterface is not valid!");
        return MOS_STATUS_NO_SPACE;
    }
#if USE_CODECHAL_DEBUG_TOOL
    CodechalDebugInterface *debugInterface = MOS_New(CodechalDebugInterface);
    if (debugInterface == nullptr)
    {
        CODECHAL_PUBLIC_ASSERTMESSAGE("debugInterface is not valid!");
        return MOS_STATUS_NO_SPACE;
    }
    if (debugInterface->Initialize(hwInterface, CodecFunction) != MOS_STATUS_SUCCESS)
    {
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
            m_codechalDevice = MOS_New(Decode::Mpeg2, hwInterface, debugInterface, info);
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
            m_codechalDevice = MOS_New(Decode::Avc, hwInterface, debugInterface, info);

#ifdef _DECODE_PROCESSING_SUPPORTED
            if (settings != nullptr && ((CodechalSetting *)settings)->downsamplingHinted)
            {
                CodechalDecode *decoder = dynamic_cast<CodechalDecode *>(m_codechalDevice);
                if (decoder == nullptr)
                {
                    CODECHAL_PUBLIC_ASSERTMESSAGE("Failed to create decode device!");
                    return MOS_STATUS_NO_SPACE;
                }
                FieldScalingInterface *fieldScalingInterface =
                    MOS_New(Decode::FieldScaling, hwInterface);
                if (fieldScalingInterface == nullptr)
                {
                    CODECHAL_PUBLIC_ASSERTMESSAGE("Failed to create field scaling interface!");
                    return MOS_STATUS_NO_SPACE;
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
            m_codechalDevice = MOS_New(Decode::Jpeg, hwInterface, debugInterface, info);
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
    #ifdef _HYBRID_HEVC_DECODE_SUPPORTED
            if (info->bIsHybridCodec)
            {
                m_codechalDevice = MOS_New(CodechalDecodeHybridHevcState, hwInterface, debugInterface, info);
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
    #ifdef _HYBRID_VP9_DECODE_SUPPORTED
            if (info->bIsHybridCodec)
            {
                m_codechalDevice = MOS_New(CodechalDecodeHybridVp9State, hwInterface, debugInterface, info);
            }
            else
    #endif
            {
                m_codechalDevice = MOS_New(Decode::Vp9, hwInterface, debugInterface, info);
            }
        }
        else
    #endif
        {
            CODECHAL_PUBLIC_ASSERTMESSAGE("Decode mode requested invalid!");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        if (m_codechalDevice == nullptr)
        {
            CODECHAL_PUBLIC_ASSERTMESSAGE("Decoder device creation failed!");
            return MOS_STATUS_NO_SPACE;
        }
    }
    else if (CodecHalIsEncode(CodecFunction))
    {
        CodechalEncoderState *encoder = nullptr;
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

            encoder->m_kernelBase = (uint8_t*)IGCODECKRN_G9;
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
            encoder->m_needCheckCpEnabled = true;
        }
        else
#endif
#ifdef _HEVC_ENCODE_VME_SUPPORTED
        if (info->Mode == CODECHAL_ENCODE_MODE_HEVC)
        {
            if (CodecHalIsFeiEncode(info->CodecFunction))
            {
                encoder = MOS_New(Encode::HevcFei, hwInterface, debugInterface, info);
            }
            else
            {
                encoder = MOS_New(Encode::HevcEnc, hwInterface, debugInterface, info);
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

            encoder->m_kernelBase = (uint8_t*)IGCODECKRN_G9;
        }
        else
#endif
#if defined (_AVC_ENCODE_VME_SUPPORTED) || defined (_AVC_ENCODE_VDENC_SUPPORTED)
        if (info->Mode == CODECHAL_ENCODE_MODE_AVC)
        {
            if (CodecHalIsFeiEncode(info->CodecFunction))
            {
            #ifdef _AVC_ENCODE_VME_SUPPORTED
                encoder = MOS_New(Encode::AvcFei, hwInterface, debugInterface, info);
            #endif
            }
            else if (CodecHalUsesVdencEngine(info->CodecFunction))
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
        {
            CODECHAL_PUBLIC_ASSERTMESSAGE("Unsupported encode function requested.");
            return MOS_STATUS_INVALID_PARAMETER;
        }
#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
        if (info->Mode != CODECHAL_ENCODE_MODE_JPEG)
        {
            // Create CSC and Downscaling interface
            if ((encoder->m_cscDsState = MOS_New(Encode::CscDs, encoder)) == nullptr)
            {
                return MOS_STATUS_INVALID_PARAMETER;
            }
        }
#endif
    }
    else
    {
        CODECHAL_PUBLIC_ASSERTMESSAGE("Unsupported codec function requested.");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    return MOS_STATUS_SUCCESS;
}

static bool sklRegisteredCMHal =
    MediaInterfacesFactory<CMHalDevice>::
    RegisterHal<CMHalInterfacesG9Skl>((uint32_t)IGFX_SKYLAKE);

MOS_STATUS CMHalInterfacesG9Skl::Initialize(CM_HAL_STATE *pCmState)
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

    int gengt = PLATFORM_INTEL_GT2;
    if (MEDIA_IS_SKU(pCmState->skuTable, FtrGT1))
    {
        gengt = PLATFORM_INTEL_GT1;
    }
    else if (MEDIA_IS_SKU(pCmState->skuTable, FtrGT1_5))
    {
        gengt = PLATFORM_INTEL_GT1_5;
    }
    else if (MEDIA_IS_SKU(pCmState->skuTable, FtrGT2))
    {
        gengt = PLATFORM_INTEL_GT2;
    }
    else if (MEDIA_IS_SKU(pCmState->skuTable, FtrGT3))
    {
        gengt = PLATFORM_INTEL_GT3;
    }
    else if (MEDIA_IS_SKU(pCmState->skuTable, FtrGT4))
    {
        gengt = PLATFORM_INTEL_GT4;
    }

    m_cmhalDevice->SetGenPlatformInfo(PLATFORM_INTEL_SKL, gengt, "SKL");
    uint32_t cisaID = GENX_SKL;
    m_cmhalDevice->AddSupportedCisaIDs(&cisaID);
    return MOS_STATUS_SUCCESS;
}

static bool sklRegisteredMosUtil =
    MediaInterfacesFactory<MosUtilDevice>::
    RegisterHal<MosUtilDeviceG9Skl>((uint32_t)IGFX_SKYLAKE);

MOS_STATUS MosUtilDeviceG9Skl::Initialize()
{
#define MOSUTIL_FAILURE()                                   \
{                                                           \
    if (device != nullptr)                                  \
    {                                                       \
        delete device;                                      \
    }                                                       \
    return MOS_STATUS_NO_SPACE;                             \
}

    MosUtil *device = nullptr;

    device = MOS_New(MosUtil);
    
    if (device == nullptr)
    {
        MOSUTIL_FAILURE();
    }

    if (device->Initialize() != MOS_STATUS_SUCCESS)
    {
        MOSUTIL_FAILURE();
    }

    m_mosUtilDevice = device;

    return MOS_STATUS_SUCCESS;
}

static bool sklRegisteredRenderHal =
    MediaInterfacesFactory<RenderHalDevice>::
    RegisterHal<RenderHalInterfacesG9Skl>((uint32_t)IGFX_SKYLAKE);

MOS_STATUS RenderHalInterfacesG9Skl::Initialize()
{
    m_renderhalDevice = MOS_New(XRenderHal);
    if (m_renderhalDevice == nullptr)
    {
        MHW_ASSERTMESSAGE("Create Render Hal interfaces failed.")
        return MOS_STATUS_NO_SPACE;
    }
    return MOS_STATUS_SUCCESS;
}

static bool sklRegisteredDecodeHistogram =
MediaInterfacesFactory<DecodeHistogramDevice>::
RegisterHal<DecodeHistogramDeviceG9Skl>((uint32_t)IGFX_SKYLAKE);

MOS_STATUS DecodeHistogramDeviceG9Skl::Initialize(
    CodechalHwInterface       *hwInterface,
    PMOS_INTERFACE            osInterface)
{
    m_decodeHistogramDevice = MOS_New(DecodeHistogramVebox, hwInterface, osInterface);

    if (m_decodeHistogramDevice == nullptr)
    {
        MHW_ASSERTMESSAGE("Create vebox decode histogram  interfaces failed.")
            return MOS_STATUS_NO_SPACE;
    }

    return MOS_STATUS_SUCCESS;
}

