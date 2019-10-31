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
//! \file     codechal_decode_sfc_jpeg_g12.cpp
//! \brief    Implements the decode interface extension for CSC and scaling via SFC for JPEG decoder for G12+ platform.
//! \details  Downsampling in this case is supported by the SFC fixed function HW unit.
//!

#include "codechal_decode_sfc_jpeg_g12.h"
#include "codechal_mmc.h"

MOS_STATUS CodechalJpegSfcStateG12::AddSfcCommands(
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

MOS_STATUS CodechalJpegSfcStateG12::SetSfcStateParams(
    PMHW_SFC_STATE_PARAMS               sfcStateParams,
    PMHW_SFC_OUT_SURFACE_PARAMS         outSurfaceParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;

    CODECHAL_HW_CHK_STATUS_RETURN(CodechalSfcState::SetSfcStateParams(sfcStateParams, outSurfaceParams));

    if (CodecHalMmcState::IsMmcEnabled())
    {
        MOS_MEMCOMP_STATE mmcMode = MOS_MEMCOMP_DISABLED;
        CODECHAL_HW_CHK_STATUS_RETURN(m_osInterface->pfnGetMemoryCompressionMode(m_osInterface, &m_sfcOutputSurface->OsResource, &mmcMode));
        sfcStateParams->bMMCEnable = (mmcMode != MOS_MEMCOMP_DISABLED) ? true : false;
        sfcStateParams->MMCMode = (mmcMode == MOS_MEMCOMP_RC) ? MOS_MMC_RC : MOS_MMC_MC;
        CODECHAL_HW_CHK_STATUS_RETURN(m_osInterface->pfnGetMemoryCompressionFormat(
            m_osInterface, &m_sfcOutputSurface->OsResource, &outSurfaceParams->dwCompressionFormat));
    }

    return eStatus;
}