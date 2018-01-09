/*
* Copyright (c) 2011-2018, Intel Corporation
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
//! \file     vphal_renderer.cpp
//! \brief    VPHAL top level rendering component and the entry to low level renderers
//! \details  The top renderer is responsible for coordinating the sequence of calls to low level renderers, e.g. DNDI or Comp
//!
#include "vphal_renderer.h"
#include "vphal_debug.h"
#include "vpkrnheader.h"
#include "vphal_render_composite.h"

// Slice Shutdown User feature keys
#define VPHAL_SSD_CONTROL    "SSD Control"

//==<FUNCTIONS>=================================================================

//!
//! \brief    Initialize AVS parameters shared by Renderers
//! \details  Initialize the members of the AVS parameter and allocate memory for its coefficient tables
//! \param    [in,out] pAVS_Params
//!           Pointer to MHW AVS parameter
//! \param    [in] uiYCoeffTableSize
//!           Size of the Y coefficient table
//! \param    [in] uiUVCoeffTableSize
//!           Size of the UV coefficient table
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_RenderInitAVSParams(
    PMHW_AVS_PARAMS     pAVS_Params,
    uint32_t            uiYCoeffTableSize,
    uint32_t            uiUVCoeffTableSize)
{
    MOS_STATUS  eStatus;
    int32_t     size;
    char*       ptr;

    VPHAL_RENDER_ASSERT(pAVS_Params);
    VPHAL_RENDER_ASSERT(uiYCoeffTableSize > 0);
    VPHAL_RENDER_ASSERT(uiUVCoeffTableSize > 0);
    eStatus = MOS_STATUS_SUCCESS;

    // Init AVS parameters
    pAVS_Params->Format    = Format_None;
    pAVS_Params->fScaleX   = 0.0F;
    pAVS_Params->fScaleY   = 0.0F;
    pAVS_Params->piYCoefsX = nullptr;

    // Allocate AVS coefficients, One set each for X and Y
    size = (uiYCoeffTableSize + uiUVCoeffTableSize) * 2;

    ptr = (char*)MOS_AllocAndZeroMemory(size);
    if (ptr == nullptr)
    {
        VPHAL_RENDER_ASSERTMESSAGE("No memory to allocate AVS coefficient tables.");
        eStatus = MOS_STATUS_NO_SPACE;
        goto finish;
    }

    pAVS_Params->piYCoefsX = (int32_t*)ptr; 

    ptr += uiYCoeffTableSize;
    pAVS_Params->piUVCoefsX = (int32_t*)ptr;

    ptr += uiUVCoeffTableSize;
    pAVS_Params->piYCoefsY  = (int32_t*)ptr;

    ptr += uiYCoeffTableSize;
    pAVS_Params->piUVCoefsY = (int32_t*)ptr;

finish:
    return eStatus;
}

//!
//! \brief    Destroy AVS parameters shared by Renderers
//! \details  Free the memory of AVS parameter's coefficient tables
//! \param    [in,out] pAVS_Params
//!           Pointer to VPHAL AVS parameter
//! \return   void
//!
void VpHal_RenderDestroyAVSParams(
    PMHW_AVS_PARAMS   pAVS_Params)
{
    MOS_SafeFreeMemory(pAVS_Params->piYCoefsX);
    pAVS_Params->piYCoefsX = nullptr;
}

//!
//! \brief    Get the size in byte from that in pixel
//! \details  Size_in_byte = size_in_pixel x byte/pixel
//! \param    [in] pOsInterface
//!           Pointer to OS interface
//! \param    [in] Format
//!           The format which determines the value of byte/pixel
//! \param    [in] dwPixels
//!           The size in pixel
//! \return   uint32_t
//!           Return the size in byte
//!
uint32_t VpHal_PixelsToBytes(
    PMOS_INTERFACE      pOsInterface,
    MOS_FORMAT          Format,
    uint32_t            dwPixels)
{
    MOS_STATUS eStatus;
    uint32_t   iBpp;
    uint32_t   dwBpp;

    eStatus = MOS_STATUS_SUCCESS;
    dwBpp   = 0;
    VPHAL_RENDER_CHK_STATUS(pOsInterface->pfnGetBitsPerPixel(pOsInterface, Format, &iBpp));
    dwBpp   = dwPixels * (iBpp>>3);

finish:
    return dwBpp;
}

//!
//! \brief    Save/Restore fwd references for the primary
//! \details  Based on the flag passed in to save or restore the forward references
//!           of the primary
//! \param    [in,out] pRenderer
//!           VPHAL renderer pointer
//! \param    [in,out] pPrimarySurf
//!           Pointer to the primary surface
//! \param    [in] bSave
//!           Save - true or restore - false the fwd references
//! \return   void
//!
void VpHal_SaveRestorePrimaryFwdRefs(
    VphalRenderer           *pRenderer,
    PVPHAL_SURFACE          pPrimarySurf,
    bool                    bSave)
{
    PVPHAL_SURFACE  pFwdRef;
    uint32_t        uiSources;
    uint32_t        uiIndex;

    VPHAL_RENDER_ASSERT(pRenderer);
    VPHAL_RENDER_ASSERT(pPrimarySurf && (pPrimarySurf->SurfType == SURF_IN_PRIMARY));

    pFwdRef     = nullptr;
    uiSources   = 0;
    uiIndex     = 0;

    if (bSave)
    {
        pFwdRef = pPrimarySurf->pFwdRef;

        while(pFwdRef)
        {
            pRenderer->pPrimaryFwdRef[uiIndex] = pFwdRef;
            pFwdRef                            = pFwdRef->pFwdRef;
            uiIndex++;
            if (uiIndex >= VPHAL_MAX_FUTURE_FRAMES)
            {
                break;
            }
        }
    }
    else
    {
        pFwdRef = pPrimarySurf;

        while (pRenderer->pPrimaryFwdRef[uiIndex])
        {
            pFwdRef->pFwdRef                   = pRenderer->pPrimaryFwdRef[uiIndex];
            pRenderer->pPrimaryFwdRef[uiIndex] = nullptr;
            pFwdRef                            = pFwdRef->pFwdRef;
            uiIndex++;
            if (uiIndex >= VPHAL_MAX_FUTURE_FRAMES)
            {
                break;
            }
        }
    }
}

//!
//! \brief    Align the src/dst surface rectangle and surface width/height
//! \details  The surface rects and width/height need to be aligned according to the surface format
//! \param    [in,out] pSurface
//!           Pointer to the surface
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_RndrRectSurfaceAlignment(
    PVPHAL_SURFACE       pSurface)
{
    uint16_t   wWidthAlignUnit;
    uint16_t   wHeightAlignUnit;
    MOS_STATUS eStatus;

    eStatus = MOS_STATUS_SUCCESS;

    VpHal_RndrGetAlignUnit(&wWidthAlignUnit, &wHeightAlignUnit, pSurface->Format);

    // The source rectangle is floored to the aligned unit to 
    // get rid of invalid data(ex: an odd numbered src rectangle with NV12 format 
    // will have invalid UV data for the last line of Y data).
    pSurface->rcSrc.bottom = MOS_ALIGN_FLOOR((uint32_t)pSurface->rcSrc.bottom, wHeightAlignUnit);
    pSurface->rcSrc.right  = MOS_ALIGN_FLOOR((uint32_t)pSurface->rcSrc.right, wWidthAlignUnit);

    pSurface->rcSrc.top    = MOS_ALIGN_CEIL((uint32_t)pSurface->rcSrc.top, wHeightAlignUnit);
    pSurface->rcSrc.left   = MOS_ALIGN_CEIL((uint32_t)pSurface->rcSrc.left, wWidthAlignUnit);

    // The Destination rectangle is rounded to the upper alignment unit to prevent the loss of 
    // data which was present in the source rectangle
    pSurface->rcDst.bottom = MOS_ALIGN_CEIL((uint32_t)pSurface->rcDst.bottom, wHeightAlignUnit);
    pSurface->rcDst.right  = MOS_ALIGN_CEIL((uint32_t)pSurface->rcDst.right, wWidthAlignUnit);

    pSurface->rcDst.top    = MOS_ALIGN_FLOOR((uint32_t)pSurface->rcDst.top, wHeightAlignUnit);
    pSurface->rcDst.left   = MOS_ALIGN_FLOOR((uint32_t)pSurface->rcDst.left, wWidthAlignUnit);

    if (pSurface->SurfType == SURF_OUT_RENDERTARGET)
    {
        pSurface->dwHeight = MOS_ALIGN_CEIL(pSurface->dwHeight, wHeightAlignUnit);
        pSurface->dwWidth  = MOS_ALIGN_CEIL(pSurface->dwWidth, wWidthAlignUnit);
    }
    else
    {
        pSurface->dwHeight = MOS_ALIGN_FLOOR(pSurface->dwHeight, wHeightAlignUnit);
        pSurface->dwWidth  = MOS_ALIGN_FLOOR(pSurface->dwWidth, wWidthAlignUnit);
    }
    
    if ((pSurface->rcSrc.top  == pSurface->rcSrc.bottom) ||
        (pSurface->rcSrc.left == pSurface->rcSrc.right)  ||
        (pSurface->rcDst.top  == pSurface->rcDst.bottom) ||
        (pSurface->rcDst.left == pSurface->rcDst.right)  ||
        (pSurface->dwWidth    == 0)                      ||
        (pSurface->dwHeight   == 0))
    {
        VPHAL_RENDER_ASSERTMESSAGE("Surface Parameter is invalid.");
        eStatus = MOS_STATUS_INVALID_PARAMETER;	
    }
    
    return eStatus;
}

//!
//! \brief    Prepare input surface list for top level render processing
//! \details  Prepare the inputs, e.g. adjust src/dst rectangles of stereo input or allocate
//!           and copy intermediate surfaces.
//! \param    [in,out] pRenderParams
//!           Pointer to VPHAL render parameter
//! \param    [in,out] pSrcLeft
//!           Pointer to left frame list
//! \param    [in,out] pSrcRight
//!           Pointer to right frame list
//! \param    [out] puiRenderPasses
//!           Pointer to times of the rendering.
//!           The value is 2 for S3D and 1 for the other cases.
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VphalRenderer::PrepareSources(
    PVPHAL_RENDER_PARAMS    pRenderParams,
    PVPHAL_SURFACE         *pSrcLeft,
    PVPHAL_SURFACE         *pSrcRight,
    uint32_t               *puiRenderPasses)
{
    MOS_STATUS      eStatus;
    PVPHAL_SURFACE  pcSrc;
    uint32_t        uiLeftCount;
    uint32_t        uiRightCount;
    uint32_t        uiSources;
    uint32_t        uiIndex;
    PMOS_RESOURCE   ppOsResource[VPHAL_MAX_SOURCES] = { nullptr };

    eStatus         = MOS_STATUS_SUCCESS;
    uiLeftCount     = 0;
    uiRightCount    = 0;
    uiIndex         = 0;
    uiSources       = 0;

    VPHAL_RENDER_CHK_NULL(m_pOsInterface);

    for (uiSources=0, uiIndex=0;
         (uiSources < pRenderParams->uSrcCount) && (uiIndex < VPHAL_MAX_SOURCES);
         uiIndex++)
    {
        pcSrc = pRenderParams->pSrc[uiIndex];

        if (pcSrc == nullptr)
        {
            continue;
        }

        ppOsResource[uiSources] = &pcSrc->OsResource;

        pSrcLeft[uiLeftCount++] = pcSrc;

        uiSources++;
    }

    VPHAL_RENDER_ASSERT(uiRightCount == 0);

    pRenderParams->uSrcCount = uiLeftCount;
    *puiRenderPasses         = 1;

finish:
    VPHAL_RENDER_ASSERT(eStatus == MOS_STATUS_SUCCESS);
    return eStatus;
}

//!
//! \brief    Get surface info for all input source
//! \details  Get surface info for the input surface and its reference surfaces
//! \param    [in] pRenderParams
//!           Pointer to VPHAL render parameter
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VphalRenderer::GetSurfaceInfoForSrc(
    PVPHAL_RENDER_PARAMS    pRenderParams)
{
    MOS_STATUS              eStatus;
    PVPHAL_SURFACE          pSrcSurface;                                        // Current source surface
    PVPHAL_SURFACE          pSurface;                                           // Ptr to surface
    PVPHAL_SURFACE          pTarget;                                            // Render Target
    uint32_t                uiSources;                                          // Number of Sources
    uint32_t                uiIndex;                                            // Current source index
    uint32_t                i;
    VPHAL_GET_SURFACE_INFO  Info;

    eStatus         = MOS_STATUS_SUCCESS;
    pSrcSurface     = nullptr;
    pTarget         = pRenderParams->pTarget[0];

    // Loop through the sources
    for (uiSources = 0, uiIndex = 0;
        uiSources < pRenderParams->uSrcCount && uiIndex < VPHAL_MAX_SOURCES;
        uiIndex++)
    {
        pSrcSurface = pRenderParams->pSrc[uiIndex];

        if (pSrcSurface == nullptr)
        {
            continue;
        }
        uiSources++;

        if (Mos_ResourceIsNull(&pSrcSurface->OsResource))
        {
            VPHAL_RENDER_ASSERTMESSAGE("Input resource is not valid.");
            eStatus = MOS_STATUS_UNKNOWN;
            goto finish;
        }

        MOS_ZeroMemory(&Info, sizeof(VPHAL_GET_SURFACE_INFO));

        VPHAL_RENDER_CHK_STATUS(VpHal_GetSurfaceInfo(
            m_pOsInterface,
            &Info,
            pSrcSurface));

        // Get resource info for Backward References, if any
        pSurface = pSrcSurface->pBwdRef;
        for (i = 0; i < pSrcSurface->uBwdRefCount; i++)
        {
            VPHAL_RENDER_ASSERT(pSurface); // Must have valid reference pointer
            if (pSurface)
            {
                MOS_ZeroMemory(&Info, sizeof(VPHAL_GET_SURFACE_INFO));

                VPHAL_RENDER_CHK_STATUS(VpHal_GetSurfaceInfo(
                    m_pOsInterface,
                    &Info,
                    pSurface));

                // point to the next bwd ref
                pSurface = pSurface->pBwdRef;
            }
        }

        // Get resource info for Forward References, if any
        pSurface = pSrcSurface->pFwdRef;
        for (i = 0; i < pSrcSurface->uFwdRefCount; i++)
        {
            VPHAL_RENDER_ASSERT(pSurface); // Must have valid reference pointer
            if (pSurface)
            {
                MOS_ZeroMemory(&Info, sizeof(VPHAL_GET_SURFACE_INFO));

                VPHAL_RENDER_CHK_STATUS(VpHal_GetSurfaceInfo(
                    m_pOsInterface,
                    &Info,
                    pSurface));

                // point to the next fwd ref
                pSurface = pSurface->pFwdRef;
            }
        }
    }

finish:
    return eStatus;
}

//!
//! \brief    Adjust surface parameter
//! \param    [in] pRenderParams
//!           Pointer to VPHAL render parameter
//! \param    [in,out] pSrcSurface
//!           Pointer to VPHAL surface
//! \param    [in] pGtSystemInfo
//!           Pointer to GT system information structure
//! \param    [in] bHybridDecoderFlag
//!           Hybrid Decoder or not
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VphalRenderer::AdjustSurfaceParam(
    PVPHAL_RENDER_PARAMS    pRenderParams,
    PVPHAL_SURFACE          pSrcSurface,
    MEDIA_SYSTEM_INFO       *pGtSystemInfo,
    bool                    bHybridDecoderFlag)
{
    MOS_STATUS              eStatus = MOS_STATUS_SUCCESS;
 
    VPHAL_RENDER_CHK_NULL(pSrcSurface);
    VPHAL_RENDER_CHK_NULL(pRenderParams);
    VPHAL_RENDER_CHK_NULL(pGtSystemInfo);

    // Disable VP features for 4K (Sku flag or Hybrid)
    if (pSrcSurface->rcSrc.bottom >= pSrcSurface->rcSrc.top + VPHAL_RNDR_4K_HEIGHT)
    {
        // VEBOX timing on KBL ULT might be about 2x ms when DN is on, and it seems to be
        // in relation to the lag problem on WMP after scaling up & down the playback window.
        // Disable DN for 4K to resolve this phenomenon.
        if (bSkuDisableDNFor4K &&
            pSrcSurface->pDenoiseParams)
        {
            pSrcSurface->pDenoiseParams->bAutoDetect        = false;
            pSrcSurface->pDenoiseParams->bEnableChroma      = false;
            pSrcSurface->pDenoiseParams->bEnableLuma        = false;
        }

        // Disable features if needed
        if (bSkuDisableVpFor4K || bHybridDecoderFlag)
        {
            // Denoise
            if (pSrcSurface->pDenoiseParams)
            {
                pSrcSurface->pDenoiseParams->bAutoDetect    = false;
                pSrcSurface->pDenoiseParams->bEnableChroma  = false;
                pSrcSurface->pDenoiseParams->bEnableLuma    = false;
            }

            // Sharpness(IEF)
            if (pSrcSurface->pIEFParams)
            {
                pSrcSurface->pIEFParams->bEnabled           = false;
            }

            // STE, TCC
            if (pSrcSurface->pColorPipeParams)
            {
                pSrcSurface->pColorPipeParams->bEnableSTE   = false;
                pSrcSurface->pColorPipeParams->bEnableTCC   = false;
            }
        }
    }

    // IEF is only for Y luma component
    if (IS_RGB_FORMAT(pSrcSurface->Format) && pSrcSurface->pIEFParams)
    {
        VPHAL_RENDER_NORMALMESSAGE("IEF is only for Y luma component, and IEF is always disabled for RGB input.");
        pSrcSurface->pIEFParams->bEnabled = false;
        pSrcSurface->pIEFParams->fIEFFactor = 0.0f;
    }

finish:
    return eStatus;
}

//!
//! \brief    Process render parameter
//! \param    [in,out] pRenderParams
//!           Pointer to VPHAL render parameter
//! \param    [in,out] pRenderPassData
//!           Pointer to the VPHAL render pass data
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VphalRenderer::ProcessRenderParameter(
    PVPHAL_RENDER_PARAMS    pRenderParams,
    RenderpassData          *pRenderPassData)
{
    MOS_STATUS              eStatus;
    MEDIA_SYSTEM_INFO       *pGtSystemInfo;
    uint32_t                uiIndex;
    PVPHAL_SURFACE          pSrcSurface;
    PVPHAL_SURFACE          pPrimarySurface;
    PVPHAL_PROCAMP_PARAMS   pProcampParams;
    bool                    bSingleSliceMode;
    bool                    bSliceReconfig;
    bool                    bHybridDecoderFlag;
    int32_t                 decoderFlag;

#ifdef ANDROID
    RECT                    PrimOrigDstRect;
    bool                    bDstRectWANeeded = false;
#endif

    eStatus             = MOS_STATUS_SUCCESS;
    bHybridDecoderFlag  = false;
    bSliceReconfig      = false;
    pPrimarySurface     = nullptr;

    // Get the hybrid decoder flag
    VPHAL_RENDER_CHK_STATUS(m_pOsInterface->pfnGetHybridDecoderRunningFlag(m_pOsInterface, &decoderFlag));
    bHybridDecoderFlag = decoderFlag ? true : false;

    // Get the GT system information
    pGtSystemInfo = m_pOsInterface->pfnGetGtSystemInfo(m_pOsInterface);
    VPHAL_RENDER_CHK_NULL(pGtSystemInfo);

    pRender[VPHAL_RENDER_ID_COMPOSITE]->SetStatusReportParams(this, pRenderParams);
    pRender[VPHAL_RENDER_ID_VEBOX+uiCurrentChannel]->SetStatusReportParams(this, pRenderParams);

    // Loop through the sources
    for (uiIndex = 0;
         uiIndex < VPHAL_MAX_SOURCES && uiIndex < pRenderParams->uSrcCount;
         uiIndex++)
    {
        pSrcSurface = pRenderParams->pSrc[uiIndex];

        if (pSrcSurface == nullptr)
        {
            continue;
        }

        // We need to block invalid frame sizes of 0 or negative values from
        // entering the VP render core since IVB hardware had problem of
        // handling these cases.  EU payload of the same values, U, V,
        // deltaU, deltaV, are all 0s, and are the same for 16 thread entries,
        // which will be discarded by the kernel.
        if ((pSrcSurface->rcSrc.top    >= pSrcSurface->rcSrc.bottom)    ||
            (pSrcSurface->rcSrc.left   >= pSrcSurface->rcSrc.right)     ||
            (pSrcSurface->rcDst.top    >= pSrcSurface->rcDst.bottom)    ||
            (pSrcSurface->rcDst.left   >= pSrcSurface->rcDst.right))
        {
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            VPHAL_RENDER_ASSERTMESSAGE("Invalid Rectangle Values.");
            goto finish;
        }

#ifdef ANDROID
        // If the dst rect of primary layer is exactly covered by sublayers, say primary.left == sublayer.left,
        // the primary layer should not be seen from the left side. But VpHal_RndrRectSurfaceAlignment() might adjust
        // the primary layer dst rect for alignment. It looks like the primary layer is shifted left for one or two pixels,
        // and user will see a thin line on the left side, which is a bad user experience.
        // In such cases, just make the sublayer's dst rect still aligned with primary layer.
        if (bDstRectWANeeded)
        {
            pSrcSurface->rcDst.left   = (pSrcSurface->rcDst.left   == PrimOrigDstRect.left  ) ? pPrimarySurface->rcDst.left   : pSrcSurface->rcDst.left;
            pSrcSurface->rcDst.top    = (pSrcSurface->rcDst.top    == PrimOrigDstRect.top   ) ? pPrimarySurface->rcDst.top    : pSrcSurface->rcDst.top;
            pSrcSurface->rcDst.right  = (pSrcSurface->rcDst.right  == PrimOrigDstRect.right ) ? pPrimarySurface->rcDst.right  : pSrcSurface->rcDst.right;
            pSrcSurface->rcDst.bottom = (pSrcSurface->rcDst.bottom == PrimOrigDstRect.bottom) ? pPrimarySurface->rcDst.bottom : pSrcSurface->rcDst.bottom;
        }
#endif

        if (pSrcSurface->SurfType == SURF_IN_PRIMARY)
        {
#ifdef ANDROID
            bDstRectWANeeded = true;
            PrimOrigDstRect  = pSrcSurface->rcDst;
#endif
            pRenderPassData->pPrimarySurface    = pPrimarySurface = pSrcSurface;
            pRenderPassData->uiPrimaryIndex     = uiIndex;

            // align rectangle and source surface
            VPHAL_RENDER_CHK_STATUS(VpHal_RndrRectSurfaceAlignment(pSrcSurface));

            // update max Src rect in both pRenderer and primary surface
            VpHal_RenderInitMaxRect(this, pSrcSurface);
        }

        // Add Procamp limitation before Render pass selected
        // Brightness[-100.0,100.0], Contrast & Saturation[0.0,10.0]
        pProcampParams = pSrcSurface->pProcampParams;
        if (pProcampParams && pProcampParams->bEnabled)
        {
            pProcampParams->fBrightness = MOS_MIN(MOS_MAX(-100.0f, pProcampParams->fBrightness), 100.0f);
            pProcampParams->fContrast   = MOS_MIN(MOS_MAX(   0.0f, pProcampParams->fContrast), 10.0f);
            pProcampParams->fSaturation = MOS_MIN(MOS_MAX(   0.0f, pProcampParams->fSaturation), 10.0f);
        }

        AdjustSurfaceParam(pRenderParams, pSrcSurface, pGtSystemInfo, bHybridDecoderFlag);
    }

        // Check if Slice Shutdown can be enabled  
        // Vebox performance is not impacted by slice shutdown
        if (!(pPrimarySurface == nullptr ||                                      // Valid Layer
            pRenderParams->Component == COMPONENT_VPreP))                        // VpostP usage
        {
            bSliceReconfig = true;
        }

        // Check if Slice Shutdown can be enabled
        if ((uiSsdControl == VPHAL_SSD_ENABLE)                 ||   // Force Enable in User feature keys
            (bSliceReconfig                                    &&   // Default mode
             uiSsdControl == VPHAL_SSD_DEFAULT))
        {
            bSingleSliceMode = true;
        }
        else
        {
            bSingleSliceMode = false;
        }

        pRender[VPHAL_RENDER_ID_COMPOSITE]->SetSingleSliceMode(bSingleSliceMode);

finish:
    return eStatus;
}

//!
//! \brief    Render function for the pass
//! \details  The render function coordinates the advanced renderers and basic
//!           renders in one pass
//! \param    [in,out] pRenderParams
//!           Pointer to VPHAL render parameter
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VphalRenderer::RenderPass(
    PVPHAL_RENDER_PARAMS    pRenderParams)
{
    MOS_STATUS              eStatus;
    uint32_t                uiIndex;                                            // Current source index
    uint32_t                uiSources;                                          // Number of Sources
    PVPHAL_SURFACE          pSrcSurface;                                        // Current source surface
    PVPHAL_VEBOX_EXEC_STATE pVeboxExecState;
    RenderpassData          RenderPassData;

    VPHAL_RENDER_ASSERT(m_pRenderHal);

    eStatus                 = MOS_STATUS_SUCCESS;
    pVeboxExecState         = &VeboxExecState[uiCurrentChannel];
    pSrcSurface             = nullptr;

    RenderPassData.AllocateTempOutputSurfaces();
    RenderPassData.bCompNeeded      = true;

    // Get surface info for all input source
    VPHAL_RENDER_CHK_STATUS(GetSurfaceInfoForSrc(pRenderParams));

    // Process render parameters
    VPHAL_RENDER_CHK_STATUS(ProcessRenderParameter(pRenderParams, &RenderPassData));

    // Loop through the sources
    for (uiSources = 0, uiIndex = 0;
         uiSources < pRenderParams->uSrcCount && uiIndex < VPHAL_MAX_SOURCES;
         uiIndex++)
    {
        pSrcSurface = pRenderParams->pSrc[uiIndex];

        if (pSrcSurface == nullptr)
        {
            continue;
        }

        uiSources++;

        //------------------------------------------
        VPHAL_RNDR_DUMP_SURF(
            this, uiIndex, VPHAL_DBG_DUMP_TYPE_PRE_ALL, pSrcSurface);
        //------------------------------------------

        RenderPassData.pOriginalSrcSurface  = pSrcSurface;
        RenderPassData.pSrcSurface          = pSrcSurface;
        RenderPassData.uiSrcIndex           = uiIndex;

        RenderSingleStream(pRenderParams, &RenderPassData);
    }

    if (!RenderPassData.bCompNeeded &&
        pRenderParams->pTarget[0] &&
        pRenderParams->pTarget[0]->bFastColorFill)
    {
        // with fast color fill enabled, we seperate target surface into two parts:
        // (1) upper rectangle rendered by vebox
        // (2) bottom rectangle with back ground color fill by composition
        pRenderParams->uSrcCount = 0; // set to zero for color fill
        pRenderParams->pTarget[0]->rcDst.top = pRenderParams->pSrc[0]->rcDst.bottom;
        RenderPassData.bCompNeeded = true;
    }

    if (RenderPassData.bCompNeeded)
    {
        VPHAL_RENDER_CHK_STATUS(RenderComposite(pRenderParams, &RenderPassData));
    }

    // Report Render modes
    UpdateReport(pRenderParams, &RenderPassData);

    //------------------------------------------
    VPHAL_RNDR_DUMP_SURF_PTR_ARRAY(
        this, pRenderParams->pSrc, VPHAL_MAX_SOURCES,
        pRenderParams->uSrcCount, VPHAL_DBG_DUMP_TYPE_POST_ALL);

    VPHAL_RNDR_DUMP_SURF_PTR_ARRAY(
        this, pRenderParams->pTarget, VPHAL_MAX_TARGETS,
        pRenderParams->uDstCount, VPHAL_DBG_DUMP_TYPE_POST_ALL);
    //------------------------------------------

    if (RenderPassData.pPrimarySurface)
    {
        VpHal_SaveRestorePrimaryFwdRefs(
            this,
            pRenderParams->pSrc[RenderPassData.uiPrimaryIndex] = RenderPassData.pPrimarySurface,
            false /*restore*/);
    }

