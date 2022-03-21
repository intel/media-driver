/*
* Copyright (c) 2020-2022, Intel Corporation
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
//! \file     vp_platform_interface_xe_hpm.cpp
//! \brief    Factories for vp object creation.
//!
#include "vp_feature_manager_xe_hpm.h"
#include "vp_platform_interface_xe_hpm.h"
#include "vp_vebox_cmd_packet_xe_hpm.h"
#include "media_user_settings_mgr_g12.h"
#include "vp_render_sfc_xe_xpm_base.h"
#include "vp_render_ief.h"
#include "vp_render_cmd_packet.h"
#if defined(ENABLE_KERNELS)
#include "igvpkrn_xe_hpm.h"
#include "igvpkrn_xe_hpm_cmfcpatch.h"
#if !defined(_FULL_OPEN_SOURCE)
#include "igvpkrn_isa_xe_hpm.h"
#endif
#endif
#include "vp_kernel_config_m12_base.h"

extern const Kdll_RuleEntry         g_KdllRuleTable_Xe_Hpm[];

using namespace vp;

VpPlatformInterfaceXe_Hpm::VpPlatformInterfaceXe_Hpm(PMOS_INTERFACE pOsInterface)
    : VpPlatformInterface(pOsInterface)
{
    ReadUserSetting(
        m_userSettingPtr,
        m_disableSfcDithering,
        __MEDIA_USER_FEATURE_VALUE_SFC_OUTPUT_DTR_DISABLE,
        MediaUserSetting::Group::Sequence,
        0,
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

MOS_STATUS VpPlatformInterfaceXe_Hpm::InitVpVeboxSfcHwCaps(VP_VEBOX_ENTRY_REC *veboxHwEntry, uint32_t veboxEntryCount,
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
#include "vp_feature_caps_xe_hpm.h"
    return MOS_STATUS_SUCCESS;
}

void KernelDll_ModifyFunctionPointers_g12hp(Kdll_State *pState);

MOS_STATUS VpPlatformInterfaceXe_Hpm::InitVpRenderHwCaps()
{
    VP_FUNC_CALL();

    m_modifyKdllFunctionPointers = KernelDll_ModifyFunctionPointers_g12hp;
#if defined(ENABLE_KERNELS)
    InitVPFCKernels(
       g_KdllRuleTable_Xe_Hpm,
       IGVPKRN_XE_HPM,
       IGVPKRN_XE_HPM_SIZE,
       IGVPKRN_XE_HPM_CMFCPATCH,
       IGVPKRN_XE_HPM_CMFCPATCH_SIZE,
       m_modifyKdllFunctionPointers);
#if !defined(_FULL_OPEN_SOURCE)
    VP_PUBLIC_CHK_STATUS_RETURN(InitVpCmKernels((const uint32_t *)IGVP3DLUT_GENERATION_XE_HPM, IGVP3DLUT_GENERATION_XE_HPM_SIZE));
#endif
#endif

    return MOS_STATUS_SUCCESS;
}

VPFeatureManager *VpPlatformInterfaceXe_Hpm::CreateFeatureChecker(_VP_MHWINTERFACE *hwInterface)
{
    VP_FUNC_CALL();

    return MOS_New(VPFeatureManagerXe_Hpm, hwInterface);
}

VpCmdPacket *VpPlatformInterfaceXe_Hpm::CreateVeboxPacket(MediaTask * task, _VP_MHWINTERFACE *hwInterface, VpAllocator *&allocator, VPMediaMemComp *mmc)
{
    VP_FUNC_CALL();

    return MOS_New(VpVeboxCmdPacketXe_Hpm, task, hwInterface, allocator, mmc, m_disableSfcDithering);
}

VpCmdPacket *VpPlatformInterfaceXe_Hpm::CreateRenderPacket(MediaTask * task, _VP_MHWINTERFACE *hwInterface, VpAllocator *&allocator, VPMediaMemComp *mmc, VpKernelSet* kernel)
{
    VP_FUNC_CALL();

    return MOS_New(VpRenderCmdPacket, task, hwInterface, allocator, mmc, kernel);
}

MOS_STATUS VpPlatformInterfaceXe_Hpm::VeboxQueryStatLayout(VEBOX_STAT_QUERY_TYPE queryType, uint32_t* pQuery)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    VPHAL_RENDER_ASSERT(pQuery);

    switch (queryType)
    {
    case VEBOX_STAT_QUERY_GNE_OFFEST:
        *pQuery = VP_VEBOX_STATISTICS_SURFACE_GNE_OFFSET_G12;
        break;

    case VEBOX_STAT_QUERY_PER_FRAME_SIZE:
        *pQuery = VP_VEBOX_STATISTICS_PER_FRAME_SIZE_G12;
        break;

    case VEBOX_STAT_QUERY_FMD_OFFEST:
        *pQuery = VP_VEBOX_STATISTICS_SURFACE_FMD_OFFSET_G12;
        break;

    case VEBOX_STAT_QUERY_STD_OFFEST:
        *pQuery = VP_VEBOX_STATISTICS_SURFACE_STD_OFFSET_G12;
        break;

    default:
        VPHAL_RENDER_ASSERTMESSAGE("Vebox Statistics Layout Query, type ('%d') is not implemented.", queryType);
        eStatus = MOS_STATUS_UNKNOWN;
        break;
    }

    return eStatus;
}

MOS_STATUS VpPlatformInterfaceXe_Hpm::CreateSfcRender(SfcRenderBase *&sfcRender, VP_MHWINTERFACE &vpMhwinterface, PVpAllocator allocator)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(allocator);

    sfcRender = MOS_New(SfcRenderXe_Xpm_Base,
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

VpKernelConfig &VpPlatformInterfaceXe_Hpm::GetKernelConfig()
{
    static VpKernelConfigM12_Base kernelConfig;
    return kernelConfig;
}
