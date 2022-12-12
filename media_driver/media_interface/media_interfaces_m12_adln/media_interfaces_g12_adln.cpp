/*
* Copyright (c) 2011-2022, Intel Corporation
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
//! \file     media_interfaces_g12_adln.cpp

//! \brief    Helps with Gen12 ADL-N factory creation.
//!

#include "media_interfaces_g12_adln.h"

extern template class MediaFactory<uint32_t, MhwInterfaces>;
extern template class MediaFactory<uint32_t, MmdDevice>;
extern template class MediaFactory<uint32_t, CodechalDevice>;
extern template class MediaFactory<uint32_t, CMHalDevice>;
extern template class MediaFactory<uint32_t, VphalDevice>;
extern template class MediaFactory<uint32_t, RenderHalDevice>;
extern template class MediaFactory<uint32_t, Nv12ToP010Device>;
extern template class MediaFactory<uint32_t, DecodeHistogramDevice>;

static bool adlnRegisteredVphal =
MediaFactory<uint32_t, VphalDevice>::
Register<VphalInterfacesG12Tgllp>((uint32_t)IGFX_ALDERLAKE_N);

static bool adlnRegisteredMhw =
    MediaFactory<uint32_t, MhwInterfaces>::
    Register<MhwInterfacesG12Tgllp>((uint32_t)IGFX_ALDERLAKE_N);

#ifdef _MMC_SUPPORTED
static bool adlnRegisteredMmd =
    MediaFactory<uint32_t, MmdDevice>::
    Register<MmdDeviceG12Tgllp>((uint32_t)IGFX_ALDERLAKE_N);
#endif

static bool adlnRegisteredMcpy =
    MediaFactory<uint32_t, McpyDevice>::
    Register<McpyDeviceG12Tgllp>((uint32_t)IGFX_ALDERLAKE_N);

#define PLATFORM_INTEL_ADLN   24
#define GENX_TGLLP            12

static bool adlnRegisteredNv12ToP010 =
    MediaFactory<uint32_t, Nv12ToP010Device>::
    Register<Nv12ToP010DeviceG12Tgllp>((uint32_t)IGFX_ALDERLAKE_N);

static bool adlnRegisteredCodecHal =
    MediaFactory<uint32_t, CodechalDevice>::
    Register<CodechalInterfacesG12Tgllp>((uint32_t)IGFX_ALDERLAKE_N);

static bool adlnRegisteredCMHal =
    MediaFactory<uint32_t, CMHalDevice>::
    Register<CMHalInterfacesG12Adln>((uint32_t)IGFX_ALDERLAKE_N);


MOS_STATUS CMHalInterfacesG12Adln::Initialize(CM_HAL_STATE *pCmState)
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

    m_cmhalDevice->SetGenPlatformInfo(PLATFORM_INTEL_ADLN, PLATFORM_INTEL_GT1, "TGLLP");
    uint32_t cisaIDs[] = { GENX_TGLLP };
    m_cmhalDevice->AddSupportedCisaIDs(cisaIDs, sizeof(cisaIDs)/sizeof(uint32_t));
    m_cmhalDevice->m_l3Plane = TGL_L3_PLANE;
    m_cmhalDevice->m_l3ConfigCount = TGL_L3_CONFIG_NUM;
    return MOS_STATUS_SUCCESS;
}

static bool adlnRegisteredRenderHal =
    MediaFactory<uint32_t, RenderHalDevice>::
    Register<RenderHalInterfacesG12Tgllp>((uint32_t)IGFX_ALDERLAKE_N);

static bool adlnRegisteredDecodeHistogram =
    MediaFactory<uint32_t, DecodeHistogramDevice>::
    Register<DecodeHistogramDeviceG12Tgllp>((uint32_t)IGFX_ALDERLAKE_N);

static bool adlnRegisteredHwInfo =
    MediaFactory<uint32_t, MediaInterfacesHwInfoDevice>::Register<MediaInterfacesHwInfoDeviceG12Tgllp>((uint32_t)IGFX_ALDERLAKE_N);

