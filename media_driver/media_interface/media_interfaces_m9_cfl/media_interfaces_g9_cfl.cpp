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

extern template class MediaInterfacesFactory<MhwInterfaces>;
extern template class MediaInterfacesFactory<MmdDevice>;
extern template class MediaInterfacesFactory<CodechalDevice>;
extern template class MediaInterfacesFactory<CMHalDevice>;
extern template class MediaInterfacesFactory<MosUtilDevice>;
extern template class MediaInterfacesFactory<VphalDevice>;
extern template class MediaInterfacesFactory<RenderHalDevice>;
extern template class MediaInterfacesFactory<Nv12ToP010Device>;
extern template class MediaInterfacesFactory<DecodeHistogramDevice>;

static bool cflRegisteredVphal =
    MediaInterfacesFactory<VphalDevice>::
    RegisterHal<VphalInterfacesG9Kbl>((uint32_t)IGFX_COFFEELAKE);

static bool cflRegisteredMhw =
    MediaInterfacesFactory<MhwInterfaces>::
    RegisterHal<MhwInterfacesG9Kbl>((uint32_t)IGFX_COFFEELAKE);

#ifdef _MMC_SUPPORTED
static bool cflRegisteredMmd =
    MediaInterfacesFactory<MmdDevice>::
    RegisterHal<MmdDeviceG9Kbl>((uint32_t)IGFX_COFFEELAKE);
#endif

static bool cflRegisteredNv12ToP010 =
    MediaInterfacesFactory<Nv12ToP010Device>::
    RegisterHal<Nv12ToP010DeviceG9Kbl>((uint32_t)IGFX_COFFEELAKE);

static bool cflRegisteredCodecHal =
    MediaInterfacesFactory<CodechalDevice>::
    RegisterHal<CodechalInterfacesG9Kbl>((uint32_t)IGFX_COFFEELAKE);

static bool cflRegisteredCMHal =
    MediaInterfacesFactory<CMHalDevice>::
    RegisterHal<CMHalInterfacesG9Cfl>((uint32_t)IGFX_COFFEELAKE);

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

static bool cflRegisteredMosUtil =
    MediaInterfacesFactory<MosUtilDevice>::
    RegisterHal<MosUtilDeviceG9Kbl>((uint32_t)IGFX_COFFEELAKE);

static bool cflRegisteredRenderHal =
    MediaInterfacesFactory<RenderHalDevice>::
    RegisterHal<RenderHalInterfacesG9Kbl>((uint32_t)IGFX_COFFEELAKE);

static bool cflRegisteredDecodeHistogram =
MediaInterfacesFactory<DecodeHistogramDevice>::
RegisterHal<DecodeHistogramDeviceG9Cfl>((uint32_t)IGFX_COFFEELAKE);

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
    MediaInterfacesFactory<MediaInterfacesHwInfoDevice>::RegisterHal<MediaInterfacesHwInfoDeviceG9Cfl>((uint32_t)IGFX_COFFEELAKE);

MOS_STATUS MediaInterfacesHwInfoDeviceG9Cfl::Initialize(PLATFORM platform)
{
    m_hwInfo.SetDeviceInfo(IP_VERSION_M9_0, platform.usRevId);
    return MOS_STATUS_SUCCESS;
}
