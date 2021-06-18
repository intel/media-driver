/*
* Copyright (c) 2019, Intel Corporation
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
//! \file     mos_os_virtualengine_singlepipe_specific.cpp
//! \brief    Implements the MOS interface extension for Linux, supporting virtual engine single pipe mode.
//! \details  Implements the MOS interface extension for Linux, supporting virtual engine single pipe mode. Only necessary when KMD supports virtual engine.
//!

#include "mos_os_virtualengine_singlepipe_specific_next.h"

MOS_STATUS MosOsVeSinglePipeSpecific::Initialize(
    MOS_STREAM_HANDLE stream,
    PMOS_VIRTUALENGINE_INIT_PARAMS   pVEInitParms)
{
    MOS_STATUS     eStatus = MOS_STATUS_SUCCESS;

    return eStatus;
}

MOS_STATUS MosOsVeSinglePipeSpecific::GetHintParams(
    bool bScalableMode,
    PMOS_VIRTUALENGINE_HINT_PARAMS *ppHintParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(ppHintParams);
    MOS_OS_CHECK_CONDITION((bScalableMode), "Invalid input paramerter of scalable mode", MOS_STATUS_INVALID_PARAMETER);

    *ppHintParams = &SinglePipeHintParams;

    return eStatus;
}


