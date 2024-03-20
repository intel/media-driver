/*
* Copyright (c) 2009-2021, Intel Corporation
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
//! \file     vphal.cpp
//! \brief    Vphal Interface Definition
//! \details  Vphal Interface Definition Including:
//!           const and function
//!
#include "vphal.h"
#include "mos_os.h"
#include "mos_interface.h"
#include "mhw_vebox.h"
#include "renderhal_legacy.h"
#include "vphal_renderer.h"
#include "mos_solo_generic.h"
#include "media_interfaces_vphal.h"
#include "media_interfaces_mhw.h"
#include "mhw_vebox_itf.h"
#include "mos_oca_rtlog_mgr.h"
#include "vp_user_setting.h"

//!
//! \brief    Allocate VPHAL Resources
//! \details  Allocate VPHAL Resources
//!           - Allocate and initialize HW states
//!           - Allocate and initialize renderer states
//! \param    [in] pVpHalSettings
//!           Pointer to VPHAL Settings
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VphalState::Allocate(
    const VphalSettings       *pVpHalSettings)
{
    MHW_VEBOX_GPUNODE_LIMIT     GpuNodeLimit;
    RENDERHAL_SETTINGS_LEGACY   RenderHalSettings;
    MOS_GPU_NODE                VeboxGpuNode;
    MOS_GPU_CONTEXT             VeboxGpuContext;
    bool                        checkGpuCtxOverwriten = false;
    bool                        addGpuCtxToCheckList  = false;
    MOS_STATUS                  eStatus;

    VPHAL_PUBLIC_CHK_NULL(m_osInterface);
    VPHAL_PUBLIC_CHK_NULL(pVpHalSettings);
    VPHAL_PUBLIC_CHK_NULL(m_renderHal);

    m_clearVideoViewMode = pVpHalSettings->clearVideoViewMode;

    if ((MEDIA_IS_SKU(m_skuTable, FtrVERing) ||
         MEDIA_IS_SKU(m_skuTable, FtrSFCPipe)) &&
        !m_clearVideoViewMode)
    {
        MhwInterfaces *             mhwInterfaces = nullptr;
        MhwInterfaces::CreateParams params;
        MOS_ZeroMemory(&params, sizeof(params));
        params.Flags.m_sfc   = MEDIA_IS_SKU(m_skuTable, FtrSFCPipe);
        params.Flags.m_vebox = MEDIA_IS_SKU(m_skuTable, FtrVERing);

        mhwInterfaces = MhwInterfaces::CreateFactory(params, m_osInterface);
        if (mhwInterfaces)
        {
            SetMhwVeboxInterface(mhwInterfaces->m_veboxInterface);
            SetMhwSfcInterface(mhwInterfaces->m_sfcInterface);

            // MhwInterfaces always create CP and MI interfaces, so we have to delete those we don't need.
            MOS_Delete(mhwInterfaces->m_miInterface);
            m_osInterface->pfnDeleteMhwCpInterface(mhwInterfaces->m_cpInterface);
            mhwInterfaces->m_cpInterface = nullptr;
            MOS_Delete(mhwInterfaces);
        }
        else
        {
            VPHAL_DEBUG_ASSERTMESSAGE("Allocate MhwInterfaces failed");
            eStatus = MOS_STATUS_NO_SPACE;
            MT_ERR1(MT_VP_HAL_INIT, MT_CODE_LINE, __LINE__);
        }
    }

    if (IsApoEnabled() &&
        m_osInterface->apoMosEnabled &&
        m_osInterface->bSetHandleInvalid)
    {
        checkGpuCtxOverwriten = true;
    }
    addGpuCtxToCheckList = checkGpuCtxOverwriten && !IsGpuContextReused(m_renderGpuContext);

    if (IsRenderContextBasedSchedulingNeeded())
    {
        Mos_SetVirtualEngineSupported(m_osInterface, true);
        m_osInterface->pfnVirtualEngineSupported(m_osInterface, true, true);
    }

    // Create Render GPU Context
    {
        MOS_GPUCTX_CREATOPTIONS_ENHANCED createOption = {};
        VPHAL_PUBLIC_CHK_STATUS(m_osInterface->pfnCreateGpuContext(
            m_osInterface,
            m_renderGpuContext,
            m_renderGpuNode,
            &createOption));
    }

    // Set current GPU context
    VPHAL_PUBLIC_CHK_STATUS(m_osInterface->pfnSetGpuContext(
        m_osInterface,
        m_renderGpuContext));

    // Add gpu context entry, including stream 0 gpu contexts
    // In legacy path, stream 0 gpu contexts could be reused, and will keep these stream 0 gpu contexts alive
    // But its mapping relation could be overwritten in APO path
    // Now, don't reuse these stream 0 gpu contexts overwritten by MediaContext in APO path 
    // Can not add reused GPU context
    if (addGpuCtxToCheckList)
    {
        AddGpuContextToCheckList(m_renderGpuContext);
    }

    // Register Render GPU context with the event
    VPHAL_PUBLIC_CHK_STATUS(m_osInterface->pfnRegisterBBCompleteNotifyEvent(
        m_osInterface,
        m_renderGpuContext));

    if (MEDIA_IS_SKU(m_skuTable, FtrVERing) && m_veboxInterface && !m_clearVideoViewMode)
    {
        GpuNodeLimit.bSfcInUse         = MEDIA_IS_SKU(m_skuTable, FtrSFCPipe);

        if (m_veboxItf)
        {
            // Check GPU Node decide logic together in this function
            VPHAL_HW_CHK_STATUS(m_veboxItf->FindVeboxGpuNodeToUse(&GpuNodeLimit));

            VeboxGpuNode    = (MOS_GPU_NODE)(GpuNodeLimit.dwGpuNodeToUse);
            VeboxGpuContext = (VeboxGpuNode == MOS_GPU_NODE_VE) ? MOS_GPU_CONTEXT_VEBOX : MOS_GPU_CONTEXT_VEBOX2;

            VPHAL_PUBLIC_CHK_STATUS(m_veboxItf->CreateGpuContext(
                m_osInterface,
                VeboxGpuContext,
                VeboxGpuNode));
        }

        // Check GPU Node decide logic together in this function
        VPHAL_HW_CHK_STATUS(m_veboxInterface->FindVeboxGpuNodeToUse(&GpuNodeLimit));

        VeboxGpuNode    = (MOS_GPU_NODE)(GpuNodeLimit.dwGpuNodeToUse);
        VeboxGpuContext = (VeboxGpuNode == MOS_GPU_NODE_VE) ? MOS_GPU_CONTEXT_VEBOX : MOS_GPU_CONTEXT_VEBOX2;

        addGpuCtxToCheckList = checkGpuCtxOverwriten && !IsGpuContextReused(VeboxGpuContext);
      
        // Create VEBOX/VEBOX2 Context
        VPHAL_PUBLIC_CHK_STATUS(m_veboxInterface->CreateGpuContext(
            m_osInterface,
            VeboxGpuContext,
            VeboxGpuNode));

        // Add gpu context entry, including stream 0 gpu contexts
        if (addGpuCtxToCheckList)
        {
            AddGpuContextToCheckList(VeboxGpuContext);
        }
        
        // Register Vebox GPU context with the Batch Buffer completion event
        // Ignore if creation fails
        VPHAL_PUBLIC_CHK_STATUS(m_osInterface->pfnRegisterBBCompleteNotifyEvent(
            m_osInterface,
            MOS_GPU_CONTEXT_VEBOX));
    }

    // Allocate and initialize HW states
    RenderHalSettings.iMediaStates  = pVpHalSettings->mediaStates;
    VPHAL_PUBLIC_CHK_STATUS(m_renderHal->pfnInitialize(m_renderHal, &RenderHalSettings));

    if (!m_clearVideoViewMode)
    {
        if (m_veboxItf)
        {
            const MHW_VEBOX_HEAP *veboxHeap = nullptr;
            m_veboxItf->GetVeboxHeapInfo(&veboxHeap);
            uint32_t uiNumInstances = m_veboxItf->GetVeboxNumInstances();

            if (uiNumInstances > 0 &&
                veboxHeap == nullptr)
            {
                // Allocate VEBOX Heap
                VPHAL_PUBLIC_CHK_STATUS(m_veboxItf->CreateHeap());
            }
        }

        if (m_veboxInterface &&
            m_veboxInterface->m_veboxSettings.uiNumInstances > 0 &&
            m_veboxInterface->m_veboxHeap == nullptr)
        {
            VPHAL_PUBLIC_CHK_STATUS(m_veboxInterface->CreateHeap());
        }
    }

    // Create renderer
    VPHAL_RENDER_CHK_STATUS(CreateRenderer());

    // Allocate and initialize renderer states
    VPHAL_PUBLIC_CHK_STATUS(m_renderer->Initialize(pVpHalSettings, IsApoEnabled()));

finish:
    return eStatus;
}

//!
//! \brief    Check if need to apply AVS for specified surface
//! \details  Check if need to apply AVS for specified surface
//! \param    pSurf
//!           [in] Pointer to VPHAL_SURFACE
//! \return   bool
//!           Return true if the input surface needs AVS scaling
//!
static bool IsSurfNeedAvs(
    PVPHAL_SURFACE pSurf)
{
    VPHAL_PUBLIC_CHK_NULL_NO_STATUS(pSurf);

    // Not perform AVS for surface with alpha channel.
    if (IS_ALPHA_YUV_FORMAT(pSurf->Format))
    {
        return false;
    }

    if (pSurf->SurfType == SURF_IN_PRIMARY ||
        pSurf->SurfType == SURF_IN_SUBSTREAM)
    {
        if (pSurf->pBlendingParams)
        {
            return false;
        }

        if (IS_YUV_FORMAT(pSurf->Format))
        {
            // Not perform AVS for surface with VPHAL_SCALING_NEAREST
            // or VPHAL_SCALING_BILINEAR scaling mode.
            if (pSurf->ScalingMode == VPHAL_SCALING_AVS ||
                pSurf->ScalingMode == VPHAL_SCALING_ADV_QUALITY)
            {
                return true;
            }
        }
    }
finish:
    return false;
}

//!
//! \brief    Check if all surfaces format order in pcRenderParams is suitable to enable multiple avs rendering (YUV is prior than RGB)
//! \details  In VpHal_RenderWithAvsForMultiStreams() we group YUV surfaces in phase one and the others in phase two.
//!           However, if there are 2+ overlapped source surfaces, e.g., pSrc[0]=B8G8R8A8 and pSrc[1]=NV12.
//!           We don't enable multiple avs rendering for such case since we expect pSrc[1] is above pSrc[0] on target.
//!           If multiple avs rendering is enabled, the compositing order is reverse and turns out pSrc[1] is below pSrc[0],
//!           which is the case need to prevent.
//! \param    pcRenderParams
//!           [in] Pointer to const VPHAL_RENDER_PARAMS
//! \return   bool
//!           Allow to enable multiple avs rendering
//!
static bool IsYuvPriorRgbInCompositeOrder(
    PCVPHAL_RENDER_PARAMS   pcRenderParams)
{
    bool     bHasRgbSurface = false;
    uint32_t uiLastYuvIdx   = 0;
    uint32_t uiFirstRgbIdx  = 0;
    uint32_t iSourceIdx;

    VPHAL_PUBLIC_CHK_NULL_NO_STATUS(pcRenderParams);

    for (iSourceIdx = 0; iSourceIdx < pcRenderParams->uSrcCount; iSourceIdx++)
    {
        PVPHAL_SURFACE pSurf = pcRenderParams->pSrc[iSourceIdx];
        if (IsSurfNeedAvs(pSurf))
        {
                uiLastYuvIdx = iSourceIdx;
        }
        else if (!bHasRgbSurface)
        {
                bHasRgbSurface = true;
                uiFirstRgbIdx  = iSourceIdx;
        }
    }

finish:
    if (!bHasRgbSurface)
    {   // return true if not contain any RGB surface
        return true;
    }
    return (uiLastYuvIdx < uiFirstRgbIdx);
}

//!
//! \brief    Check if need to apply AVS scaling for multiple surfaces
//! \details  Check if need to apply AVS scaling for multiple surfaces
//! \param    pcRenderParams
//!           [in] Pointer to const VPHAL_RENDER_PARAMS
//! \return   bool
//!           Return true if neeeded to enable AVS for all primary surfaces.
//!
static bool VpHal_IsAvsSampleForMultiStreamsEnabled(
    PCVPHAL_RENDER_PARAMS   pcRenderParams)
{
    uint32_t uSourceIdx;
    uint32_t uSourceBIdx;
    uint32_t uAvsSurfaceCnt            = 0;
    bool     bEnableAvsForMultiSurface = false;

    VPHAL_PUBLIC_CHK_NULL_NO_STATUS(pcRenderParams);

    //disable if only one, zero or invalid number source
    if (pcRenderParams->uSrcCount <= 1 ||
        pcRenderParams->uSrcCount > VPHAL_MAX_SOURCES)
    {
       goto finish;
    }

    for (uSourceIdx = 0; uSourceIdx < pcRenderParams->uSrcCount; uSourceIdx++)
    {
        PVPHAL_SURFACE pSrcSurf = pcRenderParams->pSrc[uSourceIdx];
        if (pSrcSurf && pSrcSurf->SurfType == SURF_IN_PRIMARY)
        {
            // VPHAL_SCALING_PREFER_SFC_FOR_VEBOX means scaling mode is FastMode, don't apply AVS for multiples
            if (pSrcSurf->ScalingPreference == VPHAL_SCALING_PREFER_SFC_FOR_VEBOX)
            {
                goto finish;
            }

            if (!IS_YUV_FORMAT(pSrcSurf->Format))
            {
                goto finish;
            }

            // disabled if need denoise or deinterlace
            if (pSrcSurf->pDenoiseParams || pSrcSurf->pDeinterlaceParams)
            {
                goto finish;
            }

            if (0 < pSrcSurf->uFwdRefCount || 0 < pSrcSurf->uBwdRefCount)
            {
                goto finish;
            }
        }
        if (pSrcSurf && IsSurfNeedAvs(pSrcSurf))
        {
            uAvsSurfaceCnt++;
        }
    }

    if (uAvsSurfaceCnt <= 1)
    {
        goto finish;
    }

    if (!IsYuvPriorRgbInCompositeOrder(pcRenderParams))
    {
        goto finish;
    }
 
    // disable if any AVS surface overlay to other AVS surface.
    for (uSourceIdx = 0; uSourceIdx < pcRenderParams->uSrcCount; uSourceIdx++)
    {
        PVPHAL_SURFACE pSrcSurfA         = pcRenderParams->pSrc[uSourceIdx];
        if (pSrcSurfA && IsSurfNeedAvs(pSrcSurfA))
        {
            for (uSourceBIdx = uSourceIdx + 1; uSourceBIdx < pcRenderParams->uSrcCount; uSourceBIdx++)
            {
                PVPHAL_SURFACE pSrcSurfB = pcRenderParams->pSrc[uSourceBIdx];
                if (pSrcSurfB && IsSurfNeedAvs(pSrcSurfB) &&
                    !RECT1_OUTSIDE_RECT2(pSrcSurfA->rcDst, pSrcSurfB->rcDst)) // not outside means overlay
                {
                    goto finish;
                }
            }
        }
    }

    bEnableAvsForMultiSurface = true;
finish:
    return bEnableAvsForMultiSurface;
}

//!
//! \brief    Rendering with AVS scaling for multiple surfaces
//! \details  Rendering with AVS scaling for multiple surfaces
//!           Currently composition kernel only allows first primary surface with AVS.
//!           To apply AVS for multiple surfaces, this function do the following steps:
//!           (1) separating each AVS surface into different phases containing only one primary
//!           (2) rendering with individual primary, and let first pass with color filler
//!           (3) rendering remaining non-AVS substreams, if any, with intermediate surface in one phase to allow alpha blending.
//! \param    pRenderer
//!           [in]  Pointer to VphalRenderer
//! \param    pcRenderParams
//!           [in] Pointer to const VPHAL_RENDER_PARAMS
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
static MOS_STATUS VpHal_RenderWithAvsForMultiStreams(
    VphalRenderer        *pRenderer,
    PCVPHAL_RENDER_PARAMS   pcRenderParams)
{
    uint32_t                 uSourceIdx;
    VPHAL_RENDER_PARAMS      RenderParams;
    VPHAL_COLORFILL_PARAMS   colorFillForFirstRenderPass;
    bool                     bColorFillForFirstRender;
    bool                     bHasNonAvsSubstream;
    MOS_STATUS               eStatusSingleRender;
    PVPHAL_SURFACE           pIntermediateSurface;
    PVPHAL_SURFACE           pBackGroundSurface;
    MOS_STATUS               eStatus                   = MOS_STATUS_SUCCESS;
    VPHAL_SCALING_PREFERENCE ePrimaryScalingPreference = VPHAL_SCALING_PREFER_SFC;
    VPHAL_RENDER_PARAMS      *pRenderParams;

    VPHAL_PUBLIC_CHK_NULL(pRenderer);
    VPHAL_PUBLIC_CHK_NULL(pRenderer->GetOsInterface());
    VPHAL_PUBLIC_CHK_NULL(pcRenderParams);
    VPHAL_PUBLIC_CHK_NULL(pcRenderParams->pTarget[0]);

    pRenderParams =
        (VPHAL_RENDER_PARAMS*)pcRenderParams;

    if (pcRenderParams->pColorFillParams)
    {
        colorFillForFirstRenderPass = *pcRenderParams->pColorFillParams;
    }
    else
    {
        colorFillForFirstRenderPass.bYCbCr = false;
        colorFillForFirstRenderPass.Color  = 0;
        colorFillForFirstRenderPass.CSpace = CSpace_sRGB;
    }

    pIntermediateSurface = &pRenderer->IntermediateSurface;

    // check if having non-AVS substream
    bHasNonAvsSubstream   = false;
    pBackGroundSurface    = nullptr;
    for (uSourceIdx = 0; uSourceIdx < pcRenderParams->uSrcCount; uSourceIdx++)
    {
        PVPHAL_SURFACE pSurf = (PVPHAL_SURFACE)pcRenderParams->pSrc[uSourceIdx];
        if (pSurf && pSurf->SurfType == SURF_IN_SUBSTREAM)
        {
            if (!IS_YUV_FORMAT(pSurf->Format))
            {
                bHasNonAvsSubstream = true;
            }
        }

        if (pSurf && pSurf->SurfType == SURF_IN_BACKGROUND)
        {
            pBackGroundSurface = pSurf;
        }

        // keep scaling preference of primary for later usage
        if (pSurf && pSurf->SurfType == SURF_IN_PRIMARY)
        {
            ePrimaryScalingPreference = pSurf->ScalingPreference;
        }
    }

    // create an intermediate surface if having non-AVS substream, as phase 1 output and phase 2 input.
    if (bHasNonAvsSubstream)
    {
        bool     bAllocated;
        uint32_t dwTempWidth  = pcRenderParams->pTarget[0]->dwWidth;
        uint32_t dwTempHeight = pcRenderParams->pTarget[0]->dwHeight;

        // Allocate/Reallocate temporary output
        if (dwTempWidth  > pRenderer->IntermediateSurface.dwWidth ||
            dwTempHeight > pRenderer->IntermediateSurface.dwHeight)
        {
            dwTempWidth  = MOS_MAX(dwTempWidth , pRenderer->IntermediateSurface.dwWidth);
            dwTempHeight = MOS_MAX(dwTempHeight, pRenderer->IntermediateSurface.dwHeight);
            dwTempWidth  = MOS_ALIGN_CEIL(dwTempWidth , VPHAL_BUFFER_SIZE_INCREMENT);
            dwTempHeight = MOS_ALIGN_CEIL(dwTempHeight, VPHAL_BUFFER_SIZE_INCREMENT);

            eStatusSingleRender = VpHal_ReAllocateSurface(
                pRenderer->GetOsInterface(),
                &pRenderer->IntermediateSurface,
                "RenderIntermediateSurface",
                pcRenderParams->pTarget[0]->Format,
                MOS_GFXRES_2D,
                pcRenderParams->pTarget[0]->TileType,
                dwTempWidth,
                dwTempHeight,
                false,
                MOS_MMC_DISABLED,
                &bAllocated);

            if (MOS_SUCCEEDED(eStatusSingleRender))
            {
                pIntermediateSurface->SurfType      = SURF_IN_PRIMARY;
                pIntermediateSurface->SampleType    = SAMPLE_PROGRESSIVE;
                pIntermediateSurface->ColorSpace    = pcRenderParams->pTarget[0]->ColorSpace;
                pIntermediateSurface->ExtendedGamut = pcRenderParams->pTarget[0]->ExtendedGamut;
                pIntermediateSurface->rcSrc         = pcRenderParams->pTarget[0]->rcSrc;
                pIntermediateSurface->rcDst         = pcRenderParams->pTarget[0]->rcDst;
                pIntermediateSurface->ScalingMode   = VPHAL_SCALING_AVS;
                pIntermediateSurface->bIEF          = false;
            }
            else
            {
                eStatus             = eStatusSingleRender;
                bHasNonAvsSubstream = false;
                VPHAL_PUBLIC_ASSERTMESSAGE("Failed to create intermediate surface, eStatus: %d.\n", eStatus);
                MT_ERR1(MT_VP_HAL_REALLOC_SURF, MT_CODE_LINE, __LINE__);
            }
        }
    } 

    // phase 1: render each AVS surface in separated pass
    bColorFillForFirstRender = true;
    RenderParams             = *pRenderParams;
    RenderParams.uDstCount   = 1;
    MOS_ZeroMemory(RenderParams.pSrc, sizeof(PVPHAL_SURFACE) * VPHAL_MAX_SOURCES);
    
    for (uSourceIdx = 0; uSourceIdx < pcRenderParams->uSrcCount; uSourceIdx++)
    {
        VPHAL_SURFACE               SinglePassSource;
        PVPHAL_SURFACE              pSurf = (PVPHAL_SURFACE)pcRenderParams->pSrc[uSourceIdx];
        RenderParams.uSrcCount         = 0;

        if (pSurf && IsSurfNeedAvs(pSurf))
        {
            SinglePassSource                   = *(pSurf);
            SinglePassSource.SurfType          = SURF_IN_PRIMARY;
            SinglePassSource.ScalingMode       = VPHAL_SCALING_AVS;
            SinglePassSource.ScalingPreference = ePrimaryScalingPreference;

            RenderParams.pTarget[0]   = (bHasNonAvsSubstream ? pIntermediateSurface : pcRenderParams->pTarget[0]);

            if (bColorFillForFirstRender) // apply color-fill for the first rendering
            {
                bColorFillForFirstRender      = false;
                RenderParams.pColorFillParams = &colorFillForFirstRenderPass;

                if (pBackGroundSurface)
                {
                    RenderParams.pSrc[RenderParams.uSrcCount] = pBackGroundSurface;
                    RenderParams.uSrcCount++;
                }
            }
            else if (!bColorFillForFirstRender && SinglePassSource.pLumaKeyParams)  // colorfill is needed if lumakey enabled on a single pass
            {
                RenderParams.pColorFillParams  = &colorFillForFirstRenderPass;
                RenderParams.pTarget[0]->rcDst = SinglePassSource.rcDst;
            }
            else
            {
                RenderParams.pTarget[0]->rcDst = SinglePassSource.rcDst;
                RenderParams.pColorFillParams  = nullptr;
            }

            RenderParams.pSrc[RenderParams.uSrcCount] = &SinglePassSource;
            RenderParams.uSrcCount++;

            // continue the next pfnRender even if single pass failed, but return eStatus failed. 
            eStatusSingleRender = pRenderer->Render((PCVPHAL_RENDER_PARAMS)(&RenderParams));
            if (MOS_FAILED(eStatusSingleRender)) 
            {
                eStatus = eStatusSingleRender;
                VPHAL_PUBLIC_ASSERTMESSAGE("Failed to redner for primary streams, eStatus: %d.\n", eStatus);
                MT_ERR2(MT_VP_HAL_RENDER, MT_ERROR_CODE, eStatus, MT_CODE_LINE, __LINE__);
            }
        }
    }

    // phase 2: if having non-AVS substream, render them with previous output in one pass.
    if (bHasNonAvsSubstream)
    {
        RenderParams                = *pRenderParams;
        MOS_ZeroMemory(RenderParams.pSrc, sizeof(PVPHAL_SURFACE) * VPHAL_MAX_SOURCES);

        pIntermediateSurface->rcSrc = pcRenderParams->pTarget[0]->rcSrc;
        pIntermediateSurface->rcDst = pcRenderParams->pTarget[0]->rcDst;
        RenderParams.pSrc[0]        = pIntermediateSurface;
        RenderParams.uSrcCount      = 1;
        RenderParams.uDstCount      = 1;

        for (uSourceIdx = 0; uSourceIdx < pcRenderParams->uSrcCount; uSourceIdx++)
        {
            PVPHAL_SURFACE pSurf = pcRenderParams->pSrc[uSourceIdx];
            bool bIsSurfForPhase2   = (pSurf &&
                                    (!IsSurfNeedAvs(pSurf)) &&
                                    (pSurf->SurfType != SURF_IN_BACKGROUND));
            if (bIsSurfForPhase2)
            {
                RenderParams.pSrc[RenderParams.uSrcCount] = pSurf;
                RenderParams.uSrcCount++;
            }
        }

        eStatusSingleRender = pRenderer->Render((PCVPHAL_RENDER_PARAMS)(&RenderParams));
        if (MOS_FAILED(eStatusSingleRender)) 
        {
            eStatus = eStatusSingleRender;
            VPHAL_PUBLIC_ASSERTMESSAGE("Failed to redner for substreams, eStatus: %d.\n", eStatus);
            MT_ERR2(MT_VP_HAL_RENDER, MT_ERROR_CODE, eStatus, MT_CODE_LINE, __LINE__);
        }
    }

finish:
    return eStatus;
}

//!
//! \brief    Performs VP Rendering
//! \details  Performs VP Rendering
//!           - call default render of video
//! \param    [in] pcRenderParams
//!           Pointer to Render Params
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VphalState::Render(
    PCVPHAL_RENDER_PARAMS   pcRenderParams)
{
    MOS_STATUS          eStatus;
    VPHAL_RENDER_PARAMS RenderParams;

    VPHAL_PUBLIC_CHK_NULL(pcRenderParams);
    RenderParams    = *pcRenderParams;

    // Explicitly initialize the maxSrcRect of VphalRenderer
    // so that the maxSrcRect for the last set of surfaces does not get
    // re-used for the current set of surfaces.
    m_renderer->InitMaxSrcRect();

    if (VpHal_IsAvsSampleForMultiStreamsEnabled(pcRenderParams))
    {
        eStatus = VpHal_RenderWithAvsForMultiStreams(m_renderer, pcRenderParams);
        goto finish;
    }

    // default render of video
    RenderParams.bIsDefaultStream = true;
    eStatus = m_renderer->Render((PCVPHAL_RENDER_PARAMS)(&RenderParams));

finish:
    return eStatus;
}

//!
//! \brief    Get feature reporting from renderer
//! \details  Get feature reporting from renderer
//! \return   VphalFeatureReport*
//!           Pointer to VphalFeatureReport: rendering features reported
//!
VphalFeatureReport* VphalState::GetRenderFeatureReport()
{
    VPHAL_PUBLIC_ASSERT(m_renderer);

    return m_renderer->GetReport();
}

//!
//! \brief    VphalState Constructor
//! \details  Creates instance of VphalState
//!           - Return pointer to initialized VPHAL state, but not yet fully allocated
//!           - Caller must call pfnAllocate to allocate all VPHAL states and objects.
//! \param    [in] pOsInterface
//!           OS interface, if provided externally - may be nullptr
//! \param    [in,out] peStatus
//!           Pointer to the MOS_STATUS flag.
//!           Will assign this flag to MOS_STATUS_SUCCESS if successful, otherwise failed
//!
VphalState::VphalState(
        PMOS_INTERFACE          pOsInterface,
        MOS_STATUS              *peStatus) :
        m_platform(),
        m_skuTable(nullptr),
        m_waTable(nullptr),
        m_osInterface(pOsInterface),
        m_renderHal(nullptr),
        m_veboxInterface(nullptr),
        m_cpInterface(nullptr),
        m_sfcInterface(nullptr),
        m_renderer(nullptr),
        m_renderGpuNode(MOS_GPU_NODE_3D),
        m_renderGpuContext(MOS_GPU_CONTEXT_RENDER)
{
    MOS_STATUS                  eStatus;

    eStatus                     = MOS_STATUS_UNKNOWN;

    VPHAL_PUBLIC_CHK_NULL(m_osInterface);

    // Initialize platform, sku, wa tables
    m_osInterface->pfnGetPlatform(m_osInterface, &m_platform);
    m_skuTable = m_osInterface->pfnGetSkuTable(m_osInterface);
    m_waTable  = m_osInterface->pfnGetWaTable (m_osInterface);

    m_userSettingPtr = m_osInterface->pfnGetUserSettingInstance(m_osInterface);
    VpUserSetting::InitVpUserSetting(m_userSettingPtr);

    m_renderHal = (PRENDERHAL_INTERFACE_LEGACY)MOS_AllocAndZeroMemory(sizeof(RENDERHAL_INTERFACE_LEGACY));
    VPHAL_PUBLIC_CHK_NULL(m_renderHal);
    VPHAL_PUBLIC_CHK_STATUS(RenderHal_InitInterface_Legacy(
        m_renderHal,
        &m_cpInterface,
        m_osInterface));

finish:
    if(peStatus)
    {
        *peStatus = eStatus;
    }
}

//!
//! \brief    Vphal Destructor
//! \details  Destroys VPHAL and all internal states and objects
//! \return   void
//!
VphalState::~VphalState()
{
    MOS_STATUS              eStatus;

    // Wait for all cmd before destroy gpu resource
    if (m_osInterface && m_osInterface->pfnWaitAllCmdCompletion && m_osInterface->bDeallocateOnExit)
    {
        VP_PUBLIC_NORMALMESSAGE("WaitAllCmdCompletion in VphalState::~VphalState");
        m_osInterface->pfnWaitAllCmdCompletion(m_osInterface);
    }

    // Destroy rendering objects (intermediate surfaces, BBs, etc)
    if (m_renderer)
    {
        MOS_Delete(m_renderer);
        m_renderer = nullptr;
    }

    if (m_renderHal)
    {
        if (m_renderHal->pfnDestroy)
        {
            eStatus = m_renderHal->pfnDestroy(m_renderHal);
            if (eStatus != MOS_STATUS_SUCCESS)
            {
                VPHAL_PUBLIC_ASSERTMESSAGE("Failed to destroy RenderHal, eStatus:%d.\n", eStatus);
                MT_ERR1(MT_VP_HAL_DESTROY, MT_CODE_LINE, __LINE__);
            }
        }
        MOS_FreeMemory(m_renderHal);
    }

    if (m_cpInterface)
    {
        if (m_osInterface)
        {
            m_osInterface->pfnDeleteMhwCpInterface(m_cpInterface);
            m_cpInterface = nullptr;
        }
        else
        {
            VPHAL_PUBLIC_ASSERTMESSAGE("Failed to destroy cpInterface.");
        }
    }

    if (m_sfcInterface)
    {
        MOS_Delete(m_sfcInterface);
        m_sfcInterface = nullptr;
    }

    if (m_veboxInterface)
    {
        if (m_veboxItf)
        {
            eStatus = m_veboxItf->DestroyHeap();
            if (eStatus != MOS_STATUS_SUCCESS)
            {
                VPHAL_PUBLIC_ASSERTMESSAGE("Failed to destroy Vebox Interface, eStatus:%d.\n", eStatus);
                MT_ERR1(MT_VP_HAL_DESTROY, MT_CODE_LINE, __LINE__);
            }
        }

        eStatus = m_veboxInterface->DestroyHeap();
        MOS_Delete(m_veboxInterface);
        m_veboxInterface = nullptr;
        m_veboxItf       = nullptr;
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            VPHAL_PUBLIC_ASSERTMESSAGE("Failed to destroy Vebox Interface, eStatus:%d.\n", eStatus);
            MT_ERR1(MT_VP_HAL_DESTROY, MT_CODE_LINE, __LINE__);
        }
    }

    // Destroy OS interface objects (CBs, etc)
    if (m_osInterface)
    {
        if (m_osInterface->bDeallocateOnExit)
        {
            //Clear some overwriten gpucontext resources
            if (!m_gpuContextCheckList.empty())
            {
                DestroyGpuContextWithInvalidHandle();
                m_gpuContextCheckList.clear();
            }
            
            m_osInterface->pfnDestroy(m_osInterface, true);

            // Deallocate OS interface structure (except if externally provided)
            MOS_FreeMemory(m_osInterface);
        }
    }
}

VphalState* VphalState::VphalStateFactory(
    PMOS_INTERFACE          pOsInterface,
    PMOS_CONTEXT            pOsDriverContext,
    MOS_STATUS              *peStatus)
{
    VphalState* vphalState = dynamic_cast<VphalState*>(VphalDevice::CreateFactory(pOsInterface, pOsDriverContext, peStatus));
    if(nullptr == vphalState)
    {
        VPHAL_PUBLIC_ASSERTMESSAGE("vphal state pointer is nullptr");
        *peStatus = MOS_STATUS_NULL_POINTER;
    }
    return vphalState;
}

//!
//! \brief    Get Status Report
//! \details  Get Status Report, will return back to app indicating if related frame id is done by gpu
//! \param    [out] pQueryReport
//!           Pointer to pQueryReport, the status query report array.
//! \param    [in] wStatusNum
//!           The size of array pQueryReport.
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
MOS_STATUS VphalState::GetStatusReport(
    PQUERY_STATUS_REPORT_APP       pQueryReport,
    uint16_t                       wStatusNum)
{
    MOS_STATUS                     eStatus = MOS_STATUS_SUCCESS;

#if (!EMUL)        // this function is dummy for emul
    uint32_t                       i;
    uint32_t                       uiTableLen;
    PVPHAL_STATUS_TABLE            pStatusTable;
    PMOS_CONTEXT                   pOsContext;
    uint32_t                       uiIndex;
    uint32_t                       uiNewHead;
    PVPHAL_STATUS_ENTRY            pStatusEntry;
    bool                           bMarkNotReadyForRemains = false;

    VPHAL_PUBLIC_CHK_NULL(pQueryReport);
    VPHAL_PUBLIC_CHK_NULL(m_osInterface);
    VPHAL_PUBLIC_CHK_NULL(m_osInterface->pOsContext);

    // it should be ok if we don't consider the null render
    // eNullRender = m_pOsInterface->pfnGetNullHWRenderFlags(m_pOsInterface);

    pOsContext           = m_osInterface->pOsContext;
    pStatusTable         = &m_statusTable;
    uiNewHead            = pStatusTable->uiHead; // uiNewHead start from previous head value
    // entry length from head to tail
    if (pStatusTable->uiCurrent < pStatusTable->uiHead)
    {
        uiTableLen = pStatusTable->uiCurrent + VPHAL_STATUS_TABLE_MAX_SIZE - pStatusTable->uiHead;
    }
    else
    {
        uiTableLen = pStatusTable->uiCurrent - pStatusTable->uiHead;
    }

    // step 1 - update pStatusEntry from driver if command associated with the dwTag is done by gpu
    for (i = 0; i < wStatusNum && i < uiTableLen; i++)
    {
        uint32_t    dwGpuTag; // hardware tag updated by gpu command pipectl
        bool        bDoneByGpu;
        bool        bFailedOnSubmitCmd;

        uiIndex            = (pStatusTable->uiHead + i) & (VPHAL_STATUS_TABLE_MAX_SIZE - 1);
        pStatusEntry       = &pStatusTable->aTableEntries[uiIndex];

        // for tasks using CM, different streamIndexes may be used
        uint32_t oldStreamIndex = m_osInterface->streamIndex;
        if (pStatusEntry->isStreamIndexSet)
        {
            m_osInterface->streamIndex = pStatusEntry->streamIndex;
        }

        if (bMarkNotReadyForRemains)
        {
            // the status is set as VPREP_NOTREADY while submitting commands
            pQueryReport[i].dwStatus         = pStatusEntry->dwStatus;
            pQueryReport[i].StatusFeedBackID = pStatusEntry->StatusFeedBackID;
            continue;
        }

        dwGpuTag           = m_osInterface->pfnGetGpuStatusSyncTag(m_osInterface, pStatusEntry->GpuContextOrdinal);
        bDoneByGpu         = (dwGpuTag >= pStatusEntry->dwTag);
        bFailedOnSubmitCmd = (pStatusEntry->dwStatus == VPREP_ERROR);

#if (_DEBUG || _RELEASE_INTERNAL)
        MOS_NULL_RENDERING_FLAGS NullRender = m_osInterface->pfnGetNullHWRenderFlags(m_osInterface);
        if (NullRender.Value != 0)
        {
            bDoneByGpu = true;
        }
#endif

        if (bFailedOnSubmitCmd)
        {
            uiNewHead = (uiIndex + 1) & (VPHAL_STATUS_TABLE_MAX_SIZE - 1);
        }
        else if (bDoneByGpu)
        {
            pStatusEntry->dwStatus = VPREP_OK;
            uiNewHead = (uiIndex + 1) & (VPHAL_STATUS_TABLE_MAX_SIZE - 1);
        }
        else
        {   // here we have the first not ready entry.
#if (LINUX || ANDROID)
            uiNewHead = (uiIndex + 1) & (VPHAL_STATUS_TABLE_MAX_SIZE - 1);
#else
            uiNewHead = uiIndex;
#endif

            bMarkNotReadyForRemains = true;
        }

        if (m_osInterface->pfnIsGPUHung(m_osInterface))
        {
            pStatusEntry->dwStatus = VPREP_NOTREADY;
        }

        pQueryReport[i].dwStatus         = pStatusEntry->dwStatus;
        pQueryReport[i].StatusFeedBackID = pStatusEntry->StatusFeedBackID;

        if (pStatusEntry->isStreamIndexSet)
        {
            m_osInterface->streamIndex = oldStreamIndex;
        }
    }
    pStatusTable->uiHead = uiNewHead;
    // step 2 - mark VPREP_NOTAVAILABLE for unused entry
    for (/* continue from previous i */; i < wStatusNum; i++)
    {
        pQueryReport[i].dwStatus         = VPREP_NOTAVAILABLE;
        pQueryReport[i].StatusFeedBackID = 0;
    }

