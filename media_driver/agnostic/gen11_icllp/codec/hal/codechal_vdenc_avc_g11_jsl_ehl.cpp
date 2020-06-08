/*
* Copyright (c) 2020, Intel Corporation
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
//! \file     codechal_vdenc_avc_g11_jsl_ehl.h
//! \brief    This file defines the base C++ class/interface for JSL and EHL
//!           AVC VDENC encoding to be used across CODECHAL components.
//!

#include "codechal_vdenc_avc_g11_jsl_ehl.h"

MOS_STATUS CodechalVdencAvcStateG11JslEhl::Initialize(CodechalSetting* settings)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    // common initilization
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalVdencAvcState::Initialize(settings));
    m_cscDsState->DisableSfc(); // EHL and JSL don't support SFC

    return eStatus;
}

void CodechalVdencAvcStateG11JslEhl::MotionEstimationDisableCheck()
{
    m_16xMeSupported = false;
    m_32xMeSupported = false;
}
