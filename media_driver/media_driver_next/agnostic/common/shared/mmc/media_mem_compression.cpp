/*
* Copyright (c) 2018, Intel Corporation
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

#include "mos_defs.h"
#include "media_mem_compression.h"
#include "mhw_mi_g12_X.h"

MediaMemComp::MediaMemComp(PMOS_INTERFACE osInterface, MhwMiInterface *miInterface):
    m_osInterface(osInterface),
    m_mhwMiInterface(miInterface),
    m_mmcEnabled(false),
    m_mmcFeatureId(__MOS_USER_FEATURE_KEY_MAX_ID),
    m_mmcInuseFeatureId(__MOS_USER_FEATURE_KEY_MAX_ID)
{
    uint32_t auxTableBaseAddrHigh = 0;
    uint32_t auxTableBaseAddrLow = 0;

    m_auxTableBaseAddr = m_osInterface->pfnGetAuxTableBaseAddr(m_osInterface);

    auxTableBaseAddrHigh = (m_auxTableBaseAddr >> 32) & 0xffffffff;
    auxTableBaseAddrLow = m_auxTableBaseAddr & 0xffffffff;

    m_auxtableRegisterArray[renderAuxtableRegisterIndex] = {
        { {MhwMiInterfaceG12::m_mmioRcsAuxTableBaseLow    , auxTableBaseAddrLow  },
          {MhwMiInterfaceG12::m_mmioRcsAuxTableBaseHigh   , auxTableBaseAddrHigh },
          {MhwMiInterfaceG12::m_mmioCcs0AuxTableBaseLow   , auxTableBaseAddrLow  },
          {MhwMiInterfaceG12::m_mmioCcs0AuxTableBaseHigh  , auxTableBaseAddrHigh }
        },
        4
    };

    m_auxtableRegisterArray[veboxAuxtableRegisterIndex] = {
        { {MhwMiInterfaceG12::m_mmioVe0AuxTableBaseLow    , auxTableBaseAddrLow  },
          {MhwMiInterfaceG12::m_mmioVe0AuxTableBaseHigh   , auxTableBaseAddrHigh },
          {0                                              , 0},
          {0                                              , 0}
        },
        2
    };

    m_auxtableRegisterArray[vdboxAuxtableRegisterIndex] = {
        { {MhwMiInterfaceG12::m_mmioVd0AuxTableBaseLow    , auxTableBaseAddrLow  },
          {MhwMiInterfaceG12::m_mmioVd0AuxTableBaseHigh   , auxTableBaseAddrHigh },
          {MhwMiInterfaceG12::m_mmioVd2AuxTableBaseLow    , auxTableBaseAddrLow  },
          {MhwMiInterfaceG12::m_mmioVd2AuxTableBaseHigh   , auxTableBaseAddrHigh }
        },
        4
    };
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


bool MediaMemComp::IsMmcFeatureEnabled()
{
    MOS_USER_FEATURE_VALUE_DATA userFeatureData;
    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));

    userFeatureData.i32Data = m_bComponentMmcEnabled;

    userFeatureData.i32DataFlag = MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE;
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        m_mmcFeatureId,
        &userFeatureData);
    m_mmcEnabled = (userFeatureData.i32Data) ? true : false;

    return m_mmcEnabled;
}

// For VP, there is no such feature id, if need to add 1?
MOS_STATUS MediaMemComp::UpdateMmcInUseFeature()
{
    MOS_USER_FEATURE_VALUE_WRITE_DATA userFeatureWriteData;
    MOS_ZeroMemory(&userFeatureWriteData, sizeof(userFeatureWriteData));
    userFeatureWriteData.Value.i32Data = m_mmcEnabled;
    userFeatureWriteData.ValueID = m_mmcInuseFeatureId;

    return MOS_UserFeature_WriteValues_ID(nullptr, &userFeatureWriteData, 1);
}

bool MediaMemComp::IsMmcEnabled()
{
    return m_mmcEnabled;
}

void MediaMemComp::DisableMmc()
{
    m_mmcEnabled = false;
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

MOS_STATUS MediaMemComp::AddMiLoadRegisterImmCmd(PMOS_COMMAND_BUFFER cmdBuffer, uint32_t auxtableRegisterIndex)
{
    MOS_STATUS status = MOS_STATUS_SUCCESS;

    MOS_CHK_NULL_RETURN(MOS_COMPONENT_MMC, MOS_MMC_SUBCOMP_SELF, cmdBuffer);

    if (0 == m_auxTableBaseAddr)
        return MOS_STATUS_INVALID_PARAMETER;

    if(auxtableRegisterIndex > maxAuxtableRegisterIndex)
        return MOS_STATUS_INVALID_PARAMETER;

    AuxTableRegisterParams auxtableRegisterParam = m_auxtableRegisterArray[auxtableRegisterIndex];
    uint32_t size = MOS_MIN(MAX_AUXTALBE_REGISTER_COUNT, auxtableRegisterParam.size);
    for (uint32_t index = 0; index < size; index++)
    {
        status = m_mhwMiInterface->AddMiLoadRegisterImmCmd(cmdBuffer,&auxtableRegisterParam.lriParams[index]);

        MOS_CHK_STATUS_RETURN(MOS_COMPONENT_MMC, MOS_MMC_SUBCOMP_SELF, status);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaMemComp::SendPrologCmd(
    PMOS_COMMAND_BUFFER   cmdBuffer,
    MOS_GPU_CONTEXT       gpuContext)
{
    MOS_STATUS status = MOS_STATUS_SUCCESS;

    MOS_CHK_NULL_RETURN(MOS_COMPONENT_MMC, MOS_MMC_SUBCOMP_SELF, cmdBuffer);

    if (!m_mmcEnabled)
        return MOS_STATUS_SUCCESS;

    if(gpuContext >= MOS_GPU_CONTEXT_MAX )
        return MOS_STATUS_INVALID_PARAMETER;

    if (MOS_RCS_ENGINE_USED(gpuContext))
    {
        status = AddMiLoadRegisterImmCmd(cmdBuffer, renderAuxtableRegisterIndex);
    }
    else if (MOS_VECS_ENGINE_USED(gpuContext))
    {
        status = AddMiLoadRegisterImmCmd(cmdBuffer, veboxAuxtableRegisterIndex);
    }
    else
    {
        status = AddMiLoadRegisterImmCmd(cmdBuffer, vdboxAuxtableRegisterIndex);
    }

    MOS_CHK_STATUS_RETURN(MOS_COMPONENT_MMC, MOS_MMC_SUBCOMP_SELF, status);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaMemComp::SendPrologCmd(
    PMOS_COMMAND_BUFFER         cmdBuffer)
{
    MOS_STATUS status = MOS_STATUS_SUCCESS;

    MOS_CHK_NULL_RETURN(MOS_COMPONENT_MMC, MOS_MMC_SUBCOMP_SELF, cmdBuffer);
    MOS_CHK_NULL_RETURN(MOS_COMPONENT_MMC, MOS_MMC_SUBCOMP_SELF, m_mhwMiInterface);

    if (!m_mmcEnabled)
        return MOS_STATUS_SUCCESS;

    if (0 != m_auxTableBaseAddr)
    {
        MHW_MI_LOAD_REGISTER_IMM_PARAMS lriParams;
        MOS_ZeroMemory(&lriParams, sizeof(lriParams));

        lriParams.dwRegister = MhwMiInterfaceG12::m_mmioRcsAuxTableBaseLow;
        lriParams.dwData     = (m_auxTableBaseAddr & 0xffffffff);
        status               = m_mhwMiInterface->AddMiLoadRegisterImmCmd(cmdBuffer, &lriParams);
        MOS_CHK_STATUS_RETURN(MOS_COMPONENT_MMC, MOS_MMC_SUBCOMP_SELF, status);

        lriParams.dwRegister = MhwMiInterfaceG12::m_mmioRcsAuxTableBaseHigh;
        lriParams.dwData     = ((m_auxTableBaseAddr >> 32) & 0xffffffff);
        status               = m_mhwMiInterface->AddMiLoadRegisterImmCmd(cmdBuffer, &lriParams);
        MOS_CHK_STATUS_RETURN(MOS_COMPONENT_MMC, MOS_MMC_SUBCOMP_SELF, status);
    }
    return MOS_STATUS_SUCCESS;
}