finish:
#else
    MOS_UNUSED(pQueryReport);
    MOS_UNUSED(wStatusNum);
#endif // end (!EMUL && !ANDROID)
    return eStatus;
}

//!
//! \brief    Get Status Report's entry length from head to tail
//! \details  Get Status Report's entry length from head to tail
//! \param    [out] puiLength
//!           Pointer to the entry length
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VphalState::GetStatusReportEntryLength(
    uint32_t*                      puiLength)
{
    MOS_STATUS                     eStatus = MOS_STATUS_SUCCESS;
#if(!EMUL)        // this function is dummy for emul
    PVPHAL_STATUS_TABLE            pStatusTable;

    VPHAL_PUBLIC_CHK_NULL(puiLength);

    pStatusTable = &m_statusTable;

    // entry length from head to tail
    if (pStatusTable->uiCurrent < pStatusTable->uiHead)
    {
        *puiLength = pStatusTable->uiCurrent + VPHAL_STATUS_TABLE_MAX_SIZE - pStatusTable->uiHead;
    }
    else
    {
        *puiLength = pStatusTable->uiCurrent - pStatusTable->uiHead;
    }
finish:
#else
    MOS_UNUSED(puiLength);
#endif
    return eStatus;
}

MOS_STATUS VphalState::GetVpMhwInterface(
    VP_MHWINTERFACE &vpMhwinterface)
{
    MOS_STATUS eStatus              = MOS_STATUS_SUCCESS;

    vpMhwinterface.m_platform       = m_platform;
    vpMhwinterface.m_waTable        = m_waTable;
    vpMhwinterface.m_skuTable       = m_skuTable;
    vpMhwinterface.m_osInterface    = m_osInterface;
    vpMhwinterface.m_renderHal      = m_renderHal;
    vpMhwinterface.m_veboxInterface = m_veboxInterface;
    vpMhwinterface.m_sfcInterface   = m_sfcInterface;
    vpMhwinterface.m_cpInterface    = m_cpInterface;
    vpMhwinterface.m_mhwMiInterface = m_renderHal->pMhwMiInterface;
    vpMhwinterface.m_statusTable    = &m_statusTable;

    return eStatus;
}

