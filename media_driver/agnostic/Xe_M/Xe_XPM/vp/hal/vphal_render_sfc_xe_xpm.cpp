/*===================== begin_copyright_notice ==================================

# Copyright (c) 2020-2021, Intel Corporation

# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.

======================= end_copyright_notice ==================================*/
//!
//! \file     vphal_render_sfc_xe_xpm.cpp
//! \brief    VPHAL SFC rendering component for Xe_XPM
//! \details  The SFC renderer supports Scaling, IEF, CSC/ColorFill and Rotation.
//!           It's responsible for setting up HW states and generating the SFC
//!           commands.
//!
#include "mhw_sfc_xe_xpm.h"
#include "vphal_render_vebox_base.h"
#include "vphal_render_sfc_xe_xpm.h"
#include "vp_hal_ddi_utils.h"

#if __VPHAL_SFC_SUPPORTED

VphalSfcStateXe_Xpm::VphalSfcStateXe_Xpm(
    PMOS_INTERFACE       osInterface,
    PRENDERHAL_INTERFACE renderHal,
    PMHW_SFC_INTERFACE   sfcInterface) :
    VphalSfcState(osInterface, renderHal, sfcInterface),
    VphalSfcStateG12(osInterface, renderHal, sfcInterface)
{
    // get dithering flag.
    ReadUserSetting(
        m_userSettingPtr,
        m_disableSfcDithering,
        __MEDIA_USER_FEATURE_VALUE_SFC_OUTPUT_DTR_DISABLE,
        MediaUserSetting::Group::Sequence,
        0,
        true);
    VP_PUBLIC_NORMALMESSAGE("m_disableSfcDithering = %d", m_disableSfcDithering);

#if LINUX
    char *Sfc2PassPerfMode = getenv("SET_SFC2PASS_PERFMODE");
    if (Sfc2PassPerfMode)
    {
        m_bSFC2PassPerfMode = strcmp(Sfc2PassPerfMode, "ON")?false:true;
    }
#endif
}

