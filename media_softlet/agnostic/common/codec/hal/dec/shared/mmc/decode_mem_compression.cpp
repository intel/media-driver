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
//! \file     decode_mem_compression.cpp
//! \brief    Defines the common interface for decode mmc
//! \details  The mmc is to handle mmc operations,
//! including compression and decompressin of decode
//!

#include "mos_defs.h"
#include "decode_mem_compression.h"

DecodeMemComp::DecodeMemComp(CodechalHwInterface *hwInterface) :
    MediaMemComp(hwInterface->GetOsInterface(), hwInterface->GetMiInterface())
{
    m_mmcFeatureId      = __MEDIA_USER_FEATURE_VALUE_CODEC_MMC_ENABLE_ID;
    m_mmcInuseFeatureId = __MEDIA_USER_FEATURE_VALUE_CODEC_MMC_IN_USE_ID;

    if (hwInterface->m_enableCodecMmc)
    {
        m_bComponentMmcEnabled = true;
    }
    else
    {
        m_bComponentMmcEnabled = false;
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
    m_compressibleId     = __MEDIA_USER_FEATURE_VALUE_MMC_DEC_RT_COMPRESSIBLE_ID;
    m_compressModeId     = __MEDIA_USER_FEATURE_VALUE_MMC_DEC_RT_COMPRESSMODE_ID;
    m_userFeatureUpdated = true;

    MOS_USER_FEATURE_VALUE_WRITE_DATA userFeatureWriteData;
    userFeatureWriteData               = __NULL_USER_FEATURE_VALUE_WRITE_DATA__;
    userFeatureWriteData.Value.i32Data = surface->bCompressible;
    userFeatureWriteData.ValueID       = (MOS_USER_FEATURE_VALUE_ID)m_compressibleId;
    MOS_UserFeature_WriteValues_ID(nullptr, &userFeatureWriteData, 1, m_osInterface->pOsContext);

    userFeatureWriteData               = __NULL_USER_FEATURE_VALUE_WRITE_DATA__;
    userFeatureWriteData.Value.i32Data = surface->MmcState;
    userFeatureWriteData.ValueID       = (MOS_USER_FEATURE_VALUE_ID)m_compressModeId;
    MOS_UserFeature_WriteValues_ID(nullptr, &userFeatureWriteData, 1, m_osInterface->pOsContext);

    return MOS_STATUS_SUCCESS;
}
#endif

void DecodeMemComp::InitDecodeMmc(CodechalHwInterface *hwInterface)
{
    CODECHAL_HW_ASSERT(hwInterface);
    CODECHAL_HW_ASSERT(hwInterface->GetSkuTable());
    if (MEDIA_IS_SKU(hwInterface->GetSkuTable(), FtrE2ECompression))
    {
        bool                        decodeMmcEnabled = true;
        MOS_USER_FEATURE_VALUE_DATA userFeatureData;
        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
        userFeatureData.i32Data     = decodeMmcEnabled;
        userFeatureData.i32DataFlag = MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE;
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_DECODE_MMC_ENABLE_ID,
            &userFeatureData,
            m_osInterface->pOsContext);
        decodeMmcEnabled = (userFeatureData.i32Data) ? true : false;

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
