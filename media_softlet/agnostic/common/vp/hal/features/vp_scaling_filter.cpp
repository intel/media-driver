/*
* Copyright (c) 2018-2024, Intel Corporation
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
//! \file     vp_scaling_fliter.cpp
//! \brief    Defines the common interface for scaling filters
//!           this file is for the base interface which is shared by scaling in driver.
//!

#include "vp_scaling_filter.h"
#include "vp_vebox_cmd_packet.h"
#include "vp_utils.h"
#include "hw_filter.h"
#include "sw_filter_pipe.h"
#include "vp_platform_interface.h"
#include "vp_hal_ddi_utils.h"

using namespace vp;

namespace vp
{
MOS_FORMAT GetSfcInputFormat(VP_EXECUTE_CAPS &executeCaps, MOS_FORMAT inputFormat, VPHAL_CSPACE colorSpaceOutput, MOS_FORMAT outputFormat);
};

VpScalingFilter::VpScalingFilter(
    PVP_MHWINTERFACE vpMhwInterface) :
    VpFilter(vpMhwInterface)
{

}

MOS_STATUS VpScalingFilter::Init()
{
    VP_FUNC_CALL();
    m_bVdbox = false;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpScalingFilter::Init(
    CODECHAL_STANDARD           codecStandard,
    CodecDecodeJpegChromaType   jpegChromaType)
{
    VP_FUNC_CALL();

    m_bVdbox            = true;
    m_codecStandard     = codecStandard;
    m_jpegChromaType    = jpegChromaType;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpScalingFilter::SfcAdjustBoundary(
    uint32_t      *pdwSurfaceWidth,
    uint32_t      *pdwSurfaceHeight)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(m_pvpMhwInterface);
    VP_PUBLIC_CHK_NULL_RETURN(m_pvpMhwInterface->m_vpPlatformInterface);
    VP_PUBLIC_CHK_NULL_RETURN(pdwSurfaceWidth);
    VP_PUBLIC_CHK_NULL_RETURN(pdwSurfaceHeight);

    uint32_t widthAlignUnit  = 0;
    uint32_t heightAlignUnit = 0;

    VP_PUBLIC_CHK_STATUS_RETURN(m_pvpMhwInterface->m_vpPlatformInterface->GetInputFrameWidthHeightAlignUnit(m_pvpMhwInterface, widthAlignUnit, heightAlignUnit,
        m_bVdbox, m_codecStandard, m_jpegChromaType));

    uint32_t dwVeboxHeight = m_scalingParams.input.dwHeight;
    uint32_t dwVeboxWidth  = m_scalingParams.input.dwWidth;
    uint32_t dwVeboxBottom = (uint32_t)m_scalingParams.input.rcMaxSrc.bottom;
    uint32_t dwVeboxRight  = (uint32_t)m_scalingParams.input.rcMaxSrc.right;

    if (m_scalingParams.bDirectionalScalar)
    {
        dwVeboxHeight *= 2;
        dwVeboxWidth *= 2;
        dwVeboxBottom *= 2;
        dwVeboxRight *= 2;
    }
    if (MEDIA_IS_SKU(m_pvpMhwInterface->m_skuTable, FtrHeight8AlignVE3DLUTDualPipe) && (m_executeCaps.bHDR3DLUT || m_executeCaps.bDV))
    {
        VP_PUBLIC_NORMALMESSAGE("SFC Align Frame Height as 8x due to 3Dlut Dual mode Enable");
        heightAlignUnit = MOS_ALIGN_CEIL(heightAlignUnit, 8);
    }

    *pdwSurfaceHeight = MOS_ALIGN_CEIL(
        MOS_MIN(dwVeboxHeight, MOS_MAX(dwVeboxBottom, MHW_VEBOX_MIN_HEIGHT)),
        heightAlignUnit);
    *pdwSurfaceWidth = MOS_ALIGN_CEIL(
        MOS_MIN(dwVeboxWidth, MOS_MAX(dwVeboxRight, MHW_VEBOX_MIN_WIDTH)),
        widthAlignUnit);

    return MOS_STATUS_SUCCESS;
}

template <typename T>
inline void swap(T &a, T &b)
{
    T tmp = b;
    b     = a;
    a     = tmp;
}

void VpScalingFilter::GetFormatWidthHeightAlignUnit(
    MOS_FORMAT format,
    bool bOutput,
    bool bRotateNeeded,
    uint16_t & widthAlignUnit,
    uint16_t & heightAlignUnit,
    bool isInterlacedScaling)
{
    VP_FUNC_CALL();

    widthAlignUnit = 1;
    heightAlignUnit = 1;

    switch (VpHalDDIUtils::GetSurfaceColorPack(format))
    {
    case VPHAL_COLORPACK_420:
        widthAlignUnit = 2;
        heightAlignUnit = 2;
        if (isInterlacedScaling && bOutput)
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
    if (bRotateNeeded && bOutput)
    {
        // Output rect has been rotated in SwFilterScaling::Configure. Need to swap the alignUnit accordingly.
        swap(widthAlignUnit, heightAlignUnit);
    }
}

MOS_STATUS VpScalingFilter::IsColorfillEnable()
{
    VP_FUNC_CALL();

    m_bColorfillEnable = (m_scalingParams.pColorFillParams &&
        (!m_scalingParams.pColorFillParams->bDisableColorfillinSFC) &&
        (m_scalingParams.pColorFillParams->bOnePixelBiasinSFC ?
        (!RECT1_CONTAINS_RECT2_ONEPIXELBIAS(m_scalingParams.input.rcDst, m_scalingParams.output.rcDst)) :
        (!RECT1_CONTAINS_RECT2(m_scalingParams.input.rcDst, m_scalingParams.output.rcDst)))) ?
        true : false;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpScalingFilter::SetColorFillParams()
{
    VP_FUNC_CALL();

    VPHAL_COLOR_SAMPLE_8        Src = {};
    VPHAL_CSPACE                src_cspace, dst_cspace;

    VP_PUBLIC_CHK_NULL_RETURN(m_sfcScalingParams);

    m_sfcScalingParams->sfcColorfillParams.bColorfillEnable = m_bColorfillEnable;

    VP_PUBLIC_NORMALMESSAGE("isColorfillEnable %d", m_bColorfillEnable);

    if (m_bColorfillEnable)
    {   // for fp16 output format, if the colorfill params space is RGB, passed the float value of ARGB channels from DDI to mhw directly, no need convert.
        if (IS_RGB_CSPACE(m_scalingParams.pColorFillParams->CSpace) && (IS_RGB64_FLOAT_FORMAT(m_scalingParams.formatOutput)))
        {
            // Swap the channel here because HW only natively supports XBGR output
            if (m_scalingParams.formatOutput == Format_A16B16G16R16F)
            {
                m_sfcScalingParams->sfcColorfillParams.fColorFillYRPixel = m_scalingParams.pColorFillParams->Color1.B;
                m_sfcScalingParams->sfcColorfillParams.fColorFillUGPixel = m_scalingParams.pColorFillParams->Color1.G;
                m_sfcScalingParams->sfcColorfillParams.fColorFillVBPixel = m_scalingParams.pColorFillParams->Color1.R;
            }
            else
            {
                m_sfcScalingParams->sfcColorfillParams.fColorFillYRPixel = m_scalingParams.pColorFillParams->Color1.R;
                m_sfcScalingParams->sfcColorfillParams.fColorFillUGPixel = m_scalingParams.pColorFillParams->Color1.G;
                m_sfcScalingParams->sfcColorfillParams.fColorFillVBPixel = m_scalingParams.pColorFillParams->Color1.B;
            }
            m_sfcScalingParams->sfcColorfillParams.fColorFillAPixel = m_scalingParams.pColorFillParams->Color1.A;
        }
        else
        {
            VP_PUBLIC_CHK_NULL_RETURN(m_scalingParams.pColorFillParams);
            Src.dwValue = m_scalingParams.pColorFillParams->Color;
            src_cspace  = m_scalingParams.pColorFillParams->CSpace;
            dst_cspace  = m_scalingParams.csc.colorSpaceOutput;

            // Convert BG color only if not done so before. CSC is expensive!
            if ((m_colorFillColorSrc.dwValue != Src.dwValue) ||
                (m_colorFillSrcCspace != src_cspace) ||
                (m_colorFillRTCspace != dst_cspace))
            {
                VP_PUBLIC_NORMALMESSAGE("colorFillColorDst need be recalculated.");
                // Clean history Dst BG Color if hit unsupported format
                if (!VpUtils::GetCscMatrixForRender8Bit(&m_colorFillColorDst, &Src, src_cspace, dst_cspace))
                {
                    VP_PUBLIC_NORMALMESSAGE("VpUtils::GetCscMatrixForRender8Bit failed!");
                    MOS_ZeroMemory(&m_colorFillColorDst, sizeof(m_colorFillColorDst));
                }
                // store the values for next iteration
                m_colorFillColorSrc  = Src;
                m_colorFillSrcCspace = src_cspace;
                m_colorFillRTCspace  = dst_cspace;
            }

            VP_PUBLIC_NORMALMESSAGE("colorFillSrc %x, src_cspace %d, colorFillDst %x, dst_cspace %d", Src.dwValue, src_cspace, m_colorFillColorDst.dwValue, dst_cspace);

            VP_RENDER_CHK_STATUS_RETURN(SetYUVRGBPixel());
            m_sfcScalingParams->sfcColorfillParams.fColorFillAPixel = (float)Src.A / 255.0F;
        }
    }

    if (m_scalingParams.pCompAlpha)
    {
        VP_RENDER_CHK_STATUS_RETURN(SetAlphaPixelParams());
    }
    else
    {
        m_sfcScalingParams->sfcColorfillParams.fAlphaPixel = 1.0F;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpScalingFilter::SetYUVRGBPixel()
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(m_sfcScalingParams);

    if (IS_YUV_FORMAT(m_scalingParams.formatOutput) || IS_ALPHA_YUV_FORMAT(m_scalingParams.formatOutput))
    {
        m_sfcScalingParams->sfcColorfillParams.fColorFillYRPixel = (float)m_colorFillColorDst.Y / 255.0F;
        m_sfcScalingParams->sfcColorfillParams.fColorFillUGPixel = (float)m_colorFillColorDst.U / 255.0F;
        m_sfcScalingParams->sfcColorfillParams.fColorFillVBPixel = (float)m_colorFillColorDst.V / 255.0F;
    }
    else
    {
        // Swap the channel here because HW only natively supports XBGR output
        if ((m_scalingParams.formatOutput == Format_A8R8G8B8)       ||
            (m_scalingParams.formatOutput == Format_X8R8G8B8)       ||
            (m_scalingParams.formatOutput == Format_R10G10B10A2)    ||
            (m_scalingParams.formatOutput == Format_A16B16G16R16)   ||
            (m_scalingParams.formatOutput == Format_A16B16G16R16F))
        {
            m_sfcScalingParams->sfcColorfillParams.fColorFillYRPixel = (float)m_colorFillColorDst.B / 255.0F;
            m_sfcScalingParams->sfcColorfillParams.fColorFillUGPixel = (float)m_colorFillColorDst.G / 255.0F;
            m_sfcScalingParams->sfcColorfillParams.fColorFillVBPixel = (float)m_colorFillColorDst.R / 255.0F;
        }
        else
        {
            m_sfcScalingParams->sfcColorfillParams.fColorFillYRPixel = (float)m_colorFillColorDst.R / 255.0F;
            m_sfcScalingParams->sfcColorfillParams.fColorFillUGPixel = (float)m_colorFillColorDst.G / 255.0F;
            m_sfcScalingParams->sfcColorfillParams.fColorFillVBPixel = (float)m_colorFillColorDst.B / 255.0F;
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpScalingFilter::SetAlphaPixelParams()
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(m_sfcScalingParams);
    VP_PUBLIC_CHK_NULL_RETURN(m_scalingParams.pCompAlpha);

    switch (m_scalingParams.pCompAlpha->AlphaMode)
    {
    case VPHAL_ALPHA_FILL_MODE_NONE:
        if (m_scalingParams.formatOutput == Format_A8R8G8B8 ||
            m_scalingParams.formatOutput == Format_A8B8G8R8 ||
            m_scalingParams.formatOutput == Format_R10G10B10A2 ||
            m_scalingParams.formatOutput == Format_B10G10R10A2 ||
            m_scalingParams.formatOutput == Format_AYUV ||
            m_scalingParams.formatOutput == Format_Y410 ||
            m_scalingParams.formatOutput == Format_Y416)
        {
            m_sfcScalingParams->sfcColorfillParams.fAlphaPixel      = m_scalingParams.pCompAlpha->fAlpha;
            m_sfcScalingParams->sfcColorfillParams.fColorFillAPixel = m_scalingParams.pCompAlpha->fAlpha;
        }
        else
        {
            m_sfcScalingParams->sfcColorfillParams.fAlphaPixel = 1.0F;
        }
        break;

    case VPHAL_ALPHA_FILL_MODE_BACKGROUND:
        m_sfcScalingParams->sfcColorfillParams.fAlphaPixel = m_bColorfillEnable ?
            m_sfcScalingParams->sfcColorfillParams.fColorFillAPixel : 1.0F;
        break;

    case VPHAL_ALPHA_FILL_MODE_SOURCE_STREAM:
    case VPHAL_ALPHA_FILL_MODE_OPAQUE:
    default:
        if (Format_Y416 == m_scalingParams.formatOutput &&
            VPHAL_ALPHA_FILL_MODE_OPAQUE == m_scalingParams.pCompAlpha->AlphaMode)
        {
            // AlphaDefaultValue in SfcState is 10 bits, while alpha channel of Y416 is 16.
            // The high 4 bits alpha of Y416 will be missed.
            VP_PUBLIC_NORMALMESSAGE("The high 4 bits alpha of Y416 will be missed.");
        }
        m_sfcScalingParams->sfcColorfillParams.fAlphaPixel      = 1.0F;
        m_sfcScalingParams->sfcColorfillParams.fColorFillAPixel = 1.0F;
    }
    return MOS_STATUS_SUCCESS;
}


MOS_STATUS VpScalingFilter::SetRectSurfaceAlignment(bool isOutputSurf, uint32_t &width, uint32_t &height, RECT &rcSrc, RECT &rcDst)
{
    VP_FUNC_CALL();

    uint16_t   wWidthAlignUnit            = 0;
    uint16_t   wHeightAlignUnit           = 0;
    uint16_t   wWidthAlignUnitForDstRect  = 0;
    uint16_t   wHeightAlignUnitForDstRect = 0;
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    GetFormatWidthHeightAlignUnit(isOutputSurf ? m_scalingParams.formatOutput : m_scalingParams.formatInput,
        isOutputSurf, m_scalingParams.rotation.rotationNeeded, wWidthAlignUnit, wHeightAlignUnit, false);
    GetFormatWidthHeightAlignUnit(m_scalingParams.formatOutput, true, m_scalingParams.rotation.rotationNeeded, wWidthAlignUnitForDstRect, wHeightAlignUnitForDstRect, false);

    // The source rectangle is floored to the aligned unit to
    // get rid of invalid data(ex: an odd numbered src rectangle with NV12 format
    // will have invalid UV data for the last line of Y data).
    rcSrc.bottom = MOS_ALIGN_FLOOR((uint32_t)rcSrc.bottom, wHeightAlignUnit);
    rcSrc.right  = MOS_ALIGN_FLOOR((uint32_t)rcSrc.right, wWidthAlignUnit);

    rcSrc.top    = MOS_ALIGN_CEIL((uint32_t)rcSrc.top, wHeightAlignUnit);
    rcSrc.left   = MOS_ALIGN_CEIL((uint32_t)rcSrc.left, wWidthAlignUnit);

    // The Destination rectangle is rounded to the upper alignment unit to prevent the loss of
    // data which was present in the source rectangle
    // Current output alignment is based on input format.
    // Better output alignmentg solution should be based on output format.
    rcDst.bottom = MOS_ALIGN_CEIL((uint32_t)rcDst.bottom, wHeightAlignUnitForDstRect);
    rcDst.right  = MOS_ALIGN_CEIL((uint32_t)rcDst.right, wWidthAlignUnitForDstRect);

    rcDst.top    = MOS_ALIGN_FLOOR((uint32_t)rcDst.top, wHeightAlignUnitForDstRect);
    rcDst.left   = MOS_ALIGN_FLOOR((uint32_t)rcDst.left, wWidthAlignUnitForDstRect);

    if (isOutputSurf)
    {
        height = MOS_ALIGN_CEIL(height, wHeightAlignUnit);
        width  = MOS_ALIGN_CEIL(width, wWidthAlignUnit);
    }
    else
    {
        height = MOS_ALIGN_FLOOR(height, wHeightAlignUnit);
        width  = MOS_ALIGN_FLOOR(width, wWidthAlignUnit);
    }

    if ((rcSrc.top  == rcSrc.bottom) ||
        (rcSrc.left == rcSrc.right)  ||
        (rcDst.top  == rcDst.bottom) ||
        (rcDst.left == rcDst.right)  ||
        (width == 0)                      ||
        (height == 0))
    {
        VP_PUBLIC_NORMALMESSAGE("Surface Parameter is invalid.");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
    }

    return eStatus;
}

MOS_STATUS VpScalingFilter::SetTargetRectangle(uint16_t iWidthAlignUnit, uint16_t iHeightAlignUnit, uint16_t oWidthAlignUnit, uint16_t oHeightAlignUnit, float scaleX, float scaleY)
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_NULL_RETURN(m_pvpMhwInterface);
    if (MEDIA_IS_SKU(m_pvpMhwInterface->m_skuTable, FtrSFCTargetRectangle))
    {
        m_sfcScalingParams->bRectangleEnabled = m_scalingParams.bTargetRectangle;
        uint32_t dstTargetLeftAligned  = MOS_ALIGN_FLOOR((uint32_t)m_scalingParams.output.rcDst.left, oWidthAlignUnit);
        uint32_t dstTargetTopAligned   = MOS_ALIGN_FLOOR((uint32_t)m_scalingParams.output.rcDst.top, oHeightAlignUnit);
        uint32_t dstTargetRightAligned = MOS_ALIGN_FLOOR((uint32_t)m_scalingParams.output.rcDst.right, oWidthAlignUnit);
        uint32_t dstTargetDownAligned  = MOS_ALIGN_FLOOR((uint32_t)m_scalingParams.output.rcDst.bottom, oHeightAlignUnit);

        // Source rectangle is not contained with target rectangle
        uint32_t top_shift    = MOS_MAX(m_scalingParams.output.rcDst.top, m_scalingParams.input.rcDst.top) - m_scalingParams.input.rcDst.top;
        uint32_t left_shift   = MOS_MAX(m_scalingParams.output.rcDst.left, m_scalingParams.input.rcDst.left) - m_scalingParams.input.rcDst.left;
        uint32_t bottom_shift = m_scalingParams.input.rcDst.bottom - MOS_MIN(m_scalingParams.output.rcDst.bottom, m_scalingParams.input.rcDst.bottom);
        uint32_t right_shift  = m_scalingParams.input.rcDst.right - MOS_MIN(m_scalingParams.output.rcDst.right, m_scalingParams.input.rcDst.right);

        uint32_t srcTop = 0, srcLeft = 0, srcBottom = 0, srcRight = 0;
        uint32_t dstTop = 0, dstLeft = 0, dstBottom = 0, dstRight = 0;
        uint32_t dstInputLeftAligned = 0, dstInputTopAligned = 0;

        if (top_shift > 0 || left_shift > 0 || bottom_shift > 0 || right_shift > 0)
        {
            srcTop                                  = m_scalingParams.input.rcSrc.top + MOS_UF_ROUND(top_shift / scaleY);
            srcLeft                                 = m_scalingParams.input.rcSrc.left + MOS_UF_ROUND(left_shift / scaleX);
            srcBottom                               = m_scalingParams.input.rcSrc.bottom - MOS_UF_ROUND(bottom_shift / scaleY);
            srcRight                                = m_scalingParams.input.rcSrc.right - MOS_UF_ROUND(right_shift / scaleX);
            m_sfcScalingParams->dwSourceRegionWidth  = MOS_ALIGN_FLOOR(srcRight - srcLeft, iWidthAlignUnit);
            m_sfcScalingParams->dwSourceRegionHeight = MOS_ALIGN_FLOOR(srcBottom - srcTop, iHeightAlignUnit);
            m_sfcScalingParams->dwSourceRegionHorizontalOffset = MOS_ALIGN_FLOOR(srcLeft, iWidthAlignUnit);
            m_sfcScalingParams->dwSourceRegionVerticalOffset   = MOS_ALIGN_FLOOR(srcTop, iHeightAlignUnit);

            dstTop = MOS_MAX(m_scalingParams.output.rcDst.top, m_scalingParams.input.rcDst.top);
            dstLeft = MOS_MAX(m_scalingParams.output.rcDst.left, m_scalingParams.input.rcDst.left);
            dstBottom = MOS_MIN(m_scalingParams.output.rcDst.bottom, m_scalingParams.input.rcDst.bottom);
            dstRight  = MOS_MIN(m_scalingParams.output.rcDst.right, m_scalingParams.input.rcDst.right);

            m_sfcScalingParams->dwScaledRegionWidth  = MOS_ALIGN_FLOOR(dstRight - dstLeft, oWidthAlignUnit);
            m_sfcScalingParams->dwScaledRegionHeight = MOS_ALIGN_FLOOR(dstBottom - dstTop, oHeightAlignUnit);

            dstInputLeftAligned = MOS_ALIGN_FLOOR(dstLeft, oWidthAlignUnit);
            dstInputTopAligned  = MOS_ALIGN_FLOOR(dstTop, oHeightAlignUnit);
        }
        else
        {
            dstInputLeftAligned = MOS_ALIGN_FLOOR((uint32_t)m_scalingParams.input.rcDst.left, oWidthAlignUnit);
            dstInputTopAligned  = MOS_ALIGN_FLOOR((uint32_t)m_scalingParams.input.rcDst.top, oHeightAlignUnit);
        }


        if (m_scalingParams.rotation.rotationNeeded)
        {
            m_sfcScalingParams->dwScaledRegionHorizontalOffset         = dstInputTopAligned;
            m_sfcScalingParams->dwScaledRegionVerticalOffset           = dstInputLeftAligned;

            m_sfcScalingParams->dwTargetRectangleStartHorizontalOffset = dstTargetTopAligned;
            m_sfcScalingParams->dwTargetRectangleStartVerticalOffset   = dstTargetLeftAligned;
            m_sfcScalingParams->dwTargetRectangleEndHorizontalOffset   = dstTargetDownAligned;
            m_sfcScalingParams->dwTargetRectangleEndVerticalOffset     = dstTargetRightAligned;
        }
        else
        {
            m_sfcScalingParams->dwScaledRegionHorizontalOffset         = dstInputLeftAligned;
            m_sfcScalingParams->dwScaledRegionVerticalOffset           = dstInputTopAligned;

            m_sfcScalingParams->dwTargetRectangleStartHorizontalOffset = dstTargetLeftAligned;
            m_sfcScalingParams->dwTargetRectangleStartVerticalOffset   = dstTargetTopAligned;
            m_sfcScalingParams->dwTargetRectangleEndHorizontalOffset   = dstTargetRightAligned;
            m_sfcScalingParams->dwTargetRectangleEndVerticalOffset     = dstTargetDownAligned;
        }

    }
    return MOS_STATUS_SUCCESS;
}
    // Prepare
MOS_STATUS VpScalingFilter::SetExecuteEngineCaps(
    FeatureParamScaling     &scalingParams,
    VP_EXECUTE_CAPS         vpExecuteCaps)
{
    VP_FUNC_CALL();

    m_executeCaps   = vpExecuteCaps;

    m_scalingParams = scalingParams;
    if (!m_bVdbox)
    {
        m_scalingParams.input.rcMaxSrc = m_scalingParams.input.rcSrc;
    }

    // Set Src/Dst Surface Rect
    VP_PUBLIC_CHK_STATUS_RETURN(SetRectSurfaceAlignment(false, m_scalingParams.input.dwWidth,
        m_scalingParams.input.dwHeight, m_scalingParams.input.rcSrc, m_scalingParams.input.rcDst));
    VP_PUBLIC_CHK_STATUS_RETURN(SetRectSurfaceAlignment(true, m_scalingParams.output.dwWidth,
        m_scalingParams.output.dwHeight, m_scalingParams.output.rcSrc, m_scalingParams.output.rcDst));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpScalingFilter::CalculateEngineParams()
{
    VP_FUNC_CALL();

    VP_RENDER_CHK_STATUS_RETURN(IsColorfillEnable());

    if (m_executeCaps.bSFC)
    {
        uint32_t                    dwSurfaceWidth = 0;
        uint32_t                    dwSurfaceHeight = 0;
        uint16_t                    wOutputWidthAlignUnit = 1;
        uint16_t                    wOutputHeightAlignUnit = 1;
        uint16_t                    wInputWidthAlignUnit = 1;
        uint16_t                    wInputHeightAlignUnit = 1;
        uint32_t                    wOutputRegionWidth = 0;
        uint32_t                    wOutputRegionHeight = 0;
        float                       fScaleX = 0.0f;
        float                       fScaleY = 0.0f;

        if (!m_sfcScalingParams)
        {
            m_sfcScalingParams = (SFC_SCALING_PARAMS*)MOS_AllocAndZeroMemory(sizeof(SFC_SCALING_PARAMS));

            if (m_sfcScalingParams == nullptr)
            {
                VP_PUBLIC_ASSERTMESSAGE("sfc Scaling Pamas buffer allocate failed, return nullpointer");
                return MOS_STATUS_NO_SPACE;
            }
        }
        else
        {
            MOS_ZeroMemory(m_sfcScalingParams, sizeof(SFC_SCALING_PARAMS));
        }

        // Set Scaling Mode
        m_sfcScalingParams->bBilinearScaling = (VPHAL_SCALING_BILINEAR == m_scalingParams.scalingMode);

        //Set input/Output boundary
        VP_RENDER_CHK_STATUS_RETURN(SfcAdjustBoundary(
            &dwSurfaceWidth,
            &dwSurfaceHeight));

        m_scalingParams.formatInput             = GetSfcInputFormat(m_executeCaps,
                                                                    m_scalingParams.formatInput,
                                                                    m_scalingParams.csc.colorSpaceOutput,
                                                                    m_scalingParams.formatOutput);
        m_sfcScalingParams->inputFrameFormat    = m_scalingParams.formatInput;
        m_sfcScalingParams->dwInputFrameHeight  = dwSurfaceHeight;
        m_sfcScalingParams->dwInputFrameWidth   = dwSurfaceWidth;

        // Apply alignment restriction to the Region of the output frame.
        GetFormatWidthHeightAlignUnit(
            m_scalingParams.formatOutput,
            true,
            m_scalingParams.rotation.rotationNeeded,
            wOutputWidthAlignUnit,
            wOutputHeightAlignUnit,
            m_scalingParams.interlacedScalingType == ISCALING_INTERLEAVED_TO_INTERLEAVED);

        // Apply alignment restriction to Region of the input frame.
        GetFormatWidthHeightAlignUnit(
            m_sfcScalingParams->inputFrameFormat,
            false,
            m_scalingParams.rotation.rotationNeeded,
            wInputWidthAlignUnit,
            wInputHeightAlignUnit,
            m_scalingParams.interlacedScalingType == ISCALING_INTERLEAVED_TO_INTERLEAVED);

        m_sfcScalingParams->dwOutputFrameHeight = MOS_ALIGN_CEIL(m_scalingParams.output.dwHeight, wOutputHeightAlignUnit);
        m_sfcScalingParams->dwOutputFrameWidth  = MOS_ALIGN_CEIL(m_scalingParams.output.dwWidth, wOutputWidthAlignUnit);

        //Set source input offset in Horizontal/vertical
        m_sfcScalingParams->dwSourceRegionHorizontalOffset = MOS_ALIGN_CEIL((uint32_t)m_scalingParams.input.rcSrc.left, wInputWidthAlignUnit);
        m_sfcScalingParams->dwSourceRegionVerticalOffset   = MOS_ALIGN_CEIL((uint32_t)m_scalingParams.input.rcSrc.top, wInputHeightAlignUnit);

        // Exclude padding area of the SFC input
        m_sfcScalingParams->dwSourceRegionHeight           = MOS_ALIGN_FLOOR(
            MOS_MIN((uint32_t)(m_scalingParams.input.rcSrc.bottom - m_scalingParams.input.rcSrc.top), m_sfcScalingParams->dwInputFrameHeight),
            wInputHeightAlignUnit);
        m_sfcScalingParams->dwSourceRegionWidth            = MOS_ALIGN_FLOOR(
            MOS_MIN((uint32_t)(m_scalingParams.input.rcSrc.right - m_scalingParams.input.rcSrc.left), m_sfcScalingParams->dwInputFrameWidth),
            wInputWidthAlignUnit);

        // Size of the Output Region over the Render Target
        wOutputRegionHeight = MOS_ALIGN_CEIL(
            MOS_MIN((uint32_t)(m_scalingParams.input.rcDst.bottom - m_scalingParams.input.rcDst.top), m_scalingParams.output.dwHeight),
            wOutputHeightAlignUnit);
        wOutputRegionWidth = MOS_ALIGN_CEIL(
            MOS_MIN((uint32_t)(m_scalingParams.input.rcDst.right - m_scalingParams.input.rcDst.left), m_scalingParams.output.dwWidth),
            wOutputWidthAlignUnit);

        fScaleX = (float)wOutputRegionWidth / (float)m_sfcScalingParams->dwSourceRegionWidth;
        fScaleY = (float)wOutputRegionHeight / (float)m_sfcScalingParams->dwSourceRegionHeight;

        if (m_bVdbox)
        {
            // In VD-to-SFC modes, source region must be programmed to same value as Input Frame Resolution.
            // SourceRegion should be updated after fScale being calculated, or scaling ratio may be incorrect.
            m_sfcScalingParams->dwSourceRegionHeight    = m_sfcScalingParams->dwInputFrameHeight;
            m_sfcScalingParams->dwSourceRegionWidth     = m_sfcScalingParams->dwInputFrameWidth;
        }

        // Size of the Scaled Region over the Render Target
        m_sfcScalingParams->dwScaledRegionHeight           = MOS_ALIGN_CEIL(MOS_UF_ROUND(fScaleY * m_sfcScalingParams->dwSourceRegionHeight), wOutputHeightAlignUnit);
        m_sfcScalingParams->dwScaledRegionWidth            = MOS_ALIGN_CEIL(MOS_UF_ROUND(fScaleX * m_sfcScalingParams->dwSourceRegionWidth), wOutputWidthAlignUnit);

        m_sfcScalingParams->dwScaledRegionHeight = MOS_MIN(m_sfcScalingParams->dwScaledRegionHeight, m_sfcScalingParams->dwOutputFrameHeight);
        m_sfcScalingParams->dwScaledRegionWidth  = MOS_MIN(m_sfcScalingParams->dwScaledRegionWidth, m_sfcScalingParams->dwOutputFrameWidth);

        if (m_bVdbox)
        {
            // In VD-to-SFC modes, scaled region should be programmed to same value as Output Frame Resolution.
            // Output Frame Resolution should be updated after scaled region being calculated, or scaling ratio may be incorrect.
            m_sfcScalingParams->dwOutputFrameHeight = m_sfcScalingParams->dwScaledRegionHeight;
            m_sfcScalingParams->dwOutputFrameWidth  = m_sfcScalingParams->dwScaledRegionWidth;
        }

        uint32_t dstInputLeftAligned = MOS_ALIGN_FLOOR((uint32_t)m_scalingParams.input.rcDst.left, wOutputWidthAlignUnit);
        uint32_t dstInputTopAligned = MOS_ALIGN_FLOOR((uint32_t)m_scalingParams.input.rcDst.top, wOutputHeightAlignUnit);

        if (m_scalingParams.rotation.rotationNeeded)
        {
            m_sfcScalingParams->dwScaledRegionHorizontalOffset = dstInputTopAligned;
            m_sfcScalingParams->dwScaledRegionVerticalOffset   = dstInputLeftAligned;
        }
        else
        {
            m_sfcScalingParams->dwScaledRegionHorizontalOffset = dstInputLeftAligned;
            m_sfcScalingParams->dwScaledRegionVerticalOffset   = dstInputTopAligned;
        }

        if (m_scalingParams.bTargetRectangle)
        {
            VP_RENDER_CHK_STATUS_RETURN(SetTargetRectangle(wInputWidthAlignUnit, wOutputHeightAlignUnit, wOutputWidthAlignUnit, wOutputHeightAlignUnit, fScaleX, fScaleY));
        }

        // Refine the Scaling ratios in the X and Y direction. SFC output Scaled size may be changed based on the restriction of SFC alignment.
        // The scaling ratio could be changed and not equal to the fScaleX/Y.
        // Driver must make sure that the scaling ratio should be matched with the output/input size before send to HW
        m_sfcScalingParams->fAVSXScalingRatio = (float)m_sfcScalingParams->dwScaledRegionWidth / (float)m_sfcScalingParams->dwSourceRegionWidth;
        m_sfcScalingParams->fAVSYScalingRatio = (float)m_sfcScalingParams->dwScaledRegionHeight / (float)m_sfcScalingParams->dwSourceRegionHeight;

        m_sfcScalingParams->sfcScalingMode = m_scalingParams.scalingMode;

        m_sfcScalingParams->interlacedScalingType = m_scalingParams.interlacedScalingType;
        m_sfcScalingParams->srcSampleType         = m_scalingParams.input.sampleType;
        m_sfcScalingParams->dstSampleType         = m_scalingParams.output.sampleType;
        m_sfcScalingParams->isDemosaicNeeded      = m_executeCaps.bDemosaicInUse;

        VP_RENDER_CHK_STATUS_RETURN(SetColorFillParams());

        m_sfcScalingParams->b1stPassOfSfc2PassScaling = m_executeCaps.b1stPassOfSfc2PassScaling;
    }
    // Need add support for Render engine
    else
    {

    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpScalingFilter::Prepare()
{
    VP_FUNC_CALL();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpScalingFilter::Destroy()
{
    VP_FUNC_CALL();

    if (m_sfcScalingParams)
    {
        MOS_FreeMemory(m_sfcScalingParams);
        m_sfcScalingParams = nullptr;
    }

    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                                  HwFilter Scaling Parameter                                      */
