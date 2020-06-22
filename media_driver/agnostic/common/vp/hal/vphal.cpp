/*
* Copyright (c) 2009-2018, Intel Corporation
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
#include "mhw_vebox.h"
#include "renderhal.h"
#include "vphal_renderer.h"
#include "mos_solo_generic.h"
#include "media_interfaces_vphal.h"
#include "media_interfaces_mhw.h"

void VphalFeatureReport::InitReportValue()
{
    IECP                =   false;
    IEF                 =   false;
    Denoise             =   false;
    ChromaDenoise       =   false;
    DeinterlaceMode     =   VPHAL_DI_REPORT_PROGRESSIVE;
    ScalingMode         =   VPHAL_SCALING_NEAREST;
    OutputPipeMode      =   VPHAL_OUTPUT_PIPE_MODE_COMP;
    VPMMCInUse          =   false;
    RTCompressible      =   false;
    RTCompressMode      =   0;
    FFDICompressible    =   false;
    FFDICompressMode    =   0;
    FFDNCompressible    =   false;
    FFDNCompressMode    =   0;
    STMMCompressible    =   false;
    STMMCompressMode    =   0;
    ScalerCompressible  =   false;
    ScalerCompressMode  =   0;
    PrimaryCompressible =   false;
    PrimaryCompressMode =   0;
    CompositionMode     =   VPHAL_NO_COMPOSITION;
    DiScdMode           =   false;
    VEFeatureInUse      =   false;
    HDRMode             =   VPHAL_HDR_MODE_NONE;
}

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
    RENDERHAL_SETTINGS          RenderHalSettings;
    MOS_GPU_NODE                VeboxGpuNode;
    MOS_GPU_CONTEXT             VeboxGpuContext;
    MOS_STATUS                  eStatus;

    VPHAL_PUBLIC_CHK_NULL(pVpHalSettings);
    VPHAL_PUBLIC_CHK_NULL(m_renderHal);

    // Create Render GPU Context
    {
        MOS_GPUCTX_CREATOPTIONS createOption;
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

    // Register Render GPU context with the event
    VPHAL_PUBLIC_CHK_STATUS(m_osInterface->pfnRegisterBBCompleteNotifyEvent(
        m_osInterface,
        m_renderGpuContext));

    if (MEDIA_IS_SKU(m_skuTable, FtrVERing) && m_veboxInterface)
    {
        GpuNodeLimit.bSfcInUse         = MEDIA_IS_SKU(m_skuTable, FtrSFCPipe);

        // Check GPU Node decide logic together in this function
        VPHAL_HW_CHK_STATUS(m_veboxInterface->FindVeboxGpuNodeToUse(&GpuNodeLimit));

        VeboxGpuNode    = (MOS_GPU_NODE)(GpuNodeLimit.dwGpuNodeToUse);
        VeboxGpuContext = (VeboxGpuNode == MOS_GPU_NODE_VE) ? MOS_GPU_CONTEXT_VEBOX : MOS_GPU_CONTEXT_VEBOX2;

        // Create VEBOX/VEBOX2 Context
        VPHAL_PUBLIC_CHK_STATUS(m_veboxInterface->CreateGpuContext(
            m_osInterface,
            VeboxGpuContext,
            VeboxGpuNode));

        // Register Vebox GPU context with the Batch Buffer completion event
        // Ignore if creation fails
        VPHAL_PUBLIC_CHK_STATUS(m_osInterface->pfnRegisterBBCompleteNotifyEvent(
            m_osInterface,
            MOS_GPU_CONTEXT_VEBOX));
    }

    // Allocate and initialize HW states
    RenderHalSettings.iMediaStates  = pVpHalSettings->mediaStates;
    VPHAL_PUBLIC_CHK_STATUS(m_renderHal->pfnInitialize(m_renderHal, &RenderHalSettings));

    if (m_veboxInterface &&
        m_veboxInterface->m_veboxSettings.uiNumInstances > 0 &&
        m_veboxInterface->m_veboxHeap == nullptr)
    {
        // Allocate VEBOX Heap
        VPHAL_PUBLIC_CHK_STATUS(m_veboxInterface->CreateHeap());
    }

    // Create renderer
    VPHAL_RENDER_CHK_STATUS(CreateRenderer());

    // Allocate and initialize renderer states
    VPHAL_PUBLIC_CHK_STATUS(m_renderer->Initialize(pVpHalSettings));

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
    if (pSurf->Format == Format_AYUV || pSurf->Format == Format_AUYV)
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
            return true;
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
        }

        //Free the temporary surface
        pRenderer->GetOsInterface()->pfnFreeResource(pRenderer->GetOsInterface(), &(pIntermediateSurface->OsResource));

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
//! \param    [in] pOsDriverContext
//!           OS driver context (UMD context, pShared, ...)
//! \param    [in,out] peStatus
//!           Pointer to the MOS_STATUS flag.
//!           Will assign this flag to MOS_STATUS_SUCCESS if successful, otherwise failed
//!
VphalState::VphalState(
        PMOS_INTERFACE          pOsInterface,
        PMOS_CONTEXT            pOsDriverContext,
        MOS_STATUS              *peStatus) :
        m_gpuAppTaskEvent(nullptr),
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

    m_renderHal = (PRENDERHAL_INTERFACE)MOS_AllocAndZeroMemory(sizeof(*m_renderHal));
    VPHAL_PUBLIC_CHK_NULL(m_renderHal);
    VPHAL_PUBLIC_CHK_STATUS(RenderHal_InitInterface(
        m_renderHal,
        &m_cpInterface,
        m_osInterface));

    if (MEDIA_IS_SKU(m_skuTable, FtrVERing) ||
        MEDIA_IS_SKU(m_skuTable, FtrSFCPipe))
    {
        MhwInterfaces *mhwInterfaces = nullptr;
        MhwInterfaces::CreateParams params;
        MOS_ZeroMemory(&params, sizeof(params));
        params.Flags.m_sfc   = MEDIA_IS_SKU(m_skuTable, FtrSFCPipe);
        params.Flags.m_vebox = MEDIA_IS_SKU(m_skuTable, FtrVERing);

        mhwInterfaces = MhwInterfaces::CreateFactory(params, pOsInterface);
        if (mhwInterfaces)
        {
            SetMhwVeboxInterface(mhwInterfaces->m_veboxInterface);
            SetMhwSfcInterface(mhwInterfaces->m_sfcInterface);

            // MhwInterfaces always create CP and MI interfaces, so we have to delete those we don't need.
            MOS_Delete(mhwInterfaces->m_miInterface);
            Delete_MhwCpInterface(mhwInterfaces->m_cpInterface);
            mhwInterfaces->m_cpInterface = nullptr;
            MOS_Delete(mhwInterfaces);
        }
        else
        {
            VPHAL_DEBUG_ASSERTMESSAGE("Allocate MhwInterfaces failed");
            eStatus = MOS_STATUS_NO_SPACE;
        }
    }

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
            }
        }
        MOS_FreeMemory(m_renderHal);
    }

    if (m_cpInterface)
    {
        Delete_MhwCpInterface(m_cpInterface);
        m_cpInterface = nullptr;
    }

    if (m_sfcInterface)
    {
        MOS_Delete(m_sfcInterface);
        m_sfcInterface = nullptr;
    }

    if (m_veboxInterface)
    {
        eStatus = m_veboxInterface->DestroyHeap();
        MOS_Delete(m_veboxInterface);
        m_veboxInterface = nullptr;
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            VPHAL_PUBLIC_ASSERTMESSAGE("Failed to destroy Vebox Interface, eStatus:%d.\n", eStatus);
        }
    }

    // Destroy OS interface objects (CBs, etc)
    if (m_osInterface)
    {
        if (m_osInterface->bDeallocateOnExit)
        {
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
    return VphalDevice::CreateFactory(pOsInterface, pOsDriverContext, peStatus);
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
    uiTableLen           = (pStatusTable->uiCurrent - pStatusTable->uiHead) & (VPHAL_STATUS_TABLE_MAX_SIZE - 1);

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

#if (LINUX || ANDROID)
        dwGpuTag           = pOsContext->GetGPUTag(m_osInterface, pStatusEntry->GpuContextOrdinal);
#else
        dwGpuTag           = pOsContext->GetGPUTag(pOsContext->GetGpuContextHandle(pStatusEntry->GpuContextOrdinal, m_osInterface->streamIndex));
#endif
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
    *puiLength = (pStatusTable->uiCurrent - pStatusTable->uiHead) & (VPHAL_STATUS_TABLE_MAX_SIZE - 1);
finish:
#else
    MOS_UNUSED(puiLength);
#endif
    return eStatus;
}
