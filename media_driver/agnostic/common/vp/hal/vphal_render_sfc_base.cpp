/*
* Copyright (c) 2012-2019, Intel Corporation
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
//! \file     vphal_render_sfc_base.cpp
//! \brief    VPHAL SFC rendering component
//! \details  The SFC renderer supports Scaling, IEF, CSC/ColorFill and Rotation.
//!           It's responsible for setting up HW states and generating the SFC
//!           commands.
//!
#include "vphal_render_vebox_base.h"
#include "vphal_render_ief.h"
#include "vphal_render_sfc_base.h"

#if __VPHAL_SFC_SUPPORTED

//!
//! \brief Constants used to derive Line Buffer sizes
//!
#define SFC_CACHELINE_SIZE_IN_BYTES                     (512 / 8)
#define SFC_AVS_LINEBUFFER_SIZE_PER_VERTICAL_PIXEL      (5 * SFC_CACHELINE_SIZE_IN_BYTES / 8)
#define SFC_IEF_LINEBUFFER_SIZE_PER_VERTICAL_PIXEL      (1 * SFC_CACHELINE_SIZE_IN_BYTES / 4)

//!
//! \brief    Initialize SFC Output Surface Command parameters
//! \details  Initialize MHW SFC Output Surface Command parameters from SFC Pipe output Surface
//! \param    [in] pSfcPipeOutSurface
//!           pointer to SFC Pipe output Surface
//! \param    [out] pMhwOutSurfParams
//!           pointer to SFC Output Surface Command parameters
//! \return   MOS_STATUS
//!
MOS_STATUS VpHal_InitMhwOutSurfParams(
    PVPHAL_SURFACE                           pSfcPipeOutSurface,
    PMHW_SFC_OUT_SURFACE_PARAMS              pMhwOutSurfParams)
{
    MOS_STATUS                   eStatus = MOS_STATUS_SUCCESS;

    VPHAL_RENDER_CHK_NULL(pSfcPipeOutSurface);
    VPHAL_RENDER_CHK_NULL(pMhwOutSurfParams);

    MOS_ZeroMemory(pMhwOutSurfParams, sizeof(*pMhwOutSurfParams));

    pMhwOutSurfParams->ChromaSiting                = pSfcPipeOutSurface->ChromaSiting;
    pMhwOutSurfParams->dwWidth                     = pSfcPipeOutSurface->dwWidth;
    pMhwOutSurfParams->dwHeight                    = pSfcPipeOutSurface->dwHeight;
    pMhwOutSurfParams->dwPitch                     = pSfcPipeOutSurface->dwPitch;
    pMhwOutSurfParams->TileType                    = pSfcPipeOutSurface->TileType;
    pMhwOutSurfParams->TileModeGMM                 = pSfcPipeOutSurface->TileModeGMM;
    pMhwOutSurfParams->bGMMTileEnabled             = pSfcPipeOutSurface->bGMMTileEnabled;
    pMhwOutSurfParams->pOsResource                 = &(pSfcPipeOutSurface->OsResource);
    pMhwOutSurfParams->Format                      = pSfcPipeOutSurface->Format;
    pMhwOutSurfParams->bCompressible               = pSfcPipeOutSurface->bCompressible;
    pMhwOutSurfParams->dwCompressionFormat         = pSfcPipeOutSurface->CompressionFormat;
    pMhwOutSurfParams->dwSurfaceXOffset            = pSfcPipeOutSurface->YPlaneOffset.iXOffset;
    pMhwOutSurfParams->dwSurfaceYOffset            = pSfcPipeOutSurface->YPlaneOffset.iYOffset;

    if (pSfcPipeOutSurface->dwPitch > 0)
    {
        pMhwOutSurfParams->dwUYoffset = ((pSfcPipeOutSurface->UPlaneOffset.iSurfaceOffset - pSfcPipeOutSurface->YPlaneOffset.iSurfaceOffset) / pSfcPipeOutSurface->dwPitch) + pSfcPipeOutSurface->UPlaneOffset.iYOffset;
    }

finish:
    return eStatus;
}

//!
//! \brief    Get SFC Rotation mode parameter
//! \details  Get MHW SFC Rotation mode parameter
//! \param    [in] Rotation
//!           VPHAL roration mode parameter
//! \return   MHW_ROTATION
//!
MHW_ROTATION VpHal_GetMhwRotationParam(VPHAL_ROTATION Rotation)
{
    switch (Rotation)
    {
        case VPHAL_ROTATION_90:
            return MHW_ROTATION_90;                         // 90 Degree Rotation

        case VPHAL_ROTATION_180:
            return MHW_ROTATION_180;                        // 180 Degree Rotation

        case VPHAL_ROTATION_270:
            return MHW_ROTATION_270;                        // 270 Degree Rotation

        case VPHAL_MIRROR_HORIZONTAL:
            return MHW_MIRROR_HORIZONTAL;                   // Horizontal Mirror

        case VPHAL_MIRROR_VERTICAL:
            return MHW_MIRROR_VERTICAL;                     // Vertical Mirror

        case VPHAL_ROTATE_90_MIRROR_VERTICAL:
            return MHW_ROTATION_270;                        // 270 Degree rotation and Horizontal Mirror

        case VPHAL_ROTATE_90_MIRROR_HORIZONTAL:
            return MHW_ROTATION_90;                         // 90 Degree rotation and Horizontal Mirror

        default:
        case VPHAL_ROTATION_IDENTITY:
            return MHW_ROTATION_IDENTITY;
    }
}

bool VphalSfcState::IsFormatMMCSupported(
    MOS_FORMAT                  Format)
{
    // Check if Sample Format is supported
    if ((Format != Format_NV12) &&
        (Format != Format_UYVY) &&
        (Format != Format_YUYV))
    {
        VPHAL_RENDER_NORMALMESSAGE("Unsupported Format '0x%08x' for SFC MMC.", Format);
        return false;
    }

    return true;
}

VphalSfcState::VphalSfcState(
    PMOS_INTERFACE       osInterface,
    PRENDERHAL_INTERFACE renderHal,
    PMHW_SFC_INTERFACE   sfcInterface)
{
    VPHAL_RENDER_ASSERT(osInterface);
    VPHAL_RENDER_ASSERT(renderHal);
    VPHAL_RENDER_ASSERT(sfcInterface);

    m_renderHal       = renderHal;
    m_sfcInterface    = sfcInterface;
    m_osInterface     = osInterface;

    // Allocate AVS state
    VpHal_RndrCommonInitAVSParams(
        &m_AvsParameters,
        POLYPHASE_Y_COEFFICIENT_TABLE_SIZE_G9,
        POLYPHASE_UV_COEFFICIENT_TABLE_SIZE_G9);
}

VphalSfcState::~VphalSfcState()
{
    VpHal_RndrCommonDestroyAVSParams(&m_AvsParameters);
    MOS_FreeMemAndSetNull(m_renderData.SfcStateParams);
}

bool VphalSfcState::IsOutputCapable(
    bool            isColorFill,
    PVPHAL_SURFACE  src,
    PVPHAL_SURFACE  renderTarget)
{
    bool isOutputCapable = false;

    VPHAL_RENDER_NORMALMESSAGE(
        "isColorFill %d, \
         src->rcDst.top %d, \
         src->rcDst.left %d, \
         renderTarget->TileType %d, \
         renderTarget->Format %d",
        isColorFill,
        src->rcDst.top,
        src->rcDst.left,
        renderTarget->TileType,
        renderTarget->Format);

    // H/W does not support ColorFill, the (OffsetX, OffsetY)
    // of scaled region not being (0, 0) or the tile type not being
    // Tile_Y on NV12/P010/P016 output surface. Disable SFC even if other
    // features are supported.
    if ((isColorFill         ||
        src->rcDst.top  != 0 ||
        src->rcDst.left != 0 ||
        renderTarget->TileType != MOS_TILE_Y) &&
        (renderTarget->Format == Format_NV12 ||
         renderTarget->Format == Format_P010 ||
         renderTarget->Format == Format_P016))
    {
        isOutputCapable = false;
    }
    else
    {
        isOutputCapable = true;
    }

    return isOutputCapable;
}

void VphalSfcState::AdjustBoundary(
    PVPHAL_SURFACE              pSurface,
    uint32_t*                   pdwSurfaceWidth,
    uint32_t*                   pdwSurfaceHeight)
{
    uint32_t   dwVeboxHeight;
    uint32_t   dwVeboxWidth;
    uint32_t   dwVeboxBottom;
    uint32_t   dwVeboxRight;

    VPHAL_RENDER_CHK_NULL_NO_STATUS(m_sfcInterface);
    VPHAL_RENDER_CHK_NULL_NO_STATUS(pSurface);
    VPHAL_RENDER_CHK_NULL_NO_STATUS(pdwSurfaceWidth);
    VPHAL_RENDER_CHK_NULL_NO_STATUS(pdwSurfaceHeight);

    // For the VEBOX output to SFC, the width is multiple of 16 and height
    // is multiple of 4
    dwVeboxHeight = pSurface->dwHeight;
    dwVeboxWidth  = pSurface->dwWidth;
    dwVeboxBottom = (uint32_t)pSurface->rcMaxSrc.bottom;
    dwVeboxRight  = (uint32_t)pSurface->rcMaxSrc.right;

    if(pSurface->bDirectionalScalar)
    {
        dwVeboxHeight *= 2;
        dwVeboxWidth  *= 2;
        dwVeboxBottom *= 2;
        dwVeboxRight  *= 2;
    }

    *pdwSurfaceHeight = MOS_ALIGN_CEIL(
        MOS_MIN(dwVeboxHeight, MOS_MAX(dwVeboxBottom, MHW_VEBOX_MIN_HEIGHT)),
        m_sfcInterface->m_veHeightAlignment);
    *pdwSurfaceWidth  = MOS_ALIGN_CEIL(
        MOS_MIN(dwVeboxWidth, MOS_MAX(dwVeboxRight, MHW_VEBOX_MIN_WIDTH)),
        m_sfcInterface->m_veWidthAlignment);

finish:
    return;
}

bool VphalSfcState::IsOutputPipeSfcFeasible(
    PCVPHAL_RENDER_PARAMS       pcRenderParams,
    PVPHAL_SURFACE              pSrcSurface,
    PVPHAL_SURFACE              pRenderTarget)
{
    VPHAL_RENDER_NORMALMESSAGE(
        "IsDisabled %d, \
         uDstCount %d, \
         Rotation %d, \
         pTarget[0]->TileType %d, \
         IsFormatSupported %d, \
         InputFormat %d, \
         OutputFormat %d, \
         pCompAlpha %p, \
         pDeinterlaceParams %p, \
         bQueryVariance %d",
        IsDisabled(),
        pcRenderParams->uDstCount,
        pSrcSurface->Rotation,
        pcRenderParams->pTarget[0]->TileType,
        IsFormatSupported(pSrcSurface, pcRenderParams->pTarget[0], pcRenderParams->pCompAlpha),
        pSrcSurface->Format,
        pcRenderParams->pTarget[0]->Format,
        pcRenderParams->pCompAlpha,
        pSrcSurface->pDeinterlaceParams,
        pSrcSurface->bQueryVariance);

    //!
    //! \brief SFC can be the output pipe when the following conditions are all met
    //!        1.  User feature keys value "SFC Disable" is false
    //!        2.  Single render target only
    //!        3.  Rotation disabled or ONLY Rotation enabled when the SFC output is Y-tile
    //!        4.  i/o format is supported by SFC, taking into account the alpha fill info
    //!        5.  Comp DI(ARGB/ABGR) is disabled
    //!        6.  Variance Query is disabled
    //!
    if (IsDisabled()                            == false                                        &&
        pcRenderParams->uDstCount               == 1                                            &&
        (pSrcSurface->Rotation                  == VPHAL_ROTATION_IDENTITY                      ||
         (pSrcSurface->Rotation                 <= VPHAL_ROTATION_270                           &&
          pcRenderParams->pTarget[0]->TileType  == MOS_TILE_Y))                                 &&
        IsFormatSupported(pSrcSurface, pcRenderParams->pTarget[0], pcRenderParams->pCompAlpha)  &&
        (pSrcSurface->pDeinterlaceParams        == nullptr                                      ||
         (pSrcSurface->Format != Format_A8R8G8B8 && pSrcSurface->Format != Format_A8B8G8R8))    &&
        pSrcSurface->bQueryVariance             == false)
    {
        // For platforms with VEBOX disabled but procamp enabled, go Render path
        if (MEDIA_IS_SKU(m_renderHal->pSkuTable, FtrDisableVEBoxFeatures) && pSrcSurface->pProcampParams != nullptr)
        {
            return false;
        }

        return true;
    }

    return false;
}

VPHAL_OUTPUT_PIPE_MODE VphalSfcState::GetOutputPipe(
    PVPHAL_SURFACE              pSrc,
    PVPHAL_SURFACE              pRenderTarget,
    PCVPHAL_RENDER_PARAMS       pcRenderParams)
{
    float                       fScaleX;
    float                       fScaleY;
    uint32_t                    dwSurfaceWidth;
    uint32_t                    dwSurfaceHeight;
    VPHAL_OUTPUT_PIPE_MODE      OutputPipe;
    bool                        bColorFill;
    uint16_t                    wWidthAlignUnit;
    uint16_t                    wHeightAlignUnit;
    uint32_t                    dwSourceRegionWidth;
    uint32_t                    dwSourceRegionHeight;
    uint32_t                    dwOutputRegionWidth;
    uint32_t                    dwOutputRegionHeight;
    uint32_t                    dwSfcMaxWidth;
    uint32_t                    dwSfcMaxHeight;
    uint32_t                    dwSfcMinWidth;
    uint32_t                    dwSfcMinHeight;

    OutputPipe = VPHAL_OUTPUT_PIPE_MODE_COMP;

    VPHAL_RENDER_CHK_NULL_NO_STATUS(m_sfcInterface);
    VPHAL_RENDER_CHK_NULL_NO_STATUS(pSrc);
    VPHAL_RENDER_CHK_NULL_NO_STATUS(pRenderTarget);
    VPHAL_RENDER_CHK_NULL_NO_STATUS(pcRenderParams);

    dwSfcMaxWidth       = m_sfcInterface->m_maxWidth;
    dwSfcMaxHeight      = m_sfcInterface->m_maxHeight;
    dwSfcMinWidth       = m_sfcInterface->m_minWidth;
    dwSfcMinHeight      = m_sfcInterface->m_minHeight;
    wWidthAlignUnit     = 1;
    wHeightAlignUnit    = 1;

    // Check if the feature can be supported by SFC output pipe
    if (!IsOutputPipeSfcFeasible(pcRenderParams, pSrc, pRenderTarget))
    {
        VPHAL_RENDER_NORMALMESSAGE("Feature or surface format not supported by SFC Pipe.");
        OutputPipe = VPHAL_OUTPUT_PIPE_MODE_COMP;
        return OutputPipe;
    }

    // Get the SFC input surface size from Vebox
    AdjustBoundary(
        pSrc,
        &dwSurfaceWidth,
        &dwSurfaceHeight);

    // Apply alignment restriction to the source and scaled regions.
    switch(pRenderTarget->Format)
    {
        case Format_NV12:
            wWidthAlignUnit     = 2;
            wHeightAlignUnit    = 2;
            break;
        case Format_YUY2:
        case Format_UYVY:
            wWidthAlignUnit     = 2;
            break;
        default:
            break;
    }

    // Region of the input frame which needs to be processed by SFC
    dwSourceRegionHeight = MOS_ALIGN_FLOOR(
                            MOS_MIN((uint32_t)(pSrc->rcSrc.bottom - pSrc->rcSrc.top), dwSurfaceHeight),
                            wHeightAlignUnit);
    dwSourceRegionWidth  = MOS_ALIGN_FLOOR(
                            MOS_MIN((uint32_t)(pSrc->rcSrc.right  - pSrc->rcSrc.left), dwSurfaceWidth),
                            wWidthAlignUnit);

    // Size of the Output Region over the Render Target
    dwOutputRegionHeight = MOS_ALIGN_CEIL(
                            (uint32_t)(pSrc->rcDst.bottom - pSrc->rcDst.top),
                            wHeightAlignUnit);
    dwOutputRegionWidth  = MOS_ALIGN_CEIL(
                            (uint32_t)(pSrc->rcDst.right - pSrc->rcDst.left),
                            wWidthAlignUnit);

    // SFC i/o width and height should fall into the range of [128, 4K]
    if (OUT_OF_BOUNDS(dwSurfaceWidth, dwSfcMinWidth, dwSfcMaxWidth)         ||
        OUT_OF_BOUNDS(dwSurfaceHeight, dwSfcMinHeight, dwSfcMaxHeight)      ||
        OUT_OF_BOUNDS(dwSourceRegionWidth, dwSfcMinWidth, dwSfcMaxWidth)    ||
        OUT_OF_BOUNDS(dwSourceRegionHeight, dwSfcMinHeight, dwSfcMaxHeight) ||
        OUT_OF_BOUNDS(dwOutputRegionWidth, dwSfcMinWidth, dwSfcMaxWidth)    ||
        OUT_OF_BOUNDS(dwOutputRegionHeight, dwSfcMinHeight, dwSfcMaxHeight) ||
        OUT_OF_BOUNDS(pRenderTarget->dwWidth, dwSfcMinWidth, dwSfcMaxWidth) ||
        OUT_OF_BOUNDS(pRenderTarget->dwHeight, dwSfcMinHeight, dwSfcMaxHeight))
    {
        VPHAL_RENDER_NORMALMESSAGE("Surface dimensions not supported by SFC Pipe.");
        OutputPipe = VPHAL_OUTPUT_PIPE_MODE_COMP;
        return OutputPipe;
    }

    // Size of the Output Region over the Render Target
    dwOutputRegionHeight = MOS_MIN(dwOutputRegionHeight, pRenderTarget->dwHeight);
    dwOutputRegionWidth  = MOS_MIN(dwOutputRegionWidth, pRenderTarget->dwWidth);

    // Calculate the scaling ratio
    // Both source region and scaled region are pre-rotated
    if (pSrc->Rotation == VPHAL_ROTATION_IDENTITY ||
        pSrc->Rotation == VPHAL_ROTATION_180      ||
        pSrc->Rotation == VPHAL_MIRROR_HORIZONTAL ||
        pSrc->Rotation == VPHAL_MIRROR_VERTICAL)
    {
        fScaleX      = (float)dwOutputRegionWidth  / (float)dwSourceRegionWidth;
        fScaleY      = (float)dwOutputRegionHeight / (float)dwSourceRegionHeight;
    }
    else
    {
        // VPHAL_ROTATION_90 || VPHAL_ROTATION_270 || VPHAL_ROTATE_90_MIRROR_VERTICAL || VPHAL_ROTATE_90_MIRROR_HORIZONTAL
        fScaleX      = (float)dwOutputRegionHeight / (float)dwSourceRegionWidth;
        fScaleY      = (float)dwOutputRegionWidth  / (float)dwSourceRegionHeight;
    }

    // SFC scaling range is [0.125, 8] for both X and Y direction.
    if ((fScaleX < 0.125F)  || (fScaleX > 8.0F) ||
        (fScaleY < 0.125F)  || (fScaleY > 8.0F))
    {
        VPHAL_RENDER_NORMALMESSAGE("Scaling factor not supported by SFC Pipe.");
        OutputPipe = VPHAL_OUTPUT_PIPE_MODE_COMP;
        return OutputPipe;
    }

    if (MEDIA_IS_WA(m_renderHal->pWaTable, WaDisableSFCSrcCrop) &&
        dwSurfaceHeight > 1120 &&
        (((pSrc->rcSrc.left > 0) || (dwSurfaceWidth - pSrc->rcSrc.right > 0))      ||
         ((pSrc->rcSrc.bottom > 1120) && (pSrc->rcSrc.bottom < (int32_t)dwSurfaceHeight)) ||
         ((pSrc->rcSrc.top > 1120) && (pSrc->rcSrc.top < (int32_t)dwSurfaceHeight))       ||
         (pSrc->rcSrc.bottom < (int32_t)dwSurfaceHeight)))
    {
        VPHAL_RENDER_NORMALMESSAGE("Fallback to comp path as SW WA for SFC Cropping TDR.");
        OutputPipe = VPHAL_OUTPUT_PIPE_MODE_COMP;
        return OutputPipe;
    }

    // if ScalingPreference == Composition, switch to use composition path
    // This flag can be set by app.
    if (pSrc->ScalingPreference == VPHAL_SCALING_PREFER_COMP)
    {
        VPHAL_RENDER_NORMALMESSAGE("DDI set ScalingPreference to Composition to use render for scaling.");
        OutputPipe = VPHAL_OUTPUT_PIPE_MODE_COMP;
        return OutputPipe;
    }

    bColorFill = (pcRenderParams->pColorFillParams &&
                  (!RECT1_CONTAINS_RECT2(pSrc->rcDst, pRenderTarget->rcDst))) ?
                 true : false;

    if (IsOutputCapable(bColorFill, pSrc, pRenderTarget))
    {
        OutputPipe = VPHAL_OUTPUT_PIPE_MODE_SFC;
    }
    else
    {
        OutputPipe = VPHAL_OUTPUT_PIPE_MODE_COMP;
    }

finish:
    return OutputPipe;
}

void VphalSfcState::DetermineCscParams(
    PVPHAL_SURFACE                  src,
    PVPHAL_SURFACE                  renderTarget)
{
    // Determine if CSC is required in SFC pipe
    if (IS_RGB_CSPACE(src->ColorSpace))
    {
        if (IS_YUV_CSPACE(renderTarget->ColorSpace))
        {
            m_renderData.SfcInputCspace = renderTarget->ColorSpace;
        }
        else if (MEDIA_IS_HDCONTENT(src->dwWidth, src->dwHeight))
        {
            m_renderData.SfcInputCspace = CSpace_BT709;
        }
        else
        {
            m_renderData.SfcInputCspace = CSpace_BT601;
        }
    }
    else
    {
        m_renderData.SfcInputCspace = src->ColorSpace;
    }

    if (m_renderData.SfcInputCspace != renderTarget->ColorSpace)
    {
        m_renderData.bCSC = true;
    }
}

void VphalSfcState::DetermineInputFormat(
    PVPHAL_SURFACE                  src,
    PVPHAL_VEBOX_RENDER_DATA        veboxRenderData)
{
    // Determine SFC input surface format
    if (IS_RGB_FORMAT(src->Format))
    {
        m_renderData.SfcInputFormat = Format_AYUV;
    }
    else if (veboxRenderData->bDeinterlace)
    {
        m_renderData.SfcInputFormat = Format_YUY2;
    }
    else
    {
        m_renderData.SfcInputFormat = src->Format;
    }
}

void VphalSfcState::SetRenderingFlags(
    PVPHAL_COLORFILL_PARAMS         pColorFillParams,
    PVPHAL_ALPHA_PARAMS             pAlphaParams,
    PVPHAL_SURFACE                  pSrc,
    PVPHAL_SURFACE                  pRenderTarget,
    PVPHAL_VEBOX_RENDER_DATA        pRenderData)
{
    PRENDERHAL_INTERFACE    pRenderHal;
    float                   fScaleX;
    float                   fScaleY;
    uint32_t                dwSurfaceWidth;
    uint32_t                dwSurfaceHeight;
    uint16_t                wWidthAlignUnit;
    uint16_t                wHeightAlignUnit;
    uint32_t                dwSourceRegionWidth;
    uint32_t                dwSourceRegionHeight;
    uint32_t                dwOutputRegionWidth;
    uint32_t                dwOutputRegionHeight;
    uint32_t                dwVeboxBottom;
    uint32_t                dwVeboxRight;
    VPHAL_COLORPACK         dstColorPack;

    VPHAL_RENDER_CHK_NULL_NO_STATUS(pSrc);
    VPHAL_RENDER_CHK_NULL_NO_STATUS(pRenderTarget);

    pRenderHal       = m_renderHal;
    wWidthAlignUnit  = 1;
    wHeightAlignUnit = 1;
    dwVeboxBottom    = (uint32_t)pSrc->rcSrc.bottom;
    dwVeboxRight     = (uint32_t)pSrc->rcSrc.right;
    dstColorPack     = VpHal_GetSurfaceColorPack(pRenderTarget->Format);

    // Get the SFC input surface size from Vebox
    AdjustBoundary(
        pSrc,
        &dwSurfaceWidth,
        &dwSurfaceHeight);

    // Apply alignment restriction to the source and scaled regions.
    switch (dstColorPack)
    {
        case VPHAL_COLORPACK_420:
            wWidthAlignUnit     = 2;
            wHeightAlignUnit    = 2;
            break;
        case VPHAL_COLORPACK_422:
            wWidthAlignUnit     = 2;
            break;
        default:
            break;
    }

    if(pSrc->bDirectionalScalar)
    {
        dwVeboxBottom *= 2;
        dwVeboxRight  *= 2;
    }

    // Region of the input frame which needs to be processed by SFC
    dwSourceRegionHeight = MOS_ALIGN_FLOOR(
                            MOS_MIN((uint32_t)(dwVeboxBottom - pSrc->rcSrc.top), dwSurfaceHeight),
                            wHeightAlignUnit);
    dwSourceRegionWidth  = MOS_ALIGN_FLOOR(
                            MOS_MIN((uint32_t)(dwVeboxRight  - pSrc->rcSrc.left), dwSurfaceWidth),
                            wWidthAlignUnit);

    // Size of the Output Region over the Render Target
    dwOutputRegionHeight = MOS_ALIGN_CEIL(
                            MOS_MIN((uint32_t)(pSrc->rcDst.bottom - pSrc->rcDst.top), pRenderTarget->dwHeight),
                            wHeightAlignUnit);
    dwOutputRegionWidth  = MOS_ALIGN_CEIL(
                            MOS_MIN((uint32_t)(pSrc->rcDst.right - pSrc->rcDst.left), pRenderTarget->dwWidth),
                            wWidthAlignUnit);

    // Calculate the scaling ratio
    // Both source region and scaled region are pre-rotated
    if (pSrc->Rotation == VPHAL_ROTATION_IDENTITY ||
        pSrc->Rotation == VPHAL_ROTATION_180      ||
        pSrc->Rotation == VPHAL_MIRROR_HORIZONTAL ||
        pSrc->Rotation == VPHAL_MIRROR_VERTICAL)
    {
        fScaleX      = (float)dwOutputRegionWidth  / (float)dwSourceRegionWidth;
        fScaleY      = (float)dwOutputRegionHeight / (float)dwSourceRegionHeight;
    }
    else
    {
        // VPHAL_ROTATION_90 || VPHAL_ROTATION_270 || VPHAL_ROTATE_90_MIRROR_VERTICAL || VPHAL_ROTATE_90_MIRROR_HORIZONTAL
        fScaleX      = (float)dwOutputRegionHeight / (float)dwSourceRegionWidth;
        fScaleY      = (float)dwOutputRegionWidth  / (float)dwSourceRegionHeight;
    }

    // Set RenderData flags
    m_renderData.bScaling   = ((fScaleX == 1.0F) && (fScaleY == 1.0F)) ?
                                 false : true;

    m_renderData.bColorFill = (pColorFillParams && pSrc->InterlacedScalingType == ISCALING_NONE &&
                                  (!RECT1_CONTAINS_RECT2(pSrc->rcDst, pRenderTarget->rcDst))) ?
                                 true : false;

    m_renderData.bIEF       = (pSrc->pIEFParams              &&
                                  pSrc->pIEFParams->bEnabled    &&
                                  (pSrc->pIEFParams->fIEFFactor > 0.0f)) ?
                                 true : false;

    // Determine if CSC is required in SFC pipe
    DetermineCscParams(
        pSrc,
        pRenderTarget);

    // Determine SFC input surface format
    DetermineInputFormat(
        pSrc,
        pRenderData);

    m_renderData.fScaleX            = fScaleX;
    m_renderData.fScaleY            = fScaleY;
    m_renderData.pColorFillParams   = m_renderData.bColorFill ? pColorFillParams : nullptr;
    m_renderData.pAvsParams         = &m_AvsParameters;
    m_renderData.pAlphaParams       = pAlphaParams;
    m_renderData.pSfcPipeOutSurface = pRenderTarget;
    m_renderData.SfcRotation        = pSrc->Rotation;
    m_renderData.SfcScalingMode     = pSrc->ScalingMode;

    // In SFC, we have a lot of HW restrictions on Chroma Sitting Programming.
    // So prevent any invalid input for SFC to avoid HW problems.
    // Prevent invalid input for input surface and format.
    m_renderData.SfcSrcChromaSiting = pSrc->ChromaSiting;
    if (m_renderData.SfcSrcChromaSiting == MHW_CHROMA_SITING_NONE)
    {
        m_renderData.SfcSrcChromaSiting = (CHROMA_SITING_HORZ_LEFT | CHROMA_SITING_VERT_CENTER);
    }
    switch (VpHal_GetSurfaceColorPack(m_renderData.SfcInputFormat))
    {
        case VPHAL_COLORPACK_422:
            m_renderData.SfcSrcChromaSiting = (m_renderData.SfcSrcChromaSiting & 0x7) | CHROMA_SITING_VERT_TOP;
            break;
        case VPHAL_COLORPACK_444:
            m_renderData.SfcSrcChromaSiting = CHROMA_SITING_HORZ_LEFT | CHROMA_SITING_VERT_TOP;
            break;
        default:
            break;
    }
    // Prevent invalid input for output surface and format
    if (pRenderTarget->ChromaSiting == MHW_CHROMA_SITING_NONE)
    {
        pRenderTarget->ChromaSiting = (CHROMA_SITING_HORZ_LEFT | CHROMA_SITING_VERT_CENTER);
    }
    switch (dstColorPack)
    {
        case VPHAL_COLORPACK_422:
            pRenderTarget->ChromaSiting = (pRenderTarget->ChromaSiting & 0x7) | CHROMA_SITING_VERT_TOP;
            break;
        case VPHAL_COLORPACK_444:
            pRenderTarget->ChromaSiting = CHROMA_SITING_HORZ_LEFT | CHROMA_SITING_VERT_TOP;
            break;
        default:
            break;
    }

    m_renderData.bForcePolyPhaseCoefs = VpHal_IsChromaUpSamplingNeeded(pSrc, pRenderTarget);

    // Cache Render Target pointer
    pRenderData->pRenderTarget = pRenderTarget;

    VPHAL_RENDER_NORMALMESSAGE(
        "RenderData: bScaling %d, bColorFill %d, bIEF %d, SfcInputFormat %d, SfcRotation %d, SfcScalingMode %d, SfcSrcChromaSiting %d",
        m_renderData.bScaling,
        m_renderData.bColorFill,
        m_renderData.bIEF,
        m_renderData.SfcInputFormat,
        m_renderData.SfcRotation,
        m_renderData.SfcScalingMode,
        m_renderData.SfcSrcChromaSiting);

finish:
    return;
}

bool VphalSfcState::IsFormatSupported(
    PVPHAL_SURFACE              pSrcSurface,
    PVPHAL_SURFACE              pOutSurface,
    PVPHAL_ALPHA_PARAMS         pAlphaParams)
{
    // Init to false for in case the input parameters are nullptr
    bool ret  = false;

    VPHAL_RENDER_CHK_NULL_NO_STATUS(pSrcSurface);
    VPHAL_RENDER_CHK_NULL_NO_STATUS(pOutSurface);

    // Default to true
    ret = true;

    // Check if Input Format is supported
    if (!IsInputFormatSupported(pSrcSurface))
    {
        VPHAL_RENDER_NORMALMESSAGE("Unsupported Source Format '0x%08x' for SFC.", pSrcSurface->Format);
        ret = false;
        return ret;
    }

    // SFC can not support fp16 output. HDR path is the only way to handle any fp16 output.
    // Before entering into HDR path, it is possible that we need to use SFC to do P010->ARGB10.
    // As for SFC is needed or not, we use bHDRSfc to decide.
    if (pOutSurface->Format == Format_A16R16G16B16F ||
        pOutSurface->Format == Format_A16B16G16R16F)
    {
        ret = false;
        return ret;
    }

    // Check if Output Format is supported
    if (!IsOutputFormatSupported(pOutSurface))
    {
        ret = false;
        return ret;
    }

    // Check if the input/output combination is supported, given certain alpha fill mode.
    // So far SFC only supports filling constant alpha.
    if (pAlphaParams &&
        pAlphaParams->AlphaMode == VPHAL_ALPHA_FILL_MODE_SOURCE_STREAM)
    {
        if ((pOutSurface->Format == Format_A8R8G8B8    ||
             pOutSurface->Format == Format_A8B8G8R8    ||
             pOutSurface->Format == Format_R10G10B10A2 ||
             pOutSurface->Format == Format_B10G10R10A2 ||
             pOutSurface->Format == Format_Y410        ||
             pOutSurface->Format == Format_Y416        ||
             pOutSurface->Format == Format_AYUV)       &&
            (pSrcSurface->Format == Format_A8B8G8R8    ||
             pSrcSurface->Format == Format_A8R8G8B8    ||
             pSrcSurface->Format == Format_Y410        ||
             pSrcSurface->Format == Format_Y416        ||
             pSrcSurface->Format == Format_AYUV))
        {
            ret = false;
        }
    }

finish:
    return ret;
}

void VphalSfcState::FreeResources()
{
    // Free AVS Line Buffer surface for SFC
    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_AVSLineBufferSurface.OsResource);

    // Free IEF Line Buffer surface for SFC
    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_IEFLineBufferSurface.OsResource);

    return;
}

MOS_STATUS VphalSfcState::AllocateResources()
{
    MOS_STATUS              eStatus;
    uint32_t                dwWidth;
    uint32_t                dwHeight;
    uint32_t                dwSize;
    bool                    bAllocated;
    PMHW_SFC_STATE_PARAMS   pSfcStateParams;

    eStatus         = MOS_STATUS_UNKNOWN;
    bAllocated      = false;
    pSfcStateParams = m_renderData.SfcStateParams;

    // Allocate AVS Line Buffer surface----------------------------------------------
    dwWidth  = 1;
    dwHeight = pSfcStateParams->dwInputFrameHeight * SFC_AVS_LINEBUFFER_SIZE_PER_VERTICAL_PIXEL;
    dwSize   = dwWidth * dwHeight;

    VPHAL_RENDER_CHK_STATUS(VpHal_ReAllocateSurface(
        m_osInterface,
        &m_AVSLineBufferSurface,
        "SfcAVSLineBufferSurface",
        Format_Buffer,
        MOS_GFXRES_BUFFER,
        MOS_TILE_LINEAR,
        dwSize,
        1,
        false,
        MOS_MMC_DISABLED,
        &bAllocated));

    // Allocate IEF Line Buffer surface----------------------------------------------
    dwWidth  = 1;
    dwHeight = pSfcStateParams->dwScaledRegionHeight * SFC_IEF_LINEBUFFER_SIZE_PER_VERTICAL_PIXEL;
    dwSize   = dwWidth * dwHeight;

    VPHAL_RENDER_CHK_STATUS(VpHal_ReAllocateSurface(
        m_osInterface,
        &m_IEFLineBufferSurface,
        "SfcIEFLineBufferSurface",
        Format_Buffer,
        MOS_GFXRES_BUFFER,
        MOS_TILE_LINEAR,
        dwSize,
        1,
        false,
        MOS_MMC_DISABLED,
        &bAllocated));

finish:
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        FreeResources();
    }

    return eStatus;
}

void VphalSfcState::GetOutputWidthHeightAlignUnit(
    MOS_FORMAT              outputFormat,
    uint16_t                &widthAlignUnit,
    uint16_t                &heightAlignUnit)
{
    widthAlignUnit  = 1;
    heightAlignUnit = 1;

    switch (VpHal_GetSurfaceColorPack(outputFormat))
    {
        case VPHAL_COLORPACK_420:
            widthAlignUnit  = 2;
            heightAlignUnit = 2;
            break;
        case VPHAL_COLORPACK_422:
            widthAlignUnit  = 2;
            break;
        default:
            break;
    }
}

void VphalSfcState::SetSfcStateInputOrderingMode(
    PVPHAL_VEBOX_RENDER_DATA    veboxRenderData,
    PMHW_SFC_STATE_PARAMS       sfcStateParams)
{
    sfcStateParams->dwVDVEInputOrderingMode = MEDIASTATE_SFC_INPUT_ORDERING_VE_4x8;
}

MOS_STATUS VphalSfcState::SetSfcStateParams(
    PVPHAL_VEBOX_RENDER_DATA    pRenderData,
    PVPHAL_SURFACE              pSrcSurface,
    PVPHAL_SURFACE              pOutSurface)
{
    MOS_STATUS                  eStatus;
    PMOS_INTERFACE              pOsInterface;
    PMHW_SFC_STATE_PARAMS       pSfcStateParams;
    PVPHAL_ALPHA_PARAMS         pAlphaParams;
    VPHAL_COLOR_SAMPLE_8        Src;
    VPHAL_CSPACE                src_cspace, dst_cspace;
    uint16_t                    wOutputWidthAlignUnit;
    uint16_t                    wOutputHeightAlignUnit;
    uint16_t                    wInputWidthAlignUnit;
    uint16_t                    wInputHeightAlignUnit;
    uint32_t                    dwSurfaceWidth;
    uint32_t                    dwSurfaceHeight;
    uint32_t                    dwVeboxBottom;
    uint32_t                    dwVeboxRight;
    VPHAL_GET_SURFACE_INFO      Info;
    VPHAL_COLORPACK             dstColorPack;

    VPHAL_RENDER_CHK_NULL(pSrcSurface);
    VPHAL_RENDER_CHK_NULL(pOutSurface);

    eStatus                = MOS_STATUS_UNKNOWN;
    pOsInterface           = m_osInterface;
    pSfcStateParams        = m_renderData.SfcStateParams;
    pAlphaParams           = m_renderData.pAlphaParams;
    wOutputWidthAlignUnit  = 1;
    wOutputHeightAlignUnit = 1;
    wInputWidthAlignUnit   = 1;
    wInputHeightAlignUnit  = 1;
    dwVeboxBottom          = (uint32_t)pSrcSurface->rcSrc.bottom;
    dwVeboxRight           = (uint32_t)pSrcSurface->rcSrc.right;
    dstColorPack           = VpHal_GetSurfaceColorPack(pOutSurface->Format);

    MOS_ZeroMemory(pSfcStateParams, sizeof(*pSfcStateParams));

    pSfcStateParams->sfcPipeMode = MEDIASTATE_SFC_PIPE_VE_TO_SFC;

    // Setup General params
    // Set chroma subsampling type according to the Vebox output, but
    // when Vebox is bypassed, set it according to the source surface format.
    if (pRenderData->bIECP)
    {
        pSfcStateParams->dwInputChromaSubSampling = MEDIASTATE_SFC_CHROMA_SUBSAMPLING_444;
        m_renderData.SfcSrcChromaSiting           = MHW_CHROMA_SITING_HORZ_LEFT | MHW_CHROMA_SITING_VERT_TOP;
        pSfcStateParams->b8tapChromafiltering     = true;
    }
    else if (pRenderData->bDeinterlace)
    {
        pSfcStateParams->dwInputChromaSubSampling = MEDIASTATE_SFC_CHROMA_SUBSAMPLING_422H;
        pSfcStateParams->b8tapChromafiltering     = false;
    }
    else
    {
        if (m_renderData.SfcInputFormat == Format_NV12   ||
            (m_renderData.SfcInputFormat == Format_P010) ||
            (m_renderData.SfcInputFormat == Format_P016))
        {
            pSfcStateParams->dwInputChromaSubSampling = MEDIASTATE_SFC_CHROMA_SUBSAMPLING_420;
            pSfcStateParams->b8tapChromafiltering     = false;
        }
        else if (VpHal_GetSurfaceColorPack(m_renderData.SfcInputFormat) == VPHAL_COLORPACK_422)
        {
            pSfcStateParams->dwInputChromaSubSampling = MEDIASTATE_SFC_CHROMA_SUBSAMPLING_422H;
            pSfcStateParams->b8tapChromafiltering     = false;
        }
        else if (VpHal_GetSurfaceColorPack(m_renderData.SfcInputFormat) == VPHAL_COLORPACK_444)
        {
            pSfcStateParams->dwInputChromaSubSampling = MEDIASTATE_SFC_CHROMA_SUBSAMPLING_444;
            pSfcStateParams->b8tapChromafiltering     = true;
        }
        else
        {
            pSfcStateParams->dwInputChromaSubSampling = MEDIASTATE_SFC_CHROMA_SUBSAMPLING_400;
            pSfcStateParams->b8tapChromafiltering     = false;
        }
    }

    // Default to Horizontal Left, Vertical Top
    pSfcStateParams->dwChromaDownSamplingVerticalCoef   = (pOutSurface->ChromaSiting & MHW_CHROMA_SITING_VERT_CENTER) ?
                                                          MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_COEF_4_OVER_8 :
                                                          MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_COEF_0_OVER_8;
    pSfcStateParams->dwChromaDownSamplingHorizontalCoef = (pOutSurface->ChromaSiting & MHW_CHROMA_SITING_HORZ_CENTER) ?
                                                          MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_COEF_4_OVER_8 :
                                                          MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_COEF_0_OVER_8;

    // Set the Pre-AVS chroma downsampling param according to SFC i/o chroma subsampling type
    switch (pSfcStateParams->dwInputChromaSubSampling)
    {
        case MEDIASTATE_SFC_CHROMA_SUBSAMPLING_444:
            if (dstColorPack == VPHAL_COLORPACK_420)
            {
                pSfcStateParams->dwChromaDownSamplingMode = MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_444TO420;
            }
            else if (dstColorPack == VPHAL_COLORPACK_422)
            {
                pSfcStateParams->dwChromaDownSamplingMode = MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_444TO422;
            }
            break;

        case MEDIASTATE_SFC_CHROMA_SUBSAMPLING_422H:
            if (dstColorPack == VPHAL_COLORPACK_420)
            {
                pSfcStateParams->dwChromaDownSamplingMode = MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_422TO420;
            }
            break;

        default:
            pSfcStateParams->dwChromaDownSamplingMode = MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_DISABLED;
            break;
    }

    VPHAL_RENDER_NORMALMESSAGE("SfcStateParams: dwInputChromaSubSampling %d, b8tapChromafiltering %d, dwChromaDownSamplingMode %d.",
        pSfcStateParams->dwInputChromaSubSampling,
        pSfcStateParams->b8tapChromafiltering,
        pSfcStateParams->dwChromaDownSamplingMode);

    SetSfcStateInputOrderingMode(pRenderData, pSfcStateParams);

    pSfcStateParams->OutputFrameFormat = pOutSurface->Format;

    pSfcStateParams->fChromaSubSamplingXSiteOffset = 0.0F;
    pSfcStateParams->fChromaSubSamplingYSiteOffset = 0.0F;

    // Setup all parameters in SFC_STATE related to Scaling and AVS
    pSfcStateParams->dwAVSFilterMode = (m_renderData.SfcScalingMode == VPHAL_SCALING_BILINEAR) ?
                                       MEDIASTATE_SFC_AVS_FILTER_BILINEAR                         :
                                       MEDIASTATE_SFC_AVS_FILTER_8x8;

    // Get the SFC input surface size from Vebox
    AdjustBoundary(
        pSrcSurface,
        &dwSurfaceWidth,
        &dwSurfaceHeight);

    // This should be set to the height and width of the frame streaming from Vebox
    pSfcStateParams->dwInputFrameHeight             = dwSurfaceHeight;
    pSfcStateParams->dwInputFrameWidth              = dwSurfaceWidth;

    // Apply alignment restriction to the Region of the output frame.
    GetOutputWidthHeightAlignUnit(
        pSfcStateParams->OutputFrameFormat,
        wOutputWidthAlignUnit,
        wOutputHeightAlignUnit);

    // Apply alignment restriction to Region of the input frame.
    GetInputWidthHeightAlignUnit(
        m_renderData.SfcInputFormat,
        pSfcStateParams->OutputFrameFormat,
        wInputWidthAlignUnit,
        wInputHeightAlignUnit);

    if(pSrcSurface->bDirectionalScalar)
    {
        dwVeboxBottom *= 2;
        dwVeboxRight  *= 2;
    }

    // This should be set to the height and width of the Render Target
    pSfcStateParams->dwOutputFrameHeight            = MOS_ALIGN_CEIL(pOutSurface->dwHeight, wOutputHeightAlignUnit);
    pSfcStateParams->dwOutputFrameWidth             = MOS_ALIGN_CEIL(pOutSurface->dwWidth,  wOutputWidthAlignUnit);

    // Region of the input frame which needs to be processed by SFC
    pSfcStateParams->dwSourceRegionVerticalOffset   = MOS_ALIGN_CEIL((uint32_t)pSrcSurface->rcSrc.top,  wInputHeightAlignUnit);
    pSfcStateParams->dwSourceRegionHorizontalOffset = MOS_ALIGN_CEIL((uint32_t)pSrcSurface->rcSrc.left, wInputWidthAlignUnit);
    pSfcStateParams->dwSourceRegionHeight           = MOS_ALIGN_FLOOR(
                                                        MOS_MIN((uint32_t)(dwVeboxBottom - pSrcSurface->rcSrc.top), pSfcStateParams->dwInputFrameHeight),
                                                        wInputHeightAlignUnit);
    pSfcStateParams->dwSourceRegionWidth            = MOS_ALIGN_FLOOR(
                                                        MOS_MIN((uint32_t)(dwVeboxRight  - pSrcSurface->rcSrc.left), pSfcStateParams->dwInputFrameWidth),
                                                        wInputWidthAlignUnit);

    // Size of the Scaled Region over the Render Target
    pSfcStateParams->dwScaledRegionHeight           = MOS_ALIGN_CEIL(MOS_UF_ROUND(m_renderData.fScaleY * pSfcStateParams->dwSourceRegionHeight), wOutputHeightAlignUnit);
    pSfcStateParams->dwScaledRegionWidth            = MOS_ALIGN_CEIL(MOS_UF_ROUND(m_renderData.fScaleX * pSfcStateParams->dwSourceRegionWidth), wOutputWidthAlignUnit);

    // Scaled region is pre-rotated. Adjust its width and height with those of the output frame
    if (m_renderData.SfcRotation == VPHAL_ROTATION_IDENTITY ||
        m_renderData.SfcRotation == VPHAL_ROTATION_180      ||
        m_renderData.SfcRotation == VPHAL_MIRROR_HORIZONTAL ||
        m_renderData.SfcRotation == VPHAL_MIRROR_VERTICAL)
    {
        pSfcStateParams->dwScaledRegionHeight = MOS_MIN(pSfcStateParams->dwScaledRegionHeight, pSfcStateParams->dwOutputFrameHeight);
        pSfcStateParams->dwScaledRegionWidth  = MOS_MIN(pSfcStateParams->dwScaledRegionWidth, pSfcStateParams->dwOutputFrameWidth);
    }
    else
    {
        pSfcStateParams->dwScaledRegionHeight = MOS_MIN(pSfcStateParams->dwScaledRegionHeight, pSfcStateParams->dwOutputFrameWidth);
        pSfcStateParams->dwScaledRegionWidth  = MOS_MIN(pSfcStateParams->dwScaledRegionWidth, pSfcStateParams->dwOutputFrameHeight);
    }

    // Refine the Scaling ratios in the X and Y direction. SFC output Scaled size may be changed based on the restriction of SFC alignment.
    // The scaling ratio could be changed and not equal to the fScaleX/Y.
    // Driver must make sure that the scaling ratio should be matched with the output/input size before send to HW  
    pSfcStateParams->fAVSXScalingRatio              = (float)pSfcStateParams->dwScaledRegionWidth / (float)pSfcStateParams->dwSourceRegionWidth;
    pSfcStateParams->fAVSYScalingRatio              = (float)pSfcStateParams->dwScaledRegionHeight / (float)pSfcStateParams->dwSourceRegionHeight;

    pSfcStateParams->dwScaledRegionVerticalOffset   = MOS_ALIGN_FLOOR((uint32_t)pSrcSurface->rcDst.top,  wOutputHeightAlignUnit);
    pSfcStateParams->dwScaledRegionHorizontalOffset = MOS_ALIGN_FLOOR((uint32_t)pSrcSurface->rcDst.left, wOutputWidthAlignUnit);

    // Enable Adaptive Filtering for YUV input only, if it is being upscaled
    // in either direction. We must check for this before clamping the SF.
    if (IS_YUV_FORMAT(m_renderData.SfcInputFormat) &&
        (m_renderData.fScaleX > 1.0F               ||
         m_renderData.fScaleY > 1.0F))
    {
        pSfcStateParams->bBypassXAdaptiveFilter = false;
        pSfcStateParams->bBypassYAdaptiveFilter = false;
    }
    else
    {
        pSfcStateParams->bBypassXAdaptiveFilter = true;
        pSfcStateParams->bBypassYAdaptiveFilter = true;
    }

    if (IS_RGB_FORMAT(m_renderData.SfcInputFormat) &&
        pSfcStateParams->b8tapChromafiltering == true)
    {
        pSfcStateParams->bRGBAdaptive = true;
    }
    else
    {
        pSfcStateParams->bRGBAdaptive = false;
    }

    pSfcStateParams->bAVSChromaUpsamplingEnable = (m_renderData.bScaling || m_renderData.bForcePolyPhaseCoefs);

    // Rotation params
    if (m_renderData.SfcRotation <= VPHAL_ROTATION_270)
    {
        // Rotation only
        pSfcStateParams->RotationMode  = VpHal_GetMhwRotationParam(m_renderData.SfcRotation);
        pSfcStateParams->bMirrorEnable = false;
    }
    else if (m_renderData.SfcRotation <= VPHAL_MIRROR_VERTICAL)
    {
        // Mirror only
        pSfcStateParams->dwMirrorType  = VpHal_GetMhwRotationParam(m_renderData.SfcRotation) - 4;
        pSfcStateParams->RotationMode  = MHW_ROTATION_IDENTITY;
        pSfcStateParams->bMirrorEnable = true;
    }
    else
    {
        // Rotation + Mirror
        pSfcStateParams->dwMirrorType  = MHW_MIRROR_HORIZONTAL;
        pSfcStateParams->RotationMode  = VpHal_GetMhwRotationParam(m_renderData.SfcRotation);
        pSfcStateParams->bMirrorEnable = true;
    }

    VPHAL_RENDER_NORMALMESSAGE("SfcStateParams: dwMirrorType %d, RotationMode %d, bMirrorEnable %d.",
        pSfcStateParams->dwMirrorType,
        pSfcStateParams->RotationMode,
        pSfcStateParams->bMirrorEnable);

    // ColorFill params
    if (m_renderData.bColorFill)
    {
        pSfcStateParams->bColorFillEnable = true;

        Src.dwValue = m_renderData.pColorFillParams->Color;
        src_cspace  = m_renderData.pColorFillParams->CSpace;
        dst_cspace  = pOutSurface->ColorSpace;

        // Convert BG color only if not done so before. CSC is expensive!
        if ((m_colorFillColorSrc.dwValue   != Src.dwValue) ||
            (m_colorFillSrcCspace          != src_cspace)  ||
            (m_colorFillRTCspace           != dst_cspace))
        {
            VpHal_CSC_8(&m_colorFillColorDst, &Src, src_cspace, dst_cspace);

            // store the values for next iteration
            m_colorFillColorSrc    = Src;
            m_colorFillSrcCspace   = src_cspace;
            m_colorFillRTCspace    = dst_cspace;
        }

        if (IS_YUV_FORMAT(pOutSurface->Format) || (pOutSurface->Format == Format_AYUV))
        {
            pSfcStateParams->fColorFillYRPixel = (float)m_colorFillColorDst.Y / 255.0F;
            pSfcStateParams->fColorFillUGPixel = (float)m_colorFillColorDst.U / 255.0F;
            pSfcStateParams->fColorFillVBPixel = (float)m_colorFillColorDst.V / 255.0F;
        }
        else
        {
            // Swap the channel here because HW only natively supports XBGR output
            if ((pOutSurface->Format == Format_A8R8G8B8) || (pOutSurface->Format == Format_X8R8G8B8) || (pOutSurface->Format == Format_R10G10B10A2))
            {
                pSfcStateParams->fColorFillYRPixel = (float)m_colorFillColorDst.B / 255.0F;
                pSfcStateParams->fColorFillUGPixel = (float)m_colorFillColorDst.G / 255.0F;
                pSfcStateParams->fColorFillVBPixel = (float)m_colorFillColorDst.R / 255.0F;
            }
            else
            {
                pSfcStateParams->fColorFillYRPixel = (float)m_colorFillColorDst.R / 255.0F;
                pSfcStateParams->fColorFillUGPixel = (float)m_colorFillColorDst.G / 255.0F;
                pSfcStateParams->fColorFillVBPixel = (float)m_colorFillColorDst.B / 255.0F;
            }
        }
        pSfcStateParams->fColorFillAPixel  = (float)Src.A / 255.0F;
    }

    if (pAlphaParams                              &&
        ((pOutSurface->Format == Format_A8R8G8B8) ||
         (pOutSurface->Format == Format_A8B8G8R8) ||
         (pOutSurface->Format  == Format_AYUV)))
    {
        switch (pAlphaParams->AlphaMode)
        {
            case VPHAL_ALPHA_FILL_MODE_NONE:
                pSfcStateParams->fAlphaPixel      = pAlphaParams->fAlpha;
                pSfcStateParams->fColorFillAPixel = pAlphaParams->fAlpha;
                break;

            case VPHAL_ALPHA_FILL_MODE_BACKGROUND:
                pSfcStateParams->fAlphaPixel = m_renderData.bColorFill ?
                    pSfcStateParams->fColorFillAPixel : 1.0F;
                break;

            case VPHAL_ALPHA_FILL_MODE_SOURCE_STREAM:
            case VPHAL_ALPHA_FILL_MODE_OPAQUE:
            default:
                pSfcStateParams->fAlphaPixel      = 1.0F;
                pSfcStateParams->fColorFillAPixel = 1.0F;
        }
    }
    else
    {
        pSfcStateParams->fAlphaPixel = 1.0F;
    }

    // CSC params
    pSfcStateParams->bCSCEnable      = m_renderData.bCSC;

    // ARGB8,ABGR10 output format need to enable swap
    if (pOutSurface->Format == Format_X8R8G8B8 ||
        pOutSurface->Format == Format_A8R8G8B8 ||
        pOutSurface->Format == Format_R10G10B10A2)
    {
        pSfcStateParams->bRGBASwapEnable = true;
    }
    else
    {
        pSfcStateParams->bRGBASwapEnable = false;
    }

    if (IS_RGB_CSPACE(pSrcSurface->ColorSpace))
    {
        pSfcStateParams->bInputColorSpace = true;
    }
    else
    {
        pSfcStateParams->bInputColorSpace = false;
    }

    VPHAL_RENDER_NORMALMESSAGE("SfcStateParams: bCSCEnable %d, bRGBASwapEnable %d, bMirrorEnable %d.",
        pSfcStateParams->bCSCEnable,
        pSfcStateParams->bRGBASwapEnable,
        pSfcStateParams->bMirrorEnable);

    // Set MMC status
    VPHAL_RENDER_CHK_STATUS(SetSfcMmcStatus(
        pRenderData,
        pOutSurface,
        pSfcStateParams));

    VPHAL_RENDER_CHK_STATUS(AllocateResources());

    // Set OS resources used by SFC state
    pSfcStateParams->pOsResAVSLineBuffer = &m_AVSLineBufferSurface.OsResource;
    pSfcStateParams->pOsResIEFLineBuffer = &m_IEFLineBufferSurface.OsResource;
    pSfcStateParams->pOsResOutputSurface = &pOutSurface->OsResource;

    MOS_ZeroMemory(&Info, sizeof(VPHAL_GET_SURFACE_INFO));

    Info.S3dChannel = pOutSurface->Channel;
    Info.ArraySlice = m_currentChannel;

    VPHAL_RENDER_CHK_STATUS(VpHal_GetSurfaceInfo(
        pOsInterface,
        &Info,
        pOutSurface));

    pSfcStateParams->dwOutputSurfaceOffset  = pOutSurface->YPlaneOffset.iSurfaceOffset;
    pSfcStateParams->wOutputSurfaceUXOffset = (uint16_t) pOutSurface->UPlaneOffset.iXOffset;
    pSfcStateParams->wOutputSurfaceUYOffset = (uint16_t) pOutSurface->UPlaneOffset.iYOffset;
    pSfcStateParams->wOutputSurfaceVXOffset = (uint16_t) pOutSurface->VPlaneOffset.iXOffset;
    pSfcStateParams->wOutputSurfaceVYOffset = (uint16_t) pOutSurface->VPlaneOffset.iYOffset;

finish:
    return eStatus;
}

MOS_STATUS VphalSfcState::SetSfcMmcStatus(
    PVPHAL_VEBOX_RENDER_DATA renderData,
    PVPHAL_SURFACE           outSurface,
    PMHW_SFC_STATE_PARAMS    sfcStateParams)
{
    MOS_STATUS       eStatus = MOS_STATUS_SUCCESS;

    if (IsFormatMMCSupported(outSurface->Format)        &&                 // SFC output MMC only support several format
        (renderData->Component      == COMPONENT_VPreP) &&                 // SFC output MMC only enable in Vprep
        (renderData->bEnableMMC     == true)            &&
        (outSurface->bCompressible  == true)            &&
        (outSurface->TileType       == MOS_TILE_Y))
    {
        if ((m_renderData.fScaleX >= 0.5F) &&
            (m_renderData.fScaleY >= 0.5F))
        {
            sfcStateParams->bMMCEnable = true;
            sfcStateParams->MMCMode    = MOS_MMC_HORIZONTAL;
        }
        else if ((m_renderData.fScaleX < 0.5F) &&
                 (m_renderData.fScaleY < 0.5F))
        {
            sfcStateParams->bMMCEnable = true;
            sfcStateParams->MMCMode    = MOS_MMC_VERTICAL;
        }
        else
        {
            sfcStateParams->bMMCEnable = false;
            sfcStateParams->MMCMode    = MOS_MMC_DISABLED;
        }

        VPHAL_RENDER_NORMALMESSAGE("SfcStateParams: bMMCEnable %d, MMCMode %d.",
            sfcStateParams->bMMCEnable,
            sfcStateParams->MMCMode);

        // Set mmc status output surface for output surface
        m_osInterface->pfnSetMemoryCompressionMode(m_osInterface, &outSurface->OsResource, MOS_MEMCOMP_STATE(sfcStateParams->MMCMode));
    }

    return eStatus;
}

MOS_STATUS VphalSfcState::SetAvsStateParams()
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;
    PMHW_SFC_AVS_STATE          pMhwAvsState;

    VPHAL_RENDER_CHK_NULL(m_sfcInterface);

    pMhwAvsState    = &m_avsState.AvsStateParams;

    pMhwAvsState->dwInputHorizontalSiting = (m_renderData.SfcSrcChromaSiting  & MHW_CHROMA_SITING_HORZ_CENTER) ? SFC_AVS_INPUT_SITING_COEF_4_OVER_8 :
                                            ((m_renderData.SfcSrcChromaSiting & MHW_CHROMA_SITING_HORZ_RIGHT)  ? SFC_AVS_INPUT_SITING_COEF_8_OVER_8 :
                                            SFC_AVS_INPUT_SITING_COEF_0_OVER_8);

    pMhwAvsState->dwInputVerticalSitting  = (m_renderData.SfcSrcChromaSiting  & MHW_CHROMA_SITING_VERT_CENTER) ? SFC_AVS_INPUT_SITING_COEF_4_OVER_8 :
                                            ((m_renderData.SfcSrcChromaSiting & MHW_CHROMA_SITING_VERT_BOTTOM) ? SFC_AVS_INPUT_SITING_COEF_8_OVER_8 :
                                            SFC_AVS_INPUT_SITING_COEF_0_OVER_8);

    if (m_renderData.SfcSrcChromaSiting == MHW_CHROMA_SITING_NONE)
    {
        m_renderData.SfcSrcChromaSiting = MHW_CHROMA_SITING_HORZ_LEFT | MHW_CHROMA_SITING_VERT_TOP;

        if (VpHal_GetSurfaceColorPack(m_renderData.SfcInputFormat) == VPHAL_COLORPACK_420)  // For 420, default is Left & Center, else default is Left & Top
        {
            pMhwAvsState->dwInputVerticalSitting = SFC_AVS_INPUT_SITING_COEF_4_OVER_8;
        }
    }

    m_renderData.pAvsParams->bForcePolyPhaseCoefs = m_renderData.bForcePolyPhaseCoefs;

    VPHAL_RENDER_CHK_STATUS(m_sfcInterface->SetSfcSamplerTable(
        &m_avsState.LumaCoeffs,
        &m_avsState.ChromaCoeffs,
        m_renderData.pAvsParams,
        m_renderData.SfcInputFormat,
        m_renderData.fScaleX,
        m_renderData.fScaleY,
        m_renderData.SfcSrcChromaSiting,
        true));

finish:
    return eStatus;
}

void VphalSfcState::SetIefStateCscParams(
    PMHW_SFC_STATE_PARAMS           sfcStateParams,
    PMHW_SFC_IEF_STATE_PARAMS       iefStateParams)
{

    // Setup CSC params
    if (m_renderData.bCSC)
    {
        sfcStateParams->bCSCEnable = true;
        iefStateParams->bCSCEnable = true;

        // Calculate matrix if not done so before. CSC is expensive!
        if ((m_cscInputCspace  != m_renderData.SfcInputCspace)    ||
            (m_cscRTCspace     != m_renderData.pSfcPipeOutSurface->ColorSpace))
        {
            // Get the matrix to use for conversion
            VpHal_GetCscMatrix(
                m_renderData.SfcInputCspace,
                m_renderData.pSfcPipeOutSurface->ColorSpace,
                m_cscCoeff,
                m_cscInOffset,
                m_cscOutOffset);

            // Store it for next BLT
            m_cscInputCspace   = m_renderData.SfcInputCspace;
            m_cscRTCspace      = m_renderData.pSfcPipeOutSurface->ColorSpace;
        }

        // Copy the values into IEF Params
        iefStateParams->pfCscCoeff     = m_cscCoeff;
        iefStateParams->pfCscInOffset  = m_cscInOffset;
        iefStateParams->pfCscOutOffset = m_cscOutOffset;
    }

}

void VphalSfcState::SetIefStateParams(
    PVPHAL_VEBOX_RENDER_DATA        veboxRenderData,
    PMHW_SFC_STATE_PARAMS           sfcStateParams,
    PVPHAL_SURFACE                  inputSurface)
{
    PMHW_SFC_IEF_STATE_PARAMS   iefStateParams;

    MOS_UNUSED(veboxRenderData);
    VPHAL_RENDER_CHK_NULL_NO_STATUS(sfcStateParams);
    VPHAL_RENDER_CHK_NULL_NO_STATUS(inputSurface);

    iefStateParams = &m_renderData.IEFStateParams;
    MOS_ZeroMemory(iefStateParams, sizeof(*iefStateParams));

    // Setup IEF and STE params
    if (m_renderData.bIEF)
    {
        Ief ief(
            inputSurface);

        ief.SetHwState(
            sfcStateParams,
            iefStateParams);
    } // end of setup IEF and STE params

    // Setup CSC params
    SetIefStateCscParams(
        sfcStateParams,
        iefStateParams);

finish:
    return;
}

MOS_STATUS VphalSfcState::UpdateRenderingFlags(
    PVPHAL_SURFACE                  pSrcSurface,
    PVPHAL_SURFACE                  pOutSurface,
    PVPHAL_VEBOX_RENDER_DATA        pRenderData)
{
    MOS_STATUS                      eStatus;

    MOS_UNUSED(pSrcSurface);
    MOS_UNUSED(pOutSurface);
    MOS_UNUSED(pRenderData);

    eStatus = MOS_STATUS_SUCCESS;

    return eStatus;
}

MOS_STATUS VphalSfcState::SetupSfcState(
    PVPHAL_SURFACE                  pSrcSurface,
    PVPHAL_SURFACE                  pOutSurface,
    PVPHAL_VEBOX_RENDER_DATA        pRenderData)
{
    MOS_STATUS                      eStatus;
    PMOS_INTERFACE                  pOsInterface;
    PRENDERHAL_INTERFACE            pRenderHal;

    VPHAL_RENDER_CHK_NULL(pSrcSurface);
    VPHAL_RENDER_CHK_NULL(pOutSurface);
    VPHAL_RENDER_CHK_NULL(pRenderData);

    eStatus        = MOS_STATUS_UNKNOWN;
    pOsInterface   = m_osInterface;
    pRenderHal     = m_renderHal;

    // Update SFC rendering flags if any
    VPHAL_RENDER_CHK_STATUS(UpdateRenderingFlags(
        pSrcSurface,
        pOutSurface,
        pRenderData));

    // Setup params related to SFC_STATE
    VPHAL_RENDER_CHK_STATUS(SetSfcStateParams(
            pRenderData,
            pSrcSurface,
            pOutSurface));

    // Setup params related to SFC_AVS_STATE
    if (m_renderData.bScaling ||
        m_renderData.bForcePolyPhaseCoefs)
    {
        VPHAL_RENDER_CHK_STATUS(SetAvsStateParams());
    }

    // Setup params related to SFC_IEF_STATE
    if (m_renderData.bIEF ||
        m_renderData.bCSC)
    {
        SetIefStateParams(
            pRenderData,
            m_renderData.SfcStateParams,
            pSrcSurface);
    }

finish:
    return eStatus;
}

MOS_STATUS VphalSfcState::SendSfcCmd(
    PVPHAL_VEBOX_RENDER_DATA        pRenderData,
    PMOS_COMMAND_BUFFER             pCmdBuffer)
{
    PMHW_SFC_INTERFACE              pSfcInterface;
    MHW_SFC_LOCK_PARAMS             SfcLockParams;
    MOS_STATUS                      eStatus;
    MHW_SFC_OUT_SURFACE_PARAMS      OutSurfaceParam;

    VPHAL_RENDER_CHK_NULL(m_sfcInterface);
    VPHAL_RENDER_CHK_NULL(m_osInterface);
    VPHAL_RENDER_CHK_NULL(pRenderData);
    VPHAL_RENDER_CHK_NULL(pCmdBuffer);

    eStatus                 = MOS_STATUS_SUCCESS;
    pSfcInterface           = m_sfcInterface;

    // Ensure VEBOX can write
    m_osInterface->pfnSyncOnResource(
        m_osInterface,
        &m_renderData.pSfcPipeOutSurface->OsResource,
        MOS_GPU_CONTEXT_VEBOX,
        true);

    if (m_renderData.pSfcPipeOutSurface->bOverlay)
    {
        m_osInterface->pfnSyncOnOverlayResource(
            m_osInterface,
            &m_renderData.pSfcPipeOutSurface->OsResource,
            MOS_GPU_CONTEXT_VEBOX);
    }

    // Setup params for SFC Lock command
    SfcLockParams.sfcPipeMode           = MhwSfcInterface::SFC_PIPE_MODE_VEBOX;
    SfcLockParams.bOutputToMemory = (pRenderData->bDeinterlace || pRenderData->bDenoise);

    // Send SFC_LOCK command to acquire SFC pipe for Vebox
    VPHAL_RENDER_CHK_STATUS(pSfcInterface->AddSfcLock(
        pCmdBuffer,
        &SfcLockParams));

    VPHAL_RENDER_CHK_STATUS(VpHal_InitMhwOutSurfParams(
        m_renderData.pSfcPipeOutSurface,
        &OutSurfaceParam));

    // Send SFC MMCD cmd
    VPHAL_RENDER_CHK_STATUS(RenderSfcMmcCMD(
        pSfcInterface,
        m_renderHal->pMhwMiInterface,
        m_osInterface,
        &OutSurfaceParam,
        pCmdBuffer));

    // Send SFC_STATE command
    VPHAL_RENDER_CHK_STATUS(pSfcInterface->AddSfcState(
        pCmdBuffer,
        m_renderData.SfcStateParams,
        &OutSurfaceParam));

    if (m_renderData.bScaling ||
        m_renderData.bForcePolyPhaseCoefs)
    {
        // Send SFC_AVS_STATE command
        VPHAL_RENDER_CHK_STATUS(pSfcInterface->AddSfcAvsState(
            pCmdBuffer,
            &m_avsState.AvsStateParams));

        // Send SFC_AVS_LUMA_TABLE command
        VPHAL_RENDER_CHK_STATUS(pSfcInterface->AddSfcAvsLumaTable(
            pCmdBuffer,
            &m_avsState.LumaCoeffs));

        // Send SFC_AVS_CHROMA_TABLE command
        VPHAL_RENDER_CHK_STATUS(pSfcInterface->AddSfcAvsChromaTable(
            pCmdBuffer,
            &m_avsState.ChromaCoeffs));
    }

    // Send SFC_IEF_STATE command
    if (m_renderData.bIEF || m_renderData.bCSC)
    {
        VPHAL_RENDER_CHK_STATUS(pSfcInterface->AddSfcIefState(
            pCmdBuffer,
            &m_renderData.IEFStateParams));
    }

    // Send SFC_FRAME_START command to start processing a frame
    VPHAL_RENDER_CHK_STATUS(pSfcInterface->AddSfcFrameStart(
        pCmdBuffer,
        MhwSfcInterface::SFC_PIPE_MODE_VEBOX));

finish:
    return eStatus;
}

#endif // __VPHAL_SFC_SUPPORTED
