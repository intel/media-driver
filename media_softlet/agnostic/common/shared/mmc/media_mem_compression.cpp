/*
* Copyright (c) 2018-2022, Intel Corporation
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
//! \file     media_mem_compression.cpp
//! \brief    Defines the common interface for media memory compression
//! \details  The mmc is to handle mmc operations,
//!

#include "media_mem_compression.h"
#include "mos_interface.h"

MediaMemComp::MediaMemComp(PMOS_INTERFACE osInterface) :
    m_osInterface(osInterface),
    m_mmcEnabled(false)
{
    if (nullptr == m_osInterface)
    {
        return;
    }
    MEDIA_FEATURE_TABLE *skuTable = m_osInterface->pfnGetSkuTable(m_osInterface);
    m_isCompSurfAllocable = m_osInterface->pfnIsCompressibelSurfaceSupported(skuTable);
    m_userSettingPtr = m_osInterface->pfnGetUserSettingInstance(m_osInterface);
}

MOS_STATUS MediaMemComp::InitMmcEnabled()
{
    MOS_CHK_NULL_RETURN(MOS_COMPONENT_MMC, MOS_MMC_SUBCOMP_SELF, m_osInterface);

    if (MEDIA_IS_SKU(m_osInterface->pfnGetSkuTable(m_osInterface), FtrE2ECompression))
    {
        m_mmcEnabled = IsMmcFeatureEnabled();
        UpdateMmcInUseFeature();
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaMemComp::DecompressResource(PMOS_RESOURCE resource)
{
    MOS_STATUS status = MOS_STATUS_SUCCESS;

    if (resource)
    {
        status = m_osInterface->pfnDecompResource(m_osInterface, resource);
    }

    return status;
}


bool MediaMemComp::IsMmcFeatureEnabled()
{
    if (m_userSettingPtr != nullptr)
    {
        ReadUserSetting(
            m_userSettingPtr,
            m_mmcEnabled,
            m_mmcEnabledKey,
            MediaUserSetting::Group::Device,
            m_bComponentMmcEnabled,
            true);
    }
    else
    {
        m_mmcEnabled = m_bComponentMmcEnabled;
    }

    if (m_osInterface && m_osInterface->bNullHwIsEnabled)
    {
        m_mmcEnabled = false;
    }

    return m_mmcEnabled;
}

// For VP, there is no such feature id, if need to add 1?
MOS_STATUS MediaMemComp::UpdateMmcInUseFeature()
{
    return ReportUserSetting(
                m_userSettingPtr,
                m_mmcInUseKey,
                m_mmcEnabled,
                MediaUserSetting::Group::Device);
}

bool MediaMemComp::IsMmcEnabled()
{
    return m_mmcEnabled;
}

void MediaMemComp::DisableMmc()
{
    m_mmcEnabled = false;
}

bool MediaMemComp::IsCompressibelSurfaceSupported()
{
    return m_isCompSurfAllocable;
}

MOS_STATUS MediaMemComp::SetSurfaceMmcMode(
    PMOS_SURFACE surface)
{
    MOS_STATUS status = MOS_STATUS_SUCCESS;

    MOS_CHK_NULL_RETURN(MOS_COMPONENT_MMC, MOS_MMC_SUBCOMP_SELF, surface);
    MOS_CHK_NULL_RETURN(MOS_COMPONENT_MMC, MOS_MMC_SUBCOMP_SELF, m_osInterface);

    if(m_mmcEnabled)
        status = m_osInterface->pfnGetMemoryCompressionMode(m_osInterface, &surface->OsResource, (PMOS_MEMCOMP_STATE)&surface->CompressionMode);
    else
        surface->CompressionMode = MOS_MMC_DISABLED;

    return status;
}

MOS_STATUS MediaMemComp::SetSurfaceMmcState(
        PMOS_SURFACE surface)
{
    MOS_STATUS status = MOS_STATUS_SUCCESS;

    MOS_CHK_NULL_RETURN(MOS_COMPONENT_MMC, MOS_MMC_SUBCOMP_SELF, surface);
    MOS_CHK_NULL_RETURN(MOS_COMPONENT_MMC, MOS_MMC_SUBCOMP_SELF, m_osInterface);

    if(m_mmcEnabled)
        status =  m_osInterface->pfnGetMemoryCompressionMode(m_osInterface, &surface->OsResource, &surface->MmcState);
    else
        surface->MmcState = MOS_MEMCOMP_DISABLED;

    return status;
}

MOS_STATUS MediaMemComp::GetSurfaceMmcState(
    PMOS_SURFACE surface,
    MOS_MEMCOMP_STATE *mmcState)
{
    MOS_STATUS status = MOS_STATUS_SUCCESS;

    MOS_CHK_NULL_RETURN(MOS_COMPONENT_MMC, MOS_MMC_SUBCOMP_SELF, surface);
    MOS_CHK_NULL_RETURN(MOS_COMPONENT_MMC, MOS_MMC_SUBCOMP_SELF, mmcState);
    MOS_CHK_NULL_RETURN(MOS_COMPONENT_MMC, MOS_MMC_SUBCOMP_SELF, m_osInterface);

    if (m_mmcEnabled)
        status = m_osInterface->pfnGetMemoryCompressionMode(m_osInterface, &surface->OsResource, mmcState);
    else
        *mmcState = MOS_MEMCOMP_DISABLED;

    return status;
}

MOS_STATUS MediaMemComp::GetSurfaceMmcFormat(
    PMOS_SURFACE       surface,
    uint32_t *mmcFormat)
{
    MOS_STATUS status = MOS_STATUS_SUCCESS;

    MOS_CHK_NULL_RETURN(MOS_COMPONENT_MMC, MOS_MMC_SUBCOMP_SELF, surface);
    MOS_CHK_NULL_RETURN(MOS_COMPONENT_MMC, MOS_MMC_SUBCOMP_SELF, mmcFormat);
    MOS_CHK_NULL_RETURN(MOS_COMPONENT_MMC, MOS_MMC_SUBCOMP_SELF, m_osInterface);

    if (m_mmcEnabled)
        status = m_osInterface->pfnGetMemoryCompressionFormat(m_osInterface, &surface->OsResource, mmcFormat);
    else
        *mmcFormat = 0;

    return status;
}

MOS_STATUS MediaMemComp::GetResourceMmcFormat(
    PMOS_RESOURCE resource,
    uint32_t    &mmcFormat)
{
    MOS_STATUS status = MOS_STATUS_SUCCESS;
    MOS_CHK_NULL_RETURN(MOS_COMPONENT_MMC, MOS_MMC_SUBCOMP_SELF, resource);
    MOS_CHK_NULL_RETURN(MOS_COMPONENT_MMC, MOS_MMC_SUBCOMP_SELF, m_osInterface);

    if (m_mmcEnabled)
        status = m_osInterface->pfnGetMemoryCompressionFormat(m_osInterface, resource, &mmcFormat);
    else
        mmcFormat = 0;

    return status;
}

MOS_STATUS MediaMemComp::SetSurfaceMmcFormat(
        PMOS_SURFACE surface)
{
    MOS_STATUS status = MOS_STATUS_SUCCESS;

    MOS_CHK_NULL_RETURN(MOS_COMPONENT_MMC, MOS_MMC_SUBCOMP_SELF, surface);
    MOS_CHK_NULL_RETURN(MOS_COMPONENT_MMC, MOS_MMC_SUBCOMP_SELF, m_osInterface);

    if(m_mmcEnabled)
        status =  m_osInterface->pfnGetMemoryCompressionFormat(m_osInterface, &surface->OsResource, &surface->CompressionFormat);
    else
        surface->CompressionFormat = 0;

    return status;
}

MOS_STATUS MediaMemComp::GetResourceMmcState(
    PMOS_RESOURCE resource,
    MOS_MEMCOMP_STATE &mmcMode)
{
    MOS_STATUS status = MOS_STATUS_SUCCESS;

    MOS_CHK_NULL_RETURN(MOS_COMPONENT_MMC, MOS_MMC_SUBCOMP_SELF, resource);
    MOS_CHK_NULL_RETURN(MOS_COMPONENT_MMC, MOS_MMC_SUBCOMP_SELF, m_osInterface);
    //Need to Check its default value m_mmcEnable
    if (m_mmcEnabled)
        status = m_osInterface->pfnGetMemoryCompressionMode(m_osInterface, resource, &mmcMode);
    else
        mmcMode = MOS_MEMCOMP_DISABLED;

    return status;
}