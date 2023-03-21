/*
* Copyright (c) 2017-2018, Intel Corporation
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
//! \file     media_interfaces_g9_cfl.cpp
//! \brief    Helps with CFL factory creation.
//!

#include "media_interfaces_g9_kbl.h"
#include "media_interfaces_g9_cfl.h"

extern template class MediaFactory<uint32_t, MhwInterfaces>;
extern template class MediaFactory<uint32_t, MmdDevice>;
extern template class MediaFactory<uint32_t, CodechalDevice>;
extern template class MediaFactory<uint32_t, CMHalDevice>;
extern template class MediaFactory<uint32_t, VphalDevice>;
extern template class MediaFactory<uint32_t, RenderHalDevice>;
extern template class MediaFactory<uint32_t, Nv12ToP010Device>;
extern template class MediaFactory<uint32_t, DecodeHistogramDevice>;

static bool cflRegisteredVphal =
    MediaFactory<uint32_t, VphalDevice>::
    Register<VphalInterfacesG9Kbl>((uint32_t)IGFX_COFFEELAKE);

static bool cflRegisteredMhw =
    MediaFactory<uint32_t, MhwInterfaces>::
    Register<MhwInterfacesG9Kbl>((uint32_t)IGFX_COFFEELAKE);

#if defined(_MMC_SUPPORTED) && defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
static bool cflRegisteredMmd =
    MediaFactory<uint32_t, MmdDevice>::
    Register<MmdDeviceG9Kbl>((uint32_t)IGFX_COFFEELAKE);
#endif

static bool cflRegisteredNv12ToP010 =
    MediaFactory<uint32_t, Nv12ToP010Device>::
    Register<Nv12ToP010DeviceG9Kbl>((uint32_t)IGFX_COFFEELAKE);

static bool cflRegisteredCodecHal =
    MediaFactory<uint32_t, CodechalDevice>::
    Register<CodechalInterfacesG9Kbl>((uint32_t)IGFX_COFFEELAKE);

static bool cflRegisteredCMHal =
    MediaFactory<uint32_t, CMHalDevice>::
    Register<CMHalInterfacesG9Cfl>((uint32_t)IGFX_COFFEELAKE);

#define PLATFORM_INTEL_CFL 17
#define GENX_SKL           5

MOS_STATUS CMHalInterfacesG9Cfl::Initialize(CM_HAL_STATE *pCmState)
{
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

    m_cmhalDevice->SetGenPlatformInfo(PLATFORM_INTEL_CFL, gengt, "SKL");
    uint32_t cisaID = GENX_SKL;
    m_cmhalDevice->AddSupportedCisaIDs(&cisaID);

    CM_HAL_G9_X *pGen9Device = static_cast<CM_HAL_G9_X *>(m_cmhalDevice);
    const char *CmSteppingInfo_CFL[] = {nullptr};
    pGen9Device->OverwriteSteppingTable(CmSteppingInfo_CFL, sizeof(CmSteppingInfo_CFL)/sizeof(const char *));
    return MOS_STATUS_SUCCESS;
}

static bool cflRegisteredRenderHal =
    MediaFactory<uint32_t, RenderHalDevice>::
    Register<RenderHalInterfacesG9Kbl>((uint32_t)IGFX_COFFEELAKE);

static bool cflRegisteredDecodeHistogram =
MediaFactory<uint32_t, DecodeHistogramDevice>::
Register<DecodeHistogramDeviceG9Cfl>((uint32_t)IGFX_COFFEELAKE);

MOS_STATUS DecodeHistogramDeviceG9Cfl::Initialize(
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

#define IP_VERSION_M9_0 0x0900
static bool cflRegisteredHwInfo =
    MediaFactory<uint32_t, MediaInterfacesHwInfoDevice>::Register<MediaInterfacesHwInfoDeviceG9Cfl>((uint32_t)IGFX_COFFEELAKE);

MOS_STATUS MediaInterfacesHwInfoDeviceG9Cfl::Initialize(PLATFORM platform)
{
    m_hwInfo.SetDeviceInfo(IP_VERSION_M9_0, platform.usRevId);
    return MOS_STATUS_SUCCESS;
}
