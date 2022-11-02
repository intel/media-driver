/*
* Copyright (c) 2020, Intel Corporation
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
//! \file     encode_mem_compression.cpp
//! \brief    Defines the common interface for encode mmc
//! \details  The mmc is to handle mmc operations,
//! including compression and decompressin of encode
//!

#include "mos_defs.h"
#include "encode_mem_compression.h"

EncodeMemComp::EncodeMemComp(CodechalHwInterfaceNext *hwInterface) :
    MediaMemComp(hwInterface->GetOsInterface()),
    m_miItf(std::static_pointer_cast<mhw::mi::Itf>(hwInterface->GetMiInterfaceNext()))
{
    m_mmcEnabledKey     = __MEDIA_USER_FEATURE_VALUE_CODEC_MMC_ENABLE;
    m_mmcInUseKey       = __MEDIA_USER_FEATURE_VALUE_CODEC_MMC_IN_USE;

    m_bComponentMmcEnabled = hwInterface->m_enableCodecMmc;

    InitMmcEnabled();
    InitEncodeMmc(hwInterface);
#if (_DEBUG || _RELEASE_INTERNAL)
    m_userFeatureUpdated = false;
#endif
}

#if (_DEBUG || _RELEASE_INTERNAL)
MOS_STATUS EncodeMemComp::UpdateUserFeatureKey(PMOS_SURFACE surface)
{
    if (!surface)
        return MOS_STATUS_NULL_POINTER;

    if (m_userFeatureUpdated)
    {
        return MOS_STATUS_SUCCESS;
    }

    m_userFeatureUpdated = true;

    ReportUserSetting(
        m_userSettingPtr,
        "Encode Recon Compressible",
        surface->bCompressible,
        MediaUserSetting::Group::Sequence);

    ReportUserSetting(
        m_userSettingPtr,
        "Encode Recon Compress Mode",
        surface->MmcState,
        MediaUserSetting::Group::Sequence);

    return MOS_STATUS_SUCCESS;
}
#endif

void EncodeMemComp::InitEncodeMmc(CodechalHwInterfaceNext *hwInterface)
{
    CODEC_HW_ASSERT(hwInterface);
    CODEC_HW_ASSERT(hwInterface->GetSkuTable());
    if (MEDIA_IS_SKU(hwInterface->GetSkuTable(), FtrE2ECompression))
    {
        //read encode mmc if available, then report encode mmc in use
        bool encodeMmcEnabled = true;
        MediaUserSetting::Value outValue;
        ReadUserSetting(
            m_userSettingPtr,
            outValue,
            "Enable Encode MMC",
            MediaUserSetting::Group::Sequence,
            m_osInterface->pOsContext,
            encodeMmcEnabled, true);
        encodeMmcEnabled = outValue.Get<bool>();

        m_mmcEnabledForEncode = m_mmcEnabled && encodeMmcEnabled;

        ReportUserSetting(
            m_userSettingPtr,
            "Encode MMC In Use",
            m_mmcEnabledForEncode,
            MediaUserSetting::Group::Sequence);
    }
}
