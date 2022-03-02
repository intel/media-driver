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

EncodeMemComp::EncodeMemComp(CodechalHwInterface *hwInterface) :
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
    m_compressibleId     = __MEDIA_USER_FEATURE_VALUE_MMC_ENC_RECON_COMPRESSIBLE_ID;
    m_compressModeId     = __MEDIA_USER_FEATURE_VALUE_MMC_ENC_RECON_COMPRESSMODE_ID;
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

void EncodeMemComp::InitEncodeMmc(CodechalHwInterface *hwInterface)
{
    CODECHAL_HW_ASSERT(hwInterface);
    CODECHAL_HW_ASSERT(hwInterface->GetSkuTable());
    if (MEDIA_IS_SKU(hwInterface->GetSkuTable(), FtrE2ECompression))
    {
        //read encode mmc if available, then report encode mmc in use
        bool                        encodeMmcEnabled = true;
        MOS_USER_FEATURE_VALUE_DATA userFeatureData;
        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
        userFeatureData.i32Data     = encodeMmcEnabled;
        userFeatureData.i32DataFlag = MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE;
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_ENCODE_MMC_ENABLE_ID,
            &userFeatureData,
            m_osInterface->pOsContext);
        encodeMmcEnabled = (userFeatureData.i32Data) ? true : false;

        m_mmcEnabledForEncode = m_mmcEnabled && encodeMmcEnabled;

        MOS_USER_FEATURE_VALUE_WRITE_DATA userFeatureWriteData;
        MOS_ZeroMemory(&userFeatureWriteData, sizeof(userFeatureWriteData));
        userFeatureWriteData.Value.i32Data = m_mmcEnabledForEncode;
        userFeatureWriteData.ValueID       = __MEDIA_USER_FEATURE_VALUE_ENCODE_MMC_IN_USE_ID;
        MOS_UserFeature_WriteValues_ID(nullptr, &userFeatureWriteData, 1, m_osInterface->pOsContext);
    }

#if (_DEBUG || _RELEASE_INTERNAL)
    m_compressibleId = __MEDIA_USER_FEATURE_VALUE_MMC_ENC_RECON_COMPRESSIBLE_ID;
    m_compressModeId = __MEDIA_USER_FEATURE_VALUE_MMC_ENC_RECON_COMPRESSMODE_ID;
#endif
}
