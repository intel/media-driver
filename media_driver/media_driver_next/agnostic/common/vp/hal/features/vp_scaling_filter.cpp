/*
* Copyright (c) 2018-2020, Intel Corporation
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

using namespace vp;
VpScalingFilter::VpScalingFilter(
    PVP_MHWINTERFACE vpMhwInterface) :
    VpFilter(vpMhwInterface)
{

}

MOS_STATUS VpScalingFilter::Init()
{
    VP_FUNC_CALL();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpScalingFilter::SfcAdjustBoundary(
    uint32_t      *pdwSurfaceWidth,
    uint32_t      *pdwSurfaceHeight)
{
    VP_FUNC_CALL();

    uint32_t   dwVeboxHeight;
    uint32_t   dwVeboxWidth;
    uint32_t   dwVeboxBottom;
    uint32_t   dwVeboxRight;

    VP_PUBLIC_CHK_NULL_RETURN(m_pvpMhwInterface);
    VP_PUBLIC_CHK_NULL_RETURN(m_pvpMhwInterface->m_sfcInterface);
    VP_PUBLIC_CHK_NULL_RETURN(pdwSurfaceWidth);
    VP_PUBLIC_CHK_NULL_RETURN(pdwSurfaceHeight);

    // For the VEBOX output to SFC, the width is multiple of 16 and height
    // is multiple of 4
    dwVeboxHeight = m_scalingParams.dwHeightInput;
    dwVeboxWidth  = m_scalingParams.dwWidthInput;
    dwVeboxBottom = (uint32_t)m_scalingParams.rcMaxSrcInput.bottom;
    dwVeboxRight  = (uint32_t)m_scalingParams.rcMaxSrcInput.right;

    if (m_scalingParams.bDirectionalScalar)
    {
        dwVeboxHeight *= 2;
        dwVeboxWidth *= 2;
        dwVeboxBottom *= 2;
        dwVeboxRight *= 2;
    }

    *pdwSurfaceHeight = MOS_ALIGN_CEIL(
        MOS_MIN(dwVeboxHeight, MOS_MAX(dwVeboxBottom, MHW_VEBOX_MIN_HEIGHT)),
        m_pvpMhwInterface->m_sfcInterface->m_veHeightAlignment);
    *pdwSurfaceWidth = MOS_ALIGN_CEIL(
        MOS_MIN(dwVeboxWidth, MOS_MAX(dwVeboxRight, MHW_VEBOX_MIN_WIDTH)),
        m_pvpMhwInterface->m_sfcInterface->m_veWidthAlignment);

    return MOS_STATUS_SUCCESS;
}

void VpScalingFilter::GetFormatWidthHeightAlignUnit(
    MOS_FORMAT inputFormat,
    uint16_t & widthAlignUnit,
    uint16_t & heightAlignUnit)
{
    widthAlignUnit = 1;
    heightAlignUnit = 1;

    switch (VpHal_GetSurfaceColorPack(inputFormat))
    {
    case VPHAL_COLORPACK_420:
        widthAlignUnit = 2;
        heightAlignUnit = 2;
        break;
    case VPHAL_COLORPACK_422:
        widthAlignUnit = 2;
        break;
    default:
        break;
    }
}

MOS_STATUS VpScalingFilter::IsColorfillEnable()
{      
    m_bColorfillEnable = (m_scalingParams.pColorFillParams &&
        (!RECT1_CONTAINS_RECT2(m_scalingParams.rcDstInput, m_scalingParams.rcDstOutput))) ?
        true : false;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpScalingFilter::SetColorFillParams()
{
    VPHAL_COLOR_SAMPLE_8        Src = {};
    VPHAL_CSPACE                src_cspace, dst_cspace;

    VP_PUBLIC_CHK_NULL_RETURN(m_sfcScalingParams);

    m_sfcScalingParams->sfcColorfillParams.bColorfillEnable = m_bColorfillEnable;

    if (m_bColorfillEnable)
    {
        VP_PUBLIC_CHK_NULL_RETURN(m_scalingParams.pColorFillParams);
        Src.dwValue = m_scalingParams.pColorFillParams->Color;
        src_cspace  = m_scalingParams.pColorFillParams->CSpace;
        dst_cspace  = m_scalingParams.colorSpaceOutput;

        // Convert BG color only if not done so before. CSC is expensive!
        if ((m_colorFillColorSrc.dwValue != Src.dwValue) ||
            (m_colorFillSrcCspace != src_cspace)         ||
            (m_colorFillRTCspace  != dst_cspace))
        {
            VpHal_CSC_8(&m_colorFillColorDst, &Src, src_cspace, dst_cspace);

            // store the values for next iteration
            m_colorFillColorSrc  = Src;
            m_colorFillSrcCspace = src_cspace;
            m_colorFillRTCspace  = dst_cspace;
        }

        VP_RENDER_CHK_STATUS_RETURN(SetYUVRGBPixel());
        m_sfcScalingParams->sfcColorfillParams.fColorFillAPixel = (float)Src.A / 255.0F;
    }

    if (m_scalingParams.pCompAlpha &&
        ((m_scalingParams.formatOutput == Format_A8R8G8B8) ||
        (m_scalingParams.formatOutput  == Format_A8B8G8R8) ||
        (m_scalingParams.formatOutput  == Format_AYUV)))
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
    VP_PUBLIC_CHK_NULL_RETURN(m_sfcScalingParams);

    if (IS_YUV_FORMAT(m_scalingParams.formatOutput) || (m_scalingParams.formatOutput == Format_AYUV))
    {
        m_sfcScalingParams->sfcColorfillParams.fColorFillYRPixel = (float)m_colorFillColorDst.Y / 255.0F;
        m_sfcScalingParams->sfcColorfillParams.fColorFillUGPixel = (float)m_colorFillColorDst.U / 255.0F;
        m_sfcScalingParams->sfcColorfillParams.fColorFillVBPixel = (float)m_colorFillColorDst.V / 255.0F;
    }
    else
    {
        // Swap the channel here because HW only natively supports XBGR output
        if ((m_scalingParams.formatOutput == Format_A8R8G8B8) || (m_scalingParams.formatOutput == Format_X8R8G8B8) || (m_scalingParams.formatOutput == Format_R10G10B10A2))
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
    VP_PUBLIC_CHK_NULL_RETURN(m_sfcScalingParams);
    VP_PUBLIC_CHK_NULL_RETURN(m_scalingParams.pCompAlpha);

    switch (m_scalingParams.pCompAlpha->AlphaMode)
    {
    case VPHAL_ALPHA_FILL_MODE_NONE:
        m_sfcScalingParams->sfcColorfillParams.fAlphaPixel      = m_scalingParams.pCompAlpha->fAlpha;
        m_sfcScalingParams->sfcColorfillParams.fColorFillAPixel = m_scalingParams.pCompAlpha->fAlpha;
        break;

    case VPHAL_ALPHA_FILL_MODE_BACKGROUND:
        m_sfcScalingParams->sfcColorfillParams.fAlphaPixel = m_bColorfillEnable ?
            m_sfcScalingParams->sfcColorfillParams.fColorFillAPixel : 1.0F;
        break;

    case VPHAL_ALPHA_FILL_MODE_SOURCE_STREAM:
    case VPHAL_ALPHA_FILL_MODE_OPAQUE:
    default:
        m_sfcScalingParams->sfcColorfillParams.fAlphaPixel      = 1.0F;
        m_sfcScalingParams->sfcColorfillParams.fColorFillAPixel = 1.0F;
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpScalingFilter::SetRectSurfaceAlignment(MOS_FORMAT format, bool isOutputSurf, uint32_t &width, uint32_t &height, RECT &rcSrc, RECT &rcDst)
{
    uint16_t   wWidthAlignUnit = 0;
    uint16_t   wHeightAlignUnit = 0;
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    GetFormatWidthHeightAlignUnit(format, wWidthAlignUnit, wHeightAlignUnit);

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
    rcDst.bottom = MOS_ALIGN_CEIL((uint32_t)rcDst.bottom, wHeightAlignUnit);
    rcDst.right  = MOS_ALIGN_CEIL((uint32_t)rcDst.right, wWidthAlignUnit);

    rcDst.top    = MOS_ALIGN_FLOOR((uint32_t)rcDst.top, wHeightAlignUnit);
    rcDst.left   = MOS_ALIGN_FLOOR((uint32_t)rcDst.left, wWidthAlignUnit);

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

// Prepare
MOS_STATUS VpScalingFilter::SetExecuteEngineCaps(
    FeatureParamScaling     &scalingParams,
    VP_EXECUTE_CAPS         vpExecuteCaps)
{
    VP_FUNC_CALL();

    m_executeCaps   = vpExecuteCaps;

    m_scalingParams = scalingParams;
    m_scalingParams.rcMaxSrcInput = m_scalingParams.rcSrcInput;

    // Set Src/Dst Surface Rect
    VP_PUBLIC_CHK_STATUS_RETURN(SetRectSurfaceAlignment(m_scalingParams.formatInput, false, m_scalingParams.dwWidthInput,
        m_scalingParams.dwHeightInput, m_scalingParams.rcSrcInput, m_scalingParams.rcDstInput));
    VP_PUBLIC_CHK_STATUS_RETURN(SetRectSurfaceAlignment(m_scalingParams.formatOutput, true, m_scalingParams.dwWidthOutput,
        m_scalingParams.dwHeightOutput, m_scalingParams.rcSrcOutput, m_scalingParams.rcDstOutput));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpScalingFilter::CalculateEngineParams()
{
    VP_RENDER_CHK_STATUS_RETURN(IsColorfillEnable());

    if (m_executeCaps.bSFC)
    {
        uint32_t                    dwSurfaceWidth = 0;
        uint32_t                    dwSurfaceHeight = 0;
        uint16_t                    wOutputWidthAlignUnit = 1;
        uint16_t                    wOutputHeightAlignUnit = 1;
        uint16_t                    wInputWidthAlignUnit = 1;
        uint16_t                    wInputHeightAlignUnit = 1;
        uint32_t                    wOutputScaledwidth = 0;
        uint32_t                    wOutputscaleHeight = 0;
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
        m_sfcScalingParams->dwAVSFilterMode = (m_scalingParams.scalingMode == VPHAL_SCALING_BILINEAR) ?
            MEDIASTATE_SFC_AVS_FILTER_BILINEAR :
            MEDIASTATE_SFC_AVS_FILTER_8x8;
        //Set input/Output boundary
        VP_RENDER_CHK_STATUS_RETURN(SfcAdjustBoundary(
            &dwSurfaceWidth,
            &dwSurfaceHeight));

        m_sfcScalingParams->dwInputFrameHeight = dwSurfaceHeight;
        m_sfcScalingParams->dwInputFrameWidth  = dwSurfaceWidth;

        // Apply alignment restriction to the Region of the output frame.
        GetFormatWidthHeightAlignUnit(
            m_scalingParams.formatOutput,
            wOutputWidthAlignUnit,
            wOutputHeightAlignUnit);

        // Apply alignment restriction to Region of the input frame.
        GetFormatWidthHeightAlignUnit(
            m_executeCaps.bDI ? Format_YUY2 : m_scalingParams.formatInput,
            wInputWidthAlignUnit,
            wInputHeightAlignUnit);

        m_sfcScalingParams->dwOutputFrameHeight = MOS_ALIGN_CEIL(m_scalingParams.dwHeightOutput, wOutputHeightAlignUnit);
        m_sfcScalingParams->dwOutputFrameWidth  = MOS_ALIGN_CEIL(m_scalingParams.dwWidthOutput, wOutputWidthAlignUnit);

        //Set source input offset in Horizontal/vertical
        m_sfcScalingParams->dwSourceRegionHorizontalOffset = MOS_ALIGN_CEIL((uint32_t)m_scalingParams.rcSrcInput.left, wInputWidthAlignUnit);
        m_sfcScalingParams->dwSourceRegionVerticalOffset   = MOS_ALIGN_CEIL((uint32_t)m_scalingParams.rcSrcInput.top, wInputHeightAlignUnit);
        m_sfcScalingParams->dwSourceRegionHeight           = MOS_ALIGN_FLOOR(
            MOS_MIN((uint32_t)(m_scalingParams.rcSrcInput.bottom - m_scalingParams.rcSrcInput.top), m_sfcScalingParams->dwInputFrameHeight),
            wInputHeightAlignUnit);
        m_sfcScalingParams->dwSourceRegionWidth            = MOS_ALIGN_FLOOR(
            MOS_MIN((uint32_t)(m_scalingParams.rcSrcInput.right - m_scalingParams.rcSrcInput.left), m_sfcScalingParams->dwInputFrameWidth),
            wInputWidthAlignUnit);

        // Size of the Output Region over the Render Target
        wOutputRegionHeight = MOS_ALIGN_CEIL(
            MOS_MIN((uint32_t)(m_scalingParams.rcDstInput.bottom - m_scalingParams.rcDstInput.top), m_scalingParams.dwHeightOutput),
            wInputHeightAlignUnit);
        wOutputRegionWidth = MOS_ALIGN_CEIL(
            MOS_MIN((uint32_t)(m_scalingParams.rcDstInput.right - m_scalingParams.rcDstInput.left), m_scalingParams.dwWidthOutput),
            wInputWidthAlignUnit);

        wOutputScaledwidth = MOS_ALIGN_FLOOR(
            MOS_MIN((uint32_t)(m_scalingParams.rcSrcOutput.right - m_scalingParams.rcSrcOutput.left), m_scalingParams.dwWidthOutput),
            wOutputWidthAlignUnit);
        wOutputscaleHeight = MOS_ALIGN_FLOOR(
            MOS_MIN((uint32_t)(m_scalingParams.rcSrcOutput.bottom - m_scalingParams.rcSrcOutput.top), m_scalingParams.dwHeightOutput),
            wOutputHeightAlignUnit);

        fScaleX = (float)wOutputRegionWidth / (float)m_sfcScalingParams->dwSourceRegionWidth;
        fScaleY = (float)wOutputRegionHeight / (float)m_sfcScalingParams->dwSourceRegionHeight;

        // Size of the Scaled Region over the Render Target
        m_sfcScalingParams->dwScaledRegionHeight           = MOS_ALIGN_CEIL(MOS_UF_ROUND(fScaleY * m_sfcScalingParams->dwSourceRegionHeight), wOutputHeightAlignUnit);
        m_sfcScalingParams->dwScaledRegionWidth            = MOS_ALIGN_CEIL(MOS_UF_ROUND(fScaleX * m_sfcScalingParams->dwSourceRegionWidth), wOutputWidthAlignUnit);

        m_sfcScalingParams->dwScaledRegionHeight = MOS_MIN(m_sfcScalingParams->dwScaledRegionHeight, m_sfcScalingParams->dwOutputFrameHeight);
        m_sfcScalingParams->dwScaledRegionWidth  = MOS_MIN(m_sfcScalingParams->dwScaledRegionWidth, m_sfcScalingParams->dwOutputFrameWidth);

        m_sfcScalingParams->dwScaledRegionHorizontalOffset = MOS_ALIGN_FLOOR((uint32_t)m_scalingParams.rcDstInput.left, wOutputWidthAlignUnit);
        m_sfcScalingParams->dwScaledRegionVerticalOffset   = MOS_ALIGN_FLOOR((uint32_t)m_scalingParams.rcDstInput.top, wOutputHeightAlignUnit);

        // Refine the Scaling ratios in the X and Y direction. SFC output Scaled size may be changed based on the restriction of SFC alignment.
        // The scaling ratio could be changed and not equal to the fScaleX/Y.
        // Driver must make sure that the scaling ratio should be matched with the output/input size before send to HW
        m_sfcScalingParams->fAVSXScalingRatio = (float)m_sfcScalingParams->dwScaledRegionWidth / (float)m_sfcScalingParams->dwSourceRegionWidth;
        m_sfcScalingParams->fAVSYScalingRatio = (float)m_sfcScalingParams->dwScaledRegionHeight / (float)m_sfcScalingParams->dwSourceRegionHeight;

        VP_RENDER_CHK_STATUS_RETURN(SetColorFillParams());
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
    return hwFilter.ConfigScalingParam(m_Params);
}

MOS_STATUS HwFilterScalingParameter::Initialize(HW_FILTER_SCALING_PARAM &param)
{
    m_Params = param;
    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                                   Packet Sfc Scaling Parameter                                   */
