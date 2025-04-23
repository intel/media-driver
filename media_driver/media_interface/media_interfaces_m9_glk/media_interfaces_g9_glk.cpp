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
//! \file     media_interfaces_g9_glk.cpp
//! \brief    Helps with gen-specific factory creation.
//!

#include "media_interfaces_g9_kbl.h"
#include "media_interfaces_g9_glk.h"
#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
#include "igcodeckrn_g9.h"
#endif

extern template class MediaFactory<uint32_t, MhwInterfaces>;
extern template class MediaFactory<uint32_t, MmdDevice>;
extern template class MediaFactory<uint32_t, CodechalDevice>;
extern template class MediaFactory<uint32_t, CMHalDevice>;
extern template class MediaFactory<uint32_t, VphalDevice>;
extern template class MediaFactory<uint32_t, RenderHalDevice>;
extern template class MediaFactory<uint32_t, Nv12ToP010Device>;
extern template class MediaFactory<uint32_t, DecodeHistogramDevice>;

#define PLATFORM_INTEL_GLK 16
#define GENX_SKL           5
#define GENX_BXT           6

static bool glkRegisteredVphal =
    MediaFactory<uint32_t, VphalDevice>::
    Register<VphalInterfacesG9Glk>((uint32_t)IGFX_GEMINILAKE);

MOS_STATUS VphalInterfacesG9Glk::Initialize(
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

static bool glkRegisteredMhw =
    MediaFactory<uint32_t, MhwInterfaces>::
    Register<MhwInterfacesG9Kbl>((uint32_t)IGFX_GEMINILAKE);

#if defined(_MMC_SUPPORTED) && defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
static bool glkRegisteredMmd =
    MediaFactory<uint32_t, MmdDevice>::
    Register<MmdDeviceG9Kbl>((uint32_t)IGFX_GEMINILAKE);
#endif

static bool glkRegisteredNv12ToP010 =
    MediaFactory<uint32_t, Nv12ToP010Device>::
    Register<Nv12ToP010DeviceG9Glk>((uint32_t)IGFX_GEMINILAKE);

MOS_STATUS Nv12ToP010DeviceG9Glk::Initialize(
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

static bool glkRegisteredCodecHal =
    MediaFactory<uint32_t, CodechalDevice>::
    Register<CodechalInterfacesG9Glk>((uint32_t)IGFX_GEMINILAKE);

MOS_STATUS CodechalInterfacesG9Glk::Initialize(
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

            if (m_codechalDevice == nullptr)
            {
                CODECHAL_PUBLIC_ASSERTMESSAGE("Failed to create decode device!");
                return MOS_STATUS_NO_SPACE;
            }
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
            m_codechalDevice = MOS_New(Decode::Hevc, hwInterface, debugInterface, info);
        }
        else
    #endif
    #ifdef _VP9_DECODE_SUPPORTED
        if (info->Mode == CODECHAL_DECODE_MODE_VP9VLD)
        {
            m_codechalDevice = MOS_New(Decode::Vp9, hwInterface, debugInterface, info);
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
            encoder = MOS_New(Encode::HevcEnc, hwInterface, debugInterface, info);
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
#ifdef _VP8_ENCODE_SUPPORTED
        if (info->Mode == CODECHAL_ENCODE_MODE_VP8)
        {
            // Setup encode interface functions
            encoder = MOS_New(Encode::Vp8, hwInterface, debugInterface, info);
            if (encoder == nullptr)
            {
                CODECHAL_PUBLIC_ASSERTMESSAGE("VP8 Encode allocation failed!");
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

static bool glkRegisteredCMHal =
    MediaFactory<uint32_t, CMHalDevice>::
    Register<CMHalInterfacesG9Glk>((uint32_t)IGFX_GEMINILAKE);

MOS_STATUS CMHalInterfacesG9Glk::Initialize(CM_HAL_STATE *pCmState)
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
    if( MEDIA_IS_SKU(pCmState->skuTable, FtrGT1 ))
    {
        gengt = PLATFORM_INTEL_GT1;
    }
    else if (MEDIA_IS_SKU(pCmState->skuTable, FtrGT1_5))
    {
        gengt = PLATFORM_INTEL_GT1_5;
    }
    else if( MEDIA_IS_SKU(pCmState->skuTable, FtrGT2 ))
    {
        gengt = PLATFORM_INTEL_GT2;
    }
    else if( MEDIA_IS_SKU(pCmState->skuTable, FtrGT3 ))
    {
        gengt = PLATFORM_INTEL_GT3;
    }
    else if( MEDIA_IS_SKU(pCmState->skuTable, FtrGT4 ))
    {
        gengt = PLATFORM_INTEL_GT4;
    }

    m_cmhalDevice->SetGenPlatformInfo(PLATFORM_INTEL_GLK, gengt, "BXT");
    uint32_t cisaIDs[] = {GENX_BXT, GENX_SKL};
    m_cmhalDevice->AddSupportedCisaIDs(cisaIDs, 2);

    CM_HAL_G9_X *pGen9Device = static_cast<CM_HAL_G9_X *>(m_cmhalDevice);
    const char *CmSteppingInfo_GLK[] = { "A0", "A1", "n/a", "B0" };
    pGen9Device->OverwriteSteppingTable(CmSteppingInfo_GLK, sizeof(CmSteppingInfo_GLK)/sizeof(const char *));
    return MOS_STATUS_SUCCESS;
}

static bool glkRegisteredRenderHal =
    MediaFactory<uint32_t, RenderHalDevice>::
    Register<RenderHalInterfacesG9Kbl>((uint32_t)IGFX_GEMINILAKE);
    

static bool glkRegisteredDecodeHistogram =
MediaFactory<uint32_t, DecodeHistogramDevice>::
Register<DecodeHistogramDeviceG9Glk>((uint32_t)IGFX_GEMINILAKE);

MOS_STATUS DecodeHistogramDeviceG9Glk::Initialize(
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
