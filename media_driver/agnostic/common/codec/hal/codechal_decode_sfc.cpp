/*
* Copyright (c) 2014-2017, Intel Corporation
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
//! \file     codechal_decode_sfc.cpp
//! \brief    Implements the decode interface extension for CSC and scaling via SFC.
//! \details  Downsampling in this case is supported by the SFC fixed function HW unit.
//!

#include "codechal_decode_sfc.h"
#include "codechal_decoder.h"

MOS_STATUS CodechalSfcState::AllocateResources()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;

    if (MhwSfcInterface::SFC_PIPE_MODE_VEBOX == m_sfcPipeMode)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(pOsInterface->pfnCreateSyncResource(pOsInterface, &resSyncObject));
    }

    // Allocate AVS line buffer
    if (Mos_ResourceIsNull(&resAvsLineBuffer))
    {
        MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
        MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBufferLinear.Type     = MOS_GFXRES_BUFFER;
        allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
        allocParamsForBufferLinear.Format   = Format_Buffer;
        if (MhwSfcInterface::SFC_PIPE_MODE_VEBOX == m_sfcPipeMode)
        {
            allocParamsForBufferLinear.dwBytes = MOS_ROUNDUP_DIVIDE(pInputSurface->dwHeight, 8) * 5 * MHW_SFC_CACHELINE_SIZE;
        }
        else
        {
            allocParamsForBufferLinear.dwBytes = MOS_ROUNDUP_DIVIDE(pInputSurface->dwWidth, 8) * 3 * MHW_SFC_CACHELINE_SIZE;
        }
        allocParamsForBufferLinear.pBufName = "SfcAvsLineBuffer";

        eStatus = (MOS_STATUS)pOsInterface->pfnAllocateResource(
            pOsInterface,
            &allocParamsForBufferLinear,
            &resAvsLineBuffer);

        if (eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_DECODE_ASSERTMESSAGE("Failed to allocate Sfc Avs Line Buffer.");
            return eStatus;
        }
    }

    // Allocate IEF line buffer

    //Initialize AVS parameters, try to do once
    if (bScaling && !AvsParams.piYCoefsX)
    {
        AvsParams.Format    = Format_None;
        AvsParams.fScaleX   = 0.0F;
        AvsParams.fScaleY   = 0.0F;
        AvsParams.piYCoefsX = nullptr;

        uint32_t ycoeffTableSize  = POLYPHASE_Y_COEFFICIENT_TABLE_SIZE_G9;
        uint32_t uvcoeffTableSize = POLYPHASE_UV_COEFFICIENT_TABLE_SIZE_G9;

        int32_t size = (ycoeffTableSize + uvcoeffTableSize) * 2;

        uint8_t *ptr = (uint8_t*)MOS_AllocAndZeroMemory(size);
        if (ptr == nullptr)
        {
            CODECHAL_DECODE_ASSERTMESSAGE("No memory to allocate AVS coefficient tables.");
            eStatus = MOS_STATUS_NO_SPACE;
            return eStatus;
        }

        AvsParams.piYCoefsX  = (int32_t*)ptr; 

        ptr += ycoeffTableSize;
        AvsParams.piUVCoefsX = (int32_t*)ptr;

        ptr += uvcoeffTableSize;
        AvsParams.piYCoefsY  = (int32_t*)ptr;

        ptr += ycoeffTableSize;
        AvsParams.piUVCoefsY = (int32_t*)ptr;
    }

    return eStatus;
}

CodechalSfcState::~CodechalSfcState()
{
    CODECHAL_HW_FUNCTION_ENTER;

    if (MhwSfcInterface::SFC_PIPE_MODE_VEBOX == m_sfcPipeMode)
    {
        pOsInterface->pfnDestroySyncResource(pOsInterface, &resSyncObject);
    }

    // Free AVS Line Buffer
    pOsInterface->pfnFreeResource(pOsInterface, &resAvsLineBuffer);
    // Free resLaceOrAceOrRgbHistogram
    pOsInterface->pfnFreeResource(pOsInterface, &resLaceOrAceOrRgbHistogram);
    // Free resStatisticsOutput
    pOsInterface->pfnFreeResource(pOsInterface, &resStatisticsOutput);

    // Free buffers in AVS parameters
    MOS_FreeMemory(AvsParams.piYCoefsX);
    AvsParams.piYCoefsX = nullptr;
}

MOS_STATUS CodechalSfcState::SetVeboxStateParams(
    PMHW_VEBOX_STATE_CMD_PARAMS  veboxCmdParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;

    veboxCmdParams->bNoUseVeboxHeap                         = 1;

    veboxCmdParams->VeboxMode.ColorGamutExpansionEnable     = 0;
    veboxCmdParams->VeboxMode.ColorGamutCompressionEnable   = 0;
    // On SKL, GlobalIECP must be enabled when the output pipe is Vebox or SFC 
    veboxCmdParams->VeboxMode.GlobalIECPEnable              = 1;
    veboxCmdParams->VeboxMode.DNEnable                      = 0;
    veboxCmdParams->VeboxMode.DIEnable                      = 0;
    veboxCmdParams->VeboxMode.DNDIFirstFrame                = 0;
    veboxCmdParams->VeboxMode.DIOutputFrames                = 0;
    veboxCmdParams->VeboxMode.PipeSynchronizeDisable        = 0;
    veboxCmdParams->VeboxMode.DemosaicEnable                = 0;
    veboxCmdParams->VeboxMode.VignetteEnable                = 0;
    veboxCmdParams->VeboxMode.AlphaPlaneEnable              = 0;
    veboxCmdParams->VeboxMode.HotPixelFilteringEnable       = 0;
    // 0-both slices enabled   1-Slice 0 enabled   2-Slice 1 enabled
    // On SKL GT3 and GT4, there are 2 Veboxes. But only Vebox0 can be used,Vebox1 cannot be used
    veboxCmdParams->VeboxMode.SingleSliceVeboxEnable        = 1;
    veboxCmdParams->VeboxMode.LACECorrectionEnable          = 0;
    veboxCmdParams->VeboxMode.DisableEncoderStatistics      = 1;
    veboxCmdParams->VeboxMode.DisableTemporalDenoiseFilter  = 1;
    veboxCmdParams->VeboxMode.SinglePipeIECPEnable          = 0;
    veboxCmdParams->VeboxMode.SFCParallelWriteEnable        = 0;
    veboxCmdParams->VeboxMode.ScalarMode                    = 0;
    veboxCmdParams->VeboxMode.ForwardGammaCorrectionEnable  = 0;

    return eStatus;
}

MOS_STATUS CodechalSfcState::SetVeboxSurfaceStateParams(
    PMHW_VEBOX_SURFACE_STATE_CMD_PARAMS  veboxSurfParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;

    // Initialize SurfInput
    veboxSurfParams->SurfInput.bActive                = true;
    veboxSurfParams->SurfInput.Format                 = pInputSurface->Format;
    veboxSurfParams->SurfInput.dwWidth                = pInputSurface->dwWidth;
    veboxSurfParams->SurfInput.dwHeight               = pInputSurface->UPlaneOffset.iYOffset;// For Planar formats, pParams->SurfInput.dwHeight will be assigned to VEBOX U.Y offset, which is only used for PLANAR surface formats.
    veboxSurfParams->SurfInput.dwUYoffset             = pInputSurface->UPlaneOffset.iYOffset;
    veboxSurfParams->SurfInput.dwPitch                = pInputSurface->dwPitch;
    veboxSurfParams->SurfInput.TileType               = pInputSurface->TileType;
    veboxSurfParams->SurfInput.pOsResource            = &pInputSurface->OsResource;
    veboxSurfParams->SurfInput.rcMaxSrc.left          = 0;
    veboxSurfParams->SurfInput.rcMaxSrc.top           = 0;
    veboxSurfParams->SurfInput.rcMaxSrc.right         = MOS_ALIGN_CEIL(pInputSurface->dwWidth, pSfcInterface->m_veWidthAlignment);
    veboxSurfParams->SurfInput.rcMaxSrc.bottom        = MOS_ALIGN_CEIL(pInputSurface->dwHeight, pSfcInterface->m_veHeightAlignment);

    // Initialize SurfSTMM
    veboxSurfParams->SurfSTMM.dwPitch                 = pInputSurface->dwPitch;

    veboxSurfParams->bDIEnable                        = false;
    veboxSurfParams->bOutputValid                     = (pVeboxOutputSurface != nullptr) ? true : false;

    return eStatus;
}

MOS_STATUS CodechalSfcState::SetVeboxDiIecpParams(
    PMHW_VEBOX_DI_IECP_CMD_PARAMS  veboxDiIecpParams)
{
    uint32_t       size = 0;
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;

    uint32_t height = pInputSurface->dwHeight;
    uint32_t width  = pInputSurface->dwWidth;

    veboxDiIecpParams->dwStartingX             = 0;
    veboxDiIecpParams->dwEndingX               = width - 1;
    veboxDiIecpParams->dwCurrInputSurfOffset   = pInputSurface->dwOffset;
    veboxDiIecpParams->pOsResCurrInput         = &pInputSurface->OsResource;
    veboxDiIecpParams->CurrInputSurfCtrl.Value = 0;  //Keep it here untill VPHAL moving to new CMD definition and remove this parameter definition.

    CodecHalGetResourceInfo(
        pOsInterface,
        pInputSurface);

    veboxDiIecpParams->CurInputSurfMMCState = (MOS_MEMCOMP_STATE)(pInputSurface->CompressionMode);

    // Allocate Resource to avoid Page Fault issue since HW will access it
    if (Mos_ResourceIsNull(&resLaceOrAceOrRgbHistogram))
    {
        pHwInterface->GetHcpInterface()->GetOsResLaceOrAceOrRgbHistogramBufferSize(
            width,
            height,
            &size);

        MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
        MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
        allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
        allocParamsForBufferLinear.Format = Format_Buffer;
        allocParamsForBufferLinear.dwBytes = size;
        allocParamsForBufferLinear.pBufName = "ResLaceOrAceOrRgbHistogram";

        pOsInterface->pfnAllocateResource(
            pOsInterface,
            &allocParamsForBufferLinear,
            &resLaceOrAceOrRgbHistogram);
    }

    veboxDiIecpParams->pOsResLaceOrAceOrRgbHistogram = &resLaceOrAceOrRgbHistogram;

    // Allocate Resource to avoid Page Fault issue since HW will access it
    if (Mos_ResourceIsNull(&resStatisticsOutput))
    {
        pHwInterface->GetHcpInterface()->GetOsResStatisticsOutputBufferSize(
            width,
            height,
            &size);

        MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
        MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
        allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
        allocParamsForBufferLinear.Format = Format_Buffer;
        allocParamsForBufferLinear.dwBytes = size;
        allocParamsForBufferLinear.pBufName = "ResStatisticsOutput";

        pOsInterface->pfnAllocateResource(
            pOsInterface,
            &allocParamsForBufferLinear,
            &resStatisticsOutput);
    }

    veboxDiIecpParams->pOsResStatisticsOutput = &resStatisticsOutput;

    return eStatus;
}

MOS_STATUS CodechalSfcState::SetSfcStateParams(
    PMHW_SFC_STATE_PARAMS          sfcStateParams,
    PMHW_SFC_OUT_SURFACE_PARAMS    outSurfaceParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;

    CODECHAL_HW_CHK_NULL_RETURN(sfcStateParams);

    CODECHAL_HW_CHK_STATUS_RETURN(UpdateInputInfo(sfcStateParams));

    sfcStateParams->sfcPipeMode                    = m_sfcPipeMode;
    sfcStateParams->dwChromaDownSamplingMode       = MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_DISABLED; // IN: NV12
    sfcStateParams->bAVSChromaUpsamplingEnable     = bScaling;

    if ((sfcStateParams->fAVSXScalingRatio > 1.0F) || (sfcStateParams->fAVSYScalingRatio > 1.0F))
    {
        sfcStateParams->bBypassXAdaptiveFilter     = false;
        sfcStateParams->bBypassYAdaptiveFilter     = false;
    }
    else
    {
        sfcStateParams->bBypassXAdaptiveFilter     = true;
        sfcStateParams->bBypassYAdaptiveFilter     = true;
    }

    sfcStateParams->fChromaSubSamplingXSiteOffset  = 0.0F;
    sfcStateParams->fChromaSubSamplingYSiteOffset  = 0.0F;

    uint16_t widthAlignUnit  = 1;
    uint16_t heightAlignUnit = 1;

    switch(pSfcOutputSurface->Format)
    {
        case Format_NV12:
        case Format_P010:
            widthAlignUnit     = 2;
            heightAlignUnit    = 2;
            break;
        case Format_YUY2:
        case Format_UYVY:
            widthAlignUnit     = 2;
            break;
        default:
            break;
    }

    // Default to Horizontal Left, Vertical Top
    sfcStateParams->dwChromaDownSamplingHorizontalCoef = (uiChromaSiting & MHW_CHROMA_SITING_HORZ_CENTER) ?
                                                  MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_COEF_4_OVER_8 :
                                                  ((uiChromaSiting & MHW_CHROMA_SITING_HORZ_RIGHT) ?
                                                  MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_COEF_8_OVER_8 :
                                                  MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_COEF_0_OVER_8);

    sfcStateParams->dwChromaDownSamplingVerticalCoef = (uiChromaSiting  & MHW_CHROMA_SITING_VERT_CENTER) ?
                                                  MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_COEF_4_OVER_8 :
                                                  ((uiChromaSiting & MHW_CHROMA_SITING_VERT_BOTTOM) ?
                                                  MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_COEF_8_OVER_8 :
                                                  MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_COEF_0_OVER_8);

    outSurfaceParams->dwWidth              = pSfcOutputSurface->dwWidth;
    outSurfaceParams->dwHeight             = pSfcOutputSurface->dwHeight;
    outSurfaceParams->dwPitch              = pSfcOutputSurface->dwPitch;
    outSurfaceParams->TileType             = pSfcOutputSurface->TileType;
    outSurfaceParams->ChromaSiting         = uiChromaSiting;
    outSurfaceParams->dwUYoffset           = pSfcOutputSurface->UPlaneOffset.iYOffset;

    sfcStateParams->dwOutputFrameWidth             = MOS_ALIGN_CEIL(pSfcOutputSurface->dwWidth, widthAlignUnit);
    sfcStateParams->dwOutputFrameHeight            = MOS_ALIGN_CEIL(pSfcOutputSurface->dwHeight, heightAlignUnit);
    sfcStateParams->OutputFrameFormat              = pSfcOutputSurface->Format;
    sfcStateParams->dwOutputSurfaceOffset          = pSfcOutputSurface->dwOffset;
    sfcStateParams->pOsResOutputSurface            = &pSfcOutputSurface->OsResource;
    sfcStateParams->pOsResAVSLineBuffer            = &resAvsLineBuffer;

    sfcStateParams->dwSourceRegionHeight           = MOS_ALIGN_FLOOR(rcInputSurfaceRegion.Height, heightAlignUnit);
    sfcStateParams->dwSourceRegionWidth            = MOS_ALIGN_FLOOR(rcInputSurfaceRegion.Width, widthAlignUnit);
    sfcStateParams->dwSourceRegionVerticalOffset   = MOS_ALIGN_CEIL(rcInputSurfaceRegion.Y, heightAlignUnit);
    sfcStateParams->dwSourceRegionHorizontalOffset = MOS_ALIGN_CEIL(rcInputSurfaceRegion.X, widthAlignUnit);
    sfcStateParams->dwScaledRegionHeight           = MOS_ALIGN_CEIL(rcOutputSurfaceRegion.Height, heightAlignUnit);
    sfcStateParams->dwScaledRegionWidth            = MOS_ALIGN_CEIL(rcOutputSurfaceRegion.Width, widthAlignUnit);
    sfcStateParams->dwScaledRegionVerticalOffset   = MOS_ALIGN_FLOOR(rcOutputSurfaceRegion.Y, heightAlignUnit);
    sfcStateParams->dwScaledRegionHorizontalOffset = MOS_ALIGN_FLOOR(rcOutputSurfaceRegion.X, widthAlignUnit);
    sfcStateParams->fAVSXScalingRatio              = fScaleX;
    sfcStateParams->fAVSYScalingRatio              = fScaleY;

    sfcStateParams->fAlphaPixel                    = 1.0F;
    sfcStateParams->bColorFillEnable               = bColorFill;
    sfcStateParams->bCSCEnable                     = bCSC;
    sfcStateParams->bRGBASwapEnable                = sfcStateParams->bCSCEnable;

    // CodecHal does not support SFC rotation
    sfcStateParams->RotationMode                   = MHW_ROTATION_IDENTITY; 

    // For downsampling, expect output surface to be MMC disabled
    // For Jpeg, the only usage is CSC and the output surface format is RGB8, so also disable MMC
    sfcStateParams->bMMCEnable                     = false; 
    sfcStateParams->MMCMode                        = MOS_MMC_DISABLED;

    return eStatus;
}

MOS_STATUS CodechalSfcState::SetSfcAvsStateParams()
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;

    PMHW_SFC_AVS_STATE mhwSfcAvsState = &AvsState;

    if (uiChromaSiting == MHW_CHROMA_SITING_NONE)
    {
        uiChromaSiting = MHW_CHROMA_SITING_HORZ_LEFT | MHW_CHROMA_SITING_VERT_CENTER;
    }

    mhwSfcAvsState->sfcPipeMode             = m_sfcPipeMode;
    mhwSfcAvsState->dwInputHorizontalSiting = (uiChromaSiting & MHW_CHROMA_SITING_HORZ_CENTER) ? SFC_AVS_INPUT_SITING_COEF_4_OVER_8 :
                                               ((uiChromaSiting & MHW_CHROMA_SITING_HORZ_RIGHT) ? SFC_AVS_INPUT_SITING_COEF_8_OVER_8 :
                                               SFC_AVS_INPUT_SITING_COEF_0_OVER_8);

    mhwSfcAvsState->dwInputVerticalSitting = (uiChromaSiting  & MHW_CHROMA_SITING_VERT_CENTER) ? SFC_AVS_INPUT_SITING_COEF_4_OVER_8 :
                                              ((uiChromaSiting & MHW_CHROMA_SITING_VERT_BOTTOM) ? SFC_AVS_INPUT_SITING_COEF_8_OVER_8 :
                                              SFC_AVS_INPUT_SITING_COEF_0_OVER_8);

    CODECHAL_HW_CHK_STATUS_RETURN(pSfcInterface->SetSfcSamplerTable(
                                    &LumaTable,
                                    &ChromaTable,
                                    &AvsParams,
                                    pInputSurface->Format,
                                    fScaleX,
                                    fScaleY,
                                    uiChromaSiting,
                                    (m_sfcPipeMode == MhwSfcInterface::SFC_PIPE_MODE_VEBOX)? true: false));

    LumaTable.sfcPipeMode   = m_sfcPipeMode;
    ChromaTable.sfcPipeMode = m_sfcPipeMode;

    return eStatus;
}

MOS_STATUS CodechalSfcState::SetSfcIefStateParams(
    PMHW_SFC_IEF_STATE_PARAMS  iefStateParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;

    CODECHAL_HW_CHK_NULL_RETURN(iefStateParams);

    iefStateParams->sfcPipeMode    = m_sfcPipeMode;
    iefStateParams->bIEFEnable     = false;
    iefStateParams->bCSCEnable     = true;

    iefStateParams->pfCscCoeff     = fCscCoeff;
    iefStateParams->pfCscInOffset  = fCscInOffset;
    iefStateParams->pfCscOutOffset = fCscOutOffset;

    return eStatus;
}

MOS_STATUS CodechalSfcState::Initialize(
    PCODECHAL_DECODE_PROCESSING_PARAMS  decodeProcParams,
    uint8_t                             sfcPipeMode)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;

    CODECHAL_HW_CHK_NULL_RETURN(pDecoder);

    CODECHAL_HW_CHK_NULL_RETURN(decodeProcParams);
    CODECHAL_HW_CHK_NULL_RETURN(decodeProcParams->pInputSurface);
    CODECHAL_HW_CHK_NULL_RETURN(decodeProcParams->pOutputSurface);

    m_sfcPipeMode        = sfcPipeMode;

    pInputSurface       = decodeProcParams->pInputSurface;
    // Vebox o/p should not be written to memory for SFC, VeboxOutputSurface should be nullptr
    pVeboxOutputSurface = nullptr;
    pSfcOutputSurface   = decodeProcParams->pOutputSurface;

    uint16_t widthAlignUnit  = 1;
    uint16_t heightAlignUnit = 1;

    switch(pSfcOutputSurface->Format)
    {
        case Format_NV12:
            widthAlignUnit     = 2;
            heightAlignUnit    = 2;
            break;
        case Format_YUY2:
        case Format_UYVY:
            widthAlignUnit     = 2;
            break;
        default:
            break;
    }

    // Calculate bScaling
    uint32_t sourceRegionWidth  = MOS_ALIGN_FLOOR(decodeProcParams->rcInputSurfaceRegion.Width, widthAlignUnit);
    uint32_t sourceRegionHeight = MOS_ALIGN_FLOOR(decodeProcParams->rcInputSurfaceRegion.Height, heightAlignUnit);
    uint32_t outputRegionWidth  = MOS_ALIGN_CEIL(decodeProcParams->rcOutputSurfaceRegion.Width, widthAlignUnit);
    uint32_t outputRegionHeight = MOS_ALIGN_CEIL(decodeProcParams->rcOutputSurfaceRegion.Height, heightAlignUnit);

    fScaleX = (float)outputRegionWidth / (float)sourceRegionWidth;
    fScaleY = (float)outputRegionHeight / (float)sourceRegionHeight;

    bScaling       = ((fScaleX == 1.0F) && (fScaleY == 1.0F)) ? false : true;
    bColorFill     = false;

    if (decodeProcParams->pOutputSurface->Format == Format_A8R8G8B8)
    {
        bCSC       = true;
    }

    if (bJpegInUse && ucJpegChromaType == jpegBGR)
    {
        bCSC       = false;
    }

    if (bCSC)
    {
        if (bJpegInUse && ucJpegChromaType == jpegRGB)
        {
            fCscCoeff[0]     = 1.000000000f;
            fCscCoeff[1]     = 0.000000000f;
            fCscCoeff[2]     = 0.000000000f;
            fCscCoeff[3]     = 0.000000000f;
            fCscCoeff[4]     = 1.000000000f;
            fCscCoeff[5]     = 0.000000000f;
            fCscCoeff[6]     = 0.000000000f;
            fCscCoeff[7]     = 0.000000000f;
            fCscCoeff[8]     = 1.000000000f;

            fCscInOffset[0]  = 0.000000000f; // Adjusted to S8.2 to accommodate VPHAL
            fCscInOffset[1]  = 0.000000000f; // Adjusted to S8.2 to accommodate VPHAL
            fCscInOffset[2]  = 0.000000000f; // Adjusted to S8.2 to accommodate VPHAL
        }
        else
        {
            if (pInputSurface->Format != Format_400P)
            {
                fCscCoeff[0] = 1.16438353f;
                fCscCoeff[1] = 0.000000000f;
                fCscCoeff[2] = 1.59602666f;
                fCscCoeff[3] = 1.16438353f;
                fCscCoeff[4] = -0.391761959f;
                fCscCoeff[5] = -0.812967300f;
                fCscCoeff[6] = 1.16438353f;
                fCscCoeff[7] = 2.01723218f;
                fCscCoeff[8] = 0.000000000f;
            }
            else
            {
                fCscCoeff[0] = 1.16438353f;
                fCscCoeff[1] = 0.000000000f;
                fCscCoeff[2] = 0.000000000f;
                fCscCoeff[3] = 1.16438353f;
                fCscCoeff[4] = 0.000000000f;
                fCscCoeff[5] = 0.000000000f;
                fCscCoeff[6] = 1.16438353f;
                fCscCoeff[7] = 0.000000000f;
                fCscCoeff[8] = 0.000000000f;
            }

            fCscInOffset[0]  = -16.000000f;  // Adjusted to S8.2 to accommodate VPHAL 
            fCscInOffset[1]  = -128.000000f; // Adjusted to S8.2 to accommodate VPHAL
            fCscInOffset[2]  = -128.000000f; // Adjusted to S8.2 to accommodate VPHAL
        }

        fCscOutOffset[0]     = 0.000000000f; // Adjusted to S8.2 to accommodate VPHAL
        fCscOutOffset[1]     = 0.000000000f; // Adjusted to S8.2 to accommodate VPHAL
        fCscOutOffset[2]     = 0.000000000f; // Adjusted to S8.2 to accommodate VPHAL
    }

    uiChromaSiting = decodeProcParams->uiChromaSitingType;
    uiRotationMode = decodeProcParams->uiRotationState;

    eStatus = MOS_SecureMemcpy(&rcInputSurfaceRegion,
                               sizeof(rcInputSurfaceRegion),
                               &decodeProcParams->rcInputSurfaceRegion,
                               sizeof(decodeProcParams->rcInputSurfaceRegion));

    eStatus = MOS_SecureMemcpy(&rcOutputSurfaceRegion,
                               sizeof(rcOutputSurfaceRegion),
                               &decodeProcParams->rcOutputSurfaceRegion,
                               sizeof(decodeProcParams->rcOutputSurfaceRegion));

    CODECHAL_HW_CHK_STATUS_RETURN(AllocateResources());

    if (MhwSfcInterface::SFC_PIPE_MODE_VEBOX == sfcPipeMode)
    {
        // Create VEBOX Context
        CODECHAL_HW_CHK_STATUS_RETURN(pOsInterface->pfnCreateGpuContext(
            pOsInterface,
            MOS_GPU_CONTEXT_VEBOX,
            MOS_GPU_NODE_VE,
            MOS_GPU_CONTEXT_CREATE_DEFAULT));

        // Register Vebox GPU context with the Batch Buffer completion event
        // Ignore if creation fails
        CODECHAL_HW_CHK_STATUS_RETURN(pOsInterface->pfnRegisterBBCompleteNotifyEvent(
            pOsInterface,
            MOS_GPU_CONTEXT_VEBOX));
    }

    return eStatus;
}

MOS_STATUS CodechalSfcState::AddSfcCommands(
    PMOS_COMMAND_BUFFER             cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;

    CODECHAL_HW_CHK_NULL_RETURN(cmdBuffer);

    if (bSfcPipeOut == false)
    {
        return eStatus;
    }

    MHW_SFC_LOCK_PARAMS sfcLockParams;
    MOS_ZeroMemory(&sfcLockParams, sizeof(sfcLockParams));

    sfcLockParams.sfcPipeMode     = m_sfcPipeMode;
    sfcLockParams.bOutputToMemory = ((MhwSfcInterface::SFC_PIPE_MODE_VEBOX != m_sfcPipeMode) && !bJpegInUse);

    MHW_SFC_STATE_PARAMS sfcStateParams;
    MOS_ZeroMemory(&sfcStateParams, sizeof(sfcStateParams));
    MHW_SFC_OUT_SURFACE_PARAMS sfcOutSurfaceParams;
    MOS_ZeroMemory(&sfcOutSurfaceParams, sizeof(sfcOutSurfaceParams));
    CODECHAL_HW_CHK_STATUS_RETURN(SetSfcStateParams(&sfcStateParams, &sfcOutSurfaceParams));

    CODECHAL_HW_CHK_STATUS_RETURN(pSfcInterface->AddSfcLock(cmdBuffer, &sfcLockParams));
    CODECHAL_HW_CHK_STATUS_RETURN(pSfcInterface->AddSfcState(cmdBuffer, &sfcStateParams, &sfcOutSurfaceParams));

    if (bScaling)
    {
        CODECHAL_HW_CHK_STATUS_RETURN(SetSfcAvsStateParams());
        CODECHAL_HW_CHK_STATUS_RETURN(pSfcInterface->AddSfcAvsState(cmdBuffer, &AvsState));
        CODECHAL_HW_CHK_STATUS_RETURN(pSfcInterface->AddSfcAvsLumaTable(cmdBuffer, &LumaTable));
        CODECHAL_HW_CHK_STATUS_RETURN(pSfcInterface->AddSfcAvsChromaTable(cmdBuffer, &ChromaTable));
    }

    if (bCSC)
    {
        MHW_SFC_IEF_STATE_PARAMS sfcIefStateParams;
        MOS_ZeroMemory(&sfcIefStateParams, sizeof(sfcIefStateParams));
        CODECHAL_HW_CHK_STATUS_RETURN(SetSfcIefStateParams(&sfcIefStateParams));
        CODECHAL_HW_CHK_STATUS_RETURN(pSfcInterface->AddSfcIefState(cmdBuffer, &sfcIefStateParams));
    }

    CODECHAL_HW_CHK_STATUS_RETURN(pSfcInterface->AddSfcFrameStart(cmdBuffer, m_sfcPipeMode));

    return eStatus;
}

MOS_STATUS CodechalSfcState::RenderStart()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;

    MOS_SYNC_PARAMS syncParams = g_cInitSyncParams;
    syncParams.GpuContext = pDecoder->GetVideoContext();
    syncParams.presSyncResource = &resSyncObject;

    CODECHAL_DECODE_CHK_STATUS_RETURN(pOsInterface->pfnEngineSignal(pOsInterface, &syncParams));

    syncParams = g_cInitSyncParams;
    syncParams.GpuContext = MOS_GPU_CONTEXT_VEBOX;
    syncParams.presSyncResource = &resSyncObject;

    CODECHAL_DECODE_CHK_STATUS_RETURN(pOsInterface->pfnEngineWait(pOsInterface, &syncParams));

    // Switch GPU context to VEBOX
    pOsInterface->pfnSetGpuContext(pOsInterface, MOS_GPU_CONTEXT_VEBOX);
    // Reset allocation list and house keeping
    pOsInterface->pfnResetOsStates(pOsInterface);

    // Send command buffer header at the beginning
    MOS_COMMAND_BUFFER cmdBuffer;
    MOS_ZeroMemory(&cmdBuffer, sizeof(MOS_COMMAND_BUFFER));
    CODECHAL_HW_CHK_STATUS_RETURN(pOsInterface->pfnGetCommandBuffer(pOsInterface, &cmdBuffer, 0));
    CODECHAL_HW_CHK_STATUS_RETURN(pDecoder->SendPrologWithFrameTracking(&cmdBuffer, true));

    // Setup cmd prameters
    MHW_VEBOX_STATE_CMD_PARAMS veboxStateCmdParams;
    MOS_ZeroMemory(&veboxStateCmdParams, sizeof(veboxStateCmdParams));
    CODECHAL_HW_CHK_STATUS_RETURN(SetVeboxStateParams(&veboxStateCmdParams));

    MHW_VEBOX_SURFACE_STATE_CMD_PARAMS veboxSurfaceStateCmdParams;
    MOS_ZeroMemory(&veboxSurfaceStateCmdParams, sizeof(veboxSurfaceStateCmdParams));
    CODECHAL_HW_CHK_STATUS_RETURN(SetVeboxSurfaceStateParams(&veboxSurfaceStateCmdParams));

    MHW_VEBOX_DI_IECP_CMD_PARAMS veboxDiIecpCmdParams;
    MOS_ZeroMemory(&veboxDiIecpCmdParams, sizeof(veboxDiIecpCmdParams));
    CODECHAL_HW_CHK_STATUS_RETURN(SetVeboxDiIecpParams(&veboxDiIecpCmdParams));

    // send Vebox and SFC cmds
    CODECHAL_HW_CHK_STATUS_RETURN(pVeboxInterface->AddVeboxState(&cmdBuffer, &veboxStateCmdParams, 0));

    CODECHAL_HW_CHK_STATUS_RETURN(pVeboxInterface->AddVeboxSurfaces(&cmdBuffer, &veboxSurfaceStateCmdParams));

    CODECHAL_HW_CHK_STATUS_RETURN(AddSfcCommands(&cmdBuffer));

    CODECHAL_HW_CHK_STATUS_RETURN(pVeboxInterface->AddVeboxDiIecp(&cmdBuffer, &veboxDiIecpCmdParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(pHwInterface->GetMiInterface()->AddWatchdogTimerStopCmd(
        &cmdBuffer));

    CODECHAL_DECODE_CHK_STATUS_RETURN(pHwInterface->GetMiInterface()->AddMiBatchBufferEnd(
        &cmdBuffer,
        nullptr));

    pOsInterface->pfnReturnCommandBuffer(pOsInterface, &cmdBuffer, 0);
    CODECHAL_HW_CHK_STATUS_RETURN(pOsInterface->pfnSubmitCommandBuffer(
        pOsInterface,
        &cmdBuffer,
        pDecoder->GetVideoContextUsesNullHw()));

    pOsInterface->pfnFreeResource(
        pOsInterface,
        &veboxStateCmdParams.DummyIecpResource);

    return eStatus;
}

bool CodechalSfcState::IsSfcFormatSupported(
    MOS_FORMAT                  inputFormat,
    MOS_FORMAT                  outputFormat)
{
    if ((inputFormat != Format_NV12) &&
        (inputFormat != Format_400P) &&
        (inputFormat != Format_IMC3) &&
        (inputFormat != Format_422H) &&
        (inputFormat != Format_444P) &&
        (inputFormat != Format_P010))
    {
        CODECHAL_DECODE_ASSERTMESSAGE("Unsupported Input Format '0x%08x' for SFC.", inputFormat);
        return false;
    }

    if (outputFormat != Format_A8R8G8B8 &&
        outputFormat != Format_NV12     &&
        outputFormat != Format_P010     &&
        outputFormat != Format_YUY2)
    {
        CODECHAL_DECODE_ASSERTMESSAGE("Unsupported Output Format '0x%08x' for SFC.", outputFormat);
        return false;
    }

    return true;
}

bool CodechalSfcState::IsSfcOutputSupported(
    PCODECHAL_DECODE_PROCESSING_PARAMS  decodeProcParams,
    uint8_t                             sfcPipeMode)
{
    CODECHAL_HW_FUNCTION_ENTER;

    if (!pSfcInterface || !decodeProcParams || !decodeProcParams->pInputSurface || !decodeProcParams->pOutputSurface)
    {
        CODECHAL_DECODE_ASSERTMESSAGE("Invalid Parameters");
        return false;
    }

    PMOS_SURFACE srcSurface = decodeProcParams->pInputSurface;
    PMOS_SURFACE destSurface = decodeProcParams->pOutputSurface;

    uint32_t srcSurfWidth, srcSurfHeight;
    if (MhwSfcInterface::SFC_PIPE_MODE_VEBOX == sfcPipeMode)
    {
        // Adjust SFC input surface alignment.
        // As VEBOX doesn't do scaling, input size equals to output size
        // For the VEBOX output to SFC, width is multiple of 16 and height is multiple of 4
        srcSurface->dwWidth  = MOS_ALIGN_CEIL(srcSurface->dwWidth, pSfcInterface->m_veWidthAlignment);
        srcSurface->dwHeight = MOS_ALIGN_CEIL(srcSurface->dwHeight, pSfcInterface->m_veHeightAlignment);
        srcSurfWidth        = srcSurface->dwWidth;
        srcSurfHeight       = srcSurface->dwHeight;
    }
    else
    {
        // Check original input size (for JPEG)
        if (!MOS_WITHIN_RANGE(srcSurface->dwWidth, pSfcInterface->m_minWidth, pSfcInterface->m_maxWidth)     ||
            !MOS_WITHIN_RANGE(srcSurface->dwHeight, pSfcInterface->m_minHeight, pSfcInterface->m_maxHeight))
        {
            CODECHAL_DECODE_ASSERTMESSAGE("Unsupported Input Resolution '0x%08x'x'0x%08x' for SFC.", srcSurface->dwWidth, srcSurface->dwHeight);
            return false;
        }

        srcSurfWidth  = MOS_ALIGN_CEIL(srcSurface->dwWidth, CODECHAL_SFC_ALIGNMENT_16);
        srcSurfHeight = MOS_ALIGN_CEIL(srcSurface->dwHeight, CODECHAL_SFC_ALIGNMENT_16);
    }

    // Check input size
    if (!MOS_WITHIN_RANGE(srcSurfWidth, pSfcInterface->m_minWidth, pSfcInterface->m_maxWidth)  || 
        !MOS_WITHIN_RANGE(srcSurfHeight, pSfcInterface->m_minHeight, pSfcInterface->m_maxHeight))
    {
        CODECHAL_DECODE_ASSERTMESSAGE("Unsupported Input Resolution '0x%08x'x'0x%08x' for SFC.", srcSurfWidth, srcSurfHeight);
        return false;
    }

    // Adjust SFC output surface alignment.
    uint16_t widthAlignUnit = 1;
    uint16_t heightAlignUnit = 1;
    switch(destSurface->Format)
    {
        case Format_NV12:
            widthAlignUnit     = 2;
            heightAlignUnit    = 2;
            break;
        case Format_YUY2:
        case Format_UYVY:
            widthAlignUnit     = 2;
            break;
        default:
            break;
    }

    uint32_t dstSurfWidth  = MOS_ALIGN_CEIL(destSurface->dwWidth, widthAlignUnit);
    uint32_t dstSurfHeight = MOS_ALIGN_CEIL(destSurface->dwHeight, heightAlignUnit);

    // Check input and output format (limited only to current decode processing usage)
    if (!IsSfcFormatSupported(srcSurface->Format, destSurface->Format))
    {
        return false;
    }

    // Check input region rectangles
    uint32_t sourceRegionWidth = MOS_ALIGN_FLOOR(decodeProcParams->rcInputSurfaceRegion.Width, widthAlignUnit);
    uint32_t sourceRegionHeight = MOS_ALIGN_FLOOR(decodeProcParams->rcInputSurfaceRegion.Height, heightAlignUnit);

    if ((sourceRegionWidth > srcSurface->dwWidth) ||
        (sourceRegionHeight > srcSurface->dwHeight))
    {
        CODECHAL_DECODE_ASSERTMESSAGE("Input region is out of bound for SFC.");
        return false;
    }

    // Check output size
    if (!MOS_WITHIN_RANGE(dstSurfWidth, pSfcInterface->m_minWidth, pSfcInterface->m_maxWidth)     || 
        !MOS_WITHIN_RANGE(dstSurfHeight, pSfcInterface->m_minHeight, pSfcInterface->m_maxHeight))
    {
        CODECHAL_DECODE_ASSERTMESSAGE("Unsupported Output Resolution '0x%08x'x'0x%08x' for SFC.", dstSurfWidth, dstSurfHeight);
        return false;
    }

    // Check output region rectangles
    uint32_t outputRegionWidth = MOS_ALIGN_CEIL(decodeProcParams->rcOutputSurfaceRegion.Width, widthAlignUnit);
    uint32_t outputRegionHeight = MOS_ALIGN_CEIL(decodeProcParams->rcOutputSurfaceRegion.Height, heightAlignUnit);

    if ((outputRegionWidth > destSurface->dwWidth) ||
        (outputRegionHeight > destSurface->dwHeight))
    {
        CODECHAL_DECODE_ASSERTMESSAGE("Output region is out of bound for SFC.");
        return false;
    }

    // Check scaling ratio
    // SFC scaling range is [0.125, 8] for both X and Y direction.
    fScaleX = (float)outputRegionWidth / (float)sourceRegionWidth;
    fScaleY = (float)outputRegionHeight / (float)sourceRegionHeight;

    if (!MOS_WITHIN_RANGE(fScaleX, pSfcInterface->m_minScalingRatio, pSfcInterface->m_maxScalingRatio) ||
        !MOS_WITHIN_RANGE(fScaleY, pSfcInterface->m_minScalingRatio, pSfcInterface->m_maxScalingRatio))
    {
        CODECHAL_DECODE_ASSERTMESSAGE("Scaling factor not supported by SFC Pipe.");
        return false;
    }

    return true;
}

MOS_STATUS CodechalSfcState::InitializeSfcState(
    CodechalDecode *inDecoder,
    CodechalHwInterface   *hwInterface,
    PMOS_INTERFACE osInterface)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(inDecoder);
    CODECHAL_DECODE_CHK_NULL_RETURN(hwInterface);
    CODECHAL_DECODE_CHK_NULL_RETURN(osInterface);
    CODECHAL_DECODE_CHK_NULL_RETURN(hwInterface->GetVeboxInterface());

    pDecoder = inDecoder;
    pOsInterface = osInterface;
    pHwInterface = hwInterface;
    pVeboxInterface = hwInterface->GetVeboxInterface();
    pSfcInterface = hwInterface->GetSfcInterface();    // No need to check null for pSfcInterface. It will be checked in IsSfcSupported().

    return eStatus;
}
