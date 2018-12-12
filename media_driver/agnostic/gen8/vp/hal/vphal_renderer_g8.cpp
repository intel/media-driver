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
//! \file     vphal_renderer_g8.cpp
//! \brief    VPHAL top level rendering component and the entry to low level renderers
//! \details  The top renderer is responsible for coordinating the sequence of calls to low level renderers, e.g. DNDI or Comp
//!
#include "vphal_renderer_g8.h"
#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
#include "igvpkrn_g8.h"
#endif
#include "vphal_render_vebox_g8_base.h"
#include "vphal_render_composite_g8.h"
#include "renderhal_g8.h"

extern const Kdll_RuleEntry         g_KdllRuleTable_g8[];

// Platform restriction on BDW: Check if the BDW SKU doesn't have EDRAM
// Currently on BDW, if the surface is to be cached in eLLC and
// the HW SKU doesn't have eLLC then it gets cached in LLC.
// That causes thrashing in LLC,
// so need to make such surfaces uncachable if eLLC/eDRAM is not present.

#define VPHAL_SET_SURF_MEMOBJCTL_GEN8(VpField, GmmUsageEnum)                                                    \
    {                                                                                                           \
        Usage = GmmUsageEnum;                                                                                   \
        MemObjCtrl = pOsInterface->pfnCachePolicyGetMemoryObject(Usage, pOsInterface->pfnGetGmmClientContext(pOsInterface));                                        \
        do                                                                                                      \
        {                                                                                                       \
            if (MemObjCtrl.Gen8.TargetCache == RENDERHAL_MO_TARGET_CACHE_ELLC_G8)                               \
            {                                                                                                   \
                if (!MEDIA_IS_SKU(pSkuTable, FtrEDram))                                                      \
                {                                                                                               \
                    MemObjCtrl.Gen8.CacheControl = RENDERHAL_MO_CACHE_CONTROL_UC_G8;                            \
                }                                                                                               \
            }                                                                                                   \
        } while(0);                                                                                             \
                                                                                                                \
        VpField = MemObjCtrl.DwordValue;                                                                        \
    }

void VphalRendererG8::GetCacheCntl(
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

        VPHAL_SET_SURF_MEMOBJCTL_GEN8(pSettings->Composite.PrimaryInputSurfMemObjCtl,     MOS_MP_RESOURCE_USAGE_SurfaceState);
        VPHAL_SET_SURF_MEMOBJCTL_GEN8(pSettings->Composite.InputSurfMemObjCtl,            MOS_MP_RESOURCE_USAGE_SurfaceState);
        VPHAL_SET_SURF_MEMOBJCTL_GEN8(pSettings->Composite.TargetSurfMemObjCtl,           MOS_MP_RESOURCE_USAGE_DEFAULT);
    }
    if (pSettings->bDnDi)
    {
        pSettings->DnDi.bL3CachingEnabled = true;

        VPHAL_SET_SURF_MEMOBJCTL_GEN8(pSettings->DnDi.CurrentInputSurfMemObjCtl,          MOS_MP_RESOURCE_USAGE_No_L3_SurfaceState);
        VPHAL_SET_SURF_MEMOBJCTL_GEN8(pSettings->DnDi.PreviousInputSurfMemObjCtl,         MOS_MP_RESOURCE_USAGE_No_L3_SurfaceState);
        VPHAL_SET_SURF_MEMOBJCTL_GEN8(pSettings->DnDi.STMMInputSurfMemObjCtl,             MOS_MP_RESOURCE_USAGE_No_L3_SurfaceState);
        VPHAL_SET_SURF_MEMOBJCTL_GEN8(pSettings->DnDi.STMMOutputSurfMemObjCtl,            MOS_MP_RESOURCE_USAGE_No_LLC_L3_SurfaceState);
        VPHAL_SET_SURF_MEMOBJCTL_GEN8(pSettings->DnDi.DnOutSurfMemObjCtl,                 MOS_MP_RESOURCE_USAGE_No_LLC_L3_SurfaceState);
        VPHAL_SET_SURF_MEMOBJCTL_GEN8(pSettings->DnDi.CurrentOutputSurfMemObjCtl,         MOS_MP_RESOURCE_USAGE_No_LLC_L3_SurfaceState);
        VPHAL_SET_SURF_MEMOBJCTL_GEN8(pSettings->DnDi.StatisticsOutputSurfMemObjCtl,      MOS_MP_RESOURCE_USAGE_No_LLC_L3_SurfaceState);
        VPHAL_SET_SURF_MEMOBJCTL_GEN8(pSettings->DnDi.AlphaOrVignetteSurfMemObjCtl,       MOS_MP_RESOURCE_USAGE_DEFAULT);
    }
    if (pSettings->bLace)
    {
        // No cache setting
    }
}

MOS_STATUS VphalRendererG8::AllocateRenderComponents(
    PMHW_VEBOX_INTERFACE                pVeboxInterface,
    PMHW_SFC_INTERFACE                  pSfcInterface)
{
    MOS_STATUS              eStatus;
    VPHAL_RENDER_CACHE_CNTL CacheCntl;

    VPHAL_RENDER_CHK_NULL_RETURN(m_pRenderHal);

    eStatus = MOS_STATUS_SUCCESS;

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
    pRender[VPHAL_RENDER_ID_VEBOX]  = MOS_New(
        VPHAL_VEBOX_STATE_G8_BASE,
        m_pOsInterface,
        pVeboxInterface,
        m_pRenderHal,
        &VeboxExecState[0],
        &PerfData,
        CacheCntl.DnDi,
        &eStatus);
    if (!pRender[VPHAL_RENDER_ID_VEBOX]     ||
        (eStatus != MOS_STATUS_SUCCESS))
    {
        eStatus = MOS_STATUS_NO_SPACE;
        VPHAL_RENDER_ASSERTMESSAGE("Allocate Vebox Render Fail.");
        return eStatus;
    }

    pRender[VPHAL_RENDER_ID_VEBOX2] = MOS_New(
        VPHAL_VEBOX_STATE_G8_BASE,
        m_pOsInterface,
        pVeboxInterface,
        m_pRenderHal,
        &VeboxExecState[1],
        &PerfData,
        CacheCntl.DnDi,
        &eStatus);
    if (!pRender[VPHAL_RENDER_ID_VEBOX2]    ||
        (eStatus != MOS_STATUS_SUCCESS))
    {
        eStatus = MOS_STATUS_NO_SPACE;
        VPHAL_RENDER_ASSERTMESSAGE("Allocate Vebox Render Fail.");
        return eStatus;
    }

    // Allocate Composite State
    pRender[VPHAL_RENDER_ID_COMPOSITE] = MOS_New(
        CompositeStateG8,
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

MOS_STATUS VphalRendererG8::InitKdllParam()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    // Set KDLL parameters (Platform dependent)
#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
    pKernelDllRules         = g_KdllRuleTable_g8;
    pcKernelBin             = (const void*)IGVPKRN_G8;
    dwKernelBinSize         = IGVPKRN_G8_SIZE;
#endif

    return eStatus;
}
