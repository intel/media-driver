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
//! \file     vp_render_sfc_base.cpp
//! \brief    SFC rendering component
//! \details  The SFC renderer supports Scaling, IEF, CSC/ColorFill and Rotation.
//!           It's responsible for setting up HW states and generating the SFC
//!           commands.
//!

#include "vp_render_sfc_base_legacy.h"
#include "vp_utils.h"
#include "mhw_vebox.h"
#include "mhw_sfc.h"
#include "vp_render_ief.h"
#include "vp_hal_ddi_utils.h"
#include "mos_interface.h"

namespace vp {

    SfcRenderBaseLegacy::SfcRenderBaseLegacy(
    VP_MHWINTERFACE &vpMhwinterface,
    PVpAllocator &allocator,
    bool disbaleSfcDithering) :
    SfcRenderBase(vpMhwinterface, allocator, disbaleSfcDithering)
{
    VP_PUBLIC_CHK_NULL_NO_STATUS_RETURN(vpMhwinterface.m_sfcInterface);
    VP_PUBLIC_CHK_NULL_NO_STATUS_RETURN(vpMhwinterface.m_mhwMiInterface);

    m_miInterface   = vpMhwinterface.m_mhwMiInterface;
    m_sfcInterface  = vpMhwinterface.m_sfcInterface;
}

    SfcRenderBaseLegacy::~SfcRenderBaseLegacy()
{
    if (m_sfcStateParamsLegacy)
    {
        MOS_FreeMemAndSetNull(m_sfcStateParamsLegacy);
    }
}

MOS_STATUS SfcRenderBaseLegacy::Init()
{
    VP_FUNC_CALL();

    MOS_ZeroMemory(&m_renderDataLegacy, sizeof(m_renderDataLegacy));

    m_pipeMode = MhwSfcInterface::SFC_PIPE_MODE_VEBOX;

    m_scalabilityParams.numPipe = 1;
    m_scalabilityParams.curPipe = 0;

    MOS_ZeroMemory(&m_histogramSurf, sizeof(m_histogramSurf));

    return InitSfcStateParams();
}

MOS_STATUS SfcRenderBaseLegacy::Init(VIDEO_PARAMS& videoParams)
{
    VP_FUNC_CALL();

    MOS_ZeroMemory(&m_renderDataLegacy, sizeof(m_renderDataLegacy));
    //InitRenderData();
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


void SfcRenderBaseLegacy::SetRotationAndMirrowParams(PMHW_SFC_STATE_PARAMS psfcStateParams)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_NO_STATUS_RETURN(psfcStateParams);

    psfcStateParams->RotationMode  = (MHW_ROTATION)m_renderDataLegacy.SfcRotation;
    psfcStateParams->bMirrorEnable = m_renderDataLegacy.bMirrorEnable;
    psfcStateParams->dwMirrorType  = m_renderDataLegacy.mirrorType;
}

void SfcRenderBaseLegacy::SetChromasitingParams(PMHW_SFC_STATE_PARAMS psfcStateParams)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_NO_STATUS_RETURN(psfcStateParams);
    SetSfcStateInputChromaSubSampling(psfcStateParams);
    SetSfcStateInputOrderingMode(psfcStateParams);
}

void SfcRenderBaseLegacy::SetColorFillParams(
    PMHW_SFC_STATE_PARAMS psfcStateParams)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_NO_STATUS_RETURN(m_renderDataLegacy.pColorFillParams);

    psfcStateParams->bColorFillEnable = m_renderDataLegacy.pColorFillParams->bColorfillEnable;

    if (psfcStateParams->bColorFillEnable)
    {
        psfcStateParams->fColorFillYRPixel = m_renderDataLegacy.pColorFillParams->fColorFillYRPixel;
        psfcStateParams->fColorFillUGPixel = m_renderDataLegacy.pColorFillParams->fColorFillUGPixel;
        psfcStateParams->fColorFillVBPixel = m_renderDataLegacy.pColorFillParams->fColorFillVBPixel;
        psfcStateParams->fColorFillAPixel  = m_renderDataLegacy.pColorFillParams->fColorFillAPixel;
    }
}

void SfcRenderBaseLegacy::SetXYAdaptiveFilter(
    PMHW_SFC_STATE_PARAMS psfcStateParams)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_NO_STATUS_RETURN(psfcStateParams);

    // Enable Adaptive Filtering for YUV input only, if it is being upscaled
    // in either direction. We must check for this before clamping the SF.
    if ((IS_YUV_FORMAT(m_renderDataLegacy.SfcInputFormat) ||
            m_renderDataLegacy.SfcInputFormat == Format_AYUV) &&
        (m_renderDataLegacy.fScaleX > 1.0F ||
            m_renderDataLegacy.fScaleY > 1.0F) &&
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

void SfcRenderBaseLegacy::SetRGBAdaptive(
    PMHW_SFC_STATE_PARAMS psfcStateParams)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_NO_STATUS_RETURN(psfcStateParams);

    if (IS_RGB_FORMAT(m_renderDataLegacy.SfcInputFormat) &&
        psfcStateParams->b8tapChromafiltering == true)
    {
        psfcStateParams->bRGBAdaptive = true;
    }
    else
    {
        psfcStateParams->bRGBAdaptive = false;
    }
}

MOS_STATUS SfcRenderBaseLegacy::SetIefStateCscParams(
    PMHW_SFC_STATE_PARAMS     psfcStateParams,
    PMHW_SFC_IEF_STATE_PARAMS pIEFStateParams)
{
    VP_FUNC_CALL();

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    VP_RENDER_CHK_NULL_RETURN(psfcStateParams);
    VP_RENDER_CHK_NULL_RETURN(pIEFStateParams);

    if (m_renderDataLegacy.bCSC)
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
                if (m_renderDataLegacy.SfcInputFormat != Format_400P)
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
        else if ((m_cscInputCspace != m_renderDataLegacy.SfcInputCspace) ||
                 (m_renderDataLegacy.pSfcPipeOutSurface && m_cscRTCspace != m_renderDataLegacy.pSfcPipeOutSurface->ColorSpace))
        {
            VpUtils::GetCscMatrixForVeSfc8Bit(
                m_renderDataLegacy.SfcInputCspace,
                m_renderDataLegacy.pSfcPipeOutSurface->ColorSpace,
                m_cscCoeff,
                m_cscInOffset,
                m_cscOutOffset);

            // swap the 1st and 3rd columns of the transfer matrix for A8R8G8B8 and X8R8G8B8
            // to ensure SFC input being A8B8G8R8.
            if (IsInputChannelSwapNeeded(m_renderDataLegacy.SfcInputFormat))
            {
                float fTemp[3] = {};
                fTemp[0]       = m_cscCoeff[0];
                fTemp[1]       = m_cscCoeff[3];
                fTemp[2]       = m_cscCoeff[6];

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

            VP_RENDER_NORMALMESSAGE("sfc csc coeff calculated. (sfcInputCspace, cscRTCspace) current (%d, %d), previous (%d, %d) swap flag %d",
                m_renderDataLegacy.SfcInputCspace, m_renderDataLegacy.pSfcPipeOutSurface->ColorSpace,
                m_cscInputCspace, m_cscRTCspace, m_cscInputSwapNeeded ? 1 :0);

            m_cscInputCspace = m_renderDataLegacy.SfcInputCspace;
            m_cscRTCspace    = m_renderDataLegacy.pSfcPipeOutSurface->ColorSpace;
        }
        else if (m_cscInputSwapNeeded != IsInputChannelSwapNeeded(m_renderDataLegacy.SfcInputFormat))
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

            m_cscInputSwapNeeded = IsInputChannelSwapNeeded(m_renderDataLegacy.SfcInputFormat);

            VP_RENDER_NORMALMESSAGE("sfc csc coeff swap flag need be updated to %d. sfcInputFormat %d, sfcInputCspace %d, cscRTCspace %d",
                (m_cscInputSwapNeeded ? 1 : 0),
                m_renderDataLegacy.SfcInputFormat, m_cscInputCspace, m_cscRTCspace);
        }
        else
        {
            VP_RENDER_NORMALMESSAGE("sfc csc coeff reused. sfcInputFormat %d, sfcInputCspace %d, cscRTCspace %d, swap flag %d",
                m_renderDataLegacy.SfcInputFormat, m_cscInputCspace, m_cscRTCspace, (m_cscInputSwapNeeded ? 1 : 0));
        }

        pIEFStateParams->pfCscCoeff     = m_cscCoeff;
        pIEFStateParams->pfCscInOffset  = m_cscInOffset;
        pIEFStateParams->pfCscOutOffset = m_cscOutOffset;
    }
    return eStatus;
}

