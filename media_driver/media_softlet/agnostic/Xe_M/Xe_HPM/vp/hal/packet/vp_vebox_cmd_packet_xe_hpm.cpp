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
//! \file     vp_vebox_cmd_packet_xe_xpm.cpp
//! \brief    vebox packet which used in by mediapipline.
//! \details  vebox packet provide the structures and generate the cmd buffer which mediapipline will used.
//!

#include "vp_vebox_cmd_packet_xe_hpm.h"
#include "vp_hal_ddi_utils.h"
#include "mhw_sfc_xe_xpm.h"
#include "mhw_vebox_xe_hpm.h"

using namespace vp;

VpVeboxCmdPacketXe_Hpm::VpVeboxCmdPacketXe_Hpm(MediaTask * task, PVP_MHWINTERFACE hwInterface, PVpAllocator &allocator, VPMediaMemComp *mmc, bool disbaleSfcDithering) :
    CmdPacket(task),
    VpCmdPacket(task, hwInterface, allocator, mmc, VP_PIPELINE_PACKET_VEBOX),
    VpVeboxCmdPacketBase(task, hwInterface, allocator, mmc),
    VpVeboxCmdPacketLegacy(task, hwInterface, allocator, mmc),
    VpVeboxCmdPacketG12(task, hwInterface, allocator, mmc),
    VpVeboxCmdPacketXe_Xpm_Base(task, hwInterface, allocator, mmc, disbaleSfcDithering)
{
    MOS_ZeroMemory(&veboxChromaParams, sizeof(veboxChromaParams));
}

VpVeboxCmdPacketXe_Hpm::~VpVeboxCmdPacketXe_Hpm()
{
}

