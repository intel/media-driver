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
#include "vp_render_sfc_xe_xpm_base.h"
#include "vp_render_ief.h"
#include "vp_render_cmd_packet.h"
#include "vp_scalability_multipipe.h"
#include "vp_scalability_singlepipe.h"
#include "media_interfaces_mcpy.h"

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
       m_vpKernelBinary.kernelBin,
       m_vpKernelBinary.kernelBinSize,
       m_vpKernelBinary.fcPatchKernelBin,
       m_vpKernelBinary.fcPatchKernelBinSize,
       m_modifyKdllFunctionPointers);
#if !defined(_FULL_OPEN_SOURCE)
    // Init CM kernel form VP ISA Kernel Binary List
    for (auto &curKernelEntry : m_vpIsaKernelBinaryList)
    {
       VP_PUBLIC_CHK_STATUS_RETURN(InitVpCmKernels(curKernelEntry.kernelBin, curKernelEntry.kernelBinSize, curKernelEntry.postfix));
    }
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

MediaCopyBaseState *VpPlatformInterfaceXe_Hpm::CreateMediaCopy()
{
    VP_FUNC_CALL();
    
    MediaCopyBaseState *mediaCopy   = nullptr;
    PMOS_CONTEXT       mos_context  = nullptr;

    if (m_pOsInterface && m_pOsInterface->pfnGetMosContext)
    {
        m_pOsInterface->pfnGetMosContext(m_pOsInterface, &mos_context);
    }
    mediaCopy = static_cast<MediaCopyBaseState *>(McpyDevice::CreateFactory(mos_context));

    return mediaCopy;
}

MOS_STATUS VpPlatformInterfaceXe_Hpm::VeboxQueryStatLayout(VEBOX_STAT_QUERY_TYPE queryType, uint32_t* pQuery)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    VP_RENDER_ASSERT(pQuery);

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
        VP_RENDER_ASSERTMESSAGE("Vebox Statistics Layout Query, type ('%d') is not implemented.", queryType);
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

MOS_STATUS VpPlatformInterfaceXe_Hpm::GetInputFrameWidthHeightAlignUnit(
    PVP_MHWINTERFACE          pvpMhwInterface,
    uint32_t                 &widthAlignUnit,
    uint32_t                 &heightAlignUnit,
    bool                      bVdbox,
    CODECHAL_STANDARD         codecStandard,
    CodecDecodeJpegChromaType jpegChromaType)
{
    VP_FUNC_CALL();

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    VP_PUBLIC_CHK_NULL_RETURN(pvpMhwInterface);
    VP_PUBLIC_CHK_NULL_RETURN(pvpMhwInterface->m_sfcInterface);

    VP_PUBLIC_CHK_STATUS_RETURN(pvpMhwInterface->m_sfcInterface->GetInputFrameWidthHeightAlignUnit(widthAlignUnit, heightAlignUnit, bVdbox, codecStandard, jpegChromaType));

    return eStatus;
}

MOS_STATUS VpPlatformInterfaceXe_Hpm::GetVeboxHeapInfo(
    PVP_MHWINTERFACE          pvpMhwInterface,
    const MHW_VEBOX_HEAP    **ppVeboxHeap)
{
    VP_FUNC_CALL();

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    VP_PUBLIC_CHK_NULL_RETURN(pvpMhwInterface);
    VP_PUBLIC_CHK_NULL_RETURN(pvpMhwInterface->m_veboxInterface);

    const MHW_VEBOX_HEAP *pVeboxHeap = nullptr;

    VP_RENDER_CHK_STATUS_RETURN(pvpMhwInterface->m_veboxInterface->GetVeboxHeapInfo(
        &pVeboxHeap));
    *ppVeboxHeap = (const MHW_VEBOX_HEAP *)pVeboxHeap;

    return eStatus;
}

bool VpPlatformInterfaceXe_Hpm::IsVeboxScalabilityWith4KNotSupported(
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

MOS_STATUS VpPlatformInterfaceXe_Hpm::ConfigureVpScalability(VP_MHWINTERFACE &vpMhwInterface)
{
    VP_FUNC_CALL();

    vpMhwInterface.pfnCreateSinglePipe = vp::VpScalabilitySinglePipe::CreateSinglePipe;
    vpMhwInterface.pfnCreateMultiPipe  = vp::VpScalabilityMultiPipe::CreateMultiPipe;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpPlatformInterfaceXe_Hpm::InitVpFeatureSupportBits()
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_STATUS_RETURN(VpPlatformInterface::InitVpFeatureSupportBits());
    return MOS_STATUS_SUCCESS;
}