/*
* Copyright (c) 2017, Intel Corporation
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
//! \file     codechal_mmc.cpp
//! \brief    Impelements the public interface for CodecHal Media Memory Compression
//!

#include "codechal_hw.h"
#include "codechal_mmc.h"

CodecHalMmcState::CodecHalMmcState(CodechalHwInterface  *hwInterface)
{
    CODECHAL_HW_FUNCTION_ENTER;

    CODECHAL_HW_ASSERT(hwInterface);
    m_hwInterface   = hwInterface;
    CODECHAL_HW_ASSERT(hwInterface->GetOsInterface());
    m_osInterface   = hwInterface->GetOsInterface();
    CODECHAL_HW_ASSERT(hwInterface->GetSkuTable());
#ifdef _MMC_SUPPORTED
    m_hcpMmcEnabled = MEDIA_IS_SKU(hwInterface->GetSkuTable(), FtrHcpDecMemoryCompression);
    m_10bitMmcEnabled = MEDIA_IS_SKU(hwInterface->GetSkuTable(), Ftr10bitDecMemoryCompression);
    m_gpuMmuPageFaultEnabled = MEDIA_IS_SKU(hwInterface->GetSkuTable(), FtrGpuMmuPageFault);

    if (MEDIA_IS_SKU(hwInterface->GetSkuTable(), FtrMemoryCompression))
    {
        MOS_USER_FEATURE_VALUE_DATA userFeatureData;
        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));

        // read reg key of Codec MMC enabling. MMC default on.
        userFeatureData.i32Data = true;
        userFeatureData.i32DataFlag = MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE;

        MOS_USER_FEATURE_VALUE_ID valueId = __MEDIA_USER_FEATURE_VALUE_CODEC_MMC_ENABLE_ID;
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            valueId,
            &userFeatureData,
            m_osInterface->pOsContext);
        m_mmcEnabled = (userFeatureData.i32Data) ? true : false;

        // report in-use
        MOS_USER_FEATURE_VALUE_WRITE_DATA userFeatureWriteData;
        MOS_ZeroMemory(&userFeatureWriteData, sizeof(userFeatureWriteData));
        userFeatureWriteData.Value.i32Data = m_mmcEnabled;
        userFeatureWriteData.ValueID = __MEDIA_USER_FEATURE_VALUE_CODEC_MMC_IN_USE_ID;
        MOS_UserFeature_WriteValues_ID(nullptr, &userFeatureWriteData, 1, m_osInterface->pOsContext);
    }
#endif
}
bool CodecHalMmcState::IsMmcEnabled()
{
    CODECHAL_HW_FUNCTION_ENTER;

    return m_mmcEnabled;
}

void CodecHalMmcState::SetMmcDisabled()
{
    CODECHAL_HW_FUNCTION_ENTER;

    m_mmcEnabled = false;
}
MOS_STATUS CodecHalMmcState::GetSurfaceMmcState(PMOS_SURFACE surface)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;

    if (m_mmcEnabled)
    {
        CODECHAL_HW_CHK_STATUS_RETURN(m_osInterface->pfnGetMemoryCompressionMode(
            m_osInterface,
            &surface->OsResource,
            (PMOS_MEMCOMP_STATE)&surface->CompressionMode));
    }

    return eStatus;
}

MOS_STATUS CodecHalMmcState::GetSurfaceMmcState(PMOS_SURFACE surface,MOS_MEMCOMP_STATE *mmcState)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;

    MOS_CHK_NULL_RETURN(MOS_COMPONENT_MMC, MOS_MMC_SUBCOMP_SELF, surface);
    MOS_CHK_NULL_RETURN(MOS_COMPONENT_MMC, MOS_MMC_SUBCOMP_SELF, mmcState);
    MOS_CHK_NULL_RETURN(MOS_COMPONENT_MMC, MOS_MMC_SUBCOMP_SELF, m_osInterface);    

    if (m_mmcEnabled)
    {
        CODECHAL_HW_CHK_STATUS_RETURN(m_osInterface->pfnGetMemoryCompressionMode(
            m_osInterface,
            &surface->OsResource,
            (PMOS_MEMCOMP_STATE)&surface->CompressionMode));
        *mmcState = (MOS_MEMCOMP_STATE)surface->CompressionMode;
    }
    else
    {
        *mmcState = MOS_MEMCOMP_DISABLED;
    }

    return eStatus;
}

 MOS_STATUS CodecHalMmcState::GetSurfaceMmcFormat(
    PMOS_SURFACE surface,
    uint32_t    *mmcFormat)
{
    MOS_STATUS status = MOS_STATUS_SUCCESS;

    MOS_CHK_NULL_RETURN(MOS_COMPONENT_MMC, MOS_MMC_SUBCOMP_SELF, surface);
    MOS_CHK_NULL_RETURN(MOS_COMPONENT_MMC, MOS_MMC_SUBCOMP_SELF, mmcFormat);
    MOS_CHK_NULL_RETURN(MOS_COMPONENT_MMC, MOS_MMC_SUBCOMP_SELF, m_osInterface);

    if (m_mmcEnabled)
    {
        status = m_osInterface->pfnGetMemoryCompressionFormat(m_osInterface, &surface->OsResource, mmcFormat);
    } 
    else
    {
        *mmcFormat = 0;
    }

    return status;
}     
     

MOS_STATUS CodecHalMmcState::
DisableSurfaceMmcState(PMOS_SURFACE surface)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;

    CODECHAL_HW_CHK_STATUS_RETURN(m_osInterface->pfnSetMemoryCompressionMode(
        m_osInterface,
        &surface->OsResource,
        MOS_MEMCOMP_DISABLED));

    return eStatus;
}

MOS_STATUS CodecHalMmcState::SetSurfaceMmcMode(
    PMOS_SURFACE dstSurface,
    PMOS_SURFACE srcSurface)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;

    if (m_mmcEnabled)
    {
        if (srcSurface->CompressionMode == MOS_MMC_DISABLED)
        {
            CODECHAL_HW_CHK_STATUS_RETURN(m_osInterface->pfnSetMemoryCompressionMode(
                m_osInterface,
                &dstSurface->OsResource,
                MOS_MEMCOMP_DISABLED));
            dstSurface->CompressionMode = MOS_MMC_DISABLED;
        }
        else
        {
            if (dstSurface->bCompressible)
            {
                CODECHAL_HW_CHK_STATUS_RETURN(m_osInterface->pfnSetMemoryCompressionMode(
                    m_osInterface,
                    &dstSurface->OsResource,
                    (MOS_MEMCOMP_STATE)srcSurface->CompressionMode));
                dstSurface->CompressionMode = srcSurface->CompressionMode;
            }
            else
            {
                m_osInterface->pfnDecompResource(m_osInterface, &srcSurface->OsResource);
                CODECHAL_HW_CHK_STATUS_RETURN(m_osInterface->pfnSetMemoryCompressionMode(
                    m_osInterface,
                    &dstSurface->OsResource,
                    MOS_MEMCOMP_DISABLED));
                dstSurface->CompressionMode = MOS_MMC_DISABLED;
            }
        }
    }

    return eStatus;
}

MOS_STATUS CodecHalMmcState::SetSurfaceParams(
    PCODECHAL_SURFACE_CODEC_PARAMS surfaceParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;

    if (m_mmcEnabled)
    {
        CODECHAL_HW_CHK_NULL_RETURN(surfaceParams->psSurface);
        CODECHAL_HW_CHK_STATUS_RETURN(m_osInterface->pfnGetMemoryCompressionMode(
            m_osInterface,
            &surfaceParams->psSurface->OsResource,
            (PMOS_MEMCOMP_STATE) &surfaceParams->psSurface->CompressionMode));

        // R8_UNORM is required by MMCD. For 4x downscaling, the input surface could be MMCD-compressed.
        if (surfaceParams->psSurface->CompressionMode != MOS_MMC_DISABLED)
        {
            surfaceParams->bUse32UnormSurfaceFormat = false;
            surfaceParams->bUse16UnormSurfaceFormat = false;
        }
    }

    return eStatus;
}

#if (_DEBUG || _RELEASE_INTERNAL)
MOS_STATUS CodecHalMmcState::UpdateUserFeatureKey(PMOS_SURFACE surface)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;

    CODECHAL_HW_CHK_NULL_RETURN(surface);
#ifdef _MMC_SUPPORTED
    if (!m_userFeatureUpdated)
    {
        MOS_USER_FEATURE_VALUE_WRITE_DATA       userFeatureWriteData;
        userFeatureWriteData = __NULL_USER_FEATURE_VALUE_WRITE_DATA__;
        userFeatureWriteData.Value.i32Data = surface->bCompressible;
        userFeatureWriteData.ValueID = (MOS_USER_FEATURE_VALUE_ID)m_compressibleId;
        MOS_UserFeature_WriteValues_ID(nullptr, &userFeatureWriteData, 1, m_osInterface->pOsContext);

        userFeatureWriteData = __NULL_USER_FEATURE_VALUE_WRITE_DATA__;
        userFeatureWriteData.Value.i32Data = surface->MmcState;
        userFeatureWriteData.ValueID = (MOS_USER_FEATURE_VALUE_ID)m_compressModeId;
        MOS_UserFeature_WriteValues_ID(nullptr, &userFeatureWriteData, 1, m_osInterface->pOsContext);

        m_userFeatureUpdated = true;
    }
#endif
    return eStatus;
}
#endif