MOS_STATUS SfcRenderBaseLegacy::SetIefStateParams(
    PMHW_SFC_STATE_PARAMS psfcStateParams)
{
    VP_FUNC_CALL();

    PMHW_SFC_IEF_STATE_PARAMS pIefStateParams = nullptr;
    MOS_STATUS                eStatus         = MOS_STATUS_SUCCESS;

    VP_RENDER_CHK_NULL_RETURN(psfcStateParams);

    pIefStateParams = &m_IefStateParamsLegacy;
    MOS_ZeroMemory(pIefStateParams, sizeof(*pIefStateParams));
    pIefStateParams->sfcPipeMode = m_pipeMode;

    // Setup IEF and STE params
    if (m_renderDataLegacy.bIEF && m_renderDataLegacy.pIefParams)
    {
        VP_RENDER_CHK_NULL_RETURN(m_iefObj);
        m_iefObj->Init(m_renderDataLegacy.pIefParams, m_renderDataLegacy.SfcInputFormat, m_renderDataLegacy.fScaleX, m_renderDataLegacy.fScaleY);
        m_iefObj->SetHwState(psfcStateParams, pIefStateParams);
    }  // end of setup IEF and STE params

    // Setup CSC params
    VP_RENDER_CHK_STATUS_RETURN(SetIefStateCscParams(
        psfcStateParams,
        pIefStateParams));

    return eStatus;
}

MOS_STATUS SfcRenderBaseLegacy::SetAvsStateParams()
{
    VP_FUNC_CALL();

    MOS_STATUS         eStatus       = MOS_STATUS_SUCCESS;
    PMHW_SFC_AVS_STATE pMhwAvsState  = nullptr;
    MHW_SCALING_MODE   scalingMode   = MHW_SCALING_AVS;
    bool               bUse8x8Filter = false;

    pMhwAvsState = &m_avsStateLegacy.AvsStateParams;
    MOS_ZeroMemory(pMhwAvsState, sizeof(MHW_SFC_AVS_STATE));
    pMhwAvsState->sfcPipeMode = m_pipeMode;

    if (m_renderDataLegacy.bScaling ||
        m_renderDataLegacy.bForcePolyPhaseCoefs)
    {
        if (m_renderDataLegacy.SfcSrcChromaSiting == MHW_CHROMA_SITING_NONE)
        {
            if (VpHalDDIUtils::GetSurfaceColorPack(m_renderDataLegacy.SfcInputFormat) == VPHAL_COLORPACK_420)  // For 420, default is Left & Center, else default is Left & Top
            {
                m_renderDataLegacy.SfcSrcChromaSiting = MHW_CHROMA_SITING_HORZ_LEFT | MHW_CHROMA_SITING_VERT_CENTER;
            }
            else
            {
                m_renderDataLegacy.SfcSrcChromaSiting = MHW_CHROMA_SITING_HORZ_LEFT | MHW_CHROMA_SITING_VERT_TOP;
            }
        }

        pMhwAvsState->dwInputHorizontalSiting = (m_renderDataLegacy.SfcSrcChromaSiting & MHW_CHROMA_SITING_HORZ_CENTER) ? SFC_AVS_INPUT_SITING_COEF_4_OVER_8 : ((m_renderDataLegacy.SfcSrcChromaSiting & MHW_CHROMA_SITING_HORZ_RIGHT) ? SFC_AVS_INPUT_SITING_COEF_8_OVER_8 : SFC_AVS_INPUT_SITING_COEF_0_OVER_8);

        pMhwAvsState->dwInputVerticalSitting = (m_renderDataLegacy.SfcSrcChromaSiting & MHW_CHROMA_SITING_VERT_CENTER) ? SFC_AVS_INPUT_SITING_COEF_4_OVER_8 : ((m_renderDataLegacy.SfcSrcChromaSiting & MHW_CHROMA_SITING_VERT_BOTTOM) ? SFC_AVS_INPUT_SITING_COEF_8_OVER_8 : SFC_AVS_INPUT_SITING_COEF_0_OVER_8);

        if (m_renderDataLegacy.SfcScalingMode == VPHAL_SCALING_NEAREST)
        {
            scalingMode = MHW_SCALING_NEAREST;
        }
        else if (m_renderDataLegacy.SfcScalingMode == VPHAL_SCALING_BILINEAR)
        {
            scalingMode = MHW_SCALING_BILINEAR;
        }
        else
        {
            scalingMode = MHW_SCALING_AVS;
        }
        VP_RENDER_CHK_STATUS_RETURN(SetSfcAVSScalingMode(scalingMode));

        if (m_renderDataLegacy.sfcStateParams)
        {
            pMhwAvsState->dwAVSFilterMode = m_renderDataLegacy.sfcStateParams->dwAVSFilterMode;
        }
        else
        {
            pMhwAvsState->dwAVSFilterMode = MEDIASTATE_SFC_AVS_FILTER_8x8;
        }

        if (pMhwAvsState->dwAVSFilterMode == MEDIASTATE_SFC_AVS_FILTER_8x8)
        {
            bUse8x8Filter = true;
        }

        m_avsStateLegacy.LumaCoeffs.sfcPipeMode   = m_pipeMode;
        m_avsStateLegacy.ChromaCoeffs.sfcPipeMode = m_pipeMode;

        VP_RENDER_CHK_STATUS_RETURN(SetSfcSamplerTable(
            &m_avsStateLegacy.LumaCoeffs,
            &m_avsStateLegacy.ChromaCoeffs,
            m_renderDataLegacy.pAvsParams,
            m_renderDataLegacy.SfcInputFormat,
            m_renderDataLegacy.fScaleX,
            m_renderDataLegacy.fScaleY,
            m_renderDataLegacy.SfcSrcChromaSiting,
            bUse8x8Filter,
            0,
            0));
    }

    return eStatus;
}

