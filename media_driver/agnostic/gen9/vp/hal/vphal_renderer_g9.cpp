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
//! \file     vphal_renderer_g9.cpp
//! \brief    VPHAL top level rendering component and the entry to low level renderers
//! \details  The top renderer is responsible for coordinating the sequence of calls to low level renderers, e.g. DNDI or Comp
//!
#include "vphal_renderer_g9.h"
#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
#include "igvpkrn_g9.h"
#endif
#include "vphal_render_vebox_g9_base.h"
#include "vphal_render_composite_g9.h"

extern const Kdll_RuleEntry         g_KdllRuleTable_g9[];

void VphalRendererG9::GetCacheCntl(
    PMOS_INTERFACE                      pOsInterface,
    PLATFORM                            *pPlatform,
    MEDIA_FEATURE_TABLE                 *pSkuTable,
    PVPHAL_RENDER_CACHE_CNTL            pSettings)

{
    MOS_HW_RESOURCE_DEF                 Usage;
    MEMORY_OBJECT_CONTROL_STATE         MemObjCtrl;

    if (pSettings->bCompositing)
    {
        pSettings->Composite.bL3CachingEnabled = true;

        VPHAL_SET_SURF_MEMOBJCTL(pSettings->Composite.PrimaryInputSurfMemObjCtl,    MOS_MP_RESOURCE_USAGE_SurfaceState);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->Composite.InputSurfMemObjCtl,           MOS_MP_RESOURCE_USAGE_SurfaceState);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->Composite.TargetSurfMemObjCtl,          MOS_MP_RESOURCE_USAGE_No_LLC_L3_AGE_SurfaceState);
    }
    if (pSettings->bDnDi)
    {
        pSettings->DnDi.bL3CachingEnabled = true;

        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.CurrentInputSurfMemObjCtl,         MOS_MP_RESOURCE_USAGE_No_L3_SurfaceState);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.PreviousInputSurfMemObjCtl,        MOS_MP_RESOURCE_USAGE_No_L3_SurfaceState);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.STMMInputSurfMemObjCtl,            MOS_MP_RESOURCE_USAGE_No_L3_SurfaceState);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.STMMOutputSurfMemObjCtl,           MOS_MP_RESOURCE_USAGE_No_LLC_L3_SurfaceState);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.DnOutSurfMemObjCtl,                MOS_MP_RESOURCE_USAGE_No_LLC_L3_SurfaceState);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.CurrentOutputSurfMemObjCtl,        MOS_MP_RESOURCE_USAGE_No_LLC_L3_SurfaceState);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.StatisticsOutputSurfMemObjCtl,     MOS_MP_RESOURCE_USAGE_No_LLC_L3_SurfaceState);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.AlphaOrVignetteSurfMemObjCtl,      MOS_MP_RESOURCE_USAGE_No_LLC_L3_AGE_SurfaceState);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.LaceOrAceOrRgbHistogramSurfCtrl,   MOS_MP_RESOURCE_USAGE_SurfaceState);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.SkinScoreSurfMemObjCtl,            MOS_MP_RESOURCE_USAGE_SurfaceState);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.LaceLookUpTablesSurfMemObjCtl,     MOS_MP_RESOURCE_USAGE_SurfaceState);
    }
    if (pSettings->bLace)
    {
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->Lace.FrameHistogramSurfaceMemObjCtl,        MOS_MP_RESOURCE_USAGE_AGE3_SurfaceState);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->Lace.AggregatedHistogramSurfaceMemObjCtl,   MOS_MP_RESOURCE_USAGE_AGE3_SurfaceState);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->Lace.StdStatisticsSurfaceMemObjCtl,         MOS_MP_RESOURCE_USAGE_AGE3_SurfaceState);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->Lace.PwlfInSurfaceMemObjCtl,                MOS_MP_RESOURCE_USAGE_AGE3_SurfaceState);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->Lace.PwlfOutSurfaceMemObjCtl,               MOS_MP_RESOURCE_USAGE_AGE3_SurfaceState);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->Lace.WeitCoefSurfaceMemObjCtl,              MOS_MP_RESOURCE_USAGE_AGE3_SurfaceState);
    }
}

