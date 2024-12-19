/*
* Copyright (c) 2020-2024, Intel Corporation
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
//! \file     decode_mem_compression.cpp
//! \brief    Defines the common interface for decode mmc
//! \details  The mmc is to handle mmc operations,
//! including compression and decompressin of decode
//!

#include "mos_defs.h"
#include "decode_mem_compression.h"
#include "decode_utils.h"

DecodeMemComp::DecodeMemComp(CodechalHwInterfaceNext *hwInterface, PMOS_INTERFACE osInterface) :
    MediaMemComp(osInterface ? osInterface : hwInterface->GetOsInterface())
{
    m_mmcEnabledKey     = __MEDIA_USER_FEATURE_VALUE_CODEC_MMC_ENABLE;
    m_mmcInUseKey       = __MEDIA_USER_FEATURE_VALUE_CODEC_MMC_IN_USE;
    m_miItf             = hwInterface ? hwInterface->GetMiInterfaceNext() : nullptr;

    if (hwInterface == nullptr)
    {
        CODEC_HW_ASSERT(hwInterface);
        return;
    }
    else
    {
        m_bComponentMmcEnabled = hwInterface->m_enableCodecMmc ? true : false;
    }

    InitMmcEnabled();
    InitDecodeMmc(hwInterface);
#if (_DEBUG || _RELEASE_INTERNAL)
    m_userFeatureUpdated = false;
#endif
}

#if (_DEBUG || _RELEASE_INTERNAL)
MOS_STATUS DecodeMemComp::UpdateUserFeatureKey(PMOS_SURFACE surface)
{
    if (!surface)
        return MOS_STATUS_NULL_POINTER;

    if (m_userFeatureUpdated)
    {
        return MOS_STATUS_SUCCESS;
    }
    m_userFeatureUpdated = true;

    ReportUserSetting(m_userSettingPtr, "Decode RT Compressible", surface->bCompressible, MediaUserSetting::Group::Sequence);
    ReportUserSetting(m_userSettingPtr, "Decode RT Compress Mode", surface->MmcState, MediaUserSetting::Group::Sequence);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeMemComp::ReportSurfaceMmcMode(PMOS_SURFACE surface)
{
    if (!surface)
        return MOS_STATUS_NULL_POINTER;

    MOS_MEMCOMP_STATE mmcMode = MOS_MEMCOMP_DISABLED;
    MediaMemComp::GetSurfaceMmcState(surface, &mmcMode);

    DECODE_NORMALMESSAGE("Decode RT Compress Mode is: %d", mmcMode);

    return MOS_STATUS_SUCCESS;
}
#endif

void DecodeMemComp::InitDecodeMmc(CodechalHwInterfaceNext *hwInterface)
{
    CODEC_HW_ASSERT(hwInterface);
    CODEC_HW_ASSERT(hwInterface->GetSkuTable());
    if (MEDIA_IS_SKU(hwInterface->GetSkuTable(), FtrE2ECompression))
    {
        MediaUserSetting::Value outValue;
        ReadUserSetting(
            m_userSettingPtr,
            outValue,
            "Enable Decode MMC",
            MediaUserSetting::Group::Sequence,
            true,  // Custom value is true as default
            true);
        bool decodeMmcEnabled = outValue.Get<bool>();

        m_mmcEnabledForDecode = m_mmcEnabled && decodeMmcEnabled;

        MOS_USER_FEATURE_VALUE_WRITE_DATA userFeatureWriteData;
        MOS_ZeroMemory(&userFeatureWriteData, sizeof(userFeatureWriteData));
        userFeatureWriteData.Value.i32Data = m_mmcEnabledForDecode;
        userFeatureWriteData.ValueID       = __MEDIA_USER_FEATURE_VALUE_DECODE_MMC_IN_USE_ID;
        MOS_UserFeature_WriteValues_ID(nullptr, &userFeatureWriteData, 1, m_osInterface->pOsContext);
    }

#if (_DEBUG || _RELEASE_INTERNAL)
    m_compressibleId = __MEDIA_USER_FEATURE_VALUE_MMC_DEC_RT_COMPRESSIBLE_ID;
    m_compressModeId = __MEDIA_USER_FEATURE_VALUE_MMC_DEC_RT_COMPRESSMODE_ID;
    if (m_mmcEnabledForDecode)
    {
        DECODE_NORMALMESSAGE("Decode MMC ON ");
    }
#endif
}

bool DecodeMemComp::IsMmcEnabled()
{
    if (!m_mmcEnabledForDecode)
    {
        m_mmcEnabled = false;
    }

    return m_mmcEnabledForDecode;
}
