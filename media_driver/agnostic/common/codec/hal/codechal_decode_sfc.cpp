/*
* Copyright (c) 2014-2019, Intel Corporation
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
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnCreateSyncResource(m_osInterface, &m_resSyncObject));
    }

    // Allocate AVS line buffer
    if (Mos_ResourceIsNull(&m_resAvsLineBuffer))
    {
        MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
        MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBufferLinear.Type     = MOS_GFXRES_BUFFER;
        allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
        allocParamsForBufferLinear.Format   = Format_Buffer;
        if (MhwSfcInterface::SFC_PIPE_MODE_VEBOX == m_sfcPipeMode)
        {
            allocParamsForBufferLinear.dwBytes = MOS_ROUNDUP_DIVIDE(m_inputSurface->dwHeight, 8) * 5 * MHW_SFC_CACHELINE_SIZE;
        }
        else
        {
            allocParamsForBufferLinear.dwBytes = MOS_ROUNDUP_DIVIDE(m_inputSurface->dwWidth, 8) * 3 * MHW_SFC_CACHELINE_SIZE;
        }
        allocParamsForBufferLinear.pBufName = "SfcAvsLineBuffer";

        eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBufferLinear,
            &m_resAvsLineBuffer);

        if (eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_DECODE_ASSERTMESSAGE("Failed to allocate Sfc Avs Line Buffer.");
            return eStatus;
        }
    }

    // Allocate IEF line buffer

    //Initialize AVS parameters, try to do once
    if (m_scaling && !m_avsParams.piYCoefsX)
    {
        m_avsParams.Format    = Format_None;
        m_avsParams.fScaleX   = 0.0F;
        m_avsParams.fScaleY   = 0.0F;
        m_avsParams.piYCoefsX = nullptr;

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

        m_avsParams.piYCoefsX = (int32_t *)ptr;

        ptr += ycoeffTableSize;
        m_avsParams.piUVCoefsX = (int32_t *)ptr;

        ptr += uvcoeffTableSize;
        m_avsParams.piYCoefsY = (int32_t *)ptr;

        ptr += ycoeffTableSize;
        m_avsParams.piUVCoefsY = (int32_t *)ptr;
    }

    return eStatus;
}

CodechalSfcState::~CodechalSfcState()
{
    CODECHAL_HW_FUNCTION_ENTER;

    if (MhwSfcInterface::SFC_PIPE_MODE_VEBOX == m_sfcPipeMode)
    {
        m_osInterface->pfnDestroySyncResource(m_osInterface, &m_resSyncObject);
    }

    // Free AVS Line Buffer
    m_osInterface->pfnFreeResource(m_osInterface, &m_resAvsLineBuffer);
    // Free resLaceOrAceOrRgbHistogram
    m_osInterface->pfnFreeResource(m_osInterface, &m_resLaceOrAceOrRgbHistogram);
    // Free resStatisticsOutput
    m_osInterface->pfnFreeResource(m_osInterface, &m_resStatisticsOutput);

    // Free buffers in AVS parameters
    MOS_FreeMemory(m_avsParams.piYCoefsX);
    m_avsParams.piYCoefsX = nullptr;
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
    veboxSurfParams->SurfInput.Format                 = m_inputSurface->Format;
    veboxSurfParams->SurfInput.dwWidth                = m_inputSurface->dwWidth;
    veboxSurfParams->SurfInput.dwHeight               = m_inputSurface->UPlaneOffset.iYOffset;  // For Planar formats, pParams->SurfInput.dwHeight will be assigned to VEBOX U.Y offset, which is only used for PLANAR surface formats.
    veboxSurfParams->SurfInput.dwUYoffset             = m_inputSurface->UPlaneOffset.iYOffset;
    veboxSurfParams->SurfInput.dwPitch                = m_inputSurface->dwPitch;
    veboxSurfParams->SurfInput.TileType               = m_inputSurface->TileType;
    veboxSurfParams->SurfInput.TileModeGMM            = m_inputSurface->TileModeGMM;
    veboxSurfParams->SurfInput.bGMMTileEnabled        = m_inputSurface->bGMMTileEnabled;
    veboxSurfParams->SurfInput.pOsResource            = &m_inputSurface->OsResource;
    veboxSurfParams->SurfInput.rcMaxSrc.left          = 0;
    veboxSurfParams->SurfInput.rcMaxSrc.top           = 0;
    veboxSurfParams->SurfInput.rcMaxSrc.right         = MOS_ALIGN_CEIL(m_inputSurface->dwWidth, m_sfcInterface->m_veWidthAlignment);
    veboxSurfParams->SurfInput.rcMaxSrc.bottom        = MOS_ALIGN_CEIL(m_inputSurface->dwHeight, m_sfcInterface->m_veHeightAlignment);

    // Initialize SurfSTMM
    veboxSurfParams->SurfSTMM.dwPitch = m_inputSurface->dwPitch;

    veboxSurfParams->bDIEnable                        = false;
    veboxSurfParams->bOutputValid                     = (m_veboxOutputSurface != nullptr) ? true : false;

    return eStatus;
}

MOS_STATUS CodechalSfcState::SetVeboxDiIecpParams(
    PMHW_VEBOX_DI_IECP_CMD_PARAMS  veboxDiIecpParams)
{
    uint32_t       size = 0;
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;

    uint32_t height = m_inputSurface->dwHeight;
    uint32_t width  = m_inputSurface->dwWidth;

    veboxDiIecpParams->dwStartingX             = 0;
    veboxDiIecpParams->dwEndingX               = width - 1;
    veboxDiIecpParams->dwCurrInputSurfOffset   = m_inputSurface->dwOffset;
    veboxDiIecpParams->pOsResCurrInput         = &m_inputSurface->OsResource;
    veboxDiIecpParams->CurrInputSurfCtrl.Value = 0;  //Keep it here untill VPHAL moving to new CMD definition and remove this parameter definition.

    CodecHalGetResourceInfo(
        m_osInterface,
        m_inputSurface);

    veboxDiIecpParams->CurInputSurfMMCState = (MOS_MEMCOMP_STATE)(m_inputSurface->CompressionMode);

    // Allocate Resource to avoid Page Fault issue since HW will access it
    if (Mos_ResourceIsNull(&m_resLaceOrAceOrRgbHistogram))
    {
        m_hwInterface->GetHcpInterface()->GetOsResLaceOrAceOrRgbHistogramBufferSize(
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

        m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBufferLinear,
            &m_resLaceOrAceOrRgbHistogram);
    }

    veboxDiIecpParams->pOsResLaceOrAceOrRgbHistogram = &m_resLaceOrAceOrRgbHistogram;

    // Allocate Resource to avoid Page Fault issue since HW will access it
    if (Mos_ResourceIsNull(&m_resStatisticsOutput))
    {
        m_hwInterface->GetHcpInterface()->GetOsResStatisticsOutputBufferSize(
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

        m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBufferLinear,
            &m_resStatisticsOutput);
    }

    veboxDiIecpParams->pOsResStatisticsOutput = &m_resStatisticsOutput;

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
    sfcStateParams->bAVSChromaUpsamplingEnable     = m_scaling;

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

    CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalGetResourceInfo(
    m_osInterface,
    m_sfcOutputSurface));

    switch (m_sfcOutputSurface->Format)
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
    sfcStateParams->dwChromaDownSamplingHorizontalCoef = (m_chromaSiting & MHW_CHROMA_SITING_HORZ_CENTER) ? MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_COEF_4_OVER_8 : ((m_chromaSiting & MHW_CHROMA_SITING_HORZ_RIGHT) ? MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_COEF_8_OVER_8 : MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_COEF_0_OVER_8);

    sfcStateParams->dwChromaDownSamplingVerticalCoef = (m_chromaSiting & MHW_CHROMA_SITING_VERT_CENTER) ? MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_COEF_4_OVER_8 : ((m_chromaSiting & MHW_CHROMA_SITING_VERT_BOTTOM) ? MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_COEF_8_OVER_8 : MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_COEF_0_OVER_8);

    outSurfaceParams->dwWidth      = m_sfcOutputSurface->dwWidth;
    outSurfaceParams->dwHeight     = m_sfcOutputSurface->dwHeight;
    outSurfaceParams->dwPitch      = m_sfcOutputSurface->dwPitch;
    outSurfaceParams->TileType     = m_sfcOutputSurface->TileType;
    outSurfaceParams->ChromaSiting = m_chromaSiting;
    outSurfaceParams->dwUYoffset   = m_sfcOutputSurface->UPlaneOffset.iYOffset;
    outSurfaceParams->TileModeGMM  = m_sfcOutputSurface->TileModeGMM;
    outSurfaceParams->bGMMTileEnabled = m_sfcOutputSurface->bGMMTileEnabled;

    sfcStateParams->dwOutputFrameWidth    = MOS_ALIGN_CEIL(m_sfcOutputSurface->dwWidth, widthAlignUnit);
    sfcStateParams->dwOutputFrameHeight   = MOS_ALIGN_CEIL(m_sfcOutputSurface->dwHeight, heightAlignUnit);
    sfcStateParams->OutputFrameFormat     = m_sfcOutputSurface->Format;
    sfcStateParams->dwOutputSurfaceOffset = m_sfcOutputSurface->dwOffset;
    sfcStateParams->pOsResOutputSurface   = &m_sfcOutputSurface->OsResource;
    sfcStateParams->pOsResAVSLineBuffer   = &m_resAvsLineBuffer;

    sfcStateParams->dwSourceRegionHeight           = MOS_ALIGN_FLOOR(m_inputSurfaceRegion.Height, heightAlignUnit);
    sfcStateParams->dwSourceRegionWidth            = MOS_ALIGN_FLOOR(m_inputSurfaceRegion.Width, widthAlignUnit);
    sfcStateParams->dwSourceRegionVerticalOffset   = MOS_ALIGN_CEIL(m_inputSurfaceRegion.Y, heightAlignUnit);
    sfcStateParams->dwSourceRegionHorizontalOffset = MOS_ALIGN_CEIL(m_inputSurfaceRegion.X, widthAlignUnit);
    sfcStateParams->dwScaledRegionHeight           = MOS_ALIGN_CEIL(m_outputSurfaceRegion.Height, heightAlignUnit);
    sfcStateParams->dwScaledRegionWidth            = MOS_ALIGN_CEIL(m_outputSurfaceRegion.Width, widthAlignUnit);
    sfcStateParams->dwScaledRegionVerticalOffset   = MOS_ALIGN_FLOOR(m_outputSurfaceRegion.Y, heightAlignUnit);
    sfcStateParams->dwScaledRegionHorizontalOffset = MOS_ALIGN_FLOOR(m_outputSurfaceRegion.X, widthAlignUnit);
    sfcStateParams->fAVSXScalingRatio              = m_scaleX;
    sfcStateParams->fAVSYScalingRatio              = m_scaleY;

    sfcStateParams->fAlphaPixel                    = 1.0F;
    sfcStateParams->bColorFillEnable               = m_colorFill;
    sfcStateParams->bCSCEnable                     = m_csc;
    // ARGB8,ABGR10 output format need to enable swap
    if (m_sfcOutputSurface->Format == Format_X8R8G8B8 ||
        m_sfcOutputSurface->Format == Format_A8R8G8B8 ||
        m_sfcOutputSurface->Format == Format_R10G10B10A2)
    {
        sfcStateParams->bRGBASwapEnable = true;
    }
    else
    {
        sfcStateParams->bRGBASwapEnable = false;
    }

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

    PMHW_SFC_AVS_STATE mhwSfcAvsState = &m_avsState;

    if (m_chromaSiting == MHW_CHROMA_SITING_NONE)
    {
        m_chromaSiting = MHW_CHROMA_SITING_HORZ_LEFT | MHW_CHROMA_SITING_VERT_CENTER;
    }

    mhwSfcAvsState->sfcPipeMode             = m_sfcPipeMode;
    mhwSfcAvsState->dwInputHorizontalSiting = (m_chromaSiting & MHW_CHROMA_SITING_HORZ_CENTER) ? SFC_AVS_INPUT_SITING_COEF_4_OVER_8 : ((m_chromaSiting & MHW_CHROMA_SITING_HORZ_RIGHT) ? SFC_AVS_INPUT_SITING_COEF_8_OVER_8 : SFC_AVS_INPUT_SITING_COEF_0_OVER_8);

    mhwSfcAvsState->dwInputVerticalSitting = (m_chromaSiting & MHW_CHROMA_SITING_VERT_CENTER) ? SFC_AVS_INPUT_SITING_COEF_4_OVER_8 : ((m_chromaSiting & MHW_CHROMA_SITING_VERT_BOTTOM) ? SFC_AVS_INPUT_SITING_COEF_8_OVER_8 : SFC_AVS_INPUT_SITING_COEF_0_OVER_8);

    CODECHAL_HW_CHK_STATUS_RETURN(m_sfcInterface->SetSfcSamplerTable(
        &m_lumaTable,
        &m_chromaTable,
        &m_avsParams,
        m_inputSurface->Format,
        m_scaleX,
        m_scaleY,
        m_chromaSiting,
        (m_sfcPipeMode != MhwSfcInterface::SFC_PIPE_MODE_VDBOX) ? true : false));

    m_lumaTable.sfcPipeMode   = m_sfcPipeMode;
    m_chromaTable.sfcPipeMode = m_sfcPipeMode;

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

    iefStateParams->pfCscCoeff     = m_cscCoeff;
    iefStateParams->pfCscInOffset  = m_cscInOffset;
    iefStateParams->pfCscOutOffset = m_cscOutOffset;

    return eStatus;
}

MOS_STATUS CodechalSfcState::Initialize(
    PCODECHAL_DECODE_PROCESSING_PARAMS  decodeProcParams,
    uint8_t                             sfcPipeMode)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;

    CODECHAL_HW_CHK_NULL_RETURN(m_decoder);

    CODECHAL_HW_CHK_NULL_RETURN(decodeProcParams);
    CODECHAL_HW_CHK_NULL_RETURN(decodeProcParams->pInputSurface);
    CODECHAL_HW_CHK_NULL_RETURN(decodeProcParams->pOutputSurface);

    m_sfcPipeMode        = sfcPipeMode;

    m_inputSurface = decodeProcParams->pInputSurface;
    // Vebox o/p should not be written to memory for SFC, VeboxOutputSurface should be nullptr
    m_veboxOutputSurface = nullptr;
    m_sfcOutputSurface   = decodeProcParams->pOutputSurface;

    uint16_t widthAlignUnit  = 1;
    uint16_t heightAlignUnit = 1;

    switch (m_sfcOutputSurface->Format)
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

    m_scaleX = (float)outputRegionWidth / (float)sourceRegionWidth;
    m_scaleY = (float)outputRegionHeight / (float)sourceRegionHeight;

    m_scaling   = ((m_scaleX == 1.0F) && (m_scaleY == 1.0F)) ? false : true;
    m_colorFill = false;

    if (decodeProcParams->pOutputSurface->Format == Format_A8R8G8B8)
    {
        m_csc = true;
    }

    if (m_jpegInUse && m_jpegChromaType == jpegBGR)
    {
        m_csc = false;
    }

    if (m_csc)
    {
        if (m_jpegInUse && m_jpegChromaType == jpegRGB)
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
            if (m_inputSurface->Format != Format_400P)
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

    m_chromaSiting = decodeProcParams->uiChromaSitingType;
    m_rotationMode = decodeProcParams->uiRotationState;

    eStatus = MOS_SecureMemcpy(&m_inputSurfaceRegion,
        sizeof(m_inputSurfaceRegion),
        &decodeProcParams->rcInputSurfaceRegion,
        sizeof(decodeProcParams->rcInputSurfaceRegion));

    eStatus = MOS_SecureMemcpy(&m_outputSurfaceRegion,
        sizeof(m_outputSurfaceRegion),
        &decodeProcParams->rcOutputSurfaceRegion,
        sizeof(decodeProcParams->rcOutputSurfaceRegion));

    CODECHAL_HW_CHK_STATUS_RETURN(AllocateResources());

    if (MhwSfcInterface::SFC_PIPE_MODE_VEBOX == sfcPipeMode)
    {
        // Create VEBOX Context
        MOS_GPUCTX_CREATOPTIONS createOption;
        CODECHAL_HW_CHK_STATUS_RETURN(m_osInterface->pfnCreateGpuContext(
            m_osInterface,
            MOS_GPU_CONTEXT_VEBOX,
            MOS_GPU_NODE_VE,
            &createOption));

        // Register Vebox GPU context with the Batch Buffer completion event
        // Ignore if creation fails
        CODECHAL_HW_CHK_STATUS_RETURN(m_osInterface->pfnRegisterBBCompleteNotifyEvent(
            m_osInterface,
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

    if (m_sfcPipeOut == false)
    {
        return eStatus;
    }

    MHW_SFC_LOCK_PARAMS sfcLockParams;
    MOS_ZeroMemory(&sfcLockParams, sizeof(sfcLockParams));

    sfcLockParams.sfcPipeMode     = m_sfcPipeMode;
    sfcLockParams.bOutputToMemory = ((MhwSfcInterface::SFC_PIPE_MODE_VEBOX != m_sfcPipeMode) && !m_jpegInUse);

    MHW_SFC_STATE_PARAMS sfcStateParams;
    MOS_ZeroMemory(&sfcStateParams, sizeof(sfcStateParams));
    MHW_SFC_OUT_SURFACE_PARAMS sfcOutSurfaceParams;
    MOS_ZeroMemory(&sfcOutSurfaceParams, sizeof(sfcOutSurfaceParams));
    CODECHAL_HW_CHK_STATUS_RETURN(SetSfcStateParams(&sfcStateParams, &sfcOutSurfaceParams));

    CODECHAL_HW_CHK_STATUS_RETURN(m_sfcInterface->AddSfcLock(cmdBuffer, &sfcLockParams));
    CODECHAL_HW_CHK_STATUS_RETURN(m_sfcInterface->AddSfcState(cmdBuffer, &sfcStateParams, &sfcOutSurfaceParams));

    if (m_scaling)
    {
        CODECHAL_HW_CHK_STATUS_RETURN(SetSfcAvsStateParams());
        CODECHAL_HW_CHK_STATUS_RETURN(m_sfcInterface->AddSfcAvsState(cmdBuffer, &m_avsState));
        CODECHAL_HW_CHK_STATUS_RETURN(m_sfcInterface->AddSfcAvsLumaTable(cmdBuffer, &m_lumaTable));
        CODECHAL_HW_CHK_STATUS_RETURN(m_sfcInterface->AddSfcAvsChromaTable(cmdBuffer, &m_chromaTable));
    }

    if (m_csc)
    {
        MHW_SFC_IEF_STATE_PARAMS sfcIefStateParams;
        MOS_ZeroMemory(&sfcIefStateParams, sizeof(sfcIefStateParams));
        CODECHAL_HW_CHK_STATUS_RETURN(SetSfcIefStateParams(&sfcIefStateParams));
        CODECHAL_HW_CHK_STATUS_RETURN(m_sfcInterface->AddSfcIefState(cmdBuffer, &sfcIefStateParams));
    }

    CODECHAL_HW_CHK_STATUS_RETURN(m_sfcInterface->AddSfcFrameStart(cmdBuffer, m_sfcPipeMode));

    return eStatus;
}

MOS_STATUS CodechalSfcState::RenderStart()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;

    MOS_SYNC_PARAMS syncParams = g_cInitSyncParams;
    syncParams.GpuContext       = m_decoder->GetVideoContext();
    syncParams.presSyncResource = &m_resSyncObject;

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineSignal(m_osInterface, &syncParams));

    syncParams = g_cInitSyncParams;
    syncParams.GpuContext = MOS_GPU_CONTEXT_VEBOX;
    syncParams.presSyncResource = &m_resSyncObject;

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineWait(m_osInterface, &syncParams));

    // Switch GPU context to VEBOX
    m_osInterface->pfnSetGpuContext(m_osInterface, MOS_GPU_CONTEXT_VEBOX);
    // Reset allocation list and house keeping
    m_osInterface->pfnResetOsStates(m_osInterface);

    // Send command buffer header at the beginning
    MOS_COMMAND_BUFFER cmdBuffer;
    MOS_ZeroMemory(&cmdBuffer, sizeof(MOS_COMMAND_BUFFER));
    CODECHAL_HW_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));
    CODECHAL_HW_CHK_STATUS_RETURN(m_decoder->SendPrologWithFrameTracking(&cmdBuffer, true));

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
    CODECHAL_HW_CHK_STATUS_RETURN(m_veboxInterface->AddVeboxState(&cmdBuffer, &veboxStateCmdParams, 0));

    CODECHAL_HW_CHK_STATUS_RETURN(m_veboxInterface->AddVeboxSurfaces(&cmdBuffer, &veboxSurfaceStateCmdParams));

    CODECHAL_HW_CHK_STATUS_RETURN(AddSfcCommands(&cmdBuffer));

    CODECHAL_HW_CHK_STATUS_RETURN(m_veboxInterface->AddVeboxDiIecp(&cmdBuffer, &veboxDiIecpCmdParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hwInterface->GetMiInterface()->AddMiBatchBufferEnd(
        &cmdBuffer,
        nullptr));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);
    CODECHAL_HW_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(
        m_osInterface,
        &cmdBuffer,
        m_decoder->GetVideoContextUsesNullHw()));

    m_osInterface->pfnFreeResource(
        m_osInterface,
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

    if (!m_sfcInterface || !decodeProcParams || !decodeProcParams->pInputSurface || !decodeProcParams->pOutputSurface)
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
        srcSurface->dwWidth  = MOS_ALIGN_CEIL(srcSurface->dwWidth, m_sfcInterface->m_veWidthAlignment);
        srcSurface->dwHeight = MOS_ALIGN_CEIL(srcSurface->dwHeight, m_sfcInterface->m_veHeightAlignment);
        srcSurfWidth        = srcSurface->dwWidth;
        srcSurfHeight       = srcSurface->dwHeight;
    }
    else
    {
        // Check original input size (for JPEG)
        if (!MOS_WITHIN_RANGE(srcSurface->dwWidth, m_sfcInterface->m_minWidth, m_sfcInterface->m_maxWidth) ||
            !MOS_WITHIN_RANGE(srcSurface->dwHeight, m_sfcInterface->m_minHeight, m_sfcInterface->m_maxHeight))
        {
            return false;
        }

        srcSurfWidth  = MOS_ALIGN_CEIL(srcSurface->dwWidth, CODECHAL_SFC_ALIGNMENT_16);
        srcSurfHeight = MOS_ALIGN_CEIL(srcSurface->dwHeight, CODECHAL_SFC_ALIGNMENT_16);
    }

    // Check input size
    if (!MOS_WITHIN_RANGE(srcSurfWidth, m_sfcInterface->m_minWidth, m_sfcInterface->m_maxWidth) ||
        !MOS_WITHIN_RANGE(srcSurfHeight, m_sfcInterface->m_minHeight, m_sfcInterface->m_maxHeight))
    {
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
        return false;
    }

    // Check output size
    if (!MOS_WITHIN_RANGE(dstSurfWidth, m_sfcInterface->m_minWidth, m_sfcInterface->m_maxWidth) ||
        !MOS_WITHIN_RANGE(dstSurfHeight, m_sfcInterface->m_minHeight, m_sfcInterface->m_maxHeight))
    {
        return false;
    }

    // Check output region rectangles
    uint32_t outputRegionWidth = MOS_ALIGN_CEIL(decodeProcParams->rcOutputSurfaceRegion.Width, widthAlignUnit);
    uint32_t outputRegionHeight = MOS_ALIGN_CEIL(decodeProcParams->rcOutputSurfaceRegion.Height, heightAlignUnit);

    if ((outputRegionWidth > destSurface->dwWidth) ||
        (outputRegionHeight > destSurface->dwHeight))
    {
        return false;
    }

    // Check scaling ratio
    // SFC scaling range is [0.125, 8] for both X and Y direction.
    m_scaleX = (float)outputRegionWidth / (float)sourceRegionWidth;
    m_scaleY = (float)outputRegionHeight / (float)sourceRegionHeight;

    if (!MOS_WITHIN_RANGE(m_scaleX, m_sfcInterface->m_minScalingRatio, m_sfcInterface->m_maxScalingRatio) ||
        !MOS_WITHIN_RANGE(m_scaleY, m_sfcInterface->m_minScalingRatio, m_sfcInterface->m_maxScalingRatio))
    {
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

    m_decoder        = inDecoder;
    m_osInterface    = osInterface;
    m_hwInterface    = hwInterface;
    m_veboxInterface = hwInterface->GetVeboxInterface();
    m_sfcInterface   = hwInterface->GetSfcInterface();  // No need to check null for pSfcInterface. It will be checked in IsSfcSupported().

    return eStatus;
}