MOS_STATUS VpVeboxCmdPacketXe_Hpm::QueryStatLayoutGNE(
    VEBOX_STAT_QUERY_TYPE QueryType,
    uint32_t             *pQuery,
    uint8_t              *pStatSlice0Base,
    uint8_t              *pStatSlice1Base)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(pQuery);
    // Query platform dependent GNE offset
    VP_PUBLIC_CHK_STATUS_RETURN(QueryStatLayout(
        VEBOX_STAT_QUERY_GNE_OFFEST,
        pQuery));

    // check TGNE is valid or not.
    VP_PUBLIC_CHK_STATUS_RETURN(CheckTGNEValid(
        (uint32_t *)(pStatSlice0Base + *pQuery),
        (uint32_t *)(pStatSlice1Base + *pQuery),
        pQuery));

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Add vebox dndi state
//! \details  Add vebox dndi state
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpVeboxCmdPacketXe_Hpm::AddVeboxDndiState()
{
    VP_FUNC_CALL();

    PMHW_VEBOX_INTERFACE  pVeboxInterface = m_hwInterface->m_veboxInterface;
    VpVeboxRenderData    *renderData     = GetLastExecRenderData();

    if (renderData->DN.bDnEnabled || renderData->DI.bDeinterlace || renderData->DI.bQueryVariance)
    {
        auto pVeboxInterfaceHPM = static_cast<MhwVeboxInterfaceG12 *>(pVeboxInterface);
        VP_RENDER_CHK_STATUS_RETURN(pVeboxInterfaceHPM->SetVeboxChromaParams(&veboxChromaParams));
        return pVeboxInterface->AddVeboxDndiState(&renderData->GetDNDIParams());
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacketXe_Hpm::CheckTGNEValid(
    uint32_t *pStatSlice0GNEPtr,
    uint32_t *pStatSlice1GNEPtr,
    uint32_t *pQuery)
{
    VP_FUNC_CALL();
    MOS_STATUS                eStatus            = MOS_STATUS_SUCCESS;
    uint32_t                  bGNECountLumaValid = 0;
    uint32_t                  dwTGNEoffset       = 0;
    VP_PACKET_SHARED_CONTEXT *sharedContext      = (VP_PACKET_SHARED_CONTEXT *)m_packetSharedContext;

    VP_PUBLIC_CHK_NULL_RETURN(sharedContext);

    auto &tgneParams = sharedContext->tgneParams;
    dwTGNEoffset     = (VP_VEBOX_STATISTICS_SURFACE_TGNE_OFFSET - VP_VEBOX_STATISTICS_SURFACE_GNE_OFFSET) / 4;

    if (m_bTgneEnable)
    {
        bGNECountLumaValid = (pStatSlice0GNEPtr[dwTGNEoffset + 3] & 0x80000000) || (pStatSlice1GNEPtr[dwTGNEoffset + 3] & 0x80000000);

        VP_PUBLIC_NORMALMESSAGE("TGNE:bGNECountLumaValid %x", bGNECountLumaValid);

        if (bGNECountLumaValid)
        {
            *pQuery      = VP_VEBOX_STATISTICS_SURFACE_TGNE_OFFSET;
            m_bTgneValid = true;

            if (tgneParams.bTgneFirstFrame)
            {
                tgneParams.bTgneFirstFrame = false;
            }
        }
        else
        {
            *pQuery      = VP_VEBOX_STATISTICS_SURFACE_GNE_OFFSET;
            m_bTgneValid = false;
        }
    }
    else
    {
        *pQuery                    = VP_VEBOX_STATISTICS_SURFACE_GNE_OFFSET;
        m_bTgneValid               = false;
        tgneParams.bTgneFirstFrame = true;
    }

    VP_PUBLIC_NORMALMESSAGE("TGNE:bTGNEValid %x", m_bTgneValid);

    return eStatus;
}

MOS_STATUS VpVeboxCmdPacketXe_Hpm::GNELumaConsistentCheck(
    uint32_t &dwGNELuma,
    uint32_t *pStatSlice0GNEPtr,
    uint32_t *pStatSlice1GNEPtr)
{
    uint32_t dwGNEChromaU = 0, dwGNEChromaV = 0;
    uint32_t dwGNECountChromaU = 0, dwGNECountChromaV = 0;
    VP_PUBLIC_CHK_NULL_RETURN(pStatSlice0GNEPtr);
    VP_PUBLIC_CHK_NULL_RETURN(pStatSlice1GNEPtr);

    // Combine the GNE in slice0 and slice1 to generate the global GNE and Count
    dwGNEChromaU = pStatSlice0GNEPtr[1] + pStatSlice1GNEPtr[1];
    dwGNEChromaV = pStatSlice0GNEPtr[2] + pStatSlice1GNEPtr[2];

    dwGNECountChromaU = pStatSlice0GNEPtr[4] + pStatSlice1GNEPtr[4];
    dwGNECountChromaV = pStatSlice0GNEPtr[5] + pStatSlice1GNEPtr[5];

    // Validate GNE
    if (dwGNEChromaU == 0xFFFFFFFF || dwGNECountChromaU == 0xFFFFFFFF ||
        dwGNEChromaV == 0xFFFFFFFF || dwGNECountChromaV == 0xFFFFFFFF)
    {
        VP_RENDER_ASSERTMESSAGE("Incorrect GNE / GNE count.");
        return MOS_STATUS_UNKNOWN;
    }

    dwGNEChromaU = dwGNEChromaU * 100 / (dwGNECountChromaU + 1);
    dwGNEChromaV = dwGNEChromaV * 100 / (dwGNECountChromaV + 1);
    VP_RENDER_NORMALMESSAGE("Consistent Check: dwGNEChromaU %d  dwGNEChromaV %d", dwGNEChromaU, dwGNEChromaV);
    if ((dwGNEChromaU < NOSIE_GNE_CHROMA_THRESHOLD) &&
        (dwGNEChromaV < NOSIE_GNE_CHROMA_THRESHOLD) &&
        (dwGNEChromaU != 0) &&
        (dwGNEChromaV != 0) &&
        (dwGNELuma > NOSIE_GNE_LUMA_THRESHOLD))
    {
        dwGNELuma = dwGNELuma >> 2;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacketXe_Hpm::UpdateDnHVSParameters(
    uint32_t *pStatSlice0GNEPtr,
    uint32_t *pStatSlice1GNEPtr)
{
    VP_FUNC_CALL();

    uint32_t                      dwGNELuma = 0, dwGNEChromaU = 0, dwGNEChromaV = 0;
    uint32_t                      dwSGNELuma = 0, dwSGNEChromaU = 0, dwSGNEChromaV = 0;
    uint32_t                      dwGNECountLuma = 0, dwGNECountChromaU = 0, dwGNECountChromaV = 0;
    uint32_t                      dwSGNECountLuma = 0, dwSGNECountChromaU = 0, dwSGNECountChromaV;
    uint32_t                      resltn         = 0;
    int32_t                       sgne_offset    = 0;
    MhwVeboxInterfaceXe_Hpm      *veboxInterface = nullptr;
    VP_PACKET_SHARED_CONTEXT     *sharedContext  = (VP_PACKET_SHARED_CONTEXT *)m_packetSharedContext;
    VpVeboxRenderData            *renderData     = GetLastExecRenderData();
    VP_PUBLIC_CHK_NULL_RETURN(renderData);
    VP_PUBLIC_CHK_NULL_RETURN(sharedContext);
    VP_PUBLIC_CHK_NULL_RETURN(m_hwInterface);
    VP_PUBLIC_CHK_NULL_RETURN(m_hwInterface->m_veboxInterface);
    VP_PUBLIC_CHK_NULL_RETURN(m_hwInterface->m_waTable);
    VP_PUBLIC_CHK_NULL_RETURN(m_currentSurface);
    VP_PUBLIC_CHK_NULL_RETURN(m_currentSurface->osSurface);
    auto &tgneParams = sharedContext->tgneParams;
    auto &hvsParams  = sharedContext->hvsParams;
    veboxInterface   = static_cast<MhwVeboxInterfaceXe_Hpm *>(m_hwInterface->m_veboxInterface);
    resltn           = m_currentSurface->rcSrc.right * m_currentSurface->rcSrc.bottom;

    // Set HVS kernel params
    hvsParams.Format            = 0;
    hvsParams.Mode              = renderData->GetHVSParams().Mode;
    hvsParams.Fallback          = !m_bTgneValid && !sharedContext->isVeboxFirstFrame && !tgneParams.bTgneFirstFrame;
    hvsParams.EnableChroma      = renderData->DN.bChromaDnEnabled;
    hvsParams.FirstFrame        = sharedContext->isVeboxFirstFrame;
    hvsParams.TgneFirstFrame    = !sharedContext->isVeboxFirstFrame && tgneParams.bTgneFirstFrame;
    hvsParams.EnableTemporalGNE = m_bTgneEnable;
    hvsParams.Width             = (uint16_t)m_currentSurface->rcSrc.right;
    hvsParams.Height            = (uint16_t)m_currentSurface->rcSrc.bottom;

    // Set QP
    if (renderData->GetHVSParams().Mode == HVSDENOISE_MANUAL)
    {
        if (renderData->GetHVSParams().QP <= 18)
        {
            hvsParams.QP = 0;
        }
        else if (renderData->GetHVSParams().QP <= 22)
        {
            hvsParams.QP = 1;
        }
        else if (renderData->GetHVSParams().QP <= 27)
        {
            hvsParams.QP = 2;
        }
        else if (renderData->GetHVSParams().QP <= 32)
        {
            hvsParams.QP = 3;
        }
        else if (renderData->GetHVSParams().QP <= 37)
        {
            hvsParams.QP = 4;
        }
    }
    else
    {
        hvsParams.QP = renderData->GetHVSParams().QP;
    }

    // Set part HVS Mhw interface
    veboxInterface->bHVSAutoBdrateEnable     = renderData->GetHVSParams().Mode == HVSDENOISE_AUTO_BDRATE ? true : false;
    veboxInterface->bHVSAutoSubjectiveEnable = renderData->GetHVSParams().Mode == HVSDENOISE_AUTO_SUBJECTIVE ? true : false;

    // Combine the GNE in slice0 and slice1 to generate the global GNE and Count
    dwGNELuma         = pStatSlice0GNEPtr[0] + pStatSlice1GNEPtr[0];
    dwGNEChromaU      = pStatSlice0GNEPtr[1] + pStatSlice1GNEPtr[1];
    dwGNEChromaV      = pStatSlice0GNEPtr[2] + pStatSlice1GNEPtr[2];
    dwGNECountLuma    = pStatSlice0GNEPtr[3] + pStatSlice1GNEPtr[3];
    dwGNECountChromaU = pStatSlice0GNEPtr[4] + pStatSlice1GNEPtr[4];
    dwGNECountChromaV = pStatSlice0GNEPtr[5] + pStatSlice1GNEPtr[5];

    // Validate GNE
    if (dwGNELuma == 0xFFFFFFFF || dwGNECountLuma == 0xFFFFFFFF ||
        dwGNEChromaU == 0xFFFFFFFF || dwGNECountChromaU == 0xFFFFFFFF ||
        dwGNEChromaV == 0xFFFFFFFF || dwGNECountChromaV == 0xFFFFFFF)
    {
        VP_RENDER_ASSERTMESSAGE("Incorrect GNE / GNE count.");
        return MOS_STATUS_UNKNOWN;
    }

    // Set HVS Mode in Mhw interface
    if (renderData->GetHVSParams().Mode == HVSDENOISE_AUTO_BDRATE)
    {
        veboxInterface->bHVSAutoBdrateEnable = true;
        veboxInterface->dwBSDThreshold       = (resltn < NOSIE_GNE_RESOLUTION_THRESHOLD) ? 240 : 135;
    }
    else if (renderData->GetHVSParams().Mode == HVSDENOISE_AUTO_SUBJECTIVE)
    {
        veboxInterface->bHVSAutoSubjectiveEnable = true;
    }
    else
    {
        veboxInterface->bHVSAutoBdrateEnable     = false;
        veboxInterface->bHVSAutoSubjectiveEnable = false;
    }

    if (m_bTgneEnable && !sharedContext->isVeboxFirstFrame && tgneParams.bTgneFirstFrame)  //Second frame
    {
        // Set some mhw params
        veboxInterface->bTGNEEnable    = true;
        veboxInterface->dwLumaStadTh   = 3200;
        veboxInterface->dwChromaStadTh = 1600;
        tgneParams.bTgneFirstFrame     = false;  // next frame bTgneFirstFrame should be false

        if (MEDIA_IS_WA(m_hwInterface->m_waTable, Wa_1609102037) &&
            VpHalDDIUtils::GetSurfaceColorPack(m_currentSurface->osSurface->Format) == VPHAL_COLORPACK_444)
        {
            veboxInterface->dw4X4TGNEThCnt = ((m_currentSurface->osSurface->dwWidth - 32) *
                                                  (m_currentSurface->osSurface->dwHeight - 8)) /
                                              1600;
        }
        else
        {
            veboxInterface->dw4X4TGNEThCnt = ((m_currentSurface->osSurface->dwWidth - 8) *
                                                  (m_currentSurface->osSurface->dwHeight - 8)) /
                                              1600;
        }

        // caculate GNE
        dwGNELuma    = dwGNELuma * 100 / (dwGNECountLuma + 1);
        dwGNEChromaU = dwGNEChromaU * 100 / (dwGNECountChromaU + 1);
        dwGNEChromaV = dwGNEChromaV * 100 / (dwGNECountChromaV + 1);

        if (renderData->GetHVSParams().Mode == HVSDENOISE_AUTO_BDRATE)
        {
            veboxInterface->dwLumaStadTh   = 250;
            veboxInterface->dwChromaStadTh = 250;
            veboxInterface->dwHistoryInit  = 27;
            GNELumaConsistentCheck(dwGNELuma, pStatSlice0GNEPtr, pStatSlice1GNEPtr);
            // caculate temporal GNE
            dwGlobalNoiseLevel_Temporal  = (dwGNELuma + 50) / 100;
            dwGlobalNoiseLevelU_Temporal = (dwGNEChromaU + 50) / 100;
            dwGlobalNoiseLevelV_Temporal = (dwGNEChromaV + 50) / 100;
        }

        hvsParams.Sgne_Level          = dwGNELuma;
        hvsParams.Sgne_LevelU         = dwGNEChromaU;
        hvsParams.Sgne_LevelV         = dwGNEChromaV;
        hvsParams.Sgne_Count          = dwGNECountLuma;
        hvsParams.Sgne_CountU         = dwGNECountChromaU;
        hvsParams.Sgne_CountV         = dwGNECountChromaV;
        hvsParams.PrevNslvTemporal    = -1;
        hvsParams.PrevNslvTemporalU   = -1;
        hvsParams.PrevNslvTemporalV   = -1;
        hvsParams.dwGlobalNoiseLevel  = dwGlobalNoiseLevel_Temporal;
        hvsParams.dwGlobalNoiseLevelU = dwGlobalNoiseLevelU_Temporal;
        hvsParams.dwGlobalNoiseLevelV = dwGlobalNoiseLevelV_Temporal;
    }
    else if (m_bTgneEnable && m_bTgneValid && !sharedContext->isVeboxFirstFrame)  //Middle frame
    {
        dwGNECountLuma    = (pStatSlice0GNEPtr[3] & 0x7FFFFFFF) + (pStatSlice1GNEPtr[3] & 0x7FFFFFFF);
        dwGNECountChromaU = (pStatSlice0GNEPtr[4] & 0x7FFFFFFF) + (pStatSlice1GNEPtr[4] & 0x7FFFFFFF);
        dwGNECountChromaV = (pStatSlice0GNEPtr[5] & 0x7FFFFFFF) + (pStatSlice1GNEPtr[5] & 0x7FFFFFFF);

        sgne_offset        = -12;
        dwSGNELuma         = pStatSlice0GNEPtr[sgne_offset] + pStatSlice1GNEPtr[sgne_offset];
        dwSGNEChromaU      = pStatSlice0GNEPtr[sgne_offset + 1] + pStatSlice1GNEPtr[sgne_offset + 1];
        dwSGNEChromaV      = pStatSlice0GNEPtr[sgne_offset + 2] + pStatSlice1GNEPtr[sgne_offset + 2];
        dwSGNECountLuma    = pStatSlice0GNEPtr[sgne_offset + 3] + pStatSlice1GNEPtr[sgne_offset + 3];
        dwSGNECountChromaU = pStatSlice0GNEPtr[sgne_offset + 4] + pStatSlice1GNEPtr[sgne_offset + 4];
        dwSGNECountChromaV = pStatSlice0GNEPtr[sgne_offset + 5] + pStatSlice1GNEPtr[sgne_offset + 5];

        // Validate TGNE
        if (dwGNECountLuma == 0 || dwGNECountChromaU == 0 || dwGNECountChromaV == 0 ||
            dwSGNELuma == 0xFFFFFFFF || dwSGNECountLuma == 0xFFFFFFFF ||
            dwSGNEChromaU == 0xFFFFFFFF || dwSGNECountChromaU == 0xFFFFFFFF ||
            dwSGNEChromaV == 0xFFFFFFFF || dwSGNECountChromaV == 0xFFFFFFF)
        {
            VP_RENDER_ASSERTMESSAGE("Incorrect GNE count.");
            return MOS_STATUS_UNKNOWN;
        }

        curNoiseLevel_Temporal  = dwGNELuma / dwGNECountLuma;
        curNoiseLevelU_Temporal = dwGNEChromaU / dwGNECountChromaU;
        curNoiseLevelV_Temporal = dwGNEChromaV / dwGNECountChromaV;

        if (veboxInterface->bHVSfallback)
        {
            // last frame is fallback frame, nosie level use current noise level
            dwGlobalNoiseLevel_Temporal  = curNoiseLevel_Temporal;
            dwGlobalNoiseLevelU_Temporal = curNoiseLevelU_Temporal;
            dwGlobalNoiseLevelV_Temporal = curNoiseLevelU_Temporal;
            veboxInterface->bHVSfallback = false;
        }
        else
        {
            // last frame is tgne frame, noise level use previous and current noise level
            dwGlobalNoiseLevel_Temporal  = MOS_ROUNDUP_SHIFT(dwGlobalNoiseLevel_Temporal + curNoiseLevel_Temporal, 1);
            dwGlobalNoiseLevelU_Temporal = MOS_ROUNDUP_SHIFT(dwGlobalNoiseLevelU_Temporal + curNoiseLevelU_Temporal, 1);
            dwGlobalNoiseLevelV_Temporal = MOS_ROUNDUP_SHIFT(dwGlobalNoiseLevelV_Temporal + curNoiseLevelV_Temporal, 1);
        }

        // Set mhw parameters
        veboxInterface->bTGNEEnable    = true;
        veboxInterface->dwLumaStadTh   = (dwGlobalNoiseLevel_Temporal <= 1) ? 3200 : (veboxInterface->dwLumaStadTh + (curNoiseLevel_Temporal << 1) + 1) >> 1;
        veboxInterface->dwChromaStadTh = (dwGlobalNoiseLevelU_Temporal <= 1 || dwGlobalNoiseLevelV_Temporal <= 1) ? 1600 : (veboxInterface->dwChromaStadTh + curNoiseLevelU_Temporal + curNoiseLevelV_Temporal + 1) >> 1;

        if (renderData->GetHVSParams().Mode == HVSDENOISE_AUTO_BDRATE)
        {
            veboxInterface->dwLumaStadTh   = 250;
            veboxInterface->dwChromaStadTh = 250;
            veboxInterface->dwHistoryInit  = 27;
        }

        if (MEDIA_IS_WA(m_hwInterface->m_waTable, Wa_1609102037) &&
            VpHalDDIUtils::GetSurfaceColorPack(m_currentSurface->osSurface->Format) == VPHAL_COLORPACK_444)
        {
            veboxInterface->dw4X4TGNEThCnt = ((m_currentSurface->osSurface->dwWidth - 32) *
                                                 (m_currentSurface->osSurface->dwHeight - 8)) /
                                             1600;
        }
        else
        {
            veboxInterface->dw4X4TGNEThCnt = ((m_currentSurface->osSurface->dwWidth - 8) *
                                                 (m_currentSurface->osSurface->dwHeight - 8)) /
                                             1600;
        }
        hvsParams.Sgne_Level          = dwSGNELuma * 100 / (dwSGNECountLuma + 1);
        hvsParams.Sgne_LevelU         = dwSGNEChromaU * 100 / (dwSGNECountChromaU + 1);
        hvsParams.Sgne_LevelV         = dwSGNEChromaV * 100 / (dwSGNECountChromaV + 1);
        hvsParams.Sgne_Count          = dwSGNECountLuma;
        hvsParams.Sgne_CountU         = dwSGNECountChromaU;
        hvsParams.Sgne_CountV         = dwSGNECountChromaV;
        hvsParams.PrevNslvTemporal    = curNoiseLevel_Temporal;
        hvsParams.PrevNslvTemporalU   = curNoiseLevelU_Temporal;
        hvsParams.PrevNslvTemporalV   = curNoiseLevelV_Temporal;
        hvsParams.dwGlobalNoiseLevel  = dwGlobalNoiseLevel_Temporal;
        hvsParams.dwGlobalNoiseLevelU = dwGlobalNoiseLevelU_Temporal;
        hvsParams.dwGlobalNoiseLevelV = dwGlobalNoiseLevelV_Temporal;
    }
    else  //First frame and fallback frame
    {
        dwGNELuma    = dwGNELuma * 100 / (dwGNECountLuma + 1);
        dwGNEChromaU = dwGNEChromaU * 100 / (dwGNECountChromaU + 1);
        dwGNEChromaV = dwGNEChromaV * 100 / (dwGNECountChromaV + 1);

        GNELumaConsistentCheck(dwGNELuma, pStatSlice0GNEPtr, pStatSlice1GNEPtr);

        // Set some mhw params
        if (hvsParams.Fallback)
        {
            veboxInterface->bHVSfallback  = true;
            veboxInterface->bTGNEEnable   = true;
            veboxInterface->dwHistoryInit = 32;
        }
        else
        {
            if (renderData->GetHVSParams().Mode == HVSDENOISE_AUTO_BDRATE)
            {
                veboxInterface->dwHistoryInit = 32;
            }
            veboxInterface->bTGNEEnable    = false;
            veboxInterface->dwLumaStadTh   = 0;
            veboxInterface->dwChromaStadTh = 0;
            veboxInterface->dw4X4TGNEThCnt = 0;
        }

        hvsParams.Sgne_Level          = dwGNELuma;
        hvsParams.Sgne_LevelU         = dwGNEChromaU;
        hvsParams.Sgne_LevelV         = dwGNEChromaV;
        hvsParams.Sgne_Count          = dwGNECountLuma;
        hvsParams.Sgne_CountU         = dwGNECountChromaU;
        hvsParams.Sgne_CountV         = dwGNECountChromaV;
        hvsParams.PrevNslvTemporal    = -1;
        hvsParams.PrevNslvTemporalU   = -1;
        hvsParams.PrevNslvTemporalV   = -1;
        hvsParams.dwGlobalNoiseLevel  = dwGNELuma;
        hvsParams.dwGlobalNoiseLevelU = dwGNEChromaU;
        hvsParams.dwGlobalNoiseLevelV = dwGNEChromaV;
    }

    if (renderData->GetHVSParams().Mode == HVSDENOISE_MANUAL)
    {
        hvsParams.dwGlobalNoiseLevel  = renderData->GetHVSParams().Strength;
        hvsParams.dwGlobalNoiseLevelU = renderData->GetHVSParams().Strength;
        hvsParams.dwGlobalNoiseLevelV = renderData->GetHVSParams().Strength;
    }

    VP_PUBLIC_NORMALMESSAGE("HVS Kernel params: hvsParams.QP %d, hvsParams.Strength %d, hvsParams.Mode %d, hvsParams.Width %d, hvsParams.Height %d, hvsParams.Format %d", hvsParams.QP, hvsParams.Strength, hvsParams.Mode, hvsParams.Width, hvsParams.Height, hvsParams.Format);
    VP_PUBLIC_NORMALMESSAGE("HVS Kernel params: hvsParams.FirstFrame %d, hvsParams.TgneFirstFrame %d, hvsParams.EnableTemporalGNE %d, hvsParams.Fallback %d, hvsParams.EnableChroma %d", hvsParams.FirstFrame, hvsParams.TgneFirstFrame, hvsParams.EnableTemporalGNE, hvsParams.Fallback, hvsParams.EnableChroma);
    VP_PUBLIC_NORMALMESSAGE("HVS Kernel params: hvsParams.dwGlobalNoiseLevel %d, hvsParams.dwGlobalNoiseLevelU %d, hvsParams.dwGlobalNoiseLevelV %d", hvsParams.dwGlobalNoiseLevel, hvsParams.dwGlobalNoiseLevelU, hvsParams.dwGlobalNoiseLevelV);
    VP_PUBLIC_NORMALMESSAGE("HVS Kernel params: hvsParams.PrevNslvTemporal %d, hvsParams.PrevNslvTemporalU %d, hvsParams.PrevNslvTemporalV %d", hvsParams.PrevNslvTemporal, hvsParams.PrevNslvTemporalU, hvsParams.PrevNslvTemporalV);
    VP_PUBLIC_NORMALMESSAGE("HVS Kernel params: hvsParams.Sgne_Count %d, hvsParams.Sgne_CountU %d, hvsParams.Sgne_CountV %d", hvsParams.Sgne_Count, hvsParams.Sgne_CountU, hvsParams.Sgne_CountV);
    VP_PUBLIC_NORMALMESSAGE("HVS Kernel params: hvsParams.Sgne_Level %d, hvsParams.Sgne_LevelU %d, hvsParams.Sgne_LevelV %d", hvsParams.Sgne_Level, hvsParams.Sgne_LevelU, hvsParams.Sgne_LevelV);
    VP_PUBLIC_NORMALMESSAGE("HVS Kernel params: veboxInterface->dwLumaStadTh %d, veboxInterface->dwChromaStadTh %d", veboxInterface->dwLumaStadTh, veboxInterface->dwChromaStadTh);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacketXe_Hpm::SetupDNTableForHVS(
    PMHW_VEBOX_STATE_CMD_PARAMS veboxStateCmdParams)
{
    VP_FUNC_CALL();
    VpVeboxRenderData                            *renderData        = GetLastExecRenderData();
    PVP_SURFACE                                   surfHVSTable      = GetSurface(SurfaceTypeHVSTable);
    VP_RENDER_CHK_NULL_RETURN(renderData);

    if (nullptr == surfHVSTable || !renderData->DN.bHvsDnEnabled)
    {
        VP_RENDER_NORMALMESSAGE("HVS DN DI table not need be updated.");
        return MOS_STATUS_SUCCESS;
    }

    VP_RENDER_CHK_NULL_RETURN(m_allocator);
    VP_RENDER_CHK_NULL_RETURN(surfHVSTable);
    VP_RENDER_CHK_NULL_RETURN(surfHVSTable->osSurface);

    uint8_t  *bufHVSSurface      = (uint8_t *)m_allocator->LockResourceForWrite(&surfHVSTable->osSurface->OsResource);
    uint32_t *bufHVSDenoiseParam = (uint32_t *)bufHVSSurface;
    VP_RENDER_CHK_NULL_RETURN(bufHVSSurface);
    VP_RENDER_CHK_NULL_RETURN(bufHVSDenoiseParam);

    VP_RENDER_NORMALMESSAGE("Set HVS Denoised Parameters to VEBOX DNDI params");
    // DW0
    renderData->GetDNDIParams().dwDenoiseMaximumHistory = (bufHVSDenoiseParam[0] & 0x000000ff);
    VP_RENDER_NORMALMESSAGE("HVS: renderData->GetDNDIParams().dwDenoiseMaximumHistory %d", renderData->GetDNDIParams().dwDenoiseMaximumHistory);
    renderData->GetDNDIParams().dwDenoiseSTADThreshold = (bufHVSDenoiseParam[0] & 0xfffe0000) >> 17;
    VP_RENDER_NORMALMESSAGE("HVS: renderData->GetDNDIParams().dwDenoiseSTADThreshold %d", renderData->GetDNDIParams().dwDenoiseSTADThreshold);
    // DW1
    renderData->GetDNDIParams().dwDenoiseASDThreshold = (bufHVSDenoiseParam[1] & 0x00000fff);
    VP_RENDER_NORMALMESSAGE("HVS: renderData->GetDNDIParams().dwDenoiseASDThreshold %d", renderData->GetDNDIParams().dwDenoiseASDThreshold);
    renderData->GetDNDIParams().dwDenoiseMPThreshold = (bufHVSDenoiseParam[1] & 0x0f800000) >> 23;
    VP_RENDER_NORMALMESSAGE("HVS: renderData->GetDNDIParams().dwDenoiseMPThreshold %d", renderData->GetDNDIParams().dwDenoiseMPThreshold);
    renderData->GetDNDIParams().dwDenoiseHistoryDelta = (bufHVSDenoiseParam[1] & 0xf0000000) >> 28;
    VP_RENDER_NORMALMESSAGE("HVS: renderData->GetDNDIParams().dwDenoiseHistoryDelta %d", renderData->GetDNDIParams().dwDenoiseHistoryDelta);
    // DW2
    renderData->GetDNDIParams().dwTDThreshold = (bufHVSDenoiseParam[2] & 0xfff00000) >> 20;
    VP_RENDER_NORMALMESSAGE("HVS: renderData->GetDNDIParams().dwTDThreshold %d", renderData->GetDNDIParams().dwTDThreshold);
    // DW3
    renderData->GetDNDIParams().dwLTDThreshold = (bufHVSDenoiseParam[3] & 0xfff00000) >> 20;
    VP_RENDER_NORMALMESSAGE("HVS: renderData->GetDNDIParams().dwLTDThreshold %d", renderData->GetDNDIParams().dwLTDThreshold);
    // DW4
    renderData->GetDNDIParams().dwDenoiseSCMThreshold = (bufHVSDenoiseParam[4] & 0xfff00000) >> 20;
    VP_RENDER_NORMALMESSAGE("HVS: renderData->GetDNDIParams().dwDenoiseSCMThreshold %d", renderData->GetDNDIParams().dwDenoiseSCMThreshold);
    // DW5
    renderData->GetDNDIParams().dwChromaSTADThreshold = (bufHVSDenoiseParam[5] & 0xfffe0000) >> 17;
    VP_RENDER_NORMALMESSAGE("HVS: renderData->GetDNDIParams().dwChromaSTADThreshold %d", renderData->GetDNDIParams().dwChromaSTADThreshold);
    // DW6
    renderData->GetDNDIParams().dwChromaTDThreshold = (bufHVSDenoiseParam[6] & 0xfff00000) >> 20;
    VP_RENDER_NORMALMESSAGE("HVS: renderData->GetDNDIParams().dwChromaTDThreshold %d", renderData->GetDNDIParams().dwChromaTDThreshold);
    // DW7
    renderData->GetDNDIParams().dwChromaLTDThreshold = (bufHVSDenoiseParam[7] & 0xfff00000) >> 20;
    VP_RENDER_NORMALMESSAGE("HVS: renderData->GetDNDIParams().dwChromaLTDThreshold %d", renderData->GetDNDIParams().dwChromaLTDThreshold);
    // DW9
    renderData->GetDNDIParams().dwPixRangeWeight[0] = (bufHVSDenoiseParam[9] & 0x0000001f);
    VP_RENDER_NORMALMESSAGE("HVS: renderData->GetDNDIParams().dwPixRangeWeight[0] %d", renderData->GetDNDIParams().dwPixRangeWeight[0]);
    renderData->GetDNDIParams().dwPixRangeWeight[1] = (bufHVSDenoiseParam[9] & 0x000003e0) >> 5;
    VP_RENDER_NORMALMESSAGE("HVS: renderData->GetDNDIParams().dwPixRangeWeight[1] %d", renderData->GetDNDIParams().dwPixRangeWeight[1]);
    renderData->GetDNDIParams().dwPixRangeWeight[2] = (bufHVSDenoiseParam[9] & 0x00007c00) >> 10;
    VP_RENDER_NORMALMESSAGE("HVS: renderData->GetDNDIParams().dwPixRangeWeight[2] %d", renderData->GetDNDIParams().dwPixRangeWeight[2]);
    renderData->GetDNDIParams().dwPixRangeWeight[3] = (bufHVSDenoiseParam[9] & 0x000f8000) >> 15;
    VP_RENDER_NORMALMESSAGE("HVS: renderData->GetDNDIParams().dwPixRangeWeight[3] %d", renderData->GetDNDIParams().dwPixRangeWeight[3]);
    renderData->GetDNDIParams().dwPixRangeWeight[4] = (bufHVSDenoiseParam[9] & 0x01f00000) >> 20;
    VP_RENDER_NORMALMESSAGE("HVS: renderData->GetDNDIParams().dwPixRangeWeight[4] %d", renderData->GetDNDIParams().dwPixRangeWeight[4]);
    renderData->GetDNDIParams().dwPixRangeWeight[5] = (bufHVSDenoiseParam[9] & 0x3e000000) >> 25;
    VP_RENDER_NORMALMESSAGE("HVS: renderData->GetDNDIParams().dwPixRangeWeight[5] %d", renderData->GetDNDIParams().dwPixRangeWeight[5]);
    // DW11
    renderData->GetDNDIParams().dwPixRangeThreshold[5] = (bufHVSDenoiseParam[11] & 0x1fff0000) >> 16;
    VP_RENDER_NORMALMESSAGE("HVS: renderData->GetDNDIParams().dwPixRangeThreshold[5] %d", renderData->GetDNDIParams().dwPixRangeThreshold[5]);
    // DW12
    renderData->GetDNDIParams().dwPixRangeThreshold[4] = (bufHVSDenoiseParam[12] & 0x1fff0000) >> 16;
    VP_RENDER_NORMALMESSAGE("HVS: renderData->GetDNDIParams().dwPixRangeThreshold[4] %d", renderData->GetDNDIParams().dwPixRangeThreshold[4]);
    renderData->GetDNDIParams().dwPixRangeThreshold[3] = (bufHVSDenoiseParam[12] & 0x00001fff);
    VP_RENDER_NORMALMESSAGE("HVS: renderData->GetDNDIParams().dwPixRangeThreshold[3] %d", renderData->GetDNDIParams().dwPixRangeThreshold[3]);
    // DW13
    renderData->GetDNDIParams().dwPixRangeThreshold[2] = (bufHVSDenoiseParam[13] & 0x1fff0000) >> 16;
    VP_RENDER_NORMALMESSAGE("HVS: renderData->GetDNDIParams().dwPixRangeThreshold[2] %d", renderData->GetDNDIParams().dwPixRangeThreshold[2]);
    renderData->GetDNDIParams().dwPixRangeThreshold[1] = (bufHVSDenoiseParam[13] & 0x00001fff);
    VP_RENDER_NORMALMESSAGE("HVS: renderData->GetDNDIParams().dwPixRangeThreshold[1] %d", renderData->GetDNDIParams().dwPixRangeThreshold[1]);
    // DW14
    renderData->GetDNDIParams().dwPixRangeThreshold[0] = (bufHVSDenoiseParam[14] & 0x1fff0000) >> 16;
    VP_RENDER_NORMALMESSAGE("HVS: renderData->GetDNDIParams().dwPixRangeThreshold[0] %d", renderData->GetDNDIParams().dwPixRangeThreshold[0]);
    // DW16
    veboxChromaParams.dwPixRangeWeightChromaU[0] = (bufHVSDenoiseParam[16] & 0x0000001f);
    VP_RENDER_NORMALMESSAGE("veboxChromaParams.dwPixRangeWeightChromaU[0] %d", veboxChromaParams.dwPixRangeWeightChromaU[0]);
    veboxChromaParams.dwPixRangeWeightChromaU[1] = (bufHVSDenoiseParam[16] & 0x000003e0) >> 5;
    VP_RENDER_NORMALMESSAGE("veboxChromaParams.dwPixRangeWeightChromaU[1] %d", veboxChromaParams.dwPixRangeWeightChromaU[1]);
    veboxChromaParams.dwPixRangeWeightChromaU[2] = (bufHVSDenoiseParam[16] & 0x00007c00) >> 10;
    VP_RENDER_NORMALMESSAGE("veboxChromaParams.dwPixRangeWeightChromaU[2] %d", veboxChromaParams.dwPixRangeWeightChromaU[2]);
    veboxChromaParams.dwPixRangeWeightChromaU[3] = (bufHVSDenoiseParam[16] & 0x000f8000) >> 15;
    VP_RENDER_NORMALMESSAGE("veboxChromaParams.dwPixRangeWeightChromaU[3] %d", veboxChromaParams.dwPixRangeWeightChromaU[3]);
    veboxChromaParams.dwPixRangeWeightChromaU[4] = (bufHVSDenoiseParam[16] & 0x01f00000) >> 20;
    VP_RENDER_NORMALMESSAGE("veboxChromaParams.dwPixRangeWeightChromaU[4] %d", veboxChromaParams.dwPixRangeWeightChromaU[4]);
    veboxChromaParams.dwPixRangeWeightChromaU[5] = (bufHVSDenoiseParam[16] & 0x3e000000) >> 25;
    VP_RENDER_NORMALMESSAGE("veboxChromaParams.dwPixRangeWeightChromaU[5] %d", veboxChromaParams.dwPixRangeWeightChromaU[5]);
    //DW18
    veboxChromaParams.dwPixRangeThresholdChromaU[5] = (bufHVSDenoiseParam[18] & 0x1fff0000) >> 16;
    VP_RENDER_NORMALMESSAGE("veboxChromaParams.dwPixRangeThresholdChromaU[5] %d", veboxChromaParams.dwPixRangeThresholdChromaU[5]);
    //DW19
    veboxChromaParams.dwPixRangeThresholdChromaU[4] = (bufHVSDenoiseParam[19] & 0x1fff0000) >> 16;
    VP_RENDER_NORMALMESSAGE("veboxChromaParams.dwPixRangeThresholdChromaU[4] %d", veboxChromaParams.dwPixRangeThresholdChromaU[4]);
    veboxChromaParams.dwPixRangeThresholdChromaU[3] = (bufHVSDenoiseParam[19] & 0x00001fff);
    VP_RENDER_NORMALMESSAGE("veboxChromaParams.dwPixRangeThresholdChromaU[3] %d", veboxChromaParams.dwPixRangeThresholdChromaU[3]);
    //DW20
    veboxChromaParams.dwPixRangeThresholdChromaU[2] = (bufHVSDenoiseParam[20] & 0x1fff0000) >> 16;
    VP_RENDER_NORMALMESSAGE("veboxChromaParams.dwPixRangeThresholdChromaU[2] %d", veboxChromaParams.dwPixRangeThresholdChromaU[2]);
    veboxChromaParams.dwPixRangeThresholdChromaU[1] = (bufHVSDenoiseParam[20] & 0x00001fff);
    VP_RENDER_NORMALMESSAGE("veboxChromaParams.dwPixRangeThresholdChromaU[1] %d", veboxChromaParams.dwPixRangeThresholdChromaU[1]);
    //DW21
    veboxChromaParams.dwPixRangeThresholdChromaU[0] = (bufHVSDenoiseParam[21] & 0x1fff0000) >> 16;
    VP_RENDER_NORMALMESSAGE("veboxChromaParams.dwPixRangeThresholdChromaU[0] %d", veboxChromaParams.dwPixRangeThresholdChromaU[0]);
    //DW23
    veboxChromaParams.dwPixRangeWeightChromaV[0] = (bufHVSDenoiseParam[23] & 0x0000001f);
    VP_RENDER_NORMALMESSAGE("veboxChromaParams.dwPixRangeWeightChromaV[0] %d", veboxChromaParams.dwPixRangeWeightChromaV[0]);
    veboxChromaParams.dwPixRangeWeightChromaV[1] = (bufHVSDenoiseParam[23] & 0x000003e0) >> 5;
    VP_RENDER_NORMALMESSAGE("veboxChromaParams.dwPixRangeWeightChromaV[1] %d", veboxChromaParams.dwPixRangeWeightChromaV[1]);
    veboxChromaParams.dwPixRangeWeightChromaV[2] = (bufHVSDenoiseParam[23] & 0x00007c00) >> 10;
    VP_RENDER_NORMALMESSAGE("veboxChromaParams.dwPixRangeWeightChromaV[2] %d", veboxChromaParams.dwPixRangeWeightChromaV[2]);
    veboxChromaParams.dwPixRangeWeightChromaV[3] = (bufHVSDenoiseParam[23] & 0x000f8000) >> 15;
    VP_RENDER_NORMALMESSAGE("veboxChromaParams.dwPixRangeWeightChromaV[3] %d", veboxChromaParams.dwPixRangeWeightChromaV[3]);
    veboxChromaParams.dwPixRangeWeightChromaV[4] = (bufHVSDenoiseParam[23] & 0x01f00000) >> 20;
    VP_RENDER_NORMALMESSAGE("veboxChromaParams.dwPixRangeWeightChromaV[4] %d", veboxChromaParams.dwPixRangeWeightChromaV[4]);
    veboxChromaParams.dwPixRangeWeightChromaV[5] = (bufHVSDenoiseParam[23] & 0x3e000000) >> 25;
    VP_RENDER_NORMALMESSAGE("veboxChromaParams.dwPixRangeWeightChromaV[5] %d", veboxChromaParams.dwPixRangeWeightChromaV[5]);
    //DW25
    veboxChromaParams.dwPixRangeThresholdChromaV[5] = (bufHVSDenoiseParam[25] & 0x1fff0000) >> 16;
    VP_RENDER_NORMALMESSAGE("veboxChromaParams.dwPixRangeThresholdChromaV[5] %d", veboxChromaParams.dwPixRangeThresholdChromaV[5]);
    //DW26
    veboxChromaParams.dwPixRangeThresholdChromaV[4] = (bufHVSDenoiseParam[26] & 0x1fff0000) >> 16;
    VP_RENDER_NORMALMESSAGE("veboxChromaParams.dwPixRangeThresholdChromaV[4] %d", veboxChromaParams.dwPixRangeThresholdChromaV[4]);
    veboxChromaParams.dwPixRangeThresholdChromaV[3] = (bufHVSDenoiseParam[26] & 0x00001fff);
    VP_RENDER_NORMALMESSAGE("veboxChromaParams.dwPixRangeThresholdChromaV[3] %d", veboxChromaParams.dwPixRangeThresholdChromaV[3]);
    //DW27
    veboxChromaParams.dwPixRangeThresholdChromaV[2] = (bufHVSDenoiseParam[27] & 0x1fff0000) >> 16;
    VP_RENDER_NORMALMESSAGE("veboxChromaParams.dwPixRangeThresholdChromaV[2] %d", veboxChromaParams.dwPixRangeThresholdChromaV[2]);
    veboxChromaParams.dwPixRangeThresholdChromaV[1] = (bufHVSDenoiseParam[27] & 0x00001fff);
    VP_RENDER_NORMALMESSAGE("veboxChromaParams.dwPixRangeThresholdChromaV[1] %d", veboxChromaParams.dwPixRangeThresholdChromaV[1]);
    //DW28
    veboxChromaParams.dwPixRangeThresholdChromaV[0] = (bufHVSDenoiseParam[28] & 0x1fff0000) >> 16;
    VP_RENDER_NORMALMESSAGE("veboxChromaParams.dwPixRangeThresholdChromaV[0] %d", veboxChromaParams.dwPixRangeThresholdChromaV[0]);
    VP_RENDER_NORMALMESSAGE("amdedNosieLevelSpatial %d", bufHVSDenoiseParam[32]);

    VP_PUBLIC_CHK_STATUS_RETURN(m_allocator->UnLock(&surfHVSTable->osSurface->OsResource));
  
    //Set up the Vebox State in Clear Memory
    VP_PUBLIC_CHK_STATUS_RETURN(AddVeboxDndiState());

    return MOS_STATUS_SUCCESS;
}