finish:
    return eStatus;
}

//!
//! \brief    Render single stream
//! \param    [in] pRenderParams
//!           Pointer to VPHAL render parameter
//! \param    [in,out] pRenderPassData
//!           Pointer to the VPHAL render pass data
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VphalRenderer::RenderSingleStream(
    PVPHAL_RENDER_PARAMS    pRenderParams,
    RenderpassData          *pRenderPassData)
{
    MOS_STATUS              eStatus;

    eStatus         = MOS_STATUS_SUCCESS;

    if (pRenderPassData->pSrcSurface->SurfType == SURF_IN_PRIMARY)
    {

        VpHal_SaveRestorePrimaryFwdRefs(
            this,
            pRenderPassData->pPrimarySurface,
            true /*save*/);

        //-- DNDI/IECP-----------------------------------------------------
        VPHAL_RNDR_DUMP_SURF(
            this, pRenderPassData->uiSrcIndex, VPHAL_DBG_DUMP_TYPE_PRE_DNDI, pRenderPassData->pSrcSurface);

        VPHAL_RENDER_CHK_STATUS(VpHal_RndrRenderVebox(
            this,
            pRenderParams,
            pRenderPassData));

        if (pRenderPassData->bOutputGenerated)
        {
            pRenderPassData->pSrcSurface = pRenderPassData->pOutSurface;
            pRenderPassData->MoveToNextTempOutputSurface();
        }
        else
        {
            // Do not perform any more operations if Comp is not needed
            if (pRenderPassData->bCompNeeded == false)
            {
                VPHAL_RENDER_NORMALMESSAGE("VEBOX/SFC modified Render Target, skipping further processing.");
                goto finish;
            }
        }

        VPHAL_RNDR_DUMP_SURF(
            this, pRenderPassData->uiSrcIndex, VPHAL_DBG_DUMP_TYPE_POST_DNDI, pRenderPassData->pSrcSurface);

        // We'll continue even if Advanced render fails
        if ((eStatus == MOS_STATUS_SUCCESS) && (pRenderPassData->bCompNeeded))
        {
            pRenderParams->pSrc[pRenderPassData->uiSrcIndex] = pRenderPassData->pSrcSurface;
        }
    }

finish:
    return eStatus;
}

