/*
* Copyright (c) 2017-2019, Intel Corporation
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
//! \file     codechal_decode_sfc_vp9_g12.cpp
//! \brief    Implements the decode interface extension for CSC and scaling via SFC for VP9 decoder for G12+ platform.
//! \details  Downsampling in this case is supported by the SFC fixed function HW unit.
//!

#include "codechal_decode_sfc_vp9_g12.h"

CodechalVp9SfcStateG12::CodechalVp9SfcStateG12()
{
    CODECHAL_HW_FUNCTION_ENTER;

    MOS_ZeroMemory(&m_vp9PicParams, sizeof(m_vp9PicParams));
    MOS_ZeroMemory(&m_scalabilityState, sizeof(m_scalabilityState));
}

CodechalVp9SfcStateG12::~CodechalVp9SfcStateG12()
{
    CODECHAL_HW_FUNCTION_ENTER;

    // Free AVS Line Buffer
    if (m_resAvsLineBuffers)
    {
        for (int i = 0; i < m_numBuffersAllocated; i++)
        {
            m_osInterface->pfnFreeResource(m_osInterface, m_resAvsLineBuffers + i);
        }
        MOS_FreeMemory(m_resAvsLineBuffers);
        m_resAvsLineBuffers = nullptr;
    }
    // Free SFD Line Buffer
    if (m_resSfdLineBuffers)
    {
        for (int i = 0; i < m_numBuffersAllocated; i++)
        {
            m_osInterface->pfnFreeResource(m_osInterface, m_resSfdLineBuffers + i);
        }
        MOS_FreeMemory(m_resSfdLineBuffers);
        m_resSfdLineBuffers = nullptr;
    }
    // Free AVS Line Tile Buffer
    m_osInterface->pfnFreeResource(m_osInterface, &m_resAvsLineTileBuffer);
    // Free SFD Line Tile Buffer
    m_osInterface->pfnFreeResource(m_osInterface, &m_resSfdLineTileBuffer);
}

MOS_STATUS CodechalVp9SfcStateG12::AllocateResources()
{
    if (m_numBuffersAllocated < m_numPipe)
    {
        // Allocate AVS line buffer for input row store
        if (m_resAvsLineBuffers)
        {
            for (int i = 0; i < m_numBuffersAllocated; i++)
            {
                m_osInterface->pfnFreeResource(m_osInterface, m_resAvsLineBuffers + i);
            }
            MOS_FreeMemory(m_resAvsLineBuffers);
            m_resAvsLineBuffers = nullptr;
        }
        if (m_resAvsLineBuffers == nullptr)
        {
            m_resAvsLineBuffers = (MOS_RESOURCE *)MOS_AllocAndZeroMemory(sizeof(MOS_RESOURCE) * m_numPipe);
            CODECHAL_HW_CHK_NULL_RETURN(m_resAvsLineBuffers);

            MOS_ALLOC_GFXRES_PARAMS allocParams;
            MOS_ZeroMemory(&allocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));
            allocParams.Type = MOS_GFXRES_BUFFER;
            allocParams.TileType = MOS_TILE_LINEAR;
            allocParams.Format = Format_Buffer;
            allocParams.dwBytes = MOS_ROUNDUP_DIVIDE(m_inputFrameWidth, 8) * 6 * MHW_SFC_CACHELINE_SIZE;
            allocParams.pBufName = "SfcAvsLineBuffer";

            for (int i = 0; i < m_numPipe; i++)
            {
                CODECHAL_HW_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
                    m_osInterface,
                    &allocParams,
                    m_resAvsLineBuffers + i));
            }
        }

        // Allocate SFD line buffer for output row store, needed for 420
        if (m_resSfdLineBuffers)
        {
            for (int i = 0; i < m_numBuffersAllocated; i++)
            {
                m_osInterface->pfnFreeResource(m_osInterface, m_resSfdLineBuffers + i);
            }
            MOS_FreeMemory(m_resSfdLineBuffers);
            m_resSfdLineBuffers = nullptr;
        }
        if (m_resSfdLineBuffers == nullptr)
        {
            m_resSfdLineBuffers = (MOS_RESOURCE *)MOS_AllocAndZeroMemory(sizeof(MOS_RESOURCE) * m_numPipe);
            CODECHAL_HW_CHK_NULL_RETURN(m_resSfdLineBuffers);

            MOS_ALLOC_GFXRES_PARAMS allocParams;
            MOS_ZeroMemory(&allocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));
            allocParams.Type = MOS_GFXRES_BUFFER;
            allocParams.TileType = MOS_TILE_LINEAR;
            allocParams.Format = Format_Buffer;
            allocParams.dwBytes = MOS_ROUNDUP_DIVIDE(m_outputSurfaceRegion.Width, 10) * MHW_SFC_CACHELINE_SIZE;
            allocParams.pBufName = "SfcSfdLineBuffer";

            for (int i = 0; i < m_numPipe; i++)
            {
                CODECHAL_HW_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
                    m_osInterface,
                    &allocParams,
                    m_resSfdLineBuffers + i));
            }
        }
        m_numBuffersAllocated = m_numPipe;
    }

    // Allocate IEF line buffer - no IEF for HCP SFC

    // Allocate AVS line tile buffer for input column store
    if (Mos_ResourceIsNull(&m_resAvsLineTileBuffer))
    {
        MOS_ALLOC_GFXRES_PARAMS allocParams;
        MOS_ZeroMemory(&allocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParams.Type = MOS_GFXRES_BUFFER;
        allocParams.TileType = MOS_TILE_LINEAR;
        allocParams.Format = Format_Buffer;
        allocParams.dwBytes = MOS_ROUNDUP_DIVIDE(m_inputFrameHeight, 8) * 6 * MHW_SFC_CACHELINE_SIZE * 2; //double for safe
        allocParams.pBufName = "SfcAvsLineTileBuffer";

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParams,
            &m_resAvsLineTileBuffer));
    }
    // Allocate SFD line tile buffer for output column store
    if (Mos_ResourceIsNull(&m_resSfdLineTileBuffer))
    {
        MOS_ALLOC_GFXRES_PARAMS allocParams;
        MOS_ZeroMemory(&allocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParams.Type = MOS_GFXRES_BUFFER;
        allocParams.TileType = MOS_TILE_LINEAR;
        allocParams.Format = Format_Buffer;
        allocParams.dwBytes = MOS_ROUNDUP_DIVIDE(m_outputSurfaceRegion.Width, 10) * MHW_SFC_CACHELINE_SIZE * 2; //double for safe
        allocParams.pBufName = "SfcSfdLineTileBuffer";

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParams,
            &m_resSfdLineTileBuffer));
    }

    //Initialize AVS parameters, try to do once
    if (m_scaling && !m_avsParams.piYCoefsX)
    {
        m_avsParams.Format = Format_None;
        m_avsParams.fScaleX = 0.0F;
        m_avsParams.fScaleY = 0.0F;
        m_avsParams.piYCoefsX = nullptr;

        uint32_t ycoeffTableSize = POLYPHASE_Y_COEFFICIENT_TABLE_SIZE_G9;
        uint32_t uvcoeffTableSize = POLYPHASE_UV_COEFFICIENT_TABLE_SIZE_G9;

        int32_t size = (ycoeffTableSize + uvcoeffTableSize) * 2;

        uint8_t *ptr = (uint8_t*)MOS_AllocAndZeroMemory(size);
        CODECHAL_DECODE_CHK_NULL_RETURN(ptr);

        m_avsParams.piYCoefsX = (int32_t *)ptr;

        ptr += ycoeffTableSize;
        m_avsParams.piUVCoefsX = (int32_t *)ptr;

        ptr += uvcoeffTableSize;
        m_avsParams.piYCoefsY = (int32_t *)ptr;

        ptr += ycoeffTableSize;
        m_avsParams.piUVCoefsY = (int32_t *)ptr;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalVp9SfcStateG12::CheckAndInitialize(
    PCODECHAL_DECODE_PROCESSING_PARAMS  decProcessingParams,
    PCODEC_VP9_PIC_PARAMS               vp9PicParams,
    PCODECHAL_DECODE_SCALABILITY_STATE  scalabilityState)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;

    if (decProcessingParams)
    {
        if (IsSfcOutputSupported(decProcessingParams, MhwSfcInterfaceG12::SFC_PIPE_MODE_HCP))
        {
            m_sfcPipeOut        = true;
            m_vp9PicParams      = vp9PicParams;
            m_scalabilityState  = static_cast<PCODECHAL_DECODE_SCALABILITY_STATE_G12>(scalabilityState);
            m_numPipe           = m_scalabilityState ? m_scalabilityState->ucScalablePipeNum : 1;

            m_histogramSurface = decProcessingParams->pHistogramSurface;

            // Set the input region as the HCP output frame region
            m_inputFrameWidth                                = MOS_ALIGN_CEIL(m_vp9PicParams->FrameWidthMinus1 + 1, CODEC_VP9_MIN_BLOCK_WIDTH);
            m_inputFrameHeight                               = MOS_ALIGN_CEIL(m_vp9PicParams->FrameHeightMinus1 + 1, CODEC_VP9_MIN_BLOCK_HEIGHT);
            decProcessingParams->rcInputSurfaceRegion.X = 0;
            decProcessingParams->rcInputSurfaceRegion.Y = 0;
            decProcessingParams->rcInputSurfaceRegion.Width  = m_inputFrameWidth;
            decProcessingParams->rcInputSurfaceRegion.Height = m_inputFrameHeight;

            CODECHAL_HW_CHK_STATUS_RETURN(Initialize(decProcessingParams, MhwSfcInterfaceG12::SFC_PIPE_MODE_HCP));

            if(m_decoder)
            {
                m_decoder->SetVdSfcSupportedFlag(true);
            }
        }
        else
        {
            if(m_decoder)
            {
                m_decoder->SetVdSfcSupportedFlag(false);
            }
        }
    }

    return eStatus;
}

bool CodechalVp9SfcStateG12::IsSfcFormatSupported(
    MOS_FORMAT   inputFormat,
    MOS_FORMAT   outputFormat)
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
        CODECHAL_DECODE_ASSERTMESSAGE("Unsupported Input Format '0x%08x' for SFC.", inputFormat);
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
        CODECHAL_DECODE_ASSERTMESSAGE("Unsupported Output Format '0x%08x' for SFC.", outputFormat);
        return false;
    }
    return true;
}

MOS_STATUS CodechalVp9SfcStateG12::UpdateInputInfo(
    PMHW_SFC_STATE_PARAMS   sfcStateParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;

    CODECHAL_HW_CHK_NULL_RETURN(sfcStateParams);
    CODECHAL_HW_CHK_NULL_RETURN(m_vp9PicParams);

    uint32_t  chromaIdc = (m_vp9PicParams->subsampling_x == 0 && m_vp9PicParams->subsampling_y == 0) ? HCP_CHROMA_FORMAT_YUV444 : HCP_CHROMA_FORMAT_YUV420;

    PMHW_SFC_STATE_PARAMS_G12 sfcStateParamsG12 = static_cast<PMHW_SFC_STATE_PARAMS_G12>(sfcStateParams);

    sfcStateParamsG12->sfcPipeMode                = MhwSfcInterfaceG12::SFC_PIPE_MODE_HCP;
    sfcStateParamsG12->dwAVSFilterMode            = MEDIASTATE_SFC_AVS_FILTER_8x8;
    sfcStateParamsG12->dwVDVEInputOrderingMode    = MhwSfcInterfaceG12::LCU_64_64_VP9;
    sfcStateParamsG12->dwInputChromaSubSampling   = (HCP_CHROMA_FORMAT_YUV444 == chromaIdc) ? 4 : 1;
    sfcStateParamsG12->dwInputFrameWidth          = m_inputFrameWidth;
    sfcStateParamsG12->dwInputFrameHeight         = m_inputFrameHeight;
    if (m_sfcOutputSurface->Format == Format_NV12 ||
        m_sfcOutputSurface->Format == Format_P010 ||
        m_sfcOutputSurface->Format == Format_P016)
    {
        sfcStateParams->dwChromaDownSamplingHorizontalCoef = (m_chromaSiting & MHW_CHROMA_SITING_HORZ_CENTER) ? MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_COEF_4_OVER_8 : ((m_chromaSiting & MHW_CHROMA_SITING_HORZ_RIGHT) ? MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_COEF_8_OVER_8 : MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_COEF_0_OVER_8);
        sfcStateParams->dwChromaDownSamplingVerticalCoef = (m_chromaSiting & MHW_CHROMA_SITING_VERT_CENTER) ? MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_COEF_4_OVER_8 : ((m_chromaSiting & MHW_CHROMA_SITING_VERT_BOTTOM) ? MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_COEF_8_OVER_8 : MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_COEF_0_OVER_8);
    }
    else if (m_sfcOutputSurface->Format == Format_YUY2 ||
        m_sfcOutputSurface->Format == Format_Y210 ||
        m_sfcOutputSurface->Format == Format_Y216)
    {
        sfcStateParams->dwChromaDownSamplingHorizontalCoef = (m_chromaSiting & MHW_CHROMA_SITING_HORZ_CENTER) ? MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_COEF_4_OVER_8 : ((m_chromaSiting & MHW_CHROMA_SITING_HORZ_RIGHT) ? MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_COEF_8_OVER_8 : MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_COEF_0_OVER_8);
        sfcStateParams->dwChromaDownSamplingVerticalCoef = 0;
    }
    else
    {
        sfcStateParamsG12->dwChromaDownSamplingHorizontalCoef = 0;
        sfcStateParamsG12->dwChromaDownSamplingVerticalCoef = 0;
    }

    sfcStateParamsG12->inputBitDepth              = 0;
    if (m_inputSurface)
    {
        if (m_inputSurface->Format == Format_P010 ||
            m_inputSurface->Format == Format_Y210 ||
            m_inputSurface->Format == Format_Y410)
        {
            sfcStateParamsG12->inputBitDepth = 1;
        }
        else if (m_inputSurface->Format == Format_P016 ||
                 m_inputSurface->Format == Format_Y216 ||
                 m_inputSurface->Format == Format_Y416)
        {
            sfcStateParamsG12->inputBitDepth = 2;
        }
    }

    // Scalability parameters
    if (CodecHalDecodeScalabilityIsScalableMode(m_scalabilityState))
    {
        CODECHAL_DECODE_SFC_SCALABILITY_PARAMS  sfcScalaParams;

        MOS_ZeroMemory(&sfcScalaParams, sizeof(sfcScalaParams));
        CODECHAL_HW_CHK_STATUS_RETURN(CodecHalDecodeScalability_SetSfcState(
            m_scalabilityState, 
            m_vp9PicParams, 
            &m_inputSurfaceRegion,
            &m_outputSurfaceRegion,
            &sfcScalaParams));
        sfcStateParamsG12->engineMode   = sfcScalaParams.engineMode;
        sfcStateParamsG12->tileType     = sfcScalaParams.tileType;
        sfcStateParamsG12->srcStartX    = sfcScalaParams.srcStartX;
        sfcStateParamsG12->srcEndX      = sfcScalaParams.srcEndX;
        sfcStateParamsG12->dstStartX    = sfcScalaParams.dstStartX;
        sfcStateParamsG12->dstEndX      = sfcScalaParams.dstEndX;;

        if (m_scalabilityState->bIsRtMode)
        {
            m_curPipe = m_scalabilityState->u8RtCurPipe;
        }
        else if (CodecHalDecodeScalabilityIsBEPhaseG12(m_scalabilityState))
        {
            m_curPipe = m_scalabilityState->HcpDecPhase - CODECHAL_HCP_DECODE_PHASE_BE0;
        }
        else
        {
            m_curPipe = 0;
        }
    }

    sfcStateParamsG12->histogramSurface = m_histogramSurface;

    return eStatus;
}

MOS_STATUS CodechalVp9SfcStateG12::AddSfcCommands(
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

    sfcLockParams.sfcPipeMode = m_sfcPipeMode;
    sfcLockParams.bOutputToMemory = ((MhwSfcInterface::SFC_PIPE_MODE_VEBOX != m_sfcPipeMode) && !m_jpegInUse);

    MHW_SFC_STATE_PARAMS_G12 sfcStateParams;
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

MOS_STATUS CodechalVp9SfcStateG12::SetSfcStateParams(
    PMHW_SFC_STATE_PARAMS               sfcStateParams,
    PMHW_SFC_OUT_SURFACE_PARAMS         outSurfaceParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;

    CODECHAL_HW_CHK_STATUS_RETURN(CodechalSfcState::SetSfcStateParams(sfcStateParams, outSurfaceParams));

    PMHW_SFC_STATE_PARAMS_G12 sfcStateParamsG12 = static_cast<PMHW_SFC_STATE_PARAMS_G12>(sfcStateParams);
    sfcStateParamsG12->pOsResAVSLineBuffer = m_resAvsLineBuffers + m_curPipe;
    sfcStateParamsG12->resSfdLineBuffer = m_resSfdLineBuffers + m_curPipe;
    sfcStateParamsG12->resAvsLineTileBuffer = &m_resAvsLineTileBuffer;
    sfcStateParamsG12->resSfdLineTileBuffer = &m_resSfdLineTileBuffer;

    // Force output frame size same as output region size
    sfcStateParamsG12->dwOutputFrameWidth = sfcStateParamsG12->dwScaledRegionWidth;
    sfcStateParamsG12->dwOutputFrameHeight = sfcStateParamsG12->dwScaledRegionHeight;

    if (CodecHalMmcState::IsMmcEnabled())
    {
        MOS_MEMCOMP_STATE mmcMode   = MOS_MEMCOMP_DISABLED;
        CODECHAL_HW_CHK_STATUS_RETURN(m_osInterface->pfnGetMemoryCompressionMode(m_osInterface, &m_sfcOutputSurface->OsResource, &mmcMode));
        sfcStateParams->bMMCEnable  = (mmcMode != MOS_MEMCOMP_DISABLED) ? true : false;
        sfcStateParams->MMCMode     = (mmcMode == MOS_MEMCOMP_RC) ? MOS_MMC_RC : MOS_MMC_MC;
        CODECHAL_HW_CHK_STATUS_RETURN(m_osInterface->pfnGetMemoryCompressionFormat(
            m_osInterface, &m_sfcOutputSurface->OsResource, &outSurfaceParams->dwCompressionFormat));
    }

    return eStatus;
}

MOS_STATUS CodechalVp9SfcStateG12::SetSfcAvsStateParams()
{
    CODECHAL_HW_FUNCTION_ENTER;

    CODECHAL_HW_CHK_STATUS_RETURN(CodechalSfcState::SetSfcAvsStateParams());

    PMHW_SFC_AVS_STATE mhwSfcAvsState = &m_avsState;

    if (m_vp9PicParams->subsampling_x == 0 && m_vp9PicParams->subsampling_y == 0) // 444
    {
        mhwSfcAvsState->dwInputHorizontalSiting = 0;
        mhwSfcAvsState->dwInputVerticalSitting = 0;
    }
    else // 420
    {
        mhwSfcAvsState->dwInputHorizontalSiting = (m_chromaSiting & MHW_CHROMA_SITING_HORZ_CENTER) ? SFC_AVS_INPUT_SITING_COEF_4_OVER_8 : ((m_chromaSiting & MHW_CHROMA_SITING_HORZ_RIGHT) ? SFC_AVS_INPUT_SITING_COEF_8_OVER_8 : SFC_AVS_INPUT_SITING_COEF_0_OVER_8);
        mhwSfcAvsState->dwInputVerticalSitting = (m_chromaSiting & MHW_CHROMA_SITING_VERT_CENTER) ? SFC_AVS_INPUT_SITING_COEF_4_OVER_8 : ((m_chromaSiting & MHW_CHROMA_SITING_VERT_BOTTOM) ? SFC_AVS_INPUT_SITING_COEF_8_OVER_8 : SFC_AVS_INPUT_SITING_COEF_0_OVER_8);
    }

    return MOS_STATUS_SUCCESS;
}
