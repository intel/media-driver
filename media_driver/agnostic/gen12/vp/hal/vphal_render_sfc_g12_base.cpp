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
//! \file     vphal_render_sfc_g12_base.cpp
//! \brief    VPHAL SFC Gen12 rendering component
//! \details  The SFC renderer supports Scaling, IEF, CSC/ColorFill and Rotation.
//!           It's responsible for setting up HW states and generating the SFC
//!           commands.
//!
#include "mhw_sfc_g12_X.h"
#include "vphal_render_vebox_base.h"
#include "vphal_render_sfc_g12_base.h"
#include "media_user_settings_mgr_g12.h"

#if __VPHAL_SFC_SUPPORTED

VphalSfcStateG12::VphalSfcStateG12(
    PMOS_INTERFACE       osInterface,
    PRENDERHAL_INTERFACE renderHal,
    PMHW_SFC_INTERFACE   sfcInterface)
    : VphalSfcState(osInterface, renderHal, sfcInterface)
{
    MOS_USER_FEATURE_VALUE_DATA UserFeatureData;
    MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
    MOS_USER_FEATURE_INVALID_KEY_ASSERT(MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_SFC_OUTPUT_CENTERING_DISABLE_ID_G12,
        &UserFeatureData));

    // Setup disable render flag controlled by a user feature key for validation purpose
    // Enable output centering by default on Gen12+
    m_disableOutputCentering = UserFeatureData.bData ? true : false;
}

bool VphalSfcStateG12::IsInputFormatSupported(
    PVPHAL_SURFACE              srcSurface)
{
    bool ret = true;
    // Check if Input Format is supported
    if ((srcSurface->Format != Format_NV12)     &&
        (srcSurface->Format != Format_AYUV)     &&
        (srcSurface->Format != Format_P010)     &&
        (srcSurface->Format != Format_P016)     &&
        (srcSurface->Format != Format_Y410)     &&
        (srcSurface->Format != Format_Y210)     &&
        (srcSurface->Format != Format_Y416)     &&
        (srcSurface->Format != Format_Y216)     &&
        (srcSurface->Format != Format_A8B8G8R8) &&
        (srcSurface->Format != Format_X8B8G8R8) &&
        (srcSurface->Format != Format_A8R8G8B8) &&
        (srcSurface->Format != Format_X8R8G8B8) &&
        !IS_PA_FORMAT(srcSurface->Format))
    {
        VPHAL_RENDER_NORMALMESSAGE("Unsupported Source Format '0x%08x' for SFC.", srcSurface->Format);
        ret = false;
    }

    return ret;
}

bool VphalSfcStateG12::IsOutputFormatSupported(
    PVPHAL_SURFACE              outSurface)
{
    bool ret = true;

    if (!IS_RGB32_FORMAT(outSurface->Format)   &&
        // Remove RGB565 support due to quality issue, may reopen this after root cause in the future.
        //!IS_RGB16_FORMAT(outSurface->Format)   &&
        outSurface->Format != Format_YUY2      &&
        outSurface->Format != Format_UYVY      &&
        outSurface->Format != Format_AYUV      &&
        outSurface->Format != Format_Y210      &&
        outSurface->Format != Format_Y410      &&
        outSurface->Format != Format_Y216      &&
        outSurface->Format != Format_Y416)
    {
        if (outSurface->TileType == MOS_TILE_Y &&
             (outSurface->Format == Format_P010 ||
              outSurface->Format == Format_P016 ||
              outSurface->Format == Format_NV12))
        {
            ret = true;
        }
        else
        {
            VPHAL_RENDER_NORMALMESSAGE("Unsupported Render Target Format '0x%08x' for SFC Pipe.", outSurface->Format);
            ret = false;
        }
    }

    return ret;
}