//!
//! \brief    Compose input streams
//! \details  Use composite render to compose input streams
//! \param    [in] pRenderParams
//!           Pointer to VPHAL render parameter
//! \param    [in,out] pRenderPassData
//!           Pointer to the VPHAL render pass data
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VphalRenderer::RenderComposite(
    PVPHAL_RENDER_PARAMS    pRenderParams,
    RenderpassData          *pRenderPassData)
{
    MOS_STATUS              eStatus;

    eStatus             = MOS_STATUS_SUCCESS;

    //------------------------------------------
    VPHAL_RNDR_DUMP_SURF_PTR_ARRAY(
        this, pRenderParams->pSrc, VPHAL_MAX_SOURCES, 
        pRenderParams->uSrcCount, VPHAL_DBG_DUMP_TYPE_PRE_COMP);
    //------------------------------------------

    VPHAL_RENDER_CHK_STATUS(pRender[VPHAL_RENDER_ID_COMPOSITE]->Render(pRenderParams, nullptr));

    //------------------------------------------
    VPHAL_RNDR_DUMP_SURF_PTR_ARRAY(
        this, pRenderParams->pSrc, VPHAL_MAX_SOURCES,
        pRenderParams->uSrcCount, VPHAL_DBG_DUMP_TYPE_POST_COMP);

    VPHAL_RNDR_DUMP_SURF_PTR_ARRAY(
        this, pRenderParams->pTarget, VPHAL_MAX_TARGETS,
        pRenderParams->uDstCount, VPHAL_DBG_DUMP_TYPE_POST_COMP);
    //------------------------------------------

finish:
    return eStatus;
}

