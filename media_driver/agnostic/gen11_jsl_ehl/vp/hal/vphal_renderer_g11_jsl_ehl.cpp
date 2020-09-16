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
//! \file     vphal_renderer_g11_jsl_ehl.cpp
//! \brief    VPHAL top level rendering component and the entry to low level renderers
//! \details  The top renderer is responsible for coordinating the sequence of calls to low level renderers, e.g. DNDI or Comp
//!
#include "vphal_renderer_g11_jsl_ehl.h"
#include "vphal_render_vebox_g11_base.h"
#include "vphal_render_composite_g11_jsl_ehl.h"
#include "igvpkrn_g11_icllp.h"

extern const Kdll_RuleEntry         g_KdllRuleTable_g11[];

MOS_STATUS VphalRendererG11JslEhl::InitKdllParam()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    m_pRenderHal->bEnableP010SinglePass = true;
    // Set KDLL parameters (Platform dependent)
    pKernelDllRules = g_KdllRuleTable_g11;
    pcKernelBin = (const void*)IGVPKRN_G11_ICLLP;
    dwKernelBinSize = IGVPKRN_G11_ICLLP_SIZE;

    return eStatus;
}

MOS_STATUS VphalRendererG11JslEhl::AllocateRenderComponents(
    PMHW_VEBOX_INTERFACE                pVeboxInterface,
    PMHW_SFC_INTERFACE                  pSfcInterface)
{
    MOS_STATUS              eStatus;
    VPHAL_RENDER_CACHE_CNTL CacheCntl;

    VPHAL_RENDER_CHK_NULL_RETURN(m_pRenderHal);

    eStatus = MOS_STATUS_SUCCESS;

    // Get the cache settings
    MOS_ZeroMemory(&CacheCntl, sizeof(CacheCntl));

    CacheCntl.bDnDi = true;
    CacheCntl.bCompositing = true;

    VPHAL_RENDERER_GET_CACHE_CNTL(this,
        m_pOsInterface,
        &m_pRenderHal->Platform,
        m_pSkuTable,
        &CacheCntl);

    // Initialize Advanced Processing Interface
    pRender[VPHAL_RENDER_ID_VEBOX] = MOS_New(
        VPHAL_VEBOX_STATE_G11_BASE,
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
        VPHAL_VEBOX_STATE_G11_BASE,
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
        CompositeStateG11JslEhl,
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