bool VphalSfcStateXe_Xpm::IsOutputFormatSupported(
    PVPHAL_SURFACE              outSurface)
{
    bool ret = true;

    if (!IS_RGB32_FORMAT(outSurface->Format)   &&
        !IS_RGB64_FORMAT(outSurface->Format)   &&
        // Remove RGB565 support due to quality issue, may reopen this after root cause in the future.
        //!IS_RGB16_FORMAT(outSurface->Format)   &&
        outSurface->Format != Format_NV12      &&
        outSurface->Format != Format_YUY2      &&
        outSurface->Format != Format_UYVY      &&
        outSurface->Format != Format_AYUV      &&
        outSurface->Format != Format_Y210      &&
        outSurface->Format != Format_Y410      &&
        outSurface->Format != Format_Y216      &&
        outSurface->Format != Format_Y416      &&
        outSurface->Format != Format_VYUY      &&
        outSurface->Format != Format_YVYU      &&
        outSurface->Format != Format_Y8        &&
        outSurface->Format != Format_Y16S      &&
        outSurface->Format != Format_Y16U)
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

MOS_STATUS VphalSfcStateXe_Xpm::SetSfcStateParams(
        PVPHAL_VEBOX_RENDER_DATA    pRenderData,
        PVPHAL_SURFACE              pSrcSurface,
        PVPHAL_SURFACE              pOutSurface)
{
    MOS_STATUS                      eStatus;
    PMHW_SFC_STATE_PARAMS_XE_XPM sfcStateParams;
    int32_t                         i;

    eStatus                = MOS_STATUS_UNKNOWN;
    sfcStateParams         = (PMHW_SFC_STATE_PARAMS_XE_XPM)m_renderData.SfcStateParams;

    eStatus = VphalSfcStateG12::SetSfcStateParams(pRenderData, pSrcSurface, pOutSurface);

    // Dithering parameter
    bool isDitheringNeeded = IsDitheringNeeded(pSrcSurface->Format, pOutSurface->Format);
    if (!m_disableSfcDithering && isDitheringNeeded)
    {
        sfcStateParams->ditheringEn = true;
    }
    else
    {
        sfcStateParams->ditheringEn = false;
    }
    VPHAL_RENDER_NORMALMESSAGE("cscParams.isDitheringNeeded = %d, m_disableSfcDithering = %d, ditheringEn = %d",
        isDitheringNeeded,
        m_disableSfcDithering,
        sfcStateParams->ditheringEn);

    if (pSrcSurface->InterlacedScalingType != ISCALING_NONE)
    {
        sfcStateParams->dwOutputFrameWidth  = sfcStateParams->dwScaledRegionWidth;
        sfcStateParams->dwOutputFrameHeight = sfcStateParams->dwScaledRegionHeight;
    }
    //Interlaced scaling parameter
    switch (pSrcSurface->InterlacedScalingType)
    {
        case ISCALING_INTERLEAVED_TO_INTERLEAVED:
            sfcStateParams->iScalingType = ISCALING_INTERLEAVED_TO_INTERLEAVED;
            sfcStateParams->inputFrameDataFormat = FRAME_FORMAT_INTERLEAVED;
            sfcStateParams->outputFrameDataFormat = FRAME_FORMAT_INTERLEAVED;
            // scaling offset:1/2((1/Vertical scale factor)-1)
            sfcStateParams->bottomFieldVerticalScalingOffset = MOS_UF_ROUND(1.0F / 2.0F * ((pSrcSurface->rcSrc.bottom - pSrcSurface->rcSrc.top) / (pSrcSurface->rcDst.bottom - pSrcSurface->rcDst.top) - 1.0F));
            break;
        case ISCALING_INTERLEAVED_TO_FIELD:
            sfcStateParams->iScalingType          = ISCALING_INTERLEAVED_TO_FIELD;
            sfcStateParams->inputFrameDataFormat  = FRAME_FORMAT_INTERLEAVED;
            sfcStateParams->outputFrameDataFormat = FRAME_FORMAT_FIELD;
            sfcStateParams->tempFieldResource     = &pRenderData->pOutputTempField->OsResource;
            sfcStateParams->outputSampleType      = pOutSurface->SampleType;
            sfcStateParams->dwOutputFrameHeight   = sfcStateParams->dwOutputFrameHeight * 2;
            sfcStateParams->dwScaledRegionHeight  = sfcStateParams->dwScaledRegionHeight * 2;
            break;
        case ISCALING_FIELD_TO_INTERLEAVED:
            sfcStateParams->iScalingType          = ISCALING_FIELD_TO_INTERLEAVED;
            sfcStateParams->inputFrameDataFormat  = FRAME_FORMAT_FIELD;
            sfcStateParams->outputFrameDataFormat = FRAME_FORMAT_INTERLEAVED;
            if (pSrcSurface->SampleType == SAMPLE_SINGLE_TOP_FIELD)
            {
                sfcStateParams->topBottomField = VPHAL_TOP_FIELD;
                if (pOutSurface->SampleType == SAMPLE_INTERLEAVED_EVEN_FIRST_TOP_FIELD)
                {
                    sfcStateParams->topBottomFieldFirst = VPHAL_TOP_FIELD_FIRST;
                }
                else
                {
                    sfcStateParams->topBottomFieldFirst = VPHAL_BOTTOM_FIELD_FIRST;
                }
            }
            else
            {
                sfcStateParams->topBottomField = VPHAL_BOTTOM_FIELD;
                if (pOutSurface->SampleType == SAMPLE_INTERLEAVED_EVEN_FIRST_TOP_FIELD)
                {
                    sfcStateParams->topBottomFieldFirst = VPHAL_TOP_FIELD_FIRST;
                }
                else
                {
                    sfcStateParams->topBottomFieldFirst = VPHAL_BOTTOM_FIELD_FIRST;
                }
            }
            break;
        case ISCALING_FIELD_TO_FIELD:
        case ISCALING_NONE:
        default:
            sfcStateParams->inputFrameDataFormat = FRAME_FORMAT_PROGRESSIVE;
            sfcStateParams->outputFrameDataFormat = FRAME_FORMAT_PROGRESSIVE;
            break;
    }

    for (i = 0; i < MHW_SFC_MAX_PIPE_NUM_XE_XPM; i++)
    {
        sfcStateParams->pOsResAVSLineBufferSplit[i] = &m_AVSLineBufferSurfaceSplit[i].OsResource;
        sfcStateParams->pOsResIEFLineBufferSplit[i] = &m_IEFLineBufferSurfaceSplit[i].OsResource;
    }

    sfcStateParams->InputFrameFormat = m_renderData.SfcInputFormat;

    return eStatus;
}

VPHAL_OUTPUT_PIPE_MODE VphalSfcStateXe_Xpm::GetOutputPipe(
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

    if (pSrc->InterlacedScalingType == ISCALING_INTERLEAVED_TO_INTERLEAVED)
    {
        if (OUT_OF_BOUNDS(dwSurfaceWidth, dwSfcMinWidth, dwSfcMaxWidth)         ||
            OUT_OF_BOUNDS(dwSurfaceHeight, dwSfcMinHeight, dwSfcMaxHeight)      ||
            OUT_OF_BOUNDS(dwSourceRegionWidth, dwSfcMinWidth, dwSfcMaxWidth)    ||
            OUT_OF_BOUNDS(dwSourceRegionHeight, dwSfcMinHeight, dwSfcMaxHeight) ||
            OUT_OF_BOUNDS(dwOutputRegionWidth, dwSfcMinWidth, dwSfcMaxWidth)    ||
            OUT_OF_BOUNDS(dwOutputRegionHeight, dwSfcMinHeight, dwSfcMaxHeight) ||
            OUT_OF_BOUNDS(pRenderTarget->dwWidth, dwSfcMinWidth, dwSfcMaxWidth) ||
            OUT_OF_BOUNDS(pRenderTarget->dwHeight, dwSfcMinHeight, dwSfcMaxHeight))
        {
            VPHAL_RENDER_NORMALMESSAGE("Surface dimensions not supported by SFC Pipe, fallback to kernel");
            OutputPipe = VPHAL_OUTPUT_PIPE_MODE_COMP;
            return OutputPipe;
        }

        if (pSrc->Rotation != VPHAL_ROTATION_IDENTITY)
        {
            VPHAL_RENDER_NORMALMESSAGE("Interlaced scaling cannot support rotate or mirror by SFC pipe, fallback to kernel.");
            OutputPipe = VPHAL_OUTPUT_PIPE_MODE_COMP;
            return OutputPipe;
        }

        if (pSrc->rcSrc.left != 0 ||
            pSrc->rcSrc.top  != 0 ||
            pSrc->rcDst.left != 0 ||
            pSrc->rcDst.top  != 0)
        {
            VPHAL_RENDER_NORMALMESSAGE("Interlaced scaling cannot support offset by SFC pipe, fallback to kernel.");
            OutputPipe = VPHAL_OUTPUT_PIPE_MODE_COMP;
            return OutputPipe;
        }
    }
    else if (pSrc->InterlacedScalingType == ISCALING_FIELD_TO_INTERLEAVED)
    {
        if (OUT_OF_BOUNDS(dwSurfaceWidth, dwSfcMinWidth, dwSfcMaxWidth)         ||
            OUT_OF_BOUNDS(dwSurfaceHeight, dwSfcMinHeight, dwSfcMaxHeight)      ||
            OUT_OF_BOUNDS(dwSourceRegionWidth, dwSfcMinWidth, dwSfcMaxWidth)    ||
            OUT_OF_BOUNDS(dwSourceRegionHeight, dwSfcMinHeight, dwSfcMaxHeight) ||
            OUT_OF_BOUNDS(dwOutputRegionWidth, dwSfcMinWidth, dwSfcMaxWidth)    ||
            OUT_OF_BOUNDS(dwOutputRegionHeight, dwSfcMinHeight, dwSfcMaxHeight) ||
            OUT_OF_BOUNDS(pRenderTarget->dwWidth, dwSfcMinWidth*2, dwSfcMaxWidth) ||
            OUT_OF_BOUNDS(pRenderTarget->dwHeight, dwSfcMinHeight*2, dwSfcMaxHeight))
        {
            if (dwOutputRegionHeight == dwSourceRegionHeight &&
                dwOutputRegionWidth == dwSourceRegionWidth)
            {
                VPHAL_RENDER_NORMALMESSAGE("Surface dimensions not supported by SFC Pipe, fallback to kernel");
                OutputPipe = VPHAL_OUTPUT_PIPE_MODE_COMP;
                return OutputPipe;
            }
            else
            {
                VPHAL_RENDER_NORMALMESSAGE("Surface dimensions not supported.");
                OutputPipe = VPHAL_OUTPUT_PIPE_MODE_INVALID;
                return OutputPipe;
            }
        }

        if (pSrc->Rotation != VPHAL_ROTATION_IDENTITY)
        {
            if (dwOutputRegionHeight == dwSourceRegionHeight &&
                dwOutputRegionWidth == dwSourceRegionWidth)
            {
                VPHAL_RENDER_NORMALMESSAGE("Interlaced scaling cannot support rotate or mirror by SFC pipe, fallback to kernel.");
                OutputPipe = VPHAL_OUTPUT_PIPE_MODE_COMP;
                return OutputPipe;
            }
            else
            {
                VPHAL_RENDER_NORMALMESSAGE("Interlaced scaling cannot support rotate or mirror by SFC pipe.");
                OutputPipe = VPHAL_OUTPUT_PIPE_MODE_INVALID;
                return OutputPipe;
            }
        }

        if (pSrc->rcSrc.left != 0 ||
            pSrc->rcSrc.top  != 0 ||
            pSrc->rcDst.left != 0 ||
            pSrc->rcDst.top  != 0)
        {
            if (dwOutputRegionHeight == dwSourceRegionHeight &&
                dwOutputRegionWidth == dwSourceRegionWidth)
            {
                VPHAL_RENDER_NORMALMESSAGE("Interlaced scaling cannot support offset by SFC pipe, fallback to kernel.");
                OutputPipe = VPHAL_OUTPUT_PIPE_MODE_COMP;
                return OutputPipe;
            }
            else
            {
                VPHAL_RENDER_NORMALMESSAGE("Interlaced scaling cannot support offset by SFC pipe.");
                OutputPipe = VPHAL_OUTPUT_PIPE_MODE_INVALID;
                return OutputPipe;
            }
        }
    }
    else if (pSrc->InterlacedScalingType == ISCALING_INTERLEAVED_TO_FIELD)
    {
        if (OUT_OF_BOUNDS(dwSurfaceWidth, dwSfcMinWidth, dwSfcMaxWidth)         ||
            OUT_OF_BOUNDS(dwSurfaceHeight, dwSfcMinHeight, dwSfcMaxHeight)      ||
            OUT_OF_BOUNDS(dwSourceRegionWidth, dwSfcMinWidth, dwSfcMaxWidth)    ||
            OUT_OF_BOUNDS(dwSourceRegionHeight, dwSfcMinHeight, dwSfcMaxHeight) ||
            OUT_OF_BOUNDS(dwOutputRegionWidth, dwSfcMinWidth/2, dwSfcMaxWidth)    ||
            OUT_OF_BOUNDS(dwOutputRegionHeight, dwSfcMinHeight/2, dwSfcMaxHeight) ||
            OUT_OF_BOUNDS(pRenderTarget->dwWidth, dwSfcMinWidth/2, dwSfcMaxWidth) ||
            OUT_OF_BOUNDS(pRenderTarget->dwHeight, dwSfcMinHeight/2, dwSfcMaxHeight))
        {
            VPHAL_RENDER_NORMALMESSAGE("Surface dimensions not supported by SFC Pipe.");
            OutputPipe = VPHAL_OUTPUT_PIPE_MODE_INVALID;
            return OutputPipe;
        }

        if (pSrc->Rotation != VPHAL_ROTATION_IDENTITY)
        {
            VPHAL_RENDER_NORMALMESSAGE("Interlaced scaling cannot support rotate or mirror by SFC pipe.");
            OutputPipe = VPHAL_OUTPUT_PIPE_MODE_INVALID;
            return OutputPipe;
        }

        if (pSrc->rcSrc.left != 0 ||
            pSrc->rcSrc.top  != 0 ||
            pSrc->rcDst.left != 0 ||
            pSrc->rcDst.top  != 0)
        {
            VPHAL_RENDER_NORMALMESSAGE("Interlaced scaling cannot support offset by SFC pipe.");
            OutputPipe = VPHAL_OUTPUT_PIPE_MODE_INVALID;
            return OutputPipe;
        }
    }
    else
    {
        // SFC i/o width and height should fall into the range of [128, 16K]
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
        if (pSrc->InterlacedScalingType == ISCALING_INTERLEAVED_TO_FIELD)
        {
            fScaleX      = (float)dwOutputRegionWidth  / (float)dwSourceRegionWidth;
            fScaleY      = (float)dwOutputRegionHeight * 2.0F / (float)dwSourceRegionHeight;
        }
        else
        {
            fScaleX      = (float)dwOutputRegionWidth  / (float)dwSourceRegionWidth;
            fScaleY      = (float)dwOutputRegionHeight / (float)dwSourceRegionHeight;
        }
    }
    else
    {
        // VPHAL_ROTATION_90 || VPHAL_ROTATION_270 || VPHAL_ROTATE_90_MIRROR_VERTICAL || VPHAL_ROTATE_90_MIRROR_HORIZONTAL
        fScaleX      = (float)dwOutputRegionHeight / (float)dwSourceRegionWidth;
        fScaleY      = (float)dwOutputRegionWidth  / (float)dwSourceRegionHeight;
    }

    // one pass SFC scaling range is [1/8, 8], two pass cover[1/16, 16](AVS Removal) for both X and Y direction.
    // for 2 pass: first pass do 2X, rest for others.
    if ((fScaleX < 0.0625F)  || (fScaleX > 16.0F) ||
        (fScaleY < 0.0625F)  || (fScaleY > 16.0F))
    {
        VPHAL_RENDER_NORMALMESSAGE("Scaling factor not supported by SFC Pipe.");
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

    bColorFill = (pcRenderParams->pColorFillParams &&
                  (!pcRenderParams->pColorFillParams->bDisableColorfillinSFC) &&
                  (pcRenderParams->pColorFillParams->bOnePixelBiasinSFC ?
                  (!RECT1_CONTAINS_RECT2_ONEPIXELBIAS(pSrc->rcDst, pRenderTarget->rcDst)) :
                  (!RECT1_CONTAINS_RECT2(pSrc->rcDst, pRenderTarget->rcDst)))) ?
                  true : false;

    if (IsOutputCapable(bColorFill, pSrc, pRenderTarget))
    {
        OutputPipe = VPHAL_OUTPUT_PIPE_MODE_SFC;
    }
    else
    {
        OutputPipe = VPHAL_OUTPUT_PIPE_MODE_COMP;
    }

    //2 pass SFC.
    if (OutputPipe == VPHAL_OUTPUT_PIPE_MODE_SFC)
    {
        if (((fScaleX >= 0.0625F) && (fScaleX < 0.125F)) ||
            ((fScaleX > 8.0F) && (fScaleX <= 16.0F)) ||
            ((fScaleY >= 0.0625F) && (fScaleY < 0.125F)) ||
            ((fScaleY > 8.0F) && (fScaleY <= 16.0F)))
        {
            m_bSFC2Pass = true;
        }
        else
        {
            m_bSFC2Pass = false;
        }
    }
finish:
    return OutputPipe;
}

bool VphalSfcStateXe_Xpm::IsDitheringNeeded(MOS_FORMAT formatInput, MOS_FORMAT formatOutput)
{
    uint32_t inputBitDepth = VpHalDDIUtils::GetSurfaceBitDepth(formatInput);
    if (inputBitDepth == 0)
    {
        VPHAL_RENDER_ASSERTMESSAGE("Unknown Input format %d for bit depth, return false", formatInput);
        return false;
    }
    uint32_t outputBitDepth = VpHalDDIUtils::GetSurfaceBitDepth(formatOutput);
    if (outputBitDepth == 0)
    {
        VPHAL_RENDER_ASSERTMESSAGE("Unknown Output format %d for bit depth, return false", formatOutput);
        return false;
    }
    if (inputBitDepth > outputBitDepth)
    {
        VPHAL_RENDER_NORMALMESSAGE("inputFormat = %d, inputBitDepth = %d, outputFormat = %d, outputBitDepth = %d, return true",
            formatInput,
            inputBitDepth,
            formatOutput,
            outputBitDepth);
        return true;
    }
    else
    {
        VPHAL_RENDER_NORMALMESSAGE("inputFormat = %d, inputBitDepth = %d, outputFormat = %d, outputBitDepth = %d, return false",
            formatInput,
            inputBitDepth,
            formatOutput,
            outputBitDepth);
        return false;
    }
}

//!
//! \brief    Set Sfc index used by HW
//! \details  VPHAL set Sfc index used by HW
//! \param    [in] dwSfcIndex;
//!           set which Sfc can be used by HW
//! \param    [in] dwSfcCount;
//!           set Sfc Count
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
MOS_STATUS VphalSfcStateXe_Xpm::SetSfcIndex(
    uint32_t dwSfcIndex,
    uint32_t dwSfcCount)
{
    MOS_STATUS         eStatus = MOS_STATUS_SUCCESS;
    MhwSfcInterfaceXe_Xpm *pSfcInterfaceExt = (MhwSfcInterfaceXe_Xpm *)m_sfcInterface;

    MHW_ASSERT(dwSfcIndex < dwSfcCount);

    pSfcInterfaceExt->SetSfcIndex(dwSfcIndex, dwSfcCount);

    return eStatus;
}

//!
//! \brief    Free resources used by Xe_XPM SFC Pipe
//! \details  Free the AVS and IEF line buffer surfaces for Xe_XPM SFC
//! \return   void
//!
void VphalSfcStateXe_Xpm::FreeResources()
{
    int32_t i;

    VphalSfcStateG12::FreeResources();

    for (i = 0; i < MHW_SFC_MAX_PIPE_NUM_XE_XPM; i++)
    {
        // Free AVS Line Buffer surface for SFC
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &m_AVSLineBufferSurfaceSplit[i].OsResource);

        // Free IEF Line Buffer surface for SFC
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &m_IEFLineBufferSurfaceSplit[i].OsResource);
    }

    return;
}