//!
//! \brief    Update report data
//! \details  Update report data from each feature render
//! \param    [in] pRenderParams
//!           Pointer to VPHAL render parameter
//! \param    [in,out] pRenderPassData
//!           Pointer to the VPHAL render pass data
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
void VphalRenderer::UpdateReport(
    PVPHAL_RENDER_PARAMS    pRenderParams,
    RenderpassData          *pRenderPassData)
{
    VPHAL_GET_SURFACE_INFO  Info;

    pRender[VPHAL_RENDER_ID_COMPOSITE]->CopyReporting(m_reporting);

    if (pRenderPassData->pPrimarySurface && pRenderPassData->pPrimarySurface->bCompressible)
    {
        m_reporting->PrimaryCompressible = true;
        m_reporting->PrimaryCompressMode = (uint8_t)(pRenderPassData->pPrimarySurface->CompressionMode);
    }

    if (pRenderParams->pTarget[0]->bCompressible)
    {
        MOS_ZeroMemory(&Info, sizeof(VPHAL_GET_SURFACE_INFO));

        VpHal_GetSurfaceInfo(m_pOsInterface, &Info, pRenderParams->pTarget[0]);
        m_reporting->RTCompressible = true;
        m_reporting->RTCompressMode = (uint8_t)(pRenderParams->pTarget[0]->CompressionMode);
    }
}

