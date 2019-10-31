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
//! \file     codechal_decode_sfc_avc_g12.cpp
//! \brief    Implements the decode interface extension for CSC and scaling via SFC for AVC decoder for G12+ platform.
//! \details  Downsampling in this case is supported by the SFC fixed function HW unit.
//!

#include "codechal_decoder.h"
#include "codechal_decode_sfc_avc_g12.h"
#include "codechal_mmc.h"

MOS_STATUS CodechalAvcSfcStateG12::SetSfcStateParams(
    PMHW_SFC_STATE_PARAMS               sfcStateParams,
    PMHW_SFC_OUT_SURFACE_PARAMS         outSurfaceParams)
{
    CODECHAL_HW_FUNCTION_ENTER;

    CODECHAL_HW_CHK_STATUS_RETURN(CodechalSfcState::SetSfcStateParams(sfcStateParams, outSurfaceParams));

    if (CodecHalMmcState::IsMmcEnabled())
    {
        MOS_MEMCOMP_STATE mmcMode   = MOS_MEMCOMP_DISABLED;
        CODECHAL_HW_CHK_STATUS_RETURN(m_osInterface->pfnGetMemoryCompressionMode(m_osInterface, &m_sfcOutputSurface->OsResource, &mmcMode));
        sfcStateParams->bMMCEnable  = (mmcMode != MOS_MEMCOMP_DISABLED) ? true : false;
        sfcStateParams->MMCMode     = (mmcMode == MOS_MEMCOMP_RC) ? MOS_MMC_RC : MOS_MMC_MC;
        CODECHAL_HW_CHK_STATUS_RETURN(m_osInterface->pfnGetMemoryCompressionFormat(
            m_osInterface, &m_sfcOutputSurface->OsResource, &outSurfaceParams->dwCompressionFormat));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalAvcSfcStateG12::AddSfcCommands(
    PMOS_COMMAND_BUFFER                 cmdBuffer)
{
    CODECHAL_HW_FUNCTION_ENTER;

    CODECHAL_HW_CHK_NULL_RETURN(cmdBuffer);

    if (m_sfcPipeOut == false)
    {
        return MOS_STATUS_SUCCESS;
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

    return MOS_STATUS_SUCCESS;
}

bool CodechalAvcSfcStateG12::IsSfcFormatSupported(
    MOS_FORMAT                  inputFormat,
    MOS_FORMAT                  outputFormat)
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

MOS_STATUS CodechalAvcSfcStateG12::CheckAndInitialize(
    PCODECHAL_DECODE_PROCESSING_PARAMS  decProcessingParams,
    PCODEC_AVC_PIC_PARAMS               picParams,
    uint32_t                            width,
    uint32_t                            height,
    bool                                deblockingEnabled)
{
    MOS_STATUS status = MOS_STATUS_SUCCESS;

    status = CodechalAvcSfcState::CheckAndInitialize(decProcessingParams,picParams,width, height, deblockingEnabled);

    if(m_sfcPipeOut)
    {
        m_histogramSurface = decProcessingParams->pHistogramSurface;
    }

    return status;
}


MOS_STATUS CodechalAvcSfcStateG12::UpdateInputInfo(
    PMHW_SFC_STATE_PARAMS               sfcStateParams)
{
    MOS_STATUS status = MOS_STATUS_SUCCESS;

    status = CodechalAvcSfcState::UpdateInputInfo(sfcStateParams);

    PMHW_SFC_STATE_PARAMS_G12 sfcStateParamsG12 = static_cast<PMHW_SFC_STATE_PARAMS_G12>(sfcStateParams);

    sfcStateParamsG12->histogramSurface = m_histogramSurface;

    return status;
}