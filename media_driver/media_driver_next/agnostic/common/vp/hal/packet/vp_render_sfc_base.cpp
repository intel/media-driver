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
    PMOS_INTERFACE osInterface,
    PMHW_SFC_INTERFACE sfcInterface,
    PVpAllocator &allocator):
    m_allocator(allocator)
{
    VP_PUBLIC_CHK_NULL_NO_STATUS_RETURN(osInterface);
    VP_PUBLIC_CHK_NULL_NO_STATUS_RETURN(sfcInterface);
    m_osInterface = osInterface;
    m_sfcInterface = sfcInterface;

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
}

MOS_STATUS SfcRenderBase::Init()
{
    MOS_ZeroMemory(&m_renderData, sizeof(m_renderData));
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

    psfcStateParams->dwInputChromaSubSampling           = m_renderData.inputChromaSubSampling;
    psfcStateParams->dwChromaDownSamplingHorizontalCoef = m_renderData.chromaDownSamplingHorizontalCoef;
    psfcStateParams->dwChromaDownSamplingVerticalCoef   = m_renderData.chromaDownSamplingVerticalCoef;
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
       m_renderData.fScaleY > 1.0F))
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

    VP_RENDER_CHK_NULL_RETURN(m_sfcInterface);

    pMhwAvsState = &m_avsState.AvsStateParams;
    MOS_ZeroMemory(pMhwAvsState, sizeof(MHW_SFC_AVS_STATE));

    if (m_renderData.bScaling ||
        m_renderData.bForcePolyPhaseCoefs)
    {
        pMhwAvsState->dwInputHorizontalSiting = (m_renderData.SfcSrcChromaSiting & MHW_CHROMA_SITING_HORZ_CENTER) ? SFC_AVS_INPUT_SITING_COEF_4_OVER_8 :
                                                ((m_renderData.SfcSrcChromaSiting & MHW_CHROMA_SITING_HORZ_RIGHT) ? SFC_AVS_INPUT_SITING_COEF_8_OVER_8 :
                                                SFC_AVS_INPUT_SITING_COEF_0_OVER_8);

        pMhwAvsState->dwInputVerticalSitting = (m_renderData.SfcSrcChromaSiting & MHW_CHROMA_SITING_VERT_CENTER) ? SFC_AVS_INPUT_SITING_COEF_4_OVER_8 :
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

        VP_RENDER_CHK_STATUS_RETURN(m_sfcInterface->SetSfcSamplerTable(
            &m_avsState.LumaCoeffs,
            &m_avsState.ChromaCoeffs,
            m_renderData.pAvsParams,
            m_renderData.SfcInputFormat,
            m_renderData.fScaleX,
            m_renderData.fScaleY,
            m_renderData.SfcSrcChromaSiting,
            true,
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
    m_renderData.sfcStateParams->sfcPipeMode = MEDIASTATE_SFC_PIPE_VE_TO_SFC;

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
    m_renderData.sfcStateParams->pOsResIEFLineBuffer = &m_IEFLineBufferSurface.OsResource;
    m_renderData.sfcStateParams->pOsResAVSLineBuffer = &m_AVSLineBufferSurface.OsResource;

    return eStatus;
}

MOS_STATUS SfcRenderBase::SetScalingParams(PSFC_SCALING_PARAMS scalingParams)
{
    VP_PUBLIC_CHK_NULL_RETURN(scalingParams);

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

    m_renderData.sfcStateParams->dwAVSFilterMode                = scalingParams->dwAVSFilterMode;
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

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SfcRenderBase::SetCSCParams(PSFC_CSC_PARAMS cscParams)
{
    VP_PUBLIC_CHK_NULL_RETURN(cscParams);

    m_renderData.bIEF           = cscParams->bIEFEnable;
    m_renderData.bCSC           = cscParams->bCSCEnabled;
    m_renderData.pIefParams     = cscParams->iefParams;
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
    m_renderData.inputChromaSubSampling = cscParams->inputChromaSubSampling;

    // 8-Tap chroma filter enabled or not
    m_renderData.sfcStateParams->b8tapChromafiltering = cscParams->b8tapChromafiltering;

    // config SFC chroma down sampling
    m_renderData.chromaDownSamplingHorizontalCoef = cscParams->chromaDownSamplingHorizontalCoef;
    m_renderData.chromaDownSamplingVerticalCoef   = cscParams->chromaDownSamplingVerticalCoef;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SfcRenderBase::SetRotMirParams(PSFC_ROT_MIR_PARAMS rotMirParams)
{
    VP_PUBLIC_CHK_NULL_RETURN(rotMirParams);

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
    }

    return MOS_STATUS_SUCCESS;
}

void SfcRenderBase::SetSfcStateInputOrderingMode(
    VpVeboxRenderData           *veboxRenderData,
    PMHW_SFC_STATE_PARAMS       sfcStateParams)
{
    MOS_UNUSED(veboxRenderData);
    VP_PUBLIC_CHK_NULL_NO_STATUS_RETURN(sfcStateParams);

    sfcStateParams->dwVDVEInputOrderingMode = MEDIASTATE_SFC_INPUT_ORDERING_VE_4x8;
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

MOS_STATUS SfcRenderBase::SendSfcCmd(
    VpVeboxRenderData               *pRenderData,
    PMOS_COMMAND_BUFFER             pCmdBuffer)
{
    PMHW_SFC_INTERFACE              pSfcInterface;
    MHW_SFC_LOCK_PARAMS             SfcLockParams;
    MOS_STATUS                      eStatus;
    MHW_SFC_OUT_SURFACE_PARAMS      OutSurfaceParam;

    VP_RENDER_CHK_NULL_RETURN(m_sfcInterface);
    VP_RENDER_CHK_NULL_RETURN(pRenderData);
    VP_RENDER_CHK_NULL_RETURN(pCmdBuffer);

    eStatus                 = MOS_STATUS_SUCCESS;
    pSfcInterface           = m_sfcInterface;

    // Setup params for SFC Lock command
    SfcLockParams.sfcPipeMode     = MhwSfcInterface::SFC_PIPE_MODE_VEBOX;
    SfcLockParams.bOutputToMemory = (pRenderData->DI.bDeinterlace || pRenderData->DN.bDnEnabled);

    // Send SFC_LOCK command to acquire SFC pipe for Vebox
    VP_RENDER_CHK_STATUS_RETURN(pSfcInterface->AddSfcLock(
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
        MhwSfcInterface::SFC_PIPE_MODE_VEBOX));

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

MOS_STATUS SfcRenderBase::AllocateResources()
{
    uint32_t                width;
    uint32_t                height;
    uint32_t                size;
    bool                    allocated;
    PMHW_SFC_STATE_PARAMS   sfcStateParams;

    VP_RENDER_CHK_NULL_RETURN(m_allocator);
    VP_RENDER_CHK_NULL_RETURN(m_renderData.sfcStateParams);

    allocated = false;
    sfcStateParams = m_renderData.sfcStateParams;

    // Allocate AVS Line Buffer surface----------------------------------------------
    width = 1;
    height = sfcStateParams->dwInputFrameHeight * SFC_AVS_LINEBUFFER_SIZE_PER_VERTICAL_PIXEL;
    size = width * height;

    VP_RENDER_CHK_STATUS_RETURN(m_allocator->ReAllocateSurface(
                                  &m_AVSLineBufferSurface,
                                  "SfcAVSLineBufferSurface",
                                  Format_Buffer,
                                  MOS_GFXRES_BUFFER,
                                  MOS_TILE_LINEAR,
                                  size,
                                  1,
                                  false,
                                  MOS_MMC_DISABLED,
                                  allocated));

    // Allocate IEF Line Buffer surface----------------------------------------------
    width = 1;
    height = sfcStateParams->dwScaledRegionHeight * SFC_IEF_LINEBUFFER_SIZE_PER_VERTICAL_PIXEL;
    size = width * height;

    VP_RENDER_CHK_STATUS_RETURN(m_allocator->ReAllocateSurface(
                                  &m_IEFLineBufferSurface,
                                  "SfcIEFLineBufferSurface",
                                  Format_Buffer,
                                  MOS_GFXRES_BUFFER,
                                  MOS_TILE_LINEAR,
                                  size,
                                  1,
                                  false,
                                  MOS_MMC_DISABLED,
                                  allocated));

    // Allocate SFD Line Buffer surface----------------------------------------------
    if (NEED_SFD_LINE_BUFFER(sfcStateParams->dwScaledRegionHeight))
    {
        size = SFD_LINE_BUFFER_SIZE(sfcStateParams->dwScaledRegionHeight);

        VP_RENDER_CHK_STATUS_RETURN(m_allocator->ReAllocateSurface(
            &m_SFDLineBufferSurface,
            "SfcSFDLineBufferSurface",
            Format_Buffer,
            MOS_GFXRES_BUFFER,
            MOS_TILE_LINEAR,
            size,
            1,
            false,
            MOS_MMC_DISABLED,
            allocated));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SfcRenderBase::FreeResources()
{
    VP_RENDER_CHK_NULL_RETURN(m_allocator);
    // Free AVS Line Buffer surface for SFC
    m_allocator->FreeResource(&m_AVSLineBufferSurface.OsResource);

    // Free IEF Line Buffer surface for SFC
    m_allocator->FreeResource(&m_IEFLineBufferSurface.OsResource);

    // Free SFD Line Buffer surface for SFC
    m_allocator->FreeResource(&m_SFDLineBufferSurface.OsResource);

    return MOS_STATUS_SUCCESS;
}
static const uint16_t k_WidthAlignUnit[4] = { 2, 2, 1, 1 };
static const uint16_t k_HeightAlignUnit[4] = { 2, 1, 1, 1 };
}