//!
//! \brief    Check if Vphal renderer support some formats
//! \param    [in] pcRenderParams
//!           Const pointer to VPHAL render parameter
//! \return   bool
//!           Return true if successful, false failed
//!
bool VphalRenderer::IsFormatSupported(
    PCVPHAL_RENDER_PARAMS   pcRenderParams)
{
    bool bFormatSupported = true;

    VPHAL_RENDER_ASSERT(pcRenderParams);

    // Protection mechanism
    // P010 output support from KBL+
    if (m_pSkuTable)
    {
        if (pcRenderParams->pTarget[0])
        {
            switch (pcRenderParams->pTarget[0]->Format)
            {
            case Format_P010:
                bFormatSupported = MEDIA_IS_SKU(m_pSkuTable, FtrVpP010Output) ? true : false;
                break;
            case Format_Y210:
                bFormatSupported = MEDIA_IS_SKU(m_pSkuTable, FtrVp10BitSupport) ? true : false;
                break;
            case Format_Y410:
                bFormatSupported = MEDIA_IS_SKU(m_pSkuTable, FtrVp10BitSupport) ? true : false;
                break;
            case Format_Y216:
                bFormatSupported = MEDIA_IS_SKU(m_pSkuTable, FtrVp16BitSupport) ? true : false;
                break;
            case Format_Y416:
                bFormatSupported = MEDIA_IS_SKU(m_pSkuTable, FtrVp16BitSupport) ? true : false;
                break;
            default:
                break;
            }
        }
    }

    return bFormatSupported;
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
MOS_STATUS VphalRenderer::Render(
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

    // Protection mechanism, Only KBL+ support P010 output. 
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

    // Copy the Render Params structure (so we can update it)
    RenderParams = *pcRenderParams;

    VPHAL_DBG_PARAMETERS_DUMPPER_DUMP_XML(&RenderParams);

    // Get resource information for render target
    MOS_ZeroMemory(&Info, sizeof(VPHAL_GET_SURFACE_INFO));

    VPHAL_RENDER_CHK_STATUS(VpHal_GetSurfaceInfo(
        m_pOsInterface,
        &Info,
        RenderParams.pTarget[0]));

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

    // align rectangle and source surface
    VPHAL_RENDER_CHK_STATUS(VpHal_RndrRectSurfaceAlignment(RenderParams.pTarget[0]));

    // Predication 
    if (RenderParams.PredicationParams.predicationEnabled)
    {
        pRenderHal->PredicationParams.pPredicationResource      = 
            RenderParams.PredicationParams.pPredicationResource;
        pRenderHal->PredicationParams.ptempPredicationBuffer =
            RenderParams.PredicationParams.ptempPredicationBuffer;
        pRenderHal->PredicationParams.predicationEnabled =
            RenderParams.PredicationParams.predicationEnabled;
        pRenderHal->PredicationParams.predicationNotEqualZero =
            RenderParams.PredicationParams.predicationNotEqualZero;
        pRenderHal->PredicationParams.predicationResOffset =
            RenderParams.PredicationParams.predicationResOffset;
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

    FreeIntermediateSurfaces();

finish:
    uiFrameCounter++;
    return eStatus;
}

//!
//! \brief    Release intermediate surfaces
//! \details  Release intermediate surfaces created for main render function
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VphalRenderer::FreeIntermediateSurfaces()
{
    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Initialize the VPHAL renderer
//! \details  Initialize all the renderers supported including VEBOX, Composite.
//! \param    [in] pSettings
//!           Const pointer to VPHAL settings
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VphalRenderer::Initialize(
    const VphalSettings                 *pSettings)
{
    void*                               pKernelBin;
    void*                               pFcPatchBin;
    MOS_STATUS                          eStatus;
    PMOS_INTERFACE                      pOsInterface;
    PRENDERHAL_INTERFACE                pRenderHal;
    int32_t                             iResult;
    MHW_KERNEL_PARAM                    MhwKernelParam;
    Kdll_KernelCache                    *pKernelCache;
    Kdll_CacheEntry                     *pCacheEntryTable;

    //---------------------------------------
    VPHAL_RENDER_CHK_NULL(pSettings);
    VPHAL_RENDER_CHK_NULL(m_pOsInterface);
    VPHAL_RENDER_CHK_NULL(m_pRenderHal);
    //---------------------------------------

    eStatus         = MOS_STATUS_UNKNOWN;
    pOsInterface    = m_pOsInterface;
    pRenderHal      = m_pRenderHal;
    pFcPatchBin     = nullptr;

    // Current KDLL expects a writable memory for kernel binary. For that reason,
    // we need to copy the memory to a new location so that KDLL can overwrite.
    // !!! WARNING !!! 
    // We MUST NOT create a writable global memory since it can cause issues 
    // in multi-device cases (multiple threads operating on the memory)
    // NOTE: KDLL will release the allocated memory.
    pKernelBin = MOS_AllocMemory(dwKernelBinSize);
    VPHAL_RENDER_CHK_NULL(pKernelBin);
    MOS_SecureMemcpy(pKernelBin,
                     dwKernelBinSize,
                     pcKernelBin,
                     dwKernelBinSize);

    if ((pcFcPatchBin != nullptr) && (dwFcPatchBinSize != 0))
    {
        pFcPatchBin = MOS_AllocMemory(dwFcPatchBinSize);
        VPHAL_RENDER_CHK_NULL(pFcPatchBin);
        MOS_SecureMemcpy(pFcPatchBin,
                         dwFcPatchBinSize,
                         pcFcPatchBin,
                         dwFcPatchBinSize);
    }

    // Allocate KDLL state (Kernel Dynamic Linking)
    pKernelDllState =  KernelDll_AllocateStates(
                                            pKernelBin,
                                            dwKernelBinSize,
                                            pFcPatchBin,
                                            dwFcPatchBinSize,
                                            pKernelDllRules,
                                            m_modifyKdllFunctionPointers);
    if (!pKernelDllState)
    {
        VPHAL_RENDER_ASSERTMESSAGE("Failed to allocate KDLL state.");
        goto finish;
    }

    // Set up SIP debug kernel if enabled
    if (m_pRenderHal->bIsaAsmDebugEnable)
    {
        pKernelCache        = &pKernelDllState->ComponentKernelCache;
        pCacheEntryTable    = pKernelCache->pCacheEntries;
        VPHAL_RENDER_CHK_NULL(pCacheEntryTable);

        MOS_ZeroMemory(&MhwKernelParam, sizeof(MhwKernelParam));
        MhwKernelParam.pBinary     = pCacheEntryTable[IDR_VP_SIP_Debug].pBinary;
        MhwKernelParam.iSize       = pCacheEntryTable[IDR_VP_SIP_Debug].iSize;
        iResult = m_pRenderHal->pfnLoadDebugKernel(
            m_pRenderHal,
            &MhwKernelParam);

        if (iResult != 0)
        {
            m_pRenderHal->bIsaAsmDebugEnable = false;
        }
    }

    VeboxExecState[0].Mode               = VEBOX_EXEC_MODE_0;
    VeboxExecState[0].bDIOutputPair01    = true;
    VeboxExecState[0].bSpeculativeCopy   = false;
    VeboxExecState[0].bEnable            = (pSettings->veboxParallelExecution == VEBOX_EXECUTION_OVERRIDE_ENABLE);
    VeboxExecState[1]                    = VeboxExecState[0];

    // Initialize VEBOX renderer
    VPHAL_RENDER_CHK_STATUS(pRender[VPHAL_RENDER_ID_VEBOX]->Initialize(
           pSettings,
           pKernelDllState));

    VPHAL_RENDER_CHK_STATUS(pRender[VPHAL_RENDER_ID_VEBOX2]->Initialize(
           pSettings,
           pKernelDllState));

    // Initialize Compositing renderer
    VPHAL_RENDER_CHK_STATUS(pRender[VPHAL_RENDER_ID_COMPOSITE]->Initialize(
        pSettings,
        pKernelDllState));

    AllocateDebugDumper();

    if (MEDIA_IS_SKU(m_pSkuTable, FtrVpDisableFor4K))
    {
        bSkuDisableVpFor4K = true;
    }
    else
    {
        bSkuDisableVpFor4K = false;
    }

    eStatus = MOS_STATUS_SUCCESS;

finish:
    return eStatus;
}

//!
//! \brief    VPHAL renderer destructor
//! \details  Destory the resources allocated for the renderers
//!           including VEBOX and Composite.
//!
VphalRenderer::~VphalRenderer()
{
    VPHAL_RENDER_CHK_NULL_NO_STATUS(m_pOsInterface);

    MOS_Delete(m_reporting);

    for (int32_t i = 0; i < VPHAL_RENDER_ID_COUNT; i++)
    {
        if (pRender[i])
        {
            pRender[i]->Destroy();
            MOS_Delete(pRender[i]);
            pRender[i] = nullptr;
        }
    }
  
    // Destroy Kernel DLL objects (cache, hash table, states)
    if (pKernelDllState)
    {
        KernelDll_ReleaseStates(pKernelDllState);
    }

    // Destroy surface dumper
    VPHAL_DBG_SURF_DUMP_DESTORY(m_surfaceDumper);

    // Destroy state dumper
    VPHAL_DBG_STATE_DUMPPER_DESTORY(m_pRenderHal->pStateDumper);

    // Destroy vphal parameter dump
    VPHAL_DBG_PARAMETERS_DUMPPER_DESTORY(m_parameterDumper);

finish:
    return;
}

//!
//! \brief    Get the aligned the surface height and width unit
//! \details  Accoring to the format of the surface, get the aligned unit for the surface
//!           width and height
//! \param    [in,out] pwWidthAlignUnit
//!           Pointer to the surface width alignment unit
//! \param    [in,out] pwHeightAlignUnit
//!           Pointer to the surface height alignment unit
//! \param    [in] format
//!           The format of the surface
//! \return   void
//!
void VpHal_RndrGetAlignUnit(
    uint16_t*       pwWidthAlignUnit,
    uint16_t*       pwHeightAlignUnit,
    MOS_FORMAT      format)
{
    switch (format)
    {
        case Format_YV12:
        case Format_I420:
        case Format_IYUV:
        case Format_IMC1:
        case Format_IMC2:
        case Format_IMC3:
        case Format_IMC4:
        case Format_NV12:
        case Format_P010:
        case Format_P016:
            *pwWidthAlignUnit = 2;
            *pwHeightAlignUnit = 2;
            break;

        case Format_YVU9:
            *pwWidthAlignUnit = 4;
            *pwHeightAlignUnit = 4;
            break;

        case Format_YUY2:
        case Format_UYVY:
        case Format_YUYV:
        case Format_YVYU:
        case Format_VYUY:
        case Format_P208:
        case Format_Y210:
        case Format_Y216:
            *pwWidthAlignUnit = 2;
            *pwHeightAlignUnit = 1;
            break;

        case Format_NV11:
            *pwWidthAlignUnit = 4;
            *pwHeightAlignUnit = 1;
            break;

        default:
            *pwWidthAlignUnit = 1;
            *pwHeightAlignUnit = 1;
            break;
    }
}

//!
//! \brief    Set packed YUV component offsets
//! \details  Accoring to the format of the surface, set packed YUV component offsets
//! \param    [in] format
//!           The format of the surface
//! \param    [in,out] pOffsetY
//!           The offset of Y
//! \param    [in,out] pOffsetU
//!           The offset of U
//! \param    [in,out] pOffsetV
//!           The offset of V
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_RndrSetYUVComponents(
    MOS_FORMAT      format,
    uint8_t*        pOffsetY,
    uint8_t*        pOffsetU,
    uint8_t*        pOffsetV)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    switch (format)
    {
        case Format_PA:
        case Format_YUY2:
        case Format_YUYV:
            *pOffsetY = 0;
            *pOffsetU = 1;
            *pOffsetV = 3;
            break;

        case Format_UYVY:
            *pOffsetY = 1;
            *pOffsetU = 0;
            *pOffsetV = 2;
            break;

        case Format_YVYU:
            *pOffsetY = 0;
            *pOffsetU = 3;
            *pOffsetV = 1;
            break;

        case Format_VYUY:
            *pOffsetY = 1;
            *pOffsetU = 2;
            *pOffsetV = 0;
            break;

        case Format_Y210:
            *pOffsetY = 0;
            *pOffsetU = 2;
            *pOffsetV = 6;
            break;

        default:
            VPHAL_RENDER_ASSERTMESSAGE("Unknown Packed YUV Format.");
            eStatus = MOS_STATUS_UNKNOWN;
    }

    return eStatus;
}

//!
//! \brief    VphalRenderer constructor
//! \details  Based on the HW and OS info, initialize the renderer interfaces
//! \param    [in] pRenderHal
//!           Pointer to RenderHal Interface Structure
//! \param    [in,out] pStatus
//!           Pointer to the MOS_STATUS flag.
//!                    Will assign this flag to MOS_STATUS_SUCCESS if successful, otherwise failed
//!
VphalRenderer::VphalRenderer(
    PRENDERHAL_INTERFACE                pRenderHal,
    MOS_STATUS                          *pStatus) :
    m_pRenderHal(pRenderHal),
    m_pOsInterface(pRenderHal->pOsInterface),
    m_pSkuTable(nullptr),
    m_modifyKdllFunctionPointers(nullptr),
    uiSsdControl(0),
    bDpRotationUsed(false),
    bSkuDisableVpFor4K(false),
    bSkuDisableLaceFor4K(false),
    bSkuDisableDNFor4K(false),
    PerfData(),
    m_reporting(nullptr),
    VeboxExecState(),
    pRender(),
    pPrimaryFwdRef(),
    bVeboxUsedForCapPipe(false),
    uiCurrentChannel(0),
    pKernelDllRules(nullptr),
    pKernelDllState(nullptr),
    pcKernelBin(nullptr),
    dwKernelBinSize(0),
    pcFcPatchBin(nullptr),
    dwFcPatchBinSize(0),
    uiFrameCounter(0),
#if (_DEBUG || _RELEASE_INTERNAL)
    m_surfaceDumper(nullptr),
    m_parameterDumper(nullptr),
#endif
    StatusTable(),
    maxSrcRect()
{
    MOS_STATUS                          eStatus;
    MOS_USER_FEATURE_VALUE_DATA         UserFeatureData;

    MOS_ZeroMemory(&pRender, sizeof(pRender));


    // Read Slice Shutdown (SSD Control) User Feature Key once during initialization
    MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
    eStatus = MOS_UserFeature_ReadValue_ID(
            nullptr,
            __VPHAL_RNDR_SSD_CONTROL_ID,
            &UserFeatureData);
    if (eStatus == MOS_STATUS_SUCCESS)
    {
        uiSsdControl = UserFeatureData.u32Data;
    }

    // Do not fail if User feature keys is not present
    eStatus = MOS_STATUS_SUCCESS;

    // Get SKU table
    m_pSkuTable = m_pOsInterface->pfnGetSkuTable(m_pOsInterface);

    *pStatus = eStatus;
}

//!
//! \brief    Search for the best match BB according to the render BB arguments
//! \details  Based on the params of the BB, search the BB table and try to get
//!           the best match
//! \param    [in] pBatchBufferTable
//!           Point to the BB table to be searched
//! \param    [in] pInputBbParams
//!           Point to the BB params required for the BB needed
//! \param    [in] iBbSize
//!           The BB size required for the BB needed
//! \param    [out] ppBatchBuffer
//!           Point to the addr of the best matched BB. Point to nullptr if there's no.
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_RenderGetBestMatchBB(
    PVPHAL_BATCH_BUFFER_TABLE       pBatchBufferTable,
    PVPHAL_BATCH_BUFFER_PARAMS      pInputBbParams,
    int32_t                         iBbSize,
    PMHW_BATCH_BUFFER               *ppBatchBuffer)
{
    PMHW_BATCH_BUFFER               pBbEntry;
    PMHW_BATCH_BUFFER               pBestMatch;
    PVPHAL_BATCH_BUFFER_PARAMS      pSearchBbParams;
    void*                           pInputBbArgs;
    void*                           pSearchBbArgs;
    VPHAL_BB_TYPE                   BbType;
    int32_t                         i;
    int32_t                         iBbCount;
    int32_t                         iBbArgSize;
    MOS_STATUS                      eStatus;

    pBestMatch   = nullptr;
    pBbEntry     = pBatchBufferTable->pBatchBufferHeader;
    iBbCount     = *pBatchBufferTable->piBatchBufferCount;
    BbType       = pInputBbParams->iType;
    pInputBbArgs = &pInputBbParams->BbArgs;
    iBbArgSize   = pInputBbParams->iSize;
    eStatus      = MOS_STATUS_UNKNOWN;

    for (i = iBbCount; i > 0; i--, pBbEntry++)
    {
        pSearchBbParams = (PVPHAL_BATCH_BUFFER_PARAMS)pBbEntry->pPrivateData;

        // Must have adequate size and the batch buffer type must be the same
        if (!pSearchBbParams                  ||
            pBbEntry->iSize         < iBbSize ||
            pSearchBbParams->iType != BbType)
        {
            continue;
        }

        // Point to the start address of the union
        pSearchBbArgs = &(pSearchBbParams->BbArgs);

        // BB args must be the same to find the best match(DnDi, Frc and Istab)
        if (memcmp(pInputBbArgs, pSearchBbArgs, iBbArgSize))
        {
            continue;
        }

        // Match -> reuse the BB regardless of the running state
        pBestMatch = pBbEntry;
        ((PVPHAL_BATCH_BUFFER_PARAMS)pBestMatch->pPrivateData)->bMatch = true;

        break;
    }

    *ppBatchBuffer = pBestMatch;
    eStatus        = MOS_STATUS_SUCCESS;
    return eStatus;
}

//!
//! \brief    Search from existing BBs for a match. If none, allocate new BB
//! \details  Based on the params of the BB, search the BB table and try to get
//!           the best match. If none, try to get an old unused BB to reuse. If
//!           still none, allocate one new BB
//! \param    [in] pBatchBufferTable
//!           Pointer to the BB table to be searched
//! \param    [in] pInputBbParams
//!           Pointer to the BB params required for the BB needed
//! \param    [in] iBbSize
//!           The BB size required for the BB needed
//! \param    [in] pRenderHal
//!           Pointer to RenderHal Interface Structure
//! \param    [out] ppBatchBuffer
//!           Pointer to the addr of the available BB. Point to nullptr if there's no
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_RenderAllocateBB(
    PVPHAL_BATCH_BUFFER_TABLE     pBatchBufferTable,
    PVPHAL_BATCH_BUFFER_PARAMS    pInputBbParams,
    int32_t                       iBbSize,
    PRENDERHAL_INTERFACE          pRenderHal,
    PMHW_BATCH_BUFFER             *ppBatchBuffer)
{
    PMHW_BATCH_BUFFER           pOldest;                // Oldest BB entry
    PMHW_BATCH_BUFFER           pBatchBuffer;           // Available BB allocation
    PMHW_BATCH_BUFFER           pBbEntry;               // 2nd level BBs array entry
    PVPHAL_BATCH_BUFFER_PARAMS  pSearchBbParams;        // Search BB parameters
    PVPHAL_BATCH_BUFFER_PARAMS  pBbParams;
    int32_t                     i;
    int32_t                     iBbCount;
    VPHAL_BB_TYPE               BbType;
    int32_t                     iBbArgSize;
    MOS_STATUS                  eStatus;

    pOldest      = nullptr;
    pBatchBuffer = nullptr;
    pBbEntry     = pBatchBufferTable->pBatchBufferHeader;
    iBbCount     = *pBatchBufferTable->piBatchBufferCount;
    BbType       = pInputBbParams->iType;
    iBbArgSize   = pInputBbParams->iSize;
    eStatus      = MOS_STATUS_UNKNOWN;

    switch (BbType)
    {
        case VPHAL_BB_TYPE_COMPOSITING:
            VPHAL_RENDER_CHK_STATUS(CompositeState::GetBestMatchBB(
                pBatchBufferTable,
                pInputBbParams,
                iBbSize,
                &pBatchBuffer));
            break;

        case VPHAL_BB_TYPE_ADVANCED:
            VPHAL_RENDER_CHK_STATUS(VpHal_RenderGetBestMatchBB(
                        pBatchBufferTable, 
                        pInputBbParams, 
                        iBbSize,
                        &pBatchBuffer));
            break;

        case VPHAL_BB_TYPE_GENERIC:
            break;

        default:
            VPHAL_RENDER_ASSERTMESSAGE("Unsupported batch buffer type.");
            eStatus = MOS_STATUS_UNKNOWN;
            goto finish;
    }

    if (pBatchBuffer)
    {
        // Best available batch buffer found
        eStatus = MOS_STATUS_SUCCESS;
        goto finish;
    }

    // Search for an old unused BB to reuse
    for (i = iBbCount; i > 0; i--, pBbEntry++)
    {
        pSearchBbParams = (PVPHAL_BATCH_BUFFER_PARAMS)pBbEntry->pPrivateData;

        if (pSearchBbParams        == nullptr ||
            pSearchBbParams->iType != BbType)
        {
            continue;
        }

        // Save oldest entry, regardless of size/in-use
        if (pOldest == nullptr ||
            (int32_t)(pBbEntry->dwSyncTag - pOldest->dwSyncTag) < 0)
        {
            pOldest = pBbEntry;
        }

        // Skip busy or small batch buffers
        if (pBbEntry->bBusy || pBbEntry->iSize < iBbSize)
        {
            continue;
        }

        // Use the oldest BB with suitable size and not in use
        if (pBatchBuffer == nullptr ||
            (int32_t)(pBbEntry->dwSyncTag - pBatchBuffer->dwSyncTag) < 0)
        {
            pBatchBuffer = pBbEntry;
            pBatchBuffer->iCurrent = 0;
            pBatchBuffer->iRemaining = pBatchBuffer->iSize;
        }
    }

    // No available BB to use - allocate new
    if (!pBatchBuffer)
    {
        iBbSize = MOS_ALIGN_CEIL(iBbSize, VPHAL_BB_ALIGN_SIZE);

        if (pOldest == nullptr ||
            (pOldest->bBusy && 
             iBbCount < pBatchBufferTable->iBbCountMax))
        {
            pOldest = nullptr;
            i = iBbCount++;

            pBatchBuffer                = pBatchBufferTable->pBatchBufferHeader + i;
            pBatchBuffer->pPrivateData  = pBatchBufferTable->pBbParamsHeader + i;

            *pBatchBufferTable->piBatchBufferCount = iBbCount;
        }

        // Release old buffer - may be even in use (delayed release)
        if (pOldest)
        {
            if (pRenderHal->pfnFreeBB(pRenderHal, pOldest) != MOS_STATUS_SUCCESS)
            {
                VPHAL_RENDER_ASSERTMESSAGE("Failed to release batch buffer.");
                eStatus = MOS_STATUS_UNKNOWN;
                goto finish;
            }

            pBatchBuffer = pOldest;
        }

        // Allocate new buffer
        if (pRenderHal->pfnAllocateBB(pRenderHal, pBatchBuffer, iBbSize) != MOS_STATUS_SUCCESS)
        {
            VPHAL_RENDER_ASSERTMESSAGE("Failed to allocate batch buffer.");
            pBatchBuffer = nullptr;
            eStatus      = MOS_STATUS_UNKNOWN;
            goto finish;
        }
    }

    // Set batch buffer args
    pBbParams           = (PVPHAL_BATCH_BUFFER_PARAMS)pBatchBuffer->pPrivateData;
    pBbParams->bMatch   = false;
    pBbParams->iType    = BbType;
    pBbParams->iSize    = iBbArgSize;
    pBbParams->BbArgs   = pInputBbParams->BbArgs;

    eStatus = MOS_STATUS_SUCCESS;

finish:
    *ppBatchBuffer = pBatchBuffer;
    return eStatus;
}

//!
//! \brief    Update max src rect in VphalRenderer and primary surface based
//!           on src rectangle info from primary video
//! \details  Add max src rect for consistent statistics surface layout. Update
//!           the max src rect of the surface and its reference surfaces
//! \param    [in,out] pRenderer
//!           VPHAL renderer pointer
//! \param    [in,out] pSurface
//!           Pointer to the surface
//! \return   void
//!
void VpHal_RenderInitMaxRect(
    VphalRenderer           *pRenderer,
    PVPHAL_SURFACE          pSurface)
{
    PVPHAL_SURFACE      pRef;
    uint32_t            i;

    pSurface->bMaxRectChanged =
        (pSurface->rcSrc.right > pRenderer->maxSrcRect.right ||
        pSurface->rcSrc.bottom > pRenderer->maxSrcRect.bottom) ?
        true : false;

    // calcualte max srcRect in pRenderParams
    pRenderer->maxSrcRect.right = MOS_MAX(
        pRenderer->maxSrcRect.right, pSurface->rcSrc.right);
    pRenderer->maxSrcRect.bottom = MOS_MAX(
        pRenderer->maxSrcRect.bottom, pSurface->rcSrc.bottom);

    // copy max src rect to primary video
    pSurface->rcMaxSrc = pRenderer->maxSrcRect;

    // copy max src rect to forward reference video
    pRef = pSurface->pFwdRef;
    for (i = 0; i < pSurface->uFwdRefCount; i++)
    {
        // check surface validity
        VPHAL_RENDER_CHK_NULL_NO_STATUS(pRef);

        pRef->rcMaxSrc = pRenderer->maxSrcRect;

        // get next forward reference
        pRef = pRef->pFwdRef;
    }

    // copy max src rect to backward reference video
    pRef = pSurface->pBwdRef;
    for (i = 0; i < pSurface->uBwdRefCount; i++)
    {
        // check surface validity
        VPHAL_RENDER_CHK_NULL_NO_STATUS(pRef);

        pRef->rcMaxSrc = pRenderer->maxSrcRect;

        // get next backward reference
        pRef = pRef->pBwdRef;
    }

finish:
    return;
}

void VphalRenderer::AllocateDebugDumper()
{
    PRENDERHAL_INTERFACE pRenderHal = m_pRenderHal;

    // Allocate feature report
    m_reporting = MOS_New(VphalFeatureReport);

    // Initialize Surface Dumper 
    VPHAL_DBG_SURF_DUMP_CREATE();

    // Initialize State Dumper 
    VPHAL_DBG_STATE_DUMPPER_CREATE();

    VPHAL_DBG_PARAMETERS_DUMPPER_CREATE();
}