void VphalSfcStateG12::GetInputWidthHeightAlignUnit(
    MOS_FORMAT              inputFormat,
    MOS_FORMAT              outputFormat,
    uint16_t                &widthAlignUnit,
    uint16_t                &heightAlignUnit)
{
    MOS_UNUSED(outputFormat);
    widthAlignUnit  = 1;
    heightAlignUnit = 1;

    // Apply alignment restriction to Region of the input frame.
    switch (VpHal_GetSurfaceColorPack(inputFormat))
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

bool VphalSfcStateG12::IsOutputCapable(
    bool            isColorFill,
    PVPHAL_SURFACE  src,
    PVPHAL_SURFACE  renderTarget)
{
    bool isOutputCapable = false;
    // Only Gen12 later H/W support ColorFill, the (OffsetX, OffsetY)
    // of scaled region not being (0, 0) or the tile type not being
    // Tile_Y on NV12/P010/P016 output surface. Disable SFC even if other
    // features are supported before Gen12.
    if ((isColorFill         ||
        src->rcDst.top  != 0 ||
        src->rcDst.left != 0 ||
        renderTarget->TileType != MOS_TILE_Y) &&
        (renderTarget->Format == Format_NV12 ||
         renderTarget->Format == Format_P010 ||
         renderTarget->Format == Format_P016))
    {
        if ((renderTarget->TileType == MOS_TILE_Y))
        {
            isOutputCapable = true;
        }
        else
        {
            isOutputCapable = false;
        }
    }
    else
    {
        isOutputCapable = true;
    }

    return isOutputCapable;
}

void VphalSfcStateG12::DetermineCscParams(
    PVPHAL_SURFACE                  src,
    PVPHAL_SURFACE                  renderTarget)
{
    // Determine if CSC is required in SFC pipe
    m_renderData.SfcInputCspace = src->ColorSpace;

    if (m_renderData.SfcInputCspace != renderTarget->ColorSpace)
    {
        m_renderData.bCSC = true;
    }
}

void VphalSfcStateG12::DetermineInputFormat(
    PVPHAL_SURFACE                  src,
    PVPHAL_VEBOX_RENDER_DATA        veboxRenderData)
{
    // Determine SFC input surface format
    if (IS_RGB_FORMAT(src->Format))
    {
        m_renderData.SfcInputFormat = src->Format;
    }
    else if (veboxRenderData->bDeinterlace || veboxRenderData->bQueryVariance)
    {
        m_renderData.SfcInputFormat = Format_YUY2;
    }
    else
    {
        m_renderData.SfcInputFormat = src->Format;
    }
}

MOS_STATUS VphalSfcStateG12::SetSfcStateParams(
    PVPHAL_VEBOX_RENDER_DATA    pRenderData,
    PVPHAL_SURFACE              pSrcSurface,
    PVPHAL_SURFACE              pOutSurface)
{
    MOS_STATUS                  eStatus;
    PMHW_SFC_STATE_PARAMS       sfcStateParams;

    eStatus                = MOS_STATUS_UNKNOWN;
    sfcStateParams         = m_renderData.SfcStateParams;

    eStatus = VphalSfcState::SetSfcStateParams(pRenderData, pSrcSurface, pOutSurface);

    VPHAL_RENDER_CHK_NULL_RETURN(m_sfcInterface);
    // Output centering
    if (!m_disableOutputCentering)
    {
        dynamic_cast<MhwSfcInterfaceG12 *>(m_sfcInterface)->IsOutPutCenterEnable(true);
    }
    else
    {
        dynamic_cast<MhwSfcInterfaceG12 *>(m_sfcInterface)->IsOutPutCenterEnable(false);
    }

    // ARGB8,ABGR10,A16B16G16R16,VYUY and YVYU output format need to enable swap
    if (pOutSurface->Format == Format_X8R8G8B8     ||
        pOutSurface->Format == Format_A8R8G8B8     ||
        pOutSurface->Format == Format_R10G10B10A2  ||
        pOutSurface->Format == Format_A16B16G16R16 ||
        pOutSurface->Format == Format_VYUY         ||
        pOutSurface->Format == Format_YVYU)
    {
        sfcStateParams->bRGBASwapEnable = true;
    }
    else
    {
        sfcStateParams->bRGBASwapEnable = false;
    }

    return eStatus;
}

bool VphalSfcStateG12::IsOutputPipeSfcFeasible(
    PCVPHAL_RENDER_PARAMS       pcRenderParams,
    PVPHAL_SURFACE              pSrcSurface,
    PVPHAL_SURFACE              pRenderTarget)
{
    //!
    //! \brief SFC can be the output pipe when the following conditions are all met
    //!        1.  User feature keys value "SFC Disable" is false
    //!        2.  Single render target only
    //!        3.  Rotation disabled or ONLY Rotation enabled when the SFC output is Y-tile
    //!        4.  For TGL, SFC support Mirror+Rotation when SFC output is Y-tile
    //!        5.  i/o format is supported by SFC, taking into account the alpha fill info
    //!        6. Comp DI(ARGB/ABGR) is disabled
    //!        7. Variance Query is disabled
    //!
    if (IsDisabled()                            == false                                        &&
        pcRenderParams->uDstCount               == 1                                            &&
        (pSrcSurface->Rotation                  == VPHAL_ROTATION_IDENTITY                      ||
         (pSrcSurface->Rotation                 <= VPHAL_ROTATION_270                           &&
          pcRenderParams->pTarget[0]->TileType  == MOS_TILE_Y)                                  ||
         (pSrcSurface->Rotation                 <= VPHAL_ROTATE_90_MIRROR_HORIZONTAL            &&
          pcRenderParams->pTarget[0]->TileType  == MOS_TILE_Y                                   &&
          GFX_IS_GEN_12_OR_LATER(m_renderHal->Platform)))                                       &&
        IsFormatSupported(pSrcSurface, pcRenderParams->pTarget[0], pcRenderParams->pCompAlpha)  &&
        (pSrcSurface->pDeinterlaceParams        == nullptr                                      ||
         (pSrcSurface->Format != Format_A8R8G8B8 && pSrcSurface->Format != Format_A8B8G8R8))    &&
        pSrcSurface->bQueryVariance             == false)
    {
        return true;
    }

    return false;
}

void VphalSfcStateG12::SetRenderingFlags(
    PVPHAL_COLORFILL_PARAMS  pColorFillParams, 
    PVPHAL_ALPHA_PARAMS      pAlphaParams, 
    PVPHAL_SURFACE           pSrc, 
    PVPHAL_SURFACE           pRenderTarget, 
    PVPHAL_VEBOX_RENDER_DATA pRenderData)
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
    VPHAL_COLORPACK         srcColorPack;
    VPHAL_COLORPACK         dstColorPack;

    VPHAL_RENDER_CHK_NULL_NO_STATUS(pSrc);
    VPHAL_RENDER_CHK_NULL_NO_STATUS(pRenderTarget);

    pRenderHal       = m_renderHal;
    wWidthAlignUnit  = 1;
    wHeightAlignUnit = 1;
    dwVeboxBottom    = (uint32_t)pSrc->rcSrc.bottom;
    dwVeboxRight     = (uint32_t)pSrc->rcSrc.right;
    srcColorPack     = VpHal_GetSurfaceColorPack(pSrc->Format);
    dstColorPack     = VpHal_GetSurfaceColorPack(pRenderTarget->Format);

    // Get the SFC input surface size from Vebox
    AdjustBoundary(
        pSrc,
        &dwSurfaceWidth,
        &dwSurfaceHeight);

    // Apply alignment restriction to the source regions.
    switch (srcColorPack)
    {
        case VPHAL_COLORPACK_420:
            wWidthAlignUnit     = 2;
            wHeightAlignUnit    = 2;
            break;
        case VPHAL_COLORPACK_422:
            wWidthAlignUnit     = 2;
            wHeightAlignUnit    = 1;
            break;
        default:
            wWidthAlignUnit     = 1;
            wHeightAlignUnit    = 1;
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

    // Apply alignment restriction to the scaled regions.
    switch (dstColorPack)
    {
        case VPHAL_COLORPACK_420:
            wWidthAlignUnit     = 2;
            wHeightAlignUnit    = 2;
            break;
        case VPHAL_COLORPACK_422:
            wWidthAlignUnit     = 2;
            wHeightAlignUnit    = 1;
            break;
        default:
            wWidthAlignUnit     = 1;
            wHeightAlignUnit    = 1;
            break;
    }
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

finish:
    return;
}

MOS_STATUS VphalSfcStateG12::SetSfcMmcStatus(
    PVPHAL_VEBOX_RENDER_DATA renderData,
    PVPHAL_SURFACE           outSurface,
    PMHW_SFC_STATE_PARAMS    sfcStateParams)
{
    MOS_STATUS       eStatus = MOS_STATUS_SUCCESS;

    if (outSurface->CompressionMode               &&
        IsFormatMMCSupported(outSurface->Format)  &&
        outSurface->TileType == MOS_TILE_Y        &&
        IsSfcMmcEnabled())
    {
        sfcStateParams->bMMCEnable = true;
        sfcStateParams->MMCMode    = outSurface->CompressionMode;
    }
    else
    {
        sfcStateParams->bMMCEnable = false;
    }

    return eStatus;
}

bool VphalSfcStateG12::IsFormatMMCSupported(MOS_FORMAT Format)
{
    // Check if Sample Format is supported
    if ((Format != Format_YUY2) &&
        (Format != Format_Y210) &&
        (Format != Format_Y410) &&
        (Format != Format_Y216) &&
        (Format != Format_Y416) &&
        (Format != Format_P010) &&
        (Format != Format_P016) &&
        (Format != Format_AYUV) &&
        (Format != Format_NV21) &&
        (Format != Format_NV12) &&
        (Format != Format_UYVY) &&
        (Format != Format_YUYV) &&
        (Format != Format_R10G10B10A2)   &&
        (Format != Format_B10G10R10A2)   &&
        (Format != Format_A8B8G8R8)      &&
        (Format != Format_X8B8G8R8)      &&
        (Format != Format_A8R8G8B8)      &&
        (Format != Format_X8R8G8B8)      &&
        (Format != Format_A16B16G16R16F) &&
        (Format != Format_A16R16G16B16F))
    {
        VPHAL_RENDER_NORMALMESSAGE("Unsupported Format '0x%08x' for SFC MMC.", Format);
        return false;
    }

    return true;
}

#endif // __VPHAL_SFC_SUPPORTED