//!
//! \brief    Put GPU context entry
//! \details  Put GPU context entry in the m_gpuContextCheckList
//! \param    MOS_GPU_CONTEXT mosGpuConext
//!           [in] Mos GPU context
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VphalState::AddGpuContextToCheckList(
    MOS_GPU_CONTEXT mosGpuConext) 
{
#if !EMUL 
    MOS_GPU_CONTEXT originalGpuCtxOrdinal = m_osInterface->CurrentGpuContextOrdinal;
    if (mosGpuConext != originalGpuCtxOrdinal)
    {
        // Set GPU context temporarily
        VPHAL_PUBLIC_CHK_STATUS_RETURN(m_osInterface->pfnSetGpuContext(
            m_osInterface,
            mosGpuConext));
    }
    VPHAL_GPU_CONTEXT_ENTRY tmpEntry;
    tmpEntry.gpuCtxForMos     = mosGpuConext;
    tmpEntry.gpuContextHandle = m_osInterface->CurrentGpuContextHandle;
    tmpEntry.pGpuContext      = m_osInterface->pfnGetGpuContextbyHandle(m_osInterface, m_osInterface->CurrentGpuContextHandle);
    m_gpuContextCheckList.push_back(tmpEntry);

    if (mosGpuConext != originalGpuCtxOrdinal)
    {
        //Recover original settings
        VPHAL_PUBLIC_CHK_STATUS_RETURN(m_osInterface->pfnSetGpuContext(
            m_osInterface,
            originalGpuCtxOrdinal));
    }
#endif

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Destroy GPU context entry with invalid handle
//! \details  Release these GPU context overwritten by MediaContext
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VphalState::DestroyGpuContextWithInvalidHandle() 
{
#if !EMUL
    MOS_GPU_CONTEXT originalGpuCtxOrdinal = m_osInterface->CurrentGpuContextOrdinal;
    for (auto &curGpuEntry : m_gpuContextCheckList)
    {
        //Failure in switching GPU Context indicates that we can't find and release gpu context in normal flow later
        //So if failed to switch GPU Context, just need to check GPU context, and release it here
        //If switch GPU Context successfully, need to check both handle and GPU context
        if ((MOS_FAILED(m_osInterface->pfnSetGpuContext(m_osInterface, curGpuEntry.gpuCtxForMos)) || 
            m_osInterface->CurrentGpuContextHandle != curGpuEntry.gpuContextHandle) &&
            m_osInterface->pfnGetGpuContextbyHandle(m_osInterface, curGpuEntry.gpuContextHandle) == curGpuEntry.pGpuContext)
        {
            m_osInterface->pfnWaitForCmdCompletion(m_osInterface->osStreamState, curGpuEntry.gpuContextHandle);

            m_osInterface->pfnDestroyGpuContextByHandle(m_osInterface, curGpuEntry.gpuContextHandle);
        }
    }
    if (m_osInterface->CurrentGpuContextOrdinal != originalGpuCtxOrdinal)
    {
        //Recover original settings
        if (m_osInterface->pfnSetGpuContext(m_osInterface, originalGpuCtxOrdinal) != MOS_STATUS_SUCCESS)
        {
            VPHAL_PUBLIC_NORMALMESSAGE("Have not recovered original GPU Context settings in m_osInterface");
        }
    }
#endif

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Check whether GPU context is reused or not
//! \details  Check whether GPU context is reused or not
//! \param    MOS_GPU_CONTEXT mosGpuConext
//!           [in] Mos GPU context
//! \return   bool
//!           Return true if is reused, otherwise false
//!
bool VphalState::IsGpuContextReused(
    MOS_GPU_CONTEXT mosGpuContext)
{
    bool reuseContextFlag = false;
#if !EMUL
    MOS_GPU_CONTEXT originalGpuCtxOrdinal = m_osInterface->CurrentGpuContextOrdinal;
    if (MOS_SUCCEEDED(m_osInterface->pfnSetGpuContext(m_osInterface, mosGpuContext)))
    {
        VPHAL_PUBLIC_NORMALMESSAGE("Reuse mos GPU context %d. GPU handle %d", (uint32_t)mosGpuContext, m_osInterface->CurrentGpuContextHandle);
        reuseContextFlag = true;
    }

    if (originalGpuCtxOrdinal < MOS_GPU_CONTEXT_MAX &&
        m_osInterface->CurrentGpuContextOrdinal != originalGpuCtxOrdinal)
    {
        //Recover original settings
        if (m_osInterface->pfnSetGpuContext(m_osInterface, originalGpuCtxOrdinal) != MOS_STATUS_SUCCESS)
        {
            VPHAL_PUBLIC_NORMALMESSAGE("Have not recovered original GPU Context settings in m_osInterface");
        }
    }
#endif
    return reuseContextFlag;
}