/****************************************************************************************************/
VpPacketParameter *VpSfcScalingParameter::Create(HW_FILTER_SCALING_PARAM &param)
{
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
    VpVeboxCmdPacket *pVeboxPacket = dynamic_cast<VpVeboxCmdPacket *>(pPacket);
    if (nullptr == pVeboxPacket)
    {
        return false;
    }

    SFC_SCALING_PARAMS *pParams = m_ScalingFilter.GetSfcParams();
    if (nullptr == pParams)
    {
        return false;
    }
    return MOS_SUCCEEDED(pVeboxPacket->SetScalingParams(pParams));
}

MOS_STATUS VpSfcScalingParameter::Initialize(HW_FILTER_SCALING_PARAM &params)
{
    VP_PUBLIC_CHK_STATUS_RETURN(m_ScalingFilter.Init());
    VP_PUBLIC_CHK_STATUS_RETURN(m_ScalingFilter.SetExecuteEngineCaps(params.scalingParams, params.vpExecuteCaps));
    VP_PUBLIC_CHK_STATUS_RETURN(m_ScalingFilter.CalculateEngineParams());
    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                                   Policy Sfc Scaling Handler                                     */
/****************************************************************************************************/
PolicySfcScalingHandler::PolicySfcScalingHandler()
{
    m_Type = FeatureTypeScalingOnSfc;
}
PolicySfcScalingHandler::~PolicySfcScalingHandler()
{}

bool PolicySfcScalingHandler::IsFeatureEnabled(VP_EXECUTE_CAPS vpExecuteCaps)
{
    return vpExecuteCaps.bSfcScaling;
}

HwFilterParameter *PolicySfcScalingHandler::CreateHwFilterParam(VP_EXECUTE_CAPS vpExecuteCaps, SwFilterPipe &swFilterPipe, PVP_MHWINTERFACE pHwInterface)
{
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
