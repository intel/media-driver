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
//! \file     vp_render_sfc_base.cpp
//! \brief    SFC rendering component
//! \details  The SFC renderer supports Scaling, IEF, CSC/ColorFill and Rotation.
//!           It's responsible for setting up HW states and generating the SFC
//!           commands.
//!

#include "vp_render_sfc_base.h"
#include "vp_utils.h"
#include "mhw_vebox.h"
#include "mhw_sfc.h"
#include "vp_render_ief.h"

namespace vp {

SfcRenderBase::SfcRenderBase(
    VP_MHWINTERFACE &vpMhwinterface,
    PVpAllocator &allocator):
    m_allocator(allocator)
{
    VP_PUBLIC_CHK_NULL_NO_STATUS_RETURN(vpMhwinterface.m_osInterface);
    VP_PUBLIC_CHK_NULL_NO_STATUS_RETURN(vpMhwinterface.m_sfcInterface);
    VP_PUBLIC_CHK_NULL_NO_STATUS_RETURN(vpMhwinterface.m_mhwMiInterface);
    VP_PUBLIC_CHK_NULL_NO_STATUS_RETURN(vpMhwinterface.m_skuTable);
    VP_PUBLIC_CHK_NULL_NO_STATUS_RETURN(vpMhwinterface.m_waTable);
    m_osInterface   = vpMhwinterface.m_osInterface;
    m_sfcInterface  = vpMhwinterface.m_sfcInterface;
    m_miInterface   = vpMhwinterface.m_mhwMiInterface;
    m_skuTable      = vpMhwinterface.m_skuTable;
    m_waTable       = vpMhwinterface.m_waTable;

    // Allocate AVS state
    InitAVSParams(
      &m_AvsParameters,
      k_YCoefficientTableSize,
      k_UVCoefficientTableSize);
}

SfcRenderBase::~SfcRenderBase()
{
    DestroyAVSParams(&m_AvsParameters);

    if (m_sfcStateParams)
    {
        MOS_FreeMemAndSetNull(m_sfcStateParams);
    }

    FreeResources();

    if (m_iefObj)
    {
        MOS_Delete(m_iefObj);
    }
}

MOS_STATUS SfcRenderBase::Init()
{
    MOS_ZeroMemory(&m_renderData, sizeof(m_renderData));

    m_bVdboxToSfc = false;
    m_pipeMode = MhwSfcInterface::SFC_PIPE_MODE_VEBOX;

    return InitSfcStateParams();
}

MOS_STATUS SfcRenderBase::SetCodecPipeMode(CODECHAL_STANDARD codecStandard)
{
    if (CODECHAL_VC1 == codecStandard ||
        CODECHAL_AVC == codecStandard ||
        CODECHAL_VP8 == codecStandard ||
        CODECHAL_JPEG == codecStandard)
    {
        m_pipeMode = MhwSfcInterface::SFC_PIPE_MODE_VDBOX;
    }
    else
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SfcRenderBase::Init(CODECHAL_STANDARD codecStandard, CodecDecodeJpegChromaType jpegChromaType, bool deblockingEnabled, uint32_t lcuSize)
{
    MOS_ZeroMemory(&m_renderData, sizeof(m_renderData));

    m_bVdboxToSfc = true;

    m_videoConfig.codecStandard = codecStandard;
    m_videoConfig.jpegChromaType = jpegChromaType;
    m_videoConfig.deblockingEnabled = deblockingEnabled;
    m_videoConfig.lcuSize = lcuSize;

    VP_PUBLIC_CHK_STATUS_RETURN(SetCodecPipeMode(codecStandard));

    return InitSfcStateParams();
}

void SfcRenderBase::SetRotationAndMirrowParams(PMHW_SFC_STATE_PARAMS psfcStateParams)
{
    VP_PUBLIC_CHK_NULL_NO_STATUS_RETURN(psfcStateParams);

    psfcStateParams->RotationMode  = (MHW_ROTATION)m_renderData.SfcRotation;
    psfcStateParams->bMirrorEnable = m_renderData.bMirrorEnable;
    psfcStateParams->dwMirrorType  = m_renderData.mirrorType;
}

void SfcRenderBase::SetChromasitingParams(PMHW_SFC_STATE_PARAMS psfcStateParams)
{
    VP_PUBLIC_CHK_NULL_NO_STATUS_RETURN(psfcStateParams);
    SetSfcStateInputChromaSubSampling(psfcStateParams);
    SetSfcStateInputOrderingMode(psfcStateParams);
}

void SfcRenderBase::SetColorFillParams(
    PMHW_SFC_STATE_PARAMS       psfcStateParams)
{
    VP_PUBLIC_CHK_NULL_NO_STATUS_RETURN(m_renderData.pColorFillParams);

    psfcStateParams->bColorFillEnable = m_renderData.pColorFillParams->bColorfillEnable;

    if (psfcStateParams->bColorFillEnable)
    {
        psfcStateParams->fColorFillYRPixel = m_renderData.pColorFillParams->fColorFillYRPixel;
        psfcStateParams->fColorFillUGPixel = m_renderData.pColorFillParams->fColorFillUGPixel;
        psfcStateParams->fColorFillVBPixel = m_renderData.pColorFillParams->fColorFillVBPixel;
        psfcStateParams->fColorFillAPixel  = m_renderData.pColorFillParams->fColorFillAPixel;
    }
}

void SfcRenderBase::SetXYAdaptiveFilter(
    PMHW_SFC_STATE_PARAMS       psfcStateParams)
{
    VP_PUBLIC_CHK_NULL_NO_STATUS_RETURN(psfcStateParams);

    // Enable Adaptive Filtering for YUV input only, if it is being upscaled
    // in either direction. We must check for this before clamping the SF.
    if (IS_YUV_FORMAT(m_renderData.SfcInputFormat) &&
      (m_renderData.fScaleX > 1.0F                 ||
       m_renderData.fScaleY > 1.0F)                &&
       (psfcStateParams->dwAVSFilterMode != MEDIASTATE_SFC_AVS_FILTER_BILINEAR))
    {
      psfcStateParams->bBypassXAdaptiveFilter = false;
      psfcStateParams->bBypassYAdaptiveFilter = false;
    }
    else
    {
      psfcStateParams->bBypassXAdaptiveFilter = true;
      psfcStateParams->bBypassYAdaptiveFilter = true;
    }
}

void SfcRenderBase::SetRGBAdaptive(
    PMHW_SFC_STATE_PARAMS       psfcStateParams)
{
    VP_PUBLIC_CHK_NULL_NO_STATUS_RETURN(psfcStateParams);

    if (IS_RGB_FORMAT(m_renderData.SfcInputFormat) &&
        psfcStateParams->b8tapChromafiltering == true)
    {
        psfcStateParams->bRGBAdaptive = true;
    }
    else
    {
        psfcStateParams->bRGBAdaptive = false;
    }
}

MOS_STATUS SfcRenderBase::SetIefStateCscParams(
    PMHW_SFC_STATE_PARAMS           psfcStateParams,
    PMHW_SFC_IEF_STATE_PARAMS       pIEFStateParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    VP_RENDER_CHK_NULL_RETURN(psfcStateParams);
    VP_RENDER_CHK_NULL_RETURN(pIEFStateParams);

    if (m_renderData.bCSC)
    {
        psfcStateParams->bCSCEnable = true;
        pIEFStateParams->bCSCEnable = true;

        if ((m_cscInputCspace != m_renderData.SfcInputCspace) ||
            (m_renderData.pSfcPipeOutSurface && m_cscRTCspace != m_renderData.pSfcPipeOutSurface->ColorSpace))
        {
            VpHal_GetCscMatrix(
                m_renderData.SfcInputCspace,
                m_renderData.pSfcPipeOutSurface->ColorSpace,
                m_cscCoeff,
                m_cscInOffset,
                m_cscOutOffset);

            m_cscInputCspace = m_renderData.SfcInputCspace;
            m_cscRTCspace    = m_renderData.pSfcPipeOutSurface->ColorSpace;
        }
        pIEFStateParams->pfCscCoeff     = m_cscCoeff;
        pIEFStateParams->pfCscInOffset  = m_cscInOffset;
        pIEFStateParams->pfCscOutOffset = m_cscOutOffset;
    }
    return eStatus;
}

MOS_STATUS SfcRenderBase::SetIefStateParams(
    PMHW_SFC_STATE_PARAMS           psfcStateParams)
{
    PMHW_SFC_IEF_STATE_PARAMS   pIefStateParams = nullptr;
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    VP_RENDER_CHK_NULL_RETURN(psfcStateParams);

    pIefStateParams = &m_IefStateParams;
    MOS_ZeroMemory(pIefStateParams, sizeof(*pIefStateParams));

    // Setup IEF and STE params
    if (m_renderData.bIEF && m_renderData.pIefParams)
    {
        VP_RENDER_CHK_NULL_RETURN(m_iefObj);
        m_iefObj->Init(m_renderData.pIefParams, m_renderData.SfcInputFormat, m_renderData.fScaleX, m_renderData.fScaleY);
        m_iefObj->SetHwState(psfcStateParams, pIefStateParams);
    } // end of setup IEF and STE params

    // Setup CSC params
    VP_RENDER_CHK_STATUS_RETURN(SetIefStateCscParams(
        psfcStateParams,
        pIefStateParams));

    return eStatus;
}

MOS_STATUS SfcRenderBase::SetAvsStateParams()
{
    MOS_STATUS                  eStatus            = MOS_STATUS_SUCCESS;
    PMHW_SFC_AVS_STATE          pMhwAvsState       = nullptr;
    MHW_SCALING_MODE            scalingMode        = MHW_SCALING_AVS;
    bool                        bUse8x8Filter      = false;

    VP_RENDER_CHK_NULL_RETURN(m_sfcInterface);

    pMhwAvsState = &m_avsState.AvsStateParams;
    MOS_ZeroMemory(pMhwAvsState, sizeof(MHW_SFC_AVS_STATE));

    if (m_renderData.bScaling ||
        m_renderData.bForcePolyPhaseCoefs)
    {
        if (m_renderData.SfcSrcChromaSiting == MHW_CHROMA_SITING_NONE)
        {
            if (VpHal_GetSurfaceColorPack(m_renderData.SfcInputFormat) == VPHAL_COLORPACK_420) // For 420, default is Left & Center, else default is Left & Top
            {
                m_renderData.SfcSrcChromaSiting = MHW_CHROMA_SITING_HORZ_LEFT | MHW_CHROMA_SITING_VERT_CENTER;
            }
            else
            {
                m_renderData.SfcSrcChromaSiting = MHW_CHROMA_SITING_HORZ_LEFT | MHW_CHROMA_SITING_VERT_TOP;
            }
        }

        pMhwAvsState->dwInputHorizontalSiting = (m_renderData.SfcSrcChromaSiting & MHW_CHROMA_SITING_HORZ_CENTER) ? SFC_AVS_INPUT_SITING_COEF_4_OVER_8 :
                                                ((m_renderData.SfcSrcChromaSiting & MHW_CHROMA_SITING_HORZ_RIGHT) ? SFC_AVS_INPUT_SITING_COEF_8_OVER_8 :
                                                SFC_AVS_INPUT_SITING_COEF_0_OVER_8);

        pMhwAvsState->dwInputVerticalSitting = (m_renderData.SfcSrcChromaSiting & MHW_CHROMA_SITING_VERT_CENTER) ? SFC_AVS_INPUT_SITING_COEF_4_OVER_8 :
                                                ((m_renderData.SfcSrcChromaSiting & MHW_CHROMA_SITING_VERT_BOTTOM) ? SFC_AVS_INPUT_SITING_COEF_8_OVER_8 :
                                                SFC_AVS_INPUT_SITING_COEF_0_OVER_8);

        if (m_renderData.SfcScalingMode == VPHAL_SCALING_NEAREST)
        {
            scalingMode = MHW_SCALING_NEAREST;
        }
        else if (m_renderData.SfcScalingMode == VPHAL_SCALING_BILINEAR)
        {
            scalingMode = MHW_SCALING_BILINEAR;
        }
        else
        {
            scalingMode = MHW_SCALING_AVS;
        }
        VP_RENDER_CHK_STATUS_RETURN(m_sfcInterface->SetSfcAVSScalingMode(scalingMode));

        if (m_renderData.sfcStateParams)
        {
            pMhwAvsState->dwAVSFilterMode = m_renderData.sfcStateParams->dwAVSFilterMode;
        }
        else
        {
            pMhwAvsState->dwAVSFilterMode = MEDIASTATE_SFC_AVS_FILTER_8x8;
        }

        if (pMhwAvsState->dwAVSFilterMode == MEDIASTATE_SFC_AVS_FILTER_8x8)
        {
            bUse8x8Filter = true;
        }

        VP_RENDER_CHK_STATUS_RETURN(m_sfcInterface->SetSfcSamplerTable(
            &m_avsState.LumaCoeffs,
            &m_avsState.ChromaCoeffs,
            m_renderData.pAvsParams,
            m_renderData.SfcInputFormat,
            m_renderData.fScaleX,
            m_renderData.fScaleY,
            m_renderData.SfcSrcChromaSiting,
            bUse8x8Filter,
            0,
            0));
    }

    return eStatus;
}

MOS_STATUS SfcRenderBase::SetupSfcState(PVP_SURFACE targetSurface)
{
    MOS_STATUS              eStatus = MOS_STATUS_SUCCESS;

    VP_RENDER_CHK_NULL_RETURN(targetSurface);
    VP_RENDER_CHK_NULL_RETURN(targetSurface->osSurface);

    //---------------------------------
    // Set SFC State:  common properties
    //---------------------------------
    m_renderData.sfcStateParams->sfcPipeMode = (MEDIASTATE_SFC_PIPE_MODE)m_pipeMode;

    m_renderData.sfcStateParams->OutputFrameFormat = targetSurface->osSurface->Format;
    m_renderData.sfcStateParams->dwOutputSurfaceOffset = targetSurface->osSurface->YPlaneOffset.iSurfaceOffset;
    m_renderData.sfcStateParams->wOutputSurfaceUXOffset = (uint16_t) targetSurface->osSurface->UPlaneOffset.iXOffset;
    m_renderData.sfcStateParams->wOutputSurfaceUYOffset = (uint16_t) targetSurface->osSurface->UPlaneOffset.iYOffset;
    m_renderData.sfcStateParams->wOutputSurfaceVXOffset = (uint16_t) targetSurface->osSurface->VPlaneOffset.iXOffset;
    m_renderData.sfcStateParams->wOutputSurfaceVYOffset = (uint16_t) targetSurface->osSurface->VPlaneOffset.iYOffset;

    m_renderData.pSfcPipeOutSurface = targetSurface;
    m_renderData.pAvsParams         = &m_AvsParameters;

    //---------------------------------
    // Set SFC State:  Scaling
    //---------------------------------
    m_AvsParameters.bForcePolyPhaseCoefs = m_renderData.bForcePolyPhaseCoefs;
    VP_RENDER_CHK_STATUS_RETURN(SetAvsStateParams());
    m_renderData.sfcStateParams->bAVSChromaUpsamplingEnable = m_renderData.bScaling ||
                                                               m_renderData.bForcePolyPhaseCoefs;

    //---------------------------------
    // Set SFC State:  CSC/IEF
    //---------------------------------
    if (m_renderData.bIEF ||
        m_renderData.bCSC)
    {
        VP_RENDER_CHK_STATUS_RETURN(SetIefStateParams(
            m_renderData.sfcStateParams));
    }

    //---------------------------------
    // Set SFC State: Rotation/Mirror
    //---------------------------------
    SetRotationAndMirrowParams(m_renderData.sfcStateParams);

    //---------------------------------
    // Set SFC State:  Chromasiting
    //---------------------------------
    SetChromasitingParams(m_renderData.sfcStateParams);

    //---------------------------------
    // Set SFC State:  XY Adaptive Filter
    //---------------------------------
    SetXYAdaptiveFilter(m_renderData.sfcStateParams);

    //---------------------------------
    // Set SFC State:  RGB Adaptive Filter
    //---------------------------------
    SetRGBAdaptive(m_renderData.sfcStateParams);

    //---------------------------------
    // Set SFC State:  Colorfill
    //---------------------------------
    SetColorFillParams(m_renderData.sfcStateParams);

    VP_RENDER_CHK_STATUS_RETURN(AllocateResources());

    m_renderData.sfcStateParams->pOsResOutputSurface = &targetSurface->osSurface->OsResource;

    if (m_AVSLineBufferSurface)
    {
        VP_PUBLIC_CHK_NULL_RETURN(m_AVSLineBufferSurface->osSurface);
        m_renderData.sfcStateParams->pOsResAVSLineBuffer = &m_AVSLineBufferSurface->osSurface->OsResource;
    }
    else
    {
        m_renderData.sfcStateParams->pOsResAVSLineBuffer = nullptr;
    }

    if (m_IEFLineBufferSurface)
    {
        VP_PUBLIC_CHK_NULL_RETURN(m_IEFLineBufferSurface->osSurface);
        m_renderData.sfcStateParams->pOsResIEFLineBuffer = &m_IEFLineBufferSurface->osSurface->OsResource;
    }
    else
    {
        m_renderData.sfcStateParams->pOsResIEFLineBuffer = nullptr;
    }
    return eStatus;
}

MOS_STATUS SfcRenderBase::SetScalingParams(PSFC_SCALING_PARAMS scalingParams)
{
    VP_PUBLIC_CHK_NULL_RETURN(scalingParams);

    if (MhwSfcInterface::SFC_PIPE_MODE_VEBOX != m_pipeMode                          &&
        (scalingParams->dwInputFrameHeight != scalingParams->dwSourceRegionHeight   ||
        scalingParams->dwInputFrameWidth != scalingParams->dwSourceRegionWidth))
    {
        // For Integral Image Mode, this source region width/height is Reserved.
        // In VD modes, source region width/height must be programmed to same value as input frame resolution width/height.
        VP_PUBLIC_ASSERTMESSAGE("Source region crop is not supported by Integral Image Mode and VD Mode!!");
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    // Adjust output width/height according to rotation.
    if (VPHAL_ROTATION_90                   == m_renderData.SfcRotation ||
        VPHAL_ROTATION_270                  == m_renderData.SfcRotation ||
        VPHAL_ROTATE_90_MIRROR_VERTICAL     == m_renderData.SfcRotation ||
        VPHAL_ROTATE_90_MIRROR_HORIZONTAL   == m_renderData.SfcRotation)
    {
        m_renderData.sfcStateParams->dwOutputFrameWidth         = scalingParams->dwOutputFrameHeight;
        m_renderData.sfcStateParams->dwOutputFrameHeight        = scalingParams->dwOutputFrameWidth;
    }
    else
    {
        m_renderData.sfcStateParams->dwOutputFrameWidth         = scalingParams->dwOutputFrameWidth;
        m_renderData.sfcStateParams->dwOutputFrameHeight        = scalingParams->dwOutputFrameHeight;
    }

    m_renderData.sfcStateParams->dwInputFrameHeight             = scalingParams->dwInputFrameHeight;
    m_renderData.sfcStateParams->dwInputFrameWidth              = scalingParams->dwInputFrameWidth;
    m_renderData.sfcStateParams->dwAVSFilterMode                = scalingParams->bBilinearScaling ?
                                                                    MEDIASTATE_SFC_AVS_FILTER_BILINEAR :
                                                                    (MhwSfcInterface::SFC_PIPE_MODE_VDBOX == m_pipeMode ?
                                                                        MEDIASTATE_SFC_AVS_FILTER_5x5 : MEDIASTATE_SFC_AVS_FILTER_8x8);
    m_renderData.sfcStateParams->dwSourceRegionHeight           = scalingParams->dwSourceRegionHeight;
    m_renderData.sfcStateParams->dwSourceRegionWidth            = scalingParams->dwSourceRegionWidth;
    m_renderData.sfcStateParams->dwSourceRegionVerticalOffset   = scalingParams->dwSourceRegionVerticalOffset;
    m_renderData.sfcStateParams->dwSourceRegionHorizontalOffset = scalingParams->dwSourceRegionHorizontalOffset;
    m_renderData.sfcStateParams->dwScaledRegionHeight           = scalingParams->dwScaledRegionHeight;
    m_renderData.sfcStateParams->dwScaledRegionWidth            = scalingParams->dwScaledRegionWidth;
    m_renderData.sfcStateParams->dwScaledRegionVerticalOffset   = scalingParams->dwScaledRegionVerticalOffset;
    m_renderData.sfcStateParams->dwScaledRegionHorizontalOffset = scalingParams->dwScaledRegionHorizontalOffset;
    m_renderData.sfcStateParams->fAVSXScalingRatio              = scalingParams->fAVSXScalingRatio;
    m_renderData.sfcStateParams->fAVSYScalingRatio              = scalingParams->fAVSYScalingRatio;

    m_renderData.bScaling = ((scalingParams->fAVSXScalingRatio == 1.0F) && (scalingParams->fAVSYScalingRatio == 1.0F)) ?
        false : true;

    m_renderData.fScaleX = scalingParams->fAVSXScalingRatio;
    m_renderData.fScaleY = scalingParams->fAVSYScalingRatio;
    m_renderData.SfcScalingMode = scalingParams->sfcScalingMode;

    // ColorFill/Alpha settings
    m_renderData.pColorFillParams            = &(scalingParams->sfcColorfillParams);
    m_renderData.sfcStateParams->fAlphaPixel = scalingParams->sfcColorfillParams.fAlphaPixel;
    m_renderData.sfcStateParams->fColorFillAPixel  = scalingParams->sfcColorfillParams.fColorFillAPixel;
    m_renderData.sfcStateParams->fColorFillUGPixel = scalingParams->sfcColorfillParams.fColorFillUGPixel;
    m_renderData.sfcStateParams->fColorFillVBPixel = scalingParams->sfcColorfillParams.fColorFillVBPixel;
    m_renderData.sfcStateParams->fColorFillYRPixel = scalingParams->sfcColorfillParams.fColorFillYRPixel;

    // SfcInputFormat should be initialized during SetCscParams if SfcInputFormat not being Format_Any.
    if (Format_Any == m_renderData.SfcInputFormat)
    {
        m_renderData.SfcInputFormat = scalingParams->inputFrameFormat;
    }
    else if (m_renderData.SfcInputFormat != scalingParams->inputFrameFormat)
    {
        VP_PUBLIC_ASSERTMESSAGE("Input formats configured during SetCscParams and SetScalingParams are not same!");
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    return MOS_STATUS_SUCCESS;
}

bool SfcRenderBase::IsVdboxSfcFormatSupported(
    CODECHAL_STANDARD           codecStandard,
    MOS_FORMAT                  inputFormat,
    MOS_FORMAT                  outputFormat)
{
    if (CODECHAL_AVC == codecStandard || CODECHAL_HEVC == codecStandard || CODECHAL_VP9 == codecStandard)
    {
        if ((inputFormat != Format_NV12) &&
            (inputFormat != Format_400P) &&
            (inputFormat != Format_IMC3) &&
            (inputFormat != Format_422H) &&
            (inputFormat != Format_444P) &&
            (inputFormat != Format_P010) &&
            (inputFormat != Format_YUY2) &&
            (inputFormat != Format_AYUV) &&
            (inputFormat != Format_Y210) &&
            (inputFormat != Format_Y410) &&
            (inputFormat != Format_P016) &&
            (inputFormat != Format_Y216) &&
            (inputFormat != Format_Y416))
        {
            VP_PUBLIC_ASSERTMESSAGE("Unsupported Output Format '0x%08x' for SFC.", inputFormat);
            return false;
        }

        if ((outputFormat != Format_A8R8G8B8) &&
            (outputFormat != Format_NV12) &&
            (outputFormat != Format_P010) &&
            (outputFormat != Format_YUY2) &&
            (outputFormat != Format_AYUV) &&
            (outputFormat != Format_P016) &&
            (outputFormat != Format_Y210) &&
            (outputFormat != Format_Y216) &&
            (outputFormat != Format_Y410) &&
            (outputFormat != Format_Y416))
        {
            VP_PUBLIC_ASSERTMESSAGE("Unsupported Output Format '0x%08x' for SFC.", outputFormat);
            return false;
        }
    }
    else
    {
        if ((inputFormat != Format_NV12) &&
            (inputFormat != Format_400P) &&
            (inputFormat != Format_IMC3) &&
            (inputFormat != Format_422H) &&
            (inputFormat != Format_444P) &&
            (inputFormat != Format_P010))
        {
            VP_PUBLIC_ASSERTMESSAGE("Unsupported Input Format '0x%08x' for SFC.", inputFormat);
            return false;
        }

        if (outputFormat != Format_A8R8G8B8 &&
            outputFormat != Format_NV12     &&
            outputFormat != Format_P010     &&
            outputFormat != Format_YUY2)
        {
            VP_PUBLIC_ASSERTMESSAGE("Unsupported Output Format '0x%08x' for SFC.", outputFormat);
            return false;
        }
    }

    return true;
}

MOS_STATUS SfcRenderBase::SetCSCParams(PSFC_CSC_PARAMS cscParams)
{
    VP_PUBLIC_CHK_NULL_RETURN(cscParams);

    if (MhwSfcInterface::SFC_PIPE_MODE_VEBOX == m_pipeMode)
    {
        m_renderData.bIEF           = cscParams->bIEFEnable;
        m_renderData.pIefParams     = cscParams->iefParams;
    }
    else
    {
        if (cscParams->bIEFEnable)
        {
            VP_PUBLIC_ASSERTMESSAGE("IEF is not supported by Integral Image Mode and VD Mode!");
        }
        m_renderData.bIEF = false;
        m_renderData.pIefParams = nullptr;
    }
    m_renderData.bCSC           = cscParams->bCSCEnabled;
    m_renderData.SfcInputCspace = cscParams->inputColorSpcase;
    m_renderData.SfcInputFormat = cscParams->inputFormat;

    // ARGB8,ABGR10,A16B16G16R16,VYUY and YVYU output format need to enable swap
    if (cscParams->outputFormat == Format_X8R8G8B8 ||
        cscParams->outputFormat == Format_A8R8G8B8 ||
        cscParams->outputFormat == Format_R10G10B10A2 ||
        cscParams->outputFormat == Format_A16B16G16R16 ||
        cscParams->outputFormat == Format_VYUY ||
        cscParams->outputFormat == Format_YVYU)
    {
        m_renderData.sfcStateParams->bRGBASwapEnable = true;
    }
    else
    {
        m_renderData.sfcStateParams->bRGBASwapEnable = false;
    }
    m_renderData.sfcStateParams->bInputColorSpace = cscParams->bInputColorSpace;

    // Chromasitting config
    // config SFC chroma up sampling
    m_renderData.bForcePolyPhaseCoefs   = cscParams->bChromaUpSamplingEnable;
    m_renderData.SfcSrcChromaSiting     = cscParams->sfcSrcChromaSiting;

    // 8-Tap chroma filter enabled or not
    m_renderData.sfcStateParams->b8tapChromafiltering = cscParams->b8tapChromafiltering;

    // config SFC chroma down sampling
    m_renderData.sfcStateParams->dwChromaDownSamplingHorizontalCoef = cscParams->chromaDownSamplingHorizontalCoef;
    m_renderData.sfcStateParams->dwChromaDownSamplingVerticalCoef   = cscParams->chromaDownSamplingVerticalCoef;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SfcRenderBase::SetRotMirParams(PSFC_ROT_MIR_PARAMS rotMirParams)
{
    VP_PUBLIC_CHK_NULL_RETURN(rotMirParams);

    if (MhwSfcInterface::SFC_PIPE_MODE_VEBOX != m_pipeMode      &&
        VPHAL_ROTATION_IDENTITY != rotMirParams->rotationMode   &&
        VPHAL_MIRROR_HORIZONTAL != rotMirParams->rotationMode)
    {
        VP_PUBLIC_ASSERTMESSAGE("Rotation is not supported by Integral Image Mode and VD Mode!");
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    m_renderData.SfcRotation   = rotMirParams->rotationMode;
    m_renderData.bMirrorEnable = rotMirParams->bMirrorEnable;
    m_renderData.mirrorType  = rotMirParams->mirrorType;

    // Adjust output width/height according to rotation.
    if (VPHAL_ROTATION_90                   == m_renderData.SfcRotation ||
        VPHAL_ROTATION_270                  == m_renderData.SfcRotation ||
        VPHAL_ROTATE_90_MIRROR_VERTICAL     == m_renderData.SfcRotation ||
        VPHAL_ROTATE_90_MIRROR_HORIZONTAL   == m_renderData.SfcRotation)
    {
        uint32_t width = m_renderData.sfcStateParams->dwOutputFrameWidth;
        m_renderData.sfcStateParams->dwOutputFrameWidth  = m_renderData.sfcStateParams->dwOutputFrameHeight;
        m_renderData.sfcStateParams->dwOutputFrameHeight = width;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SfcRenderBase::SetMmcParams(PMOS_SURFACE renderTarget, bool isFormalMmcSupported, bool isMmcEnabled)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(renderTarget);
    VP_PUBLIC_CHK_NULL_RETURN(m_renderData.sfcStateParams);

    if (renderTarget->CompressionMode               &&
        isFormalMmcSupported                        &&
        renderTarget->TileType == MOS_TILE_Y        &&
        isMmcEnabled)
    {
        m_renderData.sfcStateParams->bMMCEnable = true;
        m_renderData.sfcStateParams->MMCMode    = renderTarget->CompressionMode;
    }
    else
    {
        m_renderData.sfcStateParams->bMMCEnable = false;
        m_renderData.sfcStateParams->MMCMode    = MOS_MMC_DISABLED;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SfcRenderBase::SetSfcStateInputChromaSubSampling(
    PMHW_SFC_STATE_PARAMS       sfcStateParams)
{
    VP_PUBLIC_CHK_NULL_RETURN(sfcStateParams);
    VPHAL_COLORPACK colorPack = VpHal_GetSurfaceColorPack(m_renderData.SfcInputFormat);
    if (VPHAL_COLORPACK_400 == colorPack)
    {
        sfcStateParams->dwInputChromaSubSampling = MEDIASTATE_SFC_CHROMA_SUBSAMPLING_400;
    }
    else if (VPHAL_COLORPACK_411 == colorPack)
    {
        sfcStateParams->dwInputChromaSubSampling = MEDIASTATE_SFC_CHROMA_SUBSAMPLING_411;
    }
    else if (VPHAL_COLORPACK_420 == colorPack)
    {
        sfcStateParams->dwInputChromaSubSampling = MEDIASTATE_SFC_CHROMA_SUBSAMPLING_420;
    }
    else if (VPHAL_COLORPACK_422 == colorPack)
    {
        sfcStateParams->dwInputChromaSubSampling = MEDIASTATE_SFC_CHROMA_SUBSAMPLING_422H;
    }
    else if (VPHAL_COLORPACK_444 == colorPack)
    {
        sfcStateParams->dwInputChromaSubSampling = MEDIASTATE_SFC_CHROMA_SUBSAMPLING_444;
    }
    else
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SfcRenderBase::SetSfcStateInputOrderingMode(
    PMHW_SFC_STATE_PARAMS       sfcStateParams)
{
    VP_PUBLIC_CHK_NULL_RETURN(sfcStateParams);

    if (m_bVdboxToSfc)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(SetSfcStateInputOrderingModeVdbox(sfcStateParams));
    }
    else if (MhwSfcInterface::SFC_PIPE_MODE_VEBOX == m_pipeMode)
    {
        sfcStateParams->dwVDVEInputOrderingMode = MEDIASTATE_SFC_INPUT_ORDERING_VE_4x8;
    }
    else if (MEDIASTATE_SFC_PIPE_VE_TO_SFC_INTEGRAL == m_pipeMode)
    {
        sfcStateParams->dwVDVEInputOrderingMode = MEDIASTATE_SFC_INPUT_ORDERING_VE_4x4;
    }
    else
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SfcRenderBase::SetSfcStateInputOrderingModeVdbox(
    PMHW_SFC_STATE_PARAMS       sfcStateParams)
{
    VP_PUBLIC_CHK_NULL_RETURN(sfcStateParams);
    switch (m_videoConfig.codecStandard)
    {
    case CODECHAL_VC1:
        sfcStateParams->dwVDVEInputOrderingMode = MEDIASTATE_SFC_INPUT_ORDERING_VD_16x16_NOSHIFT;
        break;
    case CODECHAL_AVC:
        sfcStateParams->dwVDVEInputOrderingMode = m_videoConfig.deblockingEnabled ? MEDIASTATE_SFC_INPUT_ORDERING_VD_16x16_SHIFT :
            MEDIASTATE_SFC_INPUT_ORDERING_VD_16x16_NOSHIFT;
        break;
    case CODECHAL_VP8:
        sfcStateParams->dwVDVEInputOrderingMode = m_videoConfig.deblockingEnabled ? MEDIASTATE_SFC_INPUT_ORDERING_VD_16x16_SHIFT :
            MEDIASTATE_SFC_INPUT_ORDERING_VD_16x16_VP8;
        break;
    case CODECHAL_JPEG:
        return SetSfcStateInputOrderingModeJpeg(sfcStateParams);
    case CODECHAL_HEVC:
    case CODECHAL_VP9:
        return SetSfcStateInputOrderingModeHcp(sfcStateParams);
    default:
        VP_PUBLIC_ASSERTMESSAGE("Unsupported codec standard.");
        return MOS_STATUS_INVALID_PARAMETER;
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SfcRenderBase::SetSfcStateInputOrderingModeJpeg(
    PMHW_SFC_STATE_PARAMS       sfcStateParams)
{
    VP_PUBLIC_CHK_NULL_RETURN(sfcStateParams);
    if (CODECHAL_JPEG != m_videoConfig.codecStandard)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }
    switch (m_videoConfig.jpegChromaType)
    {
    case jpegYUV400:
        sfcStateParams->dwVDVEInputOrderingMode = MEDIASTATE_SFC_INPUT_ORDERING_VD_8x8_JPEG;
        break;
    case jpegYUV411:
        sfcStateParams->dwVDVEInputOrderingMode = MEDIASTATE_SFC_INPUT_ORDERING_VD_8x8_JPEG;
        break;
    case jpegYUV420:
        sfcStateParams->dwVDVEInputOrderingMode = MEDIASTATE_SFC_INPUT_ORDERING_VD_16x16_JPEG;
        break;
    case jpegYUV422H2Y:
        sfcStateParams->dwVDVEInputOrderingMode = MEDIASTATE_SFC_INPUT_ORDERING_VD_8x8_JPEG;
        break;
    case jpegYUV422H4Y:
        sfcStateParams->dwVDVEInputOrderingMode = MEDIASTATE_SFC_INPUT_ORDERING_VD_16x16_JPEG;
        break;
    case jpegYUV444:
    case jpegRGB:
    case jpegBGR:
        sfcStateParams->dwVDVEInputOrderingMode = MEDIASTATE_SFC_INPUT_ORDERING_VD_8x8_JPEG;
        break;
    default:
        VP_PUBLIC_ASSERTMESSAGE("Unsupported input format of SFC.");
        return MOS_STATUS_INVALID_PARAMETER;
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SfcRenderBase::InitMhwOutSurfParams(
  PVP_SURFACE                               pSfcPipeOutSurface,
  PMHW_SFC_OUT_SURFACE_PARAMS               pMhwOutSurfParams)
{
    MOS_STATUS                   eStatus = MOS_STATUS_SUCCESS;
    VP_RENDER_CHK_NULL_RETURN(pSfcPipeOutSurface);
    VP_RENDER_CHK_NULL_RETURN(pSfcPipeOutSurface->osSurface);
    VP_RENDER_CHK_NULL_RETURN(pMhwOutSurfParams);

    MOS_ZeroMemory(pMhwOutSurfParams, sizeof(*pMhwOutSurfParams));

    pMhwOutSurfParams->ChromaSiting = pSfcPipeOutSurface->ChromaSiting;
    pMhwOutSurfParams->dwWidth = pSfcPipeOutSurface->osSurface->dwWidth;
    pMhwOutSurfParams->dwHeight = pSfcPipeOutSurface->osSurface->dwHeight;
    pMhwOutSurfParams->dwPitch = pSfcPipeOutSurface->osSurface->dwPitch;
    pMhwOutSurfParams->TileType = pSfcPipeOutSurface->osSurface->TileType;
    pMhwOutSurfParams->TileModeGMM = pSfcPipeOutSurface->osSurface->TileModeGMM;
    pMhwOutSurfParams->bGMMTileEnabled = pSfcPipeOutSurface->osSurface->bGMMTileEnabled;
    pMhwOutSurfParams->pOsResource = &(pSfcPipeOutSurface->osSurface->OsResource);
    pMhwOutSurfParams->Format = pSfcPipeOutSurface->osSurface->Format;
    pMhwOutSurfParams->bCompressible = pSfcPipeOutSurface->osSurface->bCompressible;
    pMhwOutSurfParams->dwCompressionFormat = pSfcPipeOutSurface->osSurface->CompressionFormat;
    pMhwOutSurfParams->dwSurfaceXOffset = pSfcPipeOutSurface->osSurface->YPlaneOffset.iXOffset;
    pMhwOutSurfParams->dwSurfaceYOffset = pSfcPipeOutSurface->osSurface->YPlaneOffset.iYOffset;

    if (pSfcPipeOutSurface->osSurface->dwPitch > 0)
    {
        pMhwOutSurfParams->dwUYoffset = ((pSfcPipeOutSurface->osSurface->UPlaneOffset.iSurfaceOffset - pSfcPipeOutSurface->osSurface->YPlaneOffset.iSurfaceOffset)
              / pSfcPipeOutSurface->osSurface->dwPitch) + pSfcPipeOutSurface->osSurface->UPlaneOffset.iYOffset;
    }

    return eStatus;
}

MOS_STATUS SfcRenderBase::AddSfcLock(
    PMOS_COMMAND_BUFFER            pCmdBuffer,
    PMHW_SFC_LOCK_PARAMS           pSfcLockParams)
{
    VP_RENDER_CHK_NULL_RETURN(m_sfcInterface);

    // Send SFC_LOCK command to acquire SFC pipe for Vebox
    VP_RENDER_CHK_STATUS_RETURN(m_sfcInterface->AddSfcLock(
        pCmdBuffer,
        pSfcLockParams));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SfcRenderBase::SendSfcCmd(
    bool                            bOutputToMemory,
    PMOS_COMMAND_BUFFER             pCmdBuffer)
{
    PMHW_SFC_INTERFACE              pSfcInterface;
    MHW_SFC_LOCK_PARAMS             SfcLockParams;
    MOS_STATUS                      eStatus;
    MHW_SFC_OUT_SURFACE_PARAMS      OutSurfaceParam;

    VP_RENDER_CHK_NULL_RETURN(m_sfcInterface);
    VP_RENDER_CHK_NULL_RETURN(pCmdBuffer);

    eStatus                 = MOS_STATUS_SUCCESS;
    pSfcInterface           = m_sfcInterface;

    // Setup params for SFC Lock command
    SfcLockParams.sfcPipeMode     = m_pipeMode;
    SfcLockParams.bOutputToMemory = bOutputToMemory;

    // Send SFC_LOCK command to acquire SFC pipe for Vebox
    VP_RENDER_CHK_STATUS_RETURN(AddSfcLock(
        pCmdBuffer,
        &SfcLockParams));

    VP_RENDER_CHK_STATUS_RETURN(InitMhwOutSurfParams(
        m_renderData.pSfcPipeOutSurface,
        &OutSurfaceParam));

    // Send SFC_STATE command
    VP_RENDER_CHK_STATUS_RETURN(pSfcInterface->AddSfcState(
        pCmdBuffer,
        m_renderData.sfcStateParams,
        &OutSurfaceParam));

    // Send SFC_AVS_STATE command
    VP_RENDER_CHK_STATUS_RETURN(pSfcInterface->AddSfcAvsState(
        pCmdBuffer,
        &m_avsState.AvsStateParams));

    if (m_renderData.bScaling ||
        m_renderData.bForcePolyPhaseCoefs)
    {
        // Send SFC_AVS_LUMA_TABLE command
        VP_RENDER_CHK_STATUS_RETURN(pSfcInterface->AddSfcAvsLumaTable(
            pCmdBuffer,
            &m_avsState.LumaCoeffs));

        // Send SFC_AVS_CHROMA_TABLE command
        VP_RENDER_CHK_STATUS_RETURN(pSfcInterface->AddSfcAvsChromaTable(
            pCmdBuffer,
            &m_avsState.ChromaCoeffs));
    }

    // Send SFC_IEF_STATE command
    if (m_renderData.bIEF || m_renderData.bCSC)
    {
        // Will modified when enable IEF/CSC
      VP_RENDER_CHK_STATUS_RETURN(pSfcInterface->AddSfcIefState(
            pCmdBuffer,
            &m_IefStateParams));
    }

    // Send SFC_FRAME_START command to start processing a frame
    VP_RENDER_CHK_STATUS_RETURN(pSfcInterface->AddSfcFrameStart(
        pCmdBuffer,
        m_pipeMode));

    return eStatus;
}

void SfcRenderBase::InitAVSParams(
    PMHW_AVS_PARAMS     pAVS_Params,
    uint32_t            uiYCoeffTableSize,
    uint32_t            uiUVCoeffTableSize)
{
    int32_t     size;
    char*       ptr;

    VP_PUBLIC_CHK_NULL_NO_STATUS_RETURN(pAVS_Params);
    VP_PUBLIC_CHK_NULL_NO_STATUS_RETURN((void*)(uiYCoeffTableSize > 0));
    VP_PUBLIC_CHK_NULL_NO_STATUS_RETURN((void*)(uiUVCoeffTableSize > 0));

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
        VP_RENDER_ASSERTMESSAGE("No memory to allocate AVS coefficient tables.");
        return;
    }

    pAVS_Params->piYCoefsX = (int32_t*)ptr;

    ptr += uiYCoeffTableSize;
    pAVS_Params->piUVCoefsX = (int32_t*)ptr;

    ptr += uiUVCoeffTableSize;
    pAVS_Params->piYCoefsY  = (int32_t*)ptr;

    ptr += uiYCoeffTableSize;
    pAVS_Params->piUVCoefsY = (int32_t*)ptr;
}

void SfcRenderBase::DestroyAVSParams(
    PMHW_AVS_PARAMS   pAVS_Params)
{
    VP_PUBLIC_CHK_NULL_NO_STATUS_RETURN(pAVS_Params);
    MOS_SafeFreeMemory(pAVS_Params->piYCoefsX);
    pAVS_Params->piYCoefsX = nullptr;
}

uint32_t SfcRenderBase::GetAvsLineBufferSize(bool b8tapChromafiltering, uint32_t width, uint32_t height)
{
    uint32_t size = 0;
    uint32_t linebufferSizePerPixel = 0;

    if (MhwSfcInterface::SFC_PIPE_MODE_VDBOX == m_pipeMode)
    {
        if (b8tapChromafiltering)
        {
            linebufferSizePerPixel = SFC_AVS_LINEBUFFER_SIZE_PER_PIXEL_8_TAP_8BIT;
        }
        else
        {
            linebufferSizePerPixel = SFC_AVS_LINEBUFFER_SIZE_PER_PIXEL_4_TAP_8BIT;
        }
    }
    else
    {
        // For vebox and hcp.
        if (b8tapChromafiltering)
        {
            linebufferSizePerPixel = SFC_AVS_LINEBUFFER_SIZE_PER_PIXEL_8_TAP_12BIT;
        }
        else
        {
            linebufferSizePerPixel = SFC_AVS_LINEBUFFER_SIZE_PER_PIXEL_4_TAP_12BIT;
        }
    }

    // For VD+SFC mode, width needs be used. For VE+SFC mode, height needs be used.
    if (MhwSfcInterface::SFC_PIPE_MODE_VEBOX == m_pipeMode)
    {
        size = height * linebufferSizePerPixel;
    }
    else
    {
        size = width * linebufferSizePerPixel;
    }
    return size;
}

uint32_t SfcRenderBase::GetIefLineBufferSize(uint32_t heightOutput)
{
    // For VE+SFC mode, height needs be used.
    if (MhwSfcInterface::SFC_PIPE_MODE_VEBOX == m_pipeMode)
    {
        return heightOutput * SFC_IEF_LINEBUFFER_SIZE_PER_VERTICAL_PIXEL;
    }
    else
    {
        return 0;
    }
}

uint32_t SfcRenderBase::GetSfdLineBufferSize(MOS_FORMAT formatOutput, uint32_t widthOutput, uint32_t heightOutput)
{
    int size = 0;

    // For VD+SFC mode, width needs be used. For VE+SFC mode, height needs be used.
    if (MhwSfcInterface::SFC_PIPE_MODE_VEBOX == m_pipeMode)
    {
        return (VPHAL_COLORPACK_444 == VpHal_GetSurfaceColorPack(formatOutput)) ? 0 : (heightOutput * SFC_SFD_LINEBUFFER_SIZE_PER_PIXEL);
    }
    else
    {
        return (VPHAL_COLORPACK_444 == VpHal_GetSurfaceColorPack(formatOutput) || VPHAL_COLORPACK_422 == VpHal_GetSurfaceColorPack(formatOutput)) ?
            0 : (widthOutput * SFC_SFD_LINEBUFFER_SIZE_PER_PIXEL);
    }
}

MOS_STATUS SfcRenderBase::AllocateResources()
{
    uint32_t                size;
    bool                    allocated;
    PMHW_SFC_STATE_PARAMS   sfcStateParams;

    VP_RENDER_CHK_NULL_RETURN(m_allocator);
    VP_RENDER_CHK_NULL_RETURN(m_renderData.sfcStateParams);

    allocated = false;
    sfcStateParams = m_renderData.sfcStateParams;

    // Allocate AVS Line Buffer surface----------------------------------------------
    size = GetAvsLineBufferSize(sfcStateParams->b8tapChromafiltering, sfcStateParams->dwInputFrameWidth, sfcStateParams->dwInputFrameHeight);

    if (size)
    {
        VP_RENDER_CHK_STATUS_RETURN(m_allocator->ReAllocateSurface(
                                      m_AVSLineBufferSurface,
                                      "SfcAVSLineBufferSurface",
                                      Format_Buffer,
                                      MOS_GFXRES_BUFFER,
                                      MOS_TILE_LINEAR,
                                      size,
                                      1,
                                      false,
                                      MOS_MMC_DISABLED,
                                      allocated,
                                      false,
                                      MOS_HW_RESOURCE_USAGE_VP_INTERNAL_READ_WRITE_FF));
    }
    else if (m_AVSLineBufferSurface)
    {
        m_allocator->DestroyVpSurface(m_AVSLineBufferSurface);
    }

    // Allocate IEF Line Buffer surface----------------------------------------------
    size = GetIefLineBufferSize(sfcStateParams->dwScaledRegionHeight);

    if (size)
    {
        VP_RENDER_CHK_STATUS_RETURN(m_allocator->ReAllocateSurface(
                                      m_IEFLineBufferSurface,
                                      "SfcIEFLineBufferSurface",
                                      Format_Buffer,
                                      MOS_GFXRES_BUFFER,
                                      MOS_TILE_LINEAR,
                                      size,
                                      1,
                                      false,
                                      MOS_MMC_DISABLED,
                                      allocated,
                                      false,
                                      MOS_HW_RESOURCE_USAGE_VP_INTERNAL_READ_WRITE_FF));
    }
    else if (m_IEFLineBufferSurface)
    {
        m_allocator->DestroyVpSurface(m_IEFLineBufferSurface);
    }

    // Allocate SFD Line Buffer surface
    size = GetSfdLineBufferSize(sfcStateParams->OutputFrameFormat, sfcStateParams->dwScaledRegionWidth, sfcStateParams->dwScaledRegionHeight);

    if (size)
    {
        VP_RENDER_CHK_STATUS_RETURN(m_allocator->ReAllocateSurface(
            m_SFDLineBufferSurface,
            "SfcSFDLineBufferSurface",
            Format_Buffer,
            MOS_GFXRES_BUFFER,
            MOS_TILE_LINEAR,
            size,
            1,
            false,
            MOS_MMC_DISABLED,
            allocated,
            false,
            MOS_HW_RESOURCE_USAGE_VP_INTERNAL_READ_WRITE_FF));
    }
    else if (m_SFDLineBufferSurface)
    {
        m_allocator->DestroyVpSurface(m_SFDLineBufferSurface);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SfcRenderBase::FreeResources()
{
    VP_RENDER_CHK_NULL_RETURN(m_allocator);
    // Free AVS Line Buffer surface for SFC
    m_allocator->DestroyVpSurface(m_AVSLineBufferSurface);

    // Free IEF Line Buffer surface for SFC
    m_allocator->DestroyVpSurface(m_IEFLineBufferSurface);

    // Free SFD Line Buffer surface for SFC
    m_allocator->DestroyVpSurface(m_SFDLineBufferSurface);

    return MOS_STATUS_SUCCESS;
}
}
