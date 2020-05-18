/*
* Copyright (c) 2017-2020, Intel Corporation
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

//!
//! \brief    Main render function
//! \details  The top level renderer function, which may contain multiple
//!           passes of rendering
//! \param    [in] pcRenderParams
//!           Const pointer to VPHAL render parameter
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VphalRendererG12Tgllp::Render(
    PCVPHAL_RENDER_PARAMS   pcRenderParams)
{
    MOS_STATUS              eStatus;
    PMOS_INTERFACE          pOsInterface;
    PRENDERHAL_INTERFACE    pRenderHal;
    VPHAL_RENDER_PARAMS     RenderParams;                                       // Make a copy of render params
    PVPHAL_SURFACE          pSrcLeft[VPHAL_MAX_SOURCES];                        // Array of sources referring to left view stereo content
    PVPHAL_SURFACE          pSrcRight[VPHAL_MAX_SOURCES];                       // Array of sources referring to right view stereo content
    uint32_t                uiRenderPasses;                                     // Number of rendering passes in this one call to VpHal_RndrRender()
    uint32_t                uiCurrentRenderPass;                                // Current render pass
    uint32_t                uiDst;
    VPHAL_GET_SURFACE_INFO  Info;

    //--------------------------------------------
    VPHAL_RENDER_ASSERT(pcRenderParams);
    VPHAL_RENDER_ASSERT(m_pOsInterface);
    VPHAL_RENDER_ASSERT(m_pRenderHal);
    VPHAL_RENDER_ASSERT(pKernelDllState);
    VPHAL_RENDER_ASSERT(pRender[VPHAL_RENDER_ID_COMPOSITE]);
    //--------------------------------------------

    eStatus         = MOS_STATUS_SUCCESS;
    pOsInterface    = m_pOsInterface;
    pRenderHal      = m_pRenderHal;

    // Validate render target
    if (pcRenderParams->pTarget[0] == nullptr ||
        Mos_ResourceIsNull(&(pcRenderParams->pTarget[0]->OsResource)))
    {
        VPHAL_RENDER_ASSERTMESSAGE("Invalid Render Target.");
        eStatus = MOS_STATUS_UNKNOWN;
        goto finish;
    }

    // Protection mechanism, Only SKL+ support P010 output.
    if (IsFormatSupported(pcRenderParams) == false)
    {
        VPHAL_RENDER_ASSERTMESSAGE("Invalid Render Target Output Format.");
        eStatus = MOS_STATUS_UNKNOWN;
        goto finish;
    }

    VPHAL_DBG_STATE_DUMP_SET_CURRENT_FRAME_COUNT(uiFrameCounter);

    // Validate max number sources
    if (pcRenderParams->uSrcCount > VPHAL_MAX_SOURCES)
    {
        VPHAL_RENDER_ASSERTMESSAGE("Invalid number of samples.");
        eStatus = MOS_STATUS_UNKNOWN;
        goto finish;
    }

    // Validate max number targets
    if (pcRenderParams->uDstCount > VPHAL_MAX_TARGETS)
    {
        VPHAL_RENDER_ASSERTMESSAGE("Invalid number of targets.");
        eStatus = MOS_STATUS_UNKNOWN;
        goto finish;
    }

    // Copy the Render Params structure (so we can update it)
    RenderParams = *pcRenderParams;

    VPHAL_DBG_PARAMETERS_DUMPPER_DUMP_XML(&RenderParams);

    // Get resource information for render target
    MOS_ZeroMemory(&Info, sizeof(VPHAL_GET_SURFACE_INFO));

    for (uiDst = 0; uiDst < RenderParams.uDstCount; uiDst++)
    {
        VPHAL_RENDER_CHK_STATUS(VpHal_GetSurfaceInfo(
            m_pOsInterface,
            &Info,
            RenderParams.pTarget[uiDst]));
    }

    // Set the component info
    m_pOsInterface->Component = pcRenderParams->Component;

    // Init component(DDI entry point) info for perf measurement
    m_pOsInterface->pfnSetPerfTag(m_pOsInterface, VPHAL_NONE);

    // Increment frame ID for performance measurement
    m_pOsInterface->pfnIncPerfFrameID(m_pOsInterface);

    // Enable Turbo mode if sku present and DDI requests it
    if (m_pSkuTable && MEDIA_IS_SKU(m_pSkuTable, FtrMediaTurboMode))
    {
        m_pRenderHal->bTurboMode = RenderParams.bTurboMode;
    }

    // Reset feature reporting
    m_reporting->InitReportValue();

    MOS_ZeroMemory(pSrcLeft,  sizeof(PVPHAL_SURFACE) * VPHAL_MAX_SOURCES);
    MOS_ZeroMemory(pSrcRight, sizeof(PVPHAL_SURFACE) * VPHAL_MAX_SOURCES);

    VPHAL_RENDER_CHK_STATUS(PrepareSources(
            &RenderParams,
            pSrcLeft,
            pSrcRight,
            &uiRenderPasses));

    //set GpuContext
    VPHAL_RENDER_CHK_STATUS(SetRenderGpuContext(RenderParams));

    // align rectangle and source surface
    for (uiDst = 0; uiDst < RenderParams.uDstCount; uiDst++)
    {
        VPHAL_RENDER_CHK_STATUS(VpHal_RndrRectSurfaceAlignment(RenderParams.pTarget[uiDst], RenderParams.pTarget[uiDst]->Format));
    }

    for (uiCurrentRenderPass = 0;
         uiCurrentRenderPass < uiRenderPasses;
         uiCurrentRenderPass++)
    {
        // Assign source surfaces for current rendering pass
        MOS_SecureMemcpy(
            RenderParams.pSrc,
            sizeof(PVPHAL_SURFACE) * VPHAL_MAX_SOURCES,
            (uiCurrentRenderPass == 0) ? pSrcLeft : pSrcRight,
            sizeof(PVPHAL_SURFACE) * VPHAL_MAX_SOURCES);

        MOS_ZeroMemory(&Info, sizeof(VPHAL_GET_SURFACE_INFO));

        for (uiDst = 0; uiDst < RenderParams.uDstCount; uiDst++)
        {
            Info.S3dChannel = RenderParams.pTarget[uiDst]->Channel;
            Info.ArraySlice = uiCurrentRenderPass;

            VPHAL_RENDER_CHK_STATUS(VpHal_GetSurfaceInfo(
                m_pOsInterface,
                &Info,
                RenderParams.pTarget[uiDst]));
        }

        // Update channel. 0 = mono or stereo left, 1 = stereo right
        uiCurrentChannel = uiCurrentRenderPass;

        VPHAL_RENDER_CHK_STATUS(RenderPass(&RenderParams));
    }

#if defined(__linux__)
    if (m_reporting)
    {
        WriteUserFeature(__VPHAL_VEBOX_OUTPUTPIPE_MODE_ID, m_reporting->OutputPipeMode);
        WriteUserFeature(__VPHAL_VEBOX_FEATURE_INUSE_ID, m_reporting->VEFeatureInUse);
    }
#endif

finish:
    uiFrameCounter++;
    return eStatus;
}

