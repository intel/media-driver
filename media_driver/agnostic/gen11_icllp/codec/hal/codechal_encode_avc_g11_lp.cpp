/*
* Copyright (c) 2017, Intel Corporation
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
//! \file     codechal_encode_avc_g11_lp.cpp
//! \brief    AVC dual-pipe encoder for GEN11 ICL LP.
//!
#include "codechal_encode_avc_g11.h"
#include "codechal_encode_avc_g11_lp.h"
#include "codeckrnheader.h"

CodechalEncodeAvcEncG11LP::CodechalEncodeAvcEncG11LP(
        CodechalHwInterface *   hwInterface,
        CodechalDebugInterface *debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo) : CodechalEncodeAvcEncG11(hwInterface, debugInterface, standardInfo)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    m_kuid = IDR_CODEC_AllAVCEnc;
    AddIshSize(m_kuid, m_kernelBase);

    m_kuidCommon = IDR_CODEC_HME_DS_SCOREBOARD_KERNEL;
    AddIshSize(m_kuidCommon, m_kernelBase);
}

