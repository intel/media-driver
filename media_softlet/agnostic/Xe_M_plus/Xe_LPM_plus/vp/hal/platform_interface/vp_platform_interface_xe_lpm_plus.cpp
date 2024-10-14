/*
* Copyright (c) 2020-2021, Intel Corporation
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
//! \file     vp_platform_interface_xe_lpm_plus.cpp
//! \brief    Factories for vp object creation.
//!
#include "vp_feature_manager_xe_lpm_plus.h"
#include "vp_platform_interface_xe_lpm_plus.h"
#include "vp_vebox_cmd_packet_xe_lpm_plus.h"
#include "vp_render_sfc_xe_lpm_plus.h"
#include "vp_render_ief.h"
#include "vp_render_cmd_packet.h"
#include "vp_scalability_multipipe_next.h"
#include "vp_scalability_singlepipe_next.h"
#include "media_interfaces_mcpy_next.h"

using namespace vp;
extern const Kdll_RuleEntry         g_KdllRuleTable_Next[];
#define VEBOX_KERNEL_BASE_MAX_G13 2

VpPlatformInterfacesXe_Lpm_Plus::VpPlatformInterfacesXe_Lpm_Plus(PMOS_INTERFACE pOsInterface, bool clearViewMode)
    : VpPlatformInterface(pOsInterface, clearViewMode)
{
    bool defaultValue = 0;
    // get dithering flag.
    // Dithering should be enabled by default.
    m_disableSfcDithering   = false;

    ReadUserSetting(
        m_userSettingPtr,
        m_disableSfcDithering,
        __MEDIA_USER_FEATURE_VALUE_SFC_OUTPUT_DTR_DISABLE,
        MediaUserSetting::Group::Sequence,
        defaultValue,
        true);

    VP_PUBLIC_NORMALMESSAGE("m_disableSfcDithering = %d", m_disableSfcDithering);

    m_sfc2PassScalingEnabled = true;
#if LINUX
    char *sfc2PassPerfMode = getenv("SET_SFC2PASS_PERFMODE");
    if (sfc2PassPerfMode)
    {
        m_sfc2PassScalingPerfMode = strcmp(sfc2PassPerfMode, "ON") ? false : true;
    }
#endif
}

MOS_STATUS VpPlatformInterfacesXe_Lpm_Plus::InitVpVeboxSfcHwCaps(VP_VEBOX_ENTRY_REC *veboxHwEntry, uint32_t veboxEntryCount,
                                                            VP_SFC_ENTRY_REC *sfcHwEntry, uint32_t sfcEntryCount)
{
    VP_FUNC_CALL();

    if (veboxEntryCount < Format_Count || sfcEntryCount < Format_Count ||
        nullptr == veboxHwEntry || nullptr == sfcHwEntry)
    {
        veboxEntryCount = Format_Count;
        sfcEntryCount = Format_Count;
        return MOS_STATUS_INVALID_PARAMETER;
    }
#include "vp_feature_caps_xe_lpm_plus.h"
    return MOS_STATUS_SUCCESS;
}

VPFeatureManager *VpPlatformInterfacesXe_Lpm_Plus::CreateFeatureChecker(_VP_MHWINTERFACE *hwInterface)
{
    VP_FUNC_CALL();

    return MOS_New(VPFeatureManagerXe_Lpm_Plus, hwInterface);
}

VpCmdPacket *VpPlatformInterfacesXe_Lpm_Plus::CreateVeboxPacket(MediaTask * task, _VP_MHWINTERFACE *hwInterface, VpAllocator *&allocator, VPMediaMemComp *mmc)
{
    VP_FUNC_CALL();

    return MOS_New(VpVeboxCmdPacketXe_Lpm_Plus, task, hwInterface, allocator, mmc, m_disableSfcDithering);
}

VpCmdPacket *VpPlatformInterfacesXe_Lpm_Plus::CreateRenderPacket(MediaTask * task, _VP_MHWINTERFACE *hwInterface, VpAllocator *&allocator, VPMediaMemComp *mmc, VpKernelSet* kernel)
{
    VP_FUNC_CALL();

    return MOS_New(VpRenderCmdPacket, task, hwInterface, allocator, mmc, kernel);
}

MOS_STATUS VpPlatformInterfacesXe_Lpm_Plus::CreateSfcRender(SfcRenderBase *&sfcRender, VP_MHWINTERFACE &vpMhwinterface, PVpAllocator allocator)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(allocator);

    sfcRender = MOS_New(SfcRenderXe_Lpm_Plus,
            vpMhwinterface,
            allocator,
            m_disableSfcDithering);
    VP_PUBLIC_CHK_NULL_RETURN(sfcRender);

    VpIef *iefObj = MOS_New(VpIef);

    if (nullptr == iefObj)
    {
        MOS_Delete(sfcRender);
        VP_PUBLIC_CHK_NULL_RETURN(iefObj);
    }

    MOS_STATUS status = sfcRender->SetIefObj(iefObj);
    if (MOS_FAILED(status))
    {
        MOS_Delete(sfcRender);
        VP_PUBLIC_CHK_STATUS_RETURN(status);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpPlatformInterfacesXe_Lpm_Plus::VeboxQueryStatLayout(VEBOX_STAT_QUERY_TYPE queryType, uint32_t* pQuery)
{
    VP_FUNC_CALL();

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    VP_RENDER_ASSERT(pQuery);

    switch (queryType)
    {
    case VEBOX_STAT_QUERY_GNE_OFFEST:
        *pQuery = VP_VEBOX_STATISTICS_SURFACE_GNE_OFFSET_XE_LPM_PLUS;
        break;

    case VEBOX_STAT_QUERY_PER_FRAME_SIZE:
        *pQuery = VP_VEBOX_STATISTICS_PER_FRAME_SIZE_XE_LPM_PLUS;
        break;

    case VEBOX_STAT_QUERY_FMD_OFFEST:
        *pQuery = VP_VEBOX_STATISTICS_SURFACE_FMD_OFFSET_XE_LPM_PLUS;
        break;

    case VEBOX_STAT_QUERY_STD_OFFEST:
        *pQuery = VP_VEBOX_STATISTICS_SURFACE_STD_OFFSET_XE_LPM_PLUS;
        break;

    default:
        VP_RENDER_ASSERTMESSAGE("Vebox Statistics Layout Query, type ('%d') is not implemented.", queryType);
        eStatus = MOS_STATUS_UNKNOWN;
        break;
    }

    return eStatus;
}

MOS_STATUS VpPlatformInterfacesXe_Lpm_Plus::ConfigVirtualEngine()
{
    VP_PUBLIC_CHK_NULL_RETURN(m_pOsInterface);
    Mos_SetVirtualEngineSupported(m_pOsInterface, true);
    m_pOsInterface->pfnVirtualEngineSupported(m_pOsInterface, true, true);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpPlatformInterfacesXe_Lpm_Plus::ConfigureVpScalability(VP_MHWINTERFACE &vpMhwInterface)
{
    VP_FUNC_CALL();

    vpMhwInterface.pfnCreateSinglePipe = vp::VpScalabilitySinglePipeNext::CreateSinglePipe;
    vpMhwInterface.pfnCreateMultiPipe   = vp::VpScalabilityMultiPipeNext::CreateMultiPipe;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpPlatformInterfacesXe_Lpm_Plus::InitVpFeatureSupportBits()
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_STATUS_RETURN(VpPlatformInterface::InitVpFeatureSupportBits());
    return MOS_STATUS_SUCCESS;
}
