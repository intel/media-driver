/*
* Copyright (c) 2018, Intel Corporation
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
    PVPHAL_SURFACE pSurface,
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
    VP_PUBLIC_CHK_NULL_RETURN(pSurface);
    VP_PUBLIC_CHK_NULL_RETURN(pdwSurfaceWidth);
    VP_PUBLIC_CHK_NULL_RETURN(pdwSurfaceHeight);

    // For the VEBOX output to SFC, the width is multiple of 16 and height
    // is multiple of 4
    dwVeboxHeight = pSurface->dwHeight;
    dwVeboxWidth  = pSurface->dwWidth;
    dwVeboxBottom = (uint32_t)pSurface->rcMaxSrc.bottom;
    dwVeboxRight  = (uint32_t)pSurface->rcMaxSrc.right;

    if (pSurface->bDirectionalScalar)
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
    VP_PUBLIC_CHK_NULL_RETURN(m_inputSurface);
    VP_PUBLIC_CHK_NULL_RETURN(m_targetSurface);

    m_bColorfillEnable = (m_colorFillParams &&
        (!RECT1_CONTAINS_RECT2(m_inputSurface->rcDst, m_targetSurface->rcDst))) ?
        true : false;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpScalingFilter::SetColorFillParams()
{
    VPHAL_COLOR_SAMPLE_8        Src = {};
    VPHAL_CSPACE                src_cspace, dst_cspace;

    VP_PUBLIC_CHK_NULL_RETURN(m_targetSurface);
    VP_PUBLIC_CHK_NULL_RETURN(m_sfcScalingParams);

    m_sfcScalingParams->sfcColorfillParams.bColorfillEnable = m_bColorfillEnable;

    if (m_bColorfillEnable)
    {
        Src.dwValue = m_colorFillParams->Color;
        src_cspace  = m_colorFillParams->CSpace;
        dst_cspace  = m_targetSurface->ColorSpace;

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

    if (m_colorfillAlpha &&
        ((m_targetSurface->Format == Format_A8R8G8B8) ||
        (m_targetSurface->Format  == Format_A8B8G8R8) ||
        (m_targetSurface->Format  == Format_AYUV)))
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
    VP_PUBLIC_CHK_NULL_RETURN(m_targetSurface);

    if (IS_YUV_FORMAT(m_targetSurface->Format) || (m_targetSurface->Format == Format_AYUV))
    {
        m_sfcScalingParams->sfcColorfillParams.fColorFillYRPixel = (float)m_colorFillColorDst.Y / 255.0F;
        m_sfcScalingParams->sfcColorfillParams.fColorFillUGPixel = (float)m_colorFillColorDst.U / 255.0F;
        m_sfcScalingParams->sfcColorfillParams.fColorFillVBPixel = (float)m_colorFillColorDst.V / 255.0F;
    }
    else
    {
        // Swap the channel here because HW only natively supports XBGR output
        if ((m_targetSurface->Format == Format_A8R8G8B8) || (m_targetSurface->Format == Format_X8R8G8B8) || (m_targetSurface->Format == Format_R10G10B10A2))
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
    VP_PUBLIC_CHK_NULL_RETURN(m_colorfillAlpha);

    switch (m_colorfillAlpha->AlphaMode)
    {
    case VPHAL_ALPHA_FILL_MODE_NONE:
        m_sfcScalingParams->sfcColorfillParams.fAlphaPixel      = m_colorfillAlpha->fAlpha;
        m_sfcScalingParams->sfcColorfillParams.fColorFillAPixel = m_colorfillAlpha->fAlpha;
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

MOS_STATUS VpScalingFilter::SetRectSurfaceAlignment(PVPHAL_SURFACE pSurface)
{
    uint16_t   wWidthAlignUnit;
    uint16_t   wHeightAlignUnit;
    MOS_STATUS eStatus;

    eStatus = MOS_STATUS_SUCCESS;

    VP_PUBLIC_CHK_NULL_RETURN(pSurface);

    GetFormatWidthHeightAlignUnit(pSurface->Format, wWidthAlignUnit, wHeightAlignUnit);

    // The source rectangle is floored to the aligned unit to
    // get rid of invalid data(ex: an odd numbered src rectangle with NV12 format
    // will have invalid UV data for the last line of Y data).
    pSurface->rcSrc.bottom = MOS_ALIGN_FLOOR((uint32_t)pSurface->rcSrc.bottom, wHeightAlignUnit);
    pSurface->rcSrc.right  = MOS_ALIGN_FLOOR((uint32_t)pSurface->rcSrc.right, wWidthAlignUnit);

    pSurface->rcSrc.top    = MOS_ALIGN_CEIL((uint32_t)pSurface->rcSrc.top, wHeightAlignUnit);
    pSurface->rcSrc.left   = MOS_ALIGN_CEIL((uint32_t)pSurface->rcSrc.left, wWidthAlignUnit);

    // The Destination rectangle is rounded to the upper alignment unit to prevent the loss of
    // data which was present in the source rectangle
    // Current output alignment is based on input format.
    // Better output alignmentg solution should be based on output format.
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
        VP_PUBLIC_NORMALMESSAGE("Surface Parameter is invalid.");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
    }

    return eStatus;
}

// Prepare
MOS_STATUS VpScalingFilter::SetExecuteEngineCaps(
    PVP_PIPELINE_PARAMS     vpRenderParams,
    VP_EXECUTE_CAPS         vpExecuteCaps)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(vpRenderParams);
    VP_PUBLIC_CHK_NULL_RETURN(vpRenderParams->pSrc[0]);

    m_inputSurface = vpRenderParams->pSrc[0];

    m_inputSurface->rcMaxSrc = m_inputSurface->rcSrc;

    m_targetSurface = vpRenderParams->pTarget[0];
    m_executeCaps   = vpExecuteCaps;

    // Set Src/Dst Surface Rect
    VP_PUBLIC_CHK_STATUS_RETURN(SetRectSurfaceAlignment(m_inputSurface));
    VP_PUBLIC_CHK_STATUS_RETURN(SetRectSurfaceAlignment(m_targetSurface));

    //Colrofill Params
    m_colorFillParams = vpRenderParams->pColorFillParams;

    if (m_colorFillParams)
    {
        m_colorfillAlpha = vpRenderParams->pCompAlpha;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpScalingFilter::SetExecuteEngineParams()
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_STATUS_RETURN(CalculateEngineParams());

    if (m_executeCaps.bSFC)
    {
        VpVeboxCmdPacket           *packet = (VpVeboxCmdPacket*)m_packet;

        VP_RENDER_CHK_STATUS_RETURN(packet->SetScalingParams(m_sfcScalingParams));
    }
    // Need add support for Render engine
    else
    {

    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpScalingFilter::CalculateEngineParams()
{
    VP_PUBLIC_CHK_NULL_RETURN(m_inputSurface);
    VP_PUBLIC_CHK_NULL_RETURN(m_targetSurface);

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
        m_sfcScalingParams->dwAVSFilterMode = (m_inputSurface->ScalingMode == VPHAL_SCALING_BILINEAR) ?
            MEDIASTATE_SFC_AVS_FILTER_BILINEAR :
            MEDIASTATE_SFC_AVS_FILTER_8x8;
        //Set input/Output boundary
        VP_RENDER_CHK_STATUS_RETURN(SfcAdjustBoundary(
            m_inputSurface,
            &dwSurfaceWidth,
            &dwSurfaceHeight));

        m_sfcScalingParams->dwInputFrameHeight = dwSurfaceHeight;
        m_sfcScalingParams->dwInputFrameWidth  = dwSurfaceWidth;

        // Apply alignment restriction to the Region of the output frame.
        GetFormatWidthHeightAlignUnit(
            m_targetSurface->Format,
            wOutputWidthAlignUnit,
            wOutputHeightAlignUnit);

        // Apply alignment restriction to Region of the input frame.
        GetFormatWidthHeightAlignUnit(
            m_executeCaps.bDI ? Format_YUY2 : m_inputSurface->Format,
            wInputWidthAlignUnit,
            wInputHeightAlignUnit);

        m_sfcScalingParams->dwOutputFrameHeight = MOS_ALIGN_CEIL(m_targetSurface->dwHeight, wOutputHeightAlignUnit);;
        m_sfcScalingParams->dwOutputFrameWidth  = MOS_ALIGN_CEIL(m_targetSurface->dwWidth, wOutputWidthAlignUnit);;

        //Set source input offset in Horizontal/vertical
        m_sfcScalingParams->dwSourceRegionHorizontalOffset = MOS_ALIGN_CEIL((uint32_t)m_inputSurface->rcSrc.left, wInputWidthAlignUnit);
        m_sfcScalingParams->dwSourceRegionVerticalOffset   = MOS_ALIGN_CEIL((uint32_t)m_inputSurface->rcSrc.top, wInputHeightAlignUnit);
        m_sfcScalingParams->dwSourceRegionHeight           = MOS_ALIGN_FLOOR(
            MOS_MIN((uint32_t)(m_inputSurface->rcSrc.bottom - m_inputSurface->rcSrc.top), m_sfcScalingParams->dwInputFrameHeight),
            wInputHeightAlignUnit);
        m_sfcScalingParams->dwSourceRegionWidth            = MOS_ALIGN_FLOOR(
            MOS_MIN((uint32_t)(m_inputSurface->rcSrc.right - m_inputSurface->rcSrc.left), m_sfcScalingParams->dwInputFrameWidth),
            wInputWidthAlignUnit);

        // Size of the Output Region over the Render Target
        wOutputRegionHeight = MOS_ALIGN_CEIL(
            MOS_MIN((uint32_t)(m_inputSurface->rcDst.bottom - m_inputSurface->rcDst.top), m_targetSurface->dwHeight),
            wInputHeightAlignUnit);
        wOutputRegionWidth = MOS_ALIGN_CEIL(
            MOS_MIN((uint32_t)(m_inputSurface->rcDst.right - m_inputSurface->rcDst.left), m_targetSurface->dwWidth),
            wInputWidthAlignUnit);

        wOutputScaledwidth = MOS_ALIGN_FLOOR(
            MOS_MIN((uint32_t)(m_targetSurface->rcSrc.right - m_targetSurface->rcSrc.left), m_targetSurface->dwWidth),
            wOutputWidthAlignUnit);
        wOutputscaleHeight = MOS_ALIGN_FLOOR(
            MOS_MIN((uint32_t)(m_targetSurface->rcSrc.bottom - m_targetSurface->rcSrc.top), m_targetSurface->dwHeight),
            wOutputHeightAlignUnit);

        // Scaled region is pre-rotated. Adjust its width and height with those of the output frame
        if (m_inputSurface->Rotation == VPHAL_ROTATION_IDENTITY ||
            m_inputSurface->Rotation == VPHAL_ROTATION_180 ||
            m_inputSurface->Rotation == VPHAL_MIRROR_HORIZONTAL ||
            m_inputSurface->Rotation == VPHAL_MIRROR_VERTICAL)
        {
            fScaleX = (float)wOutputRegionWidth / (float)m_sfcScalingParams->dwSourceRegionWidth;
            fScaleY = (float)wOutputRegionHeight / (float)m_sfcScalingParams->dwSourceRegionHeight;
        }
        else
        {
            fScaleX = (float)wOutputRegionHeight / (float)m_sfcScalingParams->dwSourceRegionWidth;
            fScaleY = (float)wOutputRegionWidth / (float)m_sfcScalingParams->dwSourceRegionHeight;
        }

        // Size of the Scaled Region over the Render Target
        m_sfcScalingParams->dwScaledRegionHeight           = MOS_ALIGN_CEIL(MOS_UF_ROUND(fScaleY * m_sfcScalingParams->dwSourceRegionHeight), wOutputHeightAlignUnit);
        m_sfcScalingParams->dwScaledRegionWidth            = MOS_ALIGN_CEIL(MOS_UF_ROUND(fScaleX * m_sfcScalingParams->dwSourceRegionWidth), wOutputWidthAlignUnit);

        if (m_inputSurface->Rotation == VPHAL_ROTATION_IDENTITY ||
            m_inputSurface->Rotation == VPHAL_ROTATION_180      ||
            m_inputSurface->Rotation == VPHAL_MIRROR_HORIZONTAL ||
            m_inputSurface->Rotation == VPHAL_MIRROR_VERTICAL)
        {
            m_sfcScalingParams->dwScaledRegionHeight = MOS_MIN(m_sfcScalingParams->dwScaledRegionHeight, m_sfcScalingParams->dwOutputFrameHeight);
            m_sfcScalingParams->dwScaledRegionWidth  = MOS_MIN(m_sfcScalingParams->dwScaledRegionWidth, m_sfcScalingParams->dwOutputFrameWidth);
        }
        else
        {
            m_sfcScalingParams->dwScaledRegionHeight = MOS_MIN(m_sfcScalingParams->dwScaledRegionHeight, m_sfcScalingParams->dwOutputFrameWidth);
            m_sfcScalingParams->dwScaledRegionWidth  = MOS_MIN(m_sfcScalingParams->dwScaledRegionWidth, m_sfcScalingParams->dwOutputFrameHeight);
        }

        m_sfcScalingParams->dwScaledRegionHorizontalOffset = MOS_ALIGN_FLOOR((uint32_t)m_inputSurface->rcDst.left, wOutputWidthAlignUnit);
        m_sfcScalingParams->dwScaledRegionVerticalOffset   = MOS_ALIGN_FLOOR((uint32_t)m_inputSurface->rcDst.top, wOutputHeightAlignUnit);

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
 HwFilterParameter *HwFilterScalingParameter::Create(HW_FILTER_SCALING_PARAM &param, VpFeatureType featureType)
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

HwFilterScalingParameter::HwFilterScalingParameter(VpFeatureType featureType) : HwFilterParameter(featureType)
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
    VP_PUBLIC_CHK_STATUS_RETURN(m_ScalingFilter.SetExecuteEngineCaps(params.pPipelineParams, params.vpExecuteCaps));
    VP_PUBLIC_CHK_STATUS_RETURN(m_ScalingFilter.CalculateEngineParams());
    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                                   Policy Sfc Scaling Handler                                     */
