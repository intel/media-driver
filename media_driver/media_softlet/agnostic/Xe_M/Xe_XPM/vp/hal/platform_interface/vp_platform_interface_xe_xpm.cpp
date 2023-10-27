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
//! \file     vp_platform_interface_xe_xpm.cpp
//! \brief    Factories for vp object creation.
//!
#include "vp_feature_manager_xe_xpm.h"
#include "vp_platform_interface_xe_xpm.h"
#include "vp_vebox_cmd_packet_xe_xpm.h"
#include "vp_render_sfc_xe_xpm_base.h"
#include "vp_render_ief.h"
#include "vp_scalability_multipipe.h"
#include "vp_scalability_singlepipe.h"

using namespace vp;

VpPlatformInterfaceXe_Xpm::VpPlatformInterfaceXe_Xpm(PMOS_INTERFACE pOsInterface)
    : VpPlatformInterface(pOsInterface)
{

    // get dithering flag.
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

MOS_STATUS VpPlatformInterfaceXe_Xpm::InitVpVeboxSfcHwCaps(VP_VEBOX_ENTRY_REC *veboxHwEntry, uint32_t veboxEntryCount,
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
#include "vp_feature_caps_xe_xpm.h"
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpPlatformInterfaceXe_Xpm::InitVpRenderHwCaps()
{
    VP_FUNC_CALL();

    return MOS_STATUS_SUCCESS;
}

VPFeatureManager *VpPlatformInterfaceXe_Xpm::CreateFeatureChecker(_VP_MHWINTERFACE *hwInterface)
{
    VP_FUNC_CALL();

    return MOS_New(VPFeatureManagerXe_Xpm, hwInterface);
}

VpCmdPacket *VpPlatformInterfaceXe_Xpm::CreateVeboxPacket(MediaTask * task, _VP_MHWINTERFACE *hwInterface, VpAllocator *&allocator, VPMediaMemComp *mmc)
{
    VP_FUNC_CALL();

    return MOS_New(VpVeboxCmdPacketXe_Xpm, task, hwInterface, allocator, mmc, m_disableSfcDithering);
}

VpCmdPacket *VpPlatformInterfaceXe_Xpm::CreateRenderPacket(MediaTask * task, _VP_MHWINTERFACE *hwInterface, VpAllocator *&allocator, VPMediaMemComp *mmc, VpKernelSet* kernel)
{
    VP_FUNC_CALL();

    return nullptr;
}

MOS_STATUS VpPlatformInterfaceXe_Xpm::VeboxQueryStatLayout(VEBOX_STAT_QUERY_TYPE queryType, uint32_t* pQuery)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    VP_RENDER_CHK_NULL_RETURN(pQuery);

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

MOS_STATUS VpPlatformInterfaceXe_Xpm::CreateSfcRender(SfcRenderBase *&sfcRender, VP_MHWINTERFACE &vpMhwinterface, PVpAllocator allocator)
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

MOS_STATUS VpPlatformInterfaceXe_Xpm::GetInputFrameWidthHeightAlignUnit(
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

MOS_STATUS VpPlatformInterfaceXe_Xpm::GetVeboxHeapInfo(
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

bool VpPlatformInterfaceXe_Xpm::IsVeboxScalabilityWith4KNotSupported(
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

MOS_STATUS VpPlatformInterfaceXe_Xpm::ConfigureVpScalability(VP_MHWINTERFACE &vpMhwInterface)
{
    VP_FUNC_CALL();

    vpMhwInterface.pfnCreateSinglePipe = vp::VpScalabilitySinglePipe::CreateSinglePipe;
    vpMhwInterface.pfnCreateMultiPipe  = vp::VpScalabilityMultiPipe::CreateMultiPipe;

    return MOS_STATUS_SUCCESS;
}
