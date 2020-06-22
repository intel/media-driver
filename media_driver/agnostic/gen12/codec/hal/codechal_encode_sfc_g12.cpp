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
//! \file     codechal_encode_sfc_g12.cpp
//! \brief    Implements the encode interface extension for CSC via VEBox/SFC.
//! \details  Downsampling in this case is supported by the VEBox fixed function HW unit.
//!

#include "codechal_encode_sfc_g12.h"
#include "codechal_encoder_base.h"
#include "mhw_sfc_g12_X.h"

MOS_STATUS CodecHalEncodeSfcG12::AddSfcCommands(
    PMHW_SFC_INTERFACE              sfcInterface,
    PMOS_COMMAND_BUFFER             cmdBuffer)
{
    MHW_SFC_LOCK_PARAMS            sfcLockParams;
    MHW_SFC_STATE_PARAMS_G12       sfcStateParams;
    MHW_SFC_OUT_SURFACE_PARAMS     sfcOutSurfaceParams;
    MHW_SFC_IEF_STATE_PARAMS       sfcIefStateParams;
    MOS_STATUS                     eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(sfcInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);

    MOS_ZeroMemory(&sfcLockParams, sizeof(sfcLockParams));

    sfcLockParams.sfcPipeMode = MhwSfcInterface::SFC_PIPE_MODE_VEBOX;
    sfcLockParams.bOutputToMemory = false;

    MOS_ZeroMemory(&sfcStateParams, sizeof(sfcStateParams));
    MOS_ZeroMemory(&sfcOutSurfaceParams, sizeof(sfcOutSurfaceParams));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSfcStateParams(sfcInterface, &sfcStateParams, &sfcOutSurfaceParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(sfcInterface->AddSfcLock(cmdBuffer, &sfcLockParams));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(sfcInterface->AddSfcState(cmdBuffer, &sfcStateParams, &sfcOutSurfaceParams));

    if (m_scaling)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSfcAvsStateParams(sfcInterface));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(sfcInterface->AddSfcAvsState(cmdBuffer, &m_avsState));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(sfcInterface->AddSfcAvsLumaTable(cmdBuffer, &m_lumaTable));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(sfcInterface->AddSfcAvsChromaTable(cmdBuffer, &m_chromaTable));
    }

    if (m_CSC)
    {
        MOS_ZeroMemory(&sfcIefStateParams, sizeof(sfcIefStateParams));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSfcIefStateParams(&sfcIefStateParams));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(sfcInterface->AddSfcIefState(cmdBuffer, &sfcIefStateParams));
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(sfcInterface->AddSfcFrameStart(cmdBuffer, MhwSfcInterface::SFC_PIPE_MODE_VEBOX));

    return eStatus;
}