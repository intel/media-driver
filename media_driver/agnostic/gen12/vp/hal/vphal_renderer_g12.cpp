/*
* Copyright (c) 2017-2019, Intel Corporation
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
//! \file     vphal_renderer_g12.cpp
//! \brief    VPHAL top level rendering component and the entry to low level renderers
//! \details  The top renderer is responsible for coordinating the sequence of calls to low level renderers, e.g. DNDI or Comp
//!
#include "vphal_renderer_g12.h"
#include "vphal_render_vebox_g12_base.h"
#include "vphal_render_composite_g12.h"

// May need update the cache policy once GmmCachePolicyGen11.h is created
void VphalRendererG12::GetCacheCntl(
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

        VPHAL_SET_SURF_MEMOBJCTL(pSettings->Composite.PrimaryInputSurfMemObjCtl,   MOS_MP_RESOURCE_USAGE_SurfaceState);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->Composite.InputSurfMemObjCtl,          MOS_MP_RESOURCE_USAGE_SurfaceState);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->Composite.TargetSurfMemObjCtl,         MOS_MP_RESOURCE_USAGE_DEFAULT);
    }
    if (pSettings->bDnDi)
    {
        pSettings->DnDi.bL3CachingEnabled = true;

        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.CurrentInputSurfMemObjCtl,        MOS_MP_RESOURCE_USAGE_SurfaceState);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.PreviousInputSurfMemObjCtl,       MOS_MP_RESOURCE_USAGE_SurfaceState);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.STMMInputSurfMemObjCtl,           MOS_MP_RESOURCE_USAGE_SurfaceState);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.STMMOutputSurfMemObjCtl,          MOS_MP_RESOURCE_USAGE_SurfaceState);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.DnOutSurfMemObjCtl,               MOS_MP_RESOURCE_USAGE_SurfaceState);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.CurrentOutputSurfMemObjCtl,       MOS_MP_RESOURCE_USAGE_SurfaceState);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.StatisticsOutputSurfMemObjCtl,    MOS_MP_RESOURCE_USAGE_SurfaceState);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.AlphaOrVignetteSurfMemObjCtl,     MOS_MP_RESOURCE_USAGE_SurfaceState);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.LaceOrAceOrRgbHistogramSurfCtrl,  MOS_MP_RESOURCE_USAGE_SurfaceState);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.SkinScoreSurfMemObjCtl,           MOS_MP_RESOURCE_USAGE_SurfaceState);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.LaceLookUpTablesSurfMemObjCtl,    MOS_MP_RESOURCE_USAGE_SurfaceState);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.Vebox3DLookUpTablesSurfMemObjCtl, MOS_MP_RESOURCE_USAGE_SurfaceState);
    }
    if (pSettings->bLace)
    {
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->Lace.FrameHistogramSurfaceMemObjCtl,                       MOS_MP_RESOURCE_USAGE_SurfaceState);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->Lace.AggregatedHistogramSurfaceMemObjCtl,                  MOS_MP_RESOURCE_USAGE_SurfaceState);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->Lace.StdStatisticsSurfaceMemObjCtl,                        MOS_MP_RESOURCE_USAGE_SurfaceState);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->Lace.PwlfInSurfaceMemObjCtl,                               MOS_MP_RESOURCE_USAGE_SurfaceState);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->Lace.PwlfOutSurfaceMemObjCtl,                              MOS_MP_RESOURCE_USAGE_SurfaceState);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->Lace.WeitCoefSurfaceMemObjCtl,                             MOS_MP_RESOURCE_USAGE_SurfaceState);
    }
}

MOS_STATUS VphalRendererG12::AllocateRenderComponents(
    PMHW_VEBOX_INTERFACE                pVeboxInterface,
    PMHW_SFC_INTERFACE                  pSfcInterface)
{
    MOS_STATUS              eStatus;
    VPHAL_RENDER_CACHE_CNTL CacheCntl;

    VPHAL_RENDER_CHK_NULL_RETURN(m_pRenderHal);

    eStatus = MOS_STATUS_SUCCESS;

    // Get the cache settings
    MOS_ZeroMemory(&CacheCntl, sizeof(CacheCntl));

    CacheCntl.bDnDi        = true;
    CacheCntl.bCompositing = true;

    VPHAL_RENDERER_GET_CACHE_CNTL(this,
        m_pOsInterface,
        &m_pRenderHal->Platform,
        m_pSkuTable,
        &CacheCntl);

    // Initialize Advanced Processing Interface
    pRender[VPHAL_RENDER_ID_VEBOX] = MOS_New(
        VPHAL_VEBOX_STATE_G12_BASE,
        m_pOsInterface,
        pVeboxInterface,
        pSfcInterface,
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
        VPHAL_VEBOX_STATE_G12_BASE,
        m_pOsInterface,
        pVeboxInterface,
        pSfcInterface,
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
        CompositeStateG12,
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

    return eStatus;
}