MOS_STATUS SfcRenderBaseLegacy::SetupSfcState(PVP_SURFACE targetSurface)
{
    VP_FUNC_CALL();

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    VP_RENDER_CHK_NULL_RETURN(targetSurface);
    VP_RENDER_CHK_NULL_RETURN(targetSurface->osSurface);

    //---------------------------------
    // Set SFC State:  common properties
    //---------------------------------
    m_renderDataLegacy.sfcStateParams->sfcPipeMode = (MEDIASTATE_SFC_PIPE_MODE)m_pipeMode;

    m_renderDataLegacy.sfcStateParams->InputFrameFormat       = m_renderDataLegacy.SfcInputFormat;
    m_renderDataLegacy.sfcStateParams->OutputFrameFormat      = targetSurface->osSurface->Format;
    m_renderDataLegacy.sfcStateParams->dwOutputSurfaceOffset  = targetSurface->osSurface->YPlaneOffset.iSurfaceOffset;
    m_renderDataLegacy.sfcStateParams->wOutputSurfaceUXOffset = (uint16_t)targetSurface->osSurface->UPlaneOffset.iXOffset;
    m_renderDataLegacy.sfcStateParams->wOutputSurfaceUYOffset = (uint16_t)targetSurface->osSurface->UPlaneOffset.iYOffset;
    m_renderDataLegacy.sfcStateParams->wOutputSurfaceVXOffset = (uint16_t)targetSurface->osSurface->VPlaneOffset.iXOffset;
    m_renderDataLegacy.sfcStateParams->wOutputSurfaceVYOffset = (uint16_t)targetSurface->osSurface->VPlaneOffset.iYOffset;

    m_renderDataLegacy.pSfcPipeOutSurface = targetSurface;
    m_renderDataLegacy.pAvsParams         = &m_AvsParameters;

    //---------------------------------
    // Set SFC State:  Scaling
    //---------------------------------
    m_AvsParameters.bForcePolyPhaseCoefs = m_renderDataLegacy.bForcePolyPhaseCoefs;
    VP_RENDER_CHK_STATUS_RETURN(SetAvsStateParams());
    m_renderDataLegacy.sfcStateParams->bAVSChromaUpsamplingEnable = m_renderDataLegacy.bScaling ||
                                                              m_renderDataLegacy.bForcePolyPhaseCoefs;

    //---------------------------------
    // Set SFC State:  CSC/IEF
    //---------------------------------
    if (m_renderDataLegacy.bIEF ||
        m_renderDataLegacy.bCSC)
    {
        VP_RENDER_CHK_STATUS_RETURN(SetIefStateParams(
            m_renderDataLegacy.sfcStateParams));
    }

    //---------------------------------
    // Set SFC State: Rotation/Mirror
    //---------------------------------
    SetRotationAndMirrowParams(m_renderDataLegacy.sfcStateParams);

    //---------------------------------
    // Set SFC State:  Chromasiting
    //---------------------------------
    SetChromasitingParams(m_renderDataLegacy.sfcStateParams);

    //---------------------------------
    // Set SFC State:  XY Adaptive Filter
    //---------------------------------
    SetXYAdaptiveFilter(m_renderDataLegacy.sfcStateParams);

    //---------------------------------
    // Set SFC State:  RGB Adaptive Filter
    //---------------------------------
    SetRGBAdaptive(m_renderDataLegacy.sfcStateParams);

    //---------------------------------
    // Set SFC State:  Colorfill
    //---------------------------------
    SetColorFillParams(m_renderDataLegacy.sfcStateParams);

    VP_RENDER_CHK_STATUS_RETURN(AllocateResources());

    m_renderDataLegacy.sfcStateParams->pOsResOutputSurface = &targetSurface->osSurface->OsResource;

    if (m_renderData.b1stPassOfSfc2PassScaling)
    {
        VP_RENDER_CHK_STATUS_RETURN(SetLineBuffer(m_renderDataLegacy.sfcStateParams->pOsResAVSLineBuffer, m_AVSLineBufferSurfaceArrayfor1stPassofSfc2Pass[m_scalabilityParams.curPipe]));
        VP_RENDER_CHK_STATUS_RETURN(SetLineBuffer(m_renderDataLegacy.sfcStateParams->pOsResIEFLineBuffer, m_IEFLineBufferSurfaceArrayfor1stPassofSfc2Pass[m_scalabilityParams.curPipe]));
    }
    else
    {
        VP_RENDER_CHK_STATUS_RETURN(SetLineBuffer(m_renderDataLegacy.sfcStateParams->pOsResAVSLineBuffer, m_AVSLineBufferSurfaceArray[m_scalabilityParams.curPipe]));
        VP_RENDER_CHK_STATUS_RETURN(SetLineBuffer(m_renderDataLegacy.sfcStateParams->pOsResIEFLineBuffer, m_IEFLineBufferSurfaceArray[m_scalabilityParams.curPipe]));
    }

    VP_RENDER_CHK_STATUS_RETURN(SetupScalabilityParams());

    // Decompress resource if surfaces need write from a un-align offset
    if ((targetSurface->osSurface->CompressionMode != MOS_MMC_DISABLED)        &&
        IsSFCUncompressedWriteNeeded(targetSurface))
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
        MOS_SURFACE details = {};

        eStatus = m_osInterface->pfnGetResourceInfo(m_osInterface, &targetSurface->osSurface->OsResource, &details);

        if (eStatus != MOS_STATUS_SUCCESS)
        {
            VP_RENDER_ASSERTMESSAGE("Get SFC target surface resource info failed.");
        }

        if (!targetSurface->osSurface->OsResource.bUncompressedWriteNeeded)
        {
            eStatus = m_osInterface->pfnDecompResource(m_osInterface, &targetSurface->osSurface->OsResource);

            if (eStatus != MOS_STATUS_SUCCESS)
            {
                VP_RENDER_ASSERTMESSAGE("inplace decompression failed for sfc target.");
            }
            else
            {
                VP_RENDER_NORMALMESSAGE("inplace decompression enabled for sfc target RECT is not compression block align.");
                targetSurface->osSurface->OsResource.bUncompressedWriteNeeded = 1;
            }
        }
    }

    if (targetSurface->osSurface->OsResource.bUncompressedWriteNeeded)
    {
        // Update SFC as uncompressed write
        m_renderDataLegacy.sfcStateParams->MMCMode = MOS_MMC_RC;
    }

    return eStatus;
}

bool SfcRenderBaseLegacy::IsSFCUncompressedWriteNeeded(PVP_SURFACE targetSurface)
{
    VP_FUNC_CALL();

    if ((!targetSurface)          ||
        (!targetSurface->osSurface))
    {
        return false;
    }

    if (!MEDIA_IS_SKU(m_skuTable, FtrE2ECompression))
    {
        return false;
    }

    if (m_osInterface && m_osInterface->bSimIsActive)
    {
        return false;
    }

    uint32_t byteInpixel = 1;
#if !EMUL
    if (!targetSurface->osSurface->OsResource.pGmmResInfo)
    {
        VP_RENDER_NORMALMESSAGE("IsSFCUncompressedWriteNeeded cannot support non GMM info cases");
        return false;
    }

    byteInpixel = targetSurface->osSurface->OsResource.pGmmResInfo->GetBitsPerPixel() >> 3;
#endif // !EMUL

    if (byteInpixel == 0)
    {
        VP_RENDER_NORMALMESSAGE("surface format is not a valid format for sfc");
        return false;
    }
    uint32_t writeAlignInWidth  = 32 / byteInpixel;
    uint32_t writeAlignInHeight = 8;
    

    if ((targetSurface->rcSrc.top % writeAlignInHeight) ||
        ((targetSurface->rcSrc.bottom - targetSurface->rcSrc.top) % writeAlignInHeight) ||
        (targetSurface->rcSrc.left % writeAlignInWidth) ||
        ((targetSurface->rcSrc.right - targetSurface->rcSrc.left) % writeAlignInWidth))
    {
        // full Frame Write don't need decompression as it will not hit the compressed write limitation
        if ((targetSurface->rcSrc.bottom - targetSurface->rcSrc.top) == targetSurface->osSurface->dwHeight &&
            (targetSurface->rcSrc.right - targetSurface->rcSrc.left) == targetSurface->osSurface->dwWidth)
        {
            return false;
        }

        VP_RENDER_NORMALMESSAGE(
            "SFC Render Target Uncompressed write needed, \
            targetSurface->rcSrc.top % d, \
            targetSurface->rcSrc.bottom % d, \
            targetSurface->rcSrc.left % d, \
            targetSurface->rcSrc.right % d \
            targetSurface->Format % d",
            targetSurface->rcSrc.top,
            targetSurface->rcSrc.bottom,
            targetSurface->rcSrc.left,
            targetSurface->rcSrc.right,
            targetSurface->osSurface->Format);

        return true;
    }

    return false;
}