//!
//! \brief    Allocate Resources for SFC Pipe
//! \details  Allocate the AVS and IEF line buffer surfaces for SFC
//! \return   Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VphalSfcStateXe_Xpm::AllocateResources()
{
    MOS_STATUS              eStatus;
    int32_t                 i;
    uint32_t                dwWidth;
    uint32_t                dwHeight;
    uint32_t                dwSize;
    bool                    bAllocated;
    PMHW_SFC_STATE_PARAMS   pSfcStateParams;
    Mos_MemPool             memTypeSurfVideoMem = MOS_MEMPOOL_VIDEOMEMORY;

    eStatus         = MOS_STATUS_UNKNOWN;
    bAllocated      = false;
    pSfcStateParams = m_renderData.SfcStateParams;

    if (MEDIA_IS_SKU(m_renderHal->pSkuTable, FtrLimitedLMemBar))
    {
        memTypeSurfVideoMem = MOS_MEMPOOL_DEVICEMEMORY;
    }

    VPHAL_RENDER_CHK_STATUS(VphalSfcStateG12::AllocateResources());

    for (i = 0; i < MHW_SFC_MAX_PIPE_NUM_XE_XPM; i++)
    {
        // Allocate AVS Line Buffer surface for split
        dwWidth  = m_AVSLineBufferSurface.dwWidth;
        dwHeight = m_AVSLineBufferSurface.dwHeight;
        dwSize   = dwWidth * dwHeight;

        VPHAL_RENDER_CHK_STATUS(VpHal_ReAllocateSurface(
            m_osInterface,
            &m_AVSLineBufferSurfaceSplit[i],
            "SfcAVSLineBufferSurface",
            Format_Buffer,
            MOS_GFXRES_BUFFER,
            MOS_TILE_LINEAR,
            dwSize,
            1,
            false,
            MOS_MMC_DISABLED,
            &bAllocated,
            MOS_HW_RESOURCE_DEF_MAX,
            MOS_TILE_UNSET_GMM,
            memTypeSurfVideoMem,
            VPP_INTER_RESOURCE_NOTLOCKABLE));

        // Allocate IEF Line Buffer surface for split
        dwWidth  = m_IEFLineBufferSurface.dwWidth;
        dwHeight = m_IEFLineBufferSurface.dwHeight;
        dwSize   = dwWidth * dwHeight;

        VPHAL_RENDER_CHK_STATUS(VpHal_ReAllocateSurface(
            m_osInterface,
            &m_IEFLineBufferSurfaceSplit[i],
            "SfcIEFLineBufferSurface",
            Format_Buffer,
            MOS_GFXRES_BUFFER,
            MOS_TILE_LINEAR,
            dwSize,
            1,
            false,
            MOS_MMC_DISABLED,
            &bAllocated,
            MOS_HW_RESOURCE_DEF_MAX,
            MOS_TILE_UNSET_GMM,
            memTypeSurfVideoMem,
            VPP_INTER_RESOURCE_NOTLOCKABLE));
    }

finish:
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        FreeResources();
    }

    return eStatus;
}

