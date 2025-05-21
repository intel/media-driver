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
//! \file     vp_platform_interface_xe2_hpm.cpp
//! \brief    Factories for vp object creation.
//!
#include "vp_feature_manager_xe2_hpm.h"
#include "vp_platform_interface_xe2_hpm.h"
#include "vp_vebox_cmd_packet_xe2_hpm.h"
#include "vp_render_sfc_xe2_hpm.h"
#include "vp_render_ief.h"
#include "vp_render_cmd_packet.h"
#include "vp_scalability_multipipe_next.h"
#include "vp_scalability_singlepipe_next.h"
#include "media_interfaces_mcpy_next.h"

using namespace vp;
extern const Kdll_RuleEntry         g_KdllRuleTable_Next[];

VpPlatformInterfaceXe2_Hpm::VpPlatformInterfaceXe2_Hpm(PMOS_INTERFACE pOsInterface, bool clearViewMode)
    : VpPlatformInterface(pOsInterface, clearViewMode)
{
    bool defaultValue = 0;
    // get dithering flag.
    // Dithering should be enabled by default.
    m_disableSfcDithering = false;

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

MOS_STATUS VpPlatformInterfaceXe2_Hpm::InitVpVeboxSfcHwCaps(VP_VEBOX_ENTRY_REC *veboxHwEntry, uint32_t veboxEntryCount,
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
#include "vp_feature_caps_xe2_hpm.h"
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpPlatformInterfaceXe2_Hpm::InitVpRenderHwCaps()
{
    VP_FUNC_CALL();

    if (m_isRenderDisabled)
    {
        VP_PUBLIC_NORMALMESSAGE("Bypass InitVpRenderHwCaps, since render disabled.");
        return MOS_STATUS_SUCCESS;
    }
#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
    VP_RENDER_CHK_NULL_RETURN(m_vpKernelBinary.kernelBin);
    VP_RENDER_CHK_NULL_RETURN(m_vpKernelBinary.fcPatchKernelBin);
#endif
    // Only Lpm Plus will use this base function
    m_modifyKdllFunctionPointers = KernelDll_ModifyFunctionPointers_Next;
#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
    InitVPFCKernels(
        g_KdllRuleTable_Next,
        m_vpKernelBinary.kernelBin,
        m_vpKernelBinary.kernelBinSize,
        m_vpKernelBinary.fcPatchKernelBin,
        m_vpKernelBinary.fcPatchKernelBinSize,
        m_modifyKdllFunctionPointers);
#endif

    if (!m_vpIsaKernelBinaryList.empty())
    {
        // Init CM kernel form VP ISA Kernel Binary List
        for (auto &curKernelEntry : m_vpIsaKernelBinaryList)
        {
            VP_PUBLIC_CHK_STATUS_RETURN(InitVpCmKernels(curKernelEntry.kernelBin, curKernelEntry.kernelBinSize, curKernelEntry.postfix, curKernelEntry.payloadOffset));
        }
    }

    if (!m_vpNativeAdvKernelBinaryList.empty())
    {
        // Init native adv kernel form VP Native adv kernel Binary List
        for (auto &curKernelEntry : m_vpNativeAdvKernelBinaryList)
        {
            InitVpNativeAdvKernels(curKernelEntry.first, curKernelEntry.second);
        }
    }
    return MOS_STATUS_SUCCESS;
}

VPFeatureManager *VpPlatformInterfaceXe2_Hpm::CreateFeatureChecker(_VP_MHWINTERFACE *hwInterface)
{
    VP_FUNC_CALL();

    return MOS_New(VPFeatureManagerXe2_Hpm, hwInterface);
}

VpCmdPacket *VpPlatformInterfaceXe2_Hpm::CreateVeboxPacket(MediaTask * task, _VP_MHWINTERFACE *hwInterface, VpAllocator *&allocator, VPMediaMemComp *mmc)
{
    VP_FUNC_CALL();

    return MOS_New(VpVeboxCmdPacketXe2_Hpm, task, hwInterface, allocator, mmc, m_disableSfcDithering);
}

VpCmdPacket *VpPlatformInterfaceXe2_Hpm::CreateRenderPacket(MediaTask * task, _VP_MHWINTERFACE *hwInterface, VpAllocator *&allocator, VPMediaMemComp *mmc, VpKernelSet* kernel)
{
    VP_FUNC_CALL();

    return MOS_New(VpRenderCmdPacket, task, hwInterface, allocator, mmc, kernel);
}

MOS_STATUS VpPlatformInterfaceXe2_Hpm::CreateSfcRender(SfcRenderBase *&sfcRender, VP_MHWINTERFACE &vpMhwinterface, PVpAllocator allocator)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(allocator);

    sfcRender = MOS_New(SfcRenderXe2_Hpm,
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

MOS_STATUS vp::VpPlatformInterfaceXe2_Hpm::VeboxQueryStatLayout(VEBOX_STAT_QUERY_TYPE queryType, uint32_t* pQuery)
{
    VP_FUNC_CALL();

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    VP_RENDER_ASSERT(pQuery);

    switch (queryType)
    {
    case VEBOX_STAT_QUERY_GNE_OFFEST:
        *pQuery = VP_VEBOX_STATISTICS_SURFACE_GNE_OFFSET_XE2_HPM;
        break;

    case VEBOX_STAT_QUERY_PER_FRAME_SIZE:
        *pQuery = VP_VEBOX_STATISTICS_PER_FRAME_SIZE_XE2_HPM;
        break;

    case VEBOX_STAT_QUERY_FMD_OFFEST:
        *pQuery = VP_VEBOX_STATISTICS_SURFACE_FMD_OFFSET_XE2_HPM;
        break;

    case VEBOX_STAT_QUERY_STD_OFFEST:
        *pQuery = VP_VEBOX_STATISTICS_SURFACE_STD_OFFSET_XE2_HPM;
        break;

    default:
        VP_RENDER_ASSERTMESSAGE("Vebox Statistics Layout Query, type ('%d') is not implemented.", queryType);
        eStatus = MOS_STATUS_UNKNOWN;
        break;
    }

    return eStatus;
}

MOS_STATUS VpPlatformInterfaceXe2_Hpm::GetKernelBinary(const void *&kernelBin, uint32_t &kernelSize, const void *&patchKernelBin, uint32_t &patchKernelSize)
{
    VP_FUNC_CALL();

    kernelBin       = m_vpKernelBinary.kernelBin;
    kernelSize      = m_vpKernelBinary.kernelBinSize;
    patchKernelBin  = m_vpKernelBinary.fcPatchKernelBin;
    patchKernelSize = m_vpKernelBinary.fcPatchKernelBinSize;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpPlatformInterfaceXe2_Hpm::ConfigVirtualEngine()
{
    VP_PUBLIC_CHK_NULL_RETURN(m_pOsInterface);
    Mos_SetVirtualEngineSupported(m_pOsInterface, true);
    m_pOsInterface->pfnVirtualEngineSupported(m_pOsInterface, true, true);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpPlatformInterfaceXe2_Hpm::ConfigureVpScalability(VP_MHWINTERFACE &vpMhwInterface)
{
    VP_FUNC_CALL();

    vpMhwInterface.pfnCreateSinglePipe = vp::VpScalabilitySinglePipeNext::CreateSinglePipe;
    vpMhwInterface.pfnCreateMultiPipe  = vp::VpScalabilityMultiPipeNext::CreateMultiPipe;

    return MOS_STATUS_SUCCESS;
}

bool VpPlatformInterfaceXe2_Hpm::IsVeboxScalabilityWith4KNotSupported(
        VP_MHWINTERFACE          vpMhwInterface)
{
    if (vpMhwInterface.m_veboxInterface && !(vpMhwInterface.m_veboxInterface->m_veboxScalabilitywith4K))
    {
        return true;
    }
    else
    {
        return false;
    }
}

MOS_STATUS VpPlatformInterfaceXe2_Hpm::InitPolicyRules(VP_POLICY_RULES &rules)
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_STATUS_RETURN(VpPlatformInterface::InitPolicyRules(rules));

    rules.isHDR33LutSizeEnabled = true;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpPlatformInterfaceXe2_Hpm::InitVpFeatureSupportBits()
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_STATUS_RETURN(VpPlatformInterface::InitVpFeatureSupportBits());
    return MOS_STATUS_SUCCESS;
}