MOS_STATUS VphalRendererG9::AllocateRenderComponents(
    PMHW_VEBOX_INTERFACE                pVeboxInterface,
    PMHW_SFC_INTERFACE                  pSfcInterface)
{
    MOS_STATUS              eStatus;
    PMHW_SFC_INTERFACE      pParamSfcInterface;
    VPHAL_RENDER_CACHE_CNTL CacheCntl;

    VPHAL_RENDER_CHK_NULL_RETURN(m_pRenderHal);

    eStatus = MOS_STATUS_SUCCESS;

#if __VPHAL_SFC_SUPPORTED
    pParamSfcInterface = pSfcInterface;
#else
    // Disable SFC SKU as it is not supported in this build
    MEDIA_WR_SKU(m_pRenderHal->pSkuTable, FtrSFCPipe, 0);
    pParamSfcInterface = nullptr;
#endif

    // Get the cache settings
    MOS_ZeroMemory(&CacheCntl, sizeof(CacheCntl));

    CacheCntl.bDnDi         = true;
    CacheCntl.bCompositing  = true;
    VPHAL_RENDERER_GET_CACHE_CNTL(this,
        m_pOsInterface,
        &m_pRenderHal->Platform,
        m_pSkuTable,
        &CacheCntl);

    // Initialize Advanced Processing Interface
    pRender[VPHAL_RENDER_ID_VEBOX] = MOS_New(
        VPHAL_VEBOX_STATE_G9_BASE,
        m_pOsInterface,
        pVeboxInterface,
        pParamSfcInterface,
        m_pRenderHal,
        &VeboxExecState[0],
        &PerfData,
        CacheCntl.DnDi,
        &eStatus);
    if (!pRender[VPHAL_RENDER_ID_VEBOX] ||
        (eStatus != MOS_STATUS_SUCCESS))
    {
        eStatus = MOS_STATUS_NO_SPACE;
        VPHAL_RENDER_ASSERTMESSAGE("Allocate Vebox Render Fail.");
        return eStatus;
    }

    pRender[VPHAL_RENDER_ID_VEBOX2] = MOS_New(
        VPHAL_VEBOX_STATE_G9_BASE,
        m_pOsInterface,
        pVeboxInterface,
        pParamSfcInterface,
        m_pRenderHal,
        &VeboxExecState[1],
        &PerfData,
        CacheCntl.DnDi,
        &eStatus);
    if (!pRender[VPHAL_RENDER_ID_VEBOX2] ||
        (eStatus != MOS_STATUS_SUCCESS))
    {
        eStatus = MOS_STATUS_NO_SPACE;
        VPHAL_RENDER_ASSERTMESSAGE("Allocate Vebox Render Fail.");
        return eStatus;
    }

    // Allocate Composite State
    pRender[VPHAL_RENDER_ID_COMPOSITE] = MOS_New(
        CompositeStateG9,
        m_pOsInterface,
        m_pRenderHal,
        &PerfData,
        CacheCntl.Composite,
        &eStatus);
    if (!pRender[VPHAL_RENDER_ID_COMPOSITE] ||
        (eStatus != MOS_STATUS_SUCCESS))
    {
        eStatus = MOS_STATUS_NO_SPACE;
        VPHAL_RENDER_ASSERTMESSAGE("Allocate Composite Render Fail.");
        return eStatus;
    }

    if (MEDIA_IS_SKU(m_pRenderHal->pSkuTable, FtrHDR))
    {
        pHdrState = MOS_New(VPHAL_HDR_STATE);
        if (pHdrState)
        {
            MOS_ZeroMemory(pHdrState, sizeof(VPHAL_HDR_STATE));
            VpHal_HdrInitInterface(pHdrState, m_pRenderHal);
        }
    }

    return eStatus;
}

MOS_STATUS VphalRendererG9::InitKdllParam()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
    pKernelDllRules     = g_KdllRuleTable_g9;
    pcKernelBin         = (const void*)IGVPKRN_G9;
    dwKernelBinSize     = IGVPKRN_G9_SIZE;
#endif

    return eStatus;
}