//!
//! \brief    Get width and height align unit of output format
//! \details  XeHP+ sfc need 4 alignmenet for NV12 interlaced scaling
//! \return   Return void
//!
void VphalSfcStateXe_Xpm::GetOutputWidthHeightAlignUnit(
    MOS_FORMAT outputFormat, 
    uint16_t   &widthAlignUnit, 
    uint16_t   &heightAlignUnit,
    bool       isInterlacedScaling)
{
    widthAlignUnit  = 1;
    heightAlignUnit = 1;

    switch (VpHalDDIUtils::GetSurfaceColorPack(outputFormat))
    {
    case VPHAL_COLORPACK_420:
        widthAlignUnit  = 2;
        heightAlignUnit = 2;
        if (isInterlacedScaling)
        {
            heightAlignUnit = 4;
        }
        break;
    case VPHAL_COLORPACK_422:
        widthAlignUnit = 2;
        break;
    default:
        break;
    }
}

void VphalSfcStateXe_Xpm::GetInputWidthHeightAlignUnit(
    MOS_FORMAT              inputFormat,
    MOS_FORMAT              outputFormat,
    uint16_t                &widthAlignUnit,
    uint16_t                &heightAlignUnit,
    bool                    isInterlacedScaling)
{
    MOS_UNUSED(outputFormat);
    widthAlignUnit  = 1;
    heightAlignUnit = 1;

    // Apply alignment restriction to Region of the input frame.
    switch (VpHalDDIUtils::GetSurfaceColorPack(inputFormat))
    {
        case VPHAL_COLORPACK_420:
            widthAlignUnit  = 2;
            heightAlignUnit = 2;
            if (isInterlacedScaling)
            {
                heightAlignUnit = 4;
            }
            break;
        case VPHAL_COLORPACK_422:
            widthAlignUnit  = 2;
            break;
        default:
            break;
    }
}

#endif //__VPHAL_SFC_SUPPORTED