MOS_STATUS SfcRenderBaseLegacy::SetScalingParams(PSFC_SCALING_PARAMS scalingParams)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(scalingParams);

    if (MhwSfcInterface::SFC_PIPE_MODE_VEBOX != m_pipeMode &&
        (scalingParams->dwInputFrameHeight != scalingParams->dwSourceRegionHeight ||
            scalingParams->dwInputFrameWidth != scalingParams->dwSourceRegionWidth))
    {
        // For Integral Image Mode, this source region width/height is Reserved.
        // In VD modes, source region width/height must be programmed to same value as input frame resolution width/height.
        VP_PUBLIC_ASSERTMESSAGE("Source region crop is not supported by Integral Image Mode and VD Mode!!");
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    // Adjust output width/height according to rotation.
    if (VPHAL_ROTATION_90 == m_renderDataLegacy.SfcRotation ||
        VPHAL_ROTATION_270 == m_renderDataLegacy.SfcRotation ||
        VPHAL_ROTATE_90_MIRROR_VERTICAL == m_renderDataLegacy.SfcRotation ||
        VPHAL_ROTATE_90_MIRROR_HORIZONTAL == m_renderDataLegacy.SfcRotation)
    {
        m_renderDataLegacy.sfcStateParams->dwOutputFrameWidth  = scalingParams->dwOutputFrameHeight;
        m_renderDataLegacy.sfcStateParams->dwOutputFrameHeight = scalingParams->dwOutputFrameWidth;
    }
    else
    {
        m_renderDataLegacy.sfcStateParams->dwOutputFrameWidth  = scalingParams->dwOutputFrameWidth;
        m_renderDataLegacy.sfcStateParams->dwOutputFrameHeight = scalingParams->dwOutputFrameHeight;
    }

    m_renderDataLegacy.sfcStateParams->dwInputFrameHeight             = scalingParams->dwInputFrameHeight;
    m_renderDataLegacy.sfcStateParams->dwInputFrameWidth              = scalingParams->dwInputFrameWidth;
    m_renderDataLegacy.sfcStateParams->dwAVSFilterMode                = scalingParams->bBilinearScaling ? MEDIASTATE_SFC_AVS_FILTER_BILINEAR : (MhwSfcInterface::SFC_PIPE_MODE_VDBOX == m_pipeMode ? MEDIASTATE_SFC_AVS_FILTER_5x5 : MEDIASTATE_SFC_AVS_FILTER_8x8);
    m_renderDataLegacy.sfcStateParams->dwSourceRegionHeight           = scalingParams->dwSourceRegionHeight;
    m_renderDataLegacy.sfcStateParams->dwSourceRegionWidth            = scalingParams->dwSourceRegionWidth;
    m_renderDataLegacy.sfcStateParams->dwSourceRegionVerticalOffset   = scalingParams->dwSourceRegionVerticalOffset;
    m_renderDataLegacy.sfcStateParams->dwSourceRegionHorizontalOffset = scalingParams->dwSourceRegionHorizontalOffset;
    m_renderDataLegacy.sfcStateParams->dwScaledRegionHeight           = scalingParams->dwScaledRegionHeight;
    m_renderDataLegacy.sfcStateParams->dwScaledRegionWidth            = scalingParams->dwScaledRegionWidth;
    m_renderDataLegacy.sfcStateParams->dwScaledRegionVerticalOffset   = scalingParams->dwScaledRegionVerticalOffset;
    m_renderDataLegacy.sfcStateParams->dwScaledRegionHorizontalOffset = scalingParams->dwScaledRegionHorizontalOffset;
    if (scalingParams->bRectangleEnabled)
    {
        m_renderDataLegacy.sfcStateParams->bRectangleEnabled                      = true;
        m_renderDataLegacy.sfcStateParams->dwTargetRectangleStartHorizontalOffset = scalingParams->dwTargetRectangleStartHorizontalOffset;
        m_renderDataLegacy.sfcStateParams->dwTargetRectangleStartVerticalOffset   = scalingParams->dwTargetRectangleStartVerticalOffset;
        m_renderDataLegacy.sfcStateParams->dwTargetRectangleEndHorizontalOffset   = scalingParams->dwTargetRectangleEndHorizontalOffset;
        m_renderDataLegacy.sfcStateParams->dwTargetRectangleEndVerticalOffset     = scalingParams->dwTargetRectangleEndVerticalOffset;
    }
    else
    {
        m_renderDataLegacy.sfcStateParams->bRectangleEnabled          = false;
    }
    m_renderDataLegacy.sfcStateParams->fAVSXScalingRatio              = scalingParams->fAVSXScalingRatio;
    m_renderDataLegacy.sfcStateParams->fAVSYScalingRatio              = scalingParams->fAVSYScalingRatio;

    m_renderDataLegacy.bScaling = ((scalingParams->fAVSXScalingRatio == 1.0F) && (scalingParams->fAVSYScalingRatio == 1.0F)) ? false : true;

    m_renderDataLegacy.fScaleX        = scalingParams->fAVSXScalingRatio;
    m_renderDataLegacy.fScaleY        = scalingParams->fAVSYScalingRatio;
    m_renderDataLegacy.SfcScalingMode = scalingParams->sfcScalingMode;

    // ColorFill/Alpha settings
    m_renderDataLegacy.pColorFillParams                  = &(scalingParams->sfcColorfillParams);
    m_renderDataLegacy.sfcStateParams->fAlphaPixel       = scalingParams->sfcColorfillParams.fAlphaPixel;
    m_renderDataLegacy.sfcStateParams->fColorFillAPixel  = scalingParams->sfcColorfillParams.fColorFillAPixel;
    m_renderDataLegacy.sfcStateParams->fColorFillUGPixel = scalingParams->sfcColorfillParams.fColorFillUGPixel;
    m_renderDataLegacy.sfcStateParams->fColorFillVBPixel = scalingParams->sfcColorfillParams.fColorFillVBPixel;
    m_renderDataLegacy.sfcStateParams->fColorFillYRPixel = scalingParams->sfcColorfillParams.fColorFillYRPixel;
    m_renderDataLegacy.sfcStateParams->isDemosaicEnabled = scalingParams->isDemosaicNeeded;

    // SfcInputFormat should be initialized during SetCscParams if SfcInputFormat not being Format_Any.
    if (Format_Any == m_renderDataLegacy.SfcInputFormat)
    {
        m_renderDataLegacy.SfcInputFormat = scalingParams->inputFrameFormat;
    }
    else if (m_renderDataLegacy.SfcInputFormat != scalingParams->inputFrameFormat)
    {
        VP_PUBLIC_ASSERTMESSAGE("Input formats configured during SetCscParams and SetScalingParams are not same!");
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SfcRenderBaseLegacy::UpdateIefParams(PVPHAL_IEF_PARAMS iefParams)
{
    VP_FUNC_CALL();
    m_renderDataLegacy.bIEF           = (iefParams &&
        iefParams->bEnabled &&
        iefParams->fIEFFactor > 0.0F);
    m_renderDataLegacy.pIefParams     = iefParams;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SfcRenderBaseLegacy::UpdateCscParams(FeatureParamCsc &cscParams)
{
    VP_RENDER_CHK_STATUS_RETURN(UpdateIefParams(cscParams.pIEFParams));
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SfcRenderBaseLegacy::SetCSCParams(PSFC_CSC_PARAMS cscParams)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(cscParams);

    if (MhwSfcInterface::SFC_PIPE_MODE_VEBOX == m_pipeMode)
    {
        m_renderDataLegacy.bIEF       = cscParams->bIEFEnable;
        m_renderDataLegacy.pIefParams = cscParams->iefParams;
    }
    else
    {
        if (cscParams->bIEFEnable)
        {
            VP_PUBLIC_ASSERTMESSAGE("IEF is not supported by Integral Image Mode and VD Mode!");
        }
        m_renderDataLegacy.bIEF       = false;
        m_renderDataLegacy.pIefParams = nullptr;
    }
    m_renderDataLegacy.bCSC           = IsCscNeeded(*cscParams);
    m_renderDataLegacy.SfcInputCspace = cscParams->inputColorSpace;
    m_renderDataLegacy.SfcInputFormat = cscParams->inputFormat;

    m_renderDataLegacy.sfcStateParams->bRGBASwapEnable  = IsOutputChannelSwapNeeded(cscParams->outputFormat);
    m_renderDataLegacy.sfcStateParams->bInputColorSpace = cscParams->isInputColorSpaceRGB;
    m_renderDataLegacy.sfcStateParams->isDemosaicEnabled = cscParams->isDemosaicNeeded;

    // Chromasitting config
    // VEBOX use polyphase coefficients for 1x scaling for better quality,
    // VDBOX dosen't use polyphase coefficients.
    if (MhwSfcInterface::SFC_PIPE_MODE_VEBOX == m_pipeMode)
    {
        m_renderDataLegacy.bForcePolyPhaseCoefs = cscParams->bChromaUpSamplingEnable;
    }
    else
    {
        m_renderDataLegacy.bForcePolyPhaseCoefs = false;
    }
    m_renderDataLegacy.SfcSrcChromaSiting = cscParams->sfcSrcChromaSiting;

    // 8-Tap chroma filter enabled or not
    m_renderDataLegacy.sfcStateParams->b8tapChromafiltering = cscParams->b8tapChromafiltering;

    // config SFC chroma down sampling
    m_renderDataLegacy.sfcStateParams->dwChromaDownSamplingHorizontalCoef = cscParams->chromaDownSamplingHorizontalCoef;
    m_renderDataLegacy.sfcStateParams->dwChromaDownSamplingVerticalCoef   = cscParams->chromaDownSamplingVerticalCoef;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SfcRenderBaseLegacy::SetRotMirParams(PSFC_ROT_MIR_PARAMS rotMirParams)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(rotMirParams);

    if (MhwSfcInterface::SFC_PIPE_MODE_VEBOX != m_pipeMode &&
        VPHAL_ROTATION_IDENTITY != rotMirParams->rotationMode &&
        VPHAL_MIRROR_HORIZONTAL != rotMirParams->rotationMode)
    {
        VP_PUBLIC_ASSERTMESSAGE("Rotation is not supported by Integral Image Mode and VD Mode!");
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    m_renderDataLegacy.SfcRotation   = rotMirParams->rotationMode;
    m_renderDataLegacy.bMirrorEnable = rotMirParams->bMirrorEnable;
    m_renderDataLegacy.mirrorType    = rotMirParams->mirrorType;

    // Adjust output width/height according to rotation.
    if (VPHAL_ROTATION_90 == m_renderDataLegacy.SfcRotation ||
        VPHAL_ROTATION_270 == m_renderDataLegacy.SfcRotation ||
        VPHAL_ROTATE_90_MIRROR_VERTICAL == m_renderDataLegacy.SfcRotation ||
        VPHAL_ROTATE_90_MIRROR_HORIZONTAL == m_renderDataLegacy.SfcRotation)
    {
        uint32_t width                                   = m_renderDataLegacy.sfcStateParams->dwOutputFrameWidth;
        m_renderDataLegacy.sfcStateParams->dwOutputFrameWidth  = m_renderDataLegacy.sfcStateParams->dwOutputFrameHeight;
        m_renderDataLegacy.sfcStateParams->dwOutputFrameHeight = width;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SfcRenderBaseLegacy::SetMmcParams(PMOS_SURFACE renderTarget, bool isFormatMmcSupported, bool isMmcEnabled)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(renderTarget);
    VP_PUBLIC_CHK_NULL_RETURN(m_renderDataLegacy.sfcStateParams);

    if (renderTarget->CompressionMode &&
        isFormatMmcSupported &&
        renderTarget->TileType == MOS_TILE_Y &&
        isMmcEnabled)
    {
        m_renderDataLegacy.sfcStateParams->bMMCEnable = true;
        m_renderDataLegacy.sfcStateParams->MMCMode    = renderTarget->CompressionMode;

        if (renderTarget->OsResource.bUncompressedWriteNeeded)
        {
            m_renderDataLegacy.sfcStateParams->MMCMode = MOS_MMC_RC;
        }
    }
    else
    {
        m_renderDataLegacy.sfcStateParams->bMMCEnable = false;
        m_renderDataLegacy.sfcStateParams->MMCMode    = MOS_MMC_DISABLED;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SfcRenderBaseLegacy::SetSfcStateInputChromaSubSampling(
    PMHW_SFC_STATE_PARAMS sfcStateParams)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(sfcStateParams);
    VPHAL_COLORPACK colorPack = VpHalDDIUtils::GetSurfaceColorPack(m_renderDataLegacy.SfcInputFormat);
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

MOS_STATUS SfcRenderBaseLegacy::SetSfcStateInputOrderingMode(
    PMHW_SFC_STATE_PARAMS sfcStateParams)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(sfcStateParams);

    if (m_bVdboxToSfc)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(SetSfcStateInputOrderingModeVdbox(sfcStateParams));
    }
    else if (MhwSfcInterface::SFC_PIPE_MODE_VEBOX == m_pipeMode)
    {
        if (m_renderDataLegacy.sfcStateParams && m_renderDataLegacy.sfcStateParams->isDemosaicEnabled)
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

MOS_STATUS SfcRenderBaseLegacy::SetSfcStateInputOrderingModeVdbox(
    PMHW_SFC_STATE_PARAMS sfcStateParams)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(sfcStateParams);
    switch (m_videoConfig.codecStandard)
    {
    case CODECHAL_VC1:
        sfcStateParams->dwVDVEInputOrderingMode = MEDIASTATE_SFC_INPUT_ORDERING_VD_16x16_NOSHIFT;
        break;
    case CODECHAL_AVC:
        sfcStateParams->dwVDVEInputOrderingMode = m_videoConfig.avc.deblockingEnabled ? MEDIASTATE_SFC_INPUT_ORDERING_VD_16x16_SHIFT : MEDIASTATE_SFC_INPUT_ORDERING_VD_16x16_NOSHIFT;
        break;
    case CODECHAL_VP8:
        sfcStateParams->dwVDVEInputOrderingMode = m_videoConfig.vp8.deblockingEnabled ? MEDIASTATE_SFC_INPUT_ORDERING_VD_16x16_SHIFT : MEDIASTATE_SFC_INPUT_ORDERING_VD_16x16_VP8;
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

MOS_STATUS SfcRenderBaseLegacy::SetSfcStateInputOrderingModeJpeg(
    PMHW_SFC_STATE_PARAMS sfcStateParams)
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

MOS_STATUS SfcRenderBaseLegacy::SetSfcStateInputOrderingModeHcp(
    PMHW_SFC_STATE_PARAMS pSfcStateParams)
{
    return MOS_STATUS_UNIMPLEMENTED;
}

MOS_STATUS SfcRenderBaseLegacy::SendSfcCmd(
    bool                bOutputToMemory,
    PMOS_COMMAND_BUFFER pCmdBuffer)
{
    VP_FUNC_CALL();

    MHW_SFC_LOCK_PARAMS        SfcLockParams;
    MOS_STATUS                 eStatus;
    MHW_SFC_OUT_SURFACE_PARAMS OutSurfaceParam;

    VP_RENDER_CHK_NULL_RETURN(pCmdBuffer);

    eStatus = MOS_STATUS_SUCCESS;

    // Setup params for SFC Lock command
    SfcLockParams.sfcPipeMode     = m_pipeMode;
    SfcLockParams.bOutputToMemory = bOutputToMemory;

    // Send SFC_LOCK command to acquire SFC pipe for Vebox
    VP_RENDER_CHK_STATUS_RETURN(AddSfcLock(
        pCmdBuffer,
        &SfcLockParams));

    VP_RENDER_CHK_STATUS_RETURN(InitMhwOutSurfParams(
        m_renderDataLegacy.pSfcPipeOutSurface,
        &OutSurfaceParam));

    // Send SFC_STATE command
    VP_RENDER_CHK_STATUS_RETURN(AddSfcState(
        pCmdBuffer,
        m_renderDataLegacy.sfcStateParams,
        &OutSurfaceParam));
    //SETPAR_AND_ADDCMD(SFC_STATE, sfcItf, pCmdBuffer);

    // Send SFC_AVS_STATE command
    VP_RENDER_CHK_STATUS_RETURN(AddSfcAvsState(
        pCmdBuffer,
        &m_avsStateLegacy.AvsStateParams));
    //SETPAR_AND_ADDCMD(SFC_AVS_STATE, sfcItf, pCmdBuffer);

    if (m_renderDataLegacy.bScaling ||
        m_renderDataLegacy.bForcePolyPhaseCoefs)
    {
        // Send SFC_AVS_LUMA_TABLE command
        VP_RENDER_CHK_STATUS_RETURN(AddSfcAvsLumaTable(
            pCmdBuffer,
            &m_avsStateLegacy.LumaCoeffs));
        //SETPAR_AND_ADDCMD(SFC_AVS_LUMA_Coeff_Table, sfcItf, pCmdBuffer);

        // Send SFC_AVS_CHROMA_TABLE command
        VP_RENDER_CHK_STATUS_RETURN(AddSfcAvsChromaTable(
            pCmdBuffer,
            &m_avsStateLegacy.ChromaCoeffs));
        //SETPAR_AND_ADDCMD(SFC_AVS_CHROMA_Coeff_Table, sfcItf, pCmdBuffer);
    }

    // Send SFC_IEF_STATE command
    if (m_renderDataLegacy.bIEF || m_renderDataLegacy.bCSC)
    {
        // Will modified when enable IEF/CSC
        VP_RENDER_CHK_STATUS_RETURN(AddSfcIefState(
            pCmdBuffer,
            &m_IefStateParamsLegacy));
        //SETPAR_AND_ADDCMD(SFC_IEF_STATE, sfcItf, pCmdBuffer);
    }

    // Send SFC_FRAME_START command to start processing a frame
    VP_RENDER_CHK_STATUS_RETURN(AddSfcFrameStart(
        pCmdBuffer,
        m_pipeMode));

    return eStatus;
}

uint32_t SfcRenderBaseLegacy::GetAvsLineBufferSize(bool lineTiledBuffer, bool b8tapChromafiltering, uint32_t width, uint32_t height)
{
    VP_FUNC_CALL();

    uint32_t size                   = 0;
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

uint32_t SfcRenderBaseLegacy::GetIefLineBufferSize(bool lineTiledBuffer, uint32_t heightOutput)
{
    VP_FUNC_CALL();

    uint32_t size = 0;

    // For VE+SFC mode, height needs be used.
    if (MhwSfcInterface::SFC_PIPE_MODE_VEBOX == m_pipeMode)
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

uint32_t SfcRenderBaseLegacy::GetSfdLineBufferSize(bool lineTiledBuffer, MOS_FORMAT formatOutput, uint32_t widthOutput, uint32_t heightOutput)
{
    VP_FUNC_CALL();

    int size = 0;

    // For VD+SFC mode, width needs be used. For VE+SFC mode, height needs be used.
    if (MhwSfcInterface::SFC_PIPE_MODE_VEBOX == m_pipeMode)
    {
        size = (VPHAL_COLORPACK_444 == VpHalDDIUtils::GetSurfaceColorPack(formatOutput)) ? 0 : (heightOutput * SFC_SFD_LINEBUFFER_SIZE_PER_PIXEL);
    }
    else
    {
        size = MOS_ROUNDUP_DIVIDE(widthOutput, 10) * SFC_CACHELINE_SIZE_IN_BYTES;
        size *= 2;  //double for safe
    }

    // For tile column storage, based on above calcuation, an extra 1K CL need to be added as a buffer.
    // size == 0 means line buffer not needed.
    if (lineTiledBuffer && size > 0)
    {
        size += 1024 * MHW_SFC_CACHELINE_SIZE;
    }

    return size;
}

MOS_STATUS SfcRenderBaseLegacy::AllocateResources()
{
    VP_FUNC_CALL();

    uint32_t              size;
    PMHW_SFC_STATE_PARAMS sfcStateParams;

    VP_RENDER_CHK_NULL_RETURN(m_allocator);
    VP_RENDER_CHK_NULL_RETURN(m_renderDataLegacy.sfcStateParams);

    sfcStateParams = m_renderDataLegacy.sfcStateParams;

    // for 1st pass of Sfc 2Pass case, use the standalone line buffer array to avoid line buffer reallocation
    if (m_renderDataLegacy.b1stPassOfSfc2PassScaling)
    {
        if (m_scalabilityParams.numPipe > m_lineBufferAllocatedInArrayfor1stPassofSfc2Pass ||
            nullptr == m_AVSLineBufferSurfaceArrayfor1stPassofSfc2Pass ||
            nullptr == m_IEFLineBufferSurfaceArrayfor1stPassofSfc2Pass ||
            nullptr == m_SFDLineBufferSurfaceArrayfor1stPassofSfc2Pass)
        {
            DestroyLineBufferArray(m_AVSLineBufferSurfaceArrayfor1stPassofSfc2Pass, m_lineBufferAllocatedInArrayfor1stPassofSfc2Pass);
            DestroyLineBufferArray(m_IEFLineBufferSurfaceArrayfor1stPassofSfc2Pass, m_lineBufferAllocatedInArrayfor1stPassofSfc2Pass);
            DestroyLineBufferArray(m_SFDLineBufferSurfaceArrayfor1stPassofSfc2Pass, m_lineBufferAllocatedInArrayfor1stPassofSfc2Pass);
            m_lineBufferAllocatedInArrayfor1stPassofSfc2Pass = m_scalabilityParams.numPipe;
            m_AVSLineBufferSurfaceArrayfor1stPassofSfc2Pass  = MOS_NewArray(VP_SURFACE *, m_lineBufferAllocatedInArrayfor1stPassofSfc2Pass);
            VP_RENDER_CHK_NULL_RETURN(m_AVSLineBufferSurfaceArrayfor1stPassofSfc2Pass);
            m_IEFLineBufferSurfaceArrayfor1stPassofSfc2Pass = MOS_NewArray(VP_SURFACE *, m_lineBufferAllocatedInArrayfor1stPassofSfc2Pass);
            VP_RENDER_CHK_NULL_RETURN(m_IEFLineBufferSurfaceArrayfor1stPassofSfc2Pass);
            m_SFDLineBufferSurfaceArrayfor1stPassofSfc2Pass = MOS_NewArray(VP_SURFACE *, m_lineBufferAllocatedInArrayfor1stPassofSfc2Pass);
            VP_RENDER_CHK_NULL_RETURN(m_SFDLineBufferSurfaceArrayfor1stPassofSfc2Pass);
        }

        // for AVSLineBuffer, IEFLineBuffer and SFDLineBuffer, they are only needed when surface allocation bigger than 4150.
        // for AVSLineTileBuffer, IEFLineTileBuffer and SFDLineTileBuffer, they are only needed for VdBox SFC scalability case and not needed for VeBox SFC case.

        // Allocate AVS Line Buffer surface----------------------------------------------
        size = GetAvsLineBufferSize(false, sfcStateParams->b8tapChromafiltering, sfcStateParams->dwInputFrameWidth, sfcStateParams->dwInputFrameHeight);
        VP_RENDER_CHK_STATUS_RETURN(AllocateLineBufferArray(m_AVSLineBufferSurfaceArrayfor1stPassofSfc2Pass, size, "SfcAVSLineBufferSurfacefor1stPassofSfc2Pass"));

        // Allocate IEF Line Buffer surface----------------------------------------------
        size = GetIefLineBufferSize(false, sfcStateParams->dwScaledRegionHeight);
        VP_RENDER_CHK_STATUS_RETURN(AllocateLineBufferArray(m_IEFLineBufferSurfaceArrayfor1stPassofSfc2Pass, size, "SfcIEFLineBufferSurfacefor1stPassofSfc2Pass"));

        if (sfcStateParams->dwScaledRegionHeight > SFC_LINEBUFEER_SIZE_LIMITED)
        {
            // Allocate SFD Line Buffer surface
            size = GetSfdLineBufferSize(false, sfcStateParams->OutputFrameFormat, sfcStateParams->dwScaledRegionWidth, sfcStateParams->dwScaledRegionHeight);
            VP_RENDER_CHK_STATUS_RETURN(AllocateLineBufferArray(m_SFDLineBufferSurfaceArrayfor1stPassofSfc2Pass, size, "SfcSFDLineBufferSurfacefor1stPassofSfc2Pass"));
        }
    }
    else
    {
        if (m_scalabilityParams.numPipe > m_lineBufferAllocatedInArray ||
            nullptr == m_AVSLineBufferSurfaceArray ||
            nullptr == m_IEFLineBufferSurfaceArray ||
            nullptr == m_SFDLineBufferSurfaceArray)
        {
            DestroyLineBufferArray(m_AVSLineBufferSurfaceArray, m_lineBufferAllocatedInArray);
            DestroyLineBufferArray(m_IEFLineBufferSurfaceArray, m_lineBufferAllocatedInArray);
            DestroyLineBufferArray(m_SFDLineBufferSurfaceArray, m_lineBufferAllocatedInArray);
            m_lineBufferAllocatedInArray = m_scalabilityParams.numPipe;
            m_AVSLineBufferSurfaceArray  = MOS_NewArray(VP_SURFACE *, m_lineBufferAllocatedInArray);
            VP_RENDER_CHK_NULL_RETURN(m_AVSLineBufferSurfaceArray);
            m_IEFLineBufferSurfaceArray = MOS_NewArray(VP_SURFACE *, m_lineBufferAllocatedInArray);
            VP_RENDER_CHK_NULL_RETURN(m_IEFLineBufferSurfaceArray);
            m_SFDLineBufferSurfaceArray = MOS_NewArray(VP_SURFACE *, m_lineBufferAllocatedInArray);
            VP_RENDER_CHK_NULL_RETURN(m_SFDLineBufferSurfaceArray);
        }

        // for AVSLineBuffer, IEFLineBuffer and SFDLineBuffer, they are only needed when surface allocation bigger than 4150.
        // for AVSLineTileBuffer, IEFLineTileBuffer and SFDLineTileBuffer, they are only needed for VdBox SFC scalability case and not needed for VeBox SFC case.

        // Allocate AVS Line Buffer surface----------------------------------------------
        size = GetAvsLineBufferSize(false, sfcStateParams->b8tapChromafiltering, sfcStateParams->dwInputFrameWidth, sfcStateParams->dwInputFrameHeight);
        VP_RENDER_CHK_STATUS_RETURN(AllocateLineBufferArray(m_AVSLineBufferSurfaceArray, size, "SfcAVSLineBufferSurface"));

        // Allocate IEF Line Buffer surface----------------------------------------------
        size = GetIefLineBufferSize(false, sfcStateParams->dwScaledRegionHeight);
        VP_RENDER_CHK_STATUS_RETURN(AllocateLineBufferArray(m_IEFLineBufferSurfaceArray, size, "SfcIEFLineBufferSurface"));

        if (sfcStateParams->dwScaledRegionHeight > SFC_LINEBUFEER_SIZE_LIMITED)
        {
            // Allocate SFD Line Buffer surface
            size = GetSfdLineBufferSize(false, sfcStateParams->OutputFrameFormat, sfcStateParams->dwScaledRegionWidth, sfcStateParams->dwScaledRegionHeight);
            VP_RENDER_CHK_STATUS_RETURN(AllocateLineBufferArray(m_SFDLineBufferSurfaceArray, size, "SfcSFDLineBufferSurface"));
        }
    }

    if (m_bVdboxToSfc)
    {
        // Allocate AVS Line Tile Buffer surface----------------------------------------------
        size = GetAvsLineBufferSize(true, sfcStateParams->b8tapChromafiltering, sfcStateParams->dwInputFrameWidth, sfcStateParams->dwInputFrameHeight);
        VP_RENDER_CHK_STATUS_RETURN(AllocateLineBuffer(m_AVSLineTileBufferSurface, size, "SfcAVSLineTileBufferSurface"));

        // Allocate IEF Line Tile Buffer surface----------------------------------------------
        size = GetIefLineBufferSize(true, sfcStateParams->dwScaledRegionHeight);
        VP_RENDER_CHK_STATUS_RETURN(AllocateLineBuffer(m_IEFLineTileBufferSurface, size, "SfcIEFLineTileBufferSurface"));

        // Allocate SFD Line Tile Buffer surface
        size = GetSfdLineBufferSize(true, sfcStateParams->OutputFrameFormat, sfcStateParams->dwScaledRegionWidth, sfcStateParams->dwScaledRegionHeight);
        VP_RENDER_CHK_STATUS_RETURN(AllocateLineBuffer(m_SFDLineTileBufferSurface, size, "SfcSFDLineTileBufferSurface"));
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SfcRenderBaseLegacy::AddSfcLock(
    PMOS_COMMAND_BUFFER            pCmdBuffer,
    PMHW_SFC_LOCK_PARAMS           pSfcLockParams)
{
    VP_FUNC_CALL();

    VP_RENDER_CHK_NULL_RETURN(m_sfcInterface);

    // Send SFC_LOCK command to acquire SFC pipe for Vebox
    VP_RENDER_CHK_STATUS_RETURN(m_sfcInterface->AddSfcLock(
        pCmdBuffer,
        pSfcLockParams));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SfcRenderBaseLegacy::AddSfcState(
    PMOS_COMMAND_BUFFER            pCmdBuffer,
    PMHW_SFC_STATE_PARAMS          pSfcState,
    PMHW_SFC_OUT_SURFACE_PARAMS    pOutSurface)
{
    VP_FUNC_CALL();

    PMHW_SFC_INTERFACE              pSfcInterface;
    MOS_STATUS                      eStatus;

    VP_RENDER_CHK_NULL_RETURN(m_sfcInterface);
    VP_RENDER_CHK_NULL_RETURN(pCmdBuffer);
    VP_RENDER_CHK_NULL_RETURN(pSfcState);
    VP_RENDER_CHK_NULL_RETURN(pOutSurface);

    eStatus                 = MOS_STATUS_SUCCESS;
    pSfcInterface           = m_sfcInterface;

    VP_RENDER_CHK_STATUS_RETURN(pSfcInterface->AddSfcState(
        pCmdBuffer,
        pSfcState,
        pOutSurface));

    return eStatus;
}

MOS_STATUS SfcRenderBaseLegacy::AddSfcAvsState(
    PMOS_COMMAND_BUFFER pCmdBuffer,
    PMHW_SFC_AVS_STATE  pSfcAvsStateParams)
{
    VP_FUNC_CALL();

    PMHW_SFC_INTERFACE              pSfcInterface;
    MOS_STATUS                      eStatus;

    VP_RENDER_CHK_NULL_RETURN(m_sfcInterface);
    VP_RENDER_CHK_NULL_RETURN(pCmdBuffer);
    VP_RENDER_CHK_NULL_RETURN(pSfcAvsStateParams);

    eStatus                 = MOS_STATUS_SUCCESS;
    pSfcInterface           = m_sfcInterface;

    // Send SFC_AVS_STATE command
    VP_RENDER_CHK_STATUS_RETURN(pSfcInterface->AddSfcAvsState(
        pCmdBuffer,
        pSfcAvsStateParams));

    return eStatus;
}

MOS_STATUS SfcRenderBaseLegacy::AddSfcIefState(
    PMOS_COMMAND_BUFFER       pCmdBuffer,
    PMHW_SFC_IEF_STATE_PARAMS pSfcIefStateParams)
{
    VP_FUNC_CALL();

    PMHW_SFC_INTERFACE              pSfcInterface;
    MOS_STATUS                      eStatus;

    VP_RENDER_CHK_NULL_RETURN(m_sfcInterface);
    VP_RENDER_CHK_NULL_RETURN(pCmdBuffer);
    VP_RENDER_CHK_NULL_RETURN(pSfcIefStateParams);

    eStatus                 = MOS_STATUS_SUCCESS;
    pSfcInterface           = m_sfcInterface;

    // Will modified when enable IEF/CSC
    VP_RENDER_CHK_STATUS_RETURN(pSfcInterface->AddSfcIefState(
          pCmdBuffer,
          pSfcIefStateParams));

    return eStatus;
}

MOS_STATUS SfcRenderBaseLegacy::AddSfcAvsLumaTable(
        PMOS_COMMAND_BUFFER     pCmdBuffer,
        PMHW_SFC_AVS_LUMA_TABLE pLumaTable)
{
    VP_FUNC_CALL();

    PMHW_SFC_INTERFACE              pSfcInterface;
    MOS_STATUS                      eStatus;

    VP_RENDER_CHK_NULL_RETURN(m_sfcInterface);
    VP_RENDER_CHK_NULL_RETURN(pCmdBuffer);
    VP_RENDER_CHK_NULL_RETURN(pLumaTable);

    eStatus                 = MOS_STATUS_SUCCESS;
    pSfcInterface           = m_sfcInterface;

    // Send SFC_AVS_LUMA_TABLE command
    VP_RENDER_CHK_STATUS_RETURN(pSfcInterface->AddSfcAvsLumaTable(
        pCmdBuffer,
        pLumaTable));

    return eStatus;
}

MOS_STATUS SfcRenderBaseLegacy::AddSfcAvsChromaTable(
        PMOS_COMMAND_BUFFER       pCmdBuffer,
        PMHW_SFC_AVS_CHROMA_TABLE pChromaTable)
{
    VP_FUNC_CALL();

    MOS_STATUS                      eStatus;
    PMHW_SFC_INTERFACE              pSfcInterface;

    VP_RENDER_CHK_NULL_RETURN(m_sfcInterface);
    VP_RENDER_CHK_NULL_RETURN(pCmdBuffer);
    VP_RENDER_CHK_NULL_RETURN(pChromaTable);

    eStatus                 = MOS_STATUS_SUCCESS;
    pSfcInterface           = m_sfcInterface;

    // Send SFC_AVS_CHROMA_TABLE command
    VP_RENDER_CHK_STATUS_RETURN(pSfcInterface->AddSfcAvsChromaTable(
        pCmdBuffer,
        pChromaTable));

    return eStatus;
}

MOS_STATUS SfcRenderBaseLegacy::AddSfcFrameStart(
        PMOS_COMMAND_BUFFER pCmdBuffer,
        uint8_t             sfcPipeMode)
{
    VP_FUNC_CALL();

    MOS_STATUS                      eStatus;
    PMHW_SFC_INTERFACE              pSfcInterface;

    VP_RENDER_CHK_NULL_RETURN(m_sfcInterface);
    VP_RENDER_CHK_NULL_RETURN(pCmdBuffer);

    eStatus                 = MOS_STATUS_SUCCESS;
    pSfcInterface           = m_sfcInterface;

        // Send SFC_FRAME_START command to start processing a frame
        VP_RENDER_CHK_STATUS_RETURN(pSfcInterface->AddSfcFrameStart(
            pCmdBuffer,
            sfcPipeMode));

    return eStatus;
}

MOS_STATUS SfcRenderBaseLegacy::SetSfcAVSScalingMode(
    MHW_SCALING_MODE  ScalingMode)
{
    VP_FUNC_CALL();

    MOS_STATUS                      eStatus;
    PMHW_SFC_INTERFACE              pSfcInterface;
  
    VP_RENDER_CHK_NULL_RETURN(m_sfcInterface);
    eStatus = MOS_STATUS_SUCCESS;
    pSfcInterface = m_sfcInterface;

    // Send SFC_FRAME_START command to start processing a frame
    VP_RENDER_CHK_STATUS_RETURN(pSfcInterface->SetSfcAVSScalingMode(
        ScalingMode));

    return eStatus;
}

MOS_STATUS SfcRenderBaseLegacy::SetSfcSamplerTable(
    PMHW_SFC_AVS_LUMA_TABLE         pLumaTable,
    PMHW_SFC_AVS_CHROMA_TABLE       pChromaTable,
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
    PMHW_SFC_INTERFACE              pSfcInterface;

    VP_RENDER_CHK_NULL_RETURN(m_sfcInterface);
    VP_RENDER_CHK_NULL_RETURN(pLumaTable);
    VP_RENDER_CHK_NULL_RETURN(pChromaTable);
    VP_RENDER_CHK_NULL_RETURN(pAvsParams);

    eStatus = MOS_STATUS_SUCCESS;
    pSfcInterface = m_sfcInterface;

    // Send SFC_FRAME_START command to start processing a frame
    VP_RENDER_CHK_STATUS_RETURN(pSfcInterface->SetSfcSamplerTable(
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
