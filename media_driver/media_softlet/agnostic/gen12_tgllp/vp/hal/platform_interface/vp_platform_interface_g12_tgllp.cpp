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
//! \file     vp_platform_interface_g12_tgllp.cpp
//! \brief    Factories for vp object creation.
//!
#include "vp_feature_manager_m12_0.h"
#include "vp_platform_interface_g12_tgllp.h"
#include "vp_vebox_cmd_packet_g12.h"
#include "vp_render_sfc_m12.h"
#include "vp_render_ief.h"
#if defined(ENABLE_KERNELS)
#include "igvpkrn_g12_tgllp_cmfc.h"
#include "igvpkrn_g12_tgllp_cmfcpatch.h"
#endif
#include "vp_scalability_multipipe.h"
#include "vp_scalability_singlepipe.h"

extern const Kdll_RuleEntry         g_KdllRuleTable_g12lp[];
extern const Kdll_RuleEntry         g_KdllRuleTable_g12lpcmfc[];

using namespace vp;

MOS_STATUS VpPlatformInterfaceG12Tgllp::InitVpVeboxSfcHwCaps(VP_VEBOX_ENTRY_REC *veboxHwEntry, uint32_t veboxEntryCount,
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
#include "vp_feature_caps_g12.h"
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpPlatformInterfaceG12Tgllp::InitVpRenderHwCaps()
{
    VP_FUNC_CALL();

    m_modifyKdllFunctionPointers = nullptr;
#if defined(ENABLE_KERNELS)
    InitVPFCKernels(
        g_KdllRuleTable_g12lpcmfc,
        IGVPKRN_G12_TGLLP_CMFC,
        IGVPKRN_G12_TGLLP_CMFC_SIZE,
        IGVPKRN_G12_TGLLP_CMFCPATCH,
        IGVPKRN_G12_TGLLP_CMFCPATCH_SIZE,
        m_modifyKdllFunctionPointers);
#endif

    return MOS_STATUS_SUCCESS;
}

VPFeatureManager *VpPlatformInterfaceG12Tgllp::CreateFeatureChecker(_VP_MHWINTERFACE *hwInterface)
{
    VP_FUNC_CALL();

    return MOS_New(VPFeatureManagerM12_0, hwInterface);
}

VpCmdPacket *VpPlatformInterfaceG12Tgllp::CreateVeboxPacket(MediaTask * task, _VP_MHWINTERFACE *hwInterface, VpAllocator *&allocator, VPMediaMemComp *mmc)
{
    VP_FUNC_CALL();

    return MOS_New(VpVeboxCmdPacketG12, task, hwInterface, allocator, mmc);
}

VpCmdPacket *VpPlatformInterfaceG12Tgllp::CreateRenderPacket(MediaTask * task, _VP_MHWINTERFACE *hwInterface, VpAllocator *&allocator, VPMediaMemComp *mmc, VpKernelSet* kernel)
{
    VP_FUNC_CALL();

    return nullptr;
}

MOS_STATUS VpPlatformInterfaceG12Tgllp::CreateSfcRender(SfcRenderBase *&sfcRender, VP_MHWINTERFACE &vpMhwinterface, PVpAllocator allocator)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(allocator);

    sfcRender = MOS_New(SfcRenderM12,
            vpMhwinterface,
            allocator,
            true);
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

//!
//! \brief    Vebox query statistics surface layout
//! \details  Get Specific Layout Info like GNE Offset, size of per frame info inside
//!           Vebox Statistics Surface for CNL
//!
//!           | Layout of Statistics surface when DI enabled and DN either On or Off on CNL\n
//!           |     --------------------------------------------------------------\n
//!           |     | 16 bytes for x=0, Y=0       | 16 bytes for x=16, Y=0       | ...\n
//!           |     |-------------------------------------------------------------\n
//!           |     | 16 bytes for x=0, Y=4       | ...\n
//!           |     |------------------------------\n
//!           |     | ...\n
//!           |     |------------------------------\n
//!           |     | 16 bytes for x=0, Y=height-4| ...\n
//!           |     |-----------------------------------------------Pitch--------------\n
//!           |     | 17 DW Reserved         | 2 DW STD0 | 2 DW GCC0 | 11 DW Reserved |\n
//!           |     |------------------------------------------------------------------\n
//!           |     | 11 DW FMD0 | 6 DW GNE0 | 2 DW STD0 | 2 DW GCC0 | 11 DW Reserved |\n
//!           |     |------------------------------------------------------------------\n
//!           |     | 17 DW Reserved         | 2 DW STD1 | 2 DW GCC1 | 11 DW Reserved |\n
//!           |     |------------------------------------------------------------------\n
//!           |     | 11 DW FMD1 | 6 DW GNE1 | 2 DW STD1 | 2 DW GCC1 | 11 DW Reserved |\n
//!           |     -------------------------------------------------------------------\n
//!           |\n
//!           | Layout of Statistics surface when DN enabled and DI disabled\n
//!           |     --------------------------------------------------------------\n
//!           |     | 16 bytes for x=0, Y=0       | 16 bytes for x=16, Y=0       | ...\n
//!           |     |-------------------------------------------------------------\n
//!           |     | 16 bytes for x=0, Y=4       | ...\n
//!           |     |------------------------------\n
//!           |     | ...\n
//!           |     |------------------------------\n
//!           |     | 16 bytes for x=0, Y=height-4| ...\n
//!           |     |-----------------------------------------------Pitch--------------\n
//!           |     | 11 DW FMD0 | 6 DW GNE0 | 2 DW STD0 | 2 DW GCC0 | 11 DW Reserved |\n
//!           |     |------------------------------------------------------------------\n
//!           |     | 11 DW FMD1 | 6 DW GNE1 | 2 DW STD1 | 2 DW GCC1 | 11 DW Reserved |\n
//!           |     -------------------------------------------------------------------\n
//!           |\n
//!           | Layout of Statistics surface when both DN and DI are disabled\n
//!           |     ------------------------------------------------Pitch--------------\n
//!           |     | 17 DW White Balence0   | 2 DW STD0 | 2 DW GCC0 | 11 DW Reserved |\n
//!           |     |------------------------------------------------------------------\n
//!           |     | 17 DW White Balence1   | 2 DW STD1 | 2 DW GCC1 | 11 DW Reserved |\n
//!           |     -------------------------------------------------------------------\n
//! \param    [in] QueryType
//!           Query type
//! \param    [out] pQuery
//!           return layout type
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS vp::VpPlatformInterfaceG12Tgllp::VeboxQueryStatLayout(VEBOX_STAT_QUERY_TYPE queryType, uint32_t* pQuery)
{
    VP_FUNC_CALL();

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

MOS_STATUS VpPlatformInterfaceG12Tgllp::GetInputFrameWidthHeightAlignUnit(
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

MOS_STATUS VpPlatformInterfaceG12Tgllp::GetVeboxHeapInfo(
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

bool VpPlatformInterfaceG12Tgllp::IsVeboxScalabilityWith4KNotSupported(
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

MOS_STATUS VpPlatformInterfaceG12Tgllp::ConfigureVpScalability(VP_MHWINTERFACE &vpMhwInterface)
{
    VP_FUNC_CALL();

    vpMhwInterface.pfnCreateSinglePipe = vp::VpScalabilitySinglePipe::CreateSinglePipe;
    vpMhwInterface.pfnCreateMultiPipe  = vp::VpScalabilityMultiPipe::CreateMultiPipe;

    return MOS_STATUS_SUCCESS;
}
