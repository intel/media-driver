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
//! \file     vphal_renderer_g12_tgllp.cpp
//! \brief    VPHAL top level rendering component and the entry to low level renderers
//! \details  The top renderer is responsible for coordinating the sequence of calls to low level renderers, e.g. DNDI or Comp
//!
#include "vphal_renderer_g12_tgllp.h"
#if defined(ENABLE_KERNELS)
#include "igvpkrn_g12_tgllp_cmfc.h"
#include "igvpkrn_g12_tgllp_cmfcpatch.h"
#endif

extern const Kdll_RuleEntry         g_KdllRuleTable_g12lp[];
extern const Kdll_RuleEntry         g_KdllRuleTable_g12lpcmfc[];

MOS_STATUS VphalRendererG12Tgllp::InitKdllParam()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    // Override kernel binary for CMFC/SWSB
    if (bEnableCMFC)
    {
#if defined(ENABLE_KERNELS)
        pKernelDllRules     = g_KdllRuleTable_g12lpcmfc;
        pcKernelBin         = (const void *)IGVPKRN_G12_TGLLP_CMFC;
        dwKernelBinSize     = IGVPKRN_G12_TGLLP_CMFC_SIZE;
        pcFcPatchBin        = (const void *)IGVPKRN_G12_TGLLP_CMFCPATCH;
        dwFcPatchBinSize    = IGVPKRN_G12_TGLLP_CMFCPATCH_SIZE;
#endif
    }

    if ((NULL == pcFcPatchBin) || (0 == dwFcPatchBinSize))
    {
        bEnableCMFC = false;
    }

    if (bEnableCMFC && (NULL != pcFcPatchBin) && (0 != dwFcPatchBinSize))
    {
        m_pRenderHal->bEnableP010SinglePass = true;
    }
    else
    {
        m_pRenderHal->bEnableP010SinglePass = false;
    }
    
    return eStatus;
}
