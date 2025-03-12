/*
* Copyright (c) 2022-2024, Intel Corporation
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
#include "mos_interface.h"
#include "mhw_sfc_itf.h"
#include "mhw_mi_itf.h"
#include "vp_platform_interface.h"
#include "vp_hal_ddi_utils.h"

namespace vp {

SfcRenderBase::SfcRenderBase(
    VP_MHWINTERFACE &vpMhwinterface,
    PVpAllocator &allocator,
    bool disbaleSfcDithering) :
    m_allocator(allocator),
    m_disableSfcDithering(disbaleSfcDithering)
{
    VP_PUBLIC_CHK_NULL_NO_STATUS_RETURN(vpMhwinterface.m_osInterface);
    VP_PUBLIC_CHK_NULL_NO_STATUS_RETURN(vpMhwinterface.m_skuTable);
    VP_PUBLIC_CHK_NULL_NO_STATUS_RETURN(vpMhwinterface.m_waTable);
    m_osInterface   = vpMhwinterface.m_osInterface;
    m_skuTable      = vpMhwinterface.m_skuTable;
    m_waTable       = vpMhwinterface.m_waTable;

    // Allocate AVS state
    InitAVSParams(
      &m_AvsParameters,
      k_YCoefficientTableSize,
      k_UVCoefficientTableSize);

    m_sfcItf = vpMhwinterface.m_vpPlatformInterface->GetMhwSfcItf();
    m_miItf = vpMhwinterface.m_vpPlatformInterface->GetMhwMiItf();
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
    VP_FUNC_CALL();

    MOS_ZeroMemory(&m_renderData, sizeof(m_renderData));

    m_bVdboxToSfc = false;
    m_pipeMode = mhw::sfc::SFC_PIPE_MODE_VEBOX;

    m_scalabilityParams.numPipe = 1;
    m_scalabilityParams.curPipe = 0;

    MOS_ZeroMemory(&m_histogramSurf, sizeof(m_histogramSurf));

    return InitSfcStateParams();
}

MOS_STATUS SfcRenderBase::SetCodecPipeMode(CODECHAL_STANDARD codecStandard)
{
    VP_FUNC_CALL();

    if (CODECHAL_VC1 == codecStandard ||
        CODECHAL_AVC == codecStandard ||
        CODECHAL_VP8 == codecStandard ||
        CODECHAL_JPEG == codecStandard)
    {
        m_pipeMode = mhw::sfc::SFC_PIPE_MODE_VDBOX;
    }
    else
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SfcRenderBase::Init(VIDEO_PARAMS &videoParams)
{
    VP_FUNC_CALL();

    MOS_ZeroMemory(&m_renderData, sizeof(m_renderData));

    m_bVdboxToSfc = true;

    m_videoConfig = videoParams;

    m_videoConfig.scalabilityParams.numPipe = (0 == m_videoConfig.scalabilityParams.numPipe ? 1 : m_videoConfig.scalabilityParams.numPipe);
    if (m_videoConfig.scalabilityParams.curPipe >= m_videoConfig.scalabilityParams.numPipe)
    {
        VP_RENDER_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    m_scalabilityParams = m_videoConfig.scalabilityParams;

    VP_PUBLIC_CHK_STATUS_RETURN(SetCodecPipeMode(m_videoConfig.codecStandard));

    MOS_ZeroMemory(&m_histogramSurf, sizeof(m_histogramSurf));

    return InitSfcStateParams();
}

void SfcRenderBase::SetRotationAndMirrowParams(mhw::sfc::SFC_STATE_PAR *psfcStateParams)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_NO_STATUS_RETURN(psfcStateParams);

    psfcStateParams->RotationMode  = (MHW_ROTATION)m_renderData.SfcRotation;
    psfcStateParams->bMirrorEnable = m_renderData.bMirrorEnable;
    psfcStateParams->dwMirrorType  = m_renderData.mirrorType;
}

void SfcRenderBase::SetChromasitingParams(mhw::sfc::SFC_STATE_PAR *psfcStateParams)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_NO_STATUS_RETURN(psfcStateParams);
    SetSfcStateInputChromaSubSampling(psfcStateParams);
    SetSfcStateInputOrderingMode(psfcStateParams);
}

void SfcRenderBase::SetColorFillParams(
    mhw::sfc::SFC_STATE_PAR       *psfcStateParams)
{
    VP_FUNC_CALL();

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
    mhw::sfc::SFC_STATE_PAR       *psfcStateParams)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_NO_STATUS_RETURN(psfcStateParams);

    // Enable Adaptive Filtering for YUV input only, if it is being upscaled
    // in either direction. We must check for this before clamping the SF.
    if ((IS_YUV_FORMAT(m_renderData.SfcInputFormat) ||
        m_renderData.SfcInputFormat == Format_AYUV) &&
        (m_renderData.fScaleX > 1.0F                ||
        m_renderData.fScaleY > 1.0F)                &&
        psfcStateParams->dwAVSFilterMode != MEDIASTATE_SFC_AVS_FILTER_BILINEAR)
    {
        //For AVS, we need set psfcStateParams->bBypassXAdaptiveFilter and bBypassYAdaptiveFilter as false;
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
    mhw::sfc::SFC_STATE_PAR       *psfcStateParams)
{
    VP_FUNC_CALL();

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
    mhw::sfc::SFC_STATE_PAR           *psfcStateParams,
    mhw::sfc::SFC_IEF_STATE_PAR       *pIEFStateParams)
{
    VP_FUNC_CALL();

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    VP_RENDER_CHK_NULL_RETURN(psfcStateParams);
    VP_RENDER_CHK_NULL_RETURN(pIEFStateParams);

    if (m_renderData.bCSC)
    {
        psfcStateParams->bCSCEnable = true;
        pIEFStateParams->bCSCEnable = true;
        if (m_bVdboxToSfc)
        {
            m_cscInputSwapNeeded = false;
            if (m_videoConfig.jpeg.jpegChromaType == jpegRGB && m_videoConfig.codecStandard == CODECHAL_JPEG)
            {
                m_cscCoeff[0] = 1.000000000f;
                m_cscCoeff[1] = 0.000000000f;
                m_cscCoeff[2] = 0.000000000f;
                m_cscCoeff[3] = 0.000000000f;
                m_cscCoeff[4] = 1.000000000f;
                m_cscCoeff[5] = 0.000000000f;
                m_cscCoeff[6] = 0.000000000f;
                m_cscCoeff[7] = 0.000000000f;
                m_cscCoeff[8] = 1.000000000f;

                m_cscInOffset[0] = 0.000000000f;  // Adjusted to S8.2 to accommodate VPHAL
                m_cscInOffset[1] = 0.000000000f;  // Adjusted to S8.2 to accommodate VPHAL
                m_cscInOffset[2] = 0.000000000f;  // Adjusted to S8.2 to accommodate VPHAL
            }
            else
            {
                if (m_renderData.SfcInputFormat != Format_400P)
                {
                    m_cscCoeff[0] = 1.16438353f;
                    m_cscCoeff[1] = 0.000000000f;
                    m_cscCoeff[2] = 1.59602666f;
                    m_cscCoeff[3] = 1.16438353f;
                    m_cscCoeff[4] = -0.391761959f;
                    m_cscCoeff[5] = -0.812967300f;
                    m_cscCoeff[6] = 1.16438353f;
                    m_cscCoeff[7] = 2.01723218f;
                    m_cscCoeff[8] = 0.000000000f;
                }
                else
                {
                    m_cscCoeff[0] = 1.16438353f;
                    m_cscCoeff[1] = 0.000000000f;
                    m_cscCoeff[2] = 0.000000000f;
                    m_cscCoeff[3] = 1.16438353f;
                    m_cscCoeff[4] = 0.000000000f;
                    m_cscCoeff[5] = 0.000000000f;
                    m_cscCoeff[6] = 1.16438353f;
                    m_cscCoeff[7] = 0.000000000f;
                    m_cscCoeff[8] = 0.000000000f;
                }
                m_cscInOffset[0] = -16.000000f;   // Adjusted to S8.2 to accommodate VPHAL
                m_cscInOffset[1] = -128.000000f;  // Adjusted to S8.2 to accommodate VPHAL
                m_cscInOffset[2] = -128.000000f;  // Adjusted to S8.2 to accommodate VPHAL
            }
            m_cscOutOffset[0] = 0.000000000f;  // Adjusted to S8.2 to accommodate VPHAL
            m_cscOutOffset[1] = 0.000000000f;  // Adjusted to S8.2 to accommodate VPHAL
            m_cscOutOffset[2] = 0.000000000f;  // Adjusted to S8.2 to accommodate VPHAL
        }
        else if ((m_cscInputCspace != m_renderData.SfcInputCspace) ||
                (m_renderData.pSfcPipeOutSurface && m_cscRTCspace != m_renderData.pSfcPipeOutSurface->ColorSpace))
        {
            VpUtils::GetCscMatrixForVeSfc8Bit(
                m_renderData.SfcInputCspace,
                m_renderData.pSfcPipeOutSurface->ColorSpace,
                m_cscCoeff,
                m_cscInOffset,
                m_cscOutOffset);

            // swap the 1st and 3rd columns of the transfer matrix for A8R8G8B8 and X8R8G8B8
            // to ensure SFC input being A8B8G8R8.
            if (IsInputChannelSwapNeeded(m_renderData.SfcInputFormat))
            {
                float   fTemp[3] = {};
                fTemp[0] = m_cscCoeff[0];
                fTemp[1] = m_cscCoeff[3];
                fTemp[2] = m_cscCoeff[6];

                m_cscCoeff[0] = m_cscCoeff[2];
                m_cscCoeff[3] = m_cscCoeff[5];
                m_cscCoeff[6] = m_cscCoeff[8];

                m_cscCoeff[2] = fTemp[0];
                m_cscCoeff[5] = fTemp[1];
                m_cscCoeff[8] = fTemp[2];
                m_cscInputSwapNeeded = true;
            }
            else
            {
                m_cscInputSwapNeeded = false;
            }

            VP_RENDER_NORMALMESSAGE("sfc csc coeff calculated. (sfcInputCspace, cscRTCspace) current (%d, %d), previous (%d, %d), swap flag %d",
                m_renderData.SfcInputCspace, m_renderData.pSfcPipeOutSurface->ColorSpace,
                m_cscInputCspace, m_cscRTCspace, m_cscInputSwapNeeded ? 1 :0);

            m_cscInputCspace = m_renderData.SfcInputCspace;
            m_cscRTCspace    = m_renderData.pSfcPipeOutSurface->ColorSpace;
        }
        else if (m_cscInputSwapNeeded != IsInputChannelSwapNeeded(m_renderData.SfcInputFormat))
        {
            float fTemp[3] = {};
            fTemp[0]       = m_cscCoeff[0];
            fTemp[1]       = m_cscCoeff[3];
            fTemp[2]       = m_cscCoeff[6];

            m_cscCoeff[0] = m_cscCoeff[2];
            m_cscCoeff[3] = m_cscCoeff[5];
            m_cscCoeff[6] = m_cscCoeff[8];

            m_cscCoeff[2]   = fTemp[0];
            m_cscCoeff[5]   = fTemp[1];
            m_cscCoeff[8]   = fTemp[2];

            m_cscInputSwapNeeded = IsInputChannelSwapNeeded(m_renderData.SfcInputFormat);

            VP_RENDER_NORMALMESSAGE("sfc csc coeff swap flag need be updated to %d. sfcInputFormat %d, sfcInputCspace %d, cscRTCspace %d",
                (m_cscInputSwapNeeded ? 1 : 0),
                m_renderData.SfcInputFormat, m_cscInputCspace, m_cscRTCspace);
        }
        else
        {
            VP_RENDER_NORMALMESSAGE("sfc csc coeff reused. sfcInputFormat %d, sfcInputCspace %d, cscRTCspace %d, swap flag %d",
                m_renderData.SfcInputFormat, m_cscInputCspace, m_cscRTCspace, (m_cscInputSwapNeeded ? 1 : 0));
        }

        pIEFStateParams->pfCscCoeff     = m_cscCoeff;
        pIEFStateParams->pfCscInOffset  = m_cscInOffset;
        pIEFStateParams->pfCscOutOffset = m_cscOutOffset;
    }
    return eStatus;
}

MOS_STATUS SfcRenderBase::SetIefStateParams(
    mhw::sfc::SFC_STATE_PAR           *psfcStateParams)
{
    VP_FUNC_CALL();

    mhw::sfc::SFC_IEF_STATE_PAR   *pIefStateParams = nullptr;
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    VP_RENDER_CHK_NULL_RETURN(psfcStateParams);

    auto &iefStateParams = m_sfcItf->MHW_GETPAR_F(SFC_IEF_STATE)();
    pIefStateParams      = &iefStateParams;
    MOS_ZeroMemory(pIefStateParams, sizeof(*pIefStateParams));
    pIefStateParams->sfcPipeMode = m_pipeMode;

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
    VP_FUNC_CALL();

    MOS_STATUS                  eStatus            = MOS_STATUS_SUCCESS;
    mhw::sfc::SFC_AVS_STATE_PAR *pMhwAvsState       = nullptr;
    MHW_SCALING_MODE            scalingMode        = MHW_SCALING_AVS;
    bool                        bUse8x8Filter      = false;

    auto &avsStateParams = m_sfcItf->MHW_GETPAR_F(SFC_AVS_STATE)();
    pMhwAvsState         = &avsStateParams;
    MOS_ZeroMemory(pMhwAvsState, sizeof(MHW_SFC_AVS_STATE));
    pMhwAvsState->sfcPipeMode = m_pipeMode;

    VP_RENDER_NORMALMESSAGE("bScaling %d, bForcePolyPhaseCoefs %d",
        (m_renderData.bScaling ? 1 :0),
        (m_renderData.bForcePolyPhaseCoefs ? 1 : 0));

    if (m_renderData.bScaling ||
        m_renderData.bForcePolyPhaseCoefs)
    {
        if (m_renderData.SfcSrcChromaSiting == MHW_CHROMA_SITING_NONE)
        {
            if (VpHalDDIUtils::GetSurfaceColorPack(m_renderData.SfcInputFormat) == VPHAL_COLORPACK_420) // For 420, default is Left & Center, else default is Left & Top
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
        VP_RENDER_CHK_STATUS_RETURN(SetSfcAVSScalingMode(scalingMode));

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

        // directly get the par of SFC_AVS_LUMA_Coeff_Table_PAR  and initialize.
        auto &lumaCoeffs = m_sfcItf->MHW_GETPAR_F(SFC_AVS_LUMA_Coeff_Table)();

        // directly get the par of SFC_AVS_CHROMA_Coeff_Table  and initialize.
        auto &chromaCoeffs = m_sfcItf->MHW_GETPAR_F(SFC_AVS_CHROMA_Coeff_Table)();

        lumaCoeffs.sfcPipeMode              = m_pipeMode;
        chromaCoeffs.sfcPipeMode            = m_pipeMode;

        VP_RENDER_CHK_STATUS_RETURN(SetSfcSamplerTable(
            &lumaCoeffs,
            &chromaCoeffs,
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

MOS_STATUS SfcRenderBase::SetLineBuffer(PMOS_RESOURCE &osResLineBuffer, VP_SURFACE *lineBuffer)
{
    VP_FUNC_CALL();

    if (lineBuffer)
    {
        if (nullptr == lineBuffer->osSurface || Mos_ResourceIsNull(&lineBuffer->osSurface->OsResource))
        {
            VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_NULL_POINTER);
        }
        osResLineBuffer = &lineBuffer->osSurface->OsResource;
    }
    else
    {
        osResLineBuffer = nullptr;
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SfcRenderBase::SetupSfcState(PVP_SURFACE targetSurface)
{
    VP_FUNC_CALL();

    MOS_STATUS              eStatus = MOS_STATUS_SUCCESS;

    VP_RENDER_CHK_NULL_RETURN(targetSurface);
    VP_RENDER_CHK_NULL_RETURN(targetSurface->osSurface);

    //---------------------------------
    // Set SFC State:  common properties
    //---------------------------------
    m_renderData.sfcStateParams->sfcPipeMode = (MEDIASTATE_SFC_PIPE_MODE)m_pipeMode;

    m_renderData.sfcStateParams->InputFrameFormat  = m_renderData.SfcInputFormat;
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

    if (m_renderData.b1stPassOfSfc2PassScaling)
    {
        VP_RENDER_CHK_STATUS_RETURN(SetLineBuffer(m_renderData.sfcStateParams->pOsResAVSLineBuffer, m_AVSLineBufferSurfaceArrayfor1stPassofSfc2Pass[m_scalabilityParams.curPipe]));
        VP_RENDER_CHK_STATUS_RETURN(SetLineBuffer(m_renderData.sfcStateParams->pOsResIEFLineBuffer, m_IEFLineBufferSurfaceArrayfor1stPassofSfc2Pass[m_scalabilityParams.curPipe]));
    }
    else
    {
        VP_RENDER_CHK_STATUS_RETURN(SetLineBuffer(m_renderData.sfcStateParams->pOsResAVSLineBuffer, m_AVSLineBufferSurfaceArray[m_scalabilityParams.curPipe]));
        VP_RENDER_CHK_STATUS_RETURN(SetLineBuffer(m_renderData.sfcStateParams->pOsResIEFLineBuffer, m_IEFLineBufferSurfaceArray[m_scalabilityParams.curPipe]));
    }

    VP_RENDER_CHK_STATUS_RETURN(SetupScalabilityParams());

    return eStatus;
}

MOS_STATUS SfcRenderBase::SetScalingParams(PSFC_SCALING_PARAMS scalingParams)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(scalingParams);

    if (mhw::sfc::SFC_PIPE_MODE_VEBOX      != m_pipeMode                          &&
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
                                                                    (mhw::sfc::SFC_PIPE_MODE_VDBOX == m_pipeMode ?
                                                                        MEDIASTATE_SFC_AVS_FILTER_5x5 : MEDIASTATE_SFC_AVS_FILTER_8x8);
    m_renderData.sfcStateParams->dwSourceRegionHeight           = scalingParams->dwSourceRegionHeight;
    m_renderData.sfcStateParams->dwSourceRegionWidth            = scalingParams->dwSourceRegionWidth;
    m_renderData.sfcStateParams->dwSourceRegionVerticalOffset   = scalingParams->dwSourceRegionVerticalOffset;
    m_renderData.sfcStateParams->dwSourceRegionHorizontalOffset = scalingParams->dwSourceRegionHorizontalOffset;
    m_renderData.sfcStateParams->dwScaledRegionHeight           = scalingParams->dwScaledRegionHeight;
    m_renderData.sfcStateParams->dwScaledRegionWidth            = scalingParams->dwScaledRegionWidth;
    m_renderData.sfcStateParams->dwScaledRegionVerticalOffset   = scalingParams->dwScaledRegionVerticalOffset;
    m_renderData.sfcStateParams->dwScaledRegionHorizontalOffset = scalingParams->dwScaledRegionHorizontalOffset;
    if (scalingParams->bRectangleEnabled)
    {
        m_renderData.sfcStateParams->bRectangleEnabled                      = true;
        m_renderData.sfcStateParams->dwTargetRectangleStartHorizontalOffset = scalingParams->dwTargetRectangleStartHorizontalOffset;
        m_renderData.sfcStateParams->dwTargetRectangleStartVerticalOffset   = scalingParams->dwTargetRectangleStartVerticalOffset;
        m_renderData.sfcStateParams->dwTargetRectangleEndHorizontalOffset   = scalingParams->dwTargetRectangleEndHorizontalOffset;
        m_renderData.sfcStateParams->dwTargetRectangleEndVerticalOffset     = scalingParams->dwTargetRectangleEndVerticalOffset;
    }
    else
    {
        m_renderData.sfcStateParams->bRectangleEnabled = false;
    }
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
    m_renderData.sfcStateParams->isDemosaicEnabled = scalingParams->isDemosaicNeeded;

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

    m_renderData.b1stPassOfSfc2PassScaling = scalingParams->b1stPassOfSfc2PassScaling;

    VP_RENDER_NORMALMESSAGE("fScaleX %f, fScaleY %f, bScaling %d, SfcScalingMode %d, SfcInputFormat %d, bRectangleEnabled",
        m_renderData.fScaleX, m_renderData.fScaleY,
        (m_renderData.bScaling ? 1 : 0),
        m_renderData.SfcScalingMode,
        m_renderData.SfcInputFormat,
        (scalingParams->bRectangleEnabled ? 1: 0));

    return MOS_STATUS_SUCCESS;
}

bool SfcRenderBase::IsVdboxSfcInputFormatSupported(
    CODECHAL_STANDARD           codecStandard,
    MOS_FORMAT                  inputFormat)
{
    VP_FUNC_CALL();

    if (CODECHAL_AVC == codecStandard || CODECHAL_HEVC == codecStandard ||
        CODECHAL_VP9 == codecStandard || CODECHAL_AV1 == codecStandard)
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
    }
    else if (codecStandard < CODECHAL_HCP_BASE) // For other legacy standard.
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
    }
    else
    {
        VP_PUBLIC_ASSERTMESSAGE("Unsupported standard '0x%08x' for SFC.", codecStandard);
        return false;
    }

    return true;
}

bool SfcRenderBase::IsVdboxSfcOutputFormatSupported(
    CODECHAL_STANDARD           codecStandard,
    MOS_FORMAT                  outputFormat,
    MOS_TILE_TYPE               tileType)
{
    VP_FUNC_CALL();

    if (tileType == MOS_TILE_LINEAR && (outputFormat == Format_NV12 || outputFormat == Format_P010))
    {
        VP_PUBLIC_ASSERTMESSAGE("Unsupported output format '0x%08x' on tile type '0x%08x'", outputFormat, tileType)
        return false;
    }

    if (CODECHAL_AVC == codecStandard || CODECHAL_HEVC == codecStandard ||
        CODECHAL_VP9 == codecStandard || CODECHAL_AV1 == codecStandard)
    {
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
    else if (codecStandard < CODECHAL_HCP_BASE) // For other legacy standard.
    {
        if (outputFormat != Format_A8R8G8B8 &&
            outputFormat != Format_NV12     &&
            outputFormat != Format_P010     &&
            outputFormat != Format_YUY2)
        {
            VP_PUBLIC_ASSERTMESSAGE("Unsupported Output Format '0x%08x' for SFC.", outputFormat);
            return false;
        }
    }
    else
    {
        VP_PUBLIC_ASSERTMESSAGE("Unsupported standard '0x%08x' for SFC.", codecStandard);
        return false;
    }

    return true;
}

bool SfcRenderBase::IsInputChannelSwapNeeded(MOS_FORMAT inputFormat)
{
    VP_FUNC_CALL();

    // 1st and 3rd columns of A8R8G8B8 and X8R8G8B8 need be swapped
    // to ensure SFC input being A8B8G8R8.
    if (inputFormat == Format_A8R8G8B8 ||
        inputFormat == Format_X8R8G8B8)
    {
        return true;
    }
    else
    {
        return false;
    }
}

MOS_STATUS SfcRenderBase::UpdateIefParams(PVPHAL_IEF_PARAMS iefParams)
{
    VP_FUNC_CALL();
    m_renderData.bIEF           = (iefParams &&
        iefParams->bEnabled &&
        iefParams->fIEFFactor > 0.0F);
    m_renderData.pIefParams     = iefParams;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SfcRenderBase::UpdateCscParams(FeatureParamCsc &cscParams)
{
    VP_RENDER_CHK_STATUS_RETURN(UpdateIefParams(cscParams.pIEFParams));
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SfcRenderBase::SetCSCParams(PSFC_CSC_PARAMS cscParams)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(cscParams);

    if (mhw::sfc::SFC_PIPE_MODE_VEBOX == m_pipeMode)
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
    m_renderData.bCSC           = IsCscNeeded(*cscParams);
    m_renderData.SfcInputCspace = cscParams->inputColorSpace;
    m_renderData.SfcInputFormat = cscParams->inputFormat;

    m_renderData.sfcStateParams->bRGBASwapEnable = IsOutputChannelSwapNeeded(cscParams->outputFormat);
    m_renderData.sfcStateParams->bInputColorSpace = cscParams->isInputColorSpaceRGB;
    m_renderData.sfcStateParams->isFullRgbG10P709 = cscParams->isFullRgbG10P709;
    m_renderData.sfcStateParams->isDemosaicEnabled = cscParams->isDemosaicNeeded;

    // Dithering parameter
    if (cscParams->isDitheringNeeded && !m_disableSfcDithering)
    {
        m_renderData.sfcStateParams->ditheringEn = true;
    }
    else
    {
        m_renderData.sfcStateParams->ditheringEn = false;
    }

    // Chromasitting config
    // VEBOX use polyphase coefficients for 1x scaling for better quality,
    // VDBOX dosen't use polyphase coefficients.
    if (mhw::sfc::SFC_PIPE_MODE_VEBOX == m_pipeMode)
    {
        m_renderData.bForcePolyPhaseCoefs = cscParams->bChromaUpSamplingEnable;
    }
    else
    {
        m_renderData.bForcePolyPhaseCoefs = false;
    }
    m_renderData.SfcSrcChromaSiting = cscParams->sfcSrcChromaSiting;

    // 8-Tap chroma filter enabled or not
    m_renderData.sfcStateParams->b8tapChromafiltering = cscParams->b8tapChromafiltering;

    // config SFC chroma down sampling
    m_renderData.sfcStateParams->dwChromaDownSamplingHorizontalCoef = cscParams->chromaDownSamplingHorizontalCoef;
    m_renderData.sfcStateParams->dwChromaDownSamplingVerticalCoef   = cscParams->chromaDownSamplingVerticalCoef;

    VP_RENDER_NORMALMESSAGE("cscParams.isDitheringNeeded %d, m_disableSfcDithering %d, ditheringEn %d, bForcePolyPhaseCoefs %d", 
        cscParams->isDitheringNeeded, m_disableSfcDithering, m_renderData.sfcStateParams->ditheringEn,
        (m_renderData.bForcePolyPhaseCoefs ? 1 : 0));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SfcRenderBase::SetRotMirParams(PSFC_ROT_MIR_PARAMS rotMirParams)
{

    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(rotMirParams);

    if (mhw::sfc::SFC_PIPE_MODE_VEBOX != m_pipeMode      &&
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

MOS_STATUS SfcRenderBase::SetMmcParams(PMOS_SURFACE renderTarget, bool isFormatMmcSupported, bool isMmcEnabled)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(renderTarget);
    VP_PUBLIC_CHK_NULL_RETURN(m_renderData.sfcStateParams);

    if (renderTarget->CompressionMode               &&
        isFormatMmcSupported                        &&
        renderTarget->TileType == MOS_TILE_Y        &&
        isMmcEnabled)
    {
        m_renderData.sfcStateParams->bMMCEnable = true;
        m_renderData.sfcStateParams->MMCMode    = renderTarget->CompressionMode;

        if (renderTarget->OsResource.bUncompressedWriteNeeded)
        {
            m_renderData.sfcStateParams->MMCMode = MOS_MMC_RC;
        }
    }
    else
    {
        m_renderData.sfcStateParams->bMMCEnable = false;
        m_renderData.sfcStateParams->MMCMode    = MOS_MMC_DISABLED;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SfcRenderBase::SetSfcStateInputChromaSubSampling(
    mhw::sfc::SFC_STATE_PAR       *sfcStateParams)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(sfcStateParams);
    VPHAL_COLORPACK colorPack = VpHalDDIUtils::GetSurfaceColorPack(m_renderData.SfcInputFormat);
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
    mhw::sfc::SFC_STATE_PAR       *sfcStateParams)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(sfcStateParams);

    if (m_bVdboxToSfc)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(SetSfcStateInputOrderingModeVdbox(sfcStateParams));
    }
    else if (mhw::sfc::SFC_PIPE_MODE_VEBOX == m_pipeMode)
    {
        if (m_renderData.sfcStateParams && m_renderData.sfcStateParams->isDemosaicEnabled)
        {
            sfcStateParams->dwVDVEInputOrderingMode = MEDIASTATE_SFC_INPUT_ORDERING_VE_4x4;
        }
        else
        {
            sfcStateParams->dwVDVEInputOrderingMode = MEDIASTATE_SFC_INPUT_ORDERING_VE_4x8;
        }
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
    mhw::sfc::SFC_STATE_PAR       *sfcStateParams)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(sfcStateParams);
    switch (m_videoConfig.codecStandard)
    {
    case CODECHAL_VC1:
        sfcStateParams->dwVDVEInputOrderingMode = MEDIASTATE_SFC_INPUT_ORDERING_VD_16x16_NOSHIFT;
        break;
    case CODECHAL_AVC:
        sfcStateParams->dwVDVEInputOrderingMode = m_videoConfig.avc.deblockingEnabled ? MEDIASTATE_SFC_INPUT_ORDERING_VD_16x16_SHIFT :
            MEDIASTATE_SFC_INPUT_ORDERING_VD_16x16_NOSHIFT;
        break;
    case CODECHAL_VP8:
        sfcStateParams->dwVDVEInputOrderingMode = m_videoConfig.vp8.deblockingEnabled ? MEDIASTATE_SFC_INPUT_ORDERING_VD_16x16_SHIFT :
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
    mhw::sfc::SFC_STATE_PAR       *sfcStateParams)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(sfcStateParams);
    if (CODECHAL_JPEG != m_videoConfig.codecStandard)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }
    switch (m_videoConfig.jpeg.jpegChromaType)
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

MOS_STATUS SfcRenderBase::SetSfcStateInputOrderingModeHcp(
    mhw::sfc::SFC_STATE_PAR* pSfcStateParams)
{
    return MOS_STATUS_UNIMPLEMENTED;
}

MOS_STATUS SfcRenderBase::InitMhwOutSurfParams(
  PVP_SURFACE                               pSfcPipeOutSurface,
  PMHW_SFC_OUT_SURFACE_PARAMS               pMhwOutSurfParams)
{
    VP_FUNC_CALL();

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

        pMhwOutSurfParams->dwVUoffset = ((pSfcPipeOutSurface->osSurface->VPlaneOffset.iSurfaceOffset - pSfcPipeOutSurface->osSurface->UPlaneOffset.iSurfaceOffset)
              / pSfcPipeOutSurface->osSurface->dwPitch) + pSfcPipeOutSurface->osSurface->VPlaneOffset.iYOffset;
    }

    return eStatus;
}

MOS_STATUS SfcRenderBase::SendSfcCmd(
    bool                            bOutputToMemory,
    PMOS_COMMAND_BUFFER             pCmdBuffer)
{
    VP_FUNC_CALL();

    mhw::sfc::SFC_LOCK_PAR          SfcLockParams;
    MOS_STATUS                      eStatus;
    MHW_SFC_OUT_SURFACE_PARAMS      OutSurfaceParam;

    VP_RENDER_CHK_NULL_RETURN(pCmdBuffer);

    eStatus                 = MOS_STATUS_SUCCESS;

    SfcLockParams.sfcPipeMode = m_pipeMode;
    SfcLockParams.bOutputToMemory = bOutputToMemory;

    // Send SFC_LOCK command to acquire SFC pipe for Vebox
    VP_RENDER_CHK_STATUS_RETURN(AddSfcLock(
        pCmdBuffer,
        &SfcLockParams));

    VP_RENDER_CHK_STATUS_RETURN(InitMhwOutSurfParams(
        m_renderData.pSfcPipeOutSurface,
        &OutSurfaceParam));

    // Send SFC_STATE command
    VP_RENDER_CHK_STATUS_RETURN(AddSfcState(
        pCmdBuffer,
        m_renderData.sfcStateParams,
        &OutSurfaceParam));

    // Send SFC_AVS_STATE command
    VP_RENDER_CHK_STATUS_RETURN(AddSfcAvsState(
        pCmdBuffer));

    if (m_renderData.bScaling ||
        m_renderData.bForcePolyPhaseCoefs)
    {
        // Send SFC_AVS_LUMA_TABLE command
        VP_RENDER_CHK_STATUS_RETURN(AddSfcAvsLumaTable(
            pCmdBuffer));

        // Send SFC_AVS_CHROMA_TABLE command
        VP_RENDER_CHK_STATUS_RETURN(AddSfcAvsChromaTable(
            pCmdBuffer));
    }

    // Send SFC_IEF_STATE command
    if (m_renderData.bIEF || m_renderData.bCSC)
    {
        // Will modified when enable IEF/CSC
        VP_RENDER_CHK_STATUS_RETURN(AddSfcIefState(
            pCmdBuffer));
        //SETPAR_AND_ADDCMD(SFC_IEF_STATE, sfcItf, pCmdBuffer);
    }

    // Send SFC_FRAME_START command to start processing a frame
    VP_RENDER_CHK_STATUS_RETURN(AddSfcFrameStart(
        pCmdBuffer,
        m_pipeMode));

    return eStatus;
}

void SfcRenderBase::InitAVSParams(
    PMHW_AVS_PARAMS     pAVS_Params,
    uint32_t            uiYCoeffTableSize,
    uint32_t            uiUVCoeffTableSize)
{

    VP_FUNC_CALL();

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
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_NO_STATUS_RETURN(pAVS_Params);
    MOS_SafeFreeMemory(pAVS_Params->piYCoefsX);
    pAVS_Params->piYCoefsX = nullptr;
}

uint32_t SfcRenderBase::GetAvsLineBufferSize(bool lineTiledBuffer, bool b8tapChromafiltering, uint32_t width, uint32_t height)
{
    VP_FUNC_CALL();

    uint32_t size = 0;
    uint32_t linebufferSizePerPixel = 0;

    if (mhw::sfc::SFC_PIPE_MODE_VDBOX == m_pipeMode)
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
    if (mhw::sfc::SFC_PIPE_MODE_VEBOX == m_pipeMode)
    {
        size = height * linebufferSizePerPixel;
    }
    else
    {
        // Align width to 8 for AVS buffer size compute according to VDBOX SFC requirement.
        size = MOS_ALIGN_CEIL(width, 8) * linebufferSizePerPixel;
    }

    // For tile column storage, based on above calcuation, an extra 1K CL need to be added as a buffer.
    // size == 0 means line buffer not needed.
    if (lineTiledBuffer && size > 0)
    {
        size += 1024 * MHW_SFC_CACHELINE_SIZE;
    }

    return size;
}

uint32_t SfcRenderBase::GetIefLineBufferSize(bool lineTiledBuffer, uint32_t heightOutput)
{
    VP_FUNC_CALL();

    uint32_t size = 0;

    // For VE+SFC mode, height needs be used.
    if (mhw::sfc::SFC_PIPE_MODE_VEBOX == m_pipeMode)
    {
        size = heightOutput * SFC_IEF_LINEBUFFER_SIZE_PER_VERTICAL_PIXEL;
    }
    else
    {
        return 0;
    }

    // For tile column storage, based on above calcuation, an extra 1K CL need to be added as a buffer.
    // size == 0 means line buffer not needed.
    if (lineTiledBuffer && size > 0)
    {
        size += 1024 * MHW_SFC_CACHELINE_SIZE;
    }

    return size;
}

uint32_t SfcRenderBase::GetSfdLineBufferSize(bool lineTiledBuffer, MOS_FORMAT formatOutput, uint32_t widthOutput, uint32_t heightOutput)
{
    VP_FUNC_CALL();

    int size = 0;

    // For VD+SFC mode, width needs be used. For VE+SFC mode, height needs be used.
    if (mhw::sfc::SFC_PIPE_MODE_VEBOX == m_pipeMode)
    {
        size = (VPHAL_COLORPACK_444 == VpHalDDIUtils::GetSurfaceColorPack(formatOutput)) ? 0 : (heightOutput * SFC_SFD_LINEBUFFER_SIZE_PER_PIXEL);
    }
    else
    {
        size = MOS_ROUNDUP_DIVIDE(widthOutput, 10) * SFC_CACHELINE_SIZE_IN_BYTES;
        size *= 2; //double for safe
    }

    // For tile column storage, based on above calcuation, an extra 1K CL need to be added as a buffer.
    // size == 0 means line buffer not needed.
    if (lineTiledBuffer && size > 0)
    {
        size += 1024 * MHW_SFC_CACHELINE_SIZE;
    }

    return size;
}

MOS_STATUS SfcRenderBase::AllocateLineBuffer(VP_SURFACE *&lineBuffer, uint32_t size, const char *bufName)
{
    VP_FUNC_CALL();

    bool                allocated           = false;
    MEDIA_FEATURE_TABLE *skuTable           = nullptr;
    Mos_MemPool         memTypeSurfVideoMem = MOS_MEMPOOL_VIDEOMEMORY;

    VP_PUBLIC_CHK_NULL_RETURN(m_osInterface)

    skuTable = m_osInterface->pfnGetSkuTable(m_osInterface);

    if (skuTable && MEDIA_IS_SKU(skuTable, FtrLimitedLMemBar))
    {
        memTypeSurfVideoMem = MOS_MEMPOOL_DEVICEMEMORY;
    }

    if (size)
    {
        VP_RENDER_CHK_STATUS_RETURN(m_allocator->ReAllocateSurface(
                                      lineBuffer,
                                      bufName,
                                      Format_Buffer,
                                      MOS_GFXRES_BUFFER,
                                      MOS_TILE_LINEAR,
                                      size,
                                      1,
                                      false,
                                      MOS_MMC_DISABLED,
                                      allocated,
                                      false,
                                      true,
                                      MOS_HW_RESOURCE_USAGE_VP_INTERNAL_READ_WRITE_FF,
                                      MOS_TILE_UNSET_GMM,
                                      memTypeSurfVideoMem,
                                      VPP_INTER_RESOURCE_NOTLOCKABLE));
    }
    else if (lineBuffer)
    {
        VP_RENDER_CHK_STATUS_RETURN(m_allocator->DestroyVpSurface(lineBuffer));
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SfcRenderBase::AllocateLineBufferArray(VP_SURFACE **&lineBufferArray, uint32_t size, const char *bufName)
{
    VP_FUNC_CALL();

    bool allocated = false;
    VP_RENDER_CHK_NULL_RETURN(lineBufferArray);
    // Use numPipe instead of m_lineBufferAllocatedInArray to only allocate surface in use.
    for (int32_t i = 0; i < m_scalabilityParams.numPipe; ++i)
    {
        VP_RENDER_CHK_STATUS_RETURN(AllocateLineBuffer(lineBufferArray[i], size, bufName));
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SfcRenderBase::AllocateResources()
{
    VP_FUNC_CALL();

    uint32_t                size;

    VP_RENDER_CHK_NULL_RETURN(m_allocator);
    VP_RENDER_CHK_NULL_RETURN(m_renderData.sfcStateParams);

    // for 1st pass of Sfc 2Pass case, use the standalone line buffer array to avoid line buffer reallocation
    if (m_renderData.b1stPassOfSfc2PassScaling)
    {
        if (m_scalabilityParams.numPipe > m_lineBufferAllocatedInArrayfor1stPassofSfc2Pass    ||
            nullptr == m_AVSLineBufferSurfaceArrayfor1stPassofSfc2Pass      ||
            nullptr == m_IEFLineBufferSurfaceArrayfor1stPassofSfc2Pass      ||
            nullptr == m_SFDLineBufferSurfaceArrayfor1stPassofSfc2Pass)
        {
            DestroyLineBufferArray(m_AVSLineBufferSurfaceArrayfor1stPassofSfc2Pass, m_lineBufferAllocatedInArrayfor1stPassofSfc2Pass);
            DestroyLineBufferArray(m_IEFLineBufferSurfaceArrayfor1stPassofSfc2Pass, m_lineBufferAllocatedInArrayfor1stPassofSfc2Pass);
            DestroyLineBufferArray(m_SFDLineBufferSurfaceArrayfor1stPassofSfc2Pass, m_lineBufferAllocatedInArrayfor1stPassofSfc2Pass);
            m_lineBufferAllocatedInArrayfor1stPassofSfc2Pass = m_scalabilityParams.numPipe;
            m_AVSLineBufferSurfaceArrayfor1stPassofSfc2Pass = MOS_NewArray(VP_SURFACE*, m_lineBufferAllocatedInArrayfor1stPassofSfc2Pass);
            VP_RENDER_CHK_NULL_RETURN(m_AVSLineBufferSurfaceArrayfor1stPassofSfc2Pass);
            m_IEFLineBufferSurfaceArrayfor1stPassofSfc2Pass = MOS_NewArray(VP_SURFACE*, m_lineBufferAllocatedInArrayfor1stPassofSfc2Pass);
            VP_RENDER_CHK_NULL_RETURN(m_IEFLineBufferSurfaceArrayfor1stPassofSfc2Pass);
            m_SFDLineBufferSurfaceArrayfor1stPassofSfc2Pass = MOS_NewArray(VP_SURFACE*, m_lineBufferAllocatedInArrayfor1stPassofSfc2Pass);
            VP_RENDER_CHK_NULL_RETURN(m_SFDLineBufferSurfaceArrayfor1stPassofSfc2Pass);
        }

        // for AVSLineBuffer, IEFLineBuffer and SFDLineBuffer, they are only needed when surface allocation bigger than 4150.
        // for AVSLineTileBuffer, IEFLineTileBuffer and SFDLineTileBuffer, they are only needed for VdBox SFC scalability case and not needed for VeBox SFC case.

        // Allocate AVS Line Buffer surface----------------------------------------------
        size = GetAvsLineBufferSize(false, m_renderData.sfcStateParams->b8tapChromafiltering, m_renderData.sfcStateParams->dwInputFrameWidth, m_renderData.sfcStateParams->dwInputFrameHeight);
        VP_RENDER_CHK_STATUS_RETURN(AllocateLineBufferArray(m_AVSLineBufferSurfaceArrayfor1stPassofSfc2Pass, size, "SfcAVSLineBufferSurfacefor1stPassofSfc2Pass"));

        // Allocate IEF Line Buffer surface----------------------------------------------
        size = GetIefLineBufferSize(false, m_renderData.sfcStateParams->dwScaledRegionHeight);
        VP_RENDER_CHK_STATUS_RETURN(AllocateLineBufferArray(m_IEFLineBufferSurfaceArrayfor1stPassofSfc2Pass, size, "SfcIEFLineBufferSurfacefor1stPassofSfc2Pass"));

        if (m_bVdboxToSfc || m_renderData.sfcStateParams->dwScaledRegionHeight > SFC_LINEBUFEER_SIZE_LIMITED)
        {
            // Allocate SFD Line Buffer surface
            size = GetSfdLineBufferSize(false, m_renderData.sfcStateParams->OutputFrameFormat, m_renderData.sfcStateParams->dwScaledRegionWidth, m_renderData.sfcStateParams->dwScaledRegionHeight);
            VP_RENDER_CHK_STATUS_RETURN(AllocateLineBufferArray(m_SFDLineBufferSurfaceArrayfor1stPassofSfc2Pass, size, "SfcSFDLineBufferSurfacefor1stPassofSfc2Pass"));
        }
    }
    else
    {
        if (m_scalabilityParams.numPipe > m_lineBufferAllocatedInArray    ||
            nullptr == m_AVSLineBufferSurfaceArray      ||
            nullptr == m_IEFLineBufferSurfaceArray      ||
            nullptr == m_SFDLineBufferSurfaceArray)
        {
            DestroyLineBufferArray(m_AVSLineBufferSurfaceArray, m_lineBufferAllocatedInArray);
            DestroyLineBufferArray(m_IEFLineBufferSurfaceArray, m_lineBufferAllocatedInArray);
            DestroyLineBufferArray(m_SFDLineBufferSurfaceArray, m_lineBufferAllocatedInArray);
            m_lineBufferAllocatedInArray = m_scalabilityParams.numPipe;
            m_AVSLineBufferSurfaceArray = MOS_NewArray(VP_SURFACE*, m_lineBufferAllocatedInArray);
            VP_RENDER_CHK_NULL_RETURN(m_AVSLineBufferSurfaceArray);
            m_IEFLineBufferSurfaceArray = MOS_NewArray(VP_SURFACE*, m_lineBufferAllocatedInArray);
            VP_RENDER_CHK_NULL_RETURN(m_IEFLineBufferSurfaceArray);
            m_SFDLineBufferSurfaceArray = MOS_NewArray(VP_SURFACE*, m_lineBufferAllocatedInArray);
            VP_RENDER_CHK_NULL_RETURN(m_SFDLineBufferSurfaceArray);
        }

        // for AVSLineBuffer, IEFLineBuffer and SFDLineBuffer, they are only needed when surface allocation bigger than 4150.
        // for AVSLineTileBuffer, IEFLineTileBuffer and SFDLineTileBuffer, they are only needed for VdBox SFC scalability case and not needed for VeBox SFC case.

        // Allocate AVS Line Buffer surface----------------------------------------------
        size = GetAvsLineBufferSize(false, m_renderData.sfcStateParams->b8tapChromafiltering, m_renderData.sfcStateParams->dwInputFrameWidth, m_renderData.sfcStateParams->dwInputFrameHeight);
        VP_RENDER_CHK_STATUS_RETURN(AllocateLineBufferArray(m_AVSLineBufferSurfaceArray, size, "SfcAVSLineBufferSurface"));

        // Allocate IEF Line Buffer surface----------------------------------------------
        size = GetIefLineBufferSize(false, m_renderData.sfcStateParams->dwScaledRegionHeight);
        VP_RENDER_CHK_STATUS_RETURN(AllocateLineBufferArray(m_IEFLineBufferSurfaceArray, size, "SfcIEFLineBufferSurface"));

        if (m_bVdboxToSfc || m_renderData.sfcStateParams->dwScaledRegionHeight > SFC_LINEBUFEER_SIZE_LIMITED)
        {
            // Allocate SFD Line Buffer surface
            size = GetSfdLineBufferSize(false, m_renderData.sfcStateParams->OutputFrameFormat, m_renderData.sfcStateParams->dwScaledRegionWidth, m_renderData.sfcStateParams->dwScaledRegionHeight);
            VP_RENDER_CHK_STATUS_RETURN(AllocateLineBufferArray(m_SFDLineBufferSurfaceArray, size, "SfcSFDLineBufferSurface"));
        }
    }

    if (m_bVdboxToSfc)
    {
        // Allocate AVS Line Tile Buffer surface----------------------------------------------
        size = GetAvsLineBufferSize(true, m_renderData.sfcStateParams->b8tapChromafiltering, m_renderData.sfcStateParams->dwInputFrameWidth, m_renderData.sfcStateParams->dwInputFrameHeight);
        VP_RENDER_CHK_STATUS_RETURN(AllocateLineBuffer(m_AVSLineTileBufferSurface, size, "SfcAVSLineTileBufferSurface"));

        // Allocate IEF Line Tile Buffer surface----------------------------------------------
        size = GetIefLineBufferSize(true, m_renderData.sfcStateParams->dwScaledRegionHeight);
        VP_RENDER_CHK_STATUS_RETURN(AllocateLineBuffer(m_IEFLineTileBufferSurface, size, "SfcIEFLineTileBufferSurface"));

        // Allocate SFD Line Tile Buffer surface
        size = GetSfdLineBufferSize(true, m_renderData.sfcStateParams->OutputFrameFormat, m_renderData.sfcStateParams->dwScaledRegionWidth, m_renderData.sfcStateParams->dwScaledRegionHeight);
        VP_RENDER_CHK_STATUS_RETURN(AllocateLineBuffer(m_SFDLineTileBufferSurface, size, "SfcSFDLineTileBufferSurface"));
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SfcRenderBase::DestroyLineBufferArray(VP_SURFACE **&lineBufferArray, int32_t count)
{
    VP_FUNC_CALL();

    if (nullptr == lineBufferArray)
    {
        return MOS_STATUS_SUCCESS;
    }
    // Use count instead of numPipe to destroy all surfaces.
    for (int32_t i = 0; i < count; ++i)
    {
        if (lineBufferArray[i])
        {
            m_allocator->DestroyVpSurface(lineBufferArray[i]);
        }
    }
    MOS_DeleteArray(lineBufferArray);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SfcRenderBase::FreeResources()
{
    VP_FUNC_CALL();

    VP_RENDER_CHK_NULL_RETURN(m_allocator);
    // Free AVS Line Buffer surface for SFC
    DestroyLineBufferArray(m_AVSLineBufferSurfaceArray, m_lineBufferAllocatedInArray);

    // Free IEF Line Buffer surface for SFC
    DestroyLineBufferArray(m_IEFLineBufferSurfaceArray, m_lineBufferAllocatedInArray);

    // Free SFD Line Buffer surface for SFC
    DestroyLineBufferArray(m_SFDLineBufferSurfaceArray, m_lineBufferAllocatedInArray);

    // Free AVS Line Tile Buffer surface for SFC
    m_allocator->DestroyVpSurface(m_AVSLineTileBufferSurface);

    // Free IEF Line Tile Buffer surface for SFC
    m_allocator->DestroyVpSurface(m_IEFLineTileBufferSurface);

    // Free SFD Line Tile Buffer surface for SFC
    m_allocator->DestroyVpSurface(m_SFDLineTileBufferSurface);

    // Free AVS Line Buffer surface for SFC 1st Pass of Sfc 2Pass
    DestroyLineBufferArray(m_AVSLineBufferSurfaceArrayfor1stPassofSfc2Pass, m_lineBufferAllocatedInArrayfor1stPassofSfc2Pass);

    // Free IEF Line Buffer surface for SFC 1st Pass of Sfc 2Pass
    DestroyLineBufferArray(m_IEFLineBufferSurfaceArrayfor1stPassofSfc2Pass, m_lineBufferAllocatedInArrayfor1stPassofSfc2Pass);

    // Free SFD Line Buffer surface for SFC 1st Pass of Sfc 2Pass
    DestroyLineBufferArray(m_SFDLineBufferSurfaceArrayfor1stPassofSfc2Pass, m_lineBufferAllocatedInArrayfor1stPassofSfc2Pass);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SfcRenderBase::SetHistogramBuf(PMOS_BUFFER histogramBuf)
{
    VP_FUNC_CALL();

    if (histogramBuf != nullptr)
    {
        m_histogramSurf.OsResource = histogramBuf->OsResource;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SfcRenderBase::AddSfcLock(
    PMOS_COMMAND_BUFFER            pCmdBuffer,
    mhw::sfc::SFC_LOCK_PAR         *sfcLockParams)
{
    VP_FUNC_CALL();

    VP_RENDER_CHK_NULL_RETURN(m_sfcItf);

    auto& params = m_sfcItf->MHW_GETPAR_F(SFC_LOCK)();
    params = *sfcLockParams;

    // Send SFC_LOCK command to acquire SFC pipe for Vebox
    VP_RENDER_CHK_STATUS_RETURN(m_sfcItf->MHW_ADDCMD_F(SFC_LOCK)(pCmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SfcRenderBase::AddSfcState(
    PMOS_COMMAND_BUFFER            pCmdBuffer,
    mhw::sfc::SFC_STATE_PAR        *pSfcStateParams,
    PMHW_SFC_OUT_SURFACE_PARAMS    pOutSurface)
{
    VP_FUNC_CALL();

    VP_RENDER_CHK_NULL_RETURN(pSfcStateParams);
    VP_RENDER_CHK_NULL_RETURN(m_sfcItf);

    auto& params = m_sfcItf->MHW_GETPAR_F(SFC_STATE)();
    params = {};
    params = *pSfcStateParams;
    params.pOutSurface = pOutSurface;

    // Send SFC_LOCK command to acquire SFC pipe for Vebox
    VP_RENDER_CHK_STATUS_RETURN(m_sfcItf->MHW_ADDCMD_F(SFC_STATE)(pCmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SfcRenderBase::AddSfcAvsState(
    PMOS_COMMAND_BUFFER             pCmdBuffer)
{
    VP_FUNC_CALL();
    VP_RENDER_CHK_NULL_RETURN(m_sfcItf);

    VP_RENDER_CHK_STATUS_RETURN(m_sfcItf->MHW_ADDCMD_F(SFC_AVS_STATE)(pCmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SfcRenderBase::AddSfcIefState(
    PMOS_COMMAND_BUFFER            pCmdBuffer)
{
    VP_FUNC_CALL();
    VP_RENDER_CHK_NULL_RETURN(m_sfcItf);

    VP_RENDER_CHK_STATUS_RETURN(m_sfcItf->MHW_ADDCMD_F(SFC_IEF_STATE)(pCmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SfcRenderBase::AddSfcAvsLumaTable(
    PMOS_COMMAND_BUFFER                pCmdBuffer)
{
    VP_FUNC_CALL();

    VP_RENDER_CHK_NULL_RETURN(m_sfcItf);

    VP_RENDER_CHK_STATUS_RETURN(m_sfcItf->MHW_ADDCMD_F(SFC_AVS_LUMA_Coeff_Table)(pCmdBuffer));
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SfcRenderBase::AddSfcAvsChromaTable(
    PMOS_COMMAND_BUFFER             pCmdBuffer)
{
    VP_FUNC_CALL();
    VP_RENDER_CHK_NULL_RETURN(m_sfcItf);

    VP_RENDER_CHK_STATUS_RETURN(m_sfcItf->MHW_ADDCMD_F(SFC_AVS_CHROMA_Coeff_Table)(pCmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SfcRenderBase::AddSfcFrameStart(
    PMOS_COMMAND_BUFFER            pCmdBuffer,
    uint8_t                        sfcPipeMode)
{
    VP_FUNC_CALL();
    VP_RENDER_CHK_NULL_RETURN(m_sfcItf);

    auto& SfcFrameStartParams = m_sfcItf->MHW_GETPAR_F(SFC_FRAME_START)();
    SfcFrameStartParams = {};
    SfcFrameStartParams.sfcPipeMode = m_pipeMode;
    VP_RENDER_CHK_STATUS_RETURN(m_sfcItf->MHW_ADDCMD_F(SFC_FRAME_START)(pCmdBuffer));
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SfcRenderBase::SetSfcAVSScalingMode(
    MHW_SCALING_MODE  ScalingMode)
{
    VP_FUNC_CALL();

    MOS_STATUS                      eStatus;

    eStatus = MOS_STATUS_SUCCESS;
    VP_RENDER_CHK_NULL_RETURN(m_sfcItf);

    // Send SFC_FRAME_START command to start processing a frame
    VP_RENDER_CHK_STATUS_RETURN(m_sfcItf->SetSfcAVSScalingMode(
        ScalingMode));

    return eStatus;
}

MOS_STATUS SfcRenderBase::SetSfcSamplerTable(
    mhw::sfc::SFC_AVS_LUMA_Coeff_Table_PAR   *pLumaTable,
    mhw::sfc::SFC_AVS_CHROMA_Coeff_Table_PAR *pChromaTable,
    PMHW_AVS_PARAMS                 pAvsParams,
    MOS_FORMAT                      SrcFormat,
    float                           fScaleX,
    float                           fScaleY,
    uint32_t                        dwChromaSiting,
    bool                            bUse8x8Filter,
    float                           fHPStrength,
    float                           fLanczosT)
{
    VP_FUNC_CALL();

    MOS_STATUS                      eStatus;

    eStatus = MOS_STATUS_SUCCESS;
    VP_RENDER_CHK_NULL_RETURN(m_sfcItf);
    VP_RENDER_CHK_NULL_RETURN(pLumaTable);
    VP_RENDER_CHK_NULL_RETURN(pChromaTable);
    VP_RENDER_CHK_NULL_RETURN(pAvsParams);

    // Send SFC_FRAME_START command to start processing a frame
    VP_RENDER_CHK_STATUS_RETURN(m_sfcItf->SetSfcSamplerTable(
        pLumaTable,
        pChromaTable,
        pAvsParams,
        SrcFormat,
        fScaleX,
        fScaleY,
        dwChromaSiting,
        bUse8x8Filter,
        fHPStrength,
        fLanczosT));

    return eStatus;
}
}