/****************************************************************************************************/
 HwFilterParameter *HwFilterScalingParameter::Create(HW_FILTER_SCALING_PARAM &param, FeatureType featureType)
{
    VP_FUNC_CALL();

    HwFilterScalingParameter *p = MOS_New(HwFilterScalingParameter, featureType);
    if (p)
    {
        if (MOS_FAILED(p->Initialize(param)))
        {
            MOS_Delete(p);
            return nullptr;
        }
    }
    return p;
}

HwFilterScalingParameter::HwFilterScalingParameter(FeatureType featureType) : HwFilterParameter(featureType)
{
}

HwFilterScalingParameter::~HwFilterScalingParameter()
{
}

MOS_STATUS HwFilterScalingParameter::ConfigParams(HwFilter &hwFilter)
{
    VP_FUNC_CALL();

    return hwFilter.ConfigParam(m_Params);
}

MOS_STATUS HwFilterScalingParameter::Initialize(HW_FILTER_SCALING_PARAM &param)
{
    VP_FUNC_CALL();

    m_Params = param;
    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                                   Packet Sfc Scaling Parameter                                   */
/****************************************************************************************************/
VpPacketParameter *VpSfcScalingParameter::Create(HW_FILTER_SCALING_PARAM &param)
{
    VP_FUNC_CALL();

    if (nullptr == param.pPacketParamFactory)
    {
        return nullptr;
    }
    VpSfcScalingParameter *p = dynamic_cast<VpSfcScalingParameter *>(param.pPacketParamFactory->GetPacketParameter(param.pHwInterface));
    if (p)
    {
        if (MOS_FAILED(p->Initialize(param)))
        {
            VpPacketParameter *pParam = p;
            param.pPacketParamFactory->ReturnPacketParameter(pParam);
            return nullptr;
        }
    }
    return p;
}

VpSfcScalingParameter::VpSfcScalingParameter(PVP_MHWINTERFACE pHwInterface, PacketParamFactoryBase *packetParamFactory) :
    VpPacketParameter(packetParamFactory), m_ScalingFilter(pHwInterface)
{
}

VpSfcScalingParameter::~VpSfcScalingParameter()
{
}

bool VpSfcScalingParameter::SetPacketParam(VpCmdPacket *pPacket)
{
    VP_FUNC_CALL();

    SFC_SCALING_PARAMS *params = m_ScalingFilter.GetSfcParams();
    if (nullptr == params)
    {
        VP_PUBLIC_ASSERTMESSAGE("Failed to get sfc scaling params");
        return false;
    }

    VpVeboxCmdPacketBase *packet = dynamic_cast<VpVeboxCmdPacketBase *>(pPacket);
    if (packet)
    {
        return MOS_SUCCEEDED(packet->SetScalingParams(params));
    }

    VP_PUBLIC_ASSERTMESSAGE("Invalid packet for sfc scaling");
    return false;
}

MOS_STATUS VpSfcScalingParameter::Initialize(HW_FILTER_SCALING_PARAM &params)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_STATUS_RETURN(m_ScalingFilter.Init());
    VP_PUBLIC_CHK_STATUS_RETURN(m_ScalingFilter.SetExecuteEngineCaps(params.scalingParams, params.vpExecuteCaps));
    VP_PUBLIC_CHK_STATUS_RETURN(m_ScalingFilter.CalculateEngineParams());
    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                                   Policy Sfc Scaling Handler                                     */
/****************************************************************************************************/
PolicySfcScalingHandler::PolicySfcScalingHandler(VP_HW_CAPS &hwCaps) : PolicyFeatureHandler(hwCaps)
{
    m_Type = FeatureTypeScalingOnSfc;
}
PolicySfcScalingHandler::~PolicySfcScalingHandler()
{}

bool PolicySfcScalingHandler::IsFeatureEnabled(VP_EXECUTE_CAPS vpExecuteCaps)
{
    VP_FUNC_CALL();

    return vpExecuteCaps.bSfcScaling;
}

HwFilterParameter *PolicySfcScalingHandler::CreateHwFilterParam(VP_EXECUTE_CAPS vpExecuteCaps, SwFilterPipe &swFilterPipe, PVP_MHWINTERFACE pHwInterface)
{
    VP_FUNC_CALL();

    if (IsFeatureEnabled(vpExecuteCaps))
    {
        if (SwFilterPipeType1To1 != swFilterPipe.GetSwFilterPipeType())
        {
            VP_PUBLIC_ASSERTMESSAGE("Invalid parameter! Sfc only support 1To1 swFilterPipe!");
            return nullptr;
        }

        SwFilterScaling *swFilter = dynamic_cast<SwFilterScaling *>(swFilterPipe.GetSwFilter(true, 0, FeatureTypeScalingOnSfc));

        if (nullptr == swFilter)
        {
            VP_PUBLIC_ASSERTMESSAGE("Invalid parameter! Feature enabled in vpExecuteCaps but no swFilter exists!");
            return nullptr;
        }

        FeatureParamScaling &param = swFilter->GetSwFilterParams();

        HW_FILTER_SCALING_PARAM paramScaling = {};
        paramScaling.type = m_Type;
        paramScaling.pHwInterface = pHwInterface;
        paramScaling.vpExecuteCaps = vpExecuteCaps;
        paramScaling.pPacketParamFactory = &m_PacketParamFactory;
        paramScaling.scalingParams = param;
        paramScaling.pfnCreatePacketParam = PolicySfcScalingHandler::CreatePacketParam;

        HwFilterParameter *pHwFilterParam = GetHwFeatureParameterFromPool();

        if (pHwFilterParam)
        {
            if (MOS_FAILED(((HwFilterScalingParameter*)pHwFilterParam)->Initialize(paramScaling)))
            {
                ReleaseHwFeatureParameter(pHwFilterParam);
            }
        }
        else
        {
            pHwFilterParam = HwFilterScalingParameter::Create(paramScaling, m_Type);
        }

        return pHwFilterParam;
    }
    else
    {
        return nullptr;
    }
}

uint32_t PolicySfcScalingHandler::Get1stPassScaledSize(uint32_t input, uint32_t output, bool is2PassNeeded, uint32_t alignUnit)
{
    VP_FUNC_CALL();

    if (!is2PassNeeded)
    {
        bool scalingIn1stPass = input >= output ?
            m_hwCaps.m_rules.sfcMultiPassSupport.scaling.downScaling.scalingIn1stPassIf1PassEnough :
            m_hwCaps.m_rules.sfcMultiPassSupport.scaling.upScaling.scalingIn1stPassIf1PassEnough;
        return scalingIn1stPass ? output : input;
    }

    float       ratioFor1stPass = 0;
    uint32_t    scaledSize      = 0;

    // make sure the scaled Width/Height was aligned in sfc2pass case
    if (input >= output)
    {
        ratioFor1stPass = m_hwCaps.m_rules.sfcMultiPassSupport.scaling.downScaling.ratioFor1stPass;
        scaledSize      = MOS_ALIGN_FLOOR(MOS_MAX(output, (uint32_t)(input * ratioFor1stPass)), alignUnit);
    }
    else
    {
        ratioFor1stPass = m_hwCaps.m_rules.sfcMultiPassSupport.scaling.upScaling.ratioFor1stPass;
        scaledSize      = MOS_ALIGN_CEIL(MOS_MIN(output, (uint32_t)(input * ratioFor1stPass)), alignUnit);
    }

    return scaledSize;
}

MOS_STATUS PolicySfcScalingHandler::UpdateFeaturePipe(VP_EXECUTE_CAPS caps, SwFilter &feature, SwFilterPipe &featurePipe, SwFilterPipe &executePipe, bool isInputPipe, int index)
{
    VP_FUNC_CALL();

    SwFilterScaling *featureScaling = dynamic_cast<SwFilterScaling *>(&feature);
    VP_PUBLIC_CHK_NULL_RETURN(featureScaling);

    if (caps.b1stPassOfSfc2PassScaling)
    {
        uint32_t         widthAlignUnit  = 0;
        uint32_t         heightAlignUnit = 0;
        PVP_MHWINTERFACE hwInterface     = featureScaling->GetHwInterface();
        VP_PUBLIC_CHK_NULL_RETURN(hwInterface);
        VP_PUBLIC_CHK_NULL_RETURN(hwInterface->m_vpPlatformInterface);
        hwInterface->m_vpPlatformInterface->GetInputFrameWidthHeightAlignUnit(hwInterface, widthAlignUnit, heightAlignUnit, false, CODECHAL_STANDARD_MAX, jpegYUV400);

        SwFilterScaling *filter2ndPass = featureScaling;
        SwFilterScaling *filter1ndPass = (SwFilterScaling *)feature.Clone();

        VP_PUBLIC_CHK_NULL_RETURN(filter1ndPass);
        VP_PUBLIC_CHK_NULL_RETURN(filter2ndPass);

        filter1ndPass->GetFilterEngineCaps() = filter2ndPass->GetFilterEngineCaps();
        filter1ndPass->SetFeatureType(filter2ndPass->GetFeatureType());

        FeatureParamScaling &params2ndPass = filter2ndPass->GetSwFilterParams();
        FeatureParamScaling &params1stPass = filter1ndPass->GetSwFilterParams();

        uint32_t inputWidth = params1stPass.input.rcSrc.right - params1stPass.input.rcSrc.left;
        uint32_t inputHeight = params1stPass.input.rcSrc.bottom - params1stPass.input.rcSrc.top;
        uint32_t outputWidth = params1stPass.input.rcDst.right - params1stPass.input.rcDst.left;
        uint32_t outputHeight = params1stPass.input.rcDst.bottom - params1stPass.input.rcDst.top;

        uint32_t scaledWidth  = Get1stPassScaledSize(inputWidth, outputWidth, filter1ndPass->GetFilterEngineCaps().sfc2PassScalingNeededX, widthAlignUnit);
        uint32_t scaledHeight = Get1stPassScaledSize(inputHeight, outputHeight, filter1ndPass->GetFilterEngineCaps().sfc2PassScalingNeededY, heightAlignUnit);

        VP_PUBLIC_NORMALMESSAGE("2 pass sfc scaling setting: (%dx%d)->(%dx%d)->(%dx%d)",
            inputWidth, inputHeight, scaledWidth, scaledHeight, outputWidth, outputHeight);

        params1stPass.input.rcDst.left = 0;
        params1stPass.input.rcDst.right = scaledWidth;
        params1stPass.input.rcDst.top = 0;
        params1stPass.input.rcDst.bottom = scaledHeight;

        params1stPass.output.dwWidth = scaledWidth;
        params1stPass.output.dwHeight = scaledHeight;
        params1stPass.output.rcSrc = params1stPass.input.rcDst;
        params1stPass.output.rcDst = params1stPass.input.rcDst;
        params1stPass.output.rcMaxSrc = params1stPass.output.rcSrc;

        params2ndPass.input.dwWidth = params1stPass.output.dwWidth;
        params2ndPass.input.dwHeight = params1stPass.output.dwHeight;
        params2ndPass.input.rcSrc = params1stPass.input.rcDst;
        params2ndPass.input.rcMaxSrc = params2ndPass.input.rcSrc;

        if (params2ndPass.interlacedScalingType == ISCALING_INTERLEAVED_TO_FIELD)
        {
            params2ndPass.input.dwHeight = params2ndPass.input.dwHeight / 2;
            params2ndPass.input.rcSrc.bottom = params2ndPass.input.rcSrc.bottom / 2;
            params2ndPass.input.rcMaxSrc.bottom = params2ndPass.input.rcMaxSrc.bottom / 2;
        }

        // Set engine caps for filter in 2nd pass.
        filter2ndPass->SetFeatureType(FeatureTypeScaling);
        filter2ndPass->GetFilterEngineCaps().value = 0;
        filter2ndPass->GetFilterEngineCaps().bEnabled = 1;
        filter2ndPass->GetFilterEngineCaps().SfcNeeded = 1;
        filter2ndPass->GetFilterEngineCaps().usedForNextPass = 1;

        executePipe.AddSwFilterUnordered(filter1ndPass, isInputPipe, index);
    }
    else
    {
        if (caps.bOutputPipeFeatureInuse)
        {
            return PolicyFeatureHandler::UpdateFeaturePipe(caps, feature, featurePipe, executePipe, isInputPipe, index);
        }
        else
        {
            SwFilterScaling *filter2ndPass = featureScaling;
            SwFilterScaling *filter1ndPass = (SwFilterScaling *)feature.Clone();
            VP_PUBLIC_CHK_NULL_RETURN(filter1ndPass);

            filter1ndPass->GetFilterEngineCaps().value = 0;
            filter1ndPass->SetFeatureType(FeatureType::FeatureTypeScaling);

            FeatureParamScaling &params2ndPass = filter2ndPass->GetSwFilterParams();
            FeatureParamScaling &params1stPass = filter1ndPass->GetSwFilterParams();

            uint32_t inputWidth = params1stPass.input.rcSrc.right - params1stPass.input.rcSrc.left;
            uint32_t inputHeight = params1stPass.input.rcSrc.bottom - params1stPass.input.rcSrc.top;
            uint32_t outputWidth = params1stPass.input.rcDst.right - params1stPass.input.rcDst.left;
            uint32_t outputHeight = params1stPass.input.rcDst.bottom - params1stPass.input.rcDst.top;

            VP_PUBLIC_NORMALMESSAGE("sfc scaling w/o rectangle info on target surface: (%dx%d)->(%dx%d)",
                inputWidth, inputHeight, outputWidth, outputHeight);

            params1stPass.input.rcDst.left = 0;
            params1stPass.input.rcDst.right = outputWidth;
            params1stPass.input.rcDst.top = 0;
            params1stPass.input.rcDst.bottom = outputHeight;

            params1stPass.output.dwWidth = outputWidth;
            params1stPass.output.dwHeight = outputHeight;
            params1stPass.output.rcSrc = params1stPass.input.rcDst;
            params1stPass.output.rcDst = params1stPass.input.rcDst;
            params1stPass.output.rcMaxSrc = params1stPass.output.rcSrc;

            params2ndPass.input.dwWidth = params1stPass.output.dwWidth;
            params2ndPass.input.dwHeight = params1stPass.output.dwHeight;
            params2ndPass.input.rcSrc = params1stPass.input.rcDst;
            params2ndPass.input.rcMaxSrc = params2ndPass.input.rcSrc;

            // Set engine caps for filter in 2nd pass.
            filter2ndPass->SetFeatureType(FeatureTypeScaling);
            filter2ndPass->GetFilterEngineCaps().value = 0;
            filter2ndPass->GetFilterEngineCaps().bEnabled = 1;
            filter2ndPass->GetFilterEngineCaps().SfcNeeded = 1;
            filter2ndPass->GetFilterEngineCaps().RenderNeeded = 1;
            filter2ndPass->GetFilterEngineCaps().fcSupported = 1;
            filter2ndPass->GetFilterEngineCaps().usedForNextPass = 1;

            executePipe.AddSwFilterUnordered(filter1ndPass, isInputPipe, index);

        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS PolicySfcScalingHandler::UpdateUnusedFeature(VP_EXECUTE_CAPS caps, SwFilter &feature, SwFilterPipe &featurePipe, SwFilterPipe &executePipe, bool isInputPipe, int index)
{
    // feature.GetFilterEngineCaps().bEnabled should be used here instead of feature.IsFeatureEnabled(caps)
    // to ensure the feature does not be enabled.
    // feature.IsFeatureEnabled(caps) being false means the feature is not being used in current workload,
    // in which case, the feature itself may be enable and need be processed in following workloads.
    if (caps.bVebox && !caps.bSFC &&
        0 == caps.bOutputPipeFeatureInuse &&
        !feature.GetFilterEngineCaps().bEnabled)
    {
        // feature.GetFilterEngineCaps().usedForNextPass should be used here to
        // avoid scaling filter being destroyed in Policy::UpdateFeaturePipe.
        feature.GetFilterEngineCaps().usedForNextPass = 1;
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS PolicySfcColorFillHandler::UpdateFeaturePipe(VP_EXECUTE_CAPS caps, SwFilter &feature, SwFilterPipe &featurePipe, SwFilterPipe &executePipe, bool isInputPipe, int index)
{
    VP_FUNC_CALL();

    if (caps.bSFC && caps.bSfcScaling)
    {
        if (true == isInputPipe)
        {
            VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
        }

        SwFilterScaling *scaling = dynamic_cast<SwFilterScaling *>(executePipe.GetSwFilter(true, 0, FeatureTypeScaling));
        SwFilterColorFill *colorfill = dynamic_cast<SwFilterColorFill *>(&feature);

        if (colorfill)
        {
            if (scaling)
            {
                scaling->GetSwFilterParams().pColorFillParams = colorfill->GetSwFilterParams().colorFillParams;
            }
            bool removeFeatureFromFeaturePipe = featurePipe.IsAllInputPipeSurfaceFeatureEmpty();
            if (removeFeatureFromFeaturePipe)
            {
                // Will be removed and destroyed in Policy::UpdateFeaturePipe.
                colorfill->GetFilterEngineCaps().bEnabled = false;
            }
            else
            {
                colorfill->ResetFeatureType();
            }
            return MOS_STATUS_SUCCESS;
        }
    }

    return PolicyFeatureHandler::UpdateFeaturePipe(caps, feature, featurePipe, executePipe, isInputPipe, index);
}

MOS_STATUS PolicySfcColorFillHandler::UpdateUnusedFeature(VP_EXECUTE_CAPS caps, SwFilter &feature, SwFilterPipe &featurePipe, SwFilterPipe &executePipe, bool isInputPipe, int index)
{
    VP_FUNC_CALL();

    if (true == isInputPipe)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    SwFilterColorFill *colorfill = dynamic_cast<SwFilterColorFill *>(&feature);
    if (colorfill)
    {
        bool removeFeatureFromFeaturePipe = featurePipe.IsAllInputPipeSurfaceFeatureEmpty();
        if (removeFeatureFromFeaturePipe)
        {
            // Will be removed and destroyed in Policy::UpdateFeaturePipe.
            colorfill->GetFilterEngineCaps().bEnabled = false;
        }
        else
        {
            colorfill->ResetFeatureType();
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS PolicySfcAlphaHandler::UpdateFeaturePipe(VP_EXECUTE_CAPS caps, SwFilter &feature, SwFilterPipe &featurePipe, SwFilterPipe &executePipe, bool isInputPipe, int index)
{
    VP_FUNC_CALL();

    if (caps.bSFC && caps.bSfcScaling || !caps.bSFC && caps.bVebox)
    {
        if (true == isInputPipe)
        {
            VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
        }

        SwFilterScaling *scaling = dynamic_cast<SwFilterScaling *>(executePipe.GetSwFilter(true, 0, FeatureTypeScaling));
        SwFilterCsc *csc = dynamic_cast<SwFilterCsc *>(executePipe.GetSwFilter(true, 0, FeatureTypeCsc));
        SwFilterAlpha *alpha = dynamic_cast<SwFilterAlpha *>(&feature);

        if (alpha)
        {
            if (scaling)
            {
                scaling->GetSwFilterParams().pCompAlpha = alpha->GetSwFilterParams().compAlpha;
            }
            if (csc)
            {
                csc->GetSwFilterParams().pAlphaParams = alpha->GetSwFilterParams().compAlpha;
            }
            bool removeFeatureFromFeaturePipe = featurePipe.IsAllInputPipeSurfaceFeatureEmpty();
            if (removeFeatureFromFeaturePipe)
            {
                // Will be removed and destroyed in Policy::UpdateFeaturePipe.
                alpha->GetFilterEngineCaps().bEnabled = false;
            }
            else
            {
                alpha->ResetFeatureType();
            }
            return MOS_STATUS_SUCCESS;
        }
    }

    return PolicyFeatureHandler::UpdateFeaturePipe(caps, feature, featurePipe, executePipe, isInputPipe, index);
}

MOS_STATUS PolicySfcAlphaHandler::UpdateUnusedFeature(VP_EXECUTE_CAPS caps, SwFilter &feature, SwFilterPipe &featurePipe, SwFilterPipe &executePipe, bool isInputPipe, int index)
{
    VP_FUNC_CALL();

    if (true == isInputPipe)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    SwFilterAlpha *alpha = dynamic_cast<SwFilterAlpha *>(&feature);
    if (alpha)
    {
        bool removeFeatureFromFeaturePipe = featurePipe.IsAllInputPipeSurfaceFeatureEmpty();
        if (removeFeatureFromFeaturePipe)
        {
            // Will be removed and destroyed in Policy::UpdateFeaturePipe.
            alpha->GetFilterEngineCaps().bEnabled = false;
        }
        else
        {
            alpha->ResetFeatureType();
        }
    }

    return MOS_STATUS_SUCCESS;
}