/****************************************************************************************************/
PolicySfcScalingHandler::PolicySfcScalingHandler()
{
    m_Type = VpFeatureTypeSfcScaling;
}
PolicySfcScalingHandler::~PolicySfcScalingHandler()
{}

bool PolicySfcScalingHandler::IsFeatureEnabled(VP_EXECUTE_CAPS vpExecuteCaps)
{
    return vpExecuteCaps.bSfcScaling;
}

HwFilterParameter *PolicySfcScalingHandler::GetHwFeatureParameter(SwFilterPipe &swFilterPipe)
{
    return nullptr;
}

HwFilterParameter *PolicySfcScalingHandler::GetHwFeatureParameter(VP_EXECUTE_CAPS vpExecuteCaps, VP_PIPELINE_PARAMS &pipelineParams, PVP_MHWINTERFACE pHwInterface)
{
    if (IsFeatureEnabled(vpExecuteCaps))
    {
        HW_FILTER_SCALING_PARAM paramScaling = {};
        paramScaling.pHwInterface = pHwInterface;
        paramScaling.pPipelineParams = &pipelineParams;
        paramScaling.vpExecuteCaps = vpExecuteCaps;
        paramScaling.pPacketParamFactory = &m_PacketParamFactory;
        HwFilterParameter *pHwFilterParam = GetHwFeatureParameterFromPool();

        if (pHwFilterParam)
        {
            if (MOS_FAILED(((HwFilterScalingParameter*)pHwFilterParam)->Initialize(paramScaling)))
            {
                ReturnHwFeatureParameter(pHwFilterParam);
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
